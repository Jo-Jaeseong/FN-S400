/*
 * FanPump.h
 *
 *  Created on: 2019. 5. 24.
 *      Author: monster
 */

#ifndef PERIPHERAL_FANPUMP_H_
#define PERIPHERAL_FANPUMP_H_

void InitFanPump(void);
void TurnOnFanPump(void);
void TurnOffFanPump(void);
void SpeedUpFan(int channel);
void SpeedDownFan(int channel);
void SetPanSpeed(int channel, int iPercent);
void SetFanPumpSpeedAllMin(void);
void SetFanPumpSpeedAllMin2(void);
void SetFanPumpSpeedAllMax(void);
void SetFanPumpSpeedAll(int percent);

void ReadFanControl();

void AdjustBlowerFanControl(int channel);
#endif /* PERIPHERAL_FANPUMP_H_ */
