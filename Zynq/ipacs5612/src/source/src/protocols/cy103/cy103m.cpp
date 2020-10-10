#include "cy103.h"


extern "C" void cy103m(WORD wTaskID)		
{
	CCY103m *pCY103P = new CCY103m();	
	
	if (pCY103P->Init(wTaskID) != TRUE)
	{
		pCY103P->ProtocolExit();
	}
	
	pCY103P->Run();			   	
}
void CY103m_Rx(CCY103m *obj)
{
    obj->ReceiveUdp();
}


CCY103m::CCY103m() : CPPrimary()
{
	  pCfg = new V103Para;
      memset(pCfg,0,sizeof(V103Para));
}

BOOL CCY103m::Init(WORD wTaskID)
{
	BOOL rc;
	int ret;
	rc = CPPrimary::Init(wTaskID,1,(void *)pCfg,sizeof(V103Para));
	if (!rc)
	{
		return FALSE;
	}

	event_time = m_pBaseCfg->Timer.wScanData2/10;//����������ѯʱ�������
	m_pReceive = (CY103Frame *)m_RecFrame.pBuf;
	m_pSend	 = (CY103Frame *)m_SendBuf.pBuf;
	m_TaskID = wTaskID;
	m_udprecbuf.wReadPtr = m_udprecbuf.wWritePtr = 0;
	m_udprecbuf.pBuf = (BYTE*)malloc(CY103_UDP_BUFLEN);
	SDeviceInfo = new DeviceInfo[m_wEqpNum];
	memset(SDeviceInfo,0,m_wEqpNum*sizeof(DeviceInfo));
	m_udp103.Create();
	m_udp103.Bind(CY103_UDP_PORT);
	//WriteDoEvent(NULL, 0, "m_wEqpNum = %d  ",m_wEqpNum);
	for (int i = 0; i < m_wEqpNum; i ++)
	{
		SDeviceInfo[i].bfcb = 0;
		SDeviceInfo[i].bfcv = 0;
		SDeviceInfo[i].linkstate = LINK_DISCONNECT;
		for(int j=0;j<ALLFRAMETYPE;j++)
		{
			SDeviceInfo[i].bReSendflg[j] = FALSE;
			SDeviceInfo[i].bReSendTimes[j] = 0;
			SDeviceInfo[i].timercount[j] = 0;
			SDeviceInfo[i].transstep[j] = TRANSIDLE;
			
		}
	}
	//SetAllEqpFlag(EQPFLAG_POLL);
	//if (m_pBaseCfg->wCmdControl&ALLDATAPROC)		
		//SetTaskFlag(TASKFLAG_ALLDATA);
	w_YKno = 0;
	Ycgroup_num = pCfg->YCPara.Gr_num;
	m_dwTestTimerCount = 0;
	m_pBaseCfg->wCtrlResultReqCount = 5;
	//commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	ret = myTaskSpawn("tCY103mRx", (ENTRYPTR)CY103m_Rx, (int)this, (COMMONSTACKSIZE), CY103_RX_PRIO);
	if (ret == ERROR) return ERROR;
	return TRUE;
}

void CCY103m::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();  
}

void CCY103m::SetBaseCfg(void)
{
    CProtocol::SetBaseCfg();
    //if need to modify ,to do :

    m_pBaseCfg->wCmdControl = YKOPENABLED | CLOCKENABLE | DDPROC;
    m_pBaseCfg->Timer.wAllData = 15; // 15 min
    m_pBaseCfg->Timer.wSetClock = 2; //2 min
    m_pBaseCfg->Timer.wDD = 5;             
    m_pBaseCfg->wMaxRetryCount = 3;
    m_pBaseCfg->wMaxErrCount = 3;
    m_pBaseCfg->wMaxALLen = 255;
    m_pBaseCfg->wBroadcastAddr = 0x0;

}

void CCY103m::CheckCfg(void)
{
	CProtocol::CheckCfg();
}
int CCY103m::ReadFromComm(void)
{

	int Rc;
	NeatenCommBuf(&m_RecBuf);  
	Rc = commRead(m_TaskID,m_RecBuf.pBuf+m_RecBuf.wWritePtr, CY103_UDP_BUFLEN-m_RecBuf.wWritePtr, 0);

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

int CCY103m::WriteToComm(DWORD dwFlag)
{
	int Rc=0;	
	if (m_SendBuf.wWritePtr - m_SendBuf.wReadPtr)
	{
		m_SendBuf.wOldReadPtr = m_SendBuf.wReadPtr;

		m_SendBuf.dwFlag = dwFlag;
		IncRetryCount();
		
		Rc = commWrite(m_TaskID,(m_SendBuf.pBuf+m_SendBuf.wReadPtr),m_SendBuf.wWritePtr-m_SendBuf.wReadPtr,dwFlag);

		if (Rc!=ERROR)			
			m_SendBuf.wReadPtr += Rc;
#if 0
		if ((Rc != ERROR) && (m_TaskID != MMI_ID))  
		{
			commid = 1;
		}
#endif
		if (LOWORD(dwFlag) == GetBroadcastAddr())
		{
			DisableRetry();
			DWORD dwCommCtrlTimer = 1;//20 ms
			commCtrl(m_TaskID, CCC_EVENT_CTRL|m_wCommCtrl, &dwCommCtrlTimer);
			return Rc;
		}
		else  CommStatusProcByRT(FALSE);
	}
	commCtrl(m_TaskID, CCC_EVENT_CTRL|m_wCommCtrl, &m_dwCommCtrlTimer);	
	return Rc;
}

int CCY103m::commRead (int no, BYTE* pbuf, int buflen, DWORD flags)
{
	int ret = ERROR;
	int i;
    no = no-COMM_START_NO;
	if (no < 0) return ERROR;
	
	for (i=0; i<buflen; i++)
	{
	    if (m_udprecbuf.wReadPtr == m_udprecbuf.wWritePtr)
			break;
		pbuf[i] = m_udprecbuf.pBuf[m_udprecbuf.wReadPtr];
		m_udprecbuf.wReadPtr = (m_udprecbuf.wReadPtr + 1)&(CY103_UDP_BUFLEN-1);
	}
	ret = i;
#if 0	
    if (g_CommChan[no].read != NULL)
    {
        ret = g_CommChan[no].read(no, pbuf, buflen, flags);		
    }
#endif

#ifdef INCLUDE_COMM_SHOW
	if (ret>0) 
	{
		commBufFill(no, 0, ret, pbuf);
		commPrint(no, 0, ret, pbuf);
	}	
#endif	

	return(ret);
}

int CCY103m::commWrite(int no, BYTE* pbuf, int buflen, DWORD flags)
{
	int ret = ERROR;
    no = no-COMM_START_NO;
	if (no < 0) return ERROR;

	m_udp103.Send(m_SendBuf.pBuf, m_SendBuf.wWritePtr, m_pEqpInfo[m_wEqpNo].sTelNo);

	ret = m_SendBuf.wWritePtr;

#if 0	
    if (g_CommChan[no].write != NULL)
    {		
        ret = g_CommChan[no].write(no,pbuf, buflen, flags);
	}
#endif

#ifdef INCLUDE_COMM_SHOW
	if (ret>0) 
	{
		commBufFill(no, 1, ret, pbuf);
		commPrint(no, 1, ret, pbuf);
	}	
#endif	

	return(ret);
}
void CCY103m::ReceiveUdp(void)
{
    int i,rc;
	struct timeval tmval;
		
    tmval.tv_sec = 0;
	tmval.tv_usec = 20000;
	BYTE buf[CY103_UDP_BUFLEN];

    for (;;)
    {
    	rc = m_udp103.Recv(buf, CY103_UDP_BUFLEN, &tmval, m_peerip);
		
        if (rc>0)
        {
            for (i=0; i< rc; i++)
            {
                m_udprecbuf.pBuf[m_udprecbuf.wWritePtr] = buf[i];
				m_udprecbuf.wWritePtr = (m_udprecbuf.wWritePtr + 1)&(CY103_UDP_BUFLEN-1);
            }
			evSend(m_TaskID, EV_RX_AVAIL);
        }
    }
}

/***************************************************************
	Function��OnTimeOut
		��ʱ����
	������TimerID
		TimerID ��ʱ��ID
	���أ���
***************************************************************/
void CCY103m::DoTimeOut(void)
{
	m_dwTestTimerCount++;
	CPPrimary::DoTimeOut();	
}

BOOL CCY103m::SwitchToIp(char * ip)
{
   WORD wEqpNo;

	 
   for (wEqpNo = 0; wEqpNo < m_wEqpNum; wEqpNo++)
   {
	  if(strcmp(pGetEqpInfo(wEqpNo)->sTelNo, ip) == 0)
	  {
		 SwitchToEqpNo(wEqpNo);
		 return TRUE;
	  }
   }

   return FALSE;
}

DWORD CCY103m::SearchOneFrame(BYTE *Buf, WORD Len)
{
	unsigned short FrameLen;
	m_pReceive = (CY103Frame *)Buf;
	
	switch(m_pReceive->Frame10.head)
	{
		case 0x10:
			if (m_pReceive->Frame10.stop != TAIL_H)
				return FRAME_ERR|1;  
			FrameLen =5;
			
			if (FrameLen > Len)
				return FRAME_LESS;
			
			if (SwitchToIp(m_peerip) == FALSE)
				return FRAME_ERR | FrameLen;
			return FRAME_OK | FrameLen;

		case 0x68:
			if (m_pReceive->Frame68.length != m_pReceive->Frame68.length)
				return FRAME_ERR | 1;
			if (m_pReceive->Frame68.head2 != HEAD_V)
				return FRAME_ERR | 1;
			FrameLen = m_pReceive->Frame68.length + 6;
		 
			if (FrameLen > Len)
				return FRAME_LESS;

			if (Buf[FrameLen-1] != TAIL_H)
				return FRAME_ERR | 1;
			if (Buf[FrameLen-2] != (BYTE)ChkSum((BYTE *)&m_pReceive->Frame68.control,m_pReceive->Frame68.length))
				return FRAME_ERR | 1;
				
			if (SwitchToIp(m_peerip) == FALSE)
				return FRAME_ERR | FrameLen;
			return FRAME_OK | FrameLen;
		default:	
			return FRAME_ERR | 1;
	}
}
/***************************************************************
	Function��DoReceive
			   ���ձ��Ĵ���
	��������		
	���أ�TRUE �ɹ���FALSE ʧ��
***************************************************************/
BOOL CCY103m::DoReceive()
{
	if (SearchFrame() != TRUE)
	{
		return FALSE;
	}
	m_dwTestTimerCount = 0;
	m_pReceive = (CY103Frame *)m_RecFrame.pBuf;
	
	if (m_pReceive->Frame10.head == 0x10)
	{
		RecFrame10();
	}
	if (m_pReceive->Frame68.head == 0x68)
	{
		RecFrame68();
	}
	DoSend();
	return FALSE;
}

/***************************************************************
	Function��	RecFrame10
				���չ̶�֡
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame10()
{
	BYTE bCmd = 0;
	if(m_pReceive->Frame10.control & C_PRM)
	{
		return FALSE;
	}
	
	if(m_pReceive->Frame10.control & M_ACD)
	{
		//acd = 1����������1������
		//SetACDflg(m_pReceive->Frame10.address);
		//ACD_ON
		SetEqpFlag(ACD_ON);
	}

	bCmd = (m_pReceive->Frame10.control & A_CMD);

	switch(bCmd)
	{
		case CMD_CONF_S:
			SDeviceInfo[m_wEqpNo].linkstate= LINK_CONNECT;
			break;
		case CMD_LINSTATE_S: 
			SDeviceInfo[m_wEqpNo].linkstate = LINK_CONNECT;
			break;
		case CMD_NUDATA_S:
			break;
		default:
			break;
	}
	return TRUE;
}

/***************************************************************
	Function��	SetACDflg
				�����豸��ַ�ö�Ӧacdλ
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::SetACDflg(BYTE bAddr)
{
	SDeviceInfo[m_wEqpNo].transstep[SENDPRIDATA] = TRANSSEND;	
	return TRUE;
}
int CCY103m::Chose_ID(int YkNo,BYTE *fun,BYTE *inf)
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
/***************************************************************
	Function��	CheckYKOrderSend
				�����Ҫ ң��ѡ����豸
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::SendYkCommand(void)
{
	WORD YkNo;
	BYTE DCO = 0;
	//BYTE Qualifier = 1;
	BYTE bVsq =0;
	BYTE yy = 0;
	BYTE fun,inf = 0;
	
	VYKInfo *pYKInfo;
	
	//V103Para *pq =  pCfg;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

	w_YKno= YkNo = pYKInfo->Info.wID;
	
	switch (pYKInfo->Info.byValue & 0x3)
	{
		case 0://�����ֺϷ�
			break;
		case 1://close��բ
			DCO = HE;
			break;
		case 2://open��բ
			DCO = FEN;
			break;
		case 3://�Ƿ�
			break;	
	};
	switch (pYKInfo->Head.byMsgID & 0x7f)
	{
		case MI_YKSELECT:
			yy = SENDYKSELECT;
			break;
		case MI_YKOPRATE:
			yy = SENDYKACT;
			break;
		case MI_YKCANCEL:
			yy = SENDYKREVOCAT;
			break;
	};
/*	if(pCfg->by103Cfg == 0x02)
	{	
		Chose_ID(YkNo,&fun,&inf);
		MakeFrame68Head(TYPE_ASDU10_GEN_GROUPDATA,COT_GCWRACK,fun,inf);
		bVsq = VSQ_SQ_1|0X01;
		MakeFrame68Tail_yk(m_wEqpNo,CMD_CONF_M,bVsq,yy,DCO,fun,inf);
		WriteData();
	}
	else
*/
	{
		Chose_ID(YkNo,&fun,&inf);
		MakeFrame68Head(TYPE_ASDU64_YK,COT_REMOP,fun,inf);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  Get_ykdata(yy,DCO);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
		bVsq = 0X81;
		MakeFrame68Tail(CMD_CONF_M,bVsq,yy);
		WriteData();
	
		//SendFrame68_ASDU64(YkNo,yy,DCO);
	}
	
	return TRUE;		
}
/***************************************************************
	Function��	RecFrame68
				���ձ䳤֡
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68()
{
	BYTE bAsduType = 0;
	if(m_pReceive->Frame68.control & C_PRM)
	{
		return FALSE;
	}
	
	if(m_pReceive->Frame68.control & M_ACD)
	{
		//acd = 1������1������
		if(m_pReceive->Frame68.cot != COT_START)
		SetEqpFlag(ACD_ON);
	}
	
	bAsduType = m_pReceive->Frame68.type;
	switch(bAsduType)
	{
		case TYPE_ASDU01_YBSTATE:
			RecFrame68_ASDU01();
			break;
			
		case TYPE_ASDU02_PROINF:
			RecFrame68_ASDU02();
			break;
			
		case TYPE_ASDU05_SIGNINF:
			RecFrame68_ASDU05();
			break;		

		case TYPE_ASDU08_CALLOVER:
			RecFrame68_ASDU08();
			break;
			
		case TYPE_ASDU23_RDSTATE:	
			RecFrame68_ASDU23();
			break;	
			
		case TYPE_ASDU26_RDDATAACT:
			RecFrame68_ASDU26();
			break;

		case TYPE_ASDU27_RDCOMMACT:
			RecFrame68_ASDU27();
			break;

		case TYPE_ASDU28_SIGNSTATE:
			RecFrame68_ASDU28();
			break;

		case TYPE_ASDU29_TRANSSTAT:
			RecFrame68_ASDU29();
			break;

		case TYPE_ASDU30_TRSRDDATA:
			RecFrame68_ASDU30();
			break;

		case TYPE_ASDU31_RDDATAOVER:
			RecFrame68_ASDU31();
			break;

		//case TYPE_ASDU36_SENDDNL:
	//		RecFrame68_ASDU36();
		//	break;
			
		case TYPE_ASDU38_STEPINF:
			RecFrame68_ASDU38();
			break;
			
		case TYPE_ASDU39_STEPINFSOE	:
			RecFrame68_ASDU39();
			break;
			
		case TYPE_ASDU40_YX	:
			RecFrame68_ASDU40();
			break;
			
		case TYPE_ASDU41_SOE:
			RecFrame68_ASDU41();
			break;
			
		case TYPE_ASDU44_YX	:
			RecFrame68_ASDU44();
			break;
		case TYPE_ASDU42_YX	:
			RecFrame68_ASDU42();
			break;
		case TYPE_ASDU09_YC:	
		case TYPE_ASDU50_YC:
			RecFrame68_ASDU50();
			break;
			
		case TYPE_ASDU64_YK	:
			RecFrame68_ASDU64();
			break;
		case TYPE_ASDU10_GEN_GROUPDATA:
			RecFrame68_ASDU10();
			break;
		default:
			return FALSE;
		
	}
	return TRUE;
}

/***************************************************************
	Function��	RecFrame10_Cmd0
				����ȷ��֡���ͱ�ʶ
	����:
	���أ�	��
***************************************************************/

BOOL CCY103m::RecFrame10_Cmd0()
{
	return TRUE;
}
BYTE CCY103m::Get_Value(BYTE *buf,BYTE dateType,BYTE datelen,DWORD *val1,float *val2)
{
	float de = 0.0;
	if(dateType == 4||dateType == 3)
	{
		if(datelen == 2)
			*val1 = MAKEDWORD(buf[0], buf[1]);
		else if(datelen == 4)
			*val1 = MAKEDWORD(MAKEDWORD(buf[0], buf[1]),MAKEDWORD(buf[2], buf[3]));
		return DATA_ONE;
	}else if(dateType == 5 || dateType == 6 || dateType == 7)
	{
		memcpy(&de,buf,sizeof(float));
		*val2 = de;
		return DATA_TWO;
	}
	return FALSE;
}
void CCY103m::YkResult()
{
	DoYkReturn();
	ClearEqpFlag(EQPFLAG_YKReq);

	return;
}
void CCY103m::ykresult(BYTE cot,BYTE type)
{
	if(type == 0)//select
	{
		if(cot == COT_GCWRCON)
			YkResult();
		else if(cot == COT_GCWRNACK)
			YkResult();
	}
	else if(type == 1)//execute
	{
		if(cot == COT_GCWRACK)
			YkResult();
		else if(cot == COT_GCWRNACK)
			YkResult();
	}
	return ;
}
/*
	���:
	0������
	1��ң��
	2��ң��
*/
int CCY103m::judge_yc_dd(BYTE fun,BYTE inf)
{
	int num = pCfg->YCPara.Gr_num;
	int num_dd = pCfg->DDPara.Gr_num;
	int i,j;

	for(i = 0;i<num;i++)
	{
		if(fun == pCfg->YCPara.Para[i].fun)
			return 1;
	}

	for(j = 0;j<num_dd;j++)
	{
		if(fun == pCfg->DDPara.Para[i].fun)
			return 2;
	}
	return 0;
}
/***************************************************************
	Function��	RecFrame68_ASDU0A
				ң���ң���Ĵ���
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU10()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYcNum = 0;
	BYTE i = 0;
	BOOL ret  = FALSE;
	BYTE  data_index =0 ;
	BYTE index = 0;
	DWORD data = 0;
	float data1 = 0.0;
	BYTE type = 0;
	BYTE cot;
	int result;
	struct VDBYC bYcData;
	struct VDBYC_F ycdata;

	
	bYcNum = (m_pReceive->Frame68.data[1] & ~0x80);

	cot = m_pReceive->Frame68.cot;

	if((cot == COT_GCWRCON)|| (cot == COT_GCWRNACK) || (cot == COT_GCWRACK))
	{
		if(m_pReceive->Frame68.info == 0xF9)
			ykresult(cot,0);
		else if(m_pReceive->Frame68.info == 0xFa)
			ykresult(cot,1);
	}
	for(i=0;i<bYcNum;i++)
	{

		bFun = m_pReceive->Frame68.data[2+index];
		bInf = m_pReceive->Frame68.data[3+index];
		result = judge_yc_dd(bFun,bInf);
		if(result == 1)
			ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
		else if(result == 2)
			ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,3);
		
		if(!ret)
			return FALSE;
		type = Get_Value(&m_pReceive->Frame68.data[8+index],m_pReceive->Frame68.data[5+index],m_pReceive->Frame68.data[6+index],&data,&data1);
		
		ycdata.wNo = bYcData.wNo = data_index;
		if(type == DATA_ONE)
		{
			bYcData.nValue = data;
			WriteYC(m_wEqpID,1,&bYcData);
		}
		else if(type == DATA_TWO)
		{
			ycdata.fValue = data1;
			WriteYC_F(m_wEqpID, 1, &ycdata);
		}
		index +=  m_pReceive->Frame68.data[6+index]+6;
	}
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU01
				��������ѹ�弰�澯������״̬
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU01()
{
	BYTE bFun = 0;
	BYTE bInf,data_index = 0;
//	WORD wYXaddr = 0;
	BYTE bYXstate,ret = 0;
	struct VDBCOS cos;
	struct VDBSOE soe;

	struct VSysClock time1;
    struct VCalClock time2;  
	GetSysClock(&time1, SYSCLOCK);
	BYTE num = m_pReceive->Frame68.vsq &0x7f;
	
	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;
	
	 ret= Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	for(BYTE i =0;i<num;i++)
	{
		bYXstate = m_pReceive->Frame68.data[0+5*i];
		if(bYXstate == 1)
			bYXstate = SWS_OFF;
		else if(bYXstate == 2)
			bYXstate = SWS_ON;
		else
			return TRUE;
			time1.wMSecond = m_pReceive->Frame68.data[5*i+1]+m_pReceive->Frame68.data[5*i+2]*256;
			if(m_pReceive->Frame68.data[5*i+3]&0x80)
				;
			else
				time1.byMinute = m_pReceive->Frame68.data[5*i+3]&0x7f;
		time1.byHour= m_pReceive->Frame68.data[8+i*4]; 

		CalClockTo(&time2, &time1);

		soe.byValue =cos.byValue = bYXstate;
		soe.wNo = cos.wNo = data_index;
		memcpy(&soe.Time,&time2,sizeof(VCalClock));

		if(m_pReceive->Frame68.cot == COT_SPONT)
		{
			WriteSCOS(m_wEqpID, 1, &cos);
			WriteSSOE(m_wEqpID, 1,&soe);
		}
		else
		{
			WriteRangeSYX(m_wEqpID,data_index,1,&bYXstate);		
		}
	
	}
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU02
				�������ͱ���������Ϣ
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU02()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYxNum = 0;
	BYTE i = 0;
	BYTE wYxData = 0;
//	WORD wYXaddr = 0;
	//BYTE bQsd = 0;
	BYTE yx =0 ;
	BOOL ret = 0;
	BYTE data_index = 0;
	struct VDBCOS cos;
	struct VDBSOE soe;

	struct VSysClock time1;
    struct VCalClock time2;  
	GetSysClock(&time1, SYSCLOCK);
	

	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;
	bYxNum = (m_pReceive->Frame68.vsq & (~VSQ_SQ_1));

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	
	for(i=0;i<bYxNum;i++)
	{
		time1.wMSecond = m_pReceive->Frame68.data[i+5]+m_pReceive->Frame68.data[i+6]*256;
		if(m_pReceive->Frame68.data[i+7]&0x80)
			;
		else
			time1.byMinute = m_pReceive->Frame68.data[i+7]&0x7f;
			time1.byHour= m_pReceive->Frame68.data[8+i]; 

		CalClockTo(&time2, &time1);
		
		yx = m_pReceive->Frame68.data[i];
		if(yx ==1)
		{
			wYxData = SWS_OFF;
		}
		else if(yx ==2)
		{
			wYxData = SWS_ON;
		}
		else
		{//״̬��ȷ��
			data_index++;
			continue;
		}
		
		soe.byValue =cos.byValue = wYxData;
		soe.wNo = cos.wNo = data_index;
		memcpy(&soe.Time,&time2,sizeof(VCalClock));
		WriteSCOS(m_wEqpID, 1, &cos);
		WriteSSOE(m_wEqpID, 1,&soe);
		data_index++;
	}
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU05
				�������ͱ�ʶ
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU05()
{
		return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU08
				���ղ�ѯ����
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU08()
{
	if(m_pReceive->Frame68.cot == COT_CALLSTOP)
			SetEqpFlag(TY_ON);
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU23
				�������ͱ���¼���Ŷ���
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU23()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU26
				�����Ŷ����ݴ���׼������
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU26()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU27
				���ձ���¼��ͨ������׼������
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU27()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU28
				���մ���־��״̬��λ����׼������
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU28()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU29
				���մ���־��״̬��λ����
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU29()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU30
				���մ����Ŷ�ֵ
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU30()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU31
				�����Ŷ����ݴ������
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU31()
{
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU32
				����ң������
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU32()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE wYcNum = 0;
	WORD wYcBeginNo = 0;
	BYTE *bYCbuf;
	BYTE data_index = 0;
	BOOL ret =FALSE;
	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	wYcNum = m_pReceive->Frame68.vsq & (~VSQ_SQ_1);
	wYcBeginNo = data_index;
	bYCbuf = &m_pReceive->Frame68.data[0];

	WriteRangeYC(m_wEqpNo, wYcBeginNo, wYcNum, (short *)bYCbuf);
	return TRUE;
}

#if 0
/***************************************************************
	Function��	RecFrame68_ASDU36
				���յ���������
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU36()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bDdNum = 0;

	struct VDDFT Ddbuf;
	DWORD wDdNo = 0;
	BYTE data_index = 0;
	BOOL ret =FALSE;
	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	bDdNum = (m_pReceive->Frame68.vsq & (~VSQ_SQ_1));
	//�����Ҫת��
	//Ddbuf = m_pReceive->Frame68.data[0] + m_pReceive->Frame68.data[1]<<8 + m_pReceive->Frame68.data[2]<<16 + m_pReceive->Frame68.data[3]<<24;
	//bQds = m_pReceive->Frame68.dat[4];	

	WriteRangeDDFT(m_wEqpNo,data_index,bDdNum, &Ddbuf);
	return TRUE;
}
#endif
/***************************************************************
	Function��	RecFrame68_ASDU38
				���ձ�λ���Ͳ�λ��
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU38()
{
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU39
				���ղ�λ��SOE
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU39()
{
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU40
				���ձ�λң�����ݣ�����
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU40()
{
	BYTE bFun = 0;
	BYTE bInf = 0;		
	BYTE data_index = 0;
	BOOL ret = FALSE;
	BYTE num,i,j;
	BYTE stat;
	VDBCOS RecCOS;

	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;
	num  = m_pReceive->Frame68.vsq &0x7f;
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	
	if(m_pReceive->Frame68.vsq &0x80)
	{
		for(i =0;i<num;i++)
		{
			if(i>0)
			{
				j = i-1;
				bFun = m_pReceive->Frame68.data[1+3*j];
				bInf = m_pReceive->Frame68.data[2+3*j];
				ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
				if(!ret)
					continue;
			}
			stat = m_pReceive->Frame68.data[0+3*i];
			if(stat == 0)
				stat = SWS_OFF;
			else if(stat == 1)
				stat = SWS_ON;

			RecCOS.byValue = stat;
			RecCOS.wNo = data_index;

			if(m_pReceive->Frame68.cot == COT_SPONT)
			{
				WriteSCOS(m_wEqpID,1,&RecCOS);
			}
			else
			{
				WriteRangeSYX(m_wEqpID,RecCOS.wNo,1,&RecCOS.byValue);		
			}
		}
	}
	else
	{
		for(i = 0;i<num ;i++)
		{
			if( m_pReceive->Frame68.data[i] == 0x00)
				stat = SWS_OFF;
			else
				stat = SWS_ON;
			RecCOS.byValue =stat;
			RecCOS.wNo = data_index++;

			if(m_pReceive->Frame68.cot == COT_SPONT)
			{
				WriteSCOS(m_wEqpID,1,&RecCOS);
			}
			else
			{
				WriteRangeSYX(m_wEqpID,RecCOS.wNo,1,&RecCOS.byValue);		
			}
		}
	}
	
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU41
				����SOE
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU41()
{
	BYTE bFun = 0;
	BYTE bInf = 0;		
	BOOL ret= FALSE;
	BYTE data_index = 0;
	BYTE num,bYXstate; 
	VDBSOE RecSOE;
	BYTE j;
	//struct VCalClock time2;  
	//GetSysClock(&time2, CALCLOCK);
	//memcpy(&RecSOE.Time,&time2,sizeof(VCalClock));
	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;
	num  = m_pReceive->Frame68.vsq &0x7f;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;

	for(BYTE i =0;i<num;i++)
	{
		if(i>0)
		{
			j = i-1;
			bFun = m_pReceive->Frame68.data[5+7*j];
			bInf = m_pReceive->Frame68.data[6+7*j];
			ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
			if(!ret)
				continue;
		}
		bYXstate = m_pReceive->Frame68.data[0+7*i];
		if(bYXstate == 0)
			bYXstate = SWS_OFF;
		else if(bYXstate == 1)
			bYXstate = SWS_ON;
		else
		{//��ȷ��״̬
			continue;
		}
		
		RecSOE.Time.dwMinute = m_pReceive->Frame68.data[3+7*i]+60*m_pReceive->Frame68.data[4+7*i];
		RecSOE.Time.wMSecond = MAKEWORD(m_pReceive->Frame68.data[1+7*i], m_pReceive->Frame68.data[2+7*i]);

		RecSOE.byValue = bYXstate;
		RecSOE.wNo = data_index;

		WriteSSOE(m_wEqpID, 1,&RecSOE);
	
	}
	
	
	return TRUE;
}
/***************************************************************
	Function��	Time4bytetoCalClock
				
	����:
	���أ�	��
***************************************************************/
void CCY103m::Time4bytetoCalClock(BYTE *bTime4,VCalClock *CalCloc)
{
	VSysClock SysTime;
	GetSysClock((void *)&SysTime, SYSCLOCK);
	SysTime.wMSecond = (bTime4[0]+bTime4[1]*256)%1000;
	SysTime.bySecond = (bTime4[0]+bTime4[1]*256)/1000;
	SysTime.byMinute = bTime4[2];
	SysTime.byHour = bTime4[3];

	ToCalClock(&SysTime, CalCloc);
}
/***************************************************************
	Function��	RecFrame68_ASDU42
				����˫��ȫң������
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU42()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYxNum = 0;
	BYTE i = 0;
	BYTE j = 0;
	BYTE wYxData = 0;
//	WORD wYXaddr = 0;
//	BYTE bQsd = 0;
	BYTE yx =0 ;
	BOOL ret = 0;
	BYTE data_index = 0;
	struct VDBCOS cos;

	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;
	bYxNum = (m_pReceive->Frame68.vsq & (~VSQ_SQ_1));

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;

	
	if(m_pReceive->Frame68.vsq &0x80)
	{
		for(i =0;i<bYxNum;i++)
		{
			if(i>0)
			{
				j = i-1;
				bFun = m_pReceive->Frame68.data[1+3*j];
				bInf = m_pReceive->Frame68.data[2+3*j];
				ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
				if(!ret)
					continue;
			}
			yx = m_pReceive->Frame68.data[0+3*i];
			if(yx == 1)
			{
				wYxData = SWS_OFF;
			}	
			else if(yx == 2)
			{
				wYxData = SWS_ON;
			}
			else
			{//��ȷ��״̬
				continue;
			}

			cos.byValue = wYxData;
			cos.wNo = data_index;

			if(m_pReceive->Frame68.cot == COT_SPONT)
			{
				WriteSCOS(m_wEqpID,1,&cos);
			}
			else
			{
				WriteRangeSYX(m_wEqpID,cos.wNo,1,&cos.byValue);		
			}
		}
	}
	else
	{
		for(i = 0;i<bYxNum ;i++)
		{
			yx = m_pReceive->Frame68.data[i];
			if(yx ==1)
			{
				wYxData = SWS_OFF;
			}
			else if(yx ==2)
			{
				wYxData = SWS_ON;
			}
			else
			{//��ȷ��״̬
				data_index++;
				continue;
			}
			cos.byValue = wYxData;
			cos.wNo = data_index++;

			if(m_pReceive->Frame68.cot == COT_SPONT)
			{
				WriteSCOS(m_wEqpID,1,&cos);
			}
			else
			{
				WriteRangeSYX(m_wEqpID,cos.wNo,1,&cos.byValue);		
			}
		}
	}

	return TRUE;
}
int CCY103m::GetNum_Yx()
{
	int i,total = 0;
	for(i = 0;i<pCfg->YXPara.Gr_num;i++)
		total += pCfg->YXPara.Para[i].num;
	return total;
}
/***************************************************************
	Function��	RecFrame68_ASDU44
				����ȫң������
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU44()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYxNum = 0;
	BYTE i = 0,j=0;
	BYTE wYxData;
	//WORD wYXaddr = 0;
	BYTE bYxState = 0;
	BOOL ret = 0;
	BYTE data_index = 0;
		
	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;

	bYxNum = (m_pReceive->Frame68.vsq & (~VSQ_SQ_1));

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;

	for(i = 0;i<bYxNum;i++)
	{
		wYxData = m_pReceive->Frame68.data[5*i+0]; 
		for(j=0;j<8;j++)
		{
			bYxState = (wYxData&(0x01<<j));
			if(bYxState)
				bYxState = 0x81;
			else
				bYxState = 0x01;
			WriteRangeSYX(m_wEqpID,data_index,1,&bYxState);
			data_index ++;
		}
		
		wYxData = m_pReceive->Frame68.data[5*i+1];
		for(j=0;j<8;j++)
		{
			bYxState = (wYxData &(0x01<<j));
			if(bYxState)
				bYxState = 0x81;
			else
				bYxState = 0x01;		
			WriteRangeSYX(m_wEqpID,data_index,1,&bYxState);
			data_index ++;
		}
	}
	
	return TRUE;
}
/***************************************************************
	Function��	Chose_Dev
				�ҵ�fun��inf��Ӧ�����
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::Chose_Dev(WORD no,BYTE bFun,BYTE bInf,BYTE *Gn,BYTE Flag)
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
/***************************************************************
	Function��	RecFrame68_ASDU50
				����ң������
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU50()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYcNum = 0;
	BYTE i,j;
	BOOL ret  = FALSE;
	BYTE  data_index =0 ;
	WORD wYcBeginNo;
	short YcTempValue;
	struct VDBYC_F fYCValue ;
	i=j=0;
	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	bYcNum = (m_pReceive->Frame68.vsq & (~VSQ_SQ_1));
	
	for(i=0;i<bYcNum;i++)
	{
		if(m_pReceive->Frame68.vsq & VSQ_SQ_1)
		{
			if(i>0)
			{
				j = i-1;
				bFun = m_pReceive->Frame68.data[2+4*j];
				bInf = m_pReceive->Frame68.data[3+4*j];
				ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
				if(!ret)
					continue;
			}

			YcTempValue = m_pReceive->Frame68.data[0+4*i]+ (m_pReceive->Frame68.data[1+4*i]*256);
			wYcBeginNo = data_index;
		}
		else
		{
			YcTempValue = m_pReceive->Frame68.data[0+2*i]+ (m_pReceive->Frame68.data[1+2*i]*256);
			wYcBeginNo = data_index++;
		}

		if(YcTempValue & 0x0002)//���Ʒ������
		{
			continue;
		}
		else
		{
			if (YcTempValue & 0x8000)
				YcTempValue = 0 - ((YcTempValue & 0x7fff)>>3);
			else
				YcTempValue = (YcTempValue>>3) & 0x7fff;
			
			fYCValue.fValue =YcTempValue;
			fYCValue.wNo = wYcBeginNo;
			WriteYC_F(m_wEqpID,  1,&fYCValue);
		}
	}
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU44
				����ң�ؾ������
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::RecFrame68_ASDU64()
{
	YkResult();
	
	return TRUE;	
}

/***************************************************************
	Function��	DoReceviceYc
				���յ���ң�����ݴ���
	����:
	���أ�	��
***************************************************************/
BOOL CCY103m::DoReceviceYc(BYTE bAddr,BYTE bInf,WORD bData)
{
	return TRUE;
}

/***************************************************************
	Function��	WriteData
				��������
	����:
	���أ�	��
***************************************************************/
void CCY103m::WriteData(void)
{
	WriteToComm(GetEqpAddr());
}

/***************************************************************
	Function��	MakeControlCode
				�������
	����:
	���أ�	��
***************************************************************/
BYTE CCY103m::MakeControlCode(BYTE bCmd,BYTE FramList)
{
	BYTE temp = 0;
	temp = bCmd;
	if(bCmd == CMD_CONF_M||bCmd == CMD_CALPRIDATA_M||bCmd == CMD_CALSECDATA_M|| bCmd == TYPE_ASDU64_YK)
	{
		SDeviceInfo[m_wEqpNo].bfcv = 1;
		temp |= (SDeviceInfo[m_wEqpNo].bfcv<<4);
		if(SDeviceInfo[m_wEqpNo].bReSendflg[FramList])
		{
			temp |= (SDeviceInfo[m_wEqpNo].bfcb << 5);
			SDeviceInfo[m_wEqpNo].bReSendflg[FramList] = FALSE;
		}
		else
		{
			if(SDeviceInfo[m_wEqpNo].bfcb)
			{
				SDeviceInfo[m_wEqpNo].bfcb = 0; 		
			}
			else
			{
				SDeviceInfo[m_wEqpNo].bfcb = 1; 				
			}
			temp |= (SDeviceInfo[m_wEqpNo].bfcb << 5);
		}
	}
	
	temp |= C_PRM;
		
	return temp;

}

/***************************************************************
	Function��	MakeFrame10
				��10֡
	����:
	���أ�	��
***************************************************************/
BYTE CCY103m::MakeFrame10(BYTE bCmd,BYTE FramList)
{
	m_SendBuf.wReadPtr = 0;
	m_SendBuf.wWritePtr=0;
	
	m_pSend->Frame10.head 	 = 0x10;
	m_pSend->Frame10.control = MakeControlCode(bCmd,FramList);
	m_pSend->Frame10.address = m_pEqpInfo[m_wEqpNo].wDAddress;
	m_pSend->Frame10.sum 	 = m_pSend->Frame10.control + m_pSend->Frame10.address;
	m_pSend->Frame10.stop	 = 0x16;
	return 5;
}

/***************************************************************
	Function��	MakeFrame68Head
				��68֡
	����:
	���أ�	��
***************************************************************/
BYTE CCY103m::MakeFrame68Head(BYTE type, BYTE cot, BYTE fun, BYTE info)
{
	m_SendBuf.wReadPtr = 0;
	m_SendBuf.wWritePtr =0;
	
	m_pSend->Frame68.linkaddr = m_pSend->Frame68.pubaddr = m_pEqpInfo[m_wEqpNo].wDAddress;
	m_pSend->Frame68.head  = 0x68;
	m_pSend->Frame68.head2 = 0x68;
	m_pSend->Frame68.type  = type;
	m_pSend->Frame68.cot   = cot;
	m_pSend->Frame68.fun   = fun;
	m_pSend->Frame68.info  = info;
	m_SendBuf.wWritePtr += 12;
	return 	m_SendBuf.wWritePtr ;
}
/***************************************************************
	Function��	MakeFrame68Tail_yk
		
	����:
	���أ�	��
***************************************************************/
BYTE CCY103m::MakeFrame68Tail_yk(BYTE bDevNum,BYTE bCmd,BYTE vsq,BYTE FramList,BYTE a,BYTE gr,BYTE enty)
{
	BYTE bFrameLen = 0;
	BYTE Index = 0;
	if(bDevNum == 0xff)//�㲥��ַ
	{
		m_pSend->Frame68.linkaddr = 0xff;
		m_pSend->Frame68.pubaddr  = 0xff;
	}
	else
	{
		m_pSend->Frame68.pubaddr  = m_pSend->Frame68.linkaddr = m_pEqpInfo[m_wEqpNo].wDAddress;
	}
	m_pSend->Frame68.vsq = vsq;
	m_pSend->Frame68.fun = 0xFE;
	if(FramList == SENDYKSELECT)
		m_pSend->Frame68.info = 0xF9;
	else if(FramList == SENDYKACT)
		m_pSend->Frame68.info = 0xFA;
	m_pSend->Frame68.data[Index++] = 0;
	m_pSend->Frame68.data[Index++] = 0x01;
	m_pSend->Frame68.data[Index++] = gr;
	m_pSend->Frame68.data[Index++] = enty;
	
	m_pSend->Frame68.data[Index++] = 0x01;
	
	m_pSend->Frame68.data[Index++] = 0x03;
	m_pSend->Frame68.data[Index++] = 0x01;
	m_pSend->Frame68.data[Index++] = 0x01;

	m_pSend->Frame68.data[Index++] =  a;
	
	m_SendBuf.wWritePtr +=Index;
	bFrameLen = m_SendBuf.wWritePtr - 4;
	m_pSend->Frame68.length  = bFrameLen;
	m_pSend->Frame68.length2 = bFrameLen;
	m_pSend->Frame68.control = MakeControlCode(bCmd,FramList);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]   = (BYTE )ChkSum(&m_pSend->Frame68.control, bFrameLen);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	
	return TRUE;
}
/***************************************************************
	Function��	MakeFrame68Tail
				��68֡
	����:		bDevNum = 0xff�㲥��ַ
	���أ�	��
***************************************************************/
BYTE CCY103m::MakeFrame68Tail(BYTE bCmd,BYTE vsq,BYTE FramList)
{
	BYTE bFrameLen = 0;

	bFrameLen = m_SendBuf.wWritePtr - 4;
	m_pSend->Frame68.length  = bFrameLen;
	m_pSend->Frame68.length2 = bFrameLen;
	m_pSend->Frame68.control = MakeControlCode(bCmd,FramList);
	m_pSend->Frame68.vsq = vsq;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]   = (BYTE )ChkSum(&m_pSend->Frame68.control, bFrameLen);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;

	m_bPollMasterSendFlag = FALSE;
	return TRUE;
}
/***************************************************************
	Function��	ChkSum
	����:	BYTE *buf, 
			WORD len
	���أ�	��
***************************************************************/
BYTE CCY103m::ChkSum(BYTE *buf, WORD len)
{
	WORD checksum, i;

	checksum = 0;
	for (i = 0; i < len; i++)
	{
		checksum = checksum + *buf;
		buf ++;
		
	}
	return LOBYTE(checksum & 0xff);
}
/***************************************************************
	Function��	SendFrame10_Cmd0
				���͸�λͨѶ��Ԫ
	����:
	���أ�	��
***************************************************************/
void CCY103m::SendFrame10_Cmd0()
{
	BYTE bFrameLen = 0;
	
	bFrameLen = MakeFrame10(CMD_RESTU_M,SENDRESETCU);
	m_SendBuf.wWritePtr += bFrameLen;
	WriteData();
}

/***************************************************************
	Function��	SendFrame10_Cmd07
				���͸�λ֡����λ
	����:
	���أ�	��
***************************************************************/
void CCY103m::SendFrame10_Cmd07()
{
	BYTE bFrameLen = 0;
	
	bFrameLen = MakeFrame10(CMD_RESTFCB_M,SENDRESETFCB);
	m_SendBuf.wWritePtr += bFrameLen;
	WriteData();
}
/***************************************************************
	Function��	SendFrame10_Cmd11
				��������һ������
	����:
	���أ�	��
***************************************************************/
void CCY103m::SendFrame10_Cmd10()
{
	BYTE bFrameLen = 0;
	
	bFrameLen = MakeFrame10(CMD_CALPRIDATA_M,SENDPRIDATA);
	m_SendBuf.wWritePtr += bFrameLen;
	WriteData();

	
}

/***************************************************************
	Function��	SendFrame10_Cmd11
				���������������
	����:
	���أ�	��
***************************************************************/
void CCY103m::SendFrame10_Cmd11()
{
	BYTE bFrameLen = 0;
	
	bFrameLen = MakeFrame10(CMD_CALSECDATA_M,SENDSECDATA);
	m_SendBuf.wWritePtr += bFrameLen;
	WriteData();
}
/***************************************************************
	Function��	SendFrame68_ASDU06
				��ʱ
	����:
	���أ�	��
***************************************************************/
void CCY103m::SendFrame68_ASDU06(BYTE addr)
{
	BYTE bVsq = 0;
	BYTE bFun = 0XFF;
	BYTE bInf = 0x00;
	BYTE bDataNum = 0;
	VSysClock SysTime;
	WORD MSecond;
	
	GetSysClock((void *)&SysTime, SYSCLOCK);
	MSecond = SysTime.bySecond * 1000 + SysTime.wMSecond;
	if(addr == 0xff)
		m_pSend->Frame68.pubaddr  = m_pSend->Frame68.linkaddr = 0xff;
	else
	m_pSend->Frame68.pubaddr = m_pSend->Frame68.linkaddr = m_pEqpInfo[m_wEqpNo].wDAddress;

	MakeFrame68Head(TYPE_ASDU06_TIME,COT_TIMER,bFun,bInf);
	m_pSend->Frame68.data[bDataNum++] = LOBYTE(MSecond); 
	m_pSend->Frame68.data[bDataNum++] = HIBYTE(MSecond);
	m_pSend->Frame68.data[bDataNum++] = SysTime.byMinute; 
	m_pSend->Frame68.data[bDataNum++] = SysTime.byHour;
	m_pSend->Frame68.data[bDataNum++] = (SysTime.byDay & 0x1f) | ((SysTime.byWeek << 5) & 0xe0); 
	m_pSend->Frame68.data[bDataNum++] = SysTime.byMonth;
	m_pSend->Frame68.data[bDataNum++] = SysTime.wYear - 2000; 
	m_SendBuf.wWritePtr += bDataNum;
	bVsq = VSQ_SQ_1|0X01;
	MakeFrame68Tail(CMD_CONF_M,bVsq,SENDCALLALL);
	WriteData();
}

/***************************************************************
	Function��	SendFrame68_ASDU07
				�����ܲ�ѯ
	����:
	���أ�	��
***************************************************************/
void CCY103m::SendFrame68_ASDU07()
{
	BYTE bVsq = 0;
	BYTE bFun = 0XFF;
	BYTE bInf = 0x00;

	MakeFrame68Head(TYPE_ASDU07_ALLCALL,COT_ALLCALL,bFun,bInf);
	m_pSend->Frame68.data[0] = 0x04;//SCN���ܲ�ѯ��Ч��������
	m_SendBuf.wWritePtr += 1;
	bVsq = VSQ_SQ_1|0X01;
	MakeFrame68Tail(CMD_CONF_M,bVsq,SENDCALLALL);

	WriteData();
}

/***************************************************************
	Function��	SendFrame68_ASDU58
				�����������ٻ�
	����:
	���أ�	��
***************************************************************/
void CCY103m::SendFrame68_ASDU88()
{
	BYTE bVsq = 0;
	BYTE bFun = 0x01;
	BYTE bInf = 0x00;
	
	MakeFrame68Head(TYPE_ASDU88_CALLENERGY,COT_PERCYC,bFun,bInf);
	m_pSend->Frame68.data[0] = 0x00;//�ٻ������޶���QCC
	bVsq = VSQ_SQ_1|0X01;
	MakeFrame68Tail(CMD_CONF_M,bVsq,SENDCALLENERGY);
	WriteData();
	
}
BYTE CCY103m::Get_ykdata(BYTE tt,BYTE dd)
{
	BYTE data = 0; 
	if((tt == SENDYKSELECT) || (tt == SENDYKREVOCAT))
	{
		data |= 0x80;
		if(dd == HE)
			data |= 0x02;
		if(dd == FEN)
			data |= 0x01;
	}else if(tt == SENDYKACT)
	{
		data &= (~0x80);
		if(dd == HE)
			data |= 0x02;
		if(dd == FEN)
			data |= 0x01;
	}
	return data;
}
/***************************************************************
	Function��	SendFrame68_ASDU64
				ң��ѡ��ִ�С�����
	����:
	���أ�	��
***************************************************************/
void CCY103m::SendFrame68_ASDU64(int ykId,BYTE type,BYTE data)
{
	BYTE bVsq = 0;
	BYTE bFun = pCfg->YKPara.Para[0].fun;
	BYTE bInf = pCfg->YKPara.Para[0].inf+ykId-1;//��Ҫ����ʵ�����ȷ��
	BYTE ykdata;

	Chose_ID(ykId,&bFun,&bInf);
	MakeFrame68Head(TYPE_ASDU64_YK,COT_REMOP,bFun,bInf);
	ykdata = Get_ykdata(type,data);
	m_pSend->Frame68.data[m_SendBuf.wWritePtr++] = ykdata;
	m_pSend->Frame68.data[m_SendBuf.wWritePtr++] = 0;

	bVsq = VSQ_SQ_1|0X01;
	MakeFrame68Tail(CMD_CONF_M,bVsq,type);
	
	WriteData();
	
}

/***************************************************************
	Function��	SendFrame68_ASDU21
				ͨ�÷��������
	����:
	���أ�	��
***************************************************************/
void CCY103m::SendFrame68_ASDU21(BYTE flag)
{
	BYTE bFun = 0xfe;
	BYTE bInf;//��Ҫ����ʵ�����ȷ��
	BYTE Index = 0;
	bInf = 0xf1;//ң�������

	MakeFrame68Head(TYPE_ASDU21_GEN_READ,COT_GCREAD,bFun,bInf);
	m_pSend->Frame68.data[Index++] = 0;
	m_pSend->Frame68.data[Index++] = 0x01;
	if(flag)
	{
		m_pSend->Frame68.data[Index++] = pCfg->YCPara.Para[Ycgroup_num-1].fun;
		m_pSend->Frame68.data[Index++] = pCfg->YCPara.Para[Ycgroup_num-1].inf;	
		Ycgroup_num--;
		if(Ycgroup_num)
			SetEqpFlag(TY_ON);
		else
			Ycgroup_num = pCfg->YCPara.Gr_num;
	}else
	{
		m_pSend->Frame68.data[Index++] = pCfg->DDPara.Para[0].fun;
		m_pSend->Frame68.data[Index++] = pCfg->DDPara.Para[0].inf;
	}
	m_pSend->Frame68.data[Index++] = 0x1;
	m_SendBuf.wWritePtr +=Index;

	MakeFrame68Tail(COT_GCREAD,0x81,TYPE_ASDU21_GEN_READ);
	WriteData();
}


BOOL CCY103m::DoYKReq(void)
{
	CPPrimary::DoYKReq();
	
	if (SwitchClearEqpFlag(EQPFLAG_YKReq)) 
		if (SendYkCommand()) 
			return TRUE;
	return TRUE;
}

void CCY103m::OnCommError(void)
{
    SDeviceInfo[m_wEqpNo].linkstate = LINK_DISCONNECT; 
}

#if 1
BOOL CCY103m::ReqUrgency(void)
{

	if ((SDeviceInfo[m_wEqpNo].linkstate== LINK_CONNECT) && SwitchClearEqpFlag(ACD_ON))
	{
		SendFrame10_Cmd10();
		return TRUE;
	}
/*
	if ((SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT) && SwitchClearEqpFlag(EQPFLAG_YKReq))
	{
		if (SendYkCommand()) 
			return TRUE;
	}
*/
	return FALSE;
}

void CCY103m::DoYkReturn()
{
	BYTE YKReq;
	VYKInfo *pYKInfo;

	
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

	//add yk return info proc
	pYKInfo->Head.wEqpID = m_wEqpID;
	pYKInfo->Info.wID = w_YKno;
	YKReq = m_pReceive->Frame68.data[0];
	pYKInfo->Info.byStatus = 0; 
	
	switch (YKReq&0xc0)
	{
	    case 0x80:
		    pYKInfo->Head.byMsgID = MI_YKSELECT;
		    break;
	    case 0x00:
		    pYKInfo->Head.byMsgID = MI_YKOPRATE;
		    break;
	    case 0xc0:
		    pYKInfo->Head.byMsgID = MI_YKCANCEL;
		    break;
			
	    default:
		    break;	
	}
	switch (YKReq&0x03)
	{
	    case 0x00: 
	    case 0x03:
	    	pYKInfo->Info.byStatus = 4; 
		    return;
	    case 0x01: 
		    pYKInfo->Info.byValue = 0x06;
		    break; //��
	    case 0x02: 
		    pYKInfo->Info.byValue = 0x05;
		    break; //��
	    default:
	    break;
	}

	
	CPPrimary::DoYKRet();
	
	return;	
	
}
BOOL CCY103m::ReqCyc(void)
{
	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)
	{
		if(m_pBaseCfg->wCmdControl & BROADCASTTIME)
		{
	    	if (CheckClearTaskFlag(TASKFLAG_CLOCK))
	    	{
	            SendFrame68_ASDU06(0xff);
				//commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	            return TRUE;
	    	}
		}

		if(CheckClearEqpFlag(EQPFLAG_YC))
		{
			CheckClearEqpFlag(EQPFLAG_YX);//Ϊ������һ��װ��
			SendFrame68_ASDU07();
			return TRUE;
		}

/*		if(CheckClearEqpFlag(EQPFLAG_DD))
		{
			SendFrame68_ASDU21(0);
			return TRUE;
		}
		
		if((SwitchClearEqpFlag(TY_ON))&&(Ycgroup_num > 0))
		{
			SendFrame68_ASDU21(1);
			return TRUE;
		}
*/	
	}
    return FALSE;
}
BOOL CCY103m::ReqNormal(void)
{

	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)
	{
		SendFrame10_Cmd11();
		return TRUE;
	}
	
	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_DISCONNECT)
	{
		SendFrame10_Cmd0();//��λ�豸
		return TRUE;
	}
	
    return FALSE;
}
#endif


