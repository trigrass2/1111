/*------------------------------------------------------------------------
 Module:		os.h
 Author:		solar
 Project:		 
 Creation Date: 2008-8-1
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
------------------------------------------------------------------------*/

#ifndef _OS_H
#define _OS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "syscfg.h"

#define THREAD_MIN_NUM      64        /*�̶�����*/

extern int COMM_NUM, THREAD_MAX_NUM;


#define SYS_ID  0

#define TIMER_MAX_NUM		3		  /*ÿ�̵߳Ŀ��е�ʱ����Ŀ*/

#define TICK2MSE			10

#define SCAN_TM             1 /*20ms*/

#define SECTOTICK(sec)     (sec*1000/TICK2MSE)

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

#define EV_ISR_YX       0x00000001
#define EV_ISR_PR       0x00000002
#define EV_ISR_RCD      0x00000004
#define EV_ISR_GOOSE    0x00000008
#define EV_ISR_MASK     0x0000000F

#define EV_RCD      0x00000008
#define EV_CLEAR_SOE    0x00000004

#define OS_NOWAIT                0
#define OS_ALWAYSWAIT           -1
#define OS_WAITTIME           6000 


#define MSG_REC      10
#define MSG_RECORD   20
#define MSG_FILE     25

/*------------------------------------------------------------------------
 OS�����˽�ж���ṹ����
------------------------------------------------------------------------*/
typedef struct {
	void*   pid;		/*ʱ��ID*/
	DWORD	event;		/*ʱ���¼�*/
	DWORD 	intval;		/*ʱ�Ӷ�ʱ����*/
#if (TYPE_OS == OS_LINUX)
	DWORD   ticks;       /*ʱ�Ӽ�ʱ*/
#endif
	int		thid;		/*�����̺߳�*/
	char	tmno;		/*�����߳������*/
	char	used;
}VTimer;

#define TH_NAME_LEN     16

typedef struct {
	int 	used;
	char	name[TH_NAME_LEN];
	void*	pTcb; 		/*TCBָ��*/
	void*	pEv_group;	/*Nu: ����¼����־*/
	void*	pQu;		/*Nu: �����Ϣ���б�־*/
	VTimer	tm[TIMER_MAX_NUM];	/*Nu: ���̵߳�ϵ��ʱ��*/

	int     active;	
	int     commno;
	int     watchdog;
}VThread;

#ifndef SYS_LOG_MSGLEN
#define SYS_LOG_MSGLEN    128
#endif
#ifndef SYS_LOG_BUFNUM
#define SYS_LOG_BUFNUM    128
#endif

#define LOG_ATTR_INFO     0
#define LOG_ATTR_WARN     1
#define LOG_ATTR_ERROR    2

struct VLogMsg{
    int thid;
	int attr;
    char sMsg[SYS_LOG_MSGLEN];
};
typedef struct{
    WORD wWrtPtr;
    struct VLogMsg aLogMsg[SYS_LOG_BUFNUM];
    DWORD dwSem;
}VLog; 

extern VLog sysLog;  
int myprintf(int thid, int attr, const char *fmt, ... );
int runlog(void);
int errlog(void);

extern WORD crccode[];
extern DWORD crctable[];

extern char sysFileName[];
extern char sysSp[30];
extern char sysSVer[30];
extern char sysHVer[30];
extern char sysUser[30];
extern DWORD sysCrc;
STATUS findSysFileSp(char *path, char *sp);
STATUS findSysFileVer(char *path, char *ver);
STATUS GetSvnVer(char *svnverS, char *svnver);
STATUS findSysFileUser(char *path, char *user);
DWORD findSysFileCrc(char *path);
DWORD calcSysFileCrc(char *path);
DWORD findSysFileCrcInMem(char *buf, DWORD len);
DWORD calcSysFileCrcInMem(char *buf, DWORD len);
void sysVerCrcShow(int thid);

int myTaskSpawn(char *name, ENTRYPTR entryPtr, int arg, int stack_size, int priority);

int thCreate(const char *name, ENTRYPTR entryPtr, int arg, int stack_size, int priority, int queue_len);
int thStart(int thid, int dogtm);
void DiasbleThDog(void);
void EnableThDog(void);
int thDisableDog(int thid);
int thClearDog(int thid, int dogtm);
int thRunDog(int thid, int interval);
int GetThActive(int thid);
int thPrioritySet(int thid,int newPriority);
int thSleep(DWORD ticks);
int thSuspend(int thid);
int thResume(int thid);
int thDelete(int thid);
BOOL thIsSuspended (int thid);
int smMTake(DWORD smid);
int smMGive(DWORD smid);
DWORD smMCreate(void);
int evSend(int thid, DWORD event);
int evReceive(int thid, DWORD ev_mask, DWORD *pevent);
int msgSend(int thid, const void *pbuf, int len, BYTE send_ev);
int msgReceive(int thid, void *pbuf, int max_len, int wait);
int tmEvEvery(int thid, DWORD intval, DWORD event);
int tmEvEverySet(int thid, DWORD intval, DWORD event);
int tmEvAfter(int thid, DWORD intval, DWORD event);
int tmCancel(int thid, int tmno);
int tmDelete(int thid, int tmno);
int ThreadInit(const char* name);
void *ThreadPTcb(int thid);
int ThreadReq(int thid, const char *name);
int ThNameToId(const char *thname);
int ThKeyNameToId(const char *thname);
char *GetThName(int thid);
int GetThCommNo(int thid);
void ThreadCommNoSet(int thid, int commno);
unsigned long Get100usCnt(void);

void GetStrFromMyFormatStr(char *str, int ch, int num, ...);
void  shellprintf(const char *fmt, ... );
#define logMsg(X0, X1, X2, X3, X4, X5, X6) 	shellprintf(X0, X1, X2, X3, X4, X5, X6)

#ifdef __cplusplus
}
#endif

#endif /*_OS_H*/
