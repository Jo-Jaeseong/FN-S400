/*
 * util.h
 *
 *  Created on: 2019. 5. 10.
 *      Author: PC
 */

#ifndef UTIL_H_
#define UTIL_H_

int Calc_RunningTime(float Cubic, int perCubic, int perMinute);
int Calc_Usage(int iTime, int perCubic, int perMinute);

unsigned char hex2bcd (unsigned char x);
float char2float(const unsigned char *data);
void float2char(float fValue, unsigned char *data);
void Char2Hex(unsigned char ch, char *pszHex);
void CharStr2HexStr(unsigned char const* pszCharStr, char *pszHexStr, int nSize);

#endif /* UTIL_H_ */
