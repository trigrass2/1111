/*------------------------------------------------------------------------
 $Rev: 3 $
------------------------------------------------------------------------*/
#include "syscfg.h"
	
#ifdef INCLUDE_GB101_M

#include "gb101m.h"

WORD g_ddfifflag=0, g_dddayflag=0, g_ddtideflag=0; // 记录回线号
BYTE g_ddhxnum=0; // 电度回线数
CGB101P *g_GB101p = 0;
BYTE TempID[6];
BYTE ReadParaFlag = 0;


/*
	DD_REAL_START_ADDR: 实时电度数据起始地址，默认6401
	DD_FIF_START_ADDR: 15min冻结起始地址，默认6409
	DD_DAY_START_ADDR: 日冻结起始地址，默认6411
	DD_TIDE_START_ADDR:潮流数据起始地址，默认6419

	以上起始地址可通过维护软件配置，M101规约扩展中:
	14-备用: 0使用默认值，0xFF 使用配置值
	15-备用: 实时电度数据起始地址
	16-备用: 15min冻结起始地址
	17-备用: 日冻结起始地址
	18-备用: 潮流数据起始地址
	19-备用: 电能量起始地址 默认6401
	
*/
WORD DD_START_ADDR=ADDR_DD_LO,DD_REAL_START_ADDR=0,DD_FIF_START_ADDR=0,DD_DAY_START_ADDR=0,DD_TIDE_START_ADDR=0;

extern "C" int ReadGb101mLLID(char*pbuf)
{
	ReadParaFlag = 0;
	memset(TempID,0,6);
	if(g_GB101p != NULL)
		g_GB101p->SendGetLineLossID(pbuf);
	else
		return ERROR;
	return OK;
}

extern "C" void gb101m(WORD wTaskID)		
{
	CGB101P *pGB101P = new CGB101P(); 	
	if(g_GB101p == NULL)
		g_GB101p = pGB101P;
	if (pGB101P->Init(wTaskID) != TRUE)
	{
		pGB101P->ProtocolExit();
	}
	
	pGB101P->Run();			   	
}

extern "C" void gb101m_load(int tid)
{
    char tname[30];
	
	sprintf(tname,"tGb101m%d",tid);		
	SetupCommonTask(tid, tname, COMMONPRIORITY, COMMONSTACKSIZE, (ENTRYPTR)gb101m, COMMONQLEN);
}


CGB101P::CGB101P() : CPPrimary()
{
	
}


BOOL CGB101P::Init(WORD wTaskID)
{
	BOOL rc;

	rc = CPPrimary::Init(wTaskID,1,&m_guiyuepara,sizeof(m_guiyuepara));
	if (!rc)
	{
		return FALSE;
	}

	Iec101PriInfo = new VIec101PriInfo[m_wEqpNum];
	for (int i = 0; i < m_wEqpNum; i++)
	{
		Iec101PriInfo[i].byStatus = S_INITOK;
		Iec101PriInfo[i].byLinkTimes = 0;
		Iec101PriInfo[i].byFCB = 0;
		Iec101PriInfo[i].bReqAllData = FALSE;
		Iec101PriInfo[i].bReqAllDd = FALSE;
	}
	m_w101mTaskID = 0;
	if(m_guiyuepara.tmp[6] > 0)
	{
		m_w101mTaskID = wTaskID;
	}
	#if 0
	m_pVIec101Cfg = (VIec101Cfg *)&m_Iec101Cfg;
	m_pVIec101Cfg->byLinkAdr = LOBYTE(LOWORD(m_pBaseCfg->dwRsv[0]));
	m_pVIec101Cfg->byCommAdr = HIBYTE(LOWORD(m_pBaseCfg->dwRsv[0]));
	m_pVIec101Cfg->byInfoAdr = LOBYTE(HIWORD(m_pBaseCfg->dwRsv[0]));
	m_pVIec101Cfg->byCOT = HIBYTE(HIWORD(m_pBaseCfg->dwRsv[0]));
	#endif
	m_pVIec101Cfg = (VIec101Cfg *)&m_Iec101Cfg;
	m_pVIec101Cfg->byLinkAdr =m_guiyuepara.linkaddrlen;// LOBYTE(LOWORD(m_pBaseCfg->dwRsv[0]));
	m_pVIec101Cfg->byCommAdr =m_guiyuepara.conaddrlen;// HIBYTE(LOWORD(m_pBaseCfg->dwRsv[0]));
	m_pVIec101Cfg->byInfoAdr = m_guiyuepara.infoaddlen;//LOBYTE(HIWORD(m_pBaseCfg->dwRsv[0]));
	m_pVIec101Cfg->byCOT =m_guiyuepara.COTlen;// HIBYTE(HIWORD(m_pBaseCfg->dwRsv[0]));
	ByTestFlag = 0;
	
	if (m_pVIec101Cfg->byLinkAdr == 1)
	{
		m_byLinkAdrShift = 0;
		m_byChksumShift = 1;
		m_byStopShift = 2;
		m_byTypeIDShift = 1;
		m_byQualifierShift = 2;
		m_byReasonShift = 3;
	}
	else
	{
		m_byLinkAdrShift = 0;
		m_byChksumShift = 2;
		m_byStopShift = 3;
		m_byTypeIDShift = 2;
		m_byQualifierShift = 3;
		m_byReasonShift = 4;
	}
	if (m_pVIec101Cfg->byCOT == 1)
	{
		m_byCommAdrShift = m_byReasonShift + 1;
	}
	else
	{
		m_byCommAdrShift = m_byReasonShift + 2;
	}
	if (m_pVIec101Cfg->byCommAdr == 1)
	{
		m_byInfoShift = m_byCommAdrShift + 1;
	}
	else
	{
		m_byInfoShift = m_byCommAdrShift + 2;
	}
	if(m_guiyuepara.tmp[0] == 0xFF)
	{
		DD_REAL_START_ADDR = m_guiyuepara.tmp[1];
		DD_FIF_START_ADDR  = m_guiyuepara.tmp[2];
		DD_DAY_START_ADDR  = m_guiyuepara.tmp[3];
		DD_TIDE_START_ADDR = m_guiyuepara.tmp[4];
		DD_START_ADDR      = m_guiyuepara.tmp[5];
	}
	else if(m_guiyuepara.tmp[0] == 1)
	{
		DD_REAL_START_ADDR = DD_CUR_ADDR;
		DD_FIF_START_ADDR  = 0x64B0;
		DD_DAY_START_ADDR  = 0x64B8;
		DD_TIDE_START_ADDR = 0x64C0;
		DD_START_ADDR      = ADDR_DD_LO;
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

	g_ddhxnum = g_Sys.Eqp.pInfo[m_wEqpID].pTEqpInfo->Cfg.wDDNum/32;

	//add by lqh 20060406
	//强制发送等待超时间隔为3 秒
	m_dwCommCtrlTimer = 150;//150*20ms=3seconds
	if (m_pBaseCfg->wCmdControl&ALLDATAPROC)
	{
		SetTaskFlag(TASKFLAG_ALLDATA);
	
	AllData.bFirst=TRUE;
	}
		
	return TRUE;	
}


void CGB101P::SetDefFlag(void)
{
	//must do nothing in gb101pri
	return;
}
void CGB101P::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();
	//if need to modify ,to do : 	
	//m_pBaseCfg->wCmdControl=YKOPENABLED | CLOCKENABLE | BROADCASTTIME | ALLDATAPROC|DDPROC;
}

void CGB101P::SetBaseCfg(void)
{
	VIec101Cfg *pVIec101Cfg;
	CProtocol::SetBaseCfg();
	//if need to modify ,to do :
	m_pBaseCfg->wCmdControl = YKOPENABLED | CLOCKENABLE  | ALLDATAPROC; /*| BROADCASTTIME*/
	for (int EqpNo = 0; EqpNo < m_wEqpNum; EqpNo++)
	{
		if (m_pEqpInfo[EqpNo].wDDNum  != 0)
		{
			m_pBaseCfg->wCmdControl |= DDPROC;
			break;
		}
		 
	}
	m_pBaseCfg->wMaxALLen = 1024;
	m_pBaseCfg->Timer.wAllData = 30;	//30 min
	m_pBaseCfg->Timer.wSetClock = 60;	//60 min
	m_pBaseCfg->Timer.wDD = 120;		//120 min
	m_pBaseCfg->wMaxRetryCount = 3;
	m_pBaseCfg->wMaxErrCount = 3;
	m_pBaseCfg->wBroadcastAddr = 0xFF;
	pVIec101Cfg = (VIec101Cfg *)m_pBaseCfg->dwRsv;
	pVIec101Cfg->byCommAdr = 1;
	pVIec101Cfg->byCOT = 1;
	pVIec101Cfg->byInfoAdr = 2;
	pVIec101Cfg->byLinkAdr = 1;
}

/***************************************************************
	Function：DoReceive
		接收报文处理
	参数：无
		
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CGB101P::DoReceive()
{
	BYTE byTypeId, byReason;

	if (SearchFrame() != TRUE)
		return FALSE;	

	m_dwTestTimerCount = 0;
		
	m_pReceive = (VIec101Frame *)m_RecFrame.pBuf;
	if (m_pReceive->Frame10.Start == 0x10)
		DoFrame10();
	
	if (m_pReceive->Frame68.Start1 == 0x68)
		DoFrame68();
	//{{add by lqh 20060406 
	//防止不发送报文
	DWORD dwCommCtrlTimer = 100;//=1 seconds
	commCtrl(m_wCommID, CCC_EVENT_CTRL|m_wCommCtrl, (BYTE*)&dwCommCtrlTimer);
	//}}
	byTypeId = m_pReceive->Frame68.Data[m_byTypeIDShift];
	byReason = m_pReceive->Frame68.Data[m_byReasonShift];
	if ((byTypeId == C_IC_NA) && (byReason == COT_ACTCON)
		|| ((byTypeId == M_ME_ND) && (byReason == COT_INTROGEN))
		|| ((byTypeId == M_SP_NA) && (byReason == COT_INTROGEN)))
		return FALSE;
	if ((byTypeId == C_CI_NA) && (byReason == COT_ACTCON)
		|| ((byTypeId == M_IT_NA) && (byReason == COT_SPONT)))
		return FALSE;
	if((byTypeId == M_EI_NA) && (byReason == COT_INIT))
		return FALSE;
	if (m_bPollMasterSendFlag)
	{
		DoSend();
	}
	else
	{
		commCtrl(m_wCommID, CCC_EVENT_CTRL|m_wCommCtrl, (BYTE*)&m_dwCommCtrlTimer);
	}
	return TRUE;
}


/***************************************************************
	Function：SearchOneFrame
		搜索一帧正确的报文
	参数：Buf, Len
		Buf 接收缓冲区头指针
		Len 接收缓冲区内有效数据的长度
	返回：DWORD数据，其中
		高字：处理结果
			#define FRAME_OK	   0x00010000	   //检测到一个完整的帧
			#define FRAME_ERR	   0x00020000	   //检测到一个校验错误的帧
			#define FRAME_LESS	   0x00030000	   //检测到一个不完整的帧（还未收齐）
		低字：已经处理的数据长度
***************************************************************/
DWORD CGB101P::SearchOneFrame(BYTE *Buf, WORD Len)
{
	unsigned short FrameLen;
	int i;
	WORD wLinkAddress;
	m_pReceive = (VIec101Frame *)Buf;
	
	if (m_pReceive->Frame10.Start == 0xe5)
		return FRAME_OK | 1;
	if ((m_pReceive->Frame10.Start != 0xe5) && (Len < (4 + m_pVIec101Cfg->byLinkAdr)))
		return FRAME_LESS;

	if(Len>8)
	{
		for(i=0;i<Len;i++)
		{
			if((Buf[i] == Buf[i+7])&&(Buf[i] == 0x68)&&(Buf[i+9] == 0x06))
			{
				memcpy(TempID,&Buf[i+1],6);
				ReadParaFlag =1;
			}	
		}
	}
	
	switch(m_pReceive->Frame10.Start)
	{
		case 0x10:
			if (m_pReceive->Frame10.Data[m_byStopShift] != 0x16)
				return FRAME_ERR|1;  
			FrameLen=4 + m_pVIec101Cfg->byLinkAdr;
			if (FrameLen > Len)
				return FRAME_LESS;
			if (m_pReceive->Frame10.Data[m_byChksumShift] != (BYTE)ChkSum((BYTE *)&m_pReceive->Frame10.Control, 2))
				return FRAME_ERR | 1;
			if (m_pVIec101Cfg->byLinkAdr ==1)
				wLinkAddress = m_pReceive->Frame10.Data[m_byLinkAdrShift];
			else
				wLinkAddress = MAKEWORD(m_pReceive->Frame10.Data[m_byLinkAdrShift],m_pReceive->Frame10.Data[m_byLinkAdrShift+1]);
			if (wLinkAddress != GetEqpAddr())
				return FRAME_ERR | FrameLen;
			
			if (SwitchToAddress(wLinkAddress) == FALSE)
				return FRAME_ERR | FrameLen;
			return FRAME_OK | FrameLen;

		case 0x68:
			if (m_pReceive->Frame68.Length1 != m_pReceive->Frame68.Length2)
				return FRAME_ERR | 1;
			if (m_pReceive->Frame68.Start2 != 0x68)
				return FRAME_ERR | 1;
			FrameLen = m_pReceive->Frame68.Length1 + 6;
		 
			if (FrameLen > Len)
				return FRAME_LESS;

			if (Buf[FrameLen-1] != 0x16)
				return FRAME_ERR | 1;
			if (Buf[FrameLen-2] != (BYTE)ChkSum((BYTE *)&m_pReceive->Frame68.Control,m_pReceive->Frame68.Length1))
				return FRAME_ERR | 1;
			if (m_pVIec101Cfg->byLinkAdr ==1)
				wLinkAddress = m_pReceive->Frame68.Data[m_byLinkAdrShift];
			else
				wLinkAddress = MAKEWORD(m_pReceive->Frame68.Data[m_byLinkAdrShift],m_pReceive->Frame68.Data[m_byLinkAdrShift+1]);
			if (wLinkAddress != GetEqpAddr())
				return FRAME_ERR | FrameLen;
			if (SwitchToAddress(wLinkAddress) == FALSE)
				return FRAME_ERR | FrameLen;
			return FRAME_OK | FrameLen;
		default:	
			return FRAME_ERR | 1;
	}
}


BOOL CGB101P::DoFrame10(void)
{
	if ((!m_guiyuepara.mode) && (m_pReceive->Frame10.Control & BITS_ACD))
		SetEqpFlag(ACD_ON);
	switch(m_pReceive->Frame10.Control & BITS_CODE)
	{
		case SFC_CONFIRM:
			if(!m_guiyuepara.mode)
				Iec101PriInfo[m_wEqpNo].byStatus = S_LINKOK;
			return TRUE; 
		case SFC_LINKOK: 
			Iec101PriInfo[m_wEqpNo].byStatus = S_RESETLINK;
			Iec101PriInfo[m_wEqpNo].byLinkTimes = 0;
			return TRUE; 
		case SFC_RECREQ:     //接收从方请求链路命令
			if(m_guiyuepara.mode)
				Iec101PriInfo[m_wEqpNo].byStatus = S_RECREQ;
			return TRUE;
		case SFC_RECRESET:   //接收从方复位通信命令
			if(m_guiyuepara.mode)
			{
				SendResetAck();
				thSleep(30);
				Iec101PriInfo[m_wEqpNo].byStatus = S_LINKOK;
			}
			return TRUE;
			
		case SFC_LINKBUSY: 
		case SFC_LINKNOTWORK: 
		case SFC_LINKNOTFINISH:
			return TRUE; 
		default:
			break;
	}
	return TRUE;
	
}

BOOL CGB101P::DoFrame68(void)
{
	getasdu();
	if ((!m_guiyuepara.mode) && (m_pReceive->Frame68.Control & BITS_ACD))
		SetEqpFlag(ACD_ON);
	if(m_guiyuepara.mode)
	{
		SendBaseFrame(0,0,0);
		DisableRetry();
	}
	
	switch(m_dwasdu.TypeID)
	{
		case M_SP_NA://不带时标的单点信息
		case M_DP_NA://不带时标的双点信息
	    	DoSingleYx();
	    	break;
		case M_SP_TA://带短时标的单点信息
			DoSingleSoe(SOE_TIME_SHORT);
			break;
	    case M_SP_TB://带长时标的单点信息
	    case M_DP_TB://带长时标的双点信息
	        DoSingleSoe(SOE_TIME_LONG);
	        break;
		case M_ME_ND://不带品质描述的测量值
			DoYcData();
			break;
		case M_ME_NA://不带品质描述的测量值
			DoYcFData();
			break;
		case M_ME_NC:
			DoYcFFData();
			break;
		case M_IT_NA://电能脉冲记数量
	        //DoDdData();
	        break;
		case M_IT_NB_1://电能量累积量，短浮点数
	        DoDdData();
	        break;
		case M_IT_NC_1://带时标的累积量，短浮点数
	        DoDdDataTime();
	        break;
		case M_EI_NA:
			SendReqAllData();
			break;
		case C_DC_NA://双点遥控命令
	        DoDoubleYk();
	        break;
		case C_IC_NA://召唤命令
			DoCallCommand();
			break;
		case C_CI_NA:
			DoCallDdCommand();
			break;
	    default:
			break;
	}

	return TRUE;
}

void CGB101P::DoCallCommand(void)
{
	if (m_pReceive->Frame68.Data[m_byReasonShift] == COT_ACTTERM)
		Iec101PriInfo[m_wEqpNo].bReqAllData = FALSE;
	return;
}

void CGB101P::DoCallDdCommand(void)
{
	if (m_pReceive->Frame68.Data[m_byReasonShift] == COT_ACTTERM)
		Iec101PriInfo[m_wEqpNo].bReqAllDd = FALSE;
	return;
}
void CGB101P::DoYcFData(void)
{
	if (m_dwasdu.VSQ&0x80)	
	{
		if ((m_pReceive->Frame68.Data[m_byReasonShift] >= COT_INRO9) && (m_pReceive->Frame68.Data[m_byReasonShift] <= COT_INRO12))
			DoGroupYcF();
		if ((m_pReceive->Frame68.Data[m_byReasonShift] == COT_INTROGEN)||(m_pReceive->Frame68.Data[m_byReasonShift] == COT_SPONT))
			DoGroupYcF();
	}
	else
		DoChangeYcF();
	return;	
}

void CGB101P::DoYcFFData(void)
{
	if (m_dwasdu.VSQ&0x80)	
	{
		if ((m_pReceive->Frame68.Data[m_byReasonShift] >= COT_INRO9) && (m_pReceive->Frame68.Data[m_byReasonShift] <= COT_INRO12))
			DoGroupYcFF();
		if ((m_pReceive->Frame68.Data[m_byReasonShift] == COT_INTROGEN)||(m_pReceive->Frame68.Data[m_byReasonShift] == COT_SPONT))
			DoGroupYcFF();
	}
	else
		DoChangeYcFF();
	return;	
}

void CGB101P::DoGroupYcFF(void)
{
	WORD wYcNum;
	WORD wYcBeginNo;
	//WORD *pYcValue = NULL;
	BYTE byLow, byHigh,byLow1,byHigh1;
	BYTE *pData = NULL;
	DWORD dycvalue;
	struct VYCF_F* pyc;
	pData = &(m_pReceive->Frame68.Data[m_byInfoShift]);
	pyc = (struct VYCF_F *)m_dwPubBuf;
	
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	switch (m_pVIec101Cfg->byInfoAdr)
	{
		case 1:
			byLow = *pData;
			pData ++;
			byHigh = 0;
			wYcBeginNo = MAKEWORD(byLow, byHigh) - 0x01;
			break;
		case 2:
			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData ++;
			wYcBeginNo = MAKEWORD(byLow, byHigh) - ADDR_YC_LO;
			break;
		case 3:
			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData += 2;
			wYcBeginNo = MAKEWORD(byLow, byHigh) - ADDR_YC_LO;
			break;
	}
	wYcNum = m_pReceive->Frame68.Data[m_byQualifierShift] & 0x7F;
	for (WORD i = 0; i < wYcNum; i++)
	{
		byLow = *pData;
		pData ++;
		byHigh = *pData;
		pData ++;
		byLow1 = *pData;
		pData++;
		byHigh1 = *pData;
		pData++;
		
		dycvalue = MAKEDWORD(MAKEWORD(byLow, byHigh),MAKEWORD(byLow1,byHigh1));
		pyc->fValue = *(float*)&dycvalue;
		pyc->byFlag =QDS(*pData ++);
		pyc++;	
	}
	WriteRangeYCF_F(m_wEqpID, wYcBeginNo, wYcNum, (struct VYCF_F *)m_dwPubBuf);
	return;	
}

void CGB101P::DoChangeYcFF(void)
{
	WORD wChangeYcNum;
	WORD wYcNo;
	struct VYCF_F * wYcValue;
	WORD wLow, wHigh,wLow1,wHigh1;
	BYTE *pData;
	DWORD dycvalue;
	
	pData = &(m_pReceive->Frame68.Data[m_byInfoShift]);
		wYcValue = (struct VYCF_F *)m_dwPubBuf;

	wChangeYcNum = m_pReceive->Frame68.Data[m_byQualifierShift] & 0x7F;
	for (WORD i = 0; i < wChangeYcNum; i++)
	{
		switch (m_pVIec101Cfg->byInfoAdr)
		{
			case 1:
				wLow = *pData;
				pData ++;
				wHigh = 0;
				wYcNo = MAKEWORD(wLow, wHigh) - 0x01;
				break;
			case 2:
				wLow = *pData;
				pData ++;
				wHigh = *pData;
				pData ++;
				wYcNo = MAKEWORD(wLow, wHigh) - ADDR_YC_LO;
				break;
			case 3:
				wLow = *pData;
				pData ++;
				wHigh = *pData;
				pData += 2;
				wYcNo = MAKEWORD(wLow, wHigh) - ADDR_YC_LO;
				break;
		}
		wLow = *pData;
		pData ++;
		wHigh = *pData;
		pData ++;
		wLow1 = *pData;
		pData++;
		wHigh1 = *pData;
		pData++;
		dycvalue = MAKEDWORD(MAKEWORD(wLow, wHigh),MAKEWORD(wLow1,wHigh1));
		wYcValue->fValue =  *(float*)&dycvalue;
		wYcValue->byFlag=QDS(*pData ++);
		WriteRangeYCF_F(m_wEqpID, wYcNo, 1, wYcValue);
	}
	return;	
}
	
void CGB101P::DoYcData(void)
{
	if (m_dwasdu.VSQ&0x80)	
	{
		if ((m_pReceive->Frame68.Data[m_byReasonShift] >= COT_INRO9) && (m_pReceive->Frame68.Data[m_byReasonShift] <= COT_INRO12))
			DoGroupYc();
		if ((m_pReceive->Frame68.Data[m_byReasonShift] == COT_INTROGEN)||(m_pReceive->Frame68.Data[m_byReasonShift] == COT_SPONT))
			DoGroupYc();
	}
	else
		DoChangeYc();
	return;	
}

void CGB101P::DoChangeYc(void)
{
	WORD wChangeYcNum;
	WORD wYcNo;
	WORD wYcValue;
	WORD wLow, wHigh;
	BYTE *pData;
	
	pData = &(m_pReceive->Frame68.Data[m_byInfoShift]);
	
	wChangeYcNum = m_pReceive->Frame68.Data[m_byQualifierShift] & 0x7F;
	for (WORD i = 0; i < wChangeYcNum; i++)
	{
		switch (m_pVIec101Cfg->byInfoAdr)
		{
			case 1:
				wLow = *pData;
				pData ++;
				wHigh = 0;
				wYcNo = MAKEWORD(wLow, wHigh) - 0x01;
				break;
			case 2:
				wLow = *pData;
				pData ++;
				wHigh = *pData;
				pData ++;
				wYcNo = MAKEWORD(wLow, wHigh) - ADDR_YC_LO;
				break;
			case 3:
				wLow = *pData;
				pData ++;
				wHigh = *pData;
				pData += 2;
				wYcNo = MAKEWORD(wLow, wHigh) - ADDR_YC_LO;
				break;
		}
		wLow = *pData;
		pData ++;
		wHigh = *pData;
		pData ++;
		wYcValue = MAKEWORD(wLow, wHigh);
		WriteRangeYC(m_wEqpID, wYcNo, 1, (short *)&wYcValue);
	}
	return;	
}
void CGB101P::DoChangeYcF(void)
{
	WORD wChangeYcNum;
	WORD wYcNo;
	struct VYCF * wYcValue;
	WORD wLow, wHigh;
	BYTE *pData;
	
	pData = &(m_pReceive->Frame68.Data[m_byInfoShift]);
		wYcValue = (struct VYCF *)m_dwPubBuf;

	wChangeYcNum = m_pReceive->Frame68.Data[m_byQualifierShift] & 0x7F;
	for (WORD i = 0; i < wChangeYcNum; i++)
	{
		switch (m_pVIec101Cfg->byInfoAdr)
		{
			case 1:
				wLow = *pData;
				pData ++;
				wHigh = 0;
				wYcNo = MAKEWORD(wLow, wHigh) - 0x01;
				break;
			case 2:
				wLow = *pData;
				pData ++;
				wHigh = *pData;
				pData ++;
				wYcNo = MAKEWORD(wLow, wHigh) - ADDR_YC_LO;
				break;
			case 3:
				wLow = *pData;
				pData ++;
				wHigh = *pData;
				pData += 2;
				wYcNo = MAKEWORD(wLow, wHigh) - ADDR_YC_LO;
				break;
		}
		wLow = *pData;
		pData ++;
		wHigh = *pData;
		pData ++;
		wYcValue->nValue= MAKEWORD(wLow, wHigh);
		wYcValue->byFlag=QDS(*pData ++);
		WriteRangeYCF(m_wEqpID, wYcNo, 1, wYcValue);
	}
	return;	
}

void CGB101P::DoGroupYc(void)
{
	WORD wYcNum;
	WORD wYcBeginNo;
	WORD *pYcValue = NULL;
	BYTE byLow, byHigh;
	BYTE *pData = NULL;
	
	pData = &(m_pReceive->Frame68.Data[m_byInfoShift]);
	pYcValue = (WORD *)m_dwPubBuf;
	
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	switch (m_pVIec101Cfg->byInfoAdr)
	{
		case 1:
			byLow = *pData;
			pData ++;
			byHigh = 0;
			wYcBeginNo = MAKEWORD(byLow, byHigh) - 0x01;
			break;
		case 2:
			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData ++;
			wYcBeginNo = MAKEWORD(byLow, byHigh) - ADDR_YC_LO;
			break;
		case 3:
			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData += 2;
			wYcBeginNo = MAKEWORD(byLow, byHigh) - ADDR_YC_LO;
			break;
	}
	wYcNum = m_pReceive->Frame68.Data[m_byQualifierShift] & 0x7F;
	for (WORD i = 0; i < wYcNum; i++)
	{
		byLow = *pData;
		pData ++;
		byHigh = *pData;
		pData ++;
		*pYcValue = MAKEWORD(byLow, byHigh);
		pYcValue++;	
	}
	WriteRangeYC(m_wEqpID, wYcBeginNo, wYcNum, (short *)m_dwPubBuf);
	return;	
}

void CGB101P::DoGroupYcF(void)
{
	WORD wYcNum;
	WORD wYcBeginNo;
	//WORD *pYcValue = NULL;
	BYTE byLow, byHigh;
	BYTE *pData = NULL;
	struct VYCF * pyc;
	pData = &(m_pReceive->Frame68.Data[m_byInfoShift]);
	pyc = (struct VYCF *)m_dwPubBuf;
	
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));
	switch (m_pVIec101Cfg->byInfoAdr)
	{
		case 1:
			byLow = *pData;
			pData ++;
			byHigh = 0;
			wYcBeginNo = MAKEWORD(byLow, byHigh) - 0x01;
			break;
		case 2:
			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData ++;
			wYcBeginNo = MAKEWORD(byLow, byHigh) - ADDR_YC_LO;
			break;
		case 3:
			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData += 2;
			wYcBeginNo = MAKEWORD(byLow, byHigh) - ADDR_YC_LO;
			break;
	}
	wYcNum = m_pReceive->Frame68.Data[m_byQualifierShift] & 0x7F;
	for (WORD i = 0; i < wYcNum; i++)
	{
		byLow = *pData;
		pData ++;
		byHigh = *pData;
		pData ++;
		pyc->nValue= MAKEWORD(byLow, byHigh);
		pyc->byFlag=QDS(*pData ++);
		pyc++;	
	}
	WriteRangeYCF(m_wEqpID, wYcBeginNo, wYcNum, (struct VYCF *)m_dwPubBuf);
	return;	
}
void CGB101P::DoDdData(void)
{
	WORD wDdNum,wDdNo;
	BYTE byLow, byHigh,byDd1, byDd2, byDd3, byDd4;
	DWORD dwDdValue;
	BYTE *pData = NULL;
	struct VDDFT Ddbuf;
	struct VDDFT *DdValue = (struct VDDFT* )m_dwPubBuf;
	
	pData = &(m_pReceive->Frame68.Data[m_byInfoShift]);
	wDdNum = m_pReceive->Frame68.Data[m_byQualifierShift] & 0x7f;
	if(!(m_pReceive->Frame68.Data[m_byQualifierShift] & 0x80))
	{
			for (WORD i = 0; i < wDdNum; i++)
			{
				switch (m_pVIec101Cfg->byInfoAdr)
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
				if(m_guiyuepara.tmp[0] == 1)
				{
					wDdNo = (wDdNo/8)*32+(wDdNo%8);
					WriteRangeDDFT(m_wEqpID,wDdNo,1, &Ddbuf);
				}		
				else
					WriteRangeDDFT(m_wEqpID,wDdNo,1, &Ddbuf);

			}
	}
	else
	{
		switch (m_pVIec101Cfg->byInfoAdr)
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
		
		WriteRangeDDFT(m_wEqpID,wDdNo,wDdNum, (struct VDDFT*)m_dwPubBuf);
	}
	return;	
}

void CGB101P::DoDdDataTime(void)
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
	struct VSysFlowEventInfo Flowinfo;
	BOOL flowflag = FALSE;

	memset(&Flowinfo , 0 , sizeof(struct VSysFlowEventInfo));
	pData = &(m_pReceive->Frame68.Data[m_byInfoShift]);
	wDdNum = m_pReceive->Frame68.Data[m_byQualifierShift] & 0x7f;
		if(!(m_pReceive->Frame68.Data[m_byQualifierShift] & 0x80))
		{
			for (WORD i = 0; i < wDdNum; i++)
			{
				switch (m_pVIec101Cfg->byInfoAdr)
				{
					case 1:
						byLow = *pData;
						pData ++;
						byHigh = 0;
						if(m_guiyuepara.tmp[0] == 1)
							wDdNo = MAKEWORD(byLow, byHigh) - DD_FIF_START_ADDR;
						else
							wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
						break;
					case 2:
						byLow = *pData;
						pData ++;
						byHigh = *pData;
						pData ++;
						if(m_guiyuepara.tmp[0] == 1)
							wDdNo = MAKEWORD(byLow, byHigh) - DD_FIF_START_ADDR;
						else
							wDdNo = MAKEWORD(byLow, byHigh) - DD_START_ADDR;
						break;
					case 3:
						byLow = *pData;
						pData ++;
						byHigh = *pData;
						pData += 2;
						if(m_guiyuepara.tmp[0] == 1)
							wDdNo = MAKEWORD(byLow, byHigh) - DD_FIF_START_ADDR;
						else
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

				if(m_pReceive->Frame68.Data[m_byReasonShift] == COT_SPONT)
				{
					if(m_guiyuepara.tmp[0] == 1)
					{
						for(j = 0; j < g_ddhxnum;j++)
						{
							if(((wDdNo+DD_START_ADDR) >= (DD_FIF_START_ADDR + j*24)) && ((wDdNo+DD_START_ADDR) < (DD_FIF_START_ADDR + DD_NUM+ j*24)))
							{
								g_ddfifflag |= 0x01<<j;
							}
						}

						for(j = 0; j < g_ddhxnum;j++)
						{
							if(((wDdNo+DD_START_ADDR) >= (DD_DAY_START_ADDR + j*24)) && ((wDdNo+DD_START_ADDR) < (DD_DAY_START_ADDR + DD_NUM+ j*24)))
							{
								g_dddayflag |= 0x01<<j;
							}
						}
						flowflag = FALSE;
						for(j = 0; j < g_ddhxnum;j++)
						{
							if(((wDdNo+DD_START_ADDR) >= (DD_TIDE_START_ADDR + j*24)) && ((wDdNo+DD_START_ADDR) < (DD_TIDE_START_ADDR + DD_NUM+ j*24)))
							{
								g_ddtideflag |= 0x01<<j;
								flowflag = TRUE;
							}
						}


					}
					else
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
						flowflag = FALSE;
						for(j = 0; j < g_ddhxnum;j++)
						{
							if(((wDdNo+DD_START_ADDR) >= (DD_TIDE_START_ADDR + j*DD_FLOW_LINE_NUM)) && ((wDdNo+DD_START_ADDR) < (DD_TIDE_START_ADDR + DD_NUM+ j*DD_FLOW_LINE_NUM)))
							{
								g_ddtideflag |= 0x01<<j;
								flowflag = TRUE;
							}
						}
					}
				}
				
				byLow = *pData;
				pData ++;
				byHigh = *pData;
				pData ++;
				SysTime.wMSecond = MAKEWORD(byLow, byHigh);
				SysTime.byMinute = (*pData)&0x3f;
				pData ++;
				SysTime.byHour = (*pData)&0x1f;
				pData ++;
				SysTime.byDay = (*pData)&0x1f;
				pData ++;
				SysTime.byMonth = (*pData)&0x0f;
				pData ++;

				if(*pData != 0)
				{
					SysTime.wYear = (*pData&0x7f) + 2000;
					CalendarTime.dwMinute = CalendarClock(&SysTime);
					CalendarTime.wMSecond = SysTime.wMSecond;	
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
				if(m_guiyuepara.tmp[0] == 1)
				{
					wDdNo = (wDdNo/24)*32+(wDdNo%24)+8;
					WriteRangeDDFT(m_wEqpID,wDdNo,1, &Ddbuf);
				}
				else
					WriteRangeDDFT(m_wEqpID,wDdNo,1, &Ddbuf);

				if((flowflag) && (Flowinfo.num < DD_NUM))
				{
					Flowinfo.dbflow[Flowinfo.num].Time = CalendarTime;
					Flowinfo.dbflow[Flowinfo.num].val = (long)dwDdValue;
					Flowinfo.dbflow[Flowinfo.num++].wNo = wDdNo;
				}
					
			}
			if(Flowinfo.num)
				 WriteFlowEvent(NULL, 0, 0, &Flowinfo);

		}
		else
		{
			 switch (m_pVIec101Cfg->byInfoAdr)
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
					DdValue->byFlag = 0x82;

					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData ++;
					SysTime.wMSecond = MAKEWORD(byLow, byHigh);
					SysTime.byMinute = (*pData)&0x3f;
					pData ++;
					SysTime.byHour = (*pData)&0x1f;
					pData ++;
					SysTime.byDay = (*pData)&0x1f;
					pData ++;
					SysTime.byMonth = (*pData)&0x0f;
					pData ++;
					
					if(*pData != 0)
					{
						SysTime.wYear = (*pData&0x7f) + 2000;
					
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

void CGB101P::DoSingleYx(void)
{
	if (m_dwasdu.COT == COT_SPONT)
		DoSingleCos();
	else
	{
		if ((m_dwasdu.COT >= COT_INRO1) && (m_pReceive->Frame68.Data[m_byReasonShift] <= COT_INRO8))
			DoSingleGroupYx();
		if (m_dwasdu.COT == COT_INTROGEN)
			DoSingleGroupYx();
	}	
	return;	
}

void CGB101P::DoSingleCos(void)
{
	WORD wCosNum;
	BYTE byLow, byHigh;
	BYTE *pData;
	BYTE YxValue;
	VDBCOS RecCOS;
	VDBDCOS RecDCOS;
	pData = &(m_pReceive->Frame68.Data[m_dwasdu.Infooff]);
	
	wCosNum = m_dwasdu.VSQ  & 0x7F;
	for (WORD i = 0; i < wCosNum; i++)
	{
		switch (m_guiyuepara.infoaddlen)
		{
			case 1:
				byLow = *pData;
				pData ++;
				byHigh = 0;
				YxValue = *pData;
				pData ++;
				RecCOS.wNo = dealdyxnum(MAKEWORD(byLow, byHigh) - 0x01);
				break;
			case 2:
				byLow = *pData;
				pData ++;
				byHigh = *pData;
				pData ++;
				YxValue = *pData;
				pData ++;
				RecCOS.wNo =dealdyxnum( MAKEWORD(byLow, byHigh) - ADDR_YX_LO);
				break;
			case 3:
				byLow = *pData;
				pData ++;
				byHigh = *pData;
				pData += 2;
				YxValue = *pData;
				pData ++;
				RecCOS.wNo = dealdyxnum(MAKEWORD(byLow, byHigh) - ADDR_YX_LO);
				break;
		}

		if(m_dwasdu.TypeID==1)
		{
			RecCOS.byValue=SIQ(YxValue);//QDP
			WriteSCOS(m_wEqpID, 1, &RecCOS);
		}
		if(m_dwasdu.TypeID==3)
		{
			DIQ(YxValue,&RecDCOS.byValue1,&RecDCOS.byValue2);//QDP
			WriteDCOS(m_wEqpID, 1, &RecDCOS);
		}
	}
	
	return;	
}

void CGB101P::DoSingleGroupYx(void)
{
	WORD wYxNum;
	WORD wYxBeginNo;
	BYTE *pData = NULL;
	BYTE *pYxBuf = NULL;
	BYTE byLow, byHigh;
	BYTE YxValue;
	VDBYX RecCOS;
	VDBDYX RecDCOS;
	struct VDYX *buf;
	pData = &(m_pReceive->Frame68.Data[m_dwasdu.Infooff]);
	pYxBuf = (BYTE *)m_dwPubBuf;
	buf=(struct VDYX  *)m_dwPubBuf;
	memset((void *)m_dwPubBuf, 0, sizeof(m_dwPubBuf));

	if (m_dwasdu.VSQ&0x80)	
	{

		switch (m_guiyuepara.infoaddlen)
		{
			case 1:
				byLow = *pData;
				pData ++;
				byHigh = 0;
				wYxBeginNo =dealdyxnum( MAKEWORD(byLow, byHigh) - 0x01);
				
				break;
			case 2:
				byLow = *pData;
				pData ++;
				byHigh = *pData;
				pData ++;
				wYxBeginNo = dealdyxnum(MAKEWORD(byLow, byHigh) - ADDR_YX_LO);
				
				break;
			case 3:
				byLow = *pData;
				pData ++;
				byHigh = *pData;
				pData += 2;
				wYxBeginNo = dealdyxnum(MAKEWORD(byLow, byHigh) - ADDR_YX_LO);
				
				break;
		}
		wYxNum = m_dwasdu.VSQ  & 0x7F;
		for (WORD i = 0; i < wYxNum; i++)
		{
			if(m_dwasdu.TypeID==1)
			{
				*pYxBuf = SIQ(*pData);
				pData ++;
				pYxBuf ++;
				
			}
			if(m_dwasdu.TypeID==3)
			{
				DIQ(*pData,&buf->byValue1,&buf->byValue2);//QDP
				pData ++;
				pYxBuf +=2;
			}		
		}
		if(m_dwasdu.TypeID==1)
		{
			WriteRangeSYX(m_wEqpID, wYxBeginNo, wYxNum,(BYTE *) m_dwPubBuf);
		}
		else if(m_dwasdu.TypeID==3)
			WriteRangeDYX(m_wEqpID, wYxBeginNo, wYxNum, buf);

	}
	else
	{
		wYxNum = m_dwasdu.VSQ & 0x7F;
		for (WORD i = 0; i < wYxNum; i++)
		{
			switch (m_guiyuepara.infoaddlen)
			{
				case 1:
					byLow = *pData;
					pData ++;
					byHigh = 0;
					YxValue = *pData;
					pData ++;
					RecCOS.wNo = dealdyxnum(MAKEWORD(byLow, byHigh) - 0x01);
					break;
				case 2:
					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData ++;
					YxValue = *pData;
					pData ++;
					RecCOS.wNo =dealdyxnum( MAKEWORD(byLow, byHigh) - ADDR_YX_LO);
					break;
				case 3:
					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData += 2;
					YxValue = *pData;
					pData ++;
					RecCOS.wNo = dealdyxnum(MAKEWORD(byLow, byHigh) - ADDR_YX_LO);
					break;
			}
		
			if(m_dwasdu.TypeID==1)
			{
				RecCOS.byValue=SIQ(YxValue);//QDP
				WriteSYX(m_wEqpID, 1, &RecCOS);
			}
			if(m_dwasdu.TypeID==3)
			{
				DIQ(YxValue,&RecDCOS.byValue1,&RecDCOS.byValue2);//QDP
				WriteDYX(m_wEqpID, 1, &RecDCOS,TRUE);
			}
		}
	}
	return;	
}


void CGB101P::DoSingleSoe(BYTE SoeTimeFormat)
{
	WORD wSoeNum;
	BYTE byLow, byHigh;
	BYTE *pData;
	BYTE YxValue;
	VDBSOE RecSOE;
	VDBDSOE RecDSOE;
	struct VDYX p;
	VSysClock SysTime;
	VCalClock CalendarTime;
	
	pData = &(m_pReceive->Frame68.Data[m_dwasdu.Infooff]);
	if (SoeTimeFormat == SOE_TIME_LONG)
	{
		wSoeNum = m_dwasdu.VSQ   & 0x7F;
	
		for (WORD i = 0; i < wSoeNum; i++)
		{
			switch (m_guiyuepara.infoaddlen)
			{
				case 1:
					byLow = *pData;
					pData ++;
					byHigh = 0;
					YxValue = *pData;
					pData ++;
					RecSOE.wNo = dealdyxnum(MAKEWORD(byLow, byHigh) - 0x01);
					break;
				case 2:
					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData ++;
					YxValue = *pData;
					pData ++;
					RecSOE.wNo = dealdyxnum(MAKEWORD(byLow, byHigh) - ADDR_YX_LO);
					break;
				case 3:
					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData += 2;
					YxValue = *pData;
					pData ++;
					RecSOE.wNo = dealdyxnum(MAKEWORD(byLow, byHigh) - ADDR_YX_LO);
					break;
			}
			if(m_dwasdu.TypeID==30)
				RecSOE.byValue = SIQ(YxValue);
			else if(m_dwasdu.TypeID==31)
				DIQ(YxValue,&RecDSOE.byValue1,&RecDSOE.byValue2);//QDP
			
			byLow = *pData;
			pData ++;
			byHigh = *pData;
			pData ++;
			SysTime.wMSecond = MAKEWORD(byLow, byHigh);
			SysTime.byMinute = (*pData)&0x3f;
			pData ++;
			SysTime.byHour = (*pData)&0x1f;
			pData ++;
			SysTime.byDay = (*pData)&0x1f;
			pData ++;
			SysTime.byMonth = (*pData)&0x0f;
			pData ++;
			SysTime.wYear = (*pData&0x7f) + 2000;
			pData ++;
			CalendarTime.dwMinute = CalendarClock(&SysTime);
			CalendarTime.wMSecond = SysTime.wMSecond;
		
			RecSOE.Time.dwMinute = CalendarTime.dwMinute;
			RecSOE.Time.wMSecond = CalendarTime.wMSecond;

			if(m_dwasdu.TypeID==30)
			{
				WriteRangeSYX(m_wEqpID, RecSOE.wNo, 1, &RecSOE.byValue);
				WriteSSOE(m_wEqpID, 1, &RecSOE);
			}
			else if(m_dwasdu.TypeID==31)
			{
				p.byValue1 = RecDSOE.byValue1;
				p.byValue2 = RecDSOE.byValue2;

				WriteRangeDYX(m_wEqpID, RecSOE.wNo, 1, &p); 
				WriteDSOE(m_wEqpID, 1, &RecDSOE);
			}
				
		}
	}

	if (SoeTimeFormat == SOE_TIME_SHORT)
	{
		wSoeNum = m_dwasdu.VSQ& 0x7F;
		GetSysClock((void *)&SysTime, SYSCLOCK);
		for (WORD i = 0; i < wSoeNum; i++)
		{
			switch (m_guiyuepara.infoaddlen)
			{
				case 1:
					byLow = *pData;
					pData ++;
					byHigh = 0;
					YxValue = *pData;
					pData ++;
					RecSOE.wNo = MAKEWORD(byLow, byHigh) - ADDR_YX_LO;
					break;
				case 2:
					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData ++;
					YxValue = *pData;
					pData ++;
					RecSOE.wNo = MAKEWORD(byLow, byHigh) - ADDR_YX_LO;
					break;
				case 3:
					byLow = *pData;
					pData ++;
					byHigh = *pData;
					pData += 2;
					YxValue = *pData;
					pData ++;
					RecSOE.wNo = MAKEWORD(byLow, byHigh) - ADDR_YX_LO;
					break;
			}
			if (YxValue)
				RecSOE.byValue = 0x81;	
			else
				RecSOE.byValue = 0x01;
			
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
		
    		WriteSSOE(m_wEqpID, 1, &RecSOE);
		}
	}
	return;	
}

void CGB101P::DoDoubleYk(void)
{
	DoYkReturn();
	return;	
}

void CGB101P::DoYkReturn(void)
{
	BYTE * pData = &(m_pReceive->Frame68.Data[m_dwasdu.Infooff]);
	WORD  DCO;	//遥控命令限定词
	WORD  YkNo; //遥控路号
	if(m_dwasdu.COT ==0xA)
		return;
	switch (m_pVIec101Cfg->byInfoAdr)
	{
		case 1:
			YkNo = pData[0];
			DCO = pData[1];
			break;
		case 2:
			YkNo = MAKEWORD(pData[0], pData[1]);
			DCO = pData[2];
			break;
		case 3:
			YkNo = MAKEWORD(pData[0], pData[1]);
			DCO = pData[3];
			break;
	}
	VYKInfo *pYKInfo;
	if (GetEqpFlag(EQPFLAG_YKReq))  return;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	
	//add yk return info proc
	
	switch (m_dwasdu.TypeID)
	{
	    case C_DC_NA:
			//if (m_pVIec101Cfg->byInfoAdr != 1)
				YkNo = YkNo - ADDR_YK_LO + 1;//yk no from 1 start
		    break;
	    case C_RC_NA:
			//if (m_pVIec101Cfg->byInfoAdr != 1)
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
			if (m_pReceive->Frame68.Data[m_byReasonShift] == COT_ACTCON)
			{
				pYKInfo->Head.byMsgID = MI_YKOPRATE;
			}
		    break;
	    default:
		    break;	
	}
	if (m_pReceive->Frame68.Data[m_byReasonShift] == COT_DEACTCON)
	{
		pYKInfo->Head.byMsgID = MI_YKCANCEL;	
	}
	switch (DCO & 0x03)
	{
	    case 0: 
	    case 3:
	    	pYKInfo->Info.byValue = 0x04;//DEF_YK_NULL_VALUE;//????
	    	break;
	    case 1: 
		    pYKInfo->Info.byValue = DEF_YK_OPEN_VALUE;
		    break; //分
	    case 2: 
		    pYKInfo->Info.byValue = DEF_YK_CLOSE_VALUE;
		    break; //合
	    default:
	    break;
	}
	if (m_pReceive->Frame68.Data[m_byReasonShift] & COT_PN_BIT)
	{
		pYKInfo->Info.byStatus = 5;
	}
	else
	{
		pYKInfo->Info.byStatus = 0;
	}
	CPPrimary::DoYKRet();
	return;	
	
}


BOOL CGB101P::DoYKReq(void)
{
	CPPrimary::DoYKReq();
	return TRUE;
}


void CGB101P::DoCommIdle()
{
	if ((m_wRetryCount	>=  m_pBaseCfg->wMaxRetryCount) && (m_wRetryCount != 65535))
	{
		Iec101PriInfo[m_wEqpNo].byStatus = S_INITOK; 
		Iec101PriInfo[m_wEqpNo].byLinkTimes ++;
	}
	DoSend();
	return;
}

void CGB101P::DoTimeOut(void)
{
	CPPrimary::DoTimeOut(); 
	m_dwTestTimerCount++;
	if (TimerOn(LINK_INTERVAL))
	{
		for (int i = 0; i < m_wEqpNum; i ++)
			if (Iec101PriInfo[i].byLinkTimes >= MAX_LINK_TIMES)
				Iec101PriInfo[i].byLinkTimes = MAX_LINK_TIMES - 1;
	}
	if((m_dwTestTimerCount % 10) == 0)    //10s没数据通信则发送测试帧
		ByTestFlag = 1;
}

BOOL CGB101P::SendBaseFrame(BYTE PRM, BYTE FCV, BYTE dwCode)
{
	WORD wLinkAddress;
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
	
	m_pSend = (VIec101Frame *)m_SendBuf.pBuf;
	
	m_pSend->Frame10.Start = 0x10;  
	m_pSend->Frame10.Control = GetCtrCode(PRM, FCV, dwCode);
	if (m_pVIec101Cfg->byLinkAdr == 1)
	{
		m_pSend->Frame10.Data[m_byLinkAdrShift] = GetAddress();
		m_pSend->Frame10.Data[m_byChksumShift] = (BYTE)ChkSum((BYTE *)&m_pSend->Frame10.Control,2);
		m_pSend->Frame10.Data[m_byStopShift] = 0x16;
		m_SendBuf.wWritePtr = 5;
		wLinkAddress = m_pSend->Frame10.Data[m_byLinkAdrShift];
	}
	else
	{
		m_pSend->Frame10.Data[m_byLinkAdrShift] = LOBYTE(GetAddress());
		m_pSend->Frame10.Data[m_byLinkAdrShift+1] = HIBYTE(GetAddress());
		m_pSend->Frame10.Data[m_byChksumShift] = (BYTE)ChkSum((BYTE *)&m_pSend->Frame10.Control,2+1);
		m_pSend->Frame10.Data[m_byStopShift] = 0x16;
		m_SendBuf.wWritePtr = 6;
		wLinkAddress = MAKEWORD(m_pSend->Frame10.Data[m_byLinkAdrShift],m_pSend->Frame10.Data[m_byLinkAdrShift+1]);
	}
	m_SendBuf.wReadPtr = 0;
	m_dwTestTimerCount = 0;
	WriteToComm(wLinkAddress);

	return TRUE;
}

//组织报文头
BOOL CGB101P::SendFrameHead(BYTE Style, BYTE Reason)
{
	m_SendBuf.wReadPtr = 0;
	m_pSend = (VIec101Frame *)m_SendBuf.pBuf;
	m_pSend->Frame68.Start1	= m_pSend->Frame68.Start2 = 0x68;
	if (m_pVIec101Cfg->byLinkAdr == 1)
	{
		m_pSend->Frame68.Data[m_byLinkAdrShift] = GetAddress();
	}
	else
	{
		m_pSend->Frame68.Data[m_byLinkAdrShift] = LOBYTE(GetAddress());
		m_pSend->Frame68.Data[m_byLinkAdrShift+1] = HIBYTE(GetAddress());
	}
	if (m_pVIec101Cfg->byCommAdr == 1)
	{
		m_pSend->Frame68.Data[m_byCommAdrShift] = GetAddress();	
	}
	else
	{
		m_pSend->Frame68.Data[m_byCommAdrShift] = LOBYTE(GetAddress());
		m_pSend->Frame68.Data[m_byCommAdrShift+1] = HIBYTE(GetAddress());
	}
	m_pSend->Frame68.Data[m_byTypeIDShift]	= Style;
	if (m_pVIec101Cfg->byCOT == 1)
		m_pSend->Frame68.Data[m_byReasonShift]	= Reason;
	else
	{
		m_pSend->Frame68.Data[m_byReasonShift]	= Reason;
		m_pSend->Frame68.Data[m_byReasonShift + 1]	= 0x00;//GetEqpAddr();
	}
	m_SendBuf.wWritePtr = (BYTE *)&m_pSend->Frame68.Data[m_byInfoShift] - (BYTE *)&m_pSend->Frame68.Start1;
	return TRUE;
}

//组织报文尾，并发送整帧报文
BOOL CGB101P::SendFrameTail(BYTE PRM, BYTE FCV, BYTE dwCode, BYTE Num)
{
	WORD wLinkAddress;
	m_pSend->Frame68.Length1 = m_pSend->Frame68.Length2 = m_SendBuf.wWritePtr - 4;
	m_pSend->Frame68.Control = GetCtrCode(PRM, FCV, dwCode);
	m_pSend->Frame68.Data[m_byQualifierShift] = Num;

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)ChkSum((BYTE *)&m_pSend->Frame68.Control, m_pSend->Frame68.Length1);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	if (m_pVIec101Cfg->byLinkAdr ==1)
		wLinkAddress = m_pSend->Frame68.Data[m_byLinkAdrShift];
	else
		wLinkAddress = MAKEWORD(m_pSend->Frame68.Data[m_byLinkAdrShift],m_pSend->Frame68.Data[m_byLinkAdrShift+1]);

	m_dwTestTimerCount = 0;
	WriteToComm(wLinkAddress);
	m_bPollMasterSendFlag = FALSE;
	return TRUE;
}

BOOL CGB101P::SendResetLink(BYTE PRM)
{ 
	return SendBaseFrame(PRM, FCV_OFF, MFC_RESETLINK); 
}

BOOL CGB101P::SendReqLink(void)
{ 
	return SendBaseFrame(PRM_MASTER, FCV_OFF, MFC_REQLINK); 
}

BOOL CGB101P::SendResetAck(void)
{ 
	BOOL ret;
	ret = SendBaseFrame(PRM_SLAVE, FCV_OFF, 0); 
	DisableRetry();
	return ret;
}

BOOL CGB101P::SendReqAck(void)
{ 
	BOOL ret;
	ret = SendBaseFrame(PRM_SLAVE, FCV_OFF, 0x0b); 
	DisableRetry();
	return ret;
}

void CGB101P::SendSetTime(BYTE SetMode)
{
	BYTE Qualifier = 1;
	VSysClock SysTime;
	WORD MSecond;
	
	GetSysClock((void *)&SysTime, SYSCLOCK);
	MSecond = SysTime.bySecond * 1000 + SysTime.wMSecond;
	
	SendFrameHead(C_CS_NA, COT_ACT);
	if (SetMode == STM_BROADCAST)//broadcast set time
	{
		if (m_pVIec101Cfg->byLinkAdr == 1)
		{
			m_pSend->Frame68.Data[m_byLinkAdrShift] = GetBroadcastAddr();
		}
		else
		{
			m_pSend->Frame68.Data[m_byLinkAdrShift] = LOBYTE(GetBroadcastAddr());
			m_pSend->Frame68.Data[m_byLinkAdrShift+1] = HIBYTE(GetBroadcastAddr());
		}
		if (m_pVIec101Cfg->byCommAdr == 1)
		{
			m_pSend->Frame68.Data[m_byCommAdrShift] = GetBroadcastAddr();	
		}
		else
		{
			m_pSend->Frame68.Data[m_byCommAdrShift] = LOBYTE(GetBroadcastAddr());
			m_pSend->Frame68.Data[m_byCommAdrShift+1] = HIBYTE(GetBroadcastAddr());
		}
	}
	else
	{
		if (m_pVIec101Cfg->byLinkAdr == 1)
		{
			m_pSend->Frame68.Data[m_byLinkAdrShift] = LOBYTE(GetAddress());
		}
		else
		{
			m_pSend->Frame68.Data[m_byLinkAdrShift] = LOBYTE(GetAddress());
			m_pSend->Frame68.Data[m_byLinkAdrShift+1] = HIBYTE(GetAddress());
		}
		if (m_pVIec101Cfg->byCommAdr == 1)
		{
			m_pSend->Frame68.Data[m_byCommAdrShift] = GetAddress();	
		}
		else
		{
			m_pSend->Frame68.Data[m_byCommAdrShift] = LOBYTE(GetAddress());
			m_pSend->Frame68.Data[m_byCommAdrShift+1] = HIBYTE(GetAddress());
		}
	}
	
	if (m_pVIec101Cfg->byInfoAdr == 1)
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址
	else
	{
		if (m_pVIec101Cfg->byInfoAdr == 2)
		{
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
		}
		else
		{
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
		}
	}
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(MSecond); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(MSecond);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byHour;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (SysTime.byDay & 0x1f) | ((SysTime.byWeek << 5) & 0xe0); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMonth;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.wYear - 2000; 
	 
	SendFrameTail(PRM_MASTER, FCV_ON, MFC_TRANSDATA, Qualifier);
	
	return;	
}

void CGB101P::SendReqClass1(void)
{ 
	SendBaseFrame(PRM_MASTER, FCV_ON, MFC_REQCLASS1); 
}

void CGB101P::SendReqClass2(void)
{ 
	SendBaseFrame(PRM_MASTER, FCV_ON, MFC_REQCLASS2); 
}

void CGB101P::SendReqTest(void)
{ 
	SendFrameHead(C_TS_NA, COT_ACT);
	switch (m_pVIec101Cfg->byInfoAdr)
	{
		case 1:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			break;
		case 2:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			break;
		case 3:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			break;
		default:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			break;
	}
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0xAA;	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0x55;
	SendFrameTail(PRM_MASTER, FCV_ON, MFC_TRANSDATA, 0x01); 
	Iec101PriInfo[m_wEqpNo].bReqAllData = TRUE;
	return; 
}


void CGB101P::SendReqAllData(void)
{ 
	SendFrameHead(C_IC_NA, COT_ACT);
	switch (m_pVIec101Cfg->byInfoAdr)
	{
		case 1:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			break;
		case 2:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			break;
		case 3:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			break;
		default:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			break;
	}
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0x14;
	SendFrameTail(PRM_MASTER, FCV_ON, MFC_TRANSDATA, 0x01); 
	Iec101PriInfo[m_wEqpNo].bReqAllData = TRUE;
	return; 
}


void CGB101P::SendReqAllDd(void)
{ 
	SendFrameHead(C_CI_NA, COT_ACT);
	switch (m_pVIec101Cfg->byInfoAdr)
	{
		case 1:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			break;
		case 2:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			break;
		case 3:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			break;
		default:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			break;
	}
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0x05;
	SendFrameTail(PRM_MASTER, FCV_ON, MFC_TRANSDATA, 0x01); 
	return; 
}

BOOL CGB101P::SendYkCommand(void)
{
	BYTE Reason;
	WORD YkNo;
	BYTE DCO = 0;
	BYTE Qualifier = 1;
	
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

	//if (m_pVIec101Cfg->byInfoAdr == 1)
	//	YkNo = pYKInfo->Info.wID;
	//else
	YkNo = pYKInfo->Info.wID + ADDR_YK_LO - 1;
	
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
	
	SendFrameHead(C_DC_NA, Reason);
	
	write_infoadd(YkNo);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = DCO; 
	 
	SendFrameTail(PRM_MASTER, FCV_ON, MFC_TRANSDATA, Qualifier);
	
	return TRUE;		
}

BOOL CGB101P::ReqUrgency(void)
{
	if ((!m_guiyuepara.mode) && (Iec101PriInfo[m_wEqpNo].byStatus == S_LINKOK) && SwitchClearEqpFlag(ACD_ON))
	{
		SendReqClass1();
		return TRUE;	
	}
	
	if ((Iec101PriInfo[m_wEqpNo].byStatus == S_LINKOK) && SwitchClearEqpFlag(EQPFLAG_YKReq))
	{
		if (SendYkCommand()) 
			return TRUE;
	}	

	return FALSE;	
}

BOOL CGB101P::ReqCyc(void)
{
	if (Iec101PriInfo[m_wEqpNo].byStatus == S_LINKOK)
	{
		if ((m_pBaseCfg->wCmdControl & CLOCKENABLE) && (m_pBaseCfg->wCmdControl & BROADCASTTIME))
		{
			if (CheckClearTaskFlag(TASKFLAG_CLOCK))
			{
				SendSetTime(STM_BROADCAST);
				return TRUE;
			}
		}
		else
		{
			if (m_pBaseCfg->wCmdControl & CLOCKENABLE)
				if (CheckClearEqpFlag(EQPFLAG_CLOCK))
				{
					SendSetTime(STM_SINGLE);
					return TRUE;
				}
		}
    	if (CheckClearEqpFlag(EQPFLAG_YC)) 
  		{
  			CheckClearEqpFlag(EQPFLAG_YX);//must at the same time with EQPFLAG_YC
  			SendReqAllData();
  			return TRUE;
  		}
  
  		if (CheckClearEqpFlag(EQPFLAG_DD)) 
  		{
  			SendReqAllDd();
  			return TRUE;
  		}
	}
	return FALSE;
}

BOOL CGB101P::ReqNormal(void)
{
	
	if (Iec101PriInfo[m_wEqpNo].byStatus == S_LINKOK)
	{
		if(!m_guiyuepara.mode)
		{
			SendReqClass2();
			return TRUE;
		}
	}
	if ((Iec101PriInfo[m_wEqpNo].byStatus == S_INITOK) || (Iec101PriInfo[m_wEqpNo].byStatus == S_REQLINK))
	{
		if (Iec101PriInfo[m_wEqpNo].byLinkTimes < MAX_LINK_TIMES)
		{
			SendReqLink();
			Iec101PriInfo[m_wEqpNo].byStatus = S_REQLINK;
		}
		else
		{
			DisableRetry();
			DWORD dwCommCtrlTimer = 1;//20 ms
			commCtrl(m_wCommID, CCC_EVENT_CTRL|m_wCommCtrl, (BYTE*)&dwCommCtrlTimer);
		}
		return TRUE;
	}
	if (Iec101PriInfo[m_wEqpNo].byStatus == S_RESETLINK)
	{
		Iec101PriInfo[m_wEqpNo].byStatus = S_NULL;
		SendResetLink(PRM_MASTER);
		return TRUE;	
	}

	if(Iec101PriInfo[m_wEqpNo].byStatus == S_RECREQ)
	{
		SendReqAck();
		return TRUE;
	}
	
	if(m_guiyuepara.mode && ByTestFlag)
	{
		ByTestFlag = 0;
		SendBaseFrame(PRM_MASTER,FCV_ON,0x02);
		return TRUE;
	}
	DisableRetry();
	return FALSE;	
}


BYTE CGB101P::ChkSum(BYTE *buf, WORD len)
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

DWORD CGB101P::GetAddress(void)
{
	return GetEqpAddr();
}

BYTE CGB101P::GetCtrCode(BYTE PRM, BYTE FCV, BYTE dwCode)
{
	BYTE CodeTmp = Iec101PriInfo[m_wEqpNo].byFCB;
	
	CodeTmp += dwCode;
	if (PRM)
		CodeTmp |= 0x40;
	if (FCV)
	{
		CodeTmp ^= 0x20;
		CodeTmp |= 0x10;
		Iec101PriInfo[m_wEqpNo].byFCB ^= 0x20;
	}
	else
		CodeTmp &= ~0x30; 
	return CodeTmp;
}
void CGB101P::getasdu(void)
{	BYTE off=0;
	if(m_guiyuepara.linkaddrlen==1)
	{
		m_dwasdu.LinkAddr=m_pReceive->Frame68.Data[off++];
	}
	if(m_guiyuepara.linkaddrlen==2)
	{
		m_dwasdu.LinkAddr=MAKEWORD(m_pReceive->Frame68.Data[off],m_pReceive->Frame68.Data[off+1]);
			off+=2;
	}
	if(m_guiyuepara.typeidlen==1)
	{
		m_dwasdu.TypeID=m_pReceive->Frame68.Data[off++];
	}
	if(m_guiyuepara.typeidlen==2)
	{
		m_dwasdu.TypeID=MAKEWORD(m_pReceive->Frame68.Data[off],m_pReceive->Frame68.Data[off+1]);
			off+=2;
	}
	if(m_guiyuepara.VSQlen==1)
	{
		m_dwasdu.VSQ=m_pReceive->Frame68.Data[off++];
	}
	if(m_guiyuepara.VSQlen==2)
	{
		m_dwasdu.VSQ=MAKEWORD(m_pReceive->Frame68.Data[off],m_pReceive->Frame68.Data[off+1]);
			off+=2;
	}
	if(m_guiyuepara.COTlen==1)
	{
		m_dwasdu.COT=m_pReceive->Frame68.Data[off++];
	}
	if(m_guiyuepara.COTlen==2)
	{
		m_dwasdu.COT=m_pReceive->Frame68.Data[off++];
			off++;
	}
	if(m_guiyuepara.conaddrlen==1)
	{
		m_dwasdu.Address=m_pReceive->Frame68.Data[off++];
	}
	if(m_guiyuepara.conaddrlen==2)
	{
		m_dwasdu.Address=MAKEWORD(m_pReceive->Frame68.Data[off],m_pReceive->Frame68.Data[off+1]);
			off+=2;
	}
			m_dwasdu.Infooff=off;
	if(m_guiyuepara.infoaddlen==1)
	{
		m_dwasdu.Info=m_pReceive->Frame68.Data[off++];
	}
	if(m_guiyuepara.infoaddlen==2)
	{
		m_dwasdu.Info=MAKEWORD(m_pReceive->Frame68.Data[off],m_pReceive->Frame68.Data[off+1]);
			off+=2;
	}
	if(m_guiyuepara.infoaddlen==3)
	{
		m_dwasdu.Info=MAKEWORD(m_pReceive->Frame68.Data[off+1], m_pReceive->Frame68.Data[off+2]);
			m_dwasdu.Info<<=8;
			m_dwasdu.Info|=m_pReceive->Frame68.Data[off];
			off+=3;
	}
}
void CGB101P::write_linkaddr(int  data)
{
	m_SendBuf.wWritePtr=5;
		for(BYTE i=0;i<m_guiyuepara.linkaddrlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}
void CGB101P::write_typeid(int  data)
{
	m_SendBuf.wWritePtr=5+m_guiyuepara.linkaddrlen;
		for(BYTE i=0;i<m_guiyuepara.typeidlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}
void CGB101P::write_VSQ(int  data)
{
		for(BYTE i=0;i<m_guiyuepara.VSQlen;i++)
			{
				m_SendBuf.pBuf[ i+5+m_guiyuepara.linkaddrlen+m_guiyuepara.typeidlen ]=(data>>(i*8))&0xff;
		}

}
void CGB101P::write_COT(int  data)
{
	m_SendBuf.wWritePtr=5+m_guiyuepara.linkaddrlen+m_guiyuepara.typeidlen+m_guiyuepara.VSQlen;
		for(BYTE i=0;i<m_guiyuepara.COTlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}
void CGB101P::write_conaddr(int  data)
{
	m_SendBuf.wWritePtr=5+m_guiyuepara.linkaddrlen+m_guiyuepara.typeidlen+m_guiyuepara.VSQlen+m_guiyuepara.COTlen;
		for(BYTE i=0;i<m_guiyuepara.conaddrlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}
void CGB101P::write_infoadd(int  data)
{
		for(BYTE i=0;i<m_guiyuepara.infoaddlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}

void CGB101P::CheckCfg()
{
	//char tt[80];
	if((m_guiyuepara.linkaddrlen<1)||(m_guiyuepara.linkaddrlen>2))
		{
		m_guiyuepara.linkaddrlen=2;
		m_guiyuepara.typeidlen=1;
		m_guiyuepara.conaddrlen=2;
		m_guiyuepara.VSQlen=1;
		m_guiyuepara.COTlen=2;
		m_guiyuepara.infoaddlen=2;
		m_guiyuepara.baseyear=2000;
		m_guiyuepara.mode=1;
		m_guiyuepara.yxtype=3;
		m_guiyuepara.yctype=9;
		m_guiyuepara.ddtype=15;
	}
}
void CGB101P::SetDefCfg()
{
		m_guiyuepara.linkaddrlen=2;
		m_guiyuepara.typeidlen=1;
		m_guiyuepara.conaddrlen=2;
		m_guiyuepara.VSQlen=1;
		m_guiyuepara.COTlen=2;
		m_guiyuepara.infoaddlen=2;
		m_guiyuepara.baseyear=2000;
		m_guiyuepara.mode=1;
		m_guiyuepara.yxtype=3;
		m_guiyuepara.yctype=9;
		m_guiyuepara.ddtype=15;

}
WORD  CGB101P::dealdyxnum(WORD no)
{
if(no>m_pEqpInfo[m_wEqpNo].wDYXNum)
	return no-m_pEqpInfo[m_wEqpNo].wDYXNum;
		return no;
}
 BYTE CGB101P::QDS(BYTE data)
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
  BYTE CGB101P::SIQ(BYTE data)
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
void  CGB101P::DIQ(BYTE data,BYTE *data1,BYTE *data2)
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
int CGB101P::SendGetLineLossID(char*pbuf)//获取华彩线损模块ID
{
	int state = ERROR;
#if (DEV_SP == DEV_SP_WDG)
	WORD wLinkAddress;
	int waitcnt = 0;
	BYTE TLVS[]={0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x13,0x00,0xDF,0x16};
	m_SendBuf.wWritePtr = m_SendBuf.wReadPtr = 0;

	if((m_wCommID == (COMM_SERIAL_START_NO+2)) && (GprsChan.GPRS_COMNO != 2))
	{
		memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], TLVS, sizeof(TLVS));
		m_SendBuf.wWritePtr += sizeof(TLVS);
		wLinkAddress = m_dwasdu.LinkAddr;
		WriteToComm(wLinkAddress);
		DisableRetry();
	}
	while(1)
	{
		thSleep(20);
		if(ReadParaFlag ==1)
		{
			state = OK;
			memcpy(pbuf,TempID,6);
			break;
		}
		else
		{
			waitcnt++;
			if(waitcnt>300)
				break;
		}
	}
	return state;
#else
	return state;
#endif
	
}

#endif

