/*------------------------------------------------------------------------
 Module:        ai.h
 Author:        solar
 Project:       
 Creation Date:	2008-10-13
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/
#ifndef _AI_H
#define _AI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syscfg.h"
#include "para.h"

#define INCLUDE_YK_PR_DIS

enum{
	YC_TYPE_Null = -1, 
	
	YC_TYPE_Dc,
		
	YC_TYPE_Ua,
	YC_TYPE_Ub,
	YC_TYPE_Uc,
	YC_TYPE_U0,   /*����U0*/
	YC_TYPE_SU0,  /*�Բ�U0*/ 
		
	YC_TYPE_Ia,
	YC_TYPE_Ib,
	YC_TYPE_Ic,
	YC_TYPE_I0,
	YC_TYPE_SI0,  /*�Բ�I0*/ 
		
	YC_TYPE_Pa,
	YC_TYPE_Pb,
	YC_TYPE_Pc,  
		
	YC_TYPE_Qa,  
	YC_TYPE_Qb,   
	YC_TYPE_Qc,    
		
	YC_TYPE_Uab,  
	YC_TYPE_Ubc,  
	YC_TYPE_Uca, 
		
	YC_TYPE_P,    
	YC_TYPE_Q,  
	YC_TYPE_S,    
	YC_TYPE_Cos,   
		
	YC_TYPE_Freq,
	
	YC_TYPE_SUa,  /*�Բ�Ua*/
	YC_TYPE_SUb,  /*�Բ�Ub*/
	YC_TYPE_SUc,  /*�Բ�Uc*/

	YC_TYPE_SIa,  /*�Բ�Ia*/
	YC_TYPE_SIb,  /*�Բ�Ib*/
	YC_TYPE_SIc,  /*�Բ�Ic*/

	YC_TYPE_Angle,
	
	YC_TYPE_FDc,
	YC_TYPE_SFreq,
	YC_TYPE_TEMETER,
	YC_TYPE_HYMETER,

	YC_TYPE_Har2 = 100, 
	YC_TYPE_Har3,   
	YC_TYPE_Har4,     
	YC_TYPE_Har5,    
	YC_TYPE_Har6,    
	YC_TYPE_Har7, 
	YC_TYPE_Har8, 
	YC_TYPE_Har9,
	YC_TYPE_Har10,
	YC_TYPE_Har11,
	YC_TYPE_Har12,
	YC_TYPE_Har13,
	YC_TYPE_Har1,

	
	YC_TYPE_Thd = 164,   //����
	
	YC_TYPE_Uunb = 200,
	YC_TYPE_Iunb,
	YC_TYPE_Upp,
	YC_TYPE_Udiff,
	YC_TYPE_Fdiff,
	YC_TYPE_UPhZ,
	YC_TYPE_UPhF,
	YC_TYPE_UPh0,
	YC_TYPE_IPhZ,
	YC_TYPE_IPhF,
	YC_TYPE_IPh0,
	YC_TYPE_I0T,
	YC_TYPE_I0V,
	YC_TYPE_I1T,
	YC_TYPE_I1V,

	YC_TYPE_Lrate,  //��为����
	YC_TYPE_Udot,   //��ѹԽ�������ۼ�ʱ�� 
	YC_TYPE_Udut,   //��ѹԽ�������ۼ�ʱ��
	YC_TYPE_Umqr,   //��ѹ�ºϸ���
	YC_TYPE_Umot,   //��ѹԽ�������ۼ�ʱ�� 
	YC_TYPE_Umut,   //��ѹԽ�������ۼ�ʱ��
	YC_TYPE_Uavg,   //��ѹ15����ƽ��ֵ

#if (TYPE_USER  == USER_FUJIAN)
	YC_TYPE_I1A =250, //����һ�ζ�ֵ
	YC_TYPE_I2A,
	YC_TYPE_I2T,
	YC_TYPE_I3A,
	YC_TYPE_I3T,
	YC_TYPE_I0A,
	YC_TYPE_CHT,
	YC_TYPE_CHFDT,
	YC_TYPE_GYV,
	YC_TYPE_GYT,
	YC_TYPE_GPHZ,
	YC_TYPE_GPT,
#endif
    YC_TYPE_SIGNAL = 300,
    YC_TYPE_TEMP =400,
    YC_TYPE_AttUa = 401,
    YC_TYPE_AttUb,
    YC_TYPE_AttUc,
    YC_TYPE_AttU0,
    YC_TYPE_AttIa,
    YC_TYPE_AttIb,
    YC_TYPE_AttIc,
    YC_TYPE_AttI0,
    YC_TYPE_AttPa,
    YC_TYPE_AttPb,
    YC_TYPE_AttPc,
    YC_TYPE_AttP,
    YC_TYPE_AttQa,
    YC_TYPE_AttQb,
    YC_TYPE_AttQc,
    YC_TYPE_AttQ,
};	

/*  AI type ��ȡ
   
	YC_TYPE_Ua       1
	YC_TYPE_Ub       2
	YC_TYPE_Uc       3
	YC_TYPE_U0       4   

	YC_TYPE_Ia       6
	YC_TYPE_Ib       7
	YC_TYPE_Ic       8
	YC_TYPE_I0       9   

ע��,����YC_TYPE_Uab YC_TYPE_Ubc YC_TYPE_Uca
����ʾ���ڵ����ѹ�ļ���ң��,���ʵ���ֳ�
���ߵ�ѹ����,��ͨ����ң�������Ӱ���������,
��ʱң�������в�Ӧ������������������*/

typedef struct {
    int type;
	DWORD cfg;

	DWORD addr;    /*??*/

	DWORD admax;
	DWORD admin;

    /*---�����Զ�����--*/
    int protect_enable;
    int recording;
	int pt_58v;    /*����ѹ��Ч*/

	int ycNo;
	int fdNo;
}VAiCfg;

typedef struct{
	int type;
    DWORD cfg;

	int   arg1;
	int   arg2;
	short toZero;

    /*---�����Զ�����--*/
	int cal;  /*�Ƿ����� ������ݲ����Զ����ң�����Ƿ���Ч
	            �����Pң�������Ϊ����,�����ʵ����Ч,������*/
	int fdNo;
	
	int xs1;
	int xs1_yc1;
	int xs2;	
	int k;   /*�Ŵ���*/

	int gainNo;

	int pair;  /*�Է�,��P��ӦQ�����, Q��ӦP�����, ������Ч*/

#ifdef INCLUDE_USWITCH
	int us_arg1;  /*��ѹ�л�ʹ��,P Q ��Ӧ�ĵ�ѹ���*/
#endif
}VYcCfg;	

enum{
	MMI_MEA_IA = 0,
	MMI_MEA_IB,
	MMI_MEA_IC,
	MMI_MEA_I0,
	MMI_MEA_UA,
	MMI_MEA_UB,
	MMI_MEA_UC,
	MMI_MEA_U0,
	MMI_MEA_AIF,
	MMI_MEA_NUM
};	

typedef struct{
	char kgname[GEN_NAME_LEN];
	DWORD kgid;

	DWORD cfg;   /*D0 = 1  ��ѹ�ȼ�Ϊ��ѹ  D0=0 ��ѹ�ȼ�Ϊ��ѹ(Ĭ��)
	               D1 = 1  ����һ�β������ϴ�ʱ����һλ��Чλ(��Ĭ��)
				   D2 = 1  ����һ�β๦���ϴ�ʱ��λΪMw��Mvar(��Ĭ��)
				   D3 = 1  �޹�����������Ч  D0=0 ���޹�����(Ĭ��)
				   D4 = 1  ���ϼ����Ч =0 ���ϼ����Ч(Ĭ��)
				   D5 = 1  ��ѹ����Ϊ���ѹ =0 ����Ϊ�ߵ�ѹ(Ĭ��)
				 */

	int Un;      /*���ζ��ѹ:220V��100V,ֵȡ220��100*/
    int In;      /*���ζ����:5A��1A,ֵȡ5��1*/
	int In0;
	
	int pt;       /*PT���ã�X/Un*/
	int ct;       /*CT���ã�X/In*/
	int ct0;

	int kg_stateno;
	int kg_faultno;
	int kg_ykno;
	int kg_vectorno;
	int kg_openno;
	int kg_closeno;
    int kg_startno;
	int kg_unlockno;
	int kg_ykkeepno;
	int kg_remoteno;

    /*---�����Զ�����--*/
#ifdef INCLUDE_FA_PUB
    int kg_fabsno[8];
    WORD fabs_eqpid[8];
#endif
	
	//��Чֵ��ַ
	DWORD *pdI[4];  
    DWORD *pdU[4];
	DWORD *pdUSlip[3];
    DWORD *pdUI0Bak;
    DWORD *pdI_Hw2[4];
    DWORD *pdI0;
	DWORD *pdSI0;   /*S��ʾ3*/

	DWORD *pdU0;
	DWORD *pdSU0;   /*S��ʾ3*/

    /*Uab Ubc Uca*/
	DWORD *pdUxx[3];

	//������Чֵaiͨ��������
	int mmi_meaNo_ai[MMI_MEA_NUM];
	//������Чֵyc������
	int mmi_meaNo_yc[MMI_MEA_NUM];
	
    //��Ӧgain
	long gain_u[4];
	long gain_i[4];
	long gain_u0;
	long gain_i0;

	BYTE valid;
	
#ifdef INCLUDE_YK_PR_DIS
	int yk_trip_dis;        //��բ��ֹ
	int yk_close_dis;       //��բ��ֹ
#endif
}VFdCfg;	

typedef struct{
	char *name;
	char *name_tab;
	char *unit;
	DWORD cfg;
	float k;
}VDefTVYcCfg;	

typedef VDefTVYcCfg VDefTVDdCfg;

typedef struct{
	char *name;
	char *name_tab;
	int id;
}VDefTVYxCfg_Fd;	

typedef VDefTVYxCfg_Fd VDefTVYkCfg_Fd;

typedef struct{
    DWORD ext_type;

	DWORD admask;
	
	WORD ainum;
	WORD ycnum;
	WORD fdnum;

	struct VPAiCfg *paicfg;
	struct VPYcCfg *pyccfg;
	struct VPFdCfg *pfdcfg;

	WORD ycnum_public;
	struct VPYcCfg *pyccfg_public;
	
	int tvyccfgnum;
	VDefTVYcCfg *ptvyccfg;

	int tvyccfgnum_public;
	VDefTVYcCfg *ptvyccfg_public;

	int tvddcfgnum;
	VDefTVDdCfg *ptvddcfg;

	int tvyxcfgnum_public;
	VDefTVYxCfg_Fd *ptvyxcfg_public;

	int tvyxcfgnum_fd;
	VDefTVYxCfg_Fd *ptvyxcfg_fd;		

	int tvykcfgnum_fd;
	VDefTVYxCfg_Fd *ptvykcfg_fd;		
}VDefAiCfg;	

VDefAiCfg *GetDefAiCfg(DWORD type);
int GetAdCfg(DWORD type, DWORD *pAdMask, int *pChnNum);
int GetDefPAiCfg(DWORD type, WORD ainum, struct VPAiCfg *pPCfg);
int GetDefPYcCfg(DWORD type, WORD ycnum, struct VPYcCfg *pPCfg);
int GetDefPFdCfg(DWORD type, WORD fdnum, struct VPFdCfg *pPCfg);
VDefTVYcCfg *GetDefTVYcCfg(DWORD type, int *pnum);
VDefTVDdCfg *GetDefTVDdCfg(DWORD type, int *pnum);
VDefTVYxCfg_Fd *GetDefTVYxCfg_Fd(DWORD type, int *pnum);
VDefTVYkCfg_Fd *GetDefTVYkCfg_Fd(DWORD type, int *pnum);
VDefTVYcCfg *GetDefTVYcCfg_Public(DWORD type, int *pnum);
VDefTVYxCfg_Fd *GetDefTVYxCfg_FdPublic(DWORD type, int *pnum);
VDefAiCfg *GetDefExtAiCfg(DWORD type);
int GetDefExtPAiCfg(DWORD type, WORD ainum, struct VPAiCfg *pPCfg);
int GetDefExtPYcCfg(DWORD type, WORD ycnum, struct VPYcCfg *pPCfg);
int GetDefExtPFdCfg(DWORD type, WORD fdnum, struct VPFdCfg *pPCfg);
int GetDefPYcCfg_Public(DWORD type, WORD ycnum_public, struct VPYcCfg *pPCfg);
VDefTVYcCfg *GetDefExtTVYcCfg(DWORD type, int *pnum);
VDefTVDdCfg *GetDefExtTVDdCfg(DWORD type, int *pnum);
VDefTVYxCfg_Fd *GetDefExtTVYxCfg_Fd(DWORD type, int *pnum);
VDefTVYkCfg_Fd *GetDefExtTVYkCfg_Fd(DWORD type, int *pnum);
VDefTVYcCfg *GetDefTVYcCfg_Public(DWORD type, int *pnum);

#ifdef __cplusplus
}
#endif

#endif

