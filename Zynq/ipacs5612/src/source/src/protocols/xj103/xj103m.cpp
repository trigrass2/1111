#include "xj103m.h"
#if (TYPE_USER == USER_SHANGHAIJY)

extern "C" void xj103m(WORD wTaskID)		
{
	CXJ103m *pXj103P = new CXJ103m();	
	
	if (pXj103P->Init(wTaskID) != TRUE)
	{
		pXj103P->ProtocolExit();
	}
	
	pXj103P->Run();			   	
}


CXJ103m::CXJ103m() : CPPrimary()
{
	  pCfg = new V103Para;
      memset(pCfg,0,sizeof(V103Para));
}

BOOL CXJ103m::Init(WORD wTaskID)
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
	m_pReceive = (VXJ103Frame *)m_RecFrame.pBuf;
	m_pSend	 = (VXJ103Frame *)m_SendBuf.pBuf;
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
		
	m_wSendNum = 0;	//Send Counter	
	m_wRecvNum = 0;	//Receive Counter
	m_wAckNum = 0;		//Receive Ack Counter
	m_wAckRecvNum = 0; //Have Ack Receive Counter
	Ycgroup_num = pCfg->YCPara.Gr_num;
	m_dwTestTimerCount = 0;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|COMM_IDLE_ON, &event_time);
	return TRUE;
}

void CXJ103m::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();  
}

void CXJ103m::SetBaseCfg(void)
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
DWORD CXJ103m::DoCommState(void)
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

void CXJ103m::CheckCfg(void)
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
void CXJ103m::DoTimeOut(void)
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
DWORD CXJ103m::SearchOneFrame(BYTE *Buf, WORD Len)
{
	unsigned short FrameLen;
	m_pReceive = (VXJ103Frame *)Buf;
	

	if (Len < 6)
    	return FRAME_LESS;
	switch(m_pReceive->byStartCode)
	{
		case 0x68:
			FrameLen = m_pReceive->byAPDULen+ 2;
			if(FrameLen > Len)
			{
				return FRAME_LESS;
			}
			else
			{
				return FRAME_OK | FrameLen;
			}					
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
BOOL CXJ103m::DoReceive()
{
	if (SearchFrame() != TRUE)
	{
		return FALSE;
	}
	m_pReceive = (VXJ103Frame *)m_RecFrame.pBuf;
	RecFrame68();
	DoSend();
	return FALSE;
}

/***************************************************************
	Function��	RecFrame10
				���չ̶�֡
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame10()
{
		return TRUE;
}

/***************************************************************
	Function��	SetACDflg
				�����豸��ַ�ö�Ӧacdλ
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::SetACDflg(BYTE bAddr)
{
	SDeviceInfo[m_wEqpNo].transstep[SENDPRIDATA] = TRANSSEND;	
	return TRUE;
}
int CXJ103m::Chose_ID(int YkNo,BYTE *fun,BYTE *inf)
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
BOOL CXJ103m::SendYkCommand(void)
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
	if((inf ==48)||(inf ==49))
	{
		
		SendFrameHead(FRM_I);
		SendFrame68_YK(YkNo,yy,DCO,fun,inf);
		m_pSend->byAPDULen = m_SendBuf.wWritePtr - 2;
		WriteData();

	}
	else
	{
		if((yy ==SENDYKSELECT)||(yy ==SENDYKREVOCAT))
		{
			YkResult(TRUE);
			return TRUE;
		}
		else
		{
			SendFrameHead(FRM_I);
			SendFrame68_YK(YkNo,yy,DCO,fun,inf);
			m_pSend->byAPDULen = m_SendBuf.wWritePtr - 2;
			WriteData();

		}
	}
	return TRUE;		
}
/***************************************************************
	Function��	SendSFrame
	����:
	���أ�	��
***************************************************************/
void CXJ103m::SendSFrame(void)
{
    SendFrameHead(FRM_S);
	m_pSend->byAPDULen = m_SendBuf.wWritePtr-2;
    SendFrameTail();
    return;    
}
/***************************************************************
	Function��	DoIFrame
	����:
	���أ�	��
***************************************************************/
void CXJ103m::DoIFrame(void)
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
void CXJ103m::DoSFrame(void)
{
	WORD wAckNum;
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);	
	m_wAckNum = wAckNum >> 1;
}
void CXJ103m::DoUFrame(void)
{
	if(m_pReceive->byControl1 == STARTDT_CON)
	{
		SetAllEqpFlag(EQPFLAG_YC);
	}
	if(m_pReceive->byControl1 == TESTFR_ACT)
	{
		SendUFrame(TESTFR_CON); 
		DisableRetry();
		//thSleep(300);
		//SendUFrame(STARTDT_ACT);	
	}
	SDeviceInfo[m_wEqpNo].linkstate = LINK_CONNECT;
}
/***************************************************************
	Function��	RecFrame68
				���ձ䳤֡
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68()
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

BOOL CXJ103m::RecFrame10_Cmd0()
{
	return TRUE;
}
BYTE CXJ103m::Get_Value(BYTE *buf,BYTE dateType,BYTE datelen,DWORD *val1,float *val2)
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
void CXJ103m::YkResult(BOOL Result)
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
void CXJ103m::ykresult(BYTE cot,BYTE type)
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
int CXJ103m::judge_yc_dd(BYTE fun,BYTE inf)
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
BOOL CXJ103m::RecFrame68_ASDU01()
{
	BYTE bFun = 0;
	BYTE bInf,data_index = 0;
	BYTE bYXstate,ret = 0;
	struct VDBCOS cos;
	struct VDBSOE soe;

	struct VSysClock time1;
    struct VCalClock time2;  
	GetSysClock(&time1, SYSCLOCK);

	XJASDU *pXjasdu_info = (XJASDU *)m_pReceive->byASDU;

	if(pXjasdu_info->cot == COT_YKACK_01)
	{
		YkResult(TRUE);
	}
	else if(pXjasdu_info->cot == COT_YKNACK_01)
	{
		YkResult(FALSE);
	}
	else if(pXjasdu_info->cot == COT_YKState_01)
	{
		return TRUE;
	}
	else
	{	
		BYTE num = pXjasdu_info->vsq &0x7f;
		bFun = pXjasdu_info->fun;
		bInf = pXjasdu_info->info;
	    ret= Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
		if(!ret)
			return FALSE;
		for(BYTE i =0;i<num;i++)
		{
			bYXstate = pXjasdu_info->data[0+5*i];
			if(bYXstate == 1)
				bYXstate = SWS_OFF;
			else if(bYXstate == 2)
				bYXstate = SWS_ON;
			else
				return TRUE;
			time1.wMSecond = pXjasdu_info->data[5*i+1]+pXjasdu_info->data[5*i+2]*256;
			if(pXjasdu_info->data[5*i+3]&0x80)
					;
			else
				time1.byMinute = pXjasdu_info->data[5*i+3]&0x7f;
			time1.byHour= pXjasdu_info->data[4+i*5]; 

			ToCalClock(&time1, &time2);

			soe.byValue =cos.byValue = bYXstate;
			soe.wNo = cos.wNo = data_index;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSCOS(m_wEqpID, 1, &cos);
			WriteSSOE(m_wEqpID, 1,&soe);
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
BOOL CXJ103m::RecFrame68_ASDU02()
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
//    	struct VCalClock time2;  
	GetSysClock(&time1, SYSCLOCK);

	XJASDU *pXjasdu_info = (XJASDU *)m_pReceive->byASDU;

	bFun = pXjasdu_info->fun;
	bInf = pXjasdu_info->info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
		
	yx = pXjasdu_info->data[0];
	if(yx ==1)
	{
		wYxData = SWS_OFF;
	}else if(yx ==2)
	{
		wYxData = SWS_ON;
	}else
		return FALSE;

		time1.wMSecond = pXjasdu_info->data[5]+pXjasdu_info->data[6]*256;
		if(pXjasdu_info->data[7]&0x80)
				;
		else
			time1.byMinute = pXjasdu_info->data[7]&0x7f;
		time1.byHour= pXjasdu_info->data[8]; 

	//ToCalClock(&time1, &time2);	
	Time4bytetoCalClock(&pXjasdu_info->data[5],&soe.Time);

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
BOOL CXJ103m::RecFrame68_ASDU05()
{
		return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU08
				���ղ�ѯ����
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU08()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU23
				�������ͱ���¼���Ŷ���
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU23()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU26
				�����Ŷ����ݴ���׼������
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU26()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU27
				���ձ���¼��ͨ������׼������
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU27()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU28
				���մ���־��״̬��λ����׼������
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU28()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU29
				���մ���־��״̬��λ����
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU29()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU30
				���մ����Ŷ�ֵ
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU30()
{
	return TRUE;
}

/***************************************************************
	Function��	RecFrame68_ASDU31
				�����Ŷ����ݴ������
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU31()
{
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU32
				����ң������
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU32()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE wYcNum = 0;
	WORD wYcBeginNo = 0;
	WORD bYCbuf;
	BYTE data_index = 0;
	BOOL ret =FALSE;
	BYTE i = 0;
	XJASDU *pXjasdu_info = (XJASDU *)m_pReceive->byASDU;
	
	bFun = pXjasdu_info->fun;
	bInf = pXjasdu_info->info;
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	wYcNum =pXjasdu_info->vsq & (~VSQ_SQ_1);
	wYcBeginNo = data_index;
	for(i=0;i<wYcNum;i++)
	{
		bYCbuf = MAKEWORD(pXjasdu_info->data[2*i], pXjasdu_info->data[2*i+1]);
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
BOOL CXJ103m::RecFrame68_ASDU38()
{
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU39
				���ղ�λ��SOE
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU39()
{
	return TRUE;
}
/***************************************************************
	Function��	RecFrame68_ASDU40
				���ձ�λң������
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU40()
{
	BYTE bFun = 0;
	BYTE bInf = 0;		
	BYTE data_index = 0;
	BOOL ret = FALSE;
	int i =0;
	BYTE stat;
	BYTE buf[256];
	memset(buf,0,256);
	struct VDBCOS RecCOS;
	//struct VDBSOE soe;
	
    struct VCalClock time2;  
	GetSysClock(&time2, CALCLOCK);

	XJASDU *pXjasdu_info = (XJASDU *)m_pReceive->byASDU;
	int num = pXjasdu_info->vsq &(0x7f);
	bFun = pXjasdu_info->fun;
	bInf = pXjasdu_info->info;
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	
	if(pXjasdu_info->data[0] == 0x00)
			stat = SWS_OFF;
		else
			stat = SWS_ON;
	RecCOS.byValue =stat;
	RecCOS.wNo = data_index;
	WriteSCOS(m_wEqpID,1,&RecCOS);
	
	if(pXjasdu_info->vsq &VSQ_SQ_1)
	{//������
		//memcpy(&soe.Time,&time2,sizeof(VCalClock));
		memcpy(buf,&pXjasdu_info->data[1],(num-1)*3);
		for(i = 0;i<(num-1) ;i++)
		{
			ret = Chose_Dev(m_wEqpNo,buf[0+3*i],buf[1+3*i],&data_index,2);
			if(!ret)
				return FALSE;
			if(buf[2+3*i] == 0x00)
				stat = SWS_OFF;
			else
				stat = SWS_ON;

			//memcpy(&soe.Time,&time2,sizeof(VCalClock));
			RecCOS.byValue =stat;
			RecCOS.wNo = data_index;
			WriteSCOS(m_wEqpID,1,&RecCOS);
		}
	}
	else
	{
		memcpy(buf,&pXjasdu_info->data[1],num-1);
		for(i = 0;i<(num-1);i++)
		{
			data_index++;
			if(buf[i] == 0x00)
				stat = SWS_OFF;
			else
				stat = SWS_ON;
			RecCOS.byValue =stat;
			RecCOS.wNo = data_index;
			WriteSCOS(m_wEqpID,1,&RecCOS);
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
BOOL CXJ103m::RecFrame68_ASDU41()
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
	XJASDU *pXjasdu_info = (XJASDU *)m_pReceive->byASDU;
	
	bFun = pXjasdu_info->fun;
	bInf = pXjasdu_info->info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	
	if(pXjasdu_info->data[0] == 1)
		stat = SWS_ON;
	else
		stat = SWS_OFF;
	
	cos.byValue= RecSOE.byValue = stat;
	cos.wNo = RecSOE.wNo = data_index;
	
	time1.wMSecond = pXjasdu_info->data[1]+pXjasdu_info->data[2]*256;
	if(pXjasdu_info->data[3]&0x80)
			;
	else
		time1.byMinute = pXjasdu_info->data[3]&0x7f;
	time1.byHour= pXjasdu_info->data[4]; 
	CalClockTo(&RecSOE.Time, &time1);
	Time4bytetoCalClock(&pXjasdu_info->data[1],&RecSOE.Time);
	WriteSCOS(m_wEqpID,1,&cos);
	WriteSSOE(m_wEqpID,1,&RecSOE);
	return TRUE;
}
/***************************************************************
	Function��	Time4bytetoCalClock
				
	����:
	���أ�	��
***************************************************************/
void CXJ103m::Time4bytetoCalClock(BYTE *bTime4,VCalClock *CalCloc)
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
BOOL CXJ103m::RecFrame68_ASDU42()
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

	XJASDU *pXjasdu_info = (XJASDU *)m_pReceive->byASDU;

	bFun = pXjasdu_info->fun;
	bInf = pXjasdu_info->info;
	bYxNum = (pXjasdu_info->vsq & (~VSQ_SQ_1));

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	
	for(i=0;i<bYxNum;i++)
	{
		yx = pXjasdu_info->data[i];
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
int CXJ103m::GetNum_Yx()
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
BOOL CXJ103m::RecFrame68_ASDU44()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYxNum = 0;
	BYTE i = 0 ,j=0;
	BYTE wYxData,bYxState;
	BOOL ret = 0;
	BYTE data_index = 0;
	
	XJASDU *pXjasdu_info = (XJASDU *)m_pReceive->byASDU;
		
	bFun = pXjasdu_info->fun;
	bInf = pXjasdu_info->info;
	bYxNum = (pXjasdu_info->vsq & (~VSQ_SQ_1));

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;

	for(i = 0;i<bYxNum;i++)
	{
		wYxData = pXjasdu_info->data[5*i+0];
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
		
		wYxData = pXjasdu_info->data[5*i+1];
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
BOOL CXJ103m::Chose_Dev(WORD no,BYTE bFun,BYTE bInf,BYTE *Gn,BYTE Flag)
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
#if 0
		if(bFun == 248)
		{	
			for(k =0;k<40;k++)
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
		}else if(bFun == 178)
		{
			for(k =0;k<50;k++)
			{
				if(INF_3361[k] == bInf)
					{
						*Gn = k + pp->YXPara.Para[0].num;
						return TRUE;
					}
			}
			return FALSE;	
		}
#endif
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
BOOL CXJ103m::RecFrame68_ASDU50()
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
	
	XJASDU *pXjasdu_info = (XJASDU *)m_pReceive->byASDU;
	
	bFun = pXjasdu_info->fun;
	bInf = pXjasdu_info->info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	bYcNum = ((pXjasdu_info->vsq) & (~VSQ_SQ_1));
	
	for(i=0;i<bYcNum;i++)
	{
		//bYcData.wNo = data_index;
		fYCValue.wNo= data_index;
		YcTempValue = MAKEWORD(pXjasdu_info->data[2*i+0],pXjasdu_info->data[2*i+1]);
	
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
	Function��	RecFrame68_ASDU64
				����ң�ؾ������
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU64()
{
	XJASDU *pXjasdu_info = (XJASDU *)m_pReceive->byASDU;
	if(pXjasdu_info->cot ==COT_YKACK_64)
		YkResult(TRUE);
	else if(pXjasdu_info->cot ==COT_YKNACK_64)
		YkResult(FALSE);
	return TRUE;	
}

/***************************************************************
	Function��	RecFrame68_ASDU51
				
	����:
	���أ�	��
***************************************************************/
BOOL CXJ103m::RecFrame68_ASDU51()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYcNum = 0;
	BYTE i = 0;
	BOOL ret  = FALSE;
	BYTE  data_index =0 ;
	short YcTempValue = 0;
	 struct VDBYC_F fYCValue ;
	XJASDU *pXjasdu_info = (XJASDU *)m_pReceive->byASDU;
	
	bFun = pXjasdu_info->fun;
	bInf = pXjasdu_info->info;
	bYcNum = ((pXjasdu_info->vsq) & 0x7f);
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);	
	if(!ret)
		return FALSE;
	YcTempValue = MAKEWORD(pXjasdu_info->data[0],pXjasdu_info->data[1]);
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
		bInf = pXjasdu_info->data[3*i+2];
		ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);	
		if(!ret)
			return FALSE;
		YcTempValue = MAKEWORD(pXjasdu_info->data[3*i+3],pXjasdu_info->data[3*i+4]);

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
BOOL CXJ103m::DoReceviceYc(BYTE bAddr,BYTE bInf,WORD bData)
{
	return TRUE;
}

/***************************************************************
	Function��	WriteData
				��������
	����:
	���أ�	��
***************************************************************/
void CXJ103m::WriteData(void)
{
	WriteToComm(GetEqpAddr());
}

/***************************************************************
	Function��	MakeControlCode
				�������
	����:
	���أ�	��
***************************************************************/
BYTE CXJ103m::MakeControlCode(BYTE bCmd,BYTE FramList)
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
BYTE CXJ103m::MakeFrame10(BYTE bCmd,BYTE FramList)
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
BYTE CXJ103m::MakeFrame68Head(BYTE type, BYTE cot, BYTE fun, BYTE info)
{
	int data_index =0;
	XJASDU *pXjasdu = (XJASDU *)m_pSend->byASDU;
	
	pXjasdu->type = type;
	if(type == TYPE_ASDU64_YK)
	{
		pXjasdu->vsq = 0x01;
	}
	else
	{
		pXjasdu->vsq = 0x81;
	}
	pXjasdu->cot   = cot;
	if(cot == COT_TIMER)
		pXjasdu->pubaddr = 0xff;
	else
		pXjasdu->pubaddr = ((WORD)((BYTE)((HIBYTE(m_pEqpInfo[m_wEqpNo].wDAddress)))|(((WORD)((BYTE)((LOBYTE(m_pEqpInfo[m_wEqpNo].wDAddress)))))<< 8)));

	if((info == 48)||(info == 49))
	{
		if(type == TYPE_ASDU64_YK)
		{
			pXjasdu->pubaddr = 0x0002;
		}
	}
	else
	{
		if(type == TYPE_ASDU20_YK)
		{
			pXjasdu->pubaddr = 0x0001;
		}

	}

	pXjasdu->fun   = fun;
	pXjasdu->info  = info;
	m_SendBuf.wWritePtr += 8;
	if(COT_TIMER == cot)
	{
		struct VSysClock time2;
		GetSysClock(&time2,  SYSCLOCK);
		pXjasdu->data[data_index++] = (time2.wMSecond+time2.bySecond*1000)&0xff;
		pXjasdu->data[data_index++] = (time2.wMSecond+time2.bySecond*1000)>>8;
		pXjasdu->data[data_index++] = time2.byMinute;
		pXjasdu->data[data_index++] = time2.byHour;
		pXjasdu->data[data_index++] = time2.byDay;
		pXjasdu->data[data_index++] = time2.byMonth;
		pXjasdu->data[data_index++] = time2.wYear-2000;
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
BYTE CXJ103m::ChkSum(BYTE *buf, WORD len)
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
void CXJ103m::SendFrame10_Cmd0()
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
void CXJ103m::SendFrame10_Cmd07()
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
void CXJ103m::SendFrame10_Cmd10()
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
void CXJ103m::SendFrame10_Cmd11()
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
void CXJ103m::SendFrame68_ASDU06(BYTE addr)
{
	BYTE bFun = 0XFF;
	BYTE bInf = 0x00;
	
	SendFrameHead(FRM_I);
	MakeFrame68Head(TYPE_ASDU06_TIME,COT_TIMER,bFun,bInf);
	//m_pSend->byASDU[m_SendBuf.wWritePtr++] = 0x00;
	m_pSend->byAPDULen = m_SendBuf.wWritePtr-2;
	SendFrameTail();

	//WriteData();
}

/***************************************************************
	Function��	SendFrame68_ASDU07
				�����ܲ�ѯ
	����:
	���أ�	��
***************************************************************/
void CXJ103m::SendFrame68_ASDU07(BYTE addr)
{
	BYTE bFun = 0XFF;
	BYTE bInf = 0x00;
	
	SendFrameHead(FRM_I);
	MakeFrame68Head(TYPE_ASDU07_ALLCALL,COT_ALLCALL,bFun,bInf);
	m_pSend->byASDU[4] = addr;
	m_pSend->byASDU[m_SendBuf.wWritePtr++] = 0x00;
	m_pSend->byAPDULen = m_SendBuf.wWritePtr-2;
	SendFrameTail();

	//WriteData();
}

BYTE CXJ103m::Get_ykdata(BYTE fun,BYTE tt,BYTE dd)
{
	BYTE data = 0;
	if(fun ==1)
	{
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
	}
	else
	{
		if(tt == SENDYKACT)
		{
			if(dd == HE)
				data |= 0x02;
			if(dd == FEN)
				data |= 0x01;
		}
	}
	
	return data;
}
/***************************************************************
	Function��	SendFrame68_ASDU64
				ң��ѡ��ִ�С�����
	����:
	���أ�	��
***************************************************************/
void CXJ103m::SendFrame68_YK(int ykId,BYTE type,BYTE data,BYTE fun,BYTE inf)
{
//	BYTE bVsq = 0;
	BYTE bFun = fun;
	BYTE bInf = inf;//��Ҫ����ʵ�����ȷ��
	BYTE ykdata;
	if(fun ==1)
		MakeFrame68Head(TYPE_ASDU64_YK,COT_REMOP,bFun,bInf);
	else
		MakeFrame68Head(TYPE_ASDU20_YK,COT_CMDACK,bFun,bInf);
	ykdata = Get_ykdata(fun,type,data);
	m_pSend->byASDU[m_SendBuf.wWritePtr - APCI_LEN] = ykdata;
	m_pSend->byASDU[m_SendBuf.wWritePtr - APCI_LEN+1] = 0x00;
	m_SendBuf.wWritePtr+=2;
	
}

/***************************************************************
	Function��	SendFrameHead
	����:
	���أ�	��
***************************************************************/
void CXJ103m::SendFrameHead(BYTE byFrameType)
{
    WORD wSendNum, wRecvNum;
    
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;//��շ��ͻ����� 
	
	m_pSend = (VXJ103Frame *)m_SendBuf.pBuf;
	
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
	m_SendBuf.wWritePtr = APCI_LEN;
	return;
}
/***/
/***************************************************************
	Function��	SendFrameTail
	����:
	���أ�	��
***************************************************************/
void CXJ103m::SendFrameTail()
{
	if((m_pSend->byControl1 & 0x1) == FRM_I)
		m_wSendNum ++;
	WriteData();
	return ;
}
void CXJ103m::SendUFrame(BYTE byControl)
{
    SendFrameHead(FRM_U);
	m_pSend->byControl1 = byControl;
	m_pSend->byAPDULen = m_SendBuf.wWritePtr-2;
    SendFrameTail();
    return;    
}
/***************************************************************
	Function��	SendFrame68_ASDU21
				ͨ�÷��������
	����:
	���أ�	��
***************************************************************/
void CXJ103m::SendFrame68_ASDU21(BYTE flag)
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

BOOL CXJ103m::ReqUrgency(void)
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
BOOL CXJ103m::ReqCyc(void)
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
			SendFrame68_ASDU07(1);
			SendFrame68_ASDU07(2);
			return TRUE;
		}
	}
	commCtrl(m_wCommID, CCC_EVENT_CTRL|COMM_IDLE_ON, &event_time);
    return FALSE;
}
BOOL CXJ103m::ReqNormal(void)
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
#endif


