#ifndef _OS_LINUX_H
#define _OS_LINUX_H

#include <pthread.h>
#include <semaphore.h>
#include "syscfg.h"
#define Q_MSG_MAX_LEN      256
#define TIMER_EVERY_FLAG   0x80000000
extern DWORD g_PthreadTicks; 

typedef  struct
{
    sem_t sem;
	DWORD event;
}VLinuxEvent;

typedef struct
{
    int q_Len;
	int q_rp;
	int q_wp;
	BYTE *q_Pool;
}VLinuxMsg;

#endif
