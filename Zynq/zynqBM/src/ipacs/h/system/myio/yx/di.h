/*------------------------------------------------------------------------
 Module:       	di.h
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-10-10
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef _DI_H
#define _DI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "para.h"

#define DEFAULT_YX_DELAY_TIME    20   /*20ms*/
#define DEFAULT_PULSE_WIDTH      20   /*20ms*/

#define DITYPE_SYX     0
#define DITYPE_PULSE   1
#define DITYPE_DYX     2

#define DICFG_REV      0x01  /*ȡ��*/
#define DICFG_DIS      0x02  /*��Ч*/
#define DICFG_NSOE     0x04  
#define DICFG_NCOS     0x08  
#define DICFG_HW       0x10  /*˫����Ϊ��λ*/
#define DICFG_SYX      0x20  /*˫��ң�����÷������֣�1Ϊ���������*/

	
typedef struct{
    int type; 
	DWORD cfg;
	WORD  dtime;  /*ң�ŷ���ʱ���������(1ms)*/
	WORD  yxno;
	WORD  re_yxno;//˫ң�ŵ���ص�ţ���yxno��Ӧ
}VDiCfg;

typedef struct{
	char *name;
	char *name_tab;
	int flag;
}VDefTVYxCfg;	

typedef struct{
    DWORD ext_type;
	int portnum;
	int dinum;
	WORD *di_bitmap;
	void *yxno;

	int tvyxcfgnum_di;
	VDefTVYxCfg *ptvyxcfg_di;

	int tvyxcfgnum_public;
	VDefTVYxCfg *ptvyxcfg_public;

	int tvdyxcfgnum;
	VDefTVYxCfg *ptvdyxcfg;
}VDefDiCfg;	

VDefDiCfg *GetDefDiCfg(DWORD type);
int GetDefPDiCfg(DWORD type, WORD num, struct VPDiCfg *pPCfg);
VDefTVYxCfg *GetDefTVYxCfg_Di(DWORD type, int *pnum);
VDefTVYxCfg *GetDefTVYxCfg_Public(DWORD type, int *pnum);
VDefTVYxCfg *GetDefTVYxCfg_Comm(void);
VDefTVYxCfg *GetDefTVDYxCfg(DWORD type, int *pnum);

VDefDiCfg *GetDefExtDiCfg(DWORD type);
int GetDefExtPDiCfg(DWORD type, WORD num, struct VPDiCfg *pPCfg);
VDefTVYxCfg *GetDefExtTVYxCfg_Di(DWORD type, int *pnum);
VDefTVYxCfg *GetDefExtTVYxCfg_Public(DWORD type, int *pnum);
VDefTVYxCfg *GetDefExtTVYxCfg_Comm(void);
VDefTVYxCfg *GetDefExtTVDYxCfg(DWORD type, int *pnum);

#ifdef __cplusplus
}
#endif

#endif

