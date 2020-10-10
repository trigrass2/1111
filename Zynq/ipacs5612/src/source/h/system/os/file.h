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

#define  MAXFILENAME  32           /*�ļ����Ϊ32*/

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
	char cFileName[4*MAXFILENAME];  /*�ļ���*/
	DWORD dwSize;                   /*�ļ���С����ȡʱ��0*/
	DWORD dwOffset;                 /*ƫ��*/
	DWORD  dwLen;                    /*��д���������ݳ���*/
};

struct VFileStatus{
	DWORD dwSize;                 /*�ļ���С*/
	DWORD dwMode;                 /*�ļ�����*/
	DWORD dwTime;                 /*����ʱ��*/
	WORD wCrc;                    /*У����*/
	DWORD dwTempSize;             /*��ʱ�ļ���С*/
	DWORD dwTempMode;             /*��ʱ�ļ�����*/
	DWORD dwTempTime;             /*��ʱ����ʱ��*/
	WORD wTempCrc;                /*��ʱУ����*/  
};

struct VDirInfo{
	char cName[4*MAXFILENAME];      /*Ŀ¼/�ļ���*/
	DWORD dwSize;                 /*Ŀ¼/�ļ���С*/
	DWORD dwMode;                 /*Ŀ¼/�ļ�����*/
	DWORD dwTime;                 /*Ŀ¼/�ļ� ����ʱ�� ��1970��1��1��0���0�뿪ʼ��ʱ��,�����ʾ*/
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



