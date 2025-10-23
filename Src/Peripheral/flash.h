/*
 * flash.h
 *
 *  Created on: 2019. 5. 31.
 *      Author: monster
 */

#ifndef PERIPHERAL_FLASH_H_
#define PERIPHERAL_FLASH_H_

void Write_Flash();
void Read_Flash();
void Reset_Setting_Flash();
void Reset_All_Flash();
void Write_LogData_Flash();
void Write_Shift_LogData_Flash();
void Read_LogData_Flash(int index);
#endif /* PERIPHERAL_FLASH_H_ */
