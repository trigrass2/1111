/*------------------------------------------------------------------------
 Module:        yc.h
 Author:        solar
 Project:       
 Creation Date:	2008-09-17
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/
#ifndef _YC_H
#define _YC_H

#include "syscfg.h"

#include "db.h"
#include "ai.h"
#include "ad.h"


#define MMI_MEASHOW_USER_FT

#ifdef _TEST_VER_ 
#define YC_MEAN_NUM      4  /*pow 2*/
#else
#define YC_MEAN_NUM      1  /*pow 2*/
#endif

#define YC_GAIN_AD7606_VER     0x80000000 //AD7606 采样芯片
#define YC_GAIN_AD7607_VER     0x80000001 //AD7607 采样芯片
#define YC_GAIN_ADE9078_VER    0x80000002 //ADE9078
#define YC_GAIN_ATT7022_VER    0x80000003 //att7022
#define YC_GAIN_ADS8578_VER    0x80000004 //ads8578
#define YC_GAIN_AD7616_VER     0x80000005 //ad7616

#define YC_GAIN_VALID_VER      0xfffffff0 //类别

typedef struct{
    float value;
	char *unit;
}VMeaYc;	

typedef struct{
	int id;
	char *name;
	char *unit;
}VMmiMeaTbl;	

typedef struct{
	long l0;
	long lRms;
	long lAng;
}VMeaValue;	

typedef struct{
	int valid;
	VMmiMeaTbl *tbl;
	float rms;
	float ang;
	//float xs;       
}VMmiMeaValue;	

typedef struct{
	VMmiMeaValue mmiValue[MMI_MEA_NUM];
}VFdMeaCal;	

typedef struct{
	int p_id;
	int q_id;
	int no[4];
}VMeaDd;	

#if 1
typedef struct{
    long r;
	long i;
}VFftVal;

typedef struct{
	VFftVal aiftVal[MAX_AI_NUM];
}VMmifft;
#endif

typedef struct{
    BYTE TempZone;
	BYTE ITempZone;
	BYTE SmlITZone;
	BYTE rec;
	short allgain;
	short norIgain;
	short smlIgain;
}VYcAttGain;

#define YC_GAIN_BIT_VALID          0x01
#pragma pack(1)
typedef struct{
    WORD ycNo;
	long a;   
	long b;
	BYTE fd;
	WORD type;
	BYTE status;
}VYcGain;	
#define YC_GAIN_VALID_GET(s)         (s&YC_GAIN_BIT_VALID)


#ifdef YC_GAIN_NEW
typedef struct{
    WORD  ycNo;
	WORD  type;
	long  value;
}VYcVal;
#endif
#pragma pack()
	
extern VYcGain *pMeaGain;
extern int diNUM, aiNUM, ycNum, fdNum, ddNum;
extern int gainValid, gycInit;
extern DWORD ggainver;
extern VAiCfg *pAiCfg;
extern VYcCfg *pYcCfg;
extern VFdCfg *pFdCfg;

#ifdef INCLUDE_DD
int meaRead_DbDdValue(int beginNo, int num, long *buf);
#endif

int meaRead_DbValue(int beginNo, int num, struct VYCF_L *buf);
int meaRead_Yc2(int beginNo, int num, VMeaYc *meaYc);
int meaRead_Yc1(int beginNo, int num, VMeaYc *meaYc);
int meaRead_Mmi(int fd, int beginNo, int num, VMmiMeaValue *mmiVal);
int meaRead_YcChnBuf(int ycno, BYTE *flag, int *wave_point_num, short *wave_buf, int buf_len, int ptr);
int meaRead_YcChnBuf1(int ycno, BYTE *flag, int *wave_point_num, short *wave_buf, int buf_len, int ptr);

int meaRead_FdChnBuf(int fd, BYTE *flag, int *wave_point_num, short *wave_buf, int buf_len);
int meaRead_FdChnBuf1(int fd, BYTE *flag, int *wave_point_num, short *wave_buf, int buf_len);


DWORD ReadYcPtCt(WORD no);

#ifdef YC_GAIN_NEW
int gainvalset(int flag, int num, BYTE* pdata);
int gainvalget(BYTE *pdata);
int gainget(BYTE *pgain);
int gainset(BYTE *pgain);
int gainzero(void);
int gainread(void);
int gainclear(void);
int gainsetadtype(DWORD adtype);
#else
int gainset(int flag);
int gainzero(void);
int gainread(void);
int gainclear(void);
int ddreset(void);
#endif


int meaGainAdjust(int fd, DWORD type, long a, long b, int add);
int ycGainAdjust(int fd, DWORD type, long a, long b, int add);
int meaGainAdjustSet(void);

#ifdef INCLUDE_YC
void ycInit(void);
void measure(void);
#endif

#endif 

