#include "syscfg.h"

#ifdef INCLUDE_FA_SH

#include "sys.h"
#include "gb104shs.h"

extern "C" void gb104shs(WORD wTaskID)
{
	CGB104Shs *pGB104S = new CGB104Shs(); 	
	
	if (pGB104S->Init(wTaskID) != TRUE)
 	{
		pGB104S->ProtocolExit();
	}
	pGB104S->Run();			   
}

CGB104Shs::CGB104Shs() : CPSecondary()
{
	m_wSendNum = 0;	     
	m_wRecvNum = 0;	     
	m_wAckNum = 0;		 
	m_wAckRecvNum = 0;   
	m_bDTEnable = FALSE;		   
	m_bTcpConnect = FALSE;			
	m_bContinueSend = TRUE;
	m_vTimer[1].bRun = FALSE;
	m_vTimer[1].wInitVal = PARA_T1;
	m_vTimer[2].bRun = FALSE;
	m_vTimer[2].wInitVal = PARA_T2;
	m_vTimer[3].bRun = FALSE;
	m_vTimer[3].wInitVal = PARA_T3;
	memset(m_vBackFrame, 0xff, sizeof(m_vBackFrame));
	
	m_dwYcSendFlag = 0;
	m_dwYxSendFlag = 0;
	
	m_wSendYcNum = 0;
	m_wSendYxNum = 0;
	m_wSendDYxNum = 0;
	
	m_dwYcGroupFlag = 0;
	m_dwYxGroupFlag = 0;
	
	m_eqpflag[0] = FAIPOWERUFLAGEQP;
	m_eqpflag[1] = FAINEEDUFLAGEQP;
	m_flag[0] = FAIPOWERUFLAG;
	m_flag[1] = FAINEEDUFLAG;

	m_dwSendAllDataEqpNo = 0;
	m_wRecAddress = 0;
	m_wChangeYcEqpNo = 0;
	m_allcalloverflag = 0;

}

/***************************************************************
	Function：Init
		规约初始化
	参数：wTaskID
		wTaskID 任务ID 
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CGB104Shs::Init(WORD wTaskID)
{
	BOOL rc;
	memset(&m_guiyuepara,0,sizeof(m_guiyuepara));
	rc = CPSecondary::Init(wTaskID,1,&m_guiyuepara,sizeof(m_guiyuepara));
	if (!rc)
	{
		return FALSE;
	}

	m_sourfaaddr = 0;
	m_allcallflag = 0;

	m_pReceive = (VIec104Frame *)m_RecFrame.pBuf;
	m_pSendsoenum = new VSendsoenum[m_wEqpNum];
	DoCommState();
	m_initflag = 1;
	m_retxdflag = 0;
	for (int i = 0; i < m_wEqpNum; i++)
	{
		m_pEqpInfo[i].pProtocolData = new VCtrlInfo;
		m_pSendsoenum[i].wSendSSOENum = 0;
		m_pSendsoenum[i].wSendDSOENum = 0;
	}
	initpara();
	SwitchToEqpNo(0);
	QuerySSOE(m_wEqpID, 1, 1, 1000,(VDBSOE *)m_dwPubBuf, &m_WritePtr, &m_BufLen);
	m_ReadPtr = m_WritePtr;

	event_time = 50;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	return TRUE;
}

void CGB104Shs::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();
		
    if (m_pBaseCfg->Timer.wScanData2 <= 1000)
		m_pBaseCfg->Timer.wScanData2 = 1000;
	if (m_pBaseCfg->Timer.wScanData2 > 6000) m_pBaseCfg->Timer.wScanData2 = 1000;
	
	m_pBaseCfg->Timer.wScanData2 /= TICK2MSE;	
}

void CGB104Shs::SetBaseCfg(void)
{
	CProtocol::SetBaseCfg();
}

void CGB104Shs::StartTimer(BYTE TimerNo)
{
	if (TimerNo>3)
   		return;
  	m_vTimer[TimerNo].wCounter = m_vTimer[TimerNo].wInitVal*10000;
  	m_vTimer[TimerNo].bRun = TRUE;	
}

void CGB104Shs::StopTimer(BYTE TimerNo)
{
	if (TimerNo > 3)
   		return;
  	m_vTimer[TimerNo].bRun = FALSE;	
}

void CGB104Shs::DoTimeOut(void)
{
    CPSecondary::DoTimeOut();
	DWORD msd;
	
	msd = Get100usCnt();
	if(msd - tmpms > 100000)
		tmpms = msd;
	
	msd = msd - tmpms;
	
	tmpms = Get100usCnt();
	if(startdelay)
   		startdelay--;
	if (m_vTimer[1].bRun)
   	{	
    	if (m_vTimer[1].wCounter <= msd)
     	{  
      		CloseTcp();     		
      		StopTimer(1);
     	}
		else
    		m_vTimer[1].wCounter -= msd;
			
   	}
  	if (m_vTimer[2].bRun)
   	{
    	if (m_vTimer[2].wCounter <= msd)
     	{
      		SendSFrame();
      		StopTimer(2);
		}
  		else
     		m_vTimer[2].wCounter -= msd;
 	}
		
  	if (m_vTimer[3].bRun)
   	{
 		if (m_vTimer[3].wCounter <= msd)
     	{
	 		if (!CanSendIFrame())
			    return;
	    	SendUFrame(TESTFR_ACT);
	      	StopTimer(3);
		}
		else
  	   		m_vTimer[3].wCounter -= msd;
   	}  
	
}

int CGB104Shs::atOnceProcSCOS(WORD wEqpNo)
{
	if(m_initflag) return 0;

	if(m_allcallflag) return 0;
	
    SwitchToEqpNo(wEqpNo);

	if(SendClassOneData()) return 1;

	return 0;	
}

int CGB104Shs::atOnceProcDCOS(WORD wEqpNo)
{
	if(m_initflag) return 0;

	SwitchToEqpNo(wEqpNo);

	if(SendDCOS()) return 1;

	return 0;	
}

void CGB104Shs::DoCommSendIdle(void)
{
	event_time = 0;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_OFF, &event_time);

	if(m_initflag)
	{
		if(m_allcallflag)
		{
			m_allcalloverflag = 0;
			SendInitFinish();
		}
		return;
	}

	if(startdelay)
		return;
	
	if(m_retxdflag)
	{
		m_retxdflag = 0;
		WriteToComm(m_EqpGroupInfo.wDesAddr);
		return;
	}
	if (m_dwYcSendFlag || m_dwYxSendFlag)
	{
		SendAllYcYx();
		return;	
	}
	if(m_allcalloverflag == 0) return;

	if(SendSingleSoe())  
		return;
	if (CheckClearTaskFlag(TASKFLAG_DSOEUFLAG))
	{
		for (WORD wSoeEqpNo = 0; wSoeEqpNo < m_wEqpNum; wSoeEqpNo++)
	    {
	    	SwitchToEqpNo(wSoeEqpNo);
  			if (CheckClearEqpFlag(EQPFLAG_DSOEUFLAG))
  			{
  				if(SendDSoe()) 
					return;
	    	}		
	    }
	}
	if (m_dwYcGroupFlag || m_dwYxGroupFlag)
	{
		SendSomeYcYx();
		return;	
	}

	SendChangeYC();      

	event_time = m_pBaseCfg->Timer.wScanData2;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
}

void CGB104Shs::CloseTcp(void)
{
	StopTimer(1);
	StopTimer(2);
	StopTimer(3);
	
	m_bDTEnable = FALSE;
	m_bTcpConnect = FALSE;
	
	m_wSendNum = 0; 
	m_wRecvNum = 0; 
	m_wAckNum = 0;		 
	m_wAckRecvNum = 0;  

	commCtrl(m_wCommID, CCC_CONNECT_CTRL | CONNECT_CLOSE, &event_time); 
}

DWORD CGB104Shs::DoCommState(void)
{
	BOOL bTcpState = FALSE;
	DWORD dwCommState;
	VSysClock SysTime;
	
	dwCommState = CPSecondary::DoCommState();
	if (dwCommState == ETHERNET_COMM_ON)
		bTcpState = TRUE;
	else
		bTcpState = FALSE;
	
	StopTimer(1);
	StopTimer(2);
	StopTimer(3);
	for (int i = 0; i < m_wEqpNum; i++)
	{
		m_pSendsoenum[i].wSendSSOENum = 0;
		m_pSendsoenum[i].wSendDSOENum = 0;
	}
	
	GetSysClock((void *)&SysTime, SYSCLOCK);	

	if (bTcpState)
	{
		m_bTcpConnect = TRUE;
		m_wSendNum = 0;		
	    m_wRecvNum = 0;	
	    m_wAckNum = 0;		
	    m_wAckRecvNum = 0;
	}
	else
	{
		m_bTcpConnect = FALSE;
		m_bDTEnable = FALSE;
	}
	
	m_dwYcSendFlag = 0;
	m_dwYxSendFlag = 0;
	m_wSendYcNum = 0;
	m_wSendYxNum = 0;
	m_wSendDYxNum=0;
	
	m_dwYcGroupFlag = 0;
	m_dwYxGroupFlag = 0;

    return(dwCommState);	
}


BOOL CGB104Shs::DoReceive()
{
	m_SendBuf.wWritePtr = 0;
	m_SendBuf.wReadPtr = 0;

	if (SearchFrame() != TRUE)
	{
		return FALSE;//没找到有效报文
	}

	startdelay = 0;
	m_pReceive = (VIec104Frame *)m_RecFrame.pBuf;
	
	m_pASDU = (VASDU *)&m_pReceive->byASDU;
	m_pASDUInfo = (BYTE *)&m_pASDU->byInfo;
	StopTimer(1);	
	
	StartTimer(3);  //receive any I,U,S frame, to start T3
  
	for (int i = 0; i < m_wEqpNum; i++)
	{
		if(m_pSendsoenum[i].wSendDSOENum > 16)
			m_pSendsoenum[i].wSendDSOENum = 0;

		SwitchToEqpNo(i);

		if(m_pSendsoenum[i].wSendSSOENum)
			ReadPtrIncrease(m_wTaskID, m_wEqpID, m_pSendsoenum[i].wSendSSOENum, SSOEUFLAG);

		m_pSendsoenum[i].wSendSSOENum = 0;

		if(m_pSendsoenum[i].wSendDSOENum)
			ReadPtrIncrease(m_wTaskID, m_wEqpID, m_pSendsoenum[i].wSendDSOENum, DSOEUFLAG);

		m_pSendsoenum[i].wSendDSOENum = 0;		
	}

    switch (m_pReceive->byControl1 & 0x03)
    {
    	case FRM_I:
    		DoIFrame();
    		break;
    	case FRM_S:
    		DoSFrame();
    		break;
    	case FRM_U:
    		DoUFrame();
    		break;
    	default:
    		DoIFrame();
    		break;	
    }

	if (((m_wSendNum + 1) - m_wAckNum) >= m_PARA_K)
   	{
    	m_bContinueSend = FALSE;    
    }
    else
    {
        m_bContinueSend = TRUE;    
    }

	return TRUE;	
}


DWORD CGB104Shs::SearchOneFrame(BYTE *Buf, WORD Len)
{
	DWORD Rc;
	BYTE FrameLen;
	
	m_pReceive = (VIec104Frame *)Buf;
		
    if (Len < MIN_RECEIVE_LEN)
    	return FRAME_LESS;
	  	
    Rc = SearchHead(Buf,Len,0,START_CODE);
    if (Rc)
       return FRAME_ERR|Rc;
	FrameLen = m_pReceive->byAPDULen;
	
	if (FrameLen < 4)
		return FRAME_ERR | (1);
    
    if (FrameLen > (MAX_FRAME_LEN - 2))
		return FRAME_ERR|(1);
    
    if ((FrameLen + 2) > (Len))
		return FRAME_LESS;
    
    m_pASDU = (VASDU *)&m_pReceive->byASDU;
    
    if (FrameLen > (CONTROL_LEN + ASDUID_LEN))
    {
    	m_wRecAddress = MAKEWORD(m_pASDU->byAddressLow, m_pASDU->byAddressHigh);
	}
	
	return FRAME_OK | (FrameLen + 2); //返回本帧总长度
}


void CGB104Shs::DoIFrame(void)
{
	WORD wAckNum, wRecvNum, wEqpAddr;
	
	StartTimer(2);	//when receive I frame,start T2
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);
	wRecvNum = MAKEWORD(m_pReceive->byControl1, m_pReceive->byControl2);
	wAckNum >>= 1;
	wRecvNum >>= 1;
	
	m_wRecvNum = wRecvNum + 1;
	
	m_wAckNum = wAckNum;           
		
	StopTimer(1);	//stop T1	
	
	if ((m_wRecvNum - m_wAckRecvNum) > m_PARA_W)
	{
	    SendSFrame();       
	}        
	
	getasdu();
    GotoEqpbyAddress(m_dwasdu.Address);
	if(m_dwasdu.Address != GetEqpOwnAddr())
	{
		errack(46);
		return;
	}
	m_wRecAddress = m_dwasdu.Address;
	switch (m_dwasdu.TypeID)	
	{
	    case C_IC_NA:  
	        DoCallYcYxData();
	        break;
	    case C_CS_NA:     //set time
	        DoSetTime();
	        break; 	  
		default:
			errack(44);
			break;
	}
}


void CGB104Shs::DoSFrame(void)
{
	WORD wAckNum;
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);	
	m_wAckNum = wAckNum >> 1;
	
	StopTimer(1);
	event_time=50;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	
	return;
}


void CGB104Shs::DoUFrame(void)
{
	DWORD para=1;
	switch (m_pReceive->byControl1)
	{
	    case STARTDT_ACT:
	        m_bDTEnable = TRUE;
			startdelay = 2;
			cosnum = 0;
			soenum = 0;
	      
			m_wSendNum = 0;	
			m_wRecvNum = 0;	
			m_wAckNum = 0;		
			m_wAckRecvNum = 0; 
	        
	        m_bContinueSend = TRUE;
	        evSend(m_wTaskID, EV_UFLAG);
			VDBCOS Cos;
			WORD RecCosNum;
			memset(Bitchange,0,256);
			while(1)
			{
				RecCosNum = ReadSCOS(m_wTaskID, m_wEqpID, 1,sizeof(VDBCOS), (VDBCOS *)&Cos);
				if (RecCosNum == 0)
					break;

				ReadPtrIncrease(m_wTaskID, m_wEqpID, RecCosNum, SCOSUFLAG);
				if(Cos.wNo > 0 && Cos.wNo < 256 * 8)
				{
					Bitchange[Cos.wNo / 8] |= 1 << (Cos.wNo % 8);
				}
			}
	
			VDBDCOS Cos1;
			while(1)
			{
				RecCosNum = ReadDCOS(m_wTaskID, m_wEqpID, 1,sizeof(VDBDCOS), (VDBDCOS *)&Cos1);
				if (RecCosNum == 0)
					break;
				ReadPtrIncrease(m_wTaskID, m_wEqpID, RecCosNum, DCOSUFLAG);
			}
			SendUFrame(STARTDT_CON);
			m_initflag = 1;
			m_allcallflag = 1;
	        break;
	    case STOPDT_ACT:
	    	StopTimer(1);
			StopTimer(2);
			StopTimer(3);
	        m_bDTEnable = FALSE;
	        SendUFrame(STOPDT_CON);
	        break;
	    case TESTFR_ACT:
			if (!CanSendIFrame())
				return;
	        SendUFrame(TESTFR_CON);
	        break;
	    case TESTFR_CON:
	        StopTimer(1);
	        break;    
		case REMOTE_MAINT:
			para = MAINT_ID;
			commCtrl(m_wCommID, CCC_TID_CTRL, &para);
			break;
	}
	return;
}

void CGB104Shs::DoCallYcYxData(void)
{
	if(m_dwasdu.VSQ != 1)
	{
		return errack(m_dwasdu.COT+1);
	}
	if((m_dwasdu.COT != 6))
	{
		return errack(45);
	}
	if((m_dwasdu.Info != 0))
	{
		return errack(47);
	}
	m_QOI = m_pReceive->byASDU[m_dwasdu.Infooff+m_guiyuepara.infoaddlen];
	if(m_initflag)
	{
		SendInitFinish();
	}

	SendCallYcYxAck();

	switch (m_dwasdu.COT)
	{
		case COT_ACT:		//总召唤激活，应答所有数据 包括遥测、遥信，不包括电度
			if(m_QOI==COT_INTROGEN)
			{
				m_dwYcSendFlag = 0xffffffff;
				m_dwYxSendFlag = 0xffffffff;
			}
			else if((m_QOI >= COT_INRO1) && (m_QOI <= COT_INRO8))
			{
				m_dwYxGroupFlag = m_dwYxGroupFlag | (1 << (m_QOI - COT_INRO1)) | 0x80000000;
			}
			else if((m_QOI >= COT_INRO9) && (m_QOI <= COT_INRO12))
			{
				m_dwYcGroupFlag = m_dwYcGroupFlag | (1 << (m_QOI - COT_INRO9)) | 0x80000000;
			}
			
			break;
		case COT_DEACT://停止激活
			if(m_QOI == COT_INTROGEN)
			{
				m_dwYcSendFlag = 0;
				m_dwYxSendFlag = 0;
			}
			else if((m_QOI >= COT_INRO1) && (m_QOI <= COT_INRO8))
			{
				m_dwYxGroupFlag = 0;
			}
			else if((m_QOI >= COT_INRO9) && (m_QOI <= COT_INRO12))
			{
				m_dwYcGroupFlag = 0;
			}
			break;
	}
	
    return;
}

void CGB104Shs::DoSetTime(void)
{
	BYTE * pData = m_pASDU->byInfo;
	BYTE TypeID = C_CS_NA;
	BYTE Reason = COT_ACTCON;
	VSysClock SysTime;	
	WORD MSecond;

	if(m_dwasdu.VSQ != 1)
	{
		return errack(m_dwasdu.COT+1);
	}
	if((m_dwasdu.COT != 6))
	{
		return errack(45);
	}
	if((m_dwasdu.Info != 0))
	{
		return errack(47);
	}
	
	pData = &m_pReceive->byASDU[m_dwasdu.Infooff+m_guiyuepara.infoaddlen];
	MSecond = MAKEWORD(pData[0], pData[1]); 
	SysTime.wMSecond = MSecond % 1000;
	SysTime.bySecond  = MSecond / 1000;
	SysTime.byMinute = pData[2];
	SysTime.byHour = pData[3] &0x1F;
	SysTime.byDay = pData[4] & 0x1F;
	SysTime.byWeek = pData[4] >> 5;
	SysTime.byMonth = pData[5] & 0x0F;
	SysTime.wYear = (pData[6] & 0x7F) + m_guiyuepara.baseyear;
	if (ClockIsOk(&SysTime) == ERROR)
		return errack(m_dwasdu.COT+1);

	SetSysClock((void *)&SysTime, SYSCLOCK);
	
	SendFrameHead(FRM_I);
	
	Sendframe(TypeID, Reason);
	
	write_infoadd(0);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (SysTime.wMSecond+SysTime.bySecond*1000)&0xff;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (SysTime.wMSecond+SysTime.bySecond*1000)>>8;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byHour;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byDay|(SysTime.byWeek<<5);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMonth;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.wYear%100;
	write_VSQ(1);

	SendFrameTail();
    return;
}

void CGB104Shs::errack(BYTE COT)
{
	int num;
	SendFrameHead(FRM_I);
	Sendframe(m_dwasdu.TypeID, (COT|0x40));
	
	num = m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
	if(num < 0)   return;
	memcpy(m_SendBuf.pBuf + m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,num);
	m_SendBuf.wWritePtr += m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
	write_VSQ(m_dwasdu.VSQ);

	SendFrameTail();
    return;
}

void CGB104Shs::SendSFrame(void)
{
    SendFrameHead(FRM_S);
    SendFrameTail();
    StopTimer(2);	//when send S frame, stop T2
    return;    
}

void CGB104Shs::SendUFrame(BYTE byControl)
{
    SendFrameHead(FRM_U);
    m_pSend->byControl1 = byControl;
    SendFrameTail();
    if ((byControl == TESTFR_ACT) || (byControl == STARTDT_ACT) || (byControl == STOPDT_ACT))
    {
    	if(m_vTimer[1].bRun == FALSE)
    		StartTimer(1);		//statr T1	
    }
    return;    
}

void CGB104Shs::SendFrameHead(BYTE byFrameType)
{
    WORD wSendNum, wRecvNum;
    
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;//清空发送缓冲区 
	
	m_pSend = (VIec104Frame *)m_SendBuf.pBuf;
	
	m_wSendNum %= MAX_FRAME_COUNTER;
	m_wRecvNum %= MAX_FRAME_COUNTER;
	wSendNum = m_wSendNum << 1;
	wRecvNum = m_wRecvNum << 1;
	
	m_pSend->byStartCode = START_CODE;
	
	switch (byFrameType)
	{
	    case FRM_I:
			
			if(m_vTimer[1].bRun == FALSE)
	    		StartTimer(1);		//statr T1	
	        m_pSend->byControl1 = LOBYTE(wSendNum);
	        m_pSend->byControl2 = HIBYTE(wSendNum);
	        m_pSend->byControl3 = LOBYTE(wRecvNum);
	        m_pSend->byControl4 = HIBYTE(wRecvNum);
	        m_wAckRecvNum = m_wRecvNum; 
	        if (((m_wSendNum + 1) - m_wAckNum) >= m_PARA_K)
	        {
	            m_bContinueSend = FALSE;    
	        }
	        else
	        {
	            m_bContinueSend = TRUE;    
	        }
	        break;
	    case FRM_S:
	        m_pSend->byControl1 = FRM_S;
            m_pSend->byControl2 = 0;
            m_pSend->byControl3 = LOBYTE(wRecvNum);
            m_pSend->byControl4 = HIBYTE(wRecvNum);
            m_wAckRecvNum = m_wRecvNum;
	        break;
	    case FRM_U: 
	        m_pSend->byControl1 = FRM_U;
            m_pSend->byControl2 = 0;
            m_pSend->byControl3 = 0;
            m_pSend->byControl4 = 0;   
	        break;
	    default:
	        break;
	}
    m_SendBuf.wWritePtr = APCI_LEN;
	return;
}

int CGB104Shs::SendFrameTail(void)
{
	int len;
	m_pSend->byAPDULen = m_SendBuf.wWritePtr - 2;
	
	len = WriteToComm(m_EqpGroupInfo.wDesAddr);
	if(len < 1)
		m_retxdflag = 1;

	event_time = 50;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);

	if ((m_pSend->byControl1 & 0x1) == FRM_I)
	{
		m_wSendNum ++;
		StopTimer(2);
	}

	return len;
}

BOOL CGB104Shs::SendClassOneData(void)
{
	WORD ReadNum = REQ_SCOS_NUM;
	WORD wBufLen = ASDUINFO_LEN;
	WORD RecCosNum = 0;
	VDBCOS *pRecCOS = NULL;
	
	BYTE TypeID = M_SP_NA;
	BYTE Reason = COT_SPONT;
	
	if (!CanSendIFrame())
		return FALSE;
  			
  	ClearTaskFlag(TASKFLAG_SCOSUFLAG);
  	ClearEqpFlag(EQPFLAG_SCOSUFLAG);
  			
    RecCosNum = ReadSCOS(m_wTaskID, m_wEqpID, ReadNum,wBufLen, (VDBCOS *)m_dwPubBuf);
	if (RecCosNum == 0)
		return FALSE;							
	
	SendFrameHead(FRM_I);

  	if(m_guiyuepara.yxtype == 3)
		TypeID = M_DP_NA;

	Sendframe(TypeID,Reason);
	pRecCOS = (VDBCOS *)m_dwPubBuf;                              																																															
    		                                                             																																															
    for (int i = 0; i < RecCosNum; i++)                            																																															
    {   
		write_infoadd(pRecCOS->wNo + ADDR_YX_LO+m_pEqpInfo[m_wEqpNo].wDYXNum);
		if(m_guiyuepara.yxtype == 3)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pRecCOS->byValue)+1;
		else
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pRecCOS->byValue);

		pRecCOS ++;                                              																																															
   	}  
	
	write_VSQ(RecCosNum);
		
	SendFrameTail();
	
	m_pEqpExtInfo[m_wEqpNo].wSendSCOSNum = RecCosNum;                  
	SetTaskFlag(TASKFLAG_SENDSCOS);
	SetEqpFlag(EQPFLAG_SENDSCOS);
	ClearUFlag(m_wEqpNo);	
	cosnum += RecCosNum;
	
	return TRUE;
}

BOOL CGB104Shs::SendDCOS(void)
{
	WORD ReadNum = REQ_SCOS_NUM;
	WORD wBufLen = ASDUINFO_LEN;
	WORD RecCosNum = 0;
	VDBDCOS *pRecCOS = NULL;
	
	BYTE TypeID = M_DP_NA;
	BYTE Reason = COT_SPONT;
	
	if (!CanSendIFrame())
		return FALSE;
  	
  	ClearTaskFlag(TASKFLAG_DCOSUFLAG);
  	ClearEqpFlag(EQPFLAG_DCOSUFLAG);
  			
    RecCosNum = ReadDCOS(m_wTaskID, m_wEqpID, ReadNum,wBufLen, (VDBDCOS *)m_dwPubBuf);
	if (RecCosNum == 0)
		return FALSE;							

	SendFrameHead(FRM_I);

	Sendframe(TypeID,Reason);
	pRecCOS = (VDBDCOS *)m_dwPubBuf;                              																																															
    		                                                             																																															
    for (int i = 0; i < RecCosNum; i++)                            																																															
    {   
 		write_infoadd(pRecCOS->wNo + ADDR_YX_LO);
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = DIQ(pRecCOS->byValue1,pRecCOS->byValue2);                           																																															
   		pRecCOS ++;                                              																																															
   	}                   
	write_VSQ(RecCosNum);
	m_pEqpExtInfo[m_wEqpNo].wSendDCOSNum = RecCosNum;                  
	SetTaskFlag(TASKFLAG_SENDDCOS);
	SetEqpFlag(EQPFLAG_SENDDCOS);
	ClearUFlag(m_wEqpNo);
			
	SendFrameTail();
			
	return TRUE;
}

void CGB104Shs::Sendframe(BYTE type,WORD cot)
{
	write_typeid(type);
	write_COT((GetOwnAddr() << 8)| cot);
	write_conaddr(GetEqpOwnAddr());
	if(cot == COT_SPONT)
		if(m_vTimer[1].bRun == FALSE)
			StartTimer(1);		//statr T1
}

BOOL CGB104Shs::SendSingleSoe(void)
{
	WORD ReadNum = REQ_SSOE_NUM;
	WORD RecSoeNum = 0;
	VDBSOE *pRecSOE = NULL;
    VSysClock SysTime;  
	BYTE TypeID = M_SP_TB;
	BYTE Reason = COT_SPONT;
	
	if (!CanSendIFrame())
	    return FALSE;

	if (m_WritePtr == m_ReadPtr)
	    QuerySSOE(m_wEqpID, 0, m_ReadPtr, 1000,(VDBSOE *)m_dwPubBuf, &m_WritePtr, &m_BufLen);
	
	if(m_WritePtr < m_ReadPtr)
		RecSoeNum = m_WritePtr + m_BufLen - m_ReadPtr;
	else
		RecSoeNum = m_WritePtr - m_ReadPtr;

	if(ReadNum > RecSoeNum)
		ReadNum = RecSoeNum;

	RecSoeNum = QuerySSOE(m_wEqpID, ReadNum, m_ReadPtr, 1000,(VDBSOE *)m_dwPubBuf, &m_WritePtr, &m_BufLen);

	if (RecSoeNum == 0)                                           
	    return FALSE;

	m_ReadPtr += RecSoeNum;
	m_ReadPtr %= m_BufLen;
	pRecSOE = (VDBSOE *)m_dwPubBuf;  

	SendFrameHead(FRM_I);
	if(m_guiyuepara.yxtype == 2) 
		TypeID = 2;        //短时标单点遥信
	if(m_guiyuepara.yxtype==3) 
		TypeID = 31;        //短时标单点遥信

	Sendframe(TypeID, Reason);
		
	for (int i=0; i < RecSoeNum; i++)  
	{    
		write_infoadd(pRecSOE->wNo + ADDR_YX_LO+m_pEqpInfo[m_wEqpNo].wDYXNum);
		if(m_guiyuepara.yxtype == 3)
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SIQ(pRecSOE->byValue )+1;
		else
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pRecSOE->byValue);

		SystemClock(pRecSOE->Time.dwMinute, &SysTime);                      
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(pRecSOE->Time.wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(pRecSOE->Time.wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMinute;   
		if(TypeID != 2)//短时标单点遥信 
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byHour & 0x1F;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SysTime.byDay & 0x1F) | ((SysTime.byWeek <<5) & 0xE0);    
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMonth;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.wYear % 100;
		}
		pRecSOE ++;      
	
	}                 
	
	write_VSQ(1);
	     
	write_VSQ(RecSoeNum);
		
	SendFrameTail();
	soenum += RecSoeNum;
	
	return TRUE;                                        	                            	
}
BOOL CGB104Shs::SendDSoe(void)
{
	WORD ReadNum = REQ_SSOE_NUM;
	WORD wBufLen = ASDUINFO_LEN;
	WORD RecSoeNum = 0;
	VDBDSOE *pRecSOE = NULL;
    VSysClock SysTime;   
	BYTE TypeID = M_DP_TB;
	BYTE Reason = COT_SPONT;
	
	if (!CanSendIFrame())
		return FALSE;
	
	RecSoeNum = ReadDSOE(m_wTaskID, m_wEqpID, ReadNum, wBufLen, (VDBDSOE *)m_dwPubBuf);        
	pRecSOE = (VDBDSOE *)m_dwPubBuf;  

	if(m_pSendsoenum[m_wEqpNo].wSendDSOENum > RecSoeNum)
		m_pSendsoenum[m_wEqpNo].wSendDSOENum = 0;
	
	pRecSOE += m_pSendsoenum[m_wEqpNo].wSendDSOENum;
	RecSoeNum -= m_pSendsoenum[m_wEqpNo].wSendDSOENum;
	
	if (RecSoeNum == 0)                                              
	    return FALSE;

	SendFrameHead(FRM_I);
	if(m_guiyuepara.yxtype == 2) 
		TypeID = 4;

	Sendframe(TypeID, Reason);
	                  
	write_infoadd(pRecSOE->wNo + ADDR_YX_LO);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = DIQ(pRecSOE->byValue1,pRecSOE->byValue2);
	SystemClock(pRecSOE->Time.dwMinute, &SysTime);                      
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(pRecSOE->Time.wMSecond);   
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(pRecSOE->Time.wMSecond);   
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMinute;   
	if(TypeID != 4) 
	{
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byHour & 0x1F;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SysTime.byDay & 0x1F) | ((SysTime.byWeek <<5) & 0xE0);    
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMonth;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.wYear % 100;
	}
	pRecSOE ++;    
	m_pSendsoenum[m_wEqpNo].wSendDSOENum++;
	            
    write_VSQ(1);
	RecSoeNum = 0;
	SendFrameTail();
	
	return TRUE;                                        	                            	

}

int CGB104Shs::SearchSendYc(WORD wNum, WORD wBufLen, VDBYCF_L *pBuf, BOOL bActive)
{
     int i, j;
     VYCF_L ycbuf[3];
	 VDBYCF_L *pSendYC = pBuf;
     int sendyc = 0;

	 for (i=0; i<2; i++)
	 {	
	    if (GetEqpFlag(m_wChangeYcEqpNo, m_eqpflag[i]))
	    {
	        ::ReadRangeYCF_L(pGetEqpInfo(m_wChangeYcEqpNo)->wEqpID, i*3, 3, 3*sizeof(VYCF_L) , ycbuf);
			for (j=0; j<3; j++)
		    {
	             pSendYC->wNo=i*3+j;
			     pSendYC->lValue=ycbuf[j].lValue;
			     pSendYC->byFlag=ycbuf[j].byFlag;
			     pSendYC++;	
		    } 
		    sendyc += 3;

			ClearEqpFlag(m_wChangeYcEqpNo, m_eqpflag[i]);
	    }

	 }

	 return sendyc;
}

void CGB104Shs::SendChangeYC(void)
{
	VDBYCF_L *pBuf = (VDBYCF_L *)m_dwPubBuf;
	WORD wReqNum = REQ_CHANGEYC_NUM;
	WORD wChangeYcNum = 0;
	float fd;
	DWORD dddd;
	BYTE Reason = COT_SPONT;
	int i;
	
	if (!CanSendIFrame())
		return;
	
	SwitchToEqpNo(m_wChangeYcEqpNo);
	wChangeYcNum = SearchSendYc(wReqNum, MAX_PUBBUF_LEN *4, (VDBYCF_L*)pBuf,0);
	if (!wChangeYcNum)
	{
		m_wChangeYcEqpNo++;
		if (m_wChangeYcEqpNo >= m_wEqpNum)
		{
			m_wChangeYcEqpNo = 0;	
		}
		return;
	}
		
	SendFrameHead(FRM_I);
	Sendframe(m_guiyuepara.yctype, Reason);
	for (i=0; i < wChangeYcNum; i++)
	{
		write_infoadd(pBuf->wNo + ADDR_YC_LO);
		
		if(m_guiyuepara.yctype != 13)
		{
			if(pBuf->byFlag & (1<<6))
			{
				memcpy(&fd,(void*)&pBuf->lValue,4);
				dddd = fd;
			}
			else
				dddd = pBuf->lValue;
			
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(dddd);
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(dddd);		
		}
		else
		{
			if(pBuf->byFlag & (1<<6))
				memcpy(&fd,(void*)&pBuf->lValue,4);
			else
				fd = (float)pBuf->lValue/1000;
			BYTE *p = (BYTE*)&fd;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = p[3];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = p[2];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = p[1];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = p[0];
		}
		
		if(m_guiyuepara.yctype != M_ME_ND)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = QDS(pBuf->byFlag);//QDP
		if((m_guiyuepara.yctype == 10)||(m_guiyuepara.yctype == 12))
			write_time3();
		if((m_guiyuepara.yctype == M_ME_TD)||(m_guiyuepara.yctype == 35))
			write_time();
		pBuf++;
	}	
	
	write_VSQ(i);
	SendFrameTail();
	
	return;	
}

void CGB104Shs::SendAllYcYx(void)
{
	WORD i, YxGroupNum, YcGroupNum;
	VDBYCF_L *pBuf = (VDBYCF_L *)m_dwPubBuf;
	WORD wReqNum = 100;
	
	if (!CanSendIFrame())
		return;
	
	SwitchToEqpNo(m_dwSendAllDataEqpNo);
	
	YxGroupNum = (m_pEqpInfo[m_wEqpNo].wSYXNum +m_pEqpInfo[m_wEqpNo].wVYXNum+ YX_GRP_NUM - 1) / YX_GRP_NUM;
	YcGroupNum = (m_pEqpInfo[m_wEqpNo].wYCNum + YC_GRP_NUM - 1) / YC_GRP_NUM;
	if(m_initflag)
	{
		m_initflag = 0;	
	}
	
	for(i = 0; (i < 31) && (i < YxGroupNum); i++)
	{
		if( m_dwYxSendFlag & (1 << i) )
		{
			if(SendGroupDYx(i, COT_INTROGEN))
			  return;
			if(SendGroupYx(i, COT_INTROGEN))
			  return;
			m_wSendDYxNum = 0;
			m_wSendYxNum = 0;
			m_dwYxSendFlag &= ~(1<<i);
		}
	}
	for(i = 0; (i < 31) && (i < YcGroupNum); i++)
	{
		if( m_dwYcSendFlag & (1<<i) )
		{
			if(SendGroupYc(i, COT_INTROGEN))
				return;
			m_dwYcSendFlag &= ~(1<<i);
			m_wSendYcNum = 0;
		
		}
	}
	if( m_dwYxSendFlag & 0x80000000 )
	{
		m_dwSendAllDataEqpNo++;
		if ( m_dwSendAllDataEqpNo >= m_wEqpNum)
		{
			m_dwSendAllDataEqpNo = 0;
			m_dwYxSendFlag = 0;
			m_dwYcSendFlag = 0;
			m_allcallflag = 0;
			SendStopYcYx();
			SearchChangeYC(wReqNum, MAX_PUBBUF_LEN *4, (VDBYCF_L*)pBuf,0);
		}
		else
		{
			m_dwYxSendFlag = 0xffffffff;
			m_dwYcSendFlag = 0xffffffff;	
		}
	}	
}


void CGB104Shs::SendSomeYcYx(void)
{
	WORD i, YxGroupNum, YcGroupNum;
	
	if (!CanSendIFrame())
		return;
	
	YxGroupNum = (m_pEqpInfo[m_wEqpNo].wSYXNum + m_pEqpInfo[m_wEqpNo].wVYXNum + YX_GRP_NUM - 1) / YX_GRP_NUM;
	YcGroupNum = (m_pEqpInfo[m_wEqpNo].wYCNum + YC_GRP_NUM - 1) / YC_GRP_NUM;
	
	for(i = 0; (i < 31) && (i < YcGroupNum); i++)
	{
		if( m_dwYcGroupFlag & (1<<i) )
		{
			if(SendGroupYc(i, i+COT_INRO9))
			return;
			m_dwYcGroupFlag &= ~(1<<i);
		}
	}
	for(i = 0; (i < 31) && (i < YxGroupNum); i++)
	{
		if( m_dwYxGroupFlag & (1<<i) )
		{	
			if(SendGroupDYx(i, i+COT_INRO1))
				return;
			if(SendGroupYx(i, i+COT_INRO1))
				return;
			m_wSendDYxNum = 0;
			m_wSendYxNum = 0;
			m_dwYxGroupFlag &= ~(1<<i);
		}
	}
	
	if( m_dwYxGroupFlag & 0x80000000 )
	{
		m_dwYxGroupFlag = 0;
		SendStopSomeYcYx();
	}	
	
	if( m_dwYcGroupFlag & 0x80000000 )
	{
		m_dwYcGroupFlag = 0;
		SendStopSomeYcYx();
	}	
}

void CGB104Shs::SendStopYcYx(void)
{
	BYTE TypeID = C_IC_NA;
	BYTE Reason = COT_ACTTERM;
	BYTE Qualifier = 0x1;
	
	SendFrameHead(FRM_I);
		
	Sendframe(TypeID, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x14;
	
	write_VSQ(Qualifier);
	SendFrameTail();
	m_allcalloverflag = 1;
	return;	
}

void CGB104Shs::SendStopSomeYcYx(void)
{
	BYTE TypeID = C_IC_NA;
	BYTE Reason = COT_ACTTERM;
	BYTE Qualifier = 0x1;
	
	SendFrameHead(FRM_I);
	Sendframe(TypeID, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_QOI;
	
	write_VSQ(Qualifier);
	SendFrameTail();
	return;	
}

BOOL CGB104Shs::SendGroupYc(WORD YcGroupNo,BYTE Reason)
{
	WORD YcNo;
	struct VYCF_L * YcValue;
	WORD ReadYcNum = 64;
	BYTE VSQ=0x80;
	float fd;
	DWORD dddd;
	
	if (!CanSendIFrame())
		return FALSE;

	YcNo = YcGroupNo * YC_GRP_NUM;
	YcNo+=m_wSendYcNum;
	if ((YcNo >= m_pEqpInfo[m_wEqpNo].wYCNum + m_pEqpInfo[m_wEqpNo].wVYCNum)||(YcNo>=(YcGroupNo+1) * YC_GRP_NUM))
	{
		m_wSendYcNum = 0;
		return FALSE;
	}
		
	SendFrameHead(FRM_I);
	Sendframe(m_guiyuepara.yctype, Reason);
	if((m_guiyuepara.yctype == 21)||(m_guiyuepara.yctype == 9)||(m_guiyuepara.yctype == 11)||(m_guiyuepara.yctype == 13))
		write_infoadd(YcNo + ADDR_YC_LO);

	WORD i;
	ReadYcNum=	ReadRangeYCF_L(m_wEqpID, YcNo, ReadYcNum, ReadYcNum*sizeof(struct VYCF_L ), (struct VYCF_L *)m_dwPubBuf);
	YcValue = (struct VYCF_L *)m_dwPubBuf;

	for( i = 0; (m_SendBuf.wWritePtr < 230) && (i + m_wSendYcNum < YC_GRP_NUM) && (i < ReadYcNum) && (YcNo + i < m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum); i++)
	{
		if((m_guiyuepara.yctype == 35)||(m_guiyuepara.yctype == 10)||(m_guiyuepara.yctype == 12)||(m_guiyuepara.yctype == 34))
			write_infoadd(YcNo +i+ ADDR_YC_LO);

		if(m_guiyuepara.yctype != 13)
		{
			if(YcValue[i].byFlag & (1<<6))
			{
				memcpy(&fd,(void*)&YcValue[i].lValue,4);
				dddd = fd;
			}
			else
				dddd = YcValue[i].lValue;

			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(dddd);
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(dddd);
		
		}
		else
		{
			if(YcValue[i].byFlag & (1 << 6))
				memcpy(&fd,(void*)&YcValue[i].lValue,4);
			else
				fd = (float)YcValue[i].lValue/1000;

			BYTE *p = (BYTE*)&fd;

			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = p[3];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = p[2];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = p[1];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = p[0];

		}

		if(m_guiyuepara.yctype != M_ME_ND)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = QDS(YcValue[i].byFlag);//QDP

		if((m_guiyuepara.yctype == 10)||(m_guiyuepara.yctype == 12))
			write_time3();

		if((m_guiyuepara.yctype == M_ME_TD)||(m_guiyuepara.yctype == 35))
			write_time();

		m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=false;
        		m_pEqpExtInfo[m_wEqpNo].OldYC[YcNo+i].lValue = m_pEqpExtInfo[m_wEqpNo].CurYC[i+YcNo].lValue = YcValue[i].lValue;
		m_pEqpExtInfo[m_wEqpNo].OldYC[YcNo+i].byFlag = m_pEqpExtInfo[m_wEqpNo].CurYC[i+YcNo].byFlag = YcValue[i].byFlag;
	}

	if((m_guiyuepara.yctype == 35)||(m_guiyuepara.yctype == 34)||(m_guiyuepara.yctype == 10)||(m_guiyuepara.yctype == 12))
		VSQ = 0;

	write_VSQ(i | VSQ);
	m_wSendYcNum += i;
	SendFrameTail();
	
	return TRUE;
}

BOOL CGB104Shs::SendGroupYx(WORD YxGroupNo,BYTE Reason)
{
	BYTE TypeID = M_SP_NA;
	WORD YxNo;
	BYTE *YxValue;
	WORD ReadYxNum = 128;
	BYTE VSQ = 0x80;

	if (!CanSendIFrame())
		return FALSE;

	YxNo = YxGroupNo * YX_GRP_NUM;
	YxNo += m_wSendYxNum;
	if((m_wSendYxNum+m_wSendDYxNum >= YX_GRP_NUM)|| (YxNo >= m_pEqpInfo[m_wEqpNo].wSYXNum + m_pEqpInfo[m_wEqpNo].wVYXNum))
	{
		return FALSE;
	}
		
	SendFrameHead(FRM_I);

	if(m_guiyuepara.yxtype == 1)
		TypeID = 1;

	if(m_guiyuepara.yxtype == 2)
		TypeID = 2;

	if(m_guiyuepara.yxtype == 3)
		TypeID = 3;

	Sendframe(TypeID,Reason);
	
	WORD i;

	if((m_guiyuepara.yxtype == 1)||(m_guiyuepara.yxtype == 3))
		write_infoadd(YxNo + ADDR_YX_LO + m_wSendDYxNum);

	ReadYxNum =	ReadRangeSYX(m_wEqpID, YxNo, ReadYxNum, ReadYxNum*sizeof(BYTE), (BYTE*)m_dwPubBuf);
	YxValue = (BYTE*)m_dwPubBuf;

	for(i = 0;(m_SendBuf.wWritePtr < 230)&&(i+m_wSendYxNum+m_wSendDYxNum < YX_GRP_NUM) && (i<YX_FRM_NUM) && (YxNo<m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum); i++, YxNo++)
	{
		if((m_guiyuepara.yxtype != 1) && (m_guiyuepara.yxtype != 3))
			write_infoadd(YxNo + ADDR_YX_LO + m_wSendDYxNum);

		if(m_guiyuepara.yxtype == 3)
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr ] = SIQ(YxValue[i] )+1;
		else
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr ] = SIQ(YxValue[i] );

		m_SendBuf.wWritePtr++;

		if(TypeID == 2)
			write_time3();

		if(TypeID == 30)
			write_time();
	}

	if((m_guiyuepara.yxtype != 1) && (m_guiyuepara.yxtype != 3))
		VSQ=0;

	write_VSQ(i | VSQ);
	m_wSendYxNum += i;
	SendFrameTail();
	
	return TRUE;
}


BOOL CGB104Shs::SendGroupDYx(WORD YxGroupNo,BYTE Reason)
{
	BYTE TypeID = M_DP_NA;
	WORD YxNo;
	struct VDYX* YxValue;
	WORD ReadYxNum = 1;
	BYTE VSQ = 0x80;

	if (!CanSendIFrame())
		return FALSE;

	YxNo = YxGroupNo * YX_GRP_NUM;
	YxNo += m_wSendDYxNum;
	if((m_wSendDYxNum >= YX_GRP_NUM) || (YxNo >= m_pEqpInfo[m_wEqpNo].wDYXNum))
	{
		return FALSE;
	}
		
	SendFrameHead(FRM_I);
	if(m_guiyuepara.yxtype == 1)
		TypeID = 3;

	if(m_guiyuepara.yxtype == 2)
		TypeID = 4;

	if(m_guiyuepara.yxtype == 3)
		TypeID = 31;

	Sendframe(TypeID,Reason);

	WORD i;

	if(m_guiyuepara.yxtype == 1)
		write_infoadd(YxNo + ADDR_YX_LO);

	for(i = 0;(m_SendBuf.wWritePtr < 230)&& (i + m_wSendDYxNum < YX_GRP_NUM)&&(i < YX_FRM_NUM) && (YxNo<m_pEqpInfo[m_wEqpNo].wDYXNum); i++, YxNo++)
	{
		if(m_guiyuepara.yxtype != 1)
			write_infoadd(YxNo + ADDR_YX_LO);

		ReadRangeDYX(m_wEqpID, YxNo, ReadYxNum, sizeof(struct VDYX ), (struct VDYX *)m_dwPubBuf);
		YxValue = (struct VDYX *)m_dwPubBuf;

		m_SendBuf.pBuf[ m_SendBuf.wWritePtr ] = DIQ(YxValue->byValue1,YxValue->byValue2);

		m_SendBuf.wWritePtr++;

		if(TypeID == 4)
			write_time3();

		if(TypeID == 31)
			write_time();
	}
	if(m_guiyuepara.yxtype != 1)
		VSQ = 0;

	write_VSQ(i | VSQ);
	m_wSendDYxNum += i;
	SendFrameTail();
	
	return TRUE;
}

void CGB104Shs::SendCallYcYxAck(void)
{
	BYTE Qualifier = 0x1;

	if (CanSendIFrame())
	{
		SendFrameHead(FRM_I);

		if((m_dwasdu.COT == 6)||(m_dwasdu.COT == 8))
			Sendframe(C_IC_NA, m_dwasdu.COT+1);
		else
			Sendframe(C_IC_NA, m_dwasdu.COT);

		write_infoadd(0);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_QOI;
		
		write_VSQ(Qualifier);
		SendFrameTail();
	}
	
	return;
}

void CGB104Shs::SendSomeYcYxAck(BYTE byQrp)
{
	BYTE TypeID = C_IC_NA;
	BYTE Reason = COT_ACTCON;
	BYTE Qualifier = 0x1;
	
	if (CanSendIFrame())
	{
		SendFrameHead(FRM_I);
		
		Sendframe(TypeID, Reason);
		write_infoadd(0);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = byQrp;
		write_VSQ(Qualifier);

		SendFrameTail();
	}
	
	return;
}

BOOL CGB104Shs::CanSendIFrame(void)
{
    return (m_bDTEnable && m_bContinueSend);    
}

void CGB104Shs::SendInitFinish(void)
{
	SendFrameHead(FRM_I);
	if(m_initflag)
	{
		m_initflag = 0;
	}
	Sendframe(70, 4);
	write_VSQ(1);

	write_infoadd(0);
	if((g_Sys.ResetInfo.code&0xff) == 1)
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1;
	else if((g_Sys.ResetInfo.code & 0xff) == 2)
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 2;
	else 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
	
	SendFrameTail();  

}


void CGB104Shs::initpara(void)
{
	if((m_guiyuepara.typeidlen < 1)||(m_guiyuepara.typeidlen > 2))
	{
		m_guiyuepara.typeidlen = 1;
		m_guiyuepara.conaddrlen = 2;
		m_guiyuepara.VSQlen = 1;
		m_guiyuepara.COTlen = 2;
		m_guiyuepara.infoaddlen = 3;
		m_guiyuepara.baseyear = 2000;
		m_guiyuepara.ddtype = 15;
		m_guiyuepara.yctype = 9;
		m_guiyuepara.yxtype = 1;
		m_guiyuepara.k = 12;
		m_guiyuepara.w = 8;
		m_guiyuepara.t0 = 10;
		m_guiyuepara.t1 = 15;
		m_guiyuepara.t2 = 10;
		m_guiyuepara.t3 = 20;
		m_guiyuepara.t4 = 8;

	}
}

void CGB104Shs::getasdu(void)
{	
	BYTE off = 0;
	if(m_guiyuepara.typeidlen == 1)
	{
		m_dwasdu.TypeID = m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.typeidlen == 2)
	{
		m_dwasdu.TypeID = MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
		off += 2;
	}
	if(m_guiyuepara.VSQlen == 1)
	{
		m_dwasdu.VSQ = m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.VSQlen == 2)
	{
		m_dwasdu.VSQ = MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
		off += 2;
	}
	m_dwasdu.COToff = off;
	if(m_guiyuepara.COTlen == 1)
	{
		m_dwasdu.COT = m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.COTlen == 2)
	{
		m_dwasdu.COT = m_pReceive->byASDU[off++];
		m_sourfaaddr = m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.conaddrlen == 1)
	{
		m_dwasdu.Address = m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.conaddrlen == 2)
	{
		m_dwasdu.Address = MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
		off += 2;
	}
	m_dwasdu.Infooff = off;
	if(m_guiyuepara.infoaddlen == 1)
	{
		m_dwasdu.Info = m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.infoaddlen == 2)
	{
		m_dwasdu.Info = MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
		off += 2;
	}
	if(m_guiyuepara.infoaddlen == 3)
	{
		m_dwasdu.Info = MAKEWORD(m_pReceive->byASDU[off+1], m_pReceive->byASDU[off+2]);
		m_dwasdu.Info <<= 8;
		m_dwasdu.Info |= m_pReceive->byASDU[off];
		off += 3;
	}
	if(m_guiyuepara.infoaddlen > 3)
	   m_guiyuepara.infoaddlen = 3;
}
void CGB104Shs::write_typeid(int  data)
{
	m_SendBuf.wWritePtr = 6;
	for(BYTE i = 0;i < m_guiyuepara.typeidlen;i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (data>>(i*8))&0xff;
	}

}
void CGB104Shs::write_VSQ(int  data)
{
	for(BYTE i = 0; i < m_guiyuepara.VSQlen;i++)
	{
		m_SendBuf.pBuf[ i+6+m_guiyuepara.typeidlen ] = (data>>(i*8))&0xff;
	}

}
void CGB104Shs::write_COT(int  data)
{
	m_SendBuf.wWritePtr = 6 + m_guiyuepara.typeidlen+m_guiyuepara.VSQlen;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr + 0 ] = (data) & 0xff;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr + 1] = m_sourfaaddr;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr + 2] = 0;
	m_SendBuf.wWritePtr += m_guiyuepara.COTlen;
		

}
void CGB104Shs::write_conaddr(int  data)
{
	m_SendBuf.wWritePtr = 6 + m_guiyuepara.typeidlen+m_guiyuepara.VSQlen+m_guiyuepara.COTlen;
	for(BYTE i = 0;i < m_guiyuepara.conaddrlen; i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (data>>(i*8))&0xff;
	}

}

void CGB104Shs::write_infoadd(int  data)
{
	for(BYTE i = 0; i < m_guiyuepara.infoaddlen;i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (data>>(i*8))&0xff;
	}
}

void CGB104Shs::CheckCfg()
{
	if((m_guiyuepara.typeidlen < 1)||(m_guiyuepara.typeidlen > 2))
	{
		m_guiyuepara.typeidlen = 1;
		m_guiyuepara.conaddrlen = 2;
		m_guiyuepara.VSQlen = 1;
		m_guiyuepara.COTlen = 2;
		m_guiyuepara.infoaddlen = 3;
		m_guiyuepara.baseyear = 2000;
		m_guiyuepara.ddtype = 15;
		m_guiyuepara.yctype = 9;
		m_guiyuepara.yxtype = 1;
		m_guiyuepara.k = 12;
		m_guiyuepara.w = 8;
		m_guiyuepara.t0 = 10;
		m_guiyuepara.t1 = 15;
		m_guiyuepara.t2 = 10;
		m_guiyuepara.t3 = 20;
		m_guiyuepara.t4 = 8;

	}
	m_PARA_K = m_guiyuepara.k;
	m_PARA_W = m_guiyuepara.w;
	m_vTimer[1].bRun = FALSE;
	m_vTimer[1].wInitVal = m_guiyuepara.t1;
	m_vTimer[2].bRun = FALSE;
	m_vTimer[2].wInitVal = m_guiyuepara.t2;
	m_vTimer[3].bRun = FALSE;
	m_vTimer[3].wInitVal = m_guiyuepara.t3;
	if((m_guiyuepara.baseyear != 2000) && (m_guiyuepara.baseyear != 1900))
		m_guiyuepara.baseyear = 2000;
	

}
void CGB104Shs::SetDefCfg()
{
	m_guiyuepara.typeidlen = 1;
	m_guiyuepara.conaddrlen = 2;
	m_guiyuepara.VSQlen = 1;
	m_guiyuepara.COTlen = 2;
	m_guiyuepara.infoaddlen = 3;
	m_guiyuepara.baseyear = 2000;
	m_guiyuepara.ddtype = 15;
	m_guiyuepara.yctype = 9;
	m_guiyuepara.yxtype = 1;
	m_guiyuepara.k = 12;
	m_guiyuepara.w = 8;
	m_guiyuepara.t0 = 10;
	m_guiyuepara.t1 = 12;
	m_guiyuepara.t2 = 5;
	m_guiyuepara.t3 = 15;
	m_guiyuepara.t4 = 8;

}
 BYTE CGB104Shs::QDS(BYTE data)
 {
	BYTE tt = 0;
	
	if((data & 1) == 0)
		tt |= 0x80;
	if((data & 2))
		tt |= 0x10;
	if((data & 4))
		tt |= 0x20;
	if((data & 8))
		tt |= 0xc0;
	if((data & 0x10))
		tt |= 0x1;
	return tt;	
 }

 BYTE CGB104Shs::SIQ(BYTE data)
 {
	BYTE tt = 0;
	if((data & 1) == 0)
		tt |= 0x80;
	if((data & 2))
		tt |= 0x10;
	if((data & 4))
		tt |= 0x60;
	if((data & 8))
		tt |= 0xc0;
	if((data & 0x80))
		tt |= 0x1;

	return tt;	
 }
 BYTE CGB104Shs::DIQ(BYTE data1,BYTE data2)
 {
	BYTE tt = 0;
	if((data1 & 1) == 0)
		tt |= 0x80;
	if((data1 & 2))
		tt |= 0x10;
	if((data1 & 4))
		tt |= 0x20;
	if((data1 & 8))
		tt |= 0xc0;
	if((data1 & 0x80))
		tt |= 0x1;
	if((data2 & 0x80))
		tt |= 0x2;
	return tt;	
 }

void CGB104Shs::write_time()
{
	struct VSysClock clock;	
	GetSysClock(&clock,SYSCLOCK);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (clock.wMSecond+clock.bySecond*1000)&0xff;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (clock.wMSecond+clock.bySecond*1000)>>8;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = clock.byMinute;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = clock.byHour;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = clock.byDay|(clock.byWeek<<5);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = clock.byMonth;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = clock.wYear%100;

}

void CGB104Shs::write_time3()
{
	struct VSysClock clock;	
	GetSysClock(&clock,SYSCLOCK);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(clock.wMSecond+clock.bySecond*1000)&0xff;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(clock.wMSecond+clock.bySecond*1000)>>8;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=clock.byMinute;
}

void CGB104Shs::DoUrgency(void)
{
    int i, j;
    CPSecondary::DoUrgency();

	for (i=0; i<2; i++)
	{
	    if (TestFlag(m_wTaskID, m_wEqpGroupID, m_flag[i]))
	    {
	        for (j=0; j<m_wEqpNum; j++) 
	            SetEqpFlag(j, m_eqpflag[i]);
		    ClearFlag(m_wTaskID, m_wEqpGroupID, m_flag[i]);
	    }
	}
}


 #endif
