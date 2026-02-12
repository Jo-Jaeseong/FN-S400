/*
 * USBProcess.c
 *
 *  Created on: 2019. 5. 31.
 *      Author: monster
 */

#include "main.h"
#include "fatfs.h"
#include "ff.h"
#include "usb_host.h"
#include "flash.h"
#include "lcd.h"
#include "Process.h"

extern FATFS USBHFatFS;           /* File system object for USB disk logical drive */
FIL MyFile;                   /* File object */
extern char USBHPath[4];          /* USB Host logical drive path */
extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;

extern struct data_format	g_data[1500];
extern int 					g_data_index;

extern struct log_format	startData;
//extern struct data_format	startData;

extern struct data_format	t_data;
extern int					t_data_index;
extern int firstlogcall;
extern struct data_format time_data;
extern struct data_format d_data;

int USBSECURITYonoff_Flag;
int USBConnect_Flag;

extern struct FLData		f_data[65];

void USB_Error_Handler(void)
{
  /* USER CODE BEGIN USB_Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
	//while(1)
	//{
	//}
//	char msg[80] = "USB drive is not ready. Use another drive.";
//	DisplayErrorMessage(msg);
  /* USER CODE END USB_Error_Handler */
}

void DownloadUSB(){
	FRESULT res;
	char ucFilename[28];
	memset(ucFilename, 0, 28);
	sprintf(ucFilename, "%.8s", "cleanbio");
	sprintf(ucFilename+8, "20%.2x-%.2x-%.2x %.2x_%.2x",
			startData.year[0], startData.month[0], startData.day[0], startData.hour[0], startData.minute[0]);
	if(USBSECURITYonoff_Flag==1){
		sprintf(ucFilename+24, "%.4s",".cbt");
	}
	else{
		sprintf(ucFilename+24, "%.4s",".csv");
	}

	if(t_data_index == -1) {
		char msg[80] = "There is no log. Try later.";
		DisplayErrorMessage(msg);
		return;
	}

	if(Appli_state != APPLICATION_READY) {
		USB_Error_Handler();
		return;
	}
	if(!hUsbHostFS.device.is_connected) {
		USB_Error_Handler();
		return;
	}

	/* Register the file system object to the FatFs module */
	if (f_mount(&USBHFatFS, (TCHAR const*) USBHPath, 0) != FR_OK)
	{
		/* FatFs Initialization Error */
		USB_Error_Handler();
	}
	else
	{
		/* Create and Open a new text file object with write access */
		if (f_open(&MyFile, ucFilename, FA_OPEN_APPEND | FA_WRITE) != FR_OK){
			/* 'STM32.TXT' file Open for write Error */
			USB_Error_Handler();
		}
		else{
			char buffer[80];
			char buffer1[80];
			/* Put data */
			if(USBSECURITYonoff_Flag==1){
				sprintf(buffer, "20%.2x-%.2x-%.2x,%.2x:%.2x:%.2x,",
						t_data.year, t_data.month, t_data.day,
						t_data.hour, t_data.minute, t_data.second);
				CharStr2HexStr(buffer,buffer1,20);
				f_puts(buffer1, &MyFile);
				sprintf(buffer, "%2d,", (int)t_data.temperature);
				CharStr2HexStr(buffer,buffer1,3);
				f_puts(buffer1, &MyFile);
				sprintf(buffer, "%3d,", (int)t_data.humidity);
				CharStr2HexStr(buffer,buffer1,4);
				f_puts(buffer1, &MyFile);
				sprintf(buffer, "%3d,", (int)t_data.density);
				CharStr2HexStr(buffer,buffer1,4);
				f_puts(buffer1, &MyFile);
				sprintf(buffer, "%lu", (long)t_data.volume * 1U);
				CharStr2HexStr(buffer,buffer1,4);
				f_puts(buffer1, &MyFile);
			}
			else{
				if(firstlogcall){
					f_puts("Date, Time, Board Temperature, Board Humidity, Density(ppm), Volume(cc)\n", &MyFile);
				}
				sprintf(buffer, "%.2x-%.2x-%.2x,%.2x:%.2x:%.2x,",
						t_data.year, t_data.month, t_data.day,
						t_data.hour, t_data.minute, t_data.second);
				f_puts(buffer, &MyFile);
				sprintf(buffer, "%d,", (int)t_data.temperature);
				f_puts(buffer, &MyFile);
				sprintf(buffer, "%d,", (int)t_data.humidity);
				f_puts(buffer, &MyFile);
				sprintf(buffer, "%d,", (int)t_data.density);
				f_puts(buffer, &MyFile);
				sprintf(buffer, "%lu\n", (long)t_data.volume * 1U);
				f_puts(buffer, &MyFile);
			}
			res = FR_OK;
			if (res != FR_OK){
				/* 'STM32.TXT' file Write or EOF Error */
				USB_Error_Handler();
			}
			else{
				/* Close the open text file */
				f_close(&MyFile);
				firstlogcall=0;
			}
		}
	}
}

void USBTEST(){
	FRESULT res;
	char ucFilename[28];
	memset(ucFilename, 0, 28);
	sprintf(ucFilename, "%.8s", "USBTEST_");
	sprintf(ucFilename+8, "20%.2x-%.2x-%.2x %.2x_%.2x",
			t_data.year, t_data.month, t_data.day, t_data.hour, t_data.minute);

	if(USBSECURITYonoff_Flag==1){
		sprintf(ucFilename+24, "%.4s",".cbt");
	}
	else{
		sprintf(ucFilename+24, "%.4s",".csv");
	}

	if(Appli_state != APPLICATION_READY){
		USB_Error_Handler();
		return;
	}
	if(!hUsbHostFS.device.is_connected){
		USB_Error_Handler();
		return;
	}

	/* Register the file system object to the FatFs module */
	if (f_mount(&USBHFatFS, (TCHAR const*) USBHPath, 0) != FR_OK)
	{
		/* FatFs Initialization Error */
		USB_Error_Handler();
	}
	else
	{
		/* Create and Open a new text file object with write access */
		if (f_open(&MyFile, ucFilename, FA_OPEN_APPEND | FA_WRITE) != FR_OK){
			/* 'STM32.TXT' file Open for write Error */
			USB_Error_Handler();
		}
		else{
			char buffer[80];
			char buffer1[80];
			/* Put data */
			if(USBSECURITYonoff_Flag==1){
				sprintf(buffer, "20%.2x-%.2x-%.2x,%.2x:%.2x:%.2x,",
						t_data.year, t_data.month, t_data.day,
						t_data.hour, t_data.minute, t_data.second);
				CharStr2HexStr(buffer,buffer1,20);
				f_puts(buffer1, &MyFile);
				sprintf(buffer, "%2d,", (int)t_data.temperature);
				CharStr2HexStr(buffer,buffer1,3);
				f_puts(buffer1, &MyFile);
				sprintf(buffer, "%3d,", (int)t_data.humidity);
				CharStr2HexStr(buffer,buffer1,4);
				f_puts(buffer1, &MyFile);
				sprintf(buffer, "%3d,", (int)t_data.density);
				CharStr2HexStr(buffer,buffer1,4);
				f_puts(buffer1, &MyFile);
				sprintf(buffer, "%lu", (long)t_data.volume * 1U);
				CharStr2HexStr(buffer,buffer1,4);
				f_puts(buffer1, &MyFile);
			}
			else{
				//if(firstlogcall){
				f_puts("Date, Time, Board Temperature, Board Humidity, Density(ppm), Volume(cc)\n", &MyFile);
				//}
				sprintf(buffer, "%.2x-%.2x-%.2x,%.2x:%.2x:%.2x,",
						t_data.year, t_data.month, t_data.day,
						t_data.hour, t_data.minute, t_data.second);
				f_puts(buffer, &MyFile);
				sprintf(buffer, "%d,", (int)t_data.temperature);
				f_puts(buffer, &MyFile);
				sprintf(buffer, "%d,", (int)t_data.humidity);
				f_puts(buffer, &MyFile);
				sprintf(buffer, "%d,", (int)t_data.density);
				f_puts(buffer, &MyFile);
				sprintf(buffer, "%lu\n", (long)t_data.volume * 1U);
				f_puts(buffer, &MyFile);
			}
			res = FR_OK;
			if (res != FR_OK){
				/* 'STM32.TXT' file Write or EOF Error */
				USB_Error_Handler();
			}
			else{
				/* Close the open text file */
				f_close(&MyFile);
				//firstlogcall=0;
			}
		}
	}
}
void DownloadUSB2(int index){
	if(startData.year[index]!=0){
		FRESULT res; /* FatFs function common result code */
		char ucFilename[26];
		memset(ucFilename, 0, 26);
		sprintf(ucFilename, "%.8s", "cleanbio");
		sprintf(ucFilename+8, "20%.2x-%.2x-%.2x %.2x_%.2x",
				//normal
				startData.year[index], startData.month[index], startData.day[index], startData.hour[index], startData.minute[index]);
		if(USBSECURITYonoff_Flag==1){
			sprintf(ucFilename+24, "%.4s",".cbt");
		}
		else{
			sprintf(ucFilename+24, "%.4s",".csv");
		}

		if(Appli_state != APPLICATION_READY) {
			USB_Error_Handler();
			return;
		}
		if(!hUsbHostFS.device.is_connected) {
			USB_Error_Handler();
			return;
		}

		/* Register the file system object to the FatFs module */
		if (f_mount(&USBHFatFS, (TCHAR const*) USBHPath, 0) != FR_OK)
		{
			/* FatFs Initialization Error */
			USB_Error_Handler();
		}
		else
		{
			/* Create and Open a new text file object with write access */
			//if (f_open(&MyFile, ucFilename, FA_OPEN_APPEND | FA_WRITE) != FR_OK)
			if (f_open(&MyFile, ucFilename, FA_WRITE | FA_CREATE_ALWAYS ) != FR_OK)
			{
				/* 'STM32.TXT' file Open for write Error */
				USB_Error_Handler();
			}
			else
			{
				char buffer[80];
				char buffer1[80];
				//startData.year[index], startData.month[index], startData.day[index], startData.hour[index], startData.minute[index]);

				/* Put data */
				if(USBSECURITYonoff_Flag==1){
					for(int i=1;i<65;i++){
						if(f_data[i].year!=0){
							sprintf(buffer, "20%.2x-%.2x-%.2x,%.2x:%.2x:00,",
									f_data[i].year, f_data[i].month, f_data[i].day, f_data[i].hour, f_data[i].minute);
							CharStr2HexStr(buffer,buffer1,20);
							f_puts(buffer1, &MyFile);
							sprintf(buffer, "%2d,", (int)f_data[i].temperature);
							CharStr2HexStr(buffer,buffer1,3);
							f_puts(buffer1, &MyFile);
							sprintf(buffer, "%3d,", (int)f_data[i].humidity);
							CharStr2HexStr(buffer,buffer1,4);
							f_puts(buffer1, &MyFile);
							sprintf(buffer, "%3d,", (int)f_data[i].density);
							CharStr2HexStr(buffer,buffer1,4);
							f_puts(buffer1, &MyFile);
							sprintf(buffer, "%lu", (long)f_data[i].volume * 1U);
							CharStr2HexStr(buffer,buffer1,4);
							f_puts(buffer1, &MyFile);
						}
					}
				}
				else{
					f_puts("Date, Time, Board Temperature, Board Humidity, Density(ppm), Volume(cc)\n", &MyFile);
					for(int i=1;i<65;i++){
						if(f_data[i].year!=0){
							sprintf(buffer, "%.2x-%.2x-%.2x,%.2x:%.2x,",
									f_data[i].year, f_data[i].month, f_data[i].day, f_data[i].hour, f_data[i].minute);
							f_puts(buffer, &MyFile);
							sprintf(buffer, "%d,", (int)f_data[i].temperature);
							f_puts(buffer, &MyFile);
							sprintf(buffer, "%d,", (int)f_data[i].humidity);
							f_puts(buffer, &MyFile);
							sprintf(buffer, "%d,", (int)f_data[i].density);
							f_puts(buffer, &MyFile);
							sprintf(buffer, "%lu\n", (long)f_data[i].volume * 1U);
							f_puts(buffer, &MyFile);
						}
					}
				}

				res = FR_OK;
				if (res != FR_OK)
				{
					/* 'STM32.TXT' file Write or EOF Error */
					USB_Error_Handler();
				}
				else
				{
					/* Close the open text file */
					f_close(&MyFile);
					firstlogcall=0;
				}
			}
		}
	}
}


