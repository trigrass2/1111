/*------------------------------------------------------------------------
 Module:		tmp75.h
 Author:		
 Project:		
 Creation Date: 
 Description:	
------------------------------------------------------------------------*/

#ifndef _TMP75_H
#define _TMP75_H

#include "syscfg.h"
#include "bsp.h"
#include "os.h"
#include "i2c.h"
#define TMP75_Addr		0x90
#define TEMREGISTER     0x00
BOOL Tmp75Init(BYTE TYPE);
WORD Tmp75GetTemperature(int chn);
WORD TemConversion(WORD indata);

#endif

