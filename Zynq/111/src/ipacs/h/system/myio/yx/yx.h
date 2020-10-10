/*------------------------------------------------------------------------
 Module:       	yx.h
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-9-2
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef _YX_H
#define _YX_H

#include "clock.h"

#if(DEV_SP == DEV_SP_DTU)
#define MAX_YX_PORT_NUM          20
#define MAX_YX_NUM_PER_PORT      8
#else
#define MAX_YX_PORT_NUM          1
#define MAX_YX_NUM_PER_PORT      16
#endif

#define SOE_REC_BUF_SIZE         256
#define YX_FILTER_TM_UNIT        (1000000/TIMER_TICKS_PER_SECOND) //500us

typedef struct{
    WORD  flt_flag;
	WORD  cur_state;
	WORD  rev;

	DWORD addr;
	WORD mask;
}VYxPort;

typedef struct{
	int yxno;	
    int type; 
	DWORD cfg;
	DWORD dtime;              /*ң�ŷ���ʱ���������(�жϵ�λʱ��)*/
	int re_yxno;
	int value;
	DWORD cnt;	
	int dyxno;		  //�洢yxno��Ӧ��˫ң�ŵ�num
	//DWORD chg_tm;           /*��λʱ�������*/
	struct VCalClock chg_tm;
    
}VYxChn;

typedef struct{
	int pairno;               /*˫��ң�ŶԷ����*/ 
	VYxChn *pair;
}VDYxChn;	

typedef struct{	
    int type;
	
	VYxChn *pchn;

	BYTE value1;
	BYTE value2;
	//DWORD chg_tm;
	
	struct VCalClock chg_tm;
}VSOEREC;	

typedef struct{
	VSOEREC soe[SOE_REC_BUF_SIZE];
	WORD wp;
	WORD rp;
#if TYPE_IO == IO_TYPE_EXTIO
	WORD bus_rp;
#endif
}VSoeBuf;

typedef struct{
	VYxChn *pchn;
	
	int port;
	WORD bit;
}VYxNo2Port;	

extern WORD g_RawDiValue[];
extern VYxNo2Port g_YxNo2Port[];

void yxInit(void);
void yxRefresh(int dyx);
void yxWrite(void);
void yxFilter(void);

int GetMyDiValue(int no);

void WriteYxChnTime(void);

#if (TYPE_IO == IO_TYPE_EXTIO)
int diValueRead(int *len, WORD *buf);
int diSoeRead(int num, VExtIoDiSoe *buf);
void diSoeClear(WORD num);
int diMapRead(int no, int buflen, VExtIoDiMap *pMap, int *dyx);
#endif

#endif /*_YX_H*/
