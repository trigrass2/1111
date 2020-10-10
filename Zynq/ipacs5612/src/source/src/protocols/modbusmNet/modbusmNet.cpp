/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef INCLUDE_MODBUSNET_M

#include "modbusmNet.h"

extern "C"void modbusmNet(WORD wTaskID)             
{
    COPENMODBUSPNET *pMODBUSPNET = new COPENMODBUSPNET();    
    
    if (pMODBUSPNET->Init(wTaskID) != TRUE)
    {
        pMODBUSPNET->ProtocolExit();
    }
    
    pMODBUSPNET->Run(); 
                
}

extern "C" void modbusm_loadNet(int tid)
{
    char tname[30];
	
	sprintf(tname,"tmodbusmNet%d",tid);		
	SetupCommonTask(tid, tname, COMMONPRIORITY, COMMONSTACKSIZE, (ENTRYPTR)modbusmNet, COMMONQLEN);
}

COPENMODBUSPNET::COPENMODBUSPNET() : CPPrimary()
{
	YKRegAddr = 0xffff;
	YKSetValue = 0xffff;
	YTRegAddr = 0xffff;
	YTSetValue = 0xffff;
	m_bTcpConnect=FALSE;
	m_RegNum = 0;
	m_pCfg = new VModbusPara;
	memset(m_pCfg,0,sizeof(VModbusPara));

}

BOOL COPENMODBUSPNET::Init(WORD wTaskID)
{
    BOOL rc;
    rc = CPPrimary::Init(wTaskID,1,(void *)m_pCfg,sizeof(VModbusPara));   

    m_wCommCtrl = COMM_IDLE_ON;
    m_dwCommCtrlTimer *= 5;
    commCtrl(m_wTaskID,CCC_EVENT_CTRL|m_wCommCtrl,&m_dwCommCtrlTimer);
	DoCommState();
    if (!rc)
    {
        return FALSE;
    }
 
    return TRUE;    
}

DWORD COPENMODBUSPNET::DoCommState(void)
{
	DWORD dwCommState;
	BOOL bTcpState = FALSE;
	
	dwCommState=CPPrimary::DoCommState();
	if (dwCommState == MODBUS_TCP_COMM_ON)
		bTcpState = TRUE;
	else
		bTcpState = FALSE;
		
	if (bTcpState && !m_bTcpConnect)
	{
		ReqNormal();
	}
	m_bTcpConnect=bTcpState;
	
	return dwCommState;
}

void COPENMODBUSPNET::CheckBaseCfg(void)
{
    CProtocol::CheckBaseCfg();
}

void COPENMODBUSPNET::SetBaseCfg(void)
{
    CProtocol::SetBaseCfg();
    //if need to modify ,to do :

    m_pBaseCfg->wCmdControl = YKOPENABLED | CLOCKENABLE | DDPROC;
    m_pBaseCfg->Timer.wAllData = 1; // 3 min
    m_pBaseCfg->Timer.wSetClock = 60;       //60 min
    m_pBaseCfg->Timer.wDD = 20;             //20 min insted for call Soe
    m_pBaseCfg->wMaxRetryCount = 1;
    m_pBaseCfg->wMaxErrCount = 3;
    m_pBaseCfg->wMaxALLen = MAX_MODBUS_FRAME_SIZE;
    m_pBaseCfg->wBroadcastAddr = 0x0;

	m_pCfg->byModbusCfg=0xb;
	m_pCfg->YCPara.byFunCode=3;
	m_pCfg->YCPara.byRegGroup=1;
	m_pCfg->YCPara.Reg[0].wRegAddress=100;
	m_pCfg->YCPara.Reg[0].wRegNum=20;
	m_pCfg->YCPara.Reg[0].wData.wNum=20;
	m_pCfg->YKPara.byFunCode=5;
	m_pCfg->YKPara.byRegGroup=2;
	m_pCfg->YKPara.Reg[0].wRegAddress=10;
	m_pCfg->YKPara.Reg[0].wRegNum=12;
	m_pCfg->YKPara.Reg[0].wData.wNum=0x0000;
	m_pCfg->YKPara.Reg[1].wRegAddress=10;
	m_pCfg->YKPara.Reg[1].wRegNum=12;
	m_pCfg->YKPara.Reg[1].wData.wNum=0xff00;
	m_pCfg->YXPara.byFunCode=1;
	m_pCfg->YXPara.byRegGroup=1;
	m_pCfg->YXPara.Reg[0].wRegAddress=10;
	m_pCfg->YXPara.Reg[0].wRegNum=12;
	m_pCfg->YXPara.Reg[0].wData.wNum=12;
	m_pCfg->DDPara.byFunCode=0;
	m_pCfg->DDPara.byRegGroup=0;
	m_pCfg->DDPara.Reg[0].wRegAddress=10;
	m_pCfg->DDPara.Reg[0].wRegNum=12;
	m_pCfg->DDPara.Reg[0].wData.wNum=12;
	m_pCfg->YTPara.byFunCode=5;
	m_pCfg->YTPara.byRegGroup=2;
	m_pCfg->YTPara.Reg[0].wRegAddress=10;
	m_pCfg->YTPara.Reg[0].wRegNum=12;
	m_pCfg->YTPara.Reg[0].wData.wNum=0x0000;
	m_pCfg->YTPara.Reg[1].wRegAddress=10;
	m_pCfg->YTPara.Reg[1].wRegNum=12;
	m_pCfg->YTPara.Reg[1].wData.wNum=0xff00;
}

void COPENMODBUSPNET::CheckCfg(void)
{
   //VRegPara *RegPara;
	if (m_pCfg->byModbusCfg == 0)
	{
	    myprintf(m_wTaskID,LOG_ATTR_WARN,"This modbus protocol have no operation.");
		return;
	}
	
		/*#if (_BYTE_ORDER == _BIG_ENDIAN)  //Moto系统
		int i;
		if (m_pCfg->byModbusCfg & YXENABLE)
		{
		   RegPara = (VRegPara *)m_pCfg->YXPara.Reg;
		   for(i=0; i<m_pCfg->YXPara.byRegGroup; i++)
		   {
		      RegPara->wRegAddress = ADJUSTWORD(RegPara->wRegAddress);
			  RegPara->wData.wValue = ADJUSTWORD(RegPara->wData.wNum);
			  RegPara->wRegNum = ADJUSTWORD(RegPara->wRegNum);
			  RegPara ++;
		   }
		}
		if (m_pCfg->byModbusCfg & YCENABLE)
		{
		   RegPara = (VRegPara *)m_pCfg->YCPara.Reg;
		   for(i=0; i<m_pCfg->YCPara.byRegGroup; i++)
		   {
		      RegPara->wRegAddress = ADJUSTWORD(RegPara->wRegAddress);
			  RegPara->wData.wValue = ADJUSTWORD(RegPara->wData.wNum);
			  RegPara->wRegNum = ADJUSTWORD(RegPara->wRegNum);
			  RegPara ++;
		   }
		}
		if (m_pCfg->byModbusCfg & DDENABLE)
		{
		   RegPara = (VRegPara *)m_pCfg->DDPara.Reg;
		   for(i=0; i<m_pCfg->DDPara.byRegGroup; i++)
		   {
		      RegPara->wRegAddress = ADJUSTWORD(RegPara->wRegAddress);
			  RegPara->wData.wValue = ADJUSTWORD(RegPara->wData.wNum);
			  RegPara->wRegNum = ADJUSTWORD(RegPara->wRegNum);
			  RegPara ++;
		   }
		}
		if (m_pCfg->byModbusCfg & YKENABLE)
		{
		   RegPara = (VRegPara *)m_pCfg->YKPara.Reg;
		   for(i=0; i<m_pCfg->YKPara.byRegGroup; i++)
		   {
		      RegPara->wRegAddress = ADJUSTWORD(RegPara->wRegAddress);
			  RegPara->wData.wValue = ADJUSTWORD(RegPara->wData.wValue);
			  RegPara->wRegNum = ADJUSTWORD(RegPara->wRegNum);
			  RegPara ++;
		   }
		}
		#endif*/
		
	if (m_pCfg->byModbusCfg & YXENABLE)
		m_byNextReq = REQ_YX;
	else if (m_pCfg->byModbusCfg & YCENABLE)
		m_byNextReq = REQ_YC;
	else
		m_byNextReq = 0;

    /*if (m_pCfg->byModbusCfg & YXENABLE)
	{
	   if ((m_pCfg->YXPara.byFunCode == FUN_READ_COIL) ||(m_pCfg->YXPara.byFunCode == FUN_READ_DI))
	   	  m_byYXStatus = YX_BIT;
	   else
	   	  m_byYXStatus = YX_WORD;		 		    
	}*/
	
	if(!(m_pCfg->byModbusCfg & DDENABLE))
		m_pBaseCfg->wCmdControl &= ~DDPROC;
	else
		m_pBaseCfg->wCmdControl |= DDPROC;			
	m_pBaseCfg->wCmdControl &= ~ALLDATAPROC;

	//change by wyj
    m_pBaseCfg->wMaxRetryCount = 1;
    m_pBaseCfg->wMaxErrCount = 10;
}


DWORD COPENMODBUSPNET::SearchOneFrame(BYTE *Buf, WORD Len)
{
    //WORD CheckOffset,CheckRemote,CheckLocal;
	WORD FrameLen = 1;
    BYTE byAddress;

    m_pReceive = (VOpenModbusFrame *)(Buf);

    if ( Len < MODBUS_MINFRAMELEN )  
            return(FRAME_LESS);

    byAddress = m_pReceive->byAddress;
   /* if ( SwitchToAddress(byAddress) == FALSE )*/
   if(byAddress != m_SendAddress)
            return (FRAME_ERR | 1);

    if ((m_pReceive->byFunCode == FUN_READ_KEEPREG) ||(m_pReceive->byFunCode == FUN_READ_DI)\
        ||(m_pReceive->byFunCode == FUN_READ_SINGLEREG) ||(m_pReceive->byFunCode == FUN_READ_COIL))
            FrameLen = m_pReceive->byData + 5;
    else if ((m_pReceive->byFunCode == FUN_WRITE_SINGLEREG)||(m_pReceive->byFunCode == FUN_WRITE_COIL)\
        ||(m_pReceive->byFunCode == FUN_WRITE_MUTIREG))
            FrameLen = 8;
	//else WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "Rx %X %X %X : %X %X %X",m_pReceive->eventPID,m_pReceive->pType,m_pReceive->len,m_pReceive->byAddress,m_pReceive->byFunCode,m_pReceive->byData);

    if (FrameLen<2)  return (FRAME_ERR|1);

    if (Len < FrameLen)
            return (FRAME_LESS);

    //CheckOffset = FrameLen - 2;//CRC校验码的位置
            
    //CheckRemote = Crc16(0xFFFF,Buf,CheckOffset);//接收到的校验码            
    //CheckLocal = MAKEWORD(Buf[CheckOffset], Buf[CheckOffset+1]);//根据接收报文计算的校验码

    //if ( CheckLocal != CheckRemote )   
		//return (FRAME_ERR | 1);
		//return (FRAME_ERR | Len);  //change by wyj,modbus 无同步字,全扔
    
    return (FRAME_OK | FrameLen);
}

BOOL COPENMODBUSPNET::DoReceive(void)
{
	if (SearchFrame() == FALSE)  
		return (FALSE); 
	m_pReceive = ( VOpenModbusFrame *)m_RecFrame.pBuf;
	m_pbyData = &m_pReceive->byData;

	if(m_pReceive->byFunCode!=6)
	{
		switch(m_byReqDataType)
		{
			case REQ_YX:		  	   
				DoYxData();
				break;

			case REQ_YC:
				DoYcData();
				break;

			case REQ_DD:
				DoDdData();
				break;

			case REQ_YK:
				DoYKData();
				break;
			case REQ_YT:
				DoYTData();
				break;

			default:
				myprintf(m_wTaskID,LOG_ATTR_INFO,"MODBUSPNET Receive Data OK!");
				break;

		}
	}
	else WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "Rx %X %X %X : %X %X %X %X %X %X",m_pReceive->eventPID,m_pReceive->pType,m_pReceive->len,m_pReceive->byAddress,m_pReceive->byFunCode,m_pbyData[0],m_pbyData[1],m_pbyData[2],m_pbyData[3]);

	if (m_bPollMasterSendFlag)//wdj 2019.1.21 
		//DoSend();
		stopflag=1;
	else
		commCtrl(m_wTaskID, CCC_EVENT_CTRL|m_wCommCtrl, &m_dwCommCtrlTimer);
	return(TRUE); 
}

BOOL COPENMODBUSPNET::ReqUrgency(void)
{
	//VYKInfo *pYKInfo;
	//VRegPara *pYKOpenPara,*pYKClosePara;
	//VYTInfo *pYTInfo;
	//VRegPara *pYTPara;
//    if(m_pCfg->byModbusCfg & YKENABLE)
//    {
//    	if (SwitchToEqpFlag(EQPFLAG_YKReq))
//		{     
//            ClearEqpFlag(EQPFLAG_YKReq);/*立刻清标志*/
//            pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
//			if(m_pCfg->YKPara.byRegGroup>2)
//	        	pYKClosePara = (VRegPara *)(m_pCfg->YKPara.Reg)+2*(pYKInfo->Info.wID-1); //??
//			else
//	       		pYKClosePara  = (VRegPara *)(m_pCfg->YKPara.Reg); //??
//				
//			pYKOpenPara = pYKClosePara +1;
//            switch (pYKInfo->Head.byMsgID & 0x7f)
//            {
//                case MI_YKSELECT:  
//		             YkResult(TRUE);
//				     return(FALSE); /*没有遥控预置*/
//                case MI_YKOPRATE:  
//                     if ((pYKInfo->Info.byValue & 0x3) == 1)//close
//                     {
//						YKRegAddr = pYKClosePara->wRegAddress;    
//						YKSetValue = pYKClosePara->wData.wValue;    //??
//                     }
//                     if ((pYKInfo->Info.byValue & 0x3) == 2)//open
//                     {
//						YKRegAddr = pYKOpenPara->wRegAddress; 
//						YKSetValue = pYKOpenPara->wData.wValue;    //??
//                     }   
//					if(m_pCfg->YKPara.byRegGroup==2)
//						YKRegAddr+=pYKInfo->Info.wID-1;
//			         m_byReqDataType= REQ_YK;
//					 if(m_pCfg->YKPara.byFunCode == FUN_WRITE_MUTIREG)
//					    SendWriteMReg(YKRegAddr, 1, 0, &YKSetValue);	 					    
//					 else
//                        SendWriteCmd(m_pCfg->YKPara.byFunCode,YKRegAddr,0,YKSetValue); 
//                     break;
//                  case MI_YKCANCEL:
//                      YkResult(TRUE);
//                      return FALSE;
//             }
//             return TRUE;
//                
//          }
//   	}
#if 0
//	if(m_pCfg->byModbusCfg & YTENABLE)
//	{
//       if (SwitchToEqpFlag(EQPFLAG_YTReq))
//       {     
//	        ClearEqpFlag(EQPFLAG_YTReq);/*立刻清标志*/
//	        pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);

//			pYTPara = (VRegPara *)(m_pCfg->YTPara.Reg)+(pYTInfo->Info.wID-1); //??
//			
//	        switch (pYTInfo->Head.byMsgID & 0x7f)
//	        {
//				case MI_YTSELECT:  
//					YtResult(TRUE);
//					return(FALSE); 
//	            case MI_YTOPRATE:  
//	                 
//                     YTRegAddr = pYTPara->wRegAddress;    
//	                 YTSetValue = pYTPara->wData.wValue;    //??
//	                
//					m_byReqDataType = REQ_YT;

//					if(m_pCfg->YTPara.byFunCode == FUN_WRITE_MUTIREG)
//						SendWriteMReg(YTRegAddr, 1, 0, &YTSetValue);	 					    
//					else
//						SendWriteCmd(m_pCfg->YTPara.byFunCode,YTRegAddr,0,YTSetValue); 
//					break;
//	             case MI_YTCANCEL:
//					YtResult(TRUE);
//					return FALSE;
//			}
//			return TRUE;
//            
//		}
//   	}
#endif
    return  FALSE;
}

BOOL COPENMODBUSPNET::ReqCyc(void)
{
  
    if ((m_pBaseCfg->wCmdControl & CLOCKENABLE) && (!(m_pBaseCfg->wCmdControl & BROADCASTTIME)))
        if (CheckClearEqpFlag(EQPFLAG_CLOCK))
        {
             SendSetTime();
             return TRUE;
        }
	if(m_pBaseCfg->wCmdControl & DDPROC)
	{
	     if (GetEqpFlag(EQPFLAG_DD)&&(m_byNextReq!= REQ_DD))
	     {
	  	    m_byNextReq = REQ_DD;
			m_RegNum = 0;
			return FALSE;
	  	  }
	    
	}
        return FALSE;
}
   
BOOL COPENMODBUSPNET::ReqNormal(void)
{
   	VRegPara *pYXInfo,*pYCInfo,*pDDInfo;
    if (m_byNextReq == REQ_YX)
    {
		m_byReqDataType = REQ_YX;

		SetEqpFlag(EQPFLAG_YC);
		SetEqpFlag(EQPFLAG_YX);


		pYXInfo = (VRegPara*)m_pCfg->YXPara.Reg + m_RegNum;
		SendReadCmd(m_pCfg->YXPara.byFunCode, pYXInfo->wRegAddress,0,pYXInfo->wRegNum); 

		m_RegNow = m_RegNum;
		m_RegNum ++; 
		if (m_RegNum >= m_pCfg->YXPara.byRegGroup)
		{
		   m_RegNum = 0;
		   if (m_pCfg->byModbusCfg & YCENABLE)
				m_byNextReq = REQ_YC;
		   else
		   {
				ClearEqpFlag(EQPFLAG_YC);
				ClearEqpFlag(EQPFLAG_YX);
		   }
		   	 
		}
		return TRUE;
     }

     if (m_byNextReq == REQ_YC)
     {
		m_byReqDataType = REQ_YC;

		SetEqpFlag(EQPFLAG_YC);
		SetEqpFlag(EQPFLAG_YX);
 
		pYCInfo = (VRegPara*)m_pCfg->YCPara.Reg + m_RegNum;
		SendReadCmd(m_pCfg->YCPara.byFunCode,pYCInfo->wRegAddress,0,pYCInfo->wRegNum);

		m_RegNow = m_RegNum;
		m_RegNum ++;
		if (m_RegNum >= m_pCfg->YCPara.byRegGroup)
		{
			m_RegNum = 0;
			if (m_pCfg->byModbusCfg & YXENABLE)
				m_byNextReq = REQ_YX;
			ClearEqpFlag(EQPFLAG_YC);
			ClearEqpFlag(EQPFLAG_YX);
		}
		return TRUE;
    }
	if (m_byNextReq == REQ_YT)
	{		 
		m_byReqDataType = REQ_YT;
		if(m_pCfg->YTPara.byFunCode == FUN_WRITE_MUTIREG)
			SendWriteMReg(YTRegAddr, 1, 0, &YTSetValue);	 					    
		if(m_pCfg->YTPara.byFunCode == FUN_WRITE_SINGLEREG)
		{
			if(YTSetValue==0x5555)
			{
				SendWriteYkCmd(m_pCfg->YTPara.byFunCode,YTRegAddr,0,YTSetValue);
			}
			else 
			{
				SendWriteCmd(m_pCfg->YTPara.byFunCode,YTRegAddr,0,YTSet); 
			}
		}
		m_byNextReq=ytykflag;

		return TRUE;				
	}
		
	if(m_byNextReq == REQ_DD)
	{
		m_byReqDataType = REQ_DD;

		pDDInfo = (VRegPara*)m_pCfg->DDPara.Reg + m_RegNum;
		SendReadCmd(m_pCfg->DDPara.byFunCode,pDDInfo->wRegAddress,0,pDDInfo->wRegNum);

		m_RegNow = m_RegNum;
		m_RegNum ++;
		if(m_RegNum >= m_pCfg->DDPara.byRegGroup)
		{
			m_RegNum = 0;
			if(m_pCfg->byModbusCfg & YXENABLE)
				m_byNextReq = REQ_YX;
			else if (m_pCfg->byModbusCfg & YCENABLE)
				m_byNextReq = REQ_YC;
			else
				m_byNextReq = 0;
			ClearEqpFlag(EQPFLAG_DD);
		}
		return TRUE;
	}
 
	return FALSE;
}


void COPENMODBUSPNET::SendFrameHead(BYTE FunCode, BYTE ShiftAddress)
{	
	static WORD eventPID;
	
    m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
    m_pSend = (VOpenModbusFrame *)(m_SendBuf.pBuf);
	if(m_pSend->eventPID<65535)
		eventPID++;
	else
		eventPID=0;
	
	m_pSend->eventPID = (LOBYTE(eventPID)<<8) | HIBYTE(eventPID);
	
	m_pSend->pType=0;
    if(ShiftAddress == 0xff)
        m_pSend->byAddress = GetBroadcastAddr();
    else
        m_pSend->byAddress = (BYTE)m_pEqpInfo[m_wEqpNo].wDAddress + ShiftAddress;
	m_SendAddress = m_pSend->byAddress;
    m_pSend->byFunCode = FunCode;

    m_SendBuf.wWritePtr = (DWORD)&m_pSend->byData - (DWORD)m_pSend;
    //len=m_SendBuf.wWritePtr-2;
	//m_pSend->len = (LOBYTE(len)<<8) | HIBYTE(len);
}


void COPENMODBUSPNET::SendFrameTail(void)
{  
    //WORD CrcCode;
    BYTE byAddress;
	WORD len;
            
    //CrcCode = Crc16(0xFFFF,  m_SendBuf.pBuf, m_SendBuf.wWritePtr);

    //m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(CrcCode);
    //m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(CrcCode);

	len=m_SendBuf.wWritePtr-6;
	m_pSend->len = (LOBYTE(len)<<8) | HIBYTE(len);

    byAddress = m_pSend->byAddress;

	if(m_pSend->byFunCode==6)
	  WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "Tx:%X %X %X %X %X %X : %X %X %X %X %X %X ",m_SendBuf.pBuf[0],m_SendBuf.pBuf[1],m_SendBuf.pBuf[2],m_SendBuf.pBuf[3],m_SendBuf.pBuf[4],m_SendBuf.pBuf[5],m_SendBuf.pBuf[6],m_SendBuf.pBuf[7],m_SendBuf.pBuf[8],m_SendBuf.pBuf[9],m_SendBuf.pBuf[10],m_SendBuf.pBuf[11]);       
	
    WriteToComm(byAddress);
    m_bPollMasterSendFlag = FALSE;
    //DisableRetry();//for test
}

void COPENMODBUSPNET::SendSetTime(void)
{
    VSysClock SysTime;
	VRegPara *pYCInfo;
	
    GetSysClock((void *)&SysTime, SYSCLOCK);
    WORD *pwBuf = (WORD *)m_dwPubBuf;
    pYCInfo = (VRegPara*)m_pCfg->YCPara.Reg;
	
    pwBuf[0] = SysTime.wMSecond&0x00ff;
	pwBuf[1] = (SysTime.wMSecond&0xff00)>>8;
	pwBuf[2] = (WORD)SysTime.bySecond;
	pwBuf[3] = (WORD)SysTime.byMinute;
	pwBuf[4] = (WORD)SysTime.byHour;
	pwBuf[5] = (WORD)SysTime.byDay;
	pwBuf[6] = (WORD)SysTime.byMonth;
    pwBuf[7] = SysTime.wYear&0x00ff;
	pwBuf[8] = (SysTime.wYear&0xff00)>>8;
  
    SendWriteMReg(pYCInfo->wRegAddress, pYCInfo->wData.wNum, 0, pwBuf);

}


void COPENMODBUSPNET::SendReadSReg(WORD RegAddr, WORD RegNum, BYTE ShiftAddress)
{
    SendFrameHead(FUN_READ_SINGLEREG, ShiftAddress);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegAddr);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegAddr);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegNum);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegNum);
    SendFrameTail();
}

void COPENMODBUSPNET::SendWriteCmd(BYTE FunCode,WORD RegAddr, BYTE ShiftAddress, float Value)
{
	DWORD *tempValue;
	tempValue =(DWORD*) &Value;
//	WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "YT");
	SendFrameHead(FunCode, ShiftAddress);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegAddr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegAddr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(*tempValue));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(*tempValue));	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(*tempValue));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(*tempValue));
	SendFrameTail();
}
void COPENMODBUSPNET::SendWriteYkCmd(BYTE FunCode,WORD RegAddr, BYTE ShiftAddress, short Value)
{
//	WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "Yk");
	SendFrameHead(FunCode, ShiftAddress);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegAddr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegAddr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(Value);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(Value);	
//	  WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "Value:%d",Value);
//	WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "m_pSend->byAddress:%d",m_pSend->byAddress);
	SendFrameTail();
}

void COPENMODBUSPNET::SendReadCmd(BYTE FunCode,WORD RegAddr, BYTE ShiftAddress, WORD Length)
{
	SendFrameHead(FunCode, ShiftAddress);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegAddr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegAddr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(Length);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(Length);
	SendFrameTail();
}

void COPENMODBUSPNET::SendReadMReg(WORD RegAddr, WORD RegNum, BYTE ShiftAddress)
{
	SendFrameHead(FUN_READ_KEEPREG, ShiftAddress);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegAddr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegAddr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegNum);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegNum);
	SendFrameTail();
}

void COPENMODBUSPNET::SendWriteMReg(WORD RegAddrStart, BYTE RegNum, BYTE ShiftAddress, WORD *Value)
{
//		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "4");
    SendFrameHead(FUN_WRITE_MUTIREG, ShiftAddress);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegAddrStart);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegAddrStart);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegNum);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegNum);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegNum*2);
    for (int i = 0; i < RegNum; i++)
    {
        m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(Value[i]);
        m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(Value[i]);
    }
    SendFrameTail();
}

void COPENMODBUSPNET::SendReadDI(WORD RegAddr, WORD RegNum, BYTE ShiftAddress)
{
    SendFrameHead(FUN_READ_DI, ShiftAddress);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegAddr);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegAddr);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(RegNum);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(RegNum);
    SendFrameTail();
}


void COPENMODBUSPNET::DoTimeOut(void)
{
	CPPrimary::DoTimeOut();	
	DoCommState();
	DoSend();
}

void COPENMODBUSPNET::DoYKData(void)
{
	WORD RegAddr;
	WORD SetValue;
	bool res;
	
	RegAddr = MAKEWORD(m_pbyData[1],m_pbyData[0]);
	SetValue = MAKEWORD(m_pbyData[3],m_pbyData[2]);
	if(m_pCfg->YKPara.byFunCode == FUN_WRITE_MUTIREG)
	{
	  if ((RegAddr == YKRegAddr) && (SetValue == 1))
		res = TRUE;
	  else 
		res = FALSE;
	}
	else
	{
	  if ((RegAddr == YKRegAddr) && (SetValue == YKSetValue))
		res = TRUE;
	  else 
		res = FALSE;
	}
	YKRegAddr = 0xffff;  /*将这两个全局变量复归为初始化值*/
	YKSetValue = 0xffff;
	YkResult(res);
	return ;
}

BOOL COPENMODBUSPNET::DoYKReq(void)
{
	CPPrimary::DoYKReq();
	return TRUE;
}

void COPENMODBUSPNET::YkResult(BOOL Result)
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

void COPENMODBUSPNET::DoYTData(void)
{
	WORD RegAddr;
	WORD SetValue;
	bool res;
	
	RegAddr = MAKEWORD(m_pbyData[1],m_pbyData[0]);
	SetValue = MAKEWORD(m_pbyData[3],m_pbyData[2]);
	if(m_pCfg->YTPara.byFunCode == FUN_WRITE_MUTIREG)
	{
		if ((RegAddr == YTRegAddr) && (SetValue == 1))
			res = TRUE;
		else 
			res = FALSE;
	}
	else
	{
	  if ((RegAddr == YTRegAddr) && (SetValue == YTSetValue))
		res = TRUE;
	  else 
		res = FALSE;
	}
	YTRegAddr = 0xffff;  /*将这两个全局变量复归为初始化值*/
	YTSetValue = 0xffff;
	YtResult(res);
	return ;
}


BOOL COPENMODBUSPNET::DoYTReq(void)
{

	CPPrimary::DoYTReq();
	
	//if (SwitchClearEqpFlag(EQPFLAG_YTReq)) 
//	WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "DoYTReq");
	if (SendYT()) 
		return TRUE;
	
	return TRUE;
}


void COPENMODBUSPNET::YtResult(BOOL Result)
{
	VYTInfo *pYTInfo;
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);

	if (Result)
		pYTInfo->Info.byStatus = 0;
	else
	{
		pYTInfo->Info.byStatus = 3;
		ClearEqpFlag(EQPFLAG_YTReq);
	}
	CPPrimary::DoYTRet();
	return;
}



void COPENMODBUSPNET::DoYcData(void)
{
	BYTE byYcNum;
	BYTE byBeginNo = 0;
	float wYcValue;
	long temp;
	int i;
	short *pYC_S = (short *)m_dwPubBuf;
	long *pYC_L = (long *)m_dwPubBuf;

    VRegPara* pYcInfo = (VRegPara *)m_pCfg->YCPara.Reg;

	for(i = 0; i < m_RegNow; i++)
	{
	  byBeginNo += pYcInfo->wData.wNum;
	  pYcInfo ++;
	}
	  
	byYcNum = pYcInfo->wData.wNum;
	m_byYCStatus = (*m_pbyData)/byYcNum;   
	m_pbyData ++;

	if (m_byYCStatus == 2)
	{
	   for (i = 0; i < byYcNum; i++)
	   {
			*pYC_S = MAKEWORD(m_pbyData[1], m_pbyData[0]);
			*pYC_S++;
			m_pbyData += 2;
	   }

	   WriteRangeYC(m_wEqpID, byBeginNo, byYcNum, (short *)m_dwPubBuf);
	}
	else if (m_byYCStatus == 4)
	{
	   for (i = 0; i < byYcNum; i++)
	   {
			temp=MAKEDWORD(MAKEWORD(m_pbyData[3], m_pbyData[2]), MAKEWORD(m_pbyData[1], m_pbyData[0]));
			memcpy(&wYcValue,&temp,4);
			*pYC_L =wYcValue*1000;
			*pYC_L++;
			m_pbyData += 4;
	   }

	   WriteRangeYC_L(m_wEqpID, byBeginNo, byYcNum, (long *)m_dwPubBuf);
	}   	
}

void COPENMODBUSPNET::DoYxData(void)
{
    BYTE byYxNum;
	int bytenum, i;
	BYTE byBeginNo = 0;
	BYTE *pbyByte = (BYTE *)m_dwPubBuf;
	VRegPara* pYxInfo = (VRegPara *)m_pCfg->YXPara.Reg;
    
    for (i = 0; i < m_RegNow; i++)
    {
      byBeginNo += pYxInfo->wData.wNum;
	  pYxInfo ++;
    }

	byYxNum = pYxInfo->wData.wNum;
	bytenum = (byYxNum+7)/8;
	m_byYXStatus = (*m_pbyData)/bytenum;   
    m_pbyData++;

 	if (m_byYXStatus == 4)
 	{
		for (i = 0; i < byYxNum; i++)
		{
			*pbyByte++ = m_pbyData[3];
			*pbyByte++ = m_pbyData[2];
			*pbyByte++ = m_pbyData[1];
			*pbyByte++ = m_pbyData[0];
			m_pbyData += 4;				  
		}
 	}  
 	else if (m_byYXStatus == 2)
 	{
		for (i = 0; i < byYxNum; i++)
	    {
	        *pbyByte++ = m_pbyData[1];
		    *pbyByte++ = m_pbyData[0];
		    m_pbyData += 2;		   		  
	  	}
 	}  
	else if (m_byYXStatus == 1)
	{
        for (i = 0; i<byYxNum; i++)
	      *pbyByte++ = *m_pbyData++;
	}
	else
		return;
   
   WriteRangeSYXBit(m_wEqpID, byBeginNo, byYxNum, (BYTE *)m_dwPubBuf);
}

void COPENMODBUSPNET::DoDdData(void)
{
	DWORD *pdwDword = (DWORD *)m_dwPubBuf;
	BYTE byDdNum, byDdUnit;
	WORD lowword,highword;
	int i;
    BYTE byBeginNo = 0;
    VRegPara* pDDInfo = (VRegPara *)m_pCfg->DDPara.Reg;

    for(i = 0; i < m_RegNow; i++)
    {
		byBeginNo += pDDInfo->wData.wNum;
		pDDInfo ++;
    }
 
    m_byDDStatus = pDDInfo->wRegNum / pDDInfo->wData.wNum / 2;
    byDdUnit = m_byDDStatus * 4;
    byDdNum = (*m_pbyData)/byDdUnit;
    m_pbyData ++;
  	
	for (i = 0; i < byDdNum; i ++)
	{
		highword = MAKEWORD(m_pbyData[1],m_pbyData[0]);
		lowword = MAKEWORD(m_pbyData[3],m_pbyData[2]);
		*pdwDword = MAKEDWORD(lowword, highword);
		pdwDword ++;
		m_pbyData += 4;
	}

	WriteRangeDD(m_wEqpID, byBeginNo, byDdNum, (long *)m_dwPubBuf);
	
}
//lw add 2016-03-15 16:46:21
BOOL COPENMODBUSPNET::SendYT(void)
{
	VYTInfo *pYTInfo;
//	VRegPara *pYTPara;

	
	if(m_pCfg->byModbusCfg & YTENABLE)
	{
//		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "YTENABLE");
       if (SwitchToEqpFlag(EQPFLAG_YTReq))
       {     
	        ClearEqpFlag(EQPFLAG_YTReq);/*立刻清标志*/
			//pYTInfo->Info.wID=ID;
			//pYTInfo->Info.dwValue=Value;
	        pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);

		 // pYTPara = (VRegPara *)(m_pCfg->YTPara.Reg)+(pYTInfo->Info.wID-1); 
			
	        switch (pYTInfo->Head.byMsgID & 0x7f)
	        {
				case MI_YTSELECT:  
					YtResult(TRUE);
					return(FALSE); 
					
				case MI_YTOPRATE: 								
					YTRegAddr = searchAddr(pYTInfo->Info.wID);  
					//WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "YTRegAddr:%d",YTRegAddr);
					YTSet = pYTInfo->Info.dwValue;   
					YTSetValue=pYTInfo->Info.dwValue;
					if(m_byNextReq==REQ_YC || m_byNextReq==REQ_YX)
						ytykflag = m_byNextReq;
					m_byNextReq = REQ_YT;
					//m_byReqDataType = REQ_YT;
					//WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "YTSetValue:%d",YTSetValue);
					break;
					
				case MI_YTCANCEL:
					YtResult(TRUE);
					return FALSE;

				}
				return TRUE;    
		   }
  }
	
	return  FALSE;
}

WORD COPENMODBUSPNET::searchAddr(WORD YTno)
{
	//WORD Addr;
	switch (YTno)
	{
		case 1:
			return 0x780;
		case 2:
			return 0x781;
		case 3:
			return 0x782;
		case 4:
			return 0x783;
		case 5:
			return 0x7d2;
		case 6:
			return 0x7d3;
		case 7:
			return 0x900;
		case 8:
			return 0x901;
		case 9:
			return 0x902;
		case 10:
			return 0x903;	
		case 11:
			return 0x952;	
		case 12:
			return 0x953;	
		case 13:
			return 0xb00;	
		case 14:
			return 0xb02;			
		case 15:
			return 0xb04;
		case 16:
			return 0xb06;
		case 17:
			return 0xb20;
		case 18:
			return 0xb22;
		case 19:
			return 0xb24;
		case 20:
			return 0xb26;
		case 21:
			return 0x770;
	}
	return  0;
}


#endif

