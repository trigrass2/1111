#ifndef _PRSET_H
#define _PRSET_H

#include "syscfg.h"

#define PROT_SET_1             0x81                    /*一次值*/
#define PROT_SET_2             0x82
#define PROT_TYPE              PROT_SET_2
//
//-----------------------------定值----------------------
//
#define PR_SET_SIZE             1024
#define PR_SET_NAME             64
//定值序号
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
  SET_YXTZ,//外部遥信对应的跳闸

  SET_FGP, //过频定值
  SET_TGP, //过频时间
  SET_FDP, //低频定值
  SET_TDP, //低频时间
  SET_IHJS,  //后加速定值
  SET_I0HJS,  //后加速零序定值
  SET_I0TJS,
  SET_UUMX,  //母线过电压定值
  SET_TUMX,	 //母线过电压时间
  SET_UDMX,  //母线欠电压定值
  SET_TDMX,  //母线欠电压时间
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
  SET_PUB_SLFD_ANOTHER,  //另一个回线
  SET_PUB_IBUS,
  
  SET_PUB_LXFZ,//连续分闸      add by wdj
  SET_PUB_TLXFZ,

  SET_PUB_AI_S,
  SET_PUB_AI_F,
  
  SET_PUB_NUM
};

//定值类型
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

//定值操作，隐藏，只读，可写
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
//定值
//定值属性
#define VALUEATTRIB_SIG      (1<<7)     //bit7=signal
#define VALUEATTRIB_DOT3     (1<<2)
#define VALUEATTRIB_DOT2     (1<<1)     //bit 2-0=小数位数
#define VALUEATTRIB_DOT1     (1<<0)     //

#define SETATTRIB_TYPEMASK    0x0f        
#define SETATTRIB_OPTMASK	  0xf0       //bit 3-0=定值类型

//对于每个整定值的结构
//其中byAttrib定义如下
//<7..4>:=操作类型
//<3..0>:=定值类型
//<7>   :=符号位
//<6..2>:=保留
//<1..0>:=小数位数(对二进制无效)
//当定值位十进制时,byValLo和byValHi以压缩BCD码表示
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
	  TSETVAL tMin;//最小值
	  TSETVAL tMax;//最大值
	  TSETVAL tDef;//缺省值
} TSETTABLE;

#pragma pack()

//压板序号
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
	 YB_GP, //过频
	 YB_TZ = 16, //外部遥信触发跳闸压板
	 YB_DP, //低频
	 YB_MXQY,//母线欠压保护
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

//控制字序号
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
	  BYTE relateno[10];   //参数中与某压板相关
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

//压板
#define PR_YB_GZJC_EN           0x00000001  /*故障检测*/
#define PR_YB_I1                0x00000002  /*过流一段*/
#define PR_YB_I2                0x00000004  /*过流二段*/
#define PR_YB_I3                0x00000008  /*过流三段*/
#define PR_YB_GFH               0x00000010  /*过负荷*/
#define PR_YB_GY                0x00000020  /*过电压*/
#define PR_YB_DY                0x00000040  /*低电压*/
#define PR_YB_I01               0x00000080  /*零流一段*/
#define PR_YB_I02               0x00000100  /*零流二段*/
#define PR_YB_I03               0x00000200  /*零流三段*/
#define PR_YB_DJQD              0x00000400  /*励磁涌流*/
#define PR_YB_PT                0x00000800  /*PT断线*/
#define PR_YB_CH                0x00001000
#define PR_YB_TRIP              0x00002000
#define PR_YB_JS                0x00004000
#define PR_YB_GP	            0x00008000
#define PR_YB_YXTZ	            0x00010000  /* 外部遥信触发跳闸    */
#define PR_YB_DP	            0x00020000
#define PR_YB_MXQY              0x00040000  /*母线欠压*/


#define PR_YB_MASK           0x0001FFFE

#define PR_YB_SLT               0x10000000  /*SLT闭锁*/
#define PR_YB_SLTBAK            0x20000000  /*SLT后备跳*/


//控制字1
#define PR_CFG1_FSX_CURV        0x00003      /*反时限曲线*/
#define PR_CFG1_FSX_START       0x00004      /*启动闭锁反时限*/
#define PR_CFG1_GFH_TRIP        0x00008      /*过负荷跳闸*/

#define PR_CFG1_U_PT            0x00010      /*PT断线闭锁*/
#define PR_CFG1_I_DY            0x00020      /*过流一段低压闭锁*/
#define PR_CFG1_I_DIR           0x00040
#define PR_CFG1_I_FX            0x00080      /*过流方向*/
#define PR_CFG1_DY_CYY          0x00100
#define PR_CFG1_PT_A            0x00200
#define PR_CFG1_PT_B            0x00400
#define PR_CFG1_PT_C            0x00800
#define PR_CFG1_I0_FX           0x01000      /*过流方向*/
#define PR_CFG1_I0_DIR          0x02000      /*零流方向元件*/
#define PR_CFG1_I0_U            0x04000      /*零流电压启动*/
#define PR_CFG1_CH_DDLBS        0x08000      /*大电流闭锁重合*/
#define PR_CFG1_CD_MODE         0x10000      /*重合闸充电模式*/
#define PR_CFG1_TRIP_MODE       0xE0000      /*过流跳闸模式3位
                                              {过流跳闸,
                                               过流失压跳闸,
                                               二次过流失压跳闸,
                                               开关过流自跳,
                                               不跳闸}*/
#define PR_CFG1_FAULT_REPORT    0x300000     /*故障信息上报2位
                                              {过流上报,
                                               跳闸上报,
                                               一次重合闸失败上报,                                              
                                              }*/
#define PR_CFG1_TEST_VOL        0x400000      /*线路有压无压判断*/
#define PR_CFG1_BHQD_QUIT       0x800000     /*保护启动不记录*/
#define PR_CFG1_FZD_BH          0x1000000     /*非遮断电流保护*/
#define PR_CFG1_HW2_ZD          0x2000000    /*励磁涌流谐波制动*/
//#define PR_CFG1_PR_TRIP          0x4000000    /*保护出口*/
#define PR_CFG1_LED_FG          0x4000000    /*故障指示灯自动复归*/
#define PR_CFG1_I1_TRIP       0x8000000         /*过流1段出口*/
#define PR_CFG1_I2_TRIP       0x10000000         /*过流二段出口*/
#define PR_CFG1_I0_TRIP       0x20000000	        /*零序过流出口*/

#define PR_CFG1_I2_DY            0x40000000     /*过流二段低压闭锁*/
#define PR_CFG1_I2_DIR           0x80000000     /*过流二段经方向*/


//控制2
#define PR_CFG2_IYX_DIR         0x00004
#define PR_CFG2_COEF            0x00008
#define PR_CFG2_RCD_GL          0x00010
#define PR_CFG2_RCD_SY          0x00020
#define PR_CFG2_RCD_U0          0x00040
#define PR_CFG2_RCD_I0          0x00080
#define PR_CFG2_XDL_YB          0x00100       /* 小电流接地投退 */
#define PR_CFG2_XDL_TRIP        0x00200
#define PR_CFG2_XDL_JR          0x00400
#define PR_CFG2_RCD_GLBH   0x00800
#define PR_CFG2_SWITCH_TRIP_F   0x01000
#define PR_CFG2_XDL_DLFX        0x02000
#define PR_CFG2_GY_TRIP	        0x04000         /* 过压出口*/
#define PR_CFG2_DY_TRIP	        0x08000 
#define PR_CFG2_RCD_I0TB        0x10000
#define PR_CFG2_GF_TRIP	        0x20000         /* 过频出口*/
#define PR_CFG2_DF_TRIP         0x40000        /*低频出口*/
#define PR_CFG2_I3_TRIP       	0x80000         /*过流三段出口*/
#define PR_CFG2_MXQY_TRIP       0x100000         /*母线欠压*/

#define PR_CFG1_MASK           0x383E0040
#define PR_CFG2_MASK           0x00000304

#define MAX_TABLE_FILE_SIZE     8192

//pub
#define PR_YB_SUDY              0x00000001  /*单侧得压*/
#define PR_YB_SUSY              0x00000002  /*单测失压*/
#define PR_YB_U0                0x00000004  /*零压分闸*/
#define PR_YB_UDIFF             0x00000008  /*合环合闸*/
#define PR_YB_SLWY              0x00000010  /*无压分闸*/
#define PR_YB_U_HZ              0x0000000B    
#define PR_YB_I_DL              0x00000020
#define PR_YB_DF                0x00000040  /*低频*/
#define PR_YB_SHTQ              0x00000080
#define PR_YB_JD                   0x00000100 //接地故障处理
#define PR_YB_YJCY              0x00000200  /*硬件残压模块处理*/
#define PR_YB_BUS               0x00000400
#define PR_YB_LXFZ               0x00000800

#define PR_PUBYB_MASK            0x000003ff

#ifdef INCLUDE_FA
#define PR_YB_PUBU              (PR_YB_SLWY|PR_YB_U_HZ)
#endif

#define PR_CFG1_X_BS            0x00001      /*X时限闭锁*/
#define PR_CFG1_Y_BS            0x00002      /*Y时限闭锁*/
#define PR_CFG1_DU_BS           0x00004      /*双压闭锁*/
#define PR_CFG1_SY_BS           0x00008      /*瞬压闭锁*/
#define PR_CFG1_UI_TYPE         0x00010      /*电压电流型*/
#define PR_CFG1_U0_TRIP         0x00020      /*U0跳闸*/
#define PR_CFG1_DFJL            0x00040      /*解列、减载*/
#define PR_CFG1_DF_IBS          0x00080      /*低频电流闭锁*/
#define PR_CFG1_CHTQ_MODE       0x00300      /*4*/
#define PR_CFG1_SHTQ_MODE       0x01C00      /*6*/
#define PR_CFG1_WY_ANY          0x02000
#define PR_CFG1_TQ_LINE         0x04000      /*同一回线，判断A,C，不同回线，则是两回线的A相*/
#define PR_CFG1_JS_MODE         0x08000     /*闭锁解锁方式*/
#define PR_CFG1_FTU_SD          0x010000    /*首端FTU */

#define PR_CFG1_ZSY_XJDL        0x020000   /*自适应相间短路故障处理投入*/
#define PR_CFG1_ZSY_DXJD        0x040000     /*自适应单相接地故障处理投入*/

#define PR_CFG1_POWER_INDEX     0x080000     /*电源属性*/
#define PR_CFG1_FZ_BS    0x100000     /* 分闸闭锁*/
#define PR_CFG1_PUB_D  0x200000     /* 电压型双回线*/
#define PR_CFG1_FTU_SD1          0x400000    /*首端FTU双回线*/
#define PR_CFG1_BUS_OPT          0x800000    /*母线故障*/

#define PR_CFG2_CONTROL_MODE    0x000007    /*0:分段,1:联络,2:保护,3:分界*/
#define PR_CFG2_FA_MODE    (0x000007<<3)    /*0 默认,1:电压时间型,2:电压电流型,3:自适应*/
#if (TYPE_USER == USER_BEIJING)
#define PR_CFG2_KG_MODE  0x00000040      /*0 弹簧开关， 1 电磁开关*/
#endif
#if (TYPE_USER == USER_GUIYANG)&&(DEV_SP == DEV_SP_WDG)
#define PR_CFG2_AUTO_MODE  0x00000080      /*0 自动化投入， 1 自动化退出*/
#endif
#define PR_CFG2_DLXBS_MODE  0x00000100      
#define PR_CFG2_FA_I_TRIP  0x00000200      /*0 闭锁， 1 闭锁且跳闸*/
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

#define PR_MODE_SEG          0   //分段
#define PR_MODE_TIE          1   //联络
#define PR_MODE_PR           2   //保护
#define PR_MODE_FJ           3   //分界

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
		BYTE byIb;			//I段过流压板
		float fI1GLDZ;		//i段过流定值
		float fT1GL;			//i段过流时间
		BYTE byIPrTripYb;      //i段出口压板
		BYTE bTZFS;			//跳闸方式
		BYTE bFAReport;		//报故障方式
		BYTE bCHZYB;		//重合闸压板
		BYTE bCHNum;		//重合次数
		float fTCH1;			//一次重合时间
		float fTCH2;			//二次重合时间
		float fTCH3;			//三次重合时间
		float fTCHBS;		//重合闭锁时间
		BYTE byIIb;			//II段过流压板
		float fI2GLDZ;		//II段过流定值
		float fT2GL;			//II段过流时间
		BYTE byIIPrTripYb;      //II段出口压板
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
		BYTE byIb;			//I段过流压板
		float fI1GLDZ;		//i段过流定值
		float fT1GL;			//i段过流时间
		BYTE byIPrTripYb;      //i段出口压板
		BYTE bTZFS;			//跳闸方式
		BYTE bFAReport;		//报故障方式
		BYTE bCHZYB;		//重合闸压板
		BYTE bCHNum;		//重合次数
		float fTCH1;			//一次重合时间
		float fTCH2;			//二次重合时间
		float fTCH3;			//三次重合时间
		float fTCHBS;		//重合闭锁时间
		BYTE byIIb;			//II段过流压板
		float fI2GLDZ;		//II段过流定值
		float fT2GL;			//II段过流时间
		BYTE byIIPrTripYb;      //II段出口压板
		BYTE bI0Yb;			//I0段零流压板
		float  fI0Dz;			//I0段零流定值
		float  fTI0;			//I0段时间
		BYTE byI0PrTripYb;	//I0段出口压板
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
		BYTE BSFunyb;			//单侧失压跳闸压板
		BYTE BLFunyb;		//联络开关功能压板
		BYTE BSLLineNo;			//SL回线号
		float fXsx;			//x时限
		float fYsx;		//y时限
		float fZsx;		//Z时限
		BYTE BXbsyb;      //x闭锁压板
		BYTE BYbsyb;      //Y闭锁压板
		BYTE BSybsyb;      //瞬压闭锁压板	
		BYTE LoseVolYb;
		float fTcytime;  //残压时间
		float cydz;   //残压定值
		BYTE U0Yb; //零压压板
		float  U0Dz;  //零压定值
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
}  VPrdyxSet;		//电压型保护参数

typedef struct{
		BYTE BDljsyb;			//电流计数压板
		BYTE BSLLineNo;			//SL回线号
		BYTE BDljsNUM;			//电流计数次数
		float fTdljsReset;			//电流计数复位时间
		float fTdljsTotal;		//电流计数累计时间
		BYTE OCIYb;			//过流一段压板
		float	 OCIDz;			//过流一段定值
		float	 OCITime;		//过流一段时间
		BYTE bTZFS;			//跳闸方式
		BYTE bFAReport;		//报故障方式
		BYTE bCHZYB;		//重合闸压板
		BYTE bCHNum;		//重合次数
		float fTCH1;			//一次重合时间
		float fTCH2;			//二次重合时间
		float fTCH3;			//三次重合时间
		float fTCHBS;		//重合闭锁时间
		BYTE byIPrYb;    // 一段出口压板
             
  }VPrdljsxSet;		//电流计数型保护参数

typedef struct{
		BYTE BSFunyb;			//单侧失压跳闸压板
		BYTE BLFunyb;		//联络开关功能压板
		BYTE BSLLineNo;			//SL回线号
		float fXsx;			//x时限
		float fYsx;		//y时限
		float fZsx;		//z时限
		float fSsx;          //s时限
		float fCsx;         //c时限
		BYTE BXbsyb;      //x闭锁压板
		BYTE BYbsyb;      //Y闭锁压板
		BYTE BSybsyb;      //瞬压闭锁压板	
         	BYTE LoseVolYb;	//无压压板
		BYTE byIb;			//I段过流压板
		float fI1GLDZ;		//i段过流定值
		float fT1GL;			//i段过流时间
		BYTE byIPrTripYb;      //i段出口压板
		BYTE byxjdl;			//自适应相间短路
		float fTcytime;  //残压时间
		float cydz;   //残压定值
		BYTE U0Yb; //零压压板
		float  U0Dz;  //零压定值
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
}  VPrzsyxSet;		//自适应保护参数
typedef struct{
		BYTE BSFunyb;			//单侧失压跳闸压板
		BYTE BLFunyb;		//联络开关功能压板
		BYTE BSLLineNo; 		//SL回线号
		float fXsx; 		//x时限
		float fYsx; 	//y时限
		float fZsx; 	//Z时限
		BYTE BXbsyb;	  //x闭锁压板
		BYTE BYbsyb;	  //Y闭锁压板
		BYTE BSybsyb;	   //瞬压闭锁压板 
		BYTE LoseVolYb; 
		BYTE BDyDlYb;	   //电压电流压板
		BYTE byIb;			//I段过流压板
		float fI1GLDZ;		//i段过流定值
		float fT1GL;			//i段过流时间
		BYTE byIPrTripYb;	   //i段出口压板
		float fTcytime;  //残压时间
		float cydz;   //残压定值
		BYTE U0Yb; //零压压板
		float  U0Dz;  //零压定值
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
	DWORD dIM[3];                          /*启动电流*/
	DWORD dT[3];
	DWORD dTIRet;
	DWORD dTDjqd;                       /*电机启动时间*/
	BOOL  bIDYBS;
	BOOL  bIDir;
	BOOL  bIDirMX;
	BOOL  bIYxDir;                   /*遥信显示是否含方向*/
	BYTE  bITrip;
	BYTE  bIReport;

    BYTE  bFsx;
	DWORD dIGfh[4];
	DWORD dIGfhRet[4];
    DWORD dTGfh;				      /*过负荷时间，定时限，反时限*/
	BOOL  bGfhGJ;
	BOOL  bGfhSX;

	short coef;

	DWORD dIKG[3];
  
 	DWORD dI0[4];                    /*零流三段*/
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
	BOOL bXdlFx; // 测小电流接地时万一电压或电流接线反了,方便现场测试

	BOOL bDdlBsch;  //大电流闭锁重合

	BOOL bRcdGl; //大于过流定值录波
	BOOL bRcdGlbh;  //报保护时录波
	BOOL bRcdSy;
	BOOL bRcdU0;
	BOOL bRcdI0;
	BOOL bRcdI0TB;
	BOOL bSwitchTrip;  //开关跳闸异常

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

	DWORD dUDy[3];                       /*低电压*/
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
	DWORD dCHBSDL;                // 重合闭锁电流
	DWORD bCHNum;
	DWORD dTCH1;
	DWORD dTCH2;
	DWORD dTCH3;
	DWORD dTCHBS;
	DWORD dTCHBAY;
    DWORD wYxNum; //外部遥信对应的跳闸（遥信从0开始）

	DWORD dTJWY;      //检无压时间
	DWORD dGFreq;     //过频定值
	DWORD dTGFreq;   //过频时间
	DWORD dDFreq;     //低频定值
	DWORD dTDFreq;   //低频时间
	BOOL  bGpGJ;     //过频告警/跳闸
	BOOL  bDpGJ;     //低频告警/跳闸
	DWORD dI0TJS;    //零序后加速时间
	BOOL  bI2DYBS;   //过流二段低压
	BOOL  bI2Dir;    //过流二段经方向
	BOOL  bMXQYGJ;   //母线欠压告警/出口
	DWORD dUUMX[3];   //母线过电压定值
	DWORD dUUMXRet[3];
	DWORD dTUMX;      //母线过电压时间
	DWORD dUDMX[3];   //母线低电压定值
	DWORD dUDMXRet[3];
	DWORD dTDMX;      //母线低电压时间

	BOOL  bCd;
	BOOL  bKgMode;
	BOOL  bLedZdFg;
	BOOL  bGyGJ; //过压告警
	BOOL  bDyGJ; 


	DWORD dTWY;

	BOOL  bQdQuit;
	BOOL  bfZdbh;
	BOOL  bHw2Zd;
	BOOL  bTestVol;

	DWORD dU0_8V;                    /*内部定值*/ 
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
	DWORD dTJD;//正向
	DWORD dTJDrevs;//逆向时间
	long dUAng;
	DWORD dRecNum;
	
	DWORD dDYLJ;      // 得压累计次数
	DWORD dTDYLJ;	// 得压累计时间
	
	DWORD dIBus;
	
	DWORD dLXFZ;      // add by wdj 连续分闸次数
	DWORD dTLXFZ;	  // 连续分闸时间

	DWORD dAi_S_No; //电源侧电压通道
	DWORD dAi_F_No; //负荷侧电压通道
	
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
	BYTE  byWorkMode;	 //只有FTU才有工作模式
	BYTE bFaWorkMode;   // 电压时间型模式
#if (TYPE_USER == USER_BEIJING)
	BOOL bKgMode; // 配套开关类型
#endif

	BOOL bAuto; // 集中式就地型选择
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
	BOOL  bFTUSD;   /*首端FTU*/
	BOOL  bXJDL;    /*相间短路*/
	BOOL  bDXJD;   /*单相接地*/
	BOOL  bFZBS;  /* 分闸闭锁 */
	BOOL  bSetChg;
}VPrSetPublic;

typedef struct{
	DWORD dYb;

	WORD wCfg[4];

	float fI[3];
	float fT[3];

	float fI0[3];
	float fT0[3];

    float fGfh;                         /*过负荷*/
	float fTGfh;                        

	float fTTrip;                       /*失压跳闸延时*/

    float fTRc;                         /*重合闸时间*/
    float fTRcLock;                     /*确认重合闭锁时间*/

	float fTReset;

	
    float fGy;                          /*过压*/
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

