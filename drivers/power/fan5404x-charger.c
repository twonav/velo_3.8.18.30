/* Copyright (c) 2014 Motorola Mobility LLC. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/i2c.h>
#include <linux/debugfs.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/bitops.h>
#include <linux/mutex.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include <linux/reboot.h>
#include <linux/qpnp/qpnp-adc.h>

#ifndef GENMASK
/* Mask/Bit helpers */
#define GENMASK(u, l) (((1 << ((u) - (l) + 1)) - 1) << (l))
#endif

/* CONTROL0 Register */
#define REG_CONTROL0         0x00
#define CONTROL0_TMR_RST     BIT(7)
#define CONTROL0_EN_STAT     BIT(6)
#define CONTROL0_STAT        GENMASK(5, 4)
#define CONTROL0_STAT_SHIFT  4
#define STAT_READY           0x00
#define STAT_PWM_ENABLED     0x01
#define STAT_CHARGE_DONE     0x02
#define STAT_FAULT           0x03
#define CONTROL0_BOOST       BIT(3)
#define CONTROL0_FAULT       GENMASK(2, 0)
#define CONTROL0_FAULT_SHIFT 0
#define FAULT_NONE           0x00
#define FAULT_VBUS_OVP       0x01
#define FAULT_SLEEP_MODE     0x02
#define FAULT_POOR_INPUT     0x03
#define FAULT_BATT_OVP       0x04
#define FAULT_THERM_SHUTDOWN 0x05
#define FAULT_TIMER_FAULT    0x06
#define FAULT_NO_BATTERY     0x07

/* CONTROL1 Register */
#define REG_CONTROL1           0x01
#define CONTROL1_IBUSLIM       GENMASK(7, 6)
#define CONTROL1_IBUSLIM_SHIFT 6
#define IBUSLIM_FAN54046_100MA     0x00
#define IBUSLIM_FAN54046_500MA     0x01
#define IBUSLIM_FAN54046_800MA     0x02
#define IBUSLIM_FAN54046_NO_LIMIT  0x03
#define IBUSLIM_FAN54053_500MA     0x00
#define IBUSLIM_FAN54053_800MA     0x01
#define IBUSLIM_FAN54053_1100MA    0x02
#define IBUSLIM_FAN54053_NO_LIMIT  0x03
#define CONTROL1_VLOWV         GENMASK(5, 4)
#define CONTROL1_VLOWV_SHIFT   4
#define VLOWV_3_4V             0
#define VLOWV_3_5V             1
#define VLOWV_3_6V             2
#define VLOWV_3_7V             3
#define CONTROL1_TE            BIT(3)
#define CONTROL1_CE_N          BIT(2)
#define CONTROL1_HZ_MODE       BIT(1)
#define CONTROL1_OPA_MODE      BIT(0)

/* OREG Register */
#define REG_OREG              0x02
#define OREG_OREG             GENMASK(7, 2)
#define OREG_OREG_SHIFT       2
#define OREG_DBAT_B           BIT(1)
#define OREG_EOC              BIT(0)

/* IC INFO Register */
#define REG_IC_INFO           0x03
#define IC_INFO_VENDOR_CODE   GENMASK(7, 6)
#define VENDOR_FAIRCHILD_VAL  0x80
#define IC_INFO_PN            GENMASK(5, 3)
#define IC_INFO_PN_SHIFT      3
#define PN_FAN54040_VAL       0x00
#define PN_FAN54041_VAL       0x08
#define PN_FAN54042_VAL       0x10
#define PN_FAN54045_VAL       0x28
#define PN_FAN54046_VAL       0x30
#define PN_FAN54047_VAL       0x30 /* Spec correct? Same as 54046... */
#define IC_INFO_REV           GENMASK(2, 0)
#define IC_INFO_REV_SHIFT     0

/* IBAT Register */
#define REG_IBAT              0x04
#define IBAT_RESET            BIT(7)
#define IBAT_IOCHARGE         GENMASK(6, 3)
#define IBAT_IOCHARGE_SHIFT   3
#define IBAT_ITERM            GENMASK(2, 0)
#define IBAT_ITERM_SHIFT      0

/* VBUS CONTROL Register */
#define REG_VBUS_CONTROL      0x05
#define VBUS_PROD             BIT(6)
#define VBUS_IO_LEVEL         BIT(5)
#define VBUS_VBUS_CON         BIT(4)
#define VBUS_SP               BIT(3)
#define VBUS_VBUSLIM          GENMASK(2, 0)
#define VBUS_VBUSLIM_SHIFT    0

/* SAFETY Register */
#define REG_SAFETY            0x06
#define SAFETY_ISAFE          GENMASK(7, 4)
#define SAFETY_ISAFE_SHIFT    4
#define SAFETY_VSAFE          GENMASK(3, 0)
#define SAFETY_VSAFE_SHIFT    0

/* POST CHARGING Register */
#define REG_POST_CHARGING     0x07
#define PC_BDET               GENMASK(7, 6)
#define PC_BDET_SHIFT         6
#define PC_VBUS_LOAD          GENMASK(5, 4)
#define PC_VBUS_LOAD_SHIFT    4
#define PC_PC_EN              BIT(3)
#define PC_PC_IT              GENMASK(2, 0)
#define PC_PC_IT_SHIFT        0

/* MONITOR0 Register */
#define REG_MONITOR0          0x10
#define MONITOR0_ITERM_CMP    BIT(7)
#define MONITOR0_VBAT_CMP     BIT(6)
#define MONITOR0_LINCHG       BIT(5)
#define MONITOR0_T_120        BIT(4)
#define MONITOR0_ICHG         BIT(3)
#define MONITOR0_IBUS         BIT(2)
#define MONITOR0_VBUS_VALID   BIT(1)
#define MONITOR0_CV           BIT(0)

/* MONITOR1 Register */
#define REG_MONITOR1          0x11
#define MONITOR1_GATE         BIT(7)
#define MONITOR1_VBAT         BIT(6)
#define MONITOR1_POK_B        BIT(5)
#define MONITOR1_DIS_LEVEL    BIT(4)
#define MONITOR1_NOBAT        BIT(3)
#define MONITOR1_PC_ON        BIT(2)

/* NTC Register */
#define REG_NTC               0x12

/* WD CONTROL Register */
#define REG_WD_CONTROL        0x13
#define WD_CONTROL_EN_VREG    BIT(2)
#define WD_CONTROL_WD_DIS     BIT(1)

/* REG RESTART Register */
#define REG_RESTART           0xFA

/* FAN540XX IC PART NO */
#define FAN54046    0X06
#define FAN54053    0X02

#define BOOST_CHECK_DELAY 1000 /* 1 second */

static char *version_str[] = {
	[0] = "fan54040",
	[1] = "fan54041",
	[2] = "fan54042",
	[3] = "unknown",
	[4] = "unknown",
	[5] = "fan54045",
	[6] = "fan54046",
	[7] = "fan54047",

};

struct fan5404x_regulator {
	struct regulator_desc	rdesc;
	struct regulator_dev	*rdev;
};

struct fan5404x_reg {
	char *regname;
	u8   regaddress;
};

static struct fan5404x_reg fan_regs[] = {
	{"CONTROL0",   REG_CONTROL0},
	{"CONTROL1",   REG_CONTROL1},
	{"OREG",       REG_OREG},
	{"IC INFO",    REG_IC_INFO},
	{"IBAT",       REG_IBAT},
	{"VBUS CONTROL", REG_VBUS_CONTROL},
	{"SAFETY",     REG_SAFETY},
	{"POST CHARGING", REG_POST_CHARGING},
	{"MONITOR0",   REG_MONITOR0},
	{"MONITOR1",   REG_MONITOR1},
	{"NTC",        REG_NTC},
	{"WD CONTROL", REG_WD_CONTROL},
};

struct fan_wakeup_source {
	struct wakeup_source	source;
	unsigned long	disabled;
};

struct fan5404x_chg {
	struct i2c_client *client;
	struct device *dev;
	struct mutex read_write_lock;

	struct power_supply *usb_psy;
	struct power_supply batt_psy;
	struct power_supply *bms_psy;
	const char *bms_psy_name;

	bool test_mode;
	int test_mode_soc;
	int test_mode_temp;

	bool factory_mode;
	bool factory_present;
	bool chg_enabled;
	bool usb_present;
	bool batt_present;
	bool chg_done_batt_full;
	bool charging;
	bool batt_hot;
	bool batt_cold;
	bool batt_warm;
	bool batt_cool;
	uint8_t	prev_hz_opa;

	struct delayed_work heartbeat_work;
	struct notifier_block notifier;
	struct qpnp_vadc_chip	*vadc_dev;
	struct dentry	    *debug_root;
	u32    peek_poke_address;
	struct fan5404x_regulator	otg_vreg;
	int ext_temp_volt_mv;
	int ext_high_temp;
	int temp_check;
	int bms_check;
	unsigned int voreg_mv;
	unsigned int low_voltage_uv;
	struct fan_wakeup_source fan_wake_source;
	struct qpnp_adc_tm_chip		*adc_tm_dev;
	struct qpnp_adc_tm_btm_param	vbat_monitor_params;
	bool poll_fast;
	bool shutdown_voltage_tripped;
	atomic_t otg_enabled;
	int ic_info_pn;
	bool factory_configured;
	int max_rate_cap;

	bool demo_mode;
	bool usb_suspended;
};

static struct fan5404x_chg *the_chip;
static void fan5404x_set_chrg_path_temp(struct fan5404x_chg *chip);
static int fan5404x_check_temp_range(struct fan5404x_chg *chip);

static void fan_stay_awake(struct fan_wakeup_source *source)
{
	if (__test_and_clear_bit(0, &source->disabled)) {
		__pm_stay_awake(&source->source);
		pr_debug("enabled source %s\n", source->source.name);
	}
}

static void fan_relax(struct fan_wakeup_source *source)
{
	if (!__test_and_set_bit(0, &source->disabled)) {
		__pm_relax(&source->source);
		pr_debug("disabled source %s\n", source->source.name);
	}
}

static int __fan5404x_read(struct fan5404x_chg *chip, int reg,
				uint8_t *val)
{
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, reg);
	if (ret < 0) {
		dev_err(chip->dev,
			"i2c read fail: can't read from %02x: %d\n", reg, ret);
		return ret;
	} else {
		*val = ret;
	}

	return 0;
}

static int __fan5404x_write(struct fan5404x_chg *chip, int reg,
						uint8_t val)
{
	int ret;

	if (chip->factory_mode)
		return 0;

	ret = i2c_smbus_write_byte_data(chip->client, reg, val);
	if (ret < 0) {
		dev_err(chip->dev,
			"i2c write fail: can't write %02x to %02x: %d\n",
			val, reg, ret);
		return ret;
	}

	dev_dbg(chip->dev, "Writing 0x%02x=0x%02x\n", reg, val);
	return 0;
}

static int fan5404x_read(struct fan5404x_chg *chip, int reg,
				uint8_t *val)
{
	int rc;

	mutex_lock(&chip->read_write_lock);
	rc = __fan5404x_read(chip, reg, val);
	mutex_unlock(&chip->read_write_lock);

	return rc;
}

static int fan5404x_masked_write(struct fan5404x_chg *chip, int reg,
						uint8_t mask, uint8_t val)
{
	int rc;
	uint8_t temp;

	mutex_lock(&chip->read_write_lock);
	rc = __fan5404x_read(chip, reg, &temp);
	if (rc < 0) {
		dev_err(chip->dev, "read failed: reg=%03X, rc=%d\n", reg, rc);
		goto out;
	}
	temp &= ~mask;
	temp |= val & mask;
	rc = __fan5404x_write(chip, reg, temp);
	if (rc < 0) {
		dev_err(chip->dev,
			"write failed: reg=%03X, rc=%d\n", reg, rc);
	}
out:
	mutex_unlock(&chip->read_write_lock);
	return rc;
}

static int __fan5404x_write_fac(struct fan5404x_chg *chip, int reg,
				uint8_t val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(chip->client, reg, val);
	if (ret < 0) {
		dev_err(chip->dev,
			"i2c write fail: can't write %02x to %02x: %d\n",
			val, reg, ret);
		return ret;
	}

	dev_dbg(chip->dev, "Writing 0x%02x=0x%02x\n", reg, val);
	return 0;
}

static int fan5404x_masked_write_fac(struct fan5404x_chg *chip, int reg,
				     uint8_t mask, uint8_t val)
{
	int rc;
	uint8_t temp;

	mutex_lock(&chip->read_write_lock);
	rc = __fan5404x_read(chip, reg, &temp);
	if (rc < 0) {
		dev_err(chip->dev, "read failed: reg=%03X, rc=%d\n", reg, rc);
		goto out;
	}
	temp &= ~mask;
	temp |= val & mask;
	rc = __fan5404x_write_fac(chip, reg, temp);
	if (rc < 0) {
		dev_err(chip->dev,
			"write failed: reg=%03X, rc=%d\n", reg, rc);
	}
out:
	mutex_unlock(&chip->read_write_lock);
	return rc;
}


static int fan5404x_stat_read(struct fan5404x_chg *chip)
{
	int rc;
	uint8_t reg;

	rc = fan5404x_read(chip, REG_CONTROL0, &reg);
	if (rc < 0) {
		dev_err(chip->dev, "Couldn't read STAT rc = %d\n", rc);
		return rc;
	}

	return (reg & CONTROL0_STAT) >> CONTROL0_STAT_SHIFT;
}

static int fan5404x_fault_read(struct fan5404x_chg *chip)
{
	int rc;
	uint8_t reg;

	rc = fan5404x_read(chip, REG_CONTROL0, &reg);
	if (rc < 0) {
		dev_err(chip->dev, "Couldn't read STAT rc = %d\n", rc);
		return rc;
	}

	return (reg & CONTROL0_FAULT) >> CONTROL0_FAULT_SHIFT;
}

static int fan5404x_boost_read(struct fan5404x_chg *chip)
{
	int rc;
	uint8_t reg;

	rc = fan5404x_read(chip, REG_CONTROL0, &reg);
	if (rc < 0) {
		dev_err(chip->dev, "Couldn't read STAT rc = %d\n", rc);
		return rc;
	}

	return  (reg & CONTROL0_BOOST) ? 1 : 0;
}

#define OREG_MIN      3500
#define OREG_STEP_MV  20
#define OREG_STEPS    48
#define OREG_VALUE(s) (OREG_MIN + s*OREG_STEP_MV)
static int fan5404x_set_oreg(struct fan5404x_chg *chip, int value)
{
	int i;
	int rc;

	for (i = OREG_STEPS; i >= 0; i--)
		if (value >= OREG_VALUE(i))
			break;

	if (i < 0)
		return -EINVAL;

	rc = fan5404x_masked_write(chip, REG_OREG, OREG_OREG,
					i << OREG_OREG_SHIFT);
	if (rc) {
		dev_err(chip->dev, "Failed to set OREG_OREG: %d\n", rc);
		return rc;
	}

	return 0;
}

static int ibuslim_fan54046_vals[] = {
	[IBUSLIM_FAN54046_100MA] = 100,
	[IBUSLIM_FAN54046_500MA] = 500,
	[IBUSLIM_FAN54046_800MA] = 800,
	[IBUSLIM_FAN54046_NO_LIMIT] = INT_MAX
};

static int ibuslim_fan54053_vals[] = {
	[IBUSLIM_FAN54053_500MA]   = 500,
	[IBUSLIM_FAN54053_800MA]   = 800,
	[IBUSLIM_FAN54053_1100MA]  = 1100,
	[IBUSLIM_FAN54053_NO_LIMIT] = INT_MAX
};

static int fan5404x_set_ibuslim(struct fan5404x_chg *chip,
				int limit)
{
	int i;
	int rc;

	if (chip->ic_info_pn == FAN54046) {
		for (i = ARRAY_SIZE(ibuslim_fan54046_vals) - 1 -
			     chip->max_rate_cap;
		     i >= 0; i--)
			if (limit >= ibuslim_fan54046_vals[i])
				break;
	} else {
		for (i = ARRAY_SIZE(ibuslim_fan54053_vals) - 1 -
			     chip->max_rate_cap;
		     i >= 0; i--)
			if (limit >= ibuslim_fan54053_vals[i])
				break;
	}

	if (i < 0)
		return -EINVAL;

	rc = fan5404x_masked_write(chip, REG_CONTROL1, CONTROL1_IBUSLIM,
					i << CONTROL1_IBUSLIM_SHIFT);
	if (rc) {
		dev_err(chip->dev, "Failed to set IBUSLIM: %d\n", rc);
		return rc;
	} else if (chip->ic_info_pn == FAN54046) {
		dev_warn(chip->dev, "Set IBUSLIM: %d mA\n",
			 ibuslim_fan54046_vals[i]);
	} else {
		dev_warn(chip->dev, "Set IBUSLIM: %d mA\n",
			 ibuslim_fan54053_vals[i]);
	}

	return 0;
}

#define IBAT_IOCHARGE_MIN     550
#define IBAT_IOCHARGE_STEP_MA 100
#define IBAT_IOCHARGE_STEPS   11
#define IBAT_STEP_CURRENT(s)  (IBAT_IOCHARGE_MIN + s*IBAT_IOCHARGE_STEP_MA)
static int fan5404x_set_iocharge(struct fan5404x_chg *chip,
				int limit)
{
	int i;
	int rc;

	for (i = IBAT_IOCHARGE_STEPS; i >= 0; i--)
		if (limit >= IBAT_STEP_CURRENT(i))
			break;

	if (i < 0)
		return -EINVAL;

	/* Need to keep RESET low... */
	rc = fan5404x_masked_write(chip, REG_IBAT, IBAT_IOCHARGE | IBAT_RESET, // IBAT_RESET = 0
						i << IBAT_IOCHARGE_SHIFT);
	if (rc) {
		dev_err(chip->dev, "Failed to set IOCHARGE: %d\n", rc);
		return rc;
	}

	return 0;
}

static int fan5404x_set_safety(struct fan5404x_chg *chip,
		int limit)
{
	fan5404x_masked_write(chip,REG_SAFETY, SAFETY_ISAFE, 6 << SAFETY_ISAFE_SHIFT);
	fan5404x_masked_write(chip,REG_SAFETY, SAFETY_VSAFE, 7 << SAFETY_VSAFE_SHIFT);
	return 0;
}

#define OREG_FACTORY      0x23
#define IBUSLIM_UNLIMITED 0x03

static int start_charging(struct fan5404x_chg *chip)
{
	union power_supply_propval prop = {0,};
	int rc;
	int current_limit = 0;

	if (!chip->chg_enabled) {
		dev_dbg(chip->dev, "%s: charging enable = %d\n",
				 __func__, chip->chg_enabled);
		return 0;
	}

	dev_dbg(chip->dev, "starting to charge...\n");

	/* Set TMR_RST */
	rc = fan5404x_masked_write(chip, REG_CONTROL0,
				   CONTROL0_TMR_RST,
				   CONTROL0_TMR_RST);
	if (rc) {
		dev_err(chip->dev, "start-charge: Couldn't set TMR_RST\n");
		return rc;
	}

	if (!chip->usb_psy){
		// Hardcode POWER_SUPPLY_PROP_CURRENT_MAX
		current_limit = INT_MAX; // TEST THIS VALUE !!!!!!
	}
	else
	{
		rc = chip->usb_psy->get_property(chip->usb_psy,
					POWER_SUPPLY_PROP_CURRENT_MAX, &prop);
		if (rc < 0) {
			dev_err(chip->dev,
				"could not read USB current_max property, rc=%d\n", rc);
			return rc;
		}

		current_limit = prop.intval / 1000;
	}
	rc = fan5404x_set_ibuslim(chip, current_limit);
	if (rc)
		return rc;

	/* Set IOCHARGE */
	rc = fan5404x_set_iocharge(chip, 1450);
	if (rc)
		return rc;

	/* Clear IO_LEVEL */
	rc = fan5404x_masked_write(chip, REG_VBUS_CONTROL, VBUS_IO_LEVEL, 0); // battery is controlled by IOCHARGE
	if (rc) {
		dev_err(chip->dev, "start-charge: Couldn't clear IOLEVEL\n");
		return rc;
	}

	/* Set OREG to 4.35V */
	rc = fan5404x_set_oreg(chip, chip->voreg_mv);
	if (rc)
		return rc;

	/* Disable T32 */
	rc = fan5404x_masked_write(chip, REG_WD_CONTROL, WD_CONTROL_WD_DIS,
							WD_CONTROL_WD_DIS);
	if (rc) {
		dev_err(chip->dev, "start-charge: couldn't disable T32\n");
		return rc;
	}

	/* Check battery charging temp thresholds */
	chip->temp_check = fan5404x_check_temp_range(chip);

	if (chip->temp_check || chip->ext_high_temp)
		fan5404x_set_chrg_path_temp(chip); // if high temp limit output voltage
	else {
		/* Set CE# Low (enable), TE Low (disable) */
		rc = fan5404x_masked_write(chip, REG_CONTROL1,
				CONTROL1_TE | CONTROL1_CE_N, 0);
		if (rc) {
			dev_err(chip->dev,
				"start-charge: Failed to set TE/CE_N\n");
			return rc;
		}

		chip->charging = true;
	}

	cancel_delayed_work(&chip->heartbeat_work);
	schedule_delayed_work(&chip->heartbeat_work, msecs_to_jiffies(0));

	return 0;
}

static int stop_charging(struct fan5404x_chg *chip)
{
	int rc;

	/* Set CE# High, TE Low */
	rc = fan5404x_masked_write(chip, REG_CONTROL1,
				CONTROL1_TE | CONTROL1_CE_N, CONTROL1_CE_N);
	if (rc) {
		dev_err(chip->dev, "stop-charge: Failed to set TE/CE_N\n");
		return rc;
	}

	chip->charging = false;
	chip->chg_done_batt_full = false;

	rc = fan5404x_set_ibuslim(chip, 500);
	if (rc)
		dev_err(chip->dev, "Failed to set Minimum input current value\n");

	chip->max_rate_cap++;
	if (chip->ic_info_pn == FAN54046) {
		if ((ARRAY_SIZE(ibuslim_fan54046_vals) - chip->max_rate_cap)
		    < 1)
			chip->max_rate_cap =
				ARRAY_SIZE(ibuslim_fan54046_vals) - 1;

	} else {
		if ((ARRAY_SIZE(ibuslim_fan54053_vals) - chip->max_rate_cap)
		    < 1)
			chip->max_rate_cap =
				ARRAY_SIZE(ibuslim_fan54046_vals) - 1;
	}


	cancel_delayed_work(&chip->heartbeat_work);
	schedule_delayed_work(&chip->heartbeat_work, msecs_to_jiffies(0));

	return 0;
}

static irqreturn_t fan5404x_chg_stat_handler(int irq, void *dev_id)
{
	struct fan5404x_chg *chip = dev_id;
	int stat;
	int fault;
	int boost;
	int rc;
	uint8_t ctrl;

	if (chip->factory_mode) {
		rc = fan5404x_read(chip, REG_VBUS_CONTROL, &ctrl);
		if (rc < 0) {
			pr_err("Unable to read VBUS_CONTROL rc = %d\n", rc);
		}else if (chip->usb_psy && !(ctrl & VBUS_VBUS_CON)){
			power_supply_changed(chip->usb_psy);
		}
	}

	stat = fan5404x_stat_read(chip);
	fault = fan5404x_fault_read(chip);
	boost = fan5404x_boost_read(chip);

	pr_debug("CONTROL0.STAT: %X CONTROL0.FAULT: %X CONTROL0.BOOST: %X\n",
							stat, fault, boost);
	if (chip->charging && stat == STAT_PWM_ENABLED)
		start_charging(chip);

	/* Notify userspace only for any charging related events */
	if (!atomic_read(&chip->otg_enabled))
		power_supply_changed(&chip->batt_psy);

	return IRQ_HANDLED;
}

static int factory_kill_disable;
module_param(factory_kill_disable, int, 0644);

static void fan5404x_external_power_changed(struct power_supply *psy)
{
	struct fan5404x_chg *chip = container_of(psy,
				struct fan5404x_chg, batt_psy);
	union power_supply_propval prop = {0,};
	int rc;

	if (chip->bms_psy_name && !chip->bms_psy)
		chip->bms_psy = power_supply_get_by_name(
					(char *)chip->bms_psy_name);

	if (!chip->usb_psy) {
		chip->usb_present = false; // ??????? how do we detect if there is a cable connected ??????
	}
	else{
		rc = chip->usb_psy->get_property(chip->usb_psy,
				POWER_SUPPLY_PROP_PRESENT, &prop);
		pr_debug("External Power Changed: usb=%d\n", prop.intval);

		chip->usb_present = prop.intval;
	}
	if (chip->usb_present)
		start_charging(chip);
	else
		stop_charging(chip);

	if (chip->factory_mode && chip->usb_present
		&& !chip->factory_present)
		chip->factory_present = true;

	if (chip->factory_mode && chip->usb_psy && chip->factory_present
						&& !factory_kill_disable) {
		rc = chip->usb_psy->get_property(chip->usb_psy,
			POWER_SUPPLY_PROP_ONLINE, &prop);
		if (!rc && (prop.intval == 0) && !chip->usb_present /*&& !reboot_in_progress()*/) {
			pr_err("External Power Changed: UsbOnline=%d\n",
							prop.intval);
			kernel_power_off();
		}
	}
}

static enum power_supply_property fan5404x_batt_properties[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CHARGING_ENABLED,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	/* Block from Fuel Gauge */
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_AVG,
	POWER_SUPPLY_PROP_VOLTAGE_OCV,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TEMP_HOTSPOT,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_TAPER_REACHED,
	/* Notification from Fuel Gauge */
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_HEALTH,
};

static int fan5404x_reset_vbat_monitoring(struct fan5404x_chg *chip)
{
	return 0;
	int rc = 0;
	chip->vbat_monitor_params.state_request = ADC_TM_HIGH_LOW_THR_DISABLE;

	rc = qpnp_adc_tm_channel_measure(chip->adc_tm_dev,
					 &chip->vbat_monitor_params);

	if (rc) {
		dev_err(chip->dev, "tm disable failed: %d\n", rc);
	}
	return rc;
}

static int fan5404x_get_prop_batt_status(struct fan5404x_chg *chip)
{
	int rc;
	int stat_reg;
	uint8_t ctrl1;

	stat_reg = fan5404x_stat_read(chip);
	if (stat_reg < 0) {
		dev_err(chip->dev, "Fail read STAT bits, rc = %d\n", stat_reg);
		return POWER_SUPPLY_STATUS_UNKNOWN;
	}

	if (chip->usb_present && chip->chg_done_batt_full)
		return POWER_SUPPLY_STATUS_FULL;

	rc = fan5404x_read(chip, REG_CONTROL1, &ctrl1);
	if (rc < 0) {
		dev_err(chip->dev, "Unable to read REG_CONTROL1 rc = %d\n", rc);
		return POWER_SUPPLY_STATUS_UNKNOWN;
	}

	if (stat_reg == STAT_PWM_ENABLED && !(ctrl1 & CONTROL1_CE_N))
		return POWER_SUPPLY_STATUS_CHARGING;

	return POWER_SUPPLY_STATUS_DISCHARGING;
}

static int fan5404x_get_prop_batt_present(struct fan5404x_chg *chip)
{
	int rc;
	uint8_t reg;
	bool prev_batt_status;

	prev_batt_status = chip->batt_present;

	rc = fan5404x_read(chip, REG_MONITOR1, &reg);
	if (rc < 0) {
		dev_err(chip->dev, "Couldn't read monitor1 rc = %d\n", rc);
		return 0;
	}

	if (reg & MONITOR1_NOBAT)
		chip->batt_present = 0;
	else
		chip->batt_present = 1;

	if ((prev_batt_status != chip->batt_present)
		&& (!prev_batt_status))
		fan5404x_reset_vbat_monitoring(chip);

	return chip->batt_present;
}

static int fan5404x_get_prop_charge_type(struct fan5404x_chg *chip)
{
	int rc;
	int stat_reg;
	uint8_t ctrl1;
	uint8_t mon0;

	stat_reg = fan5404x_stat_read(chip);
	if (stat_reg < 0) {
		dev_err(chip->dev, "Fail read STAT bits, rc = %d\n", stat_reg);
		return POWER_SUPPLY_CHARGE_TYPE_UNKNOWN;
	}

	rc = fan5404x_read(chip, REG_MONITOR0, &mon0);
	if (rc < 0) {
		dev_err(chip->dev, "Unable to read REG_MONITOR0 rc = %d\n", rc);
		return POWER_SUPPLY_CHARGE_TYPE_TRICKLE;
	}

	if (mon0 & MONITOR0_LINCHG)
		return POWER_SUPPLY_CHARGE_TYPE_TRICKLE;

	rc = fan5404x_read(chip, REG_CONTROL1, &ctrl1);
	if (rc < 0) {
		dev_err(chip->dev, "Unable to read REG_CONTROL1 rc = %d\n", rc);
		return POWER_SUPPLY_CHARGE_TYPE_UNKNOWN;
	}

	if (stat_reg == STAT_PWM_ENABLED && !(ctrl1 & CONTROL1_CE_N))
		return POWER_SUPPLY_CHARGE_TYPE_FAST;

	return POWER_SUPPLY_CHARGE_TYPE_NONE;
}

#define DEFAULT_BATT_CAPACITY 50
static int fan5404x_get_prop_batt_capacity(struct fan5404x_chg *chip)
{
	union power_supply_propval ret = {0, };
	int cap = DEFAULT_BATT_CAPACITY;
	int rc;

	if (chip->test_mode)
		return chip->test_mode_soc;

	if (!chip->bms_psy && chip->bms_psy_name)
		chip->bms_psy =
			power_supply_get_by_name((char *)chip->bms_psy_name);

	if (chip->shutdown_voltage_tripped && !chip->factory_mode) {
		if ((chip->usb_psy) && chip->usb_present) {
			/*power_supply_set_present(chip->usb_psy, false);
			power_supply_set_online(chip->usb_psy, false);*/
			chip->usb_present = false;
		}
		return 0;
	}

	if (chip->bms_psy) {
		rc = chip->bms_psy->get_property(chip->bms_psy,
				POWER_SUPPLY_PROP_CAPACITY, &ret);
		if (rc) {
			dev_err(chip->dev, "Couldn't get batt capacity\n");
		}else {
			if (!ret.intval	&& !chip->factory_mode) {
				chip->shutdown_voltage_tripped = true;
				if ((chip->usb_psy) && chip->usb_present) {
					/*power_supply_set_present(chip->usb_psy,
									false);
					power_supply_set_online(chip->usb_psy,
									false);*/
					chip->usb_present = false;
				}
			}
			cap = ret.intval;
		}
	}

	return cap;
}

#define DEFAULT_BATT_VOLT_MV	4000
static int fan5404x_get_prop_batt_voltage_now(struct fan5404x_chg *chip,
						int *volt_mv)
/* Return current voltage */
{
	int rc = 0;
	union power_supply_propval ret = {0, };

	if (!chip->bms_psy && chip->bms_psy_name)
		chip->bms_psy =
			power_supply_get_by_name((char *)chip->bms_psy_name); // bms_psy_name : "fairchild,bms-psy-name"

	if (chip->bms_psy) {
		rc = chip->bms_psy->get_property(chip->bms_psy,
				POWER_SUPPLY_PROP_VOLTAGE_NOW, &ret);
		if (rc < 0) {
			dev_err(chip->dev, "Couldn't get batt voltage\n");
			*volt_mv = DEFAULT_BATT_VOLT_MV;
			return rc;
		}

		*volt_mv = ret.intval / 1000;
		return 0;
	}

	return -EINVAL;
}

static bool fan5404x_get_prop_taper_reached(struct fan5404x_chg *chip)
{
	int rc = 0;
	union power_supply_propval ret = {0, };

	if (!chip->bms_psy && chip->bms_psy_name)
		chip->bms_psy =
			power_supply_get_by_name((char *)chip->bms_psy_name);

	if (chip->bms_psy) {
		rc = chip->bms_psy->get_property(chip->bms_psy,
					 POWER_SUPPLY_PROP_TAPER_REACHED, &ret);
		if (rc < 0) {
			dev_err(chip->dev,
				"couldn't read Taper Reached property, rc=%d\n",
				rc);
			return false;
		}

		if (ret.intval)
			return true;
	}
	return false;
}

static void fan5404x_notify_vbat(enum qpnp_tm_state state, void *ctx)
{
	struct fan5404x_chg *chip = ctx;
	struct qpnp_vadc_result result;
	int batt_volt;
	int rc;
	int adc_volt = 0;

	pr_err("shutdown voltage tripped\n");

	if (chip->vadc_dev) {
		rc = qpnp_vadc_read(chip->vadc_dev, VBAT_SNS, &result);
		adc_volt = (int)(result.physical)/1000;
		pr_info("vbat = %d, raw = 0x%x\n", adc_volt,
							result.adc_code);
	}

	fan5404x_get_prop_batt_voltage_now(chip, &batt_volt);
	pr_info("vbat is at %d, state is at %d\n", batt_volt, state);

	if (state == ADC_TM_LOW_STATE)
		if (adc_volt <= (chip->low_voltage_uv/1000)) {
			pr_info("shutdown now\n");
			chip->shutdown_voltage_tripped = 1;
		} else {
			qpnp_adc_tm_channel_measure(chip->adc_tm_dev,
				&chip->vbat_monitor_params);
		}
	else
		qpnp_adc_tm_channel_measure(chip->adc_tm_dev,
				&chip->vbat_monitor_params);

	power_supply_changed(&chip->batt_psy);
}

static int fan5404x_setup_vbat_monitoring(struct fan5404x_chg *chip)
{
	int rc;

	chip->vbat_monitor_params.low_thr = chip->low_voltage_uv;
	chip->vbat_monitor_params.high_thr = (chip->voreg_mv * 1000) * 2;
	chip->vbat_monitor_params.state_request = ADC_TM_HIGH_LOW_THR_ENABLE;
	chip->vbat_monitor_params.channel = VBAT_SNS;
	chip->vbat_monitor_params.btm_ctx = (void *)chip;

	if (chip->poll_fast) { /* the adc polling rate is higher*/
		chip->vbat_monitor_params.timer_interval =
			ADC_MEAS1_INTERVAL_31P3MS;
	} else /* adc polling rate is default*/ {
		chip->vbat_monitor_params.timer_interval =
			ADC_MEAS1_INTERVAL_1S;
	}

	chip->vbat_monitor_params.threshold_notification =
					&fan5404x_notify_vbat;
	pr_debug("set low thr to %d and high to %d\n",
		chip->vbat_monitor_params.low_thr,
			chip->vbat_monitor_params.high_thr);

	if (!fan5404x_get_prop_batt_present(chip)) {
		pr_info("no battery inserted,vbat monitoring disabled\n");
		chip->vbat_monitor_params.state_request =
						ADC_TM_HIGH_LOW_THR_DISABLE;
	} else {
		rc = qpnp_adc_tm_channel_measure(chip->adc_tm_dev,
			&chip->vbat_monitor_params);
		if (rc) {
			pr_err("tm setup failed: %d\n", rc);
			return rc;
		}
	}

	pr_debug("vbat monitoring setup complete\n");
	return 0;
}

static int fan5404x_temp_charging(struct fan5404x_chg *chip, int enable)
{
	int rc = 0;

	pr_debug("%s: charging enable = %d\n", __func__, enable);

	if (enable && (!chip->chg_enabled)) {
		dev_dbg(chip->dev,
			"%s: chg_enabled is %d, not to enable charging\n",
					__func__, chip->chg_enabled);
		return 0;
	}

	rc = fan5404x_masked_write(chip, REG_CONTROL1, CONTROL1_CE_N,
				enable ? 0 : CONTROL1_CE_N);
	if (rc) {
		dev_err(chip->dev, "start-charge: Failed to set CE_N\n");
		return rc;
	}

	chip->charging = enable;

	return 0;
}

static void fan5404x_charge_enable(struct fan5404x_chg *chip, bool enable)
{
	if (!enable) {
		if (chip->charging) {
			stop_charging(chip);
			chip->max_rate_cap = 0;
		}
	} else {
		if (chip->usb_present && !(chip->charging))
			start_charging(chip);
	}
}

static void fan5404x_usb_suspend(struct fan5404x_chg *chip, bool enable)
{
	int rc;

	if ((chip->usb_suspended && enable) ||
	    (!chip->usb_suspended && !enable))
		return;

	rc = fan5404x_masked_write(the_chip, REG_CONTROL1,
				   CONTROL1_HZ_MODE,
				   enable ? CONTROL1_HZ_MODE : 0);
	if (rc < 0) {
		dev_err(chip->dev,
			"Failed to set USB suspend rc = %d, enable = %d\n",
			rc, (int)enable);
	}else
		chip->usb_suspended = enable;
}

static void fan5404x_set_chrg_path_temp(struct fan5404x_chg *chip)
{
	if (chip->demo_mode) {
		fan5404x_set_oreg(chip, 4000);
		fan5404x_temp_charging(chip, 1);
		return;
	} else if ((chip->batt_cool || chip->batt_warm)
		   && !chip->ext_high_temp)
		fan5404x_set_oreg(chip, chip->ext_temp_volt_mv);
	else
		fan5404x_set_oreg(chip, chip->voreg_mv);

	if (chip->ext_high_temp ||
		chip->batt_cold ||
		chip->batt_hot ||
		chip->chg_done_batt_full)
		fan5404x_temp_charging(chip, 0);
	else
		fan5404x_temp_charging(chip, 1);
}

static int fan5404x_check_temp_range(struct fan5404x_chg *chip)
{
	int batt_volt;
	int batt_soc;
	int ext_high_temp = 0;

	if (fan5404x_get_prop_batt_voltage_now(chip, &batt_volt)) // get current voltage (Volts)
		return 0;

	batt_soc = fan5404x_get_prop_batt_capacity(chip); // get current capacity

	if ((chip->batt_cool || chip->batt_warm) &&
		(batt_volt > chip->ext_temp_volt_mv))
			ext_high_temp = 1;

	if (chip->ext_high_temp != ext_high_temp) {
		chip->ext_high_temp = ext_high_temp;

		dev_warn(chip->dev, "Ext High = %s\n",
			chip->ext_high_temp ? "High" : "Low");

		return 1;
	}

	return 0;
}

static int fan5404x_get_prop_batt_health(struct fan5404x_chg *chip)
{
	int batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;

	if (chip->batt_hot)
		batt_health = POWER_SUPPLY_HEALTH_OVERHEAT;
	else if (chip->batt_cold)
		batt_health = POWER_SUPPLY_HEALTH_COLD;
	/*else if (chip->batt_warm)
		batt_health = POWER_SUPPLY_HEALTH_WARM;
	else if (chip->batt_cool)
		batt_health = POWER_SUPPLY_HEALTH_COOL;*/
	else
		batt_health = POWER_SUPPLY_HEALTH_GOOD;

	return batt_health;
}

static int fan5404x_set_prop_batt_health(struct fan5404x_chg *chip, int health)
{
	switch (health) {
	case POWER_SUPPLY_HEALTH_OVERHEAT:
		chip->batt_hot = true;
		chip->batt_cold = false;
		chip->batt_warm = false;
		chip->batt_cool = false;
		break;
	case POWER_SUPPLY_HEALTH_COLD:
		chip->batt_cold = true;
		chip->batt_hot = false;
		chip->batt_warm = false;
		chip->batt_cool = false;
		break;
	/*case POWER_SUPPLY_HEALTH_WARM:
		chip->batt_warm = true;
		chip->batt_hot = false;
		chip->batt_cold = false;
		chip->batt_cool = false;
		break;
	case POWER_SUPPLY_HEALTH_COOL:
		chip->batt_cool = true;
		chip->batt_hot = false;
		chip->batt_cold = false;
		chip->batt_warm = false;
		break;*/
	case POWER_SUPPLY_HEALTH_GOOD:
	default:
		chip->batt_hot = false;
		chip->batt_cold = false;
		chip->batt_warm = false;
		chip->batt_cool = false;
	}

	return 0;
}

static int fan5404x_batt_set_property(struct power_supply *psy,
					enum power_supply_property prop,
					const union power_supply_propval *val)
{
	struct fan5404x_chg *chip = container_of(psy,
				struct fan5404x_chg, batt_psy);

	switch (prop) {
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		chip->chg_enabled = !!val->intval;
		fan5404x_charge_enable(chip, chip->chg_enabled);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (chip->test_mode)
			chip->test_mode_soc = val->intval;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		fan_stay_awake(&chip->fan_wake_source);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		fan_stay_awake(&chip->fan_wake_source);
		fan5404x_set_prop_batt_health(chip, val->intval);
		fan5404x_check_temp_range(chip);
		fan5404x_set_chrg_path_temp(chip);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		if (chip->test_mode)
			chip->test_mode_temp = val->intval;
		break;
	default:
		return -EINVAL;
	}

	cancel_delayed_work(&chip->heartbeat_work);
	schedule_delayed_work(&chip->heartbeat_work,
			      msecs_to_jiffies(0));

	return 0;
}

static int fan5404x_batt_is_writeable(struct power_supply *psy,
					enum power_supply_property prop)
{
	int rc;

	switch (prop) {
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
	case POWER_SUPPLY_PROP_CAPACITY:
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
	case POWER_SUPPLY_PROP_HEALTH:
	case POWER_SUPPLY_PROP_TEMP:
		rc = 1;
		break;
	default:
		rc = 0;
		break;
	}

	return rc;
}

static int fan5404x_bms_get_property(struct fan5404x_chg *chip,
				    enum power_supply_property prop)
{
	union power_supply_propval ret = {0, };

	if (!chip->bms_psy && chip->bms_psy_name)
		chip->bms_psy =
			power_supply_get_by_name((char *)chip->bms_psy_name);

	if (chip->bms_psy) {
		chip->bms_psy->get_property(chip->bms_psy,
				prop, &ret);
		return ret.intval;
	}

	return -EINVAL;
}

static int fan5404x_batt_get_property(struct power_supply *psy,
					enum power_supply_property prop,
					union power_supply_propval *val)
{
	struct fan5404x_chg *chip = container_of(psy,
				struct fan5404x_chg, batt_psy);

	switch (prop) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = fan5404x_get_prop_batt_status(chip);
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = fan5404x_get_prop_batt_present(chip);
		break;
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		val->intval = chip->chg_enabled;
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		val->intval = fan5404x_get_prop_charge_type(chip);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = fan5404x_get_prop_batt_capacity(chip);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = fan5404x_get_prop_batt_health(chip);

		/*if (val->intval ==  POWER_SUPPLY_HEALTH_WARM) {
			if (chip->ext_high_temp)
				val->intval = POWER_SUPPLY_HEALTH_OVERHEAT;
			else
				val->intval = POWER_SUPPLY_HEALTH_GOOD;
		}

		if (val->intval ==  POWER_SUPPLY_HEALTH_COOL) {
			if (chip->ext_high_temp)
				val->intval = POWER_SUPPLY_HEALTH_COLD;
			else
				val->intval = POWER_SUPPLY_HEALTH_GOOD;
		}*/
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		if (chip->test_mode) {
			val->intval = chip->test_mode_temp;
			return 0;
		}
		/* Fall through if test mode temp is not set */
	/* Block from Fuel Gauge */
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
	case POWER_SUPPLY_PROP_VOLTAGE_AVG:
	case POWER_SUPPLY_PROP_VOLTAGE_OCV:
	case POWER_SUPPLY_PROP_CHARGE_FULL:
	case POWER_SUPPLY_PROP_CHARGE_COUNTER:
	case POWER_SUPPLY_PROP_TEMP_HOTSPOT:
	case POWER_SUPPLY_PROP_CURRENT_NOW:
	case POWER_SUPPLY_PROP_CURRENT_AVG:
	case POWER_SUPPLY_PROP_TAPER_REACHED:
		val->intval = fan5404x_bms_get_property(chip, prop);
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		val->intval = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#define DEMO_MODE_MAX_SOC 35
#define DEMO_MODE_HYS_SOC 5
static void heartbeat_work(struct work_struct *work)
{
	struct fan5404x_chg *chip =
		container_of(work, struct fan5404x_chg,
					heartbeat_work.work);

	/* Set TMR_RST */
	int rc;
	rc = fan5404x_masked_write(chip, REG_CONTROL0,
				   CONTROL0_TMR_RST,
				   CONTROL0_TMR_RST);
	if (rc) {
		dev_err(chip->dev, "start-charge: Couldn't set TMR_RST\n");
		return;
	}

	/* Set IBUSLIM */
	rc = fan5404x_set_ibuslim(chip, INT_MAX);
	if (rc){
		dev_err(chip->dev, "start-charge: Couldn't set IBUSLIM\n");
		return;
	}

	/* Set IOCHARGE */
	rc = fan5404x_set_iocharge(chip, 1150);
	if (rc) {
		dev_err(chip->dev, "start-charge: Couldn't set IOCHARGE\n");
		return;
	}

	/* Clear IO_LEVEL */
	rc = fan5404x_masked_write(chip, REG_VBUS_CONTROL, VBUS_IO_LEVEL, 0); // battery is controlled by IOCHARGE
	if (rc) {
		dev_err(chip->dev, "start-charge: Couldn't set IO_LEVEL\n");
		return;
	}

	/* Set SAFETY ISAFE 1150mA VSAFE 4.34V*/
	fan5404x_set_safety(chip,0);

	/* Set OREG to 4.34V */
	rc = fan5404x_set_oreg(chip, 4340);
	if (rc) {
		dev_err(chip->dev, "start-charge: Couldn't set OREG\n");
		return;
	}

	rc = fan5404x_masked_write(chip, REG_CONTROL0,
					   CONTROL0_TMR_RST,
					   CONTROL0_TMR_RST);
	if (rc) {
		dev_err(chip->dev, "start-charge: Couldn't set TMR_RST\n");
		return;
	}

	cancel_delayed_work(&chip->heartbeat_work);
	schedule_delayed_work(&chip->heartbeat_work, msecs_to_jiffies(20000));
	return;
};

static int fan5404x_of_init(struct fan5404x_chg *chip) // OF : Open Firmware
{
	int rc;
	struct device_node *node = chip->dev->of_node;

	rc = of_property_read_string(node, "fairchild,bms-psy-name",
				&chip->bms_psy_name);
	if (rc)
		chip->bms_psy_name = NULL;

	rc = of_property_read_u32(node, "fairchild,ext-temp-volt-mv",
						&chip->ext_temp_volt_mv);
	if (rc < 0)
		chip->ext_temp_volt_mv = 0;

	rc = of_property_read_u32(node, "fairchild,oreg-voltage-mv",
						&chip->voreg_mv);
	if (rc < 0)
		chip->voreg_mv = 4350;

	rc = of_property_read_u32(node, "fairchild,low-voltage-uv",
						&chip->low_voltage_uv);
	if (rc < 0)
		chip->low_voltage_uv = 3200000;
	return 0;
}

static int fan5404x_hw_init(struct fan5404x_chg *chip)
{
	int rc;

	/* Disable T32 */
	rc = fan5404x_masked_write(chip, REG_WD_CONTROL, WD_CONTROL_WD_DIS, // REG13-BIT(1) : write BIT(1) = 1 << (1)
							WD_CONTROL_WD_DIS);
	if (rc) {
		dev_err(chip->dev, "couldn't disable T32 rc = %d\n", rc);
		return rc;
	}

	return 0;
}

static int fan5404x_read_chip_id(struct fan5404x_chg *chip, uint8_t *val)
{
	int rc;

	rc = fan5404x_read(chip, REG_IC_INFO, val);
	if (rc)
		return rc;

	if ((*val & IC_INFO_VENDOR_CODE) != VENDOR_FAIRCHILD_VAL) {
		dev_err(chip->dev, "Unknown vendor IC_INFO: %.2X\n", *val);
		return -EINVAL;
	}

	dev_dbg(chip->dev, "Found PN: %s Revision: 1.%d\n",
		version_str[(*val & IC_INFO_PN) >> IC_INFO_PN_SHIFT],
		*val & IC_INFO_REV);

	return 0;
}

static struct of_device_id fan5404x_match_table[] = {
	{ .compatible = "fairchild,fan54040_charger", },
	{ },
};

#define DEFAULT_TEST_MODE_SOC  52
#define DEFAULT_TEST_MODE_TEMP  225
static int fan5404x_charger_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	int rc;
	struct fan5404x_chg *chip;

	uint8_t reg;
	union power_supply_propval ret = {0, };

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL); // device memory allocation
	if (!chip) {
		dev_err(&client->dev, "fan5404x_charger Unable to allocate memory\n");
		return -ENOMEM;
	}

	chip->client = client; // i2c_client
	chip->dev = &client->dev; // should be bus controller

	mutex_init(&chip->read_write_lock);

	rc = fan5404x_read_chip_id(chip, &reg); // identify chip, in our case is 54040
	if (rc) {
		dev_err(&client->dev, "Could not read from FAN5404x: %d\n", rc);
		return -ENODEV;
	}

	chip->ic_info_pn = (reg & IC_INFO_PN) >> IC_INFO_PN_SHIFT; // PN-> part number bits (3:5) : should be 000

	i2c_set_clientdata(client, chip); // This is a void pointer that is for the driver to use.
	                                  // One would use this pointer mainly to pass driver related data around.

	INIT_DELAYED_WORK(&chip->heartbeat_work, heartbeat_work);
	schedule_delayed_work(&chip->heartbeat_work, msecs_to_jiffies(20000));

	return 0;
}

static int fan5404x_charger_remove(struct i2c_client *client)
{
	struct fan5404x_chg *chip = i2c_get_clientdata(client);
	cancel_delayed_work(&chip->heartbeat_work);
	devm_kfree(chip->dev, chip);
	the_chip = NULL;

	return 0;
}

static int fan5404x_suspend(struct device *dev)
{
	return 0;
}

static int fan5404x_suspend_noirq(struct device *dev)
{
	return 0;
}

static int fan5404x_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops fan5404x_pm_ops = {
	.resume		= fan5404x_resume,
	.suspend_noirq	= fan5404x_suspend_noirq,
	.suspend	= fan5404x_suspend,
};

static const struct i2c_device_id fan5404x_charger_id[] = {
	{"fan5404x_charger", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, fan5404x_charger_id);

struct fan5404x_charger_platform_data {
	u16	model;				/* 2007. */
};

static struct fan5404x_charger_platform_data fan5404x_charger_pdata = {
	.model		= 54040,
};

static struct i2c_board_info fan5404x_charger_board_info[] __initdata = {
		{
			I2C_BOARD_INFO("fan5404x_charger", 0x6b),
			.platform_data = &fan5404x_charger_pdata
		},
};

static struct i2c_driver fan5404x_charger_driver = {
	.driver		= {
		.name		= "fan5404x_charger",
		.owner		= THIS_MODULE,
		.of_match_table	= fan5404x_match_table,
		.pm		= &fan5404x_pm_ops,
	},
	.probe		= fan5404x_charger_probe,
	.remove		= fan5404x_charger_remove,
	.id_table	= fan5404x_charger_id,
};

static struct i2c_client *fan5404x_client;

static int __init fan5404x_charger_init(void)
{
	int err;
	struct i2c_adapter *adapter;
	adapter = i2c_get_adapter(4);

	if (adapter == NULL) {
		err = -ENXIO;
		goto error;
	}

	fan5404x_client = i2c_new_device(adapter, &fan5404x_charger_board_info);
	err = fan5404x_client ? 0 : -ENXIO;
	return i2c_add_driver(&fan5404x_charger_driver);

error:
	return err;
}
module_init(fan5404x_charger_init);

static void __exit fan5404x_charger_exit(void)
{
	i2c_unregister_device(fan5404x_client);
	i2c_del_driver(&fan5404x_charger_driver);
}
module_exit(fan5404x_charger_exit);

MODULE_DESCRIPTION("FAN5404x Charger");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("i2c:fan5404x-charger");

