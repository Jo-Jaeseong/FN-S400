/*
 * PeristalticPump.c
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#include "main.h"
#include "define.h"
#include "Process.h"

extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim8;

extern float fInjectionPerMinute;
extern struct DeviceInfo_format DeviceInfo;

extern float Temp_Peri1_15_Value, Temp_Peri1_12_Value, Temp_Peri1_9_Value;
extern float Temp_Peri2_15_Value, Temp_Peri2_12_Value, Temp_Peri2_9_Value;


void TurnOnPeristalticPump(void){
	// PeristalticPump On.
	HAL_GPIO_WritePin(PeristalticPump1_GPIO_Port, PeristalticPump1_Pin, GPIO_PIN_SET);
	HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_2);  //startup PeristalticPumpPwm1

	HAL_GPIO_WritePin(PeristalticPump2_GPIO_Port, PeristalticPump2_Pin, GPIO_PIN_SET);
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_1);  //startup PeristalticPumpPwm2
}

void TurnOffPeristalticPump(void){
	HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_2);  //Stop PeristalticPumpPwm1
	HAL_GPIO_WritePin(PeristalticPump1_GPIO_Port, PeristalticPump1_Pin, GPIO_PIN_RESET);

	HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_1);  //Stop PeristalticPumpPwm2
	HAL_GPIO_WritePin(PeristalticPump2_GPIO_Port, PeristalticPump2_Pin, GPIO_PIN_RESET);
}

void PeristalticSpeed(){	// 분당 분사량 조절
	TIM5->CNT=0;
	TIM8->CNT=0;

	//2Channel JIHPUMP
	if(fInjectionPerMinute==15){
		TIM5->ARR = DeviceInfo.Peri1_15_Value;
		TIM8->ARR = DeviceInfo.Peri2_15_Value*2;
	}
		//20100
	else if(fInjectionPerMinute==12){
		TIM5->ARR = DeviceInfo.Peri1_12_Value;
		TIM8->ARR = DeviceInfo.Peri2_12_Value*2;
	}
	else if(fInjectionPerMinute==9){
		TIM5->ARR = DeviceInfo.Peri1_9_Value;
		TIM8->ARR = DeviceInfo.Peri2_9_Value*2;
	}
	else{
		TIM5->ARR = DeviceInfo.Peri1_12_Value;
		TIM8->ARR = DeviceInfo.Peri2_12_Value*2;
	}
}


void Peri_15_Speed(){
	TIM5->CNT=0;
	TIM8->CNT=0;

	TIM5->ARR=Temp_Peri1_15_Value;
	TIM8->ARR=Temp_Peri2_15_Value*2;
}

void Peri_12_Speed(){
	TIM5->CNT=0;
	TIM8->CNT=0;

	TIM5->ARR=Temp_Peri1_12_Value;
	TIM8->ARR=Temp_Peri2_12_Value*2;
}

void Peri_9_Speed(){
	TIM5->CNT=0;
	TIM8->CNT=0;

	TIM5->ARR=Temp_Peri1_9_Value;
	TIM8->ARR=Temp_Peri2_9_Value*2;
}
