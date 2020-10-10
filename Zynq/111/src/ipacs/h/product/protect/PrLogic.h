#ifndef _PRLOGIC_H
#define _PRLOGIC_H

#include "syscfg.h"
#include "PrSet.h"
#include "PrCal.h"
#include "PrLib.h"
#include "sys.h"
#ifdef INCLUDE_COMTRADE
#include "comtrade.h"
#endif

#define PR_KG_YX_STATUS    0x01
#define PR_KG_YX_CH        0x02

/*以下次序更改涉及写事件点号*/

#define PR_VYX_SGZ         0x00000001
#define PR_VYX_FX          0x00000002
#define PR_VYX_BHQD        0x00000004   /*保护启动*/
#define PR_VYX_I1          0x00000008
#define PR_VYX_I2          0x00000010
#define PR_VYX_I3          0x00000020
#define PR_VYX_IA          0x00000040   /*一段过流*/
#define PR_VYX_IB          0x00000080   /*二段过流*/
#define PR_VYX_IC          0x00000100   /*三段过流*/
#define PR_VYX_GFH         0x00000200   /*过负荷*/
#define PR_VYX_I0          0x00000400   /*零流*/
#define PR_VYX_DY          0x00000800   /*低电压*/
#define PR_VYX_GY          0x00001000   /*过压*/
#define PR_VYX_PT          0x00002000   /*PT断线或失压*/
#define PR_VYX_GL_WY       0x00004000
#define PR_VYX_TRIP        0x00008000

#define PR_VYX_YK_YB          0x00010000
#define PR_VYX_YKYB_YXNO  0x10

#define PR_VYX_RC          0x00020000
#define PR_VYX_TRIP_F      0x00040000  //改为分合闸异常
#if(TYPE_USER != USER_GUANGXI)
#define PR_VYX_I02     0x00080000
#endif
#define PR_VYX_HZ_BS       0x00100000  /*闭锁*/
#define PR_VYX_FZ_BS       0x00200000

#define PR_VYX_ZZ             0x00400000
#define PR_VYX_GZ             0x00800000

#define PR_VYX_DDL_CHBS	   0x01000000
#define PR_VYX_GL             0x02000000

// #define PR_VYX_I_MASK      0x000001F8
#if ((TYPE_USER == USER_BEIJING)||(TYPE_USER == USER_GUIYANG))
#define PR_VYX_I_MASK      0x010001F8         // 添加了大电流闭锁遥信
#else
#define PR_VYX_I_MASK      0x030001F8         // 添加了大电流闭锁遥信
#endif

#define PR_VYX_I0C         0x04000000
#define PR_VYX_I0_MASK     (0x00000400 | PR_VYX_I0C | PR_VYX_I02)
#define PR_VYX_TR_MASK     0x00048000
#define PR_VYX_CL_MASK     0x000A0000


#ifdef INCLUDE_FA_PUB
#define PR_VYX_FA_I        0x00000038
#define PR_VYX_FA_I0       0x00000400
#define PR_VYX_FA_WY       0x00004000
#define PR_VYX_FA_I0C      0x04000000
#endif

#define PR_VYX_GP          0x08000000

#ifdef PR_ZJJH
	#define PR_VYX_MXQY    0x10000000
#else
	#define PR_VYX_DP      0x10000000
#endif
#define PR_VYX_XL_U        0x20000000
#define PR_VYX_XL_NU       0x40000000

#define PR_VYX_I03         PR_VYX_GP
#define PR_VYX_HJS      0x80000000

#define PR_VYX_NUM         32
#if (TYPE_USER == USER_GUIYANG)
	#define PR_VYX_STS_MASK    (0xF8400000|PR_VYX_AUTOON)
#else
	#define PR_VYX_STS_MASK    0xF8400000
#endif

#if (TYPE_USER == USER_GUANGXI)
#define PR_VYX_AUTOON      0x00000002
#define PR_VYX_JSON        0x00000004
#define PR_VYX_CH1ON       0x00000800
#define PR_VYX_CH2ON       0x00001000
#define PR_VYX_JD          0x00400000
#define PR_VYX_DL          0x00800000
#define PR_VYX_ABC         0x01000000
#define PR_VYX_I02         0x02000000
#define PR_VYX_CHJS        0x04000000
#define PR_VYX_I1ON        0x08000000
#define PR_VYX_I2ON        0x10000000
#define PR_VYX_I3ON        0x20000000
#define PR_VYX_I01ON       0x40000000
#define PR_VYX_I02ON       0x80000000
#define PR_VYX_YBMASK      0xF8001806
#define PR_VYX_GZMASK      0x07C00000
#endif

#if (TYPE_USER == USER_GUIYANG)
#define PR_VYX_AUTOON      PR_VYX_GL
#endif

#if (TYPE_USER == USER_FUJIAN)
#define PR_VYX_I1DZ            0x00000040
#define PR_VYX_I2DZ            0x00000080
#define PR_VYX_I01DZ         0x00000100
#define PR_VYX_I0HJSDZ     0x00080000
#define PR_VYX_GLHJSDZ     0x02000000
#undef PR_VYX_DP
#undef PR_VYX_GP
#define PR_VYX_DP          0x01000000
#define PR_VYX_I3DZ        0x08000000
#define PR_VYX_GP          0x10000000
#endif

#define PR_CH_BS           0x01
#define PR_JS_BS           0x02
#define PR_WY_CNT          0x04
#define PR_CH_CD           0x08
#define PR_OPT_BS          0x010000
#define PR_SUDY_BS         0x020000
#define PR_SUSY_BS         0x040000
#define PR_FZ_BS           0x080000
#define PR_U0_BS           0x400000
#define PR_SY_BS           0x800000
#define PR_CY_BS           0x01000000
#define PR_DCDY_BS         0x02000000
#define PR_DU_BS		   0x04000000
#define PR_LXFZ_BS         0x08000000
#define PR_RET_BS          0x0FCF000F
#define PR_HZBS_JS         (PR_OPT_BS|PR_SUDY_BS|PR_SUSY_BS|PR_SY_BS|PR_CY_BS|PR_U0_BS | PR_DCDY_BS |PR_DU_BS|PR_LXFZ_BS)

/*公共故障遥信*/
#define PR_VYXP_I_FAULT       0x00000001
#define PR_VYXP_SL_READY      0x00000002
#define PR_VYXP_WY_FZ         0x00000004
#define PR_VYXP_S_HZ          0x00000008
#define PR_VYXP_L_HZ          0x00000010
#define PR_VYXP_HZ_BS         0x00000020
#define PR_VYXP_FZ_BS         0x00000040
#define PR_VYXP_U_MERGE       0x00000080
#define PR_VYXP_I_CNT         0x00000100
#define PR_VYXP_U0_TRIP       0x00000200
#define PR_VYXP_BS_HZZX       0x00000400
#define PR_VYXP_BS_HZFX       0x00000800
#define PR_VYXP_BS_FZZX       0x00001000
#define PR_VYXP_BS_FZFX       0x00002000
#define PR_VYXP_SEG   		  0x00004000
#define PR_VYXP_TIE           0x00008000
#define PR_VYXP_PR            0x00010000
#define PR_VYXP_FJ            0x00020000
#define PR_VYXP_DF            0x00040000   //  若定义 INCLUDE_PR_PRO  与过电压低电压遥信号重复
#define PR_VYXP_SHTQ          0x00080000   //
#define PR_VYXP_BUS           0x02000000
#define PR_VYXP_BS_DU         0x04000000   /* 两侧有压闭锁 瞬时加压闭锁 残压闭锁 遥控分闭锁 多次重合闭锁*/

#define PR_VYXP_BS_SY         0x08000000
#define PR_VYXP_BS_CY         0x10000000
#define PR_VYXP_BS_MF         0x20000000

#define PR_VYXP_BS_CH         0x40000000

#define PR_VYX_AYY            0x00100000
#define PR_VYX_CYY            0x00200000
#if (TYPE_USER == USER_BEIJING)
#define PR_VYX_FK            0x00100000
#endif
#if (TYPE_USER == USER_SHANDONG)
#define PR_VYX_S_MODE         PR_VYXP_SL_READY
#define PR_VYX_L_MODE         PR_VYXP_U_MERGE
#define PR_VYX_X_1            0x00400000
#define PR_VYX_X_2            0x00800000
#define PR_VYX_X_4            0x01000000
#define PR_VYX_X_8            0x02000000
#endif

#if ((TYPE_USER == USER_GUANGXI)||(TYPE_USER == USER_GUIYANG))
#define PR_VYXP_PU            0x00000001
#define PR_VYXP_FU            0x00000002
#define PR_VYXP_SEG_H1        0x00000100
#define PR_VYXP_SEG_H2        0x00000400
#define PR_VYXP_SEG_H3        0x00000800
#define PR_VYXP_SEG_F1        0x00001000
#define PR_VYXP_SEG_F2        0x00002000
#define PR_VYXP_SEG_F3        0x00040000
#define PR_VYXP_TIE_FZ        0x00080000
#define PR_VYXP_TIE_SH        0x00100000
#define PR_VYXP_TIE_SF        0x00200000
#define PR_VYXP_P_ZBS         0x00400000
#define PR_VYXP_P_FBS         0x00800000
#define PR_VYXP_F_ZBS         0x01000000 
#define PR_VYXP_F_FBS         0x02000000
#define PR_VYXP_SF_BS         0x04000000
#define PR_VYXP_SY_BS         0x08000000
#define PR_VYXP_HZ_JS         0x10000000
#define PR_VYXP_FG_JS         0x20000000
#define PR_VYXP_DY_JS         0x40000000
#define PR_VYXP_TIEDY_JS      0x80000000
#define PR_VYXP_MASK          0xFFFFFD03 //modify by lijun 2017-03-17 
#endif

#define PR_VYXP_WY_FZ_NO     0x02

#define PR_OT_I_FAULT      0x01
#define PR_OT_I0_FAULT     0x02
#define PR_OT_U_FAULT      0x04
#define PR_OT_IGFH_FAULT   0x08

#define KG_HZ_CONTROL      0x01
#define KG_HZ_SWITCH       0x02
#define KG_HZ_REMOTE       0x04
#define KG_FZ_CONTROL      0x08
#define KG_FZ_SWITCH       0x10
#define KG_FZ_REMOTE       0x20
#define KG_HZ_MASK         0x07
#define KG_FZ_MASK         0x38

#define HZ_BS_S            0x01
#define HZ_BS_SR           0x02
#define HZ_BS_F            0x04
#define HZ_BS_FR           0x08
#define HZ_BS_U0           0x10
#define HZ_BS_MF           0x20
#define HZ_BS_SY           0x40
#define HZ_BS_DU           0x80
#define HZ_BS_CY           0x100
#define HZ_BS_CH           0x200
#define HZ_BS_LXFZ         0x400


#define HZ_JS_MH           0x01
#define HZ_JS_FG           0x02
#define HZ_JS_DY           0x04
#define HZ_JS_TIE_DY       0x08

#define NAMELEN    16
#define MAX_BYTE_INPUT    8
#define MAX_BYTE_OUTPUT   8

#define MAX_YX_NUM       256
#define MAX_EVT_NUM      32
#define MAX_GZEVT_NUM  256
#define MAX_FAYX_NUM     64

#define MAX_GZYX_NUM  10
#define MAX_GZYC_NUM 10


#define MAX_YX_PUB_NUM  64
#define MAX_EVT_PUB_NUM  8

#define PR_EVT_QD        0x01
#define PR_EVT_TRIP      0x02
#define PR_EVT_JS        0x04
#define PR_EVT_BAK       0x08
#define PR_EVT_HH        0x10
#define PR_EVT_SLT       0x20
#define PR_EVT_I_WY      0x40
#define PR_EVT_I2_WY     0x80
#define PR_EVT_RC        0x100

#define PR_YXCHG_LOCK    0x01



//保存在铁电中
#define PR_CYBS_S 0x01
#define PR_CYBS_F 0x02
#define PR_XBS_S    0x04
#define PR_XBS_F    0x08
#define PR_YBS_S    0x10
#define PR_YBS_F    0x20
#define PR_SYBS_S   0x40    // 残压瞬压有什么区别
#define PR_SYBS_F   0x80
#define PR_HFBS_SF  0x0100

#define PR_HZBS (PR_CYBS_S |PR_CYBS_F |PR_XBS_S |PR_XBS_F | PR_YBS_S |PR_YBS_F |PR_SYBS_S |PR_SYBS_F |PR_HFBS_SF)

#ifdef _GUANGZHOU_TEST_VER_
#define EV_POWER_OFF       0x02
#endif

enum
{
  GRAPH_AD,
  GRAPH_JS,
  GRAPH_I1,
  GRAPH_I2,
  GRAPH_I3,
  GRAPH_IGFH,
  GRAPH_I01,
  GRAPH_I02,  
  GRAPH_I03,
  GRAPH_IDIR,
  GRAPH_GY,
  GRAPH_DY,
  GRAPH_WL,
  GRAPH_PT,
  GRAPH_CH,
  GRAPH_WYFZ,
  GRAPH_DJQD,
  GRAPH_BHQD,
  GRAPH_RELAY,
  GRAPH_HH,
  GRAPH_YX,
  GRAPH_IRCD,
  GRAPH_GP,
  GRAPH_DP,
  GRAPH_YXTZ,
  GRAPH_MXQY,
  GRAPH_NUM
};

enum 
{
#ifdef INCLUDE_PR_PRO
  GRAPH_CHTQ,
  GRAPH_SHTQ,
  GRAPH_DF,
#endif
  GRAPH_SLWY,
  GRAPH_FZBS,
  GRAPH_OPTBS,
  GRAPH_SYBS,
  GRAPH_SUDY,
  GRAPH_SUSY,
  GRAPH_U0,
  GRAPH_UDIFF,
  GRAPH_ICNT,
  GRAPH_LOCALYX,
  GRAPH_JD,
  GRAPH_SYHW,
  GRAPH_LXFZ,
  GRAPH_PUBLIC_NUM
};

enum
{
   BOOL_OUTPUT_A=0,
   BOOL_OUTPUT_B,
   BOOL_OUTPUT_C,
   BOOL_OUTPUT_N
};

enum
{
   BOOL_OUTPUT_FZ=0,
   BOOL_OUTPUT_IFZ,
   BOOL_OUTPUT_HZ,
   BOOL_TRIP_EVER,
   BOOL_TRIP_FAIL,
   BOOL_RC_FAIL,
   BOOL_OUTPUT_ISGZ,
   BOOL_OUTPUT_CHBS
};

enum
{
   BOOL_OUTPUT_PICK=0,
   BOOL_OUTPUT_TRIP,
   BOOL_OUTPUT_BS,
   BOOL_OUTPUT_NI1,
   BOOL_OUTPUT_NI2,
   BOOL_OUTPUT_I,
};

enum{
  BOOL_CH_CD=0, 
  BOOL_CH_QD,
  BOOL_CH_DZ,
  BOOL_CH_FD,
  BOOL_CH_FAIL
};

enum
{
   BOOL_OUTPUT_HZBS=2,
   BOOL_OUTPUT_FZBS,
   BOOL_OUTPUT_FX,
   BOOL_OUTPUT_UHZBS,
   BOOL_OUTPUT_YBS,
   BOOL_OUTPUT_U0BS,
   BOOL_OUTPUT_UCNTBS,
};


enum
{
   EVENT_PUB=100,
   EVENT_DF=0,
   EVENT_SHTQ,
   EVENT_DY_HZ,
   EVENT_SY_HZ,
   EVENT_U0,
   EVENT_UDIFF,
   EVENT_SL_WY,
   EVENT_I_CNT,
   EVENT_I_TRIP,
};

enum
{
   ValFALSE = FALSE,
   ValTRUE = TRUE
};

typedef struct TLogicElem
{
    EP_ELEMENT elem;
	BYTE       Output[MAX_BYTE_OUTPUT];
	DWORD      aplUser[8];
	void      *apvUser[2];
}LogicElem;

typedef struct TLogicEvent
{
	WORD   wType;
	char   name[NAMELEN];
}LogicEvent;

typedef struct TLogicInfo
{
    int    id;
	BYTE   name[NAMELEN];
	void  (*Init)(LogicElem *plogic);
	void  (*Func)(LogicElem *plogic);
}LogicInfo;

typedef struct TDBYxChg
{
	WORD flag;
    struct VDBSOE DBSOE;
}VDBYxChg;

typedef struct TPrYxOpt
{
	VDBYxChg yxchg[MAX_YX_NUM];
	WORD wReadPtr;
	WORD wWritePtr;
}VPrYxOpt;

typedef struct TPrYxPubOpt
{
	VDBYxChg yxchg[MAX_YX_PUB_NUM];
	WORD wReadPtr;
	WORD wWritePtr;
}VPrYxPubOpt;


typedef struct TDBGzEvent
{
	WORD fd;
	WORD YxNum;
	WORD YxNo[MAX_GZYX_NUM];
	WORD YxValue[MAX_GZYX_NUM];
	WORD YcNum;
	WORD YcNo[MAX_GZYC_NUM];
	DWORD YcValue[MAX_GZYC_NUM]; //先电压后电流
	struct VCalClock Time[MAX_GZYX_NUM];
}VDBGzEvent;

typedef struct TPrGzEvtOpt
{
	VDBGzEvent evt[MAX_GZEVT_NUM];
	WORD wReadPtr;
	WORD wWritePtr;
}VPrGzEvtOpt;

typedef struct TDBEvent
{
   WORD   fd;
   DWORD  flag;
   DWORD  code;
   DWORD  data[8];
   struct VCalClock Time; 
}VDBEvent;

typedef struct TPrEvtOpt
{
    VDBEvent evt[MAX_EVT_NUM];
	WORD wReadPtr;
	WORD wWritePtr;
}VPrEvtOpt;

typedef struct TPrEvtPubOpt
{
    VDBEvent evt[MAX_EVT_PUB_NUM];
	WORD wReadPtr;
	WORD wWritePtr;
}VPrEvtPubOpt;

typedef struct{
	struct VCalClock i_start_clock;  /*启动时间*/

	DWORD i_sec[4];           /*过流段*/
	DWORD i_sec_z;           /*过流段*/
	DWORD i_phase;	        /*过流相*/
	DWORD i_phase_bak;
	DWORD i_sgz;
	DWORD i_sgz_yx;
	BOOL  i_sgz_fx;
	DWORD freq;

	struct VCalClock i_clock;
	struct VCalClock sgz_clock;
	DWORD i_cnt;
	DWORD ifg_cnt;
	DWORD i0_sec;
	BOOL i0_start;
	BOOL i0_xdl;
	BOOL i0_xdltrip;
	BOOL i0_xdlgj;
	DWORD rc_num;
	
	DWORD i[4];	
	DWORD u[4];
	long  ang[2];

	struct VCalClock yk_clock;
}VPrFaultInfo;

#define YC_FAULT_REC     256
typedef struct {
	struct VDBYC yc[YC_FAULT_REC];
	WORD  wp;
	WORD  rp;
	WORD  size;
}VYcFaultIRec;

#ifdef INCLUDE_COMTRADE
typedef struct {
	BOOL  start;
	BOOL  stop;
	BOOL  connect_pre;
	BOOL  connect_next;
}VPrWaveInfo;
#endif

typedef struct {
    int fd;
    int set_no;
	int prSetnum;
	struct VCalClock prClock;
	DWORD vyx;
	DWORD vyx_bak;
	DWORD vyx_keep;
	WORD vyx_start_no;

	DWORD bs_reset;

	VPrFaultInfo fault;
	WORD  work_mode;
	WORD  yk_type;
	WORD  bs_type;
	DWORD dw_NVRAM_BS;
	DWORD dw_nvram_addr;
#ifdef  _YIERCI_RH_
	WORD fz_state; 
#endif
	WORD  js_type;
	BYTE  x_fx;
	BOOL  su;
	BOOL  fu;
	BOOL  no_u;
}VPrRunPublic;

typedef struct {
    int fd; 
	int set_no;
	
	struct VCalClock prClock;

	DWORD bhqd_state;
	BYTE  trip_flag;  //跳闸与否
	BOOL  gl_exist;
	BOOL  normal_sts;
	BOOL  normal_sts_bak;
	BOOL  gzjc_on;
		
	DWORD vyx;
	DWORD vyx_bak;
	WORD vyx_start_no;	
	DWORD vyx_oc_mask;
	DWORD vyx_warn_mask;

	int kg_yx_valid;  
	DWORD bs_reset;
	DWORD reset;
	VPrFaultInfo fault;

	BOOL kg_ch_ddlbs;          // 重合闭锁
#ifdef INCLUDE_COMTRADE
	VPrWaveInfo wave;
#endif

#ifdef INCLUDE_FA
	DWORD fa_states;
	DWORD fa_trip;
#endif
    BOOL i_normal;
}VPrRunInfo;  

extern int pubfdNum;
void prTick(void);
int  prInit(void);
void protect(void);
void prReset(int fd);
BOOL GetFaultYx(int fd);
void ResetProtect(int fd);
void prLightHandle(void);
void prDataHandle(void);
int prRunInfoInit(void);
int prGetBhqd(void);
#ifdef INCLUDE_FA
BOOL prFaLogic(int fd, int faulttype);
void prFaTrip(int fd,int trip);
DWORD prGetStatus(int fd);
#endif
void WriteYxDelay(int no, int value, int lock);
int prCheckShTq(int ykId, int val);
BOOL prValue_long(int fd, int ui_flag, int *value);
void prValue_float(int ui_flag, int fd, float *value);
void prSetting(int fd, DWORD *setting, int *channel);
short prNodeCoef(int fd, int flag);
BOOL prReadGzEvent(VDBGzEvent* pGzEvt);

BOOL GetHzbsYx(void);
void ykTypeSave(int type, int id, int val);

#ifdef  _YIERCI_RH_
void SbFzStateSave(int val);
#endif
#if (TYPE_USER == USER_BEIJING)
BOOL  KgType(void);
extern BOOL bFK; 
#endif

#define  EV_PR_FAULT               0x2000   


#endif
