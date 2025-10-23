/*
 * FanPump.c
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#include <process.h>
#include "main.h"
#include "define.h"
#include "i2c.h"
#include "FanPump.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;

extern uint16_t ccr, ccr_max;
extern float fModuleTemperature;

uint16_t BlowerFanControlPwmMax=0, BlowerFanControlPwmMin=0;
unsigned int AddPanDudyPercent = ConstantPanDudyPercent;
unsigned char FanControl_Error_Flag=0;
extern struct DeviceInfo_format DeviceInfo;

void InitFanPump(void)
{
	BlowerFanControlPwmMax = DeviceInfo.fan_high_speed;
	BlowerFanControlPwmMin = DeviceInfo.fan_low_speed;

	ccr = BlowerFanControlPwmMin;	// 30%


	SetFanPumpSpeedAllMin();
}

void ReadFanControl()
{
	int uiErrorCount = 0;

	while(HAL_GPIO_ReadPin(BlowerFanSignal1_GPIO_Port, BlowerFanSignal1_Pin)) {
		HAL_Delay(10);
		uiErrorCount++;
		if(uiErrorCount > 1000) {
			FanControl_Error_Flag = 1;
			return;
		}
	}
	uiErrorCount = 0;
	while(HAL_GPIO_ReadPin(BlowerFanSignal2_GPIO_Port, BlowerFanSignal2_Pin)) {
		HAL_Delay(10);
		uiErrorCount++;
		if(uiErrorCount > 1000) {
			FanControl_Error_Flag = 1;
			return;
		}
	}
	uiErrorCount = 0;
	while(HAL_GPIO_ReadPin(BlowerFanSignal3_GPIO_Port, BlowerFanSignal3_Pin)) {
		HAL_Delay(10);
		uiErrorCount++;
		if(uiErrorCount > 1000) {
			FanControl_Error_Flag = 1;
			return;
		}
	}
	uiErrorCount = 0;
	while(HAL_GPIO_ReadPin(BlowerFanSignal3_GPIO_Port, BlowerFanSignal3_Pin)) {
		HAL_Delay(10);
		uiErrorCount++;
		if(uiErrorCount > 1000) {
			FanControl_Error_Flag = 1;
			return;
		}
	}
}

void TurnOnFanPump(void)
{
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);	//startup BlowerFanControlPwm1 // channel 0
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);	//startup BlowerFanControlPwm2 // channel 1
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);	//startup BlowerFanControlPwm3 // channel 2
}

void TurnOffFanPump(void)
{
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);	//Stop BlowerFanControlPwm1 // channel 0
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);	//Stop BlowerFanControlPwm2 // channel 1
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);	//Stop BlowerFanControlPwm3 // channel 2
}

void AdjustBlowerFanControl(int addtemp){
	if (fModuleTemperature > (DeviceInfo.upper_temperature+addtemp)){
		SpeedUpFan();
	}
	else if (fModuleTemperature < (DeviceInfo.lower_temperature+addtemp)){
		SpeedDownFan();
	}
}

void SpeedUpFan(){
	if (ccr < BlowerFanControlPwmMax)
		ccr += AddPanDudyPercent;
	if (ccr > BlowerFanControlPwmMax)
		ccr = BlowerFanControlPwmMax;

	uint16_t fan_duty = (ccr_max) * ccr / 100 - 1;
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, fan_duty);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, fan_duty);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, fan_duty);
}

void SpeedDownFan(){
	if (ccr > AddPanDudyPercent)
		ccr -= AddPanDudyPercent;
	if (ccr < BlowerFanControlPwmMin)
		ccr = BlowerFanControlPwmMin;

	uint16_t fan_duty = (ccr_max) * ccr / 100 - 1;
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, fan_duty);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, fan_duty);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, fan_duty);

}

void SetPanSpeed(int iPercent){
	uint16_t fan_duty = (ccr_max) * iPercent / 100 - 1;
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, fan_duty);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, fan_duty);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, fan_duty);

}

void SetFanPumpSpeedAllMin(void){
	SetPanSpeed(BlowerFanControlPwmMin);
}

void SetFanPumpSpeedAllMid(void){
	SetPanSpeed((BlowerFanControlPwmMin+BlowerFanControlPwmMax)/2);

}


void SetFanPumpSpeedAllMax(void){
	SetPanSpeed(BlowerFanControlPwmMax);
}

void SetFanPumpSpeedAll(int percent){
	SetPanSpeed(percent);
}
