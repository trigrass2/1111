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

	int AiIndex[8];  //AIͨ��
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

int comtradeInit(WORD wTaskID);		//��ʼ��¼������
void InitFdSamRecIndex(void);	//��ʼ��¼��FDͨ����������

int StartWavRcd(int fd, struct VCalClock *time, int flag);	//����¼��
void WAV_Interrupt(void);		//�ж��м�¼������
void comtradeWaitWrite(int seg);	//¼������

#ifdef __cplusplus
}
#endif

#endif

