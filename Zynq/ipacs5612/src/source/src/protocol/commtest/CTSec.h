/*--------------- DONGFANG ELECTRONICS GROUP LTD.--------------*/
/*              MODULE:   ComTest Head File                    */
/*              PURPOSE:  Secondary Station                    */ 
/*              SUBSYSTEM:DF9200 Software system.              */
/*              AUTHOR:   WenYanJun                            */ 
/*              DATA:     1999.5                               */
/*-------------------------------------------------------------*/

#ifndef _CTSEC_H
#define _CTSEC_H

#include "sys.h"

#define MINLEN 10
#define MAXLEN 110
#define MaxComBufSize (2*MAXLEN)
#define SEND 0x80

#pragma pack(1)
struct TESTHEAD_t{
	unsigned char sync[6];
	unsigned short addr;
	unsigned char command;
	unsigned char length;
};
#pragma pack()

class VCTSec{
 private:     
   WORD m_wCommID;
   WORD m_wTaskID;
   WORD m_wAddr;

   BYTE m_SyncWord[2];   
   BYTE m_RxBuf[MaxComBufSize];
   WORD m_wRxHead;
   WORD m_wRxTail;
   BYTE m_TxBuf[MaxComBufSize];
   WORD m_wTxHead;
   WORD m_wTxTail;
   struct TESTHEAD_t *tph;                 //test packet head  
   BYTE m_byWpl;                      //wanted packet lngth
   DWORD m_dwTotalBreakNum;
   DWORD m_dwConnectOK;
   
   void RecFrm(void);
   void EditTx(void);
   void SDToBuf(void);
   void SendToTeam(void);
   BOOL FindHead(void);
 public:   
   VCTSec();
   ~VCTSec();
      
   BOOL Init(WORD wTaskID);

   void Rxd(void);
   void Txd(void);  
   void OnCommStatus(void);
};   
 
#endif
