
/*
 * lcd.h
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#ifndef PERIPHERAL_LCD_H_
#define PERIPHERAL_LCD_H_

void InitLCD(void);
void LCD_Process(void);
void LCD_Function_Process(int index, int value);
void DoActionButton(int key);
void LoginFuntionButton(int key);
void StartButtonProcess(void);

void LCD_SetValues(int index, int value);

void DisplayOperationPage(void);
void DisplayRunningPage(void);



void InitDisplayValues(void);
void DisplayStatus();

void InitLCDTimes();
void DisplayTime(int index, unsigned int icentisecond);
void DisplayPreHeatTime();
void DisplaySprayTime();
void DisplaySterileTime();
void DisplayScrubTime();
void DisplayFinishTime();
void DisplayTotalTime();

void DisplayUserValue(int index, float value);

void DisplayValue(int index, float value);
void DisplayCubic();
void DisplayBoardTemperature();
void DisplayModuleTemperature();
void DisplayHuminity();
void DisplayInjectionPerMinute();
void DisplayInjectionPerCubic();
void DisplayDensity();
void DisplayH2O2Volume();

void DisplayModeIcon(int mode);
void DisplayIcon(int index, int value);
void DisplayIcon2(int index, int value);

void DisplayPreHeatIcon(int value);
void DisplaySprayIcon(int value);
void DisplaySterileIcon(int value);
void DisplayScrubIcon(int value);
void DisplayFinishIcon(int value);
void DisplayStartIcon(int value);
void DisplaySMSonoffIcon(int value);
void DisplayUSBSecurityonoffIcon(int value);
void DisplayServerSMSonoffIcon(int value);
void DisplayUSBIcon(int value);
void DisplayH2O2SensorIcon(int value);
void DisplayACCOUNTActiveIcon();
void DisplayLOGINOnoffIcon();
void DisplayRESERVEATOINOnoffIcon();


void DisplayScrubPopUp(void);
void DisplayVersion(char ch1, char ch2, char ch3);
void DisplayErrorMessage(char *msg);
void DisplayMsg(char *msg);

void DisplayTestValue(void);
void DisplayDebug(char ch1, char ch2);

void ReadRTC(unsigned char *year, unsigned char *month, unsigned char *day, unsigned char *week,
				unsigned char *hour, unsigned char *minute, unsigned char *second);
void SetRTCFromLCD();
void RFIDCheck(char *RFIDvalue, char *FLASHValue);

void DisplayUserNumberblank();

void DisplayFirstPage();
void DisplayPage(int page);


void DisplaySerialNumber();
void DisplayModemNumber();
void DisplayUserNumber();


//LOGIN
void LCD_Password(int index, int PW);
void PasswordCheck();
void DisplayLoginStatus();
void LoginProcess();

//Setting
void SettingButton(int key);
void DisplaySettingPage(void);

void DisplayLogListPage();
void DisplayDatelog(int index);
void DisplayStartTimelog(int index);
void DisplayEndTimelog(int index);
void DisplayCubiclog(int index);
void Displaystatuslog(int index);
void DisplayIDlog(int index);

void DisplayH2O2ListPage();
void DisplayRFIDIDlog(int index);
void DisplayRFIDVolumelog(int index);
void DisplayRFIDDatelog(int index);


//Account
void DisplayAccountSettingPage();
void DisplayAccountIDlog(int index);
void DisplayAccountPWlog(int index);
void DisplayAccountLastLoginlog(int index);
void DisplayAccountAttemptslog(int index);
void DisplayAccountStatuslog(int index);

//Maintenance
void MaintenanceButton(int key);
void DisplayTestTime();
void DisplayTestComplete();

//Developer
void DeveloperButton(int key);
void DisplaySerialNumberblank();
void DisplayHardwareVersion();
void DisplayDeveloperPage();
void DisplayPeri1_15_Value();
void DisplayTempPeri1_15_Value();
void DisplayPeri2_15_Value();
void DisplayTempPeri2_15_Value();

void DisplayPeri1_12_Value();
void DisplayTempPeri1_12_Value();
void DisplayPeri2_12_Value();
void DisplayTempPeri2_12_Value();

void DisplayPeri1_9_Value();
void DisplayTempPeri1_9_Value();
void DisplayPeri2_9_Value();
void DisplayTempPeri2_9_Value();

//2023.06.01 reservation
void DisplayReservePopUp(char *msg);
void DisplayReserveSettings();
void DisplayReserveTime();
void DisplayReserveTimePage();

//2023.06.01 USB SAVE
void DisplayPageValue(int page ,int index, int value);
void DisplaySelectIcon(int page, int index, int value);
void LCD_31(int index, int value);
void Display31page();

#endif /* PERIPHERAL_LCD_H_ */
