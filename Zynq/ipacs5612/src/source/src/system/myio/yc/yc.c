/*------------------------------------------------------------------------
 Module:       	yc.c
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-09-1
 Description:  
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 13 $
------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
   计算 5A->10000
        100V-10000

   AD采样  将16位转换为14位采样     
------------------------------------------------------------------------*/        


#include <syscfg.h>
#include "sys.h"

int diNUM=0; int aiNUM=0; int ycNum=0; int fdNum=0; int ddNum=0;
int SignalNo =-1 ;
int SignalVal = 0;
#ifdef INCLUDE_YC
#include "yc.h"
#include "stdio.h"
#include "stdlib.h"
#ifdef INCLUDE_AD7913
#include "ad7913.h"
#endif
#ifdef INCLUDE_ATT7022
#include "att7022e.h"
extern VYcGain *pAttYcGain;
#endif
#include "math.h"

VAiCfg *pAiCfg = NULL;
VYcCfg *pYcCfg = NULL;
VFdCfg *pFdCfg = NULL;

static struct VYCF_L *dbYcValue;  /*经过PTCT及比例换算后的整型值,写入数据库*/
static DWORD ycValueSem;

static int gainNum, aigainNum = 0;
VYcGain *pYcGain;
VYcGain *pMeaGain;


int gainValid=0; int gycInit=0;
DWORD ggainver = YC_GAIN_AD7616_VER; //0x80000000 ad7606 ; 0x80000001 ad7607


static int harBgnNo, angleBgnNo,phBgnNo;


static int yccfgNum;
static VDefTVYcCfg *pDefTVYcCfg;


void meaGainGet(void);
	
void createYcRef(void)
{
    int i, j, k, fd, t;

	harBgnNo = angleBgnNo = phBgnNo=-1;

    //修正AI通道对应遥测通道的类型,使其一致
    //检查Uab Ubc Uca对应的遥测通道号,按正确次序排列,保证其缓冲区与
    //Ua Ub Uc一一对应, 检查对应的AI通道类型
    //检查P Q遥测对应的AI通道类型
    for (i=0; i<ycNum; i++)
    {
		pYcCfg[i].cal = 1;
		switch(pYcCfg[i].type)
		{
			case YC_TYPE_Dc:
			case YC_TYPE_Ua:
			case YC_TYPE_Ub:
			case YC_TYPE_Uc:
			case YC_TYPE_U0:
			case YC_TYPE_Ia:
			case YC_TYPE_Ib:
			case YC_TYPE_Ic:
			case YC_TYPE_I0:
				t = pAiCfg[pYcCfg[i].arg1].type;
				if (t != pYcCfg[i].type)
				{
					pYcCfg[i].type = t;
					//myprintf(SELF_ID, LOG_ATTR_WARN, "Yc%d's type invalid and fix.", i);
				}	
				break;	
			case YC_TYPE_SI0:
			case YC_TYPE_SU0:
				t = pYcCfg[pYcCfg[i].arg1].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				t = pYcCfg[pYcCfg[i].arg1+1].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				t = pYcCfg[pYcCfg[i].arg1+2].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				break;		
			case YC_TYPE_Uab:
			case YC_TYPE_Ubc:
				//Ua在前,Ub在后 或
				//Ub在前,Uc在后
				if (pYcCfg[i].arg1 > pYcCfg[i].arg2)
                {
					t = pYcCfg[i].arg1;
					pYcCfg[i].arg1 = pYcCfg[i].arg2;
					pYcCfg[i].arg2 = t;
				}
				
				t = pYcCfg[pYcCfg[i].arg1].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				t = pYcCfg[pYcCfg[i].arg2].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				break;
			case YC_TYPE_Uca:				
				//Uc在前,Ua在后 
				if (pYcCfg[i].arg1 < pYcCfg[i].arg2)
                {
					t = pYcCfg[i].arg1;
					pYcCfg[i].arg1 = pYcCfg[i].arg2;
					pYcCfg[i].arg2 = t;
				}

				t = pYcCfg[pYcCfg[i].arg1].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				t = pYcCfg[pYcCfg[i].arg2].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				break;
			case YC_TYPE_SUa:
			case YC_TYPE_SUb:
			case YC_TYPE_SUc:
			case YC_TYPE_SIa:
			case YC_TYPE_SIb:
			case YC_TYPE_SIc:
				t = pYcCfg[pYcCfg[i].arg1].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				t = pYcCfg[pYcCfg[i].arg2].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				break;				
			case YC_TYPE_Pa:
			case YC_TYPE_Pb:
			case YC_TYPE_Pc:
				t = pYcCfg[i].type;
				for (j=i+1; j<i+4; j++)
				{
					if (j >= ycNum) break;
					if (pYcCfg[j].type == t+3)
					{
						pYcCfg[i].pair = j;
						pYcCfg[j].pair = i;
						break;
					}
				}
				if (pYcCfg[i].pair == 0)
				{
					WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "交采参数错误");			
					myprintf(SYS_ID, LOG_ATTR_ERROR, "Yc%d cfg error!", i);
				}				
				t = pYcCfg[pYcCfg[i].arg1].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				t = pYcCfg[pYcCfg[i].arg2].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				break;
			case YC_TYPE_Qa:
			case YC_TYPE_Qb:
			case YC_TYPE_Qc:
				t = pYcCfg[pYcCfg[i].arg1].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				t = pYcCfg[pYcCfg[i].arg2].type;
				if ((t==YC_TYPE_Null) || (t==YC_TYPE_Dc)|| (t==YC_TYPE_I0) || (t==YC_TYPE_U0))
					pYcCfg[i].cal = 0;
				break;
			case YC_TYPE_Angle:
				if (angleBgnNo < 0) angleBgnNo = i;
				break;								
			case YC_TYPE_Har2:
			case YC_TYPE_Har3:
			case YC_TYPE_Har4:
			case YC_TYPE_Har5:
			case YC_TYPE_Har6:
			case YC_TYPE_Har7:
			case YC_TYPE_Har8:
			case YC_TYPE_Har9:
			case YC_TYPE_Har10:
			case YC_TYPE_Har11:
			case YC_TYPE_Har12:
			case YC_TYPE_Har13:
			case YC_TYPE_Har1:
				if (harBgnNo < 0) harBgnNo = i;
				break;				
			case YC_TYPE_UPhZ:
			case YC_TYPE_UPhF:
			case YC_TYPE_UPh0:
			case YC_TYPE_IPhZ:
			case YC_TYPE_IPhF:
			case YC_TYPE_IPh0:
				if(phBgnNo < 0) phBgnNo = i;
				break;
			case YC_TYPE_SIGNAL:
				if(SignalNo < 0)
					SignalNo = i;
				break;
				
		}		
	}    
	
    fd = -1;	
	for (i=0; i<ycNum; i++)
	{
        t = pYcCfg[i].type;
		if (t == YC_TYPE_Ua)  /*电压,新回线开始*/
		{
			fd++;
		}	
		else if (t == YC_TYPE_Ia) /*电流*/
		{
			if (i>0)   
			{
                /*前值是线电压*/
				if ((pYcCfg[i-1].type>=YC_TYPE_Uab) && (pYcCfg[i-1].type<=YC_TYPE_Uca))
				{
					k = pYcCfg[i-1].arg1;  /*判断之前有无电流段*/
					for (j=k; j<i-1; j++)
					{
						if (((pYcCfg[k].type >= YC_TYPE_Ia) && (pYcCfg[k].type <= YC_TYPE_SI0)) || \
						    ((pYcCfg[k].type >= YC_TYPE_SIa) && (pYcCfg[k].type <= YC_TYPE_SIc)))
						{
							fd++;    /*有电流段,新回线*/
							break;
						}
					}
						
				}	
				/*前值不为电压*/
				else if (!(((pYcCfg[i-1].type >= YC_TYPE_Ua) && (pYcCfg[i-1].type <= YC_TYPE_SU0)) || \
					       ((pYcCfg[i-1].type >= YC_TYPE_SUa) && (pYcCfg[i-1].type <= YC_TYPE_SUc))))
					fd++;
			}
			else
				fd++;
		}
		
		if (fd >= fdNum)  fd--;

		pYcCfg[i].fdNo = fd;
	}

    for (i=0; i<ycNum; i++)
    {
		switch(pYcCfg[i].type)
		{
            case YC_TYPE_Dc:
			case YC_TYPE_Ua:
			case YC_TYPE_Ub:
			case YC_TYPE_Uc:
			case YC_TYPE_Ia:
			case YC_TYPE_Ib:
			case YC_TYPE_Ic:
			case YC_TYPE_I0:
				pAiCfg[pYcCfg[i].arg1].ycNo = i;
				pAiCfg[pYcCfg[i].arg1].fdNo = pYcCfg[i].fdNo;								
				break;
			case YC_TYPE_U0:  
				//由于电压回线号可能根据功率匹配修正,此次需要修正U0为相邻电压回线号
                if (i > 0)
                {
					t = pYcCfg[i-1].type;					
					if ((t >= YC_TYPE_Ua) && (t <= YC_TYPE_Uc))
						pYcCfg[i].fdNo = pYcCfg[i-1].fdNo;					
				}
				else if (i+1 < ycNum)
				{
					t = pYcCfg[i+1].type;					
					if ((t >= YC_TYPE_Ua) && (t <= YC_TYPE_Uc))
						pYcCfg[i].fdNo = pYcCfg[i+1].fdNo;					
				}
				
				pAiCfg[pYcCfg[i].arg1].ycNo = i;
				pAiCfg[pYcCfg[i].arg1].fdNo = pYcCfg[i].fdNo;								
				break;				
			case YC_TYPE_Uab:
			case YC_TYPE_Ubc:
			case YC_TYPE_Uca:
				//线电压回线号强制为其相电压回线号
			case YC_TYPE_SU0:
			case YC_TYPE_SI0:
				//零序回线号强制为其对应电压/电流回线号
				pYcCfg[i].fdNo = pYcCfg[pYcCfg[i].arg1].fdNo;
				break;								
			case YC_TYPE_SUa:
			case YC_TYPE_SUb:
			case YC_TYPE_SUc:
			case YC_TYPE_SIa:
			case YC_TYPE_SIb:
			case YC_TYPE_SIc:
				//自产值强制为计算参数值回线号
				pYcCfg[i].fdNo = pYcCfg[pYcCfg[i].arg1].fdNo;
				break;	
			case YC_TYPE_Uunb:
			case YC_TYPE_Iunb:
			case YC_TYPE_Upp:
				pYcCfg[i].fdNo = pYcCfg[pYcCfg[i].arg1].fdNo;
				break;
		}		
	}
}

void meaPtCtSet(void)
{
    int i, j, fdno, t;

    for (i=0; i<ycNum; i++)
    {
        t = pYcCfg[i].type;
		if ((t == YC_TYPE_SU0) || (t == YC_TYPE_SI0))
			pYcCfg[i].toZero *=30;
		else if ((t>=YC_TYPE_Har2) && (t<=YC_TYPE_Har9))
			pYcCfg[i].toZero = pYcCfg[pYcCfg[i].arg1].toZero;				
		else
			pYcCfg[i].toZero *=10;

		pYcCfg[i].xs1 = 1;
		pYcCfg[i].xs1_yc1 = 1;
		pYcCfg[i].xs2 = 1;

		fdno = pYcCfg[i].fdNo;

		if (fdno >= fdNum) fdno = fdNum-1;
		if (fdno < 0) fdno = 0;

		if (fdno >= fdNum)  continue;   //must have for fdNum==0

		switch(pYcCfg[i].type)
		{
			case YC_TYPE_Ua:
			case YC_TYPE_Ub:
			case YC_TYPE_Uc:
			case YC_TYPE_U0:
			case YC_TYPE_SU0:
			case YC_TYPE_Uab:
			case YC_TYPE_Ubc:
			case YC_TYPE_Uca:
			case YC_TYPE_SUa:
			case YC_TYPE_SUb:
			case YC_TYPE_SUc:	
				pYcCfg[i].xs1 = 1000;
				pYcCfg[i].xs1_yc1 = pFdCfg[fdno].pt*100/pFdCfg[fdno].Un;
				
				if (pFdCfg[fdno].cfg & 0x02)   //减少一位有效位
					pYcCfg[i].xs2 = 10000;
				else
					pYcCfg[i].xs2 = 1000;
				pYcCfg[i].k   = 10000/pYcCfg[i].xs2;
				pYcCfg[i].xs1_yc1  = pYcCfg[i].xs1_yc1*100/pYcCfg[i].xs2;
				break;
			case YC_TYPE_Ia:
			case YC_TYPE_Ib:
			case YC_TYPE_Ic:
			case YC_TYPE_SI0:
			case YC_TYPE_SIa:
			case YC_TYPE_SIb:
			case YC_TYPE_SIc:				
				pYcCfg[i].xs1 = 500;
				pYcCfg[i].xs1_yc1 = pFdCfg[fdno].ct;
				
		        if (pFdCfg[fdno].cfg & 0x02)   //减少一位有效位
					pYcCfg[i].xs2 = 10000;
				else
					pYcCfg[i].xs2 = 1000;
				pYcCfg[i].k   = 10000/pYcCfg[i].xs2;
				pYcCfg[i].xs1_yc1  = pYcCfg[i].xs1_yc1*100/pYcCfg[i].xs2;
				break;
			case YC_TYPE_I0:
				pYcCfg[i].xs1 = 500;
				pYcCfg[i].xs1_yc1 = pFdCfg[fdno].ct0;
				
			    if (pFdCfg[fdno].cfg & 0x02)   //减少一位有效位
					pYcCfg[i].xs2 = 10000;
				else
				    pYcCfg[i].xs2 = 1000;
				pYcCfg[i].k   = 10000/pYcCfg[i].xs2;
				pYcCfg[i].xs1_yc1  = pYcCfg[i].xs1_yc1*100/pYcCfg[i].xs2;
				break;
			case YC_TYPE_Pa:
			case YC_TYPE_Pb:
			case YC_TYPE_Pc:
			case YC_TYPE_Qa:
			case YC_TYPE_Qb:
			case YC_TYPE_Qc:
			case YC_TYPE_P:
			case YC_TYPE_Q:
			case YC_TYPE_S:	
				/*pYcCfg[i].xs1 = pFdCfg[fdno].pt*pFdCfg[fdno].ct;
				pYcCfg[i].xs2 = 1000*1000*1000;*/
				pYcCfg[i].xs1 = 100*500;

				pYcCfg[i].xs1_yc1= pFdCfg[fdno].pt*10/pFdCfg[fdno].Un*pFdCfg[fdno].ct;
			
			 	if (pFdCfg[fdno].cfg & 0x02)   //减少一位有效位
					pYcCfg[i].xs2 = 100000000;
				else
					pYcCfg[i].xs2 = 10000000;
				pYcCfg[i].k   = 100000000/pYcCfg[i].xs2;
				if (pFdCfg[fdno].cfg & 0x04)  //Mw Mvar Mva
				{
					pYcCfg[i].xs1 /= 1000;
					pYcCfg[i].xs1_yc1 /= 1000;
				}
				pYcCfg[i].xs1_yc1  = pYcCfg[i].xs1_yc1*100/pYcCfg[i].xs2;
				break;
			case YC_TYPE_Har2:
			case YC_TYPE_Har3:
			case YC_TYPE_Har4:
			case YC_TYPE_Har5:   
			case YC_TYPE_Har6:
			case YC_TYPE_Har7:
			case YC_TYPE_Har8:
			case YC_TYPE_Har9:
			case YC_TYPE_Har10:
			case YC_TYPE_Har11:
			case YC_TYPE_Har12:
			case YC_TYPE_Har13:
			case YC_TYPE_UPhZ:
			case YC_TYPE_UPhF:
			case YC_TYPE_UPh0:
			case YC_TYPE_IPhZ:
			case YC_TYPE_IPhF:
			case YC_TYPE_IPh0:
				pYcCfg[i].xs1 = pYcCfg[pYcCfg[i].arg1].xs1;
				pYcCfg[i].xs1_yc1 = pYcCfg[pYcCfg[i].arg1].xs1_yc1;
				pYcCfg[i].xs2 = pYcCfg[pYcCfg[i].arg1].xs2; 									
				pYcCfg[i].k   = pYcCfg[pYcCfg[i].arg1].k;
				pYcCfg[i].xs1_yc1  = pYcCfg[i].xs1_yc1*100/pYcCfg[i].xs2;
				break;
			case YC_TYPE_Cos:
				pYcCfg[i].xs1 = 1;
				pYcCfg[i].xs1_yc1= 1;
				pYcCfg[i].xs2 = 1;	
				pYcCfg[i].k = 1000; 
				pYcCfg[i].xs1_yc1  = pYcCfg[i].xs1_yc1*100/pYcCfg[i].xs2;
				break;
			case YC_TYPE_Angle:
			case YC_TYPE_Dc:				
			case YC_TYPE_FDc:	
			case YC_TYPE_Freq:
			case YC_TYPE_Thd:
			case YC_TYPE_SFreq:
			case YC_TYPE_Uunb:
			case YC_TYPE_Iunb:
			    pYcCfg[i].xs1 = 1;
				pYcCfg[i].xs1_yc1= 1;
				pYcCfg[i].xs2 = 1;	
				pYcCfg[i].k = 100; 
				pYcCfg[i].xs1_yc1  = pYcCfg[i].xs1_yc1*100/pYcCfg[i].xs2;
				break;		
			default:
				pYcCfg[i].xs1 = 1;
				pYcCfg[i].xs2 = 1;	
				pYcCfg[i].k   = 1;
				break;
		}

        for (j=0; j<9; j++)           /*分母都尝试以10约*/ 
		{
		    if((pYcCfg[i].xs1==0)||(pYcCfg[i].xs1%10)||(pYcCfg[i].xs2%10))
				break;
			
			pYcCfg[i].xs1 = pYcCfg[i].xs1/10;
			pYcCfg[i].xs2 = pYcCfg[i].xs2/10;
        }
		for (j=0; j<2; j++)           /*分母都尝试以5约*/ 
		{
			if((pYcCfg[i].xs1==0)||(pYcCfg[i].xs1%5)||(pYcCfg[i].xs2%5))
				break;
			
			pYcCfg[i].xs1 = pYcCfg[i].xs1/5;
			pYcCfg[i].xs2 = pYcCfg[i].xs2/5;
		}
	}
}

void meaWrite_DbValue(void)
{
    int i;

    smMTake(ycValueSem);
	for (i=0; i<ycNum; i++)
	{
		dbYcValue[i].byFlag = 0x01;
		ReadBMRangeYCF_L(BM_EQP_MYIO, i, &dbYcValue[i].lValue);
	}   
	smMGive(ycValueSem);
	
	WriteRangeYCF_L(g_Sys.wIOEqpID, 0, ycNum, dbYcValue);	
}

void measure(void)
{
	meaWrite_DbValue();
}

DWORD ReadYcPtCt(WORD no)
{
	DWORD k,fdno;
	k = 1;
	
	fdno = pYcCfg[no].fdNo;
	
	if (fdno >= fdNum) fdno = fdNum-1;
	
	switch(pYcCfg[no].type)
	{
		case YC_TYPE_Ua:
		case YC_TYPE_Ub:
		case YC_TYPE_Uc:
		case YC_TYPE_U0:
		case YC_TYPE_SU0:
		case YC_TYPE_Uab:
		case YC_TYPE_Ubc:
		case YC_TYPE_Uca:
		case YC_TYPE_SUa:
		case YC_TYPE_SUb:
		case YC_TYPE_SUc:	
			k = pFdCfg[fdno].pt/pFdCfg[fdno].Un;
			break;
		case YC_TYPE_Pa:
		case YC_TYPE_Pb:
		case YC_TYPE_Pc:
		case YC_TYPE_Qa:
		case YC_TYPE_Qb:
		case YC_TYPE_Qc:
		case YC_TYPE_P:
		case YC_TYPE_Q:
		case YC_TYPE_S:	
			k = pFdCfg[fdno].pt/pFdCfg[fdno].Un*pFdCfg[fdno].ct/pFdCfg[fdno].In;
			break;
		case YC_TYPE_Ia:
		case YC_TYPE_Ib:
		case YC_TYPE_Ic:
		case YC_TYPE_SI0:
		case YC_TYPE_SIa:
		case YC_TYPE_SIb:
		case YC_TYPE_SIc:
			k = pFdCfg[fdno].ct/pFdCfg[fdno].In;
			break;
		case YC_TYPE_I0:
			k = pFdCfg[fdno].ct0/pFdCfg[fdno].In0;
			break;
		default:
			k = 1;
		break;
	}
	return k;
}

void createMeaFdRef(void)
{
    int i, j, t, k, m, n, e;
    
	for (i=0; i<fdNum; i++)
    {
		memset(pFdCfg[i].mmi_meaNo_ai, -1, MMI_MEA_NUM*sizeof(int));
		memset(pFdCfg[i].mmi_meaNo_yc, -1, MMI_MEA_NUM*sizeof(int));

        /*建立回线对AI通道的索引,如果无该项值,则索引为-1*/

		for (j=0; j<ycNum; j++)
		{
 			if (pYcCfg[j].fdNo == i)
 			{
                t = pYcCfg[j].type;
                //如果是物理通道对应的电流
				if ((t == YC_TYPE_Ia) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IA] == -1))
				{
                    pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IA] = pYcCfg[j].arg1;
                    pFdCfg[i].mmi_meaNo_yc[MMI_MEA_IA] = j;
					
                    //寻找该回线对应的电压
					k = -1;
					for (n=0; n<ycNum; n++)
					{
						if (pYcCfg[n].type == YC_TYPE_Pa)
						{
							if (pYcCfg[n].arg2 == j)
							{
								k = pYcCfg[n].arg1;
								break;
							}
							else if (pYcCfg[n].arg1 == j)
							{
								k = pYcCfg[n].arg2;
								break;
							}
						}
					}
						
					//找到该回线的电压	
					if (k >= 0)
					{
						for (e=k; e<ycNum; e++)
						{
							t = pYcCfg[e].type;

							if (!(((t>=YC_TYPE_Ua) && (t<=YC_TYPE_SU0)) || \
								 ((t>=YC_TYPE_SUa) && (t<=YC_TYPE_SUc))))
								break;
						}					

						for (m=k; m<e; m++)
                        {
							t = pYcCfg[m].type;
							//如果是物理通道对应的电压							
							if ((t == YC_TYPE_Ua) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UA] == -1))
							{
								pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UA] = pYcCfg[m].arg1;
								pFdCfg[i].mmi_meaNo_yc[MMI_MEA_UA] = m;
							}
							else if ((t == YC_TYPE_Ub) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UB] == -1))
							{
								pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UB] = pYcCfg[m].arg1;
								pFdCfg[i].mmi_meaNo_yc[MMI_MEA_UB] = m;
							}	
							else if ((t == YC_TYPE_Uc) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UC] == -1))
							{
								pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UC] = pYcCfg[m].arg1;
								pFdCfg[i].mmi_meaNo_yc[MMI_MEA_UC] = m;
							}									
							else if ((t == YC_TYPE_U0) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_U0] == -1))
							{
								pFdCfg[i].mmi_meaNo_ai[MMI_MEA_U0] = pYcCfg[m].arg1;
								pFdCfg[i].mmi_meaNo_yc[MMI_MEA_U0] = m;
							}								
                        }							
					}
				}
                //可能出现该情况, FTU中Ib设为零序,此时后续多余通道Ib会成为前一回线的Ib, 因为未能               
                //归类的通道都算做前一回线,所以此处当Ib出现在I0后面时,不认为该Ib为此回线量                
				else if ((t == YC_TYPE_Ib) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IB] == -1) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_I0] == -1))
				{
                    pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IB] = pYcCfg[j].arg1;
					pFdCfg[i].mmi_meaNo_yc[MMI_MEA_IB] = j;
				}
				else if ((t == YC_TYPE_Ic) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IC] == -1))
				{
					pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IC] = pYcCfg[j].arg1;
					pFdCfg[i].mmi_meaNo_yc[MMI_MEA_IC] = j;
				}
                //测量零序
				else if ((t == YC_TYPE_I0) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_I0] == -1))
				{
					pFdCfg[i].mmi_meaNo_ai[MMI_MEA_I0] = pYcCfg[j].arg1;
					pFdCfg[i].mmi_meaNo_yc[MMI_MEA_I0] = j;
				}
			} 
		}

		//回线无电流，仅电压型
        if (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IA] == -1)
        {
			for (m=0; m<ycNum; m++)
			{
				if (pYcCfg[m].fdNo == i) 
				{
					t = pYcCfg[m].type;
					//如果是物理通道对应的电压							
					if ((t == YC_TYPE_Ua) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UA] == -1))
					{
						pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UA] = pYcCfg[m].arg1;
						pFdCfg[i].mmi_meaNo_yc[MMI_MEA_UA] = m;
					}
					else if ((t == YC_TYPE_Ub) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UB] == -1))
					{
						pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UB] = pYcCfg[m].arg1;
						pFdCfg[i].mmi_meaNo_yc[MMI_MEA_UB] = m;
					}	
					else if ((t == YC_TYPE_Uc) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UC] == -1))
					{
						pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UC] = pYcCfg[m].arg1;
						pFdCfg[i].mmi_meaNo_yc[MMI_MEA_UC] = m;
					}									
					else if ((t == YC_TYPE_U0) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_U0] == -1))
					{
						pFdCfg[i].mmi_meaNo_ai[MMI_MEA_U0] = pYcCfg[m].arg1;
						pFdCfg[i].mmi_meaNo_yc[MMI_MEA_U0] = m;
					}	
					else if ((t == YC_TYPE_I0 ) && (pFdCfg[i].mmi_meaNo_ai[MMI_MEA_I0] == -1))
					{
						pFdCfg[i].mmi_meaNo_ai[MMI_MEA_I0] = pYcCfg[m].arg1;
						pFdCfg[i].mmi_meaNo_yc[MMI_MEA_I0] = m;
					}
				}				
			}
		}		
	}
}

void ddInit(void)
{
#ifdef  INCLUDE_ATT7022
     Init7022E();
#else
	g_Sys.MyCfg.wDDNum = 0;
	g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo->Cfg.wDDNum = 0;
	g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo->DDSendCfg.wNum = 0;
	g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo->DDHourFrz.wNum = 0;
	g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo->DDDayFrz.wNum = 0;
	g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo->DDMonthFrz.wNum = 0;
#endif
}


void SetRangeYccfg()
{
	int i;
	
	for(i =0;i<ycNum;i++)
	{
		if(pYcCfg[i].type < yccfgNum)
			WriteYCCfg(g_Sys.wIOEqpID, i, pYcCfg[i].type, pYcCfg[i].fdNo,pDefTVYcCfg[pYcCfg[i].type].unit);	
	}
}

void ycInit(void)
{
	gainValid = gycInit = 0;

	diNUM = g_Sys.MyCfg.wDINum;
	aiNUM = g_Sys.MyCfg.wAINum;
	ycNum = g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic;
	fdNum = g_Sys.MyCfg.wFDNum;

	pAiCfg = g_Sys.MyCfg.pAi;
	pYcCfg = g_Sys.MyCfg.pYc;
	pFdCfg = g_Sys.MyCfg.pFd;
	
	if ((pAiCfg == NULL)||(pYcCfg == NULL)||(pFdCfg == NULL))
		return;

	ycValueSem = smMCreate();

	createYcRef();
  
	meaGainGet();
	if (gainNum == 0) return;
	
	dbYcValue = (struct VYCF_L *)calloc(ycNum, sizeof(struct VYCF_L));

	if (!(dbYcValue))  return;

	createMeaFdRef();

	meaPtCtSet();

	pDefTVYcCfg = GetDefTVYcCfg(0, &yccfgNum);	

	ddInit();
	
	SetRangeYccfg();
	
	gycInit = 1;
}

void createGainNo2YcRef(int gainnum)
{ 
    int i, j;

	for (i=0; i<aiNUM; i++)
	{
	    pMeaGain[i].ycNo  = pAiCfg[i].ycNo;
		pMeaGain[i].fd    = pAiCfg[i].fdNo;
		pMeaGain[i].type  = pAiCfg[i].type;
	}
	
	j = 0;
	for (i=0; i<ycNum; i++)
	{
		if (((pYcCfg[i].type > YC_TYPE_Dc) && (pYcCfg[i].type <= YC_TYPE_U0))	
		   || ((pYcCfg[i].type >= YC_TYPE_Ia) && (pYcCfg[i].type <= YC_TYPE_I0))	
		   || ((pYcCfg[i].type >= YC_TYPE_Pa) && (pYcCfg[i].type <= YC_TYPE_Qc)))
		{
			pYcGain[j].ycNo = i;
			j++;
		}	
	}
}

void setDefGainValue(void)
{
    int i, j;
	
	for (i=0; i<aiNUM; i++)
	{
		pMeaGain[i].ycNo = pAiCfg[i].ycNo;
	    pMeaGain[i].a = 65536;
		pMeaGain[i].b = 0;
		pMeaGain[i].fd = pAiCfg[i].fdNo;
		pMeaGain[i].type = pAiCfg[i].type;
		pMeaGain[i].status = 0;
    }	

    j = 0;
	for (i=0; i<ycNum; i++)
	{
		switch (pYcCfg[i].type)
		{
			case YC_TYPE_Ua:
			case YC_TYPE_Ub:
			case YC_TYPE_Uc:
			case YC_TYPE_U0:
				pYcGain[j].ycNo = i;
#if (DEV_SP == DEV_SP_TTU)
				pYcGain[j].a = 0;  
#else
				pYcGain[j].a = 10000;  
#endif
				pYcGain[j].b = 0;
				pYcGain[j].fd = pYcCfg[i].fdNo;
				pYcGain[j].type = pYcCfg[i].type;
			    pYcGain[j].status = 0;
				j++;
				break;
			case YC_TYPE_Ia:
			case YC_TYPE_Ib:
			case YC_TYPE_Ic:
			case YC_TYPE_I0:
				pYcGain[j].ycNo = i;
#if (DEV_SP == DEV_SP_TTU)
				pYcGain[j].a = 0;  
#else
				pYcGain[j].a = 10000;  
#endif
				pYcGain[j].b = 0;
				pYcGain[j].fd = pYcCfg[i].fdNo;
				pYcGain[j].type = pYcCfg[i].type;
				pYcGain[j].status = 0;
				j++;
				break;
			case YC_TYPE_Pa:
			case YC_TYPE_Pb:
			case YC_TYPE_Pc:
			case YC_TYPE_Qa:
			case YC_TYPE_Qb:
			case YC_TYPE_Qc:
				pYcGain[j].ycNo = i;
	    #ifdef YC_GAIN_NEW
#if (DEV_SP == DEV_SP_TTU)
				pYcGain[j].a = 0;  
#else
				pYcGain[j].a = 10000;  
#endif
		        pYcGain[j].b = 0;
	    #else
				pYcGain[j].a = 1024; 
				pYcGain[j].b = 1024;
	    #endif
				pYcGain[j].fd = pYcCfg[i].fdNo;
				pYcGain[j].type = pYcCfg[i].type;
				pYcGain[j].status = 0;
				j++;
				break;				
		}		
	}	
}

int meaGainInit(void)
{
    int i, j, k, gainnum = 0;
	VYcGain *pGain;

	for (i=0; i<ycNum; i++)
	{
		if (((pYcCfg[i].type > YC_TYPE_Dc) && (pYcCfg[i].type <= YC_TYPE_U0))	
		   || ((pYcCfg[i].type >= YC_TYPE_Ia) && (pYcCfg[i].type <= YC_TYPE_I0))	
		   || ((pYcCfg[i].type >= YC_TYPE_Pa) && (pYcCfg[i].type <= YC_TYPE_Qc)))
			gainnum++;
		pYcCfg[i].gainNo = -1;
	}
	
	for (i=0; i<aiNUM; i++)
	{
		if (((pAiCfg[i].type > YC_TYPE_Dc) && (pAiCfg[i].type <= YC_TYPE_U0))	
		   || ((pAiCfg[i].type >= YC_TYPE_Ia) && (pAiCfg[i].type <= YC_TYPE_I0)))	
			aigainNum++;
	}

	if (gainnum == 0) return 0;

	pGain = (VYcGain *)malloc(sizeof(VYcGain)*(aiNUM+gainnum)); 
	if (pGain == NULL) return 0;
    memset(pGain, 0, sizeof(VYcGain)*(aiNUM+gainnum));
	
    pMeaGain = pGain;
	pYcGain = pGain+aiNUM;

	j = k = 0;
	for (i=0; i<ycNum; i++)
	{
		switch (pYcCfg[i].type)
		{
			case YC_TYPE_Ua:
			case YC_TYPE_Ub:
			case YC_TYPE_Uc:
			case YC_TYPE_U0:
			case YC_TYPE_Ia:
			case YC_TYPE_Ib:
			case YC_TYPE_Ic:
			case YC_TYPE_I0:
			case YC_TYPE_Pa:
			case YC_TYPE_Pb:
			case YC_TYPE_Pc:
			case YC_TYPE_Qa:
			case YC_TYPE_Qb:
			case YC_TYPE_Qc:
				pYcCfg[i].gainNo = j;
				j++;
				break;
			case YC_TYPE_SU0:
				pYcCfg[i].gainNo = gainnum+k;
				k++;
				break;												
			case YC_TYPE_SI0:
				pYcCfg[i].gainNo = gainnum+k;
				k++;
				break;												
			case YC_TYPE_Uab:
			case YC_TYPE_Ubc:
			case YC_TYPE_Uca:
				pYcCfg[i].gainNo = gainnum+k;
				k++;
				break;								
			case YC_TYPE_SUa:
			case YC_TYPE_SUb:
			case YC_TYPE_SUc:
			case YC_TYPE_SIa:
			case YC_TYPE_SIb:
			case YC_TYPE_SIc:
				pYcCfg[i].gainNo = gainnum+k;
				k++;
				break;								
		}		
	}

	createGainNo2YcRef(gainnum);
	
	return gainnum;			
}     

int meaGainFileRead(DWORD *ptype, VYcGain *pgain, int len, WORD *pcrc)
{
	FILE *fp;
	char path[4*MAXFILENAME];
	WORD mycrc;
	DWORD ver,dlen;

	GetMyPath(path, "gain.sys");
	fp = fopen(path, "rb");
	if (fp == NULL)  
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Can not find gain.sys!");
		goto Err;
	}	
	ver = 0;
	fread(&ver, 1, sizeof(DWORD), fp);
	if ((ver & YC_GAIN_VALID_VER) != YC_GAIN_AD7606_VER)
	{
	    myprintf(SYS_ID, LOG_ATTR_ERROR, "Gain.sys version error!");
		goto Err;
	}
	ggainver = ver;
	dlen = 0;
	fread(&dlen, 1, sizeof(DWORD), fp);
	if (dlen != len)
	{
	    myprintf(SYS_ID, LOG_ATTR_ERROR, "Gain.sys len error!");
		goto Err;
	}
	*ptype = 0;
	fread(ptype, 1, sizeof(DWORD), fp);
	if (*ptype != g_Sys.MyCfg.dwAIType)
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Gain.sys type error!");
		goto Err;
	}
	if (len != fread(pgain, sizeof(BYTE), len, fp))
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Gain.sys error!");
		goto Err;
	}
	*pcrc = 0;
	fread(pcrc, 1, sizeof(WORD), fp);
	mycrc = GetParaCrc16((BYTE *)pgain, len);
	if (*pcrc != mycrc)
	{
		myprintf(SYS_ID, LOG_ATTR_ERROR, "Gain.sys crc error!");
		goto Err;
	}

	fclose(fp);
	return OK;

Err:
	if (fp != NULL) fclose(fp);	
	return ERROR;
}

int meaGainFileWrite(DWORD type, VYcGain *pgain, int len, WORD crc, int flag)
{
	FILE *fp;
	char path[4*MAXFILENAME];
	DWORD ver;


	GetMyPath(path, "gain.sys");
	fp = fopen(path, "wb");
	if (fp == NULL) return ERROR;

    if(flag == 1)
    {
		if((ggainver & YC_GAIN_VALID_VER) == YC_GAIN_AD7606_VER)		
			ver = ggainver;
		else
			ver = YC_GAIN_AD7607_VER;
		
		if (sizeof(DWORD) != fwrite(&ver, 1, sizeof(DWORD), fp)) goto Err;
		if (sizeof(DWORD) != fwrite(&len, 1, sizeof(DWORD), fp)) goto Err;
    }
	if (sizeof(DWORD) != fwrite(&type, 1, sizeof(DWORD), fp)) goto Err;
	if (len != fwrite(pgain, sizeof(BYTE), len, fp)) goto Err;
	if (sizeof(WORD) != fwrite(&crc, 1, sizeof(WORD), fp)) goto Err;

    fclose(fp);
	return OK;

Err:
    fclose(fp);
	return ERROR;	
}

int meaGainVerRead(DWORD addr, DWORD *pver, int *plen, int log)
{
    int len;
    romNvRamGet(addr, (BYTE *)pver, sizeof(DWORD));
	if ((*pver & YC_GAIN_VALID_VER) != YC_GAIN_AD7606_VER)
	{
	    if (log)
		{
			WriteWarnEvent(NULL, SYS_ERR_GAIN, 0, "增益系数版本错误");		
			myprintf(SYS_ID, LOG_ATTR_ERROR, "Gain ver error!");
		}
		return ERROR;
	}
	else
	{
		ggainver = *pver;
	}
	
	romNvRamGet(addr+sizeof(DWORD), (BYTE *)&len, sizeof(DWORD));
	if (len != *plen) return ERROR;
	return OK;
}

int meaGainEERead(DWORD addr, DWORD *ptype, VYcGain *pgain, int len, WORD *pcrc, int log)
{
	WORD mycrc;

	romNvRamGet(addr, (BYTE *)ptype, sizeof(DWORD));

	if (*ptype != g_Sys.MyCfg.dwAIType)
	{
		if (log)
		{
			WriteWarnEvent(NULL, SYS_ERR_GAIN, 0, "增益系数模拟板型号错误");		
			myprintf(SYS_ID, LOG_ATTR_ERROR, "Gain type error!");
		}	
		return ERROR;
	}
	romNvRamGet(addr+sizeof(DWORD), (BYTE *)pgain, len);
	romNvRamGet(addr+sizeof(DWORD)+len, (BYTE *)pcrc, sizeof(WORD));		
	
	mycrc = GetParaCrc16((BYTE *)pgain, len);
	if (*pcrc != mycrc)
	{
        if (log)
        {
			WriteWarnEvent(NULL, SYS_ERR_GAIN, 0, "增益系数校验错误");		
			myprintf(SYS_ID, LOG_ATTR_ERROR, "Gain crc error!");
        }	
		return ERROR;
	}

	return OK;
}

int meaGainEEWrite(DWORD type, VYcGain *pgain, int len, WORD crc)
{
    DWORD ver = YC_GAIN_AD7607_VER;
	DWORD addr = NVRAM_AD_GAIN;
	
	if((ggainver & YC_GAIN_VALID_VER) == YC_GAIN_AD7606_VER)
		ver = ggainver;
	
    romNvRamSet(addr, (BYTE *)&ver, sizeof(DWORD));
	romNvRamSet(addr+sizeof(DWORD), (BYTE *)&len, sizeof(DWORD));
	romNvRamSet(addr+2*sizeof(DWORD), (BYTE *)&type, sizeof(DWORD));
	romNvRamSet(addr+3*sizeof(DWORD), (BYTE *)pgain, len);
	romNvRamSet(addr+3*sizeof(DWORD)+len, (BYTE *)&crc, sizeof(WORD));

	ver = type = crc = 0;

    if ((meaGainVerRead(addr, &ver, &len, 0) == ERROR)||
		(meaGainEERead(addr+2*sizeof(DWORD),&type, pgain, len ,&crc, 0) == ERROR))
    {
		if (pgain == pMeaGain)
		{
			createGainNo2YcRef(gainNum);   //重建,防止读错误导致索引非法
			setDefGainValue();			
		}

		WriteWarnEvent(NULL, SYS_ERR_GAIN, 0, "写增益系数失败!");		
		myprintf(SYS_ID, LOG_ATTR_ERROR, "\nWrite Gain failed, EEPROM err?");

		return ERROR;
	}

    return OK;
}

int meaGainZero(void)
{
    int len;
    WORD crc;

    if (gainNum == 0) 		
	{
		shellprintf("\nError! gain num = 0\n");
		return ERROR; 
	}	

	shellprintf("zero gain...");
	
	len = sizeof(VYcGain)*(aiNUM+gainNum);
	extNvRamGetBM(NVRAM_AD_GAIN+3*sizeof(DWORD), (BYTE *)pMeaGain, len);
	extNvRamGetBM(NVRAM_AD_GAIN+3*sizeof(DWORD)+len, (BYTE *)&crc, sizeof(WORD));
	//crc = GetParaCrc16((BYTE *)pMeaGain, len);
	if (meaGainEEWrite(g_Sys.MyCfg.dwAIType, pMeaGain, len, crc) == ERROR)
	{
		shellprintf("\nError!\n");
		return ERROR; 
	}
	return OK;
}

void meaGainGet(void)
{
    int i, len;
	DWORD type,addr,ver;
	WORD crc;
	
	gainNum = meaGainInit();
	if (gainNum == 0) return;

    addr = NVRAM_AD_GAIN;
	len = sizeof(VYcGain)*(aiNUM+gainNum);

	if ((meaGainVerRead(addr, &ver, &len, 0) == ERROR)||
		(meaGainEERead(addr+2*sizeof(DWORD), &type, pMeaGain, len, &crc, 0) == ERROR))
	{
	    myprintf(SYS_ID, LOG_ATTR_INFO, "Try read gain again...");
		thSleep(100);
		if ((meaGainVerRead(addr, &ver, &len, 1) == ERROR)||
		    (meaGainEERead(addr+2*sizeof(DWORD), &type, pMeaGain, len, &crc, 1) == ERROR))
		{
			myprintf(SYS_ID, LOG_ATTR_INFO, "Try use gain.sys...");
			if (meaGainFileRead(&type, pMeaGain, len, &crc) == ERROR)
			{
				createGainNo2YcRef(gainNum);   //重建,防止读错误导致索引非法
				setDefGainValue();
				return;
			}
			else
			{
				if (meaGainEEWrite(type, pMeaGain, len, crc) == OK)
				{
					DelFile("gain.sys");
					SysClearErr(SYS_ERR_GAIN);
				}	
			}
		}
	}

	//设置完存储到共享内存里面
	/*myprintf(SYS_ID, LOG_ATTR_INFO, "set bm nvram ver %x type %x ",ver,type);
	extNvRamSetBM(addr, (BYTE *)&ver, sizeof(DWORD));
	extNvRamSetBM(addr+sizeof(DWORD), (BYTE *)&len, sizeof(DWORD));
	extNvRamSetBM(addr+2*sizeof(DWORD), (BYTE *)&type, sizeof(DWORD));
	extNvRamSetBM(addr+3*sizeof(DWORD), (BYTE *)pMeaGain, len);
	extNvRamSetBM(addr+3*sizeof(DWORD)+len, (BYTE *)&crc, sizeof(WORD));*/
	
	createGainNo2YcRef(gainNum);   //重建,防止读错误导致索引非法

    for (i=0; i<aiNUM; i++)
	{
	    if (pMeaGain[i].a == 0) 
		{
		    pMeaGain[i].a = 65536;
			pMeaGain[i].status &= ~YC_GAIN_BIT_VALID;
		}
    }
     
    for (i=0; i<gainNum; i++)
    {
		if (pYcGain[i].a == 0) 
		{
			switch(pYcCfg[pYcGain[i].ycNo].type)
			{
				case YC_TYPE_Pa:
				case YC_TYPE_Pb:
				case YC_TYPE_Pc:
				case YC_TYPE_Qa:
				case YC_TYPE_Qb:
				case YC_TYPE_Qc:
					pYcGain[i].a = 1; 
					pYcGain[i].b = 1;
					break;
				default:
					pYcGain[i].a = 10000;
					pYcGain[i].b = 0;	
					break;
			}	
			pYcGain[i].status &= ~YC_GAIN_BIT_VALID;
		}
    }

	switch(ggainver)
	{
		case YC_GAIN_AD7606_VER:
			myprintf(SYS_ID, LOG_ATTR_INFO, "sample chip is ad7606. ");
			break;
		case YC_GAIN_AD7607_VER:
			myprintf(SYS_ID, LOG_ATTR_INFO, "sample chip is ad7607. ");
			break;
		case YC_GAIN_ADE9078_VER:
			myprintf(SYS_ID, LOG_ATTR_INFO, "sample chip is ade9078. ");
			break;
		case YC_GAIN_ATT7022_VER:
			myprintf(SYS_ID, LOG_ATTR_INFO, "sample chip is att7022. ");
			break;
		case YC_GAIN_ADS8578_VER:
			myprintf(SYS_ID, LOG_ATTR_INFO, "sample chip is ads8578. ");
			break;
		case YC_GAIN_AD7616_VER:
			myprintf(SYS_ID, LOG_ATTR_INFO, "sample chip is ad7616. ");
			break;
		default:
			myprintf(SYS_ID, LOG_ATTR_INFO, "sample chip is error!! ");
			break;
	}

	gainValid = 1;    
}


int GainRead(int log)
{
	int len;
	DWORD type,addr,ver;
	WORD crc;
	VYcGain *pgain;
	int flag = 1;

    len = sizeof(VYcGain)*(aiNUM+gainNum);
	pgain = (VYcGain *)malloc(len);
	if (pgain == NULL) return ERROR;

    addr = NVRAM_AD_GAIN;
	if (meaGainVerRead(addr, &ver, &len, 0) == OK)
		addr += 2*sizeof(DWORD);

	if (meaGainEERead(addr, &type, pgain, len, &crc, log) == ERROR) goto Err;
	if (meaGainFileWrite(type, pgain, len, crc, flag) == ERROR) goto Err;

	free(pgain);
	return OK;

Err:
	free(pgain);
	return ERROR;
}

int GainWrite(void)
{
	int len;
	DWORD type;
	WORD crc;
	VYcGain *pgain;

	len = sizeof(VYcGain)*(aiNUM+gainNum);
	pgain = (VYcGain *)malloc(len);
	if (pgain == NULL) return ERROR;

	if (meaGainFileRead(&type, pgain, len, &crc) == ERROR) goto Err;
	if (meaGainEEWrite(type, pgain, len, crc) == ERROR) goto Err;

	free(pgain);
	return OK;

Err:
	free(pgain);
	return ERROR;

}

int gainread(void)
{
	if (GainRead(1) == OK)
		shellprintf("Read gain ...done\n");
	else		
		shellprintf("Read gain ...error!\n");

	return OK;
}

int gainclear(void)
{
    DWORD type = 0;

	shellprintf("Clear gain ...");

	GainRead(0);
	//FileRename("gain.sys", "gainbak.sys");

	romNvRamSet(NVRAM_AD_GAIN, (BYTE *)&type, sizeof(DWORD));
#if (DEV_SP == DEV_SP_DTU_IU)
    meaAtt7022GainEEClear();
#endif	
	shellprintf("done\n");

	return OK;
}

int ddreset(void)
{
#if defined  (INCLUDE_DD) && !defined (INCLUDE_ATT7022)
	ddResetFlag = 1;
#endif

	return OK;
}

#endif

#ifdef YC_GAIN_NEW
int meaGainValGet(BYTE* pData)
{
    int num = 0;
#ifdef INCLUDE_YC
    int i,ycno;
	VYcVal* pbuf = (VYcVal*)pData;

	if (gainNum == 0) return 0;

	for (i=0; i<gainNum; i++)
	{
	     ycno = pYcGain[i].ycNo;

		 if (((pYcGain[i].type >= YC_TYPE_Ua)&&(pYcGain[i].type <= YC_TYPE_U0))||
		 	 ((pYcGain[i].type >= YC_TYPE_Ia)&&(pYcGain[i].type <= YC_TYPE_I0)))
		 {
		     pbuf->ycNo = ycno;
			 pbuf->value = dbYcValue[ycno].lValue;
			 pbuf->type = pYcCfg[ycno].type;
			 pbuf++;
		 }
		 else if ((pYcGain[i].type >= YC_TYPE_Pa)&&(pYcGain[i].type <= YC_TYPE_Pc))
		 {
		     pbuf->ycNo = ycno;
			 pbuf->value = dbYcValue[ycno].lValue;
			 pbuf->type = pYcCfg[ycno].type;
			 pbuf++;

             ycno = pYcCfg[ycno].pair;
			 pbuf->ycNo = ycno;
			 pbuf->value = dbYcValue[ycno].lValue;
			 pbuf->type = pYcCfg[ycno].type;
			 pbuf++;
		 }	   
	}
	#ifdef INCLUDE_ATT7022
	for (i=0; i<ycNum; i++)
	{
	    if ((pYcCfg[i].type == YC_TYPE_P)||(pYcCfg[i].type == YC_TYPE_Q))
	    {
	       pbuf->ycNo = i;
		   pbuf->value = dbYcValue[i].lValue;
		   pbuf->type = pYcCfg[i].type;
		   pbuf++;
		   num++;
	    }
	}
	#endif
	num += gainNum;
#endif

#ifdef INCLUDE_AD7913
	num += ad7913GainValGet(pbuf);
#endif

	return num;
}

/*下发数据按遥测顺序下发*/
int meaGainValSet(int flag, int num, BYTE* pData)
{
#ifdef INCLUDE_YC

    int len;
	WORD crc;
	VYcVal* pbuf = (VYcVal*)pData;

    if (gainNum == 0) 		
	{
		shellprintf("\nError! gain num = 0\n");
		return ERROR; 
	}

	if (flag == 0)
	{
	    shellprintf("channle gain...");
	}
	else if (flag == 1)
	{
	     shellprintf("line power gain...");	 
	}
#ifdef INCLUDE_ATT7022
	else if (flag == 3)   //0.5L 相角校准 Pa,Pb,Pc
	{
        shellprintf("angle gain...");
	    ret = OK;
	    for (i=0; i<num/2; i++)
	    {
	         ycno = pbuf[2*i].ycNo;
			 ycno2 = pbuf[2*i+1].ycNo;
			 ret |= Cal_angle_gain(pYcCfg[ycno].type -YC_TYPE_Pa + 1, pbuf[2*i].value);
	    }
		return ret;
	}
	else if (flag == 4)   //allgain P & T
	{
         ycno = pbuf[0].ycNo;
		 ycno2 = pbuf[1].ycNo;
		 T = *(int*)(pbuf+2);
		 ret = Cal_all_gain(pYcCfg[ycno].type -YC_TYPE_P + 1, pbuf[0].value, T);
		 return ret;
	}
#endif
	if ((flag == 0)||(flag == 1))
	{
#ifdef INCLUDE_ATT7022
#if (DEV_SP == DEV_SP_TTU)
        len = sizeof(VYcGain)*(aiNUM+gainNum);
        crc = GetParaCrc16((BYTE *)pMeaGain, len);
        if (meaGainEEWrite(g_Sys.MyCfg.dwAIType, pMeaGain, len, crc) == ERROR)
        {
        	shellprintf("\nError!\n");
        	return ERROR; 
        }
#else
        if (meaAtt7022GainEEWrite() == ERROR)       //写入铁电
        {
            shellprintf("\nATT7022EError!\n");
            return ERROR; 
        }
#endif
#endif

#ifdef INCLUDE_AD7606
		//从裸核里面取出来整定系数
		len = sizeof(VYcGain)*(aiNUM+gainNum);
		extNvRamGetBM(NVRAM_AD_GAIN+3*sizeof(DWORD), (BYTE *)pMeaGain, len);
		extNvRamGetBM(NVRAM_AD_GAIN+3*sizeof(DWORD)+len, (BYTE *)&crc, sizeof(WORD));
		if (meaGainEEWrite(g_Sys.MyCfg.dwAIType, pMeaGain, len, crc) == ERROR)
		{
			shellprintf("\nError!\n");
			return ERROR; 
		}
#endif
		return OK;
	}
#endif
#ifdef INCLUDE_AD7913
	if (flag == 2)
	{
	   return(ad7913GainValSet(num, pbuf));
	}
#endif
    return ERROR;
}

int meaGainSetGet(BYTE *pgain)
{
    int num = 0;
#ifdef INCLUDE_YC
    VYcGain *pbuf=(VYcGain*)pgain;
	int i;
	WORD ycNo, gainNo;

	if (gainNum == 0) 	return 0;


    for(i=0; i<aiNUM; i++)
	{
	    if(((pMeaGain[i].type >= YC_TYPE_Ua) && (pMeaGain[i].type <= YC_TYPE_U0))||
		   ((pMeaGain[i].type >= YC_TYPE_Ia) && (pMeaGain[i].type <= YC_TYPE_I0)))
	    {
	       pbuf->ycNo = i;
		   pbuf->fd   = pMeaGain[i].fd;
		   pbuf->a    = pMeaGain[i].a;
		   pbuf->b    = pMeaGain[i].b;
		   pbuf->type = pMeaGain[i].type;
		   pbuf->status = 0;
		   pbuf++;
	    }
	}

	for(i=0; i<gainNum; i++)
	{
	    if((pYcGain[i].type >= YC_TYPE_Qa) && (pYcGain[i].type <= YC_TYPE_Qc))
			continue;
		else if((pYcGain[i].type >= YC_TYPE_Pa) && (pYcGain[i].type <= YC_TYPE_Pc))
		{
		    pbuf->ycNo = pYcGain[i].ycNo;
			pbuf->fd   = pYcGain[i].fd;
			pbuf->a    = pYcGain[i].a;
			pbuf->b    = pYcGain[i].b;
			pbuf->type = pYcGain[i].type;
			pbuf->status = 1;

			pbuf++;
			
			ycNo = pYcCfg[pYcGain[i].ycNo].pair;
			gainNo = pYcCfg[ycNo].gainNo;

			pbuf->ycNo = pYcGain[gainNo].ycNo;
			pbuf->fd   = pYcGain[gainNo].fd;
			pbuf->a    = pYcGain[gainNo].a;
			pbuf->b    = pYcGain[gainNo].b;
			pbuf->type = pYcGain[gainNo].type;
			pbuf->status = 1;

			pbuf++;	
		}
		else
		{
		    pbuf->ycNo = pYcGain[i].ycNo;
			pbuf->fd   = pYcGain[i].fd;
			pbuf->a    = pYcGain[i].a;
			pbuf->b    = pYcGain[i].b;
			pbuf->type = pYcGain[i].type;
			pbuf->status = 1;

			pbuf++;
		}
	}
	num = gainNum+aigainNum;
#endif
#ifdef INCLUDE_AD7913
    num += ad7913GainGet(pbuf);
#endif
	return num;
}


int meaGainSetSet(BYTE *pgain)
{
    int num = 0;
	int ret = ERROR;
#ifdef INCLUDE_YC
    int i,len;
    WORD gainNo,crc;
	VYcGain *pbuf = (VYcGain*)pgain;
	
    if (gainNum == 0) 	return ERROR;

	for(i=0; i<aigainNum; i++)
	{
	    gainNo = pbuf[i].ycNo;
		if (pbuf[i].a != 0)
		{
			pMeaGain[gainNo].a = pbuf[i].a;
		}
		if (pbuf[i].b != 0)
		{
			pMeaGain[gainNo].b = pbuf[i].b;
		}
	}
	
	for(i=aigainNum; i<(aigainNum+gainNum); i++)
	{
	    gainNo = pYcCfg[pbuf[i].ycNo].gainNo;
		if (pbuf[i].a == 0)
			continue;
		pYcGain[gainNo].a = pbuf[i].a;

		if ((pbuf[i].type >= YC_TYPE_Pa)&&(pbuf[i].type <= YC_TYPE_Qc))
			pYcGain[gainNo].b = pbuf[i].b;
	}
	
	len = sizeof(VYcGain)*(aiNUM+gainNum);
	crc = GetParaCrc16((BYTE *)pMeaGain, len);
	if (meaGainEEWrite(g_Sys.MyCfg.dwAIType, pMeaGain, len, crc) == ERROR)
	{
		shellprintf("\nError!\n");
		return ERROR; 
	}
	num = aigainNum+gainNum;
	ret = OK;
#endif
#ifdef INCLUDE_AD7913
    ret = ad7913GainSet(pbuf+num);
#endif
	return ret;
	
}

int gainvalset(int flag, int num, BYTE* pdata)
{
    return(meaGainValSet(flag, num, pdata));
}

int gainvalget(BYTE *pdata)
{
    return(meaGainValGet(pdata));
}

int gainget(BYTE *pgain)
{
    return(meaGainSetGet(pgain));
}

int gainset(BYTE *pgain)
{
    return(meaGainSetSet(pgain));
}

int gainzero(void)
{
   return(meaGainZero());
}

int gainsetadtype(DWORD adtype)
{
  DWORD ver = YC_GAIN_AD7607_VER;
  DWORD addr = NVRAM_AD_GAIN;

  if((adtype & YC_GAIN_VALID_VER) == YC_GAIN_AD7606_VER)
  {
    ver = adtype;
  }
  else
    return ERROR;

  romNvRamSet(addr, (BYTE *)&ver, sizeof(DWORD));
  ver = 0;
  romNvRamGet(addr, (BYTE *)&ver, sizeof(DWORD));
  if(ver == adtype)
  {
    ggainver = adtype;
    return OK;
  }
  else
    return ERROR;
}

#endif

