/*
 * AirPump.c
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#include "main.h"

void TurnOnAirPump()
{
	// Turn on AirPump.
	HAL_GPIO_WritePin(AirPumpONOFF_GPIO_Port, AirPumpONOFF_Pin, GPIO_PIN_SET);
}

void TurnOffAirPump()
{
	// Turn on AirPump.
	HAL_GPIO_WritePin(AirPumpONOFF_GPIO_Port, AirPumpONOFF_Pin, GPIO_PIN_RESET);
}
