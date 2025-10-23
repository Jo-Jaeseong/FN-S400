/*
 * PeristalticPump.c
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#include "main.h"
#include "define.h"
#include "process.h"

extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim8;

extern float fInjectionPerMinute, fInjectionPerMinute2;
extern struct DeviceInfo_format DeviceInfo;

extern int PeristalticPumpCnt;

void TurnOnPeristalticPump(void){
	PeristalticPumpCnt++;
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
	if(DeviceInfo.device_version==8){
		if(fInjectionPerMinute2==5){
			TIM5->ARR=(550*100/DeviceInfo.peri1_speed)/5-1;
			TIM5->CCR2=((550*100/DeviceInfo.peri1_speed)/5/2);
			TIM8->ARR=(550*100/DeviceInfo.peri2_speed)/5-1;
			TIM8->CCR1=((550*100/DeviceInfo.peri2_speed)/5/2);
		}
		else if(fInjectionPerMinute2==4){
			TIM5->ARR=(550*100/DeviceInfo.peri1_speed)/4-1;
			TIM5->CCR2=((550*100/DeviceInfo.peri1_speed)/4/2);
			TIM8->ARR=(550*100/DeviceInfo.peri2_speed)/4-1;
			TIM8->CCR1=((550*100/DeviceInfo.peri2_speed)/4/2);
		}
		else if(fInjectionPerMinute2==3){
			TIM5->ARR=(550*100/DeviceInfo.peri1_speed)/3-1;
			TIM5->CCR2=((550*100/DeviceInfo.peri1_speed)/3/2);
			TIM8->ARR=(550*100/DeviceInfo.peri2_speed)/3-1;
			TIM8->CCR1=((550*100/DeviceInfo.peri2_speed)/3/2);
		}
	}
	else{
		if(fInjectionPerMinute==15){
			TIM5->ARR=(550*100/DeviceInfo.peri1_speed)/5-1;
			TIM5->CCR2=((550*100/DeviceInfo.peri1_speed)/5/2);
			TIM8->ARR=(550*100/DeviceInfo.peri2_speed)/5-1;
			TIM8->CCR1=((550*100/DeviceInfo.peri2_speed)/5/2);
		}
		else if(fInjectionPerMinute==12){
			TIM5->ARR=(550*100/DeviceInfo.peri1_speed)/4-1;
			TIM5->CCR2=((550*100/DeviceInfo.peri1_speed)/4/2);
			TIM8->ARR=(550*100/DeviceInfo.peri2_speed)/4-1;
			TIM8->CCR1=((550*100/DeviceInfo.peri2_speed)/4/2);
		}
		else if(fInjectionPerMinute==9){
			TIM5->ARR=(550*100/DeviceInfo.peri1_speed)/3-1;
			TIM5->CCR2=((550*100/DeviceInfo.peri1_speed)/3/2);
			TIM8->ARR=(550*100/DeviceInfo.peri2_speed)/3-1;
			TIM8->CCR1=((550*100/DeviceInfo.peri2_speed)/3/2);
		}
	}
}

//ARR 의 최대값 65535

/*
550	1ml
275	2ml
184	3ml
138	4ml
110	5ml
 */

void Peri_15_Speed(){
	TIM5->ARR=(550*100/DeviceInfo.peri1_speed)/5-1;
	TIM5->CCR2=((550*100/DeviceInfo.peri1_speed)/5/2)-1;
	TIM8->ARR=(550*100/DeviceInfo.peri2_speed)/5-1;
	TIM8->CCR1=((550*100/DeviceInfo.peri2_speed)/5/2)-1;
}

void Peri_12_Speed(){
	TIM5->ARR=(550*100/DeviceInfo.peri1_speed)/4-1;
	TIM5->CCR2=((550*100/DeviceInfo.peri1_speed)/4/2)-1;
	TIM8->ARR=(550*100/DeviceInfo.peri2_speed)/4-1;
	TIM8->CCR1=((550*100/DeviceInfo.peri2_speed)/4/2)-1;
}

void Peri_9_Speed(){
	TIM5->ARR=(550*100/DeviceInfo.peri1_speed)/3-1;
	TIM5->CCR2=((550*100/DeviceInfo.peri1_speed)/3/2)-1;
	TIM8->ARR=(550*100/DeviceInfo.peri2_speed)/3-1;
	TIM8->CCR1=((550*100/DeviceInfo.peri2_speed)/3/2)-1;
}
