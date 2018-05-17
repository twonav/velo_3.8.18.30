/*
 * linux/arch/arm/mach-exynos4/mach-TwoNav.c
 *
 *
 * Based on mach-hkdh4412.c
 *
 * Copyright (c) 2012 AgreeYa Mobility Co., Ltd.
 *		http://www.agreeyamobility.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/i2c/pca953x.h>
#include <linux/input.h>
#include <linux/rtc.h>
#include <linux/alarmtimer.h>
#include <linux/io.h>
#include <linux/mfd/max77686.h>
#include <linux/mmc/host.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/pwm_backlight.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/serial_core.h>
#include <linux/platform_data/s3c-hsotg.h>
#include <linux/platform_data/i2c-s3c2410.h>
#include <linux/platform_data/usb-ehci-s5p.h>
#include <linux/platform_data/usb-exynos.h>
#include <linux/platform_data/usb3503.h>
#include <linux/platform_data/tps611xx_bl.h>
#include <linux/delay.h>
#include <linux/lcd.h>
#include <linux/clk.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/reboot.h>
#include <linux/i2c/tsc2007.h>
#include <linux/wl12xx.h>

#include <asm/mach/arch.h>
#include <asm/hardware/gic.h>
#include <asm/mach-types.h>

#include <plat/backlight.h>
#include <plat/clock.h>
#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/gpio-cfg.h>
#include <plat/keypad.h>
#include <plat/mfc.h>
#include <plat/regs-serial.h>
#include <plat/sdhci.h>
#include <plat/fb.h>
#include <plat/hdmi.h>

#include <video/platform_lcd.h>
#include <video/samsung_fimd.h>
#include <linux/platform_data/spi-s3c64xx.h>

#include <mach/gpio.h>
#include <mach/map.h>
#include <mach/regs-pmu.h>
#include <mach/dwmci.h>
#include <drm/exynos_drm.h>

#include "common.h"
#include "pmic-77686.h"

extern char *device_version;

/*VELO INCLUDES*/
#include <linux/pwm_backlight.h>

#include <linux/w1-gpio.h>
#define VELO_FAN_INT    EXYNOS4X12_GPM3(0) /*IRQ XEINT8*/

//extern void exynos4_setup_dwmci_cfg_gpio(struct platform_device *dev, int width);

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define TWONAV_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define TWONAV_ULCON_DEFAULT	S3C2410_LCON_CS8

#define TWONAV_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg twonav_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= TWONAV_UCON_DEFAULT,
		.ulcon		= TWONAV_ULCON_DEFAULT,
		.ufcon		= TWONAV_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= TWONAV_UCON_DEFAULT,
		.ulcon		= TWONAV_ULCON_DEFAULT,
		.ufcon		= TWONAV_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= TWONAV_UCON_DEFAULT,
		.ulcon		= TWONAV_ULCON_DEFAULT,
		.ufcon		= TWONAV_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= TWONAV_UCON_DEFAULT,
		.ulcon		= TWONAV_ULCON_DEFAULT,
		.ufcon		= TWONAV_UFCON_DEFAULT,
	},
};

/*DS2782 BATTERY FUEL GAUGE*/
#if defined(CONFIG_BATTERY_DS2782)
#include <linux/ds2782_battery.h>
#define DS2786_RSNS    	20 /* Constant sense resistor value, mOhms */
#define MAX8814_EN    	EXYNOS4X12_GPM3(1) /* Enable GPIO */
#define MCP73833_CHARGE_MANAGER_PG    EXYNOS4X12_GPM3(0)
#define MCP73833_CHARGE_MANAGER_STAT1	EXYNOS4X12_GPM3(6)
#define MCP73833_CHARGE_MANAGER_STAT2	EXYNOS4X12_GPM4(3)

struct ds278x_platform_data ds278x_pdata = {
	.rsns = DS2786_RSNS,
	.gpio_enable_charger = MAX8814_EN,
	.gpio_pg = MCP73833_CHARGE_MANAGER_PG,
	.gpio_stat1 = MCP73833_CHARGE_MANAGER_STAT1,
	.gpio_stat2 = MCP73833_CHARGE_MANAGER_STAT2,
};
#endif
/*FAN54040 CONFIGURATION PLATDATA*/
/*FAN CHARGER INTERRUPT INIT FAN54:XEINT8*/
//static int fan54_chr_init(void)
//{
//	/* FAN54_INT: XEINT_8 *///EXYNOS4212_GPM3(0)
//	gpio_request(VELO_FAN_INT, "FAN54_INT");
//	s3c_gpio_cfgpin(VELO_FAN_INT, S3C_GPIO_SFN(0xf));
//	s3c_gpio_setpull(VELO_FAN_INT, S3C_GPIO_PULL_NONE);
//}
/*END OF FAN54040 CONFIGURATION PLATDATA*/

/*TMP103 THERMOMETER MODEL A/B PLATDATA*/
//#if defined(CONFIG_TMP103_SENSOR) || defined(CONFIG_TMP103_SENSOR_MODULE)
//static struct temp_sensor_pdata tmp103_sensor_pdata = {
//	.source_domain = "pcb_case",
//	.average_number = 20,
//	.report_delay_ms = 250,
//};
//
//static struct platform_device bowser_case_sensor_device = {
//	.name	= "tmp103_temp_sensor",
//	.id	= -1,
//	.dev	= {
//	.platform_data = &tmp103_sensor_pdata,
//	},
//};
//#endif

/* MPU9250 CONFIG */
#if defined(CONFIG_SENSOR_MPU9250) || defined(CONFIG_SENSOR_MPU9250_MODULE) /*GPA1CON5 // XEINT30 */
static struct mpu_platform_data gyro_platform_data = {
        .int_config  = 0x00,
        .level_shifter = 0,
        .orientation = {   1,  0,  0,
                           0,  1,  0,
                           0,  0, 1 },
        .sec_slave_type = SECONDARY_SLAVE_TYPE_COMPASS,
        .sec_slave_id   = COMPASS_ID_AK8963,
        .secondary_i2c_addr = 0x0C,
        .secondary_orientation = { 0,  1, 0,
                                   -1, 0,  0,
                                   0,  0,  1 },
};
#endif

/*USB 3505 SMC CCOnfiguration*/
#if defined(CONFIG_USB_HSIC_USB3503)
static struct usb3503_platform_data usb3503_pdata = {
	.initial_mode	= 	USB3503_MODE_HUB,
	.ref_clk		= 	USB3503_REFCLK_26M,
	.gpio_intn		= EXYNOS4_GPX3(0),
	.gpio_connect	= EXYNOS4_GPX3(4),
	.gpio_reset		= EXYNOS4_GPX3(5),
};
#endif
/*END OF USB 3505 SMC CCOnfiguration*/

/*touchscreen config tsc2007 XE_INT22*/
#if defined(CONFIG_TOUCHSCREEN_TSC2007) || defined(CONFIG_TOUCHSCREEN_TSC2007_MODULE)
#include <linux/i2c/tsc2007.h>
#define tsc2007_penirq_pin      EXYNOS4_GPX2(6) /*IRQ_EINT22*/
static int tsc2007_get_pendown_state(void)
{
	return !gpio_get_value(tsc2007_penirq_pin);
}
/*TOUCHSCREEN INTERRUPT INIT TOUCH_INT:XEINT22*/
static int tsc2007_init_platform_hw(void)
{
	/* TOUCH_INT: XEINT_22 */
	gpio_request(tsc2007_penirq_pin, "TOUCH_INT");
	s3c_gpio_cfgpin(tsc2007_penirq_pin, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(tsc2007_penirq_pin, S3C_GPIO_PULL_UP);

	return 0;
}

static void tsc2007_exit_platform_hw(void)
{
	gpio_free(tsc2007_penirq_pin);
}

static void tsc2007_clear_penirq(void)
{
	gpio_set_value(tsc2007_penirq_pin, 1);
}

// 2016-08-08 DNP TWON-13812: [#1] Ajustar umbral de resistencia a un valor mas bajo (max_rt)
//				=> Cuando se pulsa flojo, los valores X, Y tienen mucho error
//                            [#2] Aumentar control fuzz de los valores X, Y (fuzzx, fuzzy)
//				=> Para evitar baile de X, Y debido a variaciones electricas en el touch
// 2017-06-23 TPA:			  [#3] Adjust driver settings for new touch panel
struct tsc2007_platform_data tsc2007_info = {
	.model 		= 2007,	/* 2007. */

	.x_plate_ohms	= 265, /* must be non-zero value */
	.y_plate_ohms	= 680, /* must be non-zero value */
	/* max. resistance above which samples are ignored */
	.max_rt		= 1200, // [#1] antes 1<<12

	.poll_delay	= 20, /* delay (in ms) after pen-down event
					     before polling starts */
	.poll_period = 10,/* time (in ms) between samples */

	/* fuzz factor for X, Y and pressure axes */
	.fuzzx		= 64, // [#2] antes 64
	.fuzzy		= 64, // [#2] antes 64
	.fuzzz		= 64,

	.get_pendown_state	= tsc2007_get_pendown_state,
	.clear_penirq 		= tsc2007_clear_penirq,
	.init_platform_hw	= tsc2007_init_platform_hw,
	.exit_platform_hw	= tsc2007_exit_platform_hw
};
#endif
/*END OF touchscreen config tsc2007 XE_INT22*/

/* Keyboard Aventura/trail MCP23017 (I2C GPIO expander) XE_INT14*/
#if defined CONFIG_JOYSTICK_TWONAV_KBD
#include <linux/input/twonav_kbd.h>
#define twonav_kbd_irq_pin		EXYNOS4_GPX1(6) /*IRQ_EINT14*/

static int twonav_kbd_get_pendown_state(void)
{
    return !gpio_get_value(twonav_kbd_irq_pin);
}

static int twonav_kbd_init_platform_hw(void)
{
    /* TOUCH_INT: XEINT_22 */
    printk(KERN_INFO "twonav_kbd_init_platform_hw\n");
	gpio_request(twonav_kbd_irq_pin, "TWON_KBD_INT");
    s3c_gpio_cfgpin(twonav_kbd_irq_pin, S3C_GPIO_SFN(0xf));
    s3c_gpio_setpull(twonav_kbd_irq_pin, S3C_GPIO_PULL_UP);

    return 0;
}

static void twonav_kbd_exit_platform_hw(void)
{
    gpio_free(twonav_kbd_irq_pin);
}

struct twonav_kbd_platform_data twonav_kbd_info = {
	.base = 0x20,
	.get_pendown_state = twonav_kbd_get_pendown_state,
	.init_platform_hw =  twonav_kbd_init_platform_hw,
	.exit_platform_hw =  twonav_kbd_exit_platform_hw
};
#endif
/* END OF Keyboard Aventura/trail MCP23017 (I2C GPIO expander)*/

/* LCD Backlight data tps611xx PWM_platform_data*/

static struct tps611xx_platform_data tps611xx_data = {
	.rfa_en = 1,
	.en_gpio_num = EXYNOS4_GPD0(2),
};

static struct platform_device tps611xx = {
	.name	= "tps61165_bl",
	.dev	= {
		.platform_data	= &tps611xx_data,
	},
};

/* END OF LCD Backlight data tps611xx PWM_platform_data*/

/*Audio Codec MAX98090 Configuration*/
#if defined(CONFIG_SND_SOC_MAX98090)
#include <sound/max98090.h>
static struct max98090_pdata max98090 = {
	.digmic_left_mode	= 0,
	.digmic_right_mode	= 0,
	.digmic_3_mode		= 0,
	.digmic_4_mode		= 0,
};
#endif

/*END OF Audio Codec MAX98090 Configuration*/

/*Ambient light sensor MAX44005 Configuration*/
// static struct max44005_platform_data max44005_driver = {
// 	.class	= I2C_CLASS_HWMON,
// 	.driver  = {
// 		.name = "max44005",
// 		.owner = THIS_MODULE,
// 		.of_match_table = of_match_ptr(max44005_of_match),
// 		.pm = MAX44005_PM_OPS,
// 	},
// 	.probe	 = max44005_probe,
// 	.shutdown = max44005_shutdown,
// 	.remove  = max44005_remove,
// 	.id_table = max44005_id,
// };
/*END OF Ambient light sensor MAX44005 Configuration*/

/*Devices Conected on I2C BUS 0 LISTED ABOVE*/
static struct i2c_board_info twonav_i2c_devs0[] __initdata = {
	{
		I2C_BOARD_INFO("max77686", (0x12 >> 1)),
		.platform_data	= &exynos4_max77686_info,
	},
#if defined(CONFIG_USB_HSIC_USB3503)
	{
		I2C_BOARD_INFO("usb3503", (0x08)),
		.platform_data  = &usb3503_pdata,
	},
#endif
};
/*END OF Devices Conected on I2C BUS 0 LISTED ABOVE*/

static struct i2c_board_info twonav_i2c_devs1[] __initdata = {
#if defined(CONFIG_TOUCHSCREEN_TSC2007)
        {
                I2C_BOARD_INFO("tsc2007", 0x48),
                .platform_data  = &tsc2007_info,
                .irq            = IRQ_EINT(22),
        },
#endif
  	
#if defined(CONFIG_JOYSTICK_TWONAV_KBD)
        {
            I2C_BOARD_INFO("twonav_kbd", 0x20),
            .platform_data  = &twonav_kbd_info,
            .irq            = IRQ_EINT(14),
        },
#endif

#if defined(CONFIG_SND_SOC_MAX98090)
	{
		I2C_BOARD_INFO("max98090", (0x20>>1)),
		.platform_data  = &max98090,
		.irq		= IRQ_EINT(0),
	},
#endif

	/*UNCOMMENT WHEN READY*/
//#if defined(CONFIG_TMP103_SENSOR)
//	{
//		I2C_BOARD_INFO("tmp103_temp_sensor", 0x71),
//		.platform_data = &tmp103_sensor_pdata,
//	},
//#endif

//#if defined(CONFIG_LPS25H)
//	{
//		I2C_BOARD_INFO("lps25h", ????),/*CONFIG DIRECCTION*/
//		.platform_data  = &?????????,/*CONFIG PDATA*/
//		.irq		= ????????, /*xe.int25*/
//	},
//#endif
	/*UNCOMMENT WHEN READY*/
#if defined(CONFIG_SENSORS_MAX44005)  // ambient light sensor 
	{
		I2C_BOARD_INFO("max44005", 0x88), /*Write: 0x88 Read: 0x89*/
		.platform_data  = &max44005_driver, /*CONFIG PDATA*/

	},
#endif
};
/*END OF Devices Conected on I2C BUS 1 LISTED ABOVE*/

/* I2C4 bus GPIO-Bitbanging */
#define		GPIO_I2C4_SDA	EXYNOS4_GPB(0)
#define		GPIO_I2C4_SCL	EXYNOS4_GPB(1)
static struct 	i2c_gpio_platform_data 	i2c4_gpio_platdata = {
	.sda_pin = GPIO_I2C4_SDA,
	.scl_pin = GPIO_I2C4_SCL,
	.udelay  = 5,
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0
};

static struct 	platform_device 	gpio_device_i2c4 = {
	.name 	= "i2c-gpio",
	.id  	= 4,    // adepter number
	.dev.platform_data = &i2c4_gpio_platdata,
};
static struct i2c_board_info twonav_i2c_devs4[] __initdata = {
#if defined(CONFIG_BATTERY_DS2782)
	{
		I2C_BOARD_INFO("ds2782", 0x34),
		.platform_data  = &ds278x_pdata,
	},
#endif
#if defined(CONFIG_FAN54040)
/**/
	{
		I2C_BOARD_INFO("fan54040", 0x6B),
		.platform_data  = &tsc2007_info,
		.irq		= VELO_FAN_INT,/*xeint8 // GPM3CON0 CHAGE STATUS // GPM3CON1 DISABLE CHARGE*/
	},
#endif
#if defined(CONFIG_SENSOR_MPU9250)
	{
	     I2C_BOARD_INFO("mpu9250", 0x68),
         .irq = (IH_GPIO_BASE + MPUIRQ_GPIO),
	     .platform_data = &gyro_platform_data,
	 },
#endif
};

/*Define VELO display with DRM */
#if defined(CONFIG_LCD_T55149GD030J) && defined(CONFIG_DRM_EXYNOS_FIMD)
#if defined(CONFIG_TWONAV_AVENTURA) || defined(CONFIG_TWONAV_TRAIL)
	static struct exynos_drm_fimd_pdata drm_fimd_pdata = {
	.panel = {
		.timing = {
			.left_margin 	= 40,
			.right_margin 	= 24,
			.upper_margin 	= 7,
			.lower_margin 	= 5,
			.hsync_len 	= 32,
			.vsync_len 	= 5,
			.xres 		= 480,
			.yres 		= 640,
		},
	},
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC | VIDCON1_INV_VCLK,
	.default_win 	= 0,
	.bpp 		= 32,
	};

#else
	static struct exynos_drm_fimd_pdata drm_fimd_pdata = {
		.panel = {
			.timing = {
				.left_margin 	= 9,
				.right_margin 	= 9,
				.upper_margin 	= 5,
				.lower_margin 	= 5,
				.hsync_len 	= 1,
				.vsync_len 	= 1,
				.xres 		= 240,
				.yres 		= 400,
				.refresh	= 80,
			},
			.width_mm = 39,
			.height_mm = 65,
		},
		.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
		.vidcon1	= VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC |
					  VIDCON1_INV_VCLK | VIDCON1_INV_VDEN,
		.default_win 	= 0,
		.bpp 		= 24,
	};
#endif
	
static void lcd_t55149gd030j_set_power(struct plat_lcd_data *pd,
				   unsigned int power)
{
	if (power) {
		gpio_set_value(EXYNOS4X12_GPM1(5),1);
	} else {
		gpio_set_value(EXYNOS4X12_GPM1(5),0);
	}
	gpio_free(EXYNOS4X12_GPM1(5));

}

static struct plat_lcd_data twonav_lcd_t55149gd030j_data = {
	.set_power	= lcd_t55149gd030j_set_power,
	
};

static struct platform_device twonav_lcd_t55149gd030j = {
	.name	= "platform-lcd",
	.dev	= {
		.parent		= &s5p_device_fimd0.dev,
		.platform_data	= &twonav_lcd_t55149gd030j_data,
	},
};
#endif
/*END OF Define VELO display with DRM */

/* GPIO KEYS KEYBOARD*/
static struct gpio_keys_button twonav_gpio_keys_tables[] = {
	{
		.code			= KEY_F1,
		.gpio			= EXYNOS4X12_GPM3(7),	/* VELO SIDE BUTTON TR POWERON */
		.desc			= "KEY_POWER",
		.type			= EV_KEY,
		.active_low		= 0,
	},
	{
		.code			= KEY_F2,
		.gpio			= EXYNOS4_GPF2(5), // VELO SIDE BUTTON TL
		.desc			= "TL_BUTTON",
		.type			= EV_KEY,
		.active_low		= 1,
	},

#if defined(CONFIG_TWONAV_VELO) || defined(CONFIG_TWONAV_HORIZON)
	{
		.code			= KEY_F3,
		.gpio			= EXYNOS4_GPJ1(1), /* VELO FRONT BUTTON BR */
		.desc			= "BR_BUTTON",
		.type			= EV_KEY,
		.active_low		= 0,
	},
	{
		.code			= KEY_F4,
		.gpio			= EXYNOS4_GPJ0(1), /* VELO FRONT BUTTON BL */
		.desc			= "BL_BUTTON",
		.type			= EV_KEY,
		.active_low		= 1,
	},
#endif
};

static struct gpio_keys_platform_data twonav_gpio_keys_data = {
	.buttons	= twonav_gpio_keys_tables,
	.nbuttons	= ARRAY_SIZE(twonav_gpio_keys_tables),
};

static struct platform_device twonav_gpio_keys = {
	.name	= "gpio-keys",
	.dev	= {
		.platform_data	= &twonav_gpio_keys_data,
	},
};

void init_button_irqs(void)
	{
		/*
			Number of irqs is limited by S5P_GPIOINT_GROUP_COUNT in arch/arm/plat-samsung/include/plat/irqs.h
			Using s5p_register_gpio_interrupt(), 8 irqs are allocated (for the full 8 gpios of the chip), like defined in S5P_GPIOINT_GROUP_SIZE
		*/

		int numero_de_irq=-1;

		numero_de_irq=s5p_register_gpio_interrupt(EXYNOS4X12_GPM3(7));
		printk("twonav_gpio_keys_tables: irq %d\n",numero_de_irq);

		numero_de_irq=s5p_register_gpio_interrupt(EXYNOS4_GPF2(5));
		printk("twonav_gpio_keys_tables: irq %d\n",numero_de_irq);

		numero_de_irq=s5p_register_gpio_interrupt(EXYNOS4_GPJ0(1));
		printk("twonav_gpio_keys_tables: irq %d\n",numero_de_irq);

		numero_de_irq=s5p_register_gpio_interrupt(EXYNOS4_GPJ1(1));
		printk("twonav_gpio_keys_tables: irq %d\n",numero_de_irq);
	}
/*END OF GPIO KEYS KEYBOARD*/ 		

#if defined(CONFIG_SND_SOC_HKDK_MAX98090)
static struct platform_device hardkernel_audio_device = {
	.name	= "hkdk-snd-max98090",
	.id	= -1,
};
#endif

/* USB EHCI */
static struct s5p_ehci_platdata twonav_ehci_pdata;

static void __init twonav_ehci_init(void)
{
	struct s5p_ehci_platdata *pdata = &twonav_ehci_pdata;

	s5p_ehci_set_platdata(pdata);
}

/* USB OHCI */
static struct exynos4_ohci_platdata twonav_ohci_pdata;

static void __init twonav_ohci_init(void)
{
	struct exynos4_ohci_platdata *pdata = &twonav_ohci_pdata;

	exynos4_ohci_set_platdata(pdata);
}

/* USB OTG */
static struct s3c_hsotg_plat twonav_hsotg_pdata;

#ifdef CONFIG_USB_EXYNOS_SWITCH
static struct s5p_usbswitch_platdata twonav_usbswitch_pdata;

static void __init twonav_usbswitch_init(void)
{
	struct s5p_usbswitch_platdata *pdata = &twonav_usbswitch_pdata;
	int err;

	pdata->gpio_host_detect = EXYNOS4_GPX3(1); /* low active */
	err = gpio_request_one(pdata->gpio_host_detect, GPIOF_IN, "HOST_DETECT");
	if (err) {
		printk(KERN_ERR "failed to request gpio_host_detect\n");
		return;
	}

	s3c_gpio_cfgpin(pdata->gpio_host_detect, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(pdata->gpio_host_detect, S3C_GPIO_PULL_UP);
	gpio_free(pdata->gpio_host_detect);

	pdata->gpio_device_detect = EXYNOS4_GPX1(6); /* high active */
	err = gpio_request_one(pdata->gpio_device_detect, GPIOF_IN, "DEVICE_DETECT");
	if (err) {
		printk(KERN_ERR "failed to request gpio_host_detect for\n");
		return;
	}

	s3c_gpio_cfgpin(pdata->gpio_device_detect, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(pdata->gpio_device_detect, S3C_GPIO_PULL_DOWN);
	gpio_free(pdata->gpio_device_detect);

	pdata->gpio_host_vbus = EXYNOS4_GPL2(0);
	err = gpio_request_one(pdata->gpio_host_vbus, GPIOF_OUT_INIT_LOW, "HOST_VBUS_CONTROL");
	if (err) {
		printk(KERN_ERR "failed to request gpio_host_vbus\n");
		return;
	}

	s3c_gpio_setpull(pdata->gpio_host_vbus, S3C_GPIO_PULL_NONE);
	gpio_free(pdata->gpio_host_vbus);

	pdata->ohci_dev = &exynos4_device_ohci.dev;
	pdata->ehci_dev = &s5p_device_ehci.dev;
	pdata->s3c_hsotg_dev = &s3c_device_usb_hsotg.dev;

	s5p_usbswitch_set_platdata(pdata);
}
#endif

/*MMC SDIO*/
static struct s3c_sdhci_platdata twonav_hsmmc0_pdata __initdata = {
	.max_width		= 8,
	.host_caps	= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
			MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED,
	.cd_type		= S3C_SDHCI_CD_NONE,
};

/* SDCARD */
#if defined(CONFIG_TWONAV_AVENTURA) || defined(CONFIG_TWONAV_HORIZON)
static struct s3c_sdhci_platdata twonav_hsmmc2_pdata __initdata = {
	.max_width	= 4,
	.host_caps	= MMC_CAP_4_BIT_DATA |
			MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED,
	.cd_type	= S3C_SDHCI_CD_NONE,
};
#endif

/* WIFI SDIO */
static struct s3c_sdhci_platdata twonav_hsmmc3_pdata __initdata = {
	.max_width		= 4,
	.host_caps		= MMC_CAP_4_BIT_DATA
	| MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED,
	.cd_type		= S3C_SDHCI_CD_NONE,
};

static struct wl12xx_platform_data twonav_wl12xx_wlan_data __initdata = {
	.irq			= -1,
	.board_ref_clock	= WL12XX_REFCLOCK_26,
	.platform_quirks	= WL12XX_PLATFORM_QUIRK_EDGE_IRQ,
};

/* DWMMC */
/*
static int twonav_dwmci_get_bus_wd(u32 slot_id)
{
       return 8;
}
*/

/*
static int twonav_dwmci_init(u32 slot_id, irq_handler_t handler, void *data)
{
       return 0;
}
*/

/*
static struct dw_mci_board twonav_dwmci_pdata = {
	.num_slots			= 1,
	.quirks				= DW_MCI_QUIRK_BROKEN_CARD_DETECTION | DW_MCI_QUIRK_HIGHSPEED,
	.caps				= MMC_CAP_UHS_DDR50 | MMC_CAP_1_8V_DDR | MMC_CAP_8_BIT_DATA | MMC_CAP_CMD23,
	.fifo_depth			= 0x80,
	.bus_hz				= 104 * 1000 * 1000,
	.detect_delay_ms	= 200,
	.init				= twonav_dwmci_init,
	.get_bus_wd			= twonav_dwmci_get_bus_wd,
	.cfg_gpio			= exynos4_setup_dwmci_cfg_gpio,
};
*/

static struct resource tmu_resource[] = {
	[0] = {
		.start = EXYNOS4_PA_TMU,
		.end = EXYNOS4_PA_TMU + 0x0100,
		.flags = IORESOURCE_MEM,
	},
	[1] = { 
		.start = EXYNOS4_IRQ_TMU_TRIG0,
		.end = EXYNOS4_IRQ_TMU_TRIG0,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device twonav_tmu = {
	.id = -1,
	.name = "exynos5250-tmu",
	.num_resources = ARRAY_SIZE(tmu_resource),
	.resource = tmu_resource,
};


#if defined(CONFIG_LCD_T55149GD030J)

static int lcd_power_on(struct lcd_device *ld, int enable)
{	
	
	if (enable) {
		gpio_set_value(EXYNOS4X12_GPM1(5),1);
	} else {
		gpio_set_value(EXYNOS4X12_GPM1(5),0);
	}

	return 1;
}


static int lcd_cfg_gpio(void)
{
	int err = 0;
	
	printk("lcd_cfg_gpio()***!!!!**********\n");	
	/*Power control*/
	gpio_free(EXYNOS4_GPD0(2));
	gpio_request_one(EXYNOS4_GPD0(2), GPIOF_OUT_INIT_HIGH, "BACKLIGHT");
	gpio_free(EXYNOS4_GPD0(2));

	/* LCD _CS */
	gpio_free(EXYNOS4_GPB(5));
	err=gpio_request_one(EXYNOS4_GPB(5), GPIOF_OUT_INIT_HIGH, "GPB");	
	if (err) {
		printk(KERN_ERR "failed to request GPC1 for "
				"lcd SPI CS control\n");
		return err;
	}
	s3c_gpio_cfgpin(EXYNOS4_GPB(5), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(EXYNOS4_GPB(5), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4_GPB(5));
	gpio_set_value(EXYNOS4_GPB(5), 1);
	
	
	
	/* LCD_SCLK */
	gpio_free(EXYNOS4_GPB(4));
	err=gpio_request_one(EXYNOS4_GPB(4), GPIOF_OUT_INIT_HIGH, "GPB");	
	if (err) {
		printk(KERN_ERR "failed to request GPC1 for "
				"lcd SPI CLK control\n");
		return err;
	}
	s3c_gpio_cfgpin(EXYNOS4_GPB(4), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(EXYNOS4_GPB(4), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4_GPB(4));
	gpio_set_value(EXYNOS4_GPB(4), 1);
	
	
	/* LCD_SDI */
	gpio_free(EXYNOS4_GPB(7));
	err=gpio_request_one(EXYNOS4_GPB(7), GPIOF_OUT_INIT_HIGH, "GPB");	
	if (err) {
		printk(KERN_ERR "failed to request GPC1 for "
				"lcd SPI SDI control\n");
		return err;
	}
	s3c_gpio_cfgpin(EXYNOS4_GPB(7), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(EXYNOS4_GPB(7), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4_GPB(7));
	gpio_set_value(EXYNOS4_GPB(7), 1);
	

	gpio_free(EXYNOS4_GPC1(2));
	err = gpio_request_one(EXYNOS4_GPC1(2), GPIOF_OUT_INIT_HIGH, "GPC1");
	if (err) {
		printk(KERN_ERR "failed to request GPC1 for "
				"lcd reset control\n");
		return err;
	}
	
	s3c_gpio_cfgpin(EXYNOS4_GPC1(2), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(EXYNOS4_GPC1(2), S3C_GPIO_PULL_NONE);
	//gpio_free(EXYNOS4_GPC1(2));

	/* LCD FMARK */
	gpio_free(EXYNOS4_GPX1(5));
	gpio_request_one(EXYNOS4_GPX1(5), GPIOF_OUT_INIT_LOW, "GPX1");	
	//s3c_gpio_cfgpin(EXYNOS4_GPX1(5), S3C_GPIO_OUTPUT);
	s3c_gpio_cfgpin(EXYNOS4_GPX1(5), S3C_GPIO_INPUT);
	s3c_gpio_setpull(EXYNOS4_GPX1(5), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4_GPX1(5));
	//gpio_set_value(EXYNOS4_GPX1(5), 0);

	/* LCD IM1 */
	gpio_free(EXYNOS4_GPX1(7));
	gpio_request_one(EXYNOS4_GPX1(7), GPIOF_OUT_INIT_HIGH, "GPX1");	
	s3c_gpio_cfgpin(EXYNOS4_GPX1(7), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(EXYNOS4_GPX1(7), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4_GPX1(7));
	gpio_set_value(EXYNOS4_GPX1(7), 0); //SPI mode after reset

	/* LCD IM0 */
	gpio_free(EXYNOS4_GPX2(0));
	gpio_request_one(EXYNOS4_GPX2(0), GPIOF_OUT_INIT_HIGH, "GPX2");	
	s3c_gpio_cfgpin(EXYNOS4_GPX2(0), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(EXYNOS4_GPX2(0), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4_GPX2(0));
	gpio_set_value(EXYNOS4_GPX2(0), 0); //SPI ID
	

#if defined (CONFIG_TWONAV_HORIZON) || defined (CONFIG_TWONAV_AVENTURA) || defined(CONFIG_TWONAV_TRAIL)
	/* MCP73833 CHARGER GPM3CON(6) GPM4CON(3) */
	gpio_free(EXYNOS4X12_GPM3(6)); // STAT1 CHARGING
	s3c_gpio_cfgpin(EXYNOS4X12_GPM3(6), S3C_GPIO_INPUT);
	s3c_gpio_setpull(EXYNOS4X12_GPM3(6), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4X12_GPM3(6));

	gpio_free(EXYNOS4X12_GPM4(3)); // STAT2 CHARGED
	s3c_gpio_cfgpin(EXYNOS4X12_GPM4(3), S3C_GPIO_INPUT);
	s3c_gpio_setpull(EXYNOS4X12_GPM4(3), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4X12_GPM4(3));
#endif

	return 1;
}

static int reset_lcd(struct lcd_device *ld)
{
	int err = 0;
	
	lcd_cfg_gpio();
	
	printk("LCD reset***!!!!**********\n");
	
	gpio_set_value(EXYNOS4_GPC1(2), 1);
	mdelay(10);

	gpio_set_value(EXYNOS4_GPC1(2), 0);
	mdelay(10);

	gpio_set_value(EXYNOS4_GPC1(2), 1);

	//gpio_free(EXYNOS4_GPC1(2));

	return err;
}

static struct lcd_platform_data t55149gd030j_platform_data = {
	.reset			= reset_lcd,
	.power_on		= lcd_power_on,
	.lcd_enabled		= 0,
	.reset_delay		= 100,	/* 100ms */
};

#define		LCD_BUS_NUM	1
#define		DISPLAY_CS	EXYNOS4_GPB(5)
#define		DISPLAY_CLK	EXYNOS4_GPB(4)
#define		DISPLAY_SI	EXYNOS4_GPB(7)

// SPI1
#if 0
static struct s3c64xx_spi_csinfo spi1_csi = {
		.fb_delay = 0x2,
		.line = EXYNOS4_GPB(5),
};
#endif

static struct spi_board_info spi1_board_info[] __initdata = {
	{
		.modalias = "t55149gd030j-spi",
		.platform_data		= (void *)&t55149gd030j_platform_data,
		.max_speed_hz = 1200000, // 1.2 mhz
		.bus_num = LCD_BUS_NUM,
		.chip_select = 0,
		.mode = SPI_MODE_3,
		.controller_data =(void *)DISPLAY_CS,// &spi1_csi,
	},
};

static struct spi_gpio_platform_data t55149gd030j_spi_gpio_data = {
	.sck	= DISPLAY_CLK,
	.mosi	= DISPLAY_SI,
	.miso	= -1,
	.num_chipselect = 1,
};

static struct platform_device twonav_lcd_spi = {
	.name			= "spi_gpio",
	.id 			= LCD_BUS_NUM,
	.dev	= {
		.parent		= &s5p_device_fimd0.dev,
		.platform_data	= &t55149gd030j_spi_gpio_data,//&spi1_board_info,//
	},
};
#endif

static struct platform_device *twonav_devices[] __initdata = {
	&tps611xx,
	&s3c_device_hsmmc0,
#if defined(CONFIG_TWONAV_AVENTURA) || defined(CONFIG_TWONAV_HORIZON)
	&s3c_device_hsmmc2,
#endif
	&s3c_device_hsmmc3,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&gpio_device_i2c4,
#if defined(CONFIG_W1_MASTER_GPIO) || defined(CONFIG_W1_MASTER_GPIO_MODULE)
        &twonav_w1_device,
#endif
	&s3c_device_rtc,
	&s3c_device_usb_hsotg,
	&s3c_device_wdt,
	&s5p_device_ehci,
#ifdef CONFIG_SND_SAMSUNG_I2S
	&exynos4_device_i2s0,
#endif
	&s5p_device_fimc0,
	&s5p_device_fimc1,
	&s5p_device_fimc2,
	&s5p_device_fimc3,
	&s5p_device_fimc_md,
	&s5p_device_fimd0,
	&s5p_device_mfc,
	&s5p_device_mfc_l,
	&s5p_device_mfc_r,
	&s5p_device_g2d,
	&s5p_device_jpeg,
	&mali_gpu_device,
#if defined(CONFIG_S5P_DEV_TV)
	&s5p_device_hdmi,
	&s5p_device_cec,
	&s5p_device_i2c_hdmiphy,
	&s5p_device_mixer,
	&hdmi_fixed_voltage,
#endif
	&exynos4_device_ohci,
//	&exynos_device_dwmci,
//	&twonav_leds_gpio,
#if defined(CONFIG_LCD_T55149GD030J) && defined(CONFIG_DRM_EXYNOS_FIMD)
	&twonav_lcd_t55149gd030j,
#endif
	&twonav_gpio_keys,
	&samsung_asoc_idma,
#if defined(CONFIG_SND_SOC_HKDK_MAX98090)
	&hardkernel_audio_device,
#endif
#if defined(CONFIG_EXYNOS_THERMAL)
	&twonav_tmu,
#endif
#if defined(CONFIG_TWONAV_OTHERS_PWM_BL)
	&s3c_device_timer[1],
	&twonav_pwm_bl,
#endif

#if defined(CONFIG_LCD_T55149GD030J)
	&twonav_lcd_spi,
#else
	&s3c64xx_device_spi1,
#endif
#if defined(CONFIG_USB_EXYNOS_SWITCH)
	&s5p_device_usbswitch,
#endif
};

#if defined(CONFIG_S5P_DEV_TV)
static struct s5p_platform_cec hdmi_cec_data __initdata = {

};
#endif

static void __init twonav_map_io(void)
{
	clk_xusbxti.rate = 24000000;

	exynos_init_io(NULL, 0);
	s3c24xx_init_clocks(clk_xusbxti.rate);
	s3c24xx_init_uarts(twonav_uartcfgs, ARRAY_SIZE(twonav_uartcfgs));
}

static void __init twonav_reserve(void)
{
	s5p_mfc_reserve_mem(0x43000000, 64 << 20, 0x51000000, 64 << 20);
}

#if defined(CONFIG_S5P_DEV_TV)
/* I2C module and id for HDMIPHY */
static struct i2c_board_info hdmiphy_info = {
	I2C_BOARD_INFO("s5p_hdmiphy", 0x38),
};
#endif



static void __init twonav_gpio_init(void)
{
//	/* Peripheral power enable (P3V3) */
//	gpio_request_one(EXYNOS4_GPA1(1), GPIOF_OUT_INIT_HIGH, "p3v3_en");

	//Aventua/Trail ST
	#if defined(CONFIG_TWONAV_AVENTURA) || defined(CONFIG_TWONAV_TRAIL)
		gpio_free(EXYNOS4X12_GPM0(3));
		gpio_request_one(EXYNOS4X12_GPM0(3), GPIOF_OUT_INIT_HIGH, "AVENTURA_ST");
		gpio_free(EXYNOS4X12_GPM0(3));
	#endif

	/* Power on/off button */
	s3c_gpio_cfgpin(EXYNOS4X12_GPM3(7), S3C_GPIO_SFN(0xF));	/* VELO SIDE BUTTON TR POWERON */
	s3c_gpio_setpull(EXYNOS4X12_GPM3(7), S3C_GPIO_PULL_UP);
	
	/* TR/TL */
	gpio_request_one(EXYNOS4_GPF2(5), GPIOF_IN, "TL");
	s3c_gpio_cfgpin(EXYNOS4_GPF2(5), S3C_GPIO_INPUT );
	s3c_gpio_setpull(EXYNOS4_GPF2(5), S3C_GPIO_PULL_UP);
	gpio_free(EXYNOS4_GPF2(5));

	/* BR/BL */
	gpio_request_one(EXYNOS4_GPJ0(1), GPIOF_IN, "BR");
	s3c_gpio_cfgpin(EXYNOS4_GPJ0(1), S3C_GPIO_INPUT );
	s3c_gpio_setpull(EXYNOS4_GPJ0(1), S3C_GPIO_PULL_UP);
	gpio_free(EXYNOS4_GPJ0(1));

	gpio_request_one(EXYNOS4_GPJ1(1), GPIOF_IN, "BL");  //modificado
	s3c_gpio_cfgpin(EXYNOS4_GPJ1(1), S3C_GPIO_INPUT );
	s3c_gpio_setpull(EXYNOS4_GPJ1(1), S3C_GPIO_PULL_UP);
	gpio_free(EXYNOS4_GPJ1(1));

/*********************************************************************/
/*				WIFI MODULE CONFIGURATION									 */
/*********************************************************************/
	/* WLAN_EN */	
	gpio_request_one(EXYNOS4_GPJ1(4), GPIOF_OUT_INIT_LOW, "WLAN_EN");
        s3c_gpio_cfgpin(EXYNOS4_GPJ1(4), S3C_GPIO_OUTPUT );
        s3c_gpio_setpull(EXYNOS4_GPJ1(4), S3C_GPIO_PULL_NONE);
        gpio_free(EXYNOS4_GPJ1(4));
	/* BT_EN */	
	gpio_request_one(EXYNOS4_GPJ0(6), GPIOF_OUT_INIT_LOW, "BT_EN");
        s3c_gpio_cfgpin(EXYNOS4_GPJ0(6), S3C_GPIO_OUTPUT );
        s3c_gpio_setpull(EXYNOS4_GPJ0(6), S3C_GPIO_PULL_NONE);
        gpio_free(EXYNOS4_GPJ0(6));

	/* WLAN_IRQ */	
	gpio_request_one(EXYNOS4_GPX0(1), GPIOF_IN, "WLAN_IRQ");
    s3c_gpio_cfgpin(EXYNOS4_GPX0(1), S3C_GPIO_INPUT );
    s3c_gpio_setpull(EXYNOS4_GPX0(1), S3C_GPIO_PULL_DOWN);
	
/*********************************************************************/
/*				GPS CONFIGURATION									 */
/*********************************************************************/
	/* GPS PowerON/OFF */
	gpio_request_one(EXYNOS4X12_GPM4(2), GPIOF_OUT_INIT_LOW, "GPS_ONOFF");
		s3c_gpio_cfgpin(EXYNOS4X12_GPM4(2), S3C_GPIO_OUTPUT );
		s3c_gpio_setpull(EXYNOS4X12_GPM4(2), S3C_GPIO_PULL_NONE);
		gpio_free(EXYNOS4X12_GPM4(2));

	/* GPS Reset */
	gpio_request_one(EXYNOS4X12_GPM1(3), GPIOF_IN, "GPS_RESET");
		s3c_gpio_cfgpin(EXYNOS4X12_GPM1(3), S3C_GPIO_INPUT );
		s3c_gpio_setpull(EXYNOS4X12_GPM1(3), S3C_GPIO_PULL_NONE);
		gpio_free(EXYNOS4X12_GPM1(3));

	/* GPS Status */
	gpio_request_one(EXYNOS4X12_GPM4(5), GPIOF_IN, "GPS_STATUS");
		s3c_gpio_cfgpin(EXYNOS4X12_GPM4(5), S3C_GPIO_INPUT );
		s3c_gpio_setpull(EXYNOS4X12_GPM4(5), S3C_GPIO_PULL_NONE);
		gpio_free(EXYNOS4X12_GPM4(5));

/*********************************************************************/
/*				GPRS CONFIGURATION									 */
/*********************************************************************/
//	/* GPRS PWRKEY*/
//	gpio_request_one(EXYNOS4X12_GPM0(4), GPIOF_OUT_INIT_LOW, "GPRS_PWRKEY");
//		s3c_gpio_cfgpin(EXYNOS4X12_GPM0(4), S3C_GPIO_OUTPUT );
//		s3c_gpio_setpull(EXYNOS4X12_GPM0(4), S3C_GPIO_PULL_NONE);
//		gpio_free(EXYNOS4X12_GPM0(4));
//	/* GPRS PowerON/OFF */
//	gpio_request_one(EXYNOS4X12_GPM1(1), GPIOF_OUT_INIT_LOW, "GPRS_PON");
//		s3c_gpio_cfgpin(EXYNOS4X12_GPM1(1), S3C_GPIO_OUTPUT );
//		s3c_gpio_setpull(EXYNOS4X12_GPM1(1), S3C_GPIO_PULL_NONE);
//		gpio_free(EXYNOS4X12_GPM1(1));
//	/* GPRS STATUS*/
//	gpio_request_one(EXYNOS4X12_GPM1(6), GPIOF_IN, "GPRS_STATUS");
//		s3c_gpio_cfgpin(EXYNOS4X12_GPM1(6), S3C_GPIO_INPUT );
//		s3c_gpio_setpull(EXYNOS4X12_GPM1(6), S3C_GPIO_PULL_NONE);
//		gpio_free(EXYNOS4X12_GPM1(6));

/*********************************************************************/
/*				MAX44005 CONFIGURATION								 */
/********************************************************************
    gpio_request_one(EXYNOS4_GPX2(7), GPIOF_IN, "MAX44005_IRQ");
        s3c_gpio_cfgpin(EXYNOS4_GPX2(7), S3C_GPIO_SFN(0xF));
        s3c_gpio_setpull(EXYNOS4_GPX2(7), S3C_GPIO_PULL_NONE);
        gpio_free(EXYNOS4_GPX2(7));
*/

/*********************************************************************/
/*				MMC RESET CONFIGURATION								 */
/*********************************************************************/
    gpio_request_one(EXYNOS4_GPK0(2), GPIOF_OUT_INIT_HIGH, "MMC_RSTN");
        s3c_gpio_cfgpin(EXYNOS4_GPK0(2), S3C_GPIO_OUTPUT);
        s3c_gpio_setpull(EXYNOS4_GPK0(2), S3C_GPIO_PULL_NONE);
        gpio_free(EXYNOS4_GPK0(2));

/*********************************************************************/
/*				BUTTONS CONFIGURATION								 */
/*********************************************************************/
	s3c_gpio_setpull(EXYNOS4X12_GPM3(7), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(EXYNOS4_GPF2(5), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(EXYNOS4_GPJ0(1), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(EXYNOS4_GPJ1(1), S3C_GPIO_PULL_UP);
}

static void twonav_power_off(void)
{
	pr_emerg("Bye...\n");

	writel(0x5200, S5P_PS_HOLD_CONTROL);
	while (1) {
		pr_emerg("%s : should not reach here!\n", __func__);
		msleep(1000);
	}
}


static void set_mmc_RST_n_value(int value) {
	int err = gpio_direction_output(EXYNOS4_GPK0(2), value);
	if(err){
		pr_emerg("gpio_direction_output(EXYNOS4_GPK0(2), %d) error: %d\n", value, err);
	}
	else {
		pr_emerg("gpio_direction_output(EXYNOS4_GPK0(2), %d) OK\n", value);
	}
}


static void mmc_reset(void) {
    // eMMC HW_RST -> GPIO 139
    int err = gpio_request(EXYNOS4_GPK0(2), "MMC_RSTN");
    if(err) {
    	pr_emerg("gpio_request(EXYNOS4_GPK0(2), \"MMC_RSTN\") error: %d\n", err);
    }
    else {
    	set_mmc_RST_n_value(0);
		msleep(10);
    	set_mmc_RST_n_value(1);
		msleep(250);
		gpio_free(EXYNOS4_GPK0(2));
    	
    	pr_info("pulse_mmc_reset success\n");
	}
}

static void set_rtc_alarm(void) {
	struct rtc_device * rtc_dev = alarmtimer_get_rtcdev();
	if(rtc_dev){
		struct rtc_wkalrm alarm;
		int err = rtc_read_time(rtc_dev, &alarm.time);
		if(err){
			pr_crit("Error while reading time\n");
		}
		else {
			const int REBOOT_DELAY_SEC = 2;
			unsigned long time = 0;
			rtc_tm_to_time(&alarm.time, &time);
			time += REBOOT_DELAY_SEC;
			rtc_time_to_tm(time, &alarm.time);
			alarm.enabled = 1;
			alarm.pending = 1;
			err = rtc_set_alarm(rtc_dev, &alarm);		
			if(err){
				pr_crit("Error while setting alarm\n");
			}
		}
	}
	else {
		pr_crit("Error while getting rtc device\n");
	}
}

static int twonav_reboot_notifier(struct notifier_block *this, unsigned long code, void *_cmd) {
	pr_info("twonav_device: Notifier called -> code: %d\n", code);

	__raw_writel(0, S5P_INFORM4);
	if (code == SYS_RESTART){
		mmc_reset();
		set_rtc_alarm();
		writel(0x5200, S5P_PS_HOLD_CONTROL);
	}
	
	return NOTIFY_DONE;
}	


static struct notifier_block twonav_reboot_notifier_nb = {
	.notifier_call = twonav_reboot_notifier,
};

static void __init twonav_machine_init(void)
{
	printk(KERN_INFO "device version: %s\n", device_version);

	twonav_gpio_init();

	/* Register power off function */
	pm_power_off = twonav_power_off;

	s3c_i2c0_set_platdata(NULL);
	i2c_register_board_info(0, twonav_i2c_devs0,
				ARRAY_SIZE(twonav_i2c_devs0));

	s3c_i2c1_set_platdata(NULL);

	i2c_register_board_info(1, twonav_i2c_devs1,
				ARRAY_SIZE(twonav_i2c_devs1));

	i2c_register_board_info(4, twonav_i2c_devs4,
				ARRAY_SIZE(twonav_i2c_devs4));
	
/*SDIO_HCI CONFIGURATION ARRAY*/
	s3c_sdhci0_set_platdata(&twonav_hsmmc0_pdata);
#if defined(CONFIG_TWONAV_AVENTURA) || defined(CONFIG_TWONAV_HORIZON)
	s3c_sdhci2_set_platdata(&twonav_hsmmc2_pdata);
#endif
	s3c_sdhci3_set_platdata(&twonav_hsmmc3_pdata);

//	exynos4_setup_dwmci_cfg_gpio(NULL, MMC_BUS_WIDTH_4);
//	exynos_dwmci_set_platdata(&twonav_dwmci_pdata);

	twonav_ehci_init();
	twonav_ohci_init();
	s3c_hsotg_set_platdata(&twonav_hsotg_pdata);

#ifdef CONFIG_USB_EXYNOS_SWITCH
	twonav_usbswitch_init();
#endif

	//s3c64xx_spi1_set_platdata(NULL, 0, 1);
	spi_register_board_info(spi1_board_info, ARRAY_SIZE(spi1_board_info));

#if defined(CONFIG_LCD_T55149GD030J) && !defined(CONFIG_TWONAV_OTHERS) && defined(CONFIG_DRM_EXYNOS_FIMD)
	s5p_device_fimd0.dev.platform_data = &drm_fimd_pdata;
	exynos4_fimd0_gpio_setup_24bpp();
#endif
	init_button_irqs();

	platform_add_devices(twonav_devices, ARRAY_SIZE(twonav_devices));

	register_reboot_notifier(&twonav_reboot_notifier_nb);

	/* WIFI PLATFORM DATA
	 * FIXME: when using backports compability, the platformdata is not set properly
	 * and all the configuration is directly set in the driver. Should be changed 
	 * so that the platform data is configured in this file	*/
	twonav_wl12xx_wlan_data.irq = gpio_to_irq(EXYNOS4_GPX0(1));
	printk("twonav_wl12xx_wlan_data.irq: %d\n",twonav_wl12xx_wlan_data.irq);
	wl12xx_set_platform_data(&twonav_wl12xx_wlan_data);

}

MACHINE_START(TWONAV, "TwoNav")
	/* Maintainer: Eric Bosch <ebosch@twonav.com> */
	.atag_offset	= 0x100,
	.smp		= smp_ops(exynos_smp_ops),
	.init_irq	= exynos4_init_irq,
	.init_early	= exynos_firmware_init,
	.map_io		= twonav_map_io,
	.handle_irq	= gic_handle_irq,
	.init_machine	= twonav_machine_init,
	.init_late	= exynos_init_late,
	.timer		= &exynos4_timer,
	.restart	= exynos4_restart,
	.reserve	= &twonav_reserve,
MACHINE_END

