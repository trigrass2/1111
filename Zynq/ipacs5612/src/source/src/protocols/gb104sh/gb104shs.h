/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef GB104S_H
#define GB104S_H

#include "../../protocol/public/pslave.h"
#include "../../protocol/gb104/gb104pub.h"


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

class CGB104Shs : public CPSecondary
{
	public:
		WORD m_wSendNum;	//Send Counter	
		WORD m_wRecvNum;	//Receive Counter
		WORD m_wAckNum;		//Receive Ack Counter
		WORD m_wAckRecvNum;    //Have Ack Receive Counter
		VIec104Timer m_vTimer[4];	//iec104 timer for T0 T1 T2 T3
		BOOL m_bDTEnable;		//Data Transfer Enable Flag
		BOOL m_bTcpConnect;			//TCP Connect Flag
		BOOL m_bContinueSend;   //continue send I frame flag about K
		
		DWORD m_dwSendAllDataEqpNo;
		WORD m_wRecAddress;
		WORD m_wChangeYcEqpNo;
		
		VIec104Frame *m_pReceive;
		VIec104Frame *m_pSend;
		BYTE *m_pASDUInfo;
		VASDU *m_pASDU;
		VBackFrame m_vBackFrame[PARA_K + 2];
		
		DWORD m_dwYcSendFlag;
		DWORD m_dwYxSendFlag;
		
		WORD m_wSendYcNum;
		WORD m_wSendYxNum;
		WORD m_wSendDYxNum;
		
		DWORD m_dwYcGroupFlag;
		DWORD m_dwYxGroupFlag;
		BYTE m_allcallflag;
		BYTE m_groupcallflag;
		WORD m_ReadPtr;
		WORD m_WritePtr;
		WORD m_BufLen;
		VDWASDU m_dwasdu;
		Vgb104para m_guiyuepara;
		DWORD *limitpara;
		DWORD event_time ;	//100ms
		BYTE m_callallflag;
				
		BYTE m_groupflag;
		BYTE m_yxchangeflag;
		
		BYTE m_ycchangeflag;
		BYTE m_SOEflag;
					
		BYTE m_QOI;
		DWORD m_eqpflag[2];
		DWORD m_flag[2];

		BYTE m_sourfaaddr;
		BYTE m_initflag;
		BYTE m_soe_txdnum;
		BYTE m_Dsoe_txdnum;
		BYTE m_retxdflag;
		BYTE Bitchange[256];
		BYTE m_allcalloverflag;
		WORD m_PARA_K;
		WORD m_PARA_W;
		DWORD tmpms;
		int cosnum;
		int soenum;
		WORD  startdelay;
		VSendsoenum *m_pSendsoenum;

	public:
		CGB104Shs();
		BOOL Init(WORD wTaskID);
		void initpara(void);
		virtual void CheckBaseCfg(void);
		virtual void SetBaseCfg(void);
		
		void StartTimer(BYTE TimerNo);
		void StopTimer(BYTE TimerNo);	
		virtual void DoTimeOut(void);
		virtual int atOnceProcSCOS(WORD wEqpNo);
		virtual void DoCommSendIdle(void);
				
		void CloseTcp(void);
		virtual DWORD DoCommState(void);

		virtual void DoUrgency(void);
		virtual BOOL DoReceive();
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
		void DoIFrame(void);
		void DoSFrame(void);
		void DoUFrame(void);
        void DoCallYcYxData(void);
        void DoSetTime(void);
		void getasdu(void);
		void SendSFrame(void);
		void SendUFrame(BYTE byControl);
		void SendFrameHead(BYTE byFrameType);
		int  SendFrameTail(void);
		BOOL SendClassOneData(void);
		BOOL SendSingleSoe(void);
		int SearchSendYc(WORD wNum, WORD wBufLen, VDBYCF_L *pBuf, BOOL bActive);
		void SendChangeYC(void);
		void SendAllYcYx(void);
		void SendSomeYcYx(void);
		void SendStopYcYx(void);
		void SendStopSomeYcYx(void);
		BOOL SendGroupYc(WORD YcGroupNo,BYTE Reason);
		BOOL SendGroupYcContinue(WORD YcGroupNo,BYTE Reason);
		BOOL SendGroupYx(WORD YxGroupNo,BYTE Reason);
		BOOL SendGroupYxContinue(WORD YxGroupNo,BYTE Reason);
		void SendCallYcYxAck(void);
		void SendSomeYcYxAck(BYTE byQrp);
		BOOL SendDSoe(void);
		BOOL CanSendIFrame(void);
		void SendInitFinish(void) ;
		void write_typeid(int  data);
		void write_VSQ(int  data);
		void write_COT(int  data);
		void write_conaddr(int  data);
		void write_infoadd(int  data);
		void errack(BYTE COT);
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


};


#endif

