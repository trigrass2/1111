/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef _PCOLHOST_H
#define _PCOLHOST_H

#include "pbase.h"

struct VPriEqpExtInfo{
	WORD wWaitYKCnt;
	WORD wWaitYTCnt;
	WORD wWaitTCCnt;	
	WORD wWaitTQCnt;
	
	WORD wCommErrCount;
};

struct VPoll{
	BOOL bFirst;
	WORD wExpNo;
	WORD wBeginNo;
	WORD wCount;
};	  

class CProtocol;

class CPPrimary : public CProtocol 
{
    public:		
		CPPrimary();

		VPriEqpExtInfo *m_pEqpExtInfo;

		BOOL Init(WORD wTaskID, WORD wMinEqpNum,void *pCfg,WORD wCfgLen);
		void CheckMasterBaseCfg(void);
        virtual void SetDefFlag(void);
		
		virtual BOOL InitEqpInfo(WORD wMinEqpNum);
		BOOL GetEqpCommStauts(WORD wEqpNo); 	

		virtual WORD GetEqpAddr(void);
		virtual WORD GetOwnAddr(void);		
		virtual BOOL SwitchToAddress(WORD wAddress);
		
		virtual void CommStatusProcByRT(BOOL bCommOk);		
		virtual void CommStatusProcByRTNoErrCnt(BOOL bCommOk);
		virtual void OnCommOk(void);
		virtual void OnCommError(void);

		virtual BOOL DoYKReq(void);
		virtual BOOL DoYKRet(void);

		virtual BOOL DoYTReq(void);
		virtual BOOL DoYTRet(void);

		virtual void DoTimeOut(void);		

		virtual BOOL DoSend(void);
		virtual BOOL ReqUrgency(void);
		virtual BOOL ReqCyc(void);
		virtual BOOL ReqNormal(void);
		
		virtual void DoDataRefresh(void);

		VPoll AllData;
		VPoll Clock;
		VPoll DD;
        virtual BOOL GotoNextEqp(void);   			
};

#endif




