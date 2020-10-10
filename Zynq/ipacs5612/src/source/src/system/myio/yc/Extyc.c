/*------------------------------------------------------------------------
 Module:       	extyc.c
 Author:        hll
 Project:       
 State:			
 Creation Date:	2013-4-11
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: $
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef INCLUDE_EXT_YC

#include "extyc.h"
#include "sys.h"

#ifdef INCLUDE_AD7913
#include "ad7913.h"
#endif

#ifdef INCLUDE_TMP75_TEMP
#include "tmp75.h"
#endif
static WORD extYcNum;
static WORD extYcStart;
VYcCfg *pExtYcCfg;
static long* extYcValue;
static struct VYCF *dbExtYcValue;

void aiDcInit(void)
{
#ifdef INCLUDE_AD7913
     ad7913Init();
#endif
}

int aiDcGet(int chn)
{
#if defined INCLUDE_AD7913
     return(ad7913Get(chn));
#elif defined INCLUDE_TMP75_TEMP
	return(Tmp75GetTemperature(chn));
#else
     return 1;
#endif
}

void SetRangeExtYccfg()
{
	int i;
	VDefTVYcCfg *pDefTVYcCfg;
	int yccfgNum;
	
	pDefTVYcCfg = GetDefTVYcCfg(0, &yccfgNum);
	
	for(i =0;i<extYcNum;i++)
	{
		WriteYCCfg(g_Sys.wIOEqpID,extYcStart + i, pExtYcCfg[i].type, pExtYcCfg[i].fdNo,pDefTVYcCfg[pExtYcCfg[i].type].unit);	
	}
}

void extYcInit(void)
{

    aiDcInit();

    extYcNum   = g_Sys.MyCfg.wYCNumPublic;
    extYcStart = g_Sys.MyCfg.wYCNum - extYcNum;
    pExtYcCfg  = g_Sys.MyCfg.pYc + g_Sys.MyCfg.wYCNum - extYcNum;

	extYcValue = (long *)calloc(extYcNum, sizeof(long));
	dbExtYcValue = (struct VYCF*)calloc(extYcNum, sizeof(struct VYCF));
	SetRangeExtYccfg();
	
}

void extYcMea(void)
{
    int i;
    static BOOL first = 0;
	
    for(i=0; i<extYcNum; i++)
    {
        switch (pExtYcCfg[i].type)
        {
             case YC_TYPE_FDc:
			 case YC_TYPE_TEMETER:
			 	extYcValue[i] = aiDcGet(pExtYcCfg[i].arg1);
			 	break;
        }
    }

	for(i=0; i<extYcNum; i++)
	{
	
	    dbExtYcValue[i].nValue = extYcValue[i]; 	
		dbExtYcValue[i].byFlag = 0x01;
		
	}   
	if(first == 0)//第一次不要
	{
		first = 1;
		return;
	}
	WriteRangeYCF(g_Sys.wIOEqpID, extYcStart, extYcNum, dbExtYcValue);	
}

#endif
