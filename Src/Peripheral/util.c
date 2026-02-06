/*
 * util.c
 *
 *  Created on: 2019. 5. 10.
 *      Author: PC
 */


union fc {
	float fValue;
	char  cValue[4];
};

int Calc_RunningTime(float Cubic, int perCubic, int perMinute)
{
	int iTime;
	iTime = ((Cubic * perCubic * 60) / perMinute);
	return iTime;
}

int Calc_Usage(int iTime, int perCubic, int perMinute)
{
	int iSize;
	iSize = (iTime * perMinute * 10) / perCubic / 60;
	return iSize;
}

unsigned char hex2bcd (unsigned char x)
{
    unsigned char y;
    y = (x / 10) << 4;
    y = y | (x % 10);
    return (y);
}

float char2float(const unsigned char *data)
{
	union fc	temp;
	for(int i = 0; i < 4; i++) {
		temp.cValue[i] = data[i];
	}
	return(temp.fValue);
}

void float2char(float fValue, char *data)
{
	union fc	temp;

	temp.fValue = fValue;
	for(int i = 0; i < 4; i++) {
		data[i] = temp.cValue[i];
	}
}


int RoundAtFirstDecimal(float value)
{
	if (value >= 0.0f) {
		return (int)(value + 0.5f);
	}

	return (int)(value - 0.5f);
}

float RoundToFirstDecimal(float value)
{
	if (value >= 0.0f) {
		return ((int)(value * 10.0f + 0.5f)) / 10.0f;
	}

	return ((int)(value * 10.0f - 0.5f)) / 10.0f;
}

void Char2Hex(unsigned char ch, char *pszHex)
{
 unsigned char byte[2];
 byte[0] = ch/16;
 byte[1] = ch%16;

 for (int i=0; i<2; i++)
 {
  if (byte[i] >= 0 && byte[i] <= 9)
	  if(byte[i]==9)
		  pszHex[i] = 'A';
	  else
		  pszHex[i] = '0' + byte[i]+1;
  else
   pszHex[i] = 'A' + byte[i] - 10+1;
  //pszHex[i] = 'A' + byte[i] - 10;
 }

 pszHex[2] = 0;
}

void CharStr2HexStr(unsigned char const* pszCharStr, char *pszHexStr, int nSize)
{
 char szHex[3];

 pszHexStr[0] = 0;

 for (int i=0; i<nSize; i++)
 {
  Char2Hex(pszCharStr[i], szHex);
  strcat(pszHexStr, szHex);
 }
}
