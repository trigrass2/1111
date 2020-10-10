/*------------------------------------------------------------------------
 Module:       	Para.cpp
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-08-05
 Description:   
------------------------------------------------------------------------*/

#include "syscfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comm_cfg.h"
#include "sys.h"
#include "bsp.h"
#include "para.h"
#include "selfpara_gb2312.h"
#include "selfpara_utf8.h"
#include "fileext.h"

static BYTE ParaFileTmpBuf[MAXFILELEN];
struct VFileHead *g_pParafileTmp;
DWORD g_paraFTBSem;

static DWORD paraRWSemID;
static BYTE pkgFileBuf[MAXFILELEN];
static BYTE indFileBuf[MAXFILELEN>>2];
static struct VPIndHead *pIndHead;
static struct VPIndInfo *pFstIndInfo;
char gb_104_ip[16];
int gb_101_port;
 

DWORD g_ParaPtCtSem;
DWORD g_ParaYxFdSem;	
DWORD g_ParaFzHzSem;	
DWORD g_ParaCellSem;	

WORD GetParaCrc16(const BYTE *p, int l)
{
    int i;
	WORD windex, crc=0xFFFF;

	if (l < 0) return 0;

	for(i=0; i<l; i++)
	{
		windex = ((crc^p[i])&0x00FF);
		crc = ((crc>>8)&0x00FF) ^ crccode[windex];
	}

	return(crc);
}  

/*DWORD GetParaCrc32(BYTE *p, int l)
{
      int i;
	DWORD index, crc=0xFFFFFFFF;

	if (l < 0) return 0;

	for(i=0;i<l;i++)
	{
		index=((crc^p[i])&0x000000ff);
		crc=((crc>>8)&0x00ffffff)^crctable[index];
	}

	return(crc);
} */ 

int ReadAddrParaFile(BYTE *buf, int buf_len)
{
	struct VFileHead head;
	BYTE *p;
	if (pIndHead->addr.byValidFlag != 0x55) return ERROR;
	p = buf;
    head.nLength = sizeof(head)+sizeof(struct VPAddr);
	head.wVersion = pIndHead->wAddrVer;
	head.wAttr = CFG_ATTR_NULL;
	memcpy(p, &head, sizeof(head));
	p += sizeof(head);
	memcpy(p, &pIndHead->addr, sizeof(struct VPAddr));
	return OK;
}

int WriteAddrParaFile(struct VFileHead *buf)
{
    FILE *fp;
	struct VPAddr *paddr;
	WORD indcrc;
	int len;
	char path[4*MAXFILENAME];

	smMTake(paraRWSemID); 

	paddr = (struct VPAddr *)(buf+1);
	memcpy(&pIndHead->addr, paddr, sizeof(struct VPAddr));
	pIndHead->addr.byValidFlag = 0x55;
	pIndHead->wAddrVer = CFG_FILE_VER;

	indcrc =  GetParaCrc16((BYTE *)pIndHead, pIndHead->nLength);

	GetMyPath(path, CFG_IND_FILENAME);
	if ((fp=fopen(path, "r+b")) == NULL) 
	{
	    smMGive(paraRWSemID);
	    return ERROR;	
	}
	fseek(fp, 0, SEEK_SET);
    fwrite(&(pIndHead->addr), sizeof(BYTE), sizeof(struct VPAddr), fp);
	len = 0 - (int)sizeof(indcrc); 
	fseek(fp, len, SEEK_END);
	fwrite(&indcrc, sizeof(BYTE), sizeof(indcrc), fp);
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
    indFileBuf[pIndHead->nLength] = HIBYTE(indcrc);
    indFileBuf[pIndHead->nLength+1] = LOBYTE(indcrc);	
	
	smMGive(paraRWSemID);
	return OK;
		
}

int ReadCcdFile(BYTE *buf, int buf_len)
{
	struct dirent *pDirEnt;
	DIR *dir;
	char filename[4*MAXFILENAME];
	FILE *fp;
	int len;

	sprintf(filename,"%s/cfg", SYS_PATH_ROOT);
	dir = opendir(filename);
	do 
	{
		pDirEnt = readdir(dir);
		if (pDirEnt == NULL)
		{
			closedir(dir);
			return ERROR;
		}    
		else
		{
			if ((strcmp(pDirEnt->d_name, ".")==0) || (strcmp(pDirEnt->d_name, "..")==0) || (strcmp(pDirEnt->d_name, "lost+found") == 0))
				continue;
			if (strstr(pDirEnt->d_name, "ccd"))
				break;
		} 
	}
	while(pDirEnt!=NULL);
	closedir(dir); 

	sprintf(filename,"%s/cfg/%s", SYS_PATH_ROOT,pDirEnt->d_name);

	if ((fp=fopen(filename, "rb")) == NULL) 
		return ERROR;
		
	len  = (int)fread(buf, sizeof(BYTE), (DWORD)buf_len, fp);
	
    printf("ReadCcdFile filename %s len %d \r\n",filename,len);
	  
	fclose(fp); 

	return len;
}

int ReadFaParaFile(const char *filename,BYTE *buf, int buf_len)
{
	FILE *fp;
	int result, len; 
	WORD crc, mycrc;
	char path[4*MAXFILENAME];
	
	GetMyPath(path, (char*)filename);
	
	if ((fp=fopen(path, "rb")) == NULL) 
		return ERROR;

	result = fseek(fp, 0, SEEK_SET);
	if (result) 
	{
		fclose(fp);
		return ERROR;
	}
		
	len = (int)fread(buf, sizeof(BYTE), (DWORD)buf_len, fp);
	  
	fclose(fp);  

	if (len <= (int)(sizeof(struct VFileHead)+2)) return ERROR;

	mycrc = GetParaCrc16((BYTE *)buf, len-2);

	memcpy((BYTE*)&crc,buf+len-2,2);
	
	if (mycrc != crc)
	{
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "fa.cfg校验错误");
		myprintf(SYS_ID, LOG_ATTR_ERROR, "%s crc error!", "fa.cfg");
		return ERROR;
	}
  
	return OK;	
}
int ReadPrSetParaFile(const char *filename,BYTE *buf, int buf_len)
{
	FILE *fp;
	int result, len; 
	WORD crc, mycrc;
	char path[4*MAXFILENAME];

	sprintf(path,"%s/cfg/",SYS_PATH_ROOT);
	strcat(path,filename);
	
	if ((fp=fopen(path, "rb")) == NULL) 
		return ERROR;

	result = fseek(fp, 0, SEEK_SET);
	if (result) 
	{
		fclose(fp);
		return ERROR;
	}
		
	len = (int)fread(buf, sizeof(BYTE), (DWORD)buf_len, fp);
	  
	fclose(fp);  

	if (len <= (int)(sizeof(struct VFileHead)+2)) return ERROR;

	mycrc = GetParaCrc16((BYTE *)buf, len-2);

	memcpy((BYTE*)&crc,buf+len-2,2);
	
	if (mycrc != crc)
	{
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "保护参数模板校验错误");
		myprintf(SYS_ID, LOG_ATTR_ERROR, "%s crc error %x :%x!", filename, mycrc, crc);
		return(ERROR);
	}
  
	return OK;	
}

int ReadParaFile(const char *filename, BYTE *buf, int buf_len)
{
	FILE *fp;
	struct VPIndInfo *pInd;
	int i, result, len; 
	WORD mycrc;
	char path[4*MAXFILENAME];

	GetMyPath(path, CFG_IND_FILENAME);
	if ((fp=fopen(path, "rb")) == NULL)
		return ERROR;
   	len = (int)fread(indFileBuf, sizeof(BYTE), sizeof(indFileBuf), fp);

	fclose(fp);

    if (strcmp(filename, "addr.cfg") == 0)  
		return(ReadAddrParaFile(buf, buf_len));
	
	if((strcmp(filename, "ProSetTable.cfg") == 0)||(strcmp(filename, "ProSetPubTable.cfg") == 0)||(strcmp(filename, "diffproset.cfg") == 0))
	{
		return ReadPrSetParaFile(filename,buf,buf_len);
	}
	else if ((strcmp(filename, "fa.cfg") == 0) ||(strcmp(filename, "diffa.cfg") == 0))  
		return(ReadFaParaFile(filename,buf, buf_len));

	fp = NULL;
	smMTake(paraRWSemID); 

	pInd = pFstIndInfo;
	for (i=0; i<pIndHead->indNum; i++)
	{
		if (strcmp(pInd->cfgName, filename) == 0)  
			break;
		pInd++;
	}

    if (i == pIndHead->indNum) goto Err;
	
	GetMyPath(path, pIndHead->pkgInfo[pInd->pkgIndex].pkgName);
	if ((fp=fopen(path, "rb")) == NULL) goto Err;

	result = fseek(fp, pInd->cfgOffset, SEEK_SET);
	if (result) goto Err;

	len = fread(buf, sizeof(BYTE), (DWORD)pInd->cfgLen, fp);
	
    if (len != pInd->cfgLen) goto Err;
  
	fclose(fp);  
	smMGive(paraRWSemID);

    if (len < (int)sizeof(struct VFileHead)) return(ERROR);

    mycrc = GetParaCrc16((BYTE *)buf, len);
	
    if (mycrc != pInd->cfgCrc)
    {
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "参数校验错误");
		myprintf(SYS_ID, LOG_ATTR_ERROR, "%s crc error %x :%x!", filename, mycrc, pInd->cfgCrc);
    	return(ERROR);
    } 
  
    return(OK);  
	
Err:
    if (fp) fclose(fp);
	smMGive(paraRWSemID);
	return ERROR;	
}  

int WritePrSetParaFile(const char*filename,const struct VFileHead *buf)
{
    FILE *fp;
	WORD indcrc;
	int len;
	char path[4*MAXFILENAME];

	sprintf(path,"%s/cfg/",SYS_PATH_ROOT);
//  GetMyPath(path,filename);
	strcat(path,filename);
	len = buf->nLength;

	if ((fp=fopen(path, "wb")) == NULL)
	{
		if ((fp=fopen(path, "wb")) == NULL) goto Err;	
	}
	
	fseek(fp, 0, SEEK_SET);

    if (len != fwrite(buf, sizeof(BYTE), (DWORD)len, fp)) goto Err;	
	indcrc =  GetParaCrc16((BYTE*)buf, len);
	if(sizeof(indcrc)!= fwrite(&indcrc,sizeof(BYTE),sizeof(indcrc),fp)) goto Err;

	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
	return OK;
	
Err:
	if (fp) fclose(fp);
	return ERROR;			
}

int WriteParaProFile(const char *filename, BYTE *buf)
{
	WriteParaFile(filename,(struct VFileHead *)buf);
}

int WriteParaFile(const char *filename, struct VFileHead *buf)
{
	FILE *fp;
	WORD i;
	struct VPIndInfo *pInd, *pIndtmp;
	struct VPPkgHead pkgHead;
	int j, result, find, overwrite; 
	int len, tmp, pkglen, deltalen, deltaOffset;
	WORD cfgcrc, pkgcrc, indcrc;
	char path[4*MAXFILENAME];	
	
	GetMyPath(path, CFG_IND_FILENAME);
	if ((fp=fopen(path, "rb")) == NULL)  
	    return ERROR;
    fread(indFileBuf, sizeof(BYTE), sizeof(indFileBuf), fp);
	fclose(fp);
	
    if (strcmp(filename, "addr.cfg") == 0)  return(WriteAddrParaFile(buf));
	if((strcmp(filename, "ProSetTable.cfg") == 0)||(strcmp(filename, "ProSetPubTable.cfg") == 0)||(strcmp(filename, "diffproset.cfg") == 0))
	{
	  return WritePrSetParaFile(filename,buf);
	}
	smMTake(paraRWSemID);
	
	fp = NULL;
    find = 0;
	pInd = pFstIndInfo;
	for (i=0; i<pIndHead->indNum; i++)
	{
		if (strcmp(pInd->cfgName, filename) == 0)  
			break;
		pInd++;
	}

    if (i < pIndHead->indNum) find = 1;

	len = buf->nLength;

	cfgcrc = GetParaCrc16((BYTE *)buf, len);

    overwrite = 1;
    if (find) 
	{
        //应该判断新目标文件是否会导致原打包文件超过50k,如超过则需要
        //把原打包文件里的目标部分删除,然后打包到新的pkg文件中,
        //这里简化处理 
        i = pInd->pkgIndex;
		GetMyPath(path, pIndHead->pkgInfo[i].pkgName);
		if ((fp=fopen(path, "r+b")) == NULL)
		{
			if ((fp=fopen(path, "wb")) == NULL) goto Err;	

            memset(&pkgHead, 0, sizeof(pkgHead));
			pkgHead.nLength = sizeof(struct VPPkgHead);
			pkgHead.wVersion = CFG_FILE_VER;
			if (sizeof(struct VPPkgHead) != fwrite(&pkgHead, sizeof(BYTE), sizeof(struct VPPkgHead), fp)) 
				goto Err;			
			pInd->cfgOffset = sizeof(struct VPPkgHead);			
			pInd->cfgLen = len;
			pIndHead->pkgInfo[i].pkgLen = sizeof(struct VPPkgHead);
			pIndHead->pkgInfo[i].pkgVer = CFG_FILE_VER;
			
			overwrite = 0;
		}	
    }	
	else
	{
		//找一个pkg文件,如果它+目标文件不超过50k则将目标文件打包到该pkg
        //文件,否则产生一个新的pkg文件
		for (i=0; i<MAX_PKG_NUM; i++)
		{
			if (pIndHead->pkgInfo[i].pkgValid)
			{
				if ((pIndHead->pkgInfo[i].pkgLen+len) <= MAX_PKG_SIZE)  break;
			}
		}	

		//产生新的pkg文件
		if (i == MAX_PKG_NUM)
		{
			for (i=0; i<MAX_PKG_NUM; i++)
			{
				if (pIndHead->pkgInfo[i].pkgValid == 0)  break;
			}					

			if (i == MAX_PKG_NUM) goto Err;

	        sprintf(pIndHead->pkgInfo[i].pkgName, CFG_PKG_FILENAME, i);

			GetMyPath(path, pIndHead->pkgInfo[i].pkgName);
			if ((fp=fopen(path, "wb")) == NULL) goto Err;	

            memset(&pkgHead, 0, sizeof(pkgHead));
			pkgHead.nLength = sizeof(struct VPPkgHead);
			pkgHead.wVersion = CFG_FILE_VER;
			if (sizeof(struct VPPkgHead) != fwrite(&pkgHead, sizeof(BYTE), sizeof(struct VPPkgHead), fp)) 
				goto Err;			
			pInd->cfgOffset = sizeof(struct VPPkgHead);
			pIndHead->pkgInfo[i].pkgLen = sizeof(struct VPPkgHead);
	        pIndHead->pkgInfo[i].pkgVer = CFG_FILE_VER;
		}
        //打包到原pkg文件中
		else
		{
			GetMyPath(path, pIndHead->pkgInfo[i].pkgName);
			if ((fp=fopen(path, "r+b")) == NULL)
			{
				if ((fp=fopen(path, "wb")) == NULL) goto Err;	
				
				memset(&pkgHead, 0, sizeof(pkgHead));
				pkgHead.nLength = sizeof(struct VPPkgHead);
				pkgHead.wVersion = CFG_FILE_VER;
				if (sizeof(struct VPPkgHead) != fwrite(&pkgHead, sizeof(BYTE), sizeof(struct VPPkgHead), fp)) 
					goto Err;			
				pInd->cfgOffset = sizeof(struct VPPkgHead);				
				pIndHead->pkgInfo[i].pkgLen = sizeof(struct VPPkgHead);
				pIndHead->pkgInfo[i].pkgVer = CFG_FILE_VER;
			}
			else
				pInd->cfgOffset = pIndHead->pkgInfo[i].pkgLen;
		}
         
		pInd->cfgLen = len;
		pInd->pkgIndex = i;
		overwrite = 0;
	}

    if (len != pInd->cfgLen)
    {		
        deltaOffset = pInd->cfgOffset+pInd->cfgLen;

		result = fseek(fp, deltaOffset, SEEK_SET);
		if (result) goto Err;

		//后面部分的长度
		tmp = pIndHead->pkgInfo[i].pkgLen-deltaOffset;
		if (tmp > (int)sizeof(pkgFileBuf)) goto Err;		
		if (tmp != fread(pkgFileBuf, sizeof(BYTE), (DWORD)tmp, fp)) goto Err;

		result = fseek(fp, pInd->cfgOffset, SEEK_SET);
		if (result) goto Err;
		
		if (len != fwrite(buf, sizeof(BYTE), (DWORD)len, fp)) goto Err;
		if (tmp != fwrite(pkgFileBuf, sizeof(BYTE), (DWORD)tmp, fp)) goto Err;		

		deltalen = len - pInd->cfgLen;
		pkglen = pIndHead->pkgInfo[i].pkgLen + deltalen; 

	    //pkg文件处理
		result = fseek(fp, 0, SEEK_SET);
		if (result) goto Err;
		memset(&pkgHead, 0, sizeof(pkgHead));
		pkgHead.nLength = (DWORD)pkglen;
		pkgHead.wVersion = pIndHead->pkgInfo[i].pkgVer;
		if (sizeof(struct VPPkgHead) != fwrite(&pkgHead, sizeof(BYTE), sizeof(struct VPPkgHead), fp)) 
			goto Err;
		
		pInd->cfgLen = len;	

        //因为产生了数据移动,要对该pkg包含的ind中偏移进行更新
        pIndtmp = pFstIndInfo;
		for (j=0; j<pIndHead->indNum; j++)
		{
			if ((pIndtmp->pkgIndex==pInd->pkgIndex) && (pIndtmp->cfgOffset>=deltaOffset))
				pIndtmp->cfgOffset += deltalen;
			pIndtmp++;
		}
	}
	else
	{
		result = fseek(fp, pInd->cfgOffset, SEEK_SET);
		if (result) goto Err;
		
		if (len != fwrite(buf, sizeof(BYTE), (DWORD)len, fp)) goto Err;	

		if (!overwrite)
		{
			pkglen = pIndHead->pkgInfo[i].pkgLen+len;

			//pkg文件处理
			result = fseek(fp, 0, SEEK_SET);
			if (result) goto Err;
			memset(&pkgHead, 0, sizeof(pkgHead));
			pkgHead.nLength = (DWORD)pkglen;
			pkgHead.wVersion = pIndHead->pkgInfo[i].pkgVer;
			if (sizeof(struct VPPkgHead) != fwrite(&pkgHead, sizeof(BYTE), sizeof(struct VPPkgHead), fp)) 
				goto Err;			
		}
		else
			pkglen = pIndHead->pkgInfo[i].pkgLen;
	}

	fseek(fp, 0, SEEK_SET);
	fread(pkgFileBuf, sizeof(BYTE), (DWORD)pkglen, fp);
		
	fseek(fp, pkglen, SEEK_SET);
	pkgcrc = GetParaCrc16((BYTE *)pkgFileBuf, pkglen);
	if (sizeof(pkgcrc) != fwrite(&pkgcrc, sizeof(BYTE), sizeof(pkgcrc), fp)) 
		goto Err;			

	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
    //pkgInfo更新
    pIndHead->pkgInfo[i].pkgLen = pkglen;
	pIndHead->pkgInfo[i].pkgCrc = pkgcrc;
    pIndHead->pkgInfo[i].pkgValid = 1;
	
    //indInfo更新
    if (!find) 
	{
		strcpy(pInd->cfgName, filename);
		pIndHead->indNum++;
		pIndHead->nLength = sizeof(struct VPIndHead)+pIndHead->indNum*sizeof(struct VPIndInfo);
    }	
	pInd->cfgVer = CFG_FILE_VER;
	pInd->cfgCrc = cfgcrc;
	indcrc =  GetParaCrc16((BYTE *)pIndHead, pIndHead->nLength);
    //防止写system.cin失败,先置其校验为0
	indFileBuf[pIndHead->nLength] = 0;
    indFileBuf[pIndHead->nLength+1] = 0;	

	GetMyPath(path, CFG_IND_FILENAME);
	if ((fp=fopen(path, "wb")) == NULL) goto Err;	
	fseek(fp, 0, SEEK_SET);
    fwrite(pIndHead, sizeof(BYTE), (DWORD)pIndHead->nLength, fp);
	fwrite(&indcrc, sizeof(BYTE), sizeof(indcrc), fp);
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
    indFileBuf[pIndHead->nLength] = HIBYTE(indcrc);
    indFileBuf[pIndHead->nLength+1] = LOBYTE(indcrc);	

	smMGive(paraRWSemID);
	return(OK);

Err:
    if (fp) fclose(fp);
	smMGive(paraRWSemID);
	return ERROR;		
}  

int DelParaFile(const char *filename)
{
	FILE *fp;
	struct VPIndInfo *pInd, *pIndtmp;
	struct VPPkgHead pkgHead;
	int i, j, k, result; 
	int len, tmp, pkglen, deltalen, deltaOffset;
	WORD pkgcrc, indcrc;
	char path[4*MAXFILENAME];	

    if (strcmp(filename, "addr.cfg") == 0)  return(ERROR);
		    
	smMTake(paraRWSemID); 

	pInd = pFstIndInfo;
	for (k=0; k<pIndHead->indNum; k++)
	{
		if (strcmp(pInd->cfgName, filename) == 0)  
			break;
		pInd++;
	}

    if (k == pIndHead->indNum) 
	{
		smMGive(paraRWSemID); 
		return OK;
    }	

	i = pInd->pkgIndex;
	GetMyPath(path, pIndHead->pkgInfo[i].pkgName);

	if ((fp=fopen(path, "r+b")) == NULL) goto Err;

    deltaOffset = pInd->cfgOffset+pInd->cfgLen;

	result = fseek(fp, deltaOffset, SEEK_SET);
	if (result) goto Err;

	//后面部分的长度
	tmp = pIndHead->pkgInfo[i].pkgLen-deltaOffset;
	if (tmp > (int)sizeof(pkgFileBuf)) goto Err;		
	if (tmp != fread(pkgFileBuf, sizeof(BYTE), (DWORD)tmp, fp)) goto Err;

	result = fseek(fp, pInd->cfgOffset, SEEK_SET);
	if (result) goto Err;
	
	if (tmp != fwrite(pkgFileBuf, sizeof(BYTE), (DWORD)tmp, fp)) goto Err;		

	deltalen = 0 - pInd->cfgLen;
	pkglen = pIndHead->pkgInfo[i].pkgLen + deltalen; 

    //pkg文件处理
	result = fseek(fp, 0, SEEK_SET);
	if (result) goto Err;
	memset(&pkgHead, 0, sizeof(pkgHead));
	pkgHead.nLength = (DWORD)pkglen;
	pkgHead.wVersion = pIndHead->pkgInfo[i].pkgVer;
	if (sizeof(struct VPPkgHead) != fwrite(&pkgHead, sizeof(BYTE), sizeof(struct VPPkgHead), fp)) 
		goto Err;
	
    //因为产生了数据移动,要对该pkg包含的ind中偏移进行更新
    pIndtmp = pFstIndInfo;
	for (j=0; j<pIndHead->indNum; j++)
	{
		if ((pIndtmp->pkgIndex==pInd->pkgIndex) && (pIndtmp->cfgOffset>=deltaOffset))
			pIndtmp->cfgOffset += deltalen;
		pIndtmp++;
	}

	fseek(fp, 0, SEEK_SET);
	fread(pkgFileBuf, sizeof(BYTE), (DWORD)pkglen, fp);
		
	fseek(fp, pkglen, SEEK_SET);
	pkgcrc = GetParaCrc16((BYTE *)pkgFileBuf, pkglen);
	if (sizeof(pkgcrc) != fwrite(&pkgcrc, sizeof(BYTE), sizeof(pkgcrc), fp)) 
		goto Err;			
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
	
    //pkgInfo更新
    pIndHead->pkgInfo[i].pkgLen = pkglen;
	pIndHead->pkgInfo[i].pkgCrc = pkgcrc;
    pIndHead->pkgInfo[i].pkgValid = 1;

    len = (pIndHead->indNum-(k+1))*((int)sizeof(struct VPIndInfo));
    memcpy(pInd, pInd+1, (DWORD)len);
	pIndHead->indNum--;
	pIndHead->nLength = sizeof(struct VPIndHead)+pIndHead->indNum*sizeof(struct VPIndInfo);

	indcrc =  GetParaCrc16((BYTE *)pIndHead, pIndHead->nLength);
    //防止写system.cin失败,先置其校验为0
	indFileBuf[pIndHead->nLength] = 0;
    indFileBuf[pIndHead->nLength+1] = 0;	

	GetMyPath(path, CFG_IND_FILENAME);
	if ((fp=fopen(path, "wb")) == NULL) goto Err;	
	fseek(fp, 0, SEEK_SET);
    fwrite(pIndHead, sizeof(BYTE), (DWORD)pIndHead->nLength, fp);
	fwrite(&indcrc, sizeof(BYTE), sizeof(indcrc), fp);
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
    indFileBuf[pIndHead->nLength] = HIBYTE(indcrc);
    indFileBuf[pIndHead->nLength+1] = LOBYTE(indcrc);	

	smMGive(paraRWSemID);
	return(OK);

Err:
    if (fp) fclose(fp);
	smMGive(paraRWSemID);
	return ERROR;		
}  

int ReadParaFileCrc(const char *filename, WORD *crc)
{
	struct VPIndInfo *pInd;
	int i; 

    if (strcmp(filename, "addr.cfg") == 0)
	{
		*crc = 0;
		return OK;
    }	

	smMTake(paraRWSemID); 

	pInd = pFstIndInfo;
	for (i=0; i<pIndHead->indNum; i++)
	{
		if (strcmp(pInd->cfgName, filename) == 0)  
			break;
		pInd++;
	}
    if (i == pIndHead->indNum) goto Err;

    *crc = pInd->cfgCrc;
	
	smMGive(paraRWSemID);

    return(OK);  
	
Err:
    *crc = 0;
	smMGive(paraRWSemID);
	return ERROR;	
}  

/*------------------------------------------------------------------------
 Procedure:     GetParaFileName ID:1
 Purpose:       取得全路径的参数文件名
 Input:
 Output:        
 Errors:
------------------------------------------------------------------------*/
void  GetParaFileName(const char *path,const char *name, const char *suffix,char *pathname)
{   
   if ((path==NULL) || (strcmp(path,"")==0))
   {
     strcpy(pathname,name);
     strcat(pathname,suffix);
   }  
   else
   {      
      strcpy(pathname,myPathRoot);
      strcat(pathname,myPath[DIR_SUBCFG]);
      strcat(pathname,"/");
      strcat(pathname,path);
      MakeDir(pathname);     //????
      strcat(pathname,"/");
      strcat(pathname,name);
      strcat(pathname,suffix);    
   }  	
}   

int GetAppPortId(int para_port_id)
{
    if (para_port_id < PARA_START_NO)
		return(para_port_id);
	else if ((para_port_id >= PARA_START_NO) && (para_port_id < PARA_NET_START_NO))
		return (COMM_START_NO+para_port_id-PARA_START_NO);
	else if ((para_port_id >= PARA_NET_START_NO) && (para_port_id < PARA_NET_USR_NO))
		return (COMM_NET_START_NO+para_port_id-PARA_NET_START_NO);
	else if (para_port_id >= PARA_NET_USR_NO)
		return (COMM_NET_USR_NO+para_port_id-PARA_NET_USR_NO);
	else
		return 0;		
}

int GetParaPortId(int app_port_id)
{
    if (app_port_id < COMM_START_NO)
		return(app_port_id);
	else if ((app_port_id >= COMM_START_NO) && (app_port_id < COMM_NET_START_NO))
		return (PARA_START_NO+app_port_id-COMM_START_NO);
	else if ((app_port_id >= COMM_NET_START_NO) && (app_port_id < COMM_NET_USR_NO))
		return (PARA_NET_START_NO+app_port_id-COMM_NET_START_NO);
	else if (app_port_id >= COMM_NET_USR_NO)
		return (PARA_NET_USR_NO+app_port_id-COMM_NET_USR_NO);
	else
		return 0;		
}

int AddOneEqp(const char *path, const struct VPEqp *pPAddEqp)
{
	int i,wEqpNum;
	struct VPEqp *pVPEqp;
	char pathname[MAXFILENAME]; 

	GetParaFileName(path,"device",".cfg",pathname); 
	if (ReadParaFile(pathname,(BYTE *)g_pParafileTmp,MAXFILELEN)==ERROR)
		g_pParafileTmp->nLength = sizeof(struct VFileHead);

	wEqpNum=(g_pParafileTmp->nLength-(int)sizeof(struct VFileHead))/(int)sizeof(struct VPEqp);
	pVPEqp=(struct VPEqp *)(g_pParafileTmp+1);

	for (i=0;i<wEqpNum;i++)
	{
		//if there have the eqp alrealy then not add
		if ((pVPEqp->dwName==pPAddEqp->dwName)||(strcmp(pVPEqp->sCfgName,pPAddEqp->sCfgName)==0))
			break;     	
		pVPEqp++;
	}  
	if (i==wEqpNum)  //not exist so add 
	{
		wEqpNum++;
		g_pParafileTmp->nLength+=(int)sizeof(struct VPEqp);
		memcpy(pVPEqp,pPAddEqp,sizeof(struct VPEqp));
		pVPEqp->wCommID = (WORD)GetParaPortId(pVPEqp->wCommID);
		if (WriteParaFile(pathname,g_pParafileTmp)==OK)
			return(wEqpNum); 
		else			
			return(ERROR);
	}  
	else
		return(wEqpNum);     
}

#ifdef INCLUDE_FA
char* PFaYxPubTbl[] =
{
    "FA投入",
	"备用",
	"FA闭锁遥控",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
};
char* PFaYxTbl[] = 
{
	"开关%d联络",
	"开关%dFA启动",
	"开关%d跳闸",
	"开关%d合闸",
	"开关%d网络异常",
	"开关%dFA闭锁",
	"开关%d拒动",
	"开关%dFA结束",
	"开关%d反校失败",
	"开关%d相邻异常",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
};


int AddFaEqp(char* path, int fdnum)
{
    int i,j,len;
	struct VPEqp PEqp;  
	struct VPTECfg *pWrtPTECfg = (struct VPTECfg*)(g_pParafileTmp+1);
	struct VPTrueYX *pPSYX = (struct VPTrueYX*)(pWrtPTECfg+1);
	char pathname[MAXFILENAME]; 

    memset(pWrtPTECfg, 0, sizeof(struct VPTECfg));
	pWrtPTECfg->wSourceAddr = 1;
	pWrtPTECfg->wDesAddr = 1;		
	pWrtPTECfg->wSYXNum = FA_YX_PUB+fdnum*FA_YX_FD;

    len = 0;
    for (i=0; i<FA_YX_PUB; i++)
    {
        strcpy(pPSYX->sName, PFaYxPubTbl[i]);
		strcpy(pPSYX->sNameTab, pPSYX->sName);
			
		pPSYX->dwCfg = 1; 
		pPSYX++;

		len += sizeof(struct VPTrueYX);
    }
	
	for (i=0; i<fdnum; i++)
	{
	    for (j=0; j<FA_YX_FD; j++)
	    {
	        sprintf(pPSYX->sName, PFaYxTbl[j], i+1);
			strcpy(pPSYX->sNameTab, pPSYX->sName);
			
			pPSYX->dwCfg = 1; 
			pPSYX++;

			len += sizeof(struct VPTrueYX);
	    }

	}

	PEqp.dwName = CFG_ATTR_USEDEF<<12;
	strcpy(PEqp.sCfgName,"fa");
	PEqp.wCommID = FA_ID;
	
    g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPTECfg)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = PEqp.dwName>>12;
	GetParaFileName(path, PEqp.sCfgName, ".cde", pathname); 
	if (WriteParaFile(pathname,g_pParafileTmp)==OK)
		return(AddOneEqp(path, &PEqp));
	else			
		return(ERROR);
}
#endif
#ifdef INCLUDE_FA_SH
char* PFaYxTbl[] = 
{
    "保护动作",
	"FA启动",
	"本机故障",
	"隔离成功",
	"开关异常",
	"开关位置",
	"异常闭锁",
	"主站投退FA",
	"本地投退FA",
	"通信异常",
	"通信状态"
};

char* PFaYcTbl[] = 
{
	  "IaLeft",
	  "IbLeft",
	  "IcLeft",
	  "IaNeed",
	  "IbNeed",
	  "IcNeed",
};



int AddFaEqp(char* path, int fdnum)
{
    int i,len;
	struct VPEqp PEqp;  
	struct VPTECfg *pWrtPTECfg = (struct VPTECfg*)(g_pParafileTmp+1);
	struct VPTrueYX *pPSYX = (struct VPTrueYX*)(pWrtPTECfg+1);
	char pathname[MAXFILENAME]; 

    memset(pWrtPTECfg, 0, sizeof(struct VPTECfg));
	pWrtPTECfg->wSourceAddr = 1;
	pWrtPTECfg->wDesAddr = 1;		
	pWrtPTECfg->wSYXNum = FA_EQP_YX_NUM+1;

    len = 0;
    
    for (i=0; i<FA_EQP_YX_NUM+1; i++)
    {
        sprintf(pPSYX->sName, PFaYxTbl[i] );
		strcpy(pPSYX->sNameTab, pPSYX->sName);
		
		pPSYX->dwCfg = 1; 
		pPSYX++;

		len += sizeof(struct VPTrueYX);
    }

	PEqp.dwName = CFG_ATTR_USEDEF<<12;
	strcpy(PEqp.sCfgName,"MyFa");
	PEqp.wCommID = FA_ID;
	
    g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPTECfg)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = PEqp.dwName>>12;
	GetParaFileName(path, PEqp.sCfgName, ".cde", pathname); 
	if (WriteParaFile(pathname,g_pParafileTmp)==OK)
		return(AddOneEqp(path, &PEqp));
	else			
		return(ERROR);
}


int AddFaTrueEqp(char* path, int index, int flag)
{
    int i,j,len;
	struct VPEqp PEqp;  
	struct VPTECfg *pWrtPTECfg = (struct VPTECfg*)(g_pParafileTmp+1);
	struct VPTrueYC *pPYC = (struct VPTrueYC*)(pWrtPTECfg+1);
	struct VPTrueYX *pPSYX;
	struct VPTrueCtrl *pPYK;
	char pathname[MAXFILENAME]; 

    memset(pWrtPTECfg, 0, sizeof(struct VPTECfg));
	pWrtPTECfg->wSourceAddr = 2;
	pWrtPTECfg->wDesAddr = 2;		
	pWrtPTECfg->wSYXNum = FA_EQP_YX_NUM+1;
	pWrtPTECfg->wYCNum = FA_EQP_YC_NUM;

    len = 0;
	for (i=0; i<FA_EQP_YC_NUM; i++)
	{
	    strcpy(pPYC->sName, PFaYcTbl[i]);
		strcpy(pPYC->sNameTab, pPYC->sName);
			
		pPYC->dwCfg = 1; 
		pPYC++;

		len += sizeof(struct VPTrueYC);
	}

	pPSYX = (struct VPTrueYX*)pPYC;
	for (i=0; i<(FA_EQP_YX_NUM+1); i++)
	{
	    strcpy(pPSYX->sName, PFaYxTbl[i]);
		strcpy(pPSYX->sNameTab, pPSYX->sName);
			
		pPSYX->dwCfg = 1; 
		pPSYX++;

		len += sizeof(struct VPTrueYX);

	}

	if (flag)
	{
	    pWrtPTECfg->wYKNum = 1;
		pPYK = (struct VPTrueCtrl*)pPSYX;

		strcpy(pPYK->sName, "YK1");				
		strcpy(pPYK->sNameTab, "YK1");
		
		pPYK->dwCfg = 0; 
		pPYK->wID = 1;

		len += sizeof(struct VPTrueCtrl);
	}

	PEqp.dwName = (CFG_ATTR_FA<<12)+index;
	sprintf(PEqp.sCfgName, "FA%d", index);
	PEqp.wCommID = COMM_NET_FA_NO+index;
	
    g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPTECfg)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = PEqp.dwName>>12;
	GetParaFileName(path, PEqp.sCfgName, ".cde", pathname); 
	if (WriteParaFile(pathname,g_pParafileTmp)==OK)
		return(AddOneEqp(path, &PEqp));
	else			
		return(ERROR);
}

int AddFaVirtualEqp(char *path, int t_index, int v_index)
{
    int i,len;
    struct VPEqp PEqp;
	struct VPTECfg *pPTECfg = (struct VPTECfg*)(g_pParafile+1);
	struct VPVECfg *pWrtPVECfg = (struct VPVECfg*)(g_pParafileTmp+1);
	struct VPTrueYC *pPTYC=(struct VPTrueYC *)(pPTECfg+1);   
    struct VPVirtualYC *pPVYC=(struct VPVirtualYC *)(pWrtPVECfg+1);
	struct VPTrueYX *pPTSYX;
	struct VPVirtualYX *pPVSYX;
	char name[MAXFILENAME], pathname[MAXFILENAME]; 

    sprintf(name, "FA%d", t_index);
	GetParaFileName(path, name, ".cde", pathname); 
    if (ReadParaFile(pathname,(BYTE *)g_pParafile,MAXFILELEN)==ERROR) 
    	return(ERROR); 

	memset(pWrtPVECfg, 0, sizeof(struct VPVECfg));
	pWrtPVECfg->wSourceAddr = 2;
	pWrtPVECfg->wDesAddr = 2;

	len = 0;
	for (i=0; i<FA_EQP_YC_NUM; i++)
	{
		memset(pPVYC, 0, sizeof(struct VPVirtualYC));
		sprintf(pPVYC->sName, "%s-%s", name, pPTYC->sName);
		strcpy(pPVYC->sNameTab, pPTYC->sName);
		
		pPVYC->dwTEName = (CFG_ATTR_FA<<12)+t_index;
		pPVYC->wOffset = i;
		pPVYC->wSendNo = i;
		pPVYC->lA = 1;   
		pPVYC->lB = 1;   
		len+=sizeof(struct VPVirtualYC);
		pWrtPVECfg->wYCNum++;

		pPVYC++;
		pPTYC++;
		
	}

    pPTSYX = (struct VPTrueYX*)pPTYC;
	pPVSYX = (struct VPVirtualYX*)pPVYC;
	for (i=0; i<FA_EQP_YX_NUM-1; i++)
	{
	    memset(pPVSYX, 0, sizeof(struct VPVirtualYX));
		sprintf(pPVSYX->sName, "%s-%s", name, pPTSYX->sName);
		strcpy(pPVSYX->sNameTab, pPTSYX->sName);		
		pPVSYX->dwCfg = 0;   
		pPVSYX->dwTEName = (CFG_ATTR_FA<<12)+t_index;
		pPVSYX->wOffset = i;
		pPVSYX->wSendNo = i;
		pPTSYX++;
		pPVSYX++;
		len += sizeof(struct VPVirtualYX);     
		pWrtPVECfg->wSYXNum++;
	}


	PEqp.dwName = (CFG_ATTR_TABLE<<12)+FA_VEQP_INDEX+v_index;
	sprintf(PEqp.sCfgName, "FA%d", v_index);
	PEqp.wCommID = NULL;

	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPTECfg)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = PEqp.dwName>>12;
	GetParaFileName(path, PEqp.sCfgName, ".cdt", pathname); 
	if (WriteParaFile(pathname,g_pParafileTmp)==OK)
		return(AddOneEqp(path, &PEqp));
	else			
		return(ERROR);
	
}

int AddFaEqpGroup(char *path, int index)
{
	struct VPEqp PEqp;
	struct VPEGCfg *pWrtPEGCfg = (struct VPEGCfg *)(g_pParafileTmp+1);
	char pathname[MAXFILENAME];
	DWORD len = 0, dwEqpName;  
	BYTE *pname;

    PEqp.dwName = (CFG_ATTR_GROUP<<12)+FA_VEQP_INDEX+index;
	sprintf(PEqp.sCfgName, "FAg%d", index);
	PEqp.wCommID = COMM_NET_FA_NO+index;
	memset(ParaFileTmpBuf, 0, sizeof(ParaFileTmpBuf));

	pWrtPEGCfg=(struct VPEGCfg *)(g_pParafileTmp+1);

	pWrtPEGCfg->wSourceAddr = 2;
	pWrtPEGCfg->wDesAddr = 2;
	pWrtPEGCfg->wEqpNum = 1;
	pWrtPEGCfg->dwFlag = 0;


	pname=(BYTE *)(pWrtPEGCfg+1);
	dwEqpName = (CFG_ATTR_TABLE<<12)+FA_VEQP_INDEX+index;
	memcpy(pname, &dwEqpName, sizeof(DWORD));
    len += sizeof(DWORD);	

   	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPEGCfg)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = PEqp.dwName>>12;
	GetParaFileName(path, PEqp.sCfgName, ".cdg", pathname); 
	if (WriteParaFile(pathname, g_pParafileTmp) == OK)
		return(AddOneEqp(path, &PEqp));
	else
		return(ERROR);    

}

int AddFaPort(int index, char* ip, int port)
{
	struct VPort *pPort;
	struct VPPort PAddPort;
	
    memset(&PAddPort, 0, sizeof(PAddPort));
	PAddPort.id = COMM_NET_FA_NO+index;
	strcpy(PAddPort.pcol, "GB104sh");
	if (index <= 2)
	{
	    PAddPort.pcol_attr = 0;
		sprintf(PAddPort.cfgstr[0], "2:%s:%d", ip, port);
	}
    else
	{
	    PAddPort.pcol_attr = 1;
		sprintf(PAddPort.cfgstr[0], "1:%s:%d", ip, port);
    }
	PAddPort.bakmode = 0;
	PAddPort.bakcomm = 0;		
	return(AddOnePort(NULL, &PAddPort));
}
#endif

#ifdef INCLUDE_PR_DIFF
#define XLFAYX_TR_FLAG1   0
#define XLFAYX_TR_FLAG2   17
#define XLFAYX_BJ_FLAG    17
#define XLFAYX_BS_FLAG    33
#define XLFAYX_BHKR_FLAG  40
#define XLFAYX_YXKR_FLAG  56

#define FA_DIFF_YC_NUM  29
#define FA_DIFF_YX_NUM  192

char* PXlFadiffYcTbl[] = 
{
	"yc_iam",
	"yc_ibm",
	"yc_icm",
	"yc_i0",
	"yc_ua",
	"yc_ub",
	"yc_uc",
	"yc_uab",
	"yc_ubc",
	"yc_uca",
	"yc_3u0",
	"fabs1",
	"yc_p",
	"yc_q",
	"cos_fi",
	"phase_uaia",
	"phase_ubib",
	"phase_ucic",
	"phase_uaub",
	"phase_ubuc",
	"phase_ucua",
	"phase_uxua",
	"phase_u0i0",
	"phase_iaib",
	"phase_ibic",
	"phase_icia",
	"phase_uaiam",
	"phase_ubibm",
	"phase_ucicm",
};

char* PXlFadiffYxTbl[] = 
{
	//h1=tr_flag1; 17个,15个备用
	"整组起动",	   
	"重合闸动作", 	
	"过流Ⅰ段动作",	 
	"过流Ⅱ段动作",	 
	"过流Ⅲ段动作",	 
	"过流反时限动作", 
	"过流加速动作",	 
	"PT断线相过流",	
	"零序Ⅰ段动作",	 
	"零序Ⅱ段动作",	 
	"零序间歇动作",	 
	"零序反时限动作", 
	"零序加速动作",	 
	"低频减载动作",	 
	"差动动作",	   
	"过负荷动作", 	
	"低压减载动作",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",

	//tr_flag2 ，32个备用
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	//bj_flag 16个，16个备用
	"装置报警",	 
	"事故总",		
	"TWJ异常",	   
	"线路电压报警", 
	"频率异常",	 
	"PT断线",	   
	"控制回路断线", 
	"零序过流报警", 
	"过负荷告警",   
	"识别码错误",   
	"弹簧未储能",   
	"CT断线",	   
	"差流告警",	 
	"通讯异常",	 
	"goose跳闸", 	
	"接地报警",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",

	//bs_flag 7个，25个备用
	"装置闭锁", 
	"定值出错", 
	"RAM", 	 
	"RoM", 	 
	"电源",	   
	"出口回路", 
	"CPLD",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",


	//bhkr_flag 16个，16个备用
	"TWJ", 			  
	"HWJ",			   
	"KKJ",			  
	"遥控电源",		   
	"差动保护投入", 	 
	"低频减载投入", 	 
	"低压减载投入", 	 
	"停用重合闸",		
	"手合同期开入", 	 
	"弹簧未储能",			 
	"信号复归",			  
	"装置检修",			  
	"GPS失步",			 
	"重合闸充电",		
	"跳闸保持信号", 	 
	"合闸保持信号",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",

	//yxkr_flag 27，5个备用，1个通信
	"断路器位置",
	"kr1",   
	"kr2",   
	"kr3",   
	"kr4",   
	"kr5",   
	"kr6",   
	"kr7",   
	"kr8",   
	"kr9",   
	"kr10",  
	"kr11",			  
	"kr12",			  
	"kr13",			  
	"差动A相有效",	   
	"差动B相有效",	   
	"差动C相有效",	   
	"过流I 段有效",	   
	"过流II段有效",	   
	"过流III段有效",	   
	"过流加速段有效",	 
	"零序I 段有效",  
	"零序II段有效",  
	"零序加速段有效",
	"低频减载有效", 
	"低压减载有效",  
	"重合闸有效",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"通讯遥信",
};
	
int AddFadiffTrueEqp(char* path, int index)
{
	int i,len;
	struct VPEqp PEqp;  
	struct VPTECfg *pWrtPTECfg = (struct VPTECfg*)(g_pParafileTmp+1);
	struct VPTrueYC *pPYC = (struct VPTrueYC*)(pWrtPTECfg+1);
	struct VPTrueYX *pPSYX;
	char pathname[MAXFILENAME]; 
	
	memset(pWrtPTECfg, 0, sizeof(struct VPTECfg));
	pWrtPTECfg->wSourceAddr = 1;
	pWrtPTECfg->wDesAddr = 1;		
	pWrtPTECfg->wSYXNum = FA_DIFF_YX_NUM+1;
	pWrtPTECfg->wYCNum = FA_DIFF_YC_NUM;

	len = 0;
	
	for (i=0; i<FA_DIFF_YC_NUM; i++)
	{
		strcpy(pPYC->sName, PXlFadiffYcTbl[i]);
		strcpy(pPYC->sNameTab, pPYC->sName);
		pPYC->dwCfg = 1; 
		pPYC->lA = 1;
		pPYC->lB = 1;
		pPYC->lC = 0;
		pPYC++;
		len += sizeof(struct VPTrueYC);
	}
	
	pPSYX = (struct VPTrueYX*)pPYC;
	for (i=0; i<(FA_DIFF_YX_NUM+1); i++)
	{
		strcpy(pPSYX->sName, PXlFadiffYxTbl[i]);
		strcpy(pPSYX->sNameTab, pPSYX->sName);
		pPSYX->dwCfg = 1; 
		pPSYX++;
		len += sizeof(struct VPTrueYX);
	}
	PEqp.dwName = (CFG_ATTR_FA<<12)+index;
	sprintf(PEqp.sCfgName, "diffpro%d", index);
	if(!index)
		PEqp.wCommID = DIFFPRO1_ID+index;
	else
		PEqp.wCommID = DIFFPRO0_ID+index-1;

	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPTECfg)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = PEqp.dwName>>12;
	GetParaFileName(path, PEqp.sCfgName, ".cde", pathname); 
	if (WriteParaFile(pathname,g_pParafileTmp)==OK)
	return(AddOneEqp(path, &PEqp));
	else			
	return(ERROR);
}
#endif

#ifdef INCLUDE_FA_DIFF
char* PFaDiffYxPubTbl[] =
{
    "FADiff投入",
	"方向拓扑压板",
	"馈线保护压板",
	"断路器失灵压板",
	"恢复压板",
	"网络异常",
	"FADiff闭锁",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
};
char* PFaDiffYxTbl[] = 
{
	"开关%d联络",
	"开关%d母线故障",
	"开关%d线路故障",
	"开关%d方向拓扑故障",
	"开关%d馈线故障",
	"开关%d后备跳闸",
	"开关%d跳闸",
	"开关%d合闸",
	"开关%d拒动",
	"开关%d相邻异常",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
};


int AddFaDiffEqp(char* path, int fdnum)
{
    int i,j,len;
	struct VPEqp PEqp;  
	struct VPTECfg *pWrtPTECfg = (struct VPTECfg*)(g_pParafileTmp+1);
	struct VPTrueYX *pPSYX = (struct VPTrueYX*)(pWrtPTECfg+1);
	char pathname[MAXFILENAME]; 

    memset(pWrtPTECfg, 0, sizeof(struct VPTECfg));
	pWrtPTECfg->wSourceAddr = 1;
	pWrtPTECfg->wDesAddr = 1;		
	pWrtPTECfg->wSYXNum = FADIFF_YX_PUB+fdnum*FADIFF_YX_FD;

    len = 0;
    for (i=0; i<FADIFF_YX_PUB; i++)
    {
        strcpy(pPSYX->sName, PFaDiffYxPubTbl[i]);
		strcpy(pPSYX->sNameTab, pPSYX->sName);
			
		pPSYX->dwCfg = 1; 
		pPSYX++;

		len += sizeof(struct VPTrueYX);
    }
	
	for (i=0; i<fdnum; i++)
	{
	    for (j=0; j<FADIFF_YX_FD; j++)
	    {
	        sprintf(pPSYX->sName, PFaDiffYxTbl[j], i+1);
			strcpy(pPSYX->sNameTab, pPSYX->sName);
			
			pPSYX->dwCfg = 1; 
			pPSYX++;

			len += sizeof(struct VPTrueYX);
	    }

	}

	PEqp.dwName = CFG_ATTR_USEDEF<<12;
	strcpy(PEqp.sCfgName,"faDiff");
	PEqp.wCommID = FADIFF_ID;
	
    g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPTECfg)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = PEqp.dwName>>12;
	GetParaFileName(path, PEqp.sCfgName, ".cde", pathname); 
	if (WriteParaFile(pathname,g_pParafileTmp)==OK)
		return(AddOneEqp(path, &PEqp));
	else			
		return(ERROR);
}
#endif

#ifdef INCLUDE_SOC //两个线差，1个FA

#define LinePr_YC_NUM  29
#define LinePr_YX_NUM  18

#define SOCFA_YX_NUM  32

char* PLinePrYcTbl[LinePr_YC_NUM] = 
{
	"yc_iam",
	"yc_ibm",
	"yc_icm",
	"yc_i0",
	"yc_ua",
	"yc_ub",
	"yc_uc",
	"yc_uab",
	"yc_ubc",
	"yc_uca",
	"yc_3u0",
	"fabs1",
	"yc_p",
	"yc_q",
	"cos_fi",
	"phase_uaia",
	"phase_ubib",
	"phase_ucic",
	"phase_uaub",
	"phase_ubuc",
	"phase_ucua",
	"phase_uxua",
	"phase_u0i0",
	"phase_iaib",
	"phase_ibic",
	"phase_icia",
	"phase_uaiam",
	"phase_ubibm",
	"phase_ucicm",
};

char* PLinePrYxTbl[LinePr_YX_NUM] = 
{
	"整组起动",	   
	"重合闸动作", 	
	"过流Ⅰ段动作",	 
	"过流Ⅱ段动作",	 
	"过流Ⅲ段动作",	 
	"过流反时限动作", 
	"过流加速动作",	 
	"PT断线相过流",	
	"零序Ⅰ段动作",	 
	"零序Ⅱ段动作",	 
	"零序间歇动作",	 
	"零序反时限动作", 
	"零序加速动作",	 
	"低频减载动作",	 
	"差动动作",	   
	"过负荷动作", 	
	"低压减载动作",
	"通讯遥信",
};

char* PSocFaYxTbl[SOCFA_YX_NUM] = 
{
	"开关%d联络",
	"开关%d母线故障",
	"开关%d线路故障",
	"开关%d方向拓扑故障",
	"开关%d馈线故障",
	"开关%d后备跳闸",
	"开关%d跳闸",
	"开关%d合闸",
	"开关%d拒动",
	"开关%d相邻异常",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"备用",
	"通讯遥信",
};

int AddLinePrTrueEqp(char* path, int index)
{
	int i,len;
	struct VPEqp PEqp;  
	struct VPTECfg *pWrtPTECfg = (struct VPTECfg*)(g_pParafileTmp+1);
	struct VPTrueYC *pPYC = (struct VPTrueYC*)(pWrtPTECfg+1);
	struct VPTrueYX *pPSYX;
	char pathname[MAXFILENAME]; 
	
	memset(pWrtPTECfg, 0, sizeof(struct VPTECfg));
	pWrtPTECfg->wSourceAddr = 1;
	pWrtPTECfg->wDesAddr = 1;		
	pWrtPTECfg->wSYXNum = LinePr_YX_NUM;
	pWrtPTECfg->wYCNum = LinePr_YC_NUM;

	len = 0;
	
	for (i=0; i<LinePr_YC_NUM; i++)
	{
		strcpy(pPYC->sName, PLinePrYcTbl[i]);
		strcpy(pPYC->sNameTab, pPYC->sName);
		pPYC->dwCfg = 1; 
		pPYC->lA = 1;
		pPYC->lB = 1;
		pPYC->lC = 0;
		pPYC++;
		len += sizeof(struct VPTrueYC);
	}
	
	pPSYX = (struct VPTrueYX*)pPYC;
	for (i=0; i<(LinePr_YX_NUM); i++)
	{
		strcpy(pPSYX->sName, PLinePrYxTbl[i]);
		strcpy(pPSYX->sNameTab, pPSYX->sName);
		pPSYX->dwCfg = 1; 
		pPSYX++;
		len += sizeof(struct VPTrueYX);
	}
	PEqp.dwName = (CFG_ATTR_USEDEF<<12)+index;
	sprintf(PEqp.sCfgName, "linepro%d", index);
	
	PEqp.wCommID = LINEPRO0_ID+index;

	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPTECfg)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = PEqp.dwName>>12;
	GetParaFileName(path, PEqp.sCfgName, ".cde", pathname); 
	if (WriteParaFile(pathname,g_pParafileTmp)==OK)
	return(AddOneEqp(path, &PEqp));
	else			
	return(ERROR);
}

int AddFaSocTrueEqp(char* path, int fdnum)
{
	int i,len;
	struct VPEqp PEqp;  
	struct VPTECfg *pWrtPTECfg = (struct VPTECfg*)(g_pParafileTmp+1);
	struct VPTrueYX *pPSYX = (struct VPTrueYX*)(pWrtPTECfg+1);
	char pathname[MAXFILENAME]; 

    memset(pWrtPTECfg, 0, sizeof(struct VPTECfg));
	pWrtPTECfg->wSourceAddr = 1;
	pWrtPTECfg->wDesAddr = 1;		
	pWrtPTECfg->wSYXNum = SOCFA_YX_NUM;

    len = 0;
    for (i=0; i<SOCFA_YX_NUM; i++)
    {
        strcpy(pPSYX->sName, PSocFaYxTbl[i]);
		strcpy(pPSYX->sNameTab, pPSYX->sName);
			
		pPSYX->dwCfg = 1; 
		pPSYX++;

		len += sizeof(struct VPTrueYX);
    }
	

	PEqp.dwName = CFG_ATTR_FA<<12;
	strcpy(PEqp.sCfgName,"faSoc");
	PEqp.wCommID = FADIFF_ID;
	
    g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPTECfg)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = PEqp.dwName>>12;
	GetParaFileName(path, PEqp.sCfgName, ".cde", pathname); 
	if (WriteParaFile(pathname,g_pParafileTmp)==OK)
		return(AddOneEqp(path, &PEqp));
	else			
		return(ERROR);
}
#endif

int AddYcInMyIo(int yccfg_num, struct VPYcCfg *pCfg, struct VPTrueYC *pBuf, int *fdnum)
{
    int type,num;
	int i, j, k ,fd;
	char name[2*GEN_NAME_LEN];
	VDefTVYcCfg *pDefYcCfg;
	struct VPTrueYC *pPYC = (struct VPTrueYC *)pBuf;
#if (DEV_SP == DEV_SP_TTU)
		char tempno[20];
	char* uppstr[3] ={"Upp[ua]",  "Upp[ub]","Upp[uc]"};
    char* unbstr[2] ={"unbU[%d]", "unbI[%d]",};
	
    char* seqstr[6] ={"UPhZ[%d]", "UPhF[%d]", "UPh0[%d]",
                      "IPhZ[%d]", "IPhF[%d]", "IPh0[%d]",};

	char* thdstr[6] ={"Thd[ua]",  "Thd[ub]", "Thd[uc]",
					  "Thd[ia]",  "Thd[ib]", "Thd[ic]"};

	char* Ustr1[3] =  {"Udot[ua]", "Udot[ub]","Udot[uc]"};
	char* Ustr2[3] =  {"Udut[ua]", "Udut[ub]","Udut[uc]"};
	char* Ustr3[3] =  {"Umqr[ua]", "Umqr[ub]","Umqr[uc]"};
	char* Ustr4[3] =  {"Umot[ua]", "Umot[ub]","Umot[uc]"};
	char* Ustr5[3] =  {"Umut[ua]", "Umut[ub]","Umut[uc]"};
	char* Ustr6[3] =  {"Uavg[ua]", "Uavg[ub]","Uavg[uc]"};
#endif

	pDefYcCfg = GetDefTVYcCfg(0, &num);

	fd = 0;
    for (i=0; i<yccfg_num; i++)
    {
		if (pCfg[i].type == YC_TYPE_Ua)  /*电压,新回线开始*/
		{
			fd++;
		}	
		else if (pCfg[i].type == YC_TYPE_Ia) /*电流*/
		{
			if (i > 0)   
			{
                /*前值是线电压*/
				if ((pCfg[i-1].type>=YC_TYPE_Uab) && (pCfg[i-1].type<=YC_TYPE_Uca))
				{
					k = pCfg[i-1].arg1;  /*判断之前有无电流段*/
					for (j=k; j<(i-1); j++)
					{
						if (((pCfg[j].type >= YC_TYPE_Ia) && (pCfg[j].type <= YC_TYPE_SI0)) || \
						    ((pCfg[j].type >= YC_TYPE_SIa) && (pCfg[j].type <= YC_TYPE_SIc)))
						{
							fd++;    /*有电流段,新回线*/
							break;
						}
					}
						
				}
				else if (!(((pCfg[i-1].type >= YC_TYPE_Ua) && (pCfg[i-1].type <= YC_TYPE_SU0)) || \
					       ((pCfg[i-1].type >= YC_TYPE_SUa) && (pCfg[i-1].type <= YC_TYPE_SUc))))
					fd++;
			}
			else
				fd++;
		}
		memset(pPYC, 0, sizeof(struct VPTrueYC));

		type = pCfg[i].type - YC_TYPE_Dc;
#if(DEV_SP == DEV_SP_TTU)
		if(type == YC_TYPE_Angle)
		{
			type = pCfg[pCfg[i].arg2].type - YC_TYPE_Dc;
			sprintf(tempno, pDefYcCfg[type].name, fd);
			sprintf(name,"angle[1]_");
			strcat(name,tempno);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(tempno, pDefYcCfg[type].name_tab, fd);
			sprintf(name,"angle[1]_");
			strcat(name,tempno);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
			type = pCfg[i].type - YC_TYPE_Dc;
		}
		else if(type == YC_TYPE_Har1)
		{
			type = pCfg[pCfg[i].arg1].type - YC_TYPE_Dc;
			sprintf(name, pDefYcCfg[type].name, fd);
			strcat(name,"_har1[1]");
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, pDefYcCfg[type].name_tab, fd);
			strcat(name,"_har1[1]");
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if((type >= YC_TYPE_Har2) && (type <= YC_TYPE_Har13))
		{
			type = pCfg[pCfg[i].arg1].type - YC_TYPE_Dc;
			sprintf(name, pDefYcCfg[type].name, fd);
			sprintf(tempno, "_har%d[1]", (pCfg[i].type - YC_TYPE_Har2) + 2);
			strcat(name,tempno);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, pDefYcCfg[type].name_tab, fd);
			strcat(name,tempno);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if(type == YC_TYPE_Thd)
		{
			type = pCfg[pCfg[i].arg2].type  - YC_TYPE_Dc;
			if(pCfg[i].arg2>2)
				pCfg[i].arg2 -=1;
			strcpy(name,thdstr[pCfg[i].arg2]);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			strcpy(name,thdstr[pCfg[i].arg2]);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if((type >= YC_TYPE_Uunb) && (type <= YC_TYPE_Iunb))
		{
		    j = type - YC_TYPE_Uunb;
			type = pCfg[pCfg[g_Sys.MyCfg.pYc[i].arg1].arg1].type  - YC_TYPE_Dc;

			sprintf(name, unbstr[j], fd);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, unbstr[j], fd);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if(type == YC_TYPE_Upp)
		{
			type = pCfg[pCfg[i].arg1].type  - YC_TYPE_Dc;
			strcpy(name,uppstr[pCfg[i].arg1]);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			strcpy(name,uppstr[pCfg[i].arg1]);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if((type >= YC_TYPE_UPhZ) && (type <= YC_TYPE_IPh0))
		{
		    j = type - YC_TYPE_UPhZ;
			type = pCfg[pCfg[i].arg1].type - YC_TYPE_Dc;
			sprintf(name, seqstr[j], fd);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, seqstr[j], fd);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if(type == YC_TYPE_Lrate)
		{
			type = pCfg[pCfg[i].arg1].type  - YC_TYPE_Dc;
			sprintf(name, "Lrate[%d]", fd);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, "Lrate[%d]", fd);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if(type == YC_TYPE_Udot)
		{
		   	type = pCfg[pCfg[i].arg1].type  - YC_TYPE_Dc;
			strcpy(name,Ustr1[pCfg[i].arg1]);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			strcpy(name,Ustr1[pCfg[i].arg1]);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if(type == YC_TYPE_Udut)
		{
		   	type = pCfg[pCfg[i].arg1].type  - YC_TYPE_Dc;
			strcpy(name,Ustr2[pCfg[i].arg1]);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			strcpy(name,Ustr2[pCfg[i].arg1]);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if(type == YC_TYPE_Umqr)
		{
		   	type = pCfg[pCfg[i].arg1].type  - YC_TYPE_Dc;
			strcpy(name,Ustr3[pCfg[i].arg1]);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			strcpy(name,Ustr3[pCfg[i].arg1]);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if(type == YC_TYPE_Umot)
		{
		   	type = pCfg[pCfg[i].arg1].type  - YC_TYPE_Dc;
			strcpy(name,Ustr4[pCfg[i].arg1]);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			strcpy(name,Ustr4[pCfg[i].arg1]);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if(type == YC_TYPE_Umut)
		{
		   	type = pCfg[pCfg[i].arg1].type  - YC_TYPE_Dc;
			strcpy(name,Ustr5[pCfg[i].arg1]);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			strcpy(name,Ustr5[pCfg[i].arg1]);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else if(type == YC_TYPE_Uavg)
		{
		   	type = pCfg[pCfg[i].arg1].type  - YC_TYPE_Dc;
			strcpy(name,Ustr6[pCfg[i].arg1]);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			strcpy(name,Ustr6[pCfg[i].arg1]);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
		else
		{
			sprintf(name, pDefYcCfg[type].name, fd);
			strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
			pPYC->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, pDefYcCfg[type].name_tab, fd);
			strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		}
#else
		sprintf(name, pDefYcCfg[type].name, fd);
		strncpy(pPYC->sName, name, GEN_NAME_LEN-1);
		pPYC->sName[GEN_NAME_LEN-1] = '\0';
		sprintf(name, pDefYcCfg[type].name_tab, fd);
		strncpy(pPYC->sNameTab, name, GEN_NAMETAB_LEN-1);
		pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
#endif
		
		pPYC->dwCfg = pDefYcCfg[type].cfg;

		pPYC->lA = 1;	   /*系数	   缺省1*/
		pPYC->lB = 1;	   /*系数	   缺省1*/
		pPYC->lC = 0;	   /*修正值    缺省0	 最后运算 vlaue*a/b+c*/
		pPYC->wFrzKey = 0;
		pPYC->lLimit1 =0;
		pPYC->wLimitT1 =0;
		pPYC->wLimit1VYxNo =0;
		pPYC->lLimit2 =0;
		pPYC->wLimitT2 =0;
		pPYC->wLimit2VYxNo =0;	
		pPYC++;
    }

	pDefYcCfg = GetDefTVYcCfg_Public(0, &num);
	yccfg_num += num;

	for (i=0; i<num; i++)
	{
		memset(pPYC, 0, sizeof(struct VPTrueYC));
		
		strncpy(pPYC->sName, pDefYcCfg->name, GEN_NAME_LEN-1);
		pPYC->sName[GEN_NAME_LEN-1] = '\0';
		strncpy(pPYC->sNameTab, pDefYcCfg->name_tab, GEN_NAMETAB_LEN-1);
		pPYC->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		pPYC->dwCfg = pDefYcCfg->cfg;		

		pDefYcCfg++;
		
		pPYC->lA = 1;	   /*系数	   缺省1*/
		pPYC->lB = 1;	   /*系数	   缺省1*/
		pPYC->lC = 0;	   /*修正值    缺省0	 最后运算 vlaue*a/b+c*/
		pPYC->wFrzKey = 0;
		pPYC->lLimit1 =0;
		pPYC->wLimitT1 =0;
		pPYC->wLimit1VYxNo =0;
		pPYC->lLimit2 =0;
		pPYC->wLimitT2 =0;
		pPYC->wLimit2VYxNo =0;	
		pPYC++;
	}

	*fdnum = fd;

	return yccfg_num;

}

int AddDyxInMyio(struct VPTrueYX *pBuf)
{
    int i,j,num;
    int dyxcfg_num;
	BYTE type;
	VDefTVYxCfg *pDefDYxCfg;	
	char name[2*GEN_NAME_LEN];
	struct VPTrueYX *pPDYX = (struct VPTrueYX *)pBuf;

    dyxcfg_num  = 0;

	for (i=0; i<BSP_IO_BOARD; i++)
	{
	    type = Get_Io_Type(i);
		if (type == 0xFF) continue;
		pDefDYxCfg = GetDefTVDYxCfg(type, &num);
		if (pDefDYxCfg == NULL) continue;

		for (j=0; j<num; j++)
		{
			memset(pPDYX, 0, sizeof(struct VPTrueYX));

			sprintf(name, pDefDYxCfg->name, dyxcfg_num+j);
			strncpy(pPDYX->sName, name, GEN_NAME_LEN-1);
			pPDYX->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, pDefDYxCfg->name_tab, dyxcfg_num+j);
			strncpy(pPDYX->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPDYX->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
			pPDYX->dwCfg = 1; 

			pDefDYxCfg++;
			
			pPDYX++;
		}
		dyxcfg_num += num;	
	}

	return dyxcfg_num;

}

int AddSyxInMyio(struct VPTrueYX *pBuf, int fd)
{
    int i,j,num;
	BYTE type;
	int yxcfg_num;
    VDefTVYxCfg *pDefYxCfg;	
	VDefTVYxCfg_Fd *pDefYxCfg_Fd;
	char name[2*GEN_NAME_LEN];
    struct VPTrueYX *pPSYX=pBuf;
	BYTE fdnum=0;
	
	yxcfg_num =0;

	for (i=0; i<BSP_IO_BOARD; i++)
	{
	    type = Get_Io_Type(i);
		
		if (type == 0xFF) continue;
		pDefYxCfg = GetDefTVYxCfg_Di(type, &num);
		if (pDefYxCfg == NULL) continue;

		for (j=0; j<num; j++)
		{
			if((yxcfg_num+j)%6==0) fdnum++;
			memset(pPSYX, 0, sizeof(struct VPTrueYX));
			sprintf(name, pDefYxCfg->name, yxcfg_num+j,fdnum);
			
			strncpy(pPSYX->sName, name, GEN_NAME_LEN-1);
			pPSYX->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, pDefYxCfg->name_tab, yxcfg_num+j,fdnum);
			
			strncpy(pPSYX->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPSYX->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
			pPSYX->dwCfg = 1; 

			pDefYxCfg++;
			
			pPSYX++;
		}
		yxcfg_num += num;
		
		
	}

	pDefYxCfg = GetDefTVYxCfg_Public(0, &num);
	yxcfg_num += num;
	for (i=0; i<num; i++)
	{
		memset(pPSYX, 0, sizeof(struct VPTrueYX));

		strncpy(pPSYX->sName, pDefYxCfg->name, GEN_NAME_LEN-1);
		strncpy(pPSYX->sNameTab, pDefYxCfg->name_tab, GEN_NAMETAB_LEN-1);
		pPSYX->sName[GEN_NAME_LEN-1] = '\0';
		pPSYX->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		pPSYX->dwCfg = 1; 

		pDefYxCfg++;
		
		pPSYX++;
	}	

	for(j = 0; j < fd; j++) //每回线都有
	{
		pDefYxCfg_Fd = GetDefTVYxCfg_FdPublic(0, &num);
		yxcfg_num += num;

		for (i=0; i<num; i++)
		{
			memset(pPSYX, 0, sizeof(struct VPTrueYX));

			strncpy(pPSYX->sName, pDefYxCfg_Fd->name, GEN_NAME_LEN-1);
			sprintf(name,"[%d]",j+1);
			strcat(pPSYX->sName,name);
			pPSYX->sName[GEN_NAME_LEN-1] = '\0';
			strncpy(pPSYX->sNameTab, pDefYxCfg_Fd->name_tab, GEN_NAMETAB_LEN-1);
			sprintf(name,"%d",j+1);
			strcat(pPSYX->sNameTab,name);
			pPSYX->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
			pPSYX->dwCfg = 1; 

	        pDefYxCfg_Fd++;
			
			pPSYX++;
		}
	}
	
	pDefYxCfg_Fd = GetDefTVYxCfg_Fd(0, &num);
	if (pDefYxCfg == NULL) return yxcfg_num;

	for (i=0; i<fd; i++)
	{
		pDefYxCfg_Fd = GetDefTVYxCfg_Fd(0, &num);
	    for (j=0; j<num; j++)
		{
			memset(pPSYX, 0, sizeof(struct VPTrueYX));

			sprintf(name, pDefYxCfg_Fd->name, i+1);
			strncpy(pPSYX->sName, name, GEN_NAME_LEN-1);
			pPSYX->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, pDefYxCfg_Fd->name_tab, i+1);
			strncpy(pPSYX->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPSYX->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
			pPSYX->dwCfg = 1; 


		    pDefYxCfg_Fd++;
			pPSYX++;
		}
		yxcfg_num += num;
	}

	return yxcfg_num;
}

int AddDdInMyio(struct VPTrueDD *pBuf, int fd)
{
    int i, j, num, ddcfg_num;
	VDefTVDdCfg *pDefDdCfg;
	char name[2*GEN_NAME_LEN];
	struct VPTrueDD *pPDD = pBuf;

	pDefDdCfg = GetDefTVDdCfg(0, &num);
	if (pDefDdCfg == NULL) return 0;

	ddcfg_num = 0;
	
 	for (i=0; i<fd; i++)
    {
		
		pDefDdCfg = GetDefTVDdCfg(0, &num);
		
	    for (j=0; j<num; j++)
		{ 
	        memset(pPDD, 0, sizeof(struct VPTrueDD));

			sprintf(name, pDefDdCfg->name, i+1);
			strncpy(pPDD->sName, name, GEN_NAME_LEN-1);
			pPDD->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, pDefDdCfg->name_tab, i+1);
			strncpy(pPDD->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPDD->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
			pPDD->dwCfg = pDefDdCfg->cfg;

			pDefDdCfg++;
			
			pPDD->lA = 1;   /*系数      缺省1*/
			pPDD->lB = 1;   /*系数      缺省1*/
			pPDD->lC = 0;   /*修正值    缺省0     最后运算 vlaue*a/b+c*/
			pPDD->wFrzKey = 0;
			pPDD++;
	    }

		ddcfg_num += num;
    }

    return ddcfg_num;
}

int AddYkInMyio(struct VPTrueCtrl *pBuf, int fd)
{
    BYTE type;
	WORD ctrlid;
    int i, j, num, ykcfg_num;
	VDefTVYkCfg *pDefYkCfg;
	VDefTVYkCfg_Fd *pDefYkCfg_Fd;
	char name[2*GEN_NAME_LEN];
	struct VPTrueCtrl *pPYK = pBuf;

	ykcfg_num = 0;
	ctrlid = 1;

	for (i=0; i<BSP_IO_BOARD; i++)
    {
        type = Get_Io_Type(i);
		if (type == 0xFF) continue;

		pDefYkCfg = GetDefTVYkCfg_Do(type, &num);
		if (pDefYkCfg == NULL) continue;

		for (j=0; j<num; j++)
		{
			memset(pPYK, 0, sizeof(struct VPTrueCtrl));

			sprintf(name, pDefYkCfg->name, ctrlid);				
			strncpy(pPYK->sName, name, GEN_NAME_LEN-1);
			pPYK->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, pDefYkCfg->name_tab, ctrlid);
			strncpy(pPYK->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYK->sNameTab[GEN_NAMETAB_LEN-1] = '\0';

			pDefYkCfg++;
			
			pPYK->dwCfg = 0; 
			pPYK->wID = ctrlid;
			ctrlid++;
			pPYK++;
		}
		ykcfg_num += num;
    }	

	pDefYkCfg = GetDefTVYkCfg_Public(0, &num);
	ykcfg_num += num;
	for (i=0; i<num; i++)
	{
		memset(pPYK, 0, sizeof(struct VPTrueCtrl));

		strncpy(pPYK->sName, pDefYkCfg->name, GEN_NAME_LEN-1);
		pPYK->sName[GEN_NAME_LEN-1] = '\0';
		strncpy(pPYK->sNameTab, pDefYkCfg->name_tab, GEN_NAMETAB_LEN-1);
		pPYK->sNameTab[GEN_NAMETAB_LEN-1] = '\0';
		
		pDefYkCfg++;
		
		pPYK->dwCfg = 0; 
		pPYK->wID = ctrlid;
		ctrlid++;
		pPYK++;
	}

	pDefYkCfg_Fd = GetDefTVYkCfg_Fd(0, &num);
	if (pDefYkCfg_Fd == NULL) return ykcfg_num;

	for (i=0; i<fd; i++)
    {
    			
		pDefYkCfg_Fd = GetDefTVYkCfg_Fd(0, &num);
		ykcfg_num += num;

		for (j=0; j<num; j++)
		{
			memset(pPYK, 0, sizeof(struct VPTrueCtrl));

			sprintf(name, pDefYkCfg_Fd->name, i+1);
			strncpy(pPYK->sName, name, GEN_NAME_LEN-1);
			pPYK->sName[GEN_NAME_LEN-1] = '\0';
			sprintf(name, pDefYkCfg_Fd->name_tab, i+1);
			strncpy(pPYK->sNameTab, name, GEN_NAMETAB_LEN-1);
			pPYK->sNameTab[GEN_NAMETAB_LEN-1] = '\0';

			pDefYkCfg_Fd++;
			
			pPYK->dwCfg = 0; 
			pPYK->wID = ctrlid;
			ctrlid++;
			pPYK++;
		}
			
    }

    return ykcfg_num;
}

int AddMyIoTrueEqp(const char * path, struct VPSysConfig *pPSysCfg, const struct VPTECfg * pPTECfg, const struct VPEqp * pPAddEqp)
{
	int  fd,len;
    int  yccfg_num, dyxcfg_num, yxcfg_num, ddcfg_num, ykcfg_num, num;
    BYTE *pTemp;
	struct VPYcCfg *pPYcCfg;
	struct VPTECfg *pWrtPTECfg;
	struct VPTrueYC *pPYC;
	struct VPTrueYX *pPDYX;
	struct VPTrueYX *pPSYX;
	struct VPTrueDD *pPDD;
	struct VPTrueCtrl *pPYK;
	char pathname[MAXFILENAME]; 

    if (pPAddEqp->wCommID > THREAD_MAX_NUM) return(ERROR);   
	len = (MAXFILELEN/5)<<2;
	pPYcCfg = (struct VPYcCfg*)(ParaFileTmpBuf + len);
	pTemp = (BYTE*)pPSysCfg+sizeof(struct VPSysConfig)+
		        sizeof(struct VPFdCfg)*pPSysCfg->wFDNum+
		        sizeof(struct VPAiCfg)*pPSysCfg->wAINum+
		        sizeof(struct VPDiCfg)*pPSysCfg->wDINum+
		        sizeof(struct VPDoCfg)*pPSysCfg->wDONum;

	memcpy((BYTE*)pPYcCfg, pTemp, pPSysCfg->wYCNum*sizeof(struct VPYcCfg));

	yccfg_num = pPSysCfg->wYCNum-pPSysCfg->wYcNumPublic;
	
	memset(ParaFileTmpBuf, 0, (DWORD)len);
	pWrtPTECfg=(struct VPTECfg*)(g_pParafileTmp+1);
	memcpy(pWrtPTECfg, pPTECfg, sizeof(struct VPTECfg));

	pPYC=(struct VPTrueYC *)(pWrtPTECfg+1);
	
	fd = 0;
	len = 0;
	num = AddYcInMyIo(yccfg_num, pPYcCfg, pPYC, &fd);
    len += num*((int)sizeof(struct VPTrueYC));
	pPYC += num;
	yccfg_num = num;

	pPDYX=(struct VPTrueYX *)(pPYC);
	dyxcfg_num = AddDyxInMyio(pPDYX);
	len += dyxcfg_num*((int)sizeof(struct VPTrueYX));
	pPDYX += dyxcfg_num;

	pPSYX=(struct VPTrueYX *)(pPDYX);
	yxcfg_num = AddSyxInMyio(pPSYX, fd);
	len += yxcfg_num*((int)sizeof(struct VPTrueYX));
	pPSYX += yxcfg_num;

	pPDD=(struct VPTrueDD *)(pPSYX);
 	ddcfg_num = AddDdInMyio(pPDD, fd);
	len += ddcfg_num*((int)sizeof(struct VPTrueDD));
	pPDD += ddcfg_num;
	
	pPYK=(struct VPTrueCtrl *)(pPDD);
    ykcfg_num = AddYkInMyio(pPYK, fd);
	len += ykcfg_num*((int)sizeof(struct VPTrueCtrl));


	pWrtPTECfg->wYCNum = (WORD)yccfg_num;
	pWrtPTECfg->wDYXNum = (WORD)dyxcfg_num;	
	pWrtPTECfg->wSYXNum = (WORD)yxcfg_num;	
	pWrtPTECfg->wDDNum = (WORD)ddcfg_num; 
	pWrtPTECfg->wYKNum = (WORD)ykcfg_num; 
	if (len==0)  //NULL
		return(ERROR);
	else
	{
		g_pParafileTmp->nLength = (int)sizeof(struct VFileHead)+(int)sizeof(struct VPTECfg)+len;
		g_pParafileTmp->wVersion = CFG_FILE_VER;
		g_pParafileTmp->wAttr = (WORD)(pPAddEqp->dwName>>12);
		GetParaFileName(path, pPAddEqp->sCfgName, ".cde", pathname); 
		if (WriteParaFile(pathname,g_pParafileTmp)==OK)
			return(AddOneEqp(path, pPAddEqp));
		else			
			return(ERROR);
	}  

}

int AddOneVirtualEqp(const char *path, DWORD dwTEName, WORD wDesAddr, struct VPEqp *pPAddEqp)
{
	int wEqpNum,i;
	struct VPVECfg *pWrtPVECfg;
	struct VPTECfg *pPTECfg;
	struct VPEqp *pPEqp;
	int teqpCfgNameLen;
	char teqpCfgName[GEN_NAMETAB_LEN], name[GEN_NAME_LEN*2];
	char pathname[MAXFILENAME];
	int olddatalen,endpos,len;
	BYTE *p,*q;
	struct VPTrueYC *pPTYC;   
    struct VPVirtualYC *pPVYC;
    struct VPTrueYX *pPTDYX;
	struct VPTrueYX *pPTSYX;
	struct VPVirtualYX *pPVSYX;
	struct VPTrueDD *pPTDD;
	struct VPTrueCtrl *pPTYK ;
	struct VPVirtualCtrl *pPVYK;
	struct VPTrueCtrl *pPTYT;
	struct VPVirtualCtrl *pPVYT;

    pPAddEqp->wCommID = NULL_ID;  
   
    GetParaFileName(path,"device",".cfg",pathname); 
    if (ReadParaFile(pathname,(BYTE *)g_pParafileTmp,MAXFILELEN)==ERROR) 
        return(ERROR);   
    pPEqp=(struct VPEqp *)(g_pParafileTmp+1);
    wEqpNum=(g_pParafileTmp->nLength-(int)sizeof(struct VFileHead))/(int)sizeof(struct VPEqp);
   
	for (i=0;i<wEqpNum;i++)
	{
		if (pPEqp->dwName==dwTEName) break;
		pPEqp++;
	}
   
    if (i==wEqpNum) return(ERROR);
   
    GetParaFileName(path, pPEqp->sCfgName, ".cde", pathname); 
    if (ReadParaFile(pathname,(BYTE *)g_pParafile,MAXFILELEN)==ERROR) 
    	return(ERROR); 

	strcpy(teqpCfgName, pPEqp->sCfgName);
	teqpCfgNameLen = (int)strlen(teqpCfgName);
    pPTECfg=(struct VPTECfg *)(g_pParafile+1);

	memset(ParaFileTmpBuf, 0, sizeof(ParaFileTmpBuf));
   
    pWrtPVECfg=(struct VPVECfg *)(g_pParafileTmp+1);   
    GetParaFileName(path, pPAddEqp->sCfgName, ".cdt", pathname); 
    if (ReadParaFile(pathname,(BYTE *)g_pParafileTmp,MAXFILELEN)==ERROR)
    {
        memset(pWrtPVECfg, 0, sizeof(struct VPVECfg));
		pWrtPVECfg->wSourceAddr = pPTECfg->wSourceAddr;
		pWrtPVECfg->wDesAddr = wDesAddr;

        memset(g_pParafileTmp, 0, sizeof(struct VFileHead));
		g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPVECfg);
		g_pParafileTmp->wVersion = CFG_FILE_VER;
		g_pParafileTmp->wAttr = (WORD)(pPAddEqp->dwName>>12);
    }	

    olddatalen = g_pParafileTmp->nLength-((int)sizeof(struct VFileHead)+(int)sizeof(struct VPVECfg));
    endpos = g_pParafileTmp->nLength-1;

    pPTYC=(struct VPTrueYC *)(pPTECfg+1);   
    pPVYC=(struct VPVirtualYC *)(pWrtPVECfg+1);
    pPVYC += pWrtPVECfg->wYCNum;
    olddatalen -= (int)(pWrtPVECfg->wYCNum*sizeof(struct VPVirtualYC));      
    p = ParaFileTmpBuf+endpos;
    endpos += (int)(pPTECfg->wYCNum*sizeof(struct VPVirtualYC));
    if (endpos >= MAXFILELEN) return(ERROR);
	q = ParaFileTmpBuf+endpos;
    for (i=0;i<olddatalen;i++)
    {
        *q=*p;
        q--;
        p--;
    }     

	len = 0;
	for (i=0;i<pPTECfg->wYCNum+pPTECfg->wVYCNum;i++)
	{
        memset(pPVYC, 0, sizeof(struct VPVirtualYC));
		if ((DWORD)teqpCfgNameLen+strlen(pPTYC->sName)+1 >=  GEN_NAME_LEN)
			sprintf(name,"YC%d", pWrtPVECfg->wYCNum);
		else	
			sprintf(name,"%s-%s", pPTYC->sName,teqpCfgName);
		strcpy(pPVYC->sName, name);
		if ((DWORD)teqpCfgNameLen+strlen(pPTYC->sNameTab)+1 >=  GEN_NAMETAB_LEN)
			sprintf(name,"YC%d", pWrtPVECfg->wYCNum);
		else	
			sprintf(name,"%s-%s", pPTYC->sNameTab,teqpCfgName);
		strcpy(pPVYC->sNameTab, name);		
		pPVYC->dwCfg = 0;
		pPVYC->dwTEName = dwTEName;
		pPVYC->wOffset = (WORD)i;
		pPVYC->wSendNo = pWrtPVECfg->wYCNum;
		pPVYC->lA = 1;   
		pPVYC->lB = 1;   
		pPVYC->lC = 0;   
		pPVYC->wFrzKey = 0;
		pPTYC++;
		pPVYC++;
		len+=(int)sizeof(struct VPVirtualYC);
		pWrtPVECfg->wYCNum++;     
	}  

 /* pPTDYX=(struct VPTrueYX *)pPTYC;
	pPVDYX=(struct VPVirtualYX *)pPVYC;
	pPVDYX += pWrtPVECfg->wDYXNum;
	olddatalen -= pWrtPVECfg->wDYXNum*sizeof(struct VPVirtualYX);      
	p = ParaFileTmpBuf+endpos;
	endpos += pPTECfg->wDYXNum*sizeof(struct VPVirtualYX);
	if (endpos >= MAXFILELEN) return(ERROR);
	q = ParaFileTmpBuf+endpos;
	for (i=0; i<olddatalen; i++)
	{
		*q=*p;
		q--;
		p--;
	}     
	for (i=0;i<pPTECfg->wDYXNum;i++)
	{
    	memset(pPVDYX, 0, sizeof(struct VPVirtualYX));
		if (teqpCfgNameLen+strlen(pPTDYX->sName)+1 >=  GEN_NAME_LEN)
			sprintf(name,"DYX%d", pWrtPVECfg->wDYXNum);
		else	
			sprintf(name,"%s-%s", teqpCfgName, pPTDYX->sName);
		strcpy(pPVDYX->sName, name);
		if (teqpCfgNameLen+strlen(pPTDYX->sNameTab)+1 >=  GEN_NAMETAB_LEN)
			sprintf(name,"DYX%d", pWrtPVECfg->wDYXNum);
		else	
			sprintf(name,"%s-%s", teqpCfgName, pPTDYX->sNameTab);
		strcpy(pPVDYX->sNameTab, name);		
		pPVDYX->dwCfg = 0;   
		pPVDYX->dwTEName = dwTEName;
		pPVDYX->wOffset = i;
		pPVDYX->wSendNo = pWrtPVECfg->wDYXNum;
		pPTDYX++;
		pPVDYX++;
		len += sizeof(struct VPVirtualYX);     
		pWrtPVECfg->wDYXNum++;
	}  

	pPTSYX = (struct VPTrueYX *)pPTDYX;
	pPVSYX = (struct VPVirtualYX *)pPVDYX;
	pPVSYX += pWrtPVECfg->wSYXNum;*/

	pPTDYX=(struct VPTrueYX *)pPTYC;
	pPTDYX+= pPTECfg->wDYXNum;
	
	pPTSYX = (struct VPTrueYX *)pPTDYX;
	pPVSYX = (struct VPVirtualYX *)pPVYC;
	pPVSYX += pWrtPVECfg->wSYXNum;
	olddatalen -= (int)(pWrtPVECfg->wSYXNum*sizeof(struct VPVirtualYX));      
	p = ParaFileTmpBuf+endpos;
	endpos += (int)(pPTECfg->wSYXNum*sizeof(struct VPVirtualYX));
	if (endpos >= MAXFILELEN) return(ERROR);   
	q = ParaFileTmpBuf+endpos;
	for (i=0; i<olddatalen; i++)
	{
		*q=*p;
		q--;
		p--;
	}     
	for (i=0;i<pPTECfg->wSYXNum+pPTECfg->wVYXNum;i++)
	{
        memset(pPVSYX, 0, sizeof(struct VPVirtualYX));
		if ((DWORD)teqpCfgNameLen+strlen(pPTSYX->sName)+1 >=  GEN_NAME_LEN)
			sprintf(name,"YX%d", pWrtPVECfg->wSYXNum);
		else	
			sprintf(name,"%s-%s", teqpCfgName, pPTSYX->sName);
		strcpy(pPVSYX->sName, name);
		if ((DWORD)teqpCfgNameLen+strlen(pPTSYX->sNameTab)+1 >=  GEN_NAMETAB_LEN)
			sprintf(name,"YX%d", pWrtPVECfg->wSYXNum);
		else	
			sprintf(name,"%s-%s", teqpCfgName, pPTSYX->sNameTab);
		strcpy(pPVSYX->sNameTab, name);		
		pPVSYX->dwCfg=0;   
		pPVSYX->dwTEName=dwTEName;
		pPVSYX->wOffset=(WORD)i;
		pPVSYX->wSendNo=pWrtPVECfg->wSYXNum;
		pPTSYX++;
		pPVSYX++;
		len += (int)sizeof(struct VPVirtualYX);     
		pWrtPVECfg->wSYXNum++;
	}  

	/*pPTDD = (struct VPTrueDD *)pPTSYX;
	pPVDD = (struct VPVirtualDD *)pPVSYX;
	pPVDD += pWrtPVECfg->wDDNum;
	olddatalen -= pWrtPVECfg->wDDNum*sizeof(struct VPVirtualDD);      
	p = ParaFileTmpBuf+endpos;
	if (endpos >= MAXFILELEN) return(ERROR);      
	endpos += pPTECfg->wDDNum*sizeof(struct VPVirtualDD);   
	q = ParaFileTmpBuf+endpos;
	for (i=0; i<olddatalen; i++)
	{
		*q=*p;
		q--;
		p--;
	}     
	for (i=0;i<pPTECfg->wDDNum;i++)
	{
		memset(pPVDD, 0, sizeof(struct VPVirtualDD));
		if (teqpCfgNameLen+strlen(pPTDD->sName)+1 >=  GEN_NAME_LEN)
			sprintf(name,"DD%d", pWrtPVECfg->wDDNum);
		else	
			sprintf(name,"%s-%s", teqpCfgName, pPTDD->sName);
		strcpy(pPVDD->sName, name);
		if (teqpCfgNameLen+strlen(pPTDD->sNameTab)+1 >=  GEN_NAMETAB_LEN)
			sprintf(name,"DD%d", pWrtPVECfg->wDDNum);
		else	
			sprintf(name,"%s-%s", teqpCfgName, pPTDD->sNameTab);
		strcpy(pPVDD->sNameTab, name);		
		pPVDD->dwCfg=0;  
		pPVDD->dwTEName = dwTEName;
		pPVDD->wOffset = i;
		pPVDD->wSendNo = pWrtPVECfg->wDDNum;
		pPVDD->wFrzKey = 0;
		pPTDD++;		
		pPVDD++;
		len += sizeof(struct VPVirtualDD);     
		pWrtPVECfg->wDDNum++;
	}

	pPTYK = (struct VPTrueCtrl *)pPTDD;
	pPVYK = (struct VPVirtualCtrl *)pPVDD;
	*/  
	pPTDD = (struct VPTrueDD *)pPTSYX;
	pPTDD += pPTECfg->wDDNum;
	
	pPTYK = (struct VPTrueCtrl *)pPTDD;
	pPVYK = (struct VPVirtualCtrl *)pPVSYX;
	pPVYK += pWrtPVECfg->wYKNum;
	olddatalen -= (int)(pWrtPVECfg->wYKNum*sizeof(struct VPVirtualCtrl));      
	p=ParaFileTmpBuf+endpos;
	if (endpos >= MAXFILELEN) return(ERROR);         
	endpos += (int)(pPTECfg->wYKNum*sizeof(struct VPVirtualCtrl));   
	q = ParaFileTmpBuf+endpos;
	for (i=0; i<olddatalen; i++)
	{
		*q=*p;
		q--;
		p--;
	}     
	for (i=0; i<pPTECfg->wYKNum; i++)
	{
		memset(pPVYK, 0, sizeof(struct VPVirtualCtrl));
		if ((DWORD)teqpCfgNameLen+strlen(pPTYK->sName)+1 >=  GEN_NAME_LEN)
			sprintf(name,"YK%d", pWrtPVECfg->wYKNum+1);
		else	
			sprintf(name,"%s-%s", teqpCfgName, pPTYK->sName);
		strcpy(pPVYK->sName, name);
		if ((DWORD)teqpCfgNameLen+strlen(pPTYK->sNameTab)+1 >=  GEN_NAMETAB_LEN)
			sprintf(name,"YK%d", pWrtPVECfg->wYKNum+1);
		else	
			sprintf(name,"%s-%s", teqpCfgName, pPTYK->sNameTab);
		strcpy(pPVYK->sNameTab, name);		
		pPVYK->dwCfg = 0;
		pPVYK->dwTEName = dwTEName;
		pPVYK->wOffset = (WORD)i;
		pPVYK->wID = pWrtPVECfg->wYKNum+1;
		pPTYK++;
		pPVYK++;
		len += (int)sizeof(struct VPVirtualCtrl);     
		pWrtPVECfg->wYKNum++;
	}  

	pPTYT = (struct VPTrueCtrl *)pPTYK;
	pPVYT = (struct VPVirtualCtrl *)pPVYK;
	pPVYT += pWrtPVECfg->wYTNum;
	olddatalen -= (int)(pWrtPVECfg->wYTNum*sizeof(struct VPVirtualCtrl));      
	p=ParaFileTmpBuf+endpos;
	if (endpos >= MAXFILELEN) return(ERROR);         
	endpos += (int)(pPTECfg->wYTNum*sizeof(struct VPVirtualCtrl));   
	q = ParaFileTmpBuf+endpos;
	for (i=0; i<olddatalen; i++)
	{
		*q=*p;
		q--;
		p--;
	}     
	for (i=0; i<pPTECfg->wYTNum; i++)
	{
		memset(pPVYT, 0, sizeof(struct VPVirtualCtrl));
		if ((DWORD)teqpCfgNameLen+strlen(pPTYT->sName)+1 >=  GEN_NAME_LEN)
			sprintf(name,"YT%d", pWrtPVECfg->wYTNum+1);
		else	
			sprintf(name,"%s-%s", teqpCfgName, pPTYT->sName);
		strcpy(pPVYT->sName, name);
		if ((DWORD)teqpCfgNameLen+strlen(pPTYT->sNameTab)+1 >=  GEN_NAMETAB_LEN)
			sprintf(name,"YT%d", pWrtPVECfg->wYTNum+1);
		else	
			sprintf(name,"%s-%s", teqpCfgName, pPTYT->sNameTab);
		strcpy(pPVYT->sNameTab, name);		
		pPVYT->dwCfg = 0;
		pPVYT->dwTEName = dwTEName;
		pPVYT->wOffset = (WORD)i;
		pPVYT->wID = pWrtPVECfg->wYTNum+1;
		pPTYT++;
		pPVYT++;
		len += (int)sizeof(struct VPVirtualCtrl);     
		pWrtPVECfg->wYTNum++;
	}  

	if (len==0)
		return(ERROR);     
	else
	{
		g_pParafileTmp->nLength+=len;     
		if (WriteParaFile(pathname,g_pParafileTmp)==OK)
			return(AddOneEqp(path,pPAddEqp));
		else
			return(ERROR);
	}  
}


/*------------------------------------------------------------------------
 Procedure:     AddOneEqpGroup ID:1
 Purpose:       增加一个装置组以及其内的某一装置
 Input:         dwEqpName==0 表示只增加一个装置组
                如果已知装置组已存在则pPAddEqp可只填入sCfgName,且wEqpGSourceAddr
                和wEqpGDesAddr可为NULL
 Output:		装置个数 Error
 Errors:
------------------------------------------------------------------------*/
int AddOneEqpGroup(const char *path, DWORD dwEqpName, WORD wEqpGSourceAddr, WORD wEqpGDesAddr, const struct VPEqp *pPAddEqp)
{
	int i;
	struct VPEGCfg *pWrtPEGCfg;
	char pathname[MAXFILENAME];
	int len = 0;
	DWORD *pname;   
	BOOL write;
    BYTE* pname_temp;
	
	if (pPAddEqp->wCommID > THREAD_MAX_NUM) return(ERROR); 

	memset(ParaFileTmpBuf, 0, sizeof(ParaFileTmpBuf));

	pWrtPEGCfg=(struct VPEGCfg *)(g_pParafileTmp+1);

	GetParaFileName(path, pPAddEqp->sCfgName, ".cdg", pathname); 
	if (ReadParaFile(pathname,(BYTE *)g_pParafileTmp,MAXFILELEN)==ERROR)
	{
		pWrtPEGCfg->wSourceAddr = wEqpGSourceAddr;
		pWrtPEGCfg->wDesAddr = wEqpGDesAddr;
		pWrtPEGCfg->wEqpNum = 0;
		pWrtPEGCfg->dwFlag = 0;

		g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPEGCfg);
		g_pParafileTmp->wVersion = CFG_FILE_VER;
		g_pParafileTmp->wAttr = (WORD)(pPAddEqp->dwName>>12);
	}   

	if (dwEqpName==0)
	{
		if (pWrtPEGCfg->wEqpNum)  //modify
			write=TRUE;	
		else
			write=FALSE;	      //null eqpgroup
	}  
	else
	{
		pname=(DWORD *)(pWrtPEGCfg+1);
		for (i=0;i<pWrtPEGCfg->wEqpNum;i++)
		{
			if (*pname==dwEqpName)
				break;
			pname++;
		}  

		if (i == pWrtPEGCfg->wEqpNum)
		{
			pname_temp = (BYTE*)pname;
			memcpy(pname_temp,(BYTE*)&dwEqpName,sizeof(DWORD));
			pWrtPEGCfg->wEqpNum++;
			len=sizeof(DWORD);
			write=TRUE;	
		}  
		else     	
			write=FALSE;	         //exist
	}
   
	if (write)  
	{
		g_pParafileTmp->nLength += len;
		if (WriteParaFile(pathname, g_pParafileTmp) == OK)
			return(AddOneEqp(path, pPAddEqp));
		else
			return(ERROR);    
	}  
	else
		return(ERROR); 
}


void FillPQConfig(struct VPYcCfg *pDstCfg, const VYcCfg *pSrcCfg, int iindex, int pqnum)
{
    int k;
    for (k=0; k<6; k++)
    {
        if (pSrcCfg[k].type > 0)
        {
           memcpy(pDstCfg, pSrcCfg+k, sizeof(VYcCfg));
		   pDstCfg++;
        }
    }
	pDstCfg->type = YC_TYPE_P;
	pDstCfg->arg1 = iindex;
	pDstCfg->arg2 = pqnum;
	pDstCfg->toZero = 0;
	pDstCfg++;
	iindex += pqnum;
	pDstCfg->type = YC_TYPE_Q;
	pDstCfg->arg1 = iindex;
	pDstCfg->arg2 = pqnum;
	pDstCfg->toZero = 0;
	pDstCfg++;
	iindex += pqnum;
	pDstCfg->type = YC_TYPE_S;
	pDstCfg->arg1 = iindex;
	pDstCfg->arg2 = iindex+1;
	pDstCfg->toZero = 0;
	pDstCfg++;
	pDstCfg->type = YC_TYPE_Cos;
	pDstCfg->arg1 = iindex;
	pDstCfg->arg2 = iindex+2;
	pDstCfg->toZero = 0;
}

int FillYcConfig(struct VPYcCfg *pVYcCfg)
{
    int i,j,pq;
	short ua,ub,uc,ia,ib,ic,s,ainum;
	BYTE type,fdnum,pnum;
	short pindex,ThdIndex;
	VDefAiCfg *pDefAiCfg;
	VYcCfg pFdYcCfg[6];
	struct VPYcCfg *pTempCfg;
	struct VPYcCfg *pPYcCfg=(struct VPYcCfg*)((BYTE*)g_pParafileTmp+MAXFILELEN-10240);
#if (DEV_SP == DEV_SP_TTU)
	short uphz,iphz;
#endif
	
    ua = ub = uc = -1;
	ia = ib = ic = -1;
	s = -1;
	fdnum = 0;
	pindex = 0;
	ainum = 0;
	pTempCfg = pVYcCfg;
	for (i=0; i<BSP_AC_BOARD; i++)
	{
	    type = Get_Ai_Type(i);
		if (type == 0xFF)
		    break;
		pDefAiCfg = GetDefAiCfg(type);
		if (pDefAiCfg == NULL)
			continue;

		GetDefPYcCfg(type, pDefAiCfg->ycnum, pPYcCfg);

        memset(pFdYcCfg, -1, sizeof(VYcCfg)*6);
		pnum =0;
        pq = 0;
		for (j=0; j<pDefAiCfg->tvyccfgnum; j++)
		{
		     if (pPYcCfg[j].type == YC_TYPE_Ua)
			 {
			     if ((fdnum > 0)&&(pnum > 0))
			     {
			        FillPQConfig(pTempCfg, pFdYcCfg, pindex, pnum);
					pindex += pnum*2 + 4;
					pTempCfg += pnum*2 + 4;
					pq = 1;
					memset(pFdYcCfg, -1, sizeof(VYcCfg)*6);	
					pnum = 0;
			     }
				 ua = pindex;
		     }
			 if (pPYcCfg[j].type == YC_TYPE_Ub)
			 	ub = pindex;
			 if (pPYcCfg[j].type == YC_TYPE_Uc)
			 	uc = pindex;
			 if (pPYcCfg[j].type == YC_TYPE_Ia)
			 {
			    if (pq == 0)
			    {
				    if ((fdnum > 0)&&(pnum > 0))
				    {
				         FillPQConfig(pTempCfg, pFdYcCfg, pindex, pnum);
					     pindex += pnum*2 + 4;
						 pTempCfg += pnum*2 + 4;
						 memset(pFdYcCfg, -1, sizeof(VYcCfg)*6);	
						 pnum =0;
				    }
			    }
				ia = pindex;
				fdnum++;
				pq = 0;
			 }
			 if ((pPYcCfg[j].type == YC_TYPE_Ia)&&(ua > -1))
			 {
			    pFdYcCfg[0].type = YC_TYPE_Pa;
				pFdYcCfg[0].arg1 = ua;
				pFdYcCfg[0].arg2 = pindex;
				pFdYcCfg[0].toZero = 0;
				pFdYcCfg[3].type = YC_TYPE_Qa;
				pFdYcCfg[3].arg1 = ua;
				pFdYcCfg[3].arg2 = pindex;
				pFdYcCfg[3].toZero = 0;
				pnum++;
			 }
			 if ((pPYcCfg[j].type == YC_TYPE_Ib)&&(ub > -1))
			 {
			    pFdYcCfg[1].type = YC_TYPE_Pb;
				pFdYcCfg[1].arg1 = ub;
				pFdYcCfg[1].arg2 = pindex;
				pFdYcCfg[1].toZero = 0;
				pFdYcCfg[4].type = YC_TYPE_Qb;
				pFdYcCfg[4].arg1 = ub;
				pFdYcCfg[4].arg2 = pindex;
				pFdYcCfg[4].toZero = 0;
				pnum++;
				ib = pindex; 
			 }
			 if ((pPYcCfg[j].type == YC_TYPE_Ic)&&(uc > -1))
			 {
			    pFdYcCfg[2].type = YC_TYPE_Pc;
				pFdYcCfg[2].arg1 = uc;
				pFdYcCfg[2].arg2 = pindex;
				pFdYcCfg[2].toZero = 0;
				pFdYcCfg[5].type = YC_TYPE_Qc;
				pFdYcCfg[5].arg1 = uc;
				pFdYcCfg[5].arg2 = pindex;
				pFdYcCfg[5].toZero = 0;
				pnum++;
				ic = pindex;
			 }
			 
			 memcpy(pTempCfg, pPYcCfg+j, sizeof(struct VPYcCfg));
			 pTempCfg->arg1 = pTempCfg->arg1 + ainum;

			 if (pTempCfg->type == YC_TYPE_SI0)
			 	pTempCfg->arg1 = ia;
			 if (pTempCfg->type == YC_TYPE_SIb)
		 	 {
		 		pTempCfg->arg1 = ia;
				pTempCfg->arg2 = ic;
		 	 }
			 if (pTempCfg->type == YC_TYPE_SIc)
		 	 {
		 		pTempCfg->arg1 = ia;
				pTempCfg->arg2 = ib;
		 	 }
			 if (pTempCfg->type == YC_TYPE_SU0)
			 	pTempCfg->arg1 = ua;
			 if (pTempCfg->type == YC_TYPE_Uab)
			 {
			    pTempCfg->arg1 = ua;
				pTempCfg->arg2 = ub;
			 }
			 if (pTempCfg->type == YC_TYPE_Ubc)
			 {
			    pTempCfg->arg1 = ub;
				pTempCfg->arg2 = uc;
			 }
			 if (pTempCfg->type == YC_TYPE_Uca)
			 {
			    pTempCfg->arg1 = uc;
				pTempCfg->arg2 = ua;
			 }
			 pTempCfg++;
			 pindex++;
		}
		ainum += (short)pDefAiCfg->ainum;
		if ((fdnum > 0)&&(pnum > 0))
		{
		    FillPQConfig(pTempCfg, pFdYcCfg, pindex, pnum);
			pindex += pnum*2 + 4;
			pTempCfg += pnum*2 + 4;	

			s = pindex-2;
		}
	}
	if (pindex > 0)
	{
		pTempCfg->type = YC_TYPE_SFreq;
		pTempCfg->arg1 = 0;
		pTempCfg->arg2 = 0;
	    pindex++;
	}
	
#if(DEV_SP == DEV_SP_TTU)
	pTempCfg++;
    pTempCfg->type = YC_TYPE_TEMETER;
	pTempCfg->arg1 = 0;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = ua;
	pindex++;
	pTempCfg++;
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = ub;
	pindex++;
	pTempCfg++;
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = uc;
	pindex++;
	pTempCfg++;
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = ia;
	pindex++;
	pTempCfg++;
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = ib;
	pindex++;
	pTempCfg++;
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = ic;
	pindex++;
	pTempCfg++;

    ThdIndex = 0;
	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = (BYTE)(YC_TYPE_Har2 + i -1);
		pTempCfg->type = type;
		pTempCfg->arg1 = ua;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
		
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = ua;
	pindex++;
	pTempCfg++;

    ThdIndex = 0;
	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = (BYTE)(YC_TYPE_Har2 + i - 1);
		pTempCfg->type = type;
		pTempCfg->arg1 = ub;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
		
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = ub;
	pindex++;
	pTempCfg++;

	ThdIndex = 0;
	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = (BYTE)(YC_TYPE_Har2 + i - 1);
		pTempCfg->type = type;
		pTempCfg->arg1 = uc;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
	
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = uc;
	pindex++;
	pTempCfg++;

    ThdIndex = 0;
	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = (BYTE)(YC_TYPE_Har2 + i - 1);
		pTempCfg->type = type;
		pTempCfg->arg1 = ia;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
		
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = ia;
	pindex++;
	pTempCfg++;

    ThdIndex = 0;
	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = (BYTE)(YC_TYPE_Har2 + i - 1);
		pTempCfg->type = type;
		pTempCfg->arg1 = ib;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
		
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = ib;
	pindex++;
	pTempCfg++;

    ThdIndex = 0;
	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = (BYTE)(YC_TYPE_Har2 + i - 1);
		pTempCfg->type = type;
		pTempCfg->arg1 = ic;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
		
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = ic;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_UPhZ;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 3;
	uphz = pindex;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_UPhF;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_UPh0;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_IPhZ;
	pTempCfg->arg1 = ia;
	pTempCfg->arg2 = 3;
	iphz = pindex;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_IPhF;
	pTempCfg->arg1 = ia;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_IPh0;
	pTempCfg->arg1 = ia;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_Uunb;
	pTempCfg->arg1 = uphz;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_Iunb;
	pTempCfg->arg1 = iphz;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Upp;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Upp;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Upp;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Lrate;
	pTempCfg->arg1 = s;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udot;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udot;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udot;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udut;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udut;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udut;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umqr;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umqr;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umqr;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umot;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umot;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umot;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umut;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umut;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umut;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Uavg;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Uavg;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Uavg;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

#endif
	return pindex;

}

int CreateSysConfig(const char *path)
{
    int  len;
	BYTE type;
	WORD i,j,num,fdnum;
	DWORD aiType, ioType;
	char pathname[MAXFILENAME];  
	VDefDiCfg *pDefDiCfg;	
	VDefDoCfg *pDefDoCfg;
	VDefAiCfg *pDefAiCfg;
	struct VPFdCfg *pPFdCfg;
	struct VPAiCfg *pPAiCfg;
	struct VPDiCfg *pPDiCfg;
	struct VPDoCfg *pPDoCfg;
	struct VPYcCfg *pPYcCfg;
	struct VPSysConfig *pPSysCfg;
	char name[2*GEN_NAME_LEN];
    memset(ParaFileTmpBuf, 0, sizeof(ParaFileTmpBuf));
	
	pPSysCfg=(struct VPSysConfig *)(g_pParafileTmp+1);

	pPSysCfg->dwName = BACK_TYPE_IPACS; 
	strcpy(pPSysCfg->sByName, "IPACS");
	
	pPSysCfg->dwCfg = 1<<3; //以区分新老装置
	pPSysCfg->wSYXNum = 0;
	pPSysCfg->wYKNum= 0;
	pPSysCfg->wFDNum = 0;
	pPSysCfg->wAINum= 0;
	pPSysCfg->wDDNum = 0;
	len = 0;

    aiType = ioType = 0;
	num = 0;
	for (i=0; i<BSP_AC_BOARD; i++)
	{
	    type = Get_Ai_Type(i);
	    
		aiType |= ((DWORD)type) << (i*8);
		if (type == 0xFF) 
			break;

		num++;
		pDefAiCfg = GetDefAiCfg(type);
		if (pDefAiCfg != NULL)
		{
			pPSysCfg->wFDNum += pDefAiCfg->fdnum;
			pPSysCfg->wAINum += pDefAiCfg->ainum;

#ifdef INCLUDE_DD		
			pPSysCfg->wDDNum += (WORD)(pDefAiCfg->tvddcfgnum*pDefAiCfg->fdnum);
#else
			pPSysCfg->wDDNum = 0;
#endif
			pPSysCfg->wSYXNum += (WORD)(pDefAiCfg->tvyxcfgnum_fd*pDefAiCfg->fdnum);
			pPSysCfg->wYKNum += (WORD)(pDefAiCfg->tvykcfgnum_fd*pDefAiCfg->fdnum); 

		}
		else
		{
			WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "未知模拟板卡型号");
			myprintf(SYS_ID, LOG_ATTR_ERROR, "Can not find AIO_type %x default cfg!", type);
		}		
	}
	pPSysCfg->dwAIType = aiType;

	pPFdCfg = (struct VPFdCfg *)(pPSysCfg+1);
	pPAiCfg = (struct VPAiCfg *)(pPFdCfg+pPSysCfg->wFDNum);

    fdnum = 0;
	for (i=0; i<num; i++)
	{
	    type = (aiType >> (i*8))&0xFF;
		if (type == 0xFF)
			break;

		pDefAiCfg = GetDefAiCfg(type);
		if (pDefAiCfg == NULL)
			continue;
	   	GetDefPFdCfg(type, pDefAiCfg->fdnum, pPFdCfg);
		for (j=0; j<pDefAiCfg->fdnum; j++)
		{
		    sprintf(name, pPFdCfg->kgname, fdnum+j+1);
			strncpy(pPFdCfg->kgname, name, GEN_NAME_LEN-1);
			pPFdCfg->kgname[GEN_NAME_LEN-1] = '\0';
			pPFdCfg->kgid += fdnum;
			pPFdCfg->kg_stateno = (char)((fdnum+j)*6);
			pPFdCfg->kg_ykno = (char)(fdnum+j+1);
	        pPFdCfg ++;
		}
		fdnum += pDefAiCfg->fdnum;
	    GetDefPAiCfg(type, pDefAiCfg->ainum, pPAiCfg);
	    pPAiCfg += pDefAiCfg->ainum;	

		
	}
	len += (int)(pPSysCfg->wFDNum*sizeof(struct VPFdCfg));
	len += (int)(pPSysCfg->wAINum*sizeof(struct VPAiCfg));

    pPDiCfg = (struct VPDiCfg*)pPAiCfg;

	for (i=0; i<BSP_IO_BOARD; i++)
	{
	    type = Get_Io_Type(i)&0xF;
		ioType |= (DWORD)type << (i*4); 

		if (type == 0xF)  continue;
		pDefDiCfg = GetDefDiCfg(type);
		if (pDefDiCfg != NULL)
		{

			pPSysCfg->wDYXNum = 0;	
			
			pPSysCfg->wSYXNum += (WORD)(pDefDiCfg->tvyxcfgnum_di);	

            GetDefPDiCfg(type, (WORD)pDefDiCfg->dinum, pPDiCfg);
			for (j=0; j< pDefDiCfg->dinum; j++)
				pPDiCfg[j].yxno += pPSysCfg->wDINum;

		    pPSysCfg->wDINum += (WORD)pDefDiCfg->dinum;
			pPDiCfg += pDefDiCfg->dinum;

		}
	}
	len += (int)(pPSysCfg->wDINum*sizeof(struct VPDiCfg));


	pPDoCfg = (struct VPDoCfg*)pPDiCfg;
	for (i=0; i<BSP_IO_BOARD; i++)
	{
	    type = Get_Io_Type(i);
		
		if (type == 0xFF)  continue;
		pDefDoCfg = GetDefDoCfg(type);
		if (pDefDoCfg != NULL)
		{
			pPSysCfg->wDONum += (WORD)pDefDoCfg->donum;
			pPSysCfg->wYTNum = 0;
			pPSysCfg->wYKNum += (WORD)pDefDoCfg->tvykcfgnum_do;	

			GetDefPDoCfg(type, (WORD)pDefDoCfg->donum, pPDoCfg);
			pPDoCfg += pDefDoCfg->donum;

		}		
	}
	len += (int)(pPSysCfg->wDONum*sizeof(struct VPDoCfg));


	pPYcCfg = (struct VPYcCfg*)pPDoCfg;
	pPSysCfg->wYCNum = (WORD)FillYcConfig(pPYcCfg);
	pPYcCfg += pPSysCfg->wYCNum;
	len += (int)(pPSysCfg->wYCNum*sizeof(struct VPYcCfg));


	
	pDefDoCfg = GetDefDoCfg(0);
	pPSysCfg->wYKNum += (WORD)pDefDoCfg->tvykcfgnum_public;

	type = Get_Power_Type(0);
	if (type != 0xFF)
	{
		pDefAiCfg = GetDefAiCfg(0);
		if (pDefAiCfg)
		{
			pPSysCfg->wYcNumPublic = pDefAiCfg->tvyccfgnum_public;
		  GetDefPYcCfg_Public(0, pPSysCfg->wYcNumPublic, pPYcCfg);
		  pPSysCfg->wYCNum += pPSysCfg->wYcNumPublic;
		  len += pPSysCfg->wYcNumPublic*sizeof(struct VPYcCfg);
		
		  pPSysCfg->wSYXNum += pDefAiCfg->tvyxcfgnum_public*pPSysCfg->wFDNum;
		  pDefDiCfg = GetDefDiCfg(0);
		  pPSysCfg->wSYXNum += pDefDiCfg->tvyxcfgnum_public;
		}
		
	}

	pPSysCfg->dwType = BACK_TYPE_IPACS|type;
	pPSysCfg->dwAIType = aiType;
	pPSysCfg->dwIOType = ioType;


	g_pParafileTmp->nLength = (int)sizeof(struct VFileHead)+(int)sizeof(struct VPSysConfig)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = CFG_ATTR_NULL;

	GetParaFileName(path,"system",".cfg",pathname); 
    return(WriteParaFile(pathname,g_pParafileTmp)); 
	
}

#ifdef INCLUDE_CELL
int CreateCellConfig(void)
{
	int len;
	char pathname[MAXFILENAME];  	
	struct VPCellCfg *pPCfg;
	struct VPCellCtrl *pPCtrl;

    memset(ParaFileTmpBuf, 0, sizeof(ParaFileTmpBuf));
	
	pPCfg = (struct VPCellCfg *)(g_pParafileTmp+1);
	pPCtrl = (struct VPCellCtrl *)(pPCfg+1);
		
	pPCfg->dwCfg = 0;
	pPCfg->dwNum = 1;
	pPCfg->wID = g_Sys.MyCfg.wDONum+1;
	pPCfg->Udis = 0;

	memset(pPCtrl,0,sizeof(struct VPCellCtrl));
	pPCtrl->dwDayBits = 90;
	pPCtrl->wHour = 0;
	pPCtrl->dwMode = pPCtrl->dwMode | 0x03 |CELL_MODE_ENDTIME;
	pPCtrl->dwTime = 1;
	
	len = pPCfg->dwNum*sizeof(struct VPCellCtrl);
	
	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPCellCfg)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = CFG_ATTR_NULL;

	GetParaFileName(NULL,"cell",".cfg",pathname); 
	return(WriteParaFile(pathname,g_pParafileTmp)); 
}
#endif

void CreateRunParaConfig(void)
{
	int i;
	char pathname[MAXFILENAME];	
	VRunParaFileCfg *pRunParaCfg;

	pRunParaCfg = (VRunParaFileCfg *)(g_pParafileTmp+1);
	memset(pRunParaCfg,0,sizeof(VRunParaFileCfg));
	
	if(g_Sys.MyCfg.wFDNum > 0)
	{
		pRunParaCfg->wPt1= g_Sys.MyCfg.pFd[0].pt;
		pRunParaCfg->wPt2= g_Sys.MyCfg.pFd[0].Un;
	}
	else
	{
		pRunParaCfg->wPt1 = 10000;
		pRunParaCfg->wPt2 = 220;
	}
	pRunParaCfg->dwDyVal= 0*1000;
	pRunParaCfg->wDyT = 600; //s
	pRunParaCfg->dwGyVal= 220*2*1000;
	#if (DEV_SP == DEV_SP_TTU)
	pRunParaCfg->wGyT = 0; //s
	#else
	pRunParaCfg->wGyT = 600; //s
	#endif
	pRunParaCfg->dwZzVal= 5*2*0.7*1000;
	pRunParaCfg->wZzT = 3600; //s
	pRunParaCfg->dwGzVal= 5*2*1000;
	pRunParaCfg->wGzT = 3600; //s
	if(g_Sys.MyCfg.wDINum > 0)
	{
		pRunParaCfg->wYxfd = g_Sys.MyCfg.pDi[0].dtime;
	}
	else
	{
		pRunParaCfg->wYxfd = 200;
	}
	if(g_Sys.MyCfg.wDONum > 0)
	{
		pRunParaCfg->wFzKeepT = g_Sys.MyCfg.pDo[0].fzontime;
		pRunParaCfg->wHzKeepT = g_Sys.MyCfg.pDo[0].hzontime;
	}
	else
	{
		pRunParaCfg->wFzKeepT = 500;
		pRunParaCfg->wHzKeepT = 500;
	}
	#ifdef INCLUDE_CELL
	if((g_Sys.MyCfg.CellCfg.dwNum > 0) && (g_Sys.MyCfg.CellCfg.pCtrl != NULL))
	{
		if(g_Sys.MyCfg.CellCfg.pCtrl->dwMode == 3)
		{
			pRunParaCfg->wDchhT = g_Sys.MyCfg.CellCfg.pCtrl->dwDayBits;
			pRunParaCfg->wDchhTime = g_Sys.MyCfg.CellCfg.pCtrl->wHour;
		}
		else
		{
			pRunParaCfg->wDchhT = 90;
			pRunParaCfg->wDchhTime = 0;
		}
	}
	else
	#endif
	{
		pRunParaCfg->wDchhT = 90;
		pRunParaCfg->wDchhTime = 0;
	}
	
	for(i=0;i < MAX_FD_NUM;i++)
	{
		if(i  < g_Sys.MyCfg.wFDNum)
		{
			pRunParaCfg->LineCfg[i].wCt1 = g_Sys.MyCfg.pFd[i].ct;
			pRunParaCfg->LineCfg[i].wCt2 = g_Sys.MyCfg.pFd[i].In;
			pRunParaCfg->LineCfg[i].wCt01 = g_Sys.MyCfg.pFd[i].ct0;
			pRunParaCfg->LineCfg[i].wCt02 = g_Sys.MyCfg.pFd[i].In0;
		}
		else
		{
			pRunParaCfg->LineCfg[i].wCt01 = 20;
			pRunParaCfg->LineCfg[i].wCt1 = 600;
			pRunParaCfg->LineCfg[i].wCt2 = 5;
			pRunParaCfg->LineCfg[i].wCt02 = 5;
		}
	}

	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(VRunParaFileCfg);
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = CFG_ATTR_NULL;

	GetParaFileName(NULL,"runpara",".cfg",pathname);   
	WriteParaFile(pathname,g_pParafileTmp);  
}

int CreateSysAddr(const char *path, struct VPAddr *pPAddr)
{
	char pathname[MAXFILENAME];   
	struct VPAddr *pWrtPAddr;

	memset((BYTE*)pPAddr, 0, sizeof(struct VPAddr));
	//read addr from mmi
    //pPAddr->wDefFlag = 0x55;

	pPAddr->wAddr = pPAddr->wAsMasterAddr = 1;  

	strcpy(pPAddr->Lan1.sIP, "192.168.5.1");	
	pPAddr->Lan1.dwMask = 0xFFFFFF00;

	strcpy(pPAddr->Lan2.sIP, "172.168.5.1");
	pPAddr->Lan2.dwMask = 0xFFFFFF00;

	pWrtPAddr=(struct VPAddr *)(g_pParafileTmp+1);
	memcpy(pWrtPAddr,pPAddr,sizeof(struct VPAddr));
    memset(g_pParafileTmp, 0, sizeof(struct VFileHead));
	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPAddr);
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = CFG_ATTR_NULL;
	GetParaFileName(path,"addr",".cfg",pathname);   
	return(WriteParaFile(pathname,g_pParafileTmp));  
}

int SaveSysAddr(const char *path, const struct VPAddr *pPAddr)
{
	int ret;
	char pathname[MAXFILENAME];	
	struct VPAddr *pWrtPAddr;

	smMTake(g_paraFTBSem);
	pWrtPAddr=(struct VPAddr *)(g_pParafileTmp+1);
	memcpy(pWrtPAddr,pPAddr,sizeof(struct VPAddr));

	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPAddr);
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = CFG_ATTR_NULL;

	GetParaFileName(path,"addr",".cfg",pathname);   
	ret = WriteParaFile(pathname,g_pParafileTmp);
	smMGive(g_paraFTBSem);
	
	return ret;  
}

int AddOnePort(const char *path, const struct VPPort *pPAddPort)
{
	int i, id, wPortNum;
	struct VPPort *pPPort;
	char pathname[MAXFILENAME]; 

	if ((pPAddPort->id < COMM_START_NO) || (pPAddPort->id > COMM_START_NO+COMM_NUM)
		|| ((pPAddPort->id >= COMM_NET_START_NO) && (pPAddPort->id < COMM_NET_USR_NO)))
		return(ERROR);

   	id = GetParaPortId(pPAddPort->id);
	
   	GetParaFileName(path,"port",".cfg",pathname); 
    if (ReadParaFile(pathname,(BYTE *)g_pParafileTmp,MAXFILELEN)==ERROR)
        g_pParafileTmp->nLength=sizeof(struct VFileHead);
   
	wPortNum=(g_pParafileTmp->nLength-(int)sizeof(struct VFileHead))/(int)sizeof(struct VPPort);      
    pPPort=(struct VPPort *)(g_pParafileTmp+1);
      
	for (i=0; i<wPortNum; i++)
	{
		//if there have this portinfo already then only modify else add
		if (pPPort->id == id)  
			break;
		pPPort++;	
	}  	

    if (i == wPortNum)  //add no modify
    {
		wPortNum++;
		g_pParafileTmp->nLength+=(int)sizeof(struct VPPort);
    }  
    
    memcpy(pPPort, pPAddPort, sizeof(struct VPPort));
	pPPort->id = id;
   
    if (WriteParaFile(pathname,g_pParafileTmp)==OK)
        return(wPortNum);
    else
        return(ERROR);        
}   

/*------------------------------------------------------------------------
 Procedure:     CreateMinEqpSystme ID:1
 Purpose:       创建最小装置系统
 Input:
 Output:		装置个数 Error
 Errors:
------------------------------------------------------------------------*/
int CreateMinEqpSystme(const char *path)
{
	struct VPSysConfig *pPSysCfg;
	char pathname[MAXFILENAME];
	struct VPTECfg PTECfg;
	struct VPEqp PEqp;  
	WORD  addr;

	struct VPAddr *pAddr;
	struct VPort *pPort;
	struct VPPort PAddPort;

	GetParaFileName(path, "addr", ".cfg", pathname); 
	if (ReadParaFile(pathname, (BYTE *)g_pParafileTmp, MAXFILELEN) == ERROR)  return(0);
	pAddr = (struct VPAddr *)(g_pParafileTmp+1);
	addr = pAddr->wAddr;
	GetParaFileName(path, "system", ".cfg", pathname);   
	if (ReadParaFile(pathname, (BYTE *)g_pParafileTmp, MAXFILELEN) == ERROR)  return(0);
	pPSysCfg = (struct VPSysConfig *)(g_pParafileTmp+1);

    memset(&PTECfg, 0, sizeof(PTECfg));
	PTECfg.wSourceAddr = addr;
	PTECfg.wDesAddr = addr;		
	PTECfg.wYCNum = pPSysCfg->wYCNum;
	PTECfg.wVYCNum = 0;
	PTECfg.wDYXNum = pPSysCfg->wDYXNum;
	PTECfg.wSYXNum = pPSysCfg->wSYXNum;
	PTECfg.wVYXNum = 0;
	PTECfg.wDDNum = pPSysCfg->wDDNum;
	PTECfg.wYKNum = pPSysCfg->wYKNum;
	PTECfg.wYTNum = pPSysCfg->wYTNum;

	PTECfg.byYCFrzMD = 15;
	PTECfg.byYCFrzCP = 30;
	PTECfg.byDDFrzMD = 60;
	PTECfg.byDDFrzCP = 30;

	PEqp.dwName = CFG_ATTR_IO<<12;
	strcpy(PEqp.sCfgName,"myio");
	PEqp.wCommID = SELF_DIO_ID;
	if (AddMyIoTrueEqp(path,  pPSysCfg, &PTECfg, &PEqp) == ERROR)
		return(0);
	PEqp.dwName = (CFG_ATTR_TABLE<<12)+1;
	strcpy(PEqp.sCfgName,"veqp01");
	PEqp.wCommID = COMM_NET_USR_NO;
	if (AddOneVirtualEqp(path,  (CFG_ATTR_IO<<12), addr, &PEqp) == ERROR)
		return(0);

#ifdef INCLUDE_FA
    if (AddFaEqp(path, fdnum) == ERROR)
		return(0);
#endif
#ifdef INCLUDE_FA_DIFF
    if (AddFaDiffEqp(path, fdnum) == ERROR)
		return(0);
#endif
#ifdef INCLUDE_PR_DIFF
	 if(AddFadiffTrueEqp(NULL, 0) == ERROR)
		 return 0;
	memset(&PAddPort, 0, sizeof(PAddPort));
	PAddPort.id = DIFFPRO0_ID;
	strcpy(PAddPort.pcol, "EXTDIFF");
	PAddPort.pcol_attr = 0;
	strcpy(PAddPort.cfgstr[0], "4:230400,8,1,N");
	PAddPort.bakmode = 0;
	PAddPort.bakcomm = 0;		
	AddOnePort(NULL, &PAddPort);

	pPort = &g_Task[DIFFPRO0_ID].CommCfg.Port;
	if (pPort->id == 0)  reboot(0);
	
	 if(AddFadiffTrueEqp(NULL, 1) == ERROR)
		 return 0;
	memset(&PAddPort, 0, sizeof(PAddPort));
	PAddPort.id = DIFFPRO1_ID;
	strcpy(PAddPort.pcol, "EXTDIFF");
	PAddPort.pcol_attr = 0;
	strcpy(PAddPort.cfgstr[0], "4:230400,8,1,N");
	PAddPort.bakmode = 0;
	PAddPort.bakcomm = 0;		
	AddOnePort(NULL, &PAddPort);

	pPort = &g_Task[DIFFPRO1_ID].CommCfg.Port;
	if (pPort->id == 0)  reboot(0);
#endif

#ifdef INCLUDE_SOC //两个线差，1个FA
	if (AddFaSocTrueEqp(NULL, 0) == ERROR)
			return(0);
	 if(AddLinePrTrueEqp(NULL, 0) == ERROR)
		 return 0;
	 if(AddLinePrTrueEqp(NULL, 1) == ERROR)
		 return 0;
#endif

	PEqp.dwName = (CFG_ATTR_GROUP<<12)+2;
	strcpy(PEqp.sCfgName, "eqpg01");
	PEqp.wCommID = COMM_NET_USR_NO;	
	AddOneEqpGroup(path, (CFG_ATTR_TABLE<<12)+1, addr, 254, &PEqp);
	//AddOneEqpGroup(path, CFG_ATTR_IO<<12, addr, 254, &PEqp);
			
	//COMM_NET_USR_NO
	memset(&PAddPort, 0, sizeof(PAddPort));
	PAddPort.id = COMM_NET_USR_NO;
	strcpy(PAddPort.pcol, "GB104");
	PAddPort.pcol_attr = 1;
	strcpy(PAddPort.cfgstr[0], "1:0.0.0.0:2404");
	PAddPort.bakmode = 0;
	PAddPort.bakcomm = 0;		
	AddOnePort(NULL, &PAddPort);
	pPort = &g_Task[COMM_NET_USR_NO].CommCfg.Port;
	if (pPort->id == 0)  reboot(0);

    return OK;
}

int VerifyParaFile(void)
{
	FILE *fp;
	struct VPIndInfo *pInd;
	int i, result; 
	char path[4*MAXFILENAME];

	pInd = pFstIndInfo;
	for (i=0; i<pIndHead->indNum; i++)
	{	
		GetMyPath(path, pIndHead->pkgInfo[pInd->pkgIndex].pkgName);
		if ((fp=fopen(path, "rb")) == NULL) break;

		result = fseek(fp, pInd->cfgOffset+pInd->cfgLen, SEEK_SET);

		fclose(fp);  
		
		if (result) break;
		
		pInd++;
	}

	if (i == pIndHead->indNum)
		return OK;
	else
		return ERROR;
}

int SysParaInit(void)
{
	int len;
	FILE *fp;
	volatile WORD crc1, crc2, mycrc;
	char path[4*MAXFILENAME];

	mycrc = crc1 = crc2 = 0;
	FileManagerInit();
	g_pParafileTmp = (struct VFileHead *)ParaFileTmpBuf;

	g_paraFTBSem = smMCreate();
	paraRWSemID = smMCreate();
	memset(indFileBuf, 0, sizeof(indFileBuf));

    pIndHead = (struct VPIndHead *)indFileBuf;
	pFstIndInfo = (struct VPIndInfo *)(pIndHead+1);

	GetMyPath(path, CFG_IND_FILENAME);
	if ((fp=fopen(path, "rb")) == NULL)  goto Err;
    len = fread(indFileBuf, sizeof(BYTE), sizeof(indFileBuf), fp);
	fclose(fp);
    if (len < (int)sizeof(struct VPIndHead)+(int)sizeof(WORD)) goto Err;
 	if (pIndHead->nLength < 0) goto Err;       //防止FLASH写失败导致nLength读出为0xFFFFFFFF
    mycrc = GetParaCrc16(indFileBuf, pIndHead->nLength);
    crc1 = (indFileBuf[len-1]<<8)+indFileBuf[len-2];
    crc2 = (indFileBuf[len-2]<<8)+indFileBuf[len-1];

	if ((mycrc != crc1) && (mycrc != crc2)) goto Err;

	if (VerifyParaFile() == ERROR) goto Err;
	return OK;

Err:
	memset(indFileBuf, 0, sizeof(indFileBuf));
	pIndHead->nLength = sizeof(struct VPIndHead);
	pIndHead->wVersion = CFG_FILE_VER;
	pIndHead->indNum = 0;
	
	if ((fp=fopen(path, "wb")) == NULL) return ERROR;
	fwrite(indFileBuf, sizeof(BYTE), sizeof(struct VPIndHead), fp);
	mycrc = GetParaCrc16(indFileBuf, pIndHead->nLength);
	fwrite((BYTE*)&mycrc, sizeof(BYTE), sizeof(mycrc), fp);
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);

	return ERROR;
}

int SetSysAddr(const WORD *addr, const char *ip1, const char *ip2, const char *gateway1, const char *gateway2)
{
	WORD old_addr=0;
	int i;
	char old_ip1[20]={0}, old_ip2[20]={0}, old_gateway1[20]={0}, old_gateway2[20]={0};

	if (addr != NULL)
	{
        old_addr = g_Sys.AddrInfo.wAddr;
		g_Sys.AddrInfo.wAddr = *addr;
	}

	if (ip1 != NULL)
	{
		strcpy(old_ip1, g_Sys.AddrInfo.Lan1.sIP);
		strcpy(g_Sys.AddrInfo.Lan1.sIP, ip1);
	}	
	
	if (ip2 != NULL)
	{
		strcpy(old_ip2, g_Sys.AddrInfo.Lan2.sIP);
		strcpy(g_Sys.AddrInfo.Lan2.sIP, ip2);
	}	

	if (gateway1 != NULL)
	{
		strcpy(old_gateway1, g_Sys.AddrInfo.sGateWay1);
		strcpy(g_Sys.AddrInfo.sGateWay1, gateway1);
	}	

	if (gateway2 != NULL)
	{
		strcpy(old_gateway2, g_Sys.AddrInfo.sGateWay2);
		strcpy(g_Sys.AddrInfo.sGateWay2, gateway2);
	}	

	if (SaveSysAddr(NULL, &g_Sys.AddrInfo) == OK)
	{
        if (addr != NULL)
        {
			for (i=0; i<g_Sys.MyCfg.wFDNum; i++)
				g_Sys.MyCfg.pFd[i].kgid = *addr*100+i+1;
        }	

		AddrInit();

		return OK;
	}	
	else
	{		
		if (addr != NULL)
			g_Sys.AddrInfo.wAddr = old_addr;
		
		if (ip1 != NULL)
			strcpy(g_Sys.AddrInfo.Lan1.sIP, old_ip1);
		
		if (ip2 != NULL)
			strcpy(g_Sys.AddrInfo.Lan2.sIP, old_ip2);
		
		if (gateway1 != NULL)
			strcpy(g_Sys.AddrInfo.sGateWay1, old_gateway1);

		if (gateway2 != NULL)
			strcpy(g_Sys.AddrInfo.sGateWay2, old_gateway2);

		return ERROR;
	}	
}

int ReadPtCt(int fd, int *pt, int *Un, int *ct, int *In, int *ct0, int *In0)
{
	struct VPSysConfig *pPSysCfg;
	struct VPFdCfg *pPFdCfg;

	smMTake(g_ParafileSem);
	
	if (ReadParaFile("system.cfg", (BYTE *)g_pParafile, MAXFILELEN) != OK)
	{
		smMGive(g_ParafileSem);
		return ERROR;
	}
			
	pPSysCfg=(struct VPSysConfig *)(g_pParafile+1);
	pPFdCfg = (struct VPFdCfg *)(pPSysCfg+1);

	if (pt != NULL) *pt = pPFdCfg[fd].pt;
	if (Un != NULL) *Un = pPFdCfg[fd].Un;
	if (ct != NULL) *ct = pPFdCfg[fd].ct;
	if (In != NULL) *In = pPFdCfg[fd].In;
	if (ct0 != NULL) *ct0 = pPFdCfg[fd].ct0;
	if (In0 != NULL) *In0 = pPFdCfg[fd].In0;
	
	smMGive(g_ParafileSem);
	return OK;
}

int SavePtCt(int fd, const int *pt, const int *Un, const int *ct, const int *In, const int *ct0, const int *In0)
{
	int ret;
	
	struct VPSysConfig *pPSysCfg;
	struct VPFdCfg *pPFdCfg;

	smMTake(g_ParafileSem);
	
	if (ReadParaFile("system.cfg", (BYTE *)g_pParafile, MAXFILELEN) != OK)
	{
		smMGive(g_ParafileSem);
		return ERROR;
	}
			
	pPSysCfg=(struct VPSysConfig *)(g_pParafile+1);
	pPFdCfg = (struct VPFdCfg *)(pPSysCfg+1);

	if (pt != NULL) pPFdCfg[fd].pt = *pt;
	if (Un != NULL) pPFdCfg[fd].Un = (WORD)*Un;
	if (ct != NULL) pPFdCfg[fd].ct = *ct;
	if (In != NULL) pPFdCfg[fd].In = (WORD)*In;
	if (ct0 != NULL) pPFdCfg[fd].ct0 = *ct0;
	if (In0 != NULL) pPFdCfg[fd].In0 = (WORD)*In0;

	ret = WriteParaFile("system.cfg", g_pParafile);
	
	smMGive(g_ParafileSem);

	return ret;
}

#if 0
/*------------------------------------------------------------------------
 Procedure:     ParaFileRemove ID:1
 Purpose:       参数文件删除(如果是下级参数文件，则若目录下无文件，一并
                删除目录
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void ParaFileRemove(char *path,char *filename,char *suffix)
{
	char pathname[MAXFILENAME]; 

	GetParaFileName(path,filename,suffix,pathname); 
	DelFile(pathname);

	if ((path==NULL)||(strcmp(path,"")==0)) return;  //下级参数文件

	GetParaFileName(path,"","",pathname);    
	DelDir(pathname);   //目录下有文件自动不删除
}

/*------------------------------------------------------------------------
 Procedure:     DelOneEqp ID:1
 Purpose:       删除一个装置
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/
void DelOneEqp(char *path, DWORD dwEqpName)
{
	int i,wEqpNum,len;
	VPEqp *pPEqp;
	char pathname[MAXFILENAME]; 

	GetParaFileName(path,"device",".cfg",pathname); 
	if (ReadParaFile(pathname,(BYTE *)g_pParafileTmp,MAXFILELEN)==ERROR)
		return;

	len=g_pParafileTmp->nLength-sizeof(VFileHead);
	wEqpNum=len/sizeof(VPEqp);

	pPEqp=(VPEqp *)(g_pParafileTmp+1);
	for (i=0;i<wEqpNum;i++)
	{
		if  (pPEqp->dwName == dwEqpName)
		{
			memcpy(pPEqp, +1,len-(i+1)*sizeof(VPEqp));
			break;     	
		}  
		else
			pPEqp++;
	}  

	if (i<wEqpNum)  //find
	{
		wEqpNum--;
		len = wEqpNum*sizeof(VPEqp);
		if (len>0)
		{
			g_pParafileTmp->nLength = sizeof(VFileHead)+len;     
			WriteParaFile(pathname, g_pParafileTmp);      
		}
		else
			ParaFileRemove(path,"device",".cfg");    
	}  
}

void DelOneTEqpFromVEqp(DWORD dwTEName,char *path,char *filename)
{
	int i,k,len;
	VPVECfg *pWrtPVECfg;
	char VEPathName[MAXFILENAME]; 

	GetParaFileName(path,filename,".cdt",VEPathName); 
	if (ReadParaFile(VEPathName,(BYTE *)g_pParafileTmp,MAXFILELEN)==ERROR)
		return;   

	pWrtPVECfg = (VPVECfg *)(g_pParafileTmp+1);
	len = g_pParafileTmp->nLength-sizeof(VFileHead)-sizeof(VPVECfg);
	if (len <= 0) return;
   	
	VPVirtualYC *pPVYC=(VPVirtualYC *)(pWrtPVECfg+1);
	k=0;
	for (i=0; i<pWrtPVECfg->wYCNum; i++)
	{
		len -= sizeof(VPVirtualYC);
		if (pPVYC->dwTEName == dwTEName)
		{
			memcpy(pPVYC,pPVYC+1,len);
			k++;
		}  
		else	
		{ 
			pPVYC->wSendNo = i-k;   //insure the sendno beacase some be deled and other moved
			pPVYC++;
		} 
	}  
	pWrtPVECfg->wYCNum-=k;
   
	VPVirtualYX *pPVDYX=(VPVirtualYX *)pPVYC;
	k=0;
	for (i=0;i<pWrtPVECfg->wDYXNum;i++)
	{
		len -= sizeof(VPVirtualYX);
		if (pPVDYX->dwTEName == dwTEName)
		{
			memcpy(pPVDYX,pPVDYX+1,len);
			k++;
		}  
		else	
		{ 
			pPVDYX->wSendNo = i-k;   //insure the sendno beacase some be deled and other moved
			pPVDYX++;
		} 
	}  
	pWrtPVECfg->wDYXNum-=k;

	VPVirtualYX *pPVSYX=(VPVirtualYX *)pPVDYX;
	k=0;
	for (i=0;i<pWrtPVECfg->wSYXNum;i++)
	{
		len -= sizeof(VPVirtualYX);
		if (pPVSYX->dwTEName == dwTEName)
		{
			memcpy(pPVSYX,pPVSYX+1,len);
			k++;
		}  
		else	
		{ 
			pPVSYX->wSendNo=i-k;   //insure the sendno beacase some be deled and other moved
			pPVSYX++;
		} 
	}  
	pWrtPVECfg->wSYXNum-=k;

	VPVirtualDD *pPVDD=(VPVirtualDD *)pPVSYX;
	k=0;
	for (i=0;i<pWrtPVECfg->wDDNum;i++)
	{
		len-=sizeof(VPVirtualDD);
		if (pPVDD->dwTEName==dwTEName)
		{
			memcpy(pPVDD,pPVDD+1,len);
			k++;
		}  
		else	
		{ 
			pPVDD->wSendNo=i-k;   //insure the sendno beacase some be deled and other moved
			pPVDD++;
		} 
	}  
	pWrtPVECfg->wDDNum-=k;

	VPVirtualCtrl *pPVYK=(VPVirtualCtrl *)pPVDD;
	k=0;
	for (i=0;i<pWrtPVECfg->wYKNum;i++)
	{
		len-=sizeof(VPVirtualCtrl);
		if (pPVYK->dwTEName==dwTEName)
		{
			memcpy(pPVYK,pPVYK+1,len);
			k++;
		}  
		else	
			pPVYK++;
	}  
	pWrtPVECfg->wYKNum-=k;
   
    len=pWrtPVECfg->wYCNum*sizeof(VPVirtualYC)+(pWrtPVECfg->wDYXNum+pWrtPVECfg->wSYXNum+ \
    	   pWrtPVECfg->wDDNum*sizeof(VPVirtualDD)+pWrtPVECfg->wYKNum*sizeof(VPVirtualCtrl);
   	      
	if (len>0)
	{
		g_pParafileTmp->nLength=sizeof(VFileHead)+sizeof(VPVECfg)+len;     
		WriteParaFile(VEPathName, g_pParafileTmp);
	}
	else
		ParaFileRemove(path,filename,".cdt");  
}

void DelOneEqpFromEqpG(DWORD dwEqpName, char *path, char *filename)
{
	int i,len;
	VPEGCfg *pWrtPEGCfg;
	DWORD *pname;
	char EGPathName[MAXFILENAME];

	GetParaFileName(path, filename, ".cdg", EGPathName); 
	if (ReadParaFile(EGPathName,(BYTE *)g_pParafileTmp,MAXFILELEN)==ERROR)
		return;

	len = g_pParafileTmp->nLength-sizeof(VFileHead)-sizeof(VPEGCfg);
	if (len <= 0) return; 

	pWrtPEGCfg=(VPEGCfg *)(g_pParafileTmp+1);
	pname=(DWORD *)(pWrtPEGCfg+1);
	for (i=0;i<pWrtPEGCfg->wEqpNum;i++)
	{
		if (*pname==dwEqpName)
		{
			memcpy(pname,pname+1,len-(i+1)*sizeof(DWORD));
			break;
		}  
		else	
			pname++;
	}  

	if (i<pWrtPEGCfg->wEqpNum)  //find
	{
		pWrtPEGCfg->wEqpNum--;
		len = pWrtPEGCfg->wEqpNum*sizeof(DWORD);
		if (len>0)
		{
			g_pParafileTmp->nLength = sizeof(VFileHead)+sizeof(VPEGCfg)+len;     
			WriteParaFile(EGPathName, g_pParafileTmp);      
		}
		else
			ParaFileRemove(path, filename, ".cdg");           
	}  
}

void DelOneTrueEqp(char *path, DWORD dwTEName)
{
	char pathname[MAXFILENAME],filename[MAXFILENAME];
	int wEqpNum,i;
	WORD attr;
	VPEqp *pPEqp;

	GetParaFileName(path, "device", ".cfg",pathname); 
	/*beacuse use g_pParafileTmp with VEqp and EqpG,so there use g_pParafile*/
	if (ReadParaFile(pathname,(BYTE *)g_pParafile,MAXFILELEN)==ERROR)
		return;   

	wEqpNum=(g_pParafile->nLength-sizeof(VFileHead))/sizeof(VPEqp);
	pPEqp=(VPEqp *)(g_pParafile+1);

	for (i=0;i<wEqpNum;i++)
	{     
		if (pPEqp->dwName == dwTEName)
			strcpy(filename,pPEqp->sCfgName);
		else
		{
			attr = pPEqp->dwName>>16;              
			//virtual eqp da is virtual eqp all the same
			if (attr == CFG_ATTR_TABLE)
				DelOneTEqpFromVEqp(dwTEName, path, pPEqp->sCfgName);
			else if (attr == CFG_ATTR_GROUP)   //group eqp
				DelOneEqpFromEqpG(dwTEName, path, pPEqp->sCfgName);
		}  
		
		pPEqp++;
	}  

	DelOneEqp(path, dwTEName);

	ParaFileRemove(path,filename,".cde");
}

void DelOneVirtualEqp(char *path, DWORD dwVEName)
{
	char pathname[MAXFILENAME],filename[MAXFILENAME];
	int wEqpNum,i;
	VPEqp *pPEqp;

	GetParaFileName(path, "device", ".cfg",pathname); 
	/*beacuse use g_pParafileTmp with EqpG,so there use g_pParafile*/
	if (ReadParaFile(pathname, (BYTE *)g_pParafile, MAXFILELEN) == ERROR)
		return;   

	wEqpNum=(g_pParafile->nLength-sizeof(VFileHead))/sizeof(VPEqp);
	pPEqp=(VPEqp *)(g_pParafile+1);

	for (i=0;i<wEqpNum;i++)
	{     
		if (pPEqp->dwName == dwVEName)
			strcpy(filename, pPEqp->sCfgName);
		else
		{
			if ((pPEqp->dwName>>16) == CFG_ATTR_GROUP)   //group eqp
				DelOneEqpFromEqpG(dwVEName, path, pPEqp->sCfgName);
		} 
		
		pPEqp++;
	}  

	DelOneEqp(path, dwVEName);

	ParaFileRemove(path, filename, ".cdt");
}

void DelOneEqpGroup(char *path, DWORD dwEGName)
{
	char name[MAXFILENAME];
	int wEqpNum, i;
	VPEqp *pPEqp;
	  
	GetParaFileName(path, "device", ".cfg", name); 
	if (ReadParaFile(name, (BYTE *)g_pParafileTmp, MAXFILELEN) == ERROR)
		return;   

	wEqpNum = (g_pParafileTmp->nLength-sizeof(VFileHead))/sizeof(VPEqp);
	pPEqp=(VPEqp *)(g_pParafileTmp+1);

	for (i=0;i<wEqpNum;i++)
	{     
		if (pPEqp->dwName == dwEGName)
		{
			strcpy(name, pPEqp->sCfgName);
			break;
		}  
		pPEqp++;
	}  
	if (i<wEqpNum)   ParaFileRemove(path, name, ".cdg");

	DelOneEqp(path, dwEGName);
}

int DelOnePort(char *path, int id)
{
	int i,len;
	WORD wPortNum,wEqpNum;
	VPPort *pPPort;
	VPEqp *pPEqp;
	char pathname[MAXFILENAME]; 

    id = GetParaPortId(id);
	
	if ((id < PARA_START_NO) || (id > PARA_START_NO+COMM_NUM)
		|| ((id >= PARA_NET_START_NO) && (id < PARA_NET_USR_NO)))
		return(ERROR);

	GetParaFileName(path, "device", ".cfg", pathname); 
	if (ReadParaFile(pathname,(BYTE *)g_pParafileTmp,MAXFILELEN)==OK)
	{      
		wEqpNum=(g_pParafileTmp->nLength-sizeof(VFileHead))/sizeof(VPEqp);
		pPEqp=(VPEqp *)(g_pParafileTmp+1);

		for (i=0; i<wEqpNum; i++)
		{     
			if (pPEqp->wCommID == id)
				break;
			pPEqp++;
		}  

		if (i < wEqpNum)   return(ERROR);  //this port have dev
	}  

	GetParaFileName(path, "port", ".cfg", pathname); 
	if (ReadParaFile(pathname, (BYTE *)g_pParafileTmp, MAXFILELEN)==ERROR)
		return(ERROR);

	len=g_pParafileTmp->nLength-sizeof(VFileHead);
	wPortNum=len/sizeof(VPPort);      
	pPPort=(VPPort *)(g_pParafileTmp+1);

	for (i=0; i<wPortNum; i++)
	{
		if  (pPPort->id == id)
		{
			memcpy(pPPort, pPPort+1, len-(i+1)*sizeof(VPPort));
			break;     	
		}  
		else
			pPPort++;
	}  

	if (i < wPortNum)  //find
	{
		wPortNum--;
		len = wPortNum*sizeof(VPPort);
		if (len)
		{
			g_pParafileTmp->nLength=sizeof(VFileHead)+len;     
			if (WriteParaFile(pathname,g_pParafileTmp)==OK)
				return(wPortNum);
			else
				return(ERROR);         
		}
		else
		{
			ParaFileRemove(path,"port",".cfg");    
			return(OK);
		}   
	}  
	else
		return(ERROR);
}
#endif

int ReadPt1(WORD fd)
{
	if(g_Sys.MyCfg.wFDNum > 0)
	  return g_Sys.MyCfg.pFd[fd].pt;
	else
		return g_Sys.MyCfg.RunParaCfg.wPt1;
}
void WritePt1(WORD pt1)
{
	int i;
	smMTake(g_ParaPtCtSem);
	for(i =0;i < g_Sys.MyCfg.wFDNum;i++)
	{
		g_Sys.MyCfg.pFd[i].pt = pt1;
	}
	smMGive(g_ParaPtCtSem);
}


int ReadPt2(WORD fd)
{
	if(g_Sys.MyCfg.wFDNum > 0)
	  return g_Sys.MyCfg.pFd[fd].Un;
	else
		return g_Sys.MyCfg.RunParaCfg.wPt2;
}

void WritePt2(WORD pt2)
{
	int i;
	smMTake(g_ParaPtCtSem);
	for(i =0;i < g_Sys.MyCfg.wFDNum;i++)
	{
		g_Sys.MyCfg.pFd[i].Un= pt2;
	}
	smMGive(g_ParaPtCtSem);
}

int ReadCt1(WORD fd)
{
	if(fd < g_Sys.MyCfg.wFDNum)
	 return g_Sys.MyCfg.pFd[fd].ct;
	else
	 return g_Sys.MyCfg.RunParaCfg.LineCfg[fd].wCt1;
}

void WriteCt1(WORD ct1)
{
	int i;
	smMTake(g_ParaPtCtSem);
	for(i =0;i < g_Sys.MyCfg.wFDNum;i++)
	{
		g_Sys.MyCfg.pFd[i].ct = ct1;
	}
	smMGive(g_ParaPtCtSem);
}

int ReadCt2(WORD fd)
{
	if(fd < g_Sys.MyCfg.wFDNum)
	 return g_Sys.MyCfg.pFd[fd].In;
	else
	 return g_Sys.MyCfg.RunParaCfg.LineCfg[fd].wCt2;
}

void WriteCt2(WORD ct2)
{
	int i;
	smMTake(g_ParaPtCtSem);
	for(i =0;i < g_Sys.MyCfg.wFDNum;i++)
	{
		g_Sys.MyCfg.pFd[i].In = ct2;
	}
	smMGive(g_ParaPtCtSem);
}

int ReadCt01(WORD fd)
{
	if(fd < g_Sys.MyCfg.wFDNum)
	  return g_Sys.MyCfg.pFd[fd].ct0;
	else
		return g_Sys.MyCfg.RunParaCfg.LineCfg[fd].wCt01;
}
void WriteCt01(WORD ct01)
{
	int i;
	smMTake(g_ParaPtCtSem);
	for(i =0;i < g_Sys.MyCfg.wFDNum;i++)
	{
		g_Sys.MyCfg.pFd[i].ct0 = ct01;
	}
	smMGive(g_ParaPtCtSem);
}

int ReadCt02(WORD fd)
{
	if(fd < g_Sys.MyCfg.wFDNum)
		return g_Sys.MyCfg.pFd[fd].In0;
	else
		return g_Sys.MyCfg.RunParaCfg.LineCfg[fd].wCt02;
}
void WriteCt02(WORD ct02)
{
	int i;
	smMTake(g_ParaPtCtSem);
	for(i =0;i < g_Sys.MyCfg.wFDNum;i++)
	{
		g_Sys.MyCfg.pFd[i].In0= ct02;
	}
	smMGive(g_ParaPtCtSem);
}
	
int ReadYxFdT(WORD no)
{
	if(g_Sys.MyCfg.wDINum > 0)
		return g_Sys.MyCfg.pDi[no].dtime;
	else 
		return g_Sys.MyCfg.RunParaCfg.wYxfd;
}

void WriteYxFdT(WORD yxfdtime)
{
	int i;
	smMTake(g_ParaYxFdSem);
	for(i =0 ;i < g_Sys.MyCfg.wDINum; i++)
	{
		g_Sys.MyCfg.pDi[i].dtime = yxfdtime;
	}
	WriteYxTime_BM(yxfdtime);  //暂时去掉
	smMGive(g_ParaYxFdSem);
}

int ReadFzT(WORD no)
{
	if(g_Sys.MyCfg.wDONum > 0)
		return ((int)(g_Sys.MyCfg.pDo[no].fzontime));
	else
		return (int)g_Sys.MyCfg.RunParaCfg.wFzKeepT;
}
void WriteFzT(WORD fztime)
{
	int i;
	smMTake(g_ParaFzHzSem);
	for(i = 0;i < g_Sys.MyCfg.wDONum; i++)
	{
		g_Sys.MyCfg.pDo[i].fzontime = fztime;
	}
	WriteYkT_Bm(0,fztime);  
	smMGive(g_ParaFzHzSem);
}

int ReadHzT(WORD no)
{
	if(g_Sys.MyCfg.wDONum > 0)
		return ((int)g_Sys.MyCfg.pDo[no].hzontime);
	else
		return ((int)g_Sys.MyCfg.RunParaCfg.wHzKeepT);
}
void WriteHzT(WORD hztime)
{
	int i;
	smMTake(g_ParaFzHzSem);
	for(i = 0; i < g_Sys.MyCfg.wDONum; i++)
	{
		g_Sys.MyCfg.pDo[i].hzontime = hztime;
	}
	WriteYkT_Bm(1,hztime);  
	smMGive(g_ParaFzHzSem);
}

//pt1 pt2 ct1 ct2 ct01 ct02 yxfd fz hz
BOOL WriteSysPara()
{
	char pathname[MAXFILENAME];  
	int i,t;
	if (ReadParaFile("system.cfg", (BYTE *)g_pParafileTmp, MAXFILELEN) == OK)
	{
		struct VPSysConfig *pPSysCfg=(struct VPSysConfig *)(g_pParafileTmp+1);
		struct VPFdCfg *pPFdCfg = (struct VPFdCfg *)(pPSysCfg+1);
		struct VPAiCfg *pPAiCfg = (struct VPAiCfg *)(pPFdCfg+pPSysCfg->wFDNum);
		struct VPDiCfg *pPDiCfg = (struct VPDiCfg *)(pPAiCfg+pPSysCfg->wAINum);
		struct VPDoCfg *pPDoCfg = (struct VPDoCfg *)(pPDiCfg+pPSysCfg->wDINum);
		struct VPYcCfg *pPYcCfg = (struct VPYcCfg *)(pPDoCfg+pPSysCfg->wDONum);

		for (i = 0; i <  pPSysCfg->wFDNum; i++)
		{
			pPFdCfg[i].pt = g_Sys.MyCfg.RunParaCfg.wPt1;
			pPFdCfg[i].Un= g_Sys.MyCfg.RunParaCfg.wPt2;
			
			pPFdCfg[i].ct = g_Sys.MyCfg.RunParaCfg.LineCfg[i].wCt1;
			pPFdCfg[i].In= g_Sys.MyCfg.RunParaCfg.LineCfg[i].wCt2;
			pPFdCfg[i].ct0= g_Sys.MyCfg.RunParaCfg.LineCfg[i].wCt01;
			pPFdCfg[i].In0= g_Sys.MyCfg.RunParaCfg.LineCfg[i].wCt02;
		}
#ifdef INCLUDE_YX
		for(i = 0;i < pPSysCfg->wDINum;i++)
		{
			pPDiCfg[i].dtime = g_Sys.MyCfg.RunParaCfg.wYxfd;
		}
#endif
		
#ifdef INCLUDE_YK		
		for(i = 0;i < pPSysCfg->wDONum;i++)
		{
			pPDoCfg[i].fzontime = g_Sys.MyCfg.RunParaCfg.wFzKeepT;
			pPDoCfg[i].hzontime = g_Sys.MyCfg.RunParaCfg.wHzKeepT;
		}
#endif
		
#ifdef INCLUDE_YC
		for(i = 0; i < pPSysCfg->wYCNum ;i++)
		{
			t = g_Sys.MyCfg.pYc[i].type;
			if ((t >= YC_TYPE_Ia) && (t <= YC_TYPE_I0))
			{
				pPYcCfg[i].toZero = g_Sys.MyCfg.pYc[i].toZero/10;
			}
			else if ((t >= YC_TYPE_Ua) && (t <= YC_TYPE_U0))
			{
				pPYcCfg[i].toZero = g_Sys.MyCfg.pYc[i].toZero/10;
			}
		}
#endif	
		GetParaFileName(NULL,"system",".cfg",pathname); 
		
        return(WriteParaFile(pathname,g_pParafileTmp)); 
	}
	else
	{
		return ERROR;
	}
}

int WriteYXUIPara()
{
	struct VPTECfg *pWrtPTECfg;
	struct VPTrueYC *pPYC;
	struct VTrueYCCfg *pCfg;
	struct VTrueEqp *pTEqp;
	int maxno,i;
	char pathname[MAXFILENAME];  

	if ((pTEqp=g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo)==NULL)  return 0;
	maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
	
	if (ReadParaFile("myio.cde", (BYTE *)g_pParafileTmp, MAXFILELEN) == OK)
	{
			pWrtPTECfg=(struct VPTECfg*)(g_pParafileTmp+1);
			pPYC=(struct VPTrueYC *)(pWrtPTECfg+1);
			for( i = 0; i < maxno ; i++)  // 修改myio的电流电压越上限
			{
				pCfg=pTEqp->pYCCfg+i;
				if((pCfg->tyctype >= YC_TYPE_Ua)  && (pCfg->tyctype <= YC_TYPE_Uc))
				 pPYC->lLimit1 =  	pCfg->lLimit1;  //电压越限

				if((pCfg->tyctype >= YC_TYPE_Ia)  && (pCfg->tyctype <= YC_TYPE_Ic))
				 pPYC->lLimit1 =  	pCfg->lLimit1;  //电流越限

				if((pCfg->tyctype >= YC_TYPE_Ia)  && (pCfg->tyctype <= YC_TYPE_I0))
				 pPYC->dwCfg =  	pCfg->dwCfg;  //电流越限压板
				 
				pPYC ++;
			}
	}
	GetParaFileName(NULL,"myio",".cde",pathname); 
	return(WriteParaFile(pathname,g_pParafileTmp)); 
}

int ReadCellHHT(void)
{
#ifdef INCLUDE_CELL
	return g_Sys.MyCfg.CellCfg.pCtrl->dwDayBits;
#else
	return 0;
#endif
}
int ReadcellHHTime()
{
#ifdef INCLUDE_CELL
	return g_Sys.MyCfg.CellCfg.pCtrl->wHour;
#else
	return 0;
#endif
}

WORD ReadULP() //读零漂
{
#ifdef INCLUDE_YC
	int i,t;
	
	if((g_Sys.MyCfg.pYc == NULL) || ((g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic) == 0))
		return 0;
		
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if ((t >= YC_TYPE_Ua) && (t <= YC_TYPE_U0))
		{
			return ((WORD)(g_Sys.MyCfg.pYc[i].toZero/10));
		}
	}
#endif
	return 0;
}

WORD ReadILP() //读零漂
{
#ifdef INCLUDE_YC
	int i,t;
	
	if((g_Sys.MyCfg.pYc == NULL) || ((g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic) == 0))
		return 0;
		
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if ((t >= YC_TYPE_Ia) && (t <= YC_TYPE_I0))
		{
			return ((WORD)(g_Sys.MyCfg.pYc[i].toZero/10));
		}
	}
#endif
	return 0;
}

WORD ReadZLLP() //读直流
{
#ifdef INCLUDE_YC
	int i,t;
	
	if((g_Sys.MyCfg.pYc == NULL) || ((g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic) == 0))
		return 0;
		
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if (t == YC_TYPE_Dc)
		{
			return ((WORD)(g_Sys.MyCfg.pYc[i].toZero/10));
		}
	}
#endif
	return 0;
}

WORD ReadPLP() //读功率
{
#ifdef INCLUDE_YC
	int i,t;
	
	if((g_Sys.MyCfg.pYc == NULL) || ((g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic) == 0))
		return 0;
		
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if ((t >= YC_TYPE_Pa) && (t <= YC_TYPE_Qc)||(t >= YC_TYPE_P) && (t <= YC_TYPE_S))
		{
			return ((WORD)(g_Sys.MyCfg.pYc[i].toZero/10));
		}
	}
#endif
	return 0;
}

WORD ReadFLP() //读频率
{
#ifdef INCLUDE_YC
	int i,t;
	
	if((g_Sys.MyCfg.pYc == NULL) || ((g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic) == 0))
		return 0;
		
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if (t == YC_TYPE_Freq)
		{
			return ((WORD)(g_Sys.MyCfg.pYc[i].toZero/10));
		}
	}
#endif
	return 0;
}

WORD ReadCosLP() //读功率因素
{
#ifdef INCLUDE_YC
	int i,t;
	
	if((g_Sys.MyCfg.pYc == NULL) || ((g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic) == 0))
		return 0;
		
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if (t == YC_TYPE_Cos)
		{
			return ((WORD)(g_Sys.MyCfg.pYc[i].toZero/10));
		}
	}
#endif
	return 0;
}

int ReadUYX() //读电压越限
{
#ifdef INCLUDE_YC
	struct VTrueYCCfg *pCfg;
	struct VTrueEqp *pTEqp;
	int maxno,i;

	if ((pTEqp=g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo)==NULL)  return 0;
	maxno=(int)pTEqp->Cfg.wYCNum+(int)pTEqp->Cfg.wVYCNum-1;
	if (maxno<0)  return 0;	
	for (i=0;i<maxno;i++)
	{
		pCfg=pTEqp->pYCCfg+i;
		if ((pCfg->tyctype >= YC_TYPE_Ua)  && (pCfg->tyctype <= YC_TYPE_Uc))
		    return pCfg->lLimit1;
	}
#endif
	return 0;
}

int ReadIYX() //读电流越限
{
#ifdef INCLUDE_YC
	struct VTrueYCCfg *pCfg;
	struct VTrueEqp *pTEqp;
	int maxno,i;

	if ((pTEqp=g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo)==NULL)  return 0;
	maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
	if (maxno<0)  return 0;	
	for (i=0;i<maxno;i++)
	{
		pCfg=pTEqp->pYCCfg+i;
		if ((pCfg->tyctype >= YC_TYPE_Ia)  && (pCfg->tyctype <= YC_TYPE_Ic))
		   return pCfg->lLimit1;
	}
	
#endif
	return 0;
}

int ReadIYXYB() //读电流越限压板
{
#ifdef INCLUDE_YC
	struct VTrueYCCfg *pCfg;
	struct VTrueEqp *pTEqp;
	int maxno,i;

	if ((pTEqp=g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo)==NULL)  return 0;
	maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
	if (maxno<0)  return 0;	
	for (i=0;i<maxno;i++)
	{
		pCfg=pTEqp->pYCCfg+i;
		if ((pCfg->tyctype >= YC_TYPE_Ia)  && (pCfg->tyctype <= YC_TYPE_I0))
		   return ((pCfg->dwCfg>>4)&0x01);
	}
	
#endif
	return 0;
}
extern int myCellId;
void WriteCellHHT(WORD cellhht)
{
#ifdef INCLUDE_CELL
	VCellCfg *pcfg = (VCellCfg *)&g_Sys.MyCfg.CellCfg;
	VCellCtrl *pctrl = pcfg->pCtrl;
	smMTake(g_ParaCellSem);
	pcfg->dwCfg |= 0x01;
	pcfg->dwNum = 1;
	if(pcfg->wID == 0)
		pcfg->wID = myCellId;
	pctrl->dwDayBits = cellhht;
	pctrl->dwMode = pctrl->dwMode | 0x03 |CELL_MODE_ENDTIME;
	pctrl->dwTime = 1;
	smMGive(g_ParaCellSem);
#endif
}
void WriteCellHHTime(WORD cellhhtime)
{
#ifdef INCLUDE_CELL
	VCellCfg *pcfg = (VCellCfg *)&g_Sys.MyCfg.CellCfg;
	VCellCtrl *pctrl = pcfg->pCtrl;
	smMTake(g_ParaCellSem);
	pcfg->dwCfg |= 0x01;
	pcfg->dwNum = 1;
	if(pcfg->wID == 0)
		pcfg->wID = myCellId;
	pctrl->wHour = cellhhtime;
	pctrl->dwMode = pctrl->dwMode | 0x03 |CELL_MODE_ENDTIME;
	pctrl->dwTime = 1;
	smMGive(g_ParaCellSem);
#endif
}

void WriteULP(WORD svalue)
{
#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if ((t >= YC_TYPE_Ua) && (t <= YC_TYPE_U0))
		{
		    memcpy(&g_Sys.MyCfg.pYc[i].toZero,& svalue,USHORT_LEN);
			g_Sys.MyCfg.pYc[i].toZero *=10;
		}
	}
#endif
	WriteBMZero(0,svalue);
}

void WriteILP(WORD svalue)
{
	#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if ((t >= YC_TYPE_Ia) && (t <= YC_TYPE_I0))
		{
			memcpy(&g_Sys.MyCfg.pYc[i].toZero, &svalue,USHORT_LEN);
			g_Sys.MyCfg.pYc[i].toZero *=10;
		}
	}
	#endif
	WriteBMZero(1,svalue);
}
void WritePLP(WORD svalue)
{
#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if ((t >= YC_TYPE_Pa)&&(t >= YC_TYPE_Qc)||(t >= YC_TYPE_P) && (t <= YC_TYPE_S))
		{
		    memcpy(&g_Sys.MyCfg.pYc[i].toZero,& svalue,USHORT_LEN);
			g_Sys.MyCfg.pYc[i].toZero *=10;
		}
	}
#endif
	WriteBMZero(2,svalue);
}
void WriteFLP(WORD svalue)
{
#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if (t == YC_TYPE_Freq)
		{
		    memcpy(&g_Sys.MyCfg.pYc[i].toZero,& svalue,USHORT_LEN);
			g_Sys.MyCfg.pYc[i].toZero *=10;
		}
	}
#endif
	WriteBMZero(3,svalue);
}
void WriteZLLP(WORD svalue)
{
#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if (t == YC_TYPE_Dc)
		{
		    memcpy(&g_Sys.MyCfg.pYc[i].toZero,& svalue,USHORT_LEN);
			g_Sys.MyCfg.pYc[i].toZero *=10;
		}
	}
#endif
	WriteBMZero(4,svalue);
}
void WriteCosLP(WORD svalue)
{
#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<(g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic); i++)
	{
		t = g_Sys.MyCfg.pYc[i].type;
		if (t == YC_TYPE_Cos) 
		{
		    memcpy(&g_Sys.MyCfg.pYc[i].toZero,& svalue,USHORT_LEN);
			g_Sys.MyCfg.pYc[i].toZero *=10;
		}
	}
#endif
	WriteBMZero(5,svalue);
}
void WriteUYX(DWORD dvalue)  //电压越上限
{
	#ifdef INCLUDE_YC
	struct VTrueYCCfg *pCfg;
	struct VTrueEqp *pTEqp;
	int maxno,i;

	if ((pTEqp=g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo)==NULL)  return;
	maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
	if (maxno<0)  return;	
	for (i=0;i<maxno;i++)
	{
		pCfg=pTEqp->pYCCfg+i;
		if ((pCfg->tyctype >= YC_TYPE_Ua)  && (pCfg->tyctype <= YC_TYPE_Uc))
		   pCfg->lLimit1 = (long)dvalue ;
	}
#endif
}


void WriteIYX(DWORD dvalue)  //电流越上限
{
#ifdef INCLUDE_YC
	struct VTrueYCCfg *pCfg;
	struct VTrueEqp *pTEqp;
	int maxno,i;

	if ((pTEqp=g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo)==NULL)  return;
	maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
	if (maxno<0)  return;	
	for (i=0;i<maxno;i++)
	{
		pCfg=pTEqp->pYCCfg+i;
		if ((pCfg->tyctype >= YC_TYPE_Ia)  && (pCfg->tyctype <= YC_TYPE_Ic))
		   pCfg->lLimit1 = (long)dvalue;
	}
#endif
}
void WriteIYXYB(DWORD dvalue)  //电流越上限控制字
{
#ifdef INCLUDE_YC
	struct VTrueYCCfg *pCfg;
	struct VTrueEqp *pTEqp;
	int maxno,i;

	if ((pTEqp=g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo)==NULL)  return;
	maxno=pTEqp->Cfg.wYCNum+pTEqp->Cfg.wVYCNum-1;
	if (maxno<0)  return;	
	for (i=0;i<maxno;i++)
	{
		pCfg=pTEqp->pYCCfg+i;
		if((pCfg->tyctype >= YC_TYPE_Ia)  && (pCfg->tyctype <= YC_TYPE_I0))
		{
			if (dvalue == 1)
			    pCfg->dwCfg |= (1<<4);
			else
			    pCfg->dwCfg &= ~(1<<4);
		}
	}
#endif
}
	
BOOL WriteCellPara()
{
#ifdef INCLUDE_CELL
	char pathname[MAXFILENAME]; 
	
	if (ReadParaFile("cell.cfg", (BYTE *)g_pParafileTmp, MAXFILELEN) == OK)
	{
		struct VPCellCfg *pPCfg = (struct VPCellCfg *)(g_pParafileTmp+1);
		struct VPCellCtrl *pPCtrl = (struct VPCellCtrl *)(pPCfg+1);
		
		
		pPCfg->dwCfg |=  0x01;
		pPCfg->dwNum = 1;
		if(pPCfg->wID == 0)
			pPCfg->wID = myCellId;
		
		memset(pPCtrl,0,sizeof(struct VPCellCtrl));
		
		pPCtrl->dwDayBits = g_Sys.MyCfg.RunParaCfg.wDchhT;
		pPCtrl->wHour = g_Sys.MyCfg.RunParaCfg.wDchhTime;
		pPCtrl->dwMode = pPCtrl->dwMode | 0x03 |CELL_MODE_ENDTIME;
		pPCtrl->dwTime = 1;
		
		g_pParafileTmp->nLength = sizeof(struct VFileHead) + sizeof(struct VPCellCfg) + pPCfg->dwNum*sizeof(struct VPCellCtrl);
		g_pParafileTmp->wVersion = CFG_FILE_VER;
		g_pParafileTmp->wAttr = CFG_ATTR_NULL;
		
		GetParaFileName(NULL,"cell",".cfg",pathname); 
        return(WriteParaFile(pathname,g_pParafileTmp)); 
	}
	else
#endif
		return ERROR;
}

int ReadRunPara(WORD parano,char* pbuf)
{
	struct VParaInfo *fParaInfo;
	float fvalue;
	WORD svalue,line,lineno,no;
    DWORD dvalue;
	VRunParaCfg *prunparacfg;
	
	fParaInfo = (struct VParaInfo *)pbuf;
	
	prunparacfg = &g_Sys.MyCfg.RunParaCfg;
	svalue = 0;
	fvalue = 0.0;
	dvalue = 0;
	
	if((parano < (RUNPARA_ADDR + RPR_DV_NUM)) || (parano >= PRPARA_ADDR))
		return ERROR;
	
	if(parano < RUNPARA_LINEADDR)
	{
		no = parano - RUNPARA_ADDR;
		switch(no)
		{
			case RPR_PT1:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				svalue = ReadPt1(0);
				fvalue = (float)svalue/1000; //KV
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_PT2:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ReadPt2(0);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_DY:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ((float)prunparacfg->dwDyVal)/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_T_DY:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = (float)prunparacfg->wDyT/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_GY:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ((float)prunparacfg->dwGyVal)/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_T_GY:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = (float)prunparacfg->wGyT/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_ZZ:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ((float)prunparacfg->dwZzVal)/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_T_ZZ:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = (float)prunparacfg->wZzT/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_GZ:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ((float)prunparacfg->dwGzVal)/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_T_GZ:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = (float)prunparacfg->wGzT/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_T_YXFD:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				svalue = ReadYxFdT(0);
				fvalue = (float)svalue/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_T_FZ:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				svalue = ReadFzT(0) ;
				fvalue = (float)svalue/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_T_HZ:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				svalue = ReadHzT(0);
				fvalue = (float)svalue/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_T_HH:
				fParaInfo->type = UINT_TYPE;
				fParaInfo->len = UINT_LEN;
				dvalue = ReadCellHHT();
				memcpy(fParaInfo->valuech,&dvalue,fParaInfo->len);
				break;
			case RPR_T_HHT:
				fParaInfo->type = UINT_TYPE;
				fParaInfo->len = UINT_LEN;
				dvalue = ReadcellHHTime();
				memcpy(fParaInfo->valuech,&dvalue,fParaInfo->len);
				break;
				
			case RPR_V_ILP: //零飘
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
#if (DEV_SP == DEV_SP_TTU)
                fvalue = ReadILP()*0.01;
#else
				fvalue = ReadILP()*0.005;
#endif
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_ULP: //零飘
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ReadULP()*0.1;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_UYX: //电压越限
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ReadUYX();
				fvalue = fvalue/100;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_IYX:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ReadIYX();
				fvalue = fvalue/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_IYXYB: //电流越限压板
				fParaInfo->type = USHORT_TYPE;
				fParaInfo->len = USHORT_LEN;
				svalue = (WORD)ReadIYXYB();
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				break;
			default:
				fParaInfo->type = 4;
				fParaInfo->len = 0;
				svalue = 0;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				break;
		}
	}
	else
	{
		no = parano - RUNPARA_LINEADDR;
		line = no/RUNPARA_LINENUM ;
		lineno = no - (WORD)(line*RUNPARA_LINENUM);
		if (line >= MAX_FD_NUM)
			return ERROR;
		
		switch(lineno)
		{
			case RPR_LINE_CT1:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ReadCt1(line);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_LINE_CT2:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ReadCt2(line);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_LINE_CT01:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ReadCt01(line);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_LINE_CT02:
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				fvalue = ReadCt02(line);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_IL1P: //电流零漂
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				svalue = ReadILP();
				fvalue = (float)svalue/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_UL1P: //电压零漂
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				svalue = ReadULP();
				fvalue =  (float)svalue/1000;
				fvalue = fvalue*100/prunparacfg->wPt2;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_ZLDYLP: //直流零漂
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				svalue = ReadZLLP();
				fvalue =  (float)svalue/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_PLP://功率零漂
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				svalue = ReadPLP();
				fvalue =  (float)svalue/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_FLP: //频率零漂
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				svalue = ReadFLP();
				fvalue =  (float)svalue/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case RPR_V_COSLP: //功率因素零漂
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				svalue = ReadCosLP();
				fvalue =  (float)svalue/1000;
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;

			default:
				fParaInfo->type = 4;
				fParaInfo->len = 0;
				svalue = 0;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				break;
		}
	}
	return OK;
}

/*0 成功,
  1 信息体地址错误
  2 数据类型、长度错误
  3 值错误
  4 不配*/
int WriteRunParaYZ(WORD parano,char* pbuf)
{
	struct VParaInfo *fParaInfo;
	WORD line,no;
	fParaInfo = (struct VParaInfo*)pbuf;
	
	if((parano < RUNPARA_ADDR) || 
		((parano >= PRPARA_ADDR)&&(parano < PRPARA_TTU_ADDR+PRP_HAR_DEAD_V))||
		(parano > PRPARA_TTU_ADDR+PRP_DEAD_LOAD))
		 return 1;
	
    if((parano > RUNPARA_LINEADDR)&&(parano <= PRPARA_ADDR))
	{
		no = parano - RUNPARA_LINEADDR;
		line = no/RUNPARA_LINENUM ;
		if(line >= MAX_FD_NUM)
			return 1;
	}
	return CheckParaType_Len(fParaInfo->type,fParaInfo->len);
}

/*0 成功,
  1 信息体地址错误
  2 数据类型、长度错误
  3 值错误
  4 不配*/
int WriteRunPara(WORD parano,char* pbuf)
{
	struct VParaInfo *fParaInfo;
	float fvalue;
	DWORD dwvalue;
	WORD svalue;
	VRunParaCfg *prunparacfg;
	WORD line,lineno,no;
	BOOL bvalue =0;
	
	prunparacfg = &g_Sys.MyCfg.RunParaCfg;
	fParaInfo = (struct VParaInfo*)pbuf;
	svalue =dwvalue =0;
	if((parano < (RUNPARA_ADDR + RPR_DV_NUM)) || (parano >= PRPARA_ADDR))
		return 1;
	if(CheckParaType_Len(fParaInfo->type,fParaInfo->len))
	{
		return 2;
	}
	else
	{
		if(parano < RUNPARA_LINEADDR)
		{
			no = parano - RUNPARA_ADDR;
			switch(no)
			{
				case RPR_PT1:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wPt1 = fvalue*1000; //KV
					WritePt1(prunparacfg->wPt1);
					break;
				case RPR_PT2:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wPt2 = fvalue;
					WritePt2(fvalue);
					break;
				case RPR_V_DY:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					dwvalue =  fvalue*1000;
					prunparacfg->dwDyVal= dwvalue;
					break;
				case RPR_T_DY:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wDyT = fvalue*1000;
					break;
				case RPR_V_GY:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					dwvalue = fvalue*1000;
					prunparacfg->dwGyVal= dwvalue;
					break;
				case RPR_T_GY:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wGyT = fvalue*1000;
					break;
				case RPR_V_ZZ:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					dwvalue = fvalue*1000;
					prunparacfg->dwZzVal= dwvalue;
					break;
				case RPR_T_ZZ:			
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wZzT = fvalue*1000;
					break;
				case RPR_V_GZ:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					dwvalue = fvalue*1000;
					prunparacfg->dwGzVal= dwvalue;
					break;
				case RPR_T_GZ:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wGzT = fvalue*1000;
					break;
				case RPR_T_YXFD:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wYxfd = fvalue*1000;
					WriteYxFdT(prunparacfg->wYxfd);
					break;
				case RPR_T_FZ:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wFzKeepT = fvalue*1000;
					WriteFzT(prunparacfg->wFzKeepT);
					break;
				case RPR_T_HZ:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wHzKeepT = fvalue*1000;
					WriteHzT(prunparacfg->wHzKeepT);
					break;
				case RPR_T_HH:
					memcpy(&dwvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wDchhT = dwvalue;
					WriteCellHHT(dwvalue);
					break;
				case RPR_T_HHT:
					memcpy(&dwvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					prunparacfg->wDchhTime = dwvalue;
					WriteCellHHTime(dwvalue);
					break;
			    case RPR_V_ILP:
				   memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
#if (DEV_SP == DEV_SP_TTU)
					svalue = fvalue * 100;
#else
					svalue = fvalue * 200;
#endif
				    WriteILP(svalue);
				    break;
				case RPR_V_ULP:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
	                   svalue = fvalue * 10;
					WriteULP(svalue);
					break;
				case RPR_V_UYX:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					dwvalue = fvalue*100;
					WriteUYX(dwvalue);
					break;
				case RPR_V_IYX:
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					dwvalue = fvalue*1000;
					WriteIYX(dwvalue);
					break;
				case RPR_V_IYXYB:
					memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					WriteIYXYB(svalue);
					break;

				default:
					break;
			}
		}
		else
		{
			no = parano - RUNPARA_LINEADDR;
			line = no/RUNPARA_LINENUM ;
			lineno = no - (WORD)(line*RUNPARA_LINENUM);
			if(line >= MAX_FD_NUM)
				return 1;

		    switch(lineno)
			{
				 case RPR_LINE_CT1:
						memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
						prunparacfg->LineCfg[line].wCt1 = fvalue;
						WriteCt1(fvalue);
					break;
				 case RPR_LINE_CT2:
						memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
						prunparacfg->LineCfg[line].wCt2 = fvalue;
						WriteCt2(fvalue);
					break;
				 case RPR_LINE_CT01:
						memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
						prunparacfg->LineCfg[line].wCt01 = fvalue;
						WriteCt01(fvalue);
					break;
				 case RPR_LINE_CT02:
						memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
						prunparacfg->LineCfg[line].wCt02 = fvalue;
						WriteCt02(fvalue);
					break;		
				case RPR_V_IL1P:
						memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
						svalue = (WORD)(fvalue * 1000);
						WriteILP(svalue);
					break;
				case RPR_V_UL1P:
						memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
						svalue = (WORD)(fvalue * 10 * prunparacfg->wPt2);
						WriteULP(svalue);
					break;
				case RPR_V_ZLDYLP:
						memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
						svalue = (WORD)(fvalue * 1000);
						WriteZLLP(svalue);
					break;
				case RPR_V_PLP:
						memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
						svalue = (WORD)(fvalue * 1000);
						WritePLP(svalue);
					break;
				case RPR_V_FLP:
						memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
						svalue = (WORD)(fvalue * 1000);
						WriteFLP(svalue);
					break;
				case RPR_V_COSLP:
						memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
						svalue = (WORD)(fvalue * 1000);
						WriteCosLP(svalue);
					break;
				 default:
					 break;
			}
		}
	}
	return 0;
}

int CheckParaType_Len(BYTE Type,BYTE Len)
{
	switch(Type)
	{
		case BOOL_TYPE:
			if(Len != BOOL_LEN)
				return 2;
			break;
		case TINY_TYPE:
			if(Len != TINY_LEN)
				return 2;
			break;
		case UTINY_TYPE:
			if(Len != UTINY_LEN)
				return 2;
			break;
		case SHORT_TYPE:
			if(Len != SHORT_LEN)
				return 2;
			break;
		case USHORT_TYPE:				
			if(Len != USHORT_LEN)
			return 2;
			break;
		case INT_TYPE:
			if(Len != INT_LEN)
				return 2;
			break;
		case UINT_TYPE:
			if(Len != UINT_LEN)
				return 2;
			break;
		case LONG_TYPE:
			if(Len != LONG_LEN)
				return 2;
			break;
		case ULONG_TYPE:
			if(Len != ULONG_LEN)
				return 2;
			break;
		case FLOAT_TYPE:
			if(Len != FLOAT_LEN)
				return 2;
			break;
		case DOUBLE_TYPE:
			if(Len != DOUBLE_LEN)
				return 2;
			break;
		case STRING_TYPE:
			break;
		default:
			break;
	}
	return 0;
}

int WriteRunParaFile()
{
	VRunParaFileCfg * prunparacfgtemp;
	char fname[MAXFILENAME];
	VRunParaCfg *prunparacfg;
	BOOL ret;
	
	prunparacfg = &g_Sys.MyCfg.RunParaCfg;
	
	prunparacfgtemp = (VRunParaFileCfg*)(g_pParafileTmp + 1);
	memset(prunparacfgtemp,0,sizeof(VRunParaFileCfg));
	//依次赋值
    prunparacfgtemp->wPt1 = prunparacfg->wPt1;
    prunparacfgtemp->wPt2 = prunparacfg->wPt2;
    prunparacfgtemp->dwDyVal = prunparacfg->dwDyVal;
#if (DEV_SP == DEV_SP_TTU)
		prunparacfgtemp->wDyT= prunparacfg->wDyT;
		prunparacfgtemp->wGyT= prunparacfg->wGyT;
		prunparacfgtemp->wZzT= prunparacfg->wZzT;
		prunparacfgtemp->wGzT= prunparacfg->wGzT;
#else
    	prunparacfgtemp->wDyT= prunparacfg->wDyT/1000;
		prunparacfgtemp->wGyT= prunparacfg->wGyT/1000;
		prunparacfgtemp->wZzT= prunparacfg->wZzT/1000;
		prunparacfgtemp->wGzT= prunparacfg->wGzT/1000;
#endif
    prunparacfgtemp->dwGyVal = prunparacfg->dwGyVal;
    prunparacfgtemp->dwZzVal= prunparacfg->dwZzVal;
    prunparacfgtemp->dwGzVal= prunparacfg->dwGzVal;
    prunparacfgtemp->wYxfd= prunparacfg->wYxfd;
    prunparacfgtemp->wFzKeepT= prunparacfg->wFzKeepT;
    prunparacfgtemp->wHzKeepT= prunparacfg->wHzKeepT;
    prunparacfgtemp->wDchhT= prunparacfg->wDchhT;
    prunparacfgtemp->wDchhTime= prunparacfg->wDchhTime;
    memcpy(prunparacfgtemp->LineCfg,prunparacfg->LineCfg,MAX_FD_NUM*sizeof(VRunParaLineCfg));

	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(VRunParaFileCfg);
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = CFG_ATTR_NULL;
	sprintf(fname,"runpara.cfg");

	ret = TRUE;
	smMTake(g_ParafileSem);

	if (WriteParaFile(fname,g_pParafileTmp) ==  ERROR)
		ret = ERROR;
	BmWriteRunPara((BYTE*)&g_Sys.MyCfg.RunParaCfg,sizeof(VRunParaCfg));
	if(WriteSysPara() == ERROR)
    	ret = ERROR;
	if(WriteYXUIPara() == ERROR)
		ret = ERROR; 
	if(WriteCellPara() == ERROR)
		ret = ERROR;
	smMGive(g_ParafileSem);
	return ret;
}

void WriteProgramFile()
{
	struct VFileOPMsg FileOPMsg;
	BYTE* buf = (BYTE*) g_pParafileTmp;
	memcpy(&FileOPMsg,buf + MAXFILELEN -1024,sizeof(struct VFileOPMsg));
	WriteFile(&FileOPMsg,buf);
}

void WriteMsgFile(BYTE *pbuf)
{
    int i;
	BYTE num = *pbuf;
	struct VFileMsg *file = (struct VFileMsg*)(pbuf+1);

	for (i=0; i<num; i++)
    {
	    switch(file->type)
		{
#if 0
		    case FILE_TYPE_PR:
				//可以根据attr区分不同的文件，比如规约类，attr[0]=0为101，1为104
				WritePrFile(file->attr[0]);
				break;
#endif
			case FILE_TYPE_RUN:
				WriteRunParaFile(); //此处还缺写规约函数
				break;
			case FILE_TYPE_PEOGRAM:
				WriteProgramFile();
				break;
			default:
				break;
		}
		file++;
	}
}

void WriteFileMsg(BYTE num, const struct VFileMsg *file)
{
    BYTE buf[32];

	buf[0] = num;
	memcpy(buf+1, file, num*sizeof(struct VFileMsg));
	msgSend(B2F_ID, buf, num*sizeof(struct VFileMsg)+1, 1);
}

void remove_str_blank(char *dst,const char *src,int len)
{
	char c;
	int i;
	char *p;
	
	p = dst;
	i = 0;
	c = *src++;
	while ( (c != 0) && (i < len))
	{
		if (c != ' ')
		{
			*p++ = c;
			i++;
		}
		c = *src++;
	}
	*p = 0;
}


struct VParaDef SelfRunParaDef[] =
{
	{0x8001,"终端类型"},
	{0x8002,"终端操作系统"},
	{0x8003,"终端制造商"},
	{0x8004,"终端硬件版本"},
	{0x8005,"终端软件版本"},
	{0x8006,"终端软件版本校验码"},
	{0x8007,"终端通信规约类型"},
	{0x8008,"终端出厂型号"},
	{0x8009,"终端ID号"},
	{0x800A,"终端网卡MAC地址"},
	{0x800B,"装置IP1"},
	{0x800C,"装置IP2"},
	{0x800D,"串口1设置"},
	{0x800E,"串口2设置"},
	{0x800F,"串口3设置"},
	{0x8010,"串口4设置"},
	{0x8011,"PT二次变比"},
	{0x8012,"CT二次变比"},
	{0x8013,"终端制造商GB2312"},
	{0x8014,"终端通信规约类型GB2312"},
	{0x8020,"电流死区"},
	{0x8021,"交流电压死区"},
	{0x8022,"直流电压死区"},
	{0x8023,"功率死区"},
	{0x8024,"频率死区"},
	{0x8025,"功率因数死区"},
	{0x8435,"电压谐波死区"},
	{0x8436,"电流谐波死区"},
	{0x8437,"不平衡度死区"},
	{0x8438,"负载率死区"},
	{0x8026,"PT一次额定"},
	{0x8027,"PT二次额定"},
	{0x8028,"低电压报警门限值"},
	{0x8029,"低电压报警周期"},
	{0x802A,"过电压报警门限值"},
	{0x802B,"过电压周期"},
	{0x802C,"重载报警门限值"},
	{0x802D,"重载报警周期"},
	{0x802E,"过载报警门限值"},
	{0x802F,"过载报警周期"},
	{0x8030,"遥信防抖时间"},
	{0x8031,"分闸输出时间"},
	{0x8032,"合闸输出时间"},
	{0x8033,"蓄电池活化周期"},
	{0x8034,"蓄电池活化时刻"},
	{0x8035,"电流零漂"},
	{0x8036,"电压零漂"},
	{0x8037,"电压越限"},
	{0x8038,"电流越限"},
	{0x8039,"电流越限压板"},
};
char* DEV_TYPE[] = {"D30",
									  "D21",
									  "D22",
									  "F30",
									  "F21",
									  "F22",
									  "T20",
									  "D3A",
									  "D3C",
									  };
char* DEV_TYPE_NAME[] = {"D-30",
									  "D-21",
									  "D-22",
									  "F-30",
									  "F-21",
									  "F-22",
									  "F-30/Z",
									  "F-21/Z",
									  "F-22/Z",
									  "T",
									  };
BOOL ReadSelfRunParadef(WORD num,BYTE* buf)
{
	int max;
	max = sizeof(SelfRunParaDef)/sizeof(struct VParaDef);

	if(num >= max) 
		return ERROR;
	memcpy(buf,&SelfRunParaDef[num],sizeof(struct VParaDef));
	return OK;
}

void SelfPara_Init1() // 
{
	int i;	
	BYTE devflag = 0;
	BYTE mfrflag = 0;
	BYTE mfrflag_utf8 = 0;
	BYTE svflag = 0 , hvflag = 0;
	
	memset(&g_Sys.InPara[0][0],0,sizeof(g_Sys.InPara));
	g_Sys.byDevType = 0;

	extNvRamGet(NVRAM_DEV_ID,(BYTE*)&g_Sys.InPara[SELFPARA_ID][0],NVRAM_DEV_ID_LEN);
	extNvRamGet(NVRAM_DEV_TYPE_FLAG,&devflag,sizeof(devflag));

	//uft-8编码存新的地方，读也从新的读
	extNvRamGet(NVRAM_DEV_MFR_UTF8_FLAG,&mfrflag_utf8,sizeof(mfrflag_utf8));
	if(mfrflag_utf8 != 0x5A)
		memcpy(&g_Sys.InPara[SELFPARA_MFR][0],MFR_DEFAULT_UTF8,strlen(MFR_DEFAULT_UTF8));
	else
		extNvRamGet(NVRAM_DEV_MFR_UTF8,(BYTE*)&g_Sys.InPara[SELFPARA_MFR][0],PARACHAR_MAXLEN);

	//老的地方存放gb2312
	extNvRamGet(NVRAM_DEV_MFR_FLAG,&mfrflag,sizeof(mfrflag));
	if(mfrflag != 0x5A)
		memcpy(&g_Sys.InPara[SELFPARA_MFR_GB2312][0],MFR_DEFAULT_GB2312,strlen(MFR_DEFAULT_GB2312));
	else
		extNvRamGet(NVRAM_DEV_MFR,(BYTE*)&g_Sys.InPara[SELFPARA_MFR_GB2312][0],Old_MAXLEN);


	
	memcpy(&g_Sys.InPara[SELFPARA_TYPE][0], &g_Sys.InPara[SELFPARA_ID][0],3);
	
	for(i = 0;i < sizeof(DEV_TYPE)/sizeof(DEV_TYPE[0]);i++)
	{
		if(strcmp(g_Sys.InPara[SELFPARA_TYPE] , DEV_TYPE[i] ) == 0)
			break;
	}
	if(i >= sizeof(DEV_TYPE)/sizeof(DEV_TYPE[0])) //未设置装置类型
	{
			myprintf(SYS_ID, LOG_ATTR_ERROR, "device ID not set or set error , please set ID !");
			g_Sys.byDevType = 0;
	}
	else
	{
		if(i <3)
		{
			if(DEV_SP != DEV_SP_DTU)
			{
				WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "装置ID 设置错误,配置不是DTU");								
				myprintf(SYS_ID, LOG_ATTR_ERROR, "device ID ERROR , not DTU ,use default devtype!");
			}
			else
				g_Sys.byDevType = i;
		}
		else
		{
			if(DEV_SP == DEV_SP_DTU)
			{
				WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "装置ID 设置错误,配置不是FTU");								
				myprintf(SYS_ID, LOG_ATTR_ERROR, "device ID ERROR , not FTU ,use default devtype!");
			}
			else
				g_Sys.byDevType = i - 3;
		}
	}
		
	
    if(DEV_SP == DEV_SP_DTU)
    {
		sprintf(&g_Sys.InPara[SELFPARA_TYPE][0],DEV_TYPE[g_Sys.byDevType & 0x03]);
		if(devflag != 0x5A)
			sprintf(&g_Sys.InPara[SELFPARA_DEVTYPE][0],SELFPARA_DEVTYPE_NAME,DEV_TYPE_NAME[g_Sys.byDevType]);
    }
	else if ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_CTRL)) 
	{
		sprintf(&g_Sys.InPara[SELFPARA_TYPE][0],DEV_TYPE[(g_Sys.byDevType & 0x03) + 3]);
		if(devflag != 0x5A)
			sprintf(&g_Sys.InPara[SELFPARA_DEVTYPE][0],SELFPARA_DEVTYPE_NAME,DEV_TYPE_NAME[(g_Sys.byDevType & 0x03)+3]);
	}
	else if (DEV_SP == DEV_SP_WDG)
	{
		sprintf(&g_Sys.InPara[SELFPARA_TYPE][0],DEV_TYPE[(g_Sys.byDevType & 0x03) + 3]);
		if(devflag != 0x5A)
			sprintf(&g_Sys.InPara[SELFPARA_DEVTYPE][0],SELFPARA_DEVTYPE_NAME,DEV_TYPE_NAME[(g_Sys.byDevType & 0x03)+6]);
	}

	if(DEV_SP == DEV_SP_TTU)
	{
		sprintf(&g_Sys.InPara[SELFPARA_TYPE][0],DEV_TYPE[6]);
		if(devflag != 0x5A)
			sprintf(&g_Sys.InPara[SELFPARA_DEVTYPE][0],SELFPARA_DEVTYPE_NAME,DEV_TYPE_NAME[9]);
	}
	if(DEV_SP == DEV_SP_DTU_IU)
	{
		sprintf(&g_Sys.InPara[SELFPARA_TYPE][0],DEV_TYPE[7]);
		if(devflag != 0x5A)
			sprintf(&g_Sys.InPara[SELFPARA_DEVTYPE][0],SELFPARA_DEVTYPE_NAME,DEV_TYPE_NAME[10]);
	}
    if(DEV_SP == DEV_SP_DTU_PU)
	{
		sprintf(&g_Sys.InPara[SELFPARA_TYPE][0],DEV_TYPE[8]);
		if(devflag != 0x5A)
			sprintf(&g_Sys.InPara[SELFPARA_DEVTYPE][0],SELFPARA_DEVTYPE_NAME,DEV_TYPE_NAME[11]);
	}
	if(devflag == 0x5A)
		extNvRamGet(NVRAM_DEV_TYPE,(BYTE*)&g_Sys.InPara[SELFPARA_DEVTYPE][0],Old_MAXLEN);

	extNvRamGet(NVRAM_DEV_SV_FLAG, &svflag, 1);
	extNvRamGet(NVRAM_DEV_HV_FLAG, &hvflag, 1);
	sprintf(&g_Sys.InPara[SELFPARA_OS][0],SELFPARA_OSNAME);
	
	if(hvflag != 0x5A)
	{
		remove_str_blank(&g_Sys.InPara[SELFPARA_HV][0],sysHVer,HVVER_LEN);
	}
	else
		extNvRamGet(NVRAM_DEV_HV,(BYTE*)&g_Sys.InPara[SELFPARA_HV][0],Old_MAXLEN);
	
	if(svflag != 0x5A)
	{
		remove_str_blank(&g_Sys.InPara[SELFPARA_SV][0],sysSVer,SVVER_LEN);
	}
	else
		extNvRamGet(NVRAM_DEV_SV,(BYTE*)&g_Sys.InPara[SELFPARA_SV][0],Old_MAXLEN);
	memcpy(&g_Sys.InPara[SELFPARA_CRC][0],(BYTE*)&sysCrc,sizeof(WORD));

	ESDK_GetNetMac(0,g_Sys.InPara[SELFPARA_MAC]);
}

void SelfPara_Init() 
{
	int i;
	struct VPort *pPort;
	//Veeprom_ip_mac ip_mac;
	WORD m_wEqpGroupID; 
	char *str[3];

	for(i = 0; i < (COMM_NET_NUM - COMM_NET_SYS_NUM);i++)
	{
		pPort = &g_Task[COMM_NET_USR_NO + i].CommCfg.Port;

		if((strcmp(pPort->pcol,"GB104") == 0) && (pPort->pcol_attr == 1))
		{
			if(g_Task[COMM_NET_USR_NO + i].wEqpGroupNum)
			{
				m_wEqpGroupID = g_Task[COMM_NET_USR_NO + i].pEqpGroupRunInfo[0].wEqpID;
				
				if(g_Sys.Eqp.pInfo[m_wEqpGroupID].pEqpGInfo != NULL)
				{
					//if(g_Sys.Eqp.pInfo[pEqpG->pwEqpID[0]].pVEqpInfo != NULL)  //虚装置去掉
					{
							sprintf(&g_Sys.InPara[SELFPARA_PROTOCOL][0],SELFPARA_PROTOCOL104_UTF8);
							sprintf(&g_Sys.InPara[SELFPARA_PROTOCOL_GB2312][0],SELFPARA_PROTOCOL104_GB2312);
							break;
					}
				}
			}
			GetStrFromMyFormatStr(pPort->cfgstr[0], ':', 3, &str[0], &str[1], &str[2]);
			strcpy(gb_104_ip, str[1]);
		}
	}
	if(i == (COMM_NET_NUM - COMM_NET_SYS_NUM))
	{
		for (i = 0; i < COMM_SERIAL_NUM; i++)
		{
			pPort = &g_Task[COMM_SERIAL_START_NO + i].CommCfg.Port;

			if((strcmp(pPort->pcol,"GB101") == 0) && (pPort->pcol_attr == 1))
			{
				if(g_Task[COMM_SERIAL_START_NO + i].wEqpGroupNum)
				{
					m_wEqpGroupID = g_Task[COMM_SERIAL_START_NO + i].pEqpGroupRunInfo[0].wEqpID;
					
					if(g_Sys.Eqp.pInfo[m_wEqpGroupID].pEqpGInfo != NULL)
					{
						//if(g_Sys.Eqp.pInfo[pEqpG->pwEqpID[0]].pVEqpInfo != NULL)
						{
								sprintf(&g_Sys.InPara[SELFPARA_PROTOCOL][0],SELFPARA_PROTOCOL101_UTF8);
								sprintf(&g_Sys.InPara[SELFPARA_PROTOCOL_GB2312][0],SELFPARA_PROTOCOL101_GB2312);
								break;
						}
					}
				}
				GetStrFromMyFormatStr(pPort->cfgstr[0], ':', 2, &str[0], &str[1]);
				gb_101_port = atoi(str[0]);
			}
		}
	}

	/*extNvRamGet(NVRAM_AD_IP_MAC, (BYTE *)&ip_mac, sizeof(ip_mac));
	sprintf(&g_Sys.InPara[SELFPARA_MAC][0],"%02X:%02X:%02X:%02X:%02X:%02X" ,ip_mac.mac1[0],ip_mac.mac1[1],ip_mac.mac1[2],
	ip_mac.mac1[3],ip_mac.mac1[4],ip_mac.mac1[5]); // 一个地址只对应一个mac*/

	strcpy(&g_Sys.InPara[SELFPARA_IP1][0],g_Sys.AddrInfo.Lan1.sIP);
	strcpy(&g_Sys.InPara[SELFPARA_IP2][0],g_Sys.AddrInfo.Lan2.sIP);

	sprintf(&g_Sys.InPara[SELFPARA_PT2][0],"%d : %d",g_Sys.MyCfg.RunParaCfg.wPt1,g_Sys.MyCfg.RunParaCfg.wPt2);
	sprintf(&g_Sys.InPara[SELFPARA_CT2][0],"%d : %d",g_Sys.MyCfg.RunParaCfg.LineCfg[0].wCt1,g_Sys.MyCfg.RunParaCfg.LineCfg[0].wCt2);

	for(i = 0;i < 4;i++)
	{
		pPort = &g_Task[COMM_SERIAL_START_NO + i].CommCfg.Port;
		strcpy(&g_Sys.InPara[SELFPARA_COM1 + i][0],pPort->cfgstr[0]);
	}

	g_ParaPtCtSem = smMCreate();
	g_ParaYxFdSem = smMCreate();	
	g_ParaFzHzSem = smMCreate();
	g_ParaCellSem = smMCreate();	
	
}

int ReadSelfPara(WORD parano,char* pbuf)
{
	struct VParaInfo *fParaInfo;
	int i;
	WORD svalue;
	fParaInfo = (struct VParaInfo *)pbuf;
	
	if((parano < SELFPARA_ADDR) || (parano >= RUNPARA_ADDR))
		return ERROR;
	i =  parano - SELFPARA_ADDR;
	if(i >= SELFPARA_NUM)
	{
		fParaInfo->type = 4;
		fParaInfo->len = 0;
		svalue = 0;
		memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
	}
	else if(i != SELFPARA_CRC)
	{
		fParaInfo->type = STRING_TYPE;
		strcpy(fParaInfo->valuech, &g_Sys.InPara[i][0]);

		fParaInfo->len = (BYTE)(strlen(&g_Sys.InPara[i][0]) + 1);
	}
	else
	{
		fParaInfo->type = USHORT_TYPE;
		fParaInfo->len = USHORT_LEN;
		memcpy(fParaInfo->valuech,&g_Sys.InPara[i][0],fParaInfo->len);
	}
	
	return OK;
}

int ReadSelfParaGb2312(WORD parano,char* pbuf)
{
	struct VParaInfo *fParaInfo;
	int i;
	WORD svalue;
	fParaInfo = (struct VParaInfo *)pbuf;
	
	if((parano < SELFPARA_ADDR) || (parano >= RUNPARA_ADDR))
		return ERROR;
	i =  parano - SELFPARA_ADDR;

	if(i == SELFPARA_MFR)
		i = SELFPARA_MFR_GB2312;
	else if(i == SELFPARA_PROTOCOL)
		i = SELFPARA_PROTOCOL_GB2312;
		
	if(i >= SELFPARA_NUM)
	{
		fParaInfo->type = 4;
		fParaInfo->len = 0;
		svalue = 0;
		memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
	}
	else if(i != SELFPARA_CRC)
	{
		fParaInfo->type = STRING_TYPE;
		memcpy(fParaInfo->valuech,&g_Sys.InPara[i][0],sizeof(g_Sys.InPara[i]));

			fParaInfo->len = strlen(&g_Sys.InPara[i][0]) + 1;
	}
	else
	{
		fParaInfo->type = USHORT_TYPE;
		fParaInfo->len = USHORT_LEN;
		memcpy(fParaInfo->valuech,&g_Sys.InPara[i][0],fParaInfo->len);
	}
	
	return OK;
}

//只能从规约调用,查找 装置被主规约调用的次数
void ReadfileSynchro(WORD wTaskID, WORD wEqpID, struct VFileSynchro *VFSyn)
{
		int i,j;
		struct VVirtualEqp *pVEqp;
		WORD taskid,ddnum,teid;
		struct VPort *pPort;

		
		VFSyn->num = 0;
	    if (wEqpID>=*g_Sys.Eqp.pwNum)  
	    {
	       VFSyn->num = 0;
	       return;
	    }  
	  
		if(g_Sys.Eqp.pInfo[wEqpID].pTEqpInfo != NULL)
		{
			taskid = g_Sys.Eqp.pInfo[wEqpID].wTaskID;
			pPort = &g_Task[taskid].CommCfg.Port;
			if((strcmp(pPort->pcol,"GB101") == 0) && (pPort->pcol_attr == 0))  // 查找101主规约
			{
				VFSyn->num = 1;
				VFSyn->wTaskID[0] = taskid;
			}
			else
				VFSyn->num = 0;
			
			return;
		}
		//
		pVEqp = g_Sys.Eqp.pInfo[wEqpID].pVEqpInfo;
		if(pVEqp != NULL)
		{
			ddnum = pVEqp->Cfg.wDDNum;
			if((pVEqp->pDD == NULL) && (ddnum == 0))
			{
				VFSyn->num = 0;
				return;
			}
			for(i = 0; i < ddnum; i++)
			{
				teid = pVEqp->pDD[i].wTEID;
				if(g_Sys.Eqp.pInfo[teid].pTEqpInfo == NULL)
					continue;
				
				taskid = g_Sys.Eqp.pInfo[teid].wTaskID;
				
				for(j = 0; j < VFSyn->num;j++)
				{
					if(taskid == VFSyn->wTaskID[j])
						break;
				}
				if(j < VFSyn->num)
					continue;
				
				pPort = &g_Task[taskid].CommCfg.Port;
				if((strcmp(pPort->pcol,"GB101") == 0) && (pPort->pcol_attr == 0))  // 查找101主规约
				{
					VFSyn->wTaskID[VFSyn->num] = taskid;
					VFSyn->num ++;
				}
			}
		}	
}

