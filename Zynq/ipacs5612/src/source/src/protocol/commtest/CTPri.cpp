/*------------------------------------------------------------------------
 Module:       	ctpri.cpp
 Author:        rover2
 Project:       ds3200
 State:			创建
 Creation Date:	2002-09-25
 Description:   cometest pri
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: CTPri.cpp,v $
 Revision 1.0  2002-11-25 09:55:45+08  amine
 Initial revision

------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef	INCLUDE_COMMTEST

#include <unistd.h>
#include "CTPri.h"


extern "C" void ctpri(WORD thid)
{  
  DWORD dwEvent;
  BOOL InitOk;
  VCTPri *pCTPri;
  
  pCTPri=new VCTPri();  
  
  InitOk=pCTPri->Init(thid);
  
  if (InitOk==FALSE)  
  {
  	thSuspend(thid);
  } 
  
  pCTPri->m_thid=thid;
  
  evSend(thid,EV_SCHEDULE);
  
  for(;;)
  {
     evReceive(thid,EV_TM1 | EV_RX_AVAIL| EV_TX_AVAIL | EV_SCHEDULE | EV_COMM_STATUS,&dwEvent);

	 //printf("dwEvent %08x \n",dwEvent);
	 
     if (dwEvent&EV_RX_AVAIL)
       pCTPri->Rxd();
	
	 if (dwEvent&EV_TX_AVAIL)
       pCTPri->Txd();

     if (dwEvent&EV_TM1)
	   pCTPri->OnTimeOut();
	   
	 if (dwEvent&EV_SCHEDULE)
	   pCTPri->Schedule();  

	 if (dwEvent&EV_COMM_STATUS)
   	   pCTPri->OnCommStatus();  	
  }	 
}   

VCTPri::VCTPri()
{
  EqpList=NULL;
  m_SyncWord[0]=0xEB;
  m_SyncWord[1]=0x90;
  m_nLength=-1;
  m_nCurEqpNo=0;
  memset(m_RxBuf,0,MaxComBufSize);
  m_wRxHead=0;
  m_wRxTail=0;
  memset(m_TxBuf,0,MaxComBufSize);
  m_wTxHead=0;
  m_wTxTail=0;
  m_wTxNum=0;
  m_wRxMeNum=0;
  m_wRxRespNum=0;
  m_dwTxTotalNum=0;
  m_dwRxTotalMeNum=0;
  m_dwRxTotalRespNum=0;
  m_dwTotalRxBadNum=0;
  m_dwTotalBreakNum=0;
  m_dwTotalTimeoutNum=0;
  m_dwConnectOK=1;
  m_nTimerID = -1;
  m_wEqpNum = 0;
  m_wCommID = 0;
  m_wTaskID = 0;
  tph = NULL;
  m_dwBaudrate = 9600;
  m_thid = 0;

}

VCTPri::~VCTPri()
{
} 

BOOL VCTPri::Init(WORD wTaskID)
{	
  m_wTaskID=wTaskID;
  m_wCommID = wTaskID;
  m_wTaskID = wTaskID;
  m_thid = wTaskID;
  return(TRUE);
}

void VCTPri::Rxd(void)
{
  int RNum;
  int i;
  if (m_wRxHead<m_wRxTail)
  {
    if (m_wRxHead!=0)
    {
      i=0;
      while(m_wRxHead<m_wRxTail)
      {
      	if(m_wRxHead >= MaxComBufSize)
			break;
		
        m_RxBuf[i]=m_RxBuf[m_wRxHead];
	    m_wRxHead++;
	    i++;
      }	
      m_wRxTail=(WORD)i;
      m_wRxHead=0;
    } 
  }
  else
   m_wRxHead=m_wRxTail=0; 

  /*logMsg("m_wRxHead=%d m_wRxTail=%d,\n",m_wRxHead,m_wRxTail,0,0,0,0); */

  RNum=commRead(m_wTaskID,(m_RxBuf+m_wRxTail),MaxComBufSize-m_wRxTail,0);
  m_wRxTail+=(WORD)RNum;
  /*logMsg("RNum=%d\n",RNum,0,0,0,0,0); */


  if (m_wRxTail>=((WORD)m_nLength+MINLEN))
    RecFrm();
}  

BOOL VCTPri::FindHead(void)
{
  tph=(struct TESTHEAD_t *)(m_RxBuf+m_wRxHead);
  if((tph->sync[0]==0xeb)  && (tph->sync[1]==0x90) && (tph->sync[2]==0xeb)  && (tph->sync[3]==0x90) && (tph->sync[4]==0xeb)  && (tph->sync[5]==0x90)) 
   return(TRUE);
  m_wRxHead++;
  return(FALSE);
} 

void VCTPri::RecFrm(void)
{
  int i;
  BOOL YesMe;
  BYTE *Data;
  while (m_wRxHead<m_wRxTail)
  {
    while ((m_wRxHead<m_wRxTail)&&(m_RxBuf[m_wRxHead]!=m_SyncWord[0]))
    {
      m_wRxHead++;
    }
    if ((m_wRxTail-m_wRxHead)<(m_nLength+MINLEN))
      return;
    if (FindHead()== TRUE) 
    {
    	if (tph->command&SEND)
     		YesMe=TRUE;
    	else
     		YesMe=FALSE; 
    	if(tph->length!=m_nLength)
     	{
   		   m_dwTotalRxBadNum++;
		   /*logMsg("m_dwTotalRxBadNum=%ld m_nLength=%d RecLen=%d\n",m_dwTotalRxBadNum,m_nLength,tph->length,0,0,0);*/
     	   m_wRxHead++;
     	}	
    	else
     	{	 
      	   Data=(BYTE*)(tph+1);
           for(i=0;i<m_nLength;i++)
        	 if (Data[i]!=i)
       	 	  break;
           if (i!=m_nLength)      
           {	
        	 m_wRxHead+=MINLEN+(WORD)i;
           } 
      	   else
       	   {
       		 if (YesMe==TRUE)
        	 {
        		m_wRxMeNum++;
        		m_dwRxTotalMeNum++;
        	 } 
       		 else
       	 	 {
       	 	    m_wRxRespNum++;
        	 	m_dwRxTotalRespNum++;	
         	 }  

             m_wRxHead+=MINLEN+(WORD)m_nLength;  
       	   }	 

		   /*logMsg("m_nLength=%d RecLen=%d\n",m_nLength,tph->length,0,0,0,0);*/

           if (YesMe==FALSE)   EditTx();
     	}
    }// end of if(findhead()) 
  }//end of while(head<tail)
}           

void VCTPri::SDToBuf(void)
{
  int i;
  BYTE *Data;
  struct TESTHEAD_t* tphTemp=(struct TESTHEAD_t*)m_TxBuf;
  m_wTxHead=m_wTxTail=0;
  tphTemp->sync[0]=0xEB;
  tphTemp->sync[1]=0x90;
  tphTemp->sync[2]=0xEB;
  tphTemp->sync[3]=0x90;
  tphTemp->sync[4]=0xEB;
  tphTemp->sync[5]=0x90;
  tphTemp->addr=0xFFFF;
  tphTemp->command=SEND; 
  tphTemp->length=(BYTE)m_nLength;
  Data=(BYTE*)(tphTemp+1);
  for(i=0;i<m_nLength;i++)
    Data[i]=(BYTE)i;
  m_wTxTail=(WORD)m_nLength+MINLEN;
  m_wTxNum++;
  m_dwTxTotalNum++;
  SendToTeam();
}
 
void VCTPri::SendToTeam(void)
{
  int i;

  /*if (m_wTxHead==0)
  {
     struct TESTHEAD_t* tphTemp=(struct TESTHEAD_t*)m_TxBuf;
	 logMsg("send len=%d\n",	tphTemp->length,0,0,0,0,0);	 
  }	 	
  else
  	 logMsg("m_wTxHead=%d m_wTxTail=%d,\n",m_wTxHead,m_wTxTail,0,0,0,0);*/	 
  i=commWrite(m_wTaskID,(m_TxBuf+m_wTxHead),m_wTxTail-m_wTxHead,0);
/*  	 logMsg("i send=%d\n",i,0,0,0,0,0);	 */
  m_wTxHead+=(WORD)i;
}

void VCTPri::Txd(void)
{
  if (m_wTxHead<m_wTxTail)  SendToTeam();
}   

void VCTPri::EditTx(void)
{
  struct VSysClock tsysclock;
  DWORD j;

  if(m_nTimerID != -1) //以防在tmEvAfter之前
  	tmDelete(m_thid,m_nTimerID);

  if (m_dwConnectOK==0) return;
  
  m_nLength++;  
  if (m_nLength>(MAXLEN-MINLEN))
  {
  	if(m_wTxNum)
   		j=(m_wRxMeNum*1000)/m_wTxNum; //shellprintf 不支持打印浮点数
   	else
		j = 0;
   	/*if (j>=97)
   	 j=100;*/
   	GetSysClock((void *)&tsysclock,SYSCLOCK);
    	shellprintf("%04d-%02d-%02d %02d:%02d:%02d 端口%d自发自收正确率:%d \n",tsysclock.wYear,tsysclock.byMonth,tsysclock.byDay,tsysclock.byHour,tsysclock.byMinute,tsysclock.bySecond,m_wTaskID,j);
	if(m_wTxNum)
		j=(m_wRxRespNum*1000)/m_wTxNum;
	else
		j = 0;
  	/*if (j>=97)
   	 j=100;*/
	{
		//if(m_wRxRespNum != m_wTxNum)
		//	WriteDoEvent(NULL, 0, "收发不一致 %d %d ", m_wRxRespNum,m_wTxNum);
	  shellprintf("%04d-%02d-%02d %02d:%02d:%02d 端口%d应答正确率:%d \n",tsysclock.wYear,tsysclock.byMonth,tsysclock.byDay,tsysclock.byHour,tsysclock.byMinute,tsysclock.bySecond,m_wTaskID,j);
      shellprintf("%04d-%02d-%02d %02d:%02d:%02d 端口%d累计通断次数:%ld\n",tsysclock.wYear,tsysclock.byMonth,tsysclock.byDay,tsysclock.byHour,tsysclock.byMinute,tsysclock.bySecond,m_wTaskID,m_dwTotalBreakNum);	    
	  shellprintf("%04d-%02d-%02d %02d:%02d:%02d 端口%d累计超时次数:%ld\n",tsysclock.wYear,tsysclock.byMonth,tsysclock.byDay,tsysclock.byHour,tsysclock.byMinute,tsysclock.bySecond,m_wTaskID,m_dwTotalTimeoutNum);	     
	}  	  
   	m_wTxNum=0;
   	m_wRxMeNum=0;
   	m_wRxRespNum=0;
   	m_nLength=0;
  }	 

 // FrmLen=MINLEN+m_nLength;
  SDToBuf();

   m_nTimerID=tmEvAfter(m_wTaskID,500,EV_TM1);
} 
 
void VCTPri::OnTimeOut(void)
{
  m_dwTotalTimeoutNum++;
  EditTx();
} 
 
void VCTPri::Schedule(void)
{
  EditTx();
}

void VCTPri::OnCommStatus(void)
{  
  commCtrl(m_wCommID, CCC_CONNECT_CTRL|CONNECT_STATUS_GET, (BYTE*)&m_dwConnectOK);
  if (m_dwConnectOK)
  {
    shellprintf("嗯，恢复了!");
	m_nLength=0;  /*???*/
	EditTx();
  }	
  else
  {
    tmDelete(m_thid,m_nTimerID);
    m_dwTotalBreakNum++; 
	shellprintf("气死我了!断啦!总计第%ld次",m_dwTotalBreakNum);
  }	
}

#endif  
