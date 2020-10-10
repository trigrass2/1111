/*------------------------------------------------------------------------
 Module:        comtrade.h
 Author:         
 Project:       
 Creation Date:	2009-03-23
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef _COMTRADE_H
#define _COMTRADE_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct{
	long GainA[8];
	long GainB[8];

	int AiIndex[8];  //AI通道
	WORD wYxPort;
	WORD wYxBit; 
	DWORD DoMask;
	DWORD DhMask;
}VFdSamRecIndex;

enum
{
  REC_START=0x01,
  REC_STOP=0x02,
  REC_NORMAL=0x04,
  REC_CON=0x08,
  REC_ORGIN=0x00
};


enum
{
  REC_1MS,
  REC_4MS
};

typedef struct{
	BYTE type;
	BYTE seg;
}VFdRecMsg;

int comtradeInit(WORD wTaskID);		//初始化录波缓存
void InitFdSamRecIndex(void);	//初始化录波FD通道索引参数

int StartWavRcd(int fd, struct VCalClock *time, int flag);	//启动录波
void WAV_Interrupt(void);		//中断中记录波数据
void comtradeWaitWrite(int seg);	//录波存盘

#ifdef __cplusplus
}
#endif

#endif

