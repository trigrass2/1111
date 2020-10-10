/*------------------------------------------------------------------------
 Module:       	sys.cpp
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-09-15
 Description:   
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys.h"
#include "bsp.h"
#include "protocol.h"
#ifdef INCLUDE_HIS_TTU
#include "Dbhis_ttu.h"
#endif


static BYTE ParaFileBuf[MAXFILELEN];
struct VFileHead *g_pParafile;
DWORD g_ParafileSem;
DWORD g_fileSem;
struct VAssert *g_assert=NULL;    //给rtos使用，保存复位前出错位置
char g_assert_buf[128];

struct VSysInfo g_Sys;  
struct VTask *g_Task; 

struct VProtocol g_PriProtocol[MAXPROTOCOLNUM];  
struct VProtocol g_SecProtocol[MAXPROTOCOLNUM];

BYTE g_dtu_lcdenable=0;  // 液晶面板使能变量
BYTE g_dtu_comm = COMM_SERIAL_START_NO;


int fault(void)
{
	if (g_Sys.dwErrCode == 0) 
	{
	    printf("无故障!\n");
		return 0;
	}	

	if (g_Sys.dwErrCode & SYS_ERR_SYS) printf("致命故障!\n");
	if (g_Sys.dwErrCode & SYS_ERR_GAIN)  printf("交采标定系数非法!\n");
	if (g_Sys.dwErrCode & SYS_ERR_AD)   printf("AD异常!\n");
	if (g_Sys.dwErrCode & SYS_ERR_SLT)   printf("SLT告警!\n");
	if (g_Sys.dwErrCode & SYS_ERR_PR)   printf("保护告警!\n");
	if (g_Sys.dwErrCode & SYS_ERR_FILE)  printf("文件系统告警!\n");
	if (g_Sys.dwErrCode & SYS_ERR_ADDR)  printf("主板插槽错误或程序不匹配!\n");
	if (g_Sys.dwErrCode & SYS_ERR_CFG) printf("参数错误!\n");
	if (g_Sys.dwErrCode & SYS_ERR_OBJ)   printf("动态库错误!\n");
	if (g_Sys.dwErrCode & SYS_ERR_COMM)  printf("分板通信中断!\n");
	if (g_Sys.dwErrCode & SYS_ERR_SUBCFG) printf("分板型号与参数冲突!\n");

    if (g_Sys.dwErrCode & SYS_ERR_JIAMI)  printf("加密芯片没焊或芯片异常!\n");
	return 0;
}

/*------------------------------------------------------------------------
 Procedure:     PublicInit ID:1
 Purpose:       
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void PublicInit(void)
{   
    int i;

	g_ParafileSem = smMCreate();	
	g_pParafile = (struct VFileHead *)ParaFileBuf;	 

	g_Task = (struct VTask *)malloc(sizeof(struct VTask)*THREAD_MAX_NUM);
	if (g_Task == NULL) MyFatalErr();

	memset(g_Task, 0, sizeof(struct VTask)*THREAD_MAX_NUM);   
	memset(&g_Sys, 0, sizeof(g_Sys));

    for (i=0; i<MAX_LD_FILE_NUM; i++)
	{
		strcpy(g_Sys.LdFileTbl[i].ver, "N/A");
		g_Sys.LdFileTbl[i].crc = 0xFFFFFFFF;
    }	


	strcpy(g_Sys.LdFileTbl[0].name, "主程序");
#if (TYPE_OS == OS_LINUX)
	strcpy(g_Sys.LdFileTbl[0].filename, "Linux");
#endif
	strcpy(g_Sys.LdFileTbl[0].user, sysUser);
	strcpy(g_Sys.LdFileTbl[0].ver, sysSVer);
	g_Sys.LdFileTbl[0].crc = sysCrc;
	g_Sys.LdFileTbl[0].load = 1;
	g_Sys.LdFileNum++;	
	#if (DEV_SP == DEV_SP_WDG)
	if(VER_MODE == 0xA8)
	{
		g_wdg_ver = 1;
	}
	else if(VER_MODE == 0xAA)
	{
		g_wdg_ver = 2;
	}
	else
	{//old wdg
		g_wdg_ver = 0;
	}
	#endif	
}    

void MemoryMalloc(void **ppPtr, DWORD dwSize, BYTE byMode)
{
    if (dwSize == 0)
    {
       *ppPtr = 0;
	   return;
    }
	if ((*ppPtr = calloc(dwSize, sizeof(BYTE))) == NULL)
		MyFatalErr();
	
	return;
}   

int memIs0(BYTE *buf, int len)
{
    int i;
	BYTE *p = buf;
	
	for (i=0; i<len; i++)
		if (*(p + i) != 0) return FALSE;

	return TRUE;	
}

/*------------------------------------------------------------------------
 Procedure:     NVRamInit ID:1
 Purpose:       NVRam初始化
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/	
void NVRamInit(void)
{
	MemoryMalloc((void **)&g_Sys.pdwRestart,sizeof(DWORD),NVRAM);
}

/*------------------------------------------------------------------------
 Procedure:     PoolMalloc ID:1
 Purpose:       事件池分配
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void PoolMalloc(struct VPool *pPool,const WORD RelEleNum,const WORD PoolNum,const WORD CellLen)
{  
	if (RelEleNum)    
	{
		if (pPool->wNum!=PoolNum)
		{
			pPool->wWritePtr=0;
			pPool->wNum=PoolNum;
			pPool->wCellLen=CellLen;
			*g_Sys.pdwRestart=COLDRESTART;    
		} 

		if (pPool->wCellLen!=CellLen)
		{
			pPool->wWritePtr=0;
			pPool->wNum=PoolNum;
			pPool->wCellLen=CellLen;
			*g_Sys.pdwRestart=COLDRESTART;    
		} 
	}       
	else
	{
		if ((pPool->wWritePtr!=0)||(pPool->wNum!=0))
		{
			pPool->wWritePtr=0;
			pPool->wNum=0;    			
			pPool->wCellLen=0;
			*g_Sys.pdwRestart=COLDRESTART;    
		} 
	}     

	if (pPool->wWritePtr>pPool->wNum)
	{
		pPool->wWritePtr=0;
		*g_Sys.pdwRestart=COLDRESTART;    
	}  
	else  if (pPool->wWritePtr==pPool->wNum)
	{
		if (pPool->wWritePtr);
		{
			pPool->wWritePtr=0;
			*g_Sys.pdwRestart=COLDRESTART;    
		}   
	}  

	MemoryMalloc((void **)&pPool->pHead,pPool->wNum*CellLen,NVRAM);  
	/*if (pPool->wNum) pPool->Sem = smMCreate();*/
}

void PoolReset(struct VPool *pPool)
{  
    if (pPool->pHead == NULL) return;

	pPool->wWritePtr = 0;
	memset(pPool->pHead, 0, pPool->wNum*pPool->wCellLen);
}


#ifdef INCLUDE_B2F

void PoolScanFix(struct VPool *pPool,const WORD PoolNum,const WORD CellLen, char *fname)
{  
	VB2FSocket socket;
    WORD oldptr, wptr, lastPtr;
	int i;
	BYTE *p;
	
	oldptr = wptr = pPool->wWritePtr;

	i = 0; 
	while (i < PoolNum)
	{
		if (wptr == 0)
			lastPtr = PoolNum-1;
		else
			lastPtr = wptr-1;

        p = (BYTE *)pPool->pHead+lastPtr*CellLen;
        
		if (memIs0(p, CellLen) == FALSE) break;

		pPool->wWritePtr = lastPtr;
		wptr = lastPtr;
		i++;
	}

    if (i == PoolNum)
		pPool->wWritePtr = 0;

	if (pPool->wWritePtr != oldptr)
	{
		socket.read_write = 3;
		strcpy(socket.fname, fname);
		socket.offset = 0;
		socket.buf = (BYTE *)pPool;
		socket.len = sizeof(WORD);
		Buf2FileWrite(&socket);
	}
}

struct VSysEventFlag sysEventFlag[] = 
{
	{"", 0},
	{"fa.evt", sizeof(struct VSysEventInfo)},
	{"act.evt", sizeof(struct VSysEventInfo)},
	{"do.evt", sizeof(struct VSysEventInfo)},
	{"warn.evt", sizeof(struct VSysEventInfo)},	
	{"%s.dsoe", sizeof(struct VDBDSOE)}, 
	{"%s.soe", sizeof(struct VDBSOE)},	
	{"assert.evt", sizeof(struct VAssertInfo)},
#ifdef INCLUDE_B2F_COS
	{"%s.dcos", sizeof(struct VDBDCOS)}, 
	{"%s.cos", sizeof(struct VDBCOS)}, 
#endif	
  {"%s.co",sizeof(struct VDBCO)},
  {"%s.flow",sizeof(struct VSysFlowEventInfo)},
  {"ulog",sizeof(struct VSysEventInfo)},
};

void PoolMalloc_B2F(struct VPool *pPool,const WORD RelEleNum,const WORD PoolNum,const WORD CellLen, WORD wID, int flag)
{  
	VB2FSocket socket;
	WORD tmp[2];
	int fileerr;

	if (RelEleNum == 0) return;

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
		
	socket.read_write = 1;
	socket.offset = 0;
	socket.buf = (BYTE *)tmp;
	socket.len = sizeof(WORD)*2;
	if (Buf2FileRead(&socket) == ERROR) 
	{
		socket.read_write = 3;
		socket.offset = 0;
		socket.buf = (BYTE *)pPool;
		socket.len = sizeof(WORD)*2;
		Buf2FileWrite(&socket);
		return;
	}	

	fileerr = 0;
	if (RelEleNum)    
	{
		if (tmp[1]!=PoolNum)
		{
			fileerr = 1;
		} 
	}       
	else
	{
		if ((tmp[0]!=0)||(tmp[1]!=0))
		{
			fileerr = 1;
		} 
	}     

	if (tmp[0]>tmp[1])
	{
		fileerr = 1;
	}  
	else  if (tmp[0]==tmp[1])
	{
		if (tmp[0]);
		{
			fileerr = 1;
		}   
	}  

	if (fileerr == 0)
	{
        pPool->wWritePtr = tmp[0];
		pPool->wNum = tmp[1];
		socket.read_write = 1;
		socket.offset = sizeof(WORD)*2;
		socket.buf = (BYTE *)pPool->pHead;
		socket.len = pPool->wNum*CellLen;
		Buf2FileRead(&socket);
	}
	else
	{
		socket.read_write = 3;
		socket.offset = 0;
		socket.buf = (BYTE *)pPool;
		socket.len = sizeof(WORD)*2;
		Buf2FileWrite(&socket);
	}

	PoolScanFix(pPool, PoolNum, CellLen, socket.fname);
}   

void PoolReset_B2F(struct VPool *pPool, WORD wID, int flag)
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

	Buf2FileDel(&socket);
}

#endif

/*------------------------------------------------------------------------
 Procedure:     PubPollAndTableSetup ID:1
 Purpose:       公共的事件池和信息表建立
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/ 
void PubPollAndTableSetup(void)
{
#if 0
	MemoryMalloc((void **)&g_Sys.pFAProc,sizeof(struct VFAProc),NVRAM);
	PoolMalloc((struct VPool *)g_Sys.pFAProc,TRUE,FAPROCNUM,sizeof(struct VFAProcInfo));
#ifdef INCLUDE_B2F_EVT
	PoolMalloc_B2F((struct VPool *)g_Sys.pFAProc,TRUE,FAPROCNUM,sizeof(struct VFAProcInfo),0,SYSEV_FLAG_FA);
#endif
	g_Sys.pFAProc->dwSem = smMCreate();
#endif

	MemoryMalloc((void **)&g_Sys.pActEvent, sizeof(struct VSysEvent),NVRAM);
	PoolMalloc((struct VPool *)g_Sys.pActEvent,TRUE,SYSEVENTNUM,sizeof(struct VSysEventInfo));
#ifdef INCLUDE_B2F_EVT
	PoolMalloc_B2F((struct VPool *)g_Sys.pActEvent,TRUE,SYSEVENTNUM,sizeof(struct VSysEventInfo),0,SYSEV_FLAG_ACT);
#endif
    g_Sys.pActEvent->wMmiPtr = g_Sys.pActEvent->wWritePtr;
	g_Sys.pActEvent->dwSem = smMCreate();

	MemoryMalloc((void **)&g_Sys.pDoEvent, sizeof(struct VSysEvent),NVRAM);
	PoolMalloc((struct VPool *)g_Sys.pDoEvent,TRUE,SYSEVENTNUM,sizeof(struct VSysEventInfo));
#ifdef INCLUDE_B2F_EVT
	PoolMalloc_B2F((struct VPool *)g_Sys.pDoEvent,TRUE,SYSEVENTNUM,sizeof(struct VSysEventInfo),0,SYSEV_FLAG_DO);
#endif
	g_Sys.pDoEvent->wMmiPtr = g_Sys.pDoEvent->wWritePtr;
	g_Sys.pDoEvent->dwSem = smMCreate();

	MemoryMalloc((void **)&g_Sys.pWarnEvent, sizeof(struct VSysEvent),NVRAM);
	PoolMalloc((struct VPool *)g_Sys.pWarnEvent,TRUE,SYSEVENTNUM,sizeof(struct VSysEventInfo));
#ifdef INCLUDE_B2F_EVT
	PoolMalloc_B2F((struct VPool *)g_Sys.pWarnEvent,TRUE,SYSEVENTNUM,sizeof(struct VSysEventInfo),0,SYSEV_FLAG_WARN);
#endif
    g_Sys.pWarnEvent->wMmiPtr = g_Sys.pWarnEvent->wWritePtr;
	g_Sys.pWarnEvent->dwSem = smMCreate();

	MemoryMalloc((void **)&g_Sys.pUlogEvent, sizeof(struct VSysEvent),NVRAM);
	PoolMalloc((struct VPool *)g_Sys.pUlogEvent,TRUE,SYSULOGNUM,sizeof(struct VSysEventInfo));
	
    g_Sys.pUlogEvent->wMmiPtr = g_Sys.pUlogEvent->wWritePtr;
	g_Sys.pUlogEvent->dwSem = smMCreate();


	MemoryMalloc((void**)&g_Sys.pFlowEvent,sizeof(struct VSysFlowEvent),NVRAM);
	PoolMalloc((struct VPool *)g_Sys.pFlowEvent,TRUE,SYSFLOWNUM,sizeof(struct VSysFlowEventInfo));

    g_Sys.pFlowEvent->wMmiPtr = g_Sys.pFlowEvent->wWritePtr;
	g_Sys.pFlowEvent->dwSem = smMCreate();

	MemoryMalloc((void **)&g_assert, sizeof(struct VAssert), NVRAM);
	PoolMalloc((struct VPool *)g_assert, TRUE, SYSEVENTNUM, sizeof(struct VAssertInfo));
#ifdef INCLUDE_B2F_EVT
    PoolMalloc_B2F((struct VPool *)g_assert, TRUE, SYSEVENTNUM, sizeof(struct VAssertInfo), 0, SYSEV_FLAG_ASSERT);
#endif

	MemoryMalloc((void **)&g_Sys.pFaEvent, sizeof(struct VSysEvent),NVRAM);
	PoolMalloc((struct VPool *)g_Sys.pFaEvent,TRUE,SYSEVENTNUM,sizeof(struct VSysEventInfo));
#ifdef INCLUDE_B2F_EVT
	PoolMalloc_B2F((struct VPool *)g_Sys.pFaEvent,TRUE,SYSEVENTNUM,sizeof(struct VSysEventInfo),0,SYSEV_FLAG_FA);
#endif
    g_Sys.pFaEvent->wMmiPtr = g_Sys.pFaEvent->wWritePtr;
	g_Sys.pFaEvent->dwSem = smMCreate();
}

/*------------------------------------------------------------------------
 Procedure:     SetDefCommInfo ID:1
 Purpose:       设置缺省的通信参数
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/ 
void SetDefPortInfo(void)
{
	struct VPort *pPort;




	pPort = &g_Task[COMM_NET_USR_NO].CommCfg.Port;
	pPort->id = COMM_NET_USR_NO;
	strcpy(pPort->pcol, "GB104");
	pPort->pcol_attr = 1;
	strcpy(pPort->cfgstr[0], "1:0.0.0.0:2404");
	pPort->bakmode = 0;
	pPort->bakcomm = 0;
	pPort->load = 0;

#ifdef INCLUDE_PR_DIFF
	pPort = &g_Task[DIFFPRO0_ID].CommCfg.Port;
	pPort->id = DIFFPRO0_ID;
	strcpy(pPort->pcol, "EXTDIFF");
	pPort->pcol_attr = 0;
	strcpy(pPort->cfgstr[0], "4:230400,8,1,N");
	pPort->bakmode = 0;
	pPort->bakcomm = 0;
	pPort->load = 0;
	pPort = &g_Task[DIFFPRO1_ID].CommCfg.Port;
	pPort->id = DIFFPRO1_ID;
	strcpy(pPort->pcol, "EXTDIFF");
	pPort->pcol_attr = 0;
	strcpy(pPort->cfgstr[0], "4:230400,8,1,N");
	pPort->bakmode = 0;
	pPort->bakcomm = 0;
	pPort->load = 0;

#endif
}

/*------------------------------------------------------------------------
 Procedure: 	GetProtocolCfg ID:1
 Purpose:		检查通信参数
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/ 
void GetProtocolCfg(char *cfgname, int tid)
{
	void *pCfg;
	char *pname;

	pname = (char *)(g_pParafile+1);  
	    
	if (strcmp(pname, g_Task[tid].CommCfg.Port.pcol) != 0)
	{
		myprintf(SYS_ID, LOG_ATTR_WARN, "%s invalid!", cfgname);
		g_Task[tid].CommCfg.pvProtocolCfg=NULL;
		return;
	} 
			
	pCfg = (void *)malloc((DWORD)g_pParafile->nLength-sizeof(struct VFileHead));
	if (pCfg!=NULL)	
		memcpy(pCfg,g_pParafile+1,(DWORD)g_pParafile->nLength-sizeof(struct VFileHead));
	g_Task[tid].CommCfg.pvProtocolCfg = pCfg;
}

void CheckAndVerifyPortInfo(void)
{
#if defined INCLUDE_WIFI
	struct VPPort *pPPort, *ptemPort, PAddPort;
	int i, id, portnum;
#endif

	if (ReadParaFile("port.cfg",(BYTE *)g_pParafile,MAXFILELEN) == ERROR)
	{
		SetDefPortInfo();

		if (ReadParaFile("port.cfg",(BYTE *)g_pParafile,MAXFILELEN) == ERROR)
			return;
	}	
}
void PortInfoInit(void)
{
	struct VPort *pPort;
	struct VPPort *pPPort;
	int i, k, id, portnum;
	int para_id;
	char fname[MAXFILENAME];

	CheckAndVerifyPortInfo();

	if (ReadParaFile("port.cfg",(BYTE *)g_pParafile,MAXFILELEN)==OK)
	{
		portnum = (g_pParafile->nLength-sizeof(struct VFileHead))/sizeof(struct VPPort);
		pPPort = (struct VPPort *)(g_pParafile+1);

		for (i=0; i<portnum; i++)
		{
			id = GetAppPortId(pPPort->id);

			if ((id<COMM_START_NO) || (id>COMM_START_NO+COMM_NUM) || 
				((id>=COMM_NET_START_NO) && (id<COMM_NET_USR_NO)))
			{
				WriteWarnEvent(NULL, SYS_ERR_CFG, id, "端口参数错误", id);						
				myprintf(SYS_ID, LOG_ATTR_WARN, "Invlid port id%d!", id);
				continue; 
			}

			pPort = &g_Task[id].CommCfg.Port;
			
			pPort->id = id;
			strcpy(pPort->pcol, pPPort->pcol);
			pPort->pcol_attr = pPPort->pcol_attr;
			for (k=0; k<3; k++)
				strcpy(pPort->cfgstr[k], pPPort->cfgstr[k]);
			pPort->bakmode = pPPort->bakmode;
			pPort->bakcomm = pPPort->bakcomm;
			
			pPPort++;
		}
	}

#if (DEV_SP == DEV_SP_FTU)
    pPort = &g_Task[MMI_ID].CommCfg.Port;
	pPort->id = MMI_ID;
	strcpy(pPort->pcol, "");
	pPort->pcol_attr = 0;
	strcpy(pPort->cfgstr[0], "4:115200,8,1,N");
	pPort->bakmode = 0;
	pPort->bakcomm = 0;
	pPort->load = 0;
#endif

#if (DEV_SP == DEV_SP_DTU)
#ifdef INCLUDE_EXT_DISP
	pPort = &g_Task[COMM_SERIAL_START_NO].CommCfg.Port;
	if(strcmp(pPort->pcol,"MMI") == 0)
	{
		g_dtu_lcdenable = 1;
		g_dtu_comm = COMM_SERIAL_START_NO;
		strcpy(pPort->pcol, "");
	}
	pPort = &g_Task[COMM_SERIAL_START_NO+3].CommCfg.Port;
	if(strcmp(pPort->pcol,"MMI") == 0)
	{
		g_dtu_lcdenable = 1;
		g_dtu_comm = COMM_SERIAL_START_NO + 3 ;
        strcpy(pPort->pcol, "");
	}
#endif
#endif

  
#if (DEV_SP == DEV_SP_WDG)
  pPort = &g_Task[MMI_ID].CommCfg.Port;
	pPort->id = MMI_ID;
	strcpy(pPort->pcol, "");
	pPort->pcol_attr = 0;
	strcpy(pPort->cfgstr[0], "4:9600,8,1,N");
	pPort->bakmode = 0;
	pPort->bakcomm = 0;
	pPort->load = 0;
#endif
	
	pPort = &g_Task[MAINT_ID].CommCfg.Port;
	pPort->id = MAINT_ID;
	strcpy(pPort->pcol, "维护");
	pPort->pcol_attr = 0;
	strcpy(pPort->cfgstr[0], "1:0.0.0.0:5678");
	pPort->bakmode = 0;
	pPort->bakcomm = 0;
	pPort->load = 0;

#if (DEV_SP == DEV_SP_DTU)
    pPort = &g_Task[COMM_SERIAL_START_NO+3].CommCfg.Port;
	if(pPort->id == 0)
	{
      	pPort->id = COMM_SERIAL_START_NO+3;
      	strcpy(pPort->pcol, "维护");
      	pPort->pcol_attr = 0;
      	strcpy(pPort->cfgstr[0], "4:115200,8,1,N");
      	pPort->bakmode = 0;
      	pPort->bakcomm = 0;
      	pPort->load = 0;
	}
#endif
#if (DEV_SP == DEV_SP_DTU_PU)
    pPort = &g_Task[COMM_SERIAL_START_NO+6].CommCfg.Port;
	if(pPort->id == 0)
	{
      	pPort->id = COMM_SERIAL_START_NO+6;
      	strcpy(pPort->pcol, "GPS");
      	pPort->pcol_attr = 0;
      	strcpy(pPort->cfgstr[0], "4:9600,8,1,N");
      	pPort->bakmode = 0;
      	pPort->bakcomm = 0;
      	pPort->load = 0;
	}
#endif

#if (DEV_SP == DEV_SP_TTU)
  for(i = 0; i < 6; i++)
	{
		pPort = &g_Task[COMM_SERIAL_START_NO+i].CommCfg.Port;
		if(pPort->id == 0)
		{
			pPort->id = COMM_SERIAL_START_NO+i;
			strcpy(pPort->pcol, "维护");
			pPort->pcol_attr = 0;
			strcpy(pPort->cfgstr[0], "4:115200,8,1,N");
			pPort->bakmode = 0;
			pPort->bakcomm = 0;
			pPort->load = 0;
		}
	}
#endif
	
    for (i=COMM_START_NO; i<(COMM_START_NO+COMM_NUM); i++)
    {
		if (g_Task[i].CommCfg.Port.id)
		{
            para_id = GetParaPortId(i);
			sprintf(fname, "portcfg%d.cfg", para_id);
	
			if (ReadParaFile(fname, (BYTE *)g_pParafile, MAXFILELEN) == OK)    
				GetProtocolCfg(fname, i);
			else
				g_Task[i].CommCfg.pvProtocolCfg = NULL;
		}
    } 

}

DWORD GetIoType(void)
{
    int i;
    DWORD type;
	DWORD iotype;

    iotype =0;
	for (i=0; i<BSP_IO_BOARD; i++)
	{
	    type = Get_Io_Type(i);
		iotype |= ((type & 0x0F)<< (4*i));
	}
	return iotype;

}

DWORD GetAiType(void)
{
    int i;
	DWORD type;
	DWORD aitype;

	aitype = 0;
	for (i=0; i<BSP_AC_BOARD; i++)
	{
	    type = Get_Ai_Type(i);
		aitype |= (type << (i*8));
	}
	
	return aitype;
}

void GetSysConfig(void)
{	
    WORD i;	
	int j = 0;
    struct VMyConfig *pCfg = (struct VMyConfig *)&g_Sys.MyCfg;

	struct VPSysConfig *pPSysCfg=(struct VPSysConfig *)(g_pParafile+1);
	struct VPFdCfg *pPFdCfg = (struct VPFdCfg *)(pPSysCfg+1);
	struct VPAiCfg *pPAiCfg = (struct VPAiCfg *)(pPFdCfg+pPSysCfg->wFDNum);
	struct VPDiCfg *pPDiCfg = (struct VPDiCfg *)(pPAiCfg+pPSysCfg->wAINum);
	struct VPDoCfg *pPDoCfg = (struct VPDoCfg *)(pPDiCfg+pPSysCfg->wDINum);
	struct VPYcCfg *pPYcCfg = (struct VPYcCfg *)(pPDoCfg+pPSysCfg->wDONum);

	pCfg->dwType=pPSysCfg->dwType;
	pCfg->dwName=pPSysCfg->dwName;
	strcpy(pCfg->sByName,pPSysCfg->sByName);
	pCfg->dwCfg = pPSysCfg->dwCfg;
	pCfg->dSntpServer = pPSysCfg->dSntpServer;
	pCfg->dwAIType = pPSysCfg->dwAIType;
	pCfg->dwIOType = pPSysCfg->dwIOType;

	if (pCfg->dwIOType != GetIoType())
	{
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "数字板卡型号错误");								
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Para_DIO_type and Hard_DIO_type not same!");
	}

	pCfg->wDINum=pPSysCfg->wDINum;
	pCfg->wDONum=pPSysCfg->wDONum;
	pCfg->wDYXNum=pPSysCfg->wDYXNum;
	pCfg->wSYXNum=pPSysCfg->wSYXNum;
	pCfg->wDDNum=pPSysCfg->wDDNum;
	pCfg->wYKNum=pPSysCfg->wYKNum;
	pCfg->wYTNum=pPSysCfg->wYTNum;

    pCfg->pDi = NULL;
	pCfg->pDo = NULL;
	if ((pCfg->wDINum > 0) && (pCfg->pDi == NULL))
       pCfg->pDi = (VDiCfg *)calloc(pCfg->wDINum, sizeof(VDiCfg));
	if ((pCfg->wDONum > 0) && (pCfg->pDo == NULL))
       pCfg->pDo = (VDoCfg *)calloc(pCfg->wDONum, sizeof(VDoCfg));
	
	for (i=0; i<pCfg->wDINum; i++)
	{
		pCfg->pDi[i].type = pPDiCfg[i].type;
		pCfg->pDi[i].cfg = pPDiCfg[i].cfg;
		pCfg->pDi[i].dtime = pPDiCfg[i].dtime;
		pCfg->pDi[i].yxno = pPDiCfg[i].yxno;
		pCfg->pDi[i].re_yxno = pPDiCfg[i].re_yxno;
	}

	for (i=0; i<pCfg->wDONum; i++)
	{
		pCfg->pDo[i].hzontime = pPDoCfg[i].hzontime;
		pCfg->pDo[i].fzontime = pPDoCfg[i].fzontime;
		pCfg->pDo[i].yztime = pPDoCfg[i].yztime;
		pCfg->pDo[i].ybno = pPDoCfg[i].ybno;
	}


	if ((pCfg->dwAIType&0xEFEFEFEF) !=(GetAiType()&0xEFEFEFEF))
	{
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "模拟板卡型号错误");
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Para_AIO_type and Hard_AIO_type not same%x,%x!",pCfg->dwAIType,GetAiType());
	}
	
	pCfg->wFDNum=pPSysCfg->wFDNum;
	pCfg->wAINum=pPSysCfg->wAINum;
	pCfg->wYCNum=pPSysCfg->wYCNum;
	pCfg->wYCNumPublic=pPSysCfg->wYcNumPublic;
	pCfg->wYCNumAdd = pPSysCfg->wYcNumAdd;

    if ((pCfg->wFDNum > 0) && (pCfg->pFd == NULL))
       pCfg->pFd = (VFdCfg *)calloc(pCfg->wFDNum, sizeof(VFdCfg));
	if ((pCfg->wAINum > 0) && (pCfg->pAi == NULL))
       pCfg->pAi = (VAiCfg *)calloc(pCfg->wAINum, sizeof(VAiCfg));
	if ((pCfg->wYCNum > 0) && (pCfg->pYc == NULL))
       pCfg->pYc = (VYcCfg *)calloc(pCfg->wYCNum, sizeof(VYcCfg));

    for (i=0; i<pCfg->wFDNum; i++)
    {
		strcpy(pCfg->pFd[i].kgname, pPFdCfg[i].kgname);
        pCfg->pFd[i].kgid = g_Sys.AddrInfo.wAddr*100+i+1;			
		pCfg->pFd[i].cfg = pPFdCfg[i].cfg;
		pCfg->pFd[i].Un= pPFdCfg[i].Un;
		pCfg->pFd[i].In= pPFdCfg[i].In;
		pCfg->pFd[i].pt = pPFdCfg[i].pt;
		pCfg->pFd[i].ct = pPFdCfg[i].ct;

		pCfg->pFd[i].kg_stateno = (int)pPFdCfg[i].kg_stateno;
		pCfg->pFd[i].kg_faultno = (int)pPFdCfg[i].kg_faultno;
		pCfg->pFd[i].kg_ykno = (int)pPFdCfg[i].kg_ykno;
		pCfg->pFd[i].kg_vectorno = (int)pPFdCfg[i].kg_vectorno;
		pCfg->pFd[i].kg_openno = (int)pPFdCfg[i].kg_openno;
		pCfg->pFd[i].kg_closeno = (int)pPFdCfg[i].kg_closeno;
		pCfg->pFd[i].kg_startno= (int)pPFdCfg[i].kg_startno;
		pCfg->pFd[i].kg_unlockno= (int)pPFdCfg[i].kg_unlockno;
		pCfg->pFd[i].kg_ykkeepno= (int)pPFdCfg[i].kg_ykkeepno;
		pCfg->pFd[i].kg_remoteno = (int)pPFdCfg[i].kg_remoteno;
#if(DEV_SP != DEV_SP_DTU)
		pCfg->pFd[i].kg_remoteno = -1;
#else
		if(pPFdCfg[i].kg_remoteno ==0)
			j++;
#endif	
	    if(pPFdCfg[i].In0 == 0)
			pCfg->pFd[i].In0 = pCfg->pFd[i].In;
		else
		    pCfg->pFd[i].In0 = pPFdCfg[i].In0;
		if(pPFdCfg[i].ct0 == 0)
			pCfg->pFd[i].ct0 = pCfg->pFd[i].ct;
		else
		    pCfg->pFd[i].ct0 = pPFdCfg[i].ct0;
	}
	if(j == pCfg->wFDNum)
	{
		 for (i=0; i<pCfg->wFDNum; i++)
	    {
	    	pCfg->pFd[i].kg_remoteno =-1;
		 }
	}
	for (i=0; i<pCfg->wAINum; i++)
	{
		pCfg->pAi[i].type = pPAiCfg[i].type;
		pCfg->pAi[i].cfg = pPAiCfg[i].cfg;
		pCfg->pAi[i].admax = pPAiCfg[i].admax;
		pCfg->pAi[i].admin = pPAiCfg[i].admin;
	}
	
	for (i=0; i<pCfg->wYCNum; i++)
	{
		pCfg->pYc[i].type = pPYcCfg[i].type;
		pCfg->pYc[i].cfg = pPYcCfg[i].cfg;
		pCfg->pYc[i].arg1 = pPYcCfg[i].arg1;
		pCfg->pYc[i].arg2 = pPYcCfg[i].arg2;
		pCfg->pYc[i].toZero = pPYcCfg[i].toZero;
	}	

	pCfg->wAllIoFDNum = pCfg->wFDNum;

	pCfg->wAllIoAINum = pCfg->wAINum;
	pCfg->wAllIoDINum = pCfg->wDINum;
	pCfg->wAllIoDONum = pCfg->wDONum;
	
	pCfg->wAllIoYCNum = pCfg->wYCNum;
	pCfg->wAllIoDYXNum = pCfg->wDYXNum;
	pCfg->wAllIoSYXNum = pCfg->wSYXNum;
	pCfg->wAllIoDDNum = pCfg->wDDNum;
	pCfg->wAllIoYKNum = pCfg->wYKNum;
	pCfg->wAllIoYTNum = pCfg->wYTNum;
}    

/*------------------------------------------------------------------------
 Procedure:     SetSysConfig ID:1
 Purpose:       设置缺省系统配置
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void SetDefSysConfig(void)
{
	myprintf(SYS_ID, LOG_ATTR_INFO, "Create default system.cfg.");

	CreateSysConfig(NULL); 

	if (ReadParaFile("system.cfg",(BYTE *)g_pParafile,MAXFILELEN)==ERROR) return;	  

	GetSysConfig();
}

void InitMyFdNo(void)
{
    int i;
    struct VMyConfig *pCfg = (struct VMyConfig *)&g_Sys.MyCfg;
	VFdCfg *pFdCfg = pCfg->pFd;
  
	for (i=0; i<pCfg->wFDNum; i++)
	{	
		if (pFdCfg->kg_stateno >= pCfg->wDINum)
			pFdCfg->kg_stateno = -1;

		if ((pCfg->pFd[i].kg_faultno < 0) || (pCfg->pFd[i].kg_faultno >= pCfg->wDINum))
			pCfg->pFd[i].kg_faultno = pCfg->SYXIoNo[2].wIoNo_Low+(i+1 + (pCfg->wFDNum > 1))*PR_VYX_NUM+0;
		
		if ((pFdCfg->kg_ykno <= 0) || (pFdCfg->kg_ykno > pCfg->wDONum))
			pFdCfg->kg_ykno = -1;

		if ((pFdCfg->kg_ykkeepno<= 0) || (pFdCfg->kg_ykkeepno > pCfg->wDONum))
			pFdCfg->kg_ykkeepno = -1;

		if (pFdCfg->kg_remoteno >= pCfg->wDINum)
			pFdCfg->kg_remoteno = -1;

		if ((pCfg->pFd[i].kg_vectorno < 0) || (pCfg->pFd[i].kg_vectorno >= pCfg->wDINum))
			pCfg->pFd[i].kg_vectorno = pCfg->SYXIoNo[2].wIoNo_Low+(i+1 + (pCfg->wFDNum > 1))*PR_VYX_NUM+1;

		pFdCfg++;
	}
}

#if 1
void SetIoNo(void)
{
    int i,j,num;
	BYTE type,ua,ub,uc;
	VDefDiCfg *pDefDiCfg;	
	VDefDoCfg *pDefDoCfg;
	VDefAiCfg *pDefAiCfg;
#if (TYPE_IO == IO_TYPE_MASTER)
    int i;
	struct VExtIoConfig *pCfg;
	struct VPExtIoAddrList *pAddr;	
#endif
	struct VPYcCfg *pYcCfg;

	WORD wFDOffset = 0;
    WORD wAIOffset = 0;
	WORD wDIOffset = 0;
	WORD wDOOffset = 1;
	WORD wYCOffset = 0;
	WORD wSYXOffset = 0;	
	WORD wDYXOffset = 0;
	WORD wDDOffset = 0;
	WORD wYKOffset = 1;
	WORD wYTOffset = 1;
	WORD wAIYXOffset = 0;
	WORD wAIYKOffset = 0;

	ua = ub = uc = 0;
    for (i=0; i<BSP_AC_BOARD; i++)
    {
        type = (g_Sys.MyCfg.dwAIType >> (i*8))&0xFF;
		if (type == 0xFF) continue;
		pDefAiCfg = GetDefAiCfg(type);

		num = wYCOffset;
		
		if (pDefAiCfg != NULL)
		{
		    wFDOffset += pDefAiCfg->fdnum;
			wAIOffset += pDefAiCfg->ainum;
			wYCOffset += pDefAiCfg->ycnum;
			wDDOffset += pDefAiCfg->tvddcfgnum * pDefAiCfg->fdnum;

			wAIYXOffset+= pDefAiCfg->tvyxcfgnum_fd * pDefAiCfg->fdnum;
			wAIYKOffset+= pDefAiCfg->tvykcfgnum_fd * pDefAiCfg->fdnum;

            pYcCfg = pDefAiCfg->pyccfg;
			for (j=0; j<pDefAiCfg->ainum; j++)
			{
			    if (pYcCfg[j].type == YC_TYPE_Ua)
					ua = 1;
				if (pYcCfg[j].type == YC_TYPE_Ub)
					ub = 1;
				if (pYcCfg[j].type == YC_TYPE_Uc)
					uc = 1;
			}
			if (ua)
				wYCOffset += pDefAiCfg->fdnum*2;
			if (ub)
				wYCOffset += pDefAiCfg->fdnum*2;
			if (uc)
				wYCOffset += pDefAiCfg->fdnum*2;
			if (ua || ub || uc)
				wYCOffset += pDefAiCfg->fdnum*4;
		}
		g_Sys.MyCfg.byYcNum[i] = wYCOffset - num;
    }
	wYCOffset += (1+g_Sys.MyCfg.wYCNumAdd);
	
	memset(&g_Sys.MyCfg.SYXIoNo, 0, sizeof(g_Sys.MyCfg.SYXIoNo));

	g_Sys.MyCfg.SYXIoNo[0].wIoNo_Low = wDIOffset;    //di

	for (i=0; i<BSP_IO_BOARD; i++)
	{
	    type = (g_Sys.MyCfg.dwIOType >> (i*4))&0xF;
		if (type == 0x0F) continue;
		pDefDiCfg = GetDefDiCfg(type);

		num = wDIOffset;
		if (pDefDiCfg != NULL)
		{
		    wDIOffset += pDefDiCfg->tvyxcfgnum_di;
			wDYXOffset += pDefDiCfg->tvdyxcfgnum;
		}
		g_Sys.MyCfg.byYxNum[i] = wDIOffset - num;
	}

	
	g_Sys.MyCfg.SYXIoNo[0].wIoNo_High = wDIOffset;
	g_Sys.MyCfg.SYXIoNo[0].wNum = wDIOffset;


	memset(&g_Sys.MyCfg.YKIoNo, 0, sizeof(g_Sys.MyCfg.YKIoNo));
	g_Sys.MyCfg.YKIoNo[0].wIoNo_Low = wDOOffset;    //do

	for (i=0; i<BSP_IO_BOARD; i++)
	{
	    type = (g_Sys.MyCfg.dwIOType >> (i*4))&0xF;
		if (type == 0x0F) continue;
		pDefDoCfg = GetDefDoCfg(type);

        num = wDOOffset;
		if (pDefDoCfg != NULL)
		{
		    wDOOffset += pDefDoCfg->tvykcfgnum_do;
		}
		g_Sys.MyCfg.byYkNum[i] = wDOOffset - num;
	}

	g_Sys.MyCfg.YKIoNo[0].wIoNo_High = wDOOffset;
	g_Sys.MyCfg.YKIoNo[0].wNum = wDOOffset - 1;		

#if (TYPE_IO == IO_TYPE_MASTER)
	pAddr = g_Sys.pExtIoAddrList;
	pCfg = g_Sys.pExtIoCfg; 	
	for (i=0; i<g_Sys.wExtIoNum; i++)
	{		
		pCfg->wFDOffset = wFDOffset;
		pCfg->wAIOffset = wAIOffset;
		pCfg->wYCOffset = wYCOffset;
		pCfg->wDDOffset = wDDOffset;
		
		pDefAiCfg = GetDefExtAiCfg(pAddr->ExtIoAddr.dwType);
		if (pDefAiCfg != NULL)
		{
			wFDOffset += pDefAiCfg->fdnum;
			wAIOffset += pDefAiCfg->ainum;
			wYCOffset += pDefAiCfg->tvyccfgnum;
			wDDOffset += pDefAiCfg->tvddcfgnum;
		}
		
        memset(pCfg->SYXIoNo, 0, IONO_BUF_NUM*sizeof(struct VIoNo));

		pCfg->SYXIoNo[0].wIoNo_Low = wDIOffset;			
		pCfg->wDYXOffset = wDYXOffset;		
		pDefDiCfg = GetDefExtDiCfg(pAddr->ExtIoAddr.dwType);
		if (pDefDiCfg != NULL)
		{
			wDIOffset += pDefDiCfg->tvyxcfgnum_di;
			wDYXOffset += pDefDiCfg->tvdyxcfgnum;

			pCfg->SYXIoNo[0].wIoNo_High = wDIOffset;
			pCfg->SYXIoNo[0].wNum = pDefDiCfg->tvyxcfgnum_di;
		}

        memset(pCfg->YKIoNo, 0, IONO_BUF_NUM*sizeof(struct VIoNo));

		pCfg->YKIoNo[0].wIoNo_Low = wDOOffset;
		pDefDoCfg = GetDefExtDoCfg(pAddr->ExtIoAddr.dwType);
		if (pDefDoCfg != NULL)
		{
			wDOOffset += pDefDoCfg->tvykcfgnum_do;

			pCfg->YKIoNo[0].wIoNo_High = wDOOffset;
			pCfg->YKIoNo[0].wNum = pDefDoCfg->tvykcfgnum_do;
		}
		
		pAddr = pAddr->next;
		pCfg++;
	}
#endif

    pDefAiCfg = GetDefAiCfg(0);
    if (pDefAiCfg != NULL)
    {
        wYCOffset += pDefAiCfg->tvyccfgnum_public;
		if (wFDOffset > 0)
		   wAIYXOffset += pDefAiCfg->tvyxcfgnum_public*wFDOffset;
    }

    wSYXOffset = wDIOffset;
	g_Sys.MyCfg.SYXIoNo[1].wIoNo_Low = wSYXOffset;    //di_public	
	
	pDefDiCfg = GetDefDiCfg(0);
	if (pDefDiCfg != NULL)
	{
#if (TYPE_IO == IO_TYPE_EXTIO)
		num = 0;
#else
		num = pDefDiCfg->tvyxcfgnum_public;
#endif		

        wSYXOffset += num;

		g_Sys.MyCfg.SYXIoNo[1].wIoNo_High = wSYXOffset;	  
		g_Sys.MyCfg.SYXIoNo[1].wNum = num;
	}

    wYKOffset = wDOOffset;
	g_Sys.MyCfg.YKIoNo[1].wIoNo_Low = wYKOffset;    //do_public	
	pDefDoCfg = GetDefDoCfg(0);
	if (pDefDoCfg != NULL)
	{
#if (TYPE_IO == IO_TYPE_EXTIO)
		num = 0;
#else
		num = pDefDoCfg->tvykcfgnum_public;
#endif		

        wYKOffset += num;

		g_Sys.MyCfg.YKIoNo[1].wIoNo_High = wYKOffset;	  
		g_Sys.MyCfg.YKIoNo[1].wNum = num;

	}

	g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low = wSYXOffset;   //SoftYx
	g_Sys.MyCfg.YKIoNo[2].wIoNo_Low = wYKOffset;	   //SoftYk

    wSYXOffset += wAIYXOffset;
	wYKOffset  += wAIYKOffset;

	g_Sys.MyCfg.SYXIoNo[2].wIoNo_High = wSYXOffset;	//SoftYx
	g_Sys.MyCfg.YKIoNo[2].wIoNo_High = wYKOffset;    //SoftYk
		
	g_Sys.MyCfg.SYXIoNo[2].wNum = wAIYXOffset;
	g_Sys.MyCfg.YKIoNo[2].wNum = wAIYKOffset;


#if (TYPE_IO == IO_TYPE_MASTER)
	pAddr = g_Sys.pExtIoAddrList;
	pCfg = g_Sys.pExtIoCfg; 	
	for (i=0; i<g_Sys.wExtIoNum; i++)
	{		
		pCfg->SYXIoNo[2].wIoNo_Low = wSYXOffset;   //SoftYx
		pCfg->YKIoNo[2].wIoNo_Low = wYKOffset;     //SoftYk
		
		pDefAiCfg = GetDefExtAiCfg(pAddr->ExtIoAddr.dwType);
		if (pDefAiCfg != NULL)
		{
			wSYXOffset += pDefAiCfg->tvyxcfgnum_fd;
			wYKOffset += pDefAiCfg->tvykcfgnum_fd;
			
			pCfg->SYXIoNo[2].wIoNo_High = wSYXOffset;   //SoftYx
			pCfg->YKIoNo[2].wIoNo_High = wYKOffset;	   //SoftYk
			
			pCfg->SYXIoNo[2].wNum = pDefAiCfg->tvyxcfgnum_fd;
			pCfg->YKIoNo[2].wNum = pDefAiCfg->tvykcfgnum_fd;
		}

		pAddr = pAddr->next;
		pCfg++;
	}
#endif

	g_Sys.MyCfg.wAllIoFDNum = wFDOffset;

	g_Sys.MyCfg.wAllIoAINum = wAIOffset;
	g_Sys.MyCfg.wAllIoDINum = wDIOffset;
	g_Sys.MyCfg.wAllIoDONum = wDOOffset-1;

	g_Sys.MyCfg.wAllIoYCNum = wYCOffset;
	g_Sys.MyCfg.wAllIoDYXNum = wDYXOffset;
#if (TYPE_IO == IO_TYPE_MASTER)
	g_Sys.MyCfg.wAllIoSYXNum = wSYXOffset+g_Sys.wExtIoNum+1;	//+comm_state
#else
	g_Sys.MyCfg.wAllIoSYXNum = wSYXOffset+1;	                //+comm_state
#endif	
	g_Sys.MyCfg.wAllIoDDNum = wDDOffset;
	g_Sys.MyCfg.wAllIoYKNum = wYKOffset-1;
	g_Sys.MyCfg.wAllIoYTNum = wYTOffset-1;

	InitMyFdNo();
}
#else
void SetIoNo(void)
{
    int num;
	VDefDiCfg *pDefDiCfg;	
	VDefDoCfg *pDefDoCfg;
	VDefAiCfg *pDefAiCfg;
#if (TYPE_IO == IO_TYPE_MASTER)
    int i;
	struct VExtIoConfig *pCfg;
	struct VPExtIoAddrList *pAddr;	
#endif

	WORD wFDOffset = 0;
    WORD wAIOffset = 0;
	WORD wDIOffset = 0;
	WORD wDOOffset = 1;
	WORD wYCOffset = 0;
	WORD wSYXOffset = 0;	
	WORD wDYXOffset = 0;
	WORD wDDOffset = 0;
	WORD wYKOffset = 1;
	WORD wYTOffset = 1;
	
	pDefAiCfg = GetDefAiCfg(g_Sys.MyCfg.dwType);
	if (pDefAiCfg != NULL)
	{
		wFDOffset += pDefAiCfg->fdnum;
		wAIOffset += pDefAiCfg->ainum;
		wYCOffset += pDefAiCfg->tvyccfgnum;
		wDDOffset += pDefAiCfg->tvddcfgnum;
	}

	memset(&g_Sys.MyCfg.SYXIoNo, 0, sizeof(g_Sys.MyCfg.SYXIoNo));
	g_Sys.MyCfg.SYXIoNo[0].wIoNo_Low = wDIOffset;    //di

	pDefDiCfg = GetDefDiCfg(g_Sys.MyCfg.dwType);
	if (pDefDiCfg != NULL)
	{
		wDIOffset += pDefDiCfg->tvyxcfgnum_di;
		wDYXOffset += pDefDiCfg->tvdyxcfgnum;

		g_Sys.MyCfg.SYXIoNo[0].wIoNo_High = wDIOffset;
		g_Sys.MyCfg.SYXIoNo[0].wNum = pDefDiCfg->tvyxcfgnum_di;		
	}

	memset(&g_Sys.MyCfg.YKIoNo, 0, sizeof(g_Sys.MyCfg.YKIoNo));
	g_Sys.MyCfg.YKIoNo[0].wIoNo_Low = wDOOffset;    //do
	
	pDefDoCfg = GetDefDoCfg(g_Sys.MyCfg.dwType);
	if (pDefDoCfg != NULL)
	{
		wDOOffset += pDefDoCfg->tvykcfgnum_do;

		g_Sys.MyCfg.YKIoNo[0].wIoNo_High = wDOOffset;
		g_Sys.MyCfg.YKIoNo[0].wNum = pDefDoCfg->tvykcfgnum_do;		
	}

#if (TYPE_IO == IO_TYPE_MASTER)
	pAddr = g_Sys.pExtIoAddrList;
	pCfg = g_Sys.pExtIoCfg; 	
	for (i=0; i<g_Sys.wExtIoNum; i++)
	{		
		pCfg->wFDOffset = wFDOffset;
		pCfg->wAIOffset = wAIOffset;
		pCfg->wYCOffset = wYCOffset;
		pCfg->wDDOffset = wDDOffset;
		
		pDefAiCfg = GetDefExtAiCfg(pAddr->ExtIoAddr.dwType);
		if (pDefAiCfg != NULL)
		{
			wFDOffset += pDefAiCfg->fdnum;
			wAIOffset += pDefAiCfg->ainum;
			wYCOffset += pDefAiCfg->tvyccfgnum;
			wDDOffset += pDefAiCfg->tvddcfgnum;
		}
		
        memset(pCfg->SYXIoNo, 0, IONO_BUF_NUM*sizeof(struct VIoNo));

		pCfg->SYXIoNo[0].wIoNo_Low = wDIOffset;			
		pCfg->wDYXOffset = wDYXOffset;		
		pDefDiCfg = GetDefExtDiCfg(pAddr->ExtIoAddr.dwType);
		if (pDefDiCfg != NULL)
		{
			wDIOffset += pDefDiCfg->tvyxcfgnum_di;
			wDYXOffset += pDefDiCfg->tvdyxcfgnum;

			pCfg->SYXIoNo[0].wIoNo_High = wDIOffset;
			pCfg->SYXIoNo[0].wNum = pDefDiCfg->tvyxcfgnum_di;
		}

        memset(pCfg->YKIoNo, 0, IONO_BUF_NUM*sizeof(struct VIoNo));

		pCfg->YKIoNo[0].wIoNo_Low = wDOOffset;
		pDefDoCfg = GetDefExtDoCfg(pAddr->ExtIoAddr.dwType);
		if (pDefDoCfg != NULL)
		{
			wDOOffset += pDefDoCfg->tvykcfgnum_do;

			pCfg->YKIoNo[0].wIoNo_High = wDOOffset;
			pCfg->YKIoNo[0].wNum = pDefDoCfg->tvykcfgnum_do;
		}
		
		pAddr = pAddr->next;
		pCfg++;
	}
#endif

    pDefAiCfg = GetDefAiCfg(g_Sys.MyCfg.dwType);
    if(pDefAiCfg != NULL)
    {
        wYCOffset += pDefAiCfg->tvyccfgnum_public;
    }

	wSYXOffset = wDIOffset;
	g_Sys.MyCfg.SYXIoNo[1].wIoNo_Low = wSYXOffset;    //di_public	
	pDefDiCfg = GetDefDiCfg(g_Sys.MyCfg.dwType);
	if (pDefDiCfg != NULL)
	{
#if (TYPE_IO == IO_TYPE_EXTIO)
		num = 0;
#else
		num = pDefDiCfg->tvyxcfgnum_public;
#endif		
		wSYXOffset += num;
		
		g_Sys.MyCfg.SYXIoNo[1].wIoNo_High = wSYXOffset;	  
		g_Sys.MyCfg.SYXIoNo[1].wNum = num;
	}

	wYKOffset = wDOOffset;	
	g_Sys.MyCfg.YKIoNo[1].wIoNo_Low = wYKOffset;    //do_public	
	pDefDoCfg = GetDefDoCfg(g_Sys.MyCfg.dwType);
	if (pDefDoCfg != NULL)
	{
#if (TYPE_IO == IO_TYPE_EXTIO)
		num = 0;
#else
		num = pDefDoCfg->tvykcfgnum_public;
#endif		
		wYKOffset += num;

		g_Sys.MyCfg.YKIoNo[1].wIoNo_High = wYKOffset;	  
		g_Sys.MyCfg.YKIoNo[1].wNum = num;
	}

	g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low = wSYXOffset;   //SoftYx
	g_Sys.MyCfg.YKIoNo[2].wIoNo_Low = wYKOffset;	   //SoftYk

	pDefAiCfg = GetDefAiCfg(g_Sys.MyCfg.dwType);
	if (pDefAiCfg != NULL)
	{
#if (TYPE_IO == IO_TYPE_EXTIO)
		num = 0;
#else
		num = pDefAiCfg->tvyxcfgnum_public;
#endif		

        wSYXOffset += num;
		wSYXOffset += pDefAiCfg->tvyxcfgnum_fd;
		wYKOffset += pDefAiCfg->tvykcfgnum_fd;
		
		g_Sys.MyCfg.SYXIoNo[2].wIoNo_High = wSYXOffset;	//SoftYx
		g_Sys.MyCfg.YKIoNo[2].wIoNo_High = wYKOffset;    //SoftYk
		
		g_Sys.MyCfg.SYXIoNo[2].wNum = pDefAiCfg->tvyxcfgnum_public+pDefAiCfg->tvyxcfgnum_fd;
		g_Sys.MyCfg.YKIoNo[2].wNum = pDefAiCfg->tvykcfgnum_fd;
	}

#if (TYPE_IO == IO_TYPE_MASTER)
	pAddr = g_Sys.pExtIoAddrList;
	pCfg = g_Sys.pExtIoCfg; 	
	for (i=0; i<g_Sys.wExtIoNum; i++)
	{		
		pCfg->SYXIoNo[2].wIoNo_Low = wSYXOffset;   //SoftYx
		pCfg->YKIoNo[2].wIoNo_Low = wYKOffset;     //SoftYk
		
		pDefAiCfg = GetDefExtAiCfg(pAddr->ExtIoAddr.dwType);
		if (pDefAiCfg != NULL)
		{
			wSYXOffset += pDefAiCfg->tvyxcfgnum_fd;
			wYKOffset += pDefAiCfg->tvykcfgnum_fd;
			
			pCfg->SYXIoNo[2].wIoNo_High = wSYXOffset;   //SoftYx
			pCfg->YKIoNo[2].wIoNo_High = wYKOffset;	   //SoftYk
			
			pCfg->SYXIoNo[2].wNum = pDefAiCfg->tvyxcfgnum_fd;
			pCfg->YKIoNo[2].wNum = pDefAiCfg->tvykcfgnum_fd;
		}

		pAddr = pAddr->next;
		pCfg++;
	}
#endif

	g_Sys.MyCfg.wAllIoFDNum = wFDOffset;

	g_Sys.MyCfg.wAllIoAINum = wAIOffset;
	g_Sys.MyCfg.wAllIoDINum = wDIOffset;
	g_Sys.MyCfg.wAllIoDONum = wDOOffset-1;

	g_Sys.MyCfg.wAllIoYCNum = wYCOffset;
	g_Sys.MyCfg.wAllIoDYXNum = wDYXOffset;
#if (TYPE_IO == IO_TYPE_MASTER)
	g_Sys.MyCfg.wAllIoSYXNum = wSYXOffset+g_Sys.wExtIoNum+1;	//+comm_state
#else
	g_Sys.MyCfg.wAllIoSYXNum = wSYXOffset+1;	                //+comm_state
#endif	
	g_Sys.MyCfg.wAllIoDDNum = wDDOffset;
	g_Sys.MyCfg.wAllIoYKNum = wYKOffset-1;
	g_Sys.MyCfg.wAllIoYTNum = wYTOffset-1;

	InitMyFdNo();
}
#endif

#ifdef INCLUDE_CELL
void GetCellConfig(void)
{
    int i;
	VCellCtrl *pCtrl;
	VCellCfg *pCfg = (VCellCfg *)&g_Sys.MyCfg.CellCfg;	
	struct VPCellCfg *pPCfg = (struct VPCellCfg *)(g_pParafile+1);
	struct VPCellCtrl *pPCtrl = (struct VPCellCtrl *)(pPCfg+1);

    pCfg->dwCfg = pPCfg->dwCfg;
	pCfg->dwNum = pPCfg->dwNum;
	pCfg->wID = pPCfg->wID;
	pCfg->Udis = pPCfg->Udis;

	if (pCfg->dwNum != 0)
		pCfg->pCtrl = (VCellCtrl *)calloc(pCfg->dwNum, sizeof(VCellCtrl));
	else		
		pCfg->pCtrl = NULL;

	if (pCfg->pCtrl == NULL) return;

	pCtrl = pCfg->pCtrl;
	for (i=0; i<pCfg->dwNum; i++)
	{
	    pCtrl->bEndTime = (pPCtrl->dwMode&CELL_MODE_ENDTIME)>>31;
		pCtrl->dwMode = pPCtrl->dwMode&(~CELL_MODE_ENDTIME);
		pCtrl->wMonthBits = pPCtrl->wMonthBits;
		pCtrl->wWeekBits= pPCtrl->wWeekBits;
		pCtrl->dwDayBits = pPCtrl->dwDayBits;
		pCtrl->wHour = pPCtrl->wHour;
		pCtrl->wMin = pPCtrl->wMin;
		pCtrl->dwTime = pPCtrl->dwTime;
		pCtrl++;
		pPCtrl++;		
	}
}

void SetDefCellConfig(void)
{
	myprintf(SYS_ID, LOG_ATTR_INFO, "Create default cell.cfg.");

	CreateCellConfig(); 

	if (ReadParaFile("cell.cfg", (BYTE *)g_pParafile, MAXFILELEN) == ERROR) return;	  

	GetCellConfig();
}
#endif

#ifdef INCLUDE_USWITCH
void GetUSwitchConfig(void)
{
    int i;
	VUSwitchCtrl *pCtrl;
	VUSwitchCfg *pCfg = (VUSwitchCfg *)&g_Sys.MyCfg.USwitchCfg;	
	struct VPUSwitchCfg *pPCfg = (struct VPUSwitchCfg *)(g_pParafile+1);
	struct VPUSwitchCtrl *pPCtrl = (struct VPUSwitchCtrl *)(pPCfg+1);

    pCfg->dwCfg = pPCfg->dwCfg;
	pCfg->dwNum = pPCfg->dwNum;

	if (pCfg->dwNum != 0)
		pCfg->pCtrl = (VUSwitchCtrl *)calloc(pCfg->dwNum, sizeof(VUSwitchCtrl));
	else
		pCfg->pCtrl = NULL;

	if (pCfg->pCtrl == NULL) return;

	pCtrl = pCfg->pCtrl;
	for (i=0; i<pCfg->dwNum; i++)
	{
		pCtrl->dwMode = pPCtrl->dwMode;
		pCtrl->wUFdNo1 = pPCtrl->wUFdNo1;
		pCtrl->wUFdNo2= pPCtrl->wUFdNo2;
		pCtrl->wUFdYx1= pPCtrl->wUFdYx1;
		pCtrl->wUFdYx2= pPCtrl->wUFdYx2;
		
		pCtrl++;
		pPCtrl++;		
	}
}

void SetDefUSwitchConfig(void)
{
	/*myprintf(SYS_ID, LOG_ATTR_INFO, "Create default uswitch.cfg.");

	CreateUSwitchConfig(); 

	if (ReadParaFile("uswitch.cfg", (BYTE *)g_pParafile, MAXFILELEN) == ERROR) return;	  

	GetUSwitchConfig();*/
}
#endif

void GetRunParaConfig(void)
{	
	int i;
	VRunParaCfg *pCfg = (VRunParaCfg *)&g_Sys.MyCfg.RunParaCfg;	
	VRunParaFileCfg *pPCfg = (VRunParaFileCfg *)(g_pParafile+1);
  
    pCfg->dwDyVal = pPCfg->dwDyVal;
    pCfg->dwGyVal = pPCfg->dwGyVal;
    pCfg->dwZzVal= pPCfg->dwZzVal;
    pCfg->dwGzVal= pPCfg->dwGzVal;    
#if (DEV_SP == DEV_SP_TTU)
		pCfg->wDyT= pPCfg->wDyT;
		pCfg->wGyT= pPCfg->wGyT;
		pCfg->wGzT= pPCfg->wGzT;
		pCfg->wZzT= pPCfg->wZzT;
#else
		pCfg->wDyT= pPCfg->wDyT*1000;
		pCfg->wGyT= pPCfg->wGyT*1000;
		pCfg->wGzT= pPCfg->wGzT*1000;
		pCfg->wZzT= pPCfg->wZzT*1000;
#endif	

	if(g_Sys.MyCfg.wFDNum > 0)
	{
		pCfg->wPt1 = g_Sys.MyCfg.pFd[0].pt;
		pCfg->wPt2 = g_Sys.MyCfg.pFd[0].Un;
	}

	if(g_Sys.MyCfg.wDINum > 0)
	{
		pCfg->wYxfd = g_Sys.MyCfg.pDi[0].dtime;
	}
	if(g_Sys.MyCfg.wDONum > 0)
	{
		pCfg->wFzKeepT = g_Sys.MyCfg.pDo[0].fzontime;
		pCfg->wHzKeepT = g_Sys.MyCfg.pDo[0].hzontime;
	}
#ifdef INCLUDE_CELL
	if((g_Sys.MyCfg.CellCfg.dwNum > 0) && (g_Sys.MyCfg.CellCfg.pCtrl != NULL))
	{
		if(g_Sys.MyCfg.CellCfg.pCtrl->dwMode == 3)
		{
			pCfg->wDchhT = g_Sys.MyCfg.CellCfg.pCtrl->dwDayBits;
			pCfg->wDchhTime = g_Sys.MyCfg.CellCfg.pCtrl->wHour;
		}
	}
#endif
	for(i=0;i<g_Sys.MyCfg.wFDNum;i++)
	{
		if(i >=  MAX_FD_NUM)
			break;
		pCfg->LineCfg[i].wCt1 =  g_Sys.MyCfg.pFd[i].ct;
		pCfg->LineCfg[i].wCt2 =  g_Sys.MyCfg.pFd[i].In;
		pCfg->LineCfg[i].wCt01 =  g_Sys.MyCfg.pFd[i].ct0;
		pCfg->LineCfg[i].wCt02 =  g_Sys.MyCfg.pFd[i].In0;
	}
}

void SetRunParaConfig(void)
{
	myprintf(SYS_ID, LOG_ATTR_INFO, "Create runpara.cfg.");

	CreateRunParaConfig(); 

	if (ReadParaFile("runpara.cfg", (BYTE *)g_pParafile, MAXFILELEN) == ERROR) return;	  
		GetRunParaConfig();
}
void GetSysExtConfig(void)
{
     int i,j;
     BYTE ip[4];
     struct VSysCfgExt *pCfg = &g_Sys.MyCfg.SysExt;
	 struct VPSysExtConfig *pPCfg = (struct VPSysExtConfig *)(g_pParafile+1);

	 strcpy(pCfg->sWsName, pPCfg->sWsName);
	 strcpy(pCfg->sWsSechema, pPCfg->sWsSechema);
	 pCfg->dWsIp = pPCfg->dWsIp;
	 pCfg->dWsVer = pPCfg->dWsVer;

     ip[0] = HIBYTE(HIWORD(pPCfg->dwMaintIp));
	 ip[1] = LOBYTE(HIWORD(pPCfg->dwMaintIp));
	 ip[2] = HIBYTE(LOWORD(pPCfg->dwMaintIp));
	 ip[3] = LOBYTE(LOWORD(pPCfg->dwMaintIp));
	 sprintf(pCfg->sMaintIp, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

	 memcpy(pCfg->byMac, pPCfg->byMac, 6);
	 pCfg->dwKgTime = pPCfg->dwKgTime;

	 for (i=0; i<8; i++)
	 {
	     pCfg->YxIn[i].type = pPCfg->YxIn[i].type;
		 pCfg->YxIn[i].cfg = pPCfg->YxIn[i].cfg;
		 pCfg->YxIn[i].dtime = pPCfg->YxIn[i].dtime;
		 pCfg->YxIn[i].yxno = pPCfg->YxIn[i].yxno;
		 pCfg->YxIn[i].re_yxno = pPCfg->YxIn[i].re_yxno;
	 }
	 strcpy(pCfg->xmlVer, pPCfg->xmlVer);
	 strcpy(pCfg->xmlRev, pPCfg->xmlRev);
	 pCfg->xmlCrc = pPCfg->xmlCrc;
	 for (j=0; j<PBYXNUM; j++)
	 {
	 	pCfg->YXNO[j] = pPCfg->YXNO[j];
	 }
     memcpy(pCfg->Lan3.sIP,pPCfg->Lan3.sIP,sizeof(pPCfg->Lan3.sIP));
     pCfg->Lan3.dwMask = pPCfg->Lan3.dwMask;
     memcpy(pCfg->sGateWay3,pPCfg->sGateWay3,sizeof(pPCfg->sGateWay3));
	 pCfg->YKDelayTime = pPCfg->YKDelayTime;
	 memcpy(pCfg->Lan4.sIP,pPCfg->Lan4.sIP,sizeof(pPCfg->Lan4.sIP));
     pCfg->Lan4.dwMask = pPCfg->Lan4.dwMask;
     memcpy(pCfg->sGateWay4,pPCfg->sGateWay4,sizeof(pPCfg->sGateWay4));
	 memcpy(pCfg->sRoute3, pPCfg->sRoute3, sizeof(pPCfg->sRoute3));
	 memcpy(pCfg->sRoute4, pPCfg->sRoute4, sizeof(pPCfg->sRoute4));
}

void GetFaFdExtCfg(void)
{
    int i,j,fdnum;
    BYTE *pPFdNum = (BYTE*)(g_pParafile+1);
	BYTE *pPFdPrExtNum = pPFdNum+1;
	VFdCfg *pCfg = g_Sys.MyCfg.pFd;
	struct VPFdExtConfig *pPFdExtCfg = (struct VPFdExtConfig*)(pPFdPrExtNum+1);
	VFdCfg *pFdPrExtCfg;
	struct VPFdCfg *pPFdPrExtCfg;

	fdnum = *pPFdNum;

    for (i=0; i<fdnum ; i++)
    {
        for (j=0; j<8; j++)
        {
            pCfg[i].kg_fabsno[j] = pPFdExtCfg[i].kg_fabsno[j];
			pCfg[i].fabs_eqpid[j] = pPFdExtCfg[i].fabs_eqpid[j];
        }
    }
	
   	g_Sys.MyCfg.pFaPrFd = malloc(*pPFdPrExtNum*sizeof(VFdCfg));
	if (g_Sys.MyCfg.pFaPrFd == NULL) return;

	pFdPrExtCfg = g_Sys.MyCfg.pFaPrFd;
	pPFdPrExtCfg = (struct VPFdCfg*)(pPFdExtCfg+*pPFdNum+*pPFdPrExtNum);
	for (i=0; i<*pPFdPrExtNum; i++)
	{
	    pFdPrExtCfg[i].pt = pPFdPrExtCfg[i].pt;
		pFdPrExtCfg[i].ct = pPFdPrExtCfg[i].ct;
		pFdPrExtCfg[i].Un = pPFdPrExtCfg[i].Un;
		pFdPrExtCfg[i].In = pPFdPrExtCfg[i].In;
		for (j=0; j<8; j++)
		{
		    pFdPrExtCfg[i].kg_fabsno[j] = pPFdExtCfg[fdnum+i].kg_fabsno[j];
		    pFdPrExtCfg[i].fabs_eqpid[j] = pPFdExtCfg[fdnum+i].fabs_eqpid[j];
		}
	}
}

void SetDefFaFdExtCfg()
{
    int i,j;
	VFdCfg *pCfg = g_Sys.MyCfg.pFd;
	
    for (i=0; i<g_Sys.MyCfg.wFDNum; i++)
    {
        for (j=0; j<8; j++)
			pCfg[i].kg_fabsno[j] = -1;
    }
	g_Sys.MyCfg.pFaPrFd = NULL;
}

void SetDefSysExtConfig(void)
{
     struct VSysCfgExt *pCfg = &g_Sys.MyCfg.SysExt;

	 strcpy(pCfg->sWsName, "PDTS_DTU_1");
	 strcpy(pCfg->sWsSechema, "PeiwangWeb/services/PeiwangService");
	 pCfg->dWsIp = 0xC0A80563;
	 pCfg->dWsVer = 0;

	 sprintf(pCfg->sMaintIp, "0.0.0.0");

	 memset(pCfg->byMac, 0, 6);
	 pCfg->dwKgTime = 0;
#if (DEV_SP == DEV_SP_DTU_IU)
	strcpy(pCfg->Lan3.sIP,"192.168.1.100");
	pCfg->Lan3.dwMask = 0xFFFFFF00;
	strcpy(pCfg->sGateWay3,"192.168.1.1");
	
	strcpy(pCfg->Lan4.sIP,"0.0.0.0");
	pCfg->Lan4.dwMask = 0;
	strcpy(pCfg->sGateWay4,"0.0.0.0");
#else
	strcpy(pCfg->Lan3.sIP,"0.0.0.0");
	pCfg->Lan3.dwMask = 0;
	strcpy(pCfg->sGateWay3,"0.0.0.0");

	strcpy(pCfg->Lan4.sIP,"0.0.0.0");
	pCfg->Lan4.dwMask = 0;
	strcpy(pCfg->sGateWay4,"0.0.0.0");	
#endif
}
void AddrExtInit()
{
  struct VSysCfgExt *pCfg = &g_Sys.MyCfg.SysExt;
  char *ip,*gateway, *route;
  DWORD mask = 0;
  BYTE *ip_mask;
  char smask[20];
	Veeprom_ipext_mac ip_mac;
	int set = 0;
  ip = pCfg->Lan3.sIP;
  mask = pCfg->Lan3.dwMask;
  gateway= pCfg->sGateWay3;
  if(mask == 0)
    return;
}

/*------------------------------------------------------------------------
 Procedure:     SysConfigInit ID:1
 Purpose:       系统配置初始化
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/     
void SysConfigInit(void)
{
	if (ReadParaFile("system.cfg", (BYTE *)g_pParafile, MAXFILELEN) == OK)
	{
	    GetSysConfig();
	}
	else
	{
	    SetDefSysConfig();
	}

    g_Sys.dwAioType = GetAiType();
	g_Sys.dwDioType = GetIoType();
	g_Sys.byCpldVer = CPLD_Ver();

	myprintf(SYS_ID, LOG_ATTR_INFO, "AIO BackType:%x. AIO ParaType:%x", g_Sys.dwAioType, g_Sys.MyCfg.dwAIType);
	myprintf(SYS_ID, LOG_ATTR_INFO, "DIO BackType:%x. DIO ParaType:%x", g_Sys.dwDioType, g_Sys.MyCfg.dwIOType);    
	myprintf(SYS_ID, LOG_ATTR_INFO, "CPLD Ver:%x.%x", (g_Sys.byCpldVer>>4)&0xf, g_Sys.byCpldVer&0xf);

#ifdef INCLUDE_CELL
	if (ReadParaFile("cell.cfg", (BYTE *)g_pParafile, MAXFILELEN) == OK)
		GetCellConfig();
	else
		SetDefCellConfig();
#endif	

#ifdef INCLUDE_USWITCH
	if (ReadParaFile("uswitch.cfg", (BYTE *)g_pParafile, MAXFILELEN) == OK)
		GetUSwitchConfig();
	else
		SetDefUSwitchConfig();
#endif	

	if (ReadParaFile("runpara.cfg", (BYTE *)g_pParafile, MAXFILELEN) == OK)
		GetRunParaConfig();
	else
		SetRunParaConfig();

    if (ReadParaFile("systemext.cfg", (BYTE *)g_pParafile, MAXFILELEN) == OK)
		GetSysExtConfig();
	else
		SetDefSysExtConfig();
	


    if (ReadParaFile("fafdExt.cfg", (BYTE *)g_pParafile, MAXFILELEN) == OK)
		GetFaFdExtCfg();
	else
		SetDefFaFdExtCfg();
}

#if (TYPE_IO != IO_TYPE_LOCAL)	

void ExtIoConfigInit(void)
{
#if (TYPE_IO == IO_TYPE_MASTER)	
	g_Sys.dwExtIoSyncSem = smBCreate();   
	g_Sys.dwExtIoCmdSem = smBCreate();   
	g_Sys.dwExtIoDataSem = smMCreate();
	g_Sys.wExtIoNum = 0;

	SpecialTaskSetup(BUS_CAN_ID);
	
	if (ReadParaFile("extio.cfg",(BYTE *)g_pParafile,MAXFILELEN) == OK)
		GetExtIoConfig(0);
	else
		SetDefExtIoConfig();	
#endif


#if (TYPE_IO == IO_TYPE_EXTIO)
 	g_Sys.ExtIoAddr.dwType = BACK_TYPE_IPACS|GetAppDioType(EXT_TYPE_DIO_INPUT())|GetAppAioType(EXT_TYPE_AIO_INPUT());
	g_Sys.AddrInfo.wAddr = g_Sys.ExtIoAddr.wAddr;
	g_Sys.AddrInfo.Lan1 = g_Sys.ExtIoAddr.Lan;
	g_Sys.dwExtIoRcdSem = smBCreate();   
	g_Sys.dwExtIoDataSem = smMCreate();


	extiosInit();
#endif
}

#endif

int CheckAddr(void)
{
#if ((TYPE_IO == IO_TYPE_EXTIO) || (TYPE_IO == IO_TYPE_MASTER))
	int addr;

	//读出一个稳定的地址,说明插件已插好
	addr = readAddr();
	for (;;)
	{
		thSleep(100);
		if (addr == readAddr())
			break;
		addr = readAddr();
	}
#endif

#if (TYPE_IO == IO_TYPE_MASTER)
	if ((addr != 0x0) && (addr != 0x1))
	{
		WriteWarnEvent(NULL, SYS_ERR_ADDR, 0, "主板插槽错误或程序不匹配");			
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Master's addr error!");
		return ERROR;
	}
#endif

#if (TYPE_IO == IO_TYPE_EXTIO)
	if ((addr == 0x1F) || (addr == 0x0) || (addr == 0x1))
	{
		WriteWarnEvent(NULL, SYS_ERR_ADDR, 0, "分板插槽错误或程序不匹配");			
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Slave's addr error!");
		return ERROR;
	}
#endif

    return OK;
}


void GetSysAddr(void)
{
    //read bsp addr
    //com must be drivered for read addr from mmi

	struct VPAddr *pAddr=(struct VPAddr *)(g_pParafile+1);
	memcpy(&g_Sys.AddrInfo, pAddr, sizeof(struct VPAddr));	
}	


/*------------------------------------------------------------------------
 Procedure:     SetDefSysAddr ID:1
 Purpose:       设置缺省系统地址
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void SetDefSysAddr(void)
{
	myprintf(SYS_ID, LOG_ATTR_INFO, "Create default addr.cfg.");

	CreateSysAddr(NULL, &g_Sys.AddrInfo);
}

void AddrInit(void)
{
	char *ip1, *ip2, *gateway1, *gateway2, *saddr, addrmask1[20], addrmask2[20];
	DWORD mask1, mask2;
	WORD addr;
	DWORD ip1mask,ip2mask;
 	struct VPAddr *pAddr = &g_Sys.AddrInfo;
	char ipaddr0[20];
	char ipaddr1[20];
	memset(ipaddr0,0,20);
	memset(ipaddr1,0,20);


	if (ReadParaFile("addr.cfg", (BYTE *)g_pParafile, MAXFILELEN)==OK)
	{
		GetSysAddr();	

	}	
	else
		SetDefSysAddr();	

	saddr = pAddr->sExtAddr;
	addr = pAddr->wAddr;

    ip1 = pAddr->Lan1.sIP;
	mask1 = pAddr->Lan1.dwMask;

    ip2 = pAddr->Lan2.sIP;
	mask2 = pAddr->Lan2.dwMask;

	gateway1 = pAddr->sGateWay1;
	gateway2 = pAddr->sGateWay2;
	
	if ((saddr != NULL) && (saddr[0] != '\0'))
		myprintf(SYS_ID, LOG_ATTR_INFO, "My addr is %s.", saddr);
	else
		myprintf(SYS_ID, LOG_ATTR_INFO, "My addr is %d.", addr);


	sprintf(addrmask1,"%d.%d.%d.%d",mask1>>24,(mask1>>16) & 0xff,(mask1 >> 8) & 0xff,mask1 & 0xff);
	sprintf(addrmask2,"%d.%d.%d.%d",mask2>>24,(mask2>>16) & 0xff,(mask2 >> 8) & 0xff,mask2 & 0xff);
	
	ESDK_SetNetInfo(0, ip1, addrmask1, gateway1);
	ESDK_SetNetInfo(1, ip2, addrmask2, gateway2);


	if(ESDK_GetNetAddr(0,ipaddr0) != 0)
	{
		myprintf(SYS_ID, LOG_ATTR_INFO, "eth0 ip get failed.");
		return;
	}
	myprintf(SYS_ID, LOG_ATTR_INFO, "Lan1 IP is %s.", ipaddr0);

	if(ESDK_GetNetAddr(1,ipaddr1) != 0)
	{
		myprintf(SYS_ID, LOG_ATTR_INFO, "eth1 ip get failed.");
		return;
	}
	myprintf(SYS_ID, LOG_ATTR_INFO, "Lan2 IP is %s.", ipaddr1);

	myprintf(SYS_ID, LOG_ATTR_INFO, "Lan1 Mask %s ok.", addrmask1);
	myprintf(SYS_ID, LOG_ATTR_INFO, "Lan2 Mask %s ok.", addrmask2);

}  

STATUS CheckMinEqpSystem(WORD wEqpNum, const struct VPEqp *pPEqp)
{
	int i;
	BOOL bModifyFile;
	struct VPTECfg *pPTECfg;
	char name[MAXFILENAME];


	for(i=0; i<wEqpNum; i++)
	{
		if (pPEqp->wCommID == SELF_DIO_ID)
			break;
		pPEqp++;
	}  

	if (i==wEqpNum)  //未找到自身采集数据库
		return(ERROR);
	else
	{
		strcpy(name,"myio.cde");
		if (ReadParaFile(name,(BYTE *)g_pParafile,MAXFILELEN)==ERROR)
			return(ERROR);
		else
		{
			pPTECfg=(struct VPTECfg *)(g_pParafile+1);

			//自身采集数据库是否匹配 
			bModifyFile=FALSE;

			if (pPTECfg->wSourceAddr != g_Sys.AddrInfo.wAddr)
				bModifyFile=TRUE;
			if (pPTECfg->wDesAddr != g_Sys.AddrInfo.wAddr)
				bModifyFile=TRUE;

			if (bModifyFile==TRUE)
			{
				pPTECfg->wSourceAddr = pPTECfg->wDesAddr = g_Sys.AddrInfo.wAddr; 
				if (WriteParaFile(name, g_pParafile)==ERROR)  return(ERROR);
			}
		}
	}  

	return(OK); 
}

void CheckEqpInfo(void)
{
	struct VPEqp *pVPEqp;
	WORD wEqpNum;

	if (ReadParaFile("device.cfg",(BYTE *)g_pParafile,MAXFILELEN)==ERROR)
		CreateMinEqpSystme(NULL);
    else 
    {
		wEqpNum=(WORD)(((DWORD)g_pParafile->nLength-sizeof(struct VFileHead))/sizeof(struct VPEqp));
		pVPEqp=(struct VPEqp *)(g_pParafile+1);
		if (CheckMinEqpSystem(wEqpNum,pVPEqp)==ERROR)
			CreateMinEqpSystme(NULL);
	}	
}

STATUS GetDevInfo(void)
{
	struct VPEqp *pVPEqp;
	struct VEqpInfo *pInfo;	
	WORD wEqpNum, wCommID;
	int i,index;


	MemoryMalloc((void **)&(g_Sys.Eqp.pwNum),sizeof(WORD),NVRAM);

	CheckEqpInfo();

	if (ReadParaFile("device.cfg",(BYTE *)g_pParafile,MAXFILELEN)==ERROR) 
	{
		*g_Sys.Eqp.pwNum=0;  
		return(ERROR);
	}      

	wEqpNum=(g_pParafile->nLength-sizeof(struct VFileHead))/sizeof(struct VPEqp);
	pVPEqp=(struct VPEqp *)(g_pParafile+1);
	if (*g_Sys.Eqp.pwNum!=wEqpNum)
	{
		*g_Sys.Eqp.pwNum=wEqpNum;
		*g_Sys.pdwRestart=COLDRESTART;
	} 

	
	MemoryMalloc((void **)&g_Sys.Eqp.pInfo,(wEqpNum+1)*sizeof(struct VEqpInfo),NVRAM);
	pInfo = g_Sys.Eqp.pInfo;
	index = 0;
	for (i=0; i<*g_Sys.Eqp.pwNum; i++)
	{
		if (*g_Sys.pdwRestart!=COLDRESTART)
		{
			if (pInfo->dwName!=pVPEqp->dwName)
				*g_Sys.pdwRestart=COLDRESTART;
			else if (strcpy(pInfo->sCfgName,pVPEqp->sCfgName))
				*g_Sys.pdwRestart=COLDRESTART;
		}  
		pInfo->wID=i;
		pInfo->dwName=pVPEqp->dwName;
		if ((pInfo->dwName & (CFG_ATTR_TABLE<<12)) == (CFG_ATTR_TABLE<<12))
			index++;
		strcpy(pInfo->sCfgName, pVPEqp->sCfgName);
		wCommID = GetAppPortId(pVPEqp->wCommID);
		if (wCommID>THREAD_MAX_NUM)
		{
			*g_Sys.Eqp.pwNum=0;
			WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "参数错误");			
			myprintf(SYS_ID, LOG_ATTR_WARN, "device.cfg: invalid commid at dev %s!", pInfo->sCfgName);			
			return(ERROR);
		}
		else
		{
			pInfo->wCommID=wCommID; 
			pInfo->wTaskID=wCommID;
		}     

		pInfo++;
		pVPEqp++;
	}             
	return(OK); 
}  

DWORD TECfgNumCMP(const struct VTECfg *pTECfg,const struct VPTECfg *pPTECfg)
{
	if (*g_Sys.pdwRestart!=WARMRESTART)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}
	if (pTECfg->wYCNum!=pPTECfg->wYCNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}
	if (pTECfg->wVYCNum!=pPTECfg->wVYCNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}
	if (pTECfg->wDYXNum!=pPTECfg->wDYXNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}  
	if (pTECfg->wSYXNum!=pPTECfg->wSYXNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}  
	if (pTECfg->wVYXNum!=pPTECfg->wVYXNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}  
	if (pTECfg->wDDNum!=pPTECfg->wDDNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}  
	if (pTECfg->wYKNum!=pPTECfg->wYKNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}  
	if (pTECfg->wYTNum!=pPTECfg->wYTNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}
	if (pTECfg->wTSDataNum!=pPTECfg->wTSDataNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}  
	if (pTECfg->wTQNum!=pPTECfg->wTQNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}     

	return(WARMRESTART);
}   

/*------------------------------------------------------------------------
 Procedure:     InitTrueYC ID:1
 Purpose:       初始化实遥测
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitTrueYC(struct VTrueEqp *pTEqp,const struct VPTrueYC *pPYC)
{    
	WORD i,j,k,m,n,l;
	struct VTECfg *pTECfg=&pTEqp->Cfg;

	MemoryMalloc((void **)&pTEqp->pYCCfg,(pTECfg->wYCNum+pTECfg->wVYCNum)*sizeof(struct VTrueYCCfg),DRAM);
	  
	j=k=m=n=l=0;
	for (i=0;i<pTECfg->wYCNum+pTECfg->wVYCNum;i++)  
	{
		strcpy(pTEqp->pYCCfg[i].sName,pPYC[i].sName);
		strcpy(pTEqp->pYCCfg[i].sNameTab,pPYC[i].sNameTab);
		pTEqp->pYCCfg[i].dwCfg=pPYC[i].dwCfg;
		if (pTEqp->pYCCfg[i].dwCfg&0x01)  //send
			pTEqp->pYCCfg[i].wSendNo=j++;
		else
			pTEqp->pYCCfg[i].wSendNo=0xFFFF;
		if (pTEqp->pYCCfg[i].dwCfg&0x02)  //hour frz
			pTEqp->pYCCfg[i].wHourFrzNo=k++;
		else
			pTEqp->pYCCfg[i].wHourFrzNo=0xFFFF;
		if (pTEqp->pYCCfg[i].dwCfg&0x04)  //day frz
			pTEqp->pYCCfg[i].wDayFrzNo=m++;
		else
			pTEqp->pYCCfg[i].wDayFrzNo=0xFFFF;
		if (pTEqp->pYCCfg[i].dwCfg&0x08)  //month frz
			pTEqp->pYCCfg[i].wMonthFrzNo=n++;
		else
			pTEqp->pYCCfg[i].wMonthFrzNo=0xFFFF;
		if (pTEqp->pYCCfg[i].dwCfg&0x100)  //maxmin frz
			pTEqp->pYCCfg[i].wMaxMinNo=l++;
		else
			pTEqp->pYCCfg[i].wMaxMinNo=0xFFFF;
		pTEqp->pYCCfg[i].wVENum=0;
		pTEqp->pYCCfg[i].lA=pPYC[i].lA;
		pTEqp->pYCCfg[i].lB=pPYC[i].lB;
		if (pTEqp->pYCCfg[i].lB == 0) pTEqp->pYCCfg[i].lB = 1;
		pTEqp->pYCCfg[i].lC=pPYC[i].lC;
		pTEqp->pYCCfg[i].wFrzKey=pPYC[i].wFrzKey;

		pTEqp->pYCCfg[i].lLimit1=pPYC[i].lLimit1;
		pTEqp->pYCCfg[i].dwLimitT1=pPYC[i].wLimitT1*1000;
		pTEqp->pYCCfg[i].wLimit1VYxNo=pPYC[i].wLimit1VYxNo;
		pTEqp->pYCCfg[i].lLimit2=pPYC[i].lLimit2;
		pTEqp->pYCCfg[i].dwLimitT2=pPYC[i].wLimitT2*1000;
		pTEqp->pYCCfg[i].wLimit2VYxNo=pPYC[i].wLimit2VYxNo;		

		pTEqp->pYCCfg[i].tycfdnum = 0xFF;
		pTEqp->pYCCfg[i].tyctype = 0xFF;
		memset(pTEqp->pYCCfg[i].tycunit,0,sizeof(pTEqp->pYCCfg[i].tycunit));
	}

	pTEqp->YCSendCfg.wNum=j;
	MemoryMalloc((void **)&pTEqp->YCSendCfg.pwIndex,j*sizeof(WORD),DRAM);

	pTEqp->YCHourFrz.wNum=k;
	MemoryMalloc((void **)&pTEqp->YCHourFrz.pwIndex,k*sizeof(WORD),DRAM);

	pTEqp->YCDayFrz.wNum=m;
	MemoryMalloc((void **)&pTEqp->YCDayFrz.pwIndex,m*sizeof(WORD),DRAM);

	pTEqp->YCMonthFrz.wNum=n;
	MemoryMalloc((void **)&pTEqp->YCMonthFrz.pwIndex,n*sizeof(WORD),DRAM);

	pTECfg->wYCMaxMinNum=l;
	MemoryMalloc((void **)&pTEqp->pMaxMinYC,l*sizeof(struct VMaxMinYC),NVRAM);
    for (i=0; i<l; i++)
    {
		pTEqp->pMaxMinYC[i].lMax = 0;
		GetSysClock(&pTEqp->pMaxMinYC[i].max_tm, CALCLOCK);
		pTEqp->pMaxMinYC[i].lMin = 0x7FFFFFFF;
		GetSysClock(&pTEqp->pMaxMinYC[i].min_tm, CALCLOCK);
    }

	j=k=m=n=0;
	for (i=0;i<pTECfg->wYCNum+pTECfg->wVYCNum;i++)
	{
		if (pTEqp->pYCCfg[i].dwCfg&0x01)  //send
			pTEqp->YCSendCfg.pwIndex[j++]=i;
		if (pTEqp->pYCCfg[i].dwCfg&0x02)  //hour frz
			pTEqp->YCHourFrz.pwIndex[k++]=i;
		if (pTEqp->pYCCfg[i].dwCfg&0x04)  //day frz
			pTEqp->YCDayFrz.pwIndex[m++]=i;
		if (pTEqp->pYCCfg[i].dwCfg&0x08)  //month frz
			pTEqp->YCMonthFrz.pwIndex[n++]=i;
	}
		
	for (i=0;i<pTECfg->wYCNum+pTECfg->wVYCNum;i++)  //if nvram then nValue=0
	{
		pTEqp->pYC[i].byFlag=1;
	}	
}

/*------------------------------------------------------------------------
 Procedure:     InitTrueDYX ID:1
 Purpose:       初始化实装置双点遥信
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitTrueDYX(struct VTrueEqp *pTEqp, const struct VPTrueYX *pPDYX)
{
	WORD i,j;
	struct VTECfg *pTECfg=&pTEqp->Cfg;

	MemoryMalloc((void **)&pTEqp->pDYXCfg,pTECfg->wDYXNum*sizeof(struct VTrueYXCfg),DRAM);

	j=0;
	for (i=0;i<pTECfg->wDYXNum;i++)
	{
		strcpy(pTEqp->pDYXCfg[i].sName,pPDYX[i].sName);
		strcpy(pTEqp->pDYXCfg[i].sNameTab,pPDYX[i].sNameTab);
		pTEqp->pDYXCfg[i].dwCfg=pPDYX[i].dwCfg;
		if (pTEqp->pDYXCfg[i].dwCfg&0x01)
			pTEqp->pDYXCfg[i].wSendNo=j++;
		else
			pTEqp->pDYXCfg[i].wSendNo=0xFFFF;
		pTEqp->pDYXCfg[i].wVENum=0;
	}

	pTEqp->DYXSendCfg.wNum=j;
	MemoryMalloc((void **)&pTEqp->DYXSendCfg.pwIndex,j*sizeof(WORD),DRAM);        

	j=0;
	for (i=0;i<pTECfg->wDYXNum;i++)
	     if (pTEqp->pDYXCfg[i].dwCfg&0x01)
	         pTEqp->DYXSendCfg.pwIndex[j++]=i;

	if (*g_Sys.pdwRestart!=WARMRESTART)
	{
		*g_Sys.pdwRestart=COLDRESTART;

		for (i=0;i<pTECfg->wDYXNum;i++)
		{
			pTEqp->pDYX[i].byValue1=0x01;
			pTEqp->pDYX[i].byValue2=0x01;
		} 
		if (pTECfg->wDYXNum)
		{
			pTEqp->DSOE.wWritePtr=0;    
			memset(pTEqp->DSOE.pPoolHead,0,pTEqp->DSOE.wPoolNum*sizeof(struct VDBDSOE));

			pTEqp->DCOS.wWritePtr=0;    
			memset(pTEqp->DCOS.pPoolHead,0,pTEqp->DCOS.wPoolNum*sizeof(struct VDBDCOS));
		} 
	} 
}

/*------------------------------------------------------------------------
 Procedure:     InitTrueSYX ID:1
 Purpose:       初始化实装置单点遥信
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitTrueSYX(struct VTrueEqp *pTEqp, const struct VPTrueYX *pPSYX)
{
	WORD i,j;
	struct VTECfg *pTECfg=&pTEqp->Cfg;

	MemoryMalloc((void **)&pTEqp->pSYXCfg,(pTECfg->wSYXNum+pTECfg->wVYXNum)*sizeof(struct VTrueYXCfg),DRAM);

	j=0;
	for (i=0;i<(pTECfg->wSYXNum+pTECfg->wVYXNum);i++)
	{
		strcpy(pTEqp->pSYXCfg[i].sName,pPSYX[i].sName);
		strcpy(pTEqp->pSYXCfg[i].sNameTab,pPSYX[i].sNameTab);
		pTEqp->pSYXCfg[i].dwCfg=pPSYX[i].dwCfg;

		pTEqp->pSYXCfg[i].dwCfg |= 0x01;  //所有遥信强行发送
			
		if (pTEqp->pSYXCfg[i].dwCfg&0x01)
			pTEqp->pSYXCfg[i].wSendNo=j++;
		else
			pTEqp->pSYXCfg[i].wSendNo=0xFFFF;
		pTEqp->pSYXCfg[i].wVENum=0;
	}

	pTEqp->SYXSendCfg.wNum=j;
	MemoryMalloc((void **)&pTEqp->SYXSendCfg.pwIndex,j*sizeof(WORD),DRAM);        

	j=0;
	for (i=0;i<(pTECfg->wSYXNum+pTECfg->wVYXNum);i++)
		if (pTEqp->pSYXCfg[i].dwCfg&0x01)
			pTEqp->SYXSendCfg.pwIndex[j++]=i;

	if (pTECfg->wSYXNum+pTECfg->wVYXNum)
		pTEqp->pSYX[pTECfg->wSYXNum+pTECfg->wVYXNum-1].byValue=0x01;  //commstatus

	if (*g_Sys.pdwRestart!=WARMRESTART)
	{
		*g_Sys.pdwRestart=COLDRESTART;

		for (i=0;i<(pTECfg->wSYXNum+pTECfg->wVYXNum);i++)
			pTEqp->pSYX[i].byValue=0x01;

		if (pTECfg->wSYXNum+pTECfg->wVYXNum)
		{
			pTEqp->SSOE.wWritePtr=0;    
			memset(pTEqp->SSOE.pPoolHead,0,pTEqp->SSOE.wPoolNum*sizeof(struct VDBSOE));

			pTEqp->SCOS.wWritePtr=0;    
			memset(pTEqp->SCOS.pPoolHead,0,pTEqp->SCOS.wPoolNum*sizeof(struct VDBCOS));
		} 
	} 
}

/*------------------------------------------------------------------------
 Procedure:     InitTrueDD ID:1
 Purpose:       初始化实装置电度
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitTrueDD(struct VTrueEqp *pTEqp, const struct VPTrueDD *pPDD)
{    
	WORD i,j,k,m,n;
	struct VTECfg *pTECfg=&pTEqp->Cfg;

	MemoryMalloc((void **)&pTEqp->pDDCfg,pTECfg->wDDNum*sizeof(struct VTrueDDCfg),DRAM);

	j=k=m=n=0;
	for (i=0;i<pTECfg->wDDNum;i++)
	{
		strcpy(pTEqp->pDDCfg[i].sName,pPDD[i].sName);
		strcpy(pTEqp->pDDCfg[i].sNameTab,pPDD[i].sNameTab);
		pTEqp->pDDCfg[i].dwCfg=pPDD[i].dwCfg;
		if (pTEqp->pDDCfg[i].dwCfg&0x01)
			pTEqp->pDDCfg[i].wSendNo=j++;
		else
			pTEqp->pDDCfg[i].wSendNo=0xFFFF;
		if (pTEqp->pDDCfg[i].dwCfg&0x02)  //hour frz
			pTEqp->pDDCfg[i].wHourFrzNo=k++;
		else
			pTEqp->pDDCfg[i].wHourFrzNo=0xFFFF;
		if (pTEqp->pDDCfg[i].dwCfg&0x04)  //day frz
			pTEqp->pDDCfg[i].wDayFrzNo=m++;
		else
			pTEqp->pDDCfg[i].wDayFrzNo=0xFFFF;
		if (pTEqp->pDDCfg[i].dwCfg&0x08)  //month frz
			pTEqp->pDDCfg[i].wMonthFrzNo=n++;
		else
			pTEqp->pDDCfg[i].wMonthFrzNo=0xFFFF;
		pTEqp->pDDCfg[i].wVENum=0;
		pTEqp->pDDCfg[i].lA=pPDD[i].lA;
		pTEqp->pDDCfg[i].lB=pPDD[i].lB;
		pTEqp->pDDCfg[i].lC=pPDD[i].lC;
		pTEqp->pDDCfg[i].wFrzKey=pPDD[i].wFrzKey;
	}

	pTEqp->DDSendCfg.wNum=j;
	MemoryMalloc((void **)&pTEqp->DDSendCfg.pwIndex,j*sizeof(WORD),DRAM);        

	pTEqp->DDHourFrz.wNum=k;
	MemoryMalloc((void **)&pTEqp->DDHourFrz.pwIndex,k*sizeof(WORD),DRAM);

	pTEqp->DDDayFrz.wNum=m;
	MemoryMalloc((void **)&pTEqp->DDDayFrz.pwIndex,m*sizeof(WORD),DRAM);

	pTEqp->DDMonthFrz.wNum=n;
	MemoryMalloc((void **)&pTEqp->DDMonthFrz.pwIndex,n*sizeof(WORD),DRAM);

	j=k=m=n=0;
	for (i=0;i<pTECfg->wDDNum;i++)
	{
		if (pTEqp->pDDCfg[i].dwCfg&0x01)
			pTEqp->DDSendCfg.pwIndex[j++]=i;		
		if (pTEqp->pDDCfg[i].dwCfg&0x02)  //hour frz
			pTEqp->DDHourFrz.pwIndex[k++]=i;
		if (pTEqp->pDDCfg[i].dwCfg&0x04)  //day frz
			pTEqp->DDDayFrz.pwIndex[m++]=i;
		if (pTEqp->pDDCfg[i].dwCfg&0x08)  //month frz
			pTEqp->DDMonthFrz.pwIndex[n++]=i;
	}
	
	for (i=0;i<pTECfg->wDDNum;i++)  //if nvram then dwValue=0
		pTEqp->pDD[i].byFlag=1;
}

/*------------------------------------------------------------------------
 Procedure:     InitTrueYK ID:1
 Purpose:       初始化实装置遥控
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitTrueYK(struct VTrueEqp *pTEqp, const struct VPTrueCtrl *pPYK)
{    
	WORD i;
	struct VTECfg *pTECfg=&pTEqp->Cfg;
	  
	MemoryMalloc((void **)&pTEqp->pYK,pTECfg->wYKNum*sizeof(struct VTrueCtrl),DRAM);
	MemoryMalloc((void **)&pTEqp->pYKCfg,pTECfg->wYKNum*sizeof(struct VTrueCtrlCfg),DRAM);

	for (i=0;i<pTECfg->wYKNum;i++)
		pTEqp->pYK[i].wID=pPYK[i].wID;

	for (i=0;i<pTECfg->wYKNum;i++)
	{
		strcpy(pTEqp->pYKCfg[i].sName,pPYK[i].sName);
		strcpy(pTEqp->pYKCfg[i].sNameTab,pPYK[i].sNameTab);
		pTEqp->pYKCfg[i].dwCfg=pPYK[i].dwCfg;
		pTEqp->pYKCfg[i].byFlag=1;
		pTEqp->pYKCfg[i].byTaskID=0;
	}  
}

/*------------------------------------------------------------------------
 Procedure: 	InitTrueYT ID:1
 Purpose:		初始化实装置遥调
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitTrueYT(struct VTrueEqp *pTEqp, const struct VPTrueCtrl *pPYT)
{	 
	WORD i;
	struct VTECfg *pTECfg=&pTEqp->Cfg;
	  
	MemoryMalloc((void **)&pTEqp->pYT,pTECfg->wYTNum*sizeof(struct VTrueCtrl),DRAM);
	MemoryMalloc((void **)&pTEqp->pYTCfg,pTECfg->wYTNum*sizeof(struct VTrueCtrlCfg),DRAM);

	for (i=0;i<pTECfg->wYTNum;i++)
		pTEqp->pYT[i].wID=pPYT[i].wID;

	for (i=0;i<pTECfg->wYTNum;i++)
	{
		strcpy(pTEqp->pYTCfg[i].sName,pPYT[i].sName);
		strcpy(pTEqp->pYTCfg[i].sNameTab,pPYT[i].sNameTab);
		pTEqp->pYTCfg[i].dwCfg=pPYT[i].dwCfg;
		pTEqp->pYTCfg[i].byFlag=1;
		pTEqp->pYTCfg[i].byTaskID=0;
	}  
}

/*------------------------------------------------------------------------
 Procedure:     InitTSData ID:1
 Purpose:       初始化实装置透明数据
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitTSData(struct VTrueEqp *pTEqp, struct VPTSDataCfg *pPTSData)
{    
	WORD i,j;
	struct VTECfg *pTECfg=&pTEqp->Cfg;

	MemoryMalloc((void **)&pTEqp->pTSDataCfg,pTECfg->wTSDataNum*sizeof(struct VTSDataCfg),DRAM);

	for (i=0;i<pTECfg->wTSDataNum;i++)
	{
		strcpy(pTEqp->pTSDataCfg[i].sName,pPTSData[i].sName);
		strcpy(pTEqp->pTSDataCfg[i].sNameTab,pPTSData[i].sNameTab);
		pTEqp->pTSDataCfg[i].dwCfg=pPTSData[i].dwCfg;     		
		pTEqp->pTSDataCfg[i].wKeyWordNum=pPTSData[i].wKeyWordNum;
		for (j=0;j<14;j++)
		{       
			pTEqp->pTSDataCfg[i].abyKeyWord[j]=pPTSData[i].abyKeyWord[j];
			pTEqp->pTSDataCfg[i].awOffset[j]=pPTSData[i].awOffset[j];
		} 
		pTEqp->pTSDataCfg[i].wVYXNo=pPTSData[i].wVYXNo;     
	} 
}

#if 0
/*------------------------------------------------------------------------
 Procedure:     InitTSData ID:1
 Purpose:       初始化实装置透明数据
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitTSData(struct VTrueEqp *pTEqp, struct VPTSDataCfg *pPTSData)
{    
	WORD i,j;
	struct VTECfg *pTECfg=&pTEqp->Cfg;

	MemoryMalloc((void **)&pTEqp->pTSDataCfg,pTECfg->wTSDataNum*sizeof(struct VTSDataCfg),DRAM);

	for (i=0;i<pTECfg->wTSDataNum;i++)
	{
		strcpy(pTEqp->pTSDataCfg[i].sName,pPTSData[i].sName);
		strcpy(pTEqp->pTSDataCfg[i].sNameTab,pPTSData[i].sNameTab);
		pTEqp->pTSDataCfg[i].dwCfg=pPTSData[i].dwCfg;     		
		pTEqp->pTSDataCfg[i].wKeyWordNum=pPTSData[i].wKeyWordNum;
		for (j=0;j<14;j++)
		{       
			pTEqp->pTSDataCfg[i].abyKeyWord[j]=pPTSData[i].abyKeyWord[j];
			pTEqp->pTSDataCfg[i].awOffset[j]=pPTSData[i].awOffset[j];
		} 
		pTEqp->pTSDataCfg[i].wVYXNo=pPTSData[i].wVYXNo;     
	} 
}
#endif

void TrueDevInit(WORD wEqpID)
{
	struct VTECfg *pTECfg;  
	struct VTrueEqp *pTEqp;

	struct VPTECfg *pPTECfg=(struct VPTECfg *)(g_pParafile+1);
	struct VPTrueYC *pPYC=(struct VPTrueYC *)(pPTECfg+1);
	struct VPTrueYX *pPDYX=(struct VPTrueYX *)(pPYC+pPTECfg->wYCNum+pPTECfg->wVYCNum);
	struct VPTrueYX *pPSYX=(struct VPTrueYX *)(pPDYX+pPTECfg->wDYXNum);
	struct VPTrueDD *pPDD=(struct VPTrueDD *)(pPSYX+pPTECfg->wSYXNum+pPTECfg->wVYXNum);
	struct VPTrueCtrl *pPYK=(struct VPTrueCtrl *)(pPDD+pPTECfg->wDDNum);
	struct VPTrueCtrl *pPYT=(struct VPTrueCtrl *)(pPYK+pPTECfg->wYKNum);  
	struct VPTSDataCfg *pPTSData=(struct VPTSDataCfg *)(pPYT+pPTECfg->wYTNum);    

	MemoryMalloc((void **)&g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo,sizeof(struct VTrueEqp),NVRAM);

	g_Task[g_Sys.Eqp.pInfo[wEqpID].wTaskID].wEqpNum++;  

	pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo;
	pTECfg=&pTEqp->Cfg;

	/*?????? force addr*/
	pTECfg->wSourceAddr=g_Sys.AddrInfo.wAsMasterAddr;
	if (g_Sys.Eqp.pInfo[wEqpID].wTaskID == SELF_DIO_ID)  
	{
		pTECfg->wDesAddr = g_Sys.AddrInfo.wAddr;		
		g_Sys.wIOEqpID = wEqpID;
	}	
	else
		pTECfg->wDesAddr = pPTECfg->wDesAddr;

	if(g_Sys.Eqp.pInfo[wEqpID].wTaskID == FADIFF_ID)
	{
		g_Sys.wSocFaEqpID = wEqpID;
	}

	if(strstr(g_Sys.Eqp.pInfo[wEqpID].sCfgName,"linepro0"))
	{
		g_Sys.wLinePr0EqpID = wEqpID;
	}

	if(strstr(g_Sys.Eqp.pInfo[wEqpID].sCfgName,"linepro1"))
	{
		g_Sys.wLinePr1EqpID = wEqpID;
	}
	
    memcpy(pTECfg->sExtAddr, pPTECfg->sExtAddr, 12);
	memcpy(pTECfg->sTelNo, pPTECfg->sTelNo, 20);
	
	pTECfg->dwFlag=pPTECfg->dwFlag;
	pTECfg->wInvRefEGNum=0;
	if (TECfgNumCMP(pTECfg,pPTECfg)==COLDRESTART)
	{     
		pTECfg->wYCNum=pPTECfg->wYCNum;
		pTECfg->wVYCNum=pPTECfg->wVYCNum;
		pTECfg->wDYXNum=pPTECfg->wDYXNum;
		pTECfg->wSYXNum=pPTECfg->wSYXNum;
		pTECfg->wVYXNum=pPTECfg->wVYXNum;
		pTECfg->wDDNum=pPTECfg->wDDNum;
		pTECfg->wYKNum=pPTECfg->wYKNum;
		pTECfg->wYTNum=pPTECfg->wYTNum;
		pTECfg->wTSDataNum=pPTECfg->wTSDataNum;
		pTECfg->wTQNum=pPTECfg->wTQNum;

		pTECfg->byYCFrzMD=pPTECfg->byYCFrzMD;
		pTECfg->byYCFrzCP=pPTECfg->byYCFrzCP;
		pTECfg->byDDFrzMD=pPTECfg->byDDFrzMD;
		pTECfg->byDDFrzCP=pPTECfg->byDDFrzCP;
	}

	MemoryMalloc((void **)&pTEqp->pYC,(pTECfg->wYCNum+pTECfg->wVYCNum)*sizeof(struct VTrueYC),DRAM);

	MemoryMalloc((void **)&pTEqp->pDYX,pTECfg->wDYXNum*sizeof(struct VTrueDYX),NVRAM);
	PoolMalloc((struct VPool *)&pTEqp->DSOE,pTECfg->wDYXNum,TRUEEQPDSOENUM,sizeof(struct VDBDSOE));
	PoolMalloc((struct VPool *)&pTEqp->DCOS,pTECfg->wDYXNum,TRUEEQPDCOSNUM,sizeof(struct VDBDCOS));

	MemoryMalloc((void **)&pTEqp->pSYX,(pTECfg->wSYXNum+pPTECfg->wVYXNum)*sizeof(struct VTrueYX),NVRAM);
	PoolMalloc((struct VPool *)&pTEqp->SSOE,pTECfg->wSYXNum+pTECfg->wVYXNum,TRUEEQPSSOENUM,sizeof(struct VDBSOE));
	PoolMalloc((struct VPool *)&pTEqp->SCOS,pTECfg->wSYXNum+pTECfg->wVYXNum,TRUEEQPSCOSNUM,sizeof(struct VDBCOS));

	MemoryMalloc((void **)&pTEqp->pDD,pTECfg->wDDNum*sizeof(struct VTrueDD),DRAM);

	InitTrueDYX(pTEqp,pPDYX);  
	InitTrueSYX(pTEqp,pPSYX);

	InitTrueYC(pTEqp,pPYC);
	InitTrueDD(pTEqp,pPDD);
	InitTrueYK(pTEqp,pPYK);
	InitTrueYT(pTEqp,pPYT);
	InitTSData(pTEqp,pPTSData);    

#ifdef INCLUDE_B2F_SOE
    /*必须放在最后,否则数据项初始化会清空soe*/
	PoolMalloc_B2F((struct VPool *)&pTEqp->DSOE,pTECfg->wDYXNum,TRUEEQPDSOENUM,sizeof(struct VDBDSOE),wEqpID,SYSEV_FLAG_DSOE);
	PoolMalloc_B2F((struct VPool *)&pTEqp->SSOE,pTECfg->wSYXNum+pTECfg->wVYXNum,TRUEEQPSSOENUM,sizeof(struct VDBSOE),wEqpID,SYSEV_FLAG_SSOE);
#endif	
#ifdef INCLUDE_B2F_COS
    /*必须放在最后,否则数据项初始化会清空cos*/
	PoolMalloc_B2F((struct VPool *)&pTEqp->SCOS,pTECfg->wSYXNum+pTECfg->wVYXNum,TRUEEQPSCOSNUM,sizeof(struct VDBCOS),wEqpID,SYSEV_FLAG_SCOS);
#endif	
}

/*------------------------------------------------------------------------
 Procedure:     VYCParaCheck ID:1
 Purpose:       虚装置遥测参数检查
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS VYCParaCheck(WORD wEqpID, const struct VPVirtualYC *pPara, WORD Num)
{
	WORD i,j,wTEqpID;

	j=0;
	for (i=0;i<Num;i++)
	{
		/*if (pPara[i].wSendNo==j)
			j++;
		else if (pPara[i].wSendNo!=(j-1))
		{
			if (pPara[i].wSendNo!=i)
			{
				myprintf(SYS_ID, LOG_ATTR_ERROR, "%s YC sendno invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
				return(ERROR);
			} 
			else
				j=i+1;
		}*/
		if(i == 0)
			j = pPara[i].wSendNo;
		else
		{
			if(pPara[i].wSendNo < j)
			{
				myprintf(SYS_ID, LOG_ATTR_ERROR, "%s YC sendno invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
				return(ERROR);
			}
			else
				j = pPara[i].wSendNo;
		} 
	}

	for (i=0;i<Num;i++)
	{
		if (GetEqpID(pPara[i].dwTEName,&wTEqpID)==ERROR)
			break;
		if (g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo==NULL)
			break;
		if (pPara[i].wOffset>=(g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo->Cfg.wYCNum+g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo->Cfg.wVYCNum))
			break;
	}

	if (i<Num)
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "%s YC index invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
		return(ERROR);
	} 

	return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     VDYXParaCheck ID:1
 Purpose:       虚装置双点遥信参数检查
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS VDYXParaCheck(WORD wEqpID, const struct VPVirtualYX *pPara, WORD Num)
{
	WORD i,j,wTEqpID;

	j=0;
	for (i=0;i<Num;i++)
	{
		/*if (pPara[i].wSendNo==j)
			j++;
		else if (pPara[i].wSendNo!=(j-1))
		{
			if (pPara[i].wSendNo!=i)
			{
				myprintf(SYS_ID, LOG_ATTR_ERROR, "%s DYX sendno invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
				return(ERROR);
			} 
			else
				j=i+1;
		}*/ 
		if (i == 0)
			j =  pPara[i].wSendNo;
		else
		{
			if (pPara[i].wSendNo < j)
			{
				myprintf(SYS_ID, LOG_ATTR_ERROR, "%s DYX sendno invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
				return(ERROR);
			} 
			else
				j = pPara[i].wSendNo;
		}
	}

	for (i=0;i<Num;i++)
	{
		if (GetEqpID(pPara[i].dwTEName,&wTEqpID)==ERROR)
			break;
		if (g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo==NULL)
			break;
		if (pPara[i].wOffset>=g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo->Cfg.wDYXNum)
			break;
	}

	if (i<Num)
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "%s DYX index invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
		return(ERROR);
	} 

	return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     VSYXParaCheck ID:1
 Purpose:       虚装置单点遥信参数检查
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS VSYXParaCheck(WORD wEqpID, const struct VPVirtualYX *pPara, WORD Num)
{
	WORD i,j,wTEqpID;

	j=0;
	for (i=0;i<Num;i++)
	{
		/*if (pPara[i].wSendNo==j)
			j++;
		else if (pPara[i].wSendNo!=(j-1))
		{
			if (pPara[i].wSendNo!=i)
			{
				myprintf(SYS_ID, LOG_ATTR_ERROR, "%s SYX sendno invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
				return(ERROR);
			} 
			else
				j=i+1;
		}*/ 
		if(i == 0)
			j = pPara[i].wSendNo;
		else
		{
			if(pPara[i].wSendNo < j)
			{
				myprintf(SYS_ID, LOG_ATTR_ERROR, "%s SYX sendno invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
				return(ERROR);
			}
			else
				j = pPara[i].wSendNo;
		} 
	}

	for (i=0;i<Num;i++)
	{
		if (GetEqpID(pPara[i].dwTEName,&wTEqpID)==ERROR)
			break;
		if (g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo==NULL)
			break;
		if (pPara[i].wOffset>=(g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo->Cfg.wSYXNum+g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo->Cfg.wVYXNum))
			break;
	}

	if (i<Num)
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "%s SYX index invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
		return(ERROR);
	} 

	return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     VDDParaCheck ID:1
 Purpose:       虚装置DD参数检查
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS VDDParaCheck(WORD wEqpID, const struct VPVirtualDD *pPara, WORD Num)
{
	WORD i,j,wTEqpID;

	j=0;
	for (i=0;i<Num;i++)
	{
		/*if (pPara[i].wSendNo==j)
			j++;
		else if (pPara[i].wSendNo!=(j-1))
		{
			if (pPara[i].wSendNo!=i)
			{
				myprintf(SYS_ID, LOG_ATTR_ERROR, "%s DD sendno invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
				return(ERROR);
			}
			else
				j=i+1;
		} */
		if(i == 0)
			j = pPara[i].wSendNo;
		else
		{
			if(pPara[i].wSendNo < j)
			{
				myprintf(SYS_ID, LOG_ATTR_ERROR, "%s DD sendno invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
				return(ERROR);
			}
			else
				j = pPara[i].wSendNo;
		} 
	}

	for (i=0;i<Num;i++)
	{
		if (GetEqpID(pPara[i].dwTEName,&wTEqpID)==ERROR)
			break;
		if (g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo==NULL)
			break;
		if (pPara[i].wOffset>=g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo->Cfg.wDDNum)
			break;
	}

	if (i<Num)
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "%s DD index invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
		return(ERROR);
	} 

	return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     VYKParaCheck ID:1
 Purpose:       虚装置YK参数检查
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS VYKParaCheck(WORD wEqpID, const struct VPVirtualCtrl *pPara, WORD Num)
{
	WORD i,wTEqpID;

	for (i=0;i<Num;i++)
	{
		if (GetEqpID(pPara[i].dwTEName,&wTEqpID)==ERROR)
			break;
		if (g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo==NULL)
			break;
		if (pPara[i].wOffset>=g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo->Cfg.wYKNum)
			break;
	}

	if (i<Num)
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "%s YK index invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
		return(ERROR);
	} 

	return(OK);
}

/*------------------------------------------------------------------------
 Procedure: 	VYTParaCheck ID:1
 Purpose:		虚装置YT参数检查
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS VYTParaCheck(WORD wEqpID, const struct VPVirtualCtrl *pPara, WORD Num)
{
	WORD i,wTEqpID;

	for (i=0;i<Num;i++)
	{
		if (GetEqpID(pPara[i].dwTEName,&wTEqpID)==ERROR)
			break;
		if (g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo==NULL)
			break;
		if (pPara[i].wOffset>=g_Sys.Eqp.pInfo[wTEqpID].pTEqpInfo->Cfg.wYTNum)
			break;
	}

	if (i<Num)
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "%s YT index invalid in NO.%d!",g_Sys.Eqp.pInfo[wEqpID].sCfgName,i); 
		return(ERROR);
	} 

	return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     VEqpParaCheck ID:1
 Purpose:       虚装置配置信息检查
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS VEqpParaCheck(WORD wEqpID)
{
	struct VPVECfg *pPVECfg=(struct VPVECfg *)(g_pParafile+1);
	struct VPVirtualYC *pPYC=(struct VPVirtualYC *)(pPVECfg+1);
	struct VPVirtualYX *pPDYX=(struct VPVirtualYX *)(pPYC+pPVECfg->wYCNum);
	struct VPVirtualYX *pPSYX=(struct VPVirtualYX *)(pPDYX+pPVECfg->wDYXNum);
	struct VPVirtualDD *pPDD=(struct VPVirtualDD *)(pPSYX+pPVECfg->wSYXNum);
	struct VPVirtualCtrl *pPYK=(struct VPVirtualCtrl *)(pPDD+pPVECfg->wDDNum);
	struct VPVirtualCtrl *pPYT=(struct VPVirtualCtrl *)(pPYK+pPVECfg->wYKNum);

	if (VYCParaCheck(wEqpID,pPYC,pPVECfg->wYCNum)==ERROR) return(ERROR);
	if (VDYXParaCheck(wEqpID,pPDYX,pPVECfg->wDYXNum)==ERROR) return(ERROR);
	if (VSYXParaCheck(wEqpID,pPSYX,pPVECfg->wSYXNum)==ERROR) return(ERROR);
	if (VDDParaCheck(wEqpID,pPDD,pPVECfg->wDDNum)==ERROR) return(ERROR);
	if (VYKParaCheck(wEqpID,pPYK,pPVECfg->wYKNum)==ERROR) return(ERROR);
	if (VYTParaCheck(wEqpID,pPYT,pPVECfg->wYTNum)==ERROR) return(ERROR);

	return(OK);
}

/*------------------------------------------------------------------------
 Procedure:     VECfgNumCMP ID:1
 Purpose:       虚装置配置信息检查
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
DWORD VECfgNumCMP(const struct VVECfg *pVECfg, const struct VPVECfg *pPVECfg)
{
	if (*g_Sys.pdwRestart!=WARMRESTART)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}
	if (pVECfg->wYCNum!=pPVECfg->wYCNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}
	if (pVECfg->wDYXNum!=pPVECfg->wDYXNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}  
	if (pVECfg->wSYXNum!=pPVECfg->wSYXNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}  
	if (pVECfg->wDDNum!=pPVECfg->wDDNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}  
	if (pVECfg->wYKNum!=pPVECfg->wYKNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}  
	if (pVECfg->wYTNum!=pPVECfg->wYTNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}
	if (pVECfg->wTQNum!=pPVECfg->wTQNum)
	{
		*g_Sys.pdwRestart=COLDRESTART;
		return(COLDRESTART);
	}     
	
	return(WARMRESTART);
}   

/*------------------------------------------------------------------------
 Procedure:     InitVirtualYC ID:1
 Purpose:       初始化虚装置YC参数
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitVirtualYC(struct VVirtualEqp * pVEqp, const struct VPVirtualYC * pPYC)
{
	WORD i,j,k;
	BOOL Start;
	struct VVECfg *pVECfg=(struct VVECfg *)&pVEqp->Cfg;

	MemoryMalloc((void **)&pVEqp->pYC,pVECfg->wYCNum*sizeof(struct VVirtualYC),DRAM);
	k = 0;
	for (i=0;i<pVECfg->wYCNum;i++)
	{
		strcpy(pVEqp->pYC[i].sName,pPYC[i].sName);	
		strcpy(pVEqp->pYC[i].sNameTab,pPYC[i].sNameTab);
		pVEqp->pYC[i].dwCfg=pPYC[i].dwCfg;
		GetEqpID(pPYC[i].dwTEName,&pVEqp->pYC[i].wTEID);
		pVEqp->pYC[i].wOffset=pPYC[i].wOffset;         
		pVEqp->pYC[i].lA=(short)pPYC[i].lA;
		pVEqp->pYC[i].lB=(short)pPYC[i].lB;
		pVEqp->pYC[i].lC=(short)pPYC[i].lC;
		pVEqp->pYC[i].wSendNo = pPYC[i].wSendNo;
		
		if (pPYC[i].wOffset == 0xFFFF) continue;

		if(pVEqp->pYC[i].dwCfg & 0x800)
			k ++;

		g_Sys.Eqp.pInfo[pVEqp->pYC[i].wTEID].pTEqpInfo->pYCCfg[pVEqp->pYC[i].wOffset].wVENum++;

		Start=TRUE;
		pVEqp->pYC[i].wNum=0;
		for (j=0;j<pVECfg->wYCNum;j++)
			if (pPYC[j].wSendNo==pPYC[i].wSendNo)
			{
				if (Start)
				{
					Start=FALSE;
					pVEqp->pYC[i].wStartNo=j;
				} 
				pVEqp->pYC[i].wNum++;
			} 
	}
	pVEqp->YCHourFrz.wNum = k;
	MemoryMalloc((void **)&pVEqp->YCHourFrz.pwIndex,pVEqp->YCHourFrz.wNum*sizeof(WORD),DRAM);
	j = 0;
	for (i=0;i<pVECfg->wYCNum;i++)
	{
		if (pVEqp->pYC[i].dwCfg & 0x800)
		   pVEqp->YCHourFrz.pwIndex[j++] = i;
	}
}

/*------------------------------------------------------------------------
 Procedure:     InitVirtualYX ID:1
 Purpose:       初始化虚装置YX参数
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitVirtualYX(int nFlag, WORD wYXNum, struct VVirtualYX * pYX, const struct VPVirtualYX * pPYX)
{
	WORD i,j;
	BOOL Start;
	for (i=0;i<wYXNum;i++)
	{
		strcpy(pYX[i].sName,pPYX[i].sName);	   
		strcpy(pYX[i].sNameTab,pPYX[i].sNameTab);
		pYX[i].dwCfg=pPYX[i].dwCfg;
		GetEqpID(pPYX[i].dwTEName,&pYX[i].wTEID);
		pYX[i].wOffset=pPYX[i].wOffset;
		pYX[i].wSendNo = pPYX[i].wSendNo;

		if (pPYX[i].wOffset == 0xFFFF) continue;

		switch (nFlag)
		{
			case 1:
				g_Sys.Eqp.pInfo[pYX[i].wTEID].pTEqpInfo->pDYXCfg[pYX[i].wOffset].wVENum++;
				break;
			case 2:
				g_Sys.Eqp.pInfo[pYX[i].wTEID].pTEqpInfo->pSYXCfg[pYX[i].wOffset].wVENum++;
				break;
			default:
				break;
		}
		
		Start=TRUE;
		pYX[i].wNum=0;
		for (j=0;j<wYXNum;j++)
		if (pPYX[j].wSendNo==pPYX[i].wSendNo)
		{
			if (Start)
			{
				Start=FALSE;
				pYX[i].wStartNo=j;
			} 
			pYX[i].wNum++;
		} 
	}   
}

/*------------------------------------------------------------------------
 Procedure:     InitVirtualDYX ID:1
 Purpose:       初始化虚装置双点YX参数
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitVirtualDYX(struct VVirtualEqp * pVEqp, const struct VPVirtualYX* pPDYX)
{
	struct VVECfg *pVECfg=(struct VVECfg *)&pVEqp->Cfg;

	MemoryMalloc((void **)&pVEqp->pDYX,pVECfg->wDYXNum*sizeof(struct VVirtualYX),DRAM);
	InitVirtualYX(1,pVECfg->wDYXNum,pVEqp->pDYX,pPDYX);

	if (*g_Sys.pdwRestart!=WARMRESTART)
	{
		*g_Sys.pdwRestart=COLDRESTART;

		if (pVECfg->wDYXNum)
		{
			pVEqp->DSOE.wWritePtr=0;    
			memset(pVEqp->DSOE.pPoolHead,0,pVEqp->DSOE.wPoolNum*sizeof(struct VDBDSOE));

			pVEqp->DCOS.wWritePtr=0;    
			memset(pVEqp->DCOS.pPoolHead,0,pVEqp->DCOS.wPoolNum*sizeof(struct VDBDCOS));
		} 
	}   
}

/*------------------------------------------------------------------------
 Procedure:     InitVirtualSYX ID:1
 Purpose:       初始化虚装置单点YX参数
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitVirtualSYX(struct VVirtualEqp * pVEqp, const struct VPVirtualYX* pPSYX)
{
	struct VVECfg *pVECfg=(struct VVECfg *)&pVEqp->Cfg;

	MemoryMalloc((void **)&pVEqp->pSYX,pVECfg->wSYXNum*sizeof(struct VVirtualYX),DRAM);

	InitVirtualYX(2,pVECfg->wSYXNum,pVEqp->pSYX,pPSYX);

	if (*g_Sys.pdwRestart!=WARMRESTART)
	{
		*g_Sys.pdwRestart=COLDRESTART;

		if (pVECfg->wSYXNum)
		{
			pVEqp->SSOE.wWritePtr=0;    
			memset(pVEqp->SSOE.pPoolHead,0,pVEqp->SSOE.wPoolNum*sizeof(struct VDBSOE));

			pVEqp->SCOS.wWritePtr=0;    
			memset(pVEqp->SCOS.pPoolHead,0,pVEqp->SCOS.wPoolNum*sizeof(struct VDBCOS));
		} 
	}   
}

/*------------------------------------------------------------------------
 Procedure:     InitVirtualDD ID:1
 Purpose:       初始化虚装置DD
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitVirtualDD(struct VVirtualEqp * pVEqp, const struct VPVirtualDD * pPDD)
{
	WORD i,j;
	BOOL Start;
	struct VVECfg *pVECfg=(struct VVECfg *)&pVEqp->Cfg;

	MemoryMalloc((void **)&pVEqp->pDD,pVECfg->wDDNum*sizeof(struct VVirtualDD),DRAM);

	for (i=0;i<pVECfg->wDDNum;i++)
	{
		strcpy(pVEqp->pDD[i].sName,pPDD[i].sName);	   
		strcpy(pVEqp->pDD[i].sNameTab,pPDD[i].sNameTab);	   
		pVEqp->pDD[i].dwCfg=pPDD[i].dwCfg;
		GetEqpID(pPDD[i].dwTEName,&pVEqp->pDD[i].wTEID);
		pVEqp->pDD[i].wOffset=pPDD[i].wOffset;
		pVEqp->pDD[i].wSendNo = pPDD[i].wSendNo;

		g_Sys.Eqp.pInfo[pVEqp->pDD[i].wTEID].pTEqpInfo->pDDCfg[pVEqp->pDD[i].wOffset].wVENum++;

		Start=TRUE;
		pVEqp->pDD[i].wNum=0;
		for (j=0;j<pVECfg->wDDNum;j++)
			if (pPDD[j].wSendNo==pPDD[i].wSendNo)
			{
				if (Start)
				{
					Start=FALSE;
					pVEqp->pDD[i].wStartNo=j;
				} 
				pVEqp->pDD[i].wNum++;
			} 
	}
	
	pVEqp->DDHourFrz.wNum = pVECfg->wDDNum;
	MemoryMalloc((void**)&pVEqp->DDHourFrz.pwIndex,pVEqp->DDHourFrz.wNum*sizeof(WORD),DRAM);
	j = 0;
	for(i = 0;i < pVECfg->wDDNum;i++)
	{
		pVEqp->DDHourFrz.pwIndex[j++] = i;
	}
}

/*------------------------------------------------------------------------
 Procedure:     InitVirtualYK ID:1
 Purpose:       初始化虚装置YK
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitVirtualYK(struct VVirtualEqp * pVEqp, const struct VPVirtualCtrl * pPYK)
{
	WORD i;
	struct VVECfg *pVECfg=(struct VVECfg *)&pVEqp->Cfg;

	MemoryMalloc((void **)&pVEqp->pYK,pVECfg->wYKNum*sizeof(struct VVirtualCtrl),DRAM);

	for (i=0;i<pVECfg->wYKNum;i++)
	{
		strcpy(pVEqp->pYK[i].sName,pPYK[i].sName);	 
		strcpy(pVEqp->pYK[i].sNameTab,pPYK[i].sNameTab);	 
		pVEqp->pYK[i].dwCfg=pPYK[i].dwCfg;
		GetEqpID(pPYK[i].dwTEName,&pVEqp->pYK[i].wTEID);
		pVEqp->pYK[i].wOffset=pPYK[i].wOffset;
		pVEqp->pYK[i].wID=pPYK[i].wID;
	}   
}

/*------------------------------------------------------------------------
 Procedure: 	InitVirtualYT ID:1
 Purpose:		初始化虚装置YT
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void InitVirtualYT(struct VVirtualEqp * pVEqp, const struct VPVirtualCtrl * pPYT)
{
	WORD i;
	struct VVECfg *pVECfg=(struct VVECfg *)&pVEqp->Cfg;

	MemoryMalloc((void **)&pVEqp->pYT,pVECfg->wYTNum*sizeof(struct VVirtualCtrl),DRAM);

	for (i=0;i<pVECfg->wYTNum;i++)
	{
		strcpy(pVEqp->pYT[i].sName,pPYT[i].sName);	 
		strcpy(pVEqp->pYT[i].sNameTab,pPYT[i].sNameTab);	 
		pVEqp->pYT[i].dwCfg=pPYT[i].dwCfg;
		GetEqpID(pPYT[i].dwTEName,&pVEqp->pYT[i].wTEID);
		pVEqp->pYT[i].wOffset=pPYT[i].wOffset;
		pVEqp->pYT[i].wID=pPYT[i].wID;
	}   
}

/*------------------------------------------------------------------------
 Procedure:     VirtualEqpInit ID:1
 Purpose:       虚装置初始化
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void DevTableInit(WORD wEqpID)
{
       
	struct VVECfg *pVECfg;  
	struct VVirtualEqp *pVEqp;

	struct VPVECfg *pPVECfg=(struct VPVECfg *)(g_pParafile+1);
	struct VPVirtualYC *pPYC=(struct VPVirtualYC *)(pPVECfg+1);
	struct VPVirtualYX *pPDYX=(struct VPVirtualYX *)(pPYC+pPVECfg->wYCNum);
	struct VPVirtualYX *pPSYX=(struct VPVirtualYX *)(pPDYX+pPVECfg->wDYXNum);
	struct VPVirtualDD *pPDD=(struct VPVirtualDD *)(pPSYX+pPVECfg->wSYXNum);
	struct VPVirtualCtrl *pPYK=(struct VPVirtualCtrl *)(pPDD+pPVECfg->wDDNum);
	struct VPVirtualCtrl *pPYT=(struct VPVirtualCtrl *)(pPYK+pPVECfg->wYKNum);

	if (!(pPVECfg->dwFlag&FA_VEQP_FLAG))
	{
		if (VEqpParaCheck(wEqpID)==ERROR)
		{
			g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo=NULL;
			*g_Sys.pdwRestart=COLDRESTART;     
			WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "参数错误");			
			myprintf(SYS_ID, LOG_ATTR_ERROR, "%s.cdt is bad!",g_Sys.Eqp.pInfo[wEqpID].sCfgName);   
			return;
		}   
	}
	

	MemoryMalloc((void **)&g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo,sizeof(struct VVirtualEqp),NVRAM);

	g_Task[g_Sys.Eqp.pInfo[wEqpID].wTaskID].wEqpNum++;

	pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo;
	pVECfg=&pVEqp->Cfg;

	pPVECfg->wSourceAddr=g_Sys.AddrInfo.wAddr;
	pVECfg->wDesAddr=pPVECfg->wDesAddr;
	memcpy(pVECfg->sExtAddr, pPVECfg->sExtAddr, 12);
	strcpy(pVECfg->sTelNo, pPVECfg->sTelNo);
	pVECfg->dwFlag=pPVECfg->dwFlag;
	
	pVECfg->wInvRefEGNum=0;
	if (VECfgNumCMP(pVECfg,pPVECfg)==COLDRESTART)
	{     
		pVECfg->wYCNum=pPVECfg->wYCNum;
		pVECfg->wDYXNum=pPVECfg->wDYXNum;
		pVECfg->wSYXNum=pPVECfg->wSYXNum;
		pVECfg->wDDNum=pPVECfg->wDDNum;
		pVECfg->wYKNum=pPVECfg->wYKNum;
		pVECfg->wYTNum=pPVECfg->wYTNum;
		pVECfg->wTQNum=pPVECfg->wTQNum;
	}

	PoolMalloc((struct VPool *)&pVEqp->DSOE,pVECfg->wDYXNum,VIRTUALEQPDSOENUM,sizeof(struct VDBDSOE));
	PoolMalloc((struct VPool *)&pVEqp->DCOS,pVECfg->wDYXNum,VIRTUALEQPDCOSNUM,sizeof(struct VDBDCOS));
	PoolMalloc((struct VPool *)&pVEqp->SSOE,pVECfg->wSYXNum,VIRTUALEQPSSOENUM,sizeof(struct VDBSOE));
	PoolMalloc((struct VPool *)&pVEqp->SCOS,pVECfg->wSYXNum,VIRTUALEQPSCOSNUM,sizeof(struct VDBCOS));

	PoolMalloc((struct VPool *)&pVEqp->pCO,pVECfg->wYKNum,VIRTUALEQPCONUM,sizeof(struct VDBCO));

	InitVirtualDYX(pVEqp,pPDYX);
	InitVirtualSYX(pVEqp,pPSYX);

	InitVirtualYC(pVEqp,pPYC);
	InitVirtualDD(pVEqp,pPDD);
	InitVirtualYK(pVEqp,pPYK);
	InitVirtualYT(pVEqp,pPYT);  

#ifdef INCLUDE_B2F_SOE
    /*必须放在最后,否则数据项初始化会清空soe*/
	PoolMalloc_B2F((struct VPool *)&pVEqp->DSOE,pVECfg->wDYXNum,VIRTUALEQPDSOENUM,sizeof(struct VDBDSOE),wEqpID,SYSEV_FLAG_DSOE);
	PoolMalloc_B2F((struct VPool *)&pVEqp->SSOE,pVECfg->wSYXNum,VIRTUALEQPSSOENUM,sizeof(struct VDBSOE),wEqpID,SYSEV_FLAG_SSOE);
#endif	
#ifdef INCLUDE_B2F_COS
    /*必须放在最后,否则数据项初始化会清空cos*/
	PoolMalloc_B2F((struct VPool *)&pVEqp->SCOS,pVECfg->wSYXNum,VIRTUALEQPSCOSNUM,sizeof(struct VDBCOS),wEqpID,SYSEV_FLAG_SCOS);
#endif	

}

/*------------------------------------------------------------------------
 Procedure:     SetYCVirtualToTrueRef ID:1
 Purpose:       建立虚装置到实装置的遥测索引
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void SetYCVirtualToTrueRef(WORD wVEqpID)
{
	WORD i;
	struct VTrueYCCfg *pTrueYCCfg;
	struct VVirtualEqp *pVEqp=g_Sys.Eqp.pInfo[wVEqpID].pVEqpInfo;

	for (i=0;i<pVEqp->Cfg.wYCNum;i++)
	{
		if(pVEqp->pYC[i].wOffset == 0xFFFF)
			continue;
		pTrueYCCfg=&g_Sys.Eqp.pInfo[pVEqp->pYC[i].wTEID].pTEqpInfo->pYCCfg[pVEqp->pYC[i].wOffset];

		pTrueYCCfg->pInvRef[pTrueYCCfg->wVENum].wEqpID = wVEqpID;
		pTrueYCCfg->pInvRef[pTrueYCCfg->wVENum].wNo = i;
		pTrueYCCfg->wVENum++;
	} 
} 

/*------------------------------------------------------------------------
 Procedure:     SetYXVirtualToTrueRef ID:1
 Purpose:       建立虚装置到实装置的遥信索引
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void SetYXVirtualToTrueRef(int nFlag,WORD wVEqpID)
{
	WORD i,Num;
	struct VTrueYXCfg *pTrueYXCfg;
	struct VVirtualEqp *pVEqp=g_Sys.Eqp.pInfo[wVEqpID].pVEqpInfo;

	switch (nFlag)
	{
		case 1: 
			Num=pVEqp->Cfg.wDYXNum;
			break;
		case 2:
			Num=pVEqp->Cfg.wSYXNum;
			break;
		default:
			Num=0;
			break;
	} 

	for (i=0;i<Num;i++)
	{
    	switch (nFlag)
		{
			case 1:
				if (pVEqp->pDYX[i].wOffset == 0xFFFF) continue;
				pTrueYXCfg=&g_Sys.Eqp.pInfo[pVEqp->pDYX[i].wTEID].pTEqpInfo->pDYXCfg[pVEqp->pDYX[i].wOffset];
				break;
			case 2:
				if(pVEqp->pSYX[i].wOffset == 0xFFFF) continue;
				pTrueYXCfg=&g_Sys.Eqp.pInfo[pVEqp->pSYX[i].wTEID].pTEqpInfo->pSYXCfg[pVEqp->pSYX[i].wOffset];
				break;
            default:
				return;
		}       
		pTrueYXCfg->pInvRef[pTrueYXCfg->wVENum].wEqpID = wVEqpID;
		pTrueYXCfg->pInvRef[pTrueYXCfg->wVENum].wNo = i;
		pTrueYXCfg->wVENum++;
	} 
} 

/*------------------------------------------------------------------------
 Procedure:     SetDDVirtualToTrueRef ID:1
 Purpose:       建立虚装置到实装置的电度索引
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void SetDDVirtualToTrueRef(WORD wVEqpID)
{
	WORD i;
	struct VTrueDDCfg *pTrueDDCfg;
	struct VVirtualEqp *pVEqp=g_Sys.Eqp.pInfo[wVEqpID].pVEqpInfo;

	for (i=0;i<pVEqp->Cfg.wDDNum;i++)
	{  
		pTrueDDCfg=&g_Sys.Eqp.pInfo[pVEqp->pDD[i].wTEID].pTEqpInfo->pDDCfg[pVEqp->pDD[i].wOffset];

		pTrueDDCfg->pInvRef[pTrueDDCfg->wVENum].wEqpID = wVEqpID;
		pTrueDDCfg->pInvRef[pTrueDDCfg->wVENum].wNo = i;
		pTrueDDCfg->wVENum++;
	} 
} 

/*------------------------------------------------------------------------
 Procedure:     SetVirtualToTrueRef ID:1
 Purpose:       建立虚装置到实装置的索引
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void SetTableToDevRef(void)
{
	WORD i,j;
	struct VTrueEqp *pTEqp;

	for (i=0;i<*g_Sys.Eqp.pwNum;i++)
		if (g_Sys.Eqp.pInfo[i].pTEqpInfo!=NULL)
		{
			pTEqp=g_Sys.Eqp.pInfo[i].pTEqpInfo;

			for (j=0;j<pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum;j++)
			{
				MemoryMalloc((void **)&pTEqp->pYCCfg[j].pInvRef, pTEqp->pYCCfg[j].wVENum*sizeof(struct VDataRef), DRAM);
				pTEqp->pYCCfg[j].wVENum=0;
			} 
			for (j=0;j<pTEqp->Cfg.wDYXNum;j++)
			{
				MemoryMalloc((void **)&pTEqp->pDYXCfg[j].pInvRef, pTEqp->pDYXCfg[j].wVENum*sizeof(struct VDataRef), DRAM);
				pTEqp->pDYXCfg[j].wVENum=0;
			}
			for (j=0;j<pTEqp->Cfg.wSYXNum+pTEqp->Cfg.wVYXNum;j++)
			{
				MemoryMalloc((void **)&pTEqp->pSYXCfg[j].pInvRef, pTEqp->pSYXCfg[j].wVENum*sizeof(struct VDataRef), DRAM);
				pTEqp->pSYXCfg[j].wVENum=0;
			} 
			for (j=0;j<pTEqp->Cfg.wDDNum;j++)
			{
				MemoryMalloc((void **)&pTEqp->pDDCfg[j].pInvRef, pTEqp->pDDCfg[j].wVENum*sizeof(struct VDataRef), DRAM);
				pTEqp->pDDCfg[j].wVENum=0;
			} 
		}

	for (i=0;i<*g_Sys.Eqp.pwNum;i++)
		if (g_Sys.Eqp.pInfo[i].pVEqpInfo!=NULL)
		{
			SetYCVirtualToTrueRef(i);
			SetYXVirtualToTrueRef(1,i);
			SetYXVirtualToTrueRef(2,i);
			SetDDVirtualToTrueRef(i);
		}
}  

/*------------------------------------------------------------------------
 Procedure:     SetEqpRunInfo ID:1
 Purpose:       建立装置运行信息
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void SetEqpRunInfo(struct VEqpRunInfo *pEqpRunInfo,WORD wEqpID)
{
	DWORD dwEqpName;
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;

	dwEqpName = g_Sys.Eqp.pInfo[wEqpID].dwName;
	if ((pEqpRunInfo->wEqpID != wEqpID) || (pEqpRunInfo->dwEqpName != dwEqpName))
		*g_Sys.pdwRestart=COLDRESTART;

	pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo;
	pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo;      

	if (pTEqp!=NULL)
	{
		if (pTEqp->DSOE.wPoolNum && (pEqpRunInfo->wDSOEReadPtr >= pTEqp->DSOE.wPoolNum))
			*g_Sys.pdwRestart = COLDRESTART;
		else if (pTEqp->DCOS.wPoolNum && (pEqpRunInfo->wDCOSReadPtr >= pTEqp->DCOS.wPoolNum))
			*g_Sys.pdwRestart = COLDRESTART;
		else if (pTEqp->SSOE.wPoolNum && (pEqpRunInfo->wSSOEReadPtr >= pTEqp->SSOE.wPoolNum))
			*g_Sys.pdwRestart = COLDRESTART;
		else if (pTEqp->SCOS.wPoolNum && (pEqpRunInfo->wSCOSReadPtr >= pTEqp->SCOS.wPoolNum))
			*g_Sys.pdwRestart = COLDRESTART;
	}   
	else if (pVEqp!=NULL)
	{
		if (pVEqp->DSOE.wPoolNum &&(pEqpRunInfo->wDSOEReadPtr >= pVEqp->DSOE.wPoolNum))
			*g_Sys.pdwRestart = COLDRESTART;
		else if (pVEqp->DCOS.wPoolNum &&(pEqpRunInfo->wDCOSReadPtr >= pVEqp->DCOS.wPoolNum))
			*g_Sys.pdwRestart = COLDRESTART;
		else if (pVEqp->SSOE.wPoolNum && (pEqpRunInfo->wSSOEReadPtr >= pVEqp->SSOE.wPoolNum))
			*g_Sys.pdwRestart = COLDRESTART;
		else if (pVEqp->SCOS.wPoolNum && (pEqpRunInfo->wSCOSReadPtr >= pVEqp->SCOS.wPoolNum))
			*g_Sys.pdwRestart = COLDRESTART;
	}

//	if (g_Sys.pFAProc->wPoolNum && (pEqpRunInfo->wFAInfoReadPtr >= g_Sys.pFAProc->wPoolNum))
//		*g_Sys.pdwRestart = COLDRESTART;
	if (g_Sys.pActEvent->wPoolNum && (pEqpRunInfo->wActEventReadPtr >= g_Sys.pActEvent->wPoolNum))
		*g_Sys.pdwRestart = COLDRESTART;
	else  if (g_Sys.pDoEvent->wPoolNum && (pEqpRunInfo->wDoEventReadPtr >= g_Sys.pDoEvent->wPoolNum))
		*g_Sys.pdwRestart = COLDRESTART;
	else  if (g_Sys.pWarnEvent->wPoolNum && (pEqpRunInfo->wWarnEventReadPtr >= g_Sys.pWarnEvent->wPoolNum))
		*g_Sys.pdwRestart = COLDRESTART;

	if (*g_Sys.pdwRestart != WARMRESTART)
	{
		memset(pEqpRunInfo, 0, sizeof(struct VEqpRunInfo));
		pEqpRunInfo->wEqpID = wEqpID;
		pEqpRunInfo->dwEqpName = dwEqpName;
	}

	pEqpRunInfo->CommRunInfo.wState=COMMBREAK; 
} 

#ifdef INCLUDE_B2F_F

char* riFName[] = 
{
    "",
	"t%ddgri.bk",
	"t%dgdri.bk",
	"t%dderi.bk",
};

void SetEqpRunInfo_B2F(WORD wTaskID, struct VEqpRunInfo *pEqpRunInfo, WORD wEqpID, WORD index, int flag)
{
	DWORD dwEqpName;
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	VB2FSocket socket;
	struct VEqpRunInfo info;
	int fileerr;

	if ((flag<=0) || (flag>=MAX_RI_FLAG_NUM)) return;

	sprintf(socket.fname, riFName[flag], wTaskID);
		
	socket.offset = index*sizeof(struct VEqpRunInfo);
	socket.len = sizeof(struct VEqpRunInfo);
				
	socket.read_write = 1;
	socket.buf = (BYTE *)&info;
	if (Buf2FileRead(&socket) == ERROR) return;

    fileerr = 0;
	dwEqpName = g_Sys.Eqp.pInfo[wEqpID].dwName;
	if ((info.wEqpID != wEqpID) || (info.dwEqpName != dwEqpName))
		fileerr = 1;

	pTEqp=g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo;
	pVEqp=g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo;      

	if (pTEqp!=NULL)
	{
		if (pTEqp->DSOE.wPoolNum && (info.wDSOEReadPtr >= pTEqp->DSOE.wPoolNum))
			fileerr = 1;
		else if (pTEqp->SSOE.wPoolNum && (info.wSSOEReadPtr >= pTEqp->SSOE.wPoolNum))
			fileerr = 1;
	}   
	else if (pVEqp!=NULL)
	{
		if (pVEqp->DSOE.wPoolNum && (info.wDSOEReadPtr >= pVEqp->DSOE.wPoolNum))
			fileerr = 1;
		else if (pVEqp->SSOE.wPoolNum && (info.wSSOEReadPtr >= pVEqp->SSOE.wPoolNum))
			fileerr = 1;
	}

//	if (g_Sys.pFAProc->wPoolNum && (info.wFAInfoReadPtr >= g_Sys.pFAProc->wPoolNum))
//		fileerr = 1;
	if (g_Sys.pActEvent->wPoolNum && (info.wActEventReadPtr >= g_Sys.pActEvent->wPoolNum))
		fileerr = 1;
	else  if (g_Sys.pDoEvent->wPoolNum && (info.wDoEventReadPtr >= g_Sys.pDoEvent->wPoolNum))
		fileerr = 1;
	else  if (g_Sys.pWarnEvent->wPoolNum && (info.wWarnEventReadPtr >= g_Sys.pWarnEvent->wPoolNum))
		fileerr = 1;

	if (fileerr == 0)
	{
		memcpy(pEqpRunInfo, &info, sizeof(struct VEqpRunInfo));
#ifndef INCLUDE_B2F_COS
		pEqpRunInfo->wDCOSReadPtr = 0;
		pEqpRunInfo->wSCOSReadPtr = 0;
#endif
	}
	else
	{		
		socket.read_write = 3;
		socket.buf = (BYTE *)pEqpRunInfo;
		Buf2FileWrite(&socket);	
	}

} 
#endif

#if (TYPE_IO != IO_TYPE_EXTIO)

void DevGroupInit(WORD wEqpID)
{
	WORD i,wTempID;
	struct VEGCfg *pEGCfg;  
	struct VEqpGroup *pEqpG;
	DWORD *pdwEqpName;
#ifdef INCLUDE_XML_MODEL
    struct VPort *pPort;
#endif

	struct VPEGCfg *pPEGCfg=(struct VPEGCfg *)(g_pParafile+1);
	pdwEqpName=(DWORD *)(pPEGCfg+1);

	for (i=0;i<pPEGCfg->wEqpNum;i++)
	{
		if (GetEqpID(pdwEqpName[i],&wTempID)==ERROR)
			break;
		else  if ((g_Sys.Eqp.pInfo[wTempID].pTEqpInfo==NULL)&&(g_Sys.Eqp.pInfo[wTempID].pVEqpInfo==NULL))
			break;
	}  

	if (i<pPEGCfg->wEqpNum)
	{
		g_Sys.Eqp.pInfo[wEqpID].pEqpGInfo=NULL;
		*g_Sys.pdwRestart=COLDRESTART;  
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "参数错误");			
		myprintf(SYS_ID, LOG_ATTR_ERROR, "%s.cdg have invalid Equipment!", g_Sys.Eqp.pInfo[wEqpID].sCfgName);
		return;
	} 
	/*one commid only have one groupeqp so...*/
	if (g_Task[g_Sys.Eqp.pInfo[wEqpID].wTaskID].wEqpGroupNum==0)
	{
		g_Task[g_Sys.Eqp.pInfo[wEqpID].wTaskID].wEqpGroupNum=1;
		MemoryMalloc((void **)&g_Sys.Eqp.pInfo[wEqpID].pEqpGInfo,sizeof(struct VEqpGroup),DRAM);

		pEqpG=g_Sys.Eqp.pInfo[wEqpID].pEqpGInfo;
		pEGCfg=&pEqpG->Cfg;

		pEGCfg->wSourceAddr=g_Sys.AddrInfo.wAddr;
		pEGCfg->wDesAddr=pPEGCfg->wDesAddr;
		
		pEGCfg->dwFlag=pPEGCfg->dwFlag;

		pEGCfg->dwFlag &= 0xFE;    

		pEGCfg->wEqpNum=pPEGCfg->wEqpNum;

		MemoryMalloc((void **)&g_Task[g_Sys.Eqp.pInfo[wEqpID].wTaskID].pEqpGroupRunInfo,sizeof(struct VEqpRunInfo),NVRAM);
		SetEqpRunInfo(g_Task[g_Sys.Eqp.pInfo[wEqpID].wTaskID].pEqpGroupRunInfo,wEqpID);
#ifdef INCLUDE_B2F_F
		SetEqpRunInfo_B2F(g_Sys.Eqp.pInfo[wEqpID].wTaskID,g_Task[g_Sys.Eqp.pInfo[wEqpID].wTaskID].pEqpGroupRunInfo,wEqpID,0,RI_FLAG_EG);
#endif
		g_Task[g_Sys.Eqp.pInfo[wEqpID].wTaskID].wGroupEqpNum=pEGCfg->wEqpNum;
		MemoryMalloc((void **)&pEqpG->pwEqpID,pEGCfg->wEqpNum*sizeof(WORD),DRAM);

#ifdef INCLUDE_XML_MODEL
        pPort = &g_Task[g_Sys.Eqp.pInfo[wEqpID].wTaskID].CommCfg.Port;

        if ((GetXmlProtocol() == PROTOCOL_104) && (strcmp(pPort->pcol, "GB104") == 0))
        {
		    pEGCfg->wEqpNum = 1;
		    pEqpG->pwEqpID[0] = *g_Sys.Eqp.pwNum-1;
			g_Sys.Eqp.pInfo[pEqpG->pwEqpID[0]].pVEqpInfo->Cfg.wInvRefEGNum++;
        }
		else
#endif
		for (i=0;i<pEGCfg->wEqpNum;i++)
		{
			GetEqpID(pdwEqpName[i],pEqpG->pwEqpID+i);
			if (g_Sys.Eqp.pInfo[pEqpG->pwEqpID[i]].pTEqpInfo)
				g_Sys.Eqp.pInfo[pEqpG->pwEqpID[i]].pTEqpInfo->Cfg.wInvRefEGNum++;
			else
				g_Sys.Eqp.pInfo[pEqpG->pwEqpID[i]].pVEqpInfo->Cfg.wInvRefEGNum++; 
		} 
	}  
} 

/*------------------------------------------------------------------------
 Procedure:     SetEqpRef ID:1
 Purpose:       设置装置索引
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/   
void SetDevRef(void)    
{
	WORD i,j;
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	struct VEqpGroup *pEqpG;

	for (i=0;i<*g_Sys.Eqp.pwNum;i++)
	{
		if (g_Sys.Eqp.pInfo[i].pTEqpInfo!=NULL)
		{
			pTEqp=g_Sys.Eqp.pInfo[i].pTEqpInfo;
			MemoryMalloc((void **)&pTEqp->pwInvRefEGID,pTEqp->Cfg.wInvRefEGNum*sizeof(WORD),DRAM);
			pTEqp->Cfg.wInvRefEGNum=0;
		} 
		else if (g_Sys.Eqp.pInfo[i].pVEqpInfo!=NULL)
		{
			pVEqp=g_Sys.Eqp.pInfo[i].pVEqpInfo;
			MemoryMalloc((void **)&pVEqp->pwInvRefEGID,pVEqp->Cfg.wInvRefEGNum*sizeof(WORD),DRAM);
			pVEqp->Cfg.wInvRefEGNum=0;
		} 
	}

	for (i=0;i<*g_Sys.Eqp.pwNum;i++)
		if (g_Sys.Eqp.pInfo[i].pEqpGInfo!=NULL)
		{
			pEqpG=g_Sys.Eqp.pInfo[i].pEqpGInfo;

			for (j=0;j<pEqpG->Cfg.wEqpNum;j++)
			{
				if (g_Sys.Eqp.pInfo[pEqpG->pwEqpID[j]].pTEqpInfo!=NULL)
				{
					pTEqp=g_Sys.Eqp.pInfo[pEqpG->pwEqpID[j]].pTEqpInfo;
					pTEqp->pwInvRefEGID[pTEqp->Cfg.wInvRefEGNum]=i;
					pTEqp->Cfg.wInvRefEGNum++;
				} 
				else
				{
					pVEqp=g_Sys.Eqp.pInfo[pEqpG->pwEqpID[j]].pVEqpInfo;
					pVEqp->pwInvRefEGID[pVEqp->Cfg.wInvRefEGNum]=i;
					pVEqp->Cfg.wInvRefEGNum++;
				} 
			} 
	     }   
}      

#endif

/*------------------------------------------------------------------------
 Procedure:     RtosAssert ID:1
 Purpose:       os出现assert后先保存信息后复位
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/  
void RtosAssert(char *msg, int flag)
{
#ifdef INCLUDE_B2F_F
	VB2FSocket socket;
#endif
    struct	VAssertInfo *pInfo;

	if (g_assert == NULL) return;

	pInfo = g_assert->pPoolHead + g_assert->wWritePtr;
	GetSysClock(&(pInfo->time), CALCLOCK);
	strncpy(pInfo->msg, msg, SYS_LOG_MSGLEN);
	pInfo->msg[SYS_LOG_MSGLEN-1] = '\0';
    g_assert->wWritePtr++;
    g_assert->wWritePtr%=g_assert->wPoolNum;

#ifdef INCLUDE_B2F_F
	strcpy(socket.fname, "assert.evt");


    socket.read_write = 3;
	socket.offset = 0;
	socket.buf = (BYTE *)g_assert;
	socket.len = sizeof(WORD)*2;
	Buf2FileWrite(&socket);

	socket.read_write = 3;
	socket.offset = sizeof(WORD)*2 + (DWORD)((BYTE*)pInfo - (BYTE *)g_assert->pPoolHead);
	socket.buf = (BYTE*)pInfo;
	socket.len = g_assert->wCellLen;
	Buf2FileWrite(&socket);	 
#endif
#ifdef ENABLE_WATCHDOG
    if (flag) 
	{
	    CPU_WDG_RESET = 1;
	    quickreboot();
    }
#endif

}

/*------------------------------------------------------------------------
 Procedure:     EqpRunInfoInit ID:1
 Purpose:       装置运行信息初始化
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/  
void DevRunInfoInit(void)
{
	WORD i,j,k,*pEqpID;
	struct VEqpGroup *pEqpG;
	struct VEqpRunInfo *pEqpRunInfo;

	for (i=0; i<THREAD_MAX_NUM ;i++)
	{
		if (g_Task[i].wEqpGroupNum)
		{
			MemoryMalloc((void **)&g_Task[i].pGroupEqpRunInfo,g_Task[i].wGroupEqpNum*sizeof(struct VEqpRunInfo),NVRAM);
			pEqpRunInfo=g_Task[i].pGroupEqpRunInfo;
			pEqpG=g_Sys.Eqp.pInfo[g_Task[i].pEqpGroupRunInfo->wEqpID].pEqpGInfo;
			g_Task[i].pwGroupEqpIDList=pEqpG->pwEqpID;
			for (j=0;j<g_Task[i].wGroupEqpNum;j++)
			{
				SetEqpRunInfo(pEqpRunInfo,pEqpG->pwEqpID[j]);
#ifdef INCLUDE_B2F_F
				SetEqpRunInfo_B2F(i,pEqpRunInfo,pEqpG->pwEqpID[j],j,RI_FLAG_GE);
#endif
				pEqpRunInfo++;
			}
		} 
	
		if (g_Task[i].wEqpNum)
		{
			MemoryMalloc((void **)&g_Task[i].pEqpRunInfo,g_Task[i].wEqpNum*sizeof(struct VEqpRunInfo),NVRAM);
			MemoryMalloc((void **)&g_Task[i].pwEqpIDList,g_Task[i].wEqpNum*sizeof(WORD),DRAM);      
			pEqpID=g_Task[i].pwEqpIDList;
			pEqpRunInfo=g_Task[i].pEqpRunInfo;
#ifdef INCLUDE_B2F_F
			j = 0;
#endif
			for (k=0;k<*g_Sys.Eqp.pwNum;k++)
			{
				if ((g_Sys.Eqp.pInfo[k].wTaskID==i)&&((g_Sys.Eqp.pInfo[k].pTEqpInfo!=NULL)||(g_Sys.Eqp.pInfo[k].pVEqpInfo!=NULL)))
				{
					*pEqpID=k;
					pEqpID++;
					SetEqpRunInfo(pEqpRunInfo,k);
#ifdef INCLUDE_B2F_F
					SetEqpRunInfo_B2F(i,pEqpRunInfo,k,j,RI_FLAG_EQP);
					j++;
#endif					
					pEqpRunInfo++;
				} 
			}    
		} 

	} 
}

void DevInit(void)
{
	WORD i;
	char cfgname[60];

	for (i=0; i<*g_Sys.Eqp.pwNum; i++)
	{
		strcpy(cfgname, g_Sys.Eqp.pInfo[i].sCfgName);
		strcat(cfgname, ".cde");
		if (ReadParaFile(cfgname,(BYTE *)g_pParafile,MAXFILELEN)==OK)
		{
			TrueDevInit(i);
			myprintf(SYS_ID, LOG_ATTR_INFO, "Device %s init done.", g_Sys.Eqp.pInfo[i].sCfgName);
	    }  
		else if (g_Sys.Eqp.pInfo[i].pTEqpInfo)
		{
			g_Sys.Eqp.pInfo[i].pTEqpInfo = NULL;
			*g_Sys.pdwRestart = COLDRESTART;
		}  
	}    

#if (TYPE_IO != IO_TYPE_EXTIO)
	for (i=0;i<*g_Sys.Eqp.pwNum;i++)
	{
		strcpy(cfgname, g_Sys.Eqp.pInfo[i].sCfgName);
		strcat(cfgname, ".cdt");
		if (ReadParaFile(cfgname,(BYTE *)g_pParafile,MAXFILELEN)==OK)
		{
			DevTableInit(i);
			myprintf(SYS_ID, LOG_ATTR_INFO, "Device table %s init done.", g_Sys.Eqp.pInfo[i].sCfgName);
		}  
		else if (g_Sys.Eqp.pInfo[i].pVEqpInfo)
		{
			g_Sys.Eqp.pInfo[i].pVEqpInfo = NULL;
			*g_Sys.pdwRestart = COLDRESTART;
		}  
	} 

#ifdef INCLUDE_XML_MODEL
     if (xmlDevTableInit(*g_Sys.Eqp.pwNum-1))
	 	myprintf(SYS_ID, LOG_ATTR_INFO, "Device table %s init done, exchange veqp.", g_Sys.Eqp.pInfo[i-1].sCfgName);//by lvyi
#endif


	SetTableToDevRef();

	for (i=0;i<*g_Sys.Eqp.pwNum;i++)
	{
		strcpy(cfgname, g_Sys.Eqp.pInfo[i].sCfgName);
		strcat(cfgname, ".cdg");
		if (ReadParaFile(cfgname,(BYTE *)g_pParafile,MAXFILELEN)==OK)
		{
			DevGroupInit(i);
			myprintf(SYS_ID, LOG_ATTR_INFO, "Device group %s init done.", g_Sys.Eqp.pInfo[i].sCfgName);
		}  
		else if (g_Sys.Eqp.pInfo[i].pEqpGInfo)
		{
			g_Sys.Eqp.pInfo[i].pEqpGInfo = NULL;
			*g_Sys.pdwRestart = COLDRESTART;
		}  
	} 

	SetDevRef();	
#endif	

#ifdef DB_EPOWER_STAT
	for (i=0;i<*g_Sys.Eqp.pwNum;i++)
	{
		strcpy(paraname,g_Sys.Eqp.pInfo[i].sCfgName);
		strcat(paraname,".pee");
		if (ReadParaFile(paraname,(BYTE *)g_pParafile,MAXFILELEN)==OK)
		{
			EPowerEqpInit(i);
			myprintf(SYSID,"EPower equipment %s init done.",g_Sys.Eqp.pInfo[i].sCfgName);
		}  
		else if (g_Sys.Eqp.pInfo[i].pEEqpInfo)
		{
			g_Sys.Eqp.pInfo[i].pEEqpInfo=NULL;
			*g_Sys.pdwRestart=COLDRESTART;
		}  
	}
#endif

	DevRunInfoInit();

}  

void protocolDemo(int thid)
{
	myprintf(thid, LOG_ATTR_INFO, "This is a protocol demo.");
	for(;;)
	{
	   thSleep(100);
	}
}

struct VProtocol *GetProtocol(const char *name, int attr)
{
    int i;
	struct VProtocol *pcol;

	if (attr) pcol = g_SecProtocol;
	else pcol = g_PriProtocol;

    for (i=0; i<MAXPROTOCOLNUM; i++)
    {
		if (strcmp(name, "") == 0) break;
		else if (strcmp(name, pcol->name) == 0)  return pcol;

		pcol++;			
	}

	return NULL;
}

int ProtocolInit(void)
{
    int i;

    memset(g_PriProtocol, 0, sizeof(g_PriProtocol));
	memset(g_SecProtocol, 0, sizeof(g_SecProtocol));

    for (i=0; i<MAXPROTOCOLNUM; i++)
    {
        g_PriProtocol[i].entry = (ENTRYPTR)protocolDemo;
        g_SecProtocol[i].entry = (ENTRYPTR)protocolDemo;		
	}	

	i = 0;
	
	strcpy(g_PriProtocol[i].name, "维护");
	strcpy(g_SecProtocol[i].name, "维护");
	strcpy(g_PriProtocol[i].entryName, "maint");
	strcpy(g_SecProtocol[i].entryName, "maint");
	g_PriProtocol[i].entry = (ENTRYPTR)maint;
	g_SecProtocol[i].entry = (ENTRYPTR)maint;		
	i++;

#if (TYPE_IO != IO_TYPE_EXTIO)
	strcpy(g_PriProtocol[i].name, "GB104");
	strcpy(g_SecProtocol[i].name, "GB104");
	strcpy(g_PriProtocol[i].entryName, "gb104m");
	strcpy(g_SecProtocol[i].entryName, "gb104s");
#ifdef INCLUDE_GB104_M
	g_PriProtocol[i].entry = (ENTRYPTR)gb104m;
#endif
#ifdef INCLUDE_GB104_S
	g_SecProtocol[i].entry = (ENTRYPTR)gb104s;	
#endif
	i++;

	strcpy(g_PriProtocol[i].name, "GB101");
	strcpy(g_SecProtocol[i].name, "GB101");
	strcpy(g_PriProtocol[i].entryName, "gb101m");
	strcpy(g_SecProtocol[i].entryName, "gb101s");
#ifdef INCLUDE_GB101_M
	g_PriProtocol[i].entry = (ENTRYPTR)gb101m;
#endif
#ifdef INCLUDE_GB101_S
	g_SecProtocol[i].entry = (ENTRYPTR)gb101s;	
#endif
	i++;

	strcpy(g_PriProtocol[i].name, "MODBUS");
	strcpy(g_SecProtocol[i].name, "MODBUS");
	strcpy(g_PriProtocol[i].entryName, "modbusm");
	strcpy(g_SecProtocol[i].entryName, "modbuss");
#ifdef INCLUDE_MODBUS_M
	g_PriProtocol[i].entry = (ENTRYPTR)modbusm;
#endif
	//g_SecProtocol[i].entry = (ENTRYPTR)modbuss;		
	i++;

#ifdef INCLUDE_COMMTEST
	strcpy(g_PriProtocol[i].name, "CT");
	strcpy(g_SecProtocol[i].name, "CT");
	strcpy(g_PriProtocol[i].entryName, "ctpri");
	strcpy(g_SecProtocol[i].entryName, "ctsec");
	g_PriProtocol[i].entry = (ENTRYPTR)ctpri;
	g_SecProtocol[i].entry = (ENTRYPTR)ctsec;
	i++;
#endif

	strcpy(g_PriProtocol[i].name, "GPS");
	strcpy(g_SecProtocol[i].name, "GPS");
	strcpy(g_PriProtocol[i].entryName, "gps");
	strcpy(g_SecProtocol[i].entryName, "gps");
#if (TYPE_OS == OS_RT)
	g_PriProtocol[i].entry = (ENTRYPTR)gps;
	g_SecProtocol[i].entry = (ENTRYPTR)gps;		
#endif	
	i++;	

    strcpy(g_PriProtocol[i].name, "GB103");
    strcpy(g_SecProtocol[i].name, "GB103");
    strcpy(g_PriProtocol[i].entryName, "gb103m");
    strcpy(g_SecProtocol[i].entryName, "gb103s");
#if (TYPE_OS == OS_RT)
    g_PriProtocol[i].entry = (ENTRYPTR)gb103m;
#endif
    i++;

#ifdef INCLUDE_HMI400
	strcpy(g_PriProtocol[i].name, "MMI");
	strcpy(g_SecProtocol[i].name, "MMI");
	strcpy(g_PriProtocol[i].entryName, "hmi400");
	strcpy(g_SecProtocol[i].entryName, "hmi400");
#if (TYPE_OS == OS_RT)
	g_PriProtocol[i].entry = (ENTRYPTR)hmi400;
	g_SecProtocol[i].entry = (ENTRYPTR)hmi400;
#endif
	i++;
#endif

#if (TYPE_USER == USER_SHANGHAIJY)
    strcpy(g_PriProtocol[i].name, "PN103NET");
    strcpy(g_SecProtocol[i].name, "PN103NET");
    strcpy(g_PriProtocol[i].entryName, "pn103net");
    strcpy(g_SecProtocol[i].entryName, "pn103net");
#if (TYPE_OS == OS_RT)
    g_PriProtocol[i].entry = (ENTRYPTR)pn103m;
    g_SecProtocol[i].entry = (ENTRYPTR)pn103m;
#endif
    i++;

    strcpy(g_PriProtocol[i].name, "SF103NET");
    strcpy(g_SecProtocol[i].name, "SF103NET");
    strcpy(g_PriProtocol[i].entryName, "sf103net");
    strcpy(g_SecProtocol[i].entryName, "sf103net");
#if (TYPE_OS == OS_RT)
    g_PriProtocol[i].entry = (ENTRYPTR)sf103m;
    g_SecProtocol[i].entry = (ENTRYPTR)sf103m;
#endif
    i++;

	strcpy(g_PriProtocol[i].name, "NR103NET");
	strcpy(g_SecProtocol[i].name, "NR103NET");
	strcpy(g_PriProtocol[i].entryName, "nr103net");
	strcpy(g_SecProtocol[i].entryName, "nr103net");
#if (TYPE_OS == OS_RT)
	g_PriProtocol[i].entry = (ENTRYPTR)NR103m;
	g_SecProtocol[i].entry = (ENTRYPTR)NR103m;
#endif
	i++;

	strcpy(g_PriProtocol[i].name, "IPACS103");
	strcpy(g_SecProtocol[i].name, "IPACS103");
	strcpy(g_PriProtocol[i].entryName, "IPACS103m");
	strcpy(g_SecProtocol[i].entryName, "IPACS103s");
#if (TYPE_OS == OS_RT)
	g_PriProtocol[i].entry = (ENTRYPTR)ipacs103m;
#endif
	i++;

	strcpy(g_PriProtocol[i].name, "XJ103");
	strcpy(g_SecProtocol[i].name, "XJ103");
	strcpy(g_PriProtocol[i].entryName, "XJ103m");
	strcpy(g_SecProtocol[i].entryName, "XJ103s");
#if (TYPE_OS == OS_RT)
	g_PriProtocol[i].entry = (ENTRYPTR)xj103m;
#endif
	i++;

		strcpy(g_PriProtocol[i].name, "CY103NET");
		strcpy(g_SecProtocol[i].name, "CY103NET");
		strcpy(g_PriProtocol[i].entryName, "cy103net");
		strcpy(g_SecProtocol[i].entryName, "cy103net");
#if (TYPE_OS == OS_RT)
		g_PriProtocol[i].entry = (ENTRYPTR)cy103m;
#endif
		i++;
#if 0
		strcpy(g_PriProtocol[i].name, "SAC103");
		strcpy(g_SecProtocol[i].name, "SAC103");
		strcpy(g_PriProtocol[i].entryName, "SAC103m");
		strcpy(g_SecProtocol[i].entryName, "SAC103s");
#if (TYPE_OS == OS_RT)
		g_PriProtocol[i].entry = (ENTRYPTR)sac103m;
#endif
	   i++;
#endif

#endif

#ifdef INCLUDE_GZ101_M
	strcpy(g_PriProtocol[i].name, "MDCU101");
	strcpy(g_SecProtocol[i].name, "MDCU101");
	strcpy(g_PriProtocol[i].entryName, "MDCU101m");
	strcpy(g_SecProtocol[i].entryName, "MDCU101s");
#if (TYPE_OS == OS_RT)
	g_PriProtocol[i].entry = (ENTRYPTR)mdcu101m;
	//g_SecProtocol[i].entry = (ENTRYPTR)gb104shs;	
#endif
	i++;
#endif


#ifdef INCLUDE_FA_SH
	strcpy(g_PriProtocol[i].name, "GB104sh");
	strcpy(g_SecProtocol[i].name, "GB104sh");
	strcpy(g_PriProtocol[i].entryName, "Gb104shm");
	strcpy(g_SecProtocol[i].entryName, "Gb104shs");
#if (TYPE_OS == OS_RT)
	g_PriProtocol[i].entry = (ENTRYPTR)gb104shm;
	g_SecProtocol[i].entry = (ENTRYPTR)gb104shs;	
#endif
	i++;
#endif


#ifdef INCLUDE_CDT
		strcpy(g_PriProtocol[i].name, "CDT92");
		strcpy(g_SecProtocol[i].name, "CDT92");
		strcpy(g_PriProtocol[i].entryName, "CDT92m");
		strcpy(g_SecProtocol[i].entryName, "CDT92s");
#if (TYPE_OS == OS_RT)
		g_PriProtocol[i].entry = (ENTRYPTR)cdt92m;
		g_SecProtocol[i].entry = (ENTRYPTR)cdt92s;		
#endif	
		i++;
#endif
#ifdef INCLUDE_FA_S  
		strcpy(g_PriProtocol[i].name, "MDCP");
		strcpy(g_SecProtocol[i].name, "MDCP");
		strcpy(g_PriProtocol[i].entryName, "mdcp");
		strcpy(g_SecProtocol[i].entryName, "mdcp");
#if (TYPE_OS == OS_RT)
		g_PriProtocol[i].entry = (ENTRYPTR)mdcp;
		g_SecProtocol[i].entry = (ENTRYPTR)mdcp;		
#endif		
		i++;
#endif
#if(DEV_SP == DEV_SP_TTU)
		strcpy(g_PriProtocol[i].name, "DLQ");
		strcpy(g_SecProtocol[i].name, "DLQ");
		strcpy(g_PriProtocol[i].entryName, "DLQm");
		strcpy(g_SecProtocol[i].entryName, "DLQm");
#if (TYPE_OS == OS_RT)
		g_PriProtocol[i].entry = (ENTRYPTR)dlqm;
		g_SecProtocol[i].entry = (ENTRYPTR)dlqm;		
#endif	
		i++;
#endif

#ifdef INCLUDE_PR_DIFF
			strcpy(g_PriProtocol[i].name, "EXTDIFF");
			strcpy(g_SecProtocol[i].name, "EXTDIFF");
			strcpy(g_PriProtocol[i].entryName, "extdiff");
			strcpy(g_SecProtocol[i].entryName, "extdiff");
#if (TYPE_OS == OS_RT)
			g_PriProtocol[i].entry = (ENTRYPTR)extdiffm;
			g_SecProtocol[i].entry = (ENTRYPTR)extdiffm;		
#endif	
			i++;
#endif

#endif

	return i;
}

void LoginRunInfo(int thid, char *tname)
{
	if (((thid >COMM_SERIAL_START_NO) && (thid <COMM_CAN_STARTNO)) ||
		(thid >= COMM_NET_USR_NO))
	{
		myprintf(SYS_ID, LOG_ATTR_INFO, "%s run on %s.", tname, g_Task[thid].CommCfg.Port.cfgstr[0]); 

	}
	else		
		myprintf(SYS_ID, LOG_ATTR_INFO, "%s running...",tname); 
} 


/*------------------------------------------------------------------------
 Procedure:     SetupCommonTask ID:1
 Purpose:       建立一个普通任务
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/               
void  SetupCommonTask(int thid,char *tName,int nPriority,int nStackSize,ENTRYPTR entryPtr,int nQLen)
{
    if (thCreate(tName,entryPtr,thid,nStackSize,nPriority,nQLen) == ERROR)
    {
        myprintf(SYS_ID, LOG_ATTR_ERROR, "Fatal error on thCreate!");
	}
	else
	{
	    if (thStart(thid, COMMONDOGTM)==OK)
	        LoginRunInfo(thid,tName);    
	    else
	    {
	        myprintf(SYS_ID, LOG_ATTR_ERROR, "Fatal error on thStart!");	
	    }
	}	
} 

/*------------------------------------------------------------------------
 Procedure:     SpecialTaskSetup ID:1
 Purpose:       特殊任务建立
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void SpecialTaskSetup(struct VThSetupInfo* pthSetupInfo,int thid)
{
    struct VThSetupInfo *pInfo;

    pInfo = pthSetupInfo;
	while (pInfo->thid >= 0)
	{
		if (pInfo->thid == thid) break;
		pInfo++;
	}

	if (pInfo->thid < 0) return;

	SetupCommonTask(thid,pInfo->tName,pInfo->nPriority,pInfo->nStackSize,pInfo->entryPtr,pInfo->nQLen);
}

void CommNo2MaintNo(int portno, int *maint_portno, char *maint_portno_name_ch, char *maint_portno_name_en)
{
	int no;

	if (portno < COMM_START_NO)
	{
		if (maint_portno != NULL) *maint_portno = portno;
		if (maint_portno_name_ch != NULL)
			sprintf(maint_portno_name_ch, "系统端口-%d", portno);
		if (maint_portno_name_en != NULL)
			sprintf(maint_portno_name_en, "SysPort-%d", portno);
	}
	else if (portno < COMM_NET_START_NO)
	{
		no = portno-COMM_START_NO;
		if (maint_portno != NULL) *maint_portno = no;
		if (maint_portno_name_ch != NULL)
			sprintf(maint_portno_name_ch, "串口-%d", no);
		if (maint_portno_name_en != NULL)
			sprintf(maint_portno_name_en, "Com-%d", no);
	}
	else if (portno < COMM_NET_USR_NO)
	{
		no = (portno-COMM_NET_START_NO)+1;
		if (maint_portno != NULL) *maint_portno = no;
		if (maint_portno_name_ch != NULL)
			sprintf(maint_portno_name_ch, "系统网络连接-%d", no);
		if (maint_portno_name_en != NULL)
			sprintf(maint_portno_name_en, "SysNet-%d", no);
	}
	else
	{
		no = (portno-COMM_NET_USR_NO)+1;
		if (maint_portno != NULL) *maint_portno = no;
		if (maint_portno_name_ch != NULL)
			sprintf(maint_portno_name_ch, "网络连接-%d", no);
		if (maint_portno_name_en != NULL)
			sprintf(maint_portno_name_en, "Net-%d", no);
	}				
}

void CommTaskSetup(int no)
{
	char tname[30];
    struct VProtocol *protocol;
	char portname_ch[SYS_LOG_MSGLEN];
	char portname_en[SYS_LOG_MSGLEN];
	char msg[2*SYS_LOG_MSGLEN];

	if (g_Task[no].CommCfg.Port.id == 0)  return;

	/*hsda proc*/
	if (strcmp(g_Task[no].CommCfg.Port.pcol, "") == 0) return;

	protocol = GetProtocol(g_Task[no].CommCfg.Port.pcol, g_Task[no].CommCfg.Port.pcol_attr);
	if (protocol == NULL)  
	{
		CommNo2MaintNo(no, NULL, portname_ch, portname_en);
		sprintf(msg, "%s未知规约", portname_ch);
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, msg);
		sprintf(msg, "%s has invalid protocol!", portname_en);		
		myprintf(SYS_ID, LOG_ATTR_WARN, msg);
		return;
	}
	strcpy(tname,"t");
	strcat(tname, protocol->entryName);
	tname[1] = (char)toupper(tname[1]);	
	sprintf(tname,"%s%d",tname, no-COMM_START_NO);

	SetupCommonTask(no, tname, COMMONPRIORITY+no-COMM_START_NO, COMMONSTACKSIZE, protocol->entry, COMMONQLEN);	
}

/*------------------------------------------------------------------------
 Procedure:     TaskSetup ID:1
 Purpose:       任务建立
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void TaskSetup(void)
{
    int i;

	ProtocolInit();

	for (i=COMM_START_NO; i<COMM_START_NO+COMM_NUM; i++)
		CommTaskSetup(i);
}       

void TaskDogMon(int interval)
{
	int i;

	for (i=COMM_START_NO+1; i<COMM_START_NO+COMM_NUM; i++)
	{
#ifdef SHELL_2_COM
		if (i == SHELL_ID) continue;
#endif
		if (thRunDog(i, interval) == TRUE)
		{
		    sysRestart(COLDRESTART, SYS_RESET_LOCAL, GetThName(i));
		}
	}	
}


#if (TYPE_IO != IO_TYPE_EXTIO)

void ResetShow(void)
{
	struct VSysClock  SysClock;
	struct VSysResetInfo *pInfo, rinfo;
	struct VCalClock calclock;

	GetSysClock((void *)&calclock, CALCLOCK);
	
	pInfo = &g_Sys.ResetInfo;

	extNvRamGet(NVRAM_AD_RESET, (BYTE *)pInfo, sizeof(struct VSysResetInfo));
	if (pInfo->code == SYS_RESET_LOCAL)
	{
		WriteDoEvent(&calclock, 0, "装置上电,原因:本地复位 %s", pInfo->cause);
		WriteUlogEvent(&calclock, 01, 1,"Restart! Reason: local reset.", pInfo->cause);
		CalClockTo(&pInfo->clock, &SysClock);		
		myprintf(SYS_ID, LOG_ATTR_INFO, "%04d-%02d-%02d %02d:%02d:%02d Local reset, for %s.", \
						SysClock.wYear, SysClock.byMonth, SysClock.byDay,  \
						SysClock.byHour, SysClock.byMinute, SysClock.bySecond, pInfo->cause);			
	}	
	else if (pInfo->code == SYS_RESET_REMOTE)
	{
		WriteDoEvent(&calclock, 0, "装置上电,原因:远方复位, %s", pInfo->cause);
		WriteUlogEvent(&calclock, 01, 1,"Restart! Reason: remote reset.", pInfo->cause);
		CalClockTo(&pInfo->clock, &SysClock);		
		myprintf(SYS_ID, LOG_ATTR_INFO, "%04d-%02d-%02d %02d:%02d:%02d Remot reset, form %s.", \
			SysClock.wYear, SysClock.byMonth, SysClock.byDay,  \
			SysClock.byHour, SysClock.byMinute, SysClock.bySecond, pInfo->cause);			
	}	
	else
	{
		WriteDoEvent(&calclock, 0, "装置上电,原因:失电");
		WriteUlogEvent(&calclock, 01, 1,"Restart! Reason: power off.");
	}

    CalClockTo(&calclock, &SysClock);

	myprintf(SYS_ID, LOG_ATTR_INFO, "%04d-%02d-%02d %02d:%02d:%02d System running...",SysClock.wYear, \
					SysClock.byMonth,SysClock.byDay,SysClock.byHour, \
					SysClock.byMinute,SysClock.bySecond);	

    rinfo.code = SYS_RESET_POWER;
	rinfo.cause[0] = '\0';
	rinfo.clock = calclock;	
	extNvRamSet(NVRAM_AD_RESET, (BYTE *)&rinfo, sizeof(rinfo));
}

#endif

void sysVerCrcShow(int thid)
{
	BYTE svflag,hvflag;

	extNvRamGet(NVRAM_DEV_SV_FLAG, &svflag, 1);
	extNvRamGet(NVRAM_DEV_HV_FLAG, &hvflag, 1);
	if(hvflag == 0x5A)
		extNvRamGet(NVRAM_DEV_HV,(BYTE*)sysHVer,PARACHAR_MAXLEN);
	if(svflag == 0x5A)
		extNvRamGet(NVRAM_DEV_SV,(BYTE*)sysSVer,PARACHAR_MAXLEN);
    myprintf(thid, LOG_ATTR_INFO, "版本:%s %s 设备:%s.", sysHVer, sysSVer, sysSp);
	myprintf(thid, LOG_ATTR_INFO, "用户:%s 校验:0x%08x.", sysUser, sysCrc);
}

VPrGzEvtOpt *prGzEvtOpt = NULL;

BOOL prGzEventInit()
{
	prGzEvtOpt = (VPrGzEvtOpt*)malloc(sizeof(VPrGzEvtOpt));
	if(prGzEvtOpt == NULL) return ERROR;
	memset(prGzEvtOpt,0,sizeof(VPrGzEvtOpt));
}

int prReadGzEvent(VDBGzEvent* pGzEvt)
{
#ifdef INCLUDE_PR
	VDBGzEvent *pevt;
	if(prGzEvtOpt == NULL)
		return ERROR;
		
	if(prGzEvtOpt->wReadPtr != prGzEvtOpt->wWritePtr)
	{
		pevt = prGzEvtOpt->evt + prGzEvtOpt->wReadPtr;
		memcpy(pGzEvt,pevt,sizeof(VDBGzEvent));
		prGzEvtOpt->wReadPtr = (prGzEvtOpt->wReadPtr+1)&(MAX_GZEVT_NUM-1);
		return OK;
	}
	else
#endif
		return ERROR;
}

void prWriteGzEvent(VDBGzEvent* pGzEvt)
{
	int k,i;
	DWORD pdI[4],pdU[4];
	//struct VDBYC DBYC;
	DWORD *pfI,*pfU;
	
  	VDBGzEvent *pevt;
	pevt = prGzEvtOpt->evt + prGzEvtOpt->wWritePtr;
	memcpy(pevt , pevt ,sizeof(VDBGzEvent));
	prGzEvtOpt->wWritePtr = (prGzEvtOpt->wWritePtr+1)&(MAX_GZEVT_NUM-1);
	
}

void WriteBmEvent()
{
	BYTE buf[1024];
	int flag = ERROR;
	VDBGzEvent* pGzEvnet;
	struct VSysEventInfo *pSysEventInfo;

	flag = ReadBmEventFlag();
	
	if(flag == ERROR)
		return;
	
	switch(flag)
	{
		case SYSEV_FLAG_ACT:
			pSysEventInfo = (struct VSysEventInfo *)buf;
			if(ReadBmEvent(buf,sizeof(struct VSysEventInfo)) != flag)
			{
				printf("ReadBmEvent  ERROR  %d \r\n",flag);
			}
			WriteSysEvent(g_Sys.pActEvent, SYSEV_FLAG_ACT, 1, pSysEventInfo);
		break;
		case SYSEV_FLAG_DO:
			pSysEventInfo = (struct VSysEventInfo *)buf;
			if(ReadBmEvent(buf,sizeof(struct VSysEventInfo)) != flag)
			{
				printf("ReadBmEvent  ERROR  %d \r\n",flag);
			}
			WriteSysEvent(g_Sys.pDoEvent, SYSEV_FLAG_DO, 1, pSysEventInfo);
		break;
		case SYSEV_FLAG_WARN:
			pSysEventInfo = (struct VSysEventInfo *)buf;
			if(ReadBmEvent(buf,sizeof(struct VSysEventInfo)) != flag)
			{
				printf("ReadBmEvent  ERROR  %d \r\n",flag);
			}
			printf("recv warn errcode  %08x  \r\n",pSysEventInfo->type);
			if (pSysEventInfo->type) SysSetErr(pSysEventInfo->type);
			evSend(MMI_ID, EV_UFLAG);	
			WriteSysEvent(g_Sys.pWarnEvent, SYSEV_FLAG_WARN, 1, pSysEventInfo);
		break;
		case SYSEV_FLAG_ULOG:
			pSysEventInfo = (struct VSysEventInfo *)buf;
			if(ReadBmEvent(buf,sizeof(struct VSysEventInfo)) != flag)
			{
				printf("ReadBmEvent  ERROR  %d \r\n",flag);
			}
			WriteSysEvent(g_Sys.pUlogEvent, SYSEV_FLAG_ULOG, 1, pSysEventInfo);
		break;
		case SYSEV_FLAG_FA:
			pSysEventInfo = (struct VSysEventInfo *)buf;
			if(ReadBmEvent(buf,sizeof(struct VSysEventInfo)) != flag)
			{
				printf("ReadBmEvent  ERROR  %d \r\n",flag);
			}
			WriteSysEvent(g_Sys.pFaEvent, SYSEV_FLAG_FA, 1, pSysEventInfo);
		break;
		case 100:
			pGzEvnet = (VDBGzEvent*)buf;
			if(ReadBmEvent(buf,sizeof(VDBGzEvent)) != flag)
			{
				printf("ReadBmEvent  ERROR  %d \r\n",flag);
			}
			prWriteGzEvent(pGzEvnet);
		break;
		default:
			printf("WriteBmEvent Flag %d ERROR \r\n",flag);
		break;
	}
	
	
}

int WritePrParaYZ(WORD parano,char *pbuf)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	return CheckParaType_Len(fParaInfo->type,fParaInfo->len);
#else
	return ERROR;
#endif	
}

int WritePrPara(WORD parano,char *pbuf,WORD type)
{
	return BMWritePrPara( parano, pbuf,type,sizeof(struct VParaInfo));
}

//从BM里面读出来，写到数据库,目前两个线差一个fa
void WriteEQPFromBM()
{
	WORD i,j;
	int ret = -1;
	WORD no,value;
	struct VCalClock tm;
	
	struct VDBSOE DBSOE;
	struct VDBCOS DBCOS;
		
	struct VDBYX DBYX;
	WORD EpqIndex[5];
	struct VYCF_L YcValue;

	EpqIndex[0] = g_Sys.wIOEqpID;
	EpqIndex[1] = g_Sys.wLinePr0EqpID;
	EpqIndex[2] = g_Sys.wLinePr1EqpID;
	EpqIndex[3] = g_Sys.wSocFaEqpID;

	for(j = BM_EQP_LINE1; j <= BM_EQP_SOCFA; j++)
	{
		for(i = 0; i < g_Sys.Eqp.pInfo[EpqIndex[j]].pTEqpInfo->Cfg.wYCNum; i++)
		{
			if(ReadBMRangeYCF_L(j, i, &YcValue.lValue) == ERROR)
				break;
			YcValue.byFlag = 0x01;
			WriteRangeYCF_L(EpqIndex[j], i, 1, &YcValue);	
		}
	
		for(i = 0; i < g_Sys.Eqp.pInfo[EpqIndex[j]].pTEqpInfo->Cfg.wSYXNum; i++)
		{
			DBYX.wNo = i;
			if(ReadBMSYX(j, DBYX.wNo, &DBYX.byValue) == ERROR)
				break;
			WriteSYX(EpqIndex[j], 1, &DBYX);	
		}
	 	
		while(1)
		{
			ret = ReadBMSOE(j, &no,&value,&tm);
			if(ret == 0) //单点
			{
				DBSOE.wNo = DBYX.wNo = DBCOS.wNo = no;
				DBSOE.byValue = DBYX.byValue = DBCOS.byValue = value;
				DBSOE.Time = tm;
				WriteSYX(EpqIndex[j], 1, &DBYX);	
				WriteSCOS(EpqIndex[j], 1, &DBCOS);
				WriteSSOE(EpqIndex[j], 1, &DBSOE);	
			}
			else
			{
				break;
			}
		}
	}
}

//初始化共享给裸核
void LinuxNvramToBM(void)
{
	BYTE buf[8*1024];
	extNvRamGet( 0, buf, 8*1024);
	extNvRamSetBM( 0, buf, 8*1024);
}

void BMNvramToLinux()
{
	//BYTE buf1[8*1024];
	//BYTE buf2[8*1024];
	//将裸核自己设置的，设置到铁电,对于一些裸核自己会修改的做对比写处理
	//extNvRamGetBM();
	//extNvRamGet();
	//if(memcmp(buf1 , buf2) == 0) return;
	//extNvRamSet();
}


//版本重新设计一下
char sysSp[30] = "TTU";
char sysHVer[30] = "HV01.01"; 
char sysSVer[30] = "SV02.041";
char sysUser[30] = "缺省";
DWORD sysCrc = 0x3303;   


