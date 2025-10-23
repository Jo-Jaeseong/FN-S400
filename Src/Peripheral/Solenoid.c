/*
 * Solenoid.c
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#include "main.h"

void TurnOnSolenoidFluid(void)
{
	HAL_GPIO_WritePin(SolenoidFluidPathAirPurge1_GPIO_Port, SolenoidFluidPathAirPurge1_Pin, GPIO_PIN_SET);
}

void TurnOffSolenoidFluid(void)
{
	HAL_GPIO_WritePin(SolenoidFluidPathAirPurge1_GPIO_Port, SolenoidFluidPathAirPurge1_Pin, GPIO_PIN_RESET);
}

void TurnOnSolenoidAir(void)
{
	HAL_GPIO_WritePin(SolenoidNozzleCleaning_GPIO_Port, SolenoidNozzleCleaning_Pin, GPIO_PIN_SET);
}

void TurnOffSolenoidAir(void)
{
	HAL_GPIO_WritePin(SolenoidNozzleCleaning_GPIO_Port, SolenoidNozzleCleaning_Pin, GPIO_PIN_RESET);
}
