/*
 * Heater.c
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */
#include "main.h"
#include "define.h"
#include "i2c.h"
#include "main.h"

extern float fModuleTemperature, fModuleHumidity;
extern float LowerTemperature, UpperTemperature;

void TurnOnHeater()
{
	HAL_GPIO_WritePin(HeaterOnOff1_GPIO_Port, HeaterOnOff1_Pin, GPIO_PIN_SET);
}

void TurnOffHeater()
{
	HAL_GPIO_WritePin(HeaterOnOff1_GPIO_Port, HeaterOnOff1_Pin, GPIO_PIN_RESET);
}

void AdjustHeaterControl(int channel)
{
//	GetTemperatureFan(channel);

	if (fModuleTemperature > UpperTemperature)
	{
		TurnOffHeater();
	}
	else if (fModuleTemperature < LowerTemperature)
	{
		TurnOnHeater();
	}
}
