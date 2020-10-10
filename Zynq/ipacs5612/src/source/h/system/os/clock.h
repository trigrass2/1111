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

#define SYSCLOCK    0
#define CALCLOCK    1
#define UTCCLOCK    2

#pragma pack(1)
typedef struct VSysClock{
  WORD wYear;    /*19XX-20XX*/
  BYTE byMonth;  /*1-12*/
  BYTE byDay;    /*1-31*/
  BYTE byWeek;   /*0-6  from sunday*/
  BYTE byHour;   /*0-23*/
  BYTE byMinute; /*0-59*/
  BYTE bySecond; /*0-59*/
  WORD wMSecond; /*0-999*/
}VSysClock; 

typedef struct VCalClock{
  DWORD dwMinute;
  WORD wMSecond;
} VCalClock;

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

void ClockInit(void);
STATUS ClockIsOk(const struct VSysClock *p);
DWORD CalendarClock(struct VSysClock *pTime);
void  SystemClock(DWORD i, struct VSysClock *pTime);
void GetSysClock(void *pBuf,int nFlag);
void SetSysClock(const void *pBuf,int nFlag);
void ToCalClock(struct VSysClock *pBuf, struct VCalClock *pTime);
void CalClockTo(const struct VCalClock *pBuf, struct VSysClock *pTime);
void GetTime(struct VCalClock *pTime);
void SetTime(struct VCalClock tm);

#ifdef __cplusplus
}
#endif

#endif

