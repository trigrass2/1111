#include "syscfg.h"

#ifdef INCLUDE_FA_SH

#include "gb104shm.h"

extern "C" DWORD CalendarClock(VSysClock *pTime);
extern "C" void writeFlag(WORD wEqpID,DWORD dwFlag);

extern void WriteEqpSOE(WORD wEqpID, WORD num, struct VDBSOE *psoe);

extern "C"  void gb104shm(WORD wTaskID)		
{
	
	CGb104Shm *pGB104P = new CGb104Shm(); 	

	if (pGB104P->Init(wTaskID) != TRUE)
	{
		pGB104P->ProtocolExit();
	}
	pGB104P->Run();			   
	 
}


CGb104Shm::CGb104Shm() : CPPrimary()
{
	m_wSendNum = 0;	
	m_wRecvNum = 0;
	m_wAckNum = 0;		
	m_wAckRecvNum = 0;
	m_bDTEnable = FALSE;
	m_bTcpConnect = FALSE;
	m_bContinueSend = TRUE;
	m_bCallAllFlag = FALSE;
	m_bSetTimeFlag = FALSE;
		
	m_vTimer[1].bRun = FALSE;
	m_vTimer[1].wInitVal = PARA_T1;
	m_vTimer[2].bRun = FALSE;
	m_vTimer[2].wInitVal = PARA_T2;
	m_vTimer[3].bRun = FALSE;
	m_vTimer[3].wInitVal = PARA_T3;
	memset(m_vBackFrame, 0xff, sizeof(m_vBackFrame));	
	
	
}

BOOL CGb104Shm::Init(WORD wTaskID)
{
	BOOL rc;
	rc = CPPrimary::Init(wTaskID,1,&m_guiyuepara,sizeof(m_guiyuepara));
	if (!rc)
	{
		return FALSE;
	}
	
	DoCommState();
	
	m_PARA_K = 12;
	m_PARA_W = 8;
	initpara();
	DWORD event_time = 100;	   //100 for 1s
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);

	return TRUE;	
}

void CGb104Shm::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();
	
	m_pBaseCfg->wMaxRetryCount = 0;
	m_pBaseCfg->wMaxErrCount = 1;
}

void CGb104Shm::SetBaseCfg(void)
{
	CProtocol::SetBaseCfg();

	m_pBaseCfg->wCmdControl = YKOPENABLED | CLOCKENABLE | ALLDATAPROC;
	
	for (int EqpNo = 0; EqpNo < m_wEqpNum; EqpNo++)
	{
		if (m_pEqpInfo[EqpNo].wDDNum  != 0)
		{
			m_pBaseCfg->wCmdControl |= DDPROC;
			break;
		}
		 
	}
	
	m_pBaseCfg->Timer.wAllData = 30;	//30 min
	m_pBaseCfg->Timer.wSetClock = 5;	//5 min
	m_pBaseCfg->Timer.wDD = 60;		    //60 min
	m_pBaseCfg->wMaxRetryCount = 0;
	m_pBaseCfg->wMaxErrCount = 1;
	
}


void CGb104Shm::StartTimer(BYTE TimerNo)
{
	if (TimerNo > 3)
   		return;
  	m_vTimer[TimerNo].wCounter = m_vTimer[TimerNo].wInitVal;
  	m_vTimer[TimerNo].bRun = TRUE;	
}

void CGb104Shm::StopTimer(BYTE TimerNo)
{
	if (TimerNo > 3)
   		return;
  	m_vTimer[TimerNo].bRun = FALSE;	
}


void CGb104Shm::DoTimeOut(void)
{
	CPPrimary::DoTimeOut(); 
	
	if (m_vTimer[1].bRun)
   	{
    	m_vTimer[1].wCounter--;
    	if (m_vTimer[1].wCounter == 0)
     	{       		
      		CloseTcp();
      		    		
      		StopTimer(1);
     	}
   	}
  	if (m_vTimer[2].bRun)
   	{
    	m_vTimer[2].wCounter--;
    	if (m_vTimer[2].wCounter == 0)
     	{
      		
      		SendSFrame();
      		StopTimer(2);
		}
   	}
  	if (m_vTimer[3].bRun)
   	{
    	m_vTimer[3].wCounter--;
    	if (m_vTimer[3].wCounter == 0)
     	{
     	    SendUFrame(TESTFR_ACT);
      		StopTimer(3);
      	}
   	}  
   	   	
}


BOOL CGb104Shm::DoYKReq(void)
{
	CPPrimary::DoYKReq();
	
	if (SwitchClearEqpFlag(EQPFLAG_YKReq)) 
		if (SendYkCommand()) 
			return TRUE;
	return TRUE;
}

void CGb104Shm::DoDoubleYk(void)
{
	if (m_dwasdu.COT    == COT_ACTCON)
		DoYkReturn();
	else
	{
		if (m_dwasdu.COT == COT_DEACTCON)
			DoYkCancelAck();	
	}
	return;	
}

void CGb104Shm::DoYkReturn(void)
{
	BYTE * pData = m_pASDU->byInfo;
	WORD  DCO;	//遥控命令限定词
	WORD  YkNo; //遥控路号
	YkNo = MAKEWORD(pData[0], pData[1]);
#ifdef INCLUDE_DF104FORMAT
	DCO = pData[2];
#else
	DCO = pData[3];
#endif
	
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	
	//add yk return info proc
	
	switch (m_pASDU->byTypeID)
	{
	    case C_DC_NA:	
		    YkNo = YkNo - ADDR_YK_LO + 1;//yk no from 1 start
		    break;
	    case C_RC_NA:	
		    YkNo = YkNo - ADDR_YT_LO +1;//yt no from 1 start
		    break;
	}
	
	pYKInfo->Head.wEqpID = m_wEqpID;
	pYKInfo->Info.wID = YkNo;
	switch (DCO & 0x80)
	{
	    case 0x80:
		    pYKInfo->Head.byMsgID = MI_YKSELECT;
		    break;
	    case 0x00:
		    pYKInfo->Head.byMsgID = MI_YKOPRATE;
		    break;
	    default:
		    break;	
	}
	if (m_pASDU->byReasonLow == COT_DEACTCON)
	{
		pYKInfo->Head.byMsgID = MI_YKCANCEL;	
	}
	switch (DCO & 0x03)
	{
	    case 0: 
	    case 3:
			pYKInfo->Info.byValue = DEF_YK_INVALID_VALUE;
	    	break;
	    case 1: 
		    pYKInfo->Info.byValue = DEF_YK_CLOSE_VALUE;
		    break; //分
	    case 2: 
		    pYKInfo->Info.byValue = DEF_YK_OPEN_VALUE;
		    break; //合
	    default:
	        break;
	}
	if (m_pASDU->byReasonLow & COT_PN_BIT)
	{
		pYKInfo->Info.byStatus = CONTROLUNKNOWERROR;
	}
	else
	{
		pYKInfo->Info.byStatus = 0;
	}
	CPPrimary::DoYKRet();
	return;	
	
}


void CGb104Shm::DoYkCancelAck(void)
{
	return;	
}





void CGb104Shm::CloseTcp(void)
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
	
	DWORD dwPara;
	commCtrl(m_wCommID, CCC_CONNECT_CTRL | CONNECT_CLOSE, &dwPara); 
}

DWORD CGb104Shm::DoCommState(void)
{
	BOOL bTcpState = FALSE;
	DWORD dwCommState;
	VSysClock SysTime;
    dwCommState=CPPrimary::DoCommState();
	if (dwCommState == ETHERNET_COMM_ON)
		bTcpState = TRUE;
	else
		bTcpState = FALSE;
		
	StopTimer(1);
	StopTimer(2);
	StopTimer(3);
	GetSysClock((void *)&SysTime, SYSCLOCK);	
	if (bTcpState)
	{
		m_bTcpConnect = TRUE;
		m_bDTEnable = FALSE;
		m_wSendNum = 0;	
	    m_wRecvNum = 0;	
	    m_wAckNum = 0;		
	    m_wAckRecvNum = 0; 
	    SetTaskFlag(TASKFLAG_CLOCK);
		SetTaskFlag(TASKFLAG_ALLDATA);
	    thSleep(300);
	    SendUFrame(STARTDT_ACT);
  			
	}
	else
	{
		m_bTcpConnect = FALSE;
		m_bDTEnable = FALSE;		
	}

    return dwCommState;	
}


BOOL CGb104Shm::DoReceive()
{
	m_SendBuf.wWritePtr = 0;
	m_SendBuf.wReadPtr = 0;
	if (SearchFrame() != TRUE)
	{
		return FALSE;//没找到有效报文
	}
	
	m_pReceive = (VIec104Frame *)m_RecFrame.pBuf;
	
	m_pASDU = (VASDU *)&m_pReceive->byASDU;
	m_pASDUInfo = (BYTE *)&m_pASDU->byInfo;
	
	StartTimer(3);  //receive any I,U,S frame, to start T3
  
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
	return TRUE;	
}


DWORD CGb104Shm::SearchOneFrame(BYTE *Buf, WORD Len)
{
	DWORD Rc;
	BYTE FrameLen;
	
	m_pReceive=(VIec104Frame *)Buf;
		
    if (Len < MIN_RECEIVE_LEN)
    	return FRAME_LESS;
    	
    Rc = SearchHead(Buf,Len,0,START_CODE);
    if (Rc)
       return FRAME_ERR|Rc;

	FrameLen = m_pReceive->byAPDULen;
	
	if (FrameLen < 4)
		return FRAME_ERR | 1;
    
    if (FrameLen > (MAX_FRAME_LEN - 2))
		return FRAME_ERR|1;
    
    if ((FrameLen + 2) > Len)
		return FRAME_LESS;
    
    m_pASDU = (VASDU *)&m_pReceive->byASDU;
    
	return FRAME_OK | (FrameLen + 2); //返回本帧总长度
}


void CGb104Shm::DoIFrame(void)
{
	WORD wAckNum, wRecvNum;
	getasdu();

	if (SwitchToAddress(m_dwasdu.Address) == FALSE)
		return ; 

	StartTimer(2);	//when receive I frame,start T2
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);
	wRecvNum = MAKEWORD(m_pReceive->byControl1, m_pReceive->byControl2);

	wAckNum >>= 1;
	wRecvNum >>= 1;
	m_wRecvNum = wRecvNum + 1;
	
	m_wAckNum = wAckNum; 
	DelAckFrame(m_wAckNum - 1);           
	
	if (m_wAckNum == m_wSendNum)
	{
		StopTimer(1);
	}
	
//hll	if ((m_wRecvNum - m_wAckRecvNum) > PARA_W)
	{
	    SendSFrame();        
	}
	
	switch (m_dwasdu.TypeID)	
	{
	    case M_SP_NA:    
	    case 2:           
	    case 3:          
	    case 4:         
	 //   	DoSingleYx();
	    	break;
	    case M_SP_TB:          //带长时标的单点信息
	    case M_DP_TB:          //带长时标的单点信息
	        DoSingleSoe();
	        break;
	    case M_ME_NA:
	    case M_ME_TA:
	    case M_ME_NB:
	    case M_ME_TB:
	    case M_ME_ND:    //不带品质描述的测量值
	    case M_ME_TD:
	    case M_ME_TE:    //不带品质描述的测量值
	        DoYcData();
	        break;
		 case C_DC_NA://双点遥控命令
	        DoDoubleYk();
	        break;
	    case C_IC_NA:    //召唤命令
	        DoCallAllData();
	        break;
	    case C_CS_NA:    //时钟同步命令
	    	DoSetTime();
	    	break;
	    default:
	    	break;
	}
}


void CGb104Shm::DoSFrame(void)
{
	WORD wAckNum;
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);	
	m_wAckNum = wAckNum >> 1;
	
	if (m_wAckNum == m_wSendNum)
	{
		StopTimer(1);	//stop T1		
	}
	
}

void CGb104Shm::DoUFrame(void)
{
	switch (m_pReceive->byControl1)
	{
	    case STARTDT_CON:
	    	StopTimer(1);
	        m_bDTEnable = TRUE;
	        break;
	    case STOPDT_CON:
	    	StopTimer(1);
			StopTimer(2);
			StopTimer(3);
	        m_bDTEnable = FALSE;
	        break;
	    case TESTFR_ACT:
	        SendUFrame(TESTFR_CON);
	        break;
	    case TESTFR_CON:
	        StopTimer(1);	//stop T1
	        break;    
	}
	return;
}

void CGb104Shm::DoSingleYx(void)
{
	if (m_pASDU->byReasonLow == COT_SPONT)
		DoSingleCos();
	else
	{
		if ((m_pASDU->byReasonLow >= COT_INRO1) && (m_pASDU->byReasonLow <= COT_INRO8))
			DoSingleGroupYx();
		if (m_pASDU->byReasonLow == COT_INTROGEN)
			DoSingleGroupYx();
	}	
	return;	
}

void CGb104Shm::DoSingleCos(void)
{
	WORD wYxNum;
	WORD wYxBeginNo;
	VDBCOS RecCOS;
	VDBDCOS RecDCOS;
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;
	
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	
	wYxNum = m_dwasdu.VSQ & 0x7f;
	if(m_dwasdu.VSQ & 0x80)
	{
		wYxBeginNo = getinfoaddr(pData);
		wYxBeginNo = wYxBeginNo - ADDR_YX_LO;
		pData += m_guiyuepara.infoaddlen;
	}
	for (WORD i = 0; i < wYxNum; i++)
	{
		if((m_dwasdu.VSQ&0x80)==0)
		{
			wYxBeginNo = getinfoaddr(pData);
			wYxBeginNo = wYxBeginNo - ADDR_YX_LO;
			pData += m_guiyuepara.infoaddlen;
		}

		if((m_dwasdu.TypeID==1)||(m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==30))
		{
			RecCOS.wNo = wYxBeginNo;
			RecCOS.byValue = SIQ(*pData++);//QDP
    		WriteSCOS(m_wEqpID, 1, &RecCOS);
		}

		if((m_dwasdu.TypeID==3)||(m_dwasdu.TypeID==4)||(m_dwasdu.TypeID==31))
		{
			RecDCOS.wNo = wYxBeginNo;
			DIQ(*pData++,&RecDCOS.byValue1,&RecDCOS.byValue2);//QDP
	    	WriteDCOS(m_wEqpID, 1, &RecDCOS);
		}

		if((m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==4))
			pData += 3;

		if((m_dwasdu.TypeID==30)||(m_dwasdu.TypeID==31))
			pData += 7;
	}
	
	return;		
	
}

void CGb104Shm::DoSingleGroupYx(void)
{
	WORD wYxNum;
	WORD wYxBeginNo;
	BYTE *pYxBuf = NULL;
	struct VDYX *buf;
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;

	pYxBuf = (BYTE *)m_dwPubBuf;
	buf=(struct VDYX  *)m_dwPubBuf;

	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	
	wYxNum = m_dwasdu.VSQ & 0x7f;
	if(m_dwasdu.VSQ & 0x80)
	{
		wYxBeginNo = getinfoaddr(pData);
		wYxBeginNo = wYxBeginNo - ADDR_YX_LO;
		pData += m_guiyuepara.infoaddlen;
	}
	for (WORD i = 0; i < wYxNum; i++)
	{
		if((m_dwasdu.VSQ & 0x80) == 0)
		{
			wYxBeginNo = getinfoaddr(pData);
			wYxBeginNo = wYxBeginNo - ADDR_YX_LO;
			pData += m_guiyuepara.infoaddlen;
		}

		if((m_dwasdu.TypeID==1)||(m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==30))
		{
			*pYxBuf = SIQ(*pData++);//QDP
			if((m_dwasdu.VSQ & 0x80) == 0)
				WriteRangeSYX(m_wEqpID, wYxBeginNo, 1, pYxBuf);

			pYxBuf++;	
		}

		if((m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==4))
			pData += 3;

		if((m_dwasdu.TypeID==30)||(m_dwasdu.TypeID==31))
			pData += 7;
	}

	if(m_dwasdu.VSQ & 0x80)
	{
		if((m_dwasdu.TypeID==1)||(m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==30))
		{
			WriteRangeSYX(m_wEqpID, wYxBeginNo, wYxNum,(BYTE *) m_dwPubBuf);
		}
		
	}
	return;	
}

void CGb104Shm::DoSingleSoe(void)
{
	WORD wSoeNum;
	WORD flag = 0;
	BYTE byLow, byHigh;
	VDBSOE RecSOE; 
	VDBDSOE RecDSOE; 
	VSysClock SysTime;
	VCalClock CalendarTime;	
	BYTE *pData = m_pReceive->byASDU+m_dwasdu.Infooff;
	GetSysClock(&SysTime,SYSCLOCK);
	
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	
	wSoeNum = m_dwasdu.VSQ & 0x7f;
	if(m_dwasdu.VSQ & 0x80)
	{
		RecSOE.wNo = getinfoaddr(pData);
		RecSOE.wNo = RecSOE.wNo - ADDR_YX_LO;
		pData += m_guiyuepara.infoaddlen;
	}

	for (WORD i = 0; i < wSoeNum; i++)
	{
		if((m_dwasdu.VSQ & 0x80) == 0)
		{
			RecSOE.wNo = getinfoaddr(pData);
			RecSOE.wNo = RecSOE.wNo - ADDR_YX_LO;
			pData += m_guiyuepara.infoaddlen;
		}

		if((m_dwasdu.TypeID==1)||(m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==30))
			RecSOE.byValue = SIQ(*pData++);//QDP

		if((m_dwasdu.TypeID==3)||(m_dwasdu.TypeID==4)||(m_dwasdu.TypeID==31))
			DIQ(*pData++,&RecDSOE.byValue1,&RecDSOE.byValue2);//QDP

		if((m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==4))
		{
			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData ++;
			SysTime.wMSecond = MAKEWORD(byLow, byHigh);
			SysTime.byMinute = *pData;
			pData ++;
			CalendarTime.dwMinute = CalendarClock(&SysTime);
			CalendarTime.wMSecond = SysTime.wMSecond;
			
			RecSOE.Time.dwMinute = CalendarTime.dwMinute;
			RecSOE.Time.wMSecond = CalendarTime.wMSecond;
		}

		if((m_dwasdu.TypeID==30)||(m_dwasdu.TypeID==31))
		{
			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData ++;
			SysTime.wMSecond = MAKEWORD(byLow, byHigh);
			SysTime.byMinute = *pData;
			pData ++;
			SysTime.byHour = (*pData) & 0x1F;
			pData ++;
			SysTime.byDay = (*pData) & 0x1F;
			SysTime.byWeek = (*pData) >> 5;
			pData ++;
			SysTime.byMonth = (*pData) & 0x0F;
			pData ++;
			SysTime.wYear = *pData + 2000;
			pData ++;
			CalendarTime.dwMinute = CalendarClock(&SysTime);
			CalendarTime.wMSecond = SysTime.wMSecond;
			
			RecSOE.Time.dwMinute = CalendarTime.dwMinute;
			RecSOE.Time.wMSecond = CalendarTime.wMSecond;
		}

		if((m_dwasdu.VSQ & 0x80)==0)
		{
			if((m_dwasdu.TypeID==1)||(m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==30))
    		{
    		   WriteEqpSOE(m_wEqpID, 1, &RecSOE);
			   evSend(FA_ID, EV_UFLAG);
			}
		}
	}	
	
	return;	
}

void CGb104Shm::DoYcData(void)
{
	if (m_pASDU->byReasonLow == COT_SPONT)
		DoChangeYc();
	else
	{
//		if ((m_pASDU->byReasonLow >= COT_INRO9) && (m_pASDU->byReasonLow <= COT_INRO12))
//			DoGroupYc();
//		if (m_pASDU->byReasonLow == COT_INTROGEN)
//			DoGroupYc();
	}
	return;	
}

void CGb104Shm::DoChangeYc(void)
{
	WORD wYcNum;
	WORD wYcBeginNo;
	BYTE byLow, byHigh;
	int flag = 0;
	struct VTrueEqp *pTEqp;
	WORD wvEqpID; 
	int test;

	struct VYCF  *pYcValue = (struct VYCF  *)m_dwPubBuf;
	
	BYTE *pData = m_pReceive->byASDU + m_dwasdu.Infooff;
	
	wYcNum = m_dwasdu.VSQ & 0x7f;
	for (WORD i = 0; i < wYcNum; i++)
	{
		memset((void *)m_dwPubBuf, 0, sizeof(struct VYCF));
		wYcBeginNo = getinfoaddr(pData);
		wYcBeginNo = wYcBeginNo - ADDR_YC_LO;
		pData += m_guiyuepara.infoaddlen;
		byLow = *pData;
		pData ++;
		byHigh = *pData;
		pData ++;
		pYcValue->nValue = MAKEWORD(byLow, byHigh);

		if(m_dwasdu.TypeID != M_ME_ND)
			pYcValue->byFlag = QDS(*pData++);//QDP
		else
			pYcValue->byFlag = 1;

		if((m_dwasdu.TypeID==10)||(m_dwasdu.TypeID==12))
			pData += 3;

		if((m_dwasdu.TypeID==M_ME_TD)||(m_dwasdu.TypeID==35))
			pData += 7;

		WriteRangeYCF(m_wEqpID, wYcBeginNo, 1, pYcValue);

		if (wYcBeginNo == 0)
			flag |= FAIPOWERUFLAG;
		if (wYcBeginNo == 3)
			flag |= FAINEEDUFLAG;
	}

	if (flag)
	{
		::writeTEqpFlag(m_wEqpID, flag);
		evSend(FA_ID, EV_YCFLAG);
	}
	return;	
}

void CGb104Shm::DoGroupYc(void)
{
	WORD wYcNum;
	WORD wYcBeginNo;
	BYTE byLow, byHigh;
	
	struct VYCF  *pYcValue=(struct VYCF  *)m_dwPubBuf;
	
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));

	wYcNum = m_dwasdu.VSQ & 0x7f;
	if(m_dwasdu.VSQ & 0x80)
	{
		wYcBeginNo = getinfoaddr(pData);
		wYcBeginNo = wYcBeginNo - ADDR_YC_LO;
		pData += m_guiyuepara.infoaddlen;
	}
	
	for (WORD i = 0; i < wYcNum; i++)
	{
		if((m_dwasdu.VSQ & 0x80) == 0)
		{
			wYcBeginNo=  getinfoaddr(pData);
			wYcBeginNo = wYcBeginNo - ADDR_YC_LO;
			pData += m_guiyuepara.infoaddlen;
		}
		byLow = *pData;
		pData ++;
		byHigh = *pData;
		pData ++;
		pYcValue->nValue = MAKEWORD(byLow, byHigh);

		if(m_dwasdu.TypeID != M_ME_ND)
			pYcValue->byFlag = QDS(*pData++);//QDP
		else
			pYcValue->byFlag = 1;

		if((m_dwasdu.TypeID==10)||(m_dwasdu.TypeID==12))
			pData += 3;

		if((m_dwasdu.TypeID==M_ME_TD)||(m_dwasdu.TypeID==35))
			pData += 7;

		if((m_dwasdu.VSQ&0x80)==0)
			WriteRangeYCF(m_wEqpID, wYcBeginNo, 1, pYcValue);

		pYcValue++;	
	}
	if(m_dwasdu.VSQ & 0x80)
		WriteRangeYCF(m_wEqpID, wYcBeginNo, wYcNum, (struct VYCF  *)m_dwPubBuf);

	return;	
}


void CGb104Shm::DoCallAllData(void)
{
	if (m_pASDU->byReasonLow == COT_ACTCON)
		DoAllDataAck();
	else
	{
		if (m_pASDU->byReasonLow == COT_ACTTERM)
			DoAllDataStop();	
	}
	return;	
}

void CGb104Shm::DoAllDataAck(void)
{
	return;	
}

void CGb104Shm::DoAllDataStop(void)
{
	return;	
}

void CGb104Shm::DoSetTime(void)
{
	return;	
}

BOOL CGb104Shm::ReqCyc(void)
{
	if (m_pBaseCfg->wCmdControl & CLOCKENABLE)
  	{
    	if (CheckClearTaskFlag(TASKFLAG_CLOCK))
       	{
       		m_bSetTimeFlag = TRUE;
       		SendSetTime();
         	return TRUE;       
       	}  
  
    }
    if (CheckClearTaskFlag(TASKFLAG_ALLDATA)) 
  	{
  		m_bCallAllFlag = TRUE;
  		SendCallAllData();
  		return TRUE;
  	}
 
	return FALSE;
}

void CGb104Shm::SendSFrame(void)
{
    SendFrameHead(FRM_S);
    SendFrameTail();
    StopTimer(2);	//when send S frame, stop T2
    return;    
}


void CGb104Shm::SendUFrame(BYTE byControl)
{
    SendFrameHead(FRM_U);
    m_pSend->byControl1 = byControl;
    SendFrameTail();
    if ((byControl == TESTFR_ACT) || (byControl == STARTDT_ACT) || (byControl == STOPDT_ACT))
    {
    	StartTimer(1);		//statr T1
    	 	
    }
    return;    
}

void CGb104Shm::SendFrameHead(BYTE byFrameType)
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
	m_pReceive=(VIec104Frame *)m_RecFrame.pBuf;

    m_SendBuf.wWritePtr = APCI_LEN;
	
	return;
}


BOOL CGb104Shm::SendFrameTail(void)
{
	m_pSend->byAPDULen = m_SendBuf.wWritePtr - 2;
	SaveSendFrame(m_wSendNum, m_SendBuf.wWritePtr, m_SendBuf.pBuf);
	
	WriteToComm(GetEqpAddr());
	if ((m_pSend->byControl1 & 0x1) == FRM_I)
	{
		m_wSendNum ++;
		
		StopTimer(2);  //when send I frame, stop T2
	}
	return TRUE;
}

void CGb104Shm::SendSetTime(void)
{
	BYTE TypeID = C_CS_NA;
	BYTE Reason = COT_ACT;
	BYTE Qualifier = 1;
	VSysClock SysTime;
	WORD MSecond;
	
	GetSysClock((void *)&SysTime, SYSCLOCK);
	MSecond = SysTime.bySecond * 1000 + SysTime.wMSecond;
	
	if (!CanSendIFrame())
		return;
	
	SendFrameHead(FRM_I);
	Sendframe(TypeID, Reason);
	write_VSQ(Qualifier);
	write_infoadd(0);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(MSecond); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(MSecond);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byHour;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (SysTime.byDay & 0x1f) | ((SysTime.byWeek << 5) & 0xe0); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMonth;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.wYear - 2000; 

	SendFrameTail();
	
	return;	
}

void CGb104Shm::SendCallAllData(void)
{
	BYTE TypeID = C_IC_NA;
	BYTE Reason = COT_ACT;
	BYTE Qualifier = 1;
	
	if (!CanSendIFrame())
		return;	
	
	SendFrameHead(FRM_I);
	Sendframe(TypeID, Reason);
	write_VSQ(Qualifier);
	write_infoadd(0);

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0x14; 
	 
	SendFrameTail();
	return;	
}

BOOL CGb104Shm::SendYkCommand(void)
{
	BYTE TypeID = C_DC_NA;
	BYTE Reason;
	WORD YkNo;
	BYTE DCO = 0;
	BYTE Qualifier = 1;
//	VASDU * pSendAsdu;
	
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	
	YkNo = pYKInfo->Info.wID + ADDR_YK_LO -1;
	
	switch (pYKInfo->Info.byValue & 0x3)
	{
		case 0://不区分合分
			break;
		case 1://close合闸
			DCO |= DCO_DCS_CLOSE;
			break;
		case 2://open分闸
			DCO |= DCO_DCS_OPEN;
			break;
		case 3://非法
			break;	
	};
	switch (pYKInfo->Head.byMsgID & 0x7f)
	{
		case MI_YKSELECT:
			DCO |= DCO_SE_SELECT;
			Reason = COT_ACT;
			break;
		case MI_YKOPRATE:
			DCO |= DCO_SE_EXE;
			Reason = COT_ACT;
			break;
		case MI_YKCANCEL:
			Reason = COT_DEACT;
			break;
	};
	
	if (!CanSendIFrame())
		return FALSE;
		
	SendFrameHead(FRM_I);
	
//	pSendAsdu = (VASDU *)m_pSend->byASDU;
	#if 0
	pSendAsdu->byTypeID = TypeID;
	pSendAsdu->byReasonLow = Reason;
#ifndef INCLUDE_DF104FORMAT
	pSendAsdu->byReasonHigh = GetOwnAddr();
#endif
	pSendAsdu->byQualifier = Qualifier;
				
	m_SendBuf.wWritePtr += ASDUID_LEN;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YkNo); //信息体地址Lo
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YkNo); 
#ifndef INCLUDE_DF104FORMAT
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
#endif
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = DCO; 
#endif
	Sendframe(TypeID, Reason);
//	m_SendBuf.wWritePtr += ASDUID_LEN;
write_VSQ(Qualifier);
	write_infoadd(YkNo);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = DCO;	 
	SendFrameTail();
	#ifdef DEBUG_YC
	BYTE mode;
	mode = (pYKInfo->Head.byMsgID & 0x7f);
	logMsg("Send Yk No=%d, YkMode=%d\n",YkNo, mode,0,0,0,0);
	#endif
	return TRUE;	
}


void CGb104Shm::DelAckFrame(WORD SendNum)
{
	WORD i;
	for(i = 0; i < m_PARA_K; i++)
	{
		if( m_vBackFrame[i].wSendNum == SendNum )
		{
			m_vBackFrame[i].wSendNum = 0xffff;
			return;
		}
	}	
}

void CGb104Shm::SaveSendFrame(WORD SendNum, WORD FrameLen, BYTE *pFrame)
{
	WORD i;
	for(i = 0; i < m_PARA_K; i++)
	{
		if( m_vBackFrame[i].wSendNum == 0xffff )
		{
			m_vBackFrame[i].wSendNum = SendNum;
			m_vBackFrame[i].wFrameLen = FrameLen;
			memcpy(m_vBackFrame[i].byBuf, pFrame, FrameLen);
			return;
		}
	}	
}

BOOL CGb104Shm::CanSendIFrame(void)
{
    return (m_bDTEnable && m_bContinueSend);    
}

void CGb104Shm::Sendframe(BYTE type,WORD cot)
{
	write_typeid(type);
	write_COT((GetOwnAddr()<<8)|cot);
	write_conaddr(GetEqpAddr());

}

void CGb104Shm::initpara(void)
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
		m_guiyuepara.t3 = 10;
		m_guiyuepara.t4 = 8;

	}
}






void CGb104Shm::write_typeid(int  data)
{
	m_SendBuf.wWritePtr = 6;
	for(BYTE i = 0; i < m_guiyuepara.typeidlen;i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (data>>(i*8))&0xff;
	}

}
void CGb104Shm::write_VSQ(int  data)
{
	for(BYTE  i=0;i<m_guiyuepara.VSQlen;i++)
	{
		m_SendBuf.pBuf[ i+6+m_guiyuepara.typeidlen ]=(data>>(i*8))&0xff;
	}
}

void CGb104Shm::write_COT(int  data)
{
	m_SendBuf.wWritePtr = 6+m_guiyuepara.typeidlen+m_guiyuepara.VSQlen;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr+0 ] = (data) & 0xff;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr+1] = data >> 8;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr+2] = 0;
	m_SendBuf.wWritePtr += m_guiyuepara.COTlen;	
}

void CGb104Shm::write_conaddr(int  data)
{
	m_SendBuf.wWritePtr = 6 + m_guiyuepara.typeidlen+m_guiyuepara.VSQlen+m_guiyuepara.COTlen;
	for(BYTE i = 0;i < m_guiyuepara.conaddrlen;i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (data>>(i*8))&0xff;
	}

}

void CGb104Shm::write_infoadd(int  data)
{
	for(BYTE i = 0; i < m_guiyuepara.infoaddlen;i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (data >> (i*8))&0xff;
	}

}
void CGb104Shm::getasdu(void)
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

	if(m_guiyuepara.COTlen == 1)
	{
		m_dwasdu.COT = m_pReceive->byASDU[off++];
	}

	if(m_guiyuepara.COTlen == 2)
	{
		m_dwasdu.COT = m_pReceive->byASDU[off++];
		off++;
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
}
 BYTE CGb104Shm::QDS(BYTE data)
 {
	BYTE tt = 0;
	if((data & 0x80) == 0)
		tt |= 1;
	if((data & 1))
		tt |= 0x10;
	if((data & 0x10))
		tt |= 0x2;
	if((data & 0x20))
		tt |= 0x4;
	if((data & 0x40))
		tt |= 0x8;
	return tt;	
 }

 BYTE CGb104Shm::SIQ(BYTE data)
 {
	BYTE tt = 0;
	if(data & 1)
		tt |= 0x80;
	if((data & 0x10))
		tt |= 0x2;
	if((data & 0x20))
		tt |= 0x4;
	if((data & 0x40))
		tt |= 0x8;
	if((data & 0x80) == 0)
		tt |= 0x1;
	return tt;	
 }
void  CGb104Shm::DIQ(BYTE data,BYTE *data1,BYTE *data2)
 {
	*data1 = 0;
	*data2 = 0;
	if((data & 0x10))
		*data1 |= 0x2;
	if((data & 0x20))
		*data1 |= 0x4;
	if((data & 0x40))
		*data1 |= 0x8;
	if((data & 0x80) == 0)
		*data1 |= 0x1;

	*data2 = *data1;
	if(data & 1)
		*data1 |= 0x80;
	if(data & 2)
		*data2 |= 0x80;
	return ;	
 }
DWORD CGb104Shm:: getinfoaddr(BYTE *psrc)
{
	DWORD addr=0;
	if(m_guiyuepara.infoaddlen == 1)
	{
		addr = *psrc;
	}
	if(m_guiyuepara.infoaddlen == 2)
	{
		addr = MAKEWORD(*psrc, *(psrc+1));
	}
	if(m_guiyuepara.infoaddlen == 3)
	{
		addr = MAKEWORD( *(psrc+1), *(psrc+2));
		addr <<= 8;
		addr |= *(psrc);
	}
	return addr;
}

#endif

