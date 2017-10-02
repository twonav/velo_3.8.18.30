
/*
 *  Linux logo to be displayed on boot
 *
 *  Copyright (C) 1996 Larry Ewing (lewing@isc.tamu.edu)
 *  Copyright (C) 1996,1998 Jakub Jelinek (jj@sunsite.mff.cuni.cz)
 *  Copyright (C) 2001 Greg Banks <gnb@alphalink.com.au>
 *  Copyright (C) 2001 Jan-Benedict Glaw <jbglaw@lug-owl.de>
 *  Copyright (C) 2003 Geert Uytterhoeven <geert@linux-m68k.org>
 */

#include <linux/linux_logo.h>
#include <linux/stddef.h>
#include <linux/module.h>

#ifdef CONFIG_M68K
#include <asm/setup.h>
#endif

#ifdef CONFIG_MIPS
#include <asm/bootinfo.h>
#endif

static bool nologo;
module_param(nologo, bool, 0);
MODULE_PARM_DESC(nologo, "Disables startup logo");
extern char *device_model;
extern char *device_brand;

/* logo's are marked __initdata. Use __init_refok to tell
 * modpost that it is intended that this function uses data
 * marked __initdata.
 */
const struct linux_logo * __init_refok fb_find_logo(int depth)
{
	const struct linux_logo *logo = &logo_twonav_small_clut224;
	
	if (depth >= 8) {
		if((device_brand != NULL) && (device_brand[0] != '\0'))
		{
			if(strcmp(device_brand, "twonav")==0)
			{
				if((device_model != NULL) && (device_model[0] != '\0'))
				{
					if((strcmp(device_model, "velo")==0) || (strcmp(device_model, "horizon")==0))
					{
						/* TwoNav logo for small screens (Velo, Horizon) */
						logo = &logo_twonav_small_clut224;
					}
					else if((strcmp(device_model, "aventura")==0) || (strcmp(device_model, "trail")==0))
					{
						/* TwoNav logo for big screens (Aventura, trail) */
						logo = &logo_twonav_big_clut224;
					}
				}
			}
			else if(strcmp(device_brand, "os")==0)
			{
				if((device_model != NULL) && (device_model[0] != '\0'))
				{
					if((strcmp(device_model, "velo")==0) || (strcmp(device_model, "horizon")==0))
					{
						/* OS logo for small screens (Velo, Horizon) */
						logo = &logo_os_small_clut224;
					}
					else if((strcmp(device_model, "aventura")==0) || (strcmp(device_model, "trail")==0))
					{
						/* OS logo for big screens (Aventura, trail) */
						logo = &logo_os_big_clut224;
					}
				}
			}
			else if(strcmp(device_brand, "flasher")==0)
			{
				if((device_model != NULL) && (device_model[0] != '\0'))
				{
					if((strcmp(device_model, "base_small")==0))
					{
						/* Flasher logo for small screens (Velo, Horizon) */
						logo = &logo_flasher_small_clut224;
					}
					else if((strcmp(device_model, "base_big")==0))
					{
						/* Flasher logo for big screens (Aventura, trail) */
						logo = &logo_flasher_big_clut224;
					}
				}
			}
		}
	}
	return logo;
}
EXPORT_SYMBOL_GPL(fb_find_logo);
