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

#define DS2782_REG_RARC		0x06	/* Remaining active relative capacity */

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

#define DS2786_CURRENT_UNITS	25

struct ds278x_info;

struct ds278x_battery_ops {
	int (*get_battery_current)(struct ds278x_info *info, int *current_uA);
	int (*get_battery_voltage)(struct ds278x_info *info, int *voltage_uV);
	int (*get_battery_capacity)(struct ds278x_info *info, int *capacity);
	int (*get_battery_acr)(struct ds278x_info *info, int *acr);
};

#define to_ds278x_info(x) container_of(x, struct ds278x_info, battery)

struct ds278x_info {
	struct i2c_client	*client;
	struct power_supply	battery;
	struct ds278x_battery_ops  *ops;
	int			id;
	int                     rsns;
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
	*voltage_uV = (raw / 32) * 4800;
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
	return 1;
	int r;
	r = i2c_smbus_read_byte_data(client, DS2782_REG_RSNSP);
	if (r > 0){
		return 0;
	}
	printk(KERN_INFO "ds278x detected NEW battery\n");
	return 1;
}

static int ipow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}

static int ds278x_dec_to_hex(int decimalNumber)
{
/*	long int quotient;
    int i=1,j,temp;
    char hexadecimalNumber[100];

    quotient = decimalNumber;

    while(quotient!=0)
    {
    	temp = quotient % 16;
    	if( temp < 10)
    		temp =temp + 48;
    	else
    		temp = temp + 55;

    	hexadecimalNumber[i++]= temp;
    	quotient = quotient / 16;
    }

    printk(KERN_INFO "TEO Equivalent hexadecimal value of decimal number %d: ",decimalNumber);
    for(j = i -1 ;j> 0;j--)
    {
    	printk("%c",hexadecimalNumber[j]);
    }
*/
    return 0;
}

static int ds278x_battery_estimate_capacity_from_voltage(struct i2c_client *client)
{
	// We need to set registers ACR MSB and LSB. MSB must be set first
	// - battery initial estimate
	// 4000mAh estimation
	int ret;
	u8 value;
	s16 raw_current;
	s16 raw_voltage;
	int voltage_uV;
	int acr;
	int v2;
	int v3;
	int v4;

	ret = i2c_smbus_read_word_data(client, DS278x_REG_CURRENT_MSB);
	if (ret < 0) {
		printk(KERN_INFO "TEO error reading\n");
		//dev_err(client->dev, "ds278x_battery_estimate_capacity_from_voltage no current reading\n");
		return ret;
	}

	raw_current = swab16(ret);
	printk(KERN_INFO "TEO current reading :%d\n",raw_current);

	ret = i2c_smbus_read_word_data(client, DS278x_REG_VOLT_MSB);
	raw_voltage = swab16(ret);
	voltage_uV = (raw_voltage / 32) * 4800;
	printk(KERN_INFO "TEO voltage reading :%d\n",voltage_uV);

	// voltage_uV is in microvolts

	v2 = voltage_uV * voltage_uV;
	v3 = v2 * voltage_uV;
	v4 = v3 * voltage_uV;
	printk(KERN_INFO "TEO ACR estimation :%d\n",v4);

	float a;
	a = 2.0 * (float)v4;
	printk(KERN_INFO "TEO ACR estimation :%f\n",a);

	/*
	if (current > 0 ) {
		//Charging    y = 54090,572x^4 - 8,293E5x^3+ 4,762E6x^2 -1,213E7x +1,155E7
		acr = 54090.572 * v4 - 829300 * v3 + 4762000 * v2 - 12130000 * voltage_uV + 11550000;
	}
	else {
		//Discharging y = 9077,448x^4 - 1,4E5x^3 + 8,065E5x^2 - 2,05E6x+ 1,939E6
		acr = 9077.448 * v4 - 140000 * v3 + 806500 * v2 - 2050000 * voltage_uV + 1939000;
	}

	printk(KERN_INFO "TEO ACR estimation :%d\n",acr);

	acr = acr * 20;
	acr = acr / 6.25;

	// Convert decimal to hex
	int iacr = (int)acr;

	float a = 1.32;
	a = a * 2 / 6.25;
	int b = (int)a;
	printk(KERN_INFO "TEO ACR estimation :%d\n",b);
	//printk(KERN_INFO "TEO ACR estimation :%f\n",acr);
	//ds278x_dec_to_hex(iacr);
*/
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

static int ds2782_battery_init(struct i2c_client *client)
{
	/* Values extracted from the DS2782K Test kit

	Charge voltage	4V
	Term current	50
	AEV				3V
	AEC				200
	SR				20
	RSGain			1
	RCC				1630
	AB				0
	Full-0			1497
	Full-10			1497
	Full-20			1532
	Full-30			1544
	Full-40			1550
	AE-0			112
	AE-10			101
	AE-20			39
	AE-30			10
	AE-40			0
	SBE-0			59
	SBE-10			42
	SBE-20			9
	SBE-30			2
	SBE-40			0
	*/
	u8 new_battery;
	u8 value;

	new_battery = ds2782_detect_new_battery(client);
	if (!new_battery) return 0;

	// Configure the IC only if new battery detected;
	value = 0x00;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_CONTROL, value); // 0x60
	value = 0x00;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AB, value); // 0x61
	value = 0x32;//0x14;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AC_MSB, value); // 0x62
	value = 0x00;//0xA0;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AC_LSB, value); // 0x63
	value = 0xcd;//0xd3; // 4.137
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_VCHG, value); // 0x64
	value = 0x14;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_IMIN, value); // 0x65
	value = 0x9a;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_VAE, value); // 0x66
	value = 0x0F;//0x14;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_IAE, value); // 0x67
	value = 0x00;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_ActiveEmpty, value); // 0x68
	// TWON-14084
	// RSNS duplicated value at mach-clickARM-4412.c:
	//		#define DS2786_RSNS    20 // Constant sense resistor value, 20 mOhms = 50 siemens = 0x32
	value = 0x32;
	i2c_smbus_write_byte_data(client, DS2782_REG_RSNSP, value); // 0x69
	value = 0x32;//0x14;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full40_MSB, value); // 0x6A
	value = 0x00;//0xA0;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full40_LSB, value); // 0x6B
	value = 0x0F;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full3040Slope, value); // 0x6C
	value = 0x1c;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full2030Slope, value); // 0x6D
	value = 0x26;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full1020Slope, value); // 0x6E
	value = 0x27;//0x28;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_Full0010Slope, value); // 0x6F

	value = 0x07;//0x06;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE3040Slope, value); // 0x70
	value = 0x10;//0x11;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE2030Slope, value); // 0x71
	value = 0x1d;//0x1E;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE1020Slope, value); // 0x72
	value = 0x12;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_AE0010Slope, value); // 0x73
	value = 0x02;//0x01;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE3040Slope, value); // 0x74
	value = 0x05;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE2030Slope, value); // 0x75
	value = 0x05;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE1020Slope, value); // 0x76
	value = 0x0A;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SE0010Slope, value); // 0x77
	value = 0x04;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_RSGAIN_MSB, value); // 0x78
	value = 0x00;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_RSGAIN_LSB, value); // 0x79
	value = 0x00;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_RSTC, value); // 0x7A
	value = 0x04;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_FRSGAIN_MSB, value); // 0x7B
	value = 0x1A;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_FRSGAIN_LSB, value); // 0x7C
	value = 0x68;
	i2c_smbus_write_byte_data(client, DS2782_EEPROM_SlaveAddressConfig, value); // 0x7E

	// make an initial estimate of the capacity from voltage
	ds278x_battery_estimate_capacity_from_voltage(client);

	return 0;
}

static int ds278x_battery_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct ds278x_platform_data *pdata = client->dev.platform_data;
	struct ds278x_info *info;
	int ret;
	int num;

	/* Initialize battery registers if not set */
	ret = ds2782_battery_init(client);
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

	if (id->driver_data == DS2786)
		info->rsns = pdata->rsns;

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
