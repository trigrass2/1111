/*------------------------------------------------------------------------
 Module:       	PrLogicLow.c
 Author:        helen
 Project:       
 State:			
 Creation Date:	2014-11-06
 Description:   
------------------------------------------------------------------------*/

#include "syscfg.h"
#include "sys.h"

#ifdef _POWEROFF_TEST_VER_
VYcFaultIRec *prFaultIRec;
void createPrYcRef(void);
void PowerOffLogic(int fd);
extern float prValue_l2f(int ui_flag, VFdCfg *pCfg, long value);
#endif

#ifdef INCLUDE_PR
#include "PrLogic.h"
#include "comtrade.h"
#include "PrLogicJd.h"

static void SUDYLogic_Init(LogicElem *plogic);
static void SUDYLogic(LogicElem *plogic);
static void SUSYLogic_Init(LogicElem *plogic);
static void SUSYLogic(LogicElem *plogic);
static void OPTBSLogic_Init(LogicElem *plogic);
static void OPTBSLogic(LogicElem *plogic);
static void SYBSLogic_Init(LogicElem *plogic);
static void SYBSLogic(LogicElem *plogic);
static void U0Logic_Init(LogicElem *plogic);
static void U0Logic(LogicElem *plogic);
static void UDiffLogic_Init(LogicElem * plogic);
static void UDiffLogic(LogicElem * plogic);
static void SLWYLogic_Init(LogicElem *plogic);
static void SLWYLogic(LogicElem *plogic);
static void ICntLogic_Init(LogicElem *plogic);
static void ICntLogic(LogicElem *plogic);
static void FZBSLogic_Init(LogicElem *plogic);
static void FZBSLogic(LogicElem *plogic);
static void JDLogic_Init(LogicElem *plogic);
static void JDLogic(LogicElem *plogic);

#ifdef INCLUDE_PR_PRO
static void DfJzLogic_Init(LogicElem * plogic);
static void DfJzLogic(LogicElem * plogic);
static void ChTqLogic_Init(LogicElem *plogic);
static void ChTqLogic(LogicElem *plogic);
static void ShTqLogic_Init(LogicElem *plogic);
static void ShTqLogic(LogicElem *plogic);
#endif

static void LocalYxLogic_Init(LogicElem * plogic);
static void LocalYxLogic(LogicElem *plogic);

static void HwSYLogic_Init(LogicElem *plogic);
static void HwSYLogic(LogicElem *plogic);

static void LXFZLogic_Init(LogicElem *plogic);
static void LXFZLogic(LogicElem *plogic);

/*只执行一次，在每个逻辑里分合闸,低任务运行*/
static LogicInfo tLogicPublicTable[]=
{
#ifdef INCLUDE_PR_PRO
  {GRAPH_CHTQ,  "重合同期",   ChTqLogic_Init, ChTqLogic},
  {GRAPH_SHTQ,  "手合同期",   ShTqLogic_Init, ShTqLogic},
  {GRAPH_DF,    "低频",       DfJzLogic_Init, DfJzLogic},
#endif
  {GRAPH_SLWY,  "SL无压",     SLWYLogic_Init,  SLWYLogic},
  {GRAPH_FZBS,  "分闸闭锁",   FZBSLogic_Init,  FZBSLogic},
  {GRAPH_OPTBS, "合闸闭锁",   OPTBSLogic_Init, OPTBSLogic},
  {GRAPH_SYBS,  "瞬压闭锁",   SYBSLogic_Init,  SYBSLogic},
  {GRAPH_SUDY,  "单侧得压",   SUDYLogic_Init,  SUDYLogic},
  {GRAPH_SUSY,  "单测失压",   SUSYLogic_Init,  SUSYLogic},
  {GRAPH_U0,    "零压",       U0Logic_Init,    U0Logic},
  {GRAPH_UDIFF, "压差",       UDiffLogic_Init, UDiffLogic},
  {GRAPH_ICNT,  "过流计数",   ICntLogic_Init,  ICntLogic},
  {GRAPH_LOCALYX,  "遥信处理",   LocalYxLogic_Init,  LocalYxLogic},
  {GRAPH_JD, "接地故障",     JDLogic_Init,  JDLogic},
  {GRAPH_SYHW,  "硬件瞬压",   HwSYLogic_Init,  HwSYLogic},
  {GRAPH_LXFZ,  "连续分闸",LXFZLogic_Init,  LXFZLogic},

};

LogicElem *pLogicLow;
LogicElem *prLogicPublic;
LogicElem *pLogicPublic;

VPrRunPublic *prRunPublic;
VPrRunPublic *pRunPublic;

VFdCfg   *pCfgTq;
VFdProtCal *pValTq;
VFdCfg   *pCfgSL;
VPubProtCal *pValSL;
VPubProtCal *pubProtVal;

VPrYxPubOpt *prYxPubOpt;
VPrEvtPubOpt *prEvtPubOpt;

VPrSetPublic *pPublicSet;
int g_prPubInit=0;

extern VFdProtCal *fdProtVal;
extern VFdCfg   *pFdCfg;
extern int g_prDisable;
extern LogicElem *prLogicElem;
extern VPrSetPublic *prSetPublic[2];
extern VPrRunInfo *prRunInfo;
extern VPrRunSet *prRunSet[2];
extern VBusProtCal BusVal;
extern void SetFaDiffBusFault(int flag);

extern VAiProtCal aiProtCal[MAX_AI_NUM];

int CreateLogicLow(void)
{
    int i,j;
	
    prLogicPublic = malloc(pubfdNum*GRAPH_PUBLIC_NUM*sizeof(LogicElem));

	if(prLogicPublic == NULL)
		return ERROR;
	memset(prLogicPublic, 0, pubfdNum*GRAPH_PUBLIC_NUM*sizeof(LogicElem));
  
    for(j = 0 ; j < pubfdNum; j++)
    {
        pLogicPublic = prLogicPublic + j*GRAPH_PUBLIC_NUM;
      	for(i=0; i<GRAPH_PUBLIC_NUM; i++)
      	{
      		if(tLogicPublicTable[i].Init)  //logic 初始化
      		   tLogicPublicTable[i].Init(pLogicPublic+i);
      	}
    }
	return OK;
}

int prRunInfoPublicInit(void)
{
    int i,j;
	WORD yxno;
#if(TYPE_USER == USER_YUXI)	
	BYTE yxvalue;
#endif	
	struct VDBYX DBYX;
	
    prRunPublic = (VPrRunPublic *)malloc(fdNum*sizeof(VPrRunPublic));  //定值需要，故采用fdNum而不是pubfdNum

	if(prRunPublic == NULL)  return ERROR;

	prYxPubOpt = (VPrYxPubOpt *)malloc(sizeof(VPrYxPubOpt));  //遥信环形数组

	if (prYxPubOpt == NULL) return ERROR;

	prEvtPubOpt = (VPrEvtPubOpt *)malloc(sizeof(VPrEvtPubOpt)); //事件环形数组

	if (prEvtPubOpt == NULL) return ERROR;

	pubProtVal = (VPubProtCal *)malloc(pubfdNum*sizeof(VPubProtCal));
	if(pubProtVal == NULL) return ERROR;

	
	yxno = g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low;

	memset(prRunPublic, 0, fdNum*sizeof(VPrRunPublic));
	memset(prYxPubOpt, 0, sizeof(VPrYxPubOpt));
	memset(prEvtPubOpt, 0, sizeof(VPrEvtPubOpt));
	memset((BYTE*)pubProtVal, 0, pubfdNum*sizeof(VPubProtCal));
	
    /***考虑多回线 是否投入再决定第二回线 ?***/
	prRunPublic->vyx_start_no = yxno;
	DBYX.byValue = 0x01;
	for (j=0; j<PR_VYX_NUM; j++)
	{
		DBYX.wNo = prRunPublic->vyx_start_no+j;
		WriteSYX(BM_EQP_MYIO, DBYX.wNo,DBYX.byValue);
	}
	
#if(TYPE_USER == USER_YUXI)	
	//玉溪ftu初始化读取失电分闸遥信
	extNvRamGet(NVRAM_AD_WYFZ_YX, &yxvalue, sizeof(yxvalue));
	if(yxvalue == 0x81)
	{
		DBYX.wNo = prRunPublic->vyx_start_no + PR_VYXP_WY_FZ_NO;
		DBYX.byValue = 0x81;
		WriteSYX(BM_EQP_MYIO, DBYX.wNo,DBYX.byValue);
		prRunPublic->vyx_bak = prRunPublic->vyx |= PR_VYXP_WY_FZ;	
	}
#endif

	prRunPublic->dw_nvram_addr = NVRAM_BS;
	extNvRamGet(prRunPublic->dw_nvram_addr, (BYTE*) &prRunPublic->dw_NVRAM_BS, sizeof(prRunPublic->dw_NVRAM_BS));

	for(i = 1; i < pubfdNum;i++)
	{
		prRunPublic[i].vyx_start_no = yxno + i*PR_VYX_NUM;
		DBYX.byValue = 0x01;
		for (j=0; j<PR_VYX_NUM; j++)
		{
			DBYX.wNo = prRunPublic[i].vyx_start_no+j;
			WriteSYX(BM_EQP_MYIO, DBYX.wNo,DBYX.byValue);
		}

		//因铁电NVRAM_BS不够所有的，故前6回线放NVRAM_BS，其他的放NVRAM_BS_SP
		if(i > 6)
			prRunPublic[i].dw_nvram_addr = NVRAM_BS_SP + 4*(i-7);
		else
			prRunPublic[i].dw_nvram_addr = NVRAM_BS + 4*i;
		
		extNvRamGet(prRunPublic[i].dw_nvram_addr, (BYTE*) &prRunPublic[i].dw_NVRAM_BS, sizeof(prRunPublic[i].dw_NVRAM_BS));
	}
	     
#ifdef _POWEROFF_TEST_VER_
     createPrYcRef();
#endif
	return OK;
}


void prVyxPubHandle(void)
{
	DWORD var,mask; int i;
	VDBYxChg *pYxChg;
	struct VDBSOE *pDBSOE;
	BYTE value; WORD no;
	DWORD dYxValue;


	dYxValue = pRunPublic->vyx;

	if (dYxValue != pRunPublic->vyx_bak)
	{
		
		var = dYxValue^pRunPublic->vyx_bak;
		mask = 1;
		for (i=0; i<PR_VYX_NUM; i++)
		{
			if (var & mask)
			{
			    pYxChg = prYxPubOpt->yxchg + prYxPubOpt->wWritePtr;
				pYxChg->flag = 0;
				pDBSOE = &(pYxChg->DBSOE);
			    GetSysClock(&(pDBSOE->Time), CALCLOCK);
				no = pRunPublic->vyx_start_no+i;
				if (dYxValue & mask) value = 0x81;
				else value = 0x01;
			#if(TYPE_USER != USER_GUANGXI)
                if(value == 0x81)
					pDBSOE->Time = pRunPublic->fault.i_clock;
			#endif
				pDBSOE->wNo = no; 
				pDBSOE->byValue = value;	

				prYxPubOpt->wWritePtr = (prYxPubOpt->wWritePtr+1)&(MAX_YX_PUB_NUM-1);
#if(TYPE_USER == USER_YUXI)	
				if(mask == PR_VYXP_WY_FZ)
				{
					extNvRamSet(NVRAM_AD_WYFZ_YX, &value, sizeof(value));
				}
#endif				
    		}
			mask <<= 1;		
		}
 	    pRunPublic->vyx_bak = dYxValue;
	}	

}

#if(TYPE_USER == USER_GUIYANG)

 int GetFaAutoMode(void)
 {
 #if(DEV_SP == DEV_SP_FTU)
 	//if (GetExtMmiYx(EXTMMI_AUTO)&&(pPublicSet->bFaSelectMode == PR_FA_LOCAL))
 	if (GetMyDiValue(8)&&!GetExtMmiYx(EXTMMI_AUTO)&&GetExtMmiYx(EXTMMI_FA))
#elif(DEV_SP == DEV_SP_WDG)
	if (!GetExtMmiYx(XINEXTMMI_MS)&&!prSetPublic[prRunPublic->set_no]->bAuto)
#endif
		return 1;
	else
		return 0;
 }

#endif

BYTE prLogicPublicPre(int no)
{
#if ((TYPE_USER == USER_GUANGXI)||(TYPE_USER == USER_GUIYANG))
   VPrRunInfo *pRunInfo;
#endif

	 pLogicPublic = prLogicPublic + no*GRAPH_PUBLIC_NUM;
      pRunPublic = prRunPublic + no;
	 pRunPublic->fd = pPublicSet->bySLLine;

	 pValSL = pubProtVal + pPublicSet->bySLLine;
	 pCfgSL = pFdCfg + pPublicSet->bySLLine;
	
#ifdef INCLUDE_PR_PRO
	 pValTq = fdProtVal + pPublicSet->byTqLine;
	 pCfgTq = pFdCfg + pPublicSet->byTqLine;
#endif

#if ((TYPE_USER == USER_GUANGXI)||(TYPE_USER == USER_GUIYANG))
	pRunInfo = prRunInfo + pPublicSet->bySLLine;
	 if (!pRunInfo->gzjc_on) 
	 {
		 prRunPublic->vyx = 0;
		 prVyxPubHandle();
	 	return 1;
	 }
#endif
	 return 0;
}
void prWritePubEvent(WORD no)
{

    VDBEvent *pevt;
	pevt = prEvtPubOpt->evt + prEvtPubOpt->wWritePtr;

    pevt->flag = 0;
	pevt->code = 0;

	pevt->Time = pRunPublic->fault.i_clock;

	memset(pevt->data, 0, 8*sizeof(DWORD));
    
	switch(no)
	{
	#ifdef INCLUDE_PR_PRO
        case EVENT_DF:
			 pevt->fd = pRunPublic->fd;
			 pevt->data[0] = pPublicSet->bDfJl;
			 pevt->code |= 1<<EVENT_DF;
			 break;
		case EVENT_SHTQ:
			 pevt->code |= 1<<EVENT_SHTQ;
			 break;
	#endif
		case EVENT_U0:
			 pevt->fd = pRunPublic->fd;
			 pevt->code |= 1<<EVENT_U0;
			 break;
		case EVENT_DY_HZ:
			 pevt->fd = pRunPublic->fd;
		     pevt->code |= 1<<EVENT_DY_HZ;
			 break;
		case EVENT_SY_HZ:
			 pevt->fd = pRunPublic->fd;
	 	 	 pevt->code |= 1<<EVENT_SY_HZ;
			 break;
		case EVENT_UDIFF:
			 pevt->fd = pRunPublic->fd;		 
		 	 pevt->code |= 1<<EVENT_UDIFF;
			 break;
		case EVENT_SL_WY:
			 pevt->fd = pRunPublic->fd;
			 pevt->code |= 1<<EVENT_SL_WY;
		     break;
		case EVENT_I_CNT:
			 pevt->fd = pRunPublic->fd;
			 pevt->code |= 1<<EVENT_I_CNT;
			 break;
		case EVENT_I_TRIP:
			 pevt->fd = pRunPublic->fd;
			 pevt->code |= 1<<EVENT_I_TRIP;
			 break;
		default:
			 pevt->code = 0;
			 break;
	}
	prEvtPubOpt->wWritePtr = (prEvtPubOpt->wWritePtr+1)&(MAX_EVT_PUB_NUM-1);

}

void  prBusDrv(void)
{

#ifdef INCLUDE_FA_DIFF
    int i, j;
	BOOL BusStart;
	long f1,f2,f3,f4;

	DWORD temp1,temp2;

	static DWORD count = 0;
	static int cnt = 0;

	
    protBusValtoFd();


	if (!(pPublicSet->dYb & PR_YB_BUS)) return;


    BusStart = FALSE;
	for (i=0; i<3; i++)
	{
	   if ((BusVal.dBusI[i][18] > pPublicSet->dIBus)&&(BusVal.dBusI[i][18]*10 > BusVal.dBrkI[i]*3))
	   {
	       BusStart = TRUE;
		   break;
	   }
	}

	if (!BusStart)
	{
	   pRunPublic->vyx &= ~PR_VYXP_BUS;
	   return;
	}

    for (j=0; j<3; j++)
    {
        temp1 = BusVal.dBusI[i][j*6]*10;
		if (temp1 < (BusVal.dBrkI[i]*3))
			break;
#if 0	
		temp1 = BusVal.dBusI[i][j*6+1]*10 + BusVal.dBusI[i][j*6+2]*22 + BusVal.dBusI[i][j*6+3]*31 +
			        BusVal.dBusI[i][j*6+4]*32 + BusVal.dBusI[i][j*6+5]*25;
	    temp2 = BusVal.dBusI[i][j*6]*3;
	    if (2*temp1 > temp2)
		   break;
#endif
    }

	if ((j == 3) && !(pRunPublic->vyx & PR_VYXP_BUS))
	{
	    SetFaDiffBusFault(pPublicSet->bBusTrip);
		cnt = 0;
	    pRunPublic->vyx |= PR_VYXP_BUS;
	}
	else if ((j < 2)&&(pRunPublic->vyx & PR_VYXP_BUS))
		pRunPublic->vyx &= ~PR_VYXP_BUS;
#endif

}

void prLowDrv(void)
{
    int i,j;
	BOOL bSetChg=FALSE;


	if ((g_prDisable) || (g_Sys.dwErrCode & SYS_ERR_AIO))
	{
		return;
	}
    
	for(j = 0; j < pubfdNum; j++)
	{
		pPublicSet = prSetPublic[prRunPublic[j].set_no] + j;
		if (prLogicPublicPre(j))
			continue;

		if(pPublicSet->bSetChg)
			bSetChg = TRUE;

		protValtoPub();
		
	#ifdef INCLUDE_PR_PRO
		protCal_AiFVal();
	#endif

		for(i=0; i<GRAPH_PUBLIC_NUM; i++)
		{
				tLogicPublicTable[i].Func(pLogicPublic+i);
		}

       if(bSetChg)
		  pPublicSet->bSetChg = FALSE;
		prVyxPubHandle();  //多回线
	}	
	prBusDrv();

 
}

int prLowInit(void)
{
    if (CreateLogicLow() == ERROR) return ERROR;
	if (prRunInfoPublicInit() == ERROR) return ERROR;

#ifdef _POWEROFF_TEST_VER_
    createPrYcRef();
#endif
    g_prPubInit = 1;
	return OK;

}

void protectlow(void)
{
	int i;
	DWORD events;

	evReceive(PRLOW_ID, EV_TM1, &events);

	if (events & EV_TM1)
    {
#ifdef INCLUDE_JD_PR
		PrJdJudge();
#endif
		for (i=0; i<fdNum; i++)
		{
#ifdef _POWEROFF_TEST_VER_
			PowerOffLogic(i);
#endif
		}
		prLightHandle();
	}
}


static void prSKgDo(int ykno, int val)
{
    int ret;

	ret = ykOutput(ykno, val |PRYK);		
	if(2 == ret)
	{
		ykCancel(-1,ykno, 1);     
		ykCancel(-1,ykno, 0);
		ret = ykOutput(ykno, val |PRYK);
	}
}

#ifdef INCLUDE_PR_PRO
extern VAiTqVal *pTqVal;  
#define MY_DF_CNTLAST     (plogic->aplUser[0])
#define MY_DF_FREQLAST    (plogic->aplUser[1])
#define MY_DF_FREQSLIP    (plogic->aplUser[2])
void DfJzLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	PRLIB_30(pelm);
	
	MY_DF_CNTLAST = Get100usCnt();
	MY_DF_FREQLAST = 5000;
	MY_DF_FREQSLIP = 0;
}

void DfJzLogic(LogicElem *plogic)
{
	int freq,diff;
	BOOL bEnable = TRUE;
	BOOL bTime = TRUE;
	DWORD cnt,interval;
	VFdCfg   *pCfg;
    VFdProtCal *pVal;
	VPrRunSet  *pRunSet;
	VPrRunInfo *pRunInfo;

    EP_ELEMENT *pelm = &(plogic->elem);

	if (!(pPublicSet->dYb & PR_YB_DF))
		return;

    pCfg = pFdCfg + pPublicSet->byTqLine;
	pVal = fdProtVal + pPublicSet->byTqLine;
	pLogicLow = prLogicElem + pPublicSet->byTqLine*GRAPH_NUM;

	pRunInfo = prRunInfo + pPublicSet->byTqLine;
	pRunSet = prRunSet[pRunInfo->set_no] + pPublicSet->byTqLine;

	if (pVal->dUxx[0] < pPublicSet->dUFreq[0])
		bEnable = FALSE;

    freq = pTqVal->wTqFreq[0];
	cnt  = Get100usCnt();
	interval = cnt - MY_DF_CNTLAST;
	if(interval < 1000)
		bTime = FALSE;
	else
	{
	    diff = freq - MY_DF_FREQLAST;
		if (diff < 0)
			diff = -diff;
		MY_DF_FREQSLIP = diff * 1000/interval;
		MY_DF_CNTLAST = cnt;
		MY_DF_FREQLAST = freq;
	}

	pelm->ppioIn[_PR_30_ENABLE].bVal   = bEnable;
	pelm->ppioIn[_PR_30_UAB].ulVal     = pVal->dUxx[0];
	pelm->ppioIn[_PR_30_UBC].ulVal     = pVal->dUxx[1];
	pelm->ppioIn[_PR_30_UCA].ulVal     = pVal->dUxx[2];
	pelm->ppioIn[_PR_30_IA].ulVal      = pVal->dI[0];
	pelm->ppioIn[_PR_30_IB].ulVal      = pVal->dI[1];
	pelm->ppioIn[_PR_30_IC].ulVal      = pVal->dI[2];
	pelm->ppioIn[_PR_30_FREQ].ulVal    = MY_DF_FREQLAST;
	pelm->ppioIn[_PR_30_FSLIP].ulVal   = MY_DF_FREQSLIP;
	pelm->ppioIn[_PR_30_BLOCK].ulVal   = (pLogicLow[GRAPH_PT].Output[BOOL_OUTPUT_PICK]&pRunSet->bPTBS);
		                               
    pelm->ppioIn[_PR_30_TIME].bVal     = bTime;
    pelm->Scan_Func(pelm);

	plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_30_PICK].bVal;
	plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_30_OPERATE].bVal;

	if(plogic->Output[BOOL_OUTPUT_TRIP]&&!(pRunPublic->vyx&PR_VYXP_DF))
   	{
   	   pRunPublic->vyx |= PR_VYXP_DF;
	   GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);

	   prSKgDo(pCfg->kg_ykno, 0);
	   prWritePubEvent(EVENT_DF);
	}
    else if(!plogic->Output[BOOL_OUTPUT_TRIP]&&(pRunPublic->vyx&PR_VYXP_DF))
   	{
   	   pRunPublic->vyx &= ~PR_VYXP_DF;
	   pRunPublic->vyx &= ~PR_VYX_TRIP;
    }

}

void ChTqLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	PRLIB_31(pelm);
}

void ChTqLogic(LogicElem *plogic)
{
    BOOL bXlU,bMxU, bU75;
    EP_ELEMENT *pelm = &(plogic->elem);
     VPrRunSet  *pRunSet;
	VPrRunInfo *pRunInfo;
	
	pRunInfo = prRunInfo + pPublicSet->byTqLine;
	pRunSet = prRunSet[pRunInfo->set_no] + pPublicSet->byTqLine;
        
	if (!(pRunSet->dYb & PR_YB_CH))
		return;

	if (pPublicSet->bTqMode == PR_CH_MODE_NONE)
	{
		plogic->Output[BOOL_OUTPUT_TRIP] = 1;
		return;
	}
	bXlU = bMxU = FALSE;

	if ((pPublicSet->bTqMode == PR_CH_MODE_WY)||(pPublicSet->bTqMode == PR_CH_MODE_WYTQ))
	{

	    if (pTqVal->dTqU[1] < pPublicSet->dTqUMin[0])
			bXlU = TRUE;
		
	    if (pPublicSet->bTqWyAny)
	    {
	        if ((pValTq->dUxx[0] < pPublicSet->dTqUMin[0])&&
				(pValTq->dUxx[1] < pPublicSet->dTqUMin[1])&&
				(pValTq->dUxx[2] < pPublicSet->dTqUMin[2]))
				bMxU = TRUE;
	    }
		else
			bMxU = TRUE;
	}
	if ((pPublicSet->bTqMode == PR_CH_MODE_TQ)||(pPublicSet->bTqMode == PR_CH_MODE_WYTQ))
	{

	    bU75 = FALSE;
		
		if ((pValTq->dUxx[0] > pPublicSet->dTqU75[0])&&
			(pTqVal->dTqU[1] > pPublicSet->dTqU75[0]))
			bU75 = TRUE;
		
	    pelm->ppioIn[_PR_31_ENABLE].bVal   = !(bXlU&bMxU);
		pelm->ppioIn[_PR_31_UENABLE].bVal  = bU75;
	    pelm->ppioIn[_PR_31_MFREQ].ulVal   = pTqVal->wTqFreq[0];
	    pelm->ppioIn[_PR_31_MFSLIP].ulVal  = pTqVal->wTqFSlip[0];
	    pelm->ppioIn[_PR_31_XFREQ].ulVal   = pTqVal->wTqFreq[1];
	    pelm->ppioIn[_PR_31_XFSLIP].ulVal  = pTqVal->wTqFSlip[1];
		pelm->ppioIn[_PR_31_PRETQ].bVal    = FALSE;

		pelm->Scan_Func(pelm);
	}

	if (pPublicSet->bTqMode == PR_CH_MODE_WY)
		plogic->Output[BOOL_OUTPUT_TRIP] = bXlU&bMxU;
	else if (pPublicSet->bTqMode == PR_CH_MODE_TQ)
		plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_31_OPERATE].bVal;
	else
		plogic->Output[BOOL_OUTPUT_TRIP] = (bXlU&bMxU)|pelm->aioOut[_PR_31_OPERATE].bVal;

}


#define MY_SHTQ_TIMER1  ((TMAXTIMERELAY*)(plogic->apvUser[0]))
void ShTqLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	PRLIB_31(pelm);

	plogic->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
}


void ShTqLogic(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);
	BOOL bXlU,bMxU,bYxHwj,bTrip, bU75;
//	VPrRunSet  *pRunSet;
//	VPrRunInfo *pRunInfo;


	if (!(pPublicSet->dYb & PR_YB_SHTQ))
		return;

	pLogicLow = prLogicElem + pPublicSet->byTqLine*GRAPH_NUM;
	if (pLogicLow[GRAPH_PT].Output[BOOL_OUTPUT_TRIP])
	    return;

	if (pPublicSet->bSetChg)
    {
        InitTR(&(MY_SHTQ_TIMER1->tMaxTimeRelay[0]), pPublicSet->dTTqTwj, 0);
		InitTR(&(MY_SHTQ_TIMER1->tMaxTimeRelay[1]), 0, pPublicSet->dTTqYk);
		InitTR(&(MY_SHTQ_TIMER1->tMaxTimeRelay[2]), 0, 4000); 
    }

	bYxHwj = pValTq->dYx & PR_KG_YX_STATUS;
	RunTR(&(MY_SHTQ_TIMER1->tMaxTimeRelay[0]), !bYxHwj);
	if (!MY_SHTQ_TIMER1->tMaxTimeRelay[0].boolTrip||bYxHwj)
	{
	    plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
		return;
	}
	RunTR(&(MY_SHTQ_TIMER1->tMaxTimeRelay[1]), pTqVal->bYk);
	if (!MY_SHTQ_TIMER1->tMaxTimeRelay[1].boolTrip)
	{
	    plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
		return;
	}
	else
		pTqVal->bYk = FALSE;
	


	if ((pPublicSet->bShTqMode == PR_SH_MODE_WY)||
		(pPublicSet->bShTqMode == PR_SH_MODE_WYTQ)||
		(pPublicSet->bShTqMode == PR_SH_MODE_WYZTQ))
	{
	    bXlU = bMxU = FALSE;

	    if (pTqVal->dTqU[1] < pPublicSet->dTqUMin[0])
			bXlU = TRUE;

	    if (pPublicSet->bTqWyAny)
	    {
	        if ((pValTq->dUxx[0] < pPublicSet->dTqUMin[0])&&
				(pValTq->dUxx[1] < pPublicSet->dTqUMin[1])&&
				(pValTq->dUxx[2] < pPublicSet->dTqUMin[2]))
				bMxU = TRUE;
	    }
		else
			bMxU = TRUE;
	}
	if ((pPublicSet->bShTqMode == PR_SH_MODE_TQ)||(pPublicSet->bShTqMode == PR_SH_MODE_WYTQ))
	{

		bU75 = FALSE;
		
		if ((pValTq->dUxx[0] > pPublicSet->dTqU75[0])&&
			(pTqVal->dTqU[1] > pPublicSet->dTqU75[0]))
			bU75 = TRUE;
		
	    pelm->ppioIn[_PR_31_ENABLE].bVal   = !(bXlU&bMxU);
		pelm->ppioIn[_PR_31_UENABLE].bVal  = bU75;
	    pelm->ppioIn[_PR_31_MFREQ].ulVal   = pTqVal->wTqFreq[0];
	    pelm->ppioIn[_PR_31_MFSLIP].ulVal  = pTqVal->wTqFSlip[0];
	    pelm->ppioIn[_PR_31_XFREQ].ulVal   = pTqVal->wTqFreq[1];
	    pelm->ppioIn[_PR_31_XFSLIP].ulVal  = pTqVal->wTqFSlip[1];
		pelm->ppioIn[_PR_31_PRETQ].bVal    = FALSE;

		pelm->Scan_Func(pelm);
	}
	else if((pPublicSet->bShTqMode == PR_SH_MODE_ZTQ)||(pPublicSet->bShTqMode == PR_SH_MODE_WYZTQ))
	{
	    bU75 = FALSE;
		
		if ((pValTq->dUxx[0] > pPublicSet->dTqU75[0])&&
		    (pValTq->dUxx[1] > pPublicSet->dTqU75[1])&&
			(pValTq->dUxx[2] > pPublicSet->dTqU75[2])&&
			(pTqVal->dTqU[1] > pPublicSet->dTqU75[0]))
			bU75 = TRUE;
		
	    pelm->ppioIn[_PR_31_ENABLE].bVal   = !(bXlU&bMxU);
		pelm->ppioIn[_PR_31_UENABLE].bVal  = bU75;
	    pelm->ppioIn[_PR_31_MFREQ].ulVal   = pTqVal->wTqFreq[0];
	    pelm->ppioIn[_PR_31_MFSLIP].ulVal  = pTqVal->wTqFSlip[0];
	    pelm->ppioIn[_PR_31_XFREQ].ulVal   = pTqVal->wTqFreq[1];
	    pelm->ppioIn[_PR_31_XFSLIP].ulVal  = pTqVal->wTqFSlip[1];
		pelm->ppioIn[_PR_31_PRETQ].bVal    = TRUE;

		pelm->Scan_Func(pelm);
	}

	if (pPublicSet->bShTqMode == PR_SH_MODE_WY)
		bTrip = bXlU&bMxU;
	else if ((pPublicSet->bShTqMode == PR_SH_MODE_TQ)||(pPublicSet->bShTqMode == PR_SH_MODE_ZTQ))
		bTrip = pelm->aioOut[_PR_31_OPERATE].bVal;
	else
		bTrip = (bXlU&bMxU)|pelm->aioOut[_PR_31_OPERATE].bVal;
	RunTR(&(MY_SHTQ_TIMER1->tMaxTimeRelay[2]), bTrip);
	if (!plogic->Output[BOOL_OUTPUT_TRIP] && MY_SHTQ_TIMER1->tMaxTimeRelay[2].boolTrip)
	{
	    plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
		ResetTR(&(MY_SHTQ_TIMER1->tMaxTimeRelay[1]));
		prSKgDo(pCfgTq->kg_ykno, 1);
		prWritePubEvent(EVENT_SHTQ);
	}
	
}

/*只判断回线0开关合闸*/
int prCheckShTq(int ykId, int val)
{
    VFdCfg *pCfg;
	VPrSetPublic *pRunSet;
	
	pRunSet = prSetPublic[prRunPublic->set_no];

	pCfg = pFdCfg+pRunSet->byTqLine;


    if ((pCfg->kg_ykno == ykId)&&(val == 1))
    {
        if ((pRunSet->dYb & PR_YB_SHTQ)&&(pRunSet->bShTqMode > 0))
		{
		    pTqVal->bYk = TRUE;
		    return 1;
        }
    }
	return 0;
	
}
#endif

//电压型保护取一回线的两路电压
#define MY_SLWY_TIMER1   ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_SLWY_TIMER2   ((TMAXTIMERELAY*)(plogic->apvUser[1]))

void SLWYLogic_Init(LogicElem *plogic)
{
   	plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	plogic->apvUser[1] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
}

void SLWYLogic(LogicElem *plogic)
{
    int i;
	BOOL bNoU;
	VPrRunInfo* ppRunInfo;

	if (pPublicSet->bSetChg)
	{
		InitTR(MY_SLWY_TIMER1, pPublicSet->dTWY, 0);
		InitTR(&(MY_SLWY_TIMER2->tMaxTimeRelay[0]), 0, pPublicSet->dTWY+2000);    // gu  
		InitTR(&(MY_SLWY_TIMER2->tMaxTimeRelay[1]), 0, pPublicSet->dTWY+20000);    // 接地故障失压跳闸
		InitTR(&(MY_SLWY_TIMER2->tMaxTimeRelay[3]), 0, 500);
	}
	
    bNoU = TRUE;

    for(i=0; i<3; i++)
    {
        if(pValSL->dU[i] > pPublicSet->dSLUMin[i])
			bNoU = FALSE;
    }
		
	pLogicLow = prLogicElem + pPublicSet->bySLLine*GRAPH_NUM;

#ifndef _GUANGZHOU_TEST_VER_	
	if(pPublicSet->bXJDL || pPublicSet->bDXJD)
#endif
		bNoU &= pLogicLow[GRAPH_WL].Output[BOOL_OUTPUT_PICK] ;
		
    RunTR(&(MY_SLWY_TIMER2->tMaxTimeRelay[0]), (pValSL->dYx&PR_KG_YX_STATUS) );//gu  山东 无压时自动分位
		
	RunTR(MY_SLWY_TIMER1, bNoU &&(MY_SLWY_TIMER2->tMaxTimeRelay[0].boolTrip));//gu  山东 无压时自动脱扣变分位


	if(MY_SLWY_TIMER1->boolTrip && !plogic->Output[BOOL_OUTPUT_PICK])
	{
	   plogic->Output[BOOL_OUTPUT_PICK] = TRUE;
	}
	else if(!MY_SLWY_TIMER1->boolTrip &&plogic->Output[BOOL_OUTPUT_PICK])
	   plogic->Output[BOOL_OUTPUT_PICK] = FALSE;

	if (pPublicSet->dYb& PR_YB_SLWY)
	{
	    if (plogic->Output[BOOL_OUTPUT_PICK] && !plogic->Output[BOOL_OUTPUT_TRIP] && (!pLogicPublic[GRAPH_OPTBS].Output[BOOL_OUTPUT_FZBS]))
	    {
	       plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
	   	   pRunPublic->vyx |= PR_VYXP_WY_FZ;
	       GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	       prSKgDo(pCfgSL->kg_ykno, 0);
	       prWritePubEvent(EVENT_SL_WY);
	    }
	}

	if((! pPublicSet->bFTUSD) && (pPublicSet->bFaWorkMode == PR_FA_I))
	{ 
		// 电压电流型Y 时限内失压并检测到故障电流跳，跳闸并闭锁合闸
		if (pLogicPublic[GRAPH_OPTBS].Output[BOOL_OUTPUT_YBS])
		{
		    if (plogic->Output[BOOL_OUTPUT_PICK] && !plogic->Output[BOOL_OUTPUT_TRIP])
		    {
		       plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
		   	   pRunPublic->vyx |= PR_VYXP_WY_FZ;
		       GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		       prSKgDo(pCfgSL->kg_ykno, 0);
		       prWritePubEvent(EVENT_SL_WY);
		    }
		}
		
		// 非首端接地故障失压分闸
		ppRunInfo = prRunInfo + pPublicSet->bySLLine;
		RunTR(&(MY_SLWY_TIMER2->tMaxTimeRelay[1]), ((ppRunInfo->vyx & (PR_VYX_I0_MASK )) != 0) ); 

		if (plogic->Output[BOOL_OUTPUT_PICK] && (!plogic->Output[BOOL_OUTPUT_TRIP]) && (MY_SLWY_TIMER2->tMaxTimeRelay[1].boolTrip))
	    {
	       plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
	   	   pRunPublic->vyx |= PR_VYXP_WY_FZ;
	       GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	       prSKgDo(pCfgSL->kg_ykno, 0);
	       prWritePubEvent(EVENT_SL_WY);
	    }
	}

#if(TYPE_USER == USER_YUXI)	//玉溪版本ftu,只考虑一回线 失电分闸遥信 检测到开关从分到合才变为分
	if (!plogic->Output[BOOL_OUTPUT_PICK] && plogic->Output[BOOL_OUTPUT_TRIP])
	{
		plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	}
	
	RunTR(&(MY_SLWY_TIMER2->tMaxTimeRelay[3]), !(pValSL->dYx&PR_KG_YX_STATUS));
	if((pRunPublic->vyx & PR_VYXP_WY_FZ) && (pValSL->dYx&PR_KG_YX_STATUS) && (MY_SLWY_TIMER2->tMaxTimeRelay[3].boolTrip))
	{
		pRunPublic->vyx &= ~PR_VYXP_WY_FZ;
	}
#else 
	if (!plogic->Output[BOOL_OUTPUT_PICK] && plogic->Output[BOOL_OUTPUT_TRIP])
	{
	    plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
		pRunPublic->vyx &= ~PR_VYXP_WY_FZ;
	}
#endif
}	

//  接地故障处理逻辑 根据过流方向计时
#define MY_JD_TIMER1   ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_JD_TIMER2   ((TIMERELAY*)(plogic->apvUser[1]))
void JDLogic_Init(LogicElem *plogic)
{
   	plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	plogic->apvUser[1] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
}

void JDLogic(LogicElem *plogic)
{
	BOOL bJD1,bJD2;

	if (!(pPublicSet->dYb& PR_YB_JD))
	   return;

	if (pPublicSet->bSetChg)
	{
		InitTR(MY_JD_TIMER1, pPublicSet->dTJD, 0);
		InitTR(MY_JD_TIMER2, pPublicSet->dTJDrevs, 0);
	}

	pLogicLow = prLogicElem + pPublicSet->bySLLine*GRAPH_NUM;
	
	bJD1 = pLogicLow[GRAPH_I01].Output[BOOL_OUTPUT_PICK]&&
		  pLogicLow[GRAPH_IDIR].Output[BOOL_OUTPUT_N];

	
	bJD2 = pLogicLow[GRAPH_I01].Output[BOOL_OUTPUT_PICK]&&
		   !pLogicLow[GRAPH_IDIR].Output[BOOL_OUTPUT_N];

	RunTR(MY_JD_TIMER1, bJD1);
	RunTR(MY_JD_TIMER2, bJD2);

	if((MY_JD_TIMER1->boolTrip|MY_JD_TIMER2->boolTrip) && !plogic->Output[BOOL_OUTPUT_TRIP])//跳闸
	{
	   plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
	 
	   prSKgDo(pCfgSL->kg_ykno, 0);
	   pRunPublic->vyx |= 0x80000000; 
	   GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	}
	else if(!MY_JD_TIMER1->boolTrip &&!MY_JD_TIMER2->boolTrip &&plogic->Output[BOOL_OUTPUT_TRIP])
	{
	   plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	   
	   pRunPublic->vyx &= ~0x80000000;
	}
}



#define MY_FZBS_TIMER1   ((TMAXTIMERELAY*)(plogic->apvUser[0]))
#define MY_HZ   (plogic->aplUser[0])
enum{HZ,HW, FZBS1, FZBS2};

void FZBSLogic_Init(LogicElem *plogic)
{
	plogic->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
}

void FZBSLogic(LogicElem *plogic)
{
	BOOL bHwj,bMTHz,bHz,bHw_Hz,bDoubleU;
	BOOL bFZ;

	if(pPublicSet->bSetChg)
	{
		InitTR(&(MY_FZBS_TIMER1->tMaxTimeRelay[HZ]), 0, 20000);
		InitTR(&(MY_FZBS_TIMER1->tMaxTimeRelay[HW]), 0, 20000);
		
		InitTR(&(MY_FZBS_TIMER1->tMaxTimeRelay[FZBS1]), pPublicSet->dTY  + 100, 0);
		SetTR_TRTT(&(MY_FZBS_TIMER1->tMaxTimeRelay[FZBS1]), 0);
		
		InitTR(&(MY_FZBS_TIMER1->tMaxTimeRelay[FZBS2]), pPublicSet->dTFZ2, 0);
		SetTR_TRTT(&(MY_FZBS_TIMER1->tMaxTimeRelay[FZBS2]), 0);
	}
	
	if(pRunPublic->bs_reset& PR_FZ_BS)
	{
		ResetTR(&(MY_FZBS_TIMER1->tMaxTimeRelay[FZBS1]));
         ResetTR(&(MY_FZBS_TIMER1->tMaxTimeRelay[FZBS2]));
		plogic->Output[BOOL_OUTPUT_FZBS]  = FALSE;
         MY_HZ= 0;
		pRunPublic->bs_reset &= ~PR_FZ_BS;
	}
//	if(!(pPublicSet->dYb&PR_YB_U_HZ))
//		return;
	if(!pPublicSet->bFZBS) //增加分闸闭锁控制字
		return;
	pLogicLow = prLogicElem + pPublicSet->bySLLine*GRAPH_NUM;
	
	bHz =  pLogicPublic[GRAPH_SUDY].Output[BOOL_OUTPUT_TRIP]|
				 pLogicPublic[GRAPH_SUSY].Output[BOOL_OUTPUT_TRIP]|
				 pLogicPublic[GRAPH_UDIFF].Output[BOOL_OUTPUT_TRIP];
	RunTR(&(MY_FZBS_TIMER1->tMaxTimeRelay[HZ]),bHz);

	bHwj = pValSL->dYx&PR_KG_YX_STATUS;
	/*人工合闸*/
	bMTHz = FALSE;
	RunTR(&(MY_FZBS_TIMER1->tMaxTimeRelay[HW]), !bHwj);
	if(bHwj && MY_FZBS_TIMER1->tMaxTimeRelay[HW].boolTrip)
		bMTHz = TRUE;
	
	bHw_Hz  = bMTHz & MY_FZBS_TIMER1->tMaxTimeRelay[HZ].boolTrip;  //只能是合闸之后Y 时限内
	
	bDoubleU = FALSE;
	if((pValSL->dU[0] > pPublicSet->dSLU[0]) && (pValSL->dU[2] > pPublicSet->dSLU[2]))
		bDoubleU = TRUE;
	
	MY_HZ = bHw_Hz | (MY_HZ & bDoubleU & bHwj);

	bFZ =  pLogicLow[GRAPH_RELAY].Output[BOOL_OUTPUT_FZ]|
		   pLogicPublic[GRAPH_SLWY].Output[BOOL_OUTPUT_TRIP]|
		   pLogicPublic[GRAPH_U0].Output[BOOL_OUTPUT_TRIP];
  RunTR(&(MY_FZBS_TIMER1->tMaxTimeRelay[FZBS1]), MY_HZ&(!bFZ));
	if(MY_FZBS_TIMER1->tMaxTimeRelay[FZBS1].boolTrip)
   {
		plogic->Output[BOOL_OUTPUT_FZBS] = TRUE;
	}
	RunTR(&(MY_FZBS_TIMER1->tMaxTimeRelay[FZBS2]), plogic->Output[BOOL_OUTPUT_FZBS] );
	if(MY_FZBS_TIMER1->tMaxTimeRelay[FZBS2].boolTrip)
	{
		plogic->Output[BOOL_OUTPUT_FZBS]  = FALSE;
	}
	
}


#define MY_OPTBS_TIMER1   ((TMAXTIMERELAY*)(plogic->apvUser[0]))
#define MY_OPTBS_MTHZBS   (plogic->aplUser[0])

void OPTBSLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	plogic->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
    PRLIB_122(pelm);
}

void OPTBSLogic(LogicElem *plogic)
{
    BOOL bHz, bHwj, bMTHz, bs, bTrip,bDu;
	VPrRunInfo* ppRunInfo;
	   
    EP_ELEMENT *pelm = &(plogic->elem);

    if(pPublicSet->bSetChg)
    {
       InitTR(&(MY_OPTBS_TIMER1->tMaxTimeRelay[0]), 0, 20000);
	   InitTR(&(MY_OPTBS_TIMER1->tMaxTimeRelay[1]), 0, 5000);
	   InitTR(&(MY_OPTBS_TIMER1->tMaxTimeRelay[2]), 0, 20000);
	   InitTR(&(MY_OPTBS_TIMER1->tMaxTimeRelay[3]), 0, 5000);
	   InitTR(&(MY_OPTBS_TIMER1->tMaxTimeRelay[4]),2*pPublicSet->dTY, 0);
    }

	pLogicLow = prLogicElem + pPublicSet->bySLLine*GRAPH_NUM;
	ppRunInfo = prRunInfo + pPublicSet->bySLLine;

	bHz =  pLogicPublic[GRAPH_SUDY].Output[BOOL_OUTPUT_TRIP]|
   	       pLogicPublic[GRAPH_SUSY].Output[BOOL_OUTPUT_TRIP]|
   	       pLogicPublic[GRAPH_UDIFF].Output[BOOL_OUTPUT_TRIP];
    RunTR(&(MY_OPTBS_TIMER1->tMaxTimeRelay[0]),bHz);

    /*人工分闸，闭锁合闸*/
	bHwj = pValSL->dYx&PR_KG_YX_STATUS;
	bTrip = pLogicLow[GRAPH_RELAY].Output[BOOL_OUTPUT_TRIP]|
		    pLogicPublic[GRAPH_SLWY].Output[BOOL_OUTPUT_TRIP]|
		    pLogicPublic[GRAPH_U0].Output[BOOL_OUTPUT_TRIP]|
		    pLogicPublic[GRAPH_ICNT].Output[BOOL_OUTPUT_TRIP];
	RunTR(&(MY_OPTBS_TIMER1->tMaxTimeRelay[1]), bHwj);
	RunTR(&(MY_OPTBS_TIMER1->tMaxTimeRelay[2]), bTrip);

	if((pPublicSet->dYb&&pPublicSet->bYBS)&!bHwj&&MY_OPTBS_TIMER1->tMaxTimeRelay[1].boolTrip&&!MY_OPTBS_TIMER1->tMaxTimeRelay[2].boolTrip)
	{
       MY_OPTBS_MTHZBS = 1;
       pRunPublic->bs_type |= HZ_BS_MF;                          
       if((pRunPublic->dw_NVRAM_BS & PR_HFBS_SF)== 0)
	   {
			pRunPublic->dw_NVRAM_BS |= PR_HFBS_SF;
			extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*)&pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
	   }
#if(TYPE_USER == USER_MULTIMODE)
    if(pRunPublic->yk_type == KG_FZ_CONTROL)
#else
	 if((pRunPublic->yk_type == KG_FZ_CONTROL) || (pRunPublic->yk_type == KG_FZ_REMOTE) )
#endif
	   {
#ifdef  _YIERCI_RH_
		pFdCfg[0].yk_close_dis = 1;
#endif
		}
	   if (pRunPublic->yk_type == 0)
	   	  pRunPublic->yk_type = KG_FZ_SWITCH;
	}
	

		

	/* 双压解锁分闸闭锁 */
	 bDu = FALSE;
	 if((pValSL->dU[0]>pPublicSet->dSLU[0])&&(pValSL->dU[2]>pPublicSet->dSLU[2]))
	     bDu = TRUE;
	 RunTR(&(MY_OPTBS_TIMER1->tMaxTimeRelay[4]), bDu);
	 if(MY_OPTBS_TIMER1->tMaxTimeRelay[4].boolTrip)
	 {
		if (pRunPublic->bs_type && (pPublicSet->byWorkMode == PR_MODE_TIE))    // for nanwang
		{
		    pRunPublic->js_type |= HZ_JS_TIE_DY;
		    pRunPublic->bs_type &= 0;
		}
#if(TYPE_USER == USER_MULTIMODE)
		if(!bHwj)
		{
				if(pLogicPublic[GRAPH_SYBS].Output[BOOL_OUTPUT_HZBS])
				{
					pRunPublic->bs_reset |= (PR_SY_BS|PR_CY_BS);
				}
		}
#endif
	 }
	 
    /*人工合闸,复归闭锁,无需加控制字*/
    bMTHz = FALSE;
	RunTR(&(MY_OPTBS_TIMER1->tMaxTimeRelay[3]), !bHwj);
	if(bHwj && MY_OPTBS_TIMER1->tMaxTimeRelay[3].boolTrip)
       bMTHz = TRUE;
    if( bMTHz && (!MY_OPTBS_TIMER1->tMaxTimeRelay[0].boolTrip))
    {
		  // gu 山东, 合闸保持吸合状态，短暂停电来电后自动合闸。
		plogic->Output[BOOL_OUTPUT_YBS] = FALSE;
		plogic->Output[BOOL_OUTPUT_U0BS] = FALSE;
		pLogicPublic[GRAPH_SUDY].Output[BOOL_OUTPUT_HZBS] = 0;
		pLogicPublic[GRAPH_SUSY].Output[BOOL_OUTPUT_HZBS]  = 0;
		pLogicPublic[GRAPH_SYBS].Output[BOOL_OUTPUT_HZBS] = 0;
		pLogicPublic[GRAPH_LXFZ].Output[BOOL_OUTPUT_HZBS] = 0;
		
	   pRunPublic->bs_reset |= PR_HZBS_JS;
	   if (pRunPublic->bs_type)                     // for nanwang
	   {
	   pRunPublic->js_type  |= HZ_JS_MH;
	   pRunPublic->bs_type = 0;
	   }
#if ((TYPE_USER == USER_GUANGXI)||(TYPE_USER == USER_GUIYANG))
	       ppRunInfo->reset = 0xFFFFFFFF;
#endif
      
       if (pRunPublic->yk_type == 0)
		   	  pRunPublic->yk_type = KG_HZ_SWITCH;

#ifdef  _YIERCI_RH_
		pFdCfg[0].yk_close_dis = 0;
#endif
    }

	// 复归Y闭锁，零压
    if(pRunPublic->bs_reset& PR_OPT_BS)
	{
	   pelm->ppioIn[_PR_122_ENABLE].bVal = ValFALSE;
	   plogic->Output[BOOL_OUTPUT_YBS] = FALSE;
	   plogic->Output[BOOL_OUTPUT_U0BS] = FALSE;
	   MY_OPTBS_MTHZBS = 0;
#ifdef  _YIERCI_RH_
		pFdCfg[0].yk_close_dis = 0;
#endif
	   pRunPublic->bs_reset &= ~PR_OPT_BS;

		if((pRunPublic->dw_NVRAM_BS & (PR_YBS_S |PR_YBS_F|PR_HFBS_SF)) != 0)
		{
			pRunPublic->dw_NVRAM_BS &=~ (PR_YBS_S |PR_YBS_F|PR_HFBS_SF);
			extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*) &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
		}
	}
	else if(pPublicSet->dYb&PR_YB_U_HZ)
       pelm->ppioIn[_PR_122_ENABLE].bVal   = ValTRUE;

    pelm->ppioIn[_PR_122_US].ulVal      = pValSL->dU[0];
	pelm->ppioIn[_PR_122_UF].ulVal      = pValSL->dU[2];
	pelm->ppioIn[_PR_122_US1_PICK].ulVal = pPublicSet->dSLU[0];
	pelm->ppioIn[_PR_122_UF1_PICK].ulVal = pPublicSet->dSLU[2];
	pelm->ppioIn[_PR_122_US2_PICK].ulVal = pPublicSet->dSLUMin[0];
	pelm->ppioIn[_PR_122_UF2_PICK].ulVal = pPublicSet->dSLUMin[2];
	pelm->ppioIn[_PR_122_FAULT].bVal    = pLogicLow[GRAPH_RELAY].Output[BOOL_OUTPUT_FZ]|
		                                  pLogicPublic[GRAPH_SLWY].Output[BOOL_OUTPUT_TRIP]|
		                                  pLogicPublic[GRAPH_U0].Output[BOOL_OUTPUT_TRIP];
	pelm->ppioIn[_PR_122_GL].bVal       = ((ppRunInfo->vyx & (PR_VYX_SGZ|PR_VYX_FZ_BS)) &&pPublicSet->bIProtect)
		                                ||!pPublicSet->bIProtect;
	pelm->ppioIn[_PR_122_U0].bVal       = pLogicPublic[GRAPH_U0].Output[BOOL_OUTPUT_PICK];
	pelm->ppioIn[_PR_122_HZ_IN].bVal    = bMTHz;
	pelm->ppioIn[_PR_122_HZ_IN].bVal    &= MY_OPTBS_TIMER1->tMaxTimeRelay[0].boolTrip;
	pelm->ppioIn[_PR_122_FX].ulVal      = pLogicPublic[GRAPH_SUDY].Output[BOOL_OUTPUT_FX]+
   	                                      pLogicPublic[GRAPH_SUSY].Output[BOOL_OUTPUT_FX];

	if(pRunPublic->dw_NVRAM_BS & PR_YBS_S)
 		pelm->ppioIn[_PR_122_Y_FX].ulVal = 1;
	else if(pRunPublic->dw_NVRAM_BS & PR_YBS_F)
		pelm->ppioIn[_PR_122_Y_FX].ulVal = 2;
	else
		pelm->ppioIn[_PR_122_Y_FX].ulVal = 0;
	
    pelm->Scan_Func(pelm);

	 /*合闸后故障，故障在S侧，则为F侧正向闭锁，故障在F侧，则为S侧正向闭锁*/
     if(pelm->aioOut[_PR_122_Y_BLOCK].bVal)
     {
        plogic->Output[BOOL_OUTPUT_YBS] = TRUE;
		if (pelm->aioOut[_PR_122_BLOCK_FX].ulVal == 2)         // for nanwang
			pRunPublic->bs_type |= HZ_BS_F;
		if (pelm->aioOut[_PR_122_BLOCK_FX].ulVal == 1)   
			pRunPublic->bs_type |= HZ_BS_S;

		if((pRunPublic->dw_NVRAM_BS & (PR_YBS_S |PR_YBS_F))== 0)
	   {

		   if((pPublicSet->bFaTripMode == PR_FA_I_BSANDTRIP)&&pPublicSet->bIProtect&&bHwj)//电流型合故障直接跳闸
		   {
		   		if(!(ppRunInfo->vyx&PR_VYX_FZ_BS))
		   		{
			  		 prSKgDo(pCfgSL->kg_ykno, 0);
			  		 prWritePubEvent(EVENT_I_TRIP);
		   		}
		   }

		 	if(pelm->aioOut[_PR_122_BLOCK_FX].ulVal == 1)
				pRunPublic->dw_NVRAM_BS |= PR_YBS_S;
			if(pelm->aioOut[_PR_122_BLOCK_FX].ulVal  == 2)
				pRunPublic->dw_NVRAM_BS |= PR_YBS_F;
		 	extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*)&pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
	   }
     }
    else if(!pPublicSet->bJsMode)
    {
        plogic->Output[BOOL_OUTPUT_YBS] = FALSE;
		if((pRunPublic->dw_NVRAM_BS & (PR_YBS_S |PR_YBS_F)) != 0)
		{
			pRunPublic->dw_NVRAM_BS &=~ (PR_YBS_S |PR_YBS_F);
			extNvRamSet(pRunPublic->dw_nvram_addr,(BYTE*) &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
		}
    }
	if (pelm->aioOut[_PR_122_U0_BLOCK].bVal)
	{
	   plogic->Output[BOOL_OUTPUT_U0BS] = TRUE;
	   pRunPublic->bs_type |= HZ_BS_U0;
	}
	else if(!pPublicSet->bJsMode)
       plogic->Output[BOOL_OUTPUT_U0BS] = FALSE;
  
    if( (pRunPublic->dw_NVRAM_BS & PR_HFBS_SF) != 0)
	{
		MY_OPTBS_MTHZBS = 1;
		pRunPublic->bs_type |= HZ_BS_MF;
	}
	bs =   plogic->Output[BOOL_OUTPUT_YBS] | plogic->Output[BOOL_OUTPUT_U0BS]|
		   MY_OPTBS_MTHZBS | pLogicPublic[GRAPH_SUDY].Output[BOOL_OUTPUT_HZBS] |
	       pLogicPublic[GRAPH_SUSY].Output[BOOL_OUTPUT_HZBS] |
            pLogicPublic[GRAPH_SYBS].Output[BOOL_OUTPUT_HZBS]|
	       pLogicPublic[GRAPH_SUDY].Output[BOOL_OUTPUT_UHZBS]|
	       pLogicPublic[GRAPH_LXFZ].Output[BOOL_OUTPUT_HZBS];
	
	bs |=  pLogicPublic[GRAPH_SYHW].Output[BOOL_OUTPUT_HZBS];

#ifdef  _YIERCI_RH_
	bs |= pRunPublic->fz_state;
#endif
	
	plogic->Output[BOOL_OUTPUT_HZBS] = bs;
	plogic->Output[BOOL_OUTPUT_UHZBS] = pLogicPublic[GRAPH_SUDY].Output[BOOL_OUTPUT_UHZBS]|pLogicPublic[GRAPH_SUSY].Output[BOOL_OUTPUT_UHZBS] | pLogicPublic[GRAPH_SYBS].Output[BOOL_OUTPUT_UHZBS];
    plogic->Output[BOOL_OUTPUT_FZBS] =  pLogicPublic[GRAPH_FZBS].Output[BOOL_OUTPUT_FZBS];

	if (plogic->Output[BOOL_OUTPUT_HZBS] && !(pRunPublic->vyx&PR_VYXP_HZ_BS))
	{
		pRunPublic->vyx |= PR_VYXP_HZ_BS;
		GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	}
	else if (!plogic->Output[BOOL_OUTPUT_HZBS] && (pRunPublic->vyx&PR_VYXP_HZ_BS))
		pRunPublic->vyx &= ~PR_VYXP_HZ_BS;

	 if (plogic->Output[BOOL_OUTPUT_UHZBS])
	 {	
	 	pRunPublic->bs_type |= HZ_BS_DU;
		pFdCfg[pPublicSet->bySLLine].yk_close_dis = 1 ;
	 }
	 else
	 {
	 	pRunPublic->bs_type &= ~HZ_BS_DU;
		pFdCfg[pPublicSet->bySLLine].yk_close_dis = 0;
	 }
	 
#if (TYPE_USER == USER_BEIJING)
		if(bFK == 1)
		{
			GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
			pRunPublic->vyx |= PR_VYX_FK;
		}
		else
			pRunPublic->vyx &= ~PR_VYX_FK;
#endif

	if (plogic->Output[BOOL_OUTPUT_FZBS] && !(pRunPublic->vyx&PR_VYXP_FZ_BS) && bHwj)
	{
		pRunPublic->vyx |= PR_VYXP_FZ_BS;
		GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	}
	else if (!plogic->Output[BOOL_OUTPUT_FZBS] && (pRunPublic->vyx&PR_VYXP_FZ_BS))
		pRunPublic->vyx &= ~PR_VYXP_FZ_BS;
	
	if (plogic->Output[BOOL_OUTPUT_U0BS] &&  pLogicPublic[GRAPH_U0].Output[BOOL_OUTPUT_PICK] && !(pRunPublic->vyx&PR_VYXP_U0_TRIP)
#ifdef _GUANGZHOU_TEST_VER_
       && !pLogicLow[GRAPH_I1].Output[BOOL_OUTPUT_BS]
#endif
	   )
	{
	    pRunPublic->vyx |= PR_VYXP_U0_TRIP;
		pRunPublic->bs_type |= HZ_BS_U0;       // for
		
	    GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		if (pPublicSet->bU0Trip)
		{
			pLogicPublic[GRAPH_U0].Output[BOOL_OUTPUT_TRIP] = TRUE;
		   prSKgDo(pCfgSL->kg_ykno, 0);
		}
		prWritePubEvent(EVENT_U0);
	}
    else if(!plogic->Output[BOOL_OUTPUT_U0BS]&&(pRunPublic->vyx&PR_VYXP_U0_TRIP))
	{
	    pRunPublic->bs_type &= ~HZ_BS_U0;              // for
	    pRunPublic->vyx &= ~PR_VYXP_U0_TRIP;
    }
}

#define MY_SUDY_TIMER1  ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_SUDY_TIMER2  ((TMAXTIMERELAY*)(plogic->apvUser[1]))
#define MY_SUDY_QD      (plogic->aplUser[0])
#define MY_SUDY_GZDL    (plogic->aplUser[1])  //0代表 无故障，1代表 有故障
#define MY_ZSY_SET (plogic->aplUser[2])  //是否设置了自适应时间
#define MY_HZ_SET (plogic->aplUser[3])  // 是否需要合闸
#define MY_HZ_INIT     (plogic->aplUser[4])   /*北京需要压板，压板液晶传过来，有延时*/

void SUDYLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	plogic->apvUser[1] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
	MY_SUDY_QD = 0;
	MY_SUDY_GZDL = 0;
	MY_ZSY_SET = 0;
	MY_HZ_SET = 0;
	MY_HZ_INIT = 0;
	
    PRLIB_121(pelm);
}

void SUDYLogic(LogicElem *plogic)
{
  	 BOOL bNoU;
	VPrRunInfo *ppRunInfo;
	BOOL bReset = 0;
	 EP_ELEMENT *pelm = &(plogic->elem);

	 if(pPublicSet->bSetChg)
	 {
			InitTR(MY_SUDY_TIMER1, pPublicSet->dTWY, 0);
			InitTR(&(MY_SUDY_TIMER2->tMaxTimeRelay[0]), 0, 500); //从合到分
			InitTR(&(MY_SUDY_TIMER2->tMaxTimeRelay[1]), 1200000, 0); //故障记忆自动复归时间
			MY_SUDY_GZDL = 0;//参数在一开始就决定了，重新下载则重新置数
			bReset = 1;
	 }

#if (TYPE_USER == USER_BEIJING) //若为电磁式开关，则刚上电时无闭锁分位合闸一次
	if(MY_HZ_INIT < 3000) // 3s 因为要等液晶的遥信
		MY_HZ_INIT = MY_HZ_INIT + 4;
	if(((pRunPublic->dw_NVRAM_BS & PR_HZBS) ==0) &&(pPublicSet->dYb&PR_YB_SUDY) && (!(pValSL->dYx & PR_KG_YX_STATUS)) && ( pPublicSet->bKgMode) && (!MY_HZ_SET) &&(MY_HZ_INIT >= 3000))
	{
		prSKgDo(pCfgSL->kg_ykno, 1);
		MY_HZ_SET = 1;
	}
	if(!pPublicSet->bKgMode)
		MY_HZ_SET = 0;
#endif

	 pLogicLow = prLogicElem + pPublicSet->bySLLine*GRAPH_NUM;
	  ppRunInfo = prRunInfo+pPublicSet->bySLLine;
	 
	 bNoU = FALSE;
	 if((pValSL->dU[0] < pPublicSet->dSLUMin[0]) && (pValSL->dU[2] < pPublicSet->dSLUMin[2]))
	       bNoU = TRUE;

	 RunTR(MY_SUDY_TIMER1, bNoU);
#if(TYPE_USER == USER_YUXI)	
	if(pRunPublic->vyx & PR_VYXP_WY_FZ)
		plogic->Output[BOOL_OUTPUT_PICK] = TRUE;
	else if(pLogicPublic[GRAPH_SUSY].Output[BOOL_OUTPUT_PICK])
	 	plogic->Output[BOOL_OUTPUT_PICK] = FALSE;
	 if (pValSL->dYx & PR_KG_YX_STATUS)
	 	plogic->Output[BOOL_OUTPUT_PICK] = FALSE;
	 if(!(pRunPublic->vyx & PR_VYXP_WY_FZ))
		plogic->Output[BOOL_OUTPUT_PICK] = FALSE;
#else
	 if(MY_SUDY_TIMER1->boolTrip || pPublicSet->bSetChg)
	 	plogic->Output[BOOL_OUTPUT_PICK] = TRUE;
	 else if(pLogicPublic[GRAPH_SUSY].Output[BOOL_OUTPUT_PICK])
	 	plogic->Output[BOOL_OUTPUT_PICK] = FALSE;
	 if (pValSL->dYx & PR_KG_YX_STATUS)
	 	plogic->Output[BOOL_OUTPUT_PICK] = FALSE;
#endif
	 
	 if(pRunPublic->bs_reset & (PR_SUDY_BS|PR_DU_BS))
	 {
	    pelm->ppioIn[_PR_121_ENABLE].bVal = ValFALSE;
		plogic->Output[BOOL_OUTPUT_HZBS]  = FALSE;
        plogic->Output[BOOL_OUTPUT_UHZBS]  = FALSE;
		pRunPublic->bs_reset &= ~(PR_SUDY_BS|PR_DU_BS);
		 MY_SUDY_GZDL = 0;
		bReset = 1;
		if((pRunPublic->dw_NVRAM_BS & (PR_XBS_S |PR_XBS_F)) != 0)
		{
			pRunPublic->dw_NVRAM_BS &=~ (PR_XBS_S |PR_XBS_F);
			extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*) &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
		}
	 }
	 else
	    pelm->ppioIn[_PR_121_ENABLE].bVal   = (pPublicSet->dYb&PR_YB_SUDY)>>YB_SUDY;
    
	if(!pPublicSet->bFTUSD)
	{
		pelm->ppioIn[_PR_121_US].ulVal      = pValSL->dU[0];
	 	pelm->ppioIn[_PR_121_UF].ulVal      = pValSL->dU[2];
	}
	else
	{  //  首端FTU ,A,C 区分1,2, A,C  在电源侧
		pelm->ppioIn[_PR_121_US].ulVal      = (pValSL->dU[0] > pValSL->dU[2]) ? pValSL->dU[0] : pValSL->dU[2];
		pelm->ppioIn[_PR_121_UF].ulVal      = 0;
	}
	
	 pelm->ppioIn[_PR_121_QD].bVal       = plogic->Output[BOOL_OUTPUT_PICK];
	 pelm->ppioIn[_PR_121_BLOCK].bVal    = pLogicPublic[GRAPH_OPTBS].Output[BOOL_OUTPUT_HZBS]|
	 	                                   pLogicPublic[GRAPH_SYBS].Output[BOOL_OUTPUT_HZBS]|
	 	                                   pLogicPublic[GRAPH_OPTBS].Output[BOOL_OUTPUT_UHZBS]|
	 	                                   pLogicPublic[GRAPH_SYBS].Output[BOOL_OUTPUT_UHZBS]|
	 	                                    pLogicPublic[GRAPH_LXFZ].Output[BOOL_OUTPUT_HZBS];

     pelm->ppioIn[_PR_121_BLOCK].bVal |= pLogicPublic[GRAPH_SYHW].Output[BOOL_OUTPUT_HZBS];
	 pelm->ppioIn[_PR_121_US1_PICK].ulVal = pPublicSet->dSLU[0];
	 pelm->ppioIn[_PR_121_UF1_PICK].ulVal = pPublicSet->dSLU[2];
	 pelm->ppioIn[_PR_121_US2_PICK].ulVal = pPublicSet->dSLUMin[0];
	 pelm->ppioIn[_PR_121_UF2_PICK].ulVal = pPublicSet->dSLUMin[2];
	 
	 if(pRunPublic->dw_NVRAM_BS & PR_XBS_S)
	 	pelm->ppioIn[_PR_121_XBS_FX].ulVal = 1;
	 else if(pRunPublic->dw_NVRAM_BS & PR_XBS_F)
	 	pelm->ppioIn[_PR_121_XBS_FX].ulVal = 2;
	 else
		 pelm->ppioIn[_PR_121_XBS_FX].ulVal = 0;


	 if(((pPublicSet->bDXJD) && (ppRunInfo->vyx & PR_VYX_I0_MASK )) ||((pPublicSet->bXJDL) && (ppRunInfo->vyx & PR_VYX_I_MASK)))
	 {
		 MY_SUDY_GZDL = 1;
	 }
	 
   RunTR(&(MY_SUDY_TIMER2->tMaxTimeRelay[1]), MY_SUDY_GZDL);
   if(MY_SUDY_TIMER2->tMaxTimeRelay[1].boolTrip)
   {
		 MY_SUDY_GZDL = 0;
		 if(pelm->ppioIn[_PR_121_T_ZSYHZ].ulVal == pPublicSet->dTX) //短延时则复归后改为短延时
				bReset = 1;
	 }		 
	 
	 // 从合到分过程中 是否有故障，有则 取X，无则取S
	 RunTR(&(MY_SUDY_TIMER2->tMaxTimeRelay[0]), pValSL->dYx & PR_KG_YX_STATUS);
	 if((((!(pValSL->dYx & PR_KG_YX_STATUS)) && (MY_SUDY_TIMER2->tMaxTimeRelay[0].boolTrip) ) || bReset)&& ((pPublicSet->bDXJD) || (pPublicSet->bXJDL))) //看是否有记忆
	 {
		 if(MY_SUDY_GZDL)
			 pelm->ppioIn[_PR_121_T_ZSYHZ].ulVal = pPublicSet->dTX;
		 else
			 pelm->ppioIn[_PR_121_T_ZSYHZ].ulVal = pPublicSet->dTS;
		 if(MY_ZSY_SET == 0)
		 {
			 MY_ZSY_SET = 1;
			 pelm->bSetChg = TRUE;
			 if(MY_SUDY_GZDL)
			 	shellprintf("有故障记忆 取X  \n");
			 else
			 	shellprintf("无故障记忆取S  \n");
		 }
	 }
	 else
		 MY_ZSY_SET = 0;
	 
	 pelm->Scan_Func(pelm);

	 plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_121_OPERATE].bVal;
	 plogic->Output[BOOL_OUTPUT_UHZBS] = pelm->aioOut[_PR_121_U_BLOCK].bVal;

	 /*合闸前故障，S侧故障为S侧反向，F侧故障为F侧反向*/
     if (pelm->aioOut[_PR_121_X_BLOCK].bVal)
     {
         plogic->Output[BOOL_OUTPUT_HZBS]  = TRUE;
		 if (pelm->aioOut[_PR_121_BLOCK_FX].ulVal== 1)              // for
		 	pRunPublic->bs_type |= HZ_BS_SR;
		 else if(pelm->aioOut[_PR_121_BLOCK_FX].ulVal== 2)
		 	pRunPublic->bs_type |= HZ_BS_FR;
		if((pRunPublic->dw_NVRAM_BS & (PR_XBS_S |PR_XBS_F))== 0)
	   {
			 if(pelm->aioOut[_PR_121_BLOCK_FX].bVal == 1)
			 	pRunPublic->dw_NVRAM_BS |= PR_XBS_S;
			 if(pelm->aioOut[_PR_121_BLOCK_FX].bVal == 2)
			 	pRunPublic->dw_NVRAM_BS |= PR_XBS_F;
			 extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*) &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
		}
     }
     else if (!pPublicSet->bJsMode)
     {
          plogic->Output[BOOL_OUTPUT_HZBS]  = FALSE;
		  	if((pRunPublic->dw_NVRAM_BS & (PR_XBS_S |PR_XBS_F)) != 0)
			{
				pRunPublic->dw_NVRAM_BS &=~ (PR_XBS_S |PR_XBS_F);
				extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*) &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
			}
     }

	if(pelm->aioOut[_PR_121_X_FX].bVal) //防止x_fx 多次被赋值
	 	pRunPublic->x_fx = pelm->aioOut[_PR_121_X_FX].bVal;
	 
	 plogic->Output[BOOL_OUTPUT_FX]   = pelm->aioOut[_PR_121_FX].bVal;

	 if(plogic->Output[BOOL_OUTPUT_TRIP] && !(pRunPublic->vyx&PR_VYXP_S_HZ))
	 {
	    pRunPublic->vyx |= PR_VYXP_S_HZ;
		GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		prSKgDo(pCfgSL->kg_ykno, 1);
		prWritePubEvent(EVENT_DY_HZ);
		 MY_SUDY_GZDL = 0;
	}
	 
	 else if(!plogic->Output[BOOL_OUTPUT_TRIP] && (pRunPublic->vyx&PR_VYXP_S_HZ))
	 {
	    pRunPublic->vyx &= ~PR_VYXP_S_HZ;
	 }

	 if(!(pPublicSet->dYb&PR_YB_U_HZ))
		return;
}



#define MY_SUSY_TIMER1 ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_SUSY_QD     (plogic->aplUser[0])
void SUSYLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	MY_SUSY_QD = 0;

	PRLIB_121(pelm);
}


void SUSYLogic(LogicElem *plogic)
{
	 BOOL bDu;

	 EP_ELEMENT *pelm = &(plogic->elem);

	 if(pPublicSet->bSetChg)
	    InitTR(MY_SUSY_TIMER1, pPublicSet->dTY, 0);

	 pLogicLow = prLogicElem + pPublicSet->bySLLine*GRAPH_NUM;

     bDu = FALSE;
	 if((pValSL->dU[0]>pPublicSet->dSLU[0])&&(pValSL->dU[2]>pPublicSet->dSLU[2]))
	     bDu = TRUE;
	 RunTR(MY_SUSY_TIMER1, bDu&&!(pValSL->dYx&PR_KG_YX_STATUS));
	 if(MY_SUSY_TIMER1->boolTrip && ( (pPublicSet->dYb&PR_YB_SUSY)>>YB_SUSY))
	 	plogic->Output[BOOL_OUTPUT_PICK] = TRUE;
	 else if(pLogicPublic[GRAPH_SUDY].Output[BOOL_OUTPUT_PICK])
	 	plogic->Output[BOOL_OUTPUT_PICK] = FALSE;
	 if (pValSL->dYx & PR_KG_YX_STATUS)
	 	plogic->Output[BOOL_OUTPUT_PICK] = FALSE;

     if(pRunPublic->bs_reset & PR_SUSY_BS)
	 {
	    pelm->ppioIn[_PR_121_ENABLE].bVal = ValFALSE;
		plogic->Output[BOOL_OUTPUT_HZBS]  = FALSE;
		pRunPublic->bs_reset &= ~PR_SUSY_BS;
	 }
	 else
	    pelm->ppioIn[_PR_121_ENABLE].bVal   = (pPublicSet->dYb&PR_YB_SUSY)>>YB_SUSY;
     pelm->ppioIn[_PR_121_US].ulVal      = pValSL->dU[0];
	 pelm->ppioIn[_PR_121_UF].ulVal      = pValSL->dU[2];
	 pelm->ppioIn[_PR_121_QD].bVal       = plogic->Output[BOOL_OUTPUT_PICK];
	 pelm->ppioIn[_PR_121_BLOCK].bVal    = pLogicPublic[GRAPH_OPTBS].Output[BOOL_OUTPUT_HZBS]|
	 	                                   pLogicPublic[GRAPH_SYBS].Output[BOOL_OUTPUT_HZBS]|
	 	                                  pLogicPublic[GRAPH_OPTBS].Output[BOOL_OUTPUT_UHZBS]|
	 	                                  pLogicPublic[GRAPH_SYBS].Output[BOOL_OUTPUT_UHZBS]|
	 	                                  pLogicPublic[GRAPH_LXFZ].Output[BOOL_OUTPUT_HZBS];
	 pelm->ppioIn[_PR_121_US1_PICK].ulVal = pPublicSet->dSLU[0];
	 pelm->ppioIn[_PR_121_UF1_PICK].ulVal = pPublicSet->dSLU[2];
	 pelm->ppioIn[_PR_121_US2_PICK].ulVal = pPublicSet->dSLUMin[0];
	 pelm->ppioIn[_PR_121_UF2_PICK].ulVal = pPublicSet->dSLUMin[2];
	 pelm->ppioIn[_PR_121_GZDL].ulVal  = 0;
	 pelm->ppioIn[_PR_121_GZDL].bVal  = 0;
	
	 pelm->Scan_Func(pelm);

     plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_121_OPERATE].bVal;
	 #if(TYPE_USER == USER_GUANGXI)
	 plogic->Output[BOOL_OUTPUT_UHZBS] 	 = pelm->aioOut[_PR_121_U_BLOCK].bVal ||(pPublicSet->bDUBS && bDu);
	 #else
	 plogic->Output[BOOL_OUTPUT_UHZBS] 	 = pelm->aioOut[_PR_121_U_BLOCK].bVal;
	 #endif
	 
	 if (pelm->aioOut[_PR_121_X_BLOCK].bVal)
	 	plogic->Output[BOOL_OUTPUT_HZBS]  = TRUE;
	 else if (!pPublicSet->bJsMode)
	 	plogic->Output[BOOL_OUTPUT_HZBS]  = FALSE;
	 
	if(pelm->aioOut[_PR_121_X_FX].bVal) //防止x_fx 多次被赋值
		 pRunPublic->x_fx = pelm->aioOut[_PR_121_X_FX].bVal;
	
	 plogic->Output[BOOL_OUTPUT_FX]   = pelm->aioOut[_PR_121_FX].bVal;

	 if(plogic->Output[BOOL_OUTPUT_TRIP] && !(pRunPublic->vyx&PR_VYXP_L_HZ))
	 {
	    pRunPublic->vyx |= PR_VYXP_L_HZ;
		GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);

#if(TYPE_USER == USER_GUIYANG)
		if(0  == pPublicSet->bSUSYMode)
#endif
			prSKgDo(pCfgSL->kg_ykno, 1);
		prWritePubEvent(EVENT_SY_HZ);

	 }
	 else if(!plogic->Output[BOOL_OUTPUT_TRIP] && (pRunPublic->vyx&PR_VYXP_L_HZ))
	 {
	    pRunPublic->vyx &= ~PR_VYXP_L_HZ;
	 }

}

void SYBSLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);
    PRLIB_123(pelm);
}

void SYBSLogic(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);


	if(pRunPublic->bs_reset & (PR_SY_BS | PR_CY_BS))
	{
	   pelm->ppioIn[_PR_123_ENABLE].bVal = ValFALSE;
	   plogic->Output[BOOL_OUTPUT_HZBS]  = FALSE;

		 pRunPublic->bs_reset &= ~(PR_SY_BS|PR_CY_BS);
		if((pRunPublic->dw_NVRAM_BS & (PR_CYBS_S |PR_CYBS_F)) != 0)
		{
			pRunPublic->dw_NVRAM_BS &=~ (PR_CYBS_S |PR_CYBS_F);
			extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*) &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
		}
		if((pRunPublic->dw_NVRAM_BS & (PR_SYBS_S |PR_SYBS_F)) != 0)
		{
			pRunPublic->dw_NVRAM_BS &=~ (PR_SYBS_S |PR_SYBS_F);
			extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*) &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
		}
		pRunPublic->x_fx = 0;
	}
	else
   	   pelm->ppioIn[_PR_123_ENABLE].bVal    = pPublicSet->bSYBS&&(!(pValSL->dYx&PR_KG_YX_STATUS));

    pelm->ppioIn[_PR_123_US].ulVal       = pValSL->dU[0];
	pelm->ppioIn[_PR_123_UF].ulVal       = pValSL->dU[2];
	pelm->ppioIn[_PR_123_US1_PICK].ulVal = pPublicSet->dSLU[0];
	pelm->ppioIn[_PR_123_US2_PICK].ulVal = pPublicSet->dUCY[0];
	pelm->ppioIn[_PR_123_UF1_PICK].ulVal = pPublicSet->dSLU[2];
	pelm->ppioIn[_PR_123_UF2_PICK].ulVal = pPublicSet->dUCY[2];

	if(pRunPublic->dw_NVRAM_BS & (PR_CYBS_S |PR_SYBS_S))
	{
 		pelm->ppioIn[_PR_123_FX].ulVal = 1;
	}
	else if(pRunPublic->dw_NVRAM_BS & (PR_CYBS_F | PR_SYBS_F))
	{
		pelm->ppioIn[_PR_123_FX].ulVal = 2;
	}
	else
		pelm->ppioIn[_PR_123_FX].ulVal = 0;

	if((pRunPublic->x_fx == 0) && (pPublicSet->bSetChg)) //初次上电瞬压闭锁保持，不考虑两个都有的情况
	{
		if(pRunPublic->dw_NVRAM_BS & PR_SYBS_S)
			pRunPublic->x_fx  = 2;
		if(pRunPublic->dw_NVRAM_BS & PR_SYBS_F)
			pRunPublic->x_fx  = 1;
	}
	
	if(pRunPublic->dw_NVRAM_BS & (PR_CYBS_S | PR_CYBS_F))
		pelm->ppioIn[_PR_123_X_FX].ulVal  = 0;
	else
		pelm->ppioIn[_PR_123_X_FX].ulVal = pRunPublic->x_fx;

    pelm->Scan_Func(pelm);

    if(pelm->aioOut[_PR_123_SY_BLOCK].bVal|pelm->aioOut[_PR_123_CY_BLOCK].bVal)
	{
	   plogic->Output[BOOL_OUTPUT_HZBS] = TRUE;
	   if (pelm->aioOut[_PR_123_SY_BLOCK].bVal)
	   {
	        pRunPublic->bs_type |= HZ_BS_SY;
			if((pRunPublic->dw_NVRAM_BS & (PR_SYBS_S |PR_SYBS_F))== 0)
			{
				if(pelm->aioOut[_PR_123_OUTPUT_FX].ulVal == 1)
				{
					pRunPublic->dw_NVRAM_BS |= PR_SYBS_S;
					#if(TYPE_USER == USER_GUANGXI)
						pRunPublic->bs_type |= HZ_BS_SR;
					#endif
				}
				if(pelm->aioOut[_PR_123_OUTPUT_FX].ulVal  == 2)
				{
					pRunPublic->dw_NVRAM_BS |= PR_SYBS_F;
					#if(TYPE_USER == USER_GUANGXI)
						pRunPublic->bs_type |= HZ_BS_FR;
					#endif
				}
				extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*) &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
			}
	   }
	   if (pelm->aioOut[_PR_123_CY_BLOCK].bVal)
	   {
			pRunPublic->bs_type |= HZ_BS_CY;
			if((pRunPublic->dw_NVRAM_BS & (PR_CYBS_S |PR_CYBS_F))== 0)
			{
				if(pelm->aioOut[_PR_123_OUTPUT_FX].ulVal == 1)
				{
					pRunPublic->dw_NVRAM_BS |= PR_CYBS_S;
					#if(TYPE_USER == USER_GUANGXI)
						pRunPublic->bs_type |= HZ_BS_SR;
					#endif
				}
				if(pelm->aioOut[_PR_123_OUTPUT_FX].ulVal  == 2)
				{
					pRunPublic->dw_NVRAM_BS |= PR_CYBS_F;
					#if(TYPE_USER == USER_GUANGXI)
						pRunPublic->bs_type |= HZ_BS_FR;
					#endif
				}
				extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*) &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
			}
	   }
    }
    else if(!pPublicSet->bJsMode)
    {
       plogic->Output[BOOL_OUTPUT_HZBS] = FALSE;
	   pRunPublic->bs_type &= ~(HZ_BS_SY|HZ_BS_CY);              //for

		if((pRunPublic->dw_NVRAM_BS & (PR_CYBS_S |PR_CYBS_F)) != 0)
		{
			pRunPublic->dw_NVRAM_BS &=~ (PR_CYBS_S |PR_CYBS_F);
			extNvRamSet(pRunPublic->dw_nvram_addr,(BYTE*)  &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
		}
		if((pRunPublic->dw_NVRAM_BS & (PR_SYBS_S |PR_SYBS_F)) != 0)
		{
			pRunPublic->dw_NVRAM_BS &=~ (PR_SYBS_S |PR_SYBS_F);
			extNvRamSet(pRunPublic->dw_nvram_addr,(BYTE*)  &pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
		}
    }
	if(!pLogicPublic[GRAPH_SUDY].elem.aioOut[_PR_121_X_FX].bVal && (!pLogicPublic[GRAPH_SUSY].elem.aioOut[_PR_121_X_FX].bVal)
		&& ((pRunPublic->dw_NVRAM_BS & (PR_CYBS_S |PR_CYBS_F |PR_SYBS_S |PR_SYBS_F)) == 0))
		pRunPublic->x_fx  = 0;
}



#define MY_U0_TIMER1   ((TIMERELAY*)(plogic->apvUser[0]))
#if(TYPE_USER == USER_GUANGXI)
	#define MY_U0_TIMER2   ((TMAXTIMERELAY*)(plogic->apvUser[1]))
#else
	#define MY_U0_TIMER2   ((TIMERELAY*)(plogic->apvUser[1]))
#endif
void U0Logic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));	
#if(TYPE_USER == USER_GUANGXI)
	plogic->apvUser[1] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
#else
	plogic->apvUser[1] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
#endif
}


void U0Logic(LogicElem *plogic)
{
	 BOOL bU0 = FALSE;
#if(TYPE_USER == USER_GUANGXI)
	 BOOL bHz;
     BOOL kgStatus;
#endif

	 if(pPublicSet->bSetChg)
	 {
	    InitTR(MY_U0_TIMER1, 500+pPublicSet->dTU0, 0);
		 #if(TYPE_USER == USER_GUANGXI)
			InitTR(&(MY_U0_TIMER2->tMaxTimeRelay[0]), 0, 500+pPublicSet->dTY); 
		  InitTR(&MY_U0_TIMER2->tMaxTimeRelay[1], 0, 500+pPublicSet->dTY);
		 #endif
	 }
	 if(!(pPublicSet->dYb&PR_YB_U0))
	 	return;

   
	if(pValSL->dU[3] > pPublicSet->dU0)
	 	bU0 = TRUE;
  #if(TYPE_USER == USER_GUANGXI)
     bHz =  pLogicPublic[GRAPH_SUDY].Output[BOOL_OUTPUT_TRIP]|
			 pLogicPublic[GRAPH_SUSY].Output[BOOL_OUTPUT_TRIP]|
			 pLogicPublic[GRAPH_UDIFF].Output[BOOL_OUTPUT_TRIP];
  
	 kgStatus = pValSL->dYx & PR_KG_YX_STATUS;
	 RunTR(&(MY_U0_TIMER2->tMaxTimeRelay[0]), bU0&&!kgStatus);
	 RunTR(&(MY_U0_TIMER2->tMaxTimeRelay[1]), !MY_U0_TIMER2->tMaxTimeRelay[0].boolTrip&&bHz);

	 if(MY_U0_TIMER2->tMaxTimeRelay[1].boolTrip&&(pValSL->dU[3] > pPublicSet->dU0))
	  	bU0 = TRUE;
	 else
		bU0 = FALSE;
 #endif
 
	 RunTR(MY_U0_TIMER1, bU0);
	 plogic->Output[BOOL_OUTPUT_PICK] = MY_U0_TIMER1->boolTrip; 

	 if (!plogic->Output[BOOL_OUTPUT_PICK] && plogic->Output[BOOL_OUTPUT_TRIP])
	 {
	    plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
		pRunPublic->vyx &= ~PR_VYXP_U0_TRIP;
	 }
}

void UDiffLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);
    PRLIB_124(pelm);

}

void UDiffLogic(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	pelm->ppioIn[_PR_124_ENABLE].bVal = (pPublicSet->dYb&PR_YB_UDIFF)>>YB_UDIFF;
	pelm->ppioIn[_PR_124_US].lVal = pValSL->dU[0];
	pelm->ppioIn[_PR_124_UF].lVal = pValSL->dU[2];
	pelm->ppioIn[_PR_124_US_PICK].lVal = pPublicSet->dSLU[0];
	pelm->ppioIn[_PR_124_UF_PICK].lVal = pPublicSet->dSLU[2];
	pelm->ppioIn[_PR_124_BLOCK].bVal = pLogicPublic[GRAPH_OPTBS].Output[BOOL_OUTPUT_HZBS]|
		                               pLogicPublic[GRAPH_SYBS].Output[BOOL_OUTPUT_HZBS];
	pelm->ppioIn[_PR_124_SINDEX].lVal = pCfgSL->mmi_meaNo_ai[MMI_MEA_UA];
	pelm->ppioIn[_PR_124_FINDEX].lVal = pCfgSL->mmi_meaNo_ai[MMI_MEA_UC];
	pelm->Scan_Func(pelm);
	plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_124_OPERATE].bVal;

	if(plogic->Output[BOOL_OUTPUT_TRIP] && !(pRunPublic->vyx&PR_VYXP_U_MERGE))
	{
	   pRunPublic->vyx |= PR_VYXP_U_MERGE;
	   GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	   prSKgDo(pCfgSL->kg_ykno, 1);
	   prWritePubEvent(EVENT_UDIFF);
	   
	}
	else if(!plogic->Output[BOOL_OUTPUT_TRIP] && (pRunPublic->vyx&PR_VYXP_U_MERGE))
	{
	   pRunPublic->vyx &= ~PR_VYXP_U_MERGE;
	}

	pRunPublic->su = pelm->aioOut[_PR_124_S].bVal;
	pRunPublic->fu = pelm->aioOut[_PR_124_F].bVal;
	
}

#define MY_IDL_TIMER1   ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_IDL_TIMER2   ((TIMERELAY*)(plogic->apvUser[1]))
#define MY_IDL_CNT      (plogic->aplUser[0])
#define MY_LAST_STS     (plogic->aplUser[1])
#define MY_IDL_START    (plogic->aplUser[2])
void ICntLogic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	plogic->apvUser[1] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	MY_IDL_CNT  = 0;
	MY_LAST_STS = 0;
	MY_IDL_START = 0;
}


void ICntLogic(LogicElem *plogic)
{
	 if (!(pPublicSet->dYb&PR_YB_I_DL))
	 	return;

	 if (pPublicSet->bSetChg)
	 {
		InitTR(MY_IDL_TIMER1, pPublicSet->dTIDl1, 0);
		InitTR(MY_IDL_TIMER2, pPublicSet->dTIDl2, 0);
	 }

	 pLogicLow = prLogicElem + pPublicSet->bySLLine*GRAPH_NUM;
	 
	 if (pLogicLow[GRAPH_RELAY].Output[BOOL_OUTPUT_ISGZ]&!MY_LAST_STS)
	 {
	    MY_IDL_CNT++;
        MY_IDL_START = 1;
	 }

	 if (MY_IDL_CNT >= pPublicSet->dICnt)
	 	plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;

	 if(plogic->Output[BOOL_OUTPUT_TRIP] && pLogicPublic[GRAPH_SLWY].Output[BOOL_OUTPUT_PICK] &&
	 	!pLogicLow[GRAPH_RELAY].Output[BOOL_OUTPUT_ISGZ] && !(pRunPublic->vyx&PR_VYXP_I_CNT))
	 {
	    pRunPublic->vyx |= PR_VYXP_I_CNT;
		GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		   prSKgDo(pCfgSL->kg_ykno, 0);
		   prWritePubEvent(EVENT_I_CNT);

	 } 
	 else if(!plogic->Output[BOOL_OUTPUT_TRIP] && (pRunPublic->vyx&PR_VYXP_I_CNT))
	 {
	    pRunPublic->vyx &= ~PR_VYXP_I_CNT;
	 }

	 MY_LAST_STS = pLogicLow[GRAPH_RELAY].Output[BOOL_OUTPUT_ISGZ];

	 RunTR(MY_IDL_TIMER1, (MY_IDL_CNT!=0)&&!MY_LAST_STS);
	 if (MY_IDL_TIMER1->boolTrip)
	 {
	    MY_IDL_CNT = 0;
		MY_IDL_START = 0;
		plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	 }
	 
	 RunTR(MY_IDL_TIMER2, MY_IDL_START);    
	 if (MY_IDL_TIMER2->boolTrip)
	 {
	     MY_IDL_CNT = 0;
		 MY_IDL_START = 0;
		 plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	 }
}

#define MY_SY_YXA_TICK    (plogic->aplUser[0])
#define MY_SY_YXC_TICK    (plogic->aplUser[1])
#define MY_SY_HW_INIT     (plogic->aplUser[2])
#define MY_SY_YXA_CNT     (plogic->aplUser[3])
#define MY_SY_YXC_CNT     (plogic->aplUser[4])
#define MY_SY_HW_RESET    (plogic->aplUser[5])
#define MY_SY_HW_TIMER1   ((TMAXTIMERELAY*)(plogic->apvUser[0]))

static int hw_cnt = 0;

int GetHWYx(int i) //0 、1为两侧
{
  int ret = 0;
  if(i == 0)
  {
#if ((DEV_SP == DEV_SP_WDG) || (DEV_SP == DEV_SP_FTU))
	ret = GetDiValue(3);
#endif    
#if(TYPE_USER == USER_GUANGXI)
	ret = GetDiValue(4);
#endif
  }
  else
  {
#if ((DEV_SP == DEV_SP_WDG) || (DEV_SP == DEV_SP_FTU))
	ret = GetDiValue(4);
#endif       
#if(TYPE_USER == USER_GUANGXI)
	ret = GetDiValue(5);
#endif
  }
  return ret;
}

// 这里用的储能遥控
void ResetHwSY(void)
{
    int ret;
	
#if(DEV_SP == DEV_SP_WDG)
	if(GetHWYx(1) || GetHWYx(0))
	{
		if(g_wdg_ver)
		{
			SET_GPIO(GPIOC, GPIO_Pin_3);
		}
		else
		{
			ret = ykOutput(2, 1);		
			if(2 == ret)
			{
				ykCancel(-1,2, 1);     
				ykCancel(-1,2, 0);
				ret = ykOutput(2, 1);
			}
		}
	}
#elif(DEV_SP == DEV_SP_FTU)
	if(GetHWYx(1) ||GetHWYx(0))
	{
		ret = ykOutput(2, 1);		
		if(2 == ret)
		{
			ykCancel(-1,2, 1);     
			ykCancel(-1,2, 0);
			ret = ykOutput(2, 1);
		}
	}
#elif(TYPE_USER == USER_GUANGXI)
	if(GetHWYx(1) ||GetHWYx(0))
	{
		ret = ykOutput(2, 1);		
		if(2 == ret)
		{
			ykCancel(-1,2, 1);     
			ykCancel(-1,2, 0);
			ret = ykOutput(2, 1);
		}
	}

#endif

}

void HwSYLogic_Init(LogicElem *plogic)
{
    MY_SY_YXA_CNT = MY_SY_YXC_CNT = 0;
	MY_SY_HW_INIT = 2;
	MY_SY_YXA_TICK = MY_SY_YXC_TICK = 0;
	MY_SY_HW_RESET = 0;
    plogic->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
}

void HwSYLogic(LogicElem *plogic)
{
    BOOL bUa, bUc;

	if(pRunPublic->bs_reset & PR_DCDY_BS )
	{
		MY_SY_YXA_CNT = 0;  MY_SY_YXC_CNT = 0;
		pRunPublic->bs_reset &= ~PR_DCDY_BS;
		plogic->Output[BOOL_OUTPUT_HZBS] = FALSE;
		pRunPublic->bs_type &= ~HZ_BS_CH;
         MY_SY_HW_RESET = 1;
		ResetHwSY();
	}
	
	if (pPublicSet->bSetChg)
	{
		InitTR(&(MY_SY_HW_TIMER1->tMaxTimeRelay[0]), pPublicSet->dTDYLJ,0); //设为参数
		InitTR(&(MY_SY_HW_TIMER1->tMaxTimeRelay[1]), pPublicSet->dTDYLJ,0);
		InitTR(&(MY_SY_HW_TIMER1->tMaxTimeRelay[2]), 15000,0);
		InitTR(&(MY_SY_HW_TIMER1->tMaxTimeRelay[3]), 0, 5000);
		InitTR(&(MY_SY_HW_TIMER1->tMaxTimeRelay[4]), 10000,0);
	}
	RunTR(&(MY_SY_HW_TIMER1->tMaxTimeRelay[2]), (MY_SY_HW_RESET==1) );
	if(MY_SY_HW_TIMER1->tMaxTimeRelay[2].boolTrip )
	{
		MY_SY_HW_RESET = 0;
		//shellprintf("复位结束 %d %d \n", MY_SY_YXA_CNT,MY_SY_YXC_CNT);
#if(DEV_SP == DEV_SP_WDG)
		if(g_wdg_ver)
		{
			CLEAR_GPIO(GPIOC, GPIO_Pin_3);   
		}
#endif
	}
	if (!(pPublicSet->dYb& PR_YB_YJCY))
		return;
      
	RunTR(&(MY_SY_HW_TIMER1->tMaxTimeRelay[4]), (MY_SY_HW_INIT!=0) );

	hw_cnt++;
	

    bUa = bUc = FALSE;
	if (pValSL->dU[0] > pPublicSet->dSLU[0])
		bUa = TRUE;
	if (pValSL->dU[2] > pPublicSet->dSLU[2])
		bUc = TRUE;

	if (MY_SY_HW_INIT)
    {
	   if( MY_SY_HW_TIMER1->tMaxTimeRelay[4].boolTrip )
	   {
       if (GetHWYx(0) & !bUa)
	   	  MY_SY_YXA_TICK++;
	   if (MY_SY_YXA_TICK >= 2)
	       pRunPublic->dw_NVRAM_BS |= PR_CYBS_S;
	   if (GetHWYx(1) & !bUc)
	   	  MY_SY_YXC_TICK++;
	   if (MY_SY_YXC_TICK >= 2)
	   	   pRunPublic->dw_NVRAM_BS |= PR_CYBS_F;
	   MY_SY_HW_INIT--;
	   if (MY_SY_HW_INIT == 0)
	   {
	       MY_SY_YXA_TICK = 0;
		   MY_SY_YXC_TICK = 0;
			   if( pRunPublic->dw_NVRAM_BS & (PR_CYBS_S|PR_CYBS_F))
			        pRunPublic->bs_type |= HZ_BS_CY;
	       if (pRunPublic->dw_NVRAM_BS & (PR_CYBS_F|PR_CYBS_S))
	           extNvRamSet(pRunPublic->dw_nvram_addr, (BYTE*)&pRunPublic->dw_NVRAM_BS, sizeof(pRunPublic->dw_NVRAM_BS));
			   MY_SY_HW_RESET = 1;
			   ResetHwSY();
		   }
	   }
	}
	else
	{
	    if (GetHWYx(0) && (MY_SY_HW_RESET==0))
			MY_SY_YXA_TICK++;
		if (GetHWYx(1) && (MY_SY_HW_RESET==0))
			MY_SY_YXC_TICK++;
		if ((MY_SY_YXA_TICK >= 2)||(MY_SY_YXC_TICK >= 2))
		{
		    if (MY_SY_YXA_TICK >= 2)
				MY_SY_YXA_CNT++;
			if (MY_SY_YXC_TICK >= 2)
				MY_SY_YXC_CNT++;

			MY_SY_HW_RESET = 1;
		    ResetHwSY();
			MY_SY_YXA_TICK = 0;
			MY_SY_YXC_TICK = 0;
		}
		RunTR(&(MY_SY_HW_TIMER1->tMaxTimeRelay[0]), (MY_SY_YXA_CNT!=0));
		RunTR(&(MY_SY_HW_TIMER1->tMaxTimeRelay[1]), (MY_SY_YXC_CNT!=0));
		if( MY_SY_HW_TIMER1->tMaxTimeRelay[0].boolTrip )
			MY_SY_YXA_CNT = 0;
		if( MY_SY_HW_TIMER1->tMaxTimeRelay[1].boolTrip)
			MY_SY_YXC_CNT = 0;

		if ( (MY_SY_YXA_CNT >= pPublicSet->dDYLJ) || (MY_SY_YXC_CNT >= pPublicSet->dDYLJ) )
		{
			plogic->Output[BOOL_OUTPUT_HZBS] = TRUE;	
			pRunPublic->bs_type |= HZ_BS_CH;		
		}
	}
}


#define MY_LXFZ_RESET         (plogic->aplUser[1])
#define MY_LXFZ_CNT           (plogic->aplUser[2])
#define MY_LXFZ_HW_RESET      (plogic->aplUser[3])

#define MY_LXFZ_TIMER1   ((TMAXTIMERELAY*)(plogic->apvUser[0]))


void LXFZLogic_Init(LogicElem *plogic)
{
    MY_LXFZ_RESET = 0;
	MY_LXFZ_CNT = 0;
    plogic->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
}

void LXFZLogic(LogicElem *plogic)
{
	BOOL bHwj;


	if (!(pPublicSet->dYb & PR_YB_LXFZ))
		return;

	if(pRunPublic->bs_reset & PR_LXFZ_BS )
	{
		MY_LXFZ_CNT = 0;
		pRunPublic->bs_reset &= ~PR_LXFZ_BS;
		plogic->Output[BOOL_OUTPUT_HZBS] = FALSE;
		pRunPublic->bs_type &= ~HZ_BS_LXFZ;
		MY_LXFZ_RESET = 1;
	}

	if (pPublicSet->bSetChg)
	{
		InitTR(&(MY_LXFZ_TIMER1->tMaxTimeRelay[0]), pPublicSet->dTLXFZ,0); //设为参数
		InitTR(&(MY_LXFZ_TIMER1->tMaxTimeRelay[1]), 0,5000);
		InitTR(&(MY_LXFZ_TIMER1->tMaxTimeRelay[2]), 15000,0);
	//	InitTR(&(MY_LXFZ_TIMER1->tMaxTimeRelay[3]), 0, 5000);
	//	InitTR(&(MY_LXFZ_TIMER1->tMaxTimeRelay[4]), 10000,0);
	}

	RunTR(&(MY_LXFZ_TIMER1->tMaxTimeRelay[2]), (MY_LXFZ_RESET == 1) );
	if(MY_LXFZ_TIMER1->tMaxTimeRelay[2].boolTrip )
	{
		MY_LXFZ_RESET = 0;
	}
 //   if (GetHWYx(0) && (MY_SY_HW_RESET==0))
 //		MY_LXFZ_CNT++;

	bHwj = pValSL->dYx&PR_KG_YX_STATUS;

	RunTR(&(MY_LXFZ_TIMER1->tMaxTimeRelay[1]), bHwj);
	
	if(!bHwj&&(0 == MY_LXFZ_RESET)&&MY_LXFZ_TIMER1->tMaxTimeRelay[1].boolTrip)
	{
		MY_LXFZ_CNT++;
		ResetTR(&(MY_LXFZ_TIMER1->tMaxTimeRelay[1]));//复位时间继电器

	}
	RunTR(&(MY_LXFZ_TIMER1->tMaxTimeRelay[0]), (MY_LXFZ_CNT!=0));

	if( MY_LXFZ_TIMER1->tMaxTimeRelay[0].boolTrip )
		MY_LXFZ_CNT = 0;

	if (MY_LXFZ_CNT >= pPublicSet->dLXFZ) 
	{
		plogic->Output[BOOL_OUTPUT_HZBS] = TRUE;	
		pRunPublic->bs_type |= HZ_BS_LXFZ;		
	}

}


#if (!((TYPE_USER == USER_GUANGXI)||(TYPE_USER == USER_GUIYANG)))
#define MY_LYX_TIMER1   ((TIMERELAY*)(plogic->apvUser[0]))
void LocalYxLogic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
}

void LocalYxLogic(LogicElem *plogic)
{ 
    BOOL bsgz = FALSE;
#if (TYPE_USER == USER_SHANDONG) 
	BOOL bNoU;
#endif
    int fd;
	if (pPublicSet->bSetChg)
	{
		InitTR(MY_LYX_TIMER1, 20000, 0);
	}
	RunTR(MY_LYX_TIMER1, pRunPublic->yk_type!=0);
	if (MY_LYX_TIMER1->boolTrip)
		pRunPublic->yk_type = 0;

	if(pRunPublic->bs_type & (HZ_BS_SR|HZ_BS_FR |HZ_BS_SY|HZ_BS_CY))
	{
		pRunPublic->vyx |= PR_VYXP_BS_HZFX; 
		GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	}
	else
		pRunPublic->vyx &= ~PR_VYXP_BS_HZFX; 

    if(pRunPublic->bs_type & (HZ_BS_S|HZ_BS_F|HZ_BS_U0))
    {
		pRunPublic->vyx |= PR_VYXP_BS_HZZX; 
	    GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
    }
	else
		pRunPublic->vyx &= ~PR_VYXP_BS_HZZX; 

	
    if(pRunPublic->bs_type & HZ_BS_DU)
    {
		pRunPublic->vyx |= PR_VYXP_BS_DU;
	    GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
    }
	else
		pRunPublic->vyx &= ~PR_VYXP_BS_DU; 

	// 这里对瞬压残压遥信做了区分，
    if(pRunPublic->bs_type & HZ_BS_SY)
    {
	    pRunPublic->vyx |= PR_VYXP_BS_SY; 
    	GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);			   
    }
	else
		pRunPublic->vyx &= ~PR_VYXP_BS_SY; 


	if(pRunPublic->bs_type & HZ_BS_CY)
	{
		pRunPublic->vyx |= PR_VYXP_BS_CY; 
    	GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	}
	else
		pRunPublic->vyx &= ~PR_VYXP_BS_CY; 


	if(pRunPublic->bs_type & HZ_BS_MF)
	{
		pRunPublic->vyx |= PR_VYXP_BS_MF; 
        GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	}
	else
		pRunPublic->vyx &= ~PR_VYXP_BS_MF; 
	

	if(pRunPublic->bs_type & HZ_BS_CH)
	{
		pRunPublic->vyx |= PR_VYXP_BS_CH;
	    GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	}
	else
		pRunPublic->vyx &= ~PR_VYXP_BS_CH; 

   for (fd=0; fd<fdNum; fd++)
   {
      if(prRunInfo[fd].vyx  & PR_VYX_SGZ)
      {
        bsgz = TRUE;
        break;
      }
   }

   if(pRunPublic-> fd == 0) //仅第一回线写故障总
   {
	  if(bsgz)
	    {
	        pRunPublic->vyx |= PR_VYXP_I_FAULT;
	        GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
	    }
	    else
	       pRunPublic->vyx &= ~PR_VYXP_I_FAULT; 
   	}
	
#if (TYPE_USER == USER_SHANDONG) 
		
#ifdef LW
		
		 if(pPublicSet->byWorkMode == PR_MODE_SEG)
		 {
			if((pPublicSet->dYb&PR_YB_SUDY) && !(pRunPublic->vyx&PR_VYX_S_MODE))
			{
				pRunPublic->vyx |= PR_VYX_S_MODE;
				GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
			}
			else if( !(pPublicSet->dYb&PR_YB_SUDY) && (pRunPublic->vyx&PR_VYX_S_MODE))
			{
				pRunPublic->vyx &= ~PR_VYX_S_MODE;
			}
			if(pRunPublic->vyx&PR_VYX_L_MODE)
				pRunPublic->vyx &= ~PR_VYX_L_MODE; 
		 }
		 else if(pPublicSet->byWorkMode == PR_MODE_TIE)
		 {
			 if((pPublicSet->dYb&PR_YB_SUSY) && !(pRunPublic->vyx&PR_VYX_L_MODE))
			{
				pRunPublic->vyx |= PR_VYX_L_MODE;
				GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
			}
			else if( !(pPublicSet->dYb&PR_YB_SUSY) && (pRunPublic->vyx&PR_VYX_L_MODE))
			{
				pRunPublic->vyx &= ~PR_VYX_L_MODE;
			}
			if(pRunPublic->vyx&PR_VYX_S_MODE)
				pRunPublic->vyx &= ~PR_VYX_S_MODE;
		 }
		
#else
		 if((pPublicSet->dYb&PR_YB_SUDY) && !(pRunPublic->vyx&PR_VYX_S_MODE))
		 {
			 pRunPublic->vyx |= PR_VYX_S_MODE;
			 GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		 }
		 else if( !(pPublicSet->dYb&PR_YB_SUDY) && (pRunPublic->vyx&PR_VYX_S_MODE))
		 {
			 pRunPublic->vyx &= ~PR_VYX_S_MODE;
		 }
		
#endif
		 if((pPublicSet->dYb&PR_YB_SUDY) && !(pRunPublic->vyx&PR_VYXP_SEG))
		 {
			 pRunPublic->vyx |= PR_VYXP_SEG;
			 GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		 }
		 else if( !(pPublicSet->dYb&PR_YB_SUDY) && (pRunPublic->vyx&PR_VYXP_SEG))
		 {
			 pRunPublic->vyx &= ~PR_VYXP_SEG;
		 }
		 if((pPublicSet->dYb&PR_YB_SUSY) && !(pRunPublic->vyx&PR_VYXP_TIE))
		 {
			 pRunPublic->vyx |= PR_VYXP_TIE;
			 GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		 }
		 else if( !(pPublicSet->dYb&PR_YB_SUSY) && (pRunPublic->vyx&PR_VYXP_TIE))
		 {
			 pRunPublic->vyx &= ~PR_VYXP_TIE;
		 }
	
		 
		 if((pValSL->dU[0] <= pPublicSet->dSLUMin[0]) && !(pRunPublic->vyx&PR_VYX_AYY))
		 {
			 pRunPublic->vyx |= PR_VYX_AYY;
			 GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		 }
		 else if((pValSL->dU[0] > pPublicSet->dSLUMin[0]) && pRunPublic->vyx&PR_VYX_AYY)
		 {
			 pRunPublic->vyx &= ~PR_VYX_AYY;
		 }
		 
		 if((pValSL->dU[2] <= pPublicSet->dSLUMin[2]) && !(pRunPublic->vyx&PR_VYX_CYY))
		 {
			 pRunPublic->vyx |= PR_VYX_CYY;
			 GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		 }
		 else if((pValSL->dU[2] > pPublicSet->dSLUMin[2]) && pRunPublic->vyx&PR_VYX_CYY)
		 {
			 pRunPublic->vyx &= ~PR_VYX_CYY;
		 }
		 
		 bNoU = (pPublicSet->dTX/10000/7) & 0x01;
		 if(bNoU && !(pRunPublic->vyx&PR_VYX_X_1))
		 {
			pRunPublic->vyx |= PR_VYX_X_1;
			GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		 }
		 else if(!bNoU && pRunPublic->vyx&PR_VYX_X_1)
			pRunPublic->vyx &= ~PR_VYX_X_1;
	
		 bNoU = (pPublicSet->dTX/10000/7) & 0x02;  
		 if(bNoU && !(pRunPublic->vyx&PR_VYX_X_2))
		 {
			pRunPublic->vyx |= PR_VYX_X_2;
			GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		 }
		 else if(!bNoU && pRunPublic->vyx&PR_VYX_X_2)
			pRunPublic->vyx &= ~PR_VYX_X_2;
		 
		 bNoU = (pPublicSet->dTX/10000/7) & 0x04;	 
		 if(bNoU && !(pRunPublic->vyx&PR_VYX_X_4))
		 {
			pRunPublic->vyx |= PR_VYX_X_4;
			GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		 }
		 else if(!bNoU && pRunPublic->vyx&PR_VYX_X_4)
			pRunPublic->vyx &= ~PR_VYX_X_4;
		 
		 bNoU = (pPublicSet->dTX/10000/7) & 0x08;	 
		 if(bNoU && !(pRunPublic->vyx&PR_VYX_X_8))
		 {
			pRunPublic->vyx |= PR_VYX_X_8;
			GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		 }
		 else if(!bNoU && pRunPublic->vyx&PR_VYX_X_8)
			pRunPublic->vyx &= ~PR_VYX_X_8;
#else
		if((pValSL->dU[0] > pPublicSet->dSLU[0]) && !(pRunPublic->vyx&PR_VYX_AYY))
		{
			pRunPublic->vyx |= PR_VYX_AYY;
			GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		}
		else if((pValSL->dU[0] <= pPublicSet->dSLU[0]) && pRunPublic->vyx&PR_VYX_AYY)
		{
			pRunPublic->vyx &= ~PR_VYX_AYY;
		}
		
		if((pValSL->dU[2] > pPublicSet->dSLU[2]) && !(pRunPublic->vyx&PR_VYX_CYY))
		{
			pRunPublic->vyx |= PR_VYX_CYY;
			GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		}
		else if((pValSL->dU[2] <= pPublicSet->dSLU[2]) && pRunPublic->vyx&PR_VYX_CYY)
		{
			pRunPublic->vyx &= ~PR_VYX_CYY;
		}
#endif
}
#else
#define MY_LYX_TIMER1   ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_LYX_TIMER2   ((TIMERELAY*)(plogic->apvUser[1]))
void LocalYxLogic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	plogic->apvUser[1] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
}

void LocalYxLogic(LogicElem *plogic)
{
    if (pPublicSet->bSetChg)
	{
		InitTR(MY_LYX_TIMER1, 20000, 0);
		InitTR(MY_LYX_TIMER2, 20000, 0);
	}

	RunTR(MY_LYX_TIMER1, pRunPublic->yk_type!=0);
	if (MY_LYX_TIMER1->boolTrip)
		pRunPublic->yk_type = 0;

	RunTR(MY_LYX_TIMER2, pRunPublic->js_type!=0);
	if (MY_LYX_TIMER2->boolTrip)
		pRunPublic->js_type = 0;
	
    pRunPublic->vyx &= ~(PR_VYXP_MASK);
	
    if (pPublicSet->byWorkMode == PR_MODE_SEG)
	{
	    pRunPublic->vyx |= PR_VYXP_SEG;
		if (pRunPublic->yk_type == KG_HZ_CONTROL)
			pRunPublic->vyx |= PR_VYXP_SEG_H1;
		if (pRunPublic->yk_type ==  KG_HZ_SWITCH)
			pRunPublic->vyx |= PR_VYXP_SEG_H2;
		if (pRunPublic->yk_type == KG_HZ_REMOTE)
			pRunPublic->vyx |= PR_VYXP_SEG_H3;
		if (pRunPublic->yk_type & KG_FZ_CONTROL)
			pRunPublic->vyx |= PR_VYXP_SEG_F1;
		if (pRunPublic->yk_type ==  KG_FZ_SWITCH)
			pRunPublic->vyx |= PR_VYXP_SEG_F2;
		if (pRunPublic->yk_type == KG_FZ_REMOTE)
			pRunPublic->vyx |= PR_VYXP_SEG_F3;

		if (pRunPublic->js_type & HZ_JS_DY)
			pRunPublic->vyx |= PR_VYXP_DY_JS;
		else
			pRunPublic->vyx &= ~PR_VYXP_DY_JS;

		GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);


    }
	else if (pPublicSet->byWorkMode == PR_MODE_TIE)
	{
	    pRunPublic->vyx |= PR_VYXP_TIE;
		if (pRunPublic->yk_type & KG_HZ_MASK)
			pRunPublic->vyx |= PR_VYXP_TIE_SH;
		if (pRunPublic->yk_type & KG_FZ_MASK)
			pRunPublic->vyx |= PR_VYXP_TIE_SF;
		if (pRunPublic->js_type & HZ_JS_TIE_DY)
			pRunPublic->vyx &= ~PR_VYXP_TIEDY_JS;
		if (pRunPublic->no_u)
			pRunPublic->vyx |= PR_VYXP_TIE_FZ;
		GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);

	}
	else if (pPublicSet->byWorkMode == PR_MODE_PR)
		{pRunPublic->vyx |= PR_VYXP_PR;
		GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		}

	else if (pPublicSet->byWorkMode == PR_MODE_FJ)
		{pRunPublic->vyx |= PR_VYXP_FJ;
			GetSysClock(&(pRunPublic->fault.i_clock), CALCLOCK);
		}


    if (pRunPublic->bs_type & HZ_BS_MF)
		pRunPublic->vyx |= PR_VYXP_SF_BS;

	if (pRunPublic->bs_type & HZ_BS_SY)
		pRunPublic->vyx |= PR_VYXP_SY_BS;

	if (pPublicSet->byPowerIndex == 0)
	{
	    if (pRunPublic->bs_type & HZ_BS_S)
			pRunPublic->vyx |= PR_VYXP_P_ZBS;
		
		if (pRunPublic->bs_type & HZ_BS_SR)
			pRunPublic->vyx |= PR_VYXP_P_FBS;

		if (pRunPublic->bs_type & HZ_BS_F)
			pRunPublic->vyx |= PR_VYXP_F_ZBS;

		if (pRunPublic->bs_type & HZ_BS_FR)
			pRunPublic->vyx |= PR_VYXP_F_FBS;

		if (pRunPublic->su)
		    pRunPublic->vyx |= PR_VYXP_PU;
		if (pRunPublic->fu)
			pRunPublic->vyx |= PR_VYXP_FU;
	}
	else
	{
	    if (pRunPublic->bs_type & HZ_BS_S)
			pRunPublic->vyx |= PR_VYXP_F_ZBS;

		if (pRunPublic->bs_type & HZ_BS_SR)
			pRunPublic->vyx |= PR_VYXP_F_FBS;

		
		if (pRunPublic->bs_type & HZ_BS_F)
			pRunPublic->vyx |= PR_VYXP_P_ZBS;

		if (pRunPublic->bs_type & HZ_BS_FR)
			pRunPublic->vyx |= PR_VYXP_P_FBS;

		if (pRunPublic->su)
		    pRunPublic->vyx |= PR_VYXP_FU;
		if (pRunPublic->fu)
			pRunPublic->vyx |= PR_VYXP_PU;

	}

	if (pRunPublic->js_type & HZ_JS_MH)
		pRunPublic->vyx |= PR_VYXP_HZ_JS;

	if (pRunPublic->js_type & HZ_JS_FG)
		pRunPublic->vyx |= PR_VYXP_FG_JS;

	
}
#endif

#ifdef _POWEROFF_TEST_VER_
/*馈线故障电流*/
typedef struct
{
   short ycNo[8];
}VPrFaultYc;
VPrFaultYc *prPrFaultYc;
void createPrYcRef(void)
{
   int i,j;
   VPrFaultYc *pPrFaultYc;

    prPrFaultYc = malloc(sizeof(VPrFaultYc)*MAX_FD_NUM);
	memset(prPrFaultYc, 0, sizeof(VPrFaultYc)*MAX_FD_NUM);

	for (i=0; i<fdNum; i++)
	{
	   pPrFaultYc = prPrFaultYc+i;
	   for (j=0; j<8; j++)
	   	   pPrFaultYc->ycNo[j] = -1;
	}
   
   for(i=0; i<fdNum; i++)
   {
       pPrFaultYc = prPrFaultYc+i;
       for(j=0; j<ycNum; j++)
       {
           if(pYcCfg[j].fdNo == i)
           {
              if(pYcCfg[j].type == YC_TYPE_Ia)
                 pPrFaultYc->ycNo[0] = j;
              else if(pYcCfg[j].type == YC_TYPE_Ib)
			     pPrFaultYc->ycNo[1] = j;
			  else if(pYcCfg[j].type == YC_TYPE_Ic)
			     pPrFaultYc->ycNo[2] = j;
			  else if(pYcCfg[j].type == YC_TYPE_I0)
			     pPrFaultYc->ycNo[3] = j;
			  else if(pYcCfg[j].type == YC_TYPE_Pa)
			  	 pPrFaultYc->ycNo[4] = pYcCfg[j].arg1;
			  else if(pYcCfg[j].type == YC_TYPE_Pb)
			  	 pPrFaultYc->ycNo[5] = pYcCfg[j].arg1;
			  else if(pYcCfg[j].type == YC_TYPE_Pc)
			  {
			     pPrFaultYc->ycNo[6] = pYcCfg[j].arg1;
				 if(pYcCfg[pYcCfg[j].arg1+1].type == YC_TYPE_U0)
				 	pPrFaultYc->ycNo[7] = pYcCfg[j].arg1+1;
			  }
			  
		   }
       }
   }
}

void SaveCurrentFault(DWORD *pdI, int fd, int flag)
{
    int i;
	WORD wp,wNo;
	long val;
    VPrFaultYc *pPrFaultI;
	VFdCfg   *pCfg;
#if(TYPE_IO == IO_TYPE_EXTIO)
    WORD rp;
    VExtIoRcd *pRcd = (VExtIoRcd *)g_Sys.byIORcdBuf;
    struct VDBYC *pData = (struct VDBYC*)(pRcd+1);
#endif

	pPrFaultI = prPrFaultYc + fd;
    pCfg = pFdCfg + fd;

    if (flag == 0)
	{
	    for (i=0; i<3; i++)
		{
		   wp = prFaultIRec->wp;
		   wNo = pPrFaultI->ycNo[i];
		   prFaultIRec->yc[wp].wNo = wNo;
		   val = pdI[i]*10000/pCfg->gain_i[i];
		   val = val * pYcCfg[wNo].xs1/pYcCfg[wNo].xs2;
		   prFaultIRec->yc[wp].nValue = val;
           prFaultIRec->wp = (prFaultIRec->wp+1)&(prFaultIRec->size-1);
	    }
    }
	else
	{
	     wp = prFaultIRec->wp;
		 wNo = pPrFaultI->ycNo[3];
		 prFaultIRec->yc[wp].wNo = wNo;
		 val = pdI[3]*10000/pCfg->gain_i0;
		 val = val * pYcCfg[wNo].xs1/pYcCfg[wNo].xs2;
		 prFaultIRec->yc[wp].nValue = val;
         prFaultIRec->wp = (prFaultIRec->wp+1)&(prFaultIRec->size-1);
	}

#if(TYPE_IO == IO_TYPE_EXTIO)
    smMTake(g_Sys.dwExtIoDataSem);
	pRcd->rcd = EXTIO_RCD_PR_FAULT;
	pRcd->len = 0;
	while(prFaultIRec->wp != prFaultIRec->rp)
	{
	   rp = prFaultIRec->rp;
	   pData->wNo = prFaultIRec->yc[rp].wNo;
	   pData->nValue = prFaultIRec->yc[rp].nValue;
	   prFaultIRec->rp = (prFaultIRec->rp+1)&(prFaultIRec->size-1);
	   pData++;
	   pRcd->len += sizeof(struct VDBYC);
	}
	ExtIoRcd(NULL, NULL, SECTOTICK(20));
	smMGive(g_Sys.dwExtIoDataSem);	
#endif
}

//广州测试之馈线停电前负荷数据,仅测试临时用
#define POWER_BUF_NUM         16*10
void PowerOffLogic(int fd)
{
	static int normal[MAX_FD_NUM] = {0};
	static int cnt[MAX_FD_NUM] = {0};
	static int ptr[MAX_FD_NUM] = {0};
	static DWORD dI[POWER_BUF_NUM][3];
	static DWORD dU[POWER_BUF_NUM][3];
	static int iCount;
	VPrFaultYc *pPrFaultYc;
	VFdCfg   *pCfg;
    VFdProtCal *pVal;
	LogicElem *logic = prLogicElem+fd*GRAPH_NUM;
	VPrRunInfo *pRunInfo = prRunInfo+fd;

	WORD wp,wNo;
	float U[3],I[3];
	long val;

	BOOL ni;
	int tptr,index, i;
#if(TYPE_IO == IO_TYPE_EXTIO)
    WORD rp;
    VExtIoRcd *pRcd = (VExtIoRcd *)g_Sys.byIORcdBuf;
    struct VDBYC *pData = (struct VDBYC*)(pRcd+1);
#endif


    iCount = 1000/(100+3*fdNum);

    cnt[fd]++;
	if (cnt[fd] < iCount) return;
	cnt[fd] = 0;
	
	pVal = fdProtVal + fd;
	pCfg = pFdCfg + fd;
	ni = logic[GRAPH_WL].Output[BOOL_OUTPUT_PICK];

	if (!ni) 
	{
		index = fd*16+ptr[fd];
		normal[fd] = 1;
		dI[index][0] = pVal->dI[0];
		dI[index][1] = pVal->dI[1];
		dI[index][2] = pVal->dI[2];
		dU[index][0] = pVal->dU[0];
		dU[index][1] = pVal->dU[1];
		dU[index][2] = pVal->dU[2];
		ptr[fd]= (ptr[fd]+1)&(16-1);
	}	
		
	if (ni && normal[fd])
	{
		normal[fd] = 0;

		tptr  = (ptr[fd]-5)&(16-1);
		tptr  = fd*16+tptr;

        pPrFaultYc = prPrFaultYc + fd;

		if (!(pCfg->cfg & FD_CFG_PT_58V))
			dU[tptr][2] = 1;

        for(i=0; i<3; i++)
		{
		   if (pPrFaultYc->ycNo[i] == -1)
		   	  continue;
		   wp = prFaultIRec->wp;
		   wNo = pPrFaultYc->ycNo[i];
		   prFaultIRec->yc[wp].wNo = wNo;
		   val = dI[tptr][i]*10000/pCfg->gain_i[i];
		   val = val*pYcCfg[wNo].xs1/pYcCfg[wNo].xs2;
		   prFaultIRec->yc[wp].nValue = val;
           prFaultIRec->wp = (prFaultIRec->wp+1)&(prFaultIRec->size-1);
        }
		for(i=4; i<7; i++)
		{
		   if (pPrFaultYc->ycNo[i] == -1)
		   	  continue;
		   wp = prFaultIRec->wp;
		   wNo = pPrFaultYc->ycNo[i];
		   prFaultIRec->yc[wp].wNo = wNo;
		   val = dU[tptr][i]*10000/pCfg->gain_u[i];
		   val = val*pYcCfg[wNo].xs1/pYcCfg[wNo].xs2;
		   prFaultIRec->yc[wp].nValue = val;
           prFaultIRec->wp = (prFaultIRec->wp+1)&(prFaultIRec->size-1);
		}
 
		I[0] = prValue_l2f(5, pCfg, dI[tptr][0]);
		I[1] = prValue_l2f(6, pCfg, dI[tptr][1]);
		I[2] = prValue_l2f(7, pCfg, dI[tptr][2]); 
		U[0] = prValue_l2f(1, pCfg, dU[tptr][0]);
		U[1] = prValue_l2f(2, pCfg, dU[tptr][1]);
		U[2] = prValue_l2f(3, pCfg, dU[tptr][2]);

		WriteWarnEvent(NULL,  0, pRunInfo->fd+1, "第%d回线停电Ua=%2.1fV, Ub=%2.1fV, Uc=%2.1fV, Ia=%4.3fA, Ib=%4.3fA, Ic=%4.3fA", 
				       pRunInfo->fd+1, U[0], U[1], U[2], I[0], I[1], I[2]);

#if(TYPE_IO == IO_TYPE_EXTIO)
	    smMTake(g_Sys.dwExtIoDataSem);
		pRcd->rcd = EXTIO_RCD_PR_FAULT;
		pRcd->len = 0;
		while(prFaultIRec->wp != prFaultIRec->rp)
		{
		   rp = prFaultIRec->rp;
		   pData->wNo = prFaultIRec->yc[rp].wNo;
		   pData->nValue = prFaultIRec->yc[rp].nValue;
		   prFaultIRec->rp = (prFaultIRec->rp+1)&(prFaultIRec->size-1);
		   pRcd->len += sizeof(struct VDBYC);
		   pData++;
		}
		ExtIoRcd(NULL, NULL, SECTOTICK(20));
		smMGive(g_Sys.dwExtIoDataSem);	
#endif
	}		
}
#endif


#ifdef _POWEROFF_TEST_VER_
void InitFaultRec()
{
    prFaultIRec = malloc(sizeof(VYcFaultIRec));
    memset(prFaultIRec, 0, sizeof(VYcFaultIRec));
	prFaultIRec->size = YC_FAULT_REC;
}

int GetCurrentFault(BYTE *pBuf, int max)
{
    int len = 0;
    WORD rp;
    struct VDBYC *pYcFault = (struct VDBYC*)pBuf;

    while((prFaultIRec->rp != prFaultIRec->wp)&&(len<max))
    {
       rp = prFaultIRec->rp;
	   pYcFault->wNo = prFaultIRec->yc[rp].wNo;
	   pYcFault->nValue = prFaultIRec->yc[rp].nValue;
	   prFaultIRec->rp = (prFaultIRec->rp+1)&(prFaultIRec->size-1);
	   len += sizeof(struct VDBYC);
	   pYcFault++;
    }
	return len;
}

#endif
#endif

void ykTypeSave(int type, int id, int val)
{
#ifdef INCLUDE_PR
    VFdCfg *pfdcfg;
	VPrSetPublic* ppPublicSet; 

    int i = 0;

	for(i = 0; i < pubfdNum;i++)
	{
		ppPublicSet = prSetPublic[prRunPublic[i].set_no] + i;
		pfdcfg = pFdCfg+ppPublicSet->bySLLine;
		if(pfdcfg->kg_ykno == id)
			break;
	}
	
	if(i == pubfdNum)
		return;
	
    if (type == 0)
    {
        if (val == 1)
           prRunPublic[i].yk_type = KG_HZ_CONTROL;
        else if (val == 0)
		   prRunPublic[i].yk_type = KG_FZ_CONTROL;
    }
	else if (type == 1)
	{
	    if (val == 1)
           prRunPublic[i].yk_type = KG_HZ_REMOTE;
        else if (val == 0)
		   prRunPublic[i].yk_type = KG_FZ_REMOTE;
	}
#endif
}

#ifdef  _YIERCI_RH_
//后面几个函数FTU才有，所以不用改?
void SbFzStateSave(int val)
{
#ifdef INCLUDE_PR
    if (val == 1)
       prRunPublic->fz_state = 1;
    else if (val == 0)
   		prRunPublic->fz_state = 0;
#endif
}
#endif

BOOL GetHzbsYx(void)
{
#ifdef INCLUDE_PR
     if((prRunPublic == NULL) || (g_prPubInit != 1)) return FALSE;
     else    return (prRunPublic->vyx & PR_VYXP_HZ_BS);
#else
     return FALSE;
#endif
}

#if (TYPE_USER == USER_BEIJING)
BOOL  KgType()
{
#ifdef INCLUDE_PR
	if((prRunPublic == NULL) || (g_prPubInit != 1)) 
		return FALSE;
	else
		return pPublicSet->bKgMode;
#else
	return FALSE;
#endif	
}
#endif


