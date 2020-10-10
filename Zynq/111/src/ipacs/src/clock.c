/*------------------------------------------------------------------------
 Module:		clock.c
 Author:		solar
 Project:		 
 Creation Date: 2007-3-14
 Description:	
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef INCLUDE_TIMER

#include <stdlib.h>
#include <string.h>
#include "sys.h"
#include "clock.h" 

extern int g_init;

#define WATCHDOG_INTERVAL  25  /*500ms*//*50*/

static DWORD montomin[12]={44640,28,44640,43200,44640,43200,44640,44640,43200,44640,43200,44640};

/*------------------------------------------------------------------------
 Procedure:     CalendarClock ID:1
 Purpose:       转换为日历时钟
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/    
DWORD CalendarClock(struct VSysClock *pTime)
{
	DWORD i=0,t;

	if((pTime->wYear & 0x03)==0)
	    montomin[1]=41760;
	else
	    montomin[1]=40320;

	if(pTime->wYear>11000)  pTime->wYear=11000;

	for(t=1970;t<pTime->wYear;t++)
	{
    	if((t & 0x03)==0)
	    	i+=527040;//366*24*60
    	else
	    	i+=525600;//365*24*60
	}

	for(t=1;t<pTime->byMonth;t++)
    	i+=montomin[t-1];//24*60;
	i+=(pTime->byDay-1)*1440;//24*60;
	i+=pTime->byHour*60;//60;
	i+=pTime->byMinute;

	return(i);	
}

/*------------------------------------------------------------------------
 Procedure:     SystemClock ID:1
 Purpose:       转换为系统时钟
 Input:
 Output:		
 Errors:
------------------------------------------------------------------------*/    
void  SystemClock(DWORD i, struct VSysClock *pTime)
{
	DWORD	j,t; 

	j=i/1440+1;   /*总天数*/
	j%=7;
	j+=3; /*1970.1.1 星期4*/
	if (j>7) j-=7;
	pTime->byWeek=j;

	for(t=1970;t<11000;t++)
	{
    	if((t & 0x03)==0)
	        j=527040;			//366*24*60
	    else
	        j=525600;      //365*24*60
	    if(i>=j)
	        i-=j;
     	else
    	{
    	    pTime->wYear=(unsigned short)t;
    	    break;
     	}
	}

	if((pTime->wYear & 0x03)==0)
    	montomin[1]=41760;
	else
	    montomin[1]=40320;
	for(t=0;t<12;t++)
	{
	    if(i>=montomin[t]) //24*60;
    	    i-=montomin[t];
    	else
    	{
     	    pTime->byMonth=(BYTE)(t+1);
     	    break;
	    }
	}

	pTime->byDay=(unsigned char)(i/1440+1);
	i=i%1440;

	pTime->byHour=(unsigned char)(i/60);
	pTime->byMinute=(unsigned char)(i%60);	
}

/*------------------------------------------------------------------------
 Procedure:     GetSysClock ID:1
 Purpose:       取系统时钟
 Input:         pBuf:时钟buf  nFlag:时钟格式
 Output:		
 Errors:
------------------------------------------------------------------------*/    
void GetSysClock(void *pBuf, int nFlag)
{
	struct VCalClock tm;

    if (nFlag == CALCLOCK)
		rtcGetTime(pBuf);
	else 
	{
		rtcGetTime(&tm);
		CalClockTo(&tm,(struct VSysClock*)pBuf);  // 该函数使pBuf获得转换后的系统时间格式
	}
} 

/*------------------------------------------------------------------------
 Procedure:     SetSysClock ID:1
 Purpose:       设系统时钟
 Input:         pBuf:时钟buf  nFlag:时钟格式
 Output:		
 Errors:
------------------------------------------------------------------------*/    
void SetSysClock(void *pBuf, int nFlag)
{
	struct VCalClock tm;

	switch(nFlag)
	{
	    case SYSCLOCK:
    	    ToCalClock((struct VSysClock *)pBuf, &tm);
	        break;
    	case CALCLOCK:      
      	    memcpy(&tm,pBuf,sizeof(struct VCalClock));
      	    break;
	} 	   

	rtcSetTime(&tm);

} 

/*------------------------------------------------------------------------
 Procedure:     ToCalClock ID:1
 Purpose:       转换为日历时钟
 Input:         pBuf:输入的时钟buf 
                pTime:输出的时钟 
 Output:		
 Errors:
------------------------------------------------------------------------*/
void ToCalClock(struct VSysClock *pBuf, struct VCalClock *pTime)
{
    pTime->dwMinute=CalendarClock(pBuf);
    pTime->wMSecond=pBuf->bySecond*1000+pBuf->wMSecond;
} 

/*------------------------------------------------------------------------
 Procedure:     CalClockTo ID:1
 Purpose:       设系统时钟
 Input:         pBuf:输入的时钟buf 
                pTime:输出的时钟 
                nFlag:时钟格式
 Output:		
 Errors:
------------------------------------------------------------------------*/
void CalClockTo(struct VCalClock *pBuf, struct VSysClock *pTime)
{
    SystemClock(pBuf->dwMinute,pTime);
    pTime->bySecond = pBuf->wMSecond/1000;
    pTime->wMSecond = pBuf->wMSecond%1000;
} 

STATUS ClockIsOk(struct VSysClock *p)
{
	if ((p->wYear<2000) || (p->wYear > 2100)) return(ERROR);
	if ((p->byMonth<1) || (p->byMonth>12)) return(ERROR);
	if ((p->byDay<1) || (p->byDay>31)) return(ERROR);
	if (p->byHour>23) return(ERROR);
	if (p->byMinute>59) return(ERROR);
	if (p->bySecond>59) return(ERROR);
	if (p->wMSecond>999) return(ERROR);

	if (p->byMonth == 2)
    {
		if ((p->wYear&0x03) == 0)//闰年
		{
			if (p->byDay > 29)
				return (ERROR);
		}
		else
		{
			if (p->byDay > 28)
				return (ERROR);
		}
    }
	else if ((p->byMonth == 4) || (p->byMonth == 6) || (p->byMonth == 9) || (p->byMonth == 11))
	{
		if( p->byDay > 30 )
			return (ERROR);
	}
	
	return(OK);
}

void sys_get_time(BYTE *ptime)
{
	struct VCalClock tm;
	struct VSysClock tSys;

	rtcGetTime(&tm);
	CalClockTo(&tm,&tSys);  // 该函数使pBuf获得转换后的系统时间格式

	*ptime++=tm.wMSecond;
	*ptime++=tm.wMSecond>>8;
	*ptime++=tSys.byMinute;
	*ptime++=tSys.byHour;
	*ptime++=tSys.byDay;
	*ptime++=tSys.byMonth;
	*ptime++=tSys.wYear>2000?(tSys.wYear-2000):(tSys.wYear-1900);
}

/*------------------------------------------------------------------------
 Procedure:     CheckAndVerify ID:1
 Purpose:       检查和校验系统时钟
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/    

void CheckAndVerify(struct VSysClock *pTime)
{
	if (ClockIsOk(pTime)==ERROR)
	{
		pTime->wYear=2008;
		pTime->byMonth=8;
		pTime->byDay=8;
		pTime->byHour=20;
		pTime->byMinute=8;
		pTime->bySecond=8;
		pTime->wMSecond=0;
		SetSysClock(pTime,SYSCLOCK);
	} 
} 	

/*------------------------------------------------------------------------
 Procedure:     ClockInit ID:1
 Purpose:       系统时钟初始化
 Input:         
 Output:		
 Errors:
------------------------------------------------------------------------*/
void ClockInit(void)
{
	struct VSysClock tm;

	if (rtcInit() == -1)
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Hardware clock Wrong!");
	}

	GetSysClock(&tm,SYSCLOCK);
	CheckAndVerify(&tm);
}  


DWORD GetSecond(void)
{
    DWORD second;
    struct VCalClock tm;

	rtcGetTime(&tm);
    second = tm.dwMinute*60+tm.wMSecond/1000;
	return second;
}

#endif


