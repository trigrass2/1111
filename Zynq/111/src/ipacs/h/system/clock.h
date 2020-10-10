/*------------------------------------------------------------------------
 Module:		clock.h
 Author:		solar
 Project:		 
 Creation Date: 2008-9-1
 Description:	
------------------------------------------------------------------------*/
#ifndef _CLOCK_H
#define _CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#define TICK2MSE			10
#define SECTOTICK(sec)     (sec*1000/TICK2MSE)

#define TIMER_TICKS_PER_SECOND	2000  //1000   

extern DWORD gUsCnt;
static __inline DWORD Get100usCnt(void)
{
	return gUsCnt*5;	
}

#define SYSCLOCK    0
#define CALCLOCK    1
#define UTCCLOCK    2

#pragma pack(1)
struct VSysClock{
  WORD wYear;    /*19XX-20XX*/
  BYTE byMonth;  /*1-12*/
  BYTE byDay;    /*1-31*/
  BYTE byWeek;   /*0-6  from sunday*/
  BYTE byHour;   /*0-23*/
  BYTE byMinute; /*0-59*/
  BYTE bySecond; /*0-59*/
  WORD wMSecond; /*0-999*/
}; 

struct VCalClock{
  DWORD dwMinute;
  WORD wMSecond;
};

struct VUtcClock{
	DWORD dwSec;      	/* Number of seconds since January 1, 1970	*/
	DWORD dwFraction;  	/* Fraction of a second				*/
	DWORD dwQ;  	    /* Quality flags, 8 least-significant bits only	*/
};

struct VTimeSpec{
	DWORD tv_sec;
	DWORD tv_nsec;
};
#pragma pack()

int rtcInit(void);
void rtcGetTime(struct VCalClock *pTime);
void rtcSetTime(struct VCalClock *pTime);

void ClockInit(void);
DWORD CalendarClock(struct VSysClock *pTime);
void  SystemClock(DWORD i, struct VSysClock *pTime);
void GetSysClock(void *pBuf,int nFlag);
void SetSysClock(void *pBuf,int nFlag);
void ToCalClock(struct VSysClock *pBuf, struct VCalClock *pTime);
void CalClockTo(struct VCalClock *pBuf, struct VSysClock *pTime);
void sys_get_time(BYTE *ptime);
STATUS ClockIsOk(struct VSysClock *p);
void mywatchdog(void);

#ifdef __cplusplus
}
#endif

#endif

