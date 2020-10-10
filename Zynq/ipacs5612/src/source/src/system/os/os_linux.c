/*------------------------------------------------------------------------
 Module:		os_linux.c
 Author:		helen
 Project:		 
 Creation Date: 2017-06-26
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/
#include "syscfg.h"

#include "os_linux.h"
#include "os.h"
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>


extern VThread *g_Thread;		
int g_smCount=0;
VTimer g_timer;
DWORD g_PthreadTicks = 0; 

int myTaskSpawn(char *name, ENTRYPTR entryPtr, int arg, int stack_size, int priority)
{
    int ret;
	pthread_t thread_t;
	pthread_attr_t attr;
	struct sched_param param;

	printf("%s thread create\n", name);

    ret = pthread_attr_init(&attr);
	if (ret != 0) return ret;
	
	param.sched_priority = priority;
	ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
	if (ret != 0) return ret;
	
	ret = pthread_attr_setschedparam(&attr, &param);
	if (ret != 0) return ret;
	
    ret = pthread_attr_setstacksize(&attr, (DWORD)stack_size);
	if (ret != 0) return ret;
	
    ret = pthread_create(&thread_t, &attr, (void *)entryPtr, (void *)arg);

	return ret;
}

/*------------------------------------------------------------------------
 Procedure:     ThCreate ID:1
 Purpose:       创建线程
 Input:         
 Output:		>=0: thid; <0: ERROR
 Errors:
------------------------------------------------------------------------*/
int thCreate(const char *name, ENTRYPTR entryPtr, int arg, int stack_size, int priority, int queue_len)
{
	int thid,ret;
	VLinuxEvent *pLinuxEvent;
	VLinuxMsg *pMsgQ;
	pthread_attr_t attr;
	struct sched_param param;

	thid = ThreadReq((int)arg, name);
	if (thid == ERROR) 
	{
		printf("ThreadReq return err \n");		
	
		return(ERROR);
	}

	pLinuxEvent= (VLinuxEvent*)malloc(sizeof(VLinuxEvent));
	if (pLinuxEvent ==NULL) 
	{
		printf("pLinuxEvent return err \n");		
		return ERROR;
	}
	ret = sem_init(&pLinuxEvent->sem, 0, 0);
	if (ret != 0) 
	{
	    free(pLinuxEvent);
	    return ERROR;
	}
	pLinuxEvent->event = 0;
    g_Thread[thid].pEv_group = pLinuxEvent;
	
	
	/*创建一个消息队列对象, 归属与该线程*/
	if (queue_len)
	{
		pMsgQ = (VLinuxMsg*)calloc(1, sizeof(VLinuxMsg));
		if (pMsgQ == NULL)
		{
		    printf("calloc msg return err \n");
			return ERROR;
		}

		pMsgQ->q_Pool = (BYTE*)calloc((DWORD)queue_len, Q_MSG_MAX_LEN);
		if (pMsgQ->q_Pool == NULL) 
		{
		    free(pMsgQ);
			printf("calloc msg->pool return err:ret \n");		
			return ERROR;	
		}

		pMsgQ->q_rp = pMsgQ->q_wp = 0;
		
		pMsgQ->q_Len = queue_len;

	}
	else
		pMsgQ = NULL;

	g_Thread[thid].pQu = (void *)pMsgQ;

	ret = pthread_attr_init(&attr);
	if (ret != 0) return ERROR;
	param.sched_priority = priority;
	ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
	if (ret != 0) return ERROR;
	ret = pthread_attr_setschedparam(&attr, &param);
	if (ret != 0) return ERROR;
    ret = pthread_attr_setstacksize(&attr, (DWORD)stack_size);
	if (ret != 0) return ERROR;

    g_Thread[thid].pTcb = malloc(sizeof(pthread_t));
	if (g_Thread[thid].pTcb == NULL) 
	{
		printf("malloc return err:ret\n");
		return ERROR;
	}

    ret = pthread_create((pthread_t*)g_Thread[thid].pTcb, &attr, (void *)entryPtr, (void *)arg);
	if (ret != 0) 
	{
		printf("pthread return err:ret = %d\n",ret);
		return ERROR;
	}
	
	g_Thread[thid].active = 1;


	return thid;
}


/*------------------------------------------------------------------------
 Procedure:     ThStart ID:1
 Purpose:       启动线程
 Input:         
 Output:		>=0: thid; <0: ERROR
 Errors:
------------------------------------------------------------------------*/
int thStart(int thid, int dogtm)
{ 
	g_Thread[thid].watchdog = dogtm;
	
	return OK;
}  

/*------------------------------------------------------------------------
 Procedure:     thPrioritySet ID:1
 Purpose:       线程优先级改变
 Input:         
 Output:		old priority
 Errors:
------------------------------------------------------------------------*/
int thPrioritySet(int thid, int newPriority)
{
    if(thid >= THREAD_MAX_NUM) 
	{
		return ERROR;
	}
    if(g_Thread[thid].used != 0) 
	{
		return ERROR;
	}
	return(newPriority);
}  

/*------------------------------------------------------------------------
 Procedure:     thSleep ID:1
 Purpose:       使当前线程睡眠
 Input:         ticks: 睡眠时间*10ms
 Output:		>=0: OK; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int thSleep(DWORD ticks)
{
	return(usleep(ticks*10000));

}

/*------------------------------------------------------------------------
 Procedure:     thSuspend ID:1
 Purpose:       挂起自身
 Input:         ticks: 睡眠时间
 Output:		>=0: OK; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int thSuspend(int thid)
{
    if(thid >= THREAD_MAX_NUM) 
	{
		return ERROR;
	}
    if(g_Thread[thid].used != 0) 
	{
		return ERROR;
	}
	return OK;
}

/*------------------------------------------------------------------------
 Procedure: 	thResume ID:1
 Purpose:		恢复
 Input: 	   
 Output:		>=0: OK; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int thResume(int thid)
{
    if(thid >= THREAD_MAX_NUM) 
	{
		return ERROR;
	}
    if(g_Thread[thid].used != 0) 
	{
		return ERROR;
	}
	return OK;
}

/*------------------------------------------------------------------------
 Procedure: 	thDelete ID:1
 Purpose:		删除
 Input: 	   
 Output:		>=0: OK; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int thDelete(int thid)
{
	if(thid >= THREAD_MAX_NUM) 
	{
		return ERROR;
	}
    if(g_Thread[thid].used != 0) 
	{
		return ERROR;
	}
	return OK;
}

/*------------------------------------------------------------------------
 Procedure: 	thIsSuspended ID:1
 Purpose:		任务阻塞状态
 Input: 	   
 Output:		TRUE: 阻塞; FALSE: 非阻塞
 Errors:
------------------------------------------------------------------------*/
BOOL thIsSuspended (int thid)
{
    if(thid >= THREAD_MAX_NUM) 
	{
		return FALSE;
	}
    if(g_Thread[thid].used != 0) 
	{
		return FALSE;
	}
	return FALSE;
}

/*------------------------------------------------------------------------
 Procedure:     evSend ID:1
 Purpose:       中断向线程发送事件
 Input:         thid: 线程号; event: 发送事件
 Output:		>=0: OK; <0: ERROR
 Errors:
------------------------------------------------------------------------*/
int evISRSend(int thid, DWORD event)
{
	if(thid >= THREAD_MAX_NUM) 
	{
		return ERROR;
	}
    if(g_Thread[thid].used != 0) 
	{
		return ERROR;
	}
	if(event == 0)
	{
	    return ERROR;
	}
	return OK;
}

/*------------------------------------------------------------------------
 Procedure:     evSend ID:1
 Purpose:       向线程发送事件
 Input:         thid: 线程号; event: 发送事件
 Output:		>=0: OK; <0: ERROR
 Errors:
------------------------------------------------------------------------*/
int evSend(int thid, DWORD event)
{
	VLinuxEvent *pEV;
	int err,val;

	/*判断thid的合法性*/
	/*提高效率*/
	/*if (thid >= THREAD_MAX_NUM) return ERROR;*/
	if (g_Thread[thid].active==0) return ERROR;

	pEV = (VLinuxEvent *)g_Thread[thid].pEv_group;
	pEV->event |= event;
	err = sem_getvalue(&pEV->sem,&val);
	if(err < 0)
		return ERROR;
    if (val == 0)
	    err = sem_post(&pEV->sem);
	
	if (err == 0)
		return OK;
	else
		return ERROR;
}

/*------------------------------------------------------------------------
 Procedure:     evReceive ID:1
 Purpose:       接收线程的事件
 Input:         thid: 线程号; event: 发送事件
 Output:		>=0: OK; <0: ERROR
 Errors:
------------------------------------------------------------------------*/
int evReceive(int thid, DWORD ev_mask, DWORD *pevent)
{
	int err;
	VLinuxEvent *pEV;

	/*判断thid的合法性*/
	//if (thid >= THREAD_MAX_NUM) return ERROR;
	//if (g_Thread[thid].used==0) return ERROR;

	pEV = (VLinuxEvent *)g_Thread[thid].pEv_group;

	err = sem_wait(&pEV->sem);
	if (ev_mask & pEV->event)
	{
	    *pevent = pEV->event;
		pEV->event &= ~ev_mask;
	}

	if (err == 0)
		return OK;
	else
		return ERROR;
}

/*------------------------------------------------------------------------
 Procedure:     msgSend ID:1
 Purpose:       向线程发送消息
 Input:         thid: 线程号; pbuf: 消息缓冲
                len: 消息大小; send_ev: 是否发送事件
 Output:		  >=0: OK; <0: ERROR
 Errors:
------------------------------------------------------------------------*/
int msgSend(int thid, const void *pbuf, int len, BYTE send_ev)
{
	VLinuxMsg *pMsgQ;
	BYTE *buf;

	/*判断thid的合法性*/
	//if (thid >= THREAD_MAX_NUM) return(ERROR);
	//if (g_Thread[thid].used==0) return(ERROR);
	if (len < 0) return ERROR;
	if (len > Q_MSG_MAX_LEN) return ERROR;

	pMsgQ = (VLinuxMsg *)g_Thread[thid].pQu; 
	if (pMsgQ == NULL) return ERROR;

	buf = pMsgQ->q_Pool+pMsgQ->q_wp*Q_MSG_MAX_LEN;

	memcpy(buf, pbuf, (DWORD)len);

    pMsgQ->q_wp++;
	if (pMsgQ->q_wp == pMsgQ->q_Len) 
		pMsgQ->q_wp = 0;
    
	if(send_ev)  return(evSend(thid, EV_MSG));

	return(OK);
}


/*------------------------------------------------------------------------
 Procedure:     msgReceive ID:1
 Purpose:       从线程接收消息
 Input:         thid: 线程号; pbuf: 消息缓冲
                max_len: 最大消息大小; 
                wait: 等待选项(0:不等待, -1:永久等待, x:等待时间)
 Output:		>=0: 实际接收的字节数; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int msgReceive(int thid, void *pbuf, int max_len, int wait)
{
    int len;
	BYTE *buf;
	VLinuxMsg *pMsgQ;

	if (wait != 0) return ERROR;

	if (max_len < 0) return ERROR;

	len = (max_len < Q_MSG_MAX_LEN) ? max_len : Q_MSG_MAX_LEN;


	pMsgQ = (VLinuxMsg*)g_Thread[thid].pQu;

	if (pMsgQ->q_rp == pMsgQ->q_wp)
		return ERROR;

	buf = pMsgQ->q_Pool+pMsgQ->q_rp*Q_MSG_MAX_LEN;
	memcpy(pbuf, buf, (DWORD)len);
	
	pMsgQ->q_rp++;
	if (pMsgQ->q_rp == pMsgQ->q_Len) 
		pMsgQ->q_rp = 0;

	return len;
}

#if 0
/*------------------------------------------------------------------------
 Procedure:     tmExpire ID:1
 Purpose:       时钟超时处理函数,负责发送事件,且重新启动定时器
 Input:         
 Output:		  
 Errors:
------------------------------------------------------------------------*/
static void tmExpire(union sigval v)
{
   	int thid,i,arg;
	VTimer *ptm;
	struct itimerspec ts; 
	
	arg = (int)v.sival_int;

	thid = arg >> 16;
	i = arg & 0xFFFF;	
	
	ptm = g_Thread[thid].tm+i;

	evSend(thid, ptm->event);		
}

/*------------------------------------------------------------------------
 Procedure:     tmEvEvery ID:1
 Purpose:       为线程创建周期定时器
 Input:         thid: 线程号; intval: 时钟周期
                event: 触发事件; 
 Output:		  >=0: 时钟号; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int tmEvEvery(int thid, DWORD intval, DWORD event)
{	
	int i,arg,intlvl;
	timer_t *TmrId;
	VTimer *ptm;
	struct sigevent evp;
	struct itimerspec ts; 
	char nametmp[64];
	
	if (thid >= THREAD_MAX_NUM) return(ERROR);
	if (g_Thread[thid].used==0) return(ERROR);

	ptm = g_Thread[thid].tm;

	for(i=0; i<TIMER_MAX_NUM; i++)
	{
		if (ptm->used == 0) break;
		ptm++;
	}
	if(i >= TIMER_MAX_NUM)
	{	
		return ERROR;
	}  
	ptm->used = 1;	
	
	arg = (thid<<16)|(i&0xFFFF);  /*高字节thid，低字节时钟索引*/

	sprintf(nametmp, "%s-timer%d", g_Thread[thid].name, i);
	
	TmrId = (timer_t *)malloc(sizeof(timer_t));
	if (TmrId == NULL) return ERROR;

	memset (&evp, 0, sizeof (evp));  
	evp.sigev_value.sival_ptr = TmrId;  
	evp.sigev_notify = SIGEV_THREAD;  
	evp.sigev_notify_function = tmExpire;  
	evp.sigev_value.sival_int = arg; //作为handle()的参数

	if( timer_create(CLOCK_REALTIME, &evp, TmrId) < 0)  
        printf("timer_create error %d \n",errno);
	else
	{	 
		printf("timer_create sucess thid %d \n",thid);
		printf("timer_create sucess \n");
		printf("timer_create sucess \n");
		printf("timer_create sucess \n");
		printf("timer_create sucess \n");
	}

	ts.it_interval.tv_sec = intval/100; 
    ts.it_interval.tv_nsec = (intval%100)*10000;
    ts.it_value.tv_sec = intval/100;  
    ts.it_value.tv_nsec = (intval%100)*10000;
	
	ptm->pid = (void *)TmrId;
	ptm->intval = intval;
	ptm->event = event;

	if(timer_settime(TmrId, TIMER_ABSTIME, &ts, NULL) < 0)
        printf("timer_settime error \n");
	
	return i;
}

/*------------------------------------------------------------------------
 Procedure:     tmEvAfter ID:1
 Purpose:       周期定时后线程触发事件
 Input:         thid: 线程号; intval: 时钟周期
                event: 触发事件; 
 Output:		  >=0: 时钟号; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int tmEvAfter(int thid, DWORD intval, DWORD event)
{	
	int i,arg;
	timer_t *TmrId;
	VTimer *ptm;
	struct sigevent evp;
	struct itimerspec ts; 
	char nametmp[64];
	
	if (thid >= THREAD_MAX_NUM) return(ERROR);
	if (g_Thread[thid].used==0) return(ERROR);

	ptm = g_Thread[thid].tm;

	for(i=0; i<TIMER_MAX_NUM; i++)
	{
		if (ptm->used == 0) break;
		ptm++;
	}
	if(i >= TIMER_MAX_NUM)
	{	
		return ERROR;
	}  
	ptm->used = 1;	
	
	arg = (thid<<16)|(i&0xFFFF);  /*高字节thid，低字节时钟索引*/

	sprintf(nametmp, "%s-timer%d", g_Thread[thid].name, i);
	
	TmrId = (timer_t *)malloc(sizeof(timer_t));
	if (TmrId == NULL) return ERROR;

	memset (&evp, 0, sizeof (evp));  
	evp.sigev_value.sival_ptr = TmrId;  
	evp.sigev_notify = SIGEV_THREAD;  
	evp.sigev_notify_function = tmExpire;  
	evp.sigev_value.sival_int = thid; //作为handle()的参数

	if( timer_create(CLOCK_REALTIME, &evp, TmrId) <= 0)  
        printf("timer_create error \n");

	ts.it_interval.tv_sec = intval/100; 
    ts.it_interval.tv_nsec = (intval%100)*10000;
    ts.it_value.tv_sec = intval/100;  
    ts.it_value.tv_nsec = (intval%100)*10000;
	
	ptm->pid = (void *)TmrId;
	ptm->intval = intval;
	ptm->event = event;

	if(timer_settime(*TmrId, TIMER_ABSTIME, &ts, NULL) <= 0)
        printf("timer_settime error \n");
	
	return i;

}

/*------------------------------------------------------------------------
 Procedure:     tmCancel ID:1
 Purpose:       取消线程周期定时器
 Input:         thid: 线程号; tmno:定时器号
 Output:		>=0: OK; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int tmCancel(int thid, int tmno)
{	
	int intlvl;
	VTimer *ptm;

	if (thid >= THREAD_MAX_NUM) return(ERROR);
	if (g_Thread[thid].used==0) return(ERROR); 

	if (tmno >= TIMER_MAX_NUM) return(ERROR);
	else if (tmno < 0) return(ERROR);

	ptm = g_Thread[thid].tm+tmno;

    timer_delete(*((timer_t *)g_Thread[thid].tm[tmno].pid));
	free(g_Thread[thid].tm[tmno].pid);

	ptm->used = 0;
	ptm->pid = NULL;	

    return OK;
}  

/*------------------------------------------------------------------------
 Procedure:     tmDelete ID:1
 Purpose:       取消线程周期定时器
 Input:         thid: 线程号; tmno:定时器号
 Output:		>=0: OK; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int tmDelete(int thid, int tmno)
{	
	int intlvl;
	VTimer *ptm;

	if (thid >= THREAD_MAX_NUM) return(ERROR);
	if (g_Thread[thid].used==0) return(ERROR); 

	if (tmno >= TIMER_MAX_NUM) return(ERROR);
	else if (tmno < 0) return(ERROR);

	ptm = g_Thread[thid].tm+tmno;

    timer_delete(*((timer_t *)g_Thread[thid].tm[tmno].pid));
	free(g_Thread[thid].tm[tmno].pid);

	ptm->used = 0;
	ptm->pid = NULL;	

    return OK;
}  
#endif

#if 1
/*------------------------------------------------------------------------
 Procedure:     tmExpire ID:1
 Purpose:       时钟超时处理函数,负责发送事件,且重新启动定时器
 Input:         
 Output:		  
 Errors:
------------------------------------------------------------------------*/
static void tmExpire(union sigval v)
{
    int ret;
    if (v.sival_ptr == NULL) return;
	
    ret = evSend(g_timer.thid, g_timer.event);	
	if (ret) 
		printf("evsend timer error\n");
}

/*------------------------------------------------------------------------
 Procedure:     tmEvEvery ID:1
 Purpose:       为线程创建周期定时器
 Input:         thid: 线程号; intval: 时钟周期
                event: 触发事件; 
 Output:		  >=0: 时钟号; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int tmEvEverySet(int thid, DWORD intval, DWORD event)
{	
	struct sigevent evp;  
    struct itimerspec ts;  
    timer_t timer; 

	if (thid >= THREAD_MAX_NUM) return(ERROR);
	if (g_timer.used == 1) return(ERROR);
	g_timer.used = 1;	
	
	 memset (&evp, 0, sizeof (evp));  
	evp.sigev_value.sival_ptr = &timer;  
	evp.sigev_notify = SIGEV_THREAD;  
	evp.sigev_notify_function = tmExpire;  
	evp.sigev_value.sival_int = thid; //作为handle()的参数

    if( timer_create(CLOCK_MONOTONIC, &evp, &timer) < 0)  
        printf("timer_create error \n");

	ts.it_interval.tv_sec = (long)(intval/100); 
    ts.it_interval.tv_nsec = (intval%100)*10000;
    ts.it_value.tv_sec = (long)(intval/100);  
    ts.it_value.tv_nsec = (intval%100)*10000;

	if(timer_settime(timer, TIMER_ABSTIME, &ts, NULL) < 0)
        printf("timer_settime error \n");

	if(timer_gettime(timer, &ts) < 0)
        printf("timer_settime error \n");

	printf("timer_gettime %d %d %d %d  \n",ts.it_interval.tv_sec,ts.it_interval.tv_nsec,ts.it_value.tv_sec,ts.it_value.tv_nsec);
	
	g_timer.event = event;
	g_timer.thid = thid;
	g_timer.intval = intval;

	return OK;
}

/*------------------------------------------------------------------------
 Procedure:     tmEvEvery ID:1
 Purpose:       为线程创建周期定时器
 Input:         thid: 线程号; intval: 时钟周期
                event: 触发事件; 
 Output:		  >=0: 时钟号; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int tmEvEvery(int thid, DWORD intval, DWORD event)
{	
	int i;
	VTimer *ptm;
	
	if (thid >= THREAD_MAX_NUM) return(ERROR);
	if (g_Thread[thid].used==0) return(ERROR);

	ptm = g_Thread[thid].tm;

	for(i=0; i<TIMER_MAX_NUM; i++)
	{
		if (ptm->used == 0) break;
		ptm++;
	}
	if(i >= TIMER_MAX_NUM)
	{	
		return ERROR;
	}  
	
	ptm->ticks = intval;
	ptm->intval = intval;
	ptm->event = event;
	ptm->thid = thid;

	ptm->used = 1;	
	return i;
}

/*------------------------------------------------------------------------
 Procedure:     tmEvAfter ID:1
 Purpose:       周期定时后线程触发事件
 Input:         thid: 线程号; intval: 时钟周期
                event: 触发事件; 
 Output:		  >=0: 时钟号; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int tmEvAfter(int thid, DWORD intval, DWORD event)
{	
	int i;
	VTimer *ptm;
	if (thid >= THREAD_MAX_NUM) return(ERROR);
	if (g_Thread[thid].used==0) return(ERROR);

	ptm = g_Thread[thid].tm;

	for(i=0; i<TIMER_MAX_NUM; i++)
	{
		if (ptm->used == 0) break;
		ptm++;
	}
	if(i >= TIMER_MAX_NUM)
	{	
		return ERROR;
	}  
	
	ptm->ticks = intval;
	ptm->intval = intval;
	ptm->event = event;
	ptm->thid = thid;

	ptm->used = 1;	

	return i;
}

/*------------------------------------------------------------------------
 Procedure:     tmCancel ID:1
 Purpose:       取消线程周期定时器
 Input:         thid: 线程号; tmno:定时器号
 Output:		>=0: OK; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int tmCancel(int thid, int tmno)
{	
	if (thid >= THREAD_MAX_NUM) return(ERROR);
	if (g_Thread[thid].used==0) return(ERROR); 

	if (tmno >= TIMER_MAX_NUM) return(ERROR);
	else if (tmno < 0) return(ERROR);

	g_Thread[thid].tm[tmno].used = 0;
	g_Thread[thid].tm[tmno].ticks = 0xffffffff;	
	return OK;
}  

/*------------------------------------------------------------------------
 Procedure:     tmDelete ID:1
 Purpose:       取消线程周期定时器
 Input:         thid: 线程号; tmno:定时器号
 Output:		>=0: OK; -1: ERROR
 Errors:
------------------------------------------------------------------------*/
int tmDelete(int thid, int tmno)
{	
	VTimer *ptm;
	if (thid >= THREAD_MAX_NUM) return(ERROR);
	if (g_Thread[thid].used==0) return(ERROR); 

	if (tmno >= TIMER_MAX_NUM) return(ERROR);
	else if (tmno < 0) return(ERROR);

	ptm = g_Thread[thid].tm+tmno;

	ptm->used = 0;
	ptm->ticks = 0xffffffff;	

    return OK;

}  
#endif

void timeraddMS(struct timeval *a, int ms) 
{ 
   a->tv_usec += ms * 1000; 
   if (a->tv_usec >= 1000000) 
   { 
       a->tv_sec += a->tv_usec / 1000000; 
       a->tv_usec %= 1000000; 
   } 
} 


DWORD smBCreate(void)
{
   int ret;
   sem_t *sem;

   sem = (sem_t*)malloc(sizeof(sem_t));
   
   if (sem == NULL)
   	   return 0;
   ret = sem_init(sem, 0, 0);
   if (ret)
   {
       free(sem);
       return 0;
   }

   return (DWORD)sem;
}

DWORD smMCreate(void)
{
   int ret;
   sem_t *sem;

   sem = (sem_t*)malloc(sizeof(sem_t));   
   if(sem == NULL)
   	  return 0;
   
   ret = sem_init(sem, 0, 1);
   if (ret)
   {
       free(sem);
       return 0;
   }

   return (DWORD)sem;
   
}

int smMTake(DWORD smid)
{
	int err;

	if (smid == 0)
		return ERROR;

	err = sem_wait((sem_t*)smid);

	if (err == 0)
		return OK;
	else
		return ERROR;
}

int smMGive(DWORD smid)
{
	int err;

	if (smid == 0)
		return ERROR;

	err = sem_post((sem_t*)smid);

	if (err == 0)
		return OK;
	else
		return ERROR;
}


int smMTake_tm(DWORD smid, int wait)
{
	int ret;
    struct timeval now; 
    struct timespec outtime; 

	ret = gettimeofday(&now, NULL);
	if (ret)
		return ERROR;

	timeraddMS(&now, wait*10);

	if (smid == 0)
		return ERROR;

	outtime.tv_sec = now.tv_sec; 
    outtime.tv_nsec = now.tv_usec * 1000; 
	ret = sem_timedwait((sem_t*)smid,&outtime); 

	if (ret == 0)
		return OK;
	else
		return ERROR;
}

int smBTake(DWORD smid)
{
	int err;

	if (smid == 0)
		return ERROR;

	err = sem_wait((sem_t*)smid);

	if (err == 0)
		return OK;
	else
		return ERROR;
}

/*wait按照tick,10ms*/
int smBTake_tm(DWORD smid, int wait)
{
    int ret;
    struct timeval now; 
    struct timespec outtime; 

	ret = gettimeofday(&now, NULL);
	if (ret)
		return ERROR;

	timeraddMS(&now, wait*10);

	if (smid == 0)
		return ERROR;

	outtime.tv_sec = now.tv_sec; 
    outtime.tv_nsec = now.tv_usec * 1000; 
	ret = sem_timedwait((sem_t*)smid,&outtime); 

	if (ret == 0)
		return OK;
	else
		return ERROR;
}

unsigned long Get100usCnt(void)
{
	return (g_PthreadTicks*100) ;
}
