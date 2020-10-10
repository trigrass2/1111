#include "os.h"

VTask g_Task[MAX_TASK_NUM];

void task_null(void)
{

}

void tInit(void)
{
    int i;
	
    memset(g_Task,0,sizeof(g_Task));
    for (i=0; i<MAX_TASK_NUM; i++)
		g_Task[i].func = (ENTRYPTR)task_null;
}

int tCreate(int tid, void *targs, ENTRYPTR tfunc)
{	
	g_Task[tid].args = targs;
	g_Task[tid].func = tfunc;

	return tid;
}

void OSTimeDly(INT16U ticks);
//10ms 一个tick
void tSleep(DWORD ticks)
{
	DWORD  ms;

	ms = ticks*TICK2MSE;
	OSTimeDly(ms);
}

void evSend(int tid, DWORD event)
{
    g_Task[tid].event |= event;
}

void evReceive(int tid, DWORD ev_mask, DWORD *pevent)
{
    DWORD eventMatch;

	eventMatch = g_Task[tid].event & ev_mask;    
	g_Task[tid].event &= ~eventMatch;
	*pevent = eventMatch;
}

void tmEvEvery(int tid, DWORD intval)
{
    g_Task[tid].tmcnt = intval;
	g_Task[tid].tmlimit= intval;
}

void tmEvCancle(int tid)
{
	g_Task[tid].tmcnt = 0;
    g_Task[tid].event &= ~EV_TM1;
}

//run 10ms 进一次 放到用户任务
void tRunCnt(void)
{
    int i;
	for (i=0; i<MAX_TASK_NUM; i++)
	{	
		if(g_Task[i].tmcnt)
		{
			g_Task[i].tmcnt--;
			if(g_Task[i].tmcnt == 0)
			{
				g_Task[i].event |= EV_TM1;
				g_Task[i].tmcnt = g_Task[i].tmlimit;
			}
		}	
	}
}

void tRun(void)
{
    int i;
	for (i=0; i<MAX_TASK_NUM; i++)
	{	
		if(g_Task[i].event)		(*(g_Task[i].func))(g_Task[i].args);		
	}
}

