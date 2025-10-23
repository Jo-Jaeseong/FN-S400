/*
 * adc.c
 *
 *  Created on: 2019. 5. 30.
 *      Author: monster
 */

#include "main.h"
#include "lcd.h"
//#include "interrupt.h"

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

uint32_t uiDensity, uiTemp_AI1, uiTemp_AI2, uiTemp_AI3, uiTemp_AI4;
uint32_t adcData[5];
extern float fDensity;
extern unsigned char Running_Flag;
int maxDensity=0, Density=0;
int arrDensity[5];
int index=0;
int avgmax=0;
extern unsigned int SafetyPPM;
int StartPPM;
int H2O2Sensor_Flag;

void InitADC()
{
	// Define variable to hold the 8 ADC values
	// Why 32 bits if the resolution is 12b?
	// Start ADC DMA
/*
	hdma_adc1.Instance = DMA2_Stream0;
	hdma_adc1.Init.Channel = DMA_CHANNEL_0;
	hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
	hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	hdma_adc1.Init.Mode = DMA_CIRCULAR;
	hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
	hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
	{
	  Error_Handler();
	}
	__HAL_LINKDMA(&hadc1,DMA_Handle,hdma_adc1);
*/
	//HAL_ADC_Start(&hadc1);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcData, 5);
}

void GetDensity(){
	uiDensity = adcData[0];
	Density = uiDensity;
//	uiDensity -= 620;
	Density -= 620;
//	Density -= 780;
//	int index=0;
	if(Density < -500){
		H2O2Sensor_Flag=0;
		Density = 0;
	}
	else if(Density < 0){
		H2O2Sensor_Flag=1;
		Density = 0;
	}
	else{
		H2O2Sensor_Flag=1;
	}
//	float test = (float)Density/2.48;
//	float test = (float)Density/6.55;
	float test = (float)Density/4.96;	//500ppm
//	float test = (float)Density/8.26;	//300ppm
//	arrInsert(arrDensity,strlen(arrDensity),(int)test);
//	arrAppend(arrDensity,(int)test);
	arrDensity[index]=test;
	index++;
//	fDensity = (float)Density / 4.96;
//	if(maxDensity < (int)fDensity){
//		maxDensity=(int)fDensity;
//	}

	uiTemp_AI1 = adcData[1];
	uiTemp_AI2 = adcData[2];
	uiTemp_AI3 = adcData[3];
	uiTemp_AI4 = adcData[4];
	//DisplayH2O2SensorIcon(H2O2Sensor_Flag);
}

void DisplayAvgDensity(){
	int hap=0,max=0,min=10000;

	float avg=0;
	int repeat=sizeof(arrDensity)/sizeof(int);
	for(int i=0;i<repeat;i++){
		hap+=arrDensity[i];
		if(arrDensity[i]>max){
			max = arrDensity[i];
		}
		if(arrDensity[i]<min){
			min = arrDensity[i];
		}
	}
	hap = hap-max-min;
	avg = hap/(sizeof(arrDensity)/sizeof(int)-2);
	fDensity = avg;
	fDensity=fDensity-5;
	if(fDensity<=0){
		fDensity=0;
	}

	if(Running_Flag){
		if(avg>avgmax){
			avgmax=avg;
		}

		maxDensity=avgmax;
	}else {
		if(SafetyPPM==1){
		  if(fDensity <= StartPPM+10){
//			SavePPM(0);
			SafetyPPM=0;
		  }
		}
	}
	index=0;


}

void arrInsert(int *ar, int idx, int Iar){
	memmove(ar+idx+1,ar+idx,sizeof(ar)/sizeof(int)-idx+1);
	ar[idx]=Iar;
	idx++;
}
void arrAppend(int *ar, float Iar){

//	ar[index]=Iar;
//	index++;
	arrInsert(ar, sizeof(ar)/sizeof(int), Iar);
}
