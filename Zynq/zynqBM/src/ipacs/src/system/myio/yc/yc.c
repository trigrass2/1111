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
#include "ft.h"
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


#undef YCZERO_TEST    //遥测零漂测试
#define YCQ_TEST

#define AI_FREQ_VAL     100000  /*U 10V*/	

#if(TYPE_CPU == CPU_ZYNQ)
extern void axi_hp_set_timer_period(uint32 ns);
extern uint32 axi_hp_get_timer_period(void);

#define AI_FREQ_ARR     833333 /*0.833ms   24点采样*/
#define AI_FREQ_TIMER   (AI_FREQ_ARR*2000)
#endif

/*3.3*/
#define AI_DC_MIN       0x86E
#define AI_DC_MAX       0xCA5

//ad7607电压gain在 6062左右 (300V 3.53)
#define U_GAIN_MAX      14000 //

//ad7607电流gain在 3600 左右 (50A 7.07线圈)
#define I_GAIN_MAX      5000


VAiCfg *pAiCfg = NULL;
VYcCfg *pYcCfg = NULL;
VFdCfg *pFdCfg = NULL;

short nMeaBuf[YC_SAM_POINT_N][MAX_AI_NUM];
#ifdef YCQ_TEST
short nMeaFtBuf[MEA_SAM_POINT_N+3][MAX_AI_NUM];
#else
short nMeaFtBuf[FT_SAM_POINT_N+3][MAX_AI_NUM];
#endif

static VMeaValue meaValueTmp[MAX_AI_NUM];
VMeaValue meaValue[MAX_AI_NUM];

static int gainNum, aigainNum = 0;
VYcGain *pYcGain;
VYcGain *pMeaGain;


int gainValid=0; int gycInit=0;
DWORD ggainver = YC_GAIN_AD7616_VER; //0x80000000 ad7606 ; 0x80000001 ad7607
int g_wFreq = 5000;

#ifndef YC_GAIN_NEW
static int gainStep = 0;
#endif

static int rawPtr;
static long *rawYc;
#ifndef YC_GAIN_NEW
static long *tmpRawYc1, *tmpRawYc2;
#endif
static long *ycValue, *ycValueBak;     /*二次整型  
                                          电压 100V-10000
									      电流 5A->10000
                                          功率 5A,100V,45度  707106
                                      */ 
static struct VYCF_L *dbYcValue;  /*经过PTCT及比例换算后的整型值,写入数据库*/
static int harBgnNo, angleBgnNo,phBgnNo;




static int yccfgNum;
static VDefTVYcCfg *pDefTVYcCfg;

#ifdef YCQ_TEST
static VMmifft  *gQFft; 
#endif

#if (DEV_SP == DEV_SP_TTU)
static VMmifft  gSeqFft;
#endif

#if defined  (INCLUDE_DD) && !defined (INCLUDE_ATT7022)
static DWORD ddValueSem;
static VMeaDd meaDd[MAX_FD_NUM];     
static long dbDdValue[MAX_FD_NUM*4];   
static long rawDdValue[MAX_FD_NUM*4];   
static float ddValue[MAX_FD_NUM*4];   
static int ddResetFlag = 0;
static int gddInit = 0;
#endif

void meaGainGet(void);


static VMmiMeaTbl mmiMeaTbl[] = 
{
	{MMI_MEA_IA, "Ia", "A"},
	{MMI_MEA_IB, "Ib", "A"},
	{MMI_MEA_IC, "Ic", "A"},
	{MMI_MEA_I0, "I0", "A"},
	{MMI_MEA_UA, "Ua", "V"},
	{MMI_MEA_UB, "Ub", "V"},
	{MMI_MEA_UC, "Uc", "V"},
	{MMI_MEA_U0, "U0", "V"},
	{MMI_MEA_AIF,"Rr", "kΩ"},
};		

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
		    {
				break;
		    }
			pYcCfg[i].xs1 = pYcCfg[i].xs1/10;
			pYcCfg[i].xs2 = pYcCfg[i].xs2/10;
        }
		for (j=0; j<2; j++)           /*分母都尝试以5约*/ 
		{
			if((pYcCfg[i].xs1==0)||(pYcCfg[i].xs1%5)||(pYcCfg[i].xs2%5))
			{
				break;
			}
			pYcCfg[i].xs1 = pYcCfg[i].xs1/5;
			pYcCfg[i].xs2 = pYcCfg[i].xs2/5;
		}
	}
}

void rawValue(int gainNo, long value)
{
	long *raw;

	raw = rawYc+gainNo*YC_MEAN_NUM;

	raw[rawPtr] = value;
}

long sumValue(int gainNo)
{
    int k;
	long *raw, sum = 0;
#ifdef _TEST_VER_
  long max,min;
#endif	
	raw = rawYc+gainNo*YC_MEAN_NUM;

  sum = 0;
  
#ifdef _TEST_VER_
	min = raw[0];
	max = raw[YC_MEAN_NUM-1];
	for (k=0; k<YC_MEAN_NUM; k++)
	{
		sum += raw[k];
		if(raw[k] > max)
			max = raw[k];
		if(raw[k] < min)
			min = raw[k];
	}
	sum = (sum - max -min)*((float)YC_MEAN_NUM/(YC_MEAN_NUM-2));
#else
	for (k=0; k<YC_MEAN_NUM; k++)
		sum += raw[k];
#endif

	return (sum);
}

long avgValue(int gainNo)
{
    int k;
	long *raw, sum = 0;

	raw = rawYc+gainNo*YC_MEAN_NUM;

    sum = 0;
	for (k=0; k<YC_MEAN_NUM; k++)
		sum += raw[k];

	return (sum/YC_MEAN_NUM);
}

//120采样 每3个取一个 取成40
void meaDataLoad_Yc(BOOL flag)
{
	int i, j, ptr;
	int sum[MAX_AI_NUM];

	ptr = (SAM_BUF_LEN+wMeaSamCnt-YC_SAM_POINT_N*(AD_FT_FLAG >> 1)+1)&(SAM_BUF_LEN-1); 

    memset(sum, 0, MAX_AI_NUM*4);
	for (j=0; j<YC_SAM_POINT_N; j++)
	{
		for (i=0; i<aiNUM; i++)
		{
			nMeaBuf[j][i] = nMeaSam[ptr][i];
			if (j <MEA_SAM_POINT_N)
			   sum[i] += nMeaBuf[j][i];
		}	
		ptr = (ptr + (AD_FT_FLAG >> 1))&(SAM_BUF_LEN - 1);
	}

	if(flag == 0)
	{
		for (i=0; i<aiNUM; i++)
		{
		    sum[i] = sum[i]/MEA_SAM_POINT_N;
			for (j=0; j<YC_SAM_POINT_N; j++)
				nMeaBuf[j][i] -= sum[i];
		}
	}
}

#if 1
int meaCalc_freq(void)
{
	static short data[FREQ_SAM_POINT_N];
	register short a, b, c, d;
	register int i, pos, o, temp;
	DWORD t1=0, t2, arr;

	static DWORD freq_old = 50000;
	static DWORD freq_cnt = 0;
	
	//选第一个幅值大于10%的通道计算频率
	pos = 0xff;
	for (i=0; i<aiNUM; i++)
	{
		if (meaValue[i].lRms > AI_FREQ_VAL)
		{
			pos = i;
			break;
		}
	}
	if (pos == 0xff)
	{
		freq_cnt = 0;
		
		if( 50000 >= freq_old)
		       temp = 50000 - freq_old;
	        else
		       temp = freq_old - 50000;

		if(temp > 20)
		{
			 freq_old = 50000;
			 axi_hp_set_timer_period(AI_FREQ_ARR);
			 g_wFreq = 5000;
		}
		return 5000;
	}

	temp = (SAM_BUF_LEN+wMeaSamCnt-FREQ_SAM_POINT_N*(AD_FT_FLAG >> 1)+1)&(SAM_BUF_LEN-1); 

	//把13周波数据移到一起来，无需去零漂
	for (i=0; i<FREQ_SAM_POINT_N; i++)
	{
	   data[i] = nMeaSam[temp][pos];
	   temp = (temp+(AD_FT_FLAG >> 1))&(SAM_BUF_LEN - 1);
	}
			
	o = 0;
	for(i=1; i<FREQ_SAM_POINT_N; i++)//在5周波里搜索4个过零点
	{
		a = data[i];
		b = data[i+1];
		c = data[i+2];
		d = data[i+3];
	
		if( (a>b) && (b>=0) && (c<=0) && (c>d) )
		{
			o++;
			if( o == 1 )//计算t1=i*(TIM8->ARR+1)+t'
			{
				temp = 3*(axi_hp_get_timer_period() + 1);
				temp = ((temp*a)+((a-d)>>1))/(a-d);
				t1 = i*(axi_hp_get_timer_period() + 1)+temp;
			}
			if( o == 4 )//计算t2=i*(TIM1->ARR+1)+t'
			{
				temp = 3*(axi_hp_get_timer_period() + 1);
				temp = ((temp*a)+((a-d)>>1))/(a-d);
				t2 = i*(axi_hp_get_timer_period() + 1)+temp;
				//t2-t1=8周波时间
				t1 = ((t2-t1)+50)/100;
				t2 = (30LL*AI_FREQ_TIMER+(t1>>1))/t1;

				if( (t2 < 44000) || (t2 > 56000) )
				{
					freq_cnt = 0;
					return 5000;
				}

				if( t2 >= freq_old)
					temp = t2 - freq_old;
				else
					temp = freq_old - t2;

				freq_cnt++;
				if(temp < 20)
					freq_cnt = 0;
				
		        if ((temp > 20) && (freq_cnt > 2))
		        {
			        arr = (AI_FREQ_ARR*50000LL+(t2>>1))/t2 - 1;
			        freq_old = t2;
			        axi_hp_set_timer_period(arr);

			        freq_cnt = 0;
					g_wFreq = t2/10;
		        }

				return (t2/10);
			}
		}

	}
	return 5000;
}

int meaCalc_freqChanel(int pos)
{
	short data[FREQ_SAM_POINT_N];
	register short a, b, c, d;
	register int i, o, temp;
	DWORD t1=0, t2;


	if (meaValue[pos].lRms < AI_FREQ_VAL)
		return 5000;

	temp = (SAM_BUF_LEN+wMeaSamCnt-FREQ_SAM_POINT_N*(AD_FT_FLAG >> 1)+1)&(SAM_BUF_LEN-1); 

	//把13周波数据移到一起来，无需去零漂
	for (i=0; i<FREQ_SAM_POINT_N; i++)
	{
	   data[i] = nMeaSam[temp][pos];
	   temp = (temp+(AD_FT_FLAG >> 1))&(SAM_BUF_LEN - 1);
	}

		o = 0;
	for(i=1; i<FREQ_SAM_POINT_N; i++)//在5周波里搜索4个过零点
	{
		a = data[i];
		b = data[i+1];
		c = data[i+2];
		d = data[i+3];
		
		if( (a>b) && (b>=0) && (c<=0) && (c>d) )
		{
			o++;
			if( o == 1 )//计算t1=i*(TIM8->ARR+1)+t'
			{
				temp = 3*(axi_hp_get_timer_period()+1);
				temp = ((temp*a)+((a-d)>>1))/(a-d);
				t1 = i*(axi_hp_get_timer_period()+1)+temp;
			}
			if( o == 4 )//计算t2=i*(TIM1->ARR+1)+t'
			{
				temp = 3*(axi_hp_get_timer_period()+1);
				temp = ((temp*a)+((a-d)>>1))/(a-d);
				t2 = i*(axi_hp_get_timer_period()+1)+temp;
				//t2-t1=8周波时间
				t1 = ((t2-t1)+50)/100;
				t2 = (30LL*AI_FREQ_TIMER+(t1>>1))/t1;
				if( (t2 < 44000) || (t2 > 56000) )
					return 5000;

				return (t2/10);
			}
		}
	}
	
	return 5000;

}
#else

int meaCal_freq(int no)
{
    int i,ptr,dots,result;
	short f_cnt1,f_cnt2,f_zero_cnt;
    short nMea3Buf[FREQ_SAM_POINT_N];
	long data,f_correct1, f_correct2, diff;


	if(meaValue[no].lRms < U_FREQ_10V)
		return 0;


	ptr = (SAM_BUF_LEN+wMeaSamCnt-FREQ_SAM_POINT_N+1)&(SAM_BUF_LEN-1); 

	for(i=0; i<FREQ_SAM_POINT_N; i++)
	{
	    nMea3Buf[i] = nMeaSam[ptr][no];
		ptr = (ptr+1)&(SAM_BUF_LEN - 1);
	}
	f_cnt1 = f_cnt2 = 0;
	f_zero_cnt = f_correct1 = f_correct2 = 0;
	dots = result = 0;

    for(i=1; i<FREQ_SAM_POINT_N; i++)
    {
        if((nMea3Buf[i-1] < 0)&&(nMea3Buf[i] >= 0)&&((i-f_cnt1)>3))
        {
		   f_zero_cnt++;
		   if(f_zero_cnt == 1)
		   {
		      data = nMea3Buf[i];
			  diff = nMea3Buf[i] - nMea3Buf[i-1];
			  if(diff != 0)
		         f_correct1 = (data<<8)/diff;
			  else
			  	 f_correct1 = 0;
			   f_cnt1 = i;
		   }
		   else if(f_zero_cnt == 2)
		   {
		      data = nMea3Buf[i];
			  diff = nMea3Buf[i] - nMea3Buf[i-1];
			  if(diff != 0)
		         f_correct2 = (data<<8)/diff;
			  else
			  	 f_correct2 = 0;
			  f_cnt2 = i;
			  
			  dots = (long)((f_cnt2 - f_cnt1)<<8)+f_correct1 - f_correct2;
			  if(dots != 0)
			     result = ((50 * MEA_SAM_POINT_N * 100)<<8) /dots;
			  else
			  	 result = 0;
			  break;
		   }
        }
    }

	return result;
}
#endif

void meaCal_Yc(void)
{
    volatile int i, j, k, n1, n2, n3, gainNo, g1, g2, g3, ycNo,p,q;
	short temp;
	long *raw, a, result;
	long long longlongs;
#ifndef YCZERO_TEST
    long tlong;
#endif

	meaDataLoad_Yc(0);

#if 0

	shellprintf("\nUa***************\r\n");
	for (i= 0; i<5; i++)
	{
		for (j= 0; j<8; j++)
		{
			shellprintf("%d	 ", nMeaBuf[i*8+j][0]);
		}
	   shellprintf("\r\n");

	}	

#endif
#if 0

	shellprintf("\nUc***************\n");
	for (i=0; i<5; i++)
	{
		for (j= 0; j<8; j++)
		{
			shellprintf("%d	 ", nMeaBuf[i*8+j][6]);
		}	
		shellprintf("\n");
	}	

	shellprintf("\nIa***************\n");
	for (i= 0; i<5; i++)
	{
		for (j= 0; j<8; j++)
		{
			shellprintf("%d	 ", nMeaBuf[i*8+j][1]);
		}
	   shellprintf("\n");

	}	
#endif
		
	

	for (i=0; i<gainNum; i++)
	{
		ycNo = pYcGain[i].ycNo;
		raw = rawYc+i*YC_MEAN_NUM;

		result = 0;
		switch (pYcCfg[ycNo].type)
		{
			case YC_TYPE_Ua:
			case YC_TYPE_Ub:
			case YC_TYPE_Uc:
			case YC_TYPE_U0:
			case YC_TYPE_Ia:
			case YC_TYPE_Ib:
			case YC_TYPE_Ic:
			case YC_TYPE_I0:
				//arg1表示对应的AI通道
				n1 = pYcCfg[ycNo].arg1;				
				for (j=0; j<MEA_SAM_POINT_N; j++)
					result += (nMeaBuf[j][n1]*nMeaBuf[j][n1]);
				
				result = Sqrt_Dword(result);
				
				raw[rawPtr] = result;
                break;
			case YC_TYPE_Pa:
			case YC_TYPE_Pb:
			case YC_TYPE_Pc:
				//arg1和arg2分别表示对应运算的YC通道
				if (pYcCfg[ycNo].cal)
				{
					n1 = pYcCfg[pYcCfg[ycNo].arg1].arg1;
					n2 = pYcCfg[pYcCfg[ycNo].arg2].arg1;
					for (j=0; j<MEA_SAM_POINT_N; j++)
		                result += (nMeaBuf[j][n1]*nMeaBuf[j][n2]);

					//shellprintf("P, %d\n", result);
	               // result >>= YC_PQ_RAW_SHIFT_NUM;  
					raw[rawPtr] = result;
				}	
				break;
			case YC_TYPE_Qa:
			case YC_TYPE_Qb:
			case YC_TYPE_Qc:
				//arg1和arg2分别表示对应运算的YC通道
				if (pYcCfg[ycNo].cal)
				{
	       #ifdef YC_GAIN_NEW
		   #ifdef YCQ_TEST
		            n1 = pYcCfg[pYcCfg[ycNo].arg1].arg1;
					n2 = pYcCfg[pYcCfg[ycNo].arg2].arg1;
					for (j=0; j<13; j++)
					{
					    result += (gQFft[j].aiftVal[n1].i*gQFft[j].aiftVal[n2].r - gQFft[j].aiftVal[n1].r*gQFft[j].aiftVal[n2].i)*5/4;
					}

					raw[rawPtr] = result;
		   #else
		            n1 = pYcCfg[pYcCfg[ycNo].arg1].arg1;
					n2 = pYcCfg[pYcCfg[ycNo].arg2].arg1;
					for (j=0; j<MEA_SAM_POINT_N; j++)
						result += (nMeaBuf[j][n1]*nMeaBuf[j+10][n2]);

                
					raw[rawPtr] = result;
		   #endif
		   #else
					n1 = pYcCfg[pYcCfg[ycNo].arg1].arg1;
					n2 = pYcCfg[pYcCfg[ycNo].arg2].arg1;									
					result += nMeaBuf[0][n2]*(nMeaBuf[MEA_SAM_POINT_N-1][n1]-nMeaBuf[1][n1]);
					for (j=1; j<MEA_SAM_POINT_N-1; j++)
						result += nMeaBuf[j][n2]*(nMeaBuf[j-1][n1]-nMeaBuf[j+1][n1]);
					result += nMeaBuf[MEA_SAM_POINT_N-1][n2]*(nMeaBuf[MEA_SAM_POINT_N-2][n1]-nMeaBuf[0][n1]);

	                result >>= YC_PQ_RAW_SHIFT_NUM;  
					raw[rawPtr] = result;
			#endif
				}	
				break;	
		}	
	}

	for (i=0; i<ycNum; i++)
	{
		gainNo = pYcCfg[i].gainNo;

		result = 0;
		switch (pYcCfg[i].type)
		{
			case YC_TYPE_Dc:
				n1 = pYcCfg[i].arg1;				
				for (j=0; j<MEA_SAM_POINT_N; j++)
					result += nMeaBuf[j][n1];
				
				result = result*1000/MEA_SAM_POINT_N/5101+398; 
				
#ifndef YCZERO_TEST
				tlong = (result < 0) ? -result:result;
				if (tlong < pYcCfg[i].toZero)
					ycValue[i] = 0;
				else
#endif					
					ycValue[i] = result;				
				break;				
			case YC_TYPE_Ua:
			case YC_TYPE_Ub:
			case YC_TYPE_Uc:
			case YC_TYPE_U0:
				result = sumValue(gainNo)*10000/YC_MEAN_NUM/pYcGain[gainNo].a; 				

#ifndef YCZERO_TEST
				tlong = (result < 0) ? -result:result;
				if (tlong < pYcCfg[i].toZero)
					ycValue[i] = 0;
				else
#endif					
					ycValue[i] = result;
				break;
			case YC_TYPE_Ia:
			case YC_TYPE_Ib:
			case YC_TYPE_Ic:
			case YC_TYPE_I0:
				result = sumValue(gainNo)*10000/YC_MEAN_NUM/pYcGain[gainNo].a; 
				
#ifndef YCZERO_TEST
				tlong = (result < 0) ? -result:result;
				if (tlong < pYcCfg[i].toZero)
					ycValue[i] = 0;
				else
#endif					
					ycValue[i] = result;
				
				break;
			case YC_TYPE_SU0:
			case YC_TYPE_SI0:
				//arg1表示运算起始项对应的YC通道
				if (pYcCfg[i].cal) 
				{
					n1 = pYcCfg[pYcCfg[i].arg1].arg1;
					n2 = pYcCfg[pYcCfg[i].arg1+1].arg1;
					n3 = pYcCfg[pYcCfg[i].arg1+2].arg1;
					for (j=0; j<MEA_SAM_POINT_N; j++)
					{
						//temp = (nMeaBuf[j][n1]+nMeaBuf[j][n2]+nMeaBuf[j][n3])/3;
						//3I0 3U0
						temp = nMeaBuf[j][n1]+nMeaBuf[j][n2]+nMeaBuf[j][n3];
						result += temp*temp;
					}
					result = Sqrt_Dword(result);

					raw = rawYc+gainNo*YC_MEAN_NUM;						
					raw[rawPtr] = result;

					g1 = pYcCfg[pYcCfg[i].arg1].gainNo;
					g2 = pYcCfg[pYcCfg[i].arg1+1].gainNo;				
					g3 = pYcCfg[pYcCfg[i].arg1+2].gainNo;	
					a = (pYcGain[g1].a+pYcGain[g2].a+pYcGain[g3].a)/3;
					
					result = sumValue(gainNo)*10000/YC_MEAN_NUM/a; 			
#ifndef YCZERO_TEST
					tlong = (result < 0) ? -result:result;
					if (tlong < pYcCfg[i].toZero)
						ycValue[i] = 0;
					else
#endif						
						ycValue[i] = result;
				}	
                break;
			case YC_TYPE_Uab:
			case YC_TYPE_Ubc:
			case YC_TYPE_Uca:
				//arg1和arg2分别表示对应运算的YC通道
				if (pYcCfg[i].cal)
				{
					n1 = pYcCfg[pYcCfg[i].arg1].arg1;
					n2 = pYcCfg[pYcCfg[i].arg2].arg1;				
					for (j=0; j<MEA_SAM_POINT_N; j++)
					{
						temp = nMeaBuf[j][n1]-nMeaBuf[j][n2];
						result += temp*temp;
					}
					result = Sqrt_Dword(result);
					
					raw = rawYc+gainNo*YC_MEAN_NUM;						
					raw[rawPtr] = result;

					g1 = pYcCfg[pYcCfg[i].arg1].gainNo;
					g2 = pYcCfg[pYcCfg[i].arg2].gainNo;	
					a = (pYcGain[g1].a+pYcGain[g2].a)/2;				
					result = sumValue(gainNo)*10000/YC_MEAN_NUM/a; 

#ifndef YCZERO_TEST					
					tlong = (result < 0) ? -result:result;
					if (tlong < pYcCfg[i].toZero)
						ycValue[i] = 0;
					else
#endif						
						ycValue[i] = result;
				
				}	
				break;
			case YC_TYPE_SUa:
			case YC_TYPE_SUb:
			case YC_TYPE_SUc:
			case YC_TYPE_SIa:
			case YC_TYPE_SIb:
			case YC_TYPE_SIc:
				//arg1和arg2分别表示对应运算的YC通道
				if (pYcCfg[i].cal)
				{
					n1 = pYcCfg[pYcCfg[i].arg1].arg1;
					n2 = pYcCfg[pYcCfg[i].arg2].arg1;
					n3 = pYcCfg[pYcCfg[i].arg2+1].arg1;
					for (j=0; j<MEA_SAM_POINT_N; j++)
					{
					    k = (j + FT_SAM_POINT_N)%MEA_SAM_POINT_N;
						temp = nMeaBuf[j][n1]+nMeaBuf[j][n2]+nMeaBuf[k][n3];
						result += temp*temp;
					}
					result = Sqrt_Dword(result);
					
					raw = rawYc+gainNo*YC_MEAN_NUM; 					
					raw[rawPtr] = result;

					g1 = pYcCfg[pYcCfg[i].arg1].gainNo;
					g2 = pYcCfg[pYcCfg[i].arg2].gainNo; 
					g3 = pYcCfg[pYcCfg[i].arg2+1].gainNo;
					a = (pYcGain[g1].a+pYcGain[g2].a+pYcGain[g3].a)/3;
				
					result = sumValue(gainNo)*10000/YC_MEAN_NUM/a; 

#ifndef YCZERO_TEST					
					tlong = (result < 0) ? -result:result;
					if (tlong < pYcCfg[i].toZero)
						ycValue[i] = 0;
					else
#endif						
						ycValue[i] = result;
				}	
				break;
			case YC_TYPE_Pa:
			case YC_TYPE_Pb:
			case YC_TYPE_Pc:
				//arg1和arg2分别表示对应运算的YC通道
				if (pYcCfg[i].cal)
				{
					n1 = pYcCfg[i].arg1;
					n2 = pYcCfg[i].arg2;
					if ((ycValue[n1]==0) || (ycValue[n2]==0))
						ycValue[i] = 0;
					else
					{
		       #ifdef YC_GAIN_NEW
			            ycValue[i] =  (long long)avgValue(gainNo)*1000/pYcGain[gainNo].a;
			   #else
						g1 = pYcCfg[pYcCfg[i].pair].gainNo;
						ycValue[i] = (((long long)avgValue(gainNo)*pYcGain[gainNo].a+(long long)avgValue(g1)*pYcGain[gainNo].b))>>YC_PQ_GAIN_SHIFT_NUM;
               #endif
					}	
				}	
				break;
			case YC_TYPE_Qa:
			case YC_TYPE_Qb:
			case YC_TYPE_Qc:
				//arg1和arg2分别表示对应运算的YC通道
				if (pYcCfg[i].cal)
				{
				    n1 = pYcCfg[i].arg1;
					n2 = pYcCfg[i].arg2;
					if ((ycValue[n1]==0) || (ycValue[n2]==0))
						ycValue[i] = 0;		
					else
					{
				#ifdef YC_GAIN_NEW
			            ycValue[i] =  (long long)avgValue(gainNo)*1000/pYcGain[gainNo].a;
				        g1 = pYcCfg[i].pair;
						p = ycValue[g1] - ycValue[i]*pYcGain[gainNo].b/10000;
						q = ycValue[i] + ycValue[g1]*pYcGain[gainNo].b/10000;
						ycValue[g1] = p;
						ycValue[i] = q;
			    #else
						g1 = pYcCfg[pYcCfg[i].pair].gainNo;
						ycValue[i] = (((long long)avgValue(g1)*pYcGain[gainNo].a+(long long)avgValue(gainNo)*pYcGain[gainNo].b))>>YC_PQ_GAIN_SHIFT_NUM;
                #endif
					}	
				}
				break;	
			case YC_TYPE_P:
			case YC_TYPE_Q:
				//arg1表示第一分项的yc号；arg2为2或3，分别用于二表或三表
				n1 = pYcCfg[i].arg1;				
				n2 = pYcCfg[i].arg2;
				for (j=n1; j<n1+n2; j++)
					result += ycValue[j];
				ycValue[i] = result;				
				break;
			case YC_TYPE_S:
				//arg1为P遥测号；arg2为Q遥测号
				n1 = pYcCfg[i].arg1;				
				n2 = pYcCfg[i].arg2;
				longlongs = (long long)ycValue[n1]*ycValue[n1] + (long long)ycValue[n2]*ycValue[n2];

				j=0;
				result = longlongs>>32;	
				if (result != 0) 
				{
					while ( (result&0x80000000)==0 )
					{
						j++;
						result <<= 1;
					}
					j = 32-j;
					if (j&0x01) j++;
				}	
				
				result = longlongs>>j;								
				result = Sqrt_Dword(result);
				ycValue[i] = result<<(j>>1);			
				break;
			case YC_TYPE_Cos:
				//arg1为P遥测号；arg2为S遥测号 
				n1 = pYcCfg[i].arg1;				
				n2 = pYcCfg[i].arg2;
				if (ycValue[n2] != 0)
					ycValue[i] = ycValue[n1] *1000LL /ycValue[n2];
				else
					ycValue[i] = 0;
				break;
			case YC_TYPE_Freq:
				ycValue[i] = meaCalc_freqChanel(pYcCfg[pYcCfg[i].arg1].arg1);
				break;	
            case YC_TYPE_SFreq:
				ycValue[i] = meaCalc_freq();
				break;
			case YC_TYPE_Udiff:
				n1 = pYcCfg[i].arg1;
				n2 = pYcCfg[i].arg2;
				ycValue[i] = ycValue[n1] - ycValue[n2];
				if (ycValue[i] < 0)
					ycValue[i] = -ycValue[i];
				break;
			case YC_TYPE_Fdiff:
				n1 = pYcCfg[i].arg1;
				n2 = pYcCfg[i].arg2;
				ycValue[i] = ycValue[n1] - ycValue[n2];
				if (ycValue[i] < 0)
					ycValue[i] = -ycValue[i];
				break;
			case YC_TYPE_SIGNAL:
				ycValue[i] = SignalVal;
			  	break;
#ifdef INCLUDE_AI_HTM
            case YC_TYPE_TEMETER:
				ycValue[i] = aiHtmTGet();
				break;
            case YC_TYPE_HYMETER:
				ycValue[i] = aiHtmHGet();
				break;
#endif
#if (DEV_SP == DEV_SP_DTU_IU)
            case YC_TYPE_TEMETER:
				ycValue[i] = ReadTemp()*100;
				break;
            case YC_TYPE_AttUa:
            case YC_TYPE_AttUb:
            case YC_TYPE_AttUc:
            case YC_TYPE_AttU0:
                Real_U(pYcCfg[i].type - YC_TYPE_AttUa , &result);
                if(pYcCfg[i].type == YC_TYPE_AttU0)
                    result = result/3;					
                ycValue[i] = result;
                break;
			case YC_TYPE_AttIa:
			case YC_TYPE_AttIb:
			case YC_TYPE_AttIc:
			case YC_TYPE_AttI0:
                Real_I(pYcCfg[i].type - YC_TYPE_AttIa, &result);
				JudgeIZone(result, RATED_CURR_VALUE*1000, pYcCfg[i].type - YC_TYPE_AttIa);
				if(pYcCfg[i].type == YC_TYPE_AttI0)
					result = result/3;				
			    ycValue[i] = result;				
				break;            
			case YC_TYPE_AttPa:
			case YC_TYPE_AttPb:
			case YC_TYPE_AttPc:
                Real_PQ(pYcCfg[i].type - YC_TYPE_AttPa , &ycValue[i]);
				break;
			case YC_TYPE_AttQa:
			case YC_TYPE_AttQb:
			case YC_TYPE_AttQc:
				Real_PQ(pYcCfg[i].type - YC_TYPE_AttQa +4 , &ycValue[i]);
				break;  
            case YC_TYPE_AttP:
			case YC_TYPE_AttQ:
				//arg1表示第一分项的yc号；arg2为2或3，分别用于二表或三表
				if(pYcCfg[i].type == YC_TYPE_AttP)
					Real_PQ(pYcCfg[i].type - YC_TYPE_AttP +3 , &ycValue[i]);
				if(pYcCfg[i].type == YC_TYPE_AttQ)
					Real_PQ(pYcCfg[i].type - YC_TYPE_AttQ +7 , &ycValue[i]);
				break;
#endif

#ifdef INCLUDE_PB_RELATE
            case YC_TYPE_Uunb:
				ycValue[i] = pbGetUnBlanceU(pYcCfg[i].fdNo);
				break;
			case YC_TYPE_Iunb:
				ycValue[i] = pbGetUnBlanceI(pYcCfg[i].fdNo);
				break;
			case YC_TYPE_Upp:
				ycValue[i] = pbGetPerPassU(pYcCfg[i].fdNo,pYcCfg[i].arg1);
				break;
#endif
#ifdef INCLUDE_PR
			case  YC_TYPE_I0T:
				ycValue[i] = ReadIVIT(0,0);
				break;
			case  YC_TYPE_I0V:
				ycValue[i] = ReadIVIT(1,0);
				break;
			case YC_TYPE_I1T:
				ycValue[i] = ReadIVIT(2,0);
				break;
			case YC_TYPE_I1V:
			  	ycValue[i] = ReadIVIT(3,0);
			  	break;
#if (TYPE_USER  == USER_FUJIAN)
			case YC_TYPE_I1A:
				ycValue[i] = ReadIVIT(4,0);
			  	break;
			case YC_TYPE_I2A:
				ycValue[i] = ReadIVIT(5,0);
			  	break;
			case YC_TYPE_I2T:
				ycValue[i] = ReadIVIT(6,0);
			  	break;
			case YC_TYPE_I3A:
				ycValue[i] = ReadIVIT(7,0);
			  	break;
			case YC_TYPE_I3T:
				ycValue[i] = ReadIVIT(8,0);
			  	break;
			case YC_TYPE_I0A:
				ycValue[i] = ReadIVIT(9,0);
			  	break;
			case YC_TYPE_CHT:
				ycValue[i] = ReadIVIT(10,0);
			  	break;
			case YC_TYPE_CHFDT:
				ycValue[i] = ReadIVIT(11,0);
				break;
			case YC_TYPE_GYV:
				ycValue[i] = ReadIVIT(12,0);
			  	break;
			case YC_TYPE_GYT:
				ycValue[i] = ReadIVIT(13,0);
			  	break;
			case YC_TYPE_GPHZ:
				ycValue[i] = ReadIVIT(14,0);
			  	break;
			case YC_TYPE_GPT:
				ycValue[i] = ReadIVIT(15,0);
			  	break;
#endif			
#endif
			default:
				break;
		}	
	}

	rawPtr = (rawPtr+1)&(YC_MEAN_NUM-1);
}

void meaWrite_DbValue(void)
{
    int i;

	memcpy(ycValueBak, ycValue, sizeof(long)*ycNum);

	for (i=0; i<ycNum; i++)
	{
#if (DEV_SP == DEV_SP_TTU)
		dbYcValue[i].lValue = ycValue[i] ;	
#else
		dbYcValue[i].lValue = ycValue[i]*pYcCfg[i].xs1/pYcCfg[i].xs2; 
#endif

#ifdef _TEST_VER_
		if ((pYcCfg[i].type>=YC_TYPE_Ua) && (pYcCfg[i].type<=YC_TYPE_Uc))
		{
			if (ycValue[i] >= (12000*pFdCfg[pYcCfg[i].fdNo].Un/100))         //120%
				dbYcValue[i].byFlag = 0x11;  //overflow
			else
				dbYcValue[i].byFlag = 0x01;
		}	
		else if ((pYcCfg[i].type>=YC_TYPE_Ia) && (pYcCfg[i].type<=YC_TYPE_Ic))
		{
			if (ycValue[i] >= 12000)         //120%
				dbYcValue[i].byFlag = 0x11;  //overflow
			else
				dbYcValue[i].byFlag = 0x01;
		}			
		else
			dbYcValue[i].byFlag = 0x01;
#else
		dbYcValue[i].byFlag = 0x01;
#endif
		WriteRangeYCF_L(BM_EQP_MYIO, i, dbYcValue[i].lValue);	
	}   
}

void meaDataLoad_ft(void) 
{
	int i,j,ptr;
	int sum[MAX_AI_NUM];
	int wave_point;

	memset(sum, 0, MAX_AI_NUM*4);

#ifdef YCQ_TEST
    wave_point = MEA_SAM_POINT_N;
    ptr = (SAM_BUF_LEN+wMeaSamCnt-(wave_point+3+1)*(AD_FT_FLAG >> 1))&(SAM_BUF_LEN-1);	

	for (j=0; j<(wave_point+3); j++)
	{
		for (i=0; i<aiNUM; i++)
		{
			nMeaFtBuf[j][i] = nMeaSam[ptr][i];
			if (j < wave_point)
			   sum[i] += nMeaFtBuf[j][i];
		}	
		
		ptr = (ptr+(AD_FT_FLAG >> 1))&(SAM_BUF_LEN-1);
	}
#else
    wave_point = FT_SAM_POINT_N;
	ptr = (SAMFT_BUF_LEN+wFtSamCnt-(wave_point+3+1))&(SAMFT_BUF_LEN-1);	

	for (j=0; j<(wave_point+3); j++)
	{
		for (i=0; i<aiNUM; i++)
		{
			nMeaFtBuf[j][i] = nFtSam[ptr][i];
			if (j < wave_point)
			   sum[i] += nMeaFtBuf[j][i];
		}	
		ptr = (ptr+1)&(SAMFT_BUF_LEN-1);
	}
#endif

	for (i=0; i<aiNUM; i++)
	{
	    sum[i] = sum[i]/wave_point;
		for (j=0; j<(wave_point+3); j++)
			nMeaFtBuf[j][i] -= sum[i];
	}
}

#if 0
void meaCal_ft(int ai_chn, int wave_num, DWORD *result)
{    
	int i,index;
	long lCos;   //实部
	long lSin;   //虚部
	long lSam;
	const long *pFAFre, *pFAFim;

    result[0] = result[1] = 0;    /*result[0]  rms
                                    result[1]  ang*/											
	if ((wave_num==0)  || (wave_num>9))  return;								
	if (ai_chn >= aiNUM) return;

	 if (wFtFreq >= FREQ_MAX)
		index = FREQ_FOLLOW_NUM-1;
	else if (wFtFreq <= FREQ_MIN)
		index = 0;
	else
		index = (wFtFreq - FREQ_MIN + FREQ_STEP/2)/FREQ_STEP;
	pFAFre = &(laFactor[index].laFreqFre[0]);
	pFAFim = &(laFactor[index].laFreqFim[0]);

	lCos=0;lSin=0;		
	for(i=0; i<laFactor[index].samples; i++)
	{
		lSam=nMeaFtBuf[i+FT_DIFF][ai_chn]-nMeaFtBuf[i][ai_chn];
		lCos+=lSam*pFAFre[i];
		lSin+=lSam*pFAFim[i];

		//if (wave_num > 1) printf("\n%ld:%d\n", pFAFre[i], pFAFim[i]);
	}

	//if (wave_num > 1) printf("\nlCos--%ld:lSin%d\n", lCos, lSin);

	lCos >>=7;
	lSin >>=7;
	result[0] = AmXY(lCos,lSin);
	if (wave_num == 1) result[1] = (DWORD)AngPQ(lCos,lSin,result[0]);
}
#else
void meaCal_ft(int ai_chn, int wave_num, DWORD *result)
{    
	int i;
	long lCos;   //实部
	long lSin;   //虚部
	long lSam;
	const long *pFAFre, *pFAFim;

    int wave_max, wave_point; 

    result[0] = result[1] = 0;    /*result[0]  rms
                                    result[1]  ang*/	

#ifdef YCQ_TEST
    wave_max = 13;
    wave_point = MEA_SAM_POINT_N;
#else
    wave_max = 9;
    wave_point = FT_SAM_POINT_N;
#endif
	if ((wave_num==0)  || (wave_num>wave_max))  return;								
	if (ai_chn >= aiNUM) return;

#ifdef YCQ_TEST
    pFAFre = &laFAFre2[wave_num-1][0];	
    pFAFim = &laFAFim2[wave_num-1][0];
#else
	pFAFre = &laFAFre[wave_num-1][0];	
    pFAFim = &laFAFim[wave_num-1][0];
#endif

	lCos=0;lSin=0;		
	for(i=0; i<wave_point; i++)
	{
		lSam=nMeaFtBuf[i+FT_DIFF][ai_chn]-nMeaFtBuf[i][ai_chn];
		lCos+=lSam*pFAFre[i];
		lSin+=lSam*pFAFim[i];

		//if (wave_num > 1) printf("\n%ld:%d\n", pFAFre[i], pFAFim[i]);
	}

	//if (ai_chn == 5) printf("\nlCos--%ld:lSin%d\n", lCos, lSin);

	lCos >>=7;
	lSin >>=7;
	result[0] = AmXY(lCos,lSin);
	if (wave_num == 1)
	{
	   result[1] = (DWORD)AngPQ(lCos,lSin,result[0]);
	#if (DEV_SP == DEV_SP_TTU)
	   gSeqFft.aiftVal[ai_chn].r = lCos;
	   gSeqFft.aiftVal[ai_chn].i = lSin;
	#endif
	}

	#ifdef YCQ_TEST
       gQFft[wave_num-1].aiftVal[ai_chn].r = lCos>>6;
	   gQFft[wave_num-1].aiftVal[ai_chn].i = lSin>>6;
    #endif
}
#endif

void meaWrite_DbAngle(void)
{
    int i;
	long angle;

	for (i=angleBgnNo; i<ycNum; i++)
	{ 
		if (pYcCfg[i].type != YC_TYPE_Angle)  continue;

		if ((dbYcValue[pYcCfg[i].arg2].lValue < 500)||(dbYcValue[pYcCfg[i].arg1].lValue < 500))
		{
		    ycValue[i] = 0;
		    continue;
		}

		//YC_TYPE_Angle：arg1和arg2为求角度的两个YC通道号
		angle = meaValue[pYcCfg[pYcCfg[i].arg2].arg1].lAng - meaValue[pYcCfg[pYcCfg[i].arg1].arg1].lAng;        
        //规格化-180-180
		if (angle < -11796480)
		{
			angle += 23592960;
		}
		else if (angle > 11796480)
		{
			angle -= 23592960;
		}

		ycValue[i] = (angle*100)>>16;
	}
}

void meaWrite_DbHar(int ai_chn, int yc_chn, DWORD *result)
{
#ifndef YCZERO_TEST
    long tlong;
#endif
    long value;

	value = result[0]*10000LL/pMeaGain[ai_chn].a;
	
#ifndef YCZERO_TEST
	tlong = (value < 0) ? -value:value;
	if (tlong < pYcCfg[yc_chn].toZero)
		ycValue[yc_chn] = 0;
	else
#endif					
		ycValue[yc_chn] = value;
}

void meaCal_Thd(int ai_chn, int yc_chn, int num, DWORD *result)
{
    int i;
	long har1;
	long long sum;

	sum = 0LL;

	for(i=0; i<num; i++)
	{
	   sum += ((long long)ycValue[yc_chn+i])*ycValue[yc_chn+i];
	}
    if(sum & 0xFFFFFFFF00000000LL)
	{
	   sum >>= YC_PQ_RAW_SHIFT_NUM;
	   sum >>= YC_PQ_RAW_SHIFT_NUM;
	   sum = Sqrt_Dword((DWORD)sum);
	   sum <<= YC_PQ_RAW_SHIFT_NUM;
    }
	else
	   sum = Sqrt_Dword((DWORD)sum);
	har1 = meaValue[ai_chn].lRms*10000LL/pMeaGain[ai_chn].a;
	*result = sum*1000*100/har1;
}

#if defined  (INCLUDE_DD) && !defined (INCLUDE_ATT7022)
void meaCal_Dd_1SInt(void)
{
	FAST int i;
	FAST long value;
	FAST VMeaDd *pMeaDd;

	//*M5275_GPIO_PCLRR_UARTH(SIM_BASE) = 0xFE;

	if ((gddInit==0) || (ddResetFlag==1))  return;

	//runLight_turn();
	//MCF_GPIO_PORTNQ ^= 0x80;

	pMeaDd = meaDd;
	for (i=0; i<fdNum; i++)
	{
		if (pMeaDd->p_id >= 0)
		{
			value = ycValue[pMeaDd->p_id];
			if (value >= 0)
				rawDdValue[pMeaDd->no[0]] += value;
			else
				rawDdValue[pMeaDd->no[2]] += -value;				
		}
		if (pMeaDd->q_id >= 0)
		{
			value = ycValue[pMeaDd->q_id];
			if (value >= 0)
				rawDdValue[pMeaDd->no[1]] += value;
			else
				rawDdValue[pMeaDd->no[3]] += -value;				
		}

		pMeaDd++;
	}
	
	//*M5275_GPIO_PPDSDR_UARTH(SIM_BASE) = 0x01;
}

void meaCal_Dd(void)
{
	static int cnt = 0;
	int i, write;
#ifdef INCLUDE_B2F
	VB2FSocket socket;
#endif
	if (gddInit == 0) return;

    if (ddResetFlag)
    {
		cnt = 0;
		for (i=0; i<ddNum; i++)
	    {
			rawDdValue[i] = 0;
			ddValue[i] = 0;
		}
		write = 1;
		ddResetFlag = 0;
	}
	else
	{
		cnt++;
		if (cnt < 2) return;
		cnt = 0;

		write = 0;
	    for (i=0; i<ddNum; i++)
	    {
	        //1s电度为0.01时  P/3600=0.01
	        //P=0.01*3600=36
	        //raw*0.0000005=P
	        //raw=36/0.0000005=72000000
			while (rawDdValue[i] >= 72000000) 
			{
				rawDdValue[i] -= 72000000;
				ddValue[i] += 0.01f;
				write = 1;
			}
		}
	}	
	
	if (write)
	{
		for (i=0; i<ddNum; i++)
			dbDdValue[i] = ddValue[i]*100;

		WriteRangeDD(g_Sys.wIOEqpID, 0, ddNum, dbDdValue);	

#ifdef INCLUDE_B2F
		socket.read_write = 2;
		strcpy(socket.fname, "EXTNV");
		socket.offset = NVRAM_AD_DD+sizeof(DWORD);
		socket.buf = (BYTE *)ddValue;
		socket.len = ddNum*sizeof(ddValue[0]);
		Buf2FileWrite(&socket);
#endif
	}	
}

int meaRead_DbDdValue(int beginNo, int num, long *buf)
{    
    if (beginNo >= ycNum) return 0;
	if ((beginNo+num) > ddNum)  num = ddNum-beginNo;

    memcpy(buf, dbDdValue+beginNo, num*sizeof(long));	

	return num;
}
#endif

void measure(void)
{
	DWORD result[2];
	//static int yc_cnt = 0;
	static int harcal = 0;
    static int ai_chn = 0;
	static int yc_chn;
	static int num = MAX_AI_NUM;
	static int har_num[MAX_AI_NUM]={0};
	int i, n, t;
#ifdef YCQ_TEST
    static int har_seq = 1;
    static int har_ai = 0;
#endif

	if(gycInit == 0)
		return;

	meaCal_Yc();

 	if ((ai_chn == 0) && (harcal == 0))
		meaDataLoad_ft();
    
    if (harcal == 0)
   	{
		while (pAiCfg[ai_chn].type==YC_TYPE_Null)
		{
			ai_chn++;
			if (ai_chn == aiNUM) break;
		}

		i = 0;
	    while (ai_chn < aiNUM)
		{

			meaCal_ft(ai_chn, 1, result);
			
			meaValueTmp[ai_chn].lRms = (long)result[0];
			meaValueTmp[ai_chn].lAng = (long)result[1];

			ai_chn++;	
			i++;
	#ifndef YCQ_TEST
			if (i >= num) break;
	#endif
		}	

		for (i = 0 ; i < ycNum ; i++ )
		{ 
			if (pYcCfg[i].type != YC_TYPE_Angle)  continue;
			
		}		

        if (ai_chn == aiNUM)
		{            
			num = (aiNUM+2)/3;

			memcpy(meaValue, meaValueTmp, sizeof(meaValue));
			
			if (angleBgnNo >= 0) meaWrite_DbAngle();

			if (harBgnNo >= 0)
			{
				harcal = 1;
				yc_chn = harBgnNo;
				memset(har_num, 0, sizeof(har_num));
			}
			ai_chn = 0;
		}
    }	

#ifdef YCQ_TEST  /*2,3,4,5,6,7,8,9, 10, 11, 12, 13*/
    har_seq += 1; 
    har_ai = 0;

	if (har_seq > 13)
		har_seq = 2;
    
    while (har_ai < aiNUM)
	{
	    meaCal_ft(har_ai, har_seq, result);
		har_ai++ ;
    }
#endif
	if (harcal)
	{
	   	while (pYcCfg[yc_chn].type<YC_TYPE_Har2)
		{
			yc_chn++;
			if (yc_chn >= ycNum) break;
	   	}	

	
        if (pYcCfg[yc_chn].type<YC_TYPE_Thd)
        {
            //YC_TYPE_Har：arg1为求谐波的YC通道号
			n = pYcCfg[pYcCfg[yc_chn].arg1].arg1;
			t = pYcCfg[yc_chn].type;	     			
			meaCal_ft(n, t-YC_TYPE_Har2+2, result);			
			meaWrite_DbHar(n, yc_chn, result);	
			if(pYcCfg[yc_chn].type  != YC_TYPE_Har1) //不能加
				har_num[n]++;
              yc_chn++;
        }	
        else if (pYcCfg[yc_chn].type == YC_TYPE_Thd)
        {
            n = pYcCfg[pYcCfg[yc_chn].arg2].arg1;
            meaCal_Thd(n,pYcCfg[yc_chn].arg1, har_num[n], result);
			ycValue[yc_chn] = result[0];
			har_num[n] = 0;
            yc_chn++;
        }
		
		if (yc_chn == ycNum)
		{
			yc_chn = 0;
			harcal = 0;
		}
	}  

	meaWrite_DbValue();

#ifdef INCLUDE_ATT7022
    ITempGainReg();
	TempAllGainReg();
    ReadACInstDataFun7022E(ATT7022_REG_UI|ATT7022_REG_PQ|ATT7022_REG_E|ATT7022_REG_OTHYC|ATT7022_REG_STS);
    Att7022e_Eqp();
#endif
}

int meaRead_DbValue(int beginNo, int num, struct VYCF_L *buf)
{    
    if (beginNo >= ycNum) return 0;
	if ((beginNo+num) > ycNum)  num = ycNum-beginNo;

    memcpy(buf, dbYcValue+beginNo, num*sizeof(struct VYCF_L));	

	return num;
}

int meaRead_YcChnBuf(int ycno, BYTE *flag, int *wave_point_num, short *wave_buf, int buf_len, int ptr)
{
    int i, chn;
	DWORD mask;
	short *p = wave_buf;

	if ((ycno<0) || (ycno>=ycNum)) return ERROR;

	if ((pYcCfg[ycno].type>=YC_TYPE_Ua) && (pYcCfg[ycno].type<=YC_TYPE_U0))
		mask = 1<<(pYcCfg[ycno].type-YC_TYPE_Ua);
	else if ((pYcCfg[ycno].type>=YC_TYPE_Ia) && (pYcCfg[ycno].type<=YC_TYPE_I0))
		mask = 1<<((pYcCfg[ycno].type-YC_TYPE_Ia)+4);
	else
		mask = 0;	
	if (mask == 0) return ERROR;

	if (ptr == -1) 
	{
		ptr = (SAM_BUF_LEN+wMeaSamCnt-3*MEA_SAM_POINT_N+1)&(SAM_BUF_LEN-1); 
		*flag = 0;
	}	
	
	*flag |= mask;
	chn = pYcCfg[ycno].arg1;	
	*wave_point_num = MEA_SAM_POINT_N*2;
	for (i=0; i<*wave_point_num; i++)
	{
		*p = nMeaSam[ptr][chn];
		p++;
		ptr = (ptr+1)&(SAM_BUF_LEN - 1);
	}	
		
	return 1;	
}

//不同装置尽量同时，从linux收到udp命令则触发记录cnt
static WORD wChnMeaSamCnt = 0;
void doreadchnbuf()
{
	wChnMeaSamCnt = wMeaSamCnt;
}

int meaRead_YcChnBuf1(int ycno, BYTE *flag, int *wave_point_num, short *wave_buf, int buf_len, int ptr)
{
    int i, chn;
    DWORD mask;
    short *p = wave_buf;

    if ((ycno<0) || (ycno>=ycNum)) return ERROR;

    if ((pYcCfg[ycno].type>=YC_TYPE_Ua) && (pYcCfg[ycno].type<=YC_TYPE_U0))
        mask = 1<<(pYcCfg[ycno].type-YC_TYPE_Ua);
    else if ((pYcCfg[ycno].type>=YC_TYPE_Ia) && (pYcCfg[ycno].type<=YC_TYPE_I0))
        mask = 1<<((pYcCfg[ycno].type-YC_TYPE_Ia)+4);
    else
        mask = 0;
    if (mask == 0) return ERROR;

    if (ptr == -1)
    {
        ptr = (SAM_BUF_LEN+wChnMeaSamCnt-6*4*MEA_SAM_POINT_N+1)&(SAM_BUF_LEN-1);
        *flag = 0;
    }

    if(*flag != mask)
        return 0;
    chn = pYcCfg[ycno].arg1;
    *wave_point_num = MEA_SAM_POINT_N*5;
    for (i=0; i<*wave_point_num; i++)
    {
        *p = nMeaSam[ptr][chn];
        p++;
        ptr = (ptr+4)&(SAM_BUF_LEN - 1);
    }
	/*for(i = 0;i < *wave_point_num; i++)
	{
		shellprintf(" %04d",wave_buf[i]);
		if(i%16 == 15) shellprintf("\r\n");
	}
	shellprintf("\r\n");*/
    return 1;
}


int meaRead_FdChnBuf(int fd, BYTE *flag, int *wave_point_num, short *wave_buf, int buf_len)
{
    int i, j, ptr, k, type;
	short *p = wave_buf;
	
	if ((fd<0) || (fd>=fdNum))  return ERROR;

	ptr = (SAM_BUF_LEN+wMeaSamCnt-3*MEA_SAM_POINT_N+1)&(SAM_BUF_LEN-1); 

    j=k=0;
    for (type=YC_TYPE_Ua; type<=YC_TYPE_I0; type++)
    {
		for (i=0; i<ycNum; i++)
		{
			if (pYcCfg[i].fdNo != fd) continue;
			if (pYcCfg[i].type != type) continue;

			if (meaRead_YcChnBuf(i, flag, wave_point_num,  p, buf_len-k, ptr) == ERROR)
				break;  
			j++;
			p+= *wave_point_num;
			k+= *wave_point_num;
			if (k >= buf_len) break;			
		}
    }
	
	return j;
}
//ssz
int meaRead_FdChnBuf1(int fd, BYTE *flag, int *wave_point_num, short *wave_buf, int buf_len)
{
    int i, j, ptr, k = 0, type;
    short *p = wave_buf;
    BYTE byFlag = *flag;
    BOOL bFlag = FALSE;

    if ((fd<0) || (fd>=fdNum))  return ERROR;

    ptr = (SAM_BUF_LEN+wChnMeaSamCnt-6*4*MEA_SAM_POINT_N+1)&(SAM_BUF_LEN-1);

    for(i=0;i<8;i++)
    {
        if(byFlag&1<<i)
        {
            bFlag = true;
            break;
        }
    }
    if(!bFlag)
    {
        return 0;
    }
    if(i<4)
        type = i+YC_TYPE_Ua;//电压
    else
        type = i -4 +YC_TYPE_Ia;//电流

    j = 0;
    for (i=0; i<ycNum; i++)
    {
        if (pYcCfg[i].fdNo != fd) continue;
        if (pYcCfg[i].type != type) continue;

        if (meaRead_YcChnBuf1(i, flag, wave_point_num,  p, buf_len-k, ptr) == ERROR)
            break;
        j++;
        p+= *wave_point_num;
        k+= *wave_point_num;
        break;
    }

    return j;
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

int meaRead_Yc2(int beginNo, int num, VMeaYc *meaYc)
{
	int i, j, n;
	int fd;
	VMeaYc *pYc;
	BYTE type;

    j = 0;
	pYc = meaYc;

	for (i=beginNo; i<(beginNo+num); i++)	
	{
		if (i >= ycNum) break;

		type = pYcCfg[i].type;

		if (type >= yccfgNum) 
		{
            if (pYcCfg[i].type == YC_TYPE_Angle)
			{
				pYc->value = ycValueBak[i]/100.0;
				pYc->unit = "°";
            }
            else if ((pYcCfg[i].type >= YC_TYPE_Har2)&&(pYcCfg[i].type < YC_TYPE_Thd))
			{
				n = pYcCfg[i].arg1;
				pYc->value = ycValueBak[i]*pDefTVYcCfg[pYcCfg[n].type].k;
				pYc->unit = pDefTVYcCfg[pYcCfg[n].type].unit;
			}
			else
			{
				pYc->value = (float)ycValueBak[i];
				pYc->unit = "";
			}
		}	
		else			
		{
			pYc->value = ycValueBak[i]*pDefTVYcCfg[type].k;
			pYc->unit = pDefTVYcCfg[type].unit;
			if (((pYcCfg[i].type >= YC_TYPE_Ia)&&
				(pYcCfg[i].type <= YC_TYPE_Ic))||
				(pYcCfg[i].type == YC_TYPE_SI0))
			{
			   fd = pYcCfg[i].fdNo;
			   if(fd >= 0 && fd < fdNum)
			   	 pYc->value = pYc->value ;
			}
			if(pYcCfg[i].type == YC_TYPE_I0)
			{
			   fd = pYcCfg[i].fdNo;
			   if(fd >= 0 && fd < fdNum)
			   	 pYc->value = pYc->value ;
			}
		}	
		j++;
		pYc++;
	}

	return j;
}

int meaRead_Yc1(int beginNo, int num, VMeaYc *meaYc)
{
	int i, j, n;
	VMeaYc *pYc;
	BYTE type;

    j = 0;
	pYc = meaYc;

	for (i=beginNo; i<(beginNo+num); i++)	
	{
		if (i >= ycNum) break;

		type = pYcCfg[i].type;

		if (type >= yccfgNum) 
		{
            if (pYcCfg[i].type == YC_TYPE_Angle)
				pYc->unit = "°";
            else if ((pYcCfg[i].type >= YC_TYPE_Har2)&&(pYcCfg[i].type < YC_TYPE_Thd))
			{
				n = pYcCfg[i].arg1;
				pYc->unit = pDefTVYcCfg[pYcCfg[n].type].unit;
			}
			else
				pYc->unit = "";
		}	
		else			
			pYc->unit = pDefTVYcCfg[type].unit;
		
		pYc->value = (float)ycValue[i]*pYcCfg[i].xs1_yc1/100; 
		pYc->value = pYc->value/pYcCfg[i].k;
		j++;
		pYc++;
	}

	return j;
}

int meaRead_Mmi(int fd, int beginNo, int num, VMmiMeaValue *mmiVal)
{
    int i, j, ai_chn, yc_chn;
	long angle;
	VFdCfg *pCfg;
	VMmiMeaValue *pMmiVal;

	if ((fd<0) || (fd >= fdNum)) return 0;

	pCfg = pFdCfg+fd;

    memset(mmiVal, 0, num*sizeof(VMmiMeaValue));


	j=0;
	pMmiVal = mmiVal;
	for (i=beginNo; i<(beginNo+num); i++)	
	{
		if (i >= MMI_MEA_NUM) break;

        ai_chn = pCfg->mmi_meaNo_ai[i];
        yc_chn = pCfg->mmi_meaNo_yc[i];

		pMmiVal->tbl = mmiMeaTbl+i;   /*应通过i查询tbl的id,化简为i*/

		if ((ai_chn < 0) && (yc_chn < 0)) continue;
			
#ifdef MMI_MEASHOW_USER_FT
 		if (ai_chn >= 0)
		{
			if (i <= MMI_MEA_IC)  //I
			    pMmiVal->rms = (float)meaValue[ai_chn].lRms*5/pMeaGain[ai_chn].a;
			else if(i == MMI_MEA_I0)  //I0
		        pMmiVal->rms = (float)meaValue[ai_chn].lRms*pCfg->In0/pMeaGain[ai_chn].a;
			else if((i == MMI_MEA_U0) && (g_Sys.MyCfg.dwCfg & 0x10))                //U0
			    pMmiVal->rms = (float)meaValue[ai_chn].lRms*10/pMeaGain[ai_chn].a;
			else
				pMmiVal->rms = (float)meaValue[ai_chn].lRms*100/pMeaGain[ai_chn].a;
		}	
		else if (yc_chn >= 0)
		{
			if (i <= MMI_MEA_I0)  //I
			{
				pMmiVal->rms = ycValue[yc_chn]/200.0;
			}
			else                  //U
			{
				pMmiVal->rms = ycValue[yc_chn]/100.0;
			}					
		}
#else
		if (yc_chn >= 0)
		{
			if (i <= MMI_MEA_I0)  //I
			{
				pMmiVal->rms = ycValue[yc_chn]/200.0;
			}
			else				  //U
			{
				pMmiVal->rms = ycValue[yc_chn]/100.0;
			}					
		}
#endif
		if (ai_chn >= 0)
		{
		    //hll 所有回线均与第一回线比较角度
	        if (pFdCfg->mmi_meaNo_ai[MMI_MEA_UA] >= 0)
				angle = meaValue[ai_chn].lAng - meaValue[pFdCfg->mmi_meaNo_ai[MMI_MEA_UA]].lAng;
			else if (pCfg->mmi_meaNo_ai[MMI_MEA_IA] >= 0)
				angle = meaValue[ai_chn].lAng - meaValue[pFdCfg->mmi_meaNo_ai[MMI_MEA_IA]].lAng;

			if (angle < -11796480)
			{
				angle += 23592960;
			}
			else if (angle > 11796480)
			{
				angle -= 23592960;
			}
			pMmiVal->ang = angle/65536.0;					
		}	
		
		pMmiVal->valid = 1;
		pMmiVal++;
		j++;
	}
	
	return j;
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
  
#if defined  (INCLUDE_DD) && !defined (INCLUDE_ATT7022)
    int i;
	DWORD type;

	ddValueSem = smMCreate();
	if (gainValid == 0) return;

	memset(meaDd, 0, sizeof(meaDd));
	memset(dbDdValue, 0, sizeof(dbDdValue));
	memset(rawDdValue, 0, sizeof(rawDdValue));
	memset(ddValue, 0, sizeof(ddValue));
	ddNum = fdNum*4;

    for (i=0; i<fdNum; i++)
		meaDd[i].p_id = meaDd[i].q_id = -1;

	for (i=0; i<ycNum; i++)
	{
		if (pYcCfg[i].fdNo < 0) continue;

		if (pYcCfg[i].type == YC_TYPE_P) 
		{
				meaDd[pYcCfg[i].fdNo].p_id = i;
				meaDd[pYcCfg[i].fdNo].no[0] = pYcCfg[i].fdNo*4+0;
				meaDd[pYcCfg[i].fdNo].no[2] = pYcCfg[i].fdNo*4+2;
		}		
		else if (pYcCfg[i].type == YC_TYPE_Q) 
		{
			meaDd[pYcCfg[i].fdNo].q_id = i;
			meaDd[pYcCfg[i].fdNo].no[1] = pYcCfg[i].fdNo*4+1;
			meaDd[pYcCfg[i].fdNo].no[3] = pYcCfg[i].fdNo*4+3;
		}			
	}   

	extNvRamGet(NVRAM_AD_DD, (BYTE *)&type, sizeof(DWORD));	
	if (type != g_Sys.MyCfg.dwAIType)
	{
		type = g_Sys.MyCfg.dwAIType;
		extNvRamSet(NVRAM_AD_DD+sizeof(DWORD), (BYTE *)ddValue, sizeof(ddValue));
		extNvRamSet(NVRAM_AD_DD, (BYTE *)&type, sizeof(DWORD));
	}
	else		
		extNvRamGet(NVRAM_AD_DD+sizeof(DWORD), (BYTE *)ddValue, sizeof(ddValue));		

	for (i=0; i<ddNum; i++)
		dbDdValue[i] = ddValue[i]*100;
	WriteRangeDD(g_Sys.wIOEqpID, 0, ddNum, dbDdValue); 

#if (TYPE_CPU == CPU_STM32F4) || (TYPE_CPU == CPU_SAM9X25)
	dtmInit(DTM_DD_TIMER, 1, INT_PRIO_DTIMER, (ENTRYPTR)meaCal_Dd_1SInt);
#endif
	gddInit = 1;
#else
#ifdef  INCLUDE_ATT7022
     Init7022E();
#else
	g_Sys.MyCfg.wDDNum = 0;
	/*g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo->Cfg.wDDNum = 0;
	g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo->DDSendCfg.wNum = 0;
	g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo->DDHourFrz.wNum = 0;
	g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo->DDDayFrz.wNum = 0;
	g_Sys.Eqp.pInfo[g_Sys.wIOEqpID].pTEqpInfo->DDMonthFrz.wNum = 0;*/
#endif
#endif
}


void SetRangeYccfg()
{
	//linux侧实现

	/*int i;	
	for(i =0;i<ycNum;i++)
	{
		if(pYcCfg[i].type < yccfgNum)
			WriteYCCfg(g_Sys.wIOEqpID, i, pYcCfg[i].type, pYcCfg[i].fdNo,pDefTVYcCfg[pYcCfg[i].type].unit);	
	}*/
}

void ycInit(void)
{
#ifdef INCLUDE_AI_HTM
    aiHtmInit();
#endif
	gainValid = gycInit = 0;

    ycNum = YC_SAM_POINT_N;

	diNUM = g_Sys.MyCfg.wDINum;
	aiNUM = g_Sys.MyCfg.wAINum;
	ycNum = g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic;
	fdNum = g_Sys.MyCfg.wFDNum;

	pAiCfg = g_Sys.MyCfg.pAi;
	pYcCfg = g_Sys.MyCfg.pYc;
	pFdCfg = g_Sys.MyCfg.pFd;

	adInit(pMeaGain);
	
	if ((pAiCfg == NULL)||(pYcCfg == NULL)||(pFdCfg == NULL))
		return;

	memset(nMeaBuf, 0, sizeof(nMeaBuf));	
	memset(nMeaFtBuf, 0, sizeof(nMeaFtBuf));
	memset(meaValueTmp, 0, sizeof(meaValueTmp));
	memset(meaValue, 0, sizeof(meaValue));
#ifdef YCQ_TEST
    gQFft = (VMmifft*)calloc(13, sizeof(VMmifft));
    memset(gQFft, 0, sizeof(13*sizeof(VMmifft)));
#endif
	createYcRef();
  
	meaGainGet();
	if (gainNum == 0) return;
	
	rawPtr = 0;

	rawYc = (long *)calloc(ycNum*YC_MEAN_NUM, sizeof(long));

#ifndef YC_GAIN_NEW
	tmpRawYc1 = (long *)calloc(gainNum, sizeof(long));
	tmpRawYc2 = (long *)calloc(gainNum, sizeof(long));
#endif

	ycValue = (long *)calloc(ycNum, sizeof(long));
	ycValueBak = (long *)calloc(ycNum, sizeof(long));
	dbYcValue = (struct VYCF_L *)calloc(ycNum, sizeof(struct VYCF_L));

	if (!(rawYc  && ycValue && ycValueBak && dbYcValue))  return;

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

#if 0
int meaGainFileRead(DWORD *ptype, VYcGain *pgain, int len, WORD *pcrc)
{
	FILE *fp;
	char path[2*MAXFILENAME];
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
	char path[2*MAXFILENAME];
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
#endif

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
    int i, j, sum,len;
    WORD crc;

    if (gainNum == 0) 		
	{
		shellprintf("\nError! gain num = 0\n");
		return ERROR; 
	}	

	shellprintf("zero gain...");

	meaDataLoad_Yc(1);

	for (i=0; i<aiNUM; i++)
	{
		sum = 0;
		for (j=0; j<MEA_SAM_POINT_N; j++)
			sum += nMeaBuf[j][i];

		sum = sum*10/MEA_SAM_POINT_N;
        if (sum >= 0)
        {		
			if ((sum % 10) >= 5)
				sum = (sum+9)/10;
			else
				sum /= 10;
        }
		else
		{
			sum = -sum;
			if ((sum % 10) >= 5)
				sum = (sum+9)/10;
			else
				sum /= 10;
			sum = -sum;
		}
		
		pMeaGain[i].ycNo = pAiCfg[i].ycNo;
		
		shellprintf("%d  %d \n",i,sum);
		pMeaGain[i].b = sum;
		
	}

	meaDataLoad_Yc(0);
	
	shellprintf("done\n");

#ifndef YC_GAIN_NEW
	gainStep  = 1;
#endif

	len = sizeof(VYcGain)*(aiNUM+gainNum);
	crc = GetParaCrc16((BYTE *)pMeaGain, len);
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
		//thSleep(100);
		for(i = 0; i < 10000;i++);
		
		if ((meaGainVerRead(addr, &ver, &len, 1) == ERROR)||
		    (meaGainEERead(addr+2*sizeof(DWORD), &type, pMeaGain, len, &crc, 1) == ERROR))
		{
				createGainNo2YcRef(gainNum);   //重建,防止读错误导致索引非法
				setDefGainValue();
				return;
		}
	}

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
	//int flag = 1;

    len = sizeof(VYcGain)*(aiNUM+gainNum);
	pgain = (VYcGain *)malloc(len);
	if (pgain == NULL) return ERROR;

    addr = NVRAM_AD_GAIN;
	if (meaGainVerRead(addr, &ver, &len, 0) == OK)
		addr += 2*sizeof(DWORD);

	if (meaGainEERead(addr, &type, pgain, len, &crc, log) == ERROR) goto Err;
	//if (meaGainFileWrite(type, pgain, len, crc, flag) == ERROR) goto Err; //linux写文件

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

    int i, ycno, ycno2, aino, gainno, fdno, g1, g2,len;
	WORD crc;
	VYcVal* pbuf = (VYcVal*)pData;
	VFdCfg *pCfg;
#ifdef INCLUDE_ATT7022
	int T;
	int ret;
#endif

#ifdef INCLUDE_AD
	long p, q, p0, q0;
	//BYTE ads8578 = 1;
#endif


    if (gainNum == 0) 		
	{
		shellprintf("\nError! gain num = 0\n");
		return ERROR; 
	}

	if (flag == 0)
	{
	    shellprintf("channle gain...");
	    for (i=0; i<num; i++)
	    {
	         
	    	 ycno = pbuf[i].ycNo;
			 fdno = pYcCfg[ycno].fdNo;
			 aino = pYcCfg[ycno].arg1;
			 gainno = pYcCfg[ycno].gainNo;
			 pCfg = pFdCfg + fdno;
			 
			if((pYcCfg[ycno].type <= YC_TYPE_I0) && (pYcCfg[ycno].type >= YC_TYPE_Ia))
			{
				if ((pbuf[i].value < 1000)||(ycValue[ycno] < 250))
					continue;
			}
			else if(pYcCfg[ycno].type == YC_TYPE_U0)
			{
				if ((pbuf[i].value < 100)||(ycValue[ycno] < 250))
					continue;
			}
			else
			{
				if ((pbuf[i].value < 1000)||(ycValue[ycno] < 1000))
				    continue;
			}

#ifdef INCLUDE_ATT7022
			if((pYcCfg[ycno].type < YC_TYPE_U0) && (pYcCfg[ycno].type >= YC_TYPE_Ua))
			{
				Cal_U_gain(pYcCfg[ycno].type -YC_TYPE_Ua + 1, pbuf[i].value);
			}

			if((pYcCfg[ycno].type < YC_TYPE_I0) && (pYcCfg[ycno].type >= YC_TYPE_Ia))
			{
				Cal_I_gain(pYcCfg[ycno].type -YC_TYPE_Ia + 1, pbuf[i].value);
			}
			 
#endif
#ifdef INCLUDE_AD
			 pYcGain[gainno].a = ((long long)avgValue(gainno)*10000*pYcCfg[ycno].xs1)/(pbuf[i].value*pYcCfg[ycno].xs2);
		     pYcGain[gainno].status |= YC_GAIN_BIT_VALID;
             
#if 0
			 if(ggainver == YC_GAIN_AD7607_VER) //当整定时系数太大，则认为是ads8578
			 {
			 	 if((pYcCfg[ycno].type < YC_TYPE_U0) && (pYcCfg[ycno].type >= YC_TYPE_Ua))
				 {
					 if(pYcGain[gainno].a > U_GAIN_MAX) //符合ADS8578
					 {
						 ggainver = YC_GAIN_ADS8578_VER;
						 ads8578 = 1;
					 }
				 }
				 
				  if((pYcCfg[ycno].type < YC_TYPE_I0) && (pYcCfg[ycno].type >= YC_TYPE_Ia))
				 {
					 if(pYcGain[gainno].a > I_GAIN_MAX)  //符合ADS8578
					 {
						 ggainver = YC_GAIN_ADS8578_VER;
						 ads8578 = 1;
					 }
				 }
			 } 
#endif
#endif
            if ((pMeaGain[aino].type <= YC_TYPE_U0)&&(pMeaGain[aino].type >= YC_TYPE_Ua))
                pMeaGain[aino].a = ((long long)meaValue[aino].lRms*100*100)/pbuf[i].value; // 100 替换了 pCfg->Un
            else if((pMeaGain[aino].type < YC_TYPE_I0)&&(pMeaGain[aino].type >= YC_TYPE_Ia))
                pMeaGain[aino].a = ((long long)meaValue[aino].lRms*5*1000)/pbuf[i].value;
            else if (pMeaGain[aino].type == YC_TYPE_I0)
                pMeaGain[aino].a = ((long long)meaValue[aino].lRms*pCfg->In0*1000)/pbuf[i].value;
             pMeaGain[aino].status |= YC_GAIN_BIT_VALID;   
			
#ifdef INCLUDE_AD
		   /*if(ads8578 == 1)
		   {
			   pMeaGain[aino].a  = pMeaGain[aino].a/4;
			   pYcGain[gainno].a = pYcGain[gainno].a/4;
		   }*/
#endif
        }

#ifdef INCLUDE_AD
		for (i=0; i<ycNum; i++)
		{
		    if ((pYcCfg[i].type >= YC_TYPE_Pa)&&(pYcCfg[i].type <= YC_TYPE_Pc))
		    {
		        g1 = pYcCfg[pYcCfg[i].arg1].gainNo;
			    g2 = pYcCfg[pYcCfg[i].arg2].gainNo; 
				gainno = pYcCfg[i].gainNo;
				pYcGain[gainno].a = pYcGain[g1].a * pYcGain[g2].a / 1000;
		    }
			if ((pYcCfg[i].type >= YC_TYPE_Qa)&&(pYcCfg[i].type <= YC_TYPE_Qc))
		    {
		        g1 = pYcCfg[pYcCfg[i].arg1].gainNo;
			    g2 = pYcCfg[pYcCfg[i].arg2].gainNo; 
				gainno = pYcCfg[i].gainNo;
				pYcGain[gainno].a = pYcGain[g1].a * pYcGain[g2].a  / 1000;
		    }
		}
#endif
	}
	else if (flag == 1)
	{
	     shellprintf("line power gain...");
		 for (i=0; i<num/2; i++)
		 {
		     ycno = pbuf[2*i].ycNo;
			 ycno2 = pbuf[2*i+1].ycNo;
			 g1 = pYcCfg[ycno].gainNo;
			 g2 = pYcCfg[ycno2].gainNo;        
                            
			 

#ifdef INCLUDE_ATT7022
				if ((pbuf[2*i].value > 1000) && (ycValue[ycno] > 500) && (pbuf[2*i+1].value < 1000) )//&& (ycValue[ycno2] < 1000))
				{
					Cal_PQ_gain(pYcCfg[ycno].type -YC_TYPE_Pa + 1,pbuf[2*i].value);
				}
#endif

#ifdef INCLUDE_AD
				if ((pbuf[2*i].value < 1000)||(ycValue[ycno] < 1000)||(pbuf[2*i+1].value < 1000)||(ycValue[ycno2] < 1000))
					continue;
				p = ((long long )avgValue(g1) * 1000) / pYcGain[g1].a;
				q = ((long long )avgValue(g2) * 1000) / pYcGain[g2].a;

				p0 = pbuf[2*i].value * pYcCfg[ycno].xs2 / pYcCfg[ycno].xs1;
				q0 = pbuf[2*i+1].value * pYcCfg[ycno2].xs2 / pYcCfg[ycno2].xs1;

				pYcGain[g1].b = (p - p0 + q0 - q) * 10000/(p + q);
				pYcGain[g2].b = pYcGain[g1].b;          
#endif
		 }
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

#ifdef INCLUDE_AD
		len = sizeof(VYcGain)*(aiNUM+gainNum);
		crc = GetParaCrc16((BYTE *)pMeaGain, len);
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
    //int num = 0;
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
	//num = aigainNum+gainNum;
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

