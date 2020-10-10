/*------------------------------------------------------------------------
 $Rev: 12 $
------------------------------------------------------------------------*/
#include "syscfg.h"

#ifdef INCLUDE_PMASTER

#include "os.h"
#include "pmaster.h"

/***************************************************************
	Function:CPPrimary
		规约基类构造函数
	参数：无
	返回：无
***************************************************************/
CPPrimary::CPPrimary():CProtocol()
{
    m_byProtocolType=PRIMARY;	
	memset(&AllData,0,sizeof(AllData));
	memset(&Clock,0,sizeof(Clock));
	memset(&DD,0,sizeof(DD));	
}


BOOL CPPrimary::Init(WORD wTaskID, WORD wMinEqpNum,void *pCfg,WORD wCfgLen)
{		
	if (CProtocol::Init(wTaskID,wMinEqpNum,pCfg,wCfgLen)==FALSE)  return(FALSE);

	CheckMasterBaseCfg();
	SetDefFlag();

	m_wCommCtrl = COMM_IDLE_ON;

	commCtrl(m_wCommID,CCC_EVENT_CTRL|m_wCommCtrl,(BYTE*)&m_dwCommCtrlTimer);
	return(TRUE);
}

void CPPrimary::CheckMasterBaseCfg(void)
{//后面主protocol的cfg在此校
	if(m_pBaseCfg->wCtrlResultReqCount<=0)
	{
		m_pBaseCfg->wCtrlResultReqCount=5;
	}
}

void CPPrimary::SetDefFlag(void)
{
	if (m_pBaseCfg->wCmdControl&ALLDATAPROC)  
	{
	  SetAllEqpFlag(EQPFLAG_YC);
	  SetAllEqpFlag(EQPFLAG_YX);	  
	}
	if (m_pBaseCfg->wCmdControl&DDPROC)  
	{
	  SetAllEqpFlag(EQPFLAG_DD);	  
	}  
	if (m_pBaseCfg->wCmdControl&CLOCKENABLE)
	{
		if (m_pBaseCfg->wCmdControl&BROADCASTTIME) SetTaskFlag(TASKFLAG_CLOCK);
		else SetAllEqpFlag(EQPFLAG_CLOCK);
	}

	SetAllEqpFlag(EQPFLAG_POLL);

	//SetAllEqpFlag(EQPFLAG_VBIT);
}

/***************************************************************
	Function:InitEqpInfo
		初始化装置信息
	参数：无
	
	返回：TRUE 成功，FALSE 失败 
***************************************************************/
BOOL CPPrimary::InitEqpInfo(WORD wMinEqpNum)
{
	WORD wEqpID;
	VTVEqpInfo EqpInfo;
	int EqpNo;

	m_wEqpNum = GetTaskEqpNum(m_wTaskID);

	if (m_wEqpNum<wMinEqpNum)		
	{
		myprintf(m_wTaskID, LOG_ATTR_WARN, "Eqpnum < MinEqpum, protcol init error");
		return FALSE;
	}

	if (m_wEqpNum==0)  return(TRUE);
	
	m_pEqpInfo = (VPtEqpInfo*)calloc(m_wEqpNum, sizeof(VPtEqpInfo));
	if (!m_pEqpInfo)
		return FALSE;
	for (EqpNo=0; EqpNo<m_wEqpNum; EqpNo++)
	   memset(m_pEqpInfo+EqpNo,0,sizeof(VPtEqpInfo));	
	m_pEqpExtInfo= (VPriEqpExtInfo*)calloc(m_wEqpNum, sizeof(VPriEqpExtInfo));
	if (!m_pEqpExtInfo)
		return FALSE;
	for (EqpNo=0; EqpNo<m_wEqpNum; EqpNo++)
	   memset(m_pEqpExtInfo+EqpNo,0,sizeof(VPriEqpExtInfo));	

	m_pwEqpID = (WORD*)calloc(m_wEqpNum, sizeof(WORD));
	if (!m_pwEqpID)
		return FALSE;
	GetTaskEqpID(m_wTaskID, m_pwEqpID);
	m_wEqpID = m_pwEqpID[0];

	for (EqpNo = 0; EqpNo < m_wEqpNum; EqpNo++)
	{
		wEqpID = m_pwEqpID[EqpNo];
		if (ReadEqpInfo(m_wTaskID, wEqpID, &EqpInfo, FALSE) == ERROR)
		{
			myprintf(m_wTaskID,LOG_ATTR_INFO, "Primary Protcol Init EqpInfo Error,ReadEqpInfo Error.");
			return FALSE;
		}
		if(EqpInfo.wType != TRUEEQP)
		{
			myprintf(m_wTaskID,LOG_ATTR_INFO, "Primary Protcol Init EqpInfo Error,it is not a true Eqp.");
			return FALSE;
		}
		m_pEqpInfo[EqpNo].wEqpID = wEqpID;	
		memcpy(&m_pEqpInfo[EqpNo].sTelNo,&EqpInfo.sTelNo,20);
		m_pEqpInfo[EqpNo].dwFlag= EqpInfo.dwFlag;
		m_pEqpInfo[EqpNo].wAddress= EqpInfo.wSourceAddr;		 
		m_pEqpInfo[EqpNo].wDAddress = EqpInfo.wDesAddr; 		 
		m_pEqpInfo[EqpNo].wYCNum = EqpInfo.wYCNum;				 
		m_pEqpInfo[EqpNo].wVYCNum = EqpInfo.wVYCNum;				 
        m_pEqpInfo[EqpNo].wDYXNum = EqpInfo.wDYXNum;			 
		m_pEqpInfo[EqpNo].wSYXNum = EqpInfo.wSYXNum;			 
		m_pEqpInfo[EqpNo].wVYXNum = EqpInfo.wVYXNum;			 
		m_pEqpInfo[EqpNo].wDDNum = EqpInfo.wDDNum;				 
		m_pEqpInfo[EqpNo].wYKNum = EqpInfo.wYKNum;				 
		m_pEqpInfo[EqpNo].wTSDataNum = EqpInfo.wTSDataNum;		 
		m_pEqpInfo[EqpNo].wYTNum = EqpInfo.wYTNum;				 
		m_pEqpInfo[EqpNo].wTQNum = EqpInfo.wTQNum;				 

		m_pEqpInfo[EqpNo].pEqpRunInfo = EqpInfo.pEqpRunInfo;				
		memset(&m_pEqpInfo[EqpNo].Flags,0,sizeof(VFLAGS));				
		m_pEqpInfo[EqpNo].byEqpStatus = 0;		

		ClearSYX(wEqpID);
#ifdef INCLUDE_FLAG_CUR
        if  (wEqpID  != g_Sys.wIOEqpID)
        {
			SetRangeYCCfg(wEqpID, 0, -2, 0x40000000);
			SetRangeSYXCfg(wEqpID, 0, -2, 0x40000000);
        }	
#endif		
	}
	return TRUE;
}


/***************************************************************
	Function:GetEqpAddr
		获取当前装置地址，暂空
	参数：无
	
	返回：当前装置地址
***************************************************************/
WORD CPPrimary::GetEqpAddr(void)   
{
	return m_pEqpInfo[m_wEqpNo].wDAddress;
}

/***************************************************************
	Function:GetOwnAddr
		获取自身地址
	参数：无
	
	返回：自身地址
***************************************************************/
WORD CPPrimary::GetOwnAddr(void)		  
{
	return m_pEqpInfo[m_wEqpNo].wAddress;	
}

/***************************************************************
	Function:SwitchToAddress
		根据地址切换到相应模块
	参数：wAddress
		  wAddress 要切换到的地址
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CPPrimary::SwitchToAddress(WORD wAddress)
{
   WORD wEqpNo;

   if  (m_wEqpNum==0)
   {
	  if (wAddress==GetBroadcastAddr())	 return(TRUE);
   }	 
		 
   for (wEqpNo = 0; wEqpNo < m_wEqpNum; wEqpNo++)
   {
	  if(pGetEqpInfo(wEqpNo)->wDAddress == wAddress)
	  {
		 SwitchToEqpNo(wEqpNo);
		 return TRUE;
	  }
   }

   return FALSE;
}


BOOL CPPrimary::GetEqpCommStauts(WORD wEqpNo)
{
	if (wEqpNo >= m_wEqpNum)  return FALSE;
	return (m_pEqpInfo[wEqpNo].pEqpRunInfo->CommRunInfo.wState&COMMOK);
}

void CPPrimary::CommStatusProcByRT(BOOL bCommOk)
{
    if ((strcmp(m_ProtocolName, "GB104") == 0) ||(strcmp(m_ProtocolName, "MDCU101") == 0)
		||(strcmp(m_ProtocolName, "PN103NET") == 0)||(strcmp(m_ProtocolName, "SF103NET") == 0)
		||(strcmp(m_ProtocolName, "NR103NET") == 0)||(strcmp(m_ProtocolName, "XJ103") == 0)
		||(strcmp(m_ProtocolName, "SAC103") == 0))
		return;
	
	if (m_wEqpNum==0) return;
	
	if (bCommOk)
	{
		m_pEqpExtInfo[m_wEqpNo].wCommErrCount=0;
		if (GetEqpCommStauts(m_wEqpNo)==TRUE) return;
#ifdef INCLUDE_FLAG_CUR
		ClearRangeYCCfg(m_wEqpID, 0, -2, 0x40000000);
		ClearRangeSYXCfg(m_wEqpID, 0, -2, 0x40000000);
#endif
		::WriteCommStatus(m_wTaskID, m_wEqpID, TRUE);	
		OnCommOk();
	}
	else
	{
		if (GetEqpCommStauts(m_wEqpNo)==FALSE) return;

		m_pEqpExtInfo[m_wEqpNo].wCommErrCount++; 
		if (m_pEqpExtInfo[m_wEqpNo].wCommErrCount<=m_pBaseCfg->wMaxErrCount)	
		   return;
		if (m_pEqpExtInfo[m_wEqpNo].wCommErrCount==1) return;  /*第一次不进入因为发送主动调用此函数*/
#ifdef INCLUDE_FLAG_CUR
		SetRangeYCCfg(m_wEqpID, 0, -2, 0x40000000);
		SetRangeSYXCfg(m_wEqpID, 0, -2, 0x40000000);
#endif
		::WriteCommStatus(m_wTaskID, m_wEqpID, FALSE);				 
		OnCommError();
	}
}

void CPPrimary::CommStatusProcByRTNoErrCnt(BOOL bCommOk)
{
	if (m_wEqpNum==0) return;
	
	if (bCommOk)
	{
		m_pEqpExtInfo[m_wEqpNo].wCommErrCount=0;
		if (GetEqpCommStauts(m_wEqpNo)==TRUE) return;
#ifdef INCLUDE_FLAG_CUR
		ClearRangeYCCfg(m_wEqpID, 0, -2, 0x40000000);
		ClearRangeSYXCfg(m_wEqpID, 0, -2, 0x40000000);
#endif
		::WriteCommStatus(m_wTaskID, m_wEqpID, TRUE);	
		OnCommOk();
	}
	else
	{
		if (GetEqpCommStauts(m_wEqpNo)==FALSE) return;

		m_pEqpExtInfo[m_wEqpNo].wCommErrCount++; 
#ifdef INCLUDE_FLAG_CUR
		SetRangeYCCfg(m_wEqpID, 0, -2, 0x40000000);
		SetRangeSYXCfg(m_wEqpID, 0, -2, 0x40000000);
#endif
		::WriteCommStatus(m_wTaskID, m_wEqpID, FALSE);				 
		OnCommError();
	}
}

void CPPrimary::OnCommOk(void)
{
/*	if (m_pBaseCfg->wCmdControl&ALLDATAPROC)  
	{
	  SetEqpFlag(EQPFLAG_YC);
	  SetEqpFlag(EQPFLAG_YX);	  
	}
	if (m_pBaseCfg->wCmdControl&DDPROC)  
	{
	  SetEqpFlag(EQPFLAG_DD);	  
	}  
	if ((m_pBaseCfg->wCmdControl&CLOCKENABLE)&&(m_byProtocolType==PRIMARY))
	{
		if (m_pBaseCfg->wCmdControl&BROADCASTTIME) SetTaskFlag(TASKFLAG_CLOCK);
		else SetEqpFlag(EQPFLAG_CLOCK);
	} */
}
	
/***************************************************************
	Function:DoYKReq
		遥控设置消息处理
	参数：pMsg
		pMsg　消息缓冲区指针
	返回：TRUE 成功　FALSE 失败
***************************************************************/
BOOL CPPrimary::DoYKReq(void)
{
	VYKInfo *pYKInfo;
	VDBYK *pDBYK;
	WORD YkEqpNo;
	YkEqpNo = GetEqpNofromID(m_pMsg->Head.wEqpID);
	if (YkEqpNo == 0xFFFF)
	{
		pDBYK = (VDBYK *)m_pMsg->abyData;	  
		pDBYK->byStatus = 1;
		TaskSendMsg(DB_ID,m_wTaskID, m_pMsg->Head.wEqpID, m_pMsg->Head.byMsgID, MA_RET, sizeof(VYKInfo)-sizeof(VMsgHead), m_pMsg);
		return TRUE;		
	}
	pYKInfo = &(pGetEqpInfo(YkEqpNo)->YKInfo);
	memcpy(pYKInfo,m_pMsg,sizeof(VYKInfo));
	
	if (!(m_pBaseCfg->wCmdControl&YKOPENABLED))
	{
		pDBYK = (VDBYK *)m_pMsg->abyData;	  
		pDBYK->byStatus = 4;
		TaskSendMsg(DB_ID,m_wTaskID, pYKInfo->Head.wEqpID, pYKInfo->Head.byMsgID, MA_RET, sizeof(VYKInfo)-sizeof(VMsgHead), m_pMsg);
        return TRUE;		
	}
	
	SetEqpFlag(YkEqpNo, EQPFLAG_YKReq);
	m_pEqpExtInfo[YkEqpNo].wWaitYKCnt = m_pBaseCfg->wCtrlResultReqCount;
	SetEqpFlag(YkEqpNo, EQPFLAG_WaitYK);
	return TRUE;
}


/***************************************************************
	Function:DoYKRet
		遥控返校处理
	参数：无
	
	返回：TRUE 成功　FALSE 失败
***************************************************************/
BOOL CPPrimary::DoYKRet(void)
{	
	VYKInfo *pYKInfo;
	VDBYK *pDBYK;

	ClearEqpFlag(EQPFLAG_WaitYK);
	
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo); 
	pDBYK = (VDBYK *)m_pMsg->abyData;

	memcpy(pDBYK,&pYKInfo->Info,sizeof(VYKInfo));
		
	TaskSendMsg(DB_ID,m_wTaskID, m_wEqpID, pYKInfo->Head.byMsgID, MA_RET, sizeof(VYKInfo)-sizeof(VMsgHead), m_pMsg);
	return TRUE;		
}


BOOL CPPrimary::DoYTReq(void)
{
	VYTInfo *pYTInfo;
	VDBYT *pDBYT;

	if (SwitchToEqpID(m_pMsg->Head.wEqpID) != TRUE)
	{
		pDBYT = (VDBYT *)m_pMsg->abyData;	  
		pDBYT->byStatus = 1;
		TaskSendMsg(DB_ID,m_wTaskID,  m_pMsg->Head.wEqpID, m_pMsg->Head.byMsgID,  MA_RET, sizeof(VYTInfo)-sizeof(VMsgHead), m_pMsg);
		return TRUE;		
	}

	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	memcpy(pYTInfo,m_pMsg,sizeof(VYTInfo));
	
	if (!(m_pBaseCfg->wCmdControl&YKOPENABLED))
	{
		pDBYT = (VDBYT *)m_pMsg->abyData;	  
		pDBYT->byStatus = 4;
		TaskSendMsg(DB_ID,m_wTaskID, pYTInfo->Head.wEqpID, pYTInfo->Head.byMsgID, MA_RET, sizeof(VYTInfo)-sizeof(VMsgHead), m_pMsg);
        return TRUE;		
	}
	
	SetEqpFlag(EQPFLAG_YTReq);
	m_pEqpExtInfo[m_wEqpNo].wWaitYTCnt = m_pBaseCfg->wCtrlResultReqCount;
	SetEqpFlag(EQPFLAG_WaitYT);
	return TRUE;
}


BOOL CPPrimary::DoYTRet(void)
{	
	VYTInfo *pYTInfo;
	VDBYT *pDBYT;

	ClearEqpFlag(EQPFLAG_WaitYT);
	
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo); 
	pDBYT = (VDBYT *)m_pMsg->abyData;

	memcpy(pDBYT,&pYTInfo->Info,sizeof(VYTInfo));
		
	TaskSendMsg(DB_ID, m_wTaskID, m_wEqpID, pYTInfo->Head.byMsgID,MA_RET, sizeof(VYTInfo)-sizeof(VMsgHead), m_pMsg);
    return TRUE;		
}


void CPPrimary::DoTimeOut(void)
{
	thClearDog(m_wTaskID, COMMONDOGTM);
	m_dwTimerCount++;
	
	if (m_pBaseCfg->wCmdControl&ALLDATAPROC)
	{
		if (TimerOn(m_pBaseCfg->Timer.wAllData*60))
		{ 
			SetTaskFlag(TASKFLAG_ALLDATA);
			AllData.bFirst=TRUE;
		}  
	}	
	if (m_pBaseCfg->wCmdControl&CLOCKENABLE)
	{
		if (TimerOn(m_pBaseCfg->Timer.wSetClock*60))
		{ 
			SetTaskFlag(TASKFLAG_CLOCK);
			Clock.bFirst=TRUE;
		}  
	}	
	if (m_pBaseCfg->wCmdControl&DDPROC)
	{
		if (TimerOn(m_pBaseCfg->Timer.wDD*60))
		{ 
			SetTaskFlag(TASKFLAG_DD);
			DD.bFirst=TRUE;
		}  
	}	
}


/***************************************************************
	Function:GotoNextEqp
		切换到下一个装置
	参数：无
	
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CPPrimary::GotoNextEqp(void)
{
	WORD wEqpNo;

	if(SwitchToEqpFlag(EQPFLAG_WaitYK))
	{
		m_pEqpExtInfo[m_wEqpNo].wWaitYKCnt--;
		if (m_pEqpExtInfo[m_wEqpNo].wWaitYKCnt==0)  ClearEqpFlag(EQPFLAG_WaitYK);
		return TRUE;
	}

	if ((GetEqpFlag(EQPFLAG_YC)||GetEqpFlag(EQPFLAG_YX)||GetEqpFlag(EQPFLAG_DD))&&(GetEqpCommStauts(m_wEqpNo))) return(TRUE);
	
	if (GetTaskFlag(TASKFLAG_DD))
	{ 
	   if (DD.bFirst)
	   {
		   DD.bFirst=FALSE;
		   DD.wExpNo=m_wEqpNo;
		   DD.wBeginNo=m_wEqpNo;
		   DD.wCount=1;
	   }	
	
	   if (DD.wExpNo==m_wEqpNo)
	   {
		   DD.wCount++;
		   if (DD.wCount==2)
		   {
			   DD.wCount=0;
			   do
			   {
				 if (++DD.wExpNo>=m_wEqpNum)
				   DD.wExpNo=0;
				 if (DD.wExpNo==DD.wBeginNo)
				 {
				   ClearTaskFlag(TASKFLAG_DD);					
				   break;
				 }	
			   } while (GetEqpCommStauts(DD.wExpNo)==FALSE);
			   if (GetEqpCommStauts(m_wEqpNo)==TRUE)
			   {
				 SetEqpFlag(EQPFLAG_DD);
				 return TRUE;
			   }	   
		   }
	   }			
	}

	if (GetTaskFlag(TASKFLAG_ALLDATA))
	{ 
	   if (AllData.bFirst)
	   {
		   AllData.bFirst=FALSE;
		   AllData.wExpNo=m_wEqpNo;
		   AllData.wBeginNo=m_wEqpNo;
		   AllData.wCount=1;
	   }	

	   if (AllData.wExpNo==m_wEqpNo)
	   {
		   AllData.wCount++;
		   if (AllData.wCount==2)
		   {
			   AllData.wCount=0;
			   do
			   {
				 if (++AllData.wExpNo>=m_wEqpNum)
				   AllData.wExpNo=0;
				 if (AllData.wExpNo==AllData.wBeginNo)
				 {
				   ClearTaskFlag(TASKFLAG_ALLDATA); 				
				   break;
				 }	
			   } while (GetEqpCommStauts(AllData.wExpNo)==FALSE);
			   if (GetEqpCommStauts(m_wEqpNo)==TRUE)
			   {
				 SetEqpFlag(EQPFLAG_YC);
				 SetEqpFlag(EQPFLAG_YX);
				 return TRUE;
			   }	 
		   }
	   }			
	}

	if (GetTaskFlag(TASKFLAG_CLOCK)&&(!(m_pBaseCfg->wCmdControl&BROADCASTTIME)))
	{ 
	   if (Clock.bFirst)
	   {
		   Clock.bFirst=FALSE;
		   Clock.wExpNo=m_wEqpNo;
		   Clock.wBeginNo=m_wEqpNo;
		   Clock.wCount=1;
	   }	
	
	   if (Clock.wExpNo==m_wEqpNo)
	   {
		   Clock.wCount++;
		   if (Clock.wCount==2)
		   {
			   Clock.wCount=0;
			   do
			   {
				 if (++Clock.wExpNo>=m_wEqpNum)
				   Clock.wExpNo=0;
				 if (Clock.wExpNo==Clock.wBeginNo)
				 {
				   ClearTaskFlag(TASKFLAG_CLOCK);					
				   break;
				 }	
			   } while (GetEqpCommStauts(Clock.wExpNo)==FALSE);
			   if (GetEqpCommStauts(m_wEqpNo)==TRUE)
			   {
				 SetEqpFlag(EQPFLAG_CLOCK);
				 return TRUE;
			   }
		   }
	   }			
	}

	if(SwitchClearEqpFlag(EQPFLAG_POLL))  return TRUE;

	m_dwCycCount++;
	if (m_dwCycCount%3 == 0)
	{
		SetAllEqpFlag(EQPFLAG_POLL);	
		m_dwCycCount=0;
	}		
	else
	{
		for(wEqpNo = 0; wEqpNo < m_wEqpNum; wEqpNo++)
		{
			if (GetEqpCommStauts(wEqpNo)==FALSE) continue;
			SetEqpFlag(wEqpNo, EQPFLAG_POLL);
		}
	}

	if (SwitchClearEqpFlag(EQPFLAG_POLL))
	{
		return TRUE;
	}

	SetAllEqpFlag(EQPFLAG_POLL);		
	m_dwCycCount=0;
		
	SwitchClearEqpFlag(EQPFLAG_POLL);
	
	return FALSE;
}
BOOL CPPrimary::DoSend(void)
{
#if 0
    if (m_wRetryCount ==  m_pBaseCfg->wMaxRetryCount)
    {
    	ClearEqpFlag(EQPFLAG_YC);
		ClearEqpFlag(EQPFLAG_YX);
		ClearEqpFlag(EQPFLAG_DD);
    }
#endif
    if (GetRetryFlag()) 
    {
	   return(SendRetry());
    } 

    if (ReqUrgency()==TRUE)  return(TRUE);

    GotoNextEqp();

    if (ReqCyc()==TRUE) return(TRUE);

    ReqNormal();

    return(TRUE);
}

void CPPrimary::DoDataRefresh(void)
{
	SetDefFlag();
}

void CPPrimary::OnCommError(void)
{}

BOOL CPPrimary::ReqUrgency(void)
{
     return FALSE;
}

BOOL CPPrimary::ReqCyc(void)
{
     return FALSE;
}

BOOL CPPrimary::ReqNormal(void)
{
     return FALSE;
}

#endif

