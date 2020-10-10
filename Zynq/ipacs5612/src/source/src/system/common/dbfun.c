/*------------------------------------------------------------------------
 Module:       	dbfun.cpp
 Author:        solar
 Project:       
 State:         
 Creation Date:	2008-09-01
 Description:   
------------------------------------------------------------------------*/

#include "syscfg.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sys.h"

static struct VMsg msgbuf;
extern struct VSysInfo g_Sys;  /*系统信息*/
extern struct VTask *g_Task;  /*所有任务信息*/

DWORD dwDYXSem;
DWORD dwSYXSem;
//DWORD dwEVYXSem;
DWORD dwYCSem;
DWORD dwDDSem;
#define YKH 0x5
#define YKF 0x6

void GetValueFlag(DWORD dwCfg, BYTE *pFlag)
{
	if (dwCfg&0x80000000)  //闭锁
		*pFlag |= 0x02;	 
	else
		*pFlag &= (~0x02);	 
#ifdef INCLUDE_FLAG_CUR	  
	if (dwCfg&0x40000000)  //非当前值
		*pFlag |= 0x08;
	else
		*pFlag &= (~0x08);	 
#endif	  		
	if (dwCfg&0x20000000)  //被取代
		*pFlag |= 0x04;	
	else
		*pFlag &= (~0x04);	
}



/*------------------------------------------------------------------------
 Procedure:     GetYXValue ID:1
 Purpose:       get yx value from veqp 不经过与或操作的值
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS GetYXValue1(struct VVirtualEqp *pVEqp,WORD wNO,BYTE *pbyValue1,BYTE *pbyValue2,BYTE byFlag)
{
	BYTE value1,value2;
	WORD teid,offset;
	struct VTrueDYX *pDYX;
  struct VTrueYX *pSYX;
  struct VTrueYXCfg *pDYXCfg;
  struct VTrueYXCfg *pSYXCfg;
	
	if (byFlag==1)
	{
		teid=pVEqp->pDYX[wNO].wTEID;
		offset=pVEqp->pDYX[wNO].wOffset;      
	}								
	else  if (byFlag==2)
	{
		teid=pVEqp->pSYX[wNO].wTEID;
		offset=pVEqp->pSYX[wNO].wOffset;      
	}	
	if (g_Sys.Eqp.pInfo[teid].pTEqpInfo==NULL) return(ERROR);
	
 if (byFlag==1)
	{
		if ((pDYX=g_Sys.Eqp.pInfo[teid].pTEqpInfo->pDYX)==NULL) return(ERROR);
		pDYXCfg = g_Sys.Eqp.pInfo[teid].pTEqpInfo->pDYXCfg;	
		value1 = pDYX[offset].byValue1;
		value2 = pDYX[offset].byValue2;
		GetValueFlag(pDYXCfg[offset].dwCfg, &value1);
		GetValueFlag(pDYXCfg[offset].dwCfg, &value2);
	}
	 else  if (byFlag==2)
	{
		if ((pSYX=g_Sys.Eqp.pInfo[teid].pTEqpInfo->pSYX)==NULL) return(ERROR);      
		pSYXCfg = g_Sys.Eqp.pInfo[teid].pTEqpInfo->pSYXCfg;
		value1 = pSYX[offset].byValue;
		GetValueFlag(pSYXCfg[offset].dwCfg, &value1);
	}
		
	if (pbyValue1 != NULL) *pbyValue1=value1;
	if (pbyValue2 != NULL) *pbyValue2=value2;  
	return(OK);	
}




/*------------------------------------------------------------------------
 Procedure:     GetYXValue ID:1
 Purpose:       get yx value from veqp
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS GetYXValue(struct VVirtualEqp *pVEqp,WORD wNO,BYTE *pbyValue1,BYTE *pbyValue2,BYTE byFlag)
{
  BYTE value,value11,value22,value1,value2,tmp1,tmp2;
  struct VTrueDYX *pDYX;
  struct VTrueYX *pSYX;
  struct VTrueYXCfg *pDYXCfg;
  struct VTrueYXCfg *pSYXCfg;
  int i;
  WORD startno,num,teid,offset;
  DWORD cfg;

  if (byFlag==1)
  {
    if (pVEqp->pDYX==NULL) return(ERROR);
    startno=pVEqp->pDYX[wNO].wStartNo;
    num=pVEqp->pDYX[wNO].wNum;
    cfg=pVEqp->pDYX[wNO].dwCfg;
  }								
  else  if (byFlag==2)
  {
    if (pVEqp->pSYX==NULL) return(ERROR);
    startno=pVEqp->pSYX[wNO].wStartNo;
    num=pVEqp->pSYX[wNO].wNum;
    cfg=pVEqp->pSYX[wNO].dwCfg;
  }		

  if (cfg&0x01)  //与操作
  {
    value1=0x81;
    value2=0x81;
  }  
  else  //或操作
  {
    value1=0x01;
    value2=0x01;
  }  
  	
  for (i=startno;i<(startno+num);i++)
  {
    if (byFlag==1)
    {
      teid=pVEqp->pDYX[i].wTEID;
      offset=pVEqp->pDYX[i].wOffset;      
    }								
    else  if (byFlag==2)
    {
      teid=pVEqp->pSYX[i].wTEID;
      offset=pVEqp->pSYX[i].wOffset;      
    }								
					
    if (g_Sys.Eqp.pInfo[teid].pTEqpInfo==NULL) return(ERROR);

    if (byFlag==1)
    {
      if ((pDYX=g_Sys.Eqp.pInfo[teid].pTEqpInfo->pDYX)==NULL) return(ERROR);
	  pDYXCfg = g_Sys.Eqp.pInfo[teid].pTEqpInfo->pDYXCfg;
	  value11 = pDYX[offset].byValue1;
	  value22 = pDYX[offset].byValue2;
	  GetValueFlag(pDYXCfg[offset].dwCfg, &value11);
	  GetValueFlag(pDYXCfg[offset].dwCfg, &value22);
      if (cfg&0x01) //与操作
      { 
        tmp1=value11&0x7e;
        tmp2=(value1|tmp1)&0x7e;        
        tmp1=value11&0x81;
        value1&=tmp1;
        value1|=tmp2;
        tmp1=value22&0x7e;
        tmp2=(value2|tmp1)&0x7e;        
        tmp1=value22&0x81;
        value2&=tmp1;
        value2|=tmp2;
      }  
      else  //或操作
      {
        tmp1=value11&0xfe;
        tmp2=(value1|tmp1)&0xfe;        
        tmp1=value11&0x01;
        value1&=tmp1;
        value1|=tmp2;
        tmp1=value22&0xfe;
        tmp2=(value2|tmp1)&0xfe;        
        tmp1=value22&0x01;
        value2&=tmp1;
        value2|=tmp2;
      }
    }  
    else  if (byFlag==2)
    {
      if ((pSYX=g_Sys.Eqp.pInfo[teid].pTEqpInfo->pSYX)==NULL) return(ERROR);      
	  pSYXCfg = g_Sys.Eqp.pInfo[teid].pTEqpInfo->pSYXCfg;
      value = pSYX[offset].byValue;
	  GetValueFlag(pSYXCfg[offset].dwCfg, &value);
	  if (cfg&0x01) //与操作
      { 
        tmp1=value&0x7e;
        tmp2=(value1|tmp1)&0x7e;        
        tmp1=value&0x81;
        value1&=tmp1;
        value1|=tmp2;
		
      }  
      else  //或操作
      {
        tmp1=value&0xfe;
        tmp2=(value1|tmp1)&0xfe;        
        tmp1=value&0x01;
        value1&=tmp1;
        value1|=tmp2;
      }
    }  
  }

  if (pbyValue1 != NULL) *pbyValue1=value1;
  if (pbyValue2 != NULL) *pbyValue2=value2;  
  return(OK);
}  

/*------------------------------------------------------------------------
 Procedure:     GetYCValue ID:1
 Purpose:       get yc value from veqp
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS GetYCValue(struct VVirtualEqp *pVEqp,WORD wNo,long *pnValue,BYTE *pbyFlag)
{
  long lvalue;
  float fvalue1, fvalue;
  BYTE flag,tmp1,tmp2;
  struct VTrueYC *pYC;
  struct VTrueYCCfg *pYCCfg;
  int i;
  WORD startno,num,teid,offset;
  BYTE *pnValue_temp = (BYTE*)pnValue;
	
	
  if (pVEqp->pYC==NULL) return(ERROR);
  startno=pVEqp->pYC[wNo].wStartNo;
  num=pVEqp->pYC[wNo].wNum;

  lvalue=0;
  fvalue=0;
  flag=0x01;
  
  for (i=startno;i<(startno+num);i++)
  {
    teid=pVEqp->pYC[i].wTEID;
    offset=pVEqp->pYC[i].wOffset;      
					
    if (g_Sys.Eqp.pInfo[teid].pTEqpInfo==NULL) return(ERROR);

    if ((pYC=g_Sys.Eqp.pInfo[teid].pTEqpInfo->pYC)==NULL) return(ERROR);
	pYCCfg = g_Sys.Eqp.pInfo[teid].pTEqpInfo->pYCCfg;

	if (pYC[offset].byFlag & 0x40)  //浮点格式
	{
		memcpy(&fvalue1,  &pYC[offset].lValue, sizeof(fvalue1));
		fvalue += fvalue1;
	}		
	else
    	lvalue+=pYC[offset].lValue;
    tmp1=pYC[offset].byFlag&0xfe;
    tmp2=(flag|tmp1)&0xfe;        
    tmp1=pYC[offset].byFlag&0x01;
    flag&=tmp1;
    flag|=tmp2;
	if (pVEqp->pYC[i].dwCfg&0x200)  //active send
		flag |= 0x80;
	GetValueFlag(pYCCfg[offset].dwCfg, &flag);
  }    

  if(pVEqp->Cfg.dwFlag & YCQUOTIETYENABLED)
  {
	if (flag & 0x40)  //浮点格式
		fvalue = pVEqp->pYC[wNo].lA*fvalue/pVEqp->pYC[wNo].lB+pVEqp->pYC[wNo].lC;
	else
		lvalue = pVEqp->pYC[wNo].lA*lvalue/pVEqp->pYC[wNo].lB+pVEqp->pYC[wNo].lC;
  }
	
  if (flag & 0x40)    //浮点格式
	memcpy(pnValue_temp, &fvalue, sizeof(fvalue));
  else
    memcpy(pnValue_temp,&lvalue,sizeof(lvalue));
	
  *pbyFlag=flag;  
  
  return(OK);
}  

/*------------------------------------------------------------------------
 Procedure:     GetDDValue ID:1
 Purpose:       get dd value from veqp
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS GetDDValue(struct VVirtualEqp *pVEqp,WORD wNo,long *plValue,BYTE *pbyFlag,struct VCalClock *pTime)
{
  long value;  
  BYTE flag,tmp1,tmp2;
  struct VTrueDD *pDD;
  int i;
  WORD startno,num,teid,offset;
  DWORD cfg;
  BYTE* plValue_temp = (BYTE*)plValue;
	
	
  if (pVEqp->pDD==NULL) return(ERROR);
  startno=pVEqp->pDD[wNo].wStartNo;
  num=pVEqp->pDD[wNo].wNum;
  cfg=pVEqp->pDD[wNo].dwCfg;

  value=0;
  flag=0x01;
  
  for (i=startno;i<(startno+num);i++)
  {
    teid=pVEqp->pDD[i].wTEID;
    offset=pVEqp->pDD[i].wOffset;      
					
    if (g_Sys.Eqp.pInfo[teid].pTEqpInfo==NULL) return(ERROR);

    if ((pDD=g_Sys.Eqp.pInfo[teid].pTEqpInfo->pDD)==NULL) return(ERROR);

    if (cfg&0x01)  //op -
      value-=pDD[offset].lValue;
    else  //op +
      value+=pDD[offset].lValue;	

    tmp1=pDD[offset].byFlag&0xfe;
    tmp2=(flag|tmp1)&0xfe;        
    tmp1=pDD[offset].byFlag&0x01;
    flag&=tmp1;
    flag|=tmp2;
	*pTime=pDD[offset].Time;
  }    

	memcpy(plValue_temp,&value,sizeof(value));

  *pbyFlag=flag;  
  
  return(OK);
}  

/*------------------------------------------------------------------------
 Procedure: 	GetpRunInfo ID:1
 Purpose:		取得装置的运行信息指针
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
struct VEqpRunInfo *GetpRunInfo(WORD wTaskID,WORD wEqpID,int *flag)
{
   int i;
   struct VTask *pTask;

   if (wTaskID>THREAD_MAX_NUM) return(NULL);

   if (wEqpID>=*g_Sys.Eqp.pwNum)  return(NULL);

   pTask=g_Task+wTaskID;

   if (flag != NULL) *flag = 0;
   for (i=0;i<pTask->wEqpGroupNum;i++)
	 if (wEqpID==pTask->pEqpGroupRunInfo[i].wEqpID)
	 {
	   if (flag != NULL) *flag = RI_FLAG_EG;
	   return(pTask->pEqpGroupRunInfo+i);
	 }

   for (i=0;i<pTask->wGroupEqpNum;i++)
	 if (wEqpID==pTask->pGroupEqpRunInfo[i].wEqpID)
	 {
	   if (flag != NULL) *flag = RI_FLAG_GE;
	   return(pTask->pGroupEqpRunInfo+i);
	 }	

   for (i=0;i<pTask->wEqpNum;i++)
	 if (wEqpID==pTask->pEqpRunInfo[i].wEqpID)
	 {
		if (flag != NULL) *flag = RI_FLAG_EQP;
		return(pTask->pEqpRunInfo+i);
	 }	

   return(NULL);
}

/*------------------------------------------------------------------------
 Procedure:     GetReadPtr ID:1
 Purpose:       GetReadPtr
 Input:
 Output:        
 Errors:
------------------------------------------------------------------------*/
STATUS GetReadPtr(WORD wTaskID,WORD wEqpID,DWORD dwUFlag,WORD **ppwReadPtr)
{
   struct VEqpRunInfo *pRunInfo;

   if ((pRunInfo=GetpRunInfo(wTaskID,wEqpID,NULL))==NULL)  return(ERROR);

   switch (dwUFlag)
   {
   	   case DSOEUFLAG:
   	     *ppwReadPtr=&pRunInfo->wDSOEReadPtr;
   	     break;
       case DCOSUFLAG:
   	     *ppwReadPtr=&pRunInfo->wDCOSReadPtr;
   	     break;
   	   case SSOEUFLAG:
   	     *ppwReadPtr=&pRunInfo->wSSOEReadPtr;
   	     break;
   	   case SCOSUFLAG:
   	     *ppwReadPtr=&pRunInfo->wSCOSReadPtr;
   	     break;
   	   case FAINFOUFLAG:
   	     *ppwReadPtr=&pRunInfo->wFAInfoReadPtr;
		 break;
	   case ACTEVENTUFLAG:
	     *ppwReadPtr=&pRunInfo->wActEventReadPtr;
	     break;
	   case DOEVENTUFLAG:
		 *ppwReadPtr=&pRunInfo->wDoEventReadPtr;
		 break;
	   case WARNEVENTUFLAG:
		 *ppwReadPtr=&pRunInfo->wWarnEventReadPtr;
		 break;
   	   default:
   	  	 return(ERROR);
   }   	
   
   return(OK);
}

#ifdef INCLUDE_B2F
extern struct VSysEventFlag sysEventFlag[];
void WritePollBuf2File(struct VPool *pPool, BYTE *pBuf, WORD wID, int flag)
{
	VB2FSocket socket;

	if ((flag<=0) || (flag>=MAX_SYSEV_FLAG_NUM)) return;

	if ((flag == SYSEV_FLAG_DSOE) || (flag == SYSEV_FLAG_SSOE))
	{
		sprintf(socket.fname, sysEventFlag[flag].sFName, g_Sys.Eqp.pInfo[wID].sCfgName);
	}
#ifdef INCLUDE_B2F_COS
	else if ((flag == SYSEV_FLAG_DCOS) || (flag == SYSEV_FLAG_SCOS))
	{
		sprintf(socket.fname, sysEventFlag[flag].sFName, g_Sys.Eqp.pInfo[wID].sCfgName);
	}
#endif
	else
	    strcpy(socket.fname, sysEventFlag[flag].sFName);
    
	socket.read_write = 2;
#ifdef INCLUDE_B2F_F
	socket.offset = 0;
	socket.buf = (BYTE *)pPool;
	socket.len = sizeof(WORD)*2;
	Buf2FileWrite(&socket);
#endif
	socket.offset = sizeof(WORD)*2 + (DWORD)(pBuf - (BYTE *)pPool->pHead);
	socket.buf = pBuf;
	socket.len = sysEventFlag[flag].nCellLen;
	Buf2FileWrite(&socket);	   
}
#endif


#ifdef INCLUDE_B2F_F
extern char* riFName[];
void WriteRunInfo2File(WORD wTaskID,  struct VEqpRunInfo *pInfo, DWORD dwUFlag, int flag)
{
	VB2FSocket socket;

#ifndef INCLUDE_B2F_COS
    if ((dwUFlag==DCOSUFLAG) || (dwUFlag==SCOSUFLAG))  return;
#endif

	if ((flag <= 0) || (flag>=MAX_RI_FLAG_NUM)) return;

	if (flag == RI_FLAG_EG)
	{	
		socket.offset = (BYTE *)pInfo - (BYTE *)g_Task[wTaskID].pEqpGroupRunInfo;
	}
	else if (flag == RI_FLAG_GE)
	{
		socket.offset = (BYTE *)pInfo - (BYTE *)g_Task[wTaskID].pGroupEqpRunInfo;
	}	
	else if (flag == RI_FLAG_EQP)
	{
		socket.offset = (BYTE *)pInfo - (BYTE *)g_Task[wTaskID].pEqpRunInfo;
	}	

	sprintf(socket.fname, riFName[flag], wTaskID);	 
	socket.read_write = 2;
	socket.buf = (BYTE *)pInfo;
	socket.len = sizeof(struct VEqpRunInfo);
	Buf2FileWrite(&socket);
}
#endif

void writeTEqpFlag(WORD wEqpID, DWORD dwFlag)
{
	WORD taskid;
	struct VTask *pTask;  
	struct VTrueEqp *pTEqp;
	int j;
   pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo;
	if (pTEqp!=NULL)  
	{
		taskid=g_Sys.Eqp.pInfo[wEqpID].wTaskID;
		pTask=g_Task+taskid;

		for (j=0;j<pTask->wEqpNum;j++)
		if (wEqpID==pTask->pEqpRunInfo[j].wEqpID)
		{
		  if ((pTask->pEqpRunInfo[j].dwUFlag&dwFlag) == 0)   //判断减少文件读写调用
		  {
			pTask->pEqpRunInfo[j].dwUFlag|=dwFlag;
#ifdef INCLUDE_B2F_F
			WriteRunInfo2File(taskid, pTask->pEqpRunInfo+j, dwFlag, RI_FLAG_EG);  
#endif
		  } 
		  break;
		} 
		evSend(taskid, EV_UFLAG); 
	}
}



/*------------------------------------------------------------------------
 Procedure:     writeFlag ID:1
 Purpose:       设置装置flag并发送EV_UFLAG给装置相关的任务
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void writeFlag(WORD wEqpID,DWORD dwFlag)
{
  WORD taskid,egid;
  struct VTask *pTask;  
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  /*struct VEqpGroup *pEqpG;*/
  int i,j;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)!=NULL)  
  {
    for (i=0;i<pTEqp->Cfg.wInvRefEGNum;i++)
    {
      egid=pTEqp->pwInvRefEGID[i];
      taskid=g_Sys.Eqp.pInfo[egid].wTaskID;
      pTask=g_Task+taskid;

      for (j=0;j<pTask->wEqpGroupNum;j++)
        if (egid==pTask->pEqpGroupRunInfo[j].wEqpID)
        {
		  if ((pTask->pEqpGroupRunInfo[j].dwUFlag&dwFlag) == 0)   //判断减少文件读写调用
		  {
		    pTask->pEqpGroupRunInfo[j].dwUFlag|=dwFlag;
#ifdef INCLUDE_B2F_F
		    WriteRunInfo2File(taskid, pTask->pEqpGroupRunInfo+j, dwFlag, RI_FLAG_EG);  
#endif
		  } 
          break;
        }  

	  for (j=0;j<pTask->wGroupEqpNum;j++)
		if (wEqpID==pTask->pGroupEqpRunInfo[j].wEqpID)
		{ 
		  if ((pTask->pGroupEqpRunInfo[j].dwUFlag&dwFlag) == 0)
		  {
		    pTask->pGroupEqpRunInfo[j].dwUFlag|=dwFlag;
#ifdef INCLUDE_B2F_F
		    WriteRunInfo2File(taskid, pTask->pGroupEqpRunInfo+j, dwFlag, RI_FLAG_GE);
#endif
		  }
		  break;
		} 

      evSend(taskid, EV_UFLAG);      
    }
  }
  else  if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)!=NULL)  
  {
    for (i=0;i<pVEqp->Cfg.wInvRefEGNum;i++)
    {
      egid=pVEqp->pwInvRefEGID[i];
      taskid=g_Sys.Eqp.pInfo[egid].wTaskID;
      pTask=g_Task+taskid;

      for (j=0;j<pTask->wEqpGroupNum;j++)
        if (egid==pTask->pEqpGroupRunInfo[j].wEqpID)
        {
		  if ((pTask->pEqpGroupRunInfo[j].dwUFlag&dwFlag) == 0)
		  {
		    pTask->pEqpGroupRunInfo[j].dwUFlag|=dwFlag;
#ifdef INCLUDE_B2F_F
		    WriteRunInfo2File(taskid, pTask->pEqpGroupRunInfo+j, dwFlag, RI_FLAG_EG);
#endif
		  }
          break;
        }  

	  for (j=0;j<pTask->wGroupEqpNum;j++)
		if (wEqpID==pTask->pGroupEqpRunInfo[j].wEqpID)
		{ 
		  if ((pTask->pGroupEqpRunInfo[j].dwUFlag&dwFlag) == 0)
		  {
		    pTask->pGroupEqpRunInfo[j].dwUFlag|=dwFlag;
#ifdef INCLUDE_B2F_F
		    WriteRunInfo2File(taskid, pTask->pGroupEqpRunInfo+j, dwFlag, RI_FLAG_GE);
#endif
		  }
		  break;
		} 

      evSend(taskid, EV_UFLAG);      
    }
  }
  /*else if ((pEqpG=g_Sys.Eqp.pInfo[wEqpID].pEqpGInfo)!=NULL)*/
  else if (g_Sys.Eqp.pInfo[wEqpID].pEqpGInfo!=NULL)
  {
    taskid=g_Sys.Eqp.pInfo[wEqpID].wTaskID;
    pTask=g_Task+taskid;

    for (j=0;j<pTask->wEqpGroupNum;j++)
      if (wEqpID==pTask->pEqpGroupRunInfo[j].wEqpID)
      {
		 if ((pTask->pEqpGroupRunInfo[j].dwUFlag&dwFlag) == 0)
		 {
		   pTask->pEqpGroupRunInfo[j].dwUFlag|=dwFlag;
#ifdef INCLUDE_B2F_F
		   WriteRunInfo2File(taskid, pTask->pGroupEqpRunInfo+j, dwFlag, RI_FLAG_EG);
#endif
		 }
		 break;
      }  
      
    evSend(taskid, EV_UFLAG);
  }
}

/*------------------------------------------------------------------------
 Procedure:     atomicWriteDCOS ID:1
 Purpose:       写双点COS子函数
 Input:         semflag:根据其可知是否被应用程序直接调用还是被api函数嵌套
                调用
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteDCOS(WORD wEqpID,WORD wNum,struct VDBDCOS *buf,BOOL semflag,BOOL bLock) 
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  struct VDBDCOS * p=buf;
  struct VTrueDYX *pDYX;
  struct VTrueYXCfg *pCfg;
  struct VDBDCOS *pDBDCOS;
  WORD wVEqpID,wVNo;
  int i,j,maxno;
  BYTE value1,value2,oldvalue1,oldvalue2,newvalue1,newvalue2;
  BOOL write;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pDYX==NULL)||(pTEqp->pDYXCfg==NULL)||(pTEqp->DCOS.pPoolHead==NULL))  return;

  maxno=pTEqp->Cfg.wDYXNum-1;
  if (maxno<0)  return;

  if (semflag) smMTake(dwDYXSem);
  	  
  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno)||((p->byValue1&0x01)==0)||((p->byValue2&0x01)==0))
    {
     p++;
     continue;
    } 
    
    pCfg=pTEqp->pDYXCfg+p->wNo;
    pDYX=pTEqp->pDYX+p->wNo;       

	if (bLock&&(pCfg->dwCfg&0x80000000))	//正常写入，考虑闭锁
	{
	  p++;
	  continue;
	}

    if (pCfg->dwCfg&0x02)  //取反
    {
      p->byValue1^=0x80;
      p->byValue2^=0x80;
    }  

	GetValueFlag(pCfg->dwCfg, &p->byValue1);
	GetValueFlag(pCfg->dwCfg, &p->byValue2);
    
    if (pCfg->wSendNo!=0xFFFF)
    {
      pDBDCOS=pTEqp->DCOS.pPoolHead+pTEqp->DCOS.wWritePtr;
      pDBDCOS->wNo=p->wNo;
      pDBDCOS->byValue1=p->byValue1;
      pDBDCOS->byValue2=p->byValue2;
      pTEqp->DCOS.wWritePtr++;
      pTEqp->DCOS.wWritePtr%=pTEqp->DCOS.wPoolNum;
#ifdef INCLUDE_B2F_COS
	  WritePollBuf2File((struct VPool *)&pTEqp->DCOS, (BYTE *)pDBDCOS, wEqpID, SYSEV_FLAG_DCOS);
#endif	   
      writeFlag(wEqpID,DCOSUFLAG);
    }
    
    for (j=0;j<pCfg->wVENum;j++)
    {
       wVEqpID=pCfg->pInvRef[j].wEqpID;
       wVNo=pCfg->pInvRef[j].wNo;

       pVEqp=g_Sys.Eqp.pInfo[wVEqpID].pVEqpInfo;
       if (pVEqp==NULL) continue;
       if ((pVEqp->pDYX==NULL)||(pVEqp->DCOS.pPoolHead==NULL)) continue;

       value1=pDYX->byValue1;
       value2=pDYX->byValue2;
       pDYX->byValue1=p->byValue1;
       pDYX->byValue2=p->byValue2;
       if (GetYXValue(pVEqp,wVNo,&newvalue1,&newvalue2,1)==ERROR)
       {
          pDYX->byValue1=value1;
          pDYX->byValue2=value2;
          continue;	
       }  
       pDYX->byValue1=value1;
       pDYX->byValue2=value2;

       write=TRUE;       
       if (!semflag)    //compare    
       {
         GetYXValue(pVEqp,wVNo,&oldvalue1,&oldvalue2,1);
         if ((oldvalue1==newvalue1)&&(oldvalue2==newvalue2))
           write=FALSE;
       }  

       if (write)  //因为改写遥信有cos和yx两种途径，所以对于合并遥信会出现重复cos
       {
         pDBDCOS=pVEqp->DCOS.pPoolHead+pVEqp->DCOS.wWritePtr;
         pDBDCOS->wNo=pVEqp->pDYX[wVNo].wSendNo;
         pDBDCOS->byValue1=newvalue1;
         pDBDCOS->byValue2=newvalue2;
         pVEqp->DCOS.wWritePtr++;
         pVEqp->DCOS.wWritePtr%=pVEqp->DCOS.wPoolNum;
#ifdef INCLUDE_B2F_COS
		 WritePollBuf2File((struct VPool *)&pVEqp->DCOS, (BYTE *)pDBDCOS, wVEqpID, SYSEV_FLAG_DCOS);
#endif	   		 
         writeFlag(wVEqpID,DCOSUFLAG);
       }  
    }  
    
    if (semflag)
    {
      pDYX->byValue1=p->byValue1;
      pDYX->byValue2=p->byValue2;
    }  
    p++;
  }  
  
  if (semflag)  smMGive(dwDYXSem);    
}    


/*------------------------------------------------------------------------
 Procedure:     atomicWriteDSOE ID:1
 Purpose:       写双点SOE子函数
 Input:         semflag:根据其可知是否被应用程序直接调用还是被api函数嵌套
                调用
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteDSOE(WORD wEqpID,WORD wNum,struct VDBDSOE *buf,BOOL semflag,BOOL bLock) 
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  struct VDBDSOE * p=buf;
  struct VTrueDYX *pDYX;
  struct VTrueYXCfg *pCfg;
  struct VDBDSOE *pDBDSOE;
  WORD wVEqpID,wVNo;
  int i,j,maxno;
  BYTE value1,value2,oldvalue1,oldvalue2,newvalue1,newvalue2;
  BOOL write;

  if (wEqpID>=*g_Sys.Eqp.pwNum)    return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pDYX==NULL)||(pTEqp->pDYXCfg==NULL)||(pTEqp->DSOE.pPoolHead==NULL))  return;

  maxno=pTEqp->Cfg.wDYXNum-1;
  if (maxno<0)   return;

  if (semflag)  smMTake(dwDYXSem);
  	  
  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno)||((p->byValue1&0x01)==0)||((p->byValue2&0x01)==0))
    {
      p++;
      continue;
    } 

    pCfg=pTEqp->pDYXCfg+p->wNo;
    pDYX=pTEqp->pDYX+p->wNo;       

	if (bLock&&(pCfg->dwCfg&0x80000000))	//正常写入，考虑闭锁
	{
	  p++;
	  continue;
	}

    if (pCfg->dwCfg&0x02)  //取反
    {
      p->byValue1^=0x80;
      p->byValue2^=0x80;
    }  

	GetValueFlag(pCfg->dwCfg, &p->byValue1);
	GetValueFlag(pCfg->dwCfg, &p->byValue2);
    
    if (pCfg->wSendNo!=0xFFFF)
    {
       pDBDSOE=pTEqp->DSOE.pPoolHead+pTEqp->DSOE.wWritePtr;
       pDBDSOE->wNo=p->wNo;
       pDBDSOE->byValue1=p->byValue1;
       pDBDSOE->byValue2=p->byValue2;
       pDBDSOE->Time=p->Time;
       pTEqp->DSOE.wWritePtr++;
       pTEqp->DSOE.wWritePtr%=pTEqp->DSOE.wPoolNum;
#ifdef INCLUDE_B2F_SOE
	   WritePollBuf2File((struct VPool *)&pTEqp->DSOE, (BYTE *)pDBDSOE, wEqpID, SYSEV_FLAG_DSOE);
#endif	
       writeFlag(wEqpID,DSOEUFLAG);
    }
    
    for (j=0;j<pCfg->wVENum;j++)
    {
       wVEqpID=pCfg->pInvRef[j].wEqpID;
       wVNo=pCfg->pInvRef[j].wNo;

       pVEqp=g_Sys.Eqp.pInfo[wVEqpID].pVEqpInfo;
       if (pVEqp==NULL) continue;
       if ((pVEqp->pDYX==NULL)||(pVEqp->DSOE.pPoolHead==NULL)) continue;

       value1=pDYX->byValue1;
       value2=pDYX->byValue2;
       pDYX->byValue1=p->byValue1;
       pDYX->byValue2=p->byValue2;
       if (GetYXValue(pVEqp,wVNo,&newvalue1,&newvalue2,1)==ERROR)
       {
          pDYX->byValue1=value1;
          pDYX->byValue2=value2;
          continue;	
       }  
       pDYX->byValue1=value1;
       pDYX->byValue2=value2;

       write=TRUE;       
       if (!semflag)    //compare    
       {
         GetYXValue(pVEqp,wVNo,&oldvalue1,&oldvalue2,1);
         if ((oldvalue1==newvalue1)&&(oldvalue2==newvalue2))
           write=FALSE;
       }  

       if (write)  //因为改写遥信有cos和yx两种途径，所以对于合并遥信会出现重复soe
       {
         pDBDSOE=pVEqp->DSOE.pPoolHead+pVEqp->DSOE.wWritePtr;
         pDBDSOE->wNo=pVEqp->pDYX[wVNo].wSendNo;
         pDBDSOE->byValue1=newvalue1;
         pDBDSOE->byValue2=newvalue2;
         pDBDSOE->Time=p->Time;
         pVEqp->DSOE.wWritePtr++;
         pVEqp->DSOE.wWritePtr%=pVEqp->DSOE.wPoolNum;

#ifdef INCLUDE_B2F_SOE
	     WritePollBuf2File((struct VPool *)&pVEqp->DSOE, (BYTE *)pDBDSOE, wVEqpID, SYSEV_FLAG_DSOE);
#endif	   
#ifdef INCLUDE_HIS
		if(pVEqp->Cfg.dwFlag & 0x40)
			 his2file_write((BYTE*)pDBDSOE,wVEqpID,SYSEV_FLAG_DSOE);
#endif			
#ifdef INCLUDE_HIS_TTU
             if(pVEqp->Cfg.dwFlag & 0x40)
                histtu2file_write((BYTE*)pDBDSOE,wVEqpID,SYSEV_FLAG_DSOE);
#endif			

         writeFlag(wVEqpID,DSOEUFLAG);         
       }  
    }  
    
    p++;
  }  
  
  if (semflag)  smMGive(dwDYXSem);    
}    


/*------------------------------------------------------------------------
 Procedure:     atomicReadVEDYX ID:1
 Purpose:       读虚装置离散点双点遥信子函数(一字节代表一遥信) 
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEDYX(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBDYX *buf)
{
  int i,j,k,maxno;
  struct VDBDYX *p,*q;
  
  maxno=pVEqp->Cfg.wDYXNum-1; 
  if (maxno<0)  return(0);

  smMTake(dwDYXSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    q->wNo=p->wNo;
    if (GetYXValue(pVEqp,q->wNo,&q->byValue1,&q->byValue2,1)==ERROR)
    {
      p++;
      continue;
    }      	
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBDYX);
    if (k>wBufLen-sizeof(struct VDBDYX)) break;    
  }  
  
  smMGive(dwDYXSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadTEDYX ID:1
 Purpose:       读实装置离散点双点遥信子函数(一字节代表一遥信) 
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEDYX(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBDYX *buf,BOOL asSend)
{
  int i,j,k,maxno;
  struct VDBDYX *p,*q;
  WORD no;

  if (asSend)
    maxno=pTEqp->DYXSendCfg.wNum-1;
  else
    maxno=pTEqp->Cfg.wDYXNum-1;
 
  if (maxno<0) return(0);

  smMTake(dwDYXSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    if (asSend)
   	  no=pTEqp->DYXSendCfg.pwIndex[p->wNo];
    else
   	  no=p->wNo;
    q->wNo=p->wNo;
    q->byValue1=pTEqp->pDYX[no].byValue1;
    q->byValue2=pTEqp->pDYX[no].byValue2;
	GetValueFlag(pTEqp->pDYXCfg[no].dwCfg, &q->byValue1);
	GetValueFlag(pTEqp->pDYXCfg[no].dwCfg, &q->byValue2);
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBDYX);
    if (k>wBufLen-sizeof(struct VDBDYX)) break;
  }  

  smMGive(dwDYXSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadDYX ID:1
 Purpose:       读离散点双点遥信子函数(一字节代表一遥信) 
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadDYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDYX *buf,BOOL asSend)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBDYX)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if (pVEqp->pDYX==NULL)
        return(0);
      else
        j=atomicReadVEDYX(pVEqp,wNum,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->pDYX==NULL)||(pTEqp->pDYXCfg==NULL))
      return(0);
    else
      j=atomicReadTEDYX(pTEqp,wNum,wBufLen,buf,asSend);
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadVEDCOS ID:1
 Purpose:    读虚装置双点COS子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEDCOS(WORD wTaskID,WORD wEqpID,struct VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBDCOS *buf)
{
  int i,j,k;
  WORD rp,wp,poolnum,num;
  struct VDBDCOS *pDBDCOS,*p=buf;
  struct VEqpRunInfo *pRunInfo;
    
  smMTake(dwDYXSem);  

  wp=pVEqp->DCOS.wWritePtr;
  poolnum=pVEqp->DCOS.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {
     pRunInfo=GetpRunInfo(wTaskID, wEqpID, NULL);
     if (pRunInfo!=NULL)  pRunInfo->dwUFlag&=(~DCOSUFLAG);

	smMGive(dwDYXSem);
	return(0);       
  }
  
  if (num>wNum) num=wNum;	

  j=k=0;
  for (i=rp;i<(rp+num);i++)
  {    
    pDBDCOS=pVEqp->DCOS.pPoolHead+(i % poolnum);
    if ((pDBDCOS->byValue1==0) || (pDBDCOS->byValue2==0))
    {
		(*pwReadPtr)++;
        continue;
	}
    p->wNo=pDBDCOS->wNo;
    p->byValue1=pDBDCOS->byValue1;
    p->byValue2=pDBDCOS->byValue2;
    p++; 
    j++;
    k+=sizeof(struct VDBDCOS);
    if (k>wBufLen-sizeof(struct VDBDCOS)) break;      
  }

  smMGive(dwDYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadTEDCOS ID:1
 Purpose:    读实装置双点COS子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEDCOS(WORD wTaskID,WORD wEqpID,struct VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBDCOS *buf)
{
  int i,j,k;
  WORD rp,wp,poolnum,num;
  struct VDBDCOS *pDBDCOS,*p=buf;
  struct VEqpRunInfo *pRunInfo;

  smMTake(dwDYXSem);  

  wp=pTEqp->DCOS.wWritePtr;
  poolnum=pTEqp->DCOS.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {	
    pRunInfo=GetpRunInfo(wTaskID, wEqpID, NULL);
	if (pRunInfo!=NULL)  pRunInfo->dwUFlag&=(~DCOSUFLAG);

	smMGive(dwDYXSem);
	return(0);       
  }
	
  if (num>wNum) num=wNum;
  
  j=k=0;  
  for (i=rp;i<(rp+num);i++)
  {
    pDBDCOS=pTEqp->DCOS.pPoolHead+(i % poolnum);  
    if ((pDBDCOS->byValue1==0) || (pDBDCOS->byValue2==0))
    {
		(*pwReadPtr)++;
        continue;
	}
    p->wNo=pTEqp->pDYXCfg[pDBDCOS->wNo].wSendNo;
    if (p->wNo==0xFFFF)
   	  continue;
    p->byValue1=pDBDCOS->byValue1;
    p->byValue2=pDBDCOS->byValue2;
    p++;
    j++;   
    k+=sizeof(struct VDBDCOS);
    if (k>wBufLen-sizeof(struct VDBDCOS)) break;      
  }

  smMGive(dwDYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadVEDSOE ID:1
 Purpose:    读虚装置双点SOE子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEDSOE(WORD wTaskID,WORD wEqpID,struct VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBDSOE *buf)
{
  int i,j,k,flag;
  WORD rp,wp,poolnum,num;
  struct VDBDSOE *pDBDSOE,*p=buf;
  struct VEqpRunInfo *pRunInfo;
  
  smMTake(dwDYXSem);  

  wp=pVEqp->DSOE.wWritePtr;
  poolnum=pVEqp->DSOE.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {		
	pRunInfo=GetpRunInfo(wTaskID, wEqpID, &flag);
	if ((pRunInfo!=NULL) && (pRunInfo->dwUFlag&DSOEUFLAG))
	{
		pRunInfo->dwUFlag&=(~DSOEUFLAG);
#ifdef INCLUDE_B2F_F
		WriteRunInfo2File(wTaskID, pRunInfo, DSOEUFLAG, flag);
#endif	
	}

	smMGive(dwDYXSem);
	return(0);       
  }
	
  if (num>wNum) num=wNum;
  
  j=k=0;
  for (i=rp;i<(rp+num);i++)
  {
    pDBDSOE=pVEqp->DSOE.pPoolHead+(i % poolnum);
    if ((pDBDSOE->byValue1==0) || (pDBDSOE->byValue2==0))
    {
		(*pwReadPtr)++;
        continue;
	}
    p->wNo=pDBDSOE->wNo;
    p->byValue1=pDBDSOE->byValue1;
    p->byValue2=pDBDSOE->byValue2;
    p->Time=pDBDSOE->Time;
    p++; 
    j++;
    k+=sizeof(struct VDBDSOE);
    if (k>wBufLen-sizeof(struct VDBDSOE)) break;
  }

  smMGive(dwDYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadTEDSOE ID:1
 Purpose:    读实装置双点SOE子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEDSOE(WORD wTaskID,WORD wEqpID,struct VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBDSOE *buf)
{
  int i,j,k,flag;
  WORD rp,wp,poolnum,num;
  struct VDBDSOE *pDBDSOE,*p=buf;
  struct VEqpRunInfo *pRunInfo;
  
  smMTake(dwDYXSem);  

  wp=pTEqp->DSOE.wWritePtr;
  poolnum=pTEqp->DSOE.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {			
	pRunInfo=GetpRunInfo(wTaskID, wEqpID, &flag);
	if ((pRunInfo!=NULL) && (pRunInfo->dwUFlag&DSOEUFLAG))
	{
		pRunInfo->dwUFlag&=(~DSOEUFLAG);
#ifdef INCLUDE_B2F_F
		WriteRunInfo2File(wTaskID, pRunInfo, DSOEUFLAG, flag);
#endif		
	}

	smMGive(dwDYXSem);
	return(0);       
  }
	
  if (num>wNum) num=wNum;
  
  j=k=0;  
  for (i=rp;i<(rp+num);i++)
  {
    pDBDSOE=pTEqp->DSOE.pPoolHead+(i % poolnum);  
    if ((pDBDSOE->byValue1==0) || (pDBDSOE->byValue2==0))
    {
		(*pwReadPtr)++;
        continue;
	}
    p->wNo=pTEqp->pDYXCfg[pDBDSOE->wNo].wSendNo;
    if (p->wNo==0xFFFF)
      continue;
    p->byValue1=pDBDSOE->byValue1;
    p->byValue2=pDBDSOE->byValue2;
    p->Time=pDBDSOE->Time;
    p++;
    j++;   
    k+=sizeof(struct VDBDSOE);
    if (k>wBufLen-sizeof(struct VDBDSOE)) break;
  }

  smMGive(dwDYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicQueryVEDCOS ID:1
 Purpose:    查询虚装置双点COS子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryVEDCOS(struct VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBDCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  struct VDBDCOS *pDBDCOS,*p=buf;

  *pwWriteOffset=pVEqp->DCOS.wWritePtr;
  *pwPoolLen=pVEqp->DCOS.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    pDBDCOS=pVEqp->DCOS.pPoolHead+(i % *pwPoolLen);  
    if ((pDBDCOS->byValue1==0)&&(pDBDCOS->byValue2==0))  break;
    p->wNo=pDBDCOS->wNo;
    p->byValue1=pDBDCOS->byValue1;
    p->byValue2=pDBDCOS->byValue2;
    p++; 
    j++;
    k+=sizeof(struct VDBDCOS);
    if (k>wBufLen-sizeof(struct VDBDCOS)) break;
  }
  
  return(j);    
}


/*------------------------------------------------------------------------
 Procedure:  atomicQueryTEDCOS ID:1
 Purpose:    查询实装置双点COS子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryTEDCOS(struct VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBDCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  struct VDBDCOS *pDBDCOS,*p=buf;

  *pwWriteOffset=pTEqp->DCOS.wWritePtr;
  *pwPoolLen=pTEqp->DCOS.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    pDBDCOS=pTEqp->DCOS.pPoolHead+(i % *pwPoolLen);  
    if ((pDBDCOS->byValue1==0)&&(pDBDCOS->byValue2==0))  break;
    p->wNo=pDBDCOS->wNo;
    p->byValue1=pDBDCOS->byValue1;
    p->byValue2=pDBDCOS->byValue2;
    p++; 
    j++;
    k+=sizeof(struct VDBDCOS);
    if (k>wBufLen-sizeof(struct VDBDCOS)) break;
  }
  
  return(j);    
}

/*------------------------------------------------------------------------
 Procedure:  atomicQueryVEDSOE ID:1
 Purpose:    查询虚装置双点SOE子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:SOE PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryVEDSOE(struct VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBDSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  struct VDBDSOE *pDBDSOE,*p=buf;

  *pwWriteOffset=pVEqp->DSOE.wWritePtr;
  *pwPoolLen=pVEqp->DSOE.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    pDBDSOE=pVEqp->DSOE.pPoolHead+(i % *pwPoolLen);
    if ((pDBDSOE->byValue1==0)&&(pDBDSOE->byValue2==0))  break;
    p->wNo=pDBDSOE->wNo;
    p->byValue1=pDBDSOE->byValue1;
    p->byValue2=pDBDSOE->byValue2;
    p->Time=pDBDSOE->Time;
    p++; 
    j++;
    k+=sizeof(struct VDBDSOE);
    if (k>wBufLen-sizeof(struct VDBDSOE)) break;
  }
  
  return(j);    
}


/*------------------------------------------------------------------------
 Procedure:  atomicQueryTEDSOE ID:1
 Purpose:    查询实装置双点SOE子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:SOE PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryTEDSOE(struct VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBDSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  struct VDBDSOE *pDBDSOE,*p=buf;

  *pwWriteOffset=pTEqp->DSOE.wWritePtr;
  *pwPoolLen=pTEqp->DSOE.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    pDBDSOE=pTEqp->DSOE.pPoolHead+(i % *pwPoolLen);  
    if ((pDBDSOE->byValue1==0)&&(pDBDSOE->byValue2==0))  break;
    p->wNo=pDBDSOE->wNo;
    p->byValue1=pDBDSOE->byValue1;
    p->byValue2=pDBDSOE->byValue2;
    p->Time=pDBDSOE->Time;
    p++; 
    j++;
    k+=sizeof(struct VDBDSOE);
    if (k>wBufLen-sizeof(struct VDBDSOE)) break;
  }
  
  return(j);    
}

/*------------------------------------------------------------------------
 Procedure:     atomicWriteSCOS ID:1
 Purpose:       写单点COS子函数
 Input:         semflag:根据其可知是否被应用程序直接调用还是被api函数嵌套
                调用
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteSCOS(WORD wEqpID,WORD wNum,struct VDBCOS *buf,BOOL semflag,BOOL isStatusNo,BOOL bLock) 
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  struct VDBCOS * p=buf;
  struct VTrueYX *pSYX;
  struct VTrueYXCfg *pCfg;
  struct VDBCOS *pDBCOS;
  WORD wVEqpID,wVNo;
  int i,j,k,maxno,statusno;
  BYTE value,oldvalue,newvalue;
  BOOL write;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL)||(pTEqp->SCOS.pPoolHead==NULL))  return;

  maxno=pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum-1;
  if (maxno<0)  return;
  statusno=pTEqp->Cfg.wSYXNum-1;

  if (semflag) smMTake(dwSYXSem);
  	  
  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno)||((p->byValue&0x01)==0))
    {
     p++;
     continue;
    } 
    
	if ((isStatusNo==FALSE)&&(p->wNo==statusno))
	{
	  p++;
	  continue;
	} 
	
    pCfg=pTEqp->pSYXCfg+p->wNo;
    pSYX=pTEqp->pSYX+p->wNo;       

	if (bLock&&(pCfg->dwCfg&0x80000000))	//正常写入，考虑闭锁
	{
	  p++;
	  continue;
	}

    if (pCfg->dwCfg&0x02)  //取反
      p->byValue^=0x80;

	if (pCfg->dwCfg&0x80000000)  //封锁
		pCfg->dwCfg |= 0x20000000;	//被取代
	else
		pCfg->dwCfg &= ~0x20000000;  //未被取代

	GetValueFlag(pCfg->dwCfg, &p->byValue);
	
    if (pCfg->wSendNo!=0xFFFF)
    {
      pDBCOS=pTEqp->SCOS.pPoolHead+pTEqp->SCOS.wWritePtr;
      pDBCOS->wNo=p->wNo;
      pDBCOS->byValue=p->byValue;
      pTEqp->SCOS.wWritePtr++;
      pTEqp->SCOS.wWritePtr%=pTEqp->SCOS.wPoolNum;
#ifdef INCLUDE_B2F_COS
	  WritePollBuf2File((struct VPool *)&pTEqp->SCOS, (BYTE *)pDBCOS, wEqpID, SYSEV_FLAG_SCOS);
#endif	   
      writeFlag(wEqpID,SCOSUFLAG);
    }
    
    for (j=0;j<pCfg->wVENum;j++)
    {
       wVEqpID=pCfg->pInvRef[j].wEqpID;
       wVNo=pCfg->pInvRef[j].wNo;

       pVEqp=g_Sys.Eqp.pInfo[wVEqpID].pVEqpInfo;
       if (pVEqp==NULL) continue;
       if ((pVEqp->pSYX==NULL)||(pVEqp->SCOS.pPoolHead==NULL)) continue;

       value=pSYX->byValue;
       pSYX->byValue=p->byValue;
       if (GetYXValue(pVEqp,wVNo,&newvalue,(BYTE *)NULL,2)==ERROR)
       {
          pSYX->byValue=value;
          continue;	
       }  
       pSYX->byValue=value;

       write=TRUE;       
       if (!semflag)    //compare    
       {
         GetYXValue(pVEqp,wVNo,&oldvalue,(BYTE *)NULL,2);
         if (oldvalue==newvalue)
           write=FALSE;
       }

		if(pVEqp->pSYX[wVNo].wNum > 1)
		{
			if(pVEqp->pSYX[wVNo].dwCfg & 0x01) //与操作有0则不写
			{
				for(k =pVEqp->pSYX[wVNo].wStartNo;k < (pVEqp->pSYX[wVNo].wStartNo + pVEqp->pSYX[wVNo].wNum);k++)
				{
					if(k == wVNo) continue;
					GetYXValue1(pVEqp,k,&oldvalue,(BYTE *)NULL,2);
					if(oldvalue == 0x01)
					{
						write = FALSE;
						break;
					}
				}
			}
			else //或操作 有1则不写
			{
				for(k =pVEqp->pSYX[wVNo].wStartNo;k < (pVEqp->pSYX[wVNo].wStartNo + pVEqp->pSYX[wVNo].wNum);k++)
				{
					if(k == wVNo) continue;
					GetYXValue1(pVEqp,k,&oldvalue,(BYTE *)NULL,2);
					if(oldvalue == 0x81)
					{
						write = FALSE;
						break;
					}
				}
			}
		}
		

       if (write)  //因为改写遥信有cos和yx两种途径，所以对于合并遥信会出现重复cos
       {
         pDBCOS=pVEqp->SCOS.pPoolHead+pVEqp->SCOS.wWritePtr;
         pDBCOS->wNo=pVEqp->pSYX[wVNo].wSendNo;
         pDBCOS->byValue=newvalue;
         pVEqp->SCOS.wWritePtr++;
         pVEqp->SCOS.wWritePtr%=pVEqp->SCOS.wPoolNum;
#ifdef INCLUDE_B2F_COS
		 WritePollBuf2File((struct VPool *)&pVEqp->SCOS, (BYTE *)pDBCOS, wVEqpID, SYSEV_FLAG_SCOS);
#endif	   		 
         writeFlag(wVEqpID,SCOSUFLAG);
       }  
    }  
    
    if (semflag)  pSYX->byValue=p->byValue;
    
    p++;
  }  
  
  if (semflag)  smMGive(dwSYXSem);    
}    


/*------------------------------------------------------------------------
 Procedure:     atomicWriteSSOE ID:1
 Purpose:       写单点SOE子函数
 Input:         semflag:根据其可知是否被应用程序直接调用还是被api函数嵌套
                调用
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteSSOE(WORD wEqpID,WORD wNum,struct VDBSOE *buf,BOOL semflag,BOOL isStatusNo,BOOL bLock) 
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  struct VDBSOE * p=buf;
  struct VTrueYX *pSYX;
  struct VTrueYXCfg *pCfg;
  struct VDBSOE *pDBSOE;
  WORD wVEqpID,wVNo;
  int i,j,k,maxno,statusno;
  BYTE value,oldvalue,newvalue;
  BOOL write;

  if (wEqpID>=*g_Sys.Eqp.pwNum)    return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL)||(pTEqp->SSOE.pPoolHead==NULL))  return;

  maxno=pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum-1;
  if (maxno<0)   return;
  statusno=pTEqp->Cfg.wSYXNum-1;

  if (semflag)  smMTake(dwSYXSem);

  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno)||((p->byValue&0x01)==0))
    {
      p++;
      continue;
    } 

	if ((isStatusNo==FALSE)&&(p->wNo==statusno))
	{
	  p++;
	  continue;
	} 

    pCfg=pTEqp->pSYXCfg+p->wNo;
    pSYX=pTEqp->pSYX+p->wNo;       

	if (bLock&&(pCfg->dwCfg&0x80000000))	//正常写入，考虑闭锁
	{
	  p++;
	  continue;
	}

    if (pCfg->dwCfg&0x02)  //取反
      p->byValue^=0x80;
    GetValueFlag(pCfg->dwCfg, &p->byValue);
    
    if (pCfg->wSendNo!=0xFFFF)
    {
       pDBSOE=pTEqp->SSOE.pPoolHead+pTEqp->SSOE.wWritePtr;
       pDBSOE->wNo=p->wNo;
       pDBSOE->byValue=p->byValue;
       pDBSOE->Time=p->Time;
       pTEqp->SSOE.wWritePtr++;
       pTEqp->SSOE.wWritePtr%=pTEqp->SSOE.wPoolNum;
#ifdef INCLUDE_B2F_SOE
	   WritePollBuf2File((struct VPool *)&pTEqp->SSOE, (BYTE *)pDBSOE, wEqpID, SYSEV_FLAG_SSOE);
#endif	   
       writeFlag(wEqpID,SSOEUFLAG);
    }
    
    for (j=0;j<pCfg->wVENum;j++)
    {
       wVEqpID=pCfg->pInvRef[j].wEqpID;
       wVNo=pCfg->pInvRef[j].wNo;

       pVEqp=g_Sys.Eqp.pInfo[wVEqpID].pVEqpInfo;
       if (pVEqp==NULL) continue;
       if ((pVEqp->pSYX==NULL)||(pVEqp->SSOE.pPoolHead==NULL)) continue;

       value=pSYX->byValue;
       pSYX->byValue=p->byValue;
       if (GetYXValue(pVEqp,wVNo,&newvalue,(BYTE *)NULL,2)==ERROR)
       {
          pSYX->byValue=value;
          continue;	
       }  
       pSYX->byValue=value;

       write=TRUE;       
       if (!semflag)    //compare    
       {
         GetYXValue(pVEqp,wVNo,&oldvalue,(BYTE *)NULL,2);
         if  (oldvalue==newvalue)
           write=FALSE;
       } 
		if(pVEqp->pSYX[wVNo].wNum > 1)
		{
			if(pVEqp->pSYX[wVNo].dwCfg & 0x01) //与操作有0则不写
			{
				for(k =pVEqp->pSYX[wVNo].wStartNo;k < (pVEqp->pSYX[wVNo].wStartNo + pVEqp->pSYX[wVNo].wNum);k++)
				{
					if(k == wVNo) continue;
					GetYXValue1(pVEqp,k,&oldvalue,(BYTE *)NULL,2);
					if(oldvalue == 0x01)
					{
						write = FALSE;
						break;
					}
				}
			}
			else //或操作 有1则不写
			{
				for(k =pVEqp->pSYX[wVNo].wStartNo;k < (pVEqp->pSYX[wVNo].wStartNo + pVEqp->pSYX[wVNo].wNum);k++)
				{
					if(k == wVNo) continue;
					GetYXValue1(pVEqp,k,&oldvalue,(BYTE *)NULL,2);
					if(oldvalue == 0x81)
					{
						write = FALSE;
						break;
					}
				}
			}
		}

       if (write)  //因为改写遥信有cos和yx两种途径，所以对于合并遥信会出现重复soe
       {
         pDBSOE=pVEqp->SSOE.pPoolHead+pVEqp->SSOE.wWritePtr;
         pDBSOE->wNo=pVEqp->pSYX[wVNo].wSendNo;
         pDBSOE->byValue=newvalue;
         pDBSOE->Time=p->Time;
         pVEqp->SSOE.wWritePtr++;
         pVEqp->SSOE.wWritePtr%=pVEqp->SSOE.wPoolNum;

#ifdef INCLUDE_B2F_SOE
	     //WritePollBuf2File((struct VPool *)&pVEqp->SSOE, (BYTE *)pDBSOE, wVEqpID, SYSEV_FLAG_SSOE);
#endif
#ifdef INCLUDE_HIS
		if(pVEqp->Cfg.dwFlag & 0x40)
			 his2file_write((BYTE*)pDBSOE,wVEqpID,SYSEV_FLAG_SSOE);
#endif
#ifdef INCLUDE_HIS_TTU
             if(pVEqp->Cfg.dwFlag & 0x40)
                histtu2file_write((BYTE*)pDBSOE,wVEqpID,SYSEV_FLAG_SSOE);
#endif				 

         writeFlag(wVEqpID,SSOEUFLAG);         
       }  
    }  
    
    p++;
  }  
  
  if (semflag)  smMGive(dwSYXSem);    
}    

/*------------------------------------------------------------------------
 Procedure: 	WriteSYX ID:1
 Purpose:		写离散点单点遥信(一字节代表一遥信) 
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteSYX(WORD wEqpID,WORD wNum, struct VDBYX *buf,BOOL isStatusNo,BOOL bLock,BOOL bCCos, BOOL bCSoe)
{
  struct VTrueEqp *pTEqp;
  struct VDBYX SYX,* p=buf;
  struct VTrueYX *pSYX;
  struct VTrueYXCfg *pCfg;
  struct VDBSOE DBSOE;  
  int i,maxno,statusno;
  DWORD dwCfg;
  int change;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)	return;

  if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL))	return;

  maxno=pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum-1;
  if (maxno<0)	return;
  statusno=pTEqp->Cfg.wSYXNum-1;
  
  smMTake(dwSYXSem); 	  
  
  for (i=0;i<wNum;i++)
  {
	if (p->wNo>maxno)
	{
	  p++;
	  continue;
	} 

	if ((isStatusNo==FALSE)&&(p->wNo==statusno))
	{
	  p++;
	  continue;
	} 
		
	pCfg=pTEqp->pSYXCfg+p->wNo;
	pSYX=pTEqp->pSYX+p->wNo;   
	dwCfg=pCfg->dwCfg;	

	if ((p->byValue&0x01)==0)
	{
		SYX.byValue = pSYX->byValue;   
		GetValueFlag(dwCfg, &SYX.byValue);
	}
	else
	{
		if (dwCfg&0x80000000)  //封锁
		    pCfg->dwCfg |= 0x20000000;  //被取代
		else
			pCfg->dwCfg &= ~0x20000000;  //未被取代
		SYX.byValue=p->byValue;
		GetValueFlag(dwCfg, &SYX.byValue);
	}	

	SYX.wNo=p->wNo;

	if (bLock&&(dwCfg&0x80000000))  //正常写入，考虑闭锁
	{
	  p++;
	  continue;
	}

#ifndef INCULDE_COS_CFG
	dwCfg|=0x08;  //数据库产生COS
#endif

	if (dwCfg&0x02)  //取反
	  SYX.byValue^=0x80;
    
	if ((dwCfg&0x08) || bCCos)//write cos
	{
		change = pSYX->byValue^SYX.byValue;
		if (change)  atomicWriteSCOS(wEqpID,1,(struct VDBCOS *)p,FALSE,isStatusNo,bLock);
	}

	if ((dwCfg&0x04) || bCSoe) //write soe
	{	 
		change = (pSYX->byValue&0x80)^(SYX.byValue&0x80);

		if (change)
		{
			DBSOE.wNo=p->wNo;
			DBSOE.byValue=p->byValue;
			GetSysClock(&DBSOE.Time,CALCLOCK);
			atomicWriteSSOE(wEqpID,1,&DBSOE,FALSE,isStatusNo,bLock);
		} 
	} 

	pSYX->byValue=SYX.byValue;
	p++;   
  }
   
  smMGive(dwSYXSem);
}  


/*------------------------------------------------------------------------
 Procedure:     atomicReadVESYX ID:1
 Purpose:       读虚装置离散点单点遥信子函数(一字节代表一遥信) 
 Input:         
 Output:		实际所读的个数  
 Errors:
 asSend:  以前是否按发送序号发送,现在变为取的值是否是与或计算的值
------------------------------------------------------------------------*/
WORD atomicReadVESYX(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBYX *buf,BOOL asSend)
{
  int i,j,k,maxno;
  struct VDBYX *p,*q;
  
  maxno=pVEqp->Cfg.wSYXNum-1; 
  if (maxno<0)  return(0);

  if (pVEqp->pSYX==NULL) return(0);
	 
  smMTake(dwSYXSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    q->wNo=p->wNo;
		if(asSend)
		{
			if (GetYXValue(pVEqp,q->wNo,&q->byValue,(BYTE *)NULL,2)==ERROR)
			{
				p++;
				continue;
			}
		}
		else
		{
			if (GetYXValue1(pVEqp,q->wNo,&q->byValue,(BYTE *)NULL,2)==ERROR)
			{
				p++;
				continue;
			}
		}
	
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBYX);
    if (k>wBufLen-sizeof(struct VDBYX)) break;    
  }  
  
  smMGive(dwSYXSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadTESYX ID:1
 Purpose:       读实装置离散点单点遥信子函数(一字节代表一遥信) 
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTESYX(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBYX *buf,BOOL asSend)
{
  int i,j,k,maxno;
  WORD no;
  struct VDBYX *p,*q;

  if (asSend)
    maxno=pTEqp->SYXSendCfg.wNum-1;
  else
    maxno=pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum-1;
 
  if (maxno<0) return(0);

  smMTake(dwSYXSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    if (asSend)
   	  no=pTEqp->SYXSendCfg.pwIndex[p->wNo];
    else
   	  no=p->wNo;
    q->wNo=p->wNo;
    q->byValue=pTEqp->pSYX[no].byValue;
    GetValueFlag(pTEqp->pSYXCfg[no].dwCfg, &q->byValue);
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBYX);
    if (k>wBufLen-sizeof(struct VDBYX)) break;
  }  

  smMGive(dwSYXSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadSYX ID:1
 Purpose:       读离散点单点遥信子函数(一字节代表一遥信) 
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadSYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYX *buf,BOOL asSend)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBYX)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if (pVEqp->pSYX==NULL)
        return(0);
      else
        j=atomicReadVESYX(pVEqp,wNum,wBufLen,buf,asSend);
    }   
  } 
  else
  {
    if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL))
      return(0);
    else
      j=atomicReadTESYX(pTEqp,wNum,wBufLen,buf,asSend);
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadVESCOS ID:1
 Purpose:    读虚装置单点COS子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVESCOS(WORD wTaskID,WORD wEqpID,struct VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBCOS *buf)
{
  int i,j,k;
  WORD rp,wp,poolnum,num;
  struct VDBCOS *pDBCOS,*p=buf;
  struct VEqpRunInfo *pRunInfo;
  
  smMTake(dwSYXSem);  

  wp=pVEqp->SCOS.wWritePtr;
  poolnum=pVEqp->SCOS.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {				
    pRunInfo=GetpRunInfo(wTaskID, wEqpID, NULL);
	if (pRunInfo!=NULL)  pRunInfo->dwUFlag&=(~SCOSUFLAG);

	smMGive(dwSYXSem);
	return(0);       
  }
	
  if (num>wNum)  num=wNum;

  j=k=0;
  for (i=rp;i<(rp+num);i++)
  {
    pDBCOS=pVEqp->SCOS.pPoolHead+(i % poolnum);  
    if (pDBCOS->byValue==0) 
    {
		(*pwReadPtr)++;
        continue;
	}
    p->wNo=pDBCOS->wNo;
    p->byValue=pDBCOS->byValue;
    p++; 
    j++;
    k+=sizeof(struct VDBCOS);
    if (k>wBufLen-sizeof(struct VDBCOS)) break;      
  }

  smMGive(dwSYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadTESCOS ID:1
 Purpose:    读实装置单点COS子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTESCOS(WORD wTaskID,WORD wEqpID,struct VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBCOS *buf)
{
  int i,j,k;
  WORD rp,wp,poolnum,num;
  struct VDBCOS *pDBCOS,*p=buf;
  struct VEqpRunInfo *pRunInfo;

  smMTake(dwSYXSem);  

  wp=pTEqp->SCOS.wWritePtr;
  poolnum=pTEqp->SCOS.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {					
    pRunInfo=GetpRunInfo(wTaskID, wEqpID, NULL);
	if (pRunInfo!=NULL)  pRunInfo->dwUFlag&=(~SCOSUFLAG);

	smMGive(dwSYXSem);
	return(0);       
  }
	
  if (num>wNum)  num=wNum;
  
  j=k=0;  
  for (i=rp;i<(rp+num);i++)
  {
    pDBCOS=pTEqp->SCOS.pPoolHead+(i % poolnum);  
    if (pDBCOS->byValue==0) 
    {
		(*pwReadPtr)++;
        continue;
	}
    p->wNo=pTEqp->pSYXCfg[pDBCOS->wNo].wSendNo;
    if (p->wNo==0xFFFF)
   	  continue;
    p->byValue=pDBCOS->byValue;
    p++;
    j++;   
    k+=sizeof(struct VDBCOS);
    if (k>wBufLen-sizeof(struct VDBCOS)) break;      
  }

  smMGive(dwSYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadVESSOE ID:1
 Purpose:    读虚装置单点SOE子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVESSOE(WORD wTaskID,WORD wEqpID,struct VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBSOE *buf)
{
  int i,j,k,flag;
  WORD rp,wp,poolnum,num;
  struct VDBSOE *pDBSOE,*p=buf;
  struct VEqpRunInfo *pRunInfo;
  
  num=wNum;
  
  smMTake(dwSYXSem);  

  wp=pVEqp->SSOE.wWritePtr;
  poolnum=pVEqp->SSOE.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;
   
  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {						
    pRunInfo=GetpRunInfo(wTaskID, wEqpID, &flag);
    if ((pRunInfo!=NULL) && (pRunInfo->dwUFlag&SSOEUFLAG))
	{
		pRunInfo->dwUFlag&=(~SSOEUFLAG);
#ifdef INCLUDE_B2F_F
		WriteRunInfo2File(wTaskID, pRunInfo, SSOEUFLAG, flag);
#endif		
    }
		
	smMGive(dwSYXSem);
	return(0);       
  }
	
  if (num>wNum)  num=wNum;
  
  j=k=0;
  for (i=rp;i<(rp+num);i++)
  {
    pDBSOE=pVEqp->SSOE.pPoolHead+(i % poolnum);  
    if (pDBSOE->byValue==0) 
    {
		(*pwReadPtr)++;
        continue;
	}
    p->wNo=pDBSOE->wNo;
    p->byValue=pDBSOE->byValue;
    p->Time=pDBSOE->Time;
    p++; 
    j++;
    k+=sizeof(struct VDBSOE);
    if (k>wBufLen-sizeof(struct VDBSOE)) break;
  }

  smMGive(dwSYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadTESSOE ID:1
 Purpose:    读实装置单点SOE子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTESSOE(WORD wTaskID,WORD wEqpID,struct VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBSOE *buf)
{
  int i,j,k,flag;
  WORD rp,wp,poolnum,num;
  struct VDBSOE *pDBSOE,*p=buf;
  struct VEqpRunInfo *pRunInfo;
  
  smMTake(dwSYXSem);  

  wp=pTEqp->SSOE.wWritePtr;
  poolnum=pTEqp->SSOE.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {							
	pRunInfo=GetpRunInfo(wTaskID, wEqpID, &flag);
	if ((pRunInfo!=NULL) && (pRunInfo->dwUFlag&SSOEUFLAG))
	{
		pRunInfo->dwUFlag&=(~SSOEUFLAG);
#ifdef INCLUDE_B2F_F
		WriteRunInfo2File(wTaskID, pRunInfo, SSOEUFLAG, flag);
#endif		
	}
	
	smMGive(dwSYXSem);
	return(0);
  }
	
  if (num>wNum)  num=wNum;
  
  j=k=0;  
  for (i=rp;i<(rp+num);i++)
  {
    pDBSOE=pTEqp->SSOE.pPoolHead+(i % poolnum);  
    if (pDBSOE->byValue==0) 
    {
		(*pwReadPtr)++;
        continue;
	}
    p->wNo=pTEqp->pSYXCfg[pDBSOE->wNo].wSendNo;
    if (p->wNo==0xFFFF)
      continue;
	p->byValue=pDBSOE->byValue;
    p->Time=pDBSOE->Time;
    p++;
    j++;   
    k+=sizeof(struct VDBSOE);
    if (k>wBufLen-sizeof(struct VDBSOE)) break;
  }

  smMGive(dwSYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicQueryVESCOS ID:1
 Purpose:    查询虚装置单点COS子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryVESCOS(struct VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  struct VDBCOS *pDBCOS,*p=buf;

  *pwWriteOffset=pVEqp->SCOS.wWritePtr;
  *pwPoolLen=pVEqp->SCOS.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    pDBCOS=pVEqp->SCOS.pPoolHead+(i % *pwPoolLen); 
    if (pDBCOS->byValue==0) break;
    p->wNo=pDBCOS->wNo;
    p->byValue=pDBCOS->byValue;
    p++; 
    j++;
    k+=sizeof(struct VDBCOS);
    if (k>wBufLen-sizeof(struct VDBCOS)) break;
  }
  
  return(j);    
}


/*------------------------------------------------------------------------
 Procedure:  atomicQueryTESCOS ID:1
 Purpose:    查询实装置单点COS子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryTESCOS(struct VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  struct VDBCOS *pDBCOS,*p=buf;

  *pwWriteOffset=pTEqp->SCOS.wWritePtr;
  *pwPoolLen=pTEqp->SCOS.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    pDBCOS=pTEqp->SCOS.pPoolHead+(i % *pwPoolLen);  
    if (pDBCOS->byValue==0) break;
    p->wNo=pDBCOS->wNo;
    p->byValue=pDBCOS->byValue;
    p++; 
    j++;
    k+=sizeof(struct VDBCOS);
    if (k>wBufLen-sizeof(struct VDBCOS)) break;
  }
  
  return(j);    
}

/*------------------------------------------------------------------------
 Procedure:  atomicQueryVESSOE ID:1
 Purpose:    查询虚装置单点SOE子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:SOE PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryVESSOE(struct VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  struct VDBSOE *pDBSOE,*p=buf;

  *pwWriteOffset=pVEqp->SSOE.wWritePtr;
  *pwPoolLen=pVEqp->SSOE.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    pDBSOE=pVEqp->SSOE.pPoolHead+(i % *pwPoolLen);  
    if (pDBSOE->byValue==0) break;
    p->wNo=pDBSOE->wNo;
    p->byValue=pDBSOE->byValue;
    p->Time=pDBSOE->Time;
    p++; 
    j++;
    k+=sizeof(struct VDBSOE);
    if (k>wBufLen-sizeof(struct VDBSOE)) break;
  }
  
  return(j);    
}


/*------------------------------------------------------------------------
 Procedure:  atomicQueryTESSOE ID:1
 Purpose:    查询实装置单点SOE子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:SOE PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryTESSOE(struct VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  struct VDBSOE *pDBSOE,*p=buf;

  *pwWriteOffset=pTEqp->SSOE.wWritePtr;
  *pwPoolLen=pTEqp->SSOE.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    pDBSOE=pTEqp->SSOE.pPoolHead+(i % *pwPoolLen); 
    if (pDBSOE->byValue==0) break;
    p->wNo=pDBSOE->wNo;
    p->byValue=pDBSOE->byValue;
    p->Time=pDBSOE->Time;
    p++; 
    j++;
    k+=sizeof(struct VDBSOE);
    if (k>wBufLen-sizeof(struct VDBSOE)) break;
  }
  
  return(j);    
}

#if 0
/*------------------------------------------------------------------------
 Procedure:     atomicWriteEVCOS ID:1
 Purpose:       写瞬变COS子函数
 Input:         semflag:根据其可知是否被应用程序直接调用还是被api函数嵌套
                调用
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteEVCOS(WORD wEqpID,WORD wNum,VDBCOS *buf,BOOL semflag,BOOL bLock) 
{
  VTrueEqp *pTEqp;
  VVirtualEqp *pVEqp;
  VDBCOS * p=buf;
  VTrueYX *pEVYX;
  VTrueYXCfg *pCfg;
  VDBCOS *pDBCOS;
  WORD wVEqpID,wVNo;
  int i,j,maxno;
  BYTE value,oldvalue,newvalue;
  BOOL write;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pEVYX==NULL)||(pTEqp->pEVYXCfg==NULL)||(pTEqp->EVCOS.pPoolHead==NULL))  return;

  maxno=pTEqp->Cfg.wEVYXNum-1;
  if (maxno<0)  return;

  if (semflag) smMTake(dwEVYXSem);
  	  
  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno)||((p->byValue&0x01)==0))
    {
     p++;
     continue;
    } 
    
    pCfg=pTEqp->pEVYXCfg+p->wNo;
    pEVYX=pTEqp->pEVYX+p->wNo;       

	if (bLock&&(pCfg->dwCfg&0x80000000))	//正常写入，考虑闭锁
	{
	  p++;
	  continue;
	}

    if (pCfg->dwCfg&0x02)  //取反
      p->byValue^=0x80;
    
    if (pCfg->wSendNo!=0xFFFF)
    {
      pDBCOS=pTEqp->EVCOS.pPoolHead+pTEqp->EVCOS.wWritePtr;
      pDBCOS->wNo=p->wNo;
      pDBCOS->byValue=p->byValue;
      pTEqp->EVCOS.wWritePtr++;
      pTEqp->EVCOS.wWritePtr%=pTEqp->EVCOS.wPoolNum;
      if (pCfg->dwCfg&0x10)  //产生复归信号(缺省)
      {
        pDBCOS=pTEqp->EVCOS.pPoolHead+pTEqp->EVCOS.wWritePtr;
        pDBCOS->wNo=p->wNo;
        pDBCOS->byValue=p->byValue^0x80;
        pTEqp->EVCOS.wWritePtr++;
        pTEqp->EVCOS.wWritePtr%=pTEqp->EVCOS.wPoolNum;
      }        
      writeFlag(wEqpID,EVCOSUFLAG);
    }
    
    for (j=0;j<pCfg->wVENum;j++)
    {
       wVEqpID=pCfg->pInvRef[j].wEqpID;
       wVNo=pCfg->pInvRef[j].wNo;

       pVEqp=g_Sys.Eqp.pInfo[wVEqpID].pVEqpInfo;
       if (pVEqp==NULL) continue;
       if ((pVEqp->pEVYX==NULL)||(pVEqp->EVCOS.pPoolHead==NULL)) continue;

       value=pEVYX->byValue;
       pEVYX->byValue=p->byValue;
       if (GetYXValue(pVEqp,wVNo,&newvalue,(BYTE *)NULL,3)==ERROR)
       {
          pEVYX->byValue=value;
          continue;	
       }  
       pEVYX->byValue=value;

       write=TRUE;       
       if (!semflag)    //compare    
       {
         GetYXValue(pVEqp,wVNo,&oldvalue,(BYTE *)NULL,3);
         if (oldvalue==newvalue)
           write=FALSE;
       }  

       if (write)  //因为改写遥信有cos和yx两种途径，所以对于合并遥信会出现重复cos
       {
         pDBCOS=pVEqp->EVCOS.pPoolHead+pVEqp->EVCOS.wWritePtr;
         pDBCOS->wNo=wVNo;
         pDBCOS->byValue=p->byValue;
         pVEqp->EVCOS.wWritePtr++;
         pVEqp->EVCOS.wWritePtr%=pVEqp->EVCOS.wPoolNum;
         if (pCfg->dwCfg&0x10)  //产生复归信号(缺省)
         {
           pDBCOS=pVEqp->EVCOS.pPoolHead+pVEqp->EVCOS.wWritePtr;
           pDBCOS->wNo=wVNo;
           pDBCOS->byValue=p->byValue^0x80;
           pVEqp->EVCOS.wWritePtr++;
           pVEqp->EVCOS.wWritePtr%=pVEqp->EVCOS.wPoolNum;
         }           
         writeFlag(wVEqpID,EVCOSUFLAG);
       }  
    }  
    
    if ((semflag)&&(!(pCfg->dwCfg&0x10)))  pEVYX->byValue=p->byValue;
    
    p++;
  }  
  
  if (semflag)  smMGive(dwMEVYXSem);    
}    


/*------------------------------------------------------------------------
 Procedure:     atomicWriteEVSOE ID:1
 Purpose:       写瞬变SOE子函数
 Input:         semflag:根据其可知是否被应用程序直接调用还是被api函数嵌套
                调用
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteEVSOE(WORD wEqpID,WORD wNum,VDBSOE *buf,BOOL semflag,BOOL bLock) 
{
  VTrueEqp *pTEqp;
  VVirtualEqp *pVEqp;
  VDBSOE * p=buf;
  VTrueYX *pEVYX;
  VTrueYXCfg *pCfg;
  VDBSOE *pDBSOE;
  WORD wVEqpID,wVNo;
  int i,j,maxno;
  BYTE value,oldvalue,newvalue;
  BOOL write;

  if (wEqpID>=*g_Sys.Eqp.pwNum)    return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pEVYX==NULL)||(pTEqp->pEVYXCfg==NULL)||(pTEqp->EVSOE.pPoolHead==NULL))  return;

  maxno=pTEqp->Cfg.wEVYXNum-1;
  if (maxno<0)   return;

  if (semflag)  smMTake(dwMEVYXSem);
  	  
  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno)||((p->byValue&0x01)==0))
    {
      p++;
      continue;
    } 

    pCfg=pTEqp->pEVYXCfg+p->wNo;
    pEVYX=pTEqp->pEVYX+p->wNo;       

	if (bLock&&(pCfg->dwCfg&0x80000000))	//正常写入，考虑闭锁
	{
	  p++;
	  continue;
	}

    if (pCfg->dwCfg&0x02)  //取反
      p->byValue^=0x80;
    
    if (pCfg->wSendNo!=0xFFFF)
    {
       pDBSOE=pTEqp->EVSOE.pPoolHead+pTEqp->EVSOE.wWritePtr;
       pDBSOE->wNo=p->wNo;
       pDBSOE->byValue=p->byValue;
       pDBSOE->Time=p->Time;
       pTEqp->EVSOE.wWritePtr++;
       pTEqp->EVSOE.wWritePtr%=pTEqp->EVSOE.wPoolNum;
       if (pCfg->dwCfg&0x10)  //产生复归信号(缺省)
       {
         pDBSOE=pTEqp->EVSOE.pPoolHead+pTEqp->EVSOE.wWritePtr;
         pDBSOE->wNo=p->wNo;
         pDBSOE->byValue=p->byValue^0x80;
         pDBSOE->Time=p->Time;
         pTEqp->EVSOE.wWritePtr++;
         pTEqp->EVSOE.wWritePtr%=pTEqp->EVSOE.wPoolNum;
       } 
       writeFlag(wEqpID,EVSOEUFLAG);
    }
    
    for (j=0;j<pCfg->wVENum;j++)
    {
       wVEqpID=pCfg->pInvRef[j].wEqpID;
       wVNo=pCfg->pInvRef[j].wNo;

       pVEqp=g_Sys.Eqp.pInfo[wVEqpID].pVEqpInfo;
       if (pVEqp==NULL) continue;
       if ((pVEqp->pEVYX==NULL)||(pVEqp->EVSOE.pPoolHead==NULL)) continue;

       value=pEVYX->byValue;
       pEVYX->byValue=p->byValue;
       if (GetYXValue(pVEqp,wVNo,&newvalue,(BYTE *)NULL,3)==ERROR)
       {
          pEVYX->byValue=value;
          continue;	
       }  
       pEVYX->byValue=value;

       write=TRUE;       
       if (!semflag)    //compare    
       {
         GetYXValue(pVEqp,wVNo,&oldvalue,(BYTE *)NULL,3);
         if  (oldvalue==newvalue)
           write=FALSE;
       }  

       if (write)  //因为改写遥信有cos和yx两种途径，所以对于合并遥信会出现重复soe
       {
         pDBSOE=pVEqp->EVSOE.pPoolHead+pVEqp->EVSOE.wWritePtr;
         pDBSOE->wNo=wVNo;
         pDBSOE->byValue=p->byValue;
         pDBSOE->Time=p->Time;
         pVEqp->EVSOE.wWritePtr++;
         pVEqp->EVSOE.wWritePtr%=pVEqp->EVSOE.wPoolNum;
         if (pCfg->dwCfg&0x10)  //产生复归信号(缺省)
         {
           pDBSOE=pVEqp->EVSOE.pPoolHead+pVEqp->EVSOE.wWritePtr;
           pDBSOE->wNo=wVNo;
           pDBSOE->byValue=p->byValue;
           pDBSOE->Time=p->Time;
           pVEqp->EVSOE.wWritePtr++;
           pVEqp->EVSOE.wWritePtr%=pVEqp->EVSOE.wPoolNum;
         }
         writeFlag(wVEqpID,EVSOEUFLAG);         
       }  
    }  
    
    p++;
  }  
  
  if (semflag)  smMGive(dwEVYXSem);    
}    


/*------------------------------------------------------------------------
 Procedure:     atomicReadVEEVYX ID:1
 Purpose:       读虚装置离散点瞬变遥信子函数(一字节代表一遥信) 
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEEVYX(VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBYX *buf)
{
  int i,j,k,maxno;
  struct VDBYX *p,*q;
  
  maxno=pVEqp->Cfg.wEVYXNum-1; 
  if (maxno<0)  return(0);

  smMTake(dwEVYXSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    q->wNo=p->wNo;
    if (GetYXValue(pVEqp,q->wNo,&q->byValue,(BYTE *)NULL,3)==ERROR)
    {
      p++;
      continue;
    }      	
    p++;
    q++;
    j++;
    k+=sizeof(VDBYX);
    if (k>wBufLen-sizeof(VDBYX)) break;    
  }  
  
  smMGive(dwEVYXSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadTEEVYX ID:1
 Purpose:       读实装置离散点瞬变遥信子函数(一字节代表一遥信) 
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEEVYX(VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBYX *buf,BOOL asSend)
{
  int i,j,k,maxno;
  struct VDBYX *p,*q;
  WORD no;

  if (asSend)
    maxno=pTEqp->EVYXSendCfg.wNum-1;
  else
    maxno=pTEqp->Cfg.wEVYXNum-1;
 
  if (maxno<0) return(0);

  smMTake(dwEVYXSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    if (asSend)
   	  no=pTEqp->EVYXSendCfg.pwIndex[p->wNo];
    else
   	  no=p->wNo;
    q->wNo=p->wNo;
    q->byValue=pTEqp->pEVYX[no].byValue;
    p++;
    q++;
    j++;
    k+=sizeof(VDBYX);
    if (k>wBufLen-sizeof(VDBYX)) break;
  }  

  smMGive(dwEVYXSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadEVYX ID:1
 Purpose:       读离散点瞬变遥信子函数(一字节代表一遥信) 
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadEVYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYX *buf,BOOL asSend)
{
  VTrueEqp *pTEqp;
  VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(VDBYX)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if (pVEqp->pEVYX==NULL)
        return(0);
      else
        j=atomicReadVEEVYX(pVEqp,wNum,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->pEVYX==NULL)||(pTEqp->pEVYXCfg==NULL))
      return(0);
    else
      j=atomicReadTEEVYX(pTEqp,wNum,wBufLen,buf,asSend);
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadVEEVCOS ID:1
 Purpose:    读虚装置瞬变COS子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEEVCOS(WORD wTaskID,WORD wEqpID,VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBCOS *buf)
{
  int i,j,k;
  WORD rp,wp,poolnum,num;
  VDBCOS *pDBCOS,*p=buf;
  VEqpRunInfo *pRunInfo;
  
  smMTake(dwEVYXSem);  

  wp=pVEqp->EVCOS.wWritePtr;
  poolnum=pVEqp->EVCOS.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;
  
  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {			
    pRunInfo=GetpRunInfo(wTaskID, wEqpID, NULL);
    if (pRunInfo!=NULL)  pRunInfo->dwUFlag&=(~EVCOSUFLAG);

	smMGive(dwEVYXSem);
	return(0);       
  }
	
  if (num>wNum) num=wNum;
  
  j=k=0;
  for (i=rp;i<(rp+num);i++)
  {
    pDBCOS=pVEqp->EVCOS.pPoolHead+(i % poolnum);  
    p->wNo=pDBCOS->wNo;
    p->byValue=pDBCOS->byValue;
    p++; 
    j++;
    k+=sizeof(VDBCOS);
    if (k>wBufLen-sizeof(VDBCOS)) break;      
  }

  smMGive(dwEVYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadTEEVCOS ID:1
 Purpose:    读实装置瞬变COS子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEEVCOS(WORD wTaskID,WORD wEqpID,VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBCOS *buf)
{
  int i,j,k;
  WORD rp,wp,poolnum,num;
  VDBCOS *pDBCOS,*p=buf;
  VEqpRunInfo *pRunInfo;

  num=wNum;
  
  smMTake(dwEVYXSem);  

  wp=pTEqp->EVCOS.wWritePtr;
  poolnum=pTEqp->EVCOS.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {				
    pRunInfo=GetpRunInfo(wTaskID, wEqpID, NULL);
	if (pRunInfo!=NULL)  pRunInfo->dwUFlag&=(~EVCOSUFLAG);

	smMGive(dwEVYXSem);
	return(0);       
  }
	
  if (num>wNum) num=wNum;
  
  j=k=0;  
  for (i=rp;i<(rp+num);i++)
  {
    pDBCOS=pTEqp->EVCOS.pPoolHead+(i % poolnum);  
    p->wNo=pTEqp->pEVYXCfg[pDBCOS->wNo].wSendNo;
    if (p->wNo==0xFFFF)
   	  continue;
    p->byValue=pDBCOS->byValue;
    p++;
    j++;   
    k+=sizeof(VDBCOS);
    if (k>wBufLen-sizeof(VDBCOS)) break;      
  }

  smMGive(dwEVYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadVEEVSOE ID:1
 Purpose:    读虚装置瞬变SOE子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEEVSOE(WORD wTaskID,WORD wEqpID,VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBSOE *buf)
{
  int i,j,k,flag;
  WORD rp,wp,poolnum,num;
  VDBSOE *pDBSOE,*p=buf;
  VEqpRunInfo *pRunInfo;
  
  smMTake(dwEVYXSem);  

  wp=pVEqp->EVSOE.wWritePtr;
  poolnum=pVEqp->EVSOE.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {					
    pRunInfo=GetpRunInfo(wTaskID, wEqpID, &flag);
	if ((pRunInfo!=NULL) && (pRunInfo->dwUFlag&EVSOEUFLAG))  
	{
		pRunInfo->dwUFlag&=(~EVSOEUFLAG);
#ifdef INCLUDE_B2F_F
		WriteRunInfo2File(wTaskID, pRunInfo, EVSOEUFLAG, flag);
#endif				
	}	

	smMGive(dwEVYXSem);
	return(0);       
  }
	
  if (num>wNum) num=wNum;
  
  j=k=0;
  for (i=rp;i<(rp+num);i++)
  {
    pDBSOE=pVEqp->EVSOE.pPoolHead+(i % poolnum);  
    p->wNo=pDBSOE->wNo;
    p->byValue=pDBSOE->byValue;
    p->Time=pDBSOE->Time;
    p++; 
    j++;
    k+=sizeof(VDBSOE);
    if (k>wBufLen-sizeof(VDBSOE)) break;
  }

  smMGive(dwEVYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicReadTEEVSOE ID:1
 Purpose:    读实装置单点SOE子函数 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEEVSOE(WORD wTaskID,WORD wEqpID,VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBSOE *buf)
{
  int i,j,k,flag;
  WORD rp,wp,poolnum,num;
  VDBSOE *pDBSOE,*p=buf;
  VEqpRunInfo *pRunInfo;

  num=wNum;
  
  smMTake(dwEVYXSem);  

  wp=pTEqp->EVSOE.wWritePtr;
  poolnum=pTEqp->EVSOE.wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {						
    pRunInfo=GetpRunInfo(wTaskID, wEqpID, &flag);
	if ((pRunInfo!=NULL) && (pRunInfo->dwUFlag&EVSOEUFLAG))
	{
		pRunInfo->dwUFlag&=(~EVSOEUFLAG);
#ifdef INCLUDE_B2F_F
		WriteRunInfo2File(wTaskID, pRunInfo, EVSOEUFLAG, flag);
#endif		
	}	

	smMGive(dwEVYXSem);
	return(0);       
  }
	
  if (num>wNum) num=wNum;
  
  j=k=0;  
  for (i=rp;i<(rp+num);i++)
  {
    pDBSOE=pTEqp->EVSOE.pPoolHead+(i % poolnum);  
    p->wNo=pTEqp->pEVYXCfg[pDBSOE->wNo].wSendNo;
    if (p->wNo==0xFFFF)
      continue;
    p->byValue=pDBSOE->byValue;
    p->Time=pDBSOE->Time;
    p++;
    j++;   
    k+=sizeof(VDBSOE);
    if (k>wBufLen-sizeof(VDBSOE)) break;
  }

  smMGive(dwEVYXSem);
  
  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  atomicQueryVEEVCOS ID:1
 Purpose:    查询虚装置瞬变COS子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryVEEVCOS(VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  VDBCOS *pDBCOS,*p=buf;

  *pwWriteOffset=pVEqp->EVCOS.wWritePtr;
  *pwPoolLen=pVEqp->EVCOS.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    pDBCOS=pVEqp->EVCOS.pPoolHead+(i % *pwPoolLen);
    if (pDBCOS->byValue==0) break;
    p->wNo=pDBCOS->wNo;
    p->byValue=pDBCOS->byValue;
    p++; 
    j++;
    k+=sizeof(VDBCOS);
    if (k>wBufLen-sizeof(VDBCOS)) break;
  }
  
  return(j);    
}


/*------------------------------------------------------------------------
 Procedure:  atomicQueryTEEVCOS ID:1
 Purpose:    查询实装置瞬变COS子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryTEEVCOS(VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  VDBCOS *pDBCOS,*p=buf;

  *pwWriteOffset=pTEqp->EVCOS.wWritePtr;
  *pwPoolLen=pTEqp->EVCOS.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {  
    pDBCOS=pTEqp->EVCOS.pPoolHead+(i % *pwPoolLen);
    if (pDBCOS->byValue==0) break;
    p->wNo=pDBCOS->wNo;
    p->byValue=pDBCOS->byValue;
    p++; 
    j++;
    k+=sizeof(VDBCOS);
    if (k>wBufLen-sizeof(VDBCOS)) break;
  }
  
  return(j);    
}

/*------------------------------------------------------------------------
 Procedure:  atomicQueryVEEVSOE ID:1
 Purpose:    查询虚装置瞬变SOE子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:SOE PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryVEEVSOE(VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  VDBSOE *pDBSOE,*p=buf;

  *pwWriteOffset=pVEqp->EVSOE.wWritePtr;
  *pwPoolLen=pVEqp->EVSOE.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    pDBSOE=pVEqp->EVSOE.pPoolHead+(i % *pwPoolLen);
    if (pDBSOE->byValue==0) break;
    p->wNo=pDBSOE->wNo;
    p->byValue=pDBSOE->byValue;
    p->Time=pDBSOE->Time;
    p++; 
    j++;
    k+=sizeof(VDBSOE);
    if (k>wBufLen-sizeof(VDBSOE)) break;
  }
  
  return(j);    
}


/*------------------------------------------------------------------------
 Procedure:  atomicQueryTEEVSOE ID:1
 Purpose:    查询实装置单点SOE子函数  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:SOE PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD atomicQueryTEEVSOE(VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  int i,j,k;
  WORD num;
  VDBSOE *pDBSOE,*p=buf;

  *pwWriteOffset=pTEqp->EVSOE.wWritePtr;
  *pwPoolLen=pTEqp->EVSOE.wPoolNum;
  
  if (wReadOffset>=*pwPoolLen)
   return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {    
    pDBSOE=pTEqp->EVSOE.pPoolHead+(i % *pwPoolLen);
    if (pDBSOE->byValue==0) break;
    p->wNo=pDBSOE->wNo;
    p->byValue=pDBSOE->byValue;
    p->Time=pDBSOE->Time;
    p++; 
    j++;
    k+=sizeof(VDBSOE);
    if (k>wBufLen-sizeof(VDBSOE)) break;
  }
  
  return(j);    
}
#endif

int YCLimitCheckDo(int in, DWORD dwT, BYTE *pbyFlag, DWORD *pdwCnt)
{
	if (in) //上升沿
	{
		*pbyFlag &= (~0x04);    //清下降沿启动计数器标志
		if (!(*pbyFlag & 0x01))
		{
			if (*pbyFlag & 0x02)  //已启动上升计数器
			{
				if ((Get100usCnt() - *pdwCnt) >= dwT)
				{
					*pbyFlag |= 0x01;
					return TRUE;
				}		
			}
			else
			{
				*pdwCnt = Get100usCnt();
				*pbyFlag |= 0x02;
			}
		}	 
	}
	else
	{
		*pbyFlag &= (~0x02);   //清上升沿启动计数器标志
		if ((*pbyFlag & 0x01))
		{
			if (*pbyFlag & 0x04)  //已启动下降计数器
			{
				if ((Get100usCnt() - *pdwCnt) >= dwT)
				{
					*pbyFlag &= (~0x01);
					return TRUE;
				}		
			}
			else
			{
				*pdwCnt = Get100usCnt();
				*pbyFlag |= 0x04;
			}
		}	 		
	}			

	return FALSE;
}

void YCLimitCheck(WORD wEqpID, struct VTrueEqp *pTEqp, struct VTrueYC *pYC, struct VTrueYCCfg *pCfg)
{
    int in;
	DWORD cfg;
	struct VDBYX DBYX;
	struct VDBSOE DBSOE;
#ifdef INCULDE_COS_CFG
	struct VDBCOS DBCOS;	
#endif
	
	
	cfg = pCfg->dwCfg&0x30;  //D4-D5
    if (cfg)
    {
        if (cfg == 0x10)  //越上限检查
			in = (pYC->lValue > pCfg->lLimit1) ? 1:0;
		else              //越下限检查
			in = (pYC->lValue < pCfg->lLimit1) ? 1:0;

#ifdef	_GUANGZHOU_TEST_VER_
		if (pYC->lValue == 0) in = 0;    //停电时不报
#endif		

		if (YCLimitCheckDo(in, pCfg->dwLimitT1*10, &pCfg->byL1Flag, &pCfg->dwL1Cnt) == TRUE)
		{
			if (pCfg->byL1Flag & 0x01)
				DBYX.byValue = 0x81;
			else
				DBYX.byValue = 0x01;
			DBYX.wNo = pCfg->wLimit1VYxNo+pTEqp->Cfg.wSYXNum;
			WriteSYX(wEqpID, 1, &DBYX);
#ifdef INCULDE_COS_CFG
			DBCOS.wNo=DBYX.wNo;
			DBCOS.byValue=DBYX.byValue;
			atomicWriteSCOS(wEqpID,1,&DBCOS,TRUE,FALSE,TRUE); 
#endif
			DBSOE.wNo=DBYX.wNo;
			DBSOE.byValue=DBYX.byValue;
			GetSysClock(&DBSOE.Time,CALCLOCK);
			atomicWriteSSOE(wEqpID,1,&DBSOE,TRUE,FALSE,TRUE); 
		}
	}

	cfg = pCfg->dwCfg&0xC0;  //D6-D7
	if (cfg)
	{
		if (cfg == 0x40)  //越上限检查
			in = (pYC->lValue > pCfg->lLimit2) ? 1:0;
		else			  //越下限检查
			in = (pYC->lValue < pCfg->lLimit2) ? 1:0;

#ifdef	_GUANGZHOU_TEST_VER_
		if (pYC->lValue == 0) in = 0;    //停电时不报
#endif		

		if (YCLimitCheckDo(in, pCfg->dwLimitT2*10, &pCfg->byL2Flag, &pCfg->dwL2Cnt) == TRUE)
		{
			if (pCfg->byL2Flag & 0x01)
				DBYX.byValue = 0x81;
			else
				DBYX.byValue = 0x01;
			DBYX.wNo = pCfg->wLimit2VYxNo+pTEqp->Cfg.wSYXNum;
			WriteSYX(wEqpID, 1, &DBYX);
#ifdef INCULDE_COS_CFG
			DBCOS.wNo=DBYX.wNo;
			DBCOS.byValue=DBYX.byValue;
			atomicWriteSCOS(wEqpID,1,&DBCOS,TRUE,FALSE,TRUE); 
#endif
			DBSOE.wNo=DBYX.wNo;
			DBSOE.byValue=DBYX.byValue;
			GetSysClock(&DBSOE.Time,CALCLOCK);
			atomicWriteSSOE(wEqpID,1,&DBSOE,TRUE,FALSE,TRUE); 
		}
	}
}

/*------------------------------------------------------------------------
 Procedure:     atomicWriteYCF_F ID:1
 Purpose:       写离散点遥测(浮点格式,带一字节标志)
 Input:         注意:浮点格式极值和遥测越限失效
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteYCF_F(WORD wEqpID,WORD wNum, struct VDBYCF_F *buf, BOOL bLock)
{
  struct VTrueEqp *pTEqp;
  struct VDBYCF_F * p=buf;
  struct VTrueYC *pYC;
  struct VTrueYCCfg *pCfg;
  float fValue;
  int i,maxno;
  //struct VMaxMinYC *pMaxMinYC;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))  return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0)  return;

  smMTake(dwYCSem);  	  
  
  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno) || ((p->byFlag&0x01)==0))
    {
      p++;
      continue;
    } 

    pCfg=pTEqp->pYCCfg+p->wNo;
	
	if (bLock && (pCfg->dwCfg&0x80000000))   //lock
	{
	  p++;
	  continue;
	} 
    
    pYC=pTEqp->pYC+p->wNo;   
	
    if (pTEqp->Cfg.dwFlag&YCQUOTIETYENABLED)
      fValue = pCfg->lA*p->fValue/pCfg->lB+pCfg->lC;
	else
	  fValue = p->fValue;

	memcpy(&pYC->lValue, &fValue, sizeof(fValue));

	if (pCfg->dwCfg&0x80000000)  //封锁
		pCfg->dwCfg |= 0x20000000;  //被取代
	else
		pCfg->dwCfg &= ~0x20000000;	//未被取代

	pYC->byFlag=p->byFlag;
	GetValueFlag(pCfg->dwCfg, &pYC->byFlag);

	pYC->byFlag |= 0x40;   //强制浮点型

	/*if (pCfg->dwCfg & 0x100)
	{
		pMaxMinYC = pTEqp->pMaxMinYC+pCfg->wMaxMinNo;

		if (pYC->lValue >= pMaxMinYC->lMax)
		{
			pMaxMinYC->lMax = pYC->lValue;
			GetSysClock(&pMaxMinYC->max_tm, CALCLOCK);
		}
		if (pYC->lValue <= pMaxMinYC->lMin)
		{
			pMaxMinYC->lMin = pYC->lValue;
			GetSysClock(&pMaxMinYC->min_tm, CALCLOCK);
		}				
	}
	
	YCLimitCheck(wEqpID, pTEqp, pYC, pCfg);*/	
    p++;   
  }
   
  smMGive(dwYCSem);
}  

/*------------------------------------------------------------------------
 Procedure:     atomicWriteYC ID:1
 Purpose:       写离散点遥测(无标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteYC_F(WORD wEqpID,WORD wNum, struct VDBYC_F *buf, BOOL bLock)
{
  struct VTrueEqp *pTEqp;
  struct VDBYC_F * p=buf;
  struct VTrueYC *pYC;
  struct VTrueYCCfg *pCfg;
  float fValue;
  int i,maxno;
  //struct VMaxMinYC *pMaxMinYC;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))  return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0)  return;

  smMTake(dwYCSem);  	  
  
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    } 

    pCfg=pTEqp->pYCCfg+p->wNo;
	
	if (bLock && (pCfg->dwCfg&0x80000000))   //lock
	{
	  p++;
	  continue;
	} 
	    
    pYC=pTEqp->pYC+p->wNo;   
    
    if (pTEqp->Cfg.dwFlag&YCQUOTIETYENABLED)
      fValue = pCfg->lA*p->fValue/pCfg->lB+pCfg->lC;
	else
	  fValue = p->fValue;

	memcpy(&pYC->lValue, &fValue, sizeof(fValue));

	if (pCfg->dwCfg&0x80000000)  //封锁
		pCfg->dwCfg |= 0x20000000;  //被取代
	else
		pCfg->dwCfg &= ~0x20000000;	//未被取代	

	GetValueFlag(pCfg->dwCfg, &pYC->byFlag);

	pYC->byFlag |= 0x40;   //强制浮点型
	
	/*if (pCfg->dwCfg & 0x100)
	{
		pMaxMinYC = pTEqp->pMaxMinYC+pCfg->wMaxMinNo;

		if (pYC->lValue >= pMaxMinYC->lMax)
		{
			pMaxMinYC->lMax = pYC->lValue;
			GetSysClock(&pMaxMinYC->max_tm, CALCLOCK);
		}
		if (pYC->lValue <= pMaxMinYC->lMin)
		{
			pMaxMinYC->lMin = pYC->lValue;
			GetSysClock(&pMaxMinYC->min_tm, CALCLOCK);
		}				
	}
	
	YCLimitCheck(wEqpID, pTEqp, pYC, pCfg);*/	
    p++;   
  }
   
  smMGive(dwYCSem);
}  

/*------------------------------------------------------------------------
 Procedure:     atomicWriteYCF ID:1
 Purpose:       写离散点遥测(带一字节标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteYCF_L(WORD wEqpID,WORD wNum, struct VDBYCF_L *buf, BOOL bLock)
{
  struct VTrueEqp *pTEqp;
  struct VDBYCF_L * p=buf;
  struct VTrueYC *pYC;
  struct VTrueYCCfg *pCfg;
  struct VMaxMinYC *pMaxMinYC;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))  return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0)  return;

  smMTake(dwYCSem);  	  
  
  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno) || ((p->byFlag&0x01)==0))
    {
      p++;
      continue;
    } 

    pCfg=pTEqp->pYCCfg+p->wNo;
	
	if (bLock && (pCfg->dwCfg&0x80000000))   //lock
	{
	  p++;
	  continue;
	} 
    
    pYC=pTEqp->pYC+p->wNo;   
	
    if (pTEqp->Cfg.dwFlag&YCQUOTIETYENABLED)
      pYC->lValue=pCfg->lA*p->lValue/pCfg->lB+pCfg->lC;
    else
      pYC->lValue=p->lValue;

	if (pCfg->dwCfg&0x80000000)  //封锁
		pCfg->dwCfg |= 0x20000000;  //被取代
	else
		pCfg->dwCfg &= ~0x20000000;	//未被取代

	pYC->byFlag=p->byFlag;
	GetValueFlag(pCfg->dwCfg, &pYC->byFlag);

	pYC->byFlag &= 0xBF;   //强制整型

	if (pCfg->dwCfg & 0x100)
	{
		pMaxMinYC = pTEqp->pMaxMinYC+pCfg->wMaxMinNo;

		if (pYC->lValue > pMaxMinYC->lMax)
		{
			pMaxMinYC->lMax = pYC->lValue;
			GetSysClock(&pMaxMinYC->max_tm, CALCLOCK);
		}
 #if (TYPE_USER == USER_BEIJING)
		if(pYC->lValue != 0)
 #endif
		{
		if (pYC->lValue < pMaxMinYC->lMin)
		{
			pMaxMinYC->lMin = pYC->lValue;
			GetSysClock(&pMaxMinYC->min_tm, CALCLOCK);
		}
	}		
	}
	
	YCLimitCheck(wEqpID, pTEqp, pYC, pCfg);	
    p++;   
  }
   
  smMGive(dwYCSem);
}  

/*------------------------------------------------------------------------
 Procedure:     atomicWriteYC ID:1
 Purpose:       写离散点遥测(无标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteYC_L(WORD wEqpID,WORD wNum, struct VDBYC_L *buf, BOOL bLock)
{
  struct VTrueEqp *pTEqp;
  struct VDBYC_L * p=buf;
  struct VTrueYC *pYC;
  struct VTrueYCCfg *pCfg;
  int i,maxno;
  struct VMaxMinYC *pMaxMinYC;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))  return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0)  return;

  smMTake(dwYCSem);  	  
  
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    } 

    pCfg=pTEqp->pYCCfg+p->wNo;
	
	if (bLock && (pCfg->dwCfg&0x80000000))   //lock
	{
	  p++;
	  continue;
	} 
	    
    pYC=pTEqp->pYC+p->wNo;   
    
    if (pTEqp->Cfg.dwFlag&YCQUOTIETYENABLED)
      pYC->lValue=pCfg->lA*p->lValue/pCfg->lB+pCfg->lC;
    else
      pYC->lValue=p->lValue;

	if (pCfg->dwCfg&0x80000000)  //封锁
		pCfg->dwCfg |= 0x20000000;  //被取代
	else
		pCfg->dwCfg &= ~0x20000000;	//未被取代	

	GetValueFlag(pCfg->dwCfg, &pYC->byFlag);

	pYC->byFlag &= 0xBF;   //强制整型
	
	if (pCfg->dwCfg & 0x100)
	{
		pMaxMinYC = pTEqp->pMaxMinYC+pCfg->wMaxMinNo;

		if (pYC->lValue > pMaxMinYC->lMax)
		{
			pMaxMinYC->lMax = pYC->lValue;
			GetSysClock(&pMaxMinYC->max_tm, CALCLOCK);
		}
 #if (TYPE_USER == USER_BEIJING)
		if(pYC->lValue != 0)
 #endif
		{
		if (pYC->lValue < pMaxMinYC->lMin)
		{
			pMaxMinYC->lMin = pYC->lValue;
			GetSysClock(&pMaxMinYC->min_tm, CALCLOCK);
		}
		}				
	}
	
	YCLimitCheck(wEqpID, pTEqp, pYC, pCfg);	
    p++;   
  }
   
  smMGive(dwYCSem);
}  

/*------------------------------------------------------------------------
 Procedure:     atomicWriteYCF ID:1
 Purpose:       写离散点遥测(带一字节标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteYCF(WORD wEqpID,WORD wNum, struct VDBYCF *buf, BOOL bLock)
{
  struct VTrueEqp *pTEqp;
  struct VDBYCF * p=buf;
  struct VTrueYC *pYC;
  struct VTrueYCCfg *pCfg;
  struct VMaxMinYC *pMaxMinYC;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))  return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0)  return;

  smMTake(dwYCSem);  	  
  
  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno) || ((p->byFlag&0x01)==0))
    {
      p++;
      continue;
    } 

    pCfg=pTEqp->pYCCfg+p->wNo;
	
	if (bLock && (pCfg->dwCfg&0x80000000))   //lock
	{
	  p++;
	  continue;
	} 
    
    pYC=pTEqp->pYC+p->wNo;   
	
    if (pTEqp->Cfg.dwFlag&YCQUOTIETYENABLED)
      pYC->lValue=pCfg->lA*p->nValue/pCfg->lB+pCfg->lC;
    else
      pYC->lValue=p->nValue;

	if (pCfg->dwCfg&0x80000000)  //封锁
		pCfg->dwCfg |= 0x20000000;  //被取代
	else
		pCfg->dwCfg &= ~0x20000000;	//未被取代

	pYC->byFlag=p->byFlag;
	GetValueFlag(pCfg->dwCfg, &pYC->byFlag);

	pYC->byFlag &= 0xBF;   //强制整型

	if (pCfg->dwCfg & 0x100)
	{
		pMaxMinYC = pTEqp->pMaxMinYC+pCfg->wMaxMinNo;

		if (pYC->lValue > pMaxMinYC->lMax)
		{
			pMaxMinYC->lMax = pYC->lValue;
			GetSysClock(&pMaxMinYC->max_tm, CALCLOCK);
		}
 #if (TYPE_USER == USER_BEIJING)
		if(pYC->lValue != 0)
 #endif
		{
		if (pYC->lValue < pMaxMinYC->lMin)
		{
			pMaxMinYC->lMin = pYC->lValue;
			GetSysClock(&pMaxMinYC->min_tm, CALCLOCK);
		}
	}		
	}
	
	YCLimitCheck(wEqpID, pTEqp, pYC, pCfg);	
    p++;   
  }
   
  smMGive(dwYCSem);
}  


/*------------------------------------------------------------------------
 Procedure:     atomicWriteYC ID:1
 Purpose:       写离散点遥测(无标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void atomicWriteYC(WORD wEqpID,WORD wNum, struct VDBYC *buf, BOOL bLock)
{
  struct VTrueEqp *pTEqp;
  struct VDBYC * p=buf;
  struct VTrueYC *pYC;
  struct VTrueYCCfg *pCfg;
  int i,maxno;
  struct VMaxMinYC *pMaxMinYC;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))  return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0)  return;

  smMTake(dwYCSem);  	  
  
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    } 

    pCfg=pTEqp->pYCCfg+p->wNo;
	
	if (bLock && (pCfg->dwCfg&0x80000000))   //lock
	{
	  p++;
	  continue;
	} 
	    
    pYC=pTEqp->pYC+p->wNo;   
    
    if (pTEqp->Cfg.dwFlag&YCQUOTIETYENABLED)
      pYC->lValue=pCfg->lA*p->nValue/pCfg->lB+pCfg->lC;
    else
      pYC->lValue=p->nValue;

	if (pCfg->dwCfg&0x80000000)  //封锁
		pCfg->dwCfg |= 0x20000000;  //被取代
	else
		pCfg->dwCfg &= ~0x20000000;	//未被取代

	GetValueFlag(pCfg->dwCfg, &pYC->byFlag);

	pYC->byFlag &= 0xBF;   //强制整型

	if (pCfg->dwCfg & 0x100)
	{
		pMaxMinYC = pTEqp->pMaxMinYC+pCfg->wMaxMinNo;

		if (pYC->lValue > pMaxMinYC->lMax)
		{
			pMaxMinYC->lMax = pYC->lValue;
			GetSysClock(&pMaxMinYC->max_tm, CALCLOCK);
		}
 #if (TYPE_USER == USER_BEIJING)
		if(pYC->lValue != 0)
 #endif
		{
		if (pYC->lValue < pMaxMinYC->lMin)
		{
			pMaxMinYC->lMin = pYC->lValue;
			GetSysClock(&pMaxMinYC->min_tm, CALCLOCK);
		}
		}				
	}
	
	YCLimitCheck(wEqpID, pTEqp, pYC, pCfg);	
    p++;   
  }
   
  smMGive(dwYCSem);
}  

/*------------------------------------------------------------------------
 Procedure:     atomicReadVEYCF ID:1
 Purpose:       读虚装置离散点遥测子函数(带一字节标志)
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
STATUS atomicGetVEYC_ABC(struct VVirtualEqp *pVEqp, WORD wNo, long *A, long *B, long *C)
{
  struct VVirtualYC *pVYC;
  int maxno;

  maxno=pVEqp->Cfg.wYCNum-1; 
  if (wNo > maxno) return ERROR;

  pVYC=pVEqp->pYC+wNo;
  
  if (A != NULL) *A = pVYC->lA;
  if (B != NULL) *B = pVYC->lB;
  if (C != NULL) *C = pVYC->lC;
  
  return OK;
}

STATUS atomicGetTEYC_ABC(struct VTrueEqp *pTEqp, WORD wNo, long *A, long *B, long *C)
{
  struct VTrueYCCfg *pCfg;
  int maxno;
	
  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (wNo > maxno) return ERROR;

  pCfg=pTEqp->pYCCfg+wNo;

  if (A != NULL) *A = pCfg->lA;
  if (B != NULL) *B = pCfg->lB;
  if (C != NULL) *C = pCfg->lC;

  return OK;
}

/*------------------------------------------------------------------------
 Procedure:     atomicReadVEYCF ID:1
 Purpose:       读虚装置离散点遥测子函数(带一字节标志)
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEYCF_L(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBYCF_L *buf)
{
  int i,j,k,maxno;
  struct VDBYCF_L *p,*q;

  maxno=pVEqp->Cfg.wYCNum-1; 
  if (maxno<0)  return(0);

  smMTake(dwYCSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    q->wNo=p->wNo;
    if (GetYCValue(pVEqp,q->wNo,(long*)&q->lValue,&q->byFlag)==ERROR)
    {
      p++;
      continue;
    }      	
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBYCF_L);
    if (k>wBufLen-sizeof(struct VDBYCF_L)) break;    
  }  
  
  smMGive(dwYCSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadTEYCF ID:1
 Purpose:       读实装置离散点遥测子函数(带一字节标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEYCF_L(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBYCF_L *buf,BOOL asSend)
{
  int i,j,k,maxno;
  WORD no;
  struct VDBYCF_L *p,*q;

  if (asSend)
    maxno=pTEqp->YCSendCfg.wNum-1;
  else
    maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
 
  if (maxno<0) return(0);

  smMTake(dwYCSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    if (asSend)
   	  no=pTEqp->YCSendCfg.pwIndex[p->wNo];
    else
   	  no=p->wNo;
    q->wNo=p->wNo;
	memcpy((void*)&q->lValue, &pTEqp->pYC[no].lValue, sizeof(long));
    q->byFlag=pTEqp->pYC[no].byFlag;
	if (pTEqp->pYCCfg[no].dwCfg&0x200)   //active send
		q->byFlag |= 0x80;
	else
		q->byFlag &= 0x7F;		
	GetValueFlag(pTEqp->pYCCfg[no].dwCfg, &q->byFlag);
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBYCF_L);
    if (k>wBufLen-sizeof(struct VDBYCF_L)) break;
  }  

  smMGive(dwYCSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadYCF ID:1
 Purpose:       读离散点遥测子函数(带一字节标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadYCF_L(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYCF_L *buf,BOOL asSend)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBYCF_L)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if (pVEqp->pYC==NULL)
        return(0);
      else
        j=atomicReadVEYCF_L(pVEqp,wNum,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))
      return(0);
    else
      j=atomicReadTEYCF_L(pTEqp,wNum,wBufLen,buf,asSend);
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadVEYC ID:1
 Purpose:       读虚装置离散点遥测子函数(无标志)
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEYC_L(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBYC_L *buf)
{
  int i,j,k,maxno;
  struct VDBYC_L *p,*q;
  BYTE flag;
  
  maxno=pVEqp->Cfg.wYCNum-1; 
  if (maxno<0)  return(0);

  smMTake(dwYCSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    q->wNo=p->wNo;
    if (GetYCValue(pVEqp,q->wNo,(long*)&q->lValue,&flag)==ERROR)
    {
      p++;
      continue;
    }      	
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBYC_L);
    if (k>wBufLen-sizeof(struct VDBYC_L)) break;    
  }  
  
  smMGive(dwYCSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadTEYC ID:1
 Purpose:       读实装置离散点遥测子函数(无标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEYC_L(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBYC_L *buf,BOOL asSend)
{
  int i,j,k,maxno;
  struct VDBYC_L *p,*q;
  WORD no;
  
  if (asSend)
    maxno=pTEqp->YCSendCfg.wNum-1;
  else
	maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
 
  if (maxno<0) return(0);

  smMTake(dwYCSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    if (asSend)
   	  no=pTEqp->YCSendCfg.pwIndex[p->wNo];
    else
   	  no=p->wNo;
    q->wNo=p->wNo;
	memcpy((void*)&q->lValue, &pTEqp->pYC[no].lValue, sizeof(long));
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBYC_L);
    if (k>wBufLen-sizeof(struct VDBYC_L)) break;
  }  

  smMGive(dwYCSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadYC ID:1
 Purpose:       读离散点遥测子函数(无标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadYC_L(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYC_L *buf,BOOL asSend)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBYC_L)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if (pVEqp->pYC==NULL)
        return(0);
      else
        j=atomicReadVEYC_L(pVEqp,wNum,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))
      return(0);
    else
      j=atomicReadTEYC_L(pTEqp,wNum,wBufLen,buf,asSend);
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadVEYCF ID:1
 Purpose:       读虚装置离散点遥测子函数(带一字节标志)
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEYCF(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBYCF *buf)
{
  int i,j,k,maxno;
  struct VDBYCF *p,*q;
  long value;

  maxno=pVEqp->Cfg.wYCNum-1; 
  if (maxno<0)  return(0);

  smMTake(dwYCSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    q->wNo=p->wNo;
    if (GetYCValue(pVEqp,q->wNo,&value,&q->byFlag)==ERROR)
    {
      p++;
      continue;
    }    
	q->nValue=(short)value;
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBYCF);
    if (k>wBufLen-sizeof(struct VDBYCF)) break;    
  }  
  
  smMGive(dwYCSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadTEYCF ID:1
 Purpose:       读实装置离散点遥测子函数(带一字节标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEYCF(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBYCF *buf,BOOL asSend)
{
  int i,j,k,maxno;
  WORD no;
  struct VDBYCF *p,*q;
  float fvalue;
  
  if (asSend)
    maxno=pTEqp->YCSendCfg.wNum-1;
  else
    maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
 
  if (maxno<0) return(0);

  smMTake(dwYCSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    if (asSend)
   	  no=pTEqp->YCSendCfg.pwIndex[p->wNo];
    else
   	  no=p->wNo;
    q->wNo=p->wNo;
	if (pTEqp->pYC[no].byFlag & 0x40)  //浮点格式
	{
		fvalue = (float)pTEqp->pYC[no].lValue;
		q->nValue = (short)fvalue;
	}	
	else  	
    	q->nValue=(short)pTEqp->pYC[no].lValue;
    q->byFlag=pTEqp->pYC[no].byFlag;
	if (pTEqp->pYCCfg[no].dwCfg&0x200)   //active send
		q->byFlag |= 0x80;
	else
		q->byFlag &= 0x7F;	
	GetValueFlag(pTEqp->pYCCfg[no].dwCfg, &q->byFlag);
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBYCF);
    if (k>wBufLen-sizeof(struct VDBYCF)) break;
  }  

  smMGive(dwYCSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadYCF ID:1
 Purpose:       读离散点遥测子函数(带一字节标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadYCF(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYCF *buf,BOOL asSend)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBYCF)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if (pVEqp->pYC==NULL)
        return(0);
      else
        j=atomicReadVEYCF(pVEqp,wNum,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))
      return(0);
    else
      j=atomicReadTEYCF(pTEqp,wNum,wBufLen,buf,asSend);
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadVEYC ID:1
 Purpose:       读虚装置离散点遥测子函数(无标志)
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEYC(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBYC *buf)
{
  int i,j,k,maxno;
  struct VDBYC *p,*q;
  BYTE flag;
  long value;
  
  maxno=pVEqp->Cfg.wYCNum-1; 
  if (maxno<0)  return(0);

  smMTake(dwYCSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    q->wNo=p->wNo;
    if (GetYCValue(pVEqp,q->wNo,&value,&flag)==ERROR)
    {
      p++;
      continue;
    }    
	q->nValue=(short)value;	
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBYC);
    if (k>wBufLen-sizeof(struct VDBYC)) break;    
  }  
  
  smMGive(dwYCSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadTEYC ID:1
 Purpose:       读实装置离散点遥测子函数(无标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEYC(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBYC *buf,BOOL asSend)
{
  int i,j,k,maxno;
  struct VDBYC *p,*q;
  WORD no;
  float fvalue;
  
  if (asSend)
    maxno=pTEqp->YCSendCfg.wNum-1;
  else
	maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
 
  if (maxno<0) return(0);

  smMTake(dwYCSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    if (asSend)
   	  no=pTEqp->YCSendCfg.pwIndex[p->wNo];
    else
   	  no=p->wNo;
    q->wNo=p->wNo;
	if (pTEqp->pYC[no].byFlag & 0x40)  //浮点格式
	{
		fvalue = (float)pTEqp->pYC[no].lValue;
		q->nValue = (short)fvalue;
	}	
	else  	
    	q->nValue=(short)pTEqp->pYC[no].lValue;
	p++;
    q++;
    j++;
    k+=sizeof(struct VDBYC);
    if (k>wBufLen-sizeof(struct VDBYC)) break;
  }  

  smMGive(dwYCSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadYC ID:1
 Purpose:       读离散点遥测子函数(无标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadYC(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYC *buf,BOOL asSend)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBYC)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if (pVEqp->pYC==NULL)
        return(0);
      else
        j=atomicReadVEYC(pVEqp,wNum,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))
      return(0);
    else
      j=atomicReadTEYC(pTEqp,wNum,wBufLen,buf,asSend);
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadVEDDF ID:1
 Purpose:       读虚装置离散点电度子函数(带一字节标志)
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEDDF(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBDDF *buf)
{
  int i,j,k,maxno;
  struct VDBDDF *p,*q;
  struct VCalClock Time;
  
  maxno=pVEqp->Cfg.wDDNum-1; 
  if (maxno<0)  return(0);

  smMTake(dwDDSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    q->wNo=p->wNo;
    if (GetDDValue(pVEqp,q->wNo,(long*)&q->lValue,&q->byFlag,&Time)==ERROR)
    {
      p++;
      continue;
    }      	
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBDDF);
    if (k>wBufLen-sizeof(struct VDBDDF)) break;    
  }  
  
  smMGive(dwDDSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadTEDDF ID:1
 Purpose:       读实装置离散点电度子函数(带一字节标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEDDF(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBDDF *buf,BOOL asSend)
{
  int i,j,k,maxno;
  struct VDBDDF *p,*q;
  WORD no;

  if (asSend)
    maxno=pTEqp->DDSendCfg.wNum-1;
  else
    maxno=pTEqp->Cfg.wDDNum-1;
 
  if (maxno<0) return(0);

  smMTake(dwDDSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    if (asSend)
   	  no=pTEqp->DDSendCfg.pwIndex[p->wNo];
    else
   	  no=p->wNo;
    q->wNo=p->wNo;
    q->lValue=pTEqp->pDD[no].lValue;
    q->byFlag=pTEqp->pDD[no].byFlag;
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBDDF);
    if (k>wBufLen-sizeof(struct VDBDDF)) break;
  }  

  smMGive(dwDDSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadDDF ID:1
 Purpose:       读离散点遥测子函数(带一字节标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadDDF(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDDF *buf,BOOL asSend)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBDDF)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if (pVEqp->pDD==NULL)
        return(0);
      else
        j=atomicReadVEDDF(pVEqp,wNum,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL))
      return(0);
    else
      j=atomicReadTEDDF(pTEqp,wNum,wBufLen,buf,asSend);
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure: 	atomicReadVEDDT ID:1
 Purpose:		读虚装置离散点电度子函数(带时标)
 Input: 		
 Output:		实际所读的个数	
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEDDFT(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBDDFT *buf)
{
  int i,j,k,maxno;
  struct VDBDDFT *p,*q;
  
  maxno=pVEqp->Cfg.wDDNum-1; 
  if (maxno<0)	return(0);

  smMTake(dwDDSem);	
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
	if (p->wNo>maxno)
	{
	  p++;
	  continue;
	}  
	q->wNo=p->wNo;
	if (GetDDValue(pVEqp,q->wNo,(long*)&q->lValue,&q->byFlag,&q->Time)==ERROR)
	{
	  p++;
	  continue;
	}		
	p++;
	q++;
	j++;
	k+=sizeof(struct VDBDDFT);
	if (k>wBufLen-sizeof(struct VDBDDFT)) break;	
  }  
  
  smMGive(dwDDSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure: 	atomicReadTEDDT ID:1
 Purpose:		读实装置离散点电度子函数(带时标)
 Input: 		asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数	
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEDDFT(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBDDFT *buf,BOOL asSend)
{
  int i,j,k,maxno;
  struct VDBDDFT *p,*q;
  WORD no;

  if (asSend)
	maxno=pTEqp->DDSendCfg.wNum-1;
  else
	maxno=pTEqp->Cfg.wDDNum-1;
 
  if (maxno<0) return(0);

  smMTake(dwDDSem);	
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
	if (p->wNo>maxno)
	{
	  p++;
	  continue;
	}  
	if (asSend)
	  no=pTEqp->DDSendCfg.pwIndex[p->wNo];
	else
	  no=p->wNo;
	q->wNo=p->wNo;
	q->lValue=pTEqp->pDD[no].lValue;
	q->byFlag=pTEqp->pDD[no].byFlag;
	q->Time=pTEqp->pDD[no].Time;
	p++;
	q++;
	j++;
	k+=sizeof(struct VDBDDFT);
	if (k>wBufLen-sizeof(struct VDBDDFT)) break;
  }  

  smMGive(dwDDSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure: 	atomicReadDDT ID:1
 Purpose:		读离散点遥测子函数(带时标和一字节标志)
 Input: 		asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数	
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadDDFT(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDDFT *buf,BOOL asSend)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBDDF)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
	if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
	  return(0);
	else
	{
	  if (pVEqp->pDD==NULL)
		return(0);
	  else
		j=atomicReadVEDDFT(pVEqp,wNum,wBufLen,buf);
	}	
  } 
  else
  {
	if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL))
	  return(0);
	else
	  j=atomicReadTEDDFT(pTEqp,wNum,wBufLen,buf,asSend);
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure:     atomicReadVEDD ID:1
 Purpose:       读虚装置离散点遥测子函数(无标志)
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEDD(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBDD *buf)
{
  int i,j,k,maxno;
  struct VDBDD *p,*q;
  BYTE flag;  
  struct VCalClock Time;
  
  maxno=pVEqp->Cfg.wDDNum-1; 
  if (maxno<0)  return(0);

  smMTake(dwDDSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    q->wNo=p->wNo;
    if (GetDDValue(pVEqp,q->wNo,(long*)&q->lValue,&flag,&Time)==ERROR)
    {
      p++;
      continue;
    }      	
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBDD);
    if (k>wBufLen-sizeof(struct VDBDD)) break;    
  }  
  
  smMGive(dwDDSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadTEDD ID:1
 Purpose:       读实装置离散点电度子函数(无标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEDD(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBDD *buf,BOOL asSend)
{
  int i,j,k,maxno;
  struct VDBDD *p,*q;
  WORD no;
  
  if (asSend)
    maxno=pTEqp->DDSendCfg.wNum-1;
  else
    maxno=pTEqp->Cfg.wDDNum-1;
 
  if (maxno<0) return(0);

  smMTake(dwDDSem);  
  
  p=q=buf; 
  j=k=0; 
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    }  
    if (asSend)
   	  no=pTEqp->DDSendCfg.pwIndex[p->wNo];
    else
   	  no=p->wNo;
    q->wNo=p->wNo;
    q->lValue=pTEqp->pDD[no].lValue;
    p++;
    q++;
    j++;
    k+=sizeof(struct VDBDD);
    if (k>wBufLen-sizeof(struct VDBDD)) break;
  }  

  smMGive(dwDDSem);
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadDD ID:1
 Purpose:       读离散点电度子函数(无标志)
 Input:         asSend: TRUE: by sendno read FALSE: by original index read
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadDD(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDD *buf,BOOL asSend)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBYC)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if (pVEqp->pDD==NULL)
        return(0);
      else
        j=atomicReadVEDD(pVEqp,wNum,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL))
      return(0);
    else
      j=atomicReadTEDD(pTEqp,wNum,wBufLen,buf,asSend);
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadTEYK ID:1
 Purpose:       读实装置遥控配置子函数
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEYK(struct VTrueEqp *pTEqp, WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf)
{
  int i,k,maxno;
  WORD *p,j;
  struct VTrueCtrl *pYK;
  
  maxno=pTEqp->Cfg.wYKNum-1; 
  if (maxno<0) return(0);
  
  p=buf;  
  j=k=0;
  pYK=pTEqp->pYK+wBeginNo;
  
  for (i=0;i<wNum;i++)
  {
    if (wBeginNo+i > maxno ) break;

    *p=pYK->wID;
    p++;
    pYK++;   
    j++;	
    k+=sizeof(WORD);
    if (k>wBufLen-sizeof(WORD)) break;    
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     atomicReadVEYK ID:1
 Purpose:       读实装置遥控配置子函数
 Input:         
 Output:		实际所读的个数  
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEYK(struct VVirtualEqp *pVEqp, WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf)
{
  int i,k,maxno;
  WORD *p,j;
  struct VVirtualCtrl *pYK;
  
  maxno=pVEqp->Cfg.wYKNum-1; 
  if (maxno<0) return(0);
  
  p=buf;  
  j=k=0;
  pYK=pVEqp->pYK+wBeginNo;
  
  for (i=0;i<wNum;i++)
  {
    if (wBeginNo+i > maxno ) break;

    *p=pYK->wID;
    p++;
    pYK++;   
    j++;	
    k+=sizeof(WORD);
    if (k>wBufLen-sizeof(WORD)) break;    
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure: 	atomicReadTEYT ID:1
 Purpose:		读实装置遥调配置子函数
 Input: 		
 Output:		实际所读的个数	
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadTEYT(struct VTrueEqp *pTEqp, WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf)
{
  int i,k,maxno;
  WORD *p,j;
  struct VTrueCtrl *pYT;
  
  maxno=pTEqp->Cfg.wYTNum-1; 
  if (maxno<0) return(0);
  
  p=buf;  
  j=k=0;
  pYT=pTEqp->pYT+wBeginNo;
  
  for (i=0;i<wNum;i++)
  {
	if (wBeginNo+i > maxno ) break;

	*p=pYT->wID;
	p++;
	pYT++;	 
	j++;	
	k+=sizeof(WORD);
	if (k>wBufLen-sizeof(WORD)) break;	  
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure: 	atomicReadVEYT ID:1
 Purpose:		读实装置遥调配置子函数
 Input: 		
 Output:		实际所读的个数	
 Errors:
------------------------------------------------------------------------*/
WORD atomicReadVEYT(struct VVirtualEqp *pVEqp, WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf)
{
  int i,k,maxno;
  WORD *p,j;
  struct VVirtualCtrl *pYT;
  
  maxno=pVEqp->Cfg.wYTNum-1; 
  if (maxno<0) return(0);
  
  p=buf;  
  j=k=0;
  pYT=pVEqp->pYT+wBeginNo;
  
  for (i=0;i<wNum;i++)
  {
	if (wBeginNo+i > maxno ) break;

	*p=pYT->wID;
	p++;
	pYT++;	 
	j++;	
	k+=sizeof(WORD);
	if (k>wBufLen-sizeof(WORD)) break;	  
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure:     ReadTrueEqpInfo ID:1
 Purpose:       读实装置的信息
 Input:         asSend: TRUE: sendno Num FALSE:  original Num
 Output:        OK,ERROR
 Errors:
------------------------------------------------------------------------*/
STATUS ReadTrueEqpInfo(WORD wTaskID,WORD wEqpID,struct VTrueEqp *pTEqp,struct VTVEqpInfo * pTrueEqpInfo,BOOL asSend)
{     
  pTrueEqpInfo->wType=TRUEEQP;

  /*pTrueEqpInfo->wSourceAddr=pTEqp->Cfg.wSourceAddr;*/
  pTrueEqpInfo->wSourceAddr=g_Sys.AddrInfo.wAsMasterAddr;
  pTrueEqpInfo->wDesAddr=pTEqp->Cfg.wDesAddr;
  memcpy(pTrueEqpInfo->sExtAddr, pTEqp->Cfg.sExtAddr, 12);
  strcpy(pTrueEqpInfo->sTelNo, pTEqp->Cfg.sTelNo);
  
  pTrueEqpInfo->dwFlag=pTEqp->Cfg.dwFlag;

  if (asSend)
  {
	pTrueEqpInfo->wYCNum=pTEqp->YCSendCfg.wNum;
	pTrueEqpInfo->wVYCNum=0;
	pTrueEqpInfo->wDYXNum=pTEqp->DYXSendCfg.wNum;
	pTrueEqpInfo->wSYXNum=pTEqp->SYXSendCfg.wNum;
	pTrueEqpInfo->wVYXNum=0;	 //虚拟遥信个数
	pTrueEqpInfo->wDDNum=pTEqp->DDSendCfg.wNum;
  }
  else
  {
    pTrueEqpInfo->wYCNum=pTEqp->Cfg.wYCNum;
    pTrueEqpInfo->wVYCNum=pTEqp->Cfg.wVYCNum;	//虚拟遥测个数
    pTrueEqpInfo->wDYXNum=pTEqp->Cfg.wDYXNum;
    pTrueEqpInfo->wSYXNum=pTEqp->Cfg.wSYXNum;
    pTrueEqpInfo->wVYXNum=pTEqp->Cfg.wVYXNum;   //虚拟遥信个数
    pTrueEqpInfo->wDDNum=pTEqp->Cfg.wDDNum;
  }

  pTrueEqpInfo->wYKNum=pTEqp->Cfg.wYKNum;
  pTrueEqpInfo->wYTNum=pTEqp->Cfg.wYTNum;
  pTrueEqpInfo->wTSDataNum=pTEqp->Cfg.wTSDataNum;
  pTrueEqpInfo->wTQNum=pTEqp->Cfg.wTQNum;

  if ((pTrueEqpInfo->pEqpRunInfo=GetpRunInfo(wTaskID,wEqpID,NULL))==NULL)  return(ERROR);

  return(OK);  
}

/*------------------------------------------------------------------------
 Procedure:     ReadVirtualEqpInfo ID:1
 Purpose:       读虚装置的信息
 Input:         
 Output:        OK,ERROR
 Errors:
------------------------------------------------------------------------*/
STATUS ReadVirtualEqpInfo(WORD wTaskID,WORD wEqpID,struct VVirtualEqp *pVEqp,struct VTVEqpInfo * pVirtualEqpInfo)
{ 
  pVirtualEqpInfo->wType=VIRTUALEQP;

  /*pVirtualEqpInfo->wSourceAddr=pVEqp->Cfg.wSourceAddr;*/
  pVirtualEqpInfo->wSourceAddr=g_Sys.AddrInfo.wAddr;
  pVirtualEqpInfo->wDesAddr=pVEqp->Cfg.wDesAddr;
  pVirtualEqpInfo->dwFlag=pVEqp->Cfg.dwFlag;
  memcpy(pVirtualEqpInfo->sExtAddr, pVEqp->Cfg.sExtAddr, 12);
  strcpy(pVirtualEqpInfo->sTelNo, pVEqp->Cfg.sTelNo);

  pVirtualEqpInfo->wYCNum=pVEqp->Cfg.wYCNum;
  pVirtualEqpInfo->wVYCNum=0;
  pVirtualEqpInfo->wDYXNum=pVEqp->Cfg.wDYXNum;
  pVirtualEqpInfo->wSYXNum=pVEqp->Cfg.wSYXNum;
  pVirtualEqpInfo->wVYXNum=0;
  pVirtualEqpInfo->wDDNum=pVEqp->Cfg.wDDNum;
  pVirtualEqpInfo->wYKNum=pVEqp->Cfg.wYKNum;
  pVirtualEqpInfo->wYTNum=pVEqp->Cfg.wYTNum;
  pVirtualEqpInfo->wTQNum=pVEqp->Cfg.wTQNum;

  if ((pVirtualEqpInfo->pEqpRunInfo=GetpRunInfo(wTaskID,wEqpID,NULL))==NULL)  return(ERROR);
  
  return(OK);  
}


/*------------------------------------------------------------------------
 Procedure:     ReadEqpGroupInfo ID:1
 Purpose:       读装置组的信息
 Input:         
 Output:        OK,ERROR
 Errors:
------------------------------------------------------------------------*/
STATUS ReadEqpGroupInfo(WORD wTaskID,WORD wEqpID,struct VEqpGroup *pGEqp,struct VEqpGroupInfo * pEqpGroupInfo)
{  
  pEqpGroupInfo->wType=GROUPEQP;
  
  /*pEqpGroupInfo->wSourceAddr=pGEqp->Cfg.wSourceAddr;*/
  pEqpGroupInfo->wSourceAddr=g_Sys.AddrInfo.wAddr;
  pEqpGroupInfo->wDesAddr=pGEqp->Cfg.wDesAddr;
  pEqpGroupInfo->dwFlag=pGEqp->Cfg.dwFlag;
  pEqpGroupInfo->wEqpNum=pGEqp->Cfg.wEqpNum;

  if ((pEqpGroupInfo->pEqpRunInfo=GetpRunInfo(wTaskID,wEqpID,NULL))==NULL)  return(ERROR);

  pEqpGroupInfo->pwEqpID=pGEqp->pwEqpID;
  
  return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     sendYKRet ID:1
 Purpose:       YK 返回
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void sendYKRet(BYTE byStatus)
{
  struct VDBYK *pDBYK;

  pDBYK=(struct VDBYK *)msgbuf.abyData;
  pDBYK->byStatus=byStatus;

  TaskSendMsg(msgbuf.Head.wThID,DB_ID,msgbuf.Head.wEqpID,msgbuf.Head.byMsgID,MA_RET,sizeof(struct VDBYK),&msgbuf);  
}


/*------------------------------------------------------------------------
 Procedure:     TEYKReq ID:1
 Purpose:       实装置YK请求
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void TEYKReq(WORD wToEqpID,struct VDBYK *pDBYK)
{
  int i;
  struct VTrueEqp *pTEqp;
  struct VTrueCtrlCfg  *pCfg;
  struct VDBYK *pOrigDBYK;
  WORD *p; 
  struct VDBCOS DBCOS; 
  struct VDBSOE DBSOE;
    
  pTEqp=g_Sys.Eqp.pInfo[wToEqpID].pTEqpInfo;

  if (pTEqp==NULL)
  {
    sendYKRet(CONTROLIDINVAILD);
    return;
  }  
  	
  if ((pTEqp->pYK==NULL)||(pTEqp->pYKCfg==NULL))
  {
    sendYKRet(CONTROLIDINVAILD);
    return;
  }  

  if (pDBYK->byValue&0x04)  //开关号
  {
    for (i=0;i<pTEqp->Cfg.wYKNum;i++)
  	  if (pTEqp->pYK[i].wID==pDBYK->wID)
  	    break;
  	  
    if (i==pTEqp->Cfg.wYKNum)  
    {
      sendYKRet(CONTROLIDINVAILD);
      return;
    }  
  }  
  else
  {
    if (pDBYK->wID>pTEqp->Cfg.wYKNum)
    {
      sendYKRet(CONTROLIDINVAILD);
      return;
    }  
    i=pDBYK->wID;
  }  
  
  pOrigDBYK=(struct VDBYK *)msgbuf.abyData;
  pCfg=pTEqp->pYKCfg+i;

  if (msgbuf.Head.byMsgID == MI_YKSIM)
  {
	pCfg->byFlag |= 0x08;
	p=(WORD *)(pOrigDBYK+1);
    pCfg->wSimYxTEqpID = *p;
	p++;
	pCfg->wSimYxNo = *p;
	return;
  }
  else if (msgbuf.Head.byMsgID == MI_YKUNSIM)
  {
	pCfg->byFlag &= (~0x08);
	return;
  }
  
  if (pCfg->byFlag&0x08)
  {
      if (msgbuf.Head.byMsgID == MI_YKOPRATE)
      {
          if (pOrigDBYK->byValue&YK_CLOSE)
		  	DBCOS.byValue=DBSOE.byValue=0x81;
		  else
			DBCOS.byValue=DBSOE.byValue=0x01;
		  	
		  DBCOS.wNo = pCfg->wSimYxNo;
		  atomicWriteSCOS(pCfg->wSimYxTEqpID,1,&DBCOS,TRUE,FALSE,FALSE);
		  DBSOE.wNo = pCfg->wSimYxNo; 
		  GetSysClock((void *)&DBSOE.Time,CALCLOCK);
		  atomicWriteSSOE(pCfg->wSimYxTEqpID,1,&DBSOE,TRUE,FALSE,FALSE);
	  }

	  sendYKRet(CONTROLOK);
      return;	  	
  }
  
  //(pCfg->byFlag&0x80)?? onop
  if ((!(pCfg->byFlag&0x01))||(pCfg->byFlag&0x02)||(pCfg->byFlag&0x04))
  {
    sendYKRet(CONTROLFAILED);
    return;
  }  
  else
  {
    pCfg->byFlag|=0x80;
    
	if(pCfg->dwCfg&0x01)
	{
		if(pOrigDBYK->byValue == YKH)
			pOrigDBYK->byValue = YKF;
		else
			pOrigDBYK->byValue = YKH;
	}
	pCfg->dwValue=pOrigDBYK->byValue;
    pCfg->byTaskID=msgbuf.Head.wThID;
    pCfg->wEqpID=msgbuf.Head.wEqpID;
    pCfg->wID=pOrigDBYK->wID;
    //GetSysClock(&pCfg->Time,CALCLOCK);

    if (pOrigDBYK->byValue&0x04) //开关号
      pOrigDBYK->wID=pTEqp->pYK[i].wID;
    else
      pOrigDBYK->wID=i;
	msgbuf.Head.byRsv[0]=msgbuf.Head.wThID;
    TaskSendMsg(g_Sys.Eqp.pInfo[wToEqpID].wTaskID,DB_ID,wToEqpID,msgbuf.Head.byMsgID,msgbuf.Head.byMsgAttr,sizeof(struct VDBYK),&msgbuf);
  }
} 

/*------------------------------------------------------------------------
 Procedure:     VEYKReq ID:1
 Purpose:       虚装置YK请求
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void VEYKReq(struct VVirtualEqp *pVEqp)
{
  int i;
  struct VVirtualCtrl  *pYK;
  struct VDBYK *pOrigDBYK,DBBO;
  WORD wToEqpID;
  struct VDBCO* pDBCO;

  pOrigDBYK=(struct VDBYK *)msgbuf.abyData;

  if (pOrigDBYK->byValue&0x04)  //开关号
  {
    for (i=0;i<pVEqp->Cfg.wYKNum;i++)
      if (pVEqp->pYK[i].wID==pOrigDBYK->wID)
  	    break;
  	  
    if (i==pVEqp->Cfg.wYKNum)  
    {
      sendYKRet(CONTROLIDINVAILD);
      return;
    }  
  }  
  else
  {
    if (pOrigDBYK->wID>pVEqp->Cfg.wYKNum)
    {
      sendYKRet(CONTROLIDINVAILD);
      return;
    }  
    i=pOrigDBYK->wID;
  }

  pDBCO = pVEqp->pCO.pPoolHead + pVEqp->pCO.wWritePtr;
  pDBCO->cmd = msgbuf.Head.byMsgID;
  pDBCO->wNo = pOrigDBYK->wID + 0x6001 -1;
  GetSysClock(&pDBCO->Time,CALCLOCK);
	
  if((pOrigDBYK->byValue&0x3) == YK_CLOSE) pDBCO->val = 1;
  else if ((pOrigDBYK->byValue&0x3) == YK_OPEN ) pDBCO->val = 0;
  pVEqp->pCO.wWritePtr++;
  pVEqp->pCO.wWritePtr %= pVEqp->pCO.wPoolNum;

#ifdef INCLUDE_HIS
	his2file_write((BYTE*)pDBCO,msgbuf.Head.wEqpID,SYSEV_FLAG_CO);
#endif			
#ifdef INCLUDE_HIS_TTU
    histtu2file_write((BYTE*)pDBCO,msgbuf.Head.wEqpID,SYSEV_FLAG_CO);
#endif			

  pYK=pVEqp->pYK+i;
  wToEqpID=pYK->wTEID;
  DBBO.wID=pYK->wOffset;
  DBBO.byValue=pOrigDBYK->byValue&0xFB;

  TEYKReq(wToEqpID,&DBBO);
} 


/*------------------------------------------------------------------------
 Procedure:     YKReq ID:1
 Purpose:       YK请求
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void YKReq(void)
{
  WORD wEqpID;
  struct VVirtualEqp *pVEqp;
  struct VDBYK *pDBYK;

  wEqpID=msgbuf.Head.wEqpID;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  
  {
    sendYKRet(CONTROLIDINVAILD);
    return;
  }  
  
  pDBYK=(struct VDBYK *)msgbuf.abyData;
  
  if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
  {
    TEYKReq(wEqpID,pDBYK);
  }  
  else  if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)!=NULL)
  {
    if (pVEqp->pYK==NULL)
    {
      sendYKRet(CONTROLIDINVAILD);
      return;
    }  
    VEYKReq(pVEqp);  
  }  
}

/*------------------------------------------------------------------------
 Procedure:     YKRet ID:1
 Purpose:       YK应答
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void YKRet(void)
{
  int i, value;  
  WORD wEqpID;
  struct VTrueEqp *pTEqp;
  struct VDBYK *pDBYK;
  struct VTrueCtrlCfg  *pCfg;  
  char msg[SYS_LOG_MSGLEN], name[GEN_NAME_LEN];

 
  
  wEqpID=msgbuf.Head.wEqpID;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  
  {
    sendYKRet(CONTROLIDINVAILD);
    return;
  }  

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    sendYKRet(CONTROLIDINVAILD);
    return;
  }  

  if ((pTEqp->pYK==NULL)||(pTEqp->pYKCfg==NULL))
  {
    sendYKRet(CONTROLIDINVAILD);
    return;
  }  
 
  pDBYK=(struct VDBYK *)msgbuf.abyData;
  
  if (pDBYK->byValue&0x04)  //开关号
  {
    for (i=0;i<pTEqp->Cfg.wYKNum;i++)
  	  if (pTEqp->pYK[i].wID==pDBYK->wID)
  	    break;
  	  
    if (i==pTEqp->Cfg.wYKNum)  
    {
      sendYKRet(CONTROLIDINVAILD);
      return;
    }  
  }  
  else
  {
    if (pDBYK->wID>=pTEqp->Cfg.wYKNum)
    {
      sendYKRet(CONTROLIDINVAILD);
      return;
    }  
    i=pDBYK->wID;
  }  


  if ((msgbuf.Head.byMsgID==MI_YKOPRATE) && (wEqpID==g_Sys.wIOEqpID))
  {
	  if ((pDBYK->byValue&0x3) == YK_CLOSE)
	  	value = 1;
	  else if ((pDBYK->byValue&0x3) == YK_OPEN ) 
	  	value = 0;
	  else
        value = -1;

	  if (value >= 0)
	  {
		  CommNo2MaintNo(msgbuf.Head.byRsv[0], NULL, name, NULL);
		  
		  sprintf(msg, "来自%s:遥控%d执行", name, pDBYK->wID);
		  if (value == 1)
			  strcat(msg, "合");
		  else				  
			  strcat(msg, "分");
		  if (pDBYK->byStatus == 0)
			  strcat(msg, "成功");
		  else
			  strcat(msg, "失败");			  
     	  WriteDoEvent(NULL, pDBYK->wID, "%s", msg);			
	  }  

  }
  
  pCfg=pTEqp->pYKCfg+i;

  pDBYK->wID=pCfg->wID;
  pDBYK->byValue=(BYTE)pCfg->dwValue;

  TaskSendMsg(pCfg->byTaskID,DB_ID,pCfg->wEqpID,msgbuf.Head.byMsgID,msgbuf.Head.byMsgAttr,sizeof(struct VDBYK),&msgbuf);  
}

/*------------------------------------------------------------------------
 Procedure:     procYKMsg ID:1
 Purpose:       处理YK信息
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void procYKMsg(void)
{
  switch (msgbuf.Head.byMsgAttr)
  {
    case MA_REQ:
      YKReq();
      break;
    case MA_RET:
      YKRet();
      break;
  }      
}

/***************************  YT Info  *********************************/

/*------------------------------------------------------------------------
 Procedure: 	sendYTRet ID:1
 Purpose:		YT 返回
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void sendYTRet(BYTE byStatus)
{
  struct VDBYT *pDBYT;

  pDBYT=(struct VDBYT *)msgbuf.abyData;
  pDBYT->byStatus=byStatus;

  TaskSendMsg(msgbuf.Head.wThID,DB_ID,msgbuf.Head.wEqpID,msgbuf.Head.byMsgID,MA_RET,sizeof(struct VDBYT),&msgbuf);  
}


/*------------------------------------------------------------------------
 Procedure: 	TEYTReq ID:1
 Purpose:		实装置YT请求
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void TEYTReq(WORD wToEqpID,struct VDBYT *pDBYT, BOOL bByID)
{
  int i;
  struct VTrueEqp *pTEqp;
  struct VTrueCtrlCfg	*pCfg;
  struct VDBYT *pOrigDBYT;
	
  pTEqp=g_Sys.Eqp.pInfo[wToEqpID].pTEqpInfo;

  if (pTEqp==NULL)
  {
	sendYTRet(CONTROLIDINVAILD);
	return;
  }  
	
  if ((pTEqp->pYT==NULL)||(pTEqp->pYTCfg==NULL))
  {
	sendYTRet(CONTROLIDINVAILD);
	return;
  }  

  if (bByID==TRUE)	//开关号
  {
	for (i=0;i<pTEqp->Cfg.wYTNum;i++)
	  if (pTEqp->pYT[i].wID==pDBYT->wID)
		break;
	  
	if (i==pTEqp->Cfg.wYTNum)  
	{
	  sendYTRet(CONTROLIDINVAILD);
	  return;
	}  
  }  
  else
  {
	if (pDBYT->wID>pTEqp->Cfg.wYTNum)
	{
	  sendYTRet(CONTROLIDINVAILD);
	  return;
	}  
	i=pDBYT->wID;
  }  
  
  pOrigDBYT=(struct VDBYT *)msgbuf.abyData;
  pCfg=pTEqp->pYTCfg+i;

  //(pCfg->byFlag&0x80)?? onop
  if ((!(pCfg->byFlag&0x01))||(pCfg->byFlag&0x02)||(pCfg->byFlag&0x04))
  {
	sendYTRet(CONTROLFAILED);
	return;
  }  
  else
  {
	pCfg->byFlag|=0x80;
	pCfg->dwValue=pOrigDBYT->dwValue;
	pCfg->byTaskID=msgbuf.Head.wThID;
	pCfg->wEqpID=msgbuf.Head.wEqpID;
	pCfg->wID=pOrigDBYT->wID;
	//GetSysClock(&pCfg->Time,CALCLOCK);

    pOrigDBYT->wID=pTEqp->pYT[i].wID;
	TaskSendMsg(g_Sys.Eqp.pInfo[wToEqpID].wTaskID,DB_ID,wToEqpID,msgbuf.Head.byMsgID,msgbuf.Head.byMsgAttr,sizeof(struct VDBYT),&msgbuf);
  }
} 

/*------------------------------------------------------------------------
 Procedure: 	VEYTReq ID:1
 Purpose:		虚装置YT请求
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void VEYTReq(struct VVirtualEqp *pVEqp)
{
  int i;
  struct VVirtualCtrl	*pYT;
  struct VDBYT *pOrigDBYT,DBAO;
  WORD wToEqpID;

  pOrigDBYT=(struct VDBYT *)msgbuf.abyData;

  for (i=0;i<pVEqp->Cfg.wYTNum;i++)
	if (pVEqp->pYT[i].wID==pOrigDBYT->wID)
	  break;
	  
  if (i==pVEqp->Cfg.wYTNum)  
  {
	sendYTRet(CONTROLIDINVAILD);
	return;
  }  

  pYT=pVEqp->pYT+i;
  wToEqpID=pYT->wTEID;
  DBAO.wID=pYT->wOffset;
  DBAO.dwValue=pOrigDBYT->dwValue;

  TEYTReq(wToEqpID,&DBAO,FALSE);
} 


/*------------------------------------------------------------------------
 Procedure: 	YTReq ID:1
 Purpose:		YT请求
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void YTReq(void)
{
  WORD wEqpID;
  struct VVirtualEqp *pVEqp;
  struct VDBYT *pDBYT;

  wEqpID=msgbuf.Head.wEqpID;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  
  {
	sendYTRet(CONTROLIDINVAILD);
	return;
  }  
  
  pDBYT=(struct VDBYT *)msgbuf.abyData;
  
  if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
  {
	TEYTReq(wEqpID,pDBYT,TRUE);
  }  
  else	if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)!=NULL)
  {
	if (pVEqp->pYT==NULL)
	{
	  sendYTRet(CONTROLIDINVAILD);
	  return;
	}  
	VEYTReq(pVEqp);  
  }  
}

/*------------------------------------------------------------------------
 Procedure: 	YTRet ID:1
 Purpose:		YT应答
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void YTRet(void)
{
  int i;  
  WORD wEqpID;
  struct VTrueEqp *pTEqp;
  struct VDBYT *pDBYT;
  struct VTrueCtrlCfg	*pCfg;
  
  wEqpID=msgbuf.Head.wEqpID;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  
  {
	sendYTRet(CONTROLIDINVAILD);
	return;
  }  

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
	sendYTRet(CONTROLIDINVAILD);
	return;
  }  

  if ((pTEqp->pYT==NULL)||(pTEqp->pYTCfg==NULL))
  {
	sendYTRet(CONTROLIDINVAILD);
	return;
  }  

  pDBYT=(struct VDBYT *)msgbuf.abyData;
  
  for (i=0;i<pTEqp->Cfg.wYTNum;i++)
	if (pTEqp->pYT[i].wID==pDBYT->wID)
	  break;
	  
  if (i==pTEqp->Cfg.wYTNum)  
  {
	sendYTRet(CONTROLIDINVAILD);
	return;
  }  
  
  pCfg=pTEqp->pYTCfg+i;

  pDBYT->wID=pCfg->wID;
  pDBYT->dwValue=pCfg->dwValue;

  TaskSendMsg(pCfg->byTaskID,DB_ID,pCfg->wEqpID,msgbuf.Head.byMsgID,msgbuf.Head.byMsgAttr,sizeof(struct VDBYT),&msgbuf);  
}

/*------------------------------------------------------------------------
 Procedure: 	procYTMsg ID:1
 Purpose:		处理YT信息
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void procYTMsg(void)
{
  switch (msgbuf.Head.byMsgAttr)
  {
	case MA_REQ:
	  YTReq();
	  break;
	case MA_RET:
	  YTRet();
	  break;
  } 	 
}

/*------------------------------------------------------------------------
 Procedure:     procMsg ID:1
 Purpose:       处理信息
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void procMsg(void)
{
  BYTE msgId;

  msgId=msgbuf.Head.byMsgID;

  switch (msgId)
  {
    case MI_YKSELECT:
    case MI_YKOPRATE:
    case MI_YKCANCEL:
	case MI_YKSIM:
	case MI_YKUNSIM:
      procYKMsg();
      break;
    case MI_YTSELECT:
	case MI_YTOPRATE:
	case MI_YTCANCEL:
	  procYTMsg();
	  break;
    default:
      break;
  }    
}


/*------------------------------------------------------------------------
 Procedure:     database ID:1
 Purpose:       数据库初始化及控制信息处理
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void database(int thid)
{
	int num;
	DWORD evFlag;

	dwDYXSem = smMCreate();
	dwSYXSem = smMCreate();  
	dwYCSem = smMCreate(); 
	dwDDSem = smMCreate();

	for(;;)
	{
		evReceive(thid, EV_MSG, &evFlag);

		if (evFlag&EV_MSG)
		{
			for (;;)
			{
				num=msgReceive(thid, (BYTE *)&msgbuf, MAXMSGSIZE, OS_NOWAIT);	
				if (num>0)
				{
					procMsg();
				}
				else
					break;
			}    
		}
	}  
}

