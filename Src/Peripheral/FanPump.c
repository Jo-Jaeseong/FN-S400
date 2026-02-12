/*
 * FanPump.c
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#include "main.h"
#include "define.h"
#include "i2c.h"
#include "FanPump.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;

extern uint16_t ccr[5], ccr_max;
extern float fModuleTemperature;
extern float LowerTemperature, UpperTemperature;

uint16_t BlowerFanControlPwmMax[3]={}, BlowerFanControlPwmMin[3]={};
unsigned int AddPanDudyPercent = ConstantPanDudyPercent;
unsigned char FanControl_Error_Flag=0;

void InitFanPump(void)
{
	BlowerFanControlPwmMax[0] = ConstantBlowerFanControlPwmMax;
	BlowerFanControlPwmMax[1] = ConstantBlowerFanControlPwmMax;
	BlowerFanControlPwmMax[2] = ConstantBlowerFanControlPwmMax;

	BlowerFanControlPwmMin[0] = ConstantBlowerFanControlPwmMin;
	BlowerFanControlPwmMin[1] = ConstantBlowerFanControlPwmMin;
	BlowerFanControlPwmMin[2] = ConstantBlowerFanControlPwmMin;

	ccr[0] = BlowerFanControlPwmMin[0];	// 30%
	ccr[1] = BlowerFanControlPwmMin[1];	// 30%
	ccr[2] = BlowerFanControlPwmMin[2];	// 30%

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

void AdjustBlowerFanControl(int channel)
{
	//GetTemperatureFan(channel);

	if (fModuleTemperature > UpperTemperature)
	{
		SpeedUpFan(channel);
	}
	else if (fModuleTemperature < LowerTemperature)
	{
		SpeedDownFan(channel);
	}
}

void SpeedUpFan(int channel)
{
	if (ccr[channel] < BlowerFanControlPwmMax[channel])
		ccr[channel] += AddPanDudyPercent;
	if (ccr[channel] > BlowerFanControlPwmMax[channel])
		ccr[channel] = BlowerFanControlPwmMax[channel];

	uint16_t pan_duty = (ccr_max) * ccr[channel] / 100 - 1;

	switch (channel)
	{
	case 0:
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pan_duty);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pan_duty);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pan_duty);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pan_duty);	//������. �����ص� �ɵ�.
		break;
	case 1:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pan_duty);
		break;
	case 2:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pan_duty);
		break;
	case 3:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pan_duty);
		break;
	}
}

void SpeedDownFan(int channel)
{
	if (ccr[channel] > AddPanDudyPercent)
		ccr[channel] -= AddPanDudyPercent;
	if (ccr[channel] < BlowerFanControlPwmMin[channel])
		ccr[channel] = BlowerFanControlPwmMin[channel];

	uint16_t pan_duty = (ccr_max) * ccr[channel] / 100 - 1;

	switch (channel)
	{
	case 0:
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pan_duty);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pan_duty);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pan_duty);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pan_duty);	//������. �����ص� �ɵ�.
		break;
	case 1:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pan_duty);
		break;
	case 2:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pan_duty);
		break;
	case 3:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pan_duty);
		break;
	}
}

void SetPanSpeed(int channel, int iPercent)
{
	uint16_t pan_duty = (ccr_max) * iPercent / 100 - 1;
	switch (channel)
	{
	case 0:
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pan_duty);
		break;
	case 1:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pan_duty);
		break;
	case 2:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pan_duty);
		break;
	case 3:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pan_duty);	//������. �����ص� �ɵ�.
		break;
	}
}

void SetFanPumpSpeedAllMin(void)
{
	SetPanSpeed(0, BlowerFanControlPwmMin[0]);
	SetPanSpeed(1, BlowerFanControlPwmMin[1]);
	SetPanSpeed(2, BlowerFanControlPwmMin[2]);
}

void SetFanPumpSpeedAllMin2(void)
{
	SetPanSpeed(0, ConstantBlowerFanControlPwmMin);
	SetPanSpeed(1, ConstantBlowerFanControlPwmMin);
	SetPanSpeed(2, ConstantBlowerFanControlPwmMin);
}


void SetFanPumpSpeedAllMax(void)
{
	SetPanSpeed(0, BlowerFanControlPwmMax[0]);
	SetPanSpeed(1, BlowerFanControlPwmMax[1]);
	SetPanSpeed(2, BlowerFanControlPwmMax[2]);
}

void SetFanPumpSpeedAll(int percent){
	SetPanSpeed(0, percent);
	SetPanSpeed(1, percent);
	SetPanSpeed(2, percent);
}
