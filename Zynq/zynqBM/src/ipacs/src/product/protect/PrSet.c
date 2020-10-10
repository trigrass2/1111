#include "syscfg.h"
#include "sys.h"
#include "myio.h"

#ifdef INCLUDE_PR
#include "PrSet.h"

BYTE prTableBuf[MAX_TABLE_FILE_SIZE];
static BYTE prSetTmp[PR_SET_SIZE+sizeof(struct VFileHead)];
BYTE FileSetNum = 0;

/*为方便与以前的程序定值兼容，定值以后尽量在最后面添加*/

//保护整定参数 缺省值表
TSETTABLE tSetTable[]={
  {"软压板……………",{0x20,0x00,0x00,0x00,0x00,0x00},{0x20,0x00,0xff,0xff,0xff,0xff},{0x20,0x00,0x01,0x00,0x00,0x00}},
  {"控制字一…………",{0x21,0x00,0x00,0x00,0x00,0x00},{0x21,0x00,0xff,0xff,0xff,0xff},{0x21,0x00,0x00,0x0F,0x00,0x00}},//0000～FFFF
  {"控制字二…………",{0x21,0x00,0x00,0x00,0x00,0x00},{0x21,0x00,0xff,0xff,0xff,0xff},{0x21,0x00,0x00,0x00,0x00,0x00}},//0000～FFFF

  {"电流Ⅰ段…………",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.20～100.0
  {"电流Ⅰ段时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},//0.00～20.00
  {"电流Ⅱ段…………",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.20～100.0
  {"电流Ⅱ段时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},//0.00～20.00
  {"电流Ⅲ段…………",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.20～100.0
  {"电流Ⅲ段时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x99,0x00,0x00},{0x25,0x02,0x00,0x99,0x00,0x00}},//0.00～20.00
  {"过流角度下限……",{0x27,0x82,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x00,0x00,0x00}},//0.20～100.0
  {"过流角度上限……",{0x27,0x82,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x00,0x00,0x00}},//0.00～20.00
  {"过励磁电流………",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00}},//0.20～100.0
  {"过励磁时间………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x20,0x01,0x00},{0x25,0x02,0x00,0x20,0x01,0x00}},//0.10～60.00
  
  {"电流过负荷………",{0x24,0x02,0x20,0x00,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00}},//0.20～100.0
  {"电流过负荷时间…",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x72,0x00},{0x25,0x02,0x00,0x00,0x72,0x00}},//0.00～20.0 0.014
  {"负荷遮断电流……",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.20～100.0
  {"过电压……………",{0x23,0x02,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x40,0x00,0x00},{0x23,0x02,0x00,0x01,0x00,0x00}},//0~250
  {"过电压时间………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},

  {"低电压……………",{0x23,0x02,0x00,0x20,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00},{0x23,0x01,0x00,0x07,0x00,0x00}},
  {"低电压时间………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},
  {"低压闭锁电压滑差",{0x2B,0x02,0x00,0x01,0x00,0x00},{0x2B,0x01,0x00,0x10,0x00,0x00},{0x2B,0x01,0x00,0x01,0x00,0x00}},
  {"线路有压…………",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00},{0x23,0x01,0x00,0x07,0x00,0x00}},
  {"线路无压…………",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00},{0x23,0x01,0x00,0x03,0x00,0x00}}, 
  {"零压………………",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x10,0x00,0x00},{0x23,0x01,0x00,0x10,0x00,0x00}},//0.8~1.5Un
  {"零流Ⅰ段…………",{0x24,0x02,0x01,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.8~1.5Un
  {"零流Ⅰ段时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},
  {"零流Ⅱ段…………",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00}},//0.8~1.5Un
  {"零流Ⅱ段时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x72,0x00},{0x25,0x02,0x00,0x20,0x00,0x00}},
  {"零流Ⅲ段…………",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00}},//0.8~1.5Un
  {"零流Ⅲ段时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x72,0x00},{0x25,0x02,0x00,0x20,0x00,0x00}},
  {"零流角度下限……",{0x27,0x82,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x00,0x00,0x00}},//0.20～100.0
  {"零流角度上限……",{0x27,0x82,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x00,0x00,0x00}},//0.00～20.00
  {"小电流零压突变…",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00},{0x23,0x01,0x10,0x00,0x00,0x00}}, 
  {"小电流零流突变…",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x05,0x00,0x00,0x00}}, 
  {"小电流电流突变…",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x10,0x00,0x00,0x00}},
  {"小电流接地时间…",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x05,0x00,0x00}},
  {"小电流接地系数…",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x10,0x00,0x00,0x00},{0x25,0x01,0x05,0x00,0x00,0x00}},

  {"闭锁重合闸大电流",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.20～100.0
  {"重合闸次数………",{0x22,0x01,0x00,0x00,0x00,0x00},{0x22,0x01,0x30,0x00,0x00,0x00},{0x22,0x01,0x10,0x00,0x00,0x00}},//0.8~1.5Un
  {"一重时间…………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x06,0x00},{0x25,0x02,0x00,0x03,0x00,0x00}},//0.8~1.5Un
  {"二重时间…………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x00,0x20,0x01,0x00}},
  {"三重时间…………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x00,0x20,0x01,0x00}},//0.8~1.5Un
  {"重合闭锁时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x00,0x05,0x00,0x00}},//0.8~1.5Un
  {"信号保持时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x00,0x10,0x00,0x00}},
  {"故障灯复归时间…",{0x25,0x01,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x00,0x32,0x04},{0x25,0x01,0x00,0x80,0x28,0x00}},
  {"后加速时间………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x20,0x00,0x00,0x00}},
  {"后加速复归时间…",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x00,0x20,0x01,0x00}},
  {"外部遥信跳闸……",{0x22,0x00,0x00,0x00,0x00,0x00},{0x22,0x00,0x00,0x03,0x00,0x00},{0x22,0x00,0x00,0x03,0x00,0x00}},

  {"高频保护定值……",{0x29,0x02,0x00,0x45,0x00,0x00},{0x29,0x02,0x00,0x60,0x00,0x00},{0x29,0x02,0x00,0x60,0x00,0x00}},
  {"高频保护时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},
  {"低频保护定值……",{0x29,0x02,0x00,0x45,0x00,0x00},{0x29,0x02,0x00,0x60,0x00,0x00},{0x29,0x02,0x00,0x45,0x00,0x00}},
  {"低频保护时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},
  {"过流后加速电流…",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},
  {"零序后加速电流…",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},
  {"零序后加速时间…",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x20,0x00,0x00,0x00}},
  {"母线有压…………",{0x23,0x02,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x80,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00}},//0~250
  {"母线有压时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},
  {"母线欠压…………",{0x23,0x02,0x00,0x20,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00},{0x23,0x01,0x00,0x07,0x00,0x00}},
  {"母线欠压时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},

};
BYTE SET_NUMBER = sizeof(tSetTable)/sizeof(tSetTable[0]);

TYBTABLE tYbTable[] =
{
  {"故障检测…………", 0x00,  YB_GZJC},
  {"过流一段…………", 0x02,  YB_I1},
  {"过流二段…………", 0x02,  YB_I2},
  {"过流三段…………", 0x02,  YB_I3},
  {"过负荷……………", 0x02,  YB_GFH},
  {"过压………………", 0x02,  YB_GY},
  {"低压减载…………", 0x02,  YB_DY},
  {"零流一段…………", 0x02,  YB_I01},
  {"零流二段…………", 0x02,  YB_I02},
  {"零流三段…………", 0x02,  YB_I03},
  {"励磁涌流…………", 0x02,  YB_DJQD},
  {"PT断线……………", 0x02,  YB_PT},
  {"重合闸……………", 0x02,  YB_CH},
  {"过流加速…………", 0x02,  YB_JS},
  {"高频………………", 0x02,  YB_GP},
  {"低频………………", 0x02,  YB_DP},
  {"母线欠压…………", 0x02,  YB_MXQY},
  {"外部遥信跳闸投入",0x02,YB_TZ},
  
};
BYTE YB_NUMBER=sizeof(tYbTable)/sizeof(tYbTable[0]);

char* szFsxCuv[] = {"曲线1   ","曲线2   ","曲线3   "};
char* szQd[]     = {"定时限  ","反时限  "};
char* szDz[]     = {"报警    ","跳闸    "};
char* szBS[]     = {"不闭锁  ","闭锁    "};
char* bsJS[]     = {"自动解锁","手动解锁"};
char *szTt[]     = {"退出    ","投入    "};
char *szJr[]     = {"未接入  ","接入    "};
char* szPMx[]    = {"指向线路","指向母线"};
char* szCd[]     = {"配网保护","继电保护"};
char* szTrip[]   = {"过流    ","过流失压","二次失压","开关自跳","不跳闸  "};
char* szReport[] = {"过流    ","跳闸    ","重合失败"};
char* szKgMd[]   = {"线路    ","出口    "};
char* szFaultDz[]   = {"告警    ","跳闸    "};
char *szXDLJr[]     = {"零压    ","相电压  "};


TKGTABLE tKG1Table[] =
{
  {"反时限曲线………",0x02, {KG_0,KG_1,0xff}, 2, 3,szFsxCuv},
  {"过负荷保护………",0x02, {KG_2,0xff,0xff}, 1, 2,szQd},
  {"过负荷故障动作…",0x02, {KG_3,0xff,0xff}, 1, 2,szDz},
  {"PT断线闭锁………",0x02, {KG_4,0xff,0xff}, 1, 2,szBS},
  {"低电压闭锁………",0x02, {KG_5,0xff,0xff}, 1, 2,szBS},
  {"过流方向元件……",0x02, {KG_6,0xff,0xff}, 1, 2,szTt},
  {"过流方向指向……",0x02, {KG_7,0xff,0xff}, 1, 2,szPMx},
  {"曾有压元件………",0x02, {KG_8,0xff,0xff}, 1, 2,szTt},
  {"UA相电压…………",0x02, {KG_9,0xff,0xff}, 1, 2,szJr},
  {"UB相电压…………",0x02, {KG_10,0xff,0xff}, 1, 2,szJr},
  {"UC相电压…………",0x02, {KG_11,0xff,0xff},1, 2,szJr},
  {"零流方向指向……",0x02, {KG_12,0xff,0xff},1, 2,szPMx},
  {"零流方向元件……",0x02, {KG_13,0xff,0xff},1, 2,szTt},
  {"零流电压启动……",0x02, {KG_14,0xff,0xff},1, 2,szTt},
  {"大电流重合闭锁…",0x02, {KG_15,0xff,0xff},1, 2,szTt},
  {"重合闸充电模式…",0x02, {KG_16,0xff,0xff},1, 2,szCd},
  {"过流信号条件……",0x02, {KG_17,KG_18,KG_19}, 3, 5, szTrip},
  {"故障上报条件……",0x02, {KG_20,KG_21,0xff},2, 3, szReport},
  {"线路电压检测……",0x02, {KG_22,0xff,0xff},1, 2, szTt},
  {"保护启动不记录…",0x02, {KG_23,0xff,0xff},1, 2, szTt},
  {"非遮断电流保护…",0x02, {KG_24,0xff,0xff},1, 2, szTt},
  {"励磁涌流谐波制动",0x02, {KG_25,0xff,0xff},1, 2, szTt},
  {"故障灯自动复归…",0x02, {KG_26,0xff,0xff},1, 2, szTt},
  {"过流一段出口……",0x02, {KG_27,0xff,0xff},1, 2, szFaultDz},
  {"过流二段出口……",0x02, {KG_28,0xff,0xff},1, 2, szFaultDz},
  {"零序过流出口……",0x02, {KG_29,0xff,0xff},1, 2, szFaultDz},
  {"过流二低电压闭锁",0x02, {KG_30,0xff,0xff}, 1, 2,szBS},
  {"过流二段方向元件",0x02, {KG_31,0xff,0xff},1, 2,szTt},
};
const BYTE KG1_NUMBER=sizeof(tKG1Table)/sizeof(tKG1Table[0]);

char* szSgzYxMode[] = {"方向元件", "无      "};
char* szSignMode[] =  {"正      ", "负      "};
char* sZCHZMode[] = {"不检 ",  "检无压"};
TKGTABLE tKG2Table[] = 
{
  {"过流遥信引入……",0x02, {KG_2,0xff,0xff},1,2,szSgzYxMode},
  {"开关接线属性……",0x02, {KG_3,0xff,0xff},1,2,szSignMode},
  {"过流故障录波……",0x02, {KG_4,0xff,0xff},1, 2, szTt},
  {"线路失压录波……",0x02, {KG_5,0xff,0xff},1, 2, szTt},
  {"零序电压录波……",0x02, {KG_6,0xff,0xff},1, 2, szTt},
  {"零序电流录波……",0x02, {KG_7,0xff,0xff},1, 2, szTt},
  {"小电流接地投入…",0x02, {KG_8,0xff,0xff},1, 2, szTt},
  {"小电流接地出口…",0x02, {KG_9,0xff,0xff},1, 2, szFaultDz},
  {"小电流接入方式…",0x02, {KG_10,0xff,0xff},1, 2, szXDLJr},
  {"过流保护录波……",0x02, {KG_11,0xff,0xff},1, 2, szTt},
  {"开关跳闸异常投入",0x02, {KG_12,0xff,0xff},1, 2, szTt},
  {"接地判断电流方向",0x02, {KG_13,0xff,0xff},1,2,szSignMode},
  {"过压故障动作",0x02, {KG_14,0xff,0xff},1, 2, szDz},
  {"低压故障动作",0x02, {KG_15,0xff,0xff},1, 2, szDz},
  {"零序电流突变录波",0x02, {KG_16,0xff,0xff},1, 2, szTt},
  {"过频故障动作……",0x02, {KG_17,0xff,0xff},1, 2, szDz},
  {"低频故障动作……",0x02, {KG_18,0xff,0xff},1, 2, szDz},
  {"过流三段出口……",0x02, {KG_19,0xff,0xff},1, 2, szFaultDz},
  {"母线欠压出口……",0x02, {KG_20,0xff,0xff},1, 2, szFaultDz},
  
};
BYTE KG2_NUMBER=sizeof(tKG2Table)/sizeof(tKG2Table[0]);

TSETTABLE tSetPubTable[]={
  {"软压板……………",{0x20,0x00,0x00,0x00,0x00,0x00},{0x20,0x00,0xff,0xff,0xff,0xff},{0x20,0x00,0x00,0x00,0x00,0x00}},
  {"控制字一…………",{0x21,0x00,0x00,0x00,0x00,0x00},{0x21,0x00,0xff,0xff,0xff,0xff},{0x21,0x00,0x00,0x00,0x00,0x00}},//0000～FFFF
  {"控制字二…………",{0x21,0x00,0x00,0x00,0x00,0x00},{0x21,0x00,0xff,0xff,0xff,0xff},{0x21,0x00,0x00,0x00,0x00,0x00}},//0000～FFFF

  {"SL回线……………",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x20,0x00,0x00,0x00},{0x2D,0x00,0x01,0x00,0x00,0x00}},
  {"X 时限……………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x00,0x01,0x00},{0x25,0x01,0x70,0x00,0x00,0x00}},
  {"Y 时限……………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x20,0x00,0x00},{0x25,0x01,0x50,0x00,0x00,0x00}},
  {"分闸闭锁复归时间",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x20,0x00,0x00},{0x25,0x01,0x00,0x00,0x00,0x00}},
  {"零压时间…………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x02,0x00,0x32,0x00,0x00}},
  {"零压………………",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x10,0x00,0x00},{0x23,0x01,0x00,0x10,0x00,0x00}},//0.8~1.5Un
  {"瞬压残压时间……",{0x25,0x03,0x05,0x00,0x00,0x00},{0x25,0x03,0x00,0x00,0x01,0x00},{0x25,0x03,0x00,0x01,0x00,0x00}},
  {"残压………………",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x20,0x00,0x00},{0x23,0x01,0x00,0x03,0x00,0x00}},
  {"两侧压差…………",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x03,0x00,0x00},{0x23,0x01,0x00,0x03,0x00,0x00}},
  {"失压分闸时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x01,0x35,0x00,0x00,0x00}},
  {"电流计数…………",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x09,0x00,0x00,0x00},{0x2D,0x00,0x01,0x00,0x00,0x00}},
  {"电流计数复位时间",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x01,0x35,0x00,0x00,0x00}},
  {"电流计数累计时间",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x01,0x35,0x00,0x00,0x00}}, 
#ifdef INCLUDE_PR_PRO
  {"同期低频回线……",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x20,0x00,0x00,0x00},{0x2D,0x00,0x01,0x00,0x00,0x00}},
  {"低频闭锁频率滑差",{0x2A,0x02,0x50,0x00,0x00,0x00},{0x2A,0x01,0x00,0x02,0x00,0x00},{0x2A,0x01,0x10,0x00,0x00,0x00}},
  {"低频定值…………",{0x29,0x02,0x00,0x45,0x00,0x00},{0x29,0x01,0x95,0x04,0x00,0x00},{0x29,0x01,0x90,0x04,0x00,0x00}},
  {"低频时间…………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x20,0x00,0x00}},
  {"低频电压闭锁……",{0x23,0x02,0x00,0x15,0x00,0x00},{0x23,0x02,0x00,0x20,0x01,0x00},{0x23,0x02,0x00,0x20,0x01,0x00}},
  {"低频电流闭锁……",{0x24,0x02,0x04,0x00,0x00,0x00},{0x24,0x02,0x00,0x00,0x01,0x00},{0x24,0x02,0x00,0x00,0x01,0x00}},
  {"同期角差…………",{0x27,0x02,0x00,0x10,0x00,0x00},{0x27,0x02,0x00,0x50,0x00,0x00},{0x27,0x02,0x00,0x30,0x00,0x00}},
  {"合闸导前时间……",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x02,0x00,0x00},{0x25,0x02,0x20,0x00,0x00,0x00}},
  {"准同期电压差……",{0x23,0x02,0x00,0x00,0x00,0x00},{0x23,0x02,0x00,0x20,0x00,0x00},{0x23,0x02,0x00,0x00,0x00,0x00}},
  {"准同期频率差……",{0x29,0x02,0x00,0x00,0x00,0x00},{0x29,0x02,0x00,0x02,0x00,0x00},{0x29,0x02,0x50,0x00,0x00,0x00}},
  {"准同期加速度……",{0x2A,0x02,0x00,0x00,0x00,0x00},{0x2A,0x02,0x00,0x05,0x00,0x00},{0x2A,0x02,0x00,0x01,0x00,0x00}},
 #endif
  {" C 时间……………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x20,0x00,0x00},{0x25,0x01,0x00,0x03,0x00,0x00}},
  {" S 时间……………",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x00,0x01,0x00},{0x25,0x01,0x70,0x05,0x00,0x00}},
  {"选线跳闸重合时间",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x20,0x00,0x00},{0x25,0x01,0x20,0x00,0x00,0x00}},
  {"接地故障正向时间",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00}},
  {"接地故障反向时间",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00}},
  {"压差合环角差……",{0x27,0x02,0x00,0x10,0x00,0x00},{0x27,0x02,0x00,0x50,0x00,0x00},{0x27,0x02,0x00,0x30,0x00,0x00}},
  {"录波数量(重启)…",{0x2D,0x00,0x20,0x00,0x00,0x00},{0x2D,0x00,0x28,0x01,0x00,0x00},{0x2D,0x00,0x64,0x00,0x00,0x00}},

  {"得压闭锁累计次数",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x00,0x01,0x00,0x00},{0x2D,0x00,0x04,0x00,0x00,0x00}},
  {"得压闭锁累计时间",{0x25,0x00,0x00,0x00,0x00,0x00},{0x25,0x00,0x00,0x09,0x00,0x00},{0x25,0x00,0x00,0x03,0x00,0x00}},
  {"电压型另一回线…",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x20,0x00,0x00,0x00},{0x2D,0x00,0x02,0x00,0x00,0x00}},
  {"母线差动电流……",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},
 
  {"连续分闸累计次数",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x00,0x01,0x00,0x00},{0x2D,0x00,0x03,0x00,0x00,0x00}},//add by wdj 
  {"连续分闸设定时间",{0x25,0x00,0x00,0x00,0x00,0x00},{0x25,0x00,0x00,0x60,0x00,0x00},{0x25,0x00,0x00,0x06,0x00,0x00}},

  //添加电压时间型通道最大64，默认0无效
  {"电源侧电压通道…",{0x2D,0x00,0x00,0x00,0x00,0x00},{0x2D,0x00,0x64,0x00,0x00,0x00},{0x2D,0x00,0x00,0x00,0x00,0x00}},
  {"负荷侧电压通道…",{0x2D,0x00,0x00,0x00,0x00,0x00},{0x2D,0x00,0x64,0x00,0x00,0x00},{0x2D,0x00,0x00,0x00,0x00,0x00}},
};
 
BYTE SET_PUB_NUMBER = sizeof(tSetPubTable)/sizeof(tSetPubTable[0]);

TYBTABLE tYbPubTable[] =
{
  {"S 功能……………", 0x02,  YB_SUDY},
  {"L 功能……………", 0x02,  YB_SUSY},
  {"零压………………", 0x02,  YB_U0},
  {"压差合环功能……", 0x02,  YB_UDIFF},
  {"无压跳闸…………", 0x02,  YB_SLWY},
  {"电流计数…………", 0x02,  YB_DLCNT},
#ifdef INCLUDE_PR_PRO
  {"低频减载…………", 0x02,  YB_DF},
  {"手动同期合闸……", 0x02,  YB_SHTQ},
#endif
  {"接地故障处理……", 0x02,  YB_JD},
  {"硬件残压模块处理", 0x02,  YB_YJCY},
  {"母线差动投入……",0x02,   YB_BUS},

  {"连续分闸闭锁合闸",0x02,	YB_LXFZ}, //add by wdj

};
BYTE YB_PUB_NUMBER=sizeof(tYbPubTable)/sizeof(tYbPubTable[0]);

char* szUIType[]  = {"电压型  ","电压电流"};
char* szPower[]   = {"UA      ","UC      "};
char* szPubLine[] = {"单回线  ","双回线  "};
#ifdef INCLUDE_PR_PRO
char* szDf[]     = {"减载    ","解列    "};
char* szChMode[] = {"不检    ","检无压  ","检同期  ","无压同期"};
char* szShMode[] = {"不检    ","检无压  ","检同期  ","准同期  ","无压同期","无压准同"};
char* szWyAny[]  = {"检线路侧","检任一侧"};
char* szTqLine[] = {"单回线  ","双回线  "};
#endif
TKGTABLE tKG1PubTable[] = 
{
  {"X 时限闭锁………",0x02, {KG_0,0xff,0xff},1, 2,szBS},
  {"Y 时限闭锁………",0x02, {KG_1,0xff,0xff},1, 2,szBS},
  {"双压闭锁…………",0x02, {KG_2,0xff,0xff},1, 2,szBS},
  {"瞬压闭锁…………",0x02, {KG_3,0xff,0xff},1, 2,szBS},
  {"电压电流型………",0x02, {KG_4,0xff,0xff},1, 2,szUIType},
  {"零压故障判断……",0x02, {KG_5,0xff,0xff},1, 2,szDz},
#ifdef INCLUDE_PR_PRO
  {"低频动作…………",0x02, {KG_6,0xff,0xff},1, 2,szDf},
  {"低频电流闭锁……",0x02, {KG_7,0xff,0xff},1, 2,szTt},
  {"重合闸方式………",0x02, {KG_8,KG_9,0xff},2,4,szChMode},
  {"手动合闸方式……",0x02, {KG_10,KG_11,KG_12},3,6,szShMode},
  {"重合无压检查……",0x02, {KG_13,0xff,0xff},1, 2,szWyAny},
  {"同期单回线………",0x02, {KG_14,0xff,0xff},1, 2,szTqLine},
#endif
  {"合闸闭锁解锁方式",0x02, {KG_15,0xff,0xff},1, 2,bsJS},
  {"首端FTU 投入……",0x02, {KG_16,0xff,0xff},1, 2,szTt},	
  {"自适应相间短路…",0x02, {KG_17,0xff,0xff},1, 2,szTt},
  {"自适应单相接地…",0x02, {KG_18,0xff,0xff},1, 2,szTt},
  {"电源通道…………",0x02, {KG_19,0xff,0xff},1, 2,szPower},
  {"合成功分闸闭锁…",0x02, {KG_20,0xff,0xff},1, 2,szBS},
  {"电压型保护………",0x02, {KG_21,0xff,0xff},1, 2,szPubLine},    
  {"另一侧首端投入…",0x02, {KG_22,0xff,0xff},1, 2,szTt},  
  {"母线故障动作……",0x02, {KG_23,0xff,0xff},1, 2,szDz},  
};

BYTE KG1_PUB_NUMBER=sizeof(tKG1PubTable)/sizeof(tKG1PubTable[0]);

char* szWorkMode[] ={"分段    ","联络    ","保护    ","分界    "};
char* faWorkMode[] ={"默认 ","电压型","电流型","自适应型"};
#if (TYPE_USER == USER_BEIJING)
char* ptKgMode[] = {"弹簧开关","电磁开关"};
#endif
#if (TYPE_USER == USER_GUIYANG)&&(DEV_SP == DEV_SP_WDG)
char* ptAutoMode[] = {"投入","退出"};
#endif
char* ptFaTripMode[] = {"不出口","出口"};
char* ptDLXBSMode[] = {"判失压","不判失压"};
char* pt_SUSYMode[] = {"合闸","告警"};

TKGTABLE tKG2PubTable[] = 
{
   {"工作模式…………",0x02, {KG_0,KG_1,KG_2},3,4,szWorkMode},
   {"电压电流型模式…",0x02, {KG_3,KG_4,KG_5},3,4,faWorkMode},
#if (TYPE_USER == USER_BEIJING)
   {"配套开关类型……",0x02, {KG_6,0xff,0xff},1,2,ptKgMode},
#endif
#if (TYPE_USER == USER_GUIYANG)&&(DEV_SP == DEV_SP_WDG)
	{"自动化……………",0x02, {KG_7,0xff,0xff},1,2,ptAutoMode},
#endif
	{"电流型合故障判断",0x02, {KG_8,0xff,0xff},1,2,ptDLXBSMode},	
	{"电流型合故障出口",0x02, {KG_9,0xff,0xff},1,2,ptFaTripMode},
	{"L 功能合闸出口…",0x02, {KG_10,0xff,0xff},1,2,pt_SUSYMode},		

};

BYTE KG2_PUB_NUMBER=sizeof(tKG2PubTable)/sizeof(tKG2PubTable[0]);

#ifdef INCLUDE_61850
TYBXMLTABLE gYbXmlTable[]=
{
    {false, false, 0, "故障检测", "", "",  {255,}, {255,}},
    {true,  true,  1, "过流一段", "PTOC", "OvCur", {SET_I1, SET_T1, 255,}, {SET_IANG1, SET_IANG2, SET_IKG,255,}},
    {true,  true,  2, "过流二段", "PTOC", "OvCur", {SET_I2, SET_T2, 255,}, {255,}},
    {true,  true,  3, "过流三段", "PTOC", "OvCur", {SET_I3, SET_T3, 255,}, {255,}},
    {true,  true,  4, "过负荷", "PTOC", "OvCur", {SET_IGFH, SET_TGFH, 255,}, {255,}},
    {true,  true,  1, "过压", "PTOV", "OvVol", {SET_UGY, SET_TGY, 255,}, {255,}},
    {true,  true,  1, "低压减载", "PTUV", "UnVol", {SET_UDY, SET_TDY, 255,}, {SET_USLIP, 255,}},
    {true,  true,  1, "零流一段", "PTOC", "ZerOc", {SET_3I01, SET_TI01, 255,}, {SET_3U0, SET_I0ANG1, SET_I0ANG2, 255,}},
    {true,  true,  2, "零流二段", "PTOC", "ZerOC", {SET_3I02, SET_TI02, 255,}, {255,}},
    {true,  true,  3, "零流三段", "PTOC", "ZerOc", {SET_3I03, SET_TI03, 255,}, {255,}},
    {true,  true,  5, "励磁涌流", "PTOC", "OvCur", {255,},{SET_IM, SET_TQDQJ, 255,}},
    {true , false, 0, "PT断线", "", "",  {255,}, {255,}},
    {true,  true,  1, "重合闸", "RREC", "Rec",   {255,},{SET_CHNUM, SET_TCH1, SET_TCH2, SET_TCH3, SET_TQDQJ, 255,}},
    {true,  false, 0, "无压", "", "",  {255,}, {255,}},
    {true,  false, 0, "过流加速", "", "", {255,}, {255,}},
#ifdef INCLUDE_PR_U
	{true,  true,  1, "电压合闸", "RREC", "Vol",   {255,},  {SET_TX, SET_TY, 255,}},
	{true,  false, 0, "L型合闸", "", "", {255,}, {255,}},
	{true,  true,  1, "零压","PTOV", "OvVol0", {255,}, {SET_TU0, 255,}},
	{true,  true,  2, "压差合环", "RREC", "Vol",    {255,}, {SET_UDIFF, 255,}},
#endif
	{true,  true,  1, "低频减载", "PTUF", "UnHz",   {SET_FREQ, SET_TDF, SET_UFREQ, SET_IFREQ, 255}, {SET_IFREQ,255}},
	{true,  true,  2, "手合同期", "RREC", "Rec",    {255,}, {SET_TQTDQ,SET_TQUDIFF, SET_TQFDIFF, SET_TQFSDIFF,255}},
};

TSETXMLTABLE gSetXmlTable[]=
{
    {0,   "Yb",          "压板",             XML_DA_TYPE_INT},
    {0,   "KG1",         "控制字一",         XML_DA_TYPE_INT},
    {0,   "KG2",         "控制字二",         XML_DA_TYPE_INT},
    {1,   "StrVal",      "过流I段电流",      XML_DA_TYPE_VAL},
    {1,   "OpDlTmms",    "过流I段时间",      XML_DA_TYPE_INT},
    {2,   "StrVal",      "过流II段电流",     XML_DA_TYPE_VAL},
    {2,   "OpDlTmms",    "过流II段时间",     XML_DA_TYPE_INT},
    {3,   "StrVal",      "过流III段电流",    XML_DA_TYPE_VAL},
    {3,   "OpDlTmms",    "过流III段时间",    XML_DA_TYPE_INT},
    {1,   "StrAngLow",   "过流角度下限",     XML_DA_TYPE_VAL},
    {1,   "StrAngHigh",  "过流角度上限",     XML_DA_TYPE_VAL},
    {10,  "StrVal",      "过励磁电流",       XML_DA_TYPE_VAL},
    {10,  "OpDlTmms",    "过励磁时间",       XML_DA_TYPE_INT},
    {4,   "StrVal",      "过负荷电流",       XML_DA_TYPE_VAL},
    {4,   "OpDlTmms",    "过负荷时间",       XML_DA_TYPE_INT},
    {1,   "StrValZd",    "负荷遮断电流",     XML_DA_TYPE_VAL},
    {5,   "StrVal",      "过电压",           XML_DA_TYPE_VAL},
    {5,   "OpDlTmms",    "过电压时间",       XML_DA_TYPE_INT},
    {6,   "StrVal",      "低电压",           XML_DA_TYPE_VAL},
    {5,   "OpDlTmms",    "低电压时间",       XML_DA_TYPE_INT},
    {7,   "StrValU",     "零压",             XML_DA_TYPE_VAL},
    {7,   "StrVal",      "零流I段电流",      XML_DA_TYPE_VAL},
    {7,   "OpDlTmms",    "零流I段时间",      XML_DA_TYPE_INT},
    {8,   "StrVal",      "零流II段电流",     XML_DA_TYPE_VAL},
    {8,   "OpDlTmms",    "零流II段时间",     XML_DA_TYPE_INT},
    {9,   "StrVal",      "零流III段电流",    XML_DA_TYPE_VAL},
    {9,   "OpDlTmms",    "零流III段时间",    XML_DA_TYPE_INT},
    {7,   "StrAngLow",   "零流角度下限",     XML_DA_TYPE_VAL},
    {7,   "StrAngHigh",  "零流角度上限",     XML_DA_TYPE_VAL},
    {12,  "RecNum",      "重合闸次数",       XML_DA_TYPE_INT},
    {12,  "Rec1Tmms",    "一次重合时间",     XML_DA_TYPE_INT},
    {12,  "Rec2Tmms",    "二次重合时间",     XML_DA_TYPE_INT},
    {12,  "Rec3Tmms",    "三次重合时间",     XML_DA_TYPE_INT},
    {255, "RecBsTmms",   "重合闭锁时间",     XML_DA_TYPE_INT},
    {255, "UVOpDlTmms",  "失压分闸时间",     XML_DA_TYPE_INT},
#ifdef INCLUDE_PR_U
    {15,  "OpDlTmmsX",   "电压合闸X时限",    XML_DA_TYPE_INT},
    {15,  "OpDlTmmsY",   "电压合闸Y时限",    XML_DA_TYPE_INT},
    {255, "UVBsDlTmms",  "分闸闭锁复归",     XML_DA_TYPE_INT},
    {17,  "U0OpDlTmms",  "零压分闸时间",     XML_DA_TYPE_INT},
    {18,  "StrDiffU",    "两侧压差",         XML_DA_TYPE_VAL},
#endif
    {6,   "RteUBlkVal",  "电压滑差",         XML_DA_TYPE_VAL},
    {19,  "BlkValRteHz", "频率滑差",         XML_DA_TYPE_VAL},
    {19,  "StrVal",      "低频定值",         XML_DA_TYPE_VAL},
    {19,  "OpDlTmms",    "低频时间",         XML_DA_TYPE_INT},
    {19,  "BlkVal",      "低频闭锁电压",     XML_DA_TYPE_VAL},
    {19,  "BlkValA",     "低频闭锁电流",     XML_DA_TYPE_VAL},
    {12,  "SynBlkDifAng","同期角差",         XML_DA_TYPE_VAL},
    {20,  "RecDifTmms",  "合闸导前时间",     XML_DA_TYPE_INT},
    {20,  "RecDifU",     "准同期电压差",     XML_DA_TYPE_VAL},
    {20,  "RecDifF",     "准同期频率差",     XML_DA_TYPE_VAL},
    {20,  "RecDifFS",    "准同期加速度频差", XML_DA_TYPE_VAL},
};

#endif

const char sRelayName[] = "Relay";

TTABLETYPE tSetType;
VPrRunSet *prRunSet[2];
VPrSetPublic *prSetPublic[2];
VPrNbSet prNbSet;
BYTE *caSetBuf;
BYTE *prSet[MAX_FD_NUM*2];   //后续扩展多回线
BYTE prSetBuf[PR_SET_SIZE*(MAX_FD_NUM*2)];   //后续扩展多回线
extern VFdProtCal *fdProtVal;
extern VPrRunInfo *prRunInfo;
extern int g_prInit;
extern VPrRunPublic *prRunPublic;
//获取定值类型
BYTE GetSetOpt( TSETVAL *v)
{
  return ((v->bySetAttrib)&SETATTRIB_OPTMASK);
}

BYTE GetSetType( TSETVAL *v)
{
  return ((v->bySetAttrib)&SETATTRIB_TYPEMASK);
}

//压缩BCD测试
BOOL IsBcd(BYTE b)
{
  if(((b&0xf)<0x0a)&&((b&0xf0)<0xa0))return TRUE;
  return FALSE;
}

//检查是否BCD格式
BOOL IsBcdSet( TSETVAL *v)
{
  if(GetSetType(v) == SETTYPE_DEC) return FALSE;
  if(GetSetType(v) == SETTYPE_IP)  return FALSE;
  if(!IsBcd(v->byValLol))	return FALSE;
  if(!IsBcd(v->byValLoh))   return FALSE;
  if(!IsBcd(v->byValHil))	return FALSE;
  if(!IsBcd(v->byValHih))	return FALSE;
  return TRUE;
}
/*
DWORD FixHex(DWORD f)
{
  WORD t1,t2;
  t1=(WORD)(f>>16),t2=(WORD)f;
  return (((DWORD)BcdHex(t1)<<16)+FraHex(t2));
}*/
//将整定值原始值转换成浮点数
//bset[0]=定值属性
//bset[1]=定值数值的低字节
//bset[2]=定值数值的高字节
//hset=结果,其中高16位为整数部分,低16位为小数部分
BOOL SetToFloat( TSETVAL *bval,float *fval)
{
  BYTE attrib;
  if (!IsBcdSet(bval))	return FALSE;
  attrib=bval->byValAttrib;
  if(attrib & VALUEATTRIB_DOT2)
  {
    if(attrib & VALUEATTRIB_DOT1)
	{
	  //3位小数
	  *fval = (char)(bval->byValLol&0xf)*0.001+(char)(bval->byValLol>>4)*0.01;
	  *fval += (char)(bval->byValLoh&0xf)*0.1+(char)(bval->byValLoh>>4)*1.0;
      *fval += (char)(bval->byValHil&0xf)*10+(char)(bval->byValHil>>4)*100;
	  *fval += (char)(bval->byValHih&0xf)*1000+(char)(bval->byValHih>>4)*10000;
    }
    else
	{ 
	  //2位小数
      *fval = (char)(bval->byValLol&0xf)*0.01+(char)(bval->byValLol>>4)*0.1;
	  *fval += (char)(bval->byValLoh&0xf)+(char)(bval->byValLoh>>4)*10;
      *fval += (char)(bval->byValHil&0xf)*100+(char)(bval->byValHil>>4)*1000;
	  *fval += (char)(bval->byValHih&0xf)*10000+(char)(bval->byValHih>>4)*100000;
    }
  }
  else
  {
    if(attrib & VALUEATTRIB_DOT1)
	{
	  //1位小数
      *fval = (char)(bval->byValLol&0xf)*0.1+(char)(bval->byValLol>>4)*1.0;
	  *fval += (char)(bval->byValLoh&0xf)*10+(char)(bval->byValLoh>>4)*100;
      *fval += (char)(bval->byValHil&0xf)*1000+(char)(bval->byValHil>>4)*10000;
	  *fval += (char)(bval->byValHih&0xf)*100000+(char)(bval->byValHih>>4)*1000000;
    }
    else
	{ 
	  //无小数
      *fval = (char)(bval->byValLol&0xf)*1.0+(char)(bval->byValLol>>4)*10;
	  *fval += (char)(bval->byValLoh&0xf)*100+(char)(bval->byValLoh>>4)*1000;
      *fval += (char)(bval->byValHil&0xf)*10000+(char)(bval->byValHil>>4)*100000;
	  *fval += (char)(bval->byValHih&0xf)*1000000+(char)(bval->byValHih>>4)*10000000;
    }
  }
  if(attrib & VALUEATTRIB_SIG)	
  {
	  *fval =-*fval;
  }
  return TRUE;
}

//默认两位小数
BOOL FloatToSet(float fval, TSETVAL *bval)
{
   BYTE val,index;
   BYTE *pbval = &(bval->byValLol);
   float fvalue = fval*1000;
   DWORD dvalue;
   int i;

   val = bval->bySetAttrib;
   memset((BYTE*)bval, 0, sizeof(TSETVAL));
   bval->bySetAttrib = val;

   if(fvalue < 0)
   {
      bval->byValAttrib |= VALUEATTRIB_SIG;
	  fvalue = -fvalue;
   }
   
   dvalue = fvalue;
   index = 0;
   bval->byValAttrib |= VALUEATTRIB_DOT2;
   for(i=0; i<9; i++)
   {
       val = dvalue%10;
	   if((i == 0) && (val == 0))
	   {
	      dvalue = dvalue/10;
		  continue;
	   }
	   else if(i == 0)
	   	  bval->byValAttrib |= VALUEATTRIB_DOT1;
	   *pbval |= val<<(4*(index%2));
	   index++;
	   if((index%2) == 0)
	   	  pbval++;
	   dvalue = dvalue/10;
	   if((dvalue == 0)||(index >= 8))
	   	  break;
   }
   
   return true;
}


//将定值转换成16进制,并检查定值的范围
BOOL FloatSetRange(int i, int flag, float *pFloat)
{
    float fMin,fMax;

    //整定值
    if(!SetToFloat(( TSETVAL *)(caSetBuf+i*sizeof(TSETVAL)),pFloat))
    {
	    return false;
    }
	if (flag == PR_SET_FD)
    {
       SetToFloat(( TSETVAL *)(&tSetTable[i].tMin),&fMin);
       SetToFloat(( TSETVAL *)(&tSetTable[i].tMax),&fMax);
	}
	else if (flag == PR_SET_PUB)
	{
       SetToFloat(( TSETVAL *)(&tSetPubTable[i].tMin),&fMin);
       SetToFloat(( TSETVAL *)(&tSetPubTable[i].tMax),&fMax);
	}
    if((*pFloat < fMin) || (*pFloat > fMax))
    {
       return false;
    }
    return true;
}

enum SETRET
{
   SET_OK       = 0,
   SET_TYPEERR  = 0x01,
   SET_VALUEERR = 0x02,
   SET_OTHERERR = 0x04
};
//转换压板
enum SETRET CvtYb(int i,DWORD *pYb, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  BYTE b1,b2;
  WORD w1,w2;
  enum SETRET ret = SET_OK;
  //检查定值序号
  if (i<0 || i>=SET_NUMBER)
  	ret = SET_OTHERERR;
  
  //检查定值类型
  pSet += i;
  if (GetSetType(pSet) != SETTYPE_YB)
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	 pDef = &(tSetTable[i].tDef);
  else
  	 pDef = &(tSetPubTable[i].tDef);

  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
     memcpy(pSet, pDef, sizeof(TSETVAL));
  
  b1=pSet->byValLol;
  b2=pSet->byValLoh;

  w1=((WORD)b2<<8)|b1;

  b1=pSet->byValHil;
  b2=pSet->byValHih;
  w2=((WORD)b2<<8)|b1;

  *pYb = w2<<16|w1;
 
  return ret;
}

enum SETRET CvtIp(int i,DWORD *pIp, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  BYTE b1,b2;
  WORD w1,w2;
  enum SETRET ret = SET_OK;
  //检查定值序号
  if (i<0 || i>=SET_NUMBER)
  	 ret = SET_OTHERERR;
  
  //检查定值类型
  pSet += i;
  if (GetSetType(pSet) != SETTYPE_IP)
     ret = SET_TYPEERR;

  if ((ret != SET_OK) ||(GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, &(tSetTable[i].tDef), sizeof(TSETVAL));

  b1=pSet->byValLol;
  b2=pSet->byValLoh;

  w1=((WORD)b2<<8)|b1;
  

  b1=pSet->byValHil;
  b2=pSet->byValHih;

  w2=((WORD)b2<<8)|b1;

  *pIp = w2<<16|w1;
 
  return ret;
}

//转换控制字
enum SETRET CvtK(int i,DWORD *pK, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  BYTE b1,b2;
  WORD w1,w2;
  enum SETRET ret = SET_OK;

  //检查定值序号
  if (i<0 || i>=SET_NUMBER)
  	 ret = SET_OTHERERR;
  
  //检查定值类型
  pSet += i;
  if((GetSetType(pSet)!= SETTYPE_KG)&&(GetSetType(pSet)!= SETTYPE_DEC))
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	pDef = &(tSetTable[i].tDef);
  else
  	pDef = &(tSetPubTable[i].tDef);
  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	 memcpy(pSet, pDef, sizeof(TSETVAL));

  b1=pSet->byValLol;
  b2=pSet->byValLoh;
  w1=((WORD)b2<<8)|b1;

  b1=pSet->byValHil;
  b2=pSet->byValHih;
  w2=((WORD)b2<<8)|b1;

  *pK=((DWORD)w2<<16)|w1;
 
  return ret;
}

//转换系数
enum SETRET CvtF(int i, DWORD *pF, DWORD dKF, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  float f;
  enum SETRET ret = SET_OK;
  
  //检查定值序号
  if (i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;
  
  //检查定值类型
  pSet += i;
  if ((GetSetType(pSet) != SETTYPE_FACTOR) && (GetSetType(pSet)!= SETTYPE_HZ))
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	pDef = &(tSetTable[i].tDef);
  else
  	pDef = &(tSetPubTable[i].tDef);
  
  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, pDef, sizeof(TSETVAL));

  //转换定值
  if (!FloatSetRange(i,flag, &f))
  {
     SetToFloat(pDef, &f);
	 memcpy(pSet, pDef, sizeof(TSETVAL));
	 *pF= f*dKF;
	 ret = SET_VALUEERR;
     return ret;
  }
  *pF= f*dKF;
  return ret;
}

enum SETRET CvtHz(int i, DWORD *pHz, DWORD dKHz, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  float f;
  enum SETRET ret = SET_OK;
  
  //检查定值序号
  if (i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;
  
  //检查定值类型
  pSet += i;
  if ((GetSetType(pSet) != SETTYPE_HZ)&&(GetSetType(pSet)!= SETTYPE_HZPERS))
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	pDef = &(tSetTable[i].tDef);
  else
  	pDef = &(tSetPubTable[i].tDef);
  
  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, pDef, sizeof(TSETVAL));

  //转换定值
  if (!FloatSetRange(i, flag, &f))
  {
     SetToFloat(pDef, &f);
	 memcpy(pSet, pDef, sizeof(TSETVAL));
	 *pHz= f*dKHz;
	 ret = SET_VALUEERR;
     return ret;
  }
  *pHz= f*dKHz;
  return ret;
}

//转换电流定值
//*pKI: 增益，iKI:ct/in*ijz=ct(ijz=in)
enum SETRET CvtI(int i,DWORD *pI,long *pKI,int iKI, int num, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  float f;
  int j;
  enum SETRET ret = SET_OK;
 
  //检查定值序号
  if (i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;
  
  //检查定值类型
  pSet += i;
  if (GetSetType(pSet) != SETTYPE_A)
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	pDef = &(tSetTable[i].tDef);
  else
  	pDef = &(tSetPubTable[i].tDef);
  
  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, pDef, sizeof(TSETVAL));
 
  //转换定值
  if (!FloatSetRange(i, flag, &f))
  {
    SetToFloat(pDef, &f);
	memcpy(pSet, pDef, sizeof(TSETVAL));
	for (j=0; j<num; j++,pI++,pKI++)
	  *pI= f*(*pKI)/iKI;
	ret = SET_VALUEERR;
    return ret;
  }
  for (j=0; j<num; j++,pI++,pKI++)
	 *pI= f*(*pKI)/iKI;
  return ret;
}

//转换电压定值
//*pKU: 增益，iKU:pt/un*ujz(ujz=100)
enum SETRET CvtU(int i,DWORD *pU,long *pKU,int iKU, int num, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  float f;
  int k;
  enum SETRET ret = SET_OK;
 
  //检查定值序号
  if (i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;

  //检查定值类型
  pSet += i;
  if ((GetSetType(pSet) != SETTYPE_U) && (GetSetType(pSet)!= SETTYPE_UPERS))
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	 pDef = &(tSetTable[i].tDef);
  else
  	 pDef = &(tSetPubTable[i].tDef);
  if((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, pDef, sizeof(TSETVAL));
 
  //转换定值
  if (!FloatSetRange(i, flag, &f))
  {
     SetToFloat(pDef, &f);
	 memcpy(pSet, pDef, sizeof(TSETVAL));
	 for (k=0; k<num; k++,pU++,pKU++)
	    *pU= f*(*pKU)/iKU;
     ret = SET_VALUEERR;
	 return ret;
  }
  for (k=0; k<num; k++,pU++,pKU++)
	  *pU= f*(*pKU)/iKU;
  return ret;
}

//转换时间定值
enum SETRET CvtT(int i,DWORD *pT,DWORD dKT, BOOL zero, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  float f;
  enum SETRET ret = SET_OK;

  //检查定值序号
  if (i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;
  
  //检查定值类型
  pSet += i;
  if (GetSetType(pSet) != SETTYPE_S)
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	 pDef = &(tSetTable[i].tDef);
  else
  	 pDef = &(tSetPubTable[i].tDef);
  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, pDef, sizeof(TSETVAL));

  //转换定值
  if (!FloatSetRange(i, flag, &f))
  {
      SetToFloat(pDef, &f);
	  memcpy(pSet, pDef, sizeof(TSETVAL));
	  *pT= f*dKT;
	  if ((*pT < RELAY_DELAY_5MS) && !zero)
	  	*pT = RELAY_DELAY_5MS;
	  ret = SET_VALUEERR;
	  return ret;
  }
  *pT= f*dKT;
  if ((*pT < RELAY_DELAY_5MS) && !zero)
  	 *pT = RELAY_DELAY_5MS;
  return ret;
}

enum SETRET CvtAng(int i, long *pAng, DWORD dKAng, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  float f;
  enum SETRET ret = SET_OK;
 
  //检查定值序号
  if(i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;
  
  //检查定值类型
  pSet += i;
  if(GetSetType(pSet) != SETTYPE_DEGREE)
     ret = SET_TYPEERR;

  if((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4)))
  	memcpy(pSet, &(tSetTable[i].tDef), sizeof(TSETVAL));
 
  //转换定值
  if(!FloatSetRange(i,flag, &f))
  {
      SetToFloat(( TSETVAL *)(&tSetTable[i].tDef),&f);
	  memcpy(pSet, &(tSetTable[i].tDef), sizeof(TSETVAL));
	  *pAng= f*dKAng;
      ret = SET_VALUEERR;
	  return ret;
  }
  *pAng= f*dKAng;
  return ret;
}

	
enum SETRET ReverseKSet(int i, BYTE *pSet, DWORD value)
{
    TSETVAL *ptr = ((TSETVAL*)pSet)+i;
	
	//检查定值序号
    if (i<0 || i>=SET_NUMBER) return SET_OTHERERR;
  
  
    ptr->byValHih = HIBYTE(HIWORD(value));
	ptr->byValHil = LOBYTE(HIWORD(value));
	ptr->byValLoh = HIBYTE(LOWORD(value));
	ptr->byValLol = LOBYTE(LOWORD(value));

	return SET_OK;
}

VPrRunSet * prGetRunSet(int fd)
{
    VPrRunSet *pprRunSet;
   	VPrRunInfo *pInfo = prRunInfo+fd;

	pprRunSet = prRunSet[pInfo->set_no]+fd;

	return pprRunSet;
}

void prNbSetDef()
{
	prNbSet.fUxxMin  = 30.0;
	prNbSet.fUxx75   = 75.0;
	prNbSet.fIMin  = 0.05;
	prNbSet.fTLqd  = 0.2;
	prNbSet.fTPt   = 9;
	prNbSet.fTYy   = 30;
	prNbSet.fTBhfg = 3;
	prNbSet.fTBsfg = 2;
	prNbSet.fTIRet = 0;           
	prNbSet.fTI0Ret= 3;
	prNbSet.fTURet = 3;
	prNbSet.fTRet  = 120;
	prNbSet.fTJsIn = 120;
	prNbSet.fFreqMin = 45.0;

}

void prGetUpValRet(DWORD *pVal, DWORD *pValRet, DWORD num) 
{
    int i;
	for (i=0; i<num; i++)
       *(pValRet+i) = *(pVal+i)*94/100;
}

void prGetLowValRet(DWORD *pVal, DWORD *pValRet, DWORD num)
{
    int i;
	for (i=0; i<num; i++)
		*(pValRet+i) = *(pVal+i)*105/100;
}

void prCheckConflict(void)
{
    int iNo;
    VPrSetPublic*pprRunSet;
	//VPrRunSet *pRunSet;
	//VPrRunInfo *pInfo;

	if (prRunPublic->set_no == 0)
		iNo = 1;
	else
		iNo = 0;
	
    pprRunSet = prSetPublic[iNo];

	//pInfo  = prRunInfo + pprRunSet->bySLLine;
	//pRunSet = prRunSet[pInfo->set_no] + pprRunSet->bySLLine; //

	if ((pprRunSet->dYb&(PR_YB_SUDY|PR_YB_SUSY)) && (pprRunSet->dYb&PR_YB_UDIFF))
		pprRunSet->dYb &= ~PR_YB_UDIFF;
}

void prPubSetMode(VPrSetPublic* pprRunSet)
{
#if(DEV_SP != DEV_SP_DTU)  // 只有FTU才有此功能
	DWORD dKG1_OLD = 0;
	//模式暂时先删掉与回线定值有关的
    if (pprRunSet->byWorkMode == PR_MODE_SEG)
    {
    //    pprRunSet->dYb &= ~PR_YB_SUSY;
		//pRunSet->dYb &= PR_YB_I1|PR_YB_I2|PR_YB_I3|PR_YB_GZJC_EN;
    }
	else if (pprRunSet->byWorkMode == PR_MODE_TIE)
	{
	    pprRunSet->dYb &= ~PR_YB_SUDY;
		//pRunSet->dYb &= PR_YB_I1|PR_YB_I2|PR_YB_I3|PR_YB_GZJC_EN;
	}
	else if (pprRunSet->byWorkMode == PR_MODE_PR)
	{
	    pprRunSet->dYb = 0;
	}
	else if (pprRunSet->byWorkMode == PR_MODE_FJ)
	{
	    pprRunSet->dYb = 0;
		//pRunSet->dYb &= PR_YB_I1|PR_YB_I2|PR_YB_I3|PR_YB_GZJC_EN;
	}
	//根据工作模式再回写压板定值,目前只改了公共定值

	dKG1_OLD = pprRunSet->dKG1;
	if(pprRunSet->bFaWorkMode == PR_FA_U)  // 电压时间型
	{
		pprRunSet->dYb |= (PR_YB_SUDY | PR_YB_SLWY);
		pprRunSet->dYb &=  ~PR_YB_I_DL;
		pprRunSet->dKG1 |=  (PR_CFG1_X_BS | PR_CFG1_Y_BS |PR_CFG1_SY_BS);	
		pprRunSet->dKG1 &= ~(PR_CFG1_UI_TYPE |PR_CFG1_FTU_SD |PR_CFG1_ZSY_DXJD |PR_CFG1_ZSY_XJDL);
	}
	else if (pprRunSet->bFaWorkMode == PR_FA_I)  // 电压电流时间型
	{
		pprRunSet->dYb |= (PR_YB_SUDY |PR_YB_I_DL);
		pprRunSet->dYb &=  ~PR_YB_SLWY;
		pprRunSet->dKG1 |=  (PR_CFG1_X_BS |PR_CFG1_Y_BS |PR_CFG1_SY_BS | PR_CFG1_UI_TYPE |PR_CFG1_FZ_BS);	
		pprRunSet->dKG1 &= ~( PR_CFG1_ZSY_DXJD |PR_CFG1_ZSY_XJDL);
	}
	else if(pprRunSet->bFaWorkMode == PR_FA_ZSY) // 自适应型
	{
		pprRunSet->dYb |= (PR_YB_SUDY | PR_YB_SLWY);
		pprRunSet->dYb &=  ~PR_YB_I_DL;
		pprRunSet->dKG1 |=  (PR_CFG1_Y_BS |PR_CFG1_SY_BS |PR_CFG1_ZSY_DXJD |PR_CFG1_ZSY_XJDL);	
		pprRunSet->dKG1 &=~ (PR_CFG1_UI_TYPE);
	}
		
	ReverseKSet(SET_PUB_YB,caSetBuf,pprRunSet->dYb);
	if(pprRunSet->dKG1 != dKG1_OLD)
	{
		ReverseKSet(SET_PUB_KG1,caSetBuf,pprRunSet->dKG1);
		if (pprRunSet->dKG1&PR_CFG1_X_BS)        pprRunSet->bXBS      = true; else pprRunSet->bXBS      = false;
		if (pprRunSet->dKG1&PR_CFG1_Y_BS)        pprRunSet->bYBS      = true; else pprRunSet->bYBS      = false;
		if (pprRunSet->dKG1&PR_CFG1_DU_BS)       pprRunSet->bDUBS     = true; else pprRunSet->bDUBS     = false;
		if (pprRunSet->dKG1&PR_CFG1_SY_BS)       pprRunSet->bSYBS     = true; else pprRunSet->bSYBS     = false;
		if (pprRunSet->dKG1&PR_CFG1_UI_TYPE)     pprRunSet->bIProtect = true; else pprRunSet->bIProtect = false; 
		if (pprRunSet->dKG1&PR_CFG1_U0_TRIP)     pprRunSet->bU0Trip   = true; else pprRunSet->bU0Trip   = false;
		
#ifdef INCLUDE_PR_PRO	
		if (pprRunSet->dKG1&PR_CFG1_DFJL)        pprRunSet->bDfJl     = true; else pprRunSet->bDfJl     = false;
		if (pprRunSet->dKG1&PR_CFG1_DF_IBS)      pprRunSet->bDfIBs    = true; else pprRunSet->bDfIBs    = false; 
		pprRunSet->bTqMode = (pprRunSet->dKG1&PR_CFG1_CHTQ_MODE)>>KG_8;
		pprRunSet->bShTqMode = (pprRunSet->dKG1&PR_CFG1_SHTQ_MODE)>>KG_10;
		if (pprRunSet->dKG1&PR_CFG1_WY_ANY)      pprRunSet->bTqWyAny  = true; else pprRunSet->bTqWyAny  = false; 
		if (pprRunSet->dKG1&PR_CFG1_TQ_LINE)     pprRunSet->bTqLine   = true; else pprRunSet->bTqLine   = false;
#endif
	    if (pprRunSet->dKG1&PR_CFG1_JS_MODE)     pprRunSet->bJsMode   = true; else pprRunSet->bJsMode   = false;
		if (pprRunSet->dKG1&PR_CFG1_FTU_SD)     pprRunSet->bFTUSD = true; else pprRunSet->bFTUSD = false; 
		if (pprRunSet->dKG1&PR_CFG1_ZSY_XJDL)    pprRunSet->bXJDL =  true; else pprRunSet->bXJDL =  false;
		if (pprRunSet->dKG1&PR_CFG1_ZSY_DXJD)    pprRunSet->bDXJD =  true; else pprRunSet->bDXJD =  false;
		if (pprRunSet->dKG1&PR_CFG1_POWER_INDEX) pprRunSet->byPowerIndex = 1; else pprRunSet->byPowerIndex = 0; 
         if (pprRunSet->dKG1&PR_CFG1_FZ_BS) 	pprRunSet->bFZBS = true;else pprRunSet->bFZBS = false;      
	}
#endif
}

enum SETRET prPubSetConv(int fd)
{
    int  i, iNo;
    enum SETRET ret,ret1;
	DWORD dK, dKU;
#ifdef INCLUDE_PR_PRO
	DWORD dKI;
#endif
	VFdCfg *pfdCfg;
	VPrRunSet *pRunSet;
	VPrRunInfo *pInfo;
	VPrSetPublic* pprRunSet;

	if (prRunPublic[fd].set_no == 0)
		iNo = 1;
	else
		iNo = 0;
	
    pprRunSet = prSetPublic[iNo]+fd;
    memset(pprRunSet, 0, sizeof(VPrSetPublic));
	
	ret1 = SET_OK;
	
	if ((ret = CvtYb(SET_PUB_YB,&(pprRunSet->dYb), PR_SET_PUB)) != SET_OK)
		ret1 = ret;
	
    if ((ret = CvtK(SET_PUB_KG1,&(pprRunSet->dKG1), PR_SET_PUB)) != SET_OK)
		ret1 = ret;
	if (pprRunSet->dKG1&PR_CFG1_X_BS)        pprRunSet->bXBS      = true;
	if (pprRunSet->dKG1&PR_CFG1_Y_BS)        pprRunSet->bYBS      = true;
	if (pprRunSet->dKG1&PR_CFG1_DU_BS)       pprRunSet->bDUBS     = true;
	if (pprRunSet->dKG1&PR_CFG1_SY_BS)       pprRunSet->bSYBS     = true;
	if (pprRunSet->dKG1&PR_CFG1_UI_TYPE)     pprRunSet->bIProtect = true;
	if (pprRunSet->dKG1&PR_CFG1_U0_TRIP)     pprRunSet->bU0Trip   = true;
	
#ifdef INCLUDE_PR_PRO	
	if (pprRunSet->dKG1&PR_CFG1_DFJL)        pprRunSet->bDfJl     = true;
	if (pprRunSet->dKG1&PR_CFG1_DF_IBS)      pprRunSet->bDfIBs    = true;
	pprRunSet->bTqMode = (pprRunSet->dKG1&PR_CFG1_CHTQ_MODE)>>KG_8;
	pprRunSet->bShTqMode = (pprRunSet->dKG1&PR_CFG1_SHTQ_MODE)>>KG_10;
	if (pprRunSet->dKG1&PR_CFG1_WY_ANY)      pprRunSet->bTqWyAny  = true;
	if (pprRunSet->dKG1&PR_CFG1_TQ_LINE)     pprRunSet->bTqLine   = true;
#endif
    if (pprRunSet->dKG1&PR_CFG1_JS_MODE)     pprRunSet->bJsMode   = true;
	if (pprRunSet->dKG1&PR_CFG1_FTU_SD)     pprRunSet->bFTUSD = true;
	if (pprRunSet->dKG1&PR_CFG1_ZSY_XJDL)    pprRunSet->bXJDL =  true;
	if (pprRunSet->dKG1&PR_CFG1_ZSY_DXJD)    pprRunSet->bDXJD =  true;
	
	
	if (pprRunSet->dKG1&PR_CFG1_POWER_INDEX) pprRunSet->byPowerIndex = 1;
	if (pprRunSet->dKG1&PR_CFG1_FZ_BS) 	pprRunSet->bFZBS = true;

	if (pprRunSet->dKG1&PR_CFG1_BUS_OPT)     pprRunSet->bBusTrip = true;
		
	 if ((ret = CvtK(SET_PUB_KG2,&(pprRunSet->dKG2), PR_SET_PUB)) != SET_OK)
		ret1 = ret;
	
	pprRunSet->byWorkMode = (pprRunSet->dKG2&PR_CFG2_CONTROL_MODE);
	
	pprRunSet->bFaWorkMode = (pprRunSet->dKG2&PR_CFG2_FA_MODE) >> KG_3;

	//pprRunSet->bFaSelectMode = (pprRunSet->dKG2&PR_CFG2_FA_SELECT_MODE) >> KG_7;
	
#if ((TYPE_USER == USER_GUIYANG)&&(DEV_SP == DEV_SP_WDG))
	pprRunSet->bAuto = (pprRunSet->dKG2&PR_CFG2_AUTO_MODE) >> KG_7;
#endif	
	pprRunSet->bDlxbsMode = (pprRunSet->dKG2&PR_CFG2_DLXBS_MODE) >> KG_8;	
	pprRunSet->bFaTripMode = (pprRunSet->dKG2&PR_CFG2_FA_I_TRIP) >> KG_9;	
	pprRunSet->bSUSYMode =  (pprRunSet->dKG2&PR_CFG2_SUSY_TRIP) >> KG_10;	

#if (TYPE_USER == USER_BEIJING)
	if(pprRunSet->dKG2 & PR_CFG2_KG_MODE)	 pprRunSet->bKgMode = true;
#endif

    if ((ret = CvtK(SET_PUB_SLFD,&dK, PR_SET_PUB)) != SET_OK)
		ret1 = ret;
    //pprRunSet->bySLLine = dK - 1;
	pprRunSet->bySLLine = fd;
	if (pprRunSet->bySLLine >= fdNum) 
	{
	    pprRunSet->bySLLine = 0;
		ret1 = SET_VALUEERR;
	}
	ReverseKSet(SET_PUB_SLFD, caSetBuf, pprRunSet->bySLLine+1);//强制slline对应每个间隔
	
#ifdef INCLUDE_PR_PRO
    if ((ret = CvtK(SET_PUB_TQFD,&dK, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
    pprRunSet->byTqLine = dK - 1;

	if (pprRunSet->byTqLine >= fdNum) 
	{
	    pprRunSet->byTqLine = 0;
		ret1 = SET_VALUEERR;
	}
#endif

   if ((ret = CvtK(SET_PUB_ICNT,&(pprRunSet->dICnt), PR_SET_PUB)) != SET_OK)
		ret1 = ret;

   if ((ret = CvtK(SET_PUB_DYLJ,&(pprRunSet->dDYLJ), PR_SET_PUB)) != SET_OK)
		ret1 = ret;
   if ((ret = CvtT(SET_PUB_TDYLJ,&(pprRunSet->dTDYLJ),10000, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

   if ((ret = CvtK(SET_PUB_LXFZ,&(pprRunSet->dLXFZ), PR_SET_PUB)) != SET_OK)  //连续分闸次数  add by wdj
		ret1 = ret;
   if ((ret = CvtT(SET_PUB_TLXFZ,&(pprRunSet->dTLXFZ),10000, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

   if ((ret = CvtK(SET_PUB_RECORD_NUM,&(pprRunSet->dRecNum), PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	pprRunSet->dRecNum = pprRunSet->dRecNum/256*100 + (pprRunSet->dRecNum%256)/16*10 + pprRunSet->dRecNum%16;  // 16 进制转10进制,只转了三位

	//电压通道 ,不能越界
	if ((ret = CvtK(SET_PUB_AI_S,&(pprRunSet->dAi_S_No), PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	pprRunSet->dAi_S_No = pprRunSet->dAi_S_No/256*100 + (pprRunSet->dAi_S_No%256)/16*10 + pprRunSet->dAi_S_No%16;  // 16 进制转10进制,只转了三位
	if(pprRunSet->dAi_S_No > MAX_AI_NUM)
		pprRunSet->dAi_S_No = 0;
	
	if ((ret = CvtK(SET_PUB_AI_F,&(pprRunSet->dAi_F_No), PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	pprRunSet->dAi_F_No = pprRunSet->dAi_F_No/256*100 + (pprRunSet->dAi_F_No%256)/16*10 + pprRunSet->dAi_F_No%16;  // 16 进制转10进制,只转了三位
	if(pprRunSet->dAi_F_No > MAX_AI_NUM)
		pprRunSet->dAi_F_No = 0;
	
	dK = 10000;
	if ((ret = CvtT(SET_PUB_TX,&(pprRunSet->dTX),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TY,&(pprRunSet->dTY),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TFZ,&(pprRunSet->dTFZ2),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TSYCY,&(pprRunSet->dTSYCY),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TU0,&(pprRunSet->dTU0),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TWY,&(pprRunSet->dTWY),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TJD,&(pprRunSet->dTJD),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TJDRevs,&(pprRunSet->dTJDrevs),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TICNT1,&(pprRunSet->dTIDl1),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TICNT2,&(pprRunSet->dTIDl2),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	
	if ((ret = CvtT(SET_PUB_TC,&(pprRunSet->dTC),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TS,&(pprRunSet->dTS),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtT(SET_PUB_TXXCH,&(pprRunSet->dTXXCH),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtT(SET_PUB_TSYCY,&(pprRunSet->dTSY1),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	pprRunSet->dTSY2 = 2000; // 双压与瞬压的区别时间
	pprRunSet->dTFZ1 = 3000;

	pfdCfg = pFdCfg + pprRunSet->bySLLine;
	pInfo  = prRunInfo + pprRunSet->bySLLine;
	pRunSet = prRunSet[pInfo->set_no] + pprRunSet->bySLLine;

    for (i=0; i<3; i++)
    {
	    pprRunSet->dSLU[i] = pRunSet->dUYy[i];
		pprRunSet->dSLUMin[i] = pRunSet->dUWy[i];
    }
	
	if (tSetType.dExt == PROT_SET_1)
        dKU = pfdCfg->pt*100/pfdCfg->Un;
	else
		dKU = 100;
	if((ret = CvtU(SET_PUB_UDIFF,&(pprRunSet->dUDiff),&(pfdCfg->gain_u[0]), dKU, 1, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	if (tSetType.dExt == PROT_SET_1)
		dK = pfdCfg->ct;
	else
		dK = 5;
	if ((ret = CvtI(SET_PUB_IBUS, &pprRunSet->dIBus,&(pfdCfg->gain_i[0]), dK, 1, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	
	if(g_Sys.MyCfg.dwCfg & 0x10)
	{
		if((ret = CvtU(SET_PUB_U0,&(pprRunSet->dU0),&(pfdCfg->gain_u[3]), dKU/10, 1, PR_SET_PUB))!=SET_OK)
			ret1 = ret;
	}
	else
	{
		if((ret = CvtU(SET_PUB_U0,&(pprRunSet->dU0),&(pfdCfg->gain_u[3]), dKU, 1, PR_SET_PUB))!=SET_OK)
			ret1 = ret;
    }
	if((ret = CvtU(SET_PUB_UCY,pprRunSet->dUCY,&(pfdCfg->gain_u[0]), dKU, 3, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	dK = 65536;
	if ((ret = CvtAng(SET_PUB_UANG,&(pprRunSet->dUAng),dK, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	prPubSetMode(pprRunSet);
	
	if((pprRunSet->dYb&(PR_YB_SUDY|PR_YB_SUSY)) && (pprRunSet->dYb&PR_YB_UDIFF))
		pprRunSet->dYb &= ~PR_YB_UDIFF;

#ifdef INCLUDE_PR_PRO
	pfdCfg = pFdCfg + pprRunSet->byTqLine;
	pInfo  = prRunInfo + pprRunSet->byTqLine;
	pRunSet = prRunSet[pInfo->set_no] + pprRunSet->byTqLine;

	if (tSetType.dExt == PROT_SET_1)
		dKI = pfdCfg->ct;
	else
		dKI = 5;
	if (tSetType.dExt == PROT_SET_1)
        dKU = pfdCfg->pt*100/pfdCfg->Un;
	else
		dKU = 100;

	if ((ret = CvtHz(SET_PUB_FREQ,  (DWORD*)&(pprRunSet->dFreq), 100, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_PUB_IFREQ, pprRunSet->dIFreq, pfdCfg->gain_i, dKI, 3, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtU(SET_PUB_UFREQ, pprRunSet->dUFreq, pfdCfg->gain_u, dKU, 3, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TDF, &(pprRunSet->dTDf), 1000, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtHz(SET_PUB_FSLIP, (DWORD*)&(pprRunSet->dFSlip), 100, PR_SET_PUB))!= SET_OK)
		ret1 = ret;

	pprRunSet->dFreqMin = prNbSet.fFreqMin*100;

	pfdCfg = pFdCfg + pprRunSet->byTqLine;
	pInfo  = prRunInfo + pprRunSet->byTqLine;
	pRunSet = prRunSet[pInfo->set_no] + pprRunSet->byTqLine;

	if (tSetType.dExt == PROT_SET_1)
        dKU = pfdCfg->pt*100/pfdCfg->Un;
	else
		dKU = 100;

	if((ret = CvtHz(SET_PUB_TQFDIFF, (DWORD*)&(pprRunSet->dTqFDiff), 100, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	if((ret = CvtHz(SET_PUB_TQFSDIFF, (DWORD*)&(pprRunSet->dTqFsDiff), 100, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtU(SET_PUB_TQUDIFF, (DWORD*)&(pprRunSet->dTqUDiff), pfdCfg->gain_u, dKU, 1, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if((ret = CvtT(SET_PUB_TQTDQ, &(pprRunSet->dTqTAng), 1000, true, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if((ret = CvtAng(SET_PUB_TQDQJ, &(pprRunSet->dTqDqAngle), 65536, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	for (i=0; i<3; i++)
    {
	    pprRunSet->dTqU75[i] = pRunSet->dUxx75[i];
		pprRunSet->dTqUMin[i] = pRunSet->dUMin[i];
    }

	pprRunSet->dTTqTwj= 30000;
	pprRunSet->dTTqYk = 60000;

#endif

//	extNvRamSet(NVRAM_DEV_PUBYB,(BYTE*)&pprRunSet->dYb,sizeof(DWORD));
	if(0 == fd)
		extNvRamSet(NVRAM_DEV_PUBYB,(BYTE*)&pprRunSet->dYb,sizeof(DWORD));
	else
		extNvRamSet(NVRAm_DEV_PUBYB_MULTI+4*(fd-1),(BYTE*)&pprRunSet->dYb,sizeof(DWORD));
	
	pprRunSet->bSetChg = TRUE;

	return ret1;
}

enum SETRET prFdSetConv(int fd)
{
	int  iNo,i,ai_chn;
	DWORD dK;	
	VPrRunSet *pprRunSet;
	VFdCfg *pfdCfg = pFdCfg+fd;
	VPrRunInfo *pInfo = prRunInfo+fd;
	enum SETRET ret,ret1;
	BOOL bZero;

	if (pInfo->set_no == 0)
		iNo = 1;
	else
		iNo = 0;
	
	pprRunSet = prRunSet[iNo]+fd;
    memset(pprRunSet, 0, sizeof(VPrRunSet));

    ret1 = SET_OK;
    if ((ret = CvtYb(SET_YB, &(pprRunSet->dYb), PR_SET_FD)) != SET_OK)
		ret1 = ret;

	for (i=MMI_MEA_IA; i<MMI_MEA_NUM; i++)
	{
		ai_chn = pfdCfg->mmi_meaNo_ai[i];
        //多回线会共电压,因此某一回线有效即该通道有效
		if ((ai_chn>=0) && (pprRunSet->dYb&PR_YB_GZJC_EN))
			pAiCfg[ai_chn].protect_enable = 1;		
	}
 
	if ((ret = CvtK(SET_KG1, &(pprRunSet->dKG1), PR_SET_FD)) != SET_OK)
		ret1 = ret;


	if (!(pprRunSet->dKG1 & PR_CFG1_GFH_TRIP)) pprRunSet->bGfhGJ = true;
	if (pprRunSet->dKG1 & PR_CFG1_FSX_START)   pprRunSet->bGfhSX = true;
	pprRunSet->bFsx = pprRunSet->dKG1&PR_CFG1_FSX_CURV;

	if (pprRunSet->dKG1 & PR_CFG1_U_PT)        pprRunSet->bPTBS     = true;
	if (pprRunSet->dKG1 & PR_CFG1_I_DY)        pprRunSet->bIDYBS    = true;
	if (pprRunSet->dKG1 & PR_CFG1_I_DIR)       pprRunSet->bIDir     = true;
	if (pprRunSet->dKG1 & PR_CFG1_I_FX)        pprRunSet->bIDirMX  = true;
	if (pprRunSet->dKG1 & PR_CFG1_DY_CYY)      pprRunSet->bCYY      = true;
	if (pprRunSet->dKG1 & PR_CFG1_PT_A)        pprRunSet->bPTU[0]   = true;
	if (pprRunSet->dKG1 & PR_CFG1_PT_B)        pprRunSet->bPTU[1]   = true;
	if (pprRunSet->dKG1 & PR_CFG1_PT_C)        pprRunSet->bPTU[2]   = true;
	if (pprRunSet->dKG1 & PR_CFG1_I0_DIR)      pprRunSet->bI0Dir    = true;
	if (pprRunSet->dKG1 & PR_CFG1_I0_FX)       pprRunSet->bI0DirMX  = true;
	if (pprRunSet->dKG1 & PR_CFG1_I0_U )       pprRunSet->bI0U      = true;
	if (pprRunSet->dKG1 & PR_CFG1_CH_DDLBS)    pprRunSet->bDdlBsch  = true;
	if (pprRunSet->dKG1 & PR_CFG1_CD_MODE)     pprRunSet->bCd       = true;
	pprRunSet->bITrip = (pprRunSet->dKG1&PR_CFG1_TRIP_MODE)>>KG_17;
	pprRunSet->bIReport = (pprRunSet->dKG1&PR_CFG1_FAULT_REPORT)>>KG_20;
	if (pprRunSet->dKG1 & PR_CFG1_TEST_VOL)    pprRunSet->bTestVol  = true;
	if (pprRunSet->dKG1 & PR_CFG1_BHQD_QUIT)   pprRunSet->bQdQuit   = true;
	if (pprRunSet->dKG1 & PR_CFG1_FZD_BH)      pprRunSet->bfZdbh    = true;
	if (pprRunSet->dKG1 & PR_CFG1_HW2_ZD)      pprRunSet->bHw2Zd    = true;
	//if (pprRunSet->dKG1 & PR_CFG1_PR_TRIP)     pprRunSet->bPrTrip   = true;
	if(pprRunSet->dKG1 & PR_CFG1_LED_FG)     pprRunSet->bLedZdFg   = true;

	pprRunSet->bI0GJ = pprRunSet->bI1GJ = pprRunSet->bI2GJ = pprRunSet->bI3GJ = true;
	if (pprRunSet->dKG1 & PR_CFG1_I0_TRIP)    pprRunSet->bI0Trip   = true;
	if (pprRunSet->dKG1 & PR_CFG1_I1_TRIP)    pprRunSet->bI1Trip  = true;
	if (pprRunSet->dKG1 & PR_CFG1_I2_TRIP)    pprRunSet->bI2Trip  = true;


	if (pprRunSet->dKG1 & PR_CFG1_I2_DY)        pprRunSet->bI2DYBS    = true;
	if (pprRunSet->dKG1 & PR_CFG1_I2_DIR)       pprRunSet->bI2Dir     = true;


	if ((ret = CvtK(SET_KG2, &(pprRunSet->dKG2), PR_SET_FD)) != SET_OK)
		ret1 = ret;

	pprRunSet->bIYxDir = !((pprRunSet->dKG2&PR_CFG2_IYX_DIR)>>KG_2);
    pprRunSet->coef = ((pprRunSet->dKG2&PR_CFG2_COEF)>>KG_3) ? -1 : 1;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_GL)      pprRunSet->bRcdGl= true;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_SY)      pprRunSet->bRcdSy= true;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_U0)      pprRunSet->bRcdU0= true;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_I0)      pprRunSet->bRcdI0= true;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_I0TB)      pprRunSet->bRcdI0TB= true;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_GLBH)      pprRunSet->bRcdGlbh= true;
	if (pprRunSet->dKG2 & PR_CFG2_SWITCH_TRIP_F)      pprRunSet->bSwitchTrip= true; //add by lijun  2018-05-25  开关跳闸异常控制字
    
	if (pprRunSet->dKG2 & PR_CFG2_XDL_YB)      pprRunSet->bXDLYB = true;  
	pprRunSet->bXDLGJ= true;
	if (pprRunSet->dKG2 & PR_CFG2_XDL_TRIP)   pprRunSet->bXDLTrip= true;
	if (pprRunSet->dKG2 & PR_CFG2_XDL_JR)     pprRunSet->bJrU   = true;    else pprRunSet->bJrU0 = true;
  if (pprRunSet->dKG2 & PR_CFG2_XDL_DLFX)     pprRunSet->bXdlFx   = true;    else pprRunSet->bXdlFx = false;
	if (!(pprRunSet->dKG2 & PR_CFG2_GY_TRIP))     pprRunSet->bGyGJ   = true;else pprRunSet->bGyGJ = false; // 过压告警
	if (!(pprRunSet->dKG2 & PR_CFG2_DY_TRIP))     pprRunSet->bDyGJ   = true;else pprRunSet->bDyGJ = false; // 过压告警

	if (!(pprRunSet->dKG2 & PR_CFG2_GF_TRIP))     pprRunSet->bGpGJ   = true; else pprRunSet->bGpGJ   = false;//过频告警
	if (!(pprRunSet->dKG2 & PR_CFG2_DF_TRIP))     pprRunSet->bDpGJ   = true; else pprRunSet->bDpGJ   = false;//低频告警

	if (pprRunSet->dKG2 & PR_CFG2_I3_TRIP)    pprRunSet->bI3Trip  = true;
	if (!(pprRunSet->dKG2 & PR_CFG2_MXQY_TRIP))	  pprRunSet->bMXQYGJ = true;else pprRunSet->bMXQYGJ = false; // 母线欠压告警


	if (tSetType.dExt == PROT_SET_1)
		dK = pfdCfg->ct;
	else
		dK = 5;
	if ((ret = CvtI(SET_I1,pprRunSet->dI[0],pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dI[0], pprRunSet->dIRet[0], 3);
	if ((ret = CvtI(SET_I2,pprRunSet->dI[1],pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dI[1], pprRunSet->dIRet[1], 3);
	if ((ret = CvtI(SET_I3,pprRunSet->dI[2],pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dI[2], pprRunSet->dIRet[2], 3);
	if ((ret = CvtI(SET_IM, pprRunSet->dIM, pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_IGFH, pprRunSet->dIGfh, pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dIGfh, pprRunSet->dIGfhRet, 3);
	if ((ret = CvtI(SET_IKG,  pprRunSet->dIKG, pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_ITB,  &pprRunSet->dIInK, pfdCfg->gain_i, dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_CHBSDL,&(pprRunSet->dCHBSDL),pfdCfg->gain_i, dK,1, PR_SET_FD))!=SET_OK)     // 重合闭锁电流
		ret1 = ret;

    if (tSetType.dExt == PROT_SET_1)
		dK = pfdCfg->ct0;
	else
		dK = pfdCfg->In0;
	if ((ret = CvtI(SET_IGFH,&(pprRunSet->dIGfh[3]), &(pfdCfg->gain_i0), dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_3I01,&(pprRunSet->dI0[0]),&(pfdCfg->gain_i0), dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_3I02,&(pprRunSet->dI0[1]),&(pfdCfg->gain_i0), dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_3I03,&(pprRunSet->dI0[2]),&(pfdCfg->gain_i0), dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
    prGetUpValRet(pprRunSet->dI0, pprRunSet->dI0Ret, 3);
		
	if ((ret = CvtI(SET_I0TB,&pprRunSet->dII0nK,&(pfdCfg->gain_i0), dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	
	if (tSetType.dExt == PROT_SET_1)
        dK = pfdCfg->pt*100/pfdCfg->Un;
	else
		dK = 100;
	if ((ret = CvtU(SET_UGY, pprRunSet->dUGy, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dUGy, pprRunSet->dUGyRet, 3);
	if ((ret = CvtU(SET_UDY, pprRunSet->dUDy, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetLowValRet(pprRunSet->dUDy, pprRunSet->dUDyRet, 3);
	if ((ret = CvtU(SET_USLIP, pprRunSet->dUSlip, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtU(SET_UYY, pprRunSet->dUYy, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtU(SET_UWY, pprRunSet->dUWy, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;

	if ((ret = CvtU(SET_UUMX, pprRunSet->dUUMX, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dUUMX, pprRunSet->dUUMXRet, 3);
	if ((ret = CvtU(SET_UDMX, pprRunSet->dUDMX, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetLowValRet(pprRunSet->dUDMX, pprRunSet->dUDMXRet, 3);
	
    if(g_Sys.MyCfg.dwCfg & 0x10)
	{
		if ((ret = CvtU(SET_3U0,&(pprRunSet->dU0),&(pfdCfg->gain_u0), dK/10, 1, PR_SET_FD))!= SET_OK)
			ret1 = ret;
		if ((ret = CvtU(SET_U0TB,&(pprRunSet->dIUnK),&(pfdCfg->gain_u0), dK/10, 1, PR_SET_FD))!= SET_OK)
			ret1 = ret;
    }
	else
	{
		if ((ret = CvtU(SET_3U0,&(pprRunSet->dU0),&(pfdCfg->gain_u0), dK, 1, PR_SET_FD))!= SET_OK)
			ret1 = ret;
		if ((ret = CvtU(SET_U0TB,&(pprRunSet->dIUnK),&(pfdCfg->gain_u0), dK, 1, PR_SET_FD))!= SET_OK)
			ret1 = ret;
	}
	
	prGetUpValRet(&pprRunSet->dU0, &pprRunSet->dU0Ret, 1);
		
	if ((ret = CvtF(SET_CHNUM, (DWORD*)&(pprRunSet->bCHNum), 1, PR_SET_FD))!=SET_OK)
		ret1 = ret;
    if ((ret = CvtF(SET_YXTZ, (DWORD*)&(pprRunSet->wYxNum), 1, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	dK = 10000;
	bZero = false;
	if ((ret = CvtT(SET_T1,&(pprRunSet->dT[0]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_T2,&(pprRunSet->dT[1]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_T3,&(pprRunSet->dT[2]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TDJQD,&(pprRunSet->dTDjqd),dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TGFH,&(pprRunSet->dTGfh), dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TGY,&(pprRunSet->dTGy),dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TDY,&(pprRunSet->dTDy),dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TI01,&(pprRunSet->dT0[0]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TI02,&(pprRunSet->dT0[1]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TI03,&(pprRunSet->dT0[2]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TCH1,&(pprRunSet->dTCH1),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TCH2,&(pprRunSet->dTCH2),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TCH3,&(pprRunSet->dTCH3),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TCHBS,&(pprRunSet->dTCHBS),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TKEEP,&(pprRunSet->dTRet),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_LED_TKEEP,&(pprRunSet->dLedTret),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtT(SET_TJS,&(pprRunSet->dTJS),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TJSFG,&(pprRunSet->dTJSFG),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtT(SET_TDXJD,&(pprRunSet->dTDXJD),dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_XDLCOEF,&(pprRunSet->wXDLCoef),dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtHz(SET_FGP,&(pprRunSet->dGFreq),100,PR_SET_FD)) != SET_OK)  // 过频定值
		ret1 = ret;
	if ((ret = CvtT(SET_TGP,&(pprRunSet->dTGFreq),dK, false, PR_SET_FD))!=SET_OK) //过频时间
		ret1 = ret;
	
	if ((ret = CvtHz(SET_FDP,&(pprRunSet->dDFreq),100,PR_SET_FD)) != SET_OK)  // 低频定值
		ret1 = ret;
	if ((ret = CvtT(SET_TDP,&(pprRunSet->dTDFreq),dK, false, PR_SET_FD))!=SET_OK) //低频时间
		ret1 = ret;
	
	if ((ret = CvtT(SET_I0TJS,&(pprRunSet->dI0TJS),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtT(SET_TUMX,&(pprRunSet->dTUMX),dK, true, PR_SET_FD))!=SET_OK)//母线过电压时间
			ret1 = ret;
	if ((ret = CvtT(SET_TDMX,&(pprRunSet->dTDMX),dK, true, PR_SET_FD))!=SET_OK)//母线低电压时间
			ret1 = ret;


	pprRunSet->dLedTret = pprRunSet->dLedTret;

	dK = 65536;
	if ((ret = CvtAng(SET_IANG1,&(pprRunSet->lPang1),dK, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtAng(SET_IANG2,&(pprRunSet->lPang2),dK, PR_SET_FD))!=SET_OK)
		ret1 = ret;

	 if (pprRunSet->bIDirMX)
    {
       pprRunSet->lPang1 += 180<<16;
	   pprRunSet->lPang2 += 180<<16;
	   if(pprRunSet->lPang1 > (360<<16))
	   	pprRunSet->lPang1 -= 360<<16;
	   if(pprRunSet->lPang2 > (360<<16))
	   	pprRunSet->lPang2 -= 360<<16;
    }

	if (pprRunSet->lPang1 > pprRunSet->lPang2)
		pprRunSet->lPang1 -= 360<<16;

    if ((ret = CvtAng(SET_I0ANG1,&(pprRunSet->lNang1),dK, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtAng(SET_I0ANG2,&(pprRunSet->lNang2),dK, PR_SET_FD))!=SET_OK)
		ret1 = ret;

    if (pprRunSet->bI0DirMX)
    {
       pprRunSet->lNang1 += 180<<16;
	   pprRunSet->lNang2 += 180<<16;
	   if(pprRunSet->lNang1 > (360<<16))
	   	pprRunSet->lNang1 -= 360<<16;
	   if(pprRunSet->lNang2 > (360<<16))
	   	pprRunSet->lNang2 -= 360<<16;
    }
	if (pprRunSet->lNang1 > pprRunSet->lNang2)
	    pprRunSet->lNang1 -= 360<<16;

	pprRunSet->dU0_8V = 8.0*pfdCfg->gain_u0/100;
	for (i=0; i<3; i++)
	{
	  pprRunSet->dU_16V[i] = 16.0*pfdCfg->gain_u[i]/100;
	  pprRunSet->dI_1A[i]  = 1.0*pfdCfg->gain_i[i]/5;
	  pprRunSet->dUxxMin[i]= 30*pfdCfg->gain_u[i]/100;
	  pprRunSet->dUxx75[i] = 75*pfdCfg->gain_u[i]/100;
	  pprRunSet->dIMin[i]  = prNbSet.fIMin*pfdCfg->gain_i[i]/5;
	  pprRunSet->dI1_old[i]= pprRunSet->dI[0][i];

	  if (pfdCfg->cfg & FD_CFG_PT_58V)
	  {
	     pprRunSet->dUMin[i] = pprRunSet->dUxxMin[i]*1000/1732;
		 pprRunSet->dU75[i]  = pprRunSet->dUxx75[i]*1000/1732;
	  }
	  else
	  {
	     pprRunSet->dUMin[i] = pprRunSet->dUxxMin[i];
		 pprRunSet->dU75[i] = pprRunSet->dUxx75[i];
	  }
	}
	pprRunSet->dUMin[3] = prNbSet.fUxxMin/2*pfdCfg->gain_u0/100;
	pprRunSet->dIMin[3] = prNbSet.fIMin/2*pfdCfg->gain_i0/pfdCfg->In0;
	pprRunSet->dTLqd  = prNbSet.fTLqd*10000;
	pprRunSet->dTPt   = prNbSet.fTPt*10000;
	pprRunSet->dTBhfg = prNbSet.fTBhfg*10000;
	pprRunSet->dTBsfg = prNbSet.fTBsfg*10000;
	pprRunSet->dTYy   = prNbSet.fTYy*10000;
	pprRunSet->dTIRet = prNbSet.fTIRet*10000;
	pprRunSet->dTI0Ret= prNbSet.fTI0Ret*10000;
	pprRunSet->dTURet = prNbSet.fTURet*10000;
	pprRunSet->dTJSIN = prNbSet.fTJsIn*10000;
	pprRunSet->dTCHBAY= 90000;
	pprRunSet->dTWY   = 1000;
	
	if (tSetType.dExt == PROT_SET_1)
		dK = pfdCfg->ct;
	else
		dK = 5;

	if ((ret = CvtI(SET_IHJS,pprRunSet->dI[3],pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_I0HJS,&(pprRunSet->dI0[3]),&pfdCfg->gain_i0, dK, 1, PR_SET_FD))!= SET_OK)//modify by wdj 3->1
		ret1 = ret;	

    if ((pprRunSet->dYb&PR_YB_I3) && (pprRunSet->dI[3][0] > pprRunSet->dI[2][0]))
	   	memcpy(pprRunSet->dI[3], pprRunSet->dI[2], 12);
	else if ((pprRunSet->dYb&PR_YB_I2) && (pprRunSet->dI[3][0] > pprRunSet->dI[1][0]))
	    memcpy(pprRunSet->dI[3], pprRunSet->dI[1], 12);
	else if (pprRunSet->dI[3][0] > pprRunSet->dI[0][0])
	    memcpy(pprRunSet->dI[3], pprRunSet->dI[0], 12);
	
	if ((pprRunSet->dYb&PR_YB_I03) && (pprRunSet->dI0[3] > pprRunSet->dI0[2]))
		pprRunSet->dI0[3] = pprRunSet->dI0[2];
	else if ((pprRunSet->dYb&PR_YB_I02)  && (pprRunSet->dI0[3] > pprRunSet->dI0[1]))
		pprRunSet->dI0[3] = pprRunSet->dI0[1];
	else if (pprRunSet->dI0[3] > pprRunSet->dI0[0])
		pprRunSet->dI0[3] = pprRunSet->dI0[0];
	
	pprRunSet->bSetChg = TRUE;
	return ret1;
}

int prFdSetDef()
{	
    int i;
    TSETVAL *pb=(TSETVAL *)caSetBuf;
    memset(caSetBuf, 0, SET_NUMBER*sizeof(TSETVAL));
	
	for(i=0; i<SET_NUMBER; i++)
	{
		*pb = tSetTable[i].tDef;
		pb++;
	}
	return OK;
}

int prPubSetDef()
{	
    int i;
    TSETVAL *pb=(TSETVAL *)caSetBuf;
    memset(caSetBuf, 0, SET_PUB_NUMBER*sizeof(TSETVAL));
	
	for(i=0; i<SET_PUB_NUMBER; i++)
	{
		*pb = tSetPubTable[i].tDef;
		pb++;
	}
	return OK;
}

int prFdSetPartDef(BYTE begin)
{	
    int i;
    TSETVAL *pb=(TSETVAL *)(caSetBuf+ begin*sizeof(TSETVAL));
	if(begin >= SET_NUMBER)
		return ERROR;
	
	for(i = begin; i<SET_NUMBER; i++)
	{
		*pb = tSetTable[i].tDef;
		pb++;
	}
	return OK;
}

int prPubSetPartDef(BYTE begin)
{	
    int i;
    TSETVAL *pb=(TSETVAL *)(caSetBuf+ begin*sizeof(TSETVAL));
	if(begin >= SET_PUB_NUMBER)
		return ERROR;
	
	for(i=begin; i<SET_PUB_NUMBER; i++)
	{
		*pb = tSetPubTable[i].tDef;
		pb++;
	}
	return OK;
}


void prFillTableType()
{
    strcpy(tSetType.szName, sRelayName);
    tSetType.dType = DEV_SP;
	tSetType.dExt  = PROT_TYPE;
}

BOOL prSetTypeJudgy(TTABLETYPE *pType)
{
   if(strcmp(pType->szName, tSetType.szName))
   	  return false;
 //  if(pType->dType != tSetType.dType)
 //  	  return false;
   if(pType->dExt != tSetType.dExt)
   	  return false;
   return true;
}

int prSetTableRead()
{
	char fname[MAXFILENAME];
    struct VFileHead *head;
    TTABLETYPE *pTag;


    sprintf(fname, "ProSetTable.cfg");
	if (ReadParaFile(fname, prTableBuf, sizeof(prTableBuf)) == ERROR)
		return ERROR;

	head = (struct VFileHead *)prTableBuf;	
    pTag = (TTABLETYPE*)(head+1);

	if(prSetTypeJudgy(pTag))
	   return OK;
	else
		return ERROR;

}

int prSetTableWrite()
{
	char fname[MAXFILENAME];
	BYTE *pTemp;
	BYTE kg_num;
	BYTE i,j;
    struct VFileHead *head;

    pTemp = prTableBuf;
	
	head  = (struct VFileHead *)prTableBuf;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	pTemp += sizeof(struct VFileHead);
	
    memcpy(pTemp, (BYTE*)&tSetType, sizeof(TTABLETYPE));
	pTemp += sizeof(TTABLETYPE);
	memcpy(pTemp, (BYTE*)&YB_NUMBER, 1);
	pTemp += 1;
	memcpy(pTemp, (BYTE*)tYbTable, sizeof(tYbTable));
	pTemp += sizeof(tYbTable);

	kg_num = KG1_NUMBER;
	memcpy(pTemp, (BYTE*)&kg_num, 1);
	pTemp += 1;

	for(i=0; i<KG1_NUMBER; i++)
	{
	   memcpy(pTemp, (BYTE*)(tKG1Table+i), sizeof(TKGTABLE)-4);
	   pTemp += sizeof(TKGTABLE)-4;
	   for(j=0; j<tKG1Table[i].byMsNum; j++)
	   {
	      memcpy(pTemp, (BYTE*)(tKG1Table[i].szBit[j]), (4*2+1));
	      pTemp += (4*2+1);
	   }
	}

	kg_num = KG2_NUMBER;
	memcpy(pTemp, (BYTE*)&kg_num, 1);
	pTemp += 1;
	for(i=0; i<KG2_NUMBER; i++)
	{
	   memcpy(pTemp, (BYTE*)(tKG2Table+i), sizeof(TKGTABLE)-4);
	   pTemp += sizeof(TKGTABLE)-4;
	   for(j=0; j<tKG2Table[i].byMsNum; j++)
	   {
	      memcpy(pTemp, (BYTE*)(tKG2Table[i].szBit[j]), (4*2+1));
	      pTemp += (4*2+1);
	   }
	}

	memcpy(pTemp, (BYTE*)&SET_NUMBER, 1);
	pTemp += 1;
	memcpy(pTemp, (BYTE*)tSetTable, sizeof(tSetTable));
	pTemp += sizeof(tSetTable);
	
	head->nLength = pTemp - prTableBuf;
	sprintf(fname, "ProSetTable.cfg");
	if(WriteParaFile(fname, (struct VFileHead *)prTableBuf) == ERROR)
		return ERROR;
	
	return OK;
}

int prPubSetTableWrite()
{
	char fname[MAXFILENAME];
	BYTE *pTemp;
	BYTE kg_num;
	BYTE i,j;
		struct VFileHead *head;

	pTemp = prTableBuf;
	
	head  = (struct VFileHead *)prTableBuf;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	pTemp += sizeof(struct VFileHead);
	
    memcpy(pTemp, (BYTE*)&tSetType, sizeof(TTABLETYPE));
	pTemp += sizeof(TTABLETYPE);
	memcpy(pTemp, (BYTE*)&YB_PUB_NUMBER, 1);
	pTemp += 1;
	memcpy(pTemp, (BYTE*)tYbPubTable, sizeof(tYbPubTable));
	pTemp += sizeof(tYbPubTable);

	kg_num = KG1_PUB_NUMBER;
	memcpy(pTemp, (BYTE*)&kg_num, 1);
	pTemp += 1;

	for(i=0; i<KG1_PUB_NUMBER; i++)
	{
	   memcpy(pTemp, (BYTE*)(tKG1PubTable+i), sizeof(TKGTABLE)-4);
	   pTemp += sizeof(TKGTABLE)-4;
	   for(j=0; j<tKG1PubTable[i].byMsNum; j++)
	   {
	      memcpy(pTemp, (BYTE*)(tKG1PubTable[i].szBit[j]), (4*2+1));
	      pTemp += (4*2+1);
	   }
	}

	kg_num = KG2_PUB_NUMBER;
	memcpy(pTemp, (BYTE*)&kg_num, 1);
	pTemp += 1;
	for(i=0; i<KG2_PUB_NUMBER; i++)
	{
	   memcpy(pTemp, (BYTE*)(tKG2PubTable+i), sizeof(TKGTABLE)-4);
	   pTemp += sizeof(TKGTABLE)-4;
	   for(j=0; j<tKG2PubTable[i].byMsNum; j++)
	   {
	      memcpy(pTemp, (BYTE*)(tKG2PubTable[i].szBit[j]), (4*2+1));
	      pTemp += (4*2+1);
	   }
	}

	memcpy(pTemp, (BYTE*)&SET_PUB_NUMBER, 1);
	pTemp += 1;
	memcpy(pTemp, (BYTE*)tSetPubTable, sizeof(tSetPubTable));
	pTemp += sizeof(tSetPubTable);

		
	head->nLength = pTemp - prTableBuf;
	sprintf(fname, "ProSetPubTable.cfg");
	if(WriteParaFile(fname, (struct VFileHead *)prTableBuf) == ERROR)
		return ERROR;

	return OK;  
}

#ifdef INCLUDE_61850
int prSetXmlWrite()
{
    char fname[MAXFILENAME];
	BYTE *pTemp;
	BYTE i,j;
	int len;
    struct VFileHead *head;
	TYBXMLTABLE * pSet = gYbXmlTable;

    pTemp = prTableBuf;
	
	head  = (struct VFileHead *)prTableBuf;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	pTemp += sizeof(struct VFileHead);
	
    memcpy(pTemp, (BYTE*)&tSetType, sizeof(TTABLETYPE));
	pTemp += sizeof(TTABLETYPE);

	memcpy(pTemp, (BYTE*)&YB_NUMBER, 1);
	pTemp += 1;
	len = sizeof(gYbXmlTable);
	memcpy(pTemp, (BYTE*)gYbXmlTable, sizeof(gYbXmlTable));
	pTemp += sizeof(gYbXmlTable);

	memcpy(pTemp, (BYTE*)&SET_NUMBER, 1);
	pTemp += 1;
	len = sizeof(gSetXmlTable);
	memcpy(pTemp, (BYTE*)gSetXmlTable, sizeof(gSetXmlTable));
	pTemp += sizeof(gSetXmlTable);

	head->nLength = pTemp - prTableBuf;
	sprintf(fname, "ProSetXml.cfg");
	if(WriteParaFile(fname, (struct VFileHead *)prTableBuf) == ERROR)
		return ERROR;

	return OK;  

}
#endif

int prTableFileJudgy()
{
    prFillTableType();
    if(prSetTableRead() == ERROR)
    {
		prSetTableWrite();
		prPubSetTableWrite();
#ifdef INCLUDE_61850
        prSetXmlWrite();
#endif
		return ERROR;
    }
	return OK;
}

int prPubSetRead(void)
{
   	struct VFileHead *head;
    TTABLETYPE *pType;
	enum SETRET ret = SET_OK;
   
	head = (struct VFileHead *)prSetTmp;	
    pType = (TTABLETYPE*)(head+1);
	caSetBuf  = (BYTE *)(pType+1);

	if (ReadParaFile("fdPubSet.cfg", prSetTmp, sizeof(prSetTmp)) == ERROR)
	{
	    prPubSetDef(); 
	}
	else if (!prSetTypeJudgy(pType))
	{
	    prPubSetDef();
		ret = SET_OTHERERR;
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "单线路保护参数文件不匹配");
	}
	else
	{
		if(FileSetNum <  SET_PUB_NUMBER)
		{
			//设置默认值
			prPubSetPartDef(FileSetNum);
			//保存文件
			if (WriteParaFile("fdPubSet.cfg", (struct VFileHead *)prSetTmp) == ERROR)
					ret = SET_OTHERERR;
			WriteDoEvent(NULL, 0, "公共定值模板升级，补充缺少的部分定值!");
		}
	}

    ret |= prPubSetConv(0);
	if(ret != SET_OK)
	{
	   WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "单线路保护参数错误");
	   prPubSetTableWrite();
	}
	

	memcpy(prSet[prRunPublic[0].prSetnum], caSetBuf, SET_PUB_NUMBER*sizeof(TSETVAL));

	
	if (prRunPublic[0].set_no == 0)
		prRunPublic[0].set_no = 1;
	else
		prRunPublic[0].set_no = 0;
	return ret;
}

int prPubOtherSetRead(int fd)
{
   	char fname[MAXFILENAME];
   	struct VFileHead *head;
    TTABLETYPE *pType;
	enum SETRET ret = SET_OK;
   
	head = (struct VFileHead *)prSetTmp;	
    pType = (TTABLETYPE*)(head+1);
	caSetBuf  = (BYTE *)(pType+1);

	sprintf(fname, "fd%02dPubSet.cfg", fd+1);
	if (ReadParaFile(fname, prSetTmp, sizeof(prSetTmp)) == ERROR)
	{
	    prPubSetDef(); 
	}
	else if (!prSetTypeJudgy(pType))
	{
	    prPubSetDef();
		ret = SET_OTHERERR;
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "公共定值%d参数文件不匹配",fd+1);
	}
	else
	{
		if(FileSetNum <  SET_PUB_NUMBER)
		{
			//设置默认值
			prPubSetPartDef(FileSetNum);
			//保存文件
			if (WriteParaFile(fname, (struct VFileHead *)prSetTmp) == ERROR)
					ret = SET_OTHERERR;
			WriteDoEvent(NULL, 0, "公共定值%d模板升级，补充缺少的部分定值!",fd+1);
		}
	}

    ret |= prPubSetConv(fd);
	if(ret != SET_OK)
	{
	   WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "公共定值%d参数文件不匹配",fd+1);
	   prPubSetTableWrite();
	}
	
	memcpy(prSet[prRunPublic[fd].prSetnum], caSetBuf, SET_PUB_NUMBER*sizeof(TSETVAL));
	
	if (prRunPublic[fd].set_no == 0)
		prRunPublic[fd].set_no = 1;
	else
		prRunPublic[fd].set_no = 0;
	return ret;
}

int prFdSetRead(int fd)
{
   	char fname[MAXFILENAME];
   	struct VFileHead *head;
    TTABLETYPE *pType;
	enum SETRET ret = SET_OK;
   
       //pr disable 
   	if (pFdCfg[fd].cfg & 0x10) return ERROR;
   	
	head = (struct VFileHead *)prSetTmp;	
    pType = (TTABLETYPE*)(head+1);
	caSetBuf  = (BYTE *)(pType+1);

	sprintf(fname, "fd%dset.cfg", fd+1);
	if (ReadParaFile(fname, prSetTmp, sizeof(prSetTmp)) == ERROR)
	{
	    prFdSetDef(); 
	}
	else if (!prSetTypeJudgy(pType))
	{
	    prFdSetDef();
		ret = SET_OTHERERR;
		WriteWarnEvent(NULL, SYS_ERR_CFG, fd+1, "回线%d保护参数文件不匹配", fd+1);
	}
	else
	{
		if(FileSetNum <  SET_NUMBER)
		{
			//设置默认值
			prFdSetPartDef(FileSetNum);
			//保存文件
			if (WriteParaFile(fname, (struct VFileHead *)prSetTmp) == ERROR)
					ret = SET_OTHERERR;
			WriteDoEvent(NULL, 0, "回线%d保护定值模板升级，补充缺少的部分定值!",fd+1);
		}
	}
	
    ret |= prFdSetConv(fd);
	if(ret != SET_OK)
	{
	   WriteWarnEvent(NULL, SYS_ERR_CFG, fd+1, "回线%d保护参数错误", fd+1);
	   prSetTableWrite();
	}
	
	memcpy(prSet[fd+1], caSetBuf, SET_NUMBER*sizeof(TSETVAL));
	
	prFdSetNoSwitch(fd);


	return OK;
}

int TestPrSet(int fd)
{
	TSETVAL *tset = (TSETVAL*)(prSet[fd]);
	TSETVAL *bval;
	int num,i;
   	VPrRunInfo *pInfo = prRunInfo+fd;
	VPrRunSet *pprRunSet = prRunSet[pInfo->set_no]+fd;
	BYTE *wrSet;
	char fname[MAXFILENAME];
	struct VFileHead *head;
	DWORD len,yb;
	BYTE *ptemp;

	
	bval = tset + SET_I1;
	FloatToSet(1.5, bval);
	bval = tset + SET_I2;
	FloatToSet(1.0, bval);
	bval = tset + SET_3I01;
	FloatToSet(0.5, bval);
	bval = tset + SET_T1;
	FloatToSet(0, bval);
	bval = tset + SET_T2;
	FloatToSet(1, bval);
	bval = tset + SET_TI01;
	FloatToSet(2, bval);
	bval = tset + SET_CHNUM;
	FloatToSet(1, bval);
	num = 1;
	for (i=0; i<num; i++)
	{
	    bval = tset + SET_TCH1 + i;
		FloatToSet(2, bval);
	}

	yb = pprRunSet->dYb;
	yb &= ~(PR_YB_I02|PR_YB_I03);
	ReverseKSet(SET_YB, (BYTE*)tset, yb);

	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);

	len = sizeof(TTABLETYPE);
	ptemp = wrSet + len;
	memcpy(wrSet, &tSetType, len);
	memcpy(ptemp, (BYTE*)tset, SET_NUMBER*sizeof(TSETVAL));
	
	sprintf(fname, "fd%dset.cfg", fd+1);
	if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
	{
		return 2;
	}		
	
	caSetBuf = prSet[fd];
	prFdSetConv(fd);
	
	return 0;

}

/*比较装置里面的模版文件与程序里面的模版文件只判断个数，不判断内容 返回0,个数完全一致
返回1，压板、控制字1、控制字2个数不一样，重新生成保护模板即可
返回2，定值个数不一样，既要重新生成保护模板又要重写定值文件
参数1: 文件读取内容，参数2: num 取定值的个数，参数3: fd = 0公共定值模板，1为回线模板
*/	
int  prSetJudgyNum(BYTE *filetemp, BYTE* num ,int fd)
{
	BYTE* pTemp;
	BYTE kg_num,temp_num;
	int ret = 0;
	int i,j;

	//保护
	pTemp = filetemp + sizeof(struct VFileHead);
	pTemp += sizeof(TTABLETYPE);
	kg_num = *pTemp;
	if(fd == 0) // 公共定值
	{
		if(kg_num != YB_PUB_NUMBER)
			ret = 1;
		pTemp += 1;
		pTemp += sizeof(TYBTABLE)*kg_num;
		kg_num = *pTemp;
		if(kg_num != KG1_PUB_NUMBER)
			ret = 1;
		pTemp += 1;
		for(i = 0; i < kg_num ; i++)
		{
			pTemp += sizeof(TKGTABLE)-5;
			temp_num = *pTemp;
			pTemp++;
			for(j = 0 ; j < temp_num ; j++)
			{
				pTemp += (4*2+1);
			}		
		}
		kg_num = *pTemp;
		if(kg_num != KG2_PUB_NUMBER)
			ret = 1;
		pTemp++;

		for(i= 0 ; i < kg_num ; i++)
		{
			pTemp +=  sizeof(TKGTABLE)-5;
			temp_num = *pTemp;
			pTemp++;
			for( j = 0 ; j < temp_num ; j++)
			{
				pTemp += (4*2 + 1);
			}
		}
		kg_num = *pTemp;
		if(kg_num < SET_PUB_NUMBER) //比程序少
			ret = 2;
		else if(kg_num > SET_PUB_NUMBER)
			ret = 1;
		*num = kg_num;
	}
	else
	{
		if(kg_num != YB_NUMBER)
			ret = 1;
		pTemp += 1;
		pTemp += sizeof(TYBTABLE)*kg_num;
		kg_num = *pTemp;
		if(kg_num != KG1_NUMBER)
			ret = 1;
		pTemp += 1;
		for(i = 0; i < kg_num ; i++)
		{
			pTemp += sizeof(TKGTABLE)-5;
			temp_num = *pTemp;
			pTemp++;
			for(j = 0 ; j < temp_num ; j++)
			{
				pTemp += (4*2+1);
			}		
		}
		kg_num = *pTemp;
		if(kg_num != KG2_NUMBER)
			ret = 1;
		pTemp++;

		for(i= 0 ; i < kg_num ; i++)
		{
			pTemp +=  sizeof(TKGTABLE)-5;
			temp_num = *pTemp;
			pTemp++;
			for( j = 0 ; j < temp_num ; j++)
			{
				pTemp += (4*2 + 1);
			}
		}
		kg_num = *pTemp;
		if(kg_num < SET_NUMBER) //比程序少
			ret = 2;
		else if(kg_num > SET_NUMBER)
			ret = 1;
		*num = kg_num;
	}
	
	return ret;
}


int prSetInit(void)
{
    int i;
	prRunSet[0] = (VPrRunSet *)malloc(fdNum*sizeof(VPrRunSet));
	prRunSet[1] = (VPrRunSet *)malloc(fdNum*sizeof(VPrRunSet));

    if (!prRunSet[0] || !prRunSet[1] || !prRunInfo || !fdProtVal)  return ERROR;

    memset(prSetBuf, 0, sizeof(prSetBuf));
    memset(prSetTmp, 0, sizeof(prSetTmp));
	memset(prTableBuf, 0, sizeof(prTableBuf));

	prSetPublic[0] = (VPrSetPublic *)malloc(fdNum*sizeof(VPrSetPublic));
	prSetPublic[1] = (VPrSetPublic *)malloc(fdNum* sizeof(VPrSetPublic));

	if (!prSetPublic[0] || !prSetPublic[1]) return ERROR;	

	prNbSetDef();

	for(i = 0; i < fdNum; i++)
	{
		if(i == 0)
			prRunPublic[i].prSetnum = 0;
		else
			prRunPublic[i].prSetnum = fdNum + i;
	}
	

    if(prTableFileJudgy() == OK) //有保护模板文件ok，无则error
    {
		i = prSetJudgyNum(prTableBuf,&FileSetNum,1);
		if( i > 0)
			prSetTableWrite();
		
	    for (i=0; i<fdNum; i++)
	    {
			prSet[i+1] = prSetBuf + PR_SET_SIZE * (i+1);
			prFdSetRead(i);
		}

		if (ReadParaFile("ProSetPubTable.cfg", prTableBuf, sizeof(prTableBuf)) == ERROR)
		{
			prPubSetTableWrite();
			FileSetNum = SET_PUB_NUMBER;
		}
		else 
		{
			i = prSetJudgyNum(prTableBuf,&FileSetNum,0);
			if( i > 0)
				prPubSetTableWrite();
		}
		prSet[0] = prSetBuf;
		prPubSetRead(); //第一回线公共定值

		//其他回线公共定值
		for (i=1; i<fdNum; i++)
	    {
			prSet[prRunPublic[i].prSetnum] = prSetBuf + PR_SET_SIZE * (fdNum+i);
			prPubOtherSetRead(i);
		}
		
    }
	else
	{
	   	caSetBuf= prSetTmp;
	    prFdSetDef(); 
		for (i=0; i<fdNum; i++)
	    {
			prSet[i+1] = prSetBuf + PR_SET_SIZE * (i+1);
			prFdSetConv(i);
			memcpy(prSet[i+1], caSetBuf, SET_NUMBER*sizeof(TSETVAL));
	
			prFdSetNoSwitch(i);
		}

		prPubSetDef();
		prSet[0] = prSetBuf;
		prPubSetConv(0);
		memcpy(prSet[0], caSetBuf, SET_PUB_NUMBER*sizeof(TSETVAL));

		if (prRunPublic->set_no == 0)
		    prRunPublic->set_no = 1;
	    else
		    prRunPublic->set_no = 0;

		//其他回线公共定值
		for (i=1; i<fdNum; i++)
	    {
			prSet[fdNum+i] = prSetBuf + PR_SET_SIZE * (i+fdNum);
			prPubSetConv(i);
			memcpy(prSet[prRunPublic[i].prSetnum], caSetBuf, SET_PUB_NUMBER*sizeof(TSETVAL));

			if (prRunPublic[i].set_no == 0)
			    prRunPublic[i].set_no = 1;
		    else
			    prRunPublic[i].set_no = 0;
		}
	}
	return OK;
}

void prFdSetNoSwitch(int fd)
{
	VPrRunInfo *pInfo = prRunInfo+fd;
	
    if (pInfo->set_no == 0)
		pInfo->set_no = 1;
	else
		pInfo->set_no = 0;
}

#ifdef INCLUDE_FA_S
int ReadConsValueSet(int fd,int flag,BYTE*pBuf)
{
	VPrConsVlaueSet*pSet = (VPrConsVlaueSet*)pBuf;	
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunSet *pprRunSet;
	float	 fChNum;
	float fval;
	VPrRunInfo *pInfo;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;
	

	switch(flag)
	{
		case PR_DLQ:
		case PR_FHKG:
		case PR_GLBGZ:
		case PR_DLQCH:
		case PR_CKTZ:
		case PR_CKII:
		case PR_BDZCH:
		case PR_DLJSCH:
			/*一段过流压板*/
			if(pprRunSet->dYb&  PR_YB_I1)
				pSet->byIb = 0x1 ;
			else
				pSet->byIb = 0x0 ;

			/*二段过流压板*/
			if(pprRunSet->dYb&  PR_YB_I2)
				pSet->byIIb = 0x1 ;
			else
				pSet->byIIb = 0x0 ;
			
			/*重合闸压板*/	
			if(pprRunSet->dYb & PR_YB_CH)
				pSet->bCHZYB = 0x1;
			else
				pSet->bCHZYB = 0x0;
			/*一段过流定值*/	
			bval = tset + SET_I1;
			SetToFloat(bval,&fval );
			pSet->fI1GLDZ = fval;
			/*一段过流时间*/
			bval = tset + SET_T1;
			SetToFloat(bval,&fval);
			pSet->fT1GL = fval;
			/*跳闸方式*/
			pSet->bTZFS =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_TRIP_MODE) >> KG_17) + 1 ;		
			/*报故障方式*/
			pSet->bFAReport = (BYTE)((pprRunSet->dKG1 & PR_CFG1_FAULT_REPORT) >> (KG_20)) + 1;	

			/*一段出口压板*/		
			pSet->byIPrTripYb =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_I1_TRIP) >> (KG_27))  ;	
			
			/*重合闸次数*/
			bval = tset + SET_CHNUM;
			SetToFloat(bval,&fChNum);
			pSet->bCHNum = (BYTE)fChNum;
			/*一次重合闸时间*/
			bval = tset + SET_TCH1;
			SetToFloat(bval,&fval);
			pSet->fTCH1 = fval;
			/*二次重合闸时间*/
			bval = tset + SET_TCH2;
			SetToFloat(bval,&fval);
			pSet->fTCH2 = fval;
			/*三次重合时间*/
			bval = tset + SET_TCH3;
			SetToFloat(bval,&fval);
			pSet->fTCH3 = fval;
			/*重合闭锁时间*/
			bval = tset + SET_TCHBS;
			SetToFloat(bval,&fval);
			pSet->fTCHBS = fval;
			/*二段过流定值*/	
			bval = tset + SET_I2;
			SetToFloat(bval,&fval );
			pSet->fI2GLDZ = fval;
			/*二段时间定值*/
			bval = tset + SET_T2;
			SetToFloat(bval,&fval);
			pSet->fT2GL = fval;

			bval = tset + SET_IANG1;
			SetToFloat(bval,&fval );
			pSet->AngleLow = fval;
			
			bval = tset + SET_IANG2;
			SetToFloat(bval,&fval);
			pSet->AngleUpp = fval;

			bval = tset + SET_3U0;
			SetToFloat(bval,&fval);
			pSet->XDL_U0 = fval;

			bval = tset + SET_U0TB;
			SetToFloat(bval,&fval);
			pSet->XDL_U0_diff = fval;

			bval = tset + SET_I0TB;
			SetToFloat(bval,&fval);
			pSet->XDL_I0_diff = fval;

			bval = tset + SET_ITB;
			SetToFloat(bval,&fval);
			pSet->XDL_I_diff = fval;

			bval = tset + SET_TDXJD;
			SetToFloat(bval,&fval);
			pSet->XDL_Time = fval;

			bval = tset + SET_XDLCOEF;
			SetToFloat(bval,&fval);
			pSet->XDL_Coef = fval;	

			/*二段出口压板*/		
			pSet->byIIPrTripYb =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_I2_TRIP) >> (KG_28))  ;	

			if(pprRunSet->dKG1&  PR_CFG1_I_DIR)
				pSet->byGLFx = 0x1 ;
			else
				pSet->byGLFx = 0x0 ;

			if(pprRunSet->dKG2&  PR_CFG2_IYX_DIR)
				pSet->byGLYx = 0x1 ;
			else
				pSet->byGLYx = 0x0 ;

			if(pprRunSet->dKG2&  PR_CFG2_XDL_YB)
				pSet->byXDLJD = 0x1 ;
			else
				pSet->byXDLJD = 0x0 ;

			if(pprRunSet->dKG2&  PR_CFG2_XDL_TRIP)
				pSet->byXDLJD_Trip = 0x1 ;
			else
				pSet->byXDLJD_Trip = 0x0 ;

			break;
		default:
			break;
   }
	return 0;
}		
int ReadVoltageSet(int fd, BYTE*pBuf)
{
	VPrdyxSet *pSet = (VPrdyxSet*)pBuf;	
	TSETVAL *tset;
	TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);	
	TSETVAL *bval;
	VPrSetPublic *pprRunSet;
	VPrRunSet *pprRunSet1;	
	VPrRunInfo *pInfo;
	float fval;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;
	pprRunSet1 = prRunSet[pInfo->set_no]+fd;

	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);

	/*单侧失压跳闸*/
	if(pprRunSet->dYb&  PR_YB_SUDY)
		pSet->BSFunyb = 0x1 ;
	else
		pSet->BSFunyb = 0x0 ;
	/*联络开关功能压板*/
	if(pprRunSet->dYb&  PR_YB_SUSY )
		pSet->BLFunyb = 0x1 ;
	else
		pSet->BLFunyb = 0x0 ;

	if(pprRunSet->dYb&  PR_YB_SLWY)
		pSet->LoseVolYb = 0x1;    
	else
		pSet->LoseVolYb = 0x0;
	
	if(pprRunSet->dYb&  PR_YB_U0)
		pSet->U0Yb = 0x1;    
	else
		pSet->U0Yb = 0x0;
	
	if(pprRunSet->dKG1 &  PR_CFG1_X_BS)
		pSet->BXbsyb = 0x1;    
	else
		pSet->BXbsyb = 0x0;

	if(pprRunSet->dKG1 &  PR_CFG1_Y_BS)
		pSet->BYbsyb = 0x1;    
	else
		pSet->BYbsyb = 0x0;

	if(pprRunSet->dKG1 &  PR_CFG1_SY_BS)
		pSet->BSybsyb = 0x1;    
	else
		pSet->BSybsyb = 0x0;

	if(pprRunSet->dKG1 &  PR_CFG1_FZ_BS)
		pSet->FZBS = 0x1;    
	else
		pSet->FZBS = 0x0;
	
	
	 /*SL回线号*/
	 pSet->BSLLineNo = pprRunSet->bySLLine + 1;
	 /*X时限*/
	 bval = tset + SET_PUB_TX;	
	 SetToFloat(bval,&fval);
	 pSet->fXsx = fval;
	 /*Y时限*/
	 bval = tset + SET_PUB_TY;
	 SetToFloat(bval,&fval);
	 pSet->fYsx = fval;
	 /*Z时限*/
	 bval = tset + SET_PUB_TWY;
	 SetToFloat(bval,&fval);
	 pSet->fZsx = fval;
	  /*分闸闭锁复归时间*/
	 bval = tset + SET_PUB_TFZ;
	 SetToFloat(bval,&fval);
	 pSet->fZbstime = fval;
	 
	/*残压时间*/
	 bval = tset + SET_PUB_TSYCY;
	 SetToFloat(bval,&fval);
	 pSet->fTcytime = fval;
	 /* 残压定值 */
	 bval = tset + SET_PUB_UCY;
	 SetToFloat(bval,&fval);
	 pSet->cydz = fval;
	 //零压定值
	 bval = tset + SET_PUB_U0;
	 SetToFloat(bval,&fval);
	 pSet->U0Dz = fval;

	 if(pprRunSet1->dKG2&  PR_CFG2_XDL_YB)
		 pSet->byXDLJD = 0x1 ;
	 else
		 pSet->byXDLJD = 0x0 ;
	 
	 if(pprRunSet1->dKG2&  PR_CFG2_XDL_TRIP)
		 pSet->byXDLJD_Trip = 0x1 ;
	 else
		 pSet->byXDLJD_Trip = 0x0 ;
	 
	 bval = tset1 + SET_3U0;
	 SetToFloat(bval,&fval);
	 pSet->XDL_U0 = fval;
	 
	 bval = tset1 + SET_U0TB;
	 SetToFloat(bval,&fval);
	 pSet->XDL_U0_diff = fval;
	 
	 bval = tset1 + SET_I0TB;
	 SetToFloat(bval,&fval);
	 pSet->XDL_I0_diff = fval;
	 
	 bval = tset1 + SET_ITB;
	 SetToFloat(bval,&fval);
	 pSet->XDL_I_diff = fval;
	 
	 bval = tset1 + SET_TDXJD;
	 SetToFloat(bval,&fval);
	 pSet->XDL_Time = fval;
	 
	 bval = tset1 + SET_XDLCOEF;
	 SetToFloat(bval,&fval);
	 pSet->XDL_Coef = fval;  
	 
	return 0;
}

int ReadCurrentSet(int fd, BYTE*pBuf)
{
	VPrdljsxSet*pSet = (VPrdljsxSet*)pBuf;	
	TSETVAL *tset;
	TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	TSETVAL *bval1;
	VPrSetPublic *pprRunSet;
	VPrRunSet *pprRunSet1;
	float bCHNum;
	float fval;
	VPrRunInfo *pInfo;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;
	pprRunSet1 = prRunSet[pInfo->set_no]+fd;

	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
	/*电流计数压板*/
 	if(pprRunSet->dYb & PR_YB_I_DL)		
  		pSet->BDljsyb = 0x1;
	else
		pSet->BDljsyb = 0x0;
	/*SL回线号*/
	pSet->BSLLineNo = pprRunSet->bySLLine + 1;		
	/*电流计数次数*/
	pSet->BDljsNUM = (BYTE)pprRunSet->dICnt ; 
	/*电流计数复位时间*/
	bval = tset + SET_PUB_TICNT1;
	SetToFloat(bval,&fval);
	pSet->fTdljsReset = fval;
	/*电流计数累计时间*/
	bval = tset + SET_PUB_TICNT2;
	SetToFloat(bval,&fval);
	pSet->fTdljsTotal = fval;

	/*一段过流压板*/
	if(pprRunSet1->dYb&  PR_YB_I1)
		pSet->OCIYb = 0x1 ;
	else
		pSet->OCIYb = 0x0 ;
	/*重合闸压板*/	
	if(pprRunSet1->dYb & PR_YB_CH)
		pSet->bCHZYB = 0x1;
	else
		pSet->bCHZYB = 0x0;
	/*一段过流定值*/	
	bval1 = tset1 + SET_I1;
	SetToFloat(bval1,&fval );
	pSet->OCIDz = fval;
	/*一段过流时间*/
	bval1 = tset1 + SET_T1;
	SetToFloat(bval1,&fval);
	pSet->OCITime = fval;
	/*跳闸方式*/
	pSet->bTZFS =  (BYTE)((pprRunSet1->dKG1 & PR_CFG1_TRIP_MODE) >> KG_17) + 1 ;		
	/*报故障方式*/
	pSet->bFAReport = (BYTE)((pprRunSet1->dKG1 & PR_CFG1_FAULT_REPORT) >> (KG_20)) + 1;	


	/*重合闸次数*/
	bval1 = tset1 + SET_CHNUM;
	SetToFloat(bval1,&bCHNum);
	pSet->bCHNum = (BYTE)bCHNum;
	/*一次重合闸时间*/
	bval1 = tset1 + SET_TCH1;
	SetToFloat(bval1,&fval);
	pSet->fTCH1 = fval;
	/*二次重合闸时间*/
	bval1 = tset1 + SET_TCH2;
	SetToFloat(bval1,&fval);
	pSet->fTCH2 = fval;
	/*三次重合时间*/
	bval1 = tset1 + SET_TCH3;
	SetToFloat(bval1,&fval);
	pSet->fTCH3 = fval;
	/*重合闭锁时间*/
	bval1 = tset1 + SET_TCHBS;
	SetToFloat(bval1,&fval);
	pSet->fTCHBS = fval;

	/*一段出口压板*/	
	pSet->byIPrYb =  (BYTE)((pprRunSet1->dKG1 & PR_CFG1_I1_TRIP) >> (KG_27))  ;	
	
	
	return 0;
}

int ReadZsyVoltageSet(int fd, BYTE*pBuf)
{
		VPrzsyxSet *pSet = (VPrzsyxSet*)pBuf; 
		TSETVAL *tset;
		TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
		TSETVAL *bval;
		TSETVAL *bval1;
		VPrSetPublic *pprRunSet;
		VPrRunSet *pprRunSet1;
		VPrRunInfo *pInfo;
		float fval;

		tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
		pInfo = prRunInfo + fd;
		pprRunSet = prSetPublic[pInfo->set_no]+fd;
		pprRunSet1 = prRunSet[pInfo->set_no]+fd;
		/*单侧失压跳闸*/
		if(pprRunSet->dYb&  PR_YB_SUDY)
			pSet->BSFunyb = 0x1 ;
		else
			pSet->BSFunyb = 0x0 ;
		/*联络开关功能压板*/
		if(pprRunSet->dYb&  PR_YB_SUSY )
			pSet->BLFunyb = 0x1 ;
		else
			pSet->BLFunyb = 0x0 ;

		if(pprRunSet->dYb&  PR_YB_SLWY)
			pSet->LoseVolYb = 0x1;    
		else
			pSet->LoseVolYb = 0x0;  

		if(pprRunSet->dYb&  PR_YB_U0)
			pSet->U0Yb = 0x1;    
		else
			pSet->U0Yb = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_X_BS)
			pSet->BXbsyb = 0x1;    
		else
			pSet->BXbsyb = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_Y_BS)
			pSet->BYbsyb = 0x1;    
		else
			pSet->BYbsyb = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_SY_BS)
			pSet->BSybsyb = 0x1;    
		else
			pSet->BSybsyb = 0x0;

		
		if(pprRunSet->dKG1 &  PR_CFG1_FZ_BS)
			pSet->FZBS = 0x1;    
		else
			pSet->FZBS = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_ZSY_XJDL)
			pSet->byxjdl= 0x1;    
		else
			pSet->byxjdl = 0x0;

		/*SL回线号*/
		pSet->BSLLineNo = pprRunSet->bySLLine + 1;
		/*X时限*/
		bval = tset + SET_PUB_TX;  
		SetToFloat(bval,&fval);
		pSet->fXsx = fval;
		/*Y时限*/
		bval = tset + SET_PUB_TY;
		SetToFloat(bval,&fval);
		pSet->fYsx = fval;
		/*Z时限*/
		bval = tset + SET_PUB_TWY;
		SetToFloat(bval,&fval);
		pSet->fZsx = fval;
		/*S时限*/
		bval = tset + SET_PUB_TS;
		SetToFloat(bval,&fval);
		pSet->fSsx = fval;
		/*C时限*/
		bval = tset + SET_PUB_TC;
		SetToFloat(bval,&fval);
		pSet->fCsx = fval;
		/*残压时间*/
		bval = tset + SET_PUB_TSYCY;
		SetToFloat(bval,&fval);
		pSet->fTcytime = fval;
		/*残压定值*/
		bval = tset + SET_PUB_UCY;
		SetToFloat(bval,&fval);
		pSet->cydz = fval;

		 //零压定值
		bval = tset + SET_PUB_U0;
		SetToFloat(bval,&fval);
		pSet->U0Dz = fval;
		
		 //分闸闭锁复归时间
		bval = tset + SET_PUB_TFZ;
		SetToFloat(bval,&fval);
		pSet->fZbstime = fval;

		
		/*一段过流压板*/	
		if(pprRunSet1->dYb &  PR_YB_I1)
			pSet->byIb = 0x1 ;
		else
			pSet->byIb = 0x0 ;

		if(pprRunSet1->dKG2&  PR_CFG2_XDL_YB)
			pSet->byXDLJD = 0x1 ;
		else
			pSet->byXDLJD = 0x0 ;
		
		if(pprRunSet1->dKG2&  PR_CFG2_XDL_TRIP)
			pSet->byXDLJD_Trip = 0x1 ;
		else
			pSet->byXDLJD_Trip = 0x0 ;
		
		/*一段过流定值*/	
		bval1 = tset1 + SET_I1;
		SetToFloat(bval1,&fval );
		pSet->fI1GLDZ = fval;
		/*一段过流时间*/
		bval1 = tset1 + SET_T1;
		SetToFloat(bval1,&fval);
		pSet->fT1GL = fval;
		/*一段出口压板*/
		pSet->byIPrTripYb =  (BYTE)((pprRunSet1->dKG1 & PR_CFG1_I1_TRIP) >> (KG_27))  ;	

		bval = tset1 + SET_3U0;
		SetToFloat(bval,&fval);
		pSet->XDL_U0 = fval;
		
		bval = tset1 + SET_U0TB;
		SetToFloat(bval,&fval);
		pSet->XDL_U0_diff = fval;
		
		bval = tset1 + SET_I0TB;
		SetToFloat(bval,&fval);
		pSet->XDL_I0_diff = fval;
		
		bval = tset1 + SET_ITB;
		SetToFloat(bval,&fval);
		pSet->XDL_I_diff = fval;
		
		bval = tset1 + SET_TDXJD;
		SetToFloat(bval,&fval);
		pSet->XDL_Time = fval;
		
		bval = tset1 + SET_XDLCOEF;
		SetToFloat(bval,&fval);
		pSet->XDL_Coef = fval;	

		if(pprRunSet->dKG1 &  PR_CFG1_ZSY_DXJD)
			pSet->bYbZsyjd= 0x1;    
		else
			pSet->bYbZsyjd = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_DU_BS)
			pSet->bYbDu= 0x1;    
		else
			pSet->bYbDu = 0x0;
		
		if(pprRunSet->dKG1 &  PR_CFG1_U0_TRIP)
			pSet->bYbU0Trip= 0x1;    
		else
			pSet->bYbU0Trip = 0x0;

		bval = tset + SET_PUB_TU0;
		SetToFloat(bval,&fval);
		pSet->fTU0 = fval;
		
		return 0;
  }
int ReadDyDlVoltageSet(int fd, BYTE*pBuf)
{
		VPrdydlSet *pSet = (VPrdydlSet*)pBuf; 
		TSETVAL *tset;
		TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
		TSETVAL *bval;
		TSETVAL *bval1;
		VPrSetPublic *pprRunSet;
		VPrRunSet *pprRunSet1;
		VPrRunInfo *pInfo;
		float fval;

		tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
		pInfo = prRunInfo + fd;
		pprRunSet = prSetPublic[pInfo->set_no]+fd;
		pprRunSet1 = prRunSet[pInfo->set_no]+fd;
		/*单侧得压*/
		if(pprRunSet->dYb&  PR_YB_SUDY)
			pSet->BSFunyb = 0x1 ;
		else
			pSet->BSFunyb = 0x0 ;
		
		/*单侧失压*/
		if(pprRunSet->dYb&  PR_YB_SUSY )
			pSet->BLFunyb = 0x1 ;
		else
			pSet->BLFunyb = 0x0 ;
		
		/* 无压分闸*/
		if(pprRunSet->dYb&  PR_YB_SLWY)
			pSet->LoseVolYb = 0x1;    
		else
			pSet->LoseVolYb = 0x0;	

		if(pprRunSet->dYb&  PR_YB_U0)
			pSet->U0Yb = 0x1;    
		else
			pSet->U0Yb = 0x0;
		
		/* X闭锁压板*/
		if(pprRunSet->dKG1 &  PR_CFG1_X_BS)
			pSet->BXbsyb = 0x1;    
		else
			pSet->BXbsyb = 0x0;
		/* Y闭锁压板*/
		if(pprRunSet->dKG1 &  PR_CFG1_Y_BS)
			pSet->BYbsyb = 0x1;    
		else
			pSet->BYbsyb = 0x0;
		
		/* 残压闭锁压板*/
		if(pprRunSet->dKG1 &  PR_CFG1_SY_BS)
			pSet->BSybsyb = 0x1;    
		else
			pSet->BSybsyb = 0x0;

			/* 残压闭锁压板*/
		if(pprRunSet->dKG1 &  PR_CFG1_FZ_BS)
			pSet->FZBS = 0x1;    
		else
			pSet->FZBS = 0x0;
		
		/*电压电流型压板*/
		if(pprRunSet->dKG1 &  PR_CFG1_UI_TYPE)  
			pSet->BDyDlYb = 0x1;    
		else
			pSet->BDyDlYb = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_DU_BS)  
			pSet->bYbDu = 0x1;    
		else
			pSet->bYbDu = 0x0;
		
		if(pprRunSet->dKG1 &  PR_CFG1_U0_TRIP)  
			pSet->bYbU0Trip = 0x1;    
		else
			pSet->bYbU0Trip = 0x0;
		
		//回线号	
		 pSet->BSLLineNo = pprRunSet->bySLLine + 1;
		// X时限
		 bval = tset + SET_PUB_TX;	
		 SetToFloat(bval,&fval);
		 pSet->fXsx = fval;
		 //Y时限
		 bval = tset + SET_PUB_TY;
		 SetToFloat(bval,&fval);
		 pSet->fYsx = fval;
		 //Z时限
		 bval = tset + SET_PUB_TWY;
		 SetToFloat(bval,&fval);
		 pSet->fZsx = fval;
		 //残压时间
		 bval = tset + SET_PUB_TSYCY;
		 SetToFloat(bval,&fval);
		 pSet->fTcytime = fval;
		 //残压定值
		 bval = tset + SET_PUB_UCY;
		 SetToFloat(bval,&fval);
		 pSet->cydz = fval;
		//零压定值
		bval = tset + SET_PUB_U0;
		SetToFloat(bval,&fval);
		pSet->U0Dz = fval;

		//零压时间
		bval = tset + SET_PUB_TU0;
		SetToFloat(bval,&fval);
		pSet->fTU0 = fval;

		//分闸闭锁复归时间
		bval = tset + SET_PUB_TFZ;
		SetToFloat(bval,&fval);
		pSet->fZbstime = fval;
		
		// 过流一段压板
		if(pprRunSet1->dYb &  PR_YB_I1)
			pSet->byIb = 0x1 ;
		else
			pSet->byIb = 0x0 ;
		//过流一段定值
		bval1 = tset1 + SET_I1;
		SetToFloat(bval1,&fval );
		pSet->fI1GLDZ = fval;
		//过流一段时间定值
		bval1 = tset1 + SET_T1;
		SetToFloat(bval1,&fval);
		pSet->fT1GL = fval;
		//过流一段出口压板
		pSet->byIPrTripYb =  (BYTE)((pprRunSet1->dKG1 & PR_CFG1_I1_TRIP) >> (KG_27))  ;	

		if(pprRunSet1->dKG2&  PR_CFG2_XDL_YB)
			pSet->byXDLJD = 0x1 ;
		else
			pSet->byXDLJD = 0x0 ;
		
		if(pprRunSet1->dKG2&  PR_CFG2_XDL_TRIP)
			pSet->byXDLJD_Trip = 0x1 ;
		else
			pSet->byXDLJD_Trip = 0x0 ;
		
		bval = tset1 + SET_3U0;
		SetToFloat(bval,&fval);
		pSet->XDL_U0 = fval;
		
		bval = tset1 + SET_U0TB;
		SetToFloat(bval,&fval);
		pSet->XDL_U0_diff = fval;
		
		bval = tset1 + SET_I0TB;
		SetToFloat(bval,&fval);
		pSet->XDL_I0_diff = fval;
		
		bval = tset1 + SET_ITB;
		SetToFloat(bval,&fval);
		pSet->XDL_I_diff = fval;
		
		bval = tset1 + SET_TDXJD;
		SetToFloat(bval,&fval);
		pSet->XDL_Time = fval;
		
		bval = tset1 + SET_XDLCOEF;
		SetToFloat(bval,&fval);
		pSet->XDL_Coef = fval;	

		
		return 0;
  }

int ReadI0ValueSet(int fd,BYTE*pBuf)
{
	VPrI0VlaueSet*pSet = (VPrI0VlaueSet*)pBuf;	
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunSet *pprRunSet;
	float	 fChNum;
	float fval;
	VPrRunInfo *pInfo;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;
		
	/*一段过流压板*/
	if(pprRunSet->dYb&  PR_YB_I1)
		pSet->byIb = 0x1 ;
	else
		pSet->byIb = 0x0 ;

	/*二段过流压板*/
	if(pprRunSet->dYb&  PR_YB_I2)
		pSet->byIIb = 0x1 ;
	else
		pSet->byIIb = 0x0 ;

	/*零流一段压板*/
	if(pprRunSet->dYb&  PR_YB_I01)
		pSet->bI0Yb = 0x1 ;
	else
		pSet->bI0Yb = 0x0 ;

	/*重合闸压板*/	
	if(pprRunSet->dYb & PR_YB_CH)
		pSet->bCHZYB = 0x1;
	else
		pSet->bCHZYB = 0x0;
	/*一段过流定值*/	
	bval = tset + SET_I1;
	SetToFloat(bval,&fval );
	pSet->fI1GLDZ = fval;
	/*一段过流时间*/
	bval = tset + SET_T1;
	SetToFloat(bval,&fval);
	pSet->fT1GL = fval;
	/*跳闸方式*/
	pSet->bTZFS =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_TRIP_MODE) >> KG_17) + 1 ;		
	/*报故障方式*/
	pSet->bFAReport = (BYTE)((pprRunSet->dKG1 & PR_CFG1_FAULT_REPORT) >> (KG_20)) + 1;	

	/*一段出口压板*/		
	pSet->byIPrTripYb =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_I1_TRIP) >> (KG_27))  ;	

	/*重合闸次数*/
	bval = tset + SET_CHNUM;
	SetToFloat(bval,&fChNum);
	pSet->bCHNum = (BYTE)fChNum;
	/*一次重合闸时间*/
	bval = tset + SET_TCH1;
	SetToFloat(bval,&fval);
	pSet->fTCH1 = fval;
	/*二次重合闸时间*/
	bval = tset + SET_TCH2;
	SetToFloat(bval,&fval);
	pSet->fTCH2 = fval;
	/*三次重合时间*/
	bval = tset + SET_TCH3;
	SetToFloat(bval,&fval);
	pSet->fTCH3 = fval;
	/*重合闭锁时间*/
	bval = tset + SET_TCHBS;
	SetToFloat(bval,&fval);
	pSet->fTCHBS = fval;
	/*二段过流定值*/	
	bval = tset + SET_I2;
	SetToFloat(bval,&fval );
	pSet->fI2GLDZ = fval;
	/*二段时间定值*/
	bval = tset + SET_T2;
	SetToFloat(bval,&fval);
	pSet->fT2GL = fval;
	
	/*零流一段定值*/
	bval = tset + SET_3I01;
	SetToFloat(bval,&fval);
	pSet->fI0Dz = fval;

	/*零流一段时间*/
	bval = tset + SET_TI01;
	SetToFloat(bval,&fval);
	pSet->fTI0 = fval;

	
	bval = tset + SET_IANG1;
	SetToFloat(bval,&fval );
	pSet->AngleLow = fval;

	bval = tset + SET_IANG2;
	SetToFloat(bval,&fval);
	pSet->AngleUpp = fval;
	
	bval = tset + SET_3U0;
	SetToFloat(bval,&fval);
	pSet->XDL_U0 = fval;

	bval = tset + SET_U0TB;
	SetToFloat(bval,&fval);
	pSet->XDL_U0_diff = fval;

	bval = tset + SET_I0TB;
	SetToFloat(bval,&fval);
	pSet->XDL_I0_diff = fval;

	bval = tset + SET_ITB;
	SetToFloat(bval,&fval);
	pSet->XDL_I_diff = fval;

	bval = tset + SET_TDXJD;
	SetToFloat(bval,&fval);
	pSet->XDL_Time = fval;

	bval = tset + SET_XDLCOEF;
	SetToFloat(bval,&fval);
	pSet->XDL_Coef = fval;	
	

	/*零流一段出口*/
	pSet->byI0PrTripYb =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_I0_TRIP) >> (KG_29))  ;	
	/*二段出口压板*/		
	pSet->byIIPrTripYb =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_I2_TRIP) >> (KG_28))  ;
	
	if(pprRunSet->dKG1&  PR_CFG1_I_DIR)
		pSet->byGLFx = 0x1 ;
	else
		pSet->byGLFx = 0x0 ;

	if(pprRunSet->dKG2&  PR_CFG2_IYX_DIR)
		pSet->byGLYx = 0x1 ;
	else
		pSet->byGLYx = 0x0 ;

	if(pprRunSet->dKG2&  PR_CFG2_XDL_YB)
		 pSet->byXDLJD = 0x1 ;
	 else
		 pSet->byXDLJD = 0x0 ;
	 
	 if(pprRunSet->dKG2&  PR_CFG2_XDL_TRIP)
		 pSet->byXDLJD_Trip = 0x1 ;
	 else
		 pSet->byXDLJD_Trip = 0x0 ;
	 
   
	return 0;
}
int FaSimGetPara(int fd, int flag,BYTE *pBuf)
{
	
	if (g_prInit == 0)
		return 0;
	
	smMTake(prSetSem);//?是否需要

	switch(flag)
	{
		case PR_DLQ:
		case PR_FHKG:
		case PR_GLBGZ:
		case PR_DLQCH:
		case PR_CKTZ:
		case PR_CKII:
		case PR_BDZCH:
		case PR_DLJSCH:
			ReadConsValueSet(fd,flag,pBuf);
			break;	
		case PR_DYZ:
			ReadVoltageSet(fd, pBuf);
			break;
		case PR_DLJSX:
			ReadCurrentSet(fd, pBuf);
			break;
		case PR_ZSYX:
			ReadZsyVoltageSet(fd, pBuf);
		  	break;
		case PR_DYDLX:
			ReadDyDlVoltageSet(fd, pBuf);
		  	break;
		case PR_CKI0TZ:
			ReadI0ValueSet(fd,pBuf);
			break;
		default:
			break;
	}

	smMGive(prSetSem);
  
	return 0;
}
void ClearVoltageYB(int fd)

{
	TSETVAL *tset = (TSETVAL*)(prSet[0]);
	VPrSetPublic *pprRunSet;
	VPrRunInfo *pInfo;
		
	DWORD yb;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;

	/*清除公共定值所有压板*/
	yb = pprRunSet->dYb;
	yb &= ~ PR_PUBYB_MASK;
	pprRunSet->dYb = yb;
	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);



}
void ClearYBSet(int fd)
{
	TSETVAL *tset;
	VPrSetPublic *pprRunSet;
	TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet1;
	
		
	DWORD yb,yb1;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;
	pprRunSet1 = prRunSet[pInfo->set_no]+fd;

	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
		
	/*清除公共定值所有压板*/
	yb = pprRunSet->dYb;
	yb &= ~ PR_PUBYB_MASK;
	pprRunSet->dYb = yb;
	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);

	yb = pprRunSet->dKG1;
	yb &= ~ PR_PUBCFG1_MASK;
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_PUB_KG1, (BYTE*)tset, yb);

	/*清除开关1所有压板*/
	yb1 = pprRunSet1->dYb;
   	yb1 &= ~PR_YB_MASK;	
	pprRunSet1->dYb = yb1;		
   	ReverseKSet(SET_YB, (BYTE*)tset1, yb1);
	
	yb1 = pprRunSet1->dKG1;
   	yb1 &= ~ PR_CFG1_MASK;	
	pprRunSet1->dKG1 = yb1;		
   	ReverseKSet(SET_KG1, (BYTE*)tset1, yb1);

	yb1 = pprRunSet1->dKG2;
   	yb1 &= ~ PR_CFG2_MASK;	
	pprRunSet1->dKG2 = yb1;		
   	ReverseKSet(SET_KG2, (BYTE*)tset1, yb1);

	caSetBuf = prSet[prRunPublic[fd].prSetnum];
	prPubSetConv(fd);

	if (prRunPublic[fd].set_no == 0)
		prRunPublic[fd].set_no = 1;
	else
		prRunPublic[fd].set_no = 0;
	
	caSetBuf = prSet[fd+1];	
	prFdSetConv(fd);

	prFdSetNoSwitch(fd);
	
}
int WriteConsValueSet(int fd, int flag,BYTE*pSet)
{
	VPrConsVlaueSet*ptr = (VPrConsVlaueSet*)pSet;
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunSet *pprRunSet;
	VPrRunInfo *pInfo;
		
	DWORD yb,TZFS,FAReport,CHNum,PrITripYb,PrIITripYb;

	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;


	switch(flag)
	{
		case PR_DLQ:
		case PR_FHKG:
		case PR_GLBGZ:
		case PR_DLQCH:
		case PR_CKTZ:
		case PR_CKII:
		case PR_BDZCH:
		case PR_DLJSCH:
			/*一段过流压板*/
		 	yb = pprRunSet->dYb;
		  	if (ptr->byIb==0)
	       		 yb &= ~PR_YB_I1;
		   	else
				 yb |= PR_YB_I1;
			/*二段过流压板   */
			if (ptr->byIIb==0)
	       		 yb &= ~PR_YB_I2;
		   	else
				 yb |= PR_YB_I2;
			/*重合闸压板*/
		  	if ((ptr->bCHZYB == 0))
	       		 yb &= ~PR_YB_CH ;
		   	else
				 yb |= PR_YB_CH ;
			pprRunSet->dYb = yb;	
		   	ReverseKSet(SET_YB, (BYTE*)tset, yb);
			/*一段过流定值*/
			bval = tset + SET_I1;
			FloatToSet(ptr->fI1GLDZ, bval);
			/*一段过流时间*/
			bval = tset + SET_T1;
			FloatToSet(ptr->fT1GL, bval);
			/*二段过流定值*/
			bval = tset + SET_I2;
			FloatToSet(ptr->fI2GLDZ, bval);
			/*二段过流时间*/
			bval = tset + SET_T2;
			FloatToSet(ptr->fT2GL, bval);

			bval = tset + SET_IANG1;
			FloatToSet(ptr->AngleLow, bval);

			bval = tset + SET_IANG2;
			FloatToSet(ptr->AngleUpp, bval);
			
			
			/*跳闸方式*/	
			TZFS  = (pprRunSet->dKG1&~PR_CFG1_TRIP_MODE) | ((ptr->bTZFS - 1) << KG_17) ;  // D17-D19 跳闸方式
			pprRunSet->dKG1 = TZFS;	
			ReverseKSet(SET_KG1, (BYTE*)tset, TZFS);
			/*报故障方式*/
			FAReport = (pprRunSet->dKG1&~PR_CFG1_FAULT_REPORT) |( (ptr->bFAReport - 1) << (KG_20)); // D20-D21 上报故障方式
			pprRunSet->dKG1 = FAReport;
			ReverseKSet(SET_KG1, (BYTE*)tset, FAReport);

			/*一段出口压板*/	
			PrITripYb = (pprRunSet->dKG1 & ~PR_CFG1_I1_TRIP) |( (ptr->byIPrTripYb) << (KG_27)); //保护出口投入 D27
			pprRunSet->dKG1 = PrITripYb;
			ReverseKSet(SET_KG1, (BYTE*)tset, PrITripYb);

			/*二段出口压板*/	
			PrIITripYb = (pprRunSet->dKG1 & ~PR_CFG1_I2_TRIP) |( (ptr->byIIPrTripYb) << (KG_28)); //保护出口投入 D28
			pprRunSet->dKG1 = PrIITripYb;
			ReverseKSet(SET_KG1, (BYTE*)tset, PrIITripYb);

			yb = pprRunSet->dKG1;
			if (ptr->byGLFx == 0) 
				yb &= ~ PR_CFG1_I_DIR;
			else
				yb |=  PR_CFG1_I_DIR;	
			pprRunSet->dKG1 = yb;
			ReverseKSet(SET_KG1, (BYTE*)tset, yb);

			yb = pprRunSet->dKG2;
			if (ptr->byGLYx == 0) 
				yb &= ~ PR_CFG2_IYX_DIR;
			else
				yb |=  PR_CFG2_IYX_DIR;	

			if (ptr->byXDLJD == 0) 
				yb &= ~ PR_CFG2_XDL_YB;
			else
				yb |=  PR_CFG2_XDL_YB;	

			if (ptr->byXDLJD_Trip == 0) 
				yb &= ~ PR_CFG2_XDL_TRIP;
			else
				yb |=  PR_CFG2_XDL_TRIP;				
			
			pprRunSet->dKG2 = yb;
			ReverseKSet(SET_KG2, (BYTE*)tset, yb);
			
			/*重合闸次数*/
			CHNum = ptr->bCHNum; 	
			bval = tset + SET_CHNUM;	
			FloatToSet(CHNum, bval);			
			
			/*一次重合闸时间*/
			bval = tset + SET_TCH1;
			FloatToSet(ptr->fTCH1, bval);
			/*二次重合闸时间*/
			bval = tset + SET_TCH2;
			FloatToSet(ptr->fTCH2, bval);
			/*三次重合闸时间*/
			bval = tset + SET_TCH3;
			FloatToSet(ptr->fTCH3, bval);	
			/*重合闭锁时间*/
			bval = tset + SET_TCHBS;
			FloatToSet(ptr->fTCHBS, bval);

			//零压
			bval = tset + SET_3U0;
			FloatToSet(ptr->XDL_U0, bval);		

			//小电流零压突变
			bval = tset +   SET_U0TB;
			FloatToSet(ptr->XDL_U0_diff, bval);		

			//小电流零流突变
			bval = tset + SET_I0TB;
			FloatToSet(ptr->XDL_I0_diff, bval);		

			//小电流电流突变	
			bval = tset + SET_ITB;
			FloatToSet(ptr->XDL_I_diff, bval);		
			
			//小电流接地时间	
			bval = tset + SET_TDXJD;
			FloatToSet(ptr->XDL_Time, bval);		
			
			//小电流接地系数
			bval = tset + SET_XDLCOEF;
			FloatToSet(ptr->XDL_Coef, bval);		

			break;
		default:
			break;
	}
	
	
	return 0;
}
int WriteVoltageSet(int fd, BYTE*pSet)
{
	VPrdyxSet*ptr = ( VPrdyxSet*)pSet;
	TSETVAL *tset;
	TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrSetPublic *pprRunSet;
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet1;
	DWORD yb;
	BYTE SLLineNo;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;
	pprRunSet1 = prRunSet[pInfo->set_no]+fd;


	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
	/*单侧失压跳闸*/
	yb = pprRunSet->dYb;
	if(ptr->BSFunyb == 0)
		yb &= ~ PR_YB_SUDY;
	else
		yb |= PR_YB_SUDY;
	/*联络开关功能压板*/
	if(ptr->BLFunyb == 0)
		yb &= ~ PR_YB_SUSY;
	else
		yb |=  PR_YB_SUSY;	

	if (ptr->LoseVolYb == 0)
		yb &= ~ PR_YB_SLWY;
	else
		yb |=  PR_YB_SLWY;

	if (ptr->U0Yb == 0)
		yb &= ~ PR_YB_U0;
	else
		yb |=  PR_YB_U0;
			
	pprRunSet->dYb = yb;
	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);

	yb = pprRunSet->dKG1;
	/*X闭锁功能压板*/
	if (ptr->BXbsyb == 0) 
		yb &= ~ PR_CFG1_X_BS;
	else
		yb |=  PR_CFG1_X_BS;	
		
	/*Y闭锁功能压板*/ 
	if (ptr->BYbsyb == 0)
		yb &= ~ PR_CFG1_Y_BS;
	else
		yb |=  PR_CFG1_Y_BS;	
	/*  合成功闭锁分闸*/
	if (ptr->FZBS == 0)
		yb &= ~ PR_CFG1_FZ_BS;
	else
		yb |=  PR_CFG1_FZ_BS;	
	
		
	/*瞬压闭锁功能压板*/
	if (ptr->BSybsyb == 0)
		yb &= ~ PR_CFG1_SY_BS;
	else
		yb |=  PR_CFG1_SY_BS;	
     yb &= ~(PR_CFG1_ZSY_XJDL|PR_CFG1_ZSY_DXJD|PR_CFG1_UI_TYPE);
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_PUB_KG1, (BYTE*)tset, yb);
		
	/*SL回线号*/
	SLLineNo = ptr->BSLLineNo;
	ReverseKSet(SET_PUB_SLFD, (BYTE*)tset, SLLineNo);
	/*X时限*/
	bval = tset + SET_PUB_TX;
	FloatToSet(ptr->fXsx, bval);
	/*Y时限*/
	bval = tset + SET_PUB_TY;
	FloatToSet(ptr->fYsx, bval);
	/*Z时限*/
	bval = tset + SET_PUB_TWY;
	FloatToSet(ptr->fZsx, bval);

	bval = tset + SET_PUB_TSYCY;
	FloatToSet(ptr->fTcytime, bval);
	bval = tset + SET_PUB_UCY;
	FloatToSet(ptr->cydz, bval);
	//零压定值
	bval = tset + SET_PUB_U0;
	FloatToSet(ptr->U0Dz, bval);
	//分闸闭锁复归时间
	bval = tset + SET_PUB_TFZ;
	FloatToSet(ptr->fZbstime, bval);

	yb = pprRunSet1->dKG2;
	
	if (ptr->byXDLJD == 0) 
		yb &= ~ PR_CFG2_XDL_YB;
	else
		yb |=  PR_CFG2_XDL_YB;	
	
	if (ptr->byXDLJD_Trip == 0) 
		yb &= ~ PR_CFG2_XDL_TRIP;
	else
		yb |=  PR_CFG2_XDL_TRIP;				
	
	pprRunSet1->dKG2 = yb;
	ReverseKSet(SET_KG2, (BYTE*)tset1, yb);

	//零压
	bval = tset1 + SET_3U0;
	FloatToSet(ptr->XDL_U0, bval);		
	
	//小电流零压突变
	bval = tset1 +	SET_U0TB;
	FloatToSet(ptr->XDL_U0_diff, bval); 	
	
	//小电流零流突变
	bval = tset1 + SET_I0TB;
	FloatToSet(ptr->XDL_I0_diff, bval); 	
	
	//小电流电流突变 
	bval = tset1 + SET_ITB;
	FloatToSet(ptr->XDL_I_diff, bval);		
	
	//小电流接地时间 
	bval = tset1 + SET_TDXJD;
	FloatToSet(ptr->XDL_Time, bval);		
	
	//小电流接地系数
	bval = tset1 + SET_XDLCOEF;
	FloatToSet(ptr->XDL_Coef, bval);	

	return 0;
 }

int WriteCurrentSet(int fd, BYTE*pSet)
{
	VPrdljsxSet*ptr = ( VPrdljsxSet*)pSet;
	TSETVAL *tset;
	TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	TSETVAL *bval1;
	VPrSetPublic *pprRunSet;
	VPrRunSet *pprRunSet1;
	VPrRunInfo *pInfo;
	DWORD yb,yb1,DljsNUM,TZFS,FAReport,CHNum, PrITripYb;
	BYTE SLLineNo;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;
	pprRunSet1 = prRunSet[pInfo->set_no]+fd;

	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
	/*电流计数压板*/
 	yb = pprRunSet->dYb;
  	if (ptr->BDljsyb == 0)
   		yb &= ~PR_YB_I_DL;
   	else
		yb |= PR_YB_I_DL;
	pprRunSet->dYb = yb;		
   	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);
		
  yb = pprRunSet->dKG1;
	yb &= ~(PR_CFG1_ZSY_XJDL|PR_CFG1_ZSY_DXJD|PR_CFG1_UI_TYPE);
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_PUB_KG1, (BYTE*)tset, yb);	
		
	SLLineNo = ptr->BSLLineNo;							
	ReverseKSet(SET_PUB_SLFD, (BYTE*)tset, SLLineNo);		
	/*电流计数次数*/
	DljsNUM = ptr->BDljsNUM;	
	ReverseKSet(SET_PUB_ICNT, (BYTE*)tset, DljsNUM);
	/*电流计数复位时间*/
	bval = tset + SET_PUB_TICNT1;
	FloatToSet(ptr->fTdljsReset, bval);
	/*电流计数累计时间*/
	bval = tset + SET_PUB_TICNT2;
	FloatToSet(ptr->fTdljsTotal, bval);		
      /*一段过流压板*/
	yb1 = pprRunSet1->dYb;
	if (ptr->OCIYb==0)
		yb1 &= ~PR_YB_I1;
	else
	 	yb1 |= PR_YB_I1;
	/*重合闸压板*/
	if ((ptr->bCHZYB == 0))
		 yb1 &= ~PR_YB_CH ;
	else
	 	yb1 |= PR_YB_CH ;
	pprRunSet1->dYb = yb1;	
	ReverseKSet(SET_YB, (BYTE*)tset1, yb1);
	/*一段过流定值*/
	bval1 = tset1 + SET_I1;
	FloatToSet(ptr->OCIDz, bval1);
	/*一段过流时间*/
	bval1 = tset1 + SET_T1;
	FloatToSet(ptr->OCITime, bval1);
	/*跳闸方式*/	
	TZFS  = (pprRunSet1->dKG1&~PR_CFG1_TRIP_MODE) | ((ptr->bTZFS - 1) << KG_17) ;  // D17-D19 跳闸方式
	pprRunSet1->dKG1 = TZFS;	
	ReverseKSet(SET_KG1, (BYTE*)tset1, TZFS);
	/*报故障方式*/
	FAReport = (pprRunSet1->dKG1&~PR_CFG1_FAULT_REPORT) |( (ptr->bFAReport - 1) << (KG_20)); // D20-D21 上报故障方式
	pprRunSet1->dKG1 = FAReport;
	ReverseKSet(SET_KG1, (BYTE*)tset1, FAReport);
	/*重合闸次数*/
	CHNum = ptr->bCHNum; 	
	bval1 = tset1 + SET_CHNUM;			
	FloatToSet(CHNum, bval1);	

	/*一次重合闸时间*/
	bval1 = tset1 + SET_TCH1;
	FloatToSet(ptr->fTCH1, bval1);

	/* 二次重合时间*/
	bval1 = tset1 + SET_TCH2;
	FloatToSet(ptr->fTCH2, bval1);

	/*三次重合时间*/
	bval1 = tset1 + SET_TCH3;
	FloatToSet(ptr->fTCH3, bval1);
	/*重合闭锁时间*/
	bval1 = tset1 + SET_TCHBS;
	FloatToSet(ptr->fTCHBS, bval1);

	/*一段出口压板*/	
	PrITripYb = (pprRunSet1->dKG1 & ~PR_CFG1_I1_TRIP) |( (ptr->byIPrYb) << (KG_27)); //保护出口投入 D27
	pprRunSet1->dKG1 = PrITripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset1, PrITripYb);

  return 0;		
}
int WriteZsyVoltageSet(int fd, BYTE*pSet)
  {
    VPrzsyxSet*ptr = ( VPrzsyxSet*)pSet;
    TSETVAL *tset;
    TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
    TSETVAL *bval;
    TSETVAL *bval1;
    VPrSetPublic *pprRunSet;
    VPrRunSet *pprRunSet1;
    VPrRunInfo *pInfo;      
    DWORD yb,yb1,PrITripYb;
    BYTE SLLineNo;
    pInfo = prRunInfo + fd;
    pprRunSet = prSetPublic[pInfo->set_no]+fd;
    pprRunSet1 = prRunSet[pInfo->set_no]+fd;

	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
    /*单侧失压跳闸*/
	yb = pprRunSet->dYb;
	if(ptr->BSFunyb == 0)
	  yb &= ~ PR_YB_SUDY;
	else
	  yb |= PR_YB_SUDY;
	/*联络开关功能压板*/
	if(ptr->BLFunyb == 0)
	  yb &= ~ PR_YB_SUSY;
	else
	  yb |=  PR_YB_SUSY;  

	if (ptr->LoseVolYb == 0)
	  yb &= ~ PR_YB_SLWY;
	else
	  yb |=  PR_YB_SLWY;
	
	if (ptr->U0Yb == 0)
		yb &= ~ PR_YB_U0;
	else
		yb |=  PR_YB_U0;
	    
	pprRunSet->dYb = yb;
	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);

	yb = pprRunSet->dKG1;
	/*X闭锁功能压板*/
	if (ptr->BXbsyb == 0) 
	  yb &= ~ PR_CFG1_X_BS;
	else
	  yb |=  PR_CFG1_X_BS;  
	  
	/*Y闭锁功能压板*/ 
	if (ptr->BYbsyb == 0)
	  yb &= ~ PR_CFG1_Y_BS;
	else
	  yb |=  PR_CFG1_Y_BS;  

	/*  合成功闭锁分闸*/
	if (ptr->FZBS == 0)
		yb &= ~ PR_CFG1_FZ_BS;
	else
		yb |=  PR_CFG1_FZ_BS;
	  
	/*瞬压闭锁功能压板*/
	if (ptr->BSybsyb == 0)
	  yb &= ~ PR_CFG1_SY_BS;
	else
	  yb |=  PR_CFG1_SY_BS; 

	if(ptr->byxjdl ==0)
		yb &= ~ PR_CFG1_ZSY_XJDL;
	else
		yb |=  PR_CFG1_ZSY_XJDL;

	if(ptr->bYbZsyjd)
		yb |= PR_CFG1_ZSY_DXJD;
	else
		yb &= ~PR_CFG1_ZSY_DXJD;

	if(ptr->bYbDu)
		yb |= PR_CFG1_DU_BS;
	else
		yb &= ~PR_CFG1_DU_BS;

	if(ptr->bYbU0Trip)
		yb |= PR_CFG1_U0_TRIP;
	else
		yb &= ~PR_CFG1_U0_TRIP;
	
	yb &= ~PR_CFG1_UI_TYPE;
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_PUB_KG1, (BYTE*)tset, yb);
	  
	/*SL回线号*/
	SLLineNo = ptr->BSLLineNo;
	ReverseKSet(SET_PUB_SLFD, (BYTE*)tset, SLLineNo);
	/*X时限*/
	bval = tset + SET_PUB_TX;
	FloatToSet(ptr->fXsx, bval);
	/*Y时限*/
	bval = tset + SET_PUB_TY;
	FloatToSet(ptr->fYsx, bval);
	/*Z时限*/
	bval = tset + SET_PUB_TWY;
	FloatToSet(ptr->fZsx, bval);
	/*S时限*/
	bval = tset + SET_PUB_TS;
	FloatToSet(ptr->fSsx, bval);
	/*C时限*/
	bval = tset + SET_PUB_TC;
	FloatToSet(ptr->fCsx, bval);
	bval = tset + SET_PUB_TSYCY;
	FloatToSet(ptr->fTcytime, bval);
	bval = tset + SET_PUB_UCY;
	FloatToSet(ptr->cydz, bval);
	//零压定值
	bval = tset + SET_PUB_U0;
	FloatToSet(ptr->U0Dz, bval);
	//零压时间
	bval = tset + SET_PUB_TU0;
	FloatToSet(ptr->fTU0, bval);
	//分闸闭锁复归时间
	bval = tset + SET_PUB_TFZ;
	FloatToSet(ptr->fZbstime, bval);

	/*一段过流压板*/
	yb1 = pprRunSet1->dYb;
	if (ptr->byIb==0)
		yb1 &= ~PR_YB_I1;
	else
		yb1 |= PR_YB_I1;
	pprRunSet1->dYb = yb1;	
	ReverseKSet(SET_YB, (BYTE*)tset1, yb1);

	yb = pprRunSet1->dKG2;

	if (ptr->byXDLJD == 0) 
		yb &= ~ PR_CFG2_XDL_YB;
	else
		yb |=  PR_CFG2_XDL_YB;	
	
	if (ptr->byXDLJD_Trip == 0) 
		yb &= ~ PR_CFG2_XDL_TRIP;
	else
		yb |=  PR_CFG2_XDL_TRIP;				
	
	pprRunSet1->dKG2 = yb;
	ReverseKSet(SET_KG2, (BYTE*)tset1, yb);

	
	/*一段过流定值*/
	bval1 = tset1 + SET_I1;
	FloatToSet(ptr->fI1GLDZ, bval1);
	/*一段过流时间*/
	bval1 = tset1 + SET_T1;
	FloatToSet(ptr->fT1GL, bval1);

	//零压
	bval = tset1 + SET_3U0;
	FloatToSet(ptr->XDL_U0, bval);		
	
	//小电流零压突变
	bval = tset1 +	SET_U0TB;
	FloatToSet(ptr->XDL_U0_diff, bval); 	
	
	//小电流零流突变
	bval = tset1 + SET_I0TB;
	FloatToSet(ptr->XDL_I0_diff, bval); 	
	
	//小电流电流突变 
	bval = tset1 + SET_ITB;
	FloatToSet(ptr->XDL_I_diff, bval);		
	
	//小电流接地时间 
	bval = tset1 + SET_TDXJD;
	FloatToSet(ptr->XDL_Time, bval);		
	
	//小电流接地系数
	bval = tset1 + SET_XDLCOEF;
	FloatToSet(ptr->XDL_Coef, bval);	

	/*一段出口压板*/	
	PrITripYb = (pprRunSet1->dKG1 & ~PR_CFG1_I1_TRIP) |( (ptr->byIPrTripYb) << (KG_27)); //保护出口投入 D27
	pprRunSet1->dKG1 = PrITripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset1, PrITripYb);  

    return 0;
   }
int WriteDYDLVoltageSet(int fd, BYTE*pSet)
{
    VPrdydlSet*ptr = ( VPrdydlSet*)pSet;
    TSETVAL *tset;
    TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
    TSETVAL *bval;
    TSETVAL *bval1;
    VPrSetPublic *pprRunSet;
    VPrRunSet *pprRunSet1;
    VPrRunInfo *pInfo;
    DWORD yb,yb1,PrITripYb;
    BYTE SLLineNo;
	
	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
    pInfo = prRunInfo + fd;
    pprRunSet = prSetPublic[pInfo->set_no]+fd;
    pprRunSet1 = prRunSet[pInfo->set_no]+fd;
	yb = pprRunSet->dYb;
	if(ptr->BSFunyb == 0)
	  yb &= ~ PR_YB_SUDY;
	else
	  yb |= PR_YB_SUDY;
	if(ptr->BLFunyb == 0)
	  yb &= ~ PR_YB_SUSY;
	else
	  yb |=  PR_YB_SUSY;  

	if (ptr->LoseVolYb == 0)
	  yb &= ~ PR_YB_SLWY;
	else
	  yb |=  PR_YB_SLWY;

	if (ptr->U0Yb == 0)
		yb &= ~ PR_YB_U0;
	else
		yb |=  PR_YB_U0;
	pprRunSet->dYb = yb;
	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);

	yb = pprRunSet->dKG1;
	if (ptr->BXbsyb == 0) 
	  yb &= ~ PR_CFG1_X_BS;
	else
	  yb |=  PR_CFG1_X_BS;  
	if (ptr->BYbsyb == 0)
	  yb &= ~ PR_CFG1_Y_BS;
	else
	  yb |=  PR_CFG1_Y_BS;  
	if (ptr->BSybsyb == 0)
	  yb &= ~ PR_CFG1_SY_BS;
	else
	  yb |=  PR_CFG1_SY_BS; 
	if (ptr->BDyDlYb == 0)
	  yb &= ~ PR_CFG1_UI_TYPE;
	else
	  yb |=  PR_CFG1_UI_TYPE; 
	if (ptr->FZBS == 0)
		yb &= ~ PR_CFG1_FZ_BS;
	else
		yb |=  PR_CFG1_FZ_BS;

	if (ptr->bYbDu)
		yb |= PR_CFG1_DU_BS;
	else
		yb &= ~PR_CFG1_DU_BS;

	if (ptr->bYbU0Trip)
		yb |= PR_CFG1_U0_TRIP;
	else
		yb &= ~PR_CFG1_U0_TRIP;
	
	 yb &= ~(PR_CFG1_ZSY_XJDL|PR_CFG1_ZSY_DXJD);
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_PUB_KG1, (BYTE*)tset, yb);
	SLLineNo = ptr->BSLLineNo;
	ReverseKSet(SET_PUB_SLFD, (BYTE*)tset, SLLineNo);
	bval = tset + SET_PUB_TX;
	FloatToSet(ptr->fXsx, bval);
	bval = tset + SET_PUB_TY;
	FloatToSet(ptr->fYsx, bval);
	bval = tset + SET_PUB_TWY;
	FloatToSet(ptr->fZsx, bval);
	bval = tset + SET_PUB_TSYCY;
	FloatToSet(ptr->fTcytime, bval);
	bval = tset + SET_PUB_UCY;
	FloatToSet(ptr->cydz, bval);
	//零压定值
	bval = tset + SET_PUB_U0;
	FloatToSet(ptr->U0Dz, bval);

	//零压定值
	bval = tset + SET_PUB_TU0;
	FloatToSet(ptr->fTU0, bval);

	//分闸闭锁复归时间
	bval = tset + SET_PUB_TFZ;
	FloatToSet(ptr->fZbstime, bval);

	
	yb1 = pprRunSet1->dYb;
	if (ptr->byIb==0)
		yb1 &= ~PR_YB_I1;
	else
		yb1 |= PR_YB_I1;
	pprRunSet1->dYb = yb1;	
	ReverseKSet(SET_YB, (BYTE*)tset1, yb1);

	yb = pprRunSet1->dKG2;

	if (ptr->byXDLJD == 0) 
		yb &= ~ PR_CFG2_XDL_YB;
	else
		yb |=  PR_CFG2_XDL_YB;	
	
	if (ptr->byXDLJD_Trip == 0) 
		yb &= ~ PR_CFG2_XDL_TRIP;
	else
		yb |=  PR_CFG2_XDL_TRIP;				
	
	pprRunSet1->dKG2 = yb;
	ReverseKSet(SET_KG2, (BYTE*)tset1, yb);

	
	bval1 = tset1 + SET_I1;
	FloatToSet(ptr->fI1GLDZ, bval1);
	bval1 = tset1 + SET_T1;
	FloatToSet(ptr->fT1GL, bval1);
	PrITripYb = (pprRunSet1->dKG1 & ~PR_CFG1_I1_TRIP) |( (ptr->byIPrTripYb) << (KG_27)); //保护出口投入 D27
	pprRunSet1->dKG1 = PrITripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset1, PrITripYb);  

		//零压
	bval = tset1 + SET_3U0;
	FloatToSet(ptr->XDL_U0, bval);		
	
	//小电流零压突变
	bval = tset1 +	SET_U0TB;
	FloatToSet(ptr->XDL_U0_diff, bval); 	
	
	//小电流零流突变
	bval = tset1 + SET_I0TB;
	FloatToSet(ptr->XDL_I0_diff, bval); 	
	
	//小电流电流突变 
	bval = tset1 + SET_ITB;
	FloatToSet(ptr->XDL_I_diff, bval);		
	
	//小电流接地时间 
	bval = tset1 + SET_TDXJD;
	FloatToSet(ptr->XDL_Time, bval);		
	
	//小电流接地系数
	bval = tset1 + SET_XDLCOEF;
	FloatToSet(ptr->XDL_Coef, bval);	
    return 0;
   }
int WriteI0ValueSet(int fd, BYTE*pSet)
{
	
	VPrI0VlaueSet*ptr = (VPrI0VlaueSet*)pSet;
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunSet *pprRunSet;
	VPrRunInfo *pInfo;
		
	DWORD yb,TZFS,FAReport,CHNum,PrITripYb,PrIITripYb,PrI0TripYb;

	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;

	/*一段过流压板*/
	yb = pprRunSet->dYb;
	if (ptr->byIb==0)
		yb &= ~PR_YB_I1;
	else
	 	yb |= PR_YB_I1;
	/*二段过流压板   */
	if (ptr->byIIb==0)
		yb &= ~PR_YB_I2;
	else
		yb |= PR_YB_I2;
	/*重合闸压板*/
	if ((ptr->bCHZYB == 0))
		yb &= ~PR_YB_CH ;
	else
		yb |= PR_YB_CH ;
	//零流一段压板
	if ((ptr->bI0Yb == 0))
		yb &= ~PR_YB_I01 ;
	else
		yb |= PR_YB_I01 ;
	pprRunSet->dYb = yb;	
	ReverseKSet(SET_YB, (BYTE*)tset, yb);
	/*一段过流定值*/
	bval = tset + SET_I1;
	FloatToSet(ptr->fI1GLDZ, bval);
	/*一段过流时间*/
	bval = tset + SET_T1;
	FloatToSet(ptr->fT1GL, bval);
	/*二段过流定值*/
	bval = tset + SET_I2;
	FloatToSet(ptr->fI2GLDZ, bval);
	/*二段过流时间*/
	bval = tset + SET_T2;
	FloatToSet(ptr->fT2GL, bval);

	//零流一段定值
	bval = tset + SET_3I01;
	FloatToSet(ptr->fI0Dz, bval);
	//零流一段时间
	bval = tset + SET_TI01;
	FloatToSet(ptr->fTI0, bval);
	
	/*跳闸方式*/	
	TZFS  = (pprRunSet->dKG1&~PR_CFG1_TRIP_MODE) | ((ptr->bTZFS - 1) << KG_17) ;  // D17-D19 跳闸方式
	pprRunSet->dKG1 = TZFS;	
	ReverseKSet(SET_KG1, (BYTE*)tset, TZFS);
	/*报故障方式*/
	FAReport = (pprRunSet->dKG1&~PR_CFG1_FAULT_REPORT) |( (ptr->bFAReport - 1) << (KG_20)); // D20-D21 上报故障方式
	pprRunSet->dKG1 = FAReport;
	ReverseKSet(SET_KG1, (BYTE*)tset, FAReport);

	/*一段出口压板*/	
	PrITripYb = (pprRunSet->dKG1 & ~PR_CFG1_I1_TRIP) |( (ptr->byIPrTripYb) << (KG_27)); //保护出口投入 D27
	pprRunSet->dKG1 = PrITripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset, PrITripYb);

	/*二段出口压板*/	
	PrIITripYb = (pprRunSet->dKG1 & ~PR_CFG1_I2_TRIP) |( (ptr->byIIPrTripYb) << (KG_28)); //保护出口投入 D28
	pprRunSet->dKG1 = PrIITripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset, PrIITripYb);

	
	//零流一段出口压板
	PrI0TripYb = (pprRunSet->dKG1 & ~PR_CFG1_I0_TRIP) |( (ptr->byI0PrTripYb) << (KG_29)); //保护出口投入 D28
	pprRunSet->dKG1 = PrI0TripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset, PrI0TripYb);

	yb = pprRunSet->dKG1;
	if (ptr->byGLFx == 0) 
		yb &= ~ PR_CFG1_I_DIR;
	else
		yb |=  PR_CFG1_I_DIR;	
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_KG1, (BYTE*)tset, yb);

	yb = pprRunSet->dKG2;
	if (ptr->byGLYx == 0) 
		yb &= ~ PR_CFG2_IYX_DIR;
	else
		yb |=  PR_CFG2_IYX_DIR;	

	if (ptr->byXDLJD == 0) 
		yb &= ~ PR_CFG2_XDL_YB;
	else
		yb |=  PR_CFG2_XDL_YB;	
	
	if (ptr->byXDLJD_Trip == 0) 
		yb &= ~ PR_CFG2_XDL_TRIP;
	else
		yb |=  PR_CFG2_XDL_TRIP;		
	
	pprRunSet->dKG2 = yb;
	ReverseKSet(SET_KG2, (BYTE*)tset, yb);


	/*重合闸次数*/
	CHNum = ptr->bCHNum; 	
	bval = tset + SET_CHNUM;	
	FloatToSet(CHNum, bval);			

	/*一次重合闸时间*/
	bval = tset + SET_TCH1;
	FloatToSet(ptr->fTCH1, bval);
	/*二次重合闸时间*/
	bval = tset + SET_TCH2;
	FloatToSet(ptr->fTCH2, bval);
	/*三次重合闸时间*/
	bval = tset + SET_TCH3;
	FloatToSet(ptr->fTCH3, bval);	
	/*重合闭锁时间*/
	bval = tset + SET_TCHBS;
	FloatToSet(ptr->fTCHBS, bval);

	bval = tset + SET_IANG1;
	FloatToSet(ptr->AngleLow, bval);

	bval = tset + SET_IANG2;
	FloatToSet(ptr->AngleUpp, bval);

	//零压
	bval = tset + SET_3U0;
	FloatToSet(ptr->XDL_U0, bval);		
	
	//小电流零压突变
	bval = tset +	SET_U0TB;
	FloatToSet(ptr->XDL_U0_diff, bval); 	
	
	//小电流零流突变
	bval = tset + SET_I0TB;
	FloatToSet(ptr->XDL_I0_diff, bval); 	
	
	//小电流电流突变 
	bval = tset + SET_ITB;
	FloatToSet(ptr->XDL_I_diff, bval);		
	
	//小电流接地时间 
	bval = tset + SET_TDXJD;
	FloatToSet(ptr->XDL_Time, bval);		
	
	//小电流接地系数
	bval = tset + SET_XDLCOEF;
	FloatToSet(ptr->XDL_Coef, bval);

	
	return 0;
}

int FaSimWriteSet(int fd, int flag, BYTE *pSet)
{

   	VPrRunInfo *pInfo = prRunInfo+fd;
	int no;
	
	if (g_prInit == 0)
		return ERROR;

	smMTake(prSetSem);

	if(pInfo->set_no == 0)
		no = 1;
	else 
        no = 0;

	memcpy(prRunSet[no], prRunSet[pInfo->set_no], sizeof(VPrRunSet));
	
	switch (flag)
	{
		case PR_DLQ:
		case PR_FHKG:
		case PR_GLBGZ:
		case PR_DLQCH:
		case PR_CKTZ:
		case PR_CKII:
		case PR_BDZCH:
		case PR_DLJSCH:
			WriteConsValueSet(fd,flag,pSet);
			break;
		case PR_DYZ:
			WriteVoltageSet(fd, pSet);
			break;
		case PR_DLJSX:
			WriteCurrentSet(fd, pSet);
			break;
		case PR_ZSYX:
			WriteZsyVoltageSet(fd,pSet);
			break;
		case PR_DYDLX:
			WriteDYDLVoltageSet(fd,pSet);
			break;
		case PR_CKI0TZ:
			WriteI0ValueSet(fd,pSet);
			break;
		default:
			break;
	}

	smMGive(prSetSem);

	caSetBuf = prSet[prRunPublic[fd].prSetnum];
	
	prPubSetConv(fd);

	if (prRunPublic[fd].set_no == 0)
		prRunPublic[fd].set_no = 1;
	else
		prRunPublic[fd].set_no = 0;
	
	caSetBuf = prSet[fd+1];	
	prFdSetConv(fd);

	prFdSetNoSwitch(fd);
    return OK;
}
#endif

int ReadSetLen(int fd, BYTE *pBuf, VPrSetHead* pHead)
{
    struct VFileHead *pFile = (struct VFileHead*)prTableBuf;
    BYTE *pTemp;

	if (g_prInit == 0)
		return 0;
    if (fd < 0) 
	{
	    pHead->type |= PR_SET_ERR;
		memcpy(pBuf, pHead, sizeof(VPrSetHead));
	    return sizeof(VPrSetHead);
    }

	if (pHead->type & PR_SET_DESCRIBE)
	{
	    if (pHead->offset == 0)
	    {
	       if (fd == 0)
		   	  ReadParaFile("ProSetPubTable.cfg", prTableBuf, MAX_TABLE_FILE_SIZE);
		   else
	          ReadParaFile("ProSetTable.cfg", prTableBuf, MAX_TABLE_FILE_SIZE);
		   pHead->total = pFile->nLength - sizeof(struct VFileHead);
	    }	
		pTemp = prTableBuf + sizeof(struct VFileHead);
	}
	else if (pHead->type & PR_SET_PARA)
	{
	    if(pHead->offset == 0)
		{
		    ReadSet(fd, prTableBuf);
			if (fd == 0)
				pHead->total = sizeof(TTABLETYPE) + SET_PUB_NUMBER*sizeof(TSETVAL);
			else
			    pHead->total = sizeof(TTABLETYPE) + SET_NUMBER*sizeof(TSETVAL);
	    }
		pTemp = prTableBuf;

	}
	if((pHead->offset + pHead->len) > pHead->total)
	{
		pHead->len = pHead->total - pHead->offset;
	    pHead->type |= PR_SET_END;
	}
	memcpy(pBuf, pHead, sizeof(VPrSetHead));
	memcpy(pBuf+sizeof(VPrSetHead), pTemp+pHead->offset, pHead->len);

	return (pHead->len+sizeof(VPrSetHead));
}

int WriteSetLen(int fd, BYTE *pBuf, VPrSetHead* pHead)
{
    BYTE *pTemp;
	int len;
	if (g_prInit == 0)
		return 0;

	if (fd < 0) 
	{
	    pHead->type |= PR_SET_ERR;
	    return ERROR;
    }

    pTemp = prTableBuf;
	if (fd == 0)
		len = sizeof(TTABLETYPE) + SET_PUB_NUMBER*sizeof(TSETVAL);
	else
		len = sizeof(TTABLETYPE) + SET_NUMBER*sizeof(TSETVAL);

	if (pHead->total > len)
	{
		pHead->type |= PR_SET_ERR;
		return ERROR;
	}
	if (pHead->type & PR_SET_PARA)
	{
	   memcpy(pTemp+pHead->offset, pBuf, pHead->len);
	   if(pHead->type & PR_SET_END)
	   	  WriteSet(fd, pTemp);
	}
	return pHead->len;
}

#ifdef INCLUDE_EXT_MMI
//wdg TI1==0 default
int WriteMMIPrSet_1(int fd, BYTE *pSet)
{
  
	VMMIPrSet_1 *ptr =(VMMIPrSet_1 *)pSet;
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet; 
	BYTE *wrSet;
	char fname[MAXFILENAME];
	struct VFileHead *head;
	DWORD yb,kg1;
	DWORD len;
	BYTE *ptemp;

	if(g_prInit == 0)
		return -1;

    pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;
	
	smMTake(prSetSem);
	
	bval = tset + SET_T2;
	FloatToSet(ptr->T2Set, bval);

	bval = tset + SET_TI02;
	FloatToSet(ptr->T0Set, bval);
	
	bval = tset + SET_3U0;
	FloatToSet(ptr->U0Set, bval);


	yb = pprRunSet->dYb;
	if ((ptr->I2Set < 10e-6)&&(ptr->I2Set > -10e-6))
		yb &= ~PR_YB_I2;
	else
	{
		yb |= PR_YB_I2;
		bval = tset + SET_I2;
		FloatToSet(ptr->I2Set, bval);
	}
	if ((ptr->U0Set < 10e-6)&&(ptr->U0Set > -10e-6))
		yb &= ~PR_YB_I02;
	else
	{
		yb |= PR_YB_I02;
		bval = tset +SET_3U0 ;
		FloatToSet(ptr->U0Set, bval);
	}
	
	if ((ptr->I0Set < 10e-6)&&(ptr->I0Set > -10e-6))
		yb &= ~PR_YB_I02;
	else
	{
		yb |= PR_YB_I02;
		bval = tset + SET_3I02;
		FloatToSet(ptr->I0Set, bval);
	}
	ReverseKSet(SET_YB, (BYTE*)tset, yb);

	kg1 = pprRunSet->dKG1;	

	if ((ptr->I2Set < 10e-6)&&(ptr->I2Set > -10e-6))

	{
		kg1 &= ~(PR_CFG1_I2_TRIP);
	}

	if ((ptr->I0Set < 10e-6)&&(ptr->I0Set > -10e-6)&&(ptr->U0Set < 10e-6)&&(ptr->U0Set > -10e-6))
	{	
		kg1 &= ~(PR_CFG1_I0_TRIP);
	}

	kg1 &= ~(PR_CFG1_I0_U|PR_CFG1_CD_MODE);
	ReverseKSet(SET_KG1, (BYTE*)tset, kg1);


	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);

	len = sizeof(TTABLETYPE);
	ptemp = wrSet + len;
	memcpy(wrSet, &tSetType, len);
	memcpy(ptemp, (BYTE*)tset, SET_NUMBER*sizeof(TSETVAL));

	caSetBuf = prSet[fd+1];
	prFdSetConv(fd);
	prFdSetNoSwitch(fd);
	
	sprintf(fname, "fd%dset.cfg", fd+1);
	if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return 2;
	}		
	smMGive(prSetSem);
	
	return 0;


}
int WriteMMIPrSet(int fd, BYTE *pSet)
{
    VMMIPrSet *ptr = (VMMIPrSet *)pSet;
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet; 
	BYTE *wrSet;
	char fname[MAXFILENAME];
	struct VFileHead *head;
	DWORD yb,kg1;
	DWORD len;
	BYTE *ptemp;

	if(g_prInit == 0)
		return -1;

    pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;
	
	smMTake(prSetSem);
	bval = tset + SET_T1;
#if (TYPE_USER == USER_BEIJING)
	FloatToSet(ptr->T1Set, bval);
#else
	FloatToSet(0, bval);
#endif
	bval = tset + SET_T2;
	FloatToSet(ptr->T2Set, bval);

#if (TYPE_USER == USER_BEIJING)
	bval = tset + SET_TI02;
	FloatToSet(ptr->T0Set, bval);
#else
	bval = tset + SET_TI01;
	FloatToSet(ptr->T0Set, bval);
#endif	

	yb = pprRunSet->dYb;
	if ((ptr->I1Set < 10e-6)&&(ptr->I1Set > -10e-6))
        yb &= ~PR_YB_I1;
	else
	{
		yb |= PR_YB_I1;
		bval = tset + SET_I1;
		FloatToSet(ptr->I1Set, bval);
	}
	if ((ptr->I2Set < 10e-6)&&(ptr->I2Set > -10e-6))
		yb &= ~PR_YB_I2;
	else
	{
		yb |= PR_YB_I2;
		bval = tset + SET_I2;
		FloatToSet(ptr->I2Set, bval);
	}
#if (TYPE_USER != USER_BEIJING)
	if ((ptr->I0Set < 10e-6)&&(ptr->I0Set > -10e-6))
		yb &= ~PR_YB_I01;
	else
	{
		yb |= PR_YB_I01;
		bval = tset + SET_3I01;
	FloatToSet(ptr->I0Set, bval);
	}
	yb &= ~(PR_YB_I02|PR_YB_I03);
	ReverseKSet(SET_YB, (BYTE*)tset, yb);
#else
	if ((ptr->I0Set < 10e-6)&&(ptr->I0Set > -10e-6))
		yb &= ~PR_YB_I02;
	else
	{
		yb |= PR_YB_I02;
		bval = tset + SET_3I02;
	FloatToSet(ptr->I0Set, bval);
	}
	ReverseKSet(SET_YB, (BYTE*)tset, yb);
#endif

	kg1 = pprRunSet->dKG1;

#if 0
#if (TYPE_USER != USER_BEIJING)
	if (ptr->I0Gj)
	{
		kg1 &= ~PR_CFG1_I0_TRIP;
	}
	else
	{
		kg1 |= PR_CFG1_I0_TRIP;
	}
#endif
	if ((ptr->I1Set < 10e-6)&&(ptr->I1Set > -10e-6))
        kg1 &= ~(PR_CFG1_I1_TRIP );

	if ((ptr->I2Set < 10e-6)&&(ptr->I2Set > -10e-6))
		kg1 &= ~(PR_CFG1_I2_TRIP);

	if ((ptr->I0Set < 10e-6)&&(ptr->I0Set > -10e-6))
		kg1 &= ~(PR_CFG1_I0_TRIP);
#endif

#ifndef _YIERCI_RH_
#ifdef _TEST_VER_
      if((ptr->I1Set > 49) && (ptr->I1Set < 50))
        kg1 |= (PR_CFG1_I1_TRIP);
      if((ptr->I2Set > 49) && (ptr->I2Set < 50))
        kg1 |= (PR_CFG1_I2_TRIP);
#endif
#endif

	kg1 &= ~(PR_CFG1_I0_U|PR_CFG1_CD_MODE);
	ReverseKSet(SET_KG1, (BYTE*)tset, kg1);


	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);

	len = sizeof(TTABLETYPE);
	ptemp = wrSet + len;
	memcpy(wrSet, &tSetType, len);
	memcpy(ptemp, (BYTE*)tset, SET_NUMBER*sizeof(TSETVAL));

	caSetBuf = prSet[fd+1];
	prFdSetConv(fd);
	prFdSetNoSwitch(fd);
	
	sprintf(fname, "fd%dset.cfg", fd+1);
	if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return 2;
	}		
	smMGive(prSetSem);
	
	return 0;

}
#if (TYPE_USER == USER_BEIJING)
int WriteMMIU0Set(int fd,float u0set)
{
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunInfo *pInfo;
	VPrSetPublic* pprPubRunSet;
	BYTE *wrSet;
	char fname[MAXFILENAME];
	struct VFileHead *head;
	DWORD yb;
	DWORD len;
	BYTE *ptemp;
	float u0setold;

	if(g_prInit == 0)
	return -1;

	pInfo = prRunInfo + fd;
	pprPubRunSet = prSetPublic[prRunPublic->set_no];
	smMTake(prSetSem);
	
	bval = tset + SET_3U0;
	SetToFloat(bval,&u0setold);
	if(((u0set > 10e-6)||(u0set < -10e-6)))
		FloatToSet(u0set, bval);
	
	if(u0setold == u0set)
	{
		smMGive(prSetSem);
		return 0;
	}
	
	yb = pprPubRunSet->dYb;
	if ((u0set < 10e-6)&&(u0set > -10e-6))
		yb &= ~PR_YB_U0;
	else
		yb |= PR_YB_U0;
	tset = (TSETVAL*)(prSet[0]);
	ReverseKSet(SET_YB, (BYTE*)tset, yb);
	bval = tset + SET_PUB_U0;
	if((u0set > 10e-6)||(u0set < -10e-6))
		FloatToSet(u0set, bval);
	
	
	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);

	len = sizeof(TTABLETYPE);
	ptemp = wrSet + len;
	memcpy(wrSet, &tSetType, len);
	memcpy(ptemp, (BYTE*)prSet[fd+1], SET_NUMBER*sizeof(TSETVAL));

	caSetBuf = prSet[fd+1];
	prFdSetConv(fd);
	prFdSetNoSwitch(fd);
	
	sprintf(fname, "fd%dset.cfg", fd+1);
	if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return 2;
	}

	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);
	len = sizeof(TTABLETYPE);
	memcpy(wrSet,&tSetType,len);
	memcpy(wrSet+len,prSet[0],SET_NUMBER*sizeof(TSETVAL));
	if (WriteParaFile("fdPubSet.cfg", (struct VFileHead *) prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return 2;
	}
	caSetBuf = prSet[0];
	if(prPubSetConv() != SET_OK) return ERROR;
	if(prRunPublic->set_no == 0)
		prRunPublic->set_no =1;
	else
		prRunPublic->set_no = 0;
	
	smMGive(prSetSem);
	
	return 0;
	
}
#endif	
#endif
#ifdef INCLUDE_FA

int prSwitchFaMod(int pr, int fd, int wdg_flag)
{
    int i;
	static BOOL change[32] = {1};//防止一直读写铁电。
	DWORD dread;
	DWORD dreadagain;
	VPrRunInfo *pInfo;
	VPrRunSet *pprSet;
    VPrSetPublic* pprRunSet;

	if(fd > fdNum)
		return ERROR;
    if(fd > 31)
		return ERROR;

    pprRunSet = prSetPublic[prRunPublic[fd].set_no]+fd;

	pInfo = prRunInfo + fd;
	pprSet = prRunSet[pInfo->set_no]+fd;

#if(TYPE_USER == USER_GUIYANG)
#if(DEV_SP == DEV_SP_WDG)
	//if(pprPubRunSet->bFaSelectMode == PR_FA_CENTRALIZED)  //集中式不切换
	if(GetExtMmiYx(XINEXTMMI_MS))
	{
#else
	if(GetExtMmiYx(EXTMMI_AUTO))  //集中式不切换
	{
#endif
		change[fd] = true;
		return 0;
	}
#endif

	if (wdg_flag)//就地
	{
		if(pr)
			pprSet->bCHNum = 2;
		if(0 == pprRunSet->dYb)
		{	
			if(true == change[fd])
			{
				if(0 == fd)
			    	extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&dread,sizeof(DWORD));
				else
					extNvRamGet(NVRAm_DEV_PUBYB_MULTI+4*(fd-1),(BYTE*)&dread,sizeof(DWORD));
				for(i=0; i<50; i++);
				
				if(0 == fd)
			    	extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&dreadagain,sizeof(DWORD));
				else
					extNvRamGet(NVRAm_DEV_PUBYB_MULTI+4*(fd-1),(BYTE*)&dreadagain,sizeof(DWORD));
				
			    if(dread == dreadagain)
			   	{
					change[fd] = false;
					pprRunSet->dYb = dread;
					   		  
				}
			}
		}
	}
	else //智能型
	{
		change[fd] = true;
		
		if(pr)
			pprSet->bCHNum = 1;
		
		if(pprRunSet->dYb)
		{
			if(0 == fd)
				extNvRamSet(NVRAM_DEV_PUBYB,(BYTE*)&pprRunSet->dYb,sizeof(DWORD));
			else
				extNvRamSet(NVRAm_DEV_PUBYB_MULTI+4*(fd-1),(BYTE*)&pprRunSet->dYb,sizeof(DWORD));
			for(i=0; i<50; i++);

			if(0 == fd)
				extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&dread,sizeof(DWORD));
			else
				extNvRamGet(NVRAm_DEV_PUBYB_MULTI+4*(fd-1),(BYTE*)&dread,sizeof(DWORD));
			
			if(pprRunSet->dYb == dread)//设置异常等待下一个周期重新写。
			{
				pprRunSet->dYb = 0;	
			}
		}
	}

	return 0;

}

#endif
#endif

#ifdef _TEST_VER_
#if(DEV_SP == DEV_SP_DTU)
int BOMAPrset(int fd,WORD on)
{
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet; 
	BYTE *wrSet;
	char fname[MAXFILENAME];
	struct VFileHead *head;
	DWORD yb,kg1;
	DWORD len;
	BYTE *ptemp;
	BOOL  bWrite;
	
		
	 pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;

	smMTake(prSetSem);
	
	kg1 = pprRunSet->dKG1;
	yb = pprRunSet->dYb;

	bWrite = 0;
	if (on == 0)
	{
		if(kg1 & PR_CFG1_I1_TRIP)
		{
       		 kg1 &= ~PR_CFG1_I1_TRIP;
			 bWrite = 1;
		}
		/*if(yb & PR_YB_I1)
		{
			yb &= ~(PR_YB_I1);
			bWrite = 1;
		}*/
	}
	else
	{
		if((kg1 & PR_CFG1_I1_TRIP) == 0)
		{
			kg1 |= PR_CFG1_I1_TRIP;
			bWrite = 1;
		}
		/*if((yb & PR_YB_I1) == 0)
		{
			yb |= PR_YB_I1;
			bWrite = 1;
		}*/
	}

	if (on == 0)
	{
		if(kg1 & PR_CFG1_I2_TRIP)
		{
			kg1 &= ~PR_CFG1_I2_TRIP;
			bWrite = 1;
		}
		/*if(yb & PR_YB_I2)
		{
			yb &= ~(PR_YB_I2);
			bWrite = 1;
		}*/
	}
	else
	{
		if((kg1 & PR_CFG1_I2_TRIP) == 0)
		{
			kg1 |= PR_CFG1_I2_TRIP;
			bWrite = 1;
		}
		/*if((yb & PR_YB_I2) == 0)
		{
			yb |= PR_YB_I2;
			bWrite = 1;
		}*/
	}
	if(bWrite == 0)
	{
		smMGive(prSetSem);
		return 0;
	}
	
	ReverseKSet(SET_KG1, (BYTE*)tset, kg1);
	ReverseKSet(SET_YB, (BYTE*)tset, yb);

	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);

	len = sizeof(TTABLETYPE);
	ptemp = wrSet + len;
	memcpy(wrSet, &tSetType, len);
	memcpy(ptemp, (BYTE*)tset, SET_NUMBER*sizeof(TSETVAL));

	caSetBuf = prSet[fd+1];
	prFdSetConv(fd);
	prFdSetNoSwitch(fd);
	
	sprintf(fname, "fd%dset.cfg", fd+1);
	if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return 2;
	}		
	smMGive(prSetSem);

	return 0;

}
#endif
#endif

#if defined(_YIERCI_RH_) ||(TYPE_USER == USER_GUIYANG)
// 1为 设置就地(分段)，0 为集中型(保护); 
int SetFaTypePrset(BYTE on) 
{
	TSETVAL *tset = (TSETVAL*)(prSet[0]);
	DWORD yb_kg1_kg2 = 0;
	DWORD yb_read,yb_readagain;
	VPrSetPublic *pprSetPublic;
	struct VFileHead *head;
	BYTE *wrSet;
	DWORD len;
	int i;
	pprSetPublic = prSetPublic[prRunPublic->set_no];
    if(g_prInit == 0)
		return -1;
    
	yb_kg1_kg2 = pprSetPublic->dKG2;
	len = yb_kg1_kg2;
	yb_kg1_kg2 &= ~PR_CFG2_CONTROL_MODE; 
	
	if(on)
	{
		yb_kg1_kg2 |= PR_MODE_SEG;
	}
	else 
	{
		yb_kg1_kg2 |= PR_MODE_PR;
	}
	
	if(len == yb_kg1_kg2) //与之前一样返回
	{
		return OK;
	}

	if(on)//就地
	{
	    extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&yb_read,sizeof(DWORD));
		for(i=0; i<50; i++);
	    extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&yb_readagain,sizeof(DWORD));

	    if(yb_read == yb_readagain)
	   		ReverseKSet(SET_YB, (BYTE*)tset, yb_read);
		else
			return ERROR;
	}
	else//集中
	{
		
		extNvRamSet(NVRAM_DEV_PUBYB,(BYTE*)&pprSetPublic->dYb,sizeof(DWORD));
		for(i=0; i<50; i++);
		extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&yb_read,sizeof(DWORD));
	
		if(pprSetPublic->dYb != yb_read)//设置异常等待下一个周期重新写。
			return ERROR;
	}
	
	ReverseKSet(SET_KG2, (BYTE*)tset, yb_kg1_kg2);

	caSetBuf = prSet[0];
	if(prPubSetConv(0) != SET_OK) return ERROR;
	
	smMTake(prSetSem);

	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);
	len = sizeof(TTABLETYPE);
	memcpy(wrSet,&tSetType,len);
	memcpy(wrSet+len,prSet[0],SET_PUB_NUMBER*sizeof(TSETVAL));
	
	if(WriteParaFile("fdPubSet.cfg",(struct VFileHead*)prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return ERROR;
	}
	smMGive(prSetSem);
	if(prRunPublic->set_no == 0)
		prRunPublic->set_no =1;
	else
		prRunPublic->set_no = 0;
	return OK;				
}
#endif
extern int fdNum;
int ReadSetName(int fd, BYTE * pName)
{
    WORD iofd;
#if (TYPE_IO == IO_TYPE_MASTER)
	struct VExtIoConfig *pCfg;
#endif
	int ret = OK;	
		
	if (fd < 0) return ERROR;

	memset(pName, 0, PR_SET_NAME);

    if ((fd == 0)|| (fd > fdNum))
		strcpy((char*)pName, "ProSetPubTable.cfg");
	else if (GetMyIoNo(4, (WORD)(fd-1), &iofd) == OK)
		strcpy((char*)pName, "ProSetTable.cfg");
#if (TYPE_IO == IO_TYPE_MASTER)
	else
	{
	    pCfg = GetExtIoNo(4, (WORD)(fd-1), &iofd);
		if (pCfg == NULL)
			return ERROR;
		else
		    sprintf((char*)pName, "ProSetTable%d.cfg", pCfg->pAddrList->ExtIoAddr.wAddr);
	}
#endif
	return ret;

}

int ReadSetNum(int fd)
{
    WORD iofd;
    if (fd < 0) return ERROR;

	if ((fd == 0) || (fd > fdNum))
	{
	    if (GetMyIoNo(4, 0, &iofd) == OK)
	#ifdef INCLUDE_PR
		   return SET_PUB_NUMBER;
	#else
       return 0;
	#endif
	}

	if (GetMyIoNo(4, fd-1, &iofd) == OK)
	#ifdef INCLUDE_PR
		return SET_NUMBER;
   #else
	   return 0;
	#endif

	return ERROR;
}

//公共保护参数只在第一块带保护的板中
int ReadPubSet(BYTE *pSet,int fd)
{
    WORD iofd;
#if (TYPE_IO == IO_TYPE_MASTER)
	struct VExtIoConfig *pCfg;
	VExtIoCmd *pCmd = (VExtIoCmd *)g_Sys.byIOCmdBuf;
	BYTE *pData = (BYTE *)(pCmd+1);
#endif
    int ret = OK;

    if (GetMyIoNo(4, 0, &iofd) == OK)
    {
#ifdef INCLUDE_PR
//       if (g_prInit == 0)
//		   return ERROR;
       memcpy(pSet, &tSetType, sizeof(TTABLETYPE));
	   memcpy(pSet+sizeof(TTABLETYPE), prSet[prRunPublic[fd].prSetnum], SET_PUB_NUMBER*sizeof(TSETVAL));
#endif
    }
#if (TYPE_IO == IO_TYPE_MASTER)	
    else
    {
	   pCfg = GetExtIoNo(4, 0, &iofd);
	   if (pCfg == NULL)
		   return ERROR;
	   else
	   {
		   smMTake(g_Sys.dwExtIoDataSem);
           pCmd->addr = pCfg->pAddrList->ExtIoAddr.wAddr;
		   pCmd->cmd = EXTIO_CMD_PRSET_READ;
		   pCmd->len = sizeof(BYTE);
		   *pData = (BYTE)0;
		   ret = ExtIoCmd(NULL, NULL, SECTOTICK(2));
		   if (ret == OK)
			   memcpy(pSet, pData+1,PR_SET_SIZE);
		   smMGive(g_Sys.dwExtIoDataSem);			
	   }
   }
#endif
	 return ret;
}

int WritePubSet(BYTE *pSet,int fd)
{
    WORD iofd;
#ifdef INCLUDE_PR
	struct VFileHead *head;
#endif

    int ret = OK;

	if (GetMyIoNo(4, 0, &iofd) == OK)
	{	
#ifdef INCLUDE_PR
 //       if (g_prInit == 0)
//		   return ERROR;
	    caSetBuf  = pSet + sizeof(TTABLETYPE);

		if (prPubSetConv(fd) != SET_OK) return 1;	

		head = (struct VFileHead *)prSetTmp;	
		head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
		head->wVersion = CFG_FILE_VER;
		head->wAttr = CFG_ATTR_NULL;
		memcpy((head+1), pSet, PR_SET_SIZE);
	
		if (WriteParaFile("fdPubSet.cfg", (struct VFileHead *) prSetTmp) == ERROR)
		{
			return 2;
		}	
		memcpy(prSet[prRunPublic[fd].prSetnum], pSet+sizeof(TTABLETYPE), SET_PUB_NUMBER*sizeof(TSETVAL));
	
		if (prRunPublic[fd].set_no == 0)
			prRunPublic[fd].set_no = 1;
		else
			prRunPublic[fd].set_no = 0;
#endif
	}
	return ret;
}

int ReadSet(int fd, BYTE *pSet)
{
    WORD iofd;
    int ret = OK;	

 		
	if (fd < 0) return ERROR;

	memset(pSet, 0, PR_SET_SIZE);

	if (fd == 0)
       ReadPubSet(pSet,0); 
	else if(fd > fdNum)
	   ReadPubSet(pSet,fd-fdNum);
	else if (GetMyIoNo(4, (WORD)(fd-1), &iofd) == OK)
	{	
#ifdef INCLUDE_PR
  //      if (g_prInit == 0)
	//	   return ERROR;
        memcpy(pSet, &tSetType, sizeof(TTABLETYPE));
	    memcpy(pSet+sizeof(TTABLETYPE), prSet[iofd+1], SET_NUMBER*sizeof(TSETVAL));
#endif
	}	
	return ret;
}

/* 0   成功
   非0 错误
    -1 回线号错误
     1 定值非法
     2 写定值文件错误
*/     
int WriteSet(int fd, BYTE *pSet)
{
#ifdef INCLUDE_PR
	struct VFileHead *head;
	char fname[MAXFILENAME];
#endif
    WORD iofd;
    int ret = OK;	
  
	if (fd < 0) return ERROR;

	if (fd == 0)
		WritePubSet(pSet,0);
	else if(fd > fdNum)
		WritePubSet(pSet,fd-fdNum);
	else if(GetMyIoNo(4, (WORD)(fd-1), &iofd) == OK)
	{	
#ifdef INCLUDE_PR
	    caSetBuf  = pSet + sizeof(TTABLETYPE);

		if (prFdSetConv(iofd) != SET_OK) return 1;	

		head = (struct VFileHead *)prSetTmp;	
		head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
		head->wVersion = CFG_FILE_VER;
		head->wAttr = CFG_ATTR_NULL;
		memcpy((head+1), pSet, PR_SET_SIZE);
	
		sprintf(fname, "fd%dset.cfg", iofd+1);
		if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
		{
			return 2;
		}		
		
		memcpy(prSet[iofd+1], pSet+sizeof(TTABLETYPE), SET_NUMBER*sizeof(TSETVAL));
	
		prFdSetNoSwitch(iofd);
#endif
	}
	return ret;
}



void WritePrFile(int no)
{
#ifdef INCLUDE_PR
		struct VFileHead *head;
		char fname[MAXFILENAME];
		BYTE *wrSet;
		WORD len;
		
		head = (struct VFileHead *)prSetTmp;	
		head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
		head->wVersion = CFG_FILE_VER;
		head->wAttr = CFG_ATTR_NULL;
		wrSet = (BYTE*)(head+1);
		len = sizeof(TTABLETYPE);
		memcpy(wrSet,&tSetType,len);
		memcpy(wrSet+len,prSet[no],SET_NUMBER*sizeof(TSETVAL));
		
		if(no == 0)
		{
			if (WriteParaFile("fdPubSet.cfg", (struct VFileHead *) prSetTmp) == ERROR)
			{
				return ;
			}
		}
		else if(no > fdNum)
		{
			sprintf(fname, "fd%02dPubSet.cfg",no-fdNum+1);
			if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
			{
				return ;
			}
		}
		else
		{
			sprintf(fname, "fd%dset.cfg",no);
			if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
			{
				return ;
			}	
		}
#endif
}

#if (TYPE_USER != USER_FUJIAN)
BOOL b_YB_CHN = 0;
WORD w_JD_MODE = 0;	
int ReadPrPara(WORD parano,char *pbuf,WORD type)
{
#if 0
//#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	WORD fdno,lineno;
	BOOL bvalue;
	WORD svalue;
	DWORD dvalue; 
	float fvalue;
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	VPrSetPublic  *pprPubRunSet;
	TSETVAL *tset,*tpubset;
	TSETVAL *bval;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	bvalue = svalue = fvalue = dvalue = 0;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return ERROR;
	
	if(parano < PRPARA_LINE_ADDR)
	{
		fdno = 0;
		pInfo = prRunInfo + fdno;
		pprRunSet = prRunSet[pInfo->set_no] + fdno;
		tset = (TSETVAL*)(prSet[fdno+1]);

		pprPubRunSet = prSetPublic[prRunPublic->set_no];
		tpubset = (TSETVAL*)prSet[0];

		parano = parano - PRPARA_ADDR;
			
		switch(parano)
		{
			case PRP_YB_LEDKEEP:  //故障灯自动复归
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bLedZdFg)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			
				break;
			case PRP_T_LEDKEEP: //故障灯自动复归时间
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_LED_TKEEP;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_T_YXKEEP: //故障遥信保持时间
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_TKEEP;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			
				break;
			case PRP_YB_FTUSD:	//首端FTU投入
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprPubRunSet->bFTUSD)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);

				break;
			case PRP_T_X:	//X
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tpubset + SET_PUB_TX;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);

				break;
			case PRP_T_Y:  //Y
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tpubset + SET_PUB_TY;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_T_C:	//C
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tpubset + SET_PUB_TC;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_T_S:  //S
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tpubset + SET_PUB_TS;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;

			case PRP_T_DXJD:  //单相接地跳闸时间
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_TDXJD;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;

			case PRP_T_HZ: //选线跳闸重合时间定值
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tpubset + SET_PUB_TXXCH;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_YB_ZSYI:	//自适应相间短路故障处理
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprPubRunSet->bXJDL)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_YB_ZSYI0:  //自适应单相接地故障处理
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprPubRunSet->bDXJD)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_YB_CH1:  //一次重合闸投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->dYb & PR_YB_CH)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_T_CH1:  //一次重合时间
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_TCH1;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			
				break;
			case PRP_YB_IH:	//大电流闭锁重投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bDdlBsch)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_I_IH:  //大电流闭重定值
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_CHBSDL;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case PRP_YB_CHN: // 多次重合闸投退
				
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;

				bval = tset + SET_CHNUM;
				SetToFloat(bval,&fvalue);
				svalue = (WORD)fvalue;
				
				if(b_YB_CHN)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_I_CHN: // 重合闸次数
				fParaInfo->type = USHORT_TYPE;
				fParaInfo->len = USHORT_LEN;
				bval = tset + SET_CHNUM;
				SetToFloat(bval,&fvalue);
				svalue = (WORD)fvalue;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				
				break;
			case PRP_T_CHN: // 非第一次重合闸延时
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_TCH2;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_YB_JDMODE: // 系统接地方式
				fParaInfo->type = USHORT_TYPE;
				fParaInfo->len = USHORT_LEN;
				svalue = w_JD_MODE;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				
				break;
			case PRP_YB_FDFJ:  // 分段或分界
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprPubRunSet->byWorkMode == PR_MODE_FJ)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);

				break;
			case PRP_YB_FDLL: // 联络分段模式
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprPubRunSet->byWorkMode == PR_MODE_SEG)
					bvalue = 0;
				else
					bvalue = 1;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_YB_JDFA: // 就地型FA 模式
				fParaInfo->type = USHORT_TYPE;
				fParaInfo->len = USHORT_LEN;
				if(pprPubRunSet->bFaWorkMode == PR_FA_ZSY)
					svalue = 0;
				else if(pprPubRunSet->bFaWorkMode == PR_FA_U)
					svalue = 1;
				else if(pprPubRunSet->bFaWorkMode == PR_FA_I)
					svalue = 2;
				else 
					svalue = 1;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				
				break;
			default:
				fParaInfo->type = 4;
				fParaInfo->len = 0;
				svalue = 0;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				break;
		}
	}
	else if(parano >= PRPARA_LINE_ADDR)
	{
		if(parano > (PRPARA_LINE_ADDR + PRPARA_LINE_NUM*fdNum -1))
			return ERROR;
			
		fdno = (parano - PRPARA_LINE_ADDR)/PRPARA_LINE_NUM;
		lineno = parano - PRPARA_LINE_ADDR - fdno*PRPARA_LINE_NUM;
		
		pInfo = prRunInfo + fdno;
		pprRunSet = prRunSet[pInfo->set_no] + fdno;
		tset = (TSETVAL*)(prSet[fdno+1]);

		switch(lineno)
		{
			case PRP_YB_GLTRIP://过流停电跳闸投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bITrip == 0)
					bvalue = 0;
				else
					bvalue = 1;
			
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			  break;
			case PRP_YB_I1TT://过流一段告警投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->dYb & PR_YB_I1)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
			case PRP_YB_I1TRIP://过流一段出口投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if((pprRunSet->bI1Trip) && (pprRunSet->dYb & PR_YB_I1))
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
			case PRP_I_I1://过流一段定值
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_I1;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			
				break;
			case PRP_T_I1://过流一段时间

				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_T1;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);

				break;	
			case PRP_YB_I2TT://过流二段投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->dYb & PR_YB_I2)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);

				break;
			case PRP_YB_I2TRIP://过流二段出口
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if((pprRunSet->bI2Trip) &&(pprRunSet->dYb & PR_YB_I2))
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_I_I2://过流二段定值
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_I2;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			
				break;
			case PRP_T_I2://过流二段时间

				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_T2;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_YB_I0TT://零流过流投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->dYb & PR_YB_I01)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_YB_I0TRIP://零流过流出口投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if((pprRunSet->bI0Trip)&&(pprRunSet->dYb & PR_YB_I01))
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_I_I0://零流过流定值
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_3I01;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			
				break;
			case PRP_T_I0://零流过流时间
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_TI01;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);	
				break;
				
			case PRP_YB_ILTT:   //小电流接地投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				bvalue = 1;
				if(pprRunSet->bXDLYB)
					bvalue = 1;
				else
					bvalue = 0;	
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
				
			case PRP_YB_ILTRIP://小电流接地出口投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bXDLTrip && pprRunSet->bXDLYB)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
			case PRP_YB_FZDBS: // 非遮断电流闭锁投退
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bfZdbh)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
			case PRP_I_FZDBS: // 非遮断电流定值
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_IKG;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
#if 0
			case PRP_YB_GLLB:  //过流录波
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bRcdGl)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);

				break;
			case PRP_YB_SYLB:  //失压录波
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bRcdSy)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);

				break;
			case PRP_YB_U0LB:   //零序电压录波
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bRcdU0)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_YB_I0LB:  //零序电流突变录波
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bRcdI0TB)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
#endif
			default://备用
				fParaInfo->type = 4;
				fParaInfo->len = 0;
				svalue = 0;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				break;
			
		}
	}

	return OK;
#else
	return ERROR;
#endif
}

/*0 成功
1 信息体地址错误
2 数据类型、长度错误
3 值范围错误
4 不配
*/
int WritePrParaYZ(WORD parano,char *pbuf)
{
#if 0
//#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	WORD fdno,lineno;
	WORD svalue;
	float fvalue;
	DWORD dvalue;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	svalue = fvalue = dvalue =0;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	if(parano < PRPARA_LINE_ADDR)
	{
		parano = parano - PRPARA_ADDR;
		
		switch(parano)
		{
			case PRP_YB_LEDKEEP:  //故障灯自动复归
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
			
				break;
			case PRP_T_LEDKEEP: //故障灯自动复归时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 86400)
					return 3;
				
				break;
			case PRP_T_YXKEEP: //故障遥信保持时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 300)
					return 3;
			
				break;
			case PRP_YB_FTUSD:	//首端FTU投入

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_T_X:	//X
				
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				break;
			case PRP_T_Y:  //Y
				
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;

				
				break;
			case PRP_T_C:	//C

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 6000)
					return 3;
				
				break;
			case PRP_T_S:  //S
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 6000)
					return 3;
				
				break;

			case PRP_T_DXJD:  //单相接地跳闸时间

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				break;
				
			case PRP_T_HZ: //选线跳闸重合时间定值
				
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;

				break;
			case PRP_YB_ZSYI:	//自适应相间短路故障处理

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_ZSYI0:  //自适应单相接地故障处理

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_CH1:  //一次重合闸投退

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_T_CH1:  //一次重合时间

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue < 0.01) || ((fvalue) > 600))
					return 3;
			
				break;
			case PRP_YB_IH:	//大电流闭锁重投退
			
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_I_IH:  //大电流闭重定值

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
			
				break;
			case PRP_YB_CHN:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_I_CHN:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				
				break;
			case PRP_T_CHN:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue < 0) || ((fvalue*1000) > 100000))
					return 3;
				break;
			case PRP_YB_JDMODE:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				break;
			case PRP_YB_FDFJ:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_YB_FDLL:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_YB_JDFA:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				break;
			default:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
		}
	}
	else if(parano >= PRPARA_LINE_ADDR)
	{
		if(parano > (PRPARA_LINE_ADDR + PRPARA_LINE_NUM*fdNum -1))
			return 1;
			
		fdno = (parano - PRPARA_LINE_ADDR)/PRPARA_LINE_NUM;
		lineno = parano -PRPARA_LINE_ADDR - fdno*PRPARA_LINE_NUM;

		switch(lineno)
		{
			case PRP_YB_GLTRIP://过流停电跳闸投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
			  break;
			case PRP_YB_I1TT://过流一段投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_I1TRIP://过流一段出口投退
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_I_I1://过流一段定值
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
			
				break;
			case PRP_T_I1://过流一段时间

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				
				if((fvalue*1000) > 100000)
					return 3;
				break;	
			case PRP_YB_I2TT://过流二段TT
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;

				break;
			case PRP_YB_I2TRIP://过流二段出口
				if  ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_I_I2://过流二段定值
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
			
				break;
			case PRP_T_I2://过流二段时间

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue*1000) > 100000)
					return 3;
				break;
			case PRP_YB_I0TT://零流过流投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_I0TRIP://零流过流出口投退
				if  ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_I_I0://零流过流定值
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 10) || ((fvalue*1000) > 150000))
					return 3;
			
				break;
			case PRP_T_I0://零流过流时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue*1000) > 100000)
					return 3;
			
				break;
			case PRP_YB_ILTT://小电流接地投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_ILTRIP://小电流接地出口投退
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_FZDBS:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_I_FZDBS:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 10) || ((fvalue*1000) > 150000))
					return 3;

				break;
#if 0
		      case PRP_YB_GLLB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_YB_SYLB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_YB_U0LB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_YB_I0LB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
#endif
			default://备用
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
		}
	}
	return OK;
#else
	return ERROR;
#endif
}

/*0 成功
1 信息体地址错误
2 数据类型、长度错误
3 值范围错误
4 不配
5 写失败
*/
int WritePrPara(WORD parano,char *pbuf,WORD type)
{
#if 0
//#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	WORD fdno,lineno;
	BOOL bvalue;
	WORD svalue;
	DWORD dvalue;
	float fvalue;
	TSETVAL *tset,*tpubset;
	TSETVAL *bval;
	DWORD yb_kg1_kg2;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	bvalue = svalue = fvalue = dvalue = 0;
	
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	if(parano < PRPARA_LINE_ADDR)
	{
		fdno = 0;
		tset = (TSETVAL*)(prSet[fdno+1]);
		tpubset = (TSETVAL*)prSet[0];
		
		parano = parano - PRPARA_ADDR;
		switch(parano)
		{
			case PRP_YB_LEDKEEP:  //故障灯自动复归
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_LED_FG;
				else
					yb_kg1_kg2 &= ~PR_CFG1_LED_FG;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_T_LEDKEEP: //故障灯自动复归时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 86400)
					return 3;
				bval = tset + SET_LED_TKEEP;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_T_YXKEEP: //故障遥信保持时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 300)
					return 3;
				bval = tset + SET_TKEEP;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_YB_FTUSD:	//首端FTU投入
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG1,&yb_kg1_kg2,PR_SET_PUB);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_FTU_SD;
				else 
					yb_kg1_kg2 &= ~PR_CFG1_FTU_SD;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				
				break;
			case PRP_T_X:	//X
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				bval = tpubset + SET_PUB_TX;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_T_Y:  //Y
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				bval = tpubset + SET_PUB_TY;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_T_C:	//C
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 6000)
					return 3;
				
				bval = tpubset + SET_PUB_TC;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_T_S:  //S
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 6000)
					return 3;
				
				bval = tpubset + SET_PUB_TS;
				FloatToSet(fvalue,bval);
				
				break;
				
			case PRP_T_DXJD:  //单相接地跳闸时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				bval = tset + SET_TDXJD;
				FloatToSet(fvalue,bval);
				
				break;
				
			case PRP_T_HZ: //选线跳闸重合时间定值

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				bval = tpubset + SET_PUB_TXXCH;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_YB_ZSYI:	//自适应相间短路故障处理
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG1,&yb_kg1_kg2,PR_SET_PUB);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_ZSY_XJDL;
				else 
					yb_kg1_kg2 &= ~PR_CFG1_ZSY_XJDL;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_YB_ZSYI0:  //自适应单相接地故障处理

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG1,&yb_kg1_kg2,PR_SET_PUB);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_ZSY_DXJD;
				else 
					yb_kg1_kg2 &= ~PR_CFG1_ZSY_DXJD;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_YB_CH1:  //一次重合闸投退

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_CH;
				else
					yb_kg1_kg2 &= ~PR_YB_CH;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				
				
				break;
			case PRP_T_CH1:  //一次重合时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue < 0.01) || (fvalue > 600))
					return 3;
				bval = tset + SET_TCH1;
				FloatToSet(fvalue,bval);
			
				break;
			case PRP_YB_IH:	//大电流闭锁重投退
				
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_CH_DDLBS;
				else
					yb_kg1_kg2 &= ~PR_CFG1_CH_DDLBS;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_I_IH:  //大电流闭重定值

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
				bval = tset + SET_CHBSDL;
				FloatToSet(fvalue,bval);
			
				break;
			case PRP_YB_CHN:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_CH;
				b_YB_CHN = bvalue;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_I_CHN:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(svalue > 3)
					svalue = 3;
				
				fvalue = svalue;
				bval = tset + SET_CHNUM;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_T_CHN:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue < 0) || ((fvalue*1000) > 100000))
					return 3;
				
				bval = tset + SET_TCH2;
				FloatToSet(fvalue,bval);
				bval = tset + SET_TCH3;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_YB_JDMODE:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				w_JD_MODE = svalue;
				if(w_JD_MODE <2) // 0\1 将小电流投上
				{
					caSetBuf = prSet[fdno+1];
					CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_CFG2_XDL_YB;
					ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);	
				}
				else if(w_JD_MODE == 2) // 2 
				{
					caSetBuf = prSet[fdno+1];
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				
				break;
			case PRP_YB_FDFJ:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_PUB);
				yb_kg1_kg2 &= ~PR_CFG2_CONTROL_MODE;
				if(bvalue)
					yb_kg1_kg2 |= PR_MODE_FJ;
				else
					yb_kg1_kg2 &= ~PR_CFG2_CONTROL_MODE;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_YB_FDLL:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_PUB);
				yb_kg1_kg2 &= ~PR_CFG2_CONTROL_MODE;
				if(bvalue)
					yb_kg1_kg2 |= PR_MODE_TIE;
				else
					yb_kg1_kg2 &= ~PR_CFG2_CONTROL_MODE;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				caSetBuf = prSet[0];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_PUB);
				if(bvalue)
				{
					yb_kg1_kg2 |= PR_YB_SUSY;
					yb_kg1_kg2 &= ~PR_YB_SUDY;
				}
				else
				{
					yb_kg1_kg2 |= PR_YB_SUDY;
					yb_kg1_kg2 &= ~PR_YB_SUSY;
				}
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_YB_JDFA:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_PUB);
				yb_kg1_kg2 &= ~PR_CFG2_FA_MODE;
				if(bvalue == 0)
					yb_kg1_kg2 |= (PR_FA_ZSY << KG_3);
				else if(bvalue == 1)
					yb_kg1_kg2 |= (PR_FA_U << KG_3);
				else if(bvalue == 2)
					yb_kg1_kg2 |= (PR_FA_I << KG_3);
				else
					yb_kg1_kg2 &= ~PR_CFG2_FA_MODE;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				break;
			default:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
		}
	}
	else if(parano >= PRPARA_LINE_ADDR)
	{
		if(parano > (PRPARA_LINE_ADDR + PRPARA_LINE_NUM*fdNum -1))
			return 1;
			
		fdno = (parano - PRPARA_LINE_ADDR)/PRPARA_LINE_NUM;
		lineno = parano - PRPARA_LINE_ADDR - fdno*PRPARA_LINE_NUM;
		tset = (TSETVAL*)(prSet[fdno + 1]);

		switch(lineno)
		{
			case PRP_YB_GLTRIP://过流停电跳闸投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= (1 << KG_17);
				else
					yb_kg1_kg2 &= ~PR_CFG1_TRIP_MODE;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
			  break;

			case PRP_YB_I1TT://过流一段投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_I1;
				else
					yb_kg1_kg2 &= ~PR_YB_I1;
		
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);

				if(bvalue == 0)
				{
					CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_CFG1_I1_TRIP;
					ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				}

	     /*			                                      // gysh
				if((yb_kg1_kg2 & PR_CFG1_I1_WARN)|(yb_kg1_kg2 & PR_CFG1_I1_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I1;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I1;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
					*/
				break;
			case PRP_YB_I1TRIP://过流一段出口投退
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I1_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I1_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				
				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		
					yb_kg1_kg2 |= PR_YB_I1;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				
			/*	                                           // gysh
				if((yb_kg1_kg2 & PR_CFG1_I1_WARN)|(yb_kg1_kg2 & PR_CFG1_I1_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I1;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I1;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				*/
				break;
			case PRP_I_I1://过流一段定值
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
				
				bval = tset + SET_I1;
				FloatToSet(fvalue,bval);
				break;
			case PRP_T_I1://过流一段时间

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue*1000) > 100000)
					return 3;
				bval = tset + SET_T1;
				FloatToSet(fvalue,bval);
			
				break;	
			case PRP_YB_I2TT://过流二段投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
  
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_I2;
				else
					yb_kg1_kg2 &= ~PR_YB_I2;
		
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);

				if(bvalue == 0)
				{
					CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_CFG1_I2_TRIP;
					ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				}
			/*	
				if((yb_kg1_kg2 & PR_CFG1_I2_WARN)|(yb_kg1_kg2 & PR_CFG1_I2_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I2;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I2;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
           */
				break;
			case PRP_YB_I2TRIP://过流二段出口
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I2_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I2_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		
					yb_kg1_kg2 |= PR_YB_I2;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
			/*	                                         // gysh
				if((yb_kg1_kg2 & PR_CFG1_I2_WARN)|(yb_kg1_kg2 & PR_CFG1_I2_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I2;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I2;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				*/
				break;
			case PRP_I_I2://过流二段定值
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
				
				bval = tset + SET_I2;
				FloatToSet(fvalue,bval);
			
				break;
			case PRP_T_I2://过流二段时间

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue*1000) > 100000)
					return 3;
				bval = tset + SET_T2;
				FloatToSet(fvalue,bval);
			
				break;
			case PRP_YB_I0TT://零流过流投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_I01;
				else
					yb_kg1_kg2 &= ~PR_YB_I01;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);

				if(bvalue == 0)
				{
					CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_CFG1_I0_TRIP;
					ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				}

		/*		
				if((yb_kg1_kg2 & PR_CFG1_I0_WARN)|(yb_kg1_kg2 & PR_CFG1_I0_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
	   */	
				break;
			case PRP_YB_I0TRIP://零流过流出口投退
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I0_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I0_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		
					yb_kg1_kg2 |= PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
	   /*	
				if((yb_kg1_kg2 & PR_CFG1_I0_WARN)|(yb_kg1_kg2 & PR_CFG1_I0_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
		*/		
				break;
			case PRP_I_I0://零流过流定值
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 10) || ((fvalue*1000) > 150000))
					return 3;
				bval = tset + SET_3I01;
				FloatToSet(fvalue,bval);	
				break;
			case PRP_T_I0://零流过流时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue*1000) > 100000)
					return 3;	
				bval = tset + SET_TI01;
				FloatToSet(fvalue,bval);
				break;

			case PRP_YB_ILTT://小电流接地告警投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_XDL_YB;
				else
					yb_kg1_kg2 &= ~PR_CFG2_XDL_YB;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				if(bvalue == 0)
				{
					CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_CFG2_XDL_TRIP;
					ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				}
				
				break;

			case PRP_YB_ILTRIP://小电流接地出口投退
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_XDL_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG2_XDL_TRIP;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				if(bvalue)
				{
					CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_CFG2_XDL_YB;
					ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				}
				
				break;
			case PRP_YB_FZDBS:	
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_FZD_BH;
				else
					yb_kg1_kg2 &= ~PR_CFG1_FZD_BH;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_I_FZDBS:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 10) || ((fvalue*1000) > 150000))
					return 3;
				bval = tset + SET_IKG;
				FloatToSet(fvalue,bval);
				
				break;
#if 0
			case PRP_YB_GLLB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_RCD_GL;
				else
					yb_kg1_kg2 &= ~PR_CFG2_RCD_GL;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				break;
				
			case PRP_YB_SYLB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_RCD_SY;
				else
					yb_kg1_kg2 &= ~PR_CFG2_RCD_SY;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				break;
				
			case PRP_YB_U0LB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_RCD_U0;
				else
					yb_kg1_kg2 &= ~PR_CFG2_RCD_U0;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				break;
				
			case PRP_YB_I0LB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_RCD_I0TB;
				else
					yb_kg1_kg2 &= ~PR_CFG2_RCD_I0TB;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				break;
#endif
			default://备用
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				bvalue = 1;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
		}
	}
	return OK;
#else
	return ERROR;
#endif
}
#else
#if(DEV_SP != DEV_SP_DTU)
int ReadPrPara(WORD parano,char *pbuf,WORD type)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	BOOL bvalue;
	WORD svalue;
	float fvalue;
	
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	VPrSetPublic  *pprPubRunSet;
	TSETVAL *tset;
	TSETVAL *bval;
	VFdCfg *pfdcfg;
	bvalue = svalue = fvalue = 0;
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return ERROR;
	pInfo = prRunInfo;
	pprRunSet = prRunSet[pInfo->set_no];
	tset = (TSETVAL*)(prSet[1]); //就取第一回线，福建ftu；dtu未定义暂时不考虑
	pfdcfg = g_Sys.MyCfg.pFd;
	pprPubRunSet = prSetPublic[prRunPublic->set_no];	
	switch(parano)
	{
		case 0x8228: //过流一段投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((pprRunSet->bI1Trip) && (pprRunSet->dYb & PR_YB_I1)) //压板及出口都投才为跳闸，否则为告警
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8229: //过流一段定值 A
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I1;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x822A: //过流一段时间  S，国网为ms
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_T1;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x822B: //过流一段经低压闭锁  压板
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bIDYBS) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x822C: //过流一段经方向闭锁   
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bIDir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x822D: //过流二段投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((pprRunSet->bI2Trip) &&(pprRunSet->dYb & PR_YB_I2))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x822E: //过流二段定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I2;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x822F: //过流二段时间 s
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_T2;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8230: //过流二段经低压闭锁
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI2DYBS) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8231: //过流二段经方向闭锁
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI2Dir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8232: //过流后加速定值 A
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_IHJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8233: //过流后加速时间 s
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8234: //过负荷告警定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_IGFH;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8235: //过负荷告警时间
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGFH;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8236: //零序一段投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((pprRunSet->bI0Trip)&&(pprRunSet->dYb & PR_YB_I01))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8237:  //零序一段定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_3I01;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8238:  //零序一段时间
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TI01;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8239:   //零序Ⅰ段经方向闭锁
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI0Dir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x823A:  //零序后加速定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I0HJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x823B:  //零序后加速时间
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I0TJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x823C:  //重合闸投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_CH)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x823D:  //重合闸检无压投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
#ifdef INCLUDE_PR_PRO			
			if(pprPubRunSet->bTqMode & PR_CH_MODE_WY)
				bvalue = 1;
			else
				bvalue = 0;
#else
			bvalue = 0;
#endif			
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x823E:  //重合闸检同期投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
#ifdef INCLUDE_PR_PRO			
			if(pprPubRunSet->bTqMode &  PR_CH_MODE_TQ)
				bvalue = 1;
			else
				bvalue = 0;
#else
			bvalue = 0;
#endif
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x823F:  //重合闸时间
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TCH1;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8240:  // 小电流接地告警投退  0退出,1告警
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			bvalue = 1;
			if(pprRunSet->bXDLYB)
				bvalue = 1;
			else
				bvalue = 0;	
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8241:  //零序电压定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_3U0;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8242:  //过压保护投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((!pprRunSet->bGyGJ)&&(pprRunSet->dYb & PR_YB_GY))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8243:  //过压保护定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_UGY;
			SetToFloat(bval,&fvalue);
			if(type ==0)
				fvalue = fvalue/pfdcfg->Un;
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8244:  //过压保护延时
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGY;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8245:  //高频保护投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((!pprRunSet->bGpGJ)&&(pprRunSet->dYb & PR_YB_GP))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8246:  //高频保护定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_FGP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8247:  //高频保护延时
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8248:  //低频保护投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((!pprRunSet->bDpGJ)&&(pprRunSet->dYb & PR_YB_DP))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8249:  //低频保护定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_FDP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x824A: //低频保护延时
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TDP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x824B: //过流三段投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((pprRunSet->bI3Trip) && (pprRunSet->dYb & PR_YB_I3)) //压板及出口都投才为跳闸，否则为告警
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x824C: //过流三段定值 A
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I3;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x824D: //过流三段时间  S，国网为ms
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_T3;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x824E:
		case 0x824F:
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
		default:
#if 0
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
#endif
			return ERROR;

	}
	
		
	return OK;
#else
	return ERROR;
#endif	
}

int WritePrParaYZ(WORD parano,char *pbuf)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	return CheckParaType_Len(fParaInfo->type,fParaInfo->len);
#else
	return ERROR;
#endif	
}

int WritePrPara(WORD parano,char *pbuf,WORD type)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	BOOL bvalue;
	WORD svalue;
	DWORD dvalue; 
	float fvalue;
	DWORD yb_kg1_kg2;
//	VPrRunInfo *pInfo;
//	VPrRunSet *pprRunSet;
//	VPrSetPublic  *pprPubRunSet;
	TSETVAL *tset;
	TSETVAL *bval;
	VFdCfg *pfdcfg;
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	bvalue = svalue = fvalue = dvalue = 0;
//	pInfo = prRunInfo;
//	pprRunSet = prRunSet[pInfo->set_no];
	tset = (TSETVAL*)(prSet[1]);

//	pprPubRunSet = prSetPublic[prRunPublic->set_no];
	pfdcfg = g_Sys.MyCfg.pFd;
	switch(parano)
	{
		case 0x8228: //过流一段投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I1_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I1_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 |= PR_YB_I1;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
			break;
		case 0x8229: //过流一段定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_I1;
				FloatToSet(fvalue,bval);
			break;
		case 0x822A: //过流一段时间
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_T1;
				FloatToSet(fvalue,bval);
			break;
		case 0x822B: //过流一段经低压闭锁
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I_DY;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I_DY;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
					yb_kg1_kg2 |= PR_YB_DY;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
			break;
		case 0x822C: //过流一段经方向闭锁
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I_DIR;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I_DIR;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			break;
		case 0x822D: //过流二段投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I2_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I2_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 |= PR_YB_I2;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 0x822E: //过流二段定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_I2;
				FloatToSet(fvalue,bval);
			break;
		case 0x822F: //过流二段时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_T2;
				FloatToSet(fvalue,bval);
			break;
		case 0x8230: //过流二段经低压闭锁
			
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I2_DY;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I2_DY;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
					yb_kg1_kg2 |= PR_YB_DY;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
			break;
		case 0x8231: //过流二段经方向闭锁
			
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I2_DIR;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I2_DIR;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
			break;
		case 0x8232: //过流后加速定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_IHJS;
				FloatToSet(fvalue,bval);
			break;
		case 0x8233: //过流后加速时间
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_TJS;
				FloatToSet(fvalue,bval);
			break;
		case 0x8234: //过负荷告警定值
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_IGFH;
				FloatToSet(fvalue,bval);
			break;
		case 0x8235: //过负荷告警时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 600000) //600s
					return 3;
				bval = tset + SET_TGFH;
				FloatToSet(fvalue,bval);
			break;
		case 0x8236: //零序一段投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I0_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I0_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 |= PR_YB_I01;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 0x8237:  //零序一段定值
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_3I01;
				FloatToSet(fvalue,bval);
			break;
		case 0x8238:  //零序一段时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_TI01;
				FloatToSet(fvalue,bval);
			break;
		case 0x8239:   //零序Ⅰ段经方向闭锁
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I0_DIR;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I0_DIR;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			break;
		case 0x823A:  //零序后加速定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_I0HJS;
				FloatToSet(fvalue,bval);
			break;
		case 0x823B:  //零序后加速时间
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_I0TJS;
				FloatToSet(fvalue,bval);
			break;
		case 0x823C:  //重合闸投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_CH;
				else
					yb_kg1_kg2 &= ~PR_YB_CH;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 0x823D:  //重合闸检无压投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
#ifdef INCLUDE_PR_PRO	
				caSetBuf = prSet[0];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= (1<<KG_8); //
				else
					yb_kg1_kg2 &= ~(1<<KG_8); //
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
#endif
			break;
		case 0x823E:  //重合闸检同期投退    待改
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
#ifdef INCLUDE_PR_PRO	
				caSetBuf = prSet[0];
				CvtK(SET_KG1,&yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |=  (2<<KG_8); //
				else
					yb_kg1_kg2 &= ~ (2<<KG_8); //
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
#endif
			break;
		
		case 0x823F:  //重合闸时间
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 600000)
					return 3;
				bval = tset + SET_TCH1;
				FloatToSet(fvalue,bval);
			break;
		case 0x8240:  // 小电流接地告警投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_XDL_YB;
				else
					yb_kg1_kg2 &= ~PR_CFG2_XDL_YB;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
			break;
		case 0x8241:  //零序电压定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_3U0;
				FloatToSet(fvalue,bval);
			break;
		case 0x8242:  //过压保护投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_GY_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG2_GY_TRIP;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
					yb_kg1_kg2 |= PR_YB_GY;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
					yb_kg1_kg2 &= ~PR_YB_GY;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
			break;
		case 0x8243:  //过压保护定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if(type ==0)
				fvalue = fvalue*pfdcfg->Un;
			
				dvalue = fvalue*1000;
				if(dvalue > 400000) //400V
					return 3;
				bval = tset + SET_UGY;
				FloatToSet(fvalue,bval);
			break;
		case 0x8244:  //过压保护延时
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_TGY;
				FloatToSet(fvalue,bval);
			break;
		case 0x8245:  //高频保护投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_GF_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG2_GF_TRIP;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 |= PR_YB_GP;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		
		case 0x8246:  //高频保护定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000)
					return 3;
				bval = tset + SET_FGP;
				FloatToSet(fvalue,bval);
			break;
		case 0x8247:  //高频保护延时
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //
					return 3;
				bval = tset + SET_TGP;
				FloatToSet(fvalue,bval);
			break;
		case 0x8248:  //低频保护投退
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_DF_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG2_DF_TRIP;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 |= PR_YB_DP;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 0x8249:  //低频保护定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //
					return 3;
				bval = tset + SET_FDP;
				FloatToSet(fvalue,bval);
			break;
		case 0x824A: //低频保护延时
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) 
					return 3;
				bval = tset + SET_TDP;
				FloatToSet(fvalue,bval);
			break;
		case 0x824B: //过流三段投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_I3_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG2_I3_TRIP;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			yb_kg1_kg2 |= PR_YB_I3;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
		break;
	case 0x824C: //过流三段定值
		if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_I3;
			FloatToSet(fvalue,bval);
		break;
	case 0x824D: //过流三段时间
		if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_T3;
			FloatToSet(fvalue,bval);
		break;
		default:
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
	}
	
		
	return OK;
#else
	return ERROR;
#endif	
}
#else
#define PARAFDNUM 42
WORD SNnum = 1;
int ReadPrPara(WORD parano,char *pbuf,WORD type)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	BOOL bvalue;
	WORD svalue,fd;
	float fvalue;
	
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	TSETVAL *tset;
	TSETVAL *bval;
	VFdCfg *pfdcfg;
	bvalue = svalue = fvalue = fd =0;
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return ERROR;

	parano = parano - PRPARA_ADDR + 1;
	if(parano < 5)
	{
		if(parano == 1)
		{
			fParaInfo->type = USHORT_TYPE;
			fParaInfo->len = USHORT_LEN;
			memcpy(fParaInfo->valuech,&SNnum,fParaInfo->len);
		}
		else
		{
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
		}
		return OK;
	}
		
	fd = (parano - 5)/PARAFDNUM;
	parano = parano - fd*PARAFDNUM;
	
	pInfo = prRunInfo+fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;
	pfdcfg = g_Sys.MyCfg.pFd+fd;
	tset = (TSETVAL*)(prSet[fd+1]); 

	switch(parano)
	{
		case 5: //过流一段投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_I1) //压板及出口都投才为跳闸，否则为告警
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 6: //过流一段定值 A
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I1;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 7: //过流一段时间  S，国网为ms
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_T1;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 8: //过流二段投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_I2)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 9: //过流二段定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I2;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 10: //过流二段时间 s
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_T2;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 11: //过负荷告警投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_GFH)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 12: //过负荷告警定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_IGFH;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 13: //过负荷告警时间
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGFH;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 14: //零序一段投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_I01)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 15:  //零序一段定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_3I01;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 16:  //零序一段时间
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TI01;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 17:  // 小电流接地告警投退  0退出,1告警
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			bvalue = 1;
			if(pprRunSet->bXDLYB)
				bvalue = 1;
			else
				bvalue = 0;	
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 18:  //零序电压定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_3U0;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 19: //小电流接地延时
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TDXJD;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 20: //小电流接地保护方式 2为正向，3为反向
			fParaInfo->type = USHORT_TYPE;
			fParaInfo->len = USHORT_LEN;
			if(pprRunSet->bXdlFx)
				svalue = 3;
			else
				svalue = 2;	
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
		case 21: //过流一段出口投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI1Trip) //
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 22: //过流一段复压闭锁              低压闭锁？
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bIDYBS) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);		
			break;
		case 23: //过流一段方向闭锁
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bIDir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 24: //过流二段出口投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI2Trip) //
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 25: //过流二段复压闭锁             低压闭锁？
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI2DYBS) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 26: //过流二段经方向闭锁
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI2Dir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 27: //零序一段出口投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI0Trip)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 28:// 零序后加速投入
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb&PR_YB_JS)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 29:  //零序后加速定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I0HJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 30:  //零序后加速时间
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I0TJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 31:   //零序Ⅰ段经方向闭锁
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI0Dir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 32: //小电流接地出口投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			bvalue = 1;
			if(pprRunSet->bXDLTrip)
				bvalue = 1;
			else
				bvalue = 0;	
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 33: //Uab过压保护投退               缺、、、、
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((!pprRunSet->bGyGJ)&&(pprRunSet->dYb & PR_YB_GY))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 34: //Ucb过压保护投退               缺、、、、
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((!pprRunSet->bGyGJ)&&(pprRunSet->dYb & PR_YB_GY))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 35:  //过压保护定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_UGY;
			SetToFloat(bval,&fvalue);
			if(type ==0)
				fvalue = fvalue/pfdcfg->Un;
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 36:  //过压保护延时
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGY;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 37:  //高频保护投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_GP)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 38:  //高频保护定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_FGP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 39:  //高频保护延时
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 40:  //低频保护投退
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_DP)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 41:  //低频保护定值
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_FDP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 42: //低频保护延时
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TDP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		default:
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
	}
	return OK;
#else
	return ERROR;
#endif	
}

int WritePrParaYZ(WORD parano,char *pbuf)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	return CheckParaType_Len(fParaInfo->type,fParaInfo->len);
#else
	return ERROR;
#endif	
}

int WritePrPara(WORD parano,char *pbuf,WORD type)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	BOOL bvalue;
	WORD svalue,fd;
	DWORD dvalue; 
	float fvalue;
	DWORD yb_kg1_kg2;
//	VPrRunInfo *pInfo;
//	VPrRunSet *pprRunSet;
//	VPrSetPublic  *pprPubRunSet;
	TSETVAL *tset;
	TSETVAL *bval;
	VFdCfg *pfdcfg;
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	bvalue = svalue = fvalue = dvalue = fd =0;
	
	parano = parano - PRPARA_ADDR + 1;
	if(parano < 5)
	{
		if(parano == 1)
		{

			memcpy(&SNnum,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			
return OK;
		}
		else
			return OK;
	}
		
	fd = (parano - 5)/PARAFDNUM;
	parano = parano - fd*PARAFDNUM;
	pfdcfg = g_Sys.MyCfg.pFd+fd;
	tset = (TSETVAL*)(prSet[fd+1]); 
	
	switch(parano)
	{
		case 5: //过流一段告警投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 &= ~PR_CFG1_I1_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			if(svalue == 1)
				yb_kg1_kg2 |= PR_YB_I1;
			else
				yb_kg1_kg2 &= ~PR_YB_I1;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
			break;
		case 6: //过流一段定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_I1;
			FloatToSet(fvalue,bval);
			break;
		case 7: //过流一段时间
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_T1;
			FloatToSet(fvalue,bval);
			break;
		case 8: //过流二段告警投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 &= ~PR_CFG1_I2_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			if(svalue == 1)
				yb_kg1_kg2 |= PR_YB_I2;
			else
				yb_kg1_kg2 &= ~PR_YB_I2;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
			break;
		case 9: //过流二段定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_I2;
			FloatToSet(fvalue,bval);
			break;
		case 10: //过流二段时间
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_T2;
			FloatToSet(fvalue,bval);
			break;
		case 11: //过负荷告警投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 &= ~PR_CFG1_GFH_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			if(svalue == 1)
				yb_kg1_kg2 |= PR_YB_GFH;
			else
				yb_kg1_kg2 &= ~PR_YB_GFH;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 12: //过负荷定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_IGFH;
			FloatToSet(fvalue,bval);
			break;
		case 13: //过负荷告警时间
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 600000) //600s
				return 3;
			bval = tset + SET_TGFH;
			FloatToSet(fvalue,bval);
			break;
		case 14: //零序一段告警投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 &= ~PR_CFG1_I0_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			if(svalue == 1)
				yb_kg1_kg2 |= PR_YB_I01;
			else
				yb_kg1_kg2 &= ~PR_YB_I01;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 15:  //零序一段定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_3I01;
			FloatToSet(fvalue,bval);
			break;
		case 16:  //零序一段时间
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_TI01;
			FloatToSet(fvalue,bval);
			break;
		case 17:  //小电流接地告警
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 &= ~PR_CFG2_XDL_TRIP;
			if(svalue == 1)
				yb_kg1_kg2 |= PR_CFG2_XDL_YB;
			else
				yb_kg1_kg2 &= ~PR_CFG2_XDL_YB;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
			break;		
		case 18: //零压定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_3U0;
			FloatToSet(fvalue,bval);
			break;
		case 19: //接地延时
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_TDXJD;
			FloatToSet(fvalue,bval);
			break;
		case 20: //接地保护方式
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(svalue == 2)
				yb_kg1_kg2 &= ~PR_CFG2_XDL_DLFX;
			else
				yb_kg1_kg2 |= PR_CFG2_XDL_DLFX;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
			break;		
		case 21: //过流1段出口投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(svalue)
				yb_kg1_kg2 |= PR_CFG1_I1_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I1_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			yb_kg1_kg2 |= PR_YB_I1;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
			break;
		case 22: //过流一段经复压闭锁   
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I_DY;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I_DY;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			if(bvalue)
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 |= PR_YB_DY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			break;
		case 23: //过流一段经方向闭锁
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I_DIR;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I_DIR;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			break;
		case 24: //过流2段出口投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(svalue)
				yb_kg1_kg2 |= PR_CFG1_I2_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I2_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			yb_kg1_kg2 |= PR_YB_I1;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
			break;
		case 25: //过流二段经复压闭锁   
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I2_DY;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I2_DY;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			if(bvalue)
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 |= PR_YB_DY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			break;
		case 26: //过流二段经方向闭锁
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I2_DIR;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I2_DIR;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			break;
		case 27: //零序一段出口
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I0_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I0_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			yb_kg1_kg2 |= PR_YB_I01;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 28: //零序后加速
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			caSetBuf = prSet[fd + 1];
			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			if(svalue)
				yb_kg1_kg2 |= PR_YB_JS;
			else
				yb_kg1_kg2 &= ~PR_YB_JS;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 29: //零流后加速定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_I0HJS;
			FloatToSet(fvalue,bval);
			break;
		case 30: //零流后加速时间
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_I0TJS;
			FloatToSet(fvalue,bval);
			break;
		case 31:   //零序Ⅰ段经方向闭锁
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I0_DIR;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I0_DIR;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			break;
		case 32: //小电流接地出口投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 |= PR_CFG2_XDL_YB;
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_XDL_TRIP;
			else
			{
				yb_kg1_kg2 &= ~PR_CFG2_XDL_TRIP;
				yb_kg1_kg2 &= ~PR_CFG2_XDL_YB;
			}
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
			break;
		case 33: //Uab过压保护投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_GY_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG2_GY_TRIP;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

			if(bvalue)
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 |= PR_YB_GY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			else
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 &= ~PR_YB_GY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			break;
		case 34: //Ucb过压保护投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_GY_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG2_GY_TRIP;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

			if(bvalue)
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 |= PR_YB_GY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			else
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
				yb_kg1_kg2 &= ~PR_YB_GY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			break;
		case 35:  //过压保护定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if(type ==0)
				fvalue = fvalue*pfdcfg->Un;
			dvalue = fvalue*1000;
			if(dvalue > 400000) //400V
				return 3;
			bval = tset + SET_UGY;
			FloatToSet(fvalue,bval);
			break;
		case 36:  //过压保护延时
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_TGY;
			FloatToSet(fvalue,bval);
			break;
		case 37:  //高频保护投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_GF_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG2_GF_TRIP;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			if(bvalue)
				yb_kg1_kg2 |= PR_YB_GP;
			else
				yb_kg1_kg2 &= ~PR_YB_GP;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		
		case 38:  //高频保护定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //250V
				return 3;
			bval = tset + SET_FGP;
			FloatToSet(fvalue,bval);
			break;
		case 39:  //高频保护延时
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //
				return 3;
			bval = tset + SET_TGP;
			FloatToSet(fvalue,bval);
			break;
		case 40:  //低频保护投退
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_DF_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG2_DF_TRIP;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//必要投压板
			if(bvalue)
				yb_kg1_kg2 |= PR_YB_DP;
			else
				yb_kg1_kg2 &= ~PR_YB_DP;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 41:  //低频保护定值
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //
				return 3;
			bval = tset + SET_FDP;
			FloatToSet(fvalue,bval);
			break;
		case 42: //低频保护延时
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) 
				return 3;
			bval = tset + SET_TDP;
			FloatToSet(fvalue,bval);
			break;
		default:
			
			break;
	}
	return OK;
#else
	return ERROR;
#endif	
}
#endif
#endif

int WritePrParaFile()
{
#ifdef INCLUDE_PR
	WORD fdno,len,i;
	struct VFileHead *head;
	BYTE *wrSet;
	char fname[MAXFILENAME];
	
	if(g_prInit == 0) return ERROR;
	fdno = g_Sys.MyCfg.wFDNum;
	if(fdno == 0) return ERROR;
	
	//公共参数
	{
		caSetBuf = prSet[0];
		if(prPubSetConv(0) != SET_OK) return ERROR;
		head = (struct VFileHead *)prSetTmp;	
		head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
		head->wVersion = CFG_FILE_VER;
		head->wAttr = CFG_ATTR_NULL;
		wrSet = (BYTE*)(head+1);
		len = sizeof(TTABLETYPE);
		memcpy(wrSet,&tSetType,len);
		memcpy(wrSet+len,prSet[0],SET_PUB_NUMBER*sizeof(TSETVAL));
		
		if(WriteParaFile("fdPubSet.cfg",(struct VFileHead*)prSetTmp) == ERROR)
		{
			return ERROR;
		}
		if(prRunPublic->set_no == 0)
			prRunPublic->set_no =1;
		else
			prRunPublic->set_no = 0;
	}
	//回线参数
	for(i=0;i<fdno;i++)
	{
		caSetBuf = prSet[i+1];
		if(prFdSetConv(i) != SET_OK) return ERROR;
		
		head = (struct VFileHead *)prSetTmp;	
		head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
		head->wVersion = CFG_FILE_VER;
		head->wAttr = CFG_ATTR_NULL;
		wrSet = (BYTE*)(head+1);
		len = sizeof(TTABLETYPE);
		memcpy(wrSet,&tSetType,len);
		memcpy(wrSet+len,prSet[i+1],SET_NUMBER*sizeof(TSETVAL));
		
		sprintf(fname, "fd%dset.cfg", i+1);
		if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
		{
			return ERROR;
		}		
		prFdSetNoSwitch(i);
	}
	
	return OK;
#else
	return ERROR;
#endif
}

BOOL ReadGLLB(WORD fd)	
{
#ifdef INCLUDE_PR
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	if(fd >= fdNum) 
		return 0;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no] + fd;
	if(pprRunSet->bRcdGl)
		return 1;
	else
#endif
		return 0;
}
void WriteGLLB(WORD fd, BOOL gllb)
{
#ifdef INCLUDE_PR
	DWORD yb_kg1_kg2 =0;
	if(fd >= fdNum) 
		return ;
	caSetBuf = prSet[fd+1];
	CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
	if(gllb)
		yb_kg1_kg2 |= PR_CFG2_RCD_GL;
	else
		yb_kg1_kg2 &= ~PR_CFG2_RCD_GL;
	ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
#endif
}
BOOL ReadWYLB(WORD fd)	
{
#ifdef INCLUDE_PR
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	if(fd >= fdNum) 
		return 0;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no] + fd;
	if(pprRunSet->bRcdSy)
		return 1;
	else
#endif
		return 0;
}
void WriteWYLB(WORD fd, BOOL wllb)
{
#ifdef INCLUDE_PR
	DWORD yb_kg1_kg2 =0;
	if(fd >= fdNum) 
		return ;
	caSetBuf = prSet[fd+1];
	CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
	if(wllb)
		yb_kg1_kg2 |= PR_CFG2_RCD_SY;
	else
		yb_kg1_kg2 &= ~PR_CFG2_RCD_SY;
	ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
#endif
}
BOOL ReadI0LB(WORD fd)	
{
#ifdef INCLUDE_PR
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	if(fd >= fdNum) 
		return 0;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no] + fd;
	if(pprRunSet->bRcdI0TB)
		return 1;
	else
#endif
		return 0;
}
void WriteI0LB(WORD fd, BOOL i0lb)
{
#ifdef INCLUDE_PR
	DWORD yb_kg1_kg2 = 0;
	if(fd >= fdNum) 
		return ;
	caSetBuf = prSet[fd+1];
	CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
	if(i0lb)
		yb_kg1_kg2 |= PR_CFG2_RCD_I0TB;
	else
		yb_kg1_kg2 &= ~PR_CFG2_RCD_I0TB;
	ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
#endif
}
BOOL ReadU0LB(WORD fd)	
{
#ifdef INCLUDE_PR
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	if(fd >= fdNum) 
		return 0;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no] + fd;
	if(pprRunSet->bRcdU0)
		return 1;
	else
#endif
		return 0;
}
void WriteU0LB(WORD fd, BOOL u0lb)
{
#ifdef INCLUDE_PR
	DWORD yb_kg1_kg2 = 0;
	if(fd >= fdNum) 
		return ; 
	caSetBuf = prSet[fd+1];
	CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
	if(u0lb)
		yb_kg1_kg2 |= PR_CFG2_RCD_U0;
	else
		yb_kg1_kg2 &= ~PR_CFG2_RCD_U0;
	ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
#endif
}


// 获取保护一次值 及保护延时
long ReadIVIT(int no,int fd)
{
	long ycvalue = 0;
#ifdef INCLUDE_PR
	TSETVAL *tset,*bval;
	float fvalue;
	if((g_prInit == 1) && (fd < fdNum))
	{
		tset = (TSETVAL*)(prSet[fd+1]);

		switch(no)
		{
			case 0: //零序保护延时
				bval = tset + SET_TI01;
				SetToFloat(bval,&fvalue);
				ycvalue = fvalue*1000;
				break;
			case 1: //零序保护电流定值一次值
				bval = tset + SET_3I01;
				SetToFloat(bval,&fvalue);
			  ycvalue = fvalue*pFdCfg[fd].ct0/pFdCfg[fd].In0;
				break;
			case 2: //保护延时 速断
				bval = tset + SET_T1;
				SetToFloat(bval,&fvalue);
				ycvalue = fvalue*1000;
				break;
			case 3: //保护定值一次值 (速断)
				bval = tset + SET_I1;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*pFdCfg[fd].ct/pFdCfg[fd].In;
				break;
			case 4: //过流一段保护定值(二次值)
				bval = tset + SET_I1;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
			  	break;
			case 5:  //过流二段定值
				bval = tset + SET_I2;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 6: //过流二段延时
				bval = tset + SET_T2;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 7: //过流三段定值
				bval = tset + SET_I3;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 8: //过流三段延时
				bval = tset + SET_T3;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 9: //零序过流定值
				bval = tset + SET_3I01;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 10: //一次重合时间
				bval = tset +  SET_TCH1;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 11: //重合放电时间
				bval = tset +  SET_TCH1;
				SetToFloat(bval,&fvalue);
				ycvalue = fvalue*1000;
				bval = tset +  SET_TCHBS;
				SetToFloat(bval,&fvalue);
				ycvalue += fvalue*1000;
				ycvalue += 9*1000; //放电时间加9秒
				break;
			case 12: //过电压
				bval = tset + SET_UGY;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 13: //过电压时间
				bval = tset + SET_TGY;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;

			case 14: //过频定值
				bval = tset + SET_FGP;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 15: //过频时间
				bval = tset + SET_TGP;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;

			default:
				break;
		}
	}
#endif
	return ycvalue;
}

