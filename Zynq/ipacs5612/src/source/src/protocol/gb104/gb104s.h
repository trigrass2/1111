/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef GB104S_H
#define GB104S_H

#include "../public/pslave.h"	
#include "gb104pub.h"
#include "../../protocols/dlq/DLQ.h"


#define YC_FRM_NUM		80	//max yc num = 120 per frame
#define YX_FRM_NUM		100	//max yx num = 127 per frame because of the VSQ bit8 not use
#define DD_FRM_NUM		20	//max dd num = 20 per frame because each dd len = 3(no)+4(val)+1(serial) = 8

#define REQ_SCOS_NUM		50	//max req scos num = 60 (ASDUINFO_LEN/4)
#define REQ_SSOE_NUM		16	//max req ssoe num = 22 (ASDUINFO_LEN/11)
#define REQ_CHANGEYC_NUM	20	//max req changeyc num = 48 (ASDUINFO_LEN/5)

#define CMD_ACT      1
#define CMD_DEACT 2
#define CMD_ACTCONFIM 4
#define CMD_DEACTCONFIM 8
#define CMD_ACTTERM    0x10

#define MAX104LEN	200
#if(DEV_SP  == DEV_SP_TTU)
#define FRM_DDNUM	20
#else
#define FRM_DDNUM	8			
#endif
#define SWITCHCOS_BIT		0X01
#define SWITCHALLCALL_BIT   0X02

#define YXH					0X01
#define YXF    			    0X00
#define YXDelayTime         10

class CGB104S : public CPSecondary
{
	public:

		VFileDirInfo FileDirInfo; 
		VDirInfo * pVDirInfo;
		FileStatus DirStatus;
		DWORD startoffset;
		DWORD stopoffset;
		DWORD m_fileoffset;
		BYTE DirOffSet[500]; // 保存满足时间段内目录偏移
		WORD DirOffSetNum;
		BYTE m_calldir;
		BYTE* filetemp;
		BYTE m_bysoftupflag;
		VFileInfo ReadFileInfo;
		VFileInfo WriteFileInfo;
		VDelaySoe m_delaysoe[2];
		VDelayCos m_delaycos[2];
		BYTE m_byReadFileFlag; 
		BYTE m_byParaValFlag;
		DWORD m_paraoffset;
		WORD m_paraaddrnum;
		BYTE m_tdata[256];
		VParaInfo WriteParaInfo[256];
		WORD WriteParaAddr[256];
		WORD m_WriteNum;
		WORD m_paranum;
		BYTE m_fdno;
		BYTE paracmdstatue;
		DWORD m_dwfiffrzflag;
		DWORD m_dwrealdataflag;
		DWORD m_dwdayfrzflag;

		WORD m_wSendNum;	//Send Counter	
		WORD m_wRecvNum;	//Receive Counter
		WORD m_wAckNum;		//Receive Ack Counter
		WORD m_wAckRecvNum;    //Have Ack Receive Counter
		VIec104Timer m_vTimer[4];	//iec104 timer for T0 T1 T2 T3
		BOOL m_bDTEnable;		//Data Transfer Enable Flag
		BOOL m_bTcpConnect;			//TCP Connect Flag
		BOOL m_bContinueSend;   //continue send I frame flag about K
		BOOL m_bnop;   //continue send I frame flag about K
		
		DWORD m_dwSendAllDataEqpNo;
		WORD m_wRecAddress;
		WORD m_wChangeYcEqpNo;
		WORD m_wAllDdEqpNo;
		
		VIec104Frame *m_pReceive;
		VIec104Frame *m_pSend;
		BYTE *m_pASDUInfo;
		VASDU *m_pASDU;
		VBackFrame m_vBackFrame[PARA_K + 2];
		
		DWORD m_dwDdSendFlag;
		DWORD m_dwYcSendFlag;
		DWORD m_dwYxSendFlag;
		DWORD m_dwcycYcSendFlag;
		DWORD m_dwcycYxSendFlag;
		
#ifdef  _GUANGZHOU_TEST_VER_
		DWORD m_dwchangeYcSendFlag;
		WORD m_wSendchangeYcNum;
#endif

		WORD m_wSendYcNum;
		WORD m_wSendYxNum;
		WORD m_wSendDYxNum;
		WORD m_wSendDdNum;
		WORD m_wSendparaNum;
		
		DWORD m_dwYcGroupFlag;
		DWORD m_dwYxGroupFlag;
		DWORD m_dwDdGroupFlag;
		BYTE m_byYcYxQrp;
		BYTE m_byDdQcc;
		BYTE m_allcallflag;
		BYTE m_readtimeflag;
		BYTE m_groupcallflag;
		WORD m_ReadPtr;
		WORD m_WritePtr;
		WORD m_BufLen;
		VDWASDU m_dwasdu;
		Vgb104para m_guiyuepara;
		DWORD *limitpara;
			DWORD event_time ;	//100ms
			DWORD safe_time;
				BYTE m_delaychangeyc;
				BYTE m_callallflag;
				
				BYTE m_groupflag;
				BYTE m_yxchangeflag;
				
				BYTE m_ycchangeflag;
				BYTE m_SOEflag;
				
				BYTE m_YKflag;
				BYTE m_timeflag;
					
				BYTE m_testflag;
				BYTE m_resetflag;

				BYTE m_QOI;
				BYTE m_QPM;

				BYTE m_readparaflag;
				BYTE m_readparaeqpnum;

				BYTE m_allcycleflag;
				BYTE m_ddcycleflag;
				BYTE m_sourfaaddr;
				BYTE m_QRP;
				BYTE m_initflag;
				BYTE m_init;
				BYTE m_soe_txdnum;
				BYTE m_Dsoe_txdnum;
				BYTE m_YK_select_cancel;
				BYTE m_retxdflag;
				BYTE m_TNsetflag;
		BYTE Bitchange[256];
		BYTE m_allcalloverflag;
				WORD m_allsendtime;
		WORD m_PARA_K;
		WORD m_PARA_W;
		DWORD tmpms;
		int cosnum;
		int soenum;
		WORD  startdelay;
		VSendsoenum *m_pSendsoenum;
         float fIDeadValue;
         float fACDeadValue;
		 float fDCDeadValue;   //直流电压死区
		 float fPQDeadValue;   //功率死区
		 float fFDeadValue;		//频率死区
		 float fPWFDeadValue;//功率因数死区
		 float fUTHDeadValue;//电压谐波死区
		 float fITHDeadValue;//电流谐波死区
		 float fUNDeadValue;//不平衡度死区
		 float fLoadDeadValue;//负载率死区
		 

#if defined  ECC_MODE_CHIP && defined INCLUDE_ECC
		BYTE m_jiamiType;
#endif

	public:
		CGB104S();
		BOOL Init(WORD wTaskID);
		void initpara(void);
		virtual void CheckBaseCfg(void);
		virtual void SetBaseCfg(void);
		
		void StartTimer(BYTE TimerNo);
		void StopTimer(BYTE TimerNo);	
		virtual void DoTimeOut(void);
		virtual BOOL DoYKRet(void);
		void SendYkReturn(void);
		virtual int atOnceProcSCOS(WORD wEqpNo);
		virtual int atOnceProcFAInfo(WORD wEqpNo);
		virtual void DoCommSendIdle(void);
		virtual int atOnceProcSSOE(WORD wEqpNo);
		virtual int atOnceProcDSOE(WORD wEqpNo);
				
		void CloseTcp(void);
		virtual DWORD DoCommState(void);
		
		virtual BOOL DoReceive();
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
		void DoIFrame(void);
		void DoSFrame(void);
		void DoUFrame(void);
		void DoYk(void);
        void DoCallYcYxData(void);
        void DoCallDd(void);
        void DoReadData(void);
        void DoSetTime(void);
        void DoTestLink(void);
        void DoTestLink1(void);
        void DoReset(void);
		void sendasdu(void);
		BOOL getasdu(void);
		void SendSFrame(void);
		void SendUFrame(BYTE byControl);
		void SendFrameHead(BYTE byFrameType);
		int  SendFrameTail(void);
		int  SendFrameTail0x1(void);
		int  SendFrameTail0x2(void);
		void CycleSendData(void);
		BOOL SendClassOneData(void);
		void SendClassTwoData(void);
		BOOL SendSingleSoe(void);
		BOOL SendChangeYC(void);
		BOOL SendtimeReq(void);
		void SendYkCancelAck(void);
		void SendAllYcYx(void);
		void SendSomeYcYx(void);
		void SendStopYcYx(void);
		void SendStopSomeYcYx(void);
		BOOL SendGroupYc(WORD YcGroupNo,BYTE Reason);
		BOOL SendGroupYc_Addr(WORD YcGroupNo,BYTE Reason);
		BOOL SendGroupYcContinue(WORD YcGroupNo,BYTE Reason);
		BOOL SendGroupYx(WORD YxGroupNo,BYTE Reason);
		BOOL SendGroupYxContinue(WORD YxGroupNo,BYTE Reason);
		BOOL SendGroupDd(WORD DdGroupNo,BYTE Reason);
		BOOL SendGroupDdContinue(WORD DdGroupNo,BYTE Reason);
		void SendCallYcYxAck(void);
		void SendSomeYcYxAck(BYTE byQrp);
		void SendCallDdAck(void);
		void SendAllDd(void);
		void SendSomeDd(void);
		void SendDdStop(void);
		void SendReadYx(WORD wYxNo);
		void SendReadYc(WORD wYcNo);
		void SendReadDd(WORD wDdNo);
		void SendNoData(void);
		void SendYkstop();
		virtual BOOL DoFaSim(void);
		BOOL SendDSoe(void);
		void DelAckFrame(WORD SendNum);
		void SaveSendFrame(WORD SendNum, WORD FrameLen, BYTE *pFrame);
		BOOL CanSendIFrame(void);
		void SendInitFinish(void) ;
		BOOL RecYTCommand(void);
		virtual BOOL DoYTRet(void);
		void SendYtReturn(void);
		void write_typeid(int  data);
		void write_VSQ(int  data);
		void write_COT(int  data);
		void write_conaddr(int  data);
		void write_infoadd(int  data);
		void errack(BYTE COT);
		void errack0x2(BYTE COT);
		BYTE QDS(BYTE data);
		BYTE SIQ(BYTE data);
		void write_time();
		void write_time3();
		virtual void CheckCfg();
		virtual void SetDefCfg();
		BYTE DIQ(BYTE data1,BYTE data2);
		void Sendframe(BYTE type,WORD cot);
		BOOL SendDCOS(void);
		virtual int atOnceProcDCOS(WORD wEqpNo);
		BOOL SendGroupDYx(WORD YxGroupNo,BYTE Reason);
		void SendresetReturn(void);
		void SendSetparaReturn();
		int Geteqpparaaddr(WORD addr);
		void copydata(WORD len1,WORD len2,BYTE *p1,BYTE* p2);
		void Recsetpara(void);
		void Setpara(void);
		BOOL Sendpara(void);
		void writepara();
		void Recpara(void);
		void CycleSendAllYcYx(BYTE reason);
		void Recparaact(void);
		void Reclinktest(void);
		void DoYkext(void);
		void SendReadYcpara(WORD wYcNo);
		void DoSetDot();
		BOOL SendYXM_PS_NA(WORD YxGroupNo,BYTE Reason);
		void DoYTF(void);
		void DoYT(void);
		void SendYtstop(void);
#ifdef  _GUANGZHOU_TEST_VER_
		BOOL SendGroupchangeYc(WORD YcGroupNo,BYTE Reason);
		BOOL SendAllchangeYc(void);

#ifdef  _POWEROFF_TEST_VER_
		BOOL SendFaultVal(void);
		int Getfaultno(int id);
#endif

		// int atOnceProcActEvent(WORD wEqpNo);
#endif

		int GB104ReadRunPara(WORD parano,char* pbuf);
		int GB104WriteRunPara(WORD parano,char* pbuf);
		int GB104WriteRunParaFile();
		
		int ReadDeadValue(WORD parano,char*pbuf);
		int WriteDeadValue(WORD parano,char*pbuf);
		int WriteDeadValueFile();
		
		BOOL DoFileData(void);
		BOOL DoFileDir(void);
		BOOL SendFileDir(void);
		void GetRcdNametime(char *str, BYTE *buf);
		BOOL DoReadFile(void);
		BOOL SendReadFileData(void);
		BOOL DoWriteFile(void);
		BYTE SecToSystime(DWORD Second, struct VSysClock *SysTime);
		BYTE SystimeToSec(struct VSysClock *SysTime, DWORD* Second);
		BYTE ChkSum(BYTE *buf, WORD len);
		BOOL DoSwitchValNo(void);
		BOOL DoReadValNo(void);
		BOOL DoReadParaVal(void);
		BOOL SendReadParaVal(void);
		BOOL DoWriteParaVal(void);
		BOOL DoCallEnergy(void);
		BOOL SendAllEnergy(void);
		BOOL MakeRealddGroupFrame(WORD GroupNo,BYTE Reason);
		BOOL MakeFifFrzGroupFrame(WORD GroupNo,BYTE Reason);
		BOOL MakeDayFrzGroupFrame(WORD GroupNo,BYTE Reason);
		BOOL MakeTideFrzGroupFrame(WORD GroupNo,BYTE Reason);
		BOOL SendChangeFifFrz(void);
		BOOL SendChangeDayFrz(void);
		BOOL SendChangeTide(void);
		BOOL SendAllEnergyAck(void);
		BOOL SendFaultEvent(void) ;
		BOOL DoSoftUpgrade(void);
		int ReadPara(WORD addr, char* pbuf);
		int WriteParaYZ(WORD addr, char* pbuf);
		int WriteSwitchParaYZ(WORD addr, char* pbuf);
		int WriteParaGH(WORD addr, char* pbuf);
		virtual void DoClearSoe(void);
		int SendAllFrame(void);
#ifdef FENGXIAN
		BOOL SendSingleSoe_Delay(int no);
		BOOL SendClassOneData_Delay(int no);
#endif

#ifdef INCLUDE_GPRS
		void SendGetRSSI();
#endif
		
};


#endif

