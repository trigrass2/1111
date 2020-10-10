/*------------------------------------------------------------------------
 Module:        ft.h
 Author:        solar
 Project:       
 Creation Date:	2008-09-23
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/
#ifndef _FT_H
#define _FT_H

#include "ad.h"

#define FT_DIFF      3
#define FT_BUF_LEN   128

#if 0
#define FREQ_FOLLOW_NUM      21
#define FREQ_STEP            25
#define FREQ_MAX             5250
#define FREQ_MIN             4750
#define FT_SAM_POINT_MAX     21
typedef struct
{
   BYTE samples;
   WORD freq;
   long laFreqFre[FT_SAM_POINT_MAX];
   long laFreqFim[FT_SAM_POINT_MAX];
}FFT_FACTOR;

extern WORD wFtFreq;
extern FFT_FACTOR laFactor[FREQ_FOLLOW_NUM];
#endif
extern long laFAFre[9][FT_SAM_POINT_N];
extern long laFAFim[9][FT_SAM_POINT_N];
extern long laFAFim2[13][MEA_SAM_POINT_N];
extern long laFAFre2[13][MEA_SAM_POINT_N];
extern DWORD ASin[];

extern volatile WORD  wFtPtr;  
extern volatile long  lFt[FT_BUF_LEN][MAX_AI_NUM*2]; //递归富氏缓冲区,MAX_AI_NUM*2个实、虚部分量
extern volatile long  lFt0[MAX_AI_NUM*2];		     //递归富氏暂存值
extern volatile long  lFtBus[FT_BUF_LEN][36];
extern volatile short nFtBus[SAMFT_BUF_LEN][3];  //Ia,Ib,Ic

void InitGenFlags(void);
DWORD AmXY(long x, long y);
long AngPQ(long x, long y, DWORD rmt);
BYTE UIAngpb(long lAngTemp);
WORD Sqrt_Dword(DWORD x);

void protectCal_ft(void);
void protectCal_Correct(int flag);

#if 0
unsigned long _Mul_Div_U(unsigned long n,unsigned long a,unsigned long b);
long _MulFac2(long n,long a,long b);
long _MulFac(long n,long f);
DWORD _abs(long x);
#endif

#endif


