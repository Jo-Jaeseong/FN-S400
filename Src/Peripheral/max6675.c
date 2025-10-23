/*
 * max6675.c
 *
 *  Created on: May 25, 2020
 *      Author: CBT
 *  SPI 온도 센서
 */
#include "main.h"
#include "max6675.h"

uint8_t max6675ReadReg(uint16_t *reg, int Channel)
{
	HAL_StatusTypeDef hal_answer = HAL_ERROR;

	uint8_t temp[2] = {0};
	//__disable_irq();
	if(Channel==0){
		HAL_GPIO_WritePin(Tempsensor1_GPIO_Port, Tempsensor1_Pin, GPIO_PIN_RESET);
		hal_answer = HAL_SPI_Receive(MAX6675_SPI, temp, 2, 100);
		HAL_GPIO_WritePin(Tempsensor1_GPIO_Port, Tempsensor1_Pin, GPIO_PIN_SET);
	}

	//__enable_irq();
	if(hal_answer == HAL_OK)
	{
		if(temp[1] & 0x04) return MAX6675_ERROR;

		*reg = (uint16_t)(temp[1]>>3);
		*reg |=(uint16_t)(temp[0]<<5);

		return MAX6675_OK;
	}
	return MAX6675_ERROR;
}

float max6675Temp(uint16_t reg)
{
	return reg*0.25;
}

