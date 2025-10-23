/*
 * Heater.c
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */
#include "main.h"
#include "define.h"
#include "i2c.h"
#include "process.h"
#include "main.h"

extern float fModuleTemperature, fModuleHumidity;
extern struct DeviceInfo_format DeviceInfo;

void TurnOnHeater()
{
	HAL_GPIO_WritePin(HeaterOnOff1_GPIO_Port, HeaterOnOff1_Pin, GPIO_PIN_SET);
}

void TurnOffHeater()
{
	HAL_GPIO_WritePin(HeaterOnOff1_GPIO_Port, HeaterOnOff1_Pin, GPIO_PIN_RESET);
}

void AdjustHeaterControl(int addtemp)
{
//	GetTemperatureFan(channel);

	if (fModuleTemperature > (DeviceInfo.upper_temperature+addtemp))
	{
		TurnOffHeater();
	}
	else if (fModuleTemperature < (DeviceInfo.lower_temperature+addtemp))
	{
		TurnOnHeater();
	}
}
