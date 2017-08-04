/*
 * BNO055.h
 *
 *  Created on: Feb 18, 2017
 *      Author: Joey
 */

#ifndef BNO055_H_
#define BNO055_H_

//IMU address
#define IMU_ADDR			0x28

//Page 0
#define CHIP_ID				0x00
#define ACC_ID				0x01
#define MAG_ID				0x02
#define GYR_ID				0x03
#define SW_REV_ID_LSB		0x04
#define SW_REV_ID_MSB		0x05
#define BL_REV_ID			0x06
#define PAGE_ID				0x07
#define ACC_DATA_X_LSB		0x08
#define ACC_DATA_X_MSB		0x09
#define ACC_DATA_Y_LSB		0x0A
#define ACC_DATA_Y_MSB		0x0B
#define ACC_DATA_Z_LSB		0x0C
#define ACC_DATA_Z_MSB		0x0D
#define MAG_DATA_X_LSB		0x0E
#define MAG_DATA_X_MSB		0x0F
#define MAG_DATA_Y_LSB		0x10
#define MAG_DATA_Y_MSB		0x11
#define MAG_DATA_Z_LSB		0x12
#define MAG_DATA_Z_MSB		0x13
#define GYR_DATA_X_LSB		0x14
#define GYR_DATA_X_MSB		0x15
#define GYR_DATA_Y_LSB		0x16
#define GYR_DATA_Y_MSB		0x17
#define GYR_DATA_Z_LSB		0x18
#define GYR_DATA_Z_MSB		0x19
#define EUL_DATA_X_LSB		0x1A
#define EUL_DATA_X_MSB		0x1B
#define EUL_DATA_Y_LSB		0x1C
#define EUL_DATA_Y_MSB		0x1D
#define EUL_DATA_Z_LSB		0x1E
#define EUL_DATA_Z_MSB		0x1F
#define QUA_DATA_W_LSB		0x20
#define QUA_DATA_W_MSB		0x21
#define QUA_DATA_X_LSB		0x22
#define QUA_DATA_X_MSB		0x23
#define QUA_DATA_Y_LSB		0x24
#define QUA_DATA_Y_MSB		0x25
#define QUA_DATA_Z_LSB		0x26
#define QUA_DATA_Z_MSB		0x27
#define LIA_DATA_X_LSB		0x28
#define LIA_DATA_X_MSB		0x29
#define LIA_DATA_Y_LSB		0x2A
#define LIA_DATA_Y_MSB		0x2B
#define LIA_DATA_Z_LSB		0x2C
#define LIA_DATA_Z_MSB		0x2D
#define GRV_DATA_X_LSB		0x2E
#define GRV_DATA_X_MSB		0x2F
#define GRV_DATA_Y_LSB		0x30
#define GRV_DATA_Y_MSB		0x31
#define GRV_DATA_Z_LSB		0x32
#define GRV_DATA_Z_MSB		0x33
#define TEMP				0x34
#define CALIB_STAT			0x35
#define ST_RESULT			0x36
#define INT_STA				0x37
#define SYS_CLK_STATUS		0x38
#define SYS_STATUS			0x39
#define SYS_ERR				0x3A
#define UNIT_SEL			0x3B
//0x3C does not exist.
#define OPR_MODE			0x3D
#define PWR_MODE			0x3E
#define SYS_TRIGGER			0x3F
#define TEMP_SOURCE			0x40
#define AXIS_MAP_CONFIG		0x41
#define AXIS_MAP_SIGN		0x42
//0x43-0x54 does not exist.
#define ACC_OFFSET_X_LSB	0x55
#define ACC_OFFSET_X_MSB	0x56
#define ACC_OFFSET_Y_LSB	0x57
#define ACC_OFFSET_Y_MSB	0x58
#define ACC_OFFSET_Z_LSB	0x59
#define ACC_OFFSET_Z_MSB	0x5A
#define MAG_OFFSET_X_LSB	0x5B
#define MAG_OFFSET_X_MSB	0x5C
#define MAG_OFFSET_Y_LSB	0x5D
#define MAG_OFFSET_Y_MSB	0x5E
#define MAG_OFFSET_Z_LSB	0x5F
#define MAG_OFFSET_Z_MSB	0x60
#define GYR_OFFSET_X_LSB	0x61
#define GYR_OFFSET_X_MSB	0x62
#define GYR_OFFSET_Y_LSB	0x63
#define GYR_OFFSET_Y_MSB	0x64
#define GYR_OFFSET_Z_LSB	0x65
#define GYR_OFFSET_Z_MSB	0x66
#define ACC_RADIUS_LSB		0x67
#define ACC_RADIUS_MSB		0x68
#define MAG_RADIUS_LSB		0x69
#define MAG_RADIUS_MSB		0x6A

//Page 1
//#define PAGE_ID			0x07
#define ACC_CONFIG			0x08
#define MAG_CONFIG			0x09
#define GYR_CONFIG_0		0x0A
#define GYR_CONFIG_1		0x0B
#define ACC_SLEEP_CONFIG	0x0C
#define GYR_SLEEP_CONFIG	0x0D
//0x0E does not exist.
#define INT_MSK				0x0F
#define INT_EN				0x10
#define ACC_AM_THRES		0x11
#define ACC_INT_SETTINGS	0x12
#define ACC_HG_DURATION		0x13
#define ACC_HG_THRES		0x14
#define ACC_NM_THRES		0x15
#define ACC_NM_SET			0x16
#define GYR_INT_SETTING		0x17
#define GYR_HR_X_SET		0x18
#define GYR_DUR_X			0x19
#define GYR_HR_Y_SET		0x1A
#define GYR_DUR_Y			0x1B
#define GYR_HR_Z_SET		0x1C
#define GYR_DUR_Z			0x1D
#define GYR_AM_THRES		0x1E
#define GYR_AM_SET			0x1F
//0x20-0x4F does not exist.
#define UNIQUE_ID			0x50

//UNIT_SEL values
//Acceleration
#define MetersPerSec2		0<<0
#define MilliG				1<<0
//Angular rate
#define DegsPerSec			0<<1
#define RadsPerSec			1<<1
//Euler angles
#define Euler_Deg			0<<2
#define Euler_Rad			1<<2
//Temperature
#define DegreesC			0<<4
#define DegreesF			1<<4
//HID operating system
#define Windows				0<<7
#define Android				1<<7

//OPR_MODE values
//Non-Fusion modes
#define CONFIGMODE			0b0000
#define ACCONLY				0b0001
#define MAGONLY				0b0010
#define GYROONLY			0b0011
#define ACCMAG				0b0100
#define ACCGYRO				0b0101
#define MAGGYRO				0b0110
#define AMG					0b0111
//Fusion modes
#define IMU					0b1000
#define COMPASS				0b1001
#define M4G					0b1010
#define NDOF_FMC_OFF		0b1011
#define NDOF				0b1100

//PWR_MODE values
#define NORM_PWR_MODE		0b00
#define LOW_PWR_MODE		0b01
#define SUSPEND_PWR_MODE	0b10

//TEMP_SOURCE values
#define ACC_TEMP			0b00
#define GYR_TEMP			0b01

//AXIS_MAP_CONFIG values
#define X_TO_X				0b00<<0
#define X_TO_Y				0b01<<0
#define X_TO_Z				0b10<<0
#define Y_TO_X				0b00<<2
#define Y_TO_Y				0b01<<2
#define Y_TO_Z				0b10<<2
#define Z_TO_X				0b00<<4
#define Z_TO_Y				0b01<<4
#define Z_TO_Z				0b10<<4

//AXIS_MAP_SIGN values
#define NEG_X_AXIS			1<<2
#define NEG_Y_AXIS			1<<1
#define NEG_Z_AXIS			1<<0

#define POS_X_AXIS			0<<2
#define POS_Y_AXIS			0<<1
#define POS_Z_AXIS			0<<0

//ACC_CONFIG values
//Max Gs
#define ACC_Set_Gs			0
//Bandwidth in Hz
#define ACC_Bandwidth		2
//Operation mode.
#define ACC_NORMAL			0b000<<5
#define ACC_SUSPEND			0b001<<5
#define ACC_LOW_POWER_1		0b010<<5
#define ACC_STANDBY			0b011<<5
#define ACC_LOW_POWER_2		0b100<<5
#define ACC_DEEP_SUSPEND	0b101<<5

//MAG_CONFIG values
//Data output rate in Hz
#define MAG_Data_Out_Rate	0
//Operation mode
#define MAG_LOW_PWR			0b00<<3
#define MAG_REGULAR			0b01<<3
#define MAG_ENHANCED_REGLR	0b10<<3
#define MAG_HIGH_ACCURACY	0b11<<3

//GYR_CONFIG registers values
//Range in degrees per second.
#define GYR_Range			0
//Bandwidth in Hz
#define GYR_Bandwidth		3
//Gyro operation mode
#define GYR_NORMAL			0b000
#define GYR_FAST_PWR_UP		0b001
#define GYR_DEEP_SUSPEND	0b010
#define GYR_SUSPEND			0b011
#define GYR_AVNCD_PWR_SAVE	0b100

//ACC_INT_SETTINGS
#define ACC_INT_X_AXIS		1<<2
#define	ACC_INT_Y_AXIS		1<<3
#define	ACC_INT_Z_AXIS		1<<4
#define ACC_HG_X_AXIS		1<<5
#define ACC_HG_Y_AXIS		1<<6
#define ACC_HG_Z_AXIS		1<<7

//ACC_NM_SET values
#define NO_MOTION			1<<0
#define SLOW_MOTION			0<<0
#define ACC_INT_DURATION	1

//GYR_INT_SETTING values
#define GYR_AM_INT_X		1<<0
#define GYR_AM_INT_Y		1<<1
#define GYR_AM_INT_Z		1<<2
#define GYR_HR_INT_X		1<<3
#define GYR_HR_INT_Y		1<<4
#define GYR_HR_INT_Z		1<<5
#define GYR_AM_FILTERED		0<<6
#define GYR_AM_UNFILTERED	1<<6
#define GYR_HR_FILTERED 	0<<7
#define GYR_HR_UNFILTERED	1<<7

//GYR_HR_X_SET
#define GYR_HR_THRESHOLD	0
#define GYR_HR_HYSTERESIS	5

//GYR_AM_THRES
#define GYR_AM_THRESHOLD	0

//GYR_AM_SET
#define GYR_AM_SS			0
#define GYR_AM_AD			2

//SYS_TRIGGER values
#define INTERNAL_OSC		0<<7
#define EXTERNAL_OSC		1<<7
#define RST_INT				1<<6
#define RST_SYS				1<<5
#define SELF_TEST			1<<0

#endif /* BNO055_H_ */
