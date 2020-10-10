/*------------------------------------------------------------------------
 Module:       	file.h
 Author:        solar
 Project:       
 State:         
 Creation Date:	2008-10-29
 Description:   
------------------------------------------------------------------------*/

#ifndef _SYSFILE_H
#define _SYSFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mytypes.h"

#define  MAXFILELEN   (200*1024)

#define  MAXFILENAME  32           /*文件名最长为32*/

#define  DIR_ROOT   0
#define  DIR_LIB    1
#define  DIR_SYS    2
#define  DIR_TMP    3
#define  DIR_OTHER  4
#define  DIR_DATA   5
#define  DIR_CFG    6
#define  DIR_SUBCFG 7

#define  myPathRoot SYS_PATH_ROOT


struct VFileOPMsg{
	char cFileName[4*MAXFILENAME];  /*文件名*/
	DWORD dwSize;                   /*文件大小，读取时填0*/
	DWORD dwOffset;                 /*偏移*/
	DWORD  dwLen;                    /*读写操作的数据长度*/
};

struct VFileStatus{
	DWORD dwSize;                 /*文件大小*/
	DWORD dwMode;                 /*文件属性*/
	DWORD dwTime;                 /*创建时间*/
	WORD wCrc;                    /*校验码*/
	DWORD dwTempSize;             /*临时文件大小*/
	DWORD dwTempMode;             /*临时文件属性*/
	DWORD dwTempTime;             /*临时创建时间*/
	WORD wTempCrc;                /*临时校验码*/  
};

struct VDirInfo{
	char cName[4*MAXFILENAME];      /*目录/文件名*/
	DWORD dwSize;                 /*目录/文件大小*/
	DWORD dwMode;                 /*目录/文件属性*/
	DWORD dwTime;                 /*目录/文件 创建时间 从1970年1月1日0点分0秒开始的时间,以秒表示*/
};

#define FILEOK   0  
#define FILEEOF  1 

enum FileStatus{FileOk,FileEof,FileError};

extern char *myPath[];

STATUS FileManagerInit(void);

DWORD FindFileCrc(char *filename);
DWORD CalcFileCrc(char *filename);
DWORD FindFileCrcInMem(char *buf, DWORD len);
DWORD CalcFileCrcInMem(char *buf, DWORD len);
STATUS FindFileSp(char *filename, char *sp);
STATUS FindFileVer(char *filename, char *ver);
STATUS FindFileUser(char *filename, char *user);

STATUS FileIsExist(char *filename);
void GetThePath(char *path,int dirID,const char *filename);
void GetMyPath(char *path,char *filename);
STATUS GetFileSize(const char *name,DWORD *size);

enum FileStatus ReadFileStatus(struct VFileOPMsg *pFileOPMsg, struct VFileStatus *buf);
enum FileStatus ReadFile(struct VFileOPMsg *pFileOPMsg, BYTE *buf);
enum FileStatus WriteFile(struct VFileOPMsg *pFileOPMsg, const BYTE *buf);
enum FileStatus DelFile(char *cFileName);
enum FileStatus FileRename(char *OldFileName,char *NewFileName);

enum FileStatus ListDir(struct VFileOPMsg *pFileOPMsg,struct VDirInfo *buf);
enum FileStatus DelDir(const char *cDirName);
enum FileStatus MakeDir(const char *cDirName);

int GetProfileInt(char* appname, const char* keyname, int idefault, char* filename);
float GetProfileFloat(char* appname, const char* keyname, float idefault, char* filename);
int GetProfileString(char* appname, const char* keyname, char* returnname, char* filename);


#ifdef __cplusplus
}
#endif

#endif



