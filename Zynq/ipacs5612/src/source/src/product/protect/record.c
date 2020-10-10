
#include "syscfg.h"

#ifdef  INCLUDE_RECORD
#include "sys.h"
#include "fileext.h"
#include "record.h"

#ifdef  _TEST_VER_
#define WAVPOINT    (160)    //不能大于采样点160
#else
#define WAVPOINT    (80)    //不能大于采样点160
#endif

#define RECORD_COM_CFG_LEN		2048
#define RECORD_FD_NUM_MAX		MAX_FD_NUM

WORD recordFdnum;		/*需要记录的回线数*/


#define RECORD_FILE_NAME_LEN	(64*2)			/*录波文件名长度*/
#define RECORD_FILE_RCD_LEN    34

#define RECORD_FILE_RECORD_LEN 34
#define FD_RECORD_NUM  (64)

#define RECORD_FILE_DIR              "%s/COMTRADE"
#define RECORD_FILE_DIR_F          "%s/COMTRADE/%s"
#define RECORD_FILE_DIR_BAY     "%s/COMTRADE/BAY"
#define RECORD_FD_COUNT_NUM          9999


#define RECORD_CON_MAX        25 /*25*200*4=20s,25*200*20=100K*/

typedef struct
{
    WORD  wRecChanNum;
	WORD  wRecAiNum;
	WORD  wRecDiNum;
	WORD wCfgNum;//只有第一回线的有效，文件序号所有的开关依次排
	WORD wDatNum;//只有第一回线的有效
}VRecordLine;


BOOL g_record = 0;
static VRecordLine 	*recordLine;	/*每回线对应的录波结构*/
int g_rcddirinit = 0;
/////////////////////////////
VFdSamRecordIndex *FdSamRecordIndex;
struct VDirInfo *pRCDDirInfo = NULL;
DWORD dwrcdnum = 0;



int recordInit(WORD wTaskID);
static int recordFileDelete(WORD fd);
static int recordFileWrite(WORD fd);

int itoa1(char* str, int number, int len);

static char* gRecordCfg[RECORD_FD_NUM_MAX];


STATUS recordDirCheck(void)
{
	struct stat fstat;
	char  path[4*MAXFILENAME];
	
	sprintf(path, RECORD_FILE_DIR, SYS_PATH_ROOT2);
	
	if ((stat(path, &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir(path)==FileError)  
		{
		    myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir %s failed!",path);
			WriteWarnEvent(NULL, SYS_ERR_SYS, 0, "create %s 出错",path);
			return ERROR;
		}
	}

	return OK;
}

int recordInit(WORD wTaskID)
{
	int i,j,m,n,slen; float a,b,c,c0; 
	char tempStr[150];
	int k,type,t;
	char phase[4] = {'A','B','C','N'};
	VFdCfg *pfdCfg;
	char cfgname[4*MAXFILENAME];
	char datname[4*MAXFILENAME];

	InitFdSamRecordIndex();
  	recordFdnum = g_Sys.MyCfg.wFDNum;
	if(recordFdnum > RECORD_FD_NUM_MAX)
	{
		myprintf(wTaskID,LOG_ATTR_INFO, "Feeder number too many!");
		return ERROR;
	}

	if(recordDirCheck()==ERROR)
		return ERROR;

	recordLine	=( VRecordLine *)malloc(sizeof(VRecordLine)*recordFdnum);
	if (recordLine==NULL) 
		return(ERROR);
	memset(recordLine, 0, sizeof(VRecordLine)*recordFdnum);
	for(i = 0 ; i < recordFdnum;i++)
	{
		gRecordCfg[i] = (char*)malloc(RECORD_COM_CFG_LEN);
		if(gRecordCfg[i] == NULL)
			return (ERROR);
		memset(gRecordCfg[i], 0, RECORD_COM_CFG_LEN);
	}
	
	for(i = 0;i <recordFdnum; i++)
	{
		pfdCfg = pFdCfg+i;

		sprintf(tempStr,"%s,%d,1999\r\n",g_Sys.InPara[SELFPARA_DEVTYPE],g_Sys.AddrInfo.wAddr);  // gu 
		strcpy(gRecordCfg[i], tempStr);
		
    	sprintf(tempStr,"%d,%dA,%dD\r\n",recordLine[i].wRecChanNum,recordLine[i].wRecAiNum,recordLine[i].wRecDiNum);
		strcat(gRecordCfg[i],tempStr);
		k = 0;
		b = 0;
		
		c = ((float)pfdCfg->ct) / ((float)pfdCfg->In);
		c0 = ((float)pfdCfg->ct0) /((float) pfdCfg->In0);
		for (j = MMI_MEA_IA; j < MMI_MEA_UA; j++)
		{
			 
 			//if (pFdCfg[i].mmi_meaNo_ai[j] != -1) 
 			{
				type = j;
				b =  -FdSamRecordIndex[i].GainB[j];
				switch(type)
				{
				   case MMI_MEA_IA:
				   case MMI_MEA_IB:
				   case MMI_MEA_IC:
				   	    t = type - MMI_MEA_IA;
				   		a=32768*5/(64.0*1.414*FdSamRecordIndex[i].GainA[t]);

			   	   sprintf(tempStr,"%d,I%c,%c,,A,%f,%f,0.0,-8192,8192,%f,%f,S\r\n", k+1,phase[t],phase[t],a, a*b,c,1.0); // gu

						strcat(gRecordCfg[i], tempStr);
						k++;
						break;
					case MMI_MEA_I0:
						a=32768*pFdCfg[i].In0/(64.0*1.414*FdSamRecordIndex[i].GainA[3]);
				  sprintf(tempStr,"%d,IZ,Z,,A,%f,%f,0.0,-8192,8192,%f,%f,S\r\n", k+1,a, a*b,c0,1.0);   
						strcat(gRecordCfg[i], tempStr);
						k++;
						break;
					default:
						break;
				}
 			}
		}

		c = ((float)pfdCfg->pt)/ ((float)pfdCfg->Un);
		for (j = MMI_MEA_UA; j < (MMI_MEA_NUM-1); j++)
		{
			 
 			//if (pFdCfg[i].mmi_meaNo_ai[j] != -1) 
 			{
				type = j;
				b =  -FdSamRecordIndex[i].GainB[j];
				switch(type)
				{
					case MMI_MEA_UA:
					case MMI_MEA_UB:
					case MMI_MEA_UC:
						t = type - MMI_MEA_UA;
						a=32768*100.0/(64.0*1.414*FdSamRecordIndex[i].GainA[4+t]);
		                sprintf(tempStr,"%d,U%c,%c,,V,%f,%f,0.0,-8192,8192,%f,%f,S\r\n", k+1, phase[t],phase[t],a,a*b,c,1.0);  // gu 
						strcat(gRecordCfg[i], tempStr);
						k++;
						break;
					case MMI_MEA_U0:
						t = type - MMI_MEA_UA;
						a=32768*100.0/(64.0*1.414*FdSamRecordIndex[i].GainA[4+t]);
					  if(g_Sys.MyCfg.dwCfg & 0x10)
               		  		sprintf(tempStr,"%d,UZ,Z,,V,%f,%f,0.0,-8192,8192,%f,%f,S\r\n", k+1,a/10,a*b/10,c,1.0);  // gu 
						else
							 sprintf(tempStr,"%d,UZ,Z,,V,%f,%f,0.0,-8192,8192,%f,%f,S\r\n", k+1,a,a*b,c,1.0);  // gu 
						strcat(gRecordCfg[i], tempStr);
						k++;
						break;
					default:
						break;
				}
 			}
		}
		
	recordLine[i].wRecAiNum = k;
	recordLine[i].wRecDiNum = 1;
	recordLine[i].wRecChanNum = k + 4;
		
	sprintf(tempStr,"%d,KG,,,0\r\n",1);               //gu , file page 72
	strcat(gRecordCfg[i], tempStr);

	strcat(gRecordCfg[i], "50.00\r\n");
	strcat(gRecordCfg[i], "1\r\n");
		
	//重新给gRecordCfg 配值
	sprintf(tempStr,"%s,%d,1999\r\n%d,%dA,%dD\r\n",g_Sys.InPara[SELFPARA_DEVTYPE],g_Sys.AddrInfo.wAddr,
		recordLine[i].wRecAiNum + 1,recordLine[i].wRecAiNum,1);        // gu,  file page 73,line 18
		
	memcpy(gRecordCfg[i],tempStr,strlen(tempStr));
	}
	
	{
		struct dirent *pent;
		DIR *pDir;
		WORD len,fdnum_cfgdat,num_cfgdat;
		char fnamedel[300];
		char path[4*MAXFILENAME];
	
		sprintf(path, RECORD_FILE_DIR, SYS_PATH_ROOT2);
		pDir = opendir(path);
		do
		{
			pent = readdir(pDir);
			if(pent == NULL)
				break;
			if(pent->d_name[0] == '.')
				continue;
			len = strlen(pent->d_name);
			
		 if (((pent->d_name[len-1] != 't')&&(pent->d_name[len-1] != 'g'))||(len != RECORD_FILE_RECORD_LEN))
	   {
			sprintf(fnamedel, RECORD_FILE_DIR_F, SYS_PATH_ROOT2,pent->d_name);
			
			if(remove(fnamedel)==ERROR)
				shellprintf("remove %s error\n",fnamedel);
			else
				shellprintf("remove %s OK\n",fnamedel);
			continue;
	   }
		 
		 memset(fnamedel,0,RECORD_FILE_NAME_LEN);
		 memcpy(fnamedel,pent->d_name+3,2);
		 fdnum_cfgdat = atoi(fnamedel);
		 if(fdnum_cfgdat > recordFdnum)
		 {
		 	sprintf(fnamedel, RECORD_FILE_DIR_F, SYS_PATH_ROOT2,pent->d_name);
		 	if(remove(fnamedel)==ERROR)
				shellprintf("remove %s error\n",fnamedel);
			else
				shellprintf("remove %s OK\n",fnamedel);
		 }
 
		 memset(fnamedel,0,RECORD_FILE_NAME_LEN);

		 memcpy(fnamedel,pent->d_name+6,4);
		 num_cfgdat = atoi(fnamedel);
		 
		 if(pent->d_name[len-1] == 'g')
		 {
			 if(recordLine[0].wCfgNum < num_cfgdat)
				 recordLine[0].wCfgNum = num_cfgdat;
		 }
		 else if(pent->d_name[len-1] == 't')
		 {
			 if(recordLine[0].wDatNum < num_cfgdat)
				 recordLine[0].wDatNum = num_cfgdat;
		 }
		}while(pent != NULL);
		closedir(pDir);
	}
	
	pRCDDirInfo = (struct VDirInfo *)malloc(sizeof(struct VDirInfo)*(10 + 64*2));
	g_rcddirinit = 1;

	ReadRcdDir(NULL,1);

	for(m = 0; m < dwrcdnum; m++)
	{	
		slen = strlen(pRCDDirInfo[m].cName);
		if(strstr(pRCDDirInfo[m].cName,".cfg"))
		{
			memset(datname, 0, 64);
			strncpy(datname,pRCDDirInfo[m].cName,slen-4);
			strcat(datname, ".dat");
			for(n = 0; n < dwrcdnum; n++)
			{
				if(0!=strcmp(pRCDDirInfo[n].cName,datname))
				{
					if(n==(dwrcdnum-1))
					{
						memset(cfgname, 0, 64);
						sprintf(cfgname, RECORD_FILE_DIR,SYS_PATH_ROOT2);
						strcat(cfgname, "/");
						strcat(cfgname,pRCDDirInfo[m].cName);
						remove(cfgname);
					}
				}
				else
					break;
			}
		}

		if(strstr(pRCDDirInfo[m].cName,".dat"))
		{
			memset(cfgname, 0, 64);
			strncpy(cfgname,pRCDDirInfo[m].cName,slen-4);
			strcat(cfgname, ".cfg");
			for(n = 0; n < dwrcdnum; n++)
			{
				if(0!=strcmp(pRCDDirInfo[n].cName,cfgname))
				{
					if(n==(dwrcdnum-1))
					{
						memset(datname, 0, 64);
						sprintf(datname, RECORD_FILE_DIR,SYS_PATH_ROOT2);
						strcat(datname, "/");
						strcat(datname,pRCDDirInfo[m].cName);
						remove(datname);
					}
				}
				else
					break;
			}
		}
	}
	ReadRcdDir(NULL,1);
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
void recordWrite(WORD fd)
{
	recordFileDelete(fd);
	recordFileWrite(fd);
	ReadRcdDir(NULL,1);
}
/*------------------------------------------------------------------------
 Procedure:     recordFileDelete ID:1
 Purpose:       记录文件删除函数
 				删除老记录文件,让空间给新记录,
 Input:
 Output:
 Errors:		
------------------------------------------------------------------------*/

static int recordFileDelete(WORD fd)
{
	char fname[RECORD_FILE_NAME_LEN];
	char fnamenew[RECORD_FILE_NAME_LEN];
	char fnamedel[RECORD_FILE_NAME_LEN];
	char timecfg[RECORD_FILE_RECORD_LEN+1];
	char timedat[RECORD_FILE_RECORD_LEN+1];
	char temp[RECORD_FILE_RECORD_LEN+1];
	char temp1[RECORD_FILE_RECORD_LEN+1];
	int len,cfg_num,dat_num;
	//int cfgfdnum;
	DIR *pDir;
	struct dirent *pent;
	char path[4*MAXFILENAME];

	cfg_num = dat_num = 0;
	sprintf(path,RECORD_FILE_DIR,SYS_PATH_ROOT2);
  	 pDir = opendir(path);
	
	sprintf(timecfg, "21100101_000000_000");
  sprintf(timedat, "21100101_000000_000");
	memset(fname,0,RECORD_FILE_NAME_LEN);
	memset(fnamenew,0,RECORD_FILE_NAME_LEN);
	memset(fnamedel,0,RECORD_FILE_NAME_LEN);
	do
	{
	   pent = readdir(pDir);

	   if (pent == NULL)
	   	  break;
	   if (pent->d_name[0] == '.') 
	       continue;
	   len = strlen(pent->d_name);
	   
		//取开关数
		memcpy(temp1,pent->d_name,len);
		memset(temp,0,len);

		 
		 if(pent->d_name[len-1] == 'g')
		 {
			 cfg_num ++;
			 memcpy(temp1, pent->d_name,len);
			 memcpy(temp,&temp1[6+5],19);

			 if(strcmp(temp,timecfg) < 0)
			 {
				 memcpy(timecfg,temp,19);
				 memcpy(fname,pent->d_name,len);
			 }
		 }
		 if(pent->d_name[len-1] == 't')
		 {
			 dat_num ++;
			 memcpy(temp1, pent->d_name,len);
			 memcpy(temp,&temp1[6+5],19);
			 if(strcmp(temp,timedat) < 0)
			 {
				 memcpy(timedat,temp,19);
				 memcpy(fnamenew,pent->d_name,len);
			 }
		 }
	}while(pent != NULL);
	
	closedir(pDir);
	
	if(cfg_num >= FD_RECORD_NUM)
	{
		len = strlen(fname);
		memcpy(temp,fname,len);
		temp[len] = '\0';
		sprintf(fname,RECORD_FILE_DIR_F, SYS_PATH_ROOT2,temp);
		if(remove(fname)==ERROR)
				shellprintf("remove %s error\n",fname);
		else
		{
			shellprintf("remove %s OK\n",fname);
		}
	}
	if(dat_num >= FD_RECORD_NUM)
	{
		len = strlen(fnamenew);
		memcpy(temp,fnamenew,len);
		temp[len] = '\0';
		
		sprintf(fnamenew,RECORD_FILE_DIR_F, SYS_PATH_ROOT2,temp);
		if(remove(fnamenew)==ERROR)
				shellprintf("remove %s error\n",fnamenew);
		else
		{
			shellprintf("remove %s OK\n",fnamenew);
		}
	}
    	shellprintf("cfg:%d  dat:%d \n",cfg_num,dat_num);
	return OK;
}

int itoa1(char* str, int number, int len)  //与comtrade 完全独立
{
   int i,j,k;
   BYTE a;
   char temp[10];

   if(str == NULL)
   	 return -1;
   i=0;
   j=0;
   if(number < 0)
   {
      str[0]='-';
	  i=1;
	  number = 0-number;
   }
   if(number == 0)
   {
   	  temp[0]='0';
	  j=1;
   }
   while(number > 0)
   {
       a = number%10;
	   temp[j] = a+'0';
	   number = number/10;
	   j++;
   } 
   if(j<len)
   {
      for(k=0; k<(len-j); k++)
	  	str[i++] = '0';
   }
   for(k=0; k<j; k++)
   	 str[i++]=temp[j-k-1];
   str[i] = '\0';
   return i;	
}
	
/*------------------------------------------------------------------------
 Procedure:     recordFileWrite ID:1
 Purpose:       新记录文件写入函数
 Input:
 Output:
 Errors:		
------------------------------------------------------------------------*/
static int recordFileWrite(WORD fd)
{
  char tempnum[150];
	FILE *fp; 
	int j,len = 0;
	struct VCalClock time;
	struct VCalClock pretime;
	struct VSysClock systime;
	VRecordLine *pRecLine=recordLine+fd;
	struct VSysClock  clock2;
	char tempcfg[RECORD_FILE_NAME_LEN];
#if defined (_YIERCI_RH_) && defined (_TEST_VER_)
	char cptempname[RECORD_FILE_NAME_LEN]; 
#endif
	WORD smpfreq = 0,smpnum = 0;
	BYTE* recordbuf = NULL;
	
	recordbuf = ReadBmRecordData( &smpfreq, &smpnum, &time, &pretime);
	if(recordbuf == NULL)
		return ERROR;

	if(fd >= recordFdnum) 
		return ERROR;


	CalClockTo(&time, &systime);
	CalClockTo(&pretime,&clock2);

	recordLine[0].wCfgNum = recordLine[0].wCfgNum%RECORD_FD_COUNT_NUM + 1;

	sprintf(tempnum, "%04d%02d%02d_%02d%02d%02d_%03d", systime.wYear, systime.byMonth, 
	systime.byDay,systime.byHour,systime.byMinute,systime.bySecond,systime.wMSecond);

	sprintf(tempcfg, "%s/COMTRADE/BAY%02d_%04d_%s.cfg", SYS_PATH_ROOT2,fd+1,recordLine[0].wCfgNum, tempnum);

	fp = fopen(tempcfg, "r+b");
	if(fp == NULL) fp = fopen(tempcfg, "wb");
	if(fp != NULL)
	{
		j = fwrite(gRecordCfg[fd], 1, strlen(gRecordCfg[fd]), fp);
		sprintf(tempnum,"%d,%d\r\n", smpfreq, smpnum);
		j = fwrite(tempnum, 1, strlen(tempnum), fp);
	}
	
	sprintf(tempnum, "%02d/%02d/%02d,%02d:%02d:%02d.%03d000\r\n", clock2.byDay, clock2.byMonth, clock2.wYear, 
			 clock2.byHour, clock2.byMinute, clock2.bySecond, clock2.wMSecond);
	j = fwrite(tempnum, 1, strlen(tempnum), fp);
	sprintf(tempnum, "%02d/%02d/%02d,%02d:%02d:%02d.%03d000\r\n", systime.byDay, systime.byMonth, systime.wYear, 
			 systime.byHour, systime.byMinute, systime.bySecond, systime.wMSecond);
	j = fwrite(tempnum, 1, strlen(tempnum), fp);
		strcpy(tempnum, "binary\r\n");  // gu   file page 76,line 14
	j = fwrite(tempnum, 1, strlen(tempnum), fp);
	sprintf(tempnum, "%d\r\n",1);  // 时间倍乘因数
	j = fwrite(tempnum, 1, strlen(tempnum), fp);
	fclose(fp);
#if defined (_YIERCI_RH_) && defined (_TEST_VER_)
sprintf(cptempname,"%s/tmp/%d.cfg",SYS_PATH_ROOT,recordLine[0].wCfgNum);
cp(tempcfg,cptempname); 			
#endif
			
	recordLine[0].wDatNum = recordLine[0].wDatNum%RECORD_FD_COUNT_NUM + 1;
	sprintf(tempnum, "%04d%02d%02d_%02d%02d%02d_%03d", systime.wYear, systime.byMonth, 
	systime.byDay,systime.byHour,systime.byMinute,systime.bySecond,systime.wMSecond);

	sprintf(tempcfg, "%s/COMTRADE/BAY%02d_%04d_%s.dat", SYS_PATH_ROOT2,fd+1, recordLine[0].wDatNum,tempnum);

	if(recordbuf == NULL)
		return ERROR;
	
	fp = fopen(tempcfg,"r+b");
	if(fp == NULL) fp = fopen(tempcfg,"wb");
	if(fp != NULL)
	{			
		len  =  smpnum * (8 + (pRecLine->wRecAiNum + 1)*sizeof(WORD));
		j = fwrite(recordbuf, 1, len,fp);
		if(j != len)
		{
			fclose(fp);
			return ERROR;
		}
	}
	fclose(fp);
#if defined (_YIERCI_RH_) && defined (_TEST_VER_)
  sprintf(cptempname,"%s/tmp/%d.dat", SYS_PATH_ROOT,recordLine[0].wDatNum);
  cp(tempcfg,cptempname); 			
#endif			
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

		///AI
        //建立回线对AI通道的有效值地址指针,如果无该项值,则地址为缺省 -1
        k=0;
		
		for (j=0; j<4; j++)
		{
			if((pFdCfg[fd].mmi_meaNo_ai[j]) != -1)
			{
				FdSamRecordIndex[fd].AiIndex[k] = pFdCfg[fd].mmi_meaNo_ai[j];
				FdSamRecordIndex[fd].GainB[j] = pMeaGain[FdSamRecordIndex[fd].AiIndex[k]].b;
				FdSamRecordIndex[fd].GainA[j] = pMeaGain[FdSamRecordIndex[fd].AiIndex[k]].a;
				k++;
			}
			else
			{
				FdSamRecordIndex[fd].AiIndex[k] = -1;
				FdSamRecordIndex[fd].GainB[j] = 0;
				FdSamRecordIndex[fd].GainA[j] = 0;
				k++;
			}
		}
		for (j=0; j<4; j++)
		{
			if(pFdCfg[fd].mmi_meaNo_ai[j+4] != -1)
			{
				FdSamRecordIndex[fd].AiIndex[k] = pFdCfg[fd].mmi_meaNo_ai[j+4];
				FdSamRecordIndex[fd].GainB[j+4] = pMeaGain[FdSamRecordIndex[fd].AiIndex[k]].b;
				FdSamRecordIndex[fd].GainA[j+4] = pMeaGain[FdSamRecordIndex[fd].AiIndex[k]].a;
				k++;
			}
			else
			{
				FdSamRecordIndex[fd].AiIndex[k] = -1;
				FdSamRecordIndex[fd].GainB[j+4] = 0;
				FdSamRecordIndex[fd].GainA[j+4] = 0;
				k++;
			}
		}
	}
}

int WAV_Record_Scan(void)
{
	WORD fd;
	
	if(!g_record)
		return 0;

	while(ReadBmRecord(&fd) == OK)
	{
		recordWrite( fd);
		shellprintf("recordWrite scan \n");
	}
	
	return 1;
}

// 因录波目录可能有128个文件，直接读，目录耗时较长，故将录波文件先读出来以备规约调用
struct VDirInfo * ReadRcdDir(WORD *dnum,BOOL flag)
{
	struct VFileOPMsg FileOPMsg;
	WORD recordnum = 0;

	if(flag > 0)
	{
		recordnum =  64*2 + 10;

		if(g_rcddirinit != 1)
		{
			pRCDDirInfo = (struct VDirInfo *)malloc(sizeof(struct VDirInfo)*recordnum);
			g_rcddirinit = 1;
		}
		sprintf((char *)FileOPMsg.cFileName, "%s/COMTRADE",SYS_PATH_ROOT2);
		FileOPMsg.dwLen = sizeof(struct VDirInfo)*recordnum;

		FileOPMsg.dwOffset = 0;

		if(ListDir(&FileOPMsg, pRCDDirInfo) == FileOk)
			dwrcdnum = FileOPMsg.dwLen / sizeof(struct VDirInfo);              //获取的目录数量
		else
			dwrcdnum = 0;
	}
	if(dnum != NULL)
	{
		*dnum = dwrcdnum;
	}
	return pRCDDirInfo;
}
#endif /*INCLUDE_RECORD*/
