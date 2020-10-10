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
#include "bsp.h"

#ifdef ECC_MODE_CHIP
#include "sgc1126a.h"
#endif

#ifdef INCLUDE_TMP75_TEMP
#include "tmp75.h"
#endif

static struct VMsg dioMsg;

#define SYS_WARN_YXNO (g_Sys.MyCfg.SYXIoNo[1].wIoNo_Low+8)
#define SYS_KGWARN_YXNO (g_Sys.MyCfg.SYXIoNo[1].wIoNo_Low+10)


int myCellId = 0;     //默认无cellid
int myprResetId = 0;     //默认无prresetid
int myfdYkId = 0;
VCellState *g_CellState;
VKgStatus  *g_KgStatus;
VKgUnlock  *g_kgunlock;
static DWORD g_KgErr;
static int *old_tr;
static int *old_rc;

DWORD ttucomm = 0; 
#ifdef INCLUDE_YK_KEEP
static int *old_kgstate1;
static int *old_kgstate2;
static int *old_pbkgstate;
YKKeep *g_ykkeep;
static VDoCfg *pDoCfg;
#endif
#ifdef INCLUDE_YK_CZDY
CzdyYk* pCzdyYk; //操作电源遥控
static int czdyykinit = 0;
#endif
BYTE RemoteYxState = 0;
BYTE ParaYzStatue = 0;
struct VParaInfo m_ParaInfo;

extern WORD atomicReadYC(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYC *buf,BOOL asSend);

//如果没有有电池活化YK, 则必须选用PDefTVYkCfg_Public1, 同时donum为Do2Point
//如果有电池活化YK, 则必须选用PDefTVYkCfg_Public2, 同时donum为Do2Point-1
//上述规则必须严格遵守,否则做电池活化的时候会导致死机
//也就是说当有电池活化的时候,最后一个YK为出口节点,但在YK列表中隐藏
//而将其转定义的数据库中的"电池活化"点号, 当做该点的时候, 做donum+1 点YK
int cellOutput(int value)
{
#ifdef INCLUDE_CELL
#if (TYPE_IO == IO_TYPE_MASTER)
    WORD id;
	int ret;
	struct VExtIoConfig *pCfg;
	VExtIoCmd *pCmd = (VExtIoCmd *)g_Sys.byIOCmdBuf;
	VYkOutput *pYk = (VYkOutput *)(pCmd+1);
#endif

    if (myCellId == 0)  
    {
#if (TYPE_IO == IO_TYPE_MASTER)
		pCfg = GetExtIoNo(2, g_Sys.MyCfg.CellCfg.wID, &id);
		if (pCfg == NULL)
			return 1;
		else
		{
			smMTake(g_Sys.dwExtIoDataSem);
            pCmd->addr = pCfg->pAddrList->ExtIoAddr.wAddr;
			pCmd->cmd = EXTIO_CMD_YKOUTPUT;
			pCmd->len = sizeof(VYkOutput);
			pYk->id = id;
			pYk->value = value;
			pYk->timeout = 0;
			ret = ExtIoCmd(NULL, NULL, SECTOTICK(2));
			smMGive(g_Sys.dwExtIoDataSem);		

			if (ret == ERROR)
				return 1;
			else
				return 0;
		}
#else
		return 1;
#endif
	}
	else
	{
		return ykOutput(myCellId, value);
	}	
#else
	return 1;
#endif
}

//当前电池活化状态

int GetCellStatus(void)
{
#ifdef INCLUDE_CELL
	if((GetMyExtDiValue(2)==1))
	  	return 1;
	else
     return 0;  
#else
     return 0;
#endif
}

//本地控制电池活化
int cellLocalCtrl(void)
{
   if (GetCellStatus())
	   cellOutput(0);
	 else
	 	 cellOutput(1);
	
	return 0;
}

#ifdef INCLUDE_CELL
void cellInit(void)
{
	int i;
	struct VSysClock curclock;
	VCellCfg *pcfg = (VCellCfg *)&g_Sys.MyCfg.CellCfg;

	if ((pcfg->dwCfg & 0x01) && (pcfg->wID == 0))
	{
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "蓄电池管理控制点号无效!");
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Invalid Yk ID at Cellctrl!"); 
	}
	g_CellState = calloc(pcfg->dwNum, sizeof(VCellState));
	
	GetSysClock(&curclock, SYSCLOCK);
	for(i = 0;i < pcfg->dwNum;i++)
	{
		ToCalClock(&curclock, &g_CellState[i].oldtime);
	}
	myprintf(SYS_ID, LOG_ATTR_INFO, "Cellctrl management enter"); 
}

void cellCtrl(void)
{
    int i;
	WORD extYcNum,extYcStart;
	DWORD month_bit, day_bit, week_bit;
	static struct VSysClock oldclock;
	struct VSysClock curclock;
	struct VCalClock cal1,cal2;
	struct VYCF ExtYcValue;
	VCellCfg *pcfg = (VCellCfg *)&g_Sys.MyCfg.CellCfg;
	VCellCtrl *pctrl = pcfg->pCtrl;
	VCellState *pstate = g_CellState;

	extYcNum   = g_Sys.MyCfg.wYCNumPublic;
	extYcStart = g_Sys.MyCfg.wYCNum - extYcNum;
	   
	ReadRangeAllYCF(g_Sys.wIOEqpID, extYcStart, 1,sizeof(struct VYCF), &ExtYcValue);	

	if ((pcfg->dwCfg & 0x01) == 0) return;

	if (pctrl == NULL) return;

	if((ExtYcValue.nValue < g_Sys.MyCfg.CellCfg.Udis)&&(g_Sys.MyCfg.CellCfg.dwCfg&0x2))
	{
		if(GetCellStatus())
			cellOutput(0);
		return;
	}
		
	GetSysClock(&curclock, SYSCLOCK);

	if (curclock.byMinute != oldclock.byMinute)			
	{
		for (i=0; i<pcfg->dwNum; i++)
		{
			if (pctrl->dwMode == 0)  
			{
				pctrl++;
				pstate++;
				continue;
			}	

			if ((curclock.byMinute == pctrl->wMin) && (curclock.byHour == pctrl->wHour))
			{
				month_bit = 1<<curclock.byMonth;
				day_bit = 1<<curclock.byDay;
				week_bit = 1<<curclock.byWeek;
				
				if (pctrl->dwMode == 1)	//指定日期模式
				{
					if ((pctrl->wMonthBits & month_bit) && (pctrl->dwDayBits & day_bit))
					{
						cellOutput(1);
						memcpy(&(pstate->execlock), &curclock, sizeof(struct VSysClock));
		                myprintf(SYS_ID, LOG_ATTR_INFO, "Cell activation enter,%04d/%02d/%02d;%02d:%02d",
			                     curclock.wYear,curclock.byMonth,curclock.byDay,curclock.byHour,curclock.byMinute);
						pstate->exestate = TRUE;
						WriteUlogEvent(NULL, 07, 1,"Battery activation enter!");
					}					
				}
				else if (pctrl->dwMode == 2)	//指定星期模式
				{
					if ((pctrl->wMonthBits & month_bit) && (pctrl->wWeekBits & week_bit))
					{
						cellOutput(1);
						memcpy(&(pstate->execlock), &curclock, sizeof(struct VSysClock));
						pstate->exestate = TRUE;
					}
				}
				else if(pctrl->dwMode == 3)  //国网模式
				{
					ToCalClock(&curclock, &cal2);
					if((cal2.dwMinute - pstate->oldtime.dwMinute) >= (pctrl->dwDayBits*24*60))
					{
						cellOutput(1);
						memcpy(&(pstate->execlock), &curclock, sizeof(struct VSysClock));
		                myprintf(SYS_ID, LOG_ATTR_INFO, "Cell activation enter,%04d/%02d/%02d;%02d:%02d",
			                     curclock.wYear,curclock.byMonth,curclock.byDay,curclock.byHour,curclock.byMinute);
						pstate->exestate = TRUE;
						ToCalClock(&(pstate->execlock), &pstate->oldtime);
						WriteUlogEvent(NULL, 07, 1,"Battery activation enter!");
					}
				}
			}
			if ((pctrl->bEndTime) && (pstate->exestate))
			{
			    ToCalClock(&(pstate->execlock), &cal1);
				ToCalClock(&curclock, &cal2);
				if ((cal2.dwMinute - cal1.dwMinute) >= pctrl->dwTime)
				{
				    cellOutput(0);
		            myprintf(SYS_ID, LOG_ATTR_INFO, "Cell activation exit,%04d/%02d/%02d;%02d:%02d",
			                 curclock.wYear,curclock.byMonth,curclock.byDay,curclock.byHour,curclock.byMinute); 
					pstate->exestate = FALSE;
					WriteUlogEvent(NULL, 07, 0,"Battery activation exit!");
				}
			}

			pctrl++;
			pstate++;
		}
	}	
	
	memcpy(&oldclock, &curclock, sizeof(struct VSysClock));
}
#endif

#ifdef INCLUDE_USWITCH
int uswitchIsLow(int *no, int limit, short *val)
{
	int i;

	for (i=0; i<3; i++)
	{
		if (no[i] < 0) continue;

		if (val[i] > limit)  
			return 0;			
	}

	return 1;
}

int uswitchGetNormalRef(int *no)
{
    int i, t, find;
	VYcCfg *pYcCfg;
	
	pYcCfg = g_Sys.MyCfg.pYc;

	find = 0;
	for (i=no[0]+1; i<g_Sys.MyCfg.wYCNum; i++)
	{
		t = pYcCfg[i].type;

		if (t == YC_TYPE_Ua) break;

		if ((t == YC_TYPE_Pa) || (t == YC_TYPE_Qa))
		{
			find = 1;
			pYcCfg[i].us_arg1 = no[0];			
		}
		else if ((t == YC_TYPE_Pb) || (t == YC_TYPE_Qb))
		{
			pYcCfg[i].us_arg1 = no[1];
		}
		else if ((t == YC_TYPE_Pc) || (t == YC_TYPE_Qc))
		{
			pYcCfg[i].us_arg1 = no[2];
		}		
	}

	if (find) 
		return OK;
	else 
		return ERROR;
}

int uswitchGetState(int *no)
{
    int i, t;
	VYcCfg *pYcCfg;
	
	pYcCfg = g_Sys.MyCfg.pYc;

	for (i=no[0]+1; i<g_Sys.MyCfg.wYCNum; i++)
	{
		t = pYcCfg[i].type;

		if (t == YC_TYPE_Ua) break;

		if (t == YC_TYPE_Pa)
		{
			if (pYcCfg[i].us_arg1 == pYcCfg[i].arg1)
				return 1;
			else
				return 0;				
		}
	}

	return 0;
}

void uswitchChgRef(int *old_no, int *new_no)
{
    int i, t, arg1;
	VYcCfg *pYcCfg;
	
	pYcCfg = g_Sys.MyCfg.pYc;

	for (i=0; i<g_Sys.MyCfg.wYCNum; i++)
	{
		t = pYcCfg[i].type;

		arg1 = pYcCfg[i].arg1;

		if (((t == YC_TYPE_Pa) || (t == YC_TYPE_Qa)) && (arg1 == old_no[0]))
		{
			pYcCfg[i].us_arg1 = new_no[0];	
		}
		else if (((t == YC_TYPE_Pb) || (t == YC_TYPE_Qb)) && (arg1 == old_no[1]))
		{
			pYcCfg[i].us_arg1 = new_no[1];
		}
		else if (((t == YC_TYPE_Pc) || (t == YC_TYPE_Qc)) && (arg1 == old_no[2]))
		{
			pYcCfg[i].us_arg1 = new_no[2];
		}		
	}
}

int uswtichWrtiePara(VUSwitchCtrl *pctrl)
{
    int i, t, ret = ERROR, chg;
	VYcCfg *pYcCfg;
	struct VPSysConfig *pPSysCfg;
	struct VPFdCfg *pPFdCfg;
	struct VPAiCfg *pPAiCfg;
	struct VPDiCfg *pPDiCfg;
	struct VPDoCfg *pPDoCfg;
	struct VPYcCfg *pPYcCfg;
#if (TYPE_IO == IO_TYPE_EXTIO)
	struct VMsgHead *pMsgHead;
	char *fname;
#endif

	smMTake(g_paraFTBSem);
	if (ReadParaFile("system.cfg", (BYTE *)g_pParafileTmp, MAXFILELEN) == ERROR)		
	{
		pctrl->dwMode = 0;
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "电压切换系统参数文件读取失败!");
		myprintf(SYS_ID, LOG_ATTR_ERROR, "USwitch read system.cfg fail!"); 	
		ret = ERROR;
	}
	else
	{
		pYcCfg = g_Sys.MyCfg.pYc;
		pPSysCfg = (struct VPSysConfig *)(g_pParafileTmp+1);
		pPFdCfg = (struct VPFdCfg *)(pPSysCfg+1);
		pPAiCfg = (struct VPAiCfg *)(pPFdCfg+pPSysCfg->wFDNum);
		pPDiCfg = (struct VPDiCfg *)(pPAiCfg+pPSysCfg->wAINum);
		pPDoCfg = (struct VPDoCfg *)(pPDiCfg+pPSysCfg->wDINum);
		pPYcCfg = (struct VPYcCfg *)(pPDoCfg+pPSysCfg->wDONum);

		chg = 0;
		for (i=0; i<pPSysCfg->wYCNum; i++)
		{
			t = pPYcCfg[i].type;
			
			if ((t >= YC_TYPE_Pa) && (t <= YC_TYPE_Qc))
			{
				if (pPYcCfg[i].arg1 != pYcCfg[i].us_arg1)
				{
					pPYcCfg[i].arg1 = pYcCfg[i].us_arg1;
					chg = 1;
				}	
			}
		}		

		if (chg)
			ret = WriteParaFile("system.cfg", g_pParafileTmp);

		if (ret == OK)
		{
#if (TYPE_IO == IO_TYPE_EXTIO)
			pMsgHead = (struct VMsgHead *)g_pParafileTmp;
			fname = (char *)(pMsgHead+1);
			strcpy(fname, "system.cfg");
			TaskSendMsg(BUS_485_ID, MISC_ID, 0, MI_EXTIO, MA_EXTIO_CFGUPLOAD, strlen(fname), (struct VMsg *)pMsgHead);				
#else			
			shellprintf("USwitch updata cfg and reset...\n");
			thSleep(200);				
			reboot(0);
#endif
		}
	}		
	smMGive(g_paraFTBSem);

	return ret;		
}

void uswitchDo(VUSwitchCtrl *pctrl)
{
	uswitchGetNormalRef(pctrl->U1No);
	uswitchGetNormalRef(pctrl->U2No);

	if ((pctrl->kgState1 == 0) && (pctrl->kgState2 == 1))
	{
		uswitchChgRef(pctrl->U1No, pctrl->U2No);
	}
	else if ((pctrl->kgState1 == 1) && (pctrl->kgState2 == 0))
	{
		uswitchChgRef(pctrl->U2No, pctrl->U1No);
	}

	uswtichWrtiePara(pctrl);
}

void uswitchInit(void)
{
	int i, j, fd, un;
	VYcCfg *pYcCfg;
	VUSwitchCtrl *pctrl;
    VUSwitchCfg *pcfg = (VUSwitchCfg *)&g_Sys.MyCfg.USwitchCfg;
	
	if ((pcfg->dwCfg & 0x01) == 0)  return;

	pctrl = pcfg->pCtrl;
	
	if (pctrl == NULL) return; 
	
	for (i=0; i<pcfg->dwNum; i++)
	{
		if (pctrl->wUFdNo1 == pctrl->wUFdNo2)
		{
			pctrl->dwMode = 0;
			WriteWarnEvent(NULL, SYS_ERR_CFG, i+1, "电压切换控制%d中电压组数相同!", i+1);
			myprintf(SYS_ID, LOG_ATTR_ERROR, "U No. is same at USwitch%d!", i+1);	
			pctrl++;
			continue;
		}

		if ((pctrl->dwMode == 2) &&  (pctrl->wUFdYx1 == pctrl->wUFdYx2))
		{
			pctrl->dwMode = 0;
			WriteWarnEvent(NULL, SYS_ERR_CFG, i+1, "电压切换控制%d中开关遥信号相同!", i+1);
			myprintf(SYS_ID, LOG_ATTR_ERROR, "Yx No. is same at USwitch%d!", i+1);			
			pctrl++;
			continue;
		}

        for (j=0; j<3; j++)
		{
			pctrl->U1No[j] = -1;
			pctrl->U2No[j] = -1;
        }	

		pYcCfg = g_Sys.MyCfg.pYc;

		fd = -1;
		for (j=0; j<g_Sys.MyCfg.wYCNum; j++)
		{
			if (pYcCfg[j].type == YC_TYPE_Ua) 
			{
				fd++;

				if ((fd == pctrl->wUFdNo1) && (pctrl->U1No[0] == -1))
				{
					if (pctrl->dwMode == 1)
					{
						if ((pYcCfg[j].fdNo < 0) || (pYcCfg[j].fdNo >= g_Sys.MyCfg.wFDNum))
							continue;
                 
						un = g_Sys.MyCfg.pFd[pYcCfg[j].fdNo].Un*100;
						pctrl->UL1 = un*3/10;
					}
					
					pctrl->U1No[0] = j;
				}	

				if ((fd == pctrl->wUFdNo2) && (pctrl->U2No[0] == -1))
				{
					if (pctrl->dwMode == 1)
					{
						if ((pYcCfg[j].fdNo < 0) || (pYcCfg[j].fdNo >= g_Sys.MyCfg.wFDNum))
							continue;

						un = g_Sys.MyCfg.pFd[pYcCfg[j].fdNo].Un*100;
						pctrl->UL2 = un*3/10;
					}

					pctrl->U2No[0] = j;						
				}	
			}	

			if (pYcCfg[j].type == YC_TYPE_Ub) 
			{
				if ((fd == pctrl->wUFdNo1) && (pctrl->U1No[1] == -1))
					pctrl->U1No[1] = j;
			
				if ((fd == pctrl->wUFdNo2) && (pctrl->U2No[1] == -1))
					pctrl->U2No[1] = j;						
			}	
			
			if (pYcCfg[j].type == YC_TYPE_Uc) 
			{
				if ((fd == pctrl->wUFdNo1) && (pctrl->U1No[2] == -1))
					pctrl->U1No[2] = j;

				if ((fd == pctrl->wUFdNo2)  && (pctrl->U2No[2] == -1))
					pctrl->U2No[2] = j;						
			}					
		}

        if (pctrl->U1No[0] == -1)
        {
			pctrl->dwMode = 0;
			WriteWarnEvent(NULL, SYS_ERR_CFG, i+1, "电压切换控制%d中电压组数1非法!", i+1);
			myprintf(SYS_ID, LOG_ATTR_ERROR, "Invalide U1 No. at USwitch%d!", i+1);			
			pctrl++;
			continue;
		}

        if (pctrl->U2No[0] == -1)
        {
			pctrl->dwMode = 0;
			WriteWarnEvent(NULL, SYS_ERR_CFG, i+1, "电压切换控制%d中电压组数2非法!", i+1);
			myprintf(SYS_ID, LOG_ATTR_ERROR, "Invalide U2 No. at USwitch%d!", i+1);			
			pctrl++;
			continue;
		}

		if (uswitchGetNormalRef(pctrl->U1No) == ERROR)
		{
			pctrl->dwMode = 0;
			pctrl++;
			continue;
		}	
		
		if (uswitchGetNormalRef(pctrl->U2No) == ERROR)
		{
			pctrl->dwMode = 0;
			pctrl++;
			continue;
		}			
		
		pctrl->kgState1 = uswitchGetState(pctrl->U1No);
		pctrl->kgState2 = uswitchGetState(pctrl->U2No); 		

#if (TYPE_IO == IO_TYPE_EXTIO)
		if (pctrl->dwMode == 2)
		{
			refYxReg(1, &pctrl->wUFdYx1);
			refYxReg(1, &pctrl->wUFdYx2);
		}
#endif

		pctrl++;
	}	
}

void uswitchCtrl(void)
{
	int i, j, kgstate1, kgstate2;
	struct VDBYC DBYC;
	short val1[3], val2[3];
	VUSwitchCtrl *pctrl;
    VUSwitchCfg *pcfg = (VUSwitchCfg *)&g_Sys.MyCfg.USwitchCfg;
	static int old_kgstate1 = 0;
	static int old_kgstate2 = 0;

    if ((gycInit == 0) || (g_Sys.dwErrCode & SYS_ERR_AIO))  return;

	if ((pcfg->dwCfg & 0x01) == 0)  return;

	pctrl = pcfg->pCtrl;
	
	if (pctrl == NULL) return;
	
	for (i=0; i<pcfg->dwNum; i++)
	{
		if (pctrl->dwMode == 1)
		{
			for (j=0; j<3; j++)
			{
				if (pctrl->U1No[j] >= 0)
				{
					DBYC.wNo = pctrl->U1No[j];
					atomicReadYC(g_Sys.wIOEqpID, 1, sizeof(DBYC), &DBYC, FALSE);
					val1[j] = DBYC.nValue;
				}	

				if (pctrl->U2No[j] >= 0)
				{
					DBYC.wNo = pctrl->U2No[j];
					atomicReadYC(g_Sys.wIOEqpID, 1, sizeof(DBYC), &DBYC, FALSE);
					val2[j] = DBYC.nValue;
				}	
			}	

			if (uswitchIsLow(pctrl->U1No, pctrl->UL1, val1))
				kgstate1 = 0;
			else
				kgstate1 = 1;

			if (uswitchIsLow(pctrl->U2No, pctrl->UL2, val2))
				kgstate2 = 0;
			else
				kgstate2 = 1;
		}
		else if (pctrl->dwMode == 2)
		{
#if (TYPE_IO == IO_TYPE_EXTIO)
			kgstate1 = GetRefYxValue(pctrl->wUFdYx1);
			kgstate2 = GetRefYxValue(pctrl->wUFdYx2);
#else
			kgstate1 = GetDiValue(pctrl->wUFdYx1);
			kgstate2 = GetDiValue(pctrl->wUFdYx2);
#endif
		}
		else
		{
			pctrl++;
			continue;
		}

        if ((kgstate1 == 0) && (kgstate2 == 0))  
		{
			pctrl++;
			continue;
        }	

		if ((kgstate1 != pctrl->kgState1) || (kgstate2 != pctrl->kgState2))
		{
			if ((kgstate1 != old_kgstate1) || (kgstate2 != old_kgstate2))
				pctrl->chgMsCnt = Get100usCnt();

			if ((Get100usCnt() - pctrl->chgMsCnt) >= 150000) 
			{
				pctrl->kgState1 = kgstate1;
				pctrl->kgState2 = kgstate2;
				uswitchDo(pctrl);
			}
		}
		
		old_kgstate1 = kgstate1;
		old_kgstate2 = kgstate2;	

		pctrl++;
	}
}	
#endif

int GetLocalCtrl(void)
{
	//远方就地按钮接入
	//!!远方本地仅对远程控制有效,保护及当地命令行操作无效

    return (GetMyExtDiValue(4)&&!GetMyExtDiValue(5)&&!(GetMyExtDiValue(6)));
}

int GetRemoteCtrl(void)
{
	//远方就地按钮接入
	//!!远方本地仅对远程控制有效,保护及当地命令行操作无效

    return (GetMyExtDiValue(5)&&!GetMyExtDiValue(4)&&!(GetMyExtDiValue(6)));
}

int GetBsCtrl(void)
{
    return (GetMyExtDiValue(6));
}

int GetLocalYkInput(VFdCfg *pCfg,int fd)
{
	//static int old_tr[18] 直接弄 提示空间不足
  static BOOL first = true;
	int tr, rc, ret;	
	
	if(first)
	{
		old_tr = calloc(MAX_FD_NUM,sizeof(int));
		memset(old_tr,0,sizeof(int)*MAX_FD_NUM);
		
		old_rc = calloc(MAX_FD_NUM,sizeof(int));
		memset(old_rc,0,sizeof(int)*MAX_FD_NUM);
		first = false;
	}
	
	tr = rc = ret = 0;
#if((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_CTRL))
#ifdef INCLUDE_EXT_DISP
    tr = GetExtMmiYx(EXTMMI_YKF)|GetDiValue(pCfg->kg_openno);  
	rc = GetExtMmiYx(EXTMMI_YKH)|GetDiValue(pCfg->kg_closeno);
#endif
#endif
#if (DEV_SP == DEV_SP_WDG)
#ifdef INCLUDE_PR_SYHW
    tr = GetExtMmiYx(12)|GetDiValue(pCfg->kg_openno);  
    rc = GetExtMmiYx(13)|GetDiValue(pCfg->kg_closeno);
#else
	if(g_wdg_ver) //新罩式
	{

		tr = GetExtMmiYx(XINEXTMMI_SF)|GetDiValue(pCfg->kg_openno);
		rc = GetExtMmiYx(XINEXTMMI_SH)|GetDiValue(pCfg->kg_closeno);
	}
	else
	{
		tr = GetDiValue(3)|GetDiValue(pCfg->kg_openno);
		rc = GetDiValue(4)|GetDiValue(pCfg->kg_closeno);
	}
#endif
#endif
#if (DEV_SP == DEV_SP_DTU)
    tr = GetDiValue(pCfg->kg_openno);
	rc = GetDiValue(pCfg->kg_closeno);
#endif
    if (!old_tr[fd] && tr)
	   ret |= 0x01;
    else
       ret &= ~0x01;
    if (!old_rc[fd] && rc)
	   ret |= 0x02;
    else
       ret &= ~0x02;

    old_tr[fd] = tr;
    old_rc[fd] = rc;

	return ret;
}

void ykstateInit(void)
{
#ifdef INCLUDE_YC
    int i;
    VFdCfg *pCfg;
	
	for (i=0; i<fdNum; i++)
	{
		pCfg = pFdCfg+i;
		if(pCfg->kg_remoteno >-1)
		{
			RemoteYxState = 1;
			break;
		}
	}
#endif
}

#ifdef INCLUDE_YK_KEEP

void ykkeepInit(void)
{
	//int ret;
    int i,j;
    VFdCfg *pCfg;
	g_ykkeep= (YKKeep*)malloc(sizeof(YKKeep));
	pDoCfg = g_Sys.MyCfg.pDo;
	g_ykkeep->KeepFlag = 0;
	g_ykkeep->DelayFlag = 0;
	g_ykkeep->YKDelayTime = g_Sys.MyCfg.SysExt.YKDelayTime*1000;
	for(j=0;j<PBYXNUM;j++)
	{
		g_ykkeep->YXNO[j] = g_Sys.MyCfg.SysExt.YXNO[j];		
	}
	for (i=0; i<fdNum; i++)
	{
		pCfg = pFdCfg+i;
		if(i==0)
		{
			if(pCfg->kg_ykkeepno == -1)
			{
				g_ykkeep->fzTime = 2000;
			}
			else
			{
				g_ykkeep->fzTime = pDoCfg[pCfg->kg_ykkeepno-1].fzontime;
			}
			g_ykkeep->YKID = pCfg->kg_ykkeepno;
		}
		if(pCfg->kg_ykkeepno == -1)
		{
			g_ykkeep->EnableFlag[i]=0;
		}
		else
		{
			g_ykkeep->EnableFlag[i]=1;
		}
	}
#if 0
	if(g_ykkeep->YKID !=-1)
	{
		ret = prOutput(g_ykkeep->YKID, 0 |PRYK);
		if (ret)
		{
		  ykCancel(-1,g_ykkeep->YKID, 1);  
		  ykCancel(-1,g_ykkeep->YKID, 0); 
		  ret = prOutput(g_ykkeep->YKID, 0 |PRYK);
		  if (ret)
			 WriteDoEvent(NULL, 1, "遥控%d初始化执行合失败", g_ykkeep->YKID);
		}
	}
#endif
}

int GetLocalYkKeepInput(VFdCfg *pCfg,int fd)
{
    static BOOL first = true;
	int kgstate1,kgstate2,ret;	
	kgstate1 = kgstate2 = ret = 0;
	if(g_ykkeep->EnableFlag[fd] == 0)
	{
		return ret;	
	}
	if(first)
	{
		old_kgstate1 = calloc(MAX_FD_NUM,sizeof(int));
		old_kgstate2 = calloc(MAX_FD_NUM,sizeof(int));
		memset(old_kgstate1,0,sizeof(int)*MAX_FD_NUM);
		memset(old_kgstate2,0,sizeof(int)*MAX_FD_NUM);
		first = false;
	}
	
#if (DEV_SP == DEV_SP_DTU)
    kgstate1 = GetDiValue(pCfg->kg_stateno);
	kgstate2 = GetDiValue(1+pCfg->kg_stateno);

#endif
    if ((old_kgstate1[fd] ^ kgstate1)||(old_kgstate2[fd] ^ kgstate2))
	   ret |= 0x01;
    else
       ret &= ~0x01;
    
    old_kgstate1[fd] = kgstate1;
	old_kgstate2[fd] = kgstate2;
	return ret;
}
int GetLocalYkKeepPBInput(int start,int end)
{
    static BOOL first = true;
	int kgstate[PBYXNUM];
	BYTE YxValue;
	int i,ret;	
	ret = 0;
	if(first)
	{
		old_pbkgstate = calloc(PBYXNUM,sizeof(int));
		memset(old_pbkgstate,0,sizeof(int)*PBYXNUM);
		first = false;
	}
	
#if (DEV_SP == DEV_SP_DTU)
	for(i=start;i<end;i++)
	{
		kgstate[i]=0;
		if(g_ykkeep->YXNO[i] !=-1)
		{
			ReadRangeSYX(g_Sys.wIOEqpID, g_ykkeep->YXNO[i], 1, 1, &YxValue);
			kgstate[i] = YxValue>>7;
		}
	}
	for(i=start;i<end;i++)
	{
		if(g_ykkeep->YXNO[i] !=-1)
		{
			if (old_pbkgstate[i] ^ kgstate[i])
			{
				ret |= 0x01;
				break;
			}
			else
			   ret &= ~0x01;
		}
	}
	for(i=start;i<end;i++)
	{
		old_pbkgstate[i] = kgstate[i];
	}
#endif
	return ret;
}

void YkKeepWrite(void)
{
#ifdef INCLUDE_YK

	int operate1,operate2;
    int i,ret;
    VFdCfg *pCfg;
	operate1=operate2=0;
	ret = 0;


	if((g_ykkeep->YKID != -1)&&(g_ykkeep->KeepFlag == 1))
	{
		g_ykkeep->fzTime-=10;
		if(g_ykkeep->fzTime<0)
		{
			g_ykkeep->KeepFlag = 0;
		  	ykCancel(-1,g_ykkeep->YKID, 0); 
			//prOutput(g_ykkeep->YKID, 0 |PRYK);
		}
	}
	
	for (i=0; i<fdNum; i++)
	{
		pCfg = pFdCfg+i;
		if(GetLocalYkKeepInput(pCfg,i))
			operate1=1;
	} 
	operate1 = operate1|GetLocalYkKeepPBInput(2,PBYXNUM);
	
	operate2=GetLocalYkKeepPBInput(0,2);

	if((operate2 & 0x01)&&(g_ykkeep->YKID != -1))
	{
		ret = prOutput(g_ykkeep->YKID, 0 |PRYK);
		//ret = ykCancel(-1,g_ykkeep->YKID, 0);
		if(ret)
		{
			WriteDoEvent(NULL, 1, "遥控%d执行合失败", g_ykkeep->YKID);
		} 
		else
		{
			 g_ykkeep->KeepFlag = 1;
			 g_ykkeep->fzTime = pDoCfg[g_ykkeep->YKID-1].fzontime;
			 WriteDoEvent(NULL, 1, "遥控%d执行合成功", g_ykkeep->YKID);
		}
	}
	else
	{
		if(g_ykkeep->DelayFlag == 0)
		{
			if ((operate1!=0)||(operate2!=0))
			{
				if ((operate1 & 0x01)||(operate2 & 0x01))
				{
				   g_ykkeep->DelayFlag = 1;
				}
			}
		}
		else if(g_ykkeep->KeepFlag == 0)
		{
			g_ykkeep->YKDelayTime-=10;
			if((g_ykkeep->YKDelayTime<0)&&(g_ykkeep->YKID != -1))
			{
				 g_ykkeep->DelayFlag = 0;
				 g_ykkeep->YKDelayTime = g_Sys.MyCfg.SysExt.YKDelayTime*1000;
				 ret = prOutput(g_ykkeep->YKID, 0 |PRYK);
				 //ret = ykCancel(-1,g_ykkeep->YKID, 0);
				 if(ret)
				 {
					WriteDoEvent(NULL, 1, "遥控%d执行合失败", g_ykkeep->YKID);
				 } 
				 else
				 {
					 g_ykkeep->KeepFlag = 1;
					 g_ykkeep->fzTime = pDoCfg[g_ykkeep->YKID-1].fzontime;
					 WriteDoEvent(NULL, 1, "遥控%d执行合成功", g_ykkeep->YKID);
				 }
			}
		}
	}
	
   
#endif
}
#endif

#ifdef INCLUDE_YK_CZDY
void czdyykInit(void) //操作电源控制初始化
{
	CzdyYkPara* pPCzdyYk = (CzdyYkPara*)(g_pParafile+1);

	pCzdyYk = (CzdyYk*)malloc(sizeof(CzdyYk));

	if(pCzdyYk == NULL)
		return;
	if (ReadParaFile("czdyyk.cfg", (BYTE *)g_pParafile, MAXFILELEN) == ERROR)
	{
		return;
    }
	
	pCzdyYk->YkId = pPCzdyYk->YkId;
	pCzdyYk->BcTime = pPCzdyYk->BcTime;
	
	pCzdyYk->dwCnt = 0;
	pCzdyYk->StartFlag = 0;
	czdyykinit = 1;
}

void czdyykon(void)  // 执行合
{
	if(!czdyykinit)
		return;
	prOutput(pCzdyYk->YkId, 1 |PRYK);
	WriteDoEvent(NULL, 1, "预置操作电源%d合", pCzdyYk->YkId);
	pCzdyYk->dwCnt = 0;
	pCzdyYk->StartFlag = 1;
}

void czdyykscan(void)  // 执行分计时
{
	int ret;
	if(!czdyykinit)
		return;
	if(pCzdyYk->StartFlag == 1)
	{
		pCzdyYk->dwCnt += 1000;
		if(pCzdyYk->dwCnt >  pCzdyYk->BcTime)
		{
			ret = ykCancel(-1,pCzdyYk->YkId, 1);
			if(ret)
			{
				WriteDoEvent(NULL, 1, "操作电源%d复归分失败，再执行一次", pCzdyYk->YkId);
				ret = ykCancel(-1,pCzdyYk->YkId, 0);
			}	
			else
				WriteDoEvent(NULL, 1, "操作电源%d复归分", pCzdyYk->YkId);
			
			pCzdyYk->StartFlag = 0;
			pCzdyYk->dwCnt = 0;
		}
	}
}

#endif

#ifdef INCLUDE_EXT_YK
void extYKWrite(void)
{
	VFdCfg *pCfg;
	BOOL ok = true, ykon=false;
	static BOOL first=true;
	static int bst=0;
	int i,operate,ret;
	
	if (first)
	{
		first = false;
		g_kgunlock = calloc(fdNum,sizeof(VKgUnlock));
		memset(g_kgunlock, 0, sizeof(VKgUnlock)*fdNum);
	}
	for (i=0; i<fdNum; i++)
	{
		pCfg = pFdCfg+i;
#if (DEV_SP != DEV_SP_FTU)
		ok &= (pCfg->kg_startno != -1);
		ok &= (pCfg->kg_unlockno != -1);
		ok &= (pCfg->kg_openno != -1);
		ok &= (pCfg->kg_closeno != -1);
		ok &= (pCfg->kg_startno !=pCfg->kg_unlockno);
		ok &= (pCfg->kg_startno !=pCfg->kg_openno);
		ok &= (pCfg->kg_startno !=pCfg->kg_closeno);
		ok &= (pCfg->kg_unlockno!=pCfg->kg_openno);
		ok &= (pCfg->kg_unlockno!=pCfg->kg_closeno);
		ok &= (pCfg->kg_openno !=pCfg->kg_closeno);
		if (!ok)
		   continue;		
#endif	
#if ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_CTRL))
		if(
#ifdef INCLUDE_EXT_DISP
			(GetExtMmiYx(9)) && 
#endif
		bst==0)
#else
		if((GetDiValue(pCfg->kg_startno)) && bst==0)
#endif
		{
			g_kgunlock[i].ykstart = true; 
			g_kgunlock[i].ykcount = 0;
		}

#if ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_CTRL))
		if(g_kgunlock[i].ykstart 
#ifdef INCLUDE_EXT_DISP
			&& (GetExtMmiYx(10))
#endif
		&& bst==0)
#else
		if(g_kgunlock[i].ykstart && (GetDiValue(pCfg->kg_unlockno)) && bst==0)
#endif
		{
			g_kgunlock[i].ykstart = false; 
			g_kgunlock[i].ykUnlock = true; 
		}

#if ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_CTRL))
        if(
#ifdef INCLUDE_EXT_DISP
			  GetExtMmiYx(12) && 
#endif
				bst==0)
#else
		if((GetDiValue(pCfg->kg_openno)) && bst==0)
#endif
		{
			ykon = 1; 
			operate = 1;
		}
#if ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_CTRL))
		if(
#ifdef INCLUDE_EXT_DISP
			(GetExtMmiYx(11)) && 
#endif
		bst==0 && !GetFaultYx(i))//无故障信号
#else
		if((GetDiValue(pCfg->kg_closeno)) && bst==0 && !GetFaultYx(i))//无故障信号
#endif
		{
			ykon = 1; 
			operate = 2;
		}

		//遥控执行后3秒内闭锁操作
		if(bst > 10)
			bst -= 10;
		else
		{
			bst = 0;
			pCfg->yk_close_dis = 0;
			pCfg->yk_trip_dis = 0;
		}
		if(g_kgunlock[i].ykstart && g_kgunlock[i].ykUnlock && ykon)
		{
			ret = 0;
			if (operate & 0x01)
			{
				ret = ykOutput(pCfg->kg_ykno, 0);
				if(ret)
					WriteDoEvent(NULL, 1, "外部遥控器%d执行分失败", pCfg->kg_ykno);
				else
					WriteDoEvent(NULL, 1, "外部遥控器%d执行分成功", pCfg->kg_ykno);
				pCfg->yk_close_dis = 1;
			}
			if(operate & 0x02)
			{
				ret = ykOutput(pCfg->kg_ykno, 1);
				if(ret)
					WriteDoEvent(NULL, 1, "外部遥控器%d执行合失败", pCfg->kg_ykno);
				else
					WriteDoEvent(NULL, 1, "外部遥控器%d执行合成功", pCfg->kg_ykno);
				pCfg->yk_trip_dis = 1;
			}
			bst = 3000;
			g_kgunlock[i].ykstart = false;
			g_kgunlock[i].ykUnlock = false;
			
		}
	}
	for(i=0; i < fdNum; i++)
	{
		if(g_kgunlock[i].ykstart)
		{
			g_kgunlock[i].ykcount += 10;
			if(g_kgunlock[i].ykcount >= 5000)
			{
				g_kgunlock[i].ykcount = 0;
				g_kgunlock[i].ykstart = false;
				g_kgunlock[i].ykUnlock = false;
			}
		}
	}
}
#endif


void LocalYkWrite(void)
{
#ifdef INCLUDE_YC
	int operate,i,ret;
	VFdCfg *pCfg;
#endif
	
#ifdef _GUANGZHOU_TEST_VER_
    if (g_Sys.dwErrCode)
		 return;
#endif

    if (!GetLocalCtrl())
		return;

#ifdef INCLUDE_YC

    for (i=0; i<fdNum; i++)
	{
	    pCfg = pFdCfg+i;
		operate=GetLocalYkInput(pCfg,i);
	    if (operate)
     	{
	        ret = 0;
            if (operate & 0x01)
		    {
		       ret = ykOutput(pCfg->kg_ykno, 0 |PRYK);
			   if(ret)
			      WriteDoEvent(NULL, 1, "本地遥控%d执行分失败", pCfg->kg_ykno);
			   else
			   {
#ifdef INCLUDE_PR
				  ykTypeSave(0, 1, 0);
#endif
	              WriteDoEvent(NULL, 1, "本地遥控%d执行分成功", pCfg->kg_ykno);
			   }
	        }
		    if(operate & 0x02)
		    {
			   ret = ykOutput(pCfg->kg_ykno, 1 |PRYK);
			   if(ret)
			      WriteDoEvent(NULL, 1, "本地遥控%d执行合失败", pCfg->kg_ykno);
			   else
			   {
#ifdef INCLUDE_PR
				  ykTypeSave(0, 1, 1);
#endif
		          WriteDoEvent(NULL, 1, "本地遥控%d执行合成功", pCfg->kg_ykno);
			   }
		    }
      
        }
	}
#endif
}

/*------------------------------------------------------------------------
 Procedure:     writeYK ID:1
 Purpose:       相应请求者操作请求, 执行遥控操作
 Input:         
 Output:
 Errors:
------------------------------------------------------------------------*/
void ykWrite(struct VMsg *pmsg)
{
    WORD id;
	BYTE msgId, value; 
	struct VDBYK *pyk;
	int result;
	int thId;

	msgId = pmsg->Head.byMsgID;
	thId = pmsg->Head.byRsv[0];
  	pyk = (struct VDBYK *)pmsg->abyData;

#ifdef _GUANGZHOU_TEST_VER_
    if (g_Sys.dwErrCode)
	{
	    pyk->byStatus = 4;
		goto Err;
    }
#endif

#ifdef INCLUDE_YC
	if((!RemoteYxState) || (pyk->wID > fdNum))
	{
#else
	if(pyk->wID > 0)
	{
#endif
		if (GetBsCtrl())
		{
		   pyk->byStatus = 4;
		   goto Err;
		}
		if (GetLocalCtrl()&&(thId != MMI_ID)&&strcmp(g_Task[thId].CommCfg.Port.pcol,"MMI"))
		{
			pyk->byStatus = 4;
			goto Err;
		}
/*#ifndef MMI_REMOTE_YK
		if (GetRemoteCtrl()&&(thId == MMI_ID))
		{
			pyk->byStatus = 4;
			goto Err;
		}
#endif*/
	}
	else
	{
#ifdef INCLUDE_YC
		if(pFdCfg[pyk->wID-1].kg_remoteno > -1)
		{
			if(!GetDiValue(pFdCfg[pyk->wID-1].kg_remoteno))
			{
				pyk->byStatus = 4;
				goto Err;
			}

		}
		else
		{
			pyk->byStatus = 4;
			goto Err;

		}
#endif
	}
    

	if (thId == MMI_ID)
		thId = -1;

	if      ((pyk->byValue&0x3) == YK_CLOSE) value = 1;
    else if ((pyk->byValue&0x3) == YK_OPEN ) value = 0;
    else 
	{
		pyk->byStatus = 4;
		goto Err;
    }	

#if (TYPE_IO == IO_TYPE_MASTER)	
    if (GetMyIoNo(2, pyk->wID, &id) == ERROR)
    {		
		msgSend(BUS_CAN_ID, (void *)pmsg, pmsg->Head.wLength, TRUE);
		return;
	}
#else
    id = pyk->wID;
#endif

  	switch (msgId)
	{
	    case MI_YKSELECT:
#ifdef INCLUDE_YK_CZDY
			if(id < myCellId)
			{
				czdyykon();
			}
#endif
	    	result = ykSelect(thId, id, value);
	   		break;
	    case MI_YKOPRATE:
			#ifdef INCLUDE_PR_PRO
            if (prCheckShTq(id, value))
                result = 5;
			else
            #endif
	    	result = ykOperate(thId,id, value, 0);	
#ifdef INCLUDE_PR
			ykTypeSave(1, id , value);
#endif
	   		break;
	    case MI_YKCANCEL:
	    	result = ykCancel(thId,id, value);
	   		break;
	    default:
	    	result = 5;
	    	break;
  	}
    pyk->byStatus = (BYTE)result;	

Err:	
	TaskSendMsg(pmsg->Head.wThID, SELF_DIO_ID, pmsg->Head.wEqpID, msgId, MA_RET, sizeof(struct VDBYK), pmsg);
}

/*------------------------------------------------------------------------
 Procedure:     writeYT ID:1
 Purpose:       相应请求者操作请求, 执行遥调操作
 Input:         
 Output:
 Errors:
------------------------------------------------------------------------*/
void ytWrite(struct VMsg *pmsg)
{
	WORD id;
	BYTE msgId; 
	struct VDBYT *pyt;
	//BYTE value;
	int result = 5;
	int thId;


  	msgId=pmsg->Head.byMsgID;
	thId = pmsg->Head.byRsv[0];
  	pyt=(struct VDBYT *)pmsg->abyData;

	if ((GetLocalCtrl()) && (thId != MMI_ID))
	{
		pyt->byStatus = 4;
		goto Err;
	}

	if (thId == MMI_ID)
		thId = -1;

#if (TYPE_IO == IO_TYPE_MASTER)	
    if (GetMyIoNo(3, pyt->wID, &id) == ERROR)
    {		
		msgSend(BUS_CAN_ID, (void *)pmsg, pmsg->Head.wLength, TRUE);
		return;
	}
#else
	id = pyt->wID+PRPARA_ADDR-1;
#endif
#if 0
	/*点号转为路数, 跟yk.c中一致, 先分后合, 奇分偶合*/
	if (id&0x01) 
		value = 0;
	else 
		value = 1;

	id = (id+1)/2;
#endif	

  	switch (msgId)
	{
	    case MI_YTSELECT:
#ifdef INCLUDE_PR
	    	result = ReadPrPara(id , (char *)&m_ParaInfo,1);
#endif
			if(result == 0)
				ParaYzStatue = 1;
	   		break;
	    case MI_YTOPRATE:
			if(ParaYzStatue == 1)
			{
				memcpy(&m_ParaInfo.valuech,(char *)&pyt->dwValue,m_ParaInfo.len);
#ifdef INCLUDE_PR
				result = WritePrPara(id, (char *)&m_ParaInfo, 1);
#endif
				ParaYzStatue = 0;
				if((id >= PRPARA_ADDR) && (id < PRPARA_LINE_ADDREND))
				{
#ifdef INCLUDE_PR
					if(result == 0)
						WritePrParaFile();
#endif
				}
			}
	   		break;
	    case MI_YTCANCEL:
			if(ParaYzStatue == 1)
			{
				result = 0;
				ParaYzStatue = 0;
			}
	   		break;
	    default:
	    	break;
  	}

    pyt->byStatus = (BYTE)result;	

Err:		
	TaskSendMsg(pmsg->Head.wThID, SELF_DIO_ID, pmsg->Head.wEqpID, msgId, MA_RET, sizeof(struct VDBYT), pmsg);	
}

void dioProcMsg(void)
{
    int msgId;
	
	if (dioMsg.Head.byMsgAttr != MA_REQ) return;

	msgId = dioMsg.Head.byMsgID;

	if (msgId==MI_YTSELECT || msgId==MI_YTOPRATE || msgId==MI_YTCANCEL)
	  	ytWrite(&dioMsg);
	else 
	  	ykWrite(&dioMsg);		
}

void miscInit(void)
{
#ifdef INCLUDE_CELL
    cellInit();
#endif

#ifdef INCLUDE_USWITCH
	uswitchInit();
#endif

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
	
}

void ykCheckInit(void)
{
    int i;
	VFdCfg *pfdcfg;
    g_KgStatus = calloc(g_Sys.MyCfg.wFDNum, sizeof(VKgStatus));

	pfdcfg = g_Sys.MyCfg.pFd;

	for (i=0; i<g_Sys.MyCfg.wFDNum; i++)
	{
	    g_KgStatus[i].kgNo = pfdcfg->kg_ykno;
		g_KgStatus[i].yxNo = pfdcfg->kg_stateno;
		g_KgStatus[i].kgJudge = 0;
		g_KgStatus[i].optVal = 0;
		g_KgStatus[i].dwCnt = 0;

		pfdcfg++;
	}
	
}

void ykStsSet(int id, int optVal)
{
    if (id >= g_Sys.MyCfg.wFDNum) return;

	g_KgStatus[id].optVal = optVal;

#ifdef INCLUDE_YX
	if (((GetMyDiValue(g_KgStatus[id].yxNo)) != optVal)&&(g_Sys.MyCfg.SysExt.dwKgTime > 0))
	{
	   g_KgStatus[id].kgJudge = 1;
	   g_KgStatus[id].dwTimes = g_Sys.MyCfg.SysExt.dwKgTime;
	   g_KgStatus[id].dwCnt = 0;
	   g_KgStatus[id].optVal = optVal;
	}
#endif
}

void ykCheck(void)
{
     int i;
	 BOOL kgerr=FALSE;
	 struct VDBSOE DBSOE;
	 struct VDBCOS DBCOS;

	 if (g_KgErr) return;
	 
#ifdef INCLUDE_YX
	 for (i=0; i<g_Sys.MyCfg.wFDNum; i++)
	 {
	     if (g_KgStatus[i].kgJudge)
	     {
	        if ((GetMyDiValue(g_KgStatus[i].yxNo)) == g_KgStatus[i].optVal)
			{
			    g_KgStatus[i].kgJudge = 0;
				g_KgStatus[i].dwCnt = 0;
				continue;
	        }
	        g_KgStatus[i].dwCnt += 1000;
			if (g_KgStatus[i].dwCnt >= g_KgStatus[i].dwTimes)
			{
			    kgerr = TRUE;
				break;
			}
	     }
	 }
#endif

 	 if (kgerr && !g_KgErr)
 	 {
 	     DBCOS.wNo = DBSOE.wNo = SYS_KGWARN_YXNO;
	     DBCOS.byValue = DBSOE.byValue = 0x81;
	     GetSysClock(&DBSOE.Time, CALCLOCK); 	
	     WriteSSOE(g_Sys.wIOEqpID, 1, &DBSOE); 
		 WriteSCOS(g_Sys.wIOEqpID, 1, &DBCOS);
		WriteUlogEvent(&DBSOE.Time, 9, 1,"Switch operation error!");
 	 }
	 
	 g_KgErr = kgerr;

}

void ykErrReset(void)
{
    int i;
    struct VDBSOE DBSOE;
	struct VDBCOS DBCOS;

    for (i=0; i<g_Sys.MyCfg.wFDNum; i++)
	{
		g_KgStatus[i].kgJudge = 0;
		g_KgStatus[i].optVal = 0;
		g_KgStatus[i].dwCnt = 0;
	}

	if (g_KgErr)
	{
	   DBCOS.wNo = DBSOE.wNo = SYS_KGWARN_YXNO;
	   DBCOS.byValue = DBSOE.byValue = 0x01;
	   GetSysClock(&DBSOE.Time, CALCLOCK); 	
	   WriteSSOE(g_Sys.wIOEqpID, 1, &DBSOE); 
       WriteSCOS(g_Sys.wIOEqpID, 1, &DBCOS);
	}

	g_KgErr = FALSE;

}

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

#ifdef  ECC_MODE_CHIP
//测试双向加密芯片是否焊好，若没有则告警两分钟。
void jiamitest()
{
  BYTE jiami = 0;
	static BYTE jiamicnt = 0;
#ifdef INCLUDE_SEC_CHIP
    if(g_ipsecinit == 1) //南网不处理
			return;
#endif
	if(jiamicnt < 3) //刚开始读三次判断是否焊接，以后就不读状态了
	{
		jiami = JiamiInit(1);
		if((!jiami) && ((g_Sys.dwErrCode & SYS_ERR_JIAMI) == 0))
		{
			WriteWarnEvent(NULL, SYS_ERR_JIAMI, 0, "加密芯片未焊或异常！");
			WriteUlogEvent(NULL, 04, 1,"Encryption chip error!");
		}
		if(jiami && (g_Sys.dwErrCode & SYS_ERR_JIAMI))
			SysClearErr(SYS_ERR_JIAMI);	
    jiamicnt++;		
	}
	else if(jiamicnt > 120)
	{
		if(g_Sys.dwErrCode & SYS_ERR_JIAMI)
			SysClearErr(SYS_ERR_JIAMI);
	}
	else
		jiamicnt++;
}
void jiamiinit()
{
#ifdef INCLUDE_SEC_CHIP
    if(g_ipsecinit == 1) //南网不处理
			return;
#endif
	if(!JiamiInit(1))
	{
		WriteWarnEvent(NULL, SYS_ERR_JIAMI, 0, "加密芯片未焊或异常！");
		WriteUlogEvent(NULL, 04, 1,"Encryption chip error!");
	}
}
#endif
#ifdef INCLUDE_TMP75_TEMP
void tmptest()

{
	BYTE tmp75 = 0;
	static BYTE tmpcnt = 0;
	if(tmpcnt < 3) //刚开始读三次判断是否焊接，以后就不读状态了
	{
		tmp75 = Tmp75Init(1);
		if((!tmp75) && ((g_Sys.dwErrCode & SYS_ERR_TMP) == 0))
		{
			WriteWarnEvent(NULL, SYS_ERR_TMP, 0, "温度芯片未焊或异常!");
			WriteUlogEvent(NULL, 04, 1,"Temperature chip error!");
		}
		if(tmp75 && (g_Sys.dwErrCode & SYS_ERR_TMP))
			SysClearErr(SYS_ERR_TMP);	
			tmpcnt++;		
	}
	else if(tmpcnt > 120)
		{
			if(g_Sys.dwErrCode & SYS_ERR_TMP)
			SysClearErr(SYS_ERR_TMP);
		}
			else
			tmpcnt++;
}
void tmp75init()
{
	if(!Tmp75Init(1))
	{
		WriteWarnEvent(NULL, SYS_ERR_TMP, 0, "温度芯片未焊或异常");
		WriteUlogEvent(NULL, 04, 1,"Temperature chip error!");
	}

}
#endif

void ioInit(void)
{
#ifdef IRIGB_FLAG
	IrigbInit();//B码对时io口初始化
#endif

#ifdef INCLUDE_YX
	yxInit();
#endif

#ifdef INCLUDE_YK
	ykInit();
#endif

#ifdef INCLUDE_YC
	ycInit();
#endif

#ifdef INCLUDE_EXT_YC
	extYcInit();
#endif

#ifdef  ECC_MODE_CHIP
	jiamiinit();
#endif

	miscInit();

	ykCheckInit();
	
	ykstateInit();

#ifdef INCLUDE_YK_KEEP
	ykkeepInit();
#endif
#ifdef INCLUDE_YK_CZDY
	czdyykInit();
#endif

#ifdef INCLUDE_TMP75_TEMP
	tmp75init();
#endif

}

//取共享内存的IO板卡，防止linux与裸核同时访问。
void ioWarnTest(void)
{
    int i;
	BYTE type,type1,type2;
	DWORD dwAIType,dwIOType;
	BOOL ac_err, io_err;
	static BYTE ai_cnt[BSP_AC_BOARD],di_cnt[BSP_IO_BOARD];
#ifdef INCLUDE_FLAG_CUR
	int index=0;
	static BOOL ai_err[BSP_AC_BOARD],di_err[BSP_IO_BOARD];
#endif
	
    if (g_Sys.MyCfg.SYXIoNo[1].wNum == 0) return;

	if(ReadMyioType(&dwAIType ,&dwIOType) == ERROR)
		return;
	
	ac_err = io_err = false;
    for (i=0; i<BSP_AC_BOARD; i++)
	{
	    type = (g_Sys.dwAioType>>(i*8))&0xFF;
		type1 = (dwAIType>>(i*8))&0xFF;
		
		if(!type1)
			continue;
		
	    if(type&&(type1 != type))
	    {
		   ac_err = true;
		   ai_cnt[i]++;
		   if (ai_cnt[i] < 3) continue;
	       if ((g_Sys.dwErrCode&SYS_ERR_AIO) == 0)
	       {
		   	  SysSetErr(SYS_ERR_AIO);
			  WriteUlogEvent(NULL, 04, 1,"AI error!");
	       }
#ifdef INCLUDE_FLAG_CUR
           if ((g_Sys.MyCfg.byYcNum[i] > 0)&&(!ai_err[i]))
		       SetRangeYCCfg(g_Sys.wIOEqpID, index, g_Sys.MyCfg.byYcNum[i], 0x40000000);
		   ai_err[i] = true;
#endif
	    }
#ifdef INCLUDE_FLAG_CUR
		else 
		{
		    ai_cnt[i] = 0;
			if (ai_err[i])
		    {
		       ai_err[i] = false;
		       if (g_Sys.MyCfg.byYcNum[i] > 0)
		          ClearRangeYCCfg(g_Sys.wIOEqpID, index, g_Sys.MyCfg.byYcNum[i], 0x40000000);
			}
		}
		index += g_Sys.MyCfg.byYcNum[i];
#else
		else
		{
			ai_cnt[i] = 0;
		}
#endif
    }

#ifdef INCLUDE_FLAG_CUR		
	index = 0;
#endif
	for (i=0; i<BSP_IO_BOARD; i++)
	{
        type1 = (g_Sys.dwDioType>>(i*4))&0x0F;
	    type2 = (dwIOType>>(i*4))&0x0F;
		
		if(!type2)
			continue;
		
	    if(type1 != type2)
	    {
		    io_err = true;
			di_cnt[i]++;
			if (di_cnt[i] < 3) continue;
	       if ((g_Sys.dwErrCode&SYS_ERR_DIO) == 0)
	       {
		   	  SysSetErr(SYS_ERR_DIO);
			  WriteUlogEvent(NULL, 04, 1,"DIO error!");
	       }
#ifdef INCLUDE_FLAG_CUR
           if ((g_Sys.MyCfg.byYxNum[i] > 0)&&!di_err[i])
		       SetRangeSYXCfg(g_Sys.wIOEqpID, index, g_Sys.MyCfg.byYxNum[i], 0x40000000);
		   di_err[i] = true;
#endif
	    }
#ifdef INCLUDE_FLAG_CUR
        else 
        {
           di_cnt[i] = 0;
		   if (di_err[i])
           {
              di_err[i] = false;
              if (g_Sys.MyCfg.byYxNum[i] > 0)
		          ClearRangeSYXCfg(g_Sys.wIOEqpID, index, g_Sys.MyCfg.byYxNum[i], 0x40000000);
		   }
        }
        index += g_Sys.MyCfg.byYxNum[i];
#else
			  else
				{
					di_cnt[i] = 0;
				}
#endif

    }

	if ((g_Sys.dwErrCode&SYS_ERR_AIO)&&!ac_err)
	{
		SysClearErr(SYS_ERR_AIO);
		WriteUlogEvent(NULL, 04, 0,"AI resumed!");
	}

	if ((g_Sys.dwErrCode&SYS_ERR_DIO)&&!io_err)
	{
		SysClearErr(SYS_ERR_DIO);
		WriteUlogEvent(NULL, 04, 0,"DIO resumed!");
	}

}

extern int WAV_Record_Scan(void);
void dio(void)
{
    DWORD events;
	int msg_len;

    tmEvEvery(SELF_DIO_ID, 1, EV_TM1); 	  /*10ms*/

#ifdef INCLUDE_YX
	evSend(SELF_DIO_ID, EV_UFLAG);   /*防止遥信滤波中断中任务还未起来中断发送事件导致事件丢失*/
#endif	

	for (;;)
	{
	    evReceive(SELF_DIO_ID, EV_TM1|EV_UFLAG|EV_MSG|EV_DATAREFRESH, &events);

		//if (g_Sys.dwErrCode & SYS_ERR_DIO) continue;
		
#ifdef INCLUDE_YX
	    if (events & EV_UFLAG) 
		    yxWrite();
#endif

	    if (events & EV_TM1)
		{
			yxWrite();
#ifdef INCLUDE_EXT_YK
			extYKWrite();
#endif
            LocalYkWrite();

#ifdef INCLUDE_YK_KEEP
			YkKeepWrite();
#endif

#ifdef INCLUDE_RECORD	
			WAV_Record_Scan();
#endif
			WriteBmEvent();
		}	

	    if (events & EV_MSG) 
	  	{
			for (;;)
			{
				msg_len = msgReceive(SELF_DIO_ID, (BYTE *)&dioMsg, MAXMSGSIZE, OS_NOWAIT);	
				if (msg_len > 0)
					dioProcMsg();
				else
					break;
			}
	    }

		if (events & EV_DATAREFRESH)
		{
#ifdef INCLUDE_YX
			yxWrite();
#endif

#if (TYPE_IO == IO_TYPE_MASTER) 
			evSend(BUS_CAN_ID, EV_DATAREFRESH);
#endif			
		}
	}
}

void mea(void)
{
	DWORD evFlag;

#if (defined(INCLUDE_YC)||defined(INCLUDE_EXT_YC))
#ifdef INCLUDE_YC
    if (gycInit == 0) thSuspend(SELF_MEA_ID);
#endif

    tmEvEvery(SELF_MEA_ID, 40, EV_TM1);

	for (; ;)
	{		
		evReceive(SELF_MEA_ID, EV_TM1, &evFlag);

		if ((evFlag&EV_TM1)&&((g_Sys.dwErrCode & SYS_ERR_AIO) == 0))
		{
	#ifdef INCLUDE_YC
		    measure();
	#endif
	#ifdef INCLUDE_EXT_YC
	        extYcMea();
	#endif
		}
 	}
#endif	
}

void misc(void)
{
	DWORD evFlag;

	tmEvEvery(MISC_ID, SECTOTICK(1), EV_TM1); 
	
	for(;;)
	{
		evReceive(MISC_ID, EV_TM1, &evFlag);

		if (evFlag&EV_TM1)
		{ 	
#ifdef INCLUDE_CELL
			cellCtrl(); 
#endif
#ifdef INCLUDE_USWITCH
			uswitchCtrl();
#endif
			ioWarnTest();

			ykCheck();
			
#ifdef  ECC_MODE_CHIP
			jiamitest();
#endif
#ifdef INCLUDE_YK_CZDY
			czdyykscan();
#endif
#ifdef INCLUDE_TMP75_TEMP
			tmptest();
#endif
			WriteEQPFromBM();
		}
	}  
}

int GetDiValue(int no)
{
#if (TYPE_IO == IO_TYPE_MASTER)
 	WORD iono;
	struct VExtIoConfig *pCfg;
#endif

    if (no < 0) return 0;

#if (TYPE_IO == IO_TYPE_MASTER)	
	if (GetMyIoNo(1, no, &iono) == OK)
	{
#ifdef INCLUDE_YX		
		return(GetMyDiValue(iono));
#else
		return 0;
#endif
	}
	else
	{
		pCfg = GetExtIoNo(1, no, &iono);
		if (pCfg == NULL)
			return 0;
		else
			return(GetExtDiValue(pCfg, iono));
	}
#else
#ifdef INCLUDE_YX		
	return(GetMyDiValue(no));
#else
	return 0;
#endif
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



void ResetProtect(int fd)
{
	ykErrReset();
	BMResetProtect(fd); 
}


