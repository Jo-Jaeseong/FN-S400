/*
 * Timer.c
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#include "main.h"
#include "Process.h"
#include "rtos_app.h"
#include "string.h"

unsigned int	ui1msCounter, ui100msCounter, uiRunningCounter, ui1minuteCounter;

volatile unsigned int	ui1sCounter;
volatile unsigned int	uiEndTimeCounter;

volatile unsigned char Timer_Half_1s_Flag, Timer_1s_Flag, UART_Receive_Flag, Timer_DeliSecond_Flag, Timer_CentiSecond_Flag, Timer_1minute_Flag,
				Running_Flag, EndTimer_Flag, ProcessWait_Flag;

unsigned char uart1_rx_data[20] = {0};

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

extern unsigned int uiWaitTime[5];
extern volatile unsigned int uiTotalTime, uiFinishTime;
extern unsigned char ProcessMode;
extern int PeristalticPumpOnOff_Flag,FinishTimeControl_Spary;

extern volatile int Test_Start_flag;
extern unsigned int TestTime;
extern unsigned int uireservetime;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM7){
		ui1msCounter++;
		if((ui1msCounter % 10) == 0){
			if(Running_Flag){
				Timer_CentiSecond_Flag = 1;
				RTOS_Notify_Event(RTOS_EVENT_10MS);
				uiTotalTime ++;
				if(EndTimer_Flag == 0){
					uiEndTimeCounter--;
					if(uiEndTimeCounter == 0){
						EndTimer_Flag = 1;
						RTOS_Notify_Event(RTOS_EVENT_END_TIMER);
					}
					if(uiWaitTime[ProcessMode] > 0){
						uiWaitTime[ProcessMode]--;
					}
					if(uireservetime>0){
						uireservetime--;
					}
					if(ProcessWait_Flag == 0){
						if(uiFinishTime > 0 && PeristalticPumpOnOff_Flag && FinishTimeControl_Spary)
							uiFinishTime--;
					}
				}
			}
			if(Test_Start_flag){
				if(EndTimer_Flag == 0){
					uiEndTimeCounter--;
					if(TestTime>0){
						TestTime--;
					}
					if(uiEndTimeCounter == 0){
						EndTimer_Flag = 1;
						RTOS_Notify_Event(RTOS_EVENT_END_TIMER);
					}
				}
			}

		}
		if((ui1msCounter % 100) == 0){
			Timer_DeliSecond_Flag = 1;
			RTOS_Notify_Event(RTOS_EVENT_100MS);
		}
		if(ui1msCounter == 500){
			Timer_Half_1s_Flag = 1;
			RTOS_Notify_Event(RTOS_EVENT_500MS);
		}
		if(ui1msCounter == 1000){
			ui1msCounter = 0;
			Timer_1s_Flag = 1;
			Timer_Half_1s_Flag = 1;
			RTOS_Notify_Event(RTOS_EVENT_1S | RTOS_EVENT_500MS);
			if(ui1sCounter == 59){
				Timer_1minute_Flag = 1;
				RTOS_Notify_Event(RTOS_EVENT_1MIN);
				ui1sCounter = 0;
			}
			else{
				ui1sCounter++;
			}
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if(huart->Instance == USART1){
    	UART_Receive_Flag = 1;
    	RTOS_Notify_Event(RTOS_EVENT_UART_RX);
        HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
    }
    else if(huart->Instance == USART3){
    }
    else if(huart->Instance == UART4){
    }
}
