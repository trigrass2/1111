/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/
#include "syscfg.h"

#if (defined(INCLUDE_PMASTER)|| defined(INCLUDE_PSLAVE))

#include "os.h"
#include "pbase.h"

extern BYTE commid;
WORD m_w101sTaskID = 0;
WORD m_w104sTaskID = 0;
WORD m_w101mTaskID = 0;
WORD m_w104mTaskID = 0;
#if (TYPE_CPU == CPU_SAM9X25)
struct VNextParaInfo WriteNextParaInfo[256];
struct VNextParaInfo ReadNextParaInfo[256];
#endif
BYTE writeflag;


/***************************************************************
	Function:CProtocol
		规约基类构造函数
	参数：无
	返回：无
***************************************************************/
CProtocol::CProtocol()		
{
	memset(&m_SendBuf, 0, sizeof(m_SendBuf));   
    memset(&m_RecBuf, 0, sizeof(m_RecBuf));      
    memset(&m_RecFrame, 0, sizeof(m_RecFrame));  
    memset(&m_TaskFlags, 0, sizeof(VFLAGS)); 		
	memset(&m_EqpFlagsMap, 0, sizeof(VFLAGS));	
	m_wCommCtrl=0;
	m_dwCommCtrlTimer=0;
	m_wEqpNum = 0;
	m_wEqpNo = 0;
	m_wEqpID = 0;
	m_dwCycCount = 0;
	m_dwTimerCount = 0;
	DisableRetry();
	/*m_pMsg = (VMsg *)new BYTE[MAXMSGSIZE];*/
	m_pMsg = (VMsg *)m_dwPubBuf;
	m_bPollMasterSendFlag = TRUE;
}

/***************************************************************
	Function:Init
		  规约平台初始化
	参数：无
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CProtocol::Init(WORD wTaskID,WORD wMinEqpNum,void *pCfg,WORD wCfgLen)
{
	m_wTaskID = m_wCommID = wTaskID;			
	m_pTaskInfo = GetTaskInfo(m_wTaskID);
    m_ProtocolName = GetCommCfg(m_wTaskID)->Port.pcol;
	
	if (!InitEqpInfo(wMinEqpNum))
	{
		return FALSE;
	}

	if (ReadProtcolCfg(pCfg,wCfgLen)==FALSE)
	{
		SetBaseCfg();
		SetDefCfg();
	}

	CheckBaseCfg();
	CheckCfg();
	  
	m_SendBuf.wBufSize = m_pBaseCfg->wMaxDLlen+m_pBaseCfg->wMaxDLlen/2;
	m_RecBuf.wBufSize = 2*m_pBaseCfg->wMaxDLlen;

	if (m_SendBuf.wBufSize==0)	m_SendBuf.wBufSize = MAX_SEND_LEN;
	if (m_RecBuf.wBufSize==0)	m_RecBuf.wBufSize = MAX_RECV_LEN;
		
	m_SendBuf.pBuf = (BYTE*)malloc(m_SendBuf.wBufSize);
	if(!m_SendBuf.pBuf)  return FALSE;
	m_RecBuf.pBuf = (BYTE*)malloc(m_RecBuf.wBufSize);
	if(!m_RecBuf.pBuf)	 return FALSE;

    m_dwCommCtrlTimer=GetTimeOutValue(m_pBaseCfg->wMaxDLlen);
		
	return TRUE;	
}

/***************************************************************
	Function:ProtocolExit
		规约任务挂起退出
	参数：无
	返回：无
***************************************************************/
void CProtocol::ProtocolExit(void)
{
    WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "规约对应的装置参数错误");
	myprintf(m_wTaskID, LOG_ATTR_WARN, "Protocol init failed and exit!");	
	thDisableDog(m_wTaskID);
	thSuspend(m_wTaskID);	
}
	
BOOL CProtocol::ReadProtcolCfg(void *pCfg,WORD wCfgLen)
{
    VProtocolBaseCfg *pBaseCfg;

	pBaseCfg=(VProtocolBaseCfg *)g_Task[m_wTaskID].CommCfg.pvProtocolCfg;

	if (pBaseCfg==NULL)
	{
        m_pBaseCfg=(struct VProtocolBaseCfg*)calloc(1, sizeof(struct VProtocolBaseCfg));
	    return FALSE;
	}

#if 0
    #if (_BYTE_ORDER == _BIG_ENDIAN)  //Moto系统
	    int i;
        pBaseCfg->wProtcolID=ADJUSTWORD(pBaseCfg->wProtcolID);
  	    pBaseCfg->wCmdControl=ADJUSTWORD(pBaseCfg->wCmdControl);
  	    pBaseCfg->wMaxALLen=ADJUSTWORD(pBaseCfg->wMaxALLen);
  	    pBaseCfg->wMaxDLlen=ADJUSTWORD(pBaseCfg->wMaxDLlen);
        pBaseCfg->wMaxRetryCount=ADJUSTWORD(pBaseCfg->wMaxRetryCount);
        pBaseCfg->wMaxErrCount=ADJUSTWORD(pBaseCfg->wMaxErrCount);
	    pBaseCfg->wCtrlResultReqCount=ADJUSTWORD(pBaseCfg->wCtrlResultReqCount);
	    pBaseCfg->wActSendRetryCount=ADJUSTWORD(pBaseCfg->wActSendRetryCount);
	    pBaseCfg->wYCDeadValue=ADJUSTWORD(pBaseCfg->wYCDeadValue);
	    pBaseCfg->wBroadcastAddr=ADJUSTWORD(pBaseCfg->wBroadcastAddr);
	    pBaseCfg->Timer.wAllData=ADJUSTWORD(pBaseCfg->Timer.wAllData);
	    pBaseCfg->Timer.wSetClock=ADJUSTWORD(pBaseCfg->Timer.wSetClock);
	    pBaseCfg->Timer.wDD=ADJUSTWORD(pBaseCfg->Timer.wDD);
	    pBaseCfg->Timer.wScanData2=ADJUSTWORD(pBaseCfg->Timer.wScanData2);
	    for (i=0;i<4;i++)
  	      pBaseCfg->dwRsv[i]=ADJUSTDWORD(pBaseCfg->dwRsv[i]);
    #endif 
#endif

    m_pBaseCfg=pBaseCfg;
     
    if ((wCfgLen==0)||(pCfg==NULL))  return(TRUE); 
	memcpy(pCfg,pBaseCfg+1,wCfgLen);      

    return(TRUE);      
}

void CProtocol::CheckBaseCfg(void)
{
   if (m_pBaseCfg->wMaxALLen==0) m_pBaseCfg->wMaxALLen=1024;
   if (m_pBaseCfg->wMaxDLlen==0) m_pBaseCfg->wMaxDLlen=1024;

   if (m_byProtocolType==SECONDARY)  m_pBaseCfg->wCmdControl&=(~BROADCASTTIME);

    /*??check*/
    /*m_pBaseCfg->wMaxRetryCount=3;
    m_pBaseCfg->wMaxErrCount=3;
	m_pBaseCfg->wCtrlResultReqCount=5;
    m_pBaseCfg->wActSendRetryCount=3;*/
}

void CProtocol::SetBaseCfg(void)
{
    if (m_byProtocolType==PRIMARY)
      m_pBaseCfg->wCmdControl=YKOPENABLED|CLOCKENABLE|BROADCASTTIME;
    else
      m_pBaseCfg->wCmdControl=YKOPENABLED;   	
	m_pBaseCfg->wMaxALLen=1024;
  	m_pBaseCfg->wMaxDLlen=1024;
    m_pBaseCfg->wMaxRetryCount=3;
    m_pBaseCfg->wMaxErrCount=3;
	m_pBaseCfg->wCtrlResultReqCount=5;
    m_pBaseCfg->wActSendRetryCount=3;
	m_pBaseCfg->wBroadcastAddr=0xFF;
	m_pBaseCfg->Timer.wAllData=30;
	m_pBaseCfg->Timer.wSetClock=5;
	m_pBaseCfg->Timer.wDD=60;
	m_pBaseCfg->Timer.wScanData2=1000;
	m_pBaseCfg->wACDeadValue = 10;
	m_pBaseCfg->wIDeadValue = 10;
	m_pBaseCfg->wDCDeadValue = 10;
	m_pBaseCfg->wPQDeadValue = 50;
	m_pBaseCfg->wFDeadValue = 5;
	m_pBaseCfg->wPWFDeadValue = 20;
	m_pBaseCfg->wUTHDeadValue = 10;
	m_pBaseCfg->wITHDeadValue = 10;
	m_pBaseCfg->wUNDeadValue = 10;
	m_pBaseCfg->wLoadDeadValue = 10;
	m_pBaseCfg->wYCDeadValue = 10;
}
	
WORD CProtocol::GetTimeOutValue(WORD l)
{
    DWORD baudrate;
	
    if ((m_wTaskID>=COMM_SERIAL_START_NO)&&(m_wTaskID<=COMM_NET_START_NO))
    {
		commCtrl(m_wCommID, CCC_BAUD_CTRL|BAUD_GET, (BYTE*)&baudrate);
		return((l*10*SECTOTICK(1)/baudrate+SECTOTICK(1))/COMM_SCAN_TM); 
    }
	else  
	{
       return(SECTOTICK(1)/COMM_SCAN_TM);  //1s
	}
}

/***************************************************************
	Function:pGetEqpInfo
		获取装置信息
	参数：wDevNo
		wDevNo　装置号（数组序号）
	
	返回：指向装置信息的指针
***************************************************************/
VPtEqpInfo* CProtocol::pGetEqpInfo(WORD wEqpNo)
{
	if (wEqpNo >= m_wEqpNum)
		return NULL;
	return &m_pEqpInfo[wEqpNo];
}


/***************************************************************
	Function:NeatenCommBuf
		通信缓冲区的规整
	参数：pCommIO
		pCommIO　指向通信缓冲区的指针
	返回：无
***************************************************************/
void CProtocol::NeatenCommBuf(VpcolBuf *pCommBuf)
{
	register unsigned  int i,j;

	if (pCommBuf->wReadPtr == 0)
	{
		if(pCommBuf->wWritePtr >= pCommBuf->wBufSize)
		{
			pCommBuf->wWritePtr = 0;
			WriteDoEvent(NULL, 0, "NeatenCommBuf clearbuf len %d ",pCommBuf->wWritePtr);
		}
		return ; //读指针已经为0
	}


	if (pCommBuf->wReadPtr >= pCommBuf->wWritePtr)
	{
		pCommBuf->wReadPtr = pCommBuf->wWritePtr=0;
		return ;
	}


	if (pCommBuf->wWritePtr >= pCommBuf->wBufSize)
	{
		pCommBuf->wReadPtr = 0;
		pCommBuf->wWritePtr = 0;
		return ;
	}

	i = 0; 
	j = pCommBuf->wReadPtr;
	while (j < pCommBuf->wWritePtr)
	{
		pCommBuf->pBuf[i++] = pCommBuf->pBuf[j++];
	}

	pCommBuf->wReadPtr = 0; 
	pCommBuf->wWritePtr = i; 
}

/***************************************************************
	Function:SearchFrame
		搜索信息帧
	参数：无
	
	返回：TRUE 找到正确的一帧报文，FALSE 未找到一帧正确的报文
***************************************************************/
BOOL CProtocol::SearchFrame(void)
{
	DWORD Rc;
	DWORD Len;//已处理过的字节数
	short MsgLen;

	//??NeatenCommBuf(&m_RecBuf);//清理接收缓冲区
	while (1)
	{
		MsgLen = m_RecBuf.wWritePtr - m_RecBuf.wReadPtr;
		if (MsgLen <= 0)
		{
			m_dwHaveRxdFm = 0;
			
			return FALSE;
		}

		Rc = SearchOneFrame(&m_RecBuf.pBuf[m_RecBuf.wReadPtr], MsgLen);
		Len = Rc & ~FRAME_SHIELD; //已处理过的字节数

		switch	(Rc & FRAME_SHIELD)
		{
			case FRAME_OK:
				commCtrl(m_wCommID, CCC_EVENT_CTRL|(m_wCommCtrl<<1), NULL);	
				DisableRetry();
				CommStatusProcByRT(TRUE);
				m_RecFrame.pBuf = &m_RecBuf.pBuf[m_RecBuf.wReadPtr];  //记录有效报文的起始地址
				m_RecFrame.wWritePtr = (WORD)Len; //记录有效报文的长度
				m_RecBuf.wReadPtr += (WORD)Len; //指针移到下一报文处
				if (m_RecBuf.wReadPtr >= m_RecBuf.wWritePtr)
				{
					m_dwHaveRxdFm=0;
				}
				return TRUE;

			case FRAME_ERR:
				if (!Len)
				{
					//m_RecBuf.wReadPtr++;
				}
				else
				{
					m_RecBuf.wReadPtr += Len; //指针移到下一报文处
				}
				break;

			case FRAME_LESS:
				m_RecBuf.wReadPtr += (WORD)Len; //指针移到下一报文处
				m_dwHaveRxdFm = 0;
				return FALSE;
			default:
				break;//??do what
		}//switch
	}//while
}

/***************************************************************
	Function:ReadFromComm
		从通信端口读取数据
	参数：无
	
	返回：实际从端口读到的数据的字节个数
***************************************************************/
int CProtocol::ReadFromComm(void)
{
	int Rc;
	NeatenCommBuf(&m_RecBuf);  
	Rc = ::commRead(m_wCommID,m_RecBuf.pBuf+m_RecBuf.wWritePtr, m_RecBuf.wBufSize-m_RecBuf.wWritePtr, 0);
	
	/*if (m_RecBuf.wWritePtr == m_RecBuf.wBufSize)
	{
		m_RecBuf.wReadPtr = 0;
		m_RecBuf.wWritePtr = 0;
	}*/	
	
	if (Rc>0)
	{
		m_dwHaveRxdFm=1;
		m_RecBuf.wWritePtr += (WORD)Rc;
		m_bPollMasterSendFlag = TRUE;
	}
	else
	{
		m_dwHaveRxdFm=0;
	}
	
	return Rc;
}


/***************************************************************
	Function:WriteToComm
		向通信端口写数据
	参数：Address,Flag
		Address 地址，暂未用
		Flag　标志，暂未用
	返回：实际写到端口的数据的字节个数 <0 ERROR
***************************************************************/
int CProtocol::WriteToComm(DWORD dwFlag)
{
	int Rc=0;	

	if (m_SendBuf.wWritePtr - m_SendBuf.wReadPtr)
	{
		m_SendBuf.wOldReadPtr = m_SendBuf.wReadPtr;

		m_SendBuf.dwFlag = dwFlag;
		IncRetryCount();
		
		Rc = ::commWrite(m_wCommID,(m_SendBuf.pBuf+m_SendBuf.wReadPtr),m_SendBuf.wWritePtr-m_SendBuf.wReadPtr,dwFlag);

		if (Rc!=ERROR)			m_SendBuf.wReadPtr += Rc;

		if ((Rc != ERROR) && (m_wTaskID != MMI_ID))  
		{
			commid = 1;
			/*commid ^= 0x01;
			if (commid&0x01)
			turnLight(BSP_LIGHT_COMM_ID, 1);
			else
			turnLight(BSP_LIGHT_COMM_ID, 0);
			*/
		}
		if (LOWORD(dwFlag) == GetBroadcastAddr())
		{
			DisableRetry();
			DWORD dwCommCtrlTimer = 1;//20 ms
			commCtrl(m_wCommID, CCC_EVENT_CTRL|m_wCommCtrl, (BYTE*)&dwCommCtrlTimer);
			return Rc;
		}
		else  CommStatusProcByRT(FALSE);
	}
	
	commCtrl(m_wCommID, CCC_EVENT_CTRL|m_wCommCtrl, (BYTE*)&m_dwCommCtrlTimer);	

	return Rc;
}


//标志操作相关

/***************************************************************
	Function:pGetEqpFlags
		获取装置标志
	参数：dwEqpNo
		dwEqpNo 装置序号（数组号）
	返回：指向装置标志的指针
***************************************************************/
VFLAGS* CProtocol::pGetEqpFlags(DWORD dwEqpNo)
{
	return (&pGetEqpInfo(dwEqpNo)->Flags);
}

/***************************************************************
	Function:GetTaskFlag
		获取任务标志的第FlagNo个位置的状态位
	参数：FlagNo
		FlagNo 标记号
	返回：标志状态，1 有效，0 无效
***************************************************************/
DWORD CProtocol::GetTaskFlag(DWORD FlagNo)
{ 
	return ::PGetFlag(&m_TaskFlags, FlagNo); 
}

/***************************************************************
	Function:SetTaskFlag
		设置任务标志，将m_TaskFlags的第FlagNo个位置位成1
	参数：FlagNo
		FlagNo 标记号
	返回：无
***************************************************************/
void  CProtocol::SetTaskFlag(DWORD FlagNo)
{ 
	::PSetFlag(&m_TaskFlags, FlagNo); 
}

/***************************************************************
	Function:ClearTaskFlag
		清除任务标志，将m_TaskFlags的第FlagNo个位置位成0
	参数：FlagNo
		FlagNo 标记号
	返回：无
***************************************************************/
void  CProtocol::ClearTaskFlag(DWORD FlagNo)
{ 
	::PClearFlag(&m_TaskFlags, FlagNo); 
}

/***************************************************************
	Function:GetEqpFlagMap
		获取装置标志映射，m_EqpFlagsMap的第FlagNo个位置的状态
	参数：FlagNo
		FlagNo 标记号
	返回：1 有效，0 无效
***************************************************************/
DWORD CProtocol::GetEqpFlagMap(DWORD FlagNo)
{ 
	return ::PGetFlag(&m_EqpFlagsMap, FlagNo); 
}

/***************************************************************
	Function:SetEqpFlagMap
		设置装置标志映射，将m_EqpFlagsMap的第FlagNo个位置位为1
	参数：FlagNo
		FlagNo 标记号
	返回：无
***************************************************************/
void  CProtocol::SetEqpFlagMap(DWORD FlagNo)
{ 
	::PSetFlag(&m_EqpFlagsMap, FlagNo);
}

/***************************************************************
	Function:ClearEqpFlagMap
		清除装置标志映射，将m_EqpFlagsMap的第FlagNo个位置位为0
	参数：FlagNo
		FlagNo 标记号
	返回：无
***************************************************************/
void  CProtocol::ClearEqpFlagMap(DWORD FlagNo)
{ 
	::PClearFlag(&m_EqpFlagsMap, FlagNo);
}

/***************************************************************
	Function:GetEqpFlag
		获取装置标志，第dwEqpNo装置的标志的第dwFlagNo个位的状态
	参数：dwEqpNo, dwFlagNo
		dwEqpNo 装置号
		dwFlagNo 标记号
	返回：1 有效，0 无效
***************************************************************/
DWORD  CProtocol::GetEqpFlag(DWORD dwEqpNo, DWORD dwFlagNo)
{
	VFLAGS *pFlags;
	pFlags = pGetEqpFlags(dwEqpNo);

	return ::PGetFlag(pFlags, dwFlagNo);
}

/***************************************************************
	Function:SetEqpFlag
		设置装置标志，将第dwEqpNo装置的标志的第dwFlagNo个位置位为1
	参数：dwEqpNo, dwFlagNo
		dwEqpNo 装置号
		dwFlagNo 标记号
	返回：无
***************************************************************/
void   CProtocol::SetEqpFlag(DWORD dwEqpNo, DWORD dwFlagNo)
{
	VFLAGS *pFlags;
	pFlags = pGetEqpFlags(dwEqpNo);
	::PSetFlag(pFlags, dwFlagNo);
	SetEqpFlagMap(dwFlagNo);
}

/***************************************************************
	Function:ClearEqpFlag
		清除装置标志，将第dwEqpNo装置的标志的第dwFlagNo个位置位为0
	参数：dwEqpNo, dwFlagNo
		dwEqpNo 装置号
		dwFlagNo 标记号
	返回：无
***************************************************************/
void   CProtocol::ClearEqpFlag(DWORD dwEqpNo, DWORD dwFlagNo)
{
	VFLAGS *pFlags;
	pFlags = pGetEqpFlags(dwEqpNo);
	::PClearFlag(pFlags, dwFlagNo);

    //已处理本条控制消息内容，触发下一条
	if ((dwFlagNo == EQPFLAG_YKReq) || (dwFlagNo == EQPFLAG_YKRet)
		|| (dwFlagNo == EQPFLAG_YTReq) || (dwFlagNo == EQPFLAG_YTRet)
		|| (dwFlagNo == EQPFLAG_TQReq) || (dwFlagNo == EQPFLAG_TQReq))
		evSend(m_wTaskID, EV_MSG);		
}

/***************************************************************
	Function:GetEqpFlag
		获取当前装置标志第dwFlagNo个位置的状态
	参数：dwFlagNo
		dwFlagNo 标记号
	返回：1 有效，0 无效
***************************************************************/
DWORD CProtocol::GetEqpFlag(DWORD dwFlagNo)
{
	return GetEqpFlag(m_wEqpNo, dwFlagNo);
}

/***************************************************************
	Function:SetEqpFlag
		将当前装置标志第dwFlagNo位置位为1
	参数：dwFlagNo
		dwFlagNo 标记号
	返回：无
***************************************************************/
void CProtocol::SetEqpFlag(DWORD dwFlagNo)
{
	SetEqpFlag(m_wEqpNo, dwFlagNo);
}

/***************************************************************
	Function:ClearEqpFlag
		将当前装置标志第dwFlagNo位置位为0
	参数：dwFlagNo
		dwFlagNo 标记号
	返回：无
***************************************************************/
void CProtocol::ClearEqpFlag(DWORD dwFlagNo)
{
	ClearEqpFlag(m_wEqpNo, dwFlagNo);
}

/***************************************************************
	Function:SetAllEqpFlag
		将所有装置的标志的第dwFlagNo位置位为1
	参数：dwFlagNo
		dwFlagNo 标记号
	返回：无
***************************************************************/
void CProtocol::SetAllEqpFlag(DWORD dwFlagNo)
{
	DWORD dwEqpNo;
	DWORD dwEqpNum;
	dwEqpNum = m_wEqpNum;
	for (dwEqpNo=0; dwEqpNo < dwEqpNum; dwEqpNo++)
	{
		SetEqpFlag(dwEqpNo, dwFlagNo);
	}
}

/***************************************************************
	Function:ClearAllEqpFlag
		将所有装置的标志的第dwFlagNo位置位为0
	参数：wFlagNo
		dwFlagNo 标记号 
	返回：无
***************************************************************/
void CProtocol::ClearAllEqpFlag(DWORD dwFlagNo)
{
	DWORD dwEqpNo;
	DWORD dwEqpNum;
	dwEqpNum = m_wEqpNum;
	for (dwEqpNo = 0; dwEqpNo < dwEqpNum; dwEqpNo++)
	{
		ClearEqpFlag(dwEqpNo, dwFlagNo);
	}
}

/***************************************************************
	Function:SwitchToEqpFlag
		切换到标志第dwFlagNo位有效的装置，否则将装置标志映射第dwFlagNo
		位置位为0
	参数：dwFlagNo
		dwFlagNo 标记号
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CProtocol::SwitchToEqpFlag(DWORD dwFlagNo)
{
	DWORD dwEqpNo;
	DWORD dwEqpNum;
	if (!GetEqpFlagMap(dwFlagNo))
		return FALSE;
	dwEqpNum = m_wEqpNum;
	for (dwEqpNo = 0; dwEqpNo < dwEqpNum; dwEqpNo++)
	{
		if (GetEqpFlag(dwEqpNo, dwFlagNo))
		{
			SwitchToEqpNo(dwEqpNo);
			return TRUE;
		}
	}
	ClearEqpFlagMap(dwFlagNo);
	return FALSE;
}

/***************************************************************
	Function:SwitchClearEqpFlag
		切换到标志第dwFlagNo位有效的装置，否则将装置标志映射第dwFlagNo
		位置位为0，然后将装置标志第dwFlagNo位设置为0
	参数：dwFlagNo
		dwFlagNo 标记号
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CProtocol::SwitchClearEqpFlag(DWORD dwFlagNo)
{
	if (SwitchToEqpFlag(dwFlagNo))
	{
		ClearEqpFlag(m_wEqpNo, dwFlagNo);
		return TRUE;
	}
	return FALSE;	
}

/***************************************************************
	Function:CheckClearTaskFlag
		检查清除任务标志，若任务标志第dwFlagNo位有效，将其清除
		然后返回TRUE,否则返回FALSE
	参数：dwFlagNo
		标记号
	返回：TRUE 任务标志第dwFlagNo位有效
		  FALSE 任务标志第dwFlagNo位无效
***************************************************************/
BOOL CProtocol::CheckClearTaskFlag(DWORD dwFlagNo)
{
	if (!GetTaskFlag(dwFlagNo))
		return FALSE;

	ClearTaskFlag(dwFlagNo);
	return TRUE;
}

/***************************************************************
	Function:CheckClearEqpFlag
		检查清除装置标志，若装置标志第dwFlagNo位有效，将其清除
		然后返回TRUE,否则返回FALSE
	参数：dwFlagNo
		标记号
	返回：TRUE 任务标志第dwFlagNo位有效
		  FALSE 任务标志第dwFlagNo位无效
***************************************************************/
BOOL CProtocol::CheckClearEqpFlag(DWORD dwFlagNo)
{
	if (!GetEqpFlag(dwFlagNo))
		return FALSE;

	ClearEqpFlag(dwFlagNo);
	return TRUE;
}


//切换相关 
/***************************************************************
	Function:SwitchToEqpNo
		切换到第dwEqpNo号装置
	参数：dwEqpNo
		dwEqpNo 装置序号（数组序号）
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CProtocol::SwitchToEqpNo(DWORD dwEqpNo)
{
	if(dwEqpNo>m_wEqpNum)
		return FALSE;

	m_wEqpNo = (WORD)dwEqpNo;
	m_wEqpID = pGetEqpInfo(dwEqpNo)->wEqpID;

	return TRUE;
}


/***************************************************************
	Function:SwitchToEqpID
		根据装置ID切换到相应模块
	参数：wEqpID
		wEqpID 装置ID
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CProtocol::SwitchToEqpID(WORD wEqpID) 
{
	WORD wEqpNo;
	for (wEqpNo = 0; wEqpNo < m_wEqpNum; wEqpNo++)
	{
		if (pGetEqpInfo(wEqpNo)->wEqpID == wEqpID)
		{
			m_wEqpNo = wEqpNo;
			SwitchToEqpNo(wEqpNo);
			return TRUE;
		}
	}
	return FALSE;
}


/***************************************************************
	Function:GetEqpNofromID
		根据装置ID 获取装置No
	参数：wEqpID
		wEqpID 装置ID
	返回：装置No，返回0xFFFF  表示失败
***************************************************************/
WORD CProtocol::GetEqpNofromID(WORD wEqpID) 
{
	WORD wEqpNo;
	for (wEqpNo = 0; wEqpNo < m_wEqpNum; wEqpNo++)
	{
		if (pGetEqpInfo(wEqpNo)->wEqpID == wEqpID)
		{
			return wEqpNo;
		}
	}
	return 0xFFFF;
}

/***************************************************************
	Function:SearchHead
		搜索报文中的一个关键字
	参数：Buf, Len,　Offset1, Key1
		Buf 报文缓冲区头指针
		Len 要搜索的报文长度
		Offset1 关键字的在报文中的偏移位置
		Key1 关键字
	返回：关键字在报文缓冲区中的偏移位置
***************************************************************/
WORD CProtocol::SearchHead(BYTE *Buf, WORD Len,short Offset1,BYTE Key1)
{
	int i;
	for(i=0;i<(short)(Len-Offset1);i++)
	{
		if(Buf[Offset1+i]==Key1)
			return i;
	}

	return i;
}

/***************************************************************
	Function:SearchHead
		搜索报文中的两个关键字
	参数：Buf, Len, Offset1, Key1, Offset2, Key2
		Buf 报文缓冲区头指针
		Len 要搜索的报文长度
		Offset1 关键字1的在报文中的偏移位置
		Key1 关键字1
		Offset2 关键字2的在报文中的偏移位置
		Key2 关键字2
	返回：关键字1在报文缓冲区中的偏移位置
***************************************************************/
WORD CProtocol::SearchHead(BYTE *Buf, WORD Len,short Offset1,BYTE Key1,short Offset2,BYTE Key2)
{
	WORD i;
	WORD MLen;
	if(Offset1<Offset2)
		MLen=Offset2;
	else
		MLen=Offset1;
	for(i=0;i<(short)(Len-MLen);i++)
	{
		if(Buf[Offset1+i]==Key1&&Buf[Offset2+i]==Key2)
			return i;
	}

	return i;
}

/***************************************************************
	Function:SendWordLH
		向发送缓冲区中传送一个WORD，低字节在前,高字节在后
	参数：wData
		wData 要传送的WORD数据
	返回：无
***************************************************************/
void CProtocol::SendWordLH(WORD wData)
{

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(wData);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(wData);
}


/***************************************************************
	Function:SendWordLH
		向发送缓冲区中传送一个WORD，低字节在前,高字节在后
	参数：dwWPtr, wData
		dwWPtr 写指针
		wData　要传送的WORD数据
	返回：无
***************************************************************/
void CProtocol::SendWordLH(DWORD dwWPtr,WORD wData)
{

	m_SendBuf.pBuf[dwWPtr++] = LOBYTE(wData);
	m_SendBuf.pBuf[dwWPtr++] = HIBYTE(wData);
}


/***************************************************************
	Function:SendDWordLH
		向发送缓冲区中传送一个DWORD，低字节在前,高字节在后
	参数：dwData
		dwData　要传送的DWORD数据
	返回：无
***************************************************************/
void CProtocol::SendDWordLH(DWORD dwData)
{
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(dwData));//LLBYTE(dwData);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(dwData));//LHBYTE(dwData);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(dwData));//HLBYTE(dwData);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(dwData));//HHBYTE(dwData);
}


/***************************************************************
	Function:SendDWordLH
		向发送缓冲区中传送一个DWORD，低字节在前,高字节在后
	参数：dwWPtr, dwData
		dwWPtr 写指针
		dwData 要传送的DWORD数据
	返回：无
***************************************************************/
void CProtocol::SendDWordLH(DWORD dwWPtr,DWORD dwData)
{
	m_SendBuf.pBuf[dwWPtr++] = LOBYTE(LOWORD(dwData));//LLBYTE(dwData);
	m_SendBuf.pBuf[dwWPtr++] = HIBYTE(LOWORD(dwData));//LHBYTE(dwData);
	m_SendBuf.pBuf[dwWPtr++] = LOBYTE(HIWORD(dwData));//HLBYTE(dwData);
	m_SendBuf.pBuf[dwWPtr++] = HIBYTE(HIWORD(dwData));//HHBYTE(dwData);
}

/***************************************************************
	Function:GetGBAddress
		获取当前的广播地址
	参数：无
	
	返回：DWORD广播地址
***************************************************************/
WORD CProtocol::GetBroadcastAddr(void)
{
	return	m_pBaseCfg->wBroadcastAddr;
}


//重发机制相关

//获取最大重发次数
WORD CProtocol::GetMaxRetryCount(void)
{
	return	m_pBaseCfg->wMaxRetryCount;
}

//获取重发标志
BOOL CProtocol::GetRetryFlag(void)
{
	if (m_wRetryCount	<  m_pBaseCfg->wMaxRetryCount)
		return TRUE;
	if (m_wRetryCount!=65535)  commCtrl(m_wCommID, CCC_EVENT_CTRL|(m_wCommCtrl<<1), NULL);			
	m_wRetryCount = 0;
	return FALSE;
}


//获取重发次数
WORD CProtocol::GetRetryCount(void)
{
	return m_wRetryCount;
}

//重发次数清0
void CProtocol::ResetRetryCount(void)
{ 
	m_wRetryCount = 0;
}

//重发次数加1
void CProtocol::IncRetryCount(void)
{	
	m_wRetryCount++;
}

//禁止重发	  
void CProtocol::DisableRetry(void)
{	 
	m_wRetryCount = 65535;
}

//重发报文
BOOL CProtocol::SendRetry(void)
{
	m_SendBuf.wReadPtr = m_SendBuf.wOldReadPtr; 
	WriteToComm(m_SendBuf.dwFlag);
	return TRUE;	
}


/***************************************************************
	Function:TimerOn
		定时函数
	参数：TimeOut
		TimeOut：定时间隔，单位毫秒
	返回：TRUE 定时间隔到，FALSE 定时间隔未到
***************************************************************/
BOOL CProtocol::TimerOn(DWORD TimeOut)
{ 
	if(TimeOut == 0) return FALSE;
	return ((m_dwTimerCount % TimeOut) == 0); 
} 


/***************************************************************
	Function:DoMessage
		消息处理函数
	参数：无
	
	返回：无
***************************************************************/
BOOL CProtocol::DoMessage(void)
{
	BYTE byMsgAttr,msgId;
	BOOL ret = FALSE;

    msgId = m_pMsg->Head.byMsgID;
	byMsgAttr = m_pMsg->Head.byMsgAttr;
	
    switch (msgId)
    {
      case MI_YKSELECT:
      case MI_YKOPRATE:
      case MI_YKCANCEL:
        if (byMsgAttr==MA_REQ)  
          ret = DoYKReq();
        else if (byMsgAttr==MA_RET)  
          ret = DoYKRet();
        break;
	  case MI_YTSELECT:
      case MI_YTOPRATE:
      case MI_YTCANCEL:
        if (byMsgAttr==MA_REQ)  
          ret = DoYTReq();
        else if (byMsgAttr==MA_RET)  
          ret = DoYTRet();
        break;
      /*case MI_SIM:
      	if (byMsgAttr!=MA_SIM_YK)  DoFaSim();
      	break;*/
	  case MI_ROUTE:
	  	DoTSDataMsg();
	  	break;
	  case MI_PARA:
	  	DoParaMsg();
	  	break;
  	  default:
		break;
    }    

	return ret;    //当返回TRUE时，只有在当前消息处理完毕才接受下一条消息
}


/***************************************************************
	Function:OnReadCommEvent
		通信端口读事件处理
	参数：无
	
	返回：无
***************************************************************/
void CProtocol::DoReadCommEvent(void)
{
	  BOOL ok;
	ReadFromComm();
    while(m_dwHaveRxdFm)
    {
     	ok = DoReceive();

        if (ok && (m_wTaskID != MMI_ID))  
				{
					commid = 1;
				}
    }
}

/***************************************************************
	Function:OnWriteCommEvent
		通信端口写事件处理，暂空
	参数：无
	
	返回：无
***************************************************************/
void CProtocol::DoWriteCommEvent(void)
{
	WriteToComm(m_SendBuf.dwFlag);
}

void CProtocol::DoTimeOut(void)
{
	thClearDog(m_wTaskID, COMMONDOGTM);
	m_dwTimerCount++; 
}

DWORD CProtocol::DoCommState(void)
{
	DWORD dwCommState;
	
	commCtrl(m_wCommID, CCC_CONNECT_CTRL | CONNECT_STATUS_GET, (BYTE*)&dwCommState);
		
	CommStatusProcByRTNoErrCnt((BOOL)dwCommState);

	return(dwCommState);
}

/***************************************************************
	Function:Run
		规约任务主体函数
	参数：无
	
	返回：无
***************************************************************/
void CProtocol::Run(void)
{
	int num;
	DWORD evFlag;
		
	tmEvEvery(m_wTaskID, SECTOTICK(1), EV_TM1);  //pre 1 second
	
	for(;;)
	{
		evReceive(m_wTaskID, EV_UFLAG | EV_MSG  | EV_RX_AVAIL 
			| EV_TX_AVAIL | EV_TX_IDLE | EV_COMM_IDLE | EV_COMM_STATUS|EV_TM1| EV_SUSPEND
			| EV_COMMNO_MAP | EV_DATAREFRESH | EV_CLEAR_SOE, &evFlag);

        if(evFlag & EV_COMMNO_MAP)
        {
           m_wCommID = GetThCommNo(m_wTaskID);
        }

		if (evFlag & EV_UFLAG)
		{
			DoUrgency();
		}
		
		if (evFlag & EV_MSG)
		{
			for (; ;)
			{
				num = msgReceive(m_wTaskID, (BYTE *)m_pMsg, MAXMSGSIZE, OS_NOWAIT);	
				if (num > 0)
				{
					if (DoMessage() == TRUE) break;
				}
				else
					break;
			}	 
		}

		if (evFlag & EV_TM1)
		{
			DoTimeOut();
		}
		
		if (evFlag & EV_RX_AVAIL)
		{
			DoReadCommEvent();
		}
		
		if (evFlag & EV_TX_IDLE)
		{
			DoCommSendIdle();
		}
		
		if (evFlag & EV_COMM_IDLE)
		{
			DoCommIdle();
		}

		if (evFlag & EV_TX_AVAIL)
		{
			//DoWriteCommEvent();
		}
		
		if (evFlag & EV_COMM_STATUS)
		{
			DoCommState();
		}

		if (evFlag & EV_SUSPEND)
		{
			thSuspend(m_wTaskID);
		}

		if (evFlag & EV_DATAREFRESH)
		{
			DoDataRefresh();
		}	
		
		if (evFlag & EV_CLEAR_SOE)
		{
			DoClearSoe();
		}
	}
}

BOOL CProtocol::InitEqpInfo(WORD wMinEqpNum)
{
    return FALSE;
}

void CProtocol::CommStatusProcByRT(BOOL bCommOk)
{}

void CProtocol::CommStatusProcByRTNoErrCnt(BOOL bCommOk)
{}


void CProtocol::CheckCfg(void)
{}

void CProtocol::SetDefCfg(void)
{}

DWORD CProtocol::SearchOneFrame(BYTE *Buf,WORD Len)
{
   return FRAME_OK|Len;
}

BOOL CProtocol::DoReceive(void)
{ 
   return TRUE; 
}

BOOL CProtocol::DoSend(void)
{ 
   return TRUE; 
}

BOOL CProtocol::SwitchToAddress(WORD wAddress)
{
   return FALSE;
}		

void CProtocol::DoCommIdle(void)
{ 
    DoSend();
}

void CProtocol::DoCommSendIdle(void)
{
    DoSend();
}

BOOL CProtocol::DoYKReq(void)
{
    return FALSE;
}	

BOOL CProtocol::DoYKRet(void)
{
    return FALSE;
}

BOOL CProtocol::DoYTReq(void)
{
    return FALSE;
}	

BOOL CProtocol::DoYTRet(void)
{
    return FALSE;
}

BOOL CProtocol::DoFaSim(void)
{
    return FALSE;
}	

WORD CProtocol::GetEqpAddr(void)
{
    return 0;
}

WORD CProtocol::GetOwnAddr(void)
{
    return 0;
}

void CProtocol::DoUrgency(void)
{}

void CProtocol::DoDataRefresh(void)
{}

void CProtocol::DoTSDataMsg(void)
{}

void CProtocol::DoParaMsg(void)
{}

void CProtocol::DoClearSoe(void)
{}
#endif

