#include "Gb103.h"


extern "C" void gb103m(WORD wTaskID)		
{
	CGb103m *pGB103P = new CGb103m();	
	
	if (pGB103P->Init(wTaskID) != TRUE)
	{
		pGB103P->ProtocolExit();
	}
	
	pGB103P->Run();			   	
}


CGb103m::CGb103m() : CPPrimary()
{
	  pCfg = new V103Para;
      memset(pCfg,0,sizeof(V103Para));
}

BOOL CGb103m::Init(WORD wTaskID)
{
	BOOL rc;
	rc = CPPrimary::Init(wTaskID,1,(void *)pCfg,sizeof(V103Para));
	if (!rc)
	{
		return FALSE;
	}

	event_time = m_pBaseCfg->Timer.wScanData2/10;//二级数据轮询时间可设置
	m_pReceive = (Ipc103Frame *)m_RecFrame.pBuf;
	m_pSend	 = (Ipc103Frame *)m_SendBuf.pBuf;
	SDeviceInfo = new DeviceInfo[m_wEqpNum];
	memset(SDeviceInfo,0,m_wEqpNum*sizeof(DeviceInfo));

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
	w_YKno = 0;
	Ycgroup_num = pCfg->YCPara.Gr_num;
	m_dwTestTimerCount = 0;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	return TRUE;
}

void CGb103m::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();  
}

void CGb103m::SetBaseCfg(void)
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

void CGb103m::CheckCfg(void)
{
	CProtocol::CheckCfg();
}
/***************************************************************
	Function：OnTimeOut
		定时处理
	参数：TimerID
		TimerID 定时器ID
	返回：无
***************************************************************/
void CGb103m::DoTimeOut(void)
{
	m_dwTestTimerCount++;
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
	
	if(m_dwTestTimerCount %10 == 0)
		SDeviceInfo[m_wEqpNo].linkstate = LINK_DISCONNECT;
		
}
DWORD CGb103m::SearchOneFrame(BYTE *Buf, WORD Len)
{
	unsigned short FrameLen;
	WORD wLinkAddress;
	m_pReceive = (Ipc103Frame *)Buf;
	
	switch(m_pReceive->Frame10.head)
	{
		case 0x10:
			if (m_pReceive->Frame10.stop != TAIL_H)
				return FRAME_ERR|1;  
			FrameLen =5;
			wLinkAddress = m_pReceive->Frame10.address;
			if (FrameLen > Len)
				return FRAME_LESS;
			if (m_pReceive->Frame10.address!= GetEqpAddr())
				return FRAME_ERR | FrameLen;
			
			if (SwitchToAddress(wLinkAddress) == FALSE)
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
			wLinkAddress = m_pReceive->Frame68.linkaddr;
			if (wLinkAddress != GetEqpAddr())
				return FRAME_ERR | FrameLen;
			if (SwitchToAddress(wLinkAddress) == FALSE)
				return FRAME_ERR | FrameLen;
			return FRAME_OK | FrameLen;
		default:	
			return FRAME_ERR | 1;
	}
}
/***************************************************************
	Function：DoReceive
			   接收报文处理
	参数：无		
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CGb103m::DoReceive()
{
	if (SearchFrame() != TRUE)
	{
		return FALSE;
	}
	m_dwTestTimerCount = 0;
	m_pReceive = (Ipc103Frame *)m_RecFrame.pBuf;
	
	if (m_pReceive->Frame10.head == 0x10)
	{
		RecFrame10();
	}
	if (m_pReceive->Frame68.head == 0x68)
	{
		RecFrame68();
	}
	//commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	DoSend();
	return FALSE;
}

/***************************************************************
	Function：	RecFrame10
				接收固定帧
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame10()
{
	BYTE bCmd = 0;
	if(m_pReceive->Frame10.control & C_PRM)
	{
		return FALSE;
	}
	
	if(m_pReceive->Frame10.control & M_ACD)
	{
		//acd = 1；请求1级数据
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
	Function：	SetACDflg
				根据设备地址置对应acd位
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::SetACDflg(BYTE bAddr)
{
	SDeviceInfo[m_wEqpNo].transstep[SENDPRIDATA] = TRANSSEND;	
	return TRUE;
}
int CGb103m::Chose_ID(int YkNo,BYTE *fun,BYTE *inf)
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
	Function：	CheckYKOrderSend
				检出需要 遥控选择的设备
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::SendYkCommand(void)
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
		case 0://不区分合分
			break;
		case 1://close合闸
			DCO = HE;
			break;
		case 2://open分闸
			DCO = FEN;
			break;
		case 3://非法
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
		MakeFrame68Head(TYPE_ASDU64_YK,0X0C,fun,inf);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =  Get_ykdata(yy,DCO);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
		bVsq = 0X01;
		MakeFrame68Tail(CMD_CONF_M,bVsq,yy);
		WriteData();
	
		//SendFrame68_ASDU64(YkNo,yy,DCO);
	}
	
	return TRUE;		
}
/***************************************************************
	Function：	RecFrame68
				接收变长帧
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68()
{
	BYTE bAsduType = 0;
	if(m_pReceive->Frame68.control & C_PRM)
	{
		return FALSE;
	}
	
	if(m_pReceive->Frame68.control & M_ACD)
	{
		//acd = 1；请求1级数据
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
	Function：	RecFrame10_Cmd0
				接收确认帧上送标识
	参数:
	返回：	无
***************************************************************/

BOOL CGb103m::RecFrame10_Cmd0()
{
	return TRUE;
}
BYTE CGb103m::Get_Value(BYTE *buf,BYTE dateType,BYTE datelen,DWORD *val1,float *val2)
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
void CGb103m::YkResult()
{
	DoYkReturn();
	ClearEqpFlag(EQPFLAG_YKReq);

	return;
}
void CGb103m::ykresult(BYTE cot,BYTE type)
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
	结果:
	0：错误
	1：遥测
	2：遥脉
*/
int CGb103m::judge_yc_dd(BYTE fun,BYTE inf)
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
	Function：	RecFrame68_ASDU0A
				遥测和遥脉的处理
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU10()
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
	Function：	RecFrame68_ASDU01
				接收上送压板及告警开关量状态
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU01()
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
	Function：	RecFrame68_ASDU02
				接收上送保护动作信息
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU02()
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
		}else if(yx ==2)
		{
			wYxData = SWS_ON;
		}else
			return FALSE;
		
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
	Function：	RecFrame68_ASDU05
				接收上送标识
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU05()
{
		return TRUE;
}
/***************************************************************
	Function：	RecFrame68_ASDU08
				接收查询结束
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU08()
{
	if(m_pReceive->Frame68.cot == COT_CALLSTOP)
			SetEqpFlag(TY_ON);
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU23
				接收上送被记录的扰动表
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU23()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU26
				接收扰动数据传输准备就绪
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU26()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU27
				接收被记录的通道传输准备就绪
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU27()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU28
				接收带标志的状态变位传输准备就绪
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU28()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU29
				接收带标志的状态变位传输
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU29()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU30
				接收传输扰动值
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU30()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU31
				接收扰动数据传输结束
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU31()
{
	return TRUE;
}
/***************************************************************
	Function：	RecFrame68_ASDU32
				接收遥测数据
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU32()
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
	Function：	RecFrame68_ASDU36
				接收电能脉冲量
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU36()
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
	//电度需要转换
	//Ddbuf = m_pReceive->Frame68.data[0] + m_pReceive->Frame68.data[1]<<8 + m_pReceive->Frame68.data[2]<<16 + m_pReceive->Frame68.data[3]<<24;
	//bQds = m_pReceive->Frame68.dat[4];	

	WriteRangeDDFT(m_wEqpNo,data_index,bDdNum, &Ddbuf);
	return TRUE;
}
#endif
/***************************************************************
	Function：	RecFrame68_ASDU38
				接收变位上送步位置
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU38()
{
	return TRUE;
}
/***************************************************************
	Function：	RecFrame68_ASDU39
				接收步位置SOE
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU39()
{
	return TRUE;
}
/***************************************************************
	Function：	RecFrame68_ASDU40
				接收变位遥信数据
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU40()
{
	BYTE bFun = 0;
	BYTE bInf = 0;		
	BYTE data_index = 0;
	BOOL ret = FALSE;
	BYTE *pbuf;
	BYTE num,i;
	BYTE stat = SWS_OFF;
	VDBCOS RecCOS;
	struct VDBSOE soe;

    	struct VCalClock time2;  
	GetSysClock(&time2, CALCLOCK);

	
	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;

	num  = m_pReceive->Frame68.vsq &0x7f;
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;

	if( m_pReceive->Frame68.data[0] == 0x00)
		stat = SWS_OFF;
	else
		stat = SWS_ON;

	 RecCOS.byValue =stat;
	RecCOS.wNo = data_index;
	memcpy(&soe.Time,&time2,sizeof(VCalClock));
	WriteSCOS(m_wEqpID,1,&RecCOS);
		
	pbuf = &m_pReceive->Frame68.data[1];
	
	for(i = 0;i<num -1 ;i++)
	{
		ret = Chose_Dev(m_wEqpNo,pbuf[0+3*i],pbuf[1+3*i],&data_index,2);
		if(!ret)
			return FALSE;
		if(pbuf[2+3*i] == 0x00)
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
	Function：	RecFrame68_ASDU41
				接收SOE
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU41()
{
	BYTE bFun = 0;
	BYTE bInf = 0;		
	BOOL ret= FALSE;
	BYTE data_index = 0;
	BYTE stat; 
	VDBSOE RecSOE;
	VDBCOS RecCOS;
	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	
	if(m_pReceive->Frame68.data[0] == 1)
		stat = SWS_ON;
	else
		stat = SWS_OFF;
	
	RecCOS.byValue= RecSOE.byValue = stat;
	RecCOS.wNo = RecSOE.wNo = data_index;
	Time4bytetoCalClock(&m_pReceive->Frame68.data[1],&RecSOE.Time);
	WriteSCOS(m_wEqpID,1,&RecCOS);
	WriteSSOE(m_wEqpID,1,&RecSOE);
	
	return TRUE;
}
/***************************************************************
	Function：	Time4bytetoCalClock
				
	参数:
	返回：	无
***************************************************************/
void CGb103m::Time4bytetoCalClock(BYTE *bTime4,VCalClock *CalCloc)
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
	Function：	RecFrame68_ASDU42
				接收全遥信数据
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU42()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYxNum = 0;
	BYTE i = 0;
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
	
	for(i=0;i<bYxNum;i++)
	{
		yx = m_pReceive->Frame68.data[i];
		if(yx ==1)
		{
			wYxData = SWS_OFF;
		}else if(yx ==2)
		{
			wYxData = SWS_ON;
		}else
			return FALSE;
		
		cos.byValue = wYxData;
		cos.wNo = data_index;
		WriteSCOS(m_wEqpID, 1, &cos);
		//WriteRangeSYX(m_wEqpNo,wYXaddr,1,&wYxData);
		data_index++;
	}
	return TRUE;
}
int CGb103m::GetNum_Yx()
{
	int i,total = 0;
	for(i = 0;i<pCfg->YXPara.Gr_num;i++)
		total += pCfg->YXPara.Para[i].num;
	return total;
}
/***************************************************************
	Function：	RecFrame68_ASDU44
				接收全遥信数据
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU44()
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
	Function：	Chose_Dev
				找到fun，inf对应的序号
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::Chose_Dev(WORD no,BYTE bFun,BYTE bInf,BYTE *Gn,BYTE Flag)
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
	Function：	RecFrame68_ASDU50
				接收遥测数据
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU50()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYcNum = 0;
	BYTE i = 0;
	BOOL ret  = FALSE;
	BYTE  data_index =0 ;
	struct VDBYC bYcData;
	struct VDBYC_F fYCValue;
	bFun = m_pReceive->Frame68.fun;
	bInf = m_pReceive->Frame68.info;
	short YcTempValue;
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	bYcNum = (m_pReceive->Frame68.vsq & (~VSQ_SQ_1));
	
	for(i=0;i<bYcNum;i++)
	{
		fYCValue.wNo= bYcData.wNo = data_index;
		YcTempValue = bYcData.nValue = m_pReceive->Frame68.data[2*i]+ (m_pReceive->Frame68.data[2*i+1]*256);

		if(bYcData.nValue & 0x0002)//检查品质描述
		{
			break;
		}
		else
		{
			//bYcData.nValue = (bYcData.nValue >> 3);
			//WriteYC(m_wEqpID,1,&bYcData);
	 	    if (YcTempValue & 0x8000)
				YcTempValue = 0 - ((YcTempValue & 0x7fff)>>3);
		    else
				YcTempValue = (YcTempValue>>3) & 0x7fff;
			fYCValue.fValue =YcTempValue;
			WriteYC_F(m_wEqpID,  1,&fYCValue);
		}
		data_index++;
	}
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU44
				接收遥控镜像回送
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::RecFrame68_ASDU64()
{
	YkResult();
	
	return TRUE;	
}

/***************************************************************
	Function：	DoReceviceYc
				接收到的遥测数据处理
	参数:
	返回：	无
***************************************************************/
BOOL CGb103m::DoReceviceYc(BYTE bAddr,BYTE bInf,WORD bData)
{
	return TRUE;
}

/***************************************************************
	Function：	WriteData
				发送数据
	参数:
	返回：	无
***************************************************************/
void CGb103m::WriteData(void)
{
	WriteToComm(GetEqpAddr());
}

/***************************************************************
	Function：	MakeControlCode
				组控制域
	参数:
	返回：	无
***************************************************************/
BYTE CGb103m::MakeControlCode(BYTE bCmd,BYTE FramList)
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
	Function：	MakeFrame10
				组10帧
	参数:
	返回：	无
***************************************************************/
BYTE CGb103m::MakeFrame10(BYTE bCmd,BYTE FramList)
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
	Function：	MakeFrame68Head
				组68帧
	参数:
	返回：	无
***************************************************************/
BYTE CGb103m::MakeFrame68Head(BYTE type, BYTE cot, BYTE fun, BYTE info)
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
	Function：	MakeFrame68Tail_yk
		
	参数:
	返回：	无
***************************************************************/
BYTE CGb103m::MakeFrame68Tail_yk(BYTE bDevNum,BYTE bCmd,BYTE vsq,BYTE FramList,BYTE a,BYTE gr,BYTE enty)
{
	BYTE bFrameLen = 0;
	BYTE Index = 0;
	if(bDevNum == 0xff)//广播地址
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
	Function：	MakeFrame68Tail
				组68帧
	参数:		bDevNum = 0xff广播地址
	返回：	无
***************************************************************/
BYTE CGb103m::MakeFrame68Tail(BYTE bCmd,BYTE vsq,BYTE FramList)
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
	Function：	ChkSum
	参数:	BYTE *buf, 
			WORD len
	返回：	无
***************************************************************/
BYTE CGb103m::ChkSum(BYTE *buf, WORD len)
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
	Function：	SendFrame10_Cmd0
				发送复位通讯单元
	参数:
	返回：	无
***************************************************************/
void CGb103m::SendFrame10_Cmd0()
{
	BYTE bFrameLen = 0;
	
	bFrameLen = MakeFrame10(CMD_RESTU_M,SENDRESETCU);
	m_SendBuf.wWritePtr += bFrameLen;
	WriteData();
}

/***************************************************************
	Function：	SendFrame10_Cmd07
				发送复位帧计数位
	参数:
	返回：	无
***************************************************************/
void CGb103m::SendFrame10_Cmd07()
{
	BYTE bFrameLen = 0;
	
	bFrameLen = MakeFrame10(CMD_RESTFCB_M,SENDRESETFCB);
	m_SendBuf.wWritePtr += bFrameLen;
	WriteData();
}
/***************************************************************
	Function：	SendFrame10_Cmd11
				发送请求一级数据
	参数:
	返回：	无
***************************************************************/
void CGb103m::SendFrame10_Cmd10()
{
	BYTE bFrameLen = 0;
	
	bFrameLen = MakeFrame10(CMD_CALPRIDATA_M,SENDPRIDATA);
	m_SendBuf.wWritePtr += bFrameLen;
	WriteData();

	
}

/***************************************************************
	Function：	SendFrame10_Cmd11
				发送请求二级数据
	参数:
	返回：	无
***************************************************************/
void CGb103m::SendFrame10_Cmd11()
{
	BYTE bFrameLen = 0;
	
	bFrameLen = MakeFrame10(CMD_CALSECDATA_M,SENDSECDATA);
	m_SendBuf.wWritePtr += bFrameLen;
	WriteData();
}
/***************************************************************
	Function：	SendFrame68_ASDU06
				对时
	参数:
	返回：	无
***************************************************************/
void CGb103m::SendFrame68_ASDU06(BYTE addr)
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
	m_pSend->Frame68.pubaddr  = m_pSend->Frame68.linkaddr = m_pEqpInfo[m_wEqpNo].wDAddress;

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
	Function：	SendFrame68_ASDU07
				启动总查询
	参数:
	返回：	无
***************************************************************/
void CGb103m::SendFrame68_ASDU07()
{
	BYTE bVsq = 0;
	BYTE bFun = 0XFF;
	BYTE bInf = 0x00;

	MakeFrame68Head(TYPE_ASDU07_ALLCALL,COT_ALLCALL,bFun,bInf);
	m_pSend->Frame68.data[0] = 0x04;//SCN仅总查询有效，无意义
	m_SendBuf.wWritePtr += 1;
	bVsq = VSQ_SQ_1|0X01;
	MakeFrame68Tail(CMD_CONF_M,bVsq,SENDCALLALL);

	WriteData();
}

/***************************************************************
	Function：	SendFrame68_ASDU58
				电能脉冲量召唤
	参数:
	返回：	无
***************************************************************/
void CGb103m::SendFrame68_ASDU88()
{
	BYTE bVsq = 0;
	BYTE bFun = 0x01;
	BYTE bInf = 0x00;
	
	MakeFrame68Head(TYPE_ASDU88_CALLENERGY,COT_PERCYC,bFun,bInf);
	m_pSend->Frame68.data[0] = 0x00;//召唤命令限定词QCC
	bVsq = VSQ_SQ_1|0X01;
	MakeFrame68Tail(CMD_CONF_M,bVsq,SENDCALLENERGY);
	WriteData();
	
}
BYTE CGb103m::Get_ykdata(BYTE tt,BYTE dd)
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
	Function：	SendFrame68_ASDU64
				遥控选择、执行、撤销
	参数:
	返回：	无
***************************************************************/
void CGb103m::SendFrame68_ASDU64(int ykId,BYTE type,BYTE data)
{
	BYTE bVsq = 0;
	BYTE bFun = pCfg->YKPara.Para[0].fun;
	BYTE bInf = pCfg->YKPara.Para[0].inf+ykId-1;//需要根据实际情况确定
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
	Function：	SendFrame68_ASDU21
				通用分类读命令
	参数:
	返回：	无
***************************************************************/
void CGb103m::SendFrame68_ASDU21(BYTE flag)
{
	BYTE bFun = 0xfe;
	BYTE bInf;//需要根据实际情况确定
	BYTE Index = 0;
	bInf = 0xf1;//遥脉，电镀

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


BOOL CGb103m::DoYKReq(void)
{
	CPPrimary::DoYKReq();
	
	if (SwitchClearEqpFlag(EQPFLAG_YKReq)) 
		if (SendYkCommand()) 
			return TRUE;
	return TRUE;
}

#if 1
BOOL CGb103m::ReqUrgency(void)
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

void CGb103m::DoYkReturn()
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
		    break; //分
	    case 0x02: 
		    pYKInfo->Info.byValue = 0x05;
		    break; //合
	    default:
	    break;
	}

	
	CPPrimary::DoYKRet();
	
	return;	
	
}
BOOL CGb103m::ReqCyc(void)
{
	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)
	{
		if(m_pBaseCfg->wCmdControl & BROADCASTTIME)
		{
	    	if (CheckClearTaskFlag(TASKFLAG_CLOCK))
	    	{
	            SendFrame68_ASDU06(0xff);
				commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	            return TRUE;
	    	}
		}

		if(CheckClearEqpFlag(EQPFLAG_YC))
		{
			CheckClearEqpFlag(EQPFLAG_YX);//为了找下一个装置
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
BOOL CGb103m::ReqNormal(void)
{
	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)
	{
		SendFrame10_Cmd11();
		return TRUE;
	}
	
	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_DISCONNECT)
	{
		SendFrame10_Cmd0();//复位设备
		return TRUE;
	}
    return FALSE;
}
#endif


