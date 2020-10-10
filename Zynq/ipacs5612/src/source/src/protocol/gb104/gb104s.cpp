/*------------------------------------------------------------------------
 $Rev: 15 $
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef INCLUDE_GB104_S

#include "sys.h"
#include "gb104s.h"
#ifdef INCLUDE_ECC
#include "ecc.h"
#include "certificate.h"

#ifdef ECC_MODE_CHIP
#include "SecurityProcessor.h"
#include "SecurityProcessorHN.h"
#include "sgc1126a.h"
#endif
#endif

#ifdef INCLUDE_RECORD
#include "record.h"
#endif

#ifdef INCLUDE_GPRS
#include "in331g.h"
extern VGprsChan GprsChan;
extern int SignalNo;
extern int SignalVal;
#endif

#if (DEV_SP == DEV_SP_CTRL)//增加协调控制器

	extern float CRD_ytValue[20][256]; //wdj add 
	extern BOOL GetCRD_ytValue_Flag[20][256];
	extern int ykValue[20];
	
#endif

extern WORD g_ddfifflag, g_dddayflag, g_ddtideflag;
extern WORD DD_START_ADDR,DD_REAL_START_ADDR,DD_FIF_START_ADDR,DD_DAY_START_ADDR,DD_TIDE_START_ADDR;
extern BYTE g_ddhxnum;
#ifdef  _POWEROFF_TEST_VER_
extern "C" int GetOffFlag(void);
extern "C" int GetCurrentFault(BYTE *pBuf, int max);
#endif
#undef  SELF_104

CGB104S *g_GB104S = 0;

extern "C" int ReadGb104sDeadValue(WORD parano,char*pbuf)
{
	if(g_GB104S != NULL)
		g_GB104S->ReadDeadValue( parano, pbuf);
	else
		return ERROR;
	
	return OK;
	
}

extern "C" int WriteGb104sDeadValue(WORD parano,char*pbuf)
{
	if(g_GB104S != NULL)
		g_GB104S->WriteDeadValue( parano, pbuf);
	else
		return ERROR;
	
	return OK;
}

extern "C" int WriteGb104sDeadValueFile()
{
	if(g_GB104S != NULL)
		g_GB104S->WriteDeadValueFile();
	else
		return ERROR;
	return OK;
}
	
extern "C" void gb104s(WORD wTaskID)		//任务入口函数 gb101sec
{
	CGB104S *pGB104S = new CGB104S(); 	
	if(g_GB104S == NULL)
		g_GB104S = pGB104S;
	
	if (pGB104S->Init(wTaskID) != TRUE)
 	{
		pGB104S->ProtocolExit();
	}
	pGB104S->Run();			   
}
CGB104S::CGB104S() : CPSecondary()
{
	m_wSendNum = 0;	//Send Counter	
	m_wRecvNum = 0;	//Receive Counter
	m_wAckNum = 0;		//Receive Ack Counter
	m_wAckRecvNum = 0; //Have Ack Receive Counter
	m_bDTEnable = FALSE;		//Data Transfer Enable Flag
	m_bTcpConnect = FALSE;			//TCP Connect Flag
	m_bContinueSend = TRUE;
	m_vTimer[1].bRun = FALSE;
	m_vTimer[1].wInitVal = PARA_T1;
	m_vTimer[2].bRun = FALSE;
	m_vTimer[2].wInitVal = PARA_T2;
	m_vTimer[3].bRun = FALSE;
	m_vTimer[3].wInitVal = PARA_T3;
	memset(m_vBackFrame, 0xff, sizeof(m_vBackFrame));
	
	m_dwDdSendFlag = 0;
	m_dwYcSendFlag = 0;
	m_dwYxSendFlag = 0;
	
	m_wSendYcNum = 0;
	m_wSendYxNum = 0;
	m_wSendDdNum = 0;
	m_wSendDYxNum=0;
	
	m_dwYcGroupFlag = 0;
	m_dwYxGroupFlag = 0;
	m_dwDdGroupFlag = 0;
	m_byYcYxQrp = 0;
	m_byDdQcc = 0;
	
	m_dwSendAllDataEqpNo = 0;
	m_wRecAddress = 0;
	m_wChangeYcEqpNo = 0;
	m_wAllDdEqpNo = 0;
	m_allcalloverflag=0;
	/*memset(m_UFlagInfo, 0, sizeof(m_UFlagInfo));
	
	m_UFlagInfo[0].dwFlag=DCOSUFLAG  ;
	m_UFlagInfo[1].dwFlag=SCOSUFLAG  ;
	m_UFlagInfo[2].dwFlag=FAINFOUFLAG;
    m_UFlagInfo[3].dwFlag=0  ;
    m_UFlagInfo[4].dwFlag=0  ;*/

}

/***************************************************************
	Function：Init
		规约初始化
	参数：wTaskID
		wTaskID 任务ID 
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CGB104S::Init(WORD wTaskID)
{
	BOOL rc;
	memset(&m_guiyuepara,0,sizeof(m_guiyuepara));
	rc = CPSecondary::Init(wTaskID,1,&m_guiyuepara,sizeof(m_guiyuepara));
	if (!rc)
	{
		return FALSE;
	}
	m_paranum = 0;
	m_readparaflag=0;
	m_sourfaaddr=0;
	m_QRP=0;
	m_allcallflag=0;
	memset(&m_delaysoe, 0, 2*sizeof(VDelaySoe));
	memset(&m_delaycos, 0, 2*sizeof(VDelayCos));
	m_pReceive = (VIec104Frame *)m_RecFrame.pBuf;
	m_pSendsoenum= new VSendsoenum[m_wEqpNum];
	m_init = 0;
	DoCommState();
	m_init = 1;
	m_initflag=1;
	m_retxdflag=0;
	for (int i = 0; i < m_wEqpNum; i++)
	{
		m_pEqpInfo[i].pProtocolData = new VCtrlInfo;
		m_pSendsoenum[i].wSendSSOENum=0;
		m_pSendsoenum[i].wSendDSOENum=0;
	}
	initpara();
	SwitchToEqpNo(0);
	QuerySSOE(m_wEqpID, 1, 1, 1000,(VDBSOE *)m_dwPubBuf, &m_WritePtr, &m_BufLen);
	m_ReadPtr=m_WritePtr;

	m_calldir=0;
	m_byReadFileFlag = 0;
	m_fileoffset = 0;
	DirOffSetNum=0;
	m_byParaValFlag = 0;
	m_paraoffset = 0;
	m_paraaddrnum = 0;
	m_bysoftupflag=0;
	m_readtimeflag = 0;

	m_dwfiffrzflag=0;
	m_dwrealdataflag=0;
	m_dwdayfrzflag=0;
	m_allcycleflag = 0;
	m_allsendtime = 0;
	m_fdno = 0;
	m_w104sTaskID = 0;
    if(m_guiyuepara.tmp[6] > 0)
	{
		m_fdno = m_guiyuepara.tmp[6];
		m_w104sTaskID = wTaskID;
	}
	if(m_guiyuepara.tmp[5] == 2)
	{
		fIDeadValue =  (float)m_pBaseCfg->wIDeadValue/10;
		fACDeadValue = (float)m_pBaseCfg->wACDeadValue/10;
		fDCDeadValue  = (float)m_pBaseCfg->wDCDeadValue/10;   //直流电压死区
		fPQDeadValue  = (float)m_pBaseCfg->wPQDeadValue/10;   //功率死区
		fFDeadValue  = (float)m_pBaseCfg->wFDeadValue/10;		//频率死区
		fPWFDeadValue  = (float)m_pBaseCfg->wPWFDeadValue/10;//功率因数死区
		fUTHDeadValue  = (float)m_pBaseCfg->wUTHDeadValue/10;//电压谐波死区
		fITHDeadValue  = (float)m_pBaseCfg->wITHDeadValue/10;//电流谐波死区
		fUNDeadValue   = (float)m_pBaseCfg->wUNDeadValue/10;//不平衡度死区
		fLoadDeadValue = (float)m_pBaseCfg->wLoadDeadValue/10;//负载率死区		
	}
	else  if(m_guiyuepara.tmp[5] == 1)
	{
		fIDeadValue =  (float)m_pBaseCfg->wIDeadValue/1000*g_Sys.MyCfg.RunParaCfg.LineCfg[0].wCt2;
		fACDeadValue = (float)m_pBaseCfg->wACDeadValue/1000*g_Sys.MyCfg.RunParaCfg.wPt2;
		fDCDeadValue  = (float)m_pBaseCfg->wDCDeadValue/10;   //直流电压死区
		fPQDeadValue  = (float)m_pBaseCfg->wPQDeadValue/10;   //功率死区
		fFDeadValue  = (float)m_pBaseCfg->wFDeadValue/10;		//频率死区
		fPWFDeadValue  = (float)m_pBaseCfg->wPWFDeadValue/10;//功率因数死区
		fUTHDeadValue  = (float)m_pBaseCfg->wUTHDeadValue/10;//电压谐波死区
		fITHDeadValue  = (float)m_pBaseCfg->wITHDeadValue/10;//电流谐波死区
		fUNDeadValue   = (float)m_pBaseCfg->wUNDeadValue/10;//不平衡度死区
		fLoadDeadValue = (float)m_pBaseCfg->wLoadDeadValue/10;//负载率死区

	}
	else
	{
		fIDeadValue =  (float)m_pBaseCfg->wIDeadValue/1000;
		fACDeadValue =  (float)m_pBaseCfg->wACDeadValue/1000;
		fDCDeadValue  = (float)m_pBaseCfg->wDCDeadValue/1000;   //直流电压死区
		fPQDeadValue  = (float)m_pBaseCfg->wPQDeadValue/1000;   //功率死区
		fFDeadValue  = (float)m_pBaseCfg->wFDeadValue/1000;		//频率死区
		fPWFDeadValue  = (float)m_pBaseCfg->wPWFDeadValue/1000;//功率因数死区
		fUTHDeadValue  = (float)m_pBaseCfg->wUTHDeadValue/1000;//电压谐波死区
		fITHDeadValue  = (float)m_pBaseCfg->wITHDeadValue/1000;//电流谐波死区
		fUNDeadValue   = (float)m_pBaseCfg->wUNDeadValue/1000;//不平衡度死区
		fLoadDeadValue = (float)m_pBaseCfg->wLoadDeadValue/1000;//负载率死区		
		
	}

#if (DEV_CPU == CPU_SAM9X25)  // 以后9x25 没有老的加密芯片，设置1、2无效  cjl 2017年9月21日15:04:38 
	if((m_guiyuepara.jiami == 1) && (2 == m_guiyuepara.jiami))
	{
		m_guiyuepara.jiami = 0;
	}
#endif

#ifdef INCLUDE_SEC_CHIP
	if(m_wCommID == COMM_IPSEC_NO) //配置的通道为IPSEC通道则先执行IPSec初始化
		IPsecParaSet(1000);
#endif
	
#ifdef INCLUDE_ECC
	if(1 == m_guiyuepara.jiami)
		sm2init();
	if(2 == m_guiyuepara.jiami)
		certificate_Init();
#ifdef ECC_MODE_CHIP
	m_jiamiType = 0;
#ifdef INCLUDE_SEC_CHIP
	if((3 == m_guiyuepara.jiami) && (g_ipsecinit != 1))
	{
		JiamiInit(1);
	}
#else
	if(3 == m_guiyuepara.jiami)
		JiamiInit(1);
#endif
	if(4 == m_guiyuepara.jiami)
		EndataInit();//ly
#endif
#endif
	
	safe_time=12*3600;
	event_time=50;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
    
 #ifdef INCLUDE_RECORD
	ReadRcdDir(NULL,1);
#endif
	return TRUE;
}

void CGB104S::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();
	//if need to modify ,to do : 	
	 
	if (m_pBaseCfg->Timer.wScanData2 <= 100)
		m_pBaseCfg->Timer.wScanData2 = 500;
	if (m_pBaseCfg->Timer.wScanData2 > 6000) 
		m_pBaseCfg->Timer.wScanData2 = 1000;

	m_pBaseCfg->Timer.wScanData2 /= TICK2MSE;	
}

void CGB104S::SetBaseCfg(void)
{
	CProtocol::SetBaseCfg();
}


void CGB104S::StartTimer(BYTE TimerNo)
{
	if (TimerNo>3)
   		return;
  	m_vTimer[TimerNo].wCounter = m_vTimer[TimerNo].wInitVal*10000;
  	m_vTimer[TimerNo].bRun = TRUE;	
}

void CGB104S::StopTimer(BYTE TimerNo)
{
	if (TimerNo>3)
   		return;
  	m_vTimer[TimerNo].bRun = FALSE;	
}

void CGB104S::DoTimeOut(void)
{
	static BYTE cnt=0;
    CPSecondary::DoTimeOut();
	DWORD msd;
	int i=0;
	msd=Get100usCnt();
	if(msd-tmpms>100000)
		tmpms=msd;
	msd=msd-tmpms;
	if(g_Sys.dwErrCode&SYS_ERR_COMM)
		{
			if(safe_time>600)
				safe_time=600;
		}
	if((safe_time--)==0)
		{
			;//reboot(0);
		}
#ifndef INCLUDE_4G		
#ifdef INCLUDE_GPRS
	static BYTE cnt1=0;
	cnt1++;
	if((m_wCommID == (GprsChan.GPRS_COMNO + COMM_SERIAL_START_NO)) && (SignalNo != -1) && (cnt1 == 60)&&(GprsChan.gprsPara.cfg & 0x01))
	{
		cnt1 = 0;
		SendGetRSSI();
	}
#endif
#endif

#ifdef FENGXIAN
	for(i=0;i<2;i++)
	{
		if(m_delaysoe[i].flag == 1)
		{
			m_delaysoe[i].DelayTime--;
			if(m_delaysoe[i].DelayTime==0)
			{
				SendSingleSoe_Delay(i);
				m_delaysoe[i].flag = 0;
			}
		}
		if(m_delaycos[i].flag == 1)
		{
			m_delaycos[i].DelayTime--;
			if(m_delaycos[i].DelayTime==0)
			{
				SendClassOneData_Delay(i);
				m_delaycos[i].flag = 0;
			}
		}
	}
#endif
	tmpms=Get100usCnt();
   if(startdelay)
   	startdelay--;
	if (m_vTimer[1].bRun)
   	{
      		//logMsg("t1 %ld,%ld,%ld,%ld\n",m_vTimer[1].wInitVal,tmpms,msd,m_vTimer[1].wCounter,0,0);
    	if (m_vTimer[1].wCounter <=msd)
     	{  
     		//#ifdef LQH_DEBUG

		WriteDoEvent(NULL, 0, "iec104 slave T1 timeout,close TCP!\n");

      		WriteUlogEvent(NULL, 03, 1, "iec104 slave T1 timeout,close TCP!");
      		//#endif  

#ifndef _TEST_VER_	
#if( DEV_SP==DEV_SP_TTU )//4g模块 终端不主动断tcp
      		//CloseTcp();
#else
	CloseTcp();
#endif
#endif      		     		
      		StopTimer(1);
     	}
		else
    	m_vTimer[1].wCounter-=msd;
			
   	}
  	if (m_vTimer[2].bRun)
   	{
   // 	m_vTimer[2].wCounter--;
    	if (m_vTimer[2].wCounter <=msd)
     	{
      		//to do:when T2 timeout, send S frame
      		SendSFrame();
      		StopTimer(2);
	}
  	else
     	m_vTimer[2].wCounter-=msd;
 	}
		
  	if (m_vTimer[3].bRun)
   	{
    	//m_vTimer[3].wCounter--;
 	if (m_vTimer[3].wCounter <=msd)
     	{
    	    //to do:when T3 timeout
     	  //  DoTestLink1();
 	if (!CanSendIFrame())
		    return;
    	   SendUFrame(TESTFR_ACT);
      		StopTimer(3);
      	}
	else
  	   	m_vTimer[3].wCounter -=msd;
		
   	}  

	if(m_bysoftupflag)
	{
		cnt++;
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if(3 == m_guiyuepara.jiami)
		{
			if(cnt == 3)
			{
				if(SecuritySendMD5Frame(m_SendBuf.pBuf,&m_SendBuf.wWritePtr))
				m_bysoftupflag = 1;
				else
				{
				m_bysoftupflag = 0;
				cnt = 0;
				m_SendBuf.wReadPtr = 0;
				SendAllFrame();
				}
			}
		}
		if(4 == m_guiyuepara.jiami)
		{
			if(cnt == 3)
			{
				if(SecuritySendMD5Frame_HN(m_SendBuf.pBuf,&m_SendBuf.wWritePtr))
				m_bysoftupflag = 1;
				else
				{
				m_bysoftupflag = 0;
				cnt = 0;
				m_SendBuf.wReadPtr = 0;
				SendAllFrame();
				}
			}
		}
#endif
		if(cnt > 10)
		{
			shellprintf("程序升级中...%s\n",WriteFileInfo.filename);
			m_bysoftupflag = 0;
			cnt = 0;
			//UpdateUserProgram(WriteFileInfo.filename);
			
		}
	}
	else
	{
		cnt = 0;
	}
	
	//m_delaychangeyc++;
#if (DEV_SP == DEV_SP_TTU)
    CycleSendData();
#else
//#ifdef _TEST_VER_
//   	CycleSendData();
//#endif
#endif
}


BOOL CGB104S::DoYKRet(void)
{
	CPSecondary::DoYKRet();
	if (CheckClearEqpFlag(EQPFLAG_YKRet))
	{
		SendYkReturn();
		
		return TRUE;
	}
	else
	    return FALSE;
}
void CGB104S::SendresetReturn(void)
{
	BYTE Reason = COT_ACTCON;
	BYTE Qualifier = 1;
	 	
	
	if (!CanSendIFrame())
	    return;
	SendFrameHead(FRM_I);
	Sendframe(C_RP_NA, Reason);
write_VSQ(Qualifier);
write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_QRP;
	
	SendFrameTail();  
	
}
#ifdef  _POWEROFF_TEST_VER_

int CGB104S::Getfaultno(int id)
{
	int i;
	struct VVirtualEqp *peqp=g_Sys.Eqp.pInfo[m_wEqpID].pVEqpInfo;
	for(i=0;i<m_pEqpInfo[m_wEqpNo].wYCNum;i++)
		{
			if(peqp->pYC[i].wTEID!=0)
				continue;
			if(id==peqp->pYC[i].wOffset)
				return i;
			
		}
		return -1;
}
BOOL CGB104S::SendFaultVal(void)
{
		int i,k,n;
		VDBYC* pdb;
		float fd;
		int len ;
	struct VVirtualEqp *peqp=g_Sys.Eqp.pInfo[m_wEqpID].pVEqpInfo;
		SwitchToEqpNo(0);
  	if (peqp==NULL) 
		return false;
	len=GetCurrentFault((BYTE*)m_dwPubBuf,1000);
	len=len/sizeof(VDBYC);
	if(len==0) return false;
	SendFrameHead(FRM_I);
	Sendframe(m_guiyuepara.yctype, 3);
	pdb=(VDBYC*)m_dwPubBuf;
	k=0;
	for (i=0; i < len; i++)
	{
		n=Getfaultno(pdb[i].wNo);
		if(n==-1) continue;
		write_infoadd(n+ADDR_YC_LO);
		
		if(m_guiyuepara.yctype!=13)
		{
			
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(pdb[i].nValue);
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(pdb[i].nValue);		
		}
		else
		{
				fd=pdb[i].nValue;
			BYTE *p=(BYTE*)&fd;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[3];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[2];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[1];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[0];
		}
		
		if(m_guiyuepara.yctype!=M_ME_ND)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =0x1;//QDP
		if((m_guiyuepara.yctype==10)||(m_guiyuepara.yctype==12))
			write_time3();
		if((m_guiyuepara.yctype==M_ME_TD)||(m_guiyuepara.yctype==35))
			write_time();
	//	pBuf++;
	k++;
	}	
	
	write_VSQ(k);
	SendFrameTail();
	return true;
	
}
#endif
void CGB104S::SendYkReturn(void)
{
	BYTE Reason = COT_ACTCON;
	BYTE Qualifier = 1;
	VYKInfo *pYKInfo;
//	VASDU *pSendAsdu;
	 	
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	
	if (!CanSendIFrame())
	    return;
	if (pYKInfo->Head.byMsgID == MI_YKCANCEL)
	{
		Reason = COT_DEACTCON;	
		if(m_YK_select_cancel==1)
			{
				m_YK_select_cancel=0;
				return; 
		}
	}
	SendFrameHead(FRM_I);
	if(Reason==COT_ACTCON)
		m_YKflag=1;
	if(pYKInfo->Info.byStatus)
	{
		Reason|=0x40;
		m_YKflag=0;
	//	pYKInfo->Head.byMsgID =MI_YKCANCEL;
	//	m_YK_select_cancel=1;
	//	    CPSecondary::DoYKReq();
}
	if (pYKInfo->Info.wID>m_pEqpInfo[m_wEqpNo].wYKNum)
		{
		Reason = 0x6f;
	}

    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;

	Sendframe(pCtrlInfo->Type, Reason);
//	m_SendBuf.wWritePtr += ASDUID_LEN;
write_VSQ(Qualifier);
	write_infoadd(pCtrlInfo->DOT);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pCtrlInfo->DCO;
//	logMsg("104sykreq:%x-%x-%x\n", p104ykinfo[pYKInfo->Info.wID-1].type, p104ykinfo[pYKInfo->Info.wID-1].dot, p104ykinfo[pYKInfo->Info.wID-1].dco, 0, 0, 0);
	#if 0
	if (pYKInfo->Info.byStatus == 0)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[1];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[2];
#ifndef INCLUDE_DF104FORMAT
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
#endif
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[3];
	}
	else
	{
		//sfqpSendAsdu->byReasonLow |= COT_PN_BIT;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[1]; //信息体地址Lo
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[2]; //
#ifndef INCLUDE_DF104FORMAT
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
#endif
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[3]; //DCO
	}
//		m_ykdelaystop=0;
#endif

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if(3 == m_guiyuepara.jiami)
	{
		if (pYKInfo->Head.byMsgID == MI_YKCANCEL)
		{
#ifdef INCLUDE_SEC_CHIP
		if(m_wCommID == COMM_IPSEC_NO)
			SendFrameTail();//撤销确认，南网为00
		else
#endif
			SendFrameTail0x2();//撤销
			
		}
		else if(pCtrlInfo->DCO&0x80)
			SendFrameTail0x2();//预置
		else
			SendFrameTail();
	}
	else if(4 == m_guiyuepara.jiami)
	{//ly
		if (pYKInfo->Head.byMsgID == MI_YKCANCEL)
			SendFrameTail();//撤销
		else if(pCtrlInfo->DCO&0x80)
			SendFrameTail0x2();//预置
		else
			SendFrameTail(); 
	}
	else
#endif
		SendFrameTail();  
#if(TYPE_USER != USER_GUANGXI)
if((Reason&0x40)==0)
#endif
	if (pYKInfo->Head.byMsgID != MI_YKCANCEL)
		SendYkstop();
}
void CGB104S::SendSetparaReturn(void)
{

}

void CGB104S::SendYkstop(void)
{
	BYTE Reason = COT_ACTTERM;
//	BYTE Qualifier = 1;
//	VYKInfo *pYKInfo;
//	VASDU *pSendAsdu;
	 	
//	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	if(pCtrlInfo->DCO&0x80)
		return;
	SendFrameHead(FRM_I);
	Sendframe(pCtrlInfo->Type, Reason);
	write_VSQ(1);
	write_infoadd(pCtrlInfo->DOT);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pCtrlInfo->DCO;
#if 0
	if (pYKInfo->Info.byStatus == 0)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[1];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[2];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[3];
	}
	else
	{
		//sfqpSendAsdu->byReasonLow |= COT_PN_BIT;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[1]; //信息体地址Lo
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[2]; //
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pYKInfo->abyRsv[3]; //DCO
	}
	#endif
	SendFrameTail();  
//	logMsg("104sykreq:%x-%x-%x\n", p104ykinfo[pYKInfo->Info.wID-1].type, p104ykinfo[pYKInfo->Info.wID-1].dot, p104ykinfo[pYKInfo->Info.wID-1].dco, 0, 0, 0);
	
}


int CGB104S::atOnceProcSCOS(WORD wEqpNo)
{
	if(m_guiyuepara.tmp[0] & SWITCHCOS_BIT) //cos
	{
		if(m_initflag) return 0;
		if(m_allcallflag) return 0;
		
	    	SwitchToEqpNo(wEqpNo);
		if(SendClassOneData()) return 1;  
	}
	return 0;
}

int CGB104S::atOnceProcDCOS(WORD wEqpNo)
{
	if(m_guiyuepara.tmp[0] & SWITCHCOS_BIT) //cos
	{
	 	if(m_initflag) return 0;
	   	SwitchToEqpNo(wEqpNo);
		if(SendDCOS()) return 1;
	}
	return 0;
}

int CGB104S::atOnceProcSSOE(WORD wEqpNo)
{
	if(m_initflag) return 0;
	if(m_allcallflag) return 0;
	
    SwitchToEqpNo(wEqpNo);
	if(SendSingleSoe()) return 1;
	return 0;	
}

int CGB104S::atOnceProcDSOE(WORD wEqpNo)
{
	if(m_initflag) return 0;
	SwitchToEqpNo(wEqpNo);
	if(SendDSoe()) return 1;
	return 0;	
}

int CGB104S::atOnceProcFAInfo(WORD wEqpNo)
{
	WORD ReadNum = 1;
	WORD wBufLen = ASDUINFO_LEN;
	WORD RecFaInfoNum = 0;
	
	BYTE TypeID = M_FA_NA;
	BYTE Reason = COT_SPONT;
//	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return FALSE;
  			
  	ClearTaskFlag(TASKFLAG_FAINFOUFLAG);
  			
    RecFaInfoNum = ReadFAInfo(m_wTaskID, m_wEqpGroupID, ReadNum, wBufLen, (struct VFAProcInfo *)m_dwPubBuf);
	if (RecFaInfoNum == 0)
		return FALSE;							

	//add make fa info frame
	SendFrameHead(FRM_I);
	Sendframe(TypeID, Reason);
				
	memcpy((void *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (void *)m_dwPubBuf, sizeof(VFAProcInfo));
   	m_SendBuf.wWritePtr += sizeof(VFAProcInfo);
   	                   
	//pSendAsdu->byQualifier = RecFaInfoNum;
	write_VSQ(RecFaInfoNum);
	m_EqpGroupExtInfo.wSendFAInfoNum = RecFaInfoNum;                  
	SetTaskFlag(TASKFLAG_SENDFAINFO);
	ClearUFlag(0);
	SendFrameTail();

//	logMsg("Send FaInfo ok.\n",0,0,0,0,0,0);
	
	return 1;		
}

void CGB104S::DoCommSendIdle(void)
{
#ifdef _TEST_VER_
	int i;
#endif
		event_time=0;
		commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_OFF, (BYTE*)&event_time);
		m_timeflag=0;
		if(m_initflag)
			{
			return;
			}
		if(startdelay)
			return;
	
	if(m_retxdflag)
		{
		m_retxdflag=0;
		WriteToComm(m_EqpGroupInfo.wDesAddr);
		return;
		}
	if (m_dwYcSendFlag || m_dwYxSendFlag)
	{
		SendAllYcYx();
		return;	
	}
	if((m_guiyuepara.tmp[0] & SWITCHALLCALL_BIT) == 0)
	{
	if(m_allcalloverflag==0) return;
	}

	if(m_readparaflag==1)
		{
		if(Sendpara())
			return;
		m_readparaflag=0;
	}
	if (m_dwDdSendFlag)
	{
		SendAllDd();
		return;
	}
	if (m_dwDdGroupFlag)
	{
		SendSomeDd();	
		return;
	}
	if(m_QRP==1)
		{
	SendresetReturn();
		m_QRP=0;
	thSleep(30);
	sysRestart(COLDRESTART, SYS_RESET_REMOTE, GetThName(m_wTaskID));
	return;
	}
	#ifdef  _POWEROFF_TEST_VER_

	if(SendFaultVal())
		return;
	#endif
	/*if (CheckClearTaskFlag(TASKFLAG_SSOEUFLAG))
	{
		for (WORD wSoeEqpNo = 0; wSoeEqpNo < m_wEqpNum; wSoeEqpNo++)
	    {
	    	SwitchToEqpNo(wSoeEqpNo);
  			if (CheckClearEqpFlag(EQPFLAG_SSOEUFLAG))
  			{
  				if(SendSingleSoe())  
				return;
	    	}		
	    }
	}*/
  				/*if(SendSingleSoe())  
				return;
	if (CheckClearTaskFlag(TASKFLAG_DSOEUFLAG))
	{
		for (WORD wSoeEqpNo = 0; wSoeEqpNo < m_wEqpNum; wSoeEqpNo++)
	    {
	    	SwitchToEqpNo(wSoeEqpNo);
  		if (CheckClearEqpFlag(EQPFLAG_DSOEUFLAG))
  			{
  				if(SendDSoe()) 
				return;
	    	}		
	    }
	}*/
	if (m_dwYcGroupFlag || m_dwYxGroupFlag)
	{
		SendSomeYcYx();
		return;	
	}
	if(m_readtimeflag)
	{		
		SendtimeReq();
		m_readtimeflag = 0;
	}
	if(SendFaultEvent())
		return;
		
#ifdef INCLUDE_PR
#ifdef SELF_104
	if (CheckClearTaskFlag(TASKFLAG_ACTEVENTUFLAG))
	 {
		
		if(SendEvent())  
			return;
	    
	}
#endif
#endif
#ifdef  _GUANGZHOU_TEST_VER_
	/*if(m_dwchangeYcSendFlag)
	{
		//if(SendAllchangeYc())
			return;
	}*/
#endif

#ifdef _TEST_VER_
	if((m_allcalloverflag == 1) && ( m_guiyuepara.tmp[2] == 1)) //遥测一直上送 
	{
		for(i=0; i<31 ;i++)
		{
			if(SendGroupYc_Addr(i, COT_SPONT))
			{
				event_time=m_pBaseCfg->Timer.wScanData2;
				commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
				return;
			}
			m_wSendYcNum = 0;
		}
	}
#endif

	if(m_byParaValFlag)
	{
		if(SendReadParaVal())
		{
		return;
		}
	}
	if(SendChangeYC())
		return;  

/**************文件传输部分*****************************/
	if(m_calldir)
	{
		SendFileDir();
        event_time= m_pBaseCfg->Timer.wScanData2/2; //加快传输速度
		commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
		return;
	}

	if(m_byReadFileFlag)
	{
		SendReadFileData();
		event_time= m_pBaseCfg->Timer.wScanData2/2; //加快传输速度
		commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
		return;
	}
/**************文件传输部分 END*************************/


/*********************电能量*************************/

	if(m_dwdayfrzflag||m_dwfiffrzflag||m_dwrealdataflag) // 总召
	{
		if(SendAllEnergy())
			return;

	}

/*#ifdef _TEST_VER_  //暂删循环上送
        if(m_allcycleflag&0x80)
        {
                CycleSendAllYcYx(m_allcycleflag&3);
                return;
        }
#endif*/
        
#if(DEV_SP == DEV_SP_TTU)
        if(m_allcycleflag&0x80)
        {
                CycleSendAllYcYx(m_allcycleflag&3);
                return;
        }
#endif

//	if(g_ddfifflag)   //  15min冻结突变
//	{
//		if(SendChangeFifFrz())
//			return;
//	}
//	
//	if(g_dddayflag)   //  日冻结突变
//	{
//		if(SendChangeDayFrz())
//			return;
//	}

//	if(g_ddtideflag)   //  潮流突变
//	{
//		if(SendChangeTide())
//			return;
//	}

	event_time=m_pBaseCfg->Timer.wScanData2;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
	
}

void CGB104S::CloseTcp(void)
{
	StopTimer(1);
	StopTimer(2);
	StopTimer(3);
	
	m_bDTEnable = FALSE;
	m_bTcpConnect = FALSE;
	
	m_wSendNum = 0;	//Send Counter	
	m_wRecvNum = 0;	//Receive Counter
	m_wAckNum = 0;		//Receive Ack Counter
	m_wAckRecvNum = 0; //Have Ack Receive Counter
	
	//add close tcp call
	commCtrl(m_wCommID, CCC_CONNECT_CTRL | CONNECT_CLOSE, (BYTE*)&event_time); 
}

//need to be called from Class CProtocol
DWORD CGB104S::DoCommState(void)
{
	BOOL bTcpState = FALSE;
	DWORD dwCommState;
	VSysClock SysTime;
	dwCommState=CPSecondary::DoCommState();
	if (dwCommState == ETHERNET_COMM_ON)
		bTcpState = TRUE;
	else
		bTcpState = FALSE;
	StopTimer(1);
	StopTimer(2);
	StopTimer(3);
	for (int i = 0; i < m_wEqpNum; i++)
		{
				m_pSendsoenum[i].wSendSSOENum=0;
				m_pSendsoenum[i].wSendDSOENum=0;
		}
	GetSysClock((void *)&SysTime, SYSCLOCK);	
	if (bTcpState)
	{
		m_bTcpConnect = TRUE;
		//m_bDTEnable = FALSE;
		m_wSendNum = 0;	//Send Counter	
	    m_wRecvNum = 0;	//Receive Counter
	    m_wAckNum = 0;		//Receive Ack Counter
	    m_wAckRecvNum = 0; //Have Ack Receive Counter
	    
		//initpara();
		
	    	WriteUlogEvent(NULL, 02, 0, "Iec104 slave TCP server Connect!");
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if(3 == m_guiyuepara.jiami)
		{
			SecurityUnlink();

		}
		if(4 == m_guiyuepara.jiami)
		{
			SecurityUnlink_HN();

		}
#endif
	}
	else
	{
		m_bTcpConnect = FALSE;
		m_bDTEnable = FALSE;
		
		
	      	logMsg("%02d-%02d %02d:%02d:%02d:%03d Iec104 slave TCP server DISconnect!\n",
	      			SysTime.byMonth, SysTime.byDay, SysTime.byHour, SysTime.byMinute, SysTime.bySecond,SysTime.wMSecond);	
		if(m_init)
			WriteUlogEvent(NULL, 02, 1, "Iec104 slave TCP server DISconnect!");
		
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if(3 == m_guiyuepara.jiami)
		{
			SecurityUnlink();

		}
		if(4 == m_guiyuepara.jiami)
		{
			SecurityUnlink_HN();

		}
#endif
	}
	
	m_dwDdSendFlag = 0;
	m_dwYcSendFlag = 0;
	m_dwYxSendFlag = 0;
	m_wSendYcNum = 0;
	m_wSendYxNum = 0;
	m_wSendDYxNum=0;
	
	m_dwYcGroupFlag = 0;
	m_dwYxGroupFlag = 0;
	m_dwDdGroupFlag = 0;
	m_byYcYxQrp = 0;
	m_byDdQcc = 0;
	m_calldir = 0;
	m_byReadFileFlag = 0;

  return(dwCommState);	
}


BOOL CGB104S::DoReceive()
{
	WORD wRet;
	m_SendBuf.wWritePtr = 0;
	m_SendBuf.wReadPtr = 0;
	if (SearchFrame() != TRUE)
	{
		return FALSE;//没找到有效报文
	}
	startdelay=0;
	m_pReceive = (VIec104Frame *)m_RecFrame.pBuf;
	
	m_pASDU = (VASDU *)&m_pReceive->byASDU;
	m_pASDUInfo = (BYTE *)&m_pASDU->byInfo;
	StopTimer(1);	//stop T1	
	
	StartTimer(3);  //receive any I,U,S frame, to start T3
  //to do :proc ASDUs
	for (int i = 0; i < m_wEqpNum; i++)
		{
 // if(m_pSendsoenum[i].wSendSSOENum>16)
  //	m_pSendsoenum[i].wSendSSOENum=0;
  if(m_pSendsoenum[i].wSendDSOENum>16)
  	m_pSendsoenum[i].wSendDSOENum=0;
  SwitchToEqpNo(i);
//	logMsg("m_pEqpExtInfo[m_wEqpNo].wSendSSOENum %ld,m_pEqpExtInfo[i].wSendDSOENum%ld,%ld,%ld\n",m_pEqpExtInfo[i].wSendSSOENum,m_pEqpExtInfo[m_wEqpNo].wSendDSOENum,0,0,0,0);	
		if(m_pSendsoenum[i].wSendSSOENum)
		ReadPtrIncrease(m_wTaskID, m_wEqpID, m_pSendsoenum[i].wSendSSOENum, SSOEUFLAG);
		m_pSendsoenum[i].wSendSSOENum=0;
		if(m_pSendsoenum[i].wSendDSOENum)
		ReadPtrIncrease(m_wTaskID, m_wEqpID, m_pSendsoenum[i].wSendDSOENum, DSOEUFLAG);
		m_pSendsoenum[i].wSendDSOENum=0;		}

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
    
	if((m_wSendNum +1) > m_wAckNum)
	{
		wRet = (m_wSendNum + 1) - m_wAckNum;
	}
	else
	{
		wRet = (m_wSendNum + 1 + MAX_FRAME_COUNTER) - m_wAckNum;
	}

	if (wRet >= m_PARA_K)
	{
		m_bContinueSend = FALSE;    
	}
	else
	{
		m_bContinueSend = TRUE;    
	}

    
	return TRUE;	
}


DWORD CGB104S::SearchOneFrame(BYTE *Buf, WORD Len)
{
	DWORD Rc;
	BYTE FrameLen;
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	int ProcessRet,nRet;
	BYTE type;
#endif

#ifndef INCLUDE_4G
#ifdef INCLUDE_GPRS
			
		VExtGPRSFrmHead *pHead;
		int i;
		int crclen,statelen;
		WORD crc,tag;
		BYTE CrcBuf[1024];
		if(Len>8)
		{
			for(i=0;i<Len;i++)
			{
				if((Buf[i] == Buf[i+2])&&(Buf[i] == EXTGPRS_CODE2)&&(Buf[i+1] == Buf[i+3])&&(Buf[i+1] == EXTGPRS_CODE1))
				{
					pHead = (VExtGPRSFrmHead *)(&Buf[i]);
					statelen = sizeof(VExtGPRSFrmHead);
					tag = MAKEWORD(Buf[statelen+1],Buf[statelen]);
					crclen = sizeof(VExtGPRSFrmHead)+MAKEWORD(pHead->byLen2,pHead->byLen1);
					crc = MAKEWORD(Buf[i+crclen+1],Buf[i+crclen]);
					memcpy(CrcBuf,Buf,4);
					memcpy(CrcBuf+4,Buf+5,crclen-5);
					byAddCrc16(CrcBuf,crclen-1);
					if(crc == MAKEWORD(CrcBuf[crclen],CrcBuf[crclen-1]))
					{
						if(tag == EXTGPRS_TAG_RSSI)
						{
							SignalVal = -cqstodbm(Buf[crclen-1]);
						}	
					}
				}	
			}
		}				
#endif
#endif

	m_pReceive=(VIec104Frame *)Buf;
		
    if (Len < MIN_RECEIVE_LEN)
    	return FRAME_LESS;
	
#ifdef INCLUDE_ECC
	 int  flag;

	if((m_guiyuepara.jiami == 1) ||(m_guiyuepara.jiami == 2))
	{
	if(((Rc=SearcheccOneFrame(Buf,Len,1,2))&FRAME_SHIELD)
		!= FRAME_ERR)
		{
			if((Rc & FRAME_SHIELD)==FRAME_OK)
				{
				m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
										thPrioritySet(m_wTaskID,0);
					flag=sm2ver(Buf,m_SendBuf.pBuf,GetOwnAddr(),2,m_guiyuepara.jiami);
					if(flag<0) 
						{
						return FRAME_ERR|(Rc&0xfff);
						}
					logMsg("验签成功\r\n",0,0, 0, 0, 0, 0);

					if(1 == m_guiyuepara.jiami)
					{
						if(flag>0)
						{
							m_SendBuf.wWritePtr=flag;
							flag=WriteToComm(m_EqpGroupInfo.wDesAddr);
							if(flag>20)
								wr_F3();
							if(flag<1)
								m_retxdflag=1;
							event_time=50;
							commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
							return	FRAME_ERR|(Rc&0xfff);
						}
					}	
			}
			if((Rc & FRAME_SHIELD) == FRAME_LESS)
			{
				return Rc;
			}
		}
	}
#ifdef  ECC_MODE_CHIP
	if( 3 == m_guiyuepara.jiami)
	{
		Rc=SecuritySearchOneFrame(Buf,Len);
		if((Rc&FRAME_SHIELD)==FRAME_ERR)
			return FRAME_ERR|(Rc&0xfff);
		if((Rc&FRAME_SHIELD)==FRAME_LESS)
			return FRAME_LESS;

		//OK
		if((Rc&FRAME_SHIELD)==FRAME_OK)
		{
			type = 0;
			ProcessRet = SecurityAllFrameProcess(m_wCommID,Buf,Len,m_SendBuf.pBuf,&m_SendBuf.wWritePtr,1,1,&type);
			switch(ProcessRet)
			{
				case 0:
					return FRAME_ERR|(Rc&0xFFFF);
				case 1:
				{
						m_jiamiType = type;
					FrameLen = m_pReceive->byAPDULen;
					m_pASDU = (VASDU *)&m_pReceive->byASDU;

					if (FrameLen > (CONTROL_LEN + ASDUID_LEN))
					{
						m_wRecAddress = MAKEWORD(m_pASDU->byAddressLow, m_pASDU->byAddressHigh);
					}
					return Rc; //返回本帧总长度
				}
				case 2:
				{
					m_SendBuf.wReadPtr = 0;
					SendAllFrame();
					return FRAME_ERR|(Rc&0xFFFF);
				}
				case 3:
				{//不需要任何处理，直接跳过即可
					return FRAME_ERR|(Rc&0xFFFF);
				}
				case 4:
				case 5:
				{
					while(1)
					{
						nRet = SecurityPollSend(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
						if(nRet>0)
						{
							m_SendBuf.wReadPtr = 0;
							SendAllFrame();
							thSleep(50);//500ms
						}
						if(nRet == 2||nRet == 0)
							break;
					}
					return FRAME_ERR|(Rc&0xFFFF);
				}
				default:
					return FRAME_ERR|(Rc&0xFFFF);
			}
		}

	}
	if( 4 == m_guiyuepara.jiami)
		{
			Rc=SecuritySearchOneFrame_HN(Buf,Len);
			if((Rc&FRAME_SHIELD)==FRAME_ERR)
				return FRAME_ERR|(Rc&0xfff);
			if((Rc&FRAME_SHIELD)==FRAME_LESS)
				return FRAME_LESS;
	
			//OK
			if((Rc&FRAME_SHIELD)==FRAME_OK)
			{
				type = 0;
				ProcessRet = SecurityAllFrameProcess_HN(m_wCommID,Buf,Len,m_SendBuf.pBuf,&m_SendBuf.wWritePtr,1,1,&type);
				switch(ProcessRet)
				{
					case 0:
						return FRAME_ERR|(Rc&0xFFFF);
					case 1:
					{
							m_jiamiType = type;
						FrameLen = m_pReceive->byAPDULen;
						m_pASDU = (VASDU *)&m_pReceive->byASDU;
	
						if (FrameLen > (CONTROL_LEN + ASDUID_LEN))
						{
							m_wRecAddress = MAKEWORD(m_pASDU->byAddressLow, m_pASDU->byAddressHigh);
						}
						return Rc; //返回本帧总长度
					}
					case 2:
					{
						m_SendBuf.wReadPtr = 0;
						SendAllFrame();
						return FRAME_ERR|(Rc&0xFFFF);
					}
					case 3:
					{//不需要任何处理，直接跳过即可
						return FRAME_ERR|(Rc&0xFFFF);
					}
					default:
						return FRAME_ERR|(Rc&0xFFFF);
				}
			}
	
		}

#endif
#endif   	
    Rc = SearchHead(Buf,Len,0,START_CODE);
    if (Rc)
       return FRAME_ERR|Rc;
	FrameLen = m_pReceive->byAPDULen;
	
	if (FrameLen < 4)
		return FRAME_ERR | (1);
    
    if (FrameLen > (MAX_FRAME_LEN - 2))
		return FRAME_ERR|(1);

    if (((FrameLen + 2) > (Len)) && (FrameLen == CONTROL_LEN))
		return FRAME_LESS;
	
   if((FrameLen != CONTROL_LEN) && (FrameLen < (CONTROL_LEN + ASDUID_LEN))) //  不判信息体地址，读全部参数没有 
   	return FRAME_ERR|(1);
	
    m_pASDU = (VASDU *)&m_pReceive->byASDU;
    
    if (FrameLen > (CONTROL_LEN + ASDUID_LEN))
    {
    	m_wRecAddress = MAKEWORD(m_pASDU->byAddressLow, m_pASDU->byAddressHigh);
        
		if ((FrameLen + 2) > (Len))
		{
			if(m_wRecAddress != GetEqpOwnAddr())
				return FRAME_ERR|(1);
			else
				return FRAME_LESS;
		}
	}
	if(m_guiyuepara.jiami)
		{
		if((m_pASDU->byTypeID==0x2d)||(m_pASDU->byTypeID==0x2e))
		return FRAME_OK | (FrameLen + 2+0x4a+6);
	}
	return FRAME_OK | (FrameLen + 2); //返回本帧总长度
}

void CGB104S::DoIFrame(void)
{
	WORD wAckNum, wRecvNum;
	
	StartTimer(2);	//when receive I frame,start T2
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);
	wRecvNum = MAKEWORD(m_pReceive->byControl1, m_pReceive->byControl2);
	wAckNum >>= 1;
	wRecvNum >>= 1;
	/*if(m_wRecvNum != wRecvNum)
		{
	   return  CloseTcp();
		}*/
	m_wRecvNum = wRecvNum + 1;
	
	m_wAckNum = wAckNum; 
	//DelAckFrame(m_wAckNum - 1);           
		
	StopTimer(1);	//stop T1	
	
	
	if ((m_wRecvNum - m_wAckRecvNum) >= m_PARA_W)
	{
	    SendSFrame();       
	}        
	if(!getasdu())
		return;
	
    		GotoEqpbyAddress(m_dwasdu.Address);
	if(m_dwasdu.Address!=GetEqpOwnAddr())
		{
		errack(46);
		return;
		}
    	m_wRecAddress = m_dwasdu.Address;
	 switch (m_dwasdu.TypeID)	
	{
	    case C_SC_NA:
	    case C_DC_NA:
	    case C_RC_NA://101 yk
	        DoYk();
	        break;
	    case C_YK_EXT://101 yk
	        DoYkext();
	        break;
	    case C_IC_NA://call all/call group
	        DoCallYcYxData();
	        break;
	    //case C_CI_NA://call dd
	       // DoCallDd();
	       // break;
	    case C_RD_NA://read data
	        DoReadData();
	        break;
	    case C_CS_NA://set time
	        DoSetTime();
	        break;
	    case C_TS_NA://test data link
	        DoTestLink();
	        break;
	    case C_RP_NA://reset rtu
	        DoReset();
	        break;
		case C_SE_NA:
		case C_SE_TA_1:
			DoYT();
		 	break;
		case C_SE_NC_1: 
	    case C_SE_TC_1:
	        DoYTF();
	        break;
		case C_FA_SIM:
	    	DoFaSim();
		break;
	    case C_SE_NB_1:
	    case 136:
		//RecYTCommand();
		 	DoSetDot();
	    	break;	 
	    case P_ME_NA:
			Recsetpara();
	    	break;	 
	    case P_AC_NA:
			Recparaact();
	    	break;	 
	    case 107:
			Reclinktest();
	    	break;	 
		case F_FR_NA_1:    //文件传输
			DoFileData();
			break;
		case C_SR_NA_1:   //切换定值区
			DoSwitchValNo();
			break;
		case C_RR_NA_1:    //读取定值区
			DoReadValNo();
			break;
		case C_RS_NA_1:    //读多个或者全部参数定值
			DoReadParaVal();
			break;
		case C_WS_NA_1:      //修改参数
			DoWriteParaVal();
			break;
		case F_SR_NA_1:      //软件升级
			DoSoftUpgrade();
			break;
		case C_CI_NA:        //召唤电能量
			DoCallEnergy();
			break;
		case F_SC_NA_1:
		case F_AF_NA_1:
			break;
		default:
			errack(44);
			break;
	}
}


void CGB104S::DoSFrame(void)
{
	WORD wAckNum;
	wAckNum = MAKEWORD(m_pReceive->byControl3, m_pReceive->byControl4);	
	m_wAckNum = wAckNum >> 1;
	
	StopTimer(1);	//stop T1	
	if(!m_byReadFileFlag)
	{
	event_time=50;
		commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
	}
	
	return;
	/*
	if (m_wAckNum < m_wSendNum)
	{
		//figure 12,do nothing ,wait T1 timeout
		return;
	}
	else
	{
		SendSFrame();
	}
	*/
}


void CGB104S::DoUFrame(void)
{
	DWORD para=1;
	switch (m_pReceive->byControl1)
	{
		case STARTDT_ACT:
			m_bDTEnable = TRUE;
			startdelay=2;
			cosnum=0;
			soenum=0;
			// m_wSendNum = 0;
			// m_wRecvNum = 0;
			//m_wAckNum = 0;
			m_wSendNum = 0;	//Send Counter	
			m_wRecvNum = 0;	//Receive Counter
			m_wAckNum = 0;		//Receive Ack Counter
			m_wAckRecvNum = 0; //Have Ack Receive Counter

			m_bContinueSend = TRUE;//  important
			evSend(m_wTaskID, EV_UFLAG);
			VDBCOS Cos;
			WORD RecCosNum;
			memset(Bitchange,0,256);
			while(1)
			{
				RecCosNum = ReadSCOS(m_wTaskID, m_wEqpID, 1,
						sizeof(VDBCOS), (VDBCOS *)&Cos);
				if (RecCosNum == 0)
					break;

				ReadPtrIncrease(m_wTaskID, m_wEqpID, RecCosNum, SCOSUFLAG);
				if(Cos.wNo>0&&Cos.wNo<256*8)
				{
					Bitchange[Cos.wNo/8]|=1<<(Cos.wNo%8);
				}
			}

			VDBDCOS Cos1;
			
			while(1)
			{
				RecCosNum = ReadDCOS(m_wTaskID, m_wEqpID, 1,
						sizeof(VDBDCOS), (VDBDCOS *)&Cos1);
				if (RecCosNum == 0)
					break;
				ReadPtrIncrease(m_wTaskID, m_wEqpID, RecCosNum, DCOSUFLAG);
			}
			
			SendUFrame(STARTDT_CON);
			m_initflag=1;
        	if((m_guiyuepara.tmp[0] & SWITCHALLCALL_BIT) == 0)
			{
				m_allcallflag=1;
            	m_allcalloverflag=0;
          	}
			else
			{
				m_allcalloverflag = 1;
          	}
        	thSleep(5);
			SendInitFinish();	
			break;
		
		case STOPDT_ACT:
			StopTimer(1);
			StopTimer(2);
			StopTimer(3);
			m_bDTEnable = FALSE;
			SendUFrame(STOPDT_CON);
			break;
		
		case TESTFR_ACT:
			if (!CanSendIFrame())
				return;
			SendUFrame(TESTFR_CON);
			//		thSleep(100);
			//       SendUFrame(TESTFR_ACT);
			break;
			
		case TESTFR_CON:
			StopTimer(1);	//stop T1
			break;   
		
		case REMOTE_MAINT:
			para = MAINT_ID;
			commCtrl(m_wCommID, CCC_TID_CTRL, (BYTE*)&para);
			break;
	}
	return;
}


void CGB104S::DoYk(void)
{
    BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff;
	WORD  DCO;	//遥控命令限定词
	WORD  YkNo; //遥控路号
	YkNo = MAKEWORD(pData[0], pData[1]);
	DCO = pData[m_guiyuepara.infoaddlen];
	if(m_dwasdu.VSQ!=1)
		{
			return errack(m_dwasdu.COT+1);
		}
	if((m_dwasdu.COT!=6)&&(m_dwasdu.COT!=8))
		{
			return errack(45);
		}
	if((m_dwasdu.Info<ADDR_YK_LO)||(m_dwasdu.Info>ADDR_YK_HI))
		{
			return errack(47);
		}
	GotoEqpbyAddress(m_wRecAddress);

	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	if(GotoEqpbyAddress(m_wRecAddress)==false)
		{
			pYKInfo->Info.byStatus=1;
			SendYkReturn();
				return ;
	}
	
	m_YK_select_cancel=0;
	    switch (m_dwasdu.TypeID)
	    {	case C_SC_NA:
		    case C_DC_NA:	
			    YkNo = YkNo - ADDR_YK_LO + 1;//yk no from 1 start
			    break;
		    case C_RC_NA:	
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

			    pYKInfo->Head.byMsgID = MI_YKOPRATE;
			    break;
		    default:
			    break;	
	    }
	    if ((m_dwasdu.COT&0x3f )== COT_DEACT)
	    {
	    	pYKInfo->Head.byMsgID = MI_YKCANCEL;	
	    }

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	   if(3 == m_guiyuepara.jiami)
	   {
	   	if((pYKInfo->Head.byMsgID == MI_YKSELECT) || (pYKInfo->Head.byMsgID == MI_YKCANCEL))
		  {
		  	if(m_jiamiType != 5)
		  	{
		  		SecuritySend1FFrame(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
		  		m_SendBuf.wReadPtr = 0;
				SendAllFrame();
		  		return;
		  	}
		  }
		  if(pYKInfo->Head.byMsgID == MI_YKOPRATE)
	  	  {
	  		if(m_jiamiType != 7)
	  		{
	  			SecuritySend1FFrame(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
				m_SendBuf.wReadPtr = 0;
				SendAllFrame();
				return;
	  		}
	  	  }  
	   }
	   if(4 == m_guiyuepara.jiami)
		  {
		   if((pYKInfo->Head.byMsgID == MI_YKSELECT) || (pYKInfo->Head.byMsgID == MI_YKCANCEL))
			 {
			   if(m_jiamiType != 5)
			   {
				   SecuritySend1FFrame_HN(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
				   m_SendBuf.wReadPtr = 0;
				   SendAllFrame();
				   return;
			   }  
			 }
			 if(pYKInfo->Head.byMsgID == MI_YKOPRATE)
			 {
			   if(m_jiamiType != 7)
			   {
				   SecuritySend1FFrame_HN(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
				   m_SendBuf.wReadPtr = 0;
				   SendAllFrame();
				   return;
			   }
			 }	
		  }

#endif
		
		if(m_dwasdu.TypeID!= C_SC_NA)
		{
			switch (DCO & 0x03)
		    {
			    case 0: 
			    case 3: 
				    return;
			    case 1: 
				    pYKInfo->Info.byValue = 0x06|((DCO<<1)&0xf0);
				    break; //分
			    case 2: 
				    pYKInfo->Info.byValue = 0x05|((DCO<<1)&0xf0);
				    break; //合
			    default:
				    break;
		    }
		}
	    
		if(m_dwasdu.TypeID== C_SC_NA)
		{
			pYKInfo->Info.byValue=0x6|((DCO<<2)&0xf0);
			if(DCO & 1)
				pYKInfo->Info.byValue = 0x05|((DCO<<2)&0xf0);
		}
	//	pYKInfo->type= m_pASDU->byTypeID;
	  //  pYKInfo->dot=MAKEWORD(pData[0],pData[1]);
	//	pYKInfo->DCO=pData[m_guiyuepara.infoaddlen];

    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	pCtrlInfo->Type = m_dwasdu.TypeID;
	pCtrlInfo->DOT = MAKEWORD(pData[0],pData[1]);
	pCtrlInfo->DCO = pData[m_guiyuepara.infoaddlen];

		m_YKflag=0;
//	logMsg("104sykreq:%x-%x-%x\n", p104ykinfo[YkNo-1].type, p104ykinfo[YkNo-1].dot, p104ykinfo[YkNo-1].dco, 0, 0, 0);

    if (CPSecondary::DoYKReq() == FALSE)
    {
        SendYkReturn();
		return;
    }
	VSysClock SysTime;
  	GetSysClock((void *)&SysTime, SYSCLOCK);

	 switch (pYKInfo->Head.byMsgID)
  	{
	case MI_YKSELECT:		
	  if ( (pYKInfo->Info.byValue & 0x03) == 1)
	  	myprintf(m_wTaskID,LOG_ATTR_INFO,"开关%d , 合预置:%04d/%02d/%02d;%02d:%02d:%02d",YkNo,
	  	SysTime.wYear,SysTime.byMonth, SysTime.byDay, SysTime.byHour, SysTime.byMinute, SysTime.bySecond);
	  else
	  	myprintf(m_wTaskID,LOG_ATTR_INFO,"开关%d , 分预置:%04d/%02d/%02d;%02d:%02d:%02d",YkNo,
	  	SysTime.wYear,SysTime.byMonth, SysTime.byDay, SysTime.byHour, SysTime.byMinute, SysTime.bySecond);
	  break;
	case MI_YKOPRATE:
	   if ( (pYKInfo->Info.byValue & 0x03) == 1)
	   {
		  	myprintf(m_wTaskID,LOG_ATTR_INFO,"开关%d , 合执行:%04d/%02d/%02d;%02d:%02d:%02d",YkNo,
		  	SysTime.wYear,SysTime.byMonth, SysTime.byDay, SysTime.byHour, SysTime.byMinute, SysTime.bySecond);

			#if (DEV_SP == DEV_SP_CTRL)//增加协调控制器
				ykValue[YkNo]=1;
				extNvRamSet(NVRAM_CONTROLLER_YKVALUE, (BYTE*)&ykValue, sizeof(ykValue));
			#endif
	   }
	   else
	   {

	  	myprintf(m_wTaskID,LOG_ATTR_INFO,"开关%d , 分执行:%04d/%02d/%02d;%02d:%02d:%02d",YkNo,
	  	SysTime.wYear,SysTime.byMonth, SysTime.byDay, SysTime.byHour, SysTime.byMinute, SysTime.bySecond);
	 
			
			#if (DEV_SP == DEV_SP_CTRL)//增加协调控制器
				ykValue[YkNo]=0;
				extNvRamSet(NVRAM_CONTROLLER_YKVALUE, (BYTE*)&ykValue, sizeof(ykValue));
			#endif
	   }
	   break;
	 }
//	#endif
	//}
	
	//if (m_pASDU->byReasonLow == COT_DEACT)   //yk cancel
	//{
	//    SendYkCancelAck();
	//}
	
	return;
}
void CGB104S::DoYT(void)
{
    BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff;
	WORD  DCO;	//遥调命令限定词
	WORD  YTNo; //遥调点号
	BYTE infolen;
	infolen = m_guiyuepara.infoaddlen;
	YTNo = MAKEWORD(pData[0], pData[1]);
	DCO = pData[infolen+2];
	if(m_dwasdu.VSQ!=1)
		{
			return errack(m_dwasdu.COT+1);
		}
	if((m_dwasdu.COT!=6)&&(m_dwasdu.COT!=8))
		{
			return errack(45);
		}
	if((m_dwasdu.Info<ADDR_YT_LO)||(m_dwasdu.Info>ADDR_YT_HI))
		{
			return errack(47);
		}
	GotoEqpbyAddress(m_wRecAddress);

	VYTInfo *pYTInfo;
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	if(GotoEqpbyAddress(m_wRecAddress)==false)
	{
		pYTInfo->Info.byStatus=1;
			//SendYtReturn();
		return ;
	}
	
	YTNo = YTNo - ADDR_YT_LO +1;//yt no from 1 start
	
    pYTInfo->Head.wEqpID = m_wEqpID;
    pYTInfo->Info.wID = YTNo;
	if(DCO&0x80)
		pYTInfo->Head.byMsgID = MI_YTSELECT;
	else
		pYTInfo->Head.byMsgID = MI_YTOPRATE;

	if((m_dwasdu.COT&0x3f)==COT_DEACT)
	{
	    pYTInfo->Head.byMsgID = MI_YKCANCEL;	
	}
	pYTInfo->Info.dwValue=MAKEWORD(pData[infolen], pData[infolen+1]);
			
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	pCtrlInfo->Type = m_dwasdu.TypeID;
	pCtrlInfo->DOT = MAKEWORD(pData[0],pData[1]);
	pCtrlInfo->DCO = pData[infolen+2];
	if (CPSecondary::DoYTReq() == FALSE)
    {
        SendYtReturn();
		return;
    }
	return;
}

void CGB104S::DoYTF(void)
{
    BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff;
	WORD  DCO;	//遥调命令限定词
	WORD  YTNo; //遥调路号
	BYTE infolen;
	infolen = m_guiyuepara.infoaddlen;
	YTNo = MAKEWORD(pData[0], pData[1]);
	DCO = pData[infolen+4];
	if(m_dwasdu.VSQ!=1)
		{
			return errack(m_dwasdu.COT+1);
		}
	if(m_dwasdu.COT!=6)
		{
			return errack(45);
		}
	if((m_dwasdu.Info<ADDR_YT_LO)||(m_dwasdu.Info>ADDR_YT_HI))
		{
			return errack(47);
		}
	GotoEqpbyAddress(m_wRecAddress);

	VYTInfo *pYTInfo;
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	if(GotoEqpbyAddress(m_wRecAddress)==false)
	{
		pYTInfo->Info.byStatus=1;
		return ;
	}
	
	YTNo = YTNo - ADDR_YT_LO +1;//yt no from 1 start
	
    pYTInfo->Head.wEqpID = m_wEqpID;
    pYTInfo->Info.wID = YTNo;
	if(DCO&0x80)
		pYTInfo->Head.byMsgID = MI_YTSELECT;
	else
		pYTInfo->Head.byMsgID = MI_YTOPRATE;

	if((m_dwasdu.COT&0x3f )== COT_DEACT)
	{
	    pYTInfo->Head.byMsgID = MI_YKCANCEL;	
	}
	pYTInfo->Info.dwValue=MAKEDWORD(MAKEWORD(pData[infolen], pData[infolen+1]), MAKEWORD(pData[infolen+2], pData[infolen+3]));
			
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	pCtrlInfo->Type = m_dwasdu.TypeID;
	pCtrlInfo->DOT = MAKEWORD(pData[0],pData[1]);
	pCtrlInfo->DCO = pData[infolen+4];

#if (DEV_SP == DEV_SP_CTRL)
	
	if(pYTInfo->Head.byMsgID == MI_YTOPRATE)
	{ 
		CRD_ytValue[m_wRecAddress][YTNo]=pYTInfo->Info.dwValue;
		GetCRD_ytValue_Flag[m_wRecAddress][YTNo] = 1;
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "%d,%d,%f",m_wRecAddress,YTNo,CRD_ytValue[m_wRecAddress][YTNo]);
	}
	
#endif
	
	if (CPSecondary::DoYTReq() == FALSE)
    {
        SendYtReturn();
		return;
    }

	
	return;
}
void CGB104S::SendYtstop(void)
{
  //BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff;
	//WORD  DCO;	//遥控命令限定词
	//WORD  YTNo; //遥控路号
	//YTNo = MAKEWORD(pData[0], pData[1]);
	//DCO = pData[m_guiyuepara.infoaddlen];
	WORD ytvalue,byMs;
	VSysClock SysTime;
	VYTInfo *pYTInfo;
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	if(m_dwasdu.VSQ!=1)
	{
		return ;
	}
	if(m_dwasdu.COT!=6)
	{
		return ;
	}
	if((m_dwasdu.Info<ADDR_YT_LO)||(m_dwasdu.Info>ADDR_YT_HI))
	{
		return ;
	}
	GotoEqpbyAddress(m_wRecAddress);
	if(GotoEqpbyAddress(m_wRecAddress)==false)
	{
		return ;
	}
	VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	if(pCtrlInfo->DCO&0x80)
		return;

	if (!CanSendIFrame())
		return;
	thSleep(10);
	SendFrameHead(FRM_I);
	Sendframe(pCtrlInfo->Type, COT_ACTTERM);
	write_VSQ(1);
	write_infoadd(pCtrlInfo->DOT);
	if((pCtrlInfo->Type == C_SE_NA)||(pCtrlInfo->Type == C_SE_TA_1))
	{
		ytvalue = pYTInfo->Info.dwValue;
		SendWordLH(ytvalue);
	}
	else if((pCtrlInfo->Type == C_SE_NC_1)||(pCtrlInfo->Type == C_SE_TC_1))
		SendDWordLH(pYTInfo->Info.dwValue);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pCtrlInfo->DCO;
	//m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pCtrlInfo->DCO>>8;

	if((pCtrlInfo->Type == C_SE_TA_1)||(pCtrlInfo->Type == C_SE_TC_1))
	{
		GetSysClock((void *)&SysTime, SYSCLOCK);
		byMs = SysTime.bySecond*1000+SysTime.wMSecond;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(byMs);
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(byMs);
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byHour;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byDay;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMonth;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.wYear-2000;
	}
	SendFrameTail();
	return;
}

void CGB104S::DoYkext(void)
{
 #if (TYPE_USER == USER_ZJWW)
   BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff;
	WORD  DCO;	//遥控命令限定词
//	WORD  YkNo; //遥控路号
//	YkNo = MAKEWORD(pData[0], pData[1]);
	DCO = pData[m_guiyuepara.infoaddlen];

	GotoEqpbyAddress(m_wRecAddress);

	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	if(m_wRecAddress!=g_Sys.AddrInfo.wAddr)
		{
		//	pYKInfo->Info.byStatus=1;
			//SendYkReturn();
				return ;
	}

		SendFrameHead(FRM_I);
		Sendframe(150, COT_ACTCON);
		write_VSQ(m_dwasdu.VSQ);
		memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,pData,m_dwasdu.VSQ*(9+m_guiyuepara.infoaddlen));
		m_SendBuf.wWritePtr+=m_dwasdu.VSQ*(9+m_guiyuepara.infoaddlen);
		if(((DCO&0x80)==0)&&(m_dwasdu.VSQ>1))
			{
			VYkGroupCmd* pcmd=(VYkGroupCmd*)m_dwPubBuf;
			pData=pData+9+m_guiyuepara.infoaddlen;
			int j=0;
				for(int i=0;i<m_dwasdu.VSQ-1;i++)
				{
						pcmd[i].ykno=MAKEWORD(pData[j],pData[ j+1]);
						pcmd[i].ykno&=0x3ff;
						if((pData[j+m_guiyuepara.infoaddlen]&3)==1)
						pcmd[i].value=5;
						else
							pcmd[i].value=6;
						j+=m_guiyuepara.infoaddlen+1;
		pcmd[i].begintm=MAKEDWORD(MAKEWORD(pData[j], pData[j+1]), MAKEWORD(pData[j+2], pData[j+3]));
		pcmd[i].endtm=MAKEDWORD(MAKEWORD(pData[j+4], pData[j+5]), MAKEWORD(pData[j+6], pData[j+7]));
					j+=8;	
				}
				
				zjwwYkGroupOutput(m_dwasdu.VSQ-1,(VYkGroupCmd*)m_dwPubBuf);
			}
	SendFrameTail();  
#endif
	
	return;
}

void CGB104S::DoCallYcYxData(void)
{
	if(m_dwasdu.VSQ!=1)
		{
			return errack(m_dwasdu.COT+1);
		}
	if((m_dwasdu.COT!=6))
		{
			return errack(45);
		}
	if((m_dwasdu.Info!=0))
		{
			return errack(47);
		}
	m_QOI=m_pReceive->byASDU[m_dwasdu.Infooff+m_guiyuepara.infoaddlen];
		/*if(m_initflag)
		{
			SendInitFinish();
	}*/

		SendCallYcYxAck();
	switch (m_dwasdu.COT)
		{
			case COT_ACT:		//总召唤激活，应答所有数据 包括遥测、遥信，不包括电度
				if(m_QOI==COT_INTROGEN)
					{
				m_dwYcSendFlag = 0xffffffff;
				m_dwYxSendFlag = 0xffffffff;
				m_wSendYcNum = 0;
				m_wSendYxNum = 0;
				m_wSendDYxNum= 0;
					}
				else if((m_QOI>=COT_INRO1)&&(m_QOI<=COT_INRO8))
					{
					m_dwYxGroupFlag = m_dwYxGroupFlag | (1 << (m_QOI - COT_INRO1)) | 0x80000000;

				}
				else if((m_QOI>=COT_INRO9)&&(m_QOI<=COT_INRO12))
					{
					m_dwYcGroupFlag = m_dwYcGroupFlag | (1 << (m_QOI - COT_INRO9)) | 0x80000000;

				}
				if(m_QOI==34)
					{
					m_readparaflag=1;
					m_readparaeqpnum=0;
					m_wSendparaNum=0;
					m_QPM=1;
					}
				break;
			case COT_DEACT://停止激活
			if(m_QOI==COT_INTROGEN)
					{
				m_dwYcSendFlag = 0;
				m_dwYxSendFlag = 0;
				}
			else if((m_QOI>=COT_INRO1)&&(m_QOI<=COT_INRO8))
					{
					m_dwYxGroupFlag = 0;

				}
				else if((m_QOI>=COT_INRO9)&&(m_QOI<=COT_INRO12))
					{
					m_dwYcGroupFlag = 0;

				}
				if(m_QOI==34)
					{
					m_readparaflag=0;
					m_readparaeqpnum=0;
					m_wSendparaNum=0;
					}
				break;
			}
	
    return;
}


void CGB104S::DoCallDd(void)
{
	WORD wDdGroupNo;
	m_byDdQcc = m_pReceive->byASDU[m_dwasdu.Infooff+m_guiyuepara.infoaddlen];
	if (m_dwasdu.COT== COT_REQ)	//call one group dd
	{
		wDdGroupNo = (m_byDdQcc& 0x0f) - 1;
		if(wDdGroupNo==4)
			m_dwDdSendFlag = 0xf;
		else
		if( SendGroupDd(wDdGroupNo, wDdGroupNo + COT_REQCO1) == FALSE )
			SendNoData();	
		
	}
	if (m_dwasdu.COT== COT_ACT)	//call all dd
	{
			//if((m_byDdQcc>>6)==1)
				
			if ((m_byDdQcc & 0x0f) == 0x05)
				m_dwDdSendFlag = 0xffffffff;
			if (((m_byDdQcc & 0x0f) >= 1) && ((m_byDdQcc& 0x0f) <= 4))
			{
				wDdGroupNo = (m_byDdQcc& 0x0f) - 1;
				m_dwDdGroupFlag = m_dwDdGroupFlag | (1 << wDdGroupNo) | 0x80000000;
			}

			SendCallDdAck();
	}

    		return;
}


void CGB104S::DoReadData(void)
{
	WORD wDataAddr;
	
	wDataAddr = MAKEWORD(m_pASDU->byInfo[0], m_pASDU->byInfo[1]);
	
	if ((wDataAddr >= ADDR_YX_LO) && (wDataAddr <= ADDR_YX_HI))
	{
		SendReadYx(wDataAddr - 1);
		return;
	}
	if ((wDataAddr >= ADDR_YC_LO) && (wDataAddr <= ADDR_YC_HI))
	{
		SendReadYc(wDataAddr - ADDR_YC_LO);
		return;
	}
	if ((wDataAddr >= ADDR_YCPARA_LO) && (wDataAddr <= ADDR_YCPARA_HI))
	{
		SendReadYcpara(wDataAddr - ADDR_YCPARA_LO);
		return;
	}
	if ((wDataAddr >= ADDR_DD_LO) && (wDataAddr <= ADDR_DD_HI))
	{
		SendReadDd(wDataAddr - ADDR_DD_LO);
		return;
	}
	SendNoData();
    return;
}


void CGB104S::DoSetTime(void)
{
	BYTE * pData = m_pASDU->byInfo;
	BYTE TypeID = C_CS_NA;
	BYTE Reason = COT_ACTCON;
//	BYTE Qualifier = 1;
//	VASDU * pSendAsdu;
	VSysClock SysTime;	
	WORD MSecond;
	
	if(m_dwasdu.VSQ!=1)
		{
			return errack(m_dwasdu.COT+1);
		}
	if(m_dwasdu.COT == 5)
	{
		m_readtimeflag = 1;
		return ;
	}
		if((m_dwasdu.COT!=6))
		{
			return errack(45);
		}
	if((m_dwasdu.Info!=0))
		{
			return errack(47);
		}
	
	pData=&m_pReceive->byASDU[m_dwasdu.Infooff+m_guiyuepara.infoaddlen];
	MSecond = MAKEWORD(pData[0], pData[1]); 
	SysTime.wMSecond = MSecond % 1000;
	SysTime.bySecond  = MSecond / 1000;
	SysTime.byMinute = pData[2]&0x3F;
	SysTime.byHour = pData[3] &0x1F;
	SysTime.byDay = pData[4] & 0x1F;
	SysTime.byWeek = pData[4] >> 5;
	SysTime.byMonth = pData[5] & 0x0F;
	SysTime.wYear = (pData[6] & 0x7F) + m_guiyuepara.baseyear;
	if (ClockIsOk(&SysTime) == ERROR)
		return errack(m_dwasdu.COT+1);
#if 0
	VCalClock pTime;
	ToCalClock(&SysTime,&pTime);
	if(SysTime.bySecond>=56)
	{
		SysTime.bySecond = SysTime.bySecond-56;
		pTime.dwMinute+=1;
	}
	else
	{
		SysTime.bySecond = SysTime.bySecond+4;
	}
	pTime.wMSecond=SysTime.bySecond*1000+SysTime.wMSecond;
	CalClockTo(&pTime,&SysTime);
#endif
	SetSysClock((void *)&SysTime, SYSCLOCK);
	
	SendFrameHead(FRM_I);
#if(DEV_SP == DEV_SP_TTU)
	g_SetTimeFlag = TRUE;
//主规约循环发送时间
#endif
	Sendframe(TypeID, Reason);
	
	write_infoadd(0);
	//pSendAsdu->byAddressLow = m_pASDU->byAddressLow;
	//pSendAsdu->byAddressHigh = m_pASDU->byAddressHigh;
	//write_time();
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(SysTime.wMSecond+SysTime.bySecond*1000)&0xff;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(SysTime.wMSecond+SysTime.bySecond*1000)>>8;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byMinute;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byHour;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byDay|(SysTime.byWeek<<5);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.byMonth;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=SysTime.wYear%100;
	write_VSQ(1);

	SendFrameTail();
    return;
}
BOOL CGB104S::SendtimeReq(void)
{
	BYTE Style = C_CS_NA, Reason = COT_REQ;
	struct VSysClock Reqclock;
	GetSysClock((void *)&Reqclock, SYSCLOCK);
	SendFrameHead(FRM_I);
	Sendframe(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(Reqclock.wMSecond+Reqclock.bySecond*1000)&0xff;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(Reqclock.wMSecond+Reqclock.bySecond*1000)>>8;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=Reqclock.byMinute;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=Reqclock.byHour;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=Reqclock.byDay|(Reqclock.byWeek<<5);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=Reqclock.byMonth;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=Reqclock.wYear%100;
	write_VSQ(1);

	SendFrameTail();

	return TRUE;
}

void CGB104S::DoSetDot(void)
{
	if(m_dwasdu.VSQ!=1)
		{
			return errack(m_dwasdu.COT+1);
		}
	if((m_dwasdu.COT!=6)&&(m_dwasdu.COT!=8))
		{
			return errack(45);
		}
	if((m_dwasdu.Info<ADDR_YT_LO)||(m_dwasdu.Info>ADDR_YT_HI))
		{
			return errack(47);
		}
	SendFrameHead(FRM_I);
	Sendframe(m_dwasdu.TypeID, m_dwasdu.COT+1);
	
	memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
	m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
	write_VSQ(m_dwasdu.VSQ);

	SendFrameTail();
	thSleep(10);
	if(m_dwasdu.COT==COT_ACT)
		{
	SendFrameHead(FRM_I);
	Sendframe(m_dwasdu.TypeID, COT_ACTTERM);
	
	memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
	m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
	write_VSQ(m_dwasdu.VSQ);

	SendFrameTail();

	}
    return;
}

void CGB104S::errack(BYTE COT)
{
	SendFrameHead(FRM_I);
	Sendframe(m_dwasdu.TypeID, (COT|0x40));
	if((m_pReceive->byAPDULen-4-m_dwasdu.Infooff) > 0)
        {   
	    memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
	    m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
       }
	write_VSQ(m_dwasdu.VSQ);

	SendFrameTail();
    return;
}

void CGB104S::errack0x2(BYTE COT)
{
	SendFrameHead(FRM_I);
	Sendframe(m_dwasdu.TypeID, (COT|0x40));
	if((m_pReceive->byAPDULen-4-m_dwasdu.Infooff) > 0)
  	{   
	    memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
	    m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
 	}
	write_VSQ(m_dwasdu.VSQ);

	SendFrameTail0x2();
    return;
}


void CGB104S::DoTestLink(void)
{
	BYTE TypeID = C_TS_NA;
	BYTE Reason = COT_ACTCON;
	BYTE Qualifier = 1;
//	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return;
	
	SendFrameHead(FRM_I);
	
	Sendframe(TypeID, Reason);
	write_VSQ(Qualifier);
	//pSendAsdu->byQualifier = Qualifier;
				
	m_SendBuf.wWritePtr += ASDUID_LEN;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Lo
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
#ifndef INCLUDE_DF104FORMAT
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
#endif
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0xAA; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0x55;
	 
	SendFrameTail();
//	thSleep(100);
//	     	   SendUFrame(TESTFR_ACT);

}
void CGB104S::Reclinktest(void)
{
	BYTE TypeID = 107;
	BYTE Reason = COT_ACTCON;
	BYTE Qualifier = 1;
//	VASDU * pSendAsdu;
	BYTE *pData=&m_pReceive->byASDU[m_dwasdu.Infooff+m_guiyuepara.infoaddlen];
	if (!CanSendIFrame())
		return;
	
	SendFrameHead(FRM_I);
	
	Sendframe(TypeID, Reason);
	write_VSQ(Qualifier);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pData[0]; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pData[1];
	if((m_pReceive->byAPDULen- CONTROL_LEN) < 9)
		write_time();
	else
	{
		memcpy(m_SendBuf.pBuf +m_SendBuf.wWritePtr,pData+2,7 );
		m_SendBuf.wWritePtr = m_SendBuf.wWritePtr +7;
	}
            
	SendFrameTail();

}

void CGB104S::DoTestLink1(void)
{
	VASDU * pSendAsdu;
	SendFrameHead(FRM_I);
	
	pSendAsdu = (VASDU *)m_pSend->byASDU;
	pSendAsdu->byTypeID = C_TS_NA;
	pSendAsdu->byReasonLow = COT_ACT;
#ifndef INCLUDE_DF104FORMAT
	pSendAsdu->byReasonHigh = GetEqpOwnAddr();
#endif
	pSendAsdu->byQualifier = 1;
				
	m_SendBuf.wWritePtr += ASDUID_LEN;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Lo
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
#ifndef INCLUDE_DF104FORMAT
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
#endif
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0xAA; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0x55;
	 
	SendFrameTail();
    return;
}


void CGB104S::DoReset(void)
{
	//thSleep(10);
//	sysRestart(COLDRESTART, SYS_RESET_REMOTE, GetThName(m_wTaskID));

	BYTE RecSoeNum=10;
	BYTE QRP = m_pReceive->byASDU[m_dwasdu.Infooff+m_guiyuepara.infoaddlen];
	if((m_dwasdu.COT!=6))
	{
		return errack(45);
	}
	if(m_dwasdu.Info != 0)
	{		
		return errack(47);
	}
//SendAck();
m_QRP=QRP;
if(QRP==1)
{
;
//	thSleep(3);
//	sysRestart(COLDRESTART, SYS_RESET_REMOTE, GetThName(m_wTaskID));
}
if(QRP==2)
{
	while(RecSoeNum!=0)
		{
		RecSoeNum=10;
		RecSoeNum = ReadSSOE(m_wTaskID, m_wEqpID, RecSoeNum, sizeof(VDBSOE)*RecSoeNum, (VDBSOE *)&m_dwPubBuf);
		if (RecSoeNum == 0)
			break;
		ReadPtrIncrease(m_wTaskID, m_wEqpID, RecSoeNum, SSOEUFLAG);
		}
		RecSoeNum=10;
	while(RecSoeNum!=0)
		{
		RecSoeNum=10;
		RecSoeNum = ReadDSOE(m_wTaskID, m_wEqpID, RecSoeNum, sizeof(VDBSOE)*RecSoeNum, (VDBDSOE *)&m_dwPubBuf);
		if (RecSoeNum == 0)
			break;
		ReadPtrIncrease(m_wTaskID, m_wEqpID, RecSoeNum, DSOEUFLAG);
		}

}
    return;    
}


void CGB104S::SendSFrame(void)
{
    SendFrameHead(FRM_S);
    SendFrameTail();
    StopTimer(2);	//when send S frame, stop T2
    return;    
}


void CGB104S::SendUFrame(BYTE byControl)
{
    SendFrameHead(FRM_U);
    m_pSend->byControl1 = byControl;
    SendFrameTail();
    if ((byControl == TESTFR_ACT) || (byControl == STARTDT_ACT) || (byControl == STOPDT_ACT))
    {
   // if(m_vTimer[1].bRun==FALSE)
    	StartTimer(1);		//statr T1	
    }
    return;    
}


void CGB104S::SendFrameHead(BYTE byFrameType)
{
    WORD wSendNum, wRecvNum,wRet;
    
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;//清空发送缓冲区 
	
	m_pSend = (VIec104Frame *)m_SendBuf.pBuf;
	
	m_wSendNum &= MAX_FRAME_COUNTER;
	m_wRecvNum &= MAX_FRAME_COUNTER;
	wSendNum = m_wSendNum << 1;
	wRecvNum = m_wRecvNum << 1;
	
	m_pSend->byStartCode = START_CODE;
	
	switch (byFrameType)
	{
	    case FRM_I:
			
   // if(m_vTimer[1].bRun==FALSE)
    	StartTimer(1);		//statr T1	
	        m_pSend->byControl1 = LOBYTE(wSendNum);
            m_pSend->byControl2 = HIBYTE(wSendNum);
            m_pSend->byControl3 = LOBYTE(wRecvNum);
            m_pSend->byControl4 = HIBYTE(wRecvNum);
            m_wAckRecvNum = m_wRecvNum;//wRecvNum;//lqh 20030902
			if((m_wSendNum +1) > m_wAckNum)
			{
				wRet = (m_wSendNum + 1) - m_wAckNum;
			}
			else
			{
				wRet = (m_wSendNum + 1 + MAX_FRAME_COUNTER) - m_wAckNum;
			}
			
			if (wRet >= m_PARA_K)
			{
				m_bContinueSend = FALSE;	
			}
			else
			{
				m_bContinueSend = TRUE;    
			}

	        break;
	    case FRM_S:
	        m_pSend->byControl1 = FRM_S;
            m_pSend->byControl2 = 0;
            m_pSend->byControl3 = LOBYTE(wRecvNum);
            m_pSend->byControl4 = HIBYTE(wRecvNum);
            m_wAckRecvNum = m_wRecvNum;//wRecvNum;//lqh 20030902
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


int CGB104S::SendFrameTail(void)
{
	int len;
	WORD typid = 0;
	BYTE byControl1 = m_pSend->byControl1;
	m_pSend->byAPDULen = m_SendBuf.wWritePtr - 2;
	//SaveSendFrame(m_wSendNum, m_SendBuf.wWritePtr, m_SendBuf.pBuf);
	
	if(!(m_pSend->byControl1 & 0x01))
	{
		if(m_guiyuepara.typeidlen == 1)
			typid = m_SendBuf.pBuf[6];
		else if(m_guiyuepara.typeidlen == 2)
			typid = m_SendBuf.pBuf[6]+(m_SendBuf.pBuf[7]<<8);
		//shellprintf("ctl_typid %d \n",typid );
	}
	

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if(3 == m_guiyuepara.jiami)
	{
		len = SecuritySendFrame(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,1,0);
		if(len<1)
		{
			return 0;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
	} 
	if(4 == m_guiyuepara.jiami)
	{
		len = SecuritySendFrame_HN(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,1,0,typid);
		if(len<1)
		{
			return 0;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
	}
#endif

	len=WriteToComm(m_EqpGroupInfo.wDesAddr);
	if(len<1)
		m_retxdflag=1;
		event_time=50;
		commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
	if ((byControl1 & 0x1) == FRM_I)
	{
		m_wSendNum =  ((m_wSendNum+1)&MAX_FRAME_COUNTER);
		//StartTimer(1);		//statr T1
		StopTimer(2);		//when send I frame, stop T2
	}
	return len;
}

int CGB104S::SendFrameTail0x1(void)
{
	int len;
	WORD typid;
	BYTE frameType = m_pSend->byControl1 & 0x1;

	
	m_pSend->byAPDULen = m_SendBuf.wWritePtr - 2;
//jiami =4时，带上typid
	if(!(m_pSend->byControl1 & 0x01))
	{
		if(m_guiyuepara.typeidlen == 1)
			typid = m_SendBuf.pBuf[6];
		else if(m_guiyuepara.typeidlen == 2)
			typid = m_SendBuf.pBuf[6]+(m_SendBuf.pBuf[7]<<8);
	}


#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
	{
		//ssz
		if(3 == m_guiyuepara.jiami)
		{
			len = SecuritySendFrame(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,1,1);
		}
		else if(4 == m_guiyuepara.jiami)
		{
		  //湖南未加0x1类型，此函数无用
			len = SecuritySendFrame_HN(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,1,1,typid);
		}
		
		if(len<1)
		{
			return 0;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
		//
		len=WriteToComm(m_EqpGroupInfo.wDesAddr);
		if(len<1)
			m_retxdflag=1;
			event_time=50;
			commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
		if (frameType == FRM_I)
		{
			m_wSendNum =  ((m_wSendNum+1)&MAX_FRAME_COUNTER);
			//StartTimer(1);		//statr T1
			StopTimer(2);		//when send I frame, stop T2
		}
		return len;
	}
	else
#endif
		return 0;
}

int CGB104S::SendFrameTail0x2(void)
{
	int len;
	WORD typid;
	BYTE frameType = m_pSend->byControl1 & 0x1;

	
	m_pSend->byAPDULen = m_SendBuf.wWritePtr - 2;
//jiami =4时，带上typid
	if(!(m_pSend->byControl1 & 0x01))
	{
		if(m_guiyuepara.typeidlen == 1)
			typid = m_SendBuf.pBuf[6];
		else if(m_guiyuepara.typeidlen == 2)
			typid = m_SendBuf.pBuf[6]+(m_SendBuf.pBuf[7]<<8);
	}


#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
	{
		//ssz
		if(3 == m_guiyuepara.jiami)
		{
			len = SecuritySendFrame(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,1,2);
		}
		else if(4 == m_guiyuepara.jiami)
		{
			len = SecuritySendFrame_HN(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,1,2,typid);
		}
		
		if(len<1)
		{
			return 0;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
		//
		len=WriteToComm(m_EqpGroupInfo.wDesAddr);
		if(len<1)
			m_retxdflag=1;
			event_time=50;
			commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
		if (frameType == FRM_I)
		{
			m_wSendNum =  ((m_wSendNum+1)&MAX_FRAME_COUNTER);
			//StartTimer(1);		//statr T1
			StopTimer(2);		//when send I frame, stop T2
		}
		return len;
	}
	else
#endif
		return 0;
}

int CGB104S::SendAllFrame(void)//ssz:20170405
{
	int len;
	len=WriteToComm(m_EqpGroupInfo.wDesAddr);
	if(len<1)
		m_retxdflag=1;
	event_time=50;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
	return len;
}

void CGB104S::CycleSendData(void)
{

	if (!CanSendIFrame())
		return;
	SendClassTwoData();
	if(m_timeflag++>3)
		{
		event_time=50;
		commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);

	}
		m_allsendtime++;
		if(m_pBaseCfg->Timer.wAllData!=0)
	if((m_allsendtime>=m_pBaseCfg->Timer.wAllData*60)&&((m_allcycleflag&0x80)==0))
		{
		if(m_dwYcSendFlag|m_dwYxSendFlag)
			{
		m_allsendtime=0;
		return;
		}
		event_time=50;
		commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
				m_dwcycYcSendFlag = 0xffffffff;
				m_dwcycYxSendFlag = 0xffffffff;
		if(m_allcycleflag==1)
			{
			m_allcycleflag=0x81;
		}
		else
			m_allcycleflag=0x81;
	}
	

}


BOOL CGB104S::SendClassOneData(void)
{
	WORD ReadNum = REQ_SCOS_NUM;
	WORD wBufLen = ASDUINFO_LEN;
	WORD RecCosNum = 0;
	WORD SendCosNum = 0;
	VDBCOS *pRecCOS = NULL;
	//BYTE YxValue;
#ifdef FENGXIAN
    WORD m_yxno;
	BYTE m_yxvalue;
#endif
	BYTE TypeID = M_SP_NA;
	BYTE Reason = COT_SPONT;
	//int len;
	//VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return FALSE;
  			
  	ClearTaskFlag(TASKFLAG_SCOSUFLAG);
  	ClearEqpFlag(EQPFLAG_SCOSUFLAG);
  			
    RecCosNum = ReadSCOS(m_wTaskID, m_wEqpID, ReadNum,wBufLen, (VDBCOS *)m_dwPubBuf);
	if (RecCosNum == 0)
		return FALSE;							
	//add make cos frame
	SendFrameHead(FRM_I);

  	if(m_guiyuepara.yxtype==3)
		TypeID=M_DP_NA;
	Sendframe(TypeID,Reason);
	SendCosNum = RecCosNum;
	pRecCOS = (VDBCOS *)m_dwPubBuf;                              																																															
    		                                                             																																															
    for (int i=0; i < RecCosNum; i++)                            																																															
    {   
#ifdef FENGXIAN
		m_yxno = pRecCOS->wNo;
		if(m_guiyuepara.yxtype==3)
			m_yxvalue = (SIQ(pRecCOS->byValue)+1)& 0x03;
		else
			m_yxvalue = SIQ(pRecCOS->byValue)& 0x03;

		if((m_yxno ==0)||(m_yxno ==1))
		{
			if(m_yxvalue == YXF)
			{ 
				if(m_delaycos[m_yxno].flag ==0)
				{
					m_delaycos[m_yxno].CosNo = m_yxno;
					m_delaycos[m_yxno].CosValue = pRecCOS->byValue;
					m_delaycos[m_yxno].DelayTime = YXDelayTime*60;
					m_delaycos[m_yxno].flag = 1;
				}
				SendCosNum--;
				pRecCOS ++;
				continue;		
			}	
			else
			{
				if(m_delaycos[m_yxno].flag !=0)
				{
					SendCosNum--;
					pRecCOS ++;
					continue;		
				}
			}
		}
#endif
		if(SendCosNum == 0)
			return TRUE;
		write_infoadd(pRecCOS->wNo + ADDR_YX_LO);//+m_pEqpInfo[m_wEqpNo].wDYXNum);
		
#if (TYPE_USER == USER_SHANGHAIJY)
		if(m_guiyuepara.yxtype==3)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SIQ(pRecCOS->byValue)+1)& 0x03;
		else
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SIQ(pRecCOS->byValue))& 0x03;
#else
		if(m_guiyuepara.yxtype==3)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pRecCOS->byValue)+1;
		else
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pRecCOS->byValue);
#endif
		pRecCOS ++;                                              																																															
   	}  
	if(SendCosNum == 0)
		return TRUE;
	write_VSQ(SendCosNum);
	
			
	SendFrameTail();
	
	m_pEqpExtInfo[m_wEqpNo].wSendSCOSNum = RecCosNum;                  
	SetTaskFlag(TASKFLAG_SENDSCOS);
	SetEqpFlag(EQPFLAG_SENDSCOS);
	ClearUFlag(m_wEqpNo);	
	cosnum+=RecCosNum;
	//logMsg("cos :%d -%d   len=%d %d-%d\r\n",RecCosNum,cosnum,len,m_SendBuf.wReadPtr,m_SendBuf.wWritePtr,0);

	m_delaychangeyc=0;
	return TRUE;
}

BOOL CGB104S::SendDCOS(void)
{
	WORD ReadNum = REQ_SCOS_NUM;
	WORD wBufLen = ASDUINFO_LEN;
	WORD RecCosNum = 0;
	VDBDCOS *pRecCOS = NULL;
//    BYTE YxValue;
	
	BYTE TypeID = M_DP_NA;
	BYTE Reason = COT_SPONT;
//	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return FALSE;
  			
  	ClearTaskFlag(TASKFLAG_DCOSUFLAG);
  	ClearEqpFlag(EQPFLAG_DCOSUFLAG);
  			
    RecCosNum = ReadDCOS(m_wTaskID, m_wEqpID, ReadNum,
										wBufLen, (VDBDCOS *)m_dwPubBuf);
	if (RecCosNum == 0)
		return FALSE;							
	//add make cos frame
	SendFrameHead(FRM_I);

			Sendframe(TypeID,Reason);
	pRecCOS = (VDBDCOS *)m_dwPubBuf;                              																																															
    		                                                             																																															
    for (int i=0; i < RecCosNum; i++)                            																																															
    {   
 	write_infoadd(pRecCOS->wNo + ADDR_YX_LO);
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = DIQ(pRecCOS->byValue1,pRecCOS->byValue2);                           																																															
   		pRecCOS ++;                                              																																															
   	}                   
	write_VSQ(RecCosNum);
	m_pEqpExtInfo[m_wEqpNo].wSendDCOSNum = RecCosNum;                  
	SetTaskFlag(TASKFLAG_SENDDCOS);
	SetEqpFlag(EQPFLAG_SENDDCOS);
	ClearUFlag(m_wEqpNo);
			
	SendFrameTail();
			
	return TRUE;
}


void CGB104S::SendClassTwoData(void)
{
#ifdef  _GUANGZHOU_TEST_VER_
	/*	if(GetOffFlag())
			{
			m_dwSendAllDataEqpNo=0;
			m_dwchangeYcSendFlag = 0xffffffff;	
		}
		else 	if(m_delaychangeyc++>2)*/
#endif
		{
		m_delaychangeyc=0;
	//SendChangeYC();      
		}
}
void CGB104S::Sendframe(BYTE type,WORD cot)
{
	write_typeid(type);

			write_COT((GetOwnAddr()<<8)|cot);
		write_conaddr(GetEqpOwnAddr());
		if(cot==COT_SPONT)
			// if(m_vTimer[1].bRun==FALSE)
			    	StartTimer(1);		//statr T1
}

BOOL CGB104S::SendSingleSoe(void)
{
	WORD ReadNum = REQ_SSOE_NUM;
//	WORD wBufLen = ASDUINFO_LEN;
	WORD RecSoeNum = 0;
	WORD SendSoeNum = 0;
	VDBSOE *pRecSOE = NULL;
    VSysClock SysTime;
//  BYTE YxValue;
#ifdef FENGXIAN
    WORD m_yxno;
	BYTE m_yxvalue;
#endif
	BYTE TypeID = M_SP_TB;
	BYTE Reason = COT_SPONT;
//	VASDU * pSendAsdu;
	if (!CanSendIFrame())
	    return FALSE;
	
	ClearTaskFlag(TASKFLAG_SSOEUFLAG);
	ClearEqpFlag(EQPFLAG_SSOEUFLAG);

    RecSoeNum = QuerySSOE(m_wEqpID, ReadNum, m_ReadPtr, 1000,(VDBSOE *)m_dwPubBuf, &m_WritePtr, &m_BufLen);
	
	//RecSoeNum = ReadSSOE(m_wTaskID, m_wEqpID, ReadNum,wBufLen,(VDBSOE *)m_dwPubBuf);  
	if (RecSoeNum == 0)                                              
	    return FALSE;
	m_ReadPtr+=RecSoeNum;
	m_ReadPtr%=m_BufLen;
	pRecSOE = (VDBSOE *)m_dwPubBuf;  
/*	if(m_pSendsoenum[m_wEqpNo].wSendSSOENum>RecSoeNum)
		m_pSendsoenum[m_wEqpNo].wSendSSOENum=0;
	pRecSOE+=m_pSendsoenum[m_wEqpNo].wSendSSOENum;
	RecSoeNum-=m_pSendsoenum[m_wEqpNo].wSendSSOENum;
	if (RecSoeNum == 0)                                              
	    return FALSE;*/
	//make soe frame
	//logMsg("1m_pSendsoenum[m_wEqpNo].wSendSSOENum %ld,m_pSendsoenum[m_wEqpNo].wSendDSOENum%ld,%ld,%ld\n",m_pSendsoenum[m_wEqpNo].wSendSSOENum,m_pSendsoenum[m_wEqpNo].wSendDSOENum,RecSoeNum,0,0,0);	
	SendFrameHead(FRM_I);
	if(m_guiyuepara.yxtype==2) 
		TypeID = 2;        //短时标单点遥信
	if(m_guiyuepara.yxtype==3) 
		TypeID = M_DP_TB;        //短时标单点遥信
	Sendframe(TypeID, Reason);
	//	#if (TYPE_USER == USER_GUANGZHOU)
	SendSoeNum = RecSoeNum;
	for (int i=0; i < RecSoeNum; i++)  
	//	#endif
	{    
#ifdef FENGXIAN
		m_yxno = pRecSOE->wNo;
		if(m_guiyuepara.yxtype==3)
			m_yxvalue = (SIQ(pRecSOE->byValue )+1)& 0x03;
		else
			m_yxvalue = SIQ(pRecSOE->byValue)& 0x03;

		//m_yxvalue = pRecSOE->byValue;


		if((m_yxno ==0)||(m_yxno ==1))
		{
			if(m_yxvalue == YXF)
			{ 
				if(m_delaysoe[m_yxno].flag ==0)
				{
					m_delaysoe[m_yxno].SoeNo = m_yxno;
					m_delaysoe[m_yxno].SoeValue = pRecSOE->byValue;//m_yxvalue;
					m_delaysoe[m_yxno].Time.dwMinute = pRecSOE->Time.dwMinute;
					m_delaysoe[m_yxno].Time.wMSecond = pRecSOE->Time.wMSecond;
					m_delaysoe[m_yxno].DelayTime = YXDelayTime*60;
					m_delaysoe[m_yxno].flag = 1;
				}
				SendSoeNum--;
				pRecSOE ++;
				continue;   	
			}	
			else
			{
				if(m_delaysoe[m_yxno].flag !=0)
				{
					SendSoeNum--;
					pRecSOE ++;
					continue;   	
				}
			}
		}
#endif
	if(SendSoeNum == 0)
		return TRUE;
	write_infoadd(pRecSOE->wNo + ADDR_YX_LO);//+m_pEqpInfo[m_wEqpNo].wDYXNum);
#if (TYPE_USER == USER_SHANGHAIJY)
			if(m_guiyuepara.yxtype==3)
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SIQ(pRecSOE->byValue)+1)& 0x03;
			else
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SIQ(pRecSOE->byValue))& 0x03;
#else
			if(m_guiyuepara.yxtype==3)
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pRecSOE->byValue)+1;
			else
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pRecSOE->byValue);
#endif

		SystemClock(pRecSOE->Time.dwMinute, &SysTime);                      
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(pRecSOE->Time.wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(pRecSOE->Time.wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMinute;   
		if(TypeID != 2)//短时标单点遥信 
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byHour & 0x1F;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SysTime.byDay & 0x1F) | ((SysTime.byWeek <<5) & 0xE0);    
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMonth;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.wYear % 100;
		}
		pRecSOE ++;      
	/*	#if (TYPE_USER != USER_GUANGZHOU)
		m_pSendsoenum[m_wEqpNo].wSendSSOENum++;
		#else*/
			{
			//	m_pSendsoenum[m_wEqpNo].wSendSSOENum=0;
		}
	//	#endif
	}                 
	//logMsg("2m_pSendsoenum[m_wEqpNo].wSendSSOENum %ld,m_pSendsoenum[m_wEqpNo].wSendDSOENum%ld,%ld,%ld\n",m_pSendsoenum[m_wEqpNo].wSendSSOENum,m_pSendsoenum[m_wEqpNo].wSendDSOENum,RecSoeNum,0,0,0);	
	if(SendSoeNum == 0)
		return TRUE;
    write_VSQ(1);
 	//m_pEqpExtInfo[m_wEqpNo].wSendSSOENum=0;
 		//#if (TYPE_USER == USER_GUANGZHOU)
    write_VSQ(SendSoeNum);
	m_pEqpExtInfo[m_wEqpNo].wSendSSOENum = RecSoeNum;  
	SetTaskFlag(TASKFLAG_SENDSSOE);                              
	SetEqpFlag(EQPFLAG_SENDSSOE);                                
	ClearUFlag(m_wEqpNo);
	//#endif		
	SendFrameTail();
	soenum+=RecSoeNum;
	return TRUE;                                        	                            	
}
BOOL CGB104S::SendDSoe(void)
{
	WORD ReadNum = REQ_SSOE_NUM;
	WORD wBufLen = ASDUINFO_LEN;
	WORD RecSoeNum = 0;
	VDBDSOE *pRecSOE = NULL;
    	VSysClock SysTime;
//    	BYTE YxValue;
    
	BYTE TypeID = M_DP_TB;
	BYTE Reason = COT_SPONT;
//	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return FALSE;
	
//	ClearTaskFlag(TASKFLAG_DSOEUFLAG);
	//ClearEqpFlag(EQPFLAG_DSOEUFLAG);
	
	RecSoeNum = ReadDSOE(m_wTaskID, m_wEqpID, ReadNum,         
										wBufLen, (VDBDSOE *)m_dwPubBuf);        
		pRecSOE = (VDBDSOE *)m_dwPubBuf;  
		if(m_pSendsoenum[m_wEqpNo].wSendDSOENum>RecSoeNum)
			m_pSendsoenum[m_wEqpNo].wSendDSOENum=0;
	pRecSOE+=m_pSendsoenum[m_wEqpNo].wSendDSOENum;
	RecSoeNum-=m_pSendsoenum[m_wEqpNo].wSendDSOENum;
	//RecSoeNum>>=1;
	if (RecSoeNum == 0)                                              
	    return FALSE;
	//make soe frame
	SendFrameHead(FRM_I);
	if(m_guiyuepara.yxtype==2) 
		TypeID = 4;
		Sendframe(TypeID, Reason);
		
	
	//for (int i=0; i < RecSoeNum; i++)                          
	{    
	
	//		pRecSOE ++;                                            
	write_infoadd(pRecSOE->wNo + ADDR_YX_LO);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = DIQ(pRecSOE->byValue1,pRecSOE->byValue2);
		SystemClock(pRecSOE->Time.dwMinute, &SysTime);                      
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(pRecSOE->Time.wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(pRecSOE->Time.wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMinute;   
		if(TypeID!=4) 
		{
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byHour & 0x1F;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SysTime.byDay & 0x1F) | ((SysTime.byWeek <<5) & 0xE0);    
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMonth;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.wYear % 100;
			}
		pRecSOE ++;    
		m_pSendsoenum[m_wEqpNo].wSendDSOENum++;
	}                 
	
	        write_VSQ(1);
			RecSoeNum=0;
	SendFrameTail();
	return TRUE;                                        	                            	
}

BOOL CGB104S::SendChangeYC(void)
{
	VDBYCF_L *pBuf = (VDBYCF_L *)m_dwPubBuf;
	WORD wReqNum = REQ_CHANGEYC_NUM;
	WORD wChangeYcNum = 0;
	float fd;
	DWORD dddd;
	BYTE Reason = COT_SPONT;
	int i;
	WORD wSendNum;
	
	if (!CanSendIFrame())
		return FALSE;
	SwitchToEqpNo(m_wChangeYcEqpNo);
	wChangeYcNum = SearchChangeYC(wReqNum, MAX_PUBBUF_LEN *4, (VDBYCF_L*)pBuf,0);
	if (!wChangeYcNum)
	{
		m_wChangeYcEqpNo++;
		if (m_wChangeYcEqpNo >= m_wEqpNum)
		{
			m_wChangeYcEqpNo = 0;	
		}
		return FALSE;
	}
		
	SendFrameHead(FRM_I);
	Sendframe(m_guiyuepara.yctype, Reason);
	for (i=0; i < wChangeYcNum; i++)
	{
		ReadYCSendNo(m_wEqpID, pBuf->wNo,&wSendNum);
	
		write_infoadd(wSendNum+ADDR_YC_LO);
		//write_infoadd(pBuf->wNo+ADDR_YC_LO);
		
		
		if(m_guiyuepara.yctype!=13)
		{
			if(pBuf->byFlag&(1<<6))
			{
				memcpy(&fd,(void*)&pBuf->lValue,4);
				dddd=fd;
			}
			else
				dddd=pBuf->lValue;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(dddd);
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(dddd);		
		}
		else
		{
			if(pBuf->byFlag&(1<<6))
			{
				memcpy(&fd,(void*)&pBuf->lValue,4);
				BYTE *p=(BYTE*)&fd;
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[0];
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[1];
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[2];
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[3];
			}
			else
			{
				YCLongToFloat(m_wEqpID,pBuf->wNo,pBuf->lValue,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
				m_SendBuf.wWritePtr+=4;
			}
		}
		
		if(m_guiyuepara.yctype!=M_ME_ND)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = QDS(pBuf->byFlag);//QDP
		if((m_guiyuepara.yctype==10)||(m_guiyuepara.yctype==12))
			write_time3();
		if((m_guiyuepara.yctype==M_ME_TD)||(m_guiyuepara.yctype==35))
			write_time();
		pBuf++;
	}	
	
	write_VSQ(i);
	SendFrameTail();
	
	return TRUE;	
}


void CGB104S::SendYkCancelAck(void)
{
    BYTE Reason = COT_DEACTCON;
	BYTE Qualifier = 1;
	BYTE *pData = m_pASDU->byInfo;
	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
	    return;
	
	SendFrameHead(FRM_I);
	
	pSendAsdu = (VASDU *)m_pSend->byASDU;
	pSendAsdu->byTypeID = m_pASDU->byTypeID;
	pSendAsdu->byReasonLow = Reason;
#ifndef INCLUDE_DF104FORMAT
	pSendAsdu->byReasonHigh = GetEqpOwnAddr();
#endif
	pSendAsdu->byQualifier = Qualifier;
	
	m_SendBuf.wWritePtr += ASDUID_LEN;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pData[0]; //信息体地址Lo
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pData[1];
#ifndef INCLUDE_DF104FORMAT
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pData[2]; //信息体地址Hi
#endif
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pData[3]; //DCO

	SendFrameTail();   
}

void CGB104S::SendAllYcYx(void)
{
	WORD i, YxGroupNum, YcGroupNum;
	
	if (!CanSendIFrame())
		return;
	
	SwitchToEqpNo(m_dwSendAllDataEqpNo);
	
	YxGroupNum = (m_pEqpInfo[m_wEqpNo].wSYXNum +m_pEqpInfo[m_wEqpNo].wVYXNum+ YX_GRP_NUM - 1) / YX_GRP_NUM;
	YcGroupNum = (m_pEqpInfo[m_wEqpNo].wYCNum + YC_GRP_NUM - 1) / YC_GRP_NUM;
	if(m_initflag)
		{
			m_initflag=0;
			//return SendInitFinish();
	}
	
	for(i=0; (i<31) && (i<YxGroupNum); i++)
	{
		if( m_dwYxSendFlag & (1<<i) )
		{
			if(SendGroupDYx(i, COT_INTROGEN))
			  return;
			if(SendGroupYx(i, COT_INTROGEN))
			  return;
			m_wSendDYxNum=0;
			m_wSendYxNum=0;
			m_dwYxSendFlag &= ~(1<<i);
		}
	}
	for(i=0; (i<31) && (i<YcGroupNum); i++)
	{
		if( m_dwYcSendFlag & (1<<i) )
		{
			if(SendGroupYc(i, COT_INTROGEN))
			  return;
			m_dwYcSendFlag &= ~(1<<i);
			m_wSendYcNum=0;
		
		}
	}
	if( m_dwYxSendFlag & 0x80000000 )
	{
		m_dwSendAllDataEqpNo++;
		if ( m_dwSendAllDataEqpNo >= m_wEqpNum)
		{
			m_dwSendAllDataEqpNo = 0;
			m_dwYxSendFlag = 0;
			m_dwYcSendFlag = 0;
			m_allcallflag=0;
			SendStopYcYx();
		}
		else
		{
			
			m_dwYxSendFlag = 0xffffffff;
			m_dwYcSendFlag = 0xffffffff;	
		}
	}	
}

#ifdef  _GUANGZHOU_TEST_VER_
BOOL CGB104S::SendAllchangeYc(void)
{
	WORD i, YcGroupNum;
	
	if (!CanSendIFrame())
		return false;
	
	SwitchToEqpNo(m_dwSendAllDataEqpNo);
	m_delaychangeyc=0;
	YcGroupNum = (m_pEqpInfo[m_wEqpNo].wYCNum + YC_GRP_NUM - 1) / YC_GRP_NUM;
	
	for(i=0; (i<31) && (i<YcGroupNum); i++)
	{
		if( m_dwchangeYcSendFlag & (1<<i) )
		{
			if(SendGroupchangeYc(i, COT_INTROGEN))
			  return true;
			m_dwchangeYcSendFlag &= ~(1<<i);
			m_wSendchangeYcNum=0;
		
		}
	}
	
	
	if( m_dwchangeYcSendFlag & 0x80000000 )
	{
		m_dwSendAllDataEqpNo++;
		if ( m_dwSendAllDataEqpNo >= m_wEqpNum)
		{
			m_dwSendAllDataEqpNo = 0;
			m_dwchangeYcSendFlag = 0;
		}
		else
		{
			
			m_dwchangeYcSendFlag = 0xffffffff;	
		}
	}	
	return false;
}
#endif

void CGB104S::SendSomeYcYx(void)
{
	WORD i, YxGroupNum, YcGroupNum;
	
	if (!CanSendIFrame())
		return;
	
	YxGroupNum = (m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum + YX_GRP_NUM - 1) / YX_GRP_NUM;
	YcGroupNum = (m_pEqpInfo[m_wEqpNo].wYCNum + YC_GRP_NUM - 1) / YC_GRP_NUM;
	
	for(i=0; (i<31) && (i<YcGroupNum); i++)
	{
		if( m_dwYcGroupFlag & (1<<i) )
		{
			if(SendGroupYc(i, i+COT_INRO9))
			return;
			m_dwYcGroupFlag &= ~(1<<i);
		}
	}
	for(i=0; (i<31) && (i<YxGroupNum); i++)
	{
		if( m_dwYxGroupFlag & (1<<i) )
		{	
			if(SendGroupDYx(i, i+COT_INRO1))
			return;
			if(SendGroupYx(i, i+COT_INRO1))
			return;
			m_wSendDYxNum=0;
			m_wSendYxNum=0;
			m_dwYxGroupFlag &= ~(1<<i);
		}
	}
	
	if( m_dwYxGroupFlag & 0x80000000 )
	{
		m_dwYxGroupFlag = 0;
		SendStopSomeYcYx();
	}	
	
	if( m_dwYcGroupFlag & 0x80000000 )
	{
		m_dwYcGroupFlag = 0;
		SendStopSomeYcYx();
	}	
}



void CGB104S::SendStopYcYx(void)
{
	BYTE TypeID = C_IC_NA;
	BYTE Reason = COT_ACTTERM;
	BYTE Qualifier = 0x1;
	
	SendFrameHead(FRM_I);
		
	Sendframe(TypeID, Reason);
		write_infoadd(0);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x14;
	
	write_VSQ(Qualifier);
	SendFrameTail();
m_allcalloverflag=1;
	return;	
}


void CGB104S::SendStopSomeYcYx(void)
{
	BYTE TypeID = C_IC_NA;
	BYTE Reason = COT_ACTTERM;
	BYTE Qualifier = 0x1;
	
	SendFrameHead(FRM_I);
		Sendframe(TypeID, Reason);
		write_infoadd(0);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_QOI;
	
	write_VSQ(Qualifier);
	SendFrameTail();
	return;	
}

//带地址的yc,可变结构限定词 不是 0x80
BOOL CGB104S::SendGroupYc_Addr(WORD YcGroupNo,BYTE Reason)
{
	WORD YcNo;
	struct VYCF_L * YcValue;
	WORD ReadYcNum = 64;
	float fd;
	DWORD dddd;
	WORD wSendNum,wSendNum1;
	BYTE Style = m_guiyuepara.yctype; //M_ME_NA;
	
	if (!CanSendIFrame())
		return FALSE;
	if((Style == M_ME_TD) || (Style == M_ME_TA))
	    Style = M_ME_NA;
	if((Style == M_ME_TE) || (Style == M_ME_TB))
	    Style = M_ME_NB;
	if((Style == M_ME_TF) || (Style == M_ME_TC))
	    Style = M_ME_NC;
	

	YcNo = YcGroupNo * YC_GRP_NUM;
	YcNo+=m_wSendYcNum;
	if ((YcNo >= m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum)||(YcNo>=(YcGroupNo+1) * YC_GRP_NUM))
		{
		m_wSendYcNum=0;
		return FALSE;
		}
		
	SendFrameHead(FRM_I);
	Sendframe(Style, Reason);

	ReadYCSendNo(m_wEqpID,YcNo,&wSendNum);
	wSendNum1 = wSendNum;
	
	//if((Style == M_ME_NA) || (Style == M_ME_NB) || (Style == M_ME_NC) || (Style == M_ME_ND))
	 //   write_infoadd(wSendNum+ ADDR_YC_LO);//write_infoadd(YcNo + ADDR_YC_LO);

	WORD i;
	ReadYcNum=	ReadRangeYCF_L(m_wEqpID, YcNo, ReadYcNum, ReadYcNum*sizeof(struct VYCF_L ), (struct VYCF_L *)m_dwPubBuf);
	YcValue=(struct VYCF_L *)m_dwPubBuf;
	for( i=0; (m_SendBuf.wWritePtr<230) &&(i+m_wSendYcNum<YC_GRP_NUM) && (i<ReadYcNum) && (YcNo+i<m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum); i++)
	{
		ReadYCSendNo(m_wEqpID,YcNo+i,&wSendNum);
		if((wSendNum - wSendNum1) <= 1)
			wSendNum1 = wSendNum;
		else
			break;
		if((Style == M_ME_NA) || (Style == M_ME_NB) || (Style == M_ME_NC) || (Style == M_ME_ND))
	 	   write_infoadd(wSendNum+ ADDR_YC_LO);//write_infoadd(YcNo + ADDR_YC_LO);
	    
	 if(Style != M_ME_NC)
		{
		if(YcValue[i].byFlag&(1<<6))
			{
		memcpy(&fd,(void*)&YcValue[i].lValue,4);
				dddd=fd;
			}
		else
			dddd=YcValue[i].lValue;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(dddd);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(dddd);
		#ifdef _TEST_VER_
		if(dddd>65536)
			{
			YcValue[i].byFlag|=0x18;
			YcValue[i].byFlag&=0xfe;
			}
		#endif
		}
	else
		{
		if(YcValue[i].byFlag&(1<<6))
		{
			memcpy(&fd,(void*)&YcValue[i].lValue,4);
			BYTE *p=(BYTE*)&fd;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[0];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[1];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[2];
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[3];
		}
		else
		{
			YCLongToFloat(m_wEqpID,YcNo + i,YcValue[i].lValue,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
			m_SendBuf.wWritePtr+=4;
		}
		

	}
		if(Style != M_ME_ND)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = QDS(YcValue[i].byFlag);//QDP
		m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i+YcNo]=false;
		m_pEqpExtInfo[m_wEqpNo].OldYC[i+YcNo].lValue = m_pEqpExtInfo[m_wEqpNo].CurYC[i+YcNo].lValue = YcValue[i].lValue;
		m_pEqpExtInfo[m_wEqpNo].OldYC[i+YcNo].byFlag = m_pEqpExtInfo[m_wEqpNo].CurYC[i+YcNo].byFlag = YcValue[i].byFlag;
	}
		write_VSQ(i);
	m_wSendYcNum+= i;
	SendFrameTail();
	#if 0	
	if ((YC_FRM_NUM < YC_GRP_NUM) && (m_wSendYcNum != 0) && (YcNo < m_pEqpInfo[m_wEqpNo].wYCNum))
	{
		SendGroupYcContinue(YcGroupNo, Reason);
		m_wSendYcNum = 0;
	}
	else
	{

		return TRUE;
	}
	#endif
	return TRUE;
}


BOOL CGB104S::SendGroupYc(WORD YcGroupNo,BYTE Reason)
{
	WORD YcNo;
	struct VYCF_L * YcValue;
	WORD ReadYcNum = 64;
	BYTE VSQ=0x80;
	float fd;
	DWORD dddd;
	WORD wSendNum,wSendNum1;
	BYTE Style = m_guiyuepara.yctype; //M_ME_NA;
	
	if (!CanSendIFrame())
		return FALSE;
	if((Style == M_ME_TD) || (Style == M_ME_TA))
	    Style = M_ME_NA;
	if((Style == M_ME_TE) || (Style == M_ME_TB))
	    Style = M_ME_NB;
	if((Style == M_ME_TF) || (Style == M_ME_TC))
	    Style = M_ME_NC;
	

	YcNo = YcGroupNo * YC_GRP_NUM;
	YcNo+=m_wSendYcNum;
	if ((YcNo >= m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum)||(YcNo>=(YcGroupNo+1) * YC_GRP_NUM))
		{
		m_wSendYcNum=0;
		return FALSE;
		}
		
	SendFrameHead(FRM_I);
	Sendframe(Style, Reason);

	ReadYCSendNo(m_wEqpID,YcNo,&wSendNum);
	wSendNum1 = wSendNum;
	
	if(((Style == M_ME_NA) || (Style == M_ME_NB) || (Style == M_ME_NC) || (Style == M_ME_ND)) && (m_guiyuepara.tmp[3] == 0))
	    write_infoadd(wSendNum+ ADDR_YC_LO);//write_infoadd(YcNo + ADDR_YC_LO);

	WORD i;
	ReadYcNum=	ReadRangeYCF_L(m_wEqpID, YcNo, ReadYcNum, ReadYcNum*sizeof(struct VYCF_L ), (struct VYCF_L *)m_dwPubBuf);
	YcValue=(struct VYCF_L *)m_dwPubBuf;
	for( i=0; (m_SendBuf.wWritePtr<230) &&(i+m_wSendYcNum<YC_GRP_NUM) && (i<ReadYcNum) && (YcNo+i<m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum); i++)
	{
		ReadYCSendNo(m_wEqpID,YcNo+i,&wSendNum);
		
		if(m_guiyuepara.tmp[3] == 0) // 0不带地址连续发生，1按地址发送
		{	
			if((wSendNum - wSendNum1) <= 1)
				wSendNum1 = wSendNum;
			else
				break;
		}
		else
		{
			write_infoadd(wSendNum + ADDR_YC_LO);
			VSQ = 0;
		}
	
	 if(Style != M_ME_NC)
		{
		if(YcValue[i].byFlag&(1<<6))
			{
		memcpy(&fd,(void*)&YcValue[i].lValue,4);
				dddd=fd;
			}
		else
			dddd=YcValue[i].lValue;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(dddd);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(dddd);
		#ifdef _TEST_VER_
		if(dddd>65536)
			{
			YcValue[i].byFlag|=0x18;
			YcValue[i].byFlag&=0xfe;
			}
		#endif
		}
		else
		{
			if(YcValue[i].byFlag&(1<<6))
			{
				memcpy(&fd,(void*)&YcValue[i].lValue,4);
				BYTE *p=(BYTE*)&fd;
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[0];
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[1];
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[2];
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =p[3];
			}
			else
			{

				YCLongToFloat(m_wEqpID,YcNo + i,YcValue[i].lValue,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
				m_SendBuf.wWritePtr+=4;
			}
		}
		if(Style != M_ME_ND)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = QDS(YcValue[i].byFlag);//QDP
		m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[YcNo+i]=false;
		if(COT_INTROGEN == Reason)
		{
			m_pEqpExtInfo[m_wEqpNo].OldYC[YcNo+i].lValue = m_pEqpExtInfo[m_wEqpNo].CurYC[YcNo+i].lValue = YcValue[i].lValue;
			m_pEqpExtInfo[m_wEqpNo].OldYC[YcNo+i].byFlag = m_pEqpExtInfo[m_wEqpNo].CurYC[YcNo+i].byFlag = YcValue[i].byFlag;
		}
	}
		write_VSQ(i | VSQ);
	m_wSendYcNum+= i;
	SendFrameTail();
	#if 0	
	if ((YC_FRM_NUM < YC_GRP_NUM) && (m_wSendYcNum != 0) && (YcNo < m_pEqpInfo[m_wEqpNo].wYCNum))
	{
		SendGroupYcContinue(YcGroupNo, Reason);
		m_wSendYcNum = 0;
	}
	else
	{

		return TRUE;
	}
	#endif
	return TRUE;
}

#ifdef  _GUANGZHOU_TEST_VER_
BOOL CGB104S::SendGroupchangeYc(WORD YcGroupNo,BYTE Reason)
{
	//BYTE TypeID = M_ME_NA;
	WORD YcNo;
	struct VYCF_L* YcValue;
//	WORD ReadYcNum = 64;
	BYTE VSQ=0x80;
	if (!CanSendIFrame())
		return FALSE;

	YcNo = m_wSendchangeYcNum;
	if (YcNo >= m_pEqpInfo[m_wEqpNo].wYCNum)
		{
		m_wSendchangeYcNum=0;
		return FALSE;
		}
		if(m_guiyuepara.yctype==13)
			return FALSE;
	SendFrameHead(FRM_I);
	Sendframe(m_guiyuepara.yctype, 3);
	if((m_guiyuepara.yctype==21)||(m_guiyuepara.yctype==9)||(m_guiyuepara.yctype==11)||(m_guiyuepara.yctype==13))
	write_infoadd(YcNo + ADDR_YC_LO);

	WORD i;
	YcValue=m_pEqpExtInfo[m_wEqpNo].OldYC+m_wSendchangeYcNum;
	for( i=0; (m_SendBuf.wWritePtr<230)   && (m_wSendchangeYcNum+i<m_pEqpInfo[m_wEqpNo].wYCNum); i++)
	{
	if((m_guiyuepara.yctype==35)||(m_guiyuepara.yctype==10)||(m_guiyuepara.yctype==12)||(m_guiyuepara.yctype==34))
	write_infoadd(YcNo +i+ ADDR_YC_LO);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(YcValue[i].lValue);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(YcValue[i].lValue);
	
	
		if(m_guiyuepara.yctype!=M_ME_ND)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = QDS(YcValue[i].byFlag);//QDP
		if((m_guiyuepara.yctype==10)||(m_guiyuepara.yctype==12))
			write_time3();
		if((m_guiyuepara.yctype==M_ME_TD)||(m_guiyuepara.yctype==35))
			write_time();
		m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=false;
	}
		if((m_guiyuepara.yctype==35)||(m_guiyuepara.yctype==34)||(m_guiyuepara.yctype==10)||(m_guiyuepara.yctype==12))
			VSQ=0;
		write_VSQ(i | VSQ);
	m_wSendchangeYcNum+= i;
	SendFrameTail();
	
	return TRUE;
}
#endif

#if 0
BOOL CGB104S::SendGroupYcContinue(WORD YcGroupNo,BYTE Reason)
{
	BYTE TypeID = M_ME_NA;
	WORD YcNo;
	WORD YcValue;
	WORD ReadYcNum = 1;
	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return FALSE;

	YcNo = YcGroupNo * YC_GRP_NUM + m_wSendYcNum;
	if (YcNo >= m_pEqpInfo[m_wEqpNo].wYCNum)
		return FALSE;
		
	SendFrameHead(FRM_I);
	
	pSendAsdu = (VASDU *)m_pSend->byASDU;
	pSendAsdu->byTypeID = TypeID;
	pSendAsdu->byReasonLow = Reason;

	pSendAsdu->byReasonHigh = GetEqpOwnAddr();
	pSendAsdu->byAddressLow = GetEqpOwnAddr();
	pSendAsdu->byAddressHigh =0;


	m_SendBuf.wWritePtr += ASDUID_LEN;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YcNo + ADDR_YC_LO); //信息体地址Lo
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YcNo + ADDR_YC_LO); 

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi

	WORD i;
	for(i=0; (i<(YC_GRP_NUM-m_wSendYcNum)) && (YcNo<m_pEqpInfo[m_wEqpNo].wYCNum); i++, YcNo++)
	{
		ReadRangeYC(m_wEqpID, YcNo, ReadYcNum, 2/*sizeof(WORD)*/, (short *)&YcValue);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(YcValue);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(YcValue);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;//QDS
	}
	
	pSendAsdu->byQualifier = i | VSQ_SQ;
	m_wSendYcNum = 0;
	SendFrameTail();
		
	return TRUE;
}
#endif
BOOL CGB104S::SendGroupYx(WORD YxGroupNo,BYTE Reason)
{
	BYTE TypeID = M_SP_NA;
	WORD YxNo;
	WORD sendyxnum = 0;
	WORD wSendNum,wSendNum1;
	//VASDU * pSendAsdu;
	BYTE VSQ=0x80;
	struct VDBYX DBYX;
		
	if (!CanSendIFrame())
		return FALSE;

	YxNo = YxGroupNo * YX_GRP_NUM;
	YxNo+=m_wSendYxNum;
	
	if((m_wSendYxNum+m_wSendDYxNum>=YX_GRP_NUM)|| (YxNo >= m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum))
		{
		return FALSE;
		}
//	if(m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum>256)
//		return SendYXM_PS_NA( YxGroupNo,  Reason);
		
	SendFrameHead(FRM_I);

if(m_guiyuepara.yxtype==1)
		TypeID=1;
	if(m_guiyuepara.yxtype==2)
		TypeID=2;
	if(m_guiyuepara.yxtype==3)
		TypeID=3;
	Sendframe(TypeID,Reason);
	
	WORD i;
	
	ReadSYXSendNo(m_wEqpID,YxNo,&wSendNum);
	wSendNum1 = wSendNum - 1;
	
	if(((m_guiyuepara.yxtype==1)||(m_guiyuepara.yxtype==3))&&(m_guiyuepara.tmp[3] == 0))
		write_infoadd(wSendNum + ADDR_YX_LO);
	
	for(i=0;(m_SendBuf.wWritePtr<230)&&(i+m_wSendYxNum+m_wSendDYxNum<YX_GRP_NUM)&& (i<YX_FRM_NUM) && (YxNo<m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum); i++, YxNo++)
	{
	DBYX.wNo = YxNo;
	ReadSYX(m_wEqpID,1, sizeof(struct VDBYX),&DBYX);
	ReadSYXSendNo(m_wEqpID,YxNo,&wSendNum);
		
	if(m_guiyuepara.tmp[3] == 0) // 0不带地址连续发送，1则按地址发送
	{
		if(wSendNum == wSendNum1)
			continue;
		else if(wSendNum == (WORD)(wSendNum1 + 1))
			wSendNum1 = wSendNum;
		else
			break;
		if((m_guiyuepara.yxtype!=1)&&(m_guiyuepara.yxtype!=3))
			write_infoadd(wSendNum+ ADDR_YX_LO);
  }
	else
	{			
		if(wSendNum != wSendNum1 )
			wSendNum1 = wSendNum;
		else
			continue;
		if((m_guiyuepara.yxtype==1)||(m_guiyuepara.yxtype==3))	
			write_infoadd(wSendNum + ADDR_YX_LO);
     	VSQ = 0;
	}

#if (TYPE_USER == USER_SHANGHAIJY)
		if(m_guiyuepara.yxtype==3)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr] = (SIQ(DBYX.byValue)+1)& 0x03;
		else
			m_SendBuf.pBuf[m_SendBuf.wWritePtr] = (SIQ(DBYX.byValue))& 0x03;
#else
		if(m_guiyuepara.yxtype==3)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr] = SIQ(DBYX.byValue)+1;
		else
			m_SendBuf.pBuf[m_SendBuf.wWritePtr] = SIQ(DBYX.byValue);
#endif
		m_SendBuf.wWritePtr++;
		if(TypeID==2)
			write_time3();
		if(TypeID==30)
			write_time();
		sendyxnum ++;
	}
		if((m_guiyuepara.yxtype!=1)&&(m_guiyuepara.yxtype!=3))
			VSQ=0;
	write_VSQ(sendyxnum | VSQ);
	m_wSendYxNum+= i;
	SendFrameTail();
		
	
	return TRUE;
}
BOOL CGB104S::SendYXM_PS_NA(WORD YxGroupNo,BYTE Reason)
{
	BYTE TypeID = M_PS_NA;
	WORD YXNo;
//	BYTE YXValue;
	WORD ReadYXNum = m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum;
	//VASDU * pSendAsdu;
	BYTE VSQ=0x80;
	BYTE Bitstate[256];
if (!CanSendIFrame())
		return FALSE;

	YXNo = YxGroupNo*736;
	YXNo+=m_wSendYxNum;

	ReadYXNum = ReadRangeSYXBit(m_wEqpID, YXNo, ReadYXNum, 256,Bitstate);
	if( (YXNo >= m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum) || (ReadYXNum== 0))
		{
		return FALSE;
		}
		
	SendFrameHead(FRM_I);
		Sendframe(TypeID,Reason);
		write_infoadd(YXNo + ADDR_YX_LO+m_wSendDYxNum);

	WORD i;

	for(i=0;i<=(ReadYXNum-1)/16; i++)
	{
			//ReadRangeSYX(m_wEqpID, i*16, 1, 1,&YXValue);
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] =Bitstate[i*2+m_wSendYxNum/16];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] =Bitstate[i*2+1+m_wSendYxNum/16];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] =Bitchange[i*2+m_wSendYxNum/16];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] =Bitchange[i*2+1+m_wSendYxNum/16];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] =0;// SIQ(YXValue )&0xf0;
		if(i>45) break;
	}
			memset(Bitchange,0,256);
	write_VSQ(i | VSQ);
	m_wSendYxNum+= ReadYXNum;
	SendFrameTail();
		
	
	return TRUE;
}
BOOL CGB104S::SendGroupDYx(WORD YxGroupNo,BYTE Reason)
{
	BYTE TypeID = M_DP_NA;
	WORD YxNo;
	struct VDYX* YxValue;
	WORD ReadYxNum = 1;
	BYTE VSQ=0x80;
	WORD wSendNum,wSendNum1;
	
	if (!CanSendIFrame())
		return FALSE;

	YxNo = YxGroupNo * YX_GRP_NUM;
	YxNo+=m_wSendDYxNum;
	if((m_wSendDYxNum>=YX_GRP_NUM)||  (YxNo >= m_pEqpInfo[m_wEqpNo].wDYXNum))
		{
		return FALSE;
		}
		
	SendFrameHead(FRM_I);
	if(m_guiyuepara.yxtype==1)
		TypeID=3;

	Sendframe(TypeID,Reason);

	WORD i;
	ReadDYXSendNo( m_wEqpID, YxNo, & wSendNum);
	wSendNum1 = wSendNum - 1;
	
	if(((m_guiyuepara.yxtype==1)||(m_guiyuepara.yxtype==3))&&(m_guiyuepara.tmp[3] == 0))
		write_infoadd(wSendNum + ADDR_YX_LO);
	for(i=0;(m_SendBuf.wWritePtr<230)&& (i+m_wSendDYxNum<YX_GRP_NUM)&&(i<YX_FRM_NUM) && (YxNo<m_pEqpInfo[m_wEqpNo].wDYXNum); i++, YxNo++)
	{		
		ReadDYXSendNo(m_wEqpID,YxNo,&wSendNum); 
		if(m_guiyuepara.tmp[3] == 0) // 0 不带地址，连续发送
		{
			if(wSendNum == wSendNum1)
				continue;
			else if(wSendNum == (WORD)(wSendNum1 + 1))//有效点
				wSendNum1 = wSendNum;		
			else 
				break;
		}
		else
		{	
			if(wSendNum != wSendNum1)
				wSendNum1 = wSendNum;
			else
				continue;
				write_infoadd(wSendNum + ADDR_YX_LO);	
			VSQ = 0;
		}
		
		ReadRangeDYX(m_wEqpID, YxNo, ReadYxNum, sizeof(struct VDYX ), (struct VDYX *)m_dwPubBuf);
		YxValue=(struct VDYX *)m_dwPubBuf;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr ] = DIQ(YxValue->byValue1,YxValue->byValue2);
		m_SendBuf.wWritePtr++;
		if(TypeID==4)
			write_time3();
		if(TypeID==31)
			write_time();
	}
	/*if(m_guiyuepara.yxtype!=1)
		VSQ=0;*/
	write_VSQ(i | VSQ);
	m_wSendDYxNum+= i;
	SendFrameTail();
		
	
	return TRUE;
}
#if 0
BOOL CGB104S::SendGroupYxContinue(WORD YxGroupNo,BYTE Reason)
{
	BYTE TypeID = M_SP_NA;
	WORD YxNo;
	BYTE YxValue;
	WORD ReadYxNum = 1;
	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return FALSE;

	YxNo = YxGroupNo * YX_GRP_NUM + m_wSendYxNum;
	if (YxNo >= m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum)
		return FALSE;
		
	SendFrameHead(FRM_I);
	
	pSendAsdu = (VASDU *)m_pSend->byASDU;
	pSendAsdu->byTypeID = TypeID;
	pSendAsdu->byReasonLow = Reason;
#ifndef INCLUDE_DF104FORMAT
	pSendAsdu->byReasonHigh = GetEqpOwnAddr();
#endif
#if(TYPE_USER == USER_GDTEST)
	pSendAsdu->byAddressLow = GetEqpOwnAddr();
	pSendAsdu->byAddressHigh =0;
#endif
		
	m_SendBuf.wWritePtr += ASDUID_LEN;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YxNo + ADDR_YX_LO); //信息体地址Lo
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YxNo + ADDR_YX_LO); 
#ifndef INCLUDE_DF104FORMAT
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
#endif
	WORD i;
	for(i=0; (i<(YX_GRP_NUM-m_wSendYxNum)) && (YxNo<m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum); i++, YxNo++)
	{
		ReadRangeSYX(m_wEqpID, YxNo, ReadYxNum, 1/*sizeof(BYTE)*/, &YxValue);
		YxValue = YxValue & 0x80;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = YxValue ? 1 : 0;
	}
	
	pSendAsdu->byQualifier = i | VSQ_SQ;
	m_wSendYcNum = 0;
	SendFrameTail();
		
	return TRUE;
}


#endif


BOOL CGB104S::SendGroupDd(WORD DdGroupNo,BYTE Reason)
{
//	BYTE TypeID = M_IT_NA;
	BYTE Style = 0x0F;
	WORD DdNo;
	WORD wSendNum;
	DWORD* DdValue;
	WORD ReadDDNum = 25;
//	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return FALSE;

	DdNo = DdGroupNo * DD_GRP_NUM;
	if (DdNo >= m_pEqpInfo[m_wEqpNo].wDDNum)
		return FALSE;
	if(m_byDdQcc&0x40)
		Reason=3;
	SendFrameHead(FRM_I);
	if(m_guiyuepara.ddtype==37)
		Style=37;
	Sendframe(Style, Reason);
	
	WORD i;
	ReadDDNum=	ReadRangeDD(m_wEqpID, DdNo, ReadDDNum, ReadDDNum*sizeof(DWORD), (long *)m_dwPubBuf);
	DdValue=m_dwPubBuf;
	for(i=0;(m_SendBuf.wWritePtr<230)&& (i<ReadDDNum) && ((DdNo+i)<m_pEqpInfo[m_wEqpNo].wDDNum); i++)
	{
		ReadDDSendNo(m_wEqpID,DdNo+i,&wSendNum);
		write_infoadd(wSendNum+ADDR_DD_LO);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(DdValue[i]));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(DdValue[i]));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(DdValue[i]));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(DdValue[i]));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = i;//顺序号
		if(Style==37)
			write_time();
	}
	write_VSQ(i);
	m_wSendDdNum += i;
	SendFrameTail();
	return TRUE;
	#if 0
	if ((DD_FRM_NUM < DD_GRP_NUM) && (m_wSendDdNum != 0) && (DdNo < m_pEqpInfo[m_wEqpNo].wDDNum))
	{
		SendGroupDdContinue(DdGroupNo, Reason);
		m_wSendDdNum = 0;
	}
	else
	{
		return TRUE;
	}
	
	return TRUE;
	#endif
}



BOOL CGB104S::SendGroupDdContinue(WORD DdGroupNo,BYTE Reason)
{
//	BYTE TypeID = M_IT_NA;
	WORD DdNo;
	DWORD DdValue;
	WORD ReadDDNum = 1;
//	VASDU * pSendAsdu;
	
	if (!CanSendIFrame())
		return FALSE;

	DdNo = DdGroupNo * DD_GRP_NUM + m_wSendDdNum;
	if (DdNo >= m_pEqpInfo[m_wEqpNo].wDDNum)
		return FALSE;
	
	SendFrameHead(FRM_I);
	
	Sendframe(M_IT_NA, Reason);
	WORD i;
	for(i=0; (i<(DD_GRP_NUM - m_wSendDdNum)) && (DdNo < m_pEqpInfo[m_wEqpNo].wDDNum); i++)
	{
		write_infoadd(DdNo+i+ADDR_DD_LO);
		ReadRangeDD(m_wEqpID, (DdNo+i), ReadDDNum, 4/*sizeof(DWORD)*/, (long *)&DdValue);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(DdValue));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(DdValue));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(DdValue));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(DdValue));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = i + m_wSendDdNum;//顺序号
	}
	
	write_VSQ(i);
	m_wSendDdNum = 0;
	SendFrameTail();
	
	return TRUE;
}

void CGB104S::SendCallYcYxAck(void)
{
//	BYTE TypeID = C_IC_NA;
//	BYTE Reason = COT_ACTCON;
	BYTE Qualifier = 0x1;
//	VASDU * pSendAsdu;
	
	if (CanSendIFrame())
	{
		SendFrameHead(FRM_I);
		if((m_dwasdu.COT==6)||(m_dwasdu.COT==8))
		Sendframe(C_IC_NA, m_dwasdu.COT+1);
		else
			Sendframe(C_IC_NA, m_dwasdu.COT);
		write_infoadd(0);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_QOI;
		
		write_VSQ(Qualifier);
		SendFrameTail();
	}
	return;
}


void CGB104S::SendSomeYcYxAck(BYTE byQrp)
{
	BYTE TypeID = C_IC_NA;
	BYTE Reason = COT_ACTCON;
	BYTE Qualifier = 0x1;
//	VASDU * pSendAsdu;
	
	if (CanSendIFrame())
	{
		SendFrameHead(FRM_I);
		
		Sendframe(TypeID, Reason);
		write_infoadd(0);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = byQrp;
		write_VSQ(Qualifier);
//		pSendAsdu->byQualifier = Qualifier;
		SendFrameTail();
	}
	return;
}

void CGB104S::SendCallDdAck(void)
{
	BYTE TypeID = C_CI_NA;
	BYTE Reason = COT_ACTCON;
	BYTE Qualifier = 0x1;
	
	if (CanSendIFrame())
	{
		SendFrameHead(FRM_I);
		Sendframe(TypeID, Reason);
		write_infoadd(0);
		
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_byDdQcc;//for test lqh
		
		write_VSQ(Qualifier);
		SendFrameTail();
	}
	return;		
}


void CGB104S::SendAllDd(void)
{
	if (!CanSendIFrame())
		return;
		
	SwitchToEqpNo(m_wAllDdEqpNo);
	
	WORD i, DdGroupNum;
	DdGroupNum = (m_pEqpInfo[m_wEqpNo].wDDNum + DD_GRP_NUM-1)/DD_GRP_NUM;
	
	for(i=0; (i<31) && (i < DdGroupNum); i++)
	{
		if( m_dwDdSendFlag & (1<<i) )
		{
			m_dwDdSendFlag &= ~(1<<i);
			SendGroupDd(i, COT_REQCOGCN);
			return;
		}
	}
	if (m_dwDdSendFlag & 0x80000000)
	{
		m_wAllDdEqpNo ++ ;
		if (m_wAllDdEqpNo >= m_wEqpNum)
		{
			m_wAllDdEqpNo = 0;
			m_dwDdSendFlag = 0;
			SendDdStop();
		}
		else
		{
			
			m_dwDdSendFlag = 0xffffffff;
		}
	}		
}


void CGB104S::SendSomeDd(void)
{
	if (!CanSendIFrame())
		return;
	
	WORD i, DdGroupNum;
	DdGroupNum = (m_pEqpInfo[m_wEqpNo].wDDNum + DD_GRP_NUM-1)/DD_GRP_NUM;
	
	for(i=0; (i<31) && (i < DdGroupNum); i++)
	{
		if( m_dwDdGroupFlag & (1<<i) )
		{
			m_dwDdGroupFlag &= ~(1<<i);
			SendGroupDd(i, i + COT_REQCO1);
			return;
		}
	}
	if (m_dwDdGroupFlag & 0x80000000)
	{
		m_dwDdGroupFlag = 0;
		SendDdStop();
	}	
}


void CGB104S::SendDdStop(void)
{
	BYTE TypeID = C_CI_NA;
	BYTE Reason = COT_ACTTERM;
	BYTE Qualifier = 0x1;
	VASDU * pSendAsdu;
	
	SendFrameHead(FRM_I);
		
	pSendAsdu = (VASDU *)m_pSend->byASDU;
	pSendAsdu->byTypeID = TypeID;
	pSendAsdu->byReasonLow = Reason;
#ifndef INCLUDE_DF104FORMAT
	pSendAsdu->byReasonHigh = GetEqpOwnAddr();
#endif

	m_SendBuf.wWritePtr += ASDUID_LEN;
		
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
#ifndef INCLUDE_DF104FORMAT
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
#endif
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_byDdQcc;//for test lqh
	
	pSendAsdu->byQualifier = Qualifier;
	SendFrameTail();
	return;
}


void CGB104S::SendReadYx(WORD wYxNo)
{
	BYTE TypeID = M_SP_NA;
//	BYTE Reason = COT_REQ;
	BYTE Qualifier = 0x1;
//	VASDU * pSendAsdu;
	
	WORD ReadYxNum = 1;
	BYTE YxValue;
	
	if (!CanSendIFrame())
		return;
		
	ReadRangeSYX(m_wEqpID, wYxNo, ReadYxNum, 1, &YxValue);
	
		
	SendFrameHead(FRM_I);
		Sendframe(TypeID, m_dwasdu.COT);
		write_infoadd(wYxNo + ADDR_YX_LO);
		
#if (TYPE_USER == USER_SHANGHAIJY)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(YxValue)& 0x03;
#else
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(YxValue);
#endif
	write_VSQ(Qualifier);
	SendFrameTail();
	return;
}

void CGB104S::SendReadYc(WORD wYcNo)
{
	BYTE TypeID = M_ME_NA;
//	BYTE Reason = COT_REQ;
	BYTE Qualifier = 0x1;
//	VASDU * pSendAsdu;
	
	WORD ReadYcNum = 1;
	struct VYCF  YcValue;
	
	if (!CanSendIFrame())
		return;
	
	ReadRangeYCF(m_wEqpID, wYcNo, ReadYcNum, sizeof(struct VYCF), &YcValue);
	
	SendFrameHead(FRM_I);
		Sendframe(TypeID, m_dwasdu.COT);
	write_infoadd(wYcNo+ADDR_YC_LO);

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(YcValue.nValue);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(YcValue.nValue);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = QDS(YcValue.byFlag);
	write_VSQ(Qualifier);
	SendFrameTail();
	return;
}
void CGB104S::SendReadYcpara(WORD wYcNo)
{
	BYTE TypeID = P_ME_NA;
//	BYTE Reason = COT_REQ;
	BYTE Qualifier = 0x4;
//	VASDU * pSendAsdu;
	
//	WORD ReadYcNum = 1;
	BYTE*  YcValue;
	
	if (!CanSendIFrame())
		return;
	
	
	SendFrameHead(FRM_I);
		Sendframe(TypeID, m_dwasdu.COT);
	write_infoadd(wYcNo+ADDR_YCPARA_LO);
		YcValue=(BYTE*)limitpara+(limitpara[1+2]+(wYcNo)*sizeof(Vlimitpara));
	for(int i=1;i<5;i++)
		{
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = *(YcValue+1);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = *YcValue;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = i;
	YcValue+=2;
		}
	write_VSQ(Qualifier);
	SendFrameTail();
	return;
}


void CGB104S::SendReadDd(WORD wDdNo)
{
	BYTE TypeID = M_IT_NA;
	BYTE Reason = COT_REQ;
	BYTE Qualifier = 0x1;
	VASDU * pSendAsdu;
	
	WORD ReadDdNum = 1;
	DWORD DdValue;
	
	if (!CanSendIFrame())
		return;
	
	ReadRangeDD(m_wEqpID, wDdNo, ReadDdNum, 4/*sizeof(DWORD)*/, (long *)&DdValue);
	
	SendFrameHead(FRM_I);
		
	pSendAsdu = (VASDU *)m_pSend->byASDU;
	pSendAsdu->byTypeID = TypeID;
	pSendAsdu->byReasonLow = Reason;
#ifndef INCLUDE_DF104FORMAT
	pSendAsdu->byReasonHigh = GetEqpOwnAddr();
#endif

	m_SendBuf.wWritePtr += ASDUID_LEN;
		
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(wDdNo + ADDR_DD_LO);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(wDdNo + ADDR_DD_LO);
#ifndef INCLUDE_DF104FORMAT
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
#endif
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(DdValue));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(DdValue));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(DdValue));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(DdValue));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
	
	pSendAsdu->byQualifier = Qualifier;
	SendFrameTail();
	return;
}


void CGB104S::SendNoData(void)
{
	SendSFrame();//???
	
}


void CGB104S::DelAckFrame(WORD SendNum)
{
	WORD i;
	for(i = 0; i < m_PARA_K; i++)
	{
		if( m_vBackFrame[i].wSendNum == SendNum )
		{
			m_vBackFrame[i].wSendNum = 0xffff;
			return;
		}
	}	
}


void CGB104S::SaveSendFrame(WORD SendNum, WORD FrameLen, BYTE *pFrame)
{
	WORD i;
	for(i = 0; i < m_PARA_K; i++)
	{
		if( m_vBackFrame[i].wSendNum == 0xffff )
		{
			m_vBackFrame[i].wSendNum = SendNum;
			m_vBackFrame[i].wFrameLen = FrameLen;
			memcpy(m_vBackFrame[i].byBuf, pFrame, FrameLen);
			return;
		}
	}	
}


BOOL CGB104S::CanSendIFrame(void)
{
	if((m_guiyuepara.tmp[0] & SWITCHALLCALL_BIT) == 0)
	{
    return (m_bDTEnable && m_bContinueSend);    
	}
	else
	{
		return TRUE;
	}  
}


BOOL CGB104S::DoFaSim(void)
{
//	VMsg *pMsg=(VMsg *)m_pASDU->byInfo;
	
    #if (_BYTE_ORDER == _BIG_ENDIAN)  //Moto系统
		pMsg->Head.wLength=ADJUSTWORD(pMsg->Head.wLength);
    #endif	
							
//	TaskSendMsg(FAID, m_wTaskID, 0xFFFF, pMsg->Head.byMsgID, pMsg->Head.byMsgAttr, pMsg->Head.wLength-sizeof(VMsgHead), pMsg);			
	
	return TRUE;	
}


BOOL CGB104S::RecYTCommand(void)
{
	BYTE * pData = m_pASDU->byInfo;
	WORD  YTNo, YTValue; 
//	VDBYT *pDBYT;

	YTNo = MAKEWORD(pData[0], pData[1]);//高字节强制为0
	YTValue = MAKEWORD(pData[3], pData[4]);
				
	YTNo = YTNo - ADDR_YT_LO + 1;
	GotoEqpbyAddress(m_wRecAddress);

	VYTInfo *pYTInfo;
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	
	pYTInfo->Head.wEqpID = m_wEqpID;
	pYTInfo->Info.wID = YTNo;
	pYTInfo->Head.byMsgID = MI_YTOPRATE;
	pYTInfo->Info.dwValue = YTValue;
	//pYTInfo->abyRsv[0] = m_pASDU->byTypeID;
//	pYTInfo->abyRsv[1] = pData[0];
	//pYTInfo->abyRsv[2] = pData[1];
	//pYTInfo->abyRsv[3] = pData[5];
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	//p104ykinfo=&p104ykinfo[m_pEqpInfo[m_wEqpNo].wYKNum];
	pCtrlInfo->Type=m_pASDU->byTypeID;
	pCtrlInfo->DOT=MAKEWORD(pData[0], pData[1]);
	pCtrlInfo->DCO=pData[m_guiyuepara.infoaddlen];
	CPSecondary::DoYTReq();
    	return TRUE;
}


BOOL CGB104S::DoYTRet(void)
{
	CPSecondary::DoYTRet();
	if (CheckClearEqpFlag(EQPFLAG_YTRet))
	{
		SendYtReturn();
		return TRUE;
	}
	else
	    return FALSE;
}
void CGB104S::SendInitFinish(void)
{
//BYTE Reason = 4;
//	BYTE Qualifier = 1;
//	VYTInfo *pYTInfo;
//	VASDU *pSendAsdu;
//	WORD YTNo;
//	WORD YTValue;
//	WORD *pwWord = NULL;
	SendFrameHead(FRM_I);
	if(m_initflag)
		{
			m_initflag=0;
	}
	Sendframe(70, 4);
	write_VSQ(1);

	write_infoadd(0);
	if((g_Sys.ResetInfo.code&0xff)==1)
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] =1;
	else if((g_Sys.ResetInfo.code&0xff)==2)
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] =2;
	else 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] =0;
	SendFrameTail();  

}


void CGB104S::SendYtReturn(void)
{
	BYTE Reason = COT_ACTCON;
	BYTE Qualifier = 1;
	WORD ytvalue,byMs;
	VSysClock SysTime;
	VYTInfo *pYTInfo;
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	 	
	if (!CanSendIFrame())
	   return;
	if (pYTInfo->Head.byMsgID == MI_YTCANCEL)
	{
		Reason = COT_DEACTCON;	
	}
	
	SendFrameHead(FRM_I);
	if(pYTInfo->Info.byStatus)
	{
		Reason|=0x40;
	}
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	Sendframe(pCtrlInfo->Type, Reason);
	write_VSQ(Qualifier);
	write_infoadd(pCtrlInfo->DOT);
	if((pCtrlInfo->Type == C_SE_NA)||(pCtrlInfo->Type == C_SE_TA_1))
	{
		ytvalue = pYTInfo->Info.dwValue;
		SendWordLH(ytvalue);
	}
	else if((pCtrlInfo->Type == C_SE_NC_1)||(pCtrlInfo->Type == C_SE_TC_1))
		SendDWordLH(pYTInfo->Info.dwValue);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pCtrlInfo->DCO;
	//m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pCtrlInfo->DCO>>8;

	if((pCtrlInfo->Type == C_SE_TA_1)||(pCtrlInfo->Type == C_SE_TC_1))
	{
		GetSysClock((void *)&SysTime, SYSCLOCK);
		byMs = SysTime.bySecond*1000+SysTime.wMSecond;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(byMs);
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(byMs);
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byHour;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byDay;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMonth;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.wYear-2000;
	}

	SendFrameTail();  

	if (pYTInfo->Head.byMsgID != MI_YTCANCEL)
		SendYtstop();
}
void CGB104S::initpara(void)
{
	if((m_guiyuepara.typeidlen<1)||(m_guiyuepara.typeidlen>2))
		{
		m_guiyuepara.typeidlen=1;
	m_guiyuepara.conaddrlen=2;
	m_guiyuepara.VSQlen=1;
	m_guiyuepara.COTlen=2;
	m_guiyuepara.infoaddlen=3;
	m_guiyuepara.baseyear=2000;
	m_guiyuepara.ddtype=15;
	m_guiyuepara.yctype=9;
	m_guiyuepara.yxtype=1;
	m_guiyuepara.k=12;
	m_guiyuepara.w=8;
		m_guiyuepara.t0=10;
		m_guiyuepara.t1=12;
		m_guiyuepara.t2=5;
		m_guiyuepara.t3=15;
		m_guiyuepara.t4=8;

	}
}
void CGB104S::sendasdu(void)
{

}
BOOL CGB104S::getasdu(void)
{	
	BYTE off=0;
	if(m_guiyuepara.typeidlen==1)
	{
		m_dwasdu.TypeID=m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.typeidlen==2)
	{
		m_dwasdu.TypeID=MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
			off+=2;
	}
	if(m_guiyuepara.VSQlen==1)
	{
		m_dwasdu.VSQ=m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.VSQlen==2)
	{
		m_dwasdu.VSQ=MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
			off+=2;
	}
	m_dwasdu.COToff=off;
	if(m_guiyuepara.COTlen==1)
	{
		m_dwasdu.COT=m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.COTlen==2)
	{
		m_dwasdu.COT=m_pReceive->byASDU[off++];
		m_sourfaaddr=m_pReceive->byASDU[off++];
		//m_dwasdu.COT=MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
	}

	if(m_dwasdu.COT > 60)  // 添加 COT 检查，配电常用COT 0~10,13,20,37,44~50  cjl 2017年8月21日09:58:00
	{
		
		return FALSE;
	}

	
	if(m_guiyuepara.conaddrlen==1)
	{
		m_dwasdu.Address=m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.conaddrlen==2)
	{
		m_dwasdu.Address=MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
			off+=2;
	}
			m_dwasdu.Infooff=off;
	if(m_guiyuepara.infoaddlen==1)
	{
		m_dwasdu.Info=m_pReceive->byASDU[off++];
	}
	if(m_guiyuepara.infoaddlen==2)
	{
		m_dwasdu.Info=MAKEWORD(m_pReceive->byASDU[off], m_pReceive->byASDU[off+1]);
			off+=2;
	}
	if(m_guiyuepara.infoaddlen==3)
	{
		m_dwasdu.Info=MAKEWORD(m_pReceive->byASDU[off+1], m_pReceive->byASDU[off+2]);
			m_dwasdu.Info<<=8;
			m_dwasdu.Info|=m_pReceive->byASDU[off];
			off+=3;
	}
	if(m_guiyuepara.infoaddlen>3)
	   m_guiyuepara.infoaddlen = 3;

	return TRUE;
}
void CGB104S::write_typeid(int  data)
{
	m_SendBuf.wWritePtr=6;
		for(BYTE i=0;i<m_guiyuepara.typeidlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}
void CGB104S::write_VSQ(int  data)
{
		for(BYTE  i=0;i<m_guiyuepara.VSQlen;i++)
			{
				m_SendBuf.pBuf[ i+6+m_guiyuepara.typeidlen ]=(data>>(i*8))&0xff;
		}

}
void CGB104S::write_COT(int  data)
{
	m_SendBuf.wWritePtr=6+m_guiyuepara.typeidlen+m_guiyuepara.VSQlen;
	//	for(BYTE i=0;i<m_guiyuepara.COTlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr+0 ]=(data)&0xff;
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr+1]=0;//m_sourfaaddr;
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr+2]=0;
				m_SendBuf.wWritePtr+=m_guiyuepara.COTlen;
		}

}
void CGB104S::write_conaddr(int  data)
{
	m_SendBuf.wWritePtr=6+m_guiyuepara.typeidlen+m_guiyuepara.VSQlen+m_guiyuepara.COTlen;
		for(BYTE i=0;i<m_guiyuepara.conaddrlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}
void CGB104S::write_infoadd(int  data)
{
		for(BYTE i=0;i<m_guiyuepara.infoaddlen;i++)
			{
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
		}

}
void CGB104S::CheckCfg()
{
//	char tt[80];
	if((m_guiyuepara.typeidlen<1)||(m_guiyuepara.typeidlen>2))
		{
		m_guiyuepara.typeidlen=1;
	m_guiyuepara.conaddrlen=2;
	m_guiyuepara.VSQlen=1;
	m_guiyuepara.COTlen=2;
	m_guiyuepara.infoaddlen=3;
	m_guiyuepara.baseyear=2000;
	m_guiyuepara.ddtype=15;
	m_guiyuepara.yctype=9;
	m_guiyuepara.yxtype=1;
	m_guiyuepara.k=12;
	m_guiyuepara.w=8;
		m_guiyuepara.t0= 30;//10;
		m_guiyuepara.t1= 15;//12;
		m_guiyuepara.t2=  10;//5;
		m_guiyuepara.t3=  20;//15;
		m_guiyuepara.t4=   8;//8;

	}
		m_PARA_K = m_guiyuepara.k;
		m_PARA_W = m_guiyuepara.w;
			m_vTimer[1].bRun = FALSE;
	m_vTimer[1].wInitVal = m_guiyuepara.t1;
	m_vTimer[2].bRun = FALSE;
	m_vTimer[2].wInitVal = m_guiyuepara.t2;
	m_vTimer[3].bRun = FALSE;
	m_vTimer[3].wInitVal = m_guiyuepara.t3;
			if((m_guiyuepara.baseyear!=2000)&&(m_guiyuepara.baseyear!=1900))
			m_guiyuepara.baseyear=2000;
	#if 0
	sprintf(tt,"gb104: len:type:%d,VSQ:%d,COT:%d,Cadd:%d,Iadd:%d,baseyear:%d",\
					m_guiyuepara.typeidlen,m_guiyuepara.VSQlen,m_guiyuepara.COTlen,\
					m_guiyuepara.conaddrlen,m_guiyuepara.infoaddlen,m_guiyuepara.baseyear);
			myprintf(m_wTaskID,LOG_ATTR_INFO, tt);
			#endif

}
void CGB104S::SetDefCfg()
{
		m_guiyuepara.typeidlen=1;
	m_guiyuepara.conaddrlen=2;
	m_guiyuepara.VSQlen=1;
	m_guiyuepara.COTlen=2;
	m_guiyuepara.infoaddlen=3;
	m_guiyuepara.baseyear=2000;
	m_guiyuepara.ddtype=15;
	m_guiyuepara.yctype=9;
	m_guiyuepara.yxtype=1;
	m_guiyuepara.k=12;
	m_guiyuepara.w=8;
		m_guiyuepara.t0=10;
		m_guiyuepara.t1=12;
		m_guiyuepara.t2=5;
		m_guiyuepara.t3=15;
		m_guiyuepara.t4=8;
		//		memcpy(m_pBaseCfg+1,&m_guiyuepara,sizeof(m_guiyuepara));

}
 BYTE CGB104S::QDS(BYTE data)
 	{
	BYTE tt=0;
	if((data&1)==0)
		tt|=0x80;
	if((data&2))
		tt|=0x10;
	if((data&4))
		tt|=0x20;
	if((data&8))
		tt|=0xc0;
	if((data&0x10))
		tt|=0x1;
		return tt;	
 }
  BYTE CGB104S::SIQ(BYTE data)
 	{
	BYTE tt=0;
	if((data&1)==0)
		tt|=0x80;
	if((data&2))
		tt|=0x10;
	if((data&4))
		tt|=0x60;
	if((data&8))
		tt|=0xc0;
	if((data&0x80))
		tt|=0x1;
		return tt;	
 }
 BYTE CGB104S::DIQ(BYTE data1,BYTE data2)
 	{
	BYTE tt=0;
	if((data1&1)==0)
		tt|=0x80;
	if((data1&2))
		tt|=0x10;
	if((data1&4))
		tt|=0x20;
	if((data1&8))
		tt|=0xc0;
	if((data1&0x80))
		tt|=0x1;
	if((data2&0x80))
		tt|=0x2;
//	if(((tt&3)==0)||((tt&3)==3))
//		tt|=0x80;
//	if((data1&0x80)==(data2&0x80))
	//	tt|=0x80;
		return tt;	
 }

void CGB104S::write_time()
{
struct VSysClock clock;	
			GetSysClock(&clock,SYSCLOCK);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(clock.wMSecond+clock.bySecond*1000)&0xff;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(clock.wMSecond+clock.bySecond*1000)>>8;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=clock.byMinute;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=clock.byHour;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=clock.byDay|(clock.byWeek<<5);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=clock.byMonth;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=clock.wYear%100;

}
void CGB104S::write_time3()
{
struct VSysClock clock;	
			GetSysClock(&clock,SYSCLOCK);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(clock.wMSecond+clock.bySecond*1000)&0xff;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(clock.wMSecond+clock.bySecond*1000)>>8;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=clock.byMinute;
}
 int CGB104S::Geteqpparaaddr(WORD addr)
 	{
 		int i;
		for( i=0;i<m_wEqpNum;i++)
			{
			if(limitpara[1+i*3]==addr)
				break;
			}
		if(i>=m_wEqpNum) return 0;
		return i;
 }
  void CGB104S::copydata(WORD len1,WORD len2,BYTE *p1,BYTE* p2)
 	{
 	WORD len=len1;
 		if((p1==0)||(p2==0)) return ;
		if(len<len2) len=len2;
		memcpy(p2,p1,len*sizeof(Vlimitpara));
 }
  void CGB104S::Setpara(void)
{
    BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff;
	WORD  VAL;	//遥控命令限定词
	WORD  YkNo; //遥控路号
	BYTE QPM;
	YkNo = MAKEWORD(pData[0], pData[1]);
	VAL =MAKEWORD( pData[m_guiyuepara.infoaddlen],pData[m_guiyuepara.infoaddlen+1]);
	QPM=pData[m_guiyuepara.infoaddlen+2]&0x3f;
	GotoEqpbyAddress(m_wRecAddress);
	int i=Geteqpparaaddr(GetEqpOwnAddr());
	if((YkNo<ADDR_YCPARA_LO)||(YkNo>ADDR_YCPARA_HI))
		return;
	if((QPM>0)&&(QPM<5))
	pData=(BYTE*)limitpara+(limitpara[1+i*3+2]+(YkNo-ADDR_YCPARA_LO)*sizeof(Vlimitpara));
	pData+=(QPM-1)*2;
	pData[1]=VAL&0xff;
	pData[0]=VAL>>8;
	return;
}
void CGB104S::Recsetpara(void)
{
	BYTE Qualifier = 0x1;
	BYTE *pdata=m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;
	if (CanSendIFrame())
	{
		SendFrameHead(FRM_I);
		Sendframe(P_ME_NA, m_dwasdu.COT+1);
		write_infoadd(m_dwasdu.Info);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = pdata[0];
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = pdata[1];
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (pdata[2]&0x3f)|0x40;
		
		write_VSQ(Qualifier);
		SendFrameTail();
	}
	Setpara();
	writepara();
	return;
}

/*****************************文件传输*********************************************/

BOOL CGB104S::DoFileData(void)   //处理文件命令
{
	BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff; //信息对象地址
	BYTE ActID;
	
	ActID = pData[m_guiyuepara.infoaddlen+1];
		
	switch(ActID)
	{
		case 1:		//召唤目录
			DoFileDir();
			break;
		case 3:		//读文件	
		case 6:	
			DoReadFile();
			break;
		case 7:     //写文件
		case 9:     
			DoWriteFile();
			break;
		default:	//error
		
			break;
	}
	return  OK;
}

BOOL CGB104S::DoFileDir(void) 
{
	VSysClock SysTime;
	WORD MSecond,len;
	WORD DirNum;
	VFileOPMsg FileOPMsg;
	VDirInfo *pDirInfo = NULL;
	BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen; 
	BYTE *p;
	char *h;
	char tempname[100];
	int i,j;
	BYTE temp[7];
	DWORD SecRcdTime;

	if(m_dwasdu.COT != 5 )
	{
		errack(45);
		return ERROR; 
	}
	if(m_dwasdu.Info != 0)
	{
		errack(47);
		return ERROR; 
	}
	memset(&FileDirInfo, 0, sizeof(VFileDirInfo));
	m_fileoffset = 0;

	FileDirInfo.dirid = (MAKEWORD(pData[4],pData[5])<<16)|MAKEWORD(pData[2],pData[3]);
	len = FileDirInfo.dirnamelen = pData[6];

	memset(FileDirInfo.dirname, 0, sizeof(FileDirInfo.dirname));

	if(FileDirInfo.dirnamelen==0)
	{
		switch(m_guiyuepara.tmp[1]) //文件
		{
			case 0:	
				strcpy(FileDirInfo.dirname,"/tffsb/COMTRADE");
				break;
			case 1:
				strcpy(FileDirInfo.dirname,"/tffsb/HISTORY/SOE");
				break;
			case 2:
				strcpy(FileDirInfo.dirname,"/tffsb/HISTORY/CO");
				break;
			case 3:
				strcpy(FileDirInfo.dirname,"/tffsb/HISTORY/EXV");
				break;
			case 4:
				strcpy(FileDirInfo.dirname,"/tffsb/HISTORY/FIXPT");
				break;
			case 5:
				strcpy(FileDirInfo.dirname,"/tffsb/HISTORY/FRZ");
				break;
			case 6:
				strcpy(FileDirInfo.dirname,"/tffsb/HISTORY/FLOWREV");
				break;
			case 7:
				strcpy(FileDirInfo.dirname,"/tffsb/HISTORY/ULOG");
				break;
			default:
				break;
			
		}	
	}
	else
	{		
		memset(FileDirInfo.dirname,0,sizeof(FileDirInfo.dirname));
		memcpy(FileDirInfo.dirname, &pData[7], FileDirInfo.dirnamelen);
		FileDirInfo.dirname[FileDirInfo.dirnamelen] = '\0';
		memset(tempname,0,sizeof(tempname));
		strcpy(tempname,"/tffsb/");
		strcat(tempname,FileDirInfo.dirname);
		memset(FileDirInfo.dirname,0,sizeof(FileDirInfo.dirname));
		strcpy(FileDirInfo.dirname,tempname);
	}
	
	FileDirInfo.flag = pData[7+len]; //召唤标志

	p = pData+8+len; //查询起始时间

	MSecond = MAKEWORD(p[0], p[1]); 
	MSecond+=20;
	SysTime.wMSecond = MSecond % 1000;
	SysTime.bySecond  = MSecond / 1000;
	SysTime.byMinute = p[2]&0x3F;
	SysTime.byHour = p[3]&0x1F;
	SysTime.byDay = p[4] & 0x1F;
	SysTime.byWeek = p[4] >> 5;
	SysTime.byMonth = p[5] & 0x0F;
	SysTime.wYear = (p[6] & 0x7F) + m_guiyuepara.baseyear;

	SystimeToSec(&SysTime,&FileDirInfo.dwStrtTime);

	p = pData+15+len; //查询终止时间

	MSecond = MAKEWORD(p[0], p[1]); 
	MSecond+=20;
	SysTime.wMSecond = MSecond % 1000;
	SysTime.bySecond  = MSecond / 1000;
	SysTime.byMinute = p[2]&0x3F;
	SysTime.byHour = p[3]&0x1F;
	SysTime.byDay = p[4] & 0x1F;
	SysTime.byWeek = p[4] >> 5;
	SysTime.byMonth = p[5] & 0x0F;
	SysTime.wYear = (p[6] & 0x7F) + m_guiyuepara.baseyear;

	SystimeToSec(&SysTime,&FileDirInfo.dwStopTime);

	
	//减少读目录的次数,非录波的只能读53个文件
	strcpy((char *)FileOPMsg.cFileName, (char *)FileDirInfo.dirname);
	FileOPMsg.dwLen = (sizeof(m_dwPubBuf) - 1024)/sizeof(VDirInfo)*sizeof(VDirInfo);
	FileOPMsg.dwOffset = 0;
#ifdef INCLUDE_RECORD
	if(strcmp(FileOPMsg.cFileName,"/tffsb/COMTRADE") == 0) // comtrade 目录
	{
		pVDirInfo = ReadRcdDir(&DirNum,0);
		pDirInfo = pVDirInfo;
		DirStatus = FileOk;
	}
	else
#endif
	{
		DirStatus = ListDir(&FileOPMsg, (VDirInfo *)((BYTE*)m_dwPubBuf + 1024));
		DirNum = FileOPMsg.dwLen / sizeof(VDirInfo);              //获取的目录数量
		pVDirInfo = (VDirInfo *)((BYTE*)m_dwPubBuf + 1024);
		pDirInfo = pVDirInfo;
	}
	
	m_fileoffset = 0;
	DirOffSetNum=0;
	m_calldir = 1;
	i = 0;
	
	if(FileDirInfo.flag == 0)
	{
		for(i = 0; i < DirNum; i++)
		{
			DirOffSet[i] = i;
		}
		DirOffSetNum = DirNum;
	}
	else if(FileDirInfo.flag == 1)
	{	
		for(j = 0; j < DirNum; j++)
		{
			h=strstr(pDirInfo->cName, "BAY");
			if(h != NULL) // 录波用文件名中的时间 比较
			{
				GetRcdNametime(pDirInfo->cName,temp);

				MSecond = MAKEWORD(temp[0], temp[1]); 
				MSecond+=20;
				SysTime.wMSecond = MSecond % 1000;
				SysTime.bySecond  = MSecond / 1000;
				SysTime.byMinute = temp[2]&0x3F;
				SysTime.byHour = temp[3]&0x1F;
				SysTime.byDay = temp[4] & 0x1F;
				SysTime.byWeek = temp[4] >> 5;
				SysTime.byMonth = temp[5] & 0x0F;
				SysTime.wYear = (temp[6] & 0x7F) + m_guiyuepara.baseyear;
				SystimeToSec(&SysTime,&SecRcdTime);
				
				if((FileDirInfo.dwStrtTime <= SecRcdTime) && (SecRcdTime <= FileDirInfo.dwStopTime))
				{
					DirOffSet[i++] = j;
					DirOffSetNum++;
				}
			}
			else
			{
				if((FileDirInfo.dwStrtTime <= pDirInfo->dwTime) && (pDirInfo->dwTime <= FileDirInfo.dwStopTime))
				{
					DirOffSet[i++] = j;
					DirOffSetNum++;
				}
			}
			pDirInfo++;
		}
	}
	else //召唤标志只有0：所有文件 1：按时间搜索
		m_calldir = 0;
	
	return TRUE;
}

BOOL CGB104S::SendFileDir(void)   //发送文件目录
{
	BYTE Style = F_FR_NA_1, Reason = COT_REQ;

	struct VSysClock SysTime;
	FileStatus filestatus;
	WORD DirNum;
	VDirInfo *pDirInfo = NULL;
	WORD ch;
	char *p=NULL;
	BYTE timebuf[7]={0};
	
	if (!CanSendIFrame())
		return FALSE;
    //召目录
	
		filestatus = DirStatus;
		DirNum = DirOffSetNum - m_fileoffset;              //获取的目录数量
		pDirInfo = pVDirInfo;	
			
		if(DirNum > 0)
		{
			if(DirNum >= 5)
				ch = 4;
			else
				ch = DirNum;
		}
		else
			ch = 0;

		SendFrameHead(FRM_I);
		Sendframe(Style, Reason);
		write_VSQ(0x80|ch);
		write_infoadd(0);

		/*附加数据包类型*/
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;  

		/*操作标识*/
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;  

		/*结果描述符 0成功1失败*/
		if(DirNum && (filestatus == FileOk))
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
		else
		{
			if(filestatus != FileOk)
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 1;
			else 
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
			
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(FileDirInfo.dirid));  
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(FileDirInfo.dirid));
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(FileDirInfo.dirid));
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(FileDirInfo.dirid));
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;  //无后续目录
			m_calldir = 0;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0; //本帧文件数量
			
			SendFrameTail();
			return FALSE;
		}

		/*目录ID*/
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(FileDirInfo.dirid));  
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(FileDirInfo.dirid));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(FileDirInfo.dirid));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(FileDirInfo.dirid));

		if((DirNum >= 5) && (filestatus == FileOk))
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 1;  //有后续目录
		else
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;  //无后续目录
			m_calldir = 0;
		}

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = ch; //本帧文件数量

		if(filestatus == FileOk)
		{
			for(int i = 0; i < ch; i++)
			{
				pDirInfo = pVDirInfo + DirOffSet[m_fileoffset+i];
				p=strstr(pDirInfo->cName, "BAY");
				if(p != NULL) // 获取录波名中的时间
				{
					GetRcdNametime(pDirInfo->cName, timebuf);

				}
				else // 历史数据获取文件创建时间
				{
					SecToSystime(pDirInfo->dwTime,&SysTime);
					timebuf[0]=(SysTime.wMSecond+SysTime.bySecond*1000)&0xff;
					timebuf[1]=(SysTime.wMSecond+SysTime.bySecond*1000)>>8;
					timebuf[2]=SysTime.byMinute;
					timebuf[3]=SysTime.byHour;
					timebuf[4]=SysTime.byDay|(SysTime.byWeek<<5);
					timebuf[5]=SysTime.byMonth;
					timebuf[6]=SysTime.wYear%100; 
				}

			
				/*文件名长度*/
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = strlen(pDirInfo->cName); 

				/*文件名*/
				memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], pDirInfo->cName, strlen(pDirInfo->cName));
				m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + strlen(pDirInfo->cName);

				/*文件属性 自定义:1目录2文件*/ 
				if(S_ISDIR(pDirInfo->dwMode))
					m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 1; 
				else if(S_ISREG(pDirInfo->dwMode))
					m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;
				else
					m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0; 

				/*文件大小*/
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(pDirInfo->dwSize));  
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(pDirInfo->dwSize));
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(pDirInfo->dwSize));
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(pDirInfo->dwSize)); 

				/*文件时间*/
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = timebuf[0]; 
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = timebuf[1];
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = timebuf[2]; 
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = timebuf[3];
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = timebuf[4]; 
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = timebuf[5];
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = timebuf[6]; 
				
			}
		}
		
	m_fileoffset += ch;
		
	SendFrameTail();
		
	return TRUE;
		
}

void CGB104S::GetRcdNametime(char *str, BYTE *buf) // 通过录波文件名获取时间
{
	int i=0;
	char *p = str;
	buf[6] = ((str[11]-'0')*1000+(str[12]-'0')*100+(str[13]-'0')*10+(str[14]-'0'))%100;  //year
	buf[5] = (str[15]-'0')*10+(str[16]-'0');    //month
	buf[4] = ((str[17]-'0')*10+(str[18]-'0'))& 0x1F;   //day
	
	buf[3] = ((str[20]-'0')*10+(str[21]-'0')) & 0x1F;   //hour
	buf[2] = ((str[22]-'0')*10+(str[23]-'0'));    //min

	p=p+27;
	while(*p++ != '.')
		i++;
	if(i==3)
	{
		buf[1] = HIBYTE(((str[24]-'0')*10+(str[25]-'0'))*1000+((str[27]-'0')*100+ (str[28]-'0')*10+(str[29]-'0')));        
		buf[0] = LOBYTE(((str[24]-'0')*10+(str[25]-'0'))*1000+((str[27]-'0')*100+ (str[28]-'0')*10+(str[29]-'0')));  

	}
	else if(i==2)
	{
		buf[1] = HIBYTE(((str[24]-'0')*10+(str[25]-'0'))*1000+((str[27]-'0')*10+(str[28]-'0')));        
		buf[0] = LOBYTE(((str[24]-'0')*10+(str[25]-'0'))*1000+((str[27]-'0')*10+(str[28]-'0')));  
	}
	else
	{
		buf[1] = HIBYTE(((str[24]-'0')*10+(str[25]-'0'))*1000+(str[27]-'0'));        
		buf[0] = LOBYTE(((str[24]-'0')*10+(str[25]-'0'))*1000+(str[27]-'0'));  
	}
		
}
BOOL CGB104S::DoReadFile(void)
{
	BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen; 
	BYTE Style = F_FR_NA_1, Reason = COT_ACTCON;
	//BYTE dwCode = 3;
	VFileOPMsg FileOPMsg;
	FileStatus filestatus;
	char tempname[100];
	char* ptr = NULL;

	if((pData[1] == 3) && (m_dwasdu.COT == 6 ))  //  读文件激活
	{
		memset(&ReadFileInfo, 0, sizeof(VFileInfo));
		ReadFileInfo.datano = 0;
		ReadFileInfo.filenamelen = pData[2];
		memcpy(ReadFileInfo.filename, &pData[3], ReadFileInfo.filenamelen);
		ReadFileInfo.filename[ReadFileInfo.filenamelen] = '\0';

		memset(tempname,0,sizeof(tempname));
		if(ReadFileInfo.filename[0] == 'B' )  // 录波文件
		{
			strcpy(tempname, "/tffsb/COMTRADE/");
			strcat(tempname, ReadFileInfo.filename);

		}
		else if((ReadFileInfo.filename[0] == 's') && (ReadFileInfo.filename[1] == 'o')) // soe
		{
			strcpy(tempname, "/tffsb/HISTORY/SOE/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'c') && (ReadFileInfo.filename[1] == 'o')) // co
		{
			strcpy(tempname, "/tffsb/HISTORY/CO/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'e') && (ReadFileInfo.filename[1] == 'x')) // 极值
		{
			strcpy(tempname, "/tffsb/HISTORY/EXV/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'f') && (ReadFileInfo.filename[1] == 'i')) //定点
		{
			strcpy(tempname, "/tffsb/HISTORY/FIXPT/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'f') && (ReadFileInfo.filename[1] == 'r')) // 日冻结
		{
			strcpy(tempname, "/tffsb/HISTORY/FRZ/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'f') && (ReadFileInfo.filename[1] == 'l')) // 功率反向
		{
			strcpy(tempname, "/tffsb/HISTORY/FLOWREV/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'u') && (ReadFileInfo.filename[1] == 'l')) //日志
		{
			strcpy(tempname, "/tffsb/HISTORY/ULOG/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'v') && (ReadFileInfo.filename[1] == 'o'))
		{
			if((ReadFileInfo.filenamelen == strlen("voltYYYYMMDD.msg"))||(ReadFileInfo.filenamelen == strlen("volttYYYYMMDD.msg")))
			{
				strcpy(tempname, "/tffsb/HISTORY/VOLT_DAY/");
				strcat(tempname, ReadFileInfo.filename);			
			}
			else if((ReadFileInfo.filenamelen == strlen("voltYYYYMM.msg"))||(ReadFileInfo.filenamelen == strlen("volttYYYYMM.msg")))
			{
				strcpy(tempname, "/tffsb/HISTORY/VOLT_MON/");
				strcat(tempname, ReadFileInfo.filename);			
			}
		}
		else if((ReadFileInfo.filename[0] == 'h') && (ReadFileInfo.filename[1] == 'l'))
		{
			strcpy(tempname, "/tffsb/HISTORY/OL/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'o') && (ReadFileInfo.filename[1] == 'l'))
		{
			strcpy(tempname, "/tffsb/HISTORY/OL/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else
		{
			errack(45);
			return FALSE;
		}
			
		memcpy(ReadFileInfo.filename,tempname,strlen(tempname));
		memset(FileOPMsg.cFileName,0,sizeof(FileOPMsg.cFileName));
		memcpy(FileOPMsg.cFileName,tempname,strlen(tempname));			
		
		FileOPMsg.dwOffset = 0;
		FileOPMsg.dwLen = 100;
		FileOPMsg.dwSize = 0;

		filestatus = ReadFile(&FileOPMsg, (BYTE *)&m_dwPubBuf);
		
		SendFrameHead(FRM_I);
		Sendframe(Style, Reason);
		write_infoadd(0);

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;  //附加数据包类型
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 4;  //操作标识

		if (filestatus != FileOk)
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 1;  //结果描述符
			m_byReadFileFlag = 0;
		}
		else
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
			m_byReadFileFlag = 1;
		}
		
		/*文件名长度*/
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = ReadFileInfo.filenamelen;  

		/*文件名*/
		//memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], ReadFileInfo.filename,strlen(ReadFileInfo.filename));
		ptr = strrchr(ReadFileInfo.filename, '/');
		if(ptr != NULL)
			memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (ptr+1),strlen(ReadFileInfo.filename));
		else
			memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], ReadFileInfo.filename,strlen(ReadFileInfo.filename));
	
		m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + ReadFileInfo.filenamelen;

		/*文件标识*/
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x41;   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;

		/*文件大小*/
		if(filestatus != FileOk)
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;  
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;

		}
		else
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(FileOPMsg.dwSize));  
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(FileOPMsg.dwSize));
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(FileOPMsg.dwSize));
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(FileOPMsg.dwSize));
		}

		write_VSQ(1);
		SendFrameTail();
			
		return TRUE;
			
	}
	else if((pData[1] == 6)&& (m_dwasdu.COT == 5 ))  //  读文件数据传输确认
	{
		/*if(pData[9])  //有后续
			m_byReadFileFlag = 1;
		else
			m_byReadFileFlag = 0;*/
	}
	else
	{
		errack(45);
		return FALSE; 
	}
	return TRUE;
}

BOOL CGB104S::SendReadFileData(void)
{
	BYTE Style = F_FR_NA_1, Reason = COT_REQ;
	FileStatus filestatus;
	VFileOPMsg FileOPMsg;	
	BYTE CrcSum;
	static BYTE errcnt = 0;
	static DWORD errtime = 0;

	if (!CanSendIFrame())
	    return FALSE;
	
	memset(FileOPMsg.cFileName,0,sizeof(FileOPMsg.cFileName));
	memcpy(FileOPMsg.cFileName,ReadFileInfo.filename,strlen(ReadFileInfo.filename));
	
	FileOPMsg.dwSize = 0;
	FileOPMsg.dwOffset = ReadFileInfo.datano;
	FileOPMsg.dwLen = MAX104LEN;

	SendFrameHead(FRM_I);
	Sendframe(Style, Reason);
	write_infoadd(0);

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;  //附加数据包类型
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 5;  //操作标识

	/*文件标识*/
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x41;   
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;

	/*数据段号*/
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(ReadFileInfo.datano));  
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(ReadFileInfo.datano));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(ReadFileInfo.datano));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(ReadFileInfo.datano));

	filestatus = ReadFile(&FileOPMsg, &m_SendBuf.pBuf[m_SendBuf.wWritePtr+1]);

	if (filestatus == FILEOK )
	{
		errcnt = 0;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 1; //有后续
	
		CrcSum = ChkSum(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], FileOPMsg.dwLen);  
		m_SendBuf.wWritePtr += FileOPMsg.dwLen;
		ReadFileInfo.datano += FileOPMsg.dwLen;

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = CrcSum; //校验和
		m_byReadFileFlag = 1;
		
	}
	else if(filestatus == FILEEOF)
	{
		errcnt = 0;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0; //无后续

		CrcSum = ChkSum(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], FileOPMsg.dwLen);  
		m_SendBuf.wWritePtr += FileOPMsg.dwLen;
		ReadFileInfo.datano += FileOPMsg.dwLen;

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = CrcSum; //校验和
		m_byReadFileFlag = 0;
	}
	else
	{
		if(errcnt == 0)
		{
			errcnt = 1;
			errtime = Get100usCnt();
		}
		else
		{
			if((Get100usCnt() - errtime) > 100000)
				m_byReadFileFlag = 0;
		}
		StopTimer(1);	//SendFrameHead会计时t1，空的不要再计时t1了，不太合理但此不能计时t1
		return FALSE;
	}

	write_VSQ(1);
	SendFrameTail();

	return TRUE;

}

BOOL CGB104S::DoWriteFile(void)
{
	BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;
	BYTE *p,crc1,crc2;
	DWORD length;
	BYTE Style = F_FR_NA_1;
	struct VFileOPMsg FileOPMsg;
	char tempname[100]="/tffsa/other/";
	static DWORD wrptr=0;
	static DWORD off=0;
	VFileMsgs fmsg;
	
	if((pData[1] == 7) && (m_dwasdu.COT == 6 ))    //写文件激活
	{
		off=0;
		wrptr=0;
		filetemp = (BYTE*)g_pParafile;
		memset(g_pParafile,0,MAXFILELEN);
		
		memset(&WriteFileInfo, 0, sizeof(VFileInfo));
		WriteFileInfo.filenamelen = pData[2];
		memcpy(WriteFileInfo.filename, &pData[3], WriteFileInfo.filenamelen);
		WriteFileInfo.filename[WriteFileInfo.filenamelen] = '\0';

		p = &pData[WriteFileInfo.filenamelen+3];
		
		WriteFileInfo.fileid= (MAKEWORD(p[2],p[3])<<16)|MAKEWORD(p[0],p[1]);
		WriteFileInfo.filesize = (MAKEWORD(p[6],p[7])<<16)|MAKEWORD(p[4],p[5]);

		SendFrameHead(FRM_I);
		Sendframe(Style, 7);
		write_infoadd(0);
		
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;  //附加数据包类型
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 8;  //操作标识

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;   //结果描述字

		/*文件名长度*/
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = WriteFileInfo.filenamelen;   

		/*文件名*/
		memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], WriteFileInfo.filename, WriteFileInfo.filenamelen);
		m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + WriteFileInfo.filenamelen;

		/*文件ID*/
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(WriteFileInfo.fileid)); 
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(WriteFileInfo.fileid));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(WriteFileInfo.fileid));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(WriteFileInfo.fileid));
		
		/*文件大小*/
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(WriteFileInfo.filesize)); 
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(WriteFileInfo.filesize));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(WriteFileInfo.filesize));
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(WriteFileInfo.filesize));

		write_VSQ(0);
		SendFrameTail();

		strcat(tempname,WriteFileInfo.filename);
		memset(WriteFileInfo.filename, 0, strlen(WriteFileInfo.filename));
		memcpy(WriteFileInfo.filename,tempname,strlen(tempname));

		return TRUE;
		
	}
	else if((pData[1] == 9) &&(m_dwasdu.COT == 5))    //写文件数据传输
	{
		p = &pData[6];
		length = m_pReceive->byAPDULen-m_guiyuepara.typeidlen-m_guiyuepara.VSQlen-m_guiyuepara.COTlen-m_guiyuepara.conaddrlen-m_guiyuepara.infoaddlen-16;
		crc1 = *(p+length+5);
		crc2 = (BYTE)ChkSum((BYTE *)(p+5), length);
		
		if(crc1 != crc2)
			WriteFileInfo.flag |= 0x01 << 2;
		else
			WriteFileInfo.flag |= 0x01 << 0;
		
		memcpy(filetemp+wrptr,(BYTE*)(p+5),length);
		wrptr = wrptr + length;

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
		{
			MD5_Update((BYTE*)(p+5),length);
		}
#endif

		if(WriteFileInfo.filesize >= (MAXFILELEN-1024))
		{
			if((wrptr >= (MAXFILELEN-2048)) || ((wrptr>0) && (*(p+4) == 0)))
			{	

				off=off+wrptr;
				strcpy(FileOPMsg.cFileName,WriteFileInfo.filename);
				FileOPMsg.dwOffset=off-wrptr;
				FileOPMsg.dwLen=wrptr;
				FileOPMsg.dwSize=WriteFileInfo.filesize;

				fmsg.type = MSG_FILE;
				fmsg.num = 1;
				fmsg.filemsg[0].type = FILE_TYPE_PEOGRAM;
				memcpy((BYTE*)g_pParafile + MAXFILELEN - 1024,&FileOPMsg,sizeof(struct VFileOPMsg));
				memcpy((BYTE*)g_pParafileTmp,(BYTE*)g_pParafile,MAXFILELEN);	
				msgSend(B2F_ID, &fmsg, sizeof(VFileMsgs), 1);
				wrptr = 0;
			}
			
		}
		else
		{
			if(*(p+4) == 0) // 无后续开始写文件
			{
				strcpy(FileOPMsg.cFileName,WriteFileInfo.filename);
				FileOPMsg.dwOffset=0;
				FileOPMsg.dwLen=wrptr;
				FileOPMsg.dwSize=WriteFileInfo.filesize;

				fmsg.type = MSG_FILE;
				fmsg.num = 1;
				fmsg.filemsg[0].type = FILE_TYPE_PEOGRAM;
				memcpy((BYTE*)g_pParafile + MAXFILELEN - 1024,&FileOPMsg,sizeof(struct VFileOPMsg));
				memcpy((BYTE*)g_pParafileTmp,(BYTE*)g_pParafile,MAXFILELEN);	
				msgSend(B2F_ID, &fmsg, sizeof(VFileMsgs), 1);
				wrptr = 0;
				
			}

		}

		if(*(p+4) == 0)  //无后续  发送写文件数据传输确认
		{
			SendFrameHead(FRM_I);
			Sendframe(Style, 5);
			write_infoadd(0);
			
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;  //附加数据包类型
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 10;  //操作标识

			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(WriteFileInfo.fileid)); //文件ID
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(WriteFileInfo.fileid));
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(WriteFileInfo.fileid));
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(WriteFileInfo.fileid));

			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(WriteFileInfo.datano)); //数据段号
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(WriteFileInfo.datano));
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(WriteFileInfo.datano));
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(WriteFileInfo.datano));

		    /*结果描述符*/
			if(WriteFileInfo.flag & (1<<0))
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;  //成功
			else if((WriteFileInfo.flag & (1<<2)) == 0)
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;  //校验和错误
			else
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 1;  //未知错误
			
			WriteFileInfo.flag = 0;

			write_VSQ(0);
			SendFrameTail();

		}
		return TRUE;
		
	}
	else
	{
		errack(45);
		return FALSE; 
	}

}


BYTE CGB104S::SecToSystime(DWORD Second, struct VSysClock *SysTime)
{
	struct VCalClock pBuf;
	pBuf.dwMinute = Second/60;
	pBuf.wMSecond = 0;
	
	CalClockTo(&pBuf, SysTime);
	return OK;
}

BYTE CGB104S::SystimeToSec(struct VSysClock *SysTime, DWORD* Second)
{
	struct VCalClock pBuf;
	
	ToCalClock(SysTime, &pBuf);
	*Second = pBuf.dwMinute*60+pBuf.wMSecond/1000;
	return OK;
}

BYTE CGB104S::ChkSum(BYTE *buf, WORD len)
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


/*************************文件传输END*****************************************/

/***************************参数读写***************************/
BOOL CGB104S::DoSwitchValNo(void)   //切换定值区号
{
	BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen; //信息对象地址
	WORD SN;
	BYTE Style = C_SR_NA_1;
	
	if(m_dwasdu.COT != 6 )
	{
		errack(45);
		return FALSE;
	}

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if(3 == m_guiyuepara.jiami)
	{
	  	if(m_jiamiType != 1)
	  	{
	  		SecuritySend1FFrame(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
	  		m_SendBuf.wReadPtr = 0;
			SendAllFrame();
	  		return FALSE;
	  	}
	}
	if(4 == m_guiyuepara.jiami)
	{
	  	if(m_jiamiType != 0)
	  	{
	  		SecuritySend1FFrame_HN(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
	  		m_SendBuf.wReadPtr = 0;
			SendAllFrame();
	  		return FALSE;
	  	}
	}
#endif

 	SN = MAKEWORD(pData[0],pData[1]);
	SendFrameHead(FRM_I);
	Sendframe(Style, m_dwasdu.COT+1);
	write_infoadd(0);

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SN); //定值区号
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SN);

	write_VSQ(1);
	SendFrameTail();

	return TRUE;

}

BOOL CGB104S::DoReadValNo(void)   //读定值区号
{
	//BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;
	WORD SN1,SN2;
	BYTE Style = C_RR_NA_1;

	if(m_dwasdu.COT != 6)
	{
		errack(45);
		return FALSE;
	}
	if(m_dwasdu.Info != 0)
	{
		errack(47);
		return FALSE;
	}
	
	SN1 = 0; SN2 = 0;

	SendFrameHead(FRM_I);
	Sendframe(Style, m_dwasdu.COT+1);
	write_infoadd(0); 

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SN1); //定值区号SN1
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SN1);

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SN2); //定值区号SN2
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SN2);

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //定值区号SN3
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
	
	write_VSQ(1);
	SendFrameTail();

	return TRUE;	

}

BOOL CGB104S::DoReadParaVal(void)  //读参数定值
{
	//BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;
	
	if(m_dwasdu.COT != 6)
	{
		errack(45);
		return FALSE;
	}
	
	if(m_dwasdu.VSQ == 0)  //读全部参数
	{
		m_byParaValFlag = 1;
		m_paraoffset = SELFPARA_ADDR;
	
	}
	else   //读部分参数
	{
		m_byParaValFlag = 2;
		m_paraoffset = 0;
		m_paraaddrnum = m_dwasdu.VSQ&0x7F;
		memcpy(m_tdata,&m_pReceive->byStartCode,m_pReceive->byAPDULen+2);

	}

	return TRUE;

}

BOOL CGB104S::SendReadParaVal(void) //发送参数定值
{
	BYTE Style = C_RS_NA_1, Reason = COT_ACTCON;
	BYTE pi=0,pipos,j;
	DWORD SN=0,i,a;
	int state;
	DWORD infoaddr;
	BYTE *pdata;
	WORD pLen = 0;
	VParaInfo *pParaInfo;

	pParaInfo = (struct VParaInfo *)m_dwPubBuf;
	if (!CanSendIFrame())
	    return FALSE;
	SendFrameHead(FRM_I);
	Sendframe(Style, Reason);

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SN); //定值区号SN
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SN);
	pipos = m_SendBuf.wWritePtr;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pi;    //参数特征标识
	
	if(m_byParaValFlag == 1)  //读全部参数
	{	
		j = 0;
		for(i = m_paraoffset; i <= PARA_END; i++)
		{
			state = ReadPara(i, (char *)m_dwPubBuf);
			if (state == ERROR)
				continue;
			else if(state == 0x80) //待定
			{
				break;
			}
			else if(state == OK)
			{
				pLen = m_SendBuf.wWritePtr+4+pParaInfo->len;//4=地址+长度+类型
				if(pLen>(MAX_FRAME_LEN-2))
					break;
				j++;
				write_infoadd(i); 
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pParaInfo->type; // tag类型
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pParaInfo->len; // 数据长度
				memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr, pParaInfo->valuech, pParaInfo->len);
				m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + pParaInfo->len;
			}
			else 
				continue;
		}
		
		if(i > PARA_END)
		{
			m_SendBuf.pBuf[ pipos ] = 0; // 无后续
			m_byParaValFlag = 0;
		}
		else
			m_SendBuf.pBuf[ pipos ] = 1; // 有后续
		m_paraoffset = i;
		if((j == 0) && (state == 0x80))
		{
			return FALSE;
		}
		else if(!j)
		{
			errack(47);
			return FALSE;
		}
		write_VSQ(j);
		SendFrameTail();
		
		return TRUE;

	}

	if(m_byParaValFlag == 2)	  //读多个参数
	{
		j = 0;
		pdata = &m_tdata[m_dwasdu.Infooff+8];
		for(i = m_paraoffset; i < m_paraaddrnum; i++)
		{
			
			if(m_guiyuepara.infoaddlen == 1)
			{
				a=i;
				infoaddr = MAKEWORD(pdata[a], 0);
			}
			else if(m_guiyuepara.infoaddlen == 2)
			{
				a=i*2;
				infoaddr = MAKEWORD(pdata[a],pdata[a+1]);
			}
			else
			{
				a=i*3;
				infoaddr = MAKEDWORD( MAKEWORD(pdata[a],pdata[a+1]),pdata[a+2]);

			}
			state = ReadPara(infoaddr, (char *)m_dwPubBuf);
			if(state == 0x80) //待定
			{
				break;
			}
			else if(state == OK)
			{
				pLen = m_SendBuf.wWritePtr+4+pParaInfo->len;//4=地址+长度+类型
				if(pLen>(MAX_FRAME_LEN-2))
					break;
				j++;
				write_infoadd(infoaddr); 
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pParaInfo->type; // tag类型
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pParaInfo->len; // 数据长度
				memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr, pParaInfo->valuech, pParaInfo->len);
				m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + pParaInfo->len;
			}
			else 
				continue;
		}

		if(i >= m_paraaddrnum)
		{
			m_SendBuf.pBuf[ pipos ] = 0; // 无后续
			m_byParaValFlag = 0;
		}
		else
			m_SendBuf.pBuf[ pipos ] = 1; // 有后续
			
		m_paraoffset = i;

		if((j == 0) && (state == 0x80))
		{
			return FALSE;
		}
		else if(!j)
		{
			errack(47);
			return FALSE;
		}
		else
		{
			write_VSQ(j);
			SendFrameTail();
			return TRUE;
		}
	
	}
	
	return FALSE;

}

BOOL CGB104S::DoWriteParaVal(void)    //写参数
{
	BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff;
	BYTE *p,pi;
	BOOL ch;
	WORD i;
	WORD num=0;
	DWORD infoaddr;
	BOOL luboyb = 0;
	
	pi = pData[2];
	if((m_dwasdu.COT != 6)&&(m_dwasdu.COT != 8))
	{
		errack(45);
		return FALSE;
	}
	/*预置*/
	if((m_dwasdu.COT == 6) && ((pi & 0x80) == 0x80) && ((pi & 0x40) == 0)) 
	{
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if(3 == m_guiyuepara.jiami)
		{
		  	if(m_jiamiType != 1)
		  	{
		  		SecuritySend1FFrame(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
		  		m_SendBuf.wReadPtr = 0;
				SendAllFrame();
		  		return FALSE;
		  	}
		}
		if(4 == m_guiyuepara.jiami)
		{
		  	if(m_jiamiType != 1)
		  	{
		  		SecuritySend1FFrame_HN(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
		  		m_SendBuf.wReadPtr = 0;
				SendAllFrame();
		  		return FALSE;
		  	}
		}
#endif		
		p = pData+3;
		num = m_dwasdu.VSQ & 0x7f;
		for(i = 0; i < num; i++)
		{
			if(m_guiyuepara.infoaddlen == 1)
			{
				infoaddr = MAKEWORD(*p, 0);
			}
			else if(m_guiyuepara.infoaddlen == 2)
			{
				infoaddr = MAKEWORD(*p, *(p+1));
			}
			else
			{
				infoaddr = MAKEDWORD(MAKEWORD(*p,*(p+1)),*(p+2));
			}
			WriteParaAddr[m_paranum+i] = infoaddr;
			WriteParaInfo[m_paranum+i].type = *(p+m_guiyuepara.infoaddlen);
			WriteParaInfo[m_paranum+i].len = *(p+m_guiyuepara.infoaddlen+1);
			memcpy(WriteParaInfo[m_paranum+i].valuech, p+m_guiyuepara.infoaddlen+2, WriteParaInfo[m_paranum+i].len);
			
			ch = WriteParaYZ(infoaddr,(char *)&WriteParaInfo[m_paranum+i]);
			if(ch != 0)
				break;
	
			p = p+m_guiyuepara.infoaddlen+2+WriteParaInfo[m_paranum+i].len;
		}
		
		if(ch == 0)
		{
			m_paranum +=num;
			SendFrameHead(FRM_I);
			Sendframe(m_dwasdu.TypeID, m_dwasdu.COT+1);
			
			memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
			m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
			write_VSQ(m_dwasdu.VSQ);

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
			if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
				SendFrameTail0x2();
			else
#endif
				SendFrameTail();
		    
			paracmdstatue = 1;

			return TRUE;
			
		}
		else
		{	
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
			if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
				errack0x2(47);
			else
#endif
				errack(47);
			return FALSE;
		}

	} 
	/*固化*/
	else if((m_dwasdu.COT == 6) && ((pi & 0x80) == 0) && ((pi & 0x40) == 0))
	{

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if(3 == m_guiyuepara.jiami)
		{
		  	if(m_jiamiType != 3)
		  	{
		  		SecuritySend1FFrame(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
		  		m_SendBuf.wReadPtr = 0;
				SendAllFrame();
		  		return FALSE;
		  	}
		}
		if(4 == m_guiyuepara.jiami)
		{
		  	if(m_jiamiType != 3)
		  	{
		  		SecuritySend1FFrame_HN(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
		  		m_SendBuf.wReadPtr = 0;
				SendAllFrame();
		  		return FALSE;
		  	}
		}
#endif
		
		if(paracmdstatue == 1)
		{
			for(i = 0; i < m_paranum; i++)
			{
				ch = WriteParaGH(WriteParaAddr[i],(char *)&WriteParaInfo[i]);
				if(ch != 0)
					break;
	
			}
			m_WriteNum = i;
			if(ch == 0) //成功
			{
				SendFrameHead(FRM_I);
				Sendframe(m_dwasdu.TypeID, m_dwasdu.COT+1);
				
				memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
				m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
				write_VSQ(m_dwasdu.VSQ);

				SendFrameTail();
			
				paracmdstatue = 0;
				GB104WriteRunParaFile();
				for(i = 0; i< m_WriteNum;i++)
				{

					if((WriteParaAddr[i] >= RUNPARA_LINEADDR1) && (WriteParaAddr[i] < PRPARA_ADDR ))
					{

						if(((WriteParaAddr[i] - RUNPARA_LINEADDR)/RUNPARA_LINENUM < g_Sys.MyCfg.wFDNum) && ((WriteParaAddr[i] - RUNPARA_LINEADDR)%RUNPARA_LINENUM >= PRP_LBYB_GL)
						&& ((WriteParaAddr[i] - RUNPARA_LINEADDR)%RUNPARA_LINENUM <=  PRP_LBYB_U0))
							luboyb = 1;
					}

					if(((WriteParaAddr[i] >= PRPARA_ADDR) && (WriteParaAddr[i] < PRPARA_LINE_ADDREND)) || (luboyb))
					{
#ifdef INCLUDE_PR
						WritePrParaFile();
#endif
						break;
					}
				}
				m_WriteNum = 0;
				m_paranum = 0;
				return TRUE;
				
			}
			else
			{
				errack(47);
				return FALSE;
			}
		}
		else
		{
			errack(47);
			return FALSE;
		}

	} 
	/*取消预置*/
	else if((m_dwasdu.COT == 8) && ((pi & 0x80) == 0) && ((pi & 0x40) == 0x40))
	{
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if(3 == m_guiyuepara.jiami)
		{
		  	if(m_jiamiType != 3)
		  	{
		  		SecuritySend1FFrame(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
		  		m_SendBuf.wReadPtr = 0;
				SendAllFrame();
		  		return FALSE;
		  	}
		}
		if(4 == m_guiyuepara.jiami)
		{
		  	if(m_jiamiType != 3)
		  	{
		  		SecuritySend1FFrame_HN(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
		  		m_SendBuf.wReadPtr = 0;
				SendAllFrame();
		  		return FALSE;
		  	}
		}
#endif
		
		if(paracmdstatue == 1)
		{
			SendFrameHead(FRM_I);
			Sendframe(m_dwasdu.TypeID, m_dwasdu.COT+1);
			
			memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
			m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
			write_VSQ(m_dwasdu.VSQ);

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
			if(3 == m_guiyuepara.jiami)
			{
#ifdef INCLUDE_SEC_CHIP
				if(m_wCommID == COMM_IPSEC_NO)
					SendFrameTail();//撤销确认，南网为00
				else
#endif
					SendFrameTail0x2();
			}
			else if(4 == m_guiyuepara.jiami)
				SendFrameTail();
			else
#endif
				SendFrameTail();
		
			paracmdstatue = 0;
			m_paranum = 0;

			return TRUE;
			
		}
		else
		{
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
			if(3 == m_guiyuepara.jiami)
				errack0x2(49);
			else if(4 == m_guiyuepara.jiami)
				errack(49);
			else
#endif		
				errack(49);
			return FALSE;
		}
	}
	else
	{
		errack(49);
		return FALSE;
	}
	
}

/************************电能量信息*******************************/

BOOL CGB104S::DoCallEnergy(void)
{
	BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen; 
	int i;

	if(m_dwasdu.COT != 6)
	{
		errack(45);
		return FALSE;
	}
	if(m_dwasdu.VSQ != 1)
	{		
		errack(47);
		return FALSE;
	}
	if(m_dwasdu.Info != 0)
	{
		errack(47);
		return FALSE;
	}
	if(((pData[0]&0xf) != 5))
	{
		errack(47);
		return FALSE;
	}

	SendFrameHead(FRM_I);
	Sendframe(m_dwasdu.TypeID, m_dwasdu.COT+1);
	
	memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
	m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
	write_VSQ(m_dwasdu.VSQ);

	SendFrameTail();

	for(i = 0; i < g_ddhxnum; i++)
	{
		m_dwfiffrzflag |= 0x01<<i;
		m_dwrealdataflag |= 0x01<<i;
		m_dwdayfrzflag |= 0x01<<i;
	}
         m_dwrealdataflag  |= 0x10000000;
	m_dwdayfrzflag |= 0x10000000;
	
	return TRUE;

}

BOOL CGB104S::SendAllEnergy(void)
{
	int i;
#if(DEV_SP  == DEV_SP_TTU)
	for(i=0; (i<FRM_DDNUM); i++)    //  总召  实时数据
	{
		if( m_dwrealdataflag & (1<<i) )  
		{
			m_dwrealdataflag &= ~(1<<i);
			MakeRealddGroupFrame(i, 37);
			return TRUE;
	   }
	}


#else
	for(i=0; (i<FRM_DDNUM)&&(i<g_ddhxnum); i++)    //  总召  实时数据
	{
		if( m_dwrealdataflag & (1<<i) )  
		{
			m_dwrealdataflag &= ~(1<<i);
			MakeRealddGroupFrame(i, 37);
		    return TRUE;
	   }
	}

//	for(i=0; (i<FRM_DDNUM)&&(i<g_ddhxnum); i++)    //  总召  15分钟冻结数据
//	{
//		if( m_dwfiffrzflag & (1<<i) )  
//		{
//			m_dwfiffrzflag &= ~(1<<i);
//			MakeFifFrzGroupFrame(i, 37);
//			return TRUE;
//		}
//	}

//	for(i=0; (i<FRM_DDNUM)&&(i<g_ddhxnum); i++)    //  总召  日冻结数据
//	{
//		if( m_dwdayfrzflag & (1<<i) )  
//		{
//			m_dwdayfrzflag &= ~(1<<i);
//			MakeDayFrzGroupFrame(i, 37);
//			return TRUE;
//		}
//	}
#endif
	if(m_dwrealdataflag & (0x10000000))  //总召结束
	{
		m_dwfiffrzflag=0;
		m_dwrealdataflag=0;
		m_dwdayfrzflag=0;
		SendAllEnergyAck();
		return TRUE;
	}

	return FALSE;
}
BOOL CGB104S::MakeRealddGroupFrame(WORD GroupNo,BYTE Reason)
{
	BYTE Style=206;
	WORD wSendNum;
	DWORD DDSendNum,DDNo;
	struct VDDFT Ddbuf;
	DWORD j = 0;
#if(DEV_SP  != DEV_SP_TTU)
	if(GroupNo >= g_ddhxnum)
		return FALSE;
#endif
	DDNo = GroupNo*32;
    if (DDNo >= m_pEqpInfo[m_wEqpNo].wDDNum)
		return FALSE;

	SendFrameHead(FRM_I);
	Sendframe(Style, Reason);
	
	for (DDSendNum = 0;(DDSendNum < FRM_DDNUM) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum);DDSendNum++,DDNo++)
	{
		ReadDDSendNo(m_wEqpID,DDNo,&wSendNum);
		write_infoadd(wSendNum+DD_START_ADDR);
		memcpy(&Ddbuf, 0, sizeof(struct VDDFT));
		ReadRangeDDFT(m_wEqpID, DDNo, 1, sizeof(struct VDDFT), &Ddbuf);	
		//LongToFloat(0,Ddbuf.lValue,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
		//m_SendBuf.wWritePtr+=4;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(LOWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(LOWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(HIWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(HIWORD(Ddbuf.lValue));
		 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //QDS
		j++;
	}
	if(!j)
		return TRUE;
	write_VSQ(DDSendNum);
	SendFrameTail();
	
	return TRUE;
}
BOOL CGB104S::MakeFifFrzGroupFrame(WORD GroupNo,BYTE Reason)
{
	BYTE Style=207;
	DWORD DDSendNum,DDNo;
	struct VDDFT Ddbuf;
	struct VSysClock SysTime;
	DWORD j = 0;

	if(GroupNo >= g_ddhxnum)
		return FALSE;

	DDNo = GroupNo*32+8;
  if(DDNo >= m_pEqpInfo[m_wEqpNo].wDDNum)
		return FALSE;
	
	SendFrameHead(FRM_I);
	Sendframe(Style, Reason);
	
	for (DDSendNum = 0;(DDSendNum < FRM_DDNUM) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum);DDSendNum++,DDNo++)
	{
		write_infoadd(DDNo+DD_START_ADDR);
		memcpy(&Ddbuf, 0, sizeof(struct VDDFT));
		ReadRangeDDFT(m_wEqpID, DDNo, 1, sizeof(struct VDDFT), &Ddbuf);	
		//LongToFloat(0,Ddbuf.lValue,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
		//m_SendBuf.wWritePtr+=4;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(LOWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(LOWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(HIWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(HIWORD(Ddbuf.lValue));
		 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //QDS

		/* time */
		if((Ddbuf.Time.dwMinute == 0)&&(Ddbuf.Time.wMSecond==0))
		{
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			
		}
		else
		{
			CalClockTo(&Ddbuf.Time, &SysTime);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SysTime.bySecond*1000 + SysTime.wMSecond);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SysTime.bySecond*1000 + SysTime.wMSecond);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byHour; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (SysTime.byDay & 0x1F) | (SysTime.byWeek << 5); 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMonth; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.wYear -m_guiyuepara.baseyear; 
		}
		j++;
	}
	if(!j)
		return TRUE;

	write_VSQ(DDSendNum);
	SendFrameTail();
	
	return TRUE;
}

BOOL CGB104S::MakeDayFrzGroupFrame(WORD GroupNo,BYTE Reason)
{
	BYTE Style=207;
	DWORD DDSendNum,DDNo;
	struct VDDFT Ddbuf;
	struct VSysClock SysTime;
	DWORD j = 0;

	if(GroupNo >= g_ddhxnum)
		return FALSE;

	DDNo = GroupNo*32+16;
  if(DDNo >= m_pEqpInfo[m_wEqpNo].wDDNum)
		return FALSE;
	SendFrameHead(FRM_I);
	Sendframe(Style, Reason);
	
	for (DDSendNum = 0;(DDSendNum < FRM_DDNUM) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum);DDSendNum++,DDNo++)
	{
		write_infoadd(DDNo+DD_START_ADDR);
		memcpy(&Ddbuf, 0, sizeof(struct VDDFT));
		ReadRangeDDFT(m_wEqpID, DDNo, 1, sizeof(struct VDDFT), &Ddbuf);	
		//LongToFloat(0,Ddbuf.lValue,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
		//m_SendBuf.wWritePtr+=4;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(LOWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(LOWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(HIWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(HIWORD(Ddbuf.lValue));
		 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //QDS

		/*time*/
		if((Ddbuf.Time.dwMinute == 0)&&(Ddbuf.Time.wMSecond==0))
		{
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			
		}
		else
		{
			CalClockTo(&Ddbuf.Time, &SysTime);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SysTime.bySecond*1000 + SysTime.wMSecond);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SysTime.bySecond*1000 + SysTime.wMSecond);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byHour; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (SysTime.byDay & 0x1F) | (SysTime.byWeek << 5); 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMonth; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.wYear -m_guiyuepara.baseyear; 
		}
		j++;
	}
	if(!j)
		return TRUE;

	write_VSQ(DDSendNum);
	SendFrameTail();
	
	return TRUE;
}

BOOL CGB104S::MakeTideFrzGroupFrame(WORD GroupNo,BYTE Reason)
{
	BYTE Style=207;
	DWORD DDSendNum,DDNo;
	struct VDDFT Ddbuf;
	struct VSysClock SysTime;
	DWORD j = 0;
	if(GroupNo >= g_ddhxnum)
		return FALSE;

	DDNo = GroupNo*32+24;
	
  if(DDNo >= m_pEqpInfo[m_wEqpNo].wDDNum)
		return FALSE;
	
	SendFrameHead(FRM_I);
	Sendframe(Style, Reason);
	
	for (DDSendNum = 0;(DDSendNum < FRM_DDNUM) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum);DDSendNum++,DDNo++)
	{
		write_infoadd(DDNo+DD_START_ADDR);
		memcpy(&Ddbuf, 0, sizeof(struct VDDFT));
		ReadRangeDDFT(m_wEqpID, DDNo, 1, sizeof(struct VDDFT), &Ddbuf);	
		//LongToFloat(0,Ddbuf.lValue,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
		//m_SendBuf.wWritePtr+=4;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(LOWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(LOWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(HIWORD(Ddbuf.lValue));
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(HIWORD(Ddbuf.lValue));
		 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //QDS

		/* time */
		if((Ddbuf.Time.dwMinute == 0)&&(Ddbuf.Time.wMSecond==0))
		{
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
			
		}
		else
		{
			CalClockTo(&Ddbuf.Time, &SysTime);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SysTime.bySecond*1000 + SysTime.wMSecond);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SysTime.bySecond*1000 + SysTime.wMSecond);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byHour; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (SysTime.byDay & 0x1F) | (SysTime.byWeek << 5); 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMonth; 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.wYear -m_guiyuepara.baseyear; 
		} 
		j++;
	}
	if(!j)
		return TRUE;

	write_VSQ(DDSendNum);
	SendFrameTail();
	
	return TRUE;
}

BOOL CGB104S::SendChangeFifFrz(void)
{
	int i;

	for(i=0; (i<FRM_DDNUM)&&(i<g_ddhxnum); i++)
	{
		if( g_ddfifflag & (1<<i) )
		{
			g_ddfifflag &= ~(1<<i);
			MakeFifFrzGroupFrame(i, 3);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CGB104S::SendChangeDayFrz(void)  
{
	int i;

	for(i=0; (i<FRM_DDNUM)&&(i<g_ddhxnum); i++)
	{
		if( g_dddayflag & (1<<i) )
		{
			g_dddayflag &= ~(1<<i);
			MakeDayFrzGroupFrame(i, 3);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CGB104S::SendChangeTide(void) 
{
	int i;

	for(i=0; (i<FRM_DDNUM)&&(i<g_ddhxnum); i++)
	{
		if( g_ddtideflag & (1<<i) )
		{
			g_ddtideflag &= ~(1<<i);
			MakeTideFrzGroupFrame(i, 3);
			return TRUE;
		}
	}

	return FALSE;
}



//召唤所有电度的确认
BOOL CGB104S::SendAllEnergyAck(void)
{
	BYTE Style = 0x65, Reason = 10;
	BYTE Num = 1;
	
	SendFrameHead(FRM_I);
	Sendframe(Style, Reason);
	write_infoadd(0);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 5; 

	write_VSQ(Num);
	SendFrameTail();
	
	return TRUE;
}


/*************发送故障事件********************/

BOOL CGB104S::SendFaultEvent(void) 
{
	VSysClock SysTime;
	BYTE TypeID = M_FT_NA_1;
	BYTE Reason = COT_SPONT;
#if(DEV_SP  == DEV_SP_TTU)
	SwitchEvent *m_Eventinfo;
#else
	VDBGzEvent *m_Eventinfo;
	WORD VEYxNo,VEYcNo,SendNo;	
#endif
	WORD yxnumpos;
	int i,k=0,ycnumpos;
	int ch;
#if(DEV_SP  == DEV_SP_TTU)
	ch = ReadSwitchEvent((SwitchEvent *)m_dwPubBuf);
	if (ch == ERROR)                                              
	    return FALSE;
	m_Eventinfo = (SwitchEvent *)m_dwPubBuf;
	
#else
	ch = prReadGzEvent((VDBGzEvent *)m_dwPubBuf);
	if (ch == ERROR)                                              
	    return FALSE; 

	m_Eventinfo = (VDBGzEvent *)m_dwPubBuf;
#endif
		
	SendFrameHead(FRM_I);
	Sendframe(TypeID, Reason);

	/*遥信个数*/
	yxnumpos = m_SendBuf.wWritePtr;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(m_Eventinfo->YxNum);

	/*遥信类型*/
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_guiyuepara.yxtype; 

	k=0;
	for(i = 0; i < m_Eventinfo->YxNum; i++)
	{
	
#if(DEV_SP  == DEV_SP_TTU)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(m_Eventinfo->YxNo[i]);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(m_Eventinfo->YxNo[i]);

#else
		/*故障遥信点号*/
		if(TEYxNoToVEYxNo(g_Sys.wIOEqpID, m_wEqpID,m_Eventinfo->YxNo[i], &VEYxNo) != OK)
			continue;
			
		ReadSYXSendNo(m_wEqpID,VEYxNo,&SendNo);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(SendNo+ADDR_YX_LO);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(SendNo+ADDR_YX_LO);
#endif
		if(m_guiyuepara.tmp[4]&0x01) //备用四 用于控制遥信信息地址长度是2还是3，默认2个字节
		    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;

		/*遥信值*/ 
		if(m_guiyuepara.yxtype == 3) 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_Eventinfo->YxValue[i]+1;
		else
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_Eventinfo->YxValue[i];        
		
		/*故障时刻时标 7BYTE */
		SystemClock(m_Eventinfo->Time[i].dwMinute, &SysTime);                      

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(m_Eventinfo->Time[i].wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(m_Eventinfo->Time[i].wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMinute;   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byHour & 0x1F;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SysTime.byDay & 0x1F) | ((SysTime.byWeek <<5) & 0xE0);    
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMonth;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.wYear % 100;

		k++;
	}
	if(!k)
		return FALSE;
	
	m_SendBuf.pBuf[yxnumpos] = k;
	
	/*遥测个数*/
	ycnumpos = m_SendBuf.wWritePtr;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_Eventinfo->YcNum;

	/*遥测类型*/
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_guiyuepara.yctype;

	k=0;
	for(i = 0; i < m_Eventinfo->YcNum; i++)
	{
		if(m_Eventinfo->YcNo[i] == 0xFFFF)
			continue;
#if(DEV_SP  == DEV_SP_TTU)		
		write_infoadd(m_Eventinfo->YcNo[i]);
                /*遥测值*/
                if(m_guiyuepara.yctype == 13)  //短浮点型 4B
                {   
                        LongToFloat(0,m_Eventinfo->YcValue[i]*100,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
                        m_SendBuf.wWritePtr+=4;
                }

#else
		if(TEYcNoToVEYcNo(g_Sys.wIOEqpID, m_wEqpID,m_Eventinfo->YcNo[i], &VEYcNo) != OK)
			continue;
		
		ReadYCSendNo(m_wEqpID, VEYcNo,&SendNo);
		/*遥测信息体地址*/
		write_infoadd(SendNo+ ADDR_YC_LO);
		
		/*遥测值*/
		if(m_guiyuepara.yctype == 13)  //短浮点型 4B
		{	
			if(i <=  3) // 电压放大十倍
				LongToFloat(0,m_Eventinfo->YcValue[i]*10,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
			else
				LongToFloat(0,m_Eventinfo->YcValue[i],m_SendBuf.pBuf+m_SendBuf.wWritePtr);
			m_SendBuf.wWritePtr+=4;
		}
#endif	
		else  //归一化值 2B
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(m_Eventinfo->YcValue[i]);
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(m_Eventinfo->YcValue[i]);
		}
		k++;
		
	}
	if(!k)
		return FALSE;
	
	m_SendBuf.pBuf[ycnumpos] = k;
	write_VSQ(0);
	
#ifdef INCLUDE_SEC_CHIP
	if(m_wCommID == COMM_IPSEC_NO)
		SendFrameTail0x1();// 故障事件，南网为01
	else
#endif
		SendFrameTail();
	
	return TRUE;
	
}

/*************************软件升级******************************/
BOOL CGB104S::DoSoftUpgrade(void)
{
	BYTE * pData = m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;
	BYTE ctype; 
	//static BYTE cnt=0,;
	static BYTE softupcmd=0;
	
	ctype = pData[0];

	if((m_dwasdu.COT != 6)&&(m_dwasdu.COT != 8))
	{
		errack(45);
		return FALSE;
	}

	if((m_dwasdu.COT == 6) && ((ctype & 0x80)== 0x80 )) //升级启动
	{
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if(3 == m_guiyuepara.jiami)
		{
		  	if(m_jiamiType != 1)
		  	{
		  		SecuritySend1FFrame(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
		  		m_SendBuf.wReadPtr = 0;
				SendAllFrame();
		  		return FALSE;
		  	}
		}
		if(4 == m_guiyuepara.jiami)
		{
		  	if(m_jiamiType != 1)
		  	{
		  		SecuritySend1FFrame_HN(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
		  		m_SendBuf.wReadPtr = 0;
				SendAllFrame();
		  		return FALSE;
		  	}
		}
#endif
		//cnt = 0;
		softupcmd = 1;
		SendFrameHead(FRM_I);
		Sendframe(m_dwasdu.TypeID, 7);
		
		memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
		m_SendBuf.wWritePtr += m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
		write_VSQ(m_dwasdu.VSQ);

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
		{//ly
			MD5_Init();
			SendFrameTail0x2();
		}
		else
#endif
			SendFrameTail();
		
	    return TRUE;

	}
	else if((m_dwasdu.COT == 6) && ((ctype & 0x80)==0 ))
	{
		/*if((cnt == 0)&&(softupcmd == 1))  //升级执行
		{
			cnt = 1;
			SendFrameHead(FRM_I);
			Sendframe(m_dwasdu.TypeID, 7);
			
			memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
			m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
			write_VSQ(m_dwasdu.VSQ);

			SendFrameTail();
		    return TRUE;
		}
		else*/ //升级结束    
		{
			//cnt = 0;
			softupcmd = 0;
			SendFrameHead(FRM_I);
			Sendframe(m_dwasdu.TypeID, 7);
			
			memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
			m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
			write_VSQ(m_dwasdu.VSQ);
			SendFrameTail();

			m_bysoftupflag=1;
		
			return TRUE;

		}
	}
	else if((m_dwasdu.COT == 8) && ((ctype & 0x80)==0 )) //撤销命令
	{
		if(softupcmd == 1)
		{
			softupcmd = 0;
			SendFrameHead(FRM_I);
			Sendframe(m_dwasdu.TypeID, 9);
			
			memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
			m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
			write_VSQ(m_dwasdu.VSQ);

			SendFrameTail();
		    
			return TRUE;
			
		}
		else
		{
			SendFrameHead(FRM_I);
			Sendframe(m_dwasdu.TypeID, 10);
			
			memcpy(m_SendBuf.pBuf+ m_SendBuf.wWritePtr,m_pReceive->byASDU+m_dwasdu.Infooff,m_pReceive->byAPDULen-4-m_dwasdu.Infooff);
			m_SendBuf.wWritePtr+=m_pReceive->byAPDULen-4-m_dwasdu.Infooff;
			write_VSQ(m_dwasdu.VSQ);

			SendFrameTail();
			return FALSE;
		}
		
	}
	
	else
	{
		errack(45);
		return FALSE;
	}
	

}

/****************************************************************/

void CGB104S::Recparaact(void)
{
	BYTE Qualifier = 0x1;
	BYTE *pdata=m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;
	if((m_dwasdu.Info<ADDR_YCPARA_LO)||(m_dwasdu.Info>ADDR_YCPARA_HI))
		m_dwasdu.COT|=0x40;
	if (CanSendIFrame())
	{
		SendFrameHead(FRM_I);
		Sendframe(P_AC_NA, m_dwasdu.COT+1);
		write_infoadd(m_dwasdu.Info);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = pdata[0];
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = pdata[1];
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (pdata[2]&0x3f);
		
		write_VSQ(Qualifier);
		SendFrameTail();
	}
	return;
}
BOOL CGB104S::Sendpara(void)
{
	BYTE* YcValue;
//	WORD ReadYcNum = 1;
	
	if (!CanSendIFrame())
		return  FALSE;

	if (m_wSendparaNum >= m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum)
		{
		m_wSendparaNum=0;
		m_QPM++;
		}
		if(m_QPM>4)
			{
			m_QPM=1;
		m_readparaeqpnum++;
			}
		if(m_readparaeqpnum>=m_wEqpNum)
			return false;
		SwitchToEqpNo(m_dwSendAllDataEqpNo);
	SendFrameHead(FRM_I);
	Sendframe(P_ME_NA, 34);


	write_infoadd(ADDR_YCPARA_LO+m_wSendparaNum);
	WORD i;
	WORD j=Geteqpparaaddr(GetEqpOwnAddr());
		YcValue=(BYTE*)limitpara+(limitpara[1+j*3+2]+(m_wSendparaNum)*sizeof(Vlimitpara));
	for( i=0; (i<64) && (m_wSendparaNum<m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum); i++, m_wSendparaNum++)
	{
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = *(YcValue+(m_QPM-1)*2+1);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =*(YcValue+(m_QPM-1)*2);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_QPM;//QDS
		YcValue+=sizeof(Vlimitpara);
	}
	
	write_VSQ(i | VSQ_SQ);
	SendFrameTail();
		
	return TRUE;
}
void CGB104S::writepara()
{	
		
		struct VFileOPMsg FileOPMsg;
		DWORD off;
		off=0;
		off+=(m_wEqpNum*3+1)*4;
		for(int i=0;i<m_wEqpNum;i++)
			{
				SwitchToEqpNo(i);
				off+=(m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum)*sizeof(Vlimitpara);
			}
	sprintf(FileOPMsg.cFileName,"port%d.para",m_wTaskID);
	FileOPMsg.dwOffset=0;
	FileOPMsg.dwLen=off;
	FileOPMsg.dwSize=off;
	WriteFile(&FileOPMsg,(BYTE*)limitpara);
}
void CGB104S::Recpara(void)
{
	BYTE Qualifier = 0x1;
	BYTE *pdata=m_pReceive->byASDU+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;
	if (CanSendIFrame())
	{
		SendFrameHead(FRM_I);
		Sendframe(P_ME_NA, m_dwasdu.COT+1);
		write_infoadd(m_dwasdu.Info);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = pdata[0];
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = pdata[1];
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (pdata[2]&0x3f)|0x40;
		
		write_VSQ(Qualifier);
		SendFrameTail();
	}
}
void CGB104S::CycleSendAllYcYx(BYTE reason)
{
	WORD i, YxGroupNum, YcGroupNum;
	
	if (!CanSendIFrame())
		return;
	
	SwitchToEqpNo(m_dwSendAllDataEqpNo);
	
	YxGroupNum = (m_pEqpInfo[m_wEqpNo].wSYXNum +m_pEqpInfo[m_wEqpNo].wVYXNum+ YX_GRP_NUM - 1) / YX_GRP_NUM;
	YcGroupNum = (m_pEqpInfo[m_wEqpNo].wYCNum + YC_GRP_NUM - 1) / YC_GRP_NUM;
         for(i=0; (i<31) && (i<YxGroupNum); i++)
	{
		if( m_dwcycYxSendFlag & (1<<i) )
		{
			if(SendGroupDYx(i, reason))
			return;
			if(SendGroupYx(i, reason))
			return;
			m_wSendDYxNum=0;
			m_wSendYxNum=0;
			m_dwcycYxSendFlag &= ~(1<<i);
		}
	}
	#if 1
	for(i=0; (i<31) && (i<YcGroupNum); i++)
	{
		if( m_dwcycYcSendFlag & (1<<i) )
		{
			if(SendGroupYc(i, reason))
			return;
			m_dwcycYcSendFlag &= ~(1<<i);
			m_wSendYcNum=0;
		}
	}
	#endif
	event_time=m_pBaseCfg->Timer.wScanData2;
	commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
		if( m_dwcycYxSendFlag & 0x80000000 )
	{
		m_dwSendAllDataEqpNo++;
		if ( m_dwSendAllDataEqpNo >= m_wEqpNum)
		{
			m_dwSendAllDataEqpNo = 0;
			m_dwcycYxSendFlag = 0;
			m_dwcycYcSendFlag = 0;
			m_allcycleflag&=0x7f;
			m_allsendtime=0;
		}
		else
		{
			
			m_dwcycYxSendFlag = 0xffffffff;
			m_dwcycYcSendFlag = 0xffffffff;	
		}
	}
}


int CGB104S::ReadPara(WORD addr, char* pbuf)
{
	BOOL type;
	type = m_guiyuepara.tmp[7];
	if((addr >= SELFPARA_ADDR) && (addr < RUNPARA_ADDR))	
	{
			return ReadSelfPara(addr,pbuf);
	}
	else if(((addr >= RUNPARA_ADDR) && (addr < PRPARA_ADDR))
#if(DEV_SP == DEV_SP_TTU)
		||((addr >=(PRPARA_TTU_ADDR + PRP_HAR_DEAD_V))&&(addr <=(PRPARA_TTU_ADDR + PRP_DEAD_LOAD)))
#endif
        )
		return GB104ReadRunPara(addr,pbuf);
	else if((addr >= PRPARA_ADDR) && (addr <= PRPARA_LINE_ADDREND))
	{
#ifdef INCLUDE_PR
		return ReadPrPara(addr,pbuf,type);
#endif
	}
#if(DEV_SP  == DEV_SP_TTU)
	else if(((addr >= PRPARA_TTU_ADDR) && (addr <= PRPARA_TTU_ADDR + PRP_SWTICH_TIME_CYC ))||(addr==PRPARA_TTU_ADDR + PRP_SHORTCIR_CAP))
		return ReadPbRunPara(addr,pbuf);
	else
		return GetConst_value(addr,pbuf);
#else
	else
		return ERROR;
#endif
}

int CGB104S::WriteParaYZ(WORD addr, char* pbuf) //参数预置
{
	if(((addr >= RUNPARA_ADDR) && (addr < PRPARA_ADDR))||
		((addr >=(PRPARA_TTU_ADDR + PRP_HAR_DEAD_V))&&(addr <=(PRPARA_TTU_ADDR + PRP_DEAD_LOAD))))
		return WriteRunParaYZ(addr,pbuf);
	else if((addr >= PRPARA_ADDR) && (addr <= PRPARA_LINE_ADDREND))
	{
#ifdef INCLUDE_PR
		return WritePrParaYZ(addr,pbuf);
#endif
	}
#if(DEV_SP  == DEV_SP_TTU)
	else if(((addr >= PRPARA_TTU_ADDR) && (addr <= PRPARA_TTU_ADDR + PRP_SWTICH_TIME_CYC ))||(addr==PRPARA_TTU_ADDR + PRP_SHORTCIR_CAP))
		return WritePbRunParaYZ(addr,pbuf);
	else
		return WriteSwitchParaYZ(addr,pbuf);
#else
	else
		return 3;
#endif
}
int CGB104S::WriteSwitchParaYZ(WORD addr, char* pbuf)
{
	struct VParaInfo *fParaInfo;
	int no;

	fParaInfo = (struct VParaInfo*)pbuf;
	if((addr < PRPARA_SWITCH_ADDR)||(addr > PRPARA_SWITCH_ADDREND))
	{
		return ERROR;
	}
	no=(addr - PRPARA_SWITCH_ADDR)%PRPARA_EVER_SWITCH_NUM;
	switch(no)
	{
		case PRP_RESDUAL_I_VALUE:
		case PRP_RESDUAL_I_TIME:
		case PRP_SHORTDELAY_I_TIME:
		case PRP_SWITCH_RETURN_TIME:
			if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
				return 2;			
			return 0;
		case PRP_RESDUAL_I_CONTROL:
		case PRP_SHORT_I_VALUE:
		case PRP_SHORT_I_CONTROL:
		case PRP_SHORTDELAY_I_VALUE:
		case PRP_SHORTDELAY_I_CONTROL:
		case PRP_OVERLOAD_I_TIME:
		case PRP_OVERLOAD_I_CONTROL:
		case PRP_OVER_U_CONTROL:
		case PRP_UNDER_U_CONTROL:
		case PRP_BREAKPHASE_U_CONTROL:
		case PRP_ZERO_U_CONTROL:
		case PRP_REST_GATE_CONTROL:	
			if((fParaInfo->type != UTINY_TYPE) || (fParaInfo->len != UTINY_LEN))
				return 2;
			return 0;
		case PRP_OVERLOAD_I_VALUE:
		case PRP_OVER_U_VALUE:
		case PRP_UNDER_U_VALUE:
		case PRP_BREAKPHASE_U_VALUE:
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			return 0;
		case PRP_SELF_CHECK_TIME:
			if(fParaInfo->type != STRING_TYPE) 
				return 2;
			return 0;
		default:
			return 3;
	}
}
int CGB104S::WriteParaGH(WORD addr, char* pbuf) //参数固化
{
	WORD type;
	type = m_guiyuepara.tmp[7];

	if(((addr >= RUNPARA_ADDR) && (addr <  PRPARA_ADDR))||
		((addr >=(PRPARA_TTU_ADDR + PRP_HAR_DEAD_V))&&(addr <=(PRPARA_TTU_ADDR + PRP_DEAD_LOAD))))
		return GB104WriteRunPara(addr,pbuf);
	else if((addr >= PRPARA_ADDR) && (addr <= PRPARA_LINE_ADDREND))
	{
#ifdef INCLUDE_PR
		return WritePrPara(addr,pbuf,type);
#endif
	}
#if(DEV_SP  == DEV_SP_TTU)
	else if(((addr >= PRPARA_TTU_ADDR) && (addr <= PRPARA_TTU_ADDR + PRP_SWTICH_TIME_CYC ))||(addr==PRPARA_TTU_ADDR + PRP_SHORTCIR_CAP))
		return WritePbRunPara(addr,pbuf,m_guiyuepara.tmp[5]);
	else
		return SetConst_value(addr,pbuf);
#else
	else
		return 3;
#endif
}

int CGB104S::GB104ReadRunPara(WORD parano,char* pbuf)
{
	if((parano < RUNPARA_ADDR) ||
		((parano >= PRPARA_ADDR)&&(parano < PRPARA_TTU_ADDR))||
		(parano > PRPARA_TTU_ADDREND))
		return ERROR;
	
	if((parano < (RUNPARA_ADDR + RPR_DV_NUM))||
		((parano >=(PRPARA_TTU_ADDR + PRP_HAR_DEAD_V))&&(parano <=(PRPARA_TTU_ADDR + PRP_DEAD_LOAD))))
		return ReadDeadValue(parano,pbuf);
	else 
		return ReadRunPara(parano,pbuf);
	
}

int CGB104S::GB104WriteRunPara(WORD parano,char* pbuf)
{

	if((parano < (RUNPARA_ADDR + RPR_DV_NUM))||
		((parano >=(PRPARA_TTU_ADDR + PRP_HAR_DEAD_V))&&(parano <=(PRPARA_TTU_ADDR + PRP_DEAD_LOAD))))		
		return WriteDeadValue(parano,pbuf);
	else 
		return WriteRunPara(parano,pbuf);
}
int CGB104S::GB104WriteRunParaFile()
{
	BOOL ret,deadvaluefind,runparafind;
	int i;
	VFileMsgs fmsg;
	
	ret = OK;
	deadvaluefind = runparafind = 0;
	for(i = 0; i< m_WriteNum;i++)
	{
		if((WriteParaAddr[i] >= RUNPARA_ADDR) && (WriteParaAddr[i] < (RUNPARA_ADDR + RPR_DV_NUM)))
			deadvaluefind = 1;
		if((WriteParaAddr[i] >= (RUNPARA_ADDR + RPR_DV_NUM)) && (WriteParaAddr[i] < PRPARA_ADDR))
			runparafind = 1;
	}
	if(deadvaluefind)
	{
		if(WriteDeadValueFile() == ERROR)
			 ret = ERROR;
	}
	if(runparafind)
	{
	        fmsg.type = MSG_FILE;
		fmsg.num = 1;
		fmsg.filemsg[0].type = FILE_TYPE_RUN;
		msgSend(B2F_ID, &fmsg, sizeof(VFileMsgs), 1);
		ret = OK;
	}
	return ret;
}


int CGB104S::ReadDeadValue(WORD parano,char*pbuf)
{
	struct VParaInfo *fParaInfo;
	WORD no;
	
	fParaInfo = (struct VParaInfo*)pbuf;
	
	if((parano < RUNPARA_ADDR) ||
		((parano >= (RUNPARA_ADDR + RPR_DV_NUM))&&(parano <(PRPARA_TTU_ADDR + PRP_HAR_DEAD_V)))||
		(parano>(PRPARA_TTU_ADDR + PRP_DEAD_LOAD)))
		return ERROR;
	if((parano >= RUNPARA_ADDR) &&(parano <= (RUNPARA_ADDR + RPR_DV_NUM)))	
	{
	no = parano - RUNPARA_ADDR;
	
	switch(no)
	{
		case RPR_DV_I:
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			memcpy(fParaInfo->valuech,&fIDeadValue,fParaInfo->len);
			break;
		case RPR_DV_AC:
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			memcpy(fParaInfo->valuech,&fACDeadValue,fParaInfo->len);
			break;
		case RPR_DV_DC:
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			memcpy(fParaInfo->valuech,&fDCDeadValue,fParaInfo->len);
			break;
		case RPR_DV_P:
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			memcpy(fParaInfo->valuech,&fPQDeadValue,fParaInfo->len);
			break;
		case RPR_DV_F:
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			memcpy(fParaInfo->valuech,&fFDeadValue,fParaInfo->len);
			break;
		case RPR_DV_COS:
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			memcpy(fParaInfo->valuech,&fPWFDeadValue,fParaInfo->len);
			break;
		default:
			break;
		}
	}
	else
	{
		no = parano - PRPARA_TTU_ADDR;
		switch(no)
		{
			case PRP_HAR_DEAD_V:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				memcpy(fParaInfo->valuech,&fUTHDeadValue,fParaInfo->len);
				break;
			case PRP_HAR_DEAD_I:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				memcpy(fParaInfo->valuech,&fITHDeadValue,fParaInfo->len);
				break;
			case PRP_DEAD_UNBALANCE:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				memcpy(fParaInfo->valuech,&fUNDeadValue,fParaInfo->len);
				break;
			case PRP_DEAD_LOAD:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				memcpy(fParaInfo->valuech,&fLoadDeadValue,fParaInfo->len);
				break;				
		}	
	}
	return OK;
}

int CGB104S::WriteDeadValue(WORD parano,char*pbuf)
{
	struct VParaInfo *fParaInfo;
	float fvalue;
	WORD no;

	 fParaInfo = (struct VParaInfo*)pbuf;
	if((parano < RUNPARA_ADDR) ||
		((parano >= (RUNPARA_ADDR + RPR_DV_NUM))&&(parano <PRPARA_TTU_ADDR + PRP_HAR_DEAD_V))||
		(parano>PRPARA_TTU_ADDR + PRP_DEAD_LOAD))
		return ERROR;
	if((parano >= RUNPARA_ADDR) &&(parano <= (RUNPARA_ADDR + RPR_DV_NUM)))	
	{	
	no = parano - RUNPARA_ADDR;
	
	switch(no)
	{
		case RPR_DV_I:

			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			fIDeadValue = fvalue;
			if(m_guiyuepara.tmp[5] == 2)
				m_pBaseCfg->wIDeadValue = fvalue * 10;
			else  if(m_guiyuepara.tmp[5] == 1)
				m_pBaseCfg->wIDeadValue = (fvalue*1000)/g_Sys.MyCfg.RunParaCfg.LineCfg[0].wCt2;
			else
				m_pBaseCfg->wIDeadValue = fvalue * 1000;
			break;
		case RPR_DV_AC:
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			fACDeadValue = fvalue;
			if(m_guiyuepara.tmp[5] == 2)
				m_pBaseCfg->wACDeadValue = fvalue * 10;
			else  if(m_guiyuepara.tmp[5] == 1)
				m_pBaseCfg->wACDeadValue =(fvalue*1000)/g_Sys.MyCfg.RunParaCfg.wPt2;
			else
				m_pBaseCfg->wACDeadValue = fvalue * 1000;
			break;
		case RPR_DV_DC:
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			fDCDeadValue = fvalue;
			if((m_guiyuepara.tmp[5] == 2)||(m_guiyuepara.tmp[5] == 1))
				m_pBaseCfg->wDCDeadValue = fvalue * 10;
			else
				m_pBaseCfg->wDCDeadValue = fvalue * 1000;

			break;
		case RPR_DV_P:

			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				
			fPQDeadValue = fvalue;
			if((m_guiyuepara.tmp[5] == 2)||(m_guiyuepara.tmp[5] == 1))
				m_pBaseCfg->wPQDeadValue = fvalue * 10;
			else
				m_pBaseCfg->wPQDeadValue = fvalue * 1000;			
			break;
		case RPR_DV_F:
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			fFDeadValue = fvalue;
			if((m_guiyuepara.tmp[5] == 2)||(m_guiyuepara.tmp[5] == 1))
				m_pBaseCfg->wFDeadValue = fvalue * 10;
			else
				m_pBaseCfg->wFDeadValue = fvalue * 1000;	
		
			break;
		case RPR_DV_COS:

			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			fPWFDeadValue = fvalue;
			if((m_guiyuepara.tmp[5] == 2)||(m_guiyuepara.tmp[5] == 1))
				m_pBaseCfg->wPWFDeadValue = fvalue * 10;
			else
				m_pBaseCfg->wPWFDeadValue = fvalue * 1000;			
			break;
		default:
			break;
		}
	}
	else
	{
		no = parano - PRPARA_TTU_ADDR;
		switch(no)
		{
			case PRP_HAR_DEAD_V:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);					
				fUTHDeadValue = fvalue;
				if((m_guiyuepara.tmp[5] == 2)||(m_guiyuepara.tmp[5] == 1))
					m_pBaseCfg->wUTHDeadValue = fvalue * 10;
				else
					m_pBaseCfg->wUTHDeadValue = fvalue * 1000;	

				break;
			case PRP_HAR_DEAD_I:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);					
				fITHDeadValue = fvalue;
				if((m_guiyuepara.tmp[5] == 2)||(m_guiyuepara.tmp[5] == 1))
					m_pBaseCfg->wITHDeadValue = fvalue * 10;
				else
					m_pBaseCfg->wITHDeadValue = fvalue * 1000;	

				break;
			case PRP_DEAD_UNBALANCE:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);					
				fUNDeadValue = fvalue;
				if((m_guiyuepara.tmp[5] == 2)||(m_guiyuepara.tmp[5] == 1))
					m_pBaseCfg->wUNDeadValue = fvalue * 10;
				else
					m_pBaseCfg->wUNDeadValue = fvalue * 1000;	

				break;
			case PRP_DEAD_LOAD:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);	

				fLoadDeadValue = fvalue;
				if((m_guiyuepara.tmp[5] == 2)||(m_guiyuepara.tmp[5] == 1))
					m_pBaseCfg->wLoadDeadValue = fvalue * 10;
				else
					m_pBaseCfg->wLoadDeadValue = fvalue * 1000;	
				
				break;				
		}	
	}
	return 0;
}

int CGB104S::WriteDeadValueFile()
{
	int para_id;
	char fname[MAXFILENAME];
	VProtocolBaseCfg *pBaseCfg;
	
	
	para_id = GetParaPortId(m_wTaskID);
	sprintf(fname,"portcfg%d.cfg",para_id);
	
	pBaseCfg = (VProtocolBaseCfg*)(g_pParafileTmp + 1);
	memset(pBaseCfg,0,sizeof(VProtocolBaseCfg) + sizeof(m_guiyuepara));
	
	memcpy(pBaseCfg,m_pBaseCfg,sizeof(VProtocolBaseCfg));
        pBaseCfg->Timer.wScanData2 =  pBaseCfg->Timer.wScanData2*10;
	memcpy(pBaseCfg + 1,&m_guiyuepara,sizeof(m_guiyuepara));
	
	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(VProtocolBaseCfg) + sizeof(m_guiyuepara);
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = CFG_ATTR_NULL;
	
	return (WriteParaFile(fname,g_pParafileTmp));
}

void CGB104S::DoClearSoe(void)
{
	WORD cosrdptr,coswtptr;
	QuerySSOE(m_wEqpID, 1, 1, 1000,
    		(VDBSOE *)m_dwPubBuf, &m_WritePtr, &m_BufLen);
			m_ReadPtr=m_WritePtr;
	GetPoolPtr(m_wTaskID,m_wEqpID,SCOSUFLAG,&cosrdptr,&coswtptr);
	while(cosrdptr != coswtptr)
	{// 增加pRunInfo->wDSOEReadPtr，不然ReadSCOS有问题
		QuerySCOS(m_wEqpID, 1, cosrdptr, 1000,
    		(VDBCOS *)m_dwPubBuf, &coswtptr, &m_BufLen);
		cosrdptr = (cosrdptr + 1)%m_BufLen;
		ReadPtrIncrease(m_wTaskID, m_wEqpID, 1, SCOSUFLAG);
	}
	
}

#ifdef FENGXIAN
BOOL CGB104S::SendSingleSoe_Delay(int no)
{
	VDBSOE pDelaySOE;
    VSysClock SysTime;
	BYTE TypeID = M_SP_TB;
	BYTE Reason = COT_SPONT;

	if (!CanSendIFrame())
	    return FALSE;

	pDelaySOE.byValue = m_delaysoe[no].SoeValue;
	pDelaySOE.wNo = m_delaysoe[no].SoeNo;
	pDelaySOE.Time.dwMinute = m_delaysoe[no].Time.dwMinute+YXDelayTime;
	pDelaySOE.Time.wMSecond = m_delaysoe[no].Time.wMSecond;

	SendFrameHead(FRM_I);
	if(m_guiyuepara.yxtype==2) 
		TypeID = 2;        //短时标单点遥信
	if(m_guiyuepara.yxtype==3) 
		TypeID = M_DP_TB;        //短时标单点遥信
	Sendframe(TypeID, Reason);   

	write_infoadd(pDelaySOE.wNo + ADDR_YX_LO);

#if (TYPE_USER == USER_SHANGHAIJY)
	if(m_guiyuepara.yxtype==3)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SIQ(pDelaySOE.byValue)+1)& 0x03;
	else
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SIQ(pDelaySOE.byValue))& 0x03;
#else
	if(m_guiyuepara.yxtype==3)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pDelaySOE.byValue)+1;
	else
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pDelaySOE.byValue);
#endif
	
	SystemClock(pDelaySOE.Time.dwMinute, &SysTime);        	

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(pDelaySOE.Time.wMSecond);   
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(pDelaySOE.Time.wMSecond);   
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMinute;   
	if(TypeID != 2)//短时标单点遥信 
	{
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byHour & 0x1F;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SysTime.byDay & 0x1F) | ((SysTime.byWeek <<5) & 0xE0);    
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMonth;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.wYear % 100;
	}   
	
    write_VSQ(1);
	SendFrameTail();
	return TRUE; 
}

BOOL CGB104S::SendClassOneData_Delay(int no)
{
	VDBCOS pDelayCOS;
	BYTE TypeID = M_SP_NA;
	BYTE Reason = COT_SPONT;

	if (!CanSendIFrame())
	    return FALSE;

	//ClearTaskFlag(TASKFLAG_SCOSUFLAG);
	//ClearEqpFlag(EQPFLAG_SCOSUFLAG);

	pDelayCOS.byValue = m_delaycos[no].CosValue;
	pDelayCOS.wNo = m_delaycos[no].CosNo;
	
	SendFrameHead(FRM_I);
	if(m_guiyuepara.yxtype==3)
		TypeID=M_DP_NA;
	Sendframe(TypeID,Reason);
	write_infoadd(pDelayCOS.wNo + ADDR_YX_LO);
	
#if (TYPE_USER == USER_SHANGHAIJY)
	if(m_guiyuepara.yxtype==3)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SIQ(pDelayCOS.byValue)+1)& 0x03;
	else
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SIQ(pDelayCOS.byValue))& 0x03;
#else
	if(m_guiyuepara.yxtype==3)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pDelayCOS.byValue)+1;
	else
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pDelayCOS.byValue);
#endif
	
    write_VSQ(1);
	SendFrameTail();
	//SetTaskFlag(TASKFLAG_SENDSCOS);
	//SetEqpFlag(EQPFLAG_SENDSCOS);
	//ClearUFlag(m_wEqpNo);
	return TRUE; 
}

#endif


#ifndef INCLUDE_4G
#ifdef INCLUDE_GPRS
void CGB104S::SendGetRSSI(void)//获取RSSI信号
{
	BYTE TLVS[]={0x81,0x84,0x00,0x00};

	WORD wLinkAddress;
	VExtGPRSFrmHead *m_pSendBuf;
	m_SendBuf.wWritePtr = m_SendBuf.wReadPtr = 0;
	m_pSendBuf = (VExtGPRSFrmHead *)m_SendBuf.pBuf;
	m_pSendBuf->byCode1 = EXTGPRS_CODE1;
	m_pSendBuf->byCode2 = EXTGPRS_CODE2;
	m_pSendBuf->byCode3 = EXTGPRS_CODE1;
	m_pSendBuf->byCode4 = EXTGPRS_CODE2;
	m_pSendBuf->byType = EXTGPRS_TYPE_STATE;
	m_pSendBuf->byLen1 = HIBYTE(sizeof(TLVS));
	m_pSendBuf->byLen2 = LOBYTE(sizeof(TLVS));
	m_SendBuf.wWritePtr = sizeof(VExtGPRSFrmHead);
	
	memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], TLVS, sizeof(TLVS));
	m_SendBuf.wWritePtr += sizeof(TLVS);

	byAddCrc16(m_SendBuf.pBuf, m_SendBuf.wWritePtr);
	m_SendBuf.wWritePtr += 2;
	wLinkAddress = m_EqpGroupInfo.wDesAddr;

	WriteToComm(wLinkAddress);

}
#endif
#endif	
 #endif
