/*------------------------------------------------------------------------
 Module:       	extyx.h
 Author:        solar
 Description:   
------------------------------------------------------------------------*/
#ifndef _EXTYX_H
#define _EXTYX_H

#include "clock.h"

typedef struct{
	int no;	
	DWORD cnt;				
	struct VCalClock chg_tm;    
}VExtYxChn;

extern 	BYTE *pExtYx;

void extYxFilter(void);
void extYxRefresh(void);
BYTE GetMyExtDiValue(int no);
int WriteMyMyExtDiValue(WORD value);
#ifdef INCLUDE_EXT_YX
void extYxInit(void);
#endif

#endif
