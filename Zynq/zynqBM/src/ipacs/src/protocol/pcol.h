/*------------------------------------------------------------------------
 Module:		pcol.h
 Author:		solar
 Project:		pas300m
 State: 		
 Creation Date: 2005-10-17
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
 ------------------------------------------------------------------------*/
#ifndef _PCOL_H
#define _PCOL_H

#include "syscfg.h"

#define PCOLCOMMBUFLEN	 2048

#define BM_RCV_BUF_SIZE     2048

#define PCOL_FRAME_SHIELD   0xFFFF0000	   
#define PCOL_FRAME_OK	    0x00010000	   
#define PCOL_FRAME_ERR	    0x00020000	   
#define PCOL_FRAME_LESS	    0x00030000	   

typedef struct
{
	WORD   wReadPtr;						
	WORD   wWritePtr;						
	BYTE   *pBuf;			
}VPcolCommBuf;

typedef struct
{
    int nId;	
	BOOL bHaveRxFrm;
	VPcolCommBuf Rec;
	VPcolCommBuf RecFrm;
	VPcolCommBuf Send;

	void (*ProcFrm)(void *arg);	
	DWORD (*SearchOneFrame)(BYTE *Buf, int Len);
	int (*read) (BYTE* pbuf, int buflen);
	int (*write) (BYTE* pbuf, int buflen);
}VPcol;	

BYTE Bch(BYTE *p, int l);
WORD CheckSum(BYTE *buf, int l);
WORD Crc16(WORD SpecCode, BYTE *p, int l);
BYTE ascii2bin(BYTE ch);

void PcolInit(int tid, VPcol *pcol, void (*ProcFrm)(), DWORD (*SearchOneFrame)(),void *read, void *write);
void NeatenCommBuf(VPcolCommBuf *pCommBuf);
int DoReceive(VPcol *pCol);
void ProcData(VPcol *pCol);
BOOL SearchFrame(VPcol *pCol);
void DoSend(VPcol *pCol, DWORD wFlag);

#endif

