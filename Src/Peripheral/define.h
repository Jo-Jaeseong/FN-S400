/*
 * define.h
 *
 *  Created on: May 27, 2019
 *      Author: monster
 */
extern int ret;

#ifndef PERIPHERAL_DEFINE_H_
#define PERIPHERAL_DEFINE_H_



//test time
#define TIME_FAN_TEST		5900
#define TIME_AIR_TEST		5900
#define TIME_SOL_TEST		200
#define TIME_PERI_TEST		29900	//->5분

#define TIME_SPRAY1_TEST	11900
#define TIME_SPRAY2_TEST	6000

#define TIME_CLEAN1		2900
#define TIME_CLEAN2		3000

#define TIME_GAP		100




//BASIC SETTINGS

#define MAX_MODULE_COUNT			3

#define NIDEC	1
#define HSC		0

#if HSC

// PWM Cycle (25 khz)
#define ConstantBlowerFanControlPwmCycle	1200

// PWM Max Percent
#define ConstantBlowerFanControlPwmMax	 	85
// PWM Min Percent
#define ConstantBlowerFanControlPwmMin		60//65
// PWM Sterile Percent
#define ConstantBlowerFanControlPwmMid		75//65
// Pan duty add percent
#define ConstantPanDudyPercent				1

#else

// PWM Cycle (25 khz)
#define ConstantBlowerFanControlPwmCycle	1200

// PWM Max Percent
#define ConstantBlowerFanControlPwmMax	75
// PWM Min Percent
#define ConstantBlowerFanControlPwmMin		45
// PWM Sterile Percent
#define ConstantBlowerFanControlPwmMid		50//65
// Pan duty add percent
#define ConstantPanDudyPercent				1

#endif

// Peristaltic Pump Cycle (3500 -> 21cc, 3675 -> 20cc, 7350 -> 10cc, 4900 -> 15cc)
// Peristaltic Pump Cycle (JIH 19680 -> 15cc)
#define ConstantPeristalticPump_15_PwmCycle	20000 //19680//20400
#define ConstantPeristalticPump_12_PwmCycle	25000 //19680//20400
#define ConstantPeristalticPump_9_PwmCycle	35050 //19680//20400

//#define ConstantPeristalticPumpPwmCycle	20800 //19680//20400

#define ConstantInjectionPerMinute		12
#define ConstantInjectionPerMinute2	3
#define ConstantInjectionPerCubic		6

// H2O2 센서 농도 보정치(ppm), 기본값 2ppm
#define ConstantH2O2SensorCalibration	2

#define ConstantCubic					100
#define ConstantCirculationTime			0

#define NORMAL_MODE	1
#define TEST_MODE	0


#if NORMAL_MODE
//select
//NORMAL_MODE
//TEST_MODE

// Low Temperature
#define ConstantLowerTemperature			49.8//49.7//47.7//47.5 //18
// Upper Temperature
#define ConstantUpperTemperature			50.2//50//48

// Cut off temperature
#define ConstantCutoffTemperature			80
// SafeTemperature
#define ConstantSafeTemperature			34

#define OverHeat_Temperature				80


#define ConstantPreHeatTime					5	// 1 minute.
#define SprayCentiTime							100
#define ConstantLineCleanTime				1  // 1 minute.
#define ConstantNozzleCleanTime			1
#define ConstantSterileTime					60	// 60 minute.
#define ScrubCentiTime							100  // 1 second
#define FinishCentiTime							100	// 1 second


#else

// Low Temperature
#define ConstantLowerTemperature			49.8//49.7//47.7//47.5 //18
// Upper Temperature
#define ConstantUpperTemperature			50.2//50//48
// Start Temperature
#define ConstantStartTemperature			25//49.7

// Cut off temperature
#define ConstantCutoffTemperature			80
// SafeTemperature
#define ConstantSafeTemperature			32

#define OverHeat_Temperature			80

#define PreHeatCentiTime				0.2 * 60 * 100
#define SprayCentiTime					100
#define SterileWaitCentiTime			0.1 * 60 * 100
#define SterileMiddleCentiTime			0.1 * 60 * 100
#define SterileEndCentiTime				0.1 * 60 * 100
#define ScrubCentiTime					100
#define FinishCentiTime					100

#endif


#endif /* PERIPHERAL_DEFINE_H_ */
