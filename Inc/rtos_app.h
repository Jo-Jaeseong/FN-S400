#ifndef RTOS_APP_H
#define RTOS_APP_H

#include <stdint.h>

#define RTOS_EVENT_UART_RX      (1U << 0)
#define RTOS_EVENT_10MS         (1U << 1)
#define RTOS_EVENT_100MS        (1U << 2)
#define RTOS_EVENT_500MS        (1U << 3)
#define RTOS_EVENT_1S           (1U << 4)
#define RTOS_EVENT_1MIN         (1U << 5)
#define RTOS_EVENT_END_TIMER    (1U << 6)

void RTOS_App_Init(void);
void RTOS_App_RunOnce(void);
void RTOS_Notify_Event(uint32_t event);

#endif
