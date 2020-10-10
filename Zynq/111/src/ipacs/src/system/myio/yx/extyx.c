/*------------------------------------------------------------------------
 Module:       	extyx.c
 Author:        solar
 Project:       
 State:			
 Creation Date:	2010-5-29
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: $
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef INCLUDE_EXT_YX
#include "yx.h"
#include "extyx.h"
#include "sys.h"

WORD  pannelYx;

BYTE *pExtYx;
static BYTE ExtYxRev;
static BYTE rawExtYx;
static BYTE rawExtDi;
static BYTE extYxFltflag = 0;

static VExtYxChn ExtYxChn[8];
BYTE extYxGet(void);
void extDiFilter(void);
int extyx_init = 0;

#if ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_WDG)) 
WORD EXTYX_INPUT(void)
{
	WORD val;

	if (GET_DI_INPUT() & 0x2000)   //失电告警并取反
		val = 0x00;              
	else
		val = 0x08; 

    val |= (BYTE)((GET_DI_INPUT() >> 10) & 0x07);  //本地采4路遥信

	val |= (BYTE)((pannelYx<<4) & 0x70);                 //面板采3路

	return val;	
}
#elif (DEV_SP == DEV_SP_DTU)
WORD EXTYX_INPUT(void)
{
	WORD val = 0;

	CPLD_Write(BSP_PORT_YX_LATCH ,BSP_ADDR_POWER_DI); 
	val = CPLD_Read(BSP_PORT_YX_READ)^0xFF;

	val |= ((READ_GPIO( 0, GPIO_KG1) << 8 ) | 
	       ((!READ_GPIO( 0, GPIO_AJ_FW)) << 9 ) |
	       (READ_GPIO( 0, GPIO_KG2) << 10 ) |
	       ((val&0x40)<<3));
    /*闭锁*/

	/* 有一种液晶面板代替了机柜，取液晶的远方本地 */
	val |= (BYTE)((pannelYx<<4) & 0x30);

	if((!(val & 0x20))&&(!(val&0x10)))
	   val |= 0x40;
	else 
	   val &=~0x40;
 

    /*失电告警*/
	val = val^0x8;

	return val;	
}
#elif (DEV_SP == DEV_SP_TTU)
WORD EXTYX_INPUT(void)
{
	WORD val = 0;
	val |= ((!(READ_GPIO(GPIOC, GPIO_Pin_4)>>4))|
		    (READ_GPIO(GPIOA, GPIO_Pin_16)>>15)|
		   ((READ_GPIO(GPIOB, GPIO_Pin_11)>>9)));
	
	return val;
}
#elif(DEV_SP == DEV_SP_DTU_IU)
WORD EXTYX_INPUT(void)
{
	WORD val = 0;

	val |= (BYTE)(GET_DI_INPUT() & 0x04);                 //本地远方 闭锁

	if(val & 0x04)
	   val = 0x20;
	else 
	   val = 0x10;
	return val;	

}
#else
#define EXTYX_INPUT()   0
#endif

void extYxRefresh(void)
{
    int i;
	WORD mask;
	struct VDBYX DBYX;

	*pExtYx = extYxGet();

	for (i=0; i<8; i++)
	{
		mask = 1<<i;
        if (*pExtYx & mask)
        {
			DBYX.wNo = g_Sys.MyCfg.SYXIoNo[1].wIoNo_Low+i;
			DBYX.byValue = 0x81;			
			WriteSYX(BM_EQP_MYIO, DBYX.wNo, DBYX.byValue); 						
        }
		else
		{
			DBYX.wNo = g_Sys.MyCfg.SYXIoNo[1].wIoNo_Low+i;
			DBYX.byValue = 0x01;			
			WriteSYX(BM_EQP_MYIO ,DBYX.wNo, DBYX.byValue); 
		}
	}
}

BYTE GetMyExtDiValue(int no)
{
   BYTE val=0x01;

   if ((no < 0)||(no >= 16))  return 0;

   if (no < 8)
   {
      val = val<<no;

      if (*pExtYx&val) return 1;
      else return 0;
   }
   else
   {
      val = val <<(no-8);

	  if (rawExtDi&val) return 1;
	  else return 0;
   }
}

BYTE extYxGet(void)
{
	BYTE temp[3];
	BYTE temp1,temp2;
	int i;

	for(i=0; i<3; i++)
		temp[i] = (EXTYX_INPUT()&0xFF)^ExtYxRev;
	temp1 = temp[0]&temp[1];
	temp2 = temp[0]&temp[2];
	temp1 |= temp2;
	temp2 = temp[1]&temp[2];
	temp1 |= temp2;

	return(temp1); 
}

void extYxFilter(void)
{
	struct VDBSOE DBSOE;	
	FAST BYTE flt_flag;
	FAST BYTE cur_state;
	FAST BYTE chgbit;
	FAST BYTE chkbit;
	FAST int j, value;
	FAST VExtYxChn *pchn;

	//MCF_GPIO_PORTTC ^= 0x08;
	if(!extyx_init)
		return;
	
	cur_state = (EXTYX_INPUT()&0xFF)^ExtYxRev;
	chgbit = cur_state^rawExtYx;  /* 异或，找到不同的输入*/ 
	rawExtYx = cur_state;

    flt_flag = extYxFltflag;
	flt_flag |= chgbit;

	if (flt_flag)
	{
		pchn = ExtYxChn;
		extYxFltflag = flt_flag;

		chkbit = 1;  
		for (j=0; j<8; j++)        //最多8个扩展YX
		{
            if (flt_flag & chkbit)
            {
				if (chgbit & chkbit)  /* 发生变化 */
				{
					pchn->cnt = 3;    /*30ms防抖,由于任务关系,可能少于30*/
					//pchn->chg_tm.dwMinute = gpCalTime->dwMinute;
					//pchn->chg_tm.wMSecond = gpCalTime->wMSecond;
					GetSysClock(&pchn->chg_tm, CALCLOCK);
				}

				if (!pchn->cnt) 	/* 去抖动脉冲计数结束 */ 
				{
					extYxFltflag &= ~chkbit; 	/* 清除标志 */	
					
					if ((cur_state & chkbit) != (*pExtYx & chkbit))
					{
						value = (cur_state & chkbit) ? 1:0;  /* DI输入确认 */
						if (value)
							*pExtYx |= chkbit;								
						else 
							*pExtYx &= (~chkbit);
						
						DBSOE.wNo = pchn->no;
						if (value)
							DBSOE.byValue = 0x81;
						else
							DBSOE.byValue = 0x01;	
						
						DBSOE.Time = pchn->chg_tm;
						WriteSSOE(BM_EQP_MYIO, DBSOE.wNo, DBSOE.byValue, DBSOE.Time);					
					}
				}
				else
					pchn->cnt--; /* 通道滤波计数减一 */
				
				flt_flag &= ~chkbit;	/* 检测过的清零 */ 
				if (!flt_flag)		/* 判断所有的位是不是检测完 */
					break;
            }	

			pchn++;
			chkbit <<= 1;		/* 检测位左移 */ 
		}
	}	

    extDiFilter();

	WriteMyMyExtDiValue((rawExtDi << 8)|(*pExtYx));
}

void extDiFilter(void)
{
#if (DEV_SP == DEV_SP_DTU)
	FAST BYTE flt_flag;
	FAST BYTE cur_state;
	FAST BYTE chgbit;
	FAST BYTE chkbit;
	FAST int j, value;
	static BYTE diflag;
	static DWORD  dichncnt[3];


	cur_state = EXTYX_INPUT()>>8;
	chgbit = cur_state^rawExtDi;  /* 异或，找到不同的输入*/ 
	rawExtDi = cur_state;

	flt_flag = diflag;
	flt_flag |= chgbit;

	if (flt_flag)
	{
		diflag = flt_flag;

		chkbit = 1;  
		for (j=0; j<3; j++)        //最多8个扩展YX
		{
            if (flt_flag & chkbit)
            {
				if (chgbit & chkbit)  /* 发生变化 */
				{
					dichncnt[j] = 3;    /*30ms防抖,由于任务关系,可能少于30*/
				}
				if (dichncnt[j]) 	/* 去抖动脉冲计数结束 */ 
				{
					diflag &= ~chkbit; 	/* 清除标志 */	
					
					if ((cur_state & chkbit) != (rawExtDi & chkbit))
					{
						value = (cur_state & chkbit) ? 1:0;  /* DI输入确认 */
						if (value)
							rawExtDi |= chkbit;								
						else 
							rawExtDi &= (~chkbit);
						
											
					}
				}
				else
					dichncnt[j]--; /* 通道滤波计数减一 */
				
				flt_flag &= ~chkbit;	/* 检测过的清零 */ 
				if (!flt_flag)		/* 判断所有的位是不是检测完 */
					break;
            }	
			chkbit <<= 1;		/* 检测位左移 */ 
		}
	}	
#endif
}

void extYxInit(void)
{
    int i;
	BYTE  mask;

	rawExtDi = 0;
	pExtYx = (BYTE *)BSP_ESRAM_EXTYX;
	ExtYxRev = 0;

	for (i=0; i<8; i++)
	{
	     mask = 1<<i; 
	     ExtYxChn[i].no = g_Sys.MyCfg.SYXIoNo[1].wIoNo_Low+i;

		 if (g_Sys.MyCfg.SysExt.YxIn[i].cfg & DICFG_REV) ExtYxRev |= mask;
	}
	
	rawExtYx = *pExtYx = extYxGet();	
	extYxRefresh();

	extyx_init = 1;
}
#endif

