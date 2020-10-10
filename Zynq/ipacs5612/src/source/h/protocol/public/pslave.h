/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef _PSECONDARY_H
#define _PSECONDARY_H

#include "pbase.h"

struct VSecEqpExtInfo{
	WORD wYCBeginNo;
	
	VYCF_L *OldYC;
	VYCF_L  *CurYC;
	VYCF_L  *LastYC;
	BYTE* LastYCNum;
	VYCF_L_Cfg *YCCFG;
    BOOL *ChangeYCSend;
	WORD wChangYCNo;

    WORD wSendDSOENum;  
    WORD wSendDCOSNum; 
    WORD wSendSSOENum;  
    WORD wSendSCOSNum;  
    WORD wSendEVSOENum; 
    WORD wSendEVCOSNum; 
    WORD wSendFAInfoNum;	
    WORD wSendActEventNum;	
    WORD wSendDoEventNum;	
    WORD wSendWarnEventNum;		
};


struct VUFlagInfo{
    DWORD dwFlag;
	DWORD dwTaskFlag;
    DWORD dwEqpFlag;
	DWORD dwSendTaskFlag;
	DWORD dwSendEqpFlag;
};
struct Vyksel_onlyone{
	WORD  ID;
	WORD  delay;
};
extern struct Vyksel_onlyone  g_yksel_onlyone;

class CPSecondary : public CProtocol
{
	public:
		CPSecondary();
		
		BOOL Init(WORD wTaskID,WORD wMinEqpNum,void *pCfg,WORD wCfgLen);
		virtual void SetDefFlag(void);

        WORD m_wEqpGroupID; 
		VEqpGroupInfo m_EqpGroupInfo;
		VSecEqpExtInfo *m_pEqpExtInfo;
		VSecEqpExtInfo m_EqpGroupExtInfo;
		
		VUFlagInfo m_UFlagInfo[8];
		virtual BOOL InitEqpInfo(WORD wMinEqpNum);
			
		virtual WORD GetEqpAddr(void);
		virtual WORD GetOwnAddr(void);
		WORD GetEqpOwnAddr(void);
		virtual BOOL SwitchToAddress(WORD wAddress);
		BOOL GotoEqpbyAddress(WORD wAddress);

		virtual BOOL DoYKRet(void);
		virtual BOOL DoYKReq(void);		

		virtual BOOL DoYTRet(void);
		virtual BOOL DoYTReq(void);	
		
		WORD SearchChangeYC(WORD wNum, WORD wBufLen, VDBYCF_L *pBuf, BOOL bActive); 
		void FillOldData(void);		
        virtual int atOnceProcDSOE(WORD wEqpNo);
        virtual int atOnceProcDCOS(WORD wEqpNo);
        virtual int atOnceProcSSOE(WORD wEqpNo);
        virtual int atOnceProcSCOS(WORD wEqpNo);
        virtual int atOnceProcEVSOE(WORD wEqpNo);
        virtual int atOnceProcEVCOS(WORD wEqpNo);
        virtual int atOnceProcFAInfo(WORD wEqpNo);
        virtual int atOnceProcActEvent(WORD wEqpNo);
        virtual int atOnceProcDoEvent(WORD wEqpNo);
        virtual int atOnceProcWarnEvent(WORD wEqpNo);
        virtual int atOnceProcRoute(WORD wEqpNo);
		int atOnceProcUrgency(WORD wEqpNo,DWORD dwUFlag);

		WORD GetSendNum(WORD wEqpNo,DWORD dwUFlag);
		int ClearUFlag(WORD wEqpNo) ;

		virtual void DoUrgency(void);		
		virtual void DoTimeOut(void);
};

#endif

