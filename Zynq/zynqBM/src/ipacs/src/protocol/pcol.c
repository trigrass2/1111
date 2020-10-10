/*------------------------------------------------------------------------
 Module:		pcol.c
 Author:		solar
 Project:		pas300m
 State: 		
 Creation Date: 2005-10-17
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $ 
 ------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "pcol.h"

typedef struct{
    int wp;
	int rp;
	BYTE  buf[BM_RCV_BUF_SIZE];
}VBMRecv_Buf;	

//static VBMRecv_Buf  bmrecv_buf;

int BmReadLinux(BYTE* buf, int buflen);
int netWrite(BYTE* buf, int len);

void PcolInit(int tid, VPcol *pcol, void (*ProcFrm)(), DWORD (*SearchOneFrame)(),void *read, void *write)
{
	memset((void *)pcol,0,sizeof(VPcol));	
	pcol->nId = tid;
	pcol->bHaveRxFrm = FALSE;
	pcol->Rec.pBuf = (BYTE *)malloc(PCOLCOMMBUFLEN*sizeof(BYTE));
	pcol->Send.pBuf = (BYTE *)malloc(PCOLCOMMBUFLEN*sizeof(BYTE));
	pcol->ProcFrm = ProcFrm;
	pcol->SearchOneFrame = SearchOneFrame;
	pcol->read = (int (*) (BYTE*, int))read;
	pcol->write = (int (*) (BYTE*, int))write; 
	
}

void NeatenCommBuf(VPcolCommBuf *pCommBuf)
{
	register unsigned  int i,j;

	if (pCommBuf->wReadPtr == 0)
	{
		return ; 
	}

	if (pCommBuf->wReadPtr >= pCommBuf->wWritePtr)
	{
		pCommBuf->wReadPtr = pCommBuf->wWritePtr=0;
		return ;
	}

	if (pCommBuf->wWritePtr >= PCOLCOMMBUFLEN)
	{
		pCommBuf->wReadPtr = 0;
		pCommBuf->wWritePtr = 0;
		return ;
	}

	i = 0; 
	j = pCommBuf->wReadPtr;
	while (j < pCommBuf->wWritePtr)
	{
		pCommBuf->pBuf[i++] = pCommBuf->pBuf[j++];
	}

	pCommBuf->wReadPtr = 0; 
	pCommBuf->wWritePtr = i; 
}

int DoReceive(VPcol *pCol)
{
	int rc;
	
	NeatenCommBuf(&pCol->Rec);
	rc =  pCol->read(pCol->Rec.pBuf+pCol->Rec.wWritePtr, PCOLCOMMBUFLEN-pCol->Rec.wWritePtr);
	if (rc>0)
	{
		pCol->Rec.wWritePtr += (WORD)rc;
		pCol->bHaveRxFrm=TRUE;
		ProcData(pCol);
		return(rc);
	}

	return(0);
}

void ProcData(VPcol *pCol)
{
	while (pCol->bHaveRxFrm)
	{
       if (SearchFrame(pCol)==FALSE)  continue;
	   pCol->ProcFrm((void *)pCol);
	}
}

BOOL SearchFrame(VPcol *pCol)
{
	DWORD rc;
	WORD procLen;  
	int dataLen;

	while (1)
	{
		dataLen = pCol->Rec.wWritePtr - pCol->Rec.wReadPtr;
		if (dataLen <= 0)
		{
		    pCol->bHaveRxFrm=FALSE;
    		return(FALSE);
		}  

		rc = pCol->SearchOneFrame(&pCol->Rec.pBuf[pCol->Rec.wReadPtr], dataLen);
		procLen =(WORD)(rc & ~PCOL_FRAME_SHIELD);

		switch	(rc & PCOL_FRAME_SHIELD)
		{
			case PCOL_FRAME_OK:
				pCol->RecFrm.pBuf = &pCol->Rec.pBuf[pCol->Rec.wReadPtr];  
				pCol->RecFrm.wWritePtr = procLen;
				pCol->Rec.wReadPtr += procLen;
				if (pCol->Rec.wReadPtr >= pCol->Rec.wWritePtr)
				{
					pCol->bHaveRxFrm=FALSE;
				}
				return TRUE;
			case PCOL_FRAME_ERR:
				if (!procLen)  pCol->Rec.wReadPtr++;
				else  pCol->Rec.wReadPtr += procLen; 
				break;
			case PCOL_FRAME_LESS:
				pCol->Rec.wReadPtr += procLen; 
				pCol->bHaveRxFrm=FALSE;
				return FALSE;
		}
	}
}

void DoSend(VPcol *pCol, DWORD dwFlag)
{
	int rc =  pCol->write(pCol->Send.pBuf+pCol->Send.wReadPtr,pCol->Send.wWritePtr-pCol->Send.wReadPtr);
	
	if (rc > 0) pCol->Send.wReadPtr += rc;
}

