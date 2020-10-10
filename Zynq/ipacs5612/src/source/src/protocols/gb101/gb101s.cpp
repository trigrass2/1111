/*------------------------------------------------------------------------
 $Rev: 3 $
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef INCLUDE_GB101_S

#include "os.h"
#include "gb101s.h"
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

#define GRP_YCNUM	60			//每组最多发送遥测个数
#if(TYPE_USER == USER_GUANGXI)

#define GRP_YCNUM	20			//每组最多发送遥测个数
#endif
#define GRP_YXNUM	100			//每组最多发送遥信字节
#define GRP_DDNUM	8			//每组最多发送电度个数
#define GRP_SOENUM	20	  		//每组最多发送SOE个数

#define FRM_YCNUM	 64			//每帧报文最多发送遥测个数
#define FRM_YXNUM	 100		//每帧报文最多发送遥信个数
#define FRM_DDNUM	8			//每帧报文最多发送电度个数

extern WORD g_ddfifflag, g_dddayflag, g_ddtideflag;
extern WORD DD_START_ADDR,DD_REAL_START_ADDR,DD_FIF_START_ADDR,DD_DAY_START_ADDR,DD_TIDE_START_ADDR;
extern BYTE g_ddhxnum;
/***************************************************************
	Function：sec101entry
		从站类101规约入口函数
	参数：wTaskID, arg
		wTaskID 任务ID
		arg 任务参数指针
	返回：无
***************************************************************/
extern "C" void gb101s(WORD wTaskID)		
{
	CGB101S *pGB101S = new CGB101S(); 
	
	if (pGB101S->Init(wTaskID) != TRUE)
	{
		pGB101S->ProtocolExit();
		return;
	}
	pGB101S->Run();			   
	
}

extern "C" void gb101s_load(int tid)
{
    char tname[30];
	
	sprintf(tname,"tGb101s%d",tid);		
	SetupCommonTask(tid, tname, COMMONPRIORITY, COMMONSTACKSIZE, (ENTRYPTR)gb101s, COMMONQLEN);
}

/***************************************************************
	Function：CGB101S
		构造函数，暂空
	参数：无
		
	返回：无
***************************************************************/
CGB101S::CGB101S() : CPSecondary()
{
}


/***************************************************************
	Function：Init
		规约初始化
	参数：wTaskID
		wTaskID 任务ID 
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CGB101S::Init(WORD wTaskID)
{
	BOOL rc;
	rc = CPSecondary::Init(wTaskID,1,&m_guiyuepara,sizeof(m_guiyuepara));
	if (!rc)
	{
		return FALSE;
	}

	Iec101Info = new VIec101Info[m_wEqpNum];
	m_paranum = 0;
	m_nextparanum = 0;
	nextflag = 0;
	ClearAllEqpFlag(CALL_DATA);	//清除召唤数据标志
	m_wSendYcNum = 0;
	m_wSendYxNum = 0;
	m_wSendDdNum = 0;
	m_wSendDYxNum=0;
	m_pVIec101Cfg = (VIec101Cfg *)&m_Iec101Cfg;
	m_pVIec101Cfg->byLinkAdr =m_guiyuepara.linkaddrlen;// LOBYTE(LOWORD(m_pBaseCfg->dwRsv[0]));
	m_pVIec101Cfg->byCommAdr =m_guiyuepara.conaddrlen;// HIBYTE(LOWORD(m_pBaseCfg->dwRsv[0]));
	m_pVIec101Cfg->byInfoAdr = m_guiyuepara.infoaddlen;//LOBYTE(HIWORD(m_pBaseCfg->dwRsv[0]));
	m_pVIec101Cfg->byCOT =m_guiyuepara.COTlen;// HIBYTE(HIWORD(m_pBaseCfg->dwRsv[0]));
	m_YKflag=0;
	m_YTflag=0;
	m_callallflag=0;
	m_acdflag=0;
	m_yxchangeflag=0;
	m_SOEflag=0;
	m_readddflag=0;
	m_zdflag=0;
	m_linkflag=0;
	m_testflag=0;
	m_delayflag=0; 
	m_set_paraflag=0;
	m_timeflag=0;
	m_readtimeflag = 0;
	m_fcb=0x20;
	m_sourfaaddr=0;
	m_allcalloverflag=0;
	m_resetflag=0;
	m_initflag=4;
	m_errflag=0;
    m_allcycleflag = 0;
	SwitchToEqpNo(0);
	QuerySSOE(m_wEqpID, 1, 1, 1000,(VDBSOE *)m_dwPubBuf, &m_WritePtr, &m_BufLen);
	m_ReadPtr=m_WritePtr;
	if(m_guiyuepara.mode==1)
	{
		m_initflag=7;
		SwitchToEqpNo(0);
		m_dwasdu.LinkAddr=GetAddress();
				//m_recfalg=2;
		//	DWORD event_time = 500;
		//commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);//lqh 
	}
	for (int i = 0; i < m_wEqpNum; i++)
		m_pEqpInfo[i].pProtocolData = new VCtrlInfo;

	m_fileoff=0;
	m_calldir=0;
	m_byReadFileAckFlag=0;
	m_byReadFileFlag = 0;
	m_fileoffset = 0;

	m_byWriteFileAckFlag=0;
	m_byWriteFileDataAckFlag=0;
	m_bySwitchValNoFlag=0;
	m_byReadValNoFlag=0;
	m_byAllParaValFlag=0;
	m_byMoreParaValFlag=0;
	m_byWriteParaStatus=0;
	m_paraaddrnum = 0;
	m_byCallEnergyFlag=0;
	m_bySoftUpgradeStatus=0;
	m_bysoftupflag=0;

	m_dwfiffrzflag=0;
	m_dwrealdataflag=0;
	m_dwdayfrzflag=0;
	m_dwCallDDFlag=0;

	m_fdno = 0;
	m_w101sTaskID = 0;

	if(m_guiyuepara.tmp[6] > 0)
	{
		m_fdno = m_guiyuepara.tmp[6];
		m_w101sTaskID = wTaskID;
	}
	set_infoaddr = PRPARA_LINE_ADDR+m_fdno*PRPARA_LINE_NUM;
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

#ifdef INCLUDE_RECORD
	ReadRcdDir(NULL,1);
#endif

	if(!m_guiyuepara.mode)
	{
		strcpy(&g_Sys.InPara[SELFPARA_PROTOCOL][0],"2016鐗堟墿灞曢潪骞宠　寮?01");
		strcpy(&g_Sys.InPara[SELFPARA_PROTOCOL_GB2312][0],"2016版扩展非平衡式101");
	}

//	ECES_Test();
	return TRUE;
}

void CGB101S::ECES_Test()
{
 /*

ECCrefPublicKey pucPublicKey;  
pucPublicKey.bits=0x00000100;
ECCSignature pucSignature;
unsigned char kkx[32]={
	        0xB0,0x5D,0xB8,0xF3,0x05,0xE2,0x1B,0x88,
			0xE2,0x14,0x8A,0x8F,0x3D,0x7D,0x88,0xD8,
			0xAF,0x2F,0x4C,0xD3,0x64,0xB8,0x5F,0x6B,
			0x36,0xE4,0x35,0xA9,0xA6,0x38,0x24,0x81};
	unsigned char kky[32]={0x53,0x10,0xF5,0x15,0xE2,0xC8,0x14,0xDA,
			0xE4,0x17,0x66,0xF5,0x8E,0x87,0x84,0x8A,
			0x93,0x6B,0x8A,0x91,0x53,0xB8,0xF1,0x0F,
			0xD3,0xBE,0x04,0x78,0xFB,0x07,0x5D,0xDF};

	unsigned char rt[32]={
			0x7D,0x25,0xBA,0x4E,0xB3,0x83,0xA8,0x7F,
			0x2F,0x41,0x0F,0x1E,0x0A,0x19,0xAB,0x14,
			0xE2,0x36,0x17,0x4A,0xA3,0x11,0xD6,0x16,
			0x69,0xD9,0x22,0x7B,0xD3,0x35,0x8B,0x7D	
		
		};


	unsigned char st[32]={
			0x28,0x3A,0xDA,0x53,0x0F,0x6C,0xC1,0xC0,
			0x88,0x99,0x57,0x36,0x2B,0x7D,0xFD,0xD4,
			0x7E,0x4A,0xF3,0x5A,0x56,0x52,0x65,0xB2,
			0x92,0xDA,0x96,0xAA,0x20,0x99,0x70,0xBF		
		
		};
	unsigned char pub[64],prv[32];
	memcpy(pucPublicKey.x,kkx,32);

   memcpy(pucPublicKey.y,kky,32);

	memcpy(pucSignature.r,rt,32);

   memcpy(pucSignature.s,st,32);

	unsigned char pucID[18] = {
			0x41,0x4C,
			0x49,0x43,0x45,0x31,0x32,0x33,0x40,0x59,
			0x41,0x48,0x4F,0x4F,0x2E,0x43,0x4F,0x4D
		};
		unsigned char pucDataInput[14] = {
			0x6D,0x65,0x73,0x73,0x61,0x67,0x65,0x20,
			0x64,0x69,0x67,0x65,0x73,0x74
		};

 int mm;
	  {
	   mm=	SM2_Verify(pucDataInput,14,pucID,18,&pucPublicKey,&pucSignature);
	  }
		
	logMsg("mm:%d\n", mm,0,0,0,0,0);
*/
}
void CGB101S::CheckBaseCfg(void)
{
	CProtocol::CheckBaseCfg();
	if(m_pBaseCfg->wBroadcastAddr==0)
		m_pBaseCfg->wBroadcastAddr=0xff;
}

void CGB101S::SetBaseCfg(void)
{
	CProtocol::SetBaseCfg();
	m_pBaseCfg->wBroadcastAddr = 0xFF;
}


/***************************************************************
	Function：DoYKRet
		遥控返校消息处理
	参数：pMsg
		pMsg 消息缓冲区头指针
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CGB101S::DoYKRet(void)
{
	CPSecondary::DoYKRet();
	if (!CheckClearEqpFlag(EQPFLAG_YKRet))
		return FALSE;  //防止遥控返校信息回来时已切换到别的模块
	m_YKflag=1;
	return TRUE;
}

//获取变位遥信个数
WORD CGB101S::GetCosNum(WORD EqpID)
{
	if (TestFlag(m_wTaskID, EqpID, SCOSUFLAG))
		return 1;	
	else 
		return 0;
}
//获取变位遥信个数
WORD CGB101S::GetDCosNum(WORD EqpID)
{
	if (TestFlag(m_wTaskID, EqpID, DCOSUFLAG))
		return 1;	
	else 
		return 0;
}
//校验和
BYTE CGB101S::ChkSum(BYTE *buf, WORD len)
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

//链路地址域（子站站址）
DWORD CGB101S::GetAddress(void)
{
	return GetOwnAddr();
}


/***************************************************************
	Function：OnTimeOut
		定时处理
	参数：TimerID
		TimerID 定时器ID
	返回：无
***************************************************************/
void CGB101S::DoTimeOut(void)
{
	DWORD event_time;
	static BYTE cnt=0;

	CPSecondary::DoTimeOut();
#ifndef INCLUDE_4G	
#ifdef INCLUDE_GPRS
	static BYTE cnt1=0;
	cnt1++;
	if((m_wCommID == (GprsChan.GPRS_COMNO + COMM_SERIAL_START_NO)) && (SignalNo != -1) && (cnt1 == 60) &&(GprsChan.gprsPara.cfg & 0x01))
	{
		cnt1 = 0;
		SendGetRSSI();
	}

#endif
#endif
	//if((m_linkdelaytime++>10)&&(m_guiyuepara.mode==1))
	//SendLinktesetFrame(1, 2);
	if(m_linkflag==0) return;

	if(m_bysoftupflag)
	{
		cnt++;
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if((cnt == 3) && (3 == m_guiyuepara.jiami))
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
		if((cnt == 3) && (4 == m_guiyuepara.jiami))
		{//ly
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

	
	if(retrflag==1)
	{
		if(m_retime++>30)
		{
			m_retime=0;
			if(retrnum>0)
			{
				retrnum--;
				m_SendBuf.wReadPtr = m_SendBuf.wOldReadPtr; 
				WriteToComm(m_SendBuf.dwFlag);
			}
			else
			{
				retrflag=0;
				m_retime=0;
				retrnum=0;
				m_linkflag=0;
			}
		}
	}
	
	if(m_yxchangeflag|m_SOEflag|(m_ycchangeflag++> 1))
	{
		if(m_guiyuepara.mode==0) return;
		event_time = 1;
		commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);//lqh 
	}
	/*else
	{
		if (TestFlag(m_wTaskID, m_wEqpID, SCOSUFLAG))
			m_yxchangeflag=1;
		if (TestFlag(m_wTaskID, m_wEqpID, DCOSUFLAG))
			m_yxchangeflag=1;
		if (TestFlag(m_wTaskID, m_wEqpID, SSOEUFLAG))
			m_SOEflag=1;
		if (TestFlag(m_wTaskID, m_wEqpID, DSOEUFLAG))
			m_SOEflag=1;
	}*/
	
	m_allsendtime++;
	if(m_pBaseCfg->Timer.wAllData!=0)
	{
		if((m_allsendtime>=m_pBaseCfg->Timer.wAllData*60)&&((m_allcycleflag&0x80)==0))
		{
			if(m_callallflag)
			{
				m_allsendtime=0;
				return;
			}
			if(m_guiyuepara.mode==1)
			{
				event_time=1;
				commCtrl(m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
			}
				
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
}

//通道空闲事件处理
void CGB101S::DoCommIdle()
{
	m_bReceiveControl = 0;
	if(m_yxchangeflag)
	{
		DWORD event_time = 2;
		commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);//lqh 

	}
	/*else
		{
		if (TestFlag(m_wTaskID, m_wEqpID, SSOEUFLAG))
			m_yxchangeflag=1;
	}*/
		return ;
		
}

//通道发送空闲处理
void CGB101S::DoCommSendIdle(void)
{
	DWORD event_time = 0;
	commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_OFF, (BYTE*)&event_time);//lqh 
	if(m_guiyuepara.mode==1)
		m_PRM =1;
	else if(m_recfalg!=3)   //非平衡式
		return;
	if(m_recfalg)
	{
		if(m_recfalg==2)
			m_recfalg=0;
		if(m_initflag)
		{
			m_allcalloverflag=0;
			Initlink();
			return ; 
		}
					
		m_acdflag=1;
					
		if(m_callallflag&&m_initallflag)
		{
			if(SendCallAll())
			{
				m_initallflag=0;
				return ;
			} 
		}
						
		//if(m_allcalloverflag==0) return;

		if(m_YKflag)
		{
			if(m_YKflag==1)
			{
				m_YKflag++;
				if( SendYKSetAck())
					return;
			}
						
			m_YKflag=0;
					
			if(SendYKstop())
					return ; 
		}		

		if(m_YTflag)
		{
			if(m_YTflag==1)
			{
				m_YTflag++;
				if( SendYTSetAck())
					return;
			}
						
			m_YTflag=0;
					
			if(SendYTstop())
				return ; 
		}		

		if(m_delayflag)
		{
			m_delayflag=0;
			if(SenddelayAck())
				return ; 
		}
		if(m_readddflag)
		{
			if(SendCallAllDD())
				return ;  
			m_readddflag=0;
		}

		if(m_readparaflag==1)
		{
			if(Sendpara())
				return;
			m_readparaflag=0;
		}
		m_acdflag=0;
	
		if(m_timeflag)
		{
			m_timeflag=0;
			if(SendtimeAck())
				return ; 
		}
		if(m_readtimeflag)
		{
		 	m_readtimeflag = 0;
			if(SendtimeReq())
				return ;
		}
		if(m_guiyuepara.tmp[0] & SWITCHCOS_BIT)
		{
			searchcos();
			if(m_yxchangeflag)
			{
				m_acdflag=0;
				if(SendCos())
					return ; 
				if(SendDCos())
					return ; 
				m_acdflag=0;
			}
		}
		if (CheckClearTaskFlag(TASKFLAG_SSOEUFLAG))
		{
			for (WORD wSoeEqpNo = 0; wSoeEqpNo < m_wEqpNum; wSoeEqpNo++)
		    {
		    	SwitchToEqpNo(wSoeEqpNo);
	  			if (CheckClearEqpFlag(EQPFLAG_SSOEUFLAG))
	  			{
	  				if(SendSoe())  
						return;
		    	}		
		    }
		}			
		if(SendDSoe())
			return ; 
		m_SOEflag=0;
		

		if(SendFaultEvent())
			return;

		if(m_callallflag)
		{
			if(SendCallAll())
				return ; 
			m_initallflag=0;
		}
		if(m_groupflag)
		{
			if(SendCallgroup())
				return ; 
		}
		if(m_testflag)
		{
			m_testflag=0;
			if(SendTsetLinkAck())
				return ; 
		}
		if(m_resetflag)
		{
			m_resetflag=0;
			if(SendresetAck())
				return ; 
		}
		
        /************************文件传输部分****************/
		if(m_calldir)
		{
			if(SendFileDir())
				return;	
		}

		if(m_byReadFileAckFlag)
		{			
			m_byReadFileAckFlag = 0;
			if(SendReadFileAck())
				return;	
		}
		if(m_byReadFileFlag)
		{
			if(SendReadFileData())
				return;
		}

		if(m_byWriteFileAckFlag || m_byWriteFileDataAckFlag)
		{
			SendWriteFileAck();
			m_byWriteFileAckFlag = 0;
			m_byWriteFileDataAckFlag = 0;
			return;
		}
		/************************文件传输部分 END************/


		/************************电能量召唤*****************/
		if(m_byCallEnergyFlag||m_dwCallDDFlag||m_dwfiffrzflag||m_dwrealdataflag||m_dwdayfrzflag)
		{
			if(SendAllEnergy())
				return;
		}

		if(g_ddfifflag)   //  15min冻结突变
		{
			if(SendChangeFifFrz())
				return;
		}
		
		if(g_dddayflag)   //  日冻结突变
		{
			if(SendChangeDayFrz())
				return;
		}

		if(g_ddtideflag)   //  潮流突变
		{
			if(SendChangeTide())
				return;
		}
		

		
		/***************电能量召唤END**********************/
		
		/************************参数设置部分***********************/
		if(m_bySwitchValNoFlag)	   //切换定值区
		{
			m_bySwitchValNoFlag = 0;
			if(SendSwitchValNoAck())
				return;
		}

		if(m_byReadValNoFlag)      //读定值区
		{
			m_byReadValNoFlag = 0;
			if(SendReadValNoAck())
				return;
		}

		if(m_byAllParaValFlag || m_byMoreParaValFlag) //读参数
		{
			if(SendReadParaVal())
				return;
		}

		if(m_byWriteParaStatus)    //写参数
		{
			m_byWriteParaStatus = 0;
			if(SendWriteParaValAck())
				return;
		}
		if(m_errflag==1)
		{
			SenderrtypeAck();
			return;	
		}
		else if(m_errflag==2)
		{
			Senderrtype4Ack(44);
			return ;
		}
		else if(m_errflag==5)
		{
			Senderrtype4Ack(45);
			return ;
		}		
		else if(m_errflag==4)
		{
			Senderrtype4Ack(m_dwasdu.COT+1);
			return ;
		}	
		else if(m_errflag==16)
		{
			Senderrtype4Ack(46);
			return;
		}
		else if(m_errflag==7)
		{
			Senderrtype4Ack(47);
			return ;
		}

		/***********************参数设置部分END*********************/

		/****************软件升级*****************/
		if(m_bySoftUpgradeStatus)
		{
			if(SendSoftUpgradeAck())
				return;
		}
		/****************软件升级END***************/
		
					
	}
	if((m_zdflag==0)&&(m_linkflag))
	{
		if(m_YKflag && m_guiyuepara.mode)
		{
			if(m_YKflag==1)
			{
				m_YKflag++;
				if( SendYKSetAck())
					return;
			}		
			m_YKflag=0;	
			if(SendYKstop())
					return ; 
		}

		if(m_YTflag && m_guiyuepara.mode)
		{
			if(m_YTflag==1)
			{
				m_YTflag++;
				if( SendYTSetAck())
					return;
			}	
			m_YTflag=0;
			if(SendYTstop())
				return ; 
		}
            
		if(m_guiyuepara.tmp[0] & SWITCHCOS_BIT) //cos
		{
			searchcos();
			if(m_yxchangeflag)
			{
				m_acdflag=0;
				if(SendCos())
					return ; 
				if(SendDCos())
					return ; 
				m_acdflag=0;
				m_yxchangeflag=0;
			}
		}
		searchsoe();
		if(m_SOEflag)
		{
			if(SendSoe())
				return ; 
			if(SendDSoe())
				return ; 
			m_SOEflag=0;
		}
		if(m_callallflag)
		{
			if(SendCallAll())
				return ; 
			m_initallflag=0;
		}		
		if(m_ycchangeflag>1)
		{
			if(SendChangeYC())
			{
				m_ycchangeflag=0;
				return;
			}
		}

		if(SendFaultEvent())
			return;
#if(DEV_SP == DEV_SP_TTU)		
		if(m_allcycleflag&0x80)
		{
				if(CycleSendAllYcYx(m_allcycleflag&3))
				return;
		}
#endif
		m_zdflag=0;

	}
	if(m_recfalg)
	{
		m_recfalg=0;
		//SendNoData();	
	}

	event_time = 50;
	commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);//lqh 
}


//处理总召唤
BOOL CGB101S::SendCallAll(void)
{
	if(m_callallflag&0x80)
	{
		m_callallflag&=0x7f;
		RecCallAllCommand();
		m_wSendYcNum=0;
		return TRUE;
	}
	if (GetEqpFlag(CALL_YXGRP1))
	{
		if (SendDYXGroup(0,20))
			return TRUE;
		if (SendYXGroup(0,20))
			return TRUE;
		m_wSendYxNum=0;
		m_wSendDYxNum=0;
		ClearEqpFlag(CALL_YXGRP1);
	}
	if (GetEqpFlag(CALL_YXGRP2))
	{
		if (SendDYXGroup(1,20))
			return TRUE;
		if (SendYXGroup(1,20))
			return TRUE;
		m_wSendYxNum=0;
		m_wSendDYxNum=0;
		ClearEqpFlag(CALL_YXGRP2);
	}
	if (GetEqpFlag(CALL_YXGRP3))
	{
		if (SendDYXGroup(2,20))
			return TRUE;
		if (SendYXGroup(2,20))
			return TRUE;
		m_wSendYxNum=0;
		m_wSendDYxNum=0;
		ClearEqpFlag(CALL_YXGRP3);
	}
	if (GetEqpFlag(CALL_YXGRP4))
	{
		if (SendDYXGroup(3,20))
			return TRUE;
		if (SendYXGroup(3,20))
			return TRUE;
		m_wSendYxNum=0;
		m_wSendDYxNum=0;
		ClearEqpFlag(CALL_YXGRP4);
	}
	if (GetEqpFlag(CALL_YXGRP5))
	{
		if (SendDYXGroup(4,20))
			return TRUE;
		if (SendYXGroup(4,20))
			return TRUE;
		m_wSendYxNum=0;
		m_wSendDYxNum=0;
		ClearEqpFlag(CALL_YXGRP5);
	}
	if (GetEqpFlag(CALL_YXGRP6))
	{
		if (SendDYXGroup(5,20))
			return TRUE;
		if (SendYXGroup(5,20))
			return TRUE;
		m_wSendYxNum=0;
		m_wSendDYxNum=0;
		ClearEqpFlag(CALL_YXGRP6);
	}
	if (GetEqpFlag(CALL_YXGRP7))
	{
		if (SendDYXGroup(6,20))
			return TRUE;
		if (SendYXGroup(6,20))
			return TRUE;
		m_wSendYxNum=0;
		m_wSendDYxNum=0;
		ClearEqpFlag(CALL_YXGRP7);
	}
	if (GetEqpFlag(CALL_YXGRP8))
	{
		if (SendDYXGroup(7,20))
			return TRUE;
		if (SendYXGroup(7,20))
			return TRUE;
		m_wSendYxNum=0;
		m_wSendDYxNum=0;
		ClearEqpFlag(CALL_YXGRP8);
	}

	if (GetEqpFlag(CALL_YCGRP1))
	{
		if (SendYCGroup(0,20))
			return TRUE;
		ClearEqpFlag(CALL_YCGRP1);
		m_wSendYcNum=0;
	}
	if (GetEqpFlag(CALL_YCGRP2))
	{
		if (SendYCGroup(1,20))
			return TRUE;
		ClearEqpFlag(CALL_YCGRP2);
		m_wSendYcNum=0;
	}
	if (GetEqpFlag(CALL_YCGRP3))
	{
		if (SendYCGroup(2,20))
			return TRUE;
		ClearEqpFlag(CALL_YCGRP3);
		m_wSendYcNum=0;
	}
	if (GetEqpFlag(CALL_YCGRP4))
	{
		if (SendYCGroup(3,20))
			return TRUE;
		ClearEqpFlag(CALL_YCGRP4);
		m_wSendYcNum=0;
	}

	if (CheckClearEqpFlag(CALL_ALLSTOP))	
	{
		m_callallflag=0;
		m_acdflag=0;
		SearchChangeYC(100, MAX_PUBBUF_LEN *4, (VDBYCF_L*)m_dwPubBuf,0);

		if (SendAllStop())
			return TRUE; 
	}

	return FALSE;
}
BOOL CGB101S::SendCallgroup(void)
{
	if(m_groupflag&0x80)
	{
		m_groupflag&=0x7f;
		RecCallAllCommand();
		return TRUE;
	}
	if (GetEqpFlag(CALL_YCGRP1))
	{
		if (SendYCGroup(0,29))
			return TRUE;
		else
			ClearEqpFlag(CALL_YCGRP1);
		m_wSendYcNum=0;
	}
	if (GetEqpFlag(CALL_YCGRP2))
	{
		if (SendYCGroup(1,30))
			return TRUE;
		else
			ClearEqpFlag(CALL_YCGRP2);
		m_wSendYcNum=0;
	}
	if (GetEqpFlag(CALL_YCGRP3))
	{
		if (SendYCGroup(2,31))
			return TRUE;
		else
			ClearEqpFlag(CALL_YCGRP3);
	}
	if (GetEqpFlag(CALL_YCGRP4))
	{
		if (SendYCGroup(3,32))
			return TRUE;
		else
			ClearEqpFlag(CALL_YCGRP4);
	}
	if (GetEqpFlag(CALL_YXGRP1))
	{
		if (SendDYXGroup(0,21))
			return TRUE;
		if (SendYXGroup(0,21))
			return TRUE;
		ClearEqpFlag(CALL_YXGRP1);
		m_wSendDYxNum=0;
		m_wSendYxNum=0;
	}
	if (GetEqpFlag(CALL_YXGRP2))
	{
		if (SendDYXGroup(1,22))
			return TRUE;
		if (SendYXGroup(1,22))
			return TRUE;
			ClearEqpFlag(CALL_YXGRP2);
		m_wSendDYxNum=0;
		m_wSendYxNum=0;
	}
	if (GetEqpFlag(CALL_YXGRP3))
	{
		if (SendDYXGroup(2,23))
			return TRUE;
		if (SendYXGroup(2,23))
			return TRUE;
		ClearEqpFlag(CALL_YXGRP3);
		m_wSendDYxNum=0;
		m_wSendYxNum=0;
	}
	if (GetEqpFlag(CALL_YXGRP4))
	{
		if (SendDYXGroup(3,24))
			return TRUE;
		if (SendYXGroup(3,24))
			return TRUE;
		ClearEqpFlag(CALL_YXGRP4);
		m_wSendDYxNum=0;
		m_wSendYxNum=0;
	}
	if (GetEqpFlag(CALL_YXGRP5))
	{
		if (SendDYXGroup(4,25))
			return TRUE;
		if (SendYXGroup(4,25))
			return TRUE;
		ClearEqpFlag(CALL_YXGRP5);
		m_wSendDYxNum=0;
		m_wSendYxNum=0;
	}
	if (GetEqpFlag(CALL_YXGRP6))
	{
		if (SendDYXGroup(5,26))
			return TRUE;
		if (SendYXGroup(5,26))
			return TRUE;
		ClearEqpFlag(CALL_YXGRP6);
		m_wSendDYxNum=0;
		m_wSendYxNum=0;
	}
	if (GetEqpFlag(CALL_YXGRP7))
	{
		if (SendDYXGroup(6,27))
			return TRUE;
		if (SendYXGroup(6,27))
			return TRUE;
		ClearEqpFlag(CALL_YXGRP7);
		m_wSendDYxNum=0;
		m_wSendYxNum=0;
	}
	if (GetEqpFlag(CALL_YXGRP8))
	{
		if (SendDYXGroup(7,28))
			return TRUE;
		if (SendYXGroup(7,28))
			return TRUE;
		ClearEqpFlag(CALL_YXGRP8);
		m_wSendDYxNum=0;
		m_wSendYxNum=0;
	}

	if (CheckClearEqpFlag(CALL_ALLSTOP))	
	{
		m_groupflag=0;
		if (SendAllStop())
			return TRUE; 
	}

	return FALSE;
}

//处理总召唤
BOOL CGB101S::SendCallAllDD(void)
{
	if(m_readddflag&0x80)
	{
		m_readddflag&=0x7f;
		SendCallAllDDAck();
		return TRUE;
	}
	if (CheckClearEqpFlag(CALL_DDGRP1))	
		if (SendAllDDGroup(0))
			return TRUE;
	if (CheckClearEqpFlag(CALL_DDGRP2))	
		if (SendAllDDGroup(1))
			return TRUE;
	if (CheckClearEqpFlag(CALL_DDGRP3))	
		if (SendAllDDGroup(2))
			return TRUE;
	if (CheckClearEqpFlag(CALL_DDGRP4))	
		if (SendAllDDGroup(3))
			return TRUE;
	m_acdflag=0;
	if (CheckClearEqpFlag(CALL_ALLDDSTOP)) 
		if (SendAllDDStop())
			return TRUE;

	return FALSE;
	
}

//处理平衡模式
BOOL CGB101S::SendBalance(void)
{
#if 0
	if (SearchClass1())
	{
		if (SendClass1()) //发送1级数据
			return TRUE;
	}
	
	//if (CheckClearEqpFlag(CALL_LINK)) 
	//	return SendReqLink();   

	if (SendCallAll())
		return TRUE;
	if (SendCallAllDD())
		return TRUE;
   
	SendClass2(MODE_BALANCE);
	#endif
	return TRUE;
}


//测试链路
BOOL CGB101S::SendtimeAck(void)
{
	BYTE Style = 0x67, Reason = COT_ACTCON;
	BYTE PRM = 0, dwCode = 3, Num = 1;
	m_acdflag=0;
	SendFrameHead(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(delayclock.wMSecond+delayclock.bySecond*1000)&0xff;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(delayclock.wMSecond+delayclock.bySecond*1000)>>8;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=delayclock.byMinute;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=delayclock.byHour;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=delayclock.byDay|(delayclock.byWeek<<5);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=delayclock.byMonth;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=delayclock.wYear%100;
	if(m_dwasdu.LinkAddr==GetAddress())
		SendFrameTail(PRM, dwCode, Num);
	
	return TRUE;
}
BOOL CGB101S::SendtimeReq(void)
{
	BYTE Style = 0x67, Reason = COT_REQ;
	BYTE PRM = 0, dwCode = 3, Num = 1;
	 struct VSysClock Reqclock;
	 GetSysClock((void *)&Reqclock, SYSCLOCK);
	m_acdflag=0;
	SendFrameHead(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(Reqclock.wMSecond+Reqclock.bySecond*1000)&0xff;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(Reqclock.wMSecond+Reqclock.bySecond*1000)>>8;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=Reqclock.byMinute;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=Reqclock.byHour;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=Reqclock.byDay|(Reqclock.byWeek<<5);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=Reqclock.byMonth;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=Reqclock.wYear%100;
	if(m_dwasdu.LinkAddr==GetAddress())
		SendFrameTail(PRM, dwCode, Num);
	return TRUE;
}
BOOL CGB101S::SendTsetLinkAck(void)
{
	BYTE Style = 0x68, Reason = COT_ACTCON;
	BYTE PRM = 0, dwCode =3, Num = 1;

	SendFrameHead(Style, Reason);
	write_infoadd(0);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0xAA; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0x55; 
	SendFrameTail(PRM, dwCode, Num);
	
	return TRUE;
}
BOOL CGB101S::SenddelayAck(void)
{
	BYTE Style = 0x6a, Reason = COT_ACTCON;
	BYTE PRM = 0, dwCode =3, Num = 1;
	struct VSysClock clock;	
	GetSysClock(&clock,SYSCLOCK);
	WORD tdata=clock.bySecond;
	tdata*=1000;
	tdata+=clock.wMSecond;
	SendFrameHead(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = tdata; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = tdata>>8; 
	
	SendFrameTail(PRM, dwCode, Num);
	
	return TRUE;
}
BOOL CGB101S::SendsetparaAck(void)
{
	BYTE PRM = 0, dwCode =3;
	m_SendBuf.wReadPtr = 0;
	m_SendBuf.wWritePtr=0;
	if(m_tdata[0]!=0x68)
	{
		m_errflag=0;
		return false;
	}
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_tdata,m_tdata[1]+4);
	m_SendBuf.wWritePtr+=m_tdata[1]+4;
	if(m_dwasdu.COT==COT_ACT)
	{
		m_tdata[m_dwasdu.COToff+5]=COT_ACTTERM;
		m_dwasdu.COT=COT_ACTTERM;
	}
	else
	{
		m_tdata[0]=0;
		m_errflag=0;
	}
	SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);
	return TRUE;
}
BOOL CGB101S::SendresetAck(void)
{
	BYTE Style = 105, Reason = COT_ACTCON;
	BYTE PRM = 0, dwCode =3, Num = 1;

	SendFrameHead(Style, Reason);
	write_infoadd(0);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_QRP; 
	SendFrameTail(PRM, dwCode, Num);
	if(m_QRP==1)
	{
		thSleep(30);
		sysRestart(COLDRESTART, SYS_RESET_REMOTE, GetThName(m_wTaskID));
	}
	return TRUE;
}
//设置参数
BOOL CGB101S::SendSetParaAck(void)
{
	BYTE Style = 0x6E;
	BYTE PRM = 0, dwCode = 3, Num = 1;
	BYTE * pData = pReceiveFrame->Frame68.Data+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;
	if((m_dwasdu.Info<ADDR_YCPARA_LO)||(m_dwasdu.Info>ADDR_YCPARA_HI))
		m_dwasdu.COT|=0x40;
	SendFrameHead(Style, m_dwasdu.COT+1);
	write_infoadd(m_dwasdu.Info);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pData[0]; //信息体地址
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pData[1]; //信息体地址Hi
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pData[2]; //Para Value
	SendFrameTail(PRM,dwCode,Num);
	
	return TRUE;
}
BOOL CGB101S::initover(void)
{
	BYTE Style = 70, Reason = 4;
	BYTE PRM = 0, dwCode = 8, Num = 1;
	SendFrameHead(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = g_Sys.ResetInfo.code; //信息体地址Hi
	SendFrameTail(PRM,dwCode,Num);
	
	return TRUE;
}

#if 0
BOOL CGB101S::SendYCGroup(WORD GroupNo, BYTE Reason)
{
//	BYTE Style = m_guiyuepara.yctype;//M_ME_NA;
	BYTE PRM = 0;
	BYTE dwCode = 3;
	BYTE VSQ=0x80;
	WORD YCNo, YCSendNum;
	WORD ReadYCNum = 64;
		struct VYCF_L *YCValue;
	YCNo = GroupNo * GRP_YCNUM;
	YCNo+=m_wSendYcNum;
	if ((YCNo >= m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum)||(YCNo>=(GroupNo+1) * GRP_YCNUM))
	{
		return FALSE;
	}
		
	SendFrameHead(m_guiyuepara.yctype, Reason);
	if((m_guiyuepara.yctype==M_ME_ND)||(m_guiyuepara.yctype==M_ME_NA)||(m_guiyuepara.yctype==M_ME_NB))
		write_infoadd(YCNo + ADDR_YC_LO);
	
	for (YCSendNum = 0; (m_SendBuf.wWritePtr<230)&&(YCSendNum < GRP_YCNUM) && (YCNo < m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum);)
	{
		ReadYCNum=ReadRangeYCF_L(m_wEqpID, YCNo, 1, sizeof(VYCF_L),(struct VYCF_L *) m_dwPubBuf);
		if(ReadYCNum==0) break;

		#ifdef BEIJING_TEST
			YCValue <<= 4;	//for beijing test
		#endif

		YCValue=(struct VYCF_L *) m_dwPubBuf;
		//for(int i=0;i<ReadYCNum;i++)
		if((m_guiyuepara.yctype==10)||(m_guiyuepara.yctype==12)||(m_guiyuepara.yctype==34)||(m_guiyuepara.yctype==35))
			write_infoadd(YCNo + ADDR_YC_LO);

		{
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCValue->lValue); 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCValue->lValue); 
			if(YCValue->lValue>65536)
			{
				YCValue->byFlag|=0x18;
				YCValue->byFlag&=0xfe;
			}
			if(m_guiyuepara.yctype!=21)
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = QDS(YCValue->byFlag);//QDS
				m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[YCNo]=false;
		}
		if((m_guiyuepara.yctype==10)||(m_guiyuepara.yctype==12))
			write_time3();
		if((m_guiyuepara.yctype==34)||(m_guiyuepara.yctype==35))
			write_time();
		YCSendNum+=ReadYCNum;
		YCNo+=ReadYCNum;
	}
	m_wSendYcNum+= YCSendNum;
	if((m_guiyuepara.yctype==35)||(m_guiyuepara.yctype==34)||(m_guiyuepara.yctype==10)||(m_guiyuepara.yctype==12))
		VSQ=0;
	SendFrameTail(PRM,dwCode, YCSendNum | VSQ);//SET ACD
	
	//if ((FRM_YCNUM < GRP_YCNUM) && (m_wSendYcNum != 0) && (YCNo < m_pEqpInfo[m_wEqpNo].wYCNum))
	//{
	//	thSleep(8);
		//SendYCGroupContinue(GroupNo, Reason);
	//	m_wSendYcNum = 0;
	//}
	/*else
	{
		return TRUE;
	}*/
	return TRUE;
}
#endif
//按组上送遥测全部调整到不带时标
BOOL CGB101S::SendYCGroup(WORD GroupNo, BYTE Reason)
{
	BYTE Style = m_guiyuepara.yctype; //M_ME_NA;
	BYTE PRM = 0;
	BYTE dwCode = 3;
	BYTE VSQ=0x80;
	WORD YcNo;
	WORD ReadYcNum = 64;
	struct VYCF_L *YcValue;
	WORD wSendNum,wSendNum1;
	float fd;
	
	YcNo = GroupNo * GRP_YCNUM;
	YcNo+=m_wSendYcNum;
	if ((YcNo >= m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum)||(YcNo>=(GroupNo+1) * GRP_YCNUM))
	{
		return FALSE;
	}

	switch(Style)
	{
		case M_ME_TA:
		case M_ME_TD:
			Style = M_ME_NA;
			break;
		case M_ME_TB:
		case M_ME_TE:
			Style = M_ME_NB;
			break;
		case M_ME_TC:
		case M_ME_TF:
			Style = M_ME_NC;
			break; 
	}
	ReadYCSendNo(m_wEqpID,YcNo,&wSendNum);
	wSendNum1 = wSendNum;
	
	SendFrameHead(Style, Reason);

	if((Style==M_ME_NA)||(Style==M_ME_NB)||(Style==M_ME_NC)||(Style==M_ME_ND))
	//write_infoadd(YcNo + ADDR_YC_LO);
	write_infoadd(wSendNum+ ADDR_YC_LO);
	
	WORD i;
	ReadYcNum=	ReadRangeYCF_L(m_wEqpID, YcNo, ReadYcNum, ReadYcNum*sizeof(struct VYCF_L ), (struct VYCF_L *)m_dwPubBuf);
	YcValue=(struct VYCF_L *)m_dwPubBuf;
	for( i=0; (m_SendBuf.wWritePtr<230) &&(i+m_wSendYcNum<GRP_YCNUM) && (i<ReadYcNum) && (YcNo+i<m_pEqpInfo[m_wEqpNo].wYCNum+m_pEqpInfo[m_wEqpNo].wVYCNum); i++)
	{
		ReadYCSendNo(m_wEqpID,YcNo+i,&wSendNum);
		if((wSendNum - wSendNum1) <= 1)
			wSendNum1 = wSendNum;
		else
			break;
		if((m_guiyuepara.yctype==35)||(m_guiyuepara.yctype==10)||(m_guiyuepara.yctype==12)||(m_guiyuepara.yctype==34))
			write_infoadd(YcNo +i+ ADDR_YC_LO);
		if(m_guiyuepara.yctype!=13)
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(YcValue[i].lValue);
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(YcValue[i].lValue);
			if(YcValue[i].lValue>65536)
			{
				YcValue[i].byFlag|=0x18;
				YcValue[i].byFlag&=0xfe;
			}
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

				YCLongToFloat(m_wEqpID,YcNo+ i,YcValue[i].lValue,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
				m_SendBuf.wWritePtr+=4;
			}
		}

		if(m_guiyuepara.yctype!=M_ME_ND)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = QDS(YcValue[i].byFlag);//QDP
		if(COT_INTROGEN == Reason)
		{
		  m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i+YcNo]=false;
		  m_pEqpExtInfo[m_wEqpNo].OldYC[i+YcNo].lValue = m_pEqpExtInfo[m_wEqpNo].CurYC[i+YcNo].lValue = YcValue[i].lValue;
		  m_pEqpExtInfo[m_wEqpNo].OldYC[i+YcNo].byFlag = m_pEqpExtInfo[m_wEqpNo].CurYC[YcNo+i].byFlag = YcValue[i].byFlag;
		}
	}
	
	m_wSendYcNum+= i;
	SendFrameTail(PRM,dwCode, i | VSQ);//SET ACD
	
	//if ((FRM_YCNUM < GRP_YCNUM) && (m_wSendYcNum != 0) && (YCNo < m_pEqpInfo[m_wEqpNo].wYCNum))
	//{
	//	thSleep(8);
		//SendYCGroupContinue(GroupNo, Reason);
	//	m_wSendYcNum = 0;
	//}
	/*else
	{
		return TRUE;
	}*/
	return TRUE;
}


//继续发送一组遥测
BOOL CGB101S::SendYCGroupContinue(WORD GroupNo, BYTE Reason)
{
	BYTE Style = M_ME_NA;
	BYTE PRM = 0;
	BYTE dwCode = 8;
	WORD YCNo, YCSendNum, YCValue;
	WORD ReadYCNum =1;

	YCNo = GroupNo * GRP_YCNUM + m_wSendYcNum;
	if (YCNo >= m_pEqpInfo[m_wEqpNo].wYCNum)
		return FALSE;
	
	SendFrameHead(Style, Reason);

	write_infoadd(YCNo + ADDR_YC_LO);
	for (YCSendNum = 0; (YCSendNum < (GRP_YCNUM - m_wSendYcNum)) && (YCNo < m_pEqpInfo[m_wEqpNo].wYCNum); YCNo++, YCSendNum++)
	{
		ReadRangeYC(m_wEqpID, YCNo, ReadYCNum, 2/*sizeof(WORD)*/, (short *)&YCValue);
		#ifdef BEIJING_TEST
			YCValue <<= 4;	//for beijing test
		#endif
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCValue); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCValue); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;//QDS
	}
	m_wSendYcNum = 0;
	SendFrameTail(PRM,dwCode, YCSendNum | 0x80);
	return TRUE;	
}
#if 0
//发送一组遥测	OK
BOOL CGB101S::SendYCGroup(WORD GroupNo, BYTE Reason)
{
	BYTE Style = M_ME_ND;
	BYTE PRM = 0;
	BYTE dwCode = 8;
	WORD YCNo, YCSendNum, YCValue;
	WORD ReadYCNum = 1;

	YCNo = GroupNo * GRP_YCNUM;
	if (YCNo >= m_pEqpInfo[m_wEqpNo].wYCNum)
		return FALSE;
	
	SendFrameHead(Style, Reason);

	switch (m_pVIec101Cfg->byInfoAdr )
	{
		case 1:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCNo + ADDR_YC_LO);
			break;
		case 2:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCNo + ADDR_YC_LO); //信息体地址Lo
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCNo + ADDR_YC_LO); //信息体地址Hi
			break;
		case 3:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCNo + ADDR_YC_LO); //信息体地址Lo
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCNo + ADDR_YC_LO);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
			break;
		default:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCNo + ADDR_YC_LO); //信息体地址Lo
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCNo + ADDR_YC_LO); //信息体地址Hi
			break;
	}
	
	for (YCSendNum = 0; (YCSendNum < FRM_YCNUM) && (YCNo < m_pEqpInfo[m_wEqpNo].wYCNum); YCNo++, YCSendNum++)
	{
		ReadRangeYC(m_wEqpID, YCNo, ReadYCNum, 2/*sizeof(WORD)*/, (short *)&YCValue);
		#ifdef BEIJING_TEST
			YCValue <<= 4;	//for beijing test
		#endif
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCValue); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCValue); 
		
	}
	m_wSendYcNum = YCSendNum;
	SendFrameTail(PRM,dwCode, YCSendNum | 0x80);
	
	if ((FRM_YCNUM < GRP_YCNUM) && (m_wSendYcNum != 0) && (YCNo < m_pEqpInfo[m_wEqpNo].wYCNum))
	{
		thSleep(8);
		SendYCGroupContinue(GroupNo, Reason);
		m_wSendYcNum = 0;
	}
	/*else
	{
		return TRUE;
	}*/
	return TRUE;
}


//继续发送一组遥测
BOOL CGB101S::SendYCGroupContinue(WORD GroupNo, BYTE Reason)
{
	BYTE Style = M_ME_ND;
	BYTE PRM = 0;
	BYTE dwCode = 8;
	WORD YCNo, YCSendNum, YCValue;
	WORD ReadYCNum =1;

	YCNo = GroupNo * GRP_YCNUM + m_wSendYcNum;
	if (YCNo >= m_pEqpInfo[m_wEqpNo].wYCNum)
		return FALSE;
	
	SendFrameHead(Style, Reason);

	switch (m_pVIec101Cfg->byInfoAdr )
	{
		case 1:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCNo + ADDR_YC_LO);
			break;
		case 2:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCNo + ADDR_YC_LO); //信息体地址Lo
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCNo + ADDR_YC_LO); //信息体地址Hi
			break;
		case 3:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCNo + ADDR_YC_LO); //信息体地址Lo
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCNo + ADDR_YC_LO);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
			break;
		default:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCNo + ADDR_YC_LO); //信息体地址Lo
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCNo + ADDR_YC_LO); //信息体地址Hi
			break;
	}
	
	for (YCSendNum = 0; (YCSendNum < (GRP_YCNUM - m_wSendYcNum)) && (YCNo < m_pEqpInfo[m_wEqpNo].wYCNum); YCNo++, YCSendNum++)
	{
		ReadRangeYC(m_wEqpID, YCNo, ReadYCNum, 2/*sizeof(WORD)*/, (short *)&YCValue);
		#ifdef BEIJING_TEST
			YCValue <<= 4;	//for beijing test
		#endif
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCValue); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCValue); 
		
	}
	m_wSendYcNum = 0;
	SendFrameTail(PRM,dwCode, YCSendNum | 0x80);
	return TRUE;	
}
#endif


#if 0 
//发送一组遥信
BOOL CGB101S::SendYXGroup(WORD GroupNo, BYTE Reason)
{
	BYTE Style = 1;
	BYTE PRM = 0, dwCode = 3;
	WORD YXNo;
	WORD ReadYXNum = 1;
	BYTE YXValue;
	BYTE VSQ=0x80;
	YXNo = GroupNo * GRP_YXNUM;
	YXNo+=m_wSendYxNum;
	if((m_wSendYxNum+m_wSendDYxNum>=GRP_YXNUM)|| (YXNo >= m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum))
	{
		return FALSE;
	}

	if(m_guiyuepara.yxtype==1)
		Style=1;
	if(m_guiyuepara.yxtype==2)
		Style=2;
	if(m_guiyuepara.yxtype==3)
		Style=30;

	SendFrameHead(Style, Reason);

#if 0
	switch (m_pVIec101Cfg->byInfoAdr )
	{
		case 1:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YXNo + ADDR_YX_LO);
			break;
		case 2:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YXNo + ADDR_YX_LO); //信息体地址Lo
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YXNo + ADDR_YX_LO); //信息体地址Hi
			break;
		case 3:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YXNo + ADDR_YX_LO); //信息体地址Lo
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YXNo + ADDR_YX_LO);
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
			break;
		default:
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YXNo + ADDR_YX_LO); //信息体地址Lo
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YXNo + ADDR_YX_LO); //信息体地址Hi
			break;
	}
#endif
		if(m_guiyuepara.yxtype==1)
	write_infoadd(YXNo + ADDR_YX_LO+m_wSendDYxNum);
	WORD i;
	for(i=0;(m_SendBuf.wWritePtr<200) &&(i+m_wSendYxNum+m_wSendDYxNum<GRP_YXNUM)&& (i<FRM_YXNUM) && (YXNo<m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum); i++, YXNo++)
	{
		if((m_guiyuepara.yxtype==2)||(m_guiyuepara.yxtype==3))
	write_infoadd(YXNo + ADDR_YX_LO+m_wSendDYxNum);
		ReadRangeSYX(m_wEqpID, YXNo, ReadYXNum, 1/*sizeof(BYTE)*/, &YXValue);
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr ] = SIQ(YXValue );
		m_SendBuf.wWritePtr++;
		if(m_guiyuepara.yxtype==3)
			write_time();
		if(m_guiyuepara.yxtype==2)
			write_time3();
	}
	m_wSendYxNum+= i;
	if(m_guiyuepara.yxtype!=1)
		VSQ=0;
	SendFrameTail(PRM, dwCode, i | VSQ);
#if 0
	if ((FRM_YXNUM < GRP_YXNUM) && (m_wSendYxNum != 0) && (YXNo < m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum))
	{
		thSleep(8);
		#if(TYPE_USER != USER_GDTEST)
		SendYXGroupContinue(GroupNo, Reason);
		#endif
		m_wSendYxNum = 0;
	}
	else
	{
		return TRUE;
	}
#endif
	return TRUE;
}
#endif

//发送一组遥信
//old 方式:通过设置可选总召唤上送类型
//修改目标:总召唤只上送不带时标信息
BOOL CGB101S::SendYXGroup(WORD GroupNo, BYTE Reason)
{
	BYTE Style = M_SP_NA;
	BYTE PRM = 0, dwCode = 3;
	WORD YXNo;
	WORD ReadYXNum = 1;
	BYTE YXValue,sendyxnum = 0;
	BYTE VSQ=0x80;
	WORD wSendNum,wSendNum1;
	
	YXNo = GroupNo * GRP_YXNUM;
	YXNo+=m_wSendYxNum;
	if((m_wSendYxNum+m_wSendDYxNum>=GRP_YXNUM)|| (YXNo >= m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum))
	{
		return FALSE;
	}

	if(m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum>256)
		return SendYXM_PS_NA( GroupNo,  Reason);
	if(m_guiyuepara.yxtype==3) 
		Style = M_DP_NA;        //短时标单点遥信
	SendFrameHead(Style, Reason);
	VDBCOS Cos;
	WORD RecCosNum;
	while(1)
	{
		RecCosNum = ReadSCOS(m_wTaskID, m_wEqpID, 1,sizeof(VDBCOS), (VDBCOS *)&Cos);
		if (RecCosNum == 0)
			break;

		ReadPtrIncrease(m_wTaskID, m_wEqpID, RecCosNum, SCOSUFLAG);
	}

	WORD i;

	ReadSYXSendNo(m_wEqpID,YXNo,&wSendNum);
	wSendNum1 = wSendNum - 1;
	if(m_guiyuepara.tmp[3] == 0)
		write_infoadd(wSendNum + ADDR_YX_LO);
	for(i=0;(m_SendBuf.wWritePtr<200) &&(i+m_wSendYxNum+m_wSendDYxNum<GRP_YXNUM)&& (i<FRM_YXNUM) && (YXNo<m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum); i++, YXNo++)
	{
		ReadSYXSendNo(m_wEqpID,YXNo,&wSendNum);
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

		
		if(ReadRangeSYX(m_wEqpID, YXNo, ReadYXNum, 1/*sizeof(BYTE)*/, &YXValue) != ReadYXNum)  continue;
		
		if(m_guiyuepara.yxtype==3) 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr ] = SIQ(YXValue )+1;
		else
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr ] = SIQ(YXValue );
		m_SendBuf.wWritePtr++;
		sendyxnum ++;
	}
	m_wSendYxNum+= i;
	
	SendFrameTail(PRM, dwCode,sendyxnum  | VSQ);

	return TRUE;
}
BOOL CGB101S::SendYXM_PS_NA(WORD GroupNo, BYTE Reason)
{
	BYTE Style = M_PS_NA;
	BYTE PRM = 0, dwCode = 3;
	WORD YXNo;
	WORD ReadYXNum = m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum;
	BYTE VSQ=0x80;
	BYTE Bitstate[256];
	YXNo = GroupNo*736;
	YXNo+=m_wSendYxNum;
	
	ReadYXNum = ReadRangeSYXBit(m_wEqpID, YXNo, ReadYXNum, 256,Bitstate);
	if( (YXNo >= m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum) || (ReadYXNum == 0))
	{
		return FALSE;
	}
	if (YXNo >= 2048)
	{
		return FALSE;
	}

	SendFrameHead(Style, Reason);

	write_infoadd(YXNo + ADDR_YX_LO+m_wSendDYxNum);

	WORD i;
	
	//BYTE YXSendNum;
	//VDBCOS Cos;
	//WORD RecCosNum;
	//WORD ReadNum = 1;//每次读取1个COS
			
	for(i=0;i<=(ReadYXNum-1)/16; i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] =Bitstate[i*2+m_wSendYxNum/16];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] =Bitstate[i*2+1+m_wSendYxNum/16];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] =Bitchange[i*2+m_wSendYxNum/16];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++] =Bitchange[i*2+1+m_wSendYxNum/16];
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
		if(i>45) break;
	}
	m_wSendYxNum+= ReadYXNum;
	memset(Bitchange,0,256);
	
	SendFrameTail(PRM, dwCode, i| VSQ);

	return TRUE;
}

#if 1
//发送一组双点遥信
BOOL CGB101S::SendDYXGroup(WORD GroupNo, BYTE Reason)
{
	BYTE Style = 1;
	BYTE PRM = 0, dwCode = 3;
	BYTE VSQ=0x80;
	WORD YXNo, YXSendNum;
	WORD ReadYXNum = 1;
	struct VDYX *YXValue;
	WORD wSendNum,wSendNum1;
	
	YXNo = GroupNo * GRP_YXNUM;
	YXNo+=m_wSendDYxNum;
	if((m_wSendDYxNum>=GRP_YXNUM)|| (YXNo >= m_pEqpInfo[m_wEqpNo].wDYXNum))
	{
		return FALSE;
	}
	if(m_guiyuepara.yxtype==1)
		Style=3;
	if(m_guiyuepara.yxtype==2)
		Style=4;
	if(m_guiyuepara.yxtype==3)
		Style=3;

	ReadDYXSendNo( m_wEqpID, YXNo, & wSendNum);
	wSendNum1 = wSendNum - 1;
	
	SendFrameHead(Style, Reason);
	if((m_guiyuepara.yxtype==1)&&(m_guiyuepara.tmp[3] == 0))
		write_infoadd(wSendNum + ADDR_YX_LO);
	for (YXSendNum = 0;(m_SendBuf.wWritePtr<200)&& (YXSendNum < FRM_YXNUM) && (YXNo < m_pEqpInfo[m_wEqpNo].wDYXNum); YXNo++, YXSendNum++)
	{
		ReadDYXSendNo(m_wEqpID,YXNo,&wSendNum);
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

	
		ReadRangeDYX(m_wEqpID, YXNo, ReadYXNum, sizeof(struct VDYX), (struct VDYX*)m_dwPubBuf);
		if((m_guiyuepara.yxtype==2)||(m_guiyuepara.yxtype==3))
			write_infoadd(YXNo + ADDR_YX_LO);
		YXValue=(struct VDYX*)m_dwPubBuf;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = DIQ(YXValue->byValue1,YXValue->byValue2);
//		if(m_guiyuepara.yxtype==3)
//			write_time();
		if(m_guiyuepara.yxtype==2)
			write_time3();
	}
	m_wSendDYxNum +=YXSendNum;
	if(m_guiyuepara.yxtype!=1)
		VSQ=0;
	SendFrameTail(PRM, dwCode, YXSendNum | VSQ);

	return TRUE;
}
#endif

//发送一组双点遥信
//old 方式:通过设置可选总召唤上送类型
//修改目标:总召唤只上送不带时标信息
#if 0
BOOL CGB101S::SendDYXGroup(WORD GroupNo, BYTE Reason)
{
	BYTE Style = M_DP_NA;
	BYTE PRM = 0, dwCode = 3;
	BYTE VSQ=0x80;
	WORD YXNo, YXSendNum;
	WORD ReadYXNum = 1;
	struct VDYX *YXValue;
	YXNo = GroupNo * GRP_YXNUM;
	YXNo+=m_wSendDYxNum;
	if((m_wSendDYxNum>=GRP_YXNUM)|| (YXNo >= m_pEqpInfo[m_wEqpNo].wDYXNum))
	{
		return FALSE;
	}
	
	SendFrameHead(Style, Reason);
		VDBDCOS Cos;
	WORD RecCosNum;
	while(1)
	{
		RecCosNum = ReadDCOS(m_wTaskID, m_wEqpID, 1,
								sizeof(VDBDCOS), (VDBDCOS *)&Cos);
		if (RecCosNum == 0)
			break;
		ReadPtrIncrease(m_wTaskID, m_wEqpID, RecCosNum, DCOSUFLAG);
		}

	write_infoadd(YXNo + ADDR_YX_LO);

	for (YXSendNum = 0;(m_SendBuf.wWritePtr<200)&& (YXSendNum < FRM_YXNUM) && (YXNo < m_pEqpInfo[m_wEqpNo].wDYXNum); YXNo++, YXSendNum++)
	{
		ReadRangeDYX(m_wEqpID, YXNo, ReadYXNum, sizeof(struct VDYX), (struct VDYX*)m_dwPubBuf);
		YXValue=(struct VDYX*)m_dwPubBuf;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = DIQ(YXValue->byValue1,YXValue->byValue2);
	}
	m_wSendDYxNum +=YXSendNum;

	SendFrameTail(PRM, dwCode, YXSendNum | VSQ);

	return TRUE;
}

#endif

//继续发送一组遥信
BOOL CGB101S::SendYXGroupContinue(WORD GroupNo, BYTE Reason)
{
	BYTE Style = 1;
	BYTE PRM = 0, dwCode = 8;
	WORD YXNo, YXSendNum;
	WORD ReadYXNum = 1;
	BYTE YXValue;

	YXNo = GroupNo * GRP_YXNUM + m_wSendYxNum;
	if (YXNo >= m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum)//sfq
		return FALSE;
	
	SendFrameHead(Style, Reason);

	write_infoadd(YXNo + ADDR_YX_LO);
	for (YXSendNum = 0; (YXSendNum < (GRP_YXNUM - m_wSendYxNum)) && (YXNo < m_pEqpInfo[m_wEqpNo].wSYXNum+m_pEqpInfo[m_wEqpNo].wVYXNum); YXNo++, YXSendNum++)
	{
		if(ReadRangeSYX(m_wEqpID, YXNo, ReadYXNum, 1/*sizeof(BYTE)*/, &YXValue) != ReadYXNum) break;
		YXValue = YXValue & 0x80;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = YXValue ? 1 : 0;
	}
	m_wSendYxNum = 0;
	SendFrameTail(PRM, dwCode, YXSendNum | 0x80);

	return TRUE;	
}



//发送总召唤结束帧
BOOL CGB101S::SendAllStop(void)
{
	BYTE Style = 0x64;
	BYTE Reason = 0x0A;
	BYTE PRM = 0;
	BYTE dwCode = 3;
	BYTE Num = 1;
	m_acdflag=0;

	SendFrameHead(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_ztype; 
	SendFrameTail(PRM, dwCode, Num);
	m_allcalloverflag=1;
	return TRUE;
}
BOOL CGB101S::SendgroupStop(void)
{
	BYTE Style = 0x64;
	BYTE Reason = 0x0A;
	BYTE PRM = 0;
	BYTE dwCode = 8;
	BYTE Num = 1;

	SendFrameHead(Style, Reason);

	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_ztype; 
	SendFrameTail(PRM, dwCode, Num);

	m_byRTUStatus = RTU_CALLEND;
	#if (TYPE_USER == USER_GDTEST)
	m_bySendStatus = SEND_NULL;
	#endif
	return TRUE;
}


//召唤所有电度的确认
BOOL CGB101S::SendAllDDStop(void)
{
	BYTE Style = 101, Reason = 0x0A;
	BYTE PRM = 0, dwCode = 3, Num = 1;

	SendFrameHead(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_readddflag; 
	SendFrameTail(PRM, dwCode, Num);
	return TRUE;
}


//发送一组总召唤电度
BOOL CGB101S::SendAllDDGroup(WORD GroupNo)
{
	//thSleep(1000);//for cangzhou temp
	BYTE reason;
	if(m_readddflag&0x40)
		reason=3;
	else
		reason=37+(m_readddflag&0xf);
	if(reason>41) reason=37;
	return SendDDGroup(GroupNo, 8, reason);
}

//发送一组电度
BOOL CGB101S::SendDDGroup(WORD GroupNo,BYTE dwCode,BYTE Reason)
{
	
	BYTE Style = 0x0F;
	BYTE PRM = 0;
	DWORD DDNo, DDSendNum, DDValue;
	WORD ReadDDNum = 1;

	DDNo = GroupNo * GRP_DDNUM;
	if (DDNo >= m_pEqpInfo[m_wEqpNo].wDDNum)
		return FALSE;
	if(m_guiyuepara.ddtype==37)
		Style=37;
	SendFrameHead(Style, Reason);

	for (DDSendNum = 0; (m_SendBuf.wWritePtr<230)&&(DDSendNum < FRM_DDNUM) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum); DDNo++,DDSendNum++)
	{
		
		write_infoadd(DDNo + ADDR_DD_LO);
		ReadRangeDD(m_wEqpID, DDNo, ReadDDNum, 4/*sizeof(DWORD)*/, (long *)&DDValue);
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(LOWORD(DDValue)); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(LOWORD(DDValue)); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(HIWORD(DDValue)); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(HIWORD(DDValue)); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = DDSendNum + 1; //顺序号
		if(Style==37)
			write_time();
	}
	m_wSendDdNum = DDSendNum;
	SendFrameTail(PRM, dwCode, DDSendNum);

	if ((FRM_DDNUM < GRP_DDNUM) && (m_wSendDdNum != 0) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum))
	{
		thSleep(8);
		SendDDGroupContinue(GroupNo, dwCode, Reason);
		m_wSendYcNum = 0;
	}
	
	return TRUE;
}

//继续发送一组电度
BOOL CGB101S::SendDDGroupContinue(WORD GroupNo,BYTE dwCode,BYTE Reason)
{
	
	BYTE Style = 0x0F;
	BYTE PRM = 0;
	DWORD DDNo, DDSendNum, DDValue;
	WORD ReadDDNum = 1;

	DDNo = GroupNo * GRP_DDNUM + m_wSendDdNum;
	if (DDNo >= m_pEqpInfo[m_wEqpNo].wDDNum)
		return FALSE;
	
	SendFrameHead(Style, Reason);

	for (DDSendNum = 0; (DDSendNum < (GRP_DDNUM - m_wSendDdNum)) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum); DDNo++,DDSendNum++)
	{
		
		write_infoadd(DDNo + ADDR_DD_LO);
		ReadRangeDD(m_wEqpID, DDNo, ReadDDNum, 4/*sizeof(DWORD)*/, (long *)&DDValue);
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(LOWORD(DDValue)); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(LOWORD(DDValue)); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(HIWORD(DDValue)); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(HIWORD(DDValue)); 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = DDSendNum + 1; //顺序号
	}
	m_wSendDdNum = 0;
	SendFrameTail(PRM, dwCode, DDSendNum);
	
	return TRUE;
}

BOOL CGB101S::searchsoe(void) 
{
	WORD RecSoeNum;
	WORD ReadNum = 20;
	struct VDBDSOE SOEInfo;

	if(m_WritePtr<m_ReadPtr)
		RecSoeNum=m_WritePtr+m_BufLen-m_ReadPtr;
	else
		RecSoeNum=m_WritePtr-m_ReadPtr;
	if(ReadNum>RecSoeNum)
		ReadNum=RecSoeNum;
    RecSoeNum = QuerySSOE(m_wEqpID, ReadNum, m_ReadPtr, 1000,(VDBSOE *)m_dwPubBuf, &m_WritePtr, &m_BufLen);
	if (RecSoeNum)                                              
	{
		m_SOEflag=1;
	}

	ReadPtrIncrease(m_wTaskID, m_wEqpID, RecSoeNum, SSOEUFLAG);
	RecSoeNum = ReadDSOE(m_wTaskID, m_wEqpID, 1, sizeof(VDBDSOE), &SOEInfo);
	if (RecSoeNum)
	{
		m_SOEflag=1;
	}
         ReadPtrIncrease(m_wTaskID, m_wEqpID, RecSoeNum, DSOEUFLAG);

	return m_SOEflag;
}


//发送单点SOE数据
BOOL CGB101S::SendSoe(void) 
{
	BYTE Style = M_SP_TB, Reason = COT_SPONT;
	BYTE PRM = 0, dwCode = 3;
	VDBSOE *pRecSOE = NULL;
	WORD RecSoeNum;
	WORD ReadNum = 20;
	VSysClock SysTime;

	ClearTaskFlag(TASKFLAG_SSOEUFLAG);
	ClearEqpFlag(EQPFLAG_SSOEUFLAG);
	if(m_guiyuepara.yxtype==2) 
		Style = M_SP_TA;        //短时标单点遥信
	if(m_guiyuepara.yxtype==3) 
		Style = M_DP_TB;        //短时标单点遥信

	SendFrameHead(Style, Reason);
	if(m_WritePtr<m_ReadPtr)
		RecSoeNum=m_WritePtr+m_BufLen-m_ReadPtr;
	else
		RecSoeNum=m_WritePtr-m_ReadPtr;
	if(ReadNum>RecSoeNum)
		ReadNum=RecSoeNum;
    RecSoeNum = QuerySSOE(m_wEqpID, ReadNum, m_ReadPtr, 1000,(VDBSOE *)m_dwPubBuf, &m_WritePtr, &m_BufLen);
	if (RecSoeNum == 0)                                              
	    return FALSE;
	m_ReadPtr+=RecSoeNum;
	m_ReadPtr%=m_BufLen;
	pRecSOE = (VDBSOE *)m_dwPubBuf; 
	for (int i=0; i < RecSoeNum; i++)  
	{    
		write_infoadd(pRecSOE->wNo + ADDR_YX_LO);
		if(m_guiyuepara.yxtype==3)
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SIQ(pRecSOE->byValue )+1;
		else
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(pRecSOE->byValue);
		SystemClock(pRecSOE->Time.dwMinute, &SysTime);                      
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(pRecSOE->Time.wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(pRecSOE->Time.wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMinute;   
		if(Style != 2)//短时标单点遥信 
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byHour & 0x1F;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SysTime.byDay & 0x1F) | ((SysTime.byWeek <<5) & 0xE0);    
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMonth;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.wYear % 100;
		}
		pRecSOE ++;      
	
	}         
	
	if (RecSoeNum == 0)
		return FALSE;
	SetTaskFlag(TASKFLAG_SENDSSOE);                              
	SetEqpFlag(EQPFLAG_SENDSSOE);                                
	ClearUFlag(m_wEqpNo);

	SendFrameTail(PRM, dwCode, RecSoeNum);
	
	return TRUE;
}


//发送双点SOE数据
BOOL CGB101S::SendDSoe(void) 
{
	
	BYTE Style = M_DP_TB, Reason = COT_SPONT;
	BYTE PRM = 0, dwCode = 3;
	DWORD SoeSendNum;
	VDBDSOE SOEInfo;
	WORD RecSoeNum=0;
	WORD ReadNum = 1;
	VSysClock SysTime;

	if(m_guiyuepara.yxtype==2) 
		Style = M_DP_TA;
	
	SendFrameHead(Style, Reason);

	for (SoeSendNum = 0; (SoeSendNum < GRP_SOENUM) ; SoeSendNum++)
	{
		ReadPtrIncrease(m_wTaskID, m_wEqpID, RecSoeNum, DSOEUFLAG);
		RecSoeNum = ReadDSOE(m_wTaskID, m_wEqpID, ReadNum, sizeof(VDBDSOE), &SOEInfo);
		if (RecSoeNum == 0)
			break;
	
		write_infoadd(SOEInfo.wNo + ADDR_YX_LO);
		
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = DIQ(SOEInfo.byValue1,SOEInfo.byValue2);
		
		SystemClock(SOEInfo.Time.dwMinute, &SysTime);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(SOEInfo.Time.wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(SOEInfo.Time.wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMinute;   
			
		if(Style!=M_DP_TA) 
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byHour & 0x1F;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (SysTime.byDay & 0x1F) | ((SysTime.byWeek <<5) & 0xE0);    
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMonth;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.wYear % 100;
		}	
				
	}
	if (SoeSendNum == 0)
		return FALSE;

	SendFrameTail(PRM, dwCode, SoeSendNum);
	
	return TRUE;
}
#if 0
WORD CGB101S::SearchChangeYC(WORD wNum, WORD wBufLen, VDBYCF *pBuf)
{
    WORD wAbsVal;
	int i,j,k,nChangeVal,nChangeVal1,nChangeVal2;
    VYCF *pCurYC;
	VDBYCF *pSendYC;
	VYCF *pVal;
#ifdef _GUANGZHOU_TEST_VER_	
	long maxval;
#endif
//	Vlimitpara* plimitpara=(Vlimitpara*)((BYTE*)limitpara+limitpara[3]);
    if (m_pEqpInfo[m_wEqpNo].wYCNum==0)  return(0);
	if (wBufLen<sizeof(VDBYCF))  return(0);
	
	for (i=0; i<m_pEqpInfo[m_wEqpNo].wYCNum; i++)
	{
        m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=FALSE;
	}    
	
	::ReadRangeYCF(m_wEqpID, 0, m_pEqpInfo[m_wEqpNo].wYCNum, m_pEqpInfo[m_wEqpNo].wYCNum*sizeof(VYCF) , m_pEqpExtInfo[m_wEqpNo].CurYC);

    for (i=0; i<m_pEqpInfo[m_wEqpNo].wYCNum; i++)
    {         
		pCurYC=m_pEqpExtInfo[m_wEqpNo].CurYC+i;
		if ( (pCurYC->byFlag & 0x80)) continue;     //active send disable
		
		pVal=m_pEqpExtInfo[m_wEqpNo].OldYC+i;
		#if 1
		nChangeVal1=(pVal->nValue)*m_pBaseCfg->wYCDeadValue/1000;
		if (nChangeVal1==0) nChangeVal1=1;
		if (nChangeVal1<0) nChangeVal1=0-nChangeVal1;
		nChangeVal2=pCurYC->nValue*m_pBaseCfg->wYCDeadValue/1000;
		if (nChangeVal2==0) nChangeVal2=1;
		if (nChangeVal2<0) nChangeVal2=0-nChangeVal2;

		if (nChangeVal1<nChangeVal2)
			nChangeVal=nChangeVal1;
		else
			nChangeVal=nChangeVal2;
			#endif
			#if 0
			if(plimitpara[i].limitchange==0)
				nChangeVal=m_pBaseCfg->wYCDeadValue;
			else 
				nChangeVal=plimitpara[i].limitchange;
			#endif
		#ifdef _GUANGZHOU_TEST_VER_
		//nChangeVal = m_pBaseCfg->wYCDeadValue;
		 if(GetYC_ABC(m_wEqpID,i,NULL,&maxval,NULL)==OK)
			{
			if(maxval>50)
			nChangeVal=maxval*m_pBaseCfg->wYCDeadValue/1000;
		}
		#endif			
			if(nChangeVal<5) nChangeVal=5;
		if(pCurYC->nValue > pVal->nValue)
		    wAbsVal = pCurYC->nValue - pVal->nValue;
		else 
			wAbsVal = pVal->nValue - pCurYC->nValue;
		
		if(wAbsVal >= nChangeVal)   m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=TRUE;
		/*if((pCurYC->nValue>plimitpara[i].limitmax)&&(pVal->nValue<plimitpara[i].limitmax))
			m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=TRUE;
		if((pCurYC->nValue<plimitpara[i].limitmax)&&(pVal->nValue>plimitpara[i].limitmax))
			m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=TRUE;
		if((pCurYC->nValue>plimitpara[i].limitmmin)&&(pVal->nValue<plimitpara[i].limitmmin))
			m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=TRUE;
		if((pCurYC->nValue<plimitpara[i].limitmmin)&&(pVal->nValue>plimitpara[i].limitmmin))
			m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=TRUE;*/
    }	
	
	pSendYC=(VDBYCF *)pBuf; 
	i=j=k=0; 	
	while ((j<wNum)&&(i< m_pEqpInfo[m_wEqpNo].wYCNum))
	{
	    if (m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[m_pEqpExtInfo[m_wEqpNo].wChangYCNo]==TRUE)
	    {
           pSendYC->wNo=m_pEqpExtInfo[m_wEqpNo].wChangYCNo;
		   pSendYC->nValue=m_pEqpExtInfo[m_wEqpNo].CurYC[m_pEqpExtInfo[m_wEqpNo].wChangYCNo].nValue;
		   pSendYC->byFlag=m_pEqpExtInfo[m_wEqpNo].CurYC[m_pEqpExtInfo[m_wEqpNo].wChangYCNo].byFlag;
		   m_pEqpExtInfo[m_wEqpNo].OldYC[m_pEqpExtInfo[m_wEqpNo].wChangYCNo].nValue=pSendYC->nValue;
		   m_pEqpExtInfo[m_wEqpNo].OldYC[m_pEqpExtInfo[m_wEqpNo].wChangYCNo].byFlag=pSendYC->byFlag;
		   pSendYC++;
		   j++;
		   k+=sizeof(VDBYCF);
    	   if (k>wBufLen-sizeof(VDBYCF)) break;
	    }
        m_pEqpExtInfo[m_wEqpNo].wChangYCNo++;
		if (m_pEqpExtInfo[m_wEqpNo].wChangYCNo >= m_pEqpInfo[m_wEqpNo].wYCNum)
		   m_pEqpExtInfo[m_wEqpNo].wChangYCNo = 0; 		
		i++;
	}  
	
	return j;
}

#endif
//发送变化遥测数据
BOOL CGB101S::SendChangeYC(void) 
{
	BYTE  Reason = COT_SPONT;
	BYTE PRM = 0, dwCode = 8;
	WORD ChangeYCNum = 0;
	VDBYCF_L *pBuf = (VDBYCF_L *)m_dwPubBuf;
	WORD wReqNum = 10;
	int i;
	WORD wSendNum;
	float fd;
	DWORD dddd;
	
	ChangeYCNum = SearchChangeYC(wReqNum, MAX_PUBBUF_LEN *4, (VDBYCF_L *)pBuf,0);
	if (!ChangeYCNum)
	{	
		return false;
	}
	SendFrameHead(m_guiyuepara.yctype, Reason);
	m_qds++;
	for ( i=0;( i < ChangeYCNum)&&(i<25); i++)
	{
		ReadYCSendNo(m_wEqpID, pBuf->wNo,&wSendNum);
		write_infoadd(wSendNum + ADDR_YC_LO);
		//write_infoadd(pBuf->wNo + ADDR_YC_LO);
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
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = QDS(pBuf->byFlag);//QDS
		if((m_guiyuepara.yctype==10)||(m_guiyuepara.yctype==12))
			write_time3();
		if((m_guiyuepara.yctype==34)||(m_guiyuepara.yctype==35))
			write_time();
		pBuf++;
	}	

	SendFrameTail(PRM,dwCode, i );//SET ACD

	return TRUE;
}

#if 0

//发送SOE数据
BOOL CGB101S::SendSoe(void) 
{
	
	BYTE Style = M_SP_TA, Reason = COT_SPONT;
	BYTE PRM = 0, dwCode = 8;
	DWORD SoeSendNum;
	VDBSOE SOEInfo;
	DWORD YXNo;
	DWORD MSecond;

	WORD RecSoeNum;
	WORD ReadNum = 1;
	BYTE YXValue;
	VSysClock SysTime;

	#if(TYPE_USER == USER_GUANGZHOU)
	PRM = 1;
	#endif
	
	SendFrameHead(Style, Reason);

	for (SoeSendNum = 0; (SoeSendNum < GRP_SOENUM) && TestFlag(m_wTaskID, m_wEqpID, SSOEUFLAG); SoeSendNum++)
	{
		RecSoeNum = ReadSSOE(m_wTaskID, m_wEqpID, ReadNum, sizeof(VDBSOE), &SOEInfo);
		if (RecSoeNum == 0)
			break;
		ReadPtrIncrease(m_wTaskID, m_wEqpID, RecSoeNum, SSOEUFLAG);
		
		YXNo = SOEInfo.wNo + ADDR_YX_LO;
		YXValue = SOEInfo.byValue & 0x80;
		switch (m_pVIec101Cfg->byInfoAdr )
		{
			case 1:
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YXNo);
				break;
			case 2:
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YXNo); //信息体地址Lo
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YXNo); //信息体地址Hi
				break;
			case 3:
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YXNo); //信息体地址Lo
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YXNo);
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; //信息体地址Hi
				break;
			default:
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YXNo); //信息体地址Lo
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YXNo); //信息体地址Hi
				break;
		}
		
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = YXValue ? 1 : 0;	
		
		SystemClock(SOEInfo.Time.dwMinute, &SysTime);
		MSecond = SOEInfo.Time.wMSecond;
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(MSecond);	
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(MSecond);	
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SysTime.byMinute;  
				
	}
	if (SoeSendNum == 0)
		return FALSE;

	SendFrameTail(PRM, dwCode, SoeSendNum);
	
	return TRUE;
}


//发送变化遥测数据
BOOL CGB101S::SendChangeYC(void) 
{
	
	BYTE Style = M_ME_ND, Reason = COT_SPONT;
	BYTE PRM = 0, dwCode = 8;
	WORD ChangeYCNum = 0;
	VDBYCF *pBuf = (VDBYCF *)m_dwPubBuf;
	WORD wReqNum = 40;//GRP_YCNUM/2;
	ChangeYCNum = SearchChangeYC(wReqNum, MAX_PUBBUF_LEN *4, pBuf,1);
	SendFrameHead(Style, Reason);
	for (int i=0; i < ChangeYCNum; i++)
	{
		switch (m_pVIec101Cfg->byInfoAdr )
		{
			case 1:
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(pBuf->wNo + ADDR_YC_LO);
				break;
			case 2:
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(pBuf->wNo + ADDR_YC_LO); //信息体地址Lo
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(pBuf->wNo + ADDR_YC_LO); //信息体地址Hi
				break;
			case 3:
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(pBuf->wNo + ADDR_YC_LO); //信息体地址Lo
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(pBuf->wNo + ADDR_YC_LO);
					m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] =0;
				break;
			default:
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(pBuf->wNo + ADDR_YC_LO); //信息体地址Lo
				m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(pBuf->wNo + ADDR_YC_LO); //信息体地址Hi
				break;
		}
		
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(pBuf->nValue);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(pBuf->nValue);
		pBuf++;
	}	
	
	if (ChangeYCNum == 0)
		return FALSE;
	SendFrameTail(PRM, dwCode, ChangeYCNum);
	
	return TRUE;
}
#endif
//获取控制域
BYTE CGB101S::GetCtrCode(BYTE PRM,BYTE dwCode,BYTE fcv)
{
	BYTE CodeTmp = 0x00;
	dwCode&=0xf;
	CodeTmp += dwCode;
	#ifdef _GUANGZHOU_TEST_VER_
	PRM=0;
	#endif
	if(m_guiyuepara.mode==0)
	{
		m_dir_flag=0;
	}
	CodeTmp |= 	m_dir_flag;
	if (PRM)
		CodeTmp |= 0x40;
	if(m_guiyuepara.mode==0)
	{
		if (SearchClass1()&&(m_linkflag)&&(CodeTmp!=0xb))
		{
			CodeTmp |= 0x20;
		}
		m_acdflag=0;
	}
	else
	{
 		if(fcv)
		{
			CodeTmp|=(m_fcb|0x10);
			if(m_fcb==0)
				m_fcb=0x20;
			else
				m_fcb=0;
		}
	}
	return CodeTmp;
}

//发送固定帧格式基本报文
BOOL CGB101S::SendBaseFrame(BYTE PRM,BYTE dwCode)
{
	WORD wLinkAddress;
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	WORD len;
#endif

	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
	
	pSendFrame = (VIec101Frame *)m_SendBuf.pBuf;
	if(PRM==0)//添加e5命令
	{
		if((dwCode==9))
		{
			if(m_guiyuepara.mode==0)
			{
				if(SearchClass1()==false)
				{
					m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0xe5;

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
					if(3== m_guiyuepara.jiami)
					{
						len = SecuritySendFrame(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,0);
						if(len<1)
						{
							return FALSE;
						}
						else
						{
							m_SendBuf.wWritePtr  = len;
						}
					}
					if(4== m_guiyuepara.jiami)
					{//短帧的typid写为0，都不需要加密
						len = SecuritySendFrame_HN(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,0,0);
						if(len<1)
						{
							return FALSE;
						}
						else
						{
							m_SendBuf.wWritePtr  = len;
						}
					}
#endif
					if(m_dwasdu.LinkAddr==GetAddress())	
						WriteToComm(m_dwasdu.LinkAddr);

					return TRUE;

				}
							
			}
		}

	}
	pSendFrame->Frame10.Start = 0x10;  
	pSendFrame->Frame10.Control = GetCtrCode(PRM,dwCode,0);			
	write_10linkaddr(GetAddress());
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)ChkSum((BYTE *)&pSendFrame->Frame10.Control,m_guiyuepara.linkaddrlen+1);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	wLinkAddress = m_dwasdu.LinkAddr;
	if(pSendFrame->Frame10.Control &0x40)
		m_recfalg=0;

	m_SendBuf.wReadPtr = 0;

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if(3 == m_guiyuepara.jiami)
	{
		len = SecuritySendFrame(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,0);
		if(len<1)
		{
			return FALSE;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
	}
	if(4 == m_guiyuepara.jiami)
	{//ly
		len = SecuritySendFrame_HN(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,0,0);
		if(len<1)
		{
			return FALSE;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
	}
#endif

	if(m_dwasdu.LinkAddr==GetAddress())	
		WriteToComm(wLinkAddress);

	return TRUE;
}

BOOL CGB101S::SendLinktesetFrame(BYTE PRM,BYTE dwCode)
{
	WORD wLinkAddress;
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
	
	pSendFrame = (VIec101Frame *)m_SendBuf.pBuf;
	
	pSendFrame->Frame10.Start = 0x10;  
	pSendFrame->Frame10.Control = GetCtrCode(PRM,dwCode,1);			
	write_10linkaddr(GetAddress());
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)ChkSum((BYTE *)&pSendFrame->Frame10.Control,m_guiyuepara.linkaddrlen+1);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	wLinkAddress = m_dwasdu.LinkAddr;
	if(pSendFrame->Frame10.Control &0x40)
		m_recfalg=0;

	m_SendBuf.wReadPtr = 0;
	if(m_dwasdu.LinkAddr==GetAddress())	
		WriteToComm(wLinkAddress);

	return TRUE;
}


//复位远方链路
BOOL CGB101S::SendResetLink(BYTE PRM)
{ 
	return SendBaseFrame(PRM,0x00); 
}

//请求远方链路状态
BOOL CGB101S::SendReqLink(void)
{ 
	return SendBaseFrame(1, 0x09); 
}


//链路完好
BOOL CGB101S::SendReqLinkAck(void)
{ 	
	return SendBaseFrame(0,0x0b); 
}

//检查有无1级数据
BOOL CGB101S::SearchClass1(void)		
{
	//RecCosNum =TestFlag(m_wTaskID, m_wEqpID, SCOSUFLAG);
	if(m_guiyuepara.tmp[0] & SWITCHCOS_BIT)
	{
		searchcos();
	}
	searchsoe();  // COS 改为 SOE 上送
	#if 0
	if(m_guiyuepara.mode==1)
		{
	if(m_initflag)
		return TRUE;
	if (GetCosNum(m_wEqpID))
		return TRUE;
	if (GetEqpFlag(CALL_DATA))
		return TRUE;
	if(m_acdflag) 
		return TRUE;

	}
	else
	#endif
		
		
		if(m_initflag)
		return TRUE;
	if (m_YKflag)
		return TRUE;
	if (m_YTflag)
		return TRUE;
	//if (m_yxchangeflag)
		//return TRUE;
	if(m_SOEflag)
		return TRUE;
	if (m_callallflag)
		return TRUE;
	if (m_resetflag)
		return TRUE;	
	if(m_timeflag)
		return TRUE;
	if(m_readtimeflag)
		return TRUE;
	if(m_byCallEnergyFlag||m_dwCallDDFlag||m_dwfiffrzflag||m_dwrealdataflag)
		return TRUE;
	if(m_bySwitchValNoFlag)	   //切换定值区
		return TRUE;

	if(m_byReadValNoFlag)      //读定值区
		return TRUE;

	if(m_byAllParaValFlag || m_byMoreParaValFlag) //读参数
		return TRUE;

	if(m_byWriteParaStatus)    //写参数
		return TRUE;

	return FALSE;
}

//发送1级数据
BOOL CGB101S::SendClass1(void) 
{
	if(m_guiyuepara.mode==1)
	{
		SendAck();
	}
	
	if (SendCos())
		return TRUE;

	return false;
	
}

//发送2级数据
BOOL CGB101S::SendClass2(BYTE byMode) 
{
	#if (TYPE_USER == USER_GDTEST)
	
	/*if (m_bySendStatus == SEND_YK)
	{if(m_ykdelaystop++>10)
		{
		m_bySendStatus = SEND_NULL;
		return SendYKstop();//确认
		}
	}*/
	#endif
	#if (TYPE_USER == USER_GDTEST)
	if (m_bySendStatus == SEND_TIME)
	{
		m_bySendStatus = SEND_NULL;
		SendSetClockAck();//对钟的确认
		return TRUE;
		
	}
	#endif
	#ifndef BALANCE_MODE
	if (SearchClass1())//若有1级数据则以无所要求数据帧应答
		return SendNoData();
	#endif
	if (TestFlag(m_wTaskID, m_wEqpID, SSOEUFLAG))
		if(SendSoe())
			return TRUE;

	if (CheckClearEqpFlag(EQPFLAG_ROUTE))
	{
		SendTSData();
		return TRUE;
	}
	//广州DTU平衡101 模式下禁止变化遥测上送
	#if (TYPE_USER != USER_GUANGZHOU)	
	if (SendChangeYC())
		return TRUE;
	#endif
	//无数据，以E5回答
	if (byMode == MODE_UNBALANCE)
	{
		/*m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0xE5;
		WriteToComm(GetAddress());*/
		SendNoData();
	}
	return TRUE;
}

//召唤某组电度
BOOL CGB101S::SendSomeGroupDD(void)
{
	WORD GroupNo;
	BYTE * pData = pReceiveFrame->Frame68.Data;
	GroupNo = pData[2] & 0x0F - 1;
	SendSomeGroupDDData(GroupNo);
	return TRUE;
}

//召唤所有电度的确认
BOOL CGB101S::SendCallAllDDAck(void)
{
	BYTE Style = 0x65, Reason = 7;
	BYTE PRM = 0,dwCode = 3,Num = 1;

	SendFrameHead(Style, Reason);
	write_infoadd(m_dwasdu.Info);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_readddflag; 
	SendFrameTail(PRM, dwCode, Num);
	return TRUE;
}

//发送一组分组召唤电度
BOOL CGB101S::SendSomeGroupDDData(WORD GroupNo)
{
	if(SendDDGroup(GroupNo, 0, GroupNo + 38) == FALSE)
	{
		//无数据，以E5回答
		m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0xE5;
		WriteToComm(pReceiveFrame->Frame10.Data[m_byLinkAdrShift]);
		return TRUE;
	}
	return SendDDGroup(GroupNo, 0, GroupNo + 38);
}

//对钟的确认
BOOL CGB101S::SendSetClockAck(void)
{
	#if (TYPE_USER == USER_GDTEST)
	BYTE Style = 0x67, Reason = 7;
	BYTE PRM = 0, dwCode = 8, Num = 1;
	
	SendFrameHead(Style, Reason);

	write_infoadd(0);
	BYTE * pData = &pReceiveFrame->Frame68.Data[m_byInfoShift];
	pData += 2;
	for (WORD i=0; i<7; i++)
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pData[i]; 
	SendFrameTail(PRM, dwCode, Num);
	#else
	SetSysClock((void *)&m_SysTime, SYSCLOCK);
	
	BYTE Style = 0x67, Reason = 7;
	BYTE PRM = 0, dwCode = 8, Num = 1;
	
	SendFrameHead(Style, Reason);

	write_infoadd(0);
	BYTE * pData = &pReceiveFrame->Frame68.Data[m_byInfoShift];
	pData += 2;
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(m_SysTime.bySecond*1000 + m_SysTime.wMSecond);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(m_SysTime.bySecond*1000 + m_SysTime.wMSecond);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_SysTime.byMinute; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_SysTime.byHour; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = (m_SysTime.byDay & 0x1F) | (m_SysTime.byWeek << 5); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_SysTime.byMonth; 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_SysTime.wYear -m_guiyuepara.baseyear; 
		
	SendFrameTail(PRM, dwCode, Num);
	#endif
	return TRUE;
}

//处理总召唤激活命令
BOOL CGB101S::SendCallAllStartAck(void)
{
	BYTE PRM = 0, dwCode = 3,Num = 1;
	
	SendFrameHead(0x64, 0x07);

	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_ztype; 
	SendFrameTail(PRM, dwCode, Num);
	
	return TRUE;
}
BOOL CGB101S::searchcos(void) 
{

	VDBCOS Cos;
	VDBDCOS DCos;
	WORD RecCosNum;
	m_yxchangeflag=0;
	RecCosNum = ReadSCOS(m_wTaskID, m_wEqpID, 1,sizeof(VDBCOS), (VDBCOS *)&Cos);
	if(RecCosNum)
	{
		m_yxchangeflag=1;
	}
	RecCosNum = ReadDCOS(m_wTaskID, m_wEqpID, 1,sizeof(VDBDCOS), (VDBDCOS *)&DCos);
	if(RecCosNum)
	{
		m_yxchangeflag=1;
	}
	return m_yxchangeflag;
}

//发送变化遥信
BOOL CGB101S::SendCos(void) 
{
	BYTE Style = M_SP_NA;
	BYTE Reason = COT_SPONT;
	BYTE PRM = 0;
	BYTE dwCode = 3;
	BYTE YXSendNum,YXStatus;
	WORD YXNo;
	VDBCOS Cos;
	WORD RecCosNum;
	WORD ReadNum = 1;//每次读取1个COS

	#if (TYPE_USER == USER_GUANGZHOU)
	PRM = 1;
	#endif
	if(m_guiyuepara.yxtype==3) 
		Style = M_DP_NA;        //短时标单点遥信

	SendFrameHead(Style, Reason);

	for(YXSendNum = 0; YXSendNum < GRP_YXNUM / 2 ; YXSendNum++)
	{
		RecCosNum = ReadSCOS(m_wTaskID, m_wEqpID, ReadNum,sizeof(VDBCOS), (VDBCOS *)&Cos);
		if (RecCosNum == 0)
			break;

		ReadPtrIncrease(m_wTaskID, m_wEqpID, RecCosNum, SCOSUFLAG);
		YXNo = Cos.wNo;
		YXStatus = Cos.byValue ;
		
		write_infoadd(YXNo + ADDR_YX_LO);
		if(m_guiyuepara.yxtype==3) 
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SIQ(YXStatus)+1;
		else
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = SIQ(YXStatus);	
	}

	if (YXSendNum == 0)
	{	
		return false;
	}
		
	SendFrameTail(PRM, dwCode, YXSendNum);
	
	return TRUE;
	
}
//发送变化遥信
BOOL CGB101S::SendDCos(void) 
{
	BYTE Style = M_DP_NA;
	BYTE Reason = COT_SPONT;
	BYTE PRM = 1;
	BYTE dwCode = 3;
	BYTE YXSendNum;
	WORD YXNo;
	VDBDCOS Cos;
	WORD RecCosNum;
	WORD ReadNum = 1;//每次读取1个COS
	
	SendFrameHead(Style, Reason);

	for(YXSendNum = 0; YXSendNum < GRP_YXNUM / 3 && GetDCosNum(m_wEqpID); YXSendNum++)
	{
		RecCosNum = ReadDCOS(m_wTaskID, m_wEqpID, ReadNum,sizeof(VDBDCOS), (VDBDCOS *)&Cos);
		if (RecCosNum == 0)
			break;
		ReadPtrIncrease(m_wTaskID, m_wEqpID, RecCosNum, DCOSUFLAG);
		YXNo = Cos.wNo;
		write_infoadd(YXNo + ADDR_YX_LO);

		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = DIQ(Cos.byValue1,Cos.byValue2);	
	}

	if (YXSendNum == 0)
	{	
		return false;
	}
		
	SendFrameTail(PRM, dwCode, YXSendNum);
	return TRUE;
}

//组织报文头
BOOL CGB101S::SendFrameHead(BYTE Style, BYTE Reason)
{
	m_SendBuf.wReadPtr = 0;
	m_SendBuf.wWritePtr=0;
	pSendFrame = (VIec101Frame *)m_SendBuf.pBuf;
	pSendFrame->Frame68.Start1	= pSendFrame->Frame68.Start2 = 0x68;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr]=0x68;
	m_SendBuf.wWritePtr+=3;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++]=0x68;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++]=0;
	write_linkaddr(GetAddress());
	write_typeid(Style);
	write_COT((GetAddress()<<8)|Reason);
	write_conaddr(GetEqpOwnAddr());
	
	return TRUE;
}

//组织报文尾，并发送整帧报文
BOOL CGB101S::SendFrameTail(BYTE PRM, BYTE dwCode, BYTE Num)
{
	WORD wLinkAddress;
	WORD typid;
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	WORD len;
#endif
	
	pSendFrame->Frame68.Length1 = pSendFrame->Frame68.Length2 = m_SendBuf.wWritePtr - 4;
	if((m_guiyuepara.mode!=1)&&(3==(dwCode&0xf)))
	{
		dwCode&=0xf0;
		dwCode|=8;
	}
	if((m_guiyuepara.mode==1)&&(8==(dwCode&0xf)))
	{
		dwCode&=0xf0;
		dwCode|=3;
	}
	pSendFrame->Frame68.Control = GetCtrCode(m_PRM, dwCode,1);

	pSendFrame->Frame68.Data[m_guiyuepara.linkaddrlen+m_guiyuepara.typeidlen] = Num;

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)ChkSum((BYTE *)&pSendFrame->Frame68.Control, pSendFrame->Frame68.Length1);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	if (m_guiyuepara.linkaddrlen ==1)
		wLinkAddress = pSendFrame->Frame68.Data[0];
	else
		wLinkAddress = MAKEWORD(pSendFrame->Frame68.Data[0],pSendFrame->Frame68.Data[1]);
	
	if(pSendFrame->Frame68.Start1 == 0X68)
	{
		if(m_guiyuepara.typeidlen == 1)
			typid = m_SendBuf.pBuf[5+m_guiyuepara.linkaddrlen];
		else if(m_guiyuepara.typeidlen == 2)
			typid = m_SendBuf.pBuf[5+m_guiyuepara.linkaddrlen]+(m_SendBuf.pBuf[6+m_guiyuepara.linkaddrlen]<<8);
	}

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if(3 == m_guiyuepara.jiami)
	{
		//ssz
		len = SecuritySendFrame(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,0);
		if(len<1)
		{
			return FALSE;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
	}
	if(4 == m_guiyuepara.jiami)
	{//ly
		len = SecuritySendFrame_HN(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,0,typid);
		if(len<1)
		{
			return FALSE;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
	}
#endif

	m_recfalg=0;
	m_zdflag=1;
	retrnum=m_pBaseCfg->wMaxRetryCount;
	retrflag=1;
	m_retime=0;
	if(m_dwasdu.LinkAddr==wLinkAddress)
		WriteToComm(wLinkAddress);
		
	return TRUE;
}

BOOL CGB101S::SendFrameTail0x1(BYTE PRM, BYTE dwCode, BYTE Num)
{
	WORD len;
	WORD typid;
	WORD wLinkAddress;

	pSendFrame->Frame68.Length1 = pSendFrame->Frame68.Length2 = m_SendBuf.wWritePtr - 4;
	if((m_guiyuepara.mode!=1)&&(3==(dwCode&0xf)))
		{
		dwCode&=0xf0;
		dwCode|=8;
		}
	if((m_guiyuepara.mode==1)&&(8==(dwCode&0xf)))
		{
		dwCode&=0xf0;
		dwCode|=3;
		}
	pSendFrame->Frame68.Control = GetCtrCode(m_PRM, dwCode,1);

	pSendFrame->Frame68.Data[m_guiyuepara.linkaddrlen+m_guiyuepara.typeidlen] = Num;

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)ChkSum((BYTE *)&pSendFrame->Frame68.Control, pSendFrame->Frame68.Length1);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	if (m_guiyuepara.linkaddrlen ==1)
		wLinkAddress = pSendFrame->Frame68.Data[0];
	else
		wLinkAddress = MAKEWORD(pSendFrame->Frame68.Data[0],pSendFrame->Frame68.Data[1]);

	if(pSendFrame->Frame68.Start1 == 0X68)
	{
		if(m_guiyuepara.typeidlen == 1)
			typid = m_SendBuf.pBuf[5+m_guiyuepara.linkaddrlen];
		else if(m_guiyuepara.typeidlen == 2)
			typid = m_SendBuf.pBuf[5+m_guiyuepara.linkaddrlen]+(m_SendBuf.pBuf[6+m_guiyuepara.linkaddrlen]<<8);
	}

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC

	if(3== m_guiyuepara.jiami)
	{//ssz
		len = SecuritySendFrame(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,1);
	}
	else if(4== m_guiyuepara.jiami)
	{//ly 湖南函数里面没有0x1此函数无用
		len = SecuritySendFrame_HN(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,1,typid);
	}
	
	if(len<1)
	{
		return FALSE;
	}
	else
	{
		m_SendBuf.wWritePtr  = len;
	}
#endif
	
	m_recfalg=0;
	m_zdflag=1;
	retrnum=m_pBaseCfg->wMaxRetryCount;
	retrflag=1;
	m_retime=0;
	if(m_dwasdu.LinkAddr==wLinkAddress)
	WriteToComm(wLinkAddress);
		
	return TRUE;
}

BOOL CGB101S::SendFrameTail0x2(BYTE PRM, BYTE dwCode, BYTE Num)
{
	WORD len;
	WORD typid;
	WORD wLinkAddress;

	pSendFrame->Frame68.Length1 = pSendFrame->Frame68.Length2 = m_SendBuf.wWritePtr - 4;
	if((m_guiyuepara.mode!=1)&&(3==(dwCode&0xf)))
		{
		dwCode&=0xf0;
		dwCode|=8;
		}
	if((m_guiyuepara.mode==1)&&(8==(dwCode&0xf)))
		{
		dwCode&=0xf0;
		dwCode|=3;
		}
	pSendFrame->Frame68.Control = GetCtrCode(m_PRM, dwCode,1);

	pSendFrame->Frame68.Data[m_guiyuepara.linkaddrlen+m_guiyuepara.typeidlen] = Num;

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)ChkSum((BYTE *)&pSendFrame->Frame68.Control, pSendFrame->Frame68.Length1);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	if (m_guiyuepara.linkaddrlen ==1)
		wLinkAddress = pSendFrame->Frame68.Data[0];
	else
		wLinkAddress = MAKEWORD(pSendFrame->Frame68.Data[0],pSendFrame->Frame68.Data[1]);

	if(pSendFrame->Frame68.Start1 == 0X68)
	{
		if(m_guiyuepara.typeidlen == 1)
			typid = m_SendBuf.pBuf[5+m_guiyuepara.linkaddrlen];
		else if(m_guiyuepara.typeidlen == 2)
			typid = m_SendBuf.pBuf[5+m_guiyuepara.linkaddrlen]+(m_SendBuf.pBuf[6+m_guiyuepara.linkaddrlen]<<8);
	}

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC

	if(3== m_guiyuepara.jiami)
	{//ssz
		len = SecuritySendFrame(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,2);
	}
	else if(4== m_guiyuepara.jiami)
	{//ly
		len = SecuritySendFrame_HN(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,2,typid);
	}
	
	if(len<1)
	{
		return FALSE;
	}
	else
	{
		m_SendBuf.wWritePtr  = len;
	}
#endif
	
	m_recfalg=0;
	m_zdflag=1;
	retrnum=m_pBaseCfg->wMaxRetryCount;
	retrflag=1;
	m_retime=0;
	if(m_dwasdu.LinkAddr==wLinkAddress)
	WriteToComm(wLinkAddress);
		
	return TRUE;
}

int CGB101S::SendAllFrame(void)
{
	WriteToComm(m_dwasdu.LinkAddr);
	return 1;
}

//发送读遥测数据
BOOL CGB101S::SendReadYCAck(WORD YCNo)
{
	BYTE Style = 9;
	BYTE Reason = 5;
	BYTE PRM = 0;
	BYTE dwCode = 8;
	BYTE Num = 1;
	WORD YCValue;
	WORD ReadYCNum = 1;

	SendFrameHead(Style, Reason);

	write_infoadd(YCNo + ADDR_YC_LO);
	ReadRangeYC(m_wEqpID, YCNo, ReadYCNum, 2/*sizeof(WORD)*/, (short *)&YCValue);
	#ifdef BEIJING_TEST
		YCValue <<= 4;	//for beijing test
	#endif
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YCValue); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YCValue);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0; 
	SendFrameTail(PRM, dwCode, Num);
	return TRUE;
}

//发送读遥信数据	
BOOL CGB101S::SendReadYXAck(WORD YXNo)
{
	BYTE Style = 1;
	BYTE Reason = 5;
	BYTE PRM=0;
	BYTE dwCode = 8;
	BYTE Num = 1;
	BYTE YXValue;
	WORD ReadYXNum = 1;
	
	ReadRangeSYX(m_wEqpID, YXNo, ReadYXNum, 1/*sizeof(BYTE)*/, &YXValue);
	YXValue = YXValue & 0x80;
	SendFrameHead(Style, Reason);
	
	write_infoadd(YXNo + ADDR_YX_LO);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = YXValue ? 1 : 0;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
	SendFrameTail(PRM, dwCode, Num);
	return TRUE;

}

//发送读电度数据
BOOL CGB101S::SendReadDDAck(WORD DDNo)
{
	BYTE Style = 15;
	BYTE Reason = 5;
	BYTE PRM = 0;
	BYTE dwCode = 8;
	BYTE Num = 1;
	DWORD DDValue;
	WORD ReadCINum = 1;

	SendFrameHead(Style, Reason);

	write_infoadd(DDNo + ADDR_DD_LO);
	ReadRangeDD(m_wEqpID, DDNo, ReadCINum, 4/*sizeof(DWORD)*/, (long *)&DDValue);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(LOWORD(DDValue));
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(LOWORD(DDValue));
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(HIWORD(DDValue)); 
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(HIWORD(DDValue));
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;
	
	SendFrameTail(PRM, dwCode, Num);

	return TRUE;
}


//无所请求数据帧
BOOL CGB101S::SendNoData(void)
{ 
	if(m_guiyuepara.mode==0)
		return SendBaseFrame(0, 0x09); 
	return SendBaseFrame(0, 0x01); 
} 


//发送遥控预置/执行 
BOOL CGB101S::SendYKSetAck(void) 
{
	BYTE  Reason = 7;
	BYTE PRM = 0, dwCode = 3, Num = 1;
	VYKInfo *pYKInfo;
	GotoEqpbyAddress(m_dwasdu.Address);
		
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	
	if (pYKInfo->Head.byMsgID == MI_YKCANCEL)
	{
		Reason = COT_DEACTCON;	
		if(m_YK_select_cancel==1)
		{
			m_YK_select_cancel=0;
			return false; 
		}
	}
	if (pYKInfo->Info.byStatus != 0)
	{
		Reason |= COT_PN_BIT;
		m_YKflag=0;
	//	pYKInfo->Head.byMsgID =MI_YKCANCEL;
	//	m_YK_select_cancel=1;
	//	    CPSecondary::DoYKReq();
	}
	if(pYKInfo->Head.byMsgID !=MI_YKOPRATE)
		m_YKflag=0;
		
	//	m_bySendStatus = SEND_YK;
	m_ykdelaystop=0;
	//m_acdflag=1;

    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	SendFrameHead(pCtrlInfo->Type, Reason);
	write_infoadd(pCtrlInfo->DOT);
	//	YKInfo.dot=pYKInfo->dot;
	//	YKInfo.type=pYKInfo->type;
		//YKInfo.DCO=pYKInfo->DCO;
	//if (pYKInfo->Info.byStatus == 0)
	//{
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pCtrlInfo->DCO;
	//}
	//else
	//{
	//	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] =0;
		
	//}

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if(3 == m_guiyuepara.jiami)
	{
		if (pYKInfo->Head.byMsgID == MI_YKCANCEL)
		{
#ifdef INCLUDE_SEC_CHIP
			if(m_wCommID == COMM_IPSEC_NO)
				SendFrameTail(PRM, dwCode, Num);//撤销确认，南网为00
			else
#endif
				SendFrameTail0x2(PRM, dwCode, Num);//撤销
		}
		else if(pCtrlInfo->DCO&0x80)
			SendFrameTail0x2(PRM, dwCode, Num);//预置
		else
			SendFrameTail(PRM, dwCode, Num);  
	}
	else if(4 == m_guiyuepara.jiami)
	{//ly
		if (pYKInfo->Head.byMsgID == MI_YKCANCEL)
			SendFrameTail(PRM, dwCode, Num);//撤销
		else if(pCtrlInfo->DCO&0x80)
			SendFrameTail0x2(PRM, dwCode, Num);//预置
		else
			SendFrameTail(PRM, dwCode, Num);  
	}
	else
#endif
	SendFrameTail(PRM, dwCode, Num);  
	
	return TRUE;
}
//发送遥调预置/执行 
BOOL CGB101S::SendYTSetAck(void) 
{
	BYTE  Reason = 7;
	BYTE PRM = 0, dwCode = 3, Num = 1;
	WORD ytvalue,byMs;
	VSysClock SysTime;
	VYTInfo *pYTInfo;
	GotoEqpbyAddress(m_dwasdu.Address);
		
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	
	if (pYTInfo->Head.byMsgID == MI_YTCANCEL)
	{
		Reason = COT_DEACTCON;	
	}
	if (pYTInfo->Info.byStatus != 0)
	{
		Reason |= COT_PN_BIT;
		m_YTflag=0;
	}
	if(pYTInfo->Head.byMsgID !=MI_YTOPRATE)
		m_YTflag=0;
		
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	SendFrameHead(pCtrlInfo->Type, Reason);
	write_infoadd(pCtrlInfo->DOT);
	if((pCtrlInfo->Type == C_SE_NA)||(pCtrlInfo->Type == C_SE_TA_1))
	{
		ytvalue = pYTInfo->Info.dwValue;
		SendWordLH(ytvalue);
	}
	else if((pCtrlInfo->Type == C_SE_NC_1)||(pCtrlInfo->Type == C_SE_TC_1))
		SendDWordLH(pYTInfo->Info.dwValue);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pCtrlInfo->DCO;

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
	SendFrameTail(PRM, dwCode, Num);  
	return TRUE;
}

//发送遥控预置/执行 
BOOL CGB101S::SenderrtypeAck(void) 
{
	
	SendFrameHead(m_dwasdu.TypeID,(m_dwasdu.COT+1)|0x40);
	write_infoadd(m_dwasdu.Info);
	
	SendFrameTail(0, 8, 1);  
	memset(m_tdata,0,sizeof(m_tdata));
	m_errflag=0;
	return TRUE;
}
BOOL CGB101S::Senderrtype1Ack(void) 
{
	BYTE PRM = 0, dwCode =3;
	m_SendBuf.wReadPtr = 0;
	m_SendBuf.wWritePtr=0;
	if(m_tdata[0]!=0x68)
	{
		memset(m_tdata,0,sizeof(m_tdata));
		m_errflag=0;
		return false;
	}
	m_tdata[m_dwasdu.COToff+5] = (0x40|44);
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_tdata,m_tdata[1]+4);
	m_SendBuf.wWritePtr+=m_tdata[1]+4;
	memset(m_tdata,0,sizeof(m_tdata));
	//m_tdata[0]=0;
	m_errflag=0;
		
	SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);
	return TRUE;
}
BOOL CGB101S::Senderrtype4Ack(BYTE  COT) 
{
	BYTE PRM = 0, dwCode =3;
	m_SendBuf.wReadPtr = 0;
	m_SendBuf.wWritePtr=0;
	if(m_tdata[0]!=0x68)
	{
		memset(m_tdata,0,sizeof(m_tdata));
		m_errflag=0;
		return false;
	}
	m_tdata[m_dwasdu.COToff+5] = (0x40|COT);
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_tdata,m_tdata[1]+4);
	m_SendBuf.wWritePtr+=m_tdata[1]+4;
	memset(m_tdata,0,sizeof(m_tdata));	
	//m_tdata[0]=0;
	m_errflag=0;
		
	SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);
	return TRUE;
}
BOOL CGB101S::Senderrtype3Ack(void) 
{
	
	SendFrameHead(m_dwasdu.TypeID,(47)|0x40);
	write_infoadd(m_dwasdu.Info);
	
	SendFrameTail(0, 8, 1);  
	memset(m_tdata,0,sizeof(m_tdata));
	m_errflag=0;
	return TRUE;
}
//发送遥控撤销
BOOL CGB101S::SendYKCancelAck(void) 
{
	BYTE Reason = 9;
	BYTE PRM = 0, dwCode =0x3, Num = 1;
	//BYTE * pData = &pReceiveFrame->Frame68.Data[m_byInfoShift];
//	VYKInfo *pYKInfo;
		
//	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
//	Style = pReceiveFrame->Frame68.Data[m_byTypeIDShift];

    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;	
	SendFrameHead(pCtrlInfo->Type, Reason);
	write_infoadd(pCtrlInfo->DOT);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] =pCtrlInfo->DCO;

/*
	pYKInfo->abyRsv[0]=Style;
	pYKInfo->abyRsv[1]=pData[0];
	pYKInfo->abyRsv[2]=pData[1];
	pYKInfo->abyRsv[3]=pData[2];*/
	SendFrameTail(PRM, dwCode, Num);  
	//m_bySendStatus = SEND_YK;
	m_ykdelaystop=0;
	return TRUE;
}

//发送遥控结束
BOOL CGB101S::SendYKstop(void) 
{
	BYTE  Reason = 10;
	BYTE PRM = 0, dwCode = 3, Num = 1;
	VYKInfo *pYKInfo;
		
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	if(pYKInfo->Head.byMsgID != MI_YKOPRATE) 
		return false;
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	SendFrameHead(pCtrlInfo->Type, Reason);
	write_infoadd(pCtrlInfo->DOT);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pCtrlInfo->DCO;

	m_acdflag=0;
	SendFrameTail(PRM, dwCode, Num);  
	return TRUE;
}
//发送遥调结束
BOOL CGB101S::SendYTstop(void) 
{
	BYTE  Reason = 10;
	BYTE PRM = 0, dwCode = 3, Num = 1;
	WORD ytvalue,byMs;
	VSysClock SysTime;
	VYTInfo *pYTInfo;
		
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	if(pYTInfo->Head.byMsgID != MI_YTOPRATE) 
		return false;
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	SendFrameHead(pCtrlInfo->Type, Reason);
	write_infoadd(pCtrlInfo->DOT);
	if((pCtrlInfo->Type == C_SE_NA)||(pCtrlInfo->Type == C_SE_TA_1))
	{
		ytvalue = pYTInfo->Info.dwValue;
		SendWordLH(ytvalue);
	}
	else if((pCtrlInfo->Type == C_SE_NC_1)||(pCtrlInfo->Type == C_SE_TC_1))
		SendDWordLH(pYTInfo->Info.dwValue);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pCtrlInfo->DCO;

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

	m_acdflag=0;
	SendFrameTail(PRM, dwCode, Num);  
	return TRUE;
}

void CGB101S::Initlink(void) 
{
#ifdef _GUANGZHOU_TEST_VER_
	m_initflag=4;
#endif

	if(m_initflag&1)    //请求远方链路状态
	{
		m_initflag&=~1;
		SendReqLink();
		return;
	}
	if(m_initflag&2)  //复位远方链路
	{
		m_initflag&=~2;
	    SendResetLink(PRM_MASTER);
		return;
	}
	if(m_initflag&4)	
	{
		m_initflag=0;
		return SendInitFinish();
	}
}
void CGB101S::SendInitFinish(void) 
{

	SendFrameHead(M_EI_NA, COT_INIT);
	write_infoadd(0);
	if((g_Sys.ResetInfo.code&0xff)==1)
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] =1;
	else if((g_Sys.ResetInfo.code&0xff)==2)
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] =2;
	else 
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] =0;

	SendFrameTail(PRM_MASTER, 0x03, 1);//funcode=0x0a?
	if(m_guiyuepara.mode == 1)  //平衡式
		m_linkflag=1;
	
	return;
}

void CGB101S::SendAck(void)
{ 
	//thSleep(1);
	//SendBaseFrame(PRM_SLAVE, SFC_CONFIRM); 
	WORD wLinkAddress;
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	WORD len;
#endif
	
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
	
	pSendFrame = (VIec101Frame *)m_SendBuf.pBuf;
	
	pSendFrame->Frame10.Start = 0x10;  
	pSendFrame->Frame10.Control = 0x0;	
	if(m_guiyuepara.mode==0)
		pSendFrame->Frame10.Control = 0x20;	
	pSendFrame->Frame10.Control|=m_dir_flag;
	write_10linkaddr(GetAddress());
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)ChkSum((BYTE *)&pSendFrame->Frame10.Control,m_guiyuepara.linkaddrlen+1);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	wLinkAddress = m_dwasdu.LinkAddr;
	
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if(3 == m_guiyuepara.jiami)
	{
		len = SecuritySendFrame(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,0);
		if(len<1)
		{
			return;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
	}
	if(4 == m_guiyuepara.jiami)
	{
		len = SecuritySendFrame_HN(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,0,0);
		if(len<1)
		{
			return;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
	}
#endif

	m_SendBuf.wReadPtr = 0;
	//if(m_dwasdu.LinkAddr==GetAddress())	
	WriteToComm(wLinkAddress);

} 

void CGB101S::Send2Ack(void)
{ 
	//thSleep(1);
	//SendBaseFrame(PRM_SLAVE, SFC_CONFIRM); 
	WORD wLinkAddress;
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	WORD len;
#endif
	
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
	
	pSendFrame = (VIec101Frame *)m_SendBuf.pBuf;
	
	pSendFrame->Frame10.Start = 0x10;  
	pSendFrame->Frame10.Control = 0;	
	//if(m_guiyuepara.mode==0)
	//	pSendFrame->Frame10.Control = 0x20;	
	pSendFrame->Frame10.Control|=m_dir_flag;

	write_10linkaddr(GetAddress());
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)ChkSum((BYTE *)&pSendFrame->Frame10.Control,m_guiyuepara.linkaddrlen+1);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	wLinkAddress = m_dwasdu.LinkAddr;
	
	m_SendBuf.wReadPtr = 0;
	//if(m_dwasdu.LinkAddr==GetAddress())	

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if(3 == m_guiyuepara.jiami)
	{
		len = SecuritySendFrame(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,0);
		if(len<1)
		{
			return;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
	}
	if(4 == m_guiyuepara.jiami)
	{
		len = SecuritySendFrame_HN(m_wCommID,m_SendBuf.pBuf,m_SendBuf.wWritePtr,2,0,0);
		if(len<1)
		{
			return;
		}
		else
		{
			m_SendBuf.wWritePtr  = len;
		}
	}
#endif

	WriteToComm(wLinkAddress);

} 


void CGB101S::SendNOAck(void)
{ 
	//thSleep(1);
	//SendBaseFrame(PRM_SLAVE, SFC_CONFIRM); 
	WORD wLinkAddress;
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
	
	pSendFrame = (VIec101Frame *)m_SendBuf.pBuf;
	
	pSendFrame->Frame10.Start = 0x10;  
	pSendFrame->Frame10.Control = 0x0;	
	if(m_guiyuepara.mode==0)
		pSendFrame->Frame10.Control = 0x1;
	pSendFrame->Frame10.Control|=m_dir_flag;

	write_10linkaddr(GetAddress());
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)ChkSum((BYTE *)&pSendFrame->Frame10.Control,m_guiyuepara.linkaddrlen+1);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	wLinkAddress = m_dwasdu.LinkAddr;
	
	m_SendBuf.wReadPtr = 0;
	//if(m_dwasdu.LinkAddr==GetAddress())	
	WriteToComm(wLinkAddress);

} 


/***************************************************************
	Function：DoReceive
		接收报文处理
	参数：无
		
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CGB101S::DoReceive()
{
//	DWORD event_time = 0;
	if (SearchFrame() != TRUE)
		return FALSE;	

	m_linkdelaytime=0;
	// 处理接收
	if(m_guiyuepara.mode==1)
		m_PRM =1;
	else
	{
		m_PRM=0;
	}
		retrnum=0;
	retrflag=0;

	pReceiveFrame = (VIec101Frame *)m_RecFrame.pBuf;
	if (pReceiveFrame->Frame10.Start == 0x10)
		return RecFrame10();
	
	if (pReceiveFrame->Frame68.Start1 == 0x68)
		return RecFrame68();
	
	return FALSE;
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
DWORD CGB101S::SearchOneFrame(BYTE *Buf, WORD Len)
{
	unsigned short FrameLen;
	DWORD wLinkAddress;
	WORD offset;
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	int ProcessRet;
	WORD SecLen;
	int nRet;
	BYTE type;
	DWORD Rc;
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

	if (Len < 5)
		return FRAME_LESS;
	
#ifdef INCLUDE_ECC
	int flag;

if((m_guiyuepara.jiami == 1) || (2 == m_guiyuepara.jiami))
{
	if(((wLinkAddress=SearcheccOneFrame(Buf,Len,m_guiyuepara.linkaddrlen,1))&FRAME_SHIELD)!= FRAME_ERR)
	{
			offset = wLinkAddress & ~FRAME_SHIELD;
			if((wLinkAddress & FRAME_SHIELD)==FRAME_OK&&offset>0)
			{//有加密报文，并进行了正确解密
				m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
				flag=sm2ver(Buf,m_SendBuf.pBuf,GetOwnAddr(),6,m_guiyuepara.jiami);
				if(flag<0) 
				{
					return FRAME_ERR|(wLinkAddress&0xfff);
				}
				//
				if(1 == m_guiyuepara.jiami)
				{
					if(flag>0)
					{
						m_SendBuf.wWritePtr=flag;
						WriteToComm(wLinkAddress);
						if(flag>20)
							wr_F3();
						//已经处理完成,其他的处理程序只需要跳过即可。
						return	FRAME_ERR|(wLinkAddress&0xfff);
					}
				}
			}
			if((wLinkAddress & FRAME_SHIELD) == FRAME_LESS)
			{
				return	wLinkAddress;
			}
	}
}
#ifdef ECC_MODE_CHIP
if(3 == m_guiyuepara.jiami)
{
	Rc=SecuritySearchOneFrame(Buf,Len);
	if((Rc&FRAME_SHIELD)==FRAME_ERR)
	{
		return FRAME_ERR|(Rc&0xfff);
	}
	if((Rc&FRAME_SHIELD)==FRAME_LESS)
		return FRAME_LESS;

	//OK
	if((Rc&FRAME_SHIELD)==FRAME_OK)
	{
		type = 0;
		ProcessRet = SecurityAllFrameProcess(m_wCommID,Buf,Len,m_SendBuf.pBuf,&m_SendBuf.wWritePtr,2,m_guiyuepara.linkaddrlen,&type);
		switch(ProcessRet)
		{
			case 0:
				return FRAME_ERR|(Rc&0xFFFF);
			case 1:
			{
					m_jiamiType = type;
				
				SecLen = Rc&0xFFFF;
				//下面处理
				break;
			}
			case 2:
			{
				m_SendBuf.wReadPtr = 0;
				SendAllFrame();
				return FRAME_ERR|(Rc&0xFFFF);
			}
			case 3:
			{//不需要任何处理，直接跳过即可
				m_SendBuf.wReadPtr = 0;
				m_SendBuf.wWritePtr= 0;
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
						thSleep(50);//50ms
					}
					if(nRet == 2)
						break;
				}
				return FRAME_ERR|(Rc&0xFFFF);
			}
			default:
				return FRAME_ERR|(Rc&0xFFFF);
		}
	}
	
}
if(4 == m_guiyuepara.jiami)
{
	Rc=SecuritySearchOneFrame_HN(Buf,Len);
	if((Rc&FRAME_SHIELD)==FRAME_ERR)
	{
		return FRAME_ERR|(Rc&0xfff);
	}
	if((Rc&FRAME_SHIELD)==FRAME_LESS)
		return FRAME_LESS;

	//OK
	if((Rc&FRAME_SHIELD)==FRAME_OK)
	{
		type = 0;
		ProcessRet = SecurityAllFrameProcess_HN(m_wCommID,Buf,Len,m_SendBuf.pBuf,&m_SendBuf.wWritePtr,2,m_guiyuepara.linkaddrlen,&type);
		switch(ProcessRet)
		{
			case 0:
				return FRAME_ERR|(Rc&0xFFFF);
			case 1:
			{
					m_jiamiType = type;//明文应用类型
				
				SecLen = Rc&0xFFFF;
				//下面处理
				break;
			}
			case 2:
			{
				m_SendBuf.wReadPtr = 0;
				SendAllFrame();
				return FRAME_ERR|(Rc&0xFFFF);
			}
			case 3:
			{//不需要任何处理，直接跳过即可
				m_SendBuf.wReadPtr = 0;
				m_SendBuf.wWritePtr= 0;
				return FRAME_ERR|(Rc&0xFFFF);
			}
			default:
				return FRAME_ERR|(Rc&0xFFFF);
		}
	}
	
}

#endif
#endif

	pReceiveFrame = (VIec101Frame *)Buf;//这里Buf是101，104原文
	switch(pReceiveFrame->Frame10.Start)
	{
		case 0x10:
			if (4+m_guiyuepara.linkaddrlen > Len)
				return FRAME_LESS;
			if(Buf[3+m_guiyuepara.linkaddrlen]!=0x16)
			{
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
				if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
					return FRAME_ERR|SecLen;  
				else
#endif	
					return FRAME_ERR|1;  	
			}
			FrameLen=4+m_guiyuepara.linkaddrlen;
#if 0				
			if (pReceiveFrame->Frame10.Data[m_byStopShift] != 0x16)
				return FRAME_ERR|1;  
			if (m_pVIec101Cfg->byLinkAdr ==1)
			{
				FrameLen=5;
			}
			else
			{
				if (m_pVIec101Cfg->byLinkAdr == 2)
				{
					FrameLen = 6;
				}
			}
#endif	
			if((Buf[1]&0x4f)!=0x4c)
				if (Buf[2+m_guiyuepara.linkaddrlen] != (BYTE)ChkSum((BYTE *)&pReceiveFrame->Frame10.Control, m_guiyuepara.linkaddrlen+1))
				{
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
					if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
						return FRAME_ERR|SecLen;  
					else
#endif
						return FRAME_ERR|1; 
				}
			if (m_guiyuepara.linkaddrlen==1)
				wLinkAddress = pReceiveFrame->Frame10.Data[0];
			else
				wLinkAddress = MAKEWORD(pReceiveFrame->Frame10.Data[0],pReceiveFrame->Frame10.Data[0+1]);
#if 0
			if (GotoEqpbyAddress(wLinkAddress) != TRUE)
				return FRAME_ERR|FrameLen;
#endif
			m_dwasdu.LinkAddr=wLinkAddress;

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
			if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
				return FRAME_OK|SecLen;
			else
#endif
				return FRAME_OK|FrameLen;

		case 0x68:
			if (pReceiveFrame->Frame68.Length1 != pReceiveFrame->Frame68.Length2)
				return FRAME_ERR|1;
			if (pReceiveFrame->Frame68.Start2 != 0x68)
			{
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
				if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
					return FRAME_ERR|SecLen;  
				else
#endif
					return FRAME_ERR|1;
			}
			FrameLen=pReceiveFrame->Frame68.Length1+6;
		 
			/*if (FrameLen > 50)
				return FRAME_ERR|1;*/
				
			if (FrameLen > Len)
				return FRAME_LESS;
			
#if 1
			if (Buf[FrameLen-1] != 0x16)
			{
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
				if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
					return FRAME_ERR|SecLen;  
				else
#endif
					return FRAME_ERR|1;
			}
			if (Buf[FrameLen-2] != (BYTE)ChkSum((BYTE *)&pReceiveFrame->Frame68.Control,pReceiveFrame->Frame68.Length1))
			{
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
				if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
					return FRAME_ERR|SecLen;  
				else
#endif
					return FRAME_ERR|1;
			}
			#if 0
			if (m_guiyuepara.linkaddrlen==1)
				wLinkAddress = pReceiveFrame->Frame68.Data[m_byLinkAdrShift];
			else
				wLinkAddress = MAKEWORD(pReceiveFrame->Frame68.Data[m_byLinkAdrShift],pReceiveFrame->Frame68.Data[m_byLinkAdrShift+1]);
			if (SwitchToAddress(wLinkAddress) != TRUE)
				return FRAME_ERR|FrameLen;
						m_dwasdu.LinkAddr=wLinkAddress;
			#endif
			#endif
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
			if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
				return FRAME_OK|SecLen;  
			else
#endif
				return FRAME_OK|FrameLen;
		default:	
			return FRAME_ERR|1;
	}
}


//固定帧格式报文接收处理
BOOL CGB101S::RecFrame10(void)
{
	DWORD para;
	if(m_dwasdu.LinkAddr!=GetAddress())	
		return false;
	if(m_guiyuepara.mode==0)
	{
		if((pReceiveFrame->Frame10.Control&BITS_PRM)==0)	
			return false;
	}
	/*switch(pReceiveFrame->Frame10.Control & BITS_CODE)
	{
		case 0x4A: //召唤一级用户数据
		case 0x4B: //远方链路状态完好或召唤二级用户数据
			if ((pReceiveFrame->Frame10.Control &BITS_FCBV) ==m_bReceiveControl )//重发
				{
					m_retxdnum++;
					if(m_retxdnum<4) 
						return SendRetry();//lqh 尽快解决
					return FALSE;
				}
			break;	
		default:
			break;
	}	*/	
	if(pReceiveFrame->Frame10.Control&BITS_DFC)
		m_bReceiveControl = pReceiveFrame->Frame10.Control&BITS_FCBV;//保存当前状态
	m_retxdnum=0;
	
	switch(pReceiveFrame->Frame10.Control & BITS_CODE)
	{
//#if(TYPE_USER == USER_GUANGXI)
		case 0x42: 
			if(m_linkflag==0)
			{
				return true;
			}
			SendAck();
			return TRUE; //复位远方链路
//#endif
#if(TYPE_USER == USER_TIANJIN)
		case 0x42: 
		case 0x43: 
		case 0x45: 
		case 0x46: 
		case 0x47: 
		case 0x4c: 
		case 0x4d: 
		case 0x4e: 
		case 0x4f: 
			if(m_linkflag==0)
			{
				return true;
			}
			SendBaseFrame(0,14);
			return TRUE; //复位远方链路
#endif

		case 0x0: 
		case 0xb: 
			m_recfalg=2;
			m_zdflag=0;
			RecACK(); 
			return TRUE; //复位远方链路
		case 0x40: 
			if(m_guiyuepara.mode==0)
			{
				if((pReceiveFrame->Frame10.Control&BITS_DFC)!=0)	
					return false;
			}
			if(pReceiveFrame->Frame10.Control&0x80)
				m_dir_flag=0;
			else
				m_dir_flag=0x80;
					#ifdef _GUANGZHOU_TEST_VER_
					//m_dir_flag=0;
					#endif

			m_callallflag=0;
			m_datatxdflag=0;
			m_YKflag=0;
			m_YTflag=0;
			m_initallflag=1;
			m_linkflag=0;
			m_calldir = 0;
			m_byReadFileFlag = 0;
			if(m_initflag==0)
				m_initflag=7;
			RecResetLink(); 
			return TRUE; //复位远方链路
		case 0x49: 
			if(pReceiveFrame->Frame10.Control&0x80)
				m_dir_flag=0;
			else
				m_dir_flag=0x80;
					#ifdef _GUANGZHOU_TEST_VER_
					//m_dir_flag=0;
					#endif
			if(m_guiyuepara.mode==0)
			{
				if((pReceiveFrame->Frame10.Control&BITS_DFC)!=0)	
					return false;
			}
			m_zdflag=0;
			RecReqLink(); 
			return TRUE; //请求远方链路状态
		
	//	case 0x4B: 
			
	
		//		SendNoData();	
		//	return TRUE; //远方链路状态完好或召唤二级用户数据
		//case 0x03:/*ACK 的处理 */
			//DoRecAck();
			//return TRUE;
		case 0x4A: 
		case 0x4B: 
			if(m_guiyuepara.mode==0)
			{
				if((pReceiveFrame->Frame10.Control&BITS_DFC)==0)	
					return false;
			}
			if(m_linkflag==0)
			{
				return  TRUE;
			}	
			SearchClass1();
			if(m_resetflag)
			{
				m_resetflag=0;
				if(SendresetAck())
					return TRUE; 
			}
			if(m_initflag)
			{
				m_initflag=4;
				Initlink();
				m_initflag=0;
				return TRUE; 
			}

			m_initflag=0;
					
			if(m_callallflag)//&&m_initallflag)
			{
				if(SendCallAll())
					return TRUE; 
				m_callallflag=0;
			}
			m_initallflag=0;
			if(m_YKflag)
			{
				if(m_YKflag==1)
				{
					m_YKflag++;
					if( SendYKSetAck())
						return TRUE;
				}
						
				m_YKflag=0;
								
				if(SendYKstop())
					return TRUE; 
			}

			if(m_YTflag)
			{
				if(m_YTflag==1)
				{
					m_YTflag++;
					if( SendYTSetAck())
						return TRUE;
				}
						
				m_YTflag=0;
								
				if(SendYTstop())
					return TRUE; 
			}
			if((m_errflag==COT_ACT)||(m_errflag==COT_DEACT))
			{
				if(SendsetparaAck())
					return TRUE; 
			}	
			if(m_delayflag)
			{
				m_delayflag=0;
				if(SenddelayAck())
					return TRUE; 
			}		
			if(m_set_paraflag)
			{
				m_set_paraflag=0;
				SendSetParaAck();
			}
			if(m_guiyuepara.tmp[0] & SWITCHCOS_BIT)
			{
				if(m_yxchangeflag)
				{
					m_acdflag=0;
					if(SendCos())
						return TRUE; 
					if(SendDCos())
						return TRUE; 
				}
				m_yxchangeflag=0;
			}
			for (WORD wSoeEqpNo = 0; wSoeEqpNo < m_wEqpNum; wSoeEqpNo++)
			{
				SwitchToEqpNo(wSoeEqpNo);
				if(SendSoe())  
					return TRUE;

			}
			if(SendDSoe())
				return TRUE; 
			m_SOEflag=0;
			if(m_timeflag)
			{
				m_timeflag=0;
				if(SendtimeAck())
					return TRUE; 
			}
			if(m_readtimeflag)		
			{
				m_readtimeflag=0;
				if(SendtimeReq())
					return TRUE; 
			}	
			if(m_callallflag)
			{
				if(SendCallAll())
					return TRUE; 
			}	

			if(SendFaultEvent())
				return TRUE;

			/*****************文件传输部分*************************/
			if(m_calldir)    //召唤目录
			{
				if(SendFileDir())
					return TRUE;
			}
			if(m_byReadFileAckFlag)
			{
				m_byReadFileAckFlag = 0;
				if(SendReadFileAck())
					return TRUE;	
			}
			if(m_byReadFileFlag)
			{
				if(SendReadFileData())
					return TRUE;
			}

			if(m_byWriteFileAckFlag || m_byWriteFileDataAckFlag)
			{
				SendWriteFileAck();
				m_byWriteFileAckFlag = 0;
				m_byWriteFileDataAckFlag = 0;
				return TRUE;
			}
			/******************文件传输部分END***********************/

			
			/************************电能量召唤*****************/
			if(m_byCallEnergyFlag||m_dwCallDDFlag||m_dwfiffrzflag||m_dwrealdataflag||m_dwdayfrzflag)
			{
				if(SendAllEnergy())
					return TRUE;
			}

			if(g_ddfifflag)   //  15min冻结突变
			{
				if(SendChangeFifFrz())
					return TRUE;
			}
			
			if(g_dddayflag)   //  日冻结突变
			{
				if(SendChangeDayFrz())
					return TRUE;
			}

			if(g_ddtideflag)   //  潮流突变
			{
				if(SendChangeTide())
					return TRUE;
			}
			/***************电能量召唤END**********************/
			
			/************************参数设置部分***********************/
			if(m_bySwitchValNoFlag)	   //切换定值区
			{
				m_bySwitchValNoFlag = 0;
				if(SendSwitchValNoAck())
					return TRUE;
			}

			if(m_byReadValNoFlag)      //读定值区
			{
				m_byReadValNoFlag = 0;
				if(SendReadValNoAck())
					return TRUE;
			}

			if(m_byAllParaValFlag || m_byMoreParaValFlag) //读参数
			{
				if(SendReadParaVal())
					return TRUE;
			}

			if(m_byWriteParaStatus)    //写参数
			{
				m_byWriteParaStatus = 0;
				if(SendWriteParaValAck())
					return TRUE;

			}

		/***********************参数设置部分END*********************/

		/****************软件升级*****************/
			if(m_bySoftUpgradeStatus)
			{
				if(SendSoftUpgradeAck())
					return TRUE;		
			}
		/****************软件升级END***************/
			
			if(m_readddflag)
			{
				if(SendCallAllDD())
						return TRUE;  
				m_readddflag=0;
			}
			if(m_readparaflag==1)
			{
				if(Sendpara())
					return TRUE;
				m_readparaflag=0;
			}
			m_acdflag=0;
			if(m_testflag)
			{
				m_testflag=0;
				if(SendTsetLinkAck())
					return TRUE; 
			}
			m_initallflag=0;
			if(m_errflag==1)
				return	SenderrtypeAck();
			if(m_errflag==2)
				return	Senderrtype4Ack(44);
			if(m_errflag==3)
				return	Senderrtype3Ack();
			if(m_errflag==4)
				return	Senderrtype4Ack(m_dwasdu.COT+1);
			if(m_errflag==5)
				return	Senderrtype4Ack(45);
			if(m_errflag==16)
				return	Senderrtype4Ack(46);
			if(m_errflag==7)
				return	Senderrtype4Ack(47);
	
			if(m_guiyuepara.mode==0)
			{
				if((pReceiveFrame->Frame10.Control&BITS_DFC)==0)	
					return false;
			}
			if(m_linkflag==0)
			{		
				return true;
			}
			if(SearchClass1())
				return SendNoData();
			if(m_testflag)
			{
				m_testflag=0;
				if(SendTsetLinkAck())
					return TRUE; 
			}
	
			if(m_groupflag)
			{
				if(SendCallgroup())
					return TRUE; 
			}
			if(m_testflag)
			{
				m_testflag=0;
				if(SendTsetLinkAck())
					return TRUE; 
			}	

			m_ycchangeflag=0;
			if(SendChangeYC())
				return TRUE;
#if(DEV_SP == DEV_SP_TTU)			
			if(m_allcycleflag&0x80)
			{
				if(CycleSendAllYcYx(m_allcycleflag&1))
				return TRUE;
			}
 #endif
			if(m_errflag==1)
				return	SenderrtypeAck();
			if(m_errflag==2)
				return	Senderrtype4Ack(44);
			if(m_errflag==3)
				return	Senderrtype3Ack();
			if(m_errflag==4)
				return	Senderrtype4Ack(7);
			if(m_errflag==5)
				return	Senderrtype4Ack(45);
			if(m_errflag==6)
				return	Senderrtype4Ack(46);
			if(m_errflag==7)
				return	Senderrtype4Ack(47);
			SendNoData();	
			return TRUE; //召唤一级用户数据		
		#if(TYPE_USER != USER_TIANJIN)
		case 0x4c: 
			SendAck();
			para = MAINT_ID;
			commCtrl(m_wCommID, CCC_TID_CTRL, (BYTE*)&para);
			break;
			#endif
		default:
			break;
	}
	return TRUE;
}


//ACK的处理
void CGB101S::DoRecAck(void)		
{
	if (m_byRTUStatus == RTU_RECCALL)
	{
		SendCallAll();
	}
	return;
}
//复位远方链路
BOOL CGB101S::RecACK(void)		
{
	DWORD event_time = 10;	//100 ms
	commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);//lqh 
	return true;
}

//复位远方链路
BOOL CGB101S::RecResetLink(void)		
{
	BYTE PRM = 0;
	VDBCOS Cos;
	WORD RecCosNum;
	memset(Bitchange,0,256);
	while(1)
	{
		RecCosNum = ReadSCOS(m_wTaskID, m_wEqpID, 1,sizeof(VDBCOS), (VDBCOS *)&Cos);
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
		RecCosNum = ReadDCOS(m_wTaskID, m_wEqpID, 1,sizeof(VDBDCOS), (VDBDCOS *)&Cos1);
		if (RecCosNum == 0)
			break;
		ReadPtrIncrease(m_wTaskID, m_wEqpID, RecCosNum, DCOSUFLAG);
	}	
	switch (pReceiveFrame->Frame10.Control & BITS_PRM)
	{
		case 0: //主站响应从站<复位远方链路>的帧
			#if(TYPE_USER == USER_GUANGZHOU)
			/*
			by lqh 20081103
			响应主站的链路复位应答，并发初始化结束
			*/
			if (m_byRTUStatus == RTU_RESET)
			{
				SendInitFinish();
				m_byRTUStatus = RTU_INITEND;
			}
			if (m_byRTUStatus == RTU_RECCALL)
			{
				SendCallAll();
			}
			#endif
			return TRUE;
		default: //主站下发的命令
			ClearEqpFlag(Rec_FCB);	//清接收FCB状态
			m_YKflag=0;
			m_YTflag=0;
			m_callallflag=0;
			m_acdflag=1;
			m_yxchangeflag=0;
			m_readddflag=0;
			m_groupflag=0;
			if(m_guiyuepara.mode == 0)  //非平衡式
				m_linkflag=1;
			SendResetLink(PRM);
			if(m_guiyuepara.mode==1)
			{
				m_initflag|=3;
				m_recfalg=1;
				DWORD event_time = 10;	//100 ms
				commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, (BYTE*)&event_time);
			}			
			return TRUE;
	}
}

//请求远方链路状态
BOOL CGB101S::RecReqLink(void)
{
	SendReqLinkAck(); 
	return TRUE;	
}

//召唤一级用户数据
BOOL CGB101S::RecCallClass1(void)
{ 
	return SendClass1(); 
}

//远方链路状态完好或召唤二级用户数据
BOOL CGB101S::RecCallClass2(void)
{
	BYTE PRM = 1;
		
	switch (pReceiveFrame->Frame10.Control & BITS_PRM)
	{
		case 0: //主站响应从站的召唤二级用户数据
			ClearEqpFlag(CALL_LINK);   //清主动发送<召唤远方链路>状态
			SendResetLink(PRM);   //"复位远方链路"
			m_byRTUStatus = RTU_RESET;
			return TRUE;

		default: //召唤二级用户数据
			SendClass2(MODE_UNBALANCE); 
			return TRUE;
	}

}


//可变帧长格式报文接收处理
BOOL CGB101S::RecFrame68(void)
{
	if(m_linkflag==0)
	{

	}
	getasdu();
	if(m_dwasdu.LinkAddr!=GetAddress())	
		return true;
	if(m_guiyuepara.mode==0)
	{
		if((pReceiveFrame->Frame68.Control&BITS_DFC)==0)	
			return false;
		if((pReceiveFrame->Frame68.Control&BITS_PRM)==0)	
			return false;
		if((pReceiveFrame->Frame68.Control&0xf)!=3)	
			return SendBaseFrame(0,14);
	}
	if(m_guiyuepara.mode==0)
		if ((pReceiveFrame->Frame68.Control &BITS_FCBV)==m_bReceiveControl)//重发
		{
			m_retxdnum++;
			if(m_retxdnum<4) 
				return SendRetry();//lqh 尽快解决
			return FALSE;
		}
	m_retxdnum=0;
	if(pReceiveFrame->Frame68.Control&BITS_DFC)
		m_bReceiveControl=pReceiveFrame->Frame68.Control&BITS_FCBV;
	m_acdflag=1;
	m_recfalg=1;
	if(m_dwasdu.Address!=GetEqpOwnAddr())
	{
		m_errflag=16;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		SendAck();
		return true;
	}

	
	switch(m_dwasdu.TypeID)
	{
		case 0x2D:	
		case 0x2E:	
		case 0x2F:	
			RecYKCommand();  
			break;//遥控
		case C_SE_NA:
		case C_SE_TA_1:
		case C_SE_NC_1: 
	    case C_SE_TC_1:
			RecYTCommand();
		 	break;
		case 0x64:	
			RecCallSomeGroup(); 
			break;//总召唤/召唤某一组数据
		//case 0x65:	
			//RecCallDDCommand(); 
			//break;//召唤电度
		case 0x66:	
			RecReadData(); 
			break;//读数据处理
		case 0x67:	
			RecSetClock(); 
			break;//时钟同步
		case 0x68:	
			RecTestLink(); 
			break; //测试链路
		case 0x69:	
			RecResetRTU(); 
			break; //复位RTU
		case 0x6A:
			Recdelay();
			break; //复位RTU
		case 0x6E:	
			RecSetPara(); 
			break;//设置参数
		case 0x81:
			RecTSData();//接收透明数据
			break;
		case 0x82:
			RecFileData();
			break;
		//case 0x30://设置遥测值
		//case 0x31://设置遥测值
		case 0x88://设置遥测值
			RecSetycpara();
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
			m_acdflag=0;
			m_errflag=2;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
			break;
	}
	if(((pReceiveFrame->Frame68.Control&0xf)!=4)&&(m_dwasdu.LinkAddr==GetOwnAddr()))
		{
		if(((m_dwasdu.TypeID==100)&&(m_groupflag))||(m_dwasdu.TypeID==0x67)
						||(m_dwasdu.TypeID==0x68)||(m_dwasdu.TypeID==0x69)/*|| (m_dwasdu.TypeID==F_FR_NA_1)|| (m_dwasdu.TypeID==F_SR_NA_1)*/)
		Send2Ack();
		else
		SendAck();
			
		}
	if(m_guiyuepara.mode==1)
		RecACK();
	return TRUE;
}


//遥控处理
BOOL CGB101S::RecYKCommand(void)
{
	if(m_dwasdu.VSQ!=1)
	{
		m_errflag=0x4;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;

	}
	if((m_dwasdu.COT!=6)&&(m_dwasdu.COT!=8))
	{
		m_errflag=0x5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;

	}
	m_YK_select_cancel=0;
	RecYKSet();
	return TRUE;
#if 0
	switch (m_dwasdu.COT& 0x3F)//传送原因
	{
		case 6: 
			RecYKSet(); 
			return TRUE;//遥控预置/执行处理
		case 8: 
			RecYKCancel(); 
			return TRUE;//停止激活
		default:
			break;
	}
	return TRUE;
#endif
}

//遥控预置/执行命令
BOOL CGB101S::RecYKSet(void)
{	
	BYTE * pData = &pReceiveFrame->Frame68.Data[m_dwasdu.Infooff];
	WORD  DCO;	//遥控命令限定词
	WORD  YKNo; //遥控路号
	BYTE QU=0;
	YKNo=m_dwasdu.Info;
	DCO=pData[m_guiyuepara.infoaddlen];
	QU=(DCO>>2)&0x1f;
	GotoEqpbyAddress(m_dwasdu.Address);
	SetEqpFlag(EQPFLAG_CLOCK);
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	if((YKNo<ADDR_YK_LO)||(YKNo>ADDR_YK_HI))
	{
		m_errflag=7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return true;
	}
	if (YKNo != 1)
		YKNo = YKNo - ADDR_YK_LO + 1;
	
	pYKInfo->Head.wEqpID = m_wEqpID;
	pYKInfo->Info.wID = YKNo;
	switch (DCO & 0x80)
	{
		case 0x80:
			pYKInfo->Info.byStatus = 0x01;
			pYKInfo->Head.byMsgID = MI_YKSELECT;
			break;
		case 0x00:
			pYKInfo->Info.byStatus = 0x0;
			pYKInfo->Head.byMsgID = MI_YKOPRATE;
			break;
		default:
			break;	
	}
	if((m_dwasdu.COT&0xf)==8)
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
		  		return FALSE;
		  	}
		  }
		  if(pYKInfo->Head.byMsgID == MI_YKOPRATE)
	  	  {
	  		if(m_jiamiType != 7)
	  		{
	  			SecuritySend1FFrame(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
				m_SendBuf.wReadPtr = 0;
				SendAllFrame();
				return FALSE;
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
		  		return FALSE;
		  	}
		  }
		  if(pYKInfo->Head.byMsgID == MI_YKOPRATE)
	  	  {
	  		if(m_jiamiType != 7)
	  		{
	  			SecuritySend1FFrame_HN(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
				m_SendBuf.wReadPtr = 0;
				SendAllFrame();
				return FALSE;
	  		}
	  	  } 
	}

#endif
	
	if(m_dwasdu.TypeID==C_SC_NA)
	{
		if(DCO&1)
			pYKInfo->Info.byValue = 0x05|(QU<<4);
		else
			pYKInfo->Info.byValue = 0x06|(QU<<4);
	}
	else
		switch (DCO & 0x03)
		{
			case 0: 
			case 3: 
				return FALSE;
			case 1: 
				pYKInfo->Info.byValue = 0x06|(QU<<4);
				break; //分
			case 2: 
				pYKInfo->Info.byValue = 0x05|(QU<<4);
				break; //合
			default:
				break;
		}
	
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	pCtrlInfo->Type=m_dwasdu.TypeID;
	pCtrlInfo->DOT=MAKEWORD(pData[0],pData[1]);
	pCtrlInfo->DCO=pData[m_guiyuepara.infoaddlen];
	CPSecondary::DoYKReq();
	
	return TRUE;
}

//遥控撤销命令
BOOL CGB101S::RecYKCancel(void)
{
	SendYKCancelAck();//确认
	return TRUE;
}


//总召唤/召唤某一组数据
BOOL CGB101S::RecCallSomeGroup(void)
{
	if(m_dwasdu.VSQ!=1)
	{
		m_errflag=0x4;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}
	if(m_dwasdu.COT!=6)
	{
		m_errflag=0x5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}
	if(m_dwasdu.Info!=0)
	{
		m_errflag=7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return true;
	}
	m_ztype=m_RecFrame.pBuf[5+m_dwasdu.Infooff+m_guiyuepara.infoaddlen];
	if(m_ztype==20) m_callallflag=20|0x80;
	if((m_ztype>20)&&(m_ztype<32)) m_groupflag=m_ztype|0x80;
	if(m_ztype==34)
	{
		m_readparaflag=1;
		m_readparaeqpnum=0;
		m_wSendparaNum=0;
		m_QPM=1;
	}
	if((m_ztype<20)||(m_ztype>34))
	{
		m_errflag=0x1;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
	}
	return TRUE;
}


//处理总召唤命令
BOOL CGB101S::RecCallAllCommand(void)
{
	switch (m_dwasdu.COT& 0x3F)//传送原因
	{
		case 6: 
			RecCallAllStart(); 
			return TRUE;//总召唤激活
		case 8://停止激活
			RecCallAllStop(); 
			break;
		default:
			break;
	}
	return TRUE;
}


//处理总召唤激活命令
BOOL CGB101S::RecCallAllStart(void)
{

//	#if (TYPE_USER ==  USER_GUANGZHOU)
	/*if(m_guiyuepara.mode ==1) //平衡式
	{
		m_byRTUStatus = RTU_RECCALL;
		SendAck();
		thSleep(100);
	}*/
//	#endif
	
	SendCallAllStartAck();
	#if 0
	event_time = 10;	//100 ms
	#ifdef INCLUDE_SUYUAN101FORMAT
	event_time = 300;	//3000 ms
	#endif

	#if (TYPE_USER != USER_GUANGZHOU)
	commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	
	Iec101Info[m_wEqpNo].SendIdleSendFrame = SEND_CALLALL;
	#endif
	
	for (WORD EqpFlag = CALL_YXGRP1; EqpFlag <= CALL_ALLSTOP; EqpFlag++)
		SetEqpFlag(EqpFlag);
	#else
	#endif
	if(m_ztype==20)
	{
		for (WORD EqpFlag = CALL_YXGRP1; EqpFlag < CALL_ALLSTOP; EqpFlag++)
			SetEqpFlag(EqpFlag);
	}
	if((m_ztype>20)&&(m_ztype<33))
	{
		if(m_ztype-21+CALL_YXGRP1<CALL_ALLSTOP)
			SetEqpFlag(m_ztype-21+CALL_YXGRP1);
	}
	SetEqpFlag(CALL_ALLSTOP);

	return TRUE;
}
//处理总召唤激活命令
BOOL CGB101S::RecCallAllStop(void)
{
	if(m_callallflag)
	{
		if(m_ztype==20)
		{
			for (WORD EqpFlag = CALL_YXGRP1; EqpFlag < CALL_ALLSTOP; EqpFlag++)
				ClearEqpFlag(EqpFlag);
		}
		if((m_ztype>20)&&(m_ztype<28))
		{
			if(m_ztype-21+CALL_YXGRP1<CALL_ALLSTOP)
				ClearEqpFlag(m_ztype-21+CALL_YXGRP1);
		}
	}
	m_callallflag=0;
	ClearEqpFlag(CALL_ALLSTOP);
						
	if(m_ztype==34)
	{
		m_readparaflag=0;
		m_readparaeqpnum=0;
		m_wSendparaNum=0;
		m_QPM=1;
	}
	
	return TRUE;
}

//召唤电度
BOOL CGB101S::RecCallDDCommand(void)
{
	BYTE * pdata=pReceiveFrame->Frame68.Data+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;

	switch (m_dwasdu.COT& 0x3F)
	{
		case 5:  
			RecCallAllDD(); //RecCallSomeDD(); 
			m_readddflag=pdata[0];
			break;	//召唤某组电度
		case 6:  
			RecCallAllDD(); 
			SetEqpFlag(CALL_ALLDDSTOP);
			m_readddflag=pdata[0]|0x80;

			break;	//召唤所有电度
		default:
			break;
	}
	
	return TRUE;
}


//召唤某组电度
BOOL CGB101S::RecCallSomeDD(void)
{
	SendSomeGroupDD();
	return TRUE;
}

//召唤所有电度
BOOL CGB101S::RecCallAllDD(void)
{
	
	WORD FlagNo;
	BYTE * pdata=pReceiveFrame->Frame68.Data+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;

	if((pdata[0]&0xf)==5)
	for (FlagNo = CALL_DDGRP1; FlagNo < CALL_ALLDDSTOP; FlagNo++)
		SetEqpFlag(FlagNo);
	else if(((pdata[0]&0xf)>0)&&((pdata[0]&0xf)<5))
		SetEqpFlag(CALL_DDGRP1+(pdata[0]&0xf)-1);
	return true;
	#if 0
	commCtrl (m_wCommID, CCC_EVENT_CTRL|COMM_IDLE_OFF, &event_time);
	event_time = 10;	//100 ms
	#ifdef INCLUDE_SUYUAN101FORMAT
	event_time = 300;	//3000 ms
	#endif
	commCtrl (m_wCommID, CCC_EVENT_CTRL|TX_IDLE_ON, &event_time);
	
	Iec101Info[m_wEqpNo].SendIdleSendFrame = SEND_CALLALLDD;
	#endif

}


//读数据处理
BOOL CGB101S::RecReadData(void)
{
	thSleep(5);
	DoReadData();
	return TRUE;
}

//时钟同步
BOOL CGB101S::RecSetClock(void)
{
	BYTE * pData = &pReceiveFrame->Frame68.Data[m_guiyuepara.infoaddlen];
	VSysClock SysTime;	
	WORD MSecond;
	if(m_dwasdu.VSQ!=1)
	{
		m_errflag=0x4;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}
	if(m_dwasdu.COT == 5)
	{
		m_readtimeflag = 1;
		return TRUE;
	}	
	if(m_dwasdu.COT!=6)
	{
		m_errflag=0x5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}
	if(m_dwasdu.Info!=0)
	{
		m_errflag=7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return true;
	}
	pData=&pReceiveFrame->Frame68.Data[m_dwasdu.Infooff+m_guiyuepara.infoaddlen];
	GetSysClock((void *)&delayclock, SYSCLOCK);
	if(delayclock.wMSecond>20)
		delayclock.wMSecond-=20;
	else
		delayclock.wMSecond=0;
	MSecond = MAKEWORD(pData[0], pData[1]); 
	MSecond+=20;
	SysTime.wMSecond = MSecond % 1000;
	SysTime.bySecond  = MSecond / 1000;
	SysTime.byMinute = pData[2]&0x3F;
	SysTime.byHour = pData[3]&0x1F;
	SysTime.byDay = pData[4] & 0x1F;
	SysTime.byWeek = pData[4] >> 5;
	SysTime.byMonth = pData[5] & 0x0F;
	SysTime.wYear = (pData[6] & 0x7F) + m_guiyuepara.baseyear;
#if(TYPE_USER == USER_GUANGXI)
	delayclock.wMSecond = MSecond % 1000;
	delayclock.bySecond  = MSecond / 1000;
	delayclock.byMinute = pData[2];
	delayclock.byHour = pData[3];
	delayclock.byDay = pData[4] & 0x1F;
	delayclock.byWeek = pData[4] >> 5;
	delayclock.byMonth = pData[5] & 0x0F;
	delayclock.wYear = (pData[6] & 0x7F) + m_guiyuepara.baseyear;
			
#endif
	if(ClockIsOk(&SysTime) == ERROR)
	{
		m_errflag=4;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}
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
	m_timeflag=1;
	return true;
	#if 0
	#if (TYPE_USER == USER_GUANGZHOU)
	SendAck();
	m_byRTUStatus = RTU_SETTIME;
	thSleep(100);
	//#endif
	
	if (pReceiveFrame->Frame68.Data[m_byLinkAdrShift] != m_pBaseCfg->wBroadcastAddr)
	SendSetClockAck();//对钟的确认

	
	#else

	BYTE * pData = &pReceiveFrame->Frame68.Data[m_byInfoShift];
		
	WORD MSecond;

	switch (m_pVIec101Cfg->byInfoAdr )
	{
		case 1:
			pData += 1;  //指向时间信息处
			break;
		case 2:
			pData += 2;  //指向时间信息处
			break;
		case 3:
			pData += 3;  //指向时间信息处
			break;
		default:
			pData += 2;  //指向时间信息处
			break;
	}
	
	
	MSecond = MAKEWORD(pData[0], pData[1])+100; //delay 100ms
	m_SysTime.wMSecond = MSecond % 1000;
	m_SysTime.bySecond  = MSecond / 1000;
	m_SysTime.byMinute = pData[2];
	m_SysTime.byHour = pData[3];
	m_SysTime.byDay = pData[4] & 0x1F;
	m_SysTime.byWeek = pData[4] >> 5;
	m_SysTime.byMonth = pData[5] & 0x0F;
	m_SysTime.wYear = (pData[6] & 0x7F) + 2000;
	
	m_bySendStatus = SEND_TIME;
	SendBaseFrame(PRM_SLAVE, SFC_LINKOK);
	
	#endif
	return TRUE;
	#endif
}

//参数设置
BOOL CGB101S::RecSetycpara(void)
{
	if(m_dwasdu.VSQ!=1)
	{
		if(m_dwasdu.TypeID==136)
		{
			if(((pReceiveFrame->Frame68.Length1-1-m_dwasdu.Infooff)/(m_guiyuepara.infoaddlen+2))!=m_dwasdu.VSQ)
			{
				m_errflag=0x4;
				memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
				return TRUE;
			}	
		}
		else
		{
			m_errflag=0x4;
			memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
			return TRUE;
		}

	}
	if((m_dwasdu.COT!=6)&&(m_dwasdu.COT!=8))
	{
		m_errflag=0x5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}

	if((m_dwasdu.Info<ADDR_YT_LO)||(m_dwasdu.Info>ADDR_YT_HI))
	{
		m_errflag=7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return true;
	}
	memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
	if(m_dwasdu.COT==COT_ACT)
		m_tdata[m_dwasdu.COToff+5]=COT_ACTCON;
	else if(m_dwasdu.COT==COT_DEACT)
		m_tdata[m_dwasdu.COToff+5]=COT_DEACTCON;
	m_errflag=m_dwasdu.COT;
	return true;
	
}

//测试链路
BOOL CGB101S::RecTestLink(void)
{
	m_testflag=1;
	
#if (TYPE_USER == USER_GUANGZHOU)
//	SendAck();
	//thSleep(100);
#endif
	//return SendTsetLinkAck();

	return true;
}

//复位RTU
BOOL CGB101S::RecResetRTU(void)
{ 
	BYTE RecSoeNum=10;
	BYTE QRP = pReceiveFrame->Frame68.Data[m_dwasdu.Infooff+m_guiyuepara.infoaddlen];
	m_QRP=QRP;

	if(m_dwasdu.VSQ!=1)
	{
		m_errflag=0x4;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}	
	if(m_dwasdu.COT!=6)
	{
		m_errflag=0x5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}
	if(m_dwasdu.Info != 0)
	{
		m_errflag=0x7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}
	m_resetflag=1;
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
	return TRUE; 
}

//设置参数
BOOL CGB101S::RecSetPara(void)
{
	m_set_paraflag=1;
	Setpara();
	writepara();
	return true;
}
BOOL CGB101S::Recdelay(void)
{
	if(m_dwasdu.VSQ!=1)
	{
		m_errflag=0x4;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;

	}
	if((m_dwasdu.COT!=6)&&(m_dwasdu.COT!=3))
	{
		m_errflag=0x5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;

	}
	if(m_dwasdu.Info!=0)
	{
		m_errflag=7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return true;
	}
	if(m_dwasdu.COT==6)
	{
		m_delayflag=1; 
			
	}
	else
	 	m_dwasdu.TypeID=0x68;
	return true;
}

//接收透明数据
void CGB101S::RecTSData(void)
{
	BYTE * pData = &pReceiveFrame->Frame68.Data[m_guiyuepara.infoaddlen+ m_dwasdu.Infooff];
	VMsg *pMsg;

	pMsg = (VMsg *)&m_dwPubBuf;
	pMsg->Head.wLength = sizeof(VMsgHead) + pReceiveFrame->Frame68.Length1 - 8;
	pMsg->Head.byMsgID = MI_ROUTE;
	pMsg->Head.byMsgAttr = MA_TSDATA;
	pMsg->Head.wThID =m_wTaskID;
	pMsg->Head.wEqpID = m_wEqpID;
	memcpy((void *)pMsg->abyData, (void *)pData, pReceiveFrame->Frame68.Length1-8);

	SendNoData();
}

void CGB101S::DoTSDataMsg(void)
{
	m_wTSDataLen = m_pMsg->Head.wLength-sizeof(VMsgHead);
	memcpy((void *)m_byTSData, m_pMsg->abyData, m_wTSDataLen);
	
	SetEqpFlag(EQPFLAG_ROUTE);
}

void CGB101S::SendTSData(void)
{
	BYTE Style = 0x81, Reason = 5;
	BYTE PRM = 0, dwCode = 8;
	SendFrameHead(Style, Reason);
	write_infoadd(0);
	memcpy((void *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (void *)m_byTSData, m_wTSDataLen);
	
	m_SendBuf.wWritePtr += m_wTSDataLen;
	SendFrameTail(PRM, dwCode, 1);
}

//接收文件数据
void CGB101S::RecFileData(void)
{
	BYTE byFunCode,*pData;

	pData = &pReceiveFrame->Frame68.Data[m_guiyuepara.infoaddlen+ m_dwasdu.Infooff];

	byFunCode = *pData;
	pData++;
	switch(byFunCode)
	{
		case 1:
			DoReadDir(pData);
			break;
		case 2:
			DoReadFileStatus(pData);
			break;
		case 3:
			DoReadFileData(pData);
			break;
		default:
			SendNoData();
			break;
	}
	return;
}

void CGB101S::DoReadDir(BYTE *pData)
{
	BYTE Style = 0x82, Reason = 5;
	BYTE PRM = 0, dwCode = 8;
	
	FileStatus filestatus;
	VFileOPMsg FileOPMsg;
	WORD DirNum;
	VDirInfo *pDirInfo = NULL;
    BYTE *pFrame=NULL;
	BYTE ErrorCode = 4;	
	
	WORD wReadPtr, wRecOffset;
	WORD wTotleDirNum;
	
	pData[FILE_NAME_LEN - 1] = 0;	//强制将文件名最后一个字符设置为空字符
	strcpy((char *)FileOPMsg.cFileName, (char *)pData);
	FileOPMsg.dwOffset = 0;
	FileOPMsg.dwLen = 44*4;//sizeof(VDirInfo)*4;//read max 4 items each time
	
	pData += FILE_NAME_LEN;
	wRecOffset = MAKEWORD(pData[0], pData[1]);	
	FileOPMsg.dwOffset = wRecOffset;
	
	SendFrameHead(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 1;	//读目录
	
	strcpy((char *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (char *)FileOPMsg.cFileName);
	m_SendBuf.wWritePtr += FILE_NAME_LEN;
	
	filestatus = ListDir(&FileOPMsg, (VDirInfo *)&m_dwPubBuf);
	
	DirNum = FileOPMsg.dwLen / 44;//for debug//sizeof(VDirInfo);//44
	pDirInfo = (VDirInfo *)&m_dwPubBuf;
	
	wReadPtr = LOWORD(FileOPMsg.dwOffset);
	wTotleDirNum = LOWORD(FileOPMsg.dwSize);
	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(wReadPtr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(wReadPtr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(DirNum);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(DirNum);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(wTotleDirNum);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(wTotleDirNum);
	
	pFrame = &m_SendBuf.pBuf[m_SendBuf.wWritePtr];
	if (filestatus == FileOk)
	{
		for (int i = 0; i < DirNum; i++)
		{
			strcpy((char *)pFrame, (char *)pDirInfo->cName);
			pFrame += FILE_NAME_LEN;
			pFrame[0] = LOWORD(LOBYTE(pDirInfo->dwSize));
			pFrame[1] = LOWORD(HIBYTE(pDirInfo->dwSize));
			pFrame[2] = HIWORD(LOBYTE(pDirInfo->dwSize));
			pFrame[3] = HIWORD(HIBYTE(pDirInfo->dwSize));
			pFrame[4] = LOWORD(LOBYTE(pDirInfo->dwMode));
			pFrame[5] = LOWORD(HIBYTE(pDirInfo->dwMode));
			pFrame[6] = HIWORD(LOBYTE(pDirInfo->dwMode));
			pFrame[7] = HIWORD(HIBYTE(pDirInfo->dwMode));
			pFrame[8] = LOWORD(LOBYTE(pDirInfo->dwTime));
			pFrame[9] = LOWORD(HIBYTE(pDirInfo->dwTime));
			pFrame[10] = HIWORD(LOBYTE(pDirInfo->dwTime));
			pFrame[11] = HIWORD(HIBYTE(pDirInfo->dwTime));
			pFrame += 4 * 3;//(sizeof(DWORD) * 3);
			pDirInfo ++;
		}
		m_SendBuf.wWritePtr += FileOPMsg.dwLen;
	}
	else 
	{
		SendFileNAck(ErrorCode);
		return;	
	}
	SendFrameTail(PRM, dwCode, 1);
	return;
}

void CGB101S::DoReadFileStatus(BYTE *pData)
{
	BYTE Style = 0x82, Reason = 5;
	BYTE PRM = 0, dwCode = 8;
	
	FileStatus filestatus;
	VFileOPMsg FileOPMsg;
	VFileStatus *pFileStatus;
	BYTE *pFrame = NULL;
	BYTE ErrorCode = 4;	

	pData[FILE_NAME_LEN - 1] = 0;	//强制将文件名最后一个字符设置为空字符
	strcpy((char *)FileOPMsg.cFileName, (char *)pData);
	FileOPMsg.dwOffset = 0;
	FileOPMsg.dwLen = MAX_FRAME_LEN - FILE_NAME_LEN - 12;
	FileOPMsg.dwSize = 0;
	
	SendFrameHead(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 2;	//读文件状态信息
	
	strcpy((char *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (char *)FileOPMsg.cFileName);
	m_SendBuf.wWritePtr += FILE_NAME_LEN;
	
	filestatus = ReadFileStatus(&FileOPMsg, (VFileStatus *)&m_dwPubBuf);
	pFileStatus = (VFileStatus *)&m_dwPubBuf;
	pFrame = &m_SendBuf.pBuf[m_SendBuf.wWritePtr];
	if (filestatus == FileOk)
	{
		strcpy((char *)pFrame, (char *)FileOPMsg.cFileName);
		pFrame += FILE_NAME_LEN;
		pFrame[0] = LOWORD(LOBYTE(pFileStatus->dwSize));
		pFrame[1] = LOWORD(HIBYTE(pFileStatus->dwSize));
		pFrame[2] = HIWORD(LOBYTE(pFileStatus->dwSize));
		pFrame[3] = HIWORD(HIBYTE(pFileStatus->dwSize));
		
		pFrame[4] = LOWORD(LOBYTE(pFileStatus->dwMode));
		pFrame[5] = LOWORD(HIBYTE(pFileStatus->dwMode));
		pFrame[6] = HIWORD(LOBYTE(pFileStatus->dwMode));
		pFrame[7] = HIWORD(HIBYTE(pFileStatus->dwMode));
		
		pFrame[8] = LOWORD(LOBYTE(pFileStatus->dwTime));
		pFrame[9] = LOWORD(HIBYTE(pFileStatus->dwTime));
		pFrame[10] = HIWORD(LOBYTE(pFileStatus->dwTime));
		pFrame[11] = HIWORD(HIBYTE(pFileStatus->dwTime));
		
		pFrame[12] = LOBYTE(pFileStatus->wCrc);
		pFrame[13] = HIBYTE(pFileStatus->wCrc);
		
		
		pFrame[14] = LOWORD(LOBYTE(pFileStatus->dwTempSize));
		pFrame[15] = LOWORD(HIBYTE(pFileStatus->dwTempSize));
		pFrame[16] = HIWORD(LOBYTE(pFileStatus->dwTempSize));
		pFrame[17] = HIWORD(HIBYTE(pFileStatus->dwTempSize));
		
		pFrame[18] = LOWORD(LOBYTE(pFileStatus->dwTempMode));
		pFrame[19] = LOWORD(HIBYTE(pFileStatus->dwTempMode));
		pFrame[20] = HIWORD(LOBYTE(pFileStatus->dwTempMode));
		pFrame[21] = HIWORD(HIBYTE(pFileStatus->dwTempMode));
		
		pFrame[22] = LOWORD(LOBYTE(pFileStatus->dwTempTime));
		pFrame[23] = LOWORD(HIBYTE(pFileStatus->dwTempTime));
		pFrame[24] = HIWORD(LOBYTE(pFileStatus->dwTempTime));
		pFrame[25] = HIWORD(HIBYTE(pFileStatus->dwTempTime));
		
		pFrame[26] = LOBYTE(pFileStatus->wTempCrc);
		pFrame[27] = HIBYTE(pFileStatus->wTempCrc);
		
		
		m_SendBuf.wWritePtr += FileOPMsg.dwLen;
	}
	else 
	{
		SendFileNAck(ErrorCode);
		return;	
	}
	SendFrameTail(PRM, dwCode, 1);
	return;
}

void CGB101S::DoReadFileData(BYTE *pData)
{
	BYTE Style = 0x82, Reason = 5;
	BYTE PRM = 0, dwCode = 8;
	
	FileStatus filestatus;
	VFileOPMsg FileOPMsg;
	BYTE FileEndFlag;
	WORD PackLenOffset;
	WORD EndFlagOffset;
	BYTE ErrorCode=4;
	DWORD FileLenOffset;
	
	pData[FILE_NAME_LEN - 1] = 0;	//强制将文件名最后一个字符设置为空字符	
	strcpy((char *)FileOPMsg.cFileName, (char *)pData);
	pData += FILE_NAME_LEN;
	FileOPMsg.dwSize = 0;
	FileOPMsg.dwOffset = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	FileOPMsg.dwLen = MAKEWORD(pData[4], pData[5]);

	if (FileOPMsg.dwLen > MAX_FRAME_LEN - FILE_NAME_LEN - 25)
		FileOPMsg.dwLen = MAX_FRAME_LEN - FILE_NAME_LEN - 25;
	
	SendFrameHead(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 3;	//读文件内容
	
	strcpy((char *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (char *)FileOPMsg.cFileName);
	m_SendBuf.wWritePtr += FILE_NAME_LEN;
	
	FileLenOffset = m_SendBuf.wWritePtr;
	m_SendBuf.wWritePtr += 4;//sizeof(DWORD);
	SendDWordLH(FileOPMsg.dwOffset);
	PackLenOffset = m_SendBuf.wWritePtr;
	m_SendBuf.wWritePtr += 2;
	EndFlagOffset = m_SendBuf.wWritePtr;
	m_SendBuf.wWritePtr += 1;
	filestatus = ReadFile(&FileOPMsg, &m_SendBuf.pBuf[m_SendBuf.wWritePtr]);
	if ((filestatus == FILEOK) || (filestatus == FILEEOF))
	{
		m_SendBuf.wWritePtr += FileOPMsg.dwLen;
		if (filestatus == FILEOK)
		{
			FileEndFlag = 0;	
		}
		else
		{
			FileEndFlag = 0xff;
		}
	}
	else
	{
		SendFileNAck(ErrorCode);
		return;
	}
	SendDWordLH(FileLenOffset, FileOPMsg.dwSize);
	SendWordLH(PackLenOffset, FileOPMsg.dwLen);
	m_SendBuf.pBuf[EndFlagOffset] = FileEndFlag;
	SendFrameTail(PRM, dwCode, 1);
	return;
}

//文件操作失败应答
void CGB101S::SendFileNAck(BYTE ErrorCode)
{
	BYTE Style = 0x82, Reason = 5;
	BYTE PRM = 0, dwCode = 8;
	SendFrameHead(Style, Reason);
	write_infoadd(0);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = ErrorCode;
	SendFrameTail(PRM, dwCode, 1);
	return;
}


BOOL CGB101S::RecYTCommand(void)
{
	if(m_dwasdu.VSQ!=1)
	{
		m_errflag=0x4;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;

	}
	if((m_dwasdu.COT!=6)&&(m_dwasdu.COT!=8))
	{
		m_errflag=0x5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;

	}
	if((m_dwasdu.TypeID == C_SE_NA)||(m_dwasdu.TypeID == C_SE_TA_1))
		RecYTSet();
	else if((m_dwasdu.TypeID == C_SE_NC_1)||(m_dwasdu.TypeID == C_SE_TC_1))
		RecYTFSet();
	return TRUE;
}
//遥调预置/执行命令 归一化值
BOOL CGB101S::RecYTSet(void)
{	
	BYTE * pData = &pReceiveFrame->Frame68.Data[m_dwasdu.Infooff];
	WORD  DCO;	//遥调命令限定词
	WORD  YTNo; //遥调路号
	BYTE infolen;
	infolen = m_guiyuepara.infoaddlen;
	YTNo=m_dwasdu.Info;
	DCO=pData[infolen+2];
	GotoEqpbyAddress(m_dwasdu.Address);
	SetEqpFlag(EQPFLAG_CLOCK);
	VYTInfo *pYTInfo;
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	if((YTNo<ADDR_YT_LO)||(YTNo>ADDR_YT_HI))
	{
		m_errflag=7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}

	if(GotoEqpbyAddress(m_dwasdu.Address)==false)
	{
		pYTInfo->Info.byStatus=1;
		return TRUE;
	}
	YTNo = YTNo - ADDR_YT_LO + 1;
	
	pYTInfo->Head.wEqpID = m_wEqpID;
	pYTInfo->Info.wID = YTNo;
	switch (DCO & 0x80)
	{
		case 0x80:
			pYTInfo->Head.byMsgID = MI_YTSELECT;
			break;
		case 0x00:
			pYTInfo->Head.byMsgID = MI_YTOPRATE;
			break;
		default:
			break;	
	}
	if((m_dwasdu.COT&0xf)==8)
	{
		pYTInfo->Head.byMsgID = MI_YTCANCEL;
	}
	pYTInfo->Info.dwValue=MAKEWORD(pData[infolen], pData[infolen+1]);
	
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	pCtrlInfo->Type=m_dwasdu.TypeID;
	pCtrlInfo->DOT=MAKEWORD(pData[0],pData[1]);
	pCtrlInfo->DCO=pData[infolen+2];
	CPSecondary::DoYTReq();
	
	return TRUE;
}
BOOL CGB101S::RecYTFSet(void)
{	
	BYTE * pData = &pReceiveFrame->Frame68.Data[m_dwasdu.Infooff];
	WORD  DCO;	//遥调命令限定词
	WORD  YTNo; //遥调路号
	BYTE infolen;
	infolen = m_guiyuepara.infoaddlen;
	YTNo=m_dwasdu.Info;
	DCO=pData[infolen+4];
	GotoEqpbyAddress(m_dwasdu.Address);
	SetEqpFlag(EQPFLAG_CLOCK);
	VYTInfo *pYTInfo;
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	if((YTNo<ADDR_YT_LO)||(YTNo>ADDR_YT_HI))
	{
		m_errflag=7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}

	if(GotoEqpbyAddress(m_dwasdu.Address)==false)
	{
		pYTInfo->Info.byStatus=1;
		return TRUE;
	}
	YTNo = YTNo - ADDR_YT_LO + 1;
	
	pYTInfo->Head.wEqpID = m_wEqpID;
	pYTInfo->Info.wID = YTNo;
	switch (DCO & 0x80)
	{
		case 0x80:
			pYTInfo->Head.byMsgID = MI_YTSELECT;
			break;
		case 0x00:
			pYTInfo->Head.byMsgID = MI_YTOPRATE;
			break;
		default:
			break;	
	}
	if((m_dwasdu.COT&0xf)==8)
	{
		pYTInfo->Head.byMsgID = MI_YTCANCEL;
	}
	pYTInfo->Info.dwValue=MAKEDWORD(MAKEWORD(pData[infolen], pData[infolen+1]), MAKEWORD(pData[infolen+2], pData[infolen+3]));
	
    VCtrlInfo *pCtrlInfo = (VCtrlInfo *)pGetEqpInfo(m_wEqpNo)->pProtocolData;
	pCtrlInfo->Type=m_dwasdu.TypeID;
	pCtrlInfo->DOT=MAKEWORD(pData[0],pData[1]);
	pCtrlInfo->DCO=pData[infolen+4];
	CPSecondary::DoYTReq();
	
	return TRUE;
}

BOOL CGB101S::DoYTRet(void)
{
#if 0
	BYTE Style, Reason = 7;
	BYTE PRM = 0, dwCode = 8, Num = 1;
	
	WORD YTNo;
	WORD YTValue;
	WORD *pwWord = NULL;

	pwWord = (WORD *)m_pMsg->abyData;	
	YTNo = *pwWord;
	pwWord++;
	YTValue = *pwWord;
		
	#if (TYPE_USER == USER_ERDS)
	YTValue = (YTValue-4000)  * 32767  / 16000;
	#endif
	Style = 0x30;
	SendFrameHead(Style, Reason);
	YTNo = YTNo + 0x0B81 -1;
	
	write_infoadd(YTNo);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(YTValue);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(YTValue);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 0;//QOS

	SendFrameTail(PRM, dwCode, Num);  
	#if (TYPE_USER == USER_ERDS)
	myprintf(m_wTaskID,LOG_ATTR_INFO,"Send YT Return,Value=%d\n",YTValue);
	#endif
	return TRUE;
#endif
	CPSecondary::DoYTRet();
	if (!CheckClearEqpFlag(EQPFLAG_YTRet))
		return FALSE;  //防止遥控返校信息回来时已切换到别的模块
	m_YTflag=1;
	return TRUE;

}
void CGB101S::initpara(void)
{
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
	}
	#if 0
	sprintf(tt,"gb101asdu len define: len:%d,type:%d,VSQ:%d,COT:%d,Cadd:%d,Iadd:%d,mode:%d,baseyear:%d",\
					m_guiyuepara.linkaddrlen,m_guiyuepara.typeidlen,m_guiyuepara.VSQlen,m_guiyuepara.COTlen,\
					m_guiyuepara.conaddrlen,m_guiyuepara.infoaddlen,m_guiyuepara.mode,m_guiyuepara.baseyear);
			myprintf(m_wTaskID,LOG_ATTR_INFO, tt);

	#endif
}
void CGB101S::getasdu(void)
{	
	BYTE off=0;
	if(m_guiyuepara.linkaddrlen==1)
	{
		m_dwasdu.LinkAddr=pReceiveFrame->Frame68.Data[off++];
	}
	if(m_guiyuepara.linkaddrlen==2)
	{
		m_dwasdu.LinkAddr=MAKEWORD(pReceiveFrame->Frame68.Data[off],pReceiveFrame->Frame68.Data[off+1]);
		off+=2;
	}
	if(m_guiyuepara.typeidlen==1)
	{
		m_dwasdu.TypeID=pReceiveFrame->Frame68.Data[off++];
	}
	if(m_guiyuepara.typeidlen==2)
	{
		m_dwasdu.TypeID=MAKEWORD(pReceiveFrame->Frame68.Data[off],pReceiveFrame->Frame68.Data[off+1]);
		off+=2;
	}
	if(m_guiyuepara.VSQlen==1)
	{
		m_dwasdu.VSQ=pReceiveFrame->Frame68.Data[off++];
	}
	if(m_guiyuepara.VSQlen==2)
	{
		m_dwasdu.VSQ=MAKEWORD(pReceiveFrame->Frame68.Data[off],pReceiveFrame->Frame68.Data[off+1]);
		off+=2;
	}
	m_dwasdu.COToff=off;
	
	if(m_guiyuepara.COTlen==1)
	{
		m_dwasdu.COT=pReceiveFrame->Frame68.Data[off++];
	}
	if(m_guiyuepara.COTlen==2)
	{
		m_dwasdu.COT=pReceiveFrame->Frame68.Data[off++];
		m_sourfaaddr=pReceiveFrame->Frame68.Data[off++];
	}
	if(m_guiyuepara.conaddrlen==1)
	{
		m_dwasdu.Address=pReceiveFrame->Frame68.Data[off++];
	}
	if(m_guiyuepara.conaddrlen==2)
	{
		m_dwasdu.Address=MAKEWORD(pReceiveFrame->Frame68.Data[off],pReceiveFrame->Frame68.Data[off+1]);
		off+=2;
	}
	m_dwasdu.Infooff=off;
	if(m_guiyuepara.infoaddlen==1)
	{
		m_dwasdu.Info=pReceiveFrame->Frame68.Data[off++];
	}
	if(m_guiyuepara.infoaddlen==2)
	{
		m_dwasdu.Info=MAKEWORD(pReceiveFrame->Frame68.Data[off],pReceiveFrame->Frame68.Data[off+1]);
		off+=2;
	}
	if(m_guiyuepara.infoaddlen==3)
	{
		m_dwasdu.Info=MAKEWORD(pReceiveFrame->Frame68.Data[off+1], pReceiveFrame->Frame68.Data[off+2]);
		m_dwasdu.Info<<=8;
		m_dwasdu.Info|=pReceiveFrame->Frame68.Data[off];
		off+=3;
	}
}
void CGB101S::write_linkaddr(int  data)
{
	m_SendBuf.wWritePtr=5;
	for(BYTE i=0;i<m_guiyuepara.linkaddrlen;i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
	}

}
void CGB101S::write_10linkaddr(int  data)
{
	m_SendBuf.wWritePtr=2;
	for(BYTE i=0;i<m_guiyuepara.linkaddrlen;i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
	}

}

void CGB101S::write_typeid(int  data)
{
	m_SendBuf.wWritePtr=5+m_guiyuepara.linkaddrlen;
	for(BYTE i=0;i<m_guiyuepara.typeidlen;i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
	}

}
void CGB101S::write_VSQ(int  data)
{
	for(BYTE i=0;i<m_guiyuepara.VSQlen;i++)
	{
		m_SendBuf.pBuf[ i+5+m_guiyuepara.linkaddrlen+m_guiyuepara.typeidlen ]=(data>>(i*8))&0xff;
	}

}
void CGB101S::write_COT(int  data)
{
	m_SendBuf.wWritePtr=5+m_guiyuepara.linkaddrlen+m_guiyuepara.typeidlen+m_guiyuepara.VSQlen;
		
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr+0 ]=(data)&0xff;

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr+1]=0;

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr+2]=0;
	m_SendBuf.wWritePtr+=m_guiyuepara.COTlen;
		

}
void CGB101S::write_conaddr(int  data)
{
	m_SendBuf.wWritePtr=5+m_guiyuepara.linkaddrlen+m_guiyuepara.typeidlen+m_guiyuepara.VSQlen+m_guiyuepara.COTlen;
	for(BYTE i=0;i<m_guiyuepara.conaddrlen;i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
	}

}
void CGB101S::write_infoadd(int  data)
{
	for(BYTE i=0;i<m_guiyuepara.infoaddlen;i++)
	{
		m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(data>>(i*8))&0xff;
	}
}

void CGB101S::CheckCfg()
{
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

	if((m_guiyuepara.baseyear!=2000)&&(m_guiyuepara.baseyear!=1900))
		m_guiyuepara.baseyear=2000;

}
void CGB101S::SetDefCfg()
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
 BYTE CGB101S::QDS(BYTE data)
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
		return tt&YC_YX_FLAG;	
 }
 
 BYTE CGB101S::SIQ(BYTE data)
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
	return tt&YC_YX_FLAG;	
 }
 BYTE CGB101S::DIQ(BYTE data1,BYTE data2)
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
//		if((data1&0x80)==(data2&0x80))
//		tt|=0x80;
	return tt&YC_YX_FLAG;	
 }

void CGB101S::write_time()
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
void CGB101S::write_time3()
{
	struct VSysClock clock;	
	GetSysClock(&clock,SYSCLOCK);
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(clock.wMSecond+clock.bySecond*1000)&0xff;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=(clock.wMSecond+clock.bySecond*1000)>>8;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ]=clock.byMinute;
}

int CGB101S::Geteqpparaaddr(WORD addr)
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

void CGB101S::copydata(WORD len1,WORD len2,BYTE *p1,BYTE* p2)
{
 	WORD len=len1;
 	if((p1==0)||(p2==0)) return ;
	if(len>len2) len=len2;    //取小的
	memcpy(p2,p1,len*sizeof(Vlimitpara));
}

void CGB101S::Setpara(void)
{
    BYTE * pData = pReceiveFrame->Frame68.Data+m_dwasdu.Infooff;
	WORD  VAL;	//遥控命令限定词
	WORD  YkNo; //遥控路号
	BYTE QPM;
	YkNo = MAKEWORD(pData[0], pData[1]);
	VAL =MAKEWORD( pData[m_guiyuepara.infoaddlen],pData[m_guiyuepara.infoaddlen+1]);
	QPM=pData[m_guiyuepara.infoaddlen+2]&0x3f;
	GotoEqpbyAddress(m_dwasdu.Address);
	int i=Geteqpparaaddr(GetEqpOwnAddr());
	if((m_dwasdu.Info<ADDR_YCPARA_LO)||(m_dwasdu.Info>ADDR_YCPARA_HI))
		return;
	if((QPM>0)&&(QPM<5))
		pData=(BYTE*)limitpara+(limitpara[1+i*3+2]+(YkNo-ADDR_YCPARA_LO)*sizeof(Vlimitpara));
	pData+=(QPM-1)*2;
	pData[1]=VAL&0xff;
	pData[0]=VAL>>8;
	return;
}

void CGB101S::Setycpara(void)
{
    BYTE * pData = pReceiveFrame->Frame68.Data+m_dwasdu.Infooff;
	WORD  VAL;	//遥控命令限定词
	WORD  YkNo; //遥控路号
	BYTE QPM;
	YkNo = MAKEWORD(pData[0], pData[1]);
	VAL =MAKEWORD( pData[m_guiyuepara.infoaddlen],pData[m_guiyuepara.infoaddlen+1]);
	QPM=pData[m_guiyuepara.infoaddlen+2]&0x3f;
	GotoEqpbyAddress(m_dwasdu.Address);
	int i=Geteqpparaaddr(GetEqpOwnAddr());
	if((m_dwasdu.Info<ADDR_YCPARA_LO)||(m_dwasdu.Info>ADDR_YCPARA_HI))
		return;
	if((QPM>0)&&(QPM<5))
		pData=(BYTE*)limitpara+(limitpara[1+i*3+2]+(YkNo-ADDR_YCPARA_LO)*sizeof(Vlimitpara));
	pData+=(QPM-1)*2;
	pData[1]=VAL&0xff;
	pData[0]=VAL>>8;
	return;
}

void CGB101S::writepara()
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

BOOL CGB101S::Sendpara(void)
{
	BYTE* YcValue;
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
	SwitchToEqpNo(m_readparaeqpnum);
	SendFrameHead(P_ME_NA, 34);
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
	
	SendFrameTail(1, 3, i|0x80);	
	return TRUE;
}

void CGB101S::DoReadData(void)
{
	WORD wDataAddr;
	
	wDataAddr = m_dwasdu.Info;
	
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
void CGB101S::SendReadYx(WORD wYxNo)
{
	BYTE TypeID = M_SP_NA;
	BYTE Qualifier = 0x1;
	
	WORD ReadYxNum = 1;
	BYTE YxValue;
		
	ReadRangeSYX(m_wEqpID, wYxNo, ReadYxNum, 1, &YxValue);
		
	SendFrameHead(TypeID,m_dwasdu.COT);
	write_infoadd(wYxNo + ADDR_YX_LO);
	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SIQ(YxValue);
	write_VSQ(Qualifier);
	SendFrameTail(m_PRM, 3, Qualifier);	
	return;
}

void CGB101S::SendReadYc(WORD wYcNo)
{
	BYTE TypeID = M_ME_NA;
	BYTE Qualifier = 0x1;
	
	WORD ReadYcNum = 1;
	struct VYCF  YcValue;
	
	ReadRangeYCF(m_wEqpID, wYcNo, ReadYcNum, sizeof(struct VYCF), &YcValue);
	
	SendFrameHead(TypeID, m_dwasdu.COT);
	write_infoadd(wYcNo+ADDR_YC_LO);

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(YcValue.nValue);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(YcValue.nValue);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = QDS(YcValue.byFlag);
	write_VSQ(Qualifier);
	SendFrameTail(m_PRM, 3, Qualifier);	
	return;
}
void CGB101S::SendReadYcpara(WORD wYcNo)
{
	BYTE TypeID = P_ME_NA;
	BYTE Qualifier = 0x4;
	BYTE*  YcValue;
	
	SendFrameHead(TypeID, m_dwasdu.COT);
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
	SendFrameTail(m_PRM, 3, Qualifier);	
	return;
}

void CGB101S::SendReadDd(WORD wDdNo)
{
	BYTE TypeID = M_IT_NA;
	BYTE Reason = COT_REQ;
	BYTE Qualifier = 0x1;
	
	WORD ReadDdNum = 1;
	DWORD DdValue;
	
	ReadRangeDD(m_wEqpID, wDdNo, ReadDdNum, 4/*sizeof(DWORD)*/, (long *)&DdValue);
	
	SendFrameHead(TypeID,Reason);
		
	write_infoadd(wDdNo + ADDR_DD_LO);
		
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(LOWORD(DdValue));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(LOWORD(DdValue));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(HIWORD(DdValue));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(HIWORD(DdValue));
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
	
	SendFrameTail(m_PRM, 3, Qualifier);	
	return;
}

BOOL CGB101S::CycleSendAllYcYx(BYTE reason)
{
	WORD i, YxGroupNum, YcGroupNum;
	
	SwitchToEqpNo(m_dwSendAllDataEqpNo);
	
	YxGroupNum = (m_pEqpInfo[m_wEqpNo].wSYXNum +m_pEqpInfo[m_wEqpNo].wVYXNum+ GRP_YXNUM- 1) / GRP_YXNUM;
	YcGroupNum = (m_pEqpInfo[m_wEqpNo].wYCNum + GRP_YCNUM - 1) / GRP_YCNUM;
	
	for(i=0; (i<31) && (i<YcGroupNum); i++)
	{
		if( m_dwcycYcSendFlag & (1<<i) )
		{
			if(SendYCGroup(i, reason))
				return true;
			m_wSendYcNum = 0;
			m_dwcycYcSendFlag &= ~(1<<i);
			
		}
	}
	for(i=0; (i<31) && (i<YxGroupNum); i++)
	{
		if( m_dwcycYxSendFlag & (1<<i) )
		{
			if(SendDYXGroup(i, reason))
				return true;
			if(SendYXGroup(i, reason))
				return true;
			m_wSendDYxNum=0;
			m_wSendYxNum=0;
			m_dwcycYxSendFlag &= ~(1<<i);
		}
	}
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
			return false;
		}
		else
		{
			
			m_dwcycYxSendFlag = 0xffffffff;
			m_dwcycYcSendFlag = 0xffffffff;	
			return false;
		}
	}
	return false;

}

int CGB101S::ReadPara(WORD addr, char* pbuf)
{
	WORD type;
	type = m_guiyuepara.tmp[7];

	if((addr >= SELFPARA_ADDR) && (addr < RUNPARA_ADDR))
	{
		if(m_guiyuepara.tmp[8] == 0)
			return ReadSelfParaGb2312(addr,pbuf);	
		else
			return ReadSelfPara(addr,pbuf);
	}
	else if(((addr >= RUNPARA_ADDR) && (addr < PRPARA_ADDR))
#if(DEV_SP  == DEV_SP_TTU)
	||((addr >=(PRPARA_TTU_ADDR + PRP_HAR_DEAD_V))&&(addr <=(PRPARA_TTU_ADDR + PRP_DEAD_LOAD)))
#endif
		)
		return GB101ReadRunPara(addr,pbuf);
#ifdef INCLUDE_PR
	else if((addr >= PRPARA_ADDR) && (addr <= PRPARA_LINE_ADDREND))
		return ReadPrPara(addr,pbuf,type);
#endif
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

int CGB101S::WriteParaYZ(WORD addr, char* pbuf) //参数预置
{
	if(((addr >= RUNPARA_ADDR) && (addr < PRPARA_ADDR))||
		((addr >=(PRPARA_TTU_ADDR + PRP_HAR_DEAD_V))&&(addr <=(PRPARA_TTU_ADDR + PRP_DEAD_LOAD))))
		return WriteRunParaYZ(addr,pbuf);
#ifdef INCLUDE_PR
	else if((addr >= PRPARA_ADDR) && (addr <= PRPARA_LINE_ADDREND))
		return WritePrParaYZ(addr,pbuf);
#endif
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
int CGB101S::WriteSwitchParaYZ(WORD addr, char* pbuf)
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
int CGB101S::WriteParaGH(WORD addr, char* pbuf) //参数固化
{
	WORD type;
	type = m_guiyuepara.tmp[7];

	if(((addr >= RUNPARA_ADDR) && (addr <  PRPARA_ADDR))||
		((addr >=(PRPARA_TTU_ADDR + PRP_HAR_DEAD_V))&&(addr <=(PRPARA_TTU_ADDR + PRP_DEAD_LOAD))))
		return GB101WriteRunPara(addr,pbuf);
#ifdef INCLUDE_PR
	else if((addr >= PRPARA_ADDR) && (addr <= PRPARA_LINE_ADDREND))
		return WritePrPara(addr,pbuf,type);
#endif
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

int CGB101S::GB101ReadRunPara(WORD parano,char* pbuf)
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

int CGB101S::GB101WriteRunPara(WORD parano,char* pbuf)
{
	
	
	if((parano < (RUNPARA_ADDR + RPR_DV_NUM))||
		((parano >=(PRPARA_TTU_ADDR + PRP_HAR_DEAD_V))&&(parano <=(PRPARA_TTU_ADDR + PRP_DEAD_LOAD))))		
		return WriteDeadValue(parano,pbuf);
	else 
		return WriteRunPara(parano,pbuf);
}
int CGB101S::GB101WriteRunParaFile()
{
	BOOL ret,deadvaluefind,runparafind;
	int i;
	struct VFileMsgs fmsg;
	
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


int CGB101S::ReadDeadValue(WORD parano,char*pbuf)
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

int CGB101S::WriteDeadValue(WORD parano,char*pbuf)
{
	struct VParaInfo *fParaInfo;
	float fvalue;
	WORD no;

	fParaInfo =  (struct VParaInfo*)pbuf;
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

int CGB101S::WriteDeadValueFile()
{
	int para_id;
	char fname[MAXFILENAME];
	VProtocolBaseCfg *pBaseCfg;
	
	para_id = GetParaPortId(m_wTaskID);
	sprintf(fname,"portcfg%d.cfg",para_id);
	
	pBaseCfg = (VProtocolBaseCfg*)(g_pParafileTmp + 1);
	memset(pBaseCfg,0,sizeof(VProtocolBaseCfg) + sizeof(m_guiyuepara));
	
	memcpy(pBaseCfg,m_pBaseCfg,sizeof(VProtocolBaseCfg));
	memcpy(pBaseCfg + 1,&m_guiyuepara,sizeof(m_guiyuepara));
	
	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(VProtocolBaseCfg) + sizeof(m_guiyuepara);
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = CFG_ATTR_NULL;
	
	return (WriteParaFile(fname,g_pParafileTmp));
}

/*************************文件传输******************************/
BOOL CGB101S::DoFileData(void)   //处理文件命令
{
	BYTE * pData = &pReceiveFrame->Frame68.Data[m_dwasdu.Infooff]; //信息对象地址
	BYTE ActID;
	
	if((m_dwasdu.COT != 5 ) && (m_dwasdu.COT != 6 ))
	{
		m_errflag = 0x5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}
	if(m_dwasdu.Info!=0)
	{
		m_errflag=7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return true;
	}

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
	
	return TRUE;

}

BOOL CGB101S::DoFileDir(void) 
{
	VSysClock SysTime;
	WORD MSecond;
	WORD DirNum;
	VFileOPMsg FileOPMsg;
	VDirInfo *pDirInfo = NULL;
	BYTE * pData = pReceiveFrame->Frame68.Data+m_dwasdu.Infooff+m_guiyuepara.infoaddlen; 
	BYTE *p;
	char tempname[100],*h;
	BYTE temp[7];
	DWORD SecRcdTime;
	int i,j;

	memset(&FileDirInfo, 0, sizeof(VFileDirInfo));
	m_fileoffset = 0;

	FileDirInfo.dirid = (MAKEWORD(pData[4],pData[5])<<16)|MAKEWORD(pData[2],pData[3]);
	FileDirInfo.dirnamelen = pData[6];

	if(FileDirInfo.dirnamelen==0)
	{
		switch(m_guiyuepara.tmp[1])  //文件
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
		memcpy(FileDirInfo.dirname, &pData[7], FileDirInfo.dirnamelen);
		FileDirInfo.dirname[FileDirInfo.dirnamelen] = '\0';
		strcpy(tempname,"/tffsb/");	
		strcat(tempname,FileDirInfo.dirname);
		memset(FileDirInfo.dirname,0,sizeof(FileDirInfo.dirname));
		strcpy(FileDirInfo.dirname,tempname);
		
	}

	FileDirInfo.flag = pData[7+FileDirInfo.dirnamelen];

	p = pData+8+FileDirInfo.dirnamelen; //查询起始时间

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

	p = pData+15+FileDirInfo.dirnamelen; //查询终止时间

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
	
	//减少读目录的次数，非录波的只能读53个文件
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
			if(h != NULL) // 录波用文件名中的时间比较
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
	else //召唤标志只有0： 所有文件 1：按时间搜索  其他无效
		m_calldir = 0;
	
	return TRUE;
}

BOOL CGB101S::SendFileDir(void)   //发送文件目录
{
	BYTE Style = F_FR_NA_1, Reason = COT_REQ;
	BYTE PRM = 0, dwCode = 3;
	struct VSysClock SysTime;
	FileStatus filestatus;
	WORD DirNum;
	VDirInfo *pDirInfo = NULL;
	WORD ch;
	char *p;
	BYTE timebuf[7];
	 
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

		SendFrameHead(Style, Reason);
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
			SendFrameTail(PRM, dwCode, 0);
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
			for(int i = 0; i < ch; i++,pDirInfo++)
			{
				pDirInfo = pVDirInfo + DirOffSet[m_fileoffset + i];
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

				/*文件属性 自定义: 1目录 2文件*/ 
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
		SendFrameTail(PRM, dwCode, ch);
		m_fileoffset += ch;
		
	return TRUE;
		
}

BOOL CGB101S::DoReadFile(void)
{
	BYTE * pData = pReceiveFrame->Frame68.Data+m_dwasdu.Infooff+m_guiyuepara.infoaddlen; 
	char tempname[100];
	
	if(pData[1] == 3)  //  读文件激活
	{
		memset(&ReadFileInfo, 0, sizeof(VFileInfo));
		ReadFileInfo.datano = 0;
		ReadFileInfo.filenamelen = pData[2];
		memcpy(ReadFileInfo.filename, &pData[3], ReadFileInfo.filenamelen);
		ReadFileInfo.filename[ReadFileInfo.filenamelen] = '\0';

		memset(tempname,0,sizeof(tempname));
		if(ReadFileInfo.filename[0] == 'B' ) 
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
		else if((ReadFileInfo.filename[0] == 'e') && (ReadFileInfo.filename[1] == 'x')) // ???μ
		{
			strcpy(tempname, "/tffsb/HISTORY/EXV/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'f') && (ReadFileInfo.filename[1] == 'i')) //?¨μ?
		{
			strcpy(tempname, "/tffsb/HISTORY/FIXPT/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'f') && (ReadFileInfo.filename[1] == 'r')) // è??3?á
		{
			strcpy(tempname, "/tffsb/HISTORY/FRZ/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'f') && (ReadFileInfo.filename[1] == 'l')) // 1|?ê·′?ò
		{
			strcpy(tempname, "/tffsb/HISTORY/FLOWREV/");
			strcat(tempname, ReadFileInfo.filename);
		}
		else if((ReadFileInfo.filename[0] == 'u') && (ReadFileInfo.filename[1] == 'l')) //è???
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
			return FALSE;
		}
		memcpy(ReadFileInfo.filename,tempname,strlen(tempname));
	
		m_byReadFileAckFlag = 1;
		
	}

	if(pData[1] == 6)  //  读文件数据传输确认
	{
		/*if(pData[9])  //有后续
			m_byReadFileFlag = 1;
		else
			m_byReadFileFlag = 0;*/
		m_byReadFileFlag = 0;
	}
	
	return TRUE;

}

BOOL CGB101S::SendReadFileAck(void)
{
	BYTE Style = F_FR_NA_1, Reason = COT_ACTCON;
	BYTE PRM = 0, dwCode = 3;
	struct VFileOPMsg FileOPMsg;
	FileStatus filestatus;
	char *ptr=NULL;

	strcpy((char *)FileOPMsg.cFileName, (char *)ReadFileInfo.filename);
	FileOPMsg.dwOffset = 0;
	FileOPMsg.dwLen = 100;
	FileOPMsg.dwSize = 0;

	filestatus = ReadFile(&FileOPMsg, (BYTE *)&m_dwPubBuf);
	
	SendFrameHead(Style, Reason);
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
	ptr = strrchr(ReadFileInfo.filename, '/');
	if(ptr != NULL)
		memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (ptr+1),strlen(ReadFileInfo.filename));
	else
		memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], ReadFileInfo.filename,strlen(ReadFileInfo.filename));

	m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + ReadFileInfo.filenamelen;

	/*文件标识*/
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;   
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;

	/*文件大小*/
	if (filestatus != FileOk)
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
	
	SendFrameTail(PRM, dwCode, 1);
		
	return TRUE;	
}

BOOL CGB101S::SendReadFileData(void)
{
	BYTE Style = F_FR_NA_1, Reason = COT_REQ;
	BYTE PRM = 0, dwCode = 3;
	FileStatus filestatus;
	VFileOPMsg FileOPMsg;	
	BYTE CrcSum;
	
	memset(FileOPMsg.cFileName,0,sizeof(FileOPMsg.cFileName));
	memcpy(FileOPMsg.cFileName,ReadFileInfo.filename,strlen(ReadFileInfo.filename));
	
	FileOPMsg.dwSize = 0;
	FileOPMsg.dwOffset = ReadFileInfo.datano;
	FileOPMsg.dwLen = MAX101LEN;

	SendFrameHead(Style, Reason);
	write_infoadd(0);

	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;  //附加数据包类型
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 5;  //操作标识

	/*文件标识*/
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;   
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
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 1; //有后续
	
		CrcSum = ChkSum(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], FileOPMsg.dwLen);  
		m_SendBuf.wWritePtr += FileOPMsg.dwLen;
		ReadFileInfo.datano += FileOPMsg.dwLen;

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = CrcSum; //校验和
		m_byReadFileFlag = 1;
		
	}
	else if(filestatus == FILEEOF)
	{
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0; //无后续

		CrcSum = ChkSum(&m_SendBuf.pBuf[m_SendBuf.wWritePtr], FileOPMsg.dwLen);  
		m_SendBuf.wWritePtr += FileOPMsg.dwLen;
		ReadFileInfo.datano += FileOPMsg.dwLen;

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = CrcSum; //校验和
		m_byReadFileFlag = 0;
	}
	else
	{
		m_errflag = 7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return FALSE;
	}

	SendFrameTail(PRM, dwCode, 1);

	return TRUE;

}

BOOL CGB101S::DoWriteFile(void)
{
	BYTE * pData = pReceiveFrame->Frame68.Data+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;
	BYTE *p,crc1,crc2;
	DWORD length;
	struct VFileOPMsg FileOPMsg;
	static DWORD off=0;
	static DWORD wrptr=0;
	VFileMsgs fmsg;
	
	if(pData[1] == 7)    //写文件激活
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
		m_byWriteFileAckFlag = 1;
	}

	if(pData[1] == 9)    //写文件数据传输
	{
		p = &pData[6];

		length = pReceiveFrame->Frame68.Length1-m_guiyuepara.linkaddrlen-m_guiyuepara.typeidlen-m_guiyuepara.VSQlen-m_guiyuepara.COTlen-m_guiyuepara.conaddrlen-m_guiyuepara.infoaddlen-13;
		crc1 = *(p+length+5);
		crc2=(BYTE)ChkSum((BYTE *)(p+5), length);

		if(crc1 != crc2)
		{
			WriteFileInfo.flag |= 0x01 << 2;
			
		}
		else
			WriteFileInfo.flag |= 0x01 << 0;

		memcpy(filetemp+wrptr,(BYTE*)(p+5),length);
		wrptr = wrptr + length;
		
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
		if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
			MD5_Update((BYTE*)(p+5),length);
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

		if(!p[4]) //无后续
			m_byWriteFileDataAckFlag = 1;
		
	}
	
	return TRUE;

}

BOOL CGB101S::SendWriteFileAck(void)  //发送写文件确认
{
	BYTE Style = F_FR_NA_1, Reason;
	BYTE PRM = 0, dwCode = 3;
	char tempname[100]="/tffsa/other/";

	if(m_byWriteFileAckFlag == 1) //写文件激活确认
	{
		Reason = COT_ACTCON;
		SendFrameHead(Style, Reason);
		write_infoadd(0);

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;  //附加数据包类型
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 8;  //操作标识

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;  //结果描述字

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

		SendFrameTail(PRM, dwCode, 0);

		strcat(tempname,WriteFileInfo.filename);
		memset(WriteFileInfo.filename, 0, strlen(WriteFileInfo.filename));
		memcpy(WriteFileInfo.filename,tempname,strlen(tempname));
		
	}

	if(m_byWriteFileDataAckFlag)  // 写文件数据传输确认
	{
		Reason = COT_REQ;
		SendFrameHead(Style, Reason);
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

		/*结果描述字*/
		if(WriteFileInfo.flag & (1<<0))
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;  //成功  
		else if((WriteFileInfo.flag & (1<<2)) == 0)
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 2;  //校验和错误
		else
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 1;  //未知错误
		
		WriteFileInfo.flag = 0;

		SendFrameTail(PRM, dwCode, 0);
		
	}

	return TRUE;
	
}

void CGB101S::GetRcdNametime(char *str, BYTE *buf) // 通过录波文件名获取时间
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


/***********************录波文件传输 END***********************************/

BOOL CGB101S::DoSwitchValNo(void)   //切换定值区号
{
	if(m_dwasdu.VSQ!=1)
	{
		m_errflag=0x4;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}
	if(m_dwasdu.COT != 6 )
	{
		m_errflag = 0x5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
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
	  	if(m_jiamiType != 0)//湖南版本这里不加密且0x00
	  	{
	  		SecuritySend1FFrame_HN(m_SendBuf.pBuf,&m_SendBuf.wWritePtr);
	  		m_SendBuf.wReadPtr = 0;
			SendAllFrame();
	  		return FALSE;
	  	}
	}
#endif

	memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
	m_bySwitchValNoFlag = 1;
	
	return TRUE;

}

BOOL CGB101S::SendSwitchValNoAck(void)
{
	BYTE PRM = 0, dwCode = 3;
	
	PRM = 0;dwCode = 8;
	m_SendBuf.wReadPtr = 0;
	m_SendBuf.wWritePtr = 0;
	if(m_tdata[0]!=0x68)
	{
		m_errflag=0;
		return false;
	}
	
	if(m_dwasdu.COT == COT_ACT)
	{
		m_tdata[m_dwasdu.COToff+5] = COT_ACTCON;
	}
	else
	{
		m_tdata[0] = 0;
		return FALSE;
	}
	
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_tdata,m_tdata[1]+4);
	m_SendBuf.wWritePtr += m_tdata[1]+4;

	SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);

	return TRUE;
}


BOOL CGB101S::DoReadValNo(void)   //读定值区号
{	
	if(m_dwasdu.VSQ != 1)
	{
		m_errflag = 4;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return FALSE;
	}
	if(m_dwasdu.COT != 6)
	{
		m_errflag = 5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return FALSE;
	}
	if(m_dwasdu.Info != 0)
	{
		m_errflag = 7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return FALSE;
	}

	m_byReadValNoFlag = 1;
	
	return TRUE;

}

BOOL CGB101S::SendReadValNoAck(void)  //发送读定值区号
{
	BYTE Style = C_RR_NA_1, Reason = COT_ACTCON;
	BYTE PRM = 0, dwCode = 3;
	WORD SN1,SN2,SN3;
	SN1 = 0; SN2 = 0;SN3 = 0;
	
#if defined(INCLUDE_SM)||defined(INCLUDE_ZZ)
	SN1 = 1; SN2 = 0;SN3 = 8;
#endif

	SendFrameHead(Style, Reason);
	write_infoadd(0); 

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SN1); //定值区号SN1
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SN1);

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SN2); //定值区号SN2
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SN2);

	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SN3);  //定值区号SN3
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SN3);
	
	SendFrameTail(PRM, dwCode, 1);

	return TRUE;


}

BOOL CGB101S::DoReadParaVal(void)  //读参数定值
{
#if (TYPE_CPU == CPU_SAM9X25)
	BYTE *pdata;
	int i=0;
	DWORD a,infoaddr;
	int num = 0;
#endif
	nh = 1;
	readcnt = 0;

	if(m_dwasdu.COT != 6)
	{
		m_errflag = 5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return FALSE;
	}
	if(m_dwasdu.VSQ == 0)
	{
		m_byAllParaValFlag = 1;
		m_paraoffset = SELFPARA_ADDR;
#ifdef INCLUDE_SM
		m_paraoffset = SELFPARA_ADDR_SM;
#endif
		if(m_fdno>0)
		{
			ParaMsg.Head.wLength = NEXTPARALEN+sizeof(VMsgHead);
			ParaMsg.Head.byMsgID = MI_PARA;
			ParaMsg.Head.byMsgAttr = 3;//0-预置，1-执行，2-取消，3-读
			ParaMsg.abyData[0] = 0;//0-读全部参数，num-读对应num个参数
			msgSend(m_w104mTaskID, &ParaMsg, ParaMsg.Head.wLength, 1);
		}
	}
	else
	{
		m_byMoreParaValFlag = 1;
		m_paraoffset = 0;
		m_paraaddrnum = m_dwasdu.VSQ&0x7F;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
#if (TYPE_CPU == CPU_SAM9X25)		
		pdata = &m_tdata[m_dwasdu.Infooff+7];
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
				infoaddr = MAKEDWORD(MAKEWORD(pdata[a],pdata[a+1]),pdata[a+2]);
			}
			if(infoaddr>=set_infoaddr)
			{
				ReadNextParaInfo[num].infoaddr = infoaddr;
				num++;
			}
		}
		if((m_fdno>0)&&(num>0))
		{
			ParaMsg.Head.wLength = NEXTPARALEN+sizeof(VMsgHead);
			ParaMsg.Head.byMsgID = MI_PARA;
			ParaMsg.Head.byMsgAttr = 3;//0-预置，1-执行，2-取消，3-读
			ParaMsg.abyData[0] = num;//0-读全部参数，num-读对应num个参数
			msgSend(m_w104mTaskID, &ParaMsg, ParaMsg.Head.wLength, 1);
		}
#endif
	}

	return TRUE;

}

BOOL CGB101S::SendReadParaVal(void) //发送参数定值
{
	BYTE Style = C_RS_NA_1, Reason = COT_ACTCON;
	BYTE PRM = 0, dwCode = 3,pi=0,pipos,j;
	DWORD SN=0,i,a,k;
	int  state;
	DWORD infoaddr;
	BYTE *pdata;
	WORD  pLen=0;
	struct VParaInfo *pParaInfo;
	
#if defined(INCLUDE_SM)||defined(INCLUDE_ZZ)
    SN = 1;
#endif

	pParaInfo = (struct VParaInfo *)m_dwPubBuf;

	SendFrameHead(Style, Reason);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = LOBYTE(SN); //定值区号SN
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = HIBYTE(SN);
	pipos = m_SendBuf.wWritePtr;
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pi;    //参数特征标识
	
	if(m_byAllParaValFlag)  //读全部参数
	{	
		j = 0;
		if(m_fdno>0)
		{
#if (TYPE_CPU == CPU_SAM9X25)
			for(i = m_paraoffset; i <= PARA_END; i++)  //固有参数
			{
				if(i<set_infoaddr)
				{
					state = ReadPara(i, (char *)m_dwPubBuf);
				}
				else
				{
					if(nh == 0)
					{
						state = ERROR;
						for(k = 0;k<ReadParaNum;k++)
						{
							if(i==ReadNextParaInfo[k].infoaddr)
							{
								pParaInfo->type = ReadNextParaInfo[k].type;
								pParaInfo->len = ReadNextParaInfo[k].len;
								memcpy(&pParaInfo->valuech,&ReadNextParaInfo[k].valuech,ReadNextParaInfo[k].len);
								state = OK;
								break;
							}
						}
						if(state == ERROR)
							continue;
					}
					else
					{
						readcnt++;
						if(readcnt<100)
							return FALSE;
						else
							continue;
					}	
				}
				
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
#endif
		}
		else
		{
			for(i = m_paraoffset; i <= PARA_END; i++)  //固有参数
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
		}
		
		if(i >= PARA_END)
		{
			m_SendBuf.pBuf[ pipos ] = 0; // 无后续
			m_byAllParaValFlag = 0;
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
			return FALSE;
		}
		SendFrameTail(PRM, dwCode, j);
		
		return TRUE;

	}

	if(m_byMoreParaValFlag)	  //读多个参数
	{
		j = 0;
		pdata = &m_tdata[m_dwasdu.Infooff+7];
		if(m_fdno>0)
		{
#if (TYPE_CPU == CPU_SAM9X25)
			for(i = m_paraoffset; (i < m_paraaddrnum) &&  (j < 12); i++)
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
					infoaddr = MAKEDWORD(MAKEWORD(pdata[a],pdata[a+1]),pdata[a+2]);
				}
			
				if(infoaddr<set_infoaddr)
				{
					state = ReadPara(infoaddr, (char *)m_dwPubBuf);
				}
				else
				{
					if(nh == 0)
					{
						state = ERROR;
						for(k = 0;k<ReadParaNum;k++)
						{
							if(infoaddr==ReadNextParaInfo[k].infoaddr)
							{
								pParaInfo->type = ReadNextParaInfo[k].type;
								pParaInfo->len = ReadNextParaInfo[k].len;
								memcpy(&pParaInfo->valuech,&ReadNextParaInfo[k].valuech,ReadNextParaInfo[k].len);
								state = OK;
								break;
							}
						}
						if(state == ERROR)
							continue;
					}
					else
					{
						readcnt++;
						if(readcnt<100)
							return FALSE;
						else
							continue;
					}	
				}
				
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
					write_infoadd(infoaddr); 
					m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pParaInfo->type; // tag类型
					m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = pParaInfo->len; // 数据长度
					memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr, pParaInfo->valuech, pParaInfo->len);
					m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + pParaInfo->len;
				}
				else 
					continue;
			}
#endif
		}
		else
		{
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
					infoaddr = MAKEDWORD(MAKEWORD(pdata[a],pdata[a+1]),pdata[a+2]);

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
		}
		

		if(i >= m_paraaddrnum)
		{
			m_SendBuf.pBuf[ pipos ] = 0; // 无后续
			m_byMoreParaValFlag = 0;
		}
		else
			m_SendBuf.pBuf[ pipos ] = 1; // 有后续
			
		m_paraoffset = i;
		if((j == 0) && (state == 0x80))
		{		
			return FALSE;
		}
		if(!j)
		{
			m_errflag = 7;
			memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
			return FALSE;
		}
		else
		{
			SendFrameTail(PRM, dwCode, j);
			return TRUE;
		}

	}
	
	return FALSE;

}

BOOL CGB101S::DoWriteParaVal(void)    //写参数
{
	BYTE * pData = pReceiveFrame->Frame68.Data+m_dwasdu.Infooff;
	BYTE *p;
	BYTE pi;
	WORD ch=0;
    WORD num,i;
	DWORD infoaddr;
	WORD nextnum=0;
	int waitcnt = 0;
	BOOL luboyb = 0;
	pi = pData[2];
	writeflag = 1;
	if((m_dwasdu.COT != 6)&&(m_dwasdu.COT != 8))
	{
		m_errflag = 5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return FALSE;
	}
	
	if((m_dwasdu.COT == 6) && ((pi & 0x80) == 0x80) && ((pi & 0x40) == 0)) //预置
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
			if((m_fdno>0)&&(infoaddr>=set_infoaddr))
			{	
#if (TYPE_CPU == CPU_SAM9X25)
				WriteNextParaInfo[m_nextparanum].infoaddr = infoaddr-m_fdno*PRPARA_LINE_NUM;
				WriteNextParaInfo[m_nextparanum].type = *(p+m_guiyuepara.infoaddlen);
				WriteNextParaInfo[m_nextparanum].len = *(p+m_guiyuepara.infoaddlen+1);
				memcpy(WriteNextParaInfo[m_nextparanum].valuech, p+m_guiyuepara.infoaddlen+2, WriteNextParaInfo[m_nextparanum].len);

				p = p+m_guiyuepara.infoaddlen+2+WriteNextParaInfo[m_nextparanum].len;
				m_nextparanum++;
				nextnum++;
				if(i == num-1)
				{
					nextflag = 1;
					ParaMsg.Head.wLength = NEXTPARALEN+sizeof(VMsgHead);
					ParaMsg.Head.byMsgID = MI_PARA;
					ParaMsg.Head.byMsgAttr = 0;//0-预置，1-执行，2-取消
					ParaMsg.abyData[0] = nextnum;//当前帧参数个数
					ParaMsg.abyData[1] = m_nextparanum-nextnum;//当前参数位置
					msgSend(m_w104mTaskID, &ParaMsg, ParaMsg.Head.wLength, 1);
				}
#endif
			}
			else
			{
				WriteParaAddr[m_paranum] = infoaddr;
				WriteParaInfo[m_paranum].type = *(p+m_guiyuepara.infoaddlen);
				WriteParaInfo[m_paranum].len = *(p+m_guiyuepara.infoaddlen+1);
				memcpy(WriteParaInfo[m_paranum].valuech, p+m_guiyuepara.infoaddlen+2, WriteParaInfo[m_paranum].len);

				ch = WriteParaYZ(infoaddr,(char *)&WriteParaInfo[m_paranum]);
				if(ch != 0)
				{
					m_nextparanum = 0;
					m_paranum = 0;
					break;
				}
				p = p+m_guiyuepara.infoaddlen+2+WriteParaInfo[m_paranum].len;
				m_paranum++;
			}
			
		}
		
		if(nextnum == 0)
			writeflag = 0;
		
		while(1)
		{
			if(m_fdno>0)
			{
				
				thSleep(20);
				if((ch == 0)&&(writeflag == 0))
				{
					paracmdstatue = 1;
					m_byWriteParaStatus = 1;
					memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
					break;
				}
				else
				{
					waitcnt++;
					if(waitcnt>10)
					{
						m_errflag = 7;
						memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
						return FALSE;

					}
				}
			}
			else
			{
				if(ch == 0)
				{
					paracmdstatue = 1;
					m_byWriteParaStatus = 1;
					memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
					break;
				}
				else
				{	
					m_errflag = 7;
					memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
					return FALSE;
				}
			}
		}
	}
	else if((m_dwasdu.COT == 6) && ((pi & 0x80) == 0) && ((pi & 0x40) == 0)) //固化
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
			if((m_fdno>0)&&(nextflag == 1))
			{
				ParaMsg.Head.wLength = NEXTPARALEN+sizeof(VMsgHead);
				ParaMsg.Head.byMsgID = MI_PARA;
				ParaMsg.Head.byMsgAttr = 1;//0-预置，1-执行，2-取消,3-读
				msgSend(m_w104mTaskID, &ParaMsg, ParaMsg.Head.wLength, 1);

			}
			
			for(i = 0; i < m_paranum; i++)
			{
				ch = WriteParaGH(WriteParaAddr[i],(char *)&WriteParaInfo[i]);
				if(ch != 0)
					break;
			}
			m_WriteNum = i;
			if(nextflag == 0)
				writeflag = 0;
			while(1)
			{
				if(m_fdno>0)
				{
					thSleep(20);
					if((ch == 0)&&(writeflag == 0))
					{
						paracmdstatue = 0;
						m_byWriteParaStatus = 2;
						memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);

						GB101WriteRunParaFile();
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
						m_nextparanum = 0;
						break;
					}
					else
					{
						waitcnt++;
						if(waitcnt>10)
						{
							m_errflag = 7;
							memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
							return FALSE;

						}
					}
				}
				else
				{
					if(ch == 0)
					{
						paracmdstatue = 0;
						m_byWriteParaStatus = 2;
						memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);

						GB101WriteRunParaFile();
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
						break;
					}
					else
					{
						m_errflag = 7;
						memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
						return FALSE;
					}
				}
			}
		}
		else
		{
			m_errflag = 7;
			memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
			return FALSE;
		}

	}
	else if((m_dwasdu.COT == 8) && ((pi & 0x80) == 0) && ((pi & 0x40) == 0x40)) //取消预置
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

		if(m_fdno>0)
		{
			nextflag = 0;
			ParaMsg.Head.wLength = NEXTPARALEN+sizeof(VMsgHead);
			ParaMsg.Head.byMsgID = MI_PARA;
			ParaMsg.Head.byMsgAttr = 2;//0-预置，1-执行，2-取消,3-读
			msgSend(m_w104mTaskID, &ParaMsg, ParaMsg.Head.wLength, 1);

		}
		if(paracmdstatue == 1)
		{
			paracmdstatue = 0;
			m_byWriteParaStatus = 3;
			m_paranum = 0;
			m_nextparanum = 0;
			memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		}
		else
		{
			m_errflag = 7;
			memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL CGB101S::SendWriteParaValAck(void)
{
	BYTE PRM=0, dwCode=8;

	m_SendBuf.wReadPtr = 0;
	m_SendBuf.wWritePtr = 0;
	if(m_tdata[0] != 0x68)
	{
		m_errflag=0;
		return false;
	}
	
	if(m_dwasdu.COT == COT_ACT)
	{
		m_tdata[m_dwasdu.COToff+5] = COT_ACTCON;
	}
	else if(m_dwasdu.COT == COT_DEACT)
	{
		m_tdata[m_dwasdu.COToff+5] = COT_DEACTCON;
	}
	else
	{
		m_tdata[0] = 0;
	}
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_tdata,m_tdata[1]+4);
	m_SendBuf.wWritePtr += m_tdata[1]+4;
		
#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if(3 == m_guiyuepara.jiami)
	{
		if((1 == paracmdstatue)&&(m_dwasdu.COT == COT_ACT))//预置
		{
#ifdef INCLUDE_SEC_CHIP
			if(m_wCommID == COMM_IPSEC_NO)
				SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);//撤销确认，南网为00
			else
#endif
				SendFrameTail0x2(PRM, dwCode, m_dwasdu.VSQ);
		}
		else if(m_dwasdu.COT == COT_DEACT)//撤销
			SendFrameTail0x2(PRM, dwCode, m_dwasdu.VSQ);
		else//固话回复
			SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);
	}
	else if(4 == m_guiyuepara.jiami)
	{//ly
		if((1 == paracmdstatue)&&(m_dwasdu.COT == COT_ACT))//预置
			SendFrameTail0x2(PRM, dwCode, m_dwasdu.VSQ);
		else if(m_dwasdu.COT == COT_DEACT)//撤销
			SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);
		else//固话回复
			SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);
	}
	else
#endif
	SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);
	
	return TRUE;

}


/*软件升级*/
BOOL CGB101S::DoSoftUpgrade(void)
{
	BYTE * pData = pReceiveFrame->Frame68.Data+m_dwasdu.Infooff+m_guiyuepara.infoaddlen;
	BYTE ctype;
	//static BYTE cnt=0;
	static BYTE softupcmd=0;	
	ctype = pData[0];

	if((m_dwasdu.COT != 6)&&(m_dwasdu.COT != 8))
	{
		m_errflag = 5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return FALSE;
	}

	if(((m_dwasdu.COT & 0x3f)== 6) && ((ctype & 0x80)==0x80)) //升级启动
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
		m_bySoftUpgradeStatus = 1;
		softupcmd = 1;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);

	}
	else if((m_dwasdu.COT == 6) && ((ctype & 0x80)==0 )) //升级结束 升级执行
	{
		/*if((cnt == 0)&&(softupcmd == 1))  //升级执行
		{
			cnt = 1;
			m_bySoftUpgradeStatus = 2;
			memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		}
		else*/  //升级结束
		{
			//cnt = 0;
			softupcmd = 0;
			m_bySoftUpgradeStatus = 2;
			memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
			m_bysoftupflag=1;
		}
		
	}
	else if((m_dwasdu.COT == 8) && ((ctype & 0x80)==0 )) //撤销命令
	{
		if(softupcmd == 1)
		{
			softupcmd = 0;
			m_bySoftUpgradeStatus = 3;
		}
		else
		{
			m_errflag=7;
			memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
			return TRUE;
		}
		
	}
	else
		return FALSE;

	return TRUE;
	
}

BOOL CGB101S::SendSoftUpgradeAck(void)
{
	BYTE PRM=0, dwCode=0;
	
	if(m_bySoftUpgradeStatus == 1)  //发送启动升级确认
	{
		PRM = 0;dwCode = 8;
		m_SendBuf.wReadPtr = 0;
		m_SendBuf.wWritePtr = 0;
		if(m_tdata[0]!=0x68)
		{
			m_errflag=0;
			return false;
		}
		if(m_dwasdu.COT == COT_ACT)
		{
			m_tdata[m_dwasdu.COToff+5] = COT_ACTCON;
		}
		else
		{
			m_tdata[0] = 0;
		}
		memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_tdata,m_tdata[1]+4);
		m_SendBuf.wWritePtr += m_tdata[1]+4;
		
	}
	else if(m_bySoftUpgradeStatus == 2)  //发送升级结束确认
	{
		PRM = 0;dwCode = 8;
		m_SendBuf.wReadPtr = 0;
		m_SendBuf.wWritePtr = 0;
		if(m_tdata[0]!=0x68)
		{
			m_errflag=0;
			return false;
		}
		
		if(m_dwasdu.COT == COT_ACT)
		{
			m_tdata[m_dwasdu.COToff+5] = COT_ACTCON;
		}
		else
		{
			m_tdata[0] = 0;
		}
		memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_tdata,m_tdata[1]+4);
		m_SendBuf.wWritePtr += m_tdata[1]+4;

		
	}
	else if(m_bySoftUpgradeStatus == 3)  //发送终止命令确认
	{
		PRM = 0;dwCode = 8;
		m_SendBuf.wReadPtr = 0;
		m_SendBuf.wWritePtr = 0;
		if(m_tdata[0]!=0x68)
		{
			m_errflag=0;
			return false;
		}
		
		if(m_dwasdu.COT == COT_DEACT)
		{
			m_tdata[m_dwasdu.COToff+5] = COT_DEACTCON;
		}
		else
		{
			m_tdata[0] = 0;
		}
                  m_tdata[m_dwasdu.Infooff + m_guiyuepara.infoaddlen+5] = 0; //
		memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_tdata,m_tdata[1]+4);
		m_SendBuf.wWritePtr += m_tdata[1]+4;
	
	}
	else 
		return FALSE;

#if defined ECC_MODE_CHIP && defined INCLUDE_ECC
	if((3 == m_guiyuepara.jiami)||(4 == m_guiyuepara.jiami))
	{//ly
		if(m_bySoftUpgradeStatus == 1)
		{
			MD5_Init();
			SendFrameTail0x2(PRM, dwCode, m_dwasdu.VSQ);
		}
		else
			SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);
	}
	else
#endif
		SendFrameTail(PRM, dwCode, m_dwasdu.VSQ);


	m_bySoftUpgradeStatus=0;
	
	return TRUE;

}

/*召唤电能量*/
BOOL CGB101S::DoCallEnergy(void)
{
	BYTE * pData = pReceiveFrame->Frame68.Data+m_dwasdu.Infooff+m_guiyuepara.infoaddlen; 
	int i;
	if(m_dwasdu.VSQ!=1)
	{
		m_errflag=0x4;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return TRUE;
	}
	if(m_dwasdu.COT != 6)
	{
		m_errflag=5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return FALSE;
	}

	if(m_dwasdu.Info != 0)
	{
		m_errflag=7;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return FALSE;
	}
	if(((pData[0]&0xf) != 5))
	{
		m_errflag=5;
		memcpy(m_tdata,&pReceiveFrame->Frame68.Start1,pReceiveFrame->Frame68.Length1+4);
		return FALSE;
	}
	
	for(i = 0; i < g_ddhxnum; i++)
	{
		m_dwfiffrzflag |= 0x01<<i;
		m_dwrealdataflag |= 0x01<<i;
		m_dwdayfrzflag |= 0x01<<i;
	}
	m_dwdayfrzflag |= 0x10000000;
	m_byCallEnergyFlag = 1;
	
	return TRUE;

}

BOOL CGB101S::SendAllEnergy(void)
{
	int i;
	
	if(m_byCallEnergyFlag == 1)
	{
		m_byCallEnergyFlag =0;
		SendAllEnergyAck();
		return TRUE;
	}

	for(i=0; (i<FRM_DDNUM)&&(i<g_ddhxnum); i++)    //  总召  实时数据
	{
		if( m_dwrealdataflag & (1<<i) )  
		{
			m_dwrealdataflag &= ~(1<<i);
			MakeRealddGroupFrame(i, 37);
		    return TRUE;
	   }
	}

	for(i=0; (i<FRM_DDNUM)&&(i<g_ddhxnum); i++)    //  总召  15分钟冻结数据
	{
		if( m_dwfiffrzflag & (1<<i) )  
		{
			m_dwfiffrzflag &= ~(1<<i);
			MakeFifFrzGroupFrame(i, 37);
			return TRUE;
		}
	}

	for(i=0; (i<FRM_DDNUM)&&(i<g_ddhxnum); i++)    //  总召  日冻结数据
	{
		if( m_dwdayfrzflag & (1<<i) )  
		{
			m_dwdayfrzflag &= ~(1<<i);
			MakeDayFrzGroupFrame(i, 37);
			return TRUE;
		}
	}

	if(m_dwdayfrzflag & (0x10000000))  //总召结束
	{
		m_dwfiffrzflag=0;
		m_dwrealdataflag=0;
		m_dwdayfrzflag=0;
		SendAllEnergyOverAck();
		return TRUE;
	}

	return FALSE;
	
}

BOOL CGB101S::MakeRealddGroupFrame(WORD GroupNo,BYTE Reason)
{
	BYTE Style=206;
	BYTE PRM = 0,dwCode=8;
	DWORD DDNo, DDSendNum;
	struct VDDFT Ddbuf;
	DWORD j = 0;
	if(GroupNo >= g_ddhxnum)
		return FALSE;

	DDNo = GroupNo*32;

	SendFrameHead(Style, Reason);
	
	for (DDSendNum = 0;(DDSendNum < GRP_DDNUM) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum);DDSendNum++,DDNo++)
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
		j++;
	}
	if(!j)
		return TRUE;
	SendFrameTail(PRM, dwCode, DDSendNum);
	
	return TRUE;

}

BOOL CGB101S::MakeFifFrzGroupFrame(WORD GroupNo,BYTE Reason)
{
	BYTE Style=207;
	BYTE PRM = 0,dwCode=8;
	DWORD DDNo, DDSendNum;
	struct VDDFT Ddbuf;
	struct VSysClock SysTime;
	DWORD j = 0;

	if(GroupNo >= g_ddhxnum)
		return FALSE;

	DDNo = GroupNo*32+8;
	
	SendFrameHead(Style, Reason);

	for (DDSendNum = 0;(DDSendNum < GRP_DDNUM) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum);DDSendNum++,DDNo++)
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
	SendFrameTail(PRM, dwCode, DDSendNum);
	
	return TRUE;

}

BOOL CGB101S::MakeDayFrzGroupFrame(WORD GroupNo,BYTE Reason)
{
	BYTE Style=207;
	BYTE PRM = 0,dwCode=8;
	DWORD DDNo, DDSendNum;
	struct VDDFT Ddbuf;
	struct VSysClock SysTime;
	DWORD j = 0;

	if(GroupNo >= g_ddhxnum)
		return FALSE;

	DDNo = GroupNo*32+16;
	
	SendFrameHead(Style, Reason);

	for (DDSendNum = 0;(DDSendNum < GRP_DDNUM) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum);DDSendNum++,DDNo++)
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
	SendFrameTail(PRM, dwCode, DDSendNum);
	
	return TRUE;

}

BOOL CGB101S::MakeTideFrzGroupFrame(WORD GroupNo,BYTE Reason)
{
	BYTE Style=207;
	BYTE PRM = 0,dwCode=8;
	DWORD DDNo, DDSendNum;
	struct VDDFT Ddbuf;
	struct VSysClock SysTime;
	DWORD j = 0;
	if(GroupNo >= g_ddhxnum)
		return FALSE;

	DDNo = GroupNo*32+24;
	
	SendFrameHead(Style, Reason);

	for (DDSendNum = 0;(DDSendNum < GRP_DDNUM) && (DDNo < m_pEqpInfo[m_wEqpNo].wDDNum);DDSendNum++,DDNo++)
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
	SendFrameTail(PRM, dwCode, DDSendNum);
	
	return TRUE;

}


BOOL CGB101S::SendChangeFifFrz(void)   // 发送15分钟冻结 突变 数据
{
	int i;

	for(i=0; (i<GRP_DDNUM)&&(i<g_ddhxnum); i++)
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

BOOL CGB101S::SendChangeDayFrz(void)   // 发送日冻结 突变 数据
{
	int i;

	for(i=0; (i<GRP_DDNUM)&&(i<g_ddhxnum); i++)
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

BOOL CGB101S::SendChangeTide(void)   // 发送潮流 突变 数据
{
	int i;

	for(i=0; (i<GRP_DDNUM)&&(i<g_ddhxnum); i++)
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
BOOL CGB101S::SendAllEnergyAck(void)
{
	BYTE Style = 0x65, Reason = 7;
	BYTE PRM = 0,dwCode = 8,Num = 1;

	SendFrameHead(Style, Reason);
	write_infoadd(m_dwasdu.Info);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 5; 
	SendFrameTail(PRM, dwCode, Num);
	return TRUE;
}

//召唤所有电度结束的确认
BOOL CGB101S::SendAllEnergyOverAck(void)
{
	BYTE Style = 0x65, Reason = 10;
	BYTE PRM = 0,dwCode = 8,Num = 1;
	
	SendFrameHead(Style, Reason);
	write_infoadd(m_dwasdu.Info);
	
	m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = 5; 
	SendFrameTail(PRM, dwCode, Num);
	return TRUE;
}

/*************发送故障事件********************/

BOOL CGB101S::SendFaultEvent(void) 
{
	VSysClock SysTime;
	BYTE TypeID = M_FT_NA_1;
	BYTE Reason = COT_SPONT;
#if (DEV_SP  == DEV_SP_TTU)
	SwitchEvent *m_Eventinfo;
#else
	VDBGzEvent* m_Eventinfo;
	WORD VEYxNo,SendNo,VEYcNo;
#endif	
	int i,k=0,ycnum;
	BOOL ch;
	WORD yxnumpos;
	
#if (DEV_SP  == DEV_SP_TTU)
	ch = ReadSwitchEvent((SwitchEvent *)m_dwPubBuf);
	if (ch == ERROR)                                              
	    return FALSE;
	m_Eventinfo = (SwitchEvent *)m_dwPubBuf;
#else
	ch = prReadGzEvent((VDBGzEvent *)m_dwPubBuf);
	ch = FALSE;

	if (ch == ERROR)                                              
	    return FALSE; 

	m_Eventinfo = (VDBGzEvent *)m_dwPubBuf; 
#endif		
	SendFrameHead(TypeID, Reason);
    /*遥信个数*/
	yxnumpos = m_SendBuf.wWritePtr;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(m_Eventinfo->YxNum);

	/*遥信类型*/
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_guiyuepara.yxtype;  

	k=0;
	for(i = 0; i < m_Eventinfo->YxNum; i++)
	{
#if (DEV_SP  == DEV_SP_TTU)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(m_Eventinfo->YxNo[i]);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(m_Eventinfo->YxNo[i]);

#else	
		/*故障点号*/
		if(TEYxNoToVEYxNo(g_Sys.wIOEqpID, m_wEqpID,m_Eventinfo->YxNo[i], &VEYxNo) != OK)
			continue;

		ReadSYXSendNo(m_wEqpID,VEYxNo,&SendNo);	
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(SendNo+ ADDR_YX_LO);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(SendNo+ ADDR_YX_LO);
#endif

		/*遥信值  1BYTE*/
		if(m_guiyuepara.yxtype == 3)    
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_Eventinfo->YxValue[0]+1;
		else
			m_SendBuf.pBuf[ m_SendBuf.wWritePtr++ ] = m_Eventinfo->YxValue[0];        
		
		/*故障时刻时标 7BYTE */
		SystemClock(m_Eventinfo->Time[0].dwMinute, &SysTime);                      

		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(m_Eventinfo->Time[0].wMSecond);   
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(m_Eventinfo->Time[0].wMSecond);   
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
	ycnum = m_SendBuf.wWritePtr;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_Eventinfo->YcNum;

	/*遥测类型*/
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = m_guiyuepara.yctype;

	k=0;
	for(i = 0; i < m_Eventinfo->YcNum; i++)
	{
		if(m_Eventinfo->YcNo[i] == 0xFFFF)
			continue;
#if (DEV_SP  == DEV_SP_TTU)
		write_infoadd(m_Eventinfo->YcNo[i]);
#else
		if(TEYcNoToVEYcNo(g_Sys.wIOEqpID, m_wEqpID,m_Eventinfo->YcNo[i], &VEYcNo) != OK)
			continue;
		
		ReadYCSendNo(m_wEqpID, VEYcNo,&SendNo);

		/*遥测信息体地址*/
		write_infoadd(SendNo+ ADDR_YC_LO);
#endif		
		/*遥测值*/
		if(m_guiyuepara.yctype == 13)  //短浮点型 4B
		{
			if(i <= 3)
				LongToFloat(0,m_Eventinfo->YcValue[i]*10,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
			else
				LongToFloat(0,m_Eventinfo->YcValue[i],m_SendBuf.pBuf+m_SendBuf.wWritePtr);
			m_SendBuf.wWritePtr+=4;
		}
		else  //归一化值 2B
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(m_Eventinfo->YcValue[i]);
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(m_Eventinfo->YcValue[i]);
		}
		k++;
		
	}

	if(!k)
		return FALSE;

	m_SendBuf.pBuf[ycnum] = k;

#ifdef INCLUDE_SEC_CHIP
	if(m_wCommID == COMM_IPSEC_NO)
		SendFrameTail0x1(1, 3, 0); // 故障事件，南网为01
	else
#endif
		SendFrameTail(1, 3, 0);             
	
	return TRUE;    
}

BYTE CGB101S::SecToSystime(DWORD Second, struct VSysClock *SysTime)
{
	struct VCalClock pBuf;
	pBuf.dwMinute = Second/60;
	pBuf.wMSecond = 0;
	
	CalClockTo(&pBuf, SysTime);
	
	return TRUE;
}

BYTE CGB101S::SystimeToSec(struct VSysClock *SysTime, DWORD *Second)
{
	struct VCalClock pBuf;
	
	ToCalClock(SysTime, &pBuf);
	*Second = pBuf.dwMinute*60+pBuf.wMSecond/1000;
	
	return TRUE;
}

void CGB101S::DoClearSoe(void)
{
	WORD cosrdptr,coswtptr;
	QuerySSOE(m_wEqpID, 1, 1, 1000,(VDBSOE *)m_dwPubBuf, &m_WritePtr, &m_BufLen);
	m_ReadPtr=m_WritePtr;
	GetPoolPtr(m_wTaskID,m_wEqpID,SCOSUFLAG,&cosrdptr,&coswtptr);
	while(cosrdptr != coswtptr)
	{//增加pRunInfo->wDSOEReadPtr，不然ReadSCOS有问题
		QuerySCOS(m_wEqpID, 1, cosrdptr, 1000,
    		(VDBCOS *)m_dwPubBuf, &coswtptr, &m_BufLen);
		cosrdptr = (cosrdptr + 1)%m_BufLen;
		ReadPtrIncrease(m_wTaskID, m_wEqpID, 1, SCOSUFLAG);
	}
}
#ifndef INCLUDE_4G
#ifdef INCLUDE_GPRS
void CGB101S::SendGetRSSI(void)//获取RSSI信号
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

	wLinkAddress = m_dwasdu.LinkAddr;

	WriteToComm(wLinkAddress);

}
#endif
#endif
void CGB101S::DoParaMsg(void)
{
	ParaFlag = m_pMsg->Head.byMsgAttr;

	if(m_pMsg->abyData[0] == COT_ACTCON)
	{
		nh = 0;
		if(ParaFlag == 3)
		{
			ReadParaNum = m_pMsg->abyData[1];
		}
	}			
}

#endif

