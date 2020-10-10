/*
 * @Author: zhongwei
 * @Date: 2020/2/13 15:32:43
 * @Description: 执行BM侧应用
 * @File: app_bmrun.cc
 * 
 * BM启动流程详见文档《软件启动流程.vsdx》
*/

#include "plt_include.h"
#include "app_bmrun.h"
#include "mu_inner_comm.h"


/**
 * @Function: hd_map_memory
 * @Description:  
 * @author zhongwei (2020/2/7 9:58:55)
 * 
 * @param addr      映射内存起始地址
 * @param length    映射内存长度
 * 
 * @return void *   返回映射后的地址
 * 
*/
void * hd_map_memory(uint32 addr, uint32 length, int * pfd)
{
    int fd;
    PRINTF_WITH_LOG("hd_map_memory addr=0x%x length=%d\n\r", addr, length);

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0)
    {
        PRINTF_WITH_LOG("open(/dev/mem) failed (%d)\n\r", errno);
        return NULL;
    }

    int page_size = sysconf(_SC_PAGE_SIZE);     //页大小

    //映射内存必须页对齐
    ASSERT_L1((addr & (page_size-1)) == 0);
    ASSERT_L1((length & (page_size-1)) == 0);

    void * ptr = mmap(NULL,
                      length, 
                      PROT_READ | PROT_WRITE, 
                      MAP_SHARED, 
                      fd,
                      addr);
    if (ptr == MAP_FAILED)
    {
        close(fd);
        return NULL;
    }

    PRINTF_WITH_LOG("hd_map_memory addr=0x%x length=%d -> 0x%x, fd=%d\n\r", addr, length, ptr, fd);

    *pfd = fd;
    return ptr;
}

/**
 * @Function: hd_unmap_memory
 * @Description: unmap内存 
 * @author zhongwei (2020/2/7 10:14:53)
 * 
 * @param map_ptr 
 * @param length 
 * @param fd 
 * 
 * @return STATUS 
 * 
*/
STATUS hd_unmap_memory(void * map_ptr, uint32 length, int fd)
{
    PRINTF_WITH_LOG("hd_unmap_memory addr=0x%x length=%d fd=%d\n\r", map_ptr, length, fd);
    ASSERT_L4(map_ptr != NULL);
    if(munmap(map_ptr, length) ==0)
    {
        close(fd);
        return 0;
    }
    else    //unmap失败，打印失败信息
    {
        perror("hd_unmap_memory:");
        return -1;
    }
}

static int g_fdSharedMemory=0;        //fd保存

/**
 * @Function: PowerOn_Init_SharedMemory
 * @Description: 上电初始化共享内存 
 * @author zhongwei (2019/11/27 14:03:53)
 * 
 * @param void 
 *
 * 共享内存TSHARE_MEMORY_STRUCT由Linux负责初始化
 * 
*/
SECTION_PLT_CODE void PowerOn_Init_SharedMemory(void)
{
    /*
        Linux共享内存初始化
        需要检查是否已经初始化过，如果初始化过了，则不再重新初始化
    */

    extern void * hd_map_memory(uint32 addr, uint32 length, int * pfd);
    int fd;
    const uint32 uSharedStart = DDR_SHART_START_ADDR;
    size_t size_of_shared = sizeof(TSHARE_MEMORY_STRUCT);
    const uint32 uSharedLen = ((size_of_shared / 0x10000) + ((size_of_shared % 0x10000) ==0 ?0:1 )) * 0x10000;
    ASSERT_L4(g_pSharedMemory == NULL);
    void * map_ptr = hd_map_memory(uSharedStart, uSharedLen, &fd);

	printf("PowerOn_Init_SharedMemory  \n");
    if (map_ptr != NULL)
    {
        PRINTF_WITH_LOG("Map Shared Memory 0x%x(0x%x) -> 0x%x\n\r", uSharedStart, uSharedLen, (unsigned int)map_ptr);
		printf("Map Shared Memory 0x%x(0x%x) -> 0x%x\n\r", uSharedStart, uSharedLen, (unsigned int)map_ptr);
        g_pSharedMemory = (TSHARE_MEMORY_STRUCT *)map_ptr;
        g_fdSharedMemory = fd;

        if (CheckSharedMemory() == PLT_FALSE)   //初始化共享内存
        {
            TSHARE_MEMORY_STRUCT *pSharedMemory = GetSharedMemory();
            TSHARED_LINUX_STRUCT *pSharedLinux = GetSharedLinuxMemory();
			
			TSHARED_BAREMETAL_STRUCT *pSharedBM = GetSharedBMMemory();
			
			printf("pSharedLinux %08x  pSharedBM %08x \n\r",(unsigned int)pSharedLinux,(unsigned int)pSharedBM);

			
            if (pSharedMemory != NULL)
            {
                int i;

                PRINTF_WITH_LOG("Initial SharedMemory addr = 0x%x\n\r", (unsigned int)pSharedMemory);
				printf("Initial SharedMemory addr = 0x%x\n\r", (unsigned int)pSharedMemory);
                E_MEMSET(pSharedMemory, 0, sizeof(TSHARE_MEMORY_STRUCT));    //清零先

                //初始化 TSHARED_LINUX_STRUCT
                {
                    pSharedLinux->struct_version = VERSION_SHARED_MEMORY;
                    //有效标志初始化
                    for (i = FLAG_SHARED_COUNT - 1; i >= 0; i--)
                    {
                        pSharedLinux->flag_end[i] = FLAG_SHARED_MEMORY_VALID;
                        pSharedLinux->flag_begin[i] = FLAG_SHARED_MEMORY_VALID;
                    }
                }

                pSharedMemory->struct_version = VERSION_SHARED_MEMORY;
                //有效标志初始化
                for (i = FLAG_SHARED_COUNT - 1; i >= 0; i--)
                {
                    pSharedMemory->flag_end[i] = FLAG_SHARED_MEMORY_VALID;
                    pSharedMemory->flag_begin[i] = FLAG_SHARED_MEMORY_VALID;
                }
            }
        }
    }
    else 
    {
        TRACE_L1("map share memory 0x%x(0x%x) fail.", uSharedStart, uSharedLen);
		printf("map share memory 0x%x(0x%x) fail.", uSharedStart, uSharedLen);
    }

}

#define MAX_FILE_LEN  (128*1024)

//加载配置文件到共享内存
void ParaShareBm()
{
	TSHARED_LINUX_STRUCT *pSharedLinux = GetSharedLinuxMemory();
	char cfgname[64];
	int i;

	pSharedLinux->config_data.flag_begin = FLAG_SHARED_MEMORY_VALID;
	pSharedLinux->config_data.fileflag = 0;
	pSharedLinux->config_data.prsetpubfileflag = 0;
	pSharedLinux->config_data.prsetfileflag = 0;
	sprintf(cfgname,"system.cfg");
	if (ReadParaFile(cfgname,pSharedLinux->config_data.data,MAX_FILE_LEN)==OK) //有此配置文件
	{
		printf("system.cfg into memory \n");
		pSharedLinux->config_data.fileflag |= FILE_SYSTEM_FLAG;
	}

	sprintf(cfgname,"ProSetPubTable.cfg");
	if (ReadParaFile(cfgname,pSharedLinux->config_data.data + FILE_PROSETPUBTABLE_OFFSET,MAX_FILE_LEN)==OK) //有此配置文件
	{
		printf("ProSetPubTable.cfg into memory \n");
		pSharedLinux->config_data.prsetpubfileflag |= FILE_PROSETPUBTABLE_FLAG;
	}

	sprintf(cfgname,"ProSetTable.cfg");
	if (ReadParaFile(cfgname,pSharedLinux->config_data.data + FILE_PROSETTABLE_OFFSET,MAX_FILE_LEN)==OK) //有此配置文件
	{
		printf("ProSetTable.cfg into memory \n");
		pSharedLinux->config_data.prsetfileflag |= FILE_PROSETTABLE_FLAG;
	}

	for(i = 0; i < 18; i++)
	{	
		if(i == 0)
			sprintf(cfgname,"fdPubSet.cfg");
		else
			sprintf(cfgname, "fd%02dPubSet.cfg", i+1);
		
		if (ReadParaFile(cfgname,pSharedLinux->config_data.data + FILE_FDPUBSET_OFFSET + i*PR_SET_SIZE,MAX_FILE_LEN)==OK) //有此配置文件
		{
			printf("%s into memory \n",cfgname);
			pSharedLinux->config_data.prsetpubfileflag |= (FILE_FDPUBSET_FLAG << i);
		}
	}

	for(i = 0; i < 18; i++)
	{	
		sprintf(cfgname,"fd%dset.cfg",i+1);
		if (ReadParaFile(cfgname,pSharedLinux->config_data.data + FILE_FDSET_OFFSET + i*PR_SET_SIZE,MAX_FILE_LEN)==OK) //有此配置文件
		{
			printf("%s into memory \n",cfgname);
			pSharedLinux->config_data.prsetfileflag |= (FILE_FDSET_FLAG << i);
		}
	}

	//runpara.cfg
	sprintf(cfgname,"runpara.cfg");
	if (ReadParaFile(cfgname,pSharedLinux->config_data.data + FILE_RUNPARA_OFFSET,FILE_RUNPARA_LEN)==OK) //有此配置文件
	{
		printf("runpara.cfg into memory \n");
		pSharedLinux->config_data.fileflag |= FILE_RUNPARA_FLAG;
	}

	//systemext.cfg
	sprintf(cfgname,"systemext.cfg");
	if (ReadParaFile(cfgname,pSharedLinux->config_data.data + FILE_SYSEXT_OFFSET,FILE_SYSEXT_LEN)==OK) //有此配置文件
	{
		printf("systemext.cfg into memory \n");
		pSharedLinux->config_data.fileflag |= FILE_SYSEXT_FLAG;
	}

	//ccd文件
	i = ReadCcdFile(pSharedLinux->config_data.data + FILE_CCD_OFFSET + 4,FILE_CCD_LEN); //特殊处理,前4个为长度
	if (i != ERROR) //有此配置文件
	{
		printf("ccd.cfg into memory \n");
		memcpy(pSharedLinux->config_data.data + FILE_CCD_OFFSET, &i ,4);
		pSharedLinux->config_data.fileflag |= FILE_CCD_FLAG;
	}
	
	pSharedLinux->config_data.flag_end = FLAG_SHARED_MEMORY_VALID;
}

//BM 写定值文件，到linux。当没有配置文件时，裸核自动生成定值模板文件
void BMWritePrSetTable(void)
{
	//查找
	TSHARED_BAREMETAL_STRUCT *pShareBM = GetSharedBMMemory();
	char fname[64];
	DWORD prsetpubflag,prsetflag;
	int i;
	
	if(!IsSharedBMMemValid())
		return;

	prsetpubflag = pShareBM->prsetpubflag;
	prsetflag = pShareBM->prsetflag;
	
	if(!(prsetpubflag || prsetflag))
		return;
		
	if(prsetpubflag & FILE_PROSETPUBTABLE_FLAG)
	{
		sprintf(fname, "ProSetPubTable.cfg");
		if(WriteParaProFile(fname, pShareBM->prosetpubtable) == OK)
		{
			pShareBM->prsetpubflag &= ~FILE_PROSETPUBTABLE_FLAG;
		}
		printf(" %s write %s \r\n",__FUNCTION__,fname);
	}

	for(i = 0; i < 18; i++)
	{
		if(prsetpubflag & (FILE_FDPUBSET_FLAG << i))
		{
			if(i == 0)
				sprintf(fname,"fdPubSet.cfg");
			else
				sprintf(fname, "fd%02dPubSet.cfg", i+1);
			if(WriteParaProFile(fname, pShareBM->prosetpub[i]) == OK)
			{
				pShareBM->prsetpubflag &= ~(FILE_FDPUBSET_FLAG << i);
			}
			printf(" %s write %s \r\n",__FUNCTION__,fname);
		}
	}

	if(prsetflag & FILE_PROSETTABLE_FLAG)
	{
		sprintf(fname, "ProSetTable.cfg");
		if(WriteParaProFile(fname, pShareBM->prosettable) == OK)
		{
			pShareBM->prsetflag &= ~FILE_PROSETTABLE_FLAG;
		}
		printf(" %s write %s \r\n",__FUNCTION__,fname);
	}
	
	for(i = 0; i < 18; i++)
	{
		if(prsetflag & (FILE_FDSET_FLAG << i))
		{
			sprintf(fname,"fd%dset.cfg",i+1);
			if(WriteParaProFile(fname, pShareBM->proset[i]) == OK)
			{
				pShareBM->prsetflag &= ~(FILE_FDSET_FLAG << i);
			}
			printf(" %s write %s \r\n",__FUNCTION__,fname);
		}
	}
}

extern SECTION_PLT_CODE void SetLinuxSharedDateTime(void);
void BMinit(void)
{
	char cfgname[64];

	//加载配置文件到共享内存
	ParaShareBm();
	
	//设置linux共享裸核时间
	SetLinuxSharedDateTime();
	sprintf(cfgname,"/mnt/emmc_app/zynqBM.bin");
	bmrun(cfgname);
}

#include "clock.h"
#include <sys/time.h>
#include <time.h>

//获取BM共享的时间
SECTION_PLT_CODE BOOL GetBMSharedDateTime(TFullDateTime * pdt)
{
    if (IsSharedBMMemValid())
    {
        const TMC_SHARED_TIME * pBmTime = GetSharedBMTime();
        int i;
        for (i=0;i<3;i++)   //尝试三次
        {
            TFullDateTime dt1 = pBmTime->fdt1;
            TFullDateTime dt2 = pBmTime->fdt2;

			//check
			if(CheckFullTime(&dt1) == PLT_FALSE)
			{
				printf("GetBMSharedDateTime time  check error \n");
				return PLT_FALSE;
			}
			
            if (memcmp(&dt1, &dt2, sizeof(TFullDateTime)) == 0)
            {
                *pdt = dt1;
                return PLT_TRUE;
            }
        }
    }
    return PLT_FALSE;
}


//设置Linux共享时间
SECTION_PLT_CODE void SetLinuxSharedDateTime(void)
{	
	struct VSysClock systm;
	struct timeval t;
	struct VCalClock vTime;
	
	//直接取linux时间，防止干扰冲突
	gettimeofday(&t,NULL);
	t.tv_sec += 28800; 
	vTime.dwMinute = (DWORD)(t.tv_sec/60);
	vTime.wMSecond = (WORD)(t.tv_sec%60*1000 + t.tv_usec/1000);
	CalClockTo(&vTime, &systm);
	
    if (IsSharedLinuxMemValid())
    {
        TMC_SHARED_TIME *pSharedTime = GetSharedLinuxTime();

		pSharedTime->fdt1.y = systm.wYear;
		pSharedTime->fdt1.mon = systm.byMonth;
		pSharedTime->fdt1.d = systm.byDay;

		pSharedTime->fdt1.h = systm.byHour;
		pSharedTime->fdt1.min = systm.byMinute;
		pSharedTime->fdt1.s = systm.bySecond;
		pSharedTime->fdt1.us = systm.wMSecond*1000;
        memcpy( &pSharedTime->fdt2, &pSharedTime->fdt1, sizeof(pSharedTime->fdt1));
		//printf("%d %d %d %d %d %d \n",systm.wYear,systm.byMonth,systm.byDay,systm.byHour,systm.byMinute,systm.bySecond);
		//printf("%08x  %08x \n",pSharedTime,GetSharedLinuxMemory());
    }
}

TFullDateTime GetSystemFullTime(void)
{
    TFullDateTime fdt;
   	GetBMSharedDateTime(&fdt); 
    return fdt;
}

int bmrunflag = 0;
//刚开始是获取的是linux时间，等裸核定时器跑起来了，取的是裸核时间
void GetTime(struct VCalClock *pTime)
{
	TFullDateTime  dt;
	struct VSysClock time;
	struct timeval t;
	
	if(bmrunflag && (GetBMSharedDateTime(&dt) == PLT_TRUE)) //获取BM共享时间，定时器准
	{
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
	else
	{
		//get linux本身时间
		gettimeofday(&t,NULL);
		t.tv_sec += 28800; 
		pTime->dwMinute = (DWORD)(t.tv_sec/60);
		pTime->wMSecond = (WORD)(t.tv_sec%60*1000 + t.tv_usec/1000);
	}
}

extern int SetBMClock(struct VSysClock* systm);
//心跳,初始化时测试裸核是否启动，并对时
void SetBMTime(void)
{
	TFullDateTime  dt;
	struct VSysClock SysTime;
	static TFullDateTime  olddt = {0};
	DWORD time,oldtime,timesub;
	//读出时间，两次不一样则说明裸核定时器已启动
	if(bmrunflag)
		return;

	if(GetBMSharedDateTime(&dt) == PLT_TRUE) //获取BM共享时间，定时器准
	{
		time = dt.h*3600000 + dt.min*60*1000 + dt.s*1000 + dt.us/1000;
		oldtime = olddt.h*3600000 + olddt.min*60*1000 + olddt.s*1000 + olddt.us/1000;

		timesub = time - oldtime;
		if((timesub < 1100) && (timesub > 900))
		{
			
			GetSysClock((void *)&SysTime, SYSCLOCK);	
			if(SetBMClock(&SysTime) == OK)
			{
				bmrunflag = 1;
			}
			printf("check bm is runing! \r\n");
		}
		memcpy(&olddt , &dt,sizeof(TFullDateTime));
	}

}

void SetTime(struct VCalClock tm)
{
	struct VSysClock systm;
	struct VSysClock systm1;
	struct VCalClock tm1;
	char buf[100];	

	tm1 = tm;
	tm.dwMinute = tm.dwMinute - 8*60; ////去掉8个小时,utc时间
	CalClockTo(&tm,&systm);
	sprintf(buf,"date -s \"%04d-%0d-%d %d:%d:%d\" ",systm.wYear,systm.byMonth,systm.byDay,systm.byHour,systm.byMinute,systm.bySecond);
	system(buf);
	system("hwclock -w");

	//通讯设置BM时间
	if (IsSharedBMMemValid()) //裸核已启动
	{
		//发送对时报文
		CalClockTo(&tm1,&systm1);
		SetBMClock(&systm1);
	}
}

//获取共享遥测值
int ReadBMRangeYCF_L(WORD wEqpID,WORD no, long* value)
{
	long* pycvalue = GetSharedBMYC(wEqpID); 
	
	if((!IsSharedBMMemValid()) || (no > (MAX_YC_NUM-1)) || (wEqpID > (MAX_TEQP_NUM - 1)))
		return ERROR;
	
	*value = pycvalue[no];
	return OK;
}

int ReadBMSYX(WORD wEqpID,WORD no, BYTE* value)
{
	BYTE* psyxvalue = GetSharedBMSYX(wEqpID); 
	if((!IsSharedBMMemValid()) || (no > (MAX_SYX_NUM-1)) || (wEqpID > (MAX_TEQP_NUM - 1)))
		return ERROR;

	*value = psyxvalue[no];
	return OK;
}

int ReadBMMyDiValue(WORD no,  WORD* value)
{
	WORD* pdivalue = GetSharedDiValue(); 
	if((!IsSharedBMMemValid()) || (no > 19))
		return ERROR;

	*value = pdivalue[no];
	return OK;
}

int ReadBMDYX(WORD wEqpID,WORD no, BYTE* value1,BYTE* value2)
{
	WORD* pdyxvalue = GetSharedBMDYX(wEqpID);
	if((!IsSharedBMMemValid()) || (no > (MAX_DYX_NUM-1)) || (wEqpID > (MAX_TEQP_NUM - 1)))
		return ERROR;

	*value1 = (pdyxvalue[no] >> 8) & 0xFF;
	*value2 = pdyxvalue[no] & 0xFF;
	return OK;
}

int ReadBMSOE(WORD wEqpID, WORD *no, WORD *value, struct VCalClock *tm)
{
	TMC_SHARED_SOE* soe = GetSharedBMSoe();
	TMC_SHARED_SOE* psoe;
	WORD wp;
	BYTE type;
	if((!IsSharedBMMemValid()) || (wEqpID > (MAX_TEQP_NUM - 1)))
		return ERROR;
	psoe = soe + wEqpID;
	wp = psoe->wp;
	if(psoe->rp != wp)
	{
		*no = psoe->soe[psoe->rp].wNo;
		*value = psoe->soe[psoe->rp].byValue;
		tm->dwMinute = psoe->soe[psoe->rp].Time.dwMinute;
		tm->wMSecond = psoe->soe[psoe->rp].Time.wMSecond;
		type = psoe->soe[psoe->rp].type;
		psoe->rp = (psoe->rp+1)&(MAX_SOE_NUM-1);
		return type;
	}
	return ERROR;
}

BYTE GetMyExtDiValue(int no)
{
	WORD* pdivalue = GetSharedDiValue(); 
	WORD extdivalue;
	WORD val=0x01;

	if(!IsSharedBMMemValid())
		return 0;

	extdivalue = pdivalue[19];
    if ((no < 0)||(no >= 16))  return 0;

	val = val<<no;
	//printf("%d  %08x  %08x %d \n", no , extdivalue,val ,extdivalue&val);
	if (extdivalue&val) return 1;
	else return 0;
}

int ReadBmRecord(WORD* fd)
{
	VBMSHARERECORD* precord = GetSharedReCord();
	if(!IsSharedBMMemValid())
		return ERROR;
	
	if(precord->wp == precord->rp)
		return ERROR;

	*fd = precord->record[precord->rp].fd;
	return OK;
}

BYTE* ReadBmRecordData(WORD* smpfreq,WORD* smpnum, struct VCalClock* tm,struct VCalClock* prtm)
{
	VBMSHARERECORD* precord = GetSharedReCord();
	WORD rp;
	if(!IsSharedBMMemValid())
		return NULL;
	
	if(precord->wp == precord->rp)
		return NULL;

	rp = precord->rp;
	*smpnum = precord->record[precord->rp].smpnum;
	*smpfreq = precord->record[precord->rp].smpfreq;
	tm->dwMinute = precord->record[precord->rp].time.dwMinute;
	tm->wMSecond = precord->record[precord->rp].time.wMSecond;
	prtm->dwMinute = precord->record[precord->rp].pretime.dwMinute;
	prtm->wMSecond = precord->record[precord->rp].pretime.wMSecond;

	precord->rp = (precord->rp+1)&(MAX_RECORD_NUM-1);
	return precord->record[rp].record;
}

int ReadBmEventFlag()
{
	VBMSHAREEVENT* pevent = GetSharedEvent();
	if(!IsSharedBMMemValid())
		return ERROR;

	if(pevent->wp == pevent->rp)
		return ERROR;
	
	return pevent->eventdata[pevent->rp].flag;
}

int ReadBmEvent(BYTE* buf, int len)
{
	VBMSHAREEVENT* pevent = GetSharedEvent();
	int flag;
	
	if(!IsSharedBMMemValid())
		return ERROR;

	if(pevent->wp == pevent->rp)
		return ERROR;
	
	if(len > MAX_EVENT_DATA_LEN)
		len = MAX_EVENT_DATA_LEN;
	
	memcpy(buf ,pevent->eventdata[pevent->rp].data, len);
	flag = pevent->eventdata[pevent->rp].flag;
	pevent->rp = (pevent->rp+1)&(MAX_EVENT_NUM-1);
	
	return flag;
}

int ReadMyioType(DWORD* dwAIType ,DWORD* dwIOType)
{
	TSHARED_BAREMETAL_STRUCT *pSharedBM = GetSharedBMMemory();
	if(!IsSharedBMMemValid())
		return ERROR;

	*dwAIType = pSharedBM->dwAIType;
	*dwIOType = pSharedBM->dwIOType;
	return OK;
}

void SetLinuxLed(int id, BYTE on)
{
	TSHARED_LINUX_STRUCT *pSharedLinux = GetSharedLinuxMemory();
	DWORD mask = (0x01<<id);
	if(!IsSharedLinuxMemValid())
		return ;

	pSharedLinux->ledmask |= mask;	
	if (on)  pSharedLinux->ledvalue |= mask;
	else pSharedLinux->ledvalue &= ~mask;
}

void SetBmUrgency(WORD bit)
{
	TSHARED_BAREMETAL_STRUCT *pSharedBM = GetSharedBMMemory();
	if(!IsSharedBMMemValid())
		return;

	pSharedBM->dwEmergencyflag |= bit; //bit0 代表 录波
}


//共享铁电
STATUS extNvRamGetBM(DWORD offset, BYTE *buf, int len)
{
	uint32* nvram = GetSharedNvRam();
	BYTE *nvbuf = (BYTE *)nvram;
	
	if (IsSharedMemoryValid() == PLT_FALSE)
		return ERROR;

	memcpy(buf, nvbuf + offset, len);
	
	return(OK);
}

STATUS extNvRamSetBM(DWORD offset, BYTE *buf, int len)
{
	uint32* nvram = GetSharedNvRam();
	BYTE *nvbuf = (BYTE *)nvram;
	
	if (IsSharedMemoryValid() == PLT_FALSE)
		return ERROR;
	
	memcpy(nvbuf + offset, buf, len);
	
	return(OK);
}



