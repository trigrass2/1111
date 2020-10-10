/*------------------------------------------------------------------------
 $Rev: 2 $
 编写分合闸代码，从规约，虚装置
------------------------------------------------------------------------*/
#include "syscfg.h"

#ifdef INCLUDE_HMI400

#include "os.h"
#include "extmmidef.h"
#include "exthmi400.h"

extern DWORD LedLight;
extern WORD pannelYx;

extern "C" void hmi400(WORD wTaskID)             
{
	CHMI400 *phmi400 = new CHMI400();    

	if (phmi400->Init(wTaskID) != TRUE)
	{
		phmi400->ProtocolExit();
	}

	phmi400->Run();
}

CHMI400::CHMI400() : CPSecondary()
{
	m_byCnt = 0;
	m_bIn = FALSE;
	commcunt = 0;
}

BOOL CHMI400::Init(WORD wTaskID)
{
	BOOL rc;
	rc = CPSecondary::Init(wTaskID,1,NULL,0);   

	m_dwCommCtrlTimer = 20;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &m_dwCommCtrlTimer);

	m_disp_model = EXTHMI400_MODEL_DISP;
	
	return rc;    
}

DWORD CHMI400::SearchOneFrame(BYTE *Buf, WORD Len)
{
	VEXTHMI400FrmHead *pHead;
	int crclen;
	WORD crc,crc1;

	if (Len < sizeof(VEXTHMI400FrmHead))  return(PCOL_FRAME_LESS);
		
	pHead = (VEXTHMI400FrmHead *)Buf;

	if ((pHead->byCode1 != EXTHMI400_CODE1) || (pHead->byCode2 != EXTHMI400_CODE2))  return(PCOL_FRAME_ERR|1);    

	if (pHead->byLen > EXTHMI400_MAXFRM_LEN) return(PCOL_FRAME_ERR|1);

	crclen = EXTHMI400_CRCLEN;

	if (Len < (pHead->byLen+crclen)) return(PCOL_FRAME_LESS);

	crc = Buf[pHead->byLen]<<8|Buf[pHead->byLen+1];

	crc1 = Crc16(0xFFFF, Buf, pHead->byLen);

	if (crc != crc1) 
	{
	   logMsg("crc error %x, %x\n",crc,crc1,0,0,0,0);
	   return(PCOL_FRAME_ERR|1);
	}

	return(PCOL_FRAME_OK|(pHead->byLen+crclen));    
}

void CHMI400::SendFrameHead(BYTE fmType, BYTE fmFun, BYTE fmCtrl)
{
	m_pSend = (VEXTHMI400FrmHead *)m_SendBuf.pBuf;      
    m_SendBuf.wReadPtr = 0;

	m_pSend->byCode1 = EXTHMI400_CODE1;
	m_pSend->byCode2 = EXTHMI400_CODE2;

    if (fmCtrl & EXTHMI400_CTRL_MASTER)    
		m_pSend->byCnt = ++m_byCnt;
	else
		m_pSend->byCnt = m_pRec->byCnt; 
	
	m_pSend->byFun = fmFun;
	m_pSend->byType = fmType;
	m_pSend->byCtrl = fmCtrl;

    m_SendBuf.wWritePtr = sizeof(VEXTHMI400FrmHead);
}

void CHMI400::SendFrameTail()
{  
	WORD crcCode;
	
    m_pSend->byLen = m_SendBuf.wWritePtr;
	
	crcCode = Crc16(0xFFFF,m_SendBuf.pBuf,m_SendBuf.wWritePtr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(crcCode);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(crcCode);
		
	WriteToComm(m_EqpGroupInfo.wDesAddr);
}  

void CHMI400::SendConf(BYTE byType,BYTE code,BYTE DispAddr)
{
    SendFrameHead(byType, EXTHMI400_FUN_CONF, 0|(DispAddr<<6));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = code;
	SendFrameTail();
}

void CHMI400::SendConnect(void)
{
	int len;
    SendFrameHead(EXTHMI400_TYPE_CONNECT, EXTHMI400_FUN_CTRL, EXTHMI400_CTRL_MASTER);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x5A;
	len = strlen(g_Sys.InPara[SELFPARA_MFR_GB2312]);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = len;
	memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],&g_Sys.InPara[SELFPARA_MFR_GB2312][0],len);
	m_SendBuf.wWritePtr+=len;
	SendFrameTail();
}

WORD CHMI400::SendLight(DWORD light)
{
    if (!m_bIn) return 0;
	SendFrameHead(EXTHMI400_TYPE_LIGHT, EXTHMI400_FUN_WRITE, EXTHMI400_CTRL_MASTER);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(light));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(light));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(light));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(light));
	
	SendFrameTail();
	return m_SendBuf.wWritePtr;
}

//by lvyi,带上地址DispAddr
void CHMI400::SendLightMd(BYTE DispAddr)
{
    int i,num,j;
	BYTE buf[128];
	BYTE LED0=0;
	WORD wSendNum;
	BYTE YXValue;
	
    if (!m_bIn) return;

	if((DispAddr + 1)>m_wEqpNum)
	{
		return;
	}
	
	memset(buf, 0, 128);
	//by lvyi
	if((LedLight & BSP_LIGHT_COMM_ID) == BSP_LIGHT_COMM_ID)
	  	LED0 |= 0x02;
	  else
	  	LED0 &= ~0x02;

	  if((LedLight & BSP_LIGHT_WARN_ID) == BSP_LIGHT_WARN_ID)
	  	LED0 |= 0x01;
	  else
	  	LED0 &= ~0x01;

		i = 0;
		for(j = 0; j < m_pEqpInfo[DispAddr].wSYXNum; j++)
		{
			ReadSYXSendNo(m_pwEqpID[DispAddr],j,&wSendNum); //发送序号一样 多走一遍而已
			if(ReadRangeSYX(m_pwEqpID[DispAddr], j, 1, 1, &YXValue) != 1)
				break;
			i = wSendNum+1;
			if(YXValue & 0x80)
				buf[wSendNum/8] |= 1 << (wSendNum%8);
			else
				buf[wSendNum/8] &= ~(1 << (wSendNum%8));
		}
		
		num = i/8;
		if (i%8) num++;

        SendFrameHead(EXTHMI400_TYPE_LIGHTMD, EXTHMI400_FUN_WRITE, EXTHMI400_CTRL_MASTER|(DispAddr<<6));
        if(m_disp_model == EXTHMI400_MODEL_LAMP)
        {
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = num+2;//by lijun
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LED0;// by lijun
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = i; //by lijun
        }
        else
        {
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = num+1;//by lvyi
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LED0;// by lvyi
        }
        for (i=0; i<num; i++)
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(buf[i]);
        SendFrameTail();	
}

BOOL CHMI400::DoReceive(void)
{
	if (SearchFrame() == FALSE)  
		return (FALSE); 
	
	m_pRec = (VEXTHMI400FrmHead *)m_RecFrame.pBuf;

	//by lvyi,只有在接收到地址为0的面板发来的数据时，才把计数清0
	//这样面板0掉线就会重连，兼容老面板
	if((m_pRec->byCtrl&EXTHMI400_CTRL_Addr) == 0)
	{
		commcunt=0;
	}

	if(((m_pRec->byCtrl&EXTHMI400_CTRL_MASTER) == 0) && (m_byCnt != m_pRec->byCnt)) 
	{
		logMsg("error%d, %d\n",m_byCnt,m_pRec->byCnt,0,0,0,0);
		return 0;
	}

	switch (m_pRec->byType)
	{
		case EXTHMI400_TYPE_CONNECT:
			ProcConnect();
		break;
		case EXTHMI400_TYPE_YX:
			ProcYx();
		break;
		case EXTHMI400_TYPE_YKMD:
			ProcYkMd();
		break;
		default:
		break;
	}
	
	m_dwCommCtrlTimer = 20;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &m_dwCommCtrlTimer);
	return(TRUE); 
}

void CHMI400::ProcConnect(void)
{
    BYTE *pAck;
	pAck = (BYTE*)(m_pRec+1);
    if ((m_pRec->byCnt == m_byCnt)&&(*pAck == EXTHMI400_ACK_OK))
    {
		m_bIn = TRUE;
		if (((m_pRec->byCtrl)&0x3f )== 2)
		{
			m_disp_model = EXTHMI400_MODEL_NODISP;
			SendLightMd(0);//by lvyi 
		}
		else if(((m_pRec->byCtrl)&0x3f )== 4)
		{
			m_disp_model = EXTHMI400_MODEL_LAMP;
			SendLightMd(0);
		}
		else
		{		   
			SendLight(LedLight); 
			m_disp_model = EXTHMI400_MODEL_DISP;
		}
    }
	else
	{
		m_dwCommCtrlTimer = 5;
		commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &m_dwCommCtrlTimer);
	}
}

//只处理远方本地
void CHMI400:: ProcYx(void)
{
     DWORD yx;
	 BYTE *pData = (BYTE*)(m_pRec+1);
	//by lvyi,先判地址,只处理地址为0的遥信,至于灯板,默认为0，兼容
	 if((m_pRec->byCtrl&EXTHMI400_CTRL_Addr) == 0)
	 {		
		yx = MAKEDWORD(MAKEWORD(*pData, *(pData+1)), MAKEWORD(*(pData+2), *(pData+3)));

		if(yx & EXTMMI_YX_YuK)      //by lvyi,预控置位
		{
			yk_flag = 1;
		}
		else
		{
			yk_flag = 0;
		}
		//分合闸灯板处理本地远方
		if(m_disp_model == EXTHMI400_MODEL_NODISP)
		{
			if (!(yx & EXTMMI_YX_LOCAL))
			{
				pannelYx |= 0x01;
				pannelYx &= ~0x02;
			}
			else
			{
				pannelYx &= ~0x01;
				pannelYx |= 0x02;
			}

			if (yx & EXTMMI_YX_BS)
			{
				pannelYx &= ~0x3;
				pannelYx |= 0x04;
			}
		}

		if (m_pRec->byCtrl & EXTHMI400_CTRL_MASTER)
		  SendConf(EXTHMI400_TYPE_YX, EXTHMI400_ACK_OK,0);
    }
}


void CHMI400::ProcYkMd(void)
{
    int no;
	VYKInfo *pYKInfo;
	VDBYK *pDBYK;
	BYTE addr = 0;
    BYTE *pNo = (BYTE*)(m_pRec+1);

	no = *pNo/2;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	pDBYK = &pYKInfo->Info;
	
	if(m_pRec->byCtrl&EXTHMI400_CTRL_Addr)
		addr = 1;
		
	if(yk_flag == 1)
	{
		m_dwSendAllDataEqpNo = addr;
		SwitchToEqpNo(m_dwSendAllDataEqpNo);
		
		if (no >= m_pEqpInfo[addr].wYKNum)
			SendConf(EXTHMI400_TYPE_YKMD, EXTHMI400_ACK_ERR,addr);
		
		pYKInfo->Head.wEqpID = m_pwEqpID[addr];
		pYKInfo->Head.byMsgID = MI_YKSELECT;
		pDBYK->wID = no+1;
		if ( *pNo & 0x01 )
			pDBYK->byValue = 0x6; //分闸
		else
			pDBYK->byValue = 0x5; //合闸
	
		m_YKmsg.wID = pDBYK->wID;
		m_YKmsg.byValue = pDBYK->byValue & 0x0f;

		CPSecondary::DoYKReq();
		
		SendConf(EXTHMI400_TYPE_YKMD, EXTHMI400_ACK_OK,addr);
	}
	else
	{//防止地址0的面板预控没按下，而地址1的面板YK按下；
	//此时要发送错误帧，在面板程序里把temp_yk清掉
		SendConf(EXTHMI400_TYPE_YKMD, EXTHMI400_ACK_ERR,addr);
	}	
}


//100ms进一次，300ms 进一次 发灯
void CHMI400::DoTimeOut(void)
{
	CPSecondary::DoTimeOut();
	commcunt++;
	if ((commcunt >= EXTHMI400_TIMEOUT) && m_bIn)
	{
		WriteWarnEvent(NULL, SYS_ERR_COMM, 0, "HMI400板通讯异常");
		m_bIn = FALSE;
		commcunt = 0;
	}
	else
	{
		if(g_Sys.dwErrCode&SYS_ERR_COMM)
		SysClearErr(SYS_ERR_COMM);
	}
}

void CHMI400::DoCommSendIdle(void)
{	
	//初始化 第一帧
	if (!m_bIn)
	{
		SendConnect();
		m_dwCommCtrlTimer = 100;
		commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &m_dwCommCtrlTimer);
		return;
	}
	SwitchToEqpNo(m_dwSendAllDataEqpNo);
	SendLightMd(m_dwSendAllDataEqpNo);   //增加切换装置
	
	m_dwSendAllDataEqpNo++;
	if ( m_dwSendAllDataEqpNo >= m_wEqpNum)
	{
		m_dwSendAllDataEqpNo = 0;
	}
}

int CHMI400::atOnceProcSSOE(WORD wEqpNo)
{
	shellprintf("收到变位  \n");
	//485防止收发冲突，加延时
	m_dwSendAllDataEqpNo = wEqpNo;
	m_dwCommCtrlTimer = 10;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &m_dwCommCtrlTimer);
	return 0;	
}

BOOL CHMI400::DoYKRet(void)
{
	CPSecondary::DoYKRet();
	if (CheckClearEqpFlag(EQPFLAG_YKRet))
	{
		procYKMsg();
		return TRUE;
	}
	else
	    return FALSE;
}

void CHMI400::procYKMsg(void)
{
	VYKInfo *pYKInfo;
	VDBYK *pDBYK; 	
	BYTE addr = 0;
	
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	pDBYK = &pYKInfo->Info;
	
	if (m_disp_model == EXTHMI400_MODEL_NODISP)
	{		
		if(m_pRec->byCtrl&EXTHMI400_CTRL_Addr)
			addr = 1;
	    switch (pYKInfo->Head.byMsgID)
	    {
	    	case MI_YKSELECT:
		    	if ((pDBYK->byStatus == 0)&&(m_YKmsg.wID == pDBYK->wID)&&(m_YKmsg.byValue == pDBYK->byValue & 0x0F))
		    	{	
					pYKInfo->Head.wEqpID = m_pwEqpID[addr];
					pYKInfo->Head.byMsgID = MI_YKOPRATE;
					CPSecondary::DoYKReq();
		    	}
	     	 break;
			 default:
			 break;
	    }
		return;
	}
}

#endif

