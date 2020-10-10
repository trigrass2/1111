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

extern int myCellId;
BOOL bFK = 0;  /*辅开*/

#define YK_FILTER_TM_UNIT        (1000000/TIMER_TICKS_PER_SECOND)

/*extern BOOL simFtuState;
extern BOOL simFtuSwitch;
extern int  simFtuYkOperate(WORD id, int value);*/

#if (DEV_SP == DEV_SP_DTU)
#define YK_INPUT_RET(port)           ((PhyBit_Read(BSP_RAM_YKFJ, pYkPort[port].addr)^0xFF) & pYkPort[port].mask)
#define YK_INPUT_SET(port)           (PhyBit_Read(BSP_RAM_YK, port) & pYkPort[port].mask)  
#define YK_OUTPUT_0(port, bits)      PhyBit_Set(BSP_RAM_YK, pYkPort[port].addr, 1)
#define YK_OUTPUT_1(port, bits)      PhyBit_Set(BSP_RAM_YK, pYkPort[port].addr, 0)
#define YK_OUTPUT_OK(value, w_value)  (((value == 0) && (w_value == 0)) || (value && w_value))

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

		if(pDefDoCfg->portnum < 1)
			continue;
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
#define YK_INPUT_RET(port)           (PhyBit_Read(BSP_ESRAM_YK, BSP_PORT_YKFJ) & pYkPort[port].mask)
#define YK_INPUT_SET(port)           YK_INPUT_RET(port)  
#define YK_OUTPUT_1(port, bits)  PhyBit_Set(BSP_ESRAM_YK, BSP_PORT_YK, (BYTE)bits)
#define YK_OUTPUT_0(port, bits)  PhyBit_Clear(BSP_ESRAM_YK, BSP_PORT_YK, (BYTE)bits)
#define YK_OUTPUT_OK(value, w_value)  (((value == 0) && (w_value == 0)) || (value && w_value))

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

	}	
	
	pDoCfg = g_Sys.MyCfg.pDo;
}

#endif
void relayEnable(int id)
{
    BYTE index;
	DWORD mask;
	int id1;
	if (id <= 0) return;    //must have

	index = id>>5;
	id1 = id&0x1F;

	mask = 1<<(id1-1);
	g_DoEnable[index] |= mask;

	if(pYkRelay[id].en_port == -1) return;

	YK_OUTPUT_1((BYTE)pYkRelay[id].en_port, pYkRelay[id].en_bit);
}

void relayDisable(int id)
{
    BYTE index;
	DWORD mask;

	if (id <= 0) return;   //must have

	index = id>>5;
	id = id&0x1F;

	mask = 1<<(id-1);
	g_DoEnable[index] &= ~mask;

	if(pYkRelay[id].en_port == -1) return;

	if ((g_DoEnable[index] & pYkRelay[id].en_group) == 0)
	{
		YK_OUTPUT_0((BYTE)pYkRelay[id].en_port, pYkRelay[id].en_bit);
	}	
}

/*------------------------------------------------------------------------
 Procedure:     relayOutput ID:1
 Purpose:       继电器输出
 Input:         id-继电器号, value-控制; ticks-置位时间
 Output:		
 Errors:
------------------------------------------------------------------------*/
int relayOutput(int id, int value, int ticks)
{
    BYTE index;
	WORD w_value; BOOL b_err;
	int set_port, get_port;
	WORD old_set_bits;
	DWORD mask, old_state;
	DWORD cnt;
	WORD do_id;
	//id  < 0   不存在
	//id  = 0   虚拟
	//id  > 0   真实存在

    if (id < 0) return 0;   //该继电器不存在,默认正确
	//if (id>YK_RELAY_NUM) return 1;

	if (id > 0)    //真实存在的继电器,非虚拟
	{
		relayEnable(id);

        index = id>>5;
		do_id = id & 0x1F;
	    mask = 1<<(do_id-1);

		set_port = pYkRelay[id].set_port;
		old_set_bits = YK_INPUT_SET(set_port)&pYkRelay[id].set_bit;	
		old_state = g_DoState[index];
		if(value == 1)
		{
			YK_OUTPUT_1(set_port, pYkRelay[id].set_bit);
			g_DoState[index] |= mask;
		}	
		else
		{
			YK_OUTPUT_0(set_port, pYkRelay[id].set_bit);
			g_DoState[index] &= ~mask;
		}	
	
		if((pYkRelay[id].get == 0) || (value == 0)) 
		{
			if(value == 1)	pYkRelay[id].dtime = ticks;
			else			
			{
				pYkRelay[id].dtime = 0;
				relayDisable(id);
			}	
			return 0;
		}
		else
			cnt = Get100usCnt();

		/*反校*/
		get_port = pYkRelay[id].get_port;

		b_err = 1;
		w_value = 0;
		while(b_err == 1)
		{
			w_value = YK_INPUT_RET(get_port) & pYkRelay[id].get_bit;

			if (YK_OUTPUT_OK(value, w_value))  b_err = 0;
			
			if ((Get100usCnt()-cnt) > 150) break;
		}	
		
		if(b_err)
		{
			g_DoState[index] = old_state;
			if (old_set_bits)
				YK_OUTPUT_1(set_port, pYkRelay[id].set_bit);
			else
				YK_OUTPUT_0(set_port, pYkRelay[id].set_bit);

			return 3;
		}	
	}	

	if(value == 1)	pYkRelay[id].dtime = ticks;
	else			
	{
		pYkRelay[id].dtime = 0;		
		relayDisable(id);
	}	

	return 0;
}

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
}

void ykScanOnTimer(void)
{
	FAST int i;
	//FAST int set_port;
    //WORD cur_bits;
    //FAST WORD set_bit;
	FAST VYkRelay *pRelay;
    //FAST VYkPort *pPort;

	pRelay = &pYkRelay[0];	

    if (pRelay == NULL) return;
	
	for(i=0; i<=YK_RELAY_NUM; i++)
	{
		if(pRelay->dtime > 0)
		{
			pRelay->dtime --;
			if(pRelay->dtime <= 0)
				relayOutput(i, 0, 0);
		}	

		/*if (i > 0)
		{
			//软保持继电器
			set_port = pRelay->set_port;
			pPort = pYkPort+set_port;
			cur_bits = YK_INPUT_SET(set_port);
			set_bit = pRelay->set_bit;
			if (cur_bits & set_bit)
			{
				relayEnable(i);
				YK_OUTPUT_1(set_port, set_bit);
			}	
		}*/	

		pRelay++;
	}		
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
#ifdef INCLUDE_YK
#ifdef _GUANGZHOU_TEST_VER_
	int i;
#endif
	int point, ret;
	int self, pair, start, yk_select_time; 
	int mask, ybno;
	//int port, bit, mask, ybno;
#endif
    int valuetemp = value & (~PRYK);
  
    if (id <= 0) return 1;


    if (id == myprResetId)
	{
	   //ResetProtect(0);        //总复归
	   return 0;
	}
	
	if (id == myprResetId+1)
	{
		return 0;
	}

#ifdef INCLUDE_FA_DIFF
   if (id == myprResetId+2)
   {
	   return 0;
   }
#endif


	//每回线虚遥控预置均正确
	if (id >= myfdYkId)
	{
		return 0;		
	}
#ifdef INCLUDE_FA_S
    if (FaYkSelect(id, valuetemp) == OK)
		return 0;
#endif

#ifdef INCLUDE_YK	
    id = id - 1;
	
	if (pDo2Point[id].type == 1)  return 0;
	if(id < g_Sys.MyCfg.wDONum)
	{
		ybno = pDoCfg[id].ybno;
	    if (ybno == -1)
	    {
			mask = 1<<id;
			if (((g_DoSYb&mask) == 0) && ((value & PRYK) == 0)) return 4;		
		}
		else if (ybno != -2)
		{		
			if (GetDiValue(ybno) == 0)  return 4;
		}
	}
	
	if (valuetemp == 1)
		point = pDo2Point[id].c_point;
	else if (valuetemp == 0)
		point = pDo2Point[id].t_point;
	else 
		return 4;

	if (point <= 0) return 1;

	self = pYkPoint[point].self;
	pair = pYkPoint[point].pair;
	start  = pYkPoint[point].start;

#ifdef _GUANGZHOU_TEST_VER_
	for(i=1; i<=YK_RELAY_NUM; i++)
	{
		if(pYkRelay[i].dtime != 0) return 2;	
	}
#endif

	/*本组/配对继电器正在执行, 错误返回*/
	if (pYkRelay[self].dtime != 0)
		return 2;

	if ((pair >= 0) && (pYkRelay[pair].dtime != 0))
		return 2;

    if (start < 0) return 0;   //该继电器不存在，默认正确

    /*先前有预置, 取消*/
	if(pYkRelay[start].dtime != 0)
	{
	    if ((srcid != -1) && (pYkRelay[start].select_thid != -1)&&(pYkRelay[start].select_thid != srcid))
			return 2;
		ret = relayOutput(start, 0, 0);
		if(ret != 0) return ret;
	}
	
	pYkRelay[start].select = self;
	pYkRelay[start].select_thid = srcid;

    yk_select_time = pDoCfg[id].yztime;
	
	if ((yk_select_time < SECTOTICK(5)*TICK2MSE) || (yk_select_time > SECTOTICK(120)*TICK2MSE))
		yk_select_time = SECTOTICK(30)*TICK2MSE;
	
	yk_select_time = (yk_select_time*1000+(YK_FILTER_TM_UNIT-1))/YK_FILTER_TM_UNIT;

	return relayOutput(start, 1, yk_select_time);
#else
	return 1;
#endif
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
	VFdCfg *pfdcfg;
	
#ifdef INCLUDE_YK
	int point;
	int self, pair, start; 
	int ret, yk_exe_time;
	int mask, ybno;
#endif
    int valuetemp = value & (~PRYK);

    if (id <= 0) return 1;

    if (id == myprResetId)
	{
#ifdef INCLUDE_PR
	   ResetProtect(0);        //总复归
#endif
	   return 0;
	}
	if (id == myprResetId+1)
	{
#if defined(INCLUDE_FA)||defined(INCLUDE_FA_SH)
        YkFaOn(valuetemp);
#endif
        return 0;
	}
#ifdef INCLUDE_FA_DIFF
   if (id == myprResetId+2)
   {
       YkFaDiffOn(valuetemp);
	   return 0;
   }
#endif
	//每回线虚遥控
	if (id >= myfdYkId)
	{
		fd = ykId2Fd(&id);
		if (fd < 0)  return 1;

		if (id == 0)
		{
#ifdef INCLUDE_PR
			ResetProtect(fd+1);    //该回线复归
#endif
			return 0;
		}
		else if (id == 1)
		{
		    pfdcfg = g_Sys.MyCfg.pFd + fd;
		
			if(valuetemp == 0x00) //  设置软压板
				valuetemp = 0x01;
			else if(valuetemp == 0x01)
				valuetemp = 0x81;
			
			//ykSetSYb(pfdcfg->kg_ykno, valuetemp);  //按回线实现软压板投退
			return 0;
		}
		else if (id == 2)
		{
			return 1;	
		}
	}
	else if (id > myprResetId)
		return 1;
	
#ifdef INCLUDE_FA_S
    if (FaYkOperate(id, valuetemp) == OK)
		return 0;
#endif
#ifdef INCLUDE_YK
	if (id == myCellId)
    {
#if((DEV_SP == DEV_SP_TTU)||(DEV_SP == DEV_SP_DTU_IU))
		Att7022eDd_Clear();
		return 0;
#else
    	id = id - 1;
		if (time == 0) time = SECTOTICK(2);
#endif
	}	
	else
	{
    	id = id-1;

	    ybno = pDoCfg[id].ybno;
	    if (ybno == -1)
	    {
			mask = 1<<id;
			if (((g_DoSYb&mask) == 0) && ((value & PRYK) == 0))  return 4;		
		}
		else if (ybno != -2)
		{		
			if (GetDiValue(ybno) == 0)  return 4;
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
	
	
	if (pDo2Point[id].type == DO_TYPE_S)
	{
		self = pYkPoint[(BYTE)pDo2Point[id].c_point].self;
		return(relayOutput(self, valuetemp, 0));
	}	

	if (valuetemp == 1)
		point = pDo2Point[id].c_point;
	else if (valuetemp == 0)
		point = pDo2Point[id].t_point;
	else 
		return 4;

	if (point <= 0) return 1;
	self = pYkPoint[point].self;
	pair = pYkPoint[point].pair;
	start  = pYkPoint[point].start;
		
	/*本组/配对继电器正在执行, 错误返回*/
	if (pYkRelay[self].dtime != 0)
		return 2;
	
	if ((pair >= 0) && (pYkRelay[pair].dtime != 0))
		return 2;

	if(pYkRelay[start].dtime != 0)
	{
	    if ((srcid != -1) && (pYkRelay[start].select_thid != -1)&&(pYkRelay[start].select_thid != srcid))
			return 2;
	}
	
    if (start >= 0)
	{
		if (pYkRelay[start].dtime == 0) return 1;
		if (pYkRelay[start].select != self) 
		{
			relayOutput(start, 0, 0);
			return 1;
		}	
    }	

	if(time == 0)   
	{
		 //yk_exe_time = pDoCfg[id].ontime;
		if(valuetemp == 0)
			yk_exe_time = pDoCfg[id].fzontime; 
		else if(valuetemp == 1)
			yk_exe_time = pDoCfg[id].hzontime; 
	}
	else		  yk_exe_time = time*TICK2MSE;	
		
	yk_exe_time = ((yk_exe_time+4)*1000+(YK_FILTER_TM_UNIT-1))/YK_FILTER_TM_UNIT;//硬件闭合有误差，软件补偿4ms。当设为4ms以下时不准
		
	ret = relayOutput(self, 1, yk_exe_time); 
	
	if (start >= 0)
	{	
		if(ret == 0) 
			pYkRelay[start].dtime = yk_exe_time;
		else 
			relayOutput(start, 0, 0);
	}	
	
	return ret;
#else
    return 1;
#endif
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
	int fd;
#ifdef INCLUDE_YK
	int point;
	int self, start;
#endif

    if (id <= 0) return 1;

    if (id == myprResetId)
		return 0;

	if ((id > YK_NUM) && (id != myCellId))
	{
		fd = ykId2Fd(&id);
		if (fd < 0)  return 1;

		if (id == 1)
			return 0;
		else
			return 1;			
	}

#ifdef INCLUDE_YK
	id = id - 1;
	
	if (pDo2Point[id].type == 1)  return 0;

	if (value == 1)
		point = pDo2Point[id].c_point;
	else if (value == 0)
		point = pDo2Point[id].t_point;
	else return 4;

	if (point <= 0) return 1;
	self = pYkPoint[point].self;
	start  = pYkPoint[point].start;
    
	if(pYkRelay[start].dtime != 0)
	{
	    if ((srcid != -1) && (pYkRelay[start].select_thid != -1)&&(pYkRelay[start].select_thid != srcid))
			return 2;
	}
	
	return (relayOutput(self, 0, 0)||relayOutput(start, 0, 0));
#else
	return 1;
#endif
}

int ykSelectGet(int id, int value)
{
    int start, point, get_port, w_value;

	int valuetemp = value & (~PRYK);

	if (valuetemp == 1)
		point = pDo2Point[id].c_point;
	else if (valuetemp == 0)
		point = pDo2Point[id].t_point;
    else
		return 0;
	
    start = pYkPoint[point].start;

	get_port = pYkRelay[start].get_port;

	w_value = YK_INPUT_RET(get_port) & pYkRelay[start].get_bit;

	return w_value;
			
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
    if (ret&&(ret != 2)) FaYkFjErr(id - 1);
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
#if(TYPE_IO == IO_TYPE_MASTER)
    struct VExtIoConfig *pCfg1;
    struct VExtIoConfig *pCfg2;
	VExtIoCmd *pCmd = (VExtIoCmd *)g_Sys.byIOCmdBuf;
	BYTE *pData = (BYTE *)(pCmd+1);
	WORD yknum,no1,no2;
	int ret;
#endif
#ifdef INCLUDE_YK
	int i, mask, ybno;
	BYTE *p = yb;
#endif
#if(TYPE_IO == IO_TYPE_MASTER)
    num = (num < g_Sys.MyCfg.wAllIoDONum) ? num : g_Sys.MyCfg.wAllIoDONum; 
    if (GetMyIoNo(2, begin, &no1) == ERROR)
 	{
 	    pCfg1 = GetExtIoNo(2, (WORD)begin, &no1);
		if (pCfg1 == NULL)
			return 0;
		if (num > pCfg1->wDONum)
			num = pCfg1->wDONum;
		smMTake(g_Sys.dwExtIoDataSem);
        pCmd->addr = pCfg1->pAddrList->ExtIoAddr.wAddr;
		pCmd->cmd = EXTIO_CMD_YKYB_READ;
		pCmd->len = 2;
		*pData    = no1-1;
		*(pData+1)= num;
		ret = ExtIoCmd(NULL, NULL, SECTOTICK(2));
		yknum = 0;
		if (ret == OK)
		{
		    yknum = *pData;
			memcpy(yb, pData+1, yknum);
		}
		smMGive(g_Sys.dwExtIoDataSem);
		return yknum;		
 	}
	else
	{
	    begin = no1;
		if (num > g_Sys.MyCfg.wDONum)
		   num = g_Sys.MyCfg.wDONum;
	}
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

void BMSetYkYb(DWORD yb)
{
	g_DoSYb = yb;
}

