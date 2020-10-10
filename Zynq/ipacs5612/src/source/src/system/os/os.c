/*------------------------------------------------------------------------
 Module:		os.c
 Author:		solar
 Project:		 
 Creation Date: 2008-8-14
 Description:	
------------------------------------------------------------------------*/

#include "syscfg.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "os.h"
#include "bsp.h"
#include "file.h"

int COMM_NUM, THREAD_MAX_NUM;
VThread *g_Thread;	                     /*OS抽象层线程信息*/

static int thWdg_Enable;

WORD crccode[0x100]={
 0X0   ,  0Xc0c1,  0Xc181,  0X140,   0Xc301,  0X3c0,   0X280,   0Xc241,
 0Xc601,  0X6c0,   0X780,   0Xc741,  0X500,   0Xc5c1,  0Xc481,  0X440,
 0Xcc01,  0Xcc0,   0Xd80,   0Xcd41,  0Xf00,   0Xcfc1,  0Xce81,  0Xe40,
 0Xa00 ,  0Xcac1,  0Xcb81,  0Xb40 ,  0Xc901,  0X9c0,   0X880 ,  0Xc841,
 0Xd801,  0X18c0,  0X1980,  0Xd941,  0X1b00,  0Xdbc1,  0Xda81,  0X1a40,
 0X1e00,  0Xdec1,  0Xdf81,  0X1f40,  0Xdd01,  0X1dc0,  0X1c80,  0Xdc41,
 0X1400,  0Xd4c1,  0Xd581,  0X1540,  0Xd701,  0X17c0,  0X1680,  0Xd641,
 0Xd201,  0X12c0,  0X1380,  0Xd341,  0X1100,  0Xd1c1,  0Xd081,  0X1040,
 0Xf001,  0X30c0,  0X3180,  0Xf141,  0X3300,  0Xf3c1,  0Xf281,  0X3240,
 0X3600,  0Xf6c1,  0Xf781,  0X3740,  0Xf501,  0X35c0,  0X3480,  0Xf441,
 0X3c00,  0Xfcc1,  0Xfd81,  0X3d40,  0Xff01,  0X3fc0,  0X3e80,  0Xfe41,
 0Xfa01,  0X3ac0,  0X3b80,  0Xfb41,  0X3900,  0Xf9c1,  0Xf881,  0X3840,
 0X2800,  0Xe8c1,  0Xe981,  0X2940,  0Xeb01,  0X2bc0,  0X2a80,  0Xea41,
 0Xee01,  0X2ec0,  0X2f80,  0Xef41,  0X2d00,  0Xedc1,  0Xec81,  0X2c40,
 0Xe401,  0X24c0,  0X2580,  0Xe541,  0X2700,  0Xe7c1,  0Xe681,  0X2640,
 0X2200,  0Xe2c1,  0Xe381,  0X2340,  0Xe101,  0X21c0,  0X2080,  0Xe041,
 0Xa001,  0X60c0,  0X6180,  0Xa141,  0X6300,  0Xa3c1,  0Xa281,  0X6240,
 0X6600,  0Xa6c1,  0Xa781,  0X6740,  0Xa501,  0X65c0,  0X6480,  0Xa441,
 0X6c00,  0Xacc1,  0Xad81,  0X6d40,  0Xaf01,  0X6fc0,  0X6e80,  0Xae41,
 0Xaa01,  0X6ac0,  0X6b80,  0Xab41,  0X6900,  0Xa9c1,  0Xa881,  0X6840,
 0X7800,  0Xb8c1,  0Xb981,  0X7940,  0Xbb01,  0X7bc0,  0X7a80,  0Xba41,
 0Xbe01,  0X7ec0,  0X7f80,  0Xbf41,  0X7d00,  0Xbdc1,  0Xbc81,  0X7c40,
 0Xb401,  0X74c0,  0X7580,  0Xb541,  0X7700,  0Xb7c1,  0Xb681,  0X7640,
 0X7200,  0Xb2c1,  0Xb381,  0X7340,  0Xb101,  0X71c0,  0X7080,  0Xb041,
 0X5000,  0X90c1,  0X9181,  0X5140,  0X9301,  0X53c0,  0X5280,  0X9241,
 0X9601,  0X56c0,  0X5780,  0X9741,  0X5500,  0X95c1,  0X9481,  0X5440,
 0X9c01,  0X5cc0,  0X5d80,  0X9d41,  0X5f00,  0X9fc1,  0X9e81,  0X5e40,
 0X5a00,  0X9ac1,  0X9b81,  0X5b40,  0X9901,  0X59c0,  0X5880,  0X9841,
 0X8801,  0X48c0,  0X4980,  0X8941,  0X4b00,  0X8bc1,  0X8a81,  0X4a40,
 0X4e00,  0X8ec1,  0X8f81,  0X4f40,  0X8d01,  0X4dc0,  0X4c80,  0X8c41,
 0X4400,  0X84c1,  0X8581,  0X4540,  0X8701,  0X47c0,  0X4680,  0X8641,
 0X8201,  0X42c0,  0X4380,  0X8341,  0X4100,  0X81c1,  0X8081,  0X4040,
};

DWORD  crctable[256] =
{
 0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL,
 0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
 0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L,
 0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
 0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL,
 0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
 0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL,
 0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
 0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L,
 0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
 0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L,
 0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
 0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L,
 0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
 0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L,
 0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
 0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL,
 0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
 0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L,
 0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
 0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL,
 0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
 0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL,
 0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
 0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L,
 0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
 0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L,
 0x44042D73L, 0x33031DE5L, 0xAA0A4C5FL, 0xDD0D7CC9L,
 0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L,
 0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
 0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L,
 0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
 0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL,
 0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
 0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L,
 0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
 0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL,
 0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
 0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL,
 0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
 0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L,
 0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
 0xD80D2BDAL, 0xAF0A1B4CL, 0x36034AF6L, 0x41047A60L,
 0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL, 0x4669BE79L,
 0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L,
 0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
 0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L,
 0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
 0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL,
 0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
 0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L,
 0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
 0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL,
 0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
 0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL,
 0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
 0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L,
 0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
 0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L,
 0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
 0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L,
 0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
 0xB3667A2EL, 0xC4614AB8L, 0x5D681B02L, 0x2A6F2B94L,
 0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
};

#define INCLUDE_SYS_LOG
#ifdef INCLUDE_SYS_LOG
VLog sysLog;  
static char logmsg[SYS_LOG_MSGLEN];

/*------------------------------------------------------------------------
 Procedure:     LogMsgInit ID:1
 Purpose:       日志信息初始化
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/	
void LogMsgInit(void)
{
    int i;
	
	sysLog.dwSem = smMCreate();

	sysLog.wWrtPtr=0;
	for (i=0; i<SYS_LOG_BUFNUM; i++)
	    sysLog.aLogMsg[i].thid = -1;     
}

void writelog(char* name, const char* sMsg)
{
	char filename[64];
	char temp[32];
	char enter[] = "\r\n";
	FILE *fp;
	static BOOL first = 1;

	sprintf(filename,"%s/event/other/%sapp.runinfo",SYS_PATH_ROOT2,name);

	if(first == 1)
	{
		first = 0;
		fp = fopen(filename, "w");
	}
	else
		fp = fopen(filename, "a+");

	if(fp==NULL)
	{
		return;
	}
	sprintf(temp,"%s: ",name);
	fwrite(temp,1,strlen(temp), fp);
	fwrite(sMsg, 1, strlen(sMsg), fp);
	fwrite(enter, 1, strlen(enter), fp);
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
}

/*------------------------------------------------------------------------
 Procedure:     Login ID:1
 Purpose:       登记系统信息
 Input:         tid:任务号  
                sMsg:信息
 Output:        
 Errors:
------------------------------------------------------------------------*/
void Login(int thid,int attr, char *sMsg)
{
	//int i;
	DWORD len;
	char *msg;
	BOOL pCR;

	sysLog.aLogMsg[sysLog.wWrtPtr].thid = thid;	
	sysLog.aLogMsg[sysLog.wWrtPtr].attr = attr;

	msg=sysLog.aLogMsg[sysLog.wWrtPtr].sMsg;
	len = strlen(sMsg);
    len = (len>SYS_LOG_MSGLEN) ? SYS_LOG_MSGLEN-1:len;	
	memcpy(msg, sMsg, len);
	msg[len] = '\0';
		
	sysLog.wWrtPtr = (sysLog.wWrtPtr+1)&(SYS_LOG_BUFNUM-1);   
	if (sMsg[strlen(sMsg)-1] == '\n')
	    pCR=FALSE;
	else
	    pCR=TRUE;  

	printf("%s: %s \n",GetThName(thid),sMsg);  
	
	if (pCR) printf("\n");

	writelog(GetThName(SYS_ID),sMsg);
	
}  

int myprintf(int thid, int attr, const char *fmt, ... )
{
    int ret;
    va_list varg;

    ret = smMTake(sysLog.dwSem);
	if (ret)
		return (ERROR);

	va_start( varg, fmt );
	ret = vsprintf(logmsg, fmt, varg );
	if(ret < 0)
		return (ERROR);
	va_end( varg );

	Login(thid, attr, logmsg);

	ret = smMGive(sysLog.dwSem);
	if (ret)
		return (ERROR);

	return (OK);
}	 

int logshow(int log_attr)
{
	int i;
	struct VLogMsg *pLogMsg;
	BOOL pCR;

	/*clrscr();*/

	pLogMsg=sysLog.aLogMsg;

	for (i=0;i<SYS_LOG_BUFNUM;i++)
	{
		if (pLogMsg->thid == -1)  break;

		if (pLogMsg->attr >= log_attr)
		{
			if (pLogMsg->sMsg[strlen(pLogMsg->sMsg)-1] == '\n')
				pCR=FALSE;
			else
				pCR=TRUE;  

			/*vt100color("32");*/
			printf("%s: %s\n",GetThName(pLogMsg->thid), pLogMsg->sMsg);  
			/*vt100color("30");*/

			if (pCR) printf("\n");
		}

		pLogMsg++;
	}   

	return(OK);
}  

int runlog(void)
{
	return(logshow(LOG_ATTR_INFO));
}  

int errlog(void)
{
	return(logshow(LOG_ATTR_WARN));
}  

#else

void LogMsgInit(void)
{
}

void Login(int thid,int attr, char *sMsg)
{
}

int myprintf(int thid, int attr, const char *fmt, ... )
{
}

int logshow(int log_attr)
{
}

int runlog(void)
{
}

int errlog(void)
{
}

#endif

/*------------------------------------------------------------------------
 Procedure:	GetStrFromMyFormatStr
 Purpose:   取得str中ch所分割后所有对应的字串地址
            注意:str会改变,因此如果str为有用参数则不能直接传递调用,而应
            该用其复制串调用            
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/    
void GetStrFromMyFormatStr(char *str, int ch, int num, ...)
{
    char *p, *q;
	int i;
    va_list argptr;
	char ** arg;

	va_start(argptr, num);

    q = str;
	for (i=0; i<num; i++)
	{
        arg = va_arg(argptr, char **);

		*arg = NULL;

        if (q == NULL) break;

		p = strchr(q, ch);
		
		*arg = q;

		if (p == NULL) q = NULL;
		else
		{
			*p = '\0';
			q = p+1;		
		}	
	}

	va_end(argptr);
}

/*------------------------------------------------------------------------
 Procedure:     ThreadInit ID:1
 Purpose:       初始化抽象层环境
 Input:         
 Output:		>=0: thid; <0: ERROR
 Errors:
------------------------------------------------------------------------*/
int ThreadInit(const char *name)
{
    int ret;
	
	COMM_NUM = GetProfileInt("系统", "通信口数目(含socket)", 30, "system.cin"); 

	THREAD_MAX_NUM = THREAD_MIN_NUM+COMM_NUM;
	
	g_Thread = (VThread *)malloc(sizeof(VThread)*THREAD_MAX_NUM);
	if (g_Thread == NULL) return ERROR;
	
	memset(g_Thread, 0, sizeof(VThread)*THREAD_MAX_NUM);
	ret = ThreadReq(SYS_ID,name);
	if (ret)
		return ERROR;
	
	LogMsgInit();
	sysVerCrcShow(SYS_ID);
	EnableThDog();
	return OK;
}
/*------------------------------------------------------------------------
 Procedure:     ThreadReq ID:1
 Purpose:       向抽象层申请线程号
 Input:         
 Output:		>=0: thid; <0: ERROR
 Errors:
------------------------------------------------------------------------*/
int ThreadReq(int thid, const char *name)
{
	if(thid >= THREAD_MAX_NUM) 
	{
		printf("thid >= THREAD_MAX_NUM :%d %d\n",thid ,THREAD_MAX_NUM);
		return ERROR;
	}
    if(g_Thread[thid].used != 0) 
	{
		printf("g_Thread[%d].used %d\n", thid, g_Thread[thid].used);
		return ERROR;
	}
    
	strncpy(g_Thread[thid].name, name, TH_NAME_LEN-1);
	g_Thread[thid].name[TH_NAME_LEN-1] = '\0';
	g_Thread[thid].commno = thid;
	
	g_Thread[thid].used = 1;
	
	return thid;
}
/*------------------------------------------------------------------------
 Procedure:     ThreadPTcb ID:1
 Purpose:       取线程对应的TCB
 Input:         thid: 线程号
 Output:		TCB指针
 Errors:
------------------------------------------------------------------------*/
void *ThreadPTcb(int thid)
{
	return g_Thread[thid].pTcb;
}

int ThNameToId(const char *thname)
{
    int i;
	
	for(i=0; i<THREAD_MAX_NUM; i++)
	{
	   if(g_Thread[i].used == 0) continue;

	   if (strcmp(g_Thread[i].name,thname)==0) return(i);
	}   
	
	return ERROR;
}

int ThKeyNameToId(const char *thname)
{
    int i;
	
	for(i=0; i<THREAD_MAX_NUM; i++)
	{
	   if(g_Thread[i].used == 0) continue;

	   if (strstr(g_Thread[i].name,thname)!=NULL) return(i);
	}   
	
	return ERROR;
}


char *GetThName(int thid)
{
    return(g_Thread[thid].name);
}

void DiasbleThDog(void)
{
    thWdg_Enable = 0;
}

void EnableThDog(void)
{
    thWdg_Enable = 1;
}

int thDisableDog(int thid)
{
	g_Thread[thid].watchdog = 0;
	return 0;
}

int thClearDog(int thid, int dogtm)
{    
	if (g_Thread[thid].watchdog > 0)
		g_Thread[thid].watchdog = dogtm;
	return 0;
}

int thRunDog(int thid, int interval)
{    
    if (!thWdg_Enable) return FALSE;
	if (!g_Thread[thid].active) return FALSE;
	
	if (g_Thread[thid].watchdog > 0)
	{
		g_Thread[thid].watchdog -= interval;
		if (g_Thread[thid].watchdog <= 0)
		{
		    if(thIsSuspended(thid))
		       return TRUE;
			else
			{
			   g_Thread[thid].watchdog = interval*10;
			   return FALSE;
			}
		}
	}

	return FALSE;
}

int GetThCommNo(int thid)
{
    return(g_Thread[thid].commno);
}

int SetThActive(int thid, int active)
{
    g_Thread[thid].active = active;
	return 0;
}

int GetThActive(int thid)
{
    return(g_Thread[thid].active);
}

void ThreadCommNoSet(int thid, int commno)
{
    int ret;
	g_Thread[thid].commno = commno;

	ret = evSend(thid, EV_COMMNO_MAP);
	if (ret)
		printf("%d send event error EV_COMMNO_MAP\n", thid);
}

static char shellbuf[512];
void shellprintf( const char *fmt, ... )
{
	va_list varg;
    va_start( varg, fmt );
	vsprintf(shellbuf, fmt, varg );
	va_end( varg );
	printf("%s",shellbuf);
}

