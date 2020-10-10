/*------------------------------------------------------------------------
 Module:       	ctsec.cpp
 Author:        rover2
 Project:       ds3200
 State:			创建
 Creation Date:	2002-09-25
 Description:   cometest sec
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: CTSec.cpp,v $
 Revision 1.0  2002-11-25 09:55:45+08  amine
 Initial revision

------------------------------------------------------------------------*/
#include "SysCfg.h"

#ifdef	INCLUDE_COMMTEST

#include "sys.h"
#include "string.h"
#include "os.h"
#include "ctsec.h"

extern "C" void ctsec(WORD thid)
{
  DWORD dwEvent;
  VCTSec *pCTSec;
  
  
  
  pCTSec=new VCTSec();  
  
  if (!pCTSec)
  {
  	thSuspend(thid);
  }	
  
  pCTSec->Init(thid);
  
 
  for(;;)
   {
     evReceive(thid,EV_RX_AVAIL|EV_TX_AVAIL|EV_COMM_STATUS,&dwEvent);

  	 if (dwEvent&EV_RX_AVAIL)
      pCTSec->Rxd();

  	 if (dwEvent&EV_TX_AVAIL)
      pCTSec->Txd();

     if (dwEvent&EV_COMM_STATUS)
	  pCTSec->OnCommStatus();	  
	 
   }
}   

VCTSec::VCTSec()
{
  m_SyncWord[0]=0xEB;
  m_SyncWord[1]=0x90;
  memset(m_RxBuf,0,MaxComBufSize);
  m_wRxHead=0;
  m_wRxTail=0;
  memset(m_TxBuf,0,MaxComBufSize);
  m_wTxHead=0;
  m_wTxTail=0;
  m_wAddr=0xFFFF;
  m_dwTotalBreakNum=0;
  m_dwConnectOK=1;
}

VCTSec::~VCTSec()
{
} 

BOOL VCTSec::Init(WORD wTaskID)
{

  m_wTaskID=wTaskID;
  

  return(TRUE);  
}

void VCTSec::Rxd(void)
{
  int RNum;
  int i; 
  if (m_wRxHead<m_wRxTail)
   {
    if (m_wRxHead!=0)
     {
      for(i=0;i<m_wRxTail-m_wRxHead;i++)
      	m_RxBuf[i]=m_RxBuf[m_wRxHead+i] ;
      m_wRxTail-=m_wRxHead;
      m_wRxHead=0;
     } 
   }
  else
   m_wRxHead=m_wRxTail=0; 

  RNum=commRead(m_wTaskID,(m_RxBuf+m_wRxTail),MaxComBufSize-m_wRxTail,0);
  m_wRxTail+=RNum;
	
  if ((m_wRxTail-m_wRxHead)>=MINLEN)
    RecFrm();
}  

BOOL VCTSec::FindHead(void)
{
	while (m_wRxTail-m_wRxHead>=MINLEN)
	{
     tph=(struct TESTHEAD_t *)(m_RxBuf+m_wRxHead);
		 if((tph->sync[0]==0xeb)&&(tph->sync[1]==0x90)&&(tph->sync[2]==0xeb)&&(tph->sync[3]==0x90)&&(tph->sync[4]==0xeb)&&(tph->sync[5]==0x90)&&(tph->command&SEND)&&((tph->addr==0xFFFF)||(tph->addr==m_wAddr))) 
		   return(TRUE);
		 m_wRxHead++;
	}
	return(FALSE);
}
	
void VCTSec::RecFrm(void)
{
  int i;
  BYTE *data;
  struct VSysClock SystemClock;
  
  while (m_wRxTail-m_wRxHead>=MINLEN)
  {
  	if(FindHead()==FALSE) return;
    
    if(tph->length+MINLEN>m_wRxTail-m_wRxHead) return;
    
    data=(BYTE*)(tph+1);

    /*logMsg("len=%d\n",tph->length,0,0,0,0,0);*/
	
    for(i=0;i<tph->length;i++)
      if(data[i]!=i) break;
    
    if(i!=tph->length)
    {
      m_wRxHead++;
      continue;
    }
    EditTx();
    m_wRxHead+=(MINLEN+tph->length);
    if(tph->length==0)
    {
      m_byWpl=0;
    }
    else if(tph->length==0xff)
    {
      if(m_byWpl+1==0xff)
      {
     	GetSysClock((void *)&SystemClock,SYSCLOCK);
   	    shellprintf("%04d-%02d-%02d %02d:%02d:%02d 端口%d接收正确\n",SystemClock.wYear,SystemClock.byMonth,SystemClock.byDay,SystemClock.byHour,SystemClock.byMinute,SystemClock.bySecond,m_wTaskID);		
		shellprintf("%04d-%02d-%02d %02d:%02d:%02d 端口%d累计通断次数:%ld\n",SystemClock.wYear,SystemClock.byMonth,SystemClock.byDay,SystemClock.byHour,SystemClock.byMinute,SystemClock.bySecond,m_wTaskID,m_dwTotalBreakNum);
   	    m_byWpl=0;
      }
    }
    else if(m_byWpl+1!=tph->length)
   	  m_byWpl=0;
    else m_byWpl++;
  }
}           

void VCTSec::SDToBuf(void)
{
  int i;
  m_wTxHead=m_wTxTail=0;
  memcpy(m_TxBuf,m_RxBuf+m_wRxHead,MINLEN);
  m_TxBuf[8]&=(~SEND); 
  for(i=0;i<tph->length;i++)
    m_TxBuf[MINLEN+i]=i;   
  m_wTxTail=MINLEN+tph->length;    
  SendToTeam();
}
 
void VCTSec::SendToTeam(void)
{
  WORD i;

  /*if (m_wTxHead==0)
  {
	 struct TESTHEAD_t* tphTemp=(struct TESTHEAD_t*)m_TxBuf;
	 logMsg("send len=%d\n",	tphTemp->length,0,0,0,0,0);	 
  } 	
  else
	  logMsg("m_wTxHead=%d m_wTxTail=%d,\n",m_wTxHead,m_wTxTail,0,0,0,0);*/

  i=commWrite(m_wTaskID,(m_TxBuf+m_wTxHead),m_wTxTail-m_wTxHead,0);
  
  /*logMsg("i send=%d\n",i,0,0,0,0,0);*/
  m_wTxHead+=i;
}

void VCTSec::Txd(void)
{
  if (m_wTxHead<m_wTxTail)  SendToTeam();
}   

void VCTSec::EditTx(void)
{
  SDToBuf();
}

void VCTSec::OnCommStatus(void)
{  
  commCtrl(m_wCommID, CCC_CONNECT_CTRL|CONNECT_STATUS_GET, (BYTE*)&m_dwConnectOK);
  if (m_dwConnectOK)
  {
	shellprintf("端口%d 嗯，恢复了!\n", m_wTaskID);
  } 
  else
  {
	m_dwTotalBreakNum++; 
	shellprintf("端口%d 气死我了!断啦!总计第%ld次\n", m_wTaskID, m_dwTotalBreakNum);
  } 
}

#endif
