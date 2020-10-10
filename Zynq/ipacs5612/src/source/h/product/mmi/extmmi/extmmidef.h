#ifndef _EXTMMIDEF_H
#define _EXTMMIDEF_H

#ifdef __cplusplus
extern "C" {
#endif


#define EV_LIGHT             0x01
#define EV_INIT              0x02

#define EV_EXT_MMI           EV_LIGHT|EV_INIT


#define EXTMMI_YX_LOCAL         0x01
#define EXTMMI_YX_BS            0x02
#define EXTMMI_YX_FA            0x04
#define EXTMMI_YX_TRIPYB        0x08
#define EXTMMI_YX_RESET         0x10
#define EXTMMI_YX_YKSYN         0x20
#define EXTMMI_YX_YKF           0x40
#define EXTMMI_YX_YKH           0x80


// by gys 2017-7-28 16:00   user: shangdong
#define EXTMMI_YX_UA_CY         0x100        
#define EXTMMI_YX_UC_CY         0x200

#define EXTMMI_YX_YuK           0x400       //bit10 预置lvyi
#define EXTMMI_YX_SF             0x800     //手分
#define EXTMMI_YX_SH             0x1000   //手合
#define EXTMMI_YX_FUGUI             0x2000  //信号复归
#define EXTMMI_YX_MS             0x4000  // 模式(集中就地)
#define EXTMMI_YX_CHUKOU             0x8000 //出口投退

#define EXTMMI_YX_CELL          0x400000
#define EXTMMI_YX_CHANGE        0x80000000

#define EXTMMI_RESET            0
#define EXTMMI_YKF              1
#define EXTMMI_YKH              2
#define EXTMMI_TRIP             3
#define EXTMMI_CELL             5
#define EXTMMI_FA               8
#define EXTMMI_AUTO             9
#define XINEXTMMI_SF               14
#define XINEXTMMI_SH               15
#define XINEXTMMI_MS               16
#define XINEXTMMI_RESET          17
#define XINEXTMMI_TRIP		18
#define MMI_LED_BUF          16

#if (DEV_SP == DEV_SP_FTU)
#define EXTMMI_LIGHT_MASK      0xFF
#elif (DEV_SP == DEV_SP_WDG)
#define EXTMMI_LIGHT_MASK      0x3F
#elif (DEV_SP == DEV_SP_DTU)
#define EXTMMI_LIGHT_MASK      0x3FFFF
#endif

typedef struct
{
   float I1Set;
#if (TYPE_USER == USER_BEIJING)
   float T1Set;
#endif
   float I2Set;
   float I0Set;
   float T2Set;
   float T0Set;
   BOOL  I0Gj;
}VMMIPrSet;
typedef struct
{
   float I2Set;
   float I0Set;
   float T2Set;
   float T0Set;
   float U0Set;
}VMMIPrSet_1;
void extmmi_init(int thid);
void extmmi_run(int events);
void mmiLight(int id, int on);
int GetExtMmiYx(int index);

void extdisp_init(int thid);
void extdisp_run(DWORD events);

#ifdef __cplusplus
}
#endif

#endif
