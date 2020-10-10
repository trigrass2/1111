/*------------------------------------------------------------------------
 Module:        Irigb.h
 Author:         lvyi
 Project:       
 Creation Date:	2018-10-11
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/
#ifndef _IRIGB_H
#define _IRIGB_H
#include "mytypes.h"

struct TSysTime{
  WORD Year;    /*19XX-20XX*/
  BYTE Month;  /*1-12*/
  BYTE Day;    /*1-31*/
  BYTE byWeek; /*0-6*/
  BYTE Hour;   /*0-23*/
  BYTE Minute; /*0-59*/
  BYTE Second; /*0-59*/
  WORD MSecond; /*0-999*/
}; 

struct IRIG_BDATE{
  int iYear;    /*19XX-20XX*/
  BYTE iMonth;  /*1-12*/
  BYTE iDate;    /*1-31*/
}; 
struct IRIG_BTIME{
  int Year;    /*19XX-20XX*/
  int  Day;    /*1-31*/
  BYTE Hour;   /*0-6  from sunday*/
  BYTE Minute; /*0-59*/
  BYTE Second; /*0-59*/
}; 
void GpsSetBTime(void);
void Irig_B_ms_Scan(void);
void Irigb_isr(void);
void IrigbInit(void);
void Irig_B_Create_Date(struct IRIG_BDATE *pdt, const struct IRIG_BTIME *ptm);
void Irig_B_Create_AbsTime(struct IRIG_BDATE *pdt, const struct IRIG_BTIME *ptm);
void Irig_B_Parse(void);
void CaculateWeekDay(int year,int month, int day);

#endif 
