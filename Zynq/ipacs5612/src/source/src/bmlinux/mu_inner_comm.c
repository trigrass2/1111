/*
 * @Author: zhongwei
 * @Date: 2020/2/6 16:11:27
 * @Description: 与BM内部通讯
 * @File: mu_inner_comm.cc
 *
*/

#include "plt_include.h"
#include "mu_inner_comm.h"
#include <syslog.h>
#include <unistd.h>
#include "clock.h"

extern void commBufFill(int no, BYTE flag, int len, BYTE *buf);

//发送命令行到BM
void bmdo(const char * cmd)
{
    if (IsSharedLinuxMemValid())
    {
        TMC_COMMAND_LINE *pCmdLine = GetSharedLinuxCommandLine();
        if (pCmdLine->bCmd == BM_FALSE)    //FLASE才允许发送命令
        {
            E_STRNCPY(pCmdLine->str_cmd, cmd, sizeof(pCmdLine->str_cmd));
            pCmdLine->flag_begin = FLAG_SHARED_MEMORY_VALID;
            pCmdLine->flag_end = FLAG_SHARED_MEMORY_VALID;
            pCmdLine->bCmd = BM_TRUE;
            PRINTF_WITH_LOG("Send command to BM <%s>\n\r", pCmdLine->str_cmd);
        }
        else
        {
            PRINTF("BM Command isn't execute yet, cann't send another one.\n\r");
        }
    }   
    else
    {
        PRINTF("IsSharedLinuxMemValid is false, cann't send BM command.\n\r");
    }
}

//BM<->Linux内部通讯初始化 - 在shared memory初始化之后调用
void PowerOn_Init_McInnerComm(void)
{
    TMC_INNER_COMM_STRUCT *pInnerSend = GetSharedLinuxInnerComm();
    if (pInnerSend != NULL && IsSharedLinuxMemValid())
    {
        pInnerSend->nPkgBufLen = MC_INNER_COMM_PKG_BUF_NUM;
        pInnerSend->nBegin = pInnerSend->nEnd = 0;              //已发送清空
    }

    TMC_INNER_COMM_STRUCT *pInnerRcv = GetSharedBMInnerComm();
    if (pInnerRcv != NULL && IsSharedMemoryValid())
    {
        //这个暂时不做处理
    }

	mc_innerbuf_init();
}

//接收缓冲区是否为空
SECTION_PLT_CODE BOOL mc_inner_rcv_buf_is_empty(void)
{
    TMC_INNER_COMM_STRUCT *pInnerRcv = GetSharedBMInnerComm();
    if (IsSharedBMMemValid())
    {
        return BoolFrom(pInnerRcv->nBegin == pInnerRcv->nEnd);
    }

    return PLT_TRUE;    //其他清空下均为空
}


/**
 * @Function: mc_inner_rcv_from_linux
 * @Description: 从BM获取通讯数据的接口 
 * @author zhongwei (2019/11/28 15:41:09)
 * 
 * @param data 
 * @param max_len 
 * 
 * @return SECTION_PLT_CODE int32 
 * 
*/
SECTION_PLT_CODE int32 mc_inner_rcv_from_bm(uint8 * data, int32 max_len)
{
    TMC_INNER_COMM_STRUCT *pInnerRcv = GetSharedBMInnerComm();
    if (IsFalse(mc_inner_rcv_buf_is_empty()))
    {
        int32 nRcvLen = 0;
        int32 nBegin = pInnerRcv->nBegin;
        const TMC_INNER_COMM_PKG *pPkg = &pInnerRcv->pkgBuf[nBegin];

        if (pPkg->len <= max_len)
        {
            nRcvLen = pPkg->len;
            E_MEMCPY(data, pPkg->data, nRcvLen);
        }

        nBegin++;
        if (nBegin >= MC_INNER_COMM_PKG_BUF_NUM)
        {
            nBegin = 0;
        }

        pInnerRcv->nBegin = nBegin;

        return nRcvLen;
    }

    return 0;
}

//缓冲区是否满
SECTION_PLT_CODE BOOL mc_inner_send_buf_is_full(void)
{
    TMC_INNER_COMM_STRUCT *pInnerSend = GetSharedLinuxInnerComm();
    return BoolFrom(pInnerSend->nBegin == (1+pInnerSend->nEnd) ||
            (pInnerSend->nBegin == 0 && pInnerSend->nEnd == MC_INNER_COMM_PKG_BUF_NUM));
}

/**
 * @Function: mc_inner_ClearSendBuf
 * @Description: 清空发送缓冲区 
 * @author 
 * 
 * @param void 
 * 
*/
SECTION_PLT_CODE void mc_inner_ClearSendBuf(void)
{
    TMC_INNER_COMM_STRUCT *pInnerSend = GetSharedLinuxInnerComm();
    if (IsSharedLinuxMemValid())
    {
        int i;
        pInnerSend->nBegin = pInnerSend->nEnd = 0;
        for (i=0;i<MC_INNER_COMM_PKG_BUF_NUM;i++)
        {
            pInnerSend->pkgBuf[i].len = 0;      //清空数据
        }
    }
}

/**
 * @Function: mc_inner_send_to_bm
 * @Description: 发送报文到BM的接口 
 * @author 
 * 
 * @param data 
 * @param len 
 * 
 * @return SECTION_PLT_CODE BOOL 
 * 
*/
SECTION_PLT_CODE BOOL mc_inner_send_to_BM(const uint8 * data, int32 len)
{
    TMC_INNER_COMM_STRUCT *pInnerSend = GetSharedLinuxInnerComm();
    ASSERT_L1(data != NULL && len > 0);
    if (len > MC_INNER_COMM_PKG_LEN)    //超长，不发送
    {
        LOGMSG_L2("mc_inner_send_to_linux len=%d, too long!\n\r", len,0,0,0,0,0);
        return PLT_FALSE;
    }

    if (IsSharedLinuxMemValid())
    {
        int32 nEnd;
        if (mc_inner_send_buf_is_full())
        {
            LOGMSG_L1("mc_inner_buf_is_full, send fail.\n\r",0,0,0,0,0,0);
            return PLT_FALSE;
        }

        //检查缓冲区是否正常
        if (pInnerSend->nEnd < 0 || pInnerSend->nEnd >= MC_INNER_COMM_PKG_BUF_NUM)
        {
            mc_inner_ClearSendBuf();
            LOGMSG_L2("mc_inner_send_to_BM buf end error, clean all!\n\r",0,0,0,0,0,0);
        }

        nEnd = pInnerSend->nEnd;
        E_MEMCPY(pInnerSend->pkgBuf[nEnd].data, data, len);
        pInnerSend->pkgBuf[nEnd].len = len;
        nEnd++;
        if (nEnd >= MC_INNER_COMM_PKG_BUF_NUM)
        {
            nEnd = 0;
        }

        pInnerSend->nEnd = nEnd;

        return PLT_TRUE;
    }

    return PLT_FALSE;
}






////////////////////////////////////////////////////////////////

static VInnerBuf SendBuf,RecBuf,RecFrame;
DWORD g_InnerSem = 0;
WORD RecvCode = 0;

void mc_innerbuf_init(void)
{
	E_MEMSET(&SendBuf,0,sizeof(VInnerBuf));
	E_MEMSET(&RecBuf,0,sizeof(VInnerBuf));
	E_MEMSET(&RecFrame,0,sizeof(VInnerBuf));
	
	SendBuf.pBuf = (BYTE *)malloc(INNERBUFLEN*sizeof(BYTE));
	SendBuf.wBufSize = INNERBUFLEN;
	RecBuf.pBuf = (BYTE *)malloc(4*INNERBUFLEN*sizeof(BYTE));
	RecBuf.wBufSize = 4*INNERBUFLEN;
	
	g_InnerSem = smMCreate();	
}

//comm任务一直巡检接收？，然后处理报文
static void SendFrameHead(WORD wCode)
{
	struct VInnerFrame *pSend = (struct VInnerFrame *)SendBuf.pBuf;
	SendBuf.wReadPtr = SendBuf.wWritePtr = 0;

	pSend->StartCode1 = 0x68;
    pSend->StartCode2 = 0x68;
    pSend->Address = 0x00; 
	pSend->CMD= HIBYTE(wCode);
	pSend->afn= LOBYTE(wCode); 
	SendBuf.wWritePtr = sizeof(struct VInnerFrame);
}


extern WORD CheckSum(BYTE *buf, int l);
static void SendFrameTail(void)
{
	WORD Len = SendBuf.wWritePtr-6;
	struct VInnerFrame *pSend = (struct VInnerFrame *)SendBuf.pBuf;
	WORD CrcCode;

    //写帧长度
	pSend->Len1Low= LOBYTE(Len);
	pSend->Len1High= HIBYTE(Len);
 	pSend->Len2Low= LOBYTE(Len);
	pSend->Len2High= HIBYTE(Len);
   	//CRC校验
	CrcCode = CheckSum((BYTE *)&pSend->Address, Len);  
	
	SendBuf.pBuf[SendBuf.wWritePtr++] = LOBYTE(CrcCode);
	SendBuf.pBuf[SendBuf.wWritePtr++] = HIBYTE(CrcCode);
	SendBuf.pBuf[SendBuf.wWritePtr++] = 0x16;
	
	return;
}

void SendBMFrame(void)
{
	smMTake(g_InnerSem);
	if (SendBuf.wWritePtr - SendBuf.wReadPtr)
	{
		if(mc_inner_send_to_BM(SendBuf.pBuf,SendBuf.wWritePtr - SendBuf.wReadPtr) == PLT_FALSE)
		{
			printf("mc_inner_send_to_BM error \n");
		}
	}
	smMGive(g_InnerSem);
}

void NeatenInnerBuf()
{
	register unsigned  int i,j;

	if (RecBuf.wReadPtr == 0)
	{
		if(RecBuf.wWritePtr >= RecBuf.wBufSize)
		{
			RecBuf.wWritePtr = 0;
			printf("NeatenCommBuf clearbuf len %d ",RecBuf.wWritePtr);
		}
		return ; //读指针已经为0
	}


	if (RecBuf.wReadPtr >= RecBuf.wWritePtr)
	{
		RecBuf.wReadPtr = RecBuf.wWritePtr=0;
		return ;
	}


	if (RecBuf.wWritePtr >= RecBuf.wBufSize)
	{
		RecBuf.wReadPtr = 0;
		RecBuf.wWritePtr = 0;
		return ;
	}

	i = 0; 
	j = RecBuf.wReadPtr;
	while (j < RecBuf.wWritePtr)
	{
		RecBuf.pBuf[i++] = RecBuf.pBuf[j++];
	}

	RecBuf.wReadPtr = 0; 
	RecBuf.wWritePtr = i; 
}

static DWORD SearchOneFrame(BYTE *Buf,int Len)
{
	struct VInnerFrame *pHead;
	int len1, len2;
	WORD crc1,crc2;

	if (Len < sizeof(struct VInnerFrame))  return(INNER_FRAME_LESS);
		
	pHead = (struct VInnerFrame *)Buf;

	if ((pHead->StartCode1!= 0x68) || (pHead->StartCode2 != 0x68))  return(INNER_FRAME_ERR|1);	

	len1 = (pHead->Len1High<<8)|pHead->Len1Low;

	len2 = (pHead->Len2High<<8)|pHead->Len2Low;

	if (len1 != len2) return(INNER_FRAME_ERR|1);

	if (len1 > 1280) return(INNER_FRAME_ERR|1);

	len2 = len1 + 6;

	if (Len < (len2+3)) return(INNER_FRAME_LESS);

	crc1 = (Buf[len2+1]<<8)|Buf[len2];
	crc2 = CheckSum(&(pHead->Address), len1);

	if (crc1 !=crc2) return(INNER_FRAME_ERR|1);

	return(INNER_FRAME_OK|(len2+3));	
}

static BOOL SearchFrame(BOOL *bHaveRxFrm)
{
	DWORD rc;
	WORD procLen;  
	int dataLen;

	while (1)
	{
		dataLen = RecBuf.wWritePtr - RecBuf.wReadPtr;
		if (dataLen <= 0)
		{
		    *bHaveRxFrm=FALSE;
    		return(FALSE);
		}  

		rc = SearchOneFrame(&RecBuf.pBuf[RecBuf.wReadPtr], dataLen);
		procLen =(WORD)(rc & ~INNER_FRAME_SHIELD);

		switch	(rc & INNER_FRAME_SHIELD)
		{
			case INNER_FRAME_OK:
				RecFrame.pBuf = &RecBuf.pBuf[RecBuf.wReadPtr];  
				RecFrame.wWritePtr = procLen;
				RecBuf.wReadPtr += procLen;
				if (RecBuf.wReadPtr >= RecBuf.wWritePtr)
				{
					*bHaveRxFrm=FALSE;
				}
				return TRUE;
			case INNER_FRAME_ERR:
				if (!procLen)  RecBuf.wReadPtr++;
				else  RecBuf.wReadPtr += procLen; 
				break;
			case INNER_FRAME_LESS:
				RecBuf.wReadPtr += procLen; 
				*bHaveRxFrm=FALSE;
				return FALSE;
		}
	}
}

/*
static void SendACK()
{
    SendFrameHead(INNER_ACK_OK);
	SendFrameTail();
}
*/

static void SendDWordLH(DWORD dwData)
{
	SendBuf.pBuf[SendBuf.wWritePtr++] = LOBYTE(LOWORD(dwData));
	SendBuf.pBuf[SendBuf.wWritePtr++] = HIBYTE(LOWORD(dwData));
	SendBuf.pBuf[SendBuf.wWritePtr++] = LOBYTE(HIWORD(dwData));
	SendBuf.pBuf[SendBuf.wWritePtr++] = HIBYTE(HIWORD(dwData));
}

static void SendWordLH(WORD wData)
{

	SendBuf.pBuf[SendBuf.wWritePtr++] = LOBYTE(wData);
	SendBuf.pBuf[SendBuf.wWritePtr++] = HIBYTE(wData);
}

/*
static void SendWordLHOff(DWORD dwWPtr,WORD wData)
{

	SendBuf.pBuf[dwWPtr++] = LOBYTE(wData);
	SendBuf.pBuf[dwWPtr++] = HIBYTE(wData);
}

static void SendDWordLHOff(DWORD dwWPtr,DWORD dwData)
{
	SendBuf.pBuf[dwWPtr++] = LOBYTE(LOWORD(dwData));
	SendBuf.pBuf[dwWPtr++] = HIBYTE(LOWORD(dwData));
	SendBuf.pBuf[dwWPtr++] = LOBYTE(HIWORD(dwData));
	SendBuf.pBuf[dwWPtr++] = HIBYTE(HIWORD(dwData));
}


static void SendNACK(BYTE errcode)
{
    SendFrameHead(INNER_ACK_ERR);
	SendBuf.pBuf[SendBuf.wWritePtr++]  = errcode;
	SendFrameTail();
}*/

static void ProcFrm()
{
	struct VInnerFrame *pRec = (struct VInnerFrame *)RecFrame.pBuf;
	WORD dwCode;
	
	dwCode = MAKEWORD(pRec->afn, pRec->CMD);
	RecvCode = dwCode;
	printf("RecvCode %08X \n",RecvCode);
    /*switch (dwCode)
    {
        case MAINT_PROG_READ:
			ProcProgramRead(pMaint);
		    break;
		case MAINT_PROG_WRITE:
			ProcProgramWrite(pMaint);
			break;
		case MAINT_BOOT_WRITE:
			ProcBootWrite(pMaint);
			break;
		case MAINT_FORMAT_DISK:
			ProcFormat(pMaint);
			break;
		case MAINT_RUN_INFO:
			ProcRunInfo(pMaint);
			break;
		default:
			SendNACK(INNER_CMD_ERR);
			break;
    }*/

	return;
}

void ProcInnerData()
{	
	BOOL bHaveRxFrm = TRUE;

	while (bHaveRxFrm)
	{
       if (SearchFrame(&bHaveRxFrm)==FALSE)  continue;
	   		ProcFrm();
	}
}

static int InnerReceiveBM()
{
	int Rc;

	RecvCode = 0x0000;
	NeatenInnerBuf();
	Rc = mc_inner_rcv_from_bm(RecBuf.pBuf+RecBuf.wWritePtr,RecBuf.wBufSize-RecBuf.wWritePtr);
	if(Rc > 0)
	{
		commBufFill(9, 0, Rc, RecBuf.pBuf+RecBuf.wWritePtr);
		
		/*{
			int i;
			printf("Recv BM: \n");
			for(i = 0; i < Rc;i++)
			{
				if(((i + 1) % 16) == 0)
					printf(" \n");
				printf("%02X ",RecBuf.pBuf[RecBuf.wWritePtr + i]);
			}
			printf(" \n");
		}*/
		
		RecBuf.wWritePtr += (WORD)Rc;
		ProcInnerData();
		return Rc;
	}
	return 0;
}

int RecvBM(BYTE* recvbuf,int *recvlen)
{
	int rc = 0;
	rc = mc_inner_rcv_from_bm(recvbuf,MC_INNER_COMM_PKG_LEN);
	if(rc > 0)
	{
		*recvlen = rc;
		return OK;
	}
	else
		return ERROR;
}

//临时透传Linux交互函数
int Linux_To_BM(const BYTE* sendbuf, int sendlen,BYTE* recvbuf,int *recvlen)
{
	int cnt = 0;
	
	smMTake(g_InnerSem);
	mc_inner_send_to_BM(sendbuf,sendlen);
	
	while(cnt < 100)
	{
		usleep(10000); //10ms
		cnt++;
		if(RecvBM(recvbuf,recvlen))
		{
			break;
		}
	}
	
	smMGive(g_InnerSem);

	if(cnt < 100)
		return OK;
	else
		return ERROR;
}

int MAINT_Linux_To_BM(WORD wCode)
{
	int cnt = 0;
	VSysClock SysTime;

	GetSysClock((void *)&SysTime, SYSCLOCK);
	printf("send %08X %d %d %d %d \n", wCode,SysTime.byHour,SysTime.byMinute,SysTime.bySecond,SysTime.wMSecond);
	
	smMTake(g_InnerSem);
	if(mc_inner_send_to_BM(SendBuf.pBuf,SendBuf.wWritePtr - SendBuf.wReadPtr) == PLT_FALSE)
	{
		smMGive(g_InnerSem);
		return ERROR;
	}

	commBufFill(9, 1, SendBuf.wWritePtr - SendBuf.wReadPtr, SendBuf.pBuf);
	/*{
		int i;
		printf("send BM: \n");
		for(i = 0; i < SendBuf.wWritePtr;i++)
		{
			if(((i + 1) % 16) == 0)
				printf(" \n");
			printf("%02X ",SendBuf.pBuf[i]);
		}
		printf(" \n");
	}*/
	while(cnt < 100)
	{
		usleep(5000); //5ms
		cnt++;
		InnerReceiveBM();
		if(RecvCode == (wCode | 0x8000)) 
		{
			break;
		}
		if(RecvCode == INNER_ACK_OK) 
		{
			break;
		}
		if(RecvCode == INNER_ACK_ERR)
		{
			break;
		}
	}
	smMGive(g_InnerSem);
	
	GetSysClock((void *)&SysTime, SYSCLOCK);
	printf("recv %d %d %d %d \n",SysTime.byHour,SysTime.byMinute,SysTime.bySecond,SysTime.wMSecond);
	
	if(cnt < 100)
		return OK;
	else
		return ERROR;
}


void sendLedIO(DWORD LedValue,DWORD* IoValue,int iolen)
{
	int i;
	
	SendFrameHead(INNER_SET_LEDIO);
	
	SendDWordLH(LedValue);
	
	for(i = 0; i < iolen;i++)
	{
		SendDWordLH(IoValue[i]);
	}
	
	SendFrameTail();

	MAINT_Linux_To_BM(INNER_ACK_OK);
}

int ReadIO(int no,int id,BYTE* value)
{
	struct VInnerFrame* pRec;
	BYTE *pData;
	SendFrameHead(INNER_GET_IO);
	SendDWordLH(no);
	SendDWordLH(id);
	SendFrameTail();

	if(MAINT_Linux_To_BM(INNER_GET_IO) == ERROR)
		return ERROR;
	pRec = (struct VInnerFrame *)RecFrame.pBuf;
	pData = (BYTE*)(pRec + 1);
	*value = pData[0];
	return OK;
}

//设置时间
int SetBMClock(struct VSysClock* systm)
{
	SendFrameHead(INNER_SET_CLK);
	SendWordLH(systm->wYear);
	SendBuf.pBuf[SendBuf.wWritePtr++] = systm->byMonth;
	SendBuf.pBuf[SendBuf.wWritePtr++] = systm->byDay;
	SendBuf.pBuf[SendBuf.wWritePtr++] = systm->byWeek;
	SendBuf.pBuf[SendBuf.wWritePtr++] = systm->byHour;
	SendBuf.pBuf[SendBuf.wWritePtr++] = systm->byMinute;
	SendBuf.pBuf[SendBuf.wWritePtr++] = systm->bySecond;
	SendWordLH(systm->wMSecond);
	
	SendFrameTail();
	
	return (MAINT_Linux_To_BM(INNER_ACK_OK));
}

int WriteYxTime_BM(WORD yxfdtime)
{
	SendFrameHead(INNER_YX_T);
	SendWordLH(yxfdtime);
	SendFrameTail();
	return(MAINT_Linux_To_BM(INNER_ACK_OK));
}

int WriteYkT_Bm(BYTE type,WORD time)
{
	SendFrameHead(INNER_YK_T);
	SendBuf.pBuf[SendBuf.wWritePtr++] = type;
	SendWordLH(time);
	SendFrameTail();
	return(MAINT_Linux_To_BM(INNER_ACK_OK));
}

//type 0 U  1 I  2 P 3 f 4 DC  5 cos
int	WriteBMZero(BYTE type,WORD svalue)
{
	SendFrameHead(INNER_YC_ZERO);
	SendBuf.pBuf[SendBuf.wWritePtr++] = type;
	SendWordLH(svalue);
	SendFrameTail();
	return(MAINT_Linux_To_BM(INNER_ACK_OK));
}

int yk_BM(int type,int srcid,int id,int value,int time)
{
	struct VInnerFrame* pRec;
	BYTE *pData;
	int ret = 0;
	
	SendFrameHead(INNER_YK_BM);
	SendDWordLH(type);
	SendDWordLH(srcid);
	SendDWordLH(id);
	SendDWordLH(value);
	SendDWordLH(time);
	SendFrameTail();
	if(MAINT_Linux_To_BM(INNER_YK_BM) == ERROR)
		return 1;
	pRec = (struct VInnerFrame *)RecFrame.pBuf;
	pData = (BYTE*)(pRec + 1);
	ret= (int)MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	return ret;
}

int ReadPrPara(WORD parano,char *pbuf,WORD type)
{
	struct VInnerFrame* pRec;
	BYTE *pData;
	int ret = 0;
	WORD len;
	
	SendFrameHead(INNER_READ_PRPARA);
	SendWordLH(parano);
	SendWordLH(type);
	SendFrameTail();
	if(MAINT_Linux_To_BM(INNER_READ_PRPARA) == ERROR)
		return ERROR;

	pRec = (struct VInnerFrame *)RecFrame.pBuf;
	pData = (BYTE*)(pRec + 1);
	ret= (int)MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	len  = MAKEWORD(pData[4], pData[5]);
	memcpy(pbuf,pData+6,len);
	return ret;
}

int BMWritePrPara(WORD parano,char *pbuf,WORD type, WORD len)
{
	struct VInnerFrame* pRec;
	BYTE *pData;
	int ret = 0;
	
	SendFrameHead(INNER_WRITE_PRPARA);
	SendWordLH(parano);
	SendWordLH(type);
	SendWordLH(len);
	memcpy(SendBuf.pBuf + SendBuf.wWritePtr, pbuf,len);
	SendBuf.wWritePtr = SendBuf.wWritePtr + len;
	SendFrameTail();
	if(MAINT_Linux_To_BM(INNER_WRITE_PRPARA) == ERROR)
		return ERROR;

	pRec = (struct VInnerFrame *)RecFrame.pBuf;
	pData = (BYTE*)(pRec + 1);
	ret= (int)MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	return ret;
}

int WritePrParaFile()
{
	struct VInnerFrame* pRec;
	BYTE *pData;
	int ret = 0;
	
	SendFrameHead(INNER_WRITE_PRPARA_FILE);
	SendFrameTail();
	
	if(MAINT_Linux_To_BM(INNER_WRITE_PRPARA_FILE) == ERROR)
		return ERROR;

	pRec = (struct VInnerFrame *)RecFrame.pBuf;
	pData = (BYTE*)(pRec + 1);
	ret= (int)MAKEDWORD(MAKEWORD(pData[0], pData[1]), MAKEWORD(pData[2], pData[3]));
	return ret;
}

void BMResetProtect(int fd)
{	
	SendFrameHead(INNER_RESET_PROTECT);
	SendDWordLH(fd);
	SendFrameTail();
	if(MAINT_Linux_To_BM(INNER_RESET_PROTECT) == ERROR)
		return;
}

void BMSetYkYb(DWORD Yb)
{
	SendFrameHead(INNER_YK_YB);
	SendDWordLH(Yb);
	SendFrameTail();
	SendBMFrame();
}

void BMSenderrCode(DWORD dwErrCode)
{
	SendFrameHead(INNER_ERR_CODE);
	SendDWordLH(dwErrCode);
	SendFrameTail();
	SendBMFrame();
}

void ykTypeSave(int type, int id, int val)
{	
	SendFrameHead(INNER_YK_TYPE);
	SendDWordLH(type);
	SendDWordLH(id);
	SendDWordLH(id);
	SendDWordLH(val);
	SendFrameTail();
	if(MAINT_Linux_To_BM(INNER_YK_TYPE) == ERROR)
		return;
}

//设置过载过载及过电压低电压
void BmWriteRunPara(BYTE *buf,WORD len)
{
	SendFrameHead(INNER_WRITE_RUN);
	SendWordLH(len);
	memcpy(SendBuf.pBuf + SendBuf.wWritePtr, buf,len);
	SendBuf.wWritePtr = SendBuf.wWritePtr + len;
	SendFrameTail();
	if(MAINT_Linux_To_BM(INNER_WRITE_RUN) == ERROR)
		return;
}

int Maint_BM(WORD dwCode,BYTE *buf,WORD len)
{
	SendFrameHead(dwCode);
	memcpy(SendBuf.pBuf + SendBuf.wWritePtr, buf,len);
	SendBuf.wWritePtr = SendBuf.wWritePtr + len;
	SendFrameTail();
	return(MAINT_Linux_To_BM(INNER_ACK_OK));
}

int Maint_BM_RCV(WORD dwCode,BYTE *sendbuf,WORD len,BYTE* recvbuf,WORD* recvlen)
{
	struct VInnerFrame* pRec;
	BYTE *pData;
	
	SendFrameHead(dwCode);
	memcpy(SendBuf.pBuf + SendBuf.wWritePtr, sendbuf,len);
	SendBuf.wWritePtr = SendBuf.wWritePtr + len;
	SendFrameTail();
	if(MAINT_Linux_To_BM(dwCode) == ERROR)
		return ERROR;
	pRec = (struct VInnerFrame *)RecFrame.pBuf;
	pData = (BYTE*)(pRec + 1);
	*recvlen = MAKEWORD(pRec->Len1Low, pRec->Len1High) -3;
	if(*recvlen > RecBuf.wBufSize)
		return ERROR;
	memcpy(recvbuf,pData,*recvlen);
	
	return OK;
}


