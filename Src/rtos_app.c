#include "rtos_app.h"

#include "main.h"
#include "Process.h"
#include "lcd.h"

extern volatile unsigned char UART_Receive_Flag, EndTimer_Flag, Timer_Half_1s_Flag,
						Timer_1s_Flag, Timer_DeliSecond_Flag, Timer_CentiSecond_Flag, Timer_1minute_Flag;

static volatile uint32_t rtos_event_flags;

void RTOS_App_Init(void)
{
	rtos_event_flags = 0;
}

void RTOS_Notify_Event(uint32_t event)
{
	rtos_event_flags |= event;
}

static uint32_t RTOS_FetchAndClearEvents(void)
{
	uint32_t events;

	__disable_irq();
	events = rtos_event_flags;
	rtos_event_flags = 0;
	__enable_irq();

	return events;
}

void RTOS_App_RunOnce(void)
{
	uint32_t events = RTOS_FetchAndClearEvents();

	if (UART_Receive_Flag) {
		events |= RTOS_EVENT_UART_RX;
	}
	if (Timer_CentiSecond_Flag) {
		events |= RTOS_EVENT_10MS;
	}
	if (Timer_DeliSecond_Flag) {
		events |= RTOS_EVENT_100MS;
	}
	if (Timer_Half_1s_Flag) {
		events |= RTOS_EVENT_500MS;
	}
	if (Timer_1s_Flag) {
		events |= RTOS_EVENT_1S;
	}
	if (Timer_1minute_Flag) {
		events |= RTOS_EVENT_1MIN;
	}
	if (EndTimer_Flag) {
		events |= RTOS_EVENT_END_TIMER;
	}

	if (events & RTOS_EVENT_UART_RX) {
		UART_Receive_Flag = 0;
		LCD_Process();
	}
	if (events & RTOS_EVENT_10MS) {
		Timer_CentiSecond_Flag = 0;
		CentiSecondProcess();
	}
	if (events & RTOS_EVENT_100MS) {
		Timer_DeliSecond_Flag = 0;
		DeliSecondProcess();
	}
	if (events & RTOS_EVENT_500MS) {
		Timer_Half_1s_Flag = 0;
		HalfSecondProcess();
	}
	if (events & RTOS_EVENT_1S) {
		Timer_1s_Flag = 0;
		OneSecondProcess();
	}
	if (events & RTOS_EVENT_1MIN) {
		Timer_1minute_Flag = 0;
		OneMinuteProcess();
	}
	if (events & RTOS_EVENT_END_TIMER) {
		ProcessEndTimer();
		ProcessTestEndTimer();
	}
}
