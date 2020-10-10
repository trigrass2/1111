/*------------------------------------------------------------------------
 Module:       	yk.h
 Author:        
 Project:       
 State:			
 Creation Date:	2008-09-1
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/
#ifndef _YK_H
#define _YK_H

#include "syscfg.h"

extern DWORD g_DoState[2];              /*DO״̬*/
#define PRYK  0x80000000

#if ((DEV_SP != DEV_SP_DTU)&&(DEV_SP != DEV_SP_CTRL))
#define MAX_DOPOINT_NUM              3
#define MAX_YKPOINT_NUM              (MAX_DOPOINT_NUM*2)
#define MAX_RELAY_NUM                (MAX_DOPOINT_NUM*3)
#define MAX_YKPORT_NUM               (MAX_RELAY_NUM+3)
#elif (DEV_SP == DEV_SP_CTRL)
#define MAX_DOPOINT_NUM              10
#define MAX_YKPOINT_NUM              (MAX_DOPOINT_NUM*2)
#define MAX_RELAY_NUM                (MAX_DOPOINT_NUM*3)
#define MAX_YKPORT_NUM               (MAX_RELAY_NUM+3)
#else
#define MAX_DOPOINT_NUM              18
#define MAX_YKPOINT_NUM              (MAX_DOPOINT_NUM*2)
#define MAX_RELAY_NUM                (MAX_DOPOINT_NUM*3)
#define MAX_YKPORT_NUM               (MAX_RELAY_NUM+3)
#endif

typedef struct{	
	int id;
    int value;
	int timeout;
}VYkOutput;	

#ifdef INCLUDE_YK
void ykInit(void);
#endif

#define AD_YK_FLAG 0x03		

void ykScanOnTimer(void);

int ykSelect(int srcid, int id, int value);
int ykOperate(int srcid,int id, int value, int time);
int ykCancel(int srcid,int id, int value);
int ykOutput(int id, int value);
int ykGetYb(int begin, int num, BYTE *yb);
int ykSetSYb(int id, int value);

int prOutput(int id, int value);
int ykSelectGet(int id, int value);

#define prCancel(id, value) ykCancel(-1,id, value)

#endif 
