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

#undef PREFIX
#define PREFIX				"TN KEYBOARD: "

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

/* Joystick Debounce Filter */
#define JOYSTICK_ENTER_FILTER_MS 200
#define JOYSTICK_DEBOUNCE_DELAY 80

static unsigned long enter_delay_time = 0;
static unsigned long last_valid_event_time = 0;

enum KeyStatus {
	RELEASED = 0,
	PRESSED = 1
};

static struct workqueue_struct *keyboard_workqueue;
static unsigned long workqueue_debounce_delay = 0;
struct input_dev *keyboard_input_device;

static int queue_event_type = 0;
static int joystick_active_button = 0;

struct twonav_kbd_device {
	struct input_dev *input_dev;
	char			phys[32];

	struct i2c_client *i2c_client;
	
	int irq;

	wait_queue_head_t	wait;
	bool stopped;

	int	 (*get_pendown_state)(void);
};

void twonav_kdb_send_joystick_up(int key) {
	input_report_key(keyboard_input_device, key, RELEASED);
	input_sync(keyboard_input_device);
}

void twonav_kbd_filter_send_delayed_joystick_up() {
	twonav_kdb_send_joystick_up(queue_event_type);

	queue_event_type = 0;
	joystick_active_button = 0;
	last_valid_event_time = jiffies;
}

void delayed_kbd_event_handler(struct delayed_work *work)
{
	if (work && keyboard_input_device) {
		if (queue_event_type != 0) {
			twonav_kbd_filter_send_delayed_joystick_up();
		}
		else {
			pr_debug(PREFIX "Event canceled from a rebound down\n");
		}
	}
}

DECLARE_DELAYED_WORK(keyboard_delayed_work, delayed_kbd_event_handler);

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

static void twonav_kbd_filter_send_joystick_press(struct input_dev *input_dev, int key_type) {
	last_valid_event_time = jiffies;
	joystick_active_button = key_type;
	input_report_key(input_dev, key_type, PRESSED);
}

static void twonav_kbd_filter_cancel_queued_up_event(int key_type) {
	if (key_type == queue_event_type) {
		queue_event_type = 0;
	}
	else {
		pr_debug(PREFIX "invalid cancel key :%d\n", key_type);
	}
}

static void twonav_kbd_filter_enqueue_joystick_up(int key_type) {
	queue_event_type = key_type;
	queue_delayed_work(keyboard_workqueue, &keyboard_delayed_work, workqueue_debounce_delay);
}

static void twonav_kbd_process_joystick_event(struct input_dev *input_dev,
										  	  const unsigned char joystick_status,
											  const int joystick_event_type,
											  int key_type)
{
	int press = ((joystick_status & joystick_event_type) != 0)?PRESSED:RELEASED;
	pr_debug(PREFIX "Event:%d pressed:%d\n"
			"Active key:%d queue_event_type:%d\n",
			key_type,
			press,
			joystick_active_button,
			queue_event_type);

	if (press == PRESSED) {
		if (joystick_active_button == 0) {
			twonav_kbd_filter_send_joystick_press(input_dev, key_type);
		}
		else {
			if (joystick_active_button == key_type) {
				twonav_kbd_filter_cancel_queued_up_event(key_type);
			}
			else {
				pr_debug(PREFIX "Ignore simultaneous down key:%d\n", key_type);
			}
		}
	}
	else {
		if (joystick_active_button == key_type) {
			if (queue_event_type == 0) {
				twonav_kbd_filter_enqueue_joystick_up(key_type);
			}
			else {
				pr_debug(PREFIX "Ignore double up with no down in between\n");
			}
		}
		else {
			pr_debug(PREFIX "Ignore up key:%d with no down\n", key_type);
		}
	}
}

static void twonav_kbd_process_key_event(struct input_dev *input_dev,
									 const unsigned char keys_status,
									 const int key_type,
									 int key_send_type)
{
	int press = ((keys_status & key_type) != 0)?PRESSED:RELEASED;
	input_report_key(input_dev, key_send_type, press);
}

// Filter ENTER key: if it comes too fast after another key event
static int twonav_kbd_is_enter_valid() {
	int is_valid = 0;
	unsigned long diff = jiffies - last_valid_event_time;
	if ((joystick_active_button == KEY_ENTER) || (diff > enter_delay_time) ) {
		is_valid = 1;
	}
	else {
		pr_debug(PREFIX "Ignore fast enter\n");
	}
	return is_valid;
}

static void twonav_kbd_send_evts(struct twonav_kbd_device * kb, int curr)
{
	static int oldVal = 0;
	unsigned char js_curr = (curr >> 8) & 0xFF;
	unsigned char ks_curr = (curr & 0xFF);
	unsigned char joystick = (js_curr ^ ((oldVal >> 8) & 0xFF));
	unsigned char joystick_status = (joystick & js_curr);
	unsigned char keys = (ks_curr ^ (oldVal & 0xFF));
	unsigned char keys_status = (keys & ks_curr);

	if (joystick) {
		if (joystick & JOYSTICK_UP) {
			twonav_kbd_process_joystick_event(kb->input_dev, joystick_status, JOYSTICK_UP, KEY_UP);
		}
		else if (joystick & JOYSTICK_DOWN) {
			twonav_kbd_process_joystick_event(kb->input_dev, joystick_status, JOYSTICK_DOWN, KEY_DOWN);
		}
		else if (joystick & JOYSTICK_LEFT) {
			twonav_kbd_process_joystick_event(kb->input_dev, joystick_status, JOYSTICK_LEFT, KEY_LEFT);
		}
		else if (joystick & JOYSTICK_RIGHT) {
			twonav_kbd_process_joystick_event(kb->input_dev, joystick_status, JOYSTICK_RIGHT, KEY_RIGHT);
		}
		else if (joystick & JOYSTICK_BTN) {
			int is_enter_valid = twonav_kbd_is_enter_valid();
			if (is_enter_valid) {
				twonav_kbd_process_joystick_event(kb->input_dev, joystick_status, JOYSTICK_BTN, KEY_ENTER);
			}
		}
	}

	if (keys) {
		if (keys & KEY_TOP_LEFT){
			twonav_kbd_process_key_event(kb->input_dev, keys_status, KEY_TOP_LEFT, KEY_F3);
		}
		if (keys & KEY_TOP_RIGHT){
			twonav_kbd_process_key_event(kb->input_dev, keys_status, KEY_TOP_RIGHT, KEY_F4);
		}
		if (keys & KEY_BOTTOM_LEFT){
			twonav_kbd_process_key_event(kb->input_dev, keys_status, KEY_BOTTOM_LEFT, KEY_F5);
		}
		if (keys & KEY_BOTTOM_RIGHT){
			twonav_kbd_process_key_event(kb->input_dev, keys_status, KEY_BOTTOM_RIGHT, KEY_F6);
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

static ssize_t twonav_kbd_debounce_show(struct device *dev,
        								struct device_attribute *attr,
										char *buf)
{
    return sprintf(buf, "%d\n", workqueue_debounce_delay);
}

static ssize_t twonav_kbd_debounce_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	int err;
	int value;

	err = kstrtoint(buf, 10, &value);
	if (err < 0) {
		return err;
	}

	if (value > 0) {
		workqueue_debounce_delay = msecs_to_jiffies(value);
	}
	else {
		printk(KERN_INFO "twonav_kbd: invalid debounce value\n");
	}

	return count;
}

static DEVICE_ATTR(keyboard_debounce_ms, S_IRUGO | S_IWUSR, twonav_kbd_debounce_show,
		twonav_kbd_debounce_store);

static ssize_t twonav_kbd_enter_delay_show(struct device *dev,
        								struct device_attribute *attr,
										char *buf)
{
    return sprintf(buf, "%d\n", enter_delay_time);
}

static ssize_t twonav_kbd_enter_delay_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	int err;
	int value;

	err = kstrtoint(buf, 10, &value);
	if (err < 0) {
		return err;
	}

	if (value > 0) {
		enter_delay_time = msecs_to_jiffies(value);
	}
	else {
		printk(KERN_INFO "twonav_kbd: invalid enter delay value\n");
	}

	return count;
}

static DEVICE_ATTR(keyboard_enter_delay_ms, S_IRUGO | S_IWUSR, twonav_kbd_enter_delay_show,
		twonav_kbd_enter_delay_store);

static int twonav_kbd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct twonav_kbd_platform_data *pdata;
	struct twonav_kbd_device *keyboard;
	struct input_dev *input_dev;
	int error;
	struct device *dev = &client->dev;

	keyboard_workqueue = create_workqueue("keyboard_workqueue");
	workqueue_debounce_delay = msecs_to_jiffies(JOYSTICK_DEBOUNCE_DELAY);
	enter_delay_time = msecs_to_jiffies(JOYSTICK_ENTER_FILTER_MS);
	last_valid_event_time = jiffies;

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
	keyboard_input_device = input_dev;

	error = twonav_kbd_configure_chip(keyboard, pdata);
	if (error) {
		dev_err(&client->dev, "Can't configure chip");
		goto err_free_irq;
	}

	if (pdata->init_platform_hw)
		pdata->init_platform_hw();

	if (twonav_kbd_xfer(keyboard, JOYSTICK_INTERRUPT_FLAG) < 0){
		dev_err(&client->dev, "Device is not present");
		goto err_free_irq;	
	}

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


  // Register sysfs attributes
  device_create_file(dev, &dev_attr_keyboard_debounce_ms);
  device_create_file(dev, &dev_attr_keyboard_enter_delay_ms);

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
	struct device *dev = &client->dev;

	flush_workqueue( keyboard_workqueue );
	destroy_workqueue( keyboard_workqueue );

	free_irq(twonav_kbd->irq, twonav_kbd);

	input_unregister_device(twonav_kbd->input_dev);
	kfree(twonav_kbd);

	device_remove_file(dev, &dev_attr_keyboard_debounce_ms);

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
