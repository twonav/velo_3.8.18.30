#ifndef twonav_ds2782_battery_h
#define twonav_ds2782_battery_h


#define DS2782_EEPROM_VELO_CONTROL_VALUE 			0x00 //0x60 
#define DS2782_EEPROM_VELO_AB_VALUE 					0x00 //0x61 
#define DS2782_EEPROM_VELO_AC_MSB_VALUE 				0x21 //0x62 
#define DS2782_EEPROM_VELO_AC_LSB_VALUE 				0x00 //0x63 
#define DS2782_EEPROM_VELO_VCHG_VALUE 				0xD7 //0x64 
#define DS2782_EEPROM_VELO_IMIN_VALUE 				0x0A //0x65
#define DS2782_EEPROM_VELO_VAE_VALUE 				0x9A //0x66 
#define DS2782_EEPROM_VELO_IAE_VALUE 				0x10 //0x67
#define DS2782_EEPROM_VELO_ActiveEmpty_VALUE 		0x08 //0x68
#define DS2782_EEPROM_VELO_RSNS_VALUE 				0x1F //0x69
#define DS2782_EEPROM_VELO_Full40_MSB_VALUE 			0x21 //0x6A
#define DS2782_EEPROM_VELO_Full40_LSB_VALUE 			0x00 //0x6B
#define DS2782_EEPROM_VELO_Full3040Slope_VALUE 		0x0F //0x6C
#define DS2782_EEPROM_VELO_Full2030Slope_VALUE 		0x1C //0x6D
#define DS2782_EEPROM_VELO_Full1020Slope_VALUE 		0x26 //0x6E
#define DS2782_EEPROM_VELO_Full0010Slope_VALUE 		0x27 //0x6F
#define DS2782_EEPROM_VELO_AE3040Slope_VALUE 		0x07 //0x70
#define DS2782_EEPROM_VELO_AE2030Slope_VALUE 		0x10 //0x71
#define DS2782_EEPROM_VELO_AE1020Slope_VALUE 		0x1E //0x72
#define DS2782_EEPROM_VELO_AE0010Slope_VALUE 		0x12 //0x73
#define DS2782_EEPROM_VELO_SE3040Slope_VALUE 		0x02 //0x74
#define DS2782_EEPROM_VELO_SE2030Slope_VALUE 		0x04 //0x75
#define DS2782_EEPROM_VELO_SE1020Slope_VALUE 		0x05 //0x76
#define DS2782_EEPROM_VELO_SE0010Slope_VALUE 		0x0A //0x77
#define DS2782_EEPROM_VELO_RSGAIN_MSB_VALUE 			0x04 //0x78
#define DS2782_EEPROM_VELO_RSGAIN_LSB_VALUE 			0x00 //0x79
#define DS2782_EEPROM_VELO_RSTC_VALUE 				0x00 //0x7A
#define DS2782_EEPROM_VELO_FRSGAIN_MSB_VALUE 		0x04 //0x7B
#define DS2782_EEPROM_VELO_FRSGAIN_LSB_VALUE 		0x1A //0x7C
#define DS2782_EEPROM_VELO_SlaveAddressConfig_VALUE 	0x68 //0x7E

#define DS2782_EEPROM_TRAIL_CONTROL_VALUE 			0x00 //0x60
#define DS2782_EEPROM_TRAIL_AB_VALUE 					0x00 //0x61
#define DS2782_EEPROM_TRAIL_AC_MSB_VALUE 				0x54 //0x62
#define DS2782_EEPROM_TRAIL_AC_LSB_VALUE 				0x00 //0x63
#define DS2782_EEPROM_TRAIL_VCHG_VALUE 				0xD6 //0x64
#define DS2782_EEPROM_TRAIL_IMIN_VALUE 				0x4D //0x65
#define DS2782_EEPROM_TRAIL_VAE_VALUE 				0x9A //0x66
#define DS2782_EEPROM_TRAIL_IAE_VALUE 				0x10 //0x67
#define DS2782_EEPROM_TRAIL_ActiveEmpty_VALUE 		0x08 //0x68
#define DS2782_EEPROM_TRAIL_RSNS_VALUE 				0x1F //0x69
#define DS2782_EEPROM_TRAIL_Full40_MSB_VALUE 			0x54 //0x6A
#define DS2782_EEPROM_TRAIL_Full40_LSB_VALUE 			0x00 //0x6B
#define DS2782_EEPROM_TRAIL_Full3040Slope_VALUE       0x0F //0x6C
#define DS2782_EEPROM_TRAIL_Full2030Slope_VALUE       0x1C //0x6D
#define DS2782_EEPROM_TRAIL_Full1020Slope_VALUE       0x26 //0x6E
#define DS2782_EEPROM_TRAIL_Full0010Slope_VALUE       0x27 //0x6F
#define DS2782_EEPROM_TRAIL_AE3040Slope_VALUE         0x06 //0x70
#define DS2782_EEPROM_TRAIL_AE2030Slope_VALUE         0x10 //0x71
#define DS2782_EEPROM_TRAIL_AE1020Slope_VALUE         0x1E //0x72
#define DS2782_EEPROM_TRAIL_AE0010Slope_VALUE         0x12 //0x73
#define DS2782_EEPROM_TRAIL_SE3040Slope_VALUE         0x02 //0x74
#define DS2782_EEPROM_TRAIL_SE2030Slope_VALUE         0x05 //0x75
#define DS2782_EEPROM_TRAIL_SE1020Slope_VALUE         0x05 //0x76
#define DS2782_EEPROM_TRAIL_SE0010Slope_VALUE         0x0B //0x77
#define DS2782_EEPROM_TRAIL_RSGAIN_MSB_VALUE 			0x04 //0x78
#define DS2782_EEPROM_TRAIL_RSGAIN_LSB_VALUE 			0x00 //0x79
#define DS2782_EEPROM_TRAIL_RSTC_VALUE 				0x00 //0x7A
#define DS2782_EEPROM_TRAIL_FRSGAIN_MSB_VALUE 		0x04 //0x7B
#define DS2782_EEPROM_TRAIL_FRSGAIN_LSB_VALUE 		0x1A //0x7C
#define DS2782_EEPROM_TRAIL_SlaveAddressConfig_VALUE 	0x68 //0x7E

#define DS2782_EEPROM_AVENTURA_CONTROL_VALUE 			0x00 //0x60
#define DS2782_EEPROM_AVENTURA_AB_VALUE 					0x00 //0x61
#define DS2782_EEPROM_AVENTURA_AC_MSB_VALUE 				0x64 //0x62
#define DS2782_EEPROM_AVENTURA_AC_LSB_VALUE 				0x00 //0x63
#define DS2782_EEPROM_AVENTURA_VCHG_VALUE 				0xD6 //0x64
#define DS2782_EEPROM_AVENTURA_IMIN_VALUE 				0x4D //0x65
#define DS2782_EEPROM_AVENTURA_VAE_VALUE 				0x9A //0x66
#define DS2782_EEPROM_AVENTURA_IAE_VALUE 				0x10 //0x67
#define DS2782_EEPROM_AVENTURA_ActiveEmpty_VALUE 		0x08 //0x68
#define DS2782_EEPROM_AVENTURA_RSNS_VALUE 				0x1F //0x69
#define DS2782_EEPROM_AVENTURA_Full40_MSB_VALUE 			0x64 //0x6A
#define DS2782_EEPROM_AVENTURA_Full40_LSB_VALUE 			0x00 //0x6B
#define DS2782_EEPROM_AVENTURA_Full3040Slope_VALUE       0x0F //0x6C
#define DS2782_EEPROM_AVENTURA_Full2030Slope_VALUE       0x1C //0x6D
#define DS2782_EEPROM_AVENTURA_Full1020Slope_VALUE       0x26 //0x6E
#define DS2782_EEPROM_AVENTURA_Full0010Slope_VALUE       0x27 //0x6F
#define DS2782_EEPROM_AVENTURA_AE3040Slope_VALUE         0x07 //0x70
#define DS2782_EEPROM_AVENTURA_AE2030Slope_VALUE         0x10 //0x71
#define DS2782_EEPROM_AVENTURA_AE1020Slope_VALUE         0x1D //0x72
#define DS2782_EEPROM_AVENTURA_AE0010Slope_VALUE         0x12 //0x73
#define DS2782_EEPROM_AVENTURA_SE3040Slope_VALUE         0x02 //0x74
#define DS2782_EEPROM_AVENTURA_SE2030Slope_VALUE         0x05 //0x75
#define DS2782_EEPROM_AVENTURA_SE1020Slope_VALUE         0x05 //0x76
#define DS2782_EEPROM_AVENTURA_SE0010Slope_VALUE         0x0A //0x77
#define DS2782_EEPROM_AVENTURA_RSGAIN_MSB_VALUE 			0x04 //0x78
#define DS2782_EEPROM_AVENTURA_RSGAIN_LSB_VALUE 			0x00 //0x79
#define DS2782_EEPROM_AVENTURA_RSTC_VALUE 				0x00 //0x7A
#define DS2782_EEPROM_AVENTURA_FRSGAIN_MSB_VALUE 		0x04 //0x7B
#define DS2782_EEPROM_AVENTURA_FRSGAIN_LSB_VALUE 		0x1A //0x7C
#define DS2782_EEPROM_AVENTURA_SlaveAddressConfig_VALUE 	0x68 //0x7E

#define DS2782_EEPROM_HORIZON_CONTROL_VALUE 			0x00 //0x60
#define DS2782_EEPROM_HORIZON_AB_VALUE 					0x00 //0x61
#define DS2782_EEPROM_HORIZON_AC_MSB_VALUE 				0x1E //0x62
#define DS2782_EEPROM_HORIZON_AC_LSB_VALUE 				0x00 //0x63
#define DS2782_EEPROM_HORIZON_VCHG_VALUE 				0xDB //0x64
#define DS2782_EEPROM_HORIZON_IMIN_VALUE 				0x40 //0x65
#define DS2782_EEPROM_HORIZON_VAE_VALUE 				0x9A//0x66
#define DS2782_EEPROM_HORIZON_IAE_VALUE 				0x10 //0x67
#define DS2782_EEPROM_HORIZON_ActiveEmpty_VALUE 		0x08 //0x68
#define DS2782_EEPROM_HORIZON_RSNS_VALUE 				0x1F //0x69
#define DS2782_EEPROM_HORIZON_Full40_MSB_VALUE 			0x1E //0x6A
#define DS2782_EEPROM_HORIZON_Full40_LSB_VALUE          0x00 //0x6B
#define DS2782_EEPROM_HORIZON_Full3040Slope_VALUE       0x0E //0x6C
#define DS2782_EEPROM_HORIZON_Full2030Slope_VALUE       0x1C //0x6D
#define DS2782_EEPROM_HORIZON_Full1020Slope_VALUE       0x25 //0x6E
#define DS2782_EEPROM_HORIZON_Full0010Slope_VALUE       0x27 //0x6F
#define DS2782_EEPROM_HORIZON_AE3040Slope_VALUE         0x07 //0x70
#define DS2782_EEPROM_HORIZON_AE2030Slope_VALUE 		0x10 //0x71
#define DS2782_EEPROM_HORIZON_AE1020Slope_VALUE 		0x1D //0x72
#define DS2782_EEPROM_HORIZON_AE0010Slope_VALUE 		0x13 //0x73
#define DS2782_EEPROM_HORIZON_SE3040Slope_VALUE 		0x02 //0x74
#define DS2782_EEPROM_HORIZON_SE2030Slope_VALUE 		0x04 //0x75
#define DS2782_EEPROM_HORIZON_SE1020Slope_VALUE 		0x04 //0x76
#define DS2782_EEPROM_HORIZON_SE0010Slope_VALUE 		0x0B //0x77
#define DS2782_EEPROM_HORIZON_RSGAIN_MSB_VALUE 			0x04 //0x78
#define DS2782_EEPROM_HORIZON_RSGAIN_LSB_VALUE 			0x00 //0x79
#define DS2782_EEPROM_HORIZON_RSTC_VALUE 				0x00 //0x7A
#define DS2782_EEPROM_HORIZON_FRSGAIN_MSB_VALUE 		0x04 //0x7B
#define DS2782_EEPROM_HORIZON_FRSGAIN_LSB_VALUE 		0x1A //0x7C
#define DS2782_EEPROM_HORIZON_SlaveAddressConfig_VALUE 	0x68 //0x7E

#define RECHARGABLE_AVENTURA_BATTERY_MAX_VOLTAGE 		4210000 // not used

struct twonav_ds_2782_eeprom_config {
	uint8_t control;
	uint8_t ab;
	uint8_t ac_msb;
	uint8_t ac_lsb;
	uint8_t vchg;
	uint8_t imin;
	uint8_t vae;
	uint8_t iae;
	uint8_t active_empty;
	uint8_t rsns;
	uint8_t full40_msb;
	uint8_t full40_lsb;
	uint8_t full3040slope;
	uint8_t full2030slope;
	uint8_t full1020slope;
	uint8_t full0010slope;
	uint8_t ae3040slope;
	uint8_t ae2030slope;
	uint8_t ae1020slope;
	uint8_t ae0010slope;	
	uint8_t se3040slope;	
	uint8_t se2030slope;	
	uint8_t se1020slope;	
	uint8_t se0010slope;	
	uint8_t	rsgain_msb; 	
	uint8_t rsgain_lsb;
	uint8_t rstc;				
	uint8_t frsgain_msb;
	uint8_t frsgain_lsb;
	uint8_t slave_addr_config;	
};

#endif