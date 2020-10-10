 
#include "sac103m.h"

static DWORD t1=0,t2=0;
BOOL g_UDPBind = TRUE;


extern "C" void sac103m(WORD wTaskID)		
{
	CGB103P *pGB103P = new CGB103P(); 	
	
	if (pGB103P->Init(wTaskID) != TRUE)
	{
		pGB103P->ProtocolExit();
	}
	
	pGB103P->Run();			   	
}


CGB103P::CGB103P() : CPPrimary()
{
	m_bTcpConnect = FALSE;
	m_byRII = 0;
}


BOOL CGB103P::Init(WORD wTaskID)
{
	BOOL rc;
	DWORD evFlag;
	int i=0;
	rc = CPPrimary::Init(wTaskID,1,NULL,0);
	if (!rc)
	{
		return FALSE;
	}
	m_wTaskID = wTaskID;
	//if (m_vUdp.Create("255.255.255.255", UDP_PORT) < 0)
	//	return FALSE;
	if(g_UDPBind)
	{
		if(m_vUdp.Create()<0)
		{
			myprintf(wTaskID, 0, "udp creat error ");
			return FALSE;
		}
		if(m_vUdp.Bind(UDP_PORT)<0)
		{
			myprintf(wTaskID, 0, "udp Bind error ");
			return FALSE;
		}
		UdpCfgStrSet(wTaskID);
		SendUdpNetId();
		g_UDPBind = FALSE;
	}
	DoCommState();
	
	int tid=tmEvEvery(m_wTaskID, SECTOTICK(10), EV_TM2);  //pre 1 second
	
	while (1)
	{
		evReceive(m_wTaskID, EV_TM2  | EV_COMM_STATUS, &evFlag);
		
//		if (evFlag & EV_COMMERROR)
//		{
//			tmDelete(m_wTaskID, tid);
//			myprintf(m_wTaskID,LOG_ATTR_WARN, "103 Comm init error!");	
//			thSuspend(m_wTaskID);	
//		}
		
		if (evFlag & EV_COMM_STATUS)	
		{
			m_bTcpConnect = TRUE;
			SetClock();
			thSleep(30);
			GetAllData();
			DoCommState();
			thSleep(10);
			break;
		}
		
		if (evFlag & EV_TM2)
		{
		    i++;
			SendUdpNetId();
		}
			
	}
	tmDelete(m_wTaskID, tid);
	
	return TRUE;	
}

void CGB103P::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();
	//if need to modify ,to do : 	
}

void CGB103P::SetBaseCfg(void)
{
	CProtocol::SetBaseCfg();
	//if need to modify ,to do :
	m_pBaseCfg->wCmdControl = YKOPENABLED | CLOCKENABLE |BROADCASTTIME;
	m_pBaseCfg->wBroadcastAddr = 0xFF;
	m_pBaseCfg->Timer.wSetClock=60;	
	m_pBaseCfg->wMaxRetryCount=0;
}



//need to be called from Class CProtocol
DWORD CGB103P::DoCommState(void)
{
	BOOL bTcpState = FALSE;
	DWORD dwCommState;
	
	dwCommState=CPPrimary::DoCommState();
	if (dwCommState == TCP_COMM_ON)
	{
		bTcpState = TRUE;
		WriteDoEvent(NULL, 02, "sac103 slave TCP server Connect %d !",m_wTaskID);
	}
	else
	{
		bTcpState = FALSE;
		WriteDoEvent(NULL, 02, "sac103 slave TCP server disConnect %d !",m_wTaskID);
	}
		
	if (bTcpState && !m_bTcpConnect)
	{
		SetClock();
		thSleep(30);
		GetAllData();
		m_bTcpConnect = bTcpState;
	}
	if (bTcpState && m_bTcpConnect)
	{
		m_bTcpConnect = bTcpState;
	}
	if (!bTcpState)
	{
		SendUdpNetId();	
		m_bTcpConnect = FALSE;
	}
	
	return dwCommState;
}

void CGB103P::DoTimeOut(void)
{
	CPPrimary::DoTimeOut(); 
	if (TimerOn(10))
	{
		if (!m_bTcpConnect)
			SendUdpNetId();	
		//DoCommState();
	}
	/*
	#if (TYPE_USER== USER_JLLY)
	BYTE *pYxBuf = NULL;
	int num;
  	VDBSOE SOE;
	VDBCOS COS;
	
	if (TimerOn(8))
	{
		
	
		pYxBuf = (BYTE *)m_dwPubBuf;
       	 num=ReadRangeSYX(m_wEqpID,BHEVNT_BEGIN_NO, BHEVNT_NUM + GJEVNT_NUM, sizeof(m_dwPubBuf), pYxBuf);		
		for (WORD i = 0; i < num; i++)
		{
			if (*pYxBuf == 0x81)
			{
            			SOE.wNo = BHEVNT_BEGIN_NO + i;
	    			SOE.byValue = 0x01;
             
             			COS.wNo=SOE.wNo;
	            		COS.byValue=SOE.byValue;
	            		WriteSCOS(m_wEqpID, 1, &COS);
		
	            		GetSysClock((void *)&SOE.Time, CALCLOCK);
             			WriteSSOE(m_wEqpID, 1, &SOE);
		    	}
			pYxBuf ++;

		}
	}

	#endif
	*/
	return;
}	

void CGB103P::SendUdpNetId(void)//发送UDP网络标识广播报文
{
	BYTE Buf[41];
	VSysClock SysTime;
	WORD MSecond;
	
	GetSysClock((void *)&SysTime, SYSCLOCK);
	MSecond = SysTime.bySecond * 1000 + SysTime.wMSecond;
	
	

	Buf[0] = 0xff;//表示server端，00表示客户端
	Buf[1] = 0x00;//00表示不包含对时信息，01表示包含对时信息
	Buf[2] = LOBYTE(MSecond);
	Buf[3] = HIBYTE(MSecond);
	Buf[4] = SysTime.byMinute;
	Buf[5] = SysTime.byHour;
	Buf[6] = (SysTime.byDay & 0x1f) | ((SysTime.byWeek << 5) & 0xe0);
	Buf[7] = SysTime.byMonth;
	Buf[8] = LOBYTE(SysTime.wYear);
	Buf[9] = 0x44;
	Buf[10] = 0x53;
	Buf[11] = 0x33;
	Buf[12] = 0x32;
	Buf[13] = 0x30;
	Buf[14] = 0x30;
	Buf[15] = 0x0;
	
	for (int i = 15; i < 41; i++)
	{
		Buf[i] = 0x00;	
	}
	
	//m_vUdp.Send(Buf, 41);//hll
	if(m_vUdp.Send(Buf, 41, "255.255.255.255")<0)
	{
		myprintf(m_wTaskID, 0, "udp send error ");
	}
	return;	
}
void CGB103P::UdpCfgStrSet(int no)
{
    int i;
	char tmpstr[3][30], *str[3];

	memcpy(tmpstr, g_Task[no].CommCfg.Port.cfgstr, sizeof(tmpstr));

	for (i=0; i<3; i++)
	{
		if (strcmp(tmpstr[i], "") == 0)  continue;

		GetStrFromMyFormatStr(tmpstr[i], ':', 3, &str[0], &str[1], &str[2]);
		//if (str[0] != NULL) 
			//tcpChan[log_no].mode = atoi(str[0]);
		if (str[1] != NULL)
		{
			if ((*str[1] != '\0') && (strcmp(str[1], "0.0.0.0") != 0) && (strcmp(str[1], "0") != 0))
			{
				strcpy(UdpHost.ip , str[1]);
			}	
		}	
		if (str[2] != NULL) 
			UdpHost.port = (WORD)atoi(str[2]);		
	}	
}
BOOL CGB103P::DoReceive()
{
	m_SendBuf.wWritePtr = 0;
	m_SendBuf.wReadPtr = 0;
	if (SearchFrame() != TRUE)
	{
		return FALSE;//没找到有效报文
	}
	
	m_pReceive = (V103Frame *)m_RecFrame.pBuf;
	switch (m_pReceive->TypeId)
	{
		case M_GD_NA:
			DoASDU10();
			break;
		default:
			break;
				
	};	
	//SendUDP30s();//30s发送UDP报文
	return TRUE;	
}

DWORD CGB103P::SearchOneFrame(BYTE *Buf, WORD Len)
{
	BYTE FrameLen;
	
	m_pReceive = (V103Frame *)Buf;
	
	if (Len < MIN_FRM_LEN)
		return FRAME_LESS;
	if ((m_pReceive->Qualifier != 0x81) || 
			(m_pReceive->CommAddress != 0x1))//GetEqpAddr()
		return FRAME_ERR | 1;
	FrameLen = GetFrameLen(Buf, Len);
	if (FrameLen > Len)
		return FRAME_LESS;
	if (FrameLen == 0)
		return FRAME_ERR | 1;
	return FRAME_OK | FrameLen;
	
}
void CGB103P::SendUDP30s(void)
{
    struct VSysClock timeRef;
    GetSysClock((void *)&timeRef,SYSCLOCK);
	t2=timeRef.byMinute*60+timeRef.bySecond;
	if(t2<t1)
		t1=t2;
	if((t2-t1)>=30)
	{
		SendUdpNetId();
		t1=t2;
	}
}

BYTE CGB103P::GetFrameLen(BYTE *Buf, WORD Len)
{
	BYTE FrameLen = 0;
	/*BYTE *pData = NULL;*/
	
	/*pData = Buf;*/
	
	switch (Buf[0])
	{
		case M_GD_NA:
			FrameLen = GetASDU10FrameLen(Buf, Len);
			break;
		case M_ID_NA:
			FrameLen = ASDU5_LEN;
			break;
		default:
			break;	
	}
	
	return FrameLen;	
}


BYTE CGB103P::GetASDU10FrameLen(BYTE *Buf, WORD Len)
{
	BYTE FrameLen = 0;
	BYTE *pData = NULL;
	BYTE GroupNum, GroupDataLen;
	
	pData = Buf;
	
	pData += 7;//此时指向了 通用分类数据集数目（NGD）
	GroupNum = *pData;//组数
	pData ++;
	FrameLen += 8;
	for (int i = 0; i < GroupNum; i++)
	{
		GroupDataLen = pData[4] * pData[5];
		FrameLen += (6 + GroupDataLen);	//6个字节是固定的，GroupDataLen数据长度
		pData += (6 + GroupDataLen);
	}
	
	
	return FrameLen;	
}


void CGB103P::DoASDU10(void)//通用分类标识数据
{
	BYTE *pData = NULL;
	BYTE GroupNum, GroupDataLen;
	BYTE InfoNo;
	pData = &(m_pReceive->Info);
	GroupNum = pData[3];//数据组的数目
	InfoNo = pData[1];
	pData += 4;
	
	for (int i = 0; i < GroupNum; i++)
	{
		GroupDataLen = pData[4] * pData[5];
		switch (pData[0])
		{
			case GRP_EVENT_STATUS://0x04
				DoBhEvent(pData, GroupDataLen);
				break;
			case GRP_EVENT_ALARM://0x05
				DoGjEvent(pData, GroupDataLen);
				break;
			case GRP_YC_COM://0x07
				DoYcCom(pData, GroupDataLen);
				break;
			case GRP_YX_STATUS://0x08
			case GRP_YX_COM://0x09
				DoYxStatus(pData, GroupDataLen);
				break;
			case GRP_YK_YB://0x0e
				DoYkYb(pData, GroupDataLen);
				break;
			case GRP_YX_SOE://0x18
				DoSoe(pData, GroupDataLen);
				break;
			case GRP_YK_KG://0x0b
				DoYkReturn(pData, InfoNo);
				break;
			case GRP_YK_FT:
				DoYkFt(pData, GroupDataLen);
				break;
			case GRP_YT://0x0d
				DoYtReturn(pData, InfoNo);
				break;
			default:
				break;	
		}	
		pData += (6 + GroupDataLen);
	}
	return;	
}

void CGB103P::DoBhEvent(BYTE *Buf, BYTE DataLen)
{
	VDBSOE SOE;
	VDBCOS COS;
	VSysClock SysTime;
	VCalClock CalendarTime;
	WORD MSecond;
	
	if ((Buf[1] == 0) || (Buf[1] > BHEVNT_NUM))
		return;
	if (Buf[2] != 1)
		return;
	SOE.wNo = BHEVNT_BEGIN_NO + Buf[1] - 1;
	
	//change by wyj
	/*写复归信息*/
	if (Buf[6] == 2)		
		SOE.byValue = 0x81;
	else 
		SOE.byValue = 0x01;

    /*因为保护故障消失后马上产生复归，为了使得主站可以收到，此处不写复归信息，可以通过遥控0进行复归*/
	/*if (Buf[6] == 2)		
		SOE.byValue = 0x81;
	else
	#if (TYPE_USER == USER_SHANGHAIJY)
	 	SOE.byValue = 0x01;	    
	#else
		return;
	#endif*/
	COS.wNo=SOE.wNo;
	COS.byValue=SOE.byValue;
	WriteSCOS(m_wEqpID, 1, &COS);
		
	GetSysClock((void *)&SysTime, SYSCLOCK);
	MSecond = MAKEWORD(Buf[7], Buf[8]);
	SysTime.wMSecond = MSecond % 1000;
	SysTime.bySecond = MSecond / 1000;
	SysTime.byMinute = Buf[9];
	SysTime.byHour = Buf[10];	
	CalendarTime.dwMinute = CalendarClock(&SysTime);
	CalendarTime.wMSecond = SysTime.wMSecond;
	SOE.Time.dwMinute = CalendarTime.dwMinute;
	SOE.Time.wMSecond = CalendarTime.wMSecond;
	WriteSSOE(m_wEqpID, 1, &SOE);
	return;	
}

void CGB103P::DoGjEvent(BYTE *Buf, BYTE DataLen)
{
	VDBSOE SOE;
	VDBCOS COS;
	VSysClock SysTime;
	VCalClock CalendarTime;
	WORD MSecond;
	
	if ((Buf[1] == 0) || (Buf[1] > GJEVNT_NUM))
		return;
	if (Buf[2] != 1)
		return;
	SOE.wNo = GJEVNT_BEGIN_NO + Buf[1] - 1;

    //change by wyj
	/*写复归信息*/
	if (Buf[6] == 2)		
		SOE.byValue = 0x81;
	else 
		SOE.byValue = 0x01;

    /*因为保护故障消失后马上产生复归，为了使得主站可以收到，此处不写复归信息，可以通过遥控0进行复归*/
	/*if (Buf[6] == 2)		
		SOE.byValue = 0x81;
	else 
	#if (TYPE_USER == USER_JLLY)
	 	SOE.byValue = 0x01;	    
	#else
		return;
	#endif*/

	COS.wNo=SOE.wNo;
	COS.byValue=SOE.byValue;
	WriteSCOS(m_wEqpID, 1, &COS);
    
	GetSysClock((void *)&SysTime, SYSCLOCK);
	MSecond = MAKEWORD(Buf[7], Buf[8]);
	SysTime.wMSecond = MSecond % 1000;
	SysTime.bySecond = MSecond / 1000;
	SysTime.byMinute = Buf[9];
	SysTime.byHour = Buf[10];	
	CalendarTime.dwMinute = CalendarClock(&SysTime);
	CalendarTime.wMSecond = SysTime.wMSecond;
	SOE.Time.dwMinute = CalendarTime.dwMinute;
	SOE.Time.wMSecond = CalendarTime.wMSecond;	
	WriteSSOE(m_wEqpID, 1, &SOE);
	return;	
}


void CGB103P::DoYcCom(BYTE *Buf, BYTE DataLen)
{
	short YcTempValue;
	long ftmp;
        
	if ((Buf[1] == 0) || (Buf[1] > YC_NUM))
		return;
	if (Buf[2] != 1)
		return;
	if (Buf[4] == 4)
	{
  		ftmp=F754ToLong(&Buf[6],100);
    	
		if(ftmp <0)
		{
			ftmp=-ftmp;
			YcTempValue = 65536-ftmp;
		}
		else
		    YcTempValue = ftmp;
            
        YcTempValue =(short)ftmp;
	}
	else
	{
       	YcTempValue = MAKEWORD(Buf[6], Buf[7]);
 	    if (YcTempValue & 0x1000)
			YcTempValue = 0 - (YcTempValue & 0xfff);
	    else
			YcTempValue = YcTempValue & 0xfff;
	}
	WriteRangeYC(m_wEqpID, Buf[1]-1, 1, (short *)&YcTempValue);
	return;	
}

void CGB103P::DoYxStatus(BYTE *Buf, BYTE DataLen)
{
	BYTE YxValue;
	
	if ((Buf[1] == 0) || (Buf[1] > YX_NUM))
		return;
	if (Buf[2] != 1)
		return;
	if (Buf[6] == 2)		
		YxValue = 0x81;
	else 
		YxValue = 0x01;
		
	WriteRangeSYX(m_wEqpID, Buf[1]-1, 1, &YxValue);
	return;	
}

void CGB103P::DoYkYb(BYTE *Buf, BYTE DataLen)
{
	BYTE YxValue;
	
	if ((Buf[1] == 0) || (Buf[1] > YB_NUM))
		return;
	if (Buf[2] != 1)
		return;
	if (Buf[6] == 2)		
		YxValue = 0x81;
	else 
		YxValue = 0x01;
		
	WriteRangeSYX(m_wEqpID, YB_BEGIN_NO + Buf[1] - 1, 1, &YxValue);
	return;	
}


void CGB103P::DoSoe(BYTE *Buf, BYTE DataLen)
{
	VDBSOE SOE;
	VSysClock SysTime;
	VCalClock CalendarTime;
	WORD MSecond;
	
	if (Buf[1] == 0)
		return;
	if (Buf[2] != 1)
		return;
	SOE.wNo = Buf[1] - 1;
	if (Buf[6] == 2)		
		SOE.byValue = 0x81;
	else 
		SOE.byValue = 0x01;
	GetSysClock((void *)&SysTime, SYSCLOCK);
	MSecond = MAKEWORD(Buf[7], Buf[8]);
	SysTime.wMSecond = MSecond % 1000;
	SysTime.bySecond = MSecond / 1000;
	SysTime.byMinute = Buf[9];
	SysTime.byHour = Buf[10];	
	CalendarTime.dwMinute = CalendarClock(&SysTime);
	CalendarTime.wMSecond = SysTime.wMSecond;
	SOE.Time.dwMinute = CalendarTime.dwMinute;
	SOE.Time.wMSecond = CalendarTime.wMSecond;
	WriteSSOE(m_wEqpID, 1, &SOE);
	return;	
}


void CGB103P::DoYkFt(BYTE *Buf, BYTE DataLen)
{
	WORD YcTempValue;
	if ((Buf[1] == 0) || (Buf[1] > YKFT_NUM))
		return;
	if (Buf[2] != 1)
		return;
	YcTempValue = MAKEWORD(Buf[6], Buf[7]);
	YcTempValue = YcTempValue & 0xfff;
	WriteRangeYC(m_wEqpID, YKFT_BEGIN_NO+Buf[1]-1, 1, (short *)&YcTempValue);
	return;	
}



BOOL CGB103P::DoYKReq(void)
{
	CPPrimary::DoYKReq();
	
	if (SwitchClearEqpFlag(EQPFLAG_YKReq)) 
		if (SendYkCommand()) 
			return TRUE;
	return TRUE;
}

void CGB103P::DoYkReturn(BYTE *Buf, BYTE InfoNo)
{
	WORD  YkNo; //遥控路号
	YkNo = Buf[1];
	
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	
	//add yk return info proc
	
	pYKInfo->Head.wEqpID = m_wEqpID;
	pYKInfo->Info.wID = YkNo;
	switch (InfoNo)
	{
	    case 249:
		    pYKInfo->Head.byMsgID = MI_YKSELECT;
		    break;
	    case 250:
		    pYKInfo->Head.byMsgID = MI_YKOPRATE;
		    break;
	    default:
		    break;	
	}
	switch (Buf[6])
	{
	    case 0: 
	    case 3:
	    	pYKInfo->Info.byStatus = 4; 
		    return;
	    case 1: 
		    pYKInfo->Info.byValue = 0x06;
		    break; //分
	    case 2: 
		    pYKInfo->Info.byValue = 0x05;
		    break; //合
	    default:
	    break;
	}
	if ((m_pReceive->Reason == COT_GCWRACK) ||
			(m_pReceive->Reason == COT_GCWRCON))
		pYKInfo->Info.byStatus = 0;
	else 
		pYKInfo->Info.byStatus = 4;
	
	CPPrimary::DoYKRet();
	
	return;	
	
}

BOOL CGB103P::SendYkCommand(void)
{
	BYTE YkStatus, InfoNo, YkNo;
	BYTE *pYxBuf = NULL;
	int num;
  	VDBSOE SOE;
	VDBCOS COS;
	
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	
	YkNo = pYKInfo->Info.wID;
	
	if (YkNo == BHFG_YKNO)
	{
        pYxBuf = (BYTE *)m_dwPubBuf;
        num=ReadRangeSYX(m_wEqpID,BHEVNT_BEGIN_NO, BHEVNT_NUM + GJEVNT_NUM, sizeof(m_dwPubBuf), pYxBuf);		
		for (WORD i = 0; i < num; i++)
		{
			if (*pYxBuf == 0x81)
			{
            	SOE.wNo = BHEVNT_BEGIN_NO + i;
	    		SOE.byValue = 0x01;
             
             	COS.wNo=SOE.wNo;
	            COS.byValue=SOE.byValue;
	            WriteSCOS(m_wEqpID, 1, &COS);
		
	            GetSysClock((void *)&SOE.Time, CALCLOCK);
             	WriteSSOE(m_wEqpID, 1, &SOE);
		    }
			pYxBuf ++;
		}

        /*保护复归*/
    	SendFrameHead();
	
    	m_pSend->TypeId = C_GD_NA;
    	m_pSend->Qualifier = 0x81;
    	m_pSend->Reason = COT_GCWRITE;
    	m_pSend->CommAddress = 1;//GetBroadcastAddr();
	
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = FUNTYPE_GEN;
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 250; 
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //RII
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1; 
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = GRP_SYS;
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 6;      //reset
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1; 
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 10;
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1;
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1; 
    	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1; 
	 
    	SendFrameTail();
	
		pYKInfo->Info.byStatus = 0;
		CPPrimary::DoYKRet();	
		return TRUE;
	}
	
	switch (pYKInfo->Info.byValue & 0x3)
	{
		case 0://不区分合分
			YkStatus = 0;
			break;
		case 1://close合闸
			YkStatus = 2;
			break;
		case 2://open分闸
			YkStatus = 1;
			break;
		case 3://非法
			YkStatus = 0;
			break;	
	};
	switch (pYKInfo->Head.byMsgID & 0x7f)
	{
		case MI_YKSELECT:
			InfoNo = 249;
			break;
		case MI_YKOPRATE:
			InfoNo = 250;
			break;
		case MI_YKCANCEL:
			break;
	};
	
	SendFrameHead();
	
	m_pSend->TypeId = C_GD_NA;
	m_pSend->Qualifier = 0x81;
	m_pSend->Reason = COT_GCWRITE;
	m_pSend->CommAddress = 1;//GetBroadcastAddr();
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = FUNTYPE_GEN;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = InfoNo; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //RII
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = GRP_YK_KG;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = YkNo; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 9;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = YkStatus; 
	
	 
	SendFrameTail();
	
	return TRUE;	
}


BOOL CGB103P::DoYTReq(void)
{
	CPPrimary::DoYTReq();
	
	if (SwitchClearEqpFlag(EQPFLAG_YTReq)) 
		if (SendYtCommand()) 
			return TRUE;
	return TRUE;
}

void CGB103P::DoYtReturn(BYTE *Buf, BYTE InfoNo)
{
	WORD  YtNo; //遥调路号
	YtNo = Buf[1];
	
	VYTInfo *pYTInfo;
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	
	pYTInfo->Head.wEqpID = m_wEqpID;
	pYTInfo->Info.wID = YtNo;
	pYTInfo->Info.dwValue = m_wYtValue;
	
	switch (InfoNo)
	{
	    case 249:
		    pYTInfo->Head.byMsgID = MI_YTSELECT;
		    break;
	    case 250:
		    pYTInfo->Head.byMsgID = MI_YTOPRATE;
		    break;
	    default:
		    break;	
	}
	 
	if ((m_pReceive->Reason == COT_GCWRACK) ||
			(m_pReceive->Reason == COT_GCWRCON))
		pYTInfo->Info.byStatus = 0;
	else 
		pYTInfo->Info.byStatus = 4;
	
	CPPrimary::DoYTRet();
	
	/*为实现YT的两步走,特增加选择过程 add begin*/
	if (pYTInfo->Head.byMsgID == MI_YTSELECT)
		SendYtCommand();
	/*add end*/ 
		return;	
	
}

BOOL CGB103P::SendYtCommand(void)
{
	BYTE InfoNo, YtNo;
  	WORD YtValue;
	VYTInfo *pYTInfo;
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	
	YtNo = pYTInfo->Info.wID;
	YtValue = (WORD)pYTInfo->Info.dwValue;
    m_wYtValue = YtValue;
	
	/*为实现YT的两步走,特增加选择过程 add begin*/
	if (pYTInfo->Head.byMsgID == MI_YTOPRATE)
		pYTInfo->Head.byMsgID = MI_YTSELECT;
	else pYTInfo->Head.byMsgID = MI_YTOPRATE; /*add end*/  
	
	switch (pYTInfo->Head.byMsgID & 0x7f)
	{
		case MI_YTSELECT:
			InfoNo = 249;
			break;
		case MI_YTOPRATE:
			InfoNo = 250;
			break;
		case MI_YTCANCEL:
			break;
	};
	
	SendFrameHead();
	
	m_pSend->TypeId = C_GD_NA;
	m_pSend->Qualifier = 0x81;
	m_pSend->Reason = COT_GCWRITE;
	m_pSend->CommAddress = 1;//GetBroadcastAddr();
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = FUNTYPE_GEN;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = InfoNo; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //RII
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = GRP_YT;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = YtNo; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 9;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YtValue); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YtValue);
	 
	SendFrameTail();
	
	return TRUE;	
}


BOOL CGB103P::ReqCyc(void)
{
	if ((m_pBaseCfg->wCmdControl&CLOCKENABLE)&&(m_pBaseCfg->wCmdControl & BROADCASTTIME))
	{
		if (CheckClearTaskFlag(TASKFLAG_CLOCK))
		{
			//广播对钟
			if (SetClock()==TRUE) return TRUE;	   
		}  
	}
	return FALSE;
}
	

BOOL CGB103P::GetTypeId(void)
{
	SendFrameHead();
	m_pSend->TypeId = C_GRC_NA;
	m_pSend->Qualifier = 0x81;
	m_pSend->Reason = COT_START;
	m_pSend->CommAddress = 1;//GetBroadcastAddr();
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = FUNTYPE_GLB;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 4; //功能类型
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息序号
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //DCO
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_byRII; //RII
	m_byRII ++;
	
	SendFrameTail();
	return TRUE;	
}

BOOL CGB103P::SetClock(void)
{
	VSysClock SysTime;
	WORD MSecond;
	
	GetSysClock((void *)&SysTime, SYSCLOCK);
	MSecond = SysTime.bySecond * 1000 + SysTime.wMSecond;
	
	SendFrameHead();
	m_pSend->CommAddress = 1;//GetBroadcastAddr();
	m_pSend->TypeId = M_SYN_TA;
	m_pSend->Qualifier = 0x81;
	m_pSend->Reason = COT_SYNTIME;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = FUNTYPE_GLB;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(MSecond); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(MSecond);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byHour;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (SysTime.byDay & 0x1f) | ((SysTime.byWeek << 5) & 0xe0); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMonth;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.wYear - 2000;
	
	SendFrameTail();
	return TRUE;	
}

//发总招数据
BOOL CGB103P::GetAllData(void)
{
	SendFrameHead();
	m_pSend->TypeId = C_IGI_NA;//ASDU7
	m_pSend->Qualifier = 0x81;
	m_pSend->Reason = COT_CALL;//9,原因总查询
	m_pSend->CommAddress = 1;//GetBroadcastAddr();//应用服务单元公共地址
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = FUNTYPE_GLB;//全局功能类型255
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息序号
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_byRII; //扫描序号
	m_byRII ++;
	
	SendFrameTail();
	return TRUE;	
}

/*BOOL CGB103P::SendASDU20(void)
{
	SendFrameHead();
	m_pSend->TypeId = C_GRC_NA;//ASDU7
	m_pSend->Qualifier = 0x81;
	m_pSend->Reason = COT_START;
	m_pSend->CommAddress = 1;//GetBroadcastAddr();//应用服务单元公共地址
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = FUNTYPE_GLB;//全局功能类型255
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息序号
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_byRII; //扫描序号
	m_byRII ++;
	
	SendFrameTail();
	return TRUE;	
}*/
void CGB103P::SendFrameHead(void)
{
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;//清空发送缓冲区 
	
	m_pSend = (V103Frame *)m_SendBuf.pBuf;
	
	m_pSend->CommAddress = 1;//GetEqpAddr();
	
	m_SendBuf.wWritePtr += 4;
	return;	
}



void CGB103P::SendFrameTail(void)
{
	WriteToComm(GetEqpAddr());
	return;	
}


