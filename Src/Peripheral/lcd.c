/*
 * lcd.c
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "define.h"
#include "util.h"
#include "i2c.h"
#include "rfid.h"
#include "process.h"
#include "USBProcess.h"
#include "rfid.h"
#include "lcd.h"
#include "LTE_Modem.h"

#include "flash.h"

#include "adc.h"
#include "AirPump.h"
#include "FanPump.h"
#include "Heater.h"
#include "PeristalticPump.h"
#include "Solenoid.h"
#include "Scrubber.h"


extern UART_HandleTypeDef huart1;
extern RTC_HandleTypeDef hrtc;

extern unsigned char uart1_rx_data[20];
unsigned char uart1_rx_data_2[100];
extern volatile unsigned int  ui1sCounter;
extern unsigned int g_data_index;//, g_data_serial_index;
extern volatile unsigned char Running_Flag, UART_Receive_Flag;

int PeristalticPumpCnt=0;

//LCD page

#define LCD_LOGIN_ON_FIRST 				2
#define LCD_LOGIN_OFF_FIRST				70
//#define LCD_FIRST    			2
#define LCD_LOGIN_FIRST_PAGE				3
#define LCD_LOGIN_SELECT_ID				4
#define LCD_LOGIN_WRONG_PW				5
#define LCD_SELECTED_ID						6
#define LCD_INPUTED_PW						7
#define LCD_SELECT_MENU					8
#define LCD_SELECT_ERROR					9

#define LCD_OPERATTION_NORMAL			10
#define LCD_OPERATTION_RUNNING			12
#define LCD_OPERATTION_MESSAGE		13

#define LCD_USER_SETTING					16
#define LCD_SUSER_SETTING					17
#define LCD_ADMIN_SETTING					18
#define LCD_LOGINOFF_SETTING				19

#define SMS_SETTING_POPUP					20
#define MOBILE_SETTING_POPUP			21
#define USB_SETTING_POPUP					22
#define RESET_SETTING_POPUP				23

#define CHANGE_PASSWORD1_POPUP		24
#define CHANGE_PASSWORD2_POPUP		25
#define CHANGE_PASSWORD3_POPUP		26
#define CHANGE_PASSWORD4_POPUP		27

#define ACCOUNT_SETTING						28
#define ACCOUNT_POPUP1						29
#define ACCOUNT_POPUP2						30
#define HISTORY_PAGE							31
#define H2O2_List									32

#define LCD_MAINTENANCE_PAGE			35
#define LCD_MAINTENANCE_YESNO			36
#define LCD_MAINTENANCE_CONFIRM		37
#define LCD_MAINTENANCE_TEST			38

#define Loading_PAGE							42

#define LCD_DEVELOPER_PAGE				43
#define CHANGE_device_version_PAGE		44
#define SMS_SERVER_NOTICE_PAGE		45
#define CHANGE_SERIAL_PAGE				46
#define CHANGE_MODEM_PAGE				47


#define LOGIN_ONOFF_PAGE					50

#define CHANGE_VALUE_PAGE					51
#define PERI_TEST_PAGE						52
#define FAN_TEST_PAGE							53
#define TEST_YES_PAGE							54
#define TEST_COMPLETE_PAGE				55
#define SETTING_RESET_POPUP_PAGE		56



#define LCD_LOGIN_OFF_WRONG_PW		71
#define RESET_ALL_PAGE						72
//#define CHANGE_PERI_TEST_YES_PAGE	73
#define RESERVATION_ONOFF_PAGE			74
#define LCD_RESERVEPOPUP_PAGE     	75
#define LCD_RESERVESETTING_PAGE   	76
#define LCD_RESERVETIME_PAGE    		77
#define LCD_SOLUTIONCHECK_PopUp		78


//MAX VP : 0000~6FFF



#define ID0					0
#define USER1				1
#define USER2				2
#define USER3				3
#define SUPERUSER		4
#define ADMIN				5
#define MAINTENANCE	6
#define DEVLOPER			7

unsigned char start_page[7] = {0x5A, 0xA5, 0x04, 0x80, 0x03, 0x00, 0x02};
unsigned char   main_page[7] = {0x5A, 0xA5, 0x04, 0x80, 0x03, 0x00, 0x0a};

unsigned char   value_display[10] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned char   serial_display[17] = {0x5A, 0xA5, 0x0e, 0x82, 0x20, 0x41, 0x00, 0x00, 0x00};
unsigned char   Loginstatus_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x20, 0x01, 0x00, 0x00, 0x00};
unsigned char   ModemNum_display[18] = {0x5A, 0xA5, 0x0f, 0x82, 0x20, 0x11, 0x00, 0x00, 0x00};
unsigned char   UserNum_display[18] = {0x5A, 0xA5, 0x0f, 0x82, 0x20, 0x21, 0x00, 0x00, 0x00};
unsigned char   UserNum_blank_display[16] = {0x5A, 0xA5, 0x09, 0x82, 0x02, 0x74, 0x00, 0x00, 0x00};
unsigned char   version_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x20, 0x30, 0x00, 0x00, 0x00};

const unsigned char rtc_setting[13] = {0x5A, 0xA5, 0x0a, 0x80, 0x1f, 0x5a, 0x19, 0x06, 0x11, 0x03, 0x10, 0x26, 0x00};
const unsigned char rtc_date_get[6] = {0x5A, 0xA5, 0x03, 0x81, 0x20, 0x03};
const unsigned char rtc_time_get[6] = {0x5A, 0xA5, 0x03, 0x81, 0x24, 0x03};


unsigned char   value_index[8] = {0x24, 0x28, 0x34, 0x44, 0x54};

unsigned char   user_value_display[10] = {0x5A, 0xA5, 0x05, 0x82, 0x02, 0x00, 0x00, 0x00, 0x00};
unsigned char   user_value_index[8] = {0x14, 0x24, 0x34};

unsigned char   icon_display[8] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x60, 0x00, 0x01};
unsigned char   icon_index[15] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0};

//0xE0, 0xE1, 0xE2, 0xE3, 0xF0를 변환
//13,14,15,16,17
unsigned char   icon_index2[4] = {0x10,0x20,0x30,0x40};


unsigned char   error_display[100] = {0x5a, 0xa5, 0x53, 0x82, 0x01, 0x00, 0x4e, 0x75, 0x72, 0x69, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d};



extern unsigned char serialNum[13];
extern unsigned char szStartCommand[37],szStartCommandCBT[37];	//�Ǹ�ó ����ó �Է�

unsigned int uiWaitTime[5];
volatile unsigned int uiFinishTime;
volatile unsigned int uiTotalTime;
extern unsigned int scrubbingTime;
extern float fBoardTemperature, fHumidity, fModuleTemperature;
float fCubic=0;
float fInjectionPerMinute=0;
float fInjectionPerMinute2=0;
float fInjectionPerCubic=0;
int TestfInjectionPerMinute=0;
int Testfanspeed=0;
float fDensity;

struct data_format ggdata[10];
extern int ret;
extern int PeristalticPumpOnOff_Flag;
extern unsigned int uiScrubTime;

extern struct log_format startData;
extern struct log_format endData;

extern struct DeviceInfo_format DeviceInfo;

//Login
int Savepassword[4]={};
int Masterpassword[4]={4,8,5,0};

int SelectID,LOGIN_ID=0;

int changeusernum1,changeusernum2,changeusernum3;//번호 임시 저장
extern char user_number1[4],user_number2[4];
char InputPassword[4];
int PasswordOK_Flag=0;

extern int SMSonoff_Flag;
extern int USBSECURITYonoff_Flag;


//History data
unsigned char   Time_log_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x01, 0x00, 0x00, 0x00, 0x00};

unsigned char   status_log_index[5] = {0x00, 0x60, 0x70, 0x80, 0x90};
unsigned char   ID_log_index[5] = {0x00, 0xA0, 0xB0, 0xC0, 0xD0};

unsigned char   log_value_display[8] = {0x5A, 0xA5, 0x05, 0x82, 0x01, 0x00, 0x00, 0x00};
unsigned char   status_log_display[14] = {0x5A, 0xA5, 0x0b, 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char   ID_log_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x01, 0x00, 0x00, 0x00, 0x00};

//H2O2 Log data
unsigned char   H2O2RFID_log_index[5] = {0x00, 0x05, 0x10, 0x15, 0x20};
unsigned char   H2O2Volume_log_index[5] = {0x00, 0x25, 0x30, 0x35, 0x40};
unsigned char   H2O2Date_log_index[5] = {0x00, 0x45, 0x50, 0x55, 0x60};

unsigned char   H2O2RFID_log_display[9] = {0x5A, 0xA5, 0x06, 0x82, 0x04, 0x00, 0x00, 0x00, 0x00};
unsigned char   H2O2Volume_log_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x04, 0x00, 0x00, 0x00, 0x00};
unsigned char   H2O2Date_log_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x04, 0x00, 0x00, 0x00, 0x00};

extern struct RFID_format RFIDData;

//Account Settings
unsigned char   Account_ID_log_index[5] = {0x00, 0x00,0x05,0x10,0x15};
unsigned char   Account_PW_log_index[5] = {0x00, 0x20,0x25,0x30,0x35};
unsigned char   Account_LastLogin_log_index[5] = {0x00, 0x40,0x50,0x60,0x70};
unsigned char   Account_Attempts_log_index[5] = {0x00, 0x80,0x85,0x90,0x95};
unsigned char   Account_Status_log_index[5] = {0x00, 0xA0,0xA5,0xB0,0xB5};

unsigned char   Account_ID_log_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x05, 0x00, 0x00, 0x00, 0x00};
unsigned char   Account_PW_log_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x05, 0x00, 0x00, 0x00, 0x00};
unsigned char   Account_LastLogin_log_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x05, 0x00, 0x00, 0x00, 0x00};
unsigned char   Account_Attempts_log_display[7] = {0x5A, 0xA5, 0x04, 0x82, 0x05, 0x00, 0x00};
unsigned char   Account_Status_log_display[12] = {0x5A, 0xA5, 0x09, 0x82, 0x05, 0x00, 0x00, 0x00, 0x00};

/*
unsigned char   H2O2RFID_log_display[9] = {0x5A, 0xA5, 0x06, 0x82, 0x04, 0x00, 0x00, 0x00, 0x00};
unsigned char   H2O2Volume_log_display[10] = {0x5A, 0xA5, 0x07, 0x82, 0x04, 0x00, 0x00, 0x00, 0x00};
unsigned char   H2O2Date_log_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x04, 0x00, 0x00, 0x00, 0x00};
*/

extern struct Account_format IDLIST;

int Temp_Serial_Year, Temp_Serial_Month, Temp_Serial_Num;

float Temp_Peri1_15_Value, Temp_Peri1_12_Value, Temp_Peri1_9_Value;
float Temp_Peri2_15_Value, Temp_Peri2_12_Value, Temp_Peri2_9_Value;

extern int USBConnect_Flag;
extern int H2O2Sensor_Flag;

unsigned int expected_uiFinishTime;
extern unsigned int uireservetime;
extern unsigned char ProcessMode;
int select_index=1;

void InitLCD(void){
    HAL_Delay(20);
    // Start Page.
    DisplayFirstPage();
    // RTC Set
    SetRTCFromLCD();

    DisplaySerialNumber();

    //Display Version
    DisplayDebug("");
    DisplayVersion('3','3','3');
    InitDisplayValues();
}

void InitDisplayValues(){
	//Operation Time
    DisplayPreHeatTime();
    DisplaySprayTime();
    DisplaySterileTime();
    DisplayScrubTime();
    DisplayFinishTime();
    DisplayTotalTime();

    //Parameter
    DisplayCubic();
    DisplayInjectionPerMinute();
    DisplayInjectionPerCubic();

    //Status
    DisplayBoardTemperature();
    DisplayModuleTemperature(0);
    DisplayHuminity();
    DisplayDensity();
    DisplayH2O2Volume();

    DisplayModeIcon(0);
    ui1sCounter = 0;
}

void LCD_Process(){
    int iValue,PW;
    switch(uart1_rx_data[4]) {
        case 0 :    // button
            LCD_Function_Process(uart1_rx_data[5], uart1_rx_data[8]);
            break;
        case 2 :    // set value;
            iValue = uart1_rx_data[7];
            iValue <<= 8;
            iValue |= uart1_rx_data[8];
            LCD_SetValues(uart1_rx_data[5], iValue);
            break;
        case 0x03 : //Password
        	PW = uart1_rx_data[7];
        	PW <<= 8;
        	PW |= uart1_rx_data[8];
        	LCD_Password(uart1_rx_data[5],PW);
            break;
        case 0x31 :		//페이지는 변동가능
            iValue = uart1_rx_data[7];
            iValue <<= 8;
            iValue |= uart1_rx_data[8];
            LCD_31(uart1_rx_data[5], iValue);
            break;
        case 0x06 :		//페이지는 변동가능
            iValue = uart1_rx_data[7];
            iValue <<= 8;
            iValue |= uart1_rx_data[8];
            LCD_06(uart1_rx_data[5], iValue);
            break;
        case 0x07 :		//페이지는 변동가능
            iValue = uart1_rx_data[7];
            iValue <<= 8;
            iValue |= uart1_rx_data[8];
            LCD_07(uart1_rx_data[5], iValue);
            break;
        case 0x51 :		//페이지는 변동가능
            iValue = uart1_rx_data[7];
            iValue <<= 8;
            iValue |= uart1_rx_data[8];
            LCD_51(uart1_rx_data[5], iValue);
            break;
        case 0x52 :		//페이지는 변동가능
            iValue = uart1_rx_data[7];
            iValue <<= 8;
            iValue |= uart1_rx_data[8];
            LCD_52(uart1_rx_data[5], iValue);
            break;
        case 0x53 :		//페이지는 변동가능
            iValue = uart1_rx_data[7];
            iValue <<= 8;
            iValue |= uart1_rx_data[8];
            LCD_53(uart1_rx_data[5], iValue);
            break;
        case 0x54 :		//페이지는 변동가능
            iValue = uart1_rx_data[7];
            iValue <<= 8;
            iValue |= uart1_rx_data[8];
            LCD_54(uart1_rx_data[5], iValue);
            break;
        case 0x55 :		//페이지는 변동가능
            iValue = uart1_rx_data[7];
            iValue <<= 8;
            iValue |= uart1_rx_data[8];
            LCD_55(uart1_rx_data[5], iValue);
            break;
        case 0x56 :		//페이지는 변동가능
            iValue = uart1_rx_data[7];
            iValue <<= 8;
            iValue |= uart1_rx_data[8];
            LCD_56(uart1_rx_data[5], iValue);
            break;
    }
}

void LCD_Function_Process(int index, int value){
    switch(index){
        case 0:
            DoActionButton(value);
            break;
        case 2:
        	LoginFuntionButton(value);
            break;
        case 3:
        	SettingButton(value);
            break;
        case 4:
            MaintenanceButton(value);
            break;
        case 5:
			DeveloperButton(value);
			break;
    }
}

//--------------------------------------------------

#define MAINTENANCE_PAGE			0x01
#define MAINTENANCE_TO_ADMIN_PAGE	0x02

#define MAINTENANCE_FAN_TEST		0x11
#define MAINTENANCE_AIRCOMP_TEST	0x12
#define MAINTENANCE_SOL_TEST		0x13
#define MAINTENANCE_PERIPUMP_TEST	0x14
#define MAINTENANCE_SMS_TEST		0x15
#define MAINTENANCE_USB_TEST		0x16
#define MAINTENANCE_SPRAY_TEST		0x17
#define MAINTENANCE_CLEANING_TEST	0x18

#define MAINTENANCE_TEST_YES		0x19
#define MAINTENANCE_TEST_NO			0x1A
#define MAINTENANCE_TEST_CONFIRM	0x1B
#define MAINTENANCE_TEST_STOP		0x1C

#define NO_TEST						0
#define FAN_TEST					1
#define AIRCOMP_TEST				2
#define SOL_TEST					3
#define PERIPUMP_TEST				4

#define SPRAY_TEST					6
#define NOZZLE_CLEAN				8
#define TEST_COMPLETE				9

unsigned int TestTime=0;
unsigned int input_test_time=0;

int Test_flag=0;
volatile int Test_Start_flag=0;

void MaintenanceButton(int key){
	switch(key) {

		case MAINTENANCE_PAGE:
			DisplayPage(LCD_MAINTENANCE_PAGE);
			break;

		case MAINTENANCE_TO_ADMIN_PAGE:
			DisplayUserNumber();
			DisplaySMSonoffIcon((SMSonoff_Flag&0x01)==0x01);
			DisplayUSBSecurityonoffIcon(USBSECURITYonoff_Flag);
			if(DeviceInfo.loginonoff_flag==1){
				DisplayPage(LCD_ADMIN_SETTING);
			}
			else{
				DisplayPage(LCD_LOGINOFF_SETTING);
			}
			break;

		case MAINTENANCE_FAN_TEST:
			DisplayMsg("Do you want to test Fan?");
			DisplayPage(LCD_MAINTENANCE_YESNO);
			Test_flag=FAN_TEST;
			TestTime=TIME_FAN_TEST+TIME_GAP;
			break;

		case MAINTENANCE_AIRCOMP_TEST:
			Test_flag=AIRCOMP_TEST;
			DisplayMsg("Do you want to test Air_Comp?");
			DisplayPage(LCD_MAINTENANCE_YESNO);
			TestTime=TIME_AIR_TEST+TIME_GAP;
			break;

		case MAINTENANCE_SOL_TEST:
			Test_flag=SOL_TEST;
			DisplayMsg("Do you want to test Solenoid?");
			DisplayPage(LCD_MAINTENANCE_YESNO);
			TestTime=TIME_SOL_TEST+TIME_GAP;
			break;

		case MAINTENANCE_PERIPUMP_TEST:
			Test_flag=PERIPUMP_TEST;
			DisplayMsg("Do you want to test Peri_Pump?");
			DisplayPage(LCD_MAINTENANCE_YESNO);
			TestTime=TIME_PERI_TEST+TIME_GAP;
			break;

		case MAINTENANCE_SMS_TEST:
			SendTestMessage();
			DisplayMsg("Send test message");
			DisplayPage(LCD_MAINTENANCE_CONFIRM);
			break;

		case MAINTENANCE_USB_TEST:
			TestSaveUsbLog();
			DisplayMsg("Save test data");
			DisplayPage(LCD_MAINTENANCE_CONFIRM);
			break;

		case MAINTENANCE_SPRAY_TEST:
			Test_flag=SPRAY_TEST;
			DisplayMsg("Do you want to spray test?");
			DisplayPage(LCD_MAINTENANCE_YESNO);
			TestTime=TIME_SPRAY1_TEST+TIME_SPRAY2_TEST+TIME_GAP;
			break;

		case MAINTENANCE_CLEANING_TEST:
			Test_flag=NOZZLE_CLEAN;
			DisplayMsg("Do you want to clean the nozzle?");
			DisplayPage(LCD_MAINTENANCE_YESNO);
			TestTime=TIME_CLEAN1+TIME_CLEAN2+TIME_GAP;
			break;

		case MAINTENANCE_TEST_YES:
			switch(Test_flag) {
				case FAN_TEST:
					DisplayMsg("Fan Testing.");
					DisplayPage(LCD_MAINTENANCE_TEST);
					Test_Start_flag=1;
					ProcessTestEndTimer();
					break;
				case AIRCOMP_TEST:
					DisplayMsg("Air Testing.");
					DisplayPage(LCD_MAINTENANCE_TEST);
					Test_Start_flag=1;
					ProcessTestEndTimer();
					break;
				case SOL_TEST:
					DisplayMsg("Solenoid Testing.");
					DisplayPage(LCD_MAINTENANCE_TEST);
					Test_Start_flag=1;
					ProcessTestEndTimer();
					break;
				case PERIPUMP_TEST:
					DisplayMsg("Peri_Pump Testing.");
					DisplayPage(LCD_MAINTENANCE_TEST);
					Test_Start_flag=1;
					ProcessTestEndTimer();
					break;
				case SPRAY_TEST:
					DisplayMsg("Spraying.(After the test, use the cleaning function)");
					DisplayPage(LCD_MAINTENANCE_TEST);
					Test_Start_flag=1;
					ProcessTestEndTimer();
					TestTime=TIME_SPRAY1_TEST+TIME_SPRAY2_TEST+TIME_GAP;
					break;
				case NOZZLE_CLEAN:
					DisplayMsg("Cleaning.");
					DisplayPage(LCD_MAINTENANCE_TEST);
					Test_Start_flag=1;
					ProcessTestEndTimer();
					TestTime=TIME_CLEAN1+TIME_CLEAN2+TIME_GAP;
					break;
			}
			break;

		case MAINTENANCE_TEST_NO:
			Test_Start_flag=0;
			TestTime=0;
			Test_flag=0;
			DisplayPage(LCD_MAINTENANCE_PAGE);
			break;

		case MAINTENANCE_TEST_CONFIRM:
			TurnOffFanPump();
			Test_Start_flag=0;
			TestTime=0;
			Test_flag=0;
			DisplayPage(LCD_MAINTENANCE_PAGE);
			break;

		case MAINTENANCE_TEST_STOP:
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
			Test_Start_flag=0;
			TestTime=0;

			if(Test_flag==7||Test_flag==8||Test_flag==10||Test_flag==11){
				DisplayMsg("Canceled.\n\r"
						"Please turn off and on the sterilizer.");
				DisplayPage(LCD_MAINTENANCE_CONFIRM);
			}
			else{
				DisplayMsg("Canceled");
				DisplayPage(LCD_MAINTENANCE_CONFIRM);
			}
			Test_flag=0;
			break;

	}
	//display Setting icon 추가
	HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

void DisplayTestComplete(){
	DisplayPage(LCD_MAINTENANCE_CONFIRM);
}


#define DEVELOPER_PAGE				0x01

#define device_version_BUTTON		0x10
#define SERIAL_CHANGE_BUTTON		0x20
#define MODEM_CHANGE_BUTTON			0x30
#define SMS_SERVER_NOTICE_BUTTON	0x40
#define PERI_VALUE_SETTING_BUTTON	0x50

#define LOGIN_ONOFF_BUTTON				0xA0
#define RESERVATION_ONOFF_BUTTON	0xC0
#define RESET_ALL_BUTTON				0xB0



#define DEVICE_SELECT_S400V1			0x11
#define DEVICE_SELECT_S400V2			0x12
#define DEVICE_SELECT_S300V1			0x13
#define DEVICE_SELECT_S300V2			0x14
#define DEVICE_SELECT_SVC				0x15
#define DEVICE_SELECT_NON_RFID		0x16
#define DEVICE_SELECT_S100				0x18

#define CHAGNE_SERIAL_CONFIRM		0x21
#define CHAGNE_SERIAL_CANCEL		0x22


#define CHAGNE_MODEM_CONFIRM		0x31
#define CHAGNE_MODEM_CANCEL			0x32


#define CHANGE_PERI_CONFIRM			0x60

#define CHAGNE_PERI1_15_VALUE_UP	0x61
#define CHAGNE_PERI1_15_VALUE_DOWN	0x62
#define CHAGNE_PERI2_15_VALUE_UP	0x63
#define CHAGNE_PERI2_15_VALUE_DOWN	0x64
#define CHAGNE_PERI_15_VALUE_TEST	0x65
#define CHAGNE_PERI_15_VALUE_SAVE	0x66

#define CHAGNE_PERI1_12_VALUE_UP	0x67
#define CHAGNE_PERI1_12_VALUE_DOWN	0x68
#define CHAGNE_PERI2_12_VALUE_UP	0x69
#define CHAGNE_PERI2_12_VALUE_DOWN	0x70
#define CHAGNE_PERI_12_VALUE_TEST	0x71
#define CHAGNE_PERI_12_VALUE_SAVE	0x72

#define CHAGNE_PERI1_9_VALUE_UP		0x73
#define CHAGNE_PERI1_9_VALUE_DOWN	0x74
#define CHAGNE_PERI2_9_VALUE_UP		0x75
#define CHAGNE_PERI2_9_VALUE_DOWN	0x76
#define CHAGNE_PERI_9_VALUE_TEST	0x77
#define CHAGNE_PERI_9_VALUE_SAVE	0x78

#define CHAGNE_PERI_VALUE_TEST_YES	0x80
#define CHAGNE_PERI_VALUE_TEST_NO	0x81
#define CHAGNE_PERI_VALUE_TEST_STOP	0x83




#define SMS_SERVER_NOTICE_YES		0x51
#define SMS_SERVER_NOTICE_NO		0x52

#define LOGIN_ON					0x53
#define LOGIN_OFF					0x54

#define RESET_ALL_YES				0x55
#define RESET_ALL_NO				0x56

#define RESERVATION_ON					0x57
#define RESERVATION_OFF					0x58





void DeveloperButton(int key){
	switch(key) {
		case DEVELOPER_PAGE:
			DisplayDeveloperPage();
			break;

		//Button
		case device_version_BUTTON:
			DisplayPage(CHANGE_device_version_PAGE);
			break;

		case SERIAL_CHANGE_BUTTON:
			DisplaySerialNumberblank();
			DisplayPage(CHANGE_SERIAL_PAGE);
			break;

		case MODEM_CHANGE_BUTTON:
			DisplayModemNumber();
			DisplayUserNumberblank();
			DisplayPage(CHANGE_MODEM_PAGE);
			break;

		case SMS_SERVER_NOTICE_BUTTON:
			DisplayMsg("Do you want to use SMS Alarm(Server)?");
			DisplayPage(SMS_SERVER_NOTICE_PAGE);
			break;

		case PERI_VALUE_SETTING_BUTTON:
			Display51page();
			break;

		case LOGIN_ONOFF_BUTTON:
			DisplayMsg("Do you want to use Login function?");
			DisplayPage(LOGIN_ONOFF_PAGE);
			break;

		case RESERVATION_ONOFF_BUTTON:
			DisplayMsg("Do you want to use Reservation?");
			DisplayPage(RESERVATION_ONOFF_PAGE);
			break;


		case RESET_ALL_BUTTON:
			DisplayMsg("Do you want to Reset all?");
			DisplayPage(RESET_ALL_PAGE);
			break;

		//device_version_PAGE
		case DEVICE_SELECT_S400V1:
			DeviceInfo.device_version=1;
			fCubic=100;
			fInjectionPerMinute=12;
			fInjectionPerMinute2=3;
			fInjectionPerCubic=6;
			serialNum[5]=0x34;
			ReCalcTime();
			Write_Flash();
			DisplayDeveloperPage();
			break;

		case DEVICE_SELECT_S400V2:
			DeviceInfo.device_version=2;
			fCubic=100;
			fInjectionPerMinute=12;
			fInjectionPerMinute2=3;
			fInjectionPerCubic=6;
			serialNum[5]=0x34;
			ReCalcTime();
			Write_Flash();
			DisplayDeveloperPage();
			break;

		case DEVICE_SELECT_S300V1:
			DeviceInfo.device_version=3;
			fCubic=100;
			fInjectionPerMinute=12;
			fInjectionPerMinute2=3;
			fInjectionPerCubic=6;
			serialNum[5]=0x33;
			ReCalcTime();
			Write_Flash();
			DisplayDeveloperPage();
			break;

		case DEVICE_SELECT_S300V2:
			DeviceInfo.device_version=4;
			fCubic=100;
			fInjectionPerMinute=12;
			fInjectionPerMinute2=3;
			fInjectionPerCubic=6;
			serialNum[5]=0x33;
			ReCalcTime();
			Write_Flash();
			DisplayDeveloperPage();
			break;

		case DEVICE_SELECT_SVC:
			DeviceInfo.device_version=5;
			fCubic=100;
			fInjectionPerMinute=12;
			fInjectionPerMinute2=3;
			fInjectionPerCubic=6;
			serialNum[5]=0x33;
			ReCalcTime();
			Write_Flash();
			DisplayDeveloperPage();
			break;

		case DEVICE_SELECT_NON_RFID:
			DeviceInfo.device_version=6;
			fCubic=100;
			fInjectionPerMinute=12;
			fInjectionPerMinute2=3;
			fInjectionPerCubic=6;
			serialNum[5]=0x34;
			ReCalcTime();
			Write_Flash();
			DisplayDeveloperPage();
			break;

		case DEVICE_SELECT_S100:
			DeviceInfo.device_version=8;
			fCubic=10;
			fInjectionPerMinute=12;
			fInjectionPerMinute2=3;
			fInjectionPerCubic=5;
			serialNum[5]=0x31;
			ReCalcTime();
			Write_Flash();
			DisplayDeveloperPage();
			break;


		//SERIAL_CHANGE_PAGE
		case CHAGNE_SERIAL_CONFIRM:
			if(Temp_Serial_Num==0){
				Temp_Serial_Year=0;
				Temp_Serial_Month=0;
				Temp_Serial_Num=0;
			}
			else{
				DeviceInfo.year=Temp_Serial_Year+'0';
				if(Temp_Serial_Month<10){
					DeviceInfo.month=Temp_Serial_Month+'0';
				}
				else if(Temp_Serial_Month==10){
					DeviceInfo.month='A';
				}
				else if(Temp_Serial_Month==11){
					DeviceInfo.month='B';
				}
				else if(Temp_Serial_Month==12){
					DeviceInfo.month='C';
				}
				DeviceInfo.Serial1=Temp_Serial_Num/10%10+'0';
				DeviceInfo.Serial2=Temp_Serial_Num%10+'0';


				//DisplaySerialNumber();

				Write_Flash();
				Temp_Serial_Year=0;
				Temp_Serial_Month=0;
				Temp_Serial_Num=0;
			}
			DisplayDeveloperPage();
			break;

		case CHAGNE_SERIAL_CANCEL:
			//전화번호 원래 값으로...
			Temp_Serial_Year=0;
			Temp_Serial_Month=0;
			Temp_Serial_Num=0;
			DisplayDeveloperPage();
			break;




		//MODEM_CHANGE_PAGE
		case CHAGNE_MODEM_CONFIRM:
			if(changeusernum2==0||changeusernum3==0){
				changeusernum2=0;
				changeusernum3=0;
			}
			else{
				szStartCommandCBT[24]=(changeusernum2/1000)+'0';
				szStartCommandCBT[25]=(changeusernum2%1000)/100+'0';
				szStartCommandCBT[26]=(changeusernum2%100)/10+'0';
				szStartCommandCBT[27]=changeusernum2%10+'0';

				szStartCommandCBT[28]=changeusernum3/1000+'0';
				szStartCommandCBT[29]=(changeusernum3%1000)/100+'0';
				szStartCommandCBT[30]=(changeusernum3%100)/10+'0';
				szStartCommandCBT[31]=changeusernum3%10+'0';

				for(int i=0;i<4;i++){
					szStartCommand[24+i]=szStartCommandCBT[24+i];
					szStartCommand[28+i]=szStartCommandCBT[28+i];
					DeviceInfo.modem_number1[i]=szStartCommandCBT[24+i];
					DeviceInfo.modem_number2[i]=szStartCommandCBT[28+i];
				}
				Write_Flash();
				changeusernum2=0;
				changeusernum3=0;
			}
			DisplayDeveloperPage();
			break;
		case CHAGNE_MODEM_CANCEL:
			//전화번호 원래 값으로...
			changeusernum2=0;
			changeusernum3=0;
			DisplayDeveloperPage();
			break;

			//SMS 문자 on/off
		case SMS_SERVER_NOTICE_YES:
			SMSonoff_Flag|=0x10;
			InitLTEModem();
			Write_Flash();
			DisplayDeveloperPage();
			break;

		case SMS_SERVER_NOTICE_NO:
			SMSonoff_Flag&=0x01;
			InitLTEModem();
			Write_Flash();
			DisplayDeveloperPage();
			break;

		case LOGIN_ON:
			if(DeviceInfo.loginonoff_flag==1){
				DisplayDeveloperPage();
			}
			else{
				DeviceInfo.loginonoff_flag=1;
				Write_Flash();
				DisplayDeveloperPage();
			}
			break;

		case LOGIN_OFF:
			if(DeviceInfo.loginonoff_flag==0){
				DisplayDeveloperPage();
			}
			else{
				DeviceInfo.loginonoff_flag=0;
				Write_Flash();
				DisplayDeveloperPage();
			}
			break;

		case RESERVATION_ON:
			if(DeviceInfo.reservationonoff_flag==1){
				DisplayDeveloperPage();
			}
			else{
				DeviceInfo.reservationonoff_flag=1;
				Write_Flash();
				DisplayDeveloperPage();
			}
			break;

		case RESERVATION_OFF:
			if(DeviceInfo.reservationonoff_flag==0){
				DisplayDeveloperPage();
			}
			else{
				DeviceInfo.reservationonoff_flag=0;
				Write_Flash();
				DisplayDeveloperPage();
			}
			break;


		case RESET_ALL_YES:
			DeviceSettingResetAll();
			DisplayDeveloperPage();
			break;

		case RESET_ALL_NO:
			DisplayDeveloperPage();
			break;

	}
	//display Setting icon 추가
	HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}


void DisplayDeveloperPage(){
	DisplayServerSMSonoffIcon((SMSonoff_Flag&0x10)==0x10);
	DisplayHardwareVersion();
	DisplayModemNumber();
	DisplaySerialNumber();
	//DisplayPeri1_15_Value();
	//DisplayPeri2_15_Value();
	DisplayLOGINOnoffIcon();
	DisplayRESERVEATOINOnoffIcon();
	DisplayPage(LCD_DEVELOPER_PAGE);
}


void DisplaySerialNumberblank(){
	unsigned char   Serial_blank_display[13] = {0x5A, 0xA5, 0x0A, 0x82, 0x02, 0x84, 0x00, 0x00, 0x00};
    HAL_UART_Transmit(&huart1, Serial_blank_display, 13, 10);
}

void DisplayHardwareVersion(){
	unsigned char Hardware_Version_display[13] = {0x5A, 0xA5, 0x0A, 0x82, 0x20, 0x50, 'S', '4', '0', '0', '_', 'V', '1'};
	switch(DeviceInfo.device_version){
		case 1 :
			Hardware_Version_display[6]='S';
			Hardware_Version_display[7]='4';
			Hardware_Version_display[8]='0';
			Hardware_Version_display[9]='0';
			Hardware_Version_display[10]='_';
			Hardware_Version_display[11]='V';
			Hardware_Version_display[12]='1';
			break;
		case 2 :
			Hardware_Version_display[6]='S';
			Hardware_Version_display[7]='4';
			Hardware_Version_display[8]='0';
			Hardware_Version_display[9]='0';
			Hardware_Version_display[10]='_';
			Hardware_Version_display[11]='V';
			Hardware_Version_display[12]='2';
			break;
		case 3 :
			Hardware_Version_display[6]='S';
			Hardware_Version_display[7]='3';
			Hardware_Version_display[8]='0';
			Hardware_Version_display[9]='0';
			Hardware_Version_display[10]='_';
			Hardware_Version_display[11]='V';
			Hardware_Version_display[12]='1';
			break;
		case 4 :
			Hardware_Version_display[6]='S';
			Hardware_Version_display[7]='3';
			Hardware_Version_display[8]='0';
			Hardware_Version_display[9]='0';
			Hardware_Version_display[10]='_';
			Hardware_Version_display[11]='V';
			Hardware_Version_display[12]='2';
			break;
		case 5 :
			Hardware_Version_display[6]='C';
			Hardware_Version_display[7]='B';
			Hardware_Version_display[8]='T';
			Hardware_Version_display[9]='_';
			Hardware_Version_display[10]='S';
			Hardware_Version_display[11]='V';
			Hardware_Version_display[12]='C';
			break;
		case 6 :
			Hardware_Version_display[6]='S';
			Hardware_Version_display[7]='4';
			Hardware_Version_display[8]='0';
			Hardware_Version_display[9]='0';
			Hardware_Version_display[10]='_';
			Hardware_Version_display[11]='V';
			Hardware_Version_display[12]='2';
			break;
		case 8 :
			Hardware_Version_display[6]='S';
			Hardware_Version_display[7]='1';
			Hardware_Version_display[8]='0';
			Hardware_Version_display[9]='0';
			Hardware_Version_display[10]=' ';
			Hardware_Version_display[11]=' ';
			Hardware_Version_display[12]=' ';
			break;
	}
	HAL_UART_Transmit(&huart1, Hardware_Version_display, 13, 10);
}


//--------------------------------------------------

#define TOUCH_START_STOP_BUTTON 						0x02
#define TOUCH_H2O2CHANGE_BUTTON 						0x03
#define GOTO_FIRST_PAGE 											0x04
#define GOTO_OPERATION_PAGE 								0x06
#define GOTO_SETTING_PAGE 										0x07
#define GOTO_OPERATION_BEFORE_PAGE					0x08
#define GOTO_SETTING_BEFORE_PAGE							0x09

#define SOLUTION_CHECK_YES									0x0A
#define SOLUTION_CHECK_NO										0x0B

#define RESERVATOIN_YES_BUTTON							0x0D
#define RESERVATOIN_SETTING_START_BUTTON			0x0E
#define RESERVATOIN_CANCEL_BUTTON						0x0F



//0,0,KEY
void DoActionButton(int key){
    switch(key) {
    	case 0:

            break;
        case 1: // Power on case.
            InitDisplayValues();
            DisplayOperationPage();
            break;

        case TOUCH_START_STOP_BUTTON:
            StartButtonProcess();
            break;

        case TOUCH_H2O2CHANGE_BUTTON:
        	H2O2Read();
        	break;

        case GOTO_FIRST_PAGE:
        	DisplayFirstPage();
        	break;

        case 5:
        	uiScrubTime = 100;
        	uireservetime = 100;
        	ReCalcTime();
        	DisplayOperationPage();
            HAL_Delay(100);
        	StartProcess();
        	break;

        case GOTO_OPERATION_PAGE:	//LCD_OPERATTION
            if(DeviceInfo.loginonoff_flag==1){
            	DisplayLoginStatus();
            	InitDisplayValues();
            	DisplayPage(LCD_OPERATTION_NORMAL);
            }
            else{
            	LOGIN_ID=0;
				DisplayLoginStatus();
				InitDisplayValues();
				DisplayPage(LCD_OPERATTION_NORMAL);
            }
        	break;

        case GOTO_SETTING_PAGE://LCD_Setting
        	DisplaySettingPage();
        	break;

        case GOTO_OPERATION_BEFORE_PAGE:
            if(DeviceInfo.loginonoff_flag==1){
            	DisplayPage(LCD_SELECT_MENU);
            }
            else{
            	DisplayPage(LCD_LOGIN_OFF_FIRST);
            }
        	break;
        case GOTO_SETTING_BEFORE_PAGE:
            if(DeviceInfo.loginonoff_flag==1){
            	DisplayPage(LCD_SELECT_MENU);
            }
            else{
            	DisplayPage(LCD_LOGIN_OFF_FIRST);
            }
        	break;

        case RESERVATOIN_YES_BUTTON:	//Reservation Yes
        	//ReserveSettings
        	uireservetime=100;
        	DisplayReserveSettings();
        	break;
        case RESERVATOIN_SETTING_START_BUTTON:	//Reservation Setting Start
        	//LTE Message send
        	//Reserve Process start
        	Running_Flag=1;
        	ReserveProcess();
			ProcessMode=6;
        	PeristalticPumpOnOff_Flag=1;
        	SaveReserveTime();
        	SaveExpectFinishTime();
        	DisplayReserveTimePage();
        	break;

        case RESERVATOIN_CANCEL_BUTTON:	//stop
        	CancelProcess();
        	break;



        case SOLUTION_CHECK_YES:
        	if(DeviceInfo.reservationonoff_flag==0){
        		StartProcess();//수정중
        	}
        	else{
        		DisplayReservePopUp("Would you use the reservation function?");
        	}

        	break;

        case SOLUTION_CHECK_NO:
        	DisplayOperationPage();
        	break;
        case 12:
        	break;
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}


#define NOT_INPUT				0x09
#define LOGIN_BUTTON			0x0A
#define LOGIN_RETURN_BUTTON		0X0B

//#define LOGIN_RETURN_BUTTON		0X0D

//0,0,2,key
void LoginFuntionButton(int key){
    switch(key) {
    	case ID0:
    		SelectID=ID0;
    		DisplayPage(LCD_LOGIN_SELECT_ID);
    		break;
        case USER1:
        	SelectID=USER1;
        	if(IDLIST.Status[SelectID]==1){
				Display06page();
        	}
        	else{
				DisplayMsg("Account Locked, Please contact your admin");
				SelectID=0;
				LOGIN_ID=0;
				DisplayPage(LCD_LOGIN_WRONG_PW);
        	}
            break;
        case USER2:
        	SelectID=USER2;
        	if(IDLIST.Status[SelectID]==1){
        		Display06page();
        	}
        	else{
        		DisplayMsg("Account Locked, Please contact your admin");
				SelectID=0;
				LOGIN_ID=0;
				DisplayPage(LCD_LOGIN_WRONG_PW);
        	}
            break;
        case USER3:
        	SelectID=USER3;
        	if(IDLIST.Status[SelectID]==1){
        		Display06page();
        	}
        	else{
        		DisplayMsg("Account Locked, Please contact your admin");
				SelectID=0;
				LOGIN_ID=0;
				DisplayPage(LCD_LOGIN_WRONG_PW);
        	}
        	break;
        case SUPERUSER:
        	SelectID=SUPERUSER;
        	if(IDLIST.Status[SelectID]==1){
        		Display06page();
        	}
        	else{
        		DisplayMsg("Account Locked, Please contact your admin");
				SelectID=0;
				LOGIN_ID=0;
				DisplayPage(LCD_LOGIN_WRONG_PW);
        	}
            break;
        case ADMIN:
        	SelectID=ADMIN;
        	Display06page();
			break;
		case 6:
			break;

		case LOGIN_BUTTON:
			if(IDLIST.Status[SelectID]==1||SelectID==5){
				PasswordCheck();
				LoginProcess();
			}
			else if(IDLIST.Status[SelectID]==0){
				DisplayMsg("Account Locked, Please contact your admin");
				LOGIN_ID=0;
				DisplayPage(LCD_LOGIN_WRONG_PW);
			}
			break;
		case LOGIN_RETURN_BUTTON:	//B
		    switch(SelectID){
		    	case ID0:
					DisplayPage(LCD_LOGIN_SELECT_ID);
					break;
				case USER1:
					DisplayPage(LCD_SELECTED_ID);
					break;
				case USER2:
					DisplayPage(LCD_SELECTED_ID);
					break;
				case USER3:
					DisplayPage(LCD_SELECTED_ID);
					break;
				case SUPERUSER:
					DisplayPage(LCD_SELECTED_ID);
					break;
				case ADMIN:
					DisplayPage(LCD_SELECTED_ID);
					break;
		    }
			break;
			/*
		case 12:	//C
			DisplayPage(LCD_SELECT_MENU);
			break;

		case 14:	//D
			DisplayPage(LCD_SELECT_MENU);
			break;
			*/


    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

// Setting Page Button
#define SMS_ONOFF_BUTTON			0x01
#define SMS_NUMBER_CHANGE_BUTTON	0x02
#define USB_SECURITY_ONOFF_BUTTON	0x03
#define SETTING_RESET_BUTTON		0x04
#define PASSWORD_CHANGE_BUTTON		0x05
#define ACCOUNT_COMFIRM_BUTTON		0x06
#define HISTORY_COMFIRM_BUTTON		0x07
#define H2O2_COMFIRM_BUTTON			0x08

// Setting PopoUp Page Button
#define SMS_ON						0x09
#define SMS_OFF						0x0a
#define SMS_NUMBER_CHANGE_OK		0x0b
#define SMS_NUMBER_CHANGE_CANCEL	0x0c
#define USB_SECURITY_ON				0x0d
#define USB_SECURITY_OFF			0x0e
#define SETTING_RESET_OK			0x0f
#define SETTING_RESET_CANCEL		0x10

// Page 24-27 Change Password Button
#define NON_INPUT_CURRENT_PASSWORD	0x11
#define GOTO_SETTINGPAGE_BUTTON		0x12
#define WRONG_CURRENT_PASSWORD		0x13
#define NON_INPUT_NEW_PASSWORD		0x14
#define CHANGE_PASSWORD_CONFIRM		0x15

#define ACCOUNT_ACTIVE_USER1		0x20
#define ACCOUNT_ACTIVE_USER2		0x21
#define ACCOUNT_ACTIVE_USER3		0x22
#define ACCOUNT_ACTIVE_SUSER		0x23

#define ACCOUNT_ACTIVE_BUTTON		0x25
#define ACCOUNT_LOCK_BUTTON			0x26


void SettingButton(int key){	//3
	switch(key) {
	    	case 0:
	    		break;
	        case SMS_ONOFF_BUTTON:
				DisplayMsg("Do you want to use SMS Alarm?");
				DisplayPage(SMS_SETTING_POPUP);
	            break;
	        case SMS_NUMBER_CHANGE_BUTTON:
	        	DisplayUserNumberblank();
	        	DisplayPage(MOBILE_SETTING_POPUP);
	            break;
	        case USB_SECURITY_ONOFF_BUTTON:
	        	DisplayMsg("Do you want to use USB Security?");
	        	DisplayPage(USB_SETTING_POPUP);
	            break;
	        case SETTING_RESET_BUTTON:
				DisplayMsg("Do you want to Reset?");
				DisplayPage(RESET_SETTING_POPUP);
	            break;
	        case PASSWORD_CHANGE_BUTTON:
	        	DisplayPage(CHANGE_PASSWORD1_POPUP);
				break;
			case ACCOUNT_COMFIRM_BUTTON:
				DisplayAccountSettingPage();
				//DisplayPage(ACCOUNT_SETTING);
				break;
	        case HISTORY_COMFIRM_BUTTON:
	        	Read_LogData_Flash(1);
				DisplaySelectIcon(0x30,0x10,1);
				DisplaySelectIcon(0x30,0x20,0);
				DisplaySelectIcon(0x30,0x30,0);
				DisplaySelectIcon(0x30,0x40,0);
	        	DisplayLogListPage();
				break;

			case H2O2_COMFIRM_BUTTON:
				DisplayH2O2ListPage();
				DisplayPage(H2O2_List);
				break;


//----------팝업 창 선택 버튼--------------------------

				//SMS 문자 on/off
			case SMS_ON:
				SMSonoff_Flag|=0x01;
				InitLTEModem();
				Write_Flash();
				DisplaySettingPage();
				break;
			case SMS_OFF:
				SMSonoff_Flag&=0x10;
				InitLTEModem();
				Write_Flash();
				DisplaySettingPage();
				break;

				//SMS 번호 변경
			case SMS_NUMBER_CHANGE_OK:
	        	if(changeusernum2==0||changeusernum3==0){
	        		DisplaySettingPage();
					}
					else{
					szStartCommand[12]=(changeusernum2/1000)+'0';
					szStartCommand[13]=(changeusernum2%1000)/100+'0';
					szStartCommand[14]=(changeusernum2%100)/10+'0';
					szStartCommand[15]=changeusernum2%10+'0';

					szStartCommand[16]=changeusernum3/1000+'0';
					szStartCommand[17]=(changeusernum3%1000)/100+'0';
					szStartCommand[18]=(changeusernum3%100)/10+'0';
					szStartCommand[19]=changeusernum3%10+'0';
					for(int i=0;i<4;i++){
						user_number1[i]=szStartCommand[12+i];
					}

					for(int i=0;i<4;i++){
						user_number2[i]=szStartCommand[16+i];
					}
					Write_Flash();
					changeusernum2=0;
					changeusernum3=0;
	        	}
				DisplaySettingPage();
				break;

			case SMS_NUMBER_CHANGE_CANCEL:
				//전화번호 원래 값으로...
				changeusernum2=0;
				changeusernum3=0;
				DisplaySettingPage();
				break;

				//USB 암호화 기능
			case USB_SECURITY_ON:
				USBSECURITYonoff_Flag=1;
				//USB 관련 세팅 초기화 함수 추가하기
				Write_Flash();
				DisplaySettingPage();
				break;
			case USB_SECURITY_OFF:
				USBSECURITYonoff_Flag=0;
				//USB 관련 세팅 초기화 함수 추가하기
				Write_Flash();
				DisplaySettingPage();
				break;

				//Reset 기능
			case SETTING_RESET_OK:
				DeviceSettingReset();
				DisplaySettingPage();
				break;

			case SETTING_RESET_CANCEL:
				DisplaySettingPage();
				break;


			case NON_INPUT_CURRENT_PASSWORD:
				DisplayMsg("Input Current Password.");
				DisplayPage(CHANGE_PASSWORD2_POPUP);
				break;
			case GOTO_SETTINGPAGE_BUTTON:
				DisplaySettingPage();
				//DisplaySettingPage();
				break;
			case WRONG_CURRENT_PASSWORD:
				//
				DisplayPage(CHANGE_PASSWORD1_POPUP);
				break;
			case NON_INPUT_NEW_PASSWORD:
				DisplayMsg("Input New Password.");
				DisplayPage(CHANGE_PASSWORD1_POPUP);
				//DisplaySettingPage();
				break;
			case CHANGE_PASSWORD_CONFIRM:
				//Password Save and display
				for(int i=0;i<4;i++){
					Savepassword[i]=InputPassword[i];
					IDLIST.PW[LOGIN_ID][i]=Savepassword[i];
				}
				//sprintf(IDLIST.PW[LOGIN_ID],"%c%c%c%c",Savepassword[0],Savepassword[1],Savepassword[2],Savepassword[3]);
				Write_Flash();
				DisplaySettingPage();
				break;

			case ACCOUNT_ACTIVE_USER1:
				DisplayMsg("Do you want to be active user 1?");
				DisplayPage(ACCOUNT_POPUP2);
				SelectID=1;
				break;
			case ACCOUNT_ACTIVE_USER2:
				DisplayMsg("Do you want to be active user 2?");
				DisplayPage(ACCOUNT_POPUP2);
				SelectID=2;
				break;
			case ACCOUNT_ACTIVE_USER3:
				DisplayMsg("Do you want to be active user 3?");
				DisplayPage(ACCOUNT_POPUP2);
				SelectID=3;
				break;
			case ACCOUNT_ACTIVE_SUSER:
				DisplayMsg("Do you want to be active Superuser?");
				DisplayPage(ACCOUNT_POPUP2);
				SelectID=4;
				break;
			case ACCOUNT_ACTIVE_BUTTON:
				if(IDLIST.Status[SelectID]==1){
					DisplayAccountSettingPage();
				}
				else{
					IDLIST.Status[SelectID]=1;
					IDLIST.PW[SelectID][0]=1;
					IDLIST.PW[SelectID][1]=2;
					IDLIST.PW[SelectID][2]=3;
					IDLIST.PW[SelectID][3]=4;
					IDLIST.Attempts[SelectID]=0;
					Write_Flash();
					DisplayAccountSettingPage();
				}
				break;
			case ACCOUNT_LOCK_BUTTON:
				if(IDLIST.Status[SelectID]==1){
					IDLIST.Status[SelectID]=0;
					Write_Flash();
					DisplayAccountSettingPage();
				}
				else{
					DisplayAccountSettingPage();
				}
				break;



	    }
		//display Setting icon 추가
	    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}


void StartButtonProcess(void){
    if(Running_Flag == 0) {
    	if(PeristalticPumpCnt==0){
			if((fBoardTemperature>0)&&(fModuleTemperature>0)){//온도 센서 동작 여부 확인
				if(DeviceInfo.reservationonoff_flag==0){
					if(DeviceInfo.device_version==5){
						StartProcess();
					}
					else if(DeviceInfo.device_version==6){
						StartProcess();
					}
					else if(DeviceInfo.device_version==8){
						//잔량 체크 문구 출력
						DisplaySolutionCheckPopUp("Did you check the amount of H2O2");
					}
					else{
						if(H2O2Check()){
							StartProcess();
						}
					}
				}
				else{
					if(DeviceInfo.device_version==5){
						DisplayReservePopUp("Would you use the reservation function?");
					}
					else if(DeviceInfo.device_version==6){
						DisplayReservePopUp("Would you use the reservation function?");
					}
					else if(DeviceInfo.device_version==8){
						//잔량 체크 문구 출력
						DisplaySolutionCheckPopUp("Did you check the amount of H2O2 solution");
					}
					else{
						if(H2O2Check()){
							DisplayReservePopUp("Would you use the reservation function?");
						}
					}
				}
			}
			else{
				DisplayPopUpMessage("Please check temperature sensor");
			}
    	}
    	else{
    		DisplayPopUpMessage("Please restart the sterilizer");
    	}

    }
    else {
        CancelProcess();
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}


void LCD_SetValues(int index, int value){
    switch(index) {
		case 0x10 : // set Reserve time
			uireservetime =((float)value/10*60)*60*100;
			if(uireservetime==0){
				uireservetime=100;
			}
			ReCalcTime();
			HAL_Delay(100);
			DisplayReserveSettings();
			break;
        case 0x14 : // set cubic
        	if(DeviceInfo.device_version==8){
        		if(value>1000){
        			DisplayPopUpMessage("Input smaller than 100");
					DisplayModeIcon(0);
        			fCubic=10;
        		}
        		else{
        			fCubic=(float)value/10;
        		}
        	}
        	else{
                fCubic = (float)value / 10;
        	}
            DisplayCubic();
            ReCalcTime();
            break;
        case 0x24 : // set InjectionPerMinute
        	if(DeviceInfo.device_version==8){
            	if(value==3||value==4||value||5){
            		fInjectionPerMinute2 = value;
    			}
            	else{
    				DisplayPopUpMessage("Input 3 or 4 or 5");
    				InitDisplayValues();
            	}
        	}
        	else{
            	if(value==9||value==15||value==12){
            		fInjectionPerMinute = value;
    			}
            	else{
    				DisplayPopUpMessage("Input 9 or 12 or 15");
    				InitDisplayValues();
            	}
        	}
			DisplayInjectionPerMinute();
			PeristalticSpeed();
			ReCalcTime();
            break;
        case 0x34 :	// set InjectionPerCubic
			if(value>=1&&value<=9){// ť�� �Է��Ҽ��ְ� ����.
				fInjectionPerCubic = value;
			}
			else{
				//fInjectionPerCubic=6;
				DisplayPopUpMessage("Input more than 1 and less than 9");
				DisplayModeIcon(0);
			}
        	DisplayInjectionPerCubic();
            ReCalcTime();
            break;
        case 0x44 :	// set scrubtime
        	if((value/10) > 12){
        		uiScrubTime=100;
        		DisplayPopUpMessage("Input smaller than 12");
        		DisplayModeIcon(0);
        	}else{
        		if(value==0){
            		uiScrubTime=100;
            		ReCalcTime();
            		HAL_Delay(100);
            		DisplayOperationPage();
        		}
        		else{
    				uiScrubTime =((float)value*60)*60*10;
    				ReCalcTime();
    				HAL_Delay(100);
    				DisplayOperationPage();
        		}
        	}
        	break;
        case 0x54 :
        	break;
        case 0x74 :
        	changeusernum2=value;
        	break;
        case 0x78 :
        	changeusernum3=value;
        	break;
        case 0x84 :
        	Temp_Serial_Year=value;
        	break;
        case 0x85 :
        	Temp_Serial_Month=value;
        	break;
        case 0x86 :
        	Temp_Serial_Num=value;
        	break;
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

void LCD_Password(int index, int PW){
    switch(index) {
        case 0x01 : // InputPassword
        	InputPassword[0]=PW/1000;
        	InputPassword[1]=(PW%1000)/100;
        	InputPassword[2]=(PW%100)/10;
        	InputPassword[3]=PW%10;
            switch(SelectID){
        		case 1:
        			DisplayPage(LCD_INPUTED_PW);
        			break;
        		case 2:
        			DisplayPage(LCD_INPUTED_PW);
        			break;
        		case 3:
        			DisplayPage(LCD_INPUTED_PW);
        			break;
        		case 4:
        			DisplayPage(LCD_INPUTED_PW);
        			break;
        		case 5:
        			DisplayPage(LCD_INPUTED_PW);
        			break;
            }
            break;
            case 0x02 : // InputCurrentPassword
            	InputPassword[0]=PW/1000;
            	InputPassword[1]=(PW%1000)/100;
            	InputPassword[2]=(PW%100)/10;
            	InputPassword[3]=PW%10;
            	PasswordCheck();
            	if(PasswordOK_Flag==1){
            		DisplayPage(CHANGE_PASSWORD3_POPUP);
            	}
            	else{
            		DisplayMsg("Wrong Password");
            		DisplayPage(CHANGE_PASSWORD2_POPUP);
            	}

                break;
            case 0x03 : // InputNewPassword
            	InputPassword[0]=PW/1000;
            	InputPassword[1]=(PW%1000)/100;
            	InputPassword[2]=(PW%100)/10;
            	InputPassword[3]=PW%10;
            	DisplayPage(CHANGE_PASSWORD4_POPUP);
                break;

            case 0x04 : //
				InputPassword[0]=PW/1000;
				InputPassword[1]=(PW%1000)/100;
				InputPassword[2]=(PW%100)/10;
				InputPassword[3]=PW%10;
				PasswordCheck();
            	if(PasswordOK_Flag==1){
            		LOGIN_ID=5;
            		DisplaySettingPage();
            	}
            	else{
            		DisplayMsg("Wrong Password");
            		DisplayPage(LCD_LOGIN_OFF_WRONG_PW);
            	}
				break;

            case 0x05 : //
				InputPassword[0]=PW/1000;
				InputPassword[1]=(PW%1000)/100;
				InputPassword[2]=(PW%100)/10;
				InputPassword[3]=PW%10;
				if((InputPassword[0]==Masterpassword[0])&&(InputPassword[1]==Masterpassword[1])&&(InputPassword[2]==Masterpassword[2])&&(InputPassword[3]==Masterpassword[3])){
					DisplayDeveloperPage();
				}
				else{

				}
				break;




    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}


void PasswordCheck(){
	for(int j=0;j<4;j++){
		Savepassword[j]=IDLIST.PW[SelectID][j];
	}
	if((InputPassword[0]==Masterpassword[0])&&(InputPassword[1]==Masterpassword[1])&&(InputPassword[2]==Masterpassword[2])&&(InputPassword[3]==Masterpassword[3])){
		PasswordOK_Flag=1;
	}
	else if((InputPassword[0]==Savepassword[0])&&(InputPassword[1]==Savepassword[1])&&(InputPassword[2]==Savepassword[2])&&(InputPassword[3]==Savepassword[3])){
		PasswordOK_Flag=1;
	}
	else{
		PasswordOK_Flag=0;
	}
}

void LoginProcess(){
	if(PasswordOK_Flag==1){
		LOGIN_ID=SelectID;
		DisplayLoginStatus();
		Write_Flash();
		DisplayPage(LCD_SELECT_MENU);
		//Account_LastLogin_log_index	//로그인 시간
		SaveLastLoginLog();
		//Account_Attempts_log_display	//로그인 실패 횟수
		IDLIST.Attempts[LOGIN_ID]=0;
	}
	else{
		LOGIN_ID=SelectID;
		if(LOGIN_ID==5){
			DisplayMsg("Wrong Password");
		}
		else{
			IDLIST.Attempts[LOGIN_ID]++;
			//횟수 초과시 LOCK
			if(IDLIST.Attempts[LOGIN_ID]>=5){
				IDLIST.Status[LOGIN_ID]=0;
				DisplayMsg("Account Locked, Please contact your admin");
			}
			else{
				char msg[45] = "Wrong Password , Failed Login Attempts : ";
				msg[41]=IDLIST.Attempts[LOGIN_ID]+'0';
				DisplayMsg(msg);
			}
		}
		LOGIN_ID=0;
		Write_Flash();
		DisplayPage(LCD_LOGIN_WRONG_PW);
		//패스워드 틀림 알람
	}
	PasswordOK_Flag=0;
}

void DisplayFirstPage(){
    if(DeviceInfo.loginonoff_flag==1){
    	SelectID=0;

    	DisplayPage(LCD_LOGIN_ON_FIRST);
    }
    else{
    	DisplayPage(LCD_LOGIN_OFF_FIRST);
    }

}

void DisplayStatus(){
    DisplayBoardTemperature();
    DisplayModuleTemperature(0);    // Channel 0
    DisplayHuminity();
    DisplayDensity();
    DisplayH2O2Volume();

    DisplayPreHeatTime();
    DisplaySprayTime();
    DisplaySterileTime();
    DisplayScrubTime();
    DisplayFinishTime();
    DisplayTotalTime();

    //22.07.01
    DisplayTestTime();

    //23.06.01
    DisplayReserveTime();
}

void DisplayPreHeatTime(){
    DisplayTime(0x00, 0x11, uiWaitTime[1] + 99);
}

void DisplaySprayTime(){
    DisplayTime(0x00, 0x21, uiWaitTime[2] + 99);
}

void DisplaySterileTime(){
    DisplayTime(0x00, 0x31, uiWaitTime[3] + 99);
}

void DisplayScrubTime(){
    DisplayTime(0x00, 0x41, uiWaitTime[4] + 99);
}

void DisplayFinishTime(){
	DisplayTime(0x00, 0x51,uiFinishTime + 99);
}

void DisplayTotalTime(){
    DisplayTime(0x00, 0x14, uiTotalTime + 99);
}

void DisplayTestTime(){
    DisplayTime(0x00, 0xA1, TestTime + 99);
}


unsigned char   time_display[9] = {0x5A, 0xA5, 0x06, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00};
void DisplayTime(int page, int index, unsigned int icentisecond){
	time_display[4] = page;
	time_display[5] = index;
    unsigned int iMinute = icentisecond / 6000;
    unsigned int iHour = iMinute / 60;
    iMinute = iMinute - (iHour * 60);
    time_display[6] = hex2bcd(iHour);   // Hour
    time_display[7] = hex2bcd(iMinute); // Minute
    time_display[8] = hex2bcd((icentisecond / 100) % 60);   // Second
    HAL_UART_Transmit(&huart1, time_display, 9, 10);
}

void DisplayCubic(){
    DisplayUserValue(0, fCubic);
}

void DisplayBoardTemperature(){
    DisplayValue(0, fBoardTemperature);
}

void DisplayModuleTemperature(){
    DisplayValue(1, fModuleTemperature);
}

void DisplayHuminity(){
    DisplayValue(2, fHumidity);
}

void DisplayInjectionPerMinute(){
	/*
	if(DeviceInfo.device_version==8){
		DisplayUserValue(1, fInjectionPerMinute2);
	}
	else{
		DisplayUserValue(1, fInjectionPerMinute);
	}
	*/
	if(DeviceInfo.device_version==8){
		DisplayPageValue(0x02, 0x24, fInjectionPerMinute2);
	}
	else{
		DisplayPageValue(0x02, 0x24, fInjectionPerMinute);
	}

}

void DisplayInjectionPerCubic(){
	//DisplayUserValue(2, fInjectionPerCubic);
	DisplayPageValue(0x02, 0x34, fInjectionPerCubic);
}

void DisplayDensity(){
    DisplayValue(3, fDensity);
}

void DisplayH2O2Volume(){
    //DisplayValue(7, fH2O2Volume);
    unsigned int uivalue;
    if(DeviceInfo.device_version==8){
    	uivalue=fCubic*fInjectionPerCubic*10;
    }
    else{
    	uivalue = (RFIDData.fH2O2Volume * 10);
    }
    value_display[2] = 0x07;
    value_display[5] = value_index[4];

    value_display[6] = uivalue >> 24;
    value_display[7] = uivalue >> 16;
    value_display[8] = uivalue >> 8;
    value_display[9] = uivalue & 0xff;
    HAL_UART_Transmit(&huart1, value_display, 10, 10);
}

void DisplaySerialNumber(){
	switch(DeviceInfo.device_version){
		case 1 :
			serialNum[1]='F';
			serialNum[2]='N';
			serialNum[3]='-';
			serialNum[4]='S';
			serialNum[5]='4';
			serialNum[6]='0';
			serialNum[7]='0';
			break;
		case 2 :
			serialNum[1]='F';
			serialNum[2]='N';
			serialNum[3]='-';
			serialNum[4]='S';
			serialNum[5]='4';
			serialNum[6]='0';
			serialNum[7]='0';
			break;
		case 3 :
			serialNum[1]='F';
			serialNum[2]='N';
			serialNum[3]='-';
			serialNum[4]='S';
			serialNum[5]='3';
			serialNum[6]='0';
			serialNum[7]='0';
			break;
		case 4 :
			serialNum[1]='F';
			serialNum[2]='N';
			serialNum[3]='-';
			serialNum[4]='S';
			serialNum[5]='3';
			serialNum[6]='0';
			serialNum[7]='0';
			break;
		case 5 :
			serialNum[1]='C';
			serialNum[2]='B';
			serialNum[3]='T';
			serialNum[4]='_';
			serialNum[5]='S';
			serialNum[6]='V';
			serialNum[7]='C';
			break;
		case 6 :
			serialNum[1]='F';
			serialNum[2]='N';
			serialNum[3]='-';
			serialNum[4]='S';
			serialNum[5]='4';
			serialNum[6]='0';
			serialNum[7]='0';
			break;
		case 8 :
			serialNum[1]='F';
			serialNum[2]='N';
			serialNum[3]='-';
			serialNum[4]='S';
			serialNum[5]='1';
			serialNum[6]='0';
			serialNum[7]='0';
			break;
	}
	serialNum[8]=DeviceInfo.year;
	serialNum[9]=DeviceInfo.month;
	serialNum[10]=DeviceInfo.Serial1;
	serialNum[11]=DeviceInfo.Serial2;
	for(int i=1;i<12;i++){
		serial_display[5+i] = serialNum[i];
	}
    HAL_UART_Transmit(&huart1, serial_display, 17, 10);
}

void DisplayModemNumber(){
    ModemNum_display[6] = szStartCommand[21];
    ModemNum_display[7] = szStartCommand[22];
    ModemNum_display[8] = szStartCommand[23];
    ModemNum_display[9] = 0x00;
    for(int i=0;i<8;i++){
    	ModemNum_display[10+i] = szStartCommand[24+i];
    }
    HAL_UART_Transmit(&huart1, ModemNum_display, 18, 10);
}

void DisplayUserNumber(){
    UserNum_display[6] = szStartCommand[9];
    UserNum_display[7] = szStartCommand[10];
    UserNum_display[8] = szStartCommand[11];
    UserNum_display[9] = 0x00;
    for(int i=0;i<8;i++){
    	UserNum_display[10+i] = szStartCommand[12+i];
    }
    HAL_UART_Transmit(&huart1, UserNum_display, 18, 10);
}
void DisplayUserNumberblank(){
	UserNum_blank_display[2] = 0x0d;
	UserNum_blank_display[6] = 0;
	UserNum_blank_display[7] = 0;

	UserNum_blank_display[14] = 0;
	UserNum_blank_display[15] = 0;

    HAL_UART_Transmit(&huart1, UserNum_blank_display, 16, 10);
}

void DisplayValue(int index, float value){
    unsigned int uivalue;
    uivalue = (value * 10);
    value_display[2] = 0x05;
    value_display[5] = value_index[index];
    value_display[6] = uivalue >> 8;
    value_display[7] = uivalue & 0xff;
    HAL_UART_Transmit(&huart1, value_display, 8, 10);
}

void DisplayUserValue(int index, float value){
    unsigned int uivalue;
    uivalue = (value * 10);
    user_value_display[2] = 0x05;
    user_value_display[5] = user_value_index[index];
    user_value_display[6] = uivalue >> 8;
    user_value_display[7] = uivalue & 0xff;
    HAL_UART_Transmit(&huart1, user_value_display, 8, 10);
}
void DisplayScrubPopUp(void){
	__disable_irq();
	main_page[6]=0x1b;
	HAL_UART_Transmit(&huart1, main_page, 7, 10);
	__enable_irq();
}


void DisplayLoginStatus(){
    if(LOGIN_ID==1){
		Loginstatus_display[6]='U';
		Loginstatus_display[7]='S';
		Loginstatus_display[8]='E';
		Loginstatus_display[9]='R';
		Loginstatus_display[10]='1';
    }
    else if(LOGIN_ID==2){
		Loginstatus_display[6]='U';
		Loginstatus_display[7]='S';
		Loginstatus_display[8]='E';
		Loginstatus_display[9]='R';
		Loginstatus_display[10]='2';
    }
    else if(LOGIN_ID==3){
		Loginstatus_display[6]='U';
		Loginstatus_display[7]='S';
		Loginstatus_display[8]='E';
		Loginstatus_display[9]='R';
		Loginstatus_display[10]='3';
    }
    else if(LOGIN_ID==4){
		Loginstatus_display[6]='S';
		Loginstatus_display[7]='U';
		Loginstatus_display[8]='S';
		Loginstatus_display[9]='E';
		Loginstatus_display[10]='R';
    }
    else if(LOGIN_ID==5){
		Loginstatus_display[6]='A';
		Loginstatus_display[7]='D';
		Loginstatus_display[8]='M';
		Loginstatus_display[9]='I';
		Loginstatus_display[10]='N';
    }
    else{
		Loginstatus_display[6]=' ';
		Loginstatus_display[7]=' ';
		Loginstatus_display[8]=' ';
		Loginstatus_display[9]=' ';
		Loginstatus_display[10]=' ';
    }
			/*
		case 2:

			break;
		case 3:
			Loginstatus_display[6]='U';
			Loginstatus_display[7]='S';
			Loginstatus_display[8]='E';
			Loginstatus_display[9]='R';
			Loginstatus_display[10]='3';
			break;
		case 4:
			Loginstatus_display[6]='S';
			Loginstatus_display[7]='U';
			Loginstatus_display[8]='S';
			Loginstatus_display[9]='E';
			Loginstatus_display[10]='R';
			break;
		case 5:
			Loginstatus_display[6]='A';
			Loginstatus_display[7]='D';
			Loginstatus_display[8]='M';
			Loginstatus_display[9]='I';
			Loginstatus_display[10]='N';
			break;
    }
    */

	HAL_UART_Transmit(&huart1, Loginstatus_display, 11, 10);
}

void DisplayPage(int page){
	main_page[6] = page;
    HAL_UART_Transmit(&huart1, main_page, 7, 10);
}

void DisplayOperationPage(void){
	DisplayPage(LCD_OPERATTION_NORMAL);
}

void DisplaySettingPage(){
	DisplaySerialNumber();
	DisplayHardwareVersion();
	//DisplayVersion('3','1','3');
	DisplayModemNumber();

	SelectID=LOGIN_ID;
	switch(LOGIN_ID){
		case 1:
			DisplayUserNumber();
			DisplaySMSonoffIcon((SMSonoff_Flag&0x01)==0x01);
			DisplayUSBSecurityonoffIcon(USBSECURITYonoff_Flag);
			DisplayPage(LCD_USER_SETTING);
			break;
		case 2:
			DisplayUserNumber();
			DisplaySMSonoffIcon((SMSonoff_Flag&0x01)==0x01);
			DisplayUSBSecurityonoffIcon(USBSECURITYonoff_Flag);
			DisplayPage(LCD_USER_SETTING);
			break;
		case 3:
			DisplayUserNumber();
			DisplaySMSonoffIcon((SMSonoff_Flag&0x01)==0x01);
			DisplayUSBSecurityonoffIcon(USBSECURITYonoff_Flag);
			DisplayPage(LCD_USER_SETTING);
			break;
		case 4:
			DisplayUserNumber();
			DisplaySMSonoffIcon((SMSonoff_Flag&0x01)==0x01);
			DisplayUSBSecurityonoffIcon(USBSECURITYonoff_Flag);
			DisplayPage(LCD_SUSER_SETTING);
			break;
		case 5:
			DisplayUserNumber();
			DisplaySMSonoffIcon((SMSonoff_Flag&0x01)==0x01);
			DisplayUSBSecurityonoffIcon(USBSECURITYonoff_Flag);
			if(DeviceInfo.loginonoff_flag==1){
				DisplayPage(LCD_ADMIN_SETTING);
			}
			else{
				DisplayPage(LCD_LOGINOFF_SETTING);
			}
			break;
	}
}

void DisplayRunningPage(void){
	main_page[6] = 0x0c;
    HAL_UART_Transmit(&huart1, main_page, 7, 10);
}


// 아이콘 출력
void DisplayModeIcon(int mode){
    DisplayPreHeatIcon(0);
    DisplaySprayIcon(0);
    DisplaySterileIcon(0);
    DisplayScrubIcon(0);
    DisplayFinishIcon(0);
    if(mode == 1) {
        DisplayPreHeatIcon(1);
    }
    else if(mode == 2) {
        DisplaySprayIcon(1);
    }
    else if(mode == 3) {
        DisplaySterileIcon(1);
    }
    else if(mode == 4) {
        DisplayScrubIcon(1);
    }
    else if(mode == 5) {
        DisplayFinishIcon(1);
    }
    DisplayUSBIcon(USBConnect_Flag);
    DisplayH2O2SensorIcon(H2O2Sensor_Flag);
}

void DisplayPreHeatIcon(int value){
    DisplayIcon(0, value);
}

void DisplaySprayIcon(int value){
    DisplayIcon(1, value);
}

void DisplaySterileIcon(int value){
    DisplayIcon(2, value);
}

void DisplayScrubIcon(int value){
    DisplayIcon(3, value);
}

void DisplayFinishIcon(int value){
    DisplayIcon(4, value);
}

void DisplayStartIcon(int value){
    DisplayIcon(5, value);
}

void DisplaySMSonoffIcon(int value){
    DisplayIcon(6, value);
}

void DisplayUSBSecurityonoffIcon(int value){
    DisplayIcon(7, value);
}

void DisplayServerSMSonoffIcon(int value){
    DisplayIcon(8, value);
}

void DisplayUSBIcon(int value){
    DisplayIcon(9, value);
    DisplayIcon(10, value);
}

void DisplayH2O2SensorIcon(int value){
    DisplayIcon(11, value);
    DisplayIcon(12, value);
}

void DisplayACCOUNTActiveIcon(){
    DisplayIcon2(0, IDLIST.Status[1]);
    DisplayIcon2(1, IDLIST.Status[2]);
    DisplayIcon2(2, IDLIST.Status[3]);
    DisplayIcon2(3, IDLIST.Status[4]);
}
void DisplayRESERVEATOINOnoffIcon(){
	DisplayIcon(13,DeviceInfo.reservationonoff_flag);
}
void DisplayLOGINOnoffIcon(){
	DisplayIcon(14,DeviceInfo.loginonoff_flag);
}

void DisplayIcon(int index, int value){
    icon_display[5] = icon_index[index];
    icon_display[7] = value;
    HAL_UART_Transmit(&huart1, icon_display, 8, 10);
}

void DisplayIcon2(int index, int value){
	//1010
	unsigned char   icon_display2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x60, 0x00, 0x01};
    icon_display2[5] = icon_index2[index];
    icon_display2[7] = value;
    HAL_UART_Transmit(&huart1, icon_display2, 8, 10);
}

void DisplayPopUpMessage(char *msg){
	DisplayMsg(msg);
    DisplayPage(LCD_OPERATTION_MESSAGE);
    __enable_irq();
}

void DisplayMsg(char *msg){
	__disable_irq();
    memset(error_display + 6, 0, 80);
    for(int i = 0; i < 80; i++) {
        error_display[6 + i] = msg[i];
    }
    HAL_UART_Transmit(&huart1, error_display, 86, 10);
    memset(error_display + 6, 0, 80);
    __enable_irq();
}

void DisplayDebug(char *msg)
{
	DisplayPage8Char(0x10,0x50,msg);
}

void DisplayVersion(char ch1, char ch2, char ch3){
	version_display[6]=ch1;
	version_display[7]='.';
	version_display[8]=ch2;
	version_display[9]='.';
	version_display[10]=ch3;
	HAL_UART_Transmit(&huart1, version_display, 11, 10);
}

void ReadRTC(unsigned char *year, unsigned char *month, unsigned char *day, unsigned char *week, unsigned char *hour, unsigned char *minute, unsigned char *second){
    __disable_irq();
    huart1.RxState= HAL_UART_STATE_READY;
    memset(uart1_rx_data, 0, 20);
    HAL_UART_Transmit(&huart1, (uint8_t*)rtc_date_get, 6, 10);
    HAL_UART_Receive(&huart1, (uint8_t*)uart1_rx_data, 9, 10);
    *year = uart1_rx_data[6];
    *month = uart1_rx_data[7];
    *day = uart1_rx_data[8];
    *week = uart1_rx_data[9];

    memset(uart1_rx_data, 0, 20);
    HAL_UART_Transmit(&huart1, (uint8_t*)rtc_time_get, 6, 10);
    HAL_UART_Receive(&huart1, (uint8_t*)uart1_rx_data, 9, 10);
    *hour = uart1_rx_data[6];
    *minute = uart1_rx_data[7];
    *second = uart1_rx_data[8];

    UART_Receive_Flag = 0;
    __enable_irq();
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

void SetRTCFromLCD(){
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    ReadRTC(&sDate.Year, &sDate.Month, &sDate.Date, &sDate.WeekDay, &sTime.Hours, &sTime.Minutes, &sTime.Seconds);

    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
    {
        Error_Handler();
    }
}



void DisplayLogListPage(){
	//작업이력
	for(int i=1;i<5;i++){
		DisplayIDlog(i);
		DisplayDatelog(i);
		DisplayStartTimelog(i);
		DisplayEndTimelog(i);
		DisplayCubiclog(i);
		Displaystatuslog(i);
	}
    DisplayPage(HISTORY_PAGE);
}


//수정중
void DisplayH2O2ListPage(){
	//작업이력
	for(int i=1;i<5;i++){
		DisplayRFIDIDlog(i);
		DisplayRFIDVolumelog(i);
		DisplayRFIDDatelog(i);
	}
    DisplayPage(HISTORY_PAGE);
}

//수정중
void DisplayRFIDIDlog(int index){	//동작 중 중지
	if(RFIDData.RFIDValue[index][0]!='5'){
		H2O2RFID_log_display[5]=H2O2RFID_log_index[index];
		H2O2RFID_log_display[6]=RFIDData.RFIDValue[index][1];
		H2O2RFID_log_display[7]=RFIDData.RFIDValue[index][2];
		H2O2RFID_log_display[8]=RFIDData.RFIDValue[index][3];
	}
	else{
		H2O2RFID_log_display[5]=H2O2RFID_log_index[index];
		H2O2RFID_log_display[6]=' ';
		H2O2RFID_log_display[7]='-';
		H2O2RFID_log_display[8]=' ';
	}
	HAL_UART_Transmit(&huart1, H2O2RFID_log_display, 9, 10);
}


void DisplayRFIDVolumelog(int index){
    unsigned int uivalue;
    uivalue = (RFIDData.H2O2Volume[index] * 10);
    H2O2Volume_log_display[5] = H2O2Volume_log_index[index];
    H2O2Volume_log_display[6] = uivalue >> 24;
    H2O2Volume_log_display[7] = uivalue >> 16;
    H2O2Volume_log_display[8] = uivalue >> 8;
    H2O2Volume_log_display[9] = uivalue & 0xff;
    HAL_UART_Transmit(&huart1, H2O2Volume_log_display, 10, 10);
}

void DisplayRFIDDatelog(int index){	//동작 중 중지
	if(RFIDData.year[index]!=0){
		H2O2Date_log_display[5]=H2O2Date_log_index[index];
		H2O2Date_log_display[6]=(RFIDData.year[index]/10%10)+'0';
		H2O2Date_log_display[7]=(RFIDData.year[index]%10)+'0';
		H2O2Date_log_display[8]='.';
		H2O2Date_log_display[9]=(RFIDData.month[index]/10%10)+'0';
		H2O2Date_log_display[10]=(RFIDData.month[index]%10)+'0';
	}
	else{
		H2O2Date_log_display[5]=H2O2Date_log_index[index];
		H2O2Date_log_display[6]=' ';
		H2O2Date_log_display[7]=' ';
		H2O2Date_log_display[8]='-';
		H2O2Date_log_display[9]=' ';
		H2O2Date_log_display[10]=' ';
	}
	HAL_UART_Transmit(&huart1, H2O2Date_log_display, 11, 10);
}




////////////Account Settings/////////


void DisplayAccountSettingPage(){
	//ID, PW, Last Login date, Failed Login Attempts, Status
	for(int i=1;i<5;i++){
		DisplayAccountIDlog(i);
		DisplayAccountPWlog(i);
		DisplayAccountLastLoginlog(i);
		DisplayAccountAttemptslog(i);
		DisplayAccountStatuslog(i);
	}
	DisplayACCOUNTActiveIcon();
	DisplayPage(ACCOUNT_SETTING);

}



void DisplayAccountIDlog(int index){
	Account_ID_log_display[5]=Account_ID_log_index[index];
	switch(index){
		case USER1 :
			Account_ID_log_display[6]='U';
			Account_ID_log_display[7]='S';
			Account_ID_log_display[8]='E';
			Account_ID_log_display[9]='R';
			Account_ID_log_display[10]='1';
			break;
		case USER2 :
			Account_ID_log_display[6]='U';
			Account_ID_log_display[7]='S';
			Account_ID_log_display[8]='E';
			Account_ID_log_display[9]='R';
			Account_ID_log_display[10]='2';
			break;
		case USER3 :
			Account_ID_log_display[6]='U';
			Account_ID_log_display[7]='S';
			Account_ID_log_display[8]='E';
			Account_ID_log_display[9]='R';
			Account_ID_log_display[10]='3';
			break;
		case SUPERUSER :
			Account_ID_log_display[6]='S';
			Account_ID_log_display[7]='U';
			Account_ID_log_display[8]='S';
			Account_ID_log_display[9]='E';
			Account_ID_log_display[10]='R';
			break;
	}

	HAL_UART_Transmit(&huart1, Account_ID_log_display, 11, 10);
}

void DisplayAccountPWlog(int index){
	Account_PW_log_display[5]=Account_PW_log_index[index];

	Account_PW_log_display[6]=IDLIST.PW[index][0]+'0';
	Account_PW_log_display[7]=IDLIST.PW[index][1]+'0';
	Account_PW_log_display[8]=IDLIST.PW[index][2]+'0';
	Account_PW_log_display[9]=IDLIST.PW[index][3]+'0';

	HAL_UART_Transmit(&huart1, Account_PW_log_display, 10, 10);
}

void DisplayAccountLastLoginlog(int index){
	Account_LastLogin_log_display[5] = Account_LastLogin_log_index[index];
	Account_LastLogin_log_display[6] = IDLIST.year[index];
	Account_LastLogin_log_display[7] = IDLIST.month[index];
	Account_LastLogin_log_display[8] = IDLIST.day[index];
	Account_LastLogin_log_display[9] = IDLIST.hour[index];
	Account_LastLogin_log_display[10] = IDLIST.minute[index];
	HAL_UART_Transmit(&huart1,Account_LastLogin_log_display, 11, 10);
}

void DisplayAccountAttemptslog(int index){
	Account_Attempts_log_display[5]=Account_Attempts_log_index[index];

	Account_Attempts_log_display[6]=(int)IDLIST.Attempts[index]+'0';

	HAL_UART_Transmit(&huart1, Account_Attempts_log_display, 7, 10);
}


void DisplayAccountStatuslog(int index){
	Account_Status_log_display[5]=Account_Status_log_index[index];
	if(IDLIST.Status[index]==1){
		Account_Status_log_display[6] = 'A';
		Account_Status_log_display[7] = 'c';
		Account_Status_log_display[8] = 't';
		Account_Status_log_display[9] = 'i';
		Account_Status_log_display[10] = 'v';
		Account_Status_log_display[11] = 'e';
	}
	else{
		Account_Status_log_display[6] = 'L';
		Account_Status_log_display[7] = 'o';
		Account_Status_log_display[8] = 'c';
		Account_Status_log_display[9] = 'k';
		Account_Status_log_display[10] = 'e';
		Account_Status_log_display[11] = 'd';
	}
	HAL_UART_Transmit(&huart1, Account_Status_log_display, 12, 10);
}




void DisplayReserveSettings(){
	DisplayReserveTime();
	DisplayPage(LCD_RESERVESETTING_PAGE);
}

void DisplayReserveTimePage(){
	DisplayReserveTime();
	DisplayPage(LCD_RESERVETIME_PAGE);
}

void DisplayReserveTime(){
    DisplayTime(0x00, 0x47, uireservetime + 99);
}

void DisplayReservePopUp(char *msg){
	unsigned char   display_page[7] = {0x5A, 0xA5, 0x04, 0x80, 0x03, 0x00, 0x00};
	unsigned char   msg_display[100] = {
	        0x5a, 0xa5, 0x53, 0x82, 0x01, 0x00, 0x4e, 0x75, 0x72, 0x69, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d
	};
    __disable_irq();
    memset(msg_display + 6, 0, 80);
    for(int i = 0; i < 80; i++) {
    	msg_display[6 + i] = msg[i];
    }
    HAL_UART_Transmit(&huart1, msg_display, 86, 10);

    memset(msg_display + 6, 0, 80);

    __enable_irq();

	display_page[6]=LCD_RESERVEPOPUP_PAGE;
	HAL_UART_Transmit(&huart1, display_page, 7, 10);
}



void LCD_31(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					DisplaySelectIcon(0x30,0x10,1);
					DisplaySelectIcon(0x30,0x20,0);
					DisplaySelectIcon(0x30,0x30,0);
					DisplaySelectIcon(0x30,0x40,0);
					Read_LogData_Flash(1);
					select_index=1;
					break;
				case 0x02 :
					DisplaySelectIcon(0x30,0x10,0);
					DisplaySelectIcon(0x30,0x20,1);
					DisplaySelectIcon(0x30,0x30,0);
					DisplaySelectIcon(0x30,0x40,0);
					Read_LogData_Flash(2);
					select_index=2;
					break;
				case 0x03 :
					DisplaySelectIcon(0x30,0x10,0);
					DisplaySelectIcon(0x30,0x20,0);
					DisplaySelectIcon(0x30,0x30,1);
					DisplaySelectIcon(0x30,0x40,0);
					Read_LogData_Flash(3);
					select_index=3;
					break;
				case 0x04 :
					DisplaySelectIcon(0x30,0x10,0);
					DisplaySelectIcon(0x30,0x20,0);
					DisplaySelectIcon(0x30,0x30,0);
					DisplaySelectIcon(0x30,0x40,1);
					Read_LogData_Flash(4);
					select_index=4;
					break;
				case 0x05 :
					DisplayUserNumber();
					DisplaySMSonoffIcon((SMSonoff_Flag&0x01)==0x01);
					DisplayUSBSecurityonoffIcon(USBSECURITYonoff_Flag);
					if(DeviceInfo.loginonoff_flag==1){
						DisplayPage(LCD_ADMIN_SETTING);
					}
					else{
						DisplayPage(LCD_LOGINOFF_SETTING);
					}
					break;
				case 0x06 :	//Download
					DownloadUSB2(select_index);
					break;
			}
			break;
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}


void Display31page(){
	DisplayPage(HISTORY_PAGE);
	DisplayLogListPage();

}

void Displaystatuslog(int index){	//동작 중 중지
	status_log_display[5]=status_log_index[index];
	if(startData.status[index]==1){
		status_log_display[6] = 'S';
		status_log_display[7] = 'h';
		status_log_display[8] = 'u';
		status_log_display[9] = 't';
		status_log_display[10] = 'd';
		status_log_display[11] = 'o';
		status_log_display[12] = 'w';
		status_log_display[13] = 'n';
	}
	else if(startData.status[index]==2){
		status_log_display[6] = ' ';
		status_log_display[7] = 'C';
		status_log_display[8] = 'a';
		status_log_display[9] = 'n';
		status_log_display[10] = 'c';
		status_log_display[11] = 'e';
		status_log_display[12] = 'l';
		status_log_display[13] = ' ';
	}
	else if(startData.status[index]==3){
		status_log_display[6] = 'C';
		status_log_display[7] = 'o';
		status_log_display[8] = 'm';
		status_log_display[9] = 'p';
		status_log_display[10] = 'l';
		status_log_display[11] = 'e';
		status_log_display[12] = 't';
		status_log_display[13] = 'e';
	}
	else{
		status_log_display[6] = ' ';
		status_log_display[7] = ' ';
		status_log_display[8] = ' ';
		status_log_display[9] = ' ';
		status_log_display[10] = ' ';
		status_log_display[11] = ' ';
		status_log_display[12] = ' ';
		status_log_display[13] = ' ';
	}
	HAL_UART_Transmit(&huart1, status_log_display, 14, 10);
}

void DisplayIDlog(int index){	//동작 중 중지
	ID_log_display[5]=ID_log_index[index];
	if(startData.ID[index]==1){
		ID_log_display[6]='U';
		ID_log_display[7]='S';
		ID_log_display[8]='E';
		ID_log_display[9]='R';
		ID_log_display[10]='1';
    }
    else if(startData.ID[index]==2){
		ID_log_display[6]='U';
		ID_log_display[7]='S';
		ID_log_display[8]='E';
		ID_log_display[9]='R';
		ID_log_display[10]='2';
    }
    else if(startData.ID[index]==3){
		ID_log_display[6]='U';
		ID_log_display[7]='S';
		ID_log_display[8]='E';
		ID_log_display[9]='R';
		ID_log_display[10]='3';
    }
    else if(startData.ID[index]==4){
		ID_log_display[6]='S';
		ID_log_display[7]='U';
		ID_log_display[8]='S';
		ID_log_display[9]='E';
		ID_log_display[10]='R';
    }
    else if(startData.ID[index]==5){
    	ID_log_display[6]='A';
    	ID_log_display[7]='D';
    	ID_log_display[8]='M';
    	ID_log_display[9]='I';
    	ID_log_display[10]='N';
    }
    else{
    	ID_log_display[6]=' ';
    	ID_log_display[7]=' ';
    	ID_log_display[8]='-';
    	ID_log_display[9]=' ';
    	ID_log_display[10]=' ';
    }
	HAL_UART_Transmit(&huart1, ID_log_display, 11, 10);
}


void DisplayDatelog(int index){
	unsigned char   Time_log_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x01, 0x00, 0x00, 0x00, 0x00};
	unsigned char   Display_Address[5]={0x00,0x05,0x25,0x45,0x65};
	Time_log_display[2] = 6; //데이터 크기
	Time_log_display[4] = 0x31;
	Time_log_display[5] = Display_Address[index];
	Time_log_display[6] = startData.year[index];
	Time_log_display[7] = startData.month[index];
	Time_log_display[8] = startData.day[index];
	HAL_UART_Transmit(&huart1,Time_log_display, 9, 10);
}

void DisplayStartTimelog(int index){
	unsigned char   Time_log_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x01, 0x00, 0x00, 0x00, 0x00};
	unsigned char   Display_Address[5]={0x00,0x10,0x30,0x50,0x70};
	Time_log_display[2] = 5; //데이터 크기
	Time_log_display[4] = 0x31;
	Time_log_display[5] = Display_Address[index];
	Time_log_display[6] = startData.hour[index];
	Time_log_display[7] = startData.minute[index];
	HAL_UART_Transmit(&huart1,Time_log_display, 8, 10);
}

void DisplayEndTimelog(int index){
	unsigned char   Time_log_display[11] = {0x5A, 0xA5, 0x08, 0x82, 0x01, 0x00, 0x00, 0x00, 0x00};
	unsigned char   Display_Address[5]={0x00,0x15,0x35,0x55,0x75};
	Time_log_display[2] = 5; //데이터 크기
	Time_log_display[4] = 0x31;
	Time_log_display[5] = Display_Address[index];
	Time_log_display[6] = endData.hour[index];
	Time_log_display[7] = endData.minute[index];
    HAL_UART_Transmit(&huart1,Time_log_display, 8, 10);
}

void DisplayCubiclog(int index){
	unsigned char   log_value_display[8] = {0x5A, 0xA5, 0x05, 0x82, 0x01, 0x00, 0x00, 0x00};
	unsigned char   Display_Address[5]={0x00,0x1A,0x3A,0x5A,0x7A};
    unsigned int uivalue;
    uivalue = startData.cubic[index];
    log_value_display[2] = 5; //데이터 크기
    log_value_display[4] = 0x31;
    log_value_display[5] = Display_Address[index];
    log_value_display[6] = uivalue >> 8;
    log_value_display[7] = uivalue & 0xff;
    HAL_UART_Transmit(&huart1, log_value_display, 8, 10);
}
void DisplaySelectIcon(int page, int index, int value){
	unsigned char   Select_icon_display[8] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x60, 0x00, 0x01};
	Select_icon_display[4] = page;
	Select_icon_display[5] = index;
    Select_icon_display[7] = value;
    HAL_UART_Transmit(&huart1, Select_icon_display, 8, 10);
}


//23.06.29
void DisplaySolutionCheckPopUp(char *msg){
	unsigned char   display_page[7] = {0x5A, 0xA5, 0x04, 0x80, 0x03, 0x00, 0x00};
	unsigned char   msg_display[100] = {
	        0x5a, 0xa5, 0x53, 0x82, 0x01, 0x00, 0x4e, 0x75, 0x72, 0x69, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d
	};
    __disable_irq();
    memset(msg_display + 6, 0, 80);
    for(int i = 0; i < 80; i++) {
    	msg_display[6 + i] = msg[i];
    }
    HAL_UART_Transmit(&huart1, msg_display, 86, 10);

    memset(msg_display + 6, 0, 80);

    __enable_irq();

	display_page[6]=LCD_SOLUTIONCHECK_PopUp;
	HAL_UART_Transmit(&huart1, display_page, 7, 10);
}

void LCD_06(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
		    		SelectID=ID0;
		    		DisplayPage(LCD_LOGIN_SELECT_ID);
					break;
				case 0x03 :
					DisplayMsg("Please input password");
					DisplayPage(LCD_LOGIN_WRONG_PW);
					break;
			}
			break;
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}
void LCD_07(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
		    		SelectID=ID0;
		    		DisplayPage(LCD_LOGIN_SELECT_ID);
					break;
				case 0x03 :
					if(IDLIST.Status[SelectID]==1||SelectID==5){
						PasswordCheck();
						LoginProcess();
					}
					else if(IDLIST.Status[SelectID]==0){
						DisplayMsg("Account Locked, Please contact your admin");
						LOGIN_ID=0;
						DisplayPage(LCD_LOGIN_WRONG_PW);
					}
					break;
			}
			break;
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

void Display06page(){
	switch(SelectID){
		case USER1 :
			DisplayPage10Char(0x06,0x10,"   User1  ");
			break;
		case USER2 :
			DisplayPage10Char(0x06,0x10,"   User2  ");
			break;
		case USER3 :
			DisplayPage10Char(0x06,0x10,"   User3  ");
			break;
		case SUPERUSER :
			DisplayPage10Char(0x06,0x10," Superuser");
			break;
		case ADMIN :
			DisplayPage10Char(0x06,0x10,"   Admin  ");
			break;
	}

	DisplayPage(LCD_SELECTED_ID);
}


void LCD_51(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					//Test_flag=18;
					if(DeviceInfo.device_version==8){
						TestfInjectionPerMinute=fInjectionPerMinute2;
					}
					else{
						TestfInjectionPerMinute=fInjectionPerMinute;
					}
					Display52page();
					DisplayPage(PERI_TEST_PAGE);
					break;
				case 0x02 :
					//Test_flag=18;
					Display53page();
					DisplayPage(FAN_TEST_PAGE);
					break;
				case 0x04 :
					DisplayMsg("Do you want to reset configuration");
					DisplayPage(SETTING_RESET_POPUP_PAGE);
					break;
				case 0x05 :
					Test_flag=0;
					DisplayDeveloperPage();
					break;
			}
			break;
		case 0x10 :
        	if(value>=50&&value<=150){
        		DeviceInfo.peri1_speed = value;
			}
        	else{
        		DeviceInfo.peri1_speed=100;
        	}
			PeristalticSpeed();
			Write_Flash();
			Display51page();
			//DisplayDeveloperPage();
			break;
		case 0x20 :
        	if(value>=50&&value<=150){
        		DeviceInfo.peri2_speed = value;
			}
        	else{
        		DeviceInfo.peri2_speed=100;
        	}
			PeristalticSpeed();
			Write_Flash();
			Display51page();
			//DisplayDeveloperPage();
			break;
		case 0x30 :
        	if(value>=20&&value<=100){
        		DeviceInfo.fan_high_speed = value;
			}
        	else{
        		DeviceInfo.fan_high_speed=ConstantBlowerFanControlPwmMax;
        	}
        	InitFanPump();
			Write_Flash();
			Display51page();
			//DisplayDeveloperPage();
			break;
		case 0x40 :
        	if(value>=20&&value<=100){
        		DeviceInfo.fan_low_speed = value;
			}
        	else{
        		DeviceInfo.fan_low_speed=ConstantBlowerFanControlPwmMin;
        	}
        	InitFanPump();
			Write_Flash();
			Display51page();
			//DisplayDeveloperPage();
			break;
		case 0x50 :
        	if(value/10>=20&&value/10<=65){
        		DeviceInfo.lower_temperature=(float)value/10;
			}
        	else{
        		DeviceInfo.lower_temperature=ConstantLowerTemperature;
        	}
			Write_Flash();
			Display51page();
			//DisplayDeveloperPage();
			break;
		case 0x60 :
        	if(value/10>=20&&value/10<=65){
        		DeviceInfo.upper_temperature=(float)value/10;
			}
        	else{
        		DeviceInfo.upper_temperature=ConstantUpperTemperature;
        	}
			Write_Flash();
			Display51page();
			//DisplayDeveloperPage();
			break;
		case 0x70 :
        	if(value/10>=75){
        		DeviceInfo.overheat_temperature=(float)value/10;
			}
        	else{
        		DeviceInfo.overheat_temperature=OverHeat_Temperature;
        	}
			Write_Flash();
			Display51page();
				break;
		case 0x80 :
        	if(value>=2&&value<=10){
        		DeviceInfo.PreHeatTime=value;
			}
        	else{
        		DeviceInfo.PreHeatTime=ConstantPreHeatTime;
        	}
			Write_Flash();
			ReCalcTime();
			Display51page();
			break;
		case 0x90 :
        	if(value>=3&&value<=240){
        		if((DeviceInfo.LineCleanTime+DeviceInfo.NozzleCleanTime)<value){
        			DeviceInfo.SterileTime=value;
        		}
        		else{
        			DeviceInfo.SterileTime=ConstantSterileTime;
        		}
			}
        	else{
        		DeviceInfo.SterileTime=ConstantSterileTime;
        	}
			Write_Flash();
			ReCalcTime();
			Display51page();
			break;
		case 0xA0 :
			if(value>=1&&value<=5){
				if((DeviceInfo.SterileTime-DeviceInfo.LineCleanTime)<=value){
					DeviceInfo.NozzleCleanTime=ConstantNozzleCleanTime;
				}
				else{
					DeviceInfo.NozzleCleanTime=value;
				}
			}
        	else{
        		DeviceInfo.NozzleCleanTime=ConstantNozzleCleanTime;
        	}
			Write_Flash();
			ReCalcTime();
			Display51page();
			break;

    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

void LCD_52(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					Test_Start_flag=1;
					TestTime=input_test_time;
					if(TestfInjectionPerMinute==5){
						DisplayMsg("Peri_Pump(5cc/min) Testing.");
						Test_flag=16;
					}
					else if(TestfInjectionPerMinute==4){
						DisplayMsg("Peri_Pump(4cc/min) Testing.");
						Test_flag=17;
					}
					else if(TestfInjectionPerMinute==3){
						DisplayMsg("Peri_Pump(3cc/min) Testing.");
						Test_flag=18;
					}
					else if(TestfInjectionPerMinute==15){
						DisplayMsg("Peri_Pump(15cc/min) Testing.");
						Test_flag=16;
					}
					else if(TestfInjectionPerMinute==12){
						DisplayMsg("Peri_Pump(12cc/min) Testing.");
						Test_flag=17;
					}
					else if(TestfInjectionPerMinute==9){
						DisplayMsg("Peri_Pump(9cc/min) Testing.");
						Test_flag=18;
					}
					DisplayPage(TEST_YES_PAGE);
					ProcessTestEndTimer();
					break;
				case 0x02 :
					DisplayPage(CHANGE_VALUE_PAGE);
					break;
			}
			break;
		case 0x10 :
			if(value<=60){
				input_test_time=value*60*100;
			}
			else{
				input_test_time=5*60*100;
			}
			PeristalticSpeed();
			Display52page();
			//DisplayDeveloperPage();
			break;
		case 0x20 :
			if(DeviceInfo.device_version==8){
				if(value==3){
					TestfInjectionPerMinute=3;
				}
				else if(value==5){
					TestfInjectionPerMinute=5;
				}
				else{
					TestfInjectionPerMinute=4;
				}
			}
			else{
				if(value==9){
					TestfInjectionPerMinute=9;
				}
				else if(value==15){
					TestfInjectionPerMinute=15;
				}
				else{
					TestfInjectionPerMinute=12;
				}

			}
			//PeristalticSpeed();
			Display52page();
			//DisplayDeveloperPage();
			break;
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

void LCD_53(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					Test_Start_flag=1;
					TestTime=input_test_time;
					DisplayMsg("Fan testing.");
					Test_flag=21;//FAN
					DisplayPage(TEST_YES_PAGE);
					ProcessTestEndTimer();
					break;
				case 0x02 :
					DisplayPage(CHANGE_VALUE_PAGE);
					break;
			}
			break;
		case 0x10 :
				if(value<=60){
					input_test_time=value*60*100;
				}
				else{
					input_test_time=5*60*100;
				}
				Display53page();
				//DisplayDeveloperPage();
			break;
		case 0x20 :
				if(value>=20&&value<=100){
					Testfanspeed=value;
				}
				else{
					Testfanspeed=50;
				}
				Display53page();
				//DisplayDeveloperPage();
			break;
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

void LCD_54(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					Test_Start_flag=0;
					TurnOffFanPump();
					TurnOffPeristalticPump();
					if(Test_flag==20){
						DisplayMsg("Test Canceled.\n\r"
								"Please turn off and on the device.");
						DisplayTime(0x55,0x10,TestTime);
					}
					else{
						DisplayMsg("Canceled");
						DisplayTime(0x55,0x10,TestTime);
					}
					DisplayPage(TEST_COMPLETE_PAGE);
					TestTime=0;
					break;
			}
			break;
		case 0x10 :
			break;
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

void LCD_55(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					Test_Start_flag=0;
					Test_flag=0;
					TurnOffFanPump();
					TurnOffPeristalticPump();
					Display51page();
					break;
			}
			break;
		case 0x10 :
			break;
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

void LCD_56(int index, int value){	//input Value
    switch(index) {
		case 0x00 :
			switch(value) {
				case 0x01 :
					DeviceInfo.peri1_speed=100;
					DeviceInfo.peri2_speed=100;
	        		DeviceInfo.fan_high_speed=ConstantBlowerFanControlPwmMax;
	        		DeviceInfo.fan_low_speed=ConstantBlowerFanControlPwmMin;
	        		DeviceInfo.lower_temperature=ConstantLowerTemperature;
	        		DeviceInfo.upper_temperature=ConstantUpperTemperature;
	        		DeviceInfo.overheat_temperature=OverHeat_Temperature;
					PeristalticSpeed();
					Write_Flash();
					Display51page();
					break;
				case 0x02 :
					Display51page();
					break;
			}
			break;
		case 0x10 :
			break;
    }
    HAL_UART_Receive_IT(&huart1, (uint8_t*)uart1_rx_data, 9);
}

void Display51page(){
	DisplayPageValue(0x51,0x10,DeviceInfo.peri1_speed);
	DisplayPageValue(0x51,0x20,DeviceInfo.peri2_speed);
	DisplayPageValue(0x51,0x30,DeviceInfo.fan_high_speed);
	DisplayPageValue(0x51,0x40,DeviceInfo.fan_low_speed);
	DisplayPageValue(0x51,0x50,DeviceInfo.lower_temperature*10);
	DisplayPageValue(0x51,0x60,DeviceInfo.upper_temperature*10);
	DisplayPageValue(0x51,0x70,DeviceInfo.overheat_temperature*10);
	DisplayPageValue(0x51,0x80,DeviceInfo.PreHeatTime);
	DisplayPageValue(0x51,0x90,DeviceInfo.SterileTime);
	DisplayPageValue(0x51,0xA0,DeviceInfo.NozzleCleanTime);
	DisplayPage(CHANGE_VALUE_PAGE);
}

void Display52page(){
	if(input_test_time==0){
		input_test_time=5*60*100;
	}
	DisplayPageValue(0x52,0x10,input_test_time/6000);
	DisplayPageValue(0x52,0x20,TestfInjectionPerMinute);
}
void Display53page(){
	if(input_test_time==0){
		input_test_time=5*60*100;
	}
	if(Testfanspeed<20||Testfanspeed>100){
		Testfanspeed=50;
	}
	DisplayPageValue(0x53,0x10,input_test_time/6000);
	DisplayPageValue(0x53,0x20,Testfanspeed);
}
void Display54page(){

}
void Display55page(){
	DisplayTime(0x55,0x10,input_test_time);
	DisplayPage(TEST_COMPLETE_PAGE);
}

void DisplayPageValue(int page ,int index, int value){
	unsigned char   PageValue[8] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00};
	PageValue[4] = page;
	PageValue[5] = index;
	PageValue[6] = value >> 8;
	PageValue[7] = value & 0xff;
    HAL_UART_Transmit(&huart1, PageValue, 8, 10);
}

//문자 출력
void DisplayPage4Char(int page ,int index, char *msg){
	unsigned char   PageChar[10] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	PageChar[2]=7; //주소1+주소2+Data	lenth
	PageChar[4]=page;
	PageChar[5]=index;
	PageChar[6]=msg[0];
	PageChar[7]=msg[1];
	PageChar[8]=msg[2];
	PageChar[9]=msg[3];
    HAL_UART_Transmit(&huart1, PageChar, 10, 10);
}

void DisplayPage8Char(int page ,int index, char *msg){
	unsigned char   PageChar[14] = {0x5A, 0xA5, 0x0b, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	PageChar[2]=11; //주소1+주소2+Data	lenth
	PageChar[4]=page;
	PageChar[5]=index;
	PageChar[6]=msg[0];
	PageChar[7]=msg[1];
	PageChar[8]=msg[2];
	PageChar[9]=msg[3];
	PageChar[10]=msg[4];
	PageChar[11]=msg[5];
	PageChar[12]=msg[6];
	PageChar[13]=msg[7];
    HAL_UART_Transmit(&huart1, PageChar, 14, 10);
}

void DisplayPage10Char(int page ,int index, char *msg){
	unsigned char   PageChar[16] = {0x5A, 0xA5, 0x0d, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	PageChar[2]=13; //주소1+주소2+Data	lenth
	PageChar[4]=page;
	PageChar[5]=index;
	PageChar[6]=msg[0];
	PageChar[7]=msg[1];
	PageChar[8]=msg[2];
	PageChar[9]=msg[3];
	PageChar[10]=msg[4];
	PageChar[11]=msg[5];
	PageChar[12]=msg[6];
	PageChar[13]=msg[7];
	PageChar[14]=msg[8];
	PageChar[15]=msg[9];
    HAL_UART_Transmit(&huart1, PageChar, 16, 10);
}
