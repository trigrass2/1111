/*------------------------------------------------------------------------
 Module:       	myio.h
 Author:        
 Project:       
 Creation Date:	2008-10-29
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/
#ifndef _MYIO_H
#define _MYIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syscfg.h"
#include "prLib.h"
	
#include "ai.h"
#ifdef INCLUDE_YC
#include "ft.h"
#include "yc.h"
#else
#include "ad.h"
#endif

#include "di.h"
#include "extyx.h"
#ifdef INCLUDE_YX
#include "yx.h"
#endif

#include "do.h"
#ifdef INCLUDE_YK
#include "yk.h"
#endif

#ifdef INCLUDE_EXT_YC
//#include "extyc.h"
#endif

#ifdef IRIGB_FLAG
//#include "Irigb.h"
#endif


typedef struct{
	DWORD dwMode;
	WORD wMonthBits;
	WORD wWeekBits;
	DWORD dwDayBits;
	WORD wHour;
	WORD wMin;
	DWORD dwTime;
	BOOL bEndTime;
}VCellCtrl;		

typedef struct{
	DWORD dwCfg;
	DWORD dwNum;
	WORD wID;
	WORD Udis;  //��ػ��ֵ
	VCellCtrl *pCtrl;
}VCellCfg;	

#pragma pack(1)
typedef struct{
	WORD wCt1;
	WORD wCt2;
	WORD wCt01;
	WORD wCt02;
	WORD wResv[12];
}VRunParaLineCfg;

typedef struct{
	WORD wPt1;
	WORD wPt2;
	DWORD  dwDyVal;
	WORD wDyT;
	DWORD  dwGyVal;
	WORD wGyT;
	DWORD  dwZzVal;
	WORD wZzT;
	DWORD  dwGzVal;
	WORD wGzT;
	WORD wYxfd;
	WORD wFzKeepT;
	WORD wHzKeepT;
	WORD wDchhT;                //��ػ����
	WORD wDchhTime;           //��ػʱ��
	WORD wResv[11];
	VRunParaLineCfg LineCfg[MAX_FD_NUM];
}VRunParaFileCfg;	
typedef struct{
	WORD wPt1;
	WORD wPt2;
	DWORD  dwDyVal;
	DWORD wDyT;
	DWORD  dwGyVal;
	DWORD wGyT;
	DWORD  dwZzVal;
	DWORD wZzT;
	DWORD  dwGzVal;
	DWORD wGzT;
	WORD wYxfd;
	WORD wFzKeepT;
	WORD wHzKeepT;
	WORD wDchhT;                //��ػ����
	WORD wDchhTime;           //��ػʱ��
	WORD wResv[11];
	VRunParaLineCfg LineCfg[MAX_FD_NUM];
}VRunParaCfg;	
#pragma pack()

typedef struct{
	struct VSysClock execlock;
	BOOL exestate;
	struct VCalClock oldtime;//��ʱ
}VCellState;

typedef struct{
	DWORD dwMode;       //0--��ֹ1--��ѹģʽ 2-����״̬ģʽ
	WORD wUFdNo1;       //�ɼ����ڵĵ�ѹ��� 
	WORD wUFdNo2;       //�ɼ����ڵĵ�ѹ��� 

	WORD wUFdYx1;	
	WORD wUFdYx2;

    int U1No[3];
	int U2No[3];

    int UL1;
	int UL2;

	int kgState1;
	int kgState2;

	DWORD chgMsCnt;
}VUSwitchCtrl;		

typedef struct {
	DWORD dwCfg;        //D0--��ػʹ��
	DWORD dwNum;        //��������

	VUSwitchCtrl *pCtrl;
}VUSwitchCfg;	

typedef struct {
	BYTE kgNo;
	BYTE optVal;
	BYTE yxNo;
	BYTE kgJudge;
	DWORD dwTimes;
	DWORD dwCnt;
}VKgStatus;

typedef struct {
	BOOL ykstart;
	BOOL ykUnlock;
	int ykcount;
}VKgUnlock;

typedef struct
{
	int UNo[3]; //��ʱֻ����a��b��c
	DWORD dwUk[3];
	int INo[3];
	DWORD dwIk[3];
}UICheckCtrl;
typedef struct
{
	TIMERELAY ddy;
	TIMERELAY gdy;
	TIMERELAY zz;
	TIMERELAY gz;
}UICheckTime;

void ioInit(void);
void dio(void);
void mea(void);

int GetDiValue(int no);
int GetMyIoNo(int type, WORD allio_no, WORD *no);

int cellOutput(int value);
int cellLocalCtrl(void);

void ykStsSet(int id, int optVal);
void ykErrReset(void);
void ledCheck(void);

#ifdef INCLUDE_YK_KEEP
void ykkeepInit(void);
#endif

void ykstateInit(void);

#ifdef __cplusplus
}
#endif

#endif 

