
#include "syscfg.h"

#ifdef  INCLUDE_RECORD
#include "sys.h"
#include "record.h"

#define WAVPOINT    (120)    //不能大于采样点160

#define RECORD_COM_CFG_LEN		2048
#define RECORD_FD_NUM_MAX		MAX_FD_NUM

WORD recordFdnum;		/*需要记录的回线数*/

extern VPrSetPublic *prSetPublic[2];
extern VPrRunPublic *prRunPublic;

enum{
	WAV_RECORD_IDLE=0,
	//WAV_RCD_START,
	WAV_RECORD_DOING,
	WAV_RECORD_OK
};

#define AD_PRE_WAV   5
#define AD_PER_SIZE (WAVPOINT*14)//

#define AD_BUF_SIZE (2048*2*4)		//1s 可以改大些!
#define WAV_SEG_MAX (64)



#define FD_RECORD_NUM  (prSetPublic[prRunPublic->set_no]->dRecNum ? prSetPublic[prRunPublic->set_no]->dRecNum : 64)

#define RECORD_CON_MAX        25 /*25*200*4=20s,25*200*20=100K*/

typedef struct
{
	struct VCalClock time[WAV_SEG_MAX];

	int  rec_time_cnt;

  int  rec_seg;
	int  rec_seg_start;
	int  rec_seg_stop;
	int  rec_con_num;
	int  rec_state[WAV_SEG_MAX];
	BYTE rec_seg_flag[WAV_SEG_MAX];
	WORD wWavRcdCount[WAV_SEG_MAX];//已记录的数据点数
	WORD wWavDataCnt[WAV_SEG_MAX];//buffer的索引
	WORD wWavRcdNeed[WAV_SEG_MAX];//需要记录的点数
	WORD wWavSamFreq[WAV_SEG_MAX];
	WORD wSamRcdCnt;
	short wBufUsed;
    WORD  wRecChanNum;
	WORD  wRecAiNum;
	WORD  wRecDiNum;
	short *buffer;
}VRecordLine;


BOOL g_record = 0;
static VRecordLine 	*recordLine;	/*每回线对应的录波结构*/
extern int g_prInit;
/////////////////////////////
VFdSamRecordIndex *FdSamRecordIndex;

static int recordFileWrite(WORD fd, int seg);

int recordInit(WORD wTaskID)
{
	int i;

  	recordFdnum = g_Sys.MyCfg.wFDNum;
	if(recordFdnum > RECORD_FD_NUM_MAX)
	{
		myprintf(wTaskID,LOG_ATTR_INFO, "Feeder number too many!");
		return ERROR;
	}

	recordLine	=( VRecordLine *)malloc(sizeof(VRecordLine)*recordFdnum);
	if (recordLine==NULL) 
		return(ERROR);
	memset(recordLine, 0, sizeof(VRecordLine)*recordFdnum);

	for(i = 0;i <recordFdnum; i++)
	{
		recordLine[i].wRecAiNum = 8;
		recordLine[i].wRecDiNum = 1;
		recordLine[i].wRecChanNum = 8 + 4;
		recordLine[i].buffer = malloc(AD_BUF_SIZE*recordLine[i].wRecChanNum*2);
	    // gu,  file page 73,line 18		
	}
	g_record = true;
	return OK;
}

/*------------------------------------------------------------------------
 Procedure:     recordWaitWrite ID:(0~recordFdnum-1)
 Purpose:       录波文件写入,放入低级任务上下文, 
 Input:
 Output:
 Errors:		需要信号量保证缓冲区,同线连续故障
------------------------------------------------------------------------*/
void recordWrite(WORD fd, int seg)
{
	if(fd >= recordFdnum) 
		return;
	recordFileWrite(fd, seg);
}

void RecordWaitWrite(int seg)
{
	int fd;

	for(fd=0; fd<recordFdnum; fd++)
	{
		if(	WAV_RECORD_OK==recordLine[fd].rec_state[seg])
			recordWrite( fd, seg );
	}
}

static void recordclear(WORD fd, int seg)
{
   	VRecordLine *pRecLine=recordLine+fd;
	WORD count;
	count = pRecLine->wWavRcdCount[seg];
	pRecLine->rec_seg_flag[seg] = RECORD_ORGIN;
	pRecLine->wWavDataCnt[seg]  = 0;
	pRecLine->wWavRcdCount[seg] = 0;
	pRecLine->wWavRcdNeed[seg]  = 0;
	pRecLine->wWavSamFreq[seg]  = 0;
	pRecLine->wBufUsed -= count;
	if(pRecLine->wBufUsed < 0)
	   pRecLine->wBufUsed = 0;
	pRecLine->rec_state[seg]    = WAV_RECORD_IDLE;
}
	
/*------------------------------------------------------------------------
 Procedure:     recordFileWrite ID:1
 Purpose:       新记录文件写入函数
 Input:
 Output:
 Errors:		
------------------------------------------------------------------------*/
static int recordFileWrite(WORD fd, int seg)
{
	int i,j;
	int index;
	struct VCalClock time;
	struct VSysClock systime;
	VRecordLine *pRecLine=recordLine+fd;
	struct VSysClock  clock2;
	BYTE* recorddatabuf = NULL;
	int len = 0,readlen = 0;
	
	if(pRecLine->rec_seg_flag[seg] & RECORD_START)
    {
		CalClockTo(&(pRecLine->time[seg]), &systime);
		
		if(pRecLine->time[seg].wMSecond >=(FT_SAM_POINT_N*AD_PRE_WAV))
		{
			time.wMSecond = pRecLine->time[seg].wMSecond - (FT_SAM_POINT_N*AD_PRE_WAV);
			time.dwMinute = pRecLine->time[seg].dwMinute;
		}
		else
		{
			time.wMSecond = 60000 - (FT_SAM_POINT_N<<2) + pRecLine->time[seg].wMSecond;
			time.dwMinute = pRecLine->time[seg].dwMinute - 1;
		}
		
		CalClockTo(&time,&clock2);
		
		recorddatabuf = ReadRecordBuf(&readlen);
		if(recorddatabuf != NULL)
		{
			len = 0;
			for(i = 0;i < pRecLine->wWavRcdCount[seg];i++)
			{
				if((pRecLine->wWavDataCnt[seg] +i) < AD_BUF_SIZE) 
					index = (pRecLine->wWavDataCnt[seg] + i)*pRecLine->wRecChanNum ;
				else
					index = ((pRecLine->wWavDataCnt[seg] + i - AD_BUF_SIZE)&(AD_BUF_SIZE -1))*pRecLine->wRecChanNum;

				memcpy(recorddatabuf + len,&i,sizeof(DWORD));
				len += sizeof(DWORD);
				j = 20000/WAVPOINT*i;
				memcpy(recorddatabuf + len,&j,sizeof(DWORD));
				len += sizeof(DWORD);
				memcpy(recorddatabuf + len,&pRecLine->buffer[index],sizeof(WORD)* (pRecLine->wRecAiNum + 1));
				len += sizeof(WORD)* (pRecLine->wRecAiNum + 1);
			}	
		}
	}
	if(len < readlen)
		WriteBmRecord(fd, WAVPOINT*50, pRecLine->wWavRcdCount[seg], pRecLine->time[seg],time);	
	recordclear(fd, seg);
	shellprintf("录波完成 \n");
	return OK;
}

//初始化录波索引参数
void InitFdSamRecordIndex(void)
{
	int fd,port,bit,j,k;
	VFdCfg *pCfg;

	FdSamRecordIndex = (VFdSamRecordIndex*)malloc(sizeof(VFdSamRecordIndex)*MAX_FD_NUM);
	memset(FdSamRecordIndex, 0, sizeof(VFdSamRecordIndex));

	for (fd=0; fd<fdNum; fd++)
	{
		pCfg = pFdCfg+fd;

		//DI
#ifdef INCLUDE_YX
		if (pCfg->kg_stateno >= 0)
		{
			port = g_YxNo2Port[pCfg->kg_stateno].port;
			bit = g_YxNo2Port[pCfg->kg_stateno].bit;

			FdSamRecordIndex[fd].wYxPort=port;
			FdSamRecordIndex[fd].wYxBit=bit;
		}
#endif

		////DO
#ifdef INCLUDE_YK
		if( pCfg->kg_ykno > 0 ) 
		{
			FdSamRecordIndex[fd].DoMask=1<<(3*pCfg->kg_ykno-3);
			FdSamRecordIndex[fd].DhMask=1<<(3*pCfg->kg_ykno-2);		//WAAA??? NEED TEST
		}
#endif

		///GAIN   暂不考虑其系数关系。
		FdSamRecordIndex[fd].GainA[0]=pFdCfg[fd].gain_i[0];

		FdSamRecordIndex[fd].GainA[1]=pFdCfg[fd].gain_i[1];

		FdSamRecordIndex[fd].GainA[2]=pFdCfg[fd].gain_i[2];

		FdSamRecordIndex[fd].GainA[3]=pFdCfg[fd].gain_i0;

		FdSamRecordIndex[fd].GainA[4]=pFdCfg[fd].gain_u[0];
		FdSamRecordIndex[fd].GainA[5]=pFdCfg[fd].gain_u[1];
		FdSamRecordIndex[fd].GainA[6]=pFdCfg[fd].gain_u[2];
		FdSamRecordIndex[fd].GainA[7]=pFdCfg[fd].gain_u0;

		///AI
        //建立回线对AI通道的有效值地址指针,如果无该项值,则地址为缺省 -1
        k=0;
		
		for (j=0; j<4; j++)
		{
			if((pFdCfg[fd].mmi_meaNo_ai[j]) != -1)
			{
				FdSamRecordIndex[fd].AiIndex[k] = pFdCfg[fd].mmi_meaNo_ai[j];
				FdSamRecordIndex[fd].GainB[j] = pMeaGain[FdSamRecordIndex[fd].AiIndex[k]].b;
				k++;
			}
			else
			{
				FdSamRecordIndex[fd].AiIndex[k] = -1;
				FdSamRecordIndex[fd].GainB[j] = 0;
				k++;
			}
		}
		for (j=0; j<4; j++)
		{
			if(pFdCfg[fd].mmi_meaNo_ai[j+4] != -1)
			{
				FdSamRecordIndex[fd].AiIndex[k] = pFdCfg[fd].mmi_meaNo_ai[j+4];
				FdSamRecordIndex[fd].GainB[j+4] = pMeaGain[FdSamRecordIndex[fd].AiIndex[k]].b;
				k++;
			}
			else
			{
				FdSamRecordIndex[fd].AiIndex[k] = -1;
				FdSamRecordIndex[fd].GainB[j+4] = 0;
				k++;
			}
		}
	}
}

int StartWavRecord(int fd, struct VCalClock *time, int flag)
{
	VRecordLine *pRecLine;
	int seg;
	int seg_pre;
	WORD buf_free;

	if(!g_record)
		return false;

    pRecLine = recordLine+fd;
	seg = pRecLine->rec_seg;
	if(pRecLine->wBufUsed >= AD_BUF_SIZE)
	{
//	   WriteWarnEvent(NULL, SYS_ERR_PR, fd+1, "录波正在处理，返回");
	   return false;
	}
       pRecLine->rec_con_num = 0;
    buf_free = AD_BUF_SIZE-pRecLine->wBufUsed;

	pRecLine->rec_time_cnt = 0;
	if(pRecLine->rec_state[seg]==WAV_RECORD_DOING)
	{
		if(pRecLine->rec_seg_flag[seg] & RECORD_STOP)
		pRecLine->rec_seg_flag[seg] &= ~RECORD_STOP;
		pRecLine->rec_seg_flag[seg] |= flag;
		if(pRecLine->wWavRcdCount[seg] < buf_free)
		    buf_free = pRecLine->wWavRcdCount[seg];
		pRecLine->wWavRcdNeed[seg] += buf_free;
		return true;
	}
	if(pRecLine->rec_state[seg]==WAV_RECORD_OK)
    {
        seg++;
	    if(seg >= WAV_SEG_MAX)
	   	   seg = 0;
    }
	pRecLine->rec_seg = seg;
	if(pRecLine->rec_state[seg]==WAV_RECORD_IDLE)
	{
		//初始化缓冲区相关数据。
		pRecLine->rec_seg_flag[seg] = flag;

        if(time == NULL)
			GetSysClock(&(pRecLine->time[seg]), CALCLOCK);
		else
		    pRecLine->time[seg] = *time;
		
		pRecLine->wWavRcdCount[seg]=0;//已记录的数据点数

		//上面读取当前采样指针，以下计算待录波首地址。
	

	    pRecLine->wSamRcdCnt= (wMeaSamCnt - MEA_SAM_POINT_N*4*AD_PRE_WAV)&(SAM_BUF_LEN - 1);		//WAAA 取时间与取指针时间不一定能够保证同步。
//	    pRecLine->wSamRcdCnt-=(FT_SAM_POINT_N<<2);
//	    pRecLine->wSamRcdCnt &= (SAM_BUF_LEN-1);


		if(buf_free > AD_PER_SIZE)
			buf_free = AD_PER_SIZE;
		pRecLine->wWavRcdNeed[seg] = buf_free;
		    pRecLine->wWavSamFreq[seg] = WAVPOINT*50;

		if(seg == 0)
			seg_pre = WAV_SEG_MAX-1;
		else
			seg_pre = seg-1;
		pRecLine->wWavDataCnt[seg]=pRecLine->wWavDataCnt[seg_pre]+
			                       pRecLine->wWavRcdCount[seg_pre];
		pRecLine->wWavDataCnt[seg]=pRecLine->wWavDataCnt[seg]&(AD_BUF_SIZE-1);

		pRecLine->rec_state[seg]=WAV_RECORD_DOING;

		return true;
	}

//	WriteWarnEvent(NULL, SYS_ERR_PR, fd+1, "录波正在处理，返回");
	return false;

}

//采样中断程序中调用记录波数据函数
void WAV_Record_Interrupt(void)
{	
	int		i,fd;
	int     seg,index;
	short	*psam;//160点采样
	short	*pdes;
	DWORD   dWord;
//	struct VFileMsgs msg;
	VRecordLine *pRecLine;
  
	if(!g_record)
		return;
    
	for (fd=0; fd<fdNum; fd++)
	{
	    pRecLine = recordLine+fd;
		
		pRecLine->rec_time_cnt = 0;
	    seg = pRecLine->rec_seg;
		if(pRecLine->rec_state[seg]==WAV_RECORD_DOING )//|| recordLine[fd].rec_state==WAV_RCD_START)
		{
            index = pRecLine->wWavDataCnt[seg]+pRecLine->wWavRcdCount[seg];
			index = index&(AD_BUF_SIZE-1);
			index = index * pRecLine->wRecChanNum;
			psam =(short *)&nMeaSam[pRecLine->wSamRcdCnt];
			pdes =(short *)&(pRecLine->buffer[index]);

			//RECORDING
			for (i =0; i < pRecLine->wRecAiNum; i++)
			{////AI
				if(FdSamRecordIndex[fd].AiIndex[i]==-1)
					*(pdes+i) = 0;
				else
					*(pdes+i) =*(psam+ FdSamRecordIndex[fd].AiIndex[i] );
			}
			psam+=AICHAN_NUMBER;


			//////DI
#ifdef INCLUDE_YX
      	    if ( (*(psam+ FdSamRecordIndex[fd].wYxPort )) & FdSamRecordIndex[fd].wYxBit )
				pdes[pRecLine->wRecAiNum] = 1;
			else
				pdes[pRecLine->wRecAiNum] = 0;   
			psam+=MAX_YX_PORT_NUM;
            if(*psam & (0x01<<fd))
			   	pdes[pRecLine->wRecAiNum+1] = 1;
			else
			   	pdes[pRecLine->wRecAiNum+1] = 0;
			psam++;			
#endif
			//////DO
#ifdef INCLUDE_YK
			dWord=*psam;
			psam++;
			dWord+=(*psam)<<16;
			if( dWord & FdSamRecordIndex[fd].DhMask)
				pdes[pRecLine->wRecAiNum+2] =1;
			else
				pdes[pRecLine->wRecAiNum+2] =0;
#if(DEV_SP != DEV_SP_CPR)          
		    if( dWord & FdSamRecordIndex[fd].DoMask)
				pdes[pRecLine->wRecAiNum+3] =1;
			else
				pdes[pRecLine->wRecAiNum+3] =0;		
#endif
#endif         	

			//END RECORDING

            pRecLine->wBufUsed++;
			pRecLine->wWavRcdCount[seg]++;//已记录的数据点数
			pRecLine->wSamRcdCnt += 1;// 160点采样
			pRecLine->wSamRcdCnt&=(SAM_BUF_LEN-1);

			if (pRecLine->wWavRcdCount[seg] >= pRecLine->wWavRcdNeed[seg])	//recTime[fd])
			{
				pRecLine->rec_state[seg]=WAV_RECORD_OK;
				evSend(SELF_DIO_ID, EV_RCD);
				return;
			}
		}
	}
}

int WAV_Record_Scan(void)
{
	int fd,i,ret;
	VRecordLine *pRecLine;
  
	ret = 0;
	if(!g_record)
		return 0;
	
	for (fd=0; fd<fdNum; fd++)
	{
		pRecLine = recordLine+fd;
		
		for( i = 0; i < WAV_SEG_MAX ; i++)
		{
			if((pRecLine->wWavRcdCount[i] >= pRecLine->wWavRcdNeed[i])
				&& (pRecLine->rec_state[i] == WAV_RECORD_OK))
			{
				RecordWaitWrite(i);
				ret = 1;
				shellprintf("scan \r\n");
			}
		}
	}
	
	return ret;
}

#endif /*INCLUDE_RECORD*/
