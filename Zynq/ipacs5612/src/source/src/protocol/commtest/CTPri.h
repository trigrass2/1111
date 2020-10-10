/*------------------------------------------------------------------------
 Module:       	ctpri.h
 Author:        rover2
 Project:       ds3200
 State:         ´´½¨
 Creation Date:	2002-09-25
 Description:   comtest pri head
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: CTPri.h,v $
 Revision 1.0  2002-11-25 09:55:46+08  amine
 Initial revision

------------------------------------------------------------------------*/

#ifndef _CTPRI_H
#define _CTPRI_H

#include "sys.h"

#define EV_SCHEDULE    0x01

#define MINLEN 10
#define MAXLEN 110
#define MaxComBufSize (2*MAXLEN)
#define SEND 0x80

#pragma pack(1)
struct VEqpPro{
   WORD wEqpID;
   WORD wAddr;
   struct VCommRunInfo *pCommRunInfo;
}; 

struct TESTHEAD_t{
	BYTE sync[6];
	WORD addr;
	BYTE command;
	BYTE length;
};
#pragma pack()

class VCTPri{
 private:  
   int m_nCurEqpNo;
   WORD m_wEqpNum,m_wCommID,m_wTaskID;
   struct VEqpPro* EqpList;

   struct TESTHEAD_t* tph;                 // test packed head

   BYTE m_SyncWord[2];   
   int  m_nLength;
   WORD m_wTxNum;
   WORD m_wRxMeNum;
   WORD m_wRxRespNum;
   DWORD m_dwTxTotalNum;
   DWORD m_dwRxTotalMeNum;
   DWORD m_dwRxTotalRespNum;
   DWORD m_dwTotalRxBadNum;
   DWORD m_dwTotalBreakNum;
   DWORD m_dwTotalTimeoutNum;
   DWORD m_dwConnectOK;
   DWORD m_dwBaudrate;
	 
   BYTE m_RxBuf[MaxComBufSize];
   WORD m_wRxHead;
   WORD m_wRxTail;
   BYTE m_TxBuf[MaxComBufSize];
   WORD m_wTxHead;
   WORD m_wTxTail;
   
   void RecFrm(void);
   void EditTx(void);
   void SDToBuf(void);
   void SendToTeam(void);
   BOOL FindHead(void);
 public:
   WORD m_thid;
    int m_nTimerID;
   VCTPri();
   ~VCTPri();
      
   BOOL Init(WORD wTaskID);

   void Rxd(void);
   void Txd(void);
   void OnTimeOut(void);
   void Schedule(void);	 
   void OnCommStatus(void);
};   
 
#endif
