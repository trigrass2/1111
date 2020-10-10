/*------------------------------------------------------------------------
 Module:		pcol.cpp
 Author:		solar
 Project:		
 Creation Date: 2008-10-20
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
 ------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef INCLUDE_PCOL

#include <string.h>
#include "sys.h"
#include "pcol.h"

extern WORD crccode[];
static char bchcode[0x100]={
		0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
		0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D, //10
		0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
		0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, //20
		0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
		0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, //30
		0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
		0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD, //40
		0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
		0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, //50
		0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
		0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, //60
		0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
		0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A, //70
		0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
		0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,  //80
		0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
		0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,  //90
		0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
		0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,  //a0
		0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
		0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,  //b0
		0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
		0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,   //c0
		0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
		0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,   //d0
		0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
		0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,   //eo
		0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
		0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,   //f0
		0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
		0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3}
		;

//For Cdt
BYTE Bch(BYTE *p, int l)
{
	BYTE k;
	int j;
	k=0;

	for(j=0; j<l; ++j)
	{
		k = k^p[j];
		k = bchcode[k];
	};

	return (k^0xff);
};

BYTE Lpc(BYTE SpecCode, BYTE *p, int l)
{
	BYTE k=0;
	int j;

	for(j=0; j<l; j++)
		k = k^p[j];
	k ^= SpecCode;

	return (k);
}

WORD Crc16(WORD SpecCode, BYTE *p, int l)
{
	int i;
	WORD index, crc=SpecCode;

	for(i=0; i<l; i++)
	{
		index = ((crc^p[i])&0x00FF);
		crc = ((crc>>8)&0x00FF) ^ crccode[index];
	}

	return(crc);
}  

WORD CheckSum(BYTE *buf, int l)
{
	BYTE *p=buf;
	WORD tmp=0;

	for(int i=0; i<l; i++)
	{
		tmp+=*p;
		p++;
	}  

	return(tmp);
}

BYTE aBitSelect[]={
	0x80,		0x40,		0x20,		0x10,
	0x8,		0x4,		0x2,		0x1};


/***************************************************************
	Function:SetFlag
		设置标志，将pFlags缓冲区内的第FlagNo个位置位成1
	参数：pFlags, FlagNo
		pFlags 标志缓冲区首址
		FlagNo 标记号
	返回：无
***************************************************************/
void PSetFlag(VFLAGS *pFlags, DWORD FlagNo)
{
	DWORD ByteNo; //FlagNo所处的字节号
	DWORD BitNo;  //FlagNo在字节内所处的位号
	
	if (FlagNo >= MAX_FLAGNO )//越界
		return ;

	ByteNo = FlagNo >> 3;
	BitNo  = FlagNo & 0x7;

	pFlags->Flag[ByteNo] |= aBitSelect[BitNo];
}

/***************************************************************
	Function:ClearFlag
		清除标志，将pFlags缓冲区内的第FlagNo个位置位成0
	参数：pFlags, FlagNo
		pFlags 标志缓冲区首址
		FlagNo 标记号
	返回：无
***************************************************************/
void PClearFlag(VFLAGS *pFlags, DWORD FlagNo)
{
	DWORD ByteNo; 
	DWORD BitNo;  
	
	if (FlagNo >= MAX_FLAGNO )
		return ;

	ByteNo = FlagNo >> 3;
	BitNo  = FlagNo & 0x7;

	pFlags->Flag[ByteNo] &= ~aBitSelect[BitNo];
}

void PClearAllFlag(VFLAGS *pFlags)
{
	memset(pFlags, 0, sizeof(VFLAGS));
}

/***************************************************************
	Function:GetFlag
		获取pFlags缓冲区内的第FlagNo个位置的状态位
	参数：pFlags, FlagNo
		pFlags 标志缓冲区首址
		FlagNo 标记号
	返回：标志状态，1 有效，0 无效
***************************************************************/
DWORD PGetFlag(VFLAGS *pFlags, DWORD FlagNo)
{
	// 设置每个模块的标志
	DWORD ByteNo; //FlagNo所处的字节号
	DWORD BitNo;  //FlagNo在字节内所处的位号
	BYTE *pFlag = (BYTE *)pFlags;//

	ByteNo = FlagNo >> 3;//第5到8位
	BitNo  = FlagNo & 7;//取低三位

	return (pFlag[ByteNo] & aBitSelect[BitNo]);
}

BYTE ASCII2TOHEX(BYTE c1, BYTE c2)
{
	if( (c1 >= '0') && (c1 <= '9') )
		c1 -= '0';
	else if( (c1 >= 'a') && (c1 <= 'f') )
		c1 = c1 - 'a' + 10;
	else if( (c1 >= 'A') && (c1 <= 'F') )
		c1 = c1 - 'A' + 10;
	else
		c1 = 0;

	if( (c2 >= '0') && (c2 <= '9') )
		c2 -= '0';
	else if( (c2 >= 'a') && (c2 <= 'f') )
		c2 = c2 - 'a' + 10;
	else if( (c2 >= 'A') && (c2 <= 'F') )
		c2 = c2 - 'A' + 10;
	else
		c2 = 0;

	return (c1<<4)+c2;
}

VPcol::VPcol()
{
	memset((void *)&m_Rec,0,sizeof(m_Rec));	
	memset((void *)&m_RecFrm,0,sizeof(m_RecFrm));
	memset((void *)&m_Send,0,sizeof(m_Send));
	m_Rec.pBuf = (BYTE*)malloc(PCOLCOMMBUFLEN);
	m_Send.pBuf = (BYTE*)malloc(PCOLCOMMBUFLEN);
	m_bHaveRxFrm = FALSE;
}

void VPcol::NeatenCommBuf(VPcolCommBuf *pCommBuf)
{
	register unsigned  int i,j;

	if (pCommBuf->wReadPtr == 0)
	{
		return ; //读指针已经为0
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

int VPcol::DoReceive(struct timeval *tmval,int *flag)
{
	int rc;
	
	NeatenCommBuf(&m_Rec);
	rc = readComm(m_Rec.pBuf+m_Rec.wWritePtr, PCOLCOMMBUFLEN-m_Rec.wWritePtr,tmval);
	if (rc>0)
	{
		m_Rec.wWritePtr += (WORD)rc;
		m_bHaveRxFrm = TRUE;
		*flag = procData();
		return(rc);
	}

    *flag=0;
	return(0);
}

int VPcol::procData(void)
{
    int rc=0;
	
	while (m_bHaveRxFrm)
	{
       if (SearchFrame()==FALSE)  continue;
	   rc|=ProcFrm();
	}
	
	return(rc);
}

BOOL VPcol::SearchFrame(void)
{
	DWORD rc;
	WORD procLen;  //已处理过的字节数
	int dataLen;

	while (1)
	{
		dataLen = m_Rec.wWritePtr - m_Rec.wReadPtr;
		if (dataLen <= 0)
		{
		    m_bHaveRxFrm=FALSE;
    		return(FALSE);
		}  

		rc = SearchOneFrame(&m_Rec.pBuf[m_Rec.wReadPtr], dataLen);
		procLen =(WORD)(rc & ~PCOL_FRAME_SHIELD); //已处理过的字节数

		switch	(rc & PCOL_FRAME_SHIELD)
		{
			case PCOL_FRAME_OK:
				m_RecFrm.pBuf = &m_Rec.pBuf[m_Rec.wReadPtr];  //记录有效报文的起始地址
				m_RecFrm.wWritePtr = procLen; //记录有效报文的长度
				m_Rec.wReadPtr += procLen; //指针移到下一报文处
				if (m_Rec.wReadPtr >= m_Rec.wWritePtr)
				{
					m_bHaveRxFrm=FALSE;
				}
				return TRUE;

			case PCOL_FRAME_ERR:
				if (!procLen)  m_Rec.wReadPtr++;
				else  m_Rec.wReadPtr += procLen; //指针移到下一报文处
				break;
			case PCOL_FRAME_LESS:
				m_Rec.wReadPtr += procLen; //指针移到下一报文处
				m_bHaveRxFrm=FALSE;
				return FALSE;
		}//switch
	}//while
}

void VPcol::DoSend(DWORD dwFlag)
{
    int rc = writeComm(m_Send.pBuf+m_Send.wReadPtr,m_Send.wWritePtr-m_Send.wReadPtr,dwFlag);
	
	if (rc>0)	m_Send.wReadPtr += rc;
}

DWORD VPcol::SearchOneFrame(BYTE *Buf,WORD Len)
{
    return PCOL_FRAME_OK|Len;
}				

int VPcol::ProcFrm(void)
{
    return 0;
}

int VPcol::readComm(BYTE *buf, int len, struct timeval *tmval)
{
    return 0;
}	

int VPcol::writeComm(BYTE *buf, int len, DWORD dwFlag)
{
    return 0;
}
#endif

