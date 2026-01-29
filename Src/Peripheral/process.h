/*
 * Process.h
 *
 *  Created on: May 27, 2019
 *      Author: monster
 */

#ifndef PERIPHERAL_PROCESS_H_
#define PERIPHERAL_PROCESS_H_

void InitProcess(void);
void StartProcess(void);
void EndProcess(void);
void CancelProcess(void);

void ProcessEndTimer(void);
void ProcessTestEndTimer(void);

void CentiSecondProcess(void);
void DeliSecondProcess(void);
void HalfSecondProcess(void);
void OneSecondProcess(void);
void OneMinuteProcess(void);
void TenMinuteProcess();

void TestProcess(void);
int H2O2Check(void);
void H2O2Read(void);

void SprayProcess(void);
void SterileProcess(void);
void SterileMiddleProcess(void);
void SterileEndProcess(void);
void ScrubProcess(void);
void FinishProcess(void);

void ReCalcTime();

void SaveLog(void);
void SaveUsbLog(void);
void TestSaveUsbLog(void);
void SaveTimeLog(void);


struct data_format {
	unsigned char year, month, day;
	unsigned char hour, minute, second;
	float		  temperature, humidity;
	float		  module_temperature, module_humidity;
	float		  density;
	float		  volume;
};

struct log_format {
	unsigned char year[5], month[5], day[5];
	unsigned char hour[5], minute[5];
	float		  temperature, humidity;
	float		  module_temperature, module_humidity;
	float		  density;
	float		  volume;
	float		  cubic[5];
	int			  status[5];
	int		      ID[5];
};

struct FLData {
	unsigned char year, month, day;
	unsigned char hour, minute;
	unsigned char	temperature, humidity;
	float		  	density;
	float			volume;
};

struct RFID_format {
	unsigned char year[5], month[5];
	unsigned char year0, month0;
	int concentration;

	unsigned char CurrentRFIDValue[4];
	unsigned char RFIDValue[5][4];
	float H2O2Volume[5];
	float fH2O2Volume;
};

struct Account_format {
	unsigned char year[6], month[6], day[6];
	unsigned char hour[6], minute[6], second[6];
	float ID[6];
	char PW[6][4];
	int Attempts[6];
	int Status[6];
};

struct DeviceInfo_format {
	//Information
	int device_version;	//1
	unsigned char year, month, Serial1, Serial2;	//4
	char modem_number1[4], modem_number2[4];	//8

	//Configuration
	int peri1_speed, peri2_speed;
	int fan_low_speed, fan_high_speed;
	float lower_temperature, upper_temperature, overheat_temperature;

	//Time
	int PreHeatTime;
	int LineCleanTime;
	int NozzleCleanTime;
	int SterileTime;

	//Function
	int loginonoff_flag;
	int reservationonoff_flag;
};


void SaveEndLog(void);
void SaveStartLog(void);
void PreHeatOneMinuteProcess(void);
void FiveMinuteProcess(void);
void FiveSecondProcess(void);
void FiveHourProcess(void);
void TwoHourProcess(unsigned int hour);
void TimeCounterProcess(void);
void OneHourProcess(void);
void SavePPM(int hour);
void SaveFinishTime(void);
unsigned char TimeConvert(unsigned char time);

//추가 기능
void DeviceSettingReset();
void DeviceSettingResetAll();

//History
void SaveActionLog(void);
void SaveCancelLog(void);
void SaveStartLogFlash(int index);
void SaveEndLogFlash(int index);

//H2O2 log
void RFIDCompare(void);
void RFIDDataLoad(void);

//Account
void SaveLastLoginLog(void);



void SaveExpectFinishTime();
void ReserveProcess();
void SaveReserveTime();

typedef enum {
	IO_ACTION_NONE = 0,
	IO_ACTION_USB,
	IO_ACTION_SMS
} IoActionType;

void EnforceIoActionGap(IoActionType action_type);

#endif /* PERIPHERAL_PROCESS_H_ */
