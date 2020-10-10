#ifndef _OS_H
#define _OS_H

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "syscfg.h"

#define MAX_TASK_NUM  10

#define TICK2MSE			10
#define SECTOTICK(sec)      (sec*1000/TICK2MSE)

typedef struct{
	BYTE tid;
    DWORD event;
	WORD tmcnt;
	WORD tmlimit;
	void * args;
	ENTRYPTR func;
}VTask;

/*------------------------------------------------------------------------
�¼���־����(�������������,ȫ�ֶ���)
------------------------------------------------------------------------*/
#define EV_UFLAG_DI     0x00004000  /*Ӳң�Ž����¼�*/
#define EV_UFLAG_YX     0x00008000  /*��ң�Ž����¼�*/
#define EV_UFLAG        0x00010000  /*�����¼�*/
#define EV_MSG			0x00020000	/*�߳��յ���Ϣ*/
#define EV_TM1		    0x00040000	/*ʱ��1�¼�*/
#define EV_TM2	    	0x00080000	/*ʱ��2�¼�*/
#define EV_TM3   		0x00100000	/*ʱ��3�¼�*/
#define EV_RX_AVAIL		0x00200000	/*���տ���*/
#define EV_TX_AVAIL		0x00400000	/*���Ϳ���*/
#define EV_TX_IDLE		0x00800000	/*���Ϳ���*/
#define EV_COMM_IDLE	0x01000000	/*ͨѶ�ڿ���*/
#define EV_COMM_STATUS  0x02000000  /*����״̬�仯����ͨ���*/
#define EV_SUSPEND      0x04000000  /*�������*/
#define EV_COMMNO_MAP   0x08000000  /*��������ӳ����ͨ�ſ�*/
#define EV_DATAREFRESH  0x10000000  /*���ݿ����ݽ����󷢴��¼�,��Լ����
									��ɼ������յ����¼����������ˢ��
									���ݿ�*/
#define EV_RCD         0x20000000  /*���ݿ����ݽ����󷢴��¼�,��Լ����
									��ɼ������յ����¼����������ˢ��
									���ݿ�*/

extern VTask g_Task[MAX_TASK_NUM];

void tInit(void);
/*int  tCreate(WORD tid, void *__far targs, ENTRYPTR tfunc);*/
int  tCreate(int tid, void *targs, ENTRYPTR tfunc);
void tSleep(DWORD ticks);
void evSend(int tid, DWORD event);
void evReceive(int tid, DWORD ev_mask, DWORD *pevent);
void tmEvEvery(int tid, DWORD intval);
void tmEvCancle(int tid);
void tRun(void);

#endif
