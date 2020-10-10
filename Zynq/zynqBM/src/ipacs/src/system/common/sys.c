/*------------------------------------------------------------------------
 Module:       	sys.cpp
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-09-15
 Description:   
------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys.h"
#include "para.h"

#include "plt_trace.h"                  //TRACE/ASSERT功能接口
int g_init =0;


static BYTE ParaFileBuf[MAXFILELEN];
struct VFileHead *g_pParafile;

struct VSysInfo g_Sys;  

__attribute__((section(".dyn_memory")))  unsigned char BSP_ESRAM[ESRAM_TOTAL_SIZE];

extern void protectlow(void);
extern void mywatchdog(void);
extern void dio(void);
extern void mea(void);
extern void scan_maint(void);

VTask Task_list[]=
{
    {WATCHDOG_ID,    0,   0,   10,   NULL,                 (ENTRYPTR)mywatchdog},
    {SELF_DIO_ID,   0,   0,   10,   NULL,                  (ENTRYPTR)dio},
    {SELF_MEA_ID,   0,   0,   400,   NULL,                  (ENTRYPTR)mea},
    {PRLOW_ID,   0,   0,   100,   NULL,                     (ENTRYPTR)protectlow},
    {MAINT_ID,   0,   0,   10,   NULL,                     (ENTRYPTR)scan_maint},
};

void Task_Setup(void)
{
    VTask *pInfo;
    pInfo = Task_list;
	int i ,len;

	len = sizeof(Task_list)/sizeof(VTask);
	for(i = 0; i < len ; i++)
	{
		if (pInfo->tid < 0) continue;    
		tCreate(pInfo->tid, pInfo->args, (ENTRYPTR)pInfo->func);
		if(pInfo->tmlimit)
			tmEvEvery(pInfo->tid, pInfo->tmlimit);
		pInfo++;
	}
}

/*------------------------------------------------------------------------
 Procedure:     WriteSysEvent ID:1
 Purpose:       写系统记录
 Input:         
 Output:        
 Errors:
------------------------------------------------------------------------*/
void WriteSysEvent(int flag, WORD wNum, struct VSysEventInfo *buf)
{
	struct VSysEventInfo *p=buf;
	int i;
	char name[32];

	if(flag == SYSEV_FLAG_ACT)
		sprintf(name,"动作事件");
	else if(flag == SYSEV_FLAG_DO)
		sprintf(name,"操作事件");
	else if(flag == SYSEV_FLAG_WARN)
		sprintf(name,"告警事件");
	else if(flag == SYSEV_FLAG_ULOG)
		sprintf(name,"日志事件");	
	else if(flag == SYSEV_FLAG_FA)
		sprintf(name,"FA事件");	
	
	for (i=0;i<wNum;i++)
	{
		shellprintf("%s: %s \r\n",name,p->msg);  
		WriteBmEvent( flag, (BYTE*)p, sizeof(struct VSysEventInfo));
		p++;
	} 

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
	WriteSysEvent(SYSEV_FLAG_WARN, 1, &event);
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
	WriteSysEvent(SYSEV_FLAG_DO, 1, &event);
	
}

void WriteActEvent(struct VCalClock *ptm, DWORD type, int fd, const char *fmt, ... )
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
	event.type = type;
	event.para = (WORD) fd;
	event.rsv = 0;
	WriteSysEvent(SYSEV_FLAG_ACT, 1, &event);

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
	WriteSysEvent(SYSEV_FLAG_FA, 1, &event);
}

void WriteUlogEvent(struct VCalClock *ptm, DWORD type, int value, const char *fmt, ... )
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
	event.type = type;
	event.para = (WORD)value;
	event.rsv = 0;
	WriteSysEvent(SYSEV_FLAG_ULOG, 1, &event);
}

DWORD GetIoType(void)
{
    int i;
    BYTE type;
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
	BYTE type;
	DWORD aitype;

	aitype = 0;
	for (i=0; i<BSP_AC_BOARD; i++)
	{
	    type = Get_Ai_Type(i);
		aitype |= (type << (i*8));
		if (type == 0xFF)
			break;
	}
	
	return aitype;
}

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

void WriteRunPara(BYTE *pbuf,WORD len)
{
	VRunParaCfg *pCfg = (VRunParaCfg *)&g_Sys.MyCfg.RunParaCfg;	

	if(len != sizeof(VRunParaCfg))
	{
		shellprintf("WriteRunPara Error! \n");
		return;
	}
		
	memcpy(pCfg,pbuf,len);
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

void GetSysConfig(void)
{	
    int i,j=0;	
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
	if (pCfg->wDINum > 0)
       pCfg->pDi = (VDiCfg *)calloc(pCfg->wDINum, sizeof(VDiCfg));
	if (pCfg->wDONum > 0)
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

    pCfg->pFd = NULL;
	pCfg->pAi = NULL;
	pCfg->pYc = NULL;
    if (pCfg->wFDNum > 0)
       pCfg->pFd = (VFdCfg *)calloc(pCfg->wFDNum, sizeof(VFdCfg));
	if (pCfg->wAINum > 0)
       pCfg->pAi = (VAiCfg *)calloc(pCfg->wAINum, sizeof(VAiCfg));
	if (pCfg->wYCNum > 0)
       pCfg->pYc = (VYcCfg *)calloc(pCfg->wYCNum, sizeof(VYcCfg));

    for (i=0; i<pCfg->wFDNum; i++)
    {
		strcpy(pCfg->pFd[i].kgname, pPFdCfg[i].kgname);
        pCfg->pFd[i].kgid = 100+i+1;			
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

	if (ReadParaFile("runpara.cfg", (BYTE *)g_pParafile, MAXFILELEN) == OK)
		GetRunParaConfig();
	else
		SetRunParaConfig();

    if (ReadParaFile("systemext.cfg", (BYTE *)g_pParafile, MAXFILELEN) == OK)
		GetSysExtConfig();
	else
		SetDefSysExtConfig();
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
	g_pParafile = (struct VFileHead *)ParaFileBuf;	 
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

void SetIoNo(void)
{
    int i,j,num;
	BYTE type,ua,ub,uc;
	VDefDiCfg *pDefDiCfg;	
	VDefDoCfg *pDefDoCfg;
	VDefAiCfg *pDefAiCfg;
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

void ApplicationInit(void)
{   
	int id;
	
	tInit();
	
	for (id = BSP_ESRAM_YK; id < BSP_ESRAM_YX; id++)
	 	REG8(id) = 0;
	
	//缺 led 及IO 初始化
	BSP_IO_Init();
	//
	ClockInit();
	
	PublicInit();

	SysParaInit();

	SysConfigInit();

	SetIoNo();

	ioInit();

	if (prInit() == ERROR)  
		myprintf(SYS_ID, LOG_ATTR_ERROR, "prInit init error!");

	Task_Setup();
	
   	maint();
	
	Print_fpga_eth_info();
	Print_fpga_goose_rcv_cfg();

	uartns550_int(0);
	uartns550_int(1);

	/*float I[4], U[4];
	float temp;
	DWORD tick = 0;
	while(1)
	{
		temp = tick*0.00000001;
		I[0] = 1.002217 + temp;
		I[1] = 0+temp;
		I[2] = 0.001364335 + temp;
		I[3] = 1.001145 + temp;
		U[0] = 0;
		U[1] = 0.02014046 + temp;
		U[2] = 0;
		U[3] = 0.02014017 +temp;
		
		 x_printf_with_log("Ua=%3.2lfV,Uc=%3.2lfV,U0=%3.2lfV,Ia=%3.2lfA,Ib=%3.2lfA,Ic=%3.2lfA,I0=%3.2lfA\r\n",
						(double)U[0],(double)U[2],(double)U[3],(double)I[0], (double)I[1], (double)I[2],(double)I[3]);

	   tick++;
	   sleep(1);
	}*/
	g_init = 1;
}


