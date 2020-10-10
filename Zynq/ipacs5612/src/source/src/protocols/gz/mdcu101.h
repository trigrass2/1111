/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef GB101P_H
#define GB101P_H

#include "../public/pmaster.h"
#include "gb101pub.h"

#define DCO_SE_EXE			0
#define DCO_SE_SELECT		0x80
#define DCO_DCS_OPEN		0x1
#define DCO_DCS_CLOSE		0x2

#define	SOE_TIME_LONG		1
#define SOE_TIME_SHORT		2

#define			ACD_ON		(USER_EQPFLAG + 0)
#define			FCV_ON		1
#define			FCV_OFF		0

#define			STM_BROADCAST		1
#define			STM_SINGLE			2

#define			MAX_LINK_TIMES		3
#define			LINK_INTERVAL		2*60	//two minutes



#define MD_ADDR_YX_LO		0x001
#define MD_ADDR_YX_HI		0x400

#define MD_ADDR_YC_LO		0x701
#define MD_ADDR_YC_HI		0x900

#define MD_ADDR_YK_LO		0xb01
#define MD_ADDR_YK_HI		0xb80

#define MD_ADDR_YT_LO		0xb81
#define MD_ADDR_YT_HI		0xc00
#define MD_ADDR_DD_LO		0xc01
#define MD_ADDR_DD_HI		0xc80



typedef struct 
{
	BYTE byStatus;
	BYTE byLinkTimes;
	BYTE byFCB;
	BOOL bReqAllData;
	BOOL bReqAllDd;
} VIec101PriInfo; //Iec101 规约的每一模块的信息

class MDCU101P : public CPPrimary
{
	public:
		VIec101Frame * m_pReceive; 	//接收帧头指针
		VIec101Frame * m_pSend;  		//发送帧头指针
		VIec101PriInfo  *Iec101PriInfo;
		VIec101Cfg *m_pVIec101Cfg;
		VIec101Cfg m_Iec101Cfg;
		BYTE m_byFCB;
		BYTE m_byLinkAdrShift;
		BYTE m_byChksumShift;
		BYTE m_byStopShift;
		BYTE m_byTypeIDShift;
		BYTE m_byQualifierShift;
		BYTE m_byReasonShift;
		BYTE m_byCommAdrShift;
		BYTE m_byInfoShift;
		Vgb101para m_guiyuepara;
		VDWASDU m_dwasdu;
		BYTE ByTestFlag;
		DWORD m_dwTestTimerCount;

	public:
		MDCU101P();
		BOOL Init(WORD wTaskID);
		virtual void SetDefFlag(void);
		virtual void CheckBaseCfg(void);
		virtual void SetBaseCfg(void);
		
		virtual BOOL DoReceive();
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
		
		BOOL DoFrame10(void);
		BOOL DoFrame68(void);
		void DoCallCommand(void);
		void DoCallDdCommand(void);
		//Rec Yc
		void DoYcData(void);
		void DoChangeYc(void);
		void DoGroupYc(void);
		//Rec Yx Cos Soe
		void DoSingleYx(void);
		void DoSingleCos(void);
		void DoSingleGroupYx(void);
		void DoSingleSoe(BYTE SoeTimeFormat);
		//Rec Dd
		void DoDdData(void);
		//Rec Yk
		void DoDoubleYk(void);
		void DoYkReturn(void);
		
		virtual BOOL DoYKReq(void);

		virtual void DoCommIdle();
		virtual void DoTimeOut(void);
		BOOL SendResetAck(void);
		BOOL SendReqAck(void);
		
		BOOL SendBaseFrame(BYTE PRM, BYTE FCV, BYTE dwCode);
		BOOL SendFrameHead(BYTE Style, BYTE Reason);
		BOOL SendFrameTail(BYTE PRM, BYTE FCV, BYTE dwCode, BYTE Num);
		BOOL SendResetLink(BYTE PRM);
		BOOL SendReqLink(void);
		void SendSetTime(BYTE SetMode);
		void SendReqClass1(void);
		void SendReqClass2(void);
		void SendReqAllData(void);
		void SendReqAllDd(void);
		BOOL SendYkCommand(void);
		
		virtual BOOL ReqUrgency(void);
		virtual BOOL ReqCyc(void);
		virtual BOOL ReqNormal(void);
		
		BYTE ChkSum(BYTE *buf, WORD len);
		DWORD GetAddress(void);
		BYTE GetCtrCode(BYTE PRM, BYTE FCV, BYTE dwCode);
		void getasdu(void);
		void write_linkaddr(int  data);
		void write_typeid(int  data);
		void write_VSQ(int  data);
		void write_COT(int  data);
		void write_conaddr(int  data);
		void write_infoadd(int  data);
		virtual void CheckCfg();
		virtual void SetDefCfg();
		WORD  dealdyxnum(WORD no);
		 BYTE QDS(BYTE data);
		 BYTE SIQ(BYTE data);
		  void  DIQ(BYTE data,BYTE *data1,BYTE *data2);
		  void DoGroupYcF(void);
		  void DoYcFData(void);
		  void DoChangeYcF(void);
};


#endif
