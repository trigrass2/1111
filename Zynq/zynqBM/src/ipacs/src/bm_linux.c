#include "plt_include.h"
#include "plt_ram.h"
#include "plt_mc_inner_comm.h"

#define MAX_FILE_LEN  (128*1024)

#define PR_SET_SIZE             1024

//共享配置文件偏移
#define FILE_SYSTEM_OFFSET              0x00       
#define FILE_SYSTEM_LEN                (64*1024)   //sysem.cfg 64K
#define FILE_PROSETPUBTABLE_OFFSET     (FILE_SYSTEM_OFFSET + FILE_SYSTEM_LEN)  //64 
#define FILE_PROSETPUBTABLE_LEN        (8*1024) // ProSetPubTable.cfg 8K
#define FILE_PROSETTABLE_OFFSET        (FILE_PROSETPUBTABLE_OFFSET + FILE_PROSETPUBTABLE_LEN)   //72
#define FILE_PROSETTABLE_LEN           (8*1024)//prosettable.cfg     8k
#define FILE_FDPUBSET_OFFSET           (FILE_PROSETTABLE_OFFSET + FILE_PROSETTABLE_LEN)   //80 fdpubset   1K   最大18间隔
#define FILE_FDSET_OFFSET              (FILE_FDPUBSET_OFFSET + 18*PR_SET_SIZE)   //98 fdset      1K   最大18间隔
#define FILE_RUNPARA_OFFSET            (FILE_FDSET_OFFSET + 18*PR_SET_SIZE)      //116
#define FILE_RUNPARA_LEN               (8*1024)
#define FILE_SYSEXT_OFFSET             (FILE_RUNPARA_OFFSET + FILE_RUNPARA_LEN)      //116
#define FILE_SYSEXT_LEN                (8*1024)
#define FILE_CCD_OFFSET                (FILE_SYSEXT_OFFSET + FILE_SYSEXT_LEN)  //
#define FILE_CCD_LEN                   (64*1024)    



#define FILE_SYSTEM_FLAG              0x01       //sysem.cfg           bit0
#define FILE_RUNPARA_FLAG             0x02       //runpara.cfg         bit1
#define FILE_SYSEXT_FLAG              0x04       //systemext.cfg       bit2 
#define FILE_CCD_FLAG                 0x08       //ccd文件               bit3 


#define FILE_PROSETPUBTABLE_FLAG      0x01       // ProSetPubTable.cfg 
#define FILE_FDPUBSET_FLAG            0x02       //fdpubset            

#define FILE_PROSETTABLE_FLAG         0x01       //prosettable.cfg     
#define FILE_FDSET_FLAG               0x02       //fdset    


int ReadParaFile(char *filename, BYTE *buf, int buf_len)
{
	TSHARED_LINUX_STRUCT *pSharedLinux = GetSharedLinuxMemory();
	char cfgname[64];
	int i;
		
	if(pSharedLinux->config_data.flag_begin != FLAG_SHARED_MEMORY_VALID)
	{
		PRINTF_WITH_LOG("SharedLinux config_data memory flag_begin not initial.\n\r");
		return ERROR;
	}
	if(pSharedLinux->config_data.flag_end != FLAG_SHARED_MEMORY_VALID)
	{
		PRINTF_WITH_LOG("SharedLinux config_data memory flag_end not initial.\n\r");
		return ERROR;
	}
	
	//system.cfg
	if ((strcmp(filename, "system.cfg") == 0) && (pSharedLinux->config_data.fileflag & FILE_SYSTEM_FLAG))
	{
		memcpy(buf, pSharedLinux->config_data.data,buf_len);
		return OK;
	}
	
	//ProSetPubTable.cfg
	if ((strcmp(filename, "ProSetPubTable.cfg") == 0) && (pSharedLinux->config_data.prsetpubfileflag & FILE_PROSETPUBTABLE_FLAG))
	{
		memcpy(buf, pSharedLinux->config_data.data + FILE_PROSETPUBTABLE_OFFSET,FILE_PROSETPUBTABLE_LEN);
		return OK;
	}
	
	//ProSetTable.cfg
	if ((strcmp(filename, "ProSetTable.cfg") == 0) && (pSharedLinux->config_data.prsetfileflag & FILE_PROSETTABLE_FLAG))
	{
		memcpy(buf, pSharedLinux->config_data.data + FILE_PROSETTABLE_OFFSET,FILE_PROSETTABLE_LEN);
		return OK;
	}
	
	for(i = 0;i < 18;i++)
	{
		if(i == 0)
			sprintf(cfgname,"fdPubSet.cfg");
		else
			sprintf(cfgname, "fd%02dPubSet.cfg", i+1);
		
		if ((strcmp(filename, cfgname) == 0) && (pSharedLinux->config_data.prsetpubfileflag & ((FILE_FDPUBSET_FLAG << i))))
		{
			memcpy(buf, pSharedLinux->config_data.data + FILE_FDPUBSET_OFFSET + i*PR_SET_SIZE,PR_SET_SIZE);
			return OK;
		}
	}
	
	for(i = 0; i < 18; i++)
	{	
		sprintf(cfgname,"fd%dset.cfg",i+1);
		if ((strcmp(filename, cfgname) == 0) && (pSharedLinux->config_data.prsetfileflag & (FILE_FDSET_FLAG << i)))
		{
			memcpy(buf, pSharedLinux->config_data.data + FILE_FDSET_OFFSET + i*PR_SET_SIZE,PR_SET_SIZE);
			return OK;
		}
	}

	if ((strcmp(filename, "runpara.cfg") == 0) && (pSharedLinux->config_data.fileflag & FILE_RUNPARA_FLAG))
	{
		memcpy(buf, pSharedLinux->config_data.data + FILE_RUNPARA_OFFSET,FILE_RUNPARA_LEN);
		return OK;
	}

	if ((strcmp(filename, "systemext.cfg") == 0) && (pSharedLinux->config_data.fileflag & FILE_SYSEXT_FLAG))
	{
		memcpy(buf, pSharedLinux->config_data.data + FILE_SYSEXT_OFFSET,FILE_SYSEXT_LEN);
		return OK;
	}

	if ((strcmp(filename, "ccd.cfg") == 0) && (pSharedLinux->config_data.fileflag & FILE_CCD_FLAG))
	{
		memcpy(buf, pSharedLinux->config_data.data + FILE_CCD_OFFSET,FILE_CCD_LEN);
		return OK;
	}

	return ERROR;
}


//system存入linux共享内存，    保护部分存入BM共享内存,linux再写文件
int WriteParaBmFile(const char * filename,BYTE * buf,int len)
{
	//
	TSHARED_LINUX_STRUCT *pSharedLinux = GetSharedLinuxMemory();
	TSHARED_BAREMETAL_STRUCT *pShareBM = GetSharedBMMemory();
	char cfgname[64];
	int i;

	pSharedLinux->config_data.flag_begin = FLAG_SHARED_MEMORY_VALID;
	pSharedLinux->config_data.flag_end = FLAG_SHARED_MEMORY_VALID;

	if(strcmp(filename, "system.cfg") == 0)  //写入共享内存，方便调用
	{
		memcpy(pSharedLinux->config_data.data, buf,len);
		pSharedLinux->config_data.fileflag |= FILE_SYSTEM_FLAG;
		return OK;
	}

	if(strcmp(filename, "runpara.cfg") == 0)  //写入共享内存，方便调用
	{
		memcpy(pSharedLinux->config_data.data + FILE_RUNPARA_OFFSET , buf,len);
		pSharedLinux->config_data.fileflag |= FILE_RUNPARA_FLAG;
		return OK;
	}

	if(strcmp(filename, "systemext.cfg") == 0)  //写入共享内存，方便调用
	{
		memcpy(pSharedLinux->config_data.data + FILE_SYSEXT_OFFSET, buf,len);
		pSharedLinux->config_data.fileflag |= FILE_SYSEXT_LEN;
		return OK;
	}
	
	//其余写到BM共享内存,linux写文件
	if(strcmp(filename, "ProSetPubTable.cfg") == 0)
	{
		memcpy(pShareBM->prosetpubtable, buf,len);
		pShareBM->prsetpubflag |= FILE_PROSETPUBTABLE_FLAG;
		return OK;
	}

	if(strcmp(filename, "ProSetTable.cfg") == 0)
	{
		memcpy(pShareBM->prosettable , buf,len);
		pShareBM->prsetflag |= FILE_PROSETTABLE_FLAG;
		return OK;
	}

	for(i = 0; i < 18; i++)
	{	
		if(i == 0)
			sprintf(cfgname,"fdPubSet.cfg");
		else
			sprintf(cfgname, "fd%02dPubSet.cfg", i+1);

		if(strcmp(filename, cfgname) == 0)
		{
			memcpy(pShareBM->prosetpub[i], buf, len);
			pShareBM->prsetpubflag |= (FILE_FDPUBSET_FLAG << i);
			return OK;
		}
	}

	for(i = 0; i < 18; i++)
	{	
		sprintf(cfgname,"fd%dset.cfg",i+1);
		if(strcmp(filename, cfgname) == 0)
		{
			memcpy(pShareBM->proset[i], buf, len);
			pShareBM->prsetflag |= (FILE_FDSET_FLAG << i);
			return OK;
		}
	}
	
	return ERROR;
}

#include "clock.h"
//从linux共享时钟里面读时间
int rtcInit(void)
{
	TFullDateTime  pdt;

	if(GetLinuxSharedDateTime(&pdt) == PLT_FALSE)
	{
		PRINTF_WITH_LOG("GetLinuxSharedDateTime get error \r\n");
		return -1;
	}
	
	if(SetSystemFullTime(&pdt) == PLT_FALSE)
	{
		PRINTF_WITH_LOG("SetSystemFullTime set error \r\n");
		return -1;
	}

	return 0;
}

void rtcGetTime(struct VCalClock *pTime)
{
	TFullDateTime  dt;
	struct VSysClock time;
	
	dt =  GetSystemFullTime();
	
	time.wYear = dt.y;
	time.byMonth = dt.mon;
	time.byDay = dt.d;
	time.byWeek = 0;
	time.byHour = dt.h;
	time.byMinute = dt.min;
	time.bySecond = dt.s;
	time.wMSecond = dt.us/1000;

	ToCalClock(&time, pTime);
}

void rtcSetTime(struct VCalClock *pTime)
{
	struct VSysClock time;
	TFullDateTime  dt;
	
	CalClockTo(pTime, &time);

	dt.y = time.wYear;
	dt.mon = time.byMonth;
	dt.d = time.byDay;
	dt.h = time.byHour;
	dt.min = time.byMinute;
	dt.s = time.bySecond;
	dt.us= time.wMSecond*1000;

	SetSystemFullTime(&dt);
}


//共享铁电
STATUS extNvRamGet(DWORD offset, BYTE *buf, int len)
{
	uint32* nvram = GetSharedNvRam();
	BYTE *nvbuf = (BYTE *)nvram;
	
	if (IsSharedMemoryValid() == PLT_FALSE)
		return ERROR;

	memcpy(buf, nvbuf + offset, len);
	
	return(OK);
}

STATUS extNvRamSet(DWORD offset, BYTE *buf, int len)
{
	uint32* nvram = GetSharedNvRam();
	BYTE *nvbuf = (BYTE *)nvram;
	
	
	if (IsSharedMemoryValid() == PLT_FALSE)
		return ERROR;
	
	memcpy(nvbuf + offset, buf, len);
	
	return(OK);
}

void WriteRangeYCF_L(WORD wEqpID, WORD no, long value)
{
	long* pycvalue = GetSharedBMYC(wEqpID); 
	
	if((!IsSharedBMMemValid()) || (no > (MAX_YC_NUM-1)) || (wEqpID > (MAX_TEQP_NUM - 1)))
		return;
	
	pycvalue[no] = value;
}

void ReadRangeYCF_L(WORD wEqpID, WORD no, long* value)
{
	long* pycvalue = GetSharedBMYC(wEqpID); 
	
	if((!IsSharedBMMemValid()) || (no > (MAX_YC_NUM-1)) || (wEqpID > (MAX_TEQP_NUM - 1)))
		return;
	
	*value = pycvalue[no];
}

void WriteSYX(WORD wEqpID, WORD no, BYTE value)
{
	BYTE* psyxvalue = GetSharedBMSYX(wEqpID); 
	if((!IsSharedBMMemValid()) || (no > (MAX_SYX_NUM-1)) || (wEqpID > (MAX_TEQP_NUM - 1)))
		return;

	psyxvalue[no] = value;
}

void ReadSYX(WORD wEqpID,WORD no, BYTE* value)
{
	BYTE* psyxvalue = GetSharedBMSYX(wEqpID); 
	if((!IsSharedBMMemValid()) || (no > (MAX_SYX_NUM-1)) || (wEqpID > (MAX_TEQP_NUM - 1)))
		return;

	*value = psyxvalue[no];
}

void WriteMyDiValue(WORD no,  WORD value)
{
	WORD* pdivalue = GetSharedDiValue(); 
	if((!IsSharedBMMemValid()) || (no > 20))
		return;

	pdivalue[no] = value;
}

int WriteMyMyExtDiValue(WORD value)
{
	WORD* pdivalue = GetSharedDiValue(); 

	if(!IsSharedBMMemValid())
		return ERROR;

	pdivalue[19] = value;
	return OK;
}


void WriteDYX(WORD wEqpID,WORD no, BYTE value1,BYTE value2)
{
	WORD* pdyxvalue = GetSharedBMDYX(wEqpID);
	if((!IsSharedBMMemValid()) || (no > (MAX_DYX_NUM-1)) || (wEqpID > (MAX_TEQP_NUM - 1)))
		return;
	pdyxvalue[no] = (value1 << 8) | value2;
}

void WriteSSOE(WORD wEqpID,WORD no, BYTE value, struct VCalClock tm)
{
	TMC_SHARED_SOE* soe = GetSharedBMSoe();
	TMC_SHARED_SOE* psoe;
	if(!IsSharedBMMemValid()|| (wEqpID > (MAX_TEQP_NUM - 1)))
		return;
	
	psoe = soe + wEqpID;

	WriteSYX(wEqpID, no, value);
	
	psoe->soe[psoe->wp].type = 0;
	psoe->soe[psoe->wp].wNo = no;
	psoe->soe[psoe->wp].byValue = value;
	psoe->soe[psoe->wp].Time.dwMinute = tm.dwMinute;
	psoe->soe[psoe->wp].Time.wMSecond = tm.wMSecond;
	psoe->wp = (psoe->wp+1)&(MAX_SOE_NUM-1);
}

void WriteDSOE(WORD wEqpID,WORD no, BYTE value1,BYTE value2, struct VCalClock tm)
{
	TMC_SHARED_SOE* soe = GetSharedBMSoe();
	TMC_SHARED_SOE* psoe;
	if(!IsSharedBMMemValid())
		return;
	psoe = soe + wEqpID;
	WriteDYX(wEqpID,no, value1, value2);
	psoe->soe[psoe->wp].type = 1;
	psoe->soe[psoe->wp].wNo = no;
	psoe->soe[psoe->wp].byValue = (value1 << 8) | value2;
	psoe->soe[psoe->wp].Time.dwMinute = tm.dwMinute;
	psoe->soe[psoe->wp].Time.wMSecond = tm.wMSecond;
	psoe->wp = (psoe->wp+1)&(MAX_SOE_NUM-1);
}

BYTE* ReadRecordBuf(int *len)
{
	VBMSHARERECORD* precord = GetSharedReCord();
	if(!IsSharedBMMemValid())
		return NULL;
	*len= MAX_RECORD_DATA_LEN;
	return precord->record[precord->wp].record;
}

int WriteBmRecord(WORD fd, WORD smpfreq,WORD smpnum, struct VCalClock tm,struct VCalClock prtm)
{
	VBMSHARERECORD* precord = GetSharedReCord();
	if(!IsSharedBMMemValid())
		return ERROR;
	precord->record[precord->wp].fd = fd;
	precord->record[precord->wp].smpnum = smpnum;
	precord->record[precord->wp].smpfreq = smpfreq;
	precord->record[precord->wp].time.dwMinute = tm.dwMinute;
	precord->record[precord->wp].time.wMSecond = tm.wMSecond;
	precord->record[precord->wp].pretime.dwMinute = prtm.dwMinute;
	precord->record[precord->wp].pretime.wMSecond = prtm.wMSecond;

	
	precord->wp = (precord->wp+1)&(MAX_RECORD_NUM-1);
	return MAX_RECORD_DATA_LEN;
}

int WriteBmEvent(int flag,BYTE* buf, int len)
{
	VBMSHAREEVENT* pevent = GetSharedEvent();
	if(!IsSharedBMMemValid())
		return ERROR;
	
	if(len > MAX_EVENT_DATA_LEN)
		len = MAX_EVENT_DATA_LEN;
	pevent->eventdata[pevent->wp].flag = flag;
	pevent->eventdata[pevent->wp].len = len;
	memcpy(pevent->eventdata[pevent->wp].data, buf, len);
	
	pevent->wp = (pevent->wp+1)&(MAX_EVENT_NUM-1);
	
	return OK;
}

int WriteMyioType(DWORD dwAIType ,DWORD dwIOType)
{
	TSHARED_BAREMETAL_STRUCT *pSharedBM = GetSharedBMMemory();
	if(!IsSharedBMMemValid())
		return ERROR;

	pSharedBM->dwAIType = dwAIType;
	pSharedBM->dwIOType = dwIOType;
	return OK;
}

void ReadLinuxLed(DWORD *mask, DWORD* val)
{
	TSHARED_LINUX_STRUCT *pSharedLinux = GetSharedLinuxMemory();
	
	if(!IsSharedLinuxMemValid())
		return ;

	*mask = pSharedLinux->ledmask;
	*val = pSharedLinux->ledvalue;
}

void DoBmUrgency(void)
{
	TSHARED_BAREMETAL_STRUCT *pSharedBM = GetSharedBMMemory();
	if(!IsSharedBMMemValid())
		return ERROR;

	if(pSharedBM->dwEmergencyflag & 0x01) //bit0 代表 录波
	{
		doreadchnbuf();
		pSharedBM->dwEmergencyflag &= ~(0x01);
	}
}

//从通讯读出来linux核间通讯数据，
//从linux共享读取数据
int BmReadLinux(BYTE* buf, int buflen)
{
    int i;

	i = mc_inner_rcv_from_linux(buf,buflen);
	
    /*if (bmrecv_buf.rp == bmrecv_buf.wp) return 0;

	for (i=0; i<buflen; i++)
	{
	    *(buf+i) = *(bmrecv_buf.buf + bmrecv_buf.rp);	
	    bmrecv_buf.rp = (bmrecv_buf.rp+1)&(BM_RCV_BUF_SIZE-1);
		
		if (bmrecv_buf.rp == bmrecv_buf.wp)  break;
	}*/

	return (i);
}

int BmWriteLinux( BYTE* buf, int len)
{
	int ret = 0;
	//发送到linux

	/*{
		int i;
		printf("BmWriteLinux: \n");
		for(i = 0; i < len;i++)
		{
			if(((i + 1) % 16) == 0)
				printf(" \n");
			printf("%02X ",buf[ i]);
		}
		printf(" \n");
	}*/
	
	if(mc_inner_send_to_linux(buf,len) == PLT_FALSE)
		ret = 0;
	else
		ret = len;
	return ret;
}

static char logmsg[512];
int myprintf(int thid, int attr, const char *fmt, ... )
{
    va_list varg;
	

	va_start( varg, fmt );
	vsprintf(logmsg, fmt, varg );
	va_end( varg );

	PRINTF_WITH_LOG("%s: %s \r\n","tSys",logmsg);  
	
	//后期考虑同步到linux上面
	
	return(0);
}	 

static char shellbuf[512];
void shellprintf( const char *fmt, ... )
{
	va_list varg;
    va_start( varg, fmt );
	vsprintf(shellbuf, fmt, varg );
	va_end( varg );
	PRINTF_WITH_LOG("%s",shellbuf);
}

