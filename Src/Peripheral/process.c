/*
 * Process.c
 *
 *  Created on: May 27, 2019
 *      Author: monster
 */
#include "main.h"
#include "define.h"

#include "lcd.h"
#include "i2c.h"
#include "flash.h"


#include "adc.h"
#include "AirPump.h"
#include "FanPump.h"
#include "Heater.h"
#include "PeristalticPump.h"
#include "Solenoid.h"
#include "Scrubber.h"

#include "LTE_modem.h"
#include "rfid.h"

#include "util.h"

#include "process.h"
#include "USBProcess.h"

#define IO_ACTION_GAP_MS 3000U

static uint32_t last_io_action_ms;
static IoActionType last_io_action_type = IO_ACTION_NONE;

extern unsigned int uiWaitTime[5];		// 0 : Not Used, 1 : PreHeat, 2 : Spray, 3 : Sterile, 4 : Scrub
extern volatile unsigned int uiFinishTime;
extern volatile unsigned int uiTotalTime;

extern volatile unsigned int	uiEndTimeCounter, ui1sCounter;
extern volatile unsigned char Running_Flag, EndTimer_Flag, ProcessWait_Flag;
extern unsigned char Temperature_Error_Flag, FanControl_Error_Flag;

extern float fCubic;
extern float fInjectionPerMinute,fInjectionPerMinute2;
extern float fInjectionPerCubic;
extern float fBoardTemperature;
extern float fModuleTemperature;
extern float fHumidity;
extern float fDensity;

//extern uint32_t ret;
float fUsedVolume=0;
float nUsedVolume=0;

int IndexWriteFlash=0;
int NewRFIDFlag;
int BeforeRFIDFlag=1;
int SaveLastRFID;

extern int checkret, overheatFlag;
unsigned char ProcessMode, SprayEnable_Flag, Sterile_Step, Heater_Flag;
unsigned char Sms_Flag=0;

struct data_format	g_data[1500];
int 				g_data_index;

struct RFID_format RFIDData;

struct log_format startData;
int				   startIndex;

struct log_format endData;
int				   endIndex;


struct data_format t_data;
int				   t_data_index;

struct data_format time_data;
struct data_format d_data;

struct data_format hour6_data;
struct data_format testdata;

void EnforceIoActionGap(IoActionType action_type)
{
	uint32_t now = HAL_GetTick();
	if (last_io_action_type != IO_ACTION_NONE && last_io_action_type != action_type) {
		uint32_t elapsed = now - last_io_action_ms;
		if (elapsed < IO_ACTION_GAP_MS) {
			HAL_Delay(IO_ACTION_GAP_MS - elapsed);
		}
	}
	last_io_action_ms = HAL_GetTick();
	last_io_action_type = action_type;
}

struct data_format finish_data;

int ret;
int PeristalticPumpOnOff_Flag;
int FinishTimeControl_Spary;

int FirstOneMinute=0;
int DisplayUsedVolume_flag;
int IndexEndLog;
int firstlogcall;

unsigned int uiScrubTime = 100;
unsigned int sms_enable=0;
unsigned int iTenMinuteCounter;
unsigned int iFiveMinuteCounter;
unsigned int iHourCounter=0;
unsigned int iFiveHourCounter;
unsigned int iTwoHourCounter=1;
unsigned int iOneHourCounter=0;
unsigned int FiveHourSMS;
unsigned int OneHourSMS;
unsigned int SafetyPPM;
unsigned int CheckPPM6;
unsigned int ScrubberOnOff_Flag;

int PreHeatCnt=1;
extern int arrDensity[5];
extern int StartPPM;



extern int SelectID,LOGIN_ID;

struct Account_format IDLIST;



extern int H2O2Sensor_Flag;

struct DeviceInfo_format DeviceInfo;

//23.06.01 Reservation
extern unsigned int expected_uiFinishTime;
struct data_format Reserve_data;
unsigned int uireservetime = 100;

struct FLData	f_data[65];

extern int Test_flag;
extern volatile int Test_Start_flag;
extern unsigned int TestTime;
extern int Testfanspeed;


void InitProcess(void){
	//char msg[80] = "Loading";
	//DisplayPopUpMessage(msg);
	DisplayPage(42);	//Loading
	DisplayModeIcon(0);

	InitADC();
	HAL_Delay(100);
	InitRFID();
	InitTemperature();
	Read_Flash();
	ret = ReadRFID();
	if(ret==-2){//11.04추가
		for(int i=0;i<3;i++){
			ret = ReadRFID();
			if(ret==1){
				break;
			}
		}
	}

	RFIDCompare();	//06.16
	RFIDDataLoad();
	InitFanPump();
	InitLCD();
	InitLTEModem();

	ReCalcTime();
	GetTemperatureFan(0);
	GetDensity();
	DisplayStatus();

	SetRTCFromLCD();
	g_data_index = -1;
	t_data_index = 0;
	PeristalticSpeed();

}

void ReCalcTime(){
	unsigned int uiRunningTime;
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


	uiWaitTime[1] = DeviceInfo.PreHeatTime*60*100;		// PreHeatTime
	uiWaitTime[2] = SprayCentiTime;		// Spray Time
	uiWaitTime[3] = DeviceInfo.SterileTime*60*100;// Sterile Time
	if(uiScrubTime==0){
		uiScrubTime=100;
	}
	uiWaitTime[4] = uiScrubTime;		// Scrub Time

	if(DeviceInfo.device_version==8){
		uiRunningTime = Calc_RunningTime(fCubic, fInjectionPerCubic, fInjectionPerMinute2);
	}
	else{
		uiRunningTime = Calc_RunningTime(fCubic, fInjectionPerCubic, fInjectionPerMinute);
	}
	uiWaitTime[2] = uiRunningTime * 100;
	expected_uiFinishTime= uireservetime + uiWaitTime[1] + uiWaitTime[2] + uiWaitTime[3] + uiWaitTime[4] + FinishCentiTime;	//해당부분 추가
	uiFinishTime = uiWaitTime[2] + uiWaitTime[3] + uiWaitTime[4] + FinishCentiTime;

	DisplayPreHeatTime();
	DisplaySprayTime();
	DisplaySterileTime();
	DisplayScrubTime();
	DisplayFinishTime();
	DisplayTotalTime();
}

int H2O2Check(){
	if (checkret == -2){
			DisplayPopUpMessage("Must Use CBT Sterile H2O2.");
			DisplayModeIcon(0);
			return 0;
	}
	else if(checkret == -3){
		DisplayPopUpMessage("Cannot use old Sterile.");
		DisplayModeIcon(0);
		return 0;
	}else{
		if((RFIDData.fH2O2Volume) < fCubic * fInjectionPerCubic){
			DisplayPopUpMessage("Cubic is bigger than remain. Input smaller value.");
			DisplayModeIcon(0);
			return 0;
		}
		else{
			return 1;
		}
	}
}
void H2O2Read(){
	RFIDData.fH2O2Volume=0;
	sprintf(RFIDData.CurrentRFIDValue,"%c%c%c%c",0,0,0,0);
	InitRFID();
	ret = ReadRFID();
	if(ret==-2){//11.04추가
		for(int i=0;i<3;i++){
			ret = ReadRFID();
			if(ret==1){
				break;
			}
		}
	}
	if(ret==-2){
		DisplayPopUpMessage("Please check H2O2");
	}
	else{
		DisplayPopUpMessage("H2O2 changed");
	}
	Read_Flash();

	RFIDCompare();	//06.16
	RFIDDataLoad();

	DisplayModeIcon(0);
	ReCalcTime();
	DisplayCubic();
    DisplayInjectionPerMinute();
    DisplayInjectionPerCubic();
}

void StartProcess(void){
	ProcessMode = 1;
	g_data_index = 0;
	t_data_index = 0;
	nUsedVolume=0;
	ProcessWait_Flag = 0;
	uiTotalTime = 0;

	uiEndTimeCounter = uiWaitTime[ProcessMode];		// PreHeat Time.
	DisplayModeIcon(ProcessMode);
	Running_Flag = 1;
	Sms_Flag=0;
	overheatFlag=0;
	firstlogcall=1;
	DisplayRunningPage();
	SaveStartLog();
	FirstOneMinute=1;
	iHourCounter=0;
	SafetyPPM=0;
	ScrubberOnOff_Flag=0;
	SaveUsbLog();
	OneHourSMS=0;
	FiveHourSMS=0;
	iOneHourCounter=0;
	iTenMinuteCounter=0;
	iHourCounter=0;
}

void EndProcess(void){
	//char msg[80] = "Process Closing.";
	//DisplayPopUpMessage(msg);
	DisplayPage(42);	//Loading
	SaveLog();				// Save Last Log.
//	Write_Flash();
//	LCD_WriteFlash();

	Running_Flag = 0;
//	DownloadUSB();

	FinishTimeControl_Spary=0;
	FirstOneMinute=1;
	DisplayUsedVolume_flag=0;
	ReCalcTime();
	if(ProcessMode!=1&&ProcessMode!=6){
		SaveEndLogFlash(IndexEndLog);
		Write_LogData_Flash();
	}
	SendEndMessage();
	Write_Flash();
	ProcessMode=0;

	sms_enable=0;
	PreHeatCnt=1;
	FiveHourSMS=1;
	OneHourSMS=1;
	SafetyPPM=1;
	CheckPPM6=1;
	DisplayModeIcon(0);
	DisplayOperationPage();
	if(Sms_Flag==9){
		DisplayPopUpMessage("Sterilization complete");
	}
	else{
		DisplayPopUpMessage("The sterilizer has stopped");
	}
	if(PeristalticPumpOnOff_Flag==1){// 페리 동작 후 정지 되었을때.
		DisplayPopUpMessage("The sterilizer has stopped.\n\r"
			"Please turn off and on the sterilizer.");
	}

	PeristalticPumpOnOff_Flag = 0;
	Sms_Flag=0;
	InitDisplayValues();
	FirstOneMinute=0;
}

void CancelProcess(void){
	TurnOffHeater();
	TurnOffAirPump();
	TurnOffSolenoidFluid();
	TurnOffSolenoidAir();
	SetFanPumpSpeedAllMid();
	TurnOffPeristalticPump();
	TurnOffScrubber();
	Sms_Flag=6;
	if(ProcessMode!=1){
		SaveCancelLog();
		SaveEndLogFlash(IndexEndLog);
	}
	EndProcess();
	SprayEnable_Flag=0;
	Heater_Flag=0;
	g_data_index = -1;
	t_data_index = -1;
	iHourCounter=0;
	iTenMinuteCounter=0;
}

void ProcessEndTimer(void){
	if(Running_Flag) {
		if(ProcessWait_Flag == 0) {
			uiEndTimeCounter = 100;
			if(PeristalticPumpOnOff_Flag){
				ProcessWait_Flag = 1;
			}
		}
		else {
			ProcessWait_Flag = 0;
			if(ProcessMode == 6) {	//해당부분 추가
				StartProcess();		//해당부분 추가
			}
			else if(ProcessMode == 5) {
				ProcessMode = 0;
				Sms_Flag=9;
				PeristalticPumpOnOff_Flag=0;
				EndProcess();
			}
			else if(ProcessMode == 4) {
				Sms_Flag=4;
				ProcessMode = 5;
				// Finish Process
				DisplayModeIcon(ProcessMode);
				FinishProcess();
				//PeristalticPumpOnOff_Flag = 0;
				uiEndTimeCounter = uiFinishTime;
			}
			else if(ProcessMode == 3) {
				Sms_Flag=3;
				if(Sterile_Step == 0) {
					Sterile_Step = 1;
					SterileMiddleProcess();
					uiEndTimeCounter = DeviceInfo.NozzleCleanTime*60*100;
				}
				else if(Sterile_Step == 1) {
					Sterile_Step = 2;
					SterileEndProcess();
					uiEndTimeCounter = DeviceInfo.SterileTime*60*100- DeviceInfo.LineCleanTime*60*100-DeviceInfo.NozzleCleanTime*60*100;
				}
				else if(Sterile_Step == 2) {
					ScrubProcess();
					Sms_Flag=8;
					ProcessMode = 4;
					DisplayModeIcon(ProcessMode);
					uiEndTimeCounter = uiWaitTime[ProcessMode];
				}
			}
			else if(ProcessMode == 2 ) {
				// Sterile Step 0...
				Sms_Flag=2;
				Sterile_Step = 0;
				ProcessMode = 3;
				DisplayModeIcon(ProcessMode);
				SterileProcess();
				//uiEndTimeCounter = uiWaitTime[ProcessMode];
				uiEndTimeCounter = DeviceInfo.LineCleanTime*60*100;
			}
			else if(ProcessMode == 1 ) {
				SprayProcess();
				Sms_Flag=1;
				ProcessMode = 2;
				DisplayModeIcon(ProcessMode);
				uiEndTimeCounter = uiWaitTime[ProcessMode];
			}
		}
		EndTimer_Flag = 0;
	}
}

void ProcessTestEndTimer(void){
	if(Test_Start_flag==1){
		if(ProcessWait_Flag == 0) {
			uiEndTimeCounter = 100;
			ProcessWait_Flag = 1;
		}
		else {
			ProcessWait_Flag = 0;
			if(Test_flag == 1) {	//Fan Test
				TurnOnFanPump();
				SetFanPumpSpeedAll(40);
				Test_flag=9;
				uiEndTimeCounter=TIME_FAN_TEST;
			}
			else if(Test_flag == 2) {	//Air Test
				TurnOnAirPump();
				Test_flag=9;
				uiEndTimeCounter=TIME_AIR_TEST;
			}
			else if(Test_flag == 3) {	//Solenoid Test
				TurnOnSolenoidFluid();
				TurnOnSolenoidAir();
				Test_flag=9;
				uiEndTimeCounter=TIME_SOL_TEST;
			}
			else if(Test_flag == 6) {	//Spray Test1
				TurnOnFanPump();
				TurnOnHeater();
				//TurnOnSolenoidFluid();
				//TurnOffSolenoidAir();
				TurnOnAirPump();
				TurnOnPeristalticPump();
				PeristalticSpeed();
				//SetFanPumpSpeedAllMin2();
				SetFanPumpSpeedAll(60);
				Test_flag=7;
				uiEndTimeCounter=TIME_SPRAY1_TEST;
			}
			else if(Test_flag == 7) {	//Spray Test2
				TurnOnFanPump();
				TurnOffHeater();
				TurnOnSolenoidFluid();
				//TurnOffSolenoidAir();
				TurnOnAirPump();
				TurnOnPeristalticPump();
				PeristalticSpeed();
				//SetFanPumpSpeedAllMin2();
				SetFanPumpSpeedAll(60);
				Test_flag=11;
				uiEndTimeCounter=TIME_SPRAY2_TEST;
			}

			else if(Test_flag == 8) {	//Nozzle Clean1
				TurnOnFanPump();
				TurnOnHeater();
				TurnOnSolenoidFluid();
				TurnOffSolenoidAir();
				TurnOnAirPump();
				TurnOnPeristalticPump();

				//SetFanPumpSpeedAllMin2();
				SetFanPumpSpeedAll(60);
				Test_flag=10;
				uiEndTimeCounter=TIME_CLEAN1;
			}
			else if(Test_flag == 10) {	//Nozzle Clean2
				TurnOnFanPump();
				TurnOffHeater();
				TurnOffSolenoidFluid();
				TurnOnSolenoidAir();
				TurnOnAirPump();
				TurnOffPeristalticPump();
				//SetFanPumpSpeedAllMin2();
				SetFanPumpSpeedAll(60);
				Test_flag=11;
				uiEndTimeCounter=TIME_CLEAN2;
			}

			else if(Test_flag == 9) {	//초기화
				Test_Start_flag=0;
				TurnOffFanPump();
				TurnOffHeater();
				TurnOffAirPump();
				TurnOffSolenoidFluid();
				TurnOffSolenoidAir();
				//SetFanPumpSpeedAllMin2();
				SetFanPumpSpeedAll(30);
				TurnOffPeristalticPump();
				TurnOffScrubber();
				DisplayMsg("Test Complete");
				DisplayTestComplete();
			}
			else if(Test_flag == 4) {	//PeristalticPump Test
				PeristalticSpeed();
				TurnOnPeristalticPump();
				Test_flag=11;
				uiEndTimeCounter=TIME_PERI_TEST;
			}
			else if(Test_flag == 11) {	//PeristalticPump Test
				Test_Start_flag=0;
				TurnOffFanPump();
				TurnOffHeater();
				TurnOffAirPump();
				TurnOffSolenoidFluid();
				TurnOffSolenoidAir();
				//SetFanPumpSpeedAllMin2();
				SetFanPumpSpeedAll(30);
				TurnOffPeristalticPump();
				TurnOffScrubber();
				DisplayMsg("Test complete.\n\r"
						"Please turn off and on the device.");
				DisplayTestComplete();
			}

			else if(Test_flag == 16) {	//PeristalticPump Test
				Peri_15_Speed();
				TurnOnPeristalticPump();
				Test_flag=20;
				uiEndTimeCounter=TestTime;
			}
			else if(Test_flag == 17) {	//PeristalticPump Test
				Peri_12_Speed();
				TurnOnPeristalticPump();
				Test_flag=20;
				uiEndTimeCounter=TestTime;
			}
			else if(Test_flag == 18) {	//PeristalticPump Test
				Peri_9_Speed();
				TurnOnPeristalticPump();
				Test_flag=20;
				uiEndTimeCounter=TestTime;
			}
			else if(Test_flag == 20) {	//Develop Mode Peri_finish
				Test_Start_flag=0;
				TurnOffFanPump();
				TurnOffPeristalticPump();
				//테스트 완료 페이지 추가
				DisplayMsg("Test complete.\n\r"
						"Please turn off and on the device.");
				Display55page();
			}
			else if(Test_flag== 21){	//Fan Test
				TurnOnFanPump();
				SetFanPumpSpeedAll(Testfanspeed);
				Test_flag=22;
				uiEndTimeCounter=TestTime;
			}
			else if(Test_flag == 22) {	//Fan Finish
				Test_Start_flag=0;
				TurnOffFanPump();
				//테스트 완료 페이지 추가
				DisplayMsg("Test complete.");
				Display55page();
			}


		}
		EndTimer_Flag = 0;
	}
}


// 1/100 second
void CentiSecondProcess(void)
{
}

// 1/10 second
void DeliSecondProcess(void)
{
	//GetTemperatureFan(0);
	DisplayStatus();
	GetDensity();
	/*
	if(SprayEnable_Flag) {
		AdjustBlowerFanControl(0);  // BlowerFanControlPwm1	fan 3�?모두
	}
	*/
	/*여기
	if(Clean_flag) {
		AdjustBlowerFanControl(0);  // BlowerFanControlPwm1	fan 3�?모두
	}
	*/
	if(fModuleTemperature>DeviceInfo.lower_temperature){
			PeristalticPumpOnOff_Flag = 1;
	}

	if(PeristalticPumpOnOff_Flag && SprayEnable_Flag){
			TurnOnPeristalticPump();
			DisplayUsedVolume_flag=1;
	}

}

void HalfSecondProcess(){
	GetTemperatureFan(0);
}

//unsigned int iThirtySecondCounter;
unsigned int iFiveSecondCounter;
void OneSecondProcess(void)
{
	//OverHeatTempCheck(0);
	DisplayStatus();
	GetDensity();
	if(DisplayUsedVolume_flag){
		if(DeviceInfo.device_version==8){
			RFIDData.fH2O2Volume -= (fInjectionPerMinute2/60);
			nUsedVolume += (fInjectionPerMinute2/60);
		}
		else{
			RFIDData.fH2O2Volume -= (fInjectionPerMinute/60);
			nUsedVolume += (fInjectionPerMinute/60);
		}
	}
	if(iFiveSecondCounter==4){
		iFiveSecondCounter = 0;
		FiveSecondProcess();
	}else{
		iFiveSecondCounter++;
	}
	if(Running_Flag){
		if(FirstOneMinute){
			if(PreHeatCnt==60){
				PreHeatOneMinuteProcess();
			}else{
				PreHeatCnt++;
			}
		}
	}
	if(Running_Flag==0 && fModuleTemperature < ConstantSafeTemperature&&Test_Start_flag==0){
		TurnOffFanPump();
	}
	if(Running_Flag==1 && Temperature_Error_Flag) {
		CancelProcess();
	}
}
void FiveSecondProcess(void){
	DisplayAvgDensity();
	avgOverHeatTemp();
	DisplayH2O2SensorIcon(H2O2Sensor_Flag);

	if(SprayEnable_Flag) {
		AdjustBlowerFanControl(0);  // BlowerFanControlPwm1	fan 3�?모두
		AdjustHeaterControl(10);
	}
	if(Heater_Flag){
		AdjustBlowerFanControl(2);
		AdjustHeaterControl(5);
	}
}

void SaveLog(void){
	unsigned char week;
	ReadRTC( &g_data[g_data_index].year, &g_data[g_data_index].month, &g_data[g_data_index].day, &week,
			&g_data[g_data_index].hour, &g_data[g_data_index].minute, &g_data[g_data_index].second);
	HAL_Delay(10);
	g_data[g_data_index].temperature = fBoardTemperature;
	g_data[g_data_index].humidity = fHumidity;
	g_data[g_data_index].module_temperature = fModuleTemperature;
	g_data[g_data_index].density = fDensity;
	g_data[g_data_index].volume = nUsedVolume;
	g_data_index++;
	SendProcessMessage();
	iTenMinuteCounter=0;
}

void SaveUsbLog(void){
	unsigned char week;
	ReadRTC( &t_data.year, &t_data.month, &t_data.day, &week,
			&t_data.hour, &t_data.minute, &t_data.second);
	t_data.temperature = fBoardTemperature;
	t_data.humidity = fHumidity;
	t_data.module_temperature = fModuleTemperature;
	t_data.density = fDensity;
	t_data.volume = nUsedVolume;
	t_data_index++;
	if(t_data_index>=64){
		t_data_index=64;
	}
	if(Running_Flag){
		f_data[t_data_index].year=t_data.year;
		f_data[t_data_index].month=t_data.month;;
		f_data[t_data_index].day=t_data.day;

		f_data[t_data_index].hour=t_data.hour;
		f_data[t_data_index].minute=t_data.minute;
		f_data[t_data_index].temperature=fBoardTemperature;
		f_data[t_data_index].humidity = fHumidity;
		f_data[t_data_index].density = fDensity;
		f_data[t_data_index].volume = nUsedVolume;
		DisplayDebug("USB_SAVE");
		DownloadUSB();
		DisplayDebug("");
	}
}

void TestSaveUsbLog(void){
	unsigned char week;
	ReadRTC( &t_data.year, &t_data.month, &t_data.day, &week,
			&t_data.hour, &t_data.minute, &t_data.second);
	t_data.temperature = fBoardTemperature;
	t_data.humidity = fHumidity;
	t_data.module_temperature = fModuleTemperature;
	t_data.density = fDensity;
	t_data.volume = nUsedVolume;
	USBTEST();
}



/*
void SaveStartLog(void){
	unsigned char week;
	ReadRTC( &startData.year, &startData.month, &startData.day, &week,
			&startData.hour, &startData.minute, &startData.second);
	startData.temperature = fBoardTemperature;
	startData.humidity = fHumidity;
	startData.volume = 0;
	StartPPM = fDensity;
	startIndex++;
}
void SaveEndLog(void){
	unsigned char week;
	ReadRTC(&endData.year, &endData.month, &endData.day, &week,
			&endData.hour, &endData.minute, &endData.second);
	endData.temperature = fBoardTemperature;
	endData.humidity = fHumidity;
	endData.volume = nUsedVolume;
	endIndex++;
	IndexEndLog++;
}
*/
void SaveStartLog(void){
	unsigned char week, second;
	ReadRTC(&startData.year[0], &startData.month[0], &startData.day[0], &week,
			&startData.hour[0], &startData.minute[0], &second);
	startData.status[0] = 1;
	startData.cubic[0] = fCubic;
	startData.temperature = fBoardTemperature;
	startData.humidity = fHumidity;
	startData.volume = 0;
	startData.ID[0] = LOGIN_ID;
	StartPPM = fDensity;
	startIndex++;
}
void SaveEndLog(void){
	unsigned char week, second;
	ReadRTC(&endData.year[0], &endData.month[0], &endData.day[0], &week,
			&endData.hour[0], &endData.minute[0], &second);
	startData.status[0] = 3;
	endData.temperature = fBoardTemperature;
	endData.humidity = fHumidity;
	endData.volume = nUsedVolume;
	endIndex++;
	//IndexEndLog++;
}

void SavePPM(int hour){
    testdata.temperature = fBoardTemperature;
    testdata.humidity = fHumidity;
    testdata.module_temperature = fModuleTemperature;
    testdata.density = fDensity;
    testdata.volume = nUsedVolume;
    SendCurrentPPM(hour);
}

void SaveFinishTime(){		//?�작??종료?�상?�간 메시지 ?�송
	unsigned char week;
	ReadRTC(&finish_data.year, &finish_data.month, &finish_data.day, &week,
			&finish_data.hour, &finish_data.minute, &finish_data.second);
	unsigned int smsFinishTime;
	smsFinishTime = uiFinishTime+ 99;

    unsigned int iMinute = smsFinishTime / 6000;
    unsigned int iHour = iMinute / 60;
    iMinute = iMinute - (iHour * 60);
    unsigned char smsHour, smsMinute;

    smsHour = hex2bcd(iHour)+finish_data.hour;   // Hour
    smsMinute = hex2bcd(iMinute)+finish_data.minute; // Minute

    if(smsMinute >= 0x5a && smsMinute < 0x60 ){	//	0x5a~0x5f ?�이??처리
    	smsMinute = smsMinute - 0x5a;
    	smsHour = smsHour + 0x01;
    }
    else if(smsMinute >= 0x60){
    	smsMinute = smsMinute - 0x60;
    	smsHour = smsHour + 0x01;
    }
    smsHour = TimeConvert(smsHour);
    smsMinute = TimeConvert(smsMinute);
    SendFinishTime(smsHour, smsMinute);
}
unsigned char TimeConvert(unsigned char time){		//16진수 0xa~0xf �?변??

	unsigned char cTime;
	if((time>=0x0a && time<=0x0f) ||(time>=0x1a && time<=0x1f) ||(time>=0x2a && time<=0x2f)
			||(time>=0x3a && time<=0x3f) ||(time>=0x4a && time<=0x4f) ||(time>=0x5a && time<=0x5f)){
		cTime = (time - 0x0a)+0x10;
	}else{
		cTime = time;
	}

	return cTime;
}
void TimeCounterProcess(void){
	time_data.second++;
	if (time_data.second == 0x0a){
		time_data.second = 0x10;
	}else if (time_data.second == 0x1a){
		time_data.second = 0x20;
	}else if (time_data.second == 0x2a){
		time_data.second = 0x30;
	}else if (time_data.second == 0x3a){
		time_data.second = 0x40;
	}else if (time_data.second == 0x4a){
		time_data.second = 0x50;
	}else if(time_data.second == 0x5a){
		time_data.second = 0x00;
		time_data.minute++;
	}

	if(time_data.minute == 0x0a){
		time_data.minute = 0x10;
	}else if(time_data.minute == 0x1a){
		time_data.minute = 0x20;
	}else if(time_data.minute == 0x2a){
		time_data.minute = 0x30;
	}else if(time_data.minute == 0x3a){
		time_data.minute = 0x40;
	}else if(time_data.minute == 0x4a){
		time_data.minute = 0x50;
	}else if(time_data.minute == 0x5a){
		time_data.minute = 0x00;
		time_data.hour++;
	}

	if(time_data.hour == 0x0a){
		time_data.hour = 0x10;
	}else if(time_data.hour == 0x1a){
		time_data.hour = 0x20;
	}else if(time_data.hour == 0x24){
		time_data.hour = 0x00;
	}
}
void CheckHour6(void){			//?�정?�간 메시지 ?�송
	unsigned char week;
	ReadRTC( &hour6_data.year, &hour6_data.month, &hour6_data.day, &week,
				&hour6_data.hour, &hour6_data.minute, &hour6_data.second);
	if(hour6_data.hour == 0x06){
		if(hour6_data.minute == 0x00){
			SavePPM(0);
			CheckPPM6=0;
		}
	}
}


void OneMinuteProcess(void)
{
//	GetDensity();

	if(Running_Flag) {
//		SaveLog();
//		SaveUsbLog();
		if(FirstOneMinute){
//			PreHeatOneMinuteProcess();
		}
		if(SprayEnable_Flag){
//		if(PeristalticPumpOnOff_Flag){
//			fH2O2Volume -= fInjectionPerMinute;
//			nUsedVolume += fInjectionPerMinute;
		}
//		fUsedVolume += fInjectionPerMinute;
		if(iFiveMinuteCounter==4){
			iFiveMinuteCounter=0;
			FiveMinuteProcess();
		}else{
			iFiveMinuteCounter++;
		}

		if(iTenMinuteCounter == 9) {
			iTenMinuteCounter = 0;
			TenMinuteProcess();
		} else{
			iTenMinuteCounter ++;
		}
	}else {
		if(iTenMinuteCounter == 9) {
			iTenMinuteCounter = 0;
			TenMinuteProcess();
		} else{
			iTenMinuteCounter ++;
		}
//		iTenMinuteCounter = 0;
//		iFiveMinuteCounter = 0;
		if(CheckPPM6==1){
			CheckHour6();
		}
	}
	if(DisplayUsedVolume_flag){	//1분마다 log 저장
		DisplayDebug("Saving..");
		SaveActionLog();
		SaveEndLogFlash(IndexEndLog);
		Write_LogData_Flash();
		Write_Flash();
		DisplayDebug("");
	}

}

void TenMinuteProcess()
{
	/*if(SprayEnable_Flag) {
//		fH2O2Volume -= (fInjectionPerMinute*10);
//		fUsedVolume += (fInjectionPerMinute*10);
	}*/
	if(Running_Flag){
		if(sms_enable){
			if(ScrubberOnOff_Flag==0){
				SaveLog();
			}
		}
        if(iHourCounter == 5){
            OneHourProcess();
            iHourCounter=0;
        }else{
            iHourCounter++;
        }

	}else{
        if(iHourCounter == 5){
            OneHourProcess();
            iHourCounter=0;
        }else{
            iHourCounter++;
        }
	}
}

void FiveMinuteProcess(){
//	SaveLog();
	if((Running_Flag==1)&&(ProcessMode!=6)){
		SaveUsbLog();
	}

}

void OneHourProcess(){
	/*if(OneHourSMS == 1){
		SavePPM(1);
		OneHourSMS=0;
	}*/
	if(ScrubberOnOff_Flag==1){
		SaveLog();
	}
	if(OneHourSMS == 1){
		if(iOneHourCounter>=0&&iOneHourCounter<=15){
			iOneHourCounter++;
			SavePPM(iOneHourCounter);
			if(iOneHourCounter == 15){
				OneHourSMS=0;
			}
		}
		/*
		if(iTwoHourCounter == 2 || iTwoHourCounter == 4 || iTwoHourCounter == 6){
			TwoHourProcess(iTwoHourCounter);
			iTwoHourCounter++;
			if(iTwoHourCounter == 6){
				OneHourSMS=0;
			}
		}else{
			iTwoHourCounter++;
		}
		*/
	}

	if(FiveHourSMS==1){
		if(iFiveHourCounter==4){
//			FiveHourProcess();
			iFiveHourCounter=0;
		}else{
			iFiveHourCounter++;
		}
	}
}
void TwoHourProcess(unsigned int hour){
	SavePPM(hour);
}
void FiveHourProcess(){
    SavePPM(5);
    FiveHourSMS=0;
}

void PreHeatOneMinuteProcess(void){
	TurnOnAirPump();
	TurnOnFanPump();
	SetFanPumpSpeedAllMin();
	TurnOnHeater();
	Heater_Flag = 1;
	FirstOneMinute=0;
	TurnOnSolenoidAir();
}

void SprayProcess(void){
	TurnOffSolenoidAir();
	sms_enable=1;
	SaveFinishTime();
	FinishTimeControl_Spary=1;
	Heater_Flag = 0;
	Running_Flag = 0;
	TurnOnHeater();
	Running_Flag = 1;
	SprayEnable_Flag = 1;
	ui1sCounter = 0;
	SaveLog();				// Save Start Log.

	IndexEndLog++;
	if(IndexEndLog>=5){
		IndexEndLog=5;
	}
	//SaveStartLog();//11.03 추가
	SaveActionLog();
	SaveStartLogFlash(IndexEndLog);
	SaveEndLogFlash(IndexEndLog);
	Write_Shift_LogData_Flash();
}

void SterileProcess(void)
{
//	HAL_Delay(10000);
	float correctedUsedVolume = fCubic * fInjectionPerCubic;
	float estimatedStartVolume = RFIDData.fH2O2Volume + nUsedVolume;
	if(correctedUsedVolume < 0){
		correctedUsedVolume = 0;
	}
	nUsedVolume = correctedUsedVolume;
	RFIDData.fH2O2Volume = estimatedStartVolume - correctedUsedVolume;
	if(RFIDData.fH2O2Volume < 0){
		RFIDData.fH2O2Volume = 0;
	}

	SprayEnable_Flag = 0;
	SaveLog();
	TurnOnSolenoidFluid();
//	SetFanPumpSpeedAllMax();
	SetFanPumpSpeedAllMid();
//	TurnOffHeater();
	Heater_Flag = 1;
	DisplayUsedVolume_flag=0;
}

void SterileMiddleProcess(void){
	Heater_Flag = 1;
	TurnOnSolenoidAir();
	SetFanPumpSpeedAllMin();
	TurnOffSolenoidFluid();
	TurnOffPeristalticPump();
}

void SterileEndProcess(void){
	SetFanPumpSpeedAllMid();
	Heater_Flag=0;
	TurnOffHeater();
	TurnOffAirPump();
	TurnOffSolenoidFluid();
	TurnOffPeristalticPump();
	TurnOffSolenoidAir();
}

void ScrubProcess(void)
{
	ScrubberOnOff_Flag=1;
	iHourCounter=0;
	Sms_Flag=7;
	SaveLog();
	TurnOnScrubber();
}

void FinishProcess(void)
{
	ScrubberOnOff_Flag=0;
	TurnOffScrubber();
	TurnOffFanPump();
	SaveLog();
	SaveEndLog();//11.03 추가
	SaveEndLogFlash(IndexEndLog);
	//TurnOffPeristalticPump();
}


//추가 기능

void DeviceSettingReset(){
	Reset_Setting_Flash();
}
void DeviceSettingResetAll(){
	Reset_All_Flash();
}


void SaveCancelLog(void){
	unsigned char week, second;
	ReadRTC(&endData.year[0], &endData.month[0], &endData.day[0], &week,
			&endData.hour[0], &endData.minute[0], &second);
	startData.status[0] = 2;
}
void SaveActionLog(void){
	unsigned char week, second;
	ReadRTC(&endData.year[0], &endData.month[0], &endData.day[0], &week,
			&endData.hour[0], &endData.minute[0], &second);
	startData.status[0] = 1;
}

void SaveStartLogFlash(int index){
	if(index==5){
		for(int i=1;i<4;i++){
			startData.ID[i]=startData.ID[i+1];
			startData.year[i]=startData.year[i+1];
			startData.month[i]=startData.month[i+1];
			startData.day[i]=startData.day[i+1];
			startData.hour[i]=startData.hour[i+1];
			startData.minute[i]=startData.minute[i+1];
			//startData.status[i]=startData.status[i+1];
			startData.cubic[i]=startData.cubic[i+1];
		}
		startData.ID[4]=startData.ID[0];
		startData.year[4]=startData.year[0];
		startData.month[4]=startData.month[0];
		startData.day[4]=startData.day[0];
		startData.hour[4]=startData.hour[0];
		startData.minute[4]=startData.minute[0];
		//startData.status[4]=startData.status[0];
		startData.cubic[4]=startData.cubic[0];

	}
	else{
		startData.ID[index]=startData.ID[0];
		startData.year[index]=startData.year[0];
		startData.month[index]=startData.month[0];
		startData.day[index]=startData.day[0];
		startData.hour[index]=startData.hour[0];
		startData.minute[index]=startData.minute[0];
		//startData.status[index]=startData.status[0];
		startData.cubic[index]=startData.cubic[0];
	}
}
/* 수정중
void SaveStartLogFlash(int index){
	startData.ID[index]=startData.ID[0];
	startData.year[index]=startData.year[0];
	startData.month[index]=startData.month[0];
	startData.day[index]=startData.day[0];
	startData.hour[index]=startData.hour[0];
	startData.minute[index]=startData.minute[0];
	startData.status[index]=startData.status[0];
	startData.cubic[index]=startData.cubic[0];
}
*/
void SaveEndLogFlash(int index){
	if(index==5){
		if(ProcessMode==1){
			for(int i=1;i<4;i++){
				endData.year[i]=endData.year[i+1];
				endData.month[i]=endData.month[i+1];
				endData.day[i]=endData.day[i+1];
				endData.hour[i]=endData.hour[i+1];
				endData.minute[i]=endData.minute[i+1];
				startData.status[i]=startData.status[i+1];
			}
		}
		endData.year[4]=endData.year[0];
		endData.month[4]=endData.month[0];
		endData.day[4]=endData.day[0];
		endData.hour[4]=endData.hour[0];
		endData.minute[4]=endData.minute[0];
		startData.status[4]=startData.status[0];
	}
		else{
		endData.year[index]=endData.year[0];
		endData.month[index]=endData.month[0];
		endData.day[index]=endData.day[0];
		endData.hour[index]=endData.hour[0];
		endData.minute[index]=endData.minute[0];
		startData.status[index]=startData.status[0];
		}
}


void RFIDCompare(){
	/*RFID와 FLASH에 저장된 RFID 비교*/
	IndexWriteFlash=0;
	NewRFIDFlag=0;
	int comaparerf=0;
	if(checkret !=-2){
		for(int i=1;i<5;i++){
			for(int j=0;j<4;j++){
				if((unsigned char *)RFIDData.CurrentRFIDValue[j] == (unsigned char *)RFIDData.RFIDValue[i][j]){		//한 글자씩 비교
					comaparerf++;
				}else{

					break;
				}
			}
			if(comaparerf == 4){			//4글자가 다 같을 경우
				IndexWriteFlash=i;
				comaparerf=0;
				NewRFIDFlag=0;
				RFIDData.fH2O2Volume=RFIDData.H2O2Volume[i];
				break;
			}
			else{
				NewRFIDFlag=1;
				IndexWriteFlash=BeforeRFIDFlag;
				comaparerf=0;
			}
		}

	}
}
void RFIDDataLoad(){
	if(checkret !=-2){
		RFIDData.fH2O2Volume=RFIDData.H2O2Volume[IndexWriteFlash];
	}
	else{
		RFIDData.fH2O2Volume=0;
	}
	/*Index에 따른 사용량 불러오기*/

	if(NewRFIDFlag==1 && checkret !=-2){
		/*이전 사용한 RFID 10개 비교*/
		if (RFIDData.CurrentRFIDValue[0] == 0x33 ) {
			RFIDData.fH2O2Volume = 3000;
		} else if (RFIDData.CurrentRFIDValue[0] == 0x32 ) {
			RFIDData.fH2O2Volume = 2000;
		} else if (RFIDData.CurrentRFIDValue[0] == 0x31 ) {
			RFIDData.fH2O2Volume = 1000;
		} else if (RFIDData.CurrentRFIDValue[0] == 0x35 ) {
			RFIDData.fH2O2Volume = 500;
		}
	}
	else{

	}
}


void SaveLastLoginLog(void){
	unsigned char week, second;
	ReadRTC(&IDLIST.year[LOGIN_ID], &IDLIST.month[LOGIN_ID], &IDLIST.day[LOGIN_ID], &week,
			&IDLIST.hour[LOGIN_ID], &IDLIST.minute[LOGIN_ID], &second);
}


//23.06.01 Reservation
void ReserveProcess(){
	uiEndTimeCounter=uireservetime;
}
void SaveExpectFinishTime(){		//?�작??종료?�상?�간 메시지 ?�송
	unsigned char week;
	ReadRTC(&finish_data.year, &finish_data.month, &finish_data.day, &week,
			&finish_data.hour, &finish_data.minute, &finish_data.second);
	unsigned int smsFinishTime;
	smsFinishTime = expected_uiFinishTime+ 99;

    unsigned int iMinute = smsFinishTime / 6000;
    unsigned int iHour = iMinute / 60;
    iMinute = iMinute - (iHour * 60);
    unsigned char smsHour, smsMinute;

    smsHour = hex2bcd(iHour)+finish_data.hour;   // Hour
    smsMinute = hex2bcd(iMinute)+finish_data.minute; // Minute

    if(smsMinute >= 0x5a && smsMinute < 0x60 ){	//	0x5a~0x5f ?�이??처리
    	smsMinute = smsMinute - 0x5a;
    	smsHour = smsHour + 0x01;
    }
    else if(smsMinute >= 0x60){
    	smsMinute = smsMinute - 0x60;
    	smsHour = smsHour + 0x01;
    }
    smsHour = TimeConvert(smsHour);
    smsMinute = TimeConvert(smsMinute);
    SendFinishTime(smsHour, smsMinute);
}

void SaveReserveTime(){		//
	unsigned char week;
	ReadRTC(&Reserve_data.year, &Reserve_data.month, &Reserve_data.day, &week,
			&Reserve_data.hour, &Reserve_data.minute, &Reserve_data.second);
	unsigned int smsFinishTime;
	smsFinishTime = uireservetime+ 99;

    unsigned int iMinute = smsFinishTime / 6000;
    unsigned int iHour = iMinute / 60;
    iMinute = iMinute - (iHour * 60);
    unsigned char smsHour, smsMinute;

    smsHour = hex2bcd(iHour)+Reserve_data.hour;   // Hour
    smsMinute = hex2bcd(iMinute)+Reserve_data.minute; // Minute

    if(smsMinute >= 0x5a && smsMinute < 0x60 ){	//	0x5a~0x5f ?�이??처리
    	smsMinute = smsMinute - 0x5a;
    	smsHour = smsHour + 0x01;
    }
    else if(smsMinute >= 0x60){
    	smsMinute = smsMinute - 0x60;
    	smsHour = smsHour + 0x01;
    }
    smsHour = TimeConvert(smsHour);
    smsMinute = TimeConvert(smsMinute);
    SendReserveTime(smsHour, smsMinute);
}
