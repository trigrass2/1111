/*------------------------------------------------------------------------
 Module:       	yx.c
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-9-2
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef INCLUDE_YX
#include "di.h"
#include "yx.h"
#include "sys.h"

static int gYxInit = 0;

static WORD g_DiValue[MAX_YX_PORT_NUM];
WORD g_RawDiValue[MAX_YX_PORT_NUM];
VYxNo2Port g_YxNo2Port[MAX_YX_PORT_NUM*MAX_YX_NUM_PER_PORT];

static WORD pDiBitMap[MAX_YX_PORT_NUM];
static VDiCfg *pDiCfg;
static VYxChn pYxChn[MAX_YX_PORT_NUM][MAX_YX_NUM_PER_PORT];
static VDYxChn pDYxChn[MAX_YX_PORT_NUM][MAX_YX_NUM_PER_PORT];
static VYxPort pYxPort[MAX_YX_PORT_NUM];			
static VSoeBuf gSoebuf;		

static int YX_PORT_NUM;
int YX_NUM, DYX_NUM;


#if (DEV_SP == DEV_SP_FTU)
#define YX_NUM_PER_PORT     9
#endif
#if (DEV_SP == DEV_SP_CTRL)
#define YX_NUM_PER_PORT     10
#endif

#if (DEV_SP == DEV_SP_WDG)
#define YX_NUM_PER_PORT     (5 + 4*(g_wdg_ver!=0))
#endif

#if (DEV_SP == DEV_SP_TTU)
#define YX_NUM_PER_PORT     8
#endif

#if (DEV_SP == DEV_SP_DTU_PU)
#define YX_NUM_PER_PORT     6
#endif

#if (DEV_SP == DEV_SP_DTU_IU)
#define YX_NUM_PER_PORT     8
#endif

#if(DEV_SP != DEV_SP_DTU)
#define diCfgAddrSet()
#else
#define YX_NUM_PER_PORT     8
void diCfgAddrSet(void)
{
    int i,j,k;
	BYTE type;
	VDefDiCfg *pDefDiCfg;

    k = 0;
	for (i=0; i<BSP_IO_BOARD; i++)
	{
	     type = Get_Io_Type(i);
		 if (type == 0xFF) continue;

		 pDefDiCfg = GetDefDiCfg(type);
		 if (pDefDiCfg == NULL) continue;
		 if ((k + pDefDiCfg->portnum) > MAX_YX_PORT_NUM) break;
		 for (j=0; j<pDefDiCfg->portnum; j++)
	     {
	         pYxPort[k].addr = (BSP_ADDR_IO1_READ-i*8+j);  //暂定
			 k++;
		 }
	}
}
#endif


void yxInit(void)
{
    int i,type;
	VDefDiCfg *pDefDiCfg;

	gYxInit = 0;

	memset(g_DiValue, 0, sizeof(g_DiValue));
	memset(g_YxNo2Port, 0, sizeof(g_YxNo2Port));
	memset(pYxChn, 0, sizeof(pYxChn));
	memset(pDYxChn, 0, sizeof(pDYxChn));
	memset(pYxPort, 0, sizeof(pYxPort));
	memset(pDiBitMap, 0, sizeof(pDiBitMap));
	memset(&gSoebuf, 0, sizeof(VSoeBuf));

	
	for (i=0; i<BSP_IO_BOARD; i++)
	{
	     type = Get_Io_Type(i);
		 if (type == 0xFF) continue;
		 
	     pDefDiCfg = GetDefDiCfg(type);
		 if (pDefDiCfg)
		 {
		     if ((YX_PORT_NUM + pDefDiCfg->portnum) > MAX_YX_PORT_NUM) break;
		     memcpy((BYTE*)(pDiBitMap+YX_PORT_NUM),  (BYTE*)(pDefDiCfg->di_bitmap), pDefDiCfg->portnum*2);
			 YX_NUM += pDefDiCfg->dinum;
			 YX_PORT_NUM += pDefDiCfg->portnum;
		 }
	}	

	pDiCfg = g_Sys.MyCfg.pDi;
		
	diCfgAddrSet();

	gYxInit = 1;

}

//获取BM共享数据，及遥信数据，及soe、cos
void yxWrite(void)
{
	WORD i;
	int ret = -1;
	WORD no,value;
	struct VCalClock tm;
	
	struct VDBDSOE DBDSOE;
	struct VDBDCOS DBDCOS;
	struct VDBSOE DBSOE;
	struct VDBCOS DBCOS;
		
	struct VDBDYX DBDYX;
	struct VDBYX DBYX;

	static DWORD count = 0;

	if(((count++) & 63) == 0) //64次进一次
	{
		for(i = 0; i < g_Sys.MyCfg.wSYXNum; i++)
		{
			DBYX.wNo = i;
			if(ReadBMSYX(BM_EQP_MYIO, DBYX.wNo, &DBYX.byValue) == OK)
			{
				WriteSYX(g_Sys.wIOEqpID, 1, &DBYX);	
			}
		}

		for(i = 0; i < g_Sys.MyCfg.wDYXNum; i++)
		{
			DBDYX.wNo = i;
			if(ReadBMDYX(BM_EQP_MYIO, DBDYX.wNo, &DBDYX.byValue1, &DBDYX.byValue2) == OK)
			{
				WriteDYX(g_Sys.wIOEqpID, 1, &DBDYX, TRUE);	
			}
		}
	}
 		
	for( i = 0; i < YX_PORT_NUM; i++)
	{
		ReadBMMyDiValue( i, &g_DiValue[i]);
	}

	while(1)
	{
		ret = ReadBMSOE(BM_EQP_MYIO, &no,&value,&tm);
		if(ret == 0) //单点
		{
			DBSOE.wNo = DBYX.wNo = DBCOS.wNo = no;
			DBSOE.byValue = DBYX.byValue = DBCOS.byValue = value;
			DBSOE.Time = tm;
			WriteSYX(g_Sys.wIOEqpID, 1, &DBYX);	
			WriteSCOS(g_Sys.wIOEqpID, 1, &DBCOS);
			WriteSSOE(g_Sys.wIOEqpID, 1, &DBSOE);	
		}
		else if(ret == 1)//双点
		{
			DBDSOE.wNo = DBDYX.wNo = DBDCOS.wNo = no;
			DBDSOE.byValue1 = DBDYX.byValue1 = DBDCOS.byValue1 = (value>>8) & 0xFF;
			DBDSOE.byValue2 = DBDYX.byValue2 = DBDCOS.byValue2 = value & 0xFF;
			DBDSOE.Time = tm;
			WriteDYX(g_Sys.wIOEqpID, 1, &DBDYX, TRUE);
			WriteDCOS(g_Sys.wIOEqpID, 1, &DBDCOS);	
			WriteDSOE(g_Sys.wIOEqpID, 1, &DBDSOE);	
		}
		else
		{
			break;
		}
	}
		
}

int GetMyDiValue(int no)
{
	int port;
	WORD bit;

	if ((no < 0) || (no >= (MAX_YX_PORT_NUM*MAX_YX_NUM_PER_PORT)))  return 0;

	port = g_YxNo2Port[no].port;
	bit = g_YxNo2Port[no].bit;

    if (g_DiValue[port]&bit) return 1;
	else return 0;
}

#endif /*INCLUDE_YX*/

