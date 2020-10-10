/*------------------------------------------------------------------------
 Module:        ad.h
 Author:        solar
 Project:       
 Creation Date:	2008-09-16
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/
#ifndef _AD_H
#define _AD_H

#include "syscfg.h"
#include "yx.h"

#if(DEV_SP != DEV_SP_DTU)
#define MAX_FD_NUM             1
#define MAX_AI_NUM             8
#else
#define MAX_FD_NUM             18
#define MAX_AI_NUM             64
#endif

#define YC_PQ_RAW_SHIFT_NUM    10
#define YC_PQ_GAIN_SHIFT_NUM   10


#define AICHAN_NUMBER       MAX_AI_NUM


#define AITOTAL_NUMBER      (AICHAN_NUMBER+25)



extern int aiChnNum, adErr;	
extern BYTE g_wdg_ver;
extern int gycInit;

void adPhyInit(void);
void adPhyConfig(void *pGain);
void adGetData(void);
void adSample(int ms1);
void adInit(void *pGain);
void adInt(void);


#endif 

