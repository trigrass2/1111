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
	YC_TYPE_U0,   /*开口U0*/
	YC_TYPE_SU0,  /*自产U0*/ 
		
	YC_TYPE_Ia,
	YC_TYPE_Ib,
	YC_TYPE_Ic,
	YC_TYPE_I0,
	YC_TYPE_SI0,  /*自产I0*/ 
		
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
	
	YC_TYPE_SUa,  /*自产Ua*/
	YC_TYPE_SUb,  /*自产Ub*/
	YC_TYPE_SUc,  /*自产Uc*/

	YC_TYPE_SIa,  /*自产Ia*/
	YC_TYPE_SIb,  /*自产Ib*/
	YC_TYPE_SIc,  /*自产Ic*/

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

	
	YC_TYPE_Thd = 164,   //畸变
	
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

	YC_TYPE_Lrate,  //配变负载率
	YC_TYPE_Udot,   //电压越上限日累计时间 
	YC_TYPE_Udut,   //电压越下限日累计时间
	YC_TYPE_Umqr,   //电压月合格率
	YC_TYPE_Umot,   //电压越上限月累计时间 
	YC_TYPE_Umut,   //电压越下限月累计时间
	YC_TYPE_Uavg,   //电压15分钟平均值

    YC_TYPE_SIGNAL = 300,
    YC_TYPE_TEMP =400,
};	

/*  AI type 仅取
   
	YC_TYPE_Ua       1
	YC_TYPE_Ub       2
	YC_TYPE_Uc       3
	YC_TYPE_U0       4   

	YC_TYPE_Ia       6
	YC_TYPE_Ib       7
	YC_TYPE_Ic       8
	YC_TYPE_I0       9   

注意,对于YC_TYPE_Uab YC_TYPE_Ubc YC_TYPE_Uca
仅表示对于单项电压的计算遥测,如果实际现场
按线电压接线,则通道和遥测类型扔按单项配置,
此时遥测类型中不应该有上述计算量类型*/

typedef struct {
    int type;
	DWORD cfg;

	DWORD addr;    /*??*/

	DWORD admax;
	DWORD admin;

    /*---以下自动生成--*/
    int protect_enable;
    int recording;
	int pt_58v;    /*仅电压有效*/

	int ycNo;
	int fdNo;
}VAiCfg;

typedef struct{
	int type;
    DWORD cfg;

	int   arg1;
	int   arg2;
	short toZero;

    /*---以下自动生成--*/
	int cal;  /*是否运算 软件根据参数自动检测遥测项是否有效
	            如如果P遥测项参数为零序,则该项实际无效,不运算*/
	int fdNo;
	
	int xs1;
	int xs1_yc1;
	int xs2;	
	int k;   /*放大倍数*/

	int gainNo;

	int pair;  /*对方,如P对应Q的序号, Q对应P的序号, 其它无效*/

#ifdef INCLUDE_USWITCH
	int us_arg1;  /*电压切换使用,P Q 对应的电压序号*/
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

	DWORD cfg;   /*D0 = 1  电压等级为低压  D0=0 电压等级为中压(默认)
	               D1 = 1  交采一次侧数据上传时减少一位有效位(非默认)
				   D2 = 1  交采一次侧功率上传时单位为Mw或Mvar(非默认)
				   D3 = 1  无功补偿功能有效  D0=0 无无功补偿(默认)
				   D4 = 1  故障检测无效 =0 故障检测有效(默认)
				   D5 = 1  电压接入为相电压 =0 接入为线电压(默认)
				 */

	int Un;      /*二次额定电压:220V或100V,值取220或100*/
    int In;      /*二次额定电流:5A或1A,值取5或1*/
	int In0;
	
	int pt;       /*PT设置：X/Un*/
	int ct;       /*CT设置：X/In*/
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

    /*---以下自动生成--*/
    int kg_fabsno[8];
    WORD fabs_eqpid[8];
	
	//有效值地址
	DWORD *pdI[4];  
    DWORD *pdU[4];
	DWORD *pdUSlip[3];
    DWORD *pdUI0Bak;
    DWORD *pdI_Hw2[4];
    DWORD *pdI0;
	DWORD *pdSI0;   /*S表示3*/

	DWORD *pdU0;
	DWORD *pdSU0;   /*S表示3*/

    /*Uab Ubc Uca*/
	DWORD *pdUxx[3];

	//测量有效值ai通道号索引
	int mmi_meaNo_ai[MMI_MEA_NUM];
	//测量有效值yc号索引
	int mmi_meaNo_yc[MMI_MEA_NUM];
	
    //对应gain
	long gain_u[4];
	long gain_i[4];
	long gain_u0;
	long gain_i0;

	BYTE valid;
	
#ifdef INCLUDE_YK_PR_DIS
	int yk_trip_dis;        //分闸禁止
	int yk_close_dis;       //合闸禁止
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

