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
事件标志定义(定义各任务事项,全局定义)
------------------------------------------------------------------------*/
#define EV_UFLAG_DI     0x00004000  /*硬遥信紧急事件*/
#define EV_UFLAG_YX     0x00008000  /*软遥信紧急事件*/
#define EV_UFLAG        0x00010000  /*紧急事件*/
#define EV_MSG			0x00020000	/*线程收到消息*/
#define EV_TM1		    0x00040000	/*时钟1事件*/
#define EV_TM2	    	0x00080000	/*时钟2事件*/
#define EV_TM3   		0x00100000	/*时钟3事件*/
#define EV_RX_AVAIL		0x00200000	/*接收可用*/
#define EV_TX_AVAIL		0x00400000	/*发送可用*/
#define EV_TX_IDLE		0x00800000	/*发送空闲*/
#define EV_COMM_IDLE	0x01000000	/*通讯口空闲*/
#define EV_COMM_STATUS  0x02000000  /*连接状态变化，如通或断*/
#define EV_SUSPEND      0x04000000  /*任务挂起*/
#define EV_COMMNO_MAP   0x08000000  /*任务重新映射新通信口*/
#define EV_DATAREFRESH  0x10000000  /*数据库数据解锁后发次事件,规约任务
									或采集任务收到此事件后必须立即刷新
									数据库*/
#define EV_RCD         0x20000000  /*数据库数据解锁后发次事件,规约任务
									或采集任务收到此事件后必须立即刷新
									数据库*/

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
