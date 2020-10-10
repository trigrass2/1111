/*------------------------------------------------------------------------
 Module:       	dbhis.c
 Author:        helen
 Project:       
 State:			
 Creation Date:	2016-09-07
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $默认只有一个主站，一个虚装置，只针对第一个虚装置生成历史数据
------------------------------------------------------------------------*/

#include <syscfg.h>
#include <stdio.h>
#include "sys.h"
#include "File.h"
#include "FileExt.h"
#include "Dbhis.h"
#ifdef INCLUDE_HIS
static char *FileHead = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
static char *DataHead = "<DataFile>\r\n";
static char *DataTail = "</DataFile>";
static char *FileData1 = "  <Header fileType=\"%s\" fileVer=\"1.00\" devName=\"%s\"/>\r\n";
static char *FileData2 = "  <Header fileType=\"%s\" fileVer=\"1.00\" devID=\"%s\"/>\r\n";
static char *RecHead1  = "  <DataRec num=\"%04d\">\r\n";
static char *RecHead2  = "  <DataRec>\r\n";
static char *RecTail   = "  </DataRec>\r\n";
static char *AttrHead1 = "  <DataAttr num=\"%d\">\r\n";
static char *AttrHead2 = "  <DataAttr dataNum=\"%d\" sectNum=\"%02d\" interval=\"15min\">\r\n";
static char *AttrHead3 = "  <DataAttr type=\"FixFrz\" dataNum=\"%d\" sectNum= \"%02d\" interval=\"15min\">\r\n";
static char *AttrHead4 = "  <DataAttr type=\"DayFrz\" dataNum=\"%d\">\r\n";
static char *AttrTail  = "  </DataAttr>\r\n";

static char *FileType[]={"/tffsb/HISTORY/SOE/soe.xml", 
	                     "/tffsb/HISTORY/CO/co.xml", 
	                     "/tffsb/HISTORY/EXV/exv%04d%02d%02d.xml", 
	                     "/tffsb/HISTORY/FIXPT/fixpt%04d%02d%02d.xml", 
	                     "/tffsb/HISTORY/FRZ/frz%04d%02d%02d.xml",
	                     "/tffsb/HISTORY/FLOWREV/flowrev.xml", 
	                     "/tffsb/HISTORY/ULOG/ulog.xml"};

static char* HeaderFileType[] = {"SOE",
										"CO",
										"EXV",
										"FIXPT",
										"FRZ",
										"FLOWREV",
										"Ulog"};


static char *his_rcdfile = "/tffsb/cfg/hisrcd.cfg";

static char *his_buf;
#define his_buf_len 2048
static struct VHisDllMan *his_event;
static struct VHisDllMan *his_hour, *his_day;
static int his_rptr, his_wptr;
//static DWORD his_sem;
static struct VHisSocket his_event_socket[DBHIS_SOCKET_MAX];
#define his_sockets_temp 20
static int his_wptr_temp = 0;
static struct VHisSocket his_event_sockets_temp[his_sockets_temp];
static BOOL dbhis_init = 0;
static char *eventbuf;
static DWORD his_event_ary[DBHIS_SOCKET_MAX>>2];
static struct VHisRcd *his_rcd_cfg=NULL;
int his_cfg_change = 0;

extern WORD atomicReadVEDD(struct VVirtualEqp *pVEqp, WORD wNum, WORD wBufLen,struct VDBDD *buf);
extern WORD atomicReadTEDD(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBDD *buf,BOOL asSend);//删
extern WORD atomicReadVEYCF_L(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBYCF_L *buf);


static void his_Record_Write(struct VHisDllMan *pdllman, struct VSysClock *pclk);

static void hisDll_NameGet(struct VHisDll *pdll, struct VSysClock *pclk, char *fname)
{
    if ((pdll->type < 0) ||(pdll->type >= DBhis_type_num)) 
    {
       strcpy(fname, "/tffsb/error");
	   return;
    }
	
    if ((pdll->type == DBhis_type_exv)
				||(pdll->type == DBhis_type_fixpt)
				||(pdll->type == DBhis_type_frz))
	    sprintf(fname, FileType[pdll->type], pclk->wYear, pclk->byMonth, pclk->byDay);
	else 
		strcpy(fname, FileType[pdll->type]);
		
}

/*是否需要发现cfg不对后重新读取SOE,CO,FLOW,ULOG重新生成*/
static void hisDll_Scan(char *fname, int type, struct VHisRcd *pcfg)
{
    FILE *fp;
	struct stat st;

	fp = fopen(fname, "r+b");
	if (fp == NULL)
	   return;
	
    fclose(fp);
	stat(fname, &st);


	if ((pcfg->num == 0)||
		(pcfg->num_wp == 0)||
		(pcfg->file_head == 0)||
		(pcfg->file_wp == 0)||
		(pcfg->file_size == 0))
	{
	    remove(fname);
		return;
	}
}

static struct VHisDllMan *hisDll_Get(WORD wID, int flag, struct VHisDllMan *pdllman)
{
	int type;
	struct VHisDllMan *p;

	if ((flag == SYSEV_FLAG_SSOE) || (flag == SYSEV_FLAG_DSOE))
			type = DBhis_type_soe;
	else if (flag == SYSEV_FLAG_CO)
			type = DBhis_type_co;
	else if (flag == SYSEV_FLAG_FLOW)
			type = DBhis_type_flowrev;
	else if (flag == SYSEV_FLAG_ULOG)
			type = DBhis_type_ulog;
	else
			return NULL;

	for (p = pdllman; p!=NULL; p=p->next)
	{
		if (p->dll.type == type)
		{
			if ((type == DBhis_type_ulog) || (type == DBhis_type_flowrev))
			break;
			else if (p->id == wID)
			break;
			if((type == DBhis_type_exv) || (type == DBhis_type_fixpt) || (type == DBhis_type_frz))
			break;
		}
	}
	return p;
}
static struct VHisDllMan *hisDllEXVFIXP_Get(WORD wID, int flag, struct VHisDllMan *pdllman)
{
	int type;
	struct VHisDllMan *p;
	
	if(flag  >= DBhis_type_num) return NULL;
	type = flag;
	for (p = pdllman; p!=NULL; p=p->next)
	{
		if (p->dll.type == type)
		{
			if((type == DBhis_type_exv) || (type == DBhis_type_fixpt) || (type == DBhis_type_frz))
			break;
		}
	}
	return p;
}

static int hisDll_Freeze(DWORD minute, struct VSysClock *pclk, struct VHisDllMan *pdllman)
{  
    struct VHisDllMan *p;
    
	for (p = pdllman; p!=NULL; p=p->next)
    {		
        if (p->dll.md == 0) continue;

        if (pdllman == his_hour)
		{
		    if ((minute%p->dll.md) != 0) continue;
        }  
		if((pdllman == his_day) && (p->dll.type != DBhis_type_exv))
		{
			p->index->num--; // Dayfrz 减一，会写到frz文件num+1，所以减一
		}
		his_Record_Write(p, pclk);		
	}
	
    return OK;    
}

static void hisDll_hour(struct VSysClock *pclk)
{
	DWORD minute;
	minute = pclk->byMinute;
    hisDll_Freeze(minute, pclk, his_hour);
}

static void hisDll_day(struct VSysClock *pclk)
{
    struct VSysClock tmp_clock;
	DWORD minute;

	memcpy(&tmp_clock, pclk, sizeof(tmp_clock));	
	tmp_clock.byHour = 0;
	tmp_clock.byMinute = 0;

	minute = CalendarClock(&tmp_clock);
	minute -= 24*60;	/*last day*/
	SystemClock(minute,&tmp_clock);
	hisDll_Freeze(minute, &tmp_clock, his_day);
}

/*SOE, CO, flow, ulog*/
static int hisDll_RecCreate(FILE* fp, struct VHisDll *pdll, struct VHisRcd *pcfg)
{
	 char temp[128];

     fwrite(FileHead, 1, strlen(FileHead), fp);
	 fwrite(DataHead, 1, strlen(DataHead), fp);
	 if (pdll->type == DBhis_type_ulog)
	 	sprintf(temp, FileData2, HeaderFileType[pdll->type], g_Sys.InPara[SELFPARA_ID]);  // 文件名
	 else
	    sprintf(temp, FileData1, HeaderFileType[pdll->type], g_Sys.InPara[SELFPARA_DEVTYPE]); // 文件名
	 fwrite(temp, 1, strlen(temp), fp);
	 
	 sprintf(temp, RecHead1, 0);
	 fwrite(temp, 1, strlen(temp), fp);
	 pcfg->num = 0;
	 pcfg->num_wp = lfseek(fp, 0, SEEK_CUR) - 8;
	 pcfg->file_wp = pcfg->num_wp+8;
	 pcfg->file_head = pcfg->file_wp;
	 pcfg->file_size = pcfg->file_head;
	 return (pcfg->file_wp);
}

/*Exv, FIxP, 需增加虚装置遥测结构*/
static int hisDll_AttrCreate(FILE* fp, struct VHisDll *pdll, struct VHisRcd *pcfg)
{
   char temp[128];
	 char temp1[5];
	 int i,j,num, len;
	 struct VHisDllMan* pdllman;
	 struct VVirtualEqp *peqp;
	 struct VTrueEqp *pTeqp;
	 struct VTrueYCCfg *pYcCfg;
	 struct VSysClock tmp_clock;	
	 DWORD minute;
	 WORD wSendNum;
	
	 GetSysClock(&tmp_clock,SYSCLOCK);
	 tmp_clock.byHour = 0;
	 tmp_clock.byMinute = 0;
	 minute = CalendarClock(&tmp_clock);
	 if(pdll->type == DBhis_type_exv)
	 	minute -= 24*60;	/*last day*/
	 SystemClock(minute,&tmp_clock);
	
	 fwrite(FileHead, 1, strlen(FileHead), fp);
	 fwrite(DataHead, 1, strlen(DataHead), fp);
		 	
	 sprintf(temp, FileData1, HeaderFileType[pdll->type], g_Sys.InPara[SELFPARA_DEVTYPE]);
	 fwrite(temp, 1, strlen(temp), fp);
	 
	 if(pdll->type == DBhis_type_exv)
			pdllman = hisDllEXVFIXP_Get(g_Sys.wIOEqpID,pdll->type,his_day);
	 else 
		  pdllman = hisDllEXVFIXP_Get(g_Sys.wIOEqpID,pdll->type,his_hour);
	 
	 if(pdllman == NULL) return 0;
	 peqp = g_Sys.Eqp.pInfo[pdllman->id].pVEqpInfo;
	 if(peqp == NULL) return 0;
	 num = 0;
	 if(pdll->type == DBhis_type_exv)
	 {
		 for(j = 0; j < peqp->YCHourFrz.wNum ; j++)
		 {
               i = peqp->YCHourFrz.pwIndex[j];
			 if(!(peqp->pYC[i].dwCfg & 0x800)) continue; 
			 pTeqp = g_Sys.Eqp.pInfo[peqp->pYC[i].wTEID].pTEqpInfo;
			 pYcCfg = pTeqp->pYCCfg + peqp->pYC[i].wOffset;
			 if(!(pYcCfg->dwCfg & 0x100)) continue; //虚遥测对应的实遥测必须配置极值记录
			 num++;
		 }
	 }
	 else if(pdll->type == DBhis_type_fixpt)
		 num = peqp->YCHourFrz.wNum;
	 
	 if (pdll->type == DBhis_type_exv)
	 	sprintf(temp, AttrHead1, num);
	 else 
	 	sprintf(temp, AttrHead2, num, pdll->max);
	 fwrite(temp, 1,  strlen(temp), fp);
	 pcfg->num_wp = lfseek(fp, 0, SEEK_CUR) - 23;
	 
	 if(pdll->type == DBhis_type_exv)
	 {
		 for(j = 0; j < peqp->YCHourFrz.wNum ; j++)
		 {
                i = peqp->YCHourFrz.pwIndex[j];
			 if(!(peqp->pYC[i].dwCfg & 0x800)) continue;
			 pTeqp = g_Sys.Eqp.pInfo[peqp->pYC[i].wTEID].pTEqpInfo;
			 pYcCfg = pTeqp->pYCCfg + peqp->pYC[i].wOffset;
			 if(!(pYcCfg->dwCfg & 0x100)) continue;
			 ReadYCSendNo(pdllman->id,i,&wSendNum);
			 memcpy(temp1, pYcCfg->tycunit, sizeof(pYcCfg->tycunit));
			if(strcmp(temp1,"Kw") == 0)
				strcpy(temp1,"W");
			if(strcmp(temp1,"Kvar") == 0)
				strcpy(temp1,"Var");
			if(strcmp(temp1,"KVA") == 0)
				strcpy(temp1,"VA");
			if((strcmp(temp1," ℃") == 0)||(strcmp(temp1,"°") == 0)||(strcmp(temp1,"") == 0)) // UTF-8 显示不出来cjl 2017年10月24日
				strcpy(temp1," ");
			
			 
			 sprintf(temp, "    <DI ioa=\"%d\" type=\"float\" unit=\"%s\"/>\r\n",wSendNum + YC_Addr,temp1);
			 fwrite(temp, 1, strlen(temp), fp);
		 }
	 }
	 else if(pdll->type == DBhis_type_fixpt)
	 {
			for(i=0;i<peqp->YCHourFrz.wNum;i++)
		  {
				 j = peqp->YCHourFrz.pwIndex[i];
				 pTeqp = g_Sys.Eqp.pInfo[peqp->pYC[j].wTEID].pTEqpInfo;
				 pYcCfg = pTeqp->pYCCfg + peqp->pYC[j].wOffset;
				 ReadYCSendNo(pdllman->id,j,&wSendNum); //由维护软件生成发送序号后删
				memcpy(temp1, pYcCfg->tycunit, sizeof(pYcCfg->tycunit));
				if(strcmp(temp1,"Kw") == 0)
					strcpy(temp1,"W");
				if(strcmp(temp1,"Kvar") == 0)
					strcpy(temp1,"Var");
				if(strcmp(temp1,"KVA") == 0)
					strcpy(temp1,"VA");
				if((strcmp(temp1," ℃") == 0)||(strcmp(temp1,"°") == 0)||(strcmp(temp1,"") == 0)) // UTF-8 显示不出来cjl 2017年10月24日
				strcpy(temp1," ");
				
				 sprintf(temp, "    <DI ioa=\"%d\" type=\"float\" unit=\"%s\"/>\r\n",wSendNum + YC_Addr,temp1);
				 fwrite(temp, 1, strlen(temp), fp);
		  }
	 }
	 
	 fwrite(AttrTail, 1, strlen(AttrTail), fp);
	 len = lfseek(fp, 0, SEEK_CUR);

	 pcfg->num = 0;
     pcfg->file_wp = len;
	 pcfg->file_head = len;
	 pcfg->file_size = len;
	 return (len);
	 	
}

/*Frz*/
static int hisDll_FrzCreate(FILE* fp, struct VHisDll *pdll, struct VHisRcd *pcfg)
{
   char temp[128];
	 int i,j,num, len;
	 struct VSysClock tmp_clock;
	 struct VVirtualEqp *peqp;
	struct VHisDllMan* pdllman;
	 
	 fwrite(FileHead, 1, strlen(FileHead), fp);
	 fwrite(DataHead, 1, strlen(DataHead), fp);
	
	 GetSysClock(&tmp_clock,SYSCLOCK);
	
	 sprintf(temp, FileData1, HeaderFileType[pdll->type], g_Sys.InPara[SELFPARA_DEVTYPE]);
	 fwrite(temp, 1, strlen(temp), fp);

	pdllman = hisDllEXVFIXP_Get(g_Sys.wIOEqpID,pdll->type,his_hour);
	 if(pdllman == NULL) return 0;
	 peqp = g_Sys.Eqp.pInfo[pdllman->id].pVEqpInfo;
	 if(peqp == NULL) return 0;
	if(peqp->Cfg.wDDNum == 0) return 0;

	 num = peqp->Cfg.wDDNum/32*8;
	 sprintf(temp, AttrHead3, num, 96);
	 fwrite(temp, 1,  strlen(temp), fp);
	 pcfg->num_wp = lfseek(fp, 0, SEEK_CUR) - 23;
	
	 for (i=0; i< peqp->Cfg.wDDNum/32; i++)
	 {
	 	//缺少地址和单位
		 for(j=0;j<2;j++)
		 {
			 sprintf(temp, "    <DI ioa=\"%d\" type=\"float\" unit=\"kWh\"/>\r\n",0x6401+j+8+i*32);
			 fwrite(temp, 1, strlen(temp), fp);
		 }
		 for(;j<8;j++)
		 {
			 sprintf(temp, "    <DI ioa=\"%d\" type=\"float\" unit=\"kVarh\"/>\r\n",0x6401+j+8+i*32);
			 fwrite(temp, 1, strlen(temp), fp);
		 }
	 }
	 fwrite(AttrTail, 1, strlen(AttrTail), fp);
	 
	 sprintf(temp, AttrHead4, num);
	 fwrite(temp, 1,  strlen(temp), fp);
	 for (i=0; i < peqp->Cfg.wDDNum/32; i++)
	 {
	 	 for(j=0;j<2;j++)
		 {
			 sprintf(temp, "    <DI ioa=\"%d\" type=\"float\" unit=\"kWh\"/>\r\n",0x6401+j+16+i*32);
			 fwrite(temp, 1, strlen(temp), fp);
		 }
		 for(;j<8;j++)
		 {
			 sprintf(temp, "    <DI ioa=\"%d\" type=\"float\" unit=\"kVarh\"/>\r\n",0x6401+j+16+i*32);
			 fwrite(temp, 1, strlen(temp), fp);
		 }
	 }
	 fwrite(AttrTail, 1, strlen(AttrTail), fp);

	 len = lfseek(fp, 0, SEEK_CUR);

	 pcfg->num = 0;
     pcfg->file_wp = len;
	 pcfg->file_head = len;
	 return len;
}

static int hisDll_SoeGet(void *pdata, char *pbuf)
{
		int len,i,j;
		int  value;
		struct VVirtualEqp *pVEqp;
		WORD wSendNum;
		struct VSysClock time;
		struct VDBSOE *psoe = (struct VDBSOE*)pdata;
		struct VDBDSOE *pdsoe;

		len = 0;

		for(i = 0;i< *g_Sys.Eqp.pwNum;i++)
		{
			if((pVEqp = g_Sys.Eqp.pInfo[i].pVEqpInfo) != NULL)
			{
				if(pVEqp->Cfg.dwFlag & 0x20) //D5
				{
					for(j = 0;j < pVEqp->Cfg.wSYXNum ;j++)
					{
						ReadSYXSendNo(i,j,&wSendNum);
						if(wSendNum == psoe->wNo)
						{
							psoe = (struct VDBSOE*)pdata;
							CalClockTo(&(psoe->Time), &time);
							if(psoe->byValue == 0x01)
								value = 0x00;
							else if(psoe->byValue == 0x81)
								value = 0x01;
							else
								value = -1;

							sprintf(pbuf, "    <DI ioa=\"%d\" tm=\"%02d%02d%02d_%02d%02d%02d_%03d\" val=\"%d\" />\r\n", 
							psoe->wNo + YX_Addr, time.wYear-2000, time.byMonth, time.byDay, time.byHour, 
							time.byMinute, time.bySecond, time.wMSecond, value);
							len = strlen(pbuf);
							return len;
						}
					}
					for(j = 0;j < pVEqp->Cfg.wDYXNum ;j++)
					{
						ReadDYXSendNo(i,j,&wSendNum);
						if(wSendNum == psoe->wNo)
						{
							pdsoe = (struct VDBDSOE*)pdata;
							CalClockTo(&(pdsoe->Time), &time);
							if((pdsoe->byValue1 == 0x01) && (pdsoe->byValue2 == 0x01))
								value = 0x00;
							else if((pdsoe->byValue1 == 0x81) && (pdsoe->byValue2 == 0x01))
								value = 0x01;
							else if((pdsoe->byValue1 == 0x01) && (pdsoe->byValue2 == 0x81))
								value = 0x02;
							else
								value = 0x03;

							sprintf(pbuf, "    <DI ioa=\"%d\" tm=\"%02d%02d%02d_%02d%02d%02d_%03d\" val=\"%d\" />\r\n", 
							pdsoe->wNo + YX_Addr, time.wYear-2000, time.byMonth, time.byDay, time.byHour, 
							time.byMinute, time.bySecond, time.wMSecond, value);
							len = strlen(pbuf);
							return len;
						}
					}
				}
			}
		}
		return len;
}

static int hisdll_CoGet(void *pdata, char *pbuf)
{
		int len;
		char command[16];
		struct VSysClock time;
		struct VDBCO *pco = (struct VDBCO*)pdata;

		CalClockTo(&pco->Time, &time);

		if (pco->cmd == MI_YKSELECT)
			strcpy(command, "select");
		else if (pco->cmd == MI_YKOPRATE)
			strcpy(command, "oper");
		else 
			strcpy(command, "cancel");
		sprintf(pbuf, "    <DI ioa=\"%d\" tm=\"%02d%02d%02d_%02d%02d%02d_%03d\" cmd=\"%s\" val=\"%d\" />\r\n",
						pco->wNo, time.wYear-2000, time.byMonth, time.byDay, time.byHour, 
						time.byMinute, time.bySecond, time.wMSecond, command, pco->val);

		len = strlen(pbuf);
		return len;
}

static int hisdll_FlowGet(void *pdata, char *pbuf)
{
		int len;
		WORD i,num;
		float fvalue;
		struct VSysClock time;
		struct VSysFlowEventInfo *pflowinfo = (struct VSysFlowEventInfo *)pdata;
		struct VDBFLOW *pflow;
		struct VHisDllMan *pdllman;
		char*p;
		
		pdllman =  hisDll_Get(0, SYSEV_FLAG_FLOW, his_event);
		
		num = pflowinfo->num;
		if(num > 0)
			pdllman->index->num += num-1;//写一次只增加一次，所以这边加上
		len = 0;
		for(i=0;i<num;i++)
		{
			p = pbuf + len;
			pflow = &pflowinfo->dbflow[i];
			CalClockTo(&pflow->Time, &time);
			fvalue = *((float*)&pflow->val);

			sprintf(p, "    <DI ioa=\"%d\" tm=\"%02d%02d%02d_%02d%02d%02d_%03d\" val=\"%.2f\" />\r\n",
							pflow->wNo+0x6401, time.wYear-2000, time.byMonth, time.byDay, time.byHour, 
							time.byMinute, time.bySecond,time.wMSecond,fvalue);

			len += strlen(p);
		}
		return len;
}

//日志类型
static int hisdll_UlogGet(void *pdata, char *pbuf)
{
     int len;
     struct VSysClock time;
	 struct VSysEventInfo *plog = (struct VSysEventInfo*)pdata;

	 CalClockTo(&plog->time, &time);

	 sprintf(pbuf, "    <DI logType=\"%02d\" tm=\"%02d%02d%02d_%02d%02d%02d_%03d\" txt=\"%s\" val=\"%d\" />\r\n",
	 	     plog->type, time.wYear-2000, time.byMonth, time.byDay, time.byHour, 
	 	     time.byMinute, time.bySecond, time.wMSecond, plog->msg, plog->para);

	 len = strlen(pbuf);
	 return len;
}

//极值记录历史文件，虚装置从实装置读取,实装置要配置极值
static int hisDll_ExvGet(void *pdata, char *pbuf)
{
     int i, len,len1;
	 float fmax,fmin;
	 DWORD dmax,dmin;
	 char *buf=pbuf;
	 struct VVirtualEqp *peqp;
	 struct VTrueEqp *pTEqp;
	 struct VVirtualYC *pVYC;
	 struct VMaxMinYC *pYC;
	 struct VTrueYCCfg *pYcCfg;
	 struct VSysClock time1, time2;
	 struct VHisInfo *pinfo = (struct VHisInfo*)pdata;
	 WORD ycno; 

	 peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	 if (peqp == NULL) return 0;

     len = 0;
	strcpy(buf,RecHead2);
	len1 = strlen(buf);
	fwrite(buf,1,len1,pinfo->fp);
	len += len1;
	memset(buf,0,len1+1);
	
	 for (ycno=0;ycno <peqp->YCHourFrz.wNum; ycno++)
	 {
		 i = peqp->YCHourFrz.pwIndex[ycno]; 
		 pTEqp = g_Sys.Eqp.pInfo[peqp->pYC[i].wTEID].pTEqpInfo;
		 pYcCfg = pTEqp->pYCCfg + peqp->pYC[i].wOffset;
		 if (!(peqp->pYC[i].dwCfg & 0x800)) continue;
		 if(!(pYcCfg->dwCfg & 0x100)) continue;
		 
		 pYC = &(pTEqp->pMaxMinYC[pYcCfg->wMaxMinNo]);
		 if(pYcCfg->wMaxMinNo == 0xFFFF) continue;//实装置没配置
		 pVYC = peqp->pYC + i;
		 
		 if(pYC->lMin == 0x7FFFFFFF)
				pYC->lMin = 0;
		 
		 CalClockTo(&pYC->max_tm, &time1);
		 CalClockTo(&pYC->min_tm, &time2);

		 dmax = pYC->lMax*pVYC->lA/pVYC->lB + pVYC->lC;
		 dmin = pYC->lMin*pVYC->lA/pVYC->lB + pVYC->lC;
		 

		 YCLongToFloat(pinfo->id,i,dmax,(BYTE*)&fmax);
		 YCLongToFloat(pinfo->id,i,dmin,(BYTE*)&fmin);
		 
	     sprintf(buf, "    <DI max=\"%.3f\" max_tm=\"%02d%02d%02d_%02d%02d%02d\" min=\"%.3f\" min_tm=\"%02d%02d%02d_%02d%02d%02d\"/>\r\n",
	 	         fmax, time1.wYear-2000, time1.byMonth, time1.byDay, time1.byHour, time1.byMinute, time1.bySecond, 
	 	         fmin, time2.wYear-2000, time2.byMonth, time2.byDay, time2.byHour, time2.byMinute, time2.bySecond);
		 len1 = strlen(buf);
		 fwrite(buf, 1, len1, pinfo->fp);
		 len += len1;
		 memset(buf,0,len1+1);
	 }

	 sprintf(buf,RecTail);
	 len1 = strlen(buf);
	 fwrite(buf,1,len1,pinfo->fp);
	 len += len1;
	 memset(buf,0,len1+1);
	 
	 return len;
}


static int hisDll_FixptGet(void *pdata, char *pbuf)
{
     int i, len,len1;
	 float f;
	 char *buf=pbuf;
	 struct VVirtualEqp *peqp;
	 struct VDBYCF_L DBYCF_L;
	 DWORD temp;
	 struct VHisInfo *pinfo = (struct VHisInfo*)pdata;

   
	 memset(his_buf,0,his_buf_len);
	 peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	 if (peqp == NULL) return 0;
	
   len = 0;
	
	 sprintf(buf,"  <DataRec sect=\"%d\" tm =\"%02d%02d%02d_%02d%02d%02d\">\r\n",pinfo->sec,pinfo->clock.wYear-2000,pinfo->clock.byMonth,
					pinfo->clock.byDay,pinfo->clock.byHour,pinfo->clock.byMinute,pinfo->clock.bySecond);
	 len1 = strlen(buf);
	 fwrite(buf, 1, len1, pinfo->fp);
	 len += len1;
	 memset(buf,0,len1+1);
	
	 for (i=0; i<peqp->YCHourFrz.wNum; i++)
	 {
	     DBYCF_L.wNo = peqp->YCHourFrz.pwIndex[i];
		atomicReadVEYCF_L(peqp, 1, sizeof(struct VDBYCF_L), &DBYCF_L);

		if(DBYCF_L.byFlag & (1<<6))
		{
			temp = DBYCF_L.lValue;
			memcpy((BYTE*)&f,(BYTE*)&temp,4);
		}
		else
		   YCLongToFloat(pinfo->id,DBYCF_L.wNo,DBYCF_L.lValue,(BYTE*)&f);
		 
		 sprintf(buf, "    <DI val=\"%.3f\"/>\r\n", f);
		  len1 = strlen(buf);
		  fwrite(buf, 1, len1, pinfo->fp);
		  len += len1;
		  memset(buf,0,len1+1);
	 }
	 
	 strcpy(buf,RecTail);

	  len1 = strlen(buf);
	  fwrite(buf, 1, len1, pinfo->fp);
	  len += len1;
	  memset(buf,0,len1+1);
	 
	 return len;
}

//冻结数据，有DDHourFrz  的配置;虚装置暂时无电度配置，暂时用实装置读取，实装置设置电度曲线
static int hisDll_FrzGet(void *pdata, char *pbuf)
{
		int i, len,len1;
		float f;
	  DWORD dwValue = 0;
		char *buf=(char*)pbuf;
		struct VVirtualEqp *peqp;
		//struct VTrueEqp *peqp1;///删
		struct VDBDD DBDD;
		struct VHisInfo *pinfo = (struct VHisInfo*)pdata;

		peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	
		/*for(i=0;i<*g_Sys.Eqp.pwNum;i++) //删
		{
			peqp1 = g_Sys.Eqp.pInfo[i].pTEqpInfo;
			if(peqp1 == NULL) continue;
			if(peqp1->Cfg.wDDNum)
				break;
		}
		
		if(peqp1 == NULL) return 0;*/
		len = len1 = 0;
		if (peqp == NULL) return 0;

		sprintf(buf, "  <DataRec sect=\"%d\" tm=\"%02d%02d%02d_%02d%02d%02d\">\r\n", pinfo->sec, 
			 pinfo->clock.wYear-2000, pinfo->clock.byMonth, pinfo->clock.byDay, pinfo->clock.byHour, pinfo->clock.byMinute, pinfo->clock.bySecond);

		 len1 = strlen(buf);
		 fwrite(buf, 1, len1, pinfo->fp);
		 len += len1;
		 memset(buf,0,len1+1);
	 
		for (i=0; i<peqp->DDHourFrz.wNum; i++)
		{
			DBDD.wNo = peqp->DDHourFrz.pwIndex[i];
			if((DBDD.wNo%32 >= 16)||(DBDD.wNo%32 < 8)) continue;
			DBDD.wNo -= 8;
			ReadRangeAllDD(pinfo->id, DBDD.wNo, 1, 100, (long *)&dwValue);
			
			f = *((float*)&dwValue);
			
			sprintf(buf, "    <DI val=\"%f\"/>\r\n", f);
			len1 = strlen(buf);
			fwrite(buf,1,len1,pinfo->fp);
			len += len1;
			memset(buf,0,len1+1);
		}
		
			/*for (i=0; i<peqp1->DDHourFrz.wNum; i++)
			{
				DBDD.wNo = peqp1->DDHourFrz.pwIndex[i];
				if((DBDD.wNo%32 >= 16)||(DBDD.wNo%32 < 8)) continue;
			 
				atomicReadTEDD(peqp1, 1, sizeof(struct VDBDD), &DBDD,FALSE);
				f = *((float*)&DBDD.lValue);
				sprintf(buf, "    <DI val=\"%.2f\"/>\r\n", f);
				 len1 = strlen(buf);
				 fwrite(buf, 1, len1, pinfo->fp);
				 len += len1;
				 memset(buf,0,len1+1);
			}*/

	 strcpy(buf, RecTail);

	  len1 = strlen(buf);
	 fwrite(buf, 1, len1, pinfo->fp);
	 len += len1;
	 memset(buf,0,len1+1);
	 
	 return len;
}

//日冻结类型标示，第一次日冻结值;暂无虚装置，暂时用实装置 读取，实装置 需设置电度曲线
static int hisDll_FrzDayGet(void *pdata, char *pbuf)
{
		int i, len,len1;
		float f;
	  DWORD dwValue = 0;
		char *buf=pbuf;
		struct VVirtualEqp *peqp;
	  //struct VTrueEqp *peqp1;
		struct VDBDD DBDD;
		DWORD minute;
		struct VHisInfo *pinfo = (struct VHisInfo*)pdata;

		peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
		if (peqp == NULL) return 0;
		
		/*for(i=0;i<*g_Sys.Eqp.pwNum;i++)//删
		{
			peqp1 = g_Sys.Eqp.pInfo[i].pTEqpInfo;
			if(peqp1 == NULL) continue;
			if(peqp1->Cfg.wDDNum)
				break;
		}

		if(peqp1 == NULL) return 0;*/
		len = len1 = 0;
		if (peqp == NULL) return 0;

		minute = CalendarClock(&pinfo->clock);
		minute += 24*60-1;	/*前面去掉24小时所以加上*/
		SystemClock(minute,&pinfo->clock);
	
		sprintf(buf, "  <DataRec type=\"DayFrz\" tm=\"%02d%02d%02d_%02d%02d%02d\">\r\n", 
			 pinfo->clock.wYear-2000, pinfo->clock.byMonth, pinfo->clock.byDay, pinfo->clock.byHour, pinfo->clock.byMinute, pinfo->clock.bySecond);

		 len1 = strlen(buf);
		 fwrite(buf, 1, len1, pinfo->fp);
		 len += len1;
		 memset(buf,0,len1+1);
		
		/*for(i=0;i<peqp1->DDHourFrz.wNum;i++)
		{
			DBDD.wNo = peqp1->DDHourFrz.pwIndex[i];
			if((DBDD.wNo%32 >= 24) || (DBDD.wNo%32 <16)) continue;
			
			atomicReadTEDD(peqp1,1,sizeof(struct VDBDD),&DBDD,FALSE);
			f = *((float*)&DBDD.lValue);
			sprintf(buf, "    <DI val=\"%.2f\"/>\r\n", f);
			 len1 = strlen(buf);
			 fwrite(buf, 1, len1, pinfo->fp);
			 len += len1;
			 memset(buf,0,len1+1);
		}*/

		for (i=0; i<peqp->DDHourFrz.wNum; i++)
		{
			DBDD.wNo = peqp->DDHourFrz.pwIndex[i];
			if((DBDD.wNo%32 >= 24) || (DBDD.wNo%32 < 16)) continue;
			DBDD.wNo -= 16;
			ReadRangeAllDD(pinfo->id, DBDD.wNo, 1, 100, (long *)&dwValue);
			
			f = *((float*)&dwValue);
			sprintf(buf, "    <DI val=\"%f\"/>\r\n", f);
			len1 = strlen(buf);
			fwrite(buf,1,len1,pinfo->fp);
			len += len1;
			memset(buf,0,len1+1);
		}

	 strcpy(buf, RecTail);
	 len1 = strlen(buf);
	 fwrite(buf, 1, len1, pinfo->fp);
	 len += len1;
	 memset(buf,0,len1+1);
	 
	 return len;
}

static void hisDll_ResetExv(void)
{
	int i,j;
	struct VTrueEqp *pTEqp;
	
	for (j=0; j<*g_Sys.Eqp.pwNum; j++)
	{
		if ((pTEqp = g_Sys.Eqp.pInfo[j].pTEqpInfo) != NULL)
		{
			for (i=0; i<pTEqp->Cfg.wYCMaxMinNum; i++)
			{
				pTEqp->pMaxMinYC[i].lMax = 0;
				GetSysClock(&pTEqp->pMaxMinYC[i].max_tm, CALCLOCK);
				pTEqp->pMaxMinYC[i].lMin = 0x7FFFFFFF;
				GetSysClock(&pTEqp->pMaxMinYC[i].min_tm, CALCLOCK);
			}
		}
	} 
}

static int his_File_ReWrite(FILE* fp, struct VHisRcd *pindex ,int num)
{
		int i,rd_offset,wr_offset, len, filelen,delnum;
		
		filelen = pindex->file_head;
		rd_offset = pindex->file_head;
	  delnum = 0;
	  
	  do
	  {
	  	fseek(fp, rd_offset, SEEK_SET);
		 	len = fread(his_buf, 1, DBHIS_FILE_SEG, fp);
	  	if(len > DBHIS_FILE_SEG)
	  		len = DBHIS_FILE_SEG;
	  		
	  	rd_offset +=len;
	  	
	  	for (i=0; i<len; i++)
	    {
	         if ((his_buf[i] == '\r')&&(his_buf[i+1] == '\n'))
	         	delnum++;
	         	if(delnum == num)
	            break;
	    }
	  }while((delnum < num)&&(len > 0));
		
	 if(rd_offset < (strlen(RecTail)+strlen(DataTail)))
		 return pindex->file_head;
	 if((rd_offset - len + i +2) >= pindex->file_wp)
	 	return pindex->file_head;
	 if(rd_offset >= pindex->file_wp)
		 return pindex->file_head;
	 
	 fseek(fp, pindex->file_head, SEEK_SET);
	 fwrite(his_buf+i+2, 1, len-(i+2), fp);
	 wr_offset = lfseek(fp, 0, SEEK_CUR);
	 filelen = wr_offset;
	 
	   while(rd_offset < pindex->file_size)
     {
         fseek(fp, rd_offset, SEEK_SET);
				len = fread(his_buf, 1, DBHIS_FILE_SEG, fp);
				 if(len <= 0)
					 break;
		 rd_offset += len;
		 fseek(fp, wr_offset, SEEK_SET);
		 fwrite(his_buf, 1, len, fp);
		 wr_offset += len;
			 filelen += len;
     }
		filelen -= (strlen(RecTail)+strlen(DataTail));
	 return filelen;     
}

static int his_Ram_ReWrite(char *pbuf, struct VHisRcd *pindex, int num)
{
     int i,rd_offset,wr_offset, len, filelen, delnum;
	 char *buf1, *buf2;

	 buf1 = pbuf + pindex->file_head;
	 filelen = pindex->file_head;
	 
	 rd_offset = pindex->file_head;

	 delnum = 0;

	 do
	 {
	     len = pindex->file_wp - rd_offset;
		 if (len > DBHIS_FILE_SEG)
		 	len = DBHIS_FILE_SEG;
		
		 rd_offset += len;
		 
	     for (i=0; i<len; i++)
	     {
	         if ((buf1[i] == '\r')&&(buf1[i+1] == '\n'))
	            delnum++;
			 if (delnum == num)
			 	break;
	     }
		 buf1 += len;
		
	 }while ((delnum < num)&&(len > 0));

	 if (rd_offset >= pindex->file_wp)
	     return pindex->file_head;
	 
	 buf2 = buf1-len+i+2;
	 buf1 = pbuf + pindex->file_head;
	 memcpy(buf1, buf2, len-(i+2));
	
	 wr_offset = pindex->file_head + len - (i+2);
	 filelen = wr_offset;
	 
     while(rd_offset< pindex->file_size)
     {
         buf1 = pbuf + rd_offset;
		 buf2 = pbuf + wr_offset; 

		 if ((pindex->file_size - rd_offset) > DBHIS_FILE_SEG)
		 	len = DBHIS_FILE_SEG;
		 else
		  {
			len = pindex->file_size - rd_offset;
		 }
		
		 rd_offset += len;
		 memcpy(buf2, buf1, len);
		 wr_offset += len;
		 filelen += len;
     }	
	 filelen -= (strlen(RecTail)+strlen(DataTail));

	 if((0x0A != pbuf[filelen -1])||(0x0D != pbuf[filelen -2]))
	 {
		shellprintf("index Error");
	 }
	 return filelen;
}

static void his_Event_RamSort(struct VHisDllMan *pdllman, int num)
{
    FILE *fp;
	int len, total;
	char fname[128];
    char *pbuf = eventbuf;


	total = pdllman->index->num+num;
	if (total > pdllman->dll.max)
	{
	    hisDll_NameGet(&pdllman->dll, NULL, fname);
	    fp = fopen(fname, "r+");
	    if (fp == NULL)
		  return;

		if(pdllman->dll.type == DBhis_type_ulog)
		{
			len = his_File_ReWrite(fp, pdllman->index, (total-pdllman->dll.max));
			fclose(fp);
			pdllman->index->file_wp = len;
		}
		else
		{
			len = pdllman->index->file_size;
			fread(pbuf, 1, len, fp); 
			fclose(fp);
		    len = his_Ram_ReWrite(pbuf, pdllman->index, (total-pdllman->dll.max));
			pdllman->index->file_wp = len;
			pdllman->index->first = 1;
		}

	}
	
}

static void his_Event_RamWrite(struct VHisDllMan *pdllman, int num)
{
	int  len,index,i;
	char ftemp[128];
	char *pbuf;
	struct VHisSocket *psocket;
	
	pbuf = eventbuf;
	
	memset(his_buf,0,his_buf_len);

   	index = pdllman->index->file_wp;

	for (i=0; i<num; i++)
	{
	     psocket = (struct VHisSocket *)his_event_ary[i];
	     len = pdllman->dll.get_hisdll_data(psocket->buf, his_buf);
	     memcpy(pbuf+index, his_buf, len);

	     if (pdllman->index->num < pdllman->dll.max)
	     {
	        pdllman->index->num++;
	     }
		 psocket->read_write = 0;
    
	     index += len;
	}
	if(num>0)
		his_cfg_change = 1; 

	pdllman->index->file_wp = index;
    memcpy(pbuf + index, RecTail, strlen(RecTail));
	index += strlen(RecTail);
	memcpy(pbuf + index, DataTail,strlen(DataTail));
	index += strlen(DataTail);

	pdllman->index->file_size = index;

    if((pdllman->index->num < 10) && (pdllman->index->num >= 0))
	     sprintf(ftemp, "%d\">   \r\n", pdllman->index->num);//num 为 datarec  num
	else if((pdllman->index->num < 100) && (pdllman->index->num >= 0))
		sprintf(ftemp, "%d\">  \r\n", pdllman->index->num);//num 为 datarec  num
	else if((pdllman->index->num < 1000) && (pdllman->index->num >= 0))
		sprintf(ftemp, "%d\"> \r\n", pdllman->index->num);//num 为 datarec  num
	else
		sprintf(ftemp, "%04d\">\r\n", pdllman->index->num);//num 为 datarec  num
		
	index = pdllman->index->num_wp;
	memcpy(pbuf+index, ftemp, 8);
	
}

static void his_Event_RamFile(struct VHisDllMan *pdllman)
{
    FILE *fp;
	char fname[128];
	char *pbuf;

	if (pdllman->index->first == 0) return;

	hisDll_NameGet(&pdllman->dll, NULL, fname);

	pbuf = eventbuf;

	fp = fopen(fname, "w+");
	if (fp == NULL)
		return;
	fwrite(pbuf, pdllman->index->file_size, 1, fp);
	fclose(fp);
	pdllman->index->first = 0;
}

/*co, flow, ulog, soe*/
static void his_Event_FileWrite(struct VHisDllMan *pdllman, int num)
{
     FILE *fp,*fp1;
	 int  len,len1,len2, i;
	 char fname[128];
	 char ftemp[128];
	 struct VHisSocket *psocket;
	BOOL bRewrite = 0;
	
	 if (pdllman->index->first)
	 {
	    his_Event_RamWrite(pdllman, num);
		return;
	 }
	 
	 memset(his_buf,0,his_buf_len);
	
	 hisDll_NameGet(&pdllman->dll, NULL, fname);

	 fp = fopen(fname, "r+");
	 if (fp == NULL)
	 {
	     fp = fopen(fname, "w+");
		 if(fp==NULL) return;
		 pdllman->dll.create_hisdll_file(fp, &pdllman->dll, pdllman->index);
		 
	 }
	 len = lfseek(fp, 0, SEEK_END);
	 fseek(fp,0, SEEK_SET);
	 //pbuf = eventbuf;

	 //fread(pbuf, 1, pdllman->index->file_size, fp);

	 
	if(pdllman->index->file_wp>len)
		shellprintf("Error1");

		
     fseek(fp, pdllman->index->file_wp, SEEK_SET);

	 for (i=0; i<num; i++)
	 {
	     psocket = (struct VHisSocket *)his_event_ary[i];
         len = pdllman->dll.get_hisdll_data(psocket->buf, his_buf);

	     fwrite(his_buf, 1, len, fp);

		 psocket->read_write = 0;
		 if (pdllman->index->num < pdllman->dll.max)
	     {
	        pdllman->index->num++;
	     }
		 pdllman->index->file_wp += len;
	 }
	if(num>0)
		his_cfg_change = 1; 
     fwrite(RecTail, 1, strlen(RecTail), fp);
	 fwrite(DataTail, 1, strlen(DataTail), fp);
	 len1 = lfseek(fp , 0, SEEK_CUR);
	 
	 if((pdllman->index->num < 10) && (pdllman->index->num >= 0))
	     sprintf(ftemp, "%d\">   \r\n", pdllman->index->num);//num 为 datarec  num
	else if((pdllman->index->num < 100) && (pdllman->index->num >= 0))
		sprintf(ftemp, "%d\">  \r\n", pdllman->index->num);//num 为 datarec  num
	else if((pdllman->index->num < 1000) && (pdllman->index->num >= 0))
		sprintf(ftemp, "%d\"> \r\n", pdllman->index->num);//num 为 datarec  num
	else
		sprintf(ftemp, "%04d\">\r\n", pdllman->index->num);//num 为 datarec  num
	 
	 fseek(fp, pdllman->index->num_wp, SEEK_SET);
	 fwrite(ftemp, 1, 8, fp);
     len = lfseek(fp, 0, SEEK_END);
	 pdllman->index->file_size = len1;
	 if(pdllman->index->file_wp>len)
	 	shellprintf("Error2");
	 if(len1 <  len)
	 	bRewrite = 1;
	 fclose(fp);

	 if(bRewrite == 1)
	 {
	 	 fp = fopen(fname, "r+");
		 if(fp == NULL) 
		 	return;
		 fp1 = fopen("/tffsb/bakup/temp.xml","w");
		if(fp1 == NULL)
			return;
		len1 = len2 = 0;
		while(len1 < pdllman->index->file_size)
		{
			fseek(fp, len1, SEEK_SET);
			memset(his_buf,0,DBHIS_FILE_SEG);
			len2 = fread(his_buf, 1, DBHIS_FILE_SEG, fp); 
			if(len2 <= 0)
				break;
			if((len1 + len2) <= pdllman->index->file_size)
			{
				fwrite(his_buf,1,len2,fp1);
				len1+= len2;
			}
			else
			{
				fwrite(his_buf,1,pdllman->index->file_size - len1,fp1);
				len1 = pdllman->index->file_size;
			}
		}
		fclose(fp);
		fclose(fp1);

		remove(fname);
		rename("/tffsb/bakup/temp.xml",fname);
	 }
}


/*exv, fixpt, frz*/
static void his_Record_Write(struct VHisDllMan *pdllman, struct VSysClock *pclk)
{
     FILE *fp;
	 int  len, len1, len2;
	 char *pbuf = his_buf;
	 char fname[128];
	 char ftemp[128];
	 struct VHisInfo info;
	 struct VVirtualEqp *peqp;
	 struct VSysClock tmp_clock;
	 DWORD minute;
	 BOOL fileerr = 0;
	
	 memset(his_buf,0,his_buf_len);
	 hisDll_NameGet(&pdllman->dll, pclk, fname);
	
	 peqp = g_Sys.Eqp.pInfo[pdllman->id].pVEqpInfo;
	
	 if(peqp == NULL) return;
	
	 if((pdllman->dll.type == DBhis_type_exv) || (pdllman->dll.type == DBhis_type_fixpt))
	 {
		 if(peqp->Cfg.wYCNum == 0)
			 return;
		 if(peqp->YCHourFrz.wNum == 0)
		 	return;
	 }
	 else if(pdllman->dll.type == DBhis_type_frz)
	 {
		 if(peqp->Cfg.wDDNum == 0)
			 return;
	 }

	 fp = fopen(fname, "r+");
	 if (fp == NULL)
	 {
	     fp = fopen(fname, "w");
		 if(fp==NULL) return;
		 memcpy(&tmp_clock, pclk,sizeof(tmp_clock));
		 minute = CalendarClock(&tmp_clock);
		 minute = minute - 24*60*DBHIS_DAY_MAX;
		 SystemClock(minute,&tmp_clock);
		 memset(ftemp, 0 ,sizeof(ftemp));
		 hisDll_NameGet(&pdllman->dll,& tmp_clock, ftemp);
		remove(ftemp);
		shellprintf("remove %s \n",ftemp);
		memset(ftemp, 0 ,sizeof(ftemp));
		 
		 pdllman->dll.create_hisdll_file(fp, &pdllman->dll, pdllman->index);
		 pdllman->index->num = 0;
	 }
	 else // 比较一下指针是否与文件一致
	 {
		 if((pdllman->index->file_size <= pdllman->index->file_wp)//检查指针是否ok
			||(pdllman->index->file_head <= pdllman->index->num_wp)
		  ||(pdllman->index->file_wp < pdllman->index->file_head))
		 {
				fileerr = 1;
		 }
		 else
		 {
			 len = lfseek(fp, 0, SEEK_END);
			 len1 = strlen(RecTail);
			 if((len != pdllman->index->file_size) || (pdllman->index->file_wp < len1))
			 {
					fileerr = 1;
			 }
			 else
			 {
				 fseek(fp, pdllman->index->file_wp - len1, SEEK_SET);
				 len = fread(his_buf, 1, len1, fp);
				 if((len != len1) || (strstr(his_buf,RecTail) == 0))
				 {
					fileerr = 1;
				 }
			 }
		 }	
		 if(fileerr == 1)
		 {
			 fclose(fp);
			 remove(fname);
			 shellprintf(" cuowu 删文件 %s \n",fname);
			 fp = fopen(fname, "w");
			 if(fp==NULL) return;						 
			 pdllman->dll.create_hisdll_file(fp, &pdllman->dll, pdllman->index);
			 pdllman->index->num = 0; 
		 }
	 }

	 len = len1 = len2 = 0;
	 fseek(fp, pdllman->index->file_wp, SEEK_SET);

	 info.id = pdllman->id;
	 info.sec = pdllman->index->num+1; //从0开始，sec 从1开始
	 info.fp = fp;
	 memcpy(&info.clock, pclk, sizeof(struct VSysClock));
		 
	 len = pdllman->dll.get_hisdll_data(&info, pbuf);
	 pbuf += len;

	 len += (len1 + len2);
	 //fwrite(his_buf, 1, len, fp); 读的时候写
	 pdllman->index->num++;
	 pdllman->index->file_wp += len;
	 his_cfg_change = 1;
	 
	 fwrite(DataTail, 1, strlen(DataTail), fp);
	 len = lfseek(fp, 0, SEEK_CUR);
	 pdllman->index->file_size = len;


	 if(pdllman->dll.type != DBhis_type_exv)
	  {
			if((pdllman->index->num <10) && ( pdllman->index->num >=0))
				sprintf(ftemp, "%d\" ", pdllman->index->num);//num 为 datarec  num
			else if((pdllman->index->num <100) && ( pdllman->index->num >=0))
				sprintf(ftemp, "%02d\"", pdllman->index->num);//num 为 datarec  num
              else if((pdllman->index->num < 1000) && (pdllman->index->num >= 0))
                   sprintf(ftemp, "%d\"> \r\n", pdllman->index->num);//num 为 datarec  num
               else
                    sprintf(ftemp, "%04d\">\r\n", pdllman->index->num);//num 为 datarec  num
		
				
			fseek(fp, pdllman->index->num_wp, SEEK_SET);
			fwrite(ftemp, 1, 3, fp);
	  }
	 
	 fclose(fp);
}


void his_Event_Write(void)
{
    int i,j,write_num,same;
	int ptr1,ptr2;
    int wptr, num,num1;

    for (;;)
    {
        wptr = his_wptr;
		num = (wptr >= his_rptr) ? (wptr-his_rptr):(DBHIS_SOCKET_MAX-his_rptr+wptr);

		if (num == 0) break;
		
		ptr1 = (wptr-1)&(MAX_B2FSOCKET_NUM-1);
		num1 = num-1;
        for (i=0; i<num; i++, num1--, ptr1 = (ptr1-1)&(MAX_B2FSOCKET_NUM-1))
        {
			if (his_event_socket[ptr1].read_write != 2) continue;

            ptr2 = his_rptr;
			for(j=0; j<num1; j++, ptr2 = (ptr2+1)&(MAX_B2FSOCKET_NUM-1))
			{
				if (his_event_socket[ptr2].read_write != 2) continue;

				same = 1;
			    if (his_event_socket[ptr2].buf !=  his_event_socket[ptr1].buf)
					same = 0;
				else if (his_event_socket[ptr2].dllman!=  his_event_socket[ptr1].dllman)
					same = 0;

				if (same) 
				{
					his_event_socket[ptr2].buf = his_event_socket[ptr1].buf;
					his_event_socket[ptr1].read_write = 4;
				}								
			}
		}

		ptr1 = his_rptr;

		for (i=0; i<num; i++, ptr1 = (ptr1+1)&(DBHIS_SOCKET_MAX-1))
		{
			if (his_event_socket[ptr1].read_write != 2) continue;

            write_num = 0;
			his_event_ary[write_num] = (DWORD)(his_event_socket+ptr1);
			write_num++;
			
			ptr2 = (ptr1+1)&(DBHIS_SOCKET_MAX-1);
			for (j = (i+1); j<num; j++,ptr2 = (ptr2+1)&(DBHIS_SOCKET_MAX-1))
			{
			    if (his_event_socket[ptr2].read_write != 2) continue;
				if (his_event_socket[ptr2].dllman != his_event_socket[ptr1].dllman) continue;
				his_event_ary[write_num] = (DWORD)(his_event_socket+ptr2);
				write_num++;
			}
			
            his_Event_RamSort(his_event_socket[ptr1].dllman, write_num);
			his_Event_FileWrite(his_event_socket[ptr1].dllman, write_num);
			
			his_Event_RamFile(his_event_socket[ptr1].dllman);

		}	

		ptr1 = his_rptr;		
		for (i=0; i<num; i++, ptr1 = (ptr1+1)&(MAX_B2FSOCKET_NUM-1))
		{
			if (his_event_socket[ptr1].read_write == 4) 
				his_event_socket[ptr1].read_write = 0;
		}		

		his_rptr = (his_rptr+num)&(DBHIS_SOCKET_MAX-1);

    }
}
void his2fileDirCheck(void)
{
	struct stat fstat;

	if ((stat("/tffsb/cfg", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/cfg")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/cfg' failed!");

		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/cfg' success!");
	}
	if ((stat("/tffsb/HISTORY", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/HISTORY")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY' success!");
	}	
	if ((stat("/tffsb/HISTORY/SOE", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/HISTORY/SOE")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/SOE' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/SOE' success!");
	}	
	if ((stat("/tffsb/HISTORY/CO", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/HISTORY/CO")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/CO' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/CO' success!");
	}	
	if ((stat("/tffsb/HISTORY/EXV", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/HISTORY/EXV")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/EXV' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/EXV' success!");
	}
	if ((stat("/tffsb/HISTORY/FIXPT", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/HISTORY/FIXPT")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/FIXPT' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/FIXPT' success!");
	}
	if ((stat("/tffsb/HISTORY/FRZ", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/HISTORY/FRZ")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/FRZ' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/FRZ' success!");
	}
	if ((stat("/tffsb/HISTORY/FLOWREV", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/HISTORY/FLOWREV")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/FLOWREV' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/FLOWREV' success!");
	}
	if ((stat("/tffsb/HISTORY/ULOG", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/HISTORY/ULOG")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/ULOG' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/HISTORY/ULOG' success!");
	}
	
	/*线损*/
	if ((stat("/tffsb/LINELOSS", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/LINELOSS")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS' success!");
	}
	if ((stat("/tffsb/LINELOSS/FIXD", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/LINELOSS/FIXD")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/FIXD' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/FIXD' success!");
	}
	if ((stat("/tffsb/LINELOSS/RAND", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/LINELOSS/RAND")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/RAND' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/RAND' success!");
	}
	if ((stat("/tffsb/LINELOSS/FRZD", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/LINELOSS/FRZD")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/FRZD' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/FRZD' success!");
	}
	if ((stat("/tffsb/LINELOSS/SHARPD", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/LINELOSS/SHARPD")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/SHARPD' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/SHARPD' success!");
	}
	if ((stat("/tffsb/LINELOSS/MONTHD", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/LINELOSS/MONTHD")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/MONTHD' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/MONTHD' success!");
	}
	if ((stat("/tffsb/LINELOSS/EVENTD", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsb/LINELOSS/EVENTD")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/EVENTD' failed!");
		else
			myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/LINELOSS/EVENTD' success!");
	}
}

static int his_cfg_init(void)
{
	FILE *fp;
	WORD crc,crc_f;	
	struct VFileHead *filehead;
	struct VHisRcd *hisrcd;
	
	his_buf = (char*) malloc(his_buf_len);
	if(his_buf == NULL) return -1;
	memset(his_buf,0,his_buf_len);

	eventbuf = (char*) malloc(64*1024);
	if (eventbuf == NULL) return-1;
	memset(eventbuf, 0, 64*1024);
	
	filehead = (struct VFileHead*)his_buf;
	hisrcd = (struct VHisRcd*)(filehead+1);

	his_rcd_cfg =  (struct VHisRcd*)malloc(sizeof(struct VHisRcd)*DBhis_type_num);
	if (his_rcd_cfg == NULL) return -1;
	
	memset(his_rcd_cfg, 0, sizeof(struct VHisRcd)*DBhis_type_num);

	his2fileDirCheck();
		
	fp = fopen(his_rcdfile, "r+b");
	if (fp == NULL) 
	{
	  fp = fopen(his_rcdfile, "w+");
		if(fp == NULL) return -1;
		
		filehead->wVersion = 101;
		filehead->wAttr = 1;
		filehead->nLength = sizeof(struct VFileHead)+sizeof(struct VHisRcd)*DBhis_type_num+2;
		crc = GetParaCrc16((BYTE *)his_rcd_cfg, sizeof(struct VHisRcd)*DBhis_type_num);

		fwrite((BYTE*)filehead, 1, sizeof(struct VFileHead), fp);
		fwrite(his_rcd_cfg, 1, sizeof(struct VHisRcd)*DBhis_type_num, fp);

		fwrite(&crc, 1, 2, fp);
		fclose(fp);
		return 0;
	}
	
	fread((BYTE*)filehead, 1, sizeof(struct VFileHead), fp);
	fread((BYTE*)hisrcd, 1, sizeof(struct VHisRcd)*DBhis_type_num, fp);
	fread((BYTE*)&crc_f, 1, 2, fp);
	
	fclose(fp);
	
	crc = GetParaCrc16((BYTE*)hisrcd, sizeof(struct VHisRcd)*DBhis_type_num);

	if (crc != crc_f)
		return 0;

	memcpy(his_rcd_cfg, hisrcd, sizeof(struct VHisRcd)*DBhis_type_num);
	return 0;
	
}

static struct VHisDllMan *his_regist(WORD id, struct VHisDll *pdll, struct VHisDllMan **pdllman, struct VHisRcd *prcdcfg)
{
	char fname[4*MAXFILENAME];
	struct VHisDllMan *p, *q, **s;
	struct VSysClock clk;

    q = (struct VHisDllMan *)malloc(sizeof(struct VHisDllMan));
	if (q == NULL) return NULL;	

	q->index = prcdcfg;
	s = pdllman;	
	q->id = id;          
	memcpy(&q->dll, pdll, sizeof(struct VHisDll));
	q->next = NULL; 
    
	if (*s == NULL) 
	{
		*s = q;
	}
	else
	{
		p = *s;
		while (p->next != NULL) p = p->next;			

		p->next = q;
	}	

	GetSysClock(&clk, SYSCLOCK);
	hisDll_NameGet(&q->dll, &clk, fname);
	hisDll_Scan(fname, q->dll.type, prcdcfg);

    return(q);
}

//负责传递 写任务
void his2file_write(BYTE *pBuf, WORD wID, int flag)
{
     int k;
     struct VHisSocket socket;

     socket.read_write = 2;
	
	 if((dbhis_init == 0) &&((flag == SYSEV_FLAG_ULOG)||(flag == SYSEV_FLAG_SSOE)||(flag == SYSEV_FLAG_DSOE)))
	 	{
			his_event_sockets_temp[his_wptr_temp].buf = pBuf;
			his_event_sockets_temp[his_wptr_temp].read_write = flag; 
			his_wptr_temp++;
			return;
	 	}
		
	 socket.dllman = hisDll_Get(wID, flag, his_event);
	 if (socket.dllman == NULL)
	 {
		return;
	 }
	 socket.buf = pBuf;
 
	 k = 0;
	 while ((his_event_socket[his_wptr].read_write!=0) && (k<DBHIS_SOCKET_MAX))
     {
		his_wptr++;
		his_wptr &= (DBHIS_SOCKET_MAX-1);
		k++;
	 }

	if (k == DBHIS_SOCKET_MAX) 
		return;

	memcpy(&his_event_socket[his_wptr], &socket, sizeof(struct VHisSocket));
	his_event_socket[his_wptr].read_write = 2;
	his_wptr++;
	his_wptr &= (MAX_B2FSOCKET_NUM-1);
	
	evSend(B2F_ID, EV_UFLAG);
	return;
}

void his_cfg_write(void)
{
    FILE *fp;
	int len;
	WORD crc;
    crc = GetParaCrc16((BYTE *)his_rcd_cfg, sizeof(struct VHisRcd)*DBhis_type_num);

	fp = fopen(his_rcdfile, "r+");
	if (fp == NULL)
		return;
    
	len = sizeof(struct VFileHead);
	fseek(fp, len, SEEK_SET);
	fwrite((BYTE*)his_rcd_cfg,  1, sizeof(struct VHisRcd)*DBhis_type_num, fp);
	fwrite((BYTE*)&crc, 1, 2, fp);
	fclose(fp);
}


void his_Data_Write(void)
{
    struct VCalClock calclock;
	struct VSysClock sysclock;
	static struct VSysClock oldclock;
	static BOOL first = TRUE;

	GetSysClock(&calclock, CALCLOCK);
    CalClockTo(&calclock, &sysclock);
	if(first)
	{
		memcpy(&oldclock, &sysclock, sizeof(struct VSysClock));
		first = FALSE;
	}
	if (((sysclock.wYear != oldclock.wYear) || (sysclock.byMonth != oldclock.byMonth) || (sysclock.byDay != oldclock.byDay)) && (sysclock.byHour == 0))
	{
	   hisDll_day(&sysclock);
	   hisDll_ResetExv();
	}
	if (sysclock.byMinute != oldclock.byMinute)
	{
		hisDll_hour(&sysclock);
	}
	memcpy(&oldclock, &sysclock, sizeof(struct VSysClock));

}

void his2file(void)
{
    struct VCalClock calclock;
	struct VSysClock oldclock;
	struct VSysClock sysclock;
    DWORD events;

	memset(&oldclock, 0, sizeof(struct VSysClock));	

  tmEvEvery(HIS_ID, SECTOTICK(30), EV_TM1);   /*30s*/

    for (;;)
    {
        evReceive(HIS_ID, EV_TM1|EV_UFLAG, &events);

			if (events & EV_UFLAG)
			{
					his_Event_Write();//按类b2f 处理，写xml文件
			}
		if (events & EV_TM1)//30s 处理冻结
		{
		    GetSysClock(&calclock, CALCLOCK);
		    CalClockTo(&calclock, &sysclock);

			if (((sysclock.wYear != oldclock.wYear) || (sysclock.byMonth != oldclock.byMonth) || (sysclock.byDay != oldclock.byDay)) && (sysclock.byHour == 0))
			{
			   hisDll_day(&sysclock);
			   hisDll_ResetExv();
			}
			if (sysclock.byMinute != oldclock.byMinute)
			{
					hisDll_hour(&sysclock);
			}

		}

		if (his_cfg_change)
		{
		    his_cfg_write();
			his_cfg_change = 0;
		}
		memcpy(&oldclock, &sysclock, sizeof(struct VSysClock));
    }
}

void his_init(void)
{
  int i;
	struct VVirtualEqp *pVEqp;
	struct VHisDll dll;
	
  his_event = NULL;
	his_hour = NULL;
	his_day = NULL;

	if (his_cfg_init() == -1) return;
	
	for (i=0; i<*g_Sys.Eqp.pwNum; i++)
	{
		if ((pVEqp = g_Sys.Eqp.pInfo[i].pVEqpInfo) != NULL)
		{
		    if (pVEqp->Cfg.dwFlag & 0x20) // D5
		    {
					dll.type = DBhis_type_soe;
					dll.max = DBHIS_SOE_MAX;
					dll.date = 0;
					dll.md = 0;
					dll.create_hisdll_file = hisDll_RecCreate;
					dll.get_hisdll_data = hisDll_SoeGet;
					his_regist(i, &dll, &his_event, &his_rcd_cfg[DBhis_type_soe]);// his_event his_day 是数组，仅仅只有next   (注册一个后带一个)

					dll.type = DBhis_type_co;
					dll.max = DBHIS_CO_MAX;
					dll.date = 0;
					dll.md = 0;
					dll.create_hisdll_file = hisDll_RecCreate;
					dll.get_hisdll_data = hisdll_CoGet;
					his_regist(i, &dll, &his_event, &his_rcd_cfg[DBhis_type_co]);

					dll.type = DBhis_type_exv;
					dll.date = DBHIS_DAY_MAX;
					dll.max = 0;
					dll.md = 1;
					dll.create_hisdll_file = hisDll_AttrCreate;
					dll.get_hisdll_data = hisDll_ExvGet;
					his_regist(i, &dll, &his_day, &his_rcd_cfg[DBhis_type_exv]);

					dll.type = DBhis_type_fixpt;
					dll.date = DBHIS_DAY_MAX;
					dll.max = 0;
					dll.md = DBHIS_MIN_INTERVAL;
					dll.create_hisdll_file = hisDll_AttrCreate;
					dll.get_hisdll_data = hisDll_FixptGet;
					his_regist(i, &dll, &his_hour, &his_rcd_cfg[DBhis_type_fixpt]);

					dll.type = DBhis_type_frz;
					dll.date = DBHIS_MIN_MAX;
					dll.max = 0;
					dll.md = DBHIS_MIN_INTERVAL;
					dll.create_hisdll_file = hisDll_FrzCreate;
					dll.get_hisdll_data = hisDll_FrzGet;
					his_regist(i, &dll, &his_hour, &his_rcd_cfg[DBhis_type_frz]);

					dll.type = DBhis_type_frz;
					dll.date = DBHIS_DAY_MAX;
					dll.max = 0;
					dll.md = 1;
					dll.create_hisdll_file = hisDll_FrzCreate;
					dll.get_hisdll_data = hisDll_FrzDayGet;
					his_regist(i, &dll, &his_day, &his_rcd_cfg[DBhis_type_frz]);

					dll.type = DBhis_type_flowrev;
					dll.max = DBHIS_FLOW_MAX;
					dll.date = 0;
					dll.md = 0;
					dll.create_hisdll_file = hisDll_RecCreate;
					dll.get_hisdll_data = hisdll_FlowGet;
					his_regist(i, &dll, &his_event, &his_rcd_cfg[DBhis_type_flowrev]);

					dll.type = DBhis_type_ulog;
					dll.max = DBHIS_LOG_MAX;
					dll.date = 0;
					dll.md = 0;
					dll.create_hisdll_file = hisDll_RecCreate;
					dll.get_hisdll_data = hisdll_UlogGet;
					his_regist(i, &dll, &his_event, &his_rcd_cfg[DBhis_type_ulog]);
					
		    }
		}
	}	
	
	dbhis_init = 1;

	while(his_wptr_temp > 0)
	{
		his2file_write(his_event_sockets_temp[his_wptr_temp-1].buf,0,his_event_sockets_temp[his_wptr_temp-1].read_write);
		his_wptr_temp--;
	}

}
#endif
