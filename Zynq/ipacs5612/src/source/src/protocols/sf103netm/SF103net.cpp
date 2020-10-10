#include "SF103net.h"

#if (TYPE_USER == USER_SHANGHAIJY)
extern "C" void sf103m(WORD wTaskID)		
{
	CSF103NETm *psf103P = new CSF103NETm();	
	
	if (psf103P->Init(wTaskID) != TRUE)
	{
		psf103P->ProtocolExit();
	}
	
	psf103P->Run();			   	
}


CSF103NETm::CSF103NETm() : CPPrimary()
{
	  pCfg = new V103Para;
      memset(pCfg,0,sizeof(V103Para));
}

BOOL CSF103NETm::Init(WORD wTaskID)
{
	BOOL rc;
	int k,w =0;
//	char ip[4] ={0};
	VDBCOS yx;
	rc = CPPrimary::Init(wTaskID,1,(void *)pCfg,sizeof(V103Para));
	if (!rc)
		return FALSE;

	flaa = FALSE;
	
	ip_h = 0x05;
	ip_l =  m_pEqpInfo[m_wEqpNo].wDAddress;
	m_pReceive = (VIecsf103Frame *)m_RecFrame.pBuf;
	m_pSend	 = (VIecsf103Frame *)m_SendBuf.pBuf;
	SDeviceInfo = new DeviceInfo[m_wEqpNum];
	memset(SDeviceInfo,0,m_wEqpNum*sizeof(DeviceInfo));

	Auto_Recover = (AUTODELAY_RES *)malloc(sizeof(AUTODELAY_RES)*MAX_AUTO_RECOVER*MAXDEVICENUM);
	if(Auto_Recover == NULL) return ERROR;
	memset(Auto_Recover,0,sizeof(AUTODELAY_RES)*MAX_AUTO_RECOVER*MAXDEVICENUM);

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
	
	Ycgroup_num = pCfg->YCPara.Gr_num;
	Yxgroup_num = pCfg->YXPara.Gr_num;
	m_wSendNum = 0;	//Send Counter	
	m_wRecvNum = 0;	//Receive Counter
	m_wAckNum = 0;		//Receive Ack Counter
	m_wAckRecvNum = 0; //Have Ack Receive Counter
	m_dwTestTimerCount = 0;
	event_time = m_pBaseCfg->Timer.wScanData2/10;//二级数据轮询时间可设置
	commCtrl(m_wCommID, CCC_EVENT_CTRL|COMM_IDLE_ON, &event_time);
	m_dwCommCtrlTimer = event_time;
	return TRUE;
}
void CSF103NETm::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();  
}

void CSF103NETm::SetBaseCfg(void)
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

void CSF103NETm::CheckCfg(void)
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
void CSF103NETm::DoTimeOut(void)
{
    DWORD tick;
    DWORD timet;
	int i;
    AUTODELAY_RES	* pauto;
	CPPrimary::DoTimeOut();
	DWORD g_Time = 9;
	struct VDBSOE soe;
    struct VDBCOS cos;
	
	tick = Get100usCnt();
	for(i=0,pauto = Auto_Recover;i<MAXDEVICENUM*MAX_AUTO_RECOVER;i++,pauto++)
	{
		if(pauto->Dev_no)
		{
            timet = tick - pauto->tick;
			if(timet > g_Time*10000)
			{
                cos.wNo = soe.wNo =pauto->No;
                cos.byValue =soe.byValue = SWS_OFF;
                WriteSCOS(pauto->Dev_no, 1, &cos);
                soe.Time.wMSecond = pauto->VClock.wMSecond + 10000;
                soe.Time.dwMinute = pauto->VClock.dwMinute;
                if (soe.Time.wMSecond >= 60000)
                {
                   soe.Time.wMSecond -= 60000;
                   soe.Time.dwMinute +=1;
                }
                WriteSSOE(pauto->Dev_no, 1, &soe); 
                pauto->Dev_no = 0;
			}
		}
	}

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
DWORD CSF103NETm::SearchOneFrame(BYTE *Buf, WORD Len)
{
	m_pReceive = (VIecsf103Frame *)Buf;

	if(m_pReceive->byStartCode != 0x68)
		return FRAME_ERR |1;
	int len = m_pReceive->byAPDULen*256+m_pReceive->byAPDULen_L;
	if(len > Len)
		return FRAME_LESS;
	return FRAME_OK |(len+3);
}
/***************************************************************
	Function：DoReceive
			   接收报文处理
	参数：无		
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CSF103NETm::DoReceive()
{
	if (SearchFrame() != TRUE)
	{
		return FALSE;
	}
	m_pReceive = (VIecsf103Frame *)m_RecFrame.pBuf;
	RecFrame68();
	DoSend();
	return FALSE;
}

/***************************************************************
	Function：	RecFrame10
				接收固定帧
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame10()
{
		return TRUE;
}

/***************************************************************
	Function：	SetACDflg
				根据设备地址置对应acd位
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::SetACDflg(BYTE bAddr)
{
	SDeviceInfo[m_wEqpNo].transstep[SENDPRIDATA] = TRANSSEND;	
	return TRUE;
}
int CSF103NETm::Chose_ID(int YkNo,BYTE *fun,BYTE *inf)
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
BOOL CSF103NETm::SendYkCommand(void)
{
	WORD YkNo;
	BYTE DCO = 0;
	//BYTE Qualifier = 1;
//	BYTE bVsq =0;
	BYTE yy = 0;
	BYTE fun,inf = 0;
	BYTE zhi = 0x00;
	
	VYKInfo *pYKInfo;
	
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

	YkNo = pYKInfo->Info.wID;
	
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
			yy = YK_SE;
			break;
		case MI_YKOPRATE:
			yy = YK_ACT;
			break;
		case MI_YKCANCEL:
			yy = YK_CA;
			break;
	}
	Chose_ID(YkNo,&fun,&inf);
	SendFrameHead(FRM_I);
	zhi = yy + Yk_value;
	MakeFrame68Head(TYPE_ASDU10_GEN_GROUPDATA,COT_GCWRITE,fun,inf,zhi,DCO);
	m_pSend->byAPDULen_L= (m_SendBuf.wWritePtr-3)%256;
	m_pSend->byAPDULen = (m_SendBuf.wWritePtr-3)/256;
	SendFrameTail();
	return TRUE;		
}

void CSF103NETm::SendSFrame(void)
{
    SendFrameHead(FRM_S);
	m_pSend->byAPDULen_L= (m_SendBuf.wWritePtr-3)%256;
	m_pSend->byAPDULen = (m_SendBuf.wWritePtr-3)/256;
    SendFrameTail();
    return;    
}
void CSF103NETm::DoIFrame(void)
{
	WORD wAckNum, wRecvNum;

	flaa = TRUE;
	
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);
	wRecvNum = MAKEWORD(m_pReceive->byControl1, m_pReceive->byControl2);
	wAckNum >>= 1;
	wRecvNum >>= 1;
	m_wRecvNum = wRecvNum + 1;
	
	m_wAckNum = wAckNum;   
	
	switch (m_pReceive->byASDU[0])
	{
		case TYPE_ASDU05_SIGNINF:
			break;
		case TYPE_ASDU50_YC:
			RecFrame68_ASDU50();
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
		case TYPE_ASDU10_GEN_GROUPDATA:
			RecFrame68_ASDU10();
			break;
	}
}
void CSF103NETm::DoSFrame(void)
{
	WORD wAckNum;
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);	
	m_wAckNum = wAckNum >> 1;
}
void CSF103NETm::DoUFrame(void)
{
	if(m_pReceive->byControl1 == 0x0b)
	{
		SetAllEqpFlag(EQPFLAG_YC);
		SetTaskFlag(TASKFLAG_CLOCK);	
	}
	SDeviceInfo[m_wEqpNo].linkstate = LINK_CONNECT;
}
/***************************************************************
	Function：	RecFrame68
				接收变长帧
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68()
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
	Function：	RecFrame10_Cmd0
				接收确认帧上送标识
	参数:
	返回：	无
***************************************************************/

BOOL CSF103NETm::RecFrame10_Cmd0()
{
	return TRUE;
}
BYTE CSF103NETm::Get_Value(BYTE *buf,BYTE dateType,BYTE datelen,DWORD *val1,float *val2)
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
void CSF103NETm::YkResult(BOOL Result)
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
void CSF103NETm::ykresult(BYTE cot,BYTE type)
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
/***************************************************************
	Function：	judge_yc_dd
	参数:
	返回：		
	0：错误
	1：遥测
	2：遥脉
***************************************************************/
int CSF103NETm::judge_yc_dd(BYTE fun,BYTE inf)
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
	Function：	Choese_Station
	参数:
	返回：		
***************************************************************/
AUTODELAY_RES *CSF103NETm::Choese_Station()
{
    AUTODELAY_RES *Auto = Auto_Recover;
	for(BYTE k = 0;k<MAXDEVICENUM*MAX_AUTO_RECOVER;k++)
	{
		if(!Auto->Dev_no)
			return Auto;
		else 
			Auto++;
	}
 return NULL;
}

/***************************************************************
	Function：	Write_Auto_Recov
	参数:
	返回：		
***************************************************************/
void CSF103NETm::Write_Auto_Recov(WORD dev_no,WORD no, struct VSysClock *time)
{
    AUTODELAY_RES *Auto;
    Auto = Choese_Station();
    struct VCalClock time2; 
    ToCalClock(time, &time2);
    Auto->VClock.dwMinute = time2.dwMinute;
    Auto->VClock.wMSecond = time2.wMSecond;
    Auto->Dev_no = dev_no;
    Auto->No = no;
    Auto->tick = Get100usCnt();
}
/***************************************************************
	Function：	RecFrame68_ASDU01
				接收上送压板及告警开关量状态
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU01()
{
	BYTE bFun = 0;
	BYTE bInf,data_index = 0;
	BYTE bYXstate,ret = 0;
	struct VDBCOS cos;
	struct VDBSOE soe;

	struct VSysClock time1;
    struct VCalClock time2;  
	GetSysClock(&time1, SYSCLOCK);

	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;
	
	BYTE num = pn103_info->vsq &(~VSQ_SQ_1);
	
	bFun = pn103_info->fun;
	bInf = pn103_info->info;
	
	 ret= Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	for(BYTE i =0;i<num;i++)
	{
		bYXstate = pn103_info->data[0+5*i];
		if(bYXstate == 1)
			bYXstate = SWS_OFF;
		else if(bYXstate == 2)
			bYXstate = SWS_ON;
		else
			return TRUE;
			time1.wMSecond = pn103_info->data[5*i+1]+pn103_info->data[5*i+2]*256;
			if(pn103_info->data[5*i+3]&0x80)
				;
			else
				time1.byMinute = pn103_info->data[5*i+3]&0x7f;
		time1.byHour= pn103_info->data[8+i*4]; 

		CalClockTo(&time2, &time1);

		soe.byValue =cos.byValue = bYXstate;
		soe.wNo = cos.wNo = data_index;
		memcpy(&soe.Time,&time2,sizeof(VCalClock));
		WriteSCOS(m_wEqpID, 1, &cos);
		WriteSSOE(m_wEqpID, 1,&soe);
		
	}
	return TRUE;
}
/***************************************************************
	Function：	RecFrame68_ASDU02
				接收上送保护动作信息
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU02()
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

	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;

	bFun = pn103_info->fun;
	bInf = pn103_info->info;
	bYxNum = (pn103_info->vsq & (~VSQ_SQ_1));

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	
	for(i=0;i<bYxNum;i++)
	{
		time1.wMSecond = pn103_info->data[i+5]+pn103_info->data[i+6]*256;
		if(pn103_info->data[i+7]&0x80)
			;
		else
		{
			time1.byMinute = pn103_info->data[i+7]&0x7f;
			time1.byHour= pn103_info->data[8+i]; 
		}
		CalClockTo(&time2, &time1);
		
		yx = pn103_info->data[i];
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
BOOL CSF103NETm::RecFrame68_ASDU05()
{
		return TRUE;
}
/***************************************************************
	Function：	RecFrame68_ASDU08
				接收查询结束
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU08()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU23
				接收上送被记录的扰动表
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU23()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU26
				接收扰动数据传输准备就绪
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU26()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU27
				接收被记录的通道传输准备就绪
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU27()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU28
				接收带标志的状态变位传输准备就绪
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU28()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU29
				接收带标志的状态变位传输
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU29()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU30
				接收传输扰动值
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU30()
{
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU31
				接收扰动数据传输结束
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU31()
{
	return TRUE;
}
/***************************************************************
	Function：	RecFrame68_ASDU32
				接收遥测数据
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU32()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE wYcNum = 0;
	WORD wYcBeginNo = 0;
	BYTE *bYCbuf;
	BYTE data_index = 0;
	BOOL ret =FALSE;
	
	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;
	
	bFun = pn103_info->fun;
	bInf = pn103_info->info;
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	wYcNum =pn103_info->vsq & (~VSQ_SQ_1);
	wYcBeginNo = data_index;
	bYCbuf = &pn103_info->data[0];

	WriteRangeYC(m_wEqpNo, wYcBeginNo, wYcNum, (short *)bYCbuf);
	return TRUE;
}

/***************************************************************
	Function：	RecFrame68_ASDU38
				接收变位上送步位置
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU38()
{
	return TRUE;
}
/***************************************************************
	Function：	RecFrame68_ASDU39
				接收步位置SOE
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU39()
{
	return TRUE;
}
/***************************************************************
	Function：	RecFrame68_ASDU40
				接收变位遥信数据
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU40()
{
	BYTE bFun = 0;
	BYTE bInf = 0;		
	BYTE data_index = 0;
	BOOL ret = FALSE;
	VDBCOS RecCOS;

	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;
	
	
	bFun = pn103_info->fun;
	bInf = pn103_info->info;
	
	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	RecCOS.byValue = pn103_info->data[0];
	if(RecCOS.byValue & 0x80)// 检查品质描述
	{
		return FALSE;
	}
	else
	{
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
BOOL CSF103NETm::RecFrame68_ASDU41()
{
	BYTE bFun = 0;
	BYTE bInf = 0;		
	BOOL ret= FALSE;
	BYTE data_index = 0;
	VDBSOE RecSOE;
	BYTE stat;

	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;
	
	bFun = pn103_info->fun;
	bInf = pn103_info->info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	if(pn103_info->data[0] == 1)
		stat = SWS_ON;
	else
		stat = SWS_OFF;
	RecSOE.byValue = stat;
	Time4bytetoCalClock(&pn103_info->data[1],&RecSOE.Time);

	if(RecSOE.byValue & 0x80)// 检查品质描述
	{
		return FALSE;
	}
	else
	{
		RecSOE.wNo = data_index;
		WriteSSOE(m_wEqpID,1,&RecSOE);
	}
	return TRUE;
}
/***************************************************************
	Function：	Time4bytetoCalClock
				
	参数:
	返回：	无
***************************************************************/
void CSF103NETm::Time4bytetoCalClock(BYTE *bTime4,VCalClock *CalCloc)
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
BOOL CSF103NETm::RecFrame68_ASDU42()
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

	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;

	bFun = pn103_info->fun;
	bInf = pn103_info->info;
	bYxNum = (pn103_info->vsq & (~VSQ_SQ_1));

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
	if(!ret)
		return FALSE;
	
	for(i=0;i<bYxNum;i++)
	{
		yx = pn103_info->data[i];
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
		data_index++;
	}
	return TRUE;
}
/***************************************************************
	Function：	GetNum_Yx
	参数:
	返回：		
***************************************************************/
int CSF103NETm::GetNum_Yx()
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
BOOL CSF103NETm::RecFrame68_ASDU44()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	//BYTE bYxNum = 0;
	//BYTE i = 0;
	BYTE *wYxData;
	//WORD wYXaddr = 0;
	//BYTE bQsd = 0;
	BOOL ret = 0;
	BYTE data_index = 0;
	int total;

	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;
		
	bFun = pn103_info->fun;
	bInf = pn103_info->info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	wYxData = pn103_info->data;
	total = GetNum_Yx();
	WriteRangeSYX(m_wEqpID,data_index,total,wYxData);
	return TRUE;
}
/***************************************************************
	Function：	Chose_Dev
				找到fun，inf对应的序号
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::Chose_Dev(WORD no,BYTE bFun,BYTE bInf,BYTE *Gn,BYTE Flag)
{
	int num = 0;
	*Gn = 0;
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
	Function：	RecFrame68_ASDU50
				接收遥测数据
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU50()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYcNum = 0;
	BYTE i = 0;
	BOOL ret  = FALSE;
	BYTE  data_index =0 ;
	struct VDBYC bYcData;
	
	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;
	
	bFun = pn103_info->fun;
	bInf = pn103_info->info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	bYcNum = (pn103_info->vsq & (~VSQ_SQ_1));
	
	for(i=0;i<bYcNum;i++)
	{
		bYcData.wNo = data_index;
		bYcData.nValue = pn103_info->data[2*i]+ (pn103_info->data[2*i+1]*256);

		if(bYcData.nValue & 0x0002)//检查品质描述
		{
			break;
		}
		else
		{
			bYcData.nValue = (bYcData.nValue >> 3);
			WriteYC(m_wEqpID,1,&bYcData);
		}
		bInf++;
	}
	return TRUE;
}
/***************************************************************
	Function：	Find_Gz
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::Find_Gz(BYTE grop,BYTE entry)
{	
	int i,num =0;
	for(i =0;i<pCfg->DDPara.Gr_num;i++)
	{
		num = pCfg->DDPara.Para[i].inf +pCfg->DDPara.Para[i].num;
		if((pCfg->DDPara.Para[i].fun == grop) && (num > entry) &&(entry >= pCfg->DDPara.Para[i].inf))
			return TRUE;
		else
			continue;
	}
	return FALSE;
}
/***************************************************************
	Function：	RecFrame68_ASDU10_YX
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU10_YX()
{
	BYTE i,num = 0;
	BYTE bFun,bInf = 0;
	BYTE data_index,ret = 0;
	struct VCalClock time1;
	struct VSysClock time2;  
	struct VDBCOS cos;
	struct VDBSOE soe;
	GetSysClock(&time1, CALCLOCK);
	GetSysClock(&time2,  SYSCLOCK);
	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;
	TONGYONGINFO *gr = (TONGYONGINFO *)&(pn103_info->data[2]);

	num = pn103_info->data[1]&(~VSQ_SQ_1);

	for(i = 0;i<num;i++)
	{
		bFun = gr->groupno;
		bInf = gr->entyrno;
		ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
		if(!ret)
			return FALSE;
		BYTE stat = gr->data[0];
		if(stat == 0x01)
			stat = SWS_OFF;
		else if(stat == 0x02)
			stat = SWS_ON;
		soe.byValue =cos.byValue = stat;
		soe.wNo = cos.wNo = data_index;
		memcpy(&soe.Time,&time1,sizeof(VCalClock));
		WriteSCOS(m_wEqpID, 1, &cos);
		WriteSSOE(m_wEqpID, 1,&soe);
		if(Find_Gz(bFun,bInf))
			Write_Auto_Recov(m_wEqpID,data_index,&time2);
	}
	return 0;
}
/***************************************************************
	Function：	RecFrame68_ASDU10_YC
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU10_YC()
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
	
	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;
	
	bYcNum = (pn103_info->data[1] & (~VSQ_SQ_1));

	cot = pn103_info->cot;

	if((cot == COT_GCWRCON)|| (cot == COT_GCWRNACK) || (cot == COT_GCWRACK))
	{
		if(pn103_info->info == 0xF9)
			ykresult(cot,0);
		else if(pn103_info->info == 0xFa)
			ykresult(cot,1);
	}
	for(i=0;i<bYcNum;i++)
	{

		bFun = pn103_info->data[2+index];
		bInf = pn103_info->data[3+index];
		result = judge_yc_dd(bFun,bInf);
		if(result == 1)
			ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
		else if(result == 2)
			ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,3);
		
		if(!ret)
			return FALSE;
		type = Get_Value(&pn103_info->data[8+index],pn103_info->data[5+index],pn103_info->data[6+index],&data,&data1);
		
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
		index +=  pn103_info->data[6+index]+6;
	}
	return TRUE;
}
/***************************************************************
	Function：	RecFrame68_ASDU10
				通用分类数据
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU10()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYxNum = 0;
	BYTE i = 0;
	BYTE wYxData = 0;
	BYTE yx =0 ;
	BOOL ret = 0;
	BYTE data_index = 0;
	//struct VDBCOS cos;
	//struct VDBSOE soe;

	struct VSysClock time1;
 	struct VCalClock time2;  
	GetSysClock(&time1, SYSCLOCK);
	CalClockTo(&time2, &time1);
	
	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;
	

	if(pn103_info->cot == COT_GCWRCON || (pn103_info->cot == COT_GCWRACK))
		YkResult(TRUE);
	if(pn103_info->cot == COT_GCWRNACK)
		YkResult(FALSE);

	if(pn103_info->cot == 0x2A||(pn103_info->cot == 0x2))
	{
		return RecFrame68_ASDU10_YC();
	}
	if(pn103_info->cot == 0x01)
		return RecFrame68_ASDU10_YX();
	bYxNum = pn103_info->data[1]&(~VSQ_SQ_1);

	for(i = 0;i<bYxNum;i++)
	{
		bFun = pn103_info->data[2+7*i];
		bInf = pn103_info->data[3+7*i];

		ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,2);
		if(!ret)
			return FALSE;

		yx = pn103_info->data[7*i+8];
		if(yx ==1)
		{
			wYxData = SWS_OFF;
		}else if(yx ==2)
		{
			wYxData = SWS_ON;
		}else
			return FALSE;
		
		//soe.byValue =cos.byValue = wYxData;
		//soe.wNo = cos.wNo = data_index;
		//memcpy(&soe.Time,&time2,sizeof(VCalClock));
	    WriteRangeSYX(m_wEqpID,data_index,1,&wYxData);		
		//WriteSCOS(m_wEqpID, 1, &cos);
		//riteSSOE(m_wEqpID, 1,&soe);
	}
	
	return TRUE;	
}
/***************************************************************
	Function：	RecFrame68_ASDU44
				接收遥控镜像回送
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU64()
{
	YkResult(TRUE);
	return TRUE;	
}
/***************************************************************
	Function：	RecFrame68_ASDU51
				
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::RecFrame68_ASDU51()
{
	BYTE bFun = 0;
	BYTE bInf = 0;
	BYTE bYcNum = 0;
	BYTE i = 0;
	BOOL ret  = FALSE;
	BYTE  data_index =0 ;
	struct VDBYC bYcData;
	
	pn103Frame68 *pn103_info = (pn103Frame68 *)m_pReceive->byASDU;
	
	bFun = pn103_info->fun;
	bInf = pn103_info->info;

	ret = Chose_Dev(m_wEqpNo,bFun,bInf,&data_index,1);
	if(!ret)
		return FALSE;
	
	bYcNum = (pn103_info->vsq & (~VSQ_SQ_1));

	bYcData.wNo = data_index;
	bYcData.nValue = pn103_info->data[0]+ (pn103_info->data[1]*256);
	WriteYC(m_wEqpID,1,&bYcData);
	
	for(i=0;i<bYcNum-1;i++)
	{

	 	ret = Chose_Dev(m_wEqpNo,bFun,pn103_info->data[3*i+2],&data_index,1);
		if(!ret)
			return FALSE;
		bYcData.wNo = data_index;
		bYcData.nValue = pn103_info->data[3*i+3]+ (pn103_info->data[3*i+4]*256);

		if(bYcData.nValue & 0x0002)//检查品质描述
		{
			break;
		}
		else
		{
			bYcData.nValue = (bYcData.nValue >> 3);
			WriteYC(m_wEqpID,1,&bYcData);
		}
		bInf++;
	}
	return TRUE;
}
/***************************************************************
	Function：	DoReceviceYc
				接收到的遥测数据处理
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::DoReceviceYc(BYTE bAddr,BYTE bInf,WORD bData)
{
	return TRUE;
}

/***************************************************************
	Function：	WriteData
				发送数据
	参数:
	返回：	无
***************************************************************/
void CSF103NETm::WriteData(void)
{
	WriteToComm(GetEqpAddr());
}

/***************************************************************
	Function：	MakeControlCode
				组控制域
	参数:
	返回：	无
***************************************************************/
BYTE CSF103NETm::MakeControlCode(BYTE bCmd,BYTE FramList)
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
#if 1
BYTE CSF103NETm::MakeFrame10(BYTE bCmd,BYTE FramList)
{
	return 0;
}
#endif
/***************************************************************
	Function：	MakeFrame68Head
				组10帧
	参数:
	返回：	无
***************************************************************/
BYTE CSF103NETm::MakeFrame68Head(BYTE type, BYTE cot, BYTE fun, BYTE info,BYTE flag,BYTE zhi)
{
	BYTE data_index = 0;
	pn103Frame68 *pn103 = (pn103Frame68 *)m_pSend->byASDU;

	pn103->type = type;
	pn103->vsq = 0x81;
	pn103->cot   = cot;
	if(flag == 0xff)
		pn103->pubaddr = 0xff;
	else
		pn103->pubaddr = m_pEqpInfo[m_wEqpNo].wDAddress;
	pn103->fun   = fun;
	pn103->info  = info;
	if(flag == 0xff)
		;
	else
		pn103->data[data_index++] = 0x00;
	
	if(flag == 0xff)
	{
		VSysClock  time2;
		GetSysClock(&time2,  SYSCLOCK);
		DWORD mSecond = time2.bySecond*1000+time2.wMSecond;
		pn103->data[data_index++] = mSecond%256;
		pn103->data[data_index++] = mSecond/256;
		pn103->data[data_index++] = time2.byMinute;
		pn103->data[data_index++] = time2.byHour;
		pn103->data[data_index++] = (time2.byDay & 0x1f) | ((time2.byWeek << 5) & 0xe0); 
		pn103->data[data_index++] = time2.byMonth;
		pn103->data[data_index++] = time2.wYear-2000;
		m_SendBuf.wWritePtr += 6 + data_index;
		return 	m_SendBuf.wWritePtr ;
	}
	if(flag == 2)
	{
		pn103->data[data_index++] = 0x00; 
		m_SendBuf.wWritePtr += 6 + data_index;
		return 	m_SendBuf.wWritePtr ;
	}else if((flag == 0xee) || (flag == 0xef))
	{
		pn103->fun = 0xfe;
		if(flag == 0xef )
		{
			pn103->info  = 0xfa; 
			pn103->data[data_index++] = 0x00; 
			m_SendBuf.wWritePtr += 6 + data_index;
			return 	m_SendBuf.wWritePtr ;
		}
		else
		{
			pn103->info  = 0xf9; 

			pn103->data[data_index++] = 0x01;
			pn103->data[data_index++] = fun;
			pn103->data[data_index++] = info;
			pn103->data[data_index++] = 0x01; 

			pn103->data[data_index++] = 0x03;
			pn103->data[data_index++] = 0x01;
			pn103->data[data_index++] = 0x01;

			pn103->data[data_index++] =  zhi;
		}
	}
	else if(flag == 3)
	{
		pn103->data[data_index++] = 0x01; 
		pn103->cot = 0x2A;
		pn103->data[data_index++]= pCfg->YXPara.Para[Yxgroup_num-1].fun;
		pn103->data[data_index++]= 0x00;//pCfg->YCPara.Para[Ycgroup_num-1].inf;	
		Yxgroup_num--;
		if(Yxgroup_num)
			SetEqpFlag(TY_ON);
		else
			Yxgroup_num = pCfg->YXPara.Gr_num;

		pn103->data[data_index++] = 0x13;
	}
	else
	{
		pn103->data[data_index++] = 0x01; 

		#if 1
		if(flag == 2)
		{
			;
		}else if(flag == 1)
		{
			pn103->info  = 0xf1;
			pn103->data[data_index++]= pCfg->YCPara.Para[Ycgroup_num-1].fun;
			pn103->data[data_index++]= 0x00;	
			Ycgroup_num--;
			if(Ycgroup_num)
				SetEqpFlag(TY_ON);
			else
				Ycgroup_num = pCfg->YCPara.Gr_num;

		}
		else if(flag == 0)
		{
			pn103->data[data_index++]  = pCfg->DDPara.Para[0].fun;
			pn103->data[data_index++]  = 0x00;
		}
		#endif
		pn103->data[data_index++] = 0x01;
	}
	m_SendBuf.wWritePtr += 6 + data_index;
	return 	m_SendBuf.wWritePtr ;
}

/***************************************************************
	Function：	ChkSum
	参数:	BYTE *buf, 
			WORD len
	返回：	无
***************************************************************/
BYTE CSF103NETm::ChkSum(BYTE *buf, WORD len)
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
void CSF103NETm::SendFrame10_Cmd0()
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
void CSF103NETm::SendFrame10_Cmd07()
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
void CSF103NETm::SendFrame10_Cmd10()
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
void CSF103NETm::SendFrame10_Cmd11()
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
void CSF103NETm::SendFrame68_ASDU06(BYTE addr)
{
	#if 1
	BYTE bFun = 0xff;
	BYTE bInf = 0x00;//需要根据实际情况确定

	MakeFrame68Head(TYPE_ASDU06_TIME,COT_SYNTIME,bFun,bInf,0xff,0);
	#endif
}

/***************************************************************
	Function：	SendFrame68_ASDU07
				启动总查询
	参数:
	返回：	无
***************************************************************/
void CSF103NETm::SendFrame68_ASDU07(BYTE c)
{ 
	SendFrameHead(FRM_I);
	SendFrame68_ASDU21(c);
	m_pSend->byAPDULen_L= (m_SendBuf.wWritePtr-3)%256;
	m_pSend->byAPDULen = (m_SendBuf.wWritePtr-3)/256;
	SendFrameTail();

	WriteData();
}

/***************************************************************
	Function：	SendFrame68_ASDU58
				电能脉冲量召唤
	参数:
	返回：	无
***************************************************************/
void CSF103NETm::SendFrame68_ASDU88()
{
	;
}

/***************************************************************
	Function：	Get_ykdata
	参数:
	返回：	无
***************************************************************/
BYTE CSF103NETm::Get_ykdata(BYTE tt,BYTE dd)
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
void CSF103NETm::SendFrame68_ASDU64(int ykId,BYTE type,BYTE data)
{
//	BYTE bVsq = 0;
	BYTE bFun = pCfg->YKPara.Para[0].fun;
	BYTE bInf = pCfg->YKPara.Para[0].inf+ykId-1;//需要根据实际情况确定
	BYTE ykdata;

	MakeFrame68Head(TYPE_ASDU64_YK,COT_REMOP,bFun,bInf,4,0);
	ykdata = Get_ykdata(type,data);
	m_pSend->byASDU[m_SendBuf.wWritePtr++] = ykdata;
	m_pSend->byASDU[m_SendBuf.wWritePtr++] = 0x00;
	WriteData();
}
/***************************************************************
	Function：	SendFrameHead
	参数:
	返回：	无
***************************************************************/
void CSF103NETm::SendFrameHead(BYTE byFrameType)
{
    WORD wSendNum, wRecvNum;
    
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;//清空发送缓冲区 
	
	m_pSend = (VIecsf103Frame *)m_SendBuf.pBuf;
	
	m_wSendNum %= MAX_FRAME_COUNTER;
	m_wRecvNum %= MAX_FRAME_COUNTER;
	wSendNum = m_wSendNum << 1;
	wRecvNum = m_wRecvNum << 1;
	
	m_pSend->byStartCode = HEAD_V;
	m_pSend->addr_L = ip_l;
	m_pSend->addr_H = ip_h;
	
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
/***************************************************************
	Function：	SendFrameTail
	参数:
	返回：	无
***************************************************************/
void CSF103NETm::SendFrameTail()
{
	if((m_pSend->byControl1 & 0x1) == FRM_I)
		m_wSendNum ++;
	WriteData();
	return ;
}
/***************************************************************
	Function：	SendUFrame
	参数:
	返回：	无
***************************************************************/
void CSF103NETm::SendUFrame(BYTE byControl)
{
    SendFrameHead(FRM_U);
	m_pSend->byControl1 = byControl;
	m_pSend->byAPDULen_L= (m_SendBuf.wWritePtr-3)%256;
	m_pSend->byAPDULen = (m_SendBuf.wWritePtr-3)/256;
    SendFrameTail();
    return;    
}
/***************************************************************
	Function：	SendFrame68_ASDU21
				通用分类读命令
	参数:
	返回：	无
***************************************************************/
void CSF103NETm::SendFrame68_ASDU21(BYTE flag)
{
	#if 1
	BYTE bFun = 0xfe;
	BYTE bInf;//需要根据实际情况确定
//	BYTE Index = 0;
	if(flag == 3)
		bInf = 0xf1;
	else
		bInf = 0xf5;//遥脉，电镀

	MakeFrame68Head(TYPE_ASDU21_GEN_READ,COT_CALL,bFun,bInf,flag,0);
	#endif
}
/***************************************************************
	Function：	DoCommState
	参数:
	返回：	无
***************************************************************/
DWORD CSF103NETm::DoCommState(void)
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
	    SetTaskFlag(TASKFLAG_CLOCK);
		SetTaskFlag(TASKFLAG_ALLDATA);
		SetTaskFlag(TASKFLAG_DD);
	    thSleep(300);
	    SendUFrame(STARTDT_ACT);		  		
	}
	else
	{
		;
	}
	return dwCommState;

}
/***************************************************************
	Function：	ReqUrgency
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::ReqUrgency(void)
{

	if((SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)&&(flaa == TRUE))
	{
		SendSFrame();
		DisableRetry();
		flaa = FALSE;
		return TRUE;
	}
	
	if ((SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT) && SwitchClearEqpFlag(EQPFLAG_YKReq))
	{
		if (SendYkCommand()) 
			return TRUE;
	}
	return FALSE;
}
/***************************************************************
	Function：	Settime_clock
	参数:
	返回：	无
***************************************************************/
void CSF103NETm::Settime_clock(void)
{
	SendFrameHead(FRM_I);
	SendFrame68_ASDU06(0xff);
	m_pSend->byAPDULen_L= (m_SendBuf.wWritePtr-3)%256;
	m_pSend->byAPDULen = (m_SendBuf.wWritePtr-3)/256;
	SendFrameTail();

	WriteData();
}
/***************************************************************
	Function：	ReqCyc
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::ReqCyc(void)
{

	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)
	{
		if((m_pBaseCfg->wCmdControl & BROADCASTTIME) ||(m_pBaseCfg->wCmdControl & CLOCKENABLE))
		{
	    	if (CheckClearTaskFlag(TASKFLAG_CLOCK))
	    	{
	            Settime_clock();
				DisableRetry();
	            return TRUE;
	    	}
		}
	}

	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)
	{
		if(CheckClearEqpFlag(EQPFLAG_YC))
		{
			CheckClearEqpFlag(EQPFLAG_YX);//为了找下一个装置
			SendFrame68_ASDU07(2);
			return TRUE;
		}
	}
    return FALSE;
}
/***************************************************************
	Function：	ReqNormal
	参数:
	返回：	无
***************************************************************/
BOOL CSF103NETm::ReqNormal(void)
{
	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_CONNECT)
	{
		DisableRetry();
		return TRUE;
	}
	
	if(SDeviceInfo[m_wEqpNo].linkstate == LINK_DISCONNECT)
	{
		SendUFrame(STARTDT_ACT);//u帧启动
		return TRUE;
	}
    return FALSE;
}
#endif

