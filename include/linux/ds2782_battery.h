#ifndef __LINUX_DS2782_BATTERY_H
#define __LINUX_DS2782_BATTERY_H

struct ds278x_platform_data {
	int rsns;
	int gpio_enable_charger;
	int gpio_pg;
	int gpio_stat1;
	int gpio_stat2;
};

#endif
