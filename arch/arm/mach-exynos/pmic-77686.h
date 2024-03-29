/* linux/arch/arm/mach-s5pv310/dev-pmic.c
 *
 * MAX77686 PMIC platform data.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/max77686.h>

#include <mach/gpio.h>

//-----------------------------------------------------------------------------------
// BUCK1 : VDD_MIF(1.0V) (estaba a 1.1 cambio 1)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply buck1_consumer_77686 =
	REGULATOR_SUPPLY("vdd_mif", NULL);

static struct regulator_init_data max77686_buck1_data = {
	.constraints = {
		.name		= "BUCK1 vdd_mif",
		.min_uV 	= 1000000,
		.max_uV		= 1000000,
		.always_on 	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1000000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies	= &buck1_consumer_77686,
};

//-----------------------------------------------------------------------------------
// BUCK2 : VDD_ARM(1.2V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply buck2_consumer_77686 =
	REGULATOR_SUPPLY("vdd_arm", NULL);

static struct regulator_init_data max77686_buck2_data = {
	.constraints = {
		.name		= "BUCK2 vdd_arm",
		.min_uV 	= 800000,
		.max_uV		= 1500000,
		.always_on 	= 1,
		.boot_on 	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
	},
	.num_consumer_supplies = 1,
	.consumer_supplies	= &buck2_consumer_77686,
};

//-----------------------------------------------------------------------------------
// BUCK3 : VDD_INT(1.0V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply buck3_consumer_77686 =
	REGULATOR_SUPPLY("vdd_int", NULL);

static struct regulator_init_data max77686_buck3_data = {
	.constraints = {
		.name		= "BUCK3 vdd_int",
		.min_uV 	= 1000000,
		.max_uV 	= 1125000,
		.always_on 	= 1,
		.boot_on 	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		.state_mem	= {
			.uV	= 1125000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies	= &buck3_consumer_77686,
};
//-----------------------------------------------------------------------------------
// BUCK4 : VDD_G3D(1.0V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply buck4_consumer = 
	REGULATOR_SUPPLY("vdd_g3d", NULL);

static struct regulator_init_data max77686_buck4_data = {
	.constraints = {
		.name		= "BUCK4 vdd_g3d",
		.min_uV 	= 850000,
		.max_uV 	= 1200000,
		.boot_on 	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled = 1,
			.uV	= 1000000,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies	= &buck4_consumer,
};

//-----------------------------------------------------------------------------------
// BUCK5 : VDDQ_CKEM1_2,VDDQ_E1,VDDQ_E2,VDDCA_E1,VDDCA_E2(1.2V)
//-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_buck5_data = {
	.constraints	= {
		.name		= "BUCK5 VDDQ_CKEM1_2",
		.min_uV		= 1200000,
		.max_uV		= 1200000,
		.always_on	= 1,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1200000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// BUCK6 : Input to LDO1,6,7,8,15(1.35V)
//-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_buck6_data = {
	.constraints	= {
		.name		= "BUCK6 1V35",
		.min_uV		= 1350000,
		.max_uV		= 1350000,
		.always_on	= 1,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1350000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// BUCK7 : Input to LDO 3,5,9,11,17,18,19,20(2.0V)
//-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_buck7_data = {
	.constraints	= {
		.name		= "BUCK7 2V0",
		.min_uV		= 2000000,
		.max_uV		= 2000000,
		.always_on	= 1,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 2000000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// BUCK8 : IO(2.85V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply buck8_consumer =
	REGULATOR_SUPPLY("vmmc", "dw_mmc");
static struct regulator_init_data max77686_buck8_data = {
	.constraints	= {
		.name		= "vddf_emmc_2V85",
		.min_uV		= 2850000,
		.max_uV		= 2850000,
		.always_on	= 1,
		.boot_on	= 1,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 3300000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// BUCK9 : BUCK 9 POWER (3V3)
//-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_buck9_data = {
	.constraints	= {
		.name		= "BUCK9 3V3",
		.min_uV		= 1200000,
		.max_uV		= 3300000,
		.always_on	= 1,
		.boot_on	= 1,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 3300000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// LDO1 : VDD_ALIVE(1.0V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo1_consumer_77686 =
	REGULATOR_SUPPLY("vdd_alive", NULL);

static struct regulator_init_data max77686_ldo1_data = {
	.constraints	= {
		.name		= "LDO1 VDD_ALIVE",
		.min_uV		= 1000000,
		.max_uV		= 1000000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1000000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo1_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO2 : VDDQ_M1,VDDQ_M2 (1.8V) (lo cambio a 1.2V)
//-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_ldo2_data = {
	.constraints	= {
		.name		= "LDO2 VDDQ_M1_1V8",
		.min_uV		= 1200000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS, //añado esto
		.state_mem	= {
			.uV		= 1200000,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// LDO3 : VDDQ_SBUS,VDDQ_SYS02,VDDQ_AUD,VDDQ_EXT....(1.8V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo3_consumer_77686 =
	REGULATOR_SUPPLY("vddq_aud", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo3_data = {
	.constraints	= {
		.name		= "LDO3 VDD_IO_1V8",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo3_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO4 : LCD_VCI_2V8 (2.8V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo4_consumer_77686 =
	REGULATOR_SUPPLY("vddq_mmc2", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo4_data = {
	.constraints	= {
		.name		= "LDO4 LCD_VCI_2V8",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 2800000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo4_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO5 : AUDIO_1V8 (1.8V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo5_consumer_77686 =
	REGULATOR_SUPPLY("vddq_mmc1", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo5_data = {
	.constraints	= {
		.name		= "LDO5 AUDIO_1V8",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.boot_on	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo5_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO6 : VDD10_MPLL (1.0V)
//-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_ldo6_data = {
	.constraints	= {
		.name		= "LDO6 VDD10_MPLL_1V0",
		.min_uV		= 1000000,
		.max_uV		= 1000000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1000000,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// LDO7 : VDD10_A/E/VPLL (1.0V) (cambio a 1.1V)
//-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_ldo7_data = {
	.constraints	= {
		.name		= "LDO7 VDD10_EPLL_1V0",
		.min_uV		= 1000000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | //añado esto
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1100000,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// LDO8 : VDD10_HDMI,VDD10_MIPI (1.0V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo8_consumer_77686[] = {
	REGULATOR_SUPPLY("vdd", "exynos4-hdmi"), //Not used. Can be deleted
	REGULATOR_SUPPLY("vdd_pll", "exynos4-hdmi"), //Not used. Can be deleted
	REGULATOR_SUPPLY("vdd8_mipi", NULL), //Not used. Can be deleted
};

static struct regulator_init_data max77686_ldo8_data = {
	.constraints	= {
		.name		= "LDO8 VDD10_MIPI_1V0",
		.min_uV		= 1000000,
		.max_uV		= 1000000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1000000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = ARRAY_SIZE(ldo8_consumer_77686),
	.consumer_supplies  = ldo8_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO9 : SENS_1V8 (1.9V)
//-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_ldo9_data = {
	.constraints	= {
		.name		= "SENS_1V8",
		.min_uV		= 1900000,
		.max_uV		= 1900000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1900000,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// LDO10 : VDD10_HDMI,VDD10_MIPI (1.8V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo10_consumer_77686[] = {
	REGULATOR_SUPPLY("vdd_osc", "exynos4-hdmi"), //Not used. Can be deleted
	REGULATOR_SUPPLY("vdd10_mipi", NULL), //Not used. Can be deleted
	REGULATOR_SUPPLY("vdd_tmu", NULL), //Not used. Can be deleted
};

static struct regulator_init_data max77686_ldo10_data = {
	.constraints	= {
		.name		= "LDO10 VDD18_MIPI_1V8",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.boot_on	= 1,
		.always_on	= 1,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = ARRAY_SIZE(ldo10_consumer_77686),
	.consumer_supplies  = ldo10_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO11 : VDD18_ABB1 (1.8V)
//-----------------------------------------------------------------------------------
static struct regulator_init_data __initdata max77686_ldo11_data = {
	.constraints	= {
		.name		= "LDO11 VDD18_ABB1_1V8",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled	= 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// LDO12 : VDD33_UOTG (3.3V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo12_consumer_77686 =
REGULATOR_SUPPLY("vusb_a", NULL);

static struct regulator_init_data max77686_ldo12_data = {
	.constraints = {
		.name		= "vdd_ldo12 range",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on	= 1,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies  = &ldo12_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO13 : eMMC.VDD_1V8 (1.8V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo13_consumer_77686 =
	REGULATOR_SUPPLY("vdd18_mipihsi", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo13_data = {
	.constraints	= {
		.name		= "LDO13 eMMC.VDD_1V8",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo13_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO14 : VDD18_TS/ADC (1.8V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo14_consumer_77686 =
	REGULATOR_SUPPLY("vdd18_adc", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo14_data = {
	.constraints	= {
		.name		= "LDO14 VDD18_ADC_1V8",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.boot_on	= 1,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies  = &ldo14_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO15 : VDD10_OTG/HSIC (1.0V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo15_consumer_77686 =
	REGULATOR_SUPPLY("vusb_d", NULL);

static struct regulator_init_data max77686_ldo15_data = {
	.constraints	= {
		.name		= "vdd_ldo15 range",
		.min_uV		= 1000000,
		.max_uV		= 1000000,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on	= 1,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies  = &ldo15_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO16 : VDD18_HSIC (1.8V)
//-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_ldo16_data = {
	.constraints	= {
		.name		= "LDO16 VDD18_HSIC",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// LDO17 : VDDQ_CAM (1.8V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo17_consumer_77686 =
	REGULATOR_SUPPLY("vddq_cam", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo17_data = {
	.constraints	= {
		.name		= "LDO17 VDDQ_CAM_1V8", //CHECK
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo17_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO18 : VDDQ_ISP (1.8V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo18_consumer_77686 =
	REGULATOR_SUPPLY("vddq_isp", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo18_data = {
	.constraints	= {
		.name		= "LDO18 VDDQ_ISP_1V8",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
#if defined(CONFIG_ODROID_U)||defined(CONFIG_ODROID_U2)
        .always_on  = 0,
#else
		.always_on	= 1,
#endif		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo18_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO19 : GPS.VDD (1.8V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo19_consumer_77686 =
	REGULATOR_SUPPLY("vt_cam", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo19_data = {
	.constraints	= {
		.name		= "LDO19 GPS.VDD_1V8",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo19_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO20 : EMMC_IO_1_8
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo20_consumer_77686 =
	REGULATOR_SUPPLY("vqmmc", "dw_mmc");

static struct regulator_init_data max77686_ldo20_data = {
	.constraints	= {
		.name		= "vddq_emmc_1V8",
		.min_uV		= 1800000,
		.max_uV		= 3000000,
		.always_on	= 1,
		.boot_on	= 1,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.enabled = 1,
			.disabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo20_consumer_77686,
};
//-----------------------------------------------------------------------------------
// LDO21 : TFLASH (2.8V)
//-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_ldo21_data = {
	.constraints	= {
		.name		= "LDO21 TFLASH_2V8",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 2800000,
			.enabled = 1,
		},
	},
};

//-----------------------------------------------------------------------------------
// LDO22 : Not used
//-----------------------------------------------------------------------------------

	static struct regulator_init_data max77686_ldo22_data = {
		.constraints	= {
			.name		= "LDO22 2V8",
			.min_uV		= 2800000,
			.max_uV		= 2800000,
			.apply_uV	= 0,
			.always_on	= 0,
			.boot_on	= 0,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS,
			.state_mem	= {
				.uV		= 2800000,
				.enabled = 0,
			},
		},
	};

//-----------------------------------------------------------------------------------
// LDO23 : TOUCH (2.8V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo23_consumer_77686 =
	REGULATOR_SUPPLY("vdd_buzzer", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo23_data = {
	.constraints	= {
		.name		= "LDO23 AUDIO.3V",
		.min_uV		= 3000000,
		.max_uV		= 3000000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 3000000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo23_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO24 : TOUCHLED (3.3V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo24_consumer_77686 =
	REGULATOR_SUPPLY("vdd_touchled", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo24_data = {
	.constraints	= {
		.name		= "LDO24 VDD_TOUCHLED_3V3", //CHECK
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 3300000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo24_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO25 : VDDQ_LCD (3.0V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo25_consumer_77686 =
	REGULATOR_SUPPLY("vddq_lcd", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo25_data = {
	.constraints	= {
		.name		= "LDO25 VDDQ_LCD_3V0", //CHECK
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo25_consumer_77686,
};

//-----------------------------------------------------------------------------------
// LDO26 : 3V.HIGH (3.0V)
//-----------------------------------------------------------------------------------
static struct regulator_consumer_supply ldo26_consumer_77686 =
	REGULATOR_SUPPLY("vdd_motor", NULL); //Not used. Can be deleted

static struct regulator_init_data max77686_ldo26_data = {
	.constraints	= {
		.name		= "LDO26 3V.HIGH_3V0",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo26_consumer_77686,
};

////-----------------------------------------------------------------------------------
////-----------------------------------------------------------------------------------
static struct regulator_init_data max77686_32KHz_AP_data = {
	.constraints	= {
		.name		= "EN32KHz AP",
		.always_on	= 1,
	},
};
static struct regulator_init_data max77686_32KHz_CP_data = {
	.constraints	= {
		.name		= "EN32KHz CP",
		.always_on	= 1,
	},
};


//-----------------------------------------------------------------------------------
// Regulator Init data
//-----------------------------------------------------------------------------------
static struct max77686_regulator_data __refdata max77686_regulators[] = {
	{ MAX77686_LDO1,		&max77686_ldo1_data },  		
	{ MAX77686_LDO2,     	&max77686_ldo2_data },  
	{ MAX77686_LDO3,     	&max77686_ldo3_data },  
	{ MAX77686_LDO4,     	&max77686_ldo4_data },  
	{ MAX77686_LDO5,     	&max77686_ldo5_data },  
	{ MAX77686_LDO6,     	&max77686_ldo6_data },  
	{ MAX77686_LDO7,     	&max77686_ldo7_data },
	{ MAX77686_LDO8,     	&max77686_ldo8_data },
	{ MAX77686_LDO9,     	&max77686_ldo9_data },
	{ MAX77686_LDO10,    	&max77686_ldo10_data },
	{ MAX77686_LDO11,    	&max77686_ldo11_data }, 
	{ MAX77686_LDO12,    	&max77686_ldo12_data }, 
	{ MAX77686_LDO13,		&max77686_ldo13_data },  
	{ MAX77686_LDO14,    	&max77686_ldo14_data },  
	{ MAX77686_LDO15,    	&max77686_ldo15_data },  
	{ MAX77686_LDO16,    	&max77686_ldo16_data },  
	{ MAX77686_LDO17,    	&max77686_ldo17_data },  
	{ MAX77686_LDO18,    	&max77686_ldo18_data },  
	{ MAX77686_LDO19,    	&max77686_ldo19_data },  
	{ MAX77686_LDO20,    	&max77686_ldo20_data },  
	{ MAX77686_LDO21,    	&max77686_ldo21_data },  
	{ MAX77686_LDO22,    	&max77686_ldo22_data },  
	{ MAX77686_LDO23,    	&max77686_ldo23_data },  
	{ MAX77686_LDO24,    	&max77686_ldo24_data },  
	{ MAX77686_LDO25,    	&max77686_ldo25_data },  
	{ MAX77686_LDO26,    	&max77686_ldo26_data },  

	{ MAX77686_BUCK1,    	&max77686_buck1_data },  
	{ MAX77686_BUCK2,    	&max77686_buck2_data }, 
	{ MAX77686_BUCK3,    	&max77686_buck3_data }, 
	{ MAX77686_BUCK4,    	&max77686_buck4_data }, 
	{ MAX77686_BUCK5,    	&max77686_buck5_data }, 
	{ MAX77686_BUCK6,		&max77686_buck6_data },  
	{ MAX77686_BUCK7,       &max77686_buck7_data },  
	{ MAX77686_BUCK8,       &max77686_buck8_data },  
	{ MAX77686_BUCK9,       &max77686_buck9_data },  

//	{ MAX77686_EN32KHZ_AP, 	&max77686_32KHz_AP_data },  
//	{ MAX77686_EN32KHZ_CP, 	&max77686_32KHz_CP_data },  
};

static struct max77686_platform_data exynos4_max77686_info = {
	.irq_gpio	= 	EXYNOS4_GPX3(2),
	.ono		=	EXYNOS4_GPX1(2),
	.num_regulators = ARRAY_SIZE(max77686_regulators),
	.regulators = max77686_regulators,
//
//	.wakeup = true,
//	.buck2_gpiodvs 	= false,
//	.buck3_gpiodvs	= false,
//	.buck4_gpiodvs 	= false,
//
//
//	.buck234_default_idx = 0x00,
//
//	.buck234_gpios[0]	= EXYNOS4_GPX2(3),
//	.buck234_gpios[1]	= EXYNOS4_GPX2(4),
//	.buck234_gpios[2]	= EXYNOS4_GPX2(5),
//	
//	.buck2_voltage[0] = 1300000,	/* 1.3V */
//	.buck2_voltage[1] = 1000000,	/* 1.0V */
//	.buck2_voltage[2] = 950000,	/* 0.95V */
//	.buck2_voltage[3] = 900000,	/* 0.9V */
//	.buck2_voltage[4] = 1000000,	/* 1.0V */
//	.buck2_voltage[5] = 1000000,	/* 1.0V */
//	.buck2_voltage[6] = 950000,	/* 0.95V */
//	.buck2_voltage[7] = 900000,	/* 0.9V */
//
//	.buck3_voltage[0] = 1037500,	/* 1.0375V */
//	.buck3_voltage[1] = 1000000,	/* 1.0V */
//	.buck3_voltage[2] = 950000,	/* 0.95V */
//	.buck3_voltage[3] = 900000,	/* 0.9V */
//	.buck3_voltage[4] = 1000000,	/* 1.0V */
//	.buck3_voltage[5] = 1000000,	/* 1.0V */
//	.buck3_voltage[6] = 950000,	/* 0.95V */
//	.buck3_voltage[7] = 900000,	/* 0.9V */
//
//	.buck4_voltage[0] = 1100000,	/* 1.1V */
//	.buck4_voltage[1] = 1000000,	/* 1.0V */
//	.buck4_voltage[2] = 950000,	/* 0.95V */
//	.buck4_voltage[3] = 900000,	/* 0.9V */
//	.buck4_voltage[4] = 1000000,	/* 1.0V */
//	.buck4_voltage[5] = 1000000,	/* 1.0V */
//	.buck4_voltage[6] = 950000,	/* 0.95V */
//	.buck4_voltage[7] = 900000,	/* 0.9V */
};


/* HDMI 5V by Dongjim Kim */

enum fixed_regulator_id {
	FIXED_REG_ID_HDMI_5V,
};


static struct regulator_consumer_supply __initdata hdmi_fixed_consumer[] = {
	REGULATOR_SUPPLY("hdmi-en", "exynos4-hdmi"),
};

static struct regulator_init_data __initdata hdmi_fixed_voltage_init_data = {
	.constraints	= {
		.name		= "hdmi_5v",
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = ARRAY_SIZE(hdmi_fixed_consumer),
	.consumer_supplies      = hdmi_fixed_consumer,
};

static struct fixed_voltage_config __initdata hdmi_fixed_voltage_config = {
	.supply_name	= "hdmi_en",
	.microvolts	= 5000000,
	.gpio		= 0,		/* FIXME : No GPIO candidated */
	.enable_high	= true,
	.init_data	= &hdmi_fixed_voltage_init_data,
};

static struct platform_device __refdata hdmi_fixed_voltage = {
	.name	= "reg-fixed-voltage",
	.id	= FIXED_REG_ID_HDMI_5V,
	.dev	= {
		.platform_data  = &hdmi_fixed_voltage_config,
	},
};
