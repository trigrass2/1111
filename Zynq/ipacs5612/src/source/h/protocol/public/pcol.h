/*------------------------------------------------------------------------
 Module:		pcol.h
 Author:		solar
 Creation Date: 2008-10-20
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/
#ifndef _PCOL_H
#define _PCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syscfg.h"

#define PCOLCOMMBUFLEN	 1024 * 3

#define PCOL_FRAME_SHIELD   0xFFFF0000	   //屏蔽字
#define PCOL_FRAME_OK	    0x00010000	   //检测到一个完整的帧
#define PCOL_FRAME_ERR	    0x00020000	   //检测到一个校验错误的帧
#define PCOL_FRAME_LESS	    0x00030000	   //检测到一个不完整的帧（还未收齐）

BYTE Bch(BYTE *p, int l);
BYTE Lpc(BYTE SpecCode, BYTE *p, int l);
__inline BYTE Lrc(BYTE *p, int l)
{
	return Lpc(0x00,p,l);
}
WORD Crc16(WORD SpecCode, BYTE *p, int l);
WORD CheckSum(BYTE *buf, int l);

//标志位定义
#define PCOL_SYS_FLAG	    0	  //系统保留标志位区
#define PCOL_SYS_DEVFLAG    0	  //系统保留标志位区

#define PCOL_USER_FLAG	    32	  //用户标志位区
#define PCOL_USER_DEVFLAG   32	  //用户标志位区 

//标志操作相关定义
#define MAX_FLAGNO	  128	//每个模块最多的标志位数

typedef struct 
{
	BYTE Flag[ MAX_FLAGNO/8 ]; 
}VFLAGS;


//========================================================================================================
//	函数名称: PSetFlag
//	输入参数: pFlags：m_Flags缓冲区首址  FlagNo：标记号 Status ：标记的状态：ON/OFF
//	函数功能: 将Flags缓冲区内的第FlagNo个位置位成Status
//	返回值：  无
//========================================================================================================
void PSetFlag  (VFLAGS *pFlags,DWORD FlagNo);
void PClearFlag(VFLAGS *pFlags,DWORD FlagNo);
void PClearAllFlag(VFLAGS *pFlags);

//========================================================================================================
//	函数名称: PGetFlag
//	输入参数: pFlags：m_Flags缓冲区首址  FlagNo：标记号 
//	函数功能: 获取m_Flags缓冲区内的第FlagNo个位置的状态位
//	返回值：  0: 标志位为空   非0：标志位不为空
//========================================================================================================
DWORD PGetFlag(VFLAGS *pFlags,DWORD FlagNo);
BYTE ASCII2TOHEX(BYTE c1, BYTE c2);

typedef struct {
	WORD   wReadPtr;		//读指针
	WORD   wOldReadPtr; 	//读指针
	WORD   wWritePtr;		//写指针
	BYTE   *pBuf;			//缓冲区
}VPcolCommBuf;

class VPcol{			
	private:
    public:
		VPcol();

		BOOL m_bHaveRxFrm;
		VPcolCommBuf m_Rec;
		VPcolCommBuf m_RecFrm;
		VPcolCommBuf m_Send;
		
		void NeatenCommBuf(VPcolCommBuf *pCommBuf); 		
		
		int DoReceive(struct timeval *tmval, int *flag);
		int procData(void);
		BOOL SearchFrame(void);
		void DoSend(DWORD dwFlag);

		virtual DWORD SearchOneFrame(BYTE *Buf,WORD Len);			
		virtual int ProcFrm(void);
		virtual int readComm(BYTE *buf, int len, struct timeval *tmval); 	
		virtual int writeComm(BYTE *buf, int len, DWORD dwFlag); 			
};

#ifdef __cplusplus
}
#endif

#endif
