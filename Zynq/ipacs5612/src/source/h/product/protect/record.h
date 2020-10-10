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

	int AiIndex[8];  //AIͨ��
	WORD wYxPort;
	WORD wYxBit; 
	DWORD DoMask;
	DWORD DhMask;
}VFdSamRecordIndex;

int recordInit(WORD wTaskID);		//��ʼ��¼������
void InitFdSamRecordIndex(void);	//��ʼ��¼��FDͨ����������
int WAV_Record_Scan(void);
int StartWavRecord(int fd, struct VCalClock *time, int flag);	//����¼��
void WAV_Record_Interrupt(void);		//�ж��м�¼������
struct VDirInfo * ReadRcdDir(WORD *dnum,BOOL flag);
#ifdef __cplusplus
}
#endif

#endif

