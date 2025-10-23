/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define I2C_SEL4_Pin GPIO_PIN_14
#define I2C_SEL4_GPIO_Port GPIOC
#define TP74_Pin GPIO_PIN_15
#define TP74_GPIO_Port GPIOC
#define Temp_AI1_Pin GPIO_PIN_0
#define Temp_AI1_GPIO_Port GPIOC
#define BlowerFanSignal2_Pin GPIO_PIN_1
#define BlowerFanSignal2_GPIO_Port GPIOC
#define BlowerFanSignal3_Pin GPIO_PIN_2
#define BlowerFanSignal3_GPIO_Port GPIOC
#define Temp_AI4_Pin GPIO_PIN_3
#define Temp_AI4_GPIO_Port GPIOC
#define H2O2Sensor_Pin GPIO_PIN_0
#define H2O2Sensor_GPIO_Port GPIOA
#define PeristalticPumpPwm1_Pin GPIO_PIN_1
#define PeristalticPumpPwm1_GPIO_Port GPIOA
#define PeristalticPump1_Pin GPIO_PIN_2
#define PeristalticPump1_GPIO_Port GPIOA
#define SPI1_SS_Pin GPIO_PIN_3
#define SPI1_SS_GPIO_Port GPIOA
#define I2C_SEL1_Pin GPIO_PIN_4
#define I2C_SEL1_GPIO_Port GPIOA
#define SPI1_SCK_Pin GPIO_PIN_5
#define SPI1_SCK_GPIO_Port GPIOA
#define BlowerFanSignal1_Pin GPIO_PIN_6
#define BlowerFanSignal1_GPIO_Port GPIOA
#define SPI1_MOSI_Pin GPIO_PIN_7
#define SPI1_MOSI_GPIO_Port GPIOA
#define I2C_SEL2_Pin GPIO_PIN_4
#define I2C_SEL2_GPIO_Port GPIOC
#define I2C_SEL3_Pin GPIO_PIN_5
#define I2C_SEL3_GPIO_Port GPIOC
#define ScrubberONOFF_Pin GPIO_PIN_0
#define ScrubberONOFF_GPIO_Port GPIOB
#define HeaterOnOff1_Pin GPIO_PIN_1
#define HeaterOnOff1_GPIO_Port GPIOB
#define SolenoidFluidPathAirPurge1_Pin GPIO_PIN_2
#define SolenoidFluidPathAirPurge1_GPIO_Port GPIOB
#define TX_Bluetooth_Pin GPIO_PIN_10
#define TX_Bluetooth_GPIO_Port GPIOB
#define RX_Bluetooth_Pin GPIO_PIN_11
#define RX_Bluetooth_GPIO_Port GPIOB
#define HeaterOnOff12_Pin GPIO_PIN_12
#define HeaterOnOff12_GPIO_Port GPIOB
#define AirPumpONOFF_Pin GPIO_PIN_13
#define AirPumpONOFF_GPIO_Port GPIOB
#define SolenoidNozzleCleaning_Pin GPIO_PIN_14
#define SolenoidNozzleCleaning_GPIO_Port GPIOB
#define SensorFan_Pin GPIO_PIN_15
#define SensorFan_GPIO_Port GPIOB
#define PeristalticPumpPWM2_Pin GPIO_PIN_6
#define PeristalticPumpPWM2_GPIO_Port GPIOC
#define BlowerFanControlPwm3_Pin GPIO_PIN_7
#define BlowerFanControlPwm3_GPIO_Port GPIOC
#define BlowerFanControlPwm2_Pin GPIO_PIN_8
#define BlowerFanControlPwm2_GPIO_Port GPIOC
#define SDA_TEMP_RH_Pin GPIO_PIN_9
#define SDA_TEMP_RH_GPIO_Port GPIOC
#define SCL_TEMP_RH_Pin GPIO_PIN_8
#define SCL_TEMP_RH_GPIO_Port GPIOA
#define Tx_DWIN_Lcd_Pin GPIO_PIN_9
#define Tx_DWIN_Lcd_GPIO_Port GPIOA
#define Rx_DWIN_Lcd_Pin GPIO_PIN_10
#define Rx_DWIN_Lcd_GPIO_Port GPIOA
#define BlowerFanControlPwm1_Pin GPIO_PIN_15
#define BlowerFanControlPwm1_GPIO_Port GPIOA
#define Tx_LteModule_Pin GPIO_PIN_10
#define Tx_LteModule_GPIO_Port GPIOC
#define Rx_LteModule_Pin GPIO_PIN_11
#define Rx_LteModule_GPIO_Port GPIOC
#define LTEONOFF_Pin GPIO_PIN_12
#define LTEONOFF_GPIO_Port GPIOC
#define Tempsensor1_Pin GPIO_PIN_2
#define Tempsensor1_GPIO_Port GPIOD
#define SPI1_MISO_Pin GPIO_PIN_4
#define SPI1_MISO_GPIO_Port GPIOB
#define SCL_TEMP_RHB6_Pin GPIO_PIN_6
#define SCL_TEMP_RHB6_GPIO_Port GPIOB
#define SDA_TEMP_RHB7_Pin GPIO_PIN_7
#define SDA_TEMP_RHB7_GPIO_Port GPIOB
#define TP73_Pin GPIO_PIN_8
#define TP73_GPIO_Port GPIOB
#define PeristalticPump2_Pin GPIO_PIN_9
#define PeristalticPump2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
