#ifndef _PB_H
#define _PB_H

#include "mytypes.h"
#include "clock.h"

extern BYTE extEnergyT[48];
extern short mP;
extern short mQ;

typedef struct
{
   short u_index[3];
   short i_index[3];
   short uph_index[3];
   short iph_index[3];
   short s_index;
   short p_index;
   short q_index;
   BYTE  yx_start;
}VPbCfg;

typedef struct
{
   short u[3];
   short i[3];
   short p;
   short q;
   short s;
   short u_phase[3];
   short i_phase[3];
   long unBlance_U;
   long perPass_U_m[3];
   long perPass_U[3];
   long unBlance_I;
   DWORD  yx;
   DWORD  yx_bak;
   short LoadRate; 
}VPbData;

typedef struct
{
   short u15[3][15];
   short uavg[3];
   BYTE index;
   BYTE num;
}VPbAvg;

typedef struct
{
   DWORD pass_t[3];
   DWORD pass_t_m[3];
   DWORD u_unblance_t;
   DWORD i_unblance_t;
   DWORD u_off_t[3];
   DWORD u_low_t[3];
   DWORD pow_off_t;
   DWORD GZ_t;
   DWORD ZZ_t;
   DWORD UYSX_t[3];
   DWORD UYXX_t[3];
   DWORD u_over_t[3];
   DWORD u_over_t_m[3];
   DWORD u_below_t[3];
   DWORD u_below_t_m[3];
   
}VPbTime;
typedef struct
{
   DWORD  total_t;
   struct VSysClock startT;
   struct VSysClock endT;
}VPbCalTime;

typedef struct
{
   DWORD total_t;
   DWORD total_t_m;
   struct VSysClock last;
   struct VSysClock cur;
}VPbUPass;

typedef struct
{
   short u_min;
   short i_min;
   short u_rec;
   DWORD t;
}VPbUSet;

typedef struct
{
   short unblance_up;
   short unblance_rec;
   DWORD t;
}VPbUnblance;
typedef struct
{
   WORD capacity;
   float i_unblance_set;
   float u_unblance_set;
   WORD KGTime;
   float capacity_DL; 
   BYTE  EnergyT[48];
}VPbOtherSet;

typedef struct
{
   short u_pass_max;
   short u_pass_min;
   VPbUnblance u_unblance;
   VPbUnblance i_unblance;
   VPbUSet u_low;
   VPbUSet u_off;
   VPbUSet pow_off;
   VPbOtherSet otherSet;
}VPbSet;

#pragma pack(1)
typedef struct
{
   WORD  u_pass_max;  //百分比
   WORD  u_pass_min;
   WORD  u_unblance;  //百分比
   WORD  i_unblance;
   float u_unblance_t;
   float i_unblance_t;
   float u_low;      //值
   float u_low_t;
   float u_off;
   float u_off_t;
   float i_u_off;
   float pow_off;
   float pow_off_t;
   WORD  capacity;    //配变容量
   float i_unblance_set;//电流不平衡定值
   float u_unblance_set;//电压不平衡定值
   BYTE  EnergyT[48];   //电能计量时段1-48
   WORD  KGTime;        //与开关的对时周期
   float capacity_DL;   //配变最小短路容量
   DWORD rec[34];
}VPbPara;
#pragma pack()


#define PB_YX_A_OFF     0x01
#define PB_YX_B_OFF     0x02
#define PB_YX_C_OFF     0x04
#define PB_YX_A_LOW     0x08
#define PB_YX_B_LOW     0x10
#define PB_YX_C_LOW     0x20
#define PB_YX_U_UNB     0x40
#define PB_YX_I_UNB     0x80
#define PB_YX_POW_OFF   0x100
#define PB_YX_GZ     	0x200
#define PB_YX_ZZ     	0x400
#define PB_YX_POW_ON    0x800
#define PB_YX_A_Below    0x1000
#define PB_YX_B_Below    0x2000
#define PB_YX_C_Below    0x4000
#define PB_YX_A_Over   0x8000
#define PB_YX_B_Over   0x10000
#define PB_YX_C_Over   0x20000


#define PB_YX_NUM      32
#define PB_YX_START    0
#define PB_YX_TOTAL    32

long pbGetUnBlanceU(int fd);
long pbGetUnBlanceI(int fd);
short pbGetPerPassU(int fd ,int no);
void PbInit(void);
void pb(int thid);
int WritePbSet(BYTE* pdata,WORD len);
int ReadPbSet(BYTE* pdata,WORD len);
int ReadPbRunPara(WORD parano,char* pbuf);
int WritePbRunParaYZ(WORD parano,char* pbuf);
int WritePbRunPara(WORD parano,char* pbuf,WORD ParaFlag);
short pbGetLoadRate(int fd);
short pbGetPerPassUM(int fd, int no);
short pbGetOverU(int fd, int no);
short pbGetBelowU(int fd, int no);
short pbGetOverUM(int fd, int no);
short pbGetBelowUM(int fd, int no);
short pbGetAvgU(int fd, int no);

#endif
