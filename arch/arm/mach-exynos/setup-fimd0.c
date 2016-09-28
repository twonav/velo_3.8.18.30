/* linux/arch/arm/mach-exynos4/setup-fimd0.c
 *
 * Copyright (c) 2009-2011 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com
 *
 * Base Exynos4 FIMD 0 configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/fb.h>
#include <linux/gpio.h>

#include <video/samsung_fimd.h>
#include <plat/gpio-cfg.h>

#include <mach/map.h>

void exynos4_fimd0_gpio_setup_24bpp(void)
{
	unsigned int reg;

	/**
		XV.VD2		->		GPF0CON6
		XV.VD3		->		GPF0CON7
		XV.VD4		->		GPF1CON0
		XV.VD5		->		GPF1CON1
		XV.VD6		->		GPF1CON2
		XV.VD7		->		GPF1CON3
		XV.VD12		->		GPF2CON0
		XV.VD10		->		GPF1CON6
		XV.VD14		->		GPF2CON2
		XV.VD11		->		GPF1CON7
		XV.VD19		->		GPF2CON7
		XV.VD13		->		GPF2CON1
		XV.VD21		->		GPF3CON1
		XV.VD15		->		GPF2CON3
		XV.VD22		->		GPF3CON2
		XV.VD18		->		GPF2CON6
		XV.VD23		->		GPF3CON3
		XV.VD20		->		GPF3CON0
	 * */

//	s3c_gpio_cfgrange_nopull(EXYNOS4_GPF0(6), 2, S3C_GPIO_SFN(2)); //GPF0CON6 - GPF0CON7
//	s3c_gpio_cfgrange_nopull(EXYNOS4_GPF1(0), 4, S3C_GPIO_SFN(2)); //GPF1CON0 - GPF1CON3
//	s3c_gpio_cfgpin(EXYNOS4_GPF2(0), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF2(0), S3C_GPIO_PULL_NONE); //GPF2CON0
//	s3c_gpio_cfgpin(EXYNOS4_GPF1(6), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF1(6), S3C_GPIO_PULL_NONE); //GPF1CON6
//	s3c_gpio_cfgpin(EXYNOS4_GPF2(2), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF2(2), S3C_GPIO_PULL_NONE); //GPF2CON2
//	s3c_gpio_cfgpin(EXYNOS4_GPF1(7), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF1(7), S3C_GPIO_PULL_NONE); //GPF1CON7
//	s3c_gpio_cfgpin(EXYNOS4_GPF2(7), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF2(7), S3C_GPIO_PULL_NONE); //GPF2CON7
//	s3c_gpio_cfgpin(EXYNOS4_GPF2(1), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF2(1), S3C_GPIO_PULL_NONE); //GPF2CON1
//	s3c_gpio_cfgpin(EXYNOS4_GPF3(1), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF3(1), S3C_GPIO_PULL_NONE); //GPF3CON1
//	s3c_gpio_cfgpin(EXYNOS4_GPF2(3), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF2(3), S3C_GPIO_PULL_NONE); //GPF2CON3
//	s3c_gpio_cfgpin(EXYNOS4_GPF3(2), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF3(2), S3C_GPIO_PULL_NONE); //GPF3CON2
//	s3c_gpio_cfgpin(EXYNOS4_GPF2(6), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF2(6), S3C_GPIO_PULL_NONE); //GPF2CON6
//	s3c_gpio_cfgpin(EXYNOS4_GPF3(3), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF3(3), S3C_GPIO_PULL_NONE); //GPF3CON3
//	s3c_gpio_cfgpin(EXYNOS4_GPF3(0), S3C_GPIO_SFN(0x2));
//	s3c_gpio_setpull(EXYNOS4_GPF3(0), S3C_GPIO_PULL_NONE); //GPF3CON0

	s3c_gpio_cfgrange_nopull(EXYNOS4_GPF0(0), 8, S3C_GPIO_SFN(2));
	s3c_gpio_cfgrange_nopull(EXYNOS4_GPF1(0), 8, S3C_GPIO_SFN(2));
	s3c_gpio_cfgrange_nopull(EXYNOS4_GPF2(0), 8, S3C_GPIO_SFN(2));
	s3c_gpio_cfgrange_nopull(EXYNOS4_GPF3(0), 4, S3C_GPIO_SFN(2));

	/*
	 * Set DISPLAY_CONTROL register for Display path selection.
	 *
	 * DISPLAY_CONTROL[1:0]
	 * ---------------------
	 *  00 | MIE
	 *  01 | MDINE
	 *  10 | FIMD : selected
	 *  11 | FIMD
	 */
	reg = __raw_readl(S3C_VA_SYS + 0x0210);
	reg |= (1 << 1);
	__raw_writel(reg, S3C_VA_SYS + 0x0210);
}
