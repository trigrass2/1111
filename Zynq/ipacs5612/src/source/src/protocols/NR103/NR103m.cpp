#include "NR103m.h"


extern "C" void NR103m(WORD wTaskID)		
{
	CNR103m *pNR103P = new CNR103m();	
	
	if (pNR103P->Init(wTaskID) != TRUE)
	{
		pNR103P->ProtocolExit();
	}
	
	pNR103P->Run();			   	
}


CNR103m::CNR103m() : CPPrimary()
{
	pCfg = new V103Para;
	memset(pCfg,0,sizeof(V103Para));
}

BOOL CNR103m::Init(WORD wTaskID)
{
	BOOL rc;
	int k ,w =0;
	VDBCOS yx;
	rc = CPPrimary::Init(wTaskID,1,(void *)pCfg,sizeof(V103Para));
	if (!rc)
	{
		return FALSE;
	}
	m_YKflag = 0;
	m_YKState = 0;
	event_time = m_pBaseCfg->Timer.wScanData2/10;//二级数据轮询时间可设置
	m_pReceive = (NRFrameHead*)m_RecFrame.pBuf;
	m_pSend	= (NRFrameHead *)m_SendBuf.pBuf;
	m_DataNo = 0;
	m_HeartBeat = 0;
	m_dwTestTimerCount = 0;
	for (k=0; k<m_wEqpNum; k++)
	{
	    for (w=0; w<m_pEqpInfo[k].wSYXNum; w++)
		{
		    yx.wNo = w;
			yx.byValue = 0x01;
			WriteSCOS(m_pEqpInfo[k].wEqpID,1, &yx);
		}
		WriteCommStatus(m_wTaskID, m_pEqpInfo[k].wEqpID, FALSE);
	}
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	return TRUE;
}

void CNR103m::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();  
}

void CNR103m::SetBaseCfg(void)
{
    CProtocol::SetBaseCfg();
    //if need to modify ,to do :

    m_pBaseCfg->wCmdControl = YKOPENABLED | CLOCKENABLE | DDPROC;
    m_pBaseCfg->Timer.wAllData = 5; 		// 5 min
    m_pBaseCfg->Timer.wSetClock = 60; 	//60 min
    m_pBaseCfg->Timer.wDD = 20;             //20 min insted for call Soe
    m_pBaseCfg->wMaxRetryCount = 1;
    m_pBaseCfg->wMaxErrCount = 3;
    m_pBaseCfg->wMaxALLen = 255;
    m_pBaseCfg->wBroadcastAddr = 0x0;
}

void CNR103m::CheckCfg(void)
{
	CProtocol::CheckCfg();
}
DWORD CNR103m::DoCommState(void)
{
	DWORD dwCommState;
	BOOL bTcpState = FALSE;
	VSysClock SysTime;

    	dwCommState=CPPrimary::DoCommState();
	if (dwCommState == 1)
		bTcpState = TRUE;
	else
		bTcpState = FALSE;

	GetSysClock((void *)&SysTime, SYSCLOCK);	
	if (bTcpState)
	{
		
		WriteDoEvent(NULL,0,"Eqpaddr:%d NR103 master TCP client connect!",GetEqpAddr());
		logMsg("%02d-%02d %02d:%02d:%02d:%03d NR103 master TCP client connect!\n",
      			SysTime.byMonth, SysTime.byDay, SysTime.byHour, SysTime.byMinute, SysTime.bySecond,SysTime.wMSecond);
	}
	else
	{
			WriteDoEvent(NULL,0,"Eqpaddr:%d NR103 master TCP client DISconnect!",GetEqpAddr());
      		logMsg("%02d-%02d %02d:%02d:%02d:%03d NR103 master TCP client DISconnect!\n",
      			SysTime.byMonth, SysTime.byDay, SysTime.byHour, SysTime.byMinute, SysTime.bySecond,SysTime.wMSecond);
	}
	return dwCommState;
}
/***************************************************************
	Function：OnTimeOut
		定时处理
	参数：TimerID
		TimerID 定时器ID
	返回：无
***************************************************************/
void CNR103m::DoTimeOut(void)
{
	CPPrimary::DoTimeOut();
	m_dwTestTimerCount++;
	m_HeartBeat++;
	if(m_HeartBeat >= 10)
	{
		m_HeartBeat = 0;
		SendHeart();
		DisableRetry();
	}
	
	if(TimerOn(m_pBaseCfg->Timer.wAllData*60))
	{
		SetAllEqpFlag(EQPFLAG_YC);
		//commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	}
	if(TimerOn(m_pBaseCfg->Timer.wSetClock*60))
	{
		SetTaskFlag(TASKFLAG_CLOCK);
	}	
	if(TimerOn(m_pBaseCfg->Timer.wDD*60))
	{
		SetAllEqpFlag(EQPFLAG_DD);
	}
		
}
DWORD CNR103m::SearchOneFrame(BYTE *Buf, WORD Len)
{
	unsigned short FrameLen;
//	WORD wLinkAddress;
	m_pReceive = (NRFrameHead *)Buf;

	if(Len < sizeof(NRFrameHead))
		return FRAME_LESS;	
	if((m_pReceive->wFirstFlag != 0xEB90)||(m_pReceive->wSecondFlag != 0xEB90))
		return FRAME_ERR|1;  
	if (m_pReceive->wLength >Len)
		return FRAME_LESS;
	FrameLen = m_pReceive->wLength + 8;
	if (SwitchToAddress(m_pReceive->wSourceAddr) == FALSE)
		return FRAME_ERR | FrameLen;
	if (m_pReceive->wDestinationAddr !=  GetOwnAddr())
		return FRAME_ERR | FrameLen;	

	return FRAME_OK | FrameLen;	

}
/***************************************************************
	Function：DoReceive
			   接收报文处理
	参数：无		
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CNR103m::DoReceive()
{
	if (SearchFrame() != TRUE)
	{
		return FALSE;
	}
	m_dwTestTimerCount = 0;
	m_pReceive = (NRFrameHead *)m_RecFrame.pBuf;
	
	switch(m_pReceive->bTYPE)
	{
		case TYPE_ASDU05H_FLAG 				:	//标识
			break;
		case TYPE_ASDU0AH_GEN_GROUPDATA	:	//通用分类数据响应命令（装置响应的读一个组的描述）
		case TYPE_ASDU0BH_GEN_DATA			:	//通用分类数据响应命令（装置响应的读单个条目的目录）
			DoGenGroupData_Asdu0A();
			break;

		case TYPE_ASDU1CH_SOE				:	//带标志的状态变位传输准备就绪

			break;

		case TYPE_ASDU1DH_SOE				:	//传送带标志的状态变位
			break;

		default:
			break;

	}
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);

	return FALSE;
}
/***************************************************************
	Function：	Chose_Dev
				找到fun，inf对应的序号
	参数:
	返回：	无
***************************************************************/
BOOL CNR103m::Chose_Dev(WORD no,BYTE bFun,BYTE bInf,BYTE *Gn,BYTE Flag)
{
	int num = 0;
	*Gn = 0;
	V103Para *pp =  pCfg;
	if(Flag == 1)
	{
		for(BYTE i=0;i<pp->YCPara.Gr_num;i++)
		{
			num = pp->YCPara.Para[i].inf + pp->YCPara.Para[i].num;
			if((pp->YCPara.Para[i].fun == bFun)&&(bInf <= num))
			{
				*Gn  += bInf - pp->YCPara.Para[i].inf;
				return TRUE;
			}
			*Gn += pp->YCPara.Para[i].num;
		}
	}else if(Flag == 2)
	{
		for(BYTE i=0;i<pp->YXPara.Gr_num;i++)
		{
			num = pp->YXPara.Para[i].inf+pp->YXPara.Para[i].num;
			if((pp->YXPara.Para[i].fun == bFun)&&(bInf <= num))
			{
				*Gn  += bInf - pp->YXPara.Para[i].inf;
				return TRUE;
			}
			*Gn += pp->YXPara.Para[i].num;
		}
	}else if(Flag ==3)
	{
		for(BYTE i=0;i<pp->DDPara.Gr_num;i++)
		{
			num = pp->DDPara.Para[i].inf+pp->DDPara.Para[i].num;
			if((pp->DDPara.Para[i].fun == bFun)&&(bInf <= num))
			{
				*Gn  += bInf - pp->DDPara.Para[i].inf;
				return TRUE;
			}
			*Gn += pp->DDPara.Para[i].num;
		}
	}
	return FALSE;
}
void CNR103m::DoGenGroupData_Asdu0A()
{
	DataGroup *p;
	NRAPCI *pApci;
	BYTE i;
	BYTE bHeadLen = sizeof(NRFrameHead);//34
	BYTE framelen = 8+MAKEDWORD(MAKEWORD(m_RecFrame.pBuf[2], m_RecFrame.pBuf[3]), MAKEWORD(m_RecFrame.pBuf[4], m_RecFrame.pBuf[5]));
	if(framelen == bHeadLen+2)
	{
		m_YKflag = 1;
		DoRecDYK();
	}
	else
	{
		m_YKflag = 0;
		pApci = (NRAPCI *)&m_RecFrame.pBuf[bHeadLen];
		m_Offset = bHeadLen+2;

		for(i = 0;i<pApci->bNgd.bit.NO;i++)
		{
			p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
			/*
			由于不知道哪些遥信是soe，报文中的时标都未取。
			所有上送状态均认为是当前遥信状态。
			*/
			switch(p->bDataType)
			{
				case TYPE_SI754:
					DoRecYC754();
					break;
				case TYPE_DYK:
					DoRecDYK();
					break;
				case TYPE_SYK:
					DoRecSYK();			
					break;	
				case TYPE_YC_F:
					DoRecYC_F();				
					break;
				case TYPE_YX_TIME:
					DoRecYX_TIME();	
					break;	
				case TYPE_YX_RET:
					DoRecYX_RET();
					break;
				case TYPE_YC_RET:
					DoRecYC_RET();
					break;	
				case TYPE_STRUCT:
					DoStruct();
					break;
				case TYPE_YX_CP56Time2a:
					DoRecYX_CP56Time2a();
					break;
				case TYPE_YX_CP56Time2aRET:
					DoRecYX_CP56Time2aRET();				
					break;
				case TYPE_YC_CP56Time2a:
					DoRecYC_CP56Time2a();	
					break;

				case TYPE_YK_CP8:
					break;				
				default:
					m_Offset =m_Offset+sizeof(DataGroup)+(p->bDataSize *(p->bNum&0x7f));
					break;
			}
		}
	}
	
}
void CNR103m::DoStruct()
{
#if 0
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;
	p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
	Pbuf = &p->bGroup;
	wLen = p->bDataSize *(p->bNum&0x7f);
	
	bFun = Pbuf[index++];
	bInf =  Pbuf[index++];
	Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,2);
	if(Pbuf[index++]!= 0X1)
		bRet = FALSE;
	if(Pbuf[index++]!= TYPE_DYK)
		bRet = FALSE;
	index+=2;
	bYXstate = Pbuf[index++];
	if(bYXstate == 2)
		bYXstate = 0x81;
	else if(bYXstate == 1)
		bYXstate = 0x01;
	if(bRet == TRUE)
		DoYkReturn();
	m_Offset += sizeof(DataGroup)+wLen;

#endif
}

void CNR103m::DoRecDYK()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;
	if(m_YKflag == 1)
	{
		if(m_pReceive->bCOT == TYPE_ASDU29H_YK)
			bRet = FALSE;
		else
			bRet = TRUE;
		
		if(bRet == TRUE)
			DoYkReturn();
	}
	else
	{
		p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
		Pbuf = &p->bGroup;
		wLen = p->bDataSize *(p->bNum&0x7f);

		bFun = Pbuf[index++];
		bInf =	Pbuf[index++];
		Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,2);
		if(Pbuf[index++]!= 0X1)
			bRet = FALSE;
		if(Pbuf[index++]!= TYPE_DYK)
			bRet = FALSE;
		index+=2;
		bYXstate = Pbuf[index++];
		if(bYXstate == 2)
			bYXstate = 0x81;
		else if(bYXstate == 1)
			bYXstate = 0x01;
		if(bRet == TRUE)
			DoYkReturn();
		m_Offset += sizeof(DataGroup)+wLen;

	}
	
}

void CNR103m::DoRecSYK()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;
	p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
	Pbuf = &p->bGroup;

	wLen = p->bDataSize *(p->bNum&0x7f);

	bFun = Pbuf[index++];
	bInf =  Pbuf[index++];
	Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,2);
	if(Pbuf[index++]!= 0X1)
		bRet = FALSE;
	if(Pbuf[index++]!= TYPE_SYK)
		bRet = FALSE;
	index+=2;
	bYXstate = Pbuf[index++];
	if(bYXstate == 1)
		bYXstate = 0x81;
	else if(bYXstate == 0)
		bYXstate = 0x01;
	if(bRet == TRUE)
		DoYkReturn();
	m_Offset += sizeof(DataGroup)+wLen;

}
void CNR103m::DoRecYC754()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO;
	BYTE bRet = TRUE;
	struct VDBYC_F fYCValue;
	p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
	Pbuf = &p->bGroup;

	wLen = p->bDataSize *(p->bNum&0x7f);

	bFun = Pbuf[index++];
	bInf =  Pbuf[index++];
	Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,1);
	if(Pbuf[index++]!= 0X1)
		bRet = FALSE;
	if(Pbuf[index++]!= TYPE_SI754)
		bRet = FALSE;
	index+=2;
	memcpy((BYTE *)&fYCValue.fValue, &Pbuf[index],wLen);
	index+=wLen;
	fYCValue.wNo = bNO;
	if(bRet == TRUE)		
		WriteYC_F(m_wEqpID,  1,&fYCValue);

	m_Offset += sizeof(DataGroup)+wLen;
	
}

void CNR103m::DoRecYC_F()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO;
	BYTE bRet = TRUE;
	short bYCValue;
	struct VDBYC_F fYCValue;
	p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
	Pbuf = &p->bGroup;

	wLen = p->bDataSize *(p->bNum&0x7f);

	bFun = Pbuf[index++];
	bInf =  Pbuf[index++];
	Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,1);
	if(Pbuf[index++]!= 0X1)
		bRet = FALSE;
	if(Pbuf[index++]!= TYPE_YC_F)
		bRet = FALSE;
	index+=2;
	bYCValue = MAKEWORD(Pbuf[index], Pbuf[index+1]);
	index+=2;

	if (!(bYCValue & 0xc000))
		bYCValue = bYCValue&0x3fff;
	else
		bYCValue = 0;
	fYCValue.wNo = bNO;
	fYCValue.fValue =bYCValue;
	if(bRet == TRUE)		
		WriteYC_F(m_wEqpID,  1,&fYCValue);

	m_Offset += sizeof(DataGroup)+wLen;
	
}
void CNR103m::DoRecYX_TIME()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;
	struct VCalClock time;
	struct VDBSOE soe;
	struct VDBCOS cos;
 	NRFrameHead * pRec =(NRFrameHead *)m_RecFrame.pBuf;		
	GetSysClock(&time, CALCLOCK);
	
	p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
	Pbuf = &p->bGroup;
	wLen = p->bDataSize *(p->bNum&0x7f);
	
	bFun = Pbuf[index++];
	bInf =  Pbuf[index++];
	Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,2);
	if(Pbuf[index++]!= 0X1)
		bRet = FALSE;
	if(Pbuf[index++]!= TYPE_YX_TIME)
		bRet = FALSE;
	index+=2;
	bYXstate = (Pbuf[index++]&0x03);
	if(bYXstate == 2)
		bYXstate = 0x81;
	else if(bYXstate == 1)
		bYXstate = 0x01;
	cos.wNo= soe.wNo = bNO;
	cos.byValue = soe.byValue = bYXstate;
	memcpy(&soe.Time,&time,sizeof(VCalClock));
	if(bRet == TRUE)
	{
		if(pRec->bCOT == COT_SPONT)
		{
			WriteSCOS(m_wEqpID, 1, &cos);
			WriteSSOE(m_wEqpID, 1, &soe);	
		}
		else
			WriteRangeSYX(m_wEqpID,bNO,1,&bYXstate);
	}
	index+=5;
	m_Offset += sizeof(DataGroup)+wLen;
	
}

void CNR103m::DoRecYX_RET()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;

	struct VCalClock time;
	struct VDBSOE soe;
	struct VDBCOS cos;
 	NRFrameHead * pRec =(NRFrameHead *)m_RecFrame.pBuf;		
	GetSysClock(&time, CALCLOCK);
	
	p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
	Pbuf = &p->bGroup;
	wLen = p->bDataSize *(p->bNum&0x7f);
	
	bFun = Pbuf[index++];
	bInf =  Pbuf[index++];
	Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,2);
	if(Pbuf[index++]!= 0X1)
		bRet = FALSE;
	if(Pbuf[index++]!= TYPE_YX_RET)
		bRet = FALSE;
	index+=2;
	bYXstate = (Pbuf[index++]&0x03);
	if(bYXstate == 2)
		bYXstate = 0x81;
	else if(bYXstate == 1)
		bYXstate = 0x01;
	cos.wNo= soe.wNo = bNO;
	cos.byValue = soe.byValue = bYXstate;
	memcpy(&soe.Time,&time,sizeof(VCalClock));
	if(bRet == TRUE)
	{
		if(pRec->bCOT == COT_SPONT)
		{
			WriteSCOS(m_wEqpID, 1, &cos);
			WriteSSOE(m_wEqpID, 1, &soe);	
		}
		else
			WriteRangeSYX(m_wEqpID,bNO,1,&bYXstate);
	}

	index+=5;
	m_Offset += sizeof(DataGroup)+wLen;
	
}

void CNR103m::DoRecYC_RET()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO;
	BYTE bRet = TRUE;
	short bYCValue;
	struct VDBYC_F fYCValue;
	p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
	Pbuf = &p->bGroup;

	wLen = p->bDataSize *(p->bNum&0x7f);

	bFun = Pbuf[index++];
	bInf =  Pbuf[index++];
	Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,1);
	if(Pbuf[index++]!= 0X1)
		bRet = FALSE;
	if(Pbuf[index++]!= TYPE_YC_RET)
		bRet = FALSE;
	index+=2;
	bYCValue = MAKEWORD(Pbuf[index], Pbuf[index+1]);
	index+=2;

	if (!(bYCValue & 0xc000))
		bYCValue = bYCValue&0x3fff;
	else
		bYCValue = 0;
	fYCValue.wNo = bNO;
	fYCValue.fValue =bYCValue;
	if(bRet == TRUE)		
		WriteYC_F(m_wEqpID,  1,&fYCValue);

	m_Offset += sizeof(DataGroup)+wLen;
	
}

void CNR103m::DoRecYX_CP56Time2a()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;

	struct VCalClock time;
	struct VDBSOE soe;
	struct VDBCOS cos;
	
 	NRFrameHead * pRec =(NRFrameHead *)m_RecFrame.pBuf;		
	GetSysClock(&time, CALCLOCK);

	p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
	Pbuf = &p->bGroup;
	wLen = p->bDataSize *(p->bNum&0x7f);
	
	bFun = Pbuf[index++];
	bInf =  Pbuf[index++];
	Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,2);
	if(Pbuf[index++]!= 0X1)
		bRet = FALSE;
	if(Pbuf[index++]!= TYPE_YX_CP56Time2a)
		bRet = FALSE;
	index+=2;
	bYXstate = (Pbuf[index++]&0x03);
	if(bYXstate == 2)
		bYXstate = 0x81;
	else if(bYXstate == 1)
		bYXstate = 0x01;
	cos.wNo= soe.wNo = bNO;
	cos.byValue = soe.byValue = bYXstate;
	memcpy(&soe.Time,&time,sizeof(VCalClock));
	if(bRet == TRUE)
	{
		if(pRec->bCOT == COT_SPONT)
		{
			WriteSCOS(m_wEqpID, 1, &cos);
			WriteSSOE(m_wEqpID, 1, &soe);	
		}
		else
			WriteRangeSYX(m_wEqpID,bNO,1,&bYXstate);
	}

	index+=5;
	m_Offset += sizeof(DataGroup)+wLen;
	
}

void CNR103m::DoRecYX_CP56Time2aRET()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;

	struct VCalClock time;
	struct VDBSOE soe;
	struct VDBCOS cos;
	
 	NRFrameHead * pRec =(NRFrameHead *)m_RecFrame.pBuf;		
	GetSysClock(&time, CALCLOCK);
	
	p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
	Pbuf = &p->bGroup;
	wLen = p->bDataSize *(p->bNum&0x7f);
	
	bFun = Pbuf[index++];
	bInf =  Pbuf[index++];
	Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,2);
	if(Pbuf[index++]!= 0X1)
		bRet = FALSE;
	if(Pbuf[index++]!= TYPE_YX_CP56Time2aRET)
		bRet = FALSE;
	index+=2;
	bYXstate = (Pbuf[index++]&0x03);
	if(bYXstate == 2)
		bYXstate = 0x81;
	else if(bYXstate == 1)
		bYXstate = 0x01;
	cos.wNo= soe.wNo = bNO;
	cos.byValue = soe.byValue = bYXstate;
	memcpy(&soe.Time,&time,sizeof(VCalClock));
	if(bRet == TRUE)
	{
		if(pRec->bCOT == COT_SPONT)
		{
			WriteSCOS(m_wEqpID, 1, &cos);
			WriteSSOE(m_wEqpID, 1, &soe);	
		}
		else
			WriteRangeSYX(m_wEqpID,bNO,1,&bYXstate);
	}

	index+=5;
	m_Offset += sizeof(DataGroup)+wLen;
	
}
void CNR103m::DoRecYC_CP56Time2a()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO;
	BYTE bRet = TRUE;
	short bYCValue;
	struct VDBYC_F fYCValue;
	p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
	Pbuf = &p->bGroup;

	wLen = p->bDataSize *(p->bNum&0x7f);

	bFun = Pbuf[index++];
	bInf =  Pbuf[index++];
	Chose_Dev(m_wEqpNo,bFun,bInf,&bNO,1);
	if(Pbuf[index++]!= 0X1)
		bRet = FALSE;
	if(Pbuf[index++]!= TYPE_YC_CP56Time2a)
		bRet = FALSE;
	index+=2;
	bYCValue = MAKEWORD(Pbuf[index], Pbuf[index+1]);
	index+=2;

	if (!(bYCValue & 0xc000))
		bYCValue = bYCValue&0x3fff;
	else
		bYCValue = 0;
	fYCValue.wNo = bNO;
	fYCValue.fValue =bYCValue;
	if(bRet == TRUE)		
		WriteYC_F(m_wEqpID,  1,&fYCValue);

	m_Offset += sizeof(DataGroup)+wLen;
	
}

void CNR103m::DoRecYK_CP8()
{}
void CNR103m::DoGenData_Asdu0B()
{

}
int CNR103m::Chose_ID(int YkNo,BYTE *fun,BYTE *inf)
{
	int i,id =0;
	for(i = 0;i<pCfg->YKPara.Gr_num;i++)
	{
		if(YkNo > pCfg->YKPara.Para[i].num)
		{
			id += pCfg->YKPara.Para[i].num;
			YkNo -= id;
			continue;
		}
		else
		{
			*fun = pCfg->YKPara.Para[i].fun;
			*inf = pCfg->YKPara.Para[i].inf + YkNo-1;
		}
		return 0;		
	}
	return 0;
}

void CNR103m::SendFrameHead(BYTE bTYPE, BYTE bVSQ, BYTE bCOT, BYTE bFUN, BYTE bINF)
{
	m_SendBuf.wWritePtr = m_SendBuf.wReadPtr = 0;
	m_pSend->wFirstFlag 			= 0xEB90;
	m_pSend->wSecondFlag		= 0xEB90;
	m_pSend->wSourceFactoryId	= 0; 			//源厂站地址
	m_pSend->wSourceAddr		= GetOwnAddr();	//源设备地址
	m_pSend->wDestinationFactoryId = 0; 			//目标厂站地址
	m_pSend->wDestinationAddr 	= GetEqpAddr(); 	//目标设备地址
	m_pSend->wDataNumber 		= (m_DataNo++); //数据编号
	m_pSend->wDeviceType 		= 0x01; 			//设备类型
	m_pSend->wDeviceState		= 0x0050; 		//设备网络状态
	m_pSend->wFirstRouterAddr 	= 0; 			//传输路径首级路由装置地址
	m_pSend->wLastRouterAddr		= 0;		 		//传输路径末级路由装置地址
	m_pSend->wReserve1			= 0xffff; 		//保留字节0xFFFF
	m_pSend->bTYPE 				= bTYPE;		//类型标识
	m_pSend->bVSQ 				= bVSQ;	 		//
	m_pSend->bCOT		 		= bCOT;			//
	m_pSend->bASDU_ADDR 		= 0;				//公共地址
	m_pSend->bFUN 				= bFUN;
	m_pSend->bINF 				= bINF;

	m_SendBuf.wWritePtr += sizeof(NRFrameHead);
}

void CNR103m::SendFrameTail()
{
	DWORD Len;
	Len = m_SendBuf.wWritePtr-m_SendBuf.wReadPtr-8;
	m_pSend->wLength = Len; //数据长度
	WriteToComm(GetEqpAddr());
}

BOOL CNR103m::ReqCyc(void)
{
	if((m_pBaseCfg->wCmdControl & BROADCASTTIME) ||(m_pBaseCfg->wCmdControl & CLOCKENABLE))
	{
		if (CheckClearTaskFlag(TASKFLAG_CLOCK))
		{
		    SendSetClock();
		    return TRUE;
		}
	}
	if (CheckClearEqpFlag(EQPFLAG_YC)) 
	{
		CheckClearEqpFlag(EQPFLAG_YX);//must at the same time with EQPFLAG_YC
		SendAllCall();
		//commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
		return TRUE;
	}
	DisableRetry();
  return FALSE;
}

BOOL CNR103m::ReqNormal(void)
{

    return FALSE;
}

void CNR103m::DoYkReturn()
{
	BYTE YKReq;
	VYKInfo *pYKInfo;
	NRAPCI *p;
	BYTE *Pbuf;
	BYTE bHeadLen = sizeof(NRFrameHead);
	
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	pYKInfo->Head.wEqpID = m_wEqpID;
	pYKInfo->Info.wID = m_YKno;
	pYKInfo->Info.byStatus = 0; 
	if(m_pReceive->bINF == 0xf9)
		pYKInfo->Head.byMsgID = MI_YKSELECT;
	else if(m_pReceive->bINF == 0xfa)
		pYKInfo->Head.byMsgID = MI_YKOPRATE;
	
	pYKInfo->Info.byValue = m_YKState;

	if(m_YKflag != 1)
	{
		p = (NRAPCI *)&m_RecFrame.pBuf[bHeadLen];
		Pbuf = &p->bGroup;
		YKReq =Pbuf[6];
		switch (YKReq&0x03)
		{
			case 0x00: 
			case 0x03:
				pYKInfo->Info.byStatus = 4; 
			    return;
			case 0x01: 
			    pYKInfo->Info.byValue = 0x06;
			    break; //分
			case 0x02: 
			    pYKInfo->Info.byValue = 0x05;
			    break; //合
			default:
			break;
		}
	}
	
	CPPrimary::DoYKRet();
	
	return;	
	
}
BOOL CNR103m::DoYKReq(void)
{
	CPPrimary::DoYKReq();
	
	if (SwitchClearEqpFlag(EQPFLAG_YKReq)) 
		if (SendYkCommand()) 
			return TRUE;
	return TRUE;
}
BOOL CNR103m::SendYkCommand(void)
{
	WORD YkNo;
	BYTE DCO = 0;
	//BYTE Qualifier = 1;
	BYTE yy = 0;
	BYTE fun,inf = 0;
	
	VYKInfo *pYKInfo;
	
	//V103Para *pq =  pCfg;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

	m_YKno= YkNo = pYKInfo->Info.wID;
	m_YKState = 0;
	switch (pYKInfo->Info.byValue & 0x3)
	{
		case 0://不区分合分
			break;
		case 1://close合闸
			DCO = 2;
			m_YKState = 0x05;
			break;
		case 2://open分闸
			DCO = 1;
			m_YKState = 0x06;
			break;
		case 3://非法
			break;	
	};
	switch (pYKInfo->Head.byMsgID & 0x7f)
	{
		case MI_YKSELECT:
			yy = INF_CON_M;
			break;
		case MI_YKOPRATE:
			yy = INF_ACT_M;
			break;
		case MI_YKCANCEL:
			yy = 0x00;
			break;
	};

	Chose_ID(YkNo,&fun,&inf);
	SendFrameHead(TYPE_ASDU0AH_GEN_GROUPDATA,0x81, COT_GCWRITE, 0xFE,yy);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  0;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  0X01;

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  fun;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  inf;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  0X01;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  0X09;	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  0X01;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  0X01;	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  DCO;	

	SendFrameTail();
	
	return TRUE;		
}

void CNR103m::SendSetClock()
{
	struct VSysClock time2;
	GetSysClock(&time2,  SYSCLOCK);

	SendFrameHead(TYPE_ASDU06H_TIME,0x81, 0x08, 0xFF,INF_TIME_S);
	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (time2.wMSecond+time2.bySecond*1000)&0xff;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (time2.wMSecond+time2.bySecond*1000)>>8;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = time2.byMinute;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = time2.byHour;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = time2.byDay;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = time2.byMonth;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = time2.wYear-2000;
	SendFrameTail();

}
void CNR103m::SendAllCall()
{
	SendFrameHead(TYPE_ASDU15H_GEN_READ,0x81, 0x09, 0xFE,INF_CALLSTOP_M);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  0X09;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0X00;	
	SendFrameTail();
}

void CNR103m::SendHeart()
{
	m_SendBuf.wWritePtr = m_SendBuf.wReadPtr = 0;

	m_pSend->wFirstFlag 			= 0xEB90;
	m_pSend->wLength 			= 0x14; //数据长度
	
	m_pSend->wSecondFlag		= 0xEB90;
	m_pSend->wSourceFactoryId	= 0; 			//源厂站地址
	m_pSend->wSourceAddr		= GetOwnAddr();	//源设备地址
	m_pSend->wDestinationFactoryId = 0; 			//目标厂站地址
	m_pSend->wDestinationAddr 	= GetEqpAddr(); 	//目标设备地址
	m_pSend->wDataNumber 		= (m_DataNo++); //数据编号
	m_pSend->wDeviceType 		= 0x01; 			//设备类型
	m_pSend->wDeviceState		= 0x0050; 		//设备网络状态
	m_pSend->wFirstRouterAddr 	= 0; 			//传输路径首级路由装置地址
	m_pSend->wLastRouterAddr		= 0;		 		//传输路径末级路由装置地址
	m_pSend->wReserve1			= 0xffff; 		//保留字节0xFFFF
	m_SendBuf.wWritePtr+=28;
	WriteToComm(GetEqpAddr());

}
