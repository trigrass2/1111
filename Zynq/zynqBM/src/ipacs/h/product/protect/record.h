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

#ifndef _RECORD_H
#define _RECORD_H

#ifdef __cplusplus
extern "C" {
#endif

#define  RECORD_START   0x01
#define  RECORD_STOP    0x02
#define  RECORD_ORGIN   0x00

typedef struct{
	long GainA[8];
	long GainB[8];

	int AiIndex[8];  //AI通道
	WORD wYxPort;
	WORD wYxBit; 
	DWORD DoMask;
	DWORD DhMask;
}VFdSamRecordIndex;

int recordInit(WORD wTaskID);		//初始化录波缓存
void InitFdSamRecordIndex(void);	//初始化录波FD通道索引参数
int WAV_Record_Scan(void);
int StartWavRecord(int fd, struct VCalClock *time, int flag);	//启动录波
void WAV_Record_Interrupt(void);		//中断中记录波数据
void RecordWaitWrite(int seg);	//录波存盘
struct VDirInfo * ReadRcdDir(WORD *dnum,BOOL flag);

#ifdef __cplusplus
}
#endif

#endif

