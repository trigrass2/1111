#include "pcol.h"
#include "maint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "para.h"
#include "sys.h"

#ifdef INCLUDE_PR
extern BYTE prTableBuf[MAX_TABLE_FILE_SIZE];
extern DWORD g_prlubo;
#endif

WORD CheckSum(BYTE *buf, int l)
{
	int i;
	BYTE *p=buf;
	WORD tmp=0;

	for(i=0; i<l; i++)
	{
		tmp+=*p;
		p++;
	}  

	return(tmp);
}

static DWORD SearchOneFrame(BYTE *Buf,int Len)
{
	VMaintFrame *pHead;
	int len1, len2;
	WORD crc1,crc2;

	if (Len < sizeof(VMaintFrame))  return(PCOL_FRAME_LESS);
		
	pHead = (VMaintFrame *)Buf;

	if ((pHead->StartCode1!= MAINT_CODE) || (pHead->StartCode2 != MAINT_CODE))  return(PCOL_FRAME_ERR|1);	

	len1 = (pHead->Len1High<<8)|pHead->Len1Low;

	len2 = (pHead->Len2High<<8)|pHead->Len2Low;

	if (len1 != len2) return(PCOL_FRAME_ERR|1);

	if (len1 > MAINT_LEN_MAX) return(PCOL_FRAME_ERR|1);

	len2 = len1 + 6;

	if (Len < (len2+3)) return(PCOL_FRAME_LESS);

	crc1 = (Buf[len2+1]<<8)|Buf[len2];
	crc2 = CheckSum(&(pHead->Address), len1);

	if (crc1 !=crc2) return(PCOL_FRAME_ERR|1);

	return(PCOL_FRAME_OK|(len2+3));	
}


static void SendFrameHead(VMaint *pMaint, WORD cmd)
{
	VMaintFrame *pSend = (VMaintFrame *)pMaint->Col.Send.pBuf;

	pMaint->Col.Send.wReadPtr  = 0;

	pSend->StartCode1 = MAINT_CODE;
	pSend->StartCode2 = MAINT_CODE;

  	
	pSend->Address = 0;
	pSend->CMD = HIBYTE(cmd)|0x80;
	pSend->afn = LOBYTE(cmd);

    pMaint->Col.Send.wWritePtr = sizeof(VMaintFrame);
}

static void SendWordLH(VMaint *pMaint,WORD wData)
{
	BYTE *pSend = (BYTE*)pMaint->Col.Send.pBuf;
	
	pSend[pMaint->Col.Send.wWritePtr++] = LOBYTE(wData);
	pSend[pMaint->Col.Send.wWritePtr++] = HIBYTE(wData);
}

static void SendDWordLH(VMaint *pMaint, DWORD dwData)
{
	BYTE *pSend = (BYTE*)pMaint->Col.Send.pBuf;

	pSend[pMaint->Col.Send.wWritePtr++]  = LOBYTE(LOWORD(dwData));
	pSend[pMaint->Col.Send.wWritePtr++]  = HIBYTE(LOWORD(dwData));
	pSend[pMaint->Col.Send.wWritePtr++]  = LOBYTE(HIWORD(dwData));
	pSend[pMaint->Col.Send.wWritePtr++]  = HIBYTE(HIWORD(dwData));
}


static void SendFrameTail(VMaint *pMaint)
{  
	WORD crcCode;
    VMaintFrame *pSend = (VMaintFrame *)pMaint->Col.Send.pBuf;

    pSend->Len2High = pSend->Len1High = HIBYTE(pMaint->Col.Send.wWritePtr-6);
	pSend->Len2Low = pSend->Len1Low = LOBYTE(pMaint->Col.Send.wWritePtr-6);
	

	crcCode = CheckSum((BYTE*)&(pSend->Address), (pMaint->Col.Send.wWritePtr-6));
	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++] = LOBYTE(crcCode);
	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++] = HIBYTE(crcCode);
	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++] = 0x16;
		
	DoSend(&pMaint->Col, 0);   
} 	

static void SendACK(VMaint *pMaint)
{
    SendFrameHead(pMaint, MAINT_ACK_OK);
	SendFrameTail(pMaint);
}

static void SendNACK(VMaint *pMaint, BYTE errcode)
{
    SendFrameHead(pMaint, MAINT_ACK_ERR);
	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]  = errcode;
	SendFrameTail(pMaint);
}

void WriteYkT_Bm(VMaint *pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
    BYTE *pData = (BYTE*)(pRec + 1);
	WORD time;

	time = MAKEWORD(pData[1], pData[2]);
	if(pData[0] == 0)
	{
		 WriteFzT(time);
	}
	else if(pData[0] == 1)
	{
		WriteHzT(time);
	}
	else
	{
		SendNACK(pMaint, MAINT_CMD_ERR);
		return;
	}
	SendACK(pMaint);
	return;
}

void WriteYxTime_BM(VMaint *pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
    BYTE *pData = (BYTE*)(pRec + 1);
	WORD time;

	time = MAKEWORD(pData[0], pData[1]);

	WriteYxFdT(time);
	
	SendACK(pMaint);
	return;
}

void WriteYcZero(VMaint *pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
    BYTE *pData = (BYTE*)(pRec + 1);
	WORD type,svalue;

	type = pData[0];
	svalue = MAKEWORD(pData[1], pData[2]);

	switch (type)
	{
		case 0:
			WriteULP(svalue);
		break;
		case 1:
			WriteILP(svalue);
		break;
		case 2:
			WritePLP(svalue);
		break;
		case 3:
			WriteFLP(svalue);
		break;
		case 4:
			WriteZLLP(svalue);
		break;
		case 5:
			WriteCosLP(svalue);
		break;
		default:
		break;
	}
		
	
	SendACK(pMaint);
	return;
}

void yk_BM(VMaint *pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
    BYTE *pData = (BYTE*)(pRec + 1);
	int type,srcid,id,value,time,len,ret;

	len = 0;
	type = (int)MAKEDWORD(MAKEWORD(pData[len+0], pData[len+1]), MAKEWORD(pData[len+2], pData[len+3]));
	len+=4;
	srcid = (int)MAKEDWORD(MAKEWORD(pData[len+0], pData[len+1]), MAKEWORD(pData[len+2], pData[len+3]));
	len+=4;
	id = (int)MAKEDWORD(MAKEWORD(pData[len+0], pData[len+1]), MAKEWORD(pData[len+2], pData[len+3]));
	len+=4;
	value = (int)MAKEDWORD(MAKEWORD(pData[len+0], pData[len+1]), MAKEWORD(pData[len+2], pData[len+3]));
	len+=4;
	time = (int)MAKEDWORD(MAKEWORD(pData[len+0], pData[len+1]), MAKEWORD(pData[len+2], pData[len+3]));

	shellprintf("收到遥控 %d %d %d %d %d \r\n",type,srcid,id,value,time);

	if(type == 0) //选择
		ret = ykSelect(srcid, id, value);
	else if(type == 1) //执行
		ret = ykOperate(srcid, id, value, time);
	else if(type == 2) //取消
		ret = ykCancel(srcid, id, value);
	else
	{
		SendACK(pMaint);
		return;
	}

	SendFrameHead(pMaint, YK_BM);
	SendDWordLH(pMaint, (DWORD)ret);
	SendFrameTail(pMaint);
}

void SetClock(VMaint *pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
    BYTE *pData = (BYTE*)(pRec + 1);
	struct VSysClock SysTime;
	
	SysTime.wYear		= MAKEWORD(pData[0], pData[1]);
	SysTime.byMonth   	= pData[2];
	SysTime.byDay     	= pData[3];
	SysTime.byWeek     	= pData[4];
	SysTime.byHour    	= pData[5];
	SysTime.byMinute  	= pData[6];
	SysTime.bySecond  	= pData[7];
	SysTime.wMSecond 	= MAKEWORD(pData[8],pData[9]);

	//填充写系统时钟函数调用
	SetSysClock((void *)&SysTime, SYSCLOCK);
	SendACK(pMaint);
	WriteDoEvent(NULL, 0, "linux对时 %d年%d月%d日%d时%d分%d秒%d毫秒  ",
				SysTime.wYear,SysTime.byMonth,SysTime.byDay,SysTime.byHour,SysTime.byMinute,SysTime.bySecond,SysTime.wMSecond);
	return;
}

void write_ycgain(VMaint *pMaint)
{
    int ret=OK;
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
    BYTE *pData = (BYTE*)(pRec + 1);
	BYTE total = pData[0];
	BYTE start = pData[1];
	BYTE num = pData[2];
#ifdef INCLUDE_YC
	memcpy((BYTE*)pMaint->byBuf+start*sizeof(VYcGain), pData+3, num*sizeof(VYcGain));

    if ((start+num) >= total)
	  ret = gainset((BYTE*)pMaint->byBuf);

	if (ret == OK)
	{
		SendACK(pMaint);
		WriteDoEvent(NULL, 0, "人工设置整定系数!");
	}
	else
#endif
	{
		SendNACK(pMaint,1);
		WriteDoEvent(NULL, 0, "人工设置整定系数失败!");
	}
}

void write_ycgainval(VMaint *pMaint)
{
	int ret=OK;
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
    BYTE *pData = (BYTE*)(pRec + 1);
	BYTE flag = pData[0];
	BYTE total = pData[1];
	BYTE start = pData[2];
	BYTE num = pData[3];

#ifdef INCLUDE_YC
	memcpy((BYTE*)pMaint->byBuf+start*sizeof(VYcVal), pData+4, num*sizeof(VYcVal));	
    if ((start+num) >= total)
	   ret = gainvalset(flag, total, (BYTE*)pMaint->byBuf);

	if (ret == OK)
	{
		SendACK(pMaint);
		WriteDoEvent(NULL, 0, "整定!");
	}
	else
#endif
	{
		SendNACK(pMaint,1);
		WriteDoEvent(NULL, 0, "整定失败!");
	}
}

void SendAC(VMaint *pMaint)            	
{
	BYTE ErrorCode=1;

#ifdef INCLUDE_YC
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	WORD Shift;
	WORD YCNum;
	WORD YCNumOffSet;
	DWORD EqpName;
	BYTE flag1;
	BYTE flag2;
	int RecYCNum;
	WORD*pwYC = NULL;
	
	EqpName = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	//EqpName=0x00010000;
	flag1=pData[4];
	flag2=pData[5];
    Shift = MAKEWORD(pData[6], pData[7]);//取遥测数
   
    SendFrameHead(pMaint,GET_accurve);
    SendDWordLH(pMaint,EqpName);
    //SendWordLH(Shift);
    //发送遥测的原始值
    YCNumOffSet = pMaint->Col.Send.wWritePtr; //记录AI个数存放位置
    pMaint->Col.Send.wWritePtr += 2;
	SendWordLH(pMaint,Shift);
	if(flag1==2)
        YCNum = meaRead_YcChnBuf( Shift, &flag2, &RecYCNum, (short  *)pMaint->byBuf,(MAINT_LEN_MAX-50)/2,-1);
    else
	      YCNum=meaRead_FdChnBuf1(Shift,&flag2,&RecYCNum, (short  *)pMaint->byBuf,(MAINT_LEN_MAX-50)/2);
    if ((YCNum == 0)|(YCNum>10))
    {
    	ErrorCode = 2;//have no yc or yc num = 0
    	SendNACK(pMaint,ErrorCode);
    	return;	
    }
 	SendWordLH(pMaint,RecYCNum);
   
    pwYC = (WORD   *)pMaint->byBuf;
	for(int j=0;j<YCNum;j++)
    for (int i = 0; i < RecYCNum; i++)
    {
    	SendWordLH(pMaint,*pwYC++);
    }
    pMaint->Col.Send.pBuf[YCNumOffSet++] = flag1;
    pMaint->Col.Send.pBuf[YCNumOffSet++] = flag2;
	SendFrameTail(pMaint);
#else
    SendNACK(pMaint,ErrorCode);
#endif
}	


void read_youxiaozhi(VMaint *pMaint)
{
#ifdef INCLUDE_YC
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	WORD SOENum;
	WORD ReadPtr;
	WORD len;
//	WORD BufLen;//环行缓冲池大小
	WORD RecSOENum;
	WORD SOENumOffSet;
	DWORD EqpName;
	//char temp[64];
	VMmiMeaValue mmiVal[10];


	int maxnum = sizeof(mmiVal)/sizeof(mmiVal[0]);

    EqpName = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
    SOENum = MAKEWORD(pData[4], pData[5]);
    ReadPtr = MAKEWORD(pData[6], pData[7]);
	
    
    SendFrameHead(pMaint,GET_YXZ);
	SendDWordLH(pMaint,EqpName);
    SOENumOffSet = pMaint->Col.Send.wWritePtr; //记录SOE个数存放位置
    pMaint->Col.Send.wWritePtr += 2;
    SendWordLH(pMaint,ReadPtr);
	RecSOENum=0;
//#if (TYPE_OS == OS_VXWORKS)
	    SendWordLH(pMaint,0);
	    SendWordLH(pMaint,0);
		
	if(SOENum > maxnum)
		SOENum = maxnum;
	SOENum=	meaRead_Mmi(ReadPtr/8, (ReadPtr%8), SOENum, mmiVal);
	for(int i=0;i<SOENum;i++)
	{
		if(!mmiVal[i].valid) continue;
		
		if ((mmiVal[i].tbl->unit[0]=='A') || (mmiVal[i].tbl->unit[0]=='H')) 
			sprintf((char*)pMaint->byBuf,"%s=%6.3f%s %6.1f°\r\n",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);
		else
			sprintf((char*)pMaint->byBuf,"%s=%6.3f%s %6.1f°\r\n",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);
			
		len=strlen((char*)pMaint->byBuf);
		memcpy(pMaint->Col.Send.pBuf+pMaint->Col.Send.wWritePtr ,pMaint->byBuf,len);
		pMaint->Col.Send.wWritePtr+=len;
		RecSOENum++;

	}
	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=0;

	pMaint->Col.Send.pBuf[SOENumOffSet++] = LOBYTE(RecSOENum);
    pMaint->Col.Send.pBuf[SOENumOffSet++] = HIBYTE(RecSOENum);
	SendFrameTail(pMaint);
#else
	SendNACK(pMaint,1);
#endif
}

void SendYC1(VMaint *pMaint)            	
{
	BYTE ErrorCode=1;	
	
#ifdef INCLUDE_YC
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	WORD Shift;
	WORD YCNum;
	WORD YCNumOffSet;
	DWORD EqpName;
	WORD RecYCNum;
	//struct VYCF_L*pwYC = NULL;
	VMeaYc *meaYc=NULL;
	BYTE *ptmp;
	EqpName = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	//EqpName=0x00010000;
	Shift = MAKEWORD(pData[4], pData[5]);//取偏移量
    YCNum = MAKEWORD(pData[6], pData[7]);//取遥测数

    if (YCNum > ((MAINT_LEN_MAX-50)/5))
    {
		YCNum = (MAINT_LEN_MAX-50)/5;
	}
    SendFrameHead(pMaint,GET_YC1);
    SendDWordLH(pMaint,EqpName);
    SendWordLH(pMaint,Shift);
    //发送遥测的原始值
    YCNumOffSet = pMaint->Col.Send.wWritePtr; //记录AI个数存放位置
    pMaint->Col.Send.wWritePtr += 2;
    RecYCNum = meaRead_Yc1( Shift, YCNum, (VMeaYc  *)pMaint->byBuf);
    
    if (RecYCNum == 0)
    {
    	ErrorCode = 2;//have no yc or yc num = 0
    	SendNACK(pMaint,ErrorCode);
    	return;	
    }
    
    meaYc = (VMeaYc   *)pMaint->byBuf;
    for (int i = 0; i < RecYCNum; i++)
    {
    	ptmp =(BYTE*) &meaYc[i].value;
    	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=ptmp[0];
    	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=ptmp[1];
    	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=ptmp[2];
    	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=ptmp[3];
	memcpy(	pMaint->Col.Send.pBuf+pMaint->Col.Send.wWritePtr,meaYc[i].unit,8);
	pMaint->Col.Send.wWritePtr+=8;
    }
    pMaint->Col.Send.pBuf[YCNumOffSet++] = LOBYTE(RecYCNum);
    pMaint->Col.Send.pBuf[YCNumOffSet++] = HIBYTE(RecYCNum);
	SendFrameTail(pMaint);
#else	
	SendNACK(pMaint,ErrorCode);
#endif
}

void SendYCfloat(VMaint *pMaint)            	
{
	BYTE ErrorCode=1;
	
#ifdef INCLUDE_YC
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	WORD Shift;
	WORD YCNum;
	WORD YCNumOffSet;
	DWORD EqpName;
//	WORD EqpID;
	WORD RecYCNum;
	VMeaYc*pwYC = NULL;
	BYTE * wYCValue;
	
	EqpName = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	//EqpName=0x00010000;
	Shift = MAKEWORD(pData[4], pData[5]);//取偏移量
    YCNum = MAKEWORD(pData[6], pData[7]);//取遥测数
 
    if (YCNum > ((MAINT_LEN_MAX-50)/10))
    {
		YCNum = (MAINT_LEN_MAX-50)/10;
	}
    SendFrameHead(pMaint,GET_ycfloat);
    SendDWordLH(pMaint,EqpName);
    SendWordLH(pMaint,Shift);
    //发送遥测的原始值
    YCNumOffSet = pMaint->Col.Send.wWritePtr; //记录AI个数存放位置
    pMaint->Col.Send.wWritePtr += 2;
    RecYCNum = meaRead_Yc2( Shift, YCNum, (VMeaYc *)pMaint->byBuf);
    
    if (RecYCNum == 0)
    {
    	ErrorCode = 2;//have no yc or yc num = 0
    	SendNACK(pMaint,ErrorCode);
    	return;	
    }
    
    pwYC = (VMeaYc *)pMaint->byBuf;
    for (int i = 0; i < RecYCNum; i++)
    {
    	wYCValue = (BYTE *)&(pwYC->value);
		pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=*(wYCValue+3);
		pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=*(wYCValue+2);
		pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=*(wYCValue+1);
		pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=*(wYCValue);
		strcpy((char*)pMaint->Col.Send.pBuf+pMaint->Col.Send.wWritePtr,pwYC->unit);
		pMaint->Col.Send.wWritePtr+=6;
    	pwYC ++;
    }
    
	pMaint->Col.Send.pBuf[YCNumOffSet++] = LOBYTE(RecYCNum);
    pMaint->Col.Send.pBuf[YCNumOffSet++] = HIBYTE(RecYCNum);
	SendFrameTail(pMaint);
#else
	SendNACK(pMaint,ErrorCode);
#endif
}

void SendYC2(VMaint *pMaint)            	
{
	BYTE ErrorCode=1;
	
#ifdef INCLUDE_YC
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	WORD Shift;
	WORD YCNum;
	WORD YCNumOffSet;
	DWORD EqpName;
	WORD RecYCNum;
	//struct VYCF_L*pwYC = NULL;
	VMeaYc *meaYc=NULL;
	BYTE *ptmp;
	EqpName = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	//EqpName=0x00010000;
	Shift = MAKEWORD(pData[4], pData[5]);//取偏移量
    YCNum = MAKEWORD(pData[6], pData[7]);//取遥测数

    if (YCNum > ((MAINT_LEN_MAX-50)/5))
    {
		YCNum = (MAINT_LEN_MAX-50)/5;
	}
    SendFrameHead(pMaint,GET_YC2);
    SendDWordLH(pMaint,EqpName);
    SendWordLH(pMaint,Shift);
    //发送遥测的原始值
    YCNumOffSet = pMaint->Col.Send.wWritePtr; //记录AI个数存放位置
    pMaint->Col.Send.wWritePtr += 2;
    RecYCNum = meaRead_Yc2( Shift, YCNum, (VMeaYc  *)pMaint->byBuf);
    
    if (RecYCNum == 0)
    {
    	ErrorCode = 2;//have no yc or yc num = 0
    	SendNACK(pMaint,ErrorCode);
    	return;	
    }
    
    meaYc = (VMeaYc  *)pMaint->byBuf;
    for (int i = 0; i < RecYCNum; i++)
    {
    	ptmp =(BYTE*) &meaYc[i].value;
    	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=ptmp[0];
    	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=ptmp[1];
    	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=ptmp[2];
    	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=ptmp[3];
	memcpy(	pMaint->Col.Send.pBuf+pMaint->Col.Send.wWritePtr,meaYc[i].unit,8);
	pMaint->Col.Send.wWritePtr+=8;
    }
    
	pMaint->Col.Send.pBuf[YCNumOffSet++] = LOBYTE(RecYCNum);
	pMaint->Col.Send.pBuf[YCNumOffSet++] = HIBYTE(RecYCNum);
	
	SendFrameTail(pMaint);	
#else	
	SendNACK(pMaint,ErrorCode);
#endif
}

void set_baohudingzhi(VMaint *pMaint)            	
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	DWORD EqpName;
	WORD Shift;
	BYTE line;
	BYTE *pdata = pData+10;
	EqpName = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
  	line = MAKEWORD(pData[4], pData[5]);//取偏移量
  	Shift = MAKEWORD(pData[6], pData[7]);//取偏移量

	if(WriteSet(line,pdata) == OK)
		WriteDoEvent(NULL, 0, "维护软件修改保护定值!");
	else
		WriteDoEvent(NULL, 0, "维护软件修改保护定值失败!");
	SendFrameHead(pMaint,SET_BH);
	SendDWordLH(pMaint,EqpName);
 	SendWordLH(pMaint,line);
	SendWordLH(pMaint,Shift);
	SendWordLH(pMaint,pMaint->prosetnum);
	memcpy(pMaint->Col.Send.pBuf+pMaint->Col.Send.wWritePtr,pdata,pMaint->prosetnum*sizeof(TSETVAL)+sizeof(TTABLETYPE));
		pMaint->Col.Send.wWritePtr+= pMaint->prosetnum*sizeof(TSETVAL)+sizeof(TTABLETYPE);
	SendFrameTail(pMaint);
	return;
}

void Get_baohudingzhi(VMaint *pMaint)            	
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	DWORD EqpName;
	WORD Shift;
	BYTE line;
	EqpName = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
  	line = MAKEWORD(pData[4], pData[5]);//取偏移量
  	Shift = MAKEWORD(pData[6], pData[7]);//取偏移量

	pMaint->prosetnum = ReadSetNum(line);
    SendFrameHead(pMaint,GET_BH);
    SendDWordLH(pMaint,EqpName);
 	SendWordLH(pMaint,line);
	SendWordLH(pMaint,Shift);
	SendWordLH(pMaint,pMaint->prosetnum);
 	ReadSet(line,pMaint->Col.Send.pBuf+pMaint->Col.Send.wWritePtr);
			pMaint->Col.Send.wWritePtr+= pMaint->prosetnum*sizeof(TSETVAL)+sizeof(TTABLETYPE);
	SendFrameTail(pMaint);
}

WORD read_dzdef(VMaint *pMaint ,WORD off,WORD num)
{
	#ifdef INCLUDE_PR
	int ttlen;
	struct VFileHead* phead;
	int ttloff;
	WORD mm_ybnum;
	WORD mm_CON1num;
	WORD mm_CON2num;
	WORD mm_CON3num;
	WORD mm_CON4num;
		int i;
		BYTE fdname[64];
	
	if((off==0)&&(num==0))
			{
			pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=g_Sys.MyCfg.wAllIoFDNum;
			pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=g_Sys.MyCfg.wAllIoFDNum>>8;
			}
		else
			{
			//fname=getfdcfg(off>>8);
			ReadSetName(off,fdname);
			off&=0xff;
		if (ReadParaFile((char*)fdname,prTableBuf,MAX_TABLE_FILE_SIZE)==ERROR) return false;
						BYTE* paddr=(BYTE*)(prTableBuf+sizeof(struct VFileHead)+sizeof(TTABLETYPE));
						mm_ybnum=*paddr;
						paddr++;
						paddr+=mm_ybnum*sizeof(TYBTABLE);
						mm_CON1num=*paddr;
						paddr++;
						TKGTABLE *tttt;
						mm_CON2num=0;
						for(i=0;i<mm_CON1num;i++)
						{
							tttt=(TKGTABLE *)paddr;
							paddr+=sizeof(TKGTABLE)-sizeof(DWORD);
							paddr+=tttt->byMsNum*9;
							mm_CON2num+=tttt->byMsNum;
						}
						mm_CON3num=*paddr;
						paddr++;
						mm_CON4num=0;
						for(i=0;i<mm_CON3num;i++)
						{
							tttt=(TKGTABLE *)paddr;
							paddr+=sizeof(TKGTABLE)-sizeof(DWORD);
							paddr+=tttt->byMsNum*9;
							mm_CON4num+=tttt->byMsNum;
						}
						pMaint->prosetnum=*paddr;
		phead=(struct VFileHead*)prTableBuf;
		ttlen=phead->nLength;
		ttloff=off*num;
	
		if(ttloff<ttlen)
			{
			if((ttlen-ttloff)<num)
				num=ttlen-ttloff;
			memcpy(pMaint->Col.Send.pBuf+pMaint->Col.Send.wWritePtr,prTableBuf+ttloff,num);
			pMaint->Col.Send.wWritePtr+=num;
			}
		else
			num=0;
			}
			return num;
	#else
			return 0;
	#endif
}

void Senddotdef(VMaint *pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	WORD dotoff;
	WORD dotnum;
	WORD SOENumOffSet;
	DWORD EqpName;
	WORD EventType;
	WORD datalen;

    EqpName = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	EventType=pData[4];
	dotoff = MAKEWORD(pData[5], pData[6]);
    dotnum = MAKEWORD(pData[7], pData[8]);
    
    SendFrameHead(pMaint,GET_dotdef);
	SendDWordLH(pMaint,EqpName);
	pMaint->Col.Send.pBuf[pMaint->Col.Send.wWritePtr++]=EventType;
    SendWordLH(pMaint,dotoff);
    SOENumOffSet = pMaint->Col.Send.wWritePtr; //记录SOE个数存放位置
    pMaint->Col.Send.wWritePtr += 2;
	datalen=dotnum;
	if(dotnum>20) dotnum=20;
	switch(EventType)
		{
		case 5:/*定值描述*/
			dotnum=read_dzdef(pMaint,dotoff,datalen);
			break;
		default:/*运行信息部分点表信息*/
			dotnum= 0;
			break;	
	}
	
	pMaint->Col.Send.pBuf[SOENumOffSet++] = LOBYTE(dotnum);
    pMaint->Col.Send.pBuf[SOENumOffSet++] = HIBYTE(dotnum);
	
	SendFrameTail(pMaint);
	return;
}

void Do_Lubo(VMaint *pMaint)
{
	DWORD EqpName;
	WORD line;
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	
	EqpName = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	line = MAKEWORD(pData[4], pData[5]);

#ifdef INCLUDE_PR
	if(line < fdNum)
		g_prlubo |= (1 << line);
#endif
	SendFrameHead(pMaint,GET_Lubo);
  	SendDWordLH(pMaint,EqpName);
	SendWordLH(pMaint,line);
	SendFrameTail(pMaint);
	return;
}

void ReadPrParaMaint(VMaint* pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	int ret = ERROR;
	WORD parano,type,len;

	parano = MAKEWORD(pData[0], pData[1]);
	type = MAKEWORD(pData[2], pData[3]);

	ret = ReadPrPara(parano,(char *)pMaint->byBuf,type);
	
	SendFrameHead(pMaint,READ_PRPARA);
	SendDWordLH(pMaint,(DWORD)ret);
	len = sizeof(struct VParaInfo);
	SendWordLH(pMaint,len);
	memcpy(pMaint->Col.Send.pBuf + pMaint->Col.Send.wWritePtr,pMaint->byBuf,len);
	pMaint->Col.Send.wWritePtr += len;
	SendFrameTail(pMaint);
}

void WritePrParaMaint(VMaint* pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	WORD parano,type;
	int ret = ERROR;

	parano = MAKEWORD(pData[0], pData[1]);
	type = MAKEWORD(pData[2], pData[3]);
	//len = MAKEWORD(pData[4], pData[5]);
	ret = WritePrPara(parano,(char*)(pData + 6),type);
	
	SendFrameHead(pMaint,WRITE_PRPARA);
	SendDWordLH(pMaint,(DWORD)ret);
	SendFrameTail(pMaint);
}

void WritePrParaFileMaint(VMaint* pMaint)
{
	int ret = ERROR;

	ret = WritePrParaFile();

	SendFrameHead(pMaint,WRITE_PRPARA_FILE);
	SendDWordLH(pMaint,(DWORD)ret);
	SendFrameTail(pMaint);
}

void ResetProtectMaint(VMaint* pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	int fd;

	fd = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	ResetProtect(fd);

	SendFrameHead(pMaint,RESET_PROTECT);
	SendFrameTail(pMaint);
}

void ykTypeSaveMaint(VMaint* pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	int type,id,val;

	type = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	id = MAKEDWORD(MAKEWORD(pData[4], pData[5]), MAKEWORD(pData[6], pData[7]));
	val = MAKEDWORD(MAKEWORD(pData[8], pData[9]), MAKEWORD(pData[10], pData[11]));

	ykTypeSave(type,id,val);
	
	SendFrameHead(pMaint,YK_TYPE);
	SendFrameTail(pMaint);
}

void WriteRunParaMaint(VMaint* pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	WORD len;

	len  = MAKEWORD(pData[0], pData[1]);
	
	WriteRunPara(pData +2,len);
	
	SendFrameHead(pMaint,WRITE_RUN);
	SendFrameTail(pMaint);
	
}

void yk_YB(VMaint* pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	DWORD yb;

	yb = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	BMSetYkYb(yb);

	//SendFrameHead(pMaint,YK_YB);
	//SendFrameTail(pMaint);
}
void Set_ERR_CODE(VMaint* pMaint)
{
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	BYTE *pData = (BYTE*)(pRec + 1);
	DWORD errcode;

	errcode = MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	g_Sys.dwErrCode = errcode; 

	//SendFrameHead(pMaint,ERR_CODE);
	//SendFrameTail(pMaint);
}

static void ProcFrm(void *arg)
{
    VMaint *pMaint = (VMaint *)arg;
	VMaintFrame *pRec = (VMaintFrame *)pMaint->Col.RecFrm.pBuf;
	WORD dwCode;

	dwCode = MAKEWORD(pRec->afn, pRec->CMD);

    switch (dwCode)
    {
    	case SET_LEDIO: //执行对应操作
    	    //缺 cjl
    	    //setledio(pMaint);
			SendACK(pMaint);
			break;
		case YK_BM:
			yk_BM(pMaint);
			break;
		case YK_YB:
			yk_YB(pMaint);
			break;
		case YK_T:
			WriteYkT_Bm(pMaint);
		    break;
		case YX_T:
			WriteYxTime_BM(pMaint);
			break;
		case YC_ZERO:
			WriteYcZero(pMaint);
			break;
		case ERR_CODE:
			Set_ERR_CODE(pMaint);
			break;
		case GET_IO:
			
			break;
		case GET_CLK:

			break;
		case SET_CLK:
			SetClock(pMaint);
			break;
		case YC_PARAZERO:
			if(gainzero() == OK)
				SendACK(pMaint);
			else
				SendNACK(pMaint,MAINT_CMD_ERR);
			break;
		case YC_PARASET:
			write_ycgain(pMaint);
			break;
		case YC_PARAVALSET:
			write_ycgainval(pMaint);
			break;
		case GET_accurve:
			SendAC(pMaint);
			break;
		case GET_YXZ:
			read_youxiaozhi(pMaint);
			break;
		case GET_YC1:
    		SendYC1(pMaint);
    		break;
		case GET_YC2:
    		SendYC2(pMaint);
    		break;
		case GET_ycfloat:
			SendYCfloat(pMaint);
			break;
		case SET_BH:
			set_baohudingzhi(pMaint);
		break;
		case GET_BH:
			Get_baohudingzhi(pMaint);
		break;
		case GET_dotdef:
			Senddotdef(pMaint);
		break;
		case GET_Lubo:
			Do_Lubo(pMaint);
		break;
		case WRITE_RUN:
			WriteRunParaMaint(pMaint);
		break;
		case YK_TYPE:
			ykTypeSaveMaint(pMaint);
		break;
		case READ_PRPARA:
			ReadPrParaMaint(pMaint);
		break;
		case WRITE_PRPARA:
			WritePrParaMaint(pMaint);
		break;
		case WRITE_PRPARA_FILE:
			WritePrParaFileMaint(pMaint);
		break;
		case RESET_PROTECT:
			ResetProtectMaint(pMaint);
		break;
		default:
			SendNACK(pMaint, MAINT_CMD_ERR);
			break;
    }

	return;
}

VMaint *pMaint;
int MaintInit = 0;
int BmReadLinux(BYTE* buf, int buflen);
int BmWriteLinux( BYTE* buf, int len);

void maint(void)
{
	
	pMaint = (VMaint*)malloc(sizeof(VMaint));
	memset(pMaint, 0, sizeof(VMaint));

	PcolInit(MAINT_ID, &pMaint->Col, ProcFrm, SearchOneFrame, BmReadLinux, BmWriteLinux);
	MaintInit = 1;
}

//通讯读，10ms缺个读数据
void scan_maint()
{	
	DWORD events;

	evReceive(MAINT_ID, EV_TM1, &events);

	if (events & EV_TM1)
	{
		if(MaintInit)
			DoReceive(&(pMaint->Col));
	}
}


//以后还要不要考虑串口维护软件？？  


