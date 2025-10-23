/*
 * Scrubber.c
 *
 *  Created on: 2019. 8. 23.
 *      Author: CBT
 */


#include "main.h"

void TurnOnScrubber()
{
	// Turn on Scrubber
	HAL_GPIO_WritePin(ScrubberONOFF_GPIO_Port, ScrubberONOFF_Pin, GPIO_PIN_SET);
}

void TurnOffScrubber()
{
	// Turn on Scrubber
	HAL_GPIO_WritePin(ScrubberONOFF_GPIO_Port, ScrubberONOFF_Pin, GPIO_PIN_RESET);
}
