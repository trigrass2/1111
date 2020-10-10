
/*------------------------------------------------------------------------
 Module:		nvram.c
 Author:		helen
 Project:		 
 Creation Date: 2014-10-14
 Description:	
------------------------------------------------------------------------*/

#include "syscfg.h"
#include "bsp.h"
#include "i2c.h"
#include "os.h"
#define MB85RC64_ID  0xA0

STATUS extNvRamGet(DWORD offset,BYTE *buf, int len)
{
   return(I2CReceBuffer_WAddr(0,buf, MB85RC64_ID, offset, len));
}

STATUS extNvRamSet(DWORD offset,BYTE *buf, int len)
{
   return(I2CSendBuffer_WAddr(0,buf, MB85RC64_ID, offset, len));
}

