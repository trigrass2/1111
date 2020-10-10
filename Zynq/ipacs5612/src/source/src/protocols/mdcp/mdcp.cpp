#include "syscfg.h"

#ifdef INCLUDE_MDCP
	
#include "sys.h"

extern int fdNum;
extern "C" int FaSimWriteSet(int fd, int flag, BYTE *pSet);
extern "C" int FaSimGetPara(int fd, int flag,BYTE *pBuf);

extern "C" int SetSysAddr(WORD *addr, char *ip1, char *ip2, char *gateway1, char *gateway2);
extern void YkFaOn(int val);

#include "mdcp.h"

VSimData	*g_pSimData;
VSimInfo		g_SimInfo;
BYTE		g_FeederNum;
BYTE		g_FeederIndexCurrent;
DWORD		g_dwDataTime;
BYTE		g_bySimStatus;
VFaYkInfo	g_FaYkInfo;

static DWORD 	g_SimDataSem;

BYTE 		g_bySchemeNo;


VProtectPara	g_ProtectPara[SWITCHER_NUM];

void SimInit(void)
{
	g_pSimData = (VSimData *)malloc(sizeof(VSimData) * SWITCHER_NUM);
	g_SimInfo.pSimRunStatus = (VSimRunStatus *)malloc(sizeof(VSimRunStatus) * SWITCHER_NUM);

	memset(g_pSimData, 0, sizeof(VSimData) * SWITCHER_NUM);
	memset(g_SimInfo.pSimRunStatus, 0, sizeof(VSimRunStatus) * SWITCHER_NUM);

	g_FeederIndexCurrent = 0;
	g_FeederNum = fdNum;

	g_dwDataTime = 0;

	g_bySimStatus = 0;

	g_bySchemeNo = 0;

	g_SimDataSem = smMCreate();
	

	for (int i = 0; i < SWITCHER_NUM; i++)
	{
		g_ProtectPara[i].pProtectPara = malloc(64);
		memset(g_ProtectPara[i].pProtectPara, 0, 64);
	}

	g_FaYkInfo.YkFlag = 0;
		
	return;
}

void SimStart(void)
{
	g_SimInfo.SimStatus = SIM_START;
	return;
}

void SimEnd(void)
{
	g_SimInfo.SimStatus = SIM_END;
	return;
}

void SetRunMode(DWORD SwID, BYTE RunMode)
{
	BYTE FeederNo;

	FeederNo = (BYTE)(SwID & 0xFF) - 1;

	if (FeederNo >= SWITCHER_NUM)
	{
		FeederNo = 0;
	}

	g_SimInfo.pSimRunStatus[FeederNo].RunMode = RunMode;

	return;
}

BYTE GetRunMode(DWORD SwID)
{
	BYTE RunMode = 0xff;	//invalid
	BYTE FeederNo;

	FeederNo = (BYTE)(SwID & 0xFF) - 1;

	if (FeederNo >= SWITCHER_NUM)
	{
		FeederNo = 0;
	}

	RunMode = g_SimInfo.pSimRunStatus[FeederNo].RunMode;


	return RunMode;
}

void SetRunStatus(DWORD SwID, VSimInfo* SimInfo)
{
	BYTE FeederNo;

	FeederNo = (BYTE)(SwID & 0xFF) - 1;

	if (FeederNo >= SWITCHER_NUM)
	{
		FeederNo = 0;
	}

	g_SimInfo.SimStatus = SimInfo->SimStatus;
	memcpy((void *)&g_SimInfo.pSimRunStatus[FeederNo], (void *)&SimInfo->pSimRunStatus[FeederNo], sizeof(VSimRunStatus));

	return;
}

void GetRunStatus(DWORD SwID, VSimInfo *SimInfo)
{
	BYTE FeederNo;

	FeederNo = (BYTE)(SwID & 0xFF) - 1;

	if (FeederNo >= SWITCHER_NUM)
	{
		FeederNo = 0;
	}

	SimInfo->SimStatus = g_SimInfo.SimStatus;
	memcpy((void *)&SimInfo->pSimRunStatus[FeederNo], (void *)&g_SimInfo.pSimRunStatus[FeederNo], sizeof(VSimRunStatus));
	
	return;
}

void WriteSimData(DWORD SwID, void *SimData)
{
	BYTE FeederNo;

	FeederNo = (BYTE)(SwID & 0xFF) - 1;

	if (FeederNo >= SWITCHER_NUM)
	{
		FeederNo = 0;
	}

	smMTake(g_SimDataSem);
	memcpy((void *)&g_pSimData[FeederNo], SimData, sizeof(VSimData));
	smMGive(g_SimDataSem);
	return;
}

BOOL  ReadSimData(DWORD SwID, void *SimData)
{
	BYTE FeederNo;

	if (g_bySimStatus != SIM_START)
	{
		return FALSE;
	}
	FeederNo = (BYTE)(SwID & 0xFF) ;
	if (FeederNo >= SWITCHER_NUM)
	{
		FeederNo = 0;
	}
	smMTake(g_SimDataSem);
	memcpy(SimData, (void *)&(g_pSimData[FeederNo].YcVal[0]), 25);//sizeof(VSimData)); sizeof(VSimData) over the edge
	smMGive(g_SimDataSem);
	return TRUE;
}

int FaYkOperate(int YkID, int YkVal)
{
	int ret = 0;
	if( g_SimInfo.SimStatus != SIM_START )   return -1;
	if( YkVal )
		shellprintf("FA_S close!\n");
	else
		shellprintf(" FA_S trip!\n");
	g_FaYkInfo.YkFlag = 1;
	g_FaYkInfo.YkID = YkID;
	g_FaYkInfo.YkVal = YkVal;
	return ret;
}


int FaYkSelect(int YkID, int YkVal)
{
	if( g_SimInfo.SimStatus != SIM_START )   return -1;
	return 0;
}


extern "C" void mdcp(WORD wTaskID)		//任务入口函数 mdcp
{
	CMDCP *pMDCP = new CMDCP(); 	
	
	if (pMDCP->Init(wTaskID) != TRUE)
 	{
		pMDCP->ProtocolExit();
	}
	pMDCP->Run();			   
}


CMDCP::CMDCP() : CPSecondary()
{

	return;
}

BOOL CMDCP:: Init(WORD wTaskID)
{
	BOOL rc;
    DWORD event_time;
	rc = CPSecondary::Init(wTaskID, 0, NULL, 0);
	if (!rc)
	{
		return FALSE;
	}
	m_byCurrentProProMode = 0;
	SimInit();
	InitStateSet();
       event_time=10;
      commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);	return TRUE;
    
    
	
}

void CMDCP::InitStateSet(void)
{
	BYTE RunModel, Modal, SwitcherNum;
	DWORD SwID;
	BYTE FAModel, ProtectModel;

	RunModel = 0;	//real 
	Modal = 0x00;		//led status	???
	SwitcherNum = 1;	//ftu has one switcher

	g_SimInfo.SimStatus = RunModel;
	g_SimInfo.Modal = Modal;
	g_SimInfo.SwNum = SwitcherNum;
	g_FeederNum = SwitcherNum;

	if (g_SimInfo.SimStatus == 1)
	{
		g_bySimStatus = SIM_START;
	}
	else
	{
		g_bySimStatus = SIM_END;
	}

	SwID = 0xA8050101;		//192.168.5.1 : 1
	FAModel = 0;				//nothing
	ProtectModel = 0;			//nothing

	//to do state set
	g_SimInfo.pSimRunStatus[0].SwID = SwID;
	g_SimInfo.pSimRunStatus[0].FAMode = FAModel;
	g_SimInfo.pSimRunStatus[0].ProtectMode = ProtectModel;
	return;
}


BOOL CMDCP::DoReceive()
{
	if (SearchFrame() != TRUE)
		return FALSE;	

	RecFrame();
	
	return FALSE;
}


DWORD CMDCP::SearchOneFrame(BYTE *Buf, WORD Len)
{
	WORD FrameLen;
	
	if (Len < 5)
		return FRAME_LESS;

	m_pRecFrame = (VFrameHead *)Buf;
	if (m_pRecFrame->StartCode == START_CODE)
	{
		FrameLen = m_pRecFrame->Length + 3;
	 
		if (FrameLen > Len)
			return FRAME_LESS;
		
		if (Buf[FrameLen-1] != END_CODE)
			return FRAME_ERR|1;
		if (Buf[FrameLen-2] != (BYTE)ChkSum((BYTE *)&m_pRecFrame->Length, m_pRecFrame->Length))
			return FRAME_ERR|1;
		return FRAME_OK | FrameLen;
	}
	else
	{
		return FRAME_ERR | 1;
	}
}

void CMDCP::RecFrame(void)
{
	switch(m_pRecFrame->TypeId)
	{
		case FUN_01_RunStateSet:
			Rec_01_StateSet();
			Send_A1_ACK(FUN_01_RunStateSet);
			if (g_SimInfo.SimStatus == 0)	//reset device ip address according to the scheme serial number
			{
				if ((g_bySchemeNo >= 1) && (g_bySchemeNo <= 8))
				{
					//thSleep(300);	// 3  seconds
		    			//sysRestart(COLDRESTART, SYS_RESET_REMOTE, GetThName(m_wTaskID)); 
				}
				
			}
			break;
		case FUN_02_RunStateGet:
			Rec_02_StateGet();
			break;
		case FUN_03_SetValSet:
			Rec_03_SetValSet();
			Send_A1_ACK(FUN_03_SetValSet);
			break;
		case FUN_04_SetValGet:
			Rec_04_SetValGet();
			break;
		case FUN_05_SimStart:
			Rec_05_SimStart();
			Send_A1_ACK(FUN_05_SimStart);
			break;
		case FUN_06_SimDataSet:
			Rec_06_SimDataSet();
			Send_A1_ACK(FUN_06_SimDataSet);
			break;
		case FUN_07_SimYK:
			Rec_07_SimYK();
			Send_A1_ACK(FUN_07_SimYK);
			break;
		case FUN_0A_SimEnd:
			Rec_0A_SimEnd();
			Send_A1_ACK(FUN_0A_SimEnd);
			break;
		case FUN_0C_RealYK:
			Rec_0C_RealYK();
			Send_A1_ACK(FUN_0C_RealYK);
			break;
		case FUN_A0_HeartBeat:
			Send_A1_ACK(FUN_A0_HeartBeat);
			break;
		case FUN_A1_ACK:
			
			break;
		case FUN_A2_NACK:
			
			break;
		#ifdef TEST_FRAME
		case 0x16:
			Rec_16_GetSimData();
			break;
		#endif
		
		case FUN_F0_FileCmd:
			if(Rec_F0_FileCmd() == 0)
				Send_A1_ACK(FUN_F0_FileCmd);
			else
				Send_A2_NACK(FUN_F0_FileCmd);	
			break;			
		case FUN_F1_FileData:
			if(Rec_F1_FileData() == 0)
			{	
			    Send_A1_ACK(FUN_F1_FileData);
                          if(m_cfgfile.bRecBegin == FALSE)
                     {   
                            thSleep(200);//发送成功自动重启装置
                            sysRestart(COLDRESTART, SYS_RESET_REMOTE, GetThName(m_wTaskID)); 
                       }   
                   }
			else
				Send_A2_NACK(FUN_F1_FileData);	
			break;	
            case FUN_F2_FAMS:
                    Rec_F2_FAMS();  
                    Send_A1_ACK(FUN_F2_FAMS);
                    break;
             case FUN_SET_IP:
                    Set_Ip();
                    break;
		default:
			
			break;
	}
	return;
}


void CMDCP::Rec_01_StateSet(void)
{
	BYTE RunModel, Modal, SwitcherNum;
	DWORD SwID;
	BYTE FAModel, ProtectModel;
	BYTE lblw, hblw, lbhw, hbhw;
	BYTE i, datashift = 0;

	char ip[20];
	BYTE ipset[4];

	ipset[0] = 192;
	ipset[1] = 168;
	ipset[2] = 100;
	ipset[3] = 1;

	RunModel = m_pRecFrame->Data[datashift++];
	Modal = m_pRecFrame->Data[datashift++];
	SwitcherNum = m_pRecFrame->Data[datashift++];

	g_SimInfo.SimStatus = RunModel;
	g_SimInfo.Modal = Modal;
	g_SimInfo.SwNum = SwitcherNum;
	g_FeederNum = SwitcherNum;

	if (g_SimInfo.SimStatus == 1)
	{
		g_bySimStatus = SIM_START;
	}
	else
	{
		g_bySimStatus = SIM_END;
	}

	for (i = 0; i < SwitcherNum; i++)
	{
		lblw = m_pRecFrame->Data[datashift++];
		hblw = m_pRecFrame->Data[datashift++];
		lbhw = m_pRecFrame->Data[datashift++];
		hbhw = m_pRecFrame->Data[datashift++];
		SwID = MAKEDWORD(MAKEWORD(lblw, hblw), MAKEWORD(lbhw, hbhw));
		FAModel = m_pRecFrame->Data[datashift++];
		ProtectModel = m_pRecFrame->Data[datashift++];
		//g_bySchemeNo = m_pRecFrame->Data[datashift++];

		//to do state set
		g_SimInfo.pSimRunStatus[i].SwID = SwID;
		g_SimInfo.pSimRunStatus[i].FAMode = FAModel;
		g_SimInfo.pSimRunStatus[i].ProtectMode = ProtectModel;
	}
/*
	if (g_SimInfo.SimStatus == 0)	//reset device ip address according to the scheme serial number
	{
		if ((g_bySchemeNo >= 1) && (g_bySchemeNo <= 8))
		{
			ipset[3] = (g_bySchemeNo - 1) * 10 + HIBYTE(LOWORD(SwID));
			sprintf(ip,"%d.%d.%d.%d", ipset[0], ipset[1], ipset[2], ipset[3]);
			SetSysAddr(NULL,ip,NULL,NULL,NULL) ;
		}
		
	}
*/
	return;
}

void CMDCP::Rec_02_StateGet(void)
{
	BYTE RunModel, Modal, SwitcherNum;
	DWORD SwID;
	BYTE FAModel, ProtectModel;
	BYTE lblw, hblw, lbhw, hbhw;
	BYTE i, datashift = 0;

	RunModel = g_SimInfo.SimStatus;
	Modal = g_SimInfo.Modal;;
	SwitcherNum = g_SimInfo.SwNum;
	
	SendFrameHead(FUN_02_RunStateGet);

	m_pSendFrame->Data[datashift++] = RunModel;
	m_pSendFrame->Data[datashift++] = Modal;
	m_pSendFrame->Data[datashift++] = SwitcherNum;

	for (i = 0; i < SwitcherNum; i++)
	{
		FAModel = g_SimInfo.pSimRunStatus[i].FAMode;
		ProtectModel = g_SimInfo.pSimRunStatus[i].ProtectMode;
		SwID = g_SimInfo.pSimRunStatus[i].SwID;
		lblw = LOBYTE(LOWORD(SwID));
		hblw = HIBYTE(LOWORD(SwID));
		lbhw = LOBYTE(HIWORD(SwID));
		hbhw = HIBYTE(HIWORD(SwID));
		m_pSendFrame->Data[datashift++] = lblw;
		m_pSendFrame->Data[datashift++] = hblw;
		m_pSendFrame->Data[datashift++] = lbhw;
		m_pSendFrame->Data[datashift++] = hbhw;

		m_pSendFrame->Data[datashift++] = FAModel;
		m_pSendFrame->Data[datashift++] = ProtectModel;
		//m_pSendFrame->Data[datashift++] = g_bySchemeNo;
	}
	m_SendBuf.wWritePtr += datashift;
	SendFrameTail();
	return;
}

void CMDCP::Rec_03_SetValSet(void)
{
	DWORD SwID;
	BYTE FeederIndex;
	WORD SetValLength;
	BYTE SwType, FAMode, ProtectMode;
	BYTE lblw, hblw, lbhw, hbhw;
	BYTE datashift = 0;
	BYTE *pBytePtr;
	
	VBreakerProtectPara *pBreakerPrPara;
	VVProtectPara *pVprPara;
	VCCountProtectPara *pCCountPrPara;
       ZSYProtectPara *pZSYprPara;
       DYDLProtectPara *pDYDLPrPara;
        VI0ProtectPara *pVI0prPara;

	lblw = m_pRecFrame->Data[datashift++];
	hblw = m_pRecFrame->Data[datashift++];
	lbhw = m_pRecFrame->Data[datashift++];
	hbhw = m_pRecFrame->Data[datashift++];
	SwID = MAKEDWORD(MAKEWORD(lblw, hblw), MAKEWORD(lbhw, hbhw));
	FeederIndex = lblw - 1;
	if (FeederIndex >= SWITCHER_NUM)
	{
		FeederIndex = 0;
	}
	if(lblw > g_FeederNum)
		g_FeederNum = lblw;

	lblw = m_pRecFrame->Data[datashift++];
	hblw = m_pRecFrame->Data[datashift++];
	SetValLength = MAKEWORD(lblw, hblw);

	SwType = m_pRecFrame->Data[datashift++];
	FAMode = m_pRecFrame->Data[datashift++];
	ProtectMode = m_pRecFrame->Data[datashift++];

	m_byCurrentProProMode = ProtectMode;

	switch(ProtectMode)
	{
	
                case PR_DLQ:
                case PR_FHKG:
                case PR_GLBGZ:
                case PR_DLQCH:
                case PR_CKTZ:
                case PR_CKII:
                case PR_BDZCH:
                case PR_DLJSCH:
                    pBreakerPrPara = (VBreakerProtectPara *)m_setbuf;
                    pBreakerPrPara->OCIYb = m_pRecFrame->Data[datashift++]; //一段过流压板

                    pBytePtr = &m_pRecFrame->Data[datashift];  //一段过流定值
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->OCIDz,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift]; //一段过流时间
                    datashift += 4;
			memcpy((BYTE*)&pBreakerPrPara->OCITime,pBytePtr,4);
							
                    
                    pBreakerPrPara->byIPrYb= m_pRecFrame->Data[datashift++]; //一段出口压板
                    
                    pBreakerPrPara->OpenMode = m_pRecFrame->Data[datashift++];  //过流跳闸条件
                    pBreakerPrPara->ReportMode = m_pRecFrame->Data[datashift++]; //故障上报条件
                    pBreakerPrPara->RCYb = m_pRecFrame->Data[datashift++];//重合闸压板
                    pBreakerPrPara->RCCount = m_pRecFrame->Data[datashift++];//重合次数	

                    pBytePtr = &m_pRecFrame->Data[datashift];//一重时间
                    datashift += 4;
			memcpy((BYTE*)&pBreakerPrPara->RC1Time,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift];//二重时间
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->RC2Time,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift];//三重时间
                    datashift += 4;
			memcpy((BYTE*)&pBreakerPrPara->RC3Time,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift];//重合闭锁时间
                    datashift += 4;
			memcpy((BYTE*)&pBreakerPrPara->RCLockTime,pBytePtr,4);

                    pBreakerPrPara->OCIIYb = m_pRecFrame->Data[datashift++];//二段过流压板
                    
                    pBytePtr = &m_pRecFrame->Data[datashift];
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->OCIIDz,pBytePtr,4);
				
                    pBytePtr = &m_pRecFrame->Data[datashift];//二段时间定值
                    datashift += 4;
			memcpy((BYTE*)&pBreakerPrPara->OCIITime,pBytePtr,4);
					

                    pBreakerPrPara->byIIPrYb= m_pRecFrame->Data[datashift++];//二段出口压板

                    
                    pBreakerPrPara->byGLFx= m_pRecFrame->Data[datashift++]; //过流方向元件
                    pBreakerPrPara->byGLYx= m_pRecFrame->Data[datashift++]; //过流方向遥信
                    
                    pBytePtr = &m_pRecFrame->Data[datashift]; //过流下限角度
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->AngleLow,pBytePtr,4);
                    
                    pBytePtr = &m_pRecFrame->Data[datashift];//过流上限角度
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->AngleUpp,pBytePtr,4);

					pBreakerPrPara->byXDLJD= m_pRecFrame->Data[datashift++]; //小电流接地压板

					pBreakerPrPara->byXDLJD_Trip= m_pRecFrame->Data[datashift++]; //小电流接地出口

					pBytePtr = &m_pRecFrame->Data[datashift];//零压
					
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->XDL_U0,pBytePtr,4);

					pBytePtr = &m_pRecFrame->Data[datashift];//小电流零压突变
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->XDL_U0_diff,pBytePtr,4);

					pBytePtr = &m_pRecFrame->Data[datashift];//小电流零流突变
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->XDL_I0_diff,pBytePtr,4);

					pBytePtr = &m_pRecFrame->Data[datashift];//小电流电流突变
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->XDL_I_diff,pBytePtr,4);

					pBytePtr = &m_pRecFrame->Data[datashift];//小电流接地时间
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->XDL_Time,pBytePtr,4);

					pBytePtr = &m_pRecFrame->Data[datashift];//小电流接地系数
                    datashift += 4;
                    memcpy((BYTE*)&pBreakerPrPara->XDL_Coef,pBytePtr,4);
					
                    FaSimWriteSet(FeederIndex, ProtectMode, (BYTE *) pBreakerPrPara);
                    break;
		case PR_DYZ:
			pVprPara = (VVProtectPara *)m_setbuf;
			pVprPara->SVOpenYb = m_pRecFrame->Data[datashift++];
			pVprPara->LBreakerYb = m_pRecFrame->Data[datashift++];
			pVprPara->SLFeederNo = m_pRecFrame->Data[datashift++];
			
			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;	
			memcpy((BYTE*)&pVprPara->XTime,pBytePtr,4);
		

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pVprPara->YTime,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pVprPara->ZTime,pBytePtr,4);

			pVprPara->XLockYb = m_pRecFrame->Data[datashift++];
			pVprPara->YLockYb = m_pRecFrame->Data[datashift++];
			pVprPara->IVLockYb = m_pRecFrame->Data[datashift++];
			pVprPara->LoseVolYb = m_pRecFrame->Data[datashift++];	
            
                    pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pVprPara->CYTime,pBytePtr,4);
            
			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pVprPara->CYDz,pBytePtr,4);

                    pVprPara->U0Yb = m_pRecFrame->Data[datashift++];	

                    pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pVprPara->U0Dz,pBytePtr,4);

                    pVprPara->FZBS = m_pRecFrame->Data[datashift++];	

                     pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pVprPara->FZBSTime,pBytePtr,4);

			pVprPara->byXDLJD= m_pRecFrame->Data[datashift++]; //小电流接地压板

	    	pVprPara->byXDLJD_Trip= m_pRecFrame->Data[datashift++]; //小电流接地出口

			pBytePtr = &m_pRecFrame->Data[datashift];//零压

			datashift += 4;
			memcpy((BYTE*)&pVprPara->XDL_U0,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流零压突变
			datashift += 4;
			memcpy((BYTE*)&pVprPara->XDL_U0_diff,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流零流突变
			datashift += 4;
			memcpy((BYTE*)&pVprPara->XDL_I0_diff,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流电流突变
			datashift += 4;
			memcpy((BYTE*)&pVprPara->XDL_I_diff,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流接地时间
			datashift += 4;
			memcpy((BYTE*)&pVprPara->XDL_Time,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流接地系数
			datashift += 4;
			memcpy((BYTE*)&pVprPara->XDL_Coef,pBytePtr,4);
            

			FaSimWriteSet(FeederIndex, ProtectMode, (BYTE *) pVprPara);
			break;
		case PR_DLJSX:
			pCCountPrPara = (VCCountProtectPara *)m_setbuf;
			pCCountPrPara->CCountYb = m_pRecFrame->Data[datashift++];
			pCCountPrPara->SLFeederNo = m_pRecFrame->Data[datashift++];
			pCCountPrPara->CCount = m_pRecFrame->Data[datashift++];
			
			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pCCountPrPara->CCountRstTime,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pCCountPrPara->CCountAccTime,pBytePtr,4);

			pCCountPrPara->OCIYb = m_pRecFrame->Data[datashift++];

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pCCountPrPara->OCIDz,pBytePtr,4);
			
			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pCCountPrPara->OCITime,pBytePtr,4);
			
			pCCountPrPara->OpenMode = m_pRecFrame->Data[datashift++];
			pCCountPrPara->ReportMode = m_pRecFrame->Data[datashift++];
			pCCountPrPara->RCYb = m_pRecFrame->Data[datashift++];
			pCCountPrPara->RCCount = m_pRecFrame->Data[datashift++];	

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pCCountPrPara->RC1Time,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pCCountPrPara->RC2Time,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pCCountPrPara->RC3Time,pBytePtr,4);
            

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pCCountPrPara->RCLockTime,pBytePtr,4);

			pCCountPrPara->byIPrYb = m_pRecFrame->Data[datashift++];	

			FaSimWriteSet(FeederIndex, ProtectMode, (BYTE *) pCCountPrPara);
			break;
	case PR_ZSYX:
			pZSYprPara = (ZSYProtectPara *)m_setbuf;
			pZSYprPara->SVOpenYb = m_pRecFrame->Data[datashift++];
			pZSYprPara->LBreakerYb = m_pRecFrame->Data[datashift++];
			pZSYprPara->SLFeederNo = m_pRecFrame->Data[datashift++];
			
			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->XTime,pBytePtr,4);
		

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->YTime,pBytePtr,4);
		

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->ZTime,pBytePtr,4);
		

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
		        memcpy((BYTE*)&pZSYprPara->STime,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->CTime,pBytePtr,4);

			pZSYprPara->XLockYb = m_pRecFrame->Data[datashift++];
			pZSYprPara->YLockYb = m_pRecFrame->Data[datashift++];
			pZSYprPara->IVLockYb = m_pRecFrame->Data[datashift++];
			pZSYprPara->LoseVolYb = m_pRecFrame->Data[datashift++];
			pZSYprPara->OCIYb= m_pRecFrame->Data[datashift++];

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->OCIDz,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->OCITime,pBytePtr,4);

			pZSYprPara->byIPrYb= m_pRecFrame->Data[datashift++];
			pZSYprPara->byxjdl= m_pRecFrame->Data[datashift++];
                    pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->CYTime,pBytePtr,4);
			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->CYDz,pBytePtr,4);

            pZSYprPara->U0Yb = m_pRecFrame->Data[datashift++];	

            pBytePtr = &m_pRecFrame->Data[datashift];
            datashift += 4;
            memcpy((BYTE*)&pZSYprPara->U0Dz,pBytePtr,4);

            
            pZSYprPara->FZBS = m_pRecFrame->Data[datashift++];   
            
            pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->FZBSTime,pBytePtr,4);

			pZSYprPara->byXDLJD= m_pRecFrame->Data[datashift++]; //小电流接地压板

			pZSYprPara->byXDLJD_Trip= m_pRecFrame->Data[datashift++]; //小电流接地出口

			pBytePtr = &m_pRecFrame->Data[datashift];//零压

			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->XDL_U0,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流零压突变
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->XDL_U0_diff,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流零流突变
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->XDL_I0_diff,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流电流突变
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->XDL_I_diff,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流接地时间
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->XDL_Time,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流接地系数
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->XDL_Coef,pBytePtr,4);

			pZSYprPara->bYbZsyjd= m_pRecFrame->Data[datashift++]; //自适应接地压板
			pZSYprPara->bYbDu= m_pRecFrame->Data[datashift++]; //双压闭锁
			pZSYprPara->bYbU0Trip= m_pRecFrame->Data[datashift++]; //零压跳闸

			pBytePtr = &m_pRecFrame->Data[datashift];//零压时间
			datashift += 4;
			memcpy((BYTE*)&pZSYprPara->fTU0,pBytePtr,4);
			
			FaSimWriteSet(FeederIndex, ProtectMode, (BYTE *) pZSYprPara); 
			break;

            case PR_DYDLX:
                    pDYDLPrPara = (DYDLProtectPara *)m_setbuf;
			pDYDLPrPara->SVOpenYb = m_pRecFrame->Data[datashift++];
			pDYDLPrPara->LBreakerYb = m_pRecFrame->Data[datashift++];
			pDYDLPrPara->SLFeederNo = m_pRecFrame->Data[datashift++];
			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->XTime,pBytePtr,4);
			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->YTime,pBytePtr,4);
			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
		       memcpy((BYTE*)&pDYDLPrPara->ZTime,pBytePtr,4);
			pDYDLPrPara->XLockYb = m_pRecFrame->Data[datashift++];
			pDYDLPrPara->YLockYb = m_pRecFrame->Data[datashift++];
			pDYDLPrPara->IVLockYb = m_pRecFrame->Data[datashift++];
			pDYDLPrPara->LoseVolYb = m_pRecFrame->Data[datashift++];	
                    pDYDLPrPara->DYdlYb = m_pRecFrame->Data[datashift++];	
                    pDYDLPrPara->OCIYb= m_pRecFrame->Data[datashift++];
                    
			pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->OCIDz,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->OCITime,pBytePtr,4);
            
                    pDYDLPrPara->byIPrYb= m_pRecFrame->Data[datashift++];

                     pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->CYTime,pBytePtr,4);

                      pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->CYDz,pBytePtr,4);
            
                    pDYDLPrPara->U0Yb = m_pRecFrame->Data[datashift++];	
                    pBytePtr = &m_pRecFrame->Data[datashift];
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->U0Dz,pBytePtr,4);

            pDYDLPrPara->FZBS = m_pRecFrame->Data[datashift++]; 
            pBytePtr = &m_pRecFrame->Data[datashift];
            datashift += 4;
            memcpy((BYTE*)&pDYDLPrPara->FZBSTime,pBytePtr,4);

			pDYDLPrPara->byXDLJD= m_pRecFrame->Data[datashift++]; //小电流接地压板

			pDYDLPrPara->byXDLJD_Trip= m_pRecFrame->Data[datashift++]; //小电流接地出口
			
			pBytePtr = &m_pRecFrame->Data[datashift];//零压
			
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->XDL_U0,pBytePtr,4);
			
			pBytePtr = &m_pRecFrame->Data[datashift];//小电流零压突变
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->XDL_U0_diff,pBytePtr,4);
			
			pBytePtr = &m_pRecFrame->Data[datashift];//小电流零流突变
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->XDL_I0_diff,pBytePtr,4);
			
			pBytePtr = &m_pRecFrame->Data[datashift];//小电流电流突变
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->XDL_I_diff,pBytePtr,4);
			
			pBytePtr = &m_pRecFrame->Data[datashift];//小电流接地时间
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->XDL_Time,pBytePtr,4);
			
			pBytePtr = &m_pRecFrame->Data[datashift];//小电流接地系数
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->XDL_Coef,pBytePtr,4);

			pDYDLPrPara->bYbDu = m_pRecFrame->Data[datashift++];
			pDYDLPrPara->bYbU0Trip = m_pRecFrame->Data[datashift++];
			
			pBytePtr = &m_pRecFrame->Data[datashift];//零压时间
			datashift += 4;
			memcpy((BYTE*)&pDYDLPrPara->fTU0,pBytePtr,4);
			
			FaSimWriteSet(FeederIndex, ProtectMode, (BYTE *) pDYDLPrPara);
			break;
            case PR_CKI0TZ:
                    pVI0prPara = (VI0ProtectPara *)m_setbuf;
                    pVI0prPara->OCIYb = m_pRecFrame->Data[datashift++]; //一段过流压板

                    pBytePtr = &m_pRecFrame->Data[datashift];  //一段过流定值
                    datashift += 4;
                    memcpy((BYTE*)&pVI0prPara->OCIDz,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift]; //一段过流时间
                    datashift += 4;
			memcpy((BYTE*)&pVI0prPara->OCITime,pBytePtr,4);
							
                    
                    pVI0prPara->byIPrYb= m_pRecFrame->Data[datashift++]; //一段出口压板
                    pVI0prPara->OpenMode = m_pRecFrame->Data[datashift++];  //过流跳闸条件
                    pVI0prPara->ReportMode = m_pRecFrame->Data[datashift++]; //故障上报条件
                    pVI0prPara->RCYb = m_pRecFrame->Data[datashift++];//重合闸压板
                    pVI0prPara->RCCount = m_pRecFrame->Data[datashift++];//重合次数	

                    pBytePtr = &m_pRecFrame->Data[datashift];//一重时间
                    datashift += 4;
			memcpy((BYTE*)&pVI0prPara->RC1Time,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift];//二重时间
                    datashift += 4;
                    memcpy((BYTE*)&pVI0prPara->RC2Time,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift];//三重时间
                    datashift += 4;
			memcpy((BYTE*)&pVI0prPara->RC3Time,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift];//重合闭锁时间
                    datashift += 4;
			memcpy((BYTE*)&pVI0prPara->RCLockTime,pBytePtr,4);

                    pVI0prPara->OCIIYb = m_pRecFrame->Data[datashift++];//二段过流压板
                    
                    pBytePtr = &m_pRecFrame->Data[datashift];
                    datashift += 4;
                    memcpy((BYTE*)&pVI0prPara->OCIIDz,pBytePtr,4);
				
                    pBytePtr = &m_pRecFrame->Data[datashift];//二段时间定值
                    datashift += 4;
			memcpy((BYTE*)&pVI0prPara->OCIITime,pBytePtr,4);
					

                    pVI0prPara->byIIPrYb= m_pRecFrame->Data[datashift++];//二段出口压板

                    pVI0prPara->I0Yb= m_pRecFrame->Data[datashift++];//零流一段压板

                    pBytePtr = &m_pRecFrame->Data[datashift];
                    datashift += 4;
                    memcpy((BYTE*)&pVI0prPara->I0IDz,pBytePtr,4);

                    pBytePtr = &m_pRecFrame->Data[datashift];
                    datashift += 4;
                    memcpy((BYTE*)&pVI0prPara->I0ITime,pBytePtr,4);


                    pVI0prPara->byI0PrYb= m_pRecFrame->Data[datashift++];//零流I段出口压板

                    pVI0prPara->byGLFx= m_pRecFrame->Data[datashift++]; //过流方向元件
                    pVI0prPara->byGLYx= m_pRecFrame->Data[datashift++]; //过流方向遥信
                    
                    pBytePtr = &m_pRecFrame->Data[datashift]; //过流下限角度
                    datashift += 4;
                    memcpy((BYTE*)&pVI0prPara->AngleLow,pBytePtr,4);
                    
                    pBytePtr = &m_pRecFrame->Data[datashift];//过流上限角度
                    datashift += 4;
                    memcpy((BYTE*)&pVI0prPara->AngleUpp,pBytePtr,4);

			pVI0prPara->byXDLJD= m_pRecFrame->Data[datashift++]; //小电流接地压板

			pVI0prPara->byXDLJD_Trip= m_pRecFrame->Data[datashift++]; //小电流接地出口

			pBytePtr = &m_pRecFrame->Data[datashift];//零压

			datashift += 4;
			memcpy((BYTE*)&pVI0prPara->XDL_U0,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流零压突变
			datashift += 4;
			memcpy((BYTE*)&pVI0prPara->XDL_U0_diff,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流零流突变
			datashift += 4;
			memcpy((BYTE*)&pVI0prPara->XDL_I0_diff,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流电流突变
			datashift += 4;
			memcpy((BYTE*)&pVI0prPara->XDL_I_diff,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流接地时间
			datashift += 4;
			memcpy((BYTE*)&pVI0prPara->XDL_Time,pBytePtr,4);

			pBytePtr = &m_pRecFrame->Data[datashift];//小电流接地系数
			datashift += 4;
			memcpy((BYTE*)&pVI0prPara->XDL_Coef,pBytePtr,4);
                    
            FaSimWriteSet(FeederIndex, ProtectMode, (BYTE *) pVI0prPara);
            break;
            
	}
			g_ProtectPara[FeederIndex].SwID = SwID;
			g_ProtectPara[FeederIndex].ProParaLength = SetValLength;
			g_ProtectPara[FeederIndex].SwType = SwType;
			g_ProtectPara[FeederIndex].FaMode = FAMode;
			g_ProtectPara[FeederIndex].ProtectMode = ProtectMode;
	
			#ifdef TEST_FRAME
			memcpy(g_ProtectPara[FeederIndex].pProtectPara, (void *)&m_pRecFrame->Data[datashift-1], SetValLength);
			#endif

	return;
}

void CMDCP::Rec_04_SetValGet(void)
{
	DWORD SwID;
	WORD SetValLength;
	BYTE SwType, FAMode, ProtectMode;
	BYTE lblw, hblw, lbhw, hbhw;
	BYTE datashift = 0;
	
	VBreakerProtectPara *pBreakerPrPara;
	VVProtectPara *pVprPara;
	VCCountProtectPara *pCCountPrPara;
	ZSYProtectPara *pZSYprPara;
       DYDLProtectPara *pDYDLPrPara;
       VI0ProtectPara *pVI0prPara;
       
	SendFrameHead(FUN_04_SetValGet);

	SwType = g_ProtectPara[g_FeederIndexCurrent].SwType;
	FAMode = g_ProtectPara[g_FeederIndexCurrent].FaMode;
	ProtectMode = g_ProtectPara[g_FeederIndexCurrent].ProtectMode;
	SetValLength = g_ProtectPara[g_FeederIndexCurrent].ProParaLength;
	SwID = g_ProtectPara[g_FeederIndexCurrent].SwID;

	lblw = LOBYTE(LOWORD(SwID));
	hblw = HIBYTE(LOWORD(SwID));
	lbhw = LOBYTE(HIWORD(SwID));
	hbhw = HIBYTE(HIWORD(SwID));

	m_pSendFrame->Data[datashift++] = lblw;
	m_pSendFrame->Data[datashift++] = hblw;
	m_pSendFrame->Data[datashift++] = lbhw;
	m_pSendFrame->Data[datashift++] = hbhw;

	lblw = LOBYTE(SetValLength);
	hblw = HIBYTE(SetValLength);
	m_pSendFrame->Data[datashift++] = lblw;
	m_pSendFrame->Data[datashift++] = hblw;
	m_pSendFrame->Data[datashift++] = SwType;
	m_pSendFrame->Data[datashift++] = FAMode;
	m_pSendFrame->Data[datashift++] = ProtectMode;

	//need to init the pointers,to do here
	#ifdef TEST_FRAME
	(void *)pBreakerPrPara = g_ProtectPara[g_FeederIndexCurrent].pProtectPara;
	(void *)pLBPrPara = g_ProtectPara[g_FeederIndexCurrent].pProtectPara;
	(void *)pLV2OpenPara = g_ProtectPara[g_FeederIndexCurrent].pProtectPara;
	(void *)pOCReportPara = g_ProtectPara[g_FeederIndexCurrent].pProtectPara;
	(void *)pPortOpenRptPara = g_ProtectPara[g_FeederIndexCurrent].pProtectPara;
	(void *)pPortRCFaultRptPara = g_ProtectPara[g_FeederIndexCurrent].pProtectPara;
	(void *)pVprPara = g_ProtectPara[g_FeederIndexCurrent].pProtectPara;
	(void *)pCCountPrPara = g_ProtectPara[g_FeederIndexCurrent].pProtectPara;
	#endif

	switch(ProtectMode)
	{
		   case PR_DLQ:
                case PR_FHKG:
                case PR_GLBGZ:
                case PR_DLQCH:
                case PR_CKTZ:
                case PR_CKII:
                case PR_BDZCH:
                case PR_DLJSCH:
			pBreakerPrPara = (VBreakerProtectPara *)malloc(sizeof(VBreakerProtectPara));
			FaSimGetPara(g_FeederIndexCurrent, ProtectMode, (BYTE *) pBreakerPrPara); //LOBYTE(LOWORD(SwID)) - 1
			m_pSendFrame->Data[datashift++] = pBreakerPrPara->OCIYb;
            
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->OCIDz, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->OCITime, 4);
			datashift += 4;

                    m_pSendFrame->Data[datashift++] = pBreakerPrPara->byIPrYb;
			
			m_pSendFrame->Data[datashift++] = pBreakerPrPara->OpenMode;
			m_pSendFrame->Data[datashift++] = pBreakerPrPara->ReportMode;
			m_pSendFrame->Data[datashift++] = pBreakerPrPara->RCYb;
			m_pSendFrame->Data[datashift++] = pBreakerPrPara->RCCount;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->RC1Time, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->RC2Time, 4);
			datashift += 4;

                    memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->RC3Time, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->RCLockTime, 4);
			datashift += 4;

                    m_pSendFrame->Data[datashift++] = pBreakerPrPara->OCIIYb;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->OCIIDz, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->OCIITime, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pBreakerPrPara->byIIPrYb;

			m_pSendFrame->Data[datashift++] = pBreakerPrPara->byGLFx;
			m_pSendFrame->Data[datashift++] = pBreakerPrPara->byGLYx;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->AngleLow, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->AngleUpp, 4);
			datashift += 4;              

			m_pSendFrame->Data[datashift++] = pBreakerPrPara->byXDLJD;
			m_pSendFrame->Data[datashift++] = pBreakerPrPara->byXDLJD_Trip;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->XDL_U0, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->XDL_U0_diff, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->XDL_I0_diff, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->XDL_I_diff, 4);
			datashift += 4;              

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->XDL_Time, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pBreakerPrPara->XDL_Coef, 4);
			datashift += 4;              

			break;
		case PR_DYZ:
			pVprPara = (VVProtectPara*)malloc(sizeof(VVProtectPara));
			FaSimGetPara(g_FeederIndexCurrent, ProtectMode, (BYTE *) pVprPara);

			m_pSendFrame->Data[datashift++] = pVprPara->SVOpenYb;
			m_pSendFrame->Data[datashift++] = pVprPara->LBreakerYb;
			m_pSendFrame->Data[datashift++] = pVprPara->SLFeederNo;
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->XTime, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->YTime, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->ZTime, 4);
			datashift += 4;
			
			m_pSendFrame->Data[datashift++] = pVprPara->XLockYb;
			m_pSendFrame->Data[datashift++] = pVprPara->YLockYb;
			m_pSendFrame->Data[datashift++] = pVprPara->IVLockYb;
			m_pSendFrame->Data[datashift++] = pVprPara->LoseVolYb;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->CYTime, 4);
			datashift += 4;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->CYDz, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pVprPara->U0Yb;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->U0Dz, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pVprPara->FZBS;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->FZBSTime, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pVprPara->byXDLJD;
			m_pSendFrame->Data[datashift++] = pVprPara->byXDLJD_Trip;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->XDL_U0, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->XDL_U0_diff, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->XDL_I0_diff, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->XDL_I_diff, 4);
			datashift += 4;              

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->XDL_Time, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVprPara->XDL_Coef, 4);
			datashift += 4;              
			
                    break;
			
		case PR_DLJSX:
			pCCountPrPara = (VCCountProtectPara *)malloc(sizeof(VCCountProtectPara));
			FaSimGetPara(g_FeederIndexCurrent, ProtectMode, (BYTE *) pCCountPrPara);

			m_pSendFrame->Data[datashift++] = pCCountPrPara->CCountYb;
			m_pSendFrame->Data[datashift++] = pCCountPrPara->SLFeederNo;
			m_pSendFrame->Data[datashift++] = pCCountPrPara->CCount;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pCCountPrPara->CCountRstTime, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pCCountPrPara->CCountAccTime, 4);
			datashift += 4;
			
			m_pSendFrame->Data[datashift++] = pCCountPrPara->OCIYb;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pCCountPrPara->OCIDz, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pCCountPrPara->OCITime, 4);
			datashift += 4;
			
			m_pSendFrame->Data[datashift++] = pCCountPrPara->OpenMode;
			m_pSendFrame->Data[datashift++] = pCCountPrPara->ReportMode;
			m_pSendFrame->Data[datashift++] = pCCountPrPara->RCYb;
			m_pSendFrame->Data[datashift++] = pCCountPrPara->RCCount;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pCCountPrPara->RC1Time, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pCCountPrPara->RC2Time, 4);
			datashift += 4;

            memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pCCountPrPara->RC3Time, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pCCountPrPara->RCLockTime, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pCCountPrPara->byIPrYb;

			break;
		case PR_ZSYX:
			pZSYprPara = (ZSYProtectPara*)malloc(sizeof(ZSYProtectPara));//add by lij 增加自适应
			FaSimGetPara(g_FeederIndexCurrent, ProtectMode, (BYTE *) pZSYprPara);

			m_pSendFrame->Data[datashift++] = pZSYprPara->SVOpenYb;
			m_pSendFrame->Data[datashift++] = pZSYprPara->LBreakerYb;
			m_pSendFrame->Data[datashift++] = pZSYprPara->SLFeederNo;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->XTime, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->YTime, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->ZTime, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->STime, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->CTime, 4);
			datashift += 4;		

			m_pSendFrame->Data[datashift++] = pZSYprPara->XLockYb;
			m_pSendFrame->Data[datashift++] = pZSYprPara->YLockYb;
			m_pSendFrame->Data[datashift++] = pZSYprPara->IVLockYb;
			m_pSendFrame->Data[datashift++] = pZSYprPara->LoseVolYb;

			m_pSendFrame->Data[datashift++] = pZSYprPara->OCIYb;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->OCIDz, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->OCITime, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pZSYprPara->byIPrYb;
			m_pSendFrame->Data[datashift++] = pZSYprPara->byxjdl;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->CYTime, 4);
			datashift += 4;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->CYDz, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pZSYprPara->U0Yb;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->U0Dz, 4);
			datashift += 4;


			m_pSendFrame->Data[datashift++] = pZSYprPara->FZBS;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->FZBSTime, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pZSYprPara->byXDLJD;
			m_pSendFrame->Data[datashift++] = pZSYprPara->byXDLJD_Trip;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->XDL_U0, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->XDL_U0_diff, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->XDL_I0_diff, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->XDL_I_diff, 4);
			datashift += 4;              

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->XDL_Time, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->XDL_Coef, 4);
			datashift += 4;      

			m_pSendFrame->Data[datashift++] = pZSYprPara->bYbZsyjd;
			m_pSendFrame->Data[datashift++] = pZSYprPara->bYbDu;
			m_pSendFrame->Data[datashift++] = pZSYprPara->bYbU0Trip;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pZSYprPara->fTU0, 4);
			datashift += 4;  
			break;
            case PR_DYDLX:
			pDYDLPrPara = (DYDLProtectPara*)malloc(sizeof(DYDLProtectPara));
			FaSimGetPara(g_FeederIndexCurrent, ProtectMode, (BYTE *) pDYDLPrPara);
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->SVOpenYb;
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->LBreakerYb;
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->SLFeederNo;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->XTime, 4);
			datashift += 4;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->YTime, 4);
			datashift += 4;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->ZTime, 4);
			datashift += 4;
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->XLockYb;
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->YLockYb;
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->IVLockYb;
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->LoseVolYb;
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->DYdlYb;   //电压电流型压板
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->OCIYb;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->OCIDz, 4);
			datashift += 4;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->OCITime, 4);
			datashift += 4;
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->byIPrYb;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->CYTime, 4);
			datashift += 4;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->CYDz, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pDYDLPrPara->U0Yb;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->U0Dz, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pDYDLPrPara->FZBS;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->FZBSTime, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pDYDLPrPara->byXDLJD;
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->byXDLJD_Trip;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->XDL_U0, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->XDL_U0_diff, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->XDL_I0_diff, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->XDL_I_diff, 4);
			datashift += 4;              

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->XDL_Time, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->XDL_Coef, 4);
			datashift += 4;  
			
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->bYbDu;
			m_pSendFrame->Data[datashift++] = pDYDLPrPara->bYbU0Trip;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pDYDLPrPara->fTU0, 4);
			datashift += 4;  
                    break;

                case PR_CKI0TZ:
                    pVI0prPara = (VI0ProtectPara *)malloc(sizeof(VI0ProtectPara));
			FaSimGetPara(g_FeederIndexCurrent, ProtectMode, (BYTE *) pVI0prPara);
			m_pSendFrame->Data[datashift++] = pVI0prPara->OCIYb;
            
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->OCIDz, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->OCITime, 4);
			datashift += 4;

                    m_pSendFrame->Data[datashift++] = pVI0prPara->byIPrYb;
			
			m_pSendFrame->Data[datashift++] = pVI0prPara->OpenMode;
			m_pSendFrame->Data[datashift++] = pVI0prPara->ReportMode;
			m_pSendFrame->Data[datashift++] = pVI0prPara->RCYb;
			m_pSendFrame->Data[datashift++] = pVI0prPara->RCCount;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->RC1Time, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->RC2Time, 4);
			datashift += 4;

                    memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->RC3Time, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->RCLockTime, 4);
			datashift += 4;

                    m_pSendFrame->Data[datashift++] = pVI0prPara->OCIIYb;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->OCIIDz, 4);
			datashift += 4;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->OCIITime, 4);
			datashift += 4;

                    m_pSendFrame->Data[datashift++] = pVI0prPara->byIIPrYb;

                    m_pSendFrame->Data[datashift++] = pVI0prPara->I0Yb;
                    
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->I0IDz, 4);
			datashift += 4;
            
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->I0ITime, 4);
			datashift += 4;

			m_pSendFrame->Data[datashift++] = pVI0prPara->byI0PrYb;

			m_pSendFrame->Data[datashift++] = pVI0prPara->byGLFx;
			m_pSendFrame->Data[datashift++] = pVI0prPara->byGLYx;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->AngleLow, 4);
			datashift += 4;
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->AngleUpp, 4);
			datashift += 4;
			m_pSendFrame->Data[datashift++] = pVI0prPara->byXDLJD;
			m_pSendFrame->Data[datashift++] = pVI0prPara->byXDLJD_Trip;

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->XDL_U0, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->XDL_U0_diff, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->XDL_I0_diff, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->XDL_I_diff, 4);
			datashift += 4;              

			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->XDL_Time, 4);
			datashift += 4;              
			
			memcpy((void *)&m_pSendFrame->Data[datashift], (void *)&pVI0prPara->XDL_Coef, 4);
			datashift += 4;  
                    break;
	}


	m_SendBuf.wWritePtr += datashift;
	SendFrameTail();

	g_FeederIndexCurrent++;
	if (g_FeederIndexCurrent >= g_FeederNum)
	{
		g_FeederIndexCurrent = 0;
	}
	return;
}

void CMDCP::Rec_05_SimStart(void)
{
	g_bySimStatus = SIM_START;
	g_dwDataTime = 0;
	ResetProtect(0);
	shellprintf( " 收到FTT1300 仿真开始，复位保护。\n");
	return;
}

void CMDCP::Rec_06_SimDataSet(void)
{
	BYTE SwitcherNum, bComm;
	DWORD SwID, DataTime;
	BYTE i, datashift = 0;
	BYTE lblw, hblw, lbhw, hbhw;
	VSysClock SysTime;	
	struct VCalClock Time;
	VSimData recSimData;
	BYTE FeederIndex;

	SwitcherNum = m_pRecFrame->Data[datashift++];

	for (i = 0; i < SwitcherNum; i++)
	{
		lblw = m_pRecFrame->Data[datashift++];
		hblw = m_pRecFrame->Data[datashift++];
		lbhw = m_pRecFrame->Data[datashift++];
		hbhw = m_pRecFrame->Data[datashift++];
		SwID = MAKEDWORD(MAKEWORD(lblw, hblw), MAKEWORD(lbhw, hbhw));
		FeederIndex = lblw - 1;
		if (FeederIndex >= SWITCHER_NUM)
		{
			FeederIndex = 0;
		}
//老的
//		lblw = m_pRecFrame->Data[datashift++];
//		hblw = m_pRecFrame->Data[datashift++];
//		lbhw = m_pRecFrame->Data[datashift++];
//		hbhw = m_pRecFrame->Data[datashift++];
//		DataTime = MAKEDWORD(MAKEWORD(lblw, hblw), MAKEWORD(lbhw, hbhw));
//新的
		lblw = m_pRecFrame->Data[datashift++];
		hblw = m_pRecFrame->Data[datashift++];
		SysTime.wMSecond = MAKEWORD(lblw,hblw)%1000;
		SysTime.bySecond = m_pRecFrame->Data[datashift++] & 0x3f;
		SysTime.byMinute = m_pRecFrame->Data[datashift++] & 0x3f;
		SysTime.byHour = m_pRecFrame->Data[datashift++] & 0x1f;
		SysTime.byDay = m_pRecFrame->Data[datashift++] & 0x1f;
		//SysTime.byWeek = m_pRecFrame->Data[datashift++] ;
		SysTime.byMonth = m_pRecFrame->Data[datashift++] & 0x1f;
		lblw = m_pRecFrame->Data[datashift++];
		hblw = m_pRecFrame->Data[datashift++];
		SysTime.wYear = MAKEWORD(lblw,hblw);
		ToCalClock(&SysTime,&Time);
		DataTime = Time.dwMinute;
		

		recSimData.SwID = SwID;
		recSimData.DataTime = DataTime;
		//deal with yc
		memcpy((BYTE *)recSimData.YcVal, &m_pRecFrame->Data[datashift], sizeof(WORD) * 8);
		datashift += (sizeof(WORD) * 8);
		//deal with yx
		memcpy((BYTE *)recSimData.YxVal, &m_pRecFrame->Data[datashift], sizeof(BYTE) * 8);
		datashift += (sizeof(BYTE) * 8);
		//deal with bComm
		recSimData.bComm = m_pRecFrame->Data[datashift++];

		shellprintf("FTT1300下发电压: %d,  %d,  %d,  %d\n", recSimData.YcVal[0],
													   recSimData.YcVal[1],
													   recSimData.YcVal[2],
													   recSimData.YcVal[3] );
		shellprintf("FTT1300下发电流: %d,  %d,  %d,  %d\n", recSimData.YcVal[4],
													   recSimData.YcVal[5],
													   recSimData.YcVal[6],
													   recSimData.YcVal[7] );
		shellprintf("FTT1300下发遥信: %d,  %d,  %d,  %d\n", recSimData.YxVal[0],
													   recSimData.YxVal[1],
													   recSimData.YxVal[2],
													   recSimData.YxVal[3] );

		//if (recSimData.DataTime >= g_pSimData[FeederIndex]->DataTime)
		{
			WriteSimData(SwID, &recSimData);
		}

	}

	return;
}

void CMDCP::Rec_07_SimYK(void)
{
	DWORD SwID;
	BYTE YkVal;
	BYTE datashift = 0;
	BYTE lblw, hblw, lbhw, hbhw;

	lblw = m_pRecFrame->Data[datashift++];
	hblw = m_pRecFrame->Data[datashift++];
	lbhw = m_pRecFrame->Data[datashift++];
	hbhw = m_pRecFrame->Data[datashift++];
	SwID = MAKEDWORD(MAKEWORD(lblw, hblw), MAKEWORD(lbhw, hbhw));
	
	YkVal = m_pRecFrame->Data[datashift++];
	
	return;
}

void CMDCP::Rec_0A_SimEnd(void)
{
	//call proc
	g_bySimStatus = SIM_END;
	g_dwDataTime = 0;
	ResetProtect(0);
	//ClearSet(m_byCurrentProProMode);
	
	memset(g_pSimData, 0, sizeof(VSimData) * SWITCHER_NUM);
	
	shellprintf( " 收到FTT1300 仿真结束，复位保护。\n");
	
	return;
}

void CMDCP::Rec_0C_RealYK(void)
{
	DWORD SwID;
	BYTE YkVal;
	BYTE datashift = 0;
	BYTE lblw, hblw, lbhw, hbhw;

	lblw = m_pRecFrame->Data[datashift++];
	hblw = m_pRecFrame->Data[datashift++];
	lbhw = m_pRecFrame->Data[datashift++];
	hbhw = m_pRecFrame->Data[datashift++];
	SwID = MAKEDWORD(MAKEWORD(lblw, hblw), MAKEWORD(lbhw, hbhw));
	
	YkVal = m_pRecFrame->Data[datashift++];

	return;
}


int CMDCP::Rec_F0_FileCmd(void)
{
	BYTE datashift = 0;
       int ret = 0;
	BYTE cmd;
	int result;
	FILE *fp;
	cmd = m_pRecFrame->Data[datashift];
	
	switch(cmd)
	{
		case 0x01:
                    ret = remove("/tffsa/cfg/fa.cfg" );
                    Send_A1_ACK(FUN_F0_FileCmd); 
                    if(ret == 0)
                    {
                        thSleep(200);
                        sysRestart(COLDRESTART, SYS_RESET_REMOTE, GetThName(m_wTaskID));
                    }
		break;
		case 0x02:
			 m_cfgfile.wCuroffset = 0;
			 m_cfgfile.wFilelen = 0;
			 m_cfgfile.bRecBegin = true;
			 m_cfgfile.Num = 0;
			 result = 0;	
		  break;
		case 0x03:              // 重启装置
                    Send_A1_ACK(FUN_F0_FileCmd);
                    thSleep(200);
                    sysRestart(COLDRESTART, SYS_RESET_REMOTE, GetThName(m_wTaskID)); 
               break;
		default:
			break;
	}
	return result;
}


int CMDCP::Rec_F1_FileData(void) //2018年6月7日 14:51:32 fa.cfg 不大，直接内存缓存
{
	int ret = 0;
	FILE *fp;
	BYTE datashift = 0;
	BYTE *pdata;
	BYTE lw,hg,datalen;
	WORD filelen ;
	WORD crc,mycrc;
	
	if( !m_cfgfile.bRecBegin )
		return -1;
  
            
	lw = m_pRecFrame->Data[datashift++];
	hg = m_pRecFrame->Data[datashift++];
	filelen = MAKEWORD(lw,hg);
	datalen = m_pRecFrame->Data[datashift++];
	pdata = (BYTE*)&(m_pRecFrame->Data[datashift]);


	if(m_cfgfile.wCuroffset == 0)   //第一包
	{
		m_cfgfile.wCuroffset = datalen;
		memcpy( m_setbuf, pdata, datalen);
	}else
	{
		if(filelen != 0xffff)
		{
			if (m_cfgfile.wCuroffset !=  filelen) //前后两帧传输的偏移不一样
			{
				m_cfgfile.bRecBegin = FALSE;
				m_cfgfile.wCuroffset = 0;
				m_cfgfile.wFilelen = 0;
				m_cfgfile.Num = 0;
				return -1;
			}	
			memcpy( (m_setbuf+ m_cfgfile.wCuroffset)  , pdata, datalen);
			m_cfgfile.wCuroffset += datalen;
		}
		else
		{
			memcpy( (m_setbuf+ m_cfgfile.wCuroffset)  , pdata, datalen);
			m_cfgfile.wCuroffset += datalen;
			m_cfgfile.wFilelen = m_cfgfile.wCuroffset;
			
			mycrc = GetParaCrc16(m_setbuf, m_cfgfile.wFilelen-sizeof(WORD));
			memcpy((BYTE*)&crc,m_setbuf + m_cfgfile.wFilelen - sizeof(WORD),2);
			if(mycrc != crc)   
			{
				shellprintf("mycrc = %x  crc = %x not same \n",mycrc,crc);
				return -1;
			}
			else
			{
				fp = fopen("/tffsa/cfg/fa.cfg","w+");
				if(NULL == fp)
					return -1;	
				ret = fwrite(m_setbuf, sizeof(BYTE), m_cfgfile.wFilelen, fp);
				if(ret != m_cfgfile.wFilelen )
				{
					fclose(fp);
					remove("/tffsa/cfg/fa.cfg" );
					return -1;
				}
				fclose(fp);
				m_cfgfile.bRecBegin = FALSE;  //本次文件传输任务结束
				m_cfgfile.wCuroffset = 0;
				m_cfgfile.wFilelen = 0;
				m_cfgfile.Num = 0;
			}
				
		}
	}
	return 0;
}
void CMDCP::Rec_F2_FAMS(void)
{
        BYTE cmd;
        BYTE datashift = 0;
		BYTE fd;
        
        cmd = m_pRecFrame->Data[datashift];
        switch(cmd)
       {
            case 1:
                YkFaOn(0);//清除FA 投入
                for(fd = 0; fd < fdNum; fd++)
               		ClearYBSet(fd);  //清除开关1所有压板
                break;
            case 2:
                YkFaOn(1); //FA 投入
                for(fd = 0; fd < fdNum; fd++)
                	ClearYBSet(fd);
                break;
             default:
                break;
        }
        return;
}
void CMDCP::Send_07_SimYk(void)
{
	DWORD SwID;
	SendFrameHead(FUN_07_SimYK);
//老的
//	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = g_FaYkInfo.YkID;
//	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;			//???
//	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
//	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
//新的
	SwID = g_ProtectPara[g_FeederIndexCurrent].SwID;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(SwID));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(SwID));			//???
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(SwID));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(SwID));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = g_FaYkInfo.YkVal;
	SendFrameTail();
	return;
}

void CMDCP::Set_Ip(void)
{
        WORD datashift = 0;
        BYTE cmd;
        char ip[2][20];

        cmd = m_pRecFrame->Data[datashift];
        datashift += 1;
       switch(cmd)
       {
            case 0x01:            
                    sprintf(ip[0],"%d.%d.%d.%d",m_pRecFrame->Data[datashift],m_pRecFrame->Data[datashift+1],m_pRecFrame->Data[datashift+2],m_pRecFrame->Data[datashift+3]);
                     SetSysAddr(NULL,ip[0],NULL,NULL,NULL);
                    break;
            case 0x02:
                    sprintf(ip[1],"%d.%d.%d.%d",m_pRecFrame->Data[datashift],m_pRecFrame->Data[datashift+1],m_pRecFrame->Data[datashift+2],m_pRecFrame->Data[datashift+3]);
                    SetSysAddr(NULL,NULL,ip[1],NULL,NULL);
                    break;
            default:
                    break;
       }
       return;
}
void CMDCP::Send_A0_HeartBeat(void)
{
	SendFrameHead(FUN_A0_HeartBeat);
	SendFrameTail();
	return;
}

void CMDCP::Send_A1_ACK(BYTE recfuncode)
{
	SendFrameHead(FUN_A1_ACK);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = recfuncode;
	SendFrameTail();
	return;
}


void CMDCP::Send_A2_NACK(BYTE recfuncode)
{
	SendFrameHead(FUN_A2_NACK);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = recfuncode;
	SendFrameTail();

	return;
}

BOOL CMDCP::SendFrameHead(BYTE FunCode)
{
	m_SendBuf.wWritePtr = 0;
	m_SendBuf.wReadPtr = 0;
	m_pSendFrame = (VFrameHead *)m_SendBuf.pBuf;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = START_CODE;
	m_SendBuf.wWritePtr++;//length
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = FunCode;
	
	return TRUE;
}

BOOL CMDCP::SendFrameTail(void)
{
	m_SendBuf.pBuf[1] = m_SendBuf.wWritePtr - 1;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = ChkSum((BYTE *)(&m_SendBuf.pBuf[1]) , m_SendBuf.pBuf[1]);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = END_CODE;

	WriteToComm(0);
	return TRUE;
}

BYTE CMDCP::ChkSum(BYTE *buf, WORD len)
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

void CMDCP::DoTimeOut100ms(void)
{
	if (g_bySimStatus == SIM_START)
	{
		g_dwDataTime += 100;
	}

	if (g_FaYkInfo.YkFlag)
	{
		g_FaYkInfo.YkFlag = 0;
		Send_07_SimYk();
	}
	return;
}

void CMDCP::DoCommSendIdle(void)
{
    DWORD event_time;
    event_time=10;
    commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
    DoTimeOut100ms();
}

void CMDCP::DoTimeOut(void)
{
    	CPSecondary::DoTimeOut();
	if (TimerOn(10))		//send heartbeat frame per 10 seconds
	{
		Send_A0_HeartBeat();
	}
	
}


#ifdef TEST_FRAME

void CMDCP::Rec_16_GetSimData(void)
{
	BYTE RunModel, Modal, SwitcherNum;
	DWORD SwID, datatime;
	BYTE FAModel, ProtectModel;
	BYTE lblw, hblw, lbhw, hbhw;
	BYTE i, datashift = 0;
	VSimData sendSimData;

	SwitcherNum = g_SimInfo.SwNum;

	
	SendFrameHead(0x16);

	m_pSendFrame->Data[datashift++] = SwitcherNum;

	for (i = 0; i < SwitcherNum; i++)
	{
		ReadSimData(g_pSimData[i].SwID, &sendSimData);
		SwID = g_pSimData[i].SwID;
		lblw = LOBYTE(LOWORD(SwID));
		hblw = HIBYTE(LOWORD(SwID));
		lbhw = LOBYTE(HIWORD(SwID));
		hbhw = HIBYTE(HIWORD(SwID));
		m_pSendFrame->Data[datashift++] = lblw;
		m_pSendFrame->Data[datashift++] = hblw;
		m_pSendFrame->Data[datashift++] = lbhw;
		m_pSendFrame->Data[datashift++] = hbhw;

		datatime = g_pSimData[i].DataTime;
		lblw = LOBYTE(LOWORD(datatime));
		hblw = HIBYTE(LOWORD(datatime));
		lbhw = LOBYTE(HIWORD(datatime));
		hbhw = HIBYTE(HIWORD(datatime));

		m_pSendFrame->Data[datashift++] = lblw;
		m_pSendFrame->Data[datashift++] = hblw;
		m_pSendFrame->Data[datashift++] = lbhw;
		m_pSendFrame->Data[datashift++] = hbhw;

		memcpy(&m_pSendFrame->Data[datashift], sendSimData.YcVal, 16);
		datashift += 16;

		memcpy(&m_pSendFrame->Data[datashift], sendSimData.YxVal, 8);
		datashift += 8;
		
		m_pSendFrame->Data[datashift++] = sendSimData.bComm;
	}
	m_SendBuf.wWritePtr += datashift;
	SendFrameTail();
	
	return;
}

void SetProtectPara(DWORD SwID, VProtectPara* ProtectPara)
{
	BYTE FeederNo;

	FeederNo = (BYTE)(SwID & 0xFF) - 1;

	if (FeederNo >= SWITCHER_NUM)
	{
		FeederNo = 0;
	}

	g_ProtectPara[FeederNo].SwID = ProtectPara->SwID;
	g_ProtectPara[FeederNo].ProParaLength = ProtectPara->ProParaLength;
	g_ProtectPara[FeederNo].SwType = ProtectPara->SwType;
	g_ProtectPara[FeederNo].FaMode = ProtectPara->FaMode;
	g_ProtectPara[FeederNo].ProtectMode = ProtectPara->ProtectMode;
	memcpy((void *)g_ProtectPara[FeederNo].pProtectPara, ProtectPara->pProtectPara, ProtectPara->ProParaLength);

	return;
}

void GetProtectPara(DWORD SwID, VProtectPara* ProtectPara)
{
	BYTE FeederNo;

	FeederNo = (BYTE)(SwID & 0xFF) - 1;

	if (FeederNo >= SWITCHER_NUM)
	{
		FeederNo = 0;
	}

	ProtectPara->SwID = g_ProtectPara[FeederNo].SwID;
	ProtectPara->ProParaLength = g_ProtectPara[FeederNo].ProParaLength;
	ProtectPara->SwType = g_ProtectPara[FeederNo].SwType;
	ProtectPara->FaMode = g_ProtectPara[FeederNo].FaMode;
	ProtectPara->ProtectMode = g_ProtectPara[FeederNo].ProtectMode;
	memcpy(ProtectPara->pProtectPara, (void *)g_ProtectPara[FeederNo].pProtectPara, ProtectPara->ProParaLength);
	return;
}





#endif


#endif
