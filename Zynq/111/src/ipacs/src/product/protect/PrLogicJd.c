/*------------------------------------------------------------------------
 Module:       	prlogicjd.c
 Author:        helen
 Project:       
 State:			
 Creation Date:	2016-8-9
 Description:   仅针对FTU
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
------------------------------------------------------------------------*/
#include "syscfg.h"
#ifdef INCLUDE_JD_PR

#include "sys.h"
typedef struct
{
	struct VCalClock time;
    BYTE fd;
	BYTE byBufUsed;
	BYTE byBufOK;
	WORD wWavRcdCount;   //已记录的数据点数
	WORD wWavRcdNeed;    //需要记录的点数
	WORD wSamRcdCnt;
	short *buffer;
	TIMERELAY tSdchjs;
}VRecJd;
typedef struct
{
	BYTE vvmode;
	BYTE computeU0; 
	BYTE computeI0; 
	short index[8];     //{U0,I0,Ua,Ub,Uc,Ia,Ib,Ic},vv接线下计算U0不处理
}VAiRec;
#define EVENT_I01 4
#define FIR_NUM            64
#if 1         // 8000Hz
#if 0
static short h_coef_lp[FIR_NUM] ={
     37, 7, -51, -135, -233, -327, -396, -420, 
	 -383, -279, -115, 89, 300, 478, 584, 585,
	 466, 231, -90, -445, -767, -982, -1022, -839,
	 -410, 251, 1093, 2035, 2976, 3805, 4423, 4753,
	 4753, 4423, 3805, 2976, 2035, 1093, 251, -410,
	 -839, -1022, -982, -767, -445, -90, 231, 466,
	 585, 584, 478, 300, 89, -115,  -279, -383, 
	 -420, -396, -327, -233, -135, -51, 7, 37};  //放大32768
static short h_coef_bp[FIR_NUM] =
	{180, 242, 233, 145, -15, -223, -445, -638, 
	-765, -800, -735, -584, -382, -178, -28, 20,
	-67, -297, -647, -1065, -1475, -1792, -1933, -1835,
	-1466, -832, 16, 992, 1982, 2865, 3527, 3881,
	3881, 3527, 2865, 1982, 992, 16, -832, -1466, 
	-1835, -1933, -1792, -1475, -1065, -647, -297,-67,
	20, -28, -178, -382, -584, -735, -800, -765,
	-638, -445, -223, -15, 145, 233, 242, 180};   //放大32768
#endif
#else
static short h_coef_lp[FIR_NUM] ={
     -25, -91, -177, -273, -361, -424, -443, -404,
     -304, -147, 48, 254, 437, 561, 595, 522,
     340, 66, -261, -588, -854, -995, -958, -710,
    -242, 423, 1234, 2117, 2981, 3733, 4288, 4583,
     4583, 4288, 3733, 2981, 2117, 1234, 423, -242,
     -710, -958, -995, -854, -588, -261, 66, 340,
     522, 595, 561, 437, 254, 48, -147, -304,
     -404, -443, -424, -361, -273, -177, -91,-25};  //放大32768
static short h_coef_bp[FIR_NUM] = 
{
    -295, -138, 190, 435, 392, 88, -229, -282,
    -6,	381, 539, 283, -239, -644, -619, -199,
    225, 198, -413, -1254, -1720, -1434, -613, 6,	
    -305, -1637, -3197, -3697, -2192, 1180, 5095, 7709,	
    7709, 5095, 1180, -2192, -3697, -3197, -1637, -305,	
    6, -613, -1434, -1720, -1254, -413, 198, 225,	
    -199, -619, -644, -239, 283, 539, 381, -6,	
    -282, -229, 88, 392, 435, 190, -138,-295};
#endif
#define AD_JD_MULTI   4
#define AD_JD_NUM     (MEA_SAM_POINT_N*AD_JD_MULTI)  
#define JUDGE_JD_NUM  (AD_JD_NUM/10)       //2ms数据
#define WAVE_JD_NUM   (AD_JD_NUM*5)        //前3后2
#define T_INTERVAL    125                  //125us

VAiRec gAiIndex[MAX_FD_NUM];

VRecJd *pWaveJd=NULL;
int g_jdinit = 0;

int *pU0_I0[2];

extern VPrRunInfo *prRunInfo;
extern VPrRunSet *prRunSet[2];
extern VPrSetPublic *prSetPublic[2];
extern VPrRunPublic *prRunPublic;
extern VFdProtCal *fdProtVal;
extern LogicElem *prLogicElem;


void StartWavJdRcd(int fd)
{
	VRecJd *pwave;

	if ((fd < 0) ||(fd >= fdNum)) return;
	if (gAiIndex[fd].vvmode) return;

    pwave = pWaveJd + fd;
     
    if (pwave->byBufUsed) return;

    pwave->fd = fd;
	pwave->wSamRcdCnt = (wMeaSamCnt - AD_JD_NUM*3)&(SAM_BUF_LEN-1);
	pwave->wWavRcdCount = 0;
	pwave->wWavRcdNeed = WAVE_JD_NUM;
	pwave->byBufUsed = 1;

}

void WaveJd_Interrupt(void)
{
    int i,index,fd,channel;
	VRecJd *pwave;
	VPrRunInfo *pRunInfo;
	VPrRunSet *pRunSet;

	if( !g_jdinit )   return;
    for (fd=0; fd<fdNum; fd++)	
	{
	    pwave = pWaveJd +  fd;
		if ((pwave->byBufUsed == 0)||(pwave->byBufOK == 1))
			continue;
		
		index = pwave->wWavRcdCount;
		pRunInfo = prRunInfo + fd;
		pRunSet = prRunSet[pRunInfo->set_no] + fd; 
		for (i=0; i<8; i++)
		{
		    channel = gAiIndex[fd].index[i];
			if (channel != -1)
			{
				pwave->buffer[i*WAVE_JD_NUM+index] = nMeaSam[pwave->wSamRcdCnt][channel];	
				if((pRunSet->bXdlFx) && ((i==1) || (i > 4)))
				{
					 pwave->buffer[i*WAVE_JD_NUM+index] = pwave->buffer[i*WAVE_JD_NUM+index] * (-1);
				}
			}
		}
		pwave->wWavRcdCount++;
		pwave->wSamRcdCnt++;
		pwave->wSamRcdCnt &= (SAM_BUF_LEN-1);
		if (pwave->wWavRcdCount == pwave->wWavRcdNeed)
		   pwave->byBufOK = 1;
    }
}

void FilterLow(short *data1, int *data2, int flag)
{
   int i;
	
#if 0
	 short *h_coef;
	   long long sum;
	   int j;

	 if (1)
	 	h_coef = h_coef_bp;
	 else
	 	h_coef = h_coef_lp;
#endif

#if 1

	for (i=0; i<(AD_JD_NUM+FIR_NUM); i++)
		data2[i] = data1[i+FIR_NUM/2];
#endif

#if 0 // gys   国网测试里面小电流没有用滤波
	 for (i=0; i<(AD_JD_NUM+FIR_NUM); i++)
	 {
	     sum = 0;
		 for (j=0; j<FIR_NUM; j++)
		 {
		    sum += ((long)h_coef[j]*data1[i+FIR_NUM-j])>>10;
		 }
		 if (flag == 0)
		    data2[i] = sum>>2;
		 else
		 	data2[i] = sum>>1;
	 }
#endif 
	return;
}

void PrJdJudgeInit(void)
{
     int i;
	 BOOL numU0,numI0,numI,numU;
	 VRecJd *pwave;

	 pWaveJd = malloc(sizeof(VRecJd)*fdNum);
	 if (pWaveJd == NULL) return;
	 memset(pWaveJd, 0, sizeof(VRecJd)*fdNum);

	
	 for (i=0; i<fdNum; i++)
	 {
	     pwave = pWaveJd + i;
		 pwave->buffer = malloc(WAVE_JD_NUM*8*2);
		 if (pwave->buffer == NULL) return;
		 memset(pwave->buffer, 0, WAVE_JD_NUM*8*2);
	 }
	 
	 pU0_I0[0] = malloc(WAVE_JD_NUM*4);
	 pU0_I0[1] = malloc(WAVE_JD_NUM*4);

	 if ((pU0_I0[0] == NULL) || (pU0_I0[1] == NULL))
	 	return;
	 memset(pU0_I0[0], 0, WAVE_JD_NUM*4);
	 memset(pU0_I0[1], 0, WAVE_JD_NUM*4);
	 memset(&gAiIndex, 0, sizeof(VAiRec)*MAX_FD_NUM);

	 for(i = 0; i<fdNum; i++)
	 {
		 gAiIndex[i].index[0] = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_U0];
		 gAiIndex[i].index[1] = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_I0];	

		 gAiIndex[i].index[2] = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UA];
		 gAiIndex[i].index[3] = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UB];
		 gAiIndex[i].index[4] = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UC];

		 gAiIndex[i].index[5] = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IA];
		 gAiIndex[i].index[6] = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IB];
		 gAiIndex[i].index[7] = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IC];
	
		 numU0 = numI0 = numI = numU = FALSE;
		 if( gAiIndex[i].index[0] > -1 )
		 	numU0 = TRUE; 
		 if( gAiIndex[i].index[1] > -1 )
			numI0 = TRUE;
		 if( (gAiIndex[i].index[2] > -1) && (gAiIndex[i].index[3] > -1) && (gAiIndex[i].index[4] > -1) )
		 	numU = TRUE;
		 if( (gAiIndex[i].index[5] > -1) && (gAiIndex[i].index[6] > -1) && (gAiIndex[i].index[7] > -1) )
		 	numI = TRUE;
				 
		 if ( (numU0 || numU) && ( numI0 || numI ))
		 {
			if( !numU0 )
			 	gAiIndex[i].computeU0 = 1;
			if( !numI0)
			 	gAiIndex[i].computeI0 = 1;
			gAiIndex[i].vvmode = 0;
		 }
		 else
         {
			 gAiIndex[i].vvmode = 1;
             WriteWarnEvent(NULL, SYS_ERR_CFG, i+1, "回线%d无法进行零流暂态保护判断", i+1);
		 }
	 }
	 g_jdinit = 1;
	 return;
}

int GetMutation(int fd)
{
    int i,j,ii;
	long cos,sin,vfd1,vfd2;
    float diff;
    short diff1,diff2;
	const long *pFAFre, *pFAFim;
    int U0break,I0break,Ubreak,Ibreak;
	short *pU0,*pI0, *pU, *pI;
	VRecJd *pwave = pWaveJd + fd;
    VPrRunInfo *pRunInfo = prRunInfo + fd;
	VPrRunSet *pRunSet = prRunSet[pRunInfo->set_no] + fd;
	U0break = I0break = Ubreak = Ibreak = WAVE_JD_NUM + 1;


	 if (pRunSet->bJrU0)   //小电流接地零压接入控制字
	 {
		 pU0 = pwave->buffer;
		 pI0 = pwave->buffer+WAVE_JD_NUM;
		 
		 for (i=AD_JD_NUM*2; i<WAVE_JD_NUM; i++)
		 {
			diff1 = pU0[i] - pU0[i-AD_JD_NUM];
			if (diff1 < 0) diff1 = -diff1;
			diff2 = pU0[i-AD_JD_NUM] - pU0[i-AD_JD_NUM*2];
			if (diff2 < 0) diff2 = -diff2;
			diff = (diff1 - diff2)*512/1.414;
		
			if (diff > pRunSet->dIUnK)   
			{
			   U0break = i;
			   break;
			}
		 }
		
		 for (i=AD_JD_NUM*2; i<WAVE_JD_NUM; i++)
		 {
			diff1 = pI0[i] - pI0[i-AD_JD_NUM];
			if (diff1 < 0) diff1 = -diff1;
			diff2 = pI0[i-AD_JD_NUM] - pI0[i-AD_JD_NUM*2];
			if (diff2 < 0) diff2 = -diff2;
			diff = (diff1 - diff2)*512/1.414;
			if (diff >pRunSet->dII0nK)
			{
			    I0break = i;
				break;
			}
		 }
	 }

	 if (pRunSet->bJrU)      //小电流接地电压接入控制字
	 {
		pFAFre = &laFAFre[0][0];	
        pFAFim = &laFAFim[0][0];
	    for (ii=0; ii<3; ii++)
	    {
	        pU = pwave->buffer + WAVE_JD_NUM*(ii+2);
			pI = pwave->buffer + WAVE_JD_NUM*(ii+5);

            cos=0;sin=0;
			for (j=0; j<FT_SAM_POINT_N; j++)
			{
			    diff1 = pU[(j+FT_DIFF)*AD_JD_MULTI*2] - pU[j*AD_JD_MULTI*2];		
			    cos += diff1*pFAFre[i];
		        sin += diff1*pFAFim[i];
	        }

	        sin >>=7;
	        cos >>=7;
	        vfd1 = AmXY(cos,sin);           //  第一周波 基波有效值

			cos=0;sin=0;
			for (j=0; j<FT_SAM_POINT_N; j++)
			{
			    diff1 = pU[AD_JD_NUM+(j+FT_DIFF)*AD_JD_MULTI*2] - pU[AD_JD_NUM+j*AD_JD_MULTI*2];		
			    cos += diff1*pFAFre[i];
		        sin += diff1*pFAFim[i];
	        }

	        sin >>=7;
	        cos >>=7;
	        vfd2 = AmXY(cos,sin);           //  第二周波 基波有效值

			for (i=AD_JD_NUM*2; i<WAVE_JD_NUM; i++)
			{
				diff1 = pU[i] - pU[i-AD_JD_NUM];
				if (diff1 < 0) diff1 = -diff1;
				diff2 = pU[i-AD_JD_NUM] - pU[i-AD_JD_NUM*2];
				if (diff2 < 0) diff2 = -diff2;
				diff = (diff1 - diff2)*512/1.414;
			
				if (diff > pRunSet->dIUnK + 1.2*(vfd1-vfd2))   //
				{
				   Ubreak = i;
				   break;
				}
			}

			cos=0;sin=0;
			for (j=0; j<FT_SAM_POINT_N; j++)
			{
			    diff1 = pI[AD_JD_NUM+(j+FT_DIFF)*AD_JD_MULTI*2] - pI[AD_JD_NUM+j*AD_JD_MULTI*2];		
			    cos += diff1*pFAFre[i];
		        sin += diff1*pFAFim[i];
	        }

	        sin >>=7;
	        cos >>=7;
	        vfd1 = AmXY(cos,sin);
			
			cos=0;sin=0;
			for (j=0; j<FT_SAM_POINT_N; j++)
			{
			    diff1 = pI[(j+FT_DIFF)*AD_JD_MULTI*2] - pI[j*AD_JD_MULTI*2];		
			    cos += diff1*pFAFre[i];
		        sin += diff1*pFAFim[i];
	        }

	        sin >>=7;
	        cos >>=7;
	        vfd2 = AmXY(cos,sin);

			for (i=AD_JD_NUM*2; i<WAVE_JD_NUM; i++)
			{
				diff1 = pI[i] - pI[i-AD_JD_NUM];
				if (diff1 < 0) diff1 = -diff1;
				diff2 = pI[i-AD_JD_NUM] - pI[i-AD_JD_NUM*2];
				if (diff2 < 0) diff2 = -diff2;
				diff = (diff1 - diff2)*512/1.414;
			
				if (diff > pRunSet->dIInK + 1.2*(vfd1-vfd2))  //
				{
				   Ibreak = i;
				   break;
				}
			}
	    }
	 }

	 U0break = (U0break > Ubreak) ? Ubreak : U0break;
	 I0break = (I0break > Ibreak) ? Ibreak : I0break;
/*
	 if (U0break < I0break)
	 {
		i = U0break;
		do{
	   	   diff1 = pU0[i]-pU0[i-1];
		   diff2 = pU0[i-1] - pU0[i-2];
		   diff3 = pU0[i-1] - pU0[i-3];
		   if (diff1 < 0) diff1 = -diff1;
		   if (diff2 < 0) diff2 = -diff2;
		   if (diff3 < 0) diff3 = -diff3; 
	       if(diff2*2 >= diff1 || diff3 >= diff1 )
		  	   i--;
	       else
	       	   break;	  
	     }while(i >= (AD_JD_NUM*2));
		U0break = i;
	 }
	 else
	 {
		j = I0break;
		do{
		   diff1 = pI0[j]-pI0[j-1]; 
		   diff2 = pI0[j-1]-pI0[j-2];
		   diff3 = pI0[j-1] - pI0[j-3];
		   if (diff1 < 0) diff1 = -diff1;
		   if (diff2 < 0) diff2 = -diff2; 
		   if (diff3 < 0) diff3 = -diff3; 
		   if(diff2*2 >= diff1 || diff3 >= diff1 )	   
			   j--;
		   else
		   	   break;
		}while(j >= (AD_JD_NUM*2));
	    U0break = j;   
	 }
*/
	 return U0break;
}


void PrJdJudge(void)
{	
	 int i,j,dc,sum,U0break,fd;
	 long long sum1,sum2,sum3,xx;
	 int setcoef;
	 long diff1;
	 short *pU0,*pI0;
	 int count = 0,c;
	 VFdCfg *pFd;
	 VRecJd *pwave;
	 VPrRunInfo *pRunInfo;
	 VPrRunSet *pRunSet;
	 VPrSetPublic *pPublicSet;
	 DWORD i0wait;
	 DWORD dwTDXJD;
	 LogicElem *pLogicLow;
	 VFdProtCal *pval;
	 
	 if(!g_jdinit )   return;
	
	 pPublicSet =  prSetPublic[prRunPublic->set_no];
	 
	 for (fd = 0; fd < fdNum; fd++)
	 {
	 	count = 0;
	     pwave = pWaveJd + fd;
		 pFd = pFdCfg + fd;
		 pRunInfo = prRunInfo + fd;
		 pRunSet = prRunSet[pRunInfo->set_no] + fd; 
		 pval = fdProtVal+fd;
		 if(!pRunSet->bXDLYB) continue;
		 
//		 if(pRunSet->dYb & PR_YB_I01) //去掉零流与小电流的关系
//		 	i0wait = pRunSet->dT0[0];
//		 else
		 	i0wait = 0;

		 dwTDXJD = pRunSet->dTDXJD;
		 
		 if((fd == pPublicSet->bySLLine) && pPublicSet->bFTUSD)
		 {
			 pLogicLow = prLogicElem + pPublicSet->bySLLine*GRAPH_NUM;
			 if(pLogicLow[GRAPH_JS].Output[BOOL_OUTPUT_TRIP])
				 dwTDXJD = pRunSet->dTJS;
			 else
				 dwTDXJD = pRunSet->dTDXJD;
		 }
		 
		 if (pRunInfo->fault.i0_sec)
		 {
         	if ((Get100usCnt() - pRunInfo->fault.i_cnt) > dwTDXJD )
			{
				pRunInfo->fault.i_cnt = Get100usCnt();
				pRunInfo->fault.i0_xdlgj = 1;
				if(pRunSet->bXDLYB)
				{
				   pRunInfo->fault.i0_xdltrip = 1;   
				}
				pRunInfo->fault.i0_sec = 0;
	        } 
			else if( pval->dU[3]<pRunSet->dU0 )   //gu 零压过压维持故障状态
			{
				pRunInfo->fault.i0_sec = 0;
				pRunInfo->fault.i0_xdlgj = 0;
				pRunInfo->fault.i0_xdltrip = 0;
			}
		 }
		 
		 if( pRunInfo->fault.i0_xdlgj ) 
		 {
		 	if ((Get100usCnt() - pRunInfo->fault.i_cnt) > 10000)
		 		pRunInfo->fault.i0_xdlgj = 0;
		 }
		 
    	 if (pRunInfo->fault.i0_xdltrip)
		 {
		    if ((Get100usCnt() - pRunInfo->fault.i_cnt) > 10000)
               pRunInfo->fault.i0_xdltrip = 0;
		 }

		 if (pRunInfo->fault.i0_start)
		 {
		    if ((Get100usCnt() - pRunInfo->fault.i_cnt) < (i0wait + 1000))
				continue;
			pRunInfo->fault.i0_start = 0;
		 }
		 
		 if (pwave->byBufOK == 0) continue;
//		 if (pRunInfo->vyx & PR_VYX_I0)  //去掉零流与小电流的关系
//		 {
//		    pwave->byBufOK = 0;
//			pwave->byBufUsed = 0;
//		 	continue;
//		 }
		 if (gAiIndex[fd].vvmode) continue;
		
		 if (gAiIndex[fd].computeU0 == 1)
		 {
		     for (i=0; i<WAVE_JD_NUM; i++)
		         pwave->buffer[i] = pwave->buffer[2*WAVE_JD_NUM+i] + pwave->buffer[3*WAVE_JD_NUM+i] + pwave->buffer[4*WAVE_JD_NUM+i];			 
		 }
		 if (gAiIndex[fd].computeI0 == 1)
		 {
			 for (i=0; i<WAVE_JD_NUM; i++)
				 pwave->buffer[WAVE_JD_NUM+i] = pwave->buffer[5*WAVE_JD_NUM+i] + pwave->buffer[6*WAVE_JD_NUM+i] + pwave->buffer[7*WAVE_JD_NUM+i];
		 }

		 pU0 = pwave->buffer;
		 pI0 = pwave->buffer+WAVE_JD_NUM;

		 for (i=0; i<WAVE_JD_NUM; i++)
		 {
		     if (pI0[i] < 0)
			 	dc = -pI0[i];
			 else
			 	dc = pI0[i];
			 if (dc > 10)
			 	break;
		 }
		 if (i == WAVE_JD_NUM)
		 {
		     pwave->byBufUsed = 0;
		     pwave->byBufOK = 0;
		     continue;
		 }

	     U0break = GetMutation(fd);

		 if ((WAVE_JD_NUM - U0break) <= (AD_JD_NUM-3))
		 {
		     pwave->byBufUsed = 0;
		     pwave->byBufOK = 0;
		     continue;
		 }


     	/*dc = 0;
	 for (j=0; j<AD_JD_NUM; j++)
	     dc += pI0[j];
	 dc = dc/AD_JD_NUM;*/
	 dc = pMeaGain[gAiIndex[fd].index[1]].b;
	 for (j=0; j<WAVE_JD_NUM; j++)
	 	pI0[j] -= dc;
	 /*dc = 0;
	 for (j=0; j<AD_JD_NUM; j++)
	     dc += pU0[j];
	 dc = dc/AD_JD_NUM;*/
	 dc = pMeaGain[gAiIndex[fd].index[0]].b;
	 for (j=0; j<WAVE_JD_NUM; j++)
	 	pU0[j] -= dc;
 
	 FilterLow(pU0+U0break-FIR_NUM, pU0_I0[0], 1);
	 FilterLow(pI0+U0break-FIR_NUM, pU0_I0[1], 0);
	 i = U0break;
	 
#if defined (_YIERCI_RH_) && defined (_TEST_VER_)
	 shellprintf("U before\n");
	  for (j=0; j<(AD_JD_NUM+FIR_NUM); j++)
	 {
	     shellprintf("%d ", pU0[i-FIR_NUM+j]);
		   if (((j+1)%8) == 0)
				 shellprintf("\n");
	 }
	 
	 shellprintf("U after\n");
	 for (j=0; j<(AD_JD_NUM+FIR_NUM); j++)
	 {
	     shellprintf("%d ", pU0_I0[0][j]);
		   if (((j+1)%8) == 0)
				 shellprintf("\n");
	 }

	 shellprintf("I before\n");
	  for (j=0; j<(AD_JD_NUM+FIR_NUM); j++)
	 {
	     shellprintf("%d ", pI0[i-FIR_NUM+j]);
		   if (((j+1)%8) == 0)
				 shellprintf("\n");
	 }
	 shellprintf("I after\n");
	 for (j=0; j<(AD_JD_NUM+FIR_NUM); j++)
	 {
	     shellprintf("%d ", pU0_I0[1][j]);
		   if (((j+1)%8) == 0)
				 shellprintf("\n");
	 }
#endif
     for( c=-1; c<2; c++   )
     {
	     sum1 = sum2 = sum3 = 0;
		 for (j=(FIR_NUM/2 +c); j<(FIR_NUM/2+JUDGE_JD_NUM + c); j++)
	     {
	         diff1 = pU0_I0[0][j+1] - pU0_I0[0][j-1];

			 sum1 += (long long)pU0_I0[1][j]*diff1;
			 sum2 += (long long)pU0_I0[1][j]*pU0_I0[1][j];
			 sum3 += (long long)diff1*diff1;
	     }
		 while ( (sum2 >= 1<<28) || (sum3 >= 1<<28) )
		 {
		    if(sum2>sum3)
				sum2 /=400; 
			else
				sum3 /=400;
			sum1 /= 20;
		 } 
		 xx = (long long)sum2*sum3;
		 while ( (xx >= 1<<28) ||  (sum1 >= 1<<28) || (sum1 <= -1<<28) )
		 {
		    xx /= 400;
			sum1 /= 20;
		 }
		 sum = Sqrt_Dword(xx);
		 setcoef = (int)(pRunSet->wXDLCoef / 100);
         shellprintf("sum1 = %d , sum = %d ,   %d\n", (long)sum1, (long)sum, setcoef);
		
		 if (sum1*100 <-(sum*setcoef))
		 {
		   count++;
		 }	
		 if(count >= 2)
		 {
			if(	pRunSet->bXDLGJ || pRunSet->bXDLTrip)
			{
			    pRunInfo->fault.i0_sec = 1;
				pRunInfo->fault.i_cnt = Get100usCnt();
				WriteActEvent(NULL,  EVENT_I01, fd, "%s小电流接地, %d, %d", pFd->kgname, (long)sum1, (long)sum);
			}
			break;
		 }
     }
     pwave->byBufUsed = 0;
	 pwave->byBufOK = 0; 
	}
}

#endif
