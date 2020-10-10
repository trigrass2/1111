/*------------------------------------------------------------------------
 Module:       	db_frz.c
 Author:        solar
 Project:       
 State:			
 Creation Date:	2009-02-25
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#include <syscfg.h>
#include <stdio.h>
#include "sys.h"
#include "File.h"
#include  "dbfrz.h"
#include "fileext.h"

#ifdef INCLUDE_FRZ

extern WORD atomicReadTEYC_L(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBYC_L *buf,BOOL asSend);
extern WORD atomicReadTEDD(struct VTrueEqp *pTEqp, WORD wNum, WORD wBufLen,struct VDBDD *buf,BOOL asSend);

static DWORD frzSemId;
static BYTE FFRecBufTemp[FF_MAX_REC_LEN];

static struct VFunDllMan *pFunDllMan = NULL;

static struct VFFDllMan *pFFDll_Hour_Man;
static struct VFFDllMan *pFFDll_Day_Man;
static struct VFFDllMan *pFFDll_Month_Man;

int FF_MD_DEFAULT;    /*缺省冻结密度min*/

static int FF_HOUR_CAPACITY;      /*最大存储点个数*/ 
static int FF_DAY_CAPACITY;       /*最大存储点个数*/
static int FF_MONTH_CAPACITY;     /*最大存储点个数*/
static char * tabname[] =
{
	"yc",
	"dd"
};	

void FunDll_Exe(int type, struct VSysClock *pclock)
{
	struct VFunDllMan *p;

	p = pFunDllMan;

	while (p != NULL)
	{
		if (p->dll.type == type) p->dll.fun(pclock, p->dll.argc);

		p = p->next;
	}	
}

int FunDll_Regist(VFunDll *pdll)
{
	struct VFunDllMan *p, *q;

	q = (struct VFunDllMan *)malloc(sizeof(struct VFunDllMan));
	if (q == NULL) return ERROR; 

	memcpy(&q->dll, pdll, sizeof(VFunDll));		
	q->next = NULL; 
	
	if (pFunDllMan == NULL) 
	{
		pFunDllMan = q;
	}
	else
	{
		p = pFunDllMan;
		while (p->next != NULL) p = p->next;			
	
		p->next = q;
	}	

	return OK;
}

int FFIndex_Search(VFFIndex *index, DWORD minute, int *locate)
{
    int low, high, mid, midp;

    if (index->num < index->max) 
	{
		low = 0;
		high = index->wp -1;
    }	
	else 
	{
		low = index->wp;
		high = index->max+ index->wp -1;
	}	

	while (low <= high)
	{
		mid = (low + high)/2;
		
		midp = mid%index->max;

		if (minute < index->info[midp].minute) high = mid-1;
		else if (minute >index->info[midp].minute) low = mid+1;
		else 
		{
            *locate = midp;
			return OK;
		}	
	}

	*locate = low%index->max;
	return ERROR;	
}

void FFIndex_Add(VFFIndex *index, DWORD minute, DWORD *offset)
{
    int end, wp;

	if (index->wp == 0) end = index->max-1;
	else end= index->wp-1;

	if ((index->num == 0) || (minute > index->info[end].minute))
	{
		wp = index->wp;
		index->wp = (index->wp+1)%index->max;
		if (index->num < index->max) index->num++;
	}	
	else if (minute == index->info[end].minute) 
	{
		wp = end;
	}			
	else
	{
		FFIndex_Search(index, minute, &wp);        
	}

    *offset = sizeof(VFFHead)+wp*(index->len+sizeof(DWORD))+sizeof(DWORD);
	index->info[wp].minute = minute;
	index->info[wp].offset = *offset;
}

void FFDll_Scan(char *fname, VFFIndex *index)
{
	FILE *fp;
	int num;
	VFFHead head; 
	int i;
	DWORD offset, minute;

	fp = fopen(fname, "r+b");
	if (fp == NULL) return;

	num = fread(&head, 1, sizeof(VFFHead), fp);
	if(num != sizeof(VFFHead))  goto Err;

    if (index->len != head.rec_len) goto Err;
	if (index->max < head.rec_max) goto Err;

	offset = sizeof(VFFHead);
	for (i=0; i<head.rec_num; i++)
	{
		fseek(fp, offset, SEEK_SET);

		num = fread(&minute, 1, sizeof(minute), fp);
		if (num != sizeof(minute)) break;

        index->info[i].minute = minute;
		index->info[i].offset = offset+sizeof(DWORD);
 
		offset += (head.rec_len+sizeof(DWORD));
	}

	if ((i < head.rec_num) || (index->max > head.rec_max))
	{
        head.rec_num = i;
		head.rec_max = index->max;
		fseek(fp, offset, SEEK_SET);
		fwrite(&head, 1, sizeof(head), fp);			
	}
    fclose(fp);

    index->num = head.rec_num;
    index->wp = 0;
    for (i=1; i<index->max; i++)
	{
		if (index->info[i].minute < index->info[i-1].minute)
		{
			index->wp = i;
			break;
		}
    }		
	
	return;

Err:
	fclose(fp);
	remove(fname);	
	myprintf(SYS_ID, LOG_ATTR_INFO, "%s error and been inited!", fname);
}

void FFDll_NameGet(WORD id, VFFDll *pdll, char *fname)
{
	char *str;

	if (pdll->date == FF_DATE_HOUR) 
		str = FF_HOUR_DLL;
	else if (pdll->date == FF_DATE_DAY) 
		str = FF_DAY_DLL;
	else if (pdll->date == FF_DATE_MONTH) 
		str = FF_MONTH_DLL;
	else
		str = "";

    if (pdll->type == FF_TYPE_VALUE)
		sprintf(fname, str, SYS_PATH_ROOT2, g_Sys.Eqp.pInfo[id].sCfgName, tabname[pdll->tab], "");
	else if (pdll->type == FF_TYPE_MAXMIN)
		sprintf(fname, str, SYS_PATH_ROOT2, g_Sys.Eqp.pInfo[id].sCfgName, tabname[pdll->tab], "_mm");
	else
		strcpy(fname, "");
}

struct VFFDllMan *FFDll_Search(WORD id, DWORD tab, int type, struct VFFDllMan *start)
{
    struct VFFDllMan *p;

    p = start;
	while (p != NULL)
	{
		if ((p->id == id) && (p->dll.type == type) && (p->dll.tab == tab)) return(p);
		p = p->next;
	}

	return NULL;
}

struct VFFDllMan *FFDll_Point_Regist(WORD id, VFFDll *pdll)
{
	char fname[FF_NAME_LEN];
	struct VFFDllMan *p, *q, **s;
	int capacity;

	if (pdll->len > FF_MAX_REC_LEN) return NULL;

	q = (struct VFFDllMan *)malloc(sizeof(struct VFFDllMan));
	if (q == NULL) return NULL;	

	if (pdll->date == FF_DATE_HOUR) 
	{
		if (pdll->md == 0) return NULL;
		
		if (pdll->capacity == 0) capacity = FF_HOUR_CAPACITY;	
	#if (TYPE_CPU == CPU_CF52259)
	    else if(pdll->capacity > FF_HOUR_CAPACITY) capacity = FF_HOUR_CAPACITY;
	#endif
		else capacity = pdll->capacity;
		s = &pFFDll_Hour_Man;
	}	
	else if ((pdll->date == FF_DATE_DAY)) 
	{
        	pdll->md = 1;
		if (pdll->capacity == 0) capacity  = FF_DAY_CAPACITY;
		else capacity = pdll->capacity;
		s = &pFFDll_Day_Man;
	}	
	else if (pdll->date == FF_DATE_MONTH) 
	{        
        	pdll->md = 1;
		if (pdll->capacity == 0) capacity  = FF_MONTH_CAPACITY; 
		else capacity = pdll->capacity;
		s = &pFFDll_Month_Man;
	}	
	else
		return NULL;			
	    
	q->index.max = capacity;
    	q->index.info = (VFFIndexInfo *)malloc(q->index.max * sizeof(VFFIndexInfo));
	if (q->index.info == NULL) return NULL;	
	memset(q->index.info, 0, q->index.max * sizeof(VFFIndexInfo));
	q->index.wp = 0;
	q->index.len = pdll->len;		
	q->index.num = 0;		
	q->id = id;          
	memcpy(&q->dll, pdll, sizeof(VFFDll));
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

	FFDll_NameGet(id, &q->dll, fname);
	FFDll_Scan(fname, &q->index);

    return(q);
}

int FFDll_MdChange(WORD id, VFFDll *pdll)
{
	struct VFFDllMan *p;

	p = FFDll_Search(id, pdll->tab, pdll->type, pFFDll_Hour_Man);
	if (p == NULL) return ERROR;
	
	p->dll.md = pdll->md;
	
	return OK;
}

void FFDll_Write(WORD id, DWORD minute, struct VFFDllMan *dllMan)
{
	FILE *fp; 
	int num; 
	VFFHead head;
	DWORD offset;
	char fname[FF_NAME_LEN];

	smMTake(frzSemId);
	DPRINT("<开始冻结数据时间:  %d\n",minute);
	FFDll_NameGet(id, &dllMan->dll, fname);

	fp = fopen(fname, "r+b");
	if(fp == NULL)             
	{	
		fp = fopen(fname, "wb");	
		if(fp==NULL) goto Done;
		memset(&head, 0, sizeof(head));		
		head.rec_len = dllMan->index.len;
		head.rec_max = dllMan->index.max;
		dllMan->index.wp = 0;
		dllMan->index.num = 0;
	}
	else
	{
		num = fread(&head, 1, sizeof(VFFHead), fp);
		if(num != sizeof(VFFHead)) goto Done;
	}

    dllMan->dll.get_ffdll_data(id, FFRecBufTemp);

    FFIndex_Add(&dllMan->index, minute, &offset);

    fseek(fp, offset-sizeof(DWORD), SEEK_SET);
	fwrite(&minute, 1, sizeof(DWORD), fp);
	fwrite(FFRecBufTemp, 1, dllMan->index.len, fp);

	head.minute = minute;
	head.rec_num =  dllMan->index.num;
	head.rec_wp = dllMan->index.wp;

    	fseek(fp,0,SEEK_SET);
	fwrite(&head, 1, sizeof(head), fp);
	DPRINT("----结束冻结数据时间:  %d>\n",minute);
Done:
	if (fp != NULL) fclose(fp);
	smMGive(frzSemId);	
}

int FFDll_Freeze(DWORD minute,  struct VFFDllMan *pDllMan)
{  
	struct VFFDllMan *p;
    
	for (p = pDllMan; p!=NULL; p=p->next)
	{		
		if (p->dll.md == 0) continue;

		if (pDllMan == pFFDll_Hour_Man)
		{
			if ((minute%p->dll.md) != 0) continue;
		}  

		FFDll_Write(p->id, minute, p);		
	}
	
	 return OK;    
}

int FFDll_HourFreeze(DWORD minute)
{
	return (FFDll_Freeze(minute, pFFDll_Hour_Man));
}

int FFDll_DayFreeze(struct VSysClock *psys_clock)
{
	struct VSysClock tmp_clock;
	DWORD minute;

	memcpy(&tmp_clock, psys_clock, sizeof(tmp_clock));	
	tmp_clock.byHour = 0;
	tmp_clock.byMinute = 0;

	minute = CalendarClock(&tmp_clock);
	minute -= 24*60;	/*last day*/

	return(FFDll_Freeze(minute, pFFDll_Day_Man));
}

int FFDll_MonthFreeze(struct VSysClock *pclock)
{
	struct VSysClock tmp_clock;
	DWORD minute;
	
	memset(&tmp_clock, 0, sizeof(tmp_clock));
		
	if (pclock->byMonth == 1)                       /*last month*/
	{
		tmp_clock.wYear = pclock->wYear-1;
		tmp_clock.byMonth = 12;
	}
	else
	{
		tmp_clock.wYear = pclock->wYear;
		tmp_clock.byMonth = pclock->byMonth - 1;
	}	
	tmp_clock.byDay = 1;
	tmp_clock.byHour = 0;
	tmp_clock.byMinute = 0;

	minute = CalendarClock(&tmp_clock);

	return(FFDll_Freeze(minute, pFFDll_Month_Man));
}

/*------------------------------------------------------------------------
 Procedure:     lctuFFDll_CurveMiDu ID:1
 Purpose:       曲线密度换算[分钟],规整td到向后最近的点
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void FFDll_CurveMiDu(DWORD *ptd_min, int min)
{
	DWORD unit;

	unit = (*ptd_min+min-1)/min;
	*ptd_min = unit*min;
}

int FFDll_Read(WORD id,  DWORD tab, int type, struct VFFDllMan *pDllMan, struct VSysClock *td, BYTE *pbuf, WORD buf_len, DWORD rec_offset)
{
	FILE *fp; 
	int num;
	char fname[FF_NAME_LEN]; 
	VFFIndex *index;
	DWORD minute;
	int locate, ret;
	struct VFFDllMan *p;

	memset(pbuf, 0, buf_len);
			
    	smMTake(frzSemId);

	p = FFDll_Search(id, tab, type, pDllMan);
	if (p == NULL) goto Err;

	FFDll_NameGet(id, &p->dll, fname);
	
	fp = fopen(fname, "rb");
	if(fp == NULL) goto Err;

	index = &(p->index);

    	minute = CalendarClock(td);

	if (FFIndex_Search(index, minute, &locate) == ERROR) goto Err;

	ret = fseek(fp, index->info[locate].offset+rec_offset, SEEK_SET);
	if (ret) goto Err;
	
	num = fread(pbuf, 1, buf_len, fp);
	if(num != buf_len) goto Err;
	
	fclose(fp);
	smMGive(frzSemId);	
	return OK;

Err:
	if (fp != NULL) fclose(fp);
	smMGive(frzSemId);	
	return ERROR;	
}

void FFDll_CurveNoPointdata(BYTE *pbuf, int i, WORD size)
{
	int j;

	for (j=0; j<size; j++)
		pbuf[size*i+j] = 0xEE;
}

/*------------------------------------------------------------------------
 Procedure:     lctuFFDll_CurveRead ID:1
 Purpose:       通用曲线读取函数
 Input:          				
 				td: 读取开始时标
 				pbuf: 结果存储缓冲区
 				min: 冻结密度的时间间隔
 				rec_offset:所读取数据在冻结数据块内的偏移
 				size:冻结数据单元项大小 (2字节或4字节) 				
 				pn: 读取的点数
 Input:         
 Output:		
 Errors:		
------------------------------------------------------------------------*/
int FFDll_CurveRead(WORD id, DWORD tab, int type, struct VSysClock *td, BYTE *pbuf, int min, DWORD rec_offset, WORD size, WORD num)
{
	char fname[FF_NAME_LEN]; 
	DWORD tm_min, minute; 	         /*记录的时标*/   /*要搜寻的时标*/
	FILE *fp; 
	BYTE i, j; 				
	int have, locate, ret;
    	VFFIndex *index;
	struct VFFDllMan *p;
		
	memset(pbuf, 0, num*size);
	
   	 smMTake(frzSemId);

    	if (min <= 0) goto Err;

	p = FFDll_Search(id, tab, type, pFFDll_Hour_Man);
	if (p == NULL) goto Err;

	FFDll_NameGet(id, &p->dll, fname);
	
	fp = fopen(fname, "rb");
	if(fp == NULL) goto Err;
	
	index = &(p->index);
    	minute = CalendarClock(td);	
	FFDll_CurveMiDu(&minute, min);
	DPRINT("MINUTE = %d \n",minute);
	i=0;
	have = 1;
	if (FFIndex_Search(index, minute, &locate) == ERROR)
	{
		tm_min = index->info[locate].minute;
		if (minute > tm_min) have = 0;
	}
	while(have)
	{
		tm_min = index->info[locate].minute;
		DPRINT("tm_min = %d \n",tm_min);
		if(minute == tm_min)
		{
			ret = fseek(fp, index->info[locate].offset+rec_offset, SEEK_SET);
			if (ret) 
				FFDll_CurveNoPointdata(pbuf, i, size);
			else
			{
				ret = fread(&pbuf[size*i], 1, size, fp);
				if(ret != size)
				{
					FFDll_CurveNoPointdata(pbuf, i, size);
				}
			}	
			minute += min;	
			i++; 
			
			locate = (locate+1)%index->max;			
			if(locate == index->wp) break;
		}
		else if(minute < tm_min)  /*未存点*/
		{ 
			FFDll_CurveNoPointdata(pbuf, i, size);
			minute += min; 
			i++; 
		}
		else					/*多存点*/
		{ 
			locate = (locate+1)%index->max;			
			if(locate == index->wp) break;
		}
		
		if(i==num) break;
	}

	for (j=i; j<num; j++)
	{
		FFDll_CurveNoPointdata(pbuf, j, size);
	}
			
	fclose(fp);
	smMGive(frzSemId);	
	return num;

Err:	
	if (fp != NULL) fclose(fp);
	smMGive(frzSemId);	
	return 0;		
}

/*------------------------------------------------------------------------
 Procedure:     FFDayRead ID:1
 Purpose:       读日冻结
 Input:        
 				td: 读取日
 				pbuf: 结果存储缓冲区, 结构的起始位置
 				buf_len: 缓冲区长度, 具体数据类型的结构大小
 				rec_offset:所读取数据在冻结数据块中的偏移
 Output:		
 Errors:
------------------------------------------------------------------------*/
int FFDll_DayRead(WORD id, DWORD tab, int type, struct VSysClock *td, BYTE *pbuf, WORD buf_len, DWORD rec_offset)
{
	td->byHour = 0;
	td->byMinute = 0;
	return(FFDll_Read(id, tab, type, pFFDll_Day_Man, td, pbuf, buf_len, rec_offset));
}

/*------------------------------------------------------------------------
 Procedure:     FFDll_MonthRead ID:1
 Purpose:       读月冻结
 Input:         
 				td: 读取月
 				pbuf: 结果存储缓冲区, 结构的起始位置
 				buf_len: 缓冲区长度, 具体数据类型的结构大小
 				rec_offset:所读取数据在冻结数据块中的偏移
 Output:		
 Errors:		
------------------------------------------------------------------------*/
int FFDll_MonthRead(WORD id, DWORD tab, int type, struct VSysClock *td, BYTE *pbuf, WORD buf_len, DWORD rec_offset)
{
    td->byDay = 1;
	td->byHour = 0;
	td->byMinute = 0;
	return(FFDll_Read(id, tab, type, pFFDll_Month_Man, td, pbuf, buf_len, rec_offset));
}

/*------------------------------------------------------------------------
 Procedure:     lctuFXX_MiDu2Min ID:1
 Purpose:       密度对应的读取间隔
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void FF_MiDu2Min(BYTE m, int *pmin)
{
	if(m == FF_MIDU_1)  *pmin = 15;	
	else if(m == FF_MIDU_2)   *pmin = 30;
	else if(m == FF_MIDU_3)   *pmin = 60;
	else *pmin = FF_MD_DEFAULT;
}

/*------------------------------------------------------------------------
 Procedure:     lctuC1FXX_MiDu2Num ID:1
 Purpose:       小时冻结:密度对应的读取点数
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void FF_MiDu2Num(BYTE m, BYTE *pnum)
{
	if(m == FF_MIDU_1) *pnum = 4;
	if(m == FF_MIDU_2) *pnum = 2;	
	if(m == FF_MIDU_3) *pnum = 1;	
	else *pnum = 4;
}

/*------------------------------------------------------------------------
 Procedure:     lctuC1FXX_TdHandle ID:1
 Purpose:       小时冻结:取当前时钟,规整时钟
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
int FF_TdHandle(BYTE *phour, struct VSysClock *pclock)
{
	struct VCalClock time; DWORD unit;

  	GetSysClock((void *)&time, CALCLOCK);
	unit = (time.dwMinute-60)/60;	/*找前一整点*/
	time.dwMinute = unit*60;
	CalClockTo(&time, pclock);
	*phour = pclock->byHour;

	return OK;
}

int GetYcHFrzOffset(WORD *pwEqpID, WORD wNo, DWORD *pdwOffset, BOOL asSend)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	WORD no, frzno, teid, offset;
	int maxno;

	if (*pwEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;
	
	if ((pTEqp = g_Sys.Eqp.pInfo[*pwEqpID].pTEqpInfo) != NULL)
	{
		if (asSend)
		{
			maxno=pTEqp->YCSendCfg.wNum-1;
			if (wNo > maxno) return ERROR;
			no = pTEqp->YCSendCfg.pwIndex[wNo];			
		}	
		else
		{
			maxno=pTEqp->Cfg.wYCNum-1;
			if (wNo > maxno) return ERROR;
			no = wNo;			
		}	
		
		frzno = pTEqp->pYCCfg[no].wHourFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(VYC);    
	}
	else if ((pVEqp = g_Sys.Eqp.pInfo[*pwEqpID].pVEqpInfo) != NULL)
	{
		if (wNo >= pVEqp->Cfg.wYCNum) return ERROR;
		
		teid=pVEqp->pYC[wNo].wTEID;
		offset=pVEqp->pYC[wNo].wOffset;	   
						
		if ((pTEqp=g_Sys.Eqp.pInfo[teid].pTEqpInfo) == NULL) return ERROR;
		if (g_Sys.Eqp.pInfo[teid].pTEqpInfo->pYC == NULL) return ERROR;

		frzno = pTEqp->pYCCfg[offset].wHourFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(VYC);    
		*pwEqpID = teid;		
	}		
	else 
		return ERROR;

	return OK;
}

int GetYcDFrzOffset(WORD *pwEqpID, WORD wNo, DWORD *pdwOffset, BOOL asSend)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	WORD no, frzno, teid, offset;
	int maxno;

	if (*pwEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;
	
	if ((pTEqp = g_Sys.Eqp.pInfo[*pwEqpID].pTEqpInfo) != NULL)
	{
		if (asSend)
		{
			maxno=pTEqp->YCSendCfg.wNum-1;
			if (wNo > maxno) return ERROR;
			no = pTEqp->YCSendCfg.pwIndex[wNo];			
		}	
		else
		{
			maxno=pTEqp->Cfg.wYCNum-1;
			if (wNo > maxno) return ERROR;
			no = wNo;			
		}	
		
		frzno = pTEqp->pYCCfg[no].wDayFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(VYC);    
	}
	else if ((pVEqp = g_Sys.Eqp.pInfo[*pwEqpID].pVEqpInfo) != NULL)
	{
		if (wNo >= pVEqp->Cfg.wYCNum) return ERROR;
		
		teid=pVEqp->pYC[wNo].wTEID;
		offset=pVEqp->pYC[wNo].wOffset;	   
						
		if ((pTEqp=g_Sys.Eqp.pInfo[teid].pTEqpInfo) == NULL) return ERROR;
		if (g_Sys.Eqp.pInfo[teid].pTEqpInfo->pYC == NULL) return ERROR;

		frzno = pTEqp->pYCCfg[offset].wDayFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(VYC);    
		*pwEqpID = teid;		
	}		
	else 
		return ERROR;

	return OK;
}

int GetYcMFrzOffset(WORD *pwEqpID, WORD wNo, DWORD *pdwOffset, BOOL asSend)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	WORD no, frzno, teid, offset;
	int maxno;

	if (*pwEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;
	
	if ((pTEqp = g_Sys.Eqp.pInfo[*pwEqpID].pTEqpInfo) != NULL)
	{
		if (asSend)
		{
			maxno=pTEqp->YCSendCfg.wNum-1;
			if (wNo > maxno) return ERROR;
			no = pTEqp->YCSendCfg.pwIndex[wNo];			
		}	
		else
		{
			maxno=pTEqp->Cfg.wYCNum-1;
			if (wNo > maxno) return ERROR;
			no = wNo;			
		}	
		
		frzno = pTEqp->pYCCfg[no].wMonthFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(VYC);    
	}
	else if ((pVEqp = g_Sys.Eqp.pInfo[*pwEqpID].pVEqpInfo) != NULL)
	{
		if (wNo >= pVEqp->Cfg.wYCNum) return ERROR;
		
		teid=pVEqp->pYC[wNo].wTEID;
		offset=pVEqp->pYC[wNo].wOffset;	   
						
		if ((pTEqp=g_Sys.Eqp.pInfo[teid].pTEqpInfo) == NULL) return ERROR;
		if (g_Sys.Eqp.pInfo[teid].pTEqpInfo->pYC == NULL) return ERROR;

		frzno = pTEqp->pYCCfg[offset].wMonthFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(VYC);    
		*pwEqpID = teid;		
	}		
	else 
		return ERROR;

	return OK;
}

int GetYcMMDFrzOffset(WORD *pwEqpID, WORD wNo, DWORD *pdwOffset, BOOL asSend)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	WORD no, frzno, teid, offset;
	int maxno;

	if (*pwEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;
	
	if ((pTEqp = g_Sys.Eqp.pInfo[*pwEqpID].pTEqpInfo) != NULL)
	{
		if (asSend)
		{
			maxno=pTEqp->YCSendCfg.wNum-1;
			if (wNo > maxno) return ERROR;
			no = pTEqp->YCSendCfg.pwIndex[wNo];			
		}	
		else
		{
			maxno=pTEqp->Cfg.wYCNum-1;
			if (wNo > maxno) return ERROR;
			no = wNo;			
		}	
		
		frzno = pTEqp->pYCCfg[no].wMaxMinNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(struct VMaxMinYC);    
	}
	else if ((pVEqp = g_Sys.Eqp.pInfo[*pwEqpID].pVEqpInfo) != NULL)
	{
		if (wNo >= pVEqp->Cfg.wYCNum) return ERROR;
		
		teid=pVEqp->pYC[wNo].wTEID;
		offset=pVEqp->pYC[wNo].wOffset;	   
						
		if ((pTEqp=g_Sys.Eqp.pInfo[teid].pTEqpInfo) == NULL) return ERROR;
		if (g_Sys.Eqp.pInfo[teid].pTEqpInfo->pYC == NULL) return ERROR;

		frzno = pTEqp->pYCCfg[offset].wMaxMinNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(struct VMaxMinYC);    
		*pwEqpID = teid;		
	}		
	else 
		return ERROR;

	return OK;
}

int GetDdHFrzOffset(WORD *pwEqpID, WORD wNo, DWORD *pdwOffset, BOOL asSend)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	WORD no, frzno, teid, offset;
	int maxno;

	if (*pwEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;
	
	if ((pTEqp = g_Sys.Eqp.pInfo[*pwEqpID].pTEqpInfo) != NULL)
	{
		if (asSend)
		{
			maxno=pTEqp->DDSendCfg.wNum-1;
			if (wNo > maxno) return ERROR;
			no = pTEqp->DDSendCfg.pwIndex[wNo];			
		}	
		else
		{
			maxno=pTEqp->Cfg.wDDNum-1;
			if (wNo > maxno) return ERROR;
			no = wNo;			
		}	
		
		frzno = pTEqp->pDDCfg[no].wHourFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(long);    
	}
	else if ((pVEqp = g_Sys.Eqp.pInfo[*pwEqpID].pVEqpInfo) != NULL)
	{
		if (wNo >= pVEqp->Cfg.wDDNum) return ERROR;
		
		teid=pVEqp->pDD[wNo].wTEID;
		offset=pVEqp->pDD[wNo].wOffset;	   
						
		if ((pTEqp=g_Sys.Eqp.pInfo[teid].pTEqpInfo) == NULL) return ERROR;
		if (g_Sys.Eqp.pInfo[teid].pTEqpInfo->pDD == NULL) return ERROR;

		frzno = pTEqp->pDDCfg[offset].wHourFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(long);    
		*pwEqpID = teid;		
	}		
	else 
		return ERROR;

	return OK;
}

int GetDdDFrzOffset(WORD *pwEqpID, WORD wNo, DWORD *pdwOffset, BOOL asSend)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	WORD no, frzno, teid, offset;
	int maxno;

	if (*pwEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;
	
	if ((pTEqp = g_Sys.Eqp.pInfo[*pwEqpID].pTEqpInfo) != NULL)
	{
		if (asSend)
		{
			maxno=pTEqp->DDSendCfg.wNum-1;
			if (wNo > maxno) return ERROR;
			no = pTEqp->DDSendCfg.pwIndex[wNo];			
		}	
		else
		{
			maxno=pTEqp->Cfg.wDDNum-1;
			if (wNo > maxno) return ERROR;
			no = wNo;			
		}	
		
		frzno = pTEqp->pDDCfg[no].wDayFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(long);    
	}
	else if ((pVEqp = g_Sys.Eqp.pInfo[*pwEqpID].pVEqpInfo) != NULL)
	{
		if (wNo >= pVEqp->Cfg.wDDNum) return ERROR;
		
		teid=pVEqp->pDD[wNo].wTEID;
		offset=pVEqp->pDD[wNo].wOffset;	   
						
		if ((pTEqp=g_Sys.Eqp.pInfo[teid].pTEqpInfo) == NULL) return ERROR;
		if (g_Sys.Eqp.pInfo[teid].pTEqpInfo->pDD == NULL) return ERROR;

		frzno = pTEqp->pDDCfg[offset].wDayFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(long);    
		*pwEqpID = teid;		
	}		
	else 
		return ERROR;

	return OK;
}

int GetDdMFrzOffset(WORD *pwEqpID, WORD wNo, DWORD *pdwOffset, BOOL asSend)
{
	struct VTrueEqp *pTEqp;
	struct VVirtualEqp *pVEqp;
	WORD no, frzno, teid, offset;
	int maxno;

	if (*pwEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;
	
	if ((pTEqp = g_Sys.Eqp.pInfo[*pwEqpID].pTEqpInfo) != NULL)
	{
		if (asSend)
		{
			maxno=pTEqp->DDSendCfg.wNum-1;
			if (wNo > maxno) return ERROR;
			no = pTEqp->DDSendCfg.pwIndex[wNo];			
		}	
		else
		{
			maxno=pTEqp->Cfg.wDDNum-1;
			if (wNo > maxno) return ERROR;
			no = wNo;			
		}	
		
		frzno = pTEqp->pDDCfg[no].wMonthFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(long);    
	}
	else if ((pVEqp = g_Sys.Eqp.pInfo[*pwEqpID].pVEqpInfo) != NULL)
	{
		if (wNo >= pVEqp->Cfg.wDDNum) return ERROR;
		
		teid=pVEqp->pDD[wNo].wTEID;
		offset=pVEqp->pDD[wNo].wOffset;	   
						
		if ((pTEqp=g_Sys.Eqp.pInfo[teid].pTEqpInfo) == NULL) return ERROR;
		if (g_Sys.Eqp.pInfo[teid].pTEqpInfo->pDD == NULL) return ERROR;

		frzno = pTEqp->pDDCfg[offset].wMonthFrzNo;
		if (frzno == 0xFFFF) return ERROR;

		*pdwOffset = frzno*sizeof(long);    
		*pwEqpID = teid;		
	}		
	else 
		return ERROR;

	return OK;
}

static void FFDll_Get_DbYCHData(WORD wEqpID,  BYTE *buf)
{
	int i;
	struct VTrueEqp *pTEqp;
	struct VDBYC_L DBYC_L;
	VYC *p = (VYC *)buf;

	//if (wEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;

	if ((pTEqp = g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo) != NULL)
	{
        for (i=0; i<pTEqp->YCHourFrz.wNum; i++)
        {
			DBYC_L.wNo = pTEqp->YCHourFrz.pwIndex[i];
			atomicReadTEYC_L(pTEqp,1,sizeof(struct VDBYC_L),&DBYC_L,FALSE);
			*p = DBYC_L.lValue;
			DPRINT("YC_%d : %d \n",DBYC_L.wNo,DBYC_L.lValue);
			p++;
		}
	}
}

static void FFDll_Get_DbYCDData(WORD wEqpID,  BYTE *buf)
{
	int i;
	struct VTrueEqp *pTEqp;
	struct VDBYC_L DBYC_L;
	VYC *p = (VYC *)buf;

	//if (wEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;

	if ((pTEqp = g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo) != NULL)
	{
        for (i=0; i<pTEqp->YCDayFrz.wNum; i++)
        {
			DBYC_L.wNo = pTEqp->YCDayFrz.pwIndex[i];
			atomicReadTEYC_L(pTEqp,1,sizeof(struct VDBYC_L),&DBYC_L,FALSE);
			*p = DBYC_L.lValue;
			p++;
		}
	}
}

static void FFDll_Get_DbYCMData(WORD wEqpID,  BYTE *buf)
{
	int i;
	struct VTrueEqp *pTEqp;
	struct VDBYC_L DBYC_L;
	VYC *p = (VYC *)buf;

	//if (wEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;

	if ((pTEqp = g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo) != NULL)
	{
        for (i=0; i<pTEqp->YCMonthFrz.wNum; i++)
        {
			DBYC_L.wNo = pTEqp->YCMonthFrz.pwIndex[i];
			atomicReadTEYC_L(pTEqp,1,sizeof(struct VDBYC_L),&DBYC_L,FALSE);
			*p = DBYC_L.lValue;
			p++;
		}
	}
}

static void FFDll_Get_DbYCMMDData(WORD wEqpID,  BYTE *buf)
{
	int i;
	struct VTrueEqp *pTEqp;
	struct VMaxMinYC *p = (struct VMaxMinYC *)buf;

	//if (wEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;

	if ((pTEqp = g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo) != NULL)
	{
        for (i=0; i<pTEqp->Cfg.wYCMaxMinNum; i++)
        {
            *p = pTEqp->pMaxMinYC[i];
			p++;
		}
	}
}

static void FFDll_Get_DbDDHData(WORD wEqpID,  BYTE *buf)
{
	int i;
	struct VTrueEqp *pTEqp;
	struct VDBDD DBDD;
    long *p = (long *)buf;
	BYTE* p_temp = (BYTE*)p;
	//if (wEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;

	if ((pTEqp = g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo) != NULL)
	{
        for (i=0; i<pTEqp->DDHourFrz.wNum; i++)
        {
			DBDD.wNo = pTEqp->DDHourFrz.pwIndex[i];
			atomicReadTEDD(pTEqp,1,sizeof(struct VDBDD),&DBDD,FALSE);
			memcpy(p_temp,(BYTE*)&DBDD.lValue,sizeof(long));
			p++;
			p_temp = (BYTE*)p;
		}
	}
}

static void FFDll_Get_DbDDDData(WORD wEqpID,  BYTE *buf)
{
	int i;
	struct VTrueEqp *pTEqp;
	struct VDBDD DBDD;
    long *p = (long *)buf;
    BYTE* p_temp = (BYTE*)p;
	//if (wEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;

	if ((pTEqp = g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo) != NULL)
	{
        for (i=0; i<pTEqp->DDDayFrz.wNum; i++)
        {
			DBDD.wNo = pTEqp->DDDayFrz.pwIndex[i];
			atomicReadTEDD(pTEqp,1,sizeof(struct VDBDD),&DBDD,FALSE);
		    memcpy(p_temp,(BYTE*)&DBDD.lValue,sizeof(long));
			p++;
			p_temp = (BYTE*)p;		
		}
	}
}

static void FFDll_Get_DbDDMData(WORD wEqpID,  BYTE *buf)
{
	int i;
	struct VTrueEqp *pTEqp;
	struct VDBDD DBDD;
  long *p = (long *)buf;
  BYTE* p_temp = (BYTE*)p;
	//if (wEqpID >= *g_Sys.Eqp.pwNum)  return ERROR;

	if ((pTEqp = g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo) != NULL)
	{
        for (i=0; i<pTEqp->DDMonthFrz.wNum; i++)
        {
			DBDD.wNo = pTEqp->DDMonthFrz.pwIndex[i];
			atomicReadTEDD(pTEqp,1,sizeof(struct VDBDD),&DBDD,FALSE);
			memcpy(p_temp,(BYTE*)&DBDD.lValue,sizeof(long));
			p++;
			p_temp = (BYTE*)p;
		}
	}
}

static void FFDll_Reset_DbYCMMDData(void)
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
				pTEqp->pMaxMinYC[i].lMin = 0x7FFFFFFF;
			}
		}
	} 
}

void freeze(void)
{
    struct VCalClock calclock;
	struct VSysClock oldclock;
	struct VSysClock sysclock;
	DWORD events;
	DPRINT("freeze init\n");
	memset(&oldclock, 0, sizeof(struct VSysClock));	
	tmEvEvery(FRZ_ID, SECTOTICK(30), EV_TM1);   /*30s*/
	
	for(;;)
	{
        	evReceive(FRZ_ID, EV_TM1, &events);

		GetSysClock(&calclock, CALCLOCK);
		CalClockTo(&calclock, &sysclock);
		DPRINT("freeze \n");

		if (sysclock.byMinute != oldclock.byMinute)			
		{
			DPRINT("FFDll_HourFreeze \n");
			FFDll_HourFreeze(calclock.dwMinute);
		}	
		
		if (((sysclock.wYear != oldclock.wYear) || (sysclock.byMonth != oldclock.byMonth) || (sysclock.byDay != oldclock.byDay)) && (sysclock.byHour == 0))
		{			
			FFDll_DayFreeze(&sysclock);
			FunDll_Exe(FUNDLL_TYPE_DAY_RESET, NULL);
			FFDll_Reset_DbYCMMDData();
		}

		if (((sysclock.wYear != oldclock.wYear) || (sysclock.byMonth != oldclock.byMonth)) && (sysclock.byDay == 1) && (sysclock.byHour == 0))
		{			
			FFDll_MonthFreeze(&sysclock);
		}

		memcpy(&oldclock, &sysclock, sizeof(struct VSysClock));
	}
}

void FFDirCheck(void)
{
	struct stat filestat;
	char  path[4*MAXFILENAME];
	char *myPath2[]={"","/frz","/frz/hour","/frz/day","/frz/month"};
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

int FFInit(struct VThSetupInfo *pInfo)
{
	VFFDll ffdll;
	int i;
	struct VTrueEqp *pTEqp;
	int task;
	FFDirCheck();

	frzSemId = smMCreate();

	pFFDll_Hour_Man = NULL;
	pFFDll_Day_Man = NULL;
	pFFDll_Month_Man = NULL;

	FF_MD_DEFAULT = GetProfileInt("冻结", "缺省密度[min]", 15, "system.cin");
	FF_HOUR_CAPACITY = GetProfileInt("冻结", "曲线容量", 30*24*4, "system.cin");
	FF_DAY_CAPACITY = GetProfileInt("冻结", "日冻结容量", 30, "system.cin");
	FF_MONTH_CAPACITY = GetProfileInt("冻结", "月冻结容量", 12, "system.cin");

	task = 0;
	for (i=0; i<*g_Sys.Eqp.pwNum; i++)
	{
		if ((pTEqp = g_Sys.Eqp.pInfo[i].pTEqpInfo) != NULL)
		{
			if (pTEqp->YCHourFrz.wNum) 
			{
				ffdll.tab = FF_TAB_YC;
				ffdll.type = FF_TYPE_VALUE;
				ffdll.date = FF_DATE_HOUR;
				if (pTEqp->Cfg.byYCFrzMD != 0)
					ffdll.capacity = pTEqp->Cfg.byYCFrzCP*(24*60/pTEqp->Cfg.byYCFrzMD+1);
				else
					ffdll.capacity = 0;
				ffdll.len = sizeof(VYC)*pTEqp->YCHourFrz.wNum;
				ffdll.md = pTEqp->Cfg.byYCFrzMD;
				if (ffdll.md == 0) ffdll.md = FF_MD_DEFAULT;
				ffdll.get_ffdll_data = FFDll_Get_DbYCHData;
				FFDll_Point_Regist(i, &ffdll);	
				task = 1;
			}

			if (pTEqp->YCDayFrz.wNum) 
			{
				ffdll.tab = FF_TAB_YC;
				ffdll.type = FF_TYPE_VALUE;
				ffdll.date = FF_DATE_DAY;
				ffdll.capacity = pTEqp->Cfg.byYCFrzCP;
				ffdll.len = sizeof(VYC)*pTEqp->YCDayFrz.wNum;
				ffdll.md = 1;
				ffdll.get_ffdll_data = FFDll_Get_DbYCDData;
				FFDll_Point_Regist(i, &ffdll);	
				task = 1;
			}

			if (pTEqp->YCMonthFrz.wNum) 
			{
				ffdll.tab = FF_TAB_YC;
				ffdll.type = FF_TYPE_VALUE;
				ffdll.date = FF_DATE_MONTH;
				ffdll.capacity = 0;
				ffdll.len = sizeof(VYC)*pTEqp->YCMonthFrz.wNum;
				ffdll.md = 1;
				ffdll.get_ffdll_data = FFDll_Get_DbYCMData;
				FFDll_Point_Regist(i, &ffdll);	
				task = 1;
			}

			if (pTEqp->Cfg.wYCMaxMinNum) 
			{
				ffdll.tab = FF_TAB_YC;
				ffdll.type = FF_TYPE_MAXMIN;
				ffdll.date = FF_DATE_DAY;
				ffdll.capacity = pTEqp->Cfg.byYCFrzCP;
				ffdll.len = sizeof(struct VMaxMinYC)*pTEqp->Cfg.wYCMaxMinNum;
				ffdll.md = 1;
				ffdll.get_ffdll_data = FFDll_Get_DbYCMMDData;
				FFDll_Point_Regist(i, &ffdll);	
				task = 1;
			}
			
			if (pTEqp->DDHourFrz.wNum) 
			{
				ffdll.tab = FF_TAB_DD;
				ffdll.type = FF_TYPE_VALUE;
				ffdll.date = FF_DATE_HOUR;
				if (pTEqp->Cfg.byDDFrzMD != 0)
					ffdll.capacity = pTEqp->Cfg.byDDFrzCP*(24*60/pTEqp->Cfg.byDDFrzMD+1);
				else
					ffdll.capacity = 0;
				ffdll.len = sizeof(long)*pTEqp->DDHourFrz.wNum;
				ffdll.md = pTEqp->Cfg.byDDFrzMD;
				if (ffdll.md == 0) ffdll.md = FF_MD_DEFAULT;
				ffdll.get_ffdll_data = FFDll_Get_DbDDHData;
				FFDll_Point_Regist(i, &ffdll);	
				task = 1;
			}			

			if (pTEqp->DDDayFrz.wNum) 
			{
				ffdll.tab = FF_TAB_DD;
				ffdll.type = FF_TYPE_VALUE;
				ffdll.date = FF_DATE_DAY;
				ffdll.capacity = pTEqp->Cfg.byDDFrzCP;
				ffdll.len = sizeof(long)*pTEqp->DDDayFrz.wNum;
				ffdll.md = 1;
				ffdll.get_ffdll_data = FFDll_Get_DbDDDData;
				FFDll_Point_Regist(i, &ffdll);	
				task = 1;
			}

			if (pTEqp->DDMonthFrz.wNum) 
			{
				ffdll.tab = FF_TAB_DD;
				ffdll.type = FF_TYPE_VALUE;
				ffdll.date = FF_DATE_MONTH;
				ffdll.capacity = 0;
				ffdll.len = sizeof(long)*pTEqp->DDMonthFrz.wNum;
				ffdll.md = 1;
				ffdll.get_ffdll_data = FFDll_Get_DbDDMData;
				FFDll_Point_Regist(i, &ffdll);	
				task = 1;
			}
			
		}
	} 

	if (task) 
		SpecialTaskSetup(pInfo,FRZ_ID);

	return(OK);
}

void FFReset(void)
{
	FAST DIR  *pDir;		/* ptr to directory descriptor */
	FAST struct dirent	*pDirEnt;	/* ptr to dirent */
	char fname[4*MAXFILENAME];
	char path[4*MAXFILENAME];
	
	sprintf(path,"%s/frz/hour",SYS_PATH_ROOT2);
	pDir = opendir (path) ;
	if( pDir != NULL ) 
	{
		for(;;)
		{		
			pDirEnt = readdir (pDir);			

			if (pDirEnt == NULL) break;
			if (pDirEnt->d_name[0] == '.') continue;			

			sprintf(fname,"%s/frz/hour/",SYS_PATH_ROOT2);
			strcat(fname, pDirEnt->d_name);
			remove(fname);
		}
	}
	closedir (pDir);

	sprintf(path,"%s/frz/day",SYS_PATH_ROOT2);
	pDir = opendir (path) ;
	if( pDir != NULL ) 
	{
		for(;;)
		{		
			pDirEnt = readdir (pDir);			

			if (pDirEnt == NULL) break;
			if (pDirEnt->d_name[0] == '.') continue;			

			sprintf(fname,"%s/frz/day/",SYS_PATH_ROOT2);
			strcat(fname, pDirEnt->d_name);
			remove(fname);
		}
	}
	closedir (pDir);

	sprintf(path,"%s/frz/month",SYS_PATH_ROOT2);
	pDir = opendir (path) ;
	if( pDir != NULL ) 
	{
		for(;;)
		{		
			pDirEnt = readdir (pDir);			

			if (pDirEnt == NULL) break;
			if (pDirEnt->d_name[0] == '.') continue;			

			sprintf(fname,"%s/frz/month/",SYS_PATH_ROOT2);
			strcat(fname, pDirEnt->d_name);
			remove(fname);
		}
	}
	closedir (pDir);
}

WORD ReadYCCurve(WORD wEqpID, WORD wNo, struct VSysClock *td, WORD wNum, int m, long *buf, BOOL asSend)
{	
	int min;
	DWORD offset;
	
    //FF_MiDu2Min(m, &min);
    	min = m;
	if (GetYcHFrzOffset(&wEqpID, wNo, &offset, asSend) == ERROR) return 0;
	return(FFDll_CurveRead(wEqpID, FF_TAB_YC, FF_TYPE_VALUE, td, (BYTE *)buf, min, offset, sizeof(long), wNum)); 
}

WORD ReadYCDay(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend)
{	
	DWORD offset;
	int ret;
	
	if (GetYcDFrzOffset(&wEqpID, wNo, &offset, asSend) == ERROR) return 0;
	ret = FFDll_DayRead(wEqpID, FF_TAB_YC, FF_TYPE_VALUE, td, (BYTE *)buf, sizeof(long), offset); 
	if (ret == OK) 
		return 1;
	else
		return 0;
}

WORD ReadYCMonth(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend)
{	
	DWORD offset;
	int ret;
	
	if (GetYcMFrzOffset(&wEqpID, wNo, &offset, asSend) == ERROR) return 0;
	ret = FFDll_MonthRead(wEqpID, FF_TAB_YC, FF_TYPE_VALUE, td, (BYTE *)buf, sizeof(long), offset); 
	if (ret == OK) 
		return 1;
	else
		return 0;
}

WORD ReadYCMaxMin(WORD wEqpID, WORD wNo, struct VSysClock *td, struct VMaxMinYC *buf, BOOL asSend)
{	
	DWORD offset, min;
	struct VSysClock sysclock;
	struct VTrueEqp *pTEqp;
	int no, ret;
	
	if (GetYcMMDFrzOffset(&wEqpID, wNo, &offset, asSend) == ERROR) return 0;

    GetSysClock(&sysclock, SYSCLOCK);
	//今天,取当前值
	if ((td->wYear==sysclock.wYear) && (td->byMonth==sysclock.byMonth) && (td->byDay==sysclock.byDay))
	{		
		pTEqp = g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo;

        no = offset/sizeof(struct VMaxMinYC);
		*buf = pTEqp->pMaxMinYC[no];
		ret = OK;
	}
	else
	{
		ret = FFDll_DayRead(wEqpID, FF_TAB_YC, FF_TYPE_MAXMIN, td, (BYTE *)buf, sizeof(struct VMaxMinYC), offset); 
	}	

	if (ret == OK) 
	{
		td->byHour = 0;
		td->byMinute = 0;

		min = CalendarClock(td)+24*60;
		if (buf->max_tm.dwMinute >= min) 
			buf->max_tm.dwMinute = min-1;
		if (buf->min_tm.dwMinute >= min) 
			buf->min_tm.dwMinute = min-1;

		return 1;
	}
	else
		return 0;
}

WORD ReadDDCurve(WORD wEqpID, WORD wNo, struct VSysClock *td, WORD wNum, int m, long *buf, BOOL asSend)
{	
	int min;
	DWORD offset;
	
    //FF_MiDu2Min(m, &min);
    min = m;
	if (GetDdHFrzOffset(&wEqpID, wNo, &offset, asSend) == ERROR) return 0;
	return(FFDll_CurveRead(wEqpID, FF_TAB_DD, FF_TYPE_VALUE, td, (BYTE *)buf, min, offset, sizeof(long), wNum)); 
}

WORD ReadDDDay(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend)
{	
	DWORD offset;
	int ret;
	
	if (GetDdDFrzOffset(&wEqpID, wNo, &offset, asSend) == ERROR) return 0;
	ret = FFDll_DayRead(wEqpID, FF_TAB_DD, FF_TYPE_VALUE, td, (BYTE *)buf, sizeof(long), offset); 
	if (ret == OK) 
		return 1;
	else
		return 0;
}

WORD ReadDDMonth(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend)
{	
	DWORD offset;
	int ret;
	
	if (GetDdMFrzOffset(&wEqpID, wNo, &offset, asSend) == ERROR) return 0;
	ret = FFDll_MonthRead(wEqpID, FF_TAB_DD, FF_TYPE_VALUE, td, (BYTE *)buf, sizeof(long), offset); 
	if (ret == OK) 
		return 1;
	else
		return 0;
}

#else
WORD ReadYCCurve(WORD wEqpID, WORD wNo, struct VSysClock *td, WORD wNum, int m, long *buf, BOOL asSend)
{
	return 0;
}

WORD ReadYCDay(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend)
{
	return 0;
}

WORD ReadYCMonth(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend)
{
	return 0;
}

WORD ReadYCMaxMin(WORD wEqpID, WORD wNo, struct VSysClock *td, struct VMaxMinYC *buf, BOOL asSend)
{
	return 0;
}

WORD ReadDDCurve(WORD wEqpID, WORD wNo, struct VSysClock *td, WORD wNum, int m, long *buf, BOOL asSend)
{
	return 0;
}

WORD ReadDDDay(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend)
{
	return 0;
}

WORD ReadDDMonth(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend)
{
	return 0;
}

#endif

