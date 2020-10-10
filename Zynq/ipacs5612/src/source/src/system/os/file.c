/*------------------------------------------------------------------------
 Module:       	file.cpp
 Author:        solar
 Project:       
 State:         
 Creation Date:	2008-08-14
 Description:   file manager
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
------------------------------------------------------------------------*/

#include "syscfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "os.h"
#include "bsp.h"
#include "file.h"
#include "fileext.h"
#include <errno.h>

char *myPath[]={"","/lib","/sys","/tmp","/other","/dat","/cfg","/subcfg"};

static  DWORD FileSmID;
static  BYTE  tmpbuf[256];

/*------------------------------------------------------------------------
 Procedure:     GetFileCRC16 ID:1
 Purpose:       �õ��ļ���CRC16
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
WORD GetFileCRC16(char *filename)
{
	FILE *fp;
	char  path[4*MAXFILENAME];	
	WORD crc=0,i,in;
	int ret;
	DWORD len;

	GetMyPath(path, filename);

    ret = smMTake(FileSmID);
	if (ret)
		return (0);
	
	if((fp=fopen(path, "rb")) == NULL)
	{
		smMGive(FileSmID);
		return (0);
	}	

	while(!feof(fp))
	{
		len = fread(tmpbuf, 1, 256,fp);
		if (len != 256)
			if (!feof(fp))	
			{
				smMGive(FileSmID);
				fclose(fp);
				return(0);
			} 

		for(i=0;i<len;i++)
		{
			in=((crc^tmpbuf[i])&0x00FF);
			crc=((crc>>8)&0x00FF)^crccode[in];
		}
	} 
    smMGive(FileSmID);
	fclose(fp);
	
	return(crc);
}

DWORD GetFileCRC32(char* filename)
{
    FILE *fp;
    DWORD crc,i,in,len;
	char  path[4*MAXFILENAME];	

	GetMyPath(path, filename);
		
	smMTake(FileSmID);
    if((fp=fopen(path,"rb"))==NULL)
	{
		smMGive(FileSmID);
		return(0);
    }	

    crc = 0xffffffff;
    while (!feof(fp))
	{
        len=fread(tmpbuf,sizeof(BYTE),256,fp);
        if (len!=256)
           if (!feof(fp))	
		   {
			   smMGive(FileSmID);
               fclose(fp);
               return(0);
		   } 

        for(i=0;i<len;i++)
		{
	        in=((crc^tmpbuf[i])&0x000000ff);
            crc=((crc>>8)&0x00ffffff)^crctable[in];
		}
	} 	
	smMGive(FileSmID);
	fclose(fp);
	
    return(crc ^ 0xffffffff);
}

DWORD GetFileCRC32InMem(const BYTE *buf, DWORD len)
{
    DWORD crc,i,in;

    crc = 0xffffffff;

    for(i=0;i<len;i++)
	{
        in=((crc^buf[i])&0x000000ff);
        crc=((crc>>8)&0x00ffffff)^crctable[in];
	}
	
    return(crc ^ 0xffffffff);
}

#if 0
DWORD FindFileCrc(char *filename)
{
    DWORD crc;
	char  path[4*MAXFILENAME];	
	
	GetMyPath(path, filename);

	crc = findSysFileCrc(path);
	if (crc == 0) crc = GetFileCRC32(filename);

	return(crc);
}

DWORD CalcFileCrc(char *filename)
{
    DWORD crc;
	char  path[4*MAXFILENAME];	
	
	GetMyPath(path, filename);

	crc = findSysFileCrc(path);
	if (crc == 0) crc = GetFileCRC32(filename);
	else crc = calcSysFileCrc(path);

	return(crc);
}

DWORD FindFileCrcInMem(char *buf, DWORD len)
{
    DWORD crc;
	
	crc = findSysFileCrcInMem(buf, len);
	if (crc == 0) crc = GetFileCRC32InMem(buf, len);

	return(crc);
}

DWORD CalcFileCrcInMem(char *buf, DWORD len)
{
    DWORD crc;
	
	crc = findSysFileCrcInMem(buf, len);
	if (crc == 0) crc = GetFileCRC32InMem(buf, len);
	else crc = calcSysFileCrcInMem(buf, len);

	return(crc);
}

STATUS FindFileSp(char *filename, char *sp)
{	
	char  path[4*MAXFILENAME];	

	GetMyPath(path, filename);
	return(findSysFileSp(path, sp));
}

STATUS FindFileVer(char *filename, char *ver)
{	
	char  path[4*MAXFILENAME];	

	GetMyPath(path, filename);
	return(findSysFileVer(path, ver));
}

STATUS FindFileUser(char *filename, char *user)
{	
	char  path[4*MAXFILENAME];	

	GetMyPath(path, filename);
	return(findSysFileUser(path, user));
}
#endif

STATUS FileIsExist(char *filename)
{
	FILE *fp;
	char  path[4*MAXFILENAME];	

	GetMyPath(path, filename);
	if ((fp = fopen(path, "r")) == NULL)  return ERROR;
	else
	{
		fclose(fp);
		return OK;
	}
}

/*------------------------------------------------------------------------
 Procedure:     GetThePath ID:1
 Purpose:       ����Ŀ¼���ļ�ȫ·��
 Input:         
 Output:		path:����ȫ·�����ļ���
 Errors:
------------------------------------------------------------------------*/
void GetThePath(char *path,int dirID, const char *filename)
{
	if (strchr(filename,'/')!=NULL)  //use def path
		strcpy(path,filename);
	else
	{
		strcpy(path,myPathRoot);
		strcat(path,myPath[dirID]);
		strcat(path,"/");	
		strcat(path,filename);	
	}  
}

/*------------------------------------------------------------------------
 Procedure:     GetMyPath ID:1
 Purpose:       �����ļ������Զ�ȡ���ļ�ȫ·��
 Input:         filename
 Output:		path:����ȫ·�����ļ���
 Errors:
------------------------------------------------------------------------*/
void GetMyPath(char *path,char *filename)
{
	int dirID, i, len;
	char *p,*q;

	q = NULL;
	for (p = filename; (*p != ' ') && (*p != EOS); p++)
	{
		if (*p == '.') 
		{
			q = p;
			continue;
		}	
	/*	if (islower(*p) == 0)	
			*p = tolower (*p);*/
	} 

	if (strchr(filename,'/')!=NULL)  //use def path
	{
		GetThePath(path,0,filename);
		return;
	}  

	if (filename[0]=='$')
	{
	    len = (int)strlen(filename);
		for (i=1; i<=len; i++)
			filename[i-1] = filename[i];
		dirID = DIR_TMP;
	}	
	else if ((strncmp(filename,"vxworks",7)==0)||(strncmp(filename,"bootrom",7)==0))
		dirID = DIR_ROOT;
	else
	{
		strcpy(path,myPathRoot);
		strcat(path,"/other/");
		strcat(path,filename);
		if (q == NULL) return;
		if ((strcmp(q,".dsoe")==0) || (strcmp(q,".dcos")==0) || (strcmp(q,".soe")==0) || (strcmp(q,".cos")==0))
		{			
			strcpy(path,SYS_PATH_ROOT2);
			strcat(path,"/event/soe/");
			strcat(path,filename);
			return; 
		}    
		if (strcmp(q,".evt")==0)
		{			
			strcpy(path,SYS_PATH_ROOT2);
			strcat(path,"/event/other/");
			strcat(path,filename);
			return; 
		}    
		if (strcmp(q,".bk")==0)
		{			
			strcpy(path,SYS_PATH_ROOT2);
			strcat(path,"/bakup/");
			strcat(path,filename);
			return; 
		}  
		if ((strcmp(q,".cfg")==0)||(strcmp(q,".cin")==0))
			dirID = DIR_CFG;
		/*if ((strcmp(q,".cfg")==0)||(strcmp(q,".cin")==0))
		{
			strcpy(path,"/mnt/cfgfiles/");
			strcat(path,filename);
			return; 
		}*/
		else if ((strcmp(q,".z")==0)||(strcmp(q,".g")==0)||(strcmp(q,".out")==0)||(strcmp(q,".a")==0)||(strcmp(q,".o")==0)||(strcmp(q,".lib")==0))
			dirID = DIR_LIB;
		else if (strcmp(q,".sys")==0)
			dirID = DIR_SYS;
		else if (strcmp(q,".dat")==0)
			dirID = DIR_DATA;
		else  dirID = DIR_OTHER;
	}        	    	

	GetThePath(path,dirID,filename);
}

/*------------------------------------------------------------------------
 Procedure:     GetFileSize ID:1
 Purpose:       �õ��ļ���С
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS GetFileSize(const char *name, DWORD *size)
{
	struct stat Stat;  	

	if (stat(name,&Stat)==ERROR)
		return(ERROR);
	else
	{
		*size=(DWORD)Stat.st_size;
		return(OK);
	}  
}

/*------------------------------------------------------------------------
 Procedure:     ReadFileStatus ID:1
 Purpose:       ��ָ���ļ���Ϣ
 Input:         struct VFileOPMsg{
                  char cFileName[MAXFILENAME];  //�ļ���
                  DWORD dwSize;                    
                  DWORD dwOffset;                  
                  DWORD dwLen;      //����������С������Ϊ���ص����ݳ���
                }
 Output:		struct VFileStatus{
                  DWORD dwSize;                 //�ļ���С
                  DWORD dwMode;                 //�ļ�����
                  DWORD dwTime;                 //����ʱ��
                  WORD wCrc;                   //У����
                  DWORD dwTempSize;             //��ʱ�ļ���С
                  DWORD dwTempMode;             //��ʱ�ļ�����
                  DWORD dwTempTime;             //��ʱ����ʱ��
                  WORD wTempCrc;               //��ʱУ����  
                }
               ��ʱ�ļ���Ϊ�ļ���ǰ��$

 Errors:
------------------------------------------------------------------------*/       
enum FileStatus ReadFileStatus(struct VFileOPMsg *pFileOPMsg, struct VFileStatus *buf)
{
	struct stat Stat;	/*status info */
	char  path[4*MAXFILENAME];	

	GetMyPath(path, pFileOPMsg->cFileName);
	if (stat(path,&Stat)==ERROR)
	{
		buf->dwSize=0;                 //�ļ���С
		buf->dwMode=0;                 //�ļ�����
		buf->dwTime=0;                 //����ʱ��
		buf->wCrc=0xFFFF;              //У����
		buf->dwTempSize=0;             //��ʱ�ļ���С
		buf->dwTempMode=0;             //��ʱ�ļ�����
		buf->dwTempTime=0;             //��ʱ����ʱ��
		buf->wTempCrc=0xFFFF;          //��ʱУ����  
	}  
	else
	{
		buf->dwSize=(DWORD)Stat.st_size;
		buf->dwMode=Stat.st_mode;
		buf->dwTime=(DWORD)Stat.st_mtime;
		buf->wCrc=GetFileCRC16(path);
	}  

	GetThePath(path,DIR_TMP,pFileOPMsg->cFileName);
	if (stat(path,&Stat)==ERROR)
	{
		buf->dwTempSize=0;             //��ʱ�ļ���С
		buf->dwTempMode=0;             //��ʱ�ļ�����
		buf->dwTempTime=0;             //��ʱ����ʱ��
		buf->wTempCrc=0xFFFF;          //��ʱУ����  
	}  
	else
	{
		buf->dwTempSize=(DWORD)Stat.st_size;
		buf->dwTempMode=Stat.st_mode;
		buf->dwTempTime=(DWORD)Stat.st_mtime;
		buf->wTempCrc=GetFileCRC16(path);
	}  

	pFileOPMsg->dwSize=sizeof(struct VFileStatus); 				   
	pFileOPMsg->dwOffset=0; 
	pFileOPMsg->dwLen=sizeof(struct VFileStatus);
	return(FileOk);
}

#define CP_BUF_LEN 4096
int	cp(const char *oldname, const char *newname)
{
	volatile int fd1, fd2;
	int len;
	char *buf;

	fd1 = open(oldname, O_RDONLY, 0);
	if(fd1 < 0) return -1;
		
	fd2 = open(newname, O_WRONLY| O_TRUNC | O_CREAT, 0);
	if(fd2 < 0) 
	{
		close(fd1);
		return -1;
	}

	buf = (char *)malloc(CP_BUF_LEN);
	if(buf == NULL) 
	{
		close(fd1); 
		close(fd2);
		return -1;
	}	
	
	do
	{
		len  = read(fd1, buf, CP_BUF_LEN);
		if(len < 0 || len > CP_BUF_LEN)	break;
		if (write(fd2, buf, (DWORD)len) != len)
		{
			close(fd1); 
			fsync(fd2);
			close(fd2);
			free(buf);
			return -1;
		}
	}while(len == CP_BUF_LEN);
	
	close(fd1); 
	fsync(fd2);
	close(fd2);

	free(buf);
	return 0;
}
	
/*------------------------------------------------------------------------
 Procedure:     ReadFile ID:1
 Purpose:       ���ļ�
 Input:         struct VFileOPMsg{
                  char cFileName[MAXFILENAME];  //�ļ���
                  DWORD dwSize;    //�����ļ���С               
                  DWORD dwOffset;  //��ƫ��
                  DWORD dwLen;      //����������С������Ϊʵ�ʶ��������ݳ���
                }
 Output:        FileOk
                FileEof
                FileError
 Errors:
------------------------------------------------------------------------*/       
enum FileStatus ReadFile(struct VFileOPMsg *pFileOPMsg, BYTE *buf)
{
	FILE *fp;
	int result;
	DWORD len;
	enum FileStatus i;
	char  path[4*MAXFILENAME];	

	if (buf==NULL)
	{
		if (pFileOPMsg->dwLen==0)
			return(FileOk);
		else
			return(FileError);
	}
	else if (pFileOPMsg->dwLen==0)
		return(FileOk);    

	smMTake(FileSmID);

	GetMyPath(path,pFileOPMsg->cFileName);
	if ((fp=fopen(path,"rb"))==NULL)
	{
		smMGive(FileSmID);
		return(FileError);
	}	

	if (GetFileSize(path,&pFileOPMsg->dwSize)==ERROR)
	{
		fclose(fp);
		return(FileError);
	}  

	result=fseek(fp,(long)pFileOPMsg->dwOffset,SEEK_SET);
	if (result) 
	{
		if (feof(fp))
			i=FileEof;
		else
			i=FileError;     
		fclose(fp);
		smMGive(FileSmID);
		return(i);
	}  
	else
	{
		len=(DWORD)fread(buf,sizeof(BYTE),pFileOPMsg->dwLen,fp);
		if (len!=pFileOPMsg->dwLen)
		{
			if (feof(fp)) 
				i=FileEof;
			else
				i=FileError; 
		}
		else
			i=FileOk;   

		pFileOPMsg->dwLen=len;       
		fclose(fp);  
		smMGive(FileSmID);
		return(i);
	}  
}    


/*------------------------------------------------------------------------
 Procedure:     WriteFile ID:1
 Purpose:       д�ļ�
 Input:         struct VFileOPMsg{
                  char cFileName[MAXFILENAME];  //�ļ���
                  DWORD dwSize;    //�ļ���С               
                  DWORD dwOffset;  //дƫ��
                  DWORD dwLen;      //д�����
                }
 Output:        FileOk
                FileError
 Errors:
------------------------------------------------------------------------*/
enum FileStatus WriteFile(struct VFileOPMsg *pFileOPMsg, const BYTE *buf)
{
	char dstpath[4*MAXFILENAME];
	char  path[4*MAXFILENAME];	
	FILE *fp;
	int result;
	DWORD len;

	if (buf==NULL)
	{
		if (pFileOPMsg->dwLen==0)
			return(FileOk);
		else
			return(FileError);
	}
	else if (pFileOPMsg->dwLen==0)
		return(FileOk);    

	smMTake(FileSmID);

	GetThePath(path,DIR_TMP,pFileOPMsg->cFileName);

	if (pFileOPMsg->dwOffset==0)
	{
		remove(path);
		fp=fopen(path,"wb");
	}	
	else
	{
		if (GetFileSize(path,&len)==ERROR)
			result=ERROR;
		else if (len<pFileOPMsg->dwOffset)
			result=ERROR;
		else
			result=OK;   

		if (result==ERROR)
		{    
			smMGive(FileSmID);  
			return(FileError);
		}
		else
			fp=fopen(path,"ab");
	}    

	if (fp==NULL)
	{    
		smMGive(FileSmID);
		return(FileError);
	}

	result=fseek(fp,(int)pFileOPMsg->dwOffset,SEEK_SET);  
	if (result) 
	{
		fclose(fp);
		smMGive(FileSmID);
		return(FileError);
	}  
	else
	{
		len=fwrite(buf,sizeof(BYTE),pFileOPMsg->dwLen,fp);
		if (len!=pFileOPMsg->dwLen)
		{
			fclose(fp);
			smMGive(FileSmID);
			return(FileError);
		}  
		fflush(fp);
		fsync(fileno(fp));
		fclose(fp);    

		if (pFileOPMsg->dwSize<=pFileOPMsg->dwOffset+pFileOPMsg->dwLen)
		{
			if (strchr(pFileOPMsg->cFileName,'/')==NULL)  //not use def path
			{
				GetMyPath(dstpath,pFileOPMsg->cFileName);
				remove(dstpath);
				if (cp(path,dstpath)==ERROR)
				{
					smMGive(FileSmID);
					return(FileError);
				}
				remove(path);
			}   
		}  

		smMGive(FileSmID);
		return(FileOk);
	}          
}

/*------------------------------------------------------------------------
 Procedure:     DelFile ID:1
 Purpose:       ɾ���ļ�
 Input:         cFileName:�ļ���
 Output:        FileOk
                FileError
 Errors:
------------------------------------------------------------------------*/
enum FileStatus DelFile(char *cFileName)
{
	char  path[4*MAXFILENAME];	

	GetMyPath(path,cFileName);

    if (strstr(path, "vxworks") != NULL) 		
		return(FileError);	
	if (remove(path))
		return(FileError);
	else
		return(FileOk);
}

/*------------------------------------------------------------------------
 Procedure:     FileRename ID:1
 Purpose:       �ļ�����
 Input:         
 Output:        FileOk
                FileError
 Errors:
------------------------------------------------------------------------*/
enum FileStatus FileRename(char *OldFileName,char *NewFileName)
{
	char dstpath[4*MAXFILENAME];
	char path[4*MAXFILENAME];	

	GetMyPath(path,OldFileName);
	GetMyPath(dstpath,NewFileName);

	if (rename(path,dstpath)==OK)
		return(FileOk);
	else
		return(FileError);
}  

/*------------------------------------------------------------------------
 Procedure:     ListDir ID:1
 Purpose:       ��Ŀ¼
 Input:         struct VFileOPMsg{
                  char cFileName[MAXFILENAME];  //Ŀ¼��
                  DWORD dwSize;                   //Ŀ¼���ļ��ܸ���
                  DWORD dwOffset;                 //Ŀ¼���ļ���� from 0
                  DWORD dwLen;                      //����������С������Ϊ���ص����ݳ���
                }
 Output:        FileOk
                FileError
 Errors:
------------------------------------------------------------------------*/
enum FileStatus ListDir(struct VFileOPMsg *pFileOPMsg,struct VDirInfo *buf)
{
    DIR		*pDir;		/* ptr to directory descriptor */
    struct dirent	*pDirEnt;	/* ptr to dirent */
    struct stat	Stat;	/*status info */
    DWORD i,j,k,num;
    struct VDirInfo *p;
    char desname[4*MAXFILENAME];

    if (buf==NULL)
    {
        if (pFileOPMsg->dwLen==0)
            return(FileOk);
        else
            return(FileError);
    }
    else if (pFileOPMsg->dwLen==0)
        return(FileOk);    

    i=0;
    p=buf;
    k=pFileOPMsg->dwLen/sizeof(struct VDirInfo);
    if (k==0)
    {
        pFileOPMsg->dwSize=0;
        pFileOPMsg->dwLen=0;
        return(FileError);
    }    

    pDir = opendir (pFileOPMsg->cFileName) ;

    if (pDir==NULL)
    {
        pFileOPMsg->dwSize=0;
        pFileOPMsg->dwLen=0;
        return(FileError);
    }  

    errno=OK;  /*VxWorks*/
    j=0;
    num=0;
	
    do 
    {
		pDirEnt = readdir (pDir);
		if (pDirEnt==NULL)
		{
			if (errno!=OK)
			{
				closedir (pDir);
				pFileOPMsg->dwSize=0;
				pFileOPMsg->dwLen=0;
				return(FileError);
			} 
		}    
		else
		{
			if ((strcmp(pDirEnt->d_name, ".")==0) || (strcmp(pDirEnt->d_name, "..")==0) || (strcmp(pDirEnt->d_name, "lost+found") == 0))
				continue;
			j++;
		}	
    }
    while ((pDirEnt!=NULL)&&(j<=pFileOPMsg->dwOffset));
    
    if (pDirEnt!=NULL)
    {      
		strcpy(desname,pFileOPMsg->cFileName);
		if (desname [ strlen(desname) -1 ] != '/')  
			strcat(desname, "/");      
		strcat(desname,pDirEnt->d_name);
		if (stat(desname,&Stat) >= 0)
		{
		    strncpy(p->cName,pDirEnt->d_name,4*MAXFILENAME);
		    p->cName[4*MAXFILENAME-1]='\0';
		    p->dwSize = (DWORD)Stat.st_size;
		    p->dwMode = Stat.st_mode;
		    p->dwTime = (DWORD)Stat.st_mtime;
		    p++;
		    i+=sizeof(struct VDirInfo);
		    num++;
        }        
    }        
    
    while ((pDirEnt!=NULL)&&(num<k))
    {
		pDirEnt = readdir (pDir);
		if (pDirEnt==NULL)
		{
			if (errno!=OK)
			{
				closedir (pDir);
				pFileOPMsg->dwSize=0;
				pFileOPMsg->dwLen=0;
				return(FileError);
			} 
		}    
		else
		{
			if ((strcmp(pDirEnt->d_name,".")==0) || (strcmp(pDirEnt->d_name,"..")==0) || (strcmp(pDirEnt->d_name, "lost+found") == 0))
				continue;
			j++;        
			strcpy(desname,pFileOPMsg->cFileName);
			if( desname [ strlen(desname) -1 ] != '/')  
				strcat(desname, "/");      
			strcat(desname,pDirEnt->d_name);
			if (stat(desname,&Stat) >= 0)
			{
			   strncpy(p->cName,pDirEnt->d_name,4*MAXFILENAME);
			   p->cName[4*MAXFILENAME-1]='\0';
			   p->dwSize =(DWORD)Stat.st_size;
			   p->dwMode = Stat.st_mode;
			   p->dwTime = (DWORD)Stat.st_mtime;
			   p++;
			   i+=sizeof(struct VDirInfo);
			   num++;
			}
		}  
    }

	while (pDirEnt!=NULL)
	{
		pDirEnt = readdir (pDir);	
		if (pDirEnt==NULL)
		{
			if (errno!=OK)
			{
				closedir (pDir);          
				pFileOPMsg->dwSize=0;
				pFileOPMsg->dwLen=0;
				return(FileError);
			} 
		}
		else
		{
			if ((strcmp(pDirEnt->d_name,".")==0) || (strcmp(pDirEnt->d_name,"..")==0) || (strcmp(pDirEnt->d_name, "lost+found") == 0))
				continue;
			j++;
		}	
	}  

    closedir (pDir);
    pFileOPMsg->dwSize=j;
    pFileOPMsg->dwLen=i;
    return(FileOk);    
}
  

/*------------------------------------------------------------------------
 Procedure:     DelDir ID:1
 Purpose:       ɾ��Ŀ¼
 Input:         cDirName:Ŀ¼��
 Output:        FileOk
                FileError
 Errors:
------------------------------------------------------------------------*/
enum FileStatus DelDir(const char *cDirName)
{  
	if (remove(cDirName)==ERROR)
		return(FileError);
	else
		return(FileOk);
}
  
/*------------------------------------------------------------------------
 Procedure:     MakeDir ID:1
 Purpose:       ����Ŀ¼
 Input:         cDirName:Ŀ¼��
 Output:        FileOk
                FileError
 Errors:
------------------------------------------------------------------------*/
enum FileStatus MakeDir(const char *cDirName)
{
	mode_t mode = umask(0);
	if (mkdir(cDirName, S_IRWXU|S_IRWXG|S_IRWXO) < 0)
	{
		umask(mode);
		return(FileError);
	}
	else
	{
		umask(mode);
		return(FileOk);
	}
}


int GetProfileInt(char* appname, const char* keyname, int idefault, char* filename)
{
	FILE *fp;
	char* str[2];
	char name[4*MAXFILENAME], tmp[100];
	int find, no = idefault;

	GetMyPath(name, filename);
	if((fp=fopen(name,"rb")) == NULL)  return idefault;

	tmp[0] = '\0';
	find = 0;
	sprintf(name, "[%s]", appname);
	while(!feof(fp))
	{
		fscanf(fp, "%s\r\n", tmp);

		if(strcmp(tmp, name) == 0)
		{
			find = 1;
			break;
		}
	}

	if(find)
	{
		while(!feof(fp))
		{
			fscanf(fp,"%s\r\n",tmp);

			if (tmp[0] == '#') continue;
			if (tmp[0] == '[') break;

			GetStrFromMyFormatStr(tmp, '=', 2, &str[0], &str[1]);

			if(strcmp(str[0], keyname) == 0)
			{
				no = atoi(str[1]);
				break;
			}
		}
	}	
	fclose(fp);
	
	return no;
}

float GetProfileFloat(char* appname, const char* keyname, float idefault, char* filename)
{
	FILE *fp;
	char* str[2];
	char name[4*MAXFILENAME], tmp[100];
	int find;
	float no = idefault;

	GetMyPath(name, filename);
	if((fp=fopen(name,"rb")) == NULL)  return idefault;

	tmp[0] = '\0';
	find = 0;
	sprintf(name, "[%s]", appname);
	while(!feof(fp))
	{
		fscanf(fp, "%s\r\n", tmp);

		if(strcmp(tmp, name) == 0)
		{
			find = 1;
			break;
		}
	}

	if(find)
	{
		while(!feof(fp))
		{
			fscanf(fp,"%s\r\n",tmp);

			if (tmp[0] == '#') continue;
			if (tmp[0] == '[') break;

			GetStrFromMyFormatStr(tmp, '=', 2, &str[0], &str[1]);

			if(strcmp(str[0], keyname) == 0)
			{
				no = (float)atof(str[1]);
				break;
			}
		}
	}	
	fclose(fp);
	
	return no;
}

int GetProfileString(char* appname, const char* keyname, char* returnname, char* filename)
{
	FILE *fp;
	char* str[2];
	char name[4*MAXFILENAME], tmp[100];
	int find, no = 0;

	GetMyPath(name, filename);
	if((fp=fopen(name,"rb")) == NULL)
	{
		returnname[0] = '\0';
		return 0;
	}

	tmp[0] = '\0';
	find = 0;
	sprintf(name, "[%s]", appname); 
	while(!feof(fp))
	{
		fscanf(fp, "%s\r\n", tmp);

		if(strcmp(tmp, name) == 0)
		{
			find = 1;
			break;
		}
	}

	returnname[0] = '\0';
	if(find)
	{
		while(!feof(fp))
		{
			fscanf(fp,"%s\r\n",tmp);

			if (tmp[0] == '#') continue;
			if (tmp[0] == '[') break;

			GetStrFromMyFormatStr(tmp, '=', 2, &str[0], &str[1]);

			if(strcmp(str[0],keyname)==0)
			{
				strcpy(returnname, str[1]);
				no = (int)strlen(str[1]);
				break;
			}
		}
	}   
	fclose(fp);

	return no;
}



/*------------------------------------------------------------------------
 Procedure: 	DefDirCheck ID:1
 Purpose:		��鲢����ϵͳ�����ȱʡĿ¼
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
void DefDirCheck(void)
{
	DWORD  i;
	char  path[4*MAXFILENAME];	
	struct stat filestat;

	for (i=0; i<NELEMENTS(myPath); i++)
	{
		strcpy(path,myPathRoot);
		strcat(path,myPath[i]);
		if ((stat(path, &filestat) >= 0) && (S_ISDIR(filestat.st_mode)))
			continue;

		if (MakeDir(path)==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '%s' failed!", path);
		else  myprintf(SYS_ID, LOG_ATTR_INFO, "Auto create Dir '%s' success!", path);		
	}

	//log·�� 
	sprintf(path,"%s/log",SYS_PATH);
	if ((stat(path, &filestat) >= 0) && (S_ISDIR(filestat.st_mode)))
		return;

	if (MakeDir(path)==FileError)  myprintf(SYS_ID, LOG_ATTR_WARN, "Auto create Dir '%s' failed!", path);
	else  myprintf(SYS_ID, LOG_ATTR_INFO, "Auto create Dir '%s' success!", path);		
}

/*------------------------------------------------------------------------
 Procedure: 	FileManagerInit ID:1
 Purpose:		�ļ���������ʼ��
 Input: 		
 Output:		
 Errors:
------------------------------------------------------------------------*/
STATUS FileManagerInit(void)
{

	FileSmID = smMCreate();

	DefDirCheck();

	return(OK);
}  

