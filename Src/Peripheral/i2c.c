/*
 * i2c.c
 *
 *  Created on: 2019. 5. 13.
 *      Author: monster
 */

#include "main.h"
#include "define.h"
#include "i2c.h"
#include "process.h"
#include "lcd.h"
#include "LTE_Modem.h"
#include "max6675.h"

#define I2C3_SCL_Pin				SCL_TEMP_RH_Pin
#define I2C3_SCL_GPIO_Port 			SCL_TEMP_RH_GPIO_Port
#define I2C3_SDA_Pin 				SDA_TEMP_RH_Pin
#define I2C3_SDA_GPIO_Port 			SDA_TEMP_RH_GPIO_Port

#define I2C1_SCL_Pin				SCL_TEMP_RHB6_Pin
#define I2C1_SCL_GPIO_Port 			GPIOB
#define I2C1_SDA_Pin 				SDA_TEMP_RHB7_Pin
#define I2C1_SDA_GPIO_Port 			GPIOB

#define TRUE						1
#define FALSE						0

#define GET_TEMPERATURE_CMD			0xE3
#define GET_TEMPERATURE_CMD_NO_HOLD	0xF3
#define GET_HUMIDITY_CMD			0xE5
#define GET_HUMIDITY_CMD_NO_HOLD	0xF5

#define I2C_DELAY					10

float fBoardTemperature=0, fHumidity=0, fModuleHumidity=0, fModuleTemperature=0, fModuleTemperature2=0;
float fBoardTemperature_Max=0;
float fHumidity_Max=0;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c3;

unsigned char buffer[5];
unsigned int rawT, rawH;
unsigned char Temperature_Error_Flag=0;
float overheattemp=0;
int overheatcnt,overheatFlag=0;
int tempArr[10]={};
int tempIndex=0;

extern struct DeviceInfo_format DeviceInfo;

//SPI 온도 센서 관련
/*SPI 온습도 센서관련 변수*/

static uint8_t wait_for_gpio_state_timeout(GPIO_TypeDef *port, uint16_t pin,
		GPIO_PinState state, uint32_t timeout)
{
	uint32_t Tickstart = HAL_GetTick();
	uint8_t ret = TRUE;
	/* Wait until flag is set */
	for (; (state != HAL_GPIO_ReadPin(port, pin)) && (TRUE == ret);) {
		/* Check for the timeout */
		if (timeout != HAL_MAX_DELAY) {
			if ((timeout == 0U) || ((HAL_GetTick() - Tickstart) > timeout)) {
				ret = FALSE;
			} else {
			}
		}
		asm("nop");
	}
	return ret;
}

static void I2C_ClearBusyFlagErratum(I2C_HandleTypeDef* handle,
		uint32_t timeout)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// 1. Clear PE bit.
	CLEAR_BIT(handle->Instance->CR1, I2C_CR1_PE);

	//  2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
	HAL_I2C_DeInit(handle);

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;

	GPIO_InitStructure.Pin = I2C3_SCL_Pin;
	HAL_GPIO_Init(I2C3_SCL_GPIO_Port, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = I2C3_SDA_Pin;
	HAL_GPIO_Init(I2C3_SDA_GPIO_Port, &GPIO_InitStructure);

	// 3. Check SCL and SDA High level in GPIOx_IDR.
	HAL_GPIO_WritePin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_SET);

	wait_for_gpio_state_timeout(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_SET,
			timeout);
	wait_for_gpio_state_timeout(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_SET,
			timeout);

	// 4. Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
	HAL_GPIO_WritePin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_RESET);

	// 5. Check SDA Low level in GPIOx_IDR.
	wait_for_gpio_state_timeout(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin,
			GPIO_PIN_RESET, timeout);

	// 6. Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
	HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_RESET);

	// 7. Check SCL Low level in GPIOx_IDR.
	wait_for_gpio_state_timeout(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin,
			GPIO_PIN_RESET, timeout);

	// 8. Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
	HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_SET);

	// 9. Check SCL High level in GPIOx_IDR.
	wait_for_gpio_state_timeout(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_SET,
			timeout);

	// 10. Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to GPIOx_ODR).
	HAL_GPIO_WritePin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_SET);

	// 11. Check SDA High level in GPIOx_IDR.
	wait_for_gpio_state_timeout(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_SET,
			timeout);

	// 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Alternate = GPIO_AF4_I2C3;

	GPIO_InitStructure.Pin = I2C3_SCL_Pin;
	HAL_GPIO_Init(I2C3_SCL_GPIO_Port, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = I2C3_SDA_Pin;
	HAL_GPIO_Init(I2C3_SDA_GPIO_Port, &GPIO_InitStructure);

	// 13. Set SWRST bit in I2Cx_CR1 register.
	SET_BIT(handle->Instance->CR1, I2C_CR1_SWRST);
	asm("nop");

	/* 14. Clear SWRST bit in I2Cx_CR1 register. */
	CLEAR_BIT(handle->Instance->CR1, I2C_CR1_SWRST);
	asm("nop");

	/* 15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register */
	SET_BIT(handle->Instance->CR1, I2C_CR1_PE);
	asm("nop");

	// Call initialization function.
	HAL_I2C_Init(handle);
}

static void I2C_ClearBusyFlagErratum1(I2C_HandleTypeDef* handle,
		uint32_t timeout)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// 1. Clear PE bit.
	CLEAR_BIT(handle->Instance->CR1, I2C_CR1_PE);

	//  2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
	HAL_I2C_DeInit(handle);

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;

	GPIO_InitStructure.Pin = I2C1_SCL_Pin;
	HAL_GPIO_Init(I2C1_SCL_GPIO_Port, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = I2C1_SDA_Pin;
	HAL_GPIO_Init(I2C1_SDA_GPIO_Port, &GPIO_InitStructure);

	// 3. Check SCL and SDA High level in GPIOx_IDR.
	HAL_GPIO_WritePin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_SET);

	wait_for_gpio_state_timeout(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_SET,
			timeout);
	wait_for_gpio_state_timeout(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, GPIO_PIN_SET,
			timeout);

	// 4. Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
	HAL_GPIO_WritePin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, GPIO_PIN_RESET);

	// 5. Check SDA Low level in GPIOx_IDR.
	wait_for_gpio_state_timeout(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin,
			GPIO_PIN_RESET, timeout);

	// 6. Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
	HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_RESET);

	// 7. Check SCL Low level in GPIOx_IDR.
	wait_for_gpio_state_timeout(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin,
			GPIO_PIN_RESET, timeout);

	// 8. Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
	HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_SET);

	// 9. Check SCL High level in GPIOx_IDR.
	wait_for_gpio_state_timeout(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_SET,
			timeout);

	// 10. Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to GPIOx_ODR).
	HAL_GPIO_WritePin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, GPIO_PIN_SET);

	// 11. Check SDA High level in GPIOx_IDR.
	wait_for_gpio_state_timeout(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, GPIO_PIN_SET,
			timeout);

	// 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Alternate = GPIO_AF4_I2C1;

	GPIO_InitStructure.Pin = I2C1_SCL_Pin;
	HAL_GPIO_Init(I2C1_SCL_GPIO_Port, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = I2C1_SDA_Pin;
	HAL_GPIO_Init(I2C1_SDA_GPIO_Port, &GPIO_InitStructure);

	// 13. Set SWRST bit in I2Cx_CR1 register.
	SET_BIT(handle->Instance->CR1, I2C_CR1_SWRST);
	asm("nop");

	/* 14. Clear SWRST bit in I2Cx_CR1 register. */
	CLEAR_BIT(handle->Instance->CR1, I2C_CR1_SWRST);
	asm("nop");

	/* 15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register */
	SET_BIT(handle->Instance->CR1, I2C_CR1_PE);
	asm("nop");

	// Call initialization function.
	HAL_I2C_Init(handle);

}

void GetTemperatureFan(int channel)
{
/*
	// Read Temperature;
	//fBoardTemperature = 0;
	buffer[0] = GET_TEMPERATURE_CMD; //Pointer buffer
	if (HAL_I2C_Master_Transmit(&hi2c1, 0x40 << 1, buffer, 1, 100) != HAL_OK) {
		I2C_ClearBusyFlagErratum1(&hi2c1, 1000);
	} else {
		HAL_Delay(I2C_DELAY);
		if (HAL_I2C_Master_Receive(&hi2c1, 0x40 << 1, buffer, 2, 100)
				!= HAL_OK) {
			I2C_ClearBusyFlagErratum1(&hi2c1, 1000);
		} else {
			rawT = buffer[0] << 8 | buffer[1]; //combine 2 8-bit into 1 16bit
			fBoardTemperature = ((float) (rawT * 175) / 65536) - 47.0;
			if(fBoardTemperature>fBoardTemperature_Max)
							fBoardTemperature_Max=fBoardTemperature;	//Max Board Temperature
		}
	}

	//Trigger Humidity measurement buffer[0]=0x01;
	//fHumidity = 0;
	buffer[0] = GET_HUMIDITY_CMD; //Pointer buffer
	if (HAL_I2C_Master_Transmit(&hi2c1, 0x40 << 1, buffer, 1, 100) != HAL_OK) {
		I2C_ClearBusyFlagErratum1(&hi2c1, 1000);

	} else {
		HAL_Delay(I2C_DELAY);
		if (HAL_I2C_Master_Receive(&hi2c1, 0x40 << 1, buffer, 2, 100)
				!= HAL_OK) {
			I2C_ClearBusyFlagErratum1(&hi2c1, 1000);
		}
		rawH = buffer[0] << 8 | buffer[1]; //combine 2 8-bit into 1 16bit
		fHumidity = ((float) (rawH * 125) / 65536) - 6.0;
		if((fHumidity>=100||fHumidity<6)&&fBoardTemperature>24){
			fHumidity=100;
		}
		if(fHumidity>fHumidity_Max){
				fHumidity_Max=fHumidity;
		}
	}
	*/

	//ch1 selection & reading with hi2c3
	I2CChannelSelect(channel);

	//Trigger Temperature measurement
	//fModuleTemperature2 = 0;
	buffer[0] = GET_TEMPERATURE_CMD; //Pointer buffer
	if (HAL_I2C_Master_Transmit(&hi2c3, 0x40 << 1, buffer, 1, 10) != HAL_OK) {
		I2C_ClearBusyFlagErratum(&hi2c3, 1000);
	} else {
		HAL_Delay(I2C_DELAY);
		if (HAL_I2C_Master_Receive(&hi2c3, 0x40 << 1, buffer, 2, 10)
				!= HAL_OK) {
			I2C_ClearBusyFlagErratum(&hi2c3, 1000);
		}

		rawT = buffer[0] << 8 | buffer[1]; //combine 2 8-bit into 1 16bit
		fBoardTemperature = ((float) (rawT * 175) / 65536) - 47.0;
		if(fBoardTemperature>fBoardTemperature_Max){
			fBoardTemperature_Max=fBoardTemperature;	//Max Board Temperature
		}

	}

	//Trigger Humidity measurement buffer[0]=0x01;
	//fModuleHumidity = 0;
	buffer[0] = GET_HUMIDITY_CMD; //Pointer buffer
	if (HAL_I2C_Master_Transmit(&hi2c3, 0x40 << 1, buffer, 1, 10) != HAL_OK) {
		I2C_ClearBusyFlagErratum(&hi2c3, 1000);
	} else {
		HAL_Delay(I2C_DELAY);
		if (HAL_I2C_Master_Receive(&hi2c3, 0x40 << 1, buffer, 2, 10)
				!= HAL_OK) {
			I2C_ClearBusyFlagErratum(&hi2c3, 1000);
		}

		rawH = buffer[0] << 8 | buffer[1]; //combine 2 8-bit into 1 16bit
		fHumidity = ((float) (rawH * 125) / 65536) - 6.0;
		if((fHumidity>=100||fHumidity<6)){
			fHumidity=100;
		}
		if(fHumidity>fHumidity_Max){
				fHumidity_Max=fHumidity;
		}

	}

	//while(HAL_I2C_IsDeviceReady(&hi2c3, 0x40<<1, 1, HAL_MAX_DELAY)!= HAL_OK);
	I2CChannelRelease(channel);
















	Module_Temp(0);
	OverHeatTempCheck(0);
	HAL_Delay(10);
}

void Module_Temp(int Channel){	//SPI통신 - 모듈 온도 측정
	uint8_t answer = 0;
	uint16_t reg = 0;
	float ErrorCheckTemp=0;

	answer = max6675ReadReg(&reg,Channel);
	if(answer == MAX6675_OK){
		ErrorCheckTemp=max6675Temp(reg);
		//if(Channel==0){
			fModuleTemperature=ErrorCheckTemp;
		//}
		//else if(Channel==1){
		//	fModuleTemperature2=ErrorCheckTemp;
		//}
	}
	else{

	}
}
void OverHeatTempCheck(int channel){

	tempArr[tempIndex]=fModuleTemperature;
	tempIndex++;

	if(tempIndex>10){
		tempIndex=0;
	}

	if(fModuleTemperature > DeviceInfo.overheat_temperature) {
		overheattemp = fModuleTemperature;
	}
}

void avgOverHeatTemp(){
	int hap=0,max=0,min=10000;
	float avg=0;
	int repeat=sizeof(tempArr)/sizeof(int);
	int checkOverHeat=0;
	for(int i=0;i<repeat;i++){
		hap+=tempArr[i];
		if(tempArr[i]>max){
			max = tempArr[i];
		}
		if(tempArr[i]<min){
			min = tempArr[i];
		}
	}
	hap = hap-max-min;
	avg = hap/(sizeof(tempArr)/sizeof(int)-2);
	checkOverHeat = avg;

	if( checkOverHeat > DeviceInfo.overheat_temperature) {
		overheatcnt++;
	}else{
		overheatcnt=0;
	}
	if(overheatcnt>6){
		overheatFlag=1;
		Temperature_Error_Flag = 1;
	}
	else{
		overheatFlag=0;
		Temperature_Error_Flag = 0;
	}

	tempIndex=0;
//	overheatFlag=0;
}
void I2CChannelSelect(int channel)
{
	switch (channel) {
	case 0:
		HAL_GPIO_WritePin(I2C_SEL1_GPIO_Port, I2C_SEL1_Pin, GPIO_PIN_SET); //i2c 1 ch selection enable
		break;
	case 1:
		HAL_GPIO_WritePin(I2C_SEL2_GPIO_Port, I2C_SEL2_Pin, GPIO_PIN_SET); //i2c 1 ch selection enable
		break;
	case 2:
		HAL_GPIO_WritePin(I2C_SEL3_GPIO_Port, I2C_SEL3_Pin, GPIO_PIN_SET); //i2c 1 ch selection enable
		break;
	case 3:
		HAL_GPIO_WritePin(I2C_SEL4_GPIO_Port, I2C_SEL4_Pin, GPIO_PIN_SET); //i2c 1 ch selection enable
		break;
	}
	HAL_Delay(I2C_DELAY);
}

void I2CChannelRelease(int channel)
{
	switch (channel) {
	case 0:
		HAL_GPIO_WritePin(I2C_SEL1_GPIO_Port, I2C_SEL1_Pin, GPIO_PIN_RESET); //i2c 1 ch selection enable
		break;
	case 1:
		HAL_GPIO_WritePin(I2C_SEL2_GPIO_Port, I2C_SEL2_Pin, GPIO_PIN_RESET); //i2c 1 ch selection enable
		break;
	case 2:
		HAL_GPIO_WritePin(I2C_SEL3_GPIO_Port, I2C_SEL3_Pin, GPIO_PIN_RESET); //i2c 1 ch selection enable
		break;
	case 3:
		HAL_GPIO_WritePin(I2C_SEL4_GPIO_Port, I2C_SEL4_Pin, GPIO_PIN_RESET); //i2c 1 ch selection enable
		break;
	}

	HAL_Delay(I2C_DELAY);
}

void InitTemperature(void){
	I2CChannelRelease(0);
	I2CChannelRelease(1);
	I2CChannelRelease(2);
	I2CChannelRelease(3);

}
