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
#include <unistd.h>
#include "sys.h"
#include "FileExt.h"
#include "Dbhis_ttu.h"
#ifdef INCLUDE_HIS_TTU

#if (TYPE_USER == USER_JBDKY)
static char *msgFileHead = "%s,v1.0\r\n";
#else
static char *msgFileHead = "文件头：%s,v1.0\r\n";
#endif
static char *msgDataHead = "%s,%02d%02d%02d\r\n";
static char *msgeDataHead = "%s,%04d,%02d\r\n";
static char *msgolDataHead = "%s,%02d%02d%02d,%04d\r\n";
static char *msgFileTail = "";

static char histtu_rcdfile[4*MAXFILENAME];

static char *histtu_buf;
#define histtu_buf_len 2048
static struct VHisTtuDllMan *histtu_event;
static struct VHisTtuDllMan *histtu_hour, *histtu_day,*histtu_month;
static int histtu_rptr, histtu_wptr = 0;

static struct VHisTtuSocket histtu_event_socket[DBHISTTU_SOCKET_MAX];
#define histtu_sockets_temp 20
static int histtu_wptr_temp = 0;
static struct VHisTtuSocket histtu_event_sockets_temp[histtu_sockets_temp];
static BOOL dbhisttu_init = 0;

#define eventttubuflen (256*1024)


static char *eventttubuf;
static DWORD histtu_event_ary[DBHISTTU_SOCKET_MAX>>2];
static struct VHisTtuRcd *histtu_rcd_cfg=NULL;
int histtu_cfg_change = 0;

extern WORD atomicReadVEYCF_L(struct VVirtualEqp *pVEqp,WORD wNum,WORD wBufLen, struct VDBYCF_L *buf);
static void histtu_Record_Write(const struct VHisTtuDllMan *pdllman, const struct VSysClock *pclk);


int lfseek(FILE *stream, long offset, int fromwhere) //偏移之后获取文件位置
{
	int size,ret;
	ret = fseek (stream, offset, fromwhere);
	if(ret < 0)
		return -1;
	size=ftell (stream); 
	return size;
}



//事件调用可以是NULL，其他 日期可能对于不上
static void histtuDll_NameGet(const struct VHisTtuDll *pdll, const struct VSysClock *pclk, char *fname)
{
	struct VSysClock calclock;
	
    if ((pdll->type < 0) ||(pdll->type >= DBhisttu_type_num)) 
    {
	   sprintf(fname,"%s/error",SYS_PATH_ROOT2);
	   return;
    }
	if (pclk == NULL)
	{
		GetSysClock(&calclock, SYSCLOCK);
		pclk = &calclock;
	}
	switch(pdll->type)
	{
		case DBhisttu_type_soe:
			sprintf(fname,"soe.msg");
			break;
		case DBhisttu_type_co:
			sprintf(fname,"co.msg");
			break;
		case DBhisttu_type_hl:
			sprintf(fname, "hl%4d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay);
			break;
		case DBhisttu_type_ol:
			sprintf(fname, "ol%4d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay);
			break;
		case DBhisttu_type_exv:
			sprintf(fname,"exv%04d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay);
			break;
		case DBhisttu_type_fixpt:
			sprintf(fname,"fixpt%04d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay);
			break;
		case DBhisttu_type_frz:
			sprintf(fname,"frz%04d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay);
			break;
		case DBhisttu_type_voltd: //电压日合格率
			sprintf(fname,"volt%04d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay);
			break;
		case DBhisttu_type_volttd:
			sprintf(fname,"voltt%04d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay);
			break;
		case DBhisttu_type_voltm: // 月
			sprintf(fname,"volt%04d%02d.msg", pclk->wYear, pclk->byMonth);
			break;
		case DBhisttu_type_volttm:
			sprintf(fname,"voltt%04d%02d.msg", pclk->wYear, pclk->byMonth);
			break;
		case DBhisttu_type_flowrev:
			sprintf(fname,"flowrev.msg");
			break;
		case DBhisttu_type_ulog:
			sprintf(fname,"ulog.msg");
			break;
		default:
			break;
	}
	return;
}

static void histtuDll_FileNameGet(const struct VHisTtuDll *pdll, const struct VSysClock *pclk, char *fname)
{
	struct VSysClock calclock;

	if ((pdll->type < 0) ||(pdll->type >= DBhisttu_type_num)) 
    {
	   sprintf(fname,"%s/error",SYS_PATH_ROOT2);
	   return;
    }
	if (pclk == NULL)
	{
		GetSysClock(&calclock, SYSCLOCK);
		pclk = &calclock;
	}
	switch(pdll->type)
	{
		case DBhisttu_type_soe:
			sprintf(fname,"%s/HISTORY/SOE/soe.msg",SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_co:
			sprintf(fname,"%s/HISTORY/CO/co.msg",SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_hl:
			sprintf(fname, "%s/HISTORY/OL/hl%4d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay,SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_ol:
			sprintf(fname, "%s/HISTORY/OL/ol%4d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay,SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_exv:
			sprintf(fname,"%s/HISTORY/EXV/exv%04d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay,SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_fixpt:
			sprintf(fname,"%s/HISTORY/FIXPT/fixpt%04d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay,SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_frz:
			sprintf(fname,"%s/HISTORY/FRZ/frz%04d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay,SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_voltd: //电压日合格率
			sprintf(fname,"%s/HISTORY/VOLT_DAY/volt%04d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay,SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_volttd:
			sprintf(fname,"%s/HISTORY/VOLT_DAY/voltt%04d%02d%02d.msg", pclk->wYear, pclk->byMonth, pclk->byDay,SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_voltm: // 月
			sprintf(fname,"%s/HISTORY/VOLT_MON/volt%04d%02d.msg", pclk->wYear, pclk->byMonth,SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_volttm:
			sprintf(fname,"%s/HISTORY/VOLT_MON/voltt%04d%02d.msg", pclk->wYear, pclk->byMonth,SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_flowrev:
			sprintf(fname,"%s/HISTORY/FLOWREV/flowrev.msg",SYS_PATH_ROOT2);
			break;
		case DBhisttu_type_ulog:
			sprintf(fname,"%s/HISTORY/ULOG/ulog.msg",SYS_PATH_ROOT2);
			break;
		default:
			break;
	}
	return;
}

/*是否需要发现cfg不对后重新读取SOE,CO,FLOW,ULOG重新生成*/
static void histtuDll_Scan(const char *fname,const struct VHisTtuDll *dll,const struct VHisTtuRcd *pcfg)
{
	FILE *fp;
	struct stat st;
	struct VSysClock clk;
	char temp[128] = {0};
	int len,num_wp,file_head;
	
	fp = fopen(fname, "r+b");
	if (fp == NULL)
		return;
	
	fclose(fp);
	stat(fname, &st);

	if((pcfg->num == 0)||
	(pcfg->num_wp == 0)||
	(pcfg->file_head == 0)||
	(pcfg->file_wp == 0)||
	(pcfg->file_size == 0))
	{
		remove(fname);
		return;
	}

	//对一个文件的事件操作巡检格式，如有异常则记录事件
	//只对事件SOE, CO, flow, ulog OL HL 作比较
	
	if((dll->type >= DBhisttu_type_exv) && (dll->type <= DBhisttu_type_volttm))
	{
		return;
	}	
	
	if((pcfg->file_size < pcfg->file_wp)||(pcfg->file_head < pcfg->num_wp)||(pcfg->file_wp < pcfg->file_head)) //检查指针是否ok
	{
		DPRINT("*****dbhis：%s记录格式不对 filehead %d  num_wp %d file_wp %d file_size %d,删除文件 \n",fname,pcfg->file_head,pcfg->num_wp,pcfg->file_wp,pcfg->file_size);
		remove(fname);
		WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"%s记录格式不对 filehead %d  num_wp %d file_wp %d file_size %d,删除文件 \n",fname,pcfg->file_head,pcfg->num_wp,pcfg->file_wp,pcfg->file_size);
		return;
	}		
	
	//比较文件大小
	if(pcfg->file_size != st.st_size)
	{
		DPRINT("*****dbhis：%s文件大小不一样，记录%d，实际%d,删除文件 \n",fname,pcfg->file_size,(int)st.st_size);
		remove(fname);
		WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"%s文件大小不一样，记录%d，实际%d,删除文件 \n",fname,pcfg->file_size,st.st_size);
		return;
	}
	
	//检查文件头是否ok
	memset(histtu_buf,0,histtu_buf_len);
	GetSysClock(&clk, SYSCLOCK);
	histtuDll_NameGet(dll,&clk,temp);
	
	if (dll->type != DBhisttu_type_ulog)
	{
		sprintf(histtu_buf,msgFileHead,temp);
	}
	else
	{
#if (TYPE_USER != USER_JBDKY)
		strcpy(histtu_buf,"文件头：");
#endif
	}
	len = (int)strlen(histtu_buf);
	memset(histtu_buf,0,histtu_buf_len);

	if ((dll->type != DBhisttu_type_ol) && (dll->type != DBhisttu_type_hl))
	{
		sprintf(histtu_buf,msgeDataHead,g_Sys.InPara[SELFPARA_ID],0,3);
		len += (int)strlen(histtu_buf);
		num_wp = len - 9;
	}
	else
	{
		GetSysClock(&clk,SYSCLOCK);
		sprintf(histtu_buf,msgolDataHead,g_Sys.InPara[SELFPARA_ID],clk.wYear-2000,clk.byMonth,clk.byDay,3);
		len += (int)strlen(histtu_buf);
		num_wp = len - 6;
	}
	file_head = len;
	//比较
	if(num_wp != pcfg->num_wp)
	{
		DPRINT("*****dbhis：%s文件记录地址不一样，记录%d，实际%d,删除文件 \n",fname,pcfg->num_wp,num_wp);
		remove(fname);
		WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"%s文件记录地址不一样，记录%d，实际%d,删除文件 \n",fname,pcfg->num_wp,num_wp);
		return;
	}
	
	if(file_head != pcfg->file_head)
	{
		DPRINT("*****dbhis：%s文件头记录地址不一样，记录%d，实际%d,删除文件 \n",fname,pcfg->file_head,file_head);
		remove(fname);
		WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"%s文件头记录地址不一样，记录%d，实际%d,删除文件 \n",fname,pcfg->file_head,file_head);
		return;
	}

	fp = fopen(fname, "r+b");
	if (fp == NULL)
		return;
	if(fseek(fp,-2, SEEK_END) == 0)
	{
		len = (int)fread(temp, 1, 2, fp);
		if(len == 2)
		{
			if((temp[0] != 0x0D) || (temp[1] != 0x0A))
			{
				DPRINT("*****dbhis：%s文件不以回车符结尾,删除文件  \n",fname);
				fclose(fp);
				remove(fname);
				WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"%s文件不以回车符结尾,删除文件  \n",fname);
				return;
			}
		}
	}
	fclose(fp);
	
}

static struct VHisTtuDllMan *histtuDll_Get(WORD wID, int flag, struct VHisTtuDllMan *pdllman)
{
	int type;
	struct VHisTtuDllMan *p;

	if ((flag == SYSEV_FLAG_SSOE) || (flag == SYSEV_FLAG_DSOE))
			type = DBhisttu_type_soe;
	else if (flag == SYSEV_FLAG_CO)
			type = DBhisttu_type_co;
	else if (flag == SYSEV_FLAG_FLOW)
			type = DBhisttu_type_flowrev;
	else if (flag == SYSEV_FLAG_ULOG)
			type = DBhisttu_type_ulog;
	else if(flag == SYSEV_FLAG_HL)
			type = DBhisttu_type_hl;
	else if(flag == SYSEV_FLAG_OL)
			type = DBhisttu_type_ol;
	else
			return NULL;

	for (p = pdllman; p!=NULL; p=p->next)
	{
		if (p->dll.type == type)
		{
			if ((type == DBhisttu_type_ulog) || (type == DBhisttu_type_flowrev) || (type == DBhisttu_type_hl) || (type == DBhisttu_type_ol))
			    break;
			else if (p->id == wID)
			    break;
			if((type == DBhisttu_type_exv) || (type == DBhisttu_type_fixpt) || (type == DBhisttu_type_frz))
			    break;
		}
	}
	return p;
}

static int histtuDll_Freeze(DWORD minute, const struct VSysClock *pclk, struct VHisTtuDllMan *pdllman)
{  
    struct VHisTtuDllMan *p; 
	
	for (p = pdllman; p!=NULL; p=p->next)
    {		
		if (p->dll.md == 0) continue;

		if (pdllman == histtu_hour)
		{
			if ((minute%p->dll.md) != 0) continue;
		}  

		histtu_Record_Write(p, pclk);		
	}
    return OK;    
}

static void histtuDll_hour(const struct VSysClock *pclk)
{
	DWORD minute;
	minute = pclk->byMinute;
    histtuDll_Freeze(minute, pclk, histtu_hour);
}

static void histtuDll_day(const struct VSysClock *pclk)
{
  struct VSysClock tmp_clock;
	DWORD minute;

	memcpy(&tmp_clock, pclk, sizeof(tmp_clock));	
	tmp_clock.byHour = 0;
	tmp_clock.byMinute = 0;

	minute = CalendarClock(&tmp_clock);
	minute -= 2;	/*last day*/
	SystemClock(minute,&tmp_clock);
	histtuDll_Freeze(minute, &tmp_clock, histtu_day);
}

static void histtuDll_month(const struct VSysClock *pclk)
{
  struct VSysClock tmp_clock;
	DWORD minute;

	memcpy(&tmp_clock, pclk, sizeof(tmp_clock));	
	tmp_clock.byHour = 0;
	tmp_clock.byMinute = 0;

	minute = CalendarClock(&tmp_clock);
	minute -= 2;	/*last day*/
	SystemClock(minute,&tmp_clock);
	histtuDll_Freeze(minute, &tmp_clock, histtu_month);
}

/*SOE, CO, flow, ulog OL HL*/
static int histtuDll_RecCreate(FILE* fp, const struct VHisTtuDll *pdll, struct VHisTtuRcd *pcfg)
{
		char temp[128] = {0};
		struct VSysClock tmp_clock;	
		//取文件名名字
		
      	memset(histtu_buf,0,histtu_buf_len);
      	histtuDll_NameGet(pdll,NULL,temp);
        if (pdll->type != DBhisttu_type_ulog)
        {
            sprintf(histtu_buf,msgFileHead,temp);
        }
        else
        {
#if (TYPE_USER != USER_JBDKY)
            strcpy(histtu_buf,"文件头：");
#endif
        }
      	fwrite(histtu_buf, 1, strlen(histtu_buf), fp);
      	memset(histtu_buf,0,histtu_buf_len);
	

		if ((pdll->type != DBhisttu_type_ol) && (pdll->type != DBhisttu_type_hl))
		{
			sprintf(histtu_buf,msgeDataHead,g_Sys.InPara[SELFPARA_ID],0,3);
			fwrite(histtu_buf, 1, strlen(histtu_buf), fp);
			pcfg->num = 0;
			pcfg->num_wp = lfseek(fp, 0, SEEK_CUR) - 9;
			pcfg->file_wp = pcfg->num_wp+9;
		}
		else
		{
			GetSysClock(&tmp_clock,SYSCLOCK);
			sprintf(histtu_buf,msgolDataHead,g_Sys.InPara[SELFPARA_ID],tmp_clock.wYear-2000,tmp_clock.byMonth,tmp_clock.byDay,3);
			fwrite(histtu_buf, 1, strlen(histtu_buf), fp);
			pcfg->num = 0;
			pcfg->num_wp = lfseek(fp, 0, SEEK_CUR) - 6;
			pcfg->file_wp = pcfg->num_wp+6;
		}
		pcfg->file_head = pcfg->file_wp;
		pcfg->file_size = pcfg->file_head;
		return (pcfg->file_wp);
}

/*Exv, FIxP,voltd,volttd,voltm,volttm 需增加虚装置遥测结构*/
static int histtuDll_AttrCreate(FILE* fp, const struct VHisTtuDll *pdll, struct VHisTtuRcd *pcfg)
{
     char temp[128] = {0};
	 int len;
	 struct VSysClock tmp_clock;	
	 DWORD minute;
	
	 GetSysClock(&tmp_clock,SYSCLOCK);
	 minute = CalendarClock(&tmp_clock);
	 if(pdll->type != DBhisttu_type_fixpt)
	 	minute -= 2;	/*last day*/
	 SystemClock(minute,&tmp_clock);
	 
	 memset(histtu_buf,0,histtu_buf_len);

	 histtuDll_NameGet(pdll,&tmp_clock,temp);
	 sprintf(histtu_buf,msgFileHead,temp);
	 fwrite(histtu_buf, 1, strlen(histtu_buf), fp);
	 memset(temp,0,128);
	 if (pdll->type == DBhisttu_type_fixpt)
	 {
		 sprintf(temp,"%s,%02d%02d%02d,%02d,%02d\r\n",g_Sys.InPara[SELFPARA_ID],tmp_clock.wYear-2000,tmp_clock.byMonth,tmp_clock.byDay,
			0,03);
	 }
	 else if(pdll->type == DBhisttu_type_exv)
	 {
		 sprintf(temp,"%s,%02d%02d%02d,%02d\r\n",g_Sys.InPara[SELFPARA_ID],tmp_clock.wYear-2000,tmp_clock.byMonth,tmp_clock.byDay,03);
	 }
	 else
	 {
		 sprintf(temp,msgDataHead,g_Sys.InPara[SELFPARA_ID],tmp_clock.wYear-2000,tmp_clock.byMonth,tmp_clock.byDay);
	 }
	 
	 fwrite(temp, 1, strlen(temp), fp);

	 len = lfseek(fp, 0, SEEK_CUR);
	 pcfg->num_wp = len - 7; //只有fixp有用
	 pcfg->num = 0;
	 pcfg->file_wp = len;
	 pcfg->file_head = len;
	 pcfg->file_size = len;
	 return (len);
}

/*Frz*/
static int histtuDll_FrzCreate(FILE* fp, const struct VHisTtuDll *pdll, struct VHisTtuRcd *pcfg)
{
     char temp[128] = {0};
	 int  len;
	 struct VSysClock tmp_clock;	
	
	 GetSysClock(&tmp_clock,SYSCLOCK);
	 memset(histtu_buf,0,histtu_buf_len);
	 histtuDll_NameGet(pdll,NULL,temp);
	 sprintf(histtu_buf,msgFileHead,temp);
	 fwrite(histtu_buf, 1, strlen(histtu_buf), fp);
	 memset(temp,0,128);
	 sprintf(temp,"%s,%02d%02d%02d,%02d,%02d\r\n",g_Sys.InPara[SELFPARA_ID],tmp_clock.wYear-2000,tmp_clock.byMonth,tmp_clock.byDay,
		0,03);
	 
	 fwrite(temp, 1, strlen(temp), fp);

	 len = lfseek(fp, 0, SEEK_CUR);
	 pcfg->num_wp = len - 7; //只有fixp有用
	 pcfg->num = 0;
	 pcfg->file_wp = len;
	 pcfg->file_head = len;
	 pcfg->file_size = len;
	 return (len);
}

static int histtuDll_SoeGet(void *pdata, char *pbuf)
{
	int len;
	WORD i,j;
	int  value;
	struct VVirtualEqp *pVEqp;
	WORD wSendNum;
	struct VSysClock systime;
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
						CalClockTo(&(psoe->Time), &systime);
						if(psoe->byValue == 0x01)
							value = 0x01;
						else if(psoe->byValue == 0x81)
							value = 0x02;
						else
							value = 0x03;

						sprintf(pbuf,"0x%X,%01d,%04d-%02d-%02d %02d:%02d:%02d.%03d\r\n", 
						psoe->wNo + YX_Addr,value,systime.wYear, systime.byMonth, systime.byDay, systime.byHour, 
						systime.byMinute, systime.bySecond, systime.wMSecond);
						len = (int)strlen(pbuf);
						return len;
					}
				}
				for(j = 0;j < pVEqp->Cfg.wDYXNum ;j++)
				{
					ReadDYXSendNo(i,j,&wSendNum);
					if(wSendNum == psoe->wNo)
					{
						pdsoe = (struct VDBDSOE*)pdata;
						CalClockTo(&(pdsoe->Time), &systime);
						if((pdsoe->byValue1 == 0x01) && (pdsoe->byValue2 == 0x01))
							value = 0x00;
						else if((pdsoe->byValue1 == 0x81) && (pdsoe->byValue2 == 0x01))
							value = 0x01;
						else if((pdsoe->byValue1 == 0x01) && (pdsoe->byValue2 == 0x81))
							value = 0x02;
						else
							value = 0x03;

						sprintf(pbuf,"0x%X,%01d,%04d-%02d-%02d %02d:%02d:%02d.%03d\r\n", 
						psoe->wNo + YX_Addr,value,systime.wYear, systime.byMonth, systime.byDay, systime.byHour, 
						systime.byMinute, systime.bySecond, systime.wMSecond);
						len = (int)strlen(pbuf);
						return len;
					}
				}
			}
		}
	}
	return len;
}

static int histtudll_CoGet(void *pdata, char *pbuf)
{
	int len;
	char command[16] = {0};
	char commandval[10] = {0};
	struct VSysClock systime;
	struct VDBCO *pco = (struct VDBCO*)pdata;

	CalClockTo(&pco->Time, &systime);
	if (pco->cmd == MI_YKSELECT)
		strcpy(command, "选择");
	else if (pco->cmd == MI_YKOPRATE)
		strcpy(command, "执行");
	else 
		strcpy(command, "撤销");
	
	if(pco->val)
		strcpy(commandval, "合");
	else
		strcpy(commandval, "分");
	
	sprintf(pbuf, "0x%04X,%s,%s,%04d-%02d-%02d %02d:%02d:%02d.%03d\r\n",
					pco->wNo,command,commandval,systime.wYear, systime.byMonth, systime.byDay, systime.byHour, 
					systime.byMinute, systime.bySecond, systime.wMSecond);
	len = (int)strlen(pbuf);
	return len;
}

static int histtudll_hlGet(void *pdata, char *pbuf)
{
		int len;
		VPbCalTime *pbtime = (VPbCalTime*)pdata; //取数据，内容待扩
		
		sprintf(pbuf,"%04d-%02d-%02d %02d:%02d:%02d.000,%04d-%02d-%02d %02d:%02d:%02d.000,0X4015,%d\r\n",
		pbtime->startT.wYear, pbtime->startT.byMonth, pbtime->startT.byDay, pbtime->startT.byHour, pbtime->startT.byMinute, pbtime->startT.bySecond,
		pbtime->endT.wYear, pbtime->endT.byMonth, pbtime->endT.byDay, pbtime->endT.byHour,pbtime->endT.byMinute, pbtime->endT.bySecond,(int)pbtime->total_t);
		len = (int)strlen(pbuf);
		return len;
}

static int histtudll_olGet(void *pdata, char *pbuf)
{
		int len;
		VPbCalTime *pbtime = (VPbCalTime*)pdata; //取数据，内容待扩
		
		sprintf(pbuf,"%04d-%02d-%02d %02d:%02d:%02d.000,%04d-%02d-%02d %02d:%02d:%02d.000,0X4015,%d\r\n",
		pbtime->startT.wYear, pbtime->startT.byMonth, pbtime->startT.byDay, pbtime->startT.byHour, pbtime->startT.byMinute, pbtime->startT.bySecond,
		pbtime->endT.wYear, pbtime->endT.byMonth, pbtime->endT.byDay, pbtime->endT.byHour,pbtime->endT.byMinute, pbtime->endT.bySecond,(int)pbtime->total_t);
		len = (int)strlen(pbuf);
		return len;
}

static int histtudll_FlowGet(void *pdata, char *pbuf)
{
		int len = 0;
//		WORD i,num;
//		float fvalue;
//		struct VSysClock time;
//		struct VSysFlowEventInfo *pflowinfo = (struct VSysFlowEventInfo *)pdata;
//		struct VDBFLOW *pflow;
//		struct VHisDllMan *pdllman;
//		char*p;
//		
//		pdllman =  hisDll_Get(0, SYSEV_FLAG_FLOW, his_event);
//		
//		num = pflowinfo->num;
//		if(num > 0)
//			pdllman->index->num += num-1;//写一次只增加一次，所以这边加上
//		len = 0;
//		for(i=0;i<num;i++)
//		{
//			p = pbuf + len;
//			pflow = &pflowinfo->dbflow[i];
//			CalClockTo(&pflow->Time, &time);
//			fvalue = *((float*)&pflow->val);

//			sprintf(p, "    <DI ioa=\"%d\" tm=\"%02d%02d%02d_%02d%02d%02d_%03d\" val=\"%.2f\" />\r\n",
//							pflow->wNo+0x6401, time.wYear-2000, time.byMonth, time.byDay, time.byHour, 
//							time.byMinute, time.bySecond,time.wMSecond,fvalue);

//			len += strlen(p);
//		}
		return len;
}

//日志类型
static int histtudll_UlogGet(void *pdata, char *pbuf)
{
     int len;
     struct VSysClock systime;
	 struct VSysEventInfo *plog = (struct VSysEventInfo*)pdata;

	 CalClockTo(&plog->time, &systime);

	sprintf(pbuf, "%02d,%04d-%02d-%02d %02d:%02d:%02d.%03d,%s,%d\r\n",
	 	     (int)plog->type, systime.wYear, systime.byMonth, systime.byDay, systime.byHour, 
	 	     systime.byMinute, systime.bySecond, systime.wMSecond, plog->msg, plog->para);

	 len = (int)strlen(pbuf);
	 return len;
}

//极值记录历史文件，虚装置从实装置读取,实装置要配置极值
static int histtuDll_ExvGet(void *pdata, char *pbuf)
{
     int i,num;
	 float fmax,fmin;
	 DWORD len, len1;
	 long dmax,dmin;
	 char *buf=pbuf;
	 struct VVirtualEqp *peqp;
	 struct VTrueEqp *pTEqp;
	 struct VVirtualYC *pVYC;
	 struct VMaxMinYC *pYC;
	 struct VTrueYCCfg *pTrueYcCfg;
	 struct VSysClock time1, time2;
	 struct VHisTtuInfo *pinfo = (struct VHisTtuInfo*)pdata;
	 WORD ycno; 
	 WORD wSendNum;

	 peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	 if (peqp == NULL) return 0;
	 num = 0;
	 for (ycno=0;ycno <peqp->Cfg.wYCNum; ycno++)
	 {
		 i = ycno; 
		 pTEqp = g_Sys.Eqp.pInfo[peqp->pYC[i].wTEID].pTEqpInfo;
		 pTrueYcCfg = pTEqp->pYCCfg + peqp->pYC[i].wOffset;
		 if(!(pTrueYcCfg->dwCfg & 0x100))
		 	continue;
		 num++;
	 }
	 len = 0;
	 sprintf(buf,"最大值:%d",num); //个数待确认
	 len1 = strlen(buf);
	 fwrite(buf,1,len1,pinfo->fp);
	 len += len1;
	 memset(buf,0,len1+1);
	 for (ycno=0;ycno <peqp->Cfg.wYCNum; ycno++)
	 {
		 i = ycno; 
		 pTEqp = g_Sys.Eqp.pInfo[peqp->pYC[i].wTEID].pTEqpInfo;
		 pTrueYcCfg = pTEqp->pYCCfg + peqp->pYC[i].wOffset;
		 //if (!(peqp->pYC[i].dwCfg & 0x800)) continue;
		 if(!(pTrueYcCfg->dwCfg & 0x100)) 
		 	continue;
		 ReadYCSendNo((WORD)(pinfo->id),ycno,&wSendNum); //
		 pYC = &(pTEqp->pMaxMinYC[pTrueYcCfg->wMaxMinNo]);
		 if(pTrueYcCfg->wMaxMinNo == 0xFFFF) 
		 	continue;//实装置没配置
		 pVYC = peqp->pYC + i;	 
		 if(pYC->lMin == 0x7FFFFFFF)
			pYC->lMin = 0;
		 CalClockTo(&pYC->max_tm, &time1);
		 CalClockTo(&pYC->min_tm, &time2);
		 dmax = pYC->lMax*pVYC->lA/pVYC->lB + pVYC->lC;
		 dmin = pYC->lMin*pVYC->lA/pVYC->lB + pVYC->lC;
		 YCLongToFloat((WORD)(pinfo->id),i,dmax,(BYTE*)&fmax);
		 YCLongToFloat((WORD)(pinfo->id),i,dmin,(BYTE*)&fmin); 
		 sprintf(buf, ",0x%04X,%.3f,%04d-%02d-%02d %02d:%02d:%02d.00",
	 	         YC_Addr+wSendNum,fmax, time1.wYear, time1.byMonth, time1.byDay, time1.byHour, time1.byMinute, time1.bySecond);
		 len1 = strlen(buf);
		 fwrite(buf, 1, len1, pinfo->fp);
		 len += len1;
		 memset(buf,0,len1+1);
	 }
	strcpy(buf,"\r\n"); //个数待确认
	len1 = strlen(buf);
	fwrite(buf,1,len1,pinfo->fp);
	len += len1;
	memset(buf,0,len1+1);
	 
	 
	sprintf(buf,"最小值:%d",num); //个数待确认
	len1 = strlen(buf);
	fwrite(buf,1,len1,pinfo->fp);
	len += len1;
	memset(buf,0,len1+1);
	 for (ycno=0;ycno <peqp->Cfg.wYCNum; ycno++)
	 {
		 i = (int)ycno; 
		 pTEqp = g_Sys.Eqp.pInfo[peqp->pYC[i].wTEID].pTEqpInfo;
		 pTrueYcCfg = pTEqp->pYCCfg + peqp->pYC[i].wOffset;
		 //if (!(peqp->pYC[i].dwCfg & 0x800)) continue;
		 if(!(pTrueYcCfg->dwCfg & 0x100)) 
		 	continue;		 
		 pYC = &(pTEqp->pMaxMinYC[pTrueYcCfg->wMaxMinNo]);
		 if(pTrueYcCfg->wMaxMinNo == 0xFFFF) 
		 	continue;//实装置没配置
		 ReadYCSendNo((WORD)(pinfo->id),ycno,&wSendNum); //
		 pVYC = peqp->pYC + i;	 
		 if(pYC->lMin == 0x7FFFFFFF)
			pYC->lMin = 0;
		 CalClockTo(&pYC->max_tm, &time1);
		 CalClockTo(&pYC->min_tm, &time2);
		 dmax = pYC->lMax*pVYC->lA/pVYC->lB + pVYC->lC;
		 dmin = pYC->lMin*pVYC->lA/pVYC->lB + pVYC->lC;
		 YCLongToFloat((WORD)(pinfo->id),i,dmax,(BYTE*)&fmax);
		 YCLongToFloat((WORD)(pinfo->id),i,dmin,(BYTE*)&fmin); 
		 sprintf(buf, ",0x%04X,%.3f,%04d-%02d-%02d %02d:%02d:%02d.00",
	 	         YC_Addr+wSendNum,fmin, time2.wYear, time2.byMonth, time2.byDay, time2.byHour, time2.byMinute, time2.bySecond);
		 len1 = strlen(buf);
		 fwrite(buf, 1, len1, pinfo->fp);
		 len += len1;
		 memset(buf,0,len1+1);
	 }
	strcpy(buf,"\r\n"); //个数待确认
	len1 = strlen(buf);
	fwrite(buf,1,len1,pinfo->fp);
	len += len1;
	memset(buf,0,len1+1); 
	return (int)len;
}

//虚设备 
WORD sendno_to_ycnum(const struct VVirtualEqp *peqp,WORD wSendNum)
{
	WORD j;
	if(peqp == NULL)
		return 0;
	for(j = 0; j < peqp->Cfg.wYCNum;j++)
	{
		if(peqp->pYC[j].wSendNo == wSendNum)
		{
			return j;
		}
	}
	return wSendNum;
}

//电压日合格率
static int histtuDll_VoltdGet(void *pdata, char *pbuf)
{
	DWORD len,len1;
	struct VDBYCF_L DBYCF_L;
	long i,temp;
	float f;
	WORD wSendNum;
	struct VVirtualEqp *peqp;
	struct VHisTtuInfo *pinfo = (struct VHisTtuInfo*)pdata;
	
	len = 0;
	peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	if (peqp == NULL) return 0;
	sprintf(pbuf,"%d,%04d-%02d-%02d %02d:%02d:%02d.%03d",3,
	pinfo->clock.wYear, pinfo->clock.byMonth, pinfo->clock.byDay, pinfo->clock.byHour, 
	pinfo->clock.byMinute, pinfo->clock.bySecond,pinfo->clock.wMSecond);
	len += strlen(pbuf);
	fwrite(pbuf,1,len,pinfo->fp);
	memset(pbuf,0,len+1);
    for(i = 0; i < 3;i++)
	{
		wSendNum = (WORD)(0x5D+i);
		DBYCF_L.wNo = sendno_to_ycnum(peqp,wSendNum);
		atomicReadVEYCF_L(peqp, 1, sizeof(struct VDBYCF_L), &DBYCF_L);
		if(DBYCF_L.byFlag & (1<<6))
		{
			temp = DBYCF_L.lValue;
			memcpy((BYTE*)&f,(BYTE*)&temp,4);
		}
		else
			 YCLongToFloat((WORD)(pinfo->id),DBYCF_L.wNo,DBYCF_L.lValue,(BYTE*)&f);
		
		sprintf(pbuf,",0x%04X,%.3f",YC_Addr+wSendNum,f);
		len1 = strlen(pbuf);
		fwrite(pbuf,1,len1,pinfo->fp);
		len+=len1;
		memset(pbuf,1,len1+1);
	} 
	strcpy(pbuf,"\r\n");
	len1 = strlen(pbuf);
	fwrite(pbuf,1,len1,pinfo->fp);
	len+=len1;
	memset(pbuf,0,len1+1);
	return (int)len;
 }

 //电压日越限
static int histtuDll_VolttdGet(void *pdata, char *pbuf)
{
	DWORD i, len,len1;
	struct VDBYCF_L DBYCF_L;
	long temp;
	float f;
	WORD wSendNum;
	struct VVirtualEqp *peqp;
	struct VHisTtuInfo *pinfo = (struct VHisTtuInfo*)pdata;
	
	len = 0;
	peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	if (peqp == NULL) return 0;
	sprintf(pbuf,"越上限:%d",3);
	len1 = strlen(pbuf);
	len+=len1;
	fwrite(pbuf,1,len1,pinfo->fp);
	memset(pbuf,0,len1+1);
    for(i = 0; i < 3;i++)
	{
        wSendNum =  (WORD)(0x60+i*2);
		DBYCF_L.wNo = sendno_to_ycnum(peqp,wSendNum);
		atomicReadVEYCF_L(peqp, 1, sizeof(struct VDBYCF_L), &DBYCF_L);
		if(DBYCF_L.byFlag & (1<<6))
		{
			temp = DBYCF_L.lValue;
			memcpy((BYTE*)&f,(BYTE*)&temp,4);
		}
		else
			 YCLongToFloat((WORD)(pinfo->id),DBYCF_L.wNo,DBYCF_L.lValue,(BYTE*)&f);
		
		sprintf(pbuf,",0x%04X,%.3f",YC_Addr+wSendNum,f);
		len1 = strlen(pbuf);
		fwrite(pbuf,1,len1,pinfo->fp);
		len+=len1;
		memset(pbuf,0,len1+1);
	} 
	strcpy(pbuf,"\r\n");
	len1 = strlen(pbuf);
	fwrite(pbuf,1,len1,pinfo->fp);
	len+=len1;
	memset(pbuf,0,len1+1);
	
	sprintf(pbuf,"越下限:%d",3);
	len1 = strlen(pbuf);
	len+=len1;
	fwrite(pbuf,1,len1,pinfo->fp);
	memset(pbuf,0,len1+1);
    for(i = 0; i < 3;i++)
	{ 
		wSendNum = (WORD)(0x61+i*2);
		DBYCF_L.wNo = sendno_to_ycnum(peqp,wSendNum);
		
		atomicReadVEYCF_L(peqp, 1, sizeof(struct VDBYCF_L), &DBYCF_L);
		if(DBYCF_L.byFlag & (1<<6))
		{
			temp = DBYCF_L.lValue;
			memcpy((BYTE*)&f,(BYTE*)&temp,4);
		}
		else
			 YCLongToFloat((WORD)(pinfo->id),DBYCF_L.wNo,DBYCF_L.lValue,(BYTE*)&f);
		
		sprintf(pbuf,",0x%04X,%.3f",YC_Addr+wSendNum,f);
		len1 = strlen(pbuf);
		fwrite(pbuf,1,len1,pinfo->fp);
		len+=len1;
		memset(pbuf,0,len1+1);
	} 
	strcpy(pbuf,"\r\n");
	len1 = strlen(pbuf);
	fwrite(pbuf,1,len1,pinfo->fp);
	len+=len1;
	memset(pbuf,0,len1+1);	
	return (int)len;
}

//电压月合格率
static int histtuDll_VoltmGet(void *pdata, char *pbuf)
{
	DWORD i, len,len1;
	struct VDBYCF_L DBYCF_L;
	long temp;
	float f;
    WORD wSendNum;
	struct VVirtualEqp *peqp;
	struct VHisTtuInfo *pinfo = (struct VHisTtuInfo*)pdata;

	peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	if (peqp == NULL) return 0;
	len  = 0;
	sprintf(pbuf,"%d,%04d-%02d-%02d %02d:%02d:%02d.%03d",3,
	pinfo->clock.wYear, pinfo->clock.byMonth, pinfo->clock.byDay, pinfo->clock.byHour, 
	pinfo->clock.byMinute, pinfo->clock.bySecond,pinfo->clock.wMSecond);
	len += strlen(pbuf);
	fwrite(pbuf,1,len,pinfo->fp);
	memset(pbuf,0,len+1);
    for(i = 0; i < 3;i++)
	{
        wSendNum= (WORD)(0x66+i);
		DBYCF_L.wNo = sendno_to_ycnum(peqp,wSendNum);
		atomicReadVEYCF_L(peqp, 1, sizeof(struct VDBYCF_L), &DBYCF_L);
		if(DBYCF_L.byFlag & (1<<6))
		{
			temp = DBYCF_L.lValue;
			memcpy((BYTE*)&f,(BYTE*)&temp,4);
		}
		else
			 YCLongToFloat((WORD)(pinfo->id),DBYCF_L.wNo,DBYCF_L.lValue,(BYTE*)&f);
		
		sprintf(pbuf,",0x%04X,%.3f",YC_Addr+wSendNum,f);
		len1 = strlen(pbuf);
		fwrite(pbuf,1,len1,pinfo->fp);
		len+=len1;
		memset(pbuf,0,len1+1);
	} 
	strcpy(pbuf,"\r\n");
	len1 = strlen(pbuf);
	fwrite(pbuf,1,len1,pinfo->fp);
	len+=len1;
	memset(pbuf,0,len1+1);
	return (int)len;
 }

 //电压月越限
static int histtuDll_VolttmGet(void *pdata, char *pbuf)
{
	DWORD i, len,len1;
	struct VDBYCF_L DBYCF_L;
	long temp;
	float f;
    WORD wSendNum;
	struct VVirtualEqp *peqp;
	struct VHisTtuInfo *pinfo = (struct VHisTtuInfo*)pdata;

	peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	if (peqp == NULL) return 0;
	len = 0;
	sprintf(pbuf,"越上限:%d",3);
	len1 = strlen(pbuf);
	len+=len1;
	fwrite(pbuf,1,len1,pinfo->fp);
	memset(pbuf,0,len1+1);
    for(i = 0; i < 3;i++)
	{
        wSendNum = (WORD)(0x69+i*2);
		DBYCF_L.wNo = sendno_to_ycnum(peqp,wSendNum);
		atomicReadVEYCF_L(peqp, 1, sizeof(struct VDBYCF_L), &DBYCF_L);
		if(DBYCF_L.byFlag & (1<<6))
		{
			temp = DBYCF_L.lValue;
			memcpy((BYTE*)&f,(BYTE*)&temp,4);
		}
		else
			 YCLongToFloat((WORD)(pinfo->id),DBYCF_L.wNo,DBYCF_L.lValue,(BYTE*)&f);
		
		sprintf(pbuf,",0X%04X,%.3f",YC_Addr+wSendNum,f);
		len1 = strlen(pbuf);
		fwrite(pbuf,1,len1,pinfo->fp);
		len+=len1;
		memset(pbuf,0,len1+1);
	} 
	strcpy(pbuf,"\r\n");
	len1 = strlen(pbuf);
	fwrite(pbuf,1,len1,pinfo->fp);
	len+=len1;
	memset(pbuf,0,len1+1);
	
	sprintf(pbuf,"越下限:%d",3);
	len1 = strlen(pbuf);
	len+=len1;
	fwrite(pbuf,1,len1,pinfo->fp);
	memset(pbuf,0,len1+1);
    for(i = 0; i < 3;i++)
	{
        wSendNum = (WORD)(0x6A+i*2);
		DBYCF_L.wNo = sendno_to_ycnum(peqp,wSendNum);
		atomicReadVEYCF_L(peqp, 1, sizeof(struct VDBYCF_L), &DBYCF_L);
		if(DBYCF_L.byFlag & (1<<6))
		{
			temp = DBYCF_L.lValue;
			memcpy((BYTE*)&f,(BYTE*)&temp,4);
		}
		else
			YCLongToFloat((WORD)(pinfo->id),DBYCF_L.wNo,DBYCF_L.lValue,(BYTE*)&f);
		
		sprintf(pbuf,",0x%04X,%.3f",YC_Addr+wSendNum,f);
		len1 = strlen(pbuf);
		fwrite(pbuf,1,len1,pinfo->fp);
		len+=len1;
		memset(pbuf,0,len1+1);
	} 
	strcpy(pbuf,"\r\n");
	len1 = strlen(pbuf);
	fwrite(pbuf,1,len1,pinfo->fp);
	len+=len1;
	memset(pbuf,0,len1+1);	
	return (int)len;
}

static int histtuDll_FixptGet(void *pdata, char *pbuf)
{
     DWORD i, len,len1;
	 float f;
	 char *buf=pbuf;
	 struct VVirtualEqp *peqp;
	 struct VDBYCF_L DBYCF_L;
	 long temp;
     WORD wSendNum;
	 struct VHisTtuInfo *pinfo = (struct VHisTtuInfo*)pdata;

   
	 memset(histtu_buf,0,histtu_buf_len);
	 peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	 if (peqp == NULL) return 0;
	
     len = 0;
#if (TYPE_USER == USER_JBDKY)
	 sprintf(pbuf,"%d,%04d-%02d-%02d %02d:%02d:%02d.00",pinfo->sec,peqp->YCHourFrz.wNum,pinfo->clock.wYear,pinfo->clock.byMonth,pinfo->clock.byDay,
	 pinfo->clock.byHour,pinfo->clock.byMinute,pinfo->clock.bySecond);
#else
	 sprintf(pbuf,"第%d节定点数据：%d,%04d-%02d-%02d %02d:%02d:%02d.00",pinfo->sec,peqp->YCHourFrz.wNum,pinfo->clock.wYear,pinfo->clock.byMonth,pinfo->clock.byDay,
	 pinfo->clock.byHour,pinfo->clock.byMinute,pinfo->clock.bySecond);
#endif
	 len+=strlen(pbuf);
	 fwrite(pbuf,1,len,pinfo->fp);
	 memset(pbuf,0,len+1);
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
		   YCLongToFloat((WORD)(pinfo->id),DBYCF_L.wNo,DBYCF_L.lValue,(BYTE*)&f);
		ReadYCSendNo((WORD)(pinfo->id),DBYCF_L.wNo,&wSendNum); //
		sprintf(buf, ",0x%04X,%.3f",wSendNum+YC_Addr, f);
		len1 = strlen(buf);
		fwrite(buf, 1, len1, pinfo->fp);
		len += len1;
		memset(buf,0,len1+1);
	 }
	 
	 strcpy(buf,"\r\n");
	 len1 = strlen(buf);
	 fwrite(buf, 1, len1, pinfo->fp);
	 len += len1;
	 memset(buf,0,len1+1);
	 return (int)len;
}

//冻结数据，有DDHourFrz  的配置;虚装置暂时无电度配置，暂时用实装置读取，实装置设置电度曲线
static int histtuDll_FrzGet(void *pdata, char *pbuf)
{
	DWORD i, len,len1;
	float f;
    DWORD dwValue = 0;
	char *buf=(char*)pbuf;
	struct VVirtualEqp *peqp;
	struct VDBDD DBDD;
	struct VHisTtuInfo *pinfo = (struct VHisTtuInfo*)pdata;

	peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	
	len = 0;
	if (peqp == NULL) return 0;
#if (TYPE_USER == USER_JBDKY)
	sprintf(buf, "%d,%04d-%02d-%02d %02d:%02d:%02d", pinfo->sec,4, 
	pinfo->clock.wYear, pinfo->clock.byMonth, pinfo->clock.byDay, pinfo->clock.byHour, pinfo->clock.byMinute, pinfo->clock.bySecond);
#else
	sprintf(buf, "第%d节定点数据：%d,%04d-%02d-%02d %02d:%02d:%02d", pinfo->sec,4, 
	        pinfo->clock.wYear, pinfo->clock.byMonth, pinfo->clock.byDay, pinfo->clock.byHour, pinfo->clock.byMinute, pinfo->clock.bySecond);
#endif
	len1 = strlen(buf);
	fwrite(buf, 1, len1, pinfo->fp);
	len += len1;
	memset(buf,0,len1+1);
 
	for (i=0; i<peqp->DDHourFrz.wNum; i++)
	{
		DBDD.wNo = peqp->DDHourFrz.pwIndex[i];
		if((DBDD.wNo%32 > 7)||(DBDD.wNo%32 < 4)) continue; //6405 ~ 6408
		ReadRangeAllDD((WORD)(pinfo->id), DBDD.wNo, 1, 100, (long *)&dwValue);
		
		memcpy((BYTE*)&f, (BYTE*)&dwValue, 4);
		
		sprintf(buf,",0x%04X,%.3f",DBDD.wNo+DD_Addr,f);
		len1 = strlen(buf);
		fwrite(buf,1,len1,pinfo->fp);
		len += len1;
		memset(buf,0,len1+1);
	}
	
    strcpy(buf, "\r\n");
    len1 = strlen(buf);
    fwrite(buf, 1, len1, pinfo->fp);
    len += len1;
    memset(buf,0,len1+1);
    return (int)len;
}

//日冻结类型标示，第一次日冻结值;暂无虚装置，暂时用实装置 读取，实装置 需设置电度曲线
static int histtuDll_FrzDayGet(void *pdata, char *pbuf)
{
	DWORD i, len,len1;
	float f;
    DWORD dwValue = 0;
	char *buf=(char*)pbuf;
	struct VVirtualEqp *peqp;
	struct VDBDD DBDD;
	struct VHisTtuInfo *pinfo = (struct VHisTtuInfo*)pdata;

	peqp = g_Sys.Eqp.pInfo[pinfo->id].pVEqpInfo;
	
	len = 0;
	if (peqp == NULL) return 0;
	
#if (TYPE_USER == USER_JBDKY)
	sprintf(buf, "%d,%04d-%02d-%02d %02d:%02d:%02d",pinfo->sec,10, 
	pinfo->clock.wYear, pinfo->clock.byMonth, pinfo->clock.byDay, pinfo->clock.byHour, pinfo->clock.byMinute, pinfo->clock.bySecond);
#else
	sprintf(buf, "第%d节定点数据：%d,%04d-%02d-%02d %02d:%02d:%02d",pinfo->sec,10, 
	pinfo->clock.wYear, pinfo->clock.byMonth, pinfo->clock.byDay, pinfo->clock.byHour, pinfo->clock.byMinute, pinfo->clock.bySecond);
#endif
	len1 = strlen(buf);
	fwrite(buf, 1, len1, pinfo->fp);
	len += len1;
	memset(buf,0,len1+1);
 
	for (i=0; i<peqp->DDHourFrz.wNum; i++)
	{
		DBDD.wNo = peqp->DDHourFrz.pwIndex[i];
		if((DBDD.wNo%32 > 17)||(DBDD.wNo%32 < 8)) continue; //6409 ~ 64012
		ReadRangeAllDD((WORD)(pinfo->id), DBDD.wNo, 1, 100, (long *)&dwValue);
		
		memcpy((BYTE*)&f, (BYTE*)&dwValue, 4);
		
		sprintf(buf,",0x%04X,%.3f",DBDD.wNo+DD_Addr,f);
		len1 = strlen(buf);
		fwrite(buf,1,len1,pinfo->fp);
		len += len1;
		memset(buf,0,len1+1);
	}
	
    strcpy(buf, "\r\n");
    len1 = strlen(buf);
    fwrite(buf, 1, len1, pinfo->fp);
    len += len1;
    memset(buf,0,len1+1);
    return (int)len;
}

static void histtuDll_ResetExv(void)// 极值
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

//ulog msg内存足够64K 一条超不过64个字节，32个字
//static int histtu_File_ReWrite(FILE* fp, struct VHisRcd *pindex ,int num)
//{
//		int i,rd_offset,wr_offset, len, filelen,delnum;
//		
//		filelen = pindex->file_head;
//		rd_offset = pindex->file_head;
//	  delnum = 0;
//	  
//	  do
//	  {
//	  	fseek(fp, rd_offset, SEEK_SET);
//		 	len = fread(his_buf, 1, DBHIS_FILE_SEG, fp);
//	  	if(len > DBHIS_FILE_SEG)
//	  		len = DBHIS_FILE_SEG;
//	  		
//	  	rd_offset +=len;
//	  	
//	  	for (i=0; i<len; i++)
//	    {
//	         if ((his_buf[i] == '\r')&&(his_buf[i+1] == '\n'))
//	         	delnum++;
//	         	if(delnum == num)
//	            break;
//	    }
//	  }while((delnum < num)&&(len > 0));
//		
//	 if(rd_offset < (strlen(RecTail)+strlen(DataTail)))
//		 return pindex->file_head;
//	 if((rd_offset - len + i +2) >= pindex->file_wp)
//	 	return pindex->file_head;
//	 if(rd_offset >= pindex->file_wp)
//		 return pindex->file_head;
//	 
//	 fseek(fp, pindex->file_head, SEEK_SET);
//	 fwrite(his_buf+i+2, 1, len-(i+2), fp);
//	 wr_offset = lfseek(fp, 0, SEEK_CUR);
//	 filelen = wr_offset;
//	 
//	   while(rd_offset < pindex->file_size)
//     {
//         fseek(fp, rd_offset, SEEK_SET);
//				len = fread(his_buf, 1, DBHIS_FILE_SEG, fp);
//				 if(len <= 0)
//					 break;
//		 rd_offset += len;
//		 fseek(fp, wr_offset, SEEK_SET);
//		 fwrite(his_buf, 1, len, fp);
//		 wr_offset += len;
//			 filelen += len;
//     }
//		filelen -= (strlen(RecTail)+strlen(DataTail));
//	 return filelen;     
//}

static int histtu_Ram_ReWrite(char *pbuf, const struct VHisTtuRcd *pindex, int num)
{
	 int i, filelen,len, delnum, rd_offset,wr_offset;
	 char *buf1, *buf2;

	 buf1 = pbuf + pindex->file_head;
	 
	 rd_offset = pindex->file_head;
	 delnum = 0;
	 do
	 {
	     len = pindex->file_wp - rd_offset;
		 if (len > DBHISTTU_FILE_SEG)
		 	len = DBHISTTU_FILE_SEG;
		
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

	 if (rd_offset > pindex->file_wp)
	     return pindex->file_head;
	 
	 buf2 = buf1-(len-(i+2));
	 buf1 = pbuf + pindex->file_head;
	 if((len-(i+2)) < 0)
		return pindex->file_head;
	 memcpy(buf1, buf2, (DWORD)(len-(i+2)));
	
	 wr_offset = pindex->file_head + len - (i+2);
	 filelen = wr_offset;
	 
     while(rd_offset< pindex->file_size)
     {
         buf1 = pbuf + rd_offset;
		 buf2 = pbuf + wr_offset; 

		 if ((pindex->file_size - rd_offset) > DBHISTTU_FILE_SEG)
		 	len = DBHISTTU_FILE_SEG;
		 else
		  {
			len = pindex->file_size - rd_offset;
		 }
		
		 rd_offset += len;
		 memcpy(buf2, buf1, (DWORD)len);
		 wr_offset += len;
		 filelen += len;
     }	
	 filelen -= (int)strlen(msgFileTail);

	 if((0x0A != pbuf[filelen -1])||(0x0D != pbuf[filelen -2]))
	 {
		shellprintf("index Error");
	 }
	 return filelen;
}

static void histtu_Event_RamSort(const struct VHisTtuDllMan *pdllman, int num)
{
    FILE *fp;
	int len, total;
	char fname[128] = {0};
    char *pbuf = eventttubuf;

	total = pdllman->index->num + num;
	if (total > pdllman->dll.max)
	{
	    histtuDll_FileNameGet(&pdllman->dll, NULL, fname);
	    fp = fopen(fname, "r+");
	    if (fp == NULL)
		    return;

		len = pdllman->index->file_size;
		if (len > eventttubuflen)
            len = eventttubuflen;
		fread(pbuf, 1, (DWORD)len, fp); 
		fclose(fp);
	    len = histtu_Ram_ReWrite(pbuf, pdllman->index, (total-pdllman->dll.max));
		pdllman->index->file_wp = len;
		pdllman->index->first = 1;
	}
}

static void histtu_Event_RamWrite(const struct VHisTtuDllMan *pdllman, int num)
{
	int  dindex,i;
	char ftemp[128]= {0};
	char *pbuf;
	DWORD len;
	struct VHisTtuSocket *psocket;
	
	pbuf = eventttubuf;
	
	memset(histtu_buf,0,histtu_buf_len);

   	dindex = pdllman->index->file_wp;

	for (i=0; i<num; i++)
	{
#if (TYPE_USER != USER_JBDKY)
		if (pdllman->dll.type == DBhisttu_type_soe)
			sprintf(histtu_buf,"第%d条SOE：",pdllman->index->num+1);
		else if (pdllman->dll.type == DBhisttu_type_co)
		     sprintf(histtu_buf,"第%d条遥控操作记录：",pdllman->index->num+1);
		else if ((pdllman->dll.type == DBhisttu_type_hl)||(pdllman->dll.type == DBhisttu_type_ol))
		     sprintf(histtu_buf,"第%d个点：",pdllman->index->num+1);
		else if (pdllman->dll.type == DBhisttu_type_ulog)
		     sprintf(histtu_buf,"第%d条LOG：",pdllman->index->num+1);
#endif		
		len = strlen(histtu_buf);
		memcpy(pbuf+dindex, histtu_buf, len);
		memset(histtu_buf,0,len+1);
		dindex += (int)len;
			 
	    psocket = (struct VHisTtuSocket *)histtu_event_ary[i];
	    len = (DWORD)pdllman->dll.get_histtudll_data(psocket->buf, histtu_buf);
	    memcpy(pbuf+dindex, histtu_buf, len);
		memset(histtu_buf,0,len+1);
	    if (pdllman->index->num < pdllman->dll.max)
	    {
	        pdllman->index->num++;
	    }
		psocket->read_write = 0;
	    dindex += (int)len;
	}
	if(num>0)
		histtu_cfg_change = 1; 

	pdllman->index->file_wp = dindex;
    memcpy(pbuf + dindex, msgFileTail, strlen(msgFileTail));
	dindex += (int)strlen(msgFileTail);

	pdllman->index->file_size = dindex;

	sprintf(ftemp, "%04d", pdllman->index->num);//num 为 datarec  num
		
	dindex = pdllman->index->num_wp;
	memcpy(pbuf+dindex, ftemp, 4);
	
}

//从内存依次写文件
static void histtu_Event_RamFile(const struct VHisTtuDllMan *pdllman)
{
    FILE *fp;
	char fname[128]= {0};
	char *pbuf,*buf;
	int num = 0,rd_offset,len,i,diff,len1;
	
	if (pdllman->index->first == 0) return;

	histtuDll_FileNameGet(&pdllman->dll, NULL, fname);

	pbuf = eventttubuf;
	
	fp = fopen(fname, "w+");
	if (fp == NULL)
		return;
	fwrite(pbuf, 1, (DWORD)(pdllman->index->file_head), fp);
	buf = pbuf + pdllman->index->file_head;
	rd_offset = pdllman->index->file_head;
	memset(histtu_buf,0,histtu_buf_len);
	len1 = 0;
	do{
		len = pdllman->index->file_wp - rd_offset;
		if (len > DBHISTTU_FILE_SEG)
			len = DBHISTTU_FILE_SEG;
		if(len <= 0)
			break;
		for (i=0; i<len; i++) //查到：回车break，每次i都不会太大
		{
#if (TYPE_USER != USER_JBDKY)
			if(((BYTE)buf[i] == 0xA3) && ((BYTE)buf[i+1] == 0xBA)) // 查找汉字：
			{
				if (pdllman->dll.type == DBhisttu_type_soe)
				    sprintf(histtu_buf,"第%d条SOE：",num+1);
				else if (pdllman->dll.type == DBhisttu_type_co)
				     sprintf(histtu_buf,"第%d条遥控操作记录：",num+1);
				else if ((pdllman->dll.type == DBhisttu_type_hl)||(pdllman->dll.type == DBhisttu_type_ol))
				     sprintf(histtu_buf,"第%d个点：",num+1);
				else if (pdllman->dll.type == DBhisttu_type_ulog)
				     sprintf(histtu_buf,"第%d条LOG：",num+1);
				len1 = (int)strlen(histtu_buf);
				fwrite(histtu_buf, 1, (DWORD)len1, fp);
				memset(histtu_buf,0,(DWORD)(len1+1));
				len1 = rd_offset+i+2;
			}
#else
			len1 = rd_offset;
#endif
			if ((buf[i] == '\r')&&(buf[i+1] == '\n'))
			{
				num++;
				diff = (i+2+rd_offset)-len1;
				if(diff > 0)
					fwrite(pbuf+len1, 1, (DWORD)diff, fp);
				break;
			}
			if (num == pdllman->dll.max)
				break;
		}
		buf += i+2;
		rd_offset += i+2;
	}while((num <= pdllman->dll.max) && (pdllman->index->file_wp > rd_offset));
	fwrite(msgFileTail, 1, strlen(msgFileTail), fp);
	pdllman->index->file_wp = lfseek(fp, 0, SEEK_CUR);
	pdllman->index->file_size = lfseek(fp, 0, SEEK_CUR);
	sprintf(fname, "%04d", num);//
	pdllman->index->num = num;
	fseek(fp, pdllman->index->num_wp, SEEK_SET);
	fwrite(fname, 1, 4, fp);
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
	pdllman->index->first = 0;
}

/*co, flow, ulog, soe  都会进这块*/ 
static void histtu_Event_FileWrite(const struct VHisTtuDllMan *pdllman, int num)
{
	char fname[128]={0};
	char ftemp[128]={0};
	FILE *fp;
	int len,len1,i;
	struct VHisTtuSocket *psocket;
	
	if (pdllman->index->first)
	{
		histtu_Event_RamWrite(pdllman, num);
		return;
	}
	memset(histtu_buf,0,histtu_buf_len);
	
	histtuDll_FileNameGet(&pdllman->dll, NULL, fname);

	histtuDll_Scan(fname, &pdllman->dll, pdllman->index);
		
	fp = fopen(fname, "r+");
	if (fp == NULL)
	{
		fp = fopen(fname, "w+");
		if(fp==NULL) return;
		pdllman->dll.create_histtudll_file(fp, &pdllman->dll, pdllman->index);
	}
	len = lfseek(fp, 0, SEEK_END);
	fseek(fp,0, SEEK_SET);
	fseek(fp, pdllman->index->file_wp, SEEK_SET);
	
	memset(histtu_buf,0,histtu_buf_len); 
	 
	for (i=0; i<num; i++)
	{
#if (TYPE_USER != USER_JBDKY)
		if(pdllman->dll.type == DBhisttu_type_soe)
			sprintf(histtu_buf,"第%d条SOE：",pdllman->index->num+1);
		else if(pdllman->dll.type == DBhisttu_type_co)
			sprintf(histtu_buf,"第%d条遥控操作记录：",pdllman->index->num+1);
		else if((pdllman->dll.type == DBhisttu_type_hl)||(pdllman->dll.type == DBhisttu_type_ol))
			sprintf(histtu_buf,"第%d个点：",pdllman->index->num+1);
		else if(pdllman->dll.type == DBhisttu_type_ulog)
			sprintf(histtu_buf,"第%d条LOG：",pdllman->index->num+1);
#endif
		len = (int)strlen(histtu_buf);
		pdllman->index->file_wp += len;
		fwrite(histtu_buf, 1, (DWORD)len, fp);
		memset(histtu_buf,0,(DWORD)(len+1));

		psocket = (struct VHisTtuSocket *)histtu_event_ary[i];
		len = pdllman->dll.get_histtudll_data(psocket->buf, histtu_buf);

		fwrite(histtu_buf, 1, (DWORD)len, fp);
		memset(histtu_buf,0,(DWORD)(len+1));
		psocket->read_write = 0;
		if (pdllman->index->num < pdllman->dll.max)
		{
		    pdllman->index->num++;
		}
		pdllman->index->file_wp += len;
	}
	if (num>0)
	   histtu_cfg_change = 1; 
	fwrite(msgFileTail, 1, strlen(msgFileTail), fp);
	len1 = lfseek(fp , 0, SEEK_CUR);
	
	sprintf(ftemp, "%04d", pdllman->index->num);//
	 
	fseek(fp, pdllman->index->num_wp, SEEK_SET);
	fwrite(ftemp, 1, 4, fp);
	pdllman->index->file_size = len1;
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
}


/*exv, fixpt, frz 定点文件*/
static void histtu_Record_Write(const struct VHisTtuDllMan *pdllman, const struct VSysClock *pclk)
{
     FILE *fp;
	 int  len, len1;
	 char *pbuf = histtu_buf;
	 char fname[128] = {0};
	 char ftemp[128] = {0};
	 struct VHisTtuInfo info;
	 struct VVirtualEqp *peqp;
	 struct VSysClock tmp_clock;
	 DWORD minute;
	 BOOL fileerr = 0;
	
	 memset(histtu_buf,0,histtu_buf_len);
	 histtuDll_FileNameGet(&pdllman->dll, pclk, fname);
	
	 peqp = g_Sys.Eqp.pInfo[pdllman->id].pVEqpInfo;
	
	 if(peqp == NULL) return;
	 len = len1 = 0;
	 if((pdllman->dll.type == DBhisttu_type_exv) || (pdllman->dll.type == DBhisttu_type_fixpt))
	 { // 无遥测或无配置则不生成
		 if(peqp->Cfg.wYCNum == 0)
			 return;
		 if(peqp->YCHourFrz.wNum == 0)
		 	return;
	 }
	 else if(pdllman->dll.type == DBhisttu_type_frz)
	 {//冻结数据无电度则不生成
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
		 minute = minute - 24*60*DBHISTTU_DAY_MAX;
		 SystemClock(minute,&tmp_clock);
		 memset(ftemp, 0 ,sizeof(ftemp));
		 histtuDll_FileNameGet(&pdllman->dll,& tmp_clock, ftemp);
		 remove(ftemp);
		 memset(ftemp, 0 ,sizeof(ftemp));
		 
		 pdllman->dll.create_histtudll_file(fp, &pdllman->dll, pdllman->index);
		 pdllman->index->num = 0;
	 }
	 else // 比较一下指针是否与文件一致
	 {
		 if((pdllman->index->file_size < pdllman->index->file_wp)//检查指针是否ok
			||(pdllman->index->file_head < pdllman->index->num_wp)
		    ||(pdllman->index->file_wp < pdllman->index->file_head))
		 {
				fileerr = 1;
		 }
		 else
		 {
			 len = lfseek(fp, 0, SEEK_END);
			 len1 = (int)strlen(msgFileTail);
			 if((len != pdllman->index->file_size) || (pdllman->index->file_wp < len1))
			 {
					fileerr = 1;
			 }
			 else
			 {
				 fseek(fp, pdllman->index->file_wp - len1, SEEK_SET);
				 len = (int)fread(histtu_buf, 1, (DWORD)len1, fp);
				 if((len != len1) || (strstr(histtu_buf,msgFileTail) == 0))
				 {
					fileerr = 1;
				 }
			 }
		 }	
		 if(fileerr == 1)
		 {
			 fclose(fp);
			 remove(fname);
			 shellprintf(" 错误， 删文件 %s \n",fname);
			 fp = fopen(fname, "w");
			 if(fp==NULL) return;						 
			 pdllman->dll.create_histtudll_file(fp, &pdllman->dll, pdllman->index);
			 pdllman->index->num = 0; 
		 }
	 }

	 fseek(fp, pdllman->index->file_wp, SEEK_SET);

	 info.id = pdllman->id;
	 info.sec = pdllman->index->num+1; //从0开始，sec 从1开始
	 info.fp = fp;
	 memcpy(&info.clock, pclk, sizeof(struct VSysClock));
	 memset(histtu_buf,0,histtu_buf_len);	 
	 len = pdllman->dll.get_histtudll_data(&info, pbuf);

	 pdllman->index->num++;
	 pdllman->index->file_wp += len;
	 histtu_cfg_change = 1;
	 
	 fwrite(msgFileTail, 1, strlen(msgFileTail), fp);
	 len = lfseek(fp, 0, SEEK_CUR);
	 pdllman->index->file_size = len;


	 if ((pdllman->dll.type == DBhisttu_type_fixpt) || (pdllman->dll.type == DBhisttu_type_frz))
	 {
		sprintf(ftemp, "%02d", pdllman->index->num);//num 为 datarec  num
				
		fseek(fp, pdllman->index->num_wp, SEEK_SET);
		fwrite(ftemp, 1, 2, fp);
	 }
	 fflush(fp);
	 fsync(fileno(fp));
	 fclose(fp);
}


void histtu_Event_Write(void)
{
    int i,j,write_num,same;
	int ptr1,ptr2;
    int wptr, num,num1;

    for (;;)
    {
        wptr = histtu_wptr;
		num = (wptr >= histtu_rptr) ? (wptr-histtu_rptr):((DBHISTTU_SOCKET_MAX-histtu_rptr)+wptr);

		if (num == 0) break;
		
		ptr1 = (wptr-1)&(MAX_B2FSOCKET_NUM-1);
		num1 = num-1;
        for (i=0; i<num; i++, num1--, ptr1 = (ptr1-1)&(MAX_B2FSOCKET_NUM-1))
        {
			if (histtu_event_socket[ptr1].read_write != 2) continue;

            ptr2 = histtu_rptr;
			for(j=0; j<num1; j++, ptr2 = (ptr2+1)&(MAX_B2FSOCKET_NUM-1))
			{
				if (histtu_event_socket[ptr2].read_write != 2) continue;

				same = 1;
			    if (histtu_event_socket[ptr2].buf !=  histtu_event_socket[ptr1].buf)
					same = 0;
				else if (histtu_event_socket[ptr2].dllman!=  histtu_event_socket[ptr1].dllman)
					same = 0;

				if (same) 
				{
					histtu_event_socket[ptr2].buf = histtu_event_socket[ptr1].buf;
					histtu_event_socket[ptr1].read_write = 4;
				}								
			}
		}

		ptr1 = histtu_rptr;

		for (i=0; i<num; i++, ptr1 = (ptr1+1)&(DBHISTTU_SOCKET_MAX-1))
		{
			if (histtu_event_socket[ptr1].read_write != 2) continue;

            write_num = 0;
			histtu_event_ary[write_num] = (DWORD)(histtu_event_socket+ptr1);
			write_num++;
			
			ptr2 = (ptr1+1)&(DBHISTTU_SOCKET_MAX-1);
			for (j = (i+1); j<num; j++,ptr2 = (ptr2+1)&(DBHISTTU_SOCKET_MAX-1))
			{
			    if (histtu_event_socket[ptr2].read_write != 2) continue;
				if (histtu_event_socket[ptr2].dllman != histtu_event_socket[ptr1].dllman) continue;
				histtu_event_ary[write_num] = (DWORD)(histtu_event_socket+ptr2);
				write_num++;
			}
			
            histtu_Event_RamSort(histtu_event_socket[ptr1].dllman, write_num);
			histtu_Event_FileWrite(histtu_event_socket[ptr1].dllman, write_num);
			
			histtu_Event_RamFile(histtu_event_socket[ptr1].dllman);

		}	
		ptr1 = histtu_rptr;		
		for (i=0; i<num; i++, ptr1 = (ptr1+1)&(MAX_B2FSOCKET_NUM-1))
		{
			if (histtu_event_socket[ptr1].read_write == 4) 
				histtu_event_socket[ptr1].read_write = 0;
		}		
		histtu_rptr = (histtu_rptr+num)&(DBHISTTU_SOCKET_MAX-1);
    }
}
void histtu2fileDirCheck(void)
{
	char  path[4*MAXFILENAME];
	struct stat filestat;
	char *myPath2[]={"","/cfg","/HISTORY","/HISTORY/SOE","/HISTORY/CO","/HISTORY/EXV","/HISTORY/FIXPT",\
	"/HISTORY/FRZ","/HISTORY/FLOWREV","/HISTORY/ULOG","/HISTORY/VOLT_DAY","/HISTORY/VOLT_MON","/HISTORY/OL",\
	"/LINELOSS","/LINELOSS/FIXD","/LINELOSS/RAND","/LINELOSS/FRZD","/LINELOSS/SHARPD","/LINELOSS/MONTHD","/LINELOSS/EVENTD"};
	int i;

	for (i=1; i<NELEMENTS(myPath2); i++)
	{
		strcpy(path,SYS_PATH_ROOT2);
		strcat(path,myPath2[i]);
		if ((stat(path, &filestat) >= 0) && (S_ISDIR(filestat.st_mode)))
			continue;

		if (MakeDir(path)==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '%s' failed!", path);
		else  myprintf(SYS_ID, LOG_ATTR_INFO, "Auto create Dir '%s' success!", path);		
	} 
}

static int histtu_cfg_init(void)
{
	FILE *fp;
	WORD crc,crc_f;	
	struct VFileHead *filehead;
	struct VHisTtuRcd *histturcd;
	
	histtu_buf = (char*) malloc(histtu_buf_len);
	if(histtu_buf == NULL) return -1;
	memset(histtu_buf,0,histtu_buf_len);
	eventttubuf = (char*) malloc(eventttubuflen);
	if (eventttubuf == NULL) return-1;
	memset(eventttubuf, 0, 64*1024);
	
	filehead = (struct VFileHead*)histtu_buf;
	histturcd = (struct VHisTtuRcd*)(filehead+1);

	histtu_rcd_cfg =  (struct VHisTtuRcd*)malloc(sizeof(struct VHisTtuRcd)*DBhisttu_type_num);
	if (histtu_rcd_cfg == NULL) return -1;
	
	memset(histtu_rcd_cfg, 0, sizeof(struct VHisTtuRcd)*DBhisttu_type_num);

	histtu2fileDirCheck();

	sprintf(histtu_rcdfile,"%s/cfg/histturcd.cfg",SYS_PATH_ROOT2);
	
	fp = fopen(histtu_rcdfile, "r+b");
	
	if (fp == NULL) 
	{
	    fp = fopen(histtu_rcdfile, "w+");
		if(fp == NULL) return -1;
		
		filehead->wVersion = 101;
		filehead->wAttr = 1;
		filehead->nLength = sizeof(struct VFileHead)+sizeof(struct VHisTtuRcd)*DBhisttu_type_num+2;
		crc = GetParaCrc16((BYTE *)histtu_rcd_cfg, sizeof(struct VHisTtuRcd)*DBhisttu_type_num);

		fwrite((BYTE*)filehead, 1, sizeof(struct VFileHead), fp);
		fwrite(histtu_rcd_cfg, 1, sizeof(struct VHisTtuRcd)*DBhisttu_type_num, fp);

		fwrite(&crc, 1, 2, fp);
		fflush(fp);
		fsync(fileno(fp));
		fclose(fp);
		return 0;
	}
	
	fread((BYTE*)filehead, 1, sizeof(struct VFileHead), fp);
	fread((BYTE*)histturcd, 1, sizeof(struct VHisTtuRcd)*DBhisttu_type_num, fp);
	fread((BYTE*)&crc_f, 1, 2, fp);
	
	fclose(fp);
	
	crc = GetParaCrc16((BYTE*)histturcd, sizeof(struct VHisTtuRcd)*DBhisttu_type_num);

	if (crc != crc_f)
		return 0;

	memcpy(histtu_rcd_cfg, histturcd, sizeof(struct VHisTtuRcd)*DBhisttu_type_num);
	return 0;
}

static struct VHisTtuDllMan *histtu_regist(WORD id, const struct VHisTtuDll *pdll, struct VHisTtuDllMan **pdllman, struct VHisTtuRcd *prcdcfg)
{
	char fname[4*MAXFILENAME]={0};
	struct VHisTtuDllMan *p, *q, **s;
	struct VSysClock clk;
	
	q = (struct VHisTtuDllMan *)malloc(sizeof(struct VHisTtuDllMan));
	if (q == NULL) return NULL;	

	q->index = prcdcfg;
	s = pdllman;	
	q->id = id;          
	memcpy(&q->dll, pdll, sizeof(struct VHisTtuDll));
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
	histtuDll_FileNameGet(&q->dll, &clk, fname);
	histtuDll_Scan(fname, &q->dll, prcdcfg);

    return(q);
}

//负责传递 写任务
void histtu2file_write(BYTE *pBuf, WORD wID, int flag)
{
     int k;
     struct VHisTtuSocket hissocket;

     hissocket.read_write = 2;
	
	 if((dbhisttu_init == 0) &&((flag == SYSEV_FLAG_ULOG)||(flag == SYSEV_FLAG_SSOE)||(flag == SYSEV_FLAG_DSOE)))
	 	{
			histtu_event_sockets_temp[histtu_wptr_temp].buf = pBuf;
			histtu_event_sockets_temp[histtu_wptr_temp].read_write = flag; 
			histtu_wptr_temp++;
			return;
	 	}
		
	 hissocket.dllman = histtuDll_Get(wID, flag, histtu_event);
	 if (hissocket.dllman == NULL)
	 {
		return;
	 }
	 hissocket.buf = pBuf;
 
	 k = 0;
	 while ((histtu_event_socket[histtu_wptr].read_write!=0) && (k<DBHISTTU_SOCKET_MAX))
     {
		histtu_wptr++;
		histtu_wptr &= (DBHISTTU_SOCKET_MAX-1);
		k++;
	 }

	if (k == DBHISTTU_SOCKET_MAX) 
		return;

	memcpy(&histtu_event_socket[histtu_wptr], &hissocket, sizeof(struct VHisTtuSocket));
	histtu_event_socket[histtu_wptr].read_write = 2;
	histtu_wptr++;
	histtu_wptr &= (MAX_B2FSOCKET_NUM-1);
	
	evSend(HIS_ID, EV_UFLAG);
	return;
}

void histtu_cfg_write(void)
{
    FILE *fp;
	int len;
	WORD crc;
    crc = GetParaCrc16((BYTE *)histtu_rcd_cfg, sizeof(struct VHisTtuRcd)*DBhisttu_type_num);

	fp = fopen(histtu_rcdfile, "r+");
	if (fp == NULL)
		return;
    
	len = sizeof(struct VFileHead);
	fseek(fp, len, SEEK_SET);
	fwrite((BYTE*)histtu_rcd_cfg,  1, sizeof(struct VHisTtuRcd)*DBhisttu_type_num, fp);
	fwrite((BYTE*)&crc, 1, 2, fp);
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
}

void histtu_Data_Write(void)
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
	   histtuDll_day(&sysclock);
	   histtuDll_ResetExv();
	}
	if ((sysclock.wYear != oldclock.wYear) || (sysclock.byMonth != oldclock.byMonth))
	{
	   histtuDll_month(&sysclock);
	}
	if (sysclock.byMinute != oldclock.byMinute)
	{
		histtuDll_hour(&sysclock);
	}
	memcpy(&oldclock, &sysclock, sizeof(struct VSysClock));

}

void histtu2file(void)
{
	DWORD events;	
	int no;

	evSend(HIS_ID, EV_UFLAG);   //确保任务运行之前的写请求执行
	tmEvEvery(HIS_ID, SECTOTICK(30), EV_TM1);   /*30s*/

	no = -1;

	for (;;)
	{
		evReceive(HIS_ID, EV_UFLAG | EV_MSG | EV_TM1|EV_TM2, &events);

		if(events&EV_UFLAG)
		{
			if (no == -1)
				no = tmEvAfter(HIS_ID, 200, EV_TM2);
		}

		if (events&EV_TM2)
		{
			histtu_Event_Write();
			tmDelete(HIS_ID, no);
			no = -1;
		}

		if (events&EV_TM1)
		{
			histtu_Data_Write();
		}

		if (histtu_cfg_change)
		{
			histtu_cfg_write();
			histtu_cfg_change = 0;
		}
	}
}

void his_ttu_init(void)
{
    WORD i;
	struct VVirtualEqp *pVEqp;
	struct VHisTtuDll dll;
	
    histtu_event = NULL;
	histtu_hour = NULL;
	histtu_day = NULL;
	histtu_month = NULL;

	if (histtu_cfg_init() == -1) return;
	
	for (i=0; i<*g_Sys.Eqp.pwNum; i++)
	{
		if ((pVEqp = g_Sys.Eqp.pInfo[i].pVEqpInfo) != NULL)
		{
		    if (pVEqp->Cfg.dwFlag & 0x20) // D5
		    {
					dll.type = DBhisttu_type_soe; // 预留ttu的写soe接口，后续接上
					dll.max = DBHISTTU_SOE_MAX;
					dll.date = 0;
					dll.md = 0;
					dll.create_histtudll_file = histtuDll_RecCreate;
					dll.get_histtudll_data = histtuDll_SoeGet;
					histtu_regist(i, &dll, &histtu_event, &histtu_rcd_cfg[DBhisttu_type_soe]);// his_event his_day 是数组，仅仅只有next   (注册一个后带一个)

					dll.type = DBhisttu_type_co;  // 预留ttu的写co接口，后续接上
					dll.max = DBHISTTU_CO_MAX;
					dll.date = 0;
					dll.md = 0;
					dll.create_histtudll_file = histtuDll_RecCreate;
					dll.get_histtudll_data = histtudll_CoGet;
					histtu_regist(i, &dll, &histtu_event, &histtu_rcd_cfg[DBhisttu_type_co]);
					
					dll.type = DBhisttu_type_hl;  //日重载
					dll.max = DBHISTTU_LOG_MAX;
					dll.date = 0;
					dll.md = 0;
					dll.create_histtudll_file = histtuDll_RecCreate;
					dll.get_histtudll_data = histtudll_hlGet;
					histtu_regist(i, &dll, &histtu_event, &histtu_rcd_cfg[DBhisttu_type_hl]);
					
					dll.type = DBhisttu_type_ol;  //日过载
					dll.max = DBHISTTU_LOG_MAX;
					dll.date = 0;
					dll.md = 0;
					dll.create_histtudll_file = histtuDll_RecCreate;
					dll.get_histtudll_data = histtudll_olGet;
					histtu_regist(i, &dll, &histtu_event, &histtu_rcd_cfg[DBhisttu_type_ol]);

					dll.type = DBhisttu_type_exv;   //极值 exv
					dll.date = DBHISTTU_DAY_MAX;
					dll.max = 0;
					dll.md = 1;
					dll.create_histtudll_file = histtuDll_AttrCreate;
					dll.get_histtudll_data = histtuDll_ExvGet;
					histtu_regist(i, &dll, &histtu_day, &histtu_rcd_cfg[DBhisttu_type_exv]);
					
					dll.type = DBhisttu_type_voltd;   //电压日合格率
					dll.date = DBHISTTU_DAY_MAX;
					dll.max = 0;
					dll.md = 1;
					dll.create_histtudll_file = histtuDll_AttrCreate;
					dll.get_histtudll_data = histtuDll_VoltdGet;
					histtu_regist(i, &dll, &histtu_day, &histtu_rcd_cfg[DBhisttu_type_voltd]);
					
					dll.type = DBhisttu_type_volttd;   //电压日越限
					dll.date = DBHISTTU_DAY_MAX;
					dll.max = 0;
					dll.md = 1;
					dll.create_histtudll_file = histtuDll_AttrCreate;
					dll.get_histtudll_data = histtuDll_VolttdGet;
					histtu_regist(i, &dll, &histtu_day, &histtu_rcd_cfg[DBhisttu_type_volttd]);
					
					dll.type = DBhisttu_type_voltm;   //电压月合格率
					dll.date = DBHISTTU_DAY_MAX;
					dll.max = 0;
					dll.md = 1;
					dll.create_histtudll_file = histtuDll_AttrCreate;
					dll.get_histtudll_data = histtuDll_VoltmGet;
					histtu_regist(i, &dll, &histtu_month, &histtu_rcd_cfg[DBhisttu_type_voltm]);
					
					dll.type = DBhisttu_type_volttm;   //电压月越限
					dll.date = DBHISTTU_DAY_MAX;
					dll.max = 0;
					dll.md = 1;
					dll.create_histtudll_file = histtuDll_AttrCreate;
					dll.get_histtudll_data = histtuDll_VolttmGet;
					histtu_regist(i, &dll, &histtu_month, &histtu_rcd_cfg[DBhisttu_type_volttm]);

					dll.type = DBhisttu_type_fixpt;  //定点 fixp
					dll.date = DBHISTTU_DAY_MAX;
					dll.max = 0;
					dll.md = DBHISTTU_MIN_INTERVAL;
					dll.create_histtudll_file = histtuDll_AttrCreate;
					dll.get_histtudll_data = histtuDll_FixptGet;
					histtu_regist(i, &dll, &histtu_hour, &histtu_rcd_cfg[DBhisttu_type_fixpt]);

					dll.type = DBhisttu_type_frz;   // 日冻结  frz
					dll.date = DBHISTTU_MIN_MAX;
					dll.max = 0;
					dll.md = DBHISTTU_MIN_INTERVAL;
					dll.create_histtudll_file = histtuDll_FrzCreate;
					dll.get_histtudll_data = histtuDll_FrzGet;
					histtu_regist(i, &dll, &histtu_hour, &histtu_rcd_cfg[DBhisttu_type_frz]);

					dll.type = DBhisttu_type_frz;   // 日冻结 frz
					dll.date = DBHISTTU_DAY_MAX;
					dll.max = 0;
					dll.md = 1;
					dll.create_histtudll_file = histtuDll_FrzCreate;
					dll.get_histtudll_data = histtuDll_FrzDayGet;
					histtu_regist(i, &dll, &histtu_day, &histtu_rcd_cfg[DBhisttu_type_frz]);

					dll.type = DBhisttu_type_flowrev;
					dll.max = DBHISTTU_FLOW_MAX;
					dll.date = 0;
					dll.md = 0;
					dll.create_histtudll_file = histtuDll_RecCreate;
					dll.get_histtudll_data = histtudll_FlowGet;
					histtu_regist(i, &dll, &histtu_event, &histtu_rcd_cfg[DBhisttu_type_flowrev]);

					dll.type = DBhisttu_type_ulog;
					dll.max = DBHISTTU_LOG_MAX;
					dll.date = 0;
					dll.md = 0;
					dll.create_histtudll_file = histtuDll_RecCreate;
					dll.get_histtudll_data = histtudll_UlogGet;
					histtu_regist(i, &dll, &histtu_event, &histtu_rcd_cfg[DBhisttu_type_ulog]);	
		    }
		}
	}	
	
	dbhisttu_init = 1;

	while(histtu_wptr_temp > 0)
	{
		histtu2file_write(histtu_event_sockets_temp[histtu_wptr_temp-1].buf,0,histtu_event_sockets_temp[histtu_wptr_temp-1].read_write);
		histtu_wptr_temp--;
	}

}
#endif
