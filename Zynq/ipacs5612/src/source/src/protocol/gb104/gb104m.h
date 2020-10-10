/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef GB104P_H
#define GB104P_H

#include "../public/pmaster.h"
#include "gb104pub.h"

#define DCO_SE_EXE		0
#define DCO_SE_SELECT	0x80
#define DCO_DCS_OPEN	0x1
#define DCO_DCS_CLOSE	0x2

#define DD_NUM 8

#define DD_CUR_ADDR  0x6401
#define DD_CUR_LINE_NUM 32

#define DD_FIFRZ_ADDR 0x6409
#define DD_FIFRZ_LINE_NUM 32

#define DD_DAYFRZ_ADDR 0x6411
#define DD_DAYFRZ_LINE_NUM 32

#define DD_FLOW_ADDR  0x6419
#define DD_FLOW_LINE_NUM 32



class CGb104m : public CPPrimary
{
	public:
	#ifdef LQH_DEBUG
		WORD m_wtemp;
	#endif
		WORD m_wSendNum;	//Send Counter	
		WORD m_wRecvNum;	//Receive Counter
		WORD m_wAckNum;		//Receive Ack Counter
		WORD m_wAckRecvNum;    //Have Ack Receive Counter
		VIec104Timer m_vTimer[4];	//iec104 timer for T0 T1 T2 T3
		BOOL m_bDTEnable;		//Data Transfer Enable Flag
		BOOL m_bTcpConnect;			//TCP Connect Flag
		BOOL m_bContinueSend;   //continue send I frame flag about K
		BOOL m_bCallAllFlag;
		BOOL m_bCallDdFlag;
		BOOL m_bSetTimeFlag;
		BOOL m_bHaveDd;//add by lqh 20080111
		VIec104Frame *m_pReceive;
		VIec104Frame *m_pSend;
		BYTE *m_pASDUInfo;
		VASDU *m_pASDU;
		VBackFrame m_vBackFrame[PARA_K + 2];
		WORD m_PARA_K;
		WORD m_PARA_W;
		Vgb104para m_guiyuepara;
		VDWASDU m_dwasdu;

		BYTE m_fdno;
		BYTE ParaOffset;
		BYTE ParaFlag;
		BYTE ReadNum;
		WORD nextnum;
		WORD nextoffset;
		VMsg ParaMsg;
	public:
		CGb104m();
		BOOL Init(WORD wTaskID);
		virtual void CheckBaseCfg(void);
		virtual void SetBaseCfg(void);
		virtual void CheckCfg();
	  virtual void SetDefCfg();
	
		void StartTimer(BYTE TimerNo);
		void StopTimer(BYTE TimerNo);	
		virtual void DoTimeOut(void);
		virtual BOOL DoYKReq(void);
		virtual BOOL DoYTReq(void);
		void CloseTcp(void);
		virtual DWORD DoCommState(void);
		
		virtual BOOL DoReceive();
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
		void DoIFrame(void);
		void DoSFrame(void);
		void DoUFrame(void);
		
		void DoSingleYx(void);
		void DoSingleCos(void);
		void DoSingleGroupYx(void);
		void DoSingleSoe(void);
		void DoYcFFData(void);
		void DoGroupYcFF(void);
		void DoChangeYcFF(void);
		void DoYcData(void);
		void DoChangeYc(void);
		void DoGroupYc(void);
		void DoYcData_0x0B(void);
		void DoChangeYc_0x0B(void);
		void DoGroupYc_0x0B(void);
		void DoDdData(void);
		void DoDdDataTime(void);
		BOOL SendYT();
		void DoYTF(void);
		void DoYtReturn(void);
		void DoYtCancelAck(void);
		void DoDoubleYk(void);
		void DoYkReturn(void);
		void DoYkCancelAck(void);
		void DoCallAllData(void);
		void DoAllDataAck(void);
		void DoAllDataStop(void);
		void DoCallDd(void);
		void DoDdAck(void);
		void DoDdStop(void);
		void DoSetTime(void);
		void DoFaInfo(void);
		void DoWriteParaSet(void);
		void DoReadParaSet(void);
				
		virtual BOOL ReqUrgency(void);
		virtual BOOL ReqCyc(void);
		void SendSFrame(void);
		void SendUFrame(BYTE byControl);
		void SendFrameHead(BYTE byFrameType);
		BOOL SendFrameTail(void);
		void SendSetTime(void);
		void SendCallAllData(void);
		void SendCallDd(void);
		BOOL SendYkCommand(void);
		void SendWriteSetParaVal_Multi(void);
		void SendWriteSetParaSolidify(void);
		void SendWriteSetParaCancel(void);
		void SendReadSetParaAll(void);
		void SendReadSetParaPart(void);
		
		virtual BOOL DoFaSim(void);
		virtual void DoParaMsg(void);
		void SendFaSimInfo(void);
		
		void DelAckFrame(WORD SendNum);
		void SaveSendFrame(WORD SendNum, WORD FrameLen, BYTE *pFrame);
		BOOL CanSendIFrame(void);
		void Sendframe(BYTE type,WORD cot);
		void initpara(void);
		void write_typeid(int  data);
		void write_VSQ(int  data);
		void write_COT(int  data);
		void write_conaddr(int  data);
		void write_infoadd(int  data);
		void getasdu(void);
		BYTE QDS(BYTE data);
		BYTE SIQ(BYTE data);
		void DIQ(BYTE data,BYTE *data1,BYTE *data2);
		DWORD getinfoaddr(BYTE *psrc);
};

#endif
