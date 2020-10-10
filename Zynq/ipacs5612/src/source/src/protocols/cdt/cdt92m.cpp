/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/
#include "syscfg.h"
			
#ifdef INCLUDE_CDT92_M


#include "cdt92m.h"

extern "C" void cdt92m(WORD wTaskID)
{
	CCDT92P *pCDT92P = new CCDT92P();		
	
	if (pCDT92P->Init(wTaskID) != TRUE)
	{
		pCDT92P->ProtocolExit();
	}
	pCDT92P->Run(); 
}

extern "C" void cdt92m_load(int tid)
{
    char tname[30];
	
	sprintf(tname,"tCdtm%d",tid);		
	SetupCommonTask(tid, tname, COMMONPRIORITY, COMMONSTACKSIZE, (ENTRYPTR)cdt92m, COMMONQLEN);
}

////构造函数
CCDT92P::CCDT92P(void)
{
	
}

//实现CDT92规约的初始化 
BOOL CCDT92P::Init(WORD wTaskID)
{
	bool rc;
	
	rc = CCDTPri::Init(wTaskID, 1, NULL, sizeof(VCdt92Cfg));
	if (!rc)
	{
		return FALSE;
	}
	memset( &SoeFrame, 0,  sizeof(SoeFrame) );

	memset( &m_OldSoe[0], 0,  sizeof(SoeFrame)*OLDSOE_NUM );
	m_OldSoe_wp = 0;

	m_bWaitSoePart2 = 0;
	return TRUE;
}

void CCDT92P::SetBaseCfg(void)
{
	CCDTPri::SetBaseCfg();

	m_pBaseCfg->wCmdControl = YKOPENABLED | CLOCKENABLE ;
	m_pBaseCfg->wMaxRetryCount = 0;//must =0 otherwise when have no receive data,it will send retry always
	m_pBaseCfg->wMaxErrCount = 3;		 
	m_pBaseCfg->Timer.wSetClock = 5;
}


BOOL CCDT92P::DoReceive(void)
{
	if ( SearchFrame() != TRUE ) 			
		return FALSE;
	/*只接收信息字,控制字由基类处理*/
	if ( GetRecWordType() != WORD_INFO )	
		return TRUE;
	
	m_pCDTReceiveFrame = (VCDTReceiveFrame *)m_RecFrame.pBuf;
	/*插入信息字处理*/
	BYTE byFun = m_pCDTReceiveFrame->FunctionCode;
	if( (byFun & 0xe0) == 0xe0 )
	{
	  switch ( byFun )
	  {
		case FUN_YK_RET:
		#ifdef INCLUDE_ETSZ4_DWYT
		case FUN_YT_RET:
		#endif
			DoYKReturn();
			break;
		default:
			ReceiveYX();
			break;
	  }	
	  return TRUE;
	}
	/*控制字中的帧类型*/
 	switch (GetRecFrameType())
	{
		case REC_YC_A: 
		case REC_YC_B: 
		case REC_YC_C: 
			ReceiveYC(); 
			break;
		case REC_YX: 
			ReceiveYX(); 
			break;
		case REC_DD: 
			ReceiveDD(); 
			break;
		case REC_SOE: 
			ReceiveSOE(); 
			break;
		default: 
			break;
	}
	return TRUE;
}


BOOL CCDT92P::ReceiveYC(void)
{
	BYTE byOffset, byLow, byHigh;
	WORD wYCValue;
	
	byOffset = m_pCDTReceiveFrame->FunctionCode * 2;
	byLow = m_pCDTReceiveFrame->InfoByte1;
	if ( m_pCDTReceiveFrame->InfoByte2 & 0x08 )
		byHigh = m_pCDTReceiveFrame->InfoByte2 | 0xF0;
	else	
		byHigh = m_pCDTReceiveFrame->InfoByte2 & 0x0F;
	wYCValue = MAKEWORD(byLow, byHigh);
	WriteRangeYC(m_wEqpID, byOffset, 1, (short *)&wYCValue);

	byLow = m_pCDTReceiveFrame->InfoByte3;
	if ( m_pCDTReceiveFrame->InfoByte4 & 0x08 )
		byHigh = m_pCDTReceiveFrame->InfoByte4 | 0xF0;
	else	
		byHigh = m_pCDTReceiveFrame->InfoByte4 & 0x0F;
	wYCValue = MAKEWORD(byLow, byHigh);
	WriteRangeYC(m_wEqpID, byOffset+1, 1, (short *)&wYCValue);
	return TRUE;
}

BOOL CCDT92P::ReceiveYX(void)
{
	BYTE byOffset = m_pCDTReceiveFrame->FunctionCode - 0xf0;
	
	BYTE abyBuf[4];
	abyBuf[0] = m_pCDTReceiveFrame->InfoByte1;
	abyBuf[1] = m_pCDTReceiveFrame->InfoByte2;	
	abyBuf[2] = m_pCDTReceiveFrame->InfoByte3;
	abyBuf[3] = m_pCDTReceiveFrame->InfoByte4;
	
	WriteRangeSYXBit(m_wEqpID, byOffset*32, 32, &abyBuf[0]);
	return TRUE;
}

BOOL CCDT92P::ReceiveDD(void)
{
	BYTE byOffset = m_pCDTReceiveFrame->FunctionCode - 0xa0;
	BYTE abyBuf[4];
	abyBuf[0] = m_pCDTReceiveFrame->InfoByte1;
	abyBuf[1] = m_pCDTReceiveFrame->InfoByte2;	
	abyBuf[2] = m_pCDTReceiveFrame->InfoByte3;
	abyBuf[3] = m_pCDTReceiveFrame->InfoByte4;
	DWORD dwDDValue = MAKEDWORD(MAKEWORD(abyBuf[0], abyBuf[1]), MAKEWORD(abyBuf[2], abyBuf[3]));
	
	WriteRangeDD(m_wEqpID, byOffset, 1, (long *)&dwDDValue);
	return TRUE;
}

int CCDT92P::ReceiveSOE(void)
{
	BYTE fun = m_pCDTReceiveFrame->FunctionCode;
	/*保证帧连续,不完全!*/
	if((fun&1) == 1)
	{
		if (fun != m_byLastFunCode+1) 	
			return ERROR;
		if (m_bWaitSoePart2 != 1)		
			return ERROR;
	}
	/*接收SOE*/
	if((fun&1) == 0){
		SoeFrame.Code1     	= m_pCDTReceiveFrame->FunctionCode;
		SoeFrame.MSecondLo 	= m_pCDTReceiveFrame->InfoByte1;
		SoeFrame.MSecondHi 	= m_pCDTReceiveFrame->InfoByte2&0x03;
		SoeFrame.Second    	= m_pCDTReceiveFrame->InfoByte3&0x3F;
		SoeFrame.Minute    	= m_pCDTReceiveFrame->InfoByte4&0x3F;
		SoeFrame.Crc1	   	= m_pCDTReceiveFrame->BchCode;
		m_bWaitSoePart2  = 1;
	}else{
		SoeFrame.Code2     	= m_pCDTReceiveFrame->FunctionCode;
		SoeFrame.Hour	   	= m_pCDTReceiveFrame->InfoByte1&0x1F;
		SoeFrame.Day	   	= m_pCDTReceiveFrame->InfoByte2&0x1F;
		SoeFrame.YXNoLo		= m_pCDTReceiveFrame->InfoByte3;
		SoeFrame.YXNoHi		= m_pCDTReceiveFrame->InfoByte4;
		SoeFrame.Crc2	   	= m_pCDTReceiveFrame->BchCode;
		m_bWaitSoePart2  = 0;
	}
	if(fun&1){
		/*丢弃重复的SOE*/
		int i;
		for(i=0; i<OLDSOE_NUM; i++)
		{
			if(m_OldSoe[i].YXNoLo    != SoeFrame.YXNoLo) 	continue;
			if(m_OldSoe[i].YXNoHi    != SoeFrame.YXNoHi)	continue;
			if(m_OldSoe[i].MSecondLo != SoeFrame.MSecondLo) continue;
			if(m_OldSoe[i].MSecondHi != SoeFrame.MSecondHi) continue;
			if(m_OldSoe[i].Second 	 != SoeFrame.Second)    continue;
			if(m_OldSoe[i].Minute    != SoeFrame.Minute)    continue;
			if(m_OldSoe[i].Hour      != SoeFrame.Hour) 		continue;
			if(m_OldSoe[i].Day       != SoeFrame.Day) 		continue;
			break;
		}
		if (i < OLDSOE_NUM) 
			return ERROR;
		/*写入SOE*/
		WORD no = MAKEWORD((SoeFrame.YXNoLo), (SoeFrame.YXNoHi&0x0F));
		BYTE value;
		if(SoeFrame.YXNoHi & 0x80) 
			value = 0x81;
		else					 
			value = 0x01;

		struct VDBSOE soe_buf;
		struct VSysClock time;
		GetSysClock((void *)&time, SYSCLOCK);
		/*time.wYear = m_wYear;
		time.byMonth = m_byMonth;*/
		time.byDay = SoeFrame.Day;
		time.byHour = SoeFrame.Hour;
		time.byMinute = SoeFrame.Minute;
		time.bySecond = SoeFrame.Second;
		time.wMSecond = MAKEWORD(SoeFrame.MSecondLo, SoeFrame.MSecondHi);
		ToCalClock(&time,&(soe_buf.Time));
		soe_buf.wNo = no; soe_buf.byValue = value;
		
		WriteSSOE(m_wEqpID,1,&soe_buf);
		/*缓存soe*/
		m_OldSoe[m_OldSoe_wp].MSecondLo = SoeFrame.MSecondLo;
		m_OldSoe[m_OldSoe_wp].MSecondHi = SoeFrame.MSecondHi;
		m_OldSoe[m_OldSoe_wp].Second    = SoeFrame.Second;
		m_OldSoe[m_OldSoe_wp].Minute    = SoeFrame.Minute;
		m_OldSoe[m_OldSoe_wp].Hour    	= SoeFrame.Hour;
		m_OldSoe[m_OldSoe_wp].Day    	= SoeFrame.Day;
		m_OldSoe[m_OldSoe_wp].YXNoLo    = SoeFrame.YXNoLo;
		m_OldSoe[m_OldSoe_wp].YXNoHi    = SoeFrame.YXNoHi;
		m_OldSoe_wp = (m_OldSoe_wp+1) % OLDSOE_NUM;
	}	
	return OK;
}



BOOL CCDT92P::DoYKReq(void)
{
	m_SendBuf.wReadPtr = 0;
	m_SendBuf.wWritePtr = 0;
	CCDTPri::DoYKReq();
	/*遥控请求*/
	if (SwitchClearEqpFlag(EQPFLAG_YKReq))
	{
	   int result = SendYK();
	   if(result != OK)
		   MyYKReturn(result);
	}
	WriteToComm(GetEqpAddr());
	return TRUE;
}

BOOL CCDT92P::DoYKReturn(void)
{
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	
	pYKInfo->Info.byStatus = 3;
	if (m_pCDTReceiveFrame->InfoByte1 == YK_SET_CLOSE)
	{
		pYKInfo->Info.byValue = YK_CLOSE |CTR_BYID;
		pYKInfo->Info.byStatus = 0;
	}
	else 
		if(m_pCDTReceiveFrame->InfoByte1 == YK_SET_OPEN)
		{
			pYKInfo->Info.byValue = YK_OPEN |CTR_BYID;
			pYKInfo->Info.byStatus = 0;
		}
	CCDTPri::DoYKRet();
	return TRUE;
}

int CCDT92P::SendYK(void)
{
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	if (!pYKInfo) 
		return 1;
	/*得到帧类型码, 功能码,对象码和动作码*/
	BYTE frame, fun, act;
	WORD obj;
	if (m_pVCdt92Cfg->byYKNoMode == YKNO_BIN)
		if(pYKInfo->Info.wID > 255 ) 
			return 1;
	obj = pYKInfo->Info.wID;

	if ((pYKInfo->Info.byValue & 0x3) == YK_CLOSE) 
		act = YK_SET_CLOSE;
    else
    {
		if ((pYKInfo->Info.byValue & 0x3) == YK_OPEN ) 
    		act = YK_SET_OPEN;
    	else return 4;
    }
  	switch (pYKInfo->Head.byMsgID & 0x7f)
	{
      	case MI_YKSELECT:
		#ifndef INCLUDE_ETSZ4_DWYT
      		frame = CODE_YK_SELECT;
    		fun = FUN_YK_SELECT;
		#else
		frame = CODE_YT_SELECT;
    		fun = FUN_YT_SELECT;
		#endif
   	    	break;
      	case MI_YKOPRATE:
		#ifndef INCLUDE_ETSZ4_DWYT
      		frame = CODE_YK_EXE;
    		fun = FUN_YK_EXE;
		#else
		frame = CODE_YT_EXE;
    		fun = FUN_YT_EXE;
		#endif
    		act = YK_EXE;
			pYKInfo->Info.byStatus = 0;
			TaskSendMsg(DB_ID, m_wTaskID, m_wEqpID, pYKInfo->Head.byMsgID,MA_RET, sizeof(VDBYK), m_pMsg);
			break;
      	case MI_YKCANCEL:
		#ifndef INCLUDE_ETSZ4_DWYT
      		frame = CODE_YK_CANCEL;
    		fun = FUN_YK_CANCEL;
		#else
		frame = CODE_YT_CANCEL;
    		fun = FUN_YT_CANCEL;
		#endif
    		act = YK_CANCEL;
			pYKInfo->Info.byStatus = 0;
			TaskSendMsg(DB_ID, m_wTaskID, m_wEqpID, pYKInfo->Head.byMsgID,MA_RET, sizeof(VDBYK), m_pMsg);
			break;
     	default:
    		return 5;
        	/*break;*/
  	}
	/*按得到的码组帧, 遥控字需连续3次*/
	SendCtrlWord(CONTROL_BYTE, frame, 3);
	int index = m_SendBuf.wWritePtr;
	m_SendBuf.pBuf[ index++ ] = fun;
	m_SendBuf.pBuf[ index++ ] = act;
	if (m_pVCdt92Cfg->byYKNoMode == YKNO_BIN)
	{
		m_SendBuf.pBuf[ index++ ] = LOBYTE(obj);
		m_SendBuf.pBuf[ index++ ] = act;
		m_SendBuf.pBuf[ index++ ] = LOBYTE(obj);
	}
	else
	{
		m_SendBuf.pBuf[ index++ ] = 0xff;
		m_SendBuf.pBuf[ index++ ] = LOBYTE(WORD2BCD(obj));
		m_SendBuf.pBuf[ index++ ] = HIBYTE(WORD2BCD(obj));
	}
	m_SendBuf.wWritePtr = index;
	BYTE crc = GetSendBCH();
	m_SendBuf.pBuf[ index++ ] = crc;
	/*再重复两次*/
	for (int i=0; i<2; i++)
	{
		m_SendBuf.pBuf[ index++ ] = fun;
		m_SendBuf.pBuf[ index++ ] = act;
		if (m_pVCdt92Cfg->byYKNoMode == YKNO_BIN)
		{
			m_SendBuf.pBuf[ index++ ] = LOBYTE(obj);
			m_SendBuf.pBuf[ index++ ] = act;
			m_SendBuf.pBuf[ index++ ] = LOBYTE(obj);
		}
		else
		{
			m_SendBuf.pBuf[ index++ ] = 0xff;
			m_SendBuf.pBuf[ index++ ] = LOBYTE(WORD2BCD(obj));
			m_SendBuf.pBuf[ index++ ] = HIBYTE(WORD2BCD(obj));
		}
		m_SendBuf.pBuf[ index++ ] = crc;
	}
	m_SendBuf.wWritePtr = index;
	return OK;
}

int  CCDT92P::MyYKReturn(int result)
{
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	if (!pYKInfo) 
		return ERROR;

	pYKInfo->Info.byStatus = result;

	/*切换到等待返校的模块*/
	m_wEqpID = pYKInfo->Head.wEqpID;
	DoYKRet();

	return OK;
}

BOOL CCDT92P::ReqCyc(void)
{
	if (m_pBaseCfg->wCmdControl & CLOCKENABLE)
		if (CheckClearTaskFlag(TASKFLAG_CLOCK))
		{
			SendSetTime();
			return TRUE;
		}
	WriteToComm(m_SendBuf.dwFlag);//send null to make send cycle continue	
	return FALSE;
}

void CCDT92P::SendSetTime(void)
{
	BYTE crc;
	int index;
	VSysClock SysTime;
	
	GetSysClock((void *)&SysTime, SYSCLOCK);
	
	SendCtrlWord(CONTROL_BYTE, CODE_TIMESET, 2);
	index = m_SendBuf.wWritePtr;
	m_SendBuf.pBuf[ index++ ] = 0xee;
	m_SendBuf.pBuf[ index++ ] = LOBYTE(SysTime.wMSecond);
	m_SendBuf.pBuf[ index++ ] = HIBYTE(SysTime.wMSecond);
	m_SendBuf.pBuf[ index++ ] = SysTime.bySecond;
	m_SendBuf.pBuf[ index++ ] = SysTime.byMinute;
	
	m_SendBuf.wWritePtr = index;
	crc = GetSendBCH();
	m_SendBuf.pBuf[ index++ ] = crc;
	
	m_SendBuf.wWritePtr = index;
	m_SendBuf.pBuf[ index++ ] = 0xef;
	m_SendBuf.pBuf[ index++ ] = SysTime.byHour;
	m_SendBuf.pBuf[ index++ ] = SysTime.byDay;
	m_SendBuf.pBuf[ index++ ] = SysTime.byMonth;
	m_SendBuf.pBuf[ index++ ] = SysTime.wYear - 2000;
	
	m_SendBuf.wWritePtr = index;
	crc = GetSendBCH();
	m_SendBuf.pBuf[ index++ ] = crc;
	
	m_SendBuf.wWritePtr = index;

	WriteToComm(GetEqpAddr());
	return;
}

#endif


