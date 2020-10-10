/*------------------------------------------------------------------------
 Module:       	fapi.cpp
 Author:        solar
 Project:       
 State:         
 Creation Date:	2010-02-03
 Description:   
------------------------------------------------------------------------*/

#include "syscfg.h"
#include "sys.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

long F24ToLong(BYTE *m_pbyData,short factor)
{
	unsigned short usTemp;
	BYTE ucTemp;

	short	 sFlag,sExp;
	float	fVal;
	int j;

	usTemp = MAKEWORD(m_pbyData[0], m_pbyData[1]);
	
	ucTemp = m_pbyData[2];


	sFlag = 1;
	if(ucTemp & 0x80) sFlag = -1;
		fVal = sFlag * usTemp;
	
	ucTemp &= 0x7f;
	if(ucTemp &0x40) //指数为补码 <0
	{
		sExp = (ucTemp&0x7f) -128;
	}
	else sExp = ucTemp;
	
	sExp -= 16;
	if(sExp < 0) 
	{
		sExp *=-1; 
		for(j=0; j < sExp; j++)
			fVal *= 0.5f;
	}
	else
	{
		for(j=0; j < sExp; j++)
			fVal *= 2.0f;
	}

	fVal *=factor;
	return ((long)fVal);
}


long F754ToLong(BYTE *m_pbyData,short factor)
{
	union
    {
     BYTE	ucVal[4];
     float	fVal;
    }YcValue;

	YcValue.ucVal[0] = m_pbyData[3];
	YcValue.ucVal[1] = m_pbyData[2];
	YcValue.ucVal[2] = m_pbyData[1];
	YcValue.ucVal[3] = m_pbyData[0];

	YcValue.fVal *=factor;
	return ((long)YcValue.fVal);
}

void LongToFloat(int num, long ld, BYTE *fd)
{		
	float fdata;
	BYTE *p;
	
	fdata=ld;
	/*if(num<4)
		fdata/=100;
	else if(num<8)
		fdata/=1000;
	else if(num<11)
		fdata/=10;
	else 
		fdata/=100;*/

	fdata /= 1000;
	p=(BYTE*)&fdata;

	fd[0] =p[0];
	fd[1] =p[1];
	fd[2] =p[2];
	fd[3] =p[3];
}

void YCLongToFloat (WORD wEqpID,int num, long ld, BYTE *fd)
{		
	float fdata;
	BYTE *p;
	struct VYCF_L_Cfg pYcCfg;
	
	fdata=ld;

	ReadYCCfg(wEqpID, num,&pYcCfg,1);

	if(((pYcCfg.tyctype >= YC_TYPE_Ua) && (pYcCfg.tyctype <= YC_TYPE_SU0) && (pYcCfg.tyctype != YC_TYPE_U0))
			||((pYcCfg.tyctype >= YC_TYPE_Uab) && (pYcCfg.tyctype <= YC_TYPE_Uca))
			||((pYcCfg.tyctype >= YC_TYPE_SUa) && (pYcCfg.tyctype <= YC_TYPE_SUc)))//交流电压
	{
		fdata /= 100;
	}
	else if(pYcCfg.tyctype == YC_TYPE_U0)
	{
		if(g_Sys.MyCfg.dwCfg & 0x10)
			fdata /= 1000;
		else
			fdata /= 100;
	}
	else if(((pYcCfg.tyctype >= YC_TYPE_Ia) && (pYcCfg.tyctype <= YC_TYPE_SI0)) 
       			   || ((pYcCfg.tyctype >= YC_TYPE_SIa) && (pYcCfg.tyctype <= YC_TYPE_SIc)))//电流死区
	{
		fdata /= 1000;
	}
	else if(pYcCfg.tyctype == YC_TYPE_FDc) //直流电压死区
	{
		fdata /= 100;
	}
	else if(((pYcCfg.tyctype >= YC_TYPE_Pa) && (pYcCfg.tyctype <= YC_TYPE_Qc)) 
					  || (pYcCfg.tyctype == YC_TYPE_P) || (pYcCfg.tyctype == YC_TYPE_Q) || (pYcCfg.tyctype == YC_TYPE_S)) //功率死区
	{
#if(DEV_SP == DEV_SP_TTU)
		fdata /= 10000;
#else
		fdata /= 10;
#endif
	}
	else if(pYcCfg.tyctype == YC_TYPE_SFreq) //频率死区
	{
			fdata /= 100;
	}
	else if(pYcCfg.tyctype == YC_TYPE_Cos) //功率因数死区
	{
			fdata /= 1000;
	}
	else if((pYcCfg.tyctype == YC_TYPE_Angle) || (pYcCfg.tyctype == YC_TYPE_TEMETER)) 
	{
			fdata /= 100;
	}
	else
	{
		fdata /= 1000;
	}
	
	p=(BYTE*)&fdata;

	fd[0] =p[0];
	fd[1] =p[1];
	fd[2] =p[2];
	fd[3] =p[3];
}

BYTE Gb101_SendFData(BYTE *pdec, BYTE type, struct VDBYCF_L* dd)
{
	BYTE *p;
	float fd;
	DWORD dddd;

	if(type!=13)
	{
		if(dd->byFlag&(1<<6))
		{
			memcpy(&fd,(void*)&dd->lValue,4);
			dddd=fd;
		}
		else
			dddd=dd->lValue;
		pdec[0] = LOBYTE(dddd);
		pdec[1] = HIBYTE(dddd);
		return 2;
	}
	else
	{
		if(dd->byFlag&(1<<6))
			memcpy(&fd,(void*)&dd->lValue,4);
		else
			fd=(float)dd->lValue/1000;
		p=(BYTE*)&fd;
		pdec[0]  =p[0];
		pdec[1]  =p[1];
		pdec[2]  =p[2];
		pdec[3] =p[3];
		return 4;
	}
}
