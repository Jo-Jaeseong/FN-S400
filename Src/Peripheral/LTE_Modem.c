/*
 * LTE_Modem.c
 *
 *  Created on: Jun 13, 2019
 *      Author: PC
 */
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "Process.h"
#include "adc.h"
#include "i2c.h"

extern UART_HandleTypeDef huart4;

extern struct data_format	g_data[1500];
extern int 					g_data_index;

//extern struct data_format	startData;
extern struct log_format	startData;


//extern int					startIndex;
extern float fBoardTemperature_Max, fHumidity_Max;
extern struct data_format	endData;
//extern int					endIndex;
extern struct data_format	testdata;
extern struct data_format	finish_data;
extern unsigned char  Sms_Flag;
extern int maxDensity;
extern int overheatFlag;
extern float overheattemp;

unsigned char uart4_rx_data[10];
char user_number1[4],user_number2[4];

extern struct DeviceInfo_format DeviceInfo;
float UserSMS_Flag, ServerSMS_Flag;
int SMSonoff_Flag;
extern float nUsedVolume;

unsigned char szStartCommand[37] = "AT*SMSMO=01029755115,01238722145,1,1,";	//  판매처 연락처 입력
unsigned char szStartCommandCBT[37] = "AT*SMSMO=01075863426,01238722145,1,1,";	// CBT 연락처(01075863426)

//szStartCommand[25] 통신모듈 번호 첫자리(012 제외)
//szStartCommandCBT[25] 통신모듈 번호 첫자리

/*
 * N522 01238722143
 * 		01238722144
 * N400 01231409456
*/

const unsigned char szEndCommand[2] = "\r\n";
const unsigned char time[4]={		//시간
		0xBD, 0xC3, 0xB0, 0xA3
};
const unsigned char temp[4]={		//온도
		0xBF, 0xC2, 0xB5, 0xB5
};
const unsigned char humid[4]={		//습도
		0xBD, 0xC0, 0xB5, 0xB5
};
const unsigned char smsEnter[2]={	//개행문자
		0x0D, 0x0A
};
const unsigned char sterilizer[6]={	//멸균제
		0xB8, 0xEA, 0xB1, 0xD5, 0xC1, 0xA6
};
const unsigned char concentration[4]={  //농도
		0xB3, 0xF3, 0xB5, 0xB5
};
const unsigned char summary[4]={  //요약
		0xBF, 0xE4, 0xBE, 0xE0
};

/*
const unsigned char serialNum[13]={  //Serial Number 입력<FN-S3001A01>
		0x3C, 0x46, 0x4E, 0x2D, 0x53, 0x33, 0x30, 0x30, 0x31, 0x41, 0x30, 0x31, 0x3E
};
*/

unsigned char serialNum[13]={  //Serial Number 입력<FN-S3001A01>
		0x3C, 0x46, 0x4E, 0x2D, 0x53, 0x33, 0x30, 0x30, 0x31, 0x41, 0x30, 0x31, 0x3E
};

//serialNum[5] - Device Version

//serialNum[8] - year
//serialNum[9] - month
//serialNum[10] - Serial number1
//serialNum[11] - Serial number2

//5칸 시리얼 넘버 플래시 저장
//8자리 전화 번호 저장




static int NormalizeSmsDensity(float density)
{
	int ppm = (int)density;
	if (ppm < 0) {
		ppm = 0;
	}
	if (ppm <= 3) {
		ppm = 0;
	}
	return ppm;
}

void InitLTEModem(void){
	if(user_number1[0]==0&&user_number1[1]==0&&user_number1[2]==0&&user_number1[3]==0){
		szStartCommand[12] = '7';
		szStartCommand[13] = '5';
		szStartCommand[14] = '8';
		szStartCommand[15] = '6';
		szStartCommand[16] = '3';
		szStartCommand[17] = '4';
		szStartCommand[18] = '2';
		szStartCommand[19] = '6';
	}
	else{
		for(int i=0;i<4;i++){
			szStartCommand[12+i]=user_number1[i];
			szStartCommand[16+i]=user_number2[i];
		}
	}

	for(int i=0;i<4;i++){
		szStartCommand[24+i]=DeviceInfo.modem_number1[i];
		szStartCommandCBT[24+i]=DeviceInfo.modem_number1[i];
		szStartCommand[28+i]=DeviceInfo.modem_number2[i];
		szStartCommandCBT[28+i]=DeviceInfo.modem_number2[i];
	}
	//SMS ONOFF SETTING
	if((SMSonoff_Flag&0x01)==0x01){
		UserSMS_Flag=1;
	}
	else{
		UserSMS_Flag=0;
	}
	if((SMSonoff_Flag&0x10)==0x10){
		ServerSMS_Flag=1;
	}
	else{
		ServerSMS_Flag=0;
	}
}

void SendMessage(unsigned char *msg, int len){
	if(UserSMS_Flag==1){
		HAL_UART_Transmit(&huart4, (unsigned char *)szStartCommand, 37, 10);
		for(int i = 0; i < len-1; i++)
		{
			char ucHexValue[3];
			sprintf(ucHexValue, "%.2X", msg[i]);
			HAL_UART_Transmit(&huart4, (unsigned char *)ucHexValue, 2, 10);
		}
		HAL_UART_Transmit(&huart4, (unsigned char *)szEndCommand, 2, 10);
		HAL_Delay(100);
	}
}

void SendMessage1(unsigned char *msg, int len){
	if(ServerSMS_Flag==1){
		HAL_UART_Transmit(&huart4, (unsigned char *)szStartCommandCBT, 37, 10);
		for(int i = 0; i < len-1; i++)
		{
			char ucHexValue[3];
			sprintf(ucHexValue, "%.2X", msg[i]);
			HAL_UART_Transmit(&huart4, (unsigned char *)ucHexValue, 2, 10);
		}
		HAL_UART_Transmit(&huart4, (unsigned char *)szEndCommand, 2, 10);
		HAL_Delay(100);
	}
}

void SendTestMessage(void){
	EnforceIoActionGap(IO_ACTION_SMS);
	char mesg[12] = {
		0xC5, 0xD7, 0xBD, 0xBA, 0xC6, 0xAE, 0xB8, 0xDE, 0xBC, 0xBC, 0xC1, 0xF6
	};
	char startStatus[4]={
			0xBD, 0xC3, 0xC0, 0xDB
	};
	char ucMessage[150];
	memset(ucMessage, 0, 150);
	int iIndex = g_data_index - 1;
	memset(ucMessage, 0, 40);
	char ProcessStatus[6]={ 0xC8, 0xAF, 0xB1, 0xE2, 0xC1, 0xDF};
	sprintf(ucMessage, "%.13s", // @suppress("Float formatting support")
			mesg
	);
	SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
	HAL_Delay(100);

	sprintf(ucMessage, "%.13s", // @suppress("Float formatting support")
			mesg
	);
	SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));

	iIndex = 0;
}

void SendEndMessage(void)//요약
{
	EnforceIoActionGap(IO_ACTION_SMS);
	if(Sms_Flag ==9){
		char ucMessage[100];
		memset(ucMessage, 0, 100);
		sprintf(ucMessage, "%.4s %.2s"
				"%.4s:%.2dC > %.2dC %.2s"
				"%.4s:%.2d%% > %.2d%% %.2s"
				"%.4s:%dppm %.2s"
				"%.6s:%dmL %c"
				,summary, smsEnter
				,temp, (int)startData.temperature, (int)fBoardTemperature_Max, smsEnter
				,humid, (int)startData.humidity, (int)fHumidity_Max, smsEnter
				,concentration, NormalizeSmsDensity((float)maxDensity),smsEnter
				,sterilizer, (int)startData.volume-(int)nUsedVolume, "0x00"
				);
		SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
		HAL_Delay(100);
		sprintf(ucMessage, "%.4s %.2s"
				"%.4s:%.2dC > %.2dC %.2s"
				"%.4s:%.2d%% > %.2d%% %.2s"
				"%.4s:%dppm %.2s"
				"%.6s:%dmL %c"
				,summary, smsEnter
				,temp, (int)startData.temperature, (int)fBoardTemperature_Max, smsEnter
				,humid, (int)startData.humidity, (int)fHumidity_Max, smsEnter
				,concentration, NormalizeSmsDensity((float)maxDensity),smsEnter
				,sterilizer, (int)startData.volume-(int)nUsedVolume, "0x00"
				);
		SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
	}
}

void SendProcessMessage()
{
	EnforceIoActionGap(IO_ACTION_SMS);
	if(Sms_Flag == 0){ // 분사시작
		char ucMessage[50];
		int iIndex = g_data_index - 1;
		memset(ucMessage, 0, 50);
		char ProcessStatus[8]={ 0xBA, 0xD0, 0xBB, 0xE7, 0xBD, 0xC3, 0xC0, 0xDB};
		sprintf(ucMessage, "%.13s%.2s %.8s %.2s %.4s:%.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
		HAL_Delay(100);
		sprintf(ucMessage, "%.13s%.2s %.8s %.2s %.4s:%.2dC %.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature, (int)g_data[iIndex].module_temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
		iIndex = 0;
		Sms_Flag++;
	} else if(Sms_Flag == 1){ // 분사중
		char ucMessage[50];
		int iIndex = g_data_index - 1;
		memset(ucMessage, 0, 50);
		char ProcessStatus[6]={ 0xBA, 0xD0, 0xBB, 0xE7, 0xC1, 0xDF};
		sprintf(ucMessage, "%.13s%.2s %.6s %.2s %.4s:%.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
		HAL_Delay(100);
		sprintf(ucMessage, "%.13s%.2s %.6s %.2s %.4s:%.2dC %.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature, (int)g_data[iIndex].module_temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
		iIndex = 0;
	} else if(Sms_Flag ==2){ //멸균시작
		char ucMessage[60];
		int iIndex = g_data_index - 1;
		memset(ucMessage, 0, 60);
		char ProcessStatus[8]={ 0xB8, 0xEA, 0xB1, 0xD5, 0xBD, 0xC3, 0xC0, 0xDB};
		sprintf(ucMessage, "%.13s%.2s %.8s %.2s %.4s:%.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
				);
		SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
		HAL_Delay(100);
		sprintf(ucMessage, "%.13s%.2s %.8s %.2s %.4s:%.2dC %.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature, (int)g_data[iIndex].module_temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
		iIndex = 0;
	} else if(Sms_Flag ==3 ){ //멸균중
		char ucMessage[50];
		int iIndex = g_data_index - 1;
		memset(ucMessage, 0, 50);
		char ProcessStatus[6]={ 0xB8, 0xEA, 0xB1, 0xD5, 0xC1, 0xDF};
		sprintf(ucMessage, "%.13s%.2s %.6s %.2s %.4s:%.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
		HAL_Delay(100);
		sprintf(ucMessage, "%.13s%.2s %.6s %.2s %.4s:%.2dC %.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature, (int)g_data[iIndex].module_temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
		iIndex = 0;
	} else if(Sms_Flag ==4 ){ //종료
		char ucMessage[40];
		int iIndex = g_data_index - 1;
		memset(ucMessage, 0, 40);
		char ProcessStatus[4]={ 0xC1, 0xBE, 0xB7, 0xE1};
		sprintf(ucMessage, "%.13s%.2s %.4s %.2s %.4s:%.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
		HAL_Delay(100);
		sprintf(ucMessage, "%.13s%.2s %.4s %.2s %.4s:%.2dC %.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature, (int)g_data[iIndex].module_temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
		iIndex = 0;
	} else if(Sms_Flag ==6 ){ //취소
		if(overheatFlag==1){
			char ProcessStatus[8]={ 0xB0, 0xFA, 0xBF, 0xAD, 0xC2, 0xF7, 0xB4, 0xDC};
			char ucMessage[50];
			memset(ucMessage, 0, 50);
			sprintf(ucMessage, "%.8s : %dC %c",
					ProcessStatus, (int)overheattemp, "0x00");
			SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
			HAL_Delay(100);
			SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
		}else{
			char ProcessStatus[17]={ 0xC3, 0xEB, 0xBC, 0xD2, 0x20, 0xB5, 0xC7, 0xBE, 0xFA, 0xBD, 0xC0, 0xB4, 0xCF, 0xB4, 0xD9, 0x2E, 0x00};
			SendMessage((unsigned char *)ProcessStatus, strlen(ProcessStatus));
			HAL_Delay(100);
			SendMessage1((unsigned char *)ProcessStatus, strlen(ProcessStatus));

		}

	} else if(Sms_Flag ==7){ //환기시작
		char ucMessage[40];
		int iIndex = g_data_index - 1;
		memset(ucMessage, 0, 40);
		char ProcessStatus[8]={ 0xC8, 0xAF, 0xB1, 0xE2, 0xBD, 0xC3, 0xC0, 0xDB};
		sprintf(ucMessage, "%.13s%.2s %.8s %.2s %.4s:%.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
		HAL_Delay(100);
		sprintf(ucMessage, "%.13s%.2s %.8s %.2s %.4s:%.2dC %.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature, (int)g_data[iIndex].module_temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
		iIndex = 0;
	}else if(Sms_Flag==8){	//환기중
		char ucMessage[70];
		int iIndex = g_data_index - 1;
		memset(ucMessage, 0, 70);
		char ProcessStatus[6]={ 0xC8, 0xAF, 0xB1, 0xE2, 0xC1, 0xDF};
		sprintf(ucMessage, "%.13s%.2s %.6s %.2s %.4s:%.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
		HAL_Delay(100);
		sprintf(ucMessage, "%.13s%.2s %.6s %.2s %.4s:%.2dC %.2dC %.2s %.4s:%.2d%% %.2s %.4s:%dppm %c", // @suppress("Float formatting support")
				serialNum, smsEnter, ProcessStatus,
				smsEnter, temp, (int)g_data[iIndex].temperature, (int)g_data[iIndex].module_temperature,
				smsEnter, humid, (int)g_data[iIndex].humidity,
				smsEnter, concentration, NormalizeSmsDensity(g_data[iIndex].density), "0x00"
		);
		SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
		iIndex = 0;
	}

}

void SendCurrentPPM(int hour){
	EnforceIoActionGap(IO_ACTION_SMS);
	char ucMessage[100];
	memset(ucMessage, 0, 100);

	if(hour ==0){
		hour = 6;
		char ProcessStatus[7]={ 0x61, 0x6D, 0x20, 0xB3, 0xF3, 0xB5, 0xB5};
		sprintf(ucMessage, "%d%.7s : %dppm %c", // @suppress("Float formatting support")
				hour, ProcessStatus, NormalizeSmsDensity(testdata.density), "0x00"
		);
	}else{
		char ProcessStatus[7]={ 0x68, 0x2B, 0x20, 0xB3, 0xF3, 0xB5, 0xB5};
		sprintf(ucMessage, "%d%.7s : %dppm %c", // @suppress("Float formatting support")
				hour, ProcessStatus, NormalizeSmsDensity(testdata.density), "0x00"
		);
	}
	SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
	HAL_Delay(100);
	SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
}
void SendFinishTime(unsigned char smsHour, unsigned char smsMinute){
	EnforceIoActionGap(IO_ACTION_SMS);
	char ucMessage[70];
	memset(ucMessage, 0, 70);
	char Time[2]={0xBD, 0xC3};
	char Minute[2]={0xBA, 0xD0};
	char ampm[4]={0xBF, 0xC0, 0xC0, 0xFC};
	char ProcessStatus[12]={ 0xBF, 0xB9, 0xBB, 0xF3, 0xC1, 0xBE, 0xB7, 0xE1, 0xBD, 0xC3, 0xB0, 0xA3};

	if(smsHour > 0x12){
		if(smsHour >= 0x24){
			smsHour=smsHour-0x12;
		}else{
			ampm[2]=0xC8;
			ampm[3]=0xC4;
			smsHour=smsHour-0x12;
			if(smsHour==0x0f)	//헥사 계산 표기를 정정(오후9시)
				smsHour=0x09;
			else if(smsHour==0x0e)	//헥사 계산 표기를 정정(오후8시)
				smsHour=0x08;
		}
	}else{
		if(smsHour == 0x12){
			ampm[2]=0xC8;
			ampm[3]=0xC4;
		}
	}
	sprintf(ucMessage, "%.12s : %.4s %.2x%.2s %.2x%.2s %c", // @suppress("Float formatting support")
			ProcessStatus, ampm, smsHour, Time, smsMinute, Minute,	 "0x00"
	);
	SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
	HAL_Delay(100);
	SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
}

void SendReserveTime(unsigned char smsHour, unsigned char smsMinute){	//reserve time
	EnforceIoActionGap(IO_ACTION_SMS);
	char ucMessage[70];
	memset(ucMessage, 0, 70);
	char Time[2]={0xBD, 0xC3};
	char Minute[2]={0xBA, 0xD0};
	char ampm[4]={0xBF, 0xC0, 0xC0, 0xFC};
	char ProcessStatus[16]={0xBF, 0xB9, 0xBE, 0xE0, 0x20, 0xB5, 0xC7, 0xBE, 0xFA, 0xBD, 0xC0, 0xB4, 0xCF, 0xB4, 0xD9, 0x2E};
	char ProcessStatus2[12]={0xBF, 0xB9, 0xBB, 0xF3, 0xBD, 0xC3, 0xC0, 0xDB, 0xBD, 0xC3, 0xB0, 0xA3};

	if(smsHour > 0x12){
		if(smsHour >= 0x24){
			smsHour=smsHour-0x12;
		}else{
			ampm[2]=0xC8;
			ampm[3]=0xC4;
			smsHour=smsHour-0x12;
			if(smsHour==0x0f)	//헥사 계산 표기를 정정(오후9시)
				smsHour=0x09;
			else if(smsHour==0x0e)	//헥사 계산 표기를 정정(오후8시)
				smsHour=0x08;
		}
	}else{
		if(smsHour == 0x12){
			ampm[2]=0xC8;
			ampm[3]=0xC4;
		}
	}
	sprintf(ucMessage, "%.16s %.2s"
			"%.12s : %.4s %.2x%.2s %.2x%.2s %c", // @suppress("Float formatting support")
			ProcessStatus, smsEnter,
			ProcessStatus2, ampm, smsHour, Time, smsMinute, Minute, "0x00"
	);
	SendMessage((unsigned char *)ucMessage, strlen(ucMessage));
	HAL_Delay(100);
	SendMessage1((unsigned char *)ucMessage, strlen(ucMessage));
}
