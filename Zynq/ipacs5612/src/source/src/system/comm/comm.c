/*------------------------------------------------------------------------
 Module:		comm.c
 Author:		solar
 Project:		
 Creation Date: 2008-08-04
 Description:	total comm 20, serial 4, net 16                
------------------------------------------------------------------------*/

#include "comm.h"
#include "sys.h"

int COMM_NET_NUM;
VCommChan *g_CommChan;

extern void PortInfoInit(void);
static void commScanOnTimer(void);

#ifdef INCLUDE_COMM_SHOW
static struct VCommShow gCommShow;
static void commShowInit(void);
void commPrint(int no, BYTE flag, int len, BYTE *buf);
void commBufFill(int no, BYTE flag, int len, BYTE *buf);
#endif


#ifdef INCLUDE_COMM_FILE
static struct VCommFile gCommFile;
static void commFileInit(void);
void commFileWrite(int no, BYTE flag, int len,const BYTE *buf);
static BYTE writelinebuffer[COMM_WRITELINE_LEN+2];
#endif


void comm(void)
{	
	DWORD events;
	DWORD evmask;

	tmEvEvery(COMM_ID, COMM_SCAN_TM, EV_TM1);
	evmask = EV_TM1;
	
	for (;;)
	{
		evReceive(COMM_ID, evmask, &events);

	    if(events & EV_TM1) 
		{
			commScanOnTimer();
	    }
	}	
}
	
int commInit (void)
{	
    int no;

	COMM_NET_NUM = COMM_NUM-COMM_SERIAL_NUM;

	g_CommChan = (VCommChan *)malloc(sizeof(VCommChan)*COMM_NUM);
    if (g_CommChan == NULL) return ERROR;
	
    memset(g_CommChan, 0 , sizeof(VCommChan)*COMM_NUM);

    PortInfoInit();
	
#ifdef INCLUDE_SERIAL
	SerialInit();
#endif

#ifdef INCLUDE_NET
	TcpInit();
#endif

    for (no=COMM_START_NO; no<(COMM_START_NO+COMM_NUM); no++)
	{
		if (g_Task[no].CommCfg.Port.id)
		{
		   commCtrl(no, (WORD)(CCC_CFG_STR|CFG_STR_SET), (BYTE *)(g_Task[no].CommCfg.Port.cfgstr));
		}
	}

#ifdef INCLUDE_COMM_SHOW
	commShowInit();
#endif
#ifdef INCLUDE_COMM_FILE
	commFileInit();
#endif


	return OK;
}

int commRead (int no, BYTE* pbuf, int buflen, DWORD flags)
{
	int ret = ERROR;

    no = no-COMM_START_NO;
	if (no < 0) return ERROR;
	
    if (g_CommChan[no].read != NULL)
    {
        ret = g_CommChan[no].read(no, pbuf, buflen, flags);		
    }

#ifdef INCLUDE_COMM_SHOW
	if (ret>0) 
	{
		commBufFill(no, 0, ret, pbuf);
		commPrint(no, 0, ret, pbuf);
	}	
#endif

#ifdef INCLUDE_COMM_FILE
	if(ret > 0)
	{
		commFileWrite(no,0,ret,pbuf);
	}
#endif

	return(ret);
}	

int commWrite(int no, BYTE* pbuf, int buflen, DWORD flags)
{
	int ret = ERROR;

    no = no-COMM_START_NO;
	if (no < 0) return ERROR;
    if (g_CommChan[no].write != NULL)
    {		
        ret = g_CommChan[no].write(no,pbuf, buflen, flags);
	}
#ifdef INCLUDE_COMM_SHOW
	if (ret>0) 
	{
		commBufFill(no, 1, ret, pbuf);
		commPrint(no, 1, ret, pbuf);
	}	
#endif	

#ifdef INCLUDE_COMM_FILE
	if(ret > 0)
	{
		commFileWrite(no,1,ret,pbuf);
	}
#endif

	return(ret);
}

/*------------------------------------------------------------------------
 Procedure:     CommCtrl ID:1
 Purpose:       控制通讯口
 Input:         no: 通讯口号; flags: 标志(参考具体接口)			
 Output:		0:OK; -1:ERROR.
 Errors:
------------------------------------------------------------------------*/
int commCtrl (int no, WORD command, BYTE *para )
{
	int ret = ERROR;
	int toNo, tid;	
	DWORD comm_idle_limit, tx_idle_limit;	
	//DWORD ev_send;
	BYTE  comm_idle_ev, tx_idle_ev;		

    no = no-COMM_START_NO;
	if (no < 0) return ERROR;

	switch(command & 0xFF00)
	{
		case CCC_EVENT_CTRL:			
			if(command&TX_IDLE_ON){
				g_CommChan[no].tx_idle = 0;
				g_CommChan[no].tx_idle_limit = ((*para)+COMM_SCAN_TM-1)/COMM_SCAN_TM;
				//g_CommChan[no].ev_send &= ~EV_TX_IDLE;
				g_CommChan[no].tx_idle_ev = 1;
				ret = OK;
			}
			if(command&TX_IDLE_OFF){
				g_CommChan[no].tx_idle_ev = 0;
				ret = OK;
			}
			if(command&COMM_IDLE_ON){
				g_CommChan[no].rx_idle = 0;
				g_CommChan[no].tx_idle = 0;
				g_CommChan[no].comm_idle_limit = ((*para)+COMM_SCAN_TM-1)/COMM_SCAN_TM; 		
				//g_CommChan[no].ev_send &= (~EV_COMM_IDLE);
				g_CommChan[no].comm_idle_ev = 1;			
				ret = OK;
			}
			if(command&COMM_IDLE_OFF){
				g_CommChan[no].comm_idle_ev = 0;
				ret = OK;
			}
			break;
		case CCC_TID_CTRL:			
			toNo = (int)(*para);
			toNo = toNo-COMM_START_NO;
			if ((toNo >= 0) && (no != toNo))
			{
				ThreadCommNoSet(g_CommChan[toNo].tid, no+COMM_START_NO);
				ThreadCommNoSet(g_CommChan[no].tid, toNo+COMM_START_NO);
				comm_idle_ev =  g_CommChan[toNo].comm_idle_ev;
				comm_idle_limit =  g_CommChan[toNo].comm_idle_limit;
				tx_idle_ev =  g_CommChan[toNo].tx_idle_ev;
				tx_idle_limit =  g_CommChan[toNo].tx_idle_limit;
				//ev_send =  g_CommChan[toNo].ev_send;
				tid = g_CommChan[toNo].tid;
				g_CommChan[toNo].comm_idle_ev = g_CommChan[no].comm_idle_ev;		
				g_CommChan[toNo].comm_idle_limit = g_CommChan[no].comm_idle_limit;		
				g_CommChan[toNo].tx_idle_ev = g_CommChan[no].tx_idle_ev;		
				g_CommChan[toNo].tx_idle_limit = g_CommChan[no].tx_idle_limit;		
				//g_CommChan[toNo].ev_send = g_CommChan[no].ev_send;		
				g_CommChan[toNo].tid = g_CommChan[no].tid;
				g_CommChan[toNo].tid_watchdog = 0;
				g_CommChan[no].comm_idle_ev = comm_idle_ev;
				g_CommChan[no].comm_idle_limit = comm_idle_limit;
				g_CommChan[no].tx_idle_ev = tx_idle_ev;
				g_CommChan[no].tx_idle_limit = tx_idle_limit;
				//g_CommChan[no].ev_send = ev_send;
				g_CommChan[no].tid = tid;
				g_CommChan[no].tid_watchdog = 0;
				evSend(g_CommChan[toNo].tid, EV_COMM_STATUS);
				evSend(g_CommChan[no].tid, EV_COMM_STATUS);
				ret = OK;	
			}	
			break;
		case CCC_CFG_STR:	
			if (command & CFG_STR_SET)
			{
                if (memcmp((char*)g_CommChan[no].cfgstr, para, 90) != 0)
				{
					if(g_CommChan[no].ctrl == NULL ) return ERROR;
					ret = g_CommChan[no].ctrl(no, command, para);
					if (ret) memcpy(g_CommChan[no].cfgstr, para, 90);
				}
				else ret = OK;
			}	
			if (command & CFG_STR_GET)
			{
				memcpy(para, g_CommChan[no].cfgstr, 90);
				ret = OK;
			}				
			break;
		default:
			if(g_CommChan[no].ctrl == NULL ) return ERROR;
			ret = g_CommChan[no].ctrl(no, command, para);
			break;
	}	
			
	return ret;
}

int commRegister(int no, int phy_no, WORD bufsize,const void *comread,const void *comwrite,const void *comctrl)
{	
	if ((no < 0) || (no >= COMM_NUM)) return ERROR;
	if (g_CommChan[no].tid)  return ERROR;
	if ((no >= 0) && (g_Task[no+COMM_START_NO].CommCfg.Port.id == 0)) return ERROR;
	g_CommChan[no].rx_buf.buf = (BYTE *)calloc(bufsize, 1);
	if(g_CommChan[no].rx_buf.buf == NULL) return ERROR;
	g_CommChan[no].rx_buf.size = bufsize;
	memset(g_CommChan[no].rx_buf.buf, 0, bufsize);

	g_CommChan[no].tx_buf.buf = (BYTE *)calloc(bufsize, 1);
	if(g_CommChan[no].tx_buf.buf == NULL) return ERROR;
	g_CommChan[no].tx_buf.size = bufsize;
	memset(g_CommChan[no].tx_buf.buf, 0, bufsize);
	
	g_CommChan[no].read = comread;
	g_CommChan[no].write = comwrite;
	g_CommChan[no].ctrl = comctrl;
	g_CommChan[no].phy_no = phy_no;

	g_CommChan[no].tid = no+COMM_START_NO;
	g_CommChan[no].tid_watchdog = 0;
		
	return no;
}

static void commScanChan(int no)
{
	DWORD countw;
	VCommBuf *pfifo; VCommChan *pChan;
	int mytid;

	mytid = no+COMM_START_NO;
	if (GetThActive(mytid) == 0) return;

	pChan = &g_CommChan[no];	

	pfifo = &(pChan->tx_buf);
	countw=((pfifo->wp - pfifo->rp) + pfifo->size)&(pfifo->size-1);		


	if (pChan->rx_wp != pChan->rx_buf.wp )
	{
		pChan->rx_wp = pChan->rx_buf.wp;
		pChan->rx_idle = 0;
	}		
	else	
		pChan->rx_idle++;

	/*空闲事项发送*/
	if (countw == 0) 
		pChan->tx_idle++;
	else 
	{
		pChan->tx_idle = 0;
	
	}

	/*COMM_ILDE事件反复触发*/
	if (pChan->comm_idle_ev && pChan->tx_idle>=pChan->comm_idle_limit &&
		pChan->rx_idle>=pChan->comm_idle_limit )
	{
		evSend(pChan->tid, EV_COMM_IDLE);
	}	

	/*TX_ILDE事件反复触发*/
	if (pChan->tx_idle_ev && pChan->tx_idle>=pChan->tx_idle_limit)
	{
		evSend(pChan->tid, EV_TX_IDLE);
	}	

	//tid watchdog
	if ((pChan->tid != mytid) && (mytid != MAINT_ID) && pChan->tx_idle && pChan->rx_idle)
	{
		pChan->tid_watchdog++;
		if (pChan->tid_watchdog >= (10*60*1000/COMM_SCAN_TM))  //10min
			commCtrl(pChan->tid, CCC_TID_CTRL, (BYTE *)&mytid);
	}
}

/*------------------------------------------------------------------------
 Procedure:     CommScanOnTimer ID:1
 Purpose:       定时扫描各通信口, 向上层任务发送事件和消息
 Input:          				
 Output:		
 Errors:
------------------------------------------------------------------------*/
static void commScanOnTimer(void)
{
    int i;

#ifdef INCLUDE_NET
	ScanSocket();
#endif

#ifdef INCLUDE_SERIAL
	ScanSerial();
#endif
	

	for (i=0; i<COMM_NUM; i++)	
		commScanChan(i);

#ifdef INCLUDE_COMM_SHOW	
    if (gCommShow.dwBufShow) gCommShow.dwBufShow--;
    if (gCommShow.dwCount) gCommShow.dwCount--;
#endif	
}
BYTE hexCharToBYTE(const char ch)
{	
	BYTE result = 0;	
	if(ch >= '0' && ch <= '9')	
	{		
		result = (BYTE)(ch - '0');	
	}	
	else if(ch >= 'a' && ch <= 'z')
	{		
		result = (BYTE)(ch - 'a') + 10;	
	}	
	else if(ch >= 'A' && ch <= 'Z')	
	{		
		result = (BYTE)(ch - 'A') + 10;	
	}	
	else	
	{		
		result = 0;
	}		
	return result;	
}

#ifdef INCLUDE_COMM_FILE
static void commFileInit(void)
{
	int no;
	
	memset(&gCommFile, 0, sizeof(gCommFile));
	gCommFile.dwSem = smMCreate();
	gCommFile.pCommFileLine = (struct VCommFileLine *)malloc(sizeof(struct VCommFileLine)*COMM_NUM);

	memset(gCommFile.pCommFileLine,0,sizeof(struct VCommFileLine)*COMM_NUM);
	//从文件中读取指针，只读一次，每次写每次把指针写入
	for (no=COMM_START_NO; no<(COMM_START_NO+COMM_NUM); no++)
	{
		if (g_Task[no].CommCfg.Port.id)
		{
			if(no < (COMM_START_NO+COMM_SERIAL_NUM))
				sprintf(gCommFile.pCommFileLine[no-COMM_START_NO].filename,"%s/dat/uart-%d.txt",SYS_PATH_ROOT,no-COMM_START_NO);
			else if(no < (COMM_START_NO + COMM_SERIAL_NUM + COMM_NET_SYS_NUM))
				sprintf(gCommFile.pCommFileLine[no-COMM_START_NO].filename,"%s/dat/weihu%d.txt",SYS_PATH_ROOT,(no-COMM_START_NO)-COMM_SERIAL_NUM);
			else
				sprintf(gCommFile.pCommFileLine[no-COMM_START_NO].filename,"%s/dat/net-%d.txt",SYS_PATH_ROOT,no-(COMM_START_NO+COMM_SERIAL_NUM+COMM_NET_SYS_NUM));

			gCommFile.pCommFileLine[no-COMM_START_NO].dwSize = COMM_WRITEFILE_LINE;
		}
	}
	
}

void commFileWrite(int no, BYTE flag, int len,const BYTE *buf)
{
	int i,j,line;
	DWORD len1;
    struct VSysClock SysTime;
	char linenum[10];
	FILE *fp;
	
    if ((len < 1) || (no == COMM_SERIAL_NUM)) //维护报文不用存
        return;
    smMTake(gCommFile.dwSem);	
    GetSysClock((void *)&SysTime, SYSCLOCK);	
	memset(gCommFile.commbuf,0,COMM_WRITEFILE_LEN);
    //长度为3的倍数，保证对齐
    if(flag == 0)
		sprintf((char*)gCommFile.commbuf,"%02d/%02d-%02d:%02d:%02d.%03d recv(%04d):",SysTime.byMonth,SysTime.byDay,SysTime.byHour,SysTime.byMinute,SysTime.bySecond,SysTime.wMSecond,len);
	if(flag == 1)
		sprintf((char*)gCommFile.commbuf,"%02d/%02d-%02d:%02d:%02d.%03d send(%04d):",SysTime.byMonth,SysTime.byDay,SysTime.byHour,SysTime.byMinute,SysTime.bySecond,SysTime.wMSecond,len);
	if(flag == 3)
		sprintf((char*)gCommFile.commbuf,"%02d/%02d-%02d:%02d:%02d.%03d recven(%4d): ",SysTime.byMonth,SysTime.byDay,SysTime.byHour,SysTime.byMinute,SysTime.bySecond,SysTime.wMSecond,len);
	if(flag == 4)
		sprintf((char*)gCommFile.commbuf,"%02d/%02d-%02d:%02d:%02d.%03d senden(%4d): ",SysTime.byMonth,SysTime.byDay,SysTime.byHour,SysTime.byMinute,SysTime.bySecond,SysTime.wMSecond,len);
	
	j = (int)strlen((char*)gCommFile.commbuf);
    for (i=0;i<len;i++)
    {
       sprintf((char*)(gCommFile.commbuf+j)," %02X",buf[i]);
	   j = (int)strlen((char*)gCommFile.commbuf);
    }
	//打开文件
	fp = fopen(gCommFile.pCommFileLine[no].filename, "r+");
	if (fp == NULL)
	{
		fp = fopen(gCommFile.pCommFileLine[no].filename, "w"); //创建文件，并写入行数
		if(fp==NULL)
		{
			smMGive(gCommFile.dwSem);
			return;
		}
		gCommFile.pCommFileLine[no].wReadPtr = 0;
		sprintf(linenum,"%08d\r\n",(int)gCommFile.pCommFileLine[no].wReadPtr);
		fwrite(linenum, 1, strlen(linenum), fp);
	}
	else //获取行数
	{
		memset(linenum,0,sizeof(linenum));
		len1 = fread(linenum, 1, 8, fp);
		if(len1 == 0)
			gCommFile.pCommFileLine[no].wReadPtr = 0;
		else
			gCommFile.pCommFileLine[no].wReadPtr = (DWORD)atoi(linenum);	
		//printf("读出行数 %s %d\n",linenum,gCommFile.pCommFileLine[no].wReadPtr);
	}
	
	writelinebuffer[COMM_WRITELINE_LEN] = 0x0D;
	writelinebuffer[COMM_WRITELINE_LEN + 1] = 0x0A; //换行
	line = (j+COMM_WRITELINE_LEN-1)/COMM_WRITELINE_LEN;

	for(i = 0;i < line;i++)
	{
		memset(writelinebuffer,0x20,COMM_WRITELINE_LEN);
		
		if((j - i*COMM_WRITELINE_LEN) < COMM_WRITELINE_LEN)
			memcpy(writelinebuffer,gCommFile.commbuf + i*COMM_WRITELINE_LEN,(DWORD)(j - i*COMM_WRITELINE_LEN));
		else
			memcpy(writelinebuffer,gCommFile.commbuf + i*COMM_WRITELINE_LEN,COMM_WRITELINE_LEN);

		fseek(fp, (long)(gCommFile.pCommFileLine[no].wReadPtr*(COMM_WRITELINE_LEN+2)+10), SEEK_SET);
		fwrite(writelinebuffer, 1, COMM_WRITELINE_LEN+2, fp);
		gCommFile.pCommFileLine[no].wReadPtr = (gCommFile.pCommFileLine[no].wReadPtr + 1) & (gCommFile.pCommFileLine[no].dwSize-1);
	}
	//关闭文件
	fseek(fp, 0, SEEK_SET);
	sprintf(linenum,"%08d\r\n",(int)gCommFile.pCommFileLine[no].wReadPtr);
	fwrite(linenum, 1, strlen(linenum), fp);
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
    smMGive(gCommFile.dwSem);
}  
#endif

#ifdef INCLUDE_COMM_SHOW
static void commShowInit(void)
{
	memset(&gCommShow, 0, sizeof(gCommShow));
    gCommShow.nCommNo = -1;
	gCommShow.nMaintCommNo = -1;
	gCommShow.dwSem = smMCreate();
}

 void commPrint(int no, BYTE flag, int len, BYTE *buf)
{
	int i,j;
	BYTE *p;
	//BOOL isKey=FALSE;

	if (len==0) return;

	if (gCommShow.dwBufShow && (no==gCommShow.nCommNo))
	{     
		if (flag==0)
		{
			DPRINT("\nComm%d rece:\n\n", no);
		}  
		else  if(flag == 1)
		{
			DPRINT("\nComm%d send:\n\n", no);
		}
		else if (flag==3)
		{
			DPRINT("\nComm%d jiami rece:\n\n", no);
		}  
		else  if(flag == 4)
		{
			DPRINT("\nComm%d jiami send:\n\n", no);
		}			

		p=(BYTE *)buf;
		j=0;
		for (i=0;i<len;i++)
		{
			DPRINT("%02X ",*p);

			j++;
			if ((j%20==0)&&(i!=len-1)) DPRINT("\n");

			p++;  
		}
		DPRINT("\n");
	}
}
//flag:0-接收、1-发送、2-提示信息 hex显示
//扩充3-加密接收  4-加密发送 cjl 2017年10月27日08:54:54
void commBufFill(int no, BYTE flag, int len, BYTE *buf)
{
    int i;
    BYTE *p;
    BYTE byFlag;
    BYTE curFlag;
    struct VSysClock SysTime;

    if (!gCommShow.dwCount || (no != gCommShow.nMaintCommNo)) 
        return;

    if (len < 1) 
        return;
    smMTake(gCommShow.dwSem);	
    curFlag = flag;
    GetSysClock((void *)&SysTime, SYSCLOCK);	

    //标志
    if(0 == curFlag)
    {//接收
        byFlag = 0xFF;
    }
    else if(1 == curFlag)
    {
        byFlag = 0xEE;
    }
    else if(2 == curFlag)
    {
        byFlag = 0xDD;
    }
	else if(3 == curFlag)
	{
		  byFlag = 0xCC;
	}
	else if(4 == curFlag)
	{
			byFlag = 0xBB;
	}
		
    for (i=0;i<4;i++)
    {
        gCommShow.abyBuf[gCommShow.wWrtPrt] = byFlag;
        gCommShow.wWrtPrt =(gCommShow.wWrtPrt+1)&(COMM_DATABUF_LEN-1);
    }
    //长度
    len += 5;
    gCommShow.abyBuf[gCommShow.wWrtPrt] = len&0XFF;
    gCommShow.wWrtPrt =(gCommShow.wWrtPrt+1)&(COMM_DATABUF_LEN-1);
    gCommShow.abyBuf[gCommShow.wWrtPrt] = ( len>>8)&0XFF;
    gCommShow.wWrtPrt =(gCommShow.wWrtPrt+1)&(COMM_DATABUF_LEN-1);
    //时间
    gCommShow.abyBuf[gCommShow.wWrtPrt] = SysTime.byHour;
    gCommShow.wWrtPrt =(gCommShow.wWrtPrt+1)&(COMM_DATABUF_LEN-1);
    gCommShow.abyBuf[gCommShow.wWrtPrt] = SysTime.byMinute;
    gCommShow.wWrtPrt =(gCommShow.wWrtPrt+1)&(COMM_DATABUF_LEN-1);
    gCommShow.abyBuf[gCommShow.wWrtPrt] = SysTime.bySecond;
    gCommShow.wWrtPrt =(gCommShow.wWrtPrt+1)&(COMM_DATABUF_LEN-1);
    gCommShow.abyBuf[gCommShow.wWrtPrt] = SysTime.wMSecond&0xFF;
    gCommShow.wWrtPrt =(gCommShow.wWrtPrt+1)&(COMM_DATABUF_LEN-1);
    gCommShow.abyBuf[gCommShow.wWrtPrt] = (SysTime.wMSecond>>8)&0xFF;
    gCommShow.wWrtPrt =(gCommShow.wWrtPrt+1)&(COMM_DATABUF_LEN-1);
    len -= 5;
    
    //数据
    p=(BYTE *)buf;
    for (i=0;i<len;i++)
    {
        gCommShow.abyBuf[gCommShow.wWrtPrt] = *p++;
        gCommShow.wWrtPrt =(gCommShow.wWrtPrt+1)&(COMM_DATABUF_LEN-1);
    }
    smMGive(gCommShow.dwSem);
}  


/*------------------------------------------------------------------------
 Procedure:    comshow ID:1
 Purpose:      缓冲区显示
 Input:        no:串口号,lastMin 持续时间(分) 
 Output:		
 Errors:
------------------------------------------------------------------------*/
int comshow(int no, int min)
{
    int ok;

	if((no < 0) || (no >= COMM_SERIAL_NUM))
    	ok = FALSE;
	else 
    	ok = TRUE;

    if (ok == FALSE)
    	shellprintf("comshow 串口号,显示时间(分)\n");
    else
    { 
		gCommShow.nCommNo = no;
		gCommShow.dwBufShow = min*60*100/COMM_SCAN_TM;
    }  
    
    return(OK);
}

/*------------------------------------------------------------------------
 Procedure:    netshow ID:1
 Purpose:      缓冲区显示
 Input:        no:网络连接号,和维护软件对应,lastMin 持续时间(分) 
 Output:		
 Errors:
------------------------------------------------------------------------*/
int netshow(int no, int min)
{
    int ok;

	if((no <= 0) || (no >= COMM_NET_NUM-COMM_NET_SYS_NUM))
    	ok = FALSE;
	else 
    	ok = TRUE;
	
    if (ok == FALSE)
    	shellprintf("netshow 网络连接号,显示时间(分)\n");
    else
    { 
		no += (COMM_SERIAL_NUM+COMM_NET_SYS_NUM-1);
		gCommShow.nCommNo = no;
		gCommShow.dwBufShow = min*60*100/COMM_SCAN_TM;
    }  
    
    return(OK);
}

/*------------------------------------------------------------------------
 Procedure:    commshow ID:1
 Purpose:      缓冲区显示
 Input:        commNo:通信口号，lastMin 持续时间(分) 
 Output:		
 Errors:
------------------------------------------------------------------------*/
int commshow(int no, int min)
{
    int ok;

	if((no < 0) || (no >= COMM_NUM))
    	ok = FALSE;
	else 
    	ok = TRUE;

    if (ok == FALSE)
    	shellprintf("commshow 端口号,显示时间(分)\n");
    else
    { 
		gCommShow.nCommNo = no;
		gCommShow.dwBufShow = min*60*100/COMM_SCAN_TM;
    }  
    
    return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     bufshowcancle ID:1
 Purpose:      取消缓冲区显示
 Input:        
 Output:		
 Errors:
------------------------------------------------------------------------*/
int commshowcancle(void)
{
    gCommShow.dwBufShow=0;
    return(OK);
}

int commMShowReq(int no)
{	
	no = GetAppPortId(no);
	no = no-COMM_START_NO;
	if ((no<0) || (no>=COMM_NUM)) return ERROR;
	
	gCommShow.nMaintCommNo = no;
	gCommShow.wWrtPrt = gCommShow.wReadPtr = 0;
	gCommShow.dwCount = 1*60*100/COMM_SCAN_TM;
	return OK;
}

/*------------------------------------------------------------------------
 Procedure:     commBufQuery ID:1
 Purpose:       察看数据缓冲区
 Input:
 Output:        返回数据字串
 Errors:
------------------------------------------------------------------------*/    
int commBufQuery(int no, int len,char *buf)
{
	int i, num;
	char *p;
  	no = GetAppPortId(no);
	no = no-COMM_START_NO;

	if ((no<0) || (no>=COMM_NUM)) return 0;
	if (no != gCommShow.nMaintCommNo) return 0;

	gCommShow.dwCount = 1*60*100/COMM_SCAN_TM;

	smMTake(gCommShow.dwSem);  	
	num = (gCommShow.wWrtPrt >= gCommShow.wReadPtr) ? (gCommShow.wWrtPrt-gCommShow.wReadPtr) : (COMM_DATABUF_LEN-gCommShow.wReadPtr+gCommShow.wWrtPrt);
	smMGive(gCommShow.dwSem); 

	if (num > len) num = len;

    p = buf;
	for (i=0; i<num; i++)
	{
		*p = gCommShow.abyBuf[gCommShow.wReadPtr];		
		gCommShow.wReadPtr =(gCommShow.wReadPtr+1)&(COMM_DATABUF_LEN-1);
		p++;
	}  
	return(num);
}  

int commMShowCancle(void)
{
    gCommShow.dwCount=0;

	return OK;
}

#else

int commMShowReq(int no)
{
	return ERROR;
}

int commBufQuery(int no, int len, const char *buf)
{
	return 0;
}

int commMShowCancle(void)
{
	return ERROR;
}
#endif

