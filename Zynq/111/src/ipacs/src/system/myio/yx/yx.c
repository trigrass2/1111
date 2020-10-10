/*------------------------------------------------------------------------
 Module:       	yx.c
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-9-2
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef INCLUDE_YX
#include "di.h"
#include "yx.h"
#include "sys.h"

static int gYxInit = 0;

static WORD g_DiValue[MAX_YX_PORT_NUM];
WORD g_RawDiValue[MAX_YX_PORT_NUM];
VYxNo2Port g_YxNo2Port[MAX_YX_PORT_NUM*MAX_YX_NUM_PER_PORT];

static WORD pDiBitMap[MAX_YX_PORT_NUM];
static VDiCfg *pDiCfg;
static VYxChn pYxChn[MAX_YX_PORT_NUM][MAX_YX_NUM_PER_PORT];
static VDYxChn pDYxChn[MAX_YX_PORT_NUM][MAX_YX_NUM_PER_PORT];
static VYxPort pYxPort[MAX_YX_PORT_NUM];			
static VSoeBuf gSoebuf;		

static int YX_PORT_NUM;
int YX_NUM, DYX_NUM;


#if (DEV_SP == DEV_SP_FTU)
#define YX_NUM_PER_PORT     9
#endif
#if (DEV_SP == DEV_SP_CTRL)
#define YX_NUM_PER_PORT     10
#endif

#if (DEV_SP == DEV_SP_WDG)
#define YX_NUM_PER_PORT     (5 + 4*(g_wdg_ver!=0))
#endif

#if (DEV_SP == DEV_SP_TTU)
#define YX_NUM_PER_PORT     8
#endif

#if (DEV_SP == DEV_SP_DTU_PU)
#define YX_NUM_PER_PORT     6
#endif

#if (DEV_SP == DEV_SP_DTU_IU)
#define YX_NUM_PER_PORT     8
#endif

#if(DEV_SP != DEV_SP_DTU)
WORD YX_INPUT(DWORD addr)
{
	return((WORD)DI_INPUT());
}
#define diCfgAddrSet()

#else
#define YX_NUM_PER_PORT     8
WORD YX_INPUT(DWORD addr)  
{
	CPLD_Write(BSP_PORT_YX_LATCH, addr);
	
	return (CPLD_Read(BSP_PORT_YX_READ));
}
void diCfgAddrSet(void)
{
    int i,j,k;
	BYTE type;
	VDefDiCfg *pDefDiCfg;

    k = 0;
	for (i=0; i<BSP_IO_BOARD; i++)
	{
	     type = Get_Io_Type(i);
		 if (type == 0xFF) continue;

		 pDefDiCfg = GetDefDiCfg(type);
		 if (pDefDiCfg == NULL) continue;
		 if ((k + pDefDiCfg->portnum) > MAX_YX_PORT_NUM) break;
		 for (j=0; j<pDefDiCfg->portnum; j++)
	     {
	         pYxPort[k].addr = (BSP_ADDR_IO1_READ-i*8+j);  //暂定
			 k++;
		 }
	}
}

#endif

void yxGet(WORD *buf)
{
	FAST VYxPort *pport ;
	FAST WORD cur_state, temp1, temp2;
	FAST int i, j;
	WORD temp[3][MAX_YX_PORT_NUM];

	for(i=0; i<3; i++)
	{
		pport = pYxPort;
		for (j=0; j<YX_PORT_NUM; j++)
		{
			cur_state = YX_INPUT(pport->addr)^pport->rev;
			cur_state &=  pport->mask;
			temp[i][j] = cur_state;
			pport++;
		}	
	}
	
	for(i=0; i<YX_PORT_NUM; i++)
	{
		temp1 = temp[0][i]&temp[1][i];
		temp2 = temp[0][i]&temp[2][i];
		temp1 |= temp2;
		temp2 = temp[1][i]&temp[2][i];
		temp1 |= temp2;
		buf[i] = temp1;
	}
}

void yxRefresh(int dyx)
{
    int i, j;
	WORD mask;
	VYxChn *pchn;
	VDYxChn *pdchn;	
	struct VDBYX DBYX;
	struct VDBDYX DBDYX;

	for (i=0; i<YX_PORT_NUM; i++)
	{
		pchn = &pYxChn[i][0];
		for (j=0; j<YX_NUM_PER_PORT; j++)
		{
			mask = 1<<j;
 			if (pDiBitMap[i] & mask)
			{
				DBYX.wNo = pchn->yxno;
				if (g_DiValue[i] & mask)
				{
					pchn->value = 1;
					DBYX.byValue = 0x81;
				}	
				else
				{
					pchn->value = 0;
					DBYX.byValue = 0x01;
				}	
				WriteSYX(BM_EQP_MYIO,DBYX.wNo, DBYX.byValue); 						
			}

			pchn++;
		}
	}	

    if (dyx)
    {
		for (i=0; i<YX_PORT_NUM; i++)
		{
			pchn = &pYxChn[i][0];
			pdchn = &pDYxChn[i][0]; 
			for (j=0; j<YX_NUM_PER_PORT; j++)
			{
				mask = 1<<j;
				
			    if ((pDiBitMap[i] & mask) && (pchn->type == DITYPE_DYX) && (pchn->yxno < pdchn->pairno))				
				{
					if((pchn->cfg & DICFG_SYX))
					{
						DBDYX.wNo = pchn->dyxno;
					}
					else
					{
						DBDYX.wNo = pchn->yxno>>1;
					}
					if((pchn->cfg & DICFG_HW) &&  ((pdchn->pair->cfg & DICFG_HW) == 0))//配置为合位
					{
						if (pchn->value == 1)						
							DBDYX.byValue2 = 0x81;
						else
							DBDYX.byValue2 = 0x01;
						if (pdchn->pair->value == 1)						
							DBDYX.byValue1 = 0x81;
						else
							DBDYX.byValue1 = 0x01;
					}
					else
					{
						if (pchn->value == 1)						
							DBDYX.byValue1 = 0x81;
						else
							DBDYX.byValue1 = 0x01;
						if (pdchn->pair->value == 1)						
							DBDYX.byValue2 = 0x81;
						else
							DBDYX.byValue2 = 0x01;
					}					
					WriteDYX(BM_EQP_MYIO, DBDYX.wNo, DBDYX.byValue1, DBDYX.byValue2); 		
				}		
				
				pchn++;
				pdchn++;
			}
		}	
	}
}

void yxChnInit(void)
{
    int i, j, no, dyx;
	int num=0;
	WORD mask;
	VYxChn *pchn;
	VDYxChn *pdchn;
	VYxPort *pport;

    no = 0;
	pport = pYxPort;
	for (i=0; i<YX_PORT_NUM; i++)
	{
		pport->rev = 0;
		pport->mask = 0;

		pchn = &pYxChn[i][0];
		for (j=0; j<YX_NUM_PER_PORT; j++)
		{	
			
			mask = 1<<j;

			pport->rev |= mask;
			pport->mask |= mask;

 			if ((pDiBitMap[i] & mask) == 0) 
				pport->mask &= ~mask;
			else
			{
				pchn->yxno = pDiCfg[no].yxno;
				pchn->type = pDiCfg[no].type;
				pchn->cfg = pDiCfg[no].cfg;
				pchn->re_yxno= pDiCfg[no].re_yxno;

				if (pchn->type == DITYPE_PULSE) 
					pchn->dtime = (DEFAULT_PULSE_WIDTH*1000+(YX_FILTER_TM_UNIT-1))/YX_FILTER_TM_UNIT;
				else
					pchn->dtime = (pDiCfg[no].dtime*1000+(YX_FILTER_TM_UNIT-1))/YX_FILTER_TM_UNIT;

				if (pchn->cfg & DICFG_REV) pport->rev &= ~mask;
				if (pchn->cfg & DICFG_DIS) pport->mask &= ~mask;

				if (pchn->yxno < YX_NUM)
				{
					g_YxNo2Port[pchn->yxno].pchn = pchn;
					g_YxNo2Port[pchn->yxno].port = i;
					g_YxNo2Port[pchn->yxno].bit = mask;
				}

				no++;
				if (no >= g_Sys.MyCfg.wDINum) break;

			}
			pchn++;
		}
		
		if (no >= g_Sys.MyCfg.wDINum) break;
		pport++;
	}	

    //DYX
    dyx = 0;
	for (i=0; i<YX_PORT_NUM; i++)
	{
		pchn = &pYxChn[i][0];
		pdchn = &pDYxChn[i][0]; 
		for (j=0; j<YX_NUM_PER_PORT; j++)
		{
			mask = 1<<j;

 			if (pDiBitMap[i] & mask)
			{
				if((pchn->cfg & DICFG_SYX))
				{
					pdchn->pairno = pchn->re_yxno;
			
				}
				else
				{
					if (pchn->yxno & 1)
					pdchn->pairno = pchn->yxno-1;
					else
					pdchn->pairno = pchn->yxno+1;
				}
				
				pdchn->pair = g_YxNo2Port[pdchn->pairno].pchn;

				if (pchn->type == DITYPE_DYX)
				{
					dyx = 1;
					pdchn->pair->type = DITYPE_DYX;
					if(pchn->yxno < pdchn->pairno)
					{
						pchn->dyxno = num++;						
					}
					else
					{
						 pchn->dyxno = pdchn->pair->dyxno;
					}
					pport = pYxPort+g_YxNo2Port[pdchn->pairno].port;
					if (pchn->cfg & DICFG_REV)
						pport->rev &= ~(g_YxNo2Port[pdchn->pairno].bit);
					if (pchn->cfg & DICFG_DIS) 
						pport->mask &= ~(g_YxNo2Port[pdchn->pairno].bit);
				}	
			}
 			pchn++;
			pdchn++;
		}
	}	

	if (dyx == 0)
	{
		g_Sys.MyCfg.wDYXNum = 0;
		DYX_NUM = 0;
	}
	else
		DYX_NUM = YX_NUM/2;		
	
    yxGet(g_DiValue);
	yxRefresh(dyx);	

}

void WriteYxChnTime()
{
	int i,j,no;
	WORD mask;
	VYxChn *pchn;
	VYxPort *pport;
	
	no = 0;
	pport = pYxPort;
	
	for(i = 0; i< YX_PORT_NUM; i++)
	{
		pport->rev = 0;
		pport->mask = 0;

		pchn = &pYxChn[i][0];
		for (j=0; j<YX_NUM_PER_PORT; j++)
		{	
			
			mask = 1<<j;

			pport->rev |= mask;
			pport->mask |= mask;

 			if ((pDiBitMap[i] & mask) == 0) 
				pport->mask &= ~mask;
			else
			{
				pchn->yxno = pDiCfg[no].yxno;
				pchn->type = pDiCfg[no].type;
				pchn->cfg = pDiCfg[no].cfg;
				pchn->re_yxno= pDiCfg[no].re_yxno;

				if (pchn->type == DITYPE_PULSE) 
					pchn->dtime = (DEFAULT_PULSE_WIDTH*1000+(YX_FILTER_TM_UNIT-1))/YX_FILTER_TM_UNIT;
				else
					pchn->dtime = (pDiCfg[no].dtime*1000+(YX_FILTER_TM_UNIT-1))/YX_FILTER_TM_UNIT;

				if (pchn->cfg & DICFG_REV) pport->rev &= ~mask;
				if (pchn->cfg & DICFG_DIS) pport->mask &= ~mask;

				if (pchn->yxno < YX_NUM)
				{
					g_YxNo2Port[pchn->yxno].pchn = pchn;
					g_YxNo2Port[pchn->yxno].port = i;
					g_YxNo2Port[pchn->yxno].bit = mask;
				}

				no++;
				if (no >= g_Sys.MyCfg.wDINum) break;

			}
			pchn++;
		}
		
		if (no >= g_Sys.MyCfg.wDINum) break;
		pport++;
	}
	
}
void yxInit(void)
{
    int i,type;
	VDefDiCfg *pDefDiCfg;

	gYxInit = 0;

	memset(g_DiValue, 0, sizeof(g_DiValue));
	memset(g_YxNo2Port, 0, sizeof(g_YxNo2Port));
	memset(pYxChn, 0, sizeof(pYxChn));
	memset(pDYxChn, 0, sizeof(pDYxChn));
	memset(pYxPort, 0, sizeof(pYxPort));
	memset(pDiBitMap, 0, sizeof(pDiBitMap));
	memset(&gSoebuf, 0, sizeof(VSoeBuf));
	
	for (i=0; i<BSP_IO_BOARD; i++)
	{
	     type = Get_Io_Type(i);
		 if (type == 0xFF) continue;
		 
	     pDefDiCfg = GetDefDiCfg(type);
		 if (pDefDiCfg)
		 {
		     if ((YX_PORT_NUM + pDefDiCfg->portnum) > MAX_YX_PORT_NUM) break;
		     memcpy((BYTE*)(pDiBitMap+YX_PORT_NUM),  (BYTE*)(pDefDiCfg->di_bitmap), pDefDiCfg->portnum*2);
			 YX_NUM += pDefDiCfg->dinum;
			 YX_PORT_NUM += pDefDiCfg->portnum;
		 }
	}	

	pDiCfg = g_Sys.MyCfg.pDi;
		
	diCfgAddrSet();

	yxChnInit();
	
	gYxInit = 1;

}

void yxFilter(void)
{    
	FAST WORD cur_state;
	FAST WORD flt_flag;
	FAST WORD chgbit;
	FAST WORD chkbit;
	FAST int i,j,value;
	FAST WORD *pval,*prawval;
	FAST VYxPort *pport;
	FAST VYxChn *pchn;
	FAST VDYxChn *pdchn;
	FAST VSOEREC *psoe;
	FAST int yxing;
				
	if (gYxInit == 0) return;

    yxing = 0;

    pport = pYxPort;
	pval = g_DiValue;
	prawval = g_RawDiValue;
	for(i=0; i<YX_PORT_NUM; i++)
	{
        cur_state = YX_INPUT(pport->addr)^pport->rev;
		cur_state &=  pport->mask;
		*prawval = cur_state;

		chgbit = cur_state^pport->cur_state;  /* 异或，找到不同的输入*/ 
		pport->cur_state = cur_state;         /* 最后的输入 */

		flt_flag = pport->flt_flag;		  /* 滤波通道 */
		flt_flag |= chgbit;		          /* 变化通道加滤波通道 */

		if (flt_flag)		/* 有变化的或者是正在滤波阶段的 */					   
		{
			pchn = &pYxChn[i][0];
			pdchn = &pDYxChn[i][0];
			pport->flt_flag = flt_flag;		/* 归入滤波行列 */	

			chkbit = 1;  
			for (j=0; ; j++)
			{
				if (flt_flag & chkbit)  
				{
					if (chgbit & chkbit)  /* 发生变化 */
					{
						pchn->cnt = pchn->dtime+1;		/* 重置变化通道计数器 */
						//pchn->chg_tm = 	GetMsCnt();  	/* 获得开入通道变化时间 */
						//pchn->chg_tm.dwMinute = gpCalTime->dwMinute;
						//pchn->chg_tm.wMSecond = gpCalTime->wMSecond;
						GetSysClock(&pchn->chg_tm, CALCLOCK);
					}

					if (!pchn->cnt)		/* 去抖动脉冲计数结束 */ 
					{
						pport->flt_flag &= ~chkbit;		/* 清除标志 */	
						
                        if ((cur_state & chkbit) != (*pval & chkbit))
						{
							pchn->value = value = (cur_state & chkbit) ? 1:0;  /* DI输入确认 */
                            if (value)
								*pval |= chkbit;								
							else 
								*pval &= (~chkbit);

							if (pchn->type == DITYPE_PULSE)
                            {
                             //
							}
							else //不管单双点均写单点
							{
	                            psoe = &gSoebuf.soe[gSoebuf.wp];
								psoe->type = DITYPE_SYX;
								psoe->pchn = pchn;
								psoe->value1 = value;
								psoe->chg_tm = pchn->chg_tm;
								gSoebuf.wp = (gSoebuf.wp+1)&(SOE_REC_BUF_SIZE-1);
								yxing = 1;
							}

							//双点, 对方已变化完毕
							if ((pchn->type == DITYPE_DYX) && (pdchn->pair->cnt == 0))
							{
								psoe = &gSoebuf.soe[gSoebuf.wp];
								psoe->type = DITYPE_DYX;
								psoe->pchn = pchn;
								
								if (pchn->yxno < pdchn->pair->yxno)
								{
									if((pchn->cfg & DICFG_HW) &&  ((pdchn->pair->cfg & DICFG_HW) == 0))//配置为合位
									{
										psoe->value1 = pdchn->pair->value;
										psoe->value2 = value;
									}
									else
									{
										psoe->value1 = value;
										psoe->value2 = pdchn->pair->value;
									}
								}
								else
								{
									if(((pchn->cfg & DICFG_HW) == 0) && (pdchn->pair->cfg & DICFG_HW)) //配置为分位
									{
										psoe->value1 = value;
										psoe->value2 = pdchn->pair->value;
									}
									else
									{
										psoe->value1 = pdchn->pair->value;
										psoe->value2 = value;
									}
								}									
								psoe->chg_tm = pchn->chg_tm;
								gSoebuf.wp = (gSoebuf.wp+1)&(SOE_REC_BUF_SIZE-1);
								yxing = 1;									
							}								
						}
					}
					else
						pchn->cnt--; /* 通道滤波计数减一 */
			
					flt_flag &= ~chkbit; 	/* 检测过的清零 */ 
					if (!flt_flag) 		/* 判断所有的位是不是检测完 */
						break;
				}

		        pchn++;
				pdchn++;
				chkbit <<= 1;		/* 检测位左移 */ 
			}
		}

		WriteMyDiValue(i,*pval);
		pval++;
		prawval++;
        pport++; 
	}
	if(yxing)
		evSend(SELF_DIO_ID, EV_UFLAG);
}

//定时操作，
void yxWrite(void)
{
	WORD wp;
	struct VDBDSOE DBDSOE;
	struct VDBSOE DBSOE;
	VSOEREC *psoe;

	if(gYxInit == 0)
		return;
	
	while (gSoebuf.rp != gSoebuf.wp)
	{
        wp = gSoebuf.wp;
		
		while(gSoebuf.rp != wp)
		{
	        psoe = gSoebuf.soe+gSoebuf.rp;
#ifdef INCULDE_COS_CFG		
	        if ((psoe->pchn->cfg&DICFG_NSOE) == 0)
#endif        
	        {
				if (psoe->type == DITYPE_SYX)
				{
					DBSOE.wNo = psoe->pchn->yxno;
					DBSOE.byValue = (psoe->value1<<7)|0x01;
				
					DBSOE.Time = psoe->chg_tm;
					WriteSSOE(BM_EQP_MYIO, DBSOE.wNo, DBSOE.byValue, DBSOE.Time);	
				}	
				else
				{
					//双点遥信点号为单点/2
					if((psoe->pchn->cfg)& DICFG_SYX)
					{
						DBDSOE.wNo = psoe->pchn->dyxno;
					}
					else
					{
						DBDSOE.wNo = psoe->pchn->yxno>>1;
					}
					DBDSOE.byValue1 = (psoe->value1<<7)|0x01;
					DBDSOE.byValue2 = (psoe->value2<<7)|0x01;
					DBDSOE.Time = psoe->chg_tm;
					WriteDSOE(BM_EQP_MYIO, DBSOE.wNo, DBDSOE.byValue1,DBDSOE.byValue2,DBDSOE.Time);	
				}
	        }	
			gSoebuf.rp = (gSoebuf.rp+1)&(SOE_REC_BUF_SIZE-1);
		}
	}	
}

int GetMyDiValue(int no)
{
	int port;
	WORD bit;

	if ((no < 0) || (no >= (MAX_YX_PORT_NUM*MAX_YX_NUM_PER_PORT)))  return 0;

	port = g_YxNo2Port[no].port;
	bit = g_YxNo2Port[no].bit;

    if (g_DiValue[port]&bit) return 1;
	else return 0;
}
#endif /*INCLUDE_YX*/

