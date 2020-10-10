/*------------------------------------------------------------------------
 Module:		ad.c
 Author:		
 Project:		
 Creation Date: 2020-05-12
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/

#include "syscfg.h"
#include "myio.h"

#ifdef INCLUDE_YC

#include "ad.h"
#include "ai.h"

#ifdef INCLUDE_PR
#include "PrLogic.h"
#include "PrLogicJd.h"
#endif

volatile short nMeaSam[SAM_BUF_LEN][AITOTAL_NUMBER];  //测量采样数据区 //这块暂时先这样只取第一个port遥信
volatile WORD wMeaSamCnt; //测量采样指针

volatile short nFtSam[SAMFT_BUF_LEN][AITOTAL_NUMBER];  //FT采样数据区
volatile WORD wFtSamCnt; //FT采样指针


DWORD adMask;
int aiChnNum, adErr;

extern int g_init;

#ifdef INCLUDE_RECORD
	extern void WAV_Record_Interrupt(void);
#endif

extern void axi_sv_init(void);

void test_sv_rcv(void);

void Print_fpga_sv_cfg(void);

//调用科技ad初始化函数
void adInit(void *pGain)
{
	int i,chnum;
	DWORD mask; 

	InitGenFlags();

    aiChnNum = 0;
	adMask = 0;
	for (i=0; i<BSP_AC_BOARD; i++)
	{
	    GetAdCfg((g_Sys.MyCfg.dwAIType>>(i*8))&0xEF, &mask, &chnum);
		aiChnNum += chnum;
	}
	mask = (aiChnNum + 7)/8;
	for (i=0; i<mask; i++)
		adMask |= 0x01<<i;
	
	adErr = 0;
	
	axi_sv_init();
	//test_sv_rcv();
	Print_fpga_sv_cfg();
}

#endif

//新建一个定时器,500us 用来跑yx、yk等
DWORD gUsCnt = 0;
void scantime500us(void)
{	
#ifdef INCLUDE_YX
	yxFilter();
#endif

#ifdef INCLUDE_YK
	ykScanOnTimer();
#endif

#ifdef INCLUDE_MYGOOSE_TX
	if (gUsCnt & AD_GOOSE_FLAG)
		mygoose_tx_tick();
#endif
	gUsCnt++;
}

//数组,与通道非一一对应
volatile WORD svindex[] = 
{
	0 , 2 , 4, 6, 8, 10, 12 ,14,
	1 ,	3 , 5, 7, 9, 11, 13, 15,
	16 , 18, 20, 22, 24, 26, 28 ,30,
	17 , 19, 21, 23, 25, 27, 29, 31,
	32 , 34 , 36, 38, 40, 42, 44 ,46,
	33 , 35, 37, 39, 41, 43, 45, 47,
	48 , 50 , 52, 54, 56, 58, 60 ,62,
	49 , 51 , 53, 55, 57, 59, 61, 63,
};

void adInt(DWORD *nSamTmp)
{
    static DWORD cnt_2 = 0;
	FAST int counter;
	FAST WORD *w_ptr;

	if (gycInit == 0) return;
#ifdef INCLUDE_YC
	if (g_init == 0) return;
#endif
	cnt_2++;

	wMeaSamCnt++;
	wMeaSamCnt = wMeaSamCnt&(SAM_BUF_LEN-1);

	//传递数据至采样数据区
	counter=0;
	while(counter < AICHAN_NUMBER)
	{
		nMeaSam[wMeaSamCnt][counter] = (((short)nSamTmp[svindex[counter]]) >> 2);
		counter++;
	}

/*暂时没空间 ，以后再处理*/
			w_ptr = (WORD *)&(nMeaSam[wMeaSamCnt][AICHAN_NUMBER]);

#ifdef INCLUDE_YX
        for(counter=0; counter < MAX_YX_PORT_NUM; counter++)//MAX_YX_PORT_NUM
			*w_ptr++ = g_RawDiValue[counter];
#endif


#ifdef INCLUDE_JD_PR
	WaveJd_Interrupt();
#endif


#ifdef INCLUDE_RECORD
	WAV_Record_Interrupt();
#endif

#ifdef INCLUDE_PR   
	if (cnt_2  == AD_FT_FLAG)
	{
		cnt_2 = 0;
		wFtSamCnt++;
		wFtSamCnt = wFtSamCnt&(SAMFT_BUF_LEN-1);
		
		counter=0;
		while(counter < AICHAN_NUMBER)
		{
			nFtSam[wFtSamCnt][counter] = nMeaSam[wMeaSamCnt][counter];
			counter++;
		}
				w_ptr = (WORD *)&(nFtSam[wFtSamCnt][AICHAN_NUMBER]);

#ifdef INCLUDE_YX
        for(counter=0; counter<MAX_YX_PORT_NUM; counter++)
			*w_ptr++ = g_RawDiValue[counter];
#ifdef INCLUDE_PR
        *w_ptr++ = prGetBhqd();
#endif
#endif

#ifdef INCLUDE_YK
		*w_ptr++ = (WORD)(g_DoState[0]);
		*w_ptr++ = (WORD)(g_DoState[0]>>16);
		*w_ptr++ = (WORD)(g_DoState[1]);
		*w_ptr++ = (WORD)(g_DoState[1]>>16);
#endif
        protectCal_ft();
		
		//prTick(); //删除
	}
#endif

}

