/*------------------------------------------------------------------------
 Module:       	b2f.cpp
 Author:        solar
 Project:       
 State:			
 Creation Date:	2009-02-23
 Description:   buf to file
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#include <syscfg.h>

#ifdef INCLUDE_B2F
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys.h"
#include "fileext.h"
#ifdef INCLUDE_RECORD
#include "record.h"
#endif
#ifdef INCLUDE_HIS
#include "dbhis.h"
#endif
#ifdef INCLUDE_HIS_TTU
#include "dbhis_ttu.h"
#endif
#ifdef	INCLUDE_COMTRADE 
#include "comtrade.h"
#endif

static int b2fRPtr, b2fWPtr;
static DWORD b2fSem;
static VB2FSocket  B2FSocket[MAX_B2FSOCKET_NUM];
//ssz
VB2FSocket *ppB2FSocket[MAX_B2FSOCKET_NUM];

int Buf2FileFind(char *fname)
{
	int i;

	for (i=0; i<MAX_B2FSOCKET_NUM; i++)
	{
		if (B2FSocket[i].read_write != 1) continue;

		if (strcmp(B2FSocket[i].fname, fname) == 0) return TRUE;
	}

	return FALSE;
}

void Buf2FileTmpClearDir(char *dirName)
{
	DIR *pDir;				/* ptr to directory descriptor */
	struct dirent	*pDirEnt;	/* ptr to dirent */
	char path[4*MAXFILENAME];

	pDir = opendir (dirName) ;

	if (pDir !=NULL)  
	{
		do
		{
			pDirEnt = readdir (pDir);
			
			if (pDirEnt != NULL) 
			{
				if (pDirEnt->d_name[0] == '.') continue;

				if (Buf2FileFind(pDirEnt->d_name) == FALSE)
				{
					strcpy(path, dirName);
					strcat(path, "/");
					strcat(path, pDirEnt->d_name);
					remove(path);
				}	
			}  
		}while ((pDirEnt != NULL));

		closedir (pDir);
	}	
}

void Buf2FileTmpClear(void)
{
	int i;
	char  path[4*MAXFILENAME];
	
	strcpy(path,SYS_PATH_ROOT2);
	strcat(path,"/event/soe");
	Buf2FileTmpClearDir(path);
	
	strcpy(path,SYS_PATH_ROOT2);
	strcat(path,"/event/other");
	Buf2FileTmpClearDir(path);
	
	strcpy(path,SYS_PATH_ROOT2);
	strcat(path,"/bakup");
	Buf2FileTmpClearDir(path);
	
	for (i=0; i<MAX_B2FSOCKET_NUM; i++)
	{
		if (B2FSocket[i].read_write == 1) 
			B2FSocket[i].read_write = 0;
	}
}

int Buf2FileWtiteImm(VB2FSocket *pB2FSocket)
{
	char path[4*MAXFILENAME];
    FILE *fp;
	int result, len;    


	if (strcmp(pB2FSocket->fname, "EXTNV") == 0)
	{
		extNvRamSet(pB2FSocket->offset, pB2FSocket->buf, pB2FSocket->len);				
		return OK;	
	}

    GetMyPath(path, pB2FSocket->fname);
	fp = fopen(path, "r+b");
	if (fp == NULL) fp = fopen(path, "wb");
	if (fp == NULL) goto Err;
	
	result = fseek(fp, pB2FSocket->offset, SEEK_SET);
	if (result) goto Err;

	len = fwrite(pB2FSocket->buf, sizeof(BYTE), pB2FSocket->len, fp);
	
    if (len != pB2FSocket->len) goto Err;
  
	fclose(fp);  
	pB2FSocket->read_write = 0;	
    return OK;  
	
Err:
    if (fp) fclose(fp);
	pB2FSocket->read_write = 0;	
	return ERROR;		
}

//ssz:20170215
int Buf2FileWtiteImm2(VB2FSocket **ppMyB2FSocket,int Count)
{
	char path[4*MAXFILENAME];
    FILE *fp;
	int result, len;    
	VB2FSocket *pB2FSocket;
	int i = 0;
	BOOL bFlag = FALSE;
	if(Count<1)
		return ERROR;

	
	pB2FSocket = ppMyB2FSocket[0];
	
	if (strcmp(pB2FSocket->fname, "EXTNV") == 0)
	{
		for(i=0;i<Count;i++)
		{
			pB2FSocket = ppMyB2FSocket[i];
			extNvRamSet(pB2FSocket->offset, pB2FSocket->buf, pB2FSocket->len);
		}
		return OK;	
	}

       GetMyPath(path, pB2FSocket->fname);
	fp = fopen(path, "r+b");
	if (fp == NULL) fp = fopen(path, "wb");
	if (fp == NULL) 
	{
		for(i=0;i<Count;i++)
		{
			pB2FSocket = ppMyB2FSocket[i];
			pB2FSocket->read_write = 0;//对错取消标志
		}
		return ERROR;
	}
	
	for(i=0;i<Count;i++)
	{
		pB2FSocket = ppMyB2FSocket[i];
		pB2FSocket->read_write = 0;//对错取消标志
		result = fseek(fp, pB2FSocket->offset, SEEK_SET);
		if (-1 == result) 
		{//一帧的错，不影响下一帧的执行
			bFlag = TRUE;
			continue;
		}

		len = fwrite(pB2FSocket->buf, sizeof(BYTE), pB2FSocket->len, fp);

		if (len != pB2FSocket->len) 
		{
			bFlag = TRUE;
			//continue;
		}
	}
	fclose(fp);  

	if(bFlag)
    		return OK;  
	else
		return ERROR;		
}

void Buf2FileProc(void)
{
    int i, k,wptr, ptr1, num, j, ptr2, same, num1,SameCount;
	
	for(;;)
	{
		wptr = b2fWPtr;
		num = (wptr >= b2fRPtr) ? (wptr-b2fRPtr):(MAX_B2FSOCKET_NUM-b2fRPtr+wptr);

        if (num == 0) break;

        ptr1 = (wptr-1)&(MAX_B2FSOCKET_NUM-1);
		num1 = num-1;
        for (i=0; i<num; i++, num1--, ptr1 = (ptr1-1)&(MAX_B2FSOCKET_NUM-1))
        {
			if (B2FSocket[ptr1].read_write != 2) continue;

           
            ptr2 = b2fRPtr;
			for(j=0; j<num1; j++, ptr2 = (ptr2+1)&(MAX_B2FSOCKET_NUM-1))
			{
				if (B2FSocket[ptr2].read_write != 2) continue;

				same = 1;
			    if (B2FSocket[ptr2].offset !=  B2FSocket[ptr1].offset)
					same = 0;
				else if (B2FSocket[ptr2].len !=  B2FSocket[ptr1].len)
					same = 0;
				else if (strcmp(B2FSocket[ptr2].fname, B2FSocket[ptr1].fname) != 0) 
					same = 0;

				if (same) 
				{
					B2FSocket[ptr2].buf = B2FSocket[ptr1].buf;
					B2FSocket[ptr1].read_write = 4;
				}								
			}
		}
		
		//test
		SameCount = 0;
		ptr1 = b2fRPtr;	
		for (i=0; i<num; i++, ptr1 = (ptr1+1)&(MAX_B2FSOCKET_NUM-1))
		{
			if (B2FSocket[ptr1].read_write != 2) 
				SameCount++;
		}
		//

		ptr1 = b2fRPtr;		
		for (i=0; i<num; i++, ptr1 = (ptr1+1)&(MAX_B2FSOCKET_NUM-1))
		{
			if (B2FSocket[ptr1].read_write != 2) 	continue;
			
			//ssz
			//Buf2FileWtiteImm(B2FSocket+ptr1);
			ptr2 = ptr1; 
			k = 0;
			ppB2FSocket[k++] =  B2FSocket+ptr1;
			for(j = i+1;j<num;j++)
			{
				ptr2++;//从ptr1后面一个开始 
				if (B2FSocket[ptr2].read_write != 2) continue;
				 if (strcmp(B2FSocket[ptr2].fname, B2FSocket[ptr1].fname) == 0)
				 {
					ppB2FSocket[k++] =  B2FSocket+ptr2;
				 }
			}

			if(1 == k)
			{
			Buf2FileWtiteImm(B2FSocket+ptr1);
			}
			else
			{// >1
				Buf2FileWtiteImm2(ppB2FSocket,k);
			}
		}		

		ptr1 = b2fRPtr;		
		for (i=0; i<num; i++, ptr1 = (ptr1+1)&(MAX_B2FSOCKET_NUM-1))
		{
			if (B2FSocket[ptr1].read_write == 4) 
				B2FSocket[ptr1].read_write = 0;
		}		

		b2fRPtr = (b2fRPtr+num)&(MAX_B2FSOCKET_NUM-1);
	}
}

#if 1
void buf2fileDirCheck(void)
{
	char  path[4*MAXFILENAME];
	struct stat filestat;
	char *myPath2[]={"","/event","/event/soe","/event/other","/bakup"};
	int i;

	for (i=0; i<NELEMENTS(myPath2); i++)
	{
		strcpy(path,SYS_PATH_ROOT2);
		strcat(path,myPath2[i]);
		if ((stat(path, &filestat) >= 0) && (S_ISDIR(filestat.st_mode)))
			continue;

		if (MakeDir(path)==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '%s' failed!", path);
		else  myprintf(SYS_ID, LOG_ATTR_INFO, "Auto create Dir '%s' success!", path);		
	} 
}
#else
void buf2fileDirCheck(void)
{
	struct stat fstat;

	if ((stat("/tffsa/event", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsa/event")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/event' failed!");

		myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/event' success!");
	}	

	if ((stat("/tffsa/event/soe", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		 if (MakeDir("/tffsa/event/soe")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/event/soe' failed!");
	}	 

	if ((stat("/tffsa/event/other", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsa/event/other")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/event/other' failed!");
	}	

	if ((stat("/tffsa/bakup", &fstat) < 0) || (!S_ISDIR(fstat.st_mode)))
	{
		if (MakeDir("/tffsa/bakup")==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '/tffsb/bakup' failed!");
	}
}
#endif

void buf2fileInit(void)
{	
	buf2fileDirCheck();

	b2fSem = smMCreate();
	b2fRPtr = b2fWPtr = 0;
	memset(B2FSocket, 0, sizeof(B2FSocket));
}

void buf2file(void)
{
	DWORD events,count;	
	int num;
  
	int no;

	struct VFileMsgs fmsg;

	Buf2FileTmpClear();

	evSend(B2F_ID, EV_UFLAG);   //确保任务运行之前的写请求执行

	tmEvEvery(B2F_ID, SECTOTICK(30), EV_TM1);   /*30s*/

	no = -1;
	count = 0;
	for (;;)
	{
		evReceive(B2F_ID, EV_UFLAG | EV_MSG | EV_TM1|EV_TM2|EV_RCD, &events);
		smMTake(g_fileSem);
		thSleep(2);
		if(events&EV_UFLAG)
		{
			if (no == -1)
				no = tmEvAfter(B2F_ID, 200, EV_TM2);
		}
#if 1
		if (events&EV_TM2)
		{
			Buf2FileProc();
#ifdef INCLUDE_HIS			
			his_Event_Write();
#endif
#ifdef INCLUDE_HIS_TTU
			histtu_Event_Write();
#endif
			tmDelete(B2F_ID, no);
			no = -1;
		}
#endif
    if(events & EV_RCD)
		{
#ifdef INCLUDE_RECORD
			do
			{
				num = WAV_Record_Scan();
			}while(num);
#endif
		}
		if(events&EV_MSG)
		{
		   num = msgReceive(B2F_ID, &fmsg, sizeof(fmsg), OS_NOWAIT);
		   while(num > 0)
		   {
#ifdef INCLUDE_COMTRADE
		      if(fmsg.type == MSG_REC)
		        comtradeWaitWrite(fmsg.num);
#endif

//#ifdef INCLUDE_RECORD
//	  		  if(fmsg.type == MSG_RECORD)
//	  			RecordWaitWrite(fmsg.num);
//#endif
			  if(fmsg.type == MSG_FILE)
				WriteMsgFile((BYTE*)&fmsg + 1);
	  		  num = msgReceive(B2F_ID, &fmsg, sizeof(fmsg), OS_NOWAIT);	 
		   }
		}
		
		if (events&EV_TM1)
		{
			if(((count++)%30) == 0)
			{
#ifdef INCLUDE_EXTRTC
				GetExtrtcTime();
#endif
			}
#ifdef INCLUDE_HIS
		    his_Data_Write();
#endif
#ifdef INCLUDE_HIS_TTU
				histtu_Data_Write();
#endif
		}
#ifdef INCLUDE_HIS
		if (his_cfg_change)
		{
			his_cfg_write();
			his_cfg_change = 0;
		}
#endif
#ifdef INCLUDE_HIS_TTU
		if (histtu_cfg_change)
		{
			histtu_cfg_write();
			histtu_cfg_change = 0;
		}
#endif
		smMGive(g_fileSem);
	}
}

int Buf2FileRead(VB2FSocket *pB2FSocket)
{
	char path[4*MAXFILENAME];
    FILE *fp;
	int i, result;

	memset(pB2FSocket->buf, 0, pB2FSocket->len);

	for (i=0; i<MAX_B2FSOCKET_NUM; i++)
	{
        if ((B2FSocket[i].read_write==1) && (strcmp(B2FSocket[i].fname, pB2FSocket->fname)== 0))
			break;

		if (B2FSocket[i].read_write == 0)
		{
			memcpy(&B2FSocket[i], pB2FSocket, sizeof(VB2FSocket));
			B2FSocket[i].read_write = 1;
			break;
		}		
	}

	if (i == MAX_B2FSOCKET_NUM)  return ERROR;
    
    GetMyPath(path, pB2FSocket->fname);
	fp = fopen(path, "rb");

	if (fp == NULL) goto Err;
	
	result = fseek(fp, pB2FSocket->offset, SEEK_SET);
	if (result) goto Err;

	fread(pB2FSocket->buf, sizeof(BYTE), pB2FSocket->len, fp);
	  
	result = fclose(fp);
	if (result) goto Err;
    return OK;  
	
Err:
    if (fp) fclose(fp);
	return ERROR;		
}

int Buf2FileWrite(VB2FSocket *pB2FSocket)
{
	int k;


    if (pB2FSocket->read_write == 3)
		return(Buf2FileWtiteImm(pB2FSocket));
	else
	{
		smMTake(b2fSem);
		k = 0;
		while ((B2FSocket[b2fWPtr].read_write!=0) && (k<MAX_B2FSOCKET_NUM))
        {
			b2fWPtr++;
			b2fWPtr &= (MAX_B2FSOCKET_NUM-1);
			k++;
		}

		if (k == MAX_B2FSOCKET_NUM) 
		{
			smMGive(b2fSem);
			return ERROR;
		}

		memcpy(&B2FSocket[b2fWPtr], pB2FSocket, sizeof(VB2FSocket));
		B2FSocket[b2fWPtr].read_write = 2;
		b2fWPtr++;
		b2fWPtr &= (MAX_B2FSOCKET_NUM-1);
		smMGive(b2fSem);		
		evSend(B2F_ID, EV_UFLAG);
		return OK;		        
	}	
}

int Buf2FileDel(VB2FSocket *pB2FSocket)
{
	char path[4*MAXFILENAME];
    
    GetMyPath(path, pB2FSocket->fname);
	return(remove(path));
}

#endif

