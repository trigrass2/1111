/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/
#include "syscfg.h"

#ifdef INCLUDE_GB104_M

#include "gb104m.h"

extern WORD g_ddfifflag, g_dddayflag, g_ddtideflag;
extern WORD DD_START_ADDR,DD_REAL_START_ADDR,DD_FIF_START_ADDR,DD_DAY_START_ADDR,DD_TIDE_START_ADDR;
extern BYTE g_ddhxnum;
extern DWORD CalendarClock(VSysClock *pTime);
extern "C"  void gb104m(WORD wTaskID)		
{
	
	CGb104m *pGB104P = new CGb104m(); 	
	
	if (pGB104P->Init(wTaskID) != TRUE)
	{
		pGB104P->ProtocolExit();
	}
	pGB104P->Run();			   
	 
}


CGb104m::CGb104m() : CPPrimary()
{
	m_wSendNum = 0;	//Send Counter	
	m_wRecvNum = 0;	//Receive Counter
	m_wAckNum = 0;		//Receive Ack Counter
	m_wAckRecvNum = 0; //Have Ack Receive Counter
	m_bDTEnable = FALSE;		//Data Transfer Enable Flag
	m_bTcpConnect = FALSE;			//TCP Connect Flag
	m_bContinueSend = TRUE;
	m_bCallAllFlag = FALSE;
	m_bCallDdFlag = FALSE;
	m_bSetTimeFlag = FALSE;
		
	m_vTimer[1].bRun = FALSE;
	m_vTimer[1].wInitVal = PARA_T1;
	m_vTimer[2].bRun = FALSE;
	m_vTimer[2].wInitVal = PARA_T2;
	m_vTimer[3].bRun = FALSE;
	m_vTimer[3].wInitVal = PARA_T3;
	memset(m_vBackFrame, 0xff, sizeof(m_vBackFrame));	
	
	
}


BOOL CGb104m::Init(WORD wTaskID)
{
	BOOL rc;
	m_bHaveDd  = FALSE;//add by lqh 20080111
	rc = CPPrimary::Init(wTaskID,1,&m_guiyuepara,sizeof(m_guiyuepara));
	if (!rc)
	{
		return FALSE;
	}
	
	DoCommState();
	
	for (int i = 0; i < m_wEqpNum; i++)
	{
		if (m_pEqpInfo[i].wDDNum)
		{
			m_bHaveDd = TRUE;
			break;
		}
	}
	m_fdno = 0;
	ParaOffset = 0;
	m_w104mTaskID = 0;
	if(m_guiyuepara.tmp[6] > 0)
	{
	    m_fdno = m_guiyuepara.tmp[6];
		m_w104mTaskID = wTaskID;
	}

	if(m_guiyuepara.tmp[0] == 0xFF)
	{
		DD_REAL_START_ADDR = m_guiyuepara.tmp[1];
		DD_FIF_START_ADDR  = m_guiyuepara.tmp[2];
		DD_DAY_START_ADDR  = m_guiyuepara.tmp[3];
		DD_TIDE_START_ADDR = m_guiyuepara.tmp[4];
		DD_START_ADDR      = m_guiyuepara.tmp[5];
	}
	else
	{
		DD_REAL_START_ADDR = DD_CUR_ADDR;
		DD_FIF_START_ADDR  = DD_FIFRZ_ADDR;
		DD_DAY_START_ADDR  = DD_DAYFRZ_ADDR;
		DD_TIDE_START_ADDR = DD_FLOW_ADDR;
		DD_START_ADDR      = ADDR_DD_LO;
	}
	g_ddfifflag=0;
	g_dddayflag=0;
	g_ddtideflag=0;

#if 0
	#ifdef INCLUDE_DF104FORMAT
	m_PARA_K = 8;
	m_PARA_W = 4;
	#else
	m_PARA_K = 12;
	m_PARA_W = 8;
	#endif
#endif
	g_ddhxnum = g_Sys.Eqp.pInfo[m_wEqpID].pTEqpInfo->Cfg.wDDNum/32;

	DWORD event_time = 100;	//100 for 1s
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);

	return TRUE;	
}

void CGb104m::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();
	//if need to modify ,to do : 
	m_pBaseCfg->wMaxRetryCount=0;
	m_pBaseCfg->wMaxErrCount = 1;
}

void CGb104m::SetBaseCfg(void)
{
	CProtocol::SetBaseCfg();
	//if need to modify ,to do :
	m_pBaseCfg->wCmdControl = YKOPENABLED | CLOCKENABLE | ALLDATAPROC;
	
	for (int EqpNo = 0; EqpNo < m_wEqpNum; EqpNo++)//lqh 20030902
	{
		if (m_pEqpInfo[EqpNo].wDDNum  != 0)
		{
			m_pBaseCfg->wCmdControl |= DDPROC;
			break;
		}
		 
	}
	
	m_pBaseCfg->Timer.wAllData = 30;	//30 min
	m_pBaseCfg->Timer.wSetClock = 5;	//5 min
	m_pBaseCfg->Timer.wDD=60;		//60 min
	m_pBaseCfg->wMaxRetryCount=0;
	m_pBaseCfg->wMaxErrCount = 1;
}

void CGb104m::CheckCfg()
{
//	char tt[80];
	if((m_guiyuepara.typeidlen<1)||(m_guiyuepara.typeidlen>2))
		{
		m_guiyuepara.typeidlen=1;
	m_guiyuepara.conaddrlen=2;
	m_guiyuepara.VSQlen=1;
	m_guiyuepara.COTlen=2;
	m_guiyuepara.infoaddlen=3;
	m_guiyuepara.baseyear=2000;
	m_guiyuepara.ddtype=15;
	m_guiyuepara.yctype=9;
	m_guiyuepara.yxtype=1;
	m_guiyuepara.k=12;
	m_guiyuepara.w=8;
		m_guiyuepara.t0=10;
		m_guiyuepara.t1=12;
		m_guiyuepara.t2=5;
		m_guiyuepara.t3=15;
		m_guiyuepara.t4=8;

	}
		m_PARA_K = m_guiyuepara.k;
		m_PARA_W = m_guiyuepara.w;
			m_vTimer[1].bRun = FALSE;
	m_vTimer[1].wInitVal = m_guiyuepara.t1;
	m_vTimer[2].bRun = FALSE;
	m_vTimer[2].wInitVal = m_guiyuepara.t2;
	m_vTimer[3].bRun = FALSE;
	m_vTimer[3].wInitVal = m_guiyuepara.t3;
			if((m_guiyuepara.baseyear!=2000)&&(m_guiyuepara.baseyear!=1900))
			m_guiyuepara.baseyear=2000;
	#if 0
	sprintf(tt,"gb104: len:type:%d,VSQ:%d,COT:%d,Cadd:%d,Iadd:%d,baseyear:%d",\
					m_guiyuepara.typeidlen,m_guiyuepara.VSQlen,m_guiyuepara.COTlen,\
					m_guiyuepara.conaddrlen,m_guiyuepara.infoaddlen,m_guiyuepara.baseyear);
			myprintf(m_wTaskID,LOG_ATTR_INFO, tt);
			#endif

}

void CGb104m::SetDefCfg()
{
		m_guiyuepara.typeidlen=1;
	m_guiyuepara.conaddrlen=2;
	m_guiyuepara.VSQlen=1;
	m_guiyuepara.COTlen=2;
	m_guiyuepara.infoaddlen=3;
	m_guiyuepara.baseyear=2000;
	m_guiyuepara.ddtype=15;
	m_guiyuepara.yctype=9;
	m_guiyuepara.yxtype=1;
	m_guiyuepara.k=12;
	m_guiyuepara.w=8;
		m_guiyuepara.t0=10;
		m_guiyuepara.t1=12;
		m_guiyuepara.t2=5;
		m_guiyuepara.t3=15;
		m_guiyuepara.t4=8;
		//		memcpy(m_pBaseCfg+1,&m_guiyuepara,sizeof(m_guiyuepara));
}




void CGb104m::StartTimer(BYTE TimerNo)
{
	if (TimerNo>3)
   		return;
  	m_vTimer[TimerNo].wCounter = m_vTimer[TimerNo].wInitVal;
  	m_vTimer[TimerNo].bRun = TRUE;	
}

void CGb104m::StopTimer(BYTE TimerNo)
{
	if (TimerNo>3)
   		return;
  	m_vTimer[TimerNo].bRun = FALSE;	
}


void CGb104m::DoTimeOut(void)
{
	CPPrimary::DoTimeOut(); 
	
	if (m_vTimer[1].bRun)
   	{
    	m_vTimer[1].wCounter--;
    	if (m_vTimer[1].wCounter == 0)
     	{      
     		//#ifdef LQH_DEBUG
      		
      		//myprintf(m_wTaskID, LOG_ATTR_INFO, "iec104 master T1 timeout,close TCP!\n");
      		//myprintf(m_wTaskID,  LOG_ATTR_INFO, "m_wAckNum = %d      m_wSendNum = %d", m_wAckNum, m_wSendNum);
      		//#endif		
      		
      		logMsg("iec104 master T1 timeout,close TCP!\n",0,0,0,0,0,0);
      		//to do:when T1 timeout,close tcp connect
      		CloseTcp();
      		    		
      		StopTimer(1);
     	}
   	}
  	if (m_vTimer[2].bRun)
   	{
    	m_vTimer[2].wCounter--;
    	if (m_vTimer[2].wCounter == 0)
     	{
      		//to do:when T2 timeout, send S frame
      		SendSFrame();
      		StopTimer(2);
		}
   	}
  	if (m_vTimer[3].bRun)
   	{
    	m_vTimer[3].wCounter--;
    	if (m_vTimer[3].wCounter == 0)
     	{
     	    //to do:when T3 timeout
     	    SendUFrame(TESTFR_ACT);
      		StopTimer(3);
      	}
   	}  
   	   	
}

BOOL CGb104m::DoYKReq(void)
{
	CPPrimary::DoYKReq();
	
	if (SwitchClearEqpFlag(EQPFLAG_YKReq)) 
		if (SendYkCommand()) 
			return TRUE;
	return TRUE;
}

BOOL CGb104m::DoYTReq(void)
{
	CPPrimary::DoYTReq();
	
	if (SwitchClearEqpFlag(EQPFLAG_YTReq)) 
		if (SendYT()) 
			return TRUE;
	return TRUE;
}


void CGb104m::CloseTcp(void)
{
	StopTimer(1);
	StopTimer(2);
	StopTimer(3);
	
	m_bDTEnable = FALSE;
	m_bTcpConnect = FALSE;
	
	m_wSendNum = 0;	//Send Counter	
	m_wRecvNum = 0;	//Receive Counter
	m_wAckNum = 0;		//Receive Ack Counter
	m_wAckRecvNum = 0; //Have Ack Receive Counter
	
	
	//add close tcp call
	DWORD dwPara;
	commCtrl(m_wCommID, CCC_CONNECT_CTRL | CONNECT_CLOSE, (BYTE*)&dwPara); 
}

//need to be called from Class CProtocol
DWORD CGb104m::DoCommState(void)
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
		logMsg("%02d-%02d %02d:%02d:%02d:%03d Iec104 master TCP client connect!\n",
      			SysTime.byMonth, SysTime.byDay, SysTime.byHour, SysTime.byMinute, SysTime.bySecond,SysTime.wMSecond);
		m_bTcpConnect = TRUE;
		m_bDTEnable = FALSE;
		m_wSendNum = 0;	//Send Counter	
	    m_wRecvNum = 0;	//Receive Counter
	    m_wAckNum = 0;		//Receive Ack Counter
	    m_wAckRecvNum = 0; //Have Ack Receive Counter
	    SetTaskFlag(TASKFLAG_CLOCK);
		SetTaskFlag(TASKFLAG_ALLDATA);
		SetTaskFlag(TASKFLAG_DD);
	    	thSleep(300);//add by lqh 20080329
	    SendUFrame(STARTDT_ACT);
  		  		
  		#ifdef LQH_DEBUG
      		
      		myprintf(m_wTaskID, "iec104 master TCP client connect!");
      		
      	#endif
      	
  		
	}
	else
	{
		m_bTcpConnect = FALSE;
		m_bDTEnable = FALSE;
		
		#ifdef LQH_DEBUG
      		
      		myprintf(m_wTaskID, "iec104 master TCP client DISconnect!");
      		
      	#endif
      	logMsg("%02d-%02d %02d:%02d:%02d:%03d Iec104 master TCP client DISconnect!\n",
      			SysTime.byMonth, SysTime.byDay, SysTime.byHour, SysTime.byMinute, SysTime.bySecond,SysTime.wMSecond);
			
	}

    return dwCommState;	
}


BOOL CGb104m::DoReceive()
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
    //to do :proc ASDUs
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


DWORD CGb104m::SearchOneFrame(BYTE *Buf, WORD Len)
{
	DWORD Rc;
	BYTE FrameLen;
//	WORD Address;
	
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
    
 //   if (FrameLen > (CONTROL_LEN + ASDUID_LEN))
    {
 //   	Address = MAKEWORD(m_pASDU->byAddressLow, m_pASDU->byAddressHigh);
    //	if (SwitchToAddress(Address) == FALSE)
	//		return FRAME_ERR|FrameLen;   	
	}
	return FRAME_OK | (FrameLen + 2); //返回本帧总长度
}


void CGb104m::DoIFrame(void)
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
		StopTimer(1);	//stop T1
	}
	
	if ((m_wRecvNum - m_wAckRecvNum) >= m_PARA_W)
	{
	    SendSFrame();        
	}
	
	switch (m_dwasdu.TypeID)	
	{
	    case M_SP_NA://不带时标的单点信息
	    case 2://不带时标的单点信息
	    case 3://不带时标的单点信息
	    case 4://不带时标的单点信息
	    	DoSingleYx();
	    	break;
	    case M_SP_TB://带长时标的单点信息
	    case 31://带长时标的双点信息
	        DoSingleSoe();
	        break;
	    case M_ME_NA://9
	    case M_ME_TA://10
	    case M_ME_NB://11
	    case M_ME_TB://12
	    case M_ME_ND://不带品质描述的测量值
	    case M_ME_TD://34
	    case M_ME_TE://不带品质描述的测量值
	        DoYcData();
	        break;
		case M_ME_NC://短浮点数
			DoYcFFData();
			break;
	//	case M_ME_NB://add by lqh 2080329
	//		DoYcData_0x0B();
	//		break;
	    case M_IT_NA://电能脉冲记数量
	        DoDdData();
	        break;
	    case C_DC_NA://双点遥控命令
	        DoDoubleYk();
	        break;
		case C_SE_TC_1:  //遥调
		case C_SE_NC_1:
			DoYTF();
	        break;
	    case C_IC_NA://召唤命令
	        DoCallAllData();
	        break;
	    case C_CI_NA://电能脉冲召唤命令
	        DoCallDd();
	        break;
	    case C_CS_NA://时钟同步命令
	    	DoSetTime();
	    	break;
	    case M_FA_NA://FA故障信息
	    	DoFaInfo();
	    	break;
		case M_IT_NB_1://电能量累积量，短浮点数
	        DoDdData();
	        break;
		case M_IT_NC_1://带时标的累积量，短浮点数
	        DoDdDataTime();
	        break;
		 case C_RS_NA_1:   //读参数和定值
            DoReadParaSet();
            break;
		case C_WS_NA_1:   //写参数和定值
            DoWriteParaSet();
            break;
	    default:
	    	break;
	}
}


void CGb104m::DoSFrame(void)
{
	WORD wAckNum;
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);	
	m_wAckNum = wAckNum >> 1;
	
	if (m_wAckNum == m_wSendNum)
	{
		StopTimer(1);	//stop T1
			
	}
	/*
	if (m_wAckNum < m_wSendNum)
	{
		//figure 12,do nothing ,wait T1 timeout
		return;
	}
	else
	{
		SendSFrame();
	}
	*/
}


void CGb104m::DoUFrame(void)
{
	switch (m_pReceive->byControl1)
	{
	    case STARTDT_CON:
	    	StopTimer(1);
	        m_bDTEnable = TRUE;
	        /*m_wSendNum = 0;
	        m_wRecvNum = 0;
	        m_wAckNum = 0;*/
	       /* if (m_bCallAllFlag)
	        {
	        	m_bCallAllFlag = FALSE;
	        	SendCallAllData();
	        	break;
	        }
	        if (m_bCallDdFlag)
	        {
	        	m_bCallDdFlag = FALSE;
	        	SendCallDd();
	        	break;
	        }
	        if (m_bSetTimeFlag)
	        {
	        	m_bSetTimeFlag = FALSE;
	        	SendSetTime();	
	        	break;
	        }*/
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



void CGb104m::DoSingleYx(void)
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

void CGb104m::DoSingleCos(void)
{
	WORD wYxNum;
	WORD wYxBeginNo;
//	BYTE *pYxBuf = NULL;
//	BYTE byLow, byHigh;
		VDBCOS RecCOS;
		VDBDCOS RecDCOS;
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;
//	pYxBuf = (BYTE *)m_dwPubBuf;
	
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	
		wYxNum =m_dwasdu.VSQ&0x7f;
	if(m_dwasdu.VSQ&0x80)
		{
		wYxBeginNo=  getinfoaddr(pData);
	wYxBeginNo = wYxBeginNo - ADDR_YX_LO;
	pData+=m_guiyuepara.infoaddlen;
		}
	for (WORD i = 0; i < wYxNum; i++)
	{
	if((m_dwasdu.VSQ&0x80)==0)
		{
		wYxBeginNo=  getinfoaddr(pData);
	wYxBeginNo = wYxBeginNo - ADDR_YX_LO;
	pData+=m_guiyuepara.infoaddlen;
		}
	if((m_dwasdu.TypeID==1)||(m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==30))
		{
		RecCOS.wNo=wYxBeginNo;
		RecCOS.byValue=SIQ(*pData++);//QDP
    	WriteSCOS(m_wEqpID, 1, &RecCOS);
		}
	if((m_dwasdu.TypeID==3)||(m_dwasdu.TypeID==4)||(m_dwasdu.TypeID==31))
		{
		RecDCOS.wNo=wYxBeginNo;
		DIQ(*pData++,&RecDCOS.byValue1,&RecDCOS.byValue2);//QDP
    	WriteDCOS(m_wEqpID, 1, &RecDCOS);
		}
		if((m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==4))
			pData+=3;
		if((m_dwasdu.TypeID==30)||(m_dwasdu.TypeID==31))
			pData+=7;
	}
	return;		
	
}

void CGb104m::DoSingleGroupYx(void)
{
	WORD wYxNum;
	WORD wYxBeginNo;
	BYTE *pYxBuf = NULL;
//	BYTE byLow, byHigh;
	struct VDYX *buf;
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;
	pYxBuf = (BYTE *)m_dwPubBuf;
	buf=(struct VDYX  *)m_dwPubBuf;
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	
	wYxNum =m_dwasdu.VSQ&0x7f;
	if(m_dwasdu.VSQ&0x80)
	{
		wYxBeginNo=  getinfoaddr(pData);
		wYxBeginNo = wYxBeginNo - ADDR_YX_LO;
		pData+=m_guiyuepara.infoaddlen;
	}
	for (WORD i = 0; i < wYxNum; i++)
	{
		if((m_dwasdu.VSQ&0x80)==0)
		{
			wYxBeginNo=  getinfoaddr(pData);
			wYxBeginNo = wYxBeginNo - ADDR_YX_LO;
			pData+=m_guiyuepara.infoaddlen;
		}
		if((m_dwasdu.TypeID==1)||(m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==30))
		{
			*pYxBuf =SIQ(*pData++);//QDP
			if((m_dwasdu.VSQ&0x80)==0)
				WriteRangeSYX(m_wEqpID, wYxBeginNo, 1, pYxBuf);
			pYxBuf++;	
		}
		if((m_dwasdu.TypeID==3)||(m_dwasdu.TypeID==4)||(m_dwasdu.TypeID==31))
		{
			DIQ(*pData++,&buf->byValue1,&buf->byValue2);//QDP
			if((m_dwasdu.VSQ&0x80)==0)
				WriteRangeDYX(m_wEqpID, wYxBeginNo, 1, buf);
			pYxBuf+=2;	
		}
		if((m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==4))
			pData+=3;
		if((m_dwasdu.TypeID==30)||(m_dwasdu.TypeID==31))
			pData+=7;
	}
	if(m_dwasdu.VSQ&0x80)
	{
		if((m_dwasdu.TypeID==1)||(m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==30))
		{
			WriteRangeSYX(m_wEqpID, wYxBeginNo, wYxNum,(BYTE *) m_dwPubBuf);
		}
		else
			WriteRangeDYX(m_wEqpID, wYxBeginNo, wYxNum, buf);
	}
	return;	
	}


void CGb104m::DoSingleSoe(void)
{
WORD wSoeNum;
	BYTE byLow, byHigh;
//	BYTE YxValue;
	VDBSOE RecSOE; 
	VDBDSOE RecDSOE; 
	VSysClock SysTime;
	VCalClock CalendarTime;	
	struct VDYX p;
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;
		GetSysClock(&SysTime,SYSCLOCK);
	
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	
		wSoeNum =m_dwasdu.VSQ&0x7f;
	if(m_dwasdu.VSQ&0x80)
		{
		RecSOE.wNo=  getinfoaddr(pData);
	RecSOE.wNo = RecSOE.wNo - ADDR_YX_LO;
	pData+=m_guiyuepara.infoaddlen;
		}
	for (WORD i = 0; i < wSoeNum; i++)
	{
	if((m_dwasdu.VSQ&0x80)==0)
		{
		RecSOE.wNo=  getinfoaddr(pData);
	RecSOE.wNo = RecSOE.wNo - ADDR_YX_LO;
	pData+=m_guiyuepara.infoaddlen;
		}
	if((m_dwasdu.TypeID==1)||(m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==30))
		RecSOE.byValue=SIQ(*pData++);//QDP
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
		SysTime.byMinute = (*pData) & 0x3F;
		pData ++;
		SysTime.byHour = (*pData) & 0x1F;
		pData ++;
		SysTime.byDay = (*pData) & 0x1F;
		SysTime.byWeek = (*pData) >> 5;
		pData ++;
		SysTime.byMonth = (*pData) & 0x0F;
		pData ++;
		SysTime.wYear = (*pData&0x7F) + 2000;
		pData ++;
		CalendarTime.dwMinute = CalendarClock(&SysTime);
		CalendarTime.wMSecond = SysTime.wMSecond;
		
		RecSOE.Time.dwMinute = CalendarTime.dwMinute;
		RecSOE.Time.wMSecond = CalendarTime.wMSecond;
		}
	if((m_dwasdu.VSQ&0x80)==0)
		{
			if((m_dwasdu.TypeID==1)||(m_dwasdu.TypeID==2)||(m_dwasdu.TypeID==30))
			{
					WriteRangeSYX(m_wEqpID, RecSOE.wNo, 1, &RecSOE.byValue); //因老的cos不存在了
    			WriteSSOE(m_wEqpID, 1, &RecSOE);
			}
			else
			{
					p.byValue1 = RecDSOE.byValue1;
				  p.byValue2 = RecDSOE.byValue2;
				
				WriteRangeDYX(m_wEqpID, RecSOE.wNo, 1, &p); //因老的cos不存在了 2017年8月2日16:13:24
    				WriteDSOE(m_wEqpID, 1, &RecDSOE);
			}
		}
	}	
	
	
	return;	
}
void CGb104m::DoYcFFData(void)
{
#if 0
	if (m_pASDU->byReasonLow == COT_SPONT)
		DoChangeYcFF();
	else
	{
		DoGroupYcFF();
	}
	return;	
#endif

	if (m_dwasdu.VSQ&0x80)	
	{
		if ((m_pASDU->byReasonLow >= COT_INRO9) && (m_pASDU->byReasonLow <= COT_INRO12))
			DoGroupYcFF();
		if ((m_pASDU->byReasonLow == COT_INTROGEN)||(m_pASDU->byReasonLow == COT_SPONT))
			DoGroupYcFF();
	}
	else
		DoChangeYcFF();
	return; 

}
void CGb104m::DoGroupYcFF(void)
{
	WORD wYcNum;
	WORD wYcBeginNo;
	struct VYCF_F  *pYcValue=(struct VYCF_F  *)m_dwPubBuf;
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;
	float fdata = 0;
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	wYcNum =m_dwasdu.VSQ&0x7f;
	if(m_dwasdu.VSQ&0x80)
	{
		wYcBeginNo=  getinfoaddr(pData);
		wYcBeginNo = wYcBeginNo - ADDR_YC_LO;
		pData+=m_guiyuepara.infoaddlen;
	}
	for (WORD i = 0; i < wYcNum; i++)
	{
		if((m_dwasdu.VSQ&0x80)==0)
		{
			wYcBeginNo=  getinfoaddr(pData);
			wYcBeginNo = wYcBeginNo - ADDR_YC_LO;
			pData+=m_guiyuepara.infoaddlen;
		}
		memcpy(&fdata,pData,sizeof(float));
		pData+=sizeof(float);		
		pYcValue->fValue = fdata;
		pYcValue->byFlag= QDS(*pData++);//QDP
		if((m_dwasdu.VSQ&0x80)==0)
			WriteRangeYCF_F(m_wEqpID, wYcBeginNo, 1, (struct VYCF_F *)m_dwPubBuf);
		pYcValue++;	
	}
	if(m_dwasdu.VSQ&0x80)
		WriteRangeYCF_F(m_wEqpID, wYcBeginNo, wYcNum, (struct VYCF_F *)m_dwPubBuf);
	return;	
}
void CGb104m::DoChangeYcFF(void)
{
	WORD wYcNum;
	WORD wYcBeginNo;
	struct VYCF_F  *pYcValue=(struct VYCF_F  *)m_dwPubBuf;
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;
	float fdata = 0;
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	wYcNum =m_dwasdu.VSQ&0x7f;
	if(m_dwasdu.VSQ&0x80)
	{
		wYcBeginNo=  getinfoaddr(pData);
		wYcBeginNo = wYcBeginNo - ADDR_YC_LO;
		pData+=m_guiyuepara.infoaddlen;
	}
	for (WORD i = 0; i < wYcNum; i++)
	{
		if((m_dwasdu.VSQ&0x80)==0)
		{
			wYcBeginNo=  getinfoaddr(pData);
			wYcBeginNo = wYcBeginNo - ADDR_YC_LO;
			pData+=m_guiyuepara.infoaddlen;
		}
		memcpy(&fdata,pData,sizeof(float));
		pData+=sizeof(float);
		pYcValue->fValue = fdata;
		pYcValue->byFlag= QDS(*pData++);//QDP
		if((m_dwasdu.VSQ&0x80)==0)
			WriteRangeYCF_F(m_wEqpID, wYcBeginNo, 1, (struct VYCF_F *)m_dwPubBuf);
	}
	return;	
}

void CGb104m::DoYcData(void)
{
#if 0
	if (m_pASDU->byReasonLow == COT_SPONT)
		DoChangeYc();
	else
	{
		if ((m_pASDU->byReasonLow >= COT_INRO9) && (m_pASDU->byReasonLow <= COT_INRO12))
			DoGroupYc();
		if (m_pASDU->byReasonLow == COT_INTROGEN)
			DoGroupYc();
	}
	return;	
#endif
	if (m_dwasdu.VSQ&0x80)	
	{
		if ((m_pASDU->byReasonLow >= COT_INRO9) && (m_pASDU->byReasonLow <= COT_INRO12))
			DoGroupYc();
		if ((m_pASDU->byReasonLow == COT_INTROGEN)||(m_pASDU->byReasonLow == COT_SPONT))
			DoGroupYc();
	}
	else
		DoChangeYc();
	return;	
}

void CGb104m::DoChangeYc(void)
{
	
		WORD wYcNum;
	WORD wYcBeginNo;
	//WORD *pYcValue = NULL;
	BYTE byLow, byHigh;
//	BYTE *pData = NULL;
	struct VYCF  *pYcValue=(struct VYCF  *)m_dwPubBuf;
	//pData = m_pASDU->byInfo;
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;
	//pYcValue = (WORD *)m_dwPubBuf;
	
#if 0	
	byLow = *pData;
	pData ++;
	byHigh = *pData;
#ifdef INCLUDE_DF104FORMAT
	pData += 1;
#else
	pData += 2;
#endif
#endif
	wYcNum =m_dwasdu.VSQ&0x7f;
	for (WORD i = 0; i < wYcNum; i++)
	{
	memset((void *)m_dwPubBuf, 0, sizeof(struct VYCF));
		wYcBeginNo=  getinfoaddr(pData);
		wYcBeginNo = wYcBeginNo - ADDR_YC_LO;
	pData+=m_guiyuepara.infoaddlen;
		byLow = *pData;
		pData ++;
		byHigh = *pData;
		pData ++;
		pYcValue->nValue= MAKEWORD(byLow, byHigh);
		if(m_dwasdu.TypeID!=M_ME_ND)
		pYcValue->byFlag= QDS(*pData++);//QDP
		else
			pYcValue->byFlag=1;
		if((m_dwasdu.TypeID==10)||(m_dwasdu.TypeID==12))
			pData+=3;
		if((m_dwasdu.TypeID==M_ME_TD)||(m_dwasdu.TypeID==35))
			pData+=7;
	WriteRangeYCF(m_wEqpID, wYcBeginNo, 1, pYcValue);
	}
	return;	
}

void CGb104m::DoGroupYc(void)
{
	WORD wYcNum;
	WORD wYcBeginNo;
	//WORD *pYcValue = NULL;
	BYTE byLow, byHigh;
//	BYTE *pData = NULL;
	struct VYCF  *pYcValue=(struct VYCF  *)m_dwPubBuf;
	//pData = m_pASDU->byInfo;
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;
	//pYcValue = (WORD *)m_dwPubBuf;
	
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
#if 0	
	byLow = *pData;
	pData ++;
	byHigh = *pData;
#ifdef INCLUDE_DF104FORMAT
	pData += 1;
#else
	pData += 2;
#endif
#endif
	wYcNum =m_dwasdu.VSQ&0x7f;
	if(m_dwasdu.VSQ&0x80)
		{
		wYcBeginNo=  getinfoaddr(pData);
	wYcBeginNo = wYcBeginNo - ADDR_YC_LO;
	pData+=m_guiyuepara.infoaddlen;
		}
	for (WORD i = 0; i < wYcNum; i++)
	{
	if((m_dwasdu.VSQ&0x80)==0)
		{
		wYcBeginNo=  getinfoaddr(pData);
	wYcBeginNo = wYcBeginNo - ADDR_YC_LO;
	pData+=m_guiyuepara.infoaddlen;
		}
		byLow = *pData;
		pData ++;
		byHigh = *pData;
		pData ++;
		pYcValue->nValue= MAKEWORD(byLow, byHigh);
		if(m_dwasdu.TypeID!=M_ME_ND)
		pYcValue->byFlag= QDS(*pData++);//QDP
		else
			pYcValue->byFlag=1;
		if((m_dwasdu.TypeID==10)||(m_dwasdu.TypeID==12))
			pData+=3;
		if((m_dwasdu.TypeID==M_ME_TD)||(m_dwasdu.TypeID==35))
			pData+=7;
	if((m_dwasdu.VSQ&0x80)==0)
	WriteRangeYCF(m_wEqpID, wYcBeginNo, 1, pYcValue);
		pYcValue++;	
	}
	if(m_dwasdu.VSQ&0x80)
	WriteRangeYCF(m_wEqpID, wYcBeginNo, wYcNum, (struct VYCF  *)m_dwPubBuf);
	return;	
}


//add by lqh 20080329
void CGb104m::DoYcData_0x0B(void)
{
	if (m_pASDU->byReasonLow == COT_SPONT)
		DoChangeYc_0x0B();
	else
	{
		if ((m_pASDU->byReasonLow >= COT_INRO9) && (m_pASDU->byReasonLow <= COT_INRO12))
			DoGroupYc_0x0B();
		if (m_pASDU->byReasonLow == COT_INTROGEN)
			DoGroupYc_0x0B();
	}
	return;	
}

//add by lqh 20080329
void CGb104m::DoChangeYc_0x0B(void)
{
	WORD wChangeYcNum;
	WORD wYcNo;
	WORD wYcValue;
	WORD wLow, wHigh;
	BYTE *pData;
	
	pData = m_pASDU->byInfo;
	
	wChangeYcNum = (m_pReceive->byAPDULen - CONTROL_LEN - ASDUID_LEN) / 6;
	for (WORD i = 0; i < wChangeYcNum; i++)
	{
		wLow = *pData;
		pData ++;
		wHigh = *pData;
#ifdef INCLUDE_DF104FORMAT
		pData += 1;
#else
		pData += 2;
#endif
		wYcNo = MAKEWORD(wLow, wHigh) - ADDR_YC_LO;
		wLow = *pData;
		pData ++;
		wHigh = *pData;
		pData += 2;
		wYcValue = MAKEWORD(wLow, wHigh);
		WriteRangeYC(m_wEqpID, wYcNo, 1, (short *)&wYcValue);
	}
	return;	
}


//add by lqh 20080329
void CGb104m::DoGroupYc_0x0B(void)
{
	WORD wYcNum;
	WORD wYcBeginNo;
	WORD *pYcValue = NULL;
	BYTE byLow, byHigh;
	BYTE *pData = NULL;
	
	pData = m_pASDU->byInfo;
	pYcValue = (WORD *)m_dwPubBuf;
	
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	
	byLow = *pData;
	pData ++;
	byHigh = *pData;
#ifdef INCLUDE_DF104FORMAT
	pData += 1;
#else
	pData += 2;
#endif
	wYcBeginNo = MAKEWORD(byLow, byHigh) - ADDR_YC_LO;
	wYcNum = (m_pReceive->byAPDULen - CONTROL_LEN - ASDUID_LEN - INFO_ADR_LEN)/3;
	
	for (WORD i = 0; i < wYcNum; i++)
	{
		byLow = *pData;
		pData ++;
		byHigh = *pData;
		pData += 2;
		*pYcValue = MAKEWORD(byLow, byHigh);
		pYcValue++;	
	}
	WriteRangeYC(m_wEqpID, wYcBeginNo, wYcNum, (short *)m_dwPubBuf);
	return;	
}

void CGb104m::DoDdData(void)
{
	WORD wDdNum;
	WORD wDdNo;
	BYTE byLow, byHigh;
	BYTE byDd1, byDd2, byDd3, byDd4;
	DWORD dwDdValue;
	BYTE *pData=m_pReceive->byASDU+m_dwasdu.Infooff;
	struct VDDFT Ddbuf;
	struct VDDFT * DdValue = (struct VDDFT* )m_dwPubBuf;
	
	wDdNum = m_dwasdu.VSQ & 0x7f;
	if(!(m_dwasdu.VSQ & 0x80))
	{
		for (WORD i = 0; i < wDdNum; i++)
		{
			switch (m_guiyuepara.infoaddlen)
			{
				case 1:
					byLow = *pData;
					pData ++;
					byHigh = 0;
					wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
					break;
				case 2:
					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData ++;
					wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
					break;
				case 3:
					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData += 2;
					wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
					break;
			}
			byDd4 = *pData;
			pData ++;
			byDd3 = *pData;
			pData ++;
			byDd2 = *pData;
			pData ++;
			byDd1 = *pData;
			pData += 2;
			dwDdValue = MAKEDWORD(MAKEWORD(byDd4, byDd3), MAKEWORD(byDd2, byDd1));

			Ddbuf.lValue = (long)dwDdValue;
			Ddbuf.byFlag = 0;
			memcpy(&Ddbuf.Time, 0, sizeof(struct VCalClock));
			WriteRangeDDFT(m_wEqpID,wDdNo,1, &Ddbuf);
		}
	
	}
	else
	{
		switch (m_guiyuepara.infoaddlen)
		{
			case 1:
				byLow = *pData;
				pData ++;
				byHigh = 0;
				wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
				break;
			case 2:
				byLow = *pData;
				pData ++;
				byHigh = *pData;
				pData ++;
				wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
				break;
			case 3:
				byLow = *pData;
				pData ++;
				byHigh = *pData;
				pData += 2;
				wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
				break;
		}
		for(WORD i = 0 ; i < wDdNum; i++)
		{
			byDd4 = *pData;
			pData ++;
			byDd3 = *pData;
			pData ++;
			byDd2 = *pData;
			pData ++;
			byDd1 = *pData;
			pData += 2;

			dwDdValue = MAKEDWORD(MAKEWORD(byDd4, byDd3), MAKEWORD(byDd2, byDd1));
			
			DdValue->lValue = (long)dwDdValue;
			DdValue->byFlag = 0;
			memcpy(&DdValue->Time, 0, sizeof(struct VCalClock));

			DdValue++;
		}
		
		WriteRangeDDFT(m_wEqpID,wDdNo,wDdNum,(struct VDDFT* )m_dwPubBuf);
	
	}
	
	return;
	
}

void CGb104m::DoDdDataTime(void)
{
	WORD wDdNum;
	WORD wDdNo;
	BYTE byLow, byHigh;
	BYTE byDd1, byDd2, byDd3, byDd4;
	DWORD dwDdValue;
	BYTE j = 0;
	BYTE *pData = NULL;
	VSysClock SysTime;
	VCalClock CalendarTime;
	struct VDDFT Ddbuf;
	
	struct VDDFT * DdValue = (struct VDDFT* )m_dwPubBuf;
	
	pData = m_pReceive->byASDU+m_dwasdu.Infooff;
	wDdNum = m_dwasdu.VSQ & 0x7f;

	if(!(m_dwasdu.VSQ & 0x80))
	{
		for (WORD i = 0; i < wDdNum; i++)
		{
			switch (m_guiyuepara.infoaddlen)
			{
				case 1:
					byLow = *pData;
					pData ++;
					byHigh = 0;
					wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
					break;
				case 2:
					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData ++;
					wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
					break;
				case 3:
					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData += 2;
					wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
					break;
			}					
			byDd4 = *pData;
			pData ++;
			byDd3 = *pData;
			pData ++;
			byDd2 = *pData;
			pData ++;
			byDd1 = *pData;
			pData += 2;
			dwDdValue = MAKEDWORD(MAKEWORD(byDd4, byDd3), MAKEWORD(byDd2, byDd1));

			if(m_pASDU->byReasonLow == COT_SPONT)
			{
				for(j = 0; j < g_ddhxnum;j++)
				{
					if(((wDdNo+DD_START_ADDR) >= (DD_FIF_START_ADDR + j*DD_FIFRZ_LINE_NUM)) && ((wDdNo+DD_START_ADDR) < (DD_FIF_START_ADDR + DD_NUM+ j*DD_FIFRZ_LINE_NUM)))
					{
						g_ddfifflag |= 0x01<<j;
					}
				}

				for(j = 0; j < g_ddhxnum;j++)
				{
					if(((wDdNo+DD_START_ADDR) >= (DD_DAY_START_ADDR + j*DD_DAYFRZ_LINE_NUM)) && ((wDdNo+DD_START_ADDR) < (DD_DAY_START_ADDR + DD_NUM+ j*DD_DAYFRZ_LINE_NUM)))
					{
						g_dddayflag |= 0x01<<j;
					}
				}

				for(j = 0; j < g_ddhxnum;j++)
				{
					if(((wDdNo+DD_START_ADDR) >= (DD_TIDE_START_ADDR + j*DD_FLOW_LINE_NUM)) && ((wDdNo+DD_START_ADDR) < (DD_TIDE_START_ADDR + DD_NUM+ j*DD_FLOW_LINE_NUM)))
					{
						g_ddtideflag |= 0x01<<j;
					}
				}
			}
			
			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData ++;
			SysTime.wMSecond = MAKEWORD(byLow, byHigh);
			SysTime.byMinute = (*pData)&0x3F;
			pData ++;
			SysTime.byHour = (*pData)&0x1F;
			pData ++;
			SysTime.byDay = (*pData)&0x1f;
			pData ++;
			SysTime.byMonth = (*pData)&0x0F;
			pData ++;
			if(*pData != 0)
			{
				SysTime.wYear = (*pData&0x7F) + 2000;
			
				CalendarTime.dwMinute = CalendarClock(&SysTime);
				CalendarTime.wMSecond = SysTime.wMSecond;
				DdValue->Time = CalendarTime;
			}
			else
			{
				CalendarTime.dwMinute = 0;
				CalendarTime.wMSecond = 0;
			}
			pData ++;
			
			Ddbuf.lValue = (long)dwDdValue;
			Ddbuf.byFlag = 0x82;
			Ddbuf.Time = CalendarTime;
			WriteRangeDDFT(m_wEqpID,wDdNo,1, &Ddbuf);
			
		}

	}
	else
	{
		 switch (m_guiyuepara.infoaddlen)
		{
			case 1:
				byLow = *pData;
				pData ++;
				byHigh = 0;
				wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
				break;
			case 2:
				byLow = *pData;
				pData ++;
				byHigh = *pData;
				pData ++;
				wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
				break;
			case 3:
				byLow = *pData;
				pData ++;
				byHigh = *pData;
				pData += 2;
				wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
				break;
		}
		for(WORD i = 0 ; i < wDdNum; i++)
		{
			byDd4 = *pData;
			pData ++;
			byDd3 = *pData;
			pData ++;
			byDd2 = *pData;
			pData ++;
			byDd1 = *pData;
			pData += 2;
			DdValue->lValue = MAKEDWORD(MAKEWORD(byDd4, byDd3), MAKEWORD(byDd2, byDd1));
			DdValue->byFlag = 0x82;

			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData ++;
			SysTime.wMSecond = MAKEWORD(byLow, byHigh);
			SysTime.byMinute = (*pData)&0x3F;
			pData ++;
			SysTime.byHour = (*pData)&0x1F;
			pData ++;
			SysTime.byDay = (*pData)&0x1f;
			pData ++;
			SysTime.byMonth = (*pData)&0x0F;
			pData ++;
			if(*pData != 0)
			{
				SysTime.wYear = (*pData&0x7F) + 2000;
			
				CalendarTime.dwMinute = CalendarClock(&SysTime);
				CalendarTime.wMSecond = SysTime.wMSecond;
				DdValue->Time = CalendarTime;
			}
			else
			{
				CalendarTime.dwMinute = 0;
				CalendarTime.wMSecond = 0;
			}
			pData ++;
			
			DdValue++;

			
		}
		
		WriteRangeDDFT(m_wEqpID,wDdNo,wDdNum, (struct VDDFT*)m_dwPubBuf);
				
	}

	return;	
	
}

void CGb104m::DoYTF(void)
{
	if (m_dwasdu.COT  == COT_ACTCON)
		DoYtReturn();
	else
	{
		if (m_dwasdu.COT == COT_DEACTCON)
			DoYtCancelAck();	
	}
	return;	
}

void CGb104m::DoYtReturn(void)
{
	BYTE * pData = m_pASDU->byInfo;
	WORD  DCO;
	WORD  YtNo;
	VYTInfo *pYTInfo;
	
	YtNo = MAKEWORD(pData[0], pData[1]);

	DCO = pData[7];
	
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
		
	YtNo = YtNo - ADDR_YT_LO ;//yt no from 1 start
		   
	pYTInfo->Head.wEqpID = m_wEqpID;
	pYTInfo->Info.wID = YtNo;
	switch (DCO & 0x80)
	{
	    case 0x80:
		    pYTInfo->Head.byMsgID = MI_YTSELECT;
		    break;
	    case 0x00:
		    pYTInfo->Head.byMsgID = MI_YTOPRATE;
		    break;
	    default:
		    break;	
	}
	if (m_pASDU->byReasonLow == COT_DEACTCON)
	{
		pYTInfo->Head.byMsgID = MI_YTCANCEL;	
	}
	
	if (m_pASDU->byReasonLow & COT_PN_BIT)
	{
		pYTInfo->Info.byStatus = CONTROLUNKNOWERROR;
	}
	else
	{
		pYTInfo->Info.byStatus = 0;
	}

	//lw add	
	if(pYTInfo->Head.byMsgID == MI_YTSELECT)
	{
		BYTE Reason;
		BYTE Qualifier = 1;
		BYTE QL = 0;
		VSysClock SysTime;

		QL |= DCO_SE_EXE;
		Reason = COT_ACT;	

		GetSysClock((void *)&SysTime, SYSCLOCK);

		YtNo = pYTInfo->Info.wID + ADDR_YT_LO;

		SendFrameHead(FRM_I);
		Sendframe(C_SE_NC_1, Reason);
		write_VSQ(Qualifier);

		write_infoadd(YtNo);

		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=LOBYTE(LOWORD(pYTInfo->Info.dwValue));
		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=HIBYTE(LOWORD(pYTInfo->Info.dwValue));
		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=LOBYTE(HIWORD(pYTInfo->Info.dwValue));
		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=HIBYTE(HIWORD(pYTInfo->Info.dwValue));

		memcpy((void *)&m_SendBuf.pBuf[ m_SendBuf.wWritePtr ],pData+3,4);

		m_SendBuf.wWritePtr +=4;

		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = QL;

		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(SysTime.wMSecond+SysTime.bySecond*1000)&0xff;
		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(SysTime.wMSecond+SysTime.bySecond*1000)>>8;
		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byMinute;
		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byHour;
		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byDay|(SysTime.byWeek<<5);
		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byMonth;
		//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.wYear%100;
		//	
		SendFrameTail();
	}	
	
//	
//	CPPrimary::DoYTRet();
	return;	
	
}


void CGb104m::DoYtCancelAck(void)
{
	return;	
}
void CGb104m::DoDoubleYk(void)
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

void CGb104m::DoYkReturn(void)
{
	BYTE * pData = m_pASDU->byInfo;
	WORD  DCO;	//遥控命令限定词
	WORD  YkNo; //遥控路号
	if(m_dwasdu.COT ==0xA)
		return;
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
			if (m_pASDU->byReasonLow == COT_ACTCON)
	        {
				 pYKInfo->Head.byMsgID = MI_YKOPRATE;
	        }
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


void CGb104m::DoYkCancelAck(void)
{
	return;	
}


void CGb104m::DoCallAllData(void)
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

void CGb104m::DoAllDataAck(void)
{
	return;	
}

void CGb104m::DoAllDataStop(void)
{
	return;	
}

void CGb104m::DoCallDd(void)
{
	if (m_pASDU->byReasonLow == COT_ACTCON)
		DoDdAck();
	else
	{
		if (m_pASDU->byReasonLow == COT_ACTTERM)
			DoDdStop();	
	}
	return;	
}

void CGb104m::DoDdAck(void)
{
	return;	
}

void CGb104m::DoDdStop(void)
{
	return;	
}


void CGb104m::DoSetTime(void)
{/*
	#ifdef LQH_DEBUG
	if (m_wAckNum == m_wSendNum)
	{
		myprintf(m_wTaskID, LOG_ATTR_INFO,  "iec104 master Receive SetTime and stop T1!!");
		StopTimer(1);	//stop T1
	}
	
    else 
    	myprintf(m_wTaskID, LOG_ATTR_INFO,  "m_wAckNum = %d      m_wSendNum = %d", m_wAckNum, m_wSendNum);  		
      	
      		
    #endif	*/
	return;	
}


void CGb104m::DoFaInfo(void)
{
	WORD wFaInfoNum;
	BYTE *pData = NULL;
	
	pData = m_pASDU->byInfo;
	wFaInfoNum = m_pASDU->byQualifier & 0x7f;
	
	WriteFAInfo(wFaInfoNum, (VFAProcInfo *)pData);
	
	return;	
}

BOOL CGb104m::SendYT() //发送遥调
{
	BYTE Reason;
	BYTE Qualifier = 1;
	WORD YtNo;
	BYTE QL = 0;
	VYTInfo *pYTInfo;
	VSysClock SysTime;

	if(!CanSendIFrame())
	    return FALSE;

	GetSysClock((void *)&SysTime, SYSCLOCK);

	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);

	switch (pYTInfo->Head.byMsgID & 0x7f)
	{
		case MI_YTSELECT:
			QL |= DCO_SE_SELECT;
			Reason = COT_ACT;
			break;
		case MI_YTOPRATE:
			QL |= DCO_SE_EXE;
			Reason = COT_ACT;
			break;
		case MI_YTCANCEL:
			Reason = COT_DEACT;
			break;
	};

	YtNo = pYTInfo->Info.wID + ADDR_YT_LO-1;
	
	SendFrameHead(FRM_I);
	Sendframe(C_SE_NC_1, Reason);
	write_VSQ(Qualifier);
	
	write_infoadd(YtNo);

//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=LOBYTE(LOWORD(pYTInfo->Info.dwValue));
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=HIBYTE(LOWORD(pYTInfo->Info.dwValue));
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=LOBYTE(HIWORD(pYTInfo->Info.dwValue));
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=HIBYTE(HIWORD(pYTInfo->Info.dwValue));

	memcpy((void *)&m_SendBuf.pBuf[ m_SendBuf.wWritePtr ],(void *)&pYTInfo->Info.dwValue,4);
	
	m_SendBuf.wWritePtr +=4;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = QL;

//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(SysTime.wMSecond+SysTime.bySecond*1000)&0xff;
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(SysTime.wMSecond+SysTime.bySecond*1000)>>8;
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byMinute;
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byHour;
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byDay|(SysTime.byWeek<<5);
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byMonth;
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.wYear%100;
	
	SendFrameTail();

	return TRUE;
	
}


BOOL CGb104m::ReqUrgency(void)
{
	
	return FALSE;	
}

BOOL CGb104m::ReqCyc(void)
{
	if (m_pBaseCfg->wCmdControl & CLOCKENABLE)
  	{
    	if (CheckClearTaskFlag(TASKFLAG_CLOCK))
       	{
       		m_bSetTimeFlag = TRUE;
       		//SendUFrame(STARTDT_ACT);
       		SendSetTime();
         	return TRUE;       
       	}  
  
    }
    if (CheckClearTaskFlag(TASKFLAG_ALLDATA)) 
  	{
  		m_bCallAllFlag = TRUE;
  		//SendUFrame(STARTDT_ACT);
  		SendCallAllData();
  		return TRUE;
  	}
  
  	if (CheckClearTaskFlag(TASKFLAG_DD)) 
  	{
  		if (m_bHaveDd)//add by lqh 20080111
  		{
			m_bCallDdFlag = TRUE;
			//SendUFrame(STARTDT_ACT);
			SendCallDd();
  		}
  		return TRUE;
  	}
	return FALSE;
}

void CGb104m::SendSFrame(void)
{
    SendFrameHead(FRM_S);
    SendFrameTail();
    StopTimer(2);	//when send S frame, stop T2
    return;    
}


void CGb104m::SendUFrame(BYTE byControl)
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


void CGb104m::SendFrameHead(BYTE byFrameType)
{
    WORD wSendNum, wRecvNum;
//    VASDU * pSendAsdu;
    
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;//清空发送缓冲区 
	
	m_pSend = (VIec104Frame *)m_SendBuf.pBuf;
//	pSendAsdu = (VASDU *)m_pSend->byASDU;
	
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
            m_wAckRecvNum = m_wRecvNum;//wRecvNum;//lqh 20030902
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
            m_wAckRecvNum = m_wRecvNum;//wRecvNum;//lqh 20030902
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
	#if 0
	pSendAsdu->byAddressLow =  ((VASDU *)&m_pReceive->byASDU)->byReasonHigh; //LOBYTE(GetEqpOwnAddr());
	pSendAsdu->byAddressHigh =0; //HIBYTE(GetEqpOwnAddr());
	pSendAsdu->byReasonHigh = GetEqpOwnAddr();
	pSendAsdu->byAddressLow = GetEqpOwnAddr();
	pSendAsdu->byAddressHigh =0;
	#endif
    m_SendBuf.wWritePtr = APCI_LEN;
	return;
}


BOOL CGb104m::SendFrameTail(void)
{
	m_pSend->byAPDULen = m_SendBuf.wWritePtr - 2;
	SaveSendFrame(m_wSendNum, m_SendBuf.wWritePtr, m_SendBuf.pBuf);
	
	WriteToComm(GetEqpAddr());
	if ((m_pSend->byControl1 & 0x1) == FRM_I)
	{
		#ifdef LQH_DEBUG
      		
      		myprintf(m_wTaskID,  LOG_ATTR_INFO, "iec104 master Send I %04x,start T1!", m_pSend->byASDU[0]);
      		
      	#endif
		m_wSendNum ++;
		//StartTimer(1);		//statr T1
		StopTimer(2);		//when send I frame, stop T2
	}
	return TRUE;
}

void CGb104m::SendSetTime(void)
{
	BYTE TypeID = C_CS_NA;
	BYTE Reason = COT_ACT;
	BYTE Qualifier = 1;
//	VASDU * pSendAsdu;
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
	#if 0
	pSendAsdu = (VASDU *)m_pSend->byASDU;
	pSendAsdu->byTypeID = TypeID;
	pSendAsdu->byReasonLow = Reason;
#ifndef INCLUDE_DF104FORMAT
	pSendAsdu->byReasonHigh = GetOwnAddr();
#endif
	pSendAsdu->byQualifier = Qualifier;
				
	m_SendBuf.wWritePtr += ASDUID_LEN;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Lo
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
#ifndef INCLUDE_DF104FORMAT
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
#endif
#endif
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(MSecond); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(MSecond);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byHour;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (SysTime.byDay & 0x1f) | ((SysTime.byWeek << 5) & 0xe0); 
//#ifdef INCLUDE_DF104FORMAT	
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = ((HIBYTE(SysTime.wYear) << 4) & 0xf0) | (SysTime.byMonth & 0x0f);
//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SysTime.wYear); 
//#else
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMonth;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.wYear - 2000; 
//#endif
	SendFrameTail();
	
	return;	
}

void CGb104m::SendCallAllData(void)
{
	BYTE TypeID = C_IC_NA;
	BYTE Reason = COT_ACT;
	BYTE Qualifier = 1;
//	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return;	
	
	SendFrameHead(FRM_I);
	Sendframe(TypeID, Reason);
	write_VSQ(Qualifier);
	write_infoadd(0);
	#if 0
	pSendAsdu = (VASDU *)m_pSend->byASDU;
	pSendAsdu->byTypeID = TypeID;
	pSendAsdu->byReasonLow = Reason;
#ifndef INCLUDE_DF104FORMAT
	pSendAsdu->byReasonHigh = GetOwnAddr();
#endif
	pSendAsdu->byQualifier = Qualifier;
				
	m_SendBuf.wWritePtr += ASDUID_LEN;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Lo
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
#ifndef INCLUDE_DF104FORMAT
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
#endif
#endif
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0x14; 
	 
	SendFrameTail();
	return;	
}

void CGb104m::SendCallDd(void)
{
	BYTE TypeID = C_CI_NA;
	BYTE Reason = COT_ACT;
	BYTE Qualifier = 1;
//	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return;
	
	SendFrameHead(FRM_I);
		Sendframe(TypeID, Reason);
	write_VSQ(Qualifier);
	write_infoadd(0);
#if 0
	pSendAsdu = (VASDU *)m_pSend->byASDU;
	pSendAsdu->byTypeID = TypeID;
	pSendAsdu->byReasonLow = Reason;
#ifndef INCLUDE_DF104FORMAT
	pSendAsdu->byReasonHigh = GetOwnAddr();
#endif
	pSendAsdu->byQualifier = Qualifier;
				
	m_SendBuf.wWritePtr += ASDUID_LEN;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Lo
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
#ifndef INCLUDE_DF104FORMAT
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
#endif
#endif
	#ifndef GB104_V2002
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0x01;
	#else
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0x05;//for read all dd lqh 0x45; 
	#endif
	SendFrameTail();
	return;
}

BOOL CGb104m::SendYkCommand(void)
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



BOOL CGb104m::DoFaSim(void)
{
	SendFaSimInfo();
	return TRUE;	
}
void CGb104m::SendFaSimInfo(void)
{
	BYTE TypeID = C_FA_SIM;
	BYTE Reason = COT_SPONT;
	BYTE Qualifier = 1;
//	VASDU * pSendAsdu;
	WORD msglen;
	
	if (!CanSendIFrame())
		return;
	
	SendFrameHead(FRM_I);
	Sendframe(TypeID, Reason);
	write_VSQ(Qualifier);
//	write_infoadd(YkNo);
#if 0	
	pSendAsdu = (VASDU *)m_pSend->byASDU;
	pSendAsdu->byTypeID = TypeID;
	pSendAsdu->byReasonLow = Reason;
#ifndef INCLUDE_DF104FORMAT
	pSendAsdu->byReasonHigh = GetOwnAddr();
#endif
	pSendAsdu->byQualifier = Qualifier;
				
	m_SendBuf.wWritePtr += ASDUID_LEN;
#endif
	msglen=m_pMsg->Head.wLength;
	
    #if (_BYTE_ORDER == _BIG_ENDIAN)  //Moto系统
		m_pMsg->Head.wLength=ADJUSTWORD(m_pMsg->Head.wLength);
		m_pMsg->Head.wThID=ADJUSTWORD(m_pMsg->Head.wThID);
		m_pMsg->Head.wEqpID=ADJUSTWORD(m_pMsg->Head.wEqpID);
    #endif

	memcpy((void *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (void *)m_pMsg, msglen);
	m_SendBuf.wWritePtr += msglen; 		
	
	SendFrameTail();
	
	return;	
}



void CGb104m::DelAckFrame(WORD SendNum)
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


void CGb104m::SaveSendFrame(WORD SendNum, WORD FrameLen, BYTE *pFrame)
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


BOOL CGb104m::CanSendIFrame(void)
{
    return (m_bDTEnable && m_bContinueSend);    
}
void CGb104m::Sendframe(BYTE type,WORD cot)
{
	write_typeid(type);

			write_COT((GetOwnAddr()<<8)|cot);
		write_conaddr(GetEqpAddr());

}
void CGb104m::initpara(void)
{
	if((m_guiyuepara.typeidlen<1)||(m_guiyuepara.typeidlen>2))
		{
		m_guiyuepara.typeidlen=1;
	m_guiyuepara.conaddrlen=2;
	m_guiyuepara.VSQlen=1;
	m_guiyuepara.COTlen=2;
	m_guiyuepara.infoaddlen=3;

	}
	
}

void CGb104m::write_typeid(int  data)
{
	m_SendBuf.wWritePtr=6;
		for(BYTE i=0;i<m_guiyuepara.typeidlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}
void CGb104m::write_VSQ(int  data)
{
		for(BYTE  i=0;i<m_guiyuepara.VSQlen;i++)
			{
				m_SendBuf.pBuf[ i+6+m_guiyuepara.typeidlen ]=(data>>(i*8))&0xff;
		}

}
void CGb104m::write_COT(int  data)
{
	m_SendBuf.wWritePtr=6+m_guiyuepara.typeidlen+m_guiyuepara.VSQlen;
	//	for(BYTE i=0;i<m_guiyuepara.COTlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr+0 ]=(data)&0xff;
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr+1]=0;//data>>8;
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr+2]=0;
				m_SendBuf.wWritePtr+=m_guiyuepara.COTlen;
		}

}
void CGb104m::write_conaddr(int  data)
{
	m_SendBuf.wWritePtr=6+m_guiyuepara.typeidlen+m_guiyuepara.VSQlen+m_guiyuepara.COTlen;
		for(BYTE i=0;i<m_guiyuepara.conaddrlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}
void CGb104m::write_infoadd(int  data)
{
		for(BYTE i=0;i<m_guiyuepara.infoaddlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}
void CGb104m::getasdu(void)
{	BYTE off=0;
	if(m_guiyuepara.typeidlen==1)
	{
		m_dwasdu.TypeID=m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.typeidlen==2)
	{
		m_dwasdu.TypeID=MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
			off+=2;
	}
	if(m_guiyuepara.VSQlen==1)
	{
		m_dwasdu.VSQ=m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.VSQlen==2)
	{
		m_dwasdu.VSQ=MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
			off+=2;
	}
	if(m_guiyuepara.COTlen==1)
	{
		m_dwasdu.COT=m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.COTlen==2)
	{
		m_dwasdu.COT=m_pReceive->byASDU[off++];
		//m_sourfaaddr=m_pReceive->byASDU[];
		off++;
		//m_dwasdu.COT=MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
	}
	if(m_guiyuepara.conaddrlen==1)
	{
		m_dwasdu.Address=m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.conaddrlen==2)
	{
		m_dwasdu.Address=MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
			off+=2;
	}
			m_dwasdu.Infooff=off;
	if(m_guiyuepara.infoaddlen==1)
	{
		m_dwasdu.Info=m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.infoaddlen==2)
	{
		m_dwasdu.Info=MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
			off+=2;
	}
	if(m_guiyuepara.infoaddlen==3)
	{
		m_dwasdu.Info=MAKEWORD(m_pReceive->byASDU[off+1], m_pReceive->byASDU[off+2]);
			m_dwasdu.Info<<=8;
			m_dwasdu.Info|=m_pReceive->byASDU[off];
			off+=3;
	}
}
 BYTE CGb104m::QDS(BYTE data)
 	{
	BYTE tt=0;
	if((data&0x80)==0)
		tt|=1;
	if((data&1))
		tt|=0x10;
	if((data&0x10))
		tt|=0x2;
	if((data&0x20))
		tt|=0x4;
	if((data&0x40))
		tt|=0x8;
		return tt;	
 }
  BYTE CGb104m::SIQ(BYTE data)
 	{
	BYTE tt=0;
	if(data&1)
		tt|=0x80;
	if((data&0x10))
		tt|=0x2;
	if((data&0x20))
		tt|=0x4;
	if((data&0x40))
		tt|=0x8;
	if((data&0x80)==0)
		tt|=0x1;
		return tt;	
 }
void  CGb104m::DIQ(BYTE data,BYTE *data1,BYTE *data2)
 	{
	*data1=0;
	*data2=0;
	if((data&0x10))
		*data1|=0x2;
	if((data&0x20))
		*data1|=0x4;
	if((data&0x40))
		*data1|=0x8;
	if((data&0x80)==0)
		*data1|=0x1;
	*data2=*data1;
		if(data&1)
		*data1|=0x80;
		if(data&2)
		*data2|=0x80;
	return ;	
 }
DWORD CGb104m:: getinfoaddr(BYTE *psrc)
{
	DWORD addr=0;
	if(m_guiyuepara.infoaddlen==1)
	{
		addr=*psrc;
	}
	if(m_guiyuepara.infoaddlen==2)
	{
		addr=MAKEWORD(*psrc, *(psrc+1));
	}
	if(m_guiyuepara.infoaddlen==3)
	{
		addr=MAKEWORD( *(psrc+1), *(psrc+2));
			addr<<=8;
			addr|=*(psrc);
	}
	return addr;
}
void CGb104m::DoParaMsg(void)
{
	ParaFlag = m_pMsg->Head.byMsgAttr;
	if(ParaFlag == 0)
	{//预置
		nextnum = m_pMsg->abyData[0];
		nextoffset = m_pMsg->abyData[1];
		SendWriteSetParaVal_Multi();
	}
	else if(ParaFlag == 1)
	{//固化
		SendWriteSetParaSolidify();
	}
	else if(ParaFlag == 2)
	{//取消
		SendWriteSetParaCancel();
	}
	else if(ParaFlag == 3)
	{//读
		ReadNum = m_pMsg->abyData[0];
		if(ReadNum == 0)
			SendReadSetParaAll();
		else
			SendReadSetParaPart();	
	}
}

//写多个定值和参数
void CGb104m::SendWriteSetParaVal_Multi(void)
{
    BYTE TypeID = C_WS_NA_1;
	BYTE Reason = COT_ACT;
	BYTE Qualifier = 1;
    WORD wSetRegionNo = 0;
    BYTE byParaID = 0x3E;
    WORD wInfoNum;
    int i;
    BYTE byParaDataLen = 0;
    wInfoNum = nextnum;

	if (wInfoNum<1)
		return;
	
	if (!CanSendIFrame())
		return;
	
    SendFrameHead(FRM_I);
    Sendframe(TypeID, Reason);
    Qualifier = (BYTE)wInfoNum;
    write_VSQ(Qualifier);
    //write_infoadd(0);//???

    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(wSetRegionNo);
    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(wSetRegionNo);

    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = byParaID | 0x80;

    for(i = 0; i < wInfoNum; i++)
    {
#if (TYPE_CPU == CPU_SAM9X25)

        m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(WriteNextParaInfo[nextoffset+i].infoaddr); //信息体地址
        m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(WriteNextParaInfo[nextoffset+i].infoaddr);
        m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;


        m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = WriteNextParaInfo[nextoffset+i].type;
        m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = WriteNextParaInfo[nextoffset+i].len;
		byParaDataLen = WriteNextParaInfo[nextoffset+i].len;

		memcpy( m_SendBuf.pBuf+m_SendBuf.wWritePtr,&WriteNextParaInfo[nextoffset+i].valuech,byParaDataLen);
#endif
		m_SendBuf.wWritePtr+=byParaDataLen;

    }
    SendFrameTail();

    return;
}

//写参数定值固化
void CGb104m::SendWriteSetParaSolidify(void)
{
    BYTE Qualifier = 1;
    WORD wSetRegionNo = 0;
    BYTE byParaID = 0x3E;

    SendFrameHead(FRM_I);
    Sendframe(C_WS_NA_1, COT_ACT);
    write_VSQ(Qualifier);
    //write_infoadd(0);//???

    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(wSetRegionNo);
    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(wSetRegionNo);
    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = byParaID;

    SendFrameTail();
    return;
}


//写参数和定值确认
void CGb104m::DoWriteParaSet(void)
{
    BYTE *pData;
    //BYTE byParaID;
   // WORD wRegionNo;
    //BYTE byLow, byHigh;

    pData = m_pReceive->byASDU + m_dwasdu.Infooff;
    memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	
	if (COT_ACTCON!= (m_dwasdu.COT & 0xFF))
		return;

    //byLow = *pData;
    pData++;
   // byHigh = *pData;
    pData++;

    //wRegionNo = MAKEWORD(byLow, byHigh);

    //byParaID = *pData;
    pData++;
	writeflag = 0;
#if 0
	if(byParaID & 0x80)
	{//预置
		ParaMsg.Head.wLength = NEXTPARALEN+sizeof(VMsgHead);
		ParaMsg.Head.byMsgID = MI_PARA;
		ParaMsg.Head.byMsgAttr = 0;//0-预置，1-执行，2-取消
		ParaMsg.abyData[0] = m_dwasdu.COT;//返回确认原因
		msgSend(m_w101sTaskID, &ParaMsg, ParaMsg.Head.wLength, 1);
	}
	else
	{//执行
		ParaMsg.Head.wLength = NEXTPARALEN+sizeof(VMsgHead);
		ParaMsg.Head.byMsgID = MI_PARA;
		ParaMsg.Head.byMsgAttr = 1;//0-预置，1-执行，2-取消
		ParaMsg.abyData[0] = m_dwasdu.COT;//返回确认原因
		msgSend(m_w101sTaskID, &ParaMsg, ParaMsg.Head.wLength, 1);

	}
#endif
    return;
}
//写参数定值撤销
void CGb104m::SendWriteSetParaCancel(void)
{
    BYTE Qualifier = 1;
    WORD wSetRegionNo = 0;
    BYTE byParaID = 0x3E;

    SendFrameHead(FRM_I);
    Sendframe(C_WS_NA_1, COT_DEACT);
    write_VSQ(Qualifier);
    //write_infoadd(0);

    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(wSetRegionNo);
    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(wSetRegionNo);
    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = byParaID | 0x40;
	
    SendFrameTail();
    return;
}

//读多个定值和参数
void CGb104m::SendReadSetParaPart(void)
{
    BYTE Qualifier = 1;
    WORD wSetRegionNo = 0;
    WORD wInfoNum;
    int i;

    wInfoNum = ReadNum;
   
    SendFrameHead(FRM_I);
    Sendframe(C_RS_NA_1, COT_ACT);

    Qualifier = (BYTE)wInfoNum;
    write_VSQ(Qualifier);
    //write_infoadd(0);

    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(wSetRegionNo);
    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(wSetRegionNo);

    for(i = 0; i < wInfoNum; i++)
    {
#if (TYPE_CPU == CPU_SAM9X25)
		ReadNextParaInfo[i].infoaddr -=PRPARA_LINE_NUM*m_fdno;
        m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(ReadNextParaInfo[i].infoaddr); //信息体地址
        m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(ReadNextParaInfo[i].infoaddr);
        m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
#endif
    }
    SendFrameTail();
    return;
}

//读全部定值和参数
void CGb104m::SendReadSetParaAll(void)
{
    BYTE Qualifier = 0;
    WORD wSetRegionNo = 0;

    SendFrameHead(FRM_I);
    Sendframe(C_RS_NA_1, COT_ACT);
    write_VSQ(Qualifier);
    //write_infoadd(0);//信息体地址

    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(wSetRegionNo);
    m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(wSetRegionNo);

    SendFrameTail();
    return;
}
//读参数和定值
void CGb104m::DoReadParaSet(void)
{
    BYTE *pData;
    BYTE byParaID;
    //WORD wRegionNo;
    BYTE byInfoNum;
    BYTE byLow, byHigh;
	if ( COT_ACTCON!= (m_dwasdu.COT & 0xFF))
		return;


    byInfoNum = m_dwasdu.VSQ & 0x7F;
    pData = m_pReceive->byASDU + m_dwasdu.Infooff;
    memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));

   	byLow = *pData;
    pData++;
    byHigh = *pData;
    pData++;

    //wRegionNo = MAKEWORD(byLow, byHigh);

    byParaID = *pData;
    pData++;
#if (TYPE_CPU == CPU_SAM9X25)
		WORD m_infoaddr;
		int i;
    for (i = 0; i < byInfoNum; i++)
    {
        byLow = *pData;
        pData++;
        byHigh = *pData;
        pData++;

		m_infoaddr = MAKEWORD(byLow, byHigh);
		if(m_infoaddr<PRPARA_LINE_ADDR)
			continue;

		ReadNextParaInfo[ParaOffset+i].infoaddr = PRPARA_LINE_NUM*m_fdno + MAKEWORD(byLow, byHigh);
		
        pData++;		
		ReadNextParaInfo[ParaOffset+i].type = *pData;
		
        pData++;
		ReadNextParaInfo[ParaOffset+i].len = *pData;

        pData++;
		memcpy(&ReadNextParaInfo[ParaOffset+i].valuech,pData,ReadNextParaInfo[ParaOffset+i].len);
		pData+=ReadNextParaInfo[ParaOffset+i].len;

    }
	ParaOffset+=byInfoNum;
	if(!(byParaID&0x01))
	{
		ParaMsg.Head.wLength = NEXTPARALEN+sizeof(VMsgHead);
		ParaMsg.Head.byMsgID = MI_PARA;
		ParaMsg.Head.byMsgAttr = 3;
		ParaMsg.abyData[0] = m_dwasdu.COT;
		ParaMsg.abyData[1] = ParaOffset;//返回读到的个数
		msgSend(m_w101sTaskID, &ParaMsg, ParaMsg.Head.wLength, 1);
		ParaOffset = 0;
	}
#endif
    return;
}

#endif

