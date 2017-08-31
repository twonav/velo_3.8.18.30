/*
 * Copyright (c) 2017 CompeGPS 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Driver for TwoNav Aventura and Trail joystick and keyboard
 *
 * TODO:
 *	- Power on the chip when open() and power down when close()
 *	- Manage power mode
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/input/twonav_kbd.h>
#include <linux/delay.h>

#define DRIVER_DESC "Driver for TwoNav Aventura and Trail keyboard"
#define MODULE_DEVICE_ALIAS "twonav_kbd" 

MODULE_AUTHOR("Ignasi Serra <iserra@twonav.com>");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

/* registers Address*/
#define IODIR_ADDR		0x00 //GPIO direction
#define IPOL_ADDR		0x02 //GPIO polarity
#define GPINTEN_ADDR	0x04 //Enable interruption
#define DEFVAL_ADDR		0x06 //Default value
#define INTCON_ADDR		0x08 //Interrupt on change
#define IOCON_ADDR		0x0A //MCP23017 IO Configuration
#define GPPU_ADDR		0x0C //Pullup registers

#define JOYSTICK_INTERRUPT_FLAG 0x0E //Reflects the interrupt condition when a pin causes the interruption
#define BUTTON_INTERRUPT_FLAG	0x0F //
#define JOYSTICK_INTERRUPT_CAP 	0x10 //Captures the GPIO port value at the time the interrupt occurred
#define BUTTON_INTERRUPT_CAP	0x11 //
#define JOYSTICK_GPIO			0x12 //GPIO current value for joystick
#define BUTTON_GPIO				0x13 //GPIO current value for keyboard

/* Config register*/
#define PORT_A_CONF		0x1F //Port A configuration (Enable joystick gpio pins)
#define PORT_B_CONF		0x0F //Port B configuration (Enable keyboard gpio pins)
#define MCP23017_CONF	0x40 //Enable Mirror interruption

/* Joystick Mask */
#define JOYSTICK_BTN	0x01
#define JOYSTICK_DOWN	0x02
#define JOYSTICK_LEFT	0x04
#define JOYSTICK_UP		0x08
#define JOYSTICK_RIGHT	0x10

/* Keys Mask */
#define KEY_TOP_LEFT		0x01
#define KEY_TOP_RIGHT		0x02
#define KEY_BOTTOM_LEFT		0x04
#define KEY_BOTTOM_RIGHT	0x08

struct twonav_kbd_device {
	struct input_dev *input_dev;
	char			phys[32];

	struct i2c_client *i2c_client;
	
	int irq;

	wait_queue_head_t	wait;
	bool stopped;

	int	 (*get_pendown_state)(void);
};

static int twonav_kbd_i2c_write(struct i2c_client *client,
								uint8_t aregaddr,
								uint8_t avalue)
{
	uint8_t data[2] = { aregaddr, avalue };
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = I2C_M_IGNORE_NAK,
		.len = 2,
		.buf = (uint8_t *)data
	};
	int error;

	error = i2c_transfer(client->adapter, &msg, 1);
	return error < 0 ? error : 0;
}

static int twonav_kbd_i2c_read(struct i2c_client *client,
			   uint8_t aregaddr, signed char *value)
{
	uint8_t data[2] = { aregaddr };
	struct i2c_msg msg_set[2] = {
		{
			.addr = client->addr,
			.flags = I2C_M_REV_DIR_ADDR,
			.len = 1,
			.buf = (uint8_t *)data
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD | I2C_M_NOSTART,
			.len = 1,
			.buf = (uint8_t *)data
		}
	};
	int error;

	error = i2c_transfer(client->adapter, msg_set, 2);
	if (error < 0)
		return error;

	*value = data[0];
	return 0;
}

static inline int twonav_kbd_xfer(struct twonav_kbd_device *kb, u8 cmd)
{
	s32 data;
	u16 val;

	data = i2c_smbus_read_word_data(kb->i2c_client, cmd);
	if (data < 0) {
		dev_err(&kb->i2c_client->dev, "i2c io error: %d\n", data);
		return data;
	}

	val = swab16(data);

	//dev_notice(&kb->i2c_client->dev, "data: 0x%x, val: 0x%x\n", data, val);

	return val;
}

static void twonav_kbd_send_evts(struct twonav_kbd_device * kb, int curr)
{
	static int oldVal = 0;
	static int isEnterPressed = 0;
	unsigned char js_curr = (curr >> 8) & 0xFF;
	unsigned char ks_curr = (curr & 0xFF);
	unsigned char joystick = (js_curr ^ ((oldVal >> 8) & 0xFF));
	unsigned char keys = (ks_curr ^ (oldVal & 0xFF));
	int press = 0;
	struct i2c_client *client = kb->i2c_client;

	if (joystick) {
		if (joystick & JOYSTICK_UP){
			press = (((joystick & js_curr)& JOYSTICK_UP) != 0)?1:0;
			input_report_key(kb->input_dev, KEY_UP, press);
		}
		if (joystick & JOYSTICK_DOWN){
			press = (((joystick & js_curr)& JOYSTICK_DOWN) != 0)?1:0;
			input_report_key(kb->input_dev, KEY_DOWN, press);
		}
		if (joystick & JOYSTICK_LEFT){
			press = (((joystick & js_curr)& JOYSTICK_LEFT) != 0)?1:0;
			input_report_key(kb->input_dev, KEY_LEFT, press);
		}
		if (joystick & JOYSTICK_RIGHT){
			press = (((joystick & js_curr)& JOYSTICK_RIGHT) != 0)?1:0;
			input_report_key(kb->input_dev, KEY_RIGHT, press);
		}
		if ((joystick & JOYSTICK_BTN) && ((js_curr & ~JOYSTICK_BTN) == 0 || isEnterPressed)){
			press = (((joystick & js_curr)& JOYSTICK_BTN) != 0)?1:0;
			if ((js_curr & ~JOYSTICK_BTN) == 0) {
				input_report_key(kb->input_dev, KEY_ENTER, press);
				isEnterPressed = press;
			} else if (isEnterPressed && !press){
				input_report_key(kb->input_dev, KEY_ENTER, press);
				isEnterPressed = press;
			}
		}
	}

	if (keys) {
		if (keys & KEY_TOP_LEFT){
			press = (((keys & ks_curr)& KEY_TOP_LEFT) != 0)?1:0;
			input_report_key(kb->input_dev, KEY_F3, press);
		}
		if (keys & KEY_TOP_RIGHT){
			press = (((keys & ks_curr)& KEY_TOP_RIGHT) != 0)?1:0;
			input_report_key(kb->input_dev, KEY_F4, press);
		}
		if (keys & KEY_BOTTOM_LEFT){
			press = (((keys & ks_curr)& KEY_BOTTOM_LEFT) != 0)?1:0;
			input_report_key(kb->input_dev, KEY_F5, press);
		}
		if (keys & KEY_BOTTOM_RIGHT){
			press = (((keys & ks_curr)& KEY_BOTTOM_RIGHT) != 0)?1:0;
			input_report_key(kb->input_dev, KEY_F6, press);
		}
	}

	oldVal = curr;

	return;
}


static void twonav_kbd_stop(struct twonav_kbd_device *kb)
{
	dev_notice(&kb->i2c_client->dev,"twonav_kbd_stop\n");
	kb->stopped = true;
	mb();
	wake_up(&kb->wait);

	disable_irq(kb->irq);
}

static int twonav_kbd_open(struct input_dev *input_dev)
{
	struct twonav_kbd_device *kb = input_get_drvdata(input_dev);
	dev_notice(&kb->i2c_client->dev,"twonav_kbd_open\n");

	kb->stopped = false;
	mb();

	enable_irq(kb->irq);

	return 0;
}

static void twonav_kbd_close(struct input_dev *input_dev)
{
	struct twonav_kbd_device *kb = input_get_drvdata(input_dev);

	twonav_kbd_stop(kb);
	return;
}


static irqreturn_t twonav_kbd_interrupt_process(int irq, void *dev_id)
{
	struct twonav_kbd_device *kb;

	if (dev_id) {
		kb = (struct twonav_kbd_device *)dev_id;
		
		if (!kb) {
			printk(KERN_ERR "twonav_kbd_interrupt Invalid pointer\n");
			goto out;
		} 
		else if (!kb->stopped){
			int val = 0; 
			unsigned char keys = 0, joystick = 0;
			struct i2c_client *client = kb->i2c_client;

			//Read INT values
			val = twonav_kbd_xfer(kb, JOYSTICK_INTERRUPT_FLAG);
			if (val < 0){
				printk(KERN_ERR "twonav_kbd_interrupt error: twonav_kbd_i2c_read - INTERRUPT_FLAG\n");
				goto err;
			}

			//Read GPIO values
			val = twonav_kbd_xfer(kb, JOYSTICK_GPIO);
			if (val < 0){
				printk(KERN_ERR "twonav_kbd_interrupt error: twonav_kbd_i2c_read - GPIO\n");
				goto err;
			}

			twonav_kbd_send_evts(kb, val);
			input_sync(kb->input_dev);

			//dev_notice(&client->dev,"twonav_kbd_interrupt:");
			

		} else {
			printk(KERN_ERR "twonav_kbd_interrupt (stopped) :(\n");
		}
	}
out:
	return IRQ_HANDLED;

err:
	twonav_kbd_stop(kb);
	return IRQ_HANDLED;

}

static int twonav_kbd_configure_chip(struct twonav_kbd_device *twonav_kbd,
					const struct twonav_kbd_platform_data *plat_dat)
{
	struct i2c_client *client = twonav_kbd->i2c_client;
	int error;
	//signed char value;

	/* Set mcp23017 configuration*/
	error = twonav_kbd_i2c_write(client, IOCON_ADDR, MCP23017_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port A set interruption on change failed\n");
		return error;
	}
	error = twonav_kbd_i2c_write(client, (IOCON_ADDR + 0x01), MCP23017_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port B set interruption on change failed\n");
		return error;
	}

	/* Enable mcp23017 gpio pins direction and pull up*/
	error = twonav_kbd_i2c_write(client, IODIR_ADDR, PORT_A_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port A configuration failed\n");
		return error;
	}
	error = twonav_kbd_i2c_write(client, (IODIR_ADDR + 0x01), PORT_B_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port B configuration failed\n");
		return error;
	}

	error = twonav_kbd_i2c_write(client, GPPU_ADDR, PORT_A_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port A pull up failed\n");
		return error;
	}
	error = twonav_kbd_i2c_write(client, (GPPU_ADDR + 0x01), PORT_B_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port B pull up failed\n");
		return error;
	}
	
	/* Change mcp23017 gpio polarity*/
	error = twonav_kbd_i2c_write(client, IPOL_ADDR, PORT_A_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port A polarity change failed\n");
		return error;
	}
	error = twonav_kbd_i2c_write(client, (IPOL_ADDR + 0x01), PORT_B_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port B polarity change failed\n");
		return error;
	}

	/* Set mcp23017 interruptions on change*/
	error = twonav_kbd_i2c_write(client, INTCON_ADDR, PORT_A_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port A set interruption on change failed\n");
		return error;
	}
	error = twonav_kbd_i2c_write(client, (INTCON_ADDR + 0x01), PORT_B_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port B set interruption on change failed\n");
		return error;
	}
	/* Set GPIO defaulr values */
	error = twonav_kbd_i2c_write(client, DEFVAL_ADDR, 0x00);
	if (error < 0) {
		dev_err(&client->dev, "IO port A set default value failes\n");
		return error;
	}
	error = twonav_kbd_i2c_write(client, (DEFVAL_ADDR + 0x01), 0x00);
	if (error < 0) {
		dev_err(&client->dev, "IO port B set default value failes\n");
		return error;
	}

	/* Enable mcp23017 interruptions */
	error = twonav_kbd_i2c_write(client, GPINTEN_ADDR, PORT_A_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port A enable interruption failed\n");
		return error;
	}
	error = twonav_kbd_i2c_write(client, (GPINTEN_ADDR + 0x01), PORT_B_CONF);
	if (error < 0) {
		dev_err(&client->dev, "IO port B enable interruption change failed\n");
		return error;
	}

	return 0;
}

static int twonav_kbd_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct twonav_kbd_platform_data *pdata;
	struct twonav_kbd_device *keyboard;
	struct input_dev *input_dev;
	int error;

	dev_notice(&client->dev, "twonav_kbd_probe!\n");
	
	pdata = client->dev.platform_data;
	if (!pdata) {
		dev_err(&client->dev, "platform data is required!\n");
		return -EINVAL;
	}

	if (!client->irq) {
		dev_err(&client->dev, "No device IRQ?\n");
		return -EINVAL;
	}

	keyboard = kzalloc(sizeof(struct twonav_kbd_device), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!keyboard || !input_dev) {
		dev_err(&client->dev,
			"Can't allocate memory for device structure\n");
		error = -ENOMEM;
		goto err_free_mem;
	}

	keyboard->i2c_client = client;
	keyboard->irq = client->irq;
	keyboard->input_dev = input_dev;
	init_waitqueue_head(&keyboard->wait);

	keyboard->get_pendown_state = pdata->get_pendown_state;

	snprintf(keyboard->phys, sizeof(keyboard->phys),
		 "%s/input0", dev_name(&client->dev));

	input_dev->name = "TwoNav keyboard";
	input_dev->phys = keyboard->phys;
	input_dev->id.bustype = BUS_I2C;

	input_dev->open = twonav_kbd_open;
	input_dev->close = twonav_kbd_close;

	input_set_drvdata(input_dev, keyboard);

	error = twonav_kbd_configure_chip(keyboard, pdata);
	if (error) {
		dev_err(&client->dev, "Can't configure chip");
		goto err_free_irq;
	}

	if (pdata->init_platform_hw)
		pdata->init_platform_hw();

	input_dev->evbit[0] = BIT_MASK(EV_KEY);

	__set_bit(KEY_F3, input_dev->keybit);
	__set_bit(KEY_F4, input_dev->keybit);
	__set_bit(KEY_F5, input_dev->keybit);
	__set_bit(KEY_F6, input_dev->keybit);
	__set_bit(KEY_UP, input_dev->keybit);
	__set_bit(KEY_RIGHT, input_dev->keybit);
	__set_bit(KEY_DOWN, input_dev->keybit);
	__set_bit(KEY_LEFT, input_dev->keybit);
	__set_bit(KEY_ENTER, input_dev->keybit);

	error = request_threaded_irq(keyboard->irq, 
					 NULL,
				     twonav_kbd_interrupt_process, 
				     IRQF_ONESHOT,
				     client->dev.driver->name, keyboard);
	if (error) {
		dev_err(&client->dev, "Can't allocate twonav_kbd irq %d\n", keyboard->irq);
		goto err_free_irq;
	}
	else {
		dev_notice(&client->dev, "Allocated twonav_kbd irq %d\n", keyboard->irq);
	}
	
	twonav_kbd_stop(keyboard);

	error = input_register_device(keyboard->input_dev);
	if (error) {
		dev_err(&client->dev, "Failed to register input device\n");
		goto err_free_irq;
	}

	i2c_set_clientdata(client, keyboard);

	dev_notice(&client->dev, "Init twonav_kbd driver probe success\n");
	return 0;

err_free_irq:
	free_irq(keyboard->irq, keyboard);
err_free_mem:
	input_free_device(input_dev);
	kfree(keyboard);

	return error;
}

static int twonav_kbd_remove(struct i2c_client *client)
{
	struct twonav_kbd_device *twonav_kbd = i2c_get_clientdata(client);

	free_irq(twonav_kbd->irq, twonav_kbd);

	input_unregister_device(twonav_kbd->input_dev);
	kfree(twonav_kbd);

	return 0;
}

static const struct i2c_device_id twonav_kbd_id[] = {
	{ MODULE_DEVICE_ALIAS, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, twonav_kbd_id);

static struct i2c_driver twonav_kbd_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name = MODULE_DEVICE_ALIAS,
	},
	.id_table	= twonav_kbd_id,
	.probe		= twonav_kbd_probe,
	.remove		= twonav_kbd_remove,
};

module_i2c_driver(twonav_kbd_driver);
