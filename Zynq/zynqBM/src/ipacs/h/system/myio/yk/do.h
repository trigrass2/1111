/*------------------------------------------------------------------------
 Module:       	do.h
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-10-10
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
------------------------------------------------------------------------*/

#ifndef _DO_H
#define _DO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "yk.h"
#include "para.h"

#define DO_TYPE_D      0
#define DO_TYPE_S      1

typedef struct{
    char type;           //ң������

	char c_point;        //˫���ͣ��ϼ̵���  �����ͣ�����̵���
	char t_point;        //˫���ͣ��̵ּ��� 
}VDo2Point;	

typedef struct{
    char type;           /*=0 ˫���� (�Ϸ�˫�̵���)
	                      =1 ������ (��һ���̵���)
                          ��������ʱontime��Ч
                       */ 
	DWORD hzontime;      /*ms*/
	DWORD fzontime;      /*ms*/
	DWORD yztime;      /*ms*/					  
	short ybno;        /*ѹ������:  -2��ѹ��
                                    -1��ѹ��
                              >=0Ӳѹ��ң�ź�
                       */ 
}VDoCfg;	

typedef struct{
	DWORD	addr;			/*�˿ڵ�ַ*/
	WORD	mask;			/*�˿�ʹ��λ*/
	WORD	init_bits;		/*�˿ڳ�ʼλ*/
	WORD	curr_bits;		/*�˿ڵ�ǰλ*/
	BYTE	write;			/*��д*/
}VYkPort;

typedef struct {
	BYTE	get;			/*1:�뷴У, 0:���跴У*/
	BYTE 	set_port;		/*���ö˿�*/
	BYTE 	get_port;		/*��У�˿�*/
	char    en_port;        /*ʹ�ܶ˿�*/
	WORD	set_bit;		/*����λ*/
	WORD	get_bit;		/*��Уλ*/
	WORD    en_bit;         /*ʹ��λ*/
	DWORD   en_group;       /*ʹ���鶨��*/

	int	    dtime;			/*�պ���ʱʱ��*/
	int     select;         /*Ԥ��֮����*/
	int     select_thid;    /*Ԥ�õĹ�Լ��*/
}VYkRelay;

typedef struct{
	char	self;			/*����̵�����*/			
	char	pair;			/*��Լ̵�����*/
	char	start;			/*�����̵�����*/
	char	state;			/*״̬: 2:ִ�� 1:Ԥ�� 0:����*/
}VYkPoint;

typedef struct{
	char *name;
	char *name_tab;
	int id;
}VDefTVYkCfg;	

typedef struct{
    DWORD ext_type;

	int donum;
	VDo2Point *do2point;

    int portnum;
	VYkPort *yk_port;

	int relaynum;
	VYkRelay *yk_relay;
	VYkPoint *yk_point;	
	
	int tvykcfgnum_do;
	VDefTVYkCfg *ptvykcfg_do;

	int tvykcfgnum_public;
	VDefTVYkCfg *ptvykcfg_public;
}VDefDoCfg;	

VDefDoCfg *GetDefDoCfg(DWORD type);
int GetDefPDoCfg(DWORD type, WORD num, struct VPDoCfg *pPCfg);
VDefTVYkCfg *GetDefTVYkCfg_Do(DWORD type, int *pnum);
VDefTVYkCfg *GetDefTVYkCfg_Public(DWORD type, int *pnum);
VDefTVYkCfg *GetDefTVYkCfg_Yb(DWORD type, int *pnum);

VDefDoCfg *GetDefExtDoCfg(DWORD type);
int GetDefExtPDoCfg(DWORD type, WORD num, struct VPDoCfg *pPCfg);
VDefTVYkCfg *GetDefExtTVYkCfg_Do(DWORD type, int *pnum);
VDefTVYkCfg *GetDefExtTVYkCfg_Public(DWORD type, int *pnum);

#ifdef __cplusplus
}
#endif

#endif

