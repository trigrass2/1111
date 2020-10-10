#include "syscfg.h"
#include "sys.h"

extern int fdNum;
#ifdef INCLUDE_PR
#include "PrLogic.h"
#include "comtrade.h"
#include "record.h"
#include "PrLogicJd.h"


#define LOGIC_LONE_TIME    30
#define PR_TM_INTERVAL     4
#define PR_RUN_TM          20

void AiCalcLogic(LogicElem *plogic);
void IDirLogic_Init(LogicElem *plogic);
void IDirLogic(LogicElem *plogic);
void I1Logic_Init(LogicElem *plogic);
void I1Logic(LogicElem *plogic);
void I2Logic_Init(LogicElem *plogic);
void I2Logic(LogicElem *plogic);
void I3Logic_Init(LogicElem *plogic);
void I3Logic(LogicElem *plogic);
void I01Logic_Init(LogicElem *plogic);
void I01Logic(LogicElem *plogic);
void I02Logic_Init(LogicElem *plogic);
void I02Logic(LogicElem *plogic);
void I03Logic_Init(LogicElem *plogic);
void I03Logic(LogicElem *plogic);
void IGfhLogic_Init(LogicElem *plogic);
void IGfhLogic(LogicElem *plogic);
void UGyLogic_Init(LogicElem *plogic);
void UGyLogic(LogicElem *plogic);
void UDyLogic_Init(LogicElem *plogic);
void UDyLogic(LogicElem *plogic);
void UMXQYLogic_Init(LogicElem *plogic);
void UMXQYLogic(LogicElem *plogic);

void WLLogic_Init(LogicElem *plogic);
void WLLogic(LogicElem *plogic);
void PTLogic_Init(LogicElem *plogic);
void PTLogic(LogicElem *plogic);
void CHLogic_Init(LogicElem *plogic);
void CHLogic(LogicElem *plogic);
void WYFZLogic_Init(LogicElem *plogic);
void WYFZLogic(LogicElem *plogic);
void JSLogic_Init(LogicElem *plogic);
void JSLogic(LogicElem *plogic);
void DjqdLogic_Init(LogicElem *plogic);
void DjqdLogic(LogicElem *plogic);
void BhqdLogic_Init(LogicElem *plogic);
void BhqdLogic(LogicElem *plogic);
void RelayLogic_Init(LogicElem *plogic);
void RelayLogic(LogicElem *plogic);
void HHLogic_Init(LogicElem *plogic);
void HHLogic(LogicElem *plogic);
void YxLogic_Init(LogicElem *plogic);
void YxLogic(LogicElem *plogic);
void IRcdLogic_Init(LogicElem * plogic);
void IRcdLogic(LogicElem * plogic);
void GPLogic(LogicElem *plogic);
void GPLogic_Init(LogicElem *plogic);
void DPLogic(LogicElem *plogic);
void DPLogic_Init(LogicElem *plogic);
void YxTzLogic(LogicElem *plogic);
void YxTzLogic_Init(LogicElem *plogic);
extern int GetTripOnInput(void);

/*一般逻辑功能都加在跳闸逻辑之前,SLT的逻辑在跳闸逻辑之后*/
static LogicInfo tLogicTable[]=
{
  {GRAPH_AD,    "模拟量计算", NULL,       AiCalcLogic},
  {GRAPH_JS,    "加速跳闸",   JSLogic_Init,   JSLogic},
  {GRAPH_I1,    "过流一段",   I1Logic_Init,   I1Logic},
  {GRAPH_I2,    "过流二段",   I2Logic_Init,   I2Logic},
  {GRAPH_I3,    "过流三段",   I3Logic_Init,   I3Logic},
  {GRAPH_IGFH,  "过负荷",     IGfhLogic_Init, IGfhLogic},
  {GRAPH_I01,   "零流一段",   I01Logic_Init,  I01Logic},
  {GRAPH_I02,   "零流二段",   I02Logic_Init,  I02Logic},
  {GRAPH_I03,   "零流三段",   I03Logic_Init,  I03Logic},
  {GRAPH_IDIR,  "方向元件",   IDirLogic_Init, IDirLogic},
  {GRAPH_GY,    "过压",       UGyLogic_Init,  UGyLogic},
  {GRAPH_DY,    "低压",       UDyLogic_Init,  UDyLogic},
  {GRAPH_WL,    "无流",       WLLogic_Init,   WLLogic},
  {GRAPH_PT,    "TV断线",     PTLogic_Init,   PTLogic},
  {GRAPH_CH,    "重合闸",     CHLogic_Init,   CHLogic},
  {GRAPH_WYFZ,  "无压分闸",   WYFZLogic_Init, WYFZLogic},
  {GRAPH_DJQD,  "电机启动",   DjqdLogic_Init, DjqdLogic},
  {GRAPH_BHQD,  "保护启动",   BhqdLogic_Init, BhqdLogic},
  {GRAPH_RELAY, "保护跳闸",   RelayLogic_Init,RelayLogic},
  {GRAPH_HH,    "保护呼唤",   HHLogic_Init,   HHLogic},
  {GRAPH_YX,    "故障遥信",   YxLogic_Init,   YxLogic},
  {GRAPH_IRCD,  "记录电流",   IRcdLogic_Init, IRcdLogic},
  {GRAPH_GP,    "过频",     GPLogic_Init,   GPLogic}, 
  {GRAPH_DP,    "低频",     DPLogic_Init,   DPLogic}, 
  {GRAPH_YXTZ,  "外部遥信跳闸",     YxTzLogic_Init,   YxTzLogic}, 
  {GRAPH_MXQY,    "母线欠压",       UMXQYLogic_Init,  UMXQYLogic},
};

enum
{
    EVENT_NO=30,
    EVENT_I1=0,
    EVENT_I2,
	EVENT_I3,
	EVENT_IGFH,
	EVENT_I01,
	EVENT_I02,
	EVENT_I03,
	EVENT_GY,
	EVENT_DY,
	EVENT_PT,
	EVENT_CH,
	EVENT_WY_FZ,
	EVENT_TGJ,
	EVENT_HGJ,
	EVENT_XHFG,
	EVENT_DJQD,
	EVENT_KDGL,
	EVENT_ZDBS,
	EVENT_I0C,  //小电流接地
	EVENT_GP, // 过频
	EVENT_DP, // 
	EVENT_YXTZ,//外部遥信跳闸
	EVENT_MXQY,
	EVENT_BHQD=EVENT_NO,
	EVENT_TRIP=EVENT_BHQD+30,
	EVENT_HH=EVENT_TRIP+30,
	EVENT_CLOSE=EVENT_HH+30,
	EVENT_BAK=0x8000
};

static LogicEvent tEventTable[]=
{
	{EVENT_I1,   "过流一段"},
    {EVENT_I2,   "过流二段"},
    {EVENT_I3,   "过流三段"},
    {EVENT_IGFH, "过负荷"},
    {EVENT_I01,  "零流一段"},
    {EVENT_I02,  "零流二段"},
    {EVENT_I03,  "零流三段"},
    {EVENT_GY,   "过电压"},
    {EVENT_DY,   "低电压"},
    {EVENT_PT,   "TV断线"},
    {EVENT_CH,   "重合闸"},
    {EVENT_WY_FZ,"无压分闸"},
    {EVENT_TGJ,  "跳闸失败告警"},
	{EVENT_HGJ,  "合闸失败告警"},
    {EVENT_XHFG, "外部信号复归"},
	{EVENT_DJQD, "过励磁涌流启动"},
    {EVENT_KDGL, "扩大故障隔离"},
	{EVENT_ZDBS, "遮断电流闭锁分闸"},
	{EVENT_I0C,"小电流接地"},
	{EVENT_GP,	"过频"},
	{EVENT_DP,	"低频"},
	{EVENT_YXTZ,"外部遥信跳闸"},
	{EVENT_MXQY,"母线欠压跳闸"},
};

static LogicEvent tEventPubTable[]=
{
	{EVENT_DF,      "低频"},
	{EVENT_SHTQ,    "手动合闸"},
	{EVENT_DY_HZ,   " S功能延时关合"},
    {EVENT_SY_HZ,   " L功能延时关合"},
    {EVENT_U0,      "零压"},
	{EVENT_UDIFF,   "双侧压差合环"},
	{EVENT_SL_WY,   "无压分闸"},
	{EVENT_I_CNT,   "过流计数满跳闸"},
	{EVENT_I_TRIP,  "电流型合故障跳闸"}

};

int g_prInit = 0;
int g_prDisable = 1;
int g_prRunning = 1;
DWORD g_prSem=0;
DWORD g_prCalTime=0;
DWORD g_prlubo = 0;
int pubfdNum = 1;

LogicElem *prLogicElem;
LogicElem *pLogicElem;
extern LogicElem *prLogicPublic;

DWORD Val0=0;

VFdProtCal *fdProtVal;
VPrRunSet  *pRunSet;
VPrRunInfo *prRunInfo;
VPrRunInfo *pRunInfo;

VPrYxOpt  *prYxOpt;
VPrEvtOpt *prEvtOpt;
VPrGzEvtOpt *prGzEvtOpt = NULL;
#if(TYPE_USER == USER_GUANGXI)
BOOL Bhqd_GX;
#endif
extern VPrYxPubOpt *prYxPubOpt;
extern VPrEvtPubOpt *prEvtPubOpt;
extern VPrRunPublic *prRunPublic;
extern VPrRunSet *prRunSet[2];
extern VPrSetPublic *prSetPublic[2];
extern VFdProtCal *pVal;
extern VAiTqVal *pTqVal;
extern VFdCfg *pCfg;
//extern VThread *g_Thread;
extern int prLowInit(void);
extern void prLowDrv(void);
extern int g_prPubInit;
extern VPrSetPublic *pPublicSet;
extern int g_init;

void prVyxHandle(void);
void prWavRecord(int fd, struct VCalClock *time, int flag );
float prValue_l2f(int ui_flag, VFdCfg *pCfg, long value);
void FaSetYxSrc(DWORD (*GetFaYx_Src)(int));

#ifdef _GUANGZHOU_TEST_VER_
extern void SaveCurrentFault(DWORD *pdI, int fd, int flag);
#endif

BYTE prLogicPre(int fd)
{
     pRunInfo = prRunInfo+fd;
	 pRunSet = prRunSet[pRunInfo->set_no]+fd;

	 if (pRunSet->dYb & PR_YB_GZJC_EN)
	 	pRunInfo->gzjc_on = TRUE;
	 else
	 	pRunInfo->gzjc_on = FALSE;
#if (TYPE_USER == USER_GUANGXI)
  //   if (!GetExtMmiYx(EXTMMI_FA))
  	   if (!GetDiValue(11))
	 	pRunInfo->gzjc_on = FALSE;
#endif

#if (TYPE_USER == USER_GUIYANG)

	#if(DEV_SP ==  DEV_SP_WDG)
	    if (prSetPublic[prRunPublic->set_no]->bAuto)
			pRunInfo->gzjc_on = FALSE;
		
	#elif(DEV_SP ==  DEV_SP_FTU)
		 //if (!GetExtMmiYx(EXTMMI_AUTO))
		if (!GetMyDiValue(8))                   //将SYX8作为自动化投入压板
	 		pRunInfo->gzjc_on = FALSE;
	#endif
#endif


#ifdef _GUANGZHOU_TEST_VER_
     if (!GetDiValue(11))
	 	pRunInfo->gzjc_on = FALSE;
#endif

	 if (!pRunInfo->gzjc_on) 
	 {
		   pRunInfo->vyx &= PR_VYX_YK_YB;
		   prVyxHandle();
	 	return 1;
	 }

	 pVal = fdProtVal+fd;
	 pCfg = pFdCfg+fd;
	 pLogicElem = prLogicElem+fd*GRAPH_NUM;

	// if (pCfg->valid==0)
	// 	return 1;
	 return 0;
}

void prDrv(void)
{
    int i,fd;
	BOOL bSetChg=FALSE;


	if ((g_prDisable) || (g_Sys.dwErrCode & SYS_ERR_AIO))
	{
		g_prRunning = 0;
		return;
	}

	protCal_AiVal();

	for (fd=0; fd<fdNum; fd++)
	{		
#if(TYPE_USER == USER_GUANGXI) //DTU装置当FTU使用
		if(fd > 0)
			continue;
#endif
		if(prLogicPre(fd))
		   continue;

		if(pRunSet->bSetChg)
			bSetChg = TRUE;

	    for(i=0; i<GRAPH_NUM; i++)
	    {
		   tLogicTable[i].Func(pLogicElem+i);
	    }	

        if(bSetChg)
			pRunSet->bSetChg = FALSE;

		prVyxHandle();

    	
		
#ifdef INCLUDE_FA
		NotifyJudgeFault(fd);
#endif
		if(g_prlubo & (1 << fd))
		{
			g_prlubo &= ~(1 << fd);
			prWavRecord(fd, NULL, REC_START |REC_STOP);
		}
	}
#ifdef INCLUDE_FA
		NotifyJudgeFaultExp();
#endif

#ifdef INCLUDE_FA_DIFF
	PrLogicFaDiff(fd);
#endif

	prLowDrv();
//    prIntUnlock();

}

int CreateLogicElem(void)
{
    int i,j;

    prLogicElem = malloc(fdNum*GRAPH_NUM*sizeof(LogicElem));
	if(prLogicElem == NULL)
		return -1;
	memset(prLogicElem, 0, fdNum*GRAPH_NUM*sizeof(LogicElem));

	for(j=0; j<fdNum; j++)
	{
	    pLogicElem = prLogicElem+GRAPH_NUM*j;
		for(i=0; i<GRAPH_NUM; i++)
		{
		   if(tLogicTable[i].Init)
		   	  tLogicTable[i].Init(pLogicElem+i);
		}
	}
	return 0;
}

int prInit(void)
{
	g_prInit = 0;

	if (fdNum == 0) return ERROR;

	//用遥信个数来决定配置里面有多少回线的公共定值模板，当然 如果保护个数不是PR_VYX_NUM了，则不兼容！
	pubfdNum = g_Sys.MyCfg.wSYXNum - g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low - fdNum*PR_VYX_NUM;
	pubfdNum = pubfdNum/PR_VYX_NUM;
	if(pubfdNum > fdNum)
		pubfdNum = fdNum;

	createProtFdGain();
	createProtFdRef();	

	prRunInfoInit();

	if(prLowInit() == ERROR) return ERROR;

	if(prSetInit()== ERROR) return ERROR;

#ifdef INCLUDE_JD_PR
	PrJdJudgeInit();
#endif

	if (CreateLogicElem() == -1) return ERROR;

	if (gainValid == 0) 
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Gain invalid and protect exit!");
		return ERROR;
	}	
	else
		g_prDisable = 0;

#ifdef INCLUDE_COMTRADE 				
	InitFdSamRecIndex();      //初始化录波FD通道索引参数
	comtradeInit(SYS_ID);
#endif

#ifdef INCLUDE_RECORD
    InitFdSamRecordIndex(); 
    recordInit(SYS_ID);
#endif

	prLightHandle();

    g_prInit = 1;
	return OK;
}

void protect(void)
{
	if (g_prInit == 0) return;
	prDrv();
}

void prTick(void)
{
    if (g_prInit == 0) return;
	if (g_init == 0) 
	{
	   return;
	}

#ifdef  INCLUDE_FA_IDIR
       FaWave_Interrupt();
#endif
}

void WriteYxDelay(int no, int value, int lock)
{
    struct VDBSOE *pDBSOE;
	VDBYxChg *pDBYxChg;

	pDBYxChg = prYxOpt->yxchg + prYxOpt->wWritePtr;
	if(lock)
	    pDBYxChg->flag = PR_YXCHG_LOCK;
	else
		pDBYxChg->flag = 0;
	pDBSOE = &(pDBYxChg->DBSOE);

	pDBSOE->wNo = no;
	pDBSOE->byValue = value;
	GetSysClock(&(pDBSOE->Time), CALCLOCK);

	prYxOpt->wWritePtr = (prYxOpt->wWritePtr+1)&(MAX_YX_NUM-1);
}

void prDataHandle(void)
{
	struct VDBSOE *pDBSOE;
	struct VDBCOS DBCOS;
	VDBYxChg *pDBYxChg;
	VDBEvent *pDBEvt;

	VFdCfg *pCfg;
	int k, fd;
	DWORD *pdI, *pdU;
	float I[4], U[4], ang[2];
	char szBak[20],szJs[5];
	char szDf[5];
	szDf[0] = '\0';
	szBak[0] = '\0';
	szJs[0] = '\0';

	if (g_prInit == 0) return;

	while(prYxOpt->wReadPtr != prYxOpt->wWritePtr)
	{
        pDBYxChg = prYxOpt->yxchg + prYxOpt->wReadPtr;
		pDBSOE   = &(pDBYxChg->DBSOE);
		DBCOS.wNo = pDBSOE->wNo; DBCOS.byValue = pDBSOE->byValue;
		
		WriteSSOE(BM_EQP_MYIO, pDBSOE->wNo, pDBSOE->byValue, pDBSOE->Time);
		WriteSYX(BM_EQP_MYIO , DBCOS.wNo, DBCOS.byValue);

		prYxOpt->wReadPtr = (prYxOpt->wReadPtr+1)&(MAX_YX_NUM-1);
	}

	while(prEvtOpt->wReadPtr != prEvtOpt->wWritePtr)
	{
	    pDBEvt = prEvtOpt->evt + prEvtOpt->wReadPtr;

		pCfg = pFdCfg + pDBEvt->fd;

		fd = pDBEvt->fd + 1;

		pdI = pDBEvt->data;
		pdU = pDBEvt->data+4;

	    if(pDBEvt->flag & PR_EVT_JS)
		    strcpy(szJs, "加速");

		if(pDBEvt->flag & PR_EVT_QD)
		{
		   if(pDBEvt->code & (1<<EVENT_I1))
		   	 WriteActEvent(&(pDBEvt->Time), EVENT_BHQD, fd,  "%s过流保护启动", pCfg->kgname);
		   if(pDBEvt->code & (1<<EVENT_IGFH))
		   	 WriteActEvent(&(pDBEvt->Time), EVENT_BHQD, fd, "%s过负荷保护启动", pCfg->kgname);
		   if(pDBEvt->code & (1<<EVENT_I01))
		   	 WriteActEvent(&(pDBEvt->Time),  EVENT_BHQD, fd, "%s零流保护启动", pCfg->kgname);
           if(pDBEvt->code & (1<<EVENT_DY))
		   	 WriteActEvent(&(pDBEvt->Time),  EVENT_BHQD, fd, "%s低压保护启动", pCfg->kgname);
		   if(pDBEvt->code & (1<<EVENT_GY))
		   	 WriteActEvent(&(pDBEvt->Time),  EVENT_BHQD, fd, "%s过压保护启动", pCfg->kgname);
		}
		else if(pDBEvt->flag & PR_EVT_TRIP)
		{
		   for(k=0; k<4; k++)
		   {
		      if(pDBEvt->code & (1<<(EVENT_I1+k)))
		      {
				I[0] = prValue_l2f(5, pCfg, pdI[0]);
				I[1] = prValue_l2f(6, pCfg, pdI[1]);
				I[2] = prValue_l2f(7, pCfg, pdI[2]); 
				I[3] = prValue_l2f(8, pCfg, pdI[3]);

				U[0] = prValue_l2f(1, pCfg, pdU[0]);
				U[1] = prValue_l2f(2, pCfg, pdU[1]);
				U[2] = prValue_l2f(3, pCfg, pdU[2]);
				U[3] = prValue_l2f(4, pCfg, pdU[3]);
				 
				 if(k == 3)
			  	    szJs[0] = '\0';
				 WriteActEvent(&(pDBEvt->Time), EVENT_I1+k+EVENT_TRIP, fd, "%s%s%s%s动作\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA", \
				 	pCfg->kgname, szBak,tEventTable[EVENT_I1+k].name, szJs,U[0],U[2],U[3],I[0], I[1], I[2],I[3]);
#ifdef _POWEROFF_TEST_VER_
                 SaveCurrentFault(pdI, pDBEvt->fd, 0);
#endif
		      }
		   }
		   for(k=0; k<3; k++)
	       {
		      if(pDBEvt->code &(1<<(EVENT_I01+k)))
			  {
					I[0] = prValue_l2f(5, pCfg, pdI[0]);
					I[1] = prValue_l2f(6, pCfg, pdI[1]);
					I[2] = prValue_l2f(7, pCfg, pdI[2]); 
					I[3] = prValue_l2f(8, pCfg, pdI[3]);

					U[0] = prValue_l2f(1, pCfg, pdU[0]);
					U[1] = prValue_l2f(2, pCfg, pdU[1]);
					U[2] = prValue_l2f(3, pCfg, pdU[2]);
					U[3] = prValue_l2f(4, pCfg, pdU[3]);
			     WriteActEvent(&(pDBEvt->Time), EVENT_I01+k+EVENT_TRIP, fd, "%s%s%s%s动作\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA", \
					 	pCfg->kgname, szBak,tEventTable[EVENT_I01+k].name, szJs, U[0],U[2],U[3],I[0], I[1], I[2],I[3]);
#ifdef _POWEROFF_TEST_VER_
                 SaveCurrentFault(pdI, pDBEvt->fd, 1);
#endif
			  }
		   }
			 
		if(pDBEvt->code &(1<<(EVENT_I0C)))
		  {
				I[0] = prValue_l2f(5, pCfg, pdI[0]);
				I[1] = prValue_l2f(6, pCfg, pdI[1]);
				I[2] = prValue_l2f(7, pCfg, pdI[2]); 
				I[3] = prValue_l2f(8, pCfg, pdI[3]);

				U[0] = prValue_l2f(1, pCfg, pdU[0]);
				U[1] = prValue_l2f(2, pCfg, pdU[1]);
				U[2] = prValue_l2f(3, pCfg, pdU[2]);
				U[3] = prValue_l2f(4, pCfg, pdU[3]);
		     WriteActEvent(&(pDBEvt->Time), EVENT_I0C+EVENT_TRIP, fd, "%s%s%s%s动作\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA", \
				 	pCfg->kgname, szBak,tEventTable[EVENT_I0C].name, szJs, U[0],U[2],U[3],I[0], I[1], I[2],I[3]);
#ifdef _POWEROFF_TEST_VER_
               SaveCurrentFault(pdI, pDBEvt->fd, 1);
#endif
		  }
		if(pDBEvt->code &(1<<(EVENT_GP)))
		  {
				I[0] = prValue_l2f(5, pCfg, pdI[0]);
				I[1] = prValue_l2f(6, pCfg, pdI[1]);
				I[2] = prValue_l2f(7, pCfg, pdI[2]); 
				I[3] = prValue_l2f(8, pCfg, pdI[3]);

				U[0] = prValue_l2f(1, pCfg, pdU[0]);
				U[1] = prValue_l2f(2, pCfg, pdU[1]);
				U[2] = prValue_l2f(3, pCfg, pdU[2]);
				U[3] = prValue_l2f(4, pCfg, pdU[3]);
		     WriteActEvent(&(pDBEvt->Time), EVENT_GP+EVENT_TRIP, fd, "%s%s动作\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA", \
				 	pCfg->kgname,tEventTable[EVENT_GP].name, U[0],U[2],U[3],I[0], I[1], I[2],I[3]);

		  }
		if(pDBEvt->code &(1<<(EVENT_DP)))
		  {
				I[0] = prValue_l2f(5, pCfg, pdI[0]);
				I[1] = prValue_l2f(6, pCfg, pdI[1]);
				I[2] = prValue_l2f(7, pCfg, pdI[2]); 
				I[3] = prValue_l2f(8, pCfg, pdI[3]);

				U[0] = prValue_l2f(1, pCfg, pdU[0]);
				U[1] = prValue_l2f(2, pCfg, pdU[1]);
				U[2] = prValue_l2f(3, pCfg, pdU[2]);
				U[3] = prValue_l2f(4, pCfg, pdU[3]);
		     WriteActEvent(&(pDBEvt->Time), EVENT_DP+EVENT_TRIP, fd, "%s%s动作\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA", \
				 	pCfg->kgname,tEventTable[EVENT_DP].name, U[0],U[2],U[3],I[0], I[1], I[2],I[3]);

		  }
		if(pDBEvt->code &(1<<(EVENT_YXTZ)))
		  {
				I[0] = prValue_l2f(5, pCfg, pdI[0]);
				I[1] = prValue_l2f(6, pCfg, pdI[1]);
				I[2] = prValue_l2f(7, pCfg, pdI[2]); 
				I[3] = prValue_l2f(8, pCfg, pdI[3]);

				U[0] = prValue_l2f(1, pCfg, pdU[0]);
				U[1] = prValue_l2f(2, pCfg, pdU[1]);
				U[2] = prValue_l2f(3, pCfg, pdU[2]);
				U[3] = prValue_l2f(4, pCfg, pdU[3]);
		     WriteActEvent(&(pDBEvt->Time), EVENT_YXTZ+EVENT_TRIP, fd, "%s%s动作\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA", \
				 	pCfg->kgname,tEventTable[EVENT_YXTZ].name, U[0],U[2],U[3],I[0], I[1], I[2],I[3]);

		  }
          if(pDBEvt->code & (1<<(EVENT_MXQY)))
	      {
	         U[0] = prValue_l2f(1, pCfg, pdU[0]);
	         U[1] = prValue_l2f(2, pCfg, pdU[1]);
	         U[2] = prValue_l2f(3, pCfg, pdU[2]);
	         WriteActEvent(&(pDBEvt->Time), EVENT_MXQY+EVENT_TRIP, fd, "%s%s动作\nUa=%3.2fV,Ub=%3.2fV,Uc=%3.2fV", pCfg->kgname, tEventTable[EVENT_MXQY].name, U[0], U[1], U[2]);
	      }
		   for(k=0; k<2; k++)
	       {
	          if(pDBEvt->code & (1<<(EVENT_GY+k)))
		      {
		         U[0] = prValue_l2f(1, pCfg, pdU[0]);
		         U[1] = prValue_l2f(2, pCfg, pdU[1]);
		         U[2] = prValue_l2f(3, pCfg, pdU[2]);
		         WriteActEvent(&(pDBEvt->Time), EVENT_GY+k+EVENT_TRIP, fd, "%s%s动作\nUa=%3.2fV,Ub=%3.2fV,Uc=%3.2fV", pCfg->kgname, tEventTable[EVENT_GY+k].name, U[0], U[1], U[2]);
		      }
		   }
		   if(pDBEvt->code & (1<<EVENT_WY_FZ))
		   {
		      if(pDBEvt->flag & PR_EVT_I_WY)
		      { 
		         	I[0] = prValue_l2f(5, pCfg, pdI[0]);
					I[1] = prValue_l2f(6, pCfg, pdI[1]);
					I[2] = prValue_l2f(7, pCfg, pdI[2]); 
					I[3] = prValue_l2f(8, pCfg, pdI[3]);

					U[0] = prValue_l2f(1, pCfg, pdU[0]);
					U[1] = prValue_l2f(2, pCfg, pdU[1]);
					U[2] = prValue_l2f(3, pCfg, pdU[2]);
					U[3] = prValue_l2f(4, pCfg, pdU[3]);
			     WriteActEvent(&(pDBEvt->Time), EVENT_WY_FZ+EVENT_TRIP, fd, "%s过流后%s\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA",\
					 	pCfg->kgname, tEventTable[EVENT_WY_FZ].name,U[0],U[2],U[3],I[0], I[1], I[2],I[3]);
		      }
			  else if(pDBEvt->flag & PR_EVT_I2_WY)
			  {
				I[0] = prValue_l2f(5, pCfg, pdI[0]);
				I[1] = prValue_l2f(6, pCfg, pdI[1]);
				I[2] = prValue_l2f(7, pCfg, pdI[2]); 
				I[3] = prValue_l2f(8, pCfg, pdI[3]);

				U[0] = prValue_l2f(1, pCfg, pdU[0]);
				U[1] = prValue_l2f(2, pCfg, pdU[1]);
				U[2] = prValue_l2f(3, pCfg, pdU[2]);
				U[3] = prValue_l2f(4, pCfg, pdU[3]);
			     WriteActEvent(&(pDBEvt->Time), EVENT_WY_FZ+EVENT_TRIP, fd, "%s二次过流后%s\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA",\
					 	pCfg->kgname, tEventTable[EVENT_WY_FZ].name,U[0],U[2],U[3],I[0], I[1], I[2],I[3]);		  
			  }
			  else
		         WriteActEvent(&(pDBEvt->Time), EVENT_WY_FZ+EVENT_TRIP, fd, "%s%s", pCfg->kgname, tEventTable[EVENT_WY_FZ].name);
           }
		}
		else if(pDBEvt->flag & PR_EVT_RC)
		{
		    if(pDBEvt->code & (1<<EVENT_CH))
		 	   WriteActEvent(&(pDBEvt->Time), EVENT_CH+EVENT_CLOSE, fd, "%s,%d次%s", pCfg->kgname, pDBEvt->data[0],tEventTable[EVENT_CH].name);
		    if(pDBEvt->code & (1<<EVENT_SHTQ))
		 	{
		 	   ang[0] = ((long)(pDBEvt->data[0]))/100.0;
			   ang[1] = ((long)(pDBEvt->data[1]))/100.0;
		 	   WriteActEvent(&(pDBEvt->Time), EVENT_SHTQ+EVENT_CLOSE, fd, "%s%s,同期角差%.2f,导前角%.2f", pCfg->kgname, tEventTable[EVENT_SHTQ].name,ang[0],ang[1]);
		    }
		}
		else if(pDBEvt->flag & PR_EVT_HH)
		{
			for(k=0; k<4; k++)
		    {
		       if(pDBEvt->code & (1<<(EVENT_I1+k)))
		       {
					I[0] = prValue_l2f(5, pCfg, pdI[0]);
					I[1] = prValue_l2f(6, pCfg, pdI[1]);
					I[2] = prValue_l2f(7, pCfg, pdI[2]); 
					I[3] = prValue_l2f(8, pCfg, pdI[3]);

					U[0] = prValue_l2f(1, pCfg, pdU[0]);
					U[1] = prValue_l2f(2, pCfg, pdU[1]);
					U[2] = prValue_l2f(3, pCfg, pdU[2]);
					U[3] = prValue_l2f(4, pCfg, pdU[3]);
				  if(k == 3)
			  	     szJs[0] = '\0';
				  WriteActEvent(&(pDBEvt->Time), EVENT_I1+k+EVENT_HH, fd, "%s%s\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA",
						pCfg->kgname, tEventTable[EVENT_I1+k].name, U[0],U[2],U[3],I[0], I[1], I[2],I[3]);
#ifdef _POWEROFF_TEST_VER_
                  SaveCurrentFault(pdI, pDBEvt->fd, 0);
#endif
		       }
		    }
		    for(k=0; k<3; k++)
		    {
			    if(pDBEvt->code & (1<<(EVENT_I01+k)))
			    {
					I[0] = prValue_l2f(5, pCfg, pdI[0]);
					I[1] = prValue_l2f(6, pCfg, pdI[1]);
					I[2] = prValue_l2f(7, pCfg, pdI[2]); 
					I[3] = prValue_l2f(8, pCfg, pdI[3]);

					U[0] = prValue_l2f(1, pCfg, pdU[0]);
					U[1] = prValue_l2f(2, pCfg, pdU[1]);
					U[2] = prValue_l2f(3, pCfg, pdU[2]);
					U[3] = prValue_l2f(4, pCfg, pdU[3]);
			       WriteActEvent(&(pDBEvt->Time), EVENT_I01+k+EVENT_HH, fd, "%s%s\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA", \
						 	pCfg->kgname, tEventTable[EVENT_I01+k].name, U[0],U[2],U[3],I[0], I[1], I[2],I[3]);
			    }
#ifdef _POWEROFF_TEST_VER_
                SaveCurrentFault(pdI, pDBEvt->fd, 0);
#endif

		    }
		    if(pDBEvt->code & (1<<EVENT_PT))
		    {
		       U[0] = prValue_l2f(1, pCfg, pdU[0]);
		       U[1] = prValue_l2f(2, pCfg, pdU[1]);
		       U[2] = prValue_l2f(3, pCfg, pdU[2]);
		       WriteActEvent(&(pDBEvt->Time), EVENT_PT+EVENT_HH, fd, "%s%s\nUab=%3.2fV,Ubc=%3.2fV,Uca=%3.2fV", pCfg->kgname, tEventTable[EVENT_PT].name, U[0], U[1], U[2]);
		    }		
			if(pDBEvt->code &(1<<(EVENT_GP)))
		    {
				I[0] = prValue_l2f(5, pCfg, pdI[0]);
				I[1] = prValue_l2f(6, pCfg, pdI[1]);
				I[2] = prValue_l2f(7, pCfg, pdI[2]); 
				I[3] = prValue_l2f(8, pCfg, pdI[3]);
		
				U[0] = prValue_l2f(1, pCfg, pdU[0]);
				U[1] = prValue_l2f(2, pCfg, pdU[1]);
				U[2] = prValue_l2f(3, pCfg, pdU[2]);
				U[3] = prValue_l2f(4, pCfg, pdU[3]);
				 WriteActEvent(&(pDBEvt->Time), EVENT_GP+EVENT_TRIP, fd, "%s%s动作\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA", \
					pCfg->kgname,tEventTable[EVENT_GP].name, U[0],U[2],U[3],I[0], I[1], I[2],I[3]);
		
		    }
			if(pDBEvt->code &(1<<(EVENT_DP)))
			{
				I[0] = prValue_l2f(5, pCfg, pdI[0]);
				I[1] = prValue_l2f(6, pCfg, pdI[1]);
				I[2] = prValue_l2f(7, pCfg, pdI[2]); 
				I[3] = prValue_l2f(8, pCfg, pdI[3]);
		
				U[0] = prValue_l2f(1, pCfg, pdU[0]);
				U[1] = prValue_l2f(2, pCfg, pdU[1]);
				U[2] = prValue_l2f(3, pCfg, pdU[2]);
				U[3] = prValue_l2f(4, pCfg, pdU[3]);
				 WriteActEvent(&(pDBEvt->Time), EVENT_DP+EVENT_TRIP, fd, "%s%s动作\nUa=%3.2fV,Uc=%3.2fV,U0=%3.2fV,Ia=%3.2fA,Ib=%3.2fA,Ic=%3.2fA,I0=%3.2fA", \
						pCfg->kgname,tEventTable[EVENT_DP].name, U[0],U[2],U[3],I[0], I[1], I[2],I[3]);
			
			}

			
		}
		else if(pDBEvt->code & (1<<EVENT_GP))
		   	 WriteActEvent(&(pDBEvt->Time),  EVENT_BHQD, fd, "%s过频保护", pCfg->kgname);
		else if(pDBEvt->code & (1<<EVENT_DP))
		   	 WriteActEvent(&(pDBEvt->Time),  EVENT_BHQD, fd, "%s低频保护", pCfg->kgname);
        else if(pDBEvt->code & (1<<EVENT_TGJ))
			WriteActEvent(&(pDBEvt->Time), EVENT_TGJ, fd, "%s%s", pCfg->kgname, tEventTable[EVENT_TGJ].name);
		else if(pDBEvt->code & (1<<EVENT_HGJ))
			WriteActEvent(&(pDBEvt->Time), EVENT_HGJ, fd, "%s%s", pCfg->kgname, tEventTable[EVENT_HGJ].name);
		else if(pDBEvt->code & (1<<EVENT_XHFG))
			WriteActEvent(&(pDBEvt->Time), EVENT_XHFG, fd, "%s%s", pCfg->kgname, tEventTable[EVENT_XHFG].name);
		else if(pDBEvt->code & (1<<EVENT_DJQD))
			WriteActEvent(&(pDBEvt->Time), EVENT_DJQD, fd, "%s%s", pCfg->kgname, tEventTable[EVENT_DJQD].name);
		else if(pDBEvt->code & (1<<EVENT_KDGL))
			WriteActEvent(&(pDBEvt->Time), EVENT_KDGL, fd, "下级拒动,%s扩大故障隔离区域跳闸", pCfg->kgname);
		else if(pDBEvt->code & (1<<EVENT_ZDBS))
			WriteActEvent(&(pDBEvt->Time), EVENT_ZDBS, fd, "%s%s", pCfg->kgname, tEventTable[EVENT_ZDBS].name);
		else if(pDBEvt->code & (1<<EVENT_YXTZ))
			WriteActEvent(&(pDBEvt->Time), EVENT_YXTZ, fd, "%s%s", pCfg->kgname, tEventTable[EVENT_YXTZ].name);
		else if(pDBEvt->code & (1<<EVENT_MXQY))
			WriteActEvent(&(pDBEvt->Time), EVENT_MXQY, fd, "%s%s", pCfg->kgname, tEventTable[EVENT_MXQY].name);
		else
			 WriteWarnEvent(NULL, 0, fd,  "%s未知事件     ", pCfg->kgname);
		prEvtOpt->wReadPtr = (prEvtOpt->wReadPtr+1)&(MAX_EVT_NUM-1);
	}

    if (g_prPubInit == 0) return;

    while(prYxPubOpt->wReadPtr != prYxPubOpt->wWritePtr)
	{
        pDBYxChg = prYxPubOpt->yxchg + prYxPubOpt->wReadPtr;
		pDBSOE   = &(pDBYxChg->DBSOE);	
        DBCOS.wNo = pDBSOE->wNo; DBCOS.byValue = pDBSOE->byValue;
		
        WriteSSOE(BM_EQP_MYIO, pDBSOE->wNo, pDBSOE->byValue, pDBSOE->Time);
		WriteSYX(BM_EQP_MYIO, DBCOS.wNo, DBCOS.byValue);

		prYxPubOpt->wReadPtr = (prYxPubOpt->wReadPtr+1)&(MAX_YX_PUB_NUM-1);
	}


	while(prEvtPubOpt->wReadPtr != prEvtPubOpt->wWritePtr)
	{
	    pDBEvt = prEvtPubOpt->evt + prEvtPubOpt->wReadPtr;

		pCfg = pFdCfg + pDBEvt->fd;

		fd = pDBEvt->fd + 1;

		if(pDBEvt->code & (1<<EVENT_DF))
		{
		   if (pDBEvt->data[0] == 1)
			   strcpy(szDf, "解列");
		   else
			   strcpy(szDf, "减载");
		   WriteActEvent(&(pDBEvt->Time), (EVENT_DF+EVENT_PUB), fd, "%s%s动作\n", pCfg->kgname, tEventPubTable[EVENT_DF].name);
		}
		if(pDBEvt->code & (1<<EVENT_SHTQ))
	 	   WriteActEvent(&(pDBEvt->Time), (EVENT_SHTQ+EVENT_PUB), fd, "%s%s\n", pCfg->kgname, tEventPubTable[EVENT_SHTQ].name);
	    if(pDBEvt->code & (1<<(EVENT_U0)))
	       WriteActEvent(&(pDBEvt->Time), (EVENT_U0+EVENT_PUB), fd, "%s%s\n", pCfg->kgname, tEventPubTable[EVENT_U0].name);
		if(pDBEvt->code & (1<<(EVENT_DY_HZ)))
		   WriteActEvent(&(pDBEvt->Time), (EVENT_DY_HZ+EVENT_PUB), fd, "%s%s", pCfg->kgname, tEventPubTable[EVENT_DY_HZ].name);
		if(pDBEvt->code & (1<<(EVENT_SY_HZ)))
		   WriteActEvent(&(pDBEvt->Time), (EVENT_SY_HZ+EVENT_PUB), fd, "%s%s", pCfg->kgname, tEventPubTable[EVENT_SY_HZ].name);
	    if(pDBEvt->code & (1<<(EVENT_UDIFF)))
		   WriteActEvent(&(pDBEvt->Time), (EVENT_UDIFF+EVENT_PUB), fd, "%s%s", pCfg->kgname, tEventPubTable[EVENT_UDIFF].name);
		if(pDBEvt->code & (1<<(EVENT_SL_WY)))
		   WriteActEvent(&(pDBEvt->Time), (EVENT_SL_WY+EVENT_PUB), fd, "%s%s", pCfg->kgname, tEventPubTable[EVENT_SL_WY].name);
		if(pDBEvt->code & (1<<(EVENT_I_CNT)))
		   WriteActEvent(&(pDBEvt->Time), (EVENT_I_CNT+EVENT_PUB), fd, "%s%s", pCfg->kgname, tEventPubTable[EVENT_I_CNT].name);
		if(pDBEvt->code & (1<<(EVENT_I_TRIP)))
		   WriteActEvent(&(pDBEvt->Time), (EVENT_I_TRIP+EVENT_PUB), fd, "%s%s", pCfg->kgname, tEventPubTable[EVENT_I_TRIP].name);

	    prEvtPubOpt->wReadPtr = (prEvtPubOpt->wReadPtr+1)&(MAX_EVT_PUB_NUM-1);

	}

}

#ifdef _GUANGZHOU_TEST_FTU_
void prLightHandle(void)
{
    DWORD light;
    light = 0;

	if (GetMyDiValue(pFdCfg->kg_stateno)) 
	{
	   turnLight(BSP_LIGHT_KG_ID, 1);
	   turnLight(BSP_LIGHT_KGF_ID, 0);
	}
	else
	{
	   turnLight(BSP_LIGHT_KGF_ID, 1);
	   turnLight(BSP_LIGHT_KG_ID, 0);
	}

	if (prRunInfo[0].vyx & PR_VYX_I_MASK)
		turnLight(BSP_LIGHT_I_ID, 1);
	else
		turnLight(BSP_LIGHT_I_ID, 0);
	if (prRunInfo[0].vyx & PR_VYX_I0_MASK)
		turnLight(BSP_LIGHT_I0_ID, 1);
	else
		turnLight(BSP_LIGHT_I0_ID, 0);
#if(TYPE_USER != USER_GUANGXI)
	if (prRunPublic[0].vyx & PR_VYXP_TIE)
		turnLight(BSP_LIGHT_PRBS_ID, 1);
	else
		turnLight(BSP_LIGHT_PRBS_ID, 0);
	
	if ((prRunInfo[0].vyx & PR_VYX_HZ_BS)||(prRunPublic[0].vyx & PR_VYXP_HZ_BS))
		turnLight(BSP_LIGHT_CHBS_ID, 1);
	else
		turnLight(BSP_LIGHT_CHBS_ID, 0);
		
#else
	
	if ((prRunInfo[0].vyx & PR_VYX_HZ_BS)||(prRunPublic[0].vyx & PR_VYXP_HZ_BS))
		turnLight(BSP_LIGHT_PRBS_ID, 1);
	else
		turnLight(BSP_LIGHT_PRBS_ID, 0);
	
#endif

}
#else
void prLightHandle(void)
{
	int i;
	static DWORD light_state;
	VPrRunSet *pprRunSet;
	LogicElem * ppLogicElem;
	BOOL bSgz;
#if (TYPE_USER == USER_BEIJING)
	BOOL bSgzz;
    static BOOL kgstate = 0;
#endif
	static int light = 0;
	static BOOL first = FALSE;
	static TIMERELAY* lightTime;

	if (g_prInit == 0) return;
	
	if(first == FALSE)
	{
		lightTime = (TIMERELAY*)malloc((fdNum+ 1)*sizeof(TIMERELAY));
		memset(lightTime,0,(fdNum+ 1)*sizeof(TIMERELAY));
		
		if(lightTime == NULL) return;
		first = TRUE;
		for(i=0;i< fdNum;i++)
		{
			pprRunSet = prRunSet[prRunInfo[i].set_no]+i;
			InitTR(&lightTime[i],pprRunSet->dLedTret,0);
		}
		InitTR(&lightTime[i],pprRunSet->dLedTret,0);
	}

	for(i = 0;i < fdNum;i++) //检测是否在线修改
	{
		pprRunSet = prRunSet[prRunInfo[i].set_no]+i;
		if(lightTime[i].dTripThreshold != pprRunSet->dLedTret)
		{
			InitTR(&lightTime[i],pprRunSet->dLedTret,0);
			ResetTR(&lightTime[i]);
		}
	}
	if(lightTime[i].dTripThreshold != pprRunSet->dLedTret)
	{
		InitTR(&lightTime[i],pprRunSet->dLedTret,0);
		ResetTR(&lightTime[i]);
	}
	
	for (i=0; i<fdNum; i++)
	{
		pprRunSet = prRunSet[prRunInfo[i].set_no]+i;
		ppLogicElem = prLogicElem+i*GRAPH_NUM;
			
		if (!(pprRunSet->dYb&PR_YB_GZJC_EN)) continue;
		

		bSgz =ppLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_TRIP] ||ppLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_TRIP] || ppLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_TRIP]
				||ppLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_I] || ppLogicElem[GRAPH_I02].Output[BOOL_OUTPUT_I] || ppLogicElem[GRAPH_I03].Output[BOOL_OUTPUT_I]||(prRunInfo[i].vyx&PR_VYX_I0C);
	 	       ; //修改，取一段、二段、零序的动作
		
		//if (prRunInfo[i].vyx & prRunInfo[i].vyx_warn_mask) light |= (1<<BSP_LIGHT_WARN_ID); //告警灯只针对装置告警

	
  #if(DEV_SP == DEV_SP_DTU)
		if ((prRunInfo[i].vyx & (PR_VYX_I_MASK | PR_VYX_I0_MASK)) && bSgz) 
		     light |= (1<<(BSP_LIGHT_FAULT1_ID + i));
			else
		{
			if(pprRunSet->bLedZdFg)
			{
				if((lightTime[i].boolTrip) && (light & (1<<(BSP_LIGHT_FAULT1_ID+i))))
					light &= ~(1<<(BSP_LIGHT_FAULT1_ID+i));
			}
			else
				light &= ~(1<<(BSP_LIGHT_FAULT1_ID+i));		
			if(prRunInfo[i].reset == 0xffff)
				light &= ~(1<<(BSP_LIGHT_FAULT1_ID+i));
		}
		RunTR(&lightTime[i],(light & (1<<(BSP_LIGHT_FAULT1_ID+i))) &&(!bSgz));	
  #elif ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_DTU_IU))
  #if  (TYPE_USER == USER_GUANGXI)
    	if (prRunInfo[i].vyx & PR_VYX_I_MASK) 
		     light |= (1<<BSP_LIGHT_I_ID);
		else
			 light &= ~(1<<BSP_LIGHT_I_ID);
		if (prRunInfo[i].vyx & PR_VYX_I02) 
		     light |= (1<<BSP_LIGHT_I0_ID);
		else
			light &= ~(1<<BSP_LIGHT_I0_ID);
  #else
    	if ((prRunInfo[i].vyx & (PR_VYX_I_MASK)) && bSgz) 
			light |= (1<<BSP_LIGHT_I_ID);
		else
		{
			if(pprRunSet->bLedZdFg)
			{
				if((lightTime[0].boolTrip) && (light & (1<<BSP_LIGHT_I_ID)))
					light &= ~(1<<BSP_LIGHT_I_ID);
			}
			else
				light &= ~(1<<BSP_LIGHT_I_ID);		
			if(prRunInfo[i].reset == 0xffff)
				light &= ~(1<<BSP_LIGHT_I_ID);		
		}
		RunTR(&lightTime[0],(light & (1<<BSP_LIGHT_I_ID)) &&(!bSgz));
	
		if ((prRunInfo[i].vyx & (PR_VYX_I0_MASK)) && bSgz) 
			light |= (1<<BSP_LIGHT_I0_ID);
		else
		{
			if(pprRunSet->bLedZdFg)
			{
				if((lightTime[1].boolTrip) && (light & (1<<BSP_LIGHT_I0_ID)))
					light &= ~(1<<BSP_LIGHT_I0_ID);
			}
			else
				light &= ~(1<<BSP_LIGHT_I0_ID);
			if(prRunInfo[i].reset == 0xffff)
					light &=  ~(1<<BSP_LIGHT_I0_ID);
		}
		RunTR(&lightTime[1],(light & (1<<BSP_LIGHT_I0_ID)) &&(!bSgz));
	
		if (pprRunSet->dYb & (PR_YB_I1|PR_YB_I2|PR_YB_I3)) 
			light |= (1<<BSP_LIGHT_IYB_ID);
		else 
			light &= ~(1<<BSP_LIGHT_IYB_ID);

		if (pprRunSet->dYb & (PR_YB_I01|PR_YB_I02|PR_YB_I03)) light |= (1<<BSP_LIGHT_I0YB_ID);
		else 
			light &= ~(1<<BSP_LIGHT_I0YB_ID);
  #endif

        if (pprRunSet->dYb & (PR_YB_I1|PR_YB_I2|PR_YB_I3)) 
		light |= (1<<BSP_LIGHT_IYB_ID);

		if (pprRunSet->dYb & (PR_YB_I01|PR_YB_I02|PR_YB_I03)) 
		light |= (1<<BSP_LIGHT_I0YB_ID);
 
  #elif (DEV_SP == DEV_SP_WDG)
   		if( (prRunInfo[i].vyx & PR_VYX_I_MASK) && bSgz) 
			light |= (1<<BSP_LIGHT_I_ID);
		else
		{
			if(pprRunSet->bLedZdFg)
			{
				if((lightTime[0].boolTrip) && (light & (1<<BSP_LIGHT_I_ID)))
					light &= ~(1<<BSP_LIGHT_I_ID);
			}
			else
				light &= ~(1<<BSP_LIGHT_I_ID);		
			
			if(prRunInfo[i].reset == 0xffff)
				light &= ~(1<<BSP_LIGHT_I_ID);
		}
		RunTR(&lightTime[0],(light & (1<<BSP_LIGHT_I_ID)) &&(!bSgz));
	

		if ((prRunInfo[i].vyx & (PR_VYX_I0_MASK )) && bSgz) light |= (1<<BSP_LIGHT_I0_ID);
		else
		{
			if(pprRunSet->bLedZdFg)
			{
				if((lightTime[1].boolTrip) && (light & (1<<BSP_LIGHT_I0_ID)))
					light &= ~(1<<BSP_LIGHT_I0_ID);
			}
			else
				light &= ~(1<<BSP_LIGHT_I0_ID);		
			if(prRunInfo[i].reset == 0xffff)
				light &= ~(1<<BSP_LIGHT_I0_ID);
		}
		RunTR(&lightTime[1],(light & (1<<BSP_LIGHT_I0_ID)) &&(!bSgz));
	
		if (prRunInfo[i].vyx & PR_VYX_HZ_BS) 
			light |= (1<<BSP_LIGHT_BS_ID);
		else
			light &= ~(1<<BSP_LIGHT_BS_ID);
		if(g_wdg_ver)
		{
			if (prRunInfo[i].vyx & PR_VYX_RC) 
				light |= (1<<BSP_LIGHT_CH_ID);
			else
				light &= ~(1<<BSP_LIGHT_CH_ID);
		}

#if (TYPE_USER != USER_BEIJING)	
		if (prRunInfo[i].vyx & PR_VYX_SGZ) light |= (1<<BSP_LIGHT_FAULT_ID);
		else
			light &= ~(1<<BSP_LIGHT_FAULT_ID);	
#else
         //北京合闸后告警灯 遥信复归,直接判位置信号
		if((!kgstate) && GetMyDiValue(pFdCfg->kg_stateno) && (!bSgz))
		{
			light &= ~((1<<BSP_LIGHT_I0_ID) |(1<<BSP_LIGHT_I_ID));
			pRunInfo->vyx &= ~(PR_VYX_I0_MASK | PR_VYX_I_MASK | PR_VYX_SGZ );
		}
		kgstate = GetMyDiValue(pFdCfg->kg_stateno);
#endif	
#endif
	}		


#if(DEV_SP == DEV_SP_DTU)
	for (i=0; i<fdNum; i++)
	{
		if (light & (1<<(BSP_LIGHT_FAULT1_ID + i)))
		{
			if(light_state & (1<<(BSP_LIGHT_FAULT1_ID + i)))
				turnLight(BSP_LIGHT_FAULT1_ID + i, 1);
			else
				turnLight(BSP_LIGHT_FAULT1_ID + i, 0);
			light_state ^= 1<<(BSP_LIGHT_FAULT1_ID + i);
#if (TYPE_USER == USER_BEIJING)
			bSgzz = 1;
#endif
		}
		else
		{
			turnLight(BSP_LIGHT_FAULT1_ID + i, 0);
		}
	}
#if (TYPE_USER == USER_BEIJING)
	if(bSgzz == 1)
		turnLight(BSP_LIGHT_FADZ_ID, 1);
	else
		turnLight(BSP_LIGHT_FADZ_ID , 0);
#endif
#elif (DEV_SP == DEV_SP_FTU)
	i = 0;
	if (GetMyDiValue(pFdCfg->kg_stateno)) 
		light |= (1<<BSP_LIGHT_KG_ID);
	else
		light &= ~(1<<BSP_LIGHT_KG_ID);

	if (light & (1<<BSP_LIGHT_I_ID))
	{
		if(light_state & (1<<BSP_LIGHT_I_ID))
			turnLight(BSP_LIGHT_I_ID, 1);
		else
			turnLight(BSP_LIGHT_I_ID, 0);
		light_state ^= 1<<(BSP_LIGHT_I_ID);
	}
	else
	{
		turnLight(BSP_LIGHT_I_ID, 0);
	}

	if (light & (1<<BSP_LIGHT_I0_ID))
	{
		if(light_state & (1<<BSP_LIGHT_I0_ID))
			turnLight(BSP_LIGHT_I0_ID, 1);
		else
			turnLight(BSP_LIGHT_I0_ID, 0);
		light_state ^= 1<<(BSP_LIGHT_I0_ID);
	}
	else
	{
		turnLight(BSP_LIGHT_I0_ID, 0);
	}

	if(GetHzbsYx())
	{
#if(TYPE_USER == USER_GUIYANG)
		
	  if(light_state & (1<<BSP_LIGHT_HZBS_ID))
	   turnLight(BSP_LIGHT_HZBS_ID, 1);
	  else
		turnLight(BSP_LIGHT_HZBS_ID, 0);
	  light_state ^= 1<<(BSP_LIGHT_HZBS_ID);
	
#else
		turnLight(BSP_LIGHT_HZBS_ID,0);  // 防止点不亮cjl
		turnLight(BSP_LIGHT_HZBS_ID,1);
#endif

	}
	else 
		turnLight(BSP_LIGHT_HZBS_ID,0);

	if (light & (1<<BSP_LIGHT_IYB_ID))
		turnLight(BSP_LIGHT_IYB_ID, 1);
	else
		turnLight(BSP_LIGHT_IYB_ID, 0);

	if (light & (1<<BSP_LIGHT_I0YB_ID))
		turnLight(BSP_LIGHT_I0YB_ID, 1);
	else
		turnLight(BSP_LIGHT_I0YB_ID, 0);

	if (light & (1<<BSP_LIGHT_KG_ID))
	{
	    turnLight(BSP_LIGHT_KG_ID, 1);

	    turnLight(BSP_LIGHT_KGF_ID, 0);
	}
	else
	{
	    turnLight(BSP_LIGHT_KG_ID, 0);
		turnLight(BSP_LIGHT_KGF_ID, 1);
	}

#elif (DEV_SP == DEV_SP_WDG)
	i = 0;
	if (GetMyDiValue(pFdCfg->kg_stateno)) 
		light |= (1<<BSP_LIGHT_KG_ID);
	else
		light &= ~(1<<BSP_LIGHT_KG_ID);


	if (light & (1<<BSP_LIGHT_I_ID))
	{
      if(g_wdg_ver)
      {
        turnLight(BSP_LIGHT_I_ID, 1);
      }
      else
      {
        if(light_state & (1<<BSP_LIGHT_I_ID))
          turnLight(BSP_LIGHT_I_ID, 1);
        else
          turnLight(BSP_LIGHT_I_ID, 0);
        light_state ^= 1<<(BSP_LIGHT_I_ID);
      }
	}
	else
	{
		turnLight(BSP_LIGHT_I_ID, 0);
	}

	if (light & (1<<BSP_LIGHT_I0_ID))
	{
      if(g_wdg_ver)
      {
        turnLight(BSP_LIGHT_I0_ID, 1);
      }
      else
      {
        if(light_state & (1<<BSP_LIGHT_I0_ID))
         turnLight(BSP_LIGHT_I0_ID, 1);
        else
          turnLight(BSP_LIGHT_I0_ID, 0);
        light_state ^= 1<<(BSP_LIGHT_I0_ID);
      }
	}
	else
	{
		turnLight(BSP_LIGHT_I0_ID, 0);
	}

	if(GetHzbsYx())
	{
	#if(TYPE_USER == USER_GUIYANG)
		
	  if(light_state & (1<<BSP_LIGHT_HZBS_ID))
	   turnLight(BSP_LIGHT_HZBS_ID, 1);
	  else
		turnLight(BSP_LIGHT_HZBS_ID, 0);
	  light_state ^= 1<<(BSP_LIGHT_HZBS_ID);
	
	#else
		turnLight(BSP_LIGHT_HZBS_ID,0);  // 防止点不亮cjl
		turnLight(BSP_LIGHT_HZBS_ID,1);
	#endif
	}
	else 
		turnLight(BSP_LIGHT_HZBS_ID,0);

	if (light & (1<<BSP_LIGHT_KG_ID))
	    turnLight(BSP_LIGHT_KG_ID, 1);
	else
	    turnLight(BSP_LIGHT_KG_ID, 0);

	if (light & (1<<BSP_LIGHT_BS_ID))
	    turnLight(BSP_LIGHT_BS_ID, 1);
	else
	    turnLight(BSP_LIGHT_BS_ID, 0);

#if (TYPE_USER != USER_BEIJING)
  if(g_wdg_ver)
	{
		if (light &(1<<BSP_LIGHT_CH_ID))   //新罩式点亮重合灯 
			turnLight(BSP_LIGHT_CH_ID, 1);
		else
			turnLight(BSP_LIGHT_CH_ID, 0);
	}
#endif

	
#if (TYPE_USER == USER_BEIJING) //北京将本地灯改为闭锁灯
   if(GetHzbsYx())
		turnLight(BSP_LIGHT_FAULT_ID, 1);
	else
		turnLight(BSP_LIGHT_FAULT_ID, 0);
	if(g_wdg_ver == 0)
	{		
		if(bFK == 0)                   //北京辅开灯,与闭锁互斥
			turnLight(BSP_LIGHT_LOCAL_ID, 0);
		else
			turnLight(BSP_LIGHT_LOCAL_ID, 1);
	}
	else
	{
		if(bFK == 0)                   //北京辅开灯,新罩式用重合灯
			turnLight(BSP_LIGHT_CH_ID, 0);
		else
			turnLight(BSP_LIGHT_CH_ID, 1);
		turnLight(BSP_LIGHT_BY_ID, 1);  //备用用作电源灯
	}
#else
	if (light & (1<<BSP_LIGHT_FAULT_ID))
	{
		if(light_state & (1<<BSP_LIGHT_FAULT_ID))
			turnLight(BSP_LIGHT_FAULT_ID, 1);
		else
			turnLight(BSP_LIGHT_FAULT_ID, 0);
		light_state ^= 1<<(BSP_LIGHT_FAULT_ID);
	}
	else
	{
		turnLight(BSP_LIGHT_FAULT_ID, 0);
	}
#endif
#elif(DEV_SP == DEV_SP_DTU_IU)
	i = 0;
	if (GetMyDiValue(pFdCfg->kg_stateno)) 
		light |= (1<<BSP_LIGHT_KG_ID);
	else
		light &= ~(1<<BSP_LIGHT_KG_ID);

	if (light & (1<<BSP_LIGHT_I_ID))
	{
		if(light_state & (1<<BSP_LIGHT_I_ID))
			turnLight(BSP_LIGHT_I_ID, 1);
		else
			turnLight(BSP_LIGHT_I_ID, 0);
		light_state ^= 1<<(BSP_LIGHT_I_ID);
	}
	else
	{
		turnLight(BSP_LIGHT_I_ID, 0);
	}

	if (light & (1<<BSP_LIGHT_I0_ID))
	{
		if(light_state & (1<<BSP_LIGHT_I0_ID))
			turnLight(BSP_LIGHT_I0_ID, 1);
		else
			turnLight(BSP_LIGHT_I0_ID, 0);
		light_state ^= 1<<(BSP_LIGHT_I0_ID);
	}
	else
	{
		turnLight(BSP_LIGHT_I0_ID, 0);
	}

	if (light & (1<<BSP_LIGHT_KG_ID))
	{
	    turnLight(BSP_LIGHT_KG_ID, 1);

	    turnLight(BSP_LIGHT_KGF_ID, 0);
	}
	else
	{
	    turnLight(BSP_LIGHT_KG_ID, 0);
		turnLight(BSP_LIGHT_KGF_ID, 1);
	}
#endif
}
#endif

void prWavRecord(int fd, struct VCalClock *time, int flag )
{
#ifdef INCLUDE_RECORD
	StartWavRecord(fd, time, flag);
#endif
	return;
}



#if  defined(INCLUDE_FA)||defined(INCLUDE_FA_SH)

DWORD prGetStatus(int fd)
{
    DWORD type = 0;
    VPrRunInfo *pRunInfo;

	pRunInfo = prRunInfo + fd -1;

	if (pRunInfo->vyx & PR_VYX_FA_I)
		type |= FA_KG_IFAULT|FA_KG_WAVE;
	if (pRunInfo->vyx & PR_VYX_FA_I0)
		type |= FA_KG_I0FAULT;
	if (pRunInfo->vyx & PR_VYX_FA_I0C)
		type |= FA_KG_I0CFAULT;
	if (pRunInfo->vyx & PR_VYX_FX)
		type |= FA_KG_ANG;
	
	return type;
}

void prFaVyxHandle(DWORD var, DWORD dYxValue)
{
#ifdef INCLUDE_FA_IDIR
    struct VFaInfo fainfo;
#endif
    if ((var & PR_VYX_FA_I) && (dYxValue & PR_VYX_FA_I))
    {

#ifdef INCLUDE_FA_IDIR
        if (dYxValue & PR_VYX_IA)
           fainfo.phase = MMI_MEA_IA;
		else if (dYxValue & PR_VYX_IB)
		   fainfo.phase = MMI_MEA_IB;
		else 
		   fainfo.phase = MMI_MEA_IC;
		fainfo.faulttime = pRunInfo->fault.i_clock;
	    FaWave_Start(pRunInfo->fd, &fainfo);
#endif

	   if (pRunSet->bITrip == PR_OC_TRIP)
	      NotifyFault(pRunInfo->fd, FA_KG_IFAULT, 1);
    }
	if ((var & PR_VYX_FA_I0) && (dYxValue & PR_VYX_FA_I0))
	{
#ifdef INCLUDE_FA_IDIR
		fainfo.phase = MMI_MEA_I0;
		fainfo.faulttime = pRunInfo->fault.i_clock;
		FaWave_Start(pRunInfo->fd, &fainfo);
#endif

		if ((pRunSet->bITrip == PR_OC_TRIP) && !pRunSet->bI0Trip)
			NotifyFault(pRunInfo->fd, FA_KG_I0FAULT, 1);
	}
	if ((pRunSet->bITrip == PR_OC_NUI_TRIP)||(pRunSet->bITrip == PR_OC_NUI2_TRIP))
	{
		if ((var & PR_VYX_FA_WY) && (dYxValue & PR_VYX_FA_WY))
		{
		   if (dYxValue & PR_VYX_FA_I)
		      NotifyFault(pRunInfo->fd, FA_KG_IFAULT, 1);
		   if (dYxValue & PR_VYX_FA_I0)
		   	  NotifyFault(pRunInfo->fd, FA_KG_I0FAULT, 1);
		   if (dYxValue & PR_VYX_FX)
		   	  NotifyFault(pRunInfo->fd, FA_KG_ANG, 1);
		}
	}
	if ((var & PR_VYX_FX) && (dYxValue & PR_VYX_FX))
		NotifyFault(pRunInfo->fd, FA_KG_ANG, 1);

}

void prFaStsHandle(DWORD dYxValue)
{
    DWORD type;

    if ((pRunInfo->fault.i0_xdltrip) && (!(pRunInfo->fa_states & 0x80000000))  )
	{
		NotifyFault(pRunInfo->fd, FA_KG_I0CFAULT, 1);
		pRunInfo->fa_states |= 0x80000000;
    }
	if ((!pRunInfo->fault.i0_xdltrip) && (pRunInfo->fa_states & 0x80000000))
	{
	   pRunInfo->fa_states &= ~0x80000000; 
	}

	type = prLogicFaStatus(pRunInfo->fd);
	if ((type & FA_KG_FAULT) && pRunInfo->normal_sts && !pRunInfo->normal_sts_bak)
    {
        NotifyFault(pRunInfo->fd, FA_KG_IFAULT, 0);
		NotifyFault(pRunInfo->fd, FA_KG_I0FAULT, 0);
		NotifyFault(pRunInfo->fd, FA_KG_I0CFAULT, 0);
	}
	pRunInfo->normal_sts_bak = pRunInfo->normal_sts;
}
#endif

#ifdef INCLUDE_FA_DIFF
void prFaDiffVyxHandle(DWORD var, DWORD dYxValue)
{
#ifdef INCLUDE_FA_IDIR
    struct VFaInfo fainfo;
#endif
    if (var & PR_VYX_FA_I)
    {
       if (dYxValue & PR_VYX_FA_I)
          NotifyFaDiffFault(pRunInfo->fd, FADIFF_FAULT_I, 1);
	   else
	      NotifyFaDiffFault(pRunInfo->fd, FADIFF_FAULT_I, 0);
    }
	if (var & PR_VYX_FA_I0)
    {
        if (dYxValue & PR_VYX_FA_I0)
           NotifyFaDiffFault(pRunInfo->fd, FADIFF_FAULT_I0, 1);
		else
		   NotifyFaDiffFault(pRunInfo->fd, FADIFF_FAULT_I0, 0);
			
	}
	if (var & PR_VYX_FX)
	{
	    if ((dYxValue & PR_VYX_FA_I)&&(dYxValue & PR_VYX_FX))
	       NotifyFaDiffFault(pRunInfo->fd, FADIFF_FAULT_I_ANG, 1);
		else 
		   NotifyFaDiffFault(pRunInfo->fd, FADIFF_FAULT_I_ANG, 0);

		if ((dYxValue & PR_VYX_FA_I0)&&(dYxValue & PR_VYX_FX))
	       NotifyFaDiffFault(pRunInfo->fd, FADIFF_FAULT_I0_ANG, 1);
		else 
		   NotifyFaDiffFault(pRunInfo->fd, FADIFF_FAULT_I0_ANG, 0);
	}

}
#endif


void prVyxHandle(void)
{
	DWORD var,mask; int i;
	VDBYxChg *pYxChg;
	struct VDBSOE *pDBSOE;
	BYTE value; WORD no;
	DWORD dYxValue;

	dYxValue = pRunInfo->vyx;

	if (dYxValue != pRunInfo->vyx_bak)
	{
		
		var = dYxValue^pRunInfo->vyx_bak;
		mask = 1;
		for (i=0; i<PR_VYX_NUM; i++)
		{
			if (var & mask)
			{
			    pYxChg = prYxOpt->yxchg + prYxOpt->wWritePtr;
				pYxChg->flag = 0;
				pDBSOE = &(pYxChg->DBSOE);
			    GetSysClock(&(pDBSOE->Time), CALCLOCK);
				no = pRunInfo->vyx_start_no+i;
				if (dYxValue & mask) value = 0x81;
				else value = 0x01;

#if (TYPE_USER == USER_GUANGXI)
                if(value == 0x81)
                {
					if(var&mask&PR_VYX_SGZ)
					   pDBSOE->Time = pRunInfo->fault.sgz_clock;
					else if(var&mask&(PR_VYX_TR_MASK|PR_VYX_CL_MASK))
					   pDBSOE->Time = pRunInfo->fault.yk_clock;
                }
#else
                if(value == 0x81)
                {
                    if(var&mask&PR_VYX_BHQD)
					   pDBSOE->Time = pRunInfo->fault.i_start_clock;
					else if(var&mask&PR_VYX_SGZ)
					   pDBSOE->Time = pRunInfo->fault.sgz_clock;
					else if(var&mask&(PR_VYX_TR_MASK|PR_VYX_CL_MASK|PR_VYX_HZ_BS|PR_VYX_FZ_BS |PR_VYX_YK_YB))
					   pDBSOE->Time = pRunInfo->fault.yk_clock;
					else if(!(var&mask&PR_VYX_STS_MASK))
					   pDBSOE->Time = pRunInfo->fault.i_clock;
                }
#endif		

				pDBSOE->wNo = no; pDBSOE->byValue = value;	

				prYxOpt->wWritePtr = (prYxOpt->wWritePtr+1)&(MAX_YX_NUM-1);
				
    		}
			mask <<= 1;		
		}
#if defined(INCLUDE_FA)||defined(INCLUDE_FA_SH)
    prFaVyxHandle(var, dYxValue);
#endif
#ifdef INCLUDE_FA_DIFF
    prFaDiffVyxHandle(var, dYxValue);
#endif
 	pRunInfo->vyx_bak = dYxValue;
	}	
#ifdef INCLUDE_FA
	prFaStsHandle(dYxValue);
#endif
}

#ifdef INCLUDE_COMTRADE
void prComtradeStop(void)
{
   pRunInfo->wave.start = FALSE;
   pRunInfo->wave.stop  = FALSE;
   pRunInfo->wave.connect_pre = FALSE;
   pRunInfo->wave.connect_next= FALSE;
}
#endif

int  prSKgDo(int no)
{
    int ret;
#ifdef INCLUDE_COMTRADE
	BYTE bwave=0;
#endif
    GetSysClock(&(pRunInfo->fault.yk_clock), CALCLOCK);
	ret = ykOutput(pCfg->kg_ykno, no|PRYK);		
	if(2 == ret)
	{
		ykCancel(-1,pCfg->kg_ykno, 1);//To prevent Close State is running.2009-09-22
		ykCancel(-1,pCfg->kg_ykno, 0);//已经在执行跳
		ret = ykOutput(pCfg->kg_ykno, no|PRYK);
	}
    if(no == 0)
	{
#ifdef  INCLUDE_COMTRADE 
        if(!pRunInfo->wave.start)
			bwave = REC_START;
 	    if(pRunInfo->wave.connect_next)
		    StartWavRcd(pRunInfo->fd, &(pRunInfo->fault.yk_clock), bwave|REC_NORMAL);
		else
		{
		    StartWavRcd(pRunInfo->fd, &(pRunInfo->fault.yk_clock), bwave|REC_STOP);
			prComtradeStop();
		}
#endif
        if(pRunInfo->vyx & PR_VYX_TR_MASK)
        {
           pRunInfo->vyx &= ~PR_VYX_TR_MASK;
		   prVyxHandle();
        }
	    pRunInfo->vyx |= PR_VYX_TRIP;
		if (ret) 
			pRunInfo->vyx |= PR_VYX_TRIP_F;

    }
	else
	{  
#ifdef  INCLUDE_COMTRADE 
	    if(!pRunInfo->wave.start)
		   bwave = REC_START;
	    StartWavRcd(pRunInfo->fd, &(pRunInfo->fault.yk_clock), bwave|REC_STOP);
		prComtradeStop();
#endif
        if(pRunInfo->vyx & PR_VYX_TRIP_F)
        {
           pRunInfo->vyx &= ~PR_VYX_TRIP_F;
		   prVyxHandle();
        }
		if (ret) 
			pRunInfo->vyx |= PR_VYX_TRIP_F;
	}
  return ret;
}

int prRunInfoInit(void)
{
    int i, j;
	WORD yxno;
	BYTE ykyb;

	struct VDBYX DBYX;
	
    prRunInfo = (VPrRunInfo *)malloc(fdNum*sizeof(VPrRunInfo));

	if(prRunInfo == NULL)  return ERROR;

	prYxOpt = (VPrYxOpt *)malloc(sizeof(VPrYxOpt));

	if(prYxOpt == NULL) return ERROR;

	prEvtOpt = (VPrEvtOpt *)malloc(sizeof(VPrEvtOpt));

	if(prEvtOpt == NULL) return ERROR;

	fdProtVal = (VFdProtCal *)malloc(fdNum*sizeof(VFdProtCal));
	if(fdProtVal == NULL) return ERROR;


	memset(prYxOpt, 0, sizeof(VPrYxOpt));
	memset(prEvtOpt, 0, sizeof(VPrEvtOpt));

	prGzEvtOpt = (VPrGzEvtOpt*)malloc(sizeof(VPrGzEvtOpt));
	if(prGzEvtOpt == NULL) return ERROR;
	memset(prGzEvtOpt,0,sizeof(VPrGzEvtOpt));

	memset((BYTE*)prRunInfo, 0, fdNum*sizeof(VPrRunInfo));
  	memset((BYTE*)fdProtVal, 0, fdNum*sizeof(VFdProtCal));

	yxno = g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low + PR_VYX_NUM*pubfdNum;
	
#if (TYPE_IO == IO_TYPE_EXTIO)				
	//if (g_AsExtIo)
		softYxStartNoSet(yxno);
#endif
	for (i=0; i<fdNum; i++)
	{
	    pRunInfo = prRunInfo+i;

    	pRunInfo->fd = i;
	
		pRunInfo->vyx_start_no = yxno;
		yxno += PR_VYX_NUM;
    
		DBYX.byValue = 0x01;
		for (j=0; j<PR_VYX_NUM; j++)
		{
			DBYX.wNo = pRunInfo->vyx_start_no+j;
#if (TYPE_IO == IO_TYPE_EXTIO)				
			//if (g_AsExtIo)
			{
				softYxValueWrite(1, &DBYX);
			}
#endif
			WriteSYX(BM_EQP_MYIO, DBYX.wNo,DBYX.byValue);
//			WriteSYX(g_Sys.wIOEqpID, 1, &DBYX);
		}
		//遥控软压板,初始设值，不生成soe，只遥信变位
		DBYX.wNo = pRunInfo->vyx_start_no + PR_VYX_YKYB_YXNO;
		if(ykGetYb(pRunInfo->fd + 1 , 1 , &ykyb))
		{
			if(ykyb == 0x81)
			{
				DBYX.byValue = 0x81;
				pRunInfo->vyx |= PR_VYX_YK_YB;
				
			}
			else
			{
				DBYX.byValue = 0x01;
				pRunInfo->vyx &= ~(PR_VYX_YK_YB);
			}
			WriteSYX(BM_EQP_MYIO, DBYX.wNo,DBYX.byValue);
//			WriteSYX(g_Sys.wIOEqpID, 1, &DBYX);
			pRunInfo->vyx_bak = pRunInfo->vyx;
		}

	}

	return OK;
}	

float prValue_l2f(int ui_flag, VFdCfg *pCfg, long value)
{
    float tmp;

    if (ui_flag <= 3)
		tmp = (float)value*100/pCfg->gain_u[ui_flag-1];
	else if (ui_flag == 4)
	{
		tmp = (float)value*100/pCfg->gain_u0;
         if(g_Sys.MyCfg.dwCfg & 0x10)
             tmp = tmp/10;
	}
	else if (ui_flag <= 7)
		tmp = (float)value*5/pCfg->gain_i[ui_flag-5];
    else if (ui_flag == 8)
		tmp = (float)value*pCfg->In0/pCfg->gain_i0;
	else
		return 0;
    return(tmp);
}


void prWriteEvent(WORD no)
{
	int k;
	DWORD *pdI,*pdU;
	
    VDBEvent *pevt;
	pevt = prEvtOpt->evt + prEvtOpt->wWritePtr;

	pdI = pevt->data;
	pdU = pevt->data + 4;

    pevt->flag = 0;
	pevt->code = 0;

	if(pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP])
		pevt->flag |= PR_EVT_JS;

    pevt->fd = pRunInfo->fd;
	switch(no)
	{
	case EVENT_BHQD:
		 pevt->flag |= PR_EVT_QD;
		 pevt->Time = pRunInfo->fault.i_start_clock;
         if(pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_PICK]|
		 	pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_PICK]|
		 	pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_PICK])
	        pevt->code |= 1<<EVENT_I1;
	     if(pLogicElem[GRAPH_IGFH].Output[BOOL_OUTPUT_PICK])
		    pevt->code |= 1<<EVENT_IGFH;
         if(pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_PICK]|
		 	pLogicElem[GRAPH_I02].Output[BOOL_OUTPUT_PICK]|
		 	pLogicElem[GRAPH_I03].Output[BOOL_OUTPUT_PICK])
		    pevt->code |= 1<<EVENT_I01;
		 if(pLogicElem[GRAPH_DY].Output[BOOL_OUTPUT_PICK])
    	    pevt->code |= 1<<EVENT_DY;
         if(pLogicElem[GRAPH_GY].Output[BOOL_OUTPUT_PICK])
		 	pevt->code |= 1<<EVENT_GY;
		 break;
	case EVENT_TRIP:
		 pevt->flag |= PR_EVT_TRIP;
		 pevt->Time = pRunInfo->fault.yk_clock;
		 for(k=0; k<4; k++)
	     {
	        if(pLogicElem[GRAPH_I1+k].Output[BOOL_OUTPUT_TRIP])
		    {
		      pdI[0] = pRunInfo->fault.i[0];
			  pdI[1] = pRunInfo->fault.i[1];
			  pdI[2] = pRunInfo->fault.i[2];
			  pdI[3] = pRunInfo->fault.i[3];

			  pdU[0] = pRunInfo->fault.u[0];
			  pdU[1] = pRunInfo->fault.u[1];
			  pdU[2] = pRunInfo->fault.u[2];
			  pdU[3] = pRunInfo->fault.u[3];
			  pevt->code |= 1<<(EVENT_I1+k);
		    }
		 }
		 for(k=0; k<3; k++)
	     {
		     if(pLogicElem[GRAPH_I01+k].Output[BOOL_OUTPUT_TRIP])
			 {
				pdI[0] = pRunInfo->fault.i[0];
				pdI[1] = pRunInfo->fault.i[1];
				pdI[2] = pRunInfo->fault.i[2];
				pdI[3] = pRunInfo->fault.i[3];

				pdU[0] = pRunInfo->fault.u[0];
				pdU[1] = pRunInfo->fault.u[1];
				pdU[2] = pRunInfo->fault.u[2];
				pdU[3] = pRunInfo->fault.u[3];
				pevt->code |= 1<<(EVENT_I01+k);
		     }
		 }
		 for(k=0; k<2; k++)
	     {
	        if(pLogicElem[GRAPH_GY+k].Output[BOOL_OUTPUT_TRIP])
		    {
		      pdI[0] = pRunInfo->fault.i[0];
			  pdI[1] = pRunInfo->fault.i[1];
			  pdI[2] = pRunInfo->fault.i[2];
			  pdI[3] = pRunInfo->fault.i[3];

			  pdU[0] = pRunInfo->fault.u[0];
			  pdU[1] = pRunInfo->fault.u[1];
			  pdU[2] = pRunInfo->fault.u[2];
			  pdU[3] = pRunInfo->fault.u[3];
			  pevt->code |= 1<<(EVENT_GY+k);
		    }
		 }
         if(pLogicElem[GRAPH_WYFZ].Output[BOOL_OUTPUT_TRIP])
		 {
		    if(pRunSet->bITrip == PR_OC_NUI_TRIP)
		    { 
		      pevt->flag |= PR_EVT_I_WY;
			  pdI[0] = pRunInfo->fault.i[0];
			  pdI[1] = pRunInfo->fault.i[1];
			  pdI[2] = pRunInfo->fault.i[2];
			  pdI[3] = pRunInfo->fault.i[3];

			  pdU[0] = pRunInfo->fault.u[0];
			  pdU[1] = pRunInfo->fault.u[1];
			  pdU[2] = pRunInfo->fault.u[2];
			  pdU[3] = pRunInfo->fault.u[3];
	          pevt->code |= 1<<EVENT_WY_FZ;
		    }
			else if(pRunSet->bITrip == PR_OC_NUI2_TRIP)
			{
			  pevt->flag |= PR_EVT_I2_WY;
		      pdI[0] = pRunInfo->fault.i[0];
			  pdI[1] = pRunInfo->fault.i[1];
			  pdI[2] = pRunInfo->fault.i[2];
			  pdI[3] = pRunInfo->fault.i[3];

			  pdU[0] = pRunInfo->fault.u[0];
			  pdU[1] = pRunInfo->fault.u[1];
			  pdU[2] = pRunInfo->fault.u[2];
			  pdU[3] = pRunInfo->fault.u[3];
			  pevt->code |= 1<<EVENT_WY_FZ;
			}
			else
			  pevt->code |= 1<<EVENT_WY_FZ;
         }
		 if (pRunInfo->fault.i0_xdltrip)
		 {
		     pdI[0] = pRunInfo->fault.i[0];
			 pdI[1] = pRunInfo->fault.i[1];
			 pdI[2] = pRunInfo->fault.i[2];
			 pdI[3] = pRunInfo->fault.i[3];

			 pdU[0] = pRunInfo->fault.u[0];
			 pdU[1] = pRunInfo->fault.u[1];
			 pdU[2] = pRunInfo->fault.u[2];
			 pdU[3] = pRunInfo->fault.u[3];
			 pevt->code |= 1<<EVENT_I0C;
		 }
		 if(pLogicElem[GRAPH_GP].Output[BOOL_OUTPUT_TRIP])
		{
			 pdI[0] = pRunInfo->fault.i[0];
			  pdI[1] = pRunInfo->fault.i[1];
			  pdI[2] = pRunInfo->fault.i[2];
			  pdI[3] = pRunInfo->fault.i[3];

			  pdU[0] = pRunInfo->fault.u[0];
			  pdU[1] = pRunInfo->fault.u[1];
			  pdU[2] = pRunInfo->fault.u[2];
			  pdU[3] = pRunInfo->fault.u[3];
			  pevt->code |= 1<<EVENT_GP;
		 }
		 if(pLogicElem[GRAPH_DP].Output[BOOL_OUTPUT_TRIP])
		{
			 pdI[0] = pRunInfo->fault.i[0];
			  pdI[1] = pRunInfo->fault.i[1];
			  pdI[2] = pRunInfo->fault.i[2];
			  pdI[3] = pRunInfo->fault.i[3];

			  pdU[0] = pRunInfo->fault.u[0];
			  pdU[1] = pRunInfo->fault.u[1];
			  pdU[2] = pRunInfo->fault.u[2];
			  pdU[3] = pRunInfo->fault.u[3];
			  pevt->code |= 1<<EVENT_DP;
		 }
         if(pLogicElem[GRAPH_YXTZ].Output[BOOL_OUTPUT_TRIP])
        {
			pdI[0] = pRunInfo->fault.i[0];
			pdI[1] = pRunInfo->fault.i[1];
			pdI[2] = pRunInfo->fault.i[2];
			pdI[3] = pRunInfo->fault.i[3];

			pdU[0] = pRunInfo->fault.u[0];
			pdU[1] = pRunInfo->fault.u[1];
			pdU[2] = pRunInfo->fault.u[2];
			pdU[3] = pRunInfo->fault.u[3];
			pevt->code |= 1<<EVENT_YXTZ;
         }

        if(pLogicElem[GRAPH_MXQY].Output[BOOL_OUTPUT_TRIP])
	    {
	      pdI[0] = pRunInfo->fault.i[0];
		  pdI[1] = pRunInfo->fault.i[1];
		  pdI[2] = pRunInfo->fault.i[2];
		  pdI[3] = pRunInfo->fault.i[3];

		  pdU[0] = pRunInfo->fault.u[0];
		  pdU[1] = pRunInfo->fault.u[1];
		  pdU[2] = pRunInfo->fault.u[2];
		  pdU[3] = pRunInfo->fault.u[3];
		  pevt->code |= 1<<EVENT_MXQY;
	    }
		 break;
    case EVENT_CLOSE:
		 pevt->flag |= PR_EVT_RC;
		 pevt->Time = pRunInfo->fault.yk_clock;
		 if(pLogicElem[GRAPH_CH].Output[BOOL_CH_DZ])
		 {
		    pevt->data[0]= pRunInfo->fault.rc_num;
		    pevt->code |= 1<<EVENT_CH;
		 }
		 break;
	case EVENT_HH:
		 pevt->flag |= PR_EVT_HH;
		 pevt->Time = pRunInfo->fault.i_clock;
		 for(k=0; k<4; k++)
	     {
	        if(pLogicElem[GRAPH_I1+k].Output[BOOL_OUTPUT_TRIP])
		    {
		      pdI[0] = pRunInfo->fault.i[0];
			  pdI[1] = pRunInfo->fault.i[1];
			  pdI[2] = pRunInfo->fault.i[2];
			  pdI[3] = pRunInfo->fault.i[3];

			  pdU[0] = pRunInfo->fault.u[0];
			  pdU[1] = pRunInfo->fault.u[1];
			  pdU[2] = pRunInfo->fault.u[2];
			  pdU[3] = pRunInfo->fault.u[3];
			  pevt->code |= 1<<(EVENT_I1+k);
		    }
		 }
		 for(k=0; k<3; k++)
		 {
			 if(pLogicElem[GRAPH_I01+k].Output[BOOL_OUTPUT_TRIP])
			 {
				  pdI[0] = pRunInfo->fault.i[0];
				  pdI[1] = pRunInfo->fault.i[1];
				  pdI[2] = pRunInfo->fault.i[2];
				  pdI[3] = pRunInfo->fault.i[3];

				  pdU[0] = pRunInfo->fault.u[0];
				  pdU[1] = pRunInfo->fault.u[1];
				  pdU[2] = pRunInfo->fault.u[2];
				  pdU[3] = pRunInfo->fault.u[3];
				 pevt->code |= 1<<(EVENT_I01+k);
			 }
		 }
		 if(pLogicElem[GRAPH_PT].Output[BOOL_OUTPUT_TRIP])
		 {
		      pdI[0] = pRunInfo->fault.i[0];
			  pdI[1] = pRunInfo->fault.i[1];
			  pdI[2] = pRunInfo->fault.i[2];
			  pdI[3] = pRunInfo->fault.i[3];

			  pdU[0] = pRunInfo->fault.u[0];
			  pdU[1] = pRunInfo->fault.u[1];
			  pdU[2] = pRunInfo->fault.u[2];
			  pdU[3] = pRunInfo->fault.u[3];
             pevt->code |= 1<<EVENT_PT;
		 }
		 break;
	case EVENT_TGJ:
	case EVENT_HGJ:
	case EVENT_XHFG:
	case EVENT_DJQD:
	case EVENT_KDGL:
	case EVENT_ZDBS:
		 GetSysClock(&(pevt->Time), CALCLOCK);
		 pevt->code |= 1<<no;
		 break;
	case  EVENT_GP:
		pevt->flag |= PR_EVT_HH;
		 GetSysClock(&(pevt->Time), CALCLOCK);
		 pevt->code |= 1<<EVENT_GP;
		break;
        case  EVENT_DP:
		pevt->flag |= PR_EVT_HH;
		 GetSysClock(&(pevt->Time), CALCLOCK);
		 pevt->code |= 1<<EVENT_DP;
		break;
	case EVENT_YXTZ:
		 GetSysClock(&(pevt->Time), CALCLOCK);
		 pevt->code |= 1<<EVENT_YXTZ;
		break;

	case EVENT_MXQY:
	 	GetSysClock(&(pevt->Time), CALCLOCK);
	 	pevt->code |= 1<<EVENT_MXQY;
		break;
	default:
		 pevt->code = 0;
		 break;
	}	
	prEvtOpt->wWritePtr = (prEvtOpt->wWritePtr+1)&(MAX_EVT_NUM-1);
}

void prWriteGzEvent(WORD no)
{
	int k,i;
	DWORD pdI[4],pdU[4];
	//struct VDBYC DBYC;
	DWORD *pfI,*pfU;
	
  VDBGzEvent *pevt;
	pevt = prGzEvtOpt->evt + prGzEvtOpt->wWritePtr;

	pfU = pevt->YcValue;
	pfI = pevt->YcValue+ 4;

  pevt->fd = pRunInfo->fd;
	i = 0;
	pevt->YxNum = 1;
	switch(no)
	{
	case EVENT_I1:
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I1) break;
		}
		pevt->YxNo[0] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[0] = 0x01;
		break;
	case EVENT_I2:
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I2) break;
		}
		pevt->YxNo[0] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[0] = 0x01;
		break;
	case EVENT_I3:
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I3) break;
		}
		pevt->YxNo[0] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[0] = 0x01;
		break;
	case EVENT_I01:
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I0) break;
		}
		pevt->YxNo[0] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[0] = 0x01;
		break;
	case EVENT_I02:
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I02) break;
		}
		pevt->YxNo[0] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[0] = 0x01;
		break;
	case EVENT_I03:
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I0) break;
		}
		pevt->YxNo[0] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[0] = 0x01;
		break;
	case EVENT_I0C:
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I0C) break;
		}
		pevt->YxNo[0] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[0] = 0x01;
		break;
	default:
		pevt->YxNo[0] = 0xffff;
		pevt->YxValue[0] = 0;
		break;
	}
#if (TYPE_USER != USER_FUJIAN)
	//A相过流遥信
	if(pRunInfo->fault.i_phase &  PR_VYX_IA)
	{
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_IA) break;
		}
		pevt->YxNo[pevt->YxNum] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[pevt->YxNum] = 0x01;
		pevt->YxNum ++;
	}
	//B相过流遥信
	if(pRunInfo->fault.i_phase &  PR_VYX_IB)
	{
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_IB) break;
		}
		pevt->YxNo[pevt->YxNum] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[pevt->YxNum] = 0x01;
		pevt->YxNum ++;
	}
	//C相过流
	if(pRunInfo->fault.i_phase &  PR_VYX_IC)
	{
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_IC) break;
		}
		pevt->YxNo[pevt->YxNum] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[pevt->YxNum] = 0x01;
		pevt->YxNum ++;
	}
#else
	if(pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_TRIP] && pRunSet->bI1Trip && (no == EVENT_I1))
	{
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I1DZ) break;
		}
		pevt->YxNo[pevt->YxNum] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[pevt->YxNum] = 0x01;
		pevt->YxNum ++;
	}
	if(pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_TRIP] && pRunSet->bI2Trip && (no == EVENT_I2))
	{
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I2DZ) break;
		}
		pevt->YxNo[pevt->YxNum] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[pevt->YxNum] = 0x01;
		pevt->YxNum ++;
	}
	if(pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_TRIP] && pRunSet->bI3Trip && (no == EVENT_I3))
	{
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I3DZ) break;
		}
		pevt->YxNo[pevt->YxNum] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[pevt->YxNum] = 0x01;
		pevt->YxNum ++;
	}
	if(pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_TRIP] && pRunSet->bI0Trip && (no == EVENT_I01))
	{
		for(i=0;i<PR_VYX_NUM;i++)
		{
			if((1<<i) == PR_VYX_I01DZ) break;
		}
		pevt->YxNo[pevt->YxNum] = pRunInfo->vyx_start_no + i;
		pevt->YxValue[pevt->YxNum] = 0x01;
		pevt->YxNum ++;
	}	
#endif
	
	for(i = 0; i < pevt->YxNum; i++)
	{
		if((pevt->YxNo[i] == 0xffff) && (pevt->YxValue[i] == 0))
			return;
		pevt->Time[i] = pRunInfo->fault.i_clock;
	}
	
	pevt->YcNum = 8;
		
	pevt->YcNo[0] = pCfg->mmi_meaNo_yc[MMI_MEA_UA];
	pevt->YcNo[1] = pCfg->mmi_meaNo_yc[MMI_MEA_UB];
	pevt->YcNo[2] = pCfg->mmi_meaNo_yc[MMI_MEA_UC];
	pevt->YcNo[3] = pCfg->mmi_meaNo_yc[MMI_MEA_U0];
	
	pevt->YcNo[4] = pCfg->mmi_meaNo_yc[MMI_MEA_IA];
	pevt->YcNo[5] = pCfg->mmi_meaNo_yc[MMI_MEA_IB];
	pevt->YcNo[6] = pCfg->mmi_meaNo_yc[MMI_MEA_IC];
	pevt->YcNo[7] = pCfg->mmi_meaNo_yc[MMI_MEA_I0];
	
	for(k=0; k<4; k++) //保护浮点值
	{
		pdI[k] = pRunInfo->fault.i[k];
		pfI[k] = (DWORD)(prValue_l2f(k+5,pCfg,pdI[k])*1000);
		
		pdU[k] = pRunInfo->fault.u[k];
		pfU[k] =(DWORD) (prValue_l2f(k+1,pCfg,pdU[k])*100);
	}
	/*for(k=0;k<8;k++)
	{
		DBYC.wNo = pevt->YcNo[k];
		atomicReadYC(g_Sys.wIOEqpID, 1, sizeof(DBYC), &DBYC, FALSE);
		pevt->YcValue[k] = DBYC.nValue;
	}*/

	
	WriteBmEvent(100, (BYTE *)pevt, sizeof(VDBGzEvent));
	prGzEvtOpt->wWritePtr = (prGzEvtOpt->wWritePtr+1)&(MAX_GZEVT_NUM-1);
}


void AiCalcLogic(LogicElem *plogic)
{
   protValtoFd();
}

#define MY_I1_JS_SET  (plogic->aplUser[0])
#define MY_I1_DJ_SET  (plogic->aplUser[1])
void I1Logic_Init(LogicElem *plogic)
{
    EP_ELEMENT *ppelm = &(plogic->elem);

    MY_I1_JS_SET = 0;
	MY_I1_DJ_SET = 0;
	PRLIB_21(ppelm);
}


void I1Logic(LogicElem *plogic)
{
    BOOL bDy,bPt,bDjqd, bGl, bDir, bGlYx,bDdl;
	int i,phase;
    EP_ELEMENT *pelm = &(plogic->elem);

	if ((pRunSet->bfZdbh)&&
	   ((pVal->dI[0] >= pRunSet->dIKG[0]) ||
        (pVal->dI[1] >= pRunSet->dIKG[1]) ||
        (pVal->dI[2] >= pRunSet->dIKG[2])))
        plogic->Output[BOOL_OUTPUT_BS] = TRUE;
	else
	    plogic->Output[BOOL_OUTPUT_BS] = FALSE;
	
		bDy = (pLogicElem[GRAPH_DY].Output[BOOL_OUTPUT_BS]&pRunSet->bIDYBS)||(!pRunSet->bIDYBS);

    bPt = pLogicElem[GRAPH_PT].Output[BOOL_OUTPUT_PICK]&pRunSet->bPTBS;
    bDy = bPt|bDy;

    bDjqd = pLogicElem[GRAPH_DJQD].Output[BOOL_OUTPUT_PICK]
			 &!pLogicElem[GRAPH_DJQD].Output[BOOL_OUTPUT_TRIP]
			&!plogic->Output[BOOL_OUTPUT_BS];
	pelm->ppioIn[_PR_21_ENABLE].bVal     = ((pRunSet->dYb&PR_YB_I1)>>YB_I1)&bDjqd;
    pelm->ppioIn[_PR_21_A_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_A];
    pelm->ppioIn[_PR_21_B_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_B];
	pelm->ppioIn[_PR_21_C_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_C];
    if(pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP])
    {
        pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI[3][0];
	    pelm->ppioIn[_PR_21_IB_PICK].ulVal   = pRunSet->dI[3][1];
	    pelm->ppioIn[_PR_21_IC_PICK].ulVal   = pRunSet->dI[3][2];
		if(pRunSet->dT[0]> pRunSet->dTJS)
		   pelm->ppioIn[_PR_21_T_PICK].ulVal= pRunSet->dTJS;
        if(MY_I1_JS_SET == 0)
		{
		   pelm->bSetChg = TRUE;
		   MY_I1_JS_SET = 1;
        }
    }
	else if(pLogicElem[GRAPH_DJQD].Output[BOOL_OUTPUT_TRIP])
	{
	    pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI[0][0]*2;
		pelm->ppioIn[_PR_21_IB_PICK].ulVal   = pRunSet->dI[0][1]*2;
		pelm->ppioIn[_PR_21_IC_PICK].ulVal   = pRunSet->dI[0][2]*2;
		pelm->ppioIn[_PR_21_T_PICK].ulVal    = pRunSet->dT[0];
		if(MY_I1_DJ_SET == 0)
		{
		   pelm->bSetChg = TRUE;
		   MY_I1_DJ_SET = 1;
		}
	}
	else
	{
		pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI[0][0];
		pelm->ppioIn[_PR_21_IB_PICK].ulVal   = pRunSet->dI[0][1];
		pelm->ppioIn[_PR_21_IC_PICK].ulVal   = pRunSet->dI[0][2];
		pelm->ppioIn[_PR_21_T_PICK].ulVal    = pRunSet->dT[0];
		if(MY_I1_DJ_SET|MY_I1_JS_SET)
		{
		   pelm->bSetChg = TRUE;
		   MY_I1_JS_SET = 0;
		   MY_I1_DJ_SET = 0;
		}
	}
	pelm->ppioIn[_PR_21_DIR_ENABLE].bVal = ValFALSE;//pRunSet->bIDir;
	pelm->ppioIn[_PR_21_BLOCKED].bVal    = !bDy;
	pelm->ppioIn[_PR_21_IA_DROP].ulVal   = pRunSet->dIRet[0][0];
	pelm->ppioIn[_PR_21_IB_DROP].ulVal   = pRunSet->dIRet[0][1];
	pelm->ppioIn[_PR_21_IC_DROP].ulVal   = pRunSet->dIRet[0][2];
	pelm->ppioIn[_PR_21_T_DROP].ulVal    = pRunSet->dTIRet;
	pelm->ppioIn[_PR_21_INVMODE].bVal    = ValFALSE;
	pelm->ppioIn[_PR_21_I0MODE].bVal     = ValFALSE;
	pelm->ppioIn[_PR_21_CUV].ulVal       = Val0;
   
    plogic->elem.Scan_Func(pelm);
    plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_21_PICKUP].bVal;

    bDdl = bGl = bGlYx = FALSE;
	phase = 0;
	for(i=0; i<3; i++)
	{
	   bDir = (!pRunSet->bIDir)||pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_A+i];
	   pRunInfo->fault.i_sec[i] &= ~0x01;
	   if(pelm->aioOut[_PR_21_A_OPT+i].bVal)
	   {
	      if (bDir|!pRunSet->bIYxDir)
		  {
		     pRunInfo->fault.i_phase |= PR_VYX_IA<<i;
			 pRunInfo->fault.i_sec[i] |= 0x01;
			 bGlYx = TRUE;
		  }
		  if(bDir)
		  	 bGl = TRUE;
		  phase = i;
		  pRunInfo->fault.i[i] = pVal->dI[i];
	   }
	   if( pRunSet->bDdlBsch && ( pRunInfo->fault.i[i] > pRunSet->dCHBSDL))
	   		bDdl = TRUE;
	}

	if (bGlYx && (plogic->Output[BOOL_OUTPUT_I]==0))
	{
        plogic->Output[BOOL_OUTPUT_I] = phase+1;
		pRunInfo->vyx |= PR_VYX_I1;
		if(bDdl && (!pRunInfo->kg_ch_ddlbs) )
		{
			pRunInfo->kg_ch_ddlbs = true;
			pRunInfo->vyx |= PR_VYX_DDL_CHBS;
		}
		GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);

		if(pRunSet->bRcdGlbh)
			prWavRecord(pRunInfo->fd, &(pRunInfo->fault.i_clock), REC_START |REC_STOP);
		
		if(MY_I1_JS_SET)
			pRunInfo->vyx |= PR_VYX_HJS;
	}
	else if (!bGlYx && plogic->Output[BOOL_OUTPUT_I])
		plogic->Output[BOOL_OUTPUT_I] = 0;

    if(bGl&&!plogic->Output[BOOL_OUTPUT_TRIP])
   	{
   	  plogic->Output[BOOL_OUTPUT_TRIP]=TRUE;
	  pRunInfo->fault.i[0] = pVal->dI[0];
	  pRunInfo->fault.i[1] = pVal->dI[1];
	  pRunInfo->fault.i[2] = pVal->dI[2];
	  pRunInfo->fault.i[3] = pVal->dI[3];

	  pRunInfo->fault.u[0] = pVal->dU[0];
	  pRunInfo->fault.u[1] = pVal->dU[1];
	  pRunInfo->fault.u[2] = pVal->dU[2];
	  pRunInfo->fault.u[3] = pVal->dU[3];

	  prWriteGzEvent(EVENT_I1);
    }
	else if(!bGl&&plogic->Output[BOOL_OUTPUT_TRIP])
	  plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
}

#define MY_I2_JS_SET  (plogic->aplUser[0])
void I2Logic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

    MY_I2_JS_SET = 0;
	PRLIB_21(pelm);
}

void I2Logic(LogicElem *plogic)
{
    BOOL bDy,bPt,bDjqd,bGl,bGlYx,bDir,bDdl;
	int i,phase;

    EP_ELEMENT *pelm = &(plogic->elem);
#if (TYPE_USER  == USER_FUJIAN)
		bDy = (pLogicElem[GRAPH_DY].Output[BOOL_OUTPUT_BS]&pRunSet->bI2DYBS)|(!pRunSet->bI2DYBS);
#else
		bDy = (pLogicElem[GRAPH_DY].Output[BOOL_OUTPUT_BS]&pRunSet->bIDYBS)|(!pRunSet->bIDYBS);
#endif
    bPt = pLogicElem[GRAPH_PT].Output[BOOL_OUTPUT_PICK]&pRunSet->bPTBS;
    bDy = bPt|bDy;

    bDjqd = pLogicElem[GRAPH_DJQD].Output[BOOL_OUTPUT_PICK]&
		   !pLogicElem[GRAPH_DJQD].Output[BOOL_OUTPUT_TRIP]&
		   !pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_BS];

    pelm->ppioIn[_PR_21_ENABLE].bVal     = ((pRunSet->dYb&PR_YB_I2)>>YB_I2)&bDjqd;
	pelm->ppioIn[_PR_21_A_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_A];
	pelm->ppioIn[_PR_21_B_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_B];
	pelm->ppioIn[_PR_21_C_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_C];
	if(pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP])
    {
        pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI[3][0];
	    pelm->ppioIn[_PR_21_IB_PICK].ulVal   = pRunSet->dI[3][1];
	    pelm->ppioIn[_PR_21_IC_PICK].ulVal   = pRunSet->dI[3][2];
		if(pRunSet->dT[1]> pRunSet->dTJS)
			pelm->ppioIn[_PR_21_T_PICK].ulVal= pRunSet->dTJS;
		if(MY_I2_JS_SET==0)
		{
		   pelm->bSetChg = TRUE;
		   MY_I2_JS_SET = 1;
        }

    }
	else
	{
		pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI[1][0];
		pelm->ppioIn[_PR_21_IB_PICK].ulVal   = pRunSet->dI[1][1];
		pelm->ppioIn[_PR_21_IC_PICK].ulVal   = pRunSet->dI[1][2];
		pelm->ppioIn[_PR_21_T_PICK].ulVal    = pRunSet->dT[1];
		if(MY_I2_JS_SET==1)
		{
		   pelm->bSetChg = TRUE;
		   MY_I2_JS_SET = 0;
        }

	}
	pelm->ppioIn[_PR_21_DIR_ENABLE].bVal = ValFALSE;//pRunSet->bIDir;
#if (TYPE_USER  == USER_FUJIAN)
	pelm->ppioIn[_PR_21_BLOCKED].bVal    = !bDy;
#else
	pelm->ppioIn[_PR_21_BLOCKED].bVal	 = (!bDy)&pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_BS];
#endif
	pelm->ppioIn[_PR_21_IA_DROP].ulVal   = pRunSet->dIRet[1][0];
	pelm->ppioIn[_PR_21_IB_DROP].ulVal   = pRunSet->dIRet[1][1];
	pelm->ppioIn[_PR_21_IC_DROP].ulVal   = pRunSet->dIRet[1][2];
	pelm->ppioIn[_PR_21_T_DROP].ulVal    = pRunSet->dTIRet;
	pelm->ppioIn[_PR_21_INVMODE].bVal    = ValFALSE;
	pelm->ppioIn[_PR_21_I0MODE].bVal     = ValFALSE;
	pelm->ppioIn[_PR_21_CUV].ulVal       = Val0;
  
    plogic->elem.Scan_Func(pelm);
    plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_21_PICKUP].bVal;

	bDdl = bGl = bGlYx = FALSE;
	for(i=0; i<3; i++)
	{
#if (TYPE_USER  == USER_FUJIAN)
	bDir = (!pRunSet->bI2Dir)|pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_A+i];
#else
	bDir = (!pRunSet->bIDir)|pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_A+i];
#endif   
	   pRunInfo->fault.i_sec[i] &= ~0x02;
	   if(pelm->aioOut[_PR_21_A_OPT+i].bVal)
	   {
	      if (bDir|!pRunSet->bIYxDir)
		  {
		     pRunInfo->fault.i_phase |= PR_VYX_IA<<i;
		     pRunInfo->fault.i_sec[i] |= 0x02;

			 bGlYx = TRUE;
		  }
		  if(bDir)
		  	 bGl = TRUE;
		  phase = i;
		  pRunInfo->fault.i[i] = pVal->dI[i];
	   }
	   if( pRunSet->bDdlBsch && ( pRunInfo->fault.i[i] > pRunSet->dCHBSDL))
	   		bDdl = TRUE;
	}

	if (bGlYx && (plogic->Output[BOOL_OUTPUT_I]==0))
	{
	    plogic->Output[BOOL_OUTPUT_I] = phase+1;
	    pRunInfo->vyx |= PR_VYX_I2;
		if(bDdl && (!pRunInfo->kg_ch_ddlbs) )
		{
			pRunInfo->kg_ch_ddlbs = true;
			pRunInfo->vyx |= PR_VYX_DDL_CHBS;
		}
	    GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
		if(pRunSet->bRcdGlbh)
			prWavRecord(pRunInfo->fd, &(pRunInfo->fault.i_clock), REC_START |REC_STOP);
		if(MY_I2_JS_SET)
			pRunInfo->vyx |= PR_VYX_HJS;
	}
	else if (!bGlYx && plogic->Output[BOOL_OUTPUT_I])
		plogic->Output[BOOL_OUTPUT_I] = 0;
    if(bGl&&!plogic->Output[BOOL_OUTPUT_TRIP])
   	{
   	  plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
 	  pRunInfo->fault.i[0] = pVal->dI[0];
	  pRunInfo->fault.i[1] = pVal->dI[1];
	  pRunInfo->fault.i[2] = pVal->dI[2];
	  pRunInfo->fault.i[3] = pVal->dI[3];

	  pRunInfo->fault.u[0] = pVal->dU[0];
	  pRunInfo->fault.u[1] = pVal->dU[1];
	  pRunInfo->fault.u[2] = pVal->dU[2];
	  pRunInfo->fault.u[3] = pVal->dU[3];

	  if(pRunSet->bDdlBsch&&(pVal->dI[0]>pRunSet->dCHBSDL || pVal->dI[1]>pRunSet->dCHBSDL 
	  	 || pVal->dI[2]>pRunSet->dCHBSDL || pVal->dI[3]>pRunSet->dCHBSDL))
	  	pRunInfo->kg_ch_ddlbs = true;
	  else
	  	pRunInfo->kg_ch_ddlbs = false;

	  prWriteGzEvent(EVENT_I2);
    }
	if(!bGl&&plogic->Output[BOOL_OUTPUT_TRIP])
    {
      plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	}

}

#define MY_I3_JS_SET  (plogic->aplUser[0])
void I3Logic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);
	
    MY_I3_JS_SET = 0;
	PRLIB_21(pelm);
}

void I3Logic(LogicElem *plogic)
{
    BOOL bDy,bPt,bDjqd,bGl,bGlYx,bDir;
	int i,phase;

    EP_ELEMENT *pelm = &(plogic->elem);


    bDy = (pLogicElem[GRAPH_DY].Output[BOOL_OUTPUT_BS]&pRunSet->bIDYBS)|(!pRunSet->bIDYBS);
    bPt = pLogicElem[GRAPH_PT].Output[BOOL_OUTPUT_PICK]&pRunSet->bPTBS;
    bDy = bPt|bDy;

    bDjqd =  pLogicElem[GRAPH_DJQD].Output[BOOL_OUTPUT_PICK]&
		    !pLogicElem[GRAPH_DJQD].Output[BOOL_OUTPUT_TRIP]&
		    !pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_BS];

    pelm->ppioIn[_PR_21_ENABLE].bVal     = ((pRunSet->dYb&PR_YB_I3)>>YB_I3)&bDjqd;
	pelm->ppioIn[_PR_21_A_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_A];
	pelm->ppioIn[_PR_21_B_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_B];
	pelm->ppioIn[_PR_21_C_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_C];
	pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI[2][0];
	pelm->ppioIn[_PR_21_IB_PICK].ulVal   = pRunSet->dI[2][1];
	pelm->ppioIn[_PR_21_IC_PICK].ulVal   = pRunSet->dI[2][2];
    if(pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP])
    {
		if(pRunSet->dT[2]> pRunSet->dTJS)
			pelm->ppioIn[_PR_21_T_PICK].ulVal= pRunSet->dTJS;
		if(MY_I3_JS_SET == 0)
		{
		   pelm->bSetChg = TRUE;
		   MY_I3_JS_SET = 1;
        }
    }
	else
	{
	    pelm->ppioIn[_PR_21_T_PICK].ulVal    = pRunSet->dT[2];
		if(MY_I3_JS_SET == 1)
		{
		   pelm->bSetChg = TRUE;
		   MY_I3_JS_SET = 0;
        }
	}
	pelm->ppioIn[_PR_21_DIR_ENABLE].bVal = ValFALSE;//pRunSet->bIDir;
	pelm->ppioIn[_PR_21_BLOCKED].bVal    = (!bDy)&pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_BS];
	pelm->ppioIn[_PR_21_IA_DROP].ulVal   = pRunSet->dIRet[2][0];
	pelm->ppioIn[_PR_21_IB_DROP].ulVal   = pRunSet->dIRet[2][1];
	pelm->ppioIn[_PR_21_IC_DROP].ulVal   = pRunSet->dIRet[2][2];
	pelm->ppioIn[_PR_21_T_DROP].ulVal    = pRunSet->dTIRet;
	pelm->ppioIn[_PR_21_INVMODE].bVal    = ValFALSE;
	pelm->ppioIn[_PR_21_I0MODE].bVal     = ValFALSE;
	pelm->ppioIn[_PR_21_CUV].ulVal       = Val0;
  
    plogic->elem.Scan_Func(pelm);
    plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_21_PICKUP].bVal;

    bGl = bGlYx = FALSE;
	for(i=0; i<3; i++)
	{
	   bDir = (!pRunSet->bIDir)|pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_A+i];
	   pRunInfo->fault.i_sec[i] &= ~0x04;
	   if(pelm->aioOut[_PR_21_A_OPT+i].bVal)
	   {
	      if (bDir|!pRunSet->bIYxDir)
		  {
		     pRunInfo->fault.i_phase |= PR_VYX_IA<<i;
			 pRunInfo->fault.i_sec[i] |= 0x04;
			 bGlYx = TRUE;
		  }
		  if(bDir)
		  	 bGl = TRUE;
		  phase = i;
		  pRunInfo->fault.i[i] = pVal->dI[i];
	   }
	}

	if (bGlYx && (plogic->Output[BOOL_OUTPUT_I]==0))
	{
	    plogic->Output[BOOL_OUTPUT_I] = phase+1;
	    pRunInfo->vyx |= PR_VYX_I3;
	    GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
		if(pRunSet->bRcdGlbh)
			prWavRecord(pRunInfo->fd, &(pRunInfo->fault.i_clock), REC_START |REC_STOP);
		
		if(MY_I3_JS_SET)
			pRunInfo->vyx |= PR_VYX_HJS;
	}
	else if (!bGlYx && plogic->Output[BOOL_OUTPUT_I])
		plogic->Output[BOOL_OUTPUT_I] = 0;


    if(bGl&&!plogic->Output[BOOL_OUTPUT_TRIP])
   	{
   	   plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
	   pRunInfo->fault.i[0] = pVal->dI[0];
	   pRunInfo->fault.i[1] = pVal->dI[1];
	   pRunInfo->fault.i[2] = pVal->dI[2];
	   pRunInfo->fault.i[3] = pVal->dI[3];

	  pRunInfo->fault.u[0] = pVal->dU[0];
	  pRunInfo->fault.u[1] = pVal->dU[1];
	  pRunInfo->fault.u[2] = pVal->dU[2];
	  pRunInfo->fault.u[3] = pVal->dU[3];

	  prWriteGzEvent(EVENT_I3);
    }
	if(!bGl&&plogic->Output[BOOL_OUTPUT_TRIP])
       plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
}

void IGfhLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	PRLIB_21(pelm);
}

void IGfhLogic(LogicElem *plogic)
{
    BOOL bDjqd;
    EP_ELEMENT *pelm = &(plogic->elem);

	bDjqd =  pLogicElem[GRAPH_DJQD].Output[BOOL_OUTPUT_PICK]&
		    !pLogicElem[GRAPH_DJQD].Output[BOOL_OUTPUT_TRIP]&
		    !pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_BS];

    pelm->ppioIn[_PR_21_ENABLE].bVal      = ((pRunSet->dYb&PR_YB_GFH)>>YB_GFH)&bDjqd;
	pelm->ppioIn[_PR_21_A_DIR].bVal       = ValFALSE;
	pelm->ppioIn[_PR_21_B_DIR].bVal       = ValFALSE;
	pelm->ppioIn[_PR_21_C_DIR].bVal       = ValFALSE;
	pelm->ppioIn[_PR_21_IA_PICK].ulVal    = pRunSet->dIGfh[0];
	pelm->ppioIn[_PR_21_IB_PICK].ulVal    = pRunSet->dIGfh[1];
	pelm->ppioIn[_PR_21_IC_PICK].ulVal    = pRunSet->dIGfh[2];
	pelm->ppioIn[_PR_21_T_PICK].ulVal     = pRunSet->dTGfh;
	pelm->ppioIn[_PR_21_DIR_ENABLE].ulVal = ValFALSE;
	pelm->ppioIn[_PR_21_BLOCKED].bVal     = ValFALSE;
	pelm->ppioIn[_PR_21_IA_DROP].ulVal    = pRunSet->dIGfhRet[0];
	pelm->ppioIn[_PR_21_IB_DROP].ulVal    = pRunSet->dIGfhRet[0];
	pelm->ppioIn[_PR_21_IC_DROP].ulVal    = pRunSet->dIGfhRet[0];
	pelm->ppioIn[_PR_21_T_DROP].ulVal     = Val0;
	pelm->ppioIn[_PR_21_INVMODE].bVal     = pRunSet->bGfhSX;
	pelm->ppioIn[_PR_21_I0MODE].bVal      = ValFALSE;
	pelm->ppioIn[_PR_21_CUV].ulVal        = pRunSet->bFsx;
  	
    plogic->elem.Scan_Func(pelm);
    plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_21_PICKUP].bVal;
    plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_21_OPERATE].bVal;

    if(plogic->Output[BOOL_OUTPUT_TRIP]&&!(pRunInfo->vyx&PR_VYX_GFH))
   	{
   	  pRunInfo->vyx |= PR_VYX_GFH;
	  pRunInfo->fault.i[0] = pVal->dI[0];
	  pRunInfo->fault.i[1] = pVal->dI[1];
	  pRunInfo->fault.i[2] = pVal->dI[2];
	  GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
    }
    else if(!plogic->Output[BOOL_OUTPUT_TRIP]&&(pRunInfo->vyx&PR_VYX_GFH))
   	{
   	  pRunInfo->vyx &= ~PR_VYX_GFH;
    }
}

#define MY_I01_JS_SET (plogic->aplUser[0])
#define MY_U01_JD     (plogic->aplUser[1])
#define MY_I01_JD      (plogic->aplUser[2])
#define MY_XDL_JD     (plogic->aplUser[3])
#define MY_I01_TBLB     (plogic->aplUser[4])

#define MY_JDFG_TIMER1 ((TIMERELAY*)(plogic->apvUser[0]))

void I01Logic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);
	
	MY_I01_JS_SET = 0;
	MY_U01_JD = 0;
	MY_I01_JD = 0;
	MY_XDL_JD = 0;
	MY_I01_TBLB = 0;
	plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	PRLIB_21(pelm);
}

void I01Logic(LogicElem *plogic)
{
	BOOL bU,bGl,bDir,bGlYx;
    EP_ELEMENT *pelm = &(plogic->elem);

	if(pRunSet->bSetChg)
	{
	   InitTR(MY_JDFG_TIMER1, pRunSet->dTRet, 0);
	}
		
	if(pRunSet->bI0U)
	{
	    if(pVal->dU[3] > pRunSet->dU0)
			bU = TRUE;
		else
			bU = FALSE;
	}
	else
		bU = TRUE;

	if ((pVal->dU[3]>pRunSet->dU0) && (MY_U01_JD == 0))
	{
#ifdef	INCLUDE_JD_PR			
		if( pRunSet->bXDLYB )
		{	
			StartWavJdRcd(pRunInfo->fd);
			pRunInfo->fault.i0_start = TRUE;
			pRunInfo->fault.i_cnt = Get100usCnt();
			prWavRecord(pRunInfo->fd, NULL, REC_START |REC_STOP);
		}
		else 
#endif
		{
			if(pRunSet->bRcdU0)
	    	  prWavRecord(pRunInfo->fd, NULL, REC_START |REC_STOP);
		}
//		WriteActEvent(NULL,  EVENT_I01, pRunInfo->fd, "零压启动 ");
		MY_U01_JD = 1;
		
	}
	else if ((pVal->dU[3] < pRunSet->dU0Ret)&&(MY_U01_JD == 1))
	{
	     MY_U01_JD = 0;
	}

	if ( (pVal->dI[3]>pRunSet->dI0[0]) && (MY_I01_JD == 0))
	{
		if(pRunSet->bRcdI0)
	  	  prWavRecord(pRunInfo->fd, NULL, REC_START |REC_STOP);
		MY_I01_JD = 1;
	}
	else if ((pVal->dI[3] < pRunSet->dI0Ret[0])&&(MY_I01_JD == 1))
	{
	     MY_I01_JD = 0;
	}
	
	if((pVal->dI[3] > pVal->dUI0Bak) && (MY_I01_TBLB == 0) && ((pVal->dI[3] - pVal->dUI0Bak) > pRunSet->dI0[0]))
	{
		if(pRunSet->bRcdI0TB)
		{
			prWavRecord(pRunInfo->fd, NULL, REC_START |REC_STOP);
			WriteActEvent(NULL,  EVENT_I01, pRunInfo->fd, "零流突变录波");
		}
		MY_I01_TBLB = 1;
	}
	else if(((pVal->dI[3] - pVal->dUI0Bak) < pRunSet->dI0[0]) && (MY_I01_TBLB == 1) && (pVal->dI[3] >= pVal->dUI0Bak))
		MY_I01_TBLB = 0;
	
	
    pelm->ppioIn[_PR_21_ENABLE].bVal     = ((pRunSet->dYb&PR_YB_I01)>>YB_I01)&&bU;
	pelm->ppioIn[_PR_21_A_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_N];
	pelm->ppioIn[_PR_21_B_DIR].bVal      = ValFALSE;
	pelm->ppioIn[_PR_21_C_DIR].bVal      = ValFALSE;
	if(pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP])
    {
        pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI0[3];
#if (TYPE_USER  == USER_FUJIAN)	
		if(pRunSet->dT0[0]> pRunSet->dI0TJS)
			pelm->ppioIn[_PR_21_T_PICK].ulVal= pRunSet->dI0TJS;
#else
		if(pRunSet->dT0[0]> pRunSet->dTJS)
			pelm->ppioIn[_PR_21_T_PICK].ulVal= pRunSet->dTJS;
#endif

		if(MY_I01_JS_SET==0)
		{
		   pelm->bSetChg = TRUE;
		   MY_I01_JS_SET = 1;
        }
    }
	else
	{
	    pelm->ppioIn[_PR_21_T_PICK].ulVal    = pRunSet->dT0[0];
		pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI0[0];
		if(MY_I01_JS_SET==1)
		{
		   pelm->bSetChg = TRUE;
		   MY_I01_JS_SET = 0;
        }

	}
	
	pelm->ppioIn[_PR_21_IB_PICK].ulVal   = Val0;
	pelm->ppioIn[_PR_21_IC_PICK].ulVal   = Val0;
	pelm->ppioIn[_PR_21_DIR_ENABLE].bVal = ValFALSE;//pRunSet->bI0Dir;
	pelm->ppioIn[_PR_21_BLOCKED].bVal    = ValFALSE;
	pelm->ppioIn[_PR_21_IA_DROP].ulVal = pRunSet->dI0Ret[0];
	pelm->ppioIn[_PR_21_IB_DROP].ulVal   = Val0;
	pelm->ppioIn[_PR_21_IC_DROP].ulVal   = Val0;
	if(!pRunSet->bI0Trip)
	    pelm->ppioIn[_PR_21_T_DROP].ulVal   = pRunSet->dTI0Ret;
	else
	    pelm->ppioIn[_PR_21_T_DROP].ulVal   = pRunSet->dTIRet;
	pelm->ppioIn[_PR_21_INVMODE].bVal    = ValFALSE;
	pelm->ppioIn[_PR_21_I0MODE].bVal     = ValTRUE;
	pelm->ppioIn[_PR_21_CUV].ulVal       = Val0;
  
    plogic->elem.Scan_Func(pelm);
    plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_21_PICKUP].bVal;

    bGl = bGlYx = FALSE;
	bDir = (!pRunSet->bI0Dir)|pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_N];
	if(pelm->aioOut[_PR_21_OPERATE].bVal)
	{
	    if (bDir|!pRunSet->bIYxDir)
		{
		   pRunInfo->fault.i_phase |= PR_VYX_I0;

		   bGlYx = TRUE;
		}
		if(bDir)
		   bGl = TRUE;
		pRunInfo->fault.i_sec[3] |= 0x01;
	 }
	 else
	    pRunInfo->fault.i_sec[3] &= ~0x01;
#ifdef _TEST_VER_
	if(pRunSet->bXDLYB && ((pRunSet->dYb&PR_YB_I01)>>YB_I01))
	{
		if(plogic->Output[BOOL_OUTPUT_PICK] && pRunSet->bI0Dir && pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_N])
			pRunInfo->fault.i0_xdlgj = 1;
	}
#endif
	if (pRunInfo->fault.i0_xdlgj)
	{		
		if( MY_XDL_JD == 0)
		{	
			//告警延时
			
			
			if(pRunInfo->vyx&PR_VYX_I0C) //还未复归，重置遥信
			{
				pRunInfo->vyx &= ~PR_VYX_I0C;
		  		 prVyxHandle();
			}
			  GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
	  		  pRunInfo->vyx |= PR_VYX_I0C;
			 MY_XDL_JD = 1;
 
			pRunInfo->fault.i[0] = pVal->dI[0];
			pRunInfo->fault.i[1] = pVal->dI[1];
			pRunInfo->fault.i[2] = pVal->dI[2];
			pRunInfo->fault.i[3] = pVal->dI[3];

			pRunInfo->fault.u[0] = pVal->dU[0];
			pRunInfo->fault.u[1] = pVal->dU[1];
			pRunInfo->fault.u[2] = pVal->dU[2];
			pRunInfo->fault.u[3] = pVal->dU[3];

			prWriteGzEvent(EVENT_I0C);
			prLightHandle();
		}
	}
	else
	{
		MY_XDL_JD = 0;
	}

	RunTR(MY_JDFG_TIMER1,(pVal->dU[3] < pRunSet->dU0)&&(pRunInfo->vyx&PR_VYX_I0C)); //复归
	if(MY_JDFG_TIMER1->boolTrip)
	{
		pRunInfo->vyx &= ~ PR_VYX_I0C;
	}


	if (bGlYx && (plogic->Output[BOOL_OUTPUT_I]==0))
	{
	    plogic->Output[BOOL_OUTPUT_I] = 4;
	     GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
			 
		 if(MY_I01_JS_SET)
			 pRunInfo->vyx |= PR_VYX_HJS;
	}
	else if (!bGlYx && plogic->Output[BOOL_OUTPUT_I])
		plogic->Output[BOOL_OUTPUT_I] = 0;
    if(bGl&&!plogic->Output[BOOL_OUTPUT_TRIP])
    {
      plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
	   pRunInfo->fault.i[0] = pVal->dI[0];
	   pRunInfo->fault.i[1] = pVal->dI[1];
	   pRunInfo->fault.i[2] = pVal->dI[2];
	   pRunInfo->fault.i[3] = pVal->dI[3];

	  pRunInfo->fault.u[0] = pVal->dU[0];
	  pRunInfo->fault.u[1] = pVal->dU[1];
	  pRunInfo->fault.u[2] = pVal->dU[2];
	  pRunInfo->fault.u[3] = pVal->dU[3];

	  prWriteGzEvent(EVENT_I01);
    }
	if(!bGl&&plogic->Output[BOOL_OUTPUT_TRIP])
    {
       plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	}
}

#define MY_I02_JS_SET (plogic->aplUser[0])
void I02Logic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	MY_I02_JS_SET = 0;

	PRLIB_21(pelm);
}

void I02Logic(LogicElem *plogic)
{
	BOOL bU,bGl,bGlYx,bDir;
    EP_ELEMENT *pelm = &(plogic->elem);

	if(pRunSet->bI0U)
	{
	    if(pVal->dU[3] > pRunSet->dU0)
			bU = TRUE;
		else
			bU = FALSE;
	}
	else
		bU = TRUE;

    pelm->ppioIn[_PR_21_ENABLE].bVal     = ((pRunSet->dYb&PR_YB_I02)>>YB_I02)&&bU;
	pelm->ppioIn[_PR_21_A_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_N];
	pelm->ppioIn[_PR_21_B_DIR].bVal      = ValFALSE;
	pelm->ppioIn[_PR_21_C_DIR].bVal      = ValFALSE;
	if(pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP])
    {
        pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI0[3];
#if (TYPE_USER  == USER_FUJIAN)	
		if(pRunSet->dT0[1]> pRunSet->dI0TJS)
			pelm->ppioIn[_PR_21_T_PICK].ulVal= pRunSet->dI0TJS;
#else
		if(pRunSet->dT0[1]> pRunSet->dTJS)
			pelm->ppioIn[_PR_21_T_PICK].ulVal= pRunSet->dTJS;
#endif
		if(MY_I02_JS_SET==0)
		{
		   pelm->bSetChg = TRUE;
		   MY_I02_JS_SET = 1;
        }
    }
	else
	{
	    pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI0[1];
	    pelm->ppioIn[_PR_21_T_PICK].ulVal    = pRunSet->dT0[1];
		if(MY_I02_JS_SET==1)
		{
		   pelm->bSetChg = TRUE;
		   MY_I02_JS_SET = 0;
        }

	}
	pelm->ppioIn[_PR_21_IB_PICK].ulVal   = Val0;
	pelm->ppioIn[_PR_21_IC_PICK].ulVal   = Val0;
	pelm->ppioIn[_PR_21_DIR_ENABLE].bVal = ValFALSE;//pRunSet->bI0Dir;
	pelm->ppioIn[_PR_21_BLOCKED].bVal    = ValFALSE;
	pelm->ppioIn[_PR_21_IA_DROP].ulVal   = pRunSet->dI0Ret[1];
	pelm->ppioIn[_PR_21_IB_DROP].ulVal   = Val0;
	pelm->ppioIn[_PR_21_IC_DROP].ulVal   = Val0;
	if(!pRunSet->bI0Trip)
	    pelm->ppioIn[_PR_21_T_DROP].ulVal   = pRunSet->dTI0Ret;
	else
	    pelm->ppioIn[_PR_21_T_DROP].ulVal    = pRunSet->dTIRet;
	pelm->ppioIn[_PR_21_INVMODE].bVal    = ValFALSE;
	pelm->ppioIn[_PR_21_I0MODE].bVal     = ValTRUE;
	pelm->ppioIn[_PR_21_CUV].ulVal       = Val0;
  
    plogic->elem.Scan_Func(pelm);
    plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_21_PICKUP].bVal;

    bGl = bGlYx = FALSE;
	bDir = (!pRunSet->bI0Dir)|pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_N];
	if(pelm->aioOut[_PR_21_OPERATE].bVal)
	{
	    if (bDir|!pRunSet->bIYxDir)
		{
#if (TYPE_USER == USER_GUANGXI)
           pRunInfo->fault.i_phase |= PR_VYX_I02;
#elif (TYPE_USER == USER_BEIJING)
		   pRunInfo->fault.i_phase |= PR_VYX_I0;
#else
		//   pRunInfo->fault.i_phase |= PR_VYX_I0;
#endif
		   bGlYx = TRUE;
		}
		if(bDir)
		   bGl = TRUE;
		pRunInfo->fault.i_sec[3] |= 0x02;
	 }
	 else
	    pRunInfo->fault.i_sec[3] &= ~0x02;

	if (bGlYx && (plogic->Output[BOOL_OUTPUT_I]==0))
	{
	    plogic->Output[BOOL_OUTPUT_I] = 4;
	    GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);

		if(MY_I02_JS_SET)
			pRunInfo->vyx |= PR_VYX_HJS;
	}
	else if (!bGlYx && plogic->Output[BOOL_OUTPUT_I])
		plogic->Output[BOOL_OUTPUT_I] = 0;
    if(bGl&&!plogic->Output[BOOL_OUTPUT_TRIP])
    {
      plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
	   pRunInfo->fault.i[0] = pVal->dI[0];
	   pRunInfo->fault.i[1] = pVal->dI[1];
	   pRunInfo->fault.i[2] = pVal->dI[2];
	   pRunInfo->fault.i[3] = pVal->dI[3];

	  pRunInfo->fault.u[0] = pVal->dU[0];
	  pRunInfo->fault.u[1] = pVal->dU[1];
	  pRunInfo->fault.u[2] = pVal->dU[2];
	  pRunInfo->fault.u[3] = pVal->dU[3];

	  prWriteGzEvent(EVENT_I02);
    }
	if(!bGl&&plogic->Output[BOOL_OUTPUT_TRIP])
    {
       plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	}
}

#define MY_I03_JS_SET (plogic->aplUser[0])
void I03Logic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	MY_I03_JS_SET = 0;

	PRLIB_21(pelm);
}

void I03Logic(LogicElem *plogic)
{
	BOOL bU,bDir,bGl,bGlYx;

    EP_ELEMENT *pelm = &(plogic->elem);

	if(pRunSet->bI0U)
	{
	    if(pVal->dU[3] > pRunSet->dU0)
			bU = TRUE;
		else
			bU = FALSE;
	}
	else
		bU = TRUE;

    pelm->ppioIn[_PR_21_ENABLE].bVal     = ((pRunSet->dYb&PR_YB_I03)>>YB_I03)&&bU;
	pelm->ppioIn[_PR_21_A_DIR].bVal      = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_N];
	pelm->ppioIn[_PR_21_B_DIR].bVal      = ValFALSE;
	pelm->ppioIn[_PR_21_C_DIR].bVal      = ValFALSE;
	if(pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP])
    {
#if (TYPE_USER  == USER_FUJIAN)	
		if(pRunSet->dT0[2]> pRunSet->dI0TJS)
			pelm->ppioIn[_PR_21_T_PICK].ulVal= pRunSet->dI0TJS;
#else
		if(pRunSet->dT0[2]> pRunSet->dTJS)
			pelm->ppioIn[_PR_21_T_PICK].ulVal= pRunSet->dTJS;
#endif
		if(MY_I03_JS_SET==0)
		{
		   pelm->bSetChg = TRUE;
		   MY_I03_JS_SET = 1;
        }
    }
	else
	{
	    pelm->ppioIn[_PR_21_T_PICK].ulVal    = pRunSet->dT0[2];
		if(MY_I03_JS_SET==1)
		{
		   pelm->bSetChg = TRUE;
		   MY_I03_JS_SET = 0;
        }

	}
	pelm->ppioIn[_PR_21_IA_PICK].ulVal   = pRunSet->dI0[2];
	pelm->ppioIn[_PR_21_IB_PICK].ulVal   = Val0;
	pelm->ppioIn[_PR_21_IC_PICK].ulVal   = Val0;
	pelm->ppioIn[_PR_21_DIR_ENABLE].bVal = pRunSet->bI0Dir;
	pelm->ppioIn[_PR_21_BLOCKED].bVal    = ValFALSE;
	pelm->ppioIn[_PR_21_IA_DROP].ulVal   = pRunSet->dI0Ret[2];
	pelm->ppioIn[_PR_21_IB_DROP].ulVal   = Val0;
	pelm->ppioIn[_PR_21_IC_DROP].ulVal   = Val0;
	if(!pRunSet->bI0Trip)
	    pelm->ppioIn[_PR_21_T_DROP].ulVal   = pRunSet->dTI0Ret;
	else
	    pelm->ppioIn[_PR_21_T_DROP].ulVal    = pRunSet->dTIRet;
	pelm->ppioIn[_PR_21_INVMODE].bVal    = ValFALSE;
	pelm->ppioIn[_PR_21_I0MODE].bVal     = ValTRUE;
	pelm->ppioIn[_PR_21_CUV].ulVal       = Val0;
  
    plogic->elem.Scan_Func(pelm);
    plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_21_PICKUP].bVal;

	bGl = bGlYx = FALSE;
	bDir = (!pRunSet->bI0Dir)|pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_N];
	if(pelm->aioOut[_PR_21_OPERATE].bVal)
	{
	    if (bDir|!pRunSet->bIYxDir)
		{
		   pRunInfo->fault.i_phase |= PR_VYX_I0;

		   bGlYx = TRUE;
		}
		if(bDir)
		   bGl = TRUE;
		pRunInfo->fault.i_sec[3] |= 0x04;
	 }
	 else
	    pRunInfo->fault.i_sec[3] &= ~0x04;

	if (bGlYx && (plogic->Output[BOOL_OUTPUT_I]==0))
	{
	    plogic->Output[BOOL_OUTPUT_I] = 4;
	    GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
		if(MY_I03_JS_SET)
			pRunInfo->vyx |= PR_VYX_HJS;
	}
	else if (!bGlYx && plogic->Output[BOOL_OUTPUT_I])
		plogic->Output[BOOL_OUTPUT_I] = 0;
    if(bGl&&!plogic->Output[BOOL_OUTPUT_TRIP])
    {
      plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
	   pRunInfo->fault.i[0] = pVal->dI[0];
	   pRunInfo->fault.i[1] = pVal->dI[1];
	   pRunInfo->fault.i[2] = pVal->dI[2];
	   pRunInfo->fault.i[3] = pVal->dI[3];

	  pRunInfo->fault.u[0] = pVal->dU[0];
	  pRunInfo->fault.u[1] = pVal->dU[1];
	  pRunInfo->fault.u[2] = pVal->dU[2];
	  pRunInfo->fault.u[3] = pVal->dU[3];

	  prWriteGzEvent(EVENT_I03);
    }
	if(!bGl&&plogic->Output[BOOL_OUTPUT_TRIP])
    {
       plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	}
}

#define MY_XLYY_TIMER1 ((TIMERELAY*)(plogic->apvUser[0]))
void UGyLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));

	PRLIB_25(pelm);
}

void UGyLogic(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	if(pRunSet->bSetChg)
    {
#if(TYPE_USER == USER_SHANDONG)
        InitTR(MY_XLYY_TIMER1, 10000, 0);
#else
	InitTR(MY_XLYY_TIMER1, 5000, 0);
#endif
    }
	
    pelm->ppioIn[_PR_25_ENABLE].bVal  = (pRunSet->dYb&PR_YB_GY)>>YB_GY;
	pelm->ppioIn[_PR_25_UAB].ulVal    = pVal->dUxx[0];
	pelm->ppioIn[_PR_25_UBC].ulVal    = pVal->dUxx[1];
	pelm->ppioIn[_PR_25_UCA].ulVal    = pVal->dUxx[2];
	pelm->ppioIn[_PR_25_BLOCKED].bVal = pLogicElem[GRAPH_PT].Output[BOOL_OUTPUT_PICK]&pRunSet->bPTBS;

	pelm->Scan_Func(pelm);
	plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_25_PICKUP].bVal;
	plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_25_OPERATE].bVal;

	if(plogic->Output[BOOL_OUTPUT_TRIP]&&!(pRunInfo->vyx&PR_VYX_GY))
   	{
   	  pRunInfo->vyx |= PR_VYX_GY;
	  pRunInfo->fault.u[0] = pVal->dUxx[0];
	  pRunInfo->fault.u[1] = pVal->dUxx[1];
	  pRunInfo->fault.u[2] = pVal->dUxx[2];
	  GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
	}
    else if(!plogic->Output[BOOL_OUTPUT_TRIP]&&(pRunInfo->vyx&PR_VYX_GY))
   	{
   	  pRunInfo->vyx &= ~PR_VYX_GY;
	 // pRunInfo->vyx &= ~PR_VYX_TRIP;
    }

	if (pRunSet->bTestVol)
	{
	    RunTR(MY_XLYY_TIMER1, pelm->aioOut[_PR_25_YY].bVal);
	    if (MY_XLYY_TIMER1->boolTrip)
	       pRunInfo->vyx |= PR_VYX_XL_U;
	    else
		   pRunInfo->vyx &= ~PR_VYX_XL_U;
    }
	else
	{
		pRunInfo->vyx &= ~PR_VYX_XL_U;
	}

}

#define MY_XLWY_TIMER1 ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_XLWY_TIMER2 ( (TMAXTIMERELAY*)(plogic->apvUser[1]))
void UDyLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	plogic->apvUser[1] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));

	PRLIB_22(pelm);
}

void UDyLogic(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

    if(pRunSet->bSetChg)
    {
#if(TYPE_USER == USER_SHANDONG)
		InitTR(MY_XLWY_TIMER1, 10000, 0);
		InitTR(&MY_XLWY_TIMER2->tMaxTimeRelay[0], 0, 0);
		InitTR(&MY_XLWY_TIMER2->tMaxTimeRelay[1], 10000, 1);
#else
		InitTR(MY_XLWY_TIMER1, 5000, 0);
		InitTR(&MY_XLWY_TIMER2->tMaxTimeRelay[0], 0, 0);
		InitTR(&MY_XLWY_TIMER2->tMaxTimeRelay[1], 5000, 1);
#endif
		
    }

  	pelm->ppioIn[_PR_22_ENABLE].bVal  = (pRunSet->dYb&PR_YB_DY)>>YB_DY;
	pelm->ppioIn[_PR_22_UAB].ulVal    = pVal->dUxx[0];
	pelm->ppioIn[_PR_22_UBC].ulVal    = pVal->dUxx[1];
	pelm->ppioIn[_PR_22_UCA].ulVal    = pVal->dUxx[2];
	pelm->ppioIn[_PR_22_TWJ].bVal     = !(pVal->dYx&PR_KG_YX_STATUS);
	pelm->ppioIn[_PR_22_BLOCKED].bVal = pLogicElem[GRAPH_PT].Output[BOOL_OUTPUT_PICK]&pRunSet->bPTBS;

	pelm->Scan_Func(pelm);
	plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_22_PICKUP].bVal;
	plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_22_OPERATE].bVal;
	plogic->Output[BOOL_OUTPUT_BS]   = pelm->aioOut[_PR_22_DYBS].bVal;

	if(plogic->Output[BOOL_OUTPUT_TRIP]&&!(pRunInfo->vyx&PR_VYX_DY))
   	{
   	   pRunInfo->vyx |= PR_VYX_DY;
	   pRunInfo->fault.u[0] = pVal->dUxx[0];
	   pRunInfo->fault.u[1] = pVal->dUxx[1];
	   pRunInfo->fault.u[2] = pVal->dUxx[2];
	   GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
	}
    else if(!plogic->Output[BOOL_OUTPUT_TRIP]&&(pRunInfo->vyx&PR_VYX_DY))
   	{
   	   pRunInfo->vyx &= ~PR_VYX_DY;
	   pRunInfo->vyx &= ~PR_VYX_TRIP;
    }

    if (pRunSet->bTestVol)
	{
	    RunTR(MY_XLWY_TIMER1, pelm->aioOut[_PR_22_WY].bVal);
	    if (MY_XLWY_TIMER1->boolTrip)
	       pRunInfo->vyx |= PR_VYX_XL_NU;
	    else
		   pRunInfo->vyx &= ~PR_VYX_XL_NU;
    }
	else
	{
		pRunInfo->vyx &= ~PR_VYX_XL_NU;
	}
	if(pRunSet->bRcdSy)
	{
		 RunTR(&MY_XLWY_TIMER2->tMaxTimeRelay[0], pelm->aioOut[_PR_22_WY].bVal); //无压
		 
	    if ((MY_XLWY_TIMER2->tMaxTimeRelay[0].boolTrip) && (MY_XLWY_TIMER2->tMaxTimeRelay[1].boolTrip))
	    {
				prWavRecord(pRunInfo->fd, NULL, REC_START |REC_STOP);
	    }
		 RunTR(&MY_XLWY_TIMER2->tMaxTimeRelay[1], !pelm->aioOut[_PR_22_WY].bVal); //非无压
	}
	
}
#define MY_MXQY_TIMER1 ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_MXQY_TIMER2 ((TIMERELAY*)(plogic->apvUser[1]))

void UMXQYLogic_Init(LogicElem *plogic)
{
	plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	plogic->apvUser[1] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
}

void UMXQYLogic(LogicElem *plogic)
{
#ifdef PR_ZJJH
	BOOL bUTrip,bDTrip;
	int i,j;
	i = j = 0;
	if(!(pRunSet->dYb&PR_YB_MXQY))
		return;

	if(pRunSet->bSetChg)
    {
		InitTR(MY_MXQY_TIMER1,pRunSet->dTUMX, 36000000);
		InitTR(MY_MXQY_TIMER2,pRunSet->dTDMX, 0);
    }
	bUTrip = FALSE;
	for(i=0; i<3; i++)
	{
	   if(pVal->dUxx[i] > pRunSet->dUUMX[i])
	   {
	       bUTrip = TRUE;
		   break;
	   }
	}
	RunTR(MY_MXQY_TIMER1, bUTrip);

	plogic->Output[BOOL_OUTPUT_PICK] = MY_MXQY_TIMER1->boolTrip;
	bDTrip = FALSE;
	for(i=0; i<3; i++)
	{
	   if(pVal->dUxx[i] < pRunSet->dUDMX[i])
	   {
	   	   j++;
		   if(j==3)
		   {
			   bDTrip = TRUE;
			   break;
		   }
	   }
	   else
	   	break;
	}
	RunTR(MY_MXQY_TIMER2, bDTrip);

	plogic->Output[BOOL_OUTPUT_TRIP] = (MY_MXQY_TIMER1->boolTrip)&&(MY_MXQY_TIMER2->boolTrip);

	if(plogic->Output[BOOL_OUTPUT_TRIP]&&!(pRunInfo->vyx&PR_VYX_MXQY))
   	{
   	  pRunInfo->vyx |= PR_VYX_MXQY;
	  pRunInfo->fault.u[0] = pVal->dUxx[0];
	  pRunInfo->fault.u[1] = pVal->dUxx[1];
	  pRunInfo->fault.u[2] = pVal->dUxx[2];
	  GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
	  prWriteEvent(EVENT_MXQY);
	}
    else if(!plogic->Output[BOOL_OUTPUT_TRIP]&&(pRunInfo->vyx&PR_VYX_MXQY))
   	{
   	  pRunInfo->vyx &= ~PR_VYX_MXQY;
    }
#endif
}

void PTLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	PRLIB_24(pelm);
}

void PTLogic(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	pelm->ppioIn[_PR_24_ENABLE].bVal = (pRunSet->dYb&PR_YB_PT)>>YB_PT;
	pelm->ppioIn[_PR_24_VVMODE].bVal = !(pFdCfg[pRunInfo->fd].cfg & FD_CFG_PT_58V);

	pelm->Scan_Func(pelm);
	plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_24_PICKUP].bVal;
	plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_24_OPERATE].bVal;

	if(plogic->Output[BOOL_OUTPUT_TRIP]&&!(pRunInfo->vyx&PR_VYX_PT))
   	{
   	  pRunInfo->vyx |= PR_VYX_PT;
	  GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
	}
    else if(!plogic->Output[BOOL_OUTPUT_TRIP]&&(pRunInfo->vyx&PR_VYX_PT))
   	  pRunInfo->vyx &= ~PR_VYX_PT;
}

void IDirLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	PRLIB_23(pelm);
}

void IDirLogic(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);
#if (TYPE_USER  == USER_FUJIAN)
    pelm->ppioIn[_PR_23_I_ENABLE].bVal = pRunSet->bIDir|pRunSet->bI2Dir;
#else
	pelm->ppioIn[_PR_23_I_ENABLE].bVal = pRunSet->bIDir;
#endif
	pelm->ppioIn[_PR_23_I0_ENABLE].bVal = ValTRUE;
    if(pCfg->pdU0 != NULL)
	   pelm->ppioIn[_PR_23_3U0].bVal    = ValFALSE;
    else
	   pelm->ppioIn[_PR_23_3U0].bVal    = ValTRUE;

	pelm->ppioIn[_PR_23_VVMODE].bVal = !(pFdCfg[pRunInfo->fd].cfg & FD_CFG_PT_58V);
	pelm->ppioIn[_PR_23_PANG1].lVal = pRunSet->lPang1;
	pelm->ppioIn[_PR_23_PANG2].lVal = pRunSet->lPang2;
	pelm->ppioIn[_PR_23_NANG1].lVal = pRunSet->lNang1;
	pelm->ppioIn[_PR_23_NANG2].lVal = pRunSet->lNang2;

	pelm->Scan_Func(pelm);
	
	plogic->Output[BOOL_OUTPUT_A] = pelm->aioOut[_PR_23_A_DIR].bVal;
	plogic->Output[BOOL_OUTPUT_B] = pelm->aioOut[_PR_23_B_DIR].bVal;
	plogic->Output[BOOL_OUTPUT_C] = pelm->aioOut[_PR_23_C_DIR].bVal;
	plogic->Output[BOOL_OUTPUT_N] = pelm->aioOut[_PR_23_N_DIR].bVal && pRunSet->bI0Dir;
	plogic->Output[BOOL_OUTPUT_N+1] = pelm->aioOut[_PR_23_N_DIR].bVal;
}

#define MY_WL_TIMER1  ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_WL_TIMER2  ((TMAXTIMERELAY*)(plogic->apvUser[1]))
#define MY_WL_CNT     (plogic->aplUser[0])
void WLLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	plogic->apvUser[1] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
	MY_WL_CNT = 0;

	PRLIB_27(pelm);
}

void WLLogic(LogicElem *plogic)
{
    EP_ELEMENT *pelm=&(plogic->elem);

	BOOL bGl=FALSE;
	BOOL bU=FALSE;
	BOOL bNoGl = TRUE; // 电流小于故障电流
	//DWORD type;

	VRunParaCfg *prunparacfg =  &g_Sys.MyCfg.RunParaCfg;
	BOOL bZZ,bGZ;
	BYTE i;
	DWORD dDI;

	if (pRunSet->bSetChg)
    {
        InitTR(MY_WL_TIMER1, pRunSet->dTJSIN, 0);
		InitTR(&MY_WL_TIMER2->tMaxTimeRelay[0], 5000, 0);	
		InitTR(&MY_WL_TIMER2->tMaxTimeRelay[1], prunparacfg->wZzT*10, 1);
		InitTR(&MY_WL_TIMER2->tMaxTimeRelay[2], prunparacfg->wGzT*10, 1);
    }

	bGl = pRunInfo->fault.i_sgz_yx;

	RunTR(MY_WL_TIMER1, !bGl);

	if (MY_WL_TIMER1->boolTrip)
	   MY_WL_CNT = 0;

	if((pVal->dI[0] > pRunSet->dI[3][0]) || (pVal->dI[1]  > pRunSet->dI[3][1]) || (pVal->dI[2]  > pRunSet->dI[3][2])) // 有故障电流
		bNoGl = FALSE;
	//if ((pVal->dU[0] > pRunSet->dU75[0])||(pVal->dU[1] > pRunSet->dU75[1])||(pVal->dU[2] > pRunSet->dU75[2]))
	//    bU = TRUE;
	bU = TRUE;
	for(i=0;i<3;i++)
	{
		  if ((pVal->dUxx[i] < pRunSet->dUxx75[i])&pRunSet->bPTU[i])
	        bU = FALSE;
	}
	
	//type = prLogicFaStatus(pRunInfo->fd);
	RunTR(&MY_WL_TIMER2->tMaxTimeRelay[0], bNoGl &((!pLogicElem[GRAPH_WL].Output[BOOL_OUTPUT_PICK])|bU));
	if (MY_WL_TIMER2->tMaxTimeRelay[0].boolTrip)
	    pRunInfo->normal_sts = TRUE;
	else
		pRunInfo->normal_sts = FALSE;
	
	
	pelm->ppioIn[_PR_27_ENABLE].bVal  = ValTRUE;
	pelm->ppioIn[_PR_27_CTRL_YL].bVal = ValFALSE;
	pelm->ppioIn[_PR_27_T_WL].ulVal   = 300;
	pelm->ppioIn[_PR_27_T_YL].ulVal   = Val0;

	pelm->Scan_Func(pelm);

	plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_27_PICKUP].bVal;
	if(bGl)
        plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_27_OPERATE].bVal;
	else
		plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	if(plogic->Output[BOOL_OUTPUT_TRIP] && !plogic->Output[BOOL_OUTPUT_NI1])
	{
	   plogic->Output[BOOL_OUTPUT_NI1] = TRUE;
	   if(pRunSet->bIReport == PR_OC_NI2_REPORT)
	   {
	      MY_WL_CNT++;
	      if(MY_WL_CNT == 2)
	      {
	         MY_WL_CNT = 0;
			 plogic->Output[BOOL_OUTPUT_NI2] = TRUE;
	      }
		  else
		  	 pRunInfo->fault.i_sgz_yx = FALSE;
	   }
	}
	else if(!plogic->Output[BOOL_OUTPUT_TRIP] && plogic->Output[BOOL_OUTPUT_NI1])
	{
	    plogic->Output[BOOL_OUTPUT_NI1] = FALSE;
		plogic->Output[BOOL_OUTPUT_NI2] = FALSE;
	}


	//每回路的重载过载

	if(MY_WL_TIMER2->tMaxTimeRelay[1].dTripThreshold != prunparacfg->wZzT*10)
	{
		InitTR(&MY_WL_TIMER2->tMaxTimeRelay[1],prunparacfg->wZzT*10,0);
		ResetTR(&MY_WL_TIMER2->tMaxTimeRelay[1]);
	}
	if(MY_WL_TIMER2->tMaxTimeRelay[2].dTripThreshold != prunparacfg->wGzT*10)
	{
		InitTR(&MY_WL_TIMER2->tMaxTimeRelay[2],prunparacfg->wGzT*10,0);
		ResetTR(&MY_WL_TIMER2->tMaxTimeRelay[2]);
	}

	bZZ = bGZ = FALSE;
	for(i = 0; i < 3;i++)
	{
		dDI = (float)(prValue_l2f(i+5,pCfg,pVal->dI[i])) *1000;
		if(dDI > prunparacfg->dwZzVal)
			bZZ = TRUE;
		if(dDI > prunparacfg->dwGzVal)
			bGZ = TRUE;
	}
	RunTR(&MY_WL_TIMER2->tMaxTimeRelay[1], bZZ);
	RunTR(&MY_WL_TIMER2->tMaxTimeRelay[2], bGZ);
	
	//重载
	if(MY_WL_TIMER2->tMaxTimeRelay[1].boolTrip &&!(pRunInfo->vyx&PR_VYX_ZZ))
	{
		GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
		 pRunInfo->vyx |= PR_VYX_ZZ;
	}
	else if((!MY_WL_TIMER2->tMaxTimeRelay[1].boolTrip) && (pRunInfo->vyx&PR_VYX_ZZ))
	{
		pRunInfo->vyx &= ~PR_VYX_ZZ;
	}
	//过载
	if(MY_WL_TIMER2->tMaxTimeRelay[2].boolTrip &&!(pRunInfo->vyx&PR_VYX_GZ))
	{
		GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
		 pRunInfo->vyx |= PR_VYX_GZ;
	}
	else if((!MY_WL_TIMER2->tMaxTimeRelay[2].boolTrip) && (pRunInfo->vyx&PR_VYX_GZ))
	{
		pRunInfo->vyx &= ~PR_VYX_GZ;
	}	
	
}

void DjqdLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	PRLIB_28(pelm);
}

void DjqdLogic(LogicElem *plogic)
{
    EP_ELEMENT *pelm=&(plogic->elem);

	pelm->ppioIn[_PR_28_ENABLE].bVal  = (pRunSet->dYb&PR_YB_DJQD)>>YB_DJQD;

	pelm->Scan_Func(pelm);

    plogic->Output[BOOL_OUTPUT_PICK] = pelm->aioOut[_PR_28_DLQD].bVal;
	plogic->Output[BOOL_OUTPUT_TRIP] = pelm->aioOut[_PR_28_DJQD].bVal;

}

#define MY_CH_TIMER1 ((TMAXTIMERELAY*)(plogic->apvUser[0]))
#define MY_CH_QD     (plogic->aplUser[0])
#define MY_CH_DZ     (plogic->aplUser[1])

#define MY_CH_SD      (plogic->aplUser[2])   /*首端FTU标志，重合时间取首FTU选线重合闸时间*/
void CHLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	plogic->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
	MY_CH_QD = 0;
	MY_CH_DZ = 0;
	MY_CH_SD = 0;
    PRLIB_26(pelm);
}

void CHLogic(LogicElem *plogic)
{
    BOOL YxHwj,YxTwj,HandTrip=FALSE;
	BOOL bFd,bQd1,bAT,bQd3,bDz,bRcNum=FALSE;
    EP_ELEMENT *pelm = &(plogic->elem);
	  BOOL bDZ = FALSE;
#if (TYPE_USER == USER_BEIJING)	
	BOOL bYY = FALSE; // 单侧有压
#endif

    if(pRunSet->bSetChg)
	{
		InitTR(&(MY_CH_TIMER1->tMaxTimeRelay[0]), 1000, 0);
		InitTR(&(MY_CH_TIMER1->tMaxTimeRelay[1]), 0, 15000);
		InitTR(&(MY_CH_TIMER1->tMaxTimeRelay[2]), 2000,0);
		InitTR(&(MY_CH_TIMER1->tMaxTimeRelay[3]), pRunSet->dTRet,0);
		InitTR(&(MY_CH_TIMER1->tMaxTimeRelay[4]), 151000,0);
    }

    YxHwj = (pVal->dYx&PR_KG_YX_STATUS);
	YxTwj = !YxHwj;

	bFd  = pLogicElem[GRAPH_RELAY].Output[BOOL_TRIP_FAIL]|plogic->Output[BOOL_CH_FAIL];
	bFd |= pLogicElem[GRAPH_RELAY].Output[BOOL_TRIP_EVER]|plogic->Output[BOOL_CH_FD];
	bFd |= pLogicElem[GRAPH_RELAY].Output[BOOL_RC_FAIL]|pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_CHBS];
	bFd |= pRunInfo->kg_ch_ddlbs;

#if (TYPE_USER == USER_BEIJING)	
	if((pVal->dU[0] > pRunSet->dUYy[0]) &&  ((pVal->dU[2] < pRunSet->dUYy[2])))
		bYY = TRUE;
	if((pVal->dU[0] < pRunSet->dUYy[0]) &&  ((pVal->dU[2] > pRunSet->dUYy[2])))
		bYY = TRUE;
#endif
		
	if(pRunSet->bITrip >= PR_AT_AUTO_TRIP)
		bAT = TRUE;
	else
		bAT = FALSE;


	if(pRunInfo->bs_reset&PR_CH_CD)
	{
	   pelm->bSetChg = TRUE;
	   pRunInfo->bs_reset &= ~PR_CH_CD;
	}

#if (TYPE_USER == USER_BEIJING)	/*北京序列要求必须有压，并且不能判无流*/
	bQd1  = bAT;
	bQd1 &= YxTwj&plogic->Output[BOOL_CH_CD];
	RunTR(&(MY_CH_TIMER1->tMaxTimeRelay[0]), bQd1);
	RunTR(&(MY_CH_TIMER1->tMaxTimeRelay[1]), pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_FZ]);
	MY_CH_QD  = (MY_CH_QD&plogic->Output[BOOL_CH_QD])|MY_CH_TIMER1->tMaxTimeRelay[1].boolTrip;
	bQd3  = MY_CH_QD&plogic->Output[BOOL_CH_CD]&&bYY;
    bQd3 |= MY_CH_TIMER1->tMaxTimeRelay[0].boolTrip;
#else
	bQd1  = bAT&pLogicElem[GRAPH_WL].Output[BOOL_OUTPUT_PICK];
	bQd1 &= YxTwj&plogic->Output[BOOL_CH_CD];
	RunTR(&(MY_CH_TIMER1->tMaxTimeRelay[0]), bQd1);
	RunTR(&(MY_CH_TIMER1->tMaxTimeRelay[1]), pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_FZ]);
	MY_CH_QD  = (MY_CH_QD&plogic->Output[BOOL_CH_QD])|MY_CH_TIMER1->tMaxTimeRelay[1].boolTrip;
	bQd3  = MY_CH_QD&plogic->Output[BOOL_CH_CD]&pLogicElem[GRAPH_WL].Output[BOOL_OUTPUT_PICK];
    bQd3 |= MY_CH_TIMER1->tMaxTimeRelay[0].boolTrip;
#endif
#ifdef INCLUDE_PR_PRO
    bDz = prLogicPublic[GRAPH_CHTQ].Output[BOOL_OUTPUT_TRIP];
#else
    bDz = TRUE;
#endif

	pelm->ppioIn[_PR_26_ENABLE].bVal  = ((pRunSet->dYb&PR_YB_CH)&&(pVal->dYx&PR_KG_YX_CH));

    if(pRunSet->bCd)
	    pelm->ppioIn[_PR_26_CDTJ].bVal    = (!plogic->Output[BOOL_CH_QD])&YxHwj;
	else
		pelm->ppioIn[_PR_26_CDTJ].bVal    = !plogic->Output[BOOL_CH_QD];
	pelm->ppioIn[_PR_26_FDTJ].bVal    = HandTrip|bFd;
	pelm->ppioIn[_PR_26_QDTJ].bVal    = bQd3;
	pelm->ppioIn[_PR_26_DZTJ].bVal    = bDz;

	pelm->ppioIn[_PR_26_T_CH2].ulVal  = pRunSet->dTCH2;
	pelm->ppioIn[_PR_26_T_CH3].ulVal  = pRunSet->dTCH3;
	pelm->ppioIn[_PR_26_T_CH4].ulVal  = Val0;
	pelm->ppioIn[_PR_26_NUM_CH].ulVal = pRunSet->bCHNum;
  
#if (DEV_SP != DEV_SP_DTU)	
	if(prSetPublic[prRunPublic->set_no]->bFTUSD)  // 仅限FTU,不然所有时间都是C时间
	{
		if(pelm->ppioIn[_PR_26_T_CH].ulVal   != prSetPublic[prRunPublic->set_no]->dTXXCH)
		{
			pelm->ppioIn[_PR_26_T_CH].ulVal   = prSetPublic[prRunPublic->set_no]->dTXXCH;
			pelm->bSetChg = 1;
		}
		if(MY_CH_SD == 0)
		{
			pelm->bSetChg = 1;
			MY_CH_SD = 1;
		}
	}
	else
#endif
	{
		pelm->ppioIn[_PR_26_T_CH].ulVal   = pRunSet->dTCH1;
		if((MY_CH_SD == 1))
		{
			MY_CH_SD = 0;
			pelm->bSetChg = 1;
		}
	}
	
  	pelm->Scan_Func(pelm);
	
	plogic->Output[BOOL_CH_CD] = pelm->aioOut[_PR_26_CD_PICK].bVal;
    plogic->Output[BOOL_CH_QD] = pelm->aioOut[_PR_26_QD_PICK].bVal;
//	RunTR(&(MY_CH_TIMER1->tChzTimeRelay[2]), plogic->Output[BOOL_CH_DZ]);
//	RunTR(&(MY_CH_TIMER1->tChzTimeRelay[3]), bDz2);
//	bDz2 = MY_CH_TIMER1->tChzTimeRelay[2].boolTrip||(!MY_CH_TIMER1->tChzTimeRelay[3].boolTrip&bDz2&pLogicElem[GRAPH_WL].Output[BOOL_OUTPUT_PICK]);
	plogic->Output[BOOL_CH_FD] = pelm->aioOut[_PR_26_FD_PICK].bVal;
	plogic->Output[BOOL_CH_FAIL] = pelm->aioOut[_PR_26_FAIL].bVal;

	bDZ  = pelm->aioOut[_PR_26_DZ_PICK].bVal;

//		if(pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_HZ])
//		{
//		   ResetTR(&MY_CH_TIMER1->tMaxTimeRelay[5]);
//		}
//			
//	}
//#endif	
	if(bDZ&&!plogic->Output[BOOL_CH_DZ])
	{
	   if(pRunInfo->vyx & PR_VYX_RC)
	   {
	      pRunInfo->vyx &= ~PR_VYX_RC;
		  prVyxHandle();
	   }
		   pRunInfo->vyx |= PR_VYX_RC;
	   plogic->Output[BOOL_CH_DZ] = TRUE;
	   GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
	   pRunInfo->fault.rc_num = pelm->aulUser[0];

	}
	else if(!bDZ&&plogic->Output[BOOL_CH_DZ])
		plogic->Output[BOOL_CH_DZ] = FALSE;
	RunTR(&(MY_CH_TIMER1->tMaxTimeRelay[3]), !plogic->Output[BOOL_CH_DZ]&&(pRunInfo->vyx&PR_VYX_RC));
	if(MY_CH_TIMER1->tMaxTimeRelay[3].boolTrip)
		pRunInfo->vyx &= ~PR_VYX_RC;
	bRcNum = (pRunInfo->fault.rc_num!=0)?TRUE:FALSE;
	RunTR(&(MY_CH_TIMER1->tMaxTimeRelay[4]), bRcNum);
	if(MY_CH_TIMER1->tMaxTimeRelay[4].boolTrip)
	   pRunInfo->fault.rc_num = pelm->aulUser[0];

}

#define MY_JS_TIMER1  ((TMAXTIMERELAY*)(plogic->apvUser[0]))
#define MY_JS_TIMER2  ((TIMERELAY*)(plogic->apvUser[1]))
#define MY_JS_SD      (plogic->aplUser[1])   /*首端FTU标志*/
void JSLogic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
	plogic->apvUser[1] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	MY_JS_SD = 0;
}

void JSLogic(LogicElem *plogic)
{
   BOOL bJs,bCH,bHwj;

   if(!(pRunSet->dYb&PR_YB_JS))
	 	return;

   if(pRunSet->bSetChg)
   {
      InitTR(&(MY_JS_TIMER1->tMaxTimeRelay[0]), 0, 1000);
      InitTR(&(MY_JS_TIMER1->tMaxTimeRelay[1]), 0, 20000); 
 
#if (DEV_SP != DEV_SP_DTU)
			if(prSetPublic[prRunPublic->set_no]->bFTUSD)
			{
				InitTR(MY_JS_TIMER2, 0, prSetPublic[prRunPublic->set_no]->dTY);
				MY_JS_SD = 1;
			}
			else
#endif
			{	
				InitTR(MY_JS_TIMER2, 0, pRunSet->dTJSFG);
				MY_JS_SD = 0;
			}
   }
#if (DEV_SP != DEV_SP_DTU)
	 if(prSetPublic[prRunPublic->set_no]->bFTUSD && (MY_JS_SD == 0))
	 {
			InitTR(MY_JS_TIMER2, 0, prSetPublic[prRunPublic->set_no]->dTY);
			MY_JS_SD = 1;
	 }
	 else if(!prSetPublic[prRunPublic->set_no]->bFTUSD && MY_JS_SD)
	 {
			InitTR(MY_JS_TIMER2, 0, pRunSet->dTJSFG);
			MY_JS_SD = 0;
	 }
#endif
			 
   if ((pRunSet->dYb & PR_YB_CH)&&(pVal->dYx&PR_KG_YX_CH))
		bCH = TRUE;
   else
		bCH = FALSE;

   bHwj = pVal->dYx&PR_KG_YX_STATUS;
   bJs  = !bCH&&!bHwj&&(pRunInfo->vyx&PR_VYX_SGZ);
   RunTR(&(MY_JS_TIMER1->tMaxTimeRelay[0]), bJs);
   bJs  = MY_JS_TIMER1->tMaxTimeRelay[0].boolTrip&bHwj;
   bJs |= bCH&&pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_HZ];
#if (TYPE_USER == USER_SHANDONG)
#if (DEV_SP != DEV_SP_DTU)
   bJs |= bCH && ((prRunPublic->yk_type == KG_HZ_CONTROL) || (prRunPublic->yk_type == KG_HZ_REMOTE));
#endif
#endif
   RunTR(MY_JS_TIMER2, bJs);
   if(pRunInfo->bs_reset & PR_JS_BS)
   {
      ResetTR(MY_JS_TIMER2);
	  pRunInfo->bs_reset &= ~PR_JS_BS;
   }

   
    //后加速直到分位则复归,从合到分
   /*RunTR(&(MY_JS_TIMER1->tMaxTimeRelay[1]),MY_JS_TIMER2->boolTrip && bHwj);
   if(MY_JS_TIMER1->tMaxTimeRelay[1].boolTrip && (!bHwj)&(!pRunInfo->fault.i_sgz))
   {
   	  ResetTR(MY_JS_TIMER2);
   }*/
   if(MY_JS_TIMER2->boolTrip&&!pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP])
   {
	  pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP] = TRUE;
#ifdef INCLUDE_COMTRADE
	  pRunInfo->wave.connect_pre = TRUE;
#endif
   }
   else if(!MY_JS_TIMER2->boolTrip&&pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP])
   {
	  pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP] = FALSE;
   }
}

#define MY_WY_TIMER1  ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_WY_CNT     (plogic->aplUser[0])
#define MY_WY_CNT2    (plogic->aplUser[1])
void WYFZLogic_Init(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);

	plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
	MY_WY_CNT = 0;

	PRLIB_29(pelm);
}

void WYFZLogic(LogicElem *plogic)
{
    EP_ELEMENT *pelm = &(plogic->elem);
	BOOL bGl=FALSE;
	BOOL bHwj,bYb;

	if(pRunSet->bSetChg)
    {
        InitTR(MY_WY_TIMER1, pRunSet->dTJSIN, 0);
    }

	bGl = bHwj = bYb = FALSE;

    if((pRunSet->bITrip == PR_OC_NUI_TRIP)||
	   (pRunSet->bITrip == PR_OC_NUI2_TRIP))
    {
        bGl = pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_IFZ];
		bHwj = TRUE;
		bYb = TRUE;
    }

    pelm->ppioIn[_PR_29_ENABLE].bVal   = bYb&bGl&bHwj;
	pelm->ppioIn[_PR_29_WL].bVal       = pLogicElem[GRAPH_WL].Output[BOOL_OUTPUT_PICK];
	pelm->ppioIn[_PR_29_UAB].ulVal     = pVal->dUxx[0];
	pelm->ppioIn[_PR_29_UBC].ulVal     = pVal->dUxx[1];
	pelm->ppioIn[_PR_29_UCA].ulVal     = pVal->dUxx[2];

	pelm->Scan_Func(pelm);

	RunTR(MY_WY_TIMER1, !bGl);


	if((MY_WY_TIMER1->boolTrip)||(pRunInfo->bs_reset & PR_WY_CNT))
	{
	   MY_WY_CNT = 0;
	   pRunInfo->bs_reset &= ~PR_WY_CNT;
	}

	if(pelm->aioOut[_PR_29_OPERATE].bVal && !plogic->Output[BOOL_OUTPUT_PICK])
	{
	   plogic->Output[BOOL_OUTPUT_PICK] = TRUE;
	   if(pRunSet->bITrip == PR_OC_NUI2_TRIP)
	   {
	      MY_WY_CNT++;
#ifdef INCLUDE_COMTRADE
		  pRunInfo->wave.connect_pre = TRUE;
#endif
	      if(MY_WY_CNT == 2)
	      {
	         MY_WY_CNT = 0;
			 plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
	      }
		  else
		  	 pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_IFZ] = FALSE;
	   }
	   else
	   	  plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
	}
	else if(!pelm->aioOut[_PR_29_OPERATE].bVal &&plogic->Output[BOOL_OUTPUT_PICK])
	{
	   plogic->Output[BOOL_OUTPUT_PICK] = FALSE;
	   plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	}

	if(plogic->Output[BOOL_OUTPUT_TRIP] && !(pRunInfo->vyx&PR_VYX_GL_WY))
	{
	   pRunInfo->vyx |= PR_VYX_GL_WY;
	   GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
	}
	else if(!plogic->Output[BOOL_OUTPUT_TRIP] &&(pRunInfo->vyx&PR_VYX_GL_WY))
	{
	   pRunInfo->vyx &= ~PR_VYX_GL_WY;
	}
}


#define MY_BHQD_TIMER1 ((TIMERELAY*)(plogic->apvUser[0]))
#define MY_GL_LUBO  (plogic->aplUser[0])
void BhqdLogic_Init(LogicElem *plogic)
{
     plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
     MY_GL_LUBO = 0;
}

void BhqdLogic(LogicElem *plogic)
{
     BOOL bIFault,bI0Fault,bUFault;

	 if(pRunSet->bSetChg)
	 {
	    InitTR(MY_BHQD_TIMER1, 0, 100);
	 }

	 bIFault  = pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_PICK]|
					pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_PICK]|
					pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_PICK];
	 
	  if((bIFault) && (MY_GL_LUBO == 0))
		{
			if(pRunSet->bRcdGl)
				prWavRecord(pRunInfo->fd, NULL, REC_START |REC_STOP);
			MY_GL_LUBO = 1;
		}
		else if((!bIFault) && (MY_GL_LUBO == 1))
		{
			MY_GL_LUBO = 0;
		}
		
		bIFault |= pLogicElem[GRAPH_IGFH].Output[BOOL_OUTPUT_PICK];
		
	 bI0Fault = pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_PICK]|
	 	        pLogicElem[GRAPH_I02].Output[BOOL_OUTPUT_PICK]|
	 	        pLogicElem[GRAPH_I03].Output[BOOL_OUTPUT_PICK];
	 bUFault  = pLogicElem[GRAPH_DY].Output[BOOL_OUTPUT_PICK]|
	 	        pLogicElem[GRAPH_GY].Output[BOOL_OUTPUT_PICK];
   
   
   
#ifdef INCLUDE_PR_PRO
	 bUFault |= pLogicElem[GRAPH_DF].Output[BOOL_OUTPUT_PICK];
#else
     
#endif
	 plogic->Output[BOOL_OUTPUT_PICK] = bIFault|bI0Fault|bUFault;

	 if(pRunSet->bQdQuit)
	 	return;

	 RunTR(MY_BHQD_TIMER1, plogic->Output[BOOL_OUTPUT_PICK]);
#if(TYPE_USER == USER_GUANGXI)
	 if(MY_BHQD_TIMER1->boolTrip&&!Bhqd_GX)
	 {
	    Bhqd_GX = 1;
#else
	 if(MY_BHQD_TIMER1->boolTrip&&!(pRunInfo->vyx&PR_VYX_BHQD))
	 {
	    pRunInfo->vyx |= PR_VYX_BHQD;
#endif
		GetSysClock(&(pRunInfo->fault.i_start_clock), CALCLOCK);
		if(bIFault)
		    pRunInfo->bhqd_state |= PR_OT_I_FAULT;
		if(bI0Fault)
			pRunInfo->bhqd_state |= PR_OT_I0_FAULT;
		if(bUFault)
			pRunInfo->bhqd_state |= PR_OT_U_FAULT;
		prWriteEvent(EVENT_BHQD);
    
	#ifdef INCLUDE_COMTRADE
		pRunInfo->wave.start = TRUE;
	    pRunInfo->wave.stop  = TRUE;
		if(pRunInfo->wave.connect_pre)
		    StartWavRcd(pRunInfo->fd, &(pRunInfo->fault.i_start_clock), REC_NORMAL);
		else
		    StartWavRcd(pRunInfo->fd, &(pRunInfo->fault.i_start_clock), REC_START);
	#endif
	 }
#if(TYPE_USER == USER_GUANGXI)
	 else if(!MY_BHQD_TIMER1->boolTrip&&Bhqd_GX)
	 {
	    Bhqd_GX = 0;
#else
	 else if(!MY_BHQD_TIMER1->boolTrip&&(pRunInfo->vyx&PR_VYX_BHQD))
	 {
	    pRunInfo->vyx &= ~PR_VYX_BHQD;
#endif
	#ifdef INCLUDE_COMTRADE
		if(pRunInfo->wave.start&&pRunInfo->wave.stop)
		{
		    StartWavRcd(pRunInfo->fd, NULL, REC_STOP);
			prComtradeStop();
		}
	#endif
	 }
}

#define MY_RELAY_TIMER1 ((TMAXTIMERELAY*)(plogic->apvUser[0]))
#define MY_RELAY_TIMER2 ((TMAXTIMERELAY*)(plogic->apvUser[1]))
void RelayLogic_Init(LogicElem *plogic)
{
     plogic->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
     plogic->apvUser[1] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
}

void RelayLogic(LogicElem *plogic)
{
     BOOL bIFault,bI0Fault,bUFault,bIGfhFault,bWyTrip,bGFreq;
#ifdef INCLUDE_FA
	BOOL i0_xdltrip;
#endif
	 BOOL bIYxFault,bI0YxFault;
	 BOOL bFZ,bHZ,bTwj,bTwj0,bHwj,bNUFZ,bEver,bKgFZ,bSwitchTrip;
   	 BOOL bFZBS,bHZBS;
	 int i,phase;
     int ret = 0;

	 if(pRunSet->bSetChg)
	 {
		InitTR(&(MY_RELAY_TIMER1->tMaxTimeRelay[0]), 90000, 0);
		InitTR(&(MY_RELAY_TIMER1->tMaxTimeRelay[1]), 90000, 0);
		InitTR(&(MY_RELAY_TIMER1->tMaxTimeRelay[2]), 0, pRunSet->dTCHBS);
#if(TYPE_USER == USER_MULTIMODE)
		InitTR(&(MY_RELAY_TIMER1->tMaxTimeRelay[3]), 50000, 0); //多模遥控合闸后5秒复归
#else
		InitTR(&(MY_RELAY_TIMER1->tMaxTimeRelay[3]), pRunSet->dTJSIN, 0);
#endif
#ifdef _TEST_VER_
		InitTR(&(MY_RELAY_TIMER1->tMaxTimeRelay[4]), pRunSet->dTCHBS, 0);
#endif
		InitTR(&(MY_RELAY_TIMER2->tMaxTimeRelay[0]), pRunSet->dTJSIN, 0);
		InitTR(&(MY_RELAY_TIMER2->tMaxTimeRelay[1]), pRunSet->dTJSIN, 0);
		InitTR(&(MY_RELAY_TIMER2->tMaxTimeRelay[2]), pRunSet->dTRet, 0);
	 }
	  
	bFZBS = bHZBS = FALSE;
	bHZBS = pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_CHBS];
	bFZBS = pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_BS];

    GetSysClock(&(pRunInfo->fault.yk_clock), CALCLOCK);
	if(bFZBS && !(pRunInfo->vyx&PR_VYX_FZ_BS))
	{
	   pRunInfo->vyx |= PR_VYX_FZ_BS;
	   pCfg->yk_trip_dis = 1;
	   if(pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_BS])
	   	 prWriteEvent(EVENT_ZDBS);
	}
	else if(!bFZBS && (pRunInfo->vyx&PR_VYX_FZ_BS))
	{
	  	
   	   RunTR(&(MY_RELAY_TIMER2->tMaxTimeRelay[2]), !bFZBS);
	   if(MY_RELAY_TIMER2->tMaxTimeRelay[2].boolTrip)
	 	   pRunInfo->vyx &= ~PR_VYX_FZ_BS;
	   pCfg->yk_trip_dis = 0;
	}
	if(bHZBS && !(pRunInfo->vyx&PR_VYX_HZ_BS))
	{
	   pRunInfo->vyx |= PR_VYX_HZ_BS;
//	   pCfg->yk_close_dis = 1;
	}
	else if(!bHZBS && (pRunInfo->vyx&PR_VYX_HZ_BS))
	{
	   pRunInfo->vyx &= ~PR_VYX_HZ_BS;
//	   pCfg->yk_close_dis = 0;
	}
     bIFault = bIGfhFault = bI0Fault = bUFault = bIYxFault = bI0YxFault = bWyTrip= bGFreq = FALSE;
	 bIYxFault =  pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_I];
	 if(pRunSet->bI1Trip)
	 {
	 	bIFault |= pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_TRIP];
		bIYxFault  = bIYxFault || pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_I];
	 }
	 else if(pRunSet->bI1GJ)
	 	bIYxFault  = bIYxFault || pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_I];
	 
#if (TYPE_USER != USER_FUJIAN)

	 if(pRunSet->bI2Trip)
	 {
	 	bIFault |= pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_TRIP]||pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_TRIP];
		bIYxFault = bIYxFault || pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_I];
	 }
	 else if(pRunSet->bI2GJ)
	 	bIYxFault = bIYxFault || pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_I];
#else

	if(pRunSet->bI2Trip)
	{
		bIFault |= pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_TRIP];
		bIYxFault = bIYxFault || pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_I];
	}
	else if(pRunSet->bI2GJ)
		bIYxFault = bIYxFault || pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_I];

	if(pRunSet->bI3Trip)
	{
		bIFault |= pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_TRIP];
		bIYxFault = bIYxFault || pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_I];
	}
	else if(pRunSet->bI3GJ)
		bIYxFault = bIYxFault || pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_I];
#endif

	 if(!pRunSet->bGfhGJ)
	 	bIGfhFault = pLogicElem[GRAPH_IGFH].Output[BOOL_OUTPUT_TRIP];
	 if(!pRunSet->bGpGJ)
	 	bGFreq |= pLogicElem[GRAPH_GP].Output[BOOL_OUTPUT_TRIP];
         if(!pRunSet->bDpGJ)
	 	bGFreq |= pLogicElem[GRAPH_DP].Output[BOOL_OUTPUT_TRIP];
	 if(pRunSet->bI0Trip)
	 {
	    bI0Fault = pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_TRIP]|
	 	           pLogicElem[GRAPH_I02].Output[BOOL_OUTPUT_TRIP]|
	 	           pLogicElem[GRAPH_I03].Output[BOOL_OUTPUT_TRIP];
		bI0YxFault = pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_I]||
			         pLogicElem[GRAPH_I02].Output[BOOL_OUTPUT_I]||
			         pLogicElem[GRAPH_I03].Output[BOOL_OUTPUT_I];
	 }
	 else if(pRunSet->bI0GJ)
	 {
	 	bI0YxFault = pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_I]||
			         pLogicElem[GRAPH_I02].Output[BOOL_OUTPUT_I]||
			         pLogicElem[GRAPH_I03].Output[BOOL_OUTPUT_I];
	 }
	  if(!pRunSet->bDyGJ)
	 	bUFault |= pLogicElem[GRAPH_DY].Output[BOOL_OUTPUT_TRIP];
	  if(!pRunSet->bGyGJ)
		bUFault |=  pLogicElem[GRAPH_GY].Output[BOOL_OUTPUT_TRIP];
	
	 bWyTrip = pLogicElem[GRAPH_WYFZ].Output[BOOL_OUTPUT_TRIP];

     plogic->Output[BOOL_OUTPUT_ISGZ] = bIYxFault|bI0YxFault;
  	 bNUFZ = FALSE;
	 bKgFZ = FALSE;
#if (TYPE_USER != USER_FUJIAN)
	 bTwj = pRunSet->bI1Trip|pRunSet->bI2Trip;
	 bTwj0 = pRunSet->bI0Trip;
#else
	 bTwj = pRunSet->bI1Trip|pRunSet->bI2Trip|pRunSet->bI3Trip;
	 bTwj0 = pRunSet->bI0Trip;
#endif
	 switch(pRunSet->bITrip)
	 {
	    case PR_OC_TRIP:
	    #ifdef INCLUDE_FA
		if (!bTwj)
		    bIFault = (pRunInfo->fa_states & FA_KG_IFAULT)?TRUE:FALSE;
		   
		if(!bTwj0)
			 bI0Fault = (pRunInfo->fa_states & FA_KG_I0FAULT)?TRUE:FALSE;	
		
        #endif
		    bFZ = bIFault|bIGfhFault|bI0Fault|bUFault|bWyTrip|bGFreq;
			bEver = bUFault|bIGfhFault|bGFreq;
			break;
		case PR_OC_NUI_TRIP:
		case PR_OC_NUI2_TRIP:
			bNUFZ = TRUE;
			if(bIYxFault|bI0YxFault)
			   plogic->Output[BOOL_OUTPUT_IFZ] = TRUE;
		#ifdef INCLUDE_FA
		if (!(bTwj||bTwj0))
		    bWyTrip = (pRunInfo->fa_states & (FA_KG_IFAULT|FA_KG_I0FAULT))?TRUE:FALSE;
        #endif
			bFZ = bUFault|bIGfhFault|bWyTrip|bGFreq;
			bEver = bUFault|bIGfhFault|bGFreq;
		#ifdef INCLUDE_COMTRADE
			if(plogic->Output[BOOL_OUTPUT_IFZ])
				pRunInfo->wave.stop = FALSE;
		#endif
			break;
		case PR_AT_AUTO_TRIP:
		case PR_NO_TRIP:
		default:
			bKgFZ = TRUE;
			bFZ = bIGfhFault|bUFault|bGFreq;
			bEver = bIGfhFault|bUFault|bGFreq;
			break;
	 }
#ifdef INCLUDE_FA
//	 pRunInfo->fault.i0_xdltrip = (pRunInfo->fa_states & FA_KG_I0CFAULT)?TRUE:FALSE;
     i0_xdltrip = (pRunInfo->fa_states & FA_KG_I0CFAULT)?TRUE:FALSE;
    // bFZ |= i0_xdltrip&&pRunSet->bXDLTrip;

	 if(!pRunSet->bXDLTrip)
	 	bFZ |= i0_xdltrip;
	 else
	 	bFZ |= pRunInfo->fault.i0_xdltrip;
#else
	 bFZ |= pRunInfo->fault.i0_xdltrip&&pRunSet->bXDLTrip;
#endif

     bFZ |= pLogicElem[GRAPH_YXTZ].Output[BOOL_OUTPUT_TRIP]|(pLogicElem[GRAPH_MXQY].Output[BOOL_OUTPUT_TRIP]&&(!pRunSet->bMXQYGJ));


	 if(bEver)
		plogic->Output[BOOL_TRIP_EVER] = TRUE;
	 else if(bFZ && !bEver)
		plogic->Output[BOOL_TRIP_EVER] = FALSE;
	 if(bIYxFault|bI0YxFault)
	 {
	    pRunInfo->fault.i_sgz = TRUE;
		pRunInfo->fault.i_sgz_yx = TRUE;
		for (i=0; i<3; i++)
		{
		   phase = pLogicElem[GRAPH_I1+i].Output[BOOL_OUTPUT_I];
		   if (phase)
		   	  pRunInfo->fault.i_sgz_fx = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_A+phase-1];
		}
		for (i=0; i<3; i++)
		{
		   if (pLogicElem[GRAPH_I01+i].Output[BOOL_OUTPUT_I])
		   	  pRunInfo->fault.i_sgz_fx = pLogicElem[GRAPH_IDIR].Output[BOOL_OUTPUT_N];
		}
	 }
	 else
	 {
	    pRunInfo->fault.i_sgz = FALSE;
	    pRunInfo->fault.i_sgz_fx = FALSE;
	 }

     // 过流后未直接失压，转为正常状态后120s复位状态
	 RunTR(&(MY_RELAY_TIMER2->tMaxTimeRelay[0]), (!(bIYxFault|bI0YxFault))&&pRunInfo->fault.i_sgz_yx);
	 if (MY_RELAY_TIMER2->tMaxTimeRelay[0].boolTrip)
		pRunInfo->fault.i_sgz_yx = FALSE; 
	 RunTR(&(MY_RELAY_TIMER2->tMaxTimeRelay[1]), (!(bIFault|bI0Fault))&&plogic->Output[BOOL_OUTPUT_IFZ]);
	 if (MY_RELAY_TIMER2->tMaxTimeRelay[1].boolTrip)
		plogic->Output[BOOL_OUTPUT_IFZ] = FALSE;
	 
#ifdef INCLUDE_COMTRADE
	 if((pRunSet->dYb&PR_YB_CH)&&pLogicElem[GRAPH_CH].Output[BOOL_CH_CD]
	 	&&(pRunInfo->fault.rc_num<pRunSet->bCHNum)&&(!plogic->Output[BOOL_OUTPUT_CHBS]))
	 {
	    if(bFZ && (bIFault|bI0Fault))
		{
		    pRunInfo->wave.connect_next = TRUE;
			pRunInfo->wave.stop = FALSE;
	    }
		if(bNUFZ && bFZ && pLogicElem[GRAPH_WYFZ].Output[BOOL_OUTPUT_TRIP])
		{
		    pRunInfo->wave.connect_next = TRUE;
			pRunInfo->wave.stop = FALSE;
		}
	 }
#endif
	 if(bFZ&&!bKgFZ&&!pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_BS]&&
	   !(plogic->Output[BOOL_OUTPUT_FZ])&&!(plogic->Output[BOOL_TRIP_FAIL]))
	 {
	    if (GetTripOnInput())
	    {
	       pCfg->yk_trip_dis = 0;
	       ret = prSKgDo(0);
	    }
		plogic->Output[BOOL_OUTPUT_FZ] = TRUE;
		plogic->Output[BOOL_OUTPUT_IFZ] = FALSE;
	    prWriteEvent(EVENT_TRIP);
	 }
	 if(!bFZ)
	 {
	    plogic->Output[BOOL_OUTPUT_FZ] = FALSE;
     }
	
	 bTwj = plogic->Output[BOOL_OUTPUT_FZ]&(GetTripOnInput());//&&(pVal->dYx&PR_KG_YX_STATUS);
	 bTwj&= !pLogicElem[GRAPH_WYFZ].Output[BOOL_OUTPUT_TRIP];
	 RunTR(&(MY_RELAY_TIMER1->tMaxTimeRelay[0]), bTwj);

	 bSwitchTrip = pRunSet->bSwitchTrip;  //增加分闸闭锁控制字
     if(!plogic->Output[BOOL_TRIP_FAIL]&&
	 	((bSwitchTrip && MY_RELAY_TIMER1->tMaxTimeRelay[0].boolTrip)||ret))
     {
         plogic->Output[BOOL_TRIP_FAIL] = TRUE;
		 pRunInfo->vyx |= PR_VYX_TRIP_F;
	     GetSysClock(&(pRunInfo->fault.yk_clock), CALCLOCK);
		 prWriteEvent(EVENT_TGJ);
     }
	 bHZ = pLogicElem[GRAPH_CH].Output[BOOL_CH_DZ];
	 ret = 0;	 	  
	 if(bHZ && !(pVal->dYx&PR_KG_YX_STATUS)&&!bHZBS&&
	 	!plogic->Output[BOOL_OUTPUT_HZ]&&!plogic->Output[BOOL_RC_FAIL])
	 {
	    ret = prSKgDo(1);
		plogic->Output[BOOL_OUTPUT_HZ] = TRUE;
		prWriteEvent(EVENT_CLOSE);
	 }
	 if(!bHZ)
	 {
	    plogic->Output[BOOL_OUTPUT_HZ] = FALSE;
	 }

	 bHwj = pVal->dYx&PR_KG_YX_STATUS;
     RunTR(&(MY_RELAY_TIMER1->tMaxTimeRelay[2]), plogic->Output[BOOL_OUTPUT_HZ]);
	 RunTR(&(MY_RELAY_TIMER1->tMaxTimeRelay[3]), bHwj&&!bIFault&&!bI0Fault);

	 if(MY_RELAY_TIMER1->tMaxTimeRelay[2].boolTrip && (bIFault|bI0Fault))
     {
        plogic->Output[BOOL_OUTPUT_CHBS] = TRUE;
	 }

	 if(MY_RELAY_TIMER1->tMaxTimeRelay[3].boolTrip||(pRunInfo->bs_reset&PR_CH_BS))
	 {
	    plogic->Output[BOOL_OUTPUT_CHBS] = FALSE;
		pRunInfo->bs_reset &= ~PR_CH_BS;
	 }

	 bHwj = plogic->Output[BOOL_OUTPUT_HZ]&&!(pVal->dYx&PR_KG_YX_STATUS);
     RunTR(&(MY_RELAY_TIMER1->tMaxTimeRelay[1]), bHwj);
	 if(!plogic->Output[BOOL_RC_FAIL]&&
	 	(MY_RELAY_TIMER1->tMaxTimeRelay[1].boolTrip||ret))
     {
         plogic->Output[BOOL_RC_FAIL] = TRUE;
		 pRunInfo->vyx |= PR_VYX_TRIP_F;
	     GetSysClock(&(pRunInfo->fault.yk_clock), CALCLOCK);
		 prWriteEvent(EVENT_HGJ);
     }
}

#define MY_HH_TIMER1 ((TIMERELAY*)(plogic->apvUser[0]))
void HHLogic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
}

void HHLogic(LogicElem *plogic)
{
     BOOL bIFault=FALSE;

	 if(pRunSet->bSetChg)
	 	InitTR(MY_HH_TIMER1, 5000, 0);

	 if(pRunSet->bITrip >= PR_AT_AUTO_TRIP)
	 	bIFault |= pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_TRIP]|
	 	           pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_TRIP]|
	 	           pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_TRIP]|
	 	           pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_TRIP]|
	 	           pLogicElem[GRAPH_I02].Output[BOOL_OUTPUT_TRIP]|
	 	           pLogicElem[GRAPH_I03].Output[BOOL_OUTPUT_TRIP];
	 if(pRunSet->bGfhGJ)
	 	bIFault |= pLogicElem[GRAPH_IGFH].Output[BOOL_OUTPUT_TRIP];
	 if(pRunSet->bI0GJ)
	 	bIFault |= pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_TRIP]
	 				   | pLogicElem[GRAPH_I02].Output[BOOL_OUTPUT_TRIP]
	 				   | pLogicElem[GRAPH_I03].Output[BOOL_OUTPUT_TRIP];
	 
	 if(pRunSet->bI1GJ)
	 	bIFault |= pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_TRIP];
	 if(pRunSet->bI2GJ)
	 	bIFault |= pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_TRIP];
#if (TYPE_USER == USER_FUJIAN)
	 if(pRunSet->bI3GJ)
	 	bIFault |= pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_TRIP];
#endif

	 bIFault |= pLogicElem[GRAPH_PT].Output[BOOL_OUTPUT_TRIP];
	 if(bIFault&&!plogic->Output[BOOL_OUTPUT_PICK])
	 {
	    plogic->Output[BOOL_OUTPUT_PICK] = TRUE;
		prWriteEvent(EVENT_HH);
#ifdef INCLUDE_COMTRADE
		StartWavRcd(pRunInfo->fd, NULL, REC_STOP);
#endif
	 }
	 
	 RunTR(MY_HH_TIMER1, plogic->Output[BOOL_OUTPUT_PICK]&!bIFault);
	 if(MY_HH_TIMER1->boolTrip)
	 	plogic->Output[BOOL_OUTPUT_PICK] = FALSE; 
}

#define MY_YX_TIMER1  ((TMAXTIMERELAY*)(plogic->apvUser[0]))
#define MY_YX_SGZ    (plogic->aplUser[0])
#define MY_YX_SGZ_FX (plogic->aplUser[1])
#define MY_NORMAL_STS     (plogic->aplUser[2])

void YxLogic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
	MY_YX_SGZ = 0;
	MY_YX_SGZ_FX = 0;
	MY_NORMAL_STS = 0;
}

#if (TYPE_USER == USER_GUANGXI)
void YxSetLogic(LogicElem *plogic)
{
    BOOL bA,bB,bC;
    pRunInfo->vyx &= ~PR_VYX_YBMASK;
	if (pRunInfo->gzjc_on)
		{
		if(pRunInfo->fd== 0)
		pRunInfo->vyx |= PR_VYX_AUTOON;
		}
	
    if (pRunSet->dYb & PR_YB_I1)
		pRunInfo->vyx |= PR_VYX_I1ON;

	if (pRunSet->dYb & PR_YB_I2)
		pRunInfo->vyx |= PR_VYX_I2ON;

	if (pRunSet->dYb & PR_YB_I3)
		pRunInfo->vyx |= PR_VYX_I3ON;

	if (pRunSet->dYb & PR_YB_I01)
		pRunInfo->vyx |= PR_VYX_I01ON;

	if (pRunSet->dYb & PR_YB_I02)
		pRunInfo->vyx |= PR_VYX_I02ON;

	if (pRunSet->dYb & PR_YB_CH)
	{
	    if (pRunSet->bCHNum == 1)
			pRunInfo->vyx |= PR_VYX_CH1ON;
		if (pRunSet->bCHNum == 2)
			pRunInfo->vyx |= PR_VYX_CH2ON;
	}
	if (pRunSet->dYb & PR_YB_JS)
		pRunInfo->vyx |= PR_VYX_JSON;

    if (pRunSet->bSetChg)
	   InitTR(&(MY_YX_TIMER1->tMaxTimeRelay[7]), 2000, 0);


	if(pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP]&&pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_ISGZ])
		pRunInfo->vyx |= PR_VYX_CHJS;

    RunTR(&(MY_YX_TIMER1->tMaxTimeRelay[7]), (pRunInfo->vyx&PR_VYX_CHJS)!=0);
	if (MY_YX_TIMER1->tMaxTimeRelay[7].boolTrip)
		pRunInfo->vyx &= ~PR_VYX_CHJS;
	
    pRunInfo->vyx &= ~(PR_VYX_ABC|PR_VYX_DL|PR_VYX_JD);
    bA = bB = bC = FALSE;
	if (pRunInfo->vyx & PR_VYX_IA)
		bA = TRUE;
	if (pRunInfo->vyx & PR_VYX_IB)
		bB = TRUE;
	if (pRunInfo->vyx & PR_VYX_IC)
		bB = TRUE;
	if (bA&bB&bC)
		pRunInfo->vyx |= PR_VYX_ABC;
	if (bA|bB|bC)
		pRunInfo->vyx |= PR_VYX_DL;
	if (pRunInfo->vyx&(PR_VYX_I0|PR_VYX_I02))
		pRunInfo->vyx |= PR_VYX_JD;

}
#endif

#if (TYPE_USER == USER_GUIYANG)
void YxSetLogic(LogicElem *plogic)
{
    pRunInfo->vyx &= ~PR_VYX_AUTOON;
	if (pRunInfo->gzjc_on)
		pRunInfo->vyx |= PR_VYX_AUTOON;
}
#endif

#if (TYPE_USER == USER_FUJIAN)
void YxSetLogic(LogicElem *plogic)
{
	 pRunInfo->vyx &= ~(PR_VYX_I1DZ|PR_VYX_I2DZ|PR_VYX_I01DZ|PR_VYX_I0HJSDZ|PR_VYX_GLHJSDZ|PR_VYX_DP|PR_VYX_GP);
	 
	 if((pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_TRIP]) && pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_FZ] && pRunSet->bI1Trip)
	 {
		 pRunInfo->vyx |= PR_VYX_I1DZ;
	 }
	 else
		 pRunInfo->vyx &= ~PR_VYX_I1DZ;
	 
	 if((pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_TRIP]) &&  pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_FZ] && pRunSet->bI2Trip)
	 {
		 pRunInfo->vyx |= PR_VYX_I2DZ;
	 }
	 else
		 pRunInfo->vyx &= ~PR_VYX_I2DZ;
	 
	 if((pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_TRIP]) &&  pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_FZ] && pRunSet->bI3Trip)
	 {
		 pRunInfo->vyx |= PR_VYX_I3DZ;
	 }
	 else
		 pRunInfo->vyx &= ~PR_VYX_I3DZ;
	 
	 if((pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_TRIP]) &&  pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_FZ] && pRunSet->bI0Trip)
	 {
		 pRunInfo->vyx |= PR_VYX_I01DZ;
	 }
	 else
		 pRunInfo->vyx &= ~PR_VYX_I01DZ;
	 
	 if((pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP]) && (pRunInfo->vyx & (PR_VYX_I1DZ|PR_VYX_I2DZ)) )
	 {
			pRunInfo->vyx |= PR_VYX_GLHJSDZ;
	 }
	 else
		 pRunInfo->vyx &= ~PR_VYX_GLHJSDZ;
	 
	 if((pLogicElem[GRAPH_JS].Output[BOOL_OUTPUT_TRIP]) && (pRunInfo->vyx & PR_VYX_I01DZ))
	 {
			pRunInfo->vyx |= PR_VYX_I0HJSDZ;
	 }
	 else
		 pRunInfo->vyx &= ~PR_VYX_I0HJSDZ;
	 
	 if(pLogicElem[GRAPH_GP].Output[BOOL_OUTPUT_TRIP] && (pRunInfo->vyx & PR_VYX_TRIP) && (!pRunSet->bGpGJ))
	 {
		 pRunInfo->vyx |= PR_VYX_GP;
	 }
	 else
		 pRunInfo->vyx &= ~PR_VYX_GP;
	 
	 if(pLogicElem[GRAPH_DP].Output[BOOL_OUTPUT_TRIP] && (pRunInfo->vyx & PR_VYX_TRIP) && (!pRunSet->bDpGJ))
	 {
		 pRunInfo->vyx |= PR_VYX_DP;
	 }
	 else
		 pRunInfo->vyx &= ~PR_VYX_DP;

}
#endif

void YxLogic(LogicElem *plogic)
{
	BOOL bSgz;
	BYTE ykyb;
	if(pRunSet->bSetChg)
	{
	   InitTR(&(MY_YX_TIMER1->tMaxTimeRelay[0]), pRunSet->dTRet, 0);
	   InitTR(&(MY_YX_TIMER1->tMaxTimeRelay[1]), pRunSet->dTRet, 0);
	   InitTR(&(MY_YX_TIMER1->tMaxTimeRelay[2]), pRunSet->dTRet, 0);
	   InitTR(&(MY_YX_TIMER1->tMaxTimeRelay[3]), pRunSet->dTRet, 0);
	   InitTR(&(MY_YX_TIMER1->tMaxTimeRelay[4]), pRunSet->dTRet, 0);
	   InitTR(&(MY_YX_TIMER1->tMaxTimeRelay[5]), 600000, 0);
	   InitTR(&(MY_YX_TIMER1->tMaxTimeRelay[6]), 600000, 0);
		
	   pRunInfo->vyx_oc_mask = PR_VYX_I_MASK;
	   pRunInfo->vyx_warn_mask = PR_VYX_PT;
	   
		if(pRunSet->bI0GJ)
			pRunInfo->vyx_warn_mask |= PR_VYX_I0_MASK;
		else
			pRunInfo->vyx_oc_mask |= PR_VYX_I0_MASK;

		if(pRunSet->bI1GJ)
			pRunInfo->vyx_warn_mask |= PR_VYX_I1;
		else
			pRunInfo->vyx_oc_mask |= PR_VYX_I1;

		if(pRunSet->bI2GJ)
			pRunInfo->vyx_warn_mask |= PR_VYX_I2;
		else
			pRunInfo->vyx_oc_mask |= PR_VYX_I2;
	   
#if (TYPE_USER == USER_FUJIAN)
	    if(pRunSet->bI3GJ)
	   	 pRunInfo->vyx_warn_mask |= PR_VYX_I3;
	   else
	   	 pRunInfo->vyx_oc_mask |= PR_VYX_I3;
#endif

	   if(pRunSet->bGfhGJ)
	   	 pRunInfo->vyx_warn_mask |= PR_VYX_GFH;
	}

	if(pRunInfo->fault.i_phase_bak != pRunInfo->fault.i_phase)
	{
	   if ((pRunInfo->vyx & PR_VYX_I0) && (pRunInfo->fault.i_phase & PR_VYX_I0)&&(!(pRunInfo->fault.i_phase_bak & PR_VYX_I0)))
	   {
	       pRunInfo->vyx &= ~PR_VYX_I0;
		   prVyxHandle();
	   }
	   pRunInfo->vyx |= pRunInfo->fault.i_phase;
	   pRunInfo->fault.i_phase_bak = pRunInfo->fault.i_phase;
	}
	pRunInfo->fault.i_phase = 0;

	switch(pRunSet->bIReport)
	{
	    case PR_OC_REPORT:
	        bSgz = pRunInfo->fault.i_sgz;
		    break;
		case PR_OC_NI_REPORT:
			bSgz = pLogicElem[GRAPH_WL].Output[BOOL_OUTPUT_NI1];
		    break;
		case PR_OC_NI2_REPORT:
			bSgz = pLogicElem[GRAPH_WL].Output[BOOL_OUTPUT_NI2];
			break;
		default:
			bSgz = FALSE;
	}
    if(bSgz&&(MY_YX_SGZ==0))
	{
	  if(pRunInfo->vyx&PR_VYX_SGZ)
      {
         pRunInfo->vyx &= ~PR_VYX_SGZ;
		 pRunInfo->vyx &= ~pRunInfo->vyx_oc_mask;
		 prVyxHandle();
	  }
	  GetSysClock(&(pRunInfo->fault.sgz_clock), CALCLOCK);
	  pRunInfo->vyx |= PR_VYX_SGZ;
	  pRunInfo->vyx |= pRunInfo->fault.i_phase_bak;
	  pRunInfo->vyx |= (pRunInfo->fault.i_sec[0]|pRunInfo->fault.i_sec[1]|pRunInfo->fault.i_sec[2])<<3;
	  MY_YX_SGZ = 1;
	  pRunInfo->fault.i_sgz_yx = FALSE;
	  prLightHandle();
    }
	else if(!bSgz&&MY_YX_SGZ)
		MY_YX_SGZ = 0;
	if(pRunInfo->fault.i_sgz_fx && (MY_YX_SGZ_FX ==0))
	{
		if(pRunInfo->fault.i_sgz_fx)
			pRunInfo->vyx |= PR_VYX_FX;
		else
			pRunInfo->vyx &= ~PR_VYX_FX;
		 MY_YX_SGZ_FX = 1;
	}
	else if(!pRunInfo->fault.i_sgz_fx &&MY_YX_SGZ_FX)
		MY_YX_SGZ_FX = 0;

	
    RunTR(&(MY_YX_TIMER1->tMaxTimeRelay[0]), !bSgz);
	if(MY_YX_TIMER1->tMaxTimeRelay[0].boolTrip&&(pRunInfo->vyx&PR_VYX_SGZ))
	{
	   pRunInfo->vyx &= ~PR_VYX_SGZ;
	   pRunInfo->vyx &= ~PR_VYX_FX;
		prLightHandle();
	}

	//零流自动复归
    RunTR(&(MY_YX_TIMER1->tMaxTimeRelay[2]), (pRunInfo->fault.i_sec[3] == 0));
	if(MY_YX_TIMER1->tMaxTimeRelay[2].boolTrip&&pRunInfo->vyx&PR_VYX_I0)
		pRunInfo->vyx &= ~PR_VYX_I0;

	if(pRunInfo->vyx & (PR_VYX_I1 | PR_VYX_I2 |PR_VYX_I3)) // 加个过流总遥信
		pRunInfo->vyx |= PR_VYX_GL;
	if(pRunInfo->fault.i_sec[3] & 0x02) 				//add by lijun 将开关合闸异常 变位零流二段
		pRunInfo->vyx |= PR_VYX_I02;
	if(MY_YX_TIMER1->tMaxTimeRelay[2].boolTrip && pRunInfo->vyx&PR_VYX_I02)   
		pRunInfo->vyx &= ~(PR_VYX_I02);

	
	RunTR(&(MY_YX_TIMER1->tMaxTimeRelay[3]), (pRunInfo->fault.i_phase_bak == 0));

	if(MY_YX_TIMER1->tMaxTimeRelay[3].boolTrip&&pRunInfo->vyx&(PR_VYX_I_MASK|PR_VYX_HJS))
	{
		pRunInfo->vyx &= ~(PR_VYX_I_MASK|PR_VYX_HJS);
		pRunInfo->kg_ch_ddlbs = FALSE;
	}

	RunTR(&(MY_YX_TIMER1->tMaxTimeRelay[1]), !pLogicElem[GRAPH_RELAY].Output[BOOL_OUTPUT_FZ]&&(pRunInfo->vyx&PR_VYX_TRIP));
	if(MY_YX_TIMER1->tMaxTimeRelay[1].boolTrip)
		pRunInfo->vyx &= ~PR_VYX_TRIP;

	RunTR(&(MY_YX_TIMER1->tMaxTimeRelay[5]), pLogicElem[GRAPH_RELAY].Output[BOOL_TRIP_FAIL] && (!bSgz) && ((pRunInfo->vyx & PR_VYX_TRIP) == 0));//add by lijun 
	if (MY_YX_TIMER1->tMaxTimeRelay[5].boolTrip)
	{		
		pLogicElem[GRAPH_RELAY].Output[BOOL_TRIP_FAIL] = FALSE;
	}
	
	RunTR(&(MY_YX_TIMER1->tMaxTimeRelay[6]), pLogicElem[GRAPH_RELAY].Output[BOOL_RC_FAIL]  && (!bSgz) && ((pRunInfo->vyx & PR_VYX_RC) == 0));
	if (MY_YX_TIMER1->tMaxTimeRelay[6].boolTrip)
	{
		pLogicElem[GRAPH_RELAY].Output[BOOL_RC_FAIL] = FALSE;
	}
	if((pLogicElem[GRAPH_RELAY].Output[BOOL_RC_FAIL] == FALSE) && (pLogicElem[GRAPH_RELAY].Output[BOOL_TRIP_FAIL] == FALSE))
		pRunInfo->vyx &= ~PR_VYX_TRIP_F;
  

     if (pLogicElem[GRAPH_GP].Output[BOOL_OUTPUT_TRIP]) 
	 {
		 pRunInfo->vyx |= PR_VYX_GP;
	 }
	 else
		 pRunInfo->vyx &= ~PR_VYX_GP;
#ifndef PR_ZJJH
	 if (pLogicElem[GRAPH_DP].Output[BOOL_OUTPUT_TRIP]) 
	 {
		 pRunInfo->vyx |= PR_VYX_DP;
	 }
	 else
		 pRunInfo->vyx &= ~PR_VYX_DP;
#endif
#if ((TYPE_USER == USER_GUANGXI)||(TYPE_USER == USER_GUIYANG)||(TYPE_USER == USER_FUJIAN))
   	 YxSetLogic(plogic);
#endif
	
    if(pRunInfo->reset == 0xFFFF)
    {
		prLightHandle();
        if(pRunInfo->fault.i_phase_bak== 0)
           pRunInfo->vyx &= ~PR_VYX_I_MASK;
		if(pRunInfo->fault.i_sec[3] == 0)
#if (TYPE_USER != USER_BEIJING)
		   pRunInfo->vyx &= ~(PR_VYX_I0_MASK |PR_VYX_I02); //零流二段
#else
		   pRunInfo->vyx &= ~(PR_VYX_I0_MASK |PR_VYX_I02|PR_VYX_GL|PR_VYX_I03); //北京零流一二二段
#endif
		if(!bSgz)
        {
           pRunInfo->vyx &= ~PR_VYX_SGZ;
	       pRunInfo->vyx &= ~PR_VYX_FX;
		}
        pRunInfo->vyx &= ~PR_VYX_CL_MASK;
	    pRunInfo->vyx &= ~PR_VYX_TR_MASK;
		pRunInfo->vyx &= ~(PR_VYX_FZ_BS);
#if (TYPE_USER == USER_GUANGXI)
        pRunInfo->vyx &= ~PR_VYX_GZMASK;
#endif
		pRunInfo->bs_reset = PR_RET_BS;
//		pCfg->yk_close_dis = 0;
		pCfg->yk_trip_dis = 0;
		pRunInfo->reset = 0;
		pLogicElem[GRAPH_RELAY].Output[BOOL_TRIP_FAIL] = FALSE;
		pLogicElem[GRAPH_RELAY].Output[BOOL_RC_FAIL] = FALSE;

    }
	
	MY_NORMAL_STS = pRunInfo->normal_sts;
	
	//遥控软压板
	 if(ykGetYb(pRunInfo->fd + 1 , 1 , &ykyb))
	 {
	 	if(ykyb == 0x81)
	 	{
	 		pRunInfo->vyx |= PR_VYX_YK_YB;
			GetSysClock(&(pRunInfo->fault.yk_clock), CALCLOCK);
	 	}
		else
			pRunInfo->vyx &= ~(PR_VYX_YK_YB);
	 }		 
}

#define MY_IRCD_TIMER1  ((TIMERELAY*)(plogic->apvUser[0]))
void IRcdLogic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
}

void IRcdLogic(LogicElem *plogic)
{
    BOOL pick, trip;

	if(pRunSet->bSetChg)
	{
	   InitTR(MY_IRCD_TIMER1, pRunSet->dTRet, 0);
	}

	pick = pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_PICK]|
		   pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_PICK]|
		   pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_PICK]|
		   pLogicElem[GRAPH_IGFH].Output[BOOL_OUTPUT_PICK]|
	       pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_PICK]|
	 	   pLogicElem[GRAPH_I02].Output[BOOL_OUTPUT_PICK]|
	 	   pLogicElem[GRAPH_I03].Output[BOOL_OUTPUT_PICK]|
	 	   pLogicElem[GRAPH_WL].Output[BOOL_OUTPUT_PICK];

	trip = pLogicElem[GRAPH_I1].Output[BOOL_OUTPUT_TRIP]|
	 	   pLogicElem[GRAPH_I2].Output[BOOL_OUTPUT_TRIP]|
	 	   pLogicElem[GRAPH_I3].Output[BOOL_OUTPUT_TRIP]|
	 	   pLogicElem[GRAPH_IGFH].Output[BOOL_OUTPUT_TRIP]|
	 	   pLogicElem[GRAPH_I01].Output[BOOL_OUTPUT_TRIP]|
	 	   pLogicElem[GRAPH_I02].Output[BOOL_OUTPUT_TRIP]|
	 	   pLogicElem[GRAPH_I03].Output[BOOL_OUTPUT_TRIP]|
	 	   pLogicElem[GRAPH_WYFZ].Output[BOOL_OUTPUT_TRIP];

	plogic->Output[BOOL_OUTPUT_PICK] = pick|trip;

	RunTR(MY_IRCD_TIMER1, !(pick|trip));
	plogic->Output[BOOL_OUTPUT_TRIP] = MY_IRCD_TIMER1->boolTrip;

	if (plogic->Output[BOOL_OUTPUT_PICK]&!plogic->Output[BOOL_OUTPUT_TRIP])
		pRunInfo->i_normal = FALSE;
	else
		pRunInfo->i_normal = TRUE;
	
}
extern int g_wFreq;
#define MY_GP_TIMER1   ((TIMERELAY*)(plogic->apvUser[0]))
void GPLogic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
}


void GPLogic(LogicElem *plogic)
{
	 BOOL bFreq = FALSE;
	
	 if(pRunSet->bSetChg)
	 {
	    InitTR(MY_GP_TIMER1, pRunSet->dTGFreq, 0);
             plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	 }
	 if(!(pRunSet->dYb&PR_YB_GP))
	 {
              plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
              return;
	 }

   
	if(g_wFreq > pRunSet->dGFreq)
	 	bFreq = TRUE;
 
	 RunTR(MY_GP_TIMER1, bFreq);
	 plogic->Output[BOOL_OUTPUT_PICK] = MY_GP_TIMER1->boolTrip; 

	 if(plogic->Output[BOOL_OUTPUT_PICK] && !plogic->Output[BOOL_OUTPUT_TRIP])
	{
		plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
		GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
		prWriteEvent(EVENT_GP);
	 }

	 if (!plogic->Output[BOOL_OUTPUT_PICK] && plogic->Output[BOOL_OUTPUT_TRIP])
	 {
	    plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	 }
}

#define MY_DP_TIMER1   ((TIMERELAY*)(plogic->apvUser[0]))
void DPLogic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
}


void DPLogic(LogicElem *plogic)
{
	 BOOL bFreq = FALSE;
	
	 if(pRunSet->bSetChg)
	 {
	    InitTR(MY_DP_TIMER1, pRunSet->dTDFreq, 0);
             plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	 }
	 if(!(pRunSet->dYb&PR_YB_DP))
	 {
               plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
                return;
	 }

   
	if(g_wFreq < pRunSet->dDFreq)
	 	bFreq = TRUE;
 
	 RunTR(MY_DP_TIMER1, bFreq);
	 plogic->Output[BOOL_OUTPUT_PICK] = MY_DP_TIMER1->boolTrip; 

	 if(plogic->Output[BOOL_OUTPUT_PICK] && !plogic->Output[BOOL_OUTPUT_TRIP])
	{
		plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
		GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
		prWriteEvent(EVENT_DP);
	 }

	 if (!plogic->Output[BOOL_OUTPUT_PICK] && plogic->Output[BOOL_OUTPUT_TRIP])
	 {
	    plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	 }
}

void YxTzLogic_Init(LogicElem *plogic)
{
    plogic->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
}
void YxTzLogic(LogicElem *plogic)
{
	struct VDBYX DBYX;
	if (!(pRunSet->dYb& PR_YB_YXTZ))
	{
	    plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
		return;
	}
	if (pRunSet->bSetChg)
	{
		plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	}
	//readyx 吧，这样兼容 内部遥信外部遥信
	DBYX.wNo = pRunSet->wYxNum;
	ReadSYX(BM_EQP_MYIO, DBYX.wNo, &DBYX.byValue);
	if(DBYX.byValue == 0x81)
		plogic->Output[BOOL_OUTPUT_PICK] = TRUE;
	else
		plogic->Output[BOOL_OUTPUT_PICK] = FALSE;
	if(plogic->Output[BOOL_OUTPUT_PICK] && !plogic->Output[BOOL_OUTPUT_TRIP])
	{
		plogic->Output[BOOL_OUTPUT_TRIP] = TRUE;
		GetSysClock(&(pRunInfo->fault.i_clock), CALCLOCK);
		prWriteEvent(EVENT_YXTZ);
	}
	if (!plogic->Output[BOOL_OUTPUT_PICK] && plogic->Output[BOOL_OUTPUT_TRIP])
	{
		 plogic->Output[BOOL_OUTPUT_TRIP] = FALSE;
	}
}
int prGetBhqd(void)
{
    VPrRunInfo *pInfo;
	int qd,i;
	int flag;

    qd = 0;
	for(i=0; i<fdNum; i++)
	{
	   pInfo = prRunInfo+i;
	   flag = pInfo->vyx&PR_VYX_BHQD?1:0;
	   qd |= flag<<i;
	}
	return qd;
}

void prReset(int fd)
{
    int i;
	VPrRunInfo *pRunInfo;

	if(fd == 0)
	{
		for(i=0; i<fdNum; i++)
		{
		    pRunInfo = prRunInfo+i;
			pRunInfo->reset = 0xFFFF; 
		}
       for(i = 0; i < pubfdNum;i++)
       {
		prRunPublic[i].bs_reset = PR_RET_BS;
		prRunPublic[i].js_type |= HZ_JS_FG;
		prRunPublic[i].bs_type = 0;
		prRunPublic[i].vyx &= PR_VYXP_HZ_BS;
       }
	}
	else
	{
		fd = fd-1;
		if ((fd < 0) || (fd >= fdNum))  return;

		pRunInfo = prRunInfo+fd;
	    pRunInfo->reset = 0xFFFF; 
#ifdef INCLUDE_FA
		prFaLogic(fd,0);                        // gu  20170810
#endif
	}
}
#endif

void ResetProtect(int fd)
{
#if (TYPE_IO == IO_TYPE_MASTER)
	VExtIoCmd *pCmd = (VExtIoCmd *)g_Sys.byIOCmdBuf;
	BYTE *pFd = (BYTE *)(pCmd+1);
#endif

if(fdNum > 0)
	{
		#ifdef INCLUDE_PR
			prReset(fd);
		#endif
		
		#ifdef INCLUDE_FA
		    if (fd == 0)
			   FaReset();
		#endif
	}

#if (TYPE_IO == IO_TYPE_MASTER)
	smMTake(g_Sys.dwExtIoDataSem);
	
	pCmd->addr = EXTIO_BCADDR;
	pCmd->cmd = EXTIO_CMD_PR_RESET;
	pCmd->len = 1;
	*pFd = fd;
	ExtIoCmd(NULL, NULL, 0);	
	smMGive(g_Sys.dwExtIoDataSem);				
#endif	
}

BOOL GetFaultYx(int fd)
{
#ifdef INCLUDE_PR
     VPrRunInfo *pRunInfo = prRunInfo+fd;

     return (pRunInfo->vyx & PR_VYX_SGZ);
#else
     return FALSE;
#endif
}

BOOL prReadGzEvent(VDBGzEvent* pGzEvt)
{
#ifdef INCLUDE_PR
	VDBGzEvent *pevt;
	if(prGzEvtOpt == NULL)
		return ERROR;
		
	if(prGzEvtOpt->wReadPtr != prGzEvtOpt->wWritePtr)
	{
		pevt = prGzEvtOpt->evt + prGzEvtOpt->wReadPtr;
		memcpy(pGzEvt,pevt,sizeof(VDBGzEvent));
		prGzEvtOpt->wReadPtr = (prGzEvtOpt->wReadPtr+1)&(MAX_GZEVT_NUM-1);
		return OK;
	}
	else
#endif
		return ERROR;
}

short prNodeCoef(int fd, int flag)
{
#ifdef INCLUDE_PR
    VPrRunSet *pRunSet = prRunSet[pRunInfo->set_no]+fd;
    return pRunSet->coef;
#else
    return 1;
#endif
}

BOOL prValue_long(int ui_flag, int fd, int *value)
{
#ifdef INCLUDE_PR
    int i;
    float tmp[3];
	
    if (ui_flag == 0)
    {
        for (i=0; i<3; i++)
        {
           tmp[i] = prValue_l2f(i+1, pFdCfg+fd, fdProtVal[fd].dU[i]);
		   value[i] = tmp[i] * pFdCfg[fd].pt / pFdCfg[fd].Un;
        }
    }
	else if (prRunInfo[fd].i_normal)
    {
        for (i=0; i<3; i++)
        {
           tmp[i] = prValue_l2f(i+5, pFdCfg+fd, fdProtVal[fd].dI[i]);
		   value[i] = tmp[i] * pFdCfg[fd].ct / pFdCfg[fd].In;
        }
    }
	else
		return FALSE;
#endif
	return TRUE;
}

void prValue_float(int ui_flag, int fd, float *value)
{
#ifdef INCLUDE_PR
    int i;
    if (ui_flag == 0)
    {
        for (i=0; i<3; i++)
        {
           value[i] = prValue_l2f(i+1, pFdCfg+fd, fdProtVal[fd].dU[i]);
        }
    }
	else
    {
        for (i=0; i<3; i++)
        {
           value[i] = prValue_l2f(i+5, pFdCfg+fd, fdProtVal[fd].dI[i]);
        }
    }
#endif
}

void prSetting(int fd, DWORD *setting, int *channel)
{
#ifdef INCLUDE_PR
    VPrRunInfo *pRunInfo = prRunInfo+fd;
    VPrRunSet *pRunSet = prRunSet[pRunInfo->set_no]+fd;
	VFdCfg *pCfg = pFdCfg+fd;
    int i;

	for (i=0; i<4; i++)
	{
	    setting[i] = pRunSet->dIMin[i];
		channel[i] = pCfg->mmi_meaNo_ai[i];
	}
#endif
 }

#ifdef INCLUDE_FA
BOOL prFaLogic(int fd, int faulttype)
{
	int ret = FALSE;
#ifdef INCLUDE_PR
	VPrRunInfo *pRunInfo = prRunInfo + fd;
    VPrRunSet *pRunSet;
	LogicElem *pElem;

	pElem = prLogicElem+fd*GRAPH_NUM;

	if (fd >= fdNum) return ret;

    pRunSet = prRunSet[pRunInfo->set_no]+fd;

	if (faulttype & FA_KG_FAULT)
	   pRunInfo->fa_states |= faulttype;
	else
	   pRunInfo->fa_states &= 0x80000000;

	if ((faulttype & FA_KG_I0CFAULT)&& !pRunSet->bXDLTrip)
		return ret;
//	if (pRunSet->bfZdbh)
	if(pElem[GRAPH_I1].Output[BOOL_OUTPUT_BS])
		return ret;
	ret = TRUE;
#endif
	return ret;
}

void prFaTrip(int fd, int trip)
{
#ifdef INCLUDE_PR
	VPrRunInfo *pRunInfo = prRunInfo + fd;
	if (fd >= fdNum) return;
	pRunInfo->fa_trip = trip;
#endif
}

#endif

