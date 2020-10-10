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

#define AD_FT_FLAG  0x06    //120点，每6次进一次        20点保护，1ms进一次保护ft计算
#define AD_1MS_FLAG  0x01    //goose tick


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


#define FT_SAM_POINT_N      20
#define MEA_SAM_POINT_N     (FT_SAM_POINT_N*2) 
#define YC_SAM_POINT_N      (MEA_SAM_POINT_N*5/4)
#define FREQ_SAM_POINT_N    (MEA_SAM_POINT_N*5)

#if(DEV_SP == DEV_SP_TTU)
#undef MEA_SAM_POINT_N
#define MEA_SAM_POINT_N 32
#endif

#define SAM_BUF_LEN         (256*256)   /*要求2幂*/  /*256*/


#define SAMFT_BUF_LEN          256

extern volatile short nMeaSam[SAM_BUF_LEN][AITOTAL_NUMBER];  //测量采样数据区
extern volatile WORD wMeaSamCnt; //测量采样指针

extern volatile short nFtSam[SAMFT_BUF_LEN][AITOTAL_NUMBER];  //FT采样数据区
extern volatile WORD wFtSamCnt; //FT采样指针

extern DWORD adMask;
extern int aiChnNum, adErr;	
extern BYTE g_wdg_ver;
extern int gycInit;

void adPhyInit(void);
void adPhyConfig(void *pGain);
void adGetData(void);
void adSample(int ms1);
void adInit(void *pGain);
void adInt(DWORD *nSamTmp);


#endif 

