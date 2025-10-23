/*
 * i2c.h
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#ifndef PERIPHERAL_I2C_H_
#define PERIPHERAL_I2C_H_

void GetTemperatureFan(int channel);
void I2CChannelSelect(int channel);
void I2CChannelRelease(int channel);
void OverHeatTempCheck(int channel);
void avgOverHeatTemp();
void InitTemperature();
void Module_Temp(int Channel);
static void I2C_ClearBusyFlagErratum1(I2C_HandleTypeDef* handle,
		uint32_t timeout);
static void I2C_ClearBusyFlagErratum(I2C_HandleTypeDef* handle,
		uint32_t timeout);

#endif /* PERIPHERAL_I2C_H_ */
