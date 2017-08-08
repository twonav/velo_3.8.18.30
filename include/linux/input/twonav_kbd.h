#ifndef _TWONAV_KBD_H
#define _TWONAV_KBD_H

/*
 * Copyright (c) 2017 CompeGPS 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

struct twonav_kbd_platform_data {

	unsigned long poll_delay;
	unsigned long poll_period;
	unsigned int irq; /* irq number */
	unsigned char base; /*gpio first address*/

	int  (*get_pendown_state)(void);	
	int  (*init_platform_hw)(void);
	void (*exit_platform_hw)(void);
};

#endif /* _TWONAV_KBD_H */
