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
    char type;           //遥控类型

	char c_point;        //双点型－合继电器  单点型－自身继电器
	char t_point;        //双点型－分继电器 
}VDo2Point;	

typedef struct{
    char type;           /*=0 双点型 (合分双继电器)
	                      =1 单点型 (仅一个继电器)
                          当单点型时ontime无效
                       */ 
	DWORD hzontime;      /*ms*/
	DWORD fzontime;      /*ms*/
	DWORD yztime;      /*ms*/					  
	short ybno;        /*压板配置:  -2无压板
                                    -1软压板
                              >=0硬压板遥信号
                       */ 
}VDoCfg;	

typedef struct{
	DWORD	addr;			/*端口地址*/
	WORD	mask;			/*端口使用位*/
	WORD	init_bits;		/*端口初始位*/
	WORD	curr_bits;		/*端口当前位*/
	BYTE	write;			/*可写*/
}VYkPort;

typedef struct {
	BYTE	get;			/*1:须反校, 0:不需反校*/
	BYTE 	set_port;		/*设置端口*/
	BYTE 	get_port;		/*反校端口*/
	char    en_port;        /*使能端口*/
	WORD	set_bit;		/*设置位*/
	WORD	get_bit;		/*反校位*/
	WORD    en_bit;         /*使能位*/
	DWORD   en_group;       /*使能组定义*/

	int	    dtime;			/*闭合延时时间*/
	int     select;         /*预置之对象*/
	int     select_thid;    /*预置的规约号*/
}VYkRelay;

typedef struct{
	char	self;			/*本身继电器号*/			
	char	pair;			/*配对继电器号*/
	char	start;			/*启动继电器号*/
	char	state;			/*状态: 2:执行 1:预置 0:空闲*/
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

