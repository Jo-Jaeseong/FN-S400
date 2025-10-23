/*
 * LTE_modem.h
 *
 *  Created on: Jun 13, 2019
 *      Author: PC
 */

#ifndef PERIPHERAL_LTE_MODEM_H_
#define PERIPHERAL_LTE_MODEM_H_

void InitLTEModem(void);

void SendTestMessage(void);
void SendProcessMessage();
void SendEndMessage(void);
void SendCurrentPPM(int hour);
void SendFinishTime(unsigned char smsHour, unsigned char smsMinute);
void SendReserveTime(unsigned char smsHour, unsigned char smsMinute);
#endif /* PERIPHERAL_LTE_MODEM_H_ */
