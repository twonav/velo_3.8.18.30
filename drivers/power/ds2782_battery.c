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

struct task_struct *task;
int charger_enabled = 0;
int learning = 0;

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

//DS2782 EEPROM values for TwoNav

#if defined (CONFIG_TWONAV_VELO)
	#define DS2782_EEPROM_CONTROL_VALUE 			0x00 //0x60
	#define DS2782_EEPROM_AB_VALUE 					0x00 //0x61
	#define DS2782_EEPROM_AC_MSB_VALUE 				0x14 //0x62
	#define DS2782_EEPROM_AC_LSB_VALUE 				0xA0 //0x63
	#define DS2782_EEPROM_VCHG_VALUE 				0xD7 //0x64
	#define DS2782_EEPROM_IMIN_VALUE 				0x08 //0x65
	#define DS2782_EEPROM_VAE_VALUE 				0x9A //0x66
	#define DS2782_EEPROM_IAE_VALUE 				0x21 //0x67
	#define DS2782_EEPROM_ActiveEmpty_VALUE 		0x00 //0x68
	#define DS2782_EEPROM_RSNS_VALUE 				0x20 //0x69
	#define DS2782_EEPROM_Full40_MSB_VALUE 			0x14 //0x6A
	#define DS2782_EEPROM_Full40_LSB_VALUE 			0xA0 //0x6B
	#define DS2782_EEPROM_Full3040Slope_VALUE 		0x00 //0x6C
	#define DS2782_EEPROM_Full2030Slope_VALUE 		0x00 //0x6D
	#define DS2782_EEPROM_Full1020Slope_VALUE 		0x4D //0x6E
	#define DS2782_EEPROM_Full0010Slope_VALUE 		0xF8 //0x6F
	#define DS2782_EEPROM_AE3040Slope_VALUE 		0x00 //0x70
	#define DS2782_EEPROM_AE2030Slope_VALUE 		0x00 //0x71
	#define DS2782_EEPROM_AE1020Slope_VALUE 		0x42 //0x72
	#define DS2782_EEPROM_AE0010Slope_VALUE 		0x12 //0x73
	#define DS2782_EEPROM_SE3040Slope_VALUE 		0x00 //0x74
	#define DS2782_EEPROM_SE2030Slope_VALUE 		0x00 //0x75
	#define DS2782_EEPROM_SE1020Slope_VALUE 		0x0B //0x76
	#define DS2782_EEPROM_SE0010Slope_VALUE 		0x0A //0x77
	#define DS2782_EEPROM_RSGAIN_MSB_VALUE 			0x04 //0x78
	#define DS2782_EEPROM_RSGAIN_LSB_VALUE 			0x00 //0x79
	#define DS2782_EEPROM_RSTC_VALUE 				0x00 //0x7A
	#define DS2782_EEPROM_FRSGAIN_MSB_VALUE 		0x04 //0x7B
	#define DS2782_EEPROM_FRSGAIN_LSB_VALUE 		0x1A //0x7C
	#define DS2782_EEPROM_SlaveAddressConfig_VALUE 	0x68 //0x7E
#elif defined (CONFIG_TWONAV_TRAIL)
	#define DS2782_EEPROM_CONTROL_VALUE 			0x00 //0x60
	#define DS2782_EEPROM_AB_VALUE 					0x00 //0x61
	#define DS2782_EEPROM_AC_MSB_VALUE 				0xXX //0x62
	#define DS2782_EEPROM_AC_LSB_VALUE 				0xXX //0x63
	#define DS2782_EEPROM_VCHG_VALUE 				0xXX //0x64
	#define DS2782_EEPROM_IMIN_VALUE 				0xXX //0x65
	#define DS2782_EEPROM_VAE_VALUE 				0xXX //0x66
	#define DS2782_EEPROM_IAE_VALUE 				0xXX //0x67
	#define DS2782_EEPROM_ActiveEmpty_VALUE 		0x00 //0x68
	#define DS2782_EEPROM_RSNS_VALUE 				0x20 //0x69
	#define DS2782_EEPROM_Full40_MSB_VALUE 			0xXX //0x6A
	#define DS2782_EEPROM_Full40_LSB_VALUE 			0xXX //0x6B
	#define DS2782_EEPROM_Full3040Slope_VALUE 		0x00 //0x6C
	#define DS2782_EEPROM_Full2030Slope_VALUE 		0x00 //0x6D
	#define DS2782_EEPROM_Full1020Slope_VALUE 		0x4D //0x6E
	#define DS2782_EEPROM_Full0010Slope_VALUE 		0xXX //0x6F
	#define DS2782_EEPROM_AE3040Slope_VALUE 		0xXX //0x70
	#define DS2782_EEPROM_AE2030Slope_VALUE 		0xXX //0x71
	#define DS2782_EEPROM_AE1020Slope_VALUE 		0xXX //0x72
	#define DS2782_EEPROM_AE0010Slope_VALUE 		0x12 //0x73
	#define DS2782_EEPROM_SE3040Slope_VALUE 		0x00 //0x74
	#define DS2782_EEPROM_SE2030Slope_VALUE 		0x00 //0x75
	#define DS2782_EEPROM_SE1020Slope_VALUE 		0x0B //0x76
	#define DS2782_EEPROM_SE0010Slope_VALUE 		0x0A //0x77
	#define DS2782_EEPROM_RSGAIN_MSB_VALUE 			0x04 //0x78
	#define DS2782_EEPROM_RSGAIN_LSB_VALUE 			0x00 //0x79
	#define DS2782_EEPROM_RSTC_VALUE 				0x00 //0x7A
	#define DS2782_EEPROM_FRSGAIN_MSB_VALUE 		0x04 //0x7B
	#define DS2782_EEPROM_FRSGAIN_LSB_VALUE 		0x1A //0x7C
	#define DS2782_EEPROM_SlaveAddressConfig_VALUE 	0x68 //0x7E
#elif defined (CONFIG_TWONAV_AVENTURA)
	#define DS2782_EEPROM_CONTROL_VALUE 			0x00 //0x60
	#define DS2782_EEPROM_AB_VALUE 					0x00 //0x61
	#define DS2782_EEPROM_AC_MSB_VALUE 				0x3E //0x62
	#define DS2782_EEPROM_AC_LSB_VALUE 				0x80 //0x63
	#define DS2782_EEPROM_VCHG_VALUE 				0xD7 //0x64
	#define DS2782_EEPROM_IMIN_VALUE 				0x14 //0x65
	#define DS2782_EEPROM_VAE_VALUE 				0x9A //0x66
	#define DS2782_EEPROM_IAE_VALUE 				0x0F //0x67
	#define DS2782_EEPROM_ActiveEmpty_VALUE 		0x00 //0x68
	#define DS2782_EEPROM_RSNS_VALUE 				0x20 //0x69
	#define DS2782_EEPROM_Full40_MSB_VALUE 			0x3E //0x6A
	#define DS2782_EEPROM_Full40_LSB_VALUE 			0x80 //0x6B
	#define DS2782_EEPROM_Full3040Slope_VALUE 		0x00 //0x6C
	#define DS2782_EEPROM_Full2030Slope_VALUE 		0x21 //0x6D
	#define DS2782_EEPROM_Full1020Slope_VALUE 		0x42 //0x6E
	#define DS2782_EEPROM_Full0010Slope_VALUE 		0xE5 //0x6F
	#define DS2782_EEPROM_AE3040Slope_VALUE 		0x00 //0x70
	#define DS2782_EEPROM_AE2030Slope_VALUE 		0x00 //0x71
	#define DS2782_EEPROM_AE1020Slope_VALUE 		0x0E //0x72
	#define DS2782_EEPROM_AE0010Slope_VALUE 		0x04 //0x73
	#define DS2782_EEPROM_SE3040Slope_VALUE 		0x00 //0x74
	#define DS2782_EEPROM_SE2030Slope_VALUE 		0x00 //0x75
	#define DS2782_EEPROM_SE1020Slope_VALUE 		0x02 //0x76
	#define DS2782_EEPROM_SE0010Slope_VALUE 		0x02 //0x77
	#define DS2782_EEPROM_RSGAIN_MSB_VALUE 			0x04 //0x78
	#define DS2782_EEPROM_RSGAIN_LSB_VALUE 			0x00 //0x79
	#define DS2782_EEPROM_RSTC_VALUE 				0x00 //0x7A
	#define DS2782_EEPROM_FRSGAIN_MSB_VALUE 		0x04 //0x7B
	#define DS2782_EEPROM_FRSGAIN_LSB_VALUE 		0x1A //0x7C
	#define DS2782_EEPROM_SlaveAddressConfig_VALUE 	0x68 //0x7E
#elif defined (CONFIG_TWONAV_HORIZON)
	#define DS2782_EEPROM_CONTROL_VALUE 			0x00 //0x60
	#define DS2782_EEPROM_AB_VALUE 					0x00 //0x61
	#define DS2782_EEPROM_AC_MSB_VALUE 				0x15 //0x62
	#define DS2782_EEPROM_AC_LSB_VALUE 				0x40 //0x63
	#define DS2782_EEPROM_VCHG_VALUE 				0xD7 //0x64
	#define DS2782_EEPROM_IMIN_VALUE 				0x08 //0x65
	#define DS2782_EEPROM_VAE_VALUE 				0x9A //0x66
	#define DS2782_EEPROM_IAE_VALUE 				0x1E //0x67
	#define DS2782_EEPROM_ActiveEmpty_VALUE 		0x00 //0x68
	#define DS2782_EEPROM_RSNS_VALUE 				0x20 //0x69
	#define DS2782_EEPROM_Full40_MSB_VALUE 			0x15 //0x6A
	#define DS2782_EEPROM_Full40_LSB_VALUE 			0x40 //0x6B
	#define DS2782_EEPROM_Full3040Slope_VALUE 		0x00 //0x6C
	#define DS2782_EEPROM_Full2030Slope_VALUE 		0x1D //0x6D
	#define DS2782_EEPROM_Full1020Slope_VALUE 		0x87 //0x6E
	#define DS2782_EEPROM_Full0010Slope_VALUE 		0xA4 //0x6F
	#define DS2782_EEPROM_AE3040Slope_VALUE 		0x00 //0x70
	#define DS2782_EEPROM_AE2030Slope_VALUE 		0x0C //0x71
	#define DS2782_EEPROM_AE1020Slope_VALUE 		0x0D //0x72
	#define DS2782_EEPROM_AE0010Slope_VALUE 		0x1B //0x73
	#define DS2782_EEPROM_SE3040Slope_VALUE 		0x00 //0x74
	#define DS2782_EEPROM_SE2030Slope_VALUE 		0x05 //0x75
	#define DS2782_EEPROM_SE1020Slope_VALUE 		0x05 //0x76
	#define DS2782_EEPROM_SE0010Slope_VALUE 		0x10 //0x77
	#define DS2782_EEPROM_RSGAIN_MSB_VALUE 			0x04 //0x78
	#define DS2782_EEPROM_RSGAIN_LSB_VALUE 			0x00 //0x79
	#define DS2782_EEPROM_RSTC_VALUE 				0x00 //0x7A
	#define DS2782_EEPROM_FRSGAIN_MSB_VALUE 		0x04 //0x7B
	#define DS2782_EEPROM_FRSGAIN_LSB_VALUE 		0x1A //0x7C
	#define DS2782_EEPROM_SlaveAddressConfig_VALUE 	0x68 //0x7E
#endif


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
	int (*get_battery_new)(struct ds278x_info *info, int *new_batt);
};

#define to_ds278x_info(x) container_of(x, struct ds278x_info, battery)

struct ds278x_info {
	struct i2c_client	*client;
	struct power_supply	battery;
	struct ds278x_battery_ops  *ops;
	int id;
	int rsns;
	int gpio;
	int new_battery;
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

	*voltage_uV = (raw / 32) * 4880;
	return 0;
}

static int ds2782_get_capacity(struct ds278x_info *info, int *capacity)
{
	int err;
	u8 raw;

	err = ds278x_read_reg(info, DS2782_REG_RARC, &raw);
	if (err)
		return err;
	*capacity = raw;
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

static int ds2782_get_new_battery(struct ds278x_info *info, int *new_batt)
{
	*new_batt = info->new_battery;
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

	case POWER_SUPPLY_PROP_NEW_BATTERY:
		ret = info->ops->get_battery_new(info, &val->intval);
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
	POWER_SUPPLY_PROP_NEW_BATTERY,
};

static void ds278x_power_supply_init(struct power_supply *battery)
{
	battery->type			= POWER_SUPPLY_TYPE_BATTERY;
	battery->properties		= ds278x_battery_props;
	battery->num_properties		= ARRAY_SIZE(ds278x_battery_props);
	battery->get_property		= ds278x_battery_get_property;
	battery->external_power_changed	= NULL;
}

static int ds278x_battery_remove(struct i2c_client *client)
{
	struct ds278x_info *info = i2c_get_clientdata(client);

   kthread_stop(task);

	power_supply_unregister(&info->battery);
	kfree(info->battery.name);

	mutex_lock(&battery_lock);
	idr_remove(&battery_id, info->id);
	mutex_unlock(&battery_lock);

	kfree(info);

	return 0;
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
		.get_battery_new      = ds2782_get_new_battery,
	},
	[DS2786] = {
		.get_battery_current  = ds2786_get_current,
		.get_battery_voltage  = ds2786_get_voltage,
		.get_battery_capacity = ds2786_get_capacity,
		.get_battery_acr      = ds2782_get_acr,
	}
};

static int ds2782_detect_new_battery(struct i2c_client *client)
{
	int r;
	r = i2c_smbus_read_byte_data(client, DS2782_REG_RSNSP);
	if (r > 0) {
		int learn_complete = i2c_smbus_read_byte_data(client, DS2782_Register_LearnComplete);
		if (learn_complete) {
			return 0;
		}
	}
	return 1;
}

/*
static int ds278x_battery_estimate_capacity_from_voltage(struct i2c_client *client)
{
	// We need to set registers ACR LSB y MSB. MSB must be set first
	u8 value;
	// 1. ACR (LSB MSB)
	value = 0x02;
	i2c_smbus_write_byte_data(client, DS2782_ACR_MSB, value);
	value = 0x00;
	i2c_smbus_write_byte_data(client, DS2782_ACR_LSB, value);
	// 4. AS
	value = 0x80;//0x79;
	i2c_smbus_write_byte_data(client, DS2782_AS, value);

	return 0;
}
*/

static int ds2782_battery_init(struct i2c_client *client, int* new_battery)
{
	*new_battery = ds2782_detect_new_battery(client);

	if (!new_battery)
		return 0;

	printk(KERN_INFO "NEW BATTERY\n");

	// Configure the IC only if new battery detected;
	// Values extracted from the DS2782K Test kit
	// IMPORTANT: First capacity estimation is done outside kernel

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_CONTROL[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_CONTROL, (long unsigned int)DS2782_EEPROM_CONTROL_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_CONTROL, DS2782_EEPROM_CONTROL_VALUE); // 0x60
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AB, (long unsigned int)DS2782_EEPROM_AB_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AB, DS2782_EEPROM_AB_VALUE); // 0x61

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AC_MSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AC_MSB, (long unsigned int)DS2782_EEPROM_AC_MSB_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AC_MSB, DS2782_EEPROM_AC_MSB_VALUE); // 0x62
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AC_LSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AC_LSB, (long unsigned int)DS2782_EEPROM_AC_LSB_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AC_LSB, DS2782_EEPROM_AC_LSB_VALUE); // 0x63

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_VCHG[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_VCHG, (long unsigned int)DS2782_EEPROM_VCHG_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_VCHG, DS2782_EEPROM_VCHG_VALUE); //4.2 charging voltage// 0x64
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_IMIN[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_IMIN, (long unsigned int)DS2782_EEPROM_IMIN_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_IMIN, DS2782_EEPROM_IMIN_VALUE); //16.5mA minimal charge current// 0x65

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_VAE[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_VAE, (long unsigned int)DS2782_EEPROM_VAE_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_VAE, DS2782_EEPROM_VAE_VALUE); //3.0 minimum voltage// 0x66
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_IAE[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_IAE, (long unsigned int)DS2782_EEPROM_IAE_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_IAE, DS2782_EEPROM_IAE_VALUE); //330mA constant discharge current// 0x67

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_ActiveEmpty[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_ActiveEmpty, (long unsigned int)DS2782_EEPROM_ActiveEmpty_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_ActiveEmpty, DS2782_EEPROM_ActiveEmpty_VALUE); // 0x68
	printk(KERN_INFO "I2C Write: DS2782_REG_RSNSP[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_REG_RSNSP, (long unsigned int)DS2782_EEPROM_RSNS_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_REG_RSNSP, DS2782_EEPROM_RSNS_VALUE); // 0x69

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full40_MSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full40_MSB, (long unsigned int)DS2782_EEPROM_Full40_MSB_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full40_MSB, DS2782_EEPROM_Full40_MSB_VALUE); // 0x6A
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full40_LSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full40_LSB, (long unsigned int)DS2782_EEPROM_Full40_LSB_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full40_LSB, DS2782_EEPROM_Full40_LSB_VALUE); // 0x6B

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full3040Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full3040Slope, (long unsigned int)DS2782_EEPROM_Full3040Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full3040Slope, DS2782_EEPROM_Full3040Slope_VALUE); // 0x6C
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full2030Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full2030Slope, (long unsigned int)DS2782_EEPROM_Full2030Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full2030Slope, DS2782_EEPROM_Full2030Slope_VALUE); // 0x6D
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full1020Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full1020Slope, (long unsigned int)DS2782_EEPROM_Full1020Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full1020Slope, DS2782_EEPROM_Full1020Slope_VALUE); // 0x6E

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_Full0010Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_Full0010Slope, (long unsigned int)DS2782_EEPROM_Full0010Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full0010Slope, DS2782_EEPROM_Full0010Slope_VALUE); // 0x6F
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AE3040Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AE3040Slope, (long unsigned int)DS2782_EEPROM_AE3040Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE3040Slope, DS2782_EEPROM_AE3040Slope_VALUE); // 0x70
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AE2030Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AE2030Slope, (long unsigned int)DS2782_EEPROM_AE2030Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE2030Slope, DS2782_EEPROM_AE2030Slope_VALUE); // 0x71
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AE1020Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AE1020Slope, (long unsigned int)DS2782_EEPROM_AE1020Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE1020Slope, DS2782_EEPROM_AE1020Slope_VALUE); // 0x72

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_AE0010Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_AE0010Slope, (long unsigned int)DS2782_EEPROM_AE0010Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE0010Slope, DS2782_EEPROM_AE0010Slope_VALUE); // 0x73

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_SE3040Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_SE3040Slope, (long unsigned int)DS2782_EEPROM_SE3040Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE3040Slope, DS2782_EEPROM_SE3040Slope_VALUE); // 0x74

	printk(KERN_INFO "I2C Write: DS2782_EEPROM_SE2030Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_SE2030Slope, (long unsigned int)DS2782_EEPROM_SE2030Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE2030Slope, DS2782_EEPROM_SE2030Slope_VALUE); // 0x75
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_SE1020Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_SE1020Slope, (long unsigned int)DS2782_EEPROM_SE1020Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE1020Slope, DS2782_EEPROM_SE1020Slope_VALUE); // 0x76
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_SE0010Slope[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_SE0010Slope, (long unsigned int)DS2782_EEPROM_SE0010Slope_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE0010Slope, DS2782_EEPROM_SE0010Slope_VALUE); // 0x77
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_RSGAIN_MSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_RSGAIN_MSB, (long unsigned int)DS2782_EEPROM_RSGAIN_MSB_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_RSGAIN_MSB, DS2782_EEPROM_RSGAIN_MSB_VALUE); // 0x78
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_RSGAIN_LSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_RSGAIN_LSB, (long unsigned int)DS2782_EEPROM_RSGAIN_LSB_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_RSGAIN_LSB, DS2782_EEPROM_RSGAIN_LSB_VALUE); // 0x79
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_RSTC[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_RSTC, (long unsigned int)DS2782_EEPROM_RSTC_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_RSTC, DS2782_EEPROM_RSTC_VALUE); // 0x7A
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_FRSGAIN_MSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_FRSGAIN_MSB, (long unsigned int)DS2782_EEPROM_FRSGAIN_MSB_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_FRSGAIN_MSB, DS2782_EEPROM_FRSGAIN_MSB_VALUE); // 0x7B
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_FRSGAIN_LSB[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_FRSGAIN_LSB, (long unsigned int)DS2782_EEPROM_FRSGAIN_LSB_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_FRSGAIN_LSB, DS2782_EEPROM_FRSGAIN_LSB_VALUE); // 0x7C
	printk(KERN_INFO "I2C Write: DS2782_EEPROM_SlaveAddressConfig[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_EEPROM_SlaveAddressConfig, (long unsigned int)DS2782_EEPROM_SlaveAddressConfig_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SlaveAddressConfig, DS2782_EEPROM_SlaveAddressConfig_VALUE); // 0x7E
	printk(KERN_INFO "I2C Write: DS2782_AS[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_AS, (long unsigned int)DS2782_AS_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_AS, DS2782_AS_VALUE); // 0x14 Aging Scalar

	printk(KERN_INFO "I2C Write: DS2782_Register_Command[0x%04lx] = (0x%04lx)\n", (long unsigned int)DS2782_Register_Command, (long unsigned int)DS2782_Register_Command_Write_Copy_VALUE);
	i2c_smbus_write_byte_data(client, DS2782_Register_Command, DS2782_Register_Command_Write_Copy_VALUE); // 0xFE

	//printk(KERN_INFO "I2C Write: DS2782_Register_Command[0x%04lx] = (0x%04lx)\n", DS2782_Register_Command, DS2782_Register_Command_Recal_Read_VALUE);
	//i2c_smbus_write_byte_data(client, DS2782_Register_Command, DS2782_Register_Command_Recal_Read_VALUE); // 0xFE

	return 0;
}

int check_learn_complete(struct ds278x_info *info)
{
	int learn_flag;
	int full_charge_flag;
	int active_empty_flag;
	int err;
	u8 raw;

	if (info->new_battery == 0)
		return 0;

	err = ds278x_read_reg(info, DS2782_REG_Status, &raw);
	if (err)
		return err;
	learn_flag = raw >> 4 & 0x01;
	full_charge_flag = raw >> 7 & 0x01;
	active_empty_flag = raw >> 6 & 0x01;
	/*
	printk(KERN_INFO "DS2782 learn flag: :%i\n", learn_flag);
	printk(KERN_INFO "DS2782 full_charge flag: :%i\n", full_charge_flag);
	printk(KERN_INFO "DS2782 active_empty flag: :%i\n", active_empty_flag);
	printk(KERN_INFO "DS2782 learning: :%i\n", learning);
	printk(KERN_INFO "DS2782 learn complete: :%i\n", learn_complete);
	*/

	if (learn_flag)
	{
		learning = 1;
	}

	if (learning && full_charge_flag)
	{
		printk(KERN_INFO "DS2782 LEARN COMPLETE");
		i2c_smbus_write_byte_data(info->client, DS2782_Register_LearnComplete, 0x01); // 0x20
		info->new_battery = 0;
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

	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(HZ);

	err = ds278x_get_status(info, &status);
	if (err)
		return err;

	err = info->ops->get_battery_current(info, &current_uA);
	if (err)
		return err;

	err = info->ops->get_battery_voltage(info, &voltage);
	if (err)
		return err;

	err = info->ops->get_battery_capacity(info, &capacity);
	if (err)
		return err;

	if(status == POWER_SUPPLY_STATUS_FULL)
	{
		if(voltage > 4200000 && current_uA < 18000 && charger_enabled)
		{
			printk("gpio discharge\n");
			gpio_request_one(info->gpio, GPIOF_DIR_OUT, "MAX8814_EN");
			gpio_set_value(info->gpio,1);
			gpio_free(info->gpio);
			charger_enabled = 0;
		}
	}
	else
	{
		if(capacity <= 85 && !charger_enabled)
		{
			printk("gpio charge\n");
			gpio_request_one(info->gpio, GPIOF_DIR_OUT, "MAX8814_EN");
			gpio_set_value(info->gpio,0);
			gpio_free(info->gpio);
			charger_enabled = 1;
		}
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

static int ds278x_battery_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct ds278x_platform_data *pdata = client->dev.platform_data;
	struct ds278x_info *info;
	int ret;
	int num;
	int new_battery;

	/* Initialize battery registers if not set */
	ret = ds2782_battery_init(client, &new_battery);
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
	info->gpio = pdata->gpio;

	gpio_request_one(info->gpio, GPIOF_DIR_OUT, "MAX8814_EN");
	charger_enabled = !gpio_get_value(info->gpio);
	gpio_free(info->gpio);

	if(charger_enabled)
	{
		printk(KERN_INFO "Charger Enabled\n");
	}
	else
	{
		printk(KERN_INFO "Charger Disabled\n");
	}

	info->new_battery = new_battery;

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
