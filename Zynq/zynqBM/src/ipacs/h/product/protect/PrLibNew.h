#ifndef _PRLIB_H
#define _PRLIB_H

#include "syscfg.h"
#include "PrCal.h"
#include "PrSet.h"
/* typedefs */
typedef  union
{
    long   lVal;
   	DWORD  ulVal;
  	BOOL   bVal;
}VALUE_TYPE;

typedef struct			/* δ��*/
{	
    BOOL bExistFatalErrFlag;            /* �������ش����־���������ش�����Ϊ�棬����Ϊ�٣���Ϊһ��״̬��־�� 2006-11-2��*/
    BOOL bHardTestEnterFlag;            /*Ӳ�����Խ����־,��Ϊ��,��ʾ�˴�Ӳ�����Խ���,�����˳����У���־Ϊ�棬�´ν���ͷ���Ϊ��
                                          ����Ϊһ�������־*/ 
    BOOL bHardTestExitFlag;             /*Ӳ�������˳���־,��Ϊ��,��ʾ�˴�Ӳ�������˳�,��������Ͷ������  */
    BOOL  bRecvNewFgCmdFlag;            /*���յ�1���µĸ��������Ϊ�棬��ʾ�˴�ɨ����յ�һ���¸�������´ν��룬�ͷ���Ϊ��
                                          ��Ϊһ�������־  һ���µĸ��������������ֻ��һ�Ρ� 2006-12-21�� */
	BOOL bSetChg;                       /* ����ɨ�趨ֵ�����˸ı� */
	BOOL bScanIntFlag;                  /* ɨ���жϱ�־��ɨ�費���� */																					
    DWORD ulScnTime;                    /* ���ο�ʼ�߼�ͼɨ���ʱ�̣�us������ֵ�� */
    DWORD ulScnInterval;                /* ����ɨ����ϴ�ɨ���ʱ������us�� */ 
} EP_CHART_MSG;


typedef struct tag_EP_ELEMENT
{
    BOOL         bSetChg;
    VALUE_TYPE  *ppioIn;               
    VALUE_TYPE  *aioOut;
	DWORD        aulUser[8];
	void        *apvUser[2];
    void       (*Scan_Func)(struct tag_EP_ELEMENT *pelm);
} EP_ELEMENT;

typedef struct                           //timer relay struct	
{
	DWORD	dTripThreshold;
	DWORD	dRetThreshold;
	BOOL	boolTrip;
	BOOL	boolStart;
	DWORD	dTimer;
	DWORD	dTimerRetTimer;
	DWORD	dTRTThreshold;               //ʱ��̵�����ʱ�з���ʱ��
}TIMERELAY;

typedef struct INVR_CONFIG_STRUCT
{
	DWORD		_IP, _CUV, _TINV;
}	INVR_CONFIG;

typedef struct INV_RELAY_STRUCT
{
	INVR_CONFIG	config;

	DWORD   	flags;		
	DWORD		integral;
	DWORD   	tick;

}	INV_RELAY;

typedef struct INVR_3PHASE_STRUCT
{
	INV_RELAY	p[3];
}	INVR_3PHASE;

typedef struct
{
    TIMERELAY tMaxTimeRelay[8];
}TMAXTIMERELAY;

#define  PR_I_T_DROP   0
#define  PR_U_T_DROP   0
#define  PR_SET_UP     105
#define  PR_SET_DOWN   94
/*����*/
/* input signal */
enum{
_PR_21_ENABLE=0,_PR_21_IA, _PR_21_IB, _PR_21_IC, _PR_21_IA_PICK,_PR_21_IB_PICK,_PR_21_IC_PICK,
_PR_21_T_PICK=7,_PR_21_ABC_DIR, _PR_21_BLOCKED, _PR_21_CUV, _PR_21_INVMODE, _PR_21_I0MODE, _PR_21_INPUT
};

/* output signal */
enum{
_PR_21_PICKUP =0, _PR_21_OPERATE, _PR_21_A_OPT, _PR_21_B_OPT, _PR_21_C_OPT, _PR_21_OUTPUT
};

/*��ѹ����*/
/*input signal*/
enum{
_PR_22_ENABLE=0, _PR_22_UAB, _PR_22_UBC, _PR_22_UCA, _PR_22_UDELTA, _PR_22_UDELTB, _PR_22_UDELTC,
_PR_22_UAB_PICK=7, _PR_22_UBC_PICK, _PR_22_UCA_PICK, _PR_22_UDELTA_PICK, _PR_22_UDELETB_PICK, _PR_22_UDELTC_PICK,
_PR_22_UMINA=13, _PR_22_UMINB, _PR_22_UMINC, _PR_22_TWJ,_PR_22_T_PICK, _PR_22_TYY_PICK,
_PR_22_BLOCKED=19,_PR_22_PTU, _PR_22_INPUT
};

/*output signal*/
enum{
_PR_22_PICKUP=0,_PR_22_DYBS,_PR_22_WY,_PR_22_OPERATE,_PR_22_YY_OPERATE,_PR_22_OUTPUT
};

/*����Ԫ��*/
/* input signal */
enum {
_PR_23_I_ENABLE=0,_PR_23_I0_ENABLE,_PR_23_3U0,_PR_23_VVMODE,_PR_23_PANG1,_PR_23_PANG2, 
_PR_23_NANG1=6, _PR_23_NANG2, _PR_23_IACH, _PR_23_IBCH, _PR_23_ICCH, _PR_23_UACH, _PR_23_UBCH, _PR_23_UCCH, _PR_23_INPUT
};
/* output signal */
enum {
_PR_23_A_DIR =0, _PR_23_B_DIR, _PR_23_C_DIR, _PR_23_N_DIR,_PR_23_OUTPUT
};

/*PT����*/
/*input signal*/
enum{
 _PR_24_ENABLE=0, _PR_24_VVMODE, _PR_24_INPUT
};
/*output signal*/
enum{
 _PR_24_PICKUP =0, _PR_24_OPERATE, _PR_24_OUTPUT
};

/*����ѹ*/
/*input signal*/
enum{
_PR_25_ENABLE=0, _PR_25_UAB, _PR_25_UBC, _PR_25_UCA, _PR_25_BLOCKED,_PR_25_INPUT
};
enum{
_PR_25_PICKUP=0, _PR_25_OPERATE, _PR_25_YY, _PR_25_OUTPUT
};

/*�غ�բ*/
/*input signal*/
enum{
 _PR_26_ENABLE=0, _PR_26_CDTJ, _PR_26_FDTJ, _PR_26_QDTJ, _PR_26_DZTJ,
 _PR_26_T_CH=5, _PR_26_T_CH2, _PR_26_T_CH3, _PR_26_T_CH4, _PR_26_NUM_CH, _PR_26_INPUT
};
/*output signal*/
enum{
 _PR_26_CD_PICK=0, _PR_26_FD_PICK, _PR_26_QD_PICK, _PR_26_DZ_PICK, _PR_26_CH1_OPERATE, _PR_26_CH2_OPERATE,
 _PR_26_CH3_OPERATE=6, _PR_26_CH4_OPERATE, _PR_26_FAIL, _PR_26_T_1S, _PR_26_OUTPUT 
};

/*�͵���*/
/*input signal*/
enum{
_PR_27_ENABLE=0,_PR_27_CTRL_YL,_PR_27_T_WL,_PR_27_T_YL,_PR_27_INPUT
};
/*output signal*/
enum{
_PR_27_PICKUP=0,_PR_27_OPERATE,_PR_27_YL_OPERATE,_PR_27_OUTPUT
};
/*�������*/
enum{
_PR_28_ENABLE=0,_PR_28_INPUT
};
enum{
_PR_28_DLQD=0, _PR_28_DJQD, _PR_28_OUTPUT
};
/*ʧѹ*/
enum{
_PR_29_ENABLE=0, _PR_29_WL, _PR_29_UAB, _PR_29_UBC, _PR_29_UCA, _PR_29_INPUT
};
enum{
_PR_29_OPERATE=0, _PR_29_OUTPUT
};
/*��Ƶ����*/
enum{
_PR_30_ENABLE=0, _PR_30_UAB, _PR_30_UBC, _PR_30_UCA, _PR_30_FREQ, _PR_30_FSLIP, _PR_30_BLOCK, _PR_30_TIME, _PR_30_INPUT
};
enum{
_PR_30_PICK=0, _PR_30_OPERATE, _PR_30_OUTPUT
};
/*ͬ��*/
enum{
_PR_31_ENABLE=0, _PR_31_MFREQ, _PR_31_MFSLIP, _PR_31_XFREQ, _PR_31_XFSLIP, _PR_31_PRETQ, _PR_31_INPUT
};
enum{
_PR_31_OPERATE=0, _PR_31_ANGLE1, _PR_31_ANGLE2,_PR_31_OUTPUT
};

#ifdef INCLUDE_PR_U
/*������ѹ��բ*/
enum{
_PR_121_ENABLE=0, _PR_121_US, _PR_121_UF, _PR_121_QD, _PR_121_BLOCK, 
_PR_121_US1_PICK=5,_PR_121_UF1_PICK, _PR_121_US2_PICK, _PR_121_UF2_PICK,_PR_121_INPUT
};
enum{
_PR_121_OPERATE=0,_PR_121_X_BLOCK, _PR_121_U_BLOCK,_PR_121_FX, _PR_121_OUTPUT
};
/*����*/
enum
{
_PR_122_ENABLE=0, _PR_122_US, _PR_122_UF, _PR_122_US1_PICK, _PR_122_UF1_PICK,
_PR_122_US2_PICK=5, _PR_122_UF2_PICK, _PR_122_FAULT, _PR_122_GL, _PR_122_HZ_IN, 
_PR_122_FX=10, _PR_122_INPUT
};
enum{
_PR_122_Y_BLOCK, _PR_122_FZ_BLOCK, _PR_122_OUTPUT
};
enum
{
_PR_123_ENABLE=0, _PR_123_US, _PR_123_UF, _PR_123_US1_PICK,_PR_123_US2_PICK,  
_PR_123_UF1_PICK, _PR_123_UF2_PICK,_PR_123_INPUT
};
enum
{
_PR_123_SY_BLOCK, _PR_123_OUTPUT
};
enum{
_PR_124_ENABLE=0, _PR_124_US, _PR_124_UF,_PR_124_BLOCK, _PR_124_INPUT
};
enum{
_PR_124_OPERATE=0,_PR_124_FX, _PR_124_OUTPUT
};

int PRLIB_21(EP_ELEMENT *pelm);
void PRLIB_21_SCAN(EP_ELEMENT *pelm);
int PRLIB_22(EP_ELEMENT *pelm);
void PRLIB_22_SCAN(EP_ELEMENT *pelm);
int PRLIB_23(EP_ELEMENT *pelm);
void PRLIB_23_SCAN(EP_ELEMENT *pelm);
int PRLIB_24(EP_ELEMENT *pelm);
void PRLIB_24_SCAN(EP_ELEMENT *pelm);
int PRLIB_25(EP_ELEMENT *pelm);
void PRLIB_25_SCAN(EP_ELEMENT *pelm);
int PRLIB_26(EP_ELEMENT *pelm);
void PRLIB_26_SCAN(EP_ELEMENT *pelm);
int PRLIB_27(EP_ELEMENT *pelm);
void PRLIB_27_SCAN(EP_ELEMENT *pelm);
int PRLIB_28(EP_ELEMENT *pelm);
void PRLIB_28_SCAN(EP_ELEMENT *pelm);
int PRLIB_29(EP_ELEMENT *pelm);
void PRLIB_29_SCAN(EP_ELEMENT *pelm);
int PRLIB_30(EP_ELEMENT *pelm);
void PRLIB_30_SCAN(EP_ELEMENT *pelm);
int PRLIB_31(EP_ELEMENT *pelm);
void PRLIB_31_SCAN(EP_ELEMENT *pelm);
int PRLIB_121(EP_ELEMENT *pelm);
void PRLIB_121_SCAN(EP_ELEMENT *pelm);
int PRLIB_122(EP_ELEMENT *pelm);
void PRLIB_122_SCAN(EP_ELEMENT *pelm);
int PRLIB_123(EP_ELEMENT *pelm);
void PRLIB_123_SCAN(EP_ELEMENT *pelm);
int PRLIB_124(EP_ELEMENT *pelm);
void PRLIB_124_SCAN(EP_ELEMENT *pelm);

void  InitTR(TIMERELAY *pTimeRelay,DWORD dTripThreshold,DWORD dRetThreshold);
void  ResetTR(TIMERELAY *pTimeRelay);
void  RunTR(TIMERELAY *pTimeRelay,BOOL boolInput);
void  SetTR_TRTT(TIMERELAY *pTimeRelay,DWORD dTRTT);
void  PreSetTime(TIMERELAY *pTimeRelay,DWORD dPreTime);
#endif
#endif
