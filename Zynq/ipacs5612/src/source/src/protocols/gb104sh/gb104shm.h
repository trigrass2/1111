/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef GB104P_H
#define GB104P_H

#include "../../protocol/public/pmaster.h"
#include "../../protocol/gb104/gb104pub.h"

#define DCO_SE_EXE		0
#define DCO_SE_SELECT	0x80
#define DCO_DCS_OPEN	0x1
#define DCO_DCS_CLOSE	0x2


class CGb104Shm : public CPPrimary
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
		BOOL m_bCallAllFlag;
		BOOL m_bSetTimeFlag;
		VIec104Frame *m_pReceive;
		VIec104Frame *m_pSend;
		BYTE *m_pASDUInfo;
		VASDU *m_pASDU;
		VBackFrame m_vBackFrame[PARA_K + 2];
		WORD m_PARA_K;
		WORD m_PARA_W;
		Vgb104para m_guiyuepara;
		VDWASDU m_dwasdu;
	public:
		CGb104Shm();
		BOOL Init(WORD wTaskID);
		virtual void CheckBaseCfg(void);
		virtual void SetBaseCfg(void);
		
		void StartTimer(BYTE TimerNo);
		void StopTimer(BYTE TimerNo);	
		virtual void DoTimeOut(void);		
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
		void DoYcData(void);
		void DoChangeYc(void);
		void DoGroupYc(void);
		void DoCallAllData(void);
		void DoAllDataAck(void);
		void DoAllDataStop(void);
		void DoSetTime(void);				
		virtual BOOL ReqCyc(void);
		void SendSFrame(void);
		void SendUFrame(BYTE byControl);
		void SendFrameHead(BYTE byFrameType);
		BOOL SendFrameTail(void);
		void SendSetTime(void);
		void SendCallAllData(void);
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
		void  DIQ(BYTE data,BYTE *data1,BYTE *data2);
		DWORD  getinfoaddr(BYTE *psrc);
		BOOL DoYKReq(void);
		void DoDoubleYk(void);
		void DoYkReturn(void);
		void DoYkCancelAck(void);
		BOOL SendYkCommand(void);
		


};

#endif
