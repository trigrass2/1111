/*------------------------------------------------------------------------
 Module:       	main.cpp
 Author:        helen
 Project:       
 State:			
 Creation Date:	2019-05-28
 Description:  for config app 
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sys.h"
#include "protocol.h"
#include "fileext.h"
#include "yx.h"
#include "tcp.h"
#include  "dbfrz.h"
#ifdef INCLUDE_HIS_TTU
#include "dbhis_ttu.h"
#endif
#include "i2c.h"

extern void database(int thid);
extern void misc(void);
extern void comm(void);
#ifdef INCLUDE_PB_RELATE
extern void pb(int thid);
#endif
extern int recordInit(WORD wTaskID);
extern void mmi(void);
 
void myclock(void);

static struct VThSetupInfo ThSetupInfo[] = 
{
	{CLOCK_ID,      	"tClock",    	1,  					COMMONSTACKSIZE,      0,	(ENTRYPTR)myclock,		},
	{COMM_ID,       	"tComm",     	(USR_TASK_PRIO+3),		COMMONSTACKSIZE,      0,	(ENTRYPTR)comm,			},
	{COMM_CLIENT_ID,	"tClient",	 	(USR_TASK_PRIO+4),		CLIENTSTACKSIZE,	  0,	(ENTRYPTR)Client,		},
	{COMM_SERVER_ID,	"tServer",		(USR_TASK_PRIO+5),		SERVERSTACKSIZE,	  0,	(ENTRYPTR)Server,		},
	{COMM_UDP_SERVER_ID,"tUdpServer",	(USR_TASK_PRIO+7),      SERVERSTACKSIZE,	  0,	(ENTRYPTR)Udp_Server,   },
	{DB_ID,             "tDb",          (USR_TASK_PRIO+6),      COMMONSTACKSIZE,      10,   (ENTRYPTR)database,  },
	{SELF_DIO_ID,       "tDio",         (USR_TASK_PRIO+0),      COMMONSTACKSIZE,      10,   (ENTRYPTR)dio,       },
	{SELF_MEA_ID,       "tMea",         (USR_TASK_PRIO+14),     (COMMONSTACKSIZE*2),  0,    (ENTRYPTR)mea,       },
	{MISC_ID,		    "tMisc",	    (USR_TASK_PRIO+46),     COMMONSTACKSIZE,	   0,	(ENTRYPTR)misc, 	 },
	{MMI_ID,		    "tMmi", 	    (USR_TASK_PRIO+15),     COMMONSTACKSIZE,	 COMMONQLEN,   (ENTRYPTR)mmi,   },
#ifdef INCLUDE_PB_RELATE
	{PB_ID,         	"tPb",			(USR_TASK_PRIO+45),		COMMONSTACKSIZE,      0,	(ENTRYPTR)pb			},
#endif
#ifdef INCLUDE_B2F
	{B2F_ID,		    "tB2F", 	    (USR_TASK_PRIO+46),     (COMMONSTACKSIZE*2),  5,    (ENTRYPTR)buf2file,  }, 
#endif

#ifdef INCLUDE_HIS_TTU
	{HIS_ID, 			"tDbhis",		(USR_TASK_PRIO+46),		COMMONSTACKSIZE, 	  0,	(ENTRYPTR)histtu2file	},
#endif
#ifdef INCLUDE_FRZ
	{FRZ_ID,     		"freeze",		(USR_TASK_PRIO+8),		COMMONSTACKSIZE,      10,	(ENTRYPTR)freeze,  		},
#endif
	{-1,            	"",          	0,                   	0,                    0,	NULL,            		},
};

void myclock(void)
{
	int thid,i;
	DWORD events;
	VTimer *ptm;
	extern VThread *g_Thread;
	
	//定时器启动，最小10ms，所有线程定期器均由此触发
	//tmEvEverySet(CLOCK_ID, 1, EV_TM1);  //pre 10ms 
	
	for(;;) 
	{	
		//evReceive(CLOCK_ID, EV_TM1,&events); 	
		//if(events & EV_TM1)
		{
		    g_PthreadTicks++;
			/*if(g_PthreadTicks > 1000)
			{
				g_PthreadTicks = 0;
				printf("tmEvEverySet 100s \n");
			}*/
			for (thid=CLOCK_ID+1; thid<THREAD_MAX_NUM; thid++)
			{
				if(g_Thread[thid].active)
				{
					ptm = g_Thread[thid].tm;
					for(i = 0;i < TIMER_MAX_NUM;i++)
					{
						if (ptm->used == 0) break;
						ptm->ticks--;
						if(ptm->ticks == 0)
						{
							ptm->ticks = ptm->intval;
							evSend(ptm->thid,ptm->event);
						}					
						ptm++;
					}
				}
			}
		}
		usleep(10000);
	}
}

void ApplicationInit()
{
	I2CInit(0);

	PublicInit();
	
	NVRamInit();

	ClockInit();
		
	SpecialTaskSetup(ThSetupInfo,CLOCK_ID);
	
	SysParaInit();

#ifdef INCLUDE_B2F
		buf2fileInit();
#endif

	PubPollAndTableSetup();
	
	SelfPara_Init1();

	ResetShow();
	
	AddrInit();
	
	commInit();
	
#ifdef INCLUDE_NET
    SpecialTaskSetup(ThSetupInfo,COMM_CLIENT_ID);
    SpecialTaskSetup(ThSetupInfo,COMM_SERVER_ID);
    SpecialTaskSetup(ThSetupInfo,COMM_UDP_SERVER_ID);
#endif	

	SpecialTaskSetup(ThSetupInfo,COMM_ID);

	SysConfigInit();

	SetIoNo();
	
    //初始化实设备后到database注册,同时注册遥测遥信遥控信息。其他app都只读即可
	if (GetDevInfo()==OK)
	{
		myprintf(SYS_ID, LOG_ATTR_INFO, "Device.cfg init ok.");
		myprintf(SYS_ID, LOG_ATTR_INFO, "System total device num is %d.",*g_Sys.Eqp.pwNum);
		DevInit();
	}
	else
		myprintf(SYS_ID, LOG_ATTR_INFO, "Device.cfg invalid or not exist!");

	ioInit();

#ifdef INCLUDE_RECORD	
	recordInit(B2F_ID);
#endif	

	SpecialTaskSetup(ThSetupInfo,DB_ID);
	SpecialTaskSetup(ThSetupInfo,SELF_DIO_ID);
	SpecialTaskSetup(ThSetupInfo,SELF_MEA_ID);
	SpecialTaskSetup(ThSetupInfo,MISC_ID);	

#ifdef INCLUDE_PB_RELATE
	PbInit();
	SpecialTaskSetup(ThSetupInfo,PB_ID);
#endif  	

	SpecialTaskSetup(ThSetupInfo,MMI_ID);
	TaskSetup();  

#ifdef INCLUDE_HIS_TTU
	his_ttu_init();
	SpecialTaskSetup(ThSetupInfo,HIS_ID);
#endif

#ifdef INCLUDE_HIS
// xml 文件初始化
	his_init();
#endif

#ifdef INCLUDE_B2F
	SpecialTaskSetup(ThSetupInfo,B2F_ID);
#endif	

	SelfPara_Init();	

}

extern void PowerOn_Init_McInnerComm(void);
extern void PowerOn_Init_SharedMemory(void);
extern  void BMinit(void);
extern void BMWritePrSetTable(void);
extern void mc_Print_from_BM(void);
extern void BMNvramToLinux(void);
extern void LinuxNvramToBM(void);
extern void SetBMTime(void);

int main(void)
{
	PowerOn_Init_SharedMemory();
	PowerOn_Init_McInnerComm();
	
	ThreadInit("jc");
	ApplicationInit();
	prGzEventInit();
	//铁电linux共享给裸核
	LinuxNvramToBM();
	BMinit();
	
	for(;;)
	{
		sleep(1);
		BMWritePrSetTable();
		mc_Print_from_BM();
		
		//铁电裸核共享给linux
		BMNvramToLinux();
		
		//上电做几次对时裸核，当裸核起来后
		SetBMTime();
	}
}
