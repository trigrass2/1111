#ifndef _PRCAL_H
#define _PRCAL_H

#include "syscfg.h"
#define AIFREQ_BUF_LEN            16
typedef struct{
    long x;
	long y;
}COMPLEX;

/*��ͬ��,0:ĸ��,1:��·*/
typedef struct{
	WORD  wTqFreq[2];
	WORD  wTqFSlip[2];
	DWORD dTqU[2];
	COMPLEX cTqU[2];
	BOOL  bYk;
}VAiTqVal;

typedef struct{
	BOOL  bCmpOrg;
	BYTE  index;
	COMPLEX u[AIFREQ_BUF_LEN][2];
	DWORD   dU[AIFREQ_BUF_LEN][2];
	long    lAng[AIFREQ_BUF_LEN][2];
}VAiFreqBuf;

typedef struct{
	WORD  index;
	BOOL  bSlipOrg;
	WORD  freq[AIFREQ_BUF_LEN][2];
	int   delta[AIFREQ_BUF_LEN][2];
	DWORD cnt[AIFREQ_BUF_LEN];
}VSlipFreqBuf;

typedef struct{
	COMPLEX UI;
	COMPLEX UIPre;
	COMPLEX SUI0;
    COMPLEX Uxx;       /*��ѹͨ������ߵ�ѹ*/
	

    /*���´�������*/
	DWORD dUI;		   /*aiͨ����Ӧ����Чֵ*/
	DWORD dSUI0;	   /*aiͨ����Ӧ��������Чֵ,��Ia Uaͨ����Ч*/
	DWORD dUxx; 	   /*aiͨ����Ӧ���ߵ�ѹ��Чֵ*/
	DWORD dUI_Hw;
	DWORD dUDelta;     /*��ѹ����*/
	DWORD dUI0Bak;     /*100ms֮ǰ�����������Чֵ*/
}VAiProtCal;	


typedef struct{
	DWORD *pxI[4];
	DWORD *pxU[4];
}VXiCfg;

typedef struct{
	COMPLEX xU[4];    /*�洢��ǰ���ϼ���ֵ*/
	COMPLEX xI[4];
}VXiProtcal;

typedef struct{
	DWORD dI[4];  //I��Чֵ

    DWORD dU[4];	

	DWORD dI0;
	DWORD dSI0;

	DWORD dU0;
	DWORD dSU0;

	DWORD dUxx[3];

	DWORD dIMax;

	DWORD dYx;

    DWORD dI_Hw2[4];

	DWORD dUDelta[3];
    DWORD dUI0Bak;     /*100ms֮ǰ�����������Чֵ*/
}VFdProtCal;

typedef struct{
    DWORD dU[4];	
	DWORD dYx;
}VPubProtCal;

typedef struct
{
	DWORD dBusI[3][24];     //{Ia,Ib,Ic}
	DWORD dBrkI[3];
}VBusProtCal;

myinline COMPLEX SIG(COMPLEX m)
{
   COMPLEX t;
   t.x = -m.x;
   t.y = -m.y;
   return t;
}

myinline COMPLEX ADD(COMPLEX m, COMPLEX n)
{
   COMPLEX t;
   t.x = m.x + n.x;
   t.y = m.y + n.y;
   return t;
}

myinline COMPLEX SUB(COMPLEX m, COMPLEX n)
{
   COMPLEX t;
   t.x = m.x - n.x;
   t.y = m.y - n.y;
   return t;
}

myinline COMPLEX MUL(COMPLEX m, COMPLEX n)
{
   COMPLEX t;
   t.x = m.x*n.x - m.y*n.y;
   t.y = m.x*n.y + m.y*n.x;
   return t;
}

myinline COMPLEX DIV(COMPLEX m, DWORD n)
{
   COMPLEX t;
   t.x = m.x/n;
   t.y = m.y/n;
   return t;
}

#ifdef INCLUDE_FA_S
typedef struct 
{
	WORD wU[4],wI[4];
	BYTE  byYx[8],bComm;
}VFasimData;

int FaValtoFd(void);
void FaSimWriteYcUIset(int SwID,VFasimData*pbuf);
void FaSimWriteYxUIset(int SwID,VFasimData*pbuf);
#endif

#define F_AMP(xVal)  AmXY(xVal.x, xVal.y)
#define F_ANG(xVal, u)  AngPQ(xVal.x, xVal.y, u)

void protValtoFd(void);
void protValtoPub(void);
void protCal_AiFVal(void);
void protCal_AiVal(void);
void createProtFdRef(void);
void createProtFdGain(void);
int RD_Last_P(int type, WORD pts, COMPLEX *pCmp);
void protBusValtoFd(void);

#endif
