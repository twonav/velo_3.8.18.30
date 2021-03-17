/*
 * I2C client/driver for the Maxim/Dallas DS2782 Stand-Alone Fuel Gauge IC
 *
 * Copyright (C) 2009 Bluewater Systems Ltd
 *
 * Author: Ryan Mallon
 *
 * DS2786 added by Yulia Vilensky <vilensky@compulab.co.il>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/swab.h>
#include <linux/i2c.h>
#include <linux/idr.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/ds2782_battery.h>

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#include <linux/gpio.h>

#include <linux/time.h>

// signal handling
#include <linux/init.h>
#include <asm/siginfo.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

#include <linux/queue.h>
#include <linux/delay.h>

#include "twonav_ds2782_battery.h"

extern char* tn_hwtype;
extern bool tn_is_aventura;
extern bool tn_is_velo;
extern bool tn_is_horizon;
extern bool tn_is_trail;

struct dentry *low_batt_signal_file;
int pid = 0;
struct timespec charging_time_start;
struct timespec eoc_time_start;

// MCPP73833 charge manager related
#define N_STABLE_STAT_VALUES 3
#define EOC_PERIOD_CHECK 5
#define CHARGING_TIMER_THRESHOLD 10800 //3 hours
#define EOC_TIMER_THRESHOLD 43200 // 12 hours
#define MAX8814_RESET_WAIT_TIME 1 // ms
int mcp73833_power_good = 0;
int mcp73833_power_good_previous_value = 0;
int mcp73833_end_of_charge = 0;
int mcp73833_charging = -1; // STAT1
int mcp73833_charged = -1; // STAT2
int mcp73833_n_stat1_stable_values = 0;
int mcp73833_n_stat2_stable_values = 0;

#define RECHARGE_THRESHOLD 95

// AA battery related
#define AA_CAPACITY_EQUAL_MEASUREMENTS 5
#define AA_VOLTAGE_FILTER_SIZE 10
#define AA_TIMER_SAMPLE 5
queue* AA_voltage_queue;
int AA_timer_count = 0;
int AA_consecutive_equal_capacity_measurements = -1;
int AA_stable_capacity_value;
int AA_voltage_sum = 0;

enum BatteryChemistry {
	LionPoly = 0,
	Alkaline = 1,
	Lithium = 2,
	NiMH = 3,
};

enum BatteryStatus {
	NEW_BATTERY = 0,
	LEARN_COMPLETE = 1,
	FULL_CHARGE_DETECTED = 2,
	NEW_BATTERY_CONFIGURED = 3,
	CAPACITY_ESTIMATED = 4,
};

static ssize_t write_pid(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char mybuf[10];
    /* read the value from user space */
    if(count > 10)
        return -EINVAL;
    if (copy_from_user(mybuf, buf, count))
    	return -EFAULT;
    sscanf(mybuf, "%d", &pid);

    return count;
}

static ssize_t send_sigterm(int force)
{
	int ret;
	struct siginfo info;
    struct task_struct *task;

    if (pid == 0) {
    	printk(KERN_INFO "DS2782 no registered pid\n");
    	return 0;
    }

    if (force == 0) {
    	struct timespec ts;
    	getnstimeofday(&ts);
    	if (ts.tv_sec % 10 != 0) // send SIGTERM every 10 seconds
    		return -EPERM;
    }
    else {
    	printk(KERN_INFO "DS2782 send sigterm forced\n");
    }

    /****************************** send the signal *****************************/
    memset(&info, 0, sizeof(struct siginfo));
    info.si_signo = SIGTERM;
    info.si_code = SI_USER;

    rcu_read_lock();
    task = find_task_by_vpid(pid);  //find the task_struct associated with this pid
    if(task == NULL){
        printk("DS2782: no such pid\n");
        rcu_read_unlock();
        return -ENODEV;
    }
    rcu_read_unlock();
    ret = send_sig_info(SIGTERM, &info, task);    //send the signal
    if (ret < 0) {
        printk("DS2782: error sending signal\n");
    }
    return ret;
}

static const struct file_operations my_fops = {
    .write = write_pid,
};

struct task_struct *task;
int charger_enabled = 0;
int learning = 0;
int charge_termination_flag = 0;
int fully_charged = 0;

#define DS2782_REG_Status	0x01
#define DS2782_REG_RAAC		0x02	/* Remaining Active Absolute Capacity */
#define DS2782_REG_RSAC		0x04	/* Remaining Standby Absolute Capacity */
#define DS2782_REG_RARC		0x06	/* Remaining Active Relative Capacity */
#define DS2782_REG_RSRC		0x07	/* Remaining Standby Relative Capacity */

#define DS278x_REG_VOLT_MSB	0x0c
#define DS278x_REG_VOLT_LSB 0x0d

#define DS278x_REG_TEMP_MSB	0x0a
#define DS278x_REG_CURRENT_MSB	0x0e

/* EEPROM Block */
#define DS2782_REG_RSNSP	0x69	/* Sense resistor value */

/* Current unit measurement in uA for a 1 milli-ohm sense resistor */
#define DS2782_CURRENT_UNITS	1563

#define DS2786_REG_RARC		0x02	/* Remaining active relative capacity */

#define DS2782_ACR_MSB 0x10
#define DS2782_ACR_LSB 0x11
#define DS2782_AS 0x14

//DS2782 EEPROM registers
#define DS2782_EEPROM_CONTROL 0x60
#define DS2782_EEPROM_AB 0x61
#define DS2782_EEPROM_AC_MSB 0x62
#define DS2782_EEPROM_AC_LSB 0x63
#define DS2782_EEPROM_VCHG 0x64
#define DS2782_EEPROM_IMIN 0x65
#define DS2782_EEPROM_VAE 0x66
#define DS2782_EEPROM_IAE 0x67
#define DS2782_EEPROM_ActiveEmpty 0x68
#define DS2782_EEPROM_Full40_MSB 0x6A
#define DS2782_EEPROM_Full40_LSB 0x6B
#define DS2782_EEPROM_Full3040Slope 0x6C
#define DS2782_EEPROM_Full2030Slope 0x6D
#define DS2782_EEPROM_Full1020Slope 0x6E
#define DS2782_EEPROM_Full0010Slope 0x6F

#define DS2782_EEPROM_AE3040Slope 0x70
#define DS2782_EEPROM_AE2030Slope 0x71
#define DS2782_EEPROM_AE1020Slope 0x72
#define DS2782_EEPROM_AE0010Slope 0x73
#define DS2782_EEPROM_SE3040Slope 0x74
#define DS2782_EEPROM_SE2030Slope 0x75
#define DS2782_EEPROM_SE1020Slope 0x76
#define DS2782_EEPROM_SE0010Slope 0x77
#define DS2782_EEPROM_RSGAIN_MSB 0x78
#define DS2782_EEPROM_RSGAIN_LSB 0x79
#define DS2782_EEPROM_RSTC 0x7A
#define DS2782_EEPROM_FRSGAIN_MSB 0x7B
#define DS2782_EEPROM_FRSGAIN_LSB 0x7C
#define DS2782_EEPROM_SlaveAddressConfig 0x7E

#define DS2782_Register_Command 0xFE
#define DS2782_Register_LearnComplete 0x20
#define DS2782_Register_Chemistry 0x21

//DS2782 EEPROM values for TwoNav


static const struct twonav_ds_2782_eeprom_config ds_config_velo = {	
	.control = DS2782_EEPROM_VELO_CONTROL_VALUE,
	.ab = DS2782_EEPROM_VELO_AB_VALUE,
	.ac_msb = DS2782_EEPROM_VELO_AC_MSB_VALUE,
	.ac_lsb = DS2782_EEPROM_VELO_AC_LSB_VALUE,
	.vchg = DS2782_EEPROM_VELO_VCHG_VALUE,
	.imin = DS2782_EEPROM_VELO_IMIN_VALUE,
	.vae = DS2782_EEPROM_VELO_VAE_VALUE,
	.iae = DS2782_EEPROM_VELO_IAE_VALUE,
	.active_empty = DS2782_EEPROM_VELO_ActiveEmpty_VALUE,
	.rsns = DS2782_EEPROM_VELO_RSNS_VALUE,
	.full40_msb = DS2782_EEPROM_VELO_Full40_MSB_VALUE,
	.full40_lsb = DS2782_EEPROM_VELO_Full40_LSB_VALUE,
	.full3040slope = DS2782_EEPROM_VELO_Full3040Slope_VALUE,
	.full2030slope = DS2782_EEPROM_VELO_Full2030Slope_VALUE,
	.full1020slope = DS2782_EEPROM_VELO_Full1020Slope_VALUE,
	.full0010slope = DS2782_EEPROM_VELO_Full0010Slope_VALUE,
	.ae3040slope = DS2782_EEPROM_VELO_AE3040Slope_VALUE,
	.ae2030slope = DS2782_EEPROM_VELO_AE2030Slope_VALUE,
	.ae1020slope = DS2782_EEPROM_VELO_AE1020Slope_VALUE,
	.ae0010slope = DS2782_EEPROM_VELO_AE0010Slope_VALUE,
	.se3040slope = DS2782_EEPROM_VELO_SE3040Slope_VALUE,
	.se2030slope = DS2782_EEPROM_VELO_SE2030Slope_VALUE,
	.se1020slope = DS2782_EEPROM_VELO_SE1020Slope_VALUE,
	.se0010slope = DS2782_EEPROM_VELO_SE0010Slope_VALUE,
	.rsgain_msb  = DS2782_EEPROM_VELO_RSGAIN_MSB_VALUE,
	.rsgain_lsb = DS2782_EEPROM_VELO_RSGAIN_LSB_VALUE,
	.rstc = DS2782_EEPROM_VELO_RSTC_VALUE,
	.frsgain_msb = DS2782_EEPROM_VELO_FRSGAIN_MSB_VALUE,
	.frsgain_lsb = DS2782_EEPROM_VELO_FRSGAIN_LSB_VALUE,
	.slave_addr_config = DS2782_EEPROM_VELO_SlaveAddressConfig_VALUE,
};

static const struct twonav_ds_2782_eeprom_config ds_config_trail = {
	.control = DS2782_EEPROM_TRAIL_CONTROL_VALUE,
	.ab = DS2782_EEPROM_TRAIL_AB_VALUE,
	.ac_msb = DS2782_EEPROM_TRAIL_AC_MSB_VALUE,
	.ac_lsb = DS2782_EEPROM_TRAIL_AC_LSB_VALUE,
	.vchg = DS2782_EEPROM_TRAIL_VCHG_VALUE,
	.imin = DS2782_EEPROM_TRAIL_IMIN_VALUE,
	.vae = DS2782_EEPROM_TRAIL_VAE_VALUE,
	.iae = DS2782_EEPROM_TRAIL_IAE_VALUE,
	.active_empty = DS2782_EEPROM_TRAIL_ActiveEmpty_VALUE,
	.rsns = DS2782_EEPROM_TRAIL_RSNS_VALUE,
	.full40_msb = DS2782_EEPROM_TRAIL_Full40_MSB_VALUE,
	.full40_lsb = DS2782_EEPROM_TRAIL_Full40_LSB_VALUE,
	.full3040slope = DS2782_EEPROM_TRAIL_Full3040Slope_VALUE,
	.full2030slope = DS2782_EEPROM_TRAIL_Full2030Slope_VALUE,
	.full1020slope = DS2782_EEPROM_TRAIL_Full1020Slope_VALUE,
	.full0010slope = DS2782_EEPROM_TRAIL_Full0010Slope_VALUE,
	.ae3040slope = DS2782_EEPROM_TRAIL_AE3040Slope_VALUE,
	.ae2030slope = DS2782_EEPROM_TRAIL_AE2030Slope_VALUE,
	.ae1020slope = DS2782_EEPROM_TRAIL_AE1020Slope_VALUE,
	.ae0010slope = DS2782_EEPROM_TRAIL_AE0010Slope_VALUE,
	.se3040slope = DS2782_EEPROM_TRAIL_SE3040Slope_VALUE,
	.se2030slope = DS2782_EEPROM_TRAIL_SE2030Slope_VALUE,
	.se1020slope = DS2782_EEPROM_TRAIL_SE1020Slope_VALUE,
	.se0010slope = DS2782_EEPROM_TRAIL_SE0010Slope_VALUE,
	.rsgain_msb  = DS2782_EEPROM_TRAIL_RSGAIN_MSB_VALUE,
	.rsgain_lsb = DS2782_EEPROM_TRAIL_RSGAIN_LSB_VALUE,
	.rstc = DS2782_EEPROM_TRAIL_RSTC_VALUE,
	.frsgain_msb = DS2782_EEPROM_TRAIL_FRSGAIN_MSB_VALUE,
	.frsgain_lsb = DS2782_EEPROM_TRAIL_FRSGAIN_LSB_VALUE,
	.slave_addr_config = DS2782_EEPROM_TRAIL_SlaveAddressConfig_VALUE,
};

static const struct twonav_ds_2782_eeprom_config ds_config_aventura = {
	.control = DS2782_EEPROM_AVENTURA_CONTROL_VALUE,
	.ab = DS2782_EEPROM_AVENTURA_AB_VALUE,
	.ac_msb = DS2782_EEPROM_AVENTURA_AC_MSB_VALUE,
	.ac_lsb = DS2782_EEPROM_AVENTURA_AC_LSB_VALUE,
	.vchg = DS2782_EEPROM_AVENTURA_VCHG_VALUE,
	.imin = DS2782_EEPROM_AVENTURA_IMIN_VALUE,
	.vae = DS2782_EEPROM_AVENTURA_VAE_VALUE,
	.iae = DS2782_EEPROM_AVENTURA_IAE_VALUE,
	.active_empty = DS2782_EEPROM_AVENTURA_ActiveEmpty_VALUE,
	.rsns = DS2782_EEPROM_AVENTURA_RSNS_VALUE,
	.full40_msb = DS2782_EEPROM_AVENTURA_Full40_MSB_VALUE,
	.full40_lsb = DS2782_EEPROM_AVENTURA_Full40_LSB_VALUE,
	.full3040slope = DS2782_EEPROM_AVENTURA_Full3040Slope_VALUE,
	.full2030slope = DS2782_EEPROM_AVENTURA_Full2030Slope_VALUE,
	.full1020slope = DS2782_EEPROM_AVENTURA_Full1020Slope_VALUE,
	.full0010slope = DS2782_EEPROM_AVENTURA_Full0010Slope_VALUE,
	.ae3040slope = DS2782_EEPROM_AVENTURA_AE3040Slope_VALUE,
	.ae2030slope = DS2782_EEPROM_AVENTURA_AE2030Slope_VALUE,
	.ae1020slope = DS2782_EEPROM_AVENTURA_AE1020Slope_VALUE,
	.ae0010slope = DS2782_EEPROM_AVENTURA_AE0010Slope_VALUE,
	.se3040slope = DS2782_EEPROM_AVENTURA_SE3040Slope_VALUE,
	.se2030slope = DS2782_EEPROM_AVENTURA_SE2030Slope_VALUE,
	.se1020slope = DS2782_EEPROM_AVENTURA_SE1020Slope_VALUE,
	.se0010slope = DS2782_EEPROM_AVENTURA_SE0010Slope_VALUE,
	.rsgain_msb  = DS2782_EEPROM_AVENTURA_RSGAIN_MSB_VALUE,
	.rsgain_lsb = DS2782_EEPROM_AVENTURA_RSGAIN_LSB_VALUE,
	.rstc = DS2782_EEPROM_AVENTURA_RSTC_VALUE,
	.frsgain_msb = DS2782_EEPROM_AVENTURA_FRSGAIN_MSB_VALUE,
	.frsgain_lsb = DS2782_EEPROM_AVENTURA_FRSGAIN_LSB_VALUE,
	.slave_addr_config = DS2782_EEPROM_AVENTURA_SlaveAddressConfig_VALUE,
};

static const struct twonav_ds_2782_eeprom_config ds_config_horizon = {
	.control = DS2782_EEPROM_HORIZON_CONTROL_VALUE,
	.ab = DS2782_EEPROM_HORIZON_AB_VALUE,
	.ac_msb = DS2782_EEPROM_HORIZON_AC_MSB_VALUE,
	.ac_lsb = DS2782_EEPROM_HORIZON_AC_LSB_VALUE,
	.vchg = DS2782_EEPROM_HORIZON_VCHG_VALUE,
	.imin = DS2782_EEPROM_HORIZON_IMIN_VALUE,
	.vae = DS2782_EEPROM_HORIZON_VAE_VALUE,
	.iae = DS2782_EEPROM_HORIZON_IAE_VALUE,
	.active_empty = DS2782_EEPROM_HORIZON_ActiveEmpty_VALUE,
	.rsns = DS2782_EEPROM_HORIZON_RSNS_VALUE,
	.full40_msb = DS2782_EEPROM_HORIZON_Full40_MSB_VALUE,
	.full40_lsb = DS2782_EEPROM_HORIZON_Full40_LSB_VALUE,
	.full3040slope = DS2782_EEPROM_HORIZON_Full3040Slope_VALUE,
	.full2030slope = DS2782_EEPROM_HORIZON_Full2030Slope_VALUE,
	.full1020slope = DS2782_EEPROM_HORIZON_Full1020Slope_VALUE,
	.full0010slope = DS2782_EEPROM_HORIZON_Full0010Slope_VALUE,
	.ae3040slope = DS2782_EEPROM_HORIZON_AE3040Slope_VALUE,
	.ae2030slope = DS2782_EEPROM_HORIZON_AE2030Slope_VALUE,
	.ae1020slope = DS2782_EEPROM_HORIZON_AE1020Slope_VALUE,
	.ae0010slope = DS2782_EEPROM_HORIZON_AE0010Slope_VALUE,
	.se3040slope = DS2782_EEPROM_HORIZON_SE3040Slope_VALUE,
	.se2030slope = DS2782_EEPROM_HORIZON_SE2030Slope_VALUE,
	.se1020slope = DS2782_EEPROM_HORIZON_SE1020Slope_VALUE,
	.se0010slope = DS2782_EEPROM_HORIZON_SE0010Slope_VALUE,
	.rsgain_msb  = DS2782_EEPROM_HORIZON_RSGAIN_MSB_VALUE,
	.rsgain_lsb = DS2782_EEPROM_HORIZON_RSGAIN_LSB_VALUE,
	.rstc = DS2782_EEPROM_HORIZON_RSTC_VALUE,
	.frsgain_msb = DS2782_EEPROM_HORIZON_FRSGAIN_MSB_VALUE,
	.frsgain_lsb = DS2782_EEPROM_HORIZON_FRSGAIN_LSB_VALUE,
	.slave_addr_config = DS2782_EEPROM_HORIZON_SlaveAddressConfig_VALUE,
};

#define DS2782_AS_VALUE 						0x80 //0x14
#define DS2782_Register_Command_Write_Copy_VALUE 			0x44 //0xFE
#define DS2782_Register_Command_Recal_Read_VALUE 			0xb4 //0xFE

#define DS2786_CURRENT_UNITS	25

struct ds278x_info;

struct ds278x_battery_ops {
	int (*get_battery_current)(struct ds278x_info *info, int *current_uA);
	int (*get_battery_voltage)(struct ds278x_info *info, int *voltage_uV);
	int (*get_battery_capacity)(struct ds278x_info *info, int *capacity);
	int (*get_battery_acr)(struct ds278x_info *info, int *acr);
	int (*get_battery_raac)(struct ds278x_info *info, int *raac);
	int (*get_battery_rsac)(struct ds278x_info *info, int *rsac);
	int (*get_battery_rarc)(struct ds278x_info *info, int *rarc);
	int (*get_battery_rsrc)(struct ds278x_info *info, int *rsrc);
	int (*get_battery_status)(struct ds278x_info *info, int *new_batt);
	int (*get_battery_rsns)(struct ds278x_info *info, int *rsns);
	int (*get_battery_learning)(struct ds278x_info *info, int *learning);
	int (*get_battery_charge_full)(struct ds278x_info *info, int *full);
};

#define to_ds278x_info(x) container_of(x, struct ds278x_info, battery)

struct ds278x_info {
	struct i2c_client	*client;
	struct power_supply	battery;
	struct ds278x_battery_ops  *ops;
	int id;
	int rsns;
	int gpio_enable_charger;
	int battery_status;
	int gpio_charge_manager_pg;
	int gpio_charge_manager_stat1;
	int gpio_charge_manager_stat2;
};

static DEFINE_IDR(battery_id);
static DEFINE_MUTEX(battery_lock);

static inline int ds278x_read_reg(struct ds278x_info *info, int reg, u8 *val)
{
	int ret;

	ret = i2c_smbus_read_byte_data(info->client, reg);
	if (ret < 0) {
		dev_err(&info->client->dev, "register read failed\n");
		return ret;
	}

	*val = ret;
	return 0;
}

static inline int ds278x_read_reg16(struct ds278x_info *info, int reg_msb,
				    s16 *val)
{
	int ret;

	ret = i2c_smbus_read_word_data(info->client, reg_msb);
	if (ret < 0) {
		dev_err(&info->client->dev, "register read failed\n");
		return ret;
	}

	*val = swab16(ret);
	return 0;
}

static int ds278x_get_temp(struct ds278x_info *info, int *temp)
{
	s16 raw;
	int err;

	/*
	 * Temperature is measured in units of 0.125 degrees celcius, the
	 * power_supply class measures temperature in tenths of degrees
	 * celsius. The temperature value is stored as a 10 bit number, plus
	 * sign in the upper bits of a 16 bit register.
	 */
	err = ds278x_read_reg16(info, DS278x_REG_TEMP_MSB, &raw);
	if (err)
		return err;
	*temp = ((raw / 32) * 125) / 100;
	return 0;
}

static int ds2782_get_current(struct ds278x_info *info, int *current_uA)
{
	int sense_res;
	int err;
	u8 sense_res_raw;
	s16 raw;

	/*
	 * The units of measurement for current are dependent on the value of
	 * the sense resistor.
	 */
	err = ds278x_read_reg(info, DS2782_REG_RSNSP, &sense_res_raw);
	if (err)
		return err;
	if (sense_res_raw == 0) {
		dev_err(&info->client->dev, "sense resistor value is 0\n");
		return -ENXIO;
	}
	sense_res = 1000 / sense_res_raw;

	dev_dbg(&info->client->dev, "sense resistor = %d milli-ohms\n",
		sense_res);
	err = ds278x_read_reg16(info, DS278x_REG_CURRENT_MSB, &raw);
	if (err)
		return err;
	*current_uA = raw * (DS2782_CURRENT_UNITS / sense_res);
	return 0;
}

static int ds2782_raw_voltage_to_uV(const s16 raw) {
	return (raw / 32) * 4880;
}

static int ds2782_get_voltage(struct ds278x_info *info, int *voltage_uV)
{
	s16 raw;
	int err;

	/*
	 * Voltage is measured in units of 4.88mV. The voltage is stored as
	 * a 10-bit number plus sign, in the upper bits of a 16-bit register
	 */
	err = ds278x_read_reg16(info, DS278x_REG_VOLT_MSB, &raw);
	if (err)
		return err;

	*voltage_uV = ds2782_raw_voltage_to_uV(raw);
	return 0;
}

static int ds2782_filter_AA_capacity_measurement(int capacity) {
	if (AA_consecutive_equal_capacity_measurements == -1) {
		AA_stable_capacity_value = capacity;
		AA_consecutive_equal_capacity_measurements = 0;
	}

	if (capacity == AA_stable_capacity_value) {
		AA_consecutive_equal_capacity_measurements = AA_consecutive_equal_capacity_measurements + 1;
	}
	else {
		AA_consecutive_equal_capacity_measurements = AA_consecutive_equal_capacity_measurements - 1;
	}

	if (AA_consecutive_equal_capacity_measurements >= AA_CAPACITY_EQUAL_MEASUREMENTS) {
		AA_consecutive_equal_capacity_measurements = AA_CAPACITY_EQUAL_MEASUREMENTS;
	}
	else if (AA_consecutive_equal_capacity_measurements <= 0) {
		AA_stable_capacity_value = capacity;
		AA_consecutive_equal_capacity_measurements = AA_CAPACITY_EQUAL_MEASUREMENTS;
	}

	return 0;
}

static int ds2782_update_Alkaline_capacity(int voltage_now, int current_now)
{
	int capacity;

	if (current_now >= -185000) {
		if (voltage_now <= 4310000)
			capacity = 100;
		else if (voltage_now <= 4322500)
			capacity = 25;
		else
			capacity = 0;
	}
	else if (current_now >= -210000){
		if (voltage_now <= 4303000)
			capacity = 100;
		else if (voltage_now <= 4313750)
			capacity = 25;
		else
			capacity = 0;
	}
	else {
		if (voltage_now <= 4295000)
			capacity = 100;
		else if (voltage_now <= 4305000)
			capacity = 25;
		else
			capacity = 0;
	}

	ds2782_filter_AA_capacity_measurement(capacity);
	return 0;
}

static int ds2782_update_Lithium_capacity(int voltage_now, int current_now)
{
	int capacity;
	if (current_now >= -185000) {
		if (voltage_now <= 4285000)
			capacity = 100;
		else if (voltage_now <= 4290000)
			capacity = 25;
		else
			capacity = 0;
	}
	else if (current_now >= -210000){
		if (voltage_now <= 427500)
			capacity = 100;
		else if (voltage_now <= 4285000)
			capacity = 25;
		else
			capacity = 0;
	}
	else {
		if (voltage_now <= 4270000)
			capacity = 100;
		else if (voltage_now <= 4280000)
			capacity = 25;
		else
			capacity = 0;
	}

 	ds2782_filter_AA_capacity_measurement(capacity);
 	return 0;
}

static int ds2782_update_NiMH_capacity(int voltage_now, int current_now)
{
	int capacity;
	if (current_now >= -185000) {
		if (voltage_now <= 4310000)
			capacity = 100;
		else if (voltage_now <= 4340000)
			capacity = 25;
		else
			capacity = 0;
	}
	else if (current_now >= -210000){
		if (voltage_now <= 4298750)
			capacity = 100;
		else if (voltage_now <= 4320000)
			capacity = 25;
		else
			capacity = 0;
	}
	else {
		if (voltage_now <= 4287500)
			capacity = 100;
		else if (voltage_now <= 4300000)
			capacity = 25;
		else
			capacity = 0;
	}

 	ds2782_filter_AA_capacity_measurement(capacity);
 	return 0;
}

static int ds2782_calculate_AA_weighted_voltage_average(int voltage_average, const u8 battery_chemistry) {
	int sum_lower;
	int sum_higher;
	int n_lower;
	int n_higher;
	int weight_lower;
	int weight_higher;
	int weighted_average;
	struct list_head *i;

	sum_lower = 0;
	sum_higher = 0;
	n_lower = 0;
	n_higher = 0;
	list_for_each(i, &AA_voltage_queue->list) {
		struct queue_item* q_item = list_entry(i, struct queue_item, list);
		int value = *(int*)q_item->item;
		if (value >= voltage_average) {
			sum_higher = sum_higher + value;
			n_higher = n_higher + 1;
		}
		else {
			sum_lower = sum_lower + value;
			n_lower = n_lower + 1;
		}
	}

	if (battery_chemistry == Alkaline || battery_chemistry == NiMH){
		weight_lower = 2;
		weight_higher = 8;
	} else if (battery_chemistry == Lithium) {
		weight_lower = 4;
		weight_higher = 5;
	} else {
		weight_lower = 3;
		weight_higher = 7;
	}
	weighted_average = (weight_lower * sum_lower + weight_higher * sum_higher) /
			(weight_lower * n_lower + weight_higher * n_higher);

	return weighted_average;
}

static int ds2782_dequeueVoltage (queue *AA_voltage_queue) {
	int val;
	int* removed_item = queue_dequeue(AA_voltage_queue);
	if (removed_item) {
		val = *removed_item;
	}
	else {
		val = 0;
	}
	kfree(removed_item);

	return val;
}

static int ds2782_enqueueVoltage (queue *AA_voltage_queue, int voltage_now) {
	int removed_val;
	int* newVal = kmalloc(sizeof(int), GFP_ATOMIC);
	*newVal = voltage_now;

	if (AA_voltage_queue->current_size >= AA_VOLTAGE_FILTER_SIZE) {
		ds2782_dequeueVoltage(AA_voltage_queue);
	}

	queue_enqueue(AA_voltage_queue, newVal);

	return removed_val;
}

static int ds2782_update_AA_capacity(int voltage_now, int current_now, u8 battery_chemistry) {
	int voltage_average;
	int weighted_average;	
	int removed_voltage;

	if (voltage_now < 4200000) {
		printk(KERN_ALERT "DS2782: AA batteries with V < 4.2V :%d\n",voltage_now);
		return -1;
	}

	removed_voltage = ds2782_enqueueVoltage(AA_voltage_queue, voltage_now);
	AA_voltage_sum = AA_voltage_sum + voltage_now - removed_voltage;
	voltage_average = AA_voltage_sum / AA_voltage_queue->current_size;
	weighted_average = ds2782_calculate_AA_weighted_voltage_average(voltage_average, battery_chemistry);

	if (battery_chemistry == Alkaline) {
		ds2782_update_Alkaline_capacity(weighted_average, current_now);
	}
	else if (battery_chemistry == Lithium) {
		ds2782_update_Lithium_capacity(weighted_average, current_now);
	}
	else if (battery_chemistry == NiMH) {
		ds2782_update_NiMH_capacity(weighted_average, current_now);
	}
	else {
		ds2782_update_Alkaline_capacity(weighted_average, current_now); // defualt
	}

	return 0;
}

static int ds2782_get_capacity(struct ds278x_info *info, int *capacity)
{
	int err;
	u8 raw;

	u8 battery_chemistry;
	ds278x_read_reg(info, DS2782_Register_Chemistry, &battery_chemistry);

	if (battery_chemistry != LionPoly) {
		*capacity = AA_stable_capacity_value;
		return 0;
	}
	else
		err = ds278x_read_reg(info, DS2782_REG_RARC, &raw);

	if (err)
		return err;
	*capacity = raw;

	/** In case of an overestimation of capacity %, 100% can be achieved
	 *  earlier but still current will be entering. So, if power is
	 *  connected and capacity reaches 100 but we are still charging
	 *  (charge_termination_flag=0) instead of showing 100% we will show 99
	 **/
	if(tn_is_horizon || tn_is_aventura || tn_is_trail) 
	{	
		if ((mcp73833_power_good == 1) &&
			(*capacity == 100) &&
			(charge_termination_flag == 0)) 
		{
	 		*capacity = 99;
		}
	}

	return 0;
}

static int ds2782_get_acr(struct ds278x_info *info, int *capacity)
{
	int err;
	s16 raw;
	err = ds278x_read_reg16(info, DS2782_ACR_MSB, &raw);
	if (err)
		return err;
	*capacity = raw;
	return 0;
}

static int ds2782_get_raac(struct ds278x_info *info, int *capacity)
{
	int err;
	s16 raw;
	err = ds278x_read_reg16(info, DS2782_REG_RAAC, &raw);
	if (err)
		return err;
	*capacity = raw;
	return 0;
}

static int ds2782_get_rsac(struct ds278x_info *info, int *capacity)
{
	int err;
	s16 raw;
	err = ds278x_read_reg16(info, DS2782_REG_RSAC, &raw);
	if (err)
		return err;
	*capacity = raw;
	return 0;
}

static int ds2782_get_rarc(struct ds278x_info *info, int *capacity)
{
	int err;
	u8 raw;
	err = ds278x_read_reg(info, DS2782_REG_RARC, &raw);
	if (err)
		return err;
	*capacity = raw;
	return 0;
}

static int ds2782_get_rsrc(struct ds278x_info *info, int *capacity)
{
	int err;
	u8 raw;
	err = ds278x_read_reg(info, DS2782_REG_RSRC, &raw);
	if (err)
		return err;
	*capacity = raw;
	return 0;
}

static int ds2782_get_battery_status(struct ds278x_info *info, int *batt_status)
{
	*batt_status = info->battery_status;
	return 0;
}

static int ds2782_get_rsns(struct ds278x_info *info, int *rsns)
{
	int err;
	u8 sense_res_raw;
	err = ds278x_read_reg(info, DS2782_REG_RSNSP, &sense_res_raw);
	if (err)
		return err;
	if (sense_res_raw == 0) {
		dev_err(&info->client->dev, "sense resistor value is 0\n");
		return -ENXIO;
	}
	*rsns = 1000 / sense_res_raw;
	return 0;
}

static int ds2782_get_learning(struct ds278x_info *info, int *_learning)
{
	*_learning = learning;
	return 0;
}

static int ds2782_get_charge_full(struct ds278x_info *info, int *_full)
{
	*_full = fully_charged;
	return 0;
}

static int ds2786_get_current(struct ds278x_info *info, int *current_uA)
{
	int err;
	s16 raw;

	err = ds278x_read_reg16(info, DS278x_REG_CURRENT_MSB, &raw);
	if (err)
		return err;
	*current_uA = (raw / 16) * (DS2786_CURRENT_UNITS / info->rsns);
	return 0;
}

static int ds2786_get_voltage(struct ds278x_info *info, int *voltage_uV)
{
	s16 raw;
	int err;

	/*
	 * Voltage is measured in units of 1.22mV. The voltage is stored as
	 * a 10-bit number plus sign, in the upper bits of a 16-bit register
	 */
	err = ds278x_read_reg16(info, DS278x_REG_VOLT_MSB, &raw);
	if (err)
		return err;

	*voltage_uV = (raw / 8) * 1220;
	return 0;
}

static int ds2786_get_capacity(struct ds278x_info *info, int *capacity)
{
	int err;
	u8 raw;

	err = ds278x_read_reg(info, DS2786_REG_RARC, &raw);
	if (err)
		return err;
	/* Relative capacity is displayed with resolution 0.5 % */
	*capacity = raw/2 ;
	return 0;
}

static int ds278x_get_status(struct ds278x_info *info, int *status)
{
	int err;
	int current_uA;
	int capacity;

	err = info->ops->get_battery_current(info, &current_uA);
	if (err)
		return err;

	err = info->ops->get_battery_capacity(info, &capacity);
	if (err)
		return err;

	if (capacity == 100)
		*status = POWER_SUPPLY_STATUS_FULL;
	else if (current_uA == 0)
		*status = POWER_SUPPLY_STATUS_NOT_CHARGING;
	else if (current_uA < 0)
		*status = POWER_SUPPLY_STATUS_DISCHARGING;
	else
		*status = POWER_SUPPLY_STATUS_CHARGING;

	return 0;
}

static int ds278x_battery_get_property(struct power_supply *psy,
				       enum power_supply_property prop,
				       union power_supply_propval *val)
{
	struct ds278x_info *info = to_ds278x_info(psy);
	int ret;

	switch (prop) {
	case POWER_SUPPLY_PROP_STATUS:
		ret = ds278x_get_status(info, &val->intval);
		break;

	case POWER_SUPPLY_PROP_CAPACITY:
		ret = info->ops->get_battery_capacity(info, &val->intval);
		break;

	case POWER_SUPPLY_PROP_CAPACITY_ACR:
		ret = info->ops->get_battery_acr(info, &val->intval);
		break;

	case POWER_SUPPLY_PROP_CAPACITY_RAAC:
		ret = info->ops->get_battery_raac(info, &val->intval);
		break;

	case POWER_SUPPLY_PROP_CAPACITY_RSAC:
		ret = info->ops->get_battery_rsac(info, &val->intval);
		break;

	case POWER_SUPPLY_PROP_CAPACITY_RARC:
		ret = info->ops->get_battery_rarc(info, &val->intval);
		break;

	case POWER_SUPPLY_PROP_CAPACITY_RSRC:
		ret = info->ops->get_battery_rsrc(info, &val->intval);
		break;

	case POWER_SUPPLY_PROP_BATTERY_STATUS:
		ret = info->ops->get_battery_status(info, &val->intval);
		break;

	case POWER_SUPPLY_PROP_RSNS:
			ret = info->ops->get_battery_rsns(info, &val->intval);
			break;

	case POWER_SUPPLY_PROP_LEARNING:
			ret = info->ops->get_battery_learning(info, &val->intval);
			break;

	case POWER_SUPPLY_PROP_CHARGE_FULL:
				ret = info->ops->get_battery_charge_full(info, &val->intval);
				break;

	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		ret = info->ops->get_battery_voltage(info, &val->intval);
		break;

	case POWER_SUPPLY_PROP_CURRENT_NOW:
		ret = info->ops->get_battery_current(info, &val->intval);
		break;

	case POWER_SUPPLY_PROP_TEMP:
		ret = ds278x_get_temp(info, &val->intval);
		break;

	default:
		ret = -EINVAL;
	}

	return ret;
}

static enum power_supply_property ds278x_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CAPACITY_ACR,
	POWER_SUPPLY_PROP_CAPACITY_RAAC,
	POWER_SUPPLY_PROP_CAPACITY_RSAC,
	POWER_SUPPLY_PROP_CAPACITY_RARC,
	POWER_SUPPLY_PROP_CAPACITY_RSRC,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_RSNS,
	POWER_SUPPLY_PROP_BATTERY_STATUS,
	POWER_SUPPLY_PROP_LEARNING,
};

static void ds278x_power_supply_init(struct power_supply *battery)
{
	battery->type			= POWER_SUPPLY_TYPE_BATTERY;
	battery->properties		= ds278x_battery_props;
	battery->num_properties		= ARRAY_SIZE(ds278x_battery_props);
	battery->get_property		= ds278x_battery_get_property;
	battery->external_power_changed	= NULL;
}

enum ds278x_num_id {
	DS2782 = 0,
	DS2786,
};

static struct ds278x_battery_ops ds278x_ops[] = {
	[DS2782] = {
		.get_battery_current  = ds2782_get_current,
		.get_battery_voltage  = ds2782_get_voltage,
		.get_battery_capacity = ds2782_get_capacity,
		.get_battery_acr      = ds2782_get_acr,
		.get_battery_raac	  = ds2782_get_raac,
		.get_battery_rsac	  = ds2782_get_rsac,
		.get_battery_rarc	  = ds2782_get_rarc,
		.get_battery_rsrc	  = ds2782_get_rsrc,
		.get_battery_status   = ds2782_get_battery_status,
		.get_battery_rsns     = ds2782_get_rsns,
		.get_battery_learning = ds2782_get_learning,
		.get_battery_charge_full 	  = ds2782_get_charge_full,
	},
	[DS2786] = {
		.get_battery_current  = ds2786_get_current,
		.get_battery_voltage  = ds2786_get_voltage,
		.get_battery_capacity = ds2786_get_capacity,
		.get_battery_acr      = ds2782_get_acr,
	}
};

static int ds2782_detect_battery_status(struct i2c_client *client)
{
	/** To detect a newly inserted battery we take advantage of the following behavior:
	 *  Registers 0x20-0x37 are cleared to 0x00 when a battery is removed. So once we
	 *  configure the chip we set register 0x20 to a value. If the value becomes 0x00
	 *  we know that the battery has been removed.
	 * Once battery is configured it passes from state NEW_BATTERY(0 or 0x00) to NEW_BATTERY_CONFIGURED
	 * If the battery is removed DS2782_Register_LearnComplete bit is cleared to 0 (==NEW_BATTERY)
	 * So once a battery is configured it will always have this bit set to a value > 0.
	 */
	int battery_status = i2c_smbus_read_byte_data(client, DS2782_Register_LearnComplete);
	return battery_status;
}

static void ds2782_autodetect_battery_type(struct i2c_client *client) {
	s16 raw;
	int voltage;
	int chemistry;
	int err;
	err = i2c_smbus_read_word_data(client, DS278x_REG_VOLT_MSB);
	if (err < 0) {
		return;
	}

	raw = swab16(err);
	voltage = ds2782_raw_voltage_to_uV(raw);

	if (voltage > 4220000) {
		chemistry = Alkaline;
	} else {
		chemistry = LionPoly;
	}

	i2c_smbus_write_byte_data(client, DS2782_Register_Chemistry, chemistry);
}

static int ds2782_battery_init(struct i2c_client *client, int* batt_status)
{
	const struct twonav_ds_2782_eeprom_config* tn_config = NULL;
		
	if(tn_is_velo)
		tn_config = &ds_config_velo;
	else if(tn_is_horizon) 
		tn_config = &ds_config_horizon;
	else if(tn_is_trail) 
		tn_config = &ds_config_trail;
	else if(tn_is_aventura) 
	{			
		tn_config = &ds_config_aventura;
		ds2782_autodetect_battery_type(client);	
	}
	else {
		printk(KERN_INFO "ds2782_battery_init: error: unassigned twonav device");
		return -1;
	}
	
	// Values that need to be set regardless of previous configuration
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_IMIN[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_IMIN, (long unsigned int)tn_config->imin);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_IMIN, tn_config->imin); // 0x65 Charge termination current

	*batt_status = ds2782_detect_battery_status(client);
	if (*batt_status != NEW_BATTERY) { // No need to re-configure DS2782 again
		return 0;
	}

	// Values extracted from the DS2782K Test kit
	// IMPORTANT: Capacity estimation is done outside kernel

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_CONTROL[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_CONTROL, (long unsigned int)tn_config->control);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_CONTROL, tn_config->control); // 0x60
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AB, (long unsigned int)tn_config->ab);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AB, tn_config->ab); // 0x61

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AC_MSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AC_MSB, (long unsigned int)tn_config->ac_msb);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AC_MSB, tn_config->ac_msb); // 0x62
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AC_LSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AC_LSB, (long unsigned int)tn_config->ac_lsb);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AC_LSB, tn_config->ac_lsb); // 0x63

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_VCHG[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_VCHG, (long unsigned int)tn_config->vchg);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_VCHG, tn_config->vchg); //4.2 charging voltage// 0x64


	printk(KERN_INFO "I2C Write: DS2782_EEPROM_VAE[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_VAE, (long unsigned int)tn_config->vae);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_VAE, tn_config->vae); //3.0 minimum voltage// 0x66
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_IAE[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_IAE, (long unsigned int)tn_config->iae);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_IAE, tn_config->iae); //330mA constant discharge current// 0x67

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_ActiveEmpty[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_ActiveEmpty, (long unsigned int)tn_config->active_empty);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_ActiveEmpty, tn_config->active_empty); // 0x68
	printk(KERN_INFO "I2C Write: DS2782_REG_RSNSP[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_REG_RSNSP, (long unsigned int)tn_config->rsns);
	i2c_smbus_write_byte_data(client, DS2782_REG_RSNSP, tn_config->rsns); // 0x69

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full40_MSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full40_MSB, (long unsigned int)tn_config->full40_msb);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full40_MSB, tn_config->full40_msb); // 0x6A
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full40_LSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full40_LSB, (long unsigned int)tn_config->full40_lsb);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full40_LSB, tn_config->full40_lsb); // 0x6B

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full3040Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full3040Slope, (long unsigned int)tn_config->full3040slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full3040Slope, tn_config->full3040slope); // 0x6C
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full2030Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full2030Slope, (long unsigned int)tn_config->full2030slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full2030Slope, tn_config->full2030slope); // 0x6D
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full1020Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full1020Slope, (long unsigned int)tn_config->full1020slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full1020Slope, tn_config->full1020slope); // 0x6E

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full0010Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full0010Slope, (long unsigned int)tn_config->full0010slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full0010Slope, tn_config->full0010slope); // 0x6F
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AE3040Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AE3040Slope, (long unsigned int)tn_config->ae3040slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE3040Slope, tn_config->ae3040slope); // 0x70
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AE2030Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AE2030Slope, (long unsigned int)tn_config->ae2030slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE2030Slope, tn_config->ae2030slope); // 0x71
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AE1020Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AE1020Slope, (long unsigned int)tn_config->ae1020slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE1020Slope, tn_config->ae1020slope); // 0x72

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AE0010Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AE0010Slope, (long unsigned int)tn_config->ae0010slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE0010Slope, tn_config->ae0010slope); // 0x73

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_SE3040Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_SE3040Slope, (long unsigned int)tn_config->se3040slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE3040Slope, tn_config->se3040slope); // 0x74

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_SE2030Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_SE2030Slope, (long unsigned int)tn_config->se2030slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE2030Slope, tn_config->se2030slope); // 0x75
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_SE1020Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_SE1020Slope, (long unsigned int)tn_config->se1020slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE1020Slope, tn_config->se1020slope); // 0x76
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_SE0010Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_SE0010Slope, (long unsigned int)tn_config->se0010slope);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE0010Slope, tn_config->se0010slope); // 0x77
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_RSGAIN_MSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_RSGAIN_MSB, (long unsigned int)tn_config->rsgain_msb);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_RSGAIN_MSB, tn_config->rsgain_msb); // 0x78
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_RSGAIN_LSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_RSGAIN_LSB, (long unsigned int)tn_config->rsgain_lsb);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_RSGAIN_LSB, tn_config->rsgain_lsb); // 0x79
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_RSTC[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_RSTC, (long unsigned int)tn_config->rstc);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_RSTC, tn_config->rstc); // 0x7A
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_FRSGAIN_MSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_FRSGAIN_MSB, (long unsigned int)tn_config->frsgain_msb);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_FRSGAIN_MSB, tn_config->frsgain_msb); // 0x7B
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_FRSGAIN_LSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_FRSGAIN_LSB, (long unsigned int)tn_config->frsgain_lsb);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_FRSGAIN_LSB, tn_config->frsgain_lsb); // 0x7C
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_SlaveAddressConfig[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_SlaveAddressConfig, (long unsigned int)tn_config->slave_addr_config);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SlaveAddressConfig, tn_config->slave_addr_config); // 0x7E
	printk(KERN_INFO "I2C Write: DS2782_AS[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_AS, (long unsigned int)DS2782_AS_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_AS, DS2782_AS_VALUE); // 0x14 Aging Scalar

	printk(KERN_INFO "I2C Write: DS2782_Register_Command[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_Register_Command, (long unsigned int)DS2782_Register_Command_Write_Copy_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_Register_Command, DS2782_Register_Command_Write_Copy_VALUE); // 0xFE

	//printk(KERN_INFO "I2C Write: DS2782_Register_Command[0x%04lx] = (0x%04lx)\n", DS2782_Register_Command, DS2782_Register_Command_Recal_Read_VALUE);
	//i2c_smbus_write_byte_data(client, DS2782_Register_Command, DS2782_Register_Command_Recal_Read_VALUE); // 0xFE

	i2c_smbus_write_byte_data(client, DS2782_Register_LearnComplete, NEW_BATTERY_CONFIGURED);
	*batt_status = NEW_BATTERY_CONFIGURED;

	return 0;
}

void set_battery_status(struct ds278x_info *info, int status)
{
	if (info->battery_status == LEARN_COMPLETE || info->battery_status == status) {
		return;
	}

	i2c_smbus_write_byte_data(info->client, DS2782_Register_LearnComplete, status);
	info->battery_status = status;
}

int check_learn_complete(struct ds278x_info *info)
{
	int capacity;
	int err;
	u8 raw;

	err = ds278x_read_reg(info, DS2782_REG_Status, &raw);
	if (err)
		return err;

	// DS2782 full charge flag stays on until capacity falls bellow 90%
	// so the battery is considered fully charged when capacity is 100% and
	// DS2782 full charge flag on
	charge_termination_flag = raw >> 7 & 0x01;
	info->ops->get_battery_capacity(info, &capacity);
	if ((charge_termination_flag == 1) && (capacity == 100)) {
		fully_charged = 1;
	}
	else {
		fully_charged = 0;
	}

	if (charge_termination_flag == 1)
	{
		set_battery_status(info, FULL_CHARGE_DETECTED);
	}

	learning = raw >> 4 & 0x01;
	if (learning && charge_termination_flag) {
		set_battery_status(info, LEARN_COMPLETE);
	}

	return 0;
}

void enable_charger(int gpio)
{
	printk("MAX8814 enable charge\n");
	gpio_request_one(gpio, GPIOF_DIR_OUT, "MAX8814_EN");
	gpio_set_value(gpio,0);
	gpio_free(gpio);
	charger_enabled = 1;
}

void disable_charger(int gpio)
{
	printk("MAX8814 disable charge\n");
	gpio_request_one(gpio, GPIOF_DIR_OUT, "MAX8814_EN");
	gpio_set_value(gpio,1);
	gpio_free(gpio);
	charger_enabled = 0;
}

static int ds2782_reset_full_charge_flag(struct ds278x_info *info) {
	// Full charge flag is reset if % < 90% or if there is a write on the ACR register
	// In this case we write the same value to the MSB of the ACR register
	int err;
	u8 raw;
	err = ds278x_read_reg(info, DS2782_ACR_MSB, &raw);
	if (err) {
		return err;
	}

	i2c_smbus_write_byte_data(info->client, DS2782_ACR_MSB, raw);
	return 0;
}

void max8814_reset_charger(struct ds278x_info *info) {
	printk("MAX8814 reset charger\n");
	gpio_request_one(info->gpio_enable_charger, GPIOF_DIR_OUT, "MAX8814_EN");
	gpio_set_value(info->gpio_enable_charger,1);
	msleep(MAX8814_RESET_WAIT_TIME); // without this sleep reset is not triggered
	gpio_set_value(info->gpio_enable_charger,0);
	gpio_free(info->gpio_enable_charger);
}

int mcp73833_get_time_diff(struct timespec *start_time, struct timespec *time_now) {
	int diff;
	getrawmonotonic(time_now);
	diff = time_now->tv_sec - start_time->tv_sec;
	return diff;
}

void mcp73833_check_power_good(struct ds278x_info *info) {

	mcp73833_power_good_previous_value = mcp73833_power_good;
	mcp73833_power_good = gpio_get_value(info->gpio_charge_manager_pg);

	if (mcp73833_power_good != mcp73833_power_good_previous_value) {
		if (mcp73833_power_good) {
			// Cable plugged - init charging timer
			getrawmonotonic(&charging_time_start);
		}
		else {
			// Cable unplugged - reset charger variables
			mcp73833_end_of_charge = 0;
			mcp73833_charging = -1;
			mcp73833_charged = -1;
			mcp73833_n_stat1_stable_values = 0;
			mcp73833_n_stat2_stable_values = 0;
		}
	}
}

void mcp73833_reset_charger_timer_every_3_hours(struct ds278x_info *info, int current_uA) {
	if (mcp73833_power_good) {
		if ((charger_enabled == 1) &&
			(current_uA > 0) &&
			(mcp73833_end_of_charge == 0))
		{
			struct timespec time_now;
			int diff = mcp73833_get_time_diff(&charging_time_start, &time_now);
			if (diff >= CHARGING_TIMER_THRESHOLD) {
				printk(KERN_INFO "MAX8814 reset charger every 3 hours\n");
				max8814_reset_charger(info);
				charging_time_start = time_now;
			}
		}
	}
}

static void mcp73833_end_of_charge_reset(struct ds278x_info *info, int current_uA) {
	// MCP73833 recharge threshold is 94% of Vreg(4.2V/4.35V)=3.948V/4.089V, which corresponds to a
	// capacity % less than 85%.
	// On reaching EOC if there is a residual current (Aventura/Trail: -5mA), which leads to slow battery drain,
	// reset charger after 12hrs in this state in order to compensate
	if ( (mcp73833_power_good == 1) &&
		 (charge_termination_flag == 1) &&
		 (mcp73833_end_of_charge == 1))
	{
		struct timespec time_now;
		int diff = mcp73833_get_time_diff(&eoc_time_start, &time_now);
		if (diff >= EOC_TIMER_THRESHOLD) {
			if (current_uA < 0 && charger_enabled == 1) {
				printk(KERN_INFO "MAX8814 reset due to EOC timer\n");
				max8814_reset_charger(info);
				ds2782_reset_full_charge_flag(info);
			}
			eoc_time_start = time_now;
		}
	}
}

int mcp73833_read_stat1(struct ds278x_info *info)
{
	int stable_value = 0;
	int stat1 = gpio_get_value(info->gpio_charge_manager_stat1);

	if (mcp73833_charging == -1) {
		mcp73833_charging = stat1;
	}

	if (stat1 == mcp73833_charging) {
		mcp73833_n_stat1_stable_values = mcp73833_n_stat1_stable_values + 1;
		if (mcp73833_n_stat1_stable_values >= N_STABLE_STAT_VALUES) {
			mcp73833_n_stat1_stable_values = N_STABLE_STAT_VALUES;
			stable_value = 1;
		}
	}
	else {
		mcp73833_n_stat1_stable_values = mcp73833_n_stat1_stable_values - 1;
		if (mcp73833_n_stat1_stable_values < 0) {
			mcp73833_n_stat1_stable_values = 0;
			mcp73833_charging = stat1;
		}
	}

	return stable_value;
}

int mcp73833_read_stat2(struct ds278x_info *info)
{
	int stable_value = 0;
	int stat2 = gpio_get_value(info->gpio_charge_manager_stat2);

	if (mcp73833_charged == -1) {
		mcp73833_charged = stat2;
	}

	if (stat2 == mcp73833_charged) {
		mcp73833_n_stat2_stable_values = mcp73833_n_stat2_stable_values + 1;
		if (mcp73833_n_stat2_stable_values >= N_STABLE_STAT_VALUES) {
			mcp73833_n_stat2_stable_values = N_STABLE_STAT_VALUES;
			stable_value = 1;
		}
	}
	else {
		mcp73833_n_stat2_stable_values = mcp73833_n_stat2_stable_values - 1;
		if (mcp73833_n_stat2_stable_values < 0) {
			mcp73833_n_stat2_stable_values = 0;
			mcp73833_charged = stat2;
		}
	}

	return stable_value;
}

void mcp73833_send_end_of_charge_event(struct ds278x_info *info) {
	char *envp[2];
	envp[0] = "EVENT=endofcharge";
	envp[1] = NULL;
	kobject_uevent_env(&(info->client->dev.kobj),KOBJ_CHANGE, envp);
}

int mcp73833_time_to_check_eoc() {
	static int ttc = 0;
	if (ttc != 0) {
		if (ttc == EOC_PERIOD_CHECK)
			ttc = 0;
		else {
			ttc = ttc +1;
		}
		return 0;
	}
	ttc = ttc +1;
	return 1;
}

void mcp73833_send_fault_event(struct ds278x_info *info) {
	char *envp[2];
	envp[0] = "EVENT=mcp73833fault";
	envp[1] = NULL;
	kobject_uevent_env(&(info->client->dev.kobj),KOBJ_CHANGE, envp);
}

int mcp73833_detect_end_of_charge_transition(struct ds278x_info *info) {

	int stat1_stable;
	int stat2_stable; 
	int end_of_charge_previous_value;
	int is_time_to_check_eoc;

	if (mcp73833_power_good == 0)  {
		return 0;
	}

	is_time_to_check_eoc = mcp73833_time_to_check_eoc();
	if (is_time_to_check_eoc == 0) {
		return 0;
	}

	stat1_stable = mcp73833_read_stat1(info);
	stat2_stable = mcp73833_read_stat2(info);

	if ((stat1_stable == 1) && (stat2_stable ==1)) {
		end_of_charge_previous_value = mcp73833_end_of_charge;
		mcp73833_end_of_charge = (mcp73833_charged == 1) && (mcp73833_charging == 0);
		if (end_of_charge_previous_value != mcp73833_end_of_charge) {
			if (mcp73833_end_of_charge == 1) {
				getrawmonotonic(&eoc_time_start);
				return 1;
			}
		}

		if ((mcp73833_charged == 0) && (mcp73833_charging == 0)) {
			// Advice user space because charger is in Standby / Timer Fault / Temperature Fault mode
			mcp73833_send_fault_event(info);
		}
	}
	return 0;
}


int check_if_discharge(struct ds278x_info *info)
{
	int err;
	int status;
	int current_uA;
	int capacity;
	int voltage;
	int eoc_transition;
	u8 battery_chemistry;

	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(HZ);

	err = ds278x_get_status(info, &status);
	if (err) {
		return err;
	}

	err = info->ops->get_battery_voltage(info, &voltage);
	if (err) {
		return err;
	}

	// Send sigterm signal to registered app when battery too low
	if (voltage < 2950000)
		send_sigterm(0);

	err = info->ops->get_battery_current(info, &current_uA);
	if (err) {
		return err;
	}

	if(tn_is_aventura) 
	{
		err = ds278x_read_reg(info, DS2782_Register_Chemistry, &battery_chemistry);
		if (err) {
			return err;
		}
		if (battery_chemistry != LionPoly) {
			if (AA_timer_count % AA_TIMER_SAMPLE == 0) {
				ds2782_update_AA_capacity(voltage, current_uA, battery_chemistry);
				AA_timer_count = 0;
			}
			AA_timer_count = AA_timer_count +1;
			return 0; // Exit if AAA batteries are installed
		}
	}

	err = info->ops->get_battery_capacity(info, &capacity);
	if (err)
		return err;

	if(tn_is_velo) 
	{
		if(status == POWER_SUPPLY_STATUS_FULL)
		{
			if(voltage > 4200000 && current_uA < 18000 && charger_enabled)
			{
				disable_charger(info->gpio_enable_charger);
			}
		}
		else
		{
			if((capacity <= RECHARGE_THRESHOLD) && (charger_enabled == 0))
			{
				enable_charger(info->gpio_enable_charger);
			}
		}
	}

	if(tn_is_aventura || tn_is_horizon || tn_is_trail) 
	{
		mcp73833_check_power_good(info);

		mcp73833_reset_charger_timer_every_3_hours(info, current_uA);

		eoc_transition = mcp73833_detect_end_of_charge_transition(info);
		if ((eoc_transition == 1) && (capacity < 100)) { // Send EOC message only if capacity is not 100
			mcp73833_send_end_of_charge_event(info);
		}

		mcp73833_end_of_charge_reset(info, current_uA);
	}

	// CHECK_LEARN_CYCLE_COMPLETE
	check_learn_complete(info);

	return 0;

}

int check_full_battery(void *info)
{
	while (!kthread_should_stop()){
		if(check_if_discharge(info) != 0)
			break;
	}
	return 0;
}

static ssize_t chemistry_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	int value;
	struct i2c_client * client = to_i2c_client(dev);
	value = i2c_smbus_read_byte_data(client, DS2782_Register_Chemistry);
    return sprintf(buf, "%d\n", value);;
}

static ssize_t chemistry_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	int err;
	int value;
	struct i2c_client * client = to_i2c_client(dev);
	err = kstrtoint(buf, 10, &value);
	if (err < 0)
	    return err;

	if ((value>=LionPoly) && (value <= Lithium))
		i2c_smbus_write_byte_data(client, DS2782_Register_Chemistry, value);
	else
		printk(KERN_INFO "ds2782: invalid battery chemistry\n");

	return count;
}

static DEVICE_ATTR(chemistry, S_IRUGO | S_IWUSR, chemistry_show,
		chemistry_store);

static int ds278x_battery_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct ds278x_platform_data *pdata = client->dev.platform_data;
	struct ds278x_info *info;
	int ret;
	int num;
	int batt_status;

	if(tn_hwtype != NULL)  printk("LDU: ds2782_battery_init for %s", tn_hwtype);

	/* Initialize battery registers if not set */
	ret = ds2782_battery_init(client, &batt_status);
	if (ret) {
		goto fail_register;
	}

	/*
	 * ds2786 should have the sense resistor value set
	 * in the platform data
	 */
	if (id->driver_data == DS2786 && !pdata) {
		dev_err(&client->dev, "missing platform data for ds2786\n");
		return -EINVAL;
	}

	/* Get an ID for this battery */
	ret = idr_pre_get(&battery_id, GFP_KERNEL);
	if (ret == 0) {
		ret = -ENOMEM;
		goto fail_id;
	}

	mutex_lock(&battery_lock);
	ret = idr_get_new(&battery_id, client, &num);
	mutex_unlock(&battery_lock);
	if (ret < 0)
		goto fail_id;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		ret = -ENOMEM;
		goto fail_info;
	}

	info->battery.name = kasprintf(GFP_KERNEL, "%s-%d", client->name, num);
	if (!info->battery.name) {
		ret = -ENOMEM;
		goto fail_name;
	}

	info->rsns = pdata->rsns;
	info->gpio_enable_charger = pdata->gpio_enable_charger;
	info->gpio_charge_manager_pg = pdata->gpio_pg;

	gpio_request_one(info->gpio_enable_charger, GPIOF_DIR_OUT, "MAX8814_EN");
	charger_enabled = !gpio_get_value(info->gpio_enable_charger);
	gpio_free(info->gpio_enable_charger);

	info->gpio_charge_manager_stat1 = pdata->gpio_stat1;
	info->gpio_charge_manager_stat2 = pdata->gpio_stat2;

	if(charger_enabled)
	{
		printk(KERN_INFO "Charger Enabled\n");
	}
	else
	{
		printk(KERN_INFO "Charger Disabled\n");
	}

	info->battery_status = batt_status;

	i2c_set_clientdata(client, info);
	info->client = client;
	info->id = num;
	info->ops  = &ds278x_ops[id->driver_data];
	ds278x_power_supply_init(&info->battery);

	ret = power_supply_register(&client->dev, &info->battery);
	if (ret) {
		dev_err(&client->dev, "failed to register battery\n");
		goto fail_register;
	}

	printk(KERN_INFO"--------------------------------------------\n");
	task = kthread_run(&check_full_battery,info,"ds2782-battery-holder");
	if (IS_ERR(task))
		return -1;
	printk(KERN_INFO"Kernel Thread : %s\n",task->comm);

	// Userspace interface to register pid for signal
	low_batt_signal_file = debugfs_create_file("signal_low_battery", 0200, NULL, NULL, &my_fops);

    // Register sysfs attribute chemistry
    device_create_file(dev, &dev_attr_chemistry);

	if(tn_is_aventura) 
	{
    	AA_voltage_queue = queue_create();
    }

	return 0;

fail_register:
	kfree(info->battery.name);
fail_name:
	kfree(info);
fail_info:
	mutex_lock(&battery_lock);
	idr_remove(&battery_id, num);
	mutex_unlock(&battery_lock);
fail_id:
	return ret;
}

static int ds278x_battery_remove(struct i2c_client *client)
{
	struct device * dev = &client->dev;
	struct ds278x_info *info = i2c_get_clientdata(client);

	kthread_stop(task);

	power_supply_unregister(&info->battery);
	kfree(info->battery.name);

	mutex_lock(&battery_lock);
	idr_remove(&battery_id, info->id);
	mutex_unlock(&battery_lock);

	kfree(info);
	debugfs_remove(low_batt_signal_file);
	device_remove_file(dev, &dev_attr_chemistry);
	
	if(tn_is_aventura)
	{
		queue_delete(AA_voltage_queue);
	}
	

	return 0;
}

static const struct i2c_device_id ds278x_id[] = {
	{"ds2782", DS2782},
	{"ds2786", DS2786},
	{},
};
MODULE_DEVICE_TABLE(i2c, ds278x_id);

static struct i2c_driver ds278x_battery_driver = {
	.driver 	= {
		.name	= "ds2782-battery",
	},
	.probe		= ds278x_battery_probe,
	.remove		= ds278x_battery_remove,
	.id_table	= ds278x_id,
};
module_i2c_driver(ds278x_battery_driver);

MODULE_AUTHOR("Ryan Mallon");
MODULE_DESCRIPTION("Maxim/Dallas DS2782 Stand-Alone Fuel Gauage IC driver");
MODULE_LICENSE("GPL");
