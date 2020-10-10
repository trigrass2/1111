/*------------------------------------------------------------------------
 Module:       	d2f.h
 Author:        solar
 Project:       
 State:         
 Creation Date:	2009-02-23
 Description:   
------------------------------------------------------------------------*/

#ifndef _B2F_H
#define _B2F_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_B2FSOCKET_NUM   1024  /*must pow 2*/

typedef struct{
    int read_write;  /*1-read 2-write 3-write imm 4-same for remove 0-none*/
	char fname[MAXFILENAME];
	DWORD offset;
	BYTE *buf;
	int len;
}VB2FSocket;


void buf2fileInit(void);
void buf2file(void);

int Buf2FileRead(VB2FSocket *pB2FSocket);
int Buf2FileWrite(VB2FSocket *pB2FSocket);
int Buf2FileDel(VB2FSocket *pB2FSocket);

#ifdef __cplusplus
}
#endif

#endif

