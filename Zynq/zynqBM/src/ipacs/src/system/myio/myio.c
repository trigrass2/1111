/*------------------------------------------------------------------------
 Module:       	myio.c
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-10-7
 Description:  
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/

#include "myio.h"
#include "sys.h"
#include "record.h"

#define SYS_WARN_YXNO (g_Sys.MyCfg.SYXIoNo[1].wIoNo_Low+8)
#define SYS_KGWARN_YXNO (g_Sys.MyCfg.SYXIoNo[1].wIoNo_Low+10)

static DWORD warnStatus;

#define SYS_DY_YXNO  (g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low + 18)
#define SYS_GY_YXNO  (g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low + 19)

int myCellId = 0;     //默认无cellid
int myprResetId = 0;     //默认无prresetid
int myfdYkId = 0;

#if (DEV_SP != DEV_SP_TTU)
UICheckCtrl *pCheckCtrl;
UICheckTime *pChecktime;

void UICheckInit(void)
{
#ifdef INCLUDE_YC
	VYcCfg *pYcCfg;
	int i,yccfgNum; 
	VDefTVYcCfg *pDefTVYcCfg;
	VRunParaCfg *prunparacfg;
	prunparacfg = &g_Sys.MyCfg.RunParaCfg;
	
	if (!gycInit) return;
	
	pChecktime = (UICheckTime*)malloc(sizeof(UICheckTime));
	i = sizeof(UICheckTime);
	pCheckCtrl = (UICheckCtrl*)malloc(fdNum*sizeof(UICheckCtrl));
	i = fdNum*sizeof(UICheckCtrl);
	memset(pChecktime, 0, sizeof(UICheckTime));
	memset(pCheckCtrl, 0 ,fdNum*sizeof(UICheckCtrl));

	pDefTVYcCfg = GetDefTVYcCfg(0, &yccfgNum);	

	for(i=0;i< fdNum;i++)
	{
		pCheckCtrl[i].UNo[0] = -1;
		pCheckCtrl[i].UNo[1] = -1;
		pCheckCtrl[i].UNo[2] = -1;
		pCheckCtrl[i].INo[0] = -1;
		pCheckCtrl[i].INo[1] = -1;
		pCheckCtrl[i].INo[2] = -1;
	}
	
	pYcCfg = g_Sys.MyCfg.pYc;
	
	for(i=0;i<g_Sys.MyCfg.wYCNum;i++)
	{
		if(pYcCfg[i].type == YC_TYPE_Ua)
		{
			pCheckCtrl[pYcCfg[i].fdNo].UNo[0] = i;
			pCheckCtrl[pYcCfg[i].fdNo].dwUk[0] = pYcCfg[i].xs1/(pDefTVYcCfg[pYcCfg[i].type].k*pYcCfg[i].xs2);
		}
		if(pYcCfg[i].type == YC_TYPE_Ub)
		{
			pCheckCtrl[pYcCfg[i].fdNo].UNo[1] = i;
			pCheckCtrl[pYcCfg[i].fdNo].dwUk[1] = pYcCfg[i].xs1/(pDefTVYcCfg[pYcCfg[i].type].k*pYcCfg[i].xs2);
		}
		if(pYcCfg[i].type == YC_TYPE_Uc)
		{
			pCheckCtrl[pYcCfg[i].fdNo].UNo[2] = i;
			pCheckCtrl[pYcCfg[i].fdNo].dwUk[2] = pYcCfg[i].xs1/(pDefTVYcCfg[pYcCfg[i].type].k*pYcCfg[i].xs2);
		}
		if(pYcCfg[i].type == YC_TYPE_Ia)
		{
			pCheckCtrl[pYcCfg[i].fdNo].INo[0] = i;
			pCheckCtrl[pYcCfg[i].fdNo].dwIk[0] = pYcCfg[i].xs1/(pDefTVYcCfg[pYcCfg[i].type].k*pYcCfg[i].xs2);
		}
		if(pYcCfg[i].type == YC_TYPE_Ib)
		{
			pCheckCtrl[pYcCfg[i].fdNo].INo[1] = i;
			pCheckCtrl[pYcCfg[i].fdNo].dwIk[1] = pYcCfg[i].xs1/(pDefTVYcCfg[pYcCfg[i].type].k*pYcCfg[i].xs2);
		}
		if(pYcCfg[i].type == YC_TYPE_Ic)
		{
			pCheckCtrl[pYcCfg[i].fdNo].INo[2] = i;
			pCheckCtrl[pYcCfg[i].fdNo].dwIk[2] = pYcCfg[i].xs1/(pDefTVYcCfg[pYcCfg[i].type].k*pYcCfg[i].xs2);
		}
	}

	InitTR(&pChecktime->ddy,prunparacfg->wDyT*10000,0);
	InitTR(&pChecktime->gdy,prunparacfg->wGyT*10000,0);
#if (TYPE_USER == USER_SHANDONG)
	InitTR(&pChecktime->gz,prunparacfg->wGzT*10,0);
	InitTR(&pChecktime->zz,prunparacfg->wZzT*10,0);
#else
	InitTR(&pChecktime->gz,prunparacfg->wGzT*10000,0);
	InitTR(&pChecktime->zz,prunparacfg->wZzT*10000,0);
#endif
#endif
}

//取电压、电流的二次值
void UICheck(void)
{
#ifdef INCLUDE_YC
	VRunParaCfg *prunparacfg;
	int i,j;
	BOOL bDY,bGY;
	WORD ycno;
	long ycValue;
	struct VDBSOE DBSOE;
    struct VDBCOS DBCOS;
	struct VDBYX DBYX;
	DWORD dwvalue;
	
	if (!gycInit) return;
	
	prunparacfg = &g_Sys.MyCfg.RunParaCfg;

	if(pChecktime->ddy.dTripThreshold != prunparacfg->wDyT*10)
	{
		InitTR(&pChecktime->ddy,prunparacfg->wDyT*10,0);
		ResetTR(&pChecktime->ddy);
	}
	if(pChecktime->gdy.dTripThreshold != prunparacfg->wGyT*10)
	{
		InitTR(&pChecktime->gdy,prunparacfg->wGyT*10,0);
		ResetTR(&pChecktime->gdy);
	}

	bDY = TRUE;
	bGY = FALSE;
	for(i=0;i<fdNum;i++)
	{
		for(j=0;j<3;j++)
		{
			if(pCheckCtrl[i].UNo[j] == -1) continue;
			
			ycno = pCheckCtrl[i].UNo[j];
			
			ReadRangeYCF_L(BM_EQP_MYIO, ycno, &ycValue);
			
			dwvalue = ycValue*1000/pCheckCtrl[i].dwUk[j];
			
			if(dwvalue >= prunparacfg->dwDyVal)
				bDY = FALSE;
			if(dwvalue > prunparacfg->dwGyVal)
				bGY = TRUE;
		}
	}
	RunTR(&pChecktime->ddy, bDY);
	RunTR(&pChecktime->gdy, bGY);
	
/* 低电压 */
#ifndef NW_PRPUBYX
	DBYX.wNo = SYS_DY_YXNO;
	ReadSYX(BM_EQP_MYIO, DBYX.wNo, &DBYX.byValue);
	if((pChecktime->ddy.boolTrip) && (DBYX.byValue != 0x81))
	{
		DBCOS.wNo = DBSOE.wNo = SYS_DY_YXNO;
		DBCOS.byValue = DBSOE.byValue = 0x81;
		GetSysClock(&DBSOE.Time, CALCLOCK); 	
		WriteSSOE(BM_EQP_MYIO, DBSOE.wNo, DBSOE.byValue, DBSOE.Time);
		WriteSYX(BM_EQP_MYIO, DBCOS.wNo,DBCOS.byValue);
	}
	if((!pChecktime->ddy.boolTrip) && (DBYX.byValue != 0x01))
	{
		DBCOS.wNo = DBSOE.wNo = SYS_DY_YXNO;
		DBCOS.byValue = DBSOE.byValue = 0x01;
		GetSysClock(&DBSOE.Time, CALCLOCK); 	
		WriteSSOE(BM_EQP_MYIO ,DBSOE.wNo, DBSOE.byValue, DBSOE.Time);
		WriteSYX(BM_EQP_MYIO, DBCOS.wNo,DBCOS.byValue);
	}
/*过电压  */
	 DBYX.wNo = SYS_GY_YXNO;
	ReadSYX(BM_EQP_MYIO, DBYX.wNo, &DBYX.byValue);
	if((pChecktime->gdy.boolTrip) && (DBYX.byValue != 0x81))
	{
		DBCOS.wNo = DBSOE.wNo = SYS_GY_YXNO;
		DBCOS.byValue = DBSOE.byValue = 0x81;
		GetSysClock(&DBSOE.Time, CALCLOCK); 	
		WriteSSOE(BM_EQP_MYIO, DBSOE.wNo, DBSOE.byValue, DBSOE.Time);
		WriteSYX(BM_EQP_MYIO, DBCOS.wNo,DBCOS.byValue);
	}
	if((!pChecktime->gdy.boolTrip) && (DBYX.byValue != 0x01))
	{
		DBCOS.wNo = DBSOE.wNo = SYS_GY_YXNO;
		DBCOS.byValue = DBSOE.byValue = 0x01;
		GetSysClock(&DBSOE.Time, CALCLOCK); 	
		WriteSSOE(BM_EQP_MYIO, DBSOE.wNo, DBSOE.byValue, DBSOE.Time);
		WriteSYX(BM_EQP_MYIO, DBCOS.wNo,DBCOS.byValue);
	}
#endif
#endif
}
#endif

void miscInit(void)
{
	struct VDBYX DBYX;
	struct VDBSOE DBSOE;
    //cellid  prresetid 
	if (g_Sys.MyCfg.YKIoNo[1].wNum == 0)
	{
		myCellId = 0;
		myprResetId = 0;
	}
    else if (g_Sys.MyCfg.YKIoNo[1].wNum < 3)  
    {
		myCellId = 0;
		myprResetId = g_Sys.MyCfg.wDONum+1;
	}
	else
	{
		myCellId = g_Sys.MyCfg.wDONum+1;
		myprResetId = myCellId+1;
	}

	if (g_Sys.MyCfg.YKIoNo[2].wNum > 0)
		myfdYkId = g_Sys.MyCfg.YKIoNo[2].wIoNo_Low;
  else
		myfdYkId = g_Sys.MyCfg.YKIoNo[1].wIoNo_High;
	//warn light
    if (g_Sys.MyCfg.SYXIoNo[1].wNum == 0) return;

    warnStatus = g_Sys.dwErrCode;
	if (warnStatus)
	{
		DBYX.wNo = DBSOE.wNo = SYS_WARN_YXNO;
		DBYX.byValue = DBSOE.byValue = 0x81;
		WriteSYX(BM_EQP_MYIO, DBYX.wNo,DBYX.byValue);
		GetSysClock(&DBSOE.Time, CALCLOCK); 	
		WriteSSOE(BM_EQP_MYIO, DBSOE.wNo, DBSOE.byValue, DBSOE.Time);
	}
	else
	{
		DBYX.wNo = SYS_WARN_YXNO;
		DBYX.byValue = 0x01;
		WriteSYX(BM_EQP_MYIO, DBYX.wNo,DBYX.byValue);
	}
}

void ioWarn(void)
{
	struct VDBSOE DBSOE;
    struct VDBYX DBYX;
	DWORD aitype,iotype;

	aitype = GetAiType();
	iotype = GetIoType();
	
	if(g_Sys.dwAioType != aitype)
		aitype = GetAiType();
	if(g_Sys.dwDioType != iotype)
		iotype = GetIoType();
	
	WriteMyioType(g_Sys.dwAioType,g_Sys.dwDioType);
	if ((warnStatus != g_Sys.dwErrCode) && (warnStatus == 0))
	{
		DBYX.wNo = DBSOE.wNo = SYS_WARN_YXNO;
		DBYX.byValue = DBSOE.byValue = 0x81;
		WriteSYX(BM_EQP_MYIO, DBYX.wNo,DBYX.byValue);
		GetSysClock(&DBSOE.Time, CALCLOCK); 	
		WriteSSOE(BM_EQP_MYIO, DBSOE.wNo, DBSOE.byValue, DBSOE.Time);

	}
	else if ((warnStatus != g_Sys.dwErrCode) && (g_Sys.dwErrCode == 0))
	{
		DBYX.wNo = DBSOE.wNo = SYS_WARN_YXNO;
		DBYX.byValue = DBSOE.byValue = 0x01;
		WriteSYX(BM_EQP_MYIO, DBYX.wNo,DBYX.byValue);
		GetSysClock(&DBSOE.Time, CALCLOCK); 	
		WriteSSOE(BM_EQP_MYIO, DBSOE.wNo, DBSOE.byValue, DBSOE.Time);

	}	
	warnStatus = g_Sys.dwErrCode;
}

#if 0
void ledCheck(void)
{
#if (DEV_SP == DEV_SP_WDG) 

#if (TYPE_USER != USER_BEIJING) //北京将本地灯改为闭锁灯
   if(GetLocalCtrl()==1)
		turnLight(BSP_LIGHT_LOCAL_ID, 1);
	else
		turnLight(BSP_LIGHT_LOCAL_ID, 0);
	
	if(GetMyExtDiValue(2)==1)
		turnLight(BSP_LIGHT_CELL_ID, 1);
	else
		turnLight(BSP_LIGHT_CELL_ID, 0);
	
#else //北京需要电源灯,活化灯改为电源灯
	turnLight(BSP_LIGHT_CELL_ID, 1);
#endif

  if(GetRemoteCtrl()==1)
		turnLight(BSP_LIGHT_REMOTE_ID, 1);
	else
		turnLight(BSP_LIGHT_REMOTE_ID, 0);

	if(GetDiValue(2) == 1)
		turnLight(BSP_LIGHT_ENG_ID, 1);
	else
		turnLight(BSP_LIGHT_ENG_ID, 0);
	
	if((GetMyExtDiValue(3)==1))   /*取失电告警遥信点亮 TV 灯*/
		turnLight(BSP_LIGHT_TV_ID, 0);
	else
		turnLight(BSP_LIGHT_TV_ID, 1);
#endif
#ifdef _GUANGZHOU_TEST_FTU_
    if(GetLocalCtrl()==1)
		turnLight(BSP_LIGHT_LOCAL_ID, 1);
	else
		turnLight(BSP_LIGHT_LOCAL_ID, 0);
	
    if(GetRemoteCtrl()==1)
		turnLight(BSP_LIGHT_REMOTE_ID, 1);
	else
		turnLight(BSP_LIGHT_REMOTE_ID, 0);
	if(GetMyExtDiValue(1)==1)
		turnLight(BSP_LIGHT_CELLLOW_ID, 1);
	else
		turnLight(BSP_LIGHT_CELLLOW_ID, 0);
	if(GetMyExtDiValue(2)==1)
		turnLight(BSP_LIGHT_CELL_ID, 1);
	else
		turnLight(BSP_LIGHT_CELL_ID, 0);
	if(GetMyExtDiValue(3)==1)
		turnLight(BSP_LIGHT_CELLWARN_ID, 1);
	else
		turnLight(BSP_LIGHT_CELLWARN_ID, 0);
#endif

#if (DEV_SP == DEV_SP_TTU)
        int i;
        for( i = 0 ; i < 8; i++)
        {
            if(GetDiValue(i))
                turnLight(BSP_LIGHT_YX1_ID + i , 1);
            else
                turnLight(BSP_LIGHT_YX1_ID + i, 0 );
        }
		if((ttucomm & 0x3F) == 0x3F)
			turnLight(BSP_LIGHT_RF_ID , 1);
		else
			turnLight(BSP_LIGHT_RF_ID, 0);;
#endif
#if (DEV_SP == DEV_SP_DTU_IU)
	
    if(GetRemoteCtrl()==1)
		turnLight(BSP_LIGHT_REMOTE_ID, 1);
	else
		turnLight(BSP_LIGHT_REMOTE_ID, 0);
#endif

}
#endif

void dio(void)
{
	DWORD events;	

	evReceive(SELF_DIO_ID, EV_TM1|EV_UFLAG|EV_RCD, &events);
	
#ifdef INCLUDE_YX
    if (events & EV_UFLAG) 
	   yxWrite();
#endif

	if (events & EV_TM1)
	{
#ifdef INCLUDE_YX
	   yxWrite();
#endif

#ifdef INCLUDE_EXT_YX
		extYxFilter();            
#endif
#ifdef INCLUDE_PR
        prDataHandle();    
#endif
	}	

	if(events & EV_RCD)
		WAV_Record_Scan();
}

/*
testeqp(BM_EQP_LINE1);
testeqp(BM_EQP_LINE2);
testeqp(BM_EQP_SOCFA);s
void testeqp(WORD eqpid)
{
	struct VDBSOE DBSOE;
	struct VDBCOS DBCOS;
	static WORD cnt = 0;

	DBCOS.wNo = DBSOE.wNo = (cnt++) & 0x0f; 
	DBCOS.byValue = DBSOE.byValue = 0x81;
	
	GetSysClock(&(DBSOE.Time), CALCLOCK);
	WriteSSOE(eqpid, DBSOE.wNo, DBSOE.byValue, DBSOE.Time);
	WriteSYX(eqpid , DBCOS.wNo, DBCOS.byValue);

	WriteRangeYCF_L(eqpid, DBCOS.wNo, cnt*1000);	
}*/

void mea(void)
{
	DWORD evFlag;

#ifdef INCLUDE_YC
    if (gycInit == 0) return;

			
	evReceive(SELF_MEA_ID, EV_TM1, &evFlag);

	if (evFlag&EV_TM1)
	{
		if(((g_Sys.dwErrCode & SYS_ERR_AIO) == 0))
		{
#ifdef INCLUDE_YC
		    measure();
#endif
#if (DEV_SP != DEV_SP_TTU)&&(DEV_SP != DEV_SP_DTU_PU)
			UICheck();
#endif
		}
		ioWarn();
	}

#endif	
}

void mywatchdog(void)
{
	DWORD events;
	static WORD light_cnt=0;
	
	evReceive(WATCHDOG_ID , EV_TM1, &events);

	if (events&EV_TM1)
    {
    	runlinuxled();
        light_cnt++;
			
		
        if (g_Sys.dwErrCode)
        {
        	if(light_cnt > 25)
        	{
				runLight_turn();
				light_cnt = 0;
        	}
        }
		else
		{
			if (light_cnt > 50)
			{
		       runLight_turn();
			   light_cnt = 0;
			}
		}
	}
}

void ioInit(void)
{

#ifdef INCLUDE_YX
	yxInit();
#endif

#ifdef INCLUDE_YK
	ykInit();
#endif

#ifdef INCLUDE_YC
	ycInit();
#endif

#if (DEV_SP != DEV_SP_TTU)&&(DEV_SP != DEV_SP_DTU_PU)
	UICheckInit();
#endif

	miscInit();
	
#ifdef INCLUDE_EXT_YX
	extYxInit();
#endif

}

int GetDiValue(int no)
{
    if (no < 0) return 0;

#ifdef INCLUDE_YX		
	return(GetMyDiValue(no));
#else
	return 0;
#endif
}

int GetMyIoNo(int type, WORD allio_no, WORD *no)
{
	int i, j;
	struct VIoNo *pIoNo;

    if (type == 1)   //SYX
    {
		pIoNo =  g_Sys.MyCfg.SYXIoNo;

	    for (i=0; i<IONO_BUF_NUM; i++)
	    {
			if ((allio_no >= pIoNo->wIoNo_Low) && (allio_no < pIoNo->wIoNo_High)) 
			{
				*no = allio_no-pIoNo->wIoNo_Low;
				for (j=0; j<i; j++)
					*no += g_Sys.MyCfg.SYXIoNo[j].wNum;

				return OK;
			}

			pIoNo++;
		}
	}
	else if (type == 2)  //YK
	{
		pIoNo =  g_Sys.MyCfg.YKIoNo;

	    for (i=0; i<IONO_BUF_NUM; i++)
	    {
			if ((allio_no >= pIoNo->wIoNo_Low) && (allio_no < pIoNo->wIoNo_High)) 
			{
				*no = allio_no-pIoNo->wIoNo_Low+1;
				for (j=0; j<i; j++)
					*no += g_Sys.MyCfg.YKIoNo[j].wNum;

				return OK;
			}

			pIoNo++;
		}
	}	
	else if (type == 3)  //YT
	{
        if (allio_no < g_Sys.MyCfg.wYTNum)
        {
			*no = allio_no;
			return OK;
        }	
	}
	else if (type == 4)  //Fd
	{
        if (allio_no < g_Sys.MyCfg.wFDNum)
        {
			*no = allio_no;
			return OK;
        }	
	}	

	return ERROR;	
}

int GetTripOnInput(void)
{
  int val = 1;

#if (DEV_SP == DEV_SP_FTU)
#ifdef INCLUDE_EXT_DISP
	 val =  GetExtMmiYx(EXTMMI_TRIP);
#endif
#endif
#if (DEV_SP == DEV_SP_WDG)
	if(g_wdg_ver)
	 val =  GetExtMmiYx(XINEXTMMI_TRIP);  //新罩式
#endif
     return val;
}

