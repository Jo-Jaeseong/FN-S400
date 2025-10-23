/*
 * max6675.h
 *
 *  Created on: May 25, 2020
 *      Author: CBT
 */

#ifndef PERIPHERAL_MAX6675_H_
#define PERIPHERAL_MAX6675_H_

extern SPI_HandleTypeDef hspi1;
#define MAX6675_SPI	&hspi1

#define MAX6675_OK		1
#define MAX6675_ERROR	0

uint8_t max6675ReadReg(uint16_t *reg, int Channel);

float max6675Temp(uint16_t reg);

#endif /* PERIPHERAL_MAX6675_H_ */
