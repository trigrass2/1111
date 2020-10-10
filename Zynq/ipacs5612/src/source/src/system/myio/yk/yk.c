/*------------------------------------------------------------------------
 Module:       	yk.c
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-09-1
 Description:  
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
------------------------------------------------------------------------*/

#include "syscfg.h"
#include "sys.h"
static int YK_NUM = 0;
extern int myfdYkId;
#ifdef INCLUDE_ATT7022
#include "att7022e.h"
#endif
#ifdef INCLUDE_YK
#include "do.h"
#include "yk.h"

static int YK_RELAY_NUM;

DWORD g_DoState[2];              /*DO状态*/
static DWORD g_DoEnable[2];      /*DO使能*/
static DWORD g_DoSYb;
static VDoCfg *pDoCfg;

static VDo2Point pDo2Point[MAX_DOPOINT_NUM+1];
static VYkPort pYkPort[MAX_YKPORT_NUM];
static VYkRelay pYkRelay[MAX_RELAY_NUM+3];
static VYkPoint pYkPoint[MAX_YKPOINT_NUM+3];


#define YK_FILTER_TM_UNIT        (1000000/8000*(AD_YK_FLAG+1))

/*extern BOOL simFtuState;
extern BOOL simFtuSwitch;
extern int  simFtuYkOperate(WORD id, int value);*/

#if (DEV_SP == DEV_SP_DTU)
void DoCfgSet(void)
{
    int i,j;
	DWORD type,tmp;
	int portnum,pointnum;
	VDefDoCfg *pDefDoCfg;
	VYkPort *pPYkPort = pYkPort;
	VYkRelay *pPYkRelay = pYkRelay;
	VYkPoint *pPYkPoint = pYkPoint;
	VDo2Point *pPDo2Point = pDo2Point;

	YK_NUM = 0;
	YK_RELAY_NUM = 0;
	portnum = 0;
	pointnum = 0;

	memset(pPYkRelay, 0, sizeof(VYkRelay));
    pPYkRelay->select_thid = -1;
	pPYkRelay++;

	memset(pPYkPoint, 0, sizeof(VYkPoint));
	pPYkPoint++;
		
	for (i=0; i<BSP_IO_BOARD; i++)
	{
	    type = Get_Io_Type(i)&0xF;
		if (type == 0xf) continue;
		
	    pDefDoCfg = GetDefDoCfg(type);

		if (pDefDoCfg == NULL) continue;
		
        //YK ADDR
		for (j=0; j<(pDefDoCfg->portnum-1); j++)
		{
		    memcpy(pPYkPort, pDefDoCfg->yk_port+j, sizeof(VYkPort));
		    pPYkPort->addr = pDefDoCfg->yk_port[j].addr - i*18;
			pPYkPort++;
		}
		//YKFJ ADDR
		memcpy(pPYkPort, pDefDoCfg->yk_port+j, sizeof(VYkPort));
		pPYkPort->addr = pDefDoCfg->yk_port[j].addr - i*8;
		pPYkPort++;

		
		for (j=0; j<pDefDoCfg->relaynum; j++)
		{
		    memcpy(pPYkRelay, &(pDefDoCfg->yk_relay[j+1]), sizeof(VYkRelay));
			pPYkRelay->set_port = pDefDoCfg->yk_relay[j+1].set_port + portnum;
			pPYkRelay->get_port = pDefDoCfg->yk_relay[j+1].get_port + portnum;
			pPYkRelay++;
		}

		for (j=0; j<pDefDoCfg->donum; j++)
		{
		    pPYkPoint->self = pDefDoCfg->yk_point[2*j+1].self + YK_RELAY_NUM;
			pPYkPoint->pair = pDefDoCfg->yk_point[2*j+1].pair + YK_RELAY_NUM;
			pPYkPoint->start = pDefDoCfg->yk_point[2*j+1].start + YK_RELAY_NUM;
			pPYkPoint->state = 0;
   			pPYkPoint++;
			
			pPYkPoint->self = pDefDoCfg->yk_point[2*j+2].self + YK_RELAY_NUM;
			pPYkPoint->pair = pDefDoCfg->yk_point[2*j+2].pair + YK_RELAY_NUM;
			pPYkPoint->start = pDefDoCfg->yk_point[2*j+2].start + YK_RELAY_NUM;
			pPYkPoint->state = 0;
			pPYkPoint++;

			pPDo2Point->type = pDefDoCfg->do2point[j].type;
			pPDo2Point->c_point = pDefDoCfg->do2point[j].c_point + pointnum;
			pPDo2Point->t_point = pDefDoCfg->do2point[j].t_point + pointnum;
            pPDo2Point++;
			
		}
		YK_NUM += pDefDoCfg->donum;
		YK_RELAY_NUM += pDefDoCfg->relaynum;
		portnum += pDefDoCfg->portnum;
		pointnum +=  pDefDoCfg->donum*2;

		
	}
	pPYkPort->addr = BSP_PORT_YKCELL;
	pPYkPort->mask = 0x01;
	pPYkPort->init_bits = 0;
	pPYkPort->curr_bits = 0;
	pPYkPort->write = 1;
	pPYkPort++;
	pPYkPort->addr = BSP_PORT_YKCELL+1;
	pPYkPort->mask = 0x01;
	pPYkPort->init_bits = 0;
	pPYkPort->curr_bits = 0;
	pPYkPort->write = 1;

    memset(pPYkRelay, 0, sizeof(VYkRelay));
	pPYkRelay->en_port = -1;
	pPYkRelay->set_port = portnum;
	pPYkRelay->set_bit = 0x01;
	pPYkRelay++;
	memset(pPYkRelay, 0, sizeof(VYkRelay));
	pPYkRelay->en_port = -1;
	pPYkRelay->set_port = portnum+1;
	pPYkRelay->set_bit = 0x01;
	pPYkRelay++;
	YK_RELAY_NUM+=2;

	memset(pPYkPoint, 0, sizeof(VYkPoint));
	pPYkPoint->self = YK_RELAY_NUM-1;
	pPYkPoint->pair = YK_RELAY_NUM;
	pPYkPoint++;
	memset(pPYkPoint, 0, sizeof(VYkPoint));
	pPYkPoint->self = YK_RELAY_NUM;
	pPYkPoint->pair = YK_RELAY_NUM-1;
	pointnum+=2;

	memset(pPDo2Point, 0, sizeof(VDo2Point));
	pPDo2Point->c_point = pointnum;
	pPDo2Point->t_point = pointnum-1;

	extNvRamGet(NVRAM_AD_DOYB, (BYTE *)&tmp, sizeof(DWORD));
	type = tmp;
	extNvRamGet(NVRAM_AD_DOYB+4, (BYTE *)&tmp, sizeof(DWORD));
	g_DoSYb = tmp&0x3FFFF;
	
	if (type != g_Sys.MyCfg.dwIOType)
	{
	    tmp = g_Sys.MyCfg.dwIOType;
	    extNvRamSet(NVRAM_AD_DOYB, (BYTE *)&tmp, sizeof(DWORD));
		g_DoSYb = 0x3FFFF;		
		extNvRamSet(NVRAM_AD_DOYB+4, (BYTE *)&g_DoSYb, sizeof(DWORD));
	}
	pDoCfg = g_Sys.MyCfg.pDo;
}
#else
void DoCfgSet(void)
{
    DWORD tmp,type;
	WORD pointnum;
	int dio_type;
  	int i;
	
	VDefDoCfg *pDefDoCfg;
	dio_type = Get_Io_Type(0);

	pDefDoCfg = GetDefDoCfg(Get_Io_Type(0));
	if (pDefDoCfg == NULL)
	{
		YK_NUM = 0;
		YK_RELAY_NUM = 0;
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "未知数字板卡型号");			
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Can not find DIO_type %x cfg at ROM!", dio_type);
	}
	else
	{
		YK_NUM = pDefDoCfg->donum;
		YK_RELAY_NUM = pDefDoCfg->relaynum;
	
		
		memcpy(pDo2Point, pDefDoCfg->do2point, (pDefDoCfg->donum+1)*sizeof(VDo2Point));
		memcpy(pYkPort, pDefDoCfg->yk_port, pDefDoCfg->portnum*sizeof(VYkPort));
		memcpy(pYkRelay, pDefDoCfg->yk_relay, (pDefDoCfg->relaynum+1)*sizeof(VYkRelay));
		
		pointnum = 0;
		for (i=0; i<(YK_RELAY_NUM+1); i++)
	    {
	    	 if (pYkRelay[i].get)
	    	    continue;
	    	 pointnum++;
    	}
		memcpy(pYkPoint, pDefDoCfg->yk_point, (pointnum*sizeof(VYkPoint)));

		extNvRamGet(NVRAM_AD_DOYB, (BYTE *)&tmp, sizeof(DWORD));
	  type = tmp;
		extNvRamGet(NVRAM_AD_DOYB + 4, (BYTE *)&tmp, sizeof(DWORD));	
		g_DoSYb = tmp&0x3FFFF;
		
		if (type != dio_type)
		{
			tmp = dio_type;
			extNvRamSet(NVRAM_AD_DOYB, (BYTE *)&tmp, sizeof(DWORD));
			g_DoSYb = 0x3FFFF;
			extNvRamSet(NVRAM_AD_DOYB + 4, (BYTE*)&g_DoSYb,sizeof(DWORD));
		}
	}	
	
	pDoCfg = g_Sys.MyCfg.pDo;
}
#endif

/*------------------------------------------------------------------------
 Procedure:     ykInit ID:1
 Purpose:       遥控初始化, 自检
 Input:          				
 Output:		
 Errors:
------------------------------------------------------------------------*/
void ykInit(void)
{
	g_DoState[0] = g_DoState[1] = 0;
	g_DoEnable[0] = g_DoEnable[1] = 0;

    DoCfgSet();		
	//初始化设置裸核压板
	BMSetYkYb(g_DoSYb);
}
#endif 

extern int myCellId;
extern int myprResetId;

int ykId2Fd(int *pid)
{
    int fd, id;

	id = *pid-g_Sys.MyCfg.YKIoNo[0].wNum-g_Sys.MyCfg.YKIoNo[1].wNum;
	if (id <= 0) return -1;

	id = id - 1;
    *pid = id%3;
	fd = id/3;

	return fd;
}

int ykSelect(int srcid, int id, int value)
{
   return (yk_BM(0,srcid,id,value,0));
}

/*------------------------------------------------------------------------
 Procedure:     ykOperate ID:1
 Purpose:       遥控执行
 Input:         time以tick为单位, 若为0,使用缺省值.
 Output:		
 Errors:
------------------------------------------------------------------------*/
int ykOperate(int srcid,int id, int value, int time)
{
	int fd;
	int valuetemp = value & (~PRYK);
	VFdCfg *pfdcfg;
	
	if (id == myprResetId)
	{
	   ResetProtect(0);        //总复归
	   return 0;
	}
	//每回线虚遥控
	if (id >= myfdYkId)
	{
		fd = ykId2Fd(&id);
		if (fd < 0)  return 1;

		if (id == 0)
		{
			ResetProtect(fd+1);    //该回线复归
			return 0;
		}
		else if (id == 1)
		{
		    pfdcfg = g_Sys.MyCfg.pFd + fd;
		
			if(valuetemp == 0x00) //  设置软压板
				valuetemp = 0x01;
			else if(valuetemp == 0x01)
				valuetemp = 0x81;
			
			ykSetSYb(pfdcfg->kg_ykno, valuetemp);  //按回线实现软压板投退
			return 0;
		}
		else if (id == 2)
		{
			return 1;	
		}
	}
	
#ifdef	INCLUDE_YK_PR_DIS
		pfdcfg = g_Sys.MyCfg.pFd;
		for (fd=0; fd<g_Sys.MyCfg.wFDNum; fd++)
		{
			if (pfdcfg->kg_ykno == (id+1))
			{
				if ((valuetemp== 0) && (pfdcfg->yk_trip_dis)) return 4; 
				if (valuetemp && (pfdcfg->yk_close_dis)) return 4;
	
				break;
			}
	
			pfdcfg++;
		}
#endif

	ykStsSet(fd, valuetemp);
	
	return(yk_BM(1,srcid,id,value,time));
}

/*------------------------------------------------------------------------
 Procedure:     ykCancel ID:1
 Purpose:       遥控取消
 Input:          				
 Output:		
 Errors:
------------------------------------------------------------------------*/
int ykCancel(int srcid,int id, int value)
{
	return(yk_BM(2,srcid,id,value,0));		
}

/*------------------------------------------------------------------------
 Procedure:     ykOutput ID:1
 Purpose:       
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
int ykOutput(int id, int value)
{
	//!!未考虑远方本地
	//远方本地仅对远程控制有效,保护及当地命令行操作无效

#ifdef INCLUDE_YK
    int ret = 0;
#ifdef INCLUDE_FA_S
    int valuetemp = value & (~PRYK);
#endif

	ret = ykSelect(-1, id, value);
#ifdef INCLUDE_FA
    if (ret) FaYkFjErr(id - 1);
#endif
	if (ret == 0)  ret = ykOperate(-1,id, value, 0);
#ifdef INCLUDE_FA
    if (ret) FaYkErr(id -1);
#endif

	return ret;
#else 
#ifdef INCLUDE_FA_S //hll debug
    int ret = 0;

	ret = FaYkSelect(id, valuetemp);;
	if (ret == 0)  ret = FaYkOperate(id, valuetemp);	

    if (ret == OK)
		return 0;
	else
		return 1;
#else
	return 1;
#endif
#endif
}

//仅用于保护，长久闭合，由保护逻辑图撤销闭合
int prOutput(int id, int value)
{
#ifdef INCLUDE_YK
	int ret;

	ret = ykSelect(-1, id, value);
	if (ret == 0)  ret = ykOperate(-1,id, value, -1);	

	return ret;
#else
	return 1;
#endif
}

/*按遥控序号排列的压板状态区,每字节代表一个压板状态
  0-无效
  0x81-压板投入
  0x01-压板退出

  返回实际压板数
*/
int ykGetYb(int begin, int num, BYTE *yb)
{
#ifdef INCLUDE_YK
	int i, mask, ybno;
	BYTE *p = yb;
#endif
#ifdef INCLUDE_YK
    if ((YK_NUM <= 0)||(begin > YK_NUM))
		return 0;
	
    begin = begin - 1;
	if ((begin + num) > YK_NUM)
		num = YK_NUM - begin;

	for (i=begin; i<(begin+num); i++)
	{
		ybno = pDoCfg[i].ybno;			
		if (ybno == -1)
		{
			mask = 1<<i;
			if (g_DoSYb&mask)
				*p = 0x81;
			else
				*p = 0x01;				
		}
		else if (ybno == -2)
		{		
			*p = 0;
		}
		else
		{
			if (GetDiValue(ybno))
				*p = 0x81;
			else
				*p = 0x01;
		}
		
		p++;
	}

	return (num);
#else
 	return 0;
#endif
}	

/*设置遥控软压板
  value == 0x01 退出
        == 0x81 投入
*/
int ykSetSYb(int id, int value)
{
#ifdef INCLUDE_YK
    DWORD mask;
#endif
#ifdef INCLUDE_YK
 	if(id>YK_NUM || id==0) return ERROR;
	if (pDoCfg[id-1].ybno != -1) return ERROR;

	mask = 1<<(id-1);
	if (value == 0x01)
		g_DoSYb &= (~mask);
	else
		g_DoSYb |= mask;

	extNvRamSet(NVRAM_AD_DOYB+sizeof(DWORD), (BYTE *)&g_DoSYb, sizeof(DWORD));

	//设置压板给裸核
	BMSetYkYb(g_DoSYb);
	
	return OK;
#else
	return ERROR;
#endif
}

