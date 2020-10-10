/*------------------------------------------------------------------------
 Module:       	dbapi.cpp
 Author:        solar
 Project:       
 State:         
 Creation Date:	2008-09-01
 Description:   
------------------------------------------------------------------------*/
#include "syscfg.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h>
#include "sys.h"

extern DWORD dwDYXSem;
extern DWORD dwSYXSem;
extern DWORD dwYCSem;
extern DWORD dwDDSem;

void YCLimitCheck(WORD wEqpID, struct VTrueEqp *pTEqp, struct VTrueYC *pYC, struct VTrueYCCfg *pCfg);
extern void writeFlag(WORD wEqpID,DWORD dwFlag);
extern STATUS GetReadPtr(WORD wTaskID,WORD wEqpID,DWORD dwUFlag,WORD **ppwReadPtr);
extern void GetValueFlag(DWORD dwCfg, BYTE *pFlag);
/*STATUS extNvRamGet(DWORD offset, BYTE *buf, int len)
{
	int fd;
	char path[4*MAXFILENAME];

	memset(path,0,4*MAXFILENAME);
	sprintf(path,"/mnt/emmc_data/nvram.ram");

	memset(buf,0,(DWORD)len);
	
	fd = open(path, O_RDONLY);

	if (fd < 0) return ERROR;

	lseek(fd,(long)offset,SEEK_SET);
	
	len = read(fd, (char*)buf, (DWORD)len);

	close(fd);

	if (len < 0) return ERROR;

	return OK;
}

STATUS extNvRamSet(DWORD offset, BYTE *buf, int len)
{
    int fd;
	char path[4*MAXFILENAME];
	
	memset(path,0,4*MAXFILENAME);
	sprintf(path,"/mnt/emmc_data/nvram.ram");

	fd = open(path, O_WRONLY | O_CREAT);

	if (fd < 0) 
	{
		DPRINT("WriteParaFile %s err \n",path);
		return ERROR;
	}
	
	lseek(fd,(long)offset,SEEK_SET);
	
	len = write(fd, (char*)buf, (DWORD)len);

	fsync(fd);
	
	close(fd);

	if (len < 0) return ERROR;

	return OK;
}*/

#ifdef INCLUDE_B2F_F
extern void WriteRunInfo2File(WORD wTaskID,  struct VEqpRunInfo *pInfo, DWORD dwUFlag, int flag);
#endif
#ifdef INCLUDE_B2F
extern void WritePollBuf2File(struct VPool *pPool, BYTE *pBuf, WORD wID, int flag);
#endif

extern void atomicWriteDCOS(WORD wEqpID,WORD wNum,struct VDBDCOS *buf,BOOL semflag,BOOL bLock); 
extern void atomicWriteDSOE(WORD wEqpID,WORD wNum,struct VDBDSOE *buf,BOOL semflag,BOOL bLock); 
extern WORD atomicReadDYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDYX *buf,BOOL asSend);
extern WORD atomicReadVEDCOS(WORD wTaskID,WORD wEqpID,struct VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBDCOS *buf);
extern WORD atomicReadTEDCOS(WORD wTaskID,WORD wEqpID,struct VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBDCOS *buf);
extern WORD atomicReadVEDSOE(WORD wTaskID,WORD wEqpID,struct VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBDSOE *buf);
extern WORD atomicReadTEDSOE(WORD wTaskID,WORD wEqpID,struct VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBDSOE *buf);
extern WORD atomicQueryVEDCOS(struct VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBDCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
extern WORD atomicQueryTEDCOS(struct VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBDCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
extern WORD atomicQueryVEDSOE(struct VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBDSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
extern WORD atomicQueryTEDSOE(struct VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBDSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen);

extern void atomicWriteSYX(WORD wEqpID,WORD wNum, struct VDBYX *buf,BOOL isStatusNo,BOOL bLock,BOOL bCCos, BOOL bCSoe);
extern void atomicWriteSCOS(WORD wEqpID,WORD wNum,struct VDBCOS *buf,BOOL semflag,BOOL isStatusNo,BOOL bLock);
extern void atomicWriteSSOE(WORD wEqpID,WORD wNum,struct VDBSOE *buf,BOOL semflag,BOOL isStatusNo,BOOL bLock); 
extern WORD atomicReadSYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYX *buf,BOOL asSend);
extern WORD atomicReadVESCOS(WORD wTaskID,WORD wEqpID,struct VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBCOS *buf);
extern WORD atomicReadTESCOS(WORD wTaskID,WORD wEqpID,struct VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBCOS *buf);
extern WORD atomicReadVESSOE(WORD wTaskID,WORD wEqpID,struct VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBSOE *buf);
extern WORD atomicReadTESSOE(WORD wTaskID,WORD wEqpID,struct VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBSOE *buf);
extern WORD atomicQueryVESCOS(struct VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
extern WORD atomicQueryTESCOS(struct VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
extern WORD atomicQueryVESSOE(struct VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
extern WORD atomicQueryTESSOE(struct VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen);

#if 0
extern void atomicWriteEVCOS(WORD wEqpID,WORD wNum,VDBCOS *buf,BOOL semflag,BOOL bLock); 
extern void atomicWriteEVSOE(WORD wEqpID,WORD wNum,VDBSOE *buf,BOOL semflag,BOOL bLock);
extern WORD atomicReadEVYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYX *buf,BOOL asSend);
extern WORD atomicReadVEEVCOS(WORD wTaskID,WORD wEqpID,VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBCOS *buf);
extern WORD atomicReadTEEVCOS(WORD wTaskID,WORD wEqpID,VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBCOS *buf);
extern WORD atomicReadVEEVSOE(WORD wTaskID,WORD wEqpID,VVirtualEqp *pVEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBSOE *buf);
extern WORD atomicReadTEEVSOE(WORD wTaskID,WORD wEqpID,VTrueEqp *pTEqp,WORD wNum,WORD *pwReadPtr ,WORD wBufLen,struct VDBSOE *buf);
extern WORD atomicQueryVEEVCOS(VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
extern WORD atomicQueryTEEVCOS(VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
extern WORD atomicQueryVEEVSOE(VVirtualEqp *pVEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
extern WORD atomicQueryTEEVSOE(VTrueEqp *pTEqp,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
#endif

extern void atomicWriteYCF_F(WORD wEqpID,WORD wNum, struct VDBYCF_F *buf, BOOL bLock);
extern void atomicWriteYC_F(WORD wEqpID,WORD wNum, struct VDBYC_F *buf, BOOL bLock);
extern void atomicWriteYCF_L(WORD wEqpID,WORD wNum, struct VDBYCF_L *buf, BOOL bLock);
extern void atomicWriteYC_L(WORD wEqpID,WORD wNum, struct VDBYC_L *buf, BOOL bLock);
extern WORD atomicReadYCF_L(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYCF_L *buf,BOOL asSend);
extern WORD atomicReadYC_L(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYC_L *buf,BOOL asSend);
extern void atomicWriteYCF(WORD wEqpID,WORD wNum, struct VDBYCF *buf, BOOL bLock);
extern void atomicWriteYC(WORD wEqpID,WORD wNum, struct VDBYC *buf, BOOL bLock);
extern STATUS atomicGetVEYC_ABC(struct VVirtualEqp *pVEqp, WORD wNo, long *A, long *B, long *C);
extern STATUS atomicGetTEYC_ABC(struct VTrueEqp *pTEqp, WORD wNo, long *A, long *B, long *C);
extern WORD atomicReadYCF(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYCF *buf,BOOL asSend);
extern WORD atomicReadYC(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYC *buf,BOOL asSend);

extern WORD atomicReadDDF(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDDF *buf,BOOL asSend);
extern WORD atomicReadDDFT(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDDFT *buf,BOOL asSend);
extern WORD atomicReadDD(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDD *buf,BOOL asSend);
#ifdef DB_EPOWER_STAT
extern void MaxXL_Stat(WORD wEqpID, VDBDDFT *pDBDDT);
#endif

extern WORD atomicReadTEYK(struct VTrueEqp *pTEqp, WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf);
extern WORD atomicReadVEYK(struct VVirtualEqp *pVEqp, WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf);

extern WORD atomicReadTEYT(struct VTrueEqp *pTEqp, WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf);
extern WORD atomicReadVEYT(struct VVirtualEqp *pVEqp, WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf);

extern struct VEqpRunInfo *GetpRunInfo(WORD wTaskID,WORD wEqpID, int *flag);
extern STATUS ReadTrueEqpInfo(WORD wTaskID,WORD wEqpID,struct VTrueEqp *pTEqp,struct VTVEqpInfo * pTrueEqpInfo,BOOL asSend);
extern STATUS ReadVirtualEqpInfo(WORD wTaskID,WORD wEqpID,struct VVirtualEqp *pVEqp,struct VTVEqpInfo * pVirtualEqpInfo);
extern STATUS ReadEqpGroupInfo(WORD wTaskID,WORD wEqpID,struct VEqpGroup *pGEqp,struct VEqpGroupInfo * pEqpGroupInfo);


/********************************YX api**********************************/

/******************************** DYX **********************************/

/********************************write **********************************/

/*------------------------------------------------------------------------
 Procedure:     WriteDYX ID:1
 Purpose:       写离散点双点遥信(一字节代表一遥信) 
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteDYX(WORD wEqpID,WORD wNum, struct VDBDYX *buf, BOOL bLock)
{
  struct VTrueEqp *pTEqp;
  struct VDBDYX DYX,*p=buf;
  struct VTrueDYX *pDYX;
  struct VTrueYXCfg *pCfg;
  struct VDBDSOE DBDSOE;  
  int i,maxno;
  DWORD dwCfg;
  BOOL change;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pTEqp->pDYX==NULL)||(pTEqp->pDYXCfg==NULL))  return;

  maxno=pTEqp->Cfg.wDYXNum-1;
  if (maxno<0)  return;

  smMTake(dwDYXSem);  	  
  
  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno)||((p->byValue1&0x01)==0)||((p->byValue2&0x01)==0))
    {
      p++;
      continue;
    } 

    DYX.wNo=p->wNo;
    DYX.byValue1=p->byValue1;
    DYX.byValue2=p->byValue2;    
    if (((DYX.byValue1&0x01)==0)||((DYX.byValue2&0x01)==0))  //当前值
    {
      p++;
	  continue;
    }  

    pCfg=pTEqp->pDYXCfg+p->wNo;
    pDYX=pTEqp->pDYX+p->wNo;   
    dwCfg=pCfg->dwCfg;  

	if (bLock&&(dwCfg&0x80000000))  //正常写入，考虑闭锁
	{
	  p++;
	  continue;
	}

    dwCfg|=0x08;  //数据库产生COS

    if (dwCfg&0x02)  //取反
    {
      DYX.byValue1^=0x80;
      DYX.byValue2^=0x80;
    }  

    if ((dwCfg&0x08)||(dwCfg&0x04))  //write cos or soe
    {
      if ((pDYX->byValue1!=DYX.byValue1)||(pDYX->byValue2!=DYX.byValue2))
        change=TRUE;
      else
        change=FALSE;	
     
      if (change)
      {
        if (dwCfg&0x08)  //write cos
      	  atomicWriteDCOS(wEqpID,1,(struct VDBDCOS *)p,FALSE,TRUE); //sendno????
        if (dwCfg&0x04)  //write soe
        {  
          DBDSOE.wNo=p->wNo;
          DBDSOE.byValue1=p->byValue1;
          DBDSOE.byValue2=p->byValue2;
          GetSysClock(&DBDSOE.Time,CALCLOCK);
          atomicWriteDSOE(wEqpID,1,&DBDSOE,FALSE,TRUE);
        }
      } 
  	} 

    pDYX->byValue1=DYX.byValue1;
    pDYX->byValue2=DYX.byValue2;
    p++;   
  }
   
  smMGive(dwDYXSem);
}  


/*------------------------------------------------------------------------
 Procedure:     WriteRangeDYX ID:1
 Purpose:       写连续点双点遥信(一字节代表一遥信)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeDYX(WORD wEqpID,WORD wBeginNo,WORD wNum, struct VDYX *buf)
{
  struct VTrueEqp *pTEqp;
  struct VDYX * p=buf;
  struct VDBDYX DBDYX;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pDYX==NULL)||(pTEqp->pDYXCfg==NULL)) return;

  maxno=pTEqp->Cfg.wDYXNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBDYX.wNo=i+wBeginNo;
    if ((DBDYX.wNo>maxno)||((p->byValue1&0x01)==0)||((p->byValue2&0x01)==0))
    {
       p++;
       continue;
    } 
    DBDYX.byValue1=p->byValue1;
    DBDYX.byValue2=p->byValue2;
    WriteDYX(wEqpID,1,&DBDYX,TRUE);         
    p++;
  }  
}  

/*------------------------------------------------------------------------
 Procedure:     WriteDCOS ID:1
 Purpose:       写双点COS
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteDCOS(WORD wEqpID,WORD wNum,struct VDBDCOS *buf) 
{
  atomicWriteDCOS(wEqpID,wNum,buf,TRUE,TRUE) ;
}  

/*------------------------------------------------------------------------
 Procedure:     WriteDSOE ID:1
 Purpose:       写双点SOE
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteDSOE(WORD wEqpID,WORD wNum,struct VDBDSOE *buf) 
{
  atomicWriteDSOE(wEqpID,wNum,buf,TRUE,TRUE) ;
}  

/******************************** read **********************************/


/*------------------------------------------------------------------------
 Procedure:     ReadDYX ID:1
 Purpose:       读离散点双点遥信(一字节代表一遥信) 
 Input:         wNum: 要读的个数
 Output:        实际所读的个数   	
 Errors:
------------------------------------------------------------------------*/
WORD ReadDYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDYX *buf)
{
  if (wBufLen<sizeof(struct VDBDYX)) return(0);
  
  return(atomicReadDYX(wEqpID,wNum,wBufLen,buf,TRUE));
}


/*------------------------------------------------------------------------
 Procedure:     ReadRangeDYX ID:1
 Purpose:       读连续点双点遥信(一字节代表一遥信)  
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeDYX(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VDYX *buf)
{
  int i,j,k;
  struct VDBDYX DBDYX;
  struct VDYX *p=buf;

  if (wBufLen<sizeof(struct VDYX)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBDYX.wNo=i;
    if (atomicReadDYX(wEqpID,1,sizeof(struct VDBDYX),&DBDYX,TRUE)==1)
    {
      p->byValue1=DBDYX.byValue1;
      p->byValue2=DBDYX.byValue2;
      p++;
      j++;
      k+=sizeof(struct VDYX);
      if (k>wBufLen-sizeof(struct VDYX)) break;
    } 
    else
      break;	
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  ReadDCOS ID:1
 Purpose:    读双点COS 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD ReadDCOS(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VDBDCOS *buf)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j,*pwReadPtr;

  if (wBufLen<sizeof(struct VDBDCOS)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if (GetReadPtr(wTaskID,wEqpID,DCOSUFLAG,&pwReadPtr)==ERROR)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->DCOS.pPoolHead==NULL)||(pVEqp->Cfg.wDYXNum==0))
        return(0);
      else
        j=atomicReadVEDCOS(wTaskID,wEqpID,pVEqp,wNum,pwReadPtr,wBufLen,buf);
    }   
  } 
  else
  {
    if  ((pTEqp->DCOS.pPoolHead==NULL)||(pTEqp->DYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicReadTEDCOS(wTaskID,wEqpID,pTEqp,wNum,pwReadPtr,wBufLen,buf);
  }  

  return(j);
  
}


/*------------------------------------------------------------------------
 Procedure:  ReadDSOE ID:1
 Purpose:    读双点SOE 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD ReadDSOE(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VDBDSOE *buf)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j,*pwReadPtr;

  if (wBufLen<sizeof(struct VDBDSOE)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if (GetReadPtr(wTaskID,wEqpID,DSOEUFLAG,&pwReadPtr)==ERROR)  return(0);
  
  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->DSOE.pPoolHead==NULL)||(pVEqp->Cfg.wDYXNum==0))
        return(0);
      else
        j=atomicReadVEDSOE(wTaskID,wEqpID,pVEqp,wNum,pwReadPtr,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->DSOE.pPoolHead==NULL)||(pTEqp->DYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicReadTEDSOE(wTaskID,wEqpID,pTEqp,wNum,pwReadPtr,wBufLen,buf);
  }  

  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  ReadRangeAllDYX ID:1
 Purpose:    读连续点所有双点遥信(一字节代表一遥信)（不考虑其是否发送，维护用） 
 Input:      wNum: 要读的个数
             wBeginNo:起始点号
 Output:     实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllDYX(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VDYX *buf)
{
  int i,j,k;
  struct VDBDYX DBDYX;
  struct VDYX *p=buf;
   
  if (wBufLen<sizeof(struct VDYX)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBDYX.wNo=i;
    if (atomicReadDYX(wEqpID,1,sizeof(struct VDBDYX),&DBDYX,FALSE)==1)
    {
     p->byValue1=DBDYX.byValue1;
     p->byValue2=DBDYX.byValue2;
     p++;
     j++;
     k+=sizeof(struct VDYX);
     if (k>wBufLen-sizeof(struct VDYX)) break;
    } 
    else
      break;	
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  QueryDCOSWrtOffset ID:1
 Purpose:    查询双点COS的写指针偏移  (维护用)
 Input:      pwWriteOffset:写指针偏移(相对于pwPoolLen)
 Output:     		
 Errors:
------------------------------------------------------------------------*/
STATUS QueryDCOSWrtOffset(WORD wEqpID,WORD correctOffset,WORD *pwWriteOffset)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if ((pVEqp->DCOS.pPoolHead==NULL)||(pVEqp->Cfg.wDYXNum==0))
        return(ERROR);
      else
      {
        if (pVEqp->DCOS.wWritePtr<correctOffset)
          //*pwWriteOffset=pVEqp->DCOS.wPoolNum-correctOffset+pVEqp->DCOS.wWritePtr;
          *pwWriteOffset=0;
        else
          *pwWriteOffset=pVEqp->DCOS.wWritePtr-correctOffset;        
        return(OK);
      }  
    }   
  } 
  else
  {
    if ((pTEqp->DCOS.pPoolHead==NULL)||(pTEqp->DYXSendCfg.wNum==0))
      return(ERROR);
    else
    {
      if (pTEqp->DCOS.wWritePtr<correctOffset)
        //*pwWriteOffset=pTEqp->DCOS.wPoolNum-correctOffset+pTEqp->DCOS.wWritePtr;
        *pwWriteOffset=0;
      else
        *pwWriteOffset=pTEqp->DCOS.wWritePtr-correctOffset;        
      return(OK);
    }  
  }  
}


/*------------------------------------------------------------------------
 Procedure:  QueryDCOS ID:1
 Purpose:    查询双点COS  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD QueryDCOS(WORD wEqpID,WORD wNum,WORD wReadOffset ,WORD wBufLen,struct VDBDCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBDCOS)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->DCOS.pPoolHead==NULL)||(pVEqp->Cfg.wDYXNum==0))
        return(0);
      else
        j=atomicQueryVEDCOS(pVEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
    }   
  } 
  else
  {
    if ((pTEqp->DCOS.pPoolHead==NULL)||(pTEqp->DYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicQueryTEDCOS(pTEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
  }  

  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  QueryDSOEWrtOffset ID:1
 Purpose:    查询双点SOE的写指针偏移  (维护用)
 Input:      pwWriteOffset:写指针偏移(相对于pwPoolLen)
 Output:     		
 Errors:
------------------------------------------------------------------------*/
STATUS QueryDSOEWrtOffset(WORD wEqpID,WORD correctOffset,WORD *pwWriteOffset)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if ((pVEqp->DSOE.pPoolHead==NULL)||(pVEqp->Cfg.wDYXNum==0))
        return(ERROR);
      else
      {
        if (pVEqp->DSOE.wWritePtr<correctOffset)
          //*pwWriteOffset=pVEqp->DSOE.wPoolNum-correctOffset+pVEqp->DSOE.wWritePtr;
          *pwWriteOffset=0;
        else
          *pwWriteOffset=pVEqp->DSOE.wWritePtr-correctOffset;        
        return(OK);
      }  
    }   
  } 
  else
  {
    if ((pTEqp->DSOE.pPoolHead==NULL)||(pTEqp->DYXSendCfg.wNum==0))
      return(ERROR);
    else
    {
      if (pTEqp->DSOE.wWritePtr<correctOffset)
        //*pwWriteOffset=pTEqp->DSOE.wPoolNum-correctOffset+pTEqp->DSOE.wWritePtr;
        *pwWriteOffset=0;
      else
        *pwWriteOffset=pTEqp->DSOE.wWritePtr-correctOffset;        
      return(OK);
    }  
  }  
}


/*------------------------------------------------------------------------
 Procedure:  QueryDSOE ID:1
 Purpose:    查询双点SOE  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD QueryDSOE(WORD wEqpID,WORD wNum,WORD wReadOffset ,WORD wBufLen,struct VDBDSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBDSOE)) return(0);

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->DSOE.pPoolHead==NULL)||(pVEqp->Cfg.wDYXNum==0))
        return(0);
      else
        j=atomicQueryVEDSOE(pVEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
    }   
  } 
  else
  {
    if ((pTEqp->DSOE.pPoolHead==NULL)||(pTEqp->DYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicQueryTEDSOE(pTEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
  }  

  return(j);  
}

/******************************** SYX **********************************/


/********************************write **********************************/
void SetRangeSYXCfg(WORD wEqpID,WORD wBeginNo,int nNum,DWORD dwCfg)
{
	struct VTrueEqp *pTEqp;
	struct VTrueYXCfg *pCfg;
	int i,maxno;
	DWORD dwOldCfg;
	struct VDBSOE DBSOE;
	struct VDBCOS DBCOS;
	
	if (wEqpID>=*g_Sys.Eqp.pwNum)  return;
	
	if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;
	
	//if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL))  return;
	
	maxno=pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum-1;
	if (maxno<0)  return;

    if (nNum == -1)
		nNum = pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum;
	else if (nNum == -2)
		nNum = pTEqp->Cfg.wSYXNum;		
	
	smMTake(dwSYXSem);		
	
	for (i=wBeginNo;i<(wBeginNo+nNum);i++)
	{
	  if (i>maxno)  break;
	  if (i== pTEqp->Cfg.wSYXNum-1) continue;  //commstate

	  pCfg=pTEqp->pSYXCfg+i;
	   dwOldCfg = pCfg->dwCfg;
      pCfg->dwCfg |= dwCfg;

	  if(dwOldCfg != pCfg->dwCfg)
	  {
	     DBCOS.wNo = i;
		 DBCOS.byValue = pTEqp->pSYX[i].byValue;
		 GetValueFlag(pCfg->dwCfg, &(DBCOS.byValue));
		 atomicWriteSCOS(wEqpID, 1, &DBCOS, FALSE, TRUE, TRUE);

		 DBSOE.wNo = i;
		 DBSOE.byValue = DBCOS.byValue;
		 GetSysClock(&(DBSOE.Time), CALCLOCK);
		 atomicWriteSSOE(wEqpID, 1, &DBSOE, FALSE, TRUE, TRUE);
	  }

	}
	 
	smMGive(dwSYXSem);
}  

void ClearRangeSYXCfg(WORD wEqpID,WORD wBeginNo,int nNum,DWORD dwCfg)
{
	struct VTrueEqp *pTEqp;
	struct VTrueYXCfg *pCfg;
	int i,maxno;
/*	DWORD dwOldCfg;
	struct VDBSOE DBSOE;
	struct VDBCOS DBCOS;*/
	
	
	if (wEqpID>=*g_Sys.Eqp.pwNum)  return;
	
	if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;
	
	//if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL))  return;
	
	maxno=pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum-1;
	if (maxno<0)  return;

    if (nNum == -1)
		nNum = pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum;
	else if (nNum == -2)
		nNum = pTEqp->Cfg.wSYXNum;		
	
	smMTake(dwSYXSem);		
	
	for (i=wBeginNo;i<(wBeginNo+nNum);i++)
	{
	  if (i>maxno)  break;
	  if (i== pTEqp->Cfg.wSYXNum-1) continue;  //commstate

	  pCfg=pTEqp->pSYXCfg+i;
	//  dwOldCfg = pCfg->dwCfg;
      pCfg->dwCfg &= (~dwCfg);	

	/*  if(dwOldCfg != pCfg->dwCfg)
	  {
	     DBCOS.wNo = i;
		 DBCOS.byValue = pTEqp->pSYX[i].byValue;
		 GetValueFlag(pCfg->dwCfg, &(DBCOS.byValue));
		 atomicWriteSCOS(wEqpID, 1, &DBCOS, FALSE, TRUE, TRUE);

		 DBSOE.wNo = i;
		 DBSOE.byValue = DBCOS.byValue;
		 GetSysClock(&(DBSOE.Time), CALCLOCK);
		 atomicWriteSSOE(wEqpID, 1, &DBSOE, FALSE, TRUE, TRUE);
	  }*/
	}
	 
	smMGive(dwSYXSem);
}  

/*------------------------------------------------------------------------
 Procedure: 	LockSYX ID:1
 Purpose:		闭锁单点遥信
                一字节代表一遥信, 如果字节为0, 表示不取代当前值 
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void LockSYX(WORD wEqpID, WORD wNum, struct VDBYX *buf)
{
	struct VTrueEqp *pTEqp;
	struct VDBYX * p=buf;
	struct VTrueYXCfg *pCfg;
	int i,maxno;
	
	if (wEqpID>=*g_Sys.Eqp.pwNum)  return;
	
	if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;
	
	if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL))  return;
	
	maxno=pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum-1;
	if (maxno<0)  return;
	
	smMTake(dwSYXSem);		
	
	for (i=0;i<wNum;i++)
	{
	  if (p->wNo>maxno)
	  {
		p++;
		continue;
	  } 

	  pCfg=pTEqp->pSYXCfg+p->wNo;
      pCfg->dwCfg|=0x80000000;	
	  
	  p++;	 
	}
	 
	smMGive(dwSYXSem);

	atomicWriteSYX(wEqpID,wNum,buf,TRUE,FALSE,TRUE,TRUE);
}  

/*------------------------------------------------------------------------
 Procedure: 	UnLockSYX ID:1
 Purpose:		解锁单点遥信
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void UnLockSYX(WORD wEqpID, WORD wNum, struct VDBYX *buf)
{
	struct VTrueEqp *pTEqp;
	struct VDBYX * p=buf;
	struct VTrueYXCfg *pCfg;
	int i,maxno;
	
	if (wEqpID>=*g_Sys.Eqp.pwNum)  return;
	
	if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;
	
	if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL))  return;
	
	maxno=pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum-1;
	if (maxno<0)  return;
	
	smMTake(dwSYXSem);		
	
	for (i=0;i<wNum;i++)
	{
	  if (p->wNo>maxno)
	  {
		p++;
		continue;
	  } 

	  pCfg=pTEqp->pSYXCfg+p->wNo;
	  pCfg->dwCfg&=(~0x80000000);	
	  
	  p++;	 
	}
	 
	smMGive(dwSYXSem);
	
	atomicWriteSYX(wEqpID,wNum,buf,TRUE,FALSE,TRUE,TRUE);
	
	evSend(g_Sys.Eqp.pInfo[wEqpID].wTaskID, EV_DATAREFRESH);
}  


/*------------------------------------------------------------------------
 Procedure:     WriteSYX ID:1
 Purpose:       写离散点单点遥信(一字节代表一遥信) 
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteSYX(WORD wEqpID,WORD wNum, struct VDBYX *buf)
{
  atomicWriteSYX(wEqpID,wNum,buf,FALSE,TRUE,FALSE,FALSE);
}  

void ClearSYX(WORD wEqpID)
{
	struct VTrueEqp *pTEqp;
	struct VTECfg *pTECfg;
	int i, statusno;
	
	if (wEqpID>=*g_Sys.Eqp.pwNum)  return;
	
	if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;
	
	if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL))  return;

	pTECfg=&pTEqp->Cfg;
	
	statusno=pTECfg->wSYXNum-1;
	
	smMTake(dwSYXSem);		

	for (i=0;i<(pTECfg->wSYXNum+pTECfg->wVYXNum);i++)
	{
		if (i == statusno) continue;
		pTEqp->pSYX[i].byValue=0x00;
	}	

	smMGive(dwSYXSem);		
}

/*------------------------------------------------------------------------
 Procedure:     WriteRangeSYX ID:1
 Purpose:       写连续点单点遥信(一字节代表一遥信)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeSYX(WORD wEqpID,WORD wBeginNo,WORD wNum, BYTE *buf)
{
  struct VTrueEqp *pTEqp;
  BYTE * p=buf;
  struct VDBYX DBYX;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL)) return;

  maxno=pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBYX.wNo=i+wBeginNo;
    if ((DBYX.wNo>maxno)||((*p&0x01)==0))
    {
       p++;
       continue;
    } 
    DBYX.byValue=*p;
    WriteSYX(wEqpID,1,&DBYX);         
    p++;
  }  
}  


/*------------------------------------------------------------------------
 Procedure:     WriteRangeSYXBit ID:1
 Purpose:       写连续点单点遥信(一位代表一遥信)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeSYXBit(WORD wEqpID,WORD wBeginNo,WORD wNum, BYTE *buf)
{
  struct VTrueEqp *pTEqp;
  BYTE * p=buf;
  struct VDBYX DBYX;
  int i,j,maxno;

  if (wNum==0) return;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pSYX==NULL)||(pTEqp->pSYXCfg==NULL)) return;

  maxno=pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum-1;
  if (maxno<0) return;

  if (maxno>wBeginNo+wNum-1) maxno=wBeginNo+wNum-1;

  for (i=0;i<(wNum-1)/8+1;i++)
  {
    for (j=0;j<8;j++)
    {
      DBYX.wNo=i*8+wBeginNo+j;
      if (DBYX.wNo>maxno)
        return;
      DBYX.byValue=((*p&(0x01<<j))<<(7-j))|0x01;
      WriteSYX(wEqpID,1,&DBYX);         
    }  
    p++;
  }  
}  


/*------------------------------------------------------------------------
 Procedure:     WriteSCOS ID:1
 Purpose:       写单点COS
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteSCOS(WORD wEqpID,WORD wNum,struct VDBCOS *buf) 
{
  atomicWriteSCOS(wEqpID,wNum,buf,TRUE,FALSE,TRUE) ;
}  

/*------------------------------------------------------------------------
 Procedure:     WriteDSOE ID:1
 Purpose:       写单点SOE
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteSSOE(WORD wEqpID,WORD wNum,struct VDBSOE *buf) 
{
  atomicWriteSSOE(wEqpID,wNum,buf,TRUE,FALSE,TRUE) ;
}  

/******************************** read **********************************/


/*------------------------------------------------------------------------
 Procedure:     ReadSYX ID:1
 Purpose:       读离散点单点遥信(一字节代表一遥信) 
 Input:         wNum: 要读的个数
 Output:        实际所读的个数   	
 Errors:
------------------------------------------------------------------------*/
WORD ReadSYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYX *buf)
{
  if (wBufLen<sizeof(struct VDBYX)) return(0);
  
  return(atomicReadSYX(wEqpID,wNum,wBufLen,buf,TRUE));
}


/*------------------------------------------------------------------------
 Procedure:     ReadRangeSYX ID:1
 Purpose:       读连续点单点遥信(一字节代表一遥信)  
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeSYX(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf)
{
  int i,j,k;
  struct VDBYX DBYX;
  BYTE *p=buf;

  if (wBufLen<sizeof(BYTE)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYX.wNo=i;
    if (atomicReadSYX(wEqpID,1,sizeof(struct VDBYX),&DBYX,TRUE)==1)
    {
      *p=DBYX.byValue;
      p++;
      j++;
      k+=sizeof(BYTE);
      if (k>wBufLen-sizeof(BYTE)) break;
    } 
    else
      break;	
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     ReadRangeSYXBit ID:1
 Purpose:       读连续点单点遥信(一位代表一遥信)  
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeSYXBit(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf)
{
  int i,j,k,m;
  struct VDBYX DBYX;
  BYTE *p=buf;
  BYTE value;
 
  if (wBufLen<sizeof(BYTE)) return(0);
  
  j=k=0;
  m=value=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYX.wNo=i;
    if (atomicReadSYX(wEqpID,1,sizeof(struct VDBYX),&DBYX,TRUE)==1)
    {
      value|=(((DBYX.byValue&0x80)>>(7-m)));
      if (++m==8)
      {	
        *p=value;
        value=m=0;
        p++;
        k+=sizeof(BYTE);
        if (k>wBufLen-sizeof(BYTE)) break;        
      }  
      j++;
    } 
    else
   {
	*p=value; 
      break;	
   }			
  }  

  if (j%8)   *p=value;  	

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  ReadSCOS ID:1
 Purpose:    读单点COS 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD ReadSCOS(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VDBCOS *buf)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j,*pwReadPtr;

  if (wBufLen<sizeof(struct VDBCOS)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if (GetReadPtr(wTaskID,wEqpID,SCOSUFLAG,&pwReadPtr)==ERROR)  return(0);
  
  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->SCOS.pPoolHead==NULL)||(pVEqp->Cfg.wSYXNum==0))
        return(0);
      else
        j=atomicReadVESCOS(wTaskID,wEqpID,pVEqp,wNum,pwReadPtr,wBufLen,buf);
    }   
  } 
  else
  {
    if  ((pTEqp->SCOS.pPoolHead==NULL)||(pTEqp->SYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicReadTESCOS(wTaskID,wEqpID,pTEqp,wNum,pwReadPtr,wBufLen,buf);
  }  

  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  ReadSSOE ID:1
 Purpose:    读单点SOE 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD ReadSSOE(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VDBSOE *buf)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j,*pwReadPtr;

  if (wBufLen<sizeof(struct VDBSOE)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if (GetReadPtr(wTaskID,wEqpID,SSOEUFLAG,&pwReadPtr)==ERROR)  return(0);
  
  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->SSOE.pPoolHead==NULL)||(pVEqp->Cfg.wSYXNum==0))
        return(0);
      else
        j=atomicReadVESSOE(wTaskID,wEqpID,pVEqp,wNum,pwReadPtr,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->SSOE.pPoolHead==NULL)||(pTEqp->SYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicReadTESSOE(wTaskID,wEqpID,pTEqp,wNum,pwReadPtr,wBufLen,buf);
  }  

  return(j);  
}

/*------------------------------------------------------------------------
 Procedure:     ReadRangeAllSYX ID:1
 Purpose:       读连续点单点遥信(一字节代表一遥信)（不考虑其是否发送，维护用）  
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllSYX(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf)
{
  int i,j,k;
  struct VDBYX DBYX;
  BYTE *p=buf;

  if (wBufLen<sizeof(BYTE)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYX.wNo=i;
    if (atomicReadSYX(wEqpID,1,sizeof(struct VDBYX),&DBYX,FALSE)==1)
    {
      *p=DBYX.byValue;
      p++;
      j++;
      k+=sizeof(BYTE);
      if (k>wBufLen-sizeof(BYTE)) break;
    } 
    else
      break;	
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  ReadRangeAllSYXBit ID:1
 Purpose:    读连续点所有单点遥信(一位代表一遥信)（不考虑其是否发送，维护用） 
 Input:      wNum: 要读的个数
             wBeginNo:起始点号
 Output:     实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllSYXBit(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf)
{
  int i,j,k,m;
  struct VDBYX DBYX;
  BYTE *p=buf;
  BYTE value;
 
  if (wBufLen<sizeof(BYTE)) return(0);
  
  j=k=0;
  m=value=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYX.wNo=i;
    if (atomicReadSYX(wEqpID,1,sizeof(DBYX),&DBYX,FALSE)==1)
    {
      value|=(((DBYX.byValue&0x80)>>(7-m)));
      if (++m==8)
      {	
        *p=value;
        value=m=0;
        p++;
        k+=sizeof(BYTE);
        if (k>wBufLen-sizeof(BYTE)) break;        
      }  
      j++;
    } 
    else
      break;	
  }  
 
  if (j%8)   *p=value;  	
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  QuerySCOSWrtOffset ID:1
 Purpose:    查询单点COS的写指针偏移  (维护用)
 Input:      pwWriteOffset:写指针偏移(相对于pwPoolLen)
 Output:     		
 Errors:
------------------------------------------------------------------------*/
STATUS QuerySCOSWrtOffset(WORD wEqpID,WORD correctOffset,WORD *pwWriteOffset)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if ((pVEqp->SCOS.pPoolHead==NULL)||(pVEqp->Cfg.wSYXNum==0))
        return(ERROR);
      else
      {
        if (pVEqp->SCOS.wWritePtr<correctOffset)
          //*pwWriteOffset=pVEqp->SCOS.wPoolNum-correctOffset+pVEqp->SCOS.wWritePtr;
          *pwWriteOffset=0;
        else
          *pwWriteOffset=pVEqp->SCOS.wWritePtr-correctOffset;        
        return(OK);
      }  
    }   
  } 
  else
  {
    if ((pTEqp->SCOS.pPoolHead==NULL)||(pTEqp->SYXSendCfg.wNum==0))
      return(ERROR);
    else
    {
      if (pTEqp->SCOS.wWritePtr<correctOffset)
        //*pwWriteOffset=pTEqp->SCOS.wPoolNum-correctOffset+pTEqp->SCOS.wWritePtr;
        *pwWriteOffset=0;
      else
        *pwWriteOffset=pTEqp->SCOS.wWritePtr-correctOffset;        
      return(OK);
    }  
  }  
}


/*------------------------------------------------------------------------
 Procedure:  QuerySCOS ID:1
 Purpose:    查询单点COS  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD QuerySCOS(WORD wEqpID,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBCOS)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->SCOS.pPoolHead==NULL)||(pVEqp->Cfg.wSYXNum==0))
        return(0);
      else
        j=atomicQueryVESCOS(pVEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
    }   
  } 
  else
  {
    if ((pTEqp->SCOS.pPoolHead==NULL)||(pTEqp->SYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicQueryTESCOS(pTEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
  }  

  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  QuerySSOEWrtOffset ID:1
 Purpose:    查询单点SOE的写指针偏移  (维护用)
 Input:      pwWriteOffset:写指针偏移(相对于pwPoolLen)
 Output:     		
 Errors:
------------------------------------------------------------------------*/
STATUS QuerySSOEWrtOffset(WORD wEqpID,WORD correctOffset,WORD *pwWriteOffset)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if ((pVEqp->SSOE.pPoolHead==NULL)||(pVEqp->Cfg.wSYXNum==0))
        return(ERROR);
      else
      {
        if (pVEqp->SSOE.wWritePtr<correctOffset)
          //*pwWriteOffset=pVEqp->SSOE.wPoolNum-correctOffset+pVEqp->SSOE.wWritePtr;
          *pwWriteOffset=0;
        else
          *pwWriteOffset=pVEqp->SSOE.wWritePtr-correctOffset;        
        return(OK);
      }  
    }   
  } 
  else
  {
    if ((pTEqp->SSOE.pPoolHead==NULL)||(pTEqp->SYXSendCfg.wNum==0))
      return(ERROR);
    else
    {
      if (pTEqp->SSOE.wWritePtr<correctOffset)
        //*pwWriteOffset=pTEqp->SSOE.wPoolNum-correctOffset+pTEqp->SSOE.wWritePtr;
        *pwWriteOffset=0;
      else
        *pwWriteOffset=pTEqp->SSOE.wWritePtr-correctOffset;        
      return(OK);
    }  
  }  
}


/*------------------------------------------------------------------------
 Procedure:  QuerySSOE ID:1
 Purpose:    查询单点SOE  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD QuerySSOE(WORD wEqpID,WORD wNum,WORD wReadOffset ,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(struct VDBSOE)) return(0);

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->SSOE.pPoolHead==NULL)||(pVEqp->Cfg.wSYXNum==0))
        return(0);
      else
        j=atomicQueryVESSOE(pVEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
    }   
  } 
  else
  {
    if ((pTEqp->SSOE.pPoolHead==NULL)||(pTEqp->SYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicQueryTESSOE(pTEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
  }  

  return(j);  
}

#if 0
/******************************** EVYX **********************************/


/********************************write **********************************/


/*------------------------------------------------------------------------
 Procedure:     WriteEVYX ID:1
 Purpose:       写离散点瞬变遥信(一字节代表一遥信) 
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteEVYX(WORD wEqpID,WORD wNum, struct VDBYX *buf,BOOL bLock)
{
  struct VTrueEqp *pTEqp;
  struct VDBYX EVYX,* p=buf;
  struct VTrueYX *pEVYX;
  struct VTrueYXCfg *pCfg;
  struct VDBSOE DBSOE;  
  int i,maxno;
  DWORD dwCfg;
  BOOL change;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pTEqp->pEVYX==NULL)||(pTEqp->pEVYXCfg==NULL))  return;

  maxno=pTEqp->Cfg.wEVYXNum-1;
  if (maxno<0)  return;

  smMTake(dwEVYXSem);  	  
  
  for (i=0;i<wNum;i++)
  {
    if ((p->wNo>maxno)||((p->byValue&0x01)==0))
    {
      p++;
      continue;
    } 

    EVYX.wNo=p->wNo;
    EVYX.byValue=p->byValue;
    if ((EVYX.byValue&0x01)==0) //当前值
    {
      p++;
	  continue;
    }  

	if (bLock&&(dwCfg&0x0x80000000))  //正常写入，考虑闭锁
	{
	  p++;
	  continue;
	}

    pCfg=pTEqp->pEVYXCfg+p->wNo;
    pEVYX=pTEqp->pEVYX+p->wNo;   
    dwCfg=pCfg->dwCfg;  

    dwCfg|=0x08;  //数据库产生COS

    if (dwCfg&0x02)  //取反
      EVYX.byValue^=0x80;

    if ((dwCfg&0x08)||(dwCfg&0x04))  //write cos or soe
    {
      if (pEVYX->byValue!=EVYX.byValue)
        change=TRUE;
      else
        change=FALSE;	
     
      if (change)
      {
        if (dwCfg&0x08)  //write cos
      	  atomicWriteEVCOS(wEqpID,1,(VDBCOS *)p,FALSE,TRUE);
        if (dwCfg&0x04)  //write soe
        {  
          DBSOE.wNo=p->wNo;
          DBSOE.byValue=p->byValue;
          GetSysClock(&DBSOE.Time,CALCLOCK);
          atomicWriteEVSOE(wEqpID,1,&DBSOE,FALSE,TRUE);
        }
      } 
  	} 

    if (!(dwCfg&0x10))
      pEVYX->byValue=EVYX.byValue;
    p++;   
  }
   
  smMGive(dwEVYXSem);
}  


/*------------------------------------------------------------------------
 Procedure:     WriteRangeEVYX ID:1
 Purpose:       写连续点瞬变遥信(一字节代表一遥信)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeEVYX(WORD wEqpID,WORD wBeginNo,WORD wNum, BYTE *buf)
{
  struct VTrueEqp *pTEqp;
  BYTE * p=buf;
  struct VDBYX DBYX;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pEVYX==NULL)||(pTEqp->pEVYXCfg==NULL)) return;

  maxno=pTEqp->Cfg.wEVYXNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBYX.wNo=i+wBeginNo;
    if ((DBYX.wNo>maxno)||((*p&0x01)==0))
    {
       p++;
       continue;
    } 
    DBYX.byValue=*p;
    WriteEVYX(wEqpID,1,&DBYX,TRUE);         
    p++;
  }  
}  


/*------------------------------------------------------------------------
 Procedure:     WriteRangeEVYXBit ID:1
 Purpose:       写连续点瞬变遥信(一位代表一遥信)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeEVYXBit(WORD wEqpID,WORD wBeginNo,WORD wNum, BYTE *buf)
{
  struct VTrueEqp *pTEqp;
  BYTE * p=buf;
  struct VDBYX DBYX;
  int i,j,maxno;

  if (wNum==0) return;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pEVYX==NULL)||(pTEqp->pEVYXCfg==NULL)) return;

  maxno=pTEqp->Cfg.wEVYXNum-1;
  if (maxno<0) return;

  if (maxno>wBeginNo+wNum-1) maxno=wBeginNo+wNum-1;

  for (i=0;i<(wNum-1)/8+1;i++)
  {
    for (j=0;j<8;j++)
    {
      DBYX.wNo=i*8+wBeginNo+j;
      if (DBYX.wNo>maxno)
        return;
      DBYX.byValue=((*p&(0x01<<j))<<(7-j))|0x01;
      WriteEVYX(wEqpID,1,&DBYX,TRUE);         
    }  
    p++;
  }  
}  


/*------------------------------------------------------------------------
 Procedure:     WriteEVCOS ID:1
 Purpose:       写瞬变COS
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteEVCOS(WORD wEqpID,WORD wNum,struct VDBCOS *buf) 
{
  atomicWriteEVCOS(wEqpID,wNum,buf,TRUE,TRUE) ;
}  

/*------------------------------------------------------------------------
 Procedure:     WriteEVSOE ID:1
 Purpose:       写瞬变SOE
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteEVSOE(WORD wEqpID,WORD wNum,struct VDBSOE *buf) 
{
  atomicWriteEVSOE(wEqpID,wNum,buf,TRUE,TRUE) ;
}  

/******************************** read **********************************/


/*------------------------------------------------------------------------
 Procedure:     ReadEVYX ID:1
 Purpose:       读离散点瞬变遥信(一字节代表一遥信) 
 Input:         wNum: 要读的个数
 Output:        实际所读的个数   	
 Errors:
------------------------------------------------------------------------*/
WORD ReadEVYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYX *buf)
{
  if (wBufLen<sizeof(VDBYX)) return(0);
  
  return(atomicReadEVYX(wEqpID,wNum,wBufLen,buf,TRUE));
}


/*------------------------------------------------------------------------
 Procedure:     ReadRangeEVYX ID:1
 Purpose:       读连续点瞬变遥信(一字节代表一遥信)  
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeEVYX(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf)
{
  int i,j,k;
  struct VDBYX DBYX;
  BYTE *p=buf;

  if (wBufLen<sizeof(BYTE)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYX.wNo=i;
    if (atomicReadEVYX(wEqpID,1,sizeof(VDBYX),&DBYX,TRUE)==1)
    {
      *p=DBYX.byValue;
      p++;
      j++;
      k+=sizeof(BYTE);
      if (k>wBufLen-sizeof(BYTE)) break;
    } 
    else
      break;	
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     ReadRangeEVYXBit ID:1
 Purpose:       读连续点瞬变遥信(一位代表一遥信)  
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeEVYXBit(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf)
{
  int i,j,k,m;
  struct VDBYX DBYX;
  BYTE *p=buf;
  BYTE value;
 
  if (wBufLen<sizeof(BYTE)) return(0);
  
  j=k=0;
  m=value=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYX.wNo=i;
    if (atomicReadEVYX(wEqpID,1,sizeof(VDBYX),&DBYX,TRUE)==1)
    {
      value|=(((DBYX.byValue&0x80)>>(7-m)));
      if (++m==8)
      {	
        *p=value;
        value=m=0;
        p++;
        k+=sizeof(BYTE);
        if (k>wBufLen-sizeof(BYTE)) break;        
      }  
      j++;
    } 
    else
      break;	
  }  

  if (j%8)   *p=value;  	
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  ReadEVCOS ID:1
 Purpose:    读瞬变COS 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD ReadEVCOS(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VDBCOS *buf)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j,*pwReadPtr;

  if (wBufLen<sizeof(VDBCOS)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if (GetReadPtr(wTaskID,wEqpID,EVCOSUFLAG,&pwReadPtr)==ERROR)  return(0);
  
  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->EVCOS.pPoolHead==NULL)||(pVEqp->Cfg.wEVYXNum==0))
        return(0);
      else
        j=atomicReadVEEVCOS(wTaskID,wEqpID,pVEqp,wNum,pwReadPtr,wBufLen,buf);
    }   
  } 
  else
  {
    if  ((pTEqp->EVCOS.pPoolHead==NULL)||(pTEqp->EVYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicReadTEEVCOS(wTaskID,wEqpID,pTEqp,wNum,pwReadPtr,wBufLen,buf);
  }  

  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  ReadEVSOE ID:1
 Purpose:    读瞬变SOE 
 Input:      wNum: 要读的个数
             pwReadPtr:函数会自动修正错误读指针
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD ReadEVSOE(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VDBSOE *buf)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j,*pwReadPtr;

  if (wBufLen<sizeof(VDBSOE)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if (GetReadPtr(wTaskID,wEqpID,EVSOEUFLAG,&pwReadPtr)==ERROR)  return(0);
  
  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->EVSOE.pPoolHead==NULL)||(pVEqp->Cfg.wEVYXNum==0))
        return(0);
      else
        j=atomicReadVEEVSOE(wTaskID,wEqpID,pVEqp,wNum,pwReadPtr,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->EVSOE.pPoolHead==NULL)||(pTEqp->EVYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicReadTEEVSOE(wTaskID,wEqpID,pTEqp,wNum,pwReadPtr,wBufLen,buf);
  }  

  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:     ReadRangeAllEVYX ID:1
 Purpose:       读连续点瞬变遥信(一字节代表一遥信)（不考虑其是否发送，维护用）   
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllEVYX(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf)
{
  int i,j,k;
  struct VDBYX DBYX;
  BYTE *p=buf;

  if (wBufLen<sizeof(BYTE)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYX.wNo=i;
    if (atomicReadEVYX(wEqpID,1,sizeof(VDBYX),&DBYX,FALSE)==1)
    {
      *p=DBYX.byValue;
      p++;
      j++;
      k+=sizeof(BYTE);
      if (k>wBufLen-sizeof(BYTE)) break;
    } 
    else
      break;	
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  ReadRangeAllEVYXBit ID:1
 Purpose:    读连续点所有瞬变遥信(一位代表一遥信)（不考虑其是否发送，维护用） 
 Input:      wNum: 要读的个数
             wBeginNo:起始点号
 Output:     实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllEVYXBit(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf)
{
  int i,j,k,m;
  struct VDBYX DBYX;
  BYTE *p=buf;
  BYTE value;
 
  if (wBufLen<sizeof(BYTE)) return(0);
  
  j=k=0;
  m=value=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYX.wNo=i;
    if (atomicReadEVYX(wEqpID,1,sizeof(VDBYX),&DBYX,FALSE)==1)
    {
      value|=(((DBYX.byValue&0x80)>>(7-m)));
      if (++m==8)
      {	
        *p=value;
        value=m=0;
        p++;
        k+=sizeof(BYTE);
        if (k>wBufLen-sizeof(BYTE)) break;        
      }  
      j++;
    } 
    else
      break;	
  }  

  if (j%8)   *p=value;  	
  
  return(j);
}


/*------------------------------------------------------------------------
 Procedure:  QueryEVCOSWrtOffset ID:1
 Purpose:    查询瞬变COS的写指针偏移  (维护用)
 Input:      pwWriteOffset:写指针偏移(相对于pwPoolLen)
 Output:     		
 Errors:
------------------------------------------------------------------------*/
STATUS QueryEVCOSWrtOffset(WORD wEqpID,WORD correctOffset,WORD *pwWriteOffset)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if ((pVEqp->EVCOS.pPoolHead==NULL)||(pVEqp->Cfg.wEVYXNum==0))
        return(ERROR);
      else
      {
         if (pVEqp->EVCOS.wWritePtr<correctOffset)
           //*pwWriteOffset=pVEqp->EVCOS.wPoolNum-correctOffset+pVEqp->EVCOS.wWritePtr;
           *pwWriteOffset=0;
         else
           *pwWriteOffset=pVEqp->EVCOS.wWritePtr-correctOffset;        
        return(OK);
      }  
    }   
  } 
  else
  {
    if ((pTEqp->EVCOS.pPoolHead==NULL)||(pTEqp->EVYXSendCfg.wNum==0))
      return(ERROR);
    else
    {
      if (pTEqp->EVCOS.wWritePtr<correctOffset)
        //*pwWriteOffset=pTEqp->EVCOS.wPoolNum-correctOffset+pTEqp->EVCOS.wWritePtr;
        *pwWriteOffset=0;
      else
        *pwWriteOffset=pTEqp->EVCOS.wWritePtr-correctOffset;        
      return(OK);
    }  
  }  
}


/*------------------------------------------------------------------------
 Procedure:  QueryEVCOS ID:1
 Purpose:    查询瞬变COS  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD QueryEVCOS(WORD wEqpID,WORD wNum,WORD wReadOffset ,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(VDBCOS)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->EVCOS.pPoolHead==NULL)||(pVEqp->Cfg.wEVYXNum==0))
        return(0);
      else
        j=atomicQueryVEEVCOS(pVEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
    }   
  } 
  else
  {
    if ((pTEqp->EVCOS.pPoolHead==NULL)||(pTEqp->EVYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicQueryTEEVCOS(pTEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
  }  

  return(j);  
}


/*------------------------------------------------------------------------
 Procedure:  QueryEVSOEWrtOffset ID:1
 Purpose:    查询瞬变SOE的写指针偏移  (维护用)
 Input:      pwWriteOffset:写指针偏移(相对于pwPoolLen)
 Output:     		
 Errors:
------------------------------------------------------------------------*/
STATUS QueryEVSOEWrtOffset(WORD wEqpID,WORD correctOffset,WORD *pwWriteOffset)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if ((pVEqp->EVSOE.pPoolHead==NULL)||(pVEqp->Cfg.wEVYXNum==0))
        return(ERROR);
      else
      {
        if (pVEqp->EVSOE.wWritePtr<correctOffset)
          //*pwWriteOffset=pVEqp->EVSOE.wPoolNum-correctOffset+pVEqp->EVSOE.wWritePtr;
          *pwWriteOffset=0;
        else
          *pwWriteOffset=pVEqp->EVSOE.wWritePtr-correctOffset;        
        return(OK);
      }  
    }   
  } 
  else
  {
    if ((pTEqp->EVSOE.pPoolHead==NULL)||(pTEqp->EVYXSendCfg.wNum==0))
      return(ERROR);
    else
    {
      if (pTEqp->EVSOE.wWritePtr<correctOffset)
        //*pwWriteOffset=pTEqp->EVSOE.wPoolNum-correctOffset+pTEqp->EVSOE.wWritePtr;
        *pwWriteOffset=0;
      else
        *pwWriteOffset=pTEqp->EVSOE.wWritePtr-correctOffset;        
      return(OK);
    }  
  }  
}


/*------------------------------------------------------------------------
 Procedure:  QueryEVSOE ID:1
 Purpose:    查询瞬变SOE  (维护用)
 Input:      wNum: 要读的个数
             wReadOffset: 读偏移(相对于pwPoolLen)
             pwWriteOffset:写指针偏移(相对于pwPoolLen)
             pwPoolLen:COS PollNum
 Output:     实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD QueryEVSOE(WORD wEqpID,WORD wNum,WORD wReadOffset ,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  WORD j;

  if (wBufLen<sizeof(VDBSOE)) return(0);

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if ((pVEqp->EVSOE.pPoolHead==NULL)||(pVEqp->Cfg.wEVYXNum==0))
        return(0);
      else
        j=atomicQueryVEEVSOE(pVEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
    }   
  } 
  else
  {
    if ((pTEqp->EVSOE.pPoolHead==NULL)||(pTEqp->EVYXSendCfg.wNum==0))
      return(0);
    else
      j=atomicQueryTEEVSOE(pTEqp,wNum,wReadOffset,wBufLen,buf,pwWriteOffset,pwPoolLen);
  }  

  return(j);  
}
#endif

/******************************** YC **********************************/

/********************************write **********************************/
void SetRangeYCCfg(WORD wEqpID,WORD wBeginNo,int nNum,DWORD dwCfg)
{
	struct VTrueEqp *pTEqp;
	struct VTrueYCCfg *pCfg;
	int i,maxno;
	
	if (wEqpID>=*g_Sys.Eqp.pwNum)  return;
	
	if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;
	
	//if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))  return;
	
	maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
	if (maxno<0)  return;

    if (nNum == -1)
		nNum = pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum;
	else if (nNum == -2)
		nNum = pTEqp->Cfg.wYCNum;		
	
	smMTake(dwYCSem);		
	
	for (i=wBeginNo;i<(wBeginNo+nNum);i++)
	{
	  if (i>maxno)  break;

	  pCfg=pTEqp->pYCCfg+i;
      pCfg->dwCfg |= dwCfg;		  
	}
	 
	smMGive(dwYCSem);
}  

void ClearRangeYCCfg(WORD wEqpID,WORD wBeginNo,int nNum,DWORD dwCfg)
{
	struct VTrueEqp *pTEqp;
	struct VTrueYCCfg *pCfg;
	int i,maxno;
	
	if (wEqpID>=*g_Sys.Eqp.pwNum)  return;
	
	if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;
	
	//if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))  return;
	
	maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
	if (maxno<0)  return;

    if (nNum == -1)
		nNum = pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum;
	else if (nNum == -2)
		nNum = pTEqp->Cfg.wYCNum;		
	
	smMTake(dwYCSem);		
	
	for (i=wBeginNo;i<(wBeginNo+nNum);i++)
	{
	  if (i>maxno)  break;

	  pCfg=pTEqp->pYCCfg+i;
      pCfg->dwCfg &= (~dwCfg);	

	  
	}
	 
	smMGive(dwYCSem);
}  

/*------------------------------------------------------------------------
 Procedure: 	LockYC ID:1
 Purpose:		闭锁遥测并置数,如果遥测标志最低位为0则仅闭锁
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void LockYC(WORD wEqpID, WORD wNum, struct VDBYCF_L *buf)
{
	struct VTrueEqp *pTEqp;
	struct VDBYCF_L * p=buf;
	struct VTrueYCCfg *pCfg;
	int i,maxno;
	
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
      pCfg->dwCfg|=0x80000000;	
	  
	  p++;	 
	}
	 
	smMGive(dwYCSem);

	atomicWriteYCF_L(wEqpID,wNum,buf,FALSE);
}  


/*------------------------------------------------------------------------
 Procedure: 	UnLockYC ID:1
 Purpose:		解锁遥测
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void UnLockYC(WORD wEqpID, WORD wNum, struct VDBYCF_L *buf)
{
	struct VTrueEqp *pTEqp;
	struct VDBYCF_L * p=buf;
	struct VTrueYCCfg *pCfg;
	int i,maxno;
	
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
	  pCfg->dwCfg&=(~0x80000000);	

	  p++;	 
	}
	 
	smMGive(dwYCSem);	
	
	evSend(g_Sys.Eqp.pInfo[wEqpID].wTaskID, EV_DATAREFRESH);
} 


/*------------------------------------------------------------------------
 Procedure: 	LockDD ID:1
 Purpose:		闭锁电度并置数,如果电度标志最低位为0则仅闭锁
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void LockDD(WORD wEqpID, WORD wNum, struct VDBDDF *buf)
{
	struct VTrueEqp *pTEqp;
	struct VDBDDF * p=buf;
	struct VTrueDDCfg *pCfg;
	int i,maxno;
	
	if (wEqpID>=*g_Sys.Eqp.pwNum)  return;
	
	if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;
	
	if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL))  return;
	
    maxno=pTEqp->Cfg.wDDNum-1;
	if (maxno<0)  return;
	
	//smMTake(dwDDSem);		
	
	for (i=0;i<wNum;i++)
	{
	  if (p->wNo>maxno)
	  {
		p++;
		continue;
	  } 
		WriteDDF(wEqpID,wNum,buf);
		pCfg=pTEqp->pDDCfg+p->wNo;
		pCfg->dwCfg|=0x80000000;	
	  p++;	 
	}
	//smMGive(dwDDSem);
}  


/*------------------------------------------------------------------------
 Procedure: 	UnLockDD ID:1
 Purpose:		解锁DD
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void UnLockDD(WORD wEqpID, WORD wNum, struct VDBDDF *buf)
{
	struct VTrueEqp *pTEqp;
	struct VDBDDF * p=buf;
	struct VTrueDDCfg *pCfg;
	int i,maxno;
	
	if (wEqpID>=*g_Sys.Eqp.pwNum)  return;
	
	if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;
	
	if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL))  return;
	
    maxno=pTEqp->Cfg.wDDNum-1;
	if (maxno<0)  return;
	
	//smMTake(dwDDSem);		
	
	for (i=0;i<wNum;i++)
	{
	  if (p->wNo>maxno)
	  {
		p++;
		continue;
	  } 

	  pCfg=pTEqp->pDDCfg+p->wNo;
	  pCfg->dwCfg&=(~0x80000000);	
		WriteDDF(wEqpID,wNum,buf);
	  p++;	 
	}
	//smMGive(dwDDSem);	
}


/*------------------------------------------------------------------------
 Procedure:     WriteYCF ID:1
 Purpose:       写离散点遥测(带一字节标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteYCF_F(WORD wEqpID, WORD wNum, struct VDBYCF_F *buf)
{
	atomicWriteYCF_F(wEqpID, wNum, buf, TRUE);
}  

/*------------------------------------------------------------------------
 Procedure:     WriteYC ID:1
 Purpose:       写离散点遥测(无标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteYC_F(WORD wEqpID, WORD wNum, struct VDBYC_F *buf)
{
	atomicWriteYC_F(wEqpID, wNum, buf, TRUE);
}  

/*------------------------------------------------------------------------
 Procedure:     WriteRangeYCF ID:1
 Purpose:       写连续点遥测(带一字节标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeYCF_F(WORD wEqpID, WORD wBeginNo, WORD wNum, struct VYCF_F *buf)
{
  struct VTrueEqp *pTEqp;
  struct VYCF_F * p=buf;
  struct VDBYCF_F DBYCF_F;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL)) return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBYCF_F.wNo=i+wBeginNo;
    if (DBYCF_F.wNo>maxno)
    {
       p++;
       continue;
    } 
    DBYCF_F.fValue=p->fValue;
    DBYCF_F.byFlag=p->byFlag;
    WriteYCF_F(wEqpID,1,&DBYCF_F);         
    p++;
  }  
}  

/*------------------------------------------------------------------------
 Procedure:     WriteRangeYC ID:1
 Purpose:       写连续点遥测(无标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeYC_F(WORD wEqpID, WORD wBeginNo, WORD wNum, float *buf)
{
  struct VTrueEqp *pTEqp;
  float * p=buf;
  struct VDBYC_F DBYC_F;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL)) return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBYC_F.wNo=i+wBeginNo;
    if (DBYC_F.wNo>maxno)
    {
       p++;
       continue;
    } 
    DBYC_F.fValue=*p;
    WriteYC_F(wEqpID,1,&DBYC_F);         
    p++;
  }  
}  

/*------------------------------------------------------------------------
 Procedure:     WriteYCF ID:1
 Purpose:       写离散点遥测(带一字节标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteYCF_L(WORD wEqpID,WORD wNum, struct VDBYCF_L *buf)
{
	atomicWriteYCF_L(wEqpID, wNum, buf, TRUE);
}  

/*------------------------------------------------------------------------
 Procedure:     WriteYC ID:1
 Purpose:       写离散点遥测(无标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteYC_L(WORD wEqpID,WORD wNum, struct VDBYC_L *buf)
{
	atomicWriteYC_L(wEqpID, wNum, buf, TRUE);
}  

void WriteYCCfg(WORD wEqpID,WORD wNo, WORD type, WORD fdnum,char*unit)
{
	struct VTrueEqp *pTEqp;
	struct VTrueYCCfg *pCfg;
	
	if (wEqpID>=*g_Sys.Eqp.pwNum) return;
	 if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;
	if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL)) return;	

	pCfg = pTEqp->pYCCfg + wNo;
	pCfg->tyctype = type;
	pCfg->tycfdnum = fdnum;
	memcpy(pCfg->tycunit,unit,sizeof(pCfg->tycunit));
} 

STATUS ReadYCCfg(WORD wEqpID,WORD wNo,struct VYCF_L_Cfg* buf,BOOL asSend)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	WORD teid,offset;
	struct VTrueYCCfg *pCfg;
	struct VYCF_L_Cfg *p = buf;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if (pVEqp->pYC==NULL)
        return(ERROR);
      else
      {
			if(wNo >= pVEqp->Cfg.wYCNum)
				return(ERROR);
				
			teid=pVEqp->pYC[wNo].wTEID;
			offset=pVEqp->pYC[wNo].wOffset;      

			if (g_Sys.Eqp.pInfo[teid].pTEqpInfo==NULL) return(ERROR);

			if (g_Sys.Eqp.pInfo[teid].pTEqpInfo->pYC == NULL) return(ERROR);
			if(g_Sys.Eqp.pInfo[teid].pTEqpInfo->pYCCfg == NULL) return (ERROR);
			pCfg = g_Sys.Eqp.pInfo[teid].pTEqpInfo->pYCCfg + offset;
			p->tycfdnum = pCfg->tycfdnum;
			p->tyctype = pCfg->tyctype;
			memcpy(p->tycunit,pCfg->tycunit,sizeof(p->tycunit));	
      }
    }   
  } 
  else
  {
    if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))
      return(ERROR);
    else
    {
			if(asSend)
			{
				if(pTEqp->YCSendCfg.pwIndex[wNo] >= pTEqp->Cfg.wYCNum)
					return(ERROR);
				pCfg = pTEqp->pYCCfg + pTEqp->YCSendCfg.pwIndex[wNo];
			}
			else
			{
				if(wNo >= pTEqp->Cfg.wYCNum)
					return(ERROR);
				pCfg = pTEqp->pYCCfg + wNo;
			}
				
		p->tycfdnum = pCfg->tycfdnum;
		p->tyctype = pCfg->tyctype;
		memcpy(p->tycunit,pCfg->tycunit,sizeof(p->tycunit));	
    }
  }  

  return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     
 Purpose:       实装置遥测点号转为需装置遥测点号
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/

STATUS TEYcNoToVEYcNo(WORD wTEqpID, WORD wVEqpID, WORD wTENo,WORD *wVENo)
{
	struct VTrueEqp *pTEqp;
	struct VTrueYCCfg *pCfg;
	int j;

	if (wTEqpID>=*g_Sys.Eqp.pwNum)    return (ERROR);

	if ((pTEqp=g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo)==NULL)  return (ERROR);
  	
	pCfg=pTEqp->pYCCfg+wTENo;

	for (j=0;j<pCfg->wVENum;j++)
    {	
    	if(wVEqpID != pCfg->pInvRef[j].wEqpID)
			continue;
		
    	*wVENo = pCfg->pInvRef[j].wNo;
		return (OK);
	}

	return (ERROR);
	
}

STATUS ReadDDSendNo(WORD wEqpID, WORD wNo,WORD *wSendNo)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	int maxno;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) //虚装置
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if (pVEqp->pDD== NULL)
        return(ERROR);
      else
      {
			maxno = pVEqp->Cfg.wDDNum - 1;
			if(maxno < 0)
				return ERROR;
			if(wNo > maxno)
				return ERROR;
			
			*wSendNo = pVEqp->pDD[wNo].wSendNo;
			return OK;
      }
    }   
  } 
  else
  {
    if (pTEqp->pDD==NULL)
      return(ERROR);

	maxno = pTEqp->DDSendCfg.wNum - 1;

	if(maxno < 0)
		return ERROR;
	if(wNo > maxno)
		return ERROR;
	
	*wSendNo = pTEqp->DDSendCfg.pwIndex[wNo];
  }  
  return(OK);
}

STATUS ReadYCSendNo(WORD wEqpID, WORD wNo,WORD *wSendNo)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	int maxno;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) //虚装置
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if (pVEqp->pYC== NULL)
        return(ERROR);
      else
      {
			maxno = pVEqp->Cfg.wYCNum - 1;
			if(maxno < 0)
				return ERROR;
			if(wNo > maxno)
				return ERROR;
			
			*wSendNo = pVEqp->pYC[wNo].wSendNo;
			return OK;
      }
    }   
  } 
  else
  {
    if (pTEqp->pYC==NULL)
      return(ERROR);

	maxno = pTEqp->YCSendCfg.wNum - 1;

	if(maxno < 0)
		return ERROR;
	if(wNo > maxno)
		return ERROR;
	
	*wSendNo = pTEqp->YCSendCfg.pwIndex[wNo];
  }  
  return(OK);
}


STATUS ReadDYXSendNo(WORD wEqpID, WORD wNo,WORD *wSendNo)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	int maxno;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)//虚装置
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if (pVEqp->pDYX == NULL)
        return(ERROR);
      else
      {
			maxno = pVEqp->Cfg.wDYXNum - 1;
			if(maxno < 0)
				return ERROR;
			if(wNo > maxno)
				return ERROR;

			*wSendNo = pVEqp->pDYX[wNo].wSendNo;
			return OK;
      }
    }   
  } 
  else
  {
    if (pTEqp->pDYX==NULL)
      return(ERROR);

	maxno = pTEqp->DYXSendCfg.wNum - 1;

	if(maxno < 0)
		return ERROR;
	if(wNo > maxno)
		return ERROR;
	
	*wSendNo = pTEqp->DYXSendCfg.pwIndex[wNo];
  }  
  return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     
 Purpose:       实装置遥信点号转为需装置遥信点号
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/

STATUS TEYxNoToVEYxNo(WORD wTEqpID, WORD wVEqpID, WORD wTENo,WORD *wVENo)
{
	struct VTrueEqp *pTEqp;
	struct VTrueYXCfg *pCfg;
	int j;

	if (wTEqpID>=*g_Sys.Eqp.pwNum)    return (ERROR);

	if ((pTEqp=g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo)==NULL)  return (ERROR);
  	
	pCfg=pTEqp->pSYXCfg+wTENo;

	for (j=0;j<pCfg->wVENum;j++)
    {	
    	if(wVEqpID != pCfg->pInvRef[j].wEqpID)
			continue;
		
    	*wVENo = pCfg->pInvRef[j].wNo;
		return (OK);
	}

	return (ERROR);
	
}

STATUS ReadSYXSendNo(WORD wEqpID, WORD wNo,WORD *wSendNo)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	int i,maxno;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)//虚装置
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(ERROR);
    else
    {
      if (pVEqp->pSYX == NULL)
        return(ERROR);
      else
      {
			maxno = pVEqp->Cfg.wSYXNum - 1;
			if(maxno < 0)
				return ERROR;
			if(wNo > maxno)
				return ERROR;

			*wSendNo = pVEqp->pSYX[wNo].wSendNo;
			return OK;
      }
    }   
  } 
  else
  {
    if (pTEqp->pSYX==NULL)
      return(ERROR);

	maxno = pTEqp->SYXSendCfg.wNum - 1;

	if(maxno < 0)
		return ERROR;
	if(wNo > maxno)
		return ERROR;
	
	*wSendNo = pTEqp->SYXSendCfg.pwIndex[wNo];
  }  
  return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     WriteRangeYCF ID:1
 Purpose:       写连续点遥测(带一字节标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeYCF_L(WORD wEqpID,WORD wBeginNo,WORD wNum, struct VYCF_L *buf)
{
  struct VTrueEqp *pTEqp;
  struct VYCF_L * p=buf;
  struct VDBYCF_L DBYCF_L;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL)) return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBYCF_L.wNo=i+wBeginNo;
    if (DBYCF_L.wNo>maxno)
    {
       p++;
       continue;
    } 
    DBYCF_L.lValue=p->lValue;
    DBYCF_L.byFlag=p->byFlag;
    WriteYCF_L(wEqpID,1,&DBYCF_L);         
    p++;
  }  
}  

/*------------------------------------------------------------------------
 Procedure:     WriteRangeYC ID:1
 Purpose:       写连续点遥测(无标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeYC_L(WORD wEqpID,WORD wBeginNo,WORD wNum, long *buf)
{
  struct VTrueEqp *pTEqp;
  long * p=buf;
  struct VDBYC_L DBYC_L;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL)) return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBYC_L.wNo=i+wBeginNo;
    if (DBYC_L.wNo>maxno)
    {
       p++;
       continue;
    } 
    DBYC_L.lValue=*p;
    WriteYC_L(wEqpID,1,&DBYC_L);         
    p++;
  }  
}  

/*------------------------------------------------------------------------
 Procedure:     WriteYCF ID:1
 Purpose:       写离散点遥测(带一字节标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteYCF(WORD wEqpID,WORD wNum, struct VDBYCF *buf)
{
	atomicWriteYCF(wEqpID, wNum, buf, TRUE);
}  

/*------------------------------------------------------------------------
 Procedure:     WriteYC ID:1
 Purpose:       写离散点遥测(无标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteYC(WORD wEqpID,WORD wNum, struct VDBYC *buf)
{
	atomicWriteYC(wEqpID, wNum, buf, TRUE);
}  

/*------------------------------------------------------------------------
 Procedure:     WriteRangeYCF ID:1
 Purpose:       写连续点遥测(带一字节标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeYCF(WORD wEqpID,WORD wBeginNo,WORD wNum, struct VYCF *buf)
{
  struct VTrueEqp *pTEqp;
  struct VYCF * p=buf;
  struct VDBYCF DBYCF;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL)) return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBYCF.wNo=i+wBeginNo;
    if (DBYCF.wNo>maxno)
    {
       p++;
       continue;
    } 
    DBYCF.nValue=p->nValue;
    DBYCF.byFlag=p->byFlag;
    WriteYCF(wEqpID,1,&DBYCF);         
    p++;
  }  
}  

/*------------------------------------------------------------------------
 Procedure:     WriteRangeYC ID:1
 Purpose:       写连续点遥测(无标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeYC(WORD wEqpID,WORD wBeginNo,WORD wNum, short *buf)
{
  struct VTrueEqp *pTEqp;
  short * p=buf;
  struct VDBYC DBYC;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL)) return;

  maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBYC.wNo=i+wBeginNo;
    if (DBYC.wNo>maxno)
    {
       p++;
       continue;
    } 
    DBYC.nValue=*p;
    WriteYC(wEqpID,1,&DBYC);         
    p++;
  }  
}  


/******************************** read **********************************/

STATUS GetYC_ABC(WORD wEqpID, WORD wNo, long *A, long *B, long *C)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;

	if (wEqpID>=*g_Sys.Eqp.pwNum)  return ERROR;
	
	if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
	{
	  if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)	
		return ERROR;
	  else
	  {
		if (pVEqp->pYC==NULL)
		  return ERROR;
		else
		  return(atomicGetVEYC_ABC(pVEqp,wNo,A,B,C));
	  }   
	} 
	else
	{
	  if ((pTEqp->pYC==NULL)||(pTEqp->pYCCfg==NULL))
		return ERROR;
	  else
		return(atomicGetTEYC_ABC(pTEqp,wNo,A,B,C));
	}  
}

/*------------------------------------------------------------------------
 Procedure:     ReadYCF ID:1
 Purpose:       读离散点遥测(带一字节标志)
 Input:         wNum: 要读的个数
 Output:        实际所读的个数   	
 Errors:
------------------------------------------------------------------------*/
WORD ReadYCF_L(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYCF_L *buf)
{
  if (wBufLen<sizeof(struct VDBYCF_L)) return(0);
  
  return(atomicReadYCF_L(wEqpID,wNum,wBufLen,buf,TRUE));
}


/*------------------------------------------------------------------------
 Procedure:     ReadYC ID:1
 Purpose:       读离散点遥测(无标志)
 Input:         wNum: 要读的个数
 Output:        实际所读的个数   	
 Errors:
------------------------------------------------------------------------*/
WORD ReadYC_L(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYC_L *buf)
{
  if (wBufLen<sizeof(struct VDBYC_L)) return(0);
  
  return(atomicReadYC_L(wEqpID,wNum,wBufLen,buf,TRUE));
}

/*------------------------------------------------------------------------
 Procedure:     ReadRangeYCF ID:1
 Purpose:       读连续点遥测(带一字节标志) 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeYCF_L(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VYCF_L *buf)
{
  int i,j,k;
  struct VDBYCF_L DBYCF_L;
  struct VYCF_L *p=buf;

  if (wBufLen<sizeof(struct VYCF_L)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYCF_L.wNo=i;
    if (atomicReadYCF_L(wEqpID,1,sizeof(DBYCF_L),&DBYCF_L,TRUE)==1)
    {
      p->lValue=DBYCF_L.lValue;
      p->byFlag=DBYCF_L.byFlag;
      p++;
      j++;
      k+=sizeof(struct VYCF_L);
      if (k>wBufLen-sizeof(struct VYCF_L)) break;
    } 
    else
      break;	
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     ReadRangeYC ID:1
 Purpose:       读连续点遥测(无标志) 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeYC_L(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen, long *buf)
{
  int i,j,k;
  struct VDBYC_L DBYC_L;
  long *p=buf;

  if (wBufLen<sizeof(long)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYC_L.wNo=i;
    if (atomicReadYC_L(wEqpID,1,sizeof(DBYC_L),&DBYC_L,TRUE)==1)
    {
      *p=DBYC_L.lValue;
      p++;
      j++;
      k+=sizeof(long);
      if (k>wBufLen-sizeof(long)) break;
    } 
    else
      break;	
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure:     ReadRangeYCF ID:1
 Purpose:       读连续点所有遥测(带一字节标志) 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllYCF_L(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VYCF_L *buf)
{
  int i,j,k;
  struct VDBYCF_L DBYCF_L;
  struct VYCF_L *p=buf;

  if (wBufLen<sizeof(struct VYCF_L)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYCF_L.wNo=i;
    if (atomicReadYCF_L(wEqpID,1,sizeof(DBYCF_L),&DBYCF_L,FALSE)==1)
    {
      p->lValue=DBYCF_L.lValue;
      p->byFlag=DBYCF_L.byFlag;
      p++;
      j++;
      k+=sizeof(struct VYCF_L);
      if (k>wBufLen-sizeof(struct VYCF_L)) break;
    } 
    else
      break;	
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure:     ReadRangeAllYC ID:1
 Purpose:       读连续点所有遥测（无标志，不考虑其是否发送，维护用） 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllYC_L(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen, long *buf)
{
  int i,j,k;
  struct VDBYC_L DBYC_L;
  long *p=buf;

  if (wBufLen<sizeof(long)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYC_L.wNo=i;
    if (atomicReadYC_L(wEqpID,1,sizeof(DBYC_L),&DBYC_L,FALSE)==1)
    {
      *p=DBYC_L.lValue;
      p++;
      j++;
      k+=sizeof(long);
      if (k>wBufLen-sizeof(long)) break;
    } 
    else
      break;	
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure:     ReadYCF ID:1
 Purpose:       读离散点遥测(带一字节标志)
 Input:         wNum: 要读的个数
 Output:        实际所读的个数   	
 Errors:
------------------------------------------------------------------------*/
WORD ReadYCF(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYCF *buf)
{
  if (wBufLen<sizeof(struct VDBYCF)) return(0);
  
  return(atomicReadYCF(wEqpID,wNum,wBufLen,buf,TRUE));
}


/*------------------------------------------------------------------------
 Procedure:     ReadYC ID:1
 Purpose:       读离散点遥测(无标志)
 Input:         wNum: 要读的个数
 Output:        实际所读的个数   	
 Errors:
------------------------------------------------------------------------*/
WORD ReadYC(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYC *buf)
{
  if (wBufLen<sizeof(struct VDBYC)) return(0);
  
  return(atomicReadYC(wEqpID,wNum,wBufLen,buf,TRUE));
}

/*------------------------------------------------------------------------
 Procedure:     ReadRangeYCF ID:1
 Purpose:       读连续点遥测(带一字节标志) 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeYCF(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VYCF *buf)
{
  int i,j,k;
  struct VDBYCF DBYCF;
  struct VYCF *p=buf;

  if (wBufLen<sizeof(struct VYCF)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYCF.wNo=i;
    if (atomicReadYCF(wEqpID,1,sizeof(struct VDBYCF),&DBYCF,TRUE)==1)
    {
      p->nValue=DBYCF.nValue;
      p->byFlag=DBYCF.byFlag;
      p++;
      j++;
      k+=sizeof(struct VYCF);
      if (k>wBufLen-sizeof(struct VYCF)) break;
    } 
    else
      break;	
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     ReadRangeYC ID:1
 Purpose:       读连续点遥测(无标志) 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeYC(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,short *buf)
{
  int i,j,k;
  struct VDBYC DBYC;
  short *p=buf;

  if (wBufLen<sizeof(short)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYC.wNo=i;
    if (atomicReadYC(wEqpID,1,sizeof(struct VDBYC),&DBYC,TRUE)==1)
    {
      *p=DBYC.nValue;
      p++;
      j++;
      k+=sizeof(short);
      if (k>wBufLen-sizeof(short)) break;
    } 
    else
      break;	
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure:     ReadRangeYCF ID:1
 Purpose:       读连续点所有遥测(带一字节标志) 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllYCF(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VYCF *buf)
{
  int i,j,k;
  struct VDBYCF DBYCF;
  struct VYCF *p=buf;

  if (wBufLen<sizeof(struct VYCF)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYCF.wNo=i;
    if (atomicReadYCF(wEqpID,1,sizeof(struct VDBYCF),&DBYCF,FALSE)==1)
    {
      p->nValue=DBYCF.nValue;
      p->byFlag=DBYCF.byFlag;
      p++;
      j++;
      k+=sizeof(struct VYCF);
      if (k>wBufLen-sizeof(struct VYCF)) break;
    } 
    else
      break;	
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure:     ReadRangeAllYC ID:1
 Purpose:       读连续点所有遥测（无标志，不考虑其是否发送，维护用） 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllYC(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,short *buf)
{
  int i,j,k;
  struct VDBYC DBYC;
  short *p=buf;

  if (wBufLen<sizeof(short)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBYC.wNo=i;
    if (atomicReadYC(wEqpID,1,sizeof(struct VDBYC),&DBYC,FALSE)==1)
    {
      *p=DBYC.nValue;
      p++;
      j++;
      k+=sizeof(short);
      if (k>wBufLen-sizeof(short)) break;
    } 
    else
      break;	
  }  

  return(j);
}

/******************************** DD **********************************/


/********************************write **********************************/


/*------------------------------------------------------------------------
 Procedure:     WriteDDF ID:1
 Purpose:       写离散点电度(带一字节标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteDDF(WORD wEqpID,WORD wNum, struct VDBDDF *buf)
{
  struct VTrueEqp *pTEqp;
  struct VDBDDF * p=buf;
  struct VTrueDD *pDD;
  struct VTrueDDCfg *pCfg;
  int i,maxno;
  DWORD DTDD;
  float FTDD;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL))  return;

  maxno=pTEqp->Cfg.wDDNum-1;
  if (maxno<0)  return;

  smMTake(dwDDSem);  	  
  
  for (i=0;i<wNum;i++)
  {
    if (p->wNo>maxno)
    {
      p++;
      continue;
    } 
    
    pDD=pTEqp->pDD+p->wNo;   
    pCfg=pTEqp->pDDCfg+p->wNo;
    
		if(pCfg->dwCfg & 0x80000000) //闭锁
		{
			continue;
		}			
		
   if (pTEqp->Cfg.dwFlag&DDQUOTIETYENABLED)
	{
		DTDD = (DWORD)p->lValue;
		memcpy((BYTE*)&FTDD,(BYTE*)&DTDD,4);
		FTDD = pCfg->lA*FTDD/pCfg->lB+pCfg->lC;
		memcpy((BYTE*)&pDD->lValue,(BYTE*)&FTDD,4);
	}
    else
      pDD->lValue=p->lValue;
    pDD->byFlag=p->byFlag;
    pDD->byFlag&=(~0x80);     /*时标无效*/
    p++;   
  }
   
  smMGive(dwDDSem);
}  

/*------------------------------------------------------------------------
 Procedure: 	WriteDDT ID:1
 Purpose:		写离散点电度(带时标和一字节标志)
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteDDFT(WORD wEqpID,WORD wNum, struct VDBDDFT *buf)
{
  struct VTrueEqp *pTEqp;
  struct VDBDDFT * p=buf;
  struct VTrueDD *pDD;
  struct VTrueDDCfg *pCfg;
  DWORD DTDD;
  float FTDD;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)	return;

  if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL))  return;

  maxno=pTEqp->Cfg.wDDNum-1;
  if (maxno<0)	return;

  smMTake(dwDDSem);		  
  
  for (i=0;i<wNum;i++)
  {
	if (p->wNo>maxno)
	{
	  p++;
	  continue;
	} 
	
	pDD=pTEqp->pDD+p->wNo;	 
	pCfg=pTEqp->pDDCfg+p->wNo;
	
	if(pCfg->dwCfg & 0x80000000) //闭锁
	{
		continue;
	}		
	
	if (pTEqp->Cfg.dwFlag&DDQUOTIETYENABLED)
	{
		DTDD = (DWORD)p->lValue;
		memcpy((BYTE*)&FTDD,(BYTE*)&DTDD,4);
		FTDD = pCfg->lA*FTDD/pCfg->lB+pCfg->lC;
		memcpy((BYTE*)&pDD->lValue,(BYTE*)&FTDD,4);
	}
	else
	  pDD->lValue=p->lValue;
	pDD->byFlag=p->byFlag;
	pDD->byFlag|=0x80;	  /*时标有效*/	
	pDD->Time=p->Time;

#ifdef DB_EPOWER_STAT
    MaxXL_Stat(wEqpID,p);
#endif
	p++;   
  }
   
  smMGive(dwDDSem);
}  

/*------------------------------------------------------------------------
 Procedure: 	WriteDD ID:1
 Purpose:		写离散点电度(无标志)
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteDD(WORD wEqpID,WORD wNum, struct VDBDD *buf)
{
  struct VTrueEqp *pTEqp;
  struct VDBDD * p=buf;
  struct VTrueDD *pDD;
  struct VTrueDDCfg *pCfg;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)	return;

  if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL))  return;

  maxno=pTEqp->Cfg.wDDNum-1;
  if (maxno<0)	return;

  smMTake(dwDDSem);		  
  
  for (i=0;i<wNum;i++)
  {
	if (p->wNo>maxno)
	{
	  p++;
	  continue;
	} 
	
	pDD=pTEqp->pDD+p->wNo;	 
	pCfg=pTEqp->pDDCfg+p->wNo;
	
	if(pCfg->dwCfg & 0x80000000) //闭锁
	{
		continue;
	}
	
	if (pTEqp->Cfg.dwFlag&DDQUOTIETYENABLED)
	  pDD->lValue=pCfg->lA*p->lValue/pCfg->lB+pCfg->lC;
	else
	  pDD->lValue=p->lValue;
	pDD->byFlag&=(~0x80);	  /*时标无效*/
	p++;   
  }
   
  smMGive(dwDDSem);
}  

/*------------------------------------------------------------------------
 Procedure:     WriteRangeDDF ID:1
 Purpose:       写连续点电度(带一字节标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeDDF(WORD wEqpID,WORD wBeginNo,WORD wNum, struct VDDF *buf)
{
  struct VTrueEqp *pTEqp;
  struct VDDF * p=buf;
  struct VDBDDF DBDDF;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL)) return;

  maxno=pTEqp->Cfg.wDDNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBDDF.wNo=i+wBeginNo;
    if (DBDDF.wNo>maxno)
    {
       p++;
       continue;
    } 
    DBDDF.lValue=p->lValue;
    DBDDF.byFlag=p->byFlag;
    WriteDDF(wEqpID,1,&DBDDF);         
    p++;
  }  
}  

/*------------------------------------------------------------------------
 Procedure: 	WriteRangeDDT ID:1
 Purpose:		写连续点电度(带时标)
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeDDFT(WORD wEqpID,WORD wBeginNo,WORD wNum, struct VDDFT *buf)
{
  struct VTrueEqp *pTEqp;
  struct VDDFT * p=buf;
  struct VDBDDFT DBDDFT;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL)) return;

  maxno=pTEqp->Cfg.wDDNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
	DBDDFT.wNo=i+wBeginNo;
	if (DBDDFT.wNo>maxno)
	{
	   p++;
	   continue;
	} 
	DBDDFT.lValue=p->lValue;
	DBDDFT.byFlag=p->byFlag;
	DBDDFT.Time=p->Time;
	WriteDDFT(wEqpID,1,&DBDDFT);		   
	p++;
  }  
}  

/*------------------------------------------------------------------------
 Procedure:     WriteRangeDD ID:1
 Purpose:       写连续点电度(无标志)
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteRangeDD(WORD wEqpID,WORD wBeginNo,WORD wNum, long *buf)
{
  struct VTrueEqp *pTEqp;
  long * p=buf;
  struct VDBDD DBDD;
  int i,maxno;

  if (wEqpID>=*g_Sys.Eqp.pwNum) return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL) return;

  if ((pTEqp->pDD==NULL)||(pTEqp->pDDCfg==NULL)) return;

  maxno=pTEqp->Cfg.wDDNum-1;
  if (maxno<0) return;

  for (i=0;i<wNum;i++)
  {
    DBDD.wNo=i+wBeginNo;
    if (DBDD.wNo>maxno)
    {
       p++;
       continue;
    } 
    DBDD.lValue=*p;
    WriteDD(wEqpID,1,&DBDD);         
    p++;
  }  
}  


/******************************** read **********************************/


/*------------------------------------------------------------------------
 Procedure:     ReadDDF ID:1
 Purpose:       读离散点电度(带一字节标志)
 Input:         wNum: 要读的个数
 Output:        实际所读的个数   	
 Errors:
------------------------------------------------------------------------*/
WORD ReadDDF(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDDF *buf)
{
  if (wBufLen<sizeof(struct VDBDDF)) return(0);
  
  return(atomicReadDDF(wEqpID,wNum,wBufLen,buf,TRUE));
}

/*------------------------------------------------------------------------
 Procedure: 	ReadDDT ID:1
 Purpose:		读离散点电度(带时标)
 Input: 		wNum: 要读的个数
 Output:		实际所读的个数		
 Errors:
------------------------------------------------------------------------*/
WORD ReadDDFT(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDDFT *buf)
{
  if (wBufLen<sizeof(struct VDBDDFT)) return(0);
  
  return(atomicReadDDFT(wEqpID,wNum,wBufLen,buf,TRUE));
}

/*------------------------------------------------------------------------
 Procedure:     ReadDD ID:1
 Purpose:       读离散点电度(无标志)
 Input:         wNum: 要读的个数
 Output:        实际所读的个数   	
 Errors:
------------------------------------------------------------------------*/
WORD ReadDD(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDD *buf)
{
  if (wBufLen<sizeof(struct VDBDD)) return(0);
  
  return(atomicReadDD(wEqpID,wNum,wBufLen,buf,TRUE));
}


/*------------------------------------------------------------------------
 Procedure:     ReadRangeDDF ID:1
 Purpose:       读连续点电度(带一字节标志) 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeDDF(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VDDF *buf)
{
  int i,j,k;
  struct VDBDDF DBDDF;
  struct VDDF *p=buf;

  if (wBufLen<sizeof(struct VDDF)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBDDF.wNo=i;
    if (atomicReadDDF(wEqpID,1,sizeof(struct VDBDDF),&DBDDF,TRUE)==1)
    {
      p->lValue=DBDDF.lValue;
      p->byFlag=DBDDF.byFlag;
      p++;
      j++;
      k+=sizeof(struct VDDF);
      if (k>wBufLen-sizeof(struct VDDF)) break;
    } 
    else
      break;	
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure: 	ReadRangeDDF ID:1
 Purpose:		读连续点电度(带时标和一字节标志)
 Input: 		wNum: 要读的个数
				wBeginNo:起始点号
 Output:		实际所读的个数			
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeDDFT(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VDDFT *buf)
{
  int i,j,k;
  struct VDBDDFT DBDDFT;
  struct VDDFT *p=buf;

  if (wBufLen<sizeof(struct VDDFT)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
	DBDDFT.wNo=i;
	if (atomicReadDDFT(wEqpID,1,sizeof(struct VDBDDFT),&DBDDFT,TRUE)==1)
	{
	  p->lValue=DBDDFT.lValue;
	  p->byFlag=DBDDFT.byFlag;
	  p->Time=DBDDFT.Time;
	  p++;
	  j++;
	  k+=sizeof(struct VDDFT);
	  if (k>wBufLen-sizeof(struct VDDFT)) break;
	} 
	else
	  break;	
  }  

  return(j);
}

/*------------------------------------------------------------------------
 Procedure:     ReadRangeDD ID:1
 Purpose:       读连续点电度(无标志) 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeDD(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,long *buf)
{
  int i,j,k;
  struct VDBDD DBDD;
  long *p=buf;
  BYTE* p_temp = (BYTE*)p;

  if (wBufLen<sizeof(long)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBDD.wNo=i;
    if (atomicReadDD(wEqpID,1,sizeof(struct VDBDD),&DBDD,TRUE)==1)
    {
	  memcpy(p_temp,(BYTE*)&DBDD.lValue,sizeof(long));
      p++;
	  p_temp = (BYTE*)p;
      j++;
      k+=sizeof(long);
      if (k>wBufLen-sizeof(long)) break;
    } 
    else
      break;	
  }  

  return(j);
}


/*------------------------------------------------------------------------
 Procedure:     ReadRangeAllDD ID:1
 Purpose:       读连续点所有电度（无标志，不考虑其是否发送，维护用） 
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllDD(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,long *buf)
{
  int i,j,k;
  struct VDBDD DBDD;
  long *p=buf;
  BYTE* p_temp = (BYTE*)p;

  if (wBufLen<sizeof(long)) return(0);
  
  j=k=0;
  for (i=wBeginNo;i<(wBeginNo+wNum);i++)
  {
    DBDD.wNo=i;
    if (atomicReadDD(wEqpID,1,sizeof(struct VDBDD),&DBDD,FALSE)==1)
    {
      memcpy(p_temp,(BYTE*)&DBDD.lValue,sizeof(long));
      p++;
	  p_temp = (BYTE*) p;
      j++;
      k+=sizeof(long);
      if (k>wBufLen-sizeof(long)) break;
    } 
    else
      break;	
  }  

  return(j);
}


/****************************** read YK ID ****************************/

/*------------------------------------------------------------------------
 Procedure:     ReadRangeAllYK ID:1
 Purpose:       读连续点所有遥控ID
 Input:         wNum: 要读的个数
                wBeginNo:起始点号
 Output:        实际所读的个数   		
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllYK(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf)
{
  WORD j;
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;

  if (wBufLen<sizeof(WORD)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
    if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
      return(0);
    else
    {
      if (pVEqp->pYK==NULL)
        return(0);
      else
        j=atomicReadVEYK(pVEqp,wBeginNo,wNum,wBufLen,buf);
    }   
  } 
  else
  {
    if ((pTEqp->pYK==NULL)||(pTEqp->pYKCfg==NULL))
      return(0);
    else
      j=atomicReadTEYK(pTEqp,wBeginNo,wNum,wBufLen,buf);
  }  

  return(j);  
}


/****************************** read YT ID ****************************/

/*------------------------------------------------------------------------
 Procedure: 	ReadRangeAllYY ID:1
 Purpose:		读连续点所有遥调ID
 Input: 		wNum: 要读的个数
				wBeginNo:起始点号
 Output:		实际所读的个数			
 Errors:
------------------------------------------------------------------------*/
WORD ReadRangeAllYT(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf)
{
  WORD j;
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;

  if (wBufLen<sizeof(WORD)) return(0);
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)
  {
	if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)==NULL)  
	  return(0);
	else
	{
	  if (pVEqp->pYT==NULL)
		return(0);
	  else
		j=atomicReadVEYT(pVEqp,wBeginNo,wNum,wBufLen,buf);
	}	
  } 
  else
  {
	if ((pTEqp->pYT==NULL)||(pTEqp->pYTCfg==NULL))
	  return(0);
	else
	  j=atomicReadTEYT(pTEqp,wBeginNo,wNum,wBufLen,buf);
  }  

  return(j);  
}

/****************************** write TSData ****************************/

/*------------------------------------------------------------------------
 Procedure:     WriteTSData ID:1
 Purpose:       写透明数据 
 Input:         
 Output:        
 Errors:
------------------------------------------------------------------------*/
void WriteTSData(WORD wEqpID,WORD wBufLen,BYTE *buf,struct VCalClock *pTime)
{
  struct VTrueEqp *pTEqp;
  struct VTSDataCfg *pTSDataCfg;  
  //struct VDBCOS DBCOS;
  struct VDBSOE DBSOE;
  struct VDBYX DBYX;
  int i,j;
  BOOL find;
  WORD no,offset;
  BYTE value;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pTEqp->pTSDataCfg==NULL)||(pTEqp->Cfg.wTSDataNum==0)) return;
  
  for (i=0;i<pTEqp->Cfg.wTSDataNum;i++)
  {
    find=TRUE;
    pTSDataCfg=pTEqp->pTSDataCfg+i;	
    for (j=0;j<pTSDataCfg->wKeyWordNum;j++)
    {
      offset=pTSDataCfg->awOffset[j];
	  if (offset>=wBufLen) 
	  {
		find=FALSE;
		continue;
	  }	
      if (pTSDataCfg->abyKeyWord[j]!=buf[offset])
      {
        find=FALSE;
        break;
      }  
    }  
    if (find)
    {
       if (pTSDataCfg->dwCfg&0x01)
       {
         if (pTSDataCfg->dwCfg&0x02)
           value=0x81;  //he
         else
           value=0x1;   //fen
         no=pTSDataCfg->wVYXNo+pTEqp->Cfg.wSYXNum;
         DBYX.wNo=no;
         if (ReadSYX(wEqpID,1,sizeof(DBYX),&DBYX))
         {
           if (DBYX.byValue!=value)
           {
             /*DBCOS.wNo=no;
             DBCOS.byValue=value;
             WriteSCOS(wEqpID,1,&DBCOS);*/   
			 DBYX.wNo=no;
			 DBYX.byValue=value;
			 WriteSYX(wEqpID,1,&DBYX);
             DBSOE.wNo=no;
             DBSOE.byValue=value;
             if (pTime==NULL) GetSysClock(&DBSOE.Time,CALCLOCK);
             else DBSOE.Time=*pTime;             	
             WriteSSOE(wEqpID,1,&DBSOE);   
           }  
         }  
       }  
    }   
  }           
}  


/*************************** write CommStatus  *************************/

/*------------------------------------------------------------------------
 Procedure:     WriteCommStatus ID:1
 Purpose:       写通讯状态 
 Input:         bStatus:TRUE //合,通
                        FALSE //分,断
 Output:        
 Errors:
------------------------------------------------------------------------*/
void WriteCommStatus(WORD wTaskID,WORD wEqpID,BOOL bStatus)
{
  struct VTrueEqp *pTEqp;
  struct VDBCOS DBCOS;
  struct VDBSOE DBSOE;
  //struct VDBYX DBYX;
  struct VEqpRunInfo *pRunInfo;
  int no;
  BYTE value;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)==NULL)  return;

  if ((pRunInfo=GetpRunInfo(wTaskID,wEqpID, NULL))!=NULL)
  {
    if (bStatus)
      pRunInfo->CommRunInfo.wState|=COMMOK;
    else
    {
      pRunInfo->CommRunInfo.wState&=(~COMMOK);
      pRunInfo->CommRunInfo.dwBreakNum++;
    }  
  }
  
  no=pTEqp->Cfg.wSYXNum-1;
  if (no < 0) return;
  
  if (bStatus)
    value=0x81;
  else
  	value=0x01;

  DBCOS.wNo=(WORD)no;
  DBCOS.byValue=value;
  atomicWriteSCOS(wEqpID,1,&DBCOS,TRUE,TRUE,TRUE);   
  /*DBYX.wNo=(WORD)no;
  DBYX.byValue=value;
  atomicWriteSYX(wEqpID,1,&DBYX,TRUE,TRUE,FALSE,FALSE);*/
  DBSOE.wNo=(WORD)no;
  DBSOE.byValue=value;
  GetSysClock(&DBSOE.Time,CALCLOCK);
  atomicWriteSSOE(wEqpID,1,&DBSOE,TRUE,TRUE,TRUE); 
}  

/*------------------------------------------------------------------------
 Procedure:     ReadCommStatus ID:1
 Purpose:       读通讯状态 
 Input:         
 Output:        TRUE //合,通
                FALSE //分,断
 Errors:
------------------------------------------------------------------------*/
BOOL ReadCommStatus(WORD wTaskID,WORD wEqpID)
{
  struct VEqpRunInfo *pRunInfo;

  if ((pRunInfo=GetpRunInfo(wTaskID, wEqpID, NULL))==NULL)  return(FALSE);
  
  if (pRunInfo->CommRunInfo.wState&COMMOK)  return(TRUE);
  else return(FALSE);
}

/***************************  FA Info  *********************************/

/******************************   write  *******************************/

/*------------------------------------------------------------------------
 Procedure:     WriteFAInfo ID:1
 Purpose:       写FA信息 
 Input:         
 Output:        
 Errors:
------------------------------------------------------------------------*/
void WriteFAInfo(WORD wNum,struct VFAProcInfo *buf)
{
  struct VFAProcInfo *p=buf,*q;
  int i;

  if ((g_Sys.pFAProc->wPoolNum==0)||(g_Sys.pFAProc->pPoolHead==NULL)) return;

  smMTake(g_Sys.pFAProc->dwSem);
  
  for (i=0;i<wNum;i++)
  {
    q=g_Sys.pFAProc->pPoolHead+g_Sys.pFAProc->wWritePtr;    
    memcpy(q,p,sizeof(struct VFAProcInfo));
    g_Sys.pFAProc->wWritePtr++;
    g_Sys.pFAProc->wWritePtr%=g_Sys.pFAProc->wPoolNum;
    p++;	
#ifdef INCLUDE_B2F_EVT
	WritePollBuf2File((struct VPool *)g_Sys.pFAProc, (BYTE *)q, 0, SYSEV_FLAG_FA);
#endif	   		 	
  } 

  for (i=0;i<*g_Sys.Eqp.pwNum;i++)
  {
    if (g_Sys.Eqp.pInfo[i].pEqpGInfo!=NULL)
      if ((g_Sys.Eqp.pInfo[i].pEqpGInfo->Cfg.dwFlag&0x01) == 0)  //send FA
        writeFlag(i,FAINFOUFLAG);
  }   
  	
  smMGive(g_Sys.pFAProc->dwSem);  
}  

/******************************    read   *******************************/

/*------------------------------------------------------------------------
 Procedure:     ReadFAInfo ID:1
 Purpose:       读FA信息 
 Input:         wNum: 要读的个数
                pwReadPtr:函数会自动修正错误读指针
 Output:        实际所读的个数		 
 Errors:
------------------------------------------------------------------------*/
WORD ReadFAInfo(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VFAProcInfo *buf)
{
  struct VFAProcInfo *p=buf,*q;
  WORD rp,wp,poolnum,num,*pwReadPtr;
  int i,j,k,flag;
  struct VEqpRunInfo *pRunInfo;
  
  if (wBufLen<sizeof(struct VFAProcInfo)) return(0);
  
  if ((g_Sys.pFAProc->wPoolNum==0)||(g_Sys.pFAProc->pPoolHead==NULL)) return(0);
  
  if (GetReadPtr(wTaskID,wEqpID,FAINFOUFLAG,&pwReadPtr)==ERROR)  return(0);
  
  smMTake(g_Sys.pFAProc->dwSem);

  wp=g_Sys.pFAProc->wWritePtr;
  poolnum=g_Sys.pFAProc->wPoolNum;
  //*pwReadPtr%=poolnum;
  rp=*pwReadPtr;

  num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
  
  if (num==0)
  {					
	pRunInfo=GetpRunInfo(wTaskID, wEqpID, &flag);
	if ((pRunInfo!=NULL) && (pRunInfo->dwUFlag&FAINFOUFLAG))  //必须判断,减少写文件调用
	{
		pRunInfo->dwUFlag&=(~FAINFOUFLAG);	
#ifdef INCLUDE_B2F_F
		WriteRunInfo2File(wTaskID, pRunInfo, FAINFOUFLAG, flag);
#endif
	}	

	smMGive(g_Sys.pFAProc->dwSem);
	return(0);     
  }
	
  if (num>wNum)  num=wNum;
  
  j=k=0;

  for (i=rp;i<(rp+num);i++)
  {
    q=g_Sys.pFAProc->pPoolHead+(i % poolnum);      
	if (q->Time.dwMinute == 0) 
	{
		(*pwReadPtr)++;
		continue;
	}
	memcpy(p,q,sizeof(struct VFAProcInfo));
    p++; 
    j++;
    k+=sizeof(struct VFAProcInfo);
    if (k>wBufLen-sizeof(struct VFAProcInfo)) break;      
  }
   	
  smMGive(g_Sys.pFAProc->dwSem);  

  return(j);
}  


/*------------------------------------------------------------------------
 Procedure:     QueryFAInfoWtrOffset ID:1
 Purpose:       查询FA信息写偏移 
 Input:         pwWriteOffset:写指针偏移(相对于pwPoolLen)
 Output:        		 	 
 Errors:
------------------------------------------------------------------------*/
void QueryFAInfoWtrOffset(WORD *pwWriteOffset,WORD correctOffset)
{  
  if (g_Sys.pFAProc->wWritePtr<correctOffset)
    //*pwWriteOffset=g_Sys.pFAProc->wPoolNum-correctOffset+g_Sys.pFAProc->wWritePtr;
    *pwWriteOffset=0;
  else  
    *pwWriteOffset=g_Sys.pFAProc->wWritePtr-correctOffset;  
}  


/*------------------------------------------------------------------------
 Procedure:     QueryFAInfo ID:1
 Purpose:       查询FA信息 
 Input:         wNum: 要读的个数
                wReadOffset: 读偏移(相对于pwPoolLen)
                pwWriteOffset:写指针偏移(相对于pwPoolLen)
                pwPoolLen: FA pool num
 Output:        实际所读的个数		 	 
 Errors:
------------------------------------------------------------------------*/
WORD QueryFAInfo(WORD wTaskID,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VFAProcInfo *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  struct VFAProcInfo *p=buf,*q;
  WORD num;
  int i,j,k;
  
  *pwWriteOffset=g_Sys.pFAProc->wWritePtr;
  *pwPoolLen=g_Sys.pFAProc->wPoolNum;

  if (wBufLen<sizeof(struct VFAProcInfo)) return(0);
  
  if ((g_Sys.pFAProc->wPoolNum==0)||(g_Sys.pFAProc->pPoolHead==NULL)) return(0);
  
  if (wReadOffset>=*pwPoolLen)
    return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    q=g_Sys.pFAProc->pPoolHead+(i % *pwPoolLen);  
	if (q->Time.dwMinute == 0) break;
    memcpy(p,q,sizeof(struct VFAProcInfo));
    p++; 
    j++;
    k+=sizeof(struct VFAProcInfo);
    if (k>wBufLen-sizeof(struct VFAProcInfo)) break;      
  }
   	
  return(j);
}  


WORD GetSysEventMmiPtr(struct VSysEvent *pEvent)
{
	return pEvent->wMmiPtr;
}

void IncreaseSysEventMmiPtr(struct VSysEvent *pEvent, WORD wNum)
{
	pEvent->wMmiPtr = (pEvent->wMmiPtr+wNum)%pEvent->wPoolNum;
}

/*------------------------------------------------------------------------
 Procedure:     WriteSysEvent ID:1
 Purpose:       写系统记录
 Input:         
 Output:        
 Errors:
------------------------------------------------------------------------*/
void WriteSysEvent(struct VSysEvent *pEvent, int flag, WORD wNum, struct VSysEventInfo *buf)
{
  struct VSysEventInfo *p=buf,*q;
  int i;
  
  if ((pEvent->wPoolNum==0)||(pEvent->pPoolHead==NULL)) return;

  smMTake(pEvent->dwSem);

  for (i=0;i<wNum;i++)
  {
    q=pEvent->pPoolHead+pEvent->wWritePtr;    
    memcpy(q,p,sizeof(struct VSysEventInfo));
    pEvent->wWritePtr++;
    pEvent->wWritePtr%=pEvent->wPoolNum;
    p++;
#ifdef INCLUDE_B2F_EVT
	if(flag != SYSEV_FLAG_ULOG)
		WritePollBuf2File((struct VPool *)pEvent, (BYTE *)q, 0, flag);
#endif
#ifdef INCLUDE_HIS
	if(flag == SYSEV_FLAG_ULOG)
		his2file_write((BYTE*)q,g_Sys.wIOEqpID,SYSEV_FLAG_ULOG);
#endif
#ifdef INCLUDE_HIS_TTU
    if(flag == SYSEV_FLAG_ULOG)
      histtu2file_write((BYTE*)q,g_Sys.wIOEqpID,SYSEV_FLAG_ULOG);
#endif

  } 
  	
  smMGive(pEvent->dwSem);  
}  

void WriteActEvent(struct VCalClock *ptm, DWORD type, int fd, const char *fmt, ... )
{
	struct VSysEventInfo event;
    va_list varg;
	int i;

	va_start( varg, fmt );
	vsprintf(event.msg, fmt, varg );
	va_end( varg );

	
	if (ptm == NULL) 
		GetSysClock(&event.time, CALCLOCK);
	else
		event.time = *ptm;
	event.type = type;
	event.para = (WORD) fd;
	event.rsv = 0;
	WriteSysEvent(g_Sys.pActEvent, SYSEV_FLAG_ACT, 1, &event);

	for (i=0;i<*g_Sys.Eqp.pwNum;i++)
	{
	    if (g_Sys.Eqp.pInfo[i].pEqpGInfo!=NULL)
		    if ((g_Sys.Eqp.pInfo[i].pEqpGInfo->Cfg.dwFlag&0x04) == 0)  //send Act
		        writeFlag(i, ACTEVENTUFLAG);
	}	

	#ifdef INCLUDE_WEB_SERVICE
	evSend(WEBACT_ID, EV_UFLAG);
	#endif
	evSend(MMI_ID, EV_UFLAG);
}

void WriteDoEvent(struct VCalClock *ptm, int para, const char *fmt, ... )
{
	struct VSysEventInfo event;
    va_list varg;	
	//int i;

	va_start( varg, fmt );
	vsprintf(event.msg, fmt, varg );
	va_end( varg );

	if (ptm == NULL) 
		GetSysClock(&event.time, CALCLOCK);
	else
		event.time = *ptm;
	event.type = 0;
	event.para = (WORD)para;
	event.rsv = 0;
	WriteSysEvent(g_Sys.pDoEvent, SYSEV_FLAG_DO, 1, &event);
	
	/*for (i=0;i<*g_Sys.Eqp.pwNum;i++)
	{
	    if (g_Sys.Eqp.pInfo[i].pEqpGInfo!=NULL)
		    if ((g_Sys.Eqp.pInfo[i].pEqpGInfo->Cfg.dwFlag&0x08) == 0)  //send Do
		        writeFlag(i, DOEVENTUFLAG);
	}*/	
}

void WriteWarnEvent(struct VCalClock *ptm, DWORD errcode, int para, const char *fmt, ... )
{
	struct VSysEventInfo event;
    va_list varg;
	//int i;


	va_start( varg, fmt );
	vsprintf(event.msg, fmt, varg );
	va_end( varg );

	if (ptm == NULL) 
		GetSysClock(&event.time, CALCLOCK);
	else
		event.time = *ptm;	
	event.type = errcode;
	event.para = (WORD)para;
	event.rsv = 0;
	WriteSysEvent(g_Sys.pWarnEvent, SYSEV_FLAG_WARN, 1, &event);
	
	/*for (i=0;i<*g_Sys.Eqp.pwNum;i++)
	{
	    if (g_Sys.Eqp.pInfo[i].pEqpGInfo!=NULL)
		    if ((g_Sys.Eqp.pInfo[i].pEqpGInfo->Cfg.dwFlag&0x10) == 0)  //send Warn
		        writeFlag(i, WARNEVENTUFLAG);
	}*/	
	
	if (errcode) SysSetErr(errcode);
	evSend(MMI_ID, EV_UFLAG);	
}

void WriteUlogEvent(struct VCalClock *ptm, DWORD type, int value, const char *fmt, ... )
{
	struct VSysEventInfo event;
    va_list varg;
	//int i;


	va_start( varg, fmt );
	vsprintf(event.msg, fmt, varg );
	va_end( varg );

	if (ptm == NULL) 
		GetSysClock(&event.time, CALCLOCK);
	else
		event.time = *ptm;	
	event.type = type;
	event.para = (WORD)value;
	event.rsv = 0;
	WriteSysEvent(g_Sys.pUlogEvent, SYSEV_FLAG_ULOG, 1, &event);
	
}

void WriteFlowEvent(struct VCalClock *ptm, DWORD type, int value, struct VSysFlowEventInfo *pflow)
{
	struct VSysFlowEventInfo *q;
	
	if((g_Sys.pFlowEvent->wPoolNum ==0) || (g_Sys.pFlowEvent->pPoolHead == NULL)) return;
	smMTake(g_Sys.pFlowEvent->dwSem);

	q = g_Sys.pFlowEvent->pPoolHead + g_Sys.pFlowEvent->wWritePtr;
	memcpy(q,pflow,sizeof(struct VSysFlowEventInfo));
	g_Sys.pFlowEvent->wWritePtr = (g_Sys.pFlowEvent->wWritePtr + 1)%g_Sys.pFlowEvent->wPoolNum;

#ifdef INCLUDE_HIS
	his2file_write((BYTE*)q,g_Sys.wIOEqpID,SYSEV_FLAG_FLOW);
#endif
#ifdef INCLUDE_HIS_TTU
    histtu2file_write((BYTE*)q,g_Sys.wIOEqpID,SYSEV_FLAG_FLOW);
#endif
	smMGive(g_Sys.pFlowEvent->dwSem);
}

void WriteFaEvent(struct VCalClock *ptm, const char *fmt, ... )
{
	struct VSysEventInfo event;
    va_list varg;

	va_start( varg, fmt );
	vsprintf(event.msg, fmt, varg );
	va_end( varg );

	if (ptm == NULL) 
		GetSysClock(&event.time, CALCLOCK);
	else
		event.time = *ptm;	
	event.type = 0;
	event.para = 0;
	event.rsv = 0;
	WriteSysEvent(g_Sys.pFaEvent, SYSEV_FLAG_FA, 1, &event);
	
}

WORD ReadSysEvent(struct VSysEvent *pEvent, DWORD dwUFlag, WORD wTaskID, WORD wEqpID, WORD wNum, WORD wBufLen, struct VSysEventInfo *buf)
{
	struct VSysEventInfo *p=buf,*q;
	WORD rp,wp,poolnum,num,*pwReadPtr;
	int i,j,k,flag;
	struct VEqpRunInfo *pRunInfo;

	if (wBufLen<sizeof(struct VSysEventInfo)) return(0);

	if ((pEvent->wPoolNum==0)||(pEvent->pPoolHead==NULL)) return(0);

	if (wEqpID>=*g_Sys.Eqp.pwNum)  return(0);

	if (GetReadPtr(wTaskID, wEqpID, dwUFlag, &pwReadPtr) == ERROR)  return(0);

	smMTake(pEvent->dwSem);
  
	wp=pEvent->wWritePtr;
	poolnum=pEvent->wPoolNum;
	//*pwReadPtr%=poolnum;
	rp=*pwReadPtr;
  
	num=(wp>=rp) ? (wp-rp) : (poolnum-rp+wp);
	
	if (num==0)
	{				  
	  pRunInfo=GetpRunInfo(wTaskID, wEqpID, &flag);
	  if ((pRunInfo!=NULL) && (pRunInfo->dwUFlag&dwUFlag))	//必须判断,减少写文件调用
	  {
		  pRunInfo->dwUFlag&=(~dwUFlag);  
#ifdef INCLUDE_B2F_F
		  WriteRunInfo2File(wTaskID, pRunInfo, dwUFlag, flag);
#endif
	  }   
  
	  smMGive(pEvent->dwSem);
	  return(0);	 
	}
	  
	if (num>wNum)  num=wNum;
	
	j=k=0;
  
	for (i=rp;i<(rp+num);i++)
	{
	  q=pEvent->pPoolHead+(i % poolnum); 	 
	  if (q->time.dwMinute == 0) 
	  {
		  (*pwReadPtr)++;
		  continue;
	  }
	  memcpy(p,q,sizeof(struct VSysEventInfo));
	  p++; 
	  j++;
	  k+=sizeof(struct VSysEventInfo);
	  if (k>wBufLen-sizeof(struct VSysEventInfo)) break;		
	}
	  
	smMGive(pEvent->dwSem);	
  
	return(j);
}

WORD ReadActEvent(WORD wTaskID, WORD wEqpID, WORD wNum, WORD wBufLen, struct VSysEventInfo *buf)
{
	return (ReadSysEvent(g_Sys.pActEvent, ACTEVENTUFLAG, wTaskID, wEqpID, wNum, wBufLen, buf));
}

WORD ReadDoEvent(WORD wTaskID, WORD wEqpID, WORD wNum, WORD wBufLen, struct VSysEventInfo *buf)
{
	return (ReadSysEvent(g_Sys.pDoEvent, DOEVENTUFLAG, wTaskID, wEqpID, wNum, wBufLen, buf));
}

WORD ReadWarnEvent(WORD wTaskID, WORD wEqpID, WORD wNum, WORD wBufLen, struct VSysEventInfo *buf)
{
	return (ReadSysEvent(g_Sys.pWarnEvent, WARNEVENTUFLAG, wTaskID, wEqpID, wNum, wBufLen, buf));
}

WORD ReadFaEvent(WORD wTaskID, WORD wEqpID, WORD wNum, WORD wBufLen, struct VSysEventInfo *buf)
{
	return (ReadSysEvent(g_Sys.pFaEvent, FAEVENTUFLAG, wTaskID, wEqpID, wNum, wBufLen, buf));
}
/*------------------------------------------------------------------------
 Procedure:     QuerySysEventWtrOffset ID:1
 Purpose:       查询sysevent写偏移 
 Input:         pwWriteOffset:写指针偏移(相对于pwPoolLen)
 Output:        		 	 
 Errors:
------------------------------------------------------------------------*/
void QuerySysEventWtrOffset(struct VSysEvent *pEvent, WORD *pwWriteOffset,WORD correctOffset)
{  
  if (pEvent->wWritePtr<correctOffset)
    //*pwWriteOffset=pEvent->wPoolNum-correctOffset+pEvent->wWritePtr;
    *pwWriteOffset=0;
  else  
    *pwWriteOffset=pEvent->wWritePtr-correctOffset;  
}  

/*------------------------------------------------------------------------
 Procedure:     QuerySysEventInfo ID:1
 Purpose:       查询SysEvent信息 
 Input:         wNum: 要读的个数
                wReadOffset: 读偏移(相对于pwPoolLen)
                pwWriteOffset:写指针偏移(相对于pwPoolLen)
                pwPoolLen: FA pool num
 Output:        实际所读的个数		 	 
 Errors:
------------------------------------------------------------------------*/
WORD QuerySysEventInfo(struct VSysEvent *pEvent,WORD wNum,WORD wReadOffset,WORD wBufLen, struct VSysEventInfo *buf,WORD *pwWriteOffset,WORD *pwPoolLen)
{
  struct VSysEventInfo *p=buf,*q;
  WORD num;
  int i,j,k;
  
  *pwWriteOffset=pEvent->wWritePtr;
  *pwPoolLen=pEvent->wPoolNum;

  if (wBufLen<sizeof(struct VSysEventInfo)) return(0);
  
  if ((pEvent->wPoolNum==0)||(pEvent->pPoolHead==NULL)) return(0);
  
  if (wReadOffset>=*pwPoolLen)
    return(0); 	
  	
  num=wNum;
  if ((wReadOffset+num)>*pwPoolLen)
  	num=*pwPoolLen-wReadOffset;

  j=k=0;
  for (i=wReadOffset;i<(wReadOffset+num);i++)
  {
    q=pEvent->pPoolHead+(i % *pwPoolLen); 
	if (q->time.dwMinute == 0) break;
    memcpy(p,q,sizeof(struct VSysEventInfo));
    p++; 
    j++;
    k+=sizeof(struct VSysEventInfo);
    if (k>wBufLen-sizeof(struct VSysEventInfo)) break;      
  }
   	
  return(j);
}  

void ClearSysEvent(struct VSysEvent *pEvent, int flag)
{
	smMTake(pEvent->dwSem);
	PoolReset((struct VPool *)pEvent);
#ifdef INCLUDE_B2F_EVT
	PoolReset_B2F((struct VPool *)pEvent, 0, flag);
#endif	   		 		  	
	smMGive(pEvent->dwSem);  
}  

void SysEventReset(void)
{
	ClearSysEvent(g_Sys.pActEvent, SYSEV_FLAG_ACT);
	ClearSysEvent(g_Sys.pDoEvent, SYSEV_FLAG_DO);
	ClearSysEvent(g_Sys.pWarnEvent, SYSEV_FLAG_WARN);
}  

void SysSoeReset(void)
{
    int i,j;
	WORD taskid,egid;
	struct VEqpInfo *pInfo;

	for (i=0; i<*g_Sys.Eqp.pwNum; i++)
	{
	    pInfo = g_Sys.Eqp.pInfo+i;

		if (pInfo->pTEqpInfo != NULL)
		{
		   PoolReset((struct VPool *)&(pInfo->pTEqpInfo->DSOE));
	       PoolReset((struct VPool *)&(pInfo->pTEqpInfo->DCOS));
           PoolReset((struct VPool *)&(pInfo->pTEqpInfo->SSOE));
           PoolReset((struct VPool *)&(pInfo->pTEqpInfo->SCOS));
#ifdef INCLUDE_B2F_SOE
		   PoolReset_B2F((struct VPool *)&(pInfo->pTEqpInfo->DSOE), i, SYSEV_FLAG_DSOE);
		   PoolReset_B2F((struct VPool *)&(pInfo->pTEqpInfo->SSOE), i, SYSEV_FLAG_SSOE);
#endif
#ifdef INCLUDE_B2F_COS
		   PoolReset_B2F((struct VPool *)&(pInfo->pTEqpInfo->DCOS), i, SYSEV_FLAG_DCOS);
		   PoolReset_B2F((struct VPool *)&(pInfo->pTEqpInfo->SCOS), i, SYSEV_FLAG_SCOS);
#endif
			for( j = 0;j < pInfo->pTEqpInfo->Cfg.wInvRefEGNum;j++)
			{
				egid = pInfo->pTEqpInfo->pwInvRefEGID[j];
				taskid = g_Sys.Eqp.pInfo[egid].wTaskID;
				evSend(taskid, EV_CLEAR_SOE);
			}
		}
		else if (pInfo->pVEqpInfo != NULL)
		{
		   PoolReset((struct VPool *)&(pInfo->pVEqpInfo->DSOE));
	       PoolReset((struct VPool *)&(pInfo->pVEqpInfo->DCOS));
           PoolReset((struct VPool *)&(pInfo->pVEqpInfo->SSOE));
           PoolReset((struct VPool *)&(pInfo->pVEqpInfo->SCOS));
		   PoolReset((struct VPool *)&(pInfo->pVEqpInfo->pCO));	 				 
#ifdef INCLUDE_B2F_SOE   
		   PoolReset_B2F((struct VPool *)&(pInfo->pVEqpInfo->DSOE), i, SYSEV_FLAG_DSOE);
		   PoolReset_B2F((struct VPool *)&(pInfo->pVEqpInfo->SSOE), i, SYSEV_FLAG_SSOE);
#endif
#ifdef INCLUDE_B2F_COS
		   PoolReset_B2F((struct VPool *)&(pInfo->pVEqpInfo->DCOS), i, SYSEV_FLAG_DCOS);
		   PoolReset_B2F((struct VPool *)&(pInfo->pVEqpInfo->SCOS), i, SYSEV_FLAG_SCOS);
#endif
			for( j = 0;j < pInfo->pVEqpInfo->Cfg.wInvRefEGNum;j++)
			{
				egid = pInfo->pVEqpInfo->pwInvRefEGID[j];
				taskid = g_Sys.Eqp.pInfo[egid].wTaskID;
				evSend(taskid, EV_CLEAR_SOE);
			}
		}
	}
}

/***************************  Eqp Info  ***************************/

/*------------------------------------------------------------------------
 Procedure:     ReadEqpGEqpNum ID:1
 Purpose:       取得装置组管理的装置个数
 Input:         
 Output:        装置个数
 Errors:
------------------------------------------------------------------------*/
STATUS ReadEqpGEqpNum(WORD wEqpGID,WORD *pwNum)
{
  struct VEqpGroup *pGEqp;
  
  if (wEqpGID>=*g_Sys.Eqp.pwNum) return(ERROR);

  if ((pGEqp=g_Sys.Eqp.pInfo[wEqpGID].pEqpGInfo)==NULL) return(ERROR);
  
  *pwNum=pGEqp->Cfg.wEqpNum;
  
  return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     ReadEqpInfo ID:1
 Purpose:       取得装置的信息
 Input:         
 Output:        OK,ERROR
                pEqpInfo的首字(wType)为装置的类型。
                wType==TRUEEQP: pEqpInfo为VTVEqpInfo结构信息
                wType==VIRTUALEQP: pEqpInfo为VTVEqpInfo结构信息
                wType==GROUPEQP: pEqpInfo为VEqpGroupInfo结构信息                
 Errors:
------------------------------------------------------------------------*/
STATUS ReadEqpInfo(WORD wTaskID,WORD wEqpID,void *pEqpInfo,BOOL asSend)
{
  struct VTrueEqp *pTEqp;
  struct VVirtualEqp *pVEqp;
  struct VEqpGroup *pGEqp;
  
  if (wEqpID>=*g_Sys.Eqp.pwNum)  return(ERROR);

  if ((pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)!=NULL)
    return(ReadTrueEqpInfo(wTaskID,wEqpID,pTEqp,(struct VTVEqpInfo *)pEqpInfo,asSend));
  else  if ((pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)!=NULL)
    return(ReadVirtualEqpInfo(wTaskID,wEqpID,pVEqp,(struct VTVEqpInfo *)pEqpInfo));
  else  if ((pGEqp=g_Sys.Eqp.pInfo[wEqpID].pEqpGInfo)!=NULL)
    return(ReadEqpGroupInfo(wTaskID,wEqpID,pGEqp,(struct VEqpGroupInfo *)pEqpInfo));

  return(ERROR);
}


/***************************  UFlag Info  ***************************/

/*------------------------------------------------------------------------
 Procedure:     TestFlag ID:1
 Purpose:       测试UFlag
 Input:
 Output:        
 Errors:
------------------------------------------------------------------------*/
BOOL TestFlag(WORD wTaskID,WORD wEqpID,DWORD dwUFlag)
{
   struct VEqpRunInfo *pRunInfo;

   if ((pRunInfo=GetpRunInfo(wTaskID,wEqpID,NULL))==NULL)  return(FALSE);
   
   if (pRunInfo->dwUFlag&dwUFlag) 
   	 return(TRUE);
   else 
   	 return(FALSE);
}


/*------------------------------------------------------------------------
 Procedure:     ClearFlag ID:1
 Purpose:       清除UFlag
 Input:
 Output:        
 Errors:
------------------------------------------------------------------------*/
void ClearFlag(WORD wTaskID,WORD wEqpID,DWORD dwUFlag)
{
   struct VEqpRunInfo *pRunInfo;
   int flag;
#ifdef INCLUDE_B2F_F 
   DWORD uflag;
#endif

   if ((pRunInfo=GetpRunInfo(wTaskID,wEqpID,&flag))==NULL)  return;

#ifdef INCLUDE_B2F_F 
   uflag = pRunInfo->dwUFlag;
#endif

   switch (dwUFlag)
   {
     case DSOEUFLAG:
       if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
       {
         if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->DSOE.wWritePtr==pRunInfo->wDSOEReadPtr)
           pRunInfo->dwUFlag&=(~dwUFlag);
       }  
       else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
       {
         if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->DSOE.wWritePtr==pRunInfo->wDSOEReadPtr)
           pRunInfo->dwUFlag&=(~dwUFlag);
       }  
       else
         pRunInfo->dwUFlag&=(~dwUFlag);
       break;
     case DCOSUFLAG:
       if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
       {
         if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->DCOS.wWritePtr==pRunInfo->wDCOSReadPtr)
           pRunInfo->dwUFlag&=(~dwUFlag);
       }  
       else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
       {
         if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->DCOS.wWritePtr==pRunInfo->wDCOSReadPtr)
           pRunInfo->dwUFlag&=(~dwUFlag);
       }  
       else
         pRunInfo->dwUFlag&=(~dwUFlag);
       break;
     case SSOEUFLAG:
       if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
       {
         if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->SSOE.wWritePtr==pRunInfo->wSSOEReadPtr)
         {
           pRunInfo->dwUFlag&=(~dwUFlag);
         }			
       }  
       else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
       {
         if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->SSOE.wWritePtr==pRunInfo->wSSOEReadPtr)
         {
           pRunInfo->dwUFlag&=(~dwUFlag);
         }  
       }  
       else
         pRunInfo->dwUFlag&=(~dwUFlag);
       break;
     case SCOSUFLAG:
       if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
       {
         if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->SCOS.wWritePtr==pRunInfo->wSCOSReadPtr)
           pRunInfo->dwUFlag&=(~dwUFlag);
       }  
       else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
       {
         if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->SCOS.wWritePtr==pRunInfo->wSCOSReadPtr)
           pRunInfo->dwUFlag&=(~dwUFlag);
       }  
       else
         pRunInfo->dwUFlag&=(~dwUFlag);
       break;
     case FAINFOUFLAG:
       if (g_Sys.pFAProc->wWritePtr==pRunInfo->wFAInfoReadPtr)
       {
       	 pRunInfo->dwUFlag&=(~dwUFlag);
       } 
       break;
     case ACTEVENTUFLAG:
	   if (g_Sys.pActEvent->wWritePtr==pRunInfo->wActEventReadPtr)
	   {
	     pRunInfo->dwUFlag&=(~dwUFlag);
	   } 
	   break;
	 case DOEVENTUFLAG:
	   if (g_Sys.pDoEvent->wWritePtr==pRunInfo->wDoEventReadPtr)
	   {
		 pRunInfo->dwUFlag&=(~dwUFlag);
	   } 
	   break;
     case WARNEVENTUFLAG:
	   if (g_Sys.pWarnEvent->wWritePtr==pRunInfo->wWarnEventReadPtr)
	   {
		  pRunInfo->dwUFlag&=(~dwUFlag);
	   } 
	   break;
	 default:
	 	pRunInfo->dwUFlag &= (~dwUFlag);
		break;
   }    

#ifdef INCLUDE_B2F_F 
   if (uflag != pRunInfo->dwUFlag)   //减少文件调用
       WriteRunInfo2File(wTaskID, pRunInfo, dwUFlag, flag);
#endif   	   	
}


/*------------------------------------------------------------------------
 Procedure:     IncreaseReadPtr ID:1
 Purpose:       读指针增加
 Input:         wNum:增加个数
 Output:        
 Errors:
------------------------------------------------------------------------*/
void ReadPtrIncrease(WORD wTaskID,WORD wEqpID,WORD wNum,DWORD dwUFlag)
{
   struct VEqpRunInfo *pRunInfo;
   WORD poolnum=0;
   	
   if ((pRunInfo=GetpRunInfo(wTaskID,wEqpID,NULL))==NULL)  return;

   switch (dwUFlag)
   {
   	   case DSOEUFLAG:
   	     smMTake(dwDYXSem); 
  	     pRunInfo->wDSOEReadPtr+=wNum;
		 if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
			 poolnum=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->DSOE.wPoolNum;
		 else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
			 poolnum=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->DSOE.wPoolNum;	
		 if (poolnum!=0)  pRunInfo->wDSOEReadPtr%=poolnum;
   	     smMGive(dwDYXSem);
   	     break;
       case DCOSUFLAG:
   	     smMTake(dwDYXSem); 
   	     pRunInfo->wDCOSReadPtr+=wNum;
		 if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
			 poolnum=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->DCOS.wPoolNum;
		 else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
			 poolnum=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->DCOS.wPoolNum;	
		 if (poolnum!=0)  pRunInfo->wDCOSReadPtr%=poolnum;
   	     smMGive(dwDYXSem);
   	     break;
   	   case SSOEUFLAG:
   	     smMTake(dwSYXSem); 
		 pRunInfo->wSSOEReadPtr+=wNum;
		 if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
			 poolnum=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->SSOE.wPoolNum;
		 else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
			 poolnum=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->SSOE.wPoolNum;	
		 if (poolnum!=0)  pRunInfo->wSSOEReadPtr%=poolnum;
   	     smMGive(dwSYXSem);
   	     break;
   	   case SCOSUFLAG:
   	     smMTake(dwSYXSem); 
		 pRunInfo->wSCOSReadPtr+=wNum;
		 if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
			 poolnum=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->SCOS.wPoolNum;
		 else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
			 poolnum=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->SCOS.wPoolNum;	
		 if (poolnum!=0)  pRunInfo->wSCOSReadPtr%=poolnum;
   	     smMGive(dwSYXSem);
   	     break;
   	   case FAINFOUFLAG:
   	     smMTake(g_Sys.pFAProc->dwSem);
   	     pRunInfo->wFAInfoReadPtr+=wNum;
		 poolnum=g_Sys.pFAProc->wPoolNum;
		 if (poolnum!=0)  pRunInfo->wFAInfoReadPtr%=poolnum;
   	     smMGive(g_Sys.pFAProc->dwSem);
   	     break;
	   case ACTEVENTUFLAG:
	     smMTake(g_Sys.pActEvent->dwSem);
	     pRunInfo->wActEventReadPtr+=wNum;
	     poolnum=g_Sys.pActEvent->wPoolNum;
	     if (poolnum!=0)	pRunInfo->wActEventReadPtr%=poolnum;
	     smMGive(g_Sys.pActEvent->dwSem);
	     break;
	   case DOEVENTUFLAG:
		 smMTake(g_Sys.pDoEvent->dwSem);
		 pRunInfo->wDoEventReadPtr+=wNum;
		 poolnum=g_Sys.pDoEvent->wPoolNum;
		 if (poolnum!=0)	  pRunInfo->wDoEventReadPtr%=poolnum;
		 smMGive(g_Sys.pDoEvent->dwSem);
		 break;
	   case WARNEVENTUFLAG:
		 smMTake(g_Sys.pWarnEvent->dwSem);
		 pRunInfo->wWarnEventReadPtr+=wNum;
		 poolnum=g_Sys.pWarnEvent->wPoolNum;
		 if (poolnum!=0)	   pRunInfo->wWarnEventReadPtr%=poolnum;
		 smMGive(g_Sys.pWarnEvent->dwSem);
		 break;
   }   	

   ClearFlag(wTaskID,wEqpID,dwUFlag);   
}

/*------------------------------------------------------------------------
 Procedure: 	GetPoolPtr ID:1
 Purpose:		GetPoolPtr
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS GetPoolPtr(WORD wTaskID,WORD wEqpID,DWORD dwUFlag,WORD *pwReadPtr,WORD *pwWritePtr)
{
   struct VEqpRunInfo *pRunInfo;

   *pwWritePtr=0;
   if ((pRunInfo=GetpRunInfo(wTaskID,wEqpID,NULL))==NULL)  return(ERROR);

   switch (dwUFlag)
   {
	   case DSOEUFLAG:
		 *pwReadPtr=pRunInfo->wDSOEReadPtr;		 
		 if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
			 *pwWritePtr=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->DSOE.wWritePtr;
		 else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
			 *pwWritePtr=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->DSOE.wWritePtr;	
		 break;
	   case DCOSUFLAG:
		 *pwReadPtr=pRunInfo->wDCOSReadPtr;
		 if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
			 *pwWritePtr=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->DCOS.wWritePtr;
		 else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
			 *pwWritePtr=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->DCOS.wWritePtr; 
		 break;
	   case SSOEUFLAG:
		 *pwReadPtr=pRunInfo->wSSOEReadPtr;
		 if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
			 *pwWritePtr=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->SSOE.wWritePtr;
		 else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
			 *pwWritePtr=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->SSOE.wWritePtr; 
		 break;
	   case SCOSUFLAG:
		 *pwReadPtr=pRunInfo->wSCOSReadPtr;
		 if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo!=NULL)
			 *pwWritePtr=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo->SCOS.wWritePtr;
		 else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo!=NULL)
			 *pwWritePtr=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo->SCOS.wWritePtr; 
		 break;
	   case FAINFOUFLAG:
		 *pwReadPtr=pRunInfo->wFAInfoReadPtr;
		 *pwWritePtr=g_Sys.pFAProc->wWritePtr;
		 break;
 	   case ACTEVENTUFLAG:
 	     *pwReadPtr=pRunInfo->wActEventReadPtr;
		 *pwWritePtr=g_Sys.pActEvent->wWritePtr;
 	     break;
 	   case DOEVENTUFLAG:
 	     *pwReadPtr=pRunInfo->wDoEventReadPtr;
 	     *pwWritePtr=g_Sys.pDoEvent->wWritePtr;
 	     break;
	   case WARNEVENTUFLAG:
	     *pwReadPtr=pRunInfo->wWarnEventReadPtr;
	     *pwWritePtr=g_Sys.pWarnEvent->wWritePtr;
	     break;
	   default:
		 return(ERROR);
   }	
   
   return(OK);
}

/***************************  Systme Info  ***************************/

/*------------------------------------------------------------------------
 Procedure:     GetEqpID ID:1
 Purpose:       取得装置ID
 Input:         dwName:装置名
                pID:指向ID指针
 Output:        
 Errors:
------------------------------------------------------------------------*/
STATUS GetEqpID(DWORD dwName, WORD *pID)
{
  WORD i;

  for (i=0;i<*g_Sys.Eqp.pwNum;i++)
    if (g_Sys.Eqp.pInfo[i].dwName==dwName)
    {
      *pID=i;
      return(OK);
    }

  return(ERROR);
} 

/*------------------------------------------------------------------------
 Procedure:     GetEqpIDByCfgName ID:1
 Purpose:       取得装置ID
 Input:         CfgName:装置设置名
                pID:指向ID指针
 Output:        
 Errors:
------------------------------------------------------------------------*/
STATUS GetEqpIDByCfgName(char *sCfgName, WORD *pID)
{
  WORD i;

  for (i=0;i<*g_Sys.Eqp.pwNum;i++)
    if (strcmp(g_Sys.Eqp.pInfo[i].sCfgName, sCfgName) == 0)
    {
      *pID=i;
      return(OK);
    }

  return(ERROR);
} 

/*------------------------------------------------------------------------
 Procedure:     GetEqpIDByAddr ID:1
 Purpose:       取得装置ID
 Input:         wAddress:装置地址
                pID:指向ID指针
 Output:        
 Errors:
 add by lqh 20060331
------------------------------------------------------------------------*/
STATUS GetEqpIDByAddr(WORD wAddress, WORD *pID)
{
	WORD i;

	for (i=0;i<*g_Sys.Eqp.pwNum;i++)
		if(g_Sys.Eqp.pInfo[i].pTEqpInfo != NULL)
		{
			if (g_Sys.Eqp.pInfo[i].pTEqpInfo->Cfg.wDesAddr == wAddress)
			{
				*pID=i;
				return(OK);
			}
		}

	return(ERROR);
} 

/*------------------------------------------------------------------------
 Procedure:     GetTaskID ID:1
 Purpose:       取得taskID
 Input:         wEqpID:装置ID
                pwTaskID:指向ID指针
 Output:        
 Errors:
------------------------------------------------------------------------*/
STATUS GetTaskID(const WORD wEqpID,WORD * pwTaskID)
{
  if (wEqpID>=*g_Sys.Eqp.pwNum) return(ERROR);

  *pwTaskID=g_Sys.Eqp.pInfo[wEqpID].wTaskID;

  return(OK);
} 


/*------------------------------------------------------------------------
 Procedure:     GetCommCfg ID:1
 Purpose:       取得端口配置
 Input:
 Output:        
 Errors:
------------------------------------------------------------------------*/
struct VComm *GetCommCfg(const WORD wTaskID)
{
 if (wTaskID>THREAD_MAX_NUM) return(NULL);
    
 return(&g_Task[wTaskID].CommCfg);
}


/*------------------------------------------------------------------------
 Procedure:     GetTaskInfo ID:1
 Purpose:       取得任务信息
 Input:
 Output:        
 Errors:
------------------------------------------------------------------------*/
struct VTask *GetTaskInfo(const WORD wTaskID)
{
 if (wTaskID>THREAD_MAX_NUM) return(NULL);
    
 return(g_Task+wTaskID);
}


/*------------------------------------------------------------------------
 Procedure:     GetTaskEqpNum ID:1
 Purpose:       取得任务管理的装置个数
 Input:         
 Output:        装置个数
 Errors:
------------------------------------------------------------------------*/
WORD GetTaskEqpNum(WORD wTaskID)
{
  if (wTaskID>THREAD_MAX_NUM) return(0);

  if (g_Task[wTaskID].wEqpGroupNum)
    return(g_Task[wTaskID].wEqpGroupNum);
  else
  	return(g_Task[wTaskID].wEqpNum);  
}

/*------------------------------------------------------------------------
 Procedure: 	GetTaskTVEqpNum ID:1
 Purpose:		取得任务管理的装置组个数
 Input: 		
 Output:		装置个数
 Errors:
------------------------------------------------------------------------*/
WORD GetTaskTVEqpNum(WORD wTaskID)
{
  if (wTaskID>THREAD_MAX_NUM) return(0);

  return(g_Task[wTaskID].wEqpNum);  
}

/*------------------------------------------------------------------------
 Procedure: 	GetTaskTVEqpNum ID:1
 Purpose:		取得任务管理的实装置或虚装置个数
 Input: 		
 Output:		装置个数
 Errors:
------------------------------------------------------------------------*/
WORD GetTaskEqpGroupNum(WORD wTaskID)
{
  if (wTaskID>THREAD_MAX_NUM) return(0);

  return(g_Task[wTaskID].wEqpGroupNum);
}

/*------------------------------------------------------------------------
 Procedure:     GetEqpID ID:1
 Purpose:       取得任务管理的装置ID
 Input:         
 Output:        装置ID
 Errors:
------------------------------------------------------------------------*/
WORD GetTaskEqpID(WORD wTaskID,WORD *pwEqpID)
{
  int i;
  
  if (wTaskID>THREAD_MAX_NUM) return(0);

  if (g_Task[wTaskID].wEqpGroupNum)
  {
    for (i=0;i<g_Task[wTaskID].wEqpGroupNum;i++)
      pwEqpID[i]=g_Task[wTaskID].pEqpGroupRunInfo[i].wEqpID;
    return(g_Task[wTaskID].wEqpGroupNum);
  }  	
  else
  {
    for (i=0;i<g_Task[wTaskID].wEqpNum;i++)
      pwEqpID[i]=g_Task[wTaskID].pEqpRunInfo[i].wEqpID;
    return(g_Task[wTaskID].wEqpNum);
  }  
}

/*------------------------------------------------------------------------
 Procedure: 	GetEqpID ID:1
 Purpose:		取得任务管理的实装置或虚装置ID
 Input: 		
 Output:		装置ID
 Errors:
------------------------------------------------------------------------*/
WORD GetTaskTVEqpID(WORD wTaskID,WORD *pwEqpID)
{
  int i;
  
  if (wTaskID>THREAD_MAX_NUM) return(0);

  for (i=0;i<g_Task[wTaskID].wEqpNum;i++)
	  pwEqpID[i]=g_Task[wTaskID].pEqpRunInfo[i].wEqpID;
  return(g_Task[wTaskID].wEqpNum);
}

/*------------------------------------------------------------------------
 Procedure:     GetTaskEqpIDList ID:1
 Purpose:       取得任务管理的装置ID列表
 Input:         
 Output:        装置ID列表指针
 Errors:
------------------------------------------------------------------------*/
WORD *GetTaskEqpIDList(WORD wTaskID)
{  
  if (wTaskID>THREAD_MAX_NUM) return(NULL);

  if (g_Task[wTaskID].pwGroupEqpIDList!=NULL)
  	return(g_Task[wTaskID].pwGroupEqpIDList);
  else
    return(g_Task[wTaskID].pwEqpIDList);
}

/*------------------------------------------------------------------------
 Procedure: 	GetTaskTVEqpIDList ID:1
 Purpose:		取得任务管理的实装置或虚装置ID列表
 Input: 		
 Output:		装置ID列表指针
 Errors:
------------------------------------------------------------------------*/
WORD *GetTaskTVEqpIDList(WORD wTaskID)
{  
  if (wTaskID>THREAD_MAX_NUM) return(NULL);

  return(g_Task[wTaskID].pwEqpIDList);
}


/***************************  Message  Info  *********************************/

/*------------------------------------------------------------------------
 Procedure:     TaskSendMsg ID:1
 Purpose:       任务发送msg
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void TaskSendMsg(WORD wToThID,WORD wMyThID,WORD wEqpID,BYTE byMsgID,BYTE byMsgAttr,WORD wDataLen,struct VMsg *pMessage)
{
  if (wToThID==wMyThID) return;  
    
  pMessage->Head.wLength=wDataLen+sizeof(struct VMsgHead);
  pMessage->Head.byMsgID=byMsgID;
  pMessage->Head.byMsgAttr=byMsgAttr;
  pMessage->Head.wThID=wMyThID;
  pMessage->Head.wEqpID=wEqpID;
  msgSend(wToThID,(void *)pMessage,pMessage->Head.wLength,TRUE);
} 

/***************************  Restart  *********************************/

/*------------------------------------------------------------------------
 Procedure:     sysRestart ID:1
 Purpose:       
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void sysRestart(DWORD type, DWORD code,const char *cause)
{
    struct VSysResetInfo *pInfo;

	pInfo = &g_Sys.ResetInfo;

	*g_Sys.pdwRestart = type;
	pInfo->code = code;
	strncpy(pInfo->cause, cause, GEN_NAME_LEN-1);
	pInfo->cause[GEN_NAME_LEN-1] = '\0';
	GetSysClock(&pInfo->clock, CALCLOCK);
	extNvRamSet(NVRAM_AD_RESET, (BYTE *)pInfo, sizeof(struct VSysResetInfo));
	reboot(0);
}

#if 0
/***************************  Route Info  ***************************/

/*------------------------------------------------------------------------
 Procedure:     UpRouteByEqpID ID:1
 Purpose:       通过装置ID上行路由
 Input:
 Output:        
 Errors:
------------------------------------------------------------------------*/  
void UpRouteByEqpID(const WORD wEqpID,struct VMsg *pMsg)
{
  WORD wTaskID;
  VTrueEqp *pTEqp;
  VVirtualEqp *pVEqp;
  int i;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;
  
  if (g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo)
  {
    pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo;
    for (i=0;i<pTEqp->Cfg.wInvRefEGNum;i++)
    {
      GetTaskID(pTEqp->pwInvRefEGID[i],&wTaskID);
      msgSend(wTaskID,pMsg,pMsg->Head.wLength,TRUE);
    } 
  }   
  else if (g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo)
  {
    pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo;
    for (i=0;i<pVEqp->Cfg.wInvRefEGNum;i++)
    {
      GetTaskID(pVEqp->pwInvRefEGID[i],&wTaskID);
      msgSend(wTaskID,pMsg,pMsg->Head.wLength,TRUE);
    } 
  }   
  else if (g_Sys.Eqp.pInfo[wEqpID].pEqpGInfo)
  {
    GetTaskID(wEqpID,&wTaskID);
    msgSend(wTaskID,pMsg,pMsg->Head.wLength,TRUE);
  } 
}  


/*------------------------------------------------------------------------
 Procedure:     UpRoute ID:1
 Purpose:       上行路由
 Input:         如果wAddr==0xFFFF,表示发给所有的上行任务。
 Output:        
 Errors:
------------------------------------------------------------------------*/   
void UpRoute(const WORD wAddr,struct VMsg *pMsg)
{
  WORD i;

  for (i=0;i<*g_Sys.Eqp.pwNum;i++)
  {
    if (g_Sys.Eqp.pInfo[i].pTEqpInfo)
    {
      if (wAddr==g_Sys.Eqp.pInfo[i].pTEqpInfo->Cfg.wDesAddr)
        UpRouteByEqpID(i,pMsg);
    }
    else if (g_Sys.Eqp.pInfo[i].pVEqpInfo)
    {
      if (wAddr==g_Sys.Eqp.pInfo[i].pVEqpInfo->Cfg.wDesAddr)
        UpRouteByEqpID(i,pMsg);
    }
    else if (g_Sys.Eqp.pInfo[i].pEqpGInfo)
    {
      if ((wAddr==0xFFFF)||(wAddr==g_Sys.Eqp.pInfo[i].pEqpGInfo->Cfg.wDesAddr))
        UpRouteByEqpID(i,pMsg);
    }
  } 
}

/*------------------------------------------------------------------------
 Procedure: 	DownRoute ID:1
 Purpose:		通过装置ID下行路由
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void DownRouteByEqpID(const WORD wEqpID,struct VMsg *pMsg)
{
  WORD wTaskID;

  if (wEqpID>=*g_Sys.Eqp.pwNum)  return;

  pMsg->Head.wEqpID=wEqpID;	
  GetTaskID(wEqpID,&wTaskID);
  msgSend(wTaskID,pMsg,pMsg->Head.wLength,TRUE);
} 

/*------------------------------------------------------------------------
 Procedure: 	DownRoute ID:1
 Purpose:		绝对下行路由[只根据参数，不参考通信状态]
 Input:         如果wAddr==0xFFFF,表示发给所有实装置的下行任务。
 Output:		
 Errors:
------------------------------------------------------------------------*/
void AbsDownRoute(const WORD wAddr,struct VMsg *pMsg)
{
  WORD i;
  
  for (i=0;i<*g_Sys.Eqp.pwNum;i++)
  {
	if (g_Sys.Eqp.pInfo[i].pTEqpInfo)
	{
	  if ((wAddr==0xFFFF)||(wAddr==g_Sys.Eqp.pInfo[i].pTEqpInfo->Cfg.wDesAddr))
		DownRouteByEqpID(i,pMsg);
	}
	else if (g_Sys.Eqp.pInfo[i].pVEqpInfo)
	{
	  if (wAddr==g_Sys.Eqp.pInfo[i].pVEqpInfo->Cfg.wDesAddr)
		DownRouteByEqpID(i,pMsg);
	}
	else if (g_Sys.Eqp.pInfo[i].pEqpGInfo)
	{
	  if (wAddr==g_Sys.Eqp.pInfo[i].pEqpGInfo->Cfg.wDesAddr)
		DownRouteByEqpID(i,pMsg);
	}
  } 
} 

STATUS devtypeshow(void)
{
	printf("模拟板卡型号:%x.", g_Sys.byAio_Type);
	printf("数字板卡型号:%x.", g_Sys.byDio_Type);	  
	printf("参数设备型号:%x.", g_Sys.MyCfg.dwType);

	return OK;
};	

void GetAllFormatIP(char *normalformatIP, char *allformatIP)
{
  char ipcell[5],*p,*q;
  int i,k;

  strcpy(allformatIP,"");
  p=normalformatIP;  
  for (i=0;i<4;i++)
  {
	q=p;
	while ((*p!='.')&&(*p!='\0'))
	   p++;
	k=atoi(q);
	sprintf(ipcell,"%03d",k);
	strcat(allformatIP,ipcell);
	if (*p=='.') 
	{
      strcat(allformatIP,".");
	  p++;
	}  
  }	
}

void GetNormalFormatIP(char *allformatIP,char *normalformatIP)
{
  char ipcell[5],*p,*q;
  int i,k;

  strcpy(normalformatIP,"");
  p=allformatIP;  
  for (i=0;i<4;i++)
  {
	q=p;
	while ((*p!='.')&&(*p!='\0'))
	   p++;
	k=atoi(q);
	sprintf(ipcell,"%d",k);
	strcat(normalformatIP,ipcell);
	if (*p=='.') 
	{
	  strcat(normalformatIP,".");
	  p++;
	}  
  } 
}

STATUS vershow(void)
{
	int i;
	struct VLdFileTbl *pTbl;

    pTbl = g_Sys.LdFileTbl;
	for (i=0; i<g_Sys.LdFileNum; i++)
	{
		printf("%03d 模块\n", i+1);
		printf("    名称:%s\n", pTbl->name);
		printf("    版本号:%s\n", pTbl->ver);
		printf("    用户:%s\n", pTbl->user);
		printf("    校验码:0x%08x\n", pTbl->crc);
		
		pTbl++;
	}

	return OK;
}
#endif



