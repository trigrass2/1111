#include "ipacs103m.h"


extern "C" void ipacs103m(WORD wTaskID)		
{
	CIpacs103P *pIpacs103P = new CIpacs103P(); 	
	
	if (pIpacs103P->Init(wTaskID) != TRUE)
	{
		pIpacs103P->ProtocolExit();
	}
	
	pIpacs103P->Run();			   	
}



CIpacs103P::CIpacs103P() : CPPrimary()
{
	  pCfg = new V103Para;
      memset(pCfg,0,sizeof(V103Para));
}


BOOL CIpacs103P::Init(WORD wTaskID)
{
	BOOL rc;
	int k ,w =0;
	VDBCOS yx;
	rc = CPPrimary::Init(wTaskID,1,(void *)pCfg,sizeof(V103Para));
	if (!rc)
	{
		return FALSE;
	}

	event_time = m_pBaseCfg->Timer.wScanData2/10;//����������ѯʱ�������
	m_pReceive = (VIpacs103Frame *)m_RecFrame.pBuf;
	m_pSend	 = (VIpacs103Frame *)m_SendBuf.pBuf;
	SDeviceInfo = new DeviceInfo[m_wEqpNum];
	memset(SDeviceInfo,0,m_wEqpNum*sizeof(DeviceInfo));
	fla = FALSE;
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
		
	m_wSendNum = 0;		//Send Counter	
	m_wRecvNum = 0;		//Receive Counter
	m_wAckNum = 0;		//Receive Ack Counter
	m_wAckRecvNum = 0; 	//Have Ack Receive Counter
	Ycgroup_num = pCfg->YCPara.Gr_num;
	m_dwTestTimerCount = 0;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|COMM_IDLE_ON, &event_time);
	return TRUE;
}

void CIpacs103P::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();  
}

void CIpacs103P::SetBaseCfg(void)
{
    CProtocol::SetBaseCfg();
    //if need to modify ,to do :

    m_pBaseCfg->wCmdControl = YKOPENABLED | CLOCKENABLE | DDPROC;
    m_pBaseCfg->Timer.wAllData = 1; // 3 min
    m_pBaseCfg->Timer.wSetClock = 60;       //60 min
    m_pBaseCfg->Timer.wDD = 20;             //20 min insted for call Soe
    m_pBaseCfg->wMaxRetryCount = 1;
    m_pBaseCfg->wMaxErrCount = 3;
    m_pBaseCfg->wMaxALLen = 255;
    m_pBaseCfg->wBroadcastAddr = 0x0;

}

//need to be called from Class CProtocol
DWORD CIpacs103P::DoCommState(void)
{
	DWORD dwCommState;
	BOOL bTcpState = FALSE;
	
	dwCommState=CPPrimary::DoCommState();
	if (dwCommState == 1)
	{
		bTcpState = TRUE;
		WriteDoEvent(NULL, 0, "TCP ok !\n");
	}
	else
	{
		bTcpState = FALSE;
		WriteDoEvent(NULL, 0, "TCP close!\n");
	}
	
	if (bTcpState)
	{
		m_wSendNum = 0;	//Send Counter	
		m_wRecvNum = 0;	//Receive Counter
		m_wAckNum = 0;		//Receive Ack Counter
		m_wAckRecvNum = 0; //Have Ack Receive Counter
		thSleep(300);
		SendUFrame(STARTDT_ACT);		  		
  		
	}
	else
	{
		;
	}
	return dwCommState;
}

void CIpacs103P::CheckCfg(void)
{
	CProtocol::CheckCfg();
}
/***************************************************************
	Function��OnTimeOut
		��ʱ����
	������TimerID
		TimerID ��ʱ��ID
	���أ���
***************************************************************/
void CIpacs103P::DoTimeOut(void)
{
	CPPrimary::DoTimeOut();

	if(TimerOn(m_pBaseCfg->Timer.wAllData*60))
	{
		SetAllEqpFlag(EQPFLAG_YC);
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
DWORD CIpacs103P::SearchOneFrame(BYTE *Buf, WORD Len)
{
	unsigned short FrameLen;
	WORD wLinkAddress;
	WORD wOwnAddress;
	m_pReceive = (VIpacs103Frame *)Buf;

	if(Len < 7)
		return FRAME_LESS;	
	if((m_pReceive->byStartCode != 0x68))
		return FRAME_ERR|1;  

	FrameLen = m_pReceive->byAPDULenL + m_pReceive->byAPDULenH*256 + 3;
	if (FrameLen >Len)
		return FRAME_LESS;
	wLinkAddress = m_pReceive->bSourceAddrH*256+m_pReceive->bSourceAddrL;
	if (SwitchToAddress(wLinkAddress) == FALSE)
		return FRAME_ERR | FrameLen;
	wOwnAddress = m_pReceive->bDestinationAddrH*256+m_pReceive->bDestinationAddrL;
	if (wOwnAddress !=  GetOwnAddr())
		return FRAME_ERR | FrameLen;	

	return FRAME_OK | FrameLen;	

}
/***************************************************************
	Function��DoReceive
			   ���ձ��Ĵ���
	��������		
	���أ�TRUE �ɹ���FALSE ʧ��
***************************************************************/
BOOL CIpacs103P::DoReceive()
{
	if (SearchFrame() != TRUE)
	{
		return FALSE;
	}
	m_pReceive = (VIpacs103Frame *)m_RecFrame.pBuf;
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
	DoSend();
	return TRUE;
}

/***************************************************************
	Function��	RecFrame10
				���չ̶�֡
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame10()
{
		return TRUE;
}

/***************************************************************
	Function��	SetACDflg
				�����豸��ַ�ö�Ӧacdλ
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::SetACDflg(BYTE bAddr)
{
	SDeviceInfo[m_wEqpNo].transstep[SENDPRIDATA] = TRANSSEND;	
	return TRUE;
}
int CIpacs103P::Chose_ID(int YkNo,BYTE *fun,BYTE *inf)
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
#if 0
/***************************************************************
	Function��	CheckYKOrderSend
				�����Ҫ ң��ѡ����豸
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::SendYkCommand(void)
{
	WORD YkNo;
	BYTE DCO = 0;
	BYTE yy = 0;
	BYTE fun,inf = 0;
	
	VYKInfo *pYKInfo;
	
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

	YkNo = pYKInfo->Info.wID;
	
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
	}
	Chose_ID(YkNo,&fun,&inf);
	SendFrameHead(FRM_I);
	SendFrame68_ASDU64(YkNo,yy,DCO,fun,inf);
	m_pSend->byAPDULenL=LOBYTE((m_SendBuf.wWritePtr-3));
	m_pSend->byAPDULenH=HIBYTE((m_SendBuf.wWritePtr-3));
	WriteData();
	return TRUE;		
}
#endif
/***************************************************************
	Function��	SendSFrame
	����:
	���أ�	��
***************************************************************/
void CIpacs103P::SendSFrame(void)
{
	SendFrameHead(FRM_S);
	m_pSend->byAPDULenL=LOBYTE((m_SendBuf.wWritePtr-3));
	m_pSend->byAPDULenH=HIBYTE((m_SendBuf.wWritePtr-3));
	SendFrameTail();
	return;    
}
/***************************************************************
	Function��	DoIFrame
	����:
	���أ�	��
***************************************************************/
void CIpacs103P::DoIFrame(void)
{
	WORD wAckNum, wRecvNum;
	
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);
	wRecvNum = MAKEWORD(m_pReceive->byControl1, m_pReceive->byControl2);
	wAckNum >>= 1;
	wRecvNum >>= 1;
	m_wRecvNum = wRecvNum + 1;
	
	m_wAckNum = wAckNum; 
	fla = TRUE;
	switch (m_pReceive->byASDU[0])
	{
		case TYPE_ASDU01_YBSTATE:
			RecFrame68_ASDU01();
			break;
		case TYPE_ASDU05_SIGNINF:
			break;
		case TYPE_ASDU09_YC:	
		case TYPE_ASDU50_YC:
			RecFrame68_ASDU50();
			break;
		case TYPE_ASDU0AH_GEN_GROUPDATA	:	//ͨ�÷���������Ӧ���װ����Ӧ�Ķ�һ�����������
		case TYPE_ASDU0BH_GEN_DATA			:	//ͨ�÷���������Ӧ���װ����Ӧ�Ķ�������Ŀ��Ŀ¼��
			DoGenGroupData_Asdu0A();
			break;
			
		case TYPE_ASDU40_YX:
			RecFrame68_ASDU40();
			break;
		case TYPE_ASDU41_SOE:
			RecFrame68_ASDU41();
			break;
		case TYPE_ASDU02_PROINF:
			RecFrame68_ASDU02();
			break;
		case TYPE_ASDU44_YX:
			RecFrame68_ASDU44();
			break;

		case TYPE_ASDU51_YC:
			RecFrame68_ASDU51();
			break;
		case TYPE_ASDU64_YK:
			RecFrame68_ASDU64();
			break;
	}
}
/***************************************************************
	Function��	DoSFrame
	����:
	���أ�	��
***************************************************************/
void CIpacs103P::DoSFrame(void)
{
	WORD wAckNum;
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);	
	m_wAckNum = wAckNum >> 1;
}
void CIpacs103P::DoUFrame(void)
{
	if(m_pReceive->byControl1 == STARTDT_CON)
	{
		SetAllEqpFlag(EQPFLAG_YC);
	}
	if(m_pReceive->byControl1 == TESTFR_ACT)
	{
	
		SendUFrame(TESTFR_CON);	
		thSleep(300);
		SendUFrame(STARTDT_ACT);	
	}
	SDeviceInfo[m_wEqpNo].linkstate = LINK_CONNECT;
}
/***************************************************************
	Function��	RecFrame68
				���ձ䳤֡
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68()
{	
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

/***************************************************************
	Function��	RecFrame10_Cmd0
				����ȷ��֡���ͱ�ʶ
	����:
	���أ�	��
***************************************************************/

BOOL CIpacs103P::RecFrame10_Cmd0()
{
	return TRUE;
}
BYTE CIpacs103P::Get_Value(BYTE *buf,BYTE dateType,BYTE datelen,DWORD *val1,float *val2)
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
void CIpacs103P::YkResult(BOOL Result)
{
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

	if (Result)
		pYKInfo->Info.byStatus = 0;
	else
	{
		pYKInfo->Info.byStatus = 3;
		ClearEqpFlag(EQPFLAG_YKReq);
	}
	CPPrimary::DoYKRet();
	return;
}
void CIpacs103P::ykresult(BYTE cot,BYTE type)
{
	if(type == 0)//select
	{
		if(cot == COT_GCWRCON)
			YkResult(TRUE);
		else if(cot == COT_GCWRNACK)
			YkResult(FALSE);
	}
	else if(type == 1)//execute
	{
		if(cot == COT_GCWRACK)
			YkResult(TRUE);
		else if(cot == COT_GCWRNACK)
			YkResult(FALSE);
	}
	return ;
}
/*
	���:
	0������
	1��ң��
	2��ң��
*/
int CIpacs103P::judge_yc_dd(BYTE fun,BYTE inf)
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
	Function��	RecFrame68_ASDU01
				��������ѹ�弰�澯������״̬
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU01()
{
	BYTE bFun = 0;
	BYTE bInf,data_index = 0;
	BYTE bYXstate,ret = 0;
	struct VDBCOS cos;
	struct VDBSOE soe;

	struct VSysClock time1;
    struct VCalClock time2;  
	GetSysClock(&time1, SYSCLOCK);

	IPACS103ASDU *Ipacs103_asdu = (IPACS103ASDU *)m_pReceive->byASDU;
	
	BYTE num = Ipacs103_asdu->vsq &0x7f;
	
	bFun = Ipacs103_asdu->fun;
	bInf = Ipacs103_asdu->info;
	
	 ret= Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	for(BYTE i =0;i<num;i++)
	{
		bYXstate = Ipacs103_asdu->data[0+5*i];
		if(bYXstate == 1)
			bYXstate = SWS_OFF;
		else if(bYXstate == 2)
			bYXstate = SWS_ON;
		else
			return TRUE;
		time1.wMSecond = Ipacs103_asdu->data[5*i+1]+Ipacs103_asdu->data[5*i+2]*256;
		if(Ipacs103_asdu->data[5*i+3]&0x80)
				;
		else
			time1.byMinute = Ipacs103_asdu->data[5*i+3]&0x7f;
		time1.byHour= Ipacs103_asdu->data[4+i*5]; 

		ToCalClock(&time1, &time2);

		soe.byValue =cos.byValue = bYXstate;
		soe.wNo = cos.wNo = data_index;
		memcpy(&soe.Time,&time2,sizeof(VCalClock));
		WriteSCOS(m_wEqpID, 1, &cos);
		WriteSSOE(m_wEqpID, 1,&soe);
		
	}
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU02
				�������ͱ���������Ϣ
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU02()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE wYxData = 0;
	BYTE yx =0 ;
	BOOL ret = 0;
	BYTE data_index = 0;
	struct VDBCOS cos;
	struct VDBSOE soe;
	struct VSysClock time1;
  //  	struct VCalClock time2;  
	GetSysClock(&time1, SYSCLOCK);

	IPACS103ASDU *Ipacs103_asdu = (IPACS103ASDU *)m_pReceive->byASDU;

	bFun = Ipacs103_asdu->fun;
	bInf = Ipacs103_asdu->info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
		
	yx = Ipacs103_asdu->data[0];
	if(yx ==1)
	{
		wYxData = SWS_OFF;
	}else if(yx ==2)
	{
		wYxData = SWS_ON;
	}else
		return FALSE;

		time1.wMSecond = Ipacs103_asdu->data[5]+Ipacs103_asdu->data[6]*256;
		if(Ipacs103_asdu->data[7]&0x80)
				;
		else
			time1.byMinute = Ipacs103_asdu->data[7]&0x7f;
		time1.byHour= Ipacs103_asdu->data[8]; 

	//ToCalClock(&time1, &time2);	
	Time4bytetoCalClock(&Ipacs103_asdu->data[5],&soe.Time);

	soe.byValue =cos.byValue = wYxData;
	soe.wNo = cos.wNo = data_index;
	//memcpy(&soe.Time,&time2,sizeof(VCalClock));
	WriteSCOS(m_wEqpID, 1, &cos);
	WriteSSOE(m_wEqpID, 1,&soe);
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU05
				�������ͱ�ʶ
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU05()
{
		return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU08
				���ղ�ѯ����
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU08()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU23
				�������ͱ���¼���Ŷ���
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU23()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU26
				�����Ŷ����ݴ���׼������
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU26()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU27
				���ձ���¼��ͨ������׼������
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU27()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU28
				���մ���־��״̬��λ����׼������
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU28()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU29
				���մ���־��״̬��λ����
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU29()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU30
				���մ����Ŷ�ֵ
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU30()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU31
				�����Ŷ����ݴ������
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU31()
{
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU32
				����ң������
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU32()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE wYcNum = 0;
	WORD wYcBeginNo = 0;
	WORD bYCbuf;
	BYTE data_index = 0;
	BOOL ret =FALSE;
	BYTE i = 0;
	IPACS103ASDU *Ipacs103_asdu = (IPACS103ASDU *)m_pReceive->byASDU;
	
	bFun = Ipacs103_asdu->fun;
	bInf = Ipacs103_asdu->info;
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	wYcNum =Ipacs103_asdu->vsq & (~VSQ_SQ_1);
	wYcBeginNo = data_index;
	for(i=0;i<wYcNum;i++)
	{
		bYCbuf = MAKEWORD(Ipacs103_asdu->data[2*i], Ipacs103_asdu->data[2*i+1]);
		bYCbuf = (bYCbuf >> 3);
		WriteRangeYC(m_wEqpNo, wYcBeginNo, 1, (short *)&bYCbuf);
	}
	return TRUE;
}


/***************************************************************
	Function��	RecFrame68_ASDU38
				���ձ�λ���Ͳ�λ��
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU38()
{
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU39
				���ղ�λ��SOE
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU39()
{
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU40
				���ձ�λң������
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU40()
{
	BYTE bFun = 0;
	BYTE bInf = 0;		
	BYTE data_index = 0;
	BOOL ret = FALSE;
	int i =0;
	BYTE stat;
	BYTE buf[100];
	memset(buf,0,100);
	struct VDBCOS RecCOS;
	struct VDBSOE soe;
	
    struct VCalClock time2;  
	GetSysClock(&time2, CALCLOCK);

	IPACS103ASDU *Ipacs103_asdu = (IPACS103ASDU *)m_pReceive->byASDU;
	
	
	bFun = Ipacs103_asdu->fun;
	bInf = Ipacs103_asdu->info;
	int num = Ipacs103_asdu->vsq &(0x7f);
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;

	if( Ipacs103_asdu->data[0] == 0x00)
		stat = SWS_OFF;
	else
		stat = SWS_ON;

	 RecCOS.byValue =stat;
	RecCOS.wNo = data_index;
	memcpy(&soe.Time,&time2,sizeof(VCalClock));
	WriteSCOS(m_wEqpID,1,&RecCOS);
		
	memcpy(buf,&Ipacs103_asdu->data[1],(num-1)*3);
	for(i = 0;i<num -1 ;i++)
	{
		ret = Chose_Dev(m_wEqpNo,buf[0+3*i],buf[1+3*i],&data_index,2);
		if(!ret)
			return FALSE;
		if(buf[2+3*i] == 0x00)
			stat = SWS_OFF;
		else
			stat = SWS_ON;

		memcpy(&soe.Time,&time2,sizeof(VCalClock));
		RecCOS.byValue =stat;
		RecCOS.wNo = data_index;
		WriteSCOS(m_wEqpID,1,&RecCOS);
	}
	
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU41
				����SOE
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU41()
{
	BYTE bFun = 0;
	BYTE bInf = 0;		
	BOOL ret= FALSE;
	BYTE data_index = 0;
	VDBSOE RecSOE;
	VDBCOS	cos;
	BYTE stat;
	struct VSysClock time1;
	GetSysClock(&time1, SYSCLOCK);
	IPACS103ASDU *Ipacs103_asdu = (IPACS103ASDU *)m_pReceive->byASDU;
	
	bFun = Ipacs103_asdu->fun;
	bInf = Ipacs103_asdu->info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	
	if(Ipacs103_asdu->data[0] == 1)
		stat = SWS_ON;
	else
		stat = SWS_OFF;
	
	cos.byValue= RecSOE.byValue = stat;
	cos.wNo = RecSOE.wNo = data_index;
	
	time1.wMSecond = Ipacs103_asdu->data[1]+Ipacs103_asdu->data[2]*256;
	if(Ipacs103_asdu->data[3]&0x80)
			;
	else
		time1.byMinute = Ipacs103_asdu->data[3]&0x7f;
	time1.byHour= Ipacs103_asdu->data[4]; 
	CalClockTo(&RecSOE.Time, &time1);
	Time4bytetoCalClock(&Ipacs103_asdu->data[1],&RecSOE.Time);
	WriteSCOS(m_wEqpID,1,&cos);
	WriteSSOE(m_wEqpID,1,&RecSOE);
	return TRUE;
}
/***************************************************************
	Function��	Time4bytetoCalClock
				
	����:
	���أ�	��
***************************************************************/
void CIpacs103P::Time4bytetoCalClock(BYTE *bTime4,VCalClock *CalCloc)
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
				����ȫң������
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU42()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYxNum = 0;
	BYTE i = 0;
	BYTE wYxData = 0;
	BYTE yx =0 ;
	BOOL ret = 0;
	BYTE data_index = 0;
	struct VDBCOS cos;

	IPACS103ASDU *Ipacs103_asdu = (IPACS103ASDU *)m_pReceive->byASDU;

	bFun = Ipacs103_asdu->fun;
	bInf = Ipacs103_asdu->info;
	bYxNum = (Ipacs103_asdu->vsq & (~VSQ_SQ_1));

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	
	for(i=0;i<bYxNum;i++)
	{
		yx = Ipacs103_asdu->data[i];
		if(yx ==1)
			wYxData = SWS_OFF;
		else
			wYxData = SWS_ON;

		cos.byValue = wYxData;
		cos.wNo = data_index;
		WriteSCOS(m_wEqpID, 1, &cos);
		data_index++;
	}
	return TRUE;
}
int CIpacs103P::GetNum_Yx()
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
BOOL CIpacs103P::RecFrame68_ASDU44()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYxNum = 0;
	BYTE i = 0 ,j=0;
	BYTE wYxData,bYxState;
	BOOL ret = 0;
	BYTE data_index = 0;
	
	IPACS103ASDU *Ipacs103_asdu = (IPACS103ASDU *)m_pReceive->byASDU;
		
	bFun = Ipacs103_asdu->fun;
	bInf = Ipacs103_asdu->info;
	bYxNum = (Ipacs103_asdu->vsq & (~VSQ_SQ_1));

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;

	for(i = 0;i<bYxNum;i++)
	{
		wYxData = Ipacs103_asdu->data[5*i+0];
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
		
		wYxData = Ipacs103_asdu->data[5*i+1];
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
BOOL CIpacs103P::Chose_Dev(WORD no,BYTE bFun,BYTE bInf,BYTE *Gn,BYTE Flag)
{
	int num = 0;
	*Gn = 0;			
	int k = 0;

	V103Para *pp =  pCfg;
	if(Flag == 1)
	{
		for(BYTE i=0;i<pp->YCPara.Gr_num;i++)
		{
			num = pp->YCPara.Para[i].inf + pp->YCPara.Para[i].num;
			if((pp->YCPara.Para[i].fun == bFun)&&(bInf <= num)&&(bInf >= pp->YCPara.Para[i].inf))
			{
				*Gn  += bInf - pp->YCPara.Para[i].inf;
				return TRUE;
			}
			*Gn += pp->YCPara.Para[i].num;
		}
	}else if(Flag == 2)
	{

		if(bFun == 202)
		{	
			for(k =0;k<24;k++)
			{
				if(INF_3315[k] == bInf)
					{
						*Gn = k + pp->YXPara.Para[0].num;
						return TRUE;
					}
			}
			return FALSE;
		}else if(bFun == 211)
		{
			for(k =0;k<40;k++)
			{
				if(INF_3311[k] == bInf)
					{
						*Gn = k + pp->YXPara.Para[0].num;
						return TRUE;
					}
			}
			return FALSE;	
		}else if(bFun == 201)
		{
			for(k =0;k<44;k++)
			{
				if(INF_3361[k] == bInf)
					{
						*Gn = k + pp->YXPara.Para[0].num;
						return TRUE;
					}
			}
			return FALSE;	
		}
		
		for(BYTE i=0;i<pp->YXPara.Gr_num;i++)
		{
			num = pp->YXPara.Para[i].inf+pp->YXPara.Para[i].num;
			if((pp->YXPara.Para[i].fun == bFun)&&(bInf <= num)&&(bInf >= pp->YXPara.Para[i].inf))
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
			if((pp->DDPara.Para[i].fun == bFun)&&(bInf <= num)&&(bInf >= pp->DDPara.Para[i].inf))
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
BOOL CIpacs103P::RecFrame68_ASDU50()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYcNum = 0;
	BYTE i = 0;
	BOOL ret  = FALSE;
	BYTE  data_index =0 ;
	//struct VDBYC bYcData;
	struct VDBYC_F fYCValue ;

	short YcTempValue;
	
	IPACS103ASDU *Ipacs103_asdu = (IPACS103ASDU *)m_pReceive->byASDU;
	
	bFun = Ipacs103_asdu->fun;
	bInf = Ipacs103_asdu->info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	bYcNum = ((Ipacs103_asdu->vsq) & (~VSQ_SQ_1));
	
	for(i=0;i<bYcNum;i++)
	{
		//bYcData.wNo = data_index;
		fYCValue.wNo= data_index;
		YcTempValue = MAKEWORD(Ipacs103_asdu->data[2*i+0],Ipacs103_asdu->data[2*i+1]);
	
 	    if (YcTempValue & 0x8000)
			YcTempValue = 0 - ((YcTempValue & 0x7fff)>>3);
	    else
			YcTempValue = (YcTempValue>>3) & 0x7fff;
		
		//bYcData.nValue  = YcTempValue;
		//WriteYC(m_wEqpID,1,&bYcData);
		fYCValue.fValue =YcTempValue;
		WriteYC_F(m_wEqpID,  1,&fYCValue);
		
		data_index++;
	}
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU44
				����ң�ؾ������
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU64()
{
	YkResult(TRUE);
	return TRUE;	
}
/***************************************************************
	Function��	RecFrame68_ASDU51
				
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::RecFrame68_ASDU51()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYcNum = 0;
	BYTE i = 0;
	BOOL ret  = FALSE;
	BYTE  data_index =0 ;
	short YcTempValue = 0;
	 struct VDBYC_F fYCValue ;
	IPACS103ASDU *Ipacs103_asdu = (IPACS103ASDU *)m_pReceive->byASDU;
	
	bFun = Ipacs103_asdu->fun;
	bInf = Ipacs103_asdu->info;
	bYcNum = ((Ipacs103_asdu->vsq) & 0x7f);
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);	
	if(!ret)
		return FALSE;
	YcTempValue = MAKEWORD(Ipacs103_asdu->data[0],Ipacs103_asdu->data[1]);
	if (YcTempValue & 0x8000)
		YcTempValue = 0 - ((YcTempValue & 0x7fff)>>3);
	else
		YcTempValue = (YcTempValue>>3) & 0x7fff;
	fYCValue.wNo= data_index;
	fYCValue.fValue =YcTempValue;
	WriteYC_F(m_wEqpID,  1,&fYCValue);
//	WriteRangeYC(m_wEqpID, data_index, 1, (short *)&YcTempValue);

	
	for(i=0;i<(bYcNum-1);i++)
	{
		bInf = Ipacs103_asdu->data[3*i+2];
		ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);	
		if(!ret)
			return FALSE;
		YcTempValue = MAKEWORD(Ipacs103_asdu->data[3*i+3],Ipacs103_asdu->data[3*i+4]);

		if (YcTempValue & 0x8000)
			YcTempValue = 0 - ((YcTempValue & 0x7fff)>>3);
		else
			YcTempValue = (YcTempValue>>3) & 0x7fff;
		fYCValue.wNo= data_index;
		fYCValue.fValue =YcTempValue;
		WriteYC_F(m_wEqpID,  1,&fYCValue);
		//WriteRangeYC(m_wEqpID, data_index, 1, (short *)&YcTempValue);
	}
	
	return TRUE;
}
/***************************************************************
	Function��	DoReceviceYc
				���յ���ң�����ݴ���
	����:
	���أ�	��
***************************************************************/
BOOL CIpacs103P::DoReceviceYc(BYTE bAddr,BYTE bInf,WORD bData)
{
	return TRUE;
}

/***************************************************************
	Function��	WriteData
				��������
	����:
	���أ�	��
***************************************************************/
void CIpacs103P::WriteData(void)
{
	WriteToComm(GetEqpAddr());
}

/***************************************************************
	Function��	MakeControlCode
				�������
	����:
	���أ�	��
***************************************************************/
BYTE CIpacs103P::MakeControlCode(BYTE bCmd,BYTE FramList)
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
#if 1
BYTE CIpacs103P::MakeFrame10(BYTE bCmd,BYTE FramList)
{
	return 0;
}
#endif
/***************************************************************
	Function��	MakeFrame68Head
				��10֡
	����:
	���أ�	��
***************************************************************/
BYTE CIpacs103P::MakeFrame68Head(BYTE type, BYTE cot, BYTE fun, BYTE info)
{
	int data_index =0;
	IPACS103ASDU *pn103 = (IPACS103ASDU *)m_pSend->byASDU;
	
	pn103->type = type;
	pn103->vsq = 0x81;
	pn103->cot   = cot;
	if(cot == COT_TIMER)
		pn103->pubaddr = 0xff;
	else
		pn103->pubaddr = m_pEqpInfo[m_wEqpNo].wDAddress;
	pn103->fun   = fun;
	pn103->info  = info;
	m_SendBuf.wWritePtr += 6;
	if(COT_TIMER == cot)
	{
		struct VSysClock time2;
		GetSysClock(&time2,  SYSCLOCK);
		pn103->data[data_index++] = (time2.wMSecond+time2.bySecond*1000)&0xff;
		pn103->data[data_index++] = (time2.wMSecond+time2.bySecond*1000)>>8;
		pn103->data[data_index++] = time2.byMinute;
		pn103->data[data_index++] = time2.byHour;
		pn103->data[data_index++] = time2.byDay;
		pn103->data[data_index++] = time2.byMonth;
		pn103->data[data_index++] = time2.wYear-2000;
		m_SendBuf.wWritePtr += data_index;
	}
	return 	m_SendBuf.wWritePtr ;
}
/***************************************************************
	Function��	ChkSum
	����:	BYTE *buf, 
			WORD len
	���أ�	��
***************************************************************/
BYTE CIpacs103P::ChkSum(BYTE *buf, WORD len)
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
void CIpacs103P::SendFrame10_Cmd0()
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
void CIpacs103P::SendFrame10_Cmd07()
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
void CIpacs103P::SendFrame10_Cmd10()
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
void CIpacs103P::SendFrame10_Cmd11()
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
void CIpacs103P::SendFrame68_ASDU06(BYTE addr)
{
	BYTE bFun = 0XFF;
	BYTE bInf = 0x00;
	
	SendFrameHead(FRM_I);
	MakeFrame68Head(TYPE_ASDU06_TIME,COT_TIMER,bFun,bInf);
	//m_pSend->byASDU[m_SendBuf.wWritePtr++] = 0x00;
	m_pSend->byAPDULenL=LOBYTE((m_SendBuf.wWritePtr-3));
	m_pSend->byAPDULenH=HIBYTE((m_SendBuf.wWritePtr-3));
	SendFrameTail();

	WriteData();
}

/***************************************************************
	Function��	SendFrame68_ASDU07
				�����ܲ�ѯ
	����:
	���أ�	��
***************************************************************/
void CIpacs103P::SendFrame68_ASDU07()
{
	BYTE bFun = 0XFF;
	BYTE bInf = 0x00;
	
	SendFrameHead(FRM_I);
	MakeFrame68Head(TYPE_ASDU07_ALLCALL,COT_ALLCALL,bFun,bInf);
	m_pSend->byASDU[m_SendBuf.wWritePtr++] = 0x00;
	m_pSend->byAPDULenL=LOBYTE((m_SendBuf.wWritePtr-3));
	m_pSend->byAPDULenH=HIBYTE((m_SendBuf.wWritePtr-3));
	SendFrameTail();

	WriteData();
}

BYTE CIpacs103P::Get_ykdata(BYTE tt,BYTE dd)
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
void CIpacs103P::SendFrame68_ASDU64(int ykId,BYTE type,BYTE data,BYTE fun,BYTE inf)
{
//	BYTE bVsq = 0;
	BYTE bFun = fun;
	BYTE bInf = inf;//��Ҫ����ʵ�����ȷ��
	BYTE ykdata;

	MakeFrame68Head(TYPE_ASDU64_YK,COT_REMOP,bFun,bInf);
	ykdata = Get_ykdata(type,data);
	m_pSend->byASDU[m_SendBuf.wWritePtr - APCI_LEN] = ykdata;
	m_pSend->byASDU[m_SendBuf.wWritePtr - APCI_LEN+1] = 0x00;
	m_SendBuf.wWritePtr+=2;
	
}

/***************************************************************
	Function��	SendFrameHead
	����:
	���أ�	��
***************************************************************/
void CIpacs103P::SendFrameHead(BYTE byFrameType)
{
    WORD wSendNum, wRecvNum;
    
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;//��շ��ͻ����� 
	
	m_pSend = (VIpacs103Frame *)m_SendBuf.pBuf;
	
	m_wSendNum %= MAX_FRAME_COUNTER;
	m_wRecvNum %= MAX_FRAME_COUNTER;
	wSendNum = m_wSendNum << 1;
	wRecvNum = m_wRecvNum << 1;
	
	m_pSend->byStartCode = HEAD_V;
	
	switch (byFrameType)
	{
	    case FRM_I:
		m_pSend->byControl1 = LOBYTE(wSendNum);
		m_pSend->byControl2 = HIBYTE(wSendNum);
		m_pSend->byControl3 = LOBYTE(wRecvNum);
		m_pSend->byControl4 = HIBYTE(wRecvNum);
		m_wAckRecvNum = m_wRecvNum;
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
	m_pSend->bSourceFactoryId = 0;
	m_pSend->bSourceAddrL= 	LOBYTE(GetOwnAddr());
	m_pSend->bSourceAddrH=  HIBYTE(GetOwnAddr());
	m_pSend->bDestinationFactoryId= 0;
	m_pSend->bDestinationAddrL= LOBYTE(GetEqpAddr());
	m_pSend->bDestinationAddrH= HIBYTE(GetEqpAddr());
	m_pSend->bSpace1= 0;
	m_pSend->bSpace2= 0;
	
	m_SendBuf.wWritePtr = APCI_LEN;
	return;
}
/***/
/***************************************************************
	Function��	SendFrameTail
	����:
	���أ�	��
***************************************************************/
void CIpacs103P::SendFrameTail()
{
	m_pSend->byAPDULenL=LOBYTE((m_SendBuf.wWritePtr-3));
	m_pSend->byAPDULenH=HIBYTE((m_SendBuf.wWritePtr-3));

	if((m_pSend->byControl1 & 0x1) == FRM_I)
		m_wSendNum ++;
	WriteData();
	return ;
}
void CIpacs103P::SendUFrame(BYTE byControl)
{
    	SendFrameHead(FRM_U);
	m_pSend->byControl1 = byControl;
	m_pSend->byAPDULenL=LOBYTE((m_SendBuf.wWritePtr-3));
	m_pSend->byAPDULenH=HIBYTE((m_SendBuf.wWritePtr-3));
	
    SendFrameTail();
    return;    
}
/***************************************************************
	Function��	SendFrame68_ASDU21
				ͨ�÷��������
	����:
	���أ�	��
***************************************************************/
void CIpacs103P::SendFrame68_ASDU21(BYTE flag)
{
	#if 0
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
	#endif
}

BOOL CIpacs103P::ReqUrgency(void)
{

	if((SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT) &&(fla == TRUE))
	{
		fla = FALSE;
		SendSFrame();
		DisableRetry();
		return TRUE;
	}
	if ((SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT) && SwitchClearEqpFlag(EQPFLAG_YKReq))
	{
		if (SendYkCommand()) 
			return TRUE;
	}

	return FALSE;
}

BOOL CIpacs103P::ReqCyc(void)
{
	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)
	{
		if((m_pBaseCfg->wCmdControl & BROADCASTTIME) ||(m_pBaseCfg->wCmdControl & CLOCKENABLE))
		{
	    	if (CheckClearTaskFlag(TASKFLAG_CLOCK))
	    	{
	            SendFrame68_ASDU06(0xff);
				DisableRetry();
	            return TRUE;
	    	}
		}
	}

	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)
	{
		if(CheckClearEqpFlag(EQPFLAG_YC))
		{
			CheckClearEqpFlag(EQPFLAG_YX);//Ϊ������һ��װ��
			//SendFrame68_ASDU07();
			SendAllCall();			
			return TRUE;
		}
	}
    return FALSE;
}
BOOL CIpacs103P::ReqNormal(void)
{
	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)
	{
		//SendUFrame(TESTFR_ACT);//u֡����
		DisableRetry();
		return TRUE;
	}
	
	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_DISCONNECT)
	{
		SendUFrame(STARTDT_ACT);//u֡����
		return TRUE;
	}
    return FALSE;
}

void CIpacs103P::DoGenGroupData_Asdu0A()
{
	DataGroup *p;
	IPACSAPCI *pApci;
	BYTE i;
	BYTE bHeadLen = 21;
	pApci = (IPACSAPCI *)&m_RecFrame.pBuf[bHeadLen];
	m_Offset = bHeadLen+2;

	for(i = 0;i<pApci->bNgd.bit.NO;i++)
	{
		p = (DataGroup *)&m_RecFrame.pBuf[m_Offset];
		/*
		���ڲ�֪����Щң����soe�������е�ʱ�궼δȡ��
		��������״̬����Ϊ�ǵ�ǰң��״̬��
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
				break;

		}
	}
}
void CIpacs103P::DoRecDYK()
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

void CIpacs103P::DoRecSYK()
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
void CIpacs103P::DoRecYC754()
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

void CIpacs103P::DoRecYC_F()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO;
	BYTE bRet = TRUE;
	short bYCValue;
	float fValue;
	int gain = 0;
	int div = 0;
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

	if(bYCValue & 0x0003)
	{
		fValue = 0;
	}	
	else
	{
		if(bNO<JZYC_NUM)
		{
			if((bNO<4)||(bNO==11))
			{
				gain = 6000;
				if(bNO ==11)
					div = 100;
				else
					div = 1000;
			}
			else if((bNO==12)||(bNO==13))
			{
				gain = 10392;
				div = 10000;
			}
			else if(bNO == 14)
			{
				gain = 1200;
				div =1000;
			}
			else
			{
				gain = 12000;
				div = 100;
			}
			bYCValue >>=3;
			fValue = bYCValue*100/100.0;
			if(fValue>=0)
				fValue += 0.5;
			else
				fValue -= 0.5;
			fValue =((fValue*gain/4095)/div);
			//fValue =int(fValue*1000+0.5)/1000.0;
			if(bYCValue == 0)
				fValue = 0;	
		}
		else
			fValue = 0;	
	}
	fYCValue.wNo = bNO;
	fYCValue.fValue =fValue;
	if(bRet == TRUE)		
		WriteYC_F(m_wEqpID,  1,&fYCValue);

	m_Offset += sizeof(DataGroup)+wLen;
	
}
void CIpacs103P::DoRecYX_TIME()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;
	struct VCalClock time;
	struct VDBSOE soe;
	struct VDBCOS cos;
 	VIpacs103Frame * pRec =(VIpacs103Frame *)m_RecFrame.pBuf;		
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
		if(pRec->byASDU[2]== COT_SPONT)
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

void CIpacs103P::DoRecYX_RET()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;

	struct VCalClock time;
	struct VDBSOE soe;
	struct VDBCOS cos;
 	VIpacs103Frame * pRec =(VIpacs103Frame *)m_RecFrame.pBuf;		
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
		if(pRec->byASDU[2] == COT_SPONT)
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

void CIpacs103P::DoRecYC_RET()
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

void CIpacs103P::DoRecYX_CP56Time2a()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;

	struct VCalClock time;
	struct VDBSOE soe;
	struct VDBCOS cos;
	
 	VIpacs103Frame * pRec =(VIpacs103Frame *)m_RecFrame.pBuf;		
	GetSysClock(&time, CALCLOCK);
	
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
		if(pRec->byASDU[2] == COT_SPONT)
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

void CIpacs103P::DoRecYX_CP56Time2aRET()
{
	DataGroup *p;
	WORD index = 0,wLen;
	BYTE *Pbuf;
	BYTE bFun,bInf,bNO,bYXstate;
	BYTE bRet = TRUE;

	struct VCalClock time;
	struct VDBSOE soe;
	struct VDBCOS cos;
	
 	VIpacs103Frame * pRec =(VIpacs103Frame *)m_RecFrame.pBuf;		
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
		if(pRec->byASDU[2] == COT_SPONT)
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
void CIpacs103P::DoRecYC_CP56Time2a()
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

void CIpacs103P::DoYkReturn()
{
	BYTE YKReq;
	VYKInfo *pYKInfo;
	IPACSAPCI *p;
	BYTE *Pbuf;
	BYTE bHeadLen = 21;
	p = (IPACSAPCI *)&m_RecFrame.pBuf[bHeadLen];
	Pbuf = &p->bGroup;

	
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

	pYKInfo->Head.wEqpID = m_wEqpID;
	pYKInfo->Info.wID = m_YKno;
	YKReq =Pbuf[6];
	pYKInfo->Info.byStatus = 0; 
	if(m_pReceive->byASDU[5]== 0xf9)
		pYKInfo->Head.byMsgID = MI_YKSELECT;
	else if(m_pReceive->byASDU[5] == 0xfa)
		pYKInfo->Head.byMsgID = MI_YKOPRATE;

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
#if 0
BOOL CIpacs103P::DoYKReq(void)
{
	CPPrimary::DoYKReq();
	
	if (SwitchClearEqpFlag(EQPFLAG_YKReq)) 
		if (SendYkCommand()) 
			return TRUE;
	return TRUE;
}
#endif
BOOL CIpacs103P::SendYkCommand(void)
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
	
	switch (pYKInfo->Info.byValue & 0x3)
	{
		case 0://�����ֺϷ�
			break;
		case 1://close��բ
			DCO = 2;
			break;
		case 2://open��բ
			DCO = 1;
			break;
		case 3://�Ƿ�
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
	SendFrameHead(FRM_I);
	MakeFrame68Head(TYPE_ASDU0AH_GEN_GROUPDATA,COT_GCWRITE,0xFE,yy);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  0x29;
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

void CIpacs103P::SendAllCall()
{
	SendFrameHead(FRM_I);
	MakeFrame68Head(TYPE_ASDU15H_GEN_READ,0x09,0xFE,INF_CALLSTOP_M);

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  0X09;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0X00;	
	SendFrameTail();
}
