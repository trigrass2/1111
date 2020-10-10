/*------------------------------------------------------------------------
 Module:			cdt92.cpp
 Author:			
 Project:        	
 State:				
 Creation Date:		
 Description:   	从站CDT92 规约函数定义
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/
#include "syscfg.h"
		
#ifdef INCLUDE_CDT92_S


#include "cdt92s.h"

//入口函数                            
extern "C" void cdt92s(WORD wTaskID)
{
	CSCDT92 *pSCDT92 = new CSCDT92();		
	
	if (pSCDT92->Init(wTaskID) != TRUE)
	{
		pSCDT92->ProtocolExit();
	}
    pSCDT92->Run();	
}

extern "C" void cdt92s_load(int tid)
{
    char tname[30];
	
	sprintf(tname,"tCdts%d",tid);		
	SetupCommonTask(tid, tname, COMMONPRIORITY, COMMONSTACKSIZE, (ENTRYPTR)cdt92s, COMMONQLEN);
}

////构造函数
CSCDT92::CSCDT92(void)
{
	SendSoeCount = 0; 
	SOEBufSendLen = 0;	
}

//实现CDT92规约的初始化 
BOOL CSCDT92::Init(WORD wTaskID)
{
	bool rc;
	int i;
	
	rc = CCDTSec::Init(wTaskID, 1, NULL, sizeof(VCdt92Cfg));
	if (!rc)
	{
		return FALSE;
	}

	for (i = 0; i < m_wEqpNum; i++)
		m_pEqpInfo[i].pProtocolData = new VCtrlInfo;
	
	InitSendList();
	SetSendList(SendList);	
	DoSend();

    return TRUE;
}


void CSCDT92::InitSendList(void)
{
	strcpy(SendList, "ABCEABCEABCD");
}


//获取变位遥信个数
WORD CSCDT92::GetCosNum(WORD EqpID)
{
	if (TestFlag(m_wTaskID, EqpID, SCOSUFLAG))
		return 1;	
	else 
		return 0;
}


void CSCDT92::OnWriteCommEvent(void)
{
	//DoSend();
}


BOOL CSCDT92::DoYKRet(void)
{
	CPSecondary::DoYKRet();
	return TRUE;
}

void CSCDT92::DoCommSendIdle(void)
{
	DWORD event_time = 0;
	commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_OFF, &event_time); 
	m_SendBuf.wReadPtr = 0;
	m_SendBuf.wWritePtr = 0;

	if (GetSendEndFlag())
	{
		FindSendFrameType();
	}

	switch (GetSendFrameType())
	{
		case FM_YC_A:	
			SendYC_A();  
			break;  
		case FM_YC_B:	
			SendYC_B();  
			break;
		case FM_YC_C:	
			SendYC_C();  
			break;
						
		case FM_YX:	
			SendYX();   
			break;
						
		case FM_DD:	
			SendDD();   
			break;
						
		case FM_SOE: 
			SendSoe();  
			break;
		default: 
			SendYC_A();  
			break;  
	}

	WriteToComm(GetEqpAddr());
	event_time=m_pBaseCfg->Timer.wScanData2/20;
	commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
}


BYTE CSCDT92::GetInsertFrame(BYTE LastFrameType)
{
	if (m_pVCdt92Cfg->bySoeSendMode == SOE_CONTINUE)
	{
		if (GetSoeFlag() && !(GetCosFlag()||GetEqpFlag(EQPFLAG_YKRet)))
			return FM_SOE;
		else return 0;
	}
			
	if (GetSoeFlag() || (m_pVCdt92Cfg->bySoeSendMode == SOE_BREAK))
	{
		BYTE CycleEnd;
		CycleEnd = NO;
		switch (LastFrameType)
		{
			case FM_YC_A:
				if (GetYCNum() <= 32)
					CycleEnd = YES;
				break;
			case FM_YC_B: 
				if (GetYCNum() <= 64)
					CycleEnd = YES;
				break;
			case FM_YC_C: 
				if (m_byYCInfoSendNum == 0)
					CycleEnd = YES;
				break;
			default:
				if (GetYCNum() == 0)
					CycleEnd = YES;
				break;
		}
		if ((CycleEnd == YES) && !(GetCosFlag()||GetEqpFlag(EQPFLAG_YKRet)))
			return FM_SOE;
	}
	return 0;
}

BOOL CSCDT92::SendYC_A(void)
{
	if (!GetHaveSendSyncFlag())
		return SendCtrlWord(CONTROL_BYTE, SEND_YC_A, GetYCFrameLen(FM_YC_A));

	return SendYC(FM_YC_A);
}

BOOL CSCDT92::SendYC_B(void)
{
	if (!GetHaveSendSyncFlag())
		return SendCtrlWord(CONTROL_BYTE, SEND_YC_B, GetYCFrameLen(FM_YC_B));

	return SendYC(FM_YC_B);
}

BOOL CSCDT92::SendYC_C(void)
{
	if (!GetHaveSendSyncFlag())
		return SendCtrlWord(CONTROL_BYTE, SEND_YC_C, GetYCFrameLen(FM_YC_C));

	return SendYC(FM_YC_C);
}

BOOL CSCDT92::SendYC(BYTE m_bySendFrameType)
{
	WORD BeginYCNo;
	WORD YCSendNum;
	WORD YCNo,YCValue;
	
   	if (SendInsertInfo())
		return TRUE; 
	
	switch (m_bySendFrameType)
	{
		case FM_YC_A: 
			BeginYCNo = 0;					
			break;
		case FM_YC_B: 
			BeginYCNo = 32;					
			break;
		case FM_YC_C: 
			BeginYCNo = 64 + m_byYCInfoSendNum * 32;	
			break;
	}
	YCNo = BeginYCNo + GetHaveSendNum() * 2;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = YCNo / 2;
	
	for (YCSendNum = 0; YCSendNum < 2; YCSendNum++, YCNo++)
	{
		YCValue = GetCdt92YC(YCNo);
		
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCValue);
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCValue);
	}
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr ] = GetSendBCH();
	m_SendBuf.wWritePtr++;

	IncHaveSendNum();

	return TRUE;
}

WORD CSCDT92::GetCdt92YC(WORD YCNo)
{
	WORD YCValue;
	WORD ReadYCNum = 1;
	ReadRangeYC(m_wEqpID, YCNo, ReadYCNum, 2, (short *)&YCValue);
	if (YCValue & 0x8000)
 	{
		if (YCValue < (WORD)0xf800)
			YCValue = 0xc800;
    	else 
			YCValue &= 0xFFF;	 
	}
 	else
	{
		if (YCValue > (WORD)0x7ff)
			YCValue = 0xc7ff;
	}
	return YCValue;
}

BOOL CSCDT92::SendYX(void)
{
	WORD InfoNum, YXLen;
	//WORD YXSendNum, YXByteNo;
	DWORD YXValue ;
	if (!GetYCNum())
	{
		InfoNum = GetInsertInfoLen();
		YXLen = GetYXFrameLen();
		if (YXLen < InfoNum)
			YXLen = InfoNum;
		if (YXLen > 16)
			YXLen = 16;
		if (!GetHaveSendSyncFlag())
			return SendCtrlWord(CONTROL_BYTE, SEND_YX, 3 );
	
		if (SendInsertInfo())	
			return TRUE; 
	}
	else
	{
		if (!GetHaveSendSyncFlag())
			return SendCtrlWord(CONTROL_BYTE, SEND_YX, 3);
	}

	//YXByteNo = GetHaveSendNum() * 4;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0xF0 ;//+ GetHaveSendNum();
#if 0
//	for (YXSendNum = 0; YXSendNum < 4; YXSendNum++, YXByteNo++)
//	{
	YXState = ReadYXByte(YXByteNo);
	memcpy(&m_SendBuf.pBuf[ m_SendBuf.wWritePtr ] ,&YXState,sizeof(YXState));
	m_SendBuf.wWritePtr+=4;
//	}
#endif
//	for (YXSendNum = 0; YXSendNum < 4; YXSendNum++, YXByteNo++)
//	{
	  	YXValue = ReadYXByte();
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(LOWORD(YXValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(LOWORD(YXValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(HIWORD(YXValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(HIWORD(YXValue));
//	}
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr ] = GetSendBCH();
	m_SendBuf.wWritePtr++;

	IncHaveSendNum();
	return TRUE;
}

BOOL CSCDT92::SendDD(void)
{

	WORD DDNo, InfoNum, DDLen;
	WORD ReadDDNum = 1;
	DWORD DDValue;

	if (0)//允许在电度帧中插发遥信变位和遥控返校
	{
		InfoNum = GetInsertInfoLen();
		DDLen = GetDDFrameLen();
		if (DDLen < InfoNum)
			DDLen = InfoNum;
		if (DDLen > 16)
			DDLen = 16;
		if (!GetHaveSendSyncFlag())
			return SendCtrlWord(CONTROL_BYTE, SEND_DD, DDLen );
	
		if (SendInsertInfo())	
			return TRUE; 
	}
	else
	{
		if (!GetHaveSendSyncFlag())
  			return SendCtrlWord(CONTROL_BYTE, SEND_DD, GetDDFrameLen());
	}

	DDNo = m_byDDInfoSendNum * 16 + GetHaveSendNum();
	
	ReadRangeDD(m_wEqpID, DDNo, ReadDDNum, 4, (long *)&DDValue);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0xA0+DDNo;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(LOWORD(DDValue));
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(LOWORD(DDValue));
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(HIWORD(DDValue));
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(HIWORD(DDValue));
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr  ] =   GetSendBCH();
	m_SendBuf.wWritePtr++;

	IncHaveSendNum();
	return TRUE;
}


BOOL CSCDT92::SendInsertInfo(void)
{
	if (GetNotSendNum() < 3)
		return FALSE;

	if (GetCosFlag()) 
	{ 
		if(SendCos()) 
		{	
			AddHaveSendNum(3);
			return TRUE;
		}
	}	
	if (CheckClearEqpFlag(EQPFLAG_YKRet))
	{
		if (SendYKReturn()) 
		{	
			AddHaveSendNum(3);
			return TRUE;
		}
	}
		
	return FALSE;
}


BOOL CSCDT92::SendCos(void)
{
	WORD StartYXByteNo;
	BYTE YXBuf[4];
	WORD WritePtr;
	int i;	
	
	if (FindCOSByte(&StartYXByteNo, YXBuf)==FALSE )
		return FALSE;

	WritePtr = m_SendBuf.wWritePtr;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0xF0 + StartYXByteNo / 4; 
	for (i = 0; i < 4; i++) 
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = YXBuf[i];
	}
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr  ] = GetSendBCH();
	m_SendBuf.wWritePtr++;

	for (i = 0; i < 2 * 6; i++) 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_SendBuf.pBuf[WritePtr++];

	return TRUE;
}



BOOL CSCDT92::SendSoe(void)
{
	int i;
	
	if (m_pVCdt92Cfg->bySoeSendMode == SOE_CONTINUE)//(m_bySoeSendMode == SOE_CONTINUE)
	{		
		SendSoeCount = 0;
		if (!EncodeSoe())
			return FALSE;
		SendCtrlWord(CONTROL_BYTE, SEND_SOE, 6);
		for (i = 0; i<3; i++)
		{
			memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], &SoeFrame, 12);//sizeof(SoeFrame));
			m_SendBuf.wWritePtr += sizeof(SoeFrame);
		}
		SetSendEndFlag();
		return TRUE;
	}
	else
	{
		if(SendSoeCount == 0)
		{
			SOEBufSendLen = 0;
			for (i = 0; i < MAXSOENUM; i++)
			{
				if (EncodeSoe())
				{
					memcpy(&SOEBuf[SOEBufSendLen], &SoeFrame, 12);//sizeof(SoeFrame));
					SOEBufSendLen += 12;//sizeof(SoeFrame);
				}
				else
					break;
			}
			if (SOEBufSendLen)
				SendSoeCount = 3;
			else
				return FALSE;
		}
		if (SendSoeCount > 0)
		{
			SendSoeCount--;
			SendCtrlWord(CONTROL_BYTE, SEND_SOE, SOEBufSendLen / 6);
			memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], &SOEBuf[0], SOEBufSendLen);
			m_SendBuf.wWritePtr += SOEBufSendLen;
			SetSendEndFlag();
		}
	}
	return TRUE;
}


BOOL CSCDT92::SendYKReturn(void)
{
	WORD WritePtr;
	int i;	
	VYKInfo *pYKInfo;
	VCtrlInfo *pCtrlInfo;
	
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
    pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;

	ClearEqpFlag(EQPFLAG_YKRet);
	
	if(pCtrlInfo->DCO != YK_SELECT)
		return FALSE;

	WritePtr = m_SendBuf.wWritePtr;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0xE1;
	if (m_pVCdt92Cfg->byYKNoMode == YKNO_BIN)
	{
		for (i = 0; i < 2; i++)
		{
			switch (pYKInfo->Info.byStatus)
			{
				case 1:
				case 2:
				case 3: 
					m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0xFF; 
					break;
				case 0:
					if (!(m_pBaseCfg->wCmdControl&YKOPENABLED))
					{
						m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0xFF;
						break;
					}
					switch (pYKInfo->Info.byValue)
					{
						case 0x06:  
							m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x33;  
							break;
						case 0x05: 
							m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] = 0xCC;  
							break;
						default: 
							m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] = 0xFF;  
							break;
					}
					break;
				default:
					m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0xFF; 
					break;
			}
			
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = pYKInfo->Info.wID;
		}
	}
	else
	{
		switch (pYKInfo->Info.byStatus)
		{
			case 1:
			case 2:
			case 3: 
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0xFF; 
				break;
			case 0: 
				if (!(m_pBaseCfg->wCmdControl&YKOPENABLED))
				{
					m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0xFF;
					break;
				}
				switch (pYKInfo->Info.byValue)
				{
					case 0x06:  
						m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x33;  
						break;
					case 0x05: 
						m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] = 0xCC;  
						break;
					default: 
						m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] = 0xFF;  
						break;
				}
				break;
			default:
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0xFF; 
				break;
		}
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] = 0xff;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] = LOBYTE(WORD2BCD(pYKInfo->Info.wID));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] = HIBYTE(WORD2BCD(pYKInfo->Info.wID));
	}
	m_SendBuf.pBuf[m_SendBuf.wWritePtr] = GetSendBCH();
	m_SendBuf.wWritePtr++;

	for (i = 0; i < 2 * 6; i++) //再发2遍
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_SendBuf.pBuf[WritePtr++];

	return TRUE;
}


WORD CSCDT92::GetInsertInfoLen(void)
{
	WORD InfoNum = 0;
	
	if (GetEqpFlag(EQPFLAG_YKRet))
		InfoNum += 3; 

	if (GetCosFlag())	
		InfoNum += 3;
	return InfoNum;
}


WORD CSCDT92::EncodeSoe(void)
{
	WORD RecSoeNum;
	WORD ReadNum = 1;
	BYTE YXValue;
	VSysClock SysTime;
	VDBSOE SOEInfo;
	WORD MSecond;
	BYTE Second;
	
	if (CheckClearTaskFlag(TASKFLAG_SSOEUFLAG))
	{        
		if (CheckClearEqpFlag(EQPFLAG_SSOEUFLAG))
  		{  
			RecSoeNum = ReadSSOE(m_wTaskID, m_wEqpID, ReadNum, sizeof(VDBSOE), &SOEInfo);
			if (RecSoeNum == 0)
				return 0;
			//ReadPtrIncrease(m_wTaskID, m_wEqpID, RecSoeNum, SSOEUFLAG);
			YXValue = SOEInfo.byValue & 0x80; 
			MSecond = SOEInfo.Time.wMSecond % 1000;
			Second = SOEInfo.Time.wMSecond / 1000;
			SystemClock(SOEInfo.Time.dwMinute, &SysTime);
	
			SoeFrame.Code1=0x80;
			SoeFrame.MSecondLo = LOBYTE(MSecond);
			SoeFrame.MSecondHi = HIBYTE(MSecond);
			SoeFrame.Second = Second;
			SoeFrame.Minute = SysTime.byMinute;
			SoeFrame.Crc1 = GetBCH(&SoeFrame.Code1);
	
			SoeFrame.Code2 = 0x81;
			SoeFrame.Hour = SysTime.byHour;
			SoeFrame.Day = SysTime.byDay;
			SoeFrame.YXNoLo = LOBYTE(SOEInfo.wNo);
			SoeFrame.YXNoHi = HIBYTE(SOEInfo.wNo);
			if (YXValue)
				SoeFrame.YXNoHi |= 0x80;
	
			SoeFrame.Crc2=GetBCH(&SoeFrame.Code2);
			
			m_pEqpExtInfo[m_wEqpNo].wSendSSOENum = RecSoeNum;                  
			SetTaskFlag(TASKFLAG_SENDSSOE);                              
			SetEqpFlag(EQPFLAG_SENDSSOE);                                
			ClearUFlag(m_wEqpNo); 
	
			return 1;
		}
	}
	return 0;
};



BOOL CSCDT92::DoReceive(void)
{
	
    if (SearchFrame()!=TRUE)
		return FALSE;

	if (GetRecFrameType() == SYNCWORD) 
		return TRUE; 
	
	if (GetRecFrameType() == WORD_NULL) 
		return TRUE; 

	
	m_pCDTReceiveFrame = (VCDTReceiveFrame *)m_RecFrame.pBuf;

	switch (GetRecFrameType())
	{
		case CODE_YK_SELECT: 
			ReceiveYKSelect(); 
			break;
		case CODE_YK_EXE: 
			ReceiveYKExe(); 
			break;
		case CODE_YK_CANCEL: 
			ReceiveYKCancel(); 
			break;
		case CODE_YT_SELECT: 
			ReceiveYTSelect(); 
			break;
		case CODE_YT_EXE: 
			ReceiveYTExe(); 
			break;
		case CODE_YT_CANCEL: 
			ReceiveYTCancel(); 
			break;
		case CODE_TIMESET: 
			ReceiveSetTime(); 
			break;
		default: 
			break;
	}
	return TRUE;
}

BOOL CSCDT92::ReceiveYKSelect(void)
{
	if (GetRecWordType() != WORD_INFO)
		return TRUE;

	if (m_pCDTReceiveFrame->FunctionCode != FUN_YK_SELECT)
		return FALSE;

	NotRecInfoFrame();
	return DoYKYT(YK_SELECT);
}


BOOL CSCDT92::ReceiveYKExe(void)
{
	if (!(m_pBaseCfg->wCmdControl&YKOPENABLED))
	{
		return FALSE;
	}
	if (GetRecWordType() != WORD_INFO)
		return TRUE;

	if (m_pCDTReceiveFrame->FunctionCode != FUN_YK_EXE)
		return FALSE;

	NotRecInfoFrame();
	return DoYKYT(YK_EXE);
}

BOOL CSCDT92::ReceiveYKCancel(void)
{
	if (GetRecWordType() != WORD_INFO)
		return TRUE;
	
	if (m_pCDTReceiveFrame->FunctionCode != FUN_YK_CANCEL)
		return FALSE;
	
	NotRecInfoFrame();
	return DoYKYT(YK_CANCEL);
}

BOOL CSCDT92::ReceiveYTSelect(void)
{
	if (GetRecWordType() != WORD_INFO)
		return TRUE;

	if (m_pCDTReceiveFrame->FunctionCode != FUN_YT_SELECT)
		return FALSE;
	
	NotRecInfoFrame();
	return DoYKYT(YK_SELECT);
}

BOOL CSCDT92::ReceiveYTExe(void)
{
	if (GetRecWordType() != WORD_INFO)
		return TRUE;
	
	if (m_pCDTReceiveFrame->FunctionCode != FUN_YT_EXE)
		return FALSE;
	
	NotRecInfoFrame();
	return DoYKYT(YK_EXE);
}

BOOL CSCDT92::ReceiveYTCancel(void)
{
	if (GetRecWordType() != WORD_INFO)
		return TRUE;
	
	if (m_pCDTReceiveFrame->FunctionCode != FUN_YT_CANCEL)
		return FALSE;
	
	NotRecInfoFrame();
	return DoYKYT(YK_CANCEL);
}

BOOL CSCDT92::ReceiveSetTime(void)
{
	VSysClock SysTime;
	
	if (GetRecWordType() != WORD_INFO)
		return FALSE;
	  	
	if (!(m_pBaseCfg->wCmdControl&CLOCKENABLE))
	{
		return FALSE;
	}
	switch (m_pCDTReceiveFrame->FunctionCode)
	{
    case 0xee: 
 		 GetSysClock((void *)&SysTime, SYSCLOCK);
		 SysTime.wMSecond = MAKEWORD(m_pCDTReceiveFrame->InfoByte1, m_pCDTReceiveFrame->InfoByte2);
		 SysTime.bySecond = m_pCDTReceiveFrame->InfoByte3;
		 SysTime.byMinute = m_pCDTReceiveFrame->InfoByte4;
 		 SetSysClock((void *)&SysTime, SYSCLOCK);
		 break;
	
	case 0xef: 
		 NotRecInfoFrame();
 		 GetSysClock((void *)&SysTime, SYSCLOCK);
		 SysTime.byHour = m_pCDTReceiveFrame->InfoByte1;
		 SysTime.byDay = m_pCDTReceiveFrame->InfoByte2;
		 SysTime.byMonth = m_pCDTReceiveFrame->InfoByte3;
		 SysTime.wYear  = m_pCDTReceiveFrame->InfoByte4%100 + 2000;
 		 SetSysClock((void *)&SysTime, SYSCLOCK);
		 break;
	}
	
	return TRUE;
}

BOOL CSCDT92::DoYKYT(BYTE Command)
{
	WORD wYKNo;
	VYKInfo *pYKInfo;
	VCtrlInfo *pCtrlInfo;

	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
    pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	
	if (GetRecWordType() != WORD_INFO)
		return TRUE;
		
	
	if (m_pVCdt92Cfg->byYKNoMode == YKNO_BIN)
		wYKNo = m_pCDTReceiveFrame->InfoByte2;
	else
		wYKNo = BCD2WORD(MAKEWORD(m_pCDTReceiveFrame->InfoByte3, m_pCDTReceiveFrame->InfoByte4));
	pYKInfo->Head.wEqpID = m_wEqpID;
	pYKInfo->Info.wID = wYKNo;
	
	switch (Command)
	{
		case YK_SELECT:
			pCtrlInfo->DCO = YK_SELECT;
			pYKInfo->Head.byMsgID = MI_YKSELECT;
			break;
		case YK_EXE:
			pCtrlInfo->DCO = YK_EXE;
			pYKInfo->Head.byMsgID = MI_YKOPRATE;
			break;
		case YK_CANCEL:
			pCtrlInfo->DCO = YK_CANCEL;
			pYKInfo->Head.byMsgID = MI_YKCANCEL;
			break;
		default:
			break;	
	}
		
	switch (m_pCDTReceiveFrame->InfoByte1)
	{
		case 0xCC: 
			pYKInfo->Info.byValue = 0x05;
			break; //合
		case 0x33: 
			pYKInfo->Info.byValue = 0x06;
			break; 
		default:
			break;
	}
	pCtrlInfo->Type = GetRecFrameType();
	if (!(m_pBaseCfg->wCmdControl&YKOPENABLED))
	{
		SetEqpFlag(EQPFLAG_YKRet);
        return TRUE;		
	}
    CPSecondary::DoYKReq();
	return TRUE;
}

#endif


