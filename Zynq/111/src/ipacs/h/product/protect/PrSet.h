#ifndef _PRSET_H
#define _PRSET_H

#include "syscfg.h"

#define PROT_SET_1             0x81                    /*һ��ֵ*/
#define PROT_SET_2             0x82
#define PROT_TYPE              PROT_SET_2
//
//-----------------------------��ֵ----------------------
//
#define PR_SET_SIZE             1024
#define PR_SET_NAME             64
//��ֵ���
enum
{
  SET_YB,
  SET_KG1,
  SET_KG2,
  SET_I1,
  SET_T1,
  SET_I2,
  SET_T2,
  SET_I3,
  SET_T3,
  SET_IANG1,
  SET_IANG2,
  SET_IM,
  SET_TDJQD,
  SET_IGFH,
  SET_TGFH,
  SET_IKG,
  SET_UGY,
  SET_TGY,
  SET_UDY,
  SET_TDY,
  SET_USLIP,
  SET_UYY,
  SET_UWY,
  SET_3U0,
  SET_3I01,
  SET_TI01,
  SET_3I02,
  SET_TI02,
  SET_3I03,
  SET_TI03,
  SET_I0ANG1,
  SET_I0ANG2,
  SET_U0TB,
  SET_I0TB,
  SET_ITB,
  SET_TDXJD,
  SET_XDLCOEF,
  SET_CHBSDL,
  SET_CHNUM,
  SET_TCH1,
  SET_TCH2,
  SET_TCH3,
  SET_TCHBS,
  SET_TKEEP,
  SET_LED_TKEEP,
  SET_TJS,
  SET_TJSFG,
  SET_YXTZ,//�ⲿң�Ŷ�Ӧ����բ

  SET_FGP, //��Ƶ��ֵ
  SET_TGP, //��Ƶʱ��
  SET_FDP, //��Ƶ��ֵ
  SET_TDP, //��Ƶʱ��
  SET_IHJS,  //����ٶ�ֵ
  SET_I0HJS,  //���������ֵ
  SET_I0TJS,
  SET_UUMX,  //ĸ�߹���ѹ��ֵ
  SET_TUMX,	 //ĸ�߹���ѹʱ��
  SET_UDMX,  //ĸ��Ƿ��ѹ��ֵ
  SET_TDMX,  //ĸ��Ƿ��ѹʱ��
  SET_NUM
};

enum
{
  SET_PUB_YB,
  SET_PUB_KG1,
  SET_PUB_KG2,
  SET_PUB_SLFD,
  SET_PUB_TX,
  SET_PUB_TY,
  SET_PUB_TFZ,
  SET_PUB_TU0, 
  SET_PUB_U0,
  SET_PUB_TSYCY,
  SET_PUB_UCY,
  SET_PUB_UDIFF,
  SET_PUB_TWY,
  SET_PUB_ICNT,
  SET_PUB_TICNT1,
  SET_PUB_TICNT2,
#ifdef INCLUDE_PR_PRO
  SET_PUB_TQFD,
  SET_PUB_FSLIP,
  SET_PUB_FREQ,
  SET_PUB_TDF,
  SET_PUB_UFREQ,
  SET_PUB_IFREQ,
  SET_PUB_TQDQJ,
  SET_PUB_TQTDQ,
  SET_PUB_TQUDIFF,
  SET_PUB_TQFDIFF,
  SET_PUB_TQFSDIFF,  
#endif
  SET_PUB_TC,
  SET_PUB_TS,
  SET_PUB_TXXCH,
  SET_PUB_TJD,
  SET_PUB_TJDRevs,
  SET_PUB_UANG,
  SET_PUB_RECORD_NUM,
  SET_PUB_DYLJ,
  SET_PUB_TDYLJ,
  SET_PUB_SLFD_ANOTHER,  //��һ������
  SET_PUB_IBUS,
  
  SET_PUB_LXFZ,//������բ      add by wdj
  SET_PUB_TLXFZ,

  SET_PUB_AI_S,
  SET_PUB_AI_F,
  
  SET_PUB_NUM
};

//��ֵ����
enum 
{
    SETTYPE_YB=0,
	SETTYPE_KG,
	SETTYPE_FACTOR,
	SETTYPE_U,
	SETTYPE_A,
	SETTYPE_S,
	SETTYPE_OHM,
	SETTYPE_DEGREE,
	SETTYPE_KMPEROHM,
	SETTYPE_HZ,
	SETTYPE_HZPERS,
	SETTYPE_UPERS,
	SETTYPE_IP,
	SETTYPE_DEC
};

//��ֵ���������أ�ֻ������д
enum
{
    SETOPT_HIDE,
	SETOPT_READ,
	SETOPT_WRITE
};

#ifdef INCLUDE_FA_S
enum
{
	PR_DLQ=1,
	PR_FHKG,
	PR_GLBGZ,
	PR_DYZ,
	PR_DLJSX,
	PR_ZSYX,
	PR_DLQCH,
	PR_CKTZ,
	PR_CKII,
	PR_BDZCH,
	PR_DLJSCH,
	PR_DYDLX,
	PR_CKI0TZ,
};
#endif

#define RELAY_DELAY_5MS      50
//��ֵ
//��ֵ����
#define VALUEATTRIB_SIG      (1<<7)     //bit7=signal
#define VALUEATTRIB_DOT3     (1<<2)
#define VALUEATTRIB_DOT2     (1<<1)     //bit 2-0=С��λ��
#define VALUEATTRIB_DOT1     (1<<0)     //

#define SETATTRIB_TYPEMASK    0x0f        
#define SETATTRIB_OPTMASK	  0xf0       //bit 3-0=��ֵ����

//����ÿ������ֵ�Ľṹ
//����byAttrib��������
//<7..4>:=��������
//<3..0>:=��ֵ����
//<7>   :=����λ
//<6..2>:=����
//<1..0>:=С��λ��(�Զ�������Ч)
//����ֵλʮ����ʱ,byValLo��byValHi��ѹ��BCD���ʾ
#pragma pack(1)
typedef struct tagTSETVAL
{
      BYTE bySetAttrib;
	  BYTE byValAttrib;
	  BYTE byValLol;
	  BYTE byValLoh;
	  BYTE byValHil;
	  BYTE byValHih;
} TSETVAL;

typedef struct tagTSETTABLE
{
	  char 	szName[18];
	  TSETVAL tMin;//��Сֵ
	  TSETVAL tMax;//���ֵ
	  TSETVAL tDef;//ȱʡֵ
} TSETTABLE;

#pragma pack()

//ѹ�����
enum
{
     YB_GZJC,
     YB_I1,
	 YB_I2,
	 YB_I3,
	 YB_GFH,
	 YB_GY,
	 YB_DY,
	 YB_I01,
	 YB_I02,
	 YB_I03,
	 YB_DJQD,
	 YB_PT,
	 YB_CH,
	 YB_TRIP,
	 YB_JS,
	 YB_GP, //��Ƶ
	 YB_TZ = 16, //�ⲿң�Ŵ�����բѹ��
	 YB_DP, //��Ƶ
	 YB_MXQY,//ĸ��Ƿѹ����
	 YB_SLT = 28,
	 YB_SLT_BAK
};

enum
{
     YB_SUDY = 0,
	 YB_SUSY,
	 YB_U0,
	 YB_UDIFF,
	 YB_SLWY,
	 YB_DLCNT,
	 YB_DF,
	 YB_SHTQ,
	 YB_JD,
	 YB_YJCY,
	 YB_BUS,
	 YB_LXFZ, //add by wdj
};

#pragma pack(1)
typedef struct tagTYBTABLE
{
      char szName[18];
	  BYTE byYbOpt;
	  BYTE byYbNo;
}TYBTABLE;
#pragma pack()

enum
{
     KG_0,
     KG_1,
	 KG_2,
	 KG_3,
	 KG_4,
	 KG_5,
	 KG_6,
	 KG_7,
	 KG_8,
	 KG_9,
	 KG_10,
	 KG_11,
	 KG_12,
	 KG_13,
	 KG_14,
	 KG_15,
	 KG_16=16,
	 KG_17,
	 KG_18,
	 KG_19,
	 KG_20,
	 KG_21,
	 KG_22,
	 KG_23,
	 KG_24,
	 KG_25,
	 KG_26,
	 KG_27,
	 KG_28,
	 KG_29,
	 KG_30,
	 KG_31,
	 KG_32=32
};

//���������
#pragma pack(1)
typedef struct tagTKGTABLE
{
      char szName[18];
  	  BYTE byKGOpt;
	  BYTE byKGBit[3];
	  BYTE byKGNum;
	  BYTE byMsNum;
	  char **szBit;
}TKGTABLE;

typedef struct tagTABLETYPE
{
      char  szName[18];
      DWORD dType;
	  DWORD dExt;
	  DWORD dRec[8];
}TTABLETYPE;

typedef struct{
	BYTE type;
	WORD total;
	WORD offset;
	WORD len;
	WORD rec[4];
}VPrSetHead;

#pragma pack()

#ifdef INCLUDE_61850
#define XML_DA_TYPE_INT            0
#define XML_DA_TYPE_VAL            1

#pragma pack(1)
typedef struct tagYBXMLTABLE
{
	  BOOL xml;
	  BOOL event;
	  BYTE inst;
	  char name[16];
	  char inclass[10];
	  char prefix[10];
	  BYTE commno[10];
	  BYTE relateno[10];   //��������ĳѹ�����
	  DWORD rec[5];
}TYBXMLTABLE;

typedef struct tagSETXMLTABLE
{
	  BYTE yb;
	  char doName[16];
	  char name[32];
	  BYTE daType;
	  DWORD rec[5];
}TSETXMLTABLE;

#pragma pack()
#endif

#define PR_SET_REV              8

//ѹ��
#define PR_YB_GZJC_EN           0x00000001  /*���ϼ��*/
#define PR_YB_I1                0x00000002  /*����һ��*/
#define PR_YB_I2                0x00000004  /*��������*/
#define PR_YB_I3                0x00000008  /*��������*/
#define PR_YB_GFH               0x00000010  /*������*/
#define PR_YB_GY                0x00000020  /*����ѹ*/
#define PR_YB_DY                0x00000040  /*�͵�ѹ*/
#define PR_YB_I01               0x00000080  /*����һ��*/
#define PR_YB_I02               0x00000100  /*��������*/
#define PR_YB_I03               0x00000200  /*��������*/
#define PR_YB_DJQD              0x00000400  /*����ӿ��*/
#define PR_YB_PT                0x00000800  /*PT����*/
#define PR_YB_CH                0x00001000
#define PR_YB_TRIP              0x00002000
#define PR_YB_JS                0x00004000
#define PR_YB_GP	            0x00008000
#define PR_YB_YXTZ	            0x00010000  /* �ⲿң�Ŵ�����բ    */
#define PR_YB_DP	            0x00020000
#define PR_YB_MXQY              0x00040000  /*ĸ��Ƿѹ*/


#define PR_YB_MASK           0x0001FFFE

#define PR_YB_SLT               0x10000000  /*SLT����*/
#define PR_YB_SLTBAK            0x20000000  /*SLT����*/


//������1
#define PR_CFG1_FSX_CURV        0x00003      /*��ʱ������*/
#define PR_CFG1_FSX_START       0x00004      /*����������ʱ��*/
#define PR_CFG1_GFH_TRIP        0x00008      /*��������բ*/

#define PR_CFG1_U_PT            0x00010      /*PT���߱���*/
#define PR_CFG1_I_DY            0x00020      /*����һ�ε�ѹ����*/
#define PR_CFG1_I_DIR           0x00040
#define PR_CFG1_I_FX            0x00080      /*��������*/
#define PR_CFG1_DY_CYY          0x00100
#define PR_CFG1_PT_A            0x00200
#define PR_CFG1_PT_B            0x00400
#define PR_CFG1_PT_C            0x00800
#define PR_CFG1_I0_FX           0x01000      /*��������*/
#define PR_CFG1_I0_DIR          0x02000      /*��������Ԫ��*/
#define PR_CFG1_I0_U            0x04000      /*������ѹ����*/
#define PR_CFG1_CH_DDLBS        0x08000      /*����������غ�*/
#define PR_CFG1_CD_MODE         0x10000      /*�غ�բ���ģʽ*/
#define PR_CFG1_TRIP_MODE       0xE0000      /*������բģʽ3λ
                                              {������բ,
                                               ����ʧѹ��բ,
                                               ���ι���ʧѹ��բ,
                                               ���ع�������,
                                               ����բ}*/
#define PR_CFG1_FAULT_REPORT    0x300000     /*������Ϣ�ϱ�2λ
                                              {�����ϱ�,
                                               ��բ�ϱ�,
                                               һ���غ�բʧ���ϱ�,                                              
                                              }*/
#define PR_CFG1_TEST_VOL        0x400000      /*��·��ѹ��ѹ�ж�*/
#define PR_CFG1_BHQD_QUIT       0x800000     /*������������¼*/
#define PR_CFG1_FZD_BH          0x1000000     /*���ڶϵ�������*/
#define PR_CFG1_HW2_ZD          0x2000000    /*����ӿ��г���ƶ�*/
//#define PR_CFG1_PR_TRIP          0x4000000    /*��������*/
#define PR_CFG1_LED_FG          0x4000000    /*����ָʾ���Զ�����*/
#define PR_CFG1_I1_TRIP       0x8000000         /*����1�γ���*/
#define PR_CFG1_I2_TRIP       0x10000000         /*�������γ���*/
#define PR_CFG1_I0_TRIP       0x20000000	        /*�����������*/

#define PR_CFG1_I2_DY            0x40000000     /*�������ε�ѹ����*/
#define PR_CFG1_I2_DIR           0x80000000     /*�������ξ�����*/


//����2
#define PR_CFG2_IYX_DIR         0x00004
#define PR_CFG2_COEF            0x00008
#define PR_CFG2_RCD_GL          0x00010
#define PR_CFG2_RCD_SY          0x00020
#define PR_CFG2_RCD_U0          0x00040
#define PR_CFG2_RCD_I0          0x00080
#define PR_CFG2_XDL_YB          0x00100       /* С�����ӵ�Ͷ�� */
#define PR_CFG2_XDL_TRIP        0x00200
#define PR_CFG2_XDL_JR          0x00400
#define PR_CFG2_RCD_GLBH   0x00800
#define PR_CFG2_SWITCH_TRIP_F   0x01000
#define PR_CFG2_XDL_DLFX        0x02000
#define PR_CFG2_GY_TRIP	        0x04000         /* ��ѹ����*/
#define PR_CFG2_DY_TRIP	        0x08000 
#define PR_CFG2_RCD_I0TB        0x10000
#define PR_CFG2_GF_TRIP	        0x20000         /* ��Ƶ����*/
#define PR_CFG2_DF_TRIP         0x40000        /*��Ƶ����*/
#define PR_CFG2_I3_TRIP       	0x80000         /*�������γ���*/
#define PR_CFG2_MXQY_TRIP       0x100000         /*ĸ��Ƿѹ*/

#define PR_CFG1_MASK           0x383E0040
#define PR_CFG2_MASK           0x00000304

#define MAX_TABLE_FILE_SIZE     8192

//pub
#define PR_YB_SUDY              0x00000001  /*�����ѹ*/
#define PR_YB_SUSY              0x00000002  /*����ʧѹ*/
#define PR_YB_U0                0x00000004  /*��ѹ��բ*/
#define PR_YB_UDIFF             0x00000008  /*�ϻ���բ*/
#define PR_YB_SLWY              0x00000010  /*��ѹ��բ*/
#define PR_YB_U_HZ              0x0000000B    
#define PR_YB_I_DL              0x00000020
#define PR_YB_DF                0x00000040  /*��Ƶ*/
#define PR_YB_SHTQ              0x00000080
#define PR_YB_JD                   0x00000100 //�ӵع��ϴ���
#define PR_YB_YJCY              0x00000200  /*Ӳ����ѹģ�鴦��*/
#define PR_YB_BUS               0x00000400
#define PR_YB_LXFZ               0x00000800

#define PR_PUBYB_MASK            0x000003ff

#ifdef INCLUDE_FA
#define PR_YB_PUBU              (PR_YB_SLWY|PR_YB_U_HZ)
#endif

#define PR_CFG1_X_BS            0x00001      /*Xʱ�ޱ���*/
#define PR_CFG1_Y_BS            0x00002      /*Yʱ�ޱ���*/
#define PR_CFG1_DU_BS           0x00004      /*˫ѹ����*/
#define PR_CFG1_SY_BS           0x00008      /*˲ѹ����*/
#define PR_CFG1_UI_TYPE         0x00010      /*��ѹ������*/
#define PR_CFG1_U0_TRIP         0x00020      /*U0��բ*/
#define PR_CFG1_DFJL            0x00040      /*���С�����*/
#define PR_CFG1_DF_IBS          0x00080      /*��Ƶ��������*/
#define PR_CFG1_CHTQ_MODE       0x00300      /*4*/
#define PR_CFG1_SHTQ_MODE       0x01C00      /*6*/
#define PR_CFG1_WY_ANY          0x02000
#define PR_CFG1_TQ_LINE         0x04000      /*ͬһ���ߣ��ж�A,C����ͬ���ߣ����������ߵ�A��*/
#define PR_CFG1_JS_MODE         0x08000     /*����������ʽ*/
#define PR_CFG1_FTU_SD          0x010000    /*�׶�FTU */

#define PR_CFG1_ZSY_XJDL        0x020000   /*����Ӧ����·���ϴ���Ͷ��*/
#define PR_CFG1_ZSY_DXJD        0x040000     /*����Ӧ����ӵع��ϴ���Ͷ��*/

#define PR_CFG1_POWER_INDEX     0x080000     /*��Դ����*/
#define PR_CFG1_FZ_BS    0x100000     /* ��բ����*/
#define PR_CFG1_PUB_D  0x200000     /* ��ѹ��˫����*/
#define PR_CFG1_FTU_SD1          0x400000    /*�׶�FTU˫����*/
#define PR_CFG1_BUS_OPT          0x800000    /*ĸ�߹���*/

#define PR_CFG2_CONTROL_MODE    0x000007    /*0:�ֶ�,1:����,2:����,3:�ֽ�*/
#define PR_CFG2_FA_MODE    (0x000007<<3)    /*0 Ĭ��,1:��ѹʱ����,2:��ѹ������,3:����Ӧ*/
#if (TYPE_USER == USER_BEIJING)
#define PR_CFG2_KG_MODE  0x00000040      /*0 ���ɿ��أ� 1 ��ſ���*/
#endif
#if (TYPE_USER == USER_GUIYANG)&&(DEV_SP == DEV_SP_WDG)
#define PR_CFG2_AUTO_MODE  0x00000080      /*0 �Զ���Ͷ�룬 1 �Զ����˳�*/
#endif
#define PR_CFG2_DLXBS_MODE  0x00000100      
#define PR_CFG2_FA_I_TRIP  0x00000200      /*0 ������ 1 ��������բ*/
#define PR_CFG2_SUSY_TRIP  0x00000400      

#define PR_PUBCFG1_MASK          0x0016001B

#define PR_DELAY_0S      0
#define PR_DELAY_100MS   100
#define PR_DELAY_1S      1000
#define PR_DELAY_3S      3000
#define PR_DELAY_10S     10000

#define PR_OC_TRIP        0
#define PR_OC_NUI_TRIP    1
#define PR_OC_NUI2_TRIP   2
#define PR_AT_AUTO_TRIP   3
#define PR_NO_TRIP        4

#define PR_OC_REPORT         0
#define PR_OC_NI_REPORT      1
#define PR_OC_NI2_REPORT     2

#define PR_CH_MODE_NONE      0
#define PR_CH_MODE_WY        1
#define PR_CH_MODE_TQ        2
#define PR_CH_MODE_WYTQ      3

#define PR_SH_MODE_NONE      0
#define PR_SH_MODE_WY        1
#define PR_SH_MODE_TQ        2
#define PR_SH_MODE_ZTQ       3
#define PR_SH_MODE_WYTQ      4
#define PR_SH_MODE_WYZTQ     5

#define PR_MODE_SEG          0   //�ֶ�
#define PR_MODE_TIE          1   //����
#define PR_MODE_PR           2   //����
#define PR_MODE_FJ           3   //�ֽ�

#define PR_FA_I_BS           0
#define PR_FA_I_BSANDTRIP    1

#define PR_FA_NO        0
#define PR_FA_U          1
#define PR_FA_I           2
#define PR_FA_ZSY      3


#define PR_SET_DESCRIBE           0x01
#define PR_SET_PARA               0x02
#define PR_SET_START              0x04
#define PR_SET_MID                0x08
#define PR_SET_END                0x10
#define PR_SET_TYPE               0x03
#define PR_SET_ERR                0x80

#define PR_SET_FD                 0x01
#define PR_SET_PUB                0x02

#define prPubLowNum  2

typedef struct{
	float fUxxMin;
	float fUxx75;
	float fIMin;
	float fTLqd;
	float fTYy;
	float fTPt;
	float fTBhfg;
	float fTBsfg;
	float fTIRet;
	float fTI0Ret;
	float fTURet;
	float fTRet;
	float fTJsIn;
	float fFreqMin;
}VPrNbSet;

#pragma pack(1)
typedef struct{
		BYTE byIb;			//I�ι���ѹ��
		float fI1GLDZ;		//i�ι�����ֵ
		float fT1GL;			//i�ι���ʱ��
		BYTE byIPrTripYb;      //i�γ���ѹ��
		BYTE bTZFS;			//��բ��ʽ
		BYTE bFAReport;		//�����Ϸ�ʽ
		BYTE bCHZYB;		//�غ�բѹ��
		BYTE bCHNum;		//�غϴ���
		float fTCH1;			//һ���غ�ʱ��
		float fTCH2;			//�����غ�ʱ��
		float fTCH3;			//�����غ�ʱ��
		float fTCHBS;		//�غϱ���ʱ��
		BYTE byIIb;			//II�ι���ѹ��
		float fI2GLDZ;		//II�ι�����ֵ
		float fT2GL;			//II�ι���ʱ��
		BYTE byIIPrTripYb;      //II�γ���ѹ��
		BYTE byGLFx;
		BYTE byGLYx;
		float  AngleLow;
		float  AngleUpp; 

		BYTE     byXDLJD;
		BYTE     byXDLJD_Trip; 
		float    XDL_U0;
		float    XDL_U0_diff;	
		float    XDL_I0_diff;
		float    XDL_I_diff;
		float    XDL_Time;
		float    XDL_Coef;		
}  VPrConsVlaueSet;		

typedef struct{
		BYTE byIb;			//I�ι���ѹ��
		float fI1GLDZ;		//i�ι�����ֵ
		float fT1GL;			//i�ι���ʱ��
		BYTE byIPrTripYb;      //i�γ���ѹ��
		BYTE bTZFS;			//��բ��ʽ
		BYTE bFAReport;		//�����Ϸ�ʽ
		BYTE bCHZYB;		//�غ�բѹ��
		BYTE bCHNum;		//�غϴ���
		float fTCH1;			//һ���غ�ʱ��
		float fTCH2;			//�����غ�ʱ��
		float fTCH3;			//�����غ�ʱ��
		float fTCHBS;		//�غϱ���ʱ��
		BYTE byIIb;			//II�ι���ѹ��
		float fI2GLDZ;		//II�ι�����ֵ
		float fT2GL;			//II�ι���ʱ��
		BYTE byIIPrTripYb;      //II�γ���ѹ��
		BYTE bI0Yb;			//I0������ѹ��
		float  fI0Dz;			//I0��������ֵ
		float  fTI0;			//I0��ʱ��
		BYTE byI0PrTripYb;	//I0�γ���ѹ��
		BYTE byGLFx;
		BYTE byGLYx;
		float  AngleLow;
		float  AngleUpp; 
		BYTE     byXDLJD;
		BYTE     byXDLJD_Trip; 
		float    XDL_U0;
		float    XDL_U0_diff;	
		float    XDL_I0_diff;
		float    XDL_I_diff;
		float    XDL_Time;
		float    XDL_Coef;
}  VPrI0VlaueSet;	

typedef struct{
		BYTE BSFunyb;			//����ʧѹ��բѹ��
		BYTE BLFunyb;		//���翪�ع���ѹ��
		BYTE BSLLineNo;			//SL���ߺ�
		float fXsx;			//xʱ��
		float fYsx;		//yʱ��
		float fZsx;		//Zʱ��
		BYTE BXbsyb;      //x����ѹ��
		BYTE BYbsyb;      //Y����ѹ��
		BYTE BSybsyb;      //˲ѹ����ѹ��	
		BYTE LoseVolYb;
		float fTcytime;  //��ѹʱ��
		float cydz;   //��ѹ��ֵ
		BYTE U0Yb; //��ѹѹ��
		float  U0Dz;  //��ѹ��ֵ
		BYTE FZBS;
		float  fZbstime;

		BYTE     byXDLJD;
		BYTE     byXDLJD_Trip; 
		float    XDL_U0;
		float    XDL_U0_diff;	
		float    XDL_I0_diff;
		float    XDL_I_diff;
		float    XDL_Time;
		float    XDL_Coef;
}  VPrdyxSet;		//��ѹ�ͱ�������

typedef struct{
		BYTE BDljsyb;			//��������ѹ��
		BYTE BSLLineNo;			//SL���ߺ�
		BYTE BDljsNUM;			//������������
		float fTdljsReset;			//����������λʱ��
		float fTdljsTotal;		//���������ۼ�ʱ��
		BYTE OCIYb;			//����һ��ѹ��
		float	 OCIDz;			//����һ�ζ�ֵ
		float	 OCITime;		//����һ��ʱ��
		BYTE bTZFS;			//��բ��ʽ
		BYTE bFAReport;		//�����Ϸ�ʽ
		BYTE bCHZYB;		//�غ�բѹ��
		BYTE bCHNum;		//�غϴ���
		float fTCH1;			//һ���غ�ʱ��
		float fTCH2;			//�����غ�ʱ��
		float fTCH3;			//�����غ�ʱ��
		float fTCHBS;		//�غϱ���ʱ��
		BYTE byIPrYb;    // һ�γ���ѹ��
             
  }VPrdljsxSet;		//���������ͱ�������

typedef struct{
		BYTE BSFunyb;			//����ʧѹ��բѹ��
		BYTE BLFunyb;		//���翪�ع���ѹ��
		BYTE BSLLineNo;			//SL���ߺ�
		float fXsx;			//xʱ��
		float fYsx;		//yʱ��
		float fZsx;		//zʱ��
		float fSsx;          //sʱ��
		float fCsx;         //cʱ��
		BYTE BXbsyb;      //x����ѹ��
		BYTE BYbsyb;      //Y����ѹ��
		BYTE BSybsyb;      //˲ѹ����ѹ��	
         	BYTE LoseVolYb;	//��ѹѹ��
		BYTE byIb;			//I�ι���ѹ��
		float fI1GLDZ;		//i�ι�����ֵ
		float fT1GL;			//i�ι���ʱ��
		BYTE byIPrTripYb;      //i�γ���ѹ��
		BYTE byxjdl;			//����Ӧ����·
		float fTcytime;  //��ѹʱ��
		float cydz;   //��ѹ��ֵ
		BYTE U0Yb; //��ѹѹ��
		float  U0Dz;  //��ѹ��ֵ
		BYTE FZBS;
		float  fZbstime;
		BYTE     byXDLJD;
		BYTE     byXDLJD_Trip; 
		float    XDL_U0;
		float    XDL_U0_diff;	
		float    XDL_I0_diff;
		float    XDL_I_diff;
		float    XDL_Time;
		float    XDL_Coef;
		BYTE bYbZsyjd;
		BYTE bYbDu;
		BYTE bYbU0Trip;
		float fTU0;
}  VPrzsyxSet;		//����Ӧ��������
typedef struct{
		BYTE BSFunyb;			//����ʧѹ��բѹ��
		BYTE BLFunyb;		//���翪�ع���ѹ��
		BYTE BSLLineNo; 		//SL���ߺ�
		float fXsx; 		//xʱ��
		float fYsx; 	//yʱ��
		float fZsx; 	//Zʱ��
		BYTE BXbsyb;	  //x����ѹ��
		BYTE BYbsyb;	  //Y����ѹ��
		BYTE BSybsyb;	   //˲ѹ����ѹ�� 
		BYTE LoseVolYb; 
		BYTE BDyDlYb;	   //��ѹ����ѹ��
		BYTE byIb;			//I�ι���ѹ��
		float fI1GLDZ;		//i�ι�����ֵ
		float fT1GL;			//i�ι���ʱ��
		BYTE byIPrTripYb;	   //i�γ���ѹ��
		float fTcytime;  //��ѹʱ��
		float cydz;   //��ѹ��ֵ
		BYTE U0Yb; //��ѹѹ��
		float  U0Dz;  //��ѹ��ֵ
		BYTE FZBS;
		float  fZbstime;

		BYTE	 byXDLJD;
		BYTE     byXDLJD_Trip; 
		float	 XDL_U0;
		float	 XDL_U0_diff;	
		float	 XDL_I0_diff;
		float	 XDL_I_diff;
		float	 XDL_Time;
		float	 XDL_Coef;
		BYTE bYbDu;
		BYTE bYbU0Trip;
		float fTU0;
}  VPrdydlSet;	
#pragma pack()


typedef struct{
    DWORD dYb;
	DWORD dKG1;
	DWORD dKG2;
	
	DWORD dI[4][3];
    DWORD dIRet[3][3];
	DWORD dI1_old[3];
	DWORD dIM[3];                          /*��������*/
	DWORD dT[3];
	DWORD dTIRet;
	DWORD dTDjqd;                       /*�������ʱ��*/
	BOOL  bIDYBS;
	BOOL  bIDir;
	BOOL  bIDirMX;
	BOOL  bIYxDir;                   /*ң����ʾ�Ƿ񺬷���*/
	BYTE  bITrip;
	BYTE  bIReport;

    BYTE  bFsx;
	DWORD dIGfh[4];
	DWORD dIGfhRet[4];
    DWORD dTGfh;				      /*������ʱ�䣬��ʱ�ޣ���ʱ��*/
	BOOL  bGfhGJ;
	BOOL  bGfhSX;

	short coef;

	DWORD dIKG[3];
  
 	DWORD dI0[4];                    /*��������*/
    DWORD dI0Ret[3];
	DWORD dU0;
	DWORD dU0Ret;
	DWORD dT0[3];
	DWORD dTI0Ret;
	BOOL  bI0GJ;
	BOOL bI1GJ;
	BOOL bI2GJ;
	BOOL bI3GJ;
	BOOL bI1Trip;
	BOOL bI2Trip;
	BOOL bI3Trip;
	BOOL bI0Trip;
	BOOL bXDLYB;
	BOOL bXDLGJ;
	BOOL bXDLTrip;
	BOOL bXdlFx; // ��С�����ӵ�ʱ��һ��ѹ��������߷���,�����ֳ�����

	BOOL bDdlBsch;  //����������غ�

	BOOL bRcdGl; //���ڹ�����ֵ¼��
	BOOL bRcdGlbh;  //������ʱ¼��
	BOOL bRcdSy;
	BOOL bRcdU0;
	BOOL bRcdI0;
	BOOL bRcdI0TB;
	BOOL bSwitchTrip;  //������բ�쳣

	BOOL bJrU0;
	BOOL bJrU;
	DWORD dTDXJD;
	DWORD wXDLCoef;
	
	BOOL  bI0Dir;
	BOOL  bI0DirMX;
	BOOL  bI0U;
	BYTE  bI0Mode;

	DWORD dTJS;
	DWORD dTJSIN;
	DWORD dTJSFG;
	
	long lPang1;
    long lPang2;
	long lNang1;
	long lNang2;

	DWORD dUDy[3];                       /*�͵�ѹ*/
	DWORD dUDyRet[3];
	DWORD dUSlip[3];
	DWORD dTDy;
	DWORD dUGy[3];
	DWORD dUGyRet[3];
	DWORD dTGy;
	DWORD dTURet;
	DWORD dUYy[3];
	DWORD dUWy[3];
	BOOL  bPTBS;
	BOOL  bCYY;
	BOOL  bPTU[3];
	DWORD dCHBSDL;                // �غϱ�������
	DWORD bCHNum;
	DWORD dTCH1;
	DWORD dTCH2;
	DWORD dTCH3;
	DWORD dTCHBS;
	DWORD dTCHBAY;
    DWORD wYxNum; //�ⲿң�Ŷ�Ӧ����բ��ң�Ŵ�0��ʼ��

	DWORD dTJWY;      //����ѹʱ��
	DWORD dGFreq;     //��Ƶ��ֵ
	DWORD dTGFreq;   //��Ƶʱ��
	DWORD dDFreq;     //��Ƶ��ֵ
	DWORD dTDFreq;   //��Ƶʱ��
	BOOL  bGpGJ;     //��Ƶ�澯/��բ
	BOOL  bDpGJ;     //��Ƶ�澯/��բ
	DWORD dI0TJS;    //��������ʱ��
	BOOL  bI2DYBS;   //�������ε�ѹ
	BOOL  bI2Dir;    //�������ξ�����
	BOOL  bMXQYGJ;   //ĸ��Ƿѹ�澯/����
	DWORD dUUMX[3];   //ĸ�߹���ѹ��ֵ
	DWORD dUUMXRet[3];
	DWORD dTUMX;      //ĸ�߹���ѹʱ��
	DWORD dUDMX[3];   //ĸ�ߵ͵�ѹ��ֵ
	DWORD dUDMXRet[3];
	DWORD dTDMX;      //ĸ�ߵ͵�ѹʱ��

	BOOL  bCd;
	BOOL  bKgMode;
	BOOL  bLedZdFg;
	BOOL  bGyGJ; //��ѹ�澯
	BOOL  bDyGJ; 


	DWORD dTWY;

	BOOL  bQdQuit;
	BOOL  bfZdbh;
	BOOL  bHw2Zd;
	BOOL  bTestVol;

	DWORD dU0_8V;                    /*�ڲ���ֵ*/ 
	DWORD dU_16V[3];
	DWORD dI_1A[3];
	DWORD dUxxMin[3];
	DWORD dUMin[4];
	DWORD dUxx75[3];
	DWORD dU75[3];
	DWORD dIMin[4];
	DWORD dIUnK;
	DWORD dII0nK;
	DWORD dIInK;
	DWORD dTLqd;
	DWORD dTYy;
	DWORD dTPt;
	DWORD dTRet;
	DWORD dLedTret;
	DWORD dTBhfg;
	DWORD dTBsfg;
	BOOL  bSetChg;
}VPrRunSet;	

typedef struct{
	DWORD dYb;
	DWORD dKG1;
	DWORD dKG2;

	BYTE  bySLLine;
	DWORD dSLU[3];
	DWORD dSLUMin[3];
	DWORD dTX;
	DWORD dTY;
	DWORD dTFZ1;
	DWORD dTFZ2;
	DWORD dTSYCY;
	DWORD dTSY1;
	DWORD dTSY2;
	DWORD dTU0;
	DWORD dTWY;
	DWORD dTJD;//����
	DWORD dTJDrevs;//����ʱ��
	long dUAng;
	DWORD dRecNum;
	
	DWORD dDYLJ;      // ��ѹ�ۼƴ���
	DWORD dTDYLJ;	// ��ѹ�ۼ�ʱ��
	
	DWORD dIBus;
	
	DWORD dLXFZ;      // add by wdj ������բ����
	DWORD dTLXFZ;	  // ������բʱ��

	DWORD dAi_S_No; //��Դ���ѹͨ��
	DWORD dAi_F_No; //���ɲ��ѹͨ��
	
	DWORD dU0;
	DWORD dUDiff;
	DWORD dUCY[3];
	DWORD dICnt;
	DWORD dTIDl1;
	DWORD dTIDl2;
	BOOL  bXBS;
	BOOL  bYBS;
	BOOL  bDUBS;
	BOOL  bSYBS;
	BOOL  bIProtect;
	BOOL  bU0Trip;
	BOOL  bBusTrip;
	BYTE  byWorkMode;	 //ֻ��FTU���й���ģʽ
	BYTE bFaWorkMode;   // ��ѹʱ����ģʽ
#if (TYPE_USER == USER_BEIJING)
	BOOL bKgMode; // ���׿�������
#endif

	BOOL bAuto; // ����ʽ�͵���ѡ��
	BOOL bDlxbsMode;
	BOOL bFaTripMode;
	BOOL bSUSYMode;

	BYTE  byPowerIndex;

#ifdef INCLUDE_PR_PRO
	DWORD dFreq;
	DWORD dUFreq[3];
	DWORD dIFreq[3];
	DWORD dTDf;
	DWORD dFSlip;
	DWORD dFreqMin;
	
	BOOL  bDfJl;
	BOOL  bDfIBs;

    BYTE  byTqLine;
	DWORD dTqU75[3];
	DWORD dTqUMin[3];
	DWORD dTqUDiff;
	DWORD dTqFDiff;
	DWORD dTqFsDiff;
	long  dTqDqAngle;
	DWORD dTqTAng;
	DWORD dTTqTwj;
	DWORD dTTqYk;
	BYTE  bTqMode;
	BYTE  bShTqMode;
	BOOL  bTqWyAny;
	BOOL  bTqLine;
#endif
	DWORD dTC;
	DWORD dTS;
	DWORD dTXXCH;

    BOOL  bJsMode;
	BOOL  bFTUSD;   /*�׶�FTU*/
	BOOL  bXJDL;    /*����·*/
	BOOL  bDXJD;   /*����ӵ�*/
	BOOL  bFZBS;  /* ��բ���� */
	BOOL  bSetChg;
}VPrSetPublic;

typedef struct{
	DWORD dYb;

	WORD wCfg[4];

	float fI[3];
	float fT[3];

	float fI0[3];
	float fT0[3];

    float fGfh;                         /*������*/
	float fTGfh;                        

	float fTTrip;                       /*ʧѹ��բ��ʱ*/

    float fTRc;                         /*�غ�բʱ��*/
    float fTRcLock;                     /*ȷ���غϱ���ʱ��*/

	float fTReset;

	
    float fGy;                          /*��ѹ*/
	float fTGy;  

}VPrSet;
int prSetInit(void);
int prRunInfoInit(void);
VPrRunSet * prGetRunSet(int fd);
void prFdSetNoSwitch(int fd);
#ifdef INCLUDE_FA_S

 int ReadConsValueSet(int fd,int flag,BYTE*pBuf);
int ReadVoltageSet(int fd, BYTE*pBuf);
int ReadCurrentSet(int fd, BYTE*pBuf);
int FaSimGetPara(int fd, int flag,BYTE *pBuf);
 int ReadZsyVoltageSet(int fd, BYTE*pBuf);
 int ReadDyDlVoltageSet(int fd, BYTE*pBuf);
 int ReadI0ValueSet(int fd, BYTE*pBuf);

 int WriteConsValueSet(int fd, int flag,BYTE*pSet);
 int WriteVoltageSet(int fd, BYTE*pSet);
 int WriteZsyVoltageSet(int fd, BYTE*pSet);
 int WriteCurrentSet(int fd, BYTE*pSet);
 int WriteDYDLVoltageSet(int fd, BYTE*pSet);
int WriteI0Set(int fd, BYTE*pSet);
 int FaSimWriteSet(int fd, int flag, BYTE *pSet);
 void ClearVoltageYB(int fd);
void ClearYBSet(int fd);


#endif
#ifdef INCLUDE_EXT_MMI
int WriteMMIPrSet(int fd, BYTE *pSet);
#if (TYPE_USER == USER_BEIJING)
int WriteMMIU0Set(int fd,float u0set);
#endif	
int WriteMMIPrSet_1(int fd, BYTE *pSet);
#endif
#ifdef _TEST_VER_
#if(DEV_SP == DEV_SP_DTU)
int BOMAPrset(int fd,WORD on);
#endif
#endif

#ifdef _YIERCI_RH_
int SetFaTypePrset(BYTE on);
#endif

int ReadSetNum(int fd);
int ReadSet(int fd, BYTE *pSet);
int WriteSet(int fd, BYTE *pSet);
int ReadSetName(int fd, BYTE *pName);
int ReadSetLen(int fd, BYTE *pBuf, VPrSetHead* pHead);
int WriteSetLen(int fd, BYTE *pBuf, VPrSetHead* pHead);


void WritePrFile(int no);
int WritePrParaFile(void);
BOOL ReadGLLB(WORD fd);
void WriteGLLB(WORD fd, BOOL gllb);
BOOL ReadWYLB(WORD fd)	;
void WriteWYLB(WORD fd, BOOL wllb);
BOOL ReadI0LB(WORD fd);	
void WriteI0LB(WORD fd, BOOL i0lb);
BOOL ReadU0LB(WORD fd);
void WriteU0LB(WORD fd, BOOL u0lb);
int ReadPrPara(WORD parano,char *pbuf,WORD type);
int WritePrParaYZ(WORD parano,char *pbuf);
int WritePrPara(WORD parano,char *pbuf,WORD type);
#ifdef INCLUDE_FA
int prSwitchFaMod(int pr, int fd, int wdg_flag);
#endif
long ReadIVIT(int no,int fd);
#endif

