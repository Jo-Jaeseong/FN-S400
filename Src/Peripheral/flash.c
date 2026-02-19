/*
 * flash.c
 *
 *  Created on: 2019. 5. 31.
 *      Author: monster
 */

#include "main.h"
#include "define.h"
#include "process.h"
#include "util.h"
#include "rfid.h"
#include "lcd.h"
#include "flash.h"
#include "string.h"

__attribute__((__section__(".user_data"))) const char userConfig[1024];
__attribute__((__section__(".user_log_data1"))) const char userLogData1[1040];
__attribute__((__section__(".user_log_data2"))) const char userLogData2[1040];
__attribute__((__section__(".user_log_data3"))) const char userLogData3[1040];
__attribute__((__section__(".user_log_data4"))) const char userLogData4[1040];

extern int IndexEndLog;

extern struct FLData		f_data[65];

extern volatile unsigned char Running_Flag;
extern float fInjectionPerMinute, fInjectionPerMinute2, fInjectionPerCubic;
extern float  fCubic;

extern int SMSonoff_Flag;
extern int USBSECURITYonoff_Flag;
extern char user_number1[4],user_number2[4];

//RFID
extern struct RFID_format RFIDData;
extern int checkret;

extern float fUsedVolume;
extern float nUsedVolume;

extern int IndexWriteFlash;
extern int BeforeRFIDFlag;
extern int NewRFIDFlag;

//History Log 관련
extern struct log_format startData;
extern struct log_format endData;
extern int IndexEndLog;

//Account list
extern struct Account_format IDLIST;

//Device_Info
extern struct DeviceInfo_format DeviceInfo;

extern unsigned char szStartCommand[37],szStartCommandCBT[37];	//�Ǹ�ó ����ó �Է�

//Float [1], float2char [4]

// Flash 구성
#define InjectionPerMinute_DATA			0 //[1]	int
#define InjectionPerMinute2_DATA		1 //[1]	int
#define InjectionPerCubic_DATA			2 //[1] 	int
#define Cubic_DATA							8 //[4] float

#define IndexEndLog_DATA					12 //[1] int
#define SMSonoff_Flag_DATA				13 //[1] int
#define USBSECURITYonoff_Flag_DATA	14 //[1] int
#define BeforeRFID_DATA					15 //[1] int

#define user_number1_DATA				16 //[4] char[4]
#define user_number2_DATA				20 //[4] char[4]

#define device_version_DATA				24 //[1] int
#define Serial_Year_DATA					25 //[1] char
#define Serial_Month_DATA					26 //[1] char
#define Serial_Num1_DATA					27 //[1] char
#define Serial_Num2_DATA					28 //[1] char

#define modem_number1_DATA			29 //[4] char[4]
#define modem_number2_DATA			33 //[4] char[4]

#define RFIDValue_DATA						45 //[20] char[4] * 5

#define RFIDVolume_DATA					65 //[20] float * 5
#define RFIDYear_DATA						85 //[5] char * 5
#define RFIDMonth_DATA						90 //[5] char * 5

#define AccountPassword_DATA			155 //[20]
#define AccountLastLogin_DATA			175 //[25]
#define AccountAttempts_DATA			200 //[5]
#define AccountStatus_DATA				205 //[5]

#define loginonoff_flag_DATA				210 //[1] int
#define reservationonoff_flag_DATA		211 //[1] int

#define peri1_speed_DATA					220	//[1] int
#define peri2_speed_DATA					221	//[1] int
#define fan_high_speed_DATA				222	//[1] int
#define fan_low_speed_DATA				223	//[1] int

#define lower_temperature_DATA		224	//[4] float
#define upper_temperature_DATA		228	//[4] float
#define overhear_temp_DATA				232	//[4] float

#define preheat_time_DATA					236	//[1] int
#define line_clean_time_DATA				237	//[1] int
#define nozzle_clean_time_DATA			238	//[1] int
#define sterile_time_DATA					239	//[1] int

#define LOG_DATA								250	//[56]

void Reset_Setting_Flash(){
	if(Running_Flag==0){
		DisplayPage(42);	//Loading
	}

	unsigned char ucData[1024];
	memset(ucData, 0, 400);
	ucData[InjectionPerMinute_DATA]=fInjectionPerMinute;
	ucData[InjectionPerMinute2_DATA]=fInjectionPerMinute2;
	ucData[InjectionPerCubic_DATA]=fInjectionPerCubic;
	float2char(ConstantCubic, ucData + Cubic_DATA);

	//float2char(ConstantInjectionPerMinute, ucData+InjectionPerMinute_DATA);
    //float2char(ConstantInjectionPerCubic, ucData + InjectionPerCubic_DATA);
    //float2char(ConstantCubic, ucData + Cubic_DATA);


	//설정 세팅값 저장
    SMSonoff_Flag=0x10;
	USBSECURITYonoff_Flag=0;

	//설정 저장
	ucData[IndexEndLog_DATA]=IndexEndLog;
	ucData[SMSonoff_Flag_DATA]=SMSonoff_Flag;
	ucData[USBSECURITYonoff_Flag_DATA]=USBSECURITYonoff_Flag;

	if(NewRFIDFlag==1&&RFIDData.fH2O2Volume!=3000){
		NewRFIDFlag=0;
		for(int i=0;i<4;i++){
			RFIDData.RFIDValue[BeforeRFIDFlag][i]=RFIDData.CurrentRFIDValue[i];
		}
		RFIDData.H2O2Volume[BeforeRFIDFlag]=RFIDData.fH2O2Volume;
		RFIDData.year[BeforeRFIDFlag]=RFIDData.year0;
		RFIDData.month[BeforeRFIDFlag]=RFIDData.month0;
		BeforeRFIDFlag++;
		if(BeforeRFIDFlag==5){
			BeforeRFIDFlag=1;
		}
	}
	else if(NewRFIDFlag==0&&RFIDData.H2O2Volume[IndexWriteFlash]!=RFIDData.fH2O2Volume){
		for(int i=0;i<4;i++){
			RFIDData.RFIDValue[IndexWriteFlash][i]=RFIDData.CurrentRFIDValue[i];
		}
		RFIDData.H2O2Volume[IndexWriteFlash]=RFIDData.fH2O2Volume;
		RFIDData.year[IndexWriteFlash]=RFIDData.year0;
		RFIDData.month[IndexWriteFlash]=RFIDData.month0;
	}
	else{

	}


	ucData[BeforeRFID_DATA]=BeforeRFIDFlag;	//순서 중요


	for(int i=0;i<5;i++){
		for(int j=0;j<4;j++){
			ucData[RFIDValue_DATA+i*4+j]=RFIDData.RFIDValue[i][j];
		}
		float2char(RFIDData.H2O2Volume[i], ucData+(4*i+RFIDVolume_DATA));
		ucData[RFIDYear_DATA+i]=RFIDData.year[i];
		ucData[RFIDMonth_DATA+i]=RFIDData.month[i];
	}

	for(int i=0;i<5;i++){
		ucData[AccountPassword_DATA+4*i]=IDLIST.PW[i+1][0];
		ucData[AccountPassword_DATA+4*i+1]=IDLIST.PW[i+1][1];
		ucData[AccountPassword_DATA+4*i+2]=IDLIST.PW[i+1][2];
		ucData[AccountPassword_DATA+4*i+3]=IDLIST.PW[i+1][3];

		ucData[AccountLastLogin_DATA+5*i]=IDLIST.year[i+1];
		ucData[AccountLastLogin_DATA+5*i+1]=IDLIST.month[i+1];
		ucData[AccountLastLogin_DATA+5*i+2]=IDLIST.day[i+1];
		ucData[AccountLastLogin_DATA+5*i+3]=IDLIST.hour[i+1];
		ucData[AccountLastLogin_DATA+5*i+4]=IDLIST.minute[i+1];


		ucData[AccountAttempts_DATA+i]=IDLIST.Attempts[i+1];
		ucData[AccountStatus_DATA+i]=IDLIST.Status[i+1];
	}

	//전화번호 추가
	/*
	for(int i=0;i<4;i++){
		ucData[user_number1_DATA+i]=user_number1[i];
		ucData[user_number2_DATA+i]=user_number2[i];
	}
	*/


	ucData[device_version_DATA]=DeviceInfo.device_version;

	ucData[Serial_Year_DATA]=DeviceInfo.year;
	ucData[Serial_Month_DATA]=DeviceInfo.month;
	ucData[Serial_Num1_DATA]=DeviceInfo.Serial1;
	ucData[Serial_Num2_DATA]=DeviceInfo.Serial2;

	for(int i=0;i<4;i++){
		ucData[modem_number1_DATA+i]=DeviceInfo.modem_number1[i];
		ucData[modem_number2_DATA+i]=DeviceInfo.modem_number2[i];
	}

	ucData[peri1_speed_DATA]=DeviceInfo.peri1_speed;
	ucData[peri2_speed_DATA]=DeviceInfo.peri2_speed;
	ucData[fan_high_speed_DATA]=DeviceInfo.fan_high_speed;
	ucData[fan_low_speed_DATA]=DeviceInfo.fan_low_speed;

	float2char(DeviceInfo.lower_temperature, ucData+lower_temperature_DATA);
	float2char(DeviceInfo.upper_temperature, ucData+upper_temperature_DATA);
	float2char(DeviceInfo.overheat_temperature, ucData+overhear_temp_DATA);

	ucData[preheat_time_DATA]=DeviceInfo.PreHeatTime;
	ucData[line_clean_time_DATA]=DeviceInfo.LineCleanTime;
	ucData[nozzle_clean_time_DATA]=DeviceInfo.NozzleCleanTime;
	ucData[sterile_time_DATA]=DeviceInfo.SterileTime;

	ucData[loginonoff_flag_DATA]=DeviceInfo.loginonoff_flag;
	ucData[reservationonoff_flag_DATA]=DeviceInfo.reservationonoff_flag;
	//ucData[Peri_Value_DATA]=DeviceInfo.PeriValue;

	//히스토리 정보 저장
	//if()
	for(int i=0;i<4;i++){
		ucData[LOG_DATA+(i*14)]=startData.year[i+1];
		ucData[LOG_DATA+1+(i*14)]=startData.month[i+1];
		ucData[LOG_DATA+2+(i*14)]=startData.day[i+1];
		ucData[LOG_DATA+3+(i*14)]=startData.hour[i+1];
		ucData[LOG_DATA+4+(i*14)]=startData.minute[i+1];
		ucData[LOG_DATA+5+(i*14)]=(int)(startData.cubic[i+1])/100;
		ucData[LOG_DATA+6+(i*14)]=(int)(startData.cubic[i+1])%100;

		ucData[LOG_DATA+7+(i*14)]=endData.year[i+1];
		ucData[LOG_DATA+8+(i*14)]=endData.month[i+1];
		ucData[LOG_DATA+9+(i*14)]=endData.day[i+1];
		ucData[LOG_DATA+10+(i*14)]=endData.hour[i+1];
		ucData[LOG_DATA+11+(i*14)]=endData.minute[i+1];
		ucData[LOG_DATA+12+(i*14)]=startData.status[i+1];
		ucData[LOG_DATA+13+(i*14)]=startData.ID[i+1];
	}

	ucData[IndexEndLog]=IndexEndLog; //추가

	 HAL_FLASH_Unlock();
	 __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
	 FLASH_Erase_Sector(FLASH_SECTOR_6, VOLTAGE_RANGE_3);

	 for(int i = 0; i < 1024; i++) {
		 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userConfig[0] + i, ucData[i]);
	 }

	 HAL_FLASH_Lock();

	 Read_Flash();
}

void Reset_All_Flash(){
	unsigned char ucData[1024];
	memset(ucData, 0, 400);
	ucData[InjectionPerMinute_DATA]=ConstantInjectionPerMinute;
	ucData[InjectionPerMinute2_DATA]=ConstantInjectionPerMinute2;
	ucData[InjectionPerCubic_DATA]=ConstantInjectionPerCubic;

    ucData[device_version_DATA]=DeviceInfo.device_version;

    SMSonoff_Flag=0;
	USBSECURITYonoff_Flag=0;

	for(int i=0;i<4;i++){
		szStartCommand[24+i]='0';
		szStartCommandCBT[24+i]='0';
		szStartCommand[28+i]='0';
		szStartCommandCBT[28+i]='0';
	}

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
	FLASH_Erase_Sector(FLASH_SECTOR_6, VOLTAGE_RANGE_3);

	for(int i = 0; i < 1024; i++) {
	 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userConfig[0] + i, ucData[i]);
	}

	HAL_FLASH_Lock();
	Read_Flash();
}

void Write_Flash(){
	if(Running_Flag==0){
		DisplayPage(42);	//Loading
	}

	unsigned char ucData[1024];
	memset(ucData, 0, 400);
	ucData[InjectionPerMinute_DATA]=fInjectionPerMinute;
	ucData[InjectionPerMinute2_DATA]=fInjectionPerMinute2;
	ucData[InjectionPerCubic_DATA]=fInjectionPerCubic;

	//float2char(fInjectionPerMinute, ucData+InjectionPerMinute_DATA);
    //float2char(fInjectionPerCubic, ucData + InjectionPerCubic_DATA);

    float2char(fCubic, ucData + Cubic_DATA);

    //설정 저장
    ucData[IndexEndLog_DATA]=IndexEndLog;
	ucData[SMSonoff_Flag_DATA]=SMSonoff_Flag;
	ucData[USBSECURITYonoff_Flag_DATA]=USBSECURITYonoff_Flag;

    if(NewRFIDFlag==1&&RFIDData.fH2O2Volume!=3000){
    	NewRFIDFlag=0;
		for(int i=0;i<4;i++){
			RFIDData.RFIDValue[BeforeRFIDFlag][i]=RFIDData.CurrentRFIDValue[i];
		}
		RFIDData.H2O2Volume[BeforeRFIDFlag]=RFIDData.fH2O2Volume;
		RFIDData.year[BeforeRFIDFlag]=RFIDData.year0;
		RFIDData.month[BeforeRFIDFlag]=RFIDData.month0;
    	BeforeRFIDFlag++;
    	if(BeforeRFIDFlag==5){
    		BeforeRFIDFlag=1;
    	}
    }
    else if(NewRFIDFlag==0&&RFIDData.H2O2Volume[IndexWriteFlash]!=RFIDData.fH2O2Volume){
		for(int i=0;i<4;i++){
			RFIDData.RFIDValue[IndexWriteFlash][i]=RFIDData.CurrentRFIDValue[i];
		}
		RFIDData.H2O2Volume[IndexWriteFlash]=RFIDData.fH2O2Volume;
		RFIDData.year[IndexWriteFlash]=RFIDData.year0;
		RFIDData.month[IndexWriteFlash]=RFIDData.month0;
    }
    else{

    }


	ucData[BeforeRFID_DATA]=BeforeRFIDFlag;	//순서 중요


	for(int i=0;i<5;i++){
		for(int j=0;j<4;j++){
			ucData[RFIDValue_DATA+i*4+j]=RFIDData.RFIDValue[i][j];
		}
		float2char(RFIDData.H2O2Volume[i], ucData+(4*i+RFIDVolume_DATA));
		ucData[RFIDYear_DATA+i]=RFIDData.year[i];
		ucData[RFIDMonth_DATA+i]=RFIDData.month[i];
	}

    for(int i=0;i<5;i++){
    	ucData[AccountPassword_DATA+4*i]=IDLIST.PW[i+1][0];
    	ucData[AccountPassword_DATA+4*i+1]=IDLIST.PW[i+1][1];
    	ucData[AccountPassword_DATA+4*i+2]=IDLIST.PW[i+1][2];
    	ucData[AccountPassword_DATA+4*i+3]=IDLIST.PW[i+1][3];

    	ucData[AccountLastLogin_DATA+5*i]=IDLIST.year[i+1];
		ucData[AccountLastLogin_DATA+5*i+1]=IDLIST.month[i+1];
		ucData[AccountLastLogin_DATA+5*i+2]=IDLIST.day[i+1];
		ucData[AccountLastLogin_DATA+5*i+3]=IDLIST.hour[i+1];
		ucData[AccountLastLogin_DATA+5*i+4]=IDLIST.minute[i+1];


		ucData[AccountAttempts_DATA+i]=IDLIST.Attempts[i+1];
    	ucData[AccountStatus_DATA+i]=IDLIST.Status[i+1];
    }

    //전화번호 추가
	for(int i=0;i<4;i++){
		ucData[user_number1_DATA+i]=user_number1[i];
		ucData[user_number2_DATA+i]=user_number2[i];
	}


	ucData[device_version_DATA]=DeviceInfo.device_version;

	ucData[Serial_Year_DATA]=DeviceInfo.year;
	ucData[Serial_Month_DATA]=DeviceInfo.month;
	ucData[Serial_Num1_DATA]=DeviceInfo.Serial1;
	ucData[Serial_Num2_DATA]=DeviceInfo.Serial2;

	for(int i=0;i<4;i++){
		ucData[modem_number1_DATA+i]=DeviceInfo.modem_number1[i];
		ucData[modem_number2_DATA+i]=DeviceInfo.modem_number2[i];
	}

	ucData[peri1_speed_DATA]=DeviceInfo.peri1_speed;
	ucData[peri2_speed_DATA]=DeviceInfo.peri2_speed;
	ucData[fan_high_speed_DATA]=DeviceInfo.fan_high_speed;
	ucData[fan_low_speed_DATA]=DeviceInfo.fan_low_speed;

	float2char(DeviceInfo.lower_temperature, ucData+lower_temperature_DATA);
	float2char(DeviceInfo.upper_temperature, ucData+upper_temperature_DATA);
	float2char(DeviceInfo.overheat_temperature, ucData+overhear_temp_DATA);

	ucData[preheat_time_DATA]=DeviceInfo.PreHeatTime;
	ucData[line_clean_time_DATA]=DeviceInfo.LineCleanTime;
	ucData[nozzle_clean_time_DATA]=DeviceInfo.NozzleCleanTime;
	ucData[sterile_time_DATA]=DeviceInfo.SterileTime;

    ucData[loginonoff_flag_DATA]=DeviceInfo.loginonoff_flag;
    ucData[reservationonoff_flag_DATA]=DeviceInfo.reservationonoff_flag;
	//ucData[Peri_Value_DATA]=DeviceInfo.PeriValue;












    //히스토리 정보 저장
    //if()
    for(int i=0;i<4;i++){
		ucData[LOG_DATA+(i*14)]=startData.year[i+1];
		ucData[LOG_DATA+1+(i*14)]=startData.month[i+1];
		ucData[LOG_DATA+2+(i*14)]=startData.day[i+1];
		ucData[LOG_DATA+3+(i*14)]=startData.hour[i+1];
		ucData[LOG_DATA+4+(i*14)]=startData.minute[i+1];
		ucData[LOG_DATA+5+(i*14)]=(int)(startData.cubic[i+1])/100;
		ucData[LOG_DATA+6+(i*14)]=(int)(startData.cubic[i+1])%100;

		ucData[LOG_DATA+7+(i*14)]=endData.year[i+1];
		ucData[LOG_DATA+8+(i*14)]=endData.month[i+1];
		ucData[LOG_DATA+9+(i*14)]=endData.day[i+1];
		ucData[LOG_DATA+10+(i*14)]=endData.hour[i+1];
		ucData[LOG_DATA+11+(i*14)]=endData.minute[i+1];
		ucData[LOG_DATA+12+(i*14)]=startData.status[i+1];
		ucData[LOG_DATA+13+(i*14)]=startData.ID[i+1];
    }
     HAL_FLASH_Unlock();
     __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
     FLASH_Erase_Sector(FLASH_SECTOR_6, VOLTAGE_RANGE_3);

     for(int i = 0; i < 1024; i++) {
    	 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userConfig[0] + i, ucData[i]);
     }

     HAL_FLASH_Lock();

     Read_Flash();
}

void Read_Flash(){
	/*
	fInjectionPerMinute = char2float((unsigned char *)userConfig+InjectionPerMinute_DATA);
	if(fInjectionPerMinute == 0) {
		fInjectionPerMinute = ConstantInjectionPerMinute;
	}
	fInjectionPerCubic=char2float((unsigned char *)(userConfig + InjectionPerCubic_DATA));
	if(fInjectionPerCubic == 0 || userConfig[4] == 0xff) {
		fInjectionPerCubic = ConstantInjectionPerCubic;
	}
	*/
	fInjectionPerMinute=userConfig[InjectionPerMinute_DATA];
	if(fInjectionPerMinute==0){
		fInjectionPerMinute=ConstantInjectionPerMinute;
	}
	fInjectionPerMinute2=userConfig[InjectionPerMinute2_DATA];
	if(fInjectionPerMinute2==0){
		fInjectionPerMinute2=ConstantInjectionPerMinute2;
	}
	fInjectionPerCubic=userConfig[InjectionPerCubic_DATA];
	if(fInjectionPerCubic==0){
		fInjectionPerCubic=ConstantInjectionPerCubic;
	}

	fCubic = char2float((unsigned char *)(userConfig + Cubic_DATA));
	if(fCubic == 0) {
		fCubic = ConstantCubic;
	}


	BeforeRFIDFlag = userConfig[BeforeRFID_DATA];
	if(BeforeRFIDFlag==0){
		BeforeRFIDFlag=1;
	}

	//세팅 값 불러오기
	SMSonoff_Flag = userConfig[SMSonoff_Flag_DATA];
	USBSECURITYonoff_Flag = userConfig[USBSECURITYonoff_Flag_DATA];
	IndexEndLog =userConfig[IndexEndLog_DATA];


	for(int i=0;i<5;i++){
		for(int j = 0 ; j<5; j++){
			RFIDData.RFIDValue[i][j]=userConfig[RFIDValue_DATA+4*i+j];
		}
		RFIDData.H2O2Volume[i] = char2float((unsigned char *)userConfig+RFIDVolume_DATA+4*i);
		RFIDData.year[i] = userConfig[RFIDYear_DATA+i];
		RFIDData.month[i] = userConfig[RFIDMonth_DATA+i];
	}


	//전화번호 불러오기
	for(int i=0;i<4;i++){
		user_number1[i]=userConfig[user_number1_DATA+i];
		user_number2[i]=userConfig[user_number2_DATA+i];
	}
	for(int i=0;i<4;i++){
		szStartCommand[12+i]=user_number1[i];//
	}
	for(int i=0;i<4;i++){
		szStartCommand[16+i]=user_number2[i];//
	}

	//디바이스 정보 불러오기
	DeviceInfo.device_version=userConfig[device_version_DATA];
	if(DeviceInfo.device_version==0){
		DeviceInfo.device_version=1;
	}
	DeviceInfo.year=userConfig[Serial_Year_DATA];
	DeviceInfo.month=userConfig[Serial_Month_DATA];
	DeviceInfo.Serial1=userConfig[Serial_Num1_DATA];
	DeviceInfo.Serial2=userConfig[Serial_Num2_DATA];

	for(int i=0;i<4;i++){
		DeviceInfo.modem_number1[i]=userConfig[modem_number1_DATA+i];
		DeviceInfo.modem_number2[i]=userConfig[modem_number2_DATA+i];
	}

	DeviceInfo.peri1_speed=userConfig[peri1_speed_DATA];
	DeviceInfo.peri2_speed=userConfig[peri2_speed_DATA];

	if(DeviceInfo.peri1_speed==0){
		DeviceInfo.peri1_speed=100;
	}
	if(DeviceInfo.peri2_speed==0){
		DeviceInfo.peri2_speed=100;
	}

	DeviceInfo.fan_high_speed=userConfig[fan_high_speed_DATA];
	DeviceInfo.fan_low_speed=userConfig[fan_low_speed_DATA];

	if(DeviceInfo.fan_high_speed==0){
		DeviceInfo.fan_high_speed=ConstantBlowerFanControlPwmMax;
	}
	if(DeviceInfo.fan_low_speed==0){
		DeviceInfo.fan_low_speed=ConstantBlowerFanControlPwmMin;
	}

	DeviceInfo.lower_temperature=char2float((unsigned char *)(userConfig + lower_temperature_DATA));
	DeviceInfo.upper_temperature=char2float((unsigned char *)(userConfig + upper_temperature_DATA));
	DeviceInfo.overheat_temperature=char2float((unsigned char *)(userConfig + overhear_temp_DATA));

	if(DeviceInfo.lower_temperature==0){
		DeviceInfo.lower_temperature=ConstantLowerTemperature;
	}
	if(DeviceInfo.upper_temperature==0){
		DeviceInfo.upper_temperature=ConstantUpperTemperature;
	}
	if(DeviceInfo.overheat_temperature==0){
		DeviceInfo.overheat_temperature=OverHeat_Temperature;
	}

	DeviceInfo.PreHeatTime=userConfig[preheat_time_DATA];
	DeviceInfo.LineCleanTime=userConfig[line_clean_time_DATA];
	DeviceInfo.NozzleCleanTime=userConfig[nozzle_clean_time_DATA];
	DeviceInfo.SterileTime=userConfig[sterile_time_DATA];

	if(DeviceInfo.PreHeatTime==0){
		DeviceInfo.PreHeatTime=ConstantPreHeatTime;
	}
	if(DeviceInfo.LineCleanTime==0){
		DeviceInfo.LineCleanTime=ConstantLineCleanTime;
	}
	if(DeviceInfo.NozzleCleanTime==0){
		DeviceInfo.NozzleCleanTime=ConstantNozzleCleanTime;
	}
	if(DeviceInfo.SterileTime==0){
		DeviceInfo.SterileTime=ConstantSterileTime;
	}


	if(DeviceInfo.fan_high_speed==0){
		DeviceInfo.fan_high_speed=ConstantBlowerFanControlPwmMax;
	}
	if(DeviceInfo.fan_low_speed==0){
		DeviceInfo.fan_low_speed=ConstantBlowerFanControlPwmMin;
	}


	//패스워드 불러오기
	for(int i=1;i<6;i++){
		for(int j=0;j<4;j++){
			IDLIST.PW[i][j]=userConfig[AccountPassword_DATA+4*(i-1)+j];
		}
		IDLIST.year[i] = userConfig[AccountLastLogin_DATA+5*(i-1)];
		IDLIST.month[i] = userConfig[AccountLastLogin_DATA+1+5*(i-1)];
		IDLIST.day[i] = userConfig[AccountLastLogin_DATA+2+5*(i-1)];
		IDLIST.hour[i] = userConfig[AccountLastLogin_DATA+3+5*(i-1)];
		IDLIST.minute[i] = userConfig[AccountLastLogin_DATA+4+5*(i-1)];
		IDLIST.Attempts[i] = userConfig[AccountAttempts_DATA+(i-1)];
		IDLIST.Status[i] = userConfig[AccountStatus_DATA+(i-1)];
	}



	//히스토리 정보 불러오기
	for(int i=1;i<5;i++){
		startData.year[i] = userConfig[LOG_DATA+((i-1)*14)];
		startData.month[i] = userConfig[LOG_DATA+1+((i-1)*14)];
		startData.day[i] = userConfig[LOG_DATA+2+((i-1)*14)];
		startData.hour[i] = userConfig[LOG_DATA+3+((i-1)*14)];
		startData.minute[i] = userConfig[LOG_DATA+4+((i-1)*14)];
		startData.cubic[i] = ((userConfig[LOG_DATA+5+((i-1)*14)])*100)+(userConfig[LOG_DATA+6+((i-1)*14)]);

		endData.year[i] = userConfig[LOG_DATA+7+((i-1)*14)];
		endData.month[i] = userConfig[LOG_DATA+8+((i-1)*14)];
		endData.day[i] = userConfig[LOG_DATA+9+((i-1)*14)];
		endData.hour[i] = userConfig[LOG_DATA+10+((i-1)*14)];
		endData.minute[i] = userConfig[LOG_DATA+11+((i-1)*14)];
		startData.status[i] = userConfig[LOG_DATA+12+((i-1)*14)];
		startData.ID[i] = userConfig[LOG_DATA+13+((i-1)*14)];
	}

	DeviceInfo.loginonoff_flag=userConfig[loginonoff_flag_DATA];
	DeviceInfo.reservationonoff_flag=userConfig[reservationonoff_flag_DATA];

}

void Write_LogData_Flash(){
	int i;
	unsigned char ucData2[1040];

	//userLogData[4][650];
	/*
	f_data{
	unsigned char	temperature, humidity;
	float		  	density;
	float			volume;
	}
	float2char(float인 데이터, ucData+주소[4단위]);
	ucData[주소]=char 데이터;
	*/
	for(int ii=0;ii<65;ii++){
		ucData2[ii*16]=f_data[ii].year;
		ucData2[ii*16+1]=f_data[ii].month;
		ucData2[ii*16+2]=f_data[ii].day;
		ucData2[ii*16+3]=f_data[ii].hour;
		ucData2[ii*16+4]=f_data[ii].minute;
		ucData2[ii*16+5]=f_data[ii].second;
		ucData2[ii*16+6]=f_data[ii].temperature;
		ucData2[ii*16+7]=f_data[ii].humidity;
		float2char(f_data[ii].density, ucData2+ii*16+8);
		float2char(f_data[ii].volume, ucData2+ii*16+12);
	}

	//__attribute__((__section__(".user_log_data1"))) const char userLogData1[650];
	if(IndexEndLog==1){
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
		FLASH_Erase_Sector(FLASH_SECTOR_7, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
				 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData1[0] + i, ucData2[i]);

		}
		HAL_FLASH_Lock();
    }
	else if(IndexEndLog==2){
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
		FLASH_Erase_Sector(FLASH_SECTOR_8, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
				 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData2[0] + i, ucData2[i]);

		}
		HAL_FLASH_Lock();
    }
	else if(IndexEndLog==3){
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
		FLASH_Erase_Sector(FLASH_SECTOR_9, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
				 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData3[0] + i, ucData2[i]);

		}
		HAL_FLASH_Lock();
    }
	else if(IndexEndLog==4){
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
		FLASH_Erase_Sector(FLASH_SECTOR_10, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
				 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData4[0] + i, ucData2[i]);

		}
		HAL_FLASH_Lock();
    }
	else if(IndexEndLog==5){
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
		FLASH_Erase_Sector(FLASH_SECTOR_10, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
				 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData4[0] + i, ucData2[i]);

		}
		HAL_FLASH_Lock();

    }
}

void Write_Shift_LogData_Flash(){
	int i;
	unsigned char ucData2[1040];
	unsigned char ucData3[1040];

	//userLogData[4][650];
	/*
	f_data{
	unsigned char	temperature, humidity;
	float		  	density;
	float			volume;
	}
	float2char(float인 데이터, ucData+주소[4단위]);
	ucData[주소]=char 데이터;
	*/
	for(int ii=0;ii<65;ii++){
		ucData2[ii*16]=f_data[ii].year;
		ucData2[ii*16+1]=f_data[ii].month;
		ucData2[ii*16+2]=f_data[ii].day;
		ucData2[ii*16+3]=f_data[ii].hour;
		ucData2[ii*16+4]=f_data[ii].minute;
		ucData2[ii*16+5]=f_data[ii].second;
		ucData2[ii*16+6]=f_data[ii].temperature;
		ucData2[ii*16+7]=f_data[ii].humidity;
		float2char(f_data[ii].density, ucData2+ii*16+8);
		float2char(f_data[ii].volume, ucData2+ii*16+12);
	}

	//__attribute__((__section__(".user_log_data1"))) const char userLogData1[650];
	if(IndexEndLog==1){
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
		FLASH_Erase_Sector(FLASH_SECTOR_7, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
				 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData1[0] + i, ucData2[i]);

		}
		HAL_FLASH_Lock();
    }
	else if(IndexEndLog==2){
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
		FLASH_Erase_Sector(FLASH_SECTOR_8, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
				 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData2[0] + i, ucData2[i]);

		}
		HAL_FLASH_Lock();
    }
	else if(IndexEndLog==3){
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
		FLASH_Erase_Sector(FLASH_SECTOR_9, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
				 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData3[0] + i, ucData2[i]);

		}
		HAL_FLASH_Lock();
    }
	else if(IndexEndLog==4){
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
		FLASH_Erase_Sector(FLASH_SECTOR_10, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
				 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData4[0] + i, ucData2[i]);

		}
		HAL_FLASH_Lock();
    }
	else if(IndexEndLog==5){
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
		//2->1
		FLASH_Erase_Sector(FLASH_SECTOR_7, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
			ucData3[i]=userLogData2[i];
			HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData1[0] + i, ucData3[i]);
		}
		//3->2
		FLASH_Erase_Sector(FLASH_SECTOR_8, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
			ucData3[i]=userLogData3[i];
			HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData2[0] + i, ucData3[i]);
		}
		//4->3
		FLASH_Erase_Sector(FLASH_SECTOR_9, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
			ucData3[i]=userLogData4[i];
			HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData3[0] + i, ucData3[i]);
		}
		//Normal
		FLASH_Erase_Sector(FLASH_SECTOR_10, VOLTAGE_RANGE_3);
		for(i = 0; i < 1040; i++) {
				 HAL_FLASH_Program(TYPEPROGRAM_BYTE, (uint32_t)&userLogData4[0] + i, ucData2[i]);

		}
		HAL_FLASH_Lock();
    }
}
void Read_LogData_Flash(int index){
	//f_data

	/*
	f_data{
	unsigned char	temperature, humidity;
	float		  	density;
	float			volume;
	}
	float2char(float인 데이터, ucData+주소[4단위]);
	ucData[주소]=char 데이터;
	*/

	//extern struct FLData		f_data[65];

	/*
	for(int ii=0;ii<65;ii++){
		ucData2[ii*10]=f_data[ii].temperature;
		ucData2[ii*10+1]=f_data[ii].humidity;
		float2char(f_data[ii].density, ucData2+ii*10+2);
		float2char(f_data[ii].volume, ucData2+ii*10+6);
	}
	*/
	//__attribute__((__section__(".user_log_data1"))) const char userLogData1[650];
	if(index==1){
		for(int ii=0;ii<65;ii++){
			f_data[ii].year=userLogData1[ii*16];
			f_data[ii].month=userLogData1[ii*16+1];
			f_data[ii].day=userLogData1[ii*16+2];

			f_data[ii].hour=userLogData1[ii*16+3];
			f_data[ii].minute=userLogData1[ii*16+4];

			f_data[ii].second=userLogData1[ii*16+5];
			f_data[ii].temperature=userLogData1[ii*16+6];
			f_data[ii].humidity=userLogData1[ii*16+7];

			f_data[ii].density = char2float((unsigned char *)userLogData1+(ii*16+8));
			f_data[ii].volume = char2float((unsigned char *)userLogData1+(ii*16+12));

		}
	}
	else if(index==2){
		for(int ii=0;ii<65;ii++){
			f_data[ii].year=userLogData2[ii*16];
			f_data[ii].month=userLogData2[ii*16+1];
			f_data[ii].day=userLogData2[ii*16+2];

			f_data[ii].hour=userLogData2[ii*16+3];
			f_data[ii].minute=userLogData2[ii*16+4];

			f_data[ii].second=userLogData2[ii*16+5];
			f_data[ii].temperature=userLogData2[ii*16+6];
			f_data[ii].humidity=userLogData2[ii*16+7];

			f_data[ii].density = char2float((unsigned char *)userLogData2+(ii*16+8));
			f_data[ii].volume = char2float((unsigned char *)userLogData2+(ii*16+12));
		}
	}
	else if(index==3){
		for(int ii=0;ii<65;ii++){
			f_data[ii].year=userLogData3[ii*16];
			f_data[ii].month=userLogData3[ii*16+1];
			f_data[ii].day=userLogData3[ii*16+2];

			f_data[ii].hour=userLogData3[ii*16+3];
			f_data[ii].minute=userLogData3[ii*16+4];

			f_data[ii].second=userLogData3[ii*16+5];
			f_data[ii].temperature=userLogData3[ii*16+6];
			f_data[ii].humidity=userLogData3[ii*16+7];

			f_data[ii].density = char2float((unsigned char *)userLogData3+(ii*16+8));
			f_data[ii].volume = char2float((unsigned char *)userLogData3+(ii*16+12));
		}
	}
	else if(index==4){
		for(int ii=0;ii<65;ii++){
			f_data[ii].year=userLogData4[ii*16];
			f_data[ii].month=userLogData4[ii*16+1];
			f_data[ii].day=userLogData4[ii*16+2];

			f_data[ii].hour=userLogData4[ii*16+3];
			f_data[ii].minute=userLogData4[ii*16+4];

			f_data[ii].second=userLogData4[ii*16+5];
			f_data[ii].temperature=userLogData4[ii*16+6];
			f_data[ii].humidity=userLogData4[ii*16+7];

			f_data[ii].density = char2float((unsigned char *)userLogData4+(ii*16+8));
			f_data[ii].volume = char2float((unsigned char *)userLogData4+(ii*16+12));
		}
	}


}
