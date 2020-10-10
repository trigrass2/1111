/*------------------------------------------------------------------------
 Module:		aidc.h
 Author:		helen
 Project:		
 Creation Date: 
 Description:	
------------------------------------------------------------------------*/

#ifndef _AD7913_H
#define _AD7913_H

#include "syscfg.h"
#pragma pack(1)
typedef struct{
    WORD valid;
	WORD rsv;
	long a;  
	long b;
}VAiDcGain;	
#pragma pack()

#define   AD7913_IWV           0x00
#define   AD7913_V1WV          0x01
#define   AD7913_V2WV          0x02
#define   AD7913_ADC_CRC       0x04
#define   AD7913_CTRL_CRC      0x05
#define   AD7913_CNT_SNAPSHOT  0x07
#define   AD7913_CONFIG        0x08
#define   AD7913_STAUTS0       0x09
#define   AD7913_LOCK          0x0A
#define   AD7913_SYNC_SNAP     0x0B
#define   AD7913_COUNTER0      0x0C
#define   AD7913_COUNTER1      0x0D
#define   AD7913_EMI_CTRL      0x0E
#define   AD7913_STATUS1       0x0F
#define   AD7913_TEMPOS        0x18

#define   AD7913_READ          0x04
#define   AD7913_WRITE         0x00

long ad7913Get(int i);

DWORD ad7913Gain(int i, long *b);
#ifdef YC_GAIN_NEW
int ad7913GainValGet(VYcVal* pData);
int ad7913GainValSet(int num, VYcVal* pData);
int ad7913GainGet(VYcGain *pdata);
int ad7913GainSet(VYcGain *pdata);
#else
int ad7913GainSet(int flag);
#endif
void ad7913Init(void);

#endif
