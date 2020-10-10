/*------------------------------------------------------------------------
 $Rev: 28 $
------------------------------------------------------------------------*/

#include "maint.h"

#include "protocol.h"

#define maint_test 0

#ifdef INCLUDE_ATT7022E
extern "C" int att_gainread(void);
extern "C" int att_gainclear(void);
#endif


/***************************************************************
	Function：CMaint
		构造函数，暂空
	参数：无
		
	返回：无
***************************************************************/
CMaint::CMaint() : CPSecondary()
{
	m_dwEqpName = 0;
	m_wYKCode = 0;
	fileoffset = 0;
	m_sendcount = 0;
	m_wirecommflag = 0;
	m_pReceive = NULL;
	m_pSend = NULL;
	m_pFrameData = NULL;
	prosetnum = 0;
	gainnum = 0;
	m_wYCNum = NULL;
}

/***************************************************************
	Function：maint
		维护规约入口函数
	参数：wTaskID, arg
		wTaskID 任务ID
		arg 任务参数
	返回：无
***************************************************************/
extern "C" void maint(WORD wTaskID)
{
	CMaint *pMaint = new CMaint();		//构造函数
	if (pMaint->Init(wTaskID) != TRUE)
	{
		pMaint->ProtocolExit();
	}
	pMaint->Run();
	
	delete(pMaint);
}
/***************************************************************
	Function：Init
		维护规约的初始化
	参数：wTaskID
		wTaskID 任务ID
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CMaint::Init(WORD wTaskID)
{
	BOOL rc;
	int i, j;
	VTVEqpInfo EqpInfo;
	rc = CPSecondary::Init(wTaskID,0,NULL,0);
	if (!rc)
	{
		return FALSE;
	}

	m_sendcount=0;
	m_wirecommflag=0;
	prosetnum=0;
#ifdef	MAINT_YCCHAGE_SEND	
	m_wYCNum = (WORD*)calloc(*g_Sys.Eqp.pwNum, sizeof(WORD));
	m_pEqpExtInfo= (VSecEqpExtInfo*)calloc(*g_Sys.Eqp.pwNum, sizeof(VSecEqpExtInfo)); 

	if (!m_wYCNum || !m_pEqpExtInfo)
		return FALSE;
	
	for (i=0; i<*g_Sys.Eqp.pwNum; i++)
	{
		memset(m_pEqpExtInfo+i,0,sizeof(VSecEqpExtInfo));	

		memset((void *)&EqpInfo, 0, sizeof(EqpInfo));
		ReadEqpInfo(MAINT_ID, (WORD)i, &EqpInfo, FALSE);

		if ((EqpInfo.wYCNum>0) && ((EqpInfo.wType==TRUEEQP) || (EqpInfo.wType==VIRTUALEQP)))
		{
			m_pEqpExtInfo[i].OldYC= (VYCF_L*)calloc(EqpInfo.wYCNum, sizeof(VYCF_L));
			m_pEqpExtInfo[i].CurYC= (VYCF_L*)calloc(EqpInfo.wYCNum, sizeof(VYCF_L));
			m_pEqpExtInfo[i].ChangeYCSend= (BOOL*)calloc(EqpInfo.wYCNum, sizeof(BOOL));
			if (!m_pEqpExtInfo[i].OldYC||!m_pEqpExtInfo[i].CurYC||!m_pEqpExtInfo[i].ChangeYCSend)
				return FALSE;
			for (j=0; j<EqpInfo.wYCNum; j++)
				m_pEqpExtInfo[i].OldYC[j].lValue=0;			
		}
		else
		{
			m_pEqpExtInfo[i].OldYC=NULL;
			m_pEqpExtInfo[i].CurYC=NULL;
			m_pEqpExtInfo[i].ChangeYCSend=NULL;
		}
        m_wYCNum[i] = EqpInfo.wYCNum;
		m_pEqpExtInfo[i].wChangYCNo=0;			
	}//end of for
#endif

	return TRUE;
}


/***************************************************************
	Function：DoYKRet
		遥控返校信息处理
	参数：pMsg
		pMsg 消息缓冲区头指针
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CMaint::DoYKRet(void)
{
	VDBYK *pDBYk = (VDBYK *)m_pMsg->abyData;
		
	SendFrameHead(m_wYKCode);
	SendDWordLH(m_dwEqpName);
	SendWordLH(pDBYk->wID);
	SendWordLH(pDBYk->byValue);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr-1] = pDBYk->byStatus; //遥控属性高字节作为遥控结果字节
	SendFrameTail();
	return TRUE;	
}

 
BOOL CMaint::DoYTRet(void)
{
	VDBYT *pDBYt = (VDBYT *)m_pMsg->abyData;
	
	SendFrameHead(YT_SET);
	SendDWordLH(m_dwEqpName);
	SendWordLH(pDBYt->wID);
	SendWordLH((WORD)pDBYt->dwValue);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = pDBYt->byStatus;
	SendFrameTail();
	return TRUE;	
}
void CMaint::DoTSDataMsg(void)
{
	struct VFileMsgProM *pFileMsgProM;
	pFileMsgProM = (VFileMsgProM *)m_pMsg;
	SendFrameHead(MMI_PROG_WRITE);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = pFileMsgProM->flag;
	SendDWordLH(pFileMsgProM->addr);
	SendWordLH(pFileMsgProM->pLen);
	SendFrameTail();
	return;	
}

/***************************************************************
	Function：SendFrameHead
		组织发送报文的报文头
	参数：wCode
		wCode 规约中的功能码
	返回：无
***************************************************************/
void CMaint::SendFrameHead(WORD wCode)
{
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;//清空发送缓冲区 
	
	m_pSend = (VFrame *)m_SendBuf.pBuf;
	
    m_pSend->StartCode1 = 0x68;
    m_pSend->StartCode2 = 0x68;
    m_pSend->Address = m_pReceive->Address; 
	m_pSend->CMD= HIBYTE(wCode)|0x80;
	m_pSend->afn= LOBYTE(wCode); 
	m_SendBuf.wWritePtr = (WORD)((BYTE*)&(m_pSend->Data) - (BYTE*)m_pSend);
	return;
}

void CMaint::SendFrameHead1(WORD wCode)
{	
    m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;//清空发送缓冲区 		
    m_pSend = (VFrame *)m_SendBuf.pBuf;	    
	m_pSend->StartCode1 = 0x68;    
	m_pSend->StartCode2 = 0x68;    
	m_pSend->Address = m_pReceive->Address; 	
	m_pSend->CMD= HIBYTE(wCode);	
	m_pSend->afn= LOBYTE(wCode); 	
	m_SendBuf.wWritePtr = (WORD)((BYTE*)&(m_pSend->Data) - (BYTE*)m_pSend);	
	return;
}

/***************************************************************
	Function：SendFrameTail
		填充发送报文的帧尾，并发送整个报文（写到通信端口缓冲区）
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendFrameTail(void)
{
	WORD Len = m_SendBuf.wWritePtr-6;
	WORD CrcCode;

    //写帧长度
	m_pSend->Len1Low= LOBYTE(Len);
	m_pSend->Len1High= HIBYTE(Len);
 	m_pSend->Len2Low= LOBYTE(Len);
	m_pSend->Len2High= HIBYTE(Len);
   	//CRC校验
	CrcCode = CheckSum((BYTE *)&m_pSend->Address, Len);  
	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(CrcCode);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(CrcCode);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	
	WriteToComm(m_pReceive->Address);
	return;
}

void CMaint::SendFrameTail1(void)
{	
    WORD Len = m_SendBuf.wWritePtr-6;	
	WORD CrcCode;    //写帧长度	
	m_pSend->Len1Low= LOBYTE(Len);	
	m_pSend->Len1High= HIBYTE(Len); 	
	m_pSend->Len2Low= LOBYTE(Len);	
	m_pSend->Len2High= HIBYTE(Len);   	
	//CRC校验	
	CrcCode = CheckSum((BYTE *)&m_pSend->Address, Len);  		
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(CrcCode);	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(CrcCode);	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;	
	//	WriteToComm(m_pReceive->Address);	
	return;
}

/***************************************************************
	Function：SendBaseFrame
		发送简单报文
	参数：dwCode
		dwCode 规约中的功能码
	返回：无
***************************************************************/
void CMaint::SendBaseFrame(WORD dwCode)
{
    SendFrameHead(dwCode);
    SendFrameTail();
	return;
}

/***************************************************************
	Function：SendClock
		发送时钟信息
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendClock(void)
{
	
	VSysClock SysTime;
	//填充获取系统时间的函数调用
	GetSysClock((void *)&SysTime, SYSCLOCK);
	
	SendFrameHead(GET_CLK);
	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(SysTime.wYear);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(SysTime.wYear);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMonth;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byDay;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byWeek;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byHour;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.byMinute;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = SysTime.bySecond;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(SysTime.wMSecond);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(SysTime.wMSecond);
	
	SendFrameTail();
	
	return;
}

void CMaint::readClock(void)
{			
    //SendFrameHead(GET_CLK);
    WORD CrcCode;
    m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;//清空发送缓冲区 

    m_pSend = (VFrame *)m_SendBuf.pBuf;

    m_pSend->StartCode1 = 0x68;
    m_pSend->StartCode2 = 0x68;
    m_pSend->Address = 0; 
    m_pSend->CMD= HIBYTE(GET_CLK);
    m_pSend->afn= LOBYTE(GET_CLK); 
    m_SendBuf.wWritePtr = (WORD)((BYTE*)&(m_pSend->Data) - (BYTE*)m_pSend);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;	
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x10;	
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;	
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0;		
    m_pSend->Len1Low= LOBYTE(m_SendBuf.wWritePtr-6);
	m_pSend->Len1High= HIBYTE(m_SendBuf.wWritePtr-6);
 	m_pSend->Len2Low= LOBYTE(m_SendBuf.wWritePtr-6);
	m_pSend->Len2High= HIBYTE(m_SendBuf.wWritePtr-6);
   	//CRC校验
	CrcCode = CheckSum((BYTE *)&m_pSend->Address, m_SendBuf.wWritePtr-6);  
	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(CrcCode);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(CrcCode);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
	
	WriteToComm(0);		
	return;
}

/***************************************************************
	Function：SetClock
		设置时钟
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SetClock(void)
{
	
	VSysClock SysTime;
	
	SysTime.wYear		= MAKEWORD(m_pFrameData[0], m_pFrameData[1]);
	SysTime.byMonth   	= m_pFrameData[2];
	SysTime.byDay     	= m_pFrameData[3];
	SysTime.byWeek     	= m_pFrameData[4];
	SysTime.byHour    	= m_pFrameData[5];
	SysTime.byMinute  	= m_pFrameData[6];
	SysTime.bySecond  	= m_pFrameData[7];
	SysTime.wMSecond 	= MAKEWORD(m_pFrameData[8],m_pFrameData[9]);
	
	//填充写系统时钟函数调用
	SetSysClock((void *)&SysTime, SYSCLOCK);
	SendACK();
	
	WriteDoEvent(NULL, 0, "维护软件对时 %d年%d月%d日%d时%d分%d秒%d毫秒  ",
				SysTime.wYear,SysTime.byMonth,SysTime.byDay,SysTime.byHour,SysTime.byMinute,SysTime.bySecond,SysTime.wMSecond);
	return;
}

/***************************************************************
	Function：SendYC
		发送遥测数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendYC(void)            	
{
	WORD Shift;
	WORD YCNum;
	WORD YCNumOffSet;
	DWORD EqpName;
	WORD EqpID;
	BYTE ErrorCode=1;
	WORD RecYCNum;
	struct VYCF_L*pwYC;
	DWORD wYCValue;
	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    YCNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

    if (YCNum > ((MAX_FRAME_SIZE-50)/5))
    {
		YCNum = (MAX_FRAME_SIZE-50)/5;
	}
    SendFrameHead(GET_YC);
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    //发送遥测的原始值
    YCNumOffSet = m_SendBuf.wWritePtr; //记录AI个数存放位置
    m_SendBuf.wWritePtr += 2;
    RecYCNum = ReadRangeAllYCF_L(EqpID, Shift, YCNum, MAX_FRAME_SIZE-10, (struct VYCF_L  *)m_dwPubBuf);
    
    if (RecYCNum == 0)
    {
    	ErrorCode = 2;//have no yc or yc num = 0
    	SendNACK(ErrorCode);
    	return;	
    }
    
    pwYC = (struct VYCF_L   *)m_dwPubBuf;
    for (int i = 0; i < RecYCNum; i++)
    {

   #ifdef INCLUDE_YC
		if((EqpID == g_Sys.wIOEqpID) && (g_Sys.MyCfg.dwCfg & 0x04))
		{
			wYCValue = pwYC->lValue * ReadYcPtCt(i);
		}
		else if((EqpID == g_Sys.wIOEqpID) && (g_Sys.MyCfg.dwCfg & 0x10) && (g_Sys.MyCfg.pYc[i].type == YC_TYPE_U0))
		{
			wYCValue = pwYC->lValue/10;
		}
		else
   #endif
    	wYCValue = (DWORD)pwYC->lValue;
    	SendDWordLH(wYCValue);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=pwYC->byFlag;
    	pwYC ++;
    }
    
	SendWordLH(YCNumOffSet, RecYCNum);
	
	SendFrameTail();
    return;
}

/***************************************************************
	Function：SendYC
		发送遥测数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendYC1(void)            	
{
	BYTE ErrorCode=1;		
#ifdef INCLUDE_YC
#if 0
	WORD Shift;
	WORD YCNum;
	WORD YCNumOffSet;
	DWORD EqpName;
	WORD EqpID;
	WORD RecYCNum;
	//struct VYCF_L*pwYC = NULL;
	VMeaYc *meaYc;
	BYTE *ptmp;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    YCNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

    if (YCNum > ((MAX_FRAME_SIZE-50)/5))
    {
		YCNum = (MAX_FRAME_SIZE-50)/5;
	}
    SendFrameHead(GET_YC1);
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    //发送遥测的原始值
    YCNumOffSet = m_SendBuf.wWritePtr; //记录AI个数存放位置
    m_SendBuf.wWritePtr += 2;
    RecYCNum = (WORD)meaRead_Yc1( Shift, YCNum, (VMeaYc  *)m_dwPubBuf);
    
    if (RecYCNum == 0)
    {
    	ErrorCode = 2;//have no yc or yc num = 0
    	SendNACK(ErrorCode);
    	return;	
    }
    
    meaYc = (VMeaYc   *)m_dwPubBuf;
    for (int i = 0; i < RecYCNum; i++)
    {
    	ptmp =(BYTE*) &meaYc[i].value;
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=ptmp[0];
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=ptmp[1];
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=ptmp[2];
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=ptmp[3];
		memcpy(	m_SendBuf.pBuf+m_SendBuf.wWritePtr,meaYc[i].unit,8);
		m_SendBuf.wWritePtr+=8;
    }
    
	SendWordLH(YCNumOffSet, RecYCNum);
	
	SendFrameTail();
#endif
	WORD rcvlen = 0;
	
	if(Maint_BM_RCV(GET_YC1,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3,
	(BYTE*)m_dwPubBuf, &rcvlen) == ERROR)
	{
		SendNACK(ErrorCode);
		return;
	}
	
	SendFrameHead(GET_YC1);
	memcpy(m_SendBuf.pBuf + m_SendBuf.wWritePtr, m_dwPubBuf ,rcvlen);
	m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + rcvlen;
	SendFrameTail();

#else	
	SendNACK(ErrorCode);
#endif
}/***************************************************************
	Function：SendYC
		发送遥测数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendYC2(void)            	
{
	BYTE ErrorCode=1;
#ifdef INCLUDE_YC
#if 0
	WORD Shift;
	WORD YCNum;
	WORD YCNumOffSet;
	DWORD EqpName;
	WORD EqpID;
	WORD RecYCNum;
	//struct VYCF_L*pwYC = NULL;
	VMeaYc *meaYc;
	BYTE *ptmp;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    YCNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

    if (YCNum > ((MAX_FRAME_SIZE-50)/5))
    {
		YCNum = (MAX_FRAME_SIZE-50)/5;
	}
    SendFrameHead(GET_YC2);
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    //发送遥测的原始值
    YCNumOffSet = m_SendBuf.wWritePtr; //记录AI个数存放位置
    m_SendBuf.wWritePtr += 2;
    RecYCNum = (WORD)meaRead_Yc2( Shift, YCNum, (VMeaYc  *)m_dwPubBuf);
    
    if (RecYCNum == 0)
    {
    	ErrorCode = 2;//have no yc or yc num = 0
    	SendNACK(ErrorCode);
    	return;	
    }
    
    meaYc = (VMeaYc  *)m_dwPubBuf;
    for (int i = 0; i < RecYCNum; i++)
    {
    	ptmp =(BYTE*) &meaYc[i].value;
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=ptmp[0];
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=ptmp[1];
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=ptmp[2];
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=ptmp[3];
		memcpy(	m_SendBuf.pBuf+m_SendBuf.wWritePtr,meaYc[i].unit,8);
		m_SendBuf.wWritePtr+=8;
    }
    
	SendWordLH(YCNumOffSet, RecYCNum);
	
	SendFrameTail();	
#endif
	WORD rcvlen = 0;
	
	if(Maint_BM_RCV(GET_YC2,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3,
	(BYTE*)m_dwPubBuf, &rcvlen) == ERROR)
	{
		SendNACK(ErrorCode);
		return;
	}
	
	SendFrameHead(GET_YC2);
	memcpy(m_SendBuf.pBuf + m_SendBuf.wWritePtr, m_dwPubBuf ,rcvlen);
	m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + rcvlen;
	SendFrameTail();
#else	
	SendNACK(ErrorCode);
#endif
}

/***************************************************************
	Function：SendYC
		发送遥测通道点数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendAC(void)            	
{
	BYTE ErrorCode=1;

#ifdef INCLUDE_YC
#if 0
	WORD Shift;
	WORD YCNum;
	WORD YCNumOffSet;
	DWORD EqpName;
	WORD EqpID;
	BYTE flag1;
	BYTE flag2;
	int RecYCNum;
	WORD*pwYC = NULL;
	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	flag1=m_pFrameData[4];
	flag2=m_pFrameData[5];
    Shift = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

   
    SendFrameHead(GET_accurve);
    SendDWordLH(EqpName);
    //SendWordLH(Shift);
    //发送遥测的原始值
    YCNumOffSet = m_SendBuf.wWritePtr; //记录AI个数存放位置
    m_SendBuf.wWritePtr += 2;
	SendWordLH(Shift);
	if(flag1==2)
        YCNum = meaRead_YcChnBuf( Shift, &flag2, &RecYCNum, (short  *)m_dwPubBuf,(MAX_FRAME_SIZE-50)/2,-1);
    else
	      YCNum=meaRead_FdChnBuf1(Shift,&flag2,&RecYCNum, (short  *)m_dwPubBuf,(MAX_FRAME_SIZE-50)/2);
    if ((YCNum == 0)|(YCNum>10))
    {
    	ErrorCode = 2;//have no yc or yc num = 0
    	SendNACK(ErrorCode);
    	return;	
    }
 	SendWordLH(RecYCNum);
   
    pwYC = (WORD   *)m_dwPubBuf;
	for(int j=0;j<YCNum;j++)
    for (int i = 0; i < RecYCNum; i++)
    {
    	SendWordLH(*pwYC++);
    }
    m_SendBuf.pBuf[YCNumOffSet++] = flag1;
    m_SendBuf.pBuf[YCNumOffSet++] = flag2;
	SendFrameTail();
#endif
	WORD rcvlen = 0;
	BYTE flag2;

	flag2=m_pFrameData[5];

	if(flag2 == 0x01) //仅第一个通道触发
		SetBmUrgency(0x01);
	if(Maint_BM_RCV(GET_accurve,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3,
	(BYTE*)m_dwPubBuf, &rcvlen) == ERROR)
	{
		SendNACK(ErrorCode);
		return;
	}
	
	SendFrameHead(GET_accurve);
	memcpy(m_SendBuf.pBuf + m_SendBuf.wWritePtr, m_dwPubBuf ,rcvlen);
	m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + rcvlen;
	SendFrameTail();

#else
    SendNACK(ErrorCode);
#endif
}
/***************************************************************
	Function：SendYC
		发送遥测数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendYCfloat(void)            	
{
	BYTE ErrorCode=1;
#ifdef INCLUDE_YC
#if 0
	WORD Shift;
	WORD YCNum;
	WORD YCNumOffSet;
	DWORD EqpName;
//	WORD EqpID;
	WORD RecYCNum;
	VMeaYc*pwYC;
	BYTE * wYCValue;
	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    YCNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数
 
    if (YCNum > ((MAX_FRAME_SIZE-50)/10))
    {
		YCNum = (MAX_FRAME_SIZE-50)/10;
	}
    SendFrameHead(GET_ycfloat);
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    //发送遥测的原始值
    YCNumOffSet = m_SendBuf.wWritePtr; //记录AI个数存放位置
    m_SendBuf.wWritePtr += 2;
    RecYCNum = (WORD)meaRead_Yc2( Shift, YCNum, (VMeaYc *)m_dwPubBuf);
    
    if (RecYCNum == 0)
    {
    	ErrorCode = 2;//have no yc or yc num = 0
    	SendNACK(ErrorCode);
    	return;	
    }
    
    pwYC = (VMeaYc *)m_dwPubBuf;
    for (int i = 0; i < RecYCNum; i++)
    {
    	wYCValue = (BYTE *)&(pwYC->value);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=*(wYCValue+3);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=*(wYCValue+2);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=*(wYCValue+1);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=*(wYCValue);
		strcpy((char*)m_SendBuf.pBuf+m_SendBuf.wWritePtr,pwYC->unit);
		m_SendBuf.wWritePtr+=6;
    	pwYC ++;
    }
    
	SendWordLH(YCNumOffSet, RecYCNum);
	SendFrameTail();
#endif
	WORD rcvlen = 0;
	
	if(Maint_BM_RCV(GET_ycfloat,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3,
	(BYTE*)m_dwPubBuf, &rcvlen) == ERROR)
	{
		SendNACK(ErrorCode);
		return;
	}
	
	SendFrameHead(GET_ycfloat);
	memcpy(m_SendBuf.pBuf + m_SendBuf.wWritePtr, m_dwPubBuf ,rcvlen);
	m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + rcvlen;
	SendFrameTail();
#else
	SendNACK(ErrorCode);
#endif
}
	/***************************************************************
	Function：SendhisYC
		发送遥测数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendHisYC(void)            	
{
	BYTE ErrorCode=1;

#ifdef INCLUDE_FRZ
	WORD YCID;
	WORD YCNum;
	DWORD time;
	DWORD EqpName;
	WORD EqpID;
	WORD md;
	WORD RecYCNum;
	DWORD *pwYC = NULL;
	 struct VSysClock td;
	 struct VCalClock tt;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	YCID = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
	time= MAKEDWORD(MAKEWORD(m_pFrameData[6], m_pFrameData[7]), MAKEWORD(m_pFrameData[8], m_pFrameData[9]));
    md = MAKEWORD(m_pFrameData[10], m_pFrameData[11]);//取遥测数
    YCNum = MAKEWORD(m_pFrameData[12], m_pFrameData[13]);//取遥测数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

    if (YCNum > ((MAX_FRAME_SIZE-50)/4))
    {
		YCNum = (MAX_FRAME_SIZE-50)/4;
	}
    SendFrameHead(GET_HIS_YC);
    SendDWordLH(EqpName);
    SendWordLH(YCID);
     SendDWordLH(time);
      SendWordLH(md);
 //发送遥测的原始值
 tt.dwMinute=time;
   CalClockTo(&tt, &td);
   if(md<2048)
    RecYCNum = ReadYCCurve(EqpID, YCID,&td,YCNum,md,(long*)m_dwPubBuf, FALSE);
   else if(md==2048)
   RecYCNum =ReadYCDay(EqpID, YCID,&td,(long*)m_dwPubBuf, FALSE);
   else if(md==4096)
   RecYCNum =ReadYCMonth(EqpID, YCID,&td,(long*)m_dwPubBuf, FALSE);
         SendWordLH(RecYCNum);
 
   
    
    pwYC = (DWORD *)m_dwPubBuf;
    for (int i = 0; i < RecYCNum; i++)
    {
    	SendDWordLH(*pwYC);
    	pwYC ++;
    }
    
	
	SendFrameTail();
#else
    SendNACK(ErrorCode);
#endif
}
		/***************************************************************
	Function：SendhisYC
		发送遥测数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendHisdd(void)            	
{
	BYTE ErrorCode=1;
#ifdef INCLUDE_FRZ
	WORD YCID;
	WORD YCNum;
	DWORD time;
	DWORD EqpName;
	WORD EqpID;
	WORD md;
	WORD RecYCNum;
	DWORD *pwYC = NULL;
	 struct VSysClock td;
	 struct VCalClock tt;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	YCID = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
	time= MAKEDWORD(MAKEWORD(m_pFrameData[6], m_pFrameData[7]), MAKEWORD(m_pFrameData[8], m_pFrameData[9]));
    md = MAKEWORD(m_pFrameData[10], m_pFrameData[11]);//取遥测数
    YCNum = MAKEWORD(m_pFrameData[12], m_pFrameData[13]);//取遥测数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

    if (YCNum > ((MAX_FRAME_SIZE-50)/4))
    {
		YCNum = (MAX_FRAME_SIZE-50)/4;
	}
    SendFrameHead(GET_HIS_DD);
    SendDWordLH(EqpName);
    SendWordLH(YCID);
     SendDWordLH(time);
      SendWordLH(md);
 //发送遥测的原始值
 tt.dwMinute=time;
   CalClockTo(&tt, &td);
   if(md<2048)
    RecYCNum = ReadDDCurve(EqpID, YCID,&td,YCNum,md,(long*)m_dwPubBuf, FALSE);
   else if(md==2048)
   RecYCNum =ReadDDDay(EqpID, YCID,&td,(long*)m_dwPubBuf, FALSE);
   else if(md==4096)
   RecYCNum =ReadDDMonth(EqpID, YCID,&td,(long*)m_dwPubBuf, FALSE);
         SendWordLH(RecYCNum);
 
   
    
    pwYC = (DWORD *)m_dwPubBuf;
    for (int i = 0; i < RecYCNum; i++)
    {
    	SendDWordLH(*pwYC);
    	pwYC ++;
    }
    
	
	SendFrameTail();
#else
    SendNACK(ErrorCode);
#endif
}
	/***************************************************************
	Function：SendhisYC
		发送遥测数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::Sendmaxvalue(void)            	
{
	BYTE ErrorCode=1;
#ifdef INCLUDE_FRZ
	WORD YCID;
	WORD YCNum;
	DWORD time;
	DWORD EqpName;
	WORD EqpID;
	WORD md;
	WORD RecYCNum;
	struct VMaxMinYC *pwYC = NULL;
	 struct VSysClock td;
	 struct VCalClock tt;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	YCID = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
	time= MAKEDWORD(MAKEWORD(m_pFrameData[6], m_pFrameData[7]), MAKEWORD(m_pFrameData[8], m_pFrameData[9]));
    md = MAKEWORD(m_pFrameData[10], m_pFrameData[11]);//取遥测数
    YCNum = MAKEWORD(m_pFrameData[12], m_pFrameData[13]);//取遥测数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

    if (YCNum > ((MAX_FRAME_SIZE-50)/20))
    {
		YCNum = (MAX_FRAME_SIZE-50)/20;
	}
    SendFrameHead(GET_maxvalue);
    SendDWordLH(EqpName);
    SendWordLH(YCID);
     SendDWordLH(time);
      SendWordLH(md);
 //发送遥测的原始值
 tt.dwMinute=time;
   CalClockTo(&tt, &td);
    RecYCNum =ReadYCMaxMin(EqpID, YCID,&td,(struct VMaxMinYC*)m_dwPubBuf, FALSE);
          SendWordLH(RecYCNum);
 
   
    
    pwYC = (struct VMaxMinYC*)m_dwPubBuf;
    for (int i = 0; i < RecYCNum; i++)
    {
    	SendDWordLH(pwYC->lMax);
    	SendDWordLH(pwYC->max_tm.dwMinute);
    	SendWordLH(pwYC->max_tm.wMSecond);
    	SendDWordLH(pwYC->lMin);
    	SendDWordLH(pwYC->min_tm.dwMinute);
    	SendWordLH(pwYC->min_tm.wMSecond);
   	pwYC ++;
    }	
	SendFrameTail();
#else
    SendNACK(ErrorCode);
#endif
}

#ifdef	MAINT_YCCHAGE_SEND	
WORD CMaint::SearchChangeYC(WORD wEqpID, WORD wNum, WORD wBufLen, VDBYCF_L *pBuf)
{
    float wAbsVal,nChangeVal;
	int i,j,k;
    VYCF_L *pCurYC;
	VDBYCF_L *pSendYC;
	VYCF_L *pVal;
	float fval1,fval2;

    if (m_wYCNum[wEqpID] == 0) return(0);
	
	if (wBufLen<sizeof(VDBYCF))  return(0);
	
	for (i=0; i<m_wYCNum[wEqpID]; i++)
	{
        m_pEqpExtInfo[wEqpID].ChangeYCSend[i]=FALSE;
	}  
	m_wEqpNo=wEqpID;
	::ReadRangeAllYCF_L(m_wEqpID, 0, m_wYCNum[wEqpID], (WORD)(m_wYCNum[wEqpID]*sizeof(VYCF_L)) , m_pEqpExtInfo[m_wEqpNo].CurYC);

    for (i=0; i<m_wYCNum[wEqpID]; i++)
    {         
		pCurYC=m_pEqpExtInfo[m_wEqpNo].CurYC+i;
		if (pCurYC->byFlag & 0x80) continue;     //active send disable
		
		pVal=m_pEqpExtInfo[m_wEqpNo].OldYC+i;
		if (pVal->byFlag & 0x40)
		{
			memcpy((void *)&fval1,(void*)&pVal->lValue,4);
		}
		else
		{
			fval1=pVal->lValue;
		}
		nChangeVal=fval1*m_pBaseCfg->wYCDeadValue/1000;
		if (nChangeVal<0) nChangeVal=0-nChangeVal;
		if (pCurYC->byFlag & 0x40)
		{
			memcpy((void *)&fval2,(void*)&pCurYC->lValue,4);
		}
		else
		{
			fval2=pCurYC->lValue;
		}
		
		long maxval;
		if(GetYC_ABC(m_wEqpID,(WORD)i,NULL,&maxval,NULL)==OK)
		{
			if(maxval>50)
			{
				nChangeVal=maxval;
				nChangeVal=nChangeVal*m_pBaseCfg->wYCDeadValue/1000;
			}
		}
		//if(nChangeVal==0) nChangeVal=1;
		if(fval1> fval2)
		    wAbsVal = fval1- fval2;
		else 
			wAbsVal = fval2- fval1;
		if(wAbsVal!=0)
		if(wAbsVal >=nChangeVal)   m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=TRUE;
    }	
	
	pSendYC=(VDBYCF_L*)pBuf; 
	i=j=k=0; 	
	while ((j<wNum)&&(i< m_wYCNum[wEqpID]))
	{
	    if (m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[m_pEqpExtInfo[m_wEqpNo].wChangYCNo]==TRUE)
	    {
           pSendYC->wNo=m_pEqpExtInfo[m_wEqpNo].wChangYCNo;
		   pSendYC->lValue=m_pEqpExtInfo[m_wEqpNo].CurYC[m_pEqpExtInfo[m_wEqpNo].wChangYCNo].lValue;
		   pSendYC->byFlag=m_pEqpExtInfo[m_wEqpNo].CurYC[m_pEqpExtInfo[m_wEqpNo].wChangYCNo].byFlag;
		   m_pEqpExtInfo[m_wEqpNo].OldYC[m_pEqpExtInfo[m_wEqpNo].wChangYCNo].lValue=pSendYC->lValue;
		   m_pEqpExtInfo[m_wEqpNo].OldYC[m_pEqpExtInfo[m_wEqpNo].wChangYCNo].byFlag=pSendYC->byFlag;
		   pSendYC++;
		   j++;
		   k+=(int)sizeof(VDBYCF_L);
    	   if ((unsigned int)k>wBufLen-sizeof(VDBYCF_L)) break;
	    }
        m_pEqpExtInfo[m_wEqpNo].wChangYCNo++;
		if (m_pEqpExtInfo[m_wEqpNo].wChangYCNo >= m_wYCNum[wEqpID])
		   m_pEqpExtInfo[m_wEqpNo].wChangYCNo = 0; 		
		i++;
	}  
	return (WORD)j;	
}

void CMaint::SendChageYC(void)            	
{
	WORD YCNum,Shift;
	WORD YCNumOffSet,deadvalue;
	DWORD EqpName;
	WORD EqpID;
	float fd;
	DWORD dddd;
	BYTE ErrorCode=1;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

	YCNum = (MAX_FRAME_SIZE-50)/2;

    SendFrameHead(GET_CHG_YC);
    SendDWordLH(EqpName);
    m_SendBuf.wWritePtr += 2;
	if(m_pFrameData[10]!=0x16)
	{
		deadvalue=MAKEWORD(m_pFrameData[8], m_pFrameData[9]);//取死区值
		if((deadvalue>0)&&(deadvalue<1000))
			m_pBaseCfg->wYCDeadValue=deadvalue;
		//else
		//	m_pBaseCfg->wYCDeadValue=10;
	}
	m_wEqpNo=EqpID;
    SearchChangeYC( EqpID,YCNum, MAX_FRAME_SIZE-40, (VDBYCF_L *)m_dwPubBuf);
    
    
    Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    YCNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

    if (YCNum > ((MAX_FRAME_SIZE-50)/5))
    {
		YCNum = (MAX_FRAME_SIZE-50)/5;
	}
    SendFrameHead(GET_CHG_YC);
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    //发送遥测的原始值
    YCNumOffSet = m_SendBuf.wWritePtr; //记录AI个数存放位置
    m_SendBuf.wWritePtr += 2;
    SendWordLH(m_pBaseCfg->wYCDeadValue);
    WORD i;
    for ( i = 0;(i < YCNum)&&(i+Shift<m_wYCNum[EqpID]); i++)
    {
    	if((m_pEqpExtInfo[EqpID].OldYC+i+Shift)->byFlag&(1<<6))
		{
			memcpy((void *)&fd,(void*)&(m_pEqpExtInfo[EqpID].OldYC+i+Shift)->lValue,4);
    		SendDWordLH((DWORD)fd);
		}
		else
		{
			dddd=(DWORD)((m_pEqpExtInfo[EqpID].OldYC+i+Shift)->lValue);
    		SendDWordLH(dddd);
		}
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(m_pEqpExtInfo[EqpID].OldYC+i+Shift)->byFlag;
    }
    
	SendWordLH(YCNumOffSet, i);
	
	SendFrameTail();
}
#endif

/***************************************************************
	Function：SendDD
		发送电度数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendDD(void)
{
	WORD Shift;
	WORD DDNum;
	WORD DDNumOffSet;
	DWORD EqpName;
	WORD EqpID;
	BYTE ErrorCode=1;
	WORD RecDDNum;
	DWORD *pdwDD;
	DWORD dwDDValue;
	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    DDNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

    if (DDNum > ((MAX_FRAME_SIZE-50)/4))
    {
		DDNum = (MAX_FRAME_SIZE-50)/4;
	}
    SendFrameHead(GET_DD);
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    //发送遥测的原始值
    DDNumOffSet = m_SendBuf.wWritePtr; //记录AI个数存放位置
    m_SendBuf.wWritePtr += 2;
    RecDDNum = ReadRangeAllDD(EqpID, Shift, DDNum, MAX_FRAME_SIZE-10, (long *)m_dwPubBuf);
    
    if (RecDDNum == 0)
    {
    	ErrorCode = 2;//have no dd or dd num = 0
    	SendNACK(ErrorCode);
    	return;	
    }
    
    pdwDD = (DWORD *)m_dwPubBuf;
    for (int i = 0; i < RecDDNum; i++)
    {
    	dwDDValue = *pdwDD;
    	SendDWordLH(dwDDValue);
    	pdwDD ++;
    }
    
	SendWordLH(DDNumOffSet, RecDDNum);
	
	SendFrameTail();
    return;
}


/***************************************************************
	Function：SendSingleYX
		发送单点遥信数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendSingleYX(void)             	
{
	WORD Shift;
	WORD YXNum;
	WORD RecYXNum;
	WORD YXNumOffSet;
	DWORD EqpName;
	WORD EqpID;
    BYTE ErrorCode = 1;

/*    for(EqpID=0;EqpID<10000;EqpID++)
			{
			SendYC();
			thSleep(100);
		}*/
    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    YXNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥信数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }
	
	//if(g_Sys.Eqp.pInfo[EqpID].pTEqpInfo->Cfg.wSYXNum<YXNum)
	//	YXNum=g_Sys.Eqp.pInfo[EqpID].pTEqpInfo->Cfg.wSYXNum;
    if (YXNum > (MAX_FRAME_SIZE-50))
    {
		YXNum = MAX_FRAME_SIZE - 50;
	}
    	
    SendFrameHead(GET_YX_SINGLE);
   
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    
    YXNumOffSet = m_SendBuf.wWritePtr; //记录BI个数存放位置
    m_SendBuf.wWritePtr += 2;
	
	RecYXNum = ReadRangeAllSYX(EqpID, Shift, YXNum, MAX_FRAME_SIZE-10, &m_SendBuf.pBuf[m_SendBuf.wWritePtr]);
	
	if (RecYXNum == 0)
    {
    	ErrorCode = 2;//have no yx or yx num = 0
    	SendNACK(ErrorCode);
    	return;	
    }
	
	m_SendBuf.wWritePtr += RecYXNum;	
        
    SendWordLH(YXNumOffSet, RecYXNum);
    SendFrameTail();
	return;
}
/***************************************************************
	Function：SenddYX
		发送单点遥信数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SenddYX(void)             	
{
	WORD Shift;
	WORD YXNum;
	WORD RecYXNum;
	WORD YXNumOffSet;
	DWORD EqpName;
	WORD EqpID;
	VDYX* pdyx;
    BYTE ErrorCode = 1;
/*    for(EqpID=0;EqpID<10000;EqpID++)
			{
			SendYC();
			thSleep(100);
		}*/
    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    YXNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥信数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }
	
	//if(g_Sys.Eqp.pInfo[EqpID].pTEqpInfo->Cfg.wSYXNum<YXNum)
	//	YXNum=g_Sys.Eqp.pInfo[EqpID].pTEqpInfo->Cfg.wSYXNum;
    if (YXNum > (MAX_FRAME_SIZE-50)/2)
    {
		YXNum = (MAX_FRAME_SIZE - 50)/2;
	}
    	
    SendFrameHead(GET_DYX);
   
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    
    YXNumOffSet = m_SendBuf.wWritePtr; //记录BI个数存放位置
    m_SendBuf.wWritePtr += 2;
	
	RecYXNum = ReadRangeAllDYX(EqpID, Shift, YXNum, MAX_FRAME_SIZE/2, (VDYX*)m_dwPubBuf);
	pdyx=(VDYX*)m_dwPubBuf;
	if (RecYXNum == 0)
    {
    	ErrorCode = 2;//have no yx or yx num = 0
    	SendNACK(ErrorCode);
    	return;	
    }
		for(int i=0;i<RecYXNum;i++)
			{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr ++]=pdyx->byValue1;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr ++]=pdyx->byValue2;
			pdyx++;
		}
	
	//m_SendBuf.wWritePtr += RecYXNum*2;	
        
    SendWordLH(YXNumOffSet, RecYXNum);
    SendFrameTail();
	return;
}

/***************************************************************
	Function：SendSingleSOE
		发送单点SOE数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendSingleSOE(void)
{
	WORD SOENum;
	WORD ReadPtr;
	WORD WritePtr;
	WORD BufLen;//环行缓冲池大小
	WORD RecSOENum;
	WORD SOENumOffSet;
	DWORD EqpName;
	WORD EqpID;
    BYTE ErrorCode = 1;
    BYTE *pSOEBuf;
    VDBSOE *pRecSOE;
    
    
    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    SOENum = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
    ReadPtr = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }
    
    SendFrameHead(GET_SOE_SINGLE);
	SendDWordLH(EqpName);
    SOENumOffSet = m_SendBuf.wWritePtr; //记录SOE个数存放位置
    m_SendBuf.wWritePtr += 2;
    SendWordLH(ReadPtr);
    
    RecSOENum = QuerySSOE(EqpID, SOENum, ReadPtr, MAX_FRAME_SIZE-10,
    		(VDBSOE *)m_dwPubBuf, &WritePtr, &BufLen);
    
    pSOEBuf = &m_SendBuf.pBuf[m_SendBuf.wWritePtr+4];
    pRecSOE = (VDBSOE *)m_dwPubBuf;
    for (int i=0; i<RecSOENum; i++)
    {
    	pSOEBuf[0] = LOBYTE(pRecSOE->wNo);
    	pSOEBuf[1] = HIBYTE(pRecSOE->wNo);
    	pSOEBuf[2] = pRecSOE->byValue;
    	pSOEBuf[3] = LOBYTE(LOWORD(pRecSOE->Time.dwMinute));
    	pSOEBuf[4] = HIBYTE(LOWORD(pRecSOE->Time.dwMinute));
    	pSOEBuf[5] = LOBYTE(HIWORD(pRecSOE->Time.dwMinute));
    	pSOEBuf[6] = HIBYTE(HIWORD(pRecSOE->Time.dwMinute));
    	pSOEBuf[7] = LOBYTE(pRecSOE->Time.wMSecond);
    	pSOEBuf[8] = HIBYTE(pRecSOE->Time.wMSecond);
    	pSOEBuf += 9;//sizeof(VDBSOE);//for debug
    	pRecSOE ++;
    }
    
    SendWordLH(WritePtr);
    SendWordLH(BufLen);
    m_SendBuf.wWritePtr += (WORD)(9 * RecSOENum);//for debug//sizeof(VDBSOE) * RecSOENum;
    SendWordLH(SOENumOffSet, RecSOENum);
    SendFrameTail();
	return;
}
/***************************************************************
	Function：SendDSOE
		发送单点SOE数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendDSOE(void)
{
	WORD SOENum;
	WORD ReadPtr;
	WORD WritePtr;
	WORD BufLen;//环行缓冲池大小
	WORD RecSOENum;
	WORD SOENumOffSet;
	DWORD EqpName;
	WORD EqpID;
    BYTE ErrorCode = 1;
    BYTE *pSOEBuf = NULL;
    VDBDSOE *pRecSOE = NULL;
    
    
    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    SOENum = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
    ReadPtr = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }
    
    SendFrameHead(GET_DSOE);
	SendDWordLH(EqpName);
    SOENumOffSet = m_SendBuf.wWritePtr; //记录SOE个数存放位置
    m_SendBuf.wWritePtr += 2;
    SendWordLH(ReadPtr);
    
    RecSOENum = QueryDSOE(EqpID, SOENum, ReadPtr, MAX_FRAME_SIZE-10,
    		(VDBDSOE *)m_dwPubBuf, &WritePtr, &BufLen);
    
    pSOEBuf = &m_SendBuf.pBuf[m_SendBuf.wWritePtr+4];
    pRecSOE = (VDBDSOE *)m_dwPubBuf;
    for (int i=0; i<RecSOENum; i++)
    {
    	pSOEBuf[0] = LOBYTE(pRecSOE->wNo);
    	pSOEBuf[1] = HIBYTE(pRecSOE->wNo);
    	pSOEBuf[2] = pRecSOE->byValue1;
    	pSOEBuf[3] = pRecSOE->byValue2;
    	pSOEBuf[4] = LOBYTE(LOWORD(pRecSOE->Time.dwMinute));
    	pSOEBuf[5] = HIBYTE(LOWORD(pRecSOE->Time.dwMinute));
    	pSOEBuf[6] = LOBYTE(HIWORD(pRecSOE->Time.dwMinute));
    	pSOEBuf[7] = HIBYTE(HIWORD(pRecSOE->Time.dwMinute));
    	pSOEBuf[8] = LOBYTE(pRecSOE->Time.wMSecond);
    	pSOEBuf[9] = HIBYTE(pRecSOE->Time.wMSecond);
    	pSOEBuf += 10;//sizeof(VDBSOE);//for debug
    	pRecSOE ++;
    }
    
    SendWordLH(WritePtr);
    SendWordLH(BufLen);
    m_SendBuf.wWritePtr += 10 * RecSOENum;//for debug//sizeof(VDBSOE) * RecSOENum;
    SendWordLH(SOENumOffSet, RecSOENum);
    SendFrameTail();
	return;
}
/***************************************************************
	Function：SendSingleSOE
		发送单点SOE数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendEvent(void)
{
	WORD SOENum;
	WORD ReadPtr;
	WORD WritePtr;
	WORD BufLen;//环行缓冲池大小
	WORD RecSOENum;
	WORD SOENumOffSet;
	DWORD EqpName;
//	WORD EqpID;
	WORD EventType;
//    BYTE ErrorCode = 1;
//    VDBSOE *pRecSOE = NULL;
    	struct VSysEventInfo m_Eventinfo[3];

    
    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    SOENum = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
    ReadPtr = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);
    EventType = MAKEWORD(m_pFrameData[12], m_pFrameData[13]);
	
 
    
    SendFrameHead(GET_EVENT);
	SendDWordLH(EqpName);
    SOENumOffSet = m_SendBuf.wWritePtr; //记录SOE个数存放位置
    m_SendBuf.wWritePtr += 2;
    SendWordLH(ReadPtr);
	if(SOENum>3) SOENum=3;
    if(EventType==0)
		RecSOENum=QuerySysEventInfo(g_Sys.pActEvent,SOENum,ReadPtr,sizeof(m_Eventinfo[0])*3,m_Eventinfo,&WritePtr,&BufLen);
	else  if(EventType==1)
		RecSOENum=QuerySysEventInfo(g_Sys.pDoEvent,SOENum,ReadPtr,sizeof(m_Eventinfo[0])*3,m_Eventinfo,&WritePtr,&BufLen);
    else  if(EventType == 2)
		RecSOENum=QuerySysEventInfo(g_Sys.pWarnEvent,SOENum,ReadPtr,sizeof(m_Eventinfo[0])*3,m_Eventinfo,&WritePtr,&BufLen);
	else if(EventType == 3)
        RecSOENum=QuerySysEventInfo(g_Sys.pFaEvent,SOENum,ReadPtr,sizeof(m_Eventinfo[0])*3,m_Eventinfo,&WritePtr,&BufLen);
	else
        RecSOENum = 0;

	    SendWordLH(WritePtr);
	    SendWordLH(BufLen);
	    SendWordLH(EventType);
		for(int i=0;i<RecSOENum;i++)
			{
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=m_Eventinfo[i].time.dwMinute&0xff;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(m_Eventinfo[i].time.dwMinute>>8)&0xff;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(m_Eventinfo[i].time.dwMinute>>16)&0xff;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(m_Eventinfo[i].time.dwMinute>>24)&0xff;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(m_Eventinfo[i].time.wMSecond)&0xff;
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(m_Eventinfo[i].time.wMSecond>>8)&0xff;
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr, m_Eventinfo[i].msg, SYS_LOG_MSGLEN);
    m_SendBuf.wWritePtr+=SYS_LOG_MSGLEN;
			}
        SendWordLH(SOENumOffSet, RecSOENum);
SendFrameTail();
	return;
}

/***************************************************************
	Function：Senddotdef
		发送点号描述
	参数：无
		
	返回：无
***************************************************************/
void CMaint::Senddotdef(void)
{
	WORD dotoff;
	WORD dotnum;
	WORD SOENumOffSet;
	DWORD EqpName;
	WORD EqpID;
	WORD EventType;
	WORD datalen;
    BYTE ErrorCode = 1;
	WORD rcvlen = 0;
//  VDBSOE *pRecSOE = NULL;
//  struct VSysEventInfo m_Eventinfo[3];

    
    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	EventType=m_pFrameData[4];
	dotoff = MAKEWORD(m_pFrameData[5], m_pFrameData[6]);
    dotnum = MAKEWORD(m_pFrameData[7], m_pFrameData[8]);
	if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }
 
    
    SendFrameHead(GET_dotdef);
	SendDWordLH(EqpName);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(BYTE)EventType;
    SendWordLH(dotoff);
    SOENumOffSet = m_SendBuf.wWritePtr; //记录SOE个数存放位置
    m_SendBuf.wWritePtr += 2;
	datalen=dotnum;
	if(dotnum>20) dotnum=20;
	switch(EventType)
		{
		case 1:/*遥测描述*/
			dotnum=read_ycdef(EqpID,dotoff,dotnum);
			break;
		case 2:/*单点遥信描述*/
			dotnum=read_yxdef(EqpID,dotoff,dotnum);
			break;
		case 3:/*双点遥信描述*/
			dotnum=read_dyxdef(EqpID,dotoff,dotnum);
			break;
		case 4:/*压板描述*/
			dotnum=read_ybdef(dotoff,dotnum);
			break;
		case 5:/*定值描述*/
			//dotnum=read_dzdef(dotoff,datalen);
			if(Maint_BM_RCV(GET_dotdef,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3,
			(BYTE*)m_dwPubBuf, &rcvlen) == ERROR)
			{
				SendNACK(ErrorCode);
				return;
			}
			SendFrameHead(GET_dotdef);
			memcpy(m_SendBuf.pBuf + m_SendBuf.wWritePtr, m_dwPubBuf ,rcvlen);
			m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + rcvlen;
			SendFrameTail();
			return;
			break;
		case 6:/*电度描述*/
			dotnum=read_dddef(EqpID,dotoff,dotnum);
			break;
		case 7:/*遥控描述*/
			dotnum=read_ykdef(EqpID,dotoff,dotnum);
			break;
		case 8:/*回线描述*/
			dotnum=read_hxdef(dotoff,dotnum);
			break;
		case 9:/*固有信息点表信息*/
			dotnum=read_selfparadef(dotoff,dotnum);
			break;
		case 10:/*运行信息部分点表信息*/
			dotnum= read_runparadef(dotoff,dotnum);
			break;
		case 12:/*遥调描述*/
			dotnum=read_ytdef(EqpID,dotoff,dotnum);
			break;		
	}
    SendWordLH(SOENumOffSet, dotnum);
	SendFrameTail();
	return;
}
/***************************************************************
	Function：SendSingleSOE
		发送单点SOE数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::Sendruninfo(void)
{
	WORD SOENum;
	WORD ReadPtr;
//	WORD WritePtr;
//	WORD BufLen;//环行缓冲池大小
	WORD RecSOENum;
	WORD SOENumOffSet;
	DWORD EqpName;
//	char dt[]="版本1.2 2012-3-25";
//  VDBSOE *pRecSOE = NULL;
//	char tt[100];
//	struct VSysEventInfo m_Eventinfo[3];
//	DWORD msd;

    
    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    SOENum = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
    ReadPtr = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);

	if (ReadPtr >= sysLog.wWrtPtr)
		SendNACK(1);

	
    SendFrameHead(GET_RUNINFO);
	SendDWordLH(EqpName);
    SOENumOffSet = m_SendBuf.wWritePtr; //记录SOE个数存放位置
    m_SendBuf.wWritePtr += 2;
    SendWordLH(ReadPtr);
	if(SOENum>10) SOENum=10;
	RecSOENum=0;
	SendWordLH(sysLog.wWrtPtr);
	SendWordLH(SYS_LOG_BUFNUM);
	for(int i=0;(i<SOENum)&&(i+ReadPtr<sysLog.wWrtPtr);i++)
	{
	  /*if(i+ReadPtr==0)
		{
			strcpy((char*)m_SendBuf.pBuf+m_SendBuf.wWritePtr,dt);
			m_SendBuf.wWritePtr+=sizeof(dt);
		}*/
		//sprintf((char*)m_dwPubBuf,"%s:%s",GetThName(sysLog.aLogMsg[ReadPtr].thid),sysLog.aLogMsg[ReadPtr].sMsg);
		if(strlen(sysLog.aLogMsg[ReadPtr+i].sMsg)==0) break ;
		if(strlen(GetThName(sysLog.aLogMsg[ReadPtr+i].thid))>128) break ;
		strcpy((char*)m_SendBuf.pBuf+m_SendBuf.wWritePtr,GetThName(sysLog.aLogMsg[ReadPtr+i].thid));
	    m_SendBuf.wWritePtr+=(WORD)strlen(GetThName(sysLog.aLogMsg[ReadPtr+i].thid));
		if(strlen(sysLog.aLogMsg[ReadPtr+i].sMsg)>128) break;
		strcpy((char*)m_SendBuf.pBuf+m_SendBuf.wWritePtr,": ");
		m_SendBuf.wWritePtr+=2;
		strcpy((char*)m_SendBuf.pBuf+m_SendBuf.wWritePtr,sysLog.aLogMsg[ReadPtr+i].sMsg);
	    m_SendBuf.wWritePtr+=(WORD)strlen(sysLog.aLogMsg[ReadPtr+i].sMsg);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
		RecSOENum++;
	}
    SendWordLH(SOENumOffSet, RecSOENum);
	SendFrameTail();
	return;
}

/***************************************************************
	Function：SendSingleCOS
		发送单点COS数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendSingleCOS(void)
{
	WORD COSNum;
	WORD ReadPtr;
	WORD WritePtr;
	WORD BufLen;
	WORD RecCOSNum;
	WORD COSNumOffSet;
	DWORD EqpName;
	WORD EqpID;
    BYTE ErrorCode = 1;
    //VDBCOS COSData;
    BYTE *pCOSBuf; 
    VDBCOS *pRecCOS;

    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    COSNum = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
    ReadPtr = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }
    
    SendFrameHead(GET_COS_SINGLE);
	SendDWordLH(EqpName);
    COSNumOffSet = m_SendBuf.wWritePtr; //记录COS个数存放位置
    m_SendBuf.wWritePtr += 2;
    SendWordLH(ReadPtr);
    
    RecCOSNum = QuerySCOS(EqpID, COSNum, ReadPtr, MAX_FRAME_SIZE-10,
    		(VDBCOS *)m_dwPubBuf, &WritePtr, &BufLen);
    
    pCOSBuf = &m_SendBuf.pBuf[m_SendBuf.wWritePtr+4];
    pRecCOS = (VDBCOS *)m_dwPubBuf;
    for (int i=0; i<RecCOSNum; i++)
    {
    	//memcpy((void *)&COSData, (const void *)pRecCOS, sizeof(VDBCOS));
    	pCOSBuf[0] = LOBYTE(pRecCOS->wNo);
    	pCOSBuf[1] = HIBYTE(pRecCOS->wNo);
    	pCOSBuf[2] = pRecCOS->byValue;
    	pCOSBuf += 3;//for debug//sizeof(VDBCOS);
    	pRecCOS ++;
    }
    
    SendWordLH(WritePtr);
    SendWordLH(BufLen);
    m_SendBuf.wWritePtr += (WORD)(3 * RecCOSNum);//for debug//sizeof(VDBCOS)  * RecCOSNum;
    SendWordLH(COSNumOffSet, RecCOSNum);
    SendFrameTail();
	return;
}

/***************************************************************
	Function： SetYKCommand
		设置遥控命令
	参数：Command
		Command 遥控命令码
	返回：无
***************************************************************/
void CMaint::SetYKCommand(BYTE Command)
{
	DWORD EqpName;
	WORD EqpID;
	WORD YKNo;
	BYTE ErrorCode = 1;
	VDBYK *pDBYK;
	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    YKNo = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
    m_dwEqpName = EqpName;
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }

    pDBYK=(VDBYK *)m_pMsg->abyData;
    pDBYK->wID = YKNo;
    pDBYK->byValue = m_pFrameData[6];    

    TaskSendMsg(DB_ID, m_wTaskID,EqpID, Command, MA_REQ, sizeof(VDBYK), m_pMsg);
}

DWORD CMaint::DoCommState(void)
{
	DWORD dwCommState;
	dwCommState=CPSecondary::DoCommState();
		
    return(dwCommState);	
}

void CMaint::DoYTSet(void)
{
	DWORD EqpName;
	WORD EqpID;
	WORD YTNo, wYTValue;
	BYTE ErrorCode = 1;
	VDBYT *pDBYT;
	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    YTNo = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
    m_dwEqpName = EqpName;
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }
    wYTValue = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);
    pDBYT = (VDBYT *)m_pMsg->abyData;
    pDBYT->wID = YTNo;
    pDBYT->dwValue = wYTValue;    
    TaskSendMsg(DB_ID,m_wTaskID,EqpID, MI_YTOPRATE, MA_REQ, sizeof(VDBYT), m_pMsg);
	return;
}

/***************************************************************
	Function：SendFile
		发送文件数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendFile(void)
{
	FileStatus filestatus;
	VFileOPMsg FileOPMsg;
	BYTE FileEndFlag;
	WORD PackLenOffset;
	WORD EndFlagOffset;
	BYTE ErrorCode=1;
	DWORD FileLenOffset;
	char filerealname[4*MAXFILENAME];
	
	m_pFrameData[FILE_NAME_LEN*2 - 1] = 0;	//强制将文件名最后一个字符设置为空字符	
	if(m_pFrameData[0]>0xa0)
		strcpy((char *)FileOPMsg.cFileName, SYS_PATH_ROOT);
	else		
		strcpy((char *)FileOPMsg.cFileName, (char *)m_pFrameData);
	sprintf(filerealname,"%s%s",SYS_PATH,FileOPMsg.cFileName);
	
	m_pFrameData += FILE_NAME_LEN*2;
	FileOPMsg.dwSize = 0;
	FileOPMsg.dwOffset = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	FileOPMsg.dwLen = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
	
	SendFrameHead(READ_FILE);
	strcpy((char *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (char *)FileOPMsg.cFileName);
	m_SendBuf.wWritePtr += FILE_NAME_LEN*2;
	
	FileLenOffset = m_SendBuf.wWritePtr;
	m_SendBuf.wWritePtr += 4;//sizeof(DWORD);
	SendDWordLH(FileOPMsg.dwOffset);
	PackLenOffset = m_SendBuf.wWritePtr;
	m_SendBuf.wWritePtr += 2;
	EndFlagOffset = m_SendBuf.wWritePtr;
	m_SendBuf.wWritePtr += 1;

	strcpy(FileOPMsg.cFileName,filerealname);
	filestatus = ReadFile(&FileOPMsg, &m_SendBuf.pBuf[m_SendBuf.wWritePtr]);
	if (((int)filestatus == FILEOK) || ((int)filestatus == FILEEOF))
	{
		m_SendBuf.wWritePtr += (WORD)FileOPMsg.dwLen;
		if ((int)filestatus == FILEOK)
		{
			FileEndFlag = 0;	
		}
		else
		{
			FileEndFlag = 0xff;
		}
	}
	else
	{
		SendNACK(ErrorCode);
		return;
	}
	SendDWordLH(FileLenOffset, FileOPMsg.dwSize);
	SendWordLH(PackLenOffset, (WORD)FileOPMsg.dwLen);
	m_SendBuf.pBuf[EndFlagOffset] = FileEndFlag;
	SendFrameTail();
	return;
}


/***************************************************************
	Function：ReceiveFile
		接收文件数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::ReceiveFile(void)
{
	FileStatus filestatus = FileEof;
	VFileOPMsg FileOPMsg;
	/*BYTE FileEndFlag;*/
	/*BOOL FileEnd = FALSE;*/
	BYTE ErrorCode = 1;
	char *ppfilename;
	char *ppfilename1;
	static DWORD fileoffsetlen = 0;
	DWORD ulfileoffset =0;
	BYTE* filetemp;
	m_pFrameData[FILE_NAME_LEN*2 - 1] = 0;	//强制将文件名最后一个字符设置为空字符
	sprintf((char *)FileOPMsg.cFileName, "%s%s", SYS_PATH, (char *)m_pFrameData);
	m_pFrameData += FILE_NAME_LEN*2;
	
	FileOPMsg.dwSize = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	FileOPMsg.dwOffset = MAKEDWORD(MAKEWORD(m_pFrameData[4], m_pFrameData[5]), MAKEWORD(m_pFrameData[6], m_pFrameData[7]));
	FileOPMsg.dwLen = MAKEWORD(m_pFrameData[8], m_pFrameData[9]);
	/*FileEndFlag = m_pFrameData[10];*/
	/*if (FileEndFlag == 0xff)
	{
		FileEnd = TRUE;	
	}*/
	m_pFrameData += 11;
	ppfilename=strchr(FileOPMsg.cFileName,'.');
	ppfilename1=NULL;
	if(ppfilename!=NULL)
	{
		ppfilename++;
		ppfilename1=ppfilename;
		while((ppfilename=strchr(ppfilename,'.'))!=NULL)
		{
			ppfilename++;
			ppfilename1=ppfilename;
		}
	}
	if((ppfilename1!=NULL)&&(ppfilename1[0]=='b')&&(ppfilename1[2]=='n'))
	{
#if 0
		BYTE addr;

		filestatus=FileError;
			if(FileOPMsg.dwOffset==0)
				{fileoffset=0;
					for(addr=m25p_file_sector_star;addr<64;addr++)
					M25P_SectorErase(2,addr);
					M25P_AddrWrite(2, m25p_file_addr_star+fileoffset+80, m_pFrameData, FileOPMsg.dwLen);
					fileoffset+=FileOPMsg.dwLen;
					filestatus=FileOk;
					}
			else if(fileoffset==FileOPMsg.dwOffset)
				{

					M25P_AddrWrite(2, m25p_file_addr_star+FileOPMsg.dwOffset+80, m_pFrameData, FileOPMsg.dwLen);
					fileoffset=FileOPMsg.dwOffset+FileOPMsg.dwLen;
					filestatus=FileOk;
					if(fileoffset>=FileOPMsg.dwSize)
						{
								m_pFrameData[0]=0x55;
								m_pFrameData[1]=0xaa;
								m_pFrameData[2]=0x55;
								m_pFrameData[3]=0xaa;
								strcpy((char*)m_pFrameData+16,FileOPMsg.cFileName);
								m_pFrameData[60]=fileoffset&0xff;
								m_pFrameData[61]=(fileoffset>>8)&0xff;
								m_pFrameData[62]=(fileoffset>>16)&0xff;
								m_pFrameData[63]=(fileoffset>>24)&0xff;
					M25P_AddrWrite(2, m25p_file_addr_star, m_pFrameData, 72);
						}
					}
#endif					
	}
	else
	{
		filetemp = (BYTE*)g_pParafile;
		
		if(FileOPMsg.dwOffset == 0)
			fileoffsetlen = 0;
			
		if((FileOPMsg.dwOffset - fileoffsetlen) == 0)
		{
			if(FileOPMsg.dwSize < MAXFILELEN)
				memset((void *)filetemp,0,FileOPMsg.dwSize);
			else
				memset((void *)filetemp,0,MAXFILELEN);
		}
		memcpy(filetemp+FileOPMsg.dwOffset - fileoffsetlen,(const void *)m_pFrameData,FileOPMsg.dwLen);
		filestatus = FileOk;
		
		if(FileOPMsg.dwSize >= MAXFILELEN)
		{
			if(FileOPMsg.dwSize == FileOPMsg.dwOffset + FileOPMsg.dwLen)
			{
				FileOPMsg.dwOffset = fileoffsetlen;
				FileOPMsg.dwLen = FileOPMsg.dwSize - fileoffsetlen;
				fileoffsetlen = 0;
				filestatus = WriteFile(&FileOPMsg, filetemp);
			}
			else if((FileOPMsg.dwOffset - (fileoffsetlen + 2048)) > MAXFILELEN)
			{
				ulfileoffset = fileoffsetlen;//之前的偏移
				fileoffsetlen = FileOPMsg.dwOffset + FileOPMsg.dwLen;
				FileOPMsg.dwOffset = ulfileoffset;
				FileOPMsg.dwLen = fileoffsetlen - ulfileoffset;
				
				filestatus = WriteFile(&FileOPMsg, filetemp);
			}
		}
		else if(FileOPMsg.dwSize == FileOPMsg.dwOffset + FileOPMsg.dwLen)
		{
			FileOPMsg.dwOffset = 0;
			FileOPMsg.dwLen = FileOPMsg.dwSize;
			filestatus = WriteFile(&FileOPMsg, filetemp);
		}
	}				
	//filestatus = WriteFile(&FileOPMsg, m_pFrameData);
	//添加容错处理
	if (filestatus == FileOk)
	{
		SendACK();	
	}
	else
	{
		SendNACK(ErrorCode);
		fileoffsetlen = 0;
	}
	return;
}

void CMaint::SendDir(void)
{
	FileStatus filestatus;
	VFileOPMsg FileOPMsg;
	WORD DirNum;
	VDirInfo *pDirInfo;
	BYTE *pFrame;
	BYTE ErrorCode = 1;	
	char filerealname[4*MAXFILENAME];
	//{{
	
	WORD wReadPtr, wRecOffset;
	WORD wTotleDirNum;
	
	//}}
	m_pFrameData[FILE_NAME_LEN*2 - 1] = 0;	//强制将文件名最后一个字符设置为空字符
	strcpy((char *)FileOPMsg.cFileName, (char *)m_pFrameData);
	sprintf(filerealname,"%s%s",SYS_PATH,FileOPMsg.cFileName); //将目录添加前缀。
	
	FileOPMsg.dwOffset = 0;
	FileOPMsg.dwLen = sizeof(VDirInfo)*4;//sizeof(VDirInfo)*10;//read max 10 items each time
	//{{
	
	m_pFrameData += FILE_NAME_LEN*2;
	wRecOffset = MAKEWORD(m_pFrameData[0], m_pFrameData[1]);	
	FileOPMsg.dwOffset = wRecOffset;
	
	//}}
	SendFrameHead(READ_DIR);
	strcpy((char *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (char *)FileOPMsg.cFileName);
	m_SendBuf.wWritePtr += FILE_NAME_LEN*2;

	//将文件目录修改为实际目录
	strcpy(FileOPMsg.cFileName,filerealname);
	
	filestatus = ListDir(&FileOPMsg, (VDirInfo *)&m_dwPubBuf);
	
	DirNum = (WORD)FileOPMsg.dwLen /(sizeof(VDirInfo));//for debug//sizeof(VDirInfo);//44
	pDirInfo = (VDirInfo *)&m_dwPubBuf;
	
	//{{
	
	wReadPtr = LOWORD(FileOPMsg.dwOffset);
	wTotleDirNum = LOWORD(FileOPMsg.dwSize);
	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(wReadPtr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(wReadPtr);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(DirNum);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(DirNum);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(wTotleDirNum);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = HIBYTE(wTotleDirNum);
	
	//}}
	
	pFrame = &m_SendBuf.pBuf[m_SendBuf.wWritePtr];
	if (filestatus == FileOk)
	{
		for (int i = 0; i < DirNum; i++)
		{
			strcpy((char *)pFrame, (char *)pDirInfo->cName);
			pFrame += FILE_NAME_LEN*2;
			pFrame[0] = LOBYTE(LOWORD(pDirInfo->dwSize));
			pFrame[1] = HIBYTE(LOWORD(pDirInfo->dwSize));
			pFrame[2] = LOBYTE(HIWORD(pDirInfo->dwSize));
			pFrame[3] = HIBYTE(HIWORD(pDirInfo->dwSize));
			pFrame[4] = LOBYTE(LOWORD(pDirInfo->dwMode));
			pFrame[5] = HIBYTE(LOWORD(pDirInfo->dwMode));
			pFrame[6] = LOBYTE(HIWORD(pDirInfo->dwMode));
			pFrame[7] = HIBYTE(HIWORD(pDirInfo->dwMode));
			pFrame[8] = LOBYTE(LOWORD(pDirInfo->dwTime));
			pFrame[9] = HIBYTE(LOWORD(pDirInfo->dwTime));
			pFrame[10] = LOBYTE(HIWORD(pDirInfo->dwTime));
			pFrame[11] = HIBYTE(HIWORD(pDirInfo->dwTime));
			pFrame += 4 * 3;//(sizeof(DWORD) * 3);
			pDirInfo ++;
		}
		m_SendBuf.wWritePtr += (WORD)(FILE_NAME_LEN*2*DirNum+12*DirNum);
	}
	else 
	{
		SendNACK(ErrorCode);
		return;	
	}
	SendFrameTail();
	return;
}



/***************************************************************
	Function：BuildDir
		创建目录
	参数：无
		
	返回：无
***************************************************************/
void CMaint::BuildDir(void)
{
	FileStatus filestatus;
	char DirName[FILE_NAME_LEN*2];
	BYTE ErrorCode = 1;
	m_pFrameData[FILE_NAME_LEN*2 - 1] = 0;	//强制将文件名最后一个字符设置为空字符

	sprintf(DirName,"%s%s", SYS_PATH,(char *)m_pFrameData); //加上后缀
	filestatus = MakeDir(DirName);
	//添加容错处理
	if (filestatus == FileOk)
	{
		SendACK();	
	}
	else
	{
		SendNACK(ErrorCode);
	}
	return;
}

/***************************************************************
	Function：RenameFile
		文件改名
	参数：无
		
	返回：无
***************************************************************/
void CMaint::RenameFile(void)
{
	FileStatus filestatus;
	char OldFileName[FILE_NAME_LEN*2];
	char NewFileName[FILE_NAME_LEN*2];
	BYTE ErrorCode = 1;
	m_pFrameData[FILE_NAME_LEN*2 - 1] = 0;	//强制将文件名最后一个字符设置为空字符
	sprintf((char *)OldFileName, "%s%s", SYS_PATH, (char *)m_pFrameData);
	m_pFrameData += FILE_NAME_LEN*2;
	sprintf((char *)NewFileName, "%s%s", SYS_PATH, (char *)m_pFrameData);
	filestatus = FileRename((char *)OldFileName,(char *)NewFileName);
	//添加容错处理
	if (filestatus == FileOk)
	{
		SendACK();	
	}
	else
	{
		SendNACK(ErrorCode);
	}
	return;
}


void CMaint::SendFileStatus(void)
{
	FileStatus filestatus;
	VFileOPMsg FileOPMsg;
	VFileStatus *pFileStatus;
	BYTE *pFrame;
	BYTE ErrorCode = 1;	
	char filenametemp[4*MAXFILENAME];
	
	m_pFrameData[FILE_NAME_LEN*2 - 1] = 0;	//强制将文件名最后一个字符设置为空字符
	strcpy((char *)FileOPMsg.cFileName, (char *)m_pFrameData);
	FileOPMsg.dwOffset = 0;
	FileOPMsg.dwLen = MAX_SEND_LEN - (FILE_NAME_LEN*2 + 9);
	FileOPMsg.dwSize = 0;
	
	SendFrameHead(GET_FILE_ATTR);
	strcpy((char *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr], (char *)FileOPMsg.cFileName);
	m_SendBuf.wWritePtr += FILE_NAME_LEN*2;

	//先加后去
	strcpy(filenametemp,FileOPMsg.cFileName);
	sprintf(FileOPMsg.cFileName,"%s%s", SYS_PATH, filenametemp);
	filestatus = ReadFileStatus(&FileOPMsg, (VFileStatus *)&m_dwPubBuf);
	strcpy(FileOPMsg.cFileName,filenametemp);
	
	pFileStatus = (VFileStatus *)&m_dwPubBuf;
	pFrame = &m_SendBuf.pBuf[m_SendBuf.wWritePtr];
	if (filestatus == FileOk)
	{
		strcpy((char *)pFrame, (char *)FileOPMsg.cFileName);
		pFrame += FILE_NAME_LEN*2;
		pFrame[0] = LOWORD(LOBYTE(pFileStatus->dwSize));
		pFrame[1] = LOWORD(HIBYTE(pFileStatus->dwSize));
		pFrame[2] = HIWORD(LOBYTE(pFileStatus->dwSize));
		pFrame[3] = HIWORD(HIBYTE(pFileStatus->dwSize));
		
		pFrame[4] = LOWORD(LOBYTE(pFileStatus->dwMode));
		pFrame[5] = LOWORD(HIBYTE(pFileStatus->dwMode));
		pFrame[6] = HIWORD(LOBYTE(pFileStatus->dwMode));
		pFrame[7] = HIWORD(HIBYTE(pFileStatus->dwMode));
		
		pFrame[8] = LOWORD(LOBYTE(pFileStatus->dwTime));
		pFrame[9] = LOWORD(HIBYTE(pFileStatus->dwTime));
		pFrame[10] = HIWORD(LOBYTE(pFileStatus->dwTime));
		pFrame[11] = HIWORD(HIBYTE(pFileStatus->dwTime));
		
		pFrame[12] = LOBYTE(pFileStatus->wCrc);
		pFrame[13] = HIBYTE(pFileStatus->wCrc);
		
		
		pFrame[14] = LOWORD(LOBYTE(pFileStatus->dwTempSize));
		pFrame[15] = LOWORD(HIBYTE(pFileStatus->dwTempSize));
		pFrame[16] = HIWORD(LOBYTE(pFileStatus->dwTempSize));
		pFrame[17] = HIWORD(HIBYTE(pFileStatus->dwTempSize));
		
		pFrame[18] = LOWORD(LOBYTE(pFileStatus->dwTempMode));
		pFrame[19] = LOWORD(HIBYTE(pFileStatus->dwTempMode));
		pFrame[20] = HIWORD(LOBYTE(pFileStatus->dwTempMode));
		pFrame[21] = HIWORD(HIBYTE(pFileStatus->dwTempMode));
		
		pFrame[22] = LOWORD(LOBYTE(pFileStatus->dwTempTime));
		pFrame[23] = LOWORD(HIBYTE(pFileStatus->dwTempTime));
		pFrame[24] = HIWORD(LOBYTE(pFileStatus->dwTempTime));
		pFrame[25] = HIWORD(HIBYTE(pFileStatus->dwTempTime));
		
		pFrame[26] = LOBYTE(pFileStatus->wTempCrc);
		pFrame[27] = HIBYTE(pFileStatus->wTempCrc);
		
		
		m_SendBuf.wWritePtr += (WORD)FileOPMsg.dwLen;
	}
	else 
	{
		SendNACK(ErrorCode);
		return;	
	}
	SendFrameTail();
	return;
}



/***************************************************************
	Function：删除文件
		
	参数：无
		
	返回：无
***************************************************************/
void CMaint::DeleteFile(void)
{
	FileStatus filestatus;
	char FileName[FILE_NAME_LEN*2];
	BYTE ErrorCode = 1;
	m_pFrameData[FILE_NAME_LEN*2 - 1] = 0;	//强制将文件名最后一个字符设置为空字符
	sprintf((char *)FileName, "%s%s", SYS_PATH, (char *)m_pFrameData);
	filestatus = DelFile((char *)FileName);
	//添加容错处理
	
	if (filestatus == FileOk)
	{
		SendACK();	
	}
	else
	{
		SendNACK(ErrorCode);
	}
	return;
}

void CMaint::DeleteDir(void)
{
	FileStatus filestatus;
	char DirName[FILE_NAME_LEN*2];
	BYTE ErrorCode = 1;
	m_pFrameData[FILE_NAME_LEN*2 - 1] = 0;	//强制将文件名最后一个字符设置为空字符
	sprintf((char *)DirName, "%s%s", SYS_PATH, (char *)m_pFrameData);
	filestatus = DelDir((char *)DirName);
	//添加容错处理
	
	if (filestatus == FileOk)
	{
		SendACK();	
	}
	else
	{
		SendNACK(ErrorCode);
	}
	return;
}

/***************************************************************
	Function：SendACK
		发送ACK应答报文
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendACK(void)
{
	SendBaseFrame(ACK);
	return;
}


/***************************************************************
	Function：SendNACK
		发送NACK应答报文
	参数：ErrorCode
		ErrorCode 错误代码
	返回：无
***************************************************************/
void CMaint::SendNACK(BYTE ErrorCode)
{
	SendFrameHead(NACK);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = ErrorCode;
    SendFrameTail();
	return;
}

/***************************************************************
	Function：DoReceive
		接收报文处理
	参数：无
		
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CMaint::DoReceive()
{
	DWORD dwCode;
	DWORD para;
	m_sendcount=0;
	
	if (SearchFrame() != TRUE)
	{
		return FALSE;//没找到有效报文
	}
		
	m_pReceive = (VFrame *)m_RecFrame.pBuf;
	
	
	m_pFrameData = (BYTE *)&m_pReceive->Data;
    dwCode = MAKEWORD(m_pReceive->afn, m_pReceive->CMD);
    
    switch (dwCode)
    {
		
    	case RESET:
    		//填充系统复位函数调用
    		SendACK();
    		thSleep(100);
    		sysRestart(COLDRESTART, SYS_RESET_REMOTE, GetThName(m_wTaskID)); 
    		break;
    	case GET_CLK:
    		SendClock();
    		break;
    	case SET_CLK:
    		SetClock();
    		break;
    		
    	case GET_YC:
    		SendYC();
    		break;
    	case GET_YC1:
    		SendYC1();
    		break;
    	case GET_YC2:
    		SendYC2();
    		break;
    	case GET_DD:
    		SendDD();
    		break;
    	case GET_YX_SINGLE:
    		SendSingleYX();
    		break;
    	case GET_DYX:
    		SenddYX();
    		break;
     	case GET_DSOE:
    		SendDSOE();
    		break;
    	case GET_SOE_SINGLE:
    		SendSingleSOE();
    		break;
    	case GET_COS_SINGLE:
    		SendSingleCOS();
    		break;
   		case GET_accurve:
			SendAC();
			break;
			
    	case GET_HIS_YC:
			SendHisYC();
			break;
    	case GET_HIS_DD:
			SendHisdd();
			break;
    	case GET_YB:
			SendYB();
			break;
    	case GET_YBVal:
			Get_YB();
			break;
    	case GET_RUNINFO:
			Sendruninfo();
			break;
    	case GET_EVENT:
			SendEvent();
			break;
    	case GET_maxvalue:
			Sendmaxvalue();
			break;
   	case GET_ycfloat:
			SendYCfloat();
			break;
   		case GET_dotdef:
			Senddotdef();
			break;
   		case GET_ver:
			read_verinfo();
			break;
   		case GET_YXZ:
			read_youxiaozhi();
			break;
  		case GET_EQPNAME:
			read_EQPname();
			break;
   		case GET_addr:
			read_addr();
 			break;
   		case GET_COMM:
			read_comm();
			break;
			
		case GET_CHG_YC:
#ifdef MAINT_YCCHAGE_SEND
			SendChageYC();
			break;
#else
			SendNACK(1);
			break;
#endif
    	case SET_YK:
    		m_wYKCode = SET_YK;
    		SetYKCommand(MI_YKSELECT);
    		break;
    	case DO_YK:
    		m_wYKCode = DO_YK;
    		SetYKCommand(MI_YKOPRATE);
    		break;
    	case UNDO_YK:
    		m_wYKCode = UNDO_YK;
    		SetYKCommand(MI_YKCANCEL);
    		break;		
       	case SET_BH:
     		set_baohudingzhi();
   			break;
   		case GET_BH:
     		Get_baohudingzhi();
   			break;
		
    	case SET_IP:
     		set_ip();
   			break;
    	case SET_GPRSADDR:
     		set_gprsip();
   			break;
    	case SET_HZGZ:
     		set_HZGZ();
   			break;
		
    	case SET_com:
     		//set_ip();
   			break;
    	case CLR_records:
			clear_records();
   			break;
    	case GPRS_POWER:
     		gprs_power();
   			break;
		
    	case RESET_PROTECT:
        	reset_protect();
		break;

		case YT_SET:
			DoYTSet();
			break;
		case SET_DCHH:
			dchh();
			break;
    		
    	case READ_FILE:
    		SendFile();
    		break;
    	case WRITE_FILE:
    		ReceiveFile();
    		break;
    	case READ_DIR:
    		SendDir();
    		break;
    	case RGZS:
    		rgzs();
    		break;
    	case SHOWBUFATA:
    		showdatabuf();
    		break;

    	case BUILD_DIR:
    		BuildDir();
    		break;
    	case RENAME_FILE:
    		RenameFile();
    		break;
    	case GET_FILE_ATTR:
    		SendFileStatus();
    		break;
    	case SET_remotequit:
    		para = MAINT_ID;
			commCtrl(m_wCommID, CCC_TID_CTRL, (BYTE*)&para);
    		break;
		case DEL_FILE:
			DeleteFile();
			break;
		case DEL_DIR:
			DeleteDir();
			break;				
#ifndef YC_GAIN_NEW
		case YC_PARASET:
			int flag;
			
#ifdef INCLUDE_YC			
			if(m_pReceive->Data[0] == 2)
                flag=gainzero();
			else if(m_pReceive->Data[0] == 1)
				flag=gainset(1);
			else if(m_pReceive->Data[0] == 0)
				flag=gainset(0);
			else if(m_pReceive->Data[0] == 3)
				flag=gainset(3);
			else if(m_pReceive->Data[0] == 4)
				flag=gainset(4);
			else
		   		flag = OK;
#else

            flag = ERROR;
#endif
		 	if(flag==OK)
				SendACK();
			else
				SendNACK(1);
			break;
#else
        case YC_PARAVALSET:
			Maint_BM(dwCode,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3);
			write_ycgainval();
			break;
		case YC_PARAVALGET:
			read_ycgainval();
			break;
		case YC_PARASET:
			Maint_BM(dwCode,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3);
			write_ycgain();
			break;
		case YC_PARAGET:
			read_ycgain();
			break;
		case YC_PARAZERO:
		{
			 Maint_BM(dwCode,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3);
			 if(gainzero() == OK)
				SendACK();
			else
				SendNACK(1);
			break;
		}
#endif
		case FORMAT_DISK:
			ProcFormat();
			break;
		case GET_SELFRUNPARA:
			SendSelfRunPara();
			break;
		case SET_SELFRUNPARA:
			WriteSelfRunPara();
			break;
		case GET_ICMD:
			SendICmd();
			break;
		case GET_SoeCosDotDef:
			SendSoeCosdotdef();
			break;
		case Set_GainCommand:
			setGainCommand();
			break;
		case SetDevIDCmd:
			setDevID();
			break;
		case SET_DEV_TYPE:
			setDevType();
			break;
		case SET_DEV_MFR:
			setDevMFR();
			break;
		case SET_DEV_MFR_UTF8:
			setDevMFR_Utf8();
			break;
		case SET_DEV_SV:
			setDevSV();
			break;
		case Get_JIAMI:
			SendJiami();
			break;
		case GET_Lubo:
			Do_Lubo();
			break;
		case GET_PB:
			Get_Pb();
			break;
		case SET_PB:
			Set_Pb();
			break;
		case GET_ADType:
			GetADType();
		 	break;
		case SET_ADType:
			SetADType();
		 	break;
		case SET_IP_BLT:
     		set_ip_blt();
   		 	break;
		case SET_ESN:
		 	Set_Esn();
		 	break;
		case GET_ESN:
		 	Get_Esn();
		 	break;
		case GET_LTE:
		 	Get_LTE();
		 	break;
#ifdef INCLUDE_PR_DIFF
		 case SET_DIFF:
     		set_diffset();
   		 break;
		 case GET_DIFF:
     		get_diffset();
			break;
		case GET_DIFFCLK:
    		SendDiffClock();
    		break;
		case SET_DIFFPara_Init:
    		SendDiffParaInit();
    	break;
#endif
    	default:
    		break;
    	
    }//end of switch
    return TRUE;
}


/***************************************************************
	Function：SearchOneFrame
		搜索一个完整的规约报文
	参数：Buf, Len
		Buf 接收缓冲区头指针
		Len 接收缓冲区内有效数据的长度
	返回：DWORD数据，其中
		高字：处理结果
			#define FRAME_OK       0x00010000      //检测到一个完整的帧
			#define FRAME_ERR      0x00020000      //检测到一个校验错误的帧
			#define FRAME_LESS     0x00030000      //检测到一个不完整的帧（还未收齐）
		低字：已经处理的数据长度
***************************************************************/
DWORD CMaint::SearchOneFrame(BYTE *Buf, WORD Len)
{
	WORD NeedCrcLen;
 	WORD FrameLen;
	DWORD Rc;
    WORD CrcRemote,CrcLocal;
    WORD CrcOffset;

//	m_dwGBAddressFlag=1;

    if (Len<9)
    	return FRAME_LESS;
	
	Rc=SearchHead(Buf,Len,0,0x68,5,0x68);
	if(Rc == Len -5) //没查到报文头
		return FRAME_ERR|(Rc+5);
		
	FrameLen = MAKEWORD(Buf[Rc+1],Buf[Rc+2]);
	if((FrameLen > MAX_FRAME_SIZE) || (FrameLen != MAKEWORD(Buf[Rc+1],Buf[Rc+2])))
	{
		return FRAME_ERR|(Rc+5);
	}
		
    if((MAKEWORD(Buf[Rc+1],Buf[Rc+2])+Rc+9>Len))
       return FRAME_LESS;
    m_pReceive=(VFrame *)(Buf+Rc);
	Buf+=Rc;
	m_RecBuf.wReadPtr+=(WORD)Rc;
   	FrameLen=MAKEWORD( m_pReceive->Len1Low, m_pReceive->Len1High);
    
  //  if ((m_pReceive->CodeHi<0x80)||(m_pReceive->CodeHi>0x8f)||(m_pReceive->CodeLo>0x20))
   // 		return FRAME_ERR|1;
    if(m_pReceive->CMD&0x80)
		return FRAME_ERR|Len;
    if(FrameLen>MAX_FRAME_SIZE)
		return FRAME_ERR|1;
    
    if(FrameLen+9>Len)
		return FRAME_LESS;
    
    NeedCrcLen = FrameLen;
    CrcOffset  = FrameLen+6;
    
    CrcRemote = MAKEWORD( Buf[CrcOffset], Buf[CrcOffset+1] );//接收到的ＣＲＣ码
    CrcLocal  = CheckSum(Buf+6, NeedCrcLen);
    if(CrcLocal!=CrcRemote)
		return  FRAME_ERR|1;
	if(Buf[6]!=0)
	{
		if(Buf[6]!=(g_Sys.AddrInfo.wAddr&0xff))
			return  FRAME_ERR|FrameLen+9;	
	}
    return FRAME_OK|FrameLen+9; //返回本帧总长度
}

void CMaint::DoTimeOut(void)
{
	CPSecondary::DoTimeOut();

	if(m_sendcount++>10)
	{
		m_sendcount=0;
		m_SendBuf.wWritePtr=0;
#ifdef INCLUDE_TEST
		char tt[]="test comm1   1234567890";
		m_SendBuf.wReadPtr=0;
	
		memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,tt,strlen(tt));
		m_SendBuf.wWritePtr+=strlen(tt);
		if(m_wCommID==COMM_START_NO+1)
			WriteToComm(m_pReceive->Address);
		
#endif
	}
	//SendACK();
}

/***************************************************************
	Function：Sendyb
		发送压板状态
	参数：无
		
	返回：无
***************************************************************/
void CMaint::SendYB(void)            	
{
	WORD Shift;
	WORD YCNum,YCNumOffSet,RecYCNum;
	DWORD EqpName;
//	WORD EqpID;
	BYTE ErrorCode,*p;
//	WORD wYCValue;
	//BYTE tmp[100];
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    YCNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数

    if (YCNum > ((MAX_FRAME_SIZE-50)/2))
    {
		YCNum = (MAX_FRAME_SIZE-50)/2;
	}
    SendFrameHead(GET_YB);
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    //发送遥测的原始值
    YCNumOffSet = m_SendBuf.wWritePtr; //记录AI个数存放位置
    m_SendBuf.wWritePtr += 2;
    RecYCNum = ykGetYb(Shift,YCNum,(BYTE *)m_dwPubBuf);
    
    if (RecYCNum == 0)
    {
    	ErrorCode = 2;//have no yc or yc num = 0
    	SendNACK(ErrorCode);
    	return;	
    }

    p = (BYTE *)m_dwPubBuf;
    for (int i = 0; i < RecYCNum; i++)
    {
    	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = *p;
	p++;
    }
    
	SendWordLH(YCNumOffSet, RecYCNum);
	
	SendFrameTail();

    return;
}

 /***************************************************************
	Function：reset protect
		信号复归
	参数：无
		
	返回：无
***************************************************************/
void CMaint::reset_protect(void)            	
{
	DWORD EqpName;
//	WORD EqpID;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;

    SendFrameHead(RESET_PROTECT);
    SendDWordLH(EqpName);
	SendFrameTail();
#ifdef INCLUDE_PR
	ResetProtect(0);
#endif
}
 /***************************************************************
	Function：reset protect
		信号复归
	参数：无
		
	返回：无
***************************************************************/
void CMaint::dchh(void)            	
{
	DWORD EqpName;
//	WORD EqpID;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	if(m_pFrameData[4]==1)
		cellOutput(1);
	if(m_pFrameData[4]==2)
		cellOutput(0);
    SendFrameHead(SET_DCHH);
    SendDWordLH(EqpName);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =m_pFrameData[4];
	SendFrameTail();
    return;
}
 void CMaint::gprs_power(void)            	
{
#ifdef INCLUDE_GPRS
	DWORD EqpName;
//	WORD EqpID;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;
	if(m_pFrameData[4]==1)
		//BSP_GR47_PWR_ON;
	if(m_pFrameData[4]==2)
		//BSP_GR47_PWR_OFF;
    SendFrameHead(GPRS_POWER);
    SendDWordLH(EqpName);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =m_pFrameData[4];
	SendFrameTail();
    return;
#endif	
}
 /***************************************************************
	Function：set YB
		设置压板
	参数：无
		
	返回：无
***************************************************************/
void CMaint::set_baohudingzhi(void)            	
{
#if 0
	DWORD EqpName;
	WORD EqpID;
	WORD Shift;
	BYTE line;
	BYTE *pdata=m_pFrameData+10;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
  	line = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
  	Shift = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取偏移量
	if (GetEqpID(EqpName, &EqpID))
	{
		SendNACK(2);
		return;
	}
	if(WriteSet(line,pdata) == OK)
		WriteDoEvent(NULL, 0, "维护软件修改保护定值!");
	else
		WriteDoEvent(NULL, 0, "维护软件修改保护定值失败!");
	SendFrameHead(SET_BH);
	SendDWordLH(EqpName);
 	SendWordLH(line);
	SendWordLH(Shift);
	SendWordLH(prosetnum);
#ifdef INCLUDE_PR
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pdata,prosetnum*sizeof(TSETVAL)+sizeof(TTABLETYPE));
		m_SendBuf.wWritePtr+=prosetnum*sizeof(TSETVAL)+sizeof(TTABLETYPE);
#endif
	SendFrameTail();
		return;
#else
	WORD rcvlen = 0;
	
	if(Maint_BM_RCV(SET_BH,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3,
	(BYTE*)m_dwPubBuf, &rcvlen) == ERROR)
	{
		SendNACK(1);
		return;
	}
	
	SendFrameHead(SET_BH);
	memcpy(m_SendBuf.pBuf + m_SendBuf.wWritePtr, m_dwPubBuf ,rcvlen);
	m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + rcvlen;
	SendFrameTail();
#endif
}
  /***************************************************************
	Function：set YB
		设置压板
	参数：无
		
	返回：无
***************************************************************/
void CMaint::Get_baohudingzhi(void)            	
{
#if 0
DWORD EqpName;
	WORD EqpID;
	WORD Shift;
	BYTE line;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
  	line = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
  	Shift = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取偏移量
  if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(2);
    	return;
    }
#ifdef INCLUDE_PR
	prosetnum = ReadSetNum(line);   //cjl 未做
#endif
    SendFrameHead(GET_BH);
    SendDWordLH(EqpName);
 	SendWordLH(line);
	SendWordLH(Shift);
	SendWordLH(prosetnum);
#ifdef INCLUDE_PR
 	ReadSet(line,m_SendBuf.pBuf+m_SendBuf.wWritePtr);
			m_SendBuf.wWritePtr+=prosetnum*sizeof(TSETVAL)+sizeof(TTABLETYPE);
#endif
		SendFrameTail();
#else
	WORD rcvlen = 0;
	
	if(Maint_BM_RCV(GET_BH,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3,
	(BYTE*)m_dwPubBuf, &rcvlen) == ERROR)
	{
		SendNACK(1);
		return;
	}
	
	SendFrameHead(GET_BH);
	memcpy(m_SendBuf.pBuf + m_SendBuf.wWritePtr, m_dwPubBuf ,rcvlen);
	m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + rcvlen;
	SendFrameTail();
#endif
}
 
/***************************************************************
	Function：set YB
		设置压板
	参数：无
		
	返回：无
***************************************************************/
void CMaint::Get_YB(void)            	
{

	DWORD EqpName;
	WORD EqpID;
	WORD Shift;
	WORD num;
	BYTE line=0;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
  	Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
  	num = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取偏移量
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(2);
    	return;
    }
	ykSetSYb(Shift,m_pFrameData[8]);

    SendFrameHead(GET_YBVal);
    SendDWordLH(EqpName);
	SendWordLH(line);
	SendWordLH(Shift);
	SendWordLH(num);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =m_pFrameData[8];

	SendFrameTail();
    return;
}

 /***************************************************************
	Function：set ip
		设置ip
	参数：无
		
	返兀何?
***************************************************************/
void CMaint::clear_records(void)            	
{
	DWORD EqpName;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    switch(m_pFrameData[4])
     {
     	case 1:
	         SysEventReset();
	    break;
		case 4:
			 SysSoeReset();
			break;
     		
   		}

    SendFrameHead(CLR_records);
    SendDWordLH(EqpName);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] =m_pFrameData[4];

	SendFrameTail();
    return;
}
/***************************************************************
	Function：set ip
		设置ip
	参数：无
		
	返回：无
***************************************************************/
void CMaint::set_ip(void)            	
{
	WORD off=4;
	WORD addr;
	char ip[2][20];
	addr=WORD(m_pFrameData[off+1]<<8)|m_pFrameData[off];
	off+=2;
 	sprintf(ip[0],"%d.%d.%d.%d",m_pFrameData[off],m_pFrameData[off+1],m_pFrameData[off+2],m_pFrameData[off+3]);
	off+=4;

	sprintf(ip[1],"%d.%d.%d.%d",m_pFrameData[off],m_pFrameData[off+1],m_pFrameData[off+2],m_pFrameData[off+3]);
	SetSysAddr(&addr,ip[0],ip[1],NULL,NULL);

    SendFrameHead(SET_IP);
  	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_pFrameData,16);
	m_SendBuf.wWritePtr+=16;
	SendFrameTail();
    return;
}
/***************************************************************
	Function：set ip
		蓝牙设置ip1
	参数：无
		
	返回：无
***************************************************************/
void CMaint::set_ip_blt(void)            	
{
	WORD off=4;
	char ip[2][20];
	BOOL setok = TRUE;

	if(m_pFrameData[off] == 0)
	{//IP有误,忽略
		off+=4;
		if(m_pFrameData[off] == 0)
		{
			setok = FALSE;
		}
		else
		{
			sprintf(ip[1],"%d.%d.%d.%d",m_pFrameData[off],m_pFrameData[off+1],m_pFrameData[off+2],m_pFrameData[off+3]);
			if(SetSysAddr(NULL,NULL,ip[1],NULL,NULL)<0)
			{
				setok = FALSE;
			}
		}
	}
	else
	{
		sprintf(ip[0],"%d.%d.%d.%d",m_pFrameData[off],m_pFrameData[off+1],m_pFrameData[off+2],m_pFrameData[off+3]);
		off+=4;
		if(m_pFrameData[off] == 0)
		{//IP2有误
			if(SetSysAddr(NULL,ip[0],NULL,NULL,NULL)<0)
			{
				setok = FALSE;
			}
		}
		else
		{
			sprintf(ip[1],"%d.%d.%d.%d",m_pFrameData[off],m_pFrameData[off+1],m_pFrameData[off+2],m_pFrameData[off+3]);
			if(SetSysAddr(NULL,ip[0],ip[1],NULL,NULL)<0)
			{
				setok = FALSE;
			}
		}
	}
	
	
    if (setok)
	{
		SendACK();	
	}
	else
	{
		SendNACK(1);
	}
	return;
}

void CMaint::set_gprsip(void)            	
{
#ifdef INCLUDE_GPRS
/*
	WORD off=4;
			VLanIP host;

 		sprintf(host.ip,"%d.%d.%d.%d",m_pFrameData[off],m_pFrameData[off+1],m_pFrameData[off+2],m_pFrameData[off+3]);
	off+=4;
		host.port=MAKEWORD(m_pFrameData[off], m_pFrameData[off+1]);
	off+=2;
	gprsHost_Set(0,&host,0,0,0);
 		sprintf(host.ip,"%d.%d.%d.%d",m_pFrameData[off],m_pFrameData[off+1],m_pFrameData[off+2],m_pFrameData[off+3]);
	off+=4;
		host.port=MAKEWORD(m_pFrameData[off], m_pFrameData[off+1]);
	off+=2;
	gprsHost_Set(1,&host,(char*)m_pFrameData+off,(char*)m_pFrameData+off+32,(char*)m_pFrameData+off+64);

    SendFrameHead(SET_GPRSADDR);
   memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_pFrameData,112);
	m_SendBuf.wWritePtr+=112;
	SendFrameTail();
    return;
	*/
#endif	
}
void CMaint::set_HZGZ(void)            	
{
#ifdef INCLUDE_YC
	WORD off=4;

	SendFrameHead(SET_HZGZ);
   	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=m_pFrameData[off];
	SendFrameTail();
#elif defined(INCLUDE_EXT_YC)    
    SendFrameHead(SET_HZGZ);    
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=1;	
	SendFrameTail();
#else
    SendNACK(1);
#endif
}


 /***************************************************************
	Function：rgzs
		人工置数
	参数：无
		
	返回：无
***************************************************************/
void CMaint::rgzs(void)            	
{
	WORD Shift;
	WORD YCNum;
//	WORD YCNumOffSet;
	DWORD EqpName;
	WORD EqpID;
	BYTE ErrorCode=1;
//	WORD RecYCNum;
//	WORD *pwYC = NULL;
//	WORD wYCValue;
    float dd = 0;
	struct VDBYX buf;
	struct VDBYCF_L buf1;
	struct VDBDDF buf2;
	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    YCNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }
	switch(Shift)
	{
		case 0x101:
			buf.wNo=YCNum;
			buf.byValue=0;
			LockSYX(EqpID,1,&buf);
			break;
		case 0x102:
		case 0x105:			
		case 0x106:
			buf.wNo=YCNum;
			buf.byValue=m_pFrameData[8];
			LockSYX(EqpID,1,&buf);
			break;
		case 0x103:
			buf.wNo=YCNum;
			buf.byValue=0;
			UnLockSYX(EqpID,1,&buf);
			break;
		case 0x104:
			buf.wNo=YCNum;
			buf.byValue=m_pFrameData[8];
			UnLockSYX(EqpID,1,&buf);
				break;
		case 0x201:
			buf1.wNo=YCNum;
			buf1.byFlag=0;
			LockYC(EqpID,1,&buf1);
			break;
		case 0x202:
			buf1.wNo=YCNum;
			buf1.lValue=MAKEDWORD(MAKEWORD(m_pFrameData[8], m_pFrameData[9]),MAKEWORD(m_pFrameData[10], m_pFrameData[11]));
			buf1.byFlag=1;
			if(m_pFrameData[12])
			buf1.byFlag|=0x10;
			LockYC(EqpID,1,&buf1);
			break;
		case 0x203:
			buf1.wNo=YCNum;
			buf1.lValue=MAKEDWORD(MAKEWORD(m_pFrameData[8], m_pFrameData[9]),MAKEWORD(m_pFrameData[10], m_pFrameData[11]));
			UnLockYC(EqpID,1,&buf1);
			break;
		case 0x204:
			buf1.wNo=YCNum;
			buf1.lValue=MAKEDWORD(MAKEWORD(m_pFrameData[8], m_pFrameData[9]),MAKEWORD(m_pFrameData[10], m_pFrameData[11]));
			UnLockYC(EqpID,1,&buf1);
			break;
		case 0x302: //人工置数
			buf2.wNo = YCNum;
			//buf2.lValue=MAKEDWORD(MAKEWORD(m_pFrameData[8], m_pFrameData[9]),MAKEWORD(m_pFrameData[10], m_pFrameData[11]));
			dd = MAKEDWORD(MAKEWORD(m_pFrameData[8], m_pFrameData[9]),MAKEWORD(m_pFrameData[10], m_pFrameData[11]));
			dd = dd/100;
			memcpy((void*)&buf2.lValue,(void*)&dd,4);
			LockDD(EqpID,1,&buf2);
			break;
		case 0x304: //解除人工置数
			buf2.wNo = YCNum;
			//buf2.lValue=MAKEDWORD(MAKEWORD(m_pFrameData[8], m_pFrameData[9]),MAKEWORD(m_pFrameData[10], m_pFrameData[11]));
			dd = MAKEDWORD(MAKEWORD(m_pFrameData[8], m_pFrameData[9]),MAKEWORD(m_pFrameData[10], m_pFrameData[11]));
			dd = dd/100;
			memcpy((void*)&buf2.lValue,(void*)&dd,4);
			UnLockDD(EqpID,1,&buf2);
			break;
	}
    SendFrameHead(RGZS);
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,m_pFrameData+6,6);
	m_SendBuf.wWritePtr+=6;
  
	SendFrameTail();
    return;
}

/***************************************************************
	Function：showdatabuf
		浏览缓冲区数据
	参数：无
		
	返回：无
***************************************************************/
void CMaint::showdatabuf(void)            	
{
	WORD Shift;
	DWORD EqpName;
	WORD EqpID;
	BYTE ErrorCode=1;
	WORD YCNum;
	WORD YCNumOffSet;
	BYTE cmdflag;
	 
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    //YCNum = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数
    cmdflag=m_pFrameData[6];
    if (GetEqpID(EqpName, &EqpID))
    {
    	SendNACK(ErrorCode);
    	return;
    }
	switch(cmdflag)
	{
		case 0x1:
			commMShowReq(Shift);
			break;
		case 0x2:
			commMShowCancle();
			break;
		default:
			break;
	}
    SendFrameHead(SHOWBUFATA);
    SendDWordLH(EqpName);
    SendWordLH(Shift);
    YCNumOffSet = m_SendBuf.wWritePtr; //记录AI个数存放位置
    m_SendBuf.wWritePtr += 2;
	YCNum=(WORD)commBufQuery(Shift,500,(char*)m_SendBuf.pBuf+m_SendBuf.wWritePtr);
	m_SendBuf.wWritePtr+=YCNum;
    SendWordLH(YCNumOffSet, YCNum);
	SendFrameTail();
    return;
}
WORD CMaint::read_ycdef(WORD eqpID,WORD off,WORD num)
{
	struct VTrueEqp *pTEqp;
	struct VTrueYCCfg *pTYCCfg ;
	struct VVirtualEqp *pVEqp;
	struct VVirtualYC  *pVYC;
		
	WORD i = 0;
	WORD len;

	pTEqp = g_Sys.Eqp.pInfo[eqpID].pTEqpInfo;
	pVEqp = g_Sys.Eqp.pInfo[eqpID].pVEqpInfo;
	if(pTEqp)
	{
		pTYCCfg=pTEqp->pYCCfg;
		if(off>pTEqp->Cfg.wYCNum)
			num=0;
		else if(off+num>pTEqp->Cfg.wYCNum)
			num=pTEqp->Cfg.wYCNum-off;
		for(i=off;i<off+num;i++)
		{
			len=(WORD)strlen(pTYCCfg[i].sNameTab);
			if(len >= 15)
				len = 14;
			memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pTYCCfg[i].sNameTab,(DWORD)len);
			m_SendBuf.wWritePtr+=len;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
		}
	}
	else if(pVEqp)
	{
		pVYC = pVEqp->pYC;
		if(off>pVEqp->Cfg.wYCNum)
			num=0;
		else if(off+num>pVEqp->Cfg.wYCNum)
			num=pVEqp->Cfg.wYCNum-off;
		for(i=off;i<off+num;i++)
		{
			len=(WORD)strlen(pVYC[i].sNameTab);
			if(len >= 15)
				len = 14;
			memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pVYC[i].sNameTab,(DWORD)len);
			m_SendBuf.wWritePtr+=len;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
		}
	}
	return i-off;
}
WORD CMaint::read_yxdef(WORD eqpID,WORD off,WORD num)
{
	struct VTrueEqp *pTEqp;
    struct VTrueYXCfg *pTYXCfg;
    struct VVirtualEqp *pVEqp;
    struct VVirtualYX  *pVSYX;
	WORD i = 0;
	WORD len;
	WORD max;

	pTEqp = g_Sys.Eqp.pInfo[eqpID].pTEqpInfo;
	pVEqp = g_Sys.Eqp.pInfo[eqpID].pVEqpInfo;

    if(pTEqp)
    {
        pTYXCfg=pTEqp->pSYXCfg;
        max = pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;
        for(i=off;i<off+num;i++)
        {
			len=(WORD)strlen(pTYXCfg[i].sNameTab);
			if(len >= 15)
				len = 14;
			memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pTYXCfg[i].sNameTab,(DWORD)len);
			m_SendBuf.wWritePtr+=len;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
        }
    }
	else if(pVEqp)
    {
        pVSYX=pVEqp->pSYX;
        max = pVEqp->Cfg.wSYXNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;
        for(i=off;i<off+num;i++)
        {	
            len=(WORD)strlen(pVSYX[i].sNameTab);
            if(len >= 15)
				len = 14;
            memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pVSYX[i].sNameTab,(DWORD)len);
            m_SendBuf.wWritePtr+=len;
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
        }
    }
	return i-off;
}
WORD CMaint::read_dyxdef(WORD eqpID,WORD off,WORD num)
{
    struct VTrueEqp *pTEqp;
    struct VTrueYXCfg *pTYXCfg;
    struct VVirtualEqp *pVEqp;
    struct VVirtualYX  *pVDYX;
		
	WORD i = 0;
	WORD len;
	WORD max;
	
	pTEqp = g_Sys.Eqp.pInfo[eqpID].pTEqpInfo;
    pVEqp = g_Sys.Eqp.pInfo[eqpID].pVEqpInfo;

    if(pTEqp)
    {
        pTYXCfg=pTEqp->pDYXCfg;
        max = pTEqp->Cfg.wDYXNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;
        for(i=off;i<off+num;i++)
        {
            len=(WORD)strlen(pTYXCfg[i].sNameTab);
            if(len >= 15)
				len = 14;
            memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pTYXCfg[i].sNameTab,(DWORD)len);
            m_SendBuf.wWritePtr+=len;
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
        }
    }
	else if(pVEqp)
    {
        pVDYX=pVEqp->pDYX;
        max = pVEqp->Cfg.wDYXNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;
        for(i=off;i<off+num;i++)
        {
            len=(WORD)strlen(pVDYX[i].sNameTab);
            if(len >= 15)
				len = 14;
            memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pVDYX[i].sNameTab,(DWORD)len);
            m_SendBuf.wWritePtr+=len;
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
        }
    }
	return i-off;
}
WORD  CMaint::read_yxSOEdef(WORD eqpID,WORD off,WORD num)
{
	struct VTrueEqp *pTEqp;
    struct VTrueYXCfg *pTYXCfg;
    struct VVirtualEqp *pVEqp;
    struct VVirtualYX  *pVSYX;
	WORD i = 0;
	WORD len;
	WORD max;

	pTEqp = g_Sys.Eqp.pInfo[eqpID].pTEqpInfo;
	pVEqp = g_Sys.Eqp.pInfo[eqpID].pVEqpInfo;

    if(pTEqp)
    {
        pTYXCfg=pTEqp->pSYXCfg;
        max = pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;
        for(i=off;i<off+num;i++)
        {
            if(pTYXCfg[i].wSendNo != 0xFFFF)
	    	{
	    		memcpy((void *)((char*)m_SendBuf.pBuf+m_SendBuf.wWritePtr),(char*)&pTYXCfg[i].wSendNo,sizeof(WORD));
				m_SendBuf.wWritePtr+= sizeof(WORD);
				len=(WORD)strlen(pTYXCfg[i].sNameTab);
				if(len >= 15)
					len = 14;
				memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pTYXCfg[i].sNameTab,(DWORD)len);
				m_SendBuf.wWritePtr+=len;
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
		    }
        }
    }
	else if(pVEqp)
    {
        pVSYX=pVEqp->pSYX;
        max = pVEqp->Cfg.wSYXNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;
        for(i=off;i<off+num;i++)
        {
            memcpy((void *)((char*)m_SendBuf.pBuf+m_SendBuf.wWritePtr),(char*)&pVSYX[i].wSendNo,sizeof(WORD));
            m_SendBuf.wWritePtr+=sizeof(WORD);
			
            len=(WORD)strlen(pVSYX[i].sNameTab);
            if(len >= 15)
				len = 14;
            memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pVSYX[i].sNameTab,(DWORD)len);
            m_SendBuf.wWritePtr+=len;
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
        }
    }
	return i-off;
}
WORD  CMaint::read_dyxSOEdef(WORD eqpID,WORD off,WORD num)
{
	struct VTrueEqp *pTEqp;
    struct VTrueYXCfg *pTYXCfg;
    struct VVirtualEqp *pVEqp;
    struct VVirtualYX  *pVDYX;
		
	WORD i = 0;
	WORD len;
	WORD max;
	
	pTEqp = g_Sys.Eqp.pInfo[eqpID].pTEqpInfo;
    pVEqp = g_Sys.Eqp.pInfo[eqpID].pVEqpInfo;

    if(pTEqp)
    {
        pTYXCfg=pTEqp->pDYXCfg;
        max = pTEqp->Cfg.wDYXNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;
        for(i=off;i<off+num;i++)
        {
           if(pTYXCfg[i].wSendNo != 0xFFFF)
           {
           	   	memcpy((void *)((char*)m_SendBuf.pBuf+m_SendBuf.wWritePtr),(char*)&pTYXCfg[i].wSendNo,sizeof(WORD));
	           	m_SendBuf.wWritePtr+=sizeof(WORD);
           		
	           	len=(WORD)strlen(pTYXCfg[i].sNameTab);
	       		if(len >= 15)
					len = 14;
                memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pTYXCfg[i].sNameTab,(DWORD)len);
                m_SendBuf.wWritePtr+=len;
                m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
           }
        }
    }
	else if(pVEqp)
    {
        pVDYX=pVEqp->pDYX;
        max = pVEqp->Cfg.wDYXNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;
        for(i=off;i<off+num;i++)
        {
            memcpy((void *)((char*)m_SendBuf.pBuf+m_SendBuf.wWritePtr),(char*)&pVDYX[i].wSendNo,sizeof(WORD));
	    	m_SendBuf.wWritePtr+= sizeof(WORD);

            len=(WORD)strlen(pVDYX[i].sNameTab);
            if(len >= 15)
				len = 14;
            memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pVDYX[i].sNameTab,(DWORD)len);
            m_SendBuf.wWritePtr+=len;
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
        }
    }
	return i-off;
}
WORD CMaint::read_ykdef(WORD eqpID,WORD off,WORD num)
{
	struct VTrueEqp *pTEqp;
    struct VTrueCtrlCfg *pTYKCfg;
    struct VVirtualEqp *pVEqp;
    struct VVirtualCtrl  *pVYK;
    WORD i = 0;
    WORD len;
    WORD max;
	
	pTEqp = g_Sys.Eqp.pInfo[eqpID].pTEqpInfo;
    pVEqp = g_Sys.Eqp.pInfo[eqpID].pVEqpInfo;

    if(pTEqp)
    {
        pTYKCfg = pTEqp->pYKCfg;
        max=pTEqp->Cfg.wYKNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;

        for(i=off;i<off+num;i++)
        {
            len=(WORD)strlen(pTYKCfg[i].sNameTab);
            if(len >= 15)
				len = 14;
            memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pTYKCfg[i].sNameTab,(DWORD)len);
            m_SendBuf.wWritePtr+=len;
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
        }
    }
	else if(pVEqp)
    {
        pVYK = pVEqp->pYK;
        max=pVEqp->Cfg.wYKNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;

        for(i=off;i<off+num;i++)
        {
            len=(WORD)strlen(pVYK[i].sNameTab);
            if(len >= 15)
				len = 14;
            memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pVYK[i].sNameTab,(DWORD)len);
            m_SendBuf.wWritePtr+=len;
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
        }
    }
	return i-off;
}
WORD CMaint::read_ytdef(WORD eqpID,WORD off,WORD num)
{
	struct VTrueEqp *pTEqp = NULL;
    struct VTrueCtrlCfg *pTYTCfg = NULL;
    struct VVirtualEqp *pVEqp = NULL;
    struct VVirtualCtrl  *pVYT = NULL;
    WORD i;
    WORD len;
    WORD max;
	pTEqp = g_Sys.Eqp.pInfo[eqpID].pTEqpInfo;
    pVEqp = g_Sys.Eqp.pInfo[eqpID].pVEqpInfo;

    if(pTEqp)
    {
        pTYTCfg = pTEqp->pYTCfg;
        max=pTEqp->Cfg.wYTNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;

        for(i=off;i<off+num;i++)
        {
            len=strlen(pTYTCfg[i].sNameTab);
            if(len >= 15)
				len = 14;
            memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pTYTCfg[i].sNameTab,len);
            m_SendBuf.wWritePtr+=len;
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
        }
    }
	else if(pVEqp)
    {
        pVYT = pVEqp->pYT;
        max=pVEqp->Cfg.wYTNum;
        if(off>max)
            num=0;
        else if(off+num>max)
            num=max-off;

        for(i=off;i<off+num;i++)
        {
            len=strlen(pVYT[i].sNameTab);
            if(len >= 15)
				len = 14;
            memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pVYT[i].sNameTab,len);
            m_SendBuf.wWritePtr+=len;
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
        }
    }
	return i-off;
}
WORD CMaint::read_dddef(WORD eqpID,WORD off,WORD num)
{
	struct VTrueDDCfg *pddCfg;
	struct VVirtualEqp *pVEqp;
	struct VTrueEqp *pTEqp;
	struct VVirtualDD *pVDD;
	
	WORD i;
	WORD len;
	WORD max;

	pTEqp = g_Sys.Eqp.pInfo[eqpID].pTEqpInfo;
	pVEqp = g_Sys.Eqp.pInfo[eqpID].pVEqpInfo;

	if(pTEqp)
	{
		pddCfg=pTEqp->pDDCfg;
		max =pTEqp->Cfg.wDDNum;
		if(off>max)
			num=0;
		else if(off+num>max)
			num=max-off;
		for(i=off;i<off+num;i++)
		{
			len=(WORD)strlen(pddCfg[i].sNameTab);
			if(len >= 15)
				len = 14;
			memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pddCfg[i].sNameTab,(DWORD)len);
			m_SendBuf.wWritePtr+=len;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
		}
	}
	else if(pVEqp)
	{
		pVDD = pVEqp->pDD;
		if(off >  pVEqp->Cfg.wDDNum)
			num = 0;
		else if(off + num > pVEqp->Cfg.wDDNum)
			num = pVEqp->Cfg.wDDNum - off;
		for(i = off; i < off + num ;i++)
		{
			len = (WORD)strlen(pVDD[i].sNameTab);
			if(len >= 15)
				len = 14;
			memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pVDD[i].sNameTab,(DWORD)len);
			m_SendBuf.wWritePtr+=len;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
		}		
	}
	return i - off;
}

WORD CMaint::read_dzdef(WORD off,WORD num)
{
#if 0
	int ttlen;
	VFileHead* phead;
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
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=g_Sys.MyCfg.wAllIoFDNum;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=g_Sys.MyCfg.wAllIoFDNum>>8;
	}
	else
	{
		//fname=getfdcfg(off>>8);
		ReadSetName((off>>8+1),fdname);
		off&=0xff;
		if (ReadParaFile((char*)fdname,prTableBuf,MAX_TABLE_FILE_SIZE)==ERROR) return false;
		BYTE* paddr=(BYTE*)(prTableBuf+sizeof(VFileHead)+sizeof(TTABLETYPE));
		mm_ybnum=*paddr;
		paddr++;
		paddr+=mm_ybnum*sizeof(tagTYBTABLE);
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
		prosetnum=*paddr;
		phead=(VFileHead*)prTableBuf;
		ttlen=phead->nLength;
		ttloff=off*num;
	
		if(ttloff<ttlen)
		{
			if((ttlen-ttloff)<num)
				num=ttlen-ttloff;
			memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,prTableBuf+ttloff,num);
			m_SendBuf.wWritePtr+=num;
		}
		else
			num=0;
	}
	return num;
#else
	return 0;
#endif		
}

WORD CMaint::read_ybdef(WORD off,WORD num)
{
#if 0
		TYBTABLE * pYCCfg=tYbTable;
		//	Vrow *pYCCfg=m_row;

	WORD i;
	WORD len;
	if(off>get_m_tYbTable_len())
		num=0;
	else if(off+num>get_m_tYbTable_len())
		num=get_m_tYbTable_len()-off;
	for(i=off;i<off+num;i++)
		{
			len=strlen(pYCCfg[i].pNames);
			if((len>0)&&(len<15))
				{
			memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pYCCfg[i].pNames,len);
				m_SendBuf.wWritePtr+=len;
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
				}
			else
				break;
			
	}
	return i-off;
#else
	return 0;
#endif	
}
WORD CMaint::read_hxdef(WORD off,WORD num)
{
	WORD i;
	WORD len;
	WORD max=g_Sys.MyCfg.wFDNum;
	if(off>max)
		num=0;
	else if(off+num>max)
		num=max-off;
	for(i=off;i<off+num;i++)
	{
		len=(WORD)strlen(g_Sys.MyCfg.pFd[i].kgname);
		if((len>0)&&(len<15))
		{
			memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,g_Sys.MyCfg.pFd[i].kgname,(DWORD)len);
			m_SendBuf.wWritePtr+=len;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
		}
		else
			break;
			
	}
	return i-off;
}

WORD CMaint::read_selfparadef(WORD off, WORD num)
{

	WORD i;
	WORD len;
	WORD max = SELFPARA_NUM;
	if(off>max)
		num=0;
	else if(off+num>max)
		num=max-off;
	for(i=off;i<off+num;i++)
	{
		len=sizeof(struct VParaDef);
		if(ReadSelfRunParadef(i, m_SendBuf.pBuf+m_SendBuf.wWritePtr) == OK)
			m_SendBuf.wWritePtr+=len;
	}
	return i-off;
}

WORD CMaint::read_runparadef(WORD off, WORD num)
{
	WORD i;
	WORD len;
	WORD max = (RPR_T_YXFD -RPR_V_DY) + 1 + (RPR_V_IYXYB -RPR_V_ULP) + 1;
	if(off>max)
		num=0;
	else if(off+num>max)
		num=max-off;
	for(i=off;i<off+num;i++)
	{
		len=(WORD)sizeof(struct VParaDef);
		if(ReadSelfRunParadef(i + SELFPARA_NUM, m_SendBuf.pBuf+m_SendBuf.wWritePtr) == OK)
			m_SendBuf.wWritePtr+=len;
	}
	return i-off;
}
void CMaint::read_verinfo()
{
		WORD SOENum;
	WORD ReadPtr;
	//WORD BufLen;//环行缓冲池大小
	WORD RecSOENum;
	WORD SOENumOffSet;
	DWORD EqpName;
	//VDBSOE *pRecSOE = NULL;
	//char tt[100];
	//struct VSysEventInfo m_Eventinfo[3];

    
    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    SOENum = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
    ReadPtr = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);
	
    
    SendFrameHead(GET_ver);
	SendDWordLH(EqpName);
    SOENumOffSet = m_SendBuf.wWritePtr; //记录SOE个数存放位置
    m_SendBuf.wWritePtr += 2;
    SendWordLH(ReadPtr);
	if(SOENum>10) SOENum=10;
	RecSOENum=0;
#if (TYPE_OS == OS_VXWORKS)
	WORD len;

	SendWordLH(g_Sys.LdFileNum);
	SendWordLH(g_Sys.LdFileNum);
	for(int i=ReadPtr;(i<SOENum+ReadPtr)&&(i<g_Sys.LdFileNum);i++)
		{
	sprintf((char*)m_dwPubBuf,"模块名称:%s\r\n",g_Sys.LdFileTbl[i].name);
	len=strlen((char*)m_dwPubBuf);
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr ,m_dwPubBuf,len);
	m_SendBuf.wWritePtr+=len;
	sprintf((char*)m_dwPubBuf,"版本号:%s\r\n",g_Sys.LdFileTbl[i].ver);
	len=strlen((char*)m_dwPubBuf);
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr ,m_dwPubBuf,len);
	m_SendBuf.wWritePtr+=len;
	sprintf((char*)m_dwPubBuf,"校验和:%04x\r\n",g_Sys.LdFileTbl[i].crc);
	len=strlen((char*)m_dwPubBuf);
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr ,m_dwPubBuf,len);
	m_SendBuf.wWritePtr+=len;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
	RecSOENum++;
		}
#else
	    SendWordLH(0);
	    SendWordLH(0);
#endif
    SendWordLH(SOENumOffSet, RecSOENum);
	SendFrameTail();
	return;
}
void CMaint::read_youxiaozhi()
{
#ifdef INCLUDE_YC
#if 0
	WORD SOENum;
	WORD ReadPtr;
	WORD len;
//	WORD BufLen;//环行缓冲池大小
	WORD RecSOENum;
	WORD SOENumOffSet;
	DWORD EqpName;
//    VDBSOE *pRecSOE = NULL;
//	char tt[100];
    //	struct VSysEventInfo m_Eventinfo[3];

    VMmiMeaValue mmiVal[10];

	int maxnum = sizeof(mmiVal)/sizeof(mmiVal[0]);

    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    SOENum = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
    ReadPtr = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);
	
    
    SendFrameHead(GET_YXZ);
	SendDWordLH(EqpName);
    SOENumOffSet = m_SendBuf.wWritePtr; //记录SOE个数存放位置
    m_SendBuf.wWritePtr += 2;
    SendWordLH(ReadPtr);
	RecSOENum=0;
    SendWordLH(0);
    SendWordLH(0);
		
	if(SOENum > maxnum)
		SOENum = (WORD)maxnum;
	SOENum=	(WORD)meaRead_Mmi(ReadPtr/8, (ReadPtr%8), SOENum, mmiVal);
	for(int i=0;i<SOENum;i++)
	{
		if(!mmiVal[i].valid) continue;
		
		if ((mmiVal[i].tbl->unit[0]=='A') || (mmiVal[i].tbl->unit[0]=='H')) 
			sprintf((char*)m_dwPubBuf,"%s=%6.3f%s %6.1f°\r\n",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);
		else
			sprintf((char*)m_dwPubBuf,"%s=%6.3f%s %6.1f°\r\n",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);
		
		len=(WORD)strlen((char*)m_dwPubBuf);
		memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr ,m_dwPubBuf,(DWORD)len);
		m_SendBuf.wWritePtr+=len;
		RecSOENum++;
	}
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
	
    SendWordLH(SOENumOffSet, RecSOENum);
	SendFrameTail();
#endif
	WORD rcvlen = 0;
	
	if(Maint_BM_RCV(GET_YXZ,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3,
	(BYTE*)m_dwPubBuf, &rcvlen) == ERROR)
	{
		SendNACK(1);
		return;
	}
	SendFrameHead(GET_YXZ);
	memcpy(m_SendBuf.pBuf + m_SendBuf.wWritePtr, m_dwPubBuf ,rcvlen);
	m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + rcvlen;
	SendFrameTail();
#else
	SendNACK(1);
#endif
}
void CMaint::read_EQPname()
{
	WORD SOENum;
	WORD ReadPtr;
//	WORD len;
//	WORD BufLen;//环行缓冲池大小
	WORD RecSOENum;
	WORD SOENumOffSet;
	DWORD EqpName;
	VEqp *peqp=&g_Sys.Eqp;
	WORD j=0;
//    VDBSOE *pRecSOE = NULL;
//	char tt[100];
    //	struct VSysEventInfo m_Eventinfo[3];

   // 	VMmiMeaValue mmiVal[8];

    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
    SOENum = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);
    ReadPtr = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);
	
    
    SendFrameHead(GET_EQPNAME);
	SendDWordLH(EqpName);
    SOENumOffSet = m_SendBuf.wWritePtr; //记录SOE个数存放位置
    m_SendBuf.wWritePtr += 2;
    SendWordLH(ReadPtr);
	if(SOENum>10) SOENum=10;
	RecSOENum=0;
    SendWordLH(0);
    SendWordLH(0);
	//	g_Sys.Eqp
	//SOENum=	meaRead_Mmi(ReadPtr/8, (ReadPtr%8), SOENum, mmiVal);
	for(int i=0;i<*(peqp->pwNum);i++)
	{
		if(peqp->pInfo[i].pTEqpInfo)
		{
			j++;
			if((j>=ReadPtr)&&(RecSOENum<SOENum))
			{
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(BYTE)peqp->pInfo[i].pTEqpInfo->Cfg.wSourceAddr;
				SendDWordLH(peqp->pInfo[i].dwName);
				RecSOENum++;
			}
		}
	}
	//m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=0;
	
    SendWordLH(SOENumOffSet, RecSOENum);
	SOENumOffSet+=2;
    SendWordLH(SOENumOffSet, ReadPtr);
	SOENumOffSet+=2;
    SendWordLH(SOENumOffSet, ReadPtr+RecSOENum);
	SOENumOffSet+=2;
    SendWordLH(SOENumOffSet, j);
	
	SendFrameTail();
	return;
}

void CMaint::read_addr()
{

	DWORD EqpName;
//  WORD len;
	BYTE i,j;
    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	
    SendFrameHead(GET_addr);
	SendDWordLH(EqpName);
	if(m_pReceive->Address==0)
		thSleep((g_Sys.AddrInfo.wAddr%20)*100);		//20台每台间隔1秒
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(g_Sys.AddrInfo.wAddr&0xff);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(g_Sys.AddrInfo.wAddr>>8);
	memset(m_SendBuf.pBuf+m_SendBuf.wWritePtr,0,8);
	i=0;
	for(j=0;(j<20)&&(g_Sys.AddrInfo.Lan1.sIP[j]>20);j++)
	{
		if((g_Sys.AddrInfo.Lan1.sIP[j]>='0')&&(g_Sys.AddrInfo.Lan1.sIP[j]<='9'))
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr]*=10;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr]+=g_Sys.AddrInfo.Lan1.sIP[j]&0xf;
		}
		else
		{
			i++;
			m_SendBuf.wWritePtr++;
		}
		if(i>4) break;
	}
	m_SendBuf.wWritePtr++;

	i=0;
	for(j=0;(j<20)&&(g_Sys.AddrInfo.Lan2.sIP[j]>20);j++)
	{
		if((g_Sys.AddrInfo.Lan2.sIP[j]>='0')&&(g_Sys.AddrInfo.Lan2.sIP[j]<='9'))
		{
			m_SendBuf.pBuf[m_SendBuf.wWritePtr]*=10;
			m_SendBuf.pBuf[m_SendBuf.wWritePtr]+=g_Sys.AddrInfo.Lan2.sIP[j]&0xf;
		}
		else
		{
			i++;
			m_SendBuf.wWritePtr++;
		}
		if(i>4) break;	
	}
	m_SendBuf.wWritePtr++;

	SendFrameTail();
	m_wirecommflag=0x55;;

}

void CMaint::ProcFormat()
{
	char buf[30];
	
	BYTE* pData = m_pFrameData;

	if(*pData == 0x01)
		sprintf(buf,"rm -f -r %s/*",SYS_PATH_ROOT);
	else
		sprintf(buf,"rm -f -r %s/*",SYS_PATH_ROOT2);
		
	shellprintf("format %s...\n", buf);
	thSleep(100);
	if (system(buf) == OK)    //cjl  缺少删除操作
		{
			SendACK();
		shellprintf("done!\n");
			return;
		}
	SendNACK(MAINT_DISK_ERR);
	shellprintf("failed!\n");
	return;
}
void CMaint::read_comm()
{

//	DWORD EqpName;
 //   	WORD len;
//		BYTE i,j;
//    EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	
    SendFrameHead(GET_COMM);
	
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(BYTE)m_wTaskID;
	
 
	SendFrameTail();

}

#ifdef YC_GAIN_NEW
void CMaint::read_ycgainval(void)
{
	BYTE  start = *m_pFrameData;
	int   num;
#ifdef INCLUDE_YC
	if (start == 0)
		gainnum = gainvalget((BYTE*)m_dwPubBuf);

	num = gainnum-start;
	if (num > 50) num = 50;

	SendFrameHead(YC_PARAVALGET);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)gainnum;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = start;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)num;
    memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr, (BYTE*)m_dwPubBuf+start*sizeof(VYcVal), num*sizeof(VYcVal));
	m_SendBuf.wWritePtr += (WORD)(num*sizeof(VYcVal));
	SendFrameTail();
#else
    SendNACK(1);
#endif
}

void CMaint::read_ycgain(void)
{
 	BYTE  start = *m_pFrameData;
	int   num;
#ifdef INCLUDE_YC
	if (start == 0)
		gainnum = gainget((BYTE*)m_dwPubBuf);

	num = gainnum-start;
	if (num > 50) num = 50;

	SendFrameHead(YC_PARAGET);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)gainnum;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = start;
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)num;
    memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr, (BYTE*)m_dwPubBuf+start*sizeof(VYcGain), num*sizeof(VYcGain));
	m_SendBuf.wWritePtr += (WORD)(num*sizeof(VYcGain));
	SendFrameTail();
#else
    SendNACK(1);
#endif
}

void CMaint::write_ycgainval(void)
{
	int ret=OK;
	BYTE *pData = m_pFrameData;
	BYTE flag = pData[0];
	BYTE total = pData[1];
	BYTE start = pData[2];
	BYTE num = pData[3];

#ifdef INCLUDE_YC
	memcpy((BYTE*)m_dwPubBuf+start*sizeof(VYcVal), pData+4, num*sizeof(VYcVal));	
    if ((start+num) >= total)
	   ret = gainvalset(flag, total, (BYTE*)m_dwPubBuf);

	if (ret == OK)
	{
		SendACK();
		WriteDoEvent(NULL, 0, "整定!");
	}
	else
#endif
	{
		SendNACK(1);
		WriteDoEvent(NULL, 0, "整定失败!");
	}
}

void CMaint::write_ycgain(void)
{
    int ret=OK;
	BYTE *pData = m_pFrameData;
	BYTE total = pData[0];
	BYTE start = pData[1];
	BYTE num = pData[2];
#ifdef INCLUDE_YC
	memcpy((BYTE*)m_dwPubBuf+start*sizeof(VYcGain), pData+3, num*sizeof(VYcGain));

    if ((start+num) >= total)
	  ret = gainset((BYTE*)m_dwPubBuf);

	if (ret == OK)
	{
		SendACK();
		WriteDoEvent(NULL, 0, "人工设置整定系数!");
	}
	else
#endif
	{
		SendNACK(1);
		WriteDoEvent(NULL, 0, "人工设置整定系数失败!");
	}
}
#endif

void CMaint::SendSelfRunPara(void)            	
{
	WORD Shift,Num,type,i;
	WORD NumOffSet;
	DWORD EqpName;
	WORD EqpID;
	BYTE ErrorCode=1;
	WORD RecNum,NumAll;
	struct VParaInfo pVparaInfo;

	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;

	type = m_pFrameData[4];// 参数类型
	Shift = MAKEWORD(m_pFrameData[5], m_pFrameData[6]);//取偏移量
    	Num = MAKEWORD(m_pFrameData[7], m_pFrameData[8]);//取遥测数

	if(GetEqpID(EqpName,&EqpID))
	{
		SendNACK(ErrorCode);
		return;
	}

	if(type == 1)
	{
		NumAll= SELFPARA_NUM;
	}
	else if(type == 2)
	{
		NumAll = ((RPR_T_YXFD- RPR_V_DY) + 1) + ((RPR_DV_COS - RPR_DV_I) + 1) + ((RPR_V_IYXYB - RPR_V_ULP) +1)+((PRP_DEAD_LOAD - PRP_HAR_DEAD_V) +1);
	}
	else
	{
		SendNACK(ErrorCode);
		return;
	}
		
	SendFrameHead(GET_SELFRUNPARA);
	SendDWordLH(EqpName);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)type;
	
	SendWordLH(NumAll);
	
	SendWordLH(Shift);

	NumOffSet = m_SendBuf.wWritePtr;
	m_SendBuf.wWritePtr += 2;

	RecNum = 0;
	
	if((Shift == (RUNPARA_ADDR + RPR_DV_I))||(Shift == (PRPARA_TTU_ADDR + PRP_HAR_DEAD_V)))
	{
		for(i = 0 ;i < Num;i++)
		{
			memset(m_dwPubBuf , 0 , sizeof(struct VParaInfo));
		
			pVparaInfo.type = STRING_TYPE;
			sprintf(pVparaInfo.valuech,"没配置104规约");
			pVparaInfo.len = (BYTE)strlen(pVparaInfo.valuech);
			
			memcpy((void *)m_dwPubBuf,&pVparaInfo,sizeof(struct VParaInfo));
			RecNum ++;
			memcpy(&(m_SendBuf.pBuf[m_SendBuf.wWritePtr]), m_dwPubBuf,sizeof(struct VParaInfo));
			m_SendBuf.wWritePtr += sizeof(struct VParaInfo);
		}
	}
	else
	{
		for(i = 0 ;i < Num;i++)
		{
			memset(m_dwPubBuf , 0 , sizeof(struct VParaInfo));
			if(type == 1)
			{
				if(ReadSelfPara(Shift + i,(char*)m_dwPubBuf) == ERROR)
					break;
			}
			else
			{
				if(ReadRunPara(Shift + i,(char*)m_dwPubBuf) == ERROR)
					break;
			}
			RecNum ++;
			memcpy(&(m_SendBuf.pBuf[m_SendBuf.wWritePtr]), m_dwPubBuf,sizeof(struct VParaInfo));
			m_SendBuf.wWritePtr += sizeof(struct VParaInfo);
		}
	}
	SendWordLH(NumOffSet,RecNum);

	SendFrameTail();
	return;
}

void CMaint::WriteSelfRunPara(void)            	
{
	WORD Shift,Num,type,i;
	DWORD EqpName;
	WORD EqpID;
	BYTE *setpara;
	BYTE ErrorCode=1;
	BOOL bWrite = FALSE;
	WORD RecNum,NumAll;
	WORD wParaAddr;
	VFileMsgs fmsg;
	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;

	type = m_pFrameData[4];// 参数类型
	NumAll =  MAKEWORD(m_pFrameData[5], m_pFrameData[6]);//总个数
	Shift = MAKEWORD(m_pFrameData[7], m_pFrameData[8]);//取偏移量
    Num = MAKEWORD(m_pFrameData[9], m_pFrameData[10]);//本帧数据个数
	setpara = m_pFrameData + 11;
	if(GetEqpID(EqpName,&EqpID))
	{
		SendNACK(ErrorCode);
		return;
	}

	if(type == 1)
	{
		SendNACK(ErrorCode);
		return;
	}
	else if(type == 2)
	{
		if(NumAll >  ((
(RPR_T_YXFD - RPR_V_DY) + 1) + ((RPR_DV_COS - RPR_DV_I) + 1)+ ((RPR_V_IYXYB - RPR_V_ULP) + 1)+((PRP_DEAD_LOAD - PRP_HAR_DEAD_V) +1)))
		{
			SendNACK(ErrorCode);
			return;
		}
		if((Num + Shift) >  NumAll)
		{
			SendNACK(ErrorCode);
			return;
		}
		else if((Num + Shift) ==  NumAll)
		{
			bWrite = TRUE;
		}
		else
			bWrite = FALSE;
	}
	else
	{
		SendNACK(ErrorCode);
		return;
	}

	RecNum = 0;


	for(i = 0 ;i < Num;i++)
	{
		wParaAddr = MAKEWORD(setpara[0], setpara[1]);
		setpara += 2;
		if(((wParaAddr >= (RUNPARA_ADDR + RPR_DV_I)) && (wParaAddr <= (RUNPARA_ADDR + RPR_DV_COS)))||
			((wParaAddr >= (PRPARA_TTU_ADDR + PRP_HAR_DEAD_V)) && (wParaAddr <= (PRPARA_TTU_ADDR + PRP_DEAD_LOAD))))
		{

			setpara +=  sizeof(struct VParaInfo);
		}
		else
		{
			if(WriteRunPara(wParaAddr,(char*)setpara) != OK)
			{
				setpara +=  sizeof(struct VParaInfo);
				continue;
			}
		}
		RecNum ++;
		setpara +=  sizeof(struct VParaInfo);
	}
	
	if(RecNum != Num)
		SendNACK(ErrorCode);
	else
	{
		SendACK();
		if(bWrite)
			WriteRunParaFile();
	}
	return;
}

void CMaint::SendICmd(void)
{
	WORD Shift,Num,j,size;
	WORD NumOffSet;
	DWORD EqpName;
	WORD EqpID;
	BYTE ErrorCode=1;
	WORD RecNum,NumAll;
	
#if maint_test	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	//EqpName=0x00010000;

	Shift = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
    Num = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取遥测数

	if(GetEqpID(EqpName,&EqpID))
	{
		SendNACK(ErrorCode);
		return;
	}
	
	if(Shift == 0)
	{
		i();
	}
	GetINum(&NumAll,&size);
	
	SendFrameHead(GET_ICMD);
	SendDWordLH(EqpName);
	SendWordLH(NumAll);
	SendWordLH(Shift);

	NumOffSet = m_SendBuf.wWritePtr;
	m_SendBuf.wWritePtr += 2;

	RecNum = 0;
	for(j = 0;j < Num;j++)
	{
		memset(m_dwPubBuf , 0 , size);

		if(GetThreadsts(Shift + j,(BYTE*)m_dwPubBuf) == ERROR)
			break;
		RecNum ++;
		memcpy(&(m_SendBuf.pBuf[m_SendBuf.wWritePtr]), m_dwPubBuf,size);
		m_SendBuf.wWritePtr += size;
	}
	SendWordLH(NumOffSet,RecNum);
	if((RecNum + Shift) == NumAll)
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0xFF;
	else
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++]  = 0;
	
	SendFrameTail();
#endif
}

void CMaint::SendSoeCosdotdef(void)
{
	WORD dotoff;
	WORD dotnum;
	WORD SOENumOffSet;
	DWORD EqpName;
	WORD EqpID;
	WORD EventType;
	BYTE ErrorCode = 1;

	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	EventType=m_pFrameData[4];
	dotoff = MAKEWORD(m_pFrameData[5], m_pFrameData[6]);
	dotnum = MAKEWORD(m_pFrameData[7], m_pFrameData[8]);
	if (GetEqpID(EqpName, &EqpID))
	{
		SendNACK(ErrorCode);
		return;
	}


	SendFrameHead(GET_SoeCosDotDef);
	SendDWordLH(EqpName);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++]=(BYTE)EventType;
	SendWordLH(dotoff);
	SOENumOffSet = m_SendBuf.wWritePtr; //记录SOE个数存放位置
	m_SendBuf.wWritePtr += 2;
	if(dotnum>20) 
		dotnum=20;
	switch(EventType)
	{
	case 2:/*单点遥信描述*/
		dotnum=read_yxSOEdef(EqpID,dotoff,dotnum);
		break;
	case 3:/*双点遥信描述*/
		dotnum=read_dyxSOEdef(EqpID,dotoff,dotnum);
		break;
	default:
		break;
	}
	SendWordLH(SOENumOffSet, dotnum);
	SendFrameTail();
	return;
}

//ssz
void CMaint::setGainCommand()
{
	DWORD EqpName;
	WORD EqpID;
	BYTE byCommandNo;
	BYTE ErrorCode = 1;


	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	byCommandNo = m_pFrameData[4];
	if (GetEqpID(EqpName, &EqpID))
	{
		SendNACK(ErrorCode);
		return;
	}
#ifdef INCLUDE_YC
	if(1 == byCommandNo)
	{
		gainread();
		SendACK();
	}
	else if(2 == byCommandNo)
	{
		gainclear();
		SendACK();
	}
#ifdef INCLUDE_ATT7022
	else if (3 == byCommandNo)
	{
	    att_gainread();
		SendACK();
	}
	else if (4 == byCommandNo)
	{
	    att_gainclear();
		SendACK();
	}
#endif
	else
	{
		SendNACK(ErrorCode);
	}
#endif
	{
		SendNACK(ErrorCode);
	}
}

void CMaint::setDevID()
{	
	BOOL setok = TRUE;
	int i;
	BYTE* pData = m_pFrameData;

	extNvRamSet(NVRAM_DEV_ID,(BYTE*)pData,NVRAM_DEV_ID_LEN);

	extNvRamGet(NVRAM_DEV_ID,(BYTE*)&g_Sys.InPara[SELFPARA_ID][0],NVRAM_DEV_ID_LEN);

	for(i = 0; i < NVRAM_DEV_ID_LEN;i++)
	{
		if((BYTE)g_Sys.InPara[SELFPARA_ID][i] != pData[i])
			setok = FALSE;
			
	}
	if (setok)
	{
		SendACK();	
	}
	else
	{
		SendNACK(1);
	}
	return;
}

void CMaint::SendJiami()
{	
	BOOL setok0 = FALSE;
	BOOL setok1 = FALSE;
#ifdef ECC_MODE_CHIP
	WORD CerLen,i;
	FILE *fp;
	char path[4*MAXFILENAME];
  	char temp[10];
	BYTE* buf = (BYTE*)g_pParafile;
	DWORD EqpName;
	//by lvyi ,区分湖南和国网加密芯片
	setok0 = JiamiInit(0);
	setok1 = JiamiInit(1);
  	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
#endif
	if (setok0||setok1)
	{
#ifdef ECC_MODE_CHIP	
		if(setok0)
		{
			EqpName = 0;
		}
		else
		{
			EqpName = EqpName;
		}
		SendFrameHead(Get_JIAMI);
		SendDWordLH(EqpName);
		
		
		memset(buf,0,1024);
		memset(path,0,sizeof(path));
		if(setok0)
		{//1120A
			SecurityChipID_HN(buf,&CerLen);
		}
		else
		{
			SecurityChipID(buf,&CerLen);
		}
		
		sprintf(path,"%02X",buf[0]);
		for(i = 1; i < CerLen; i++)
		{
			sprintf(temp, "%02X",buf[i]);
			strcat(path,temp);
		}
		myprintf(m_wTaskID, LOG_ATTR_INFO,"Encrypt Chip Serial Num: %s ",path);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(CerLen);
		memcpy(&(m_SendBuf.pBuf[m_SendBuf.wWritePtr]), buf,CerLen);
		m_SendBuf.wWritePtr += CerLen;
		
		memset(buf,0,1024);
		memset(path,0,sizeof(path));
		if(setok0)
		{//1120A
			SecurityKeyVersion_HN(buf,&CerLen);
		}
		else
		{
			SecurityKeyVersion(buf,&CerLen);
		}
	
	
		sprintf(path,"%02X",buf[0]);
		for(i = 1; i < CerLen; i++)
		{
			sprintf(temp, "%02X",buf[i]);
			strcat(path,temp);
		}
		myprintf(m_wTaskID, LOG_ATTR_INFO,"Encrypt Chip VER: %s ",path);
		
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(CerLen);
		memcpy(&(m_SendBuf.pBuf[m_SendBuf.wWritePtr]), buf,CerLen);
		m_SendBuf.wWritePtr += CerLen;
		SendFrameTail();
		if(setok1)
		{
			SecurityGetWorkCerData(buf,&CerLen);
			GetMyPath(path, "cer.cer");
			fp = fopen(path, "wb");
			if (fp == NULL) return;
			fwrite(buf, 1, CerLen, fp);
			fflush(fp);
			fsync(fileno(fp));
			fclose(fp);
		}
		
#endif		
	}
	else
	{
		SendNACK(1);
	}
	return;
}

void CMaint::Do_Lubo()
{
#if 0
	DWORD EqpName;
	WORD line;
	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	line = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);

#ifdef INCLUDE_PR
	if(line < fdNum)
		g_prlubo |= (1 << line);
#endif
	SendFrameHead(GET_Lubo);
  	SendDWordLH(EqpName);
	SendWordLH(line);
	SendFrameTail();
	return;
#else
	WORD rcvlen = 0;
	
	if(Maint_BM_RCV(GET_Lubo,m_pFrameData,MAKEWORD(m_pReceive->Len1Low, m_pReceive->Len1High) -3,
	(BYTE*)m_dwPubBuf, &rcvlen) == ERROR)
	{
		SendNACK(1);
		return;
	}
	
	SendFrameHead(GET_Lubo);
	memcpy(m_SendBuf.pBuf + m_SendBuf.wWritePtr, m_dwPubBuf ,rcvlen);
	m_SendBuf.wWritePtr = m_SendBuf.wWritePtr + rcvlen;
	SendFrameTail();
#endif
}

void CMaint::setDevType()
{	
	BOOL setok = TRUE;
	int i;
	BYTE  len,devflag;
	BYTE* pData = m_pFrameData+1;

	len = m_pFrameData[0];

	if(len > PARACHAR_MAXLEN)
		SendNACK(1);

	memset(&g_Sys.InPara[SELFPARA_DEVTYPE][0],0,PARACHAR_MAXLEN);
	memcpy((void *)&g_Sys.InPara[SELFPARA_DEVTYPE][0],pData,(DWORD)len);
	extNvRamSet(NVRAM_DEV_TYPE,(BYTE*)&g_Sys.InPara[SELFPARA_DEVTYPE][0],PARACHAR_MAXLEN);
	extNvRamGet(NVRAM_DEV_TYPE,(BYTE*)&g_Sys.InPara[SELFPARA_DEVTYPE][0],PARACHAR_MAXLEN);

	devflag = 0x5A;
	extNvRamSet(NVRAM_DEV_TYPE_FLAG,(BYTE*)&devflag,1);

	for(i = 0; i < len;i++)
	{
		if((BYTE)g_Sys.InPara[SELFPARA_DEVTYPE][i] != pData[i])
			setok = FALSE;
			
	}
	if (setok)
	{
		SendACK();	
	}
	else
	{
		SendNACK(1);
	}
	return;
}

void CMaint::setDevMFR() //gb2312编码
{	
	BOOL setok = TRUE;
	int i;
	BYTE  len,devflag;
	BYTE* pData = m_pFrameData+1;

	len = m_pFrameData[0];

	if(len > Old_MAXLEN)
	SendNACK(1);

	memset(&g_Sys.InPara[SELFPARA_MFR_GB2312][0],0,PARACHAR_MAXLEN);
	memcpy(&g_Sys.InPara[SELFPARA_MFR_GB2312][0],pData,len);
	extNvRamSet(NVRAM_DEV_MFR,(BYTE*)&g_Sys.InPara[SELFPARA_MFR_GB2312][0],Old_MAXLEN);
	extNvRamGet(NVRAM_DEV_MFR,(BYTE*)&g_Sys.InPara[SELFPARA_MFR_GB2312][0],Old_MAXLEN);

	devflag = 0x5A;
	extNvRamSet(NVRAM_DEV_MFR_FLAG,(BYTE*)&devflag,1);
    
	for(i = 0; i < len;i++)
	{
		if((BYTE)g_Sys.InPara[SELFPARA_MFR_GB2312][i] != pData[i])
			setok = FALSE;
	}
	if (setok)
	{
		SendACK();	
	}
	else
	{
		SendNACK(1);
	}
	return;
}

void CMaint::setDevMFR_Utf8() //uft-8 编码
{	
	BOOL setok = TRUE;
	int i;
	BYTE  len,devflag;
	BYTE* pData = m_pFrameData+1;

	len = m_pFrameData[0];

	if(len > PARACHAR_MAXLEN)
	SendNACK(1);

	memset(&g_Sys.InPara[SELFPARA_MFR][0],0,PARACHAR_MAXLEN);
	memcpy(&g_Sys.InPara[SELFPARA_MFR][0],pData,len);
	extNvRamSet(NVRAM_DEV_MFR_UTF8,(BYTE*)&g_Sys.InPara[SELFPARA_MFR][0],len);
	extNvRamGet(NVRAM_DEV_MFR_UTF8,(BYTE*)&g_Sys.InPara[SELFPARA_MFR][0],len);

	devflag = 0x5A;
	extNvRamSet(NVRAM_DEV_MFR_UTF8_FLAG,(BYTE*)&devflag,1);
    
	for(i = 0; i < len;i++)
	{
		if((BYTE)g_Sys.InPara[SELFPARA_MFR][i] != pData[i])
			setok = FALSE;
	}
	if (setok)
	{
		SendACK();	
	}
	else
	{
		SendNACK(1);
	}
	return;
}


void CMaint::setDevSV()
{	
	BOOL setok = TRUE;
	int i;
	BYTE  len,svflag,hvflag;
	BYTE* pData = m_pFrameData+1;
	len = m_pFrameData[0];
	if(len > PARACHAR_MAXLEN)
		SendNACK(1);

	if(strstr((char*)pData,"HV"))
	{
		memset(&g_Sys.InPara[SELFPARA_HV][0],0,PARACHAR_MAXLEN);
		memcpy((void *)&g_Sys.InPara[SELFPARA_HV][0],pData,(DWORD)len);
		extNvRamSet(NVRAM_DEV_HV,(BYTE*)&g_Sys.InPara[SELFPARA_HV][0],PARACHAR_MAXLEN);
		extNvRamGet(NVRAM_DEV_HV,(BYTE*)&g_Sys.InPara[SELFPARA_HV][0],PARACHAR_MAXLEN);
		hvflag = 0x5A;
		extNvRamSet(NVRAM_DEV_HV_FLAG,(BYTE*)&hvflag,1);
		for(i = 0; i < len;i++)
		{
			if((BYTE)g_Sys.InPara[SELFPARA_HV][i] != pData[i])
				setok = FALSE;
		}
	}
	else
	{
		memset(&g_Sys.InPara[SELFPARA_SV][0],0,PARACHAR_MAXLEN);
		memcpy((void *)&g_Sys.InPara[SELFPARA_SV][0],pData,(DWORD)len);
		extNvRamSet(NVRAM_DEV_SV,(BYTE*)&g_Sys.InPara[SELFPARA_SV][0],PARACHAR_MAXLEN);
		extNvRamGet(NVRAM_DEV_SV,(BYTE*)&g_Sys.InPara[SELFPARA_SV][0],PARACHAR_MAXLEN);
		svflag = 0x5A;
		extNvRamSet(NVRAM_DEV_SV_FLAG,(BYTE*)&svflag,1);
		for(i = 0; i < len;i++)
		{
			if((BYTE)g_Sys.InPara[SELFPARA_SV][i] != pData[i])
				setok = FALSE;
		}
	}
	
	if (setok)
	{
		SendACK();	
	}
	else
	{
		SendNACK(1);
	}
	return;
}

void CMaint::Set_Pb(void)            	
{
#ifdef INCLUDE_PB_RELATE
	DWORD EqpName;
	WORD EqpID;
	WORD Shift;
	BYTE line;
	BYTE *pdata=m_pFrameData+8;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	line = (BYTE)MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
	Shift = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取偏移量
	if (GetEqpID(EqpName, &EqpID))
	{
		SendNACK(2);
		return;
	}

	if(WritePbSet(pdata,Shift) != OK)
	{		
		SendNACK(1);
		return;
	}
  	SendFrameHead(SET_PB);
	SendDWordLH(EqpName);
 	SendWordLH(line);
	SendWordLH(Shift);
	memcpy(m_SendBuf.pBuf+m_SendBuf.wWritePtr,pdata,(DWORD)Shift);
	m_SendBuf.wWritePtr+= Shift;
	SendFrameTail();
	return;
#else
	SendNACK(1);
	return;
#endif
}

void CMaint::Get_Pb(void)            	
{
#ifdef INCLUDE_PB_RELATE
	DWORD EqpName;
	WORD EqpID;
	WORD Shift;
	BYTE line;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	line = (BYTE)MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//取偏移量
	Shift = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//取偏移量
	if (GetEqpID(EqpName, &EqpID))
	{
		SendNACK(2);
		return;
	}
	SendFrameHead(GET_PB);
	SendDWordLH(EqpName);
	SendWordLH(line);
    SendWordLH(Shift);
	ReadPbSet(m_SendBuf.pBuf+m_SendBuf.wWritePtr,Shift);
	m_SendBuf.wWritePtr += Shift;
	SendFrameTail();
#else
	SendNACK(1);
	return;
#endif
}
#if 0
void CMaint::Set_jiamicer(void)            	
{
	WORD CerLen,Len,tmp;
	FILE *fp;
	char path[4*MAXFILENAME];
	BYTE* buf = (BYTE*)g_pParafile;
	DWORD EqpName;
	BOOL ret = TRUE;
	
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	Len = m_pFrameData[4];
	memset(path, 0, 4*MAXFILENAME);
	m_pFrameData += 5;
	if(Len > 4*MAXFILENAME)
	{
		SendNACK(1);
		return;
	}
	memcpy(path, (char*)m_pFrameData,Len);

//	strcpy(path,"/tffsa/other/cer.cer");
//  Len = strlen(path);   
	
	fp = fopen(path,"r");
	if(fp == NULL) 
	{
		SendNACK(1);
		return;
	}
	CerLen = fread(buf,1,MAXFILELEN,fp);
	fclose(fp);
	ret = SecurityUpdateCer(6,buf,CerLen,buf+8*1024,&tmp,0);
	if(ret)
	{
		SendFrameHead(SET_JIAMICER);
		SendDWordLH(EqpName);
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = LOBYTE(Len);
		memcpy(&(m_SendBuf.pBuf[m_SendBuf.wWritePtr]), path,Len);
		m_SendBuf.wWritePtr += Len;
		SendFrameTail();
	}
	else
	{
		SendNACK(1);
	}
	
	return;
}
#endif
void CMaint::GetADType()
{
#ifdef INCLUDE_YC
	DWORD EqpName;
	WORD EqpID;
	WORD Shift;
	BYTE line;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	line = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//????
	Shift = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//????
	if (GetEqpID(EqpName, &EqpID))
	{
		SendNACK(2);
		return;
	}
	line = LOWORD(ggainver);
	Shift = HIWORD(ggainver);
	SendFrameHead(GET_ADType);
	SendDWordLH(EqpName);
	SendWordLH(line);
  	SendWordLH(Shift);
	SendFrameTail();
#else
	SendNACK(1);
	return;
#endif
}

void CMaint::SetADType()
{
#ifdef INCLUDE_YC
	DWORD EqpName;
	WORD EqpID;
	WORD Shift;
	DWORD adtype;
	BYTE line;
	EqpName = MAKEDWORD(MAKEWORD(m_pFrameData[0], m_pFrameData[1]), MAKEWORD(m_pFrameData[2], m_pFrameData[3]));
	line = MAKEWORD(m_pFrameData[4], m_pFrameData[5]);//
	Shift = MAKEWORD(m_pFrameData[6], m_pFrameData[7]);//
	adtype = MAKEDWORD(line,Shift);
	if (GetEqpID(EqpName, &EqpID))
	{
		SendNACK(2);
		return;
	}
	if(gainsetadtype(adtype) == ERROR)
	{
		SendNACK(2);
		return;
	}
	SendFrameHead(SET_ADType);
	SendDWordLH(EqpName);
	SendWordLH(line);
  	SendWordLH(Shift);
	SendFrameTail();
#else
	SendNACK(1);
	return;
#endif
}

void CMaint::Set_Esn()
{	
	SendNACK(1);
}

void CMaint::Get_Esn()
{	
	SendNACK(1);
	return;
}

void CMaint::Get_LTE()
{
	SendNACK(1);
	return;
}


