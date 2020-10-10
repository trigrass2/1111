/*------------------------------------------------------------------------
 Module:       	para.h
 Author:        solar
 Project:      
 Creation Date:	2008-08-05
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
------------------------------------------------------------------------*/

#ifndef _PARA_H
#define _PARA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syscfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file.h"

#define CFG_IND_FILENAME      "ipacs.cfg"
#define CFG_PKG_FILENAME      "ipacs%02d.cfg"

#define CFG_FILE_VER          100   /*1.00*/

#define GEN_NAME_LEN          40
#define GEN_NAMETAB_LEN       15    /*缩写*/
#define GEN_NAMELONG_LEN      128

#define PBYXNUM     	 12
#define NEXTPARALEN      4


#define PARACHAR_MAXLEN  64
#define Old_MAXLEN   	 30


#define FLOAT_TYPE      38
#define FLOAT_LEN       4

#define USHORT_TYPE     45
#define USHORT_LEN       2

#define BOOL_TYPE        1
#define BOOL_LEN         1

#define TINY_TYPE       43
#define TINY_LEN        1

#define SHORT_TYPE      33
#define SHORT_LEN       2


#define DOUBLE_TYPE     39
#define DOUBLE_LEN      8

#define INT_TYPE        2
#define INT_LEN         4

#define STRING_TYPE     4
#define OCTSTR_TYPE     4

#define UINT_TYPE   	35
#define UINT_LEN    	4

#define LONG_TYPE   	36
#define LONG_LEN    	8

#define ULONG_TYPE   	37
#define ULONG_LEN    	8

#define UTINY_TYPE		32
#define UTINY_LEN		1

#define SYS_DY_YXNO  (g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low + 18)
#define SYS_GY_YXNO  (g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low + 19)

#define SELFPARA_ADDR 0x8001

#define SELFPARA_TYPE  0
#define SELFPARA_OS      1
#define SELFPARA_MFR    2
#define SELFPARA_HV      3
#define SELFPARA_SV      4
#define SELFPARA_CRC    5
#define SELFPARA_PROTOCOL   6 //这边暂没有
#define SELFPARA_DEVTYPE      7
#define SELFPARA_ID       8
#define SELFPARA_MAC    9

#define SELFPARA_IP1       10
#define SELFPARA_IP2       11

#define SELFPARA_COM1    12
#define SELFPARA_COM2    13
#define SELFPARA_COM3    14
#define SELFPARA_COM4    15

#define SELFPARA_PT2     16
#define SELFPARA_CT2      17

#define SELFPARA_MFR_GB2312         18
#define SELFPARA_PROTOCOL_GB2312   19
														   
#define SELFPARA_NUM    20


#define SELFPARA_OSNAME  "other 1.0.0.0"
#define SELFPARA_DEVTYPE_NAME "iPACS-5612%s"
#define HVVER_LEN  7
#define SVVER_LEN  8


#define RUNPARA_ADDR  0x8020
#define RPR_DV_NUM      0x06
#define RPR_DV_I               0x00
#define RPR_DV_AC            0x01
#define RPR_DV_DC            0x02
#define RPR_DV_P              0x03
#define RPR_DV_F               0x04
#define RPR_DV_COS         0x05

#define RPR_PT1                 0x06
#define RPR_PT2                 0x07
#define RPR_V_DY              0x08
#define RPR_T_DY              0x09
#define RPR_V_GY              0x0A
#define RPR_T_GY              0x0B
#define RPR_V_ZZ               0x0C
#define RPR_T_ZZ               0x0D
#define RPR_V_GZ              0x0E
#define RPR_T_GZ              0x0F
#define RPR_T_YXFD         0x10
#define RPR_T_FZ               0x11
#define RPR_T_HZ              0x12
#define RPR_T_HH              0x13
#define RPR_T_HHT            0x14
#define RPR_V_ILP              0x15
#define RPR_V_ULP            0x16
#define RPR_V_UYX           0x17
#define RPR_V_IYX            0x18
#define RPR_V_IYXYB        0x19


#define RUNPARA_LINEADDR 0x8040
#ifdef _TEST_VER_
#define RUNPARA_LINENUM   20
#else
#define RUNPARA_LINENUM   16
#endif
#define RPR_LINE_CT1       0x00
#define RPR_LINE_CT2       0x01
#define RPR_LINE_CT01     0x02
#define RPR_LINE_CT02     0x03

#define RUNPARA_LINEADDR1 0x804A
#define PRP_LBYB_GL        0x0A  //过流录波
#define PRP_LBYB_WY        0x0B // 失压录波
#define PRP_LBYB_I0       0x0C  //  零流突变
#define PRP_LBYB_U0        0x0D //零压突变
#define RPR_V_IL1P            0x0E //电流零漂
#define RPR_V_UL1P            0x0F //电压零漂
#define RPR_V_ZLDYLP            0x10 //直流电压零漂
#define RPR_V_PLP            0x11 //功率零漂
#define RPR_V_FLP            0x12 //频率零漂
#define RPR_V_COSLP            0x13 //功率因素零漂


#define PRPARA_ADDR    0x8220     //动作定值首地址
#define PRP_YB_LEDKEEP      0x00         //故障灯自动复归投入
#define PRP_T_LEDKEEP         0x01		  //故障灯自动复归时间
#define PRP_T_YXKEEP     0x02		
#define PRP_YB_FTUSD    0x03
#define PRP_T_X                0x04
#define PRP_T_Y                0x05
#define PRP_T_C                0x06
#define PRP_T_S                0x07
#define PRP_T_DXJD         0x08        //单相接地跳闸时间
#define PRP_T_HZ             0x09       //选线跳闸重合时间
#define PRP_YB_ZSYI       0x0A        //自适应相间短路故障处理
#define PRP_YB_ZSYI0     0x0B       //自适应单相接地故障处理
#define PRP_YB_CH1        0x0C       //一次重合闸
#define PRP_T_CH1           0x0D       //一次重合时间
#define PRP_YB_IH           0x0E       //大电流闭锁重投退
#define PRP_I_IH              0x0F       //大电流闭锁重定值
#define PRP_YB_CHN       0x10        // 多次重合闸投入
#define PRP_I_CHN          0x11        // 重合闸次数
#define PRP_T_CHN          0x12        // 非第一次重合闸延时
#define PRP_YB_JDMODE 0x13       //系统接地方式，0：中性点不接地1：经消弧线圈接地
														   // 2：经小电阻接地3：经高阻接地
#define PRP_YB_FDFJ       0x15       // 分段或分界，0：分段/联络开关1：分界开关
#define PRP_YB_FDLL       0x16       // 联络分段模式，0 分段 1联络
#define PRP_YB_JDFA       0x17      // 就地型FA模式，0：自适应综合型 FA  1: 电压时间2: 电压电流


#define PRPARA_LINE_ADDR     0x8240
#define PRPARA_LINE_ADDREND  0x8400
#define PRPARA_LINE_NUM      0x20

#define PRP_YB_GLTRIP    0x00      //过流停电跳闸投退
#define PRP_YB_I1TT      0x01      //过流一段投退
#define PRP_YB_I1TRIP     0x02      //过流一段出口
#define PRP_I_I1                0x03      //过流一段定值
#define PRP_T_I1               0x04      //过流一段时间
#define PRP_YB_I2TT        0x05      //过流二段投退
#define PRP_YB_I2TRIP    0x06      //过流二段出口
#define PRP_I_I2               0x07      //过流二段定值
#define PRP_T_I2               0x08      //过流二段时间
#define PRP_YB_I0TT        0x09      //零序过流投退
#define PRP_YB_I0TRIP     0x0A     //零序过流出口
#define PRP_I_I0                0x0B     //零序过流定值
#define PRP_T_I0               0x0C     //零序过流时间
#define PRP_YB_ILTT       0x0D     //小电流接地投退
#define PRP_YB_ILTRIP    0x0E      //小电流接地出口
#define PRP_YB_FZDBS    0x10   // 非遮断电流闭锁投退
#define PRP_I_FZDBS    0x11   // 非遮断电流定值
#if 0
#define PRP_YB_GLLB       0x11      //过流故障录波
#define PRP_YB_SYLB       0x12     //线路失压录波
#define PRP_YB_U0LB       0x13      //零序电压录波
#define PRP_YB_I0LB        0x14      //零序电流录波
#endif
#define PRPARA_TTU_ADDR     0x8401	//TTU 新增参数
#define PRPARA_TTU_ADDREND  0x8439
#define PRPARA_TTU_NUM      0x57
#define PRP_CAPACITY_TTU	0x00	//配变容量
#define PRP_UNBALANCE_I		0X01	//电流不平衡定值
#define PRP_UNBALANCE_V		0x02	//电压不平衡定值
#define PRP_RATE_TIME_1		0X03	//电能计量时段1
#define PRP_RATE_TIME_2		0X04	//电能计量时段2
#define PRP_RATE_TIME_3		0X05	//电能计量时段3
#define PRP_RATE_TIME_4		0X06	//电能计量时段4
#define PRP_RATE_TIME_5		0X07	//电能计量时段5
#define PRP_RATE_TIME_6		0X08	//电能计量时段6
#define PRP_RATE_TIME_7		0X09	//电能计量时段7
#define PRP_RATE_TIME_8		0X0A	//电能计量时段8
#define PRP_RATE_TIME_9		0X0B	//电能计量时段9
#define PRP_RATE_TIME_10	0X0C	//电能计量时段10
#define PRP_RATE_TIME_11	0X0D	//电能计量时段11
#define PRP_RATE_TIME_12	0X0E	//电能计量时段12
#define PRP_RATE_TIME_13	0X0F	//电能计量时段13
#define PRP_RATE_TIME_14	0X10	//电能计量时段14
#define PRP_RATE_TIME_15	0X11	//电能计量时段15
#define PRP_RATE_TIME_16	0X12	//电能计量时段16
#define PRP_RATE_TIME_17	0X13	//电能计量时段17
#define PRP_RATE_TIME_18	0X14	//电能计量时段18
#define PRP_RATE_TIME_19	0X15	//电能计量时段19
#define PRP_RATE_TIME_20	0X16	//电能计量时段20
#define PRP_RATE_TIME_21	0X17	//电能计量时段21
#define PRP_RATE_TIME_22	0X18	//电能计量时段22
#define PRP_RATE_TIME_23	0X19	//电能计量时段23
#define PRP_RATE_TIME_24	0X1A	//电能计量时段24
#define PRP_RATE_TIME_25	0X1B	//电能计量时段25
#define PRP_RATE_TIME_26	0X1C	//电能计量时段26
#define PRP_RATE_TIME_27	0X1D	//电能计量时段27
#define PRP_RATE_TIME_28	0X1E	//电能计量时段28
#define PRP_RATE_TIME_29	0X1F	//电能计量时段29
#define PRP_RATE_TIME_30	0X20	//电能计量时段30
#define PRP_RATE_TIME_31	0X21	//电能计量时段31
#define PRP_RATE_TIME_32	0X22	//电能计量时段32
#define PRP_RATE_TIME_33	0X23	//电能计量时段33
#define PRP_RATE_TIME_34	0X24	//电能计量时段34
#define PRP_RATE_TIME_35	0X25	//电能计量时段35
#define PRP_RATE_TIME_36	0X26	//电能计量时段36
#define PRP_RATE_TIME_37	0X27	//电能计量时段37
#define PRP_RATE_TIME_38	0X28	//电能计量时段38
#define PRP_RATE_TIME_39	0X29	//电能计量时段39
#define PRP_RATE_TIME_40	0X2A	//电能计量时段40
#define PRP_RATE_TIME_41	0X2B	//电能计量时段41
#define PRP_RATE_TIME_42	0X2C	//电能计量时段42
#define PRP_RATE_TIME_43	0X2D	//电能计量时段43
#define PRP_RATE_TIME_44	0X2E	//电能计量时段44
#define PRP_RATE_TIME_45	0X2F	//电能计量时段45
#define PRP_RATE_TIME_46	0X30	//电能计量时段46
#define PRP_RATE_TIME_47	0X31	//电能计量时段47
#define PRP_RATE_TIME_48	0X32	//电能计量时段48
#define PRP_SWTICH_TIME_CYC	0X33	//与开关的对时周期
#define PRP_SHORTCIR_CAP	0X38	//配变最小短路容量

#define PRP_HAR_DEAD_V		0X34	//电压谐波死区
#define PRP_HAR_DEAD_I		0X35	//电流谐波死区
#define PRP_DEAD_UNBALANCE	0X36	//不平衡度死区
#define PRP_DEAD_LOAD		0X37	//负载率死区
#define PRPARA_SWITCH_ADDR     0x8481	//低压参数开关起始地址
#define PRPARA_SWITCH_ADDREND  0x84D1	//低压参数开关结束地址
#define PRPARA_SWITCH_NUM		PRPARA_SWITCH_ADDREND-PRPARA_SWITCH_ADDR+1
#define PRPARA_EVER_SWITCH_NUM 30	//每个间隔的参数数量

#define PRP_RESDUAL_I_VALUE   	 0X00	//额定剩余电流动作值
#define	PRP_RESDUAL_I_TIME   	 0X01	//额定极限不驱动时间
#define	PRP_RESDUAL_I_CONTROL  	 0X02	//剩余电流保护控制字(控制字4）
#define	PRP_SHORT_I_VALUE		 0X03	//短路瞬时保护电流定值
#define PRP_SHORT_I_CONTROL		 0X04	//短路瞬时保护控制字(控制字3）
#define	PRP_SHORTDELAY_I_VALUE	 0X05	//短路短延时保护电流定值
#define	PRP_SHORTDELAY_I_TIME	 0X06	//短路短延时保护时间定值
#define	PRP_SHORTDELAY_I_CONTROL 0X07	//短路短延时保护控制字(控制字3）
#define	PRP_OVERLOAD_I_VALUE	 0X08	//过载保护电流定值
#define	PRP_OVERLOAD_I_TIME		 0X09	//过载保护时间定值
#define	PRP_OVERLOAD_I_CONTROL	 0X0A	//过载保护控制字(控制字3）
#define	PRP_OVER_U_VALUE		 0X0B	//过压保护电压定值
#define	PRP_OVER_U_CONTROL		 0X0C	//过压保护控制字(控制字2）
#define	PRP_UNDER_U_VALUE		 0X0D	//欠压保护电压定值
#define	PRP_UNDER_U_CONTROL		 0X0E	//欠压保护控制字(控制字2）
#define	PRP_BREAKPHASE_U_VALUE	 0X0F	//断相保护电压定值
#define	PRP_BREAKPHASE_U_CONTROL 0X10	//断相保护控制字(控制字2）
#define	PRP_ZERO_U_CONTROL		 0X11	//缺零保护控制字(控制字2）
#define	PRP_SELF_CHECK_TIME		 0X12	//定时自检时间
#define	PRP_REST_GATE_CONTROL	 0X13	//重合闸控制字(控制字1）
#define	PRP_SWITCH_RETURN_TIME	 0X14	//开关遥信复归时间


#if(DEV_SP  == DEV_SP_TTU)
#define PARA_END PRPARA_SWITCH_ADDREND
#else
#define PARA_END PRPARA_LINE_ADDREND
#endif
#define FILE_TYPE_PR       0x01
#define FILE_TYPE_RUN      0x02
#define FILE_TYPE_PEOGRAM 0x03

#pragma pack(1)

struct VParaInfo{
	BYTE type;
	BYTE len;
	char valuech[PARACHAR_MAXLEN];
};	

struct VNextParaInfo{
	WORD infoaddr;
	BYTE type;
	BYTE len;
	char valuech[PARACHAR_MAXLEN];
};	

struct VParaDef{
	DWORD parano;
	char paraname[30];
};

struct VFileHead{
	int nLength;      /*文件长度，包括文件头,数据,文件校验置于ind文件中*/
	WORD wVersion;    /*十进制数表示,次版本号占两位.如1.03为103*/
	WORD wAttr;       /*参数属性*/
	DWORD rsv[5];
};

struct VNetIP{
	char sIP[20];
	DWORD dwMask;
};

//结构次序与GetSubParaFile()有很大关系,不能轻易改动
struct VPAddr{
	WORD  wAddr;         /*设备地址,全系统唯一,若Lan1有效则自动取Lan1的低字且不允许改动*/
	struct VNetIP Lan1;
	WORD  wCantonCode;
	char  sExtAddr[12];

	WORD  wAsMasterAddr; /*设备做为主站方的地址,缺省与Addr相同,维护暂时不显示*/
	char  sTelNo[20];

	struct VNetIP Lan2;
	char sGateWay1[20];

    BYTE byDefFlag;      /*参数是否是缺省生成标志,对应维护软件,固定为0x55*/     
	BYTE byValidFlag;    /*参数有效标志,固定为0x55有效*/     

	char sGateWay2[20];
	char sGW2_Dest[20];
};

#define FD_CFG_PT_58V   0x20      

struct VPFdCfg{
	char kgname[GEN_NAME_LEN];
	DWORD kgid;  /*界面不显示，自动取装置地址缀上回线号，如地址为2的第1回线，为201*/

	DWORD cfg;   /*D0 = 1  电压等级为低压  =0 电压等级为中压(默认)
	               D1 = 1  交采一次侧数据上传时减少一位有效位(非默认)
				   D2 = 1  交采一次侧功率上传时单位为Mw或Mvar(非默认)
				   D3 = 1  无功补偿功能有效  =0 无无功补偿(默认)
				   D4 = 1  故障检测无效 =0 故障检测有效(默认)
				   D5 = 1  电压接入为相电压 =0 接入为线电压(默认)
				 */

	WORD Un;      /*二次额定电压:220V或100V,值取220或100*/
    WORD In;      /*二次额定电流:5A或1A,值取5或1*/
	
	int pt;       /*PT设置：X/Un*/
	int ct;       /*CT设置：X/In*/
	
	char  kg_stateno;    //对应开关状态硬遥信点号
	char  kg_faultno;    //对应开关故障硬遥信点号, -1表示系统软遥信
	char  kg_ykno;       //对应开关遥控点号
	char  kg_vectorno;   //对应开关故障方向硬遥信点号, -1表示系统软遥信        

	BYTE  kg_stateno_ioaddr;  //开关状态遥信对应的io插件地址, 0为本机
	BYTE  kg_faultno_ioaddr;  //开关故障遥信对应的io插件地址, 0为本机
	BYTE  kg_ykno_ioaddr;     //开关遥控对应的io插件地址, 0为本机
	BYTE  kg_vectorno_ioaddr; //开关故障方向遥信对应的io插件地址, 0为本机
    
	WORD  In0;
	WORD  rsv1;
	int   ct0;

	char  kg_closeno;       //对应开关手合遥信
	char  kg_openno;      //对应开关手分遥信
	char  kg_startno;      //对应开关启动遥信
	char  kg_unlockno;	//对应开关闭锁遥信

	char kg_ykkeepno;   //对应遥控保持的点号
	char kg_remoteno;   //环网柜远方对应的遥信点号
    BYTE rsv2[2];
    DWORD rsv[4];	
};	

struct VPAiCfg{
    int type;      //见YC_TYPE
    int rsv1;    
	DWORD  cfg;    //暂为0
	DWORD admin;
	DWORD admax;
	DWORD rsv[3];
};

struct VPDiCfg{
    int type;     /*=0 遥信（单点）
                    =1 脉冲输入（数据库中次序排在其他类型电度之后）*/
	DWORD  cfg;   /*D0：=1 遥信取反，=0 不取反	  
					D1：使能，=1 输入无效，=0 输入有效
					D2=1 不生成SOE	＝0生成SOE
					D3=1 不生成COS	＝0生成COS
					*/
	
    WORD  dtime;  /*遥信防抖时间或脉冲宽度(1ms)*/
	WORD  yxno;   /*该通道对应的逻辑遥信点号*/

	WORD T1;
	WORD N;
	float T2;
	WORD re_yxno;
	WORD rsv[5];
};

struct VPDoCfg{
	int type;           /*=0 双点型 (合分双继电器)
	                      =1 单点型 (仅一个继电器)
                          当单点型时ontime无效
                        */  	
	DWORD cfg;          //暂为0
    DWORD hzontime;       /*遥控闭合时间,1ms单位，默认2000*/
    short ybno;         /*压板配置: -2无压板
                                    -1软压板
                              >=0硬压板遥信号
                        */ 
    WORD pad;                 
	DWORD yztime; 		/*遥控预置超时时间,1ms单位,默认30000*/	
	DWORD fzontime;
	DWORD rsv[2];	    
};	

struct VPYcCfg{
	int type;
    DWORD  cfg;     //暂为0
    
	int arg1;
	int arg2;
	short toZero;     //归0值，缺省为5

	WORD bak[5];	
	DWORD rsv[5];	
};

struct  VPSysConfig
{
	DWORD dwType;     /*设备型号(四字节） 由模板生成，程序员定义*/
	DWORD dwName;     /*设备标识，系统内唯一，取(dwType&0xFFF)+该设备在系统内数目,不显示*/
	char  sByName[GEN_NAME_LEN];   /*设备名称,设备创建时用户在维护定义*/
	DWORD dwCfg;     /*D0=1 蓄电池自动活化
	                 				 D1=1 sntp对时
	                 				 D2 =1 ，遥测1次值上传维护遥测界面
	                 				 D3 = 1,国网版本,规约基本信息扩展
	                 				 D3 = 0，以前版本规约信息没扩展，电流电压死区没有
	                                 D4 = 1，零序电压小信号，D4 = 0，零序电压正常额定
	                 			*/

	WORD  wFDNum;

	WORD  wAINum;
	WORD  wDINum;
	WORD  wDONum;	

	WORD  wYCNum;
	WORD  wDYXNum;
	WORD  wSYXNum;
	WORD  wDDNum;
	WORD  wYKNum;	
	WORD  wYTNum;

	WORD  wYcNumPublic;
	WORD  wYcNumAdd;
	DWORD dSntpServer;
	DWORD dwAIType;
	DWORD dwIOType;
	DWORD rsv[5]; 	
};

struct VPSysExtConfig
{
    char sWsName[GEN_NAME_LEN];
	char sWsSechema[GEN_NAMELONG_LEN];
	DWORD dWsIp;
	DWORD dWsVer;
	DWORD dwMaintIp;
	BYTE  byMac[6];
	DWORD dwKgTime;
	struct VPDiCfg YxIn[8];
	char xmlVer[16];
	char xmlRev[16];
	DWORD xmlCrc;
	struct VNetIP Lan3;
	char sGateWay3[20];
	short YXNO[12];
	BYTE YKDelayTime;
	struct VNetIP Lan4;
	char sGateWay4[20];
	char sRoute3[20];
	char sRoute4[20];
	BYTE rsv1[3];
	DWORD rsv[52];
};

struct VPFdExtConfig
{
    short kg_fabsno[8];
	short fabs_eqpid[8];
	WORD rsv[24];
};

struct VPCellCfg{
	DWORD dwCfg;        //D0--电池活化使能
	DWORD dwNum;        //方案个数
	WORD  wID;          //电池活化ID
	WORD Udis;  //电池活化阈值
    WORD wrsv;
	DWORD rsv[4];
};	

#define CELL_MODE_ENDTIME            0x80000000	
struct VPCellCtrl{
	DWORD dwMode;       //0--禁止 1--月日模式 2---月周模式, bit31:结束时间
	WORD wMonthBits;
	WORD wWeekBits;
	DWORD dwDayBits;
	WORD wHour;
	WORD wMin;

	DWORD dwTime;

	DWORD rsv[4];	
};		

struct VPUSwitchCfg{
	DWORD dwCfg;        //D0--使能
	DWORD dwNum;        //方案个数

	DWORD rsv[5];
};	
	
struct VPUSwitchCtrl{
	DWORD dwMode;       //0--禁止1--电压模式 2-开关状态模式
	WORD wUFdNo1;       //采集板内的电压组号 
	WORD wUFdNo2;       //采集板内的电压组号 

	WORD wUFdYx1;	
	WORD wUFdYx2;
	
	DWORD rsv[5];	
};		

struct VPExtIoAddr{
	DWORD dwType;
	WORD  wAddr;         
	struct VNetIP Lan;
};

struct VPExtIoInfo{
    struct VPExtIoAddr ioAddr;
	DWORD rsv[10];  	
};	

#define PARA_SERIAL_START_NO      50
#define PARA_SERIAL_NUM           16
#define PARA_NET_SYS_NUM          10

#define PARA_NET_START_NO         (PARA_SERIAL_START_NO+PARA_SERIAL_NUM)
#define PARA_NET_USR_NO           (PARA_NET_START_NO+PARA_NET_SYS_NUM)

#define PARA_CAN_STARTNO          (PARA_NET_START_NO-1)
#define PARA_START_NO             PARA_SERIAL_START_NO

struct VPPort{
	int id;                   /*通信端口号：
                                串口：串口1-串口15对应端口号为51-65
                                以太网连接：Socket1对应端口号为76，以此类推*/
	char pcol[GEN_NAME_LEN];  /*规约名称，与protocol.cin中一致*/
	int pcol_attr;            /*规约属性，0－主方规约   1－从方规约*/
	char cfgstr[3][30];   /*规约配置串
                            服务器端:type:ip:port-1:0.0.0.0:8080
                            客户端:type:ip:port-2:192.166.0.2:8080
							串口:type:baud,databits,stopbits,parity-4:9600,8,1,N  

							端口类型 (type) 
							0:NULL
							1:TCP-SERVER
							2:TCP-CLIENT
							3:UDP
							4:RS232 */ 

	int bakmode;    //端口转发模式
	int bakcomm;    //转发端口
	
	char modelname[GEN_NAMETAB_LEN+1];
	DWORD rsv[6];	
};
  
struct VPEqp{  
	DWORD dwName;
    char sCfgName[GEN_NAME_LEN];  /*对应参数文件名称，不包括文件名后缀,如：.cde、.cdt等*/
	WORD wCommID;  /*对应端口号*/
	DWORD rsv[5];  
};

struct VPTECfg{
	WORD wSourceAddr;  /*本机地址 (组态软件暂时不显示-自动取Addr.cfg中的Addr)*/
	WORD wDesAddr;     /*下级装置地址*/
	char sExtAddr[12]; /*扩展地址*/
	char sTelNo[20];   /*手机号码*/

	DWORD dwFlag;  /*D0:遥测乘系数
	                 D1:电度乘系数
	               */  
	               
	WORD wYCNum;
	WORD wVYCNum;	/*虚拟遥测个数*/
	WORD wDYXNum;
	WORD wSYXNum;
	WORD wVYXNum; /*虚拟遥信个数*/
	WORD wDDNum;
	WORD wYKNum;
	WORD wYTNum;
	WORD wTSDataNum;
	WORD wTQNum;

	BYTE byYCFrzMD;   //遥测冻结密度 1-60min 默认15min
	BYTE byYCFrzCP;   //遥测冻结容量 1-180day 默认30day

	BYTE byDDFrzMD;   //遥测冻结密度 5-60min 默认60min
	BYTE byDDFrzCP;   //遥测冻结容量 1-180day 默认30day	
	
	DWORD rsv[4]; 	
};

struct VPTrueYC{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;  /*D0=1 发送(缺省) =0 不发送
	                D1=1 曲线 =0 不生成曲线(缺省)
	                D2=1 日冻结 =0无日冻结(缺省)
	                D3=1 月冻结 =0无月冻结(缺省)	                
					D4－D5:  限值一类型 
					   0：无效
					   1：上限
					   2：下限
					D6－D7:  限值二类型 
					   0：无效
					   1：上限
					   2：下限
					D8=1 日极值曲线 =0不生成日极值曲线(缺省)
					D9=1 主动发送禁止 =0主动发送允许(缺省)

					D29＝1 被取代 =0正常(缺省)
					D30＝1 非当前值 =0正常(缺省)
					D31＝1 闭锁 =0正常(缺省)
					*/
	long  lA;     /*系数      缺省1*/
	long  lB;     /*满度值    缺省1*/
	long  lC;     /*修正值    缺省0     最后运算 vlaue*a/b+c*/
	WORD  wFrzKey; //数据冻结关键字	

	long  lLimit1;	    //限值1
	WORD  wLimitT1;		//限值1时间 S
	WORD  wLimit1VYxNo;  //限值1对应虚遥信号
	long  lLimit2;	    //限值2
	WORD  wLimitT2;		//限值2时间 S
	WORD  wLimit2VYxNo;	//限值2对应虚遥信号
	DWORD rsv[5];
};

struct VPTrueYX{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;  /*D0=1 发送(缺省) =0 不发送
                    D1=1 取反 =0 不取反(缺省)
                    D2=1 数据库产生SOE  =0 不产生(缺省)
                    D3=1数据库产生COS  =0 不产生(缺省)
                    D4=1 产生复归信号  =0 不产生(缺省)

					D29＝1 被取代 =0正常(缺省)
					D30＝1 非当前值 =0正常(缺省)
					D31＝1 闭锁 =0正常(缺省)*/
	DWORD rsv[5];
};

struct VPTrueDD{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;  /*D0=1 发送(缺省) =0 不发送
	                D1=1 曲线 =0 不生成曲线(缺省)
	                D2=1 日冻结 =0无日冻结(缺省)
	                D3=1 月冻结 =0无月冻结(缺省)
	                
					D29＝1 被取代 =0正常(缺省)
					D30＝1 非当前值 =0正常(缺省)
					D31＝1 闭锁 =0正常(缺省)
				*/
    long  lA;     /*系数      缺省1*/
    long  lB;     /*满度值    缺省1*/    
    long  lC;     /*原始值    缺省0*/
	WORD  wFrzKey; //数据冻结关键字
	DWORD rsv[5];
};

struct VPTrueCtrl{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;
    WORD wID;
	DWORD rsv[5];
};

struct VPTSDataCfg{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;	  /*D0=1 转换为虚拟遥信 =0不转换(缺省)
				        D1=1 转换为合 =0 转换为分*/
    WORD wKeyWordNum;
    BYTE abyKeyWord[14];
    WORD awOffset[14];
    WORD wVYXNo; /*对应虚遥信点号*/
	DWORD rsv[5];
};

#define FA_VEQP_FLAG        0x80000000
struct VPVECfg{
	WORD wSourceAddr;   /*本机地址*/
	WORD wDesAddr;	    /*上级装置地址*/
	char sExtAddr[12];
	char sTelNo[20];	/*手机号码*/

    DWORD dwFlag;       /*D0=1:遥测乘系数， 
                          D1-D15: 保留*/

	WORD wYCNum;
	WORD wDYXNum;
	WORD wSYXNum;
	WORD wDDNum;
	WORD wYKNum;
	WORD wYTNum;
	WORD wTQNum;
	DWORD rsv[5];   
};

struct VPVirtual{  /*所有虚装置元素的结构头均必须与此结构一致*/
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;       
    DWORD dwTEName;   /*引用的实装置的Name*/
    WORD wOffset ;    /*在实装置中的偏移*/
    WORD wSendNo;    /*发送序号*/
};

struct VPVirtualYC{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;      /*D0-D7 保留
                        D9=1 主动发送禁止 =0主动发送允许(默认)
                        D11=1生成曲线、极值。=0 不生成曲线极值
	                  */
    DWORD dwTEName;   /*引用的实装置的Name*/
    WORD wOffset ;    /*在实装置中的偏移*/
    WORD wSendNo;    /*发送序号*/
    long  lA;  /*系数      缺省1*/
    long  lB;  /*满度值    缺省1*/
    long  lC;  /*修正值    缺省0     最后运算 vlaue*a/b+c*/ 
	WORD  wFrzKey;
	DWORD rsv[5];   
};

struct VPVirtualYX{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;       /*D0=1  相同发送序号与操作
                           =0  相同发送序号或操作
                         D1-D15 保留*/
    DWORD dwTEName;   /*引用的实装置的Name*/
    WORD wOffset ;    /*在实装置中的偏移*/
    WORD wSendNo;    /*发送序号*/
	DWORD rsv[5];   
};

struct VPVirtualDD{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;      /*D0=1  负相电度（总加时做减）
                          =0  正相电度（总加时做加）
						D1=1 曲线(缺省) =0 不生成曲线(缺省)	
                        D2-D15 保留*/ 
    DWORD dwTEName;   /*引用的实装置的Name*/
    WORD wOffset ;    /*在实装置中的偏移*/
    WORD wSendNo;    /*发送序号*/
	WORD  wFrzKey;
	DWORD rsv[5];   
};

struct VPVirtualCtrl{ 
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;       
    DWORD dwTEName;   /*引用的实装置的Name*/
    WORD wOffset ;    /*在实装置中的偏移*/

    WORD wID;
	DWORD rsv[5];   
};

struct VPEGCfg{
	WORD wSourceAddr;  /*本机地址*/
	WORD wDesAddr;     /*上级装置地址*/

	DWORD dwFlag;      /*D0: =0转发FA(默认) =1 不转发
	                     D1: =0转发QcEvent(默认) =1不转发
	                     D2: =0转发ActEvent(默认) =1不转发
	                     D3: =0转发DoEvent(默认) =1不转发
	                     D4: =0转发WarnEvent(默认) =1不转发
	                     D5-D31:保留*/            
	WORD wEqpNum;

	DWORD rsv[5];   
};

#ifdef DB_EPOWER_STAT
struct VPEECfg{ 			/*epower电表装置配置*/
	DWORD dwCfg1; 			  /*D0-生成日最大需量
								D1-生成月最大需量
								D2-有无功日最大需量冻结
								D3-有无功月最大需量冻结
								D4-象限无功日最大需量冻结
								D5-象限无功月最大需量冻结
								*/	  
	DWORD dwCfg2; 		   
	WORD wPQ_DD_Frz_Cyc;	  /*min*/
	WORD wXX_DD_Frz_Cyc; 

	DWORD dwCBTimeCfgDay;   /*D0-D30对应每月1日-31日*/
	BYTE  byCBTimeCfgHou;
	BYTE  byCBTimeCfgMin;  

	DWORD adwRsv[8];	
};	
#endif

//注意:addr.cfg逻辑文件比较特殊,它固定存储在了.ind文件之首,
//而不是.pkg文件中,因此对应add.cfg逻辑文件要特殊处理
struct VPIndInfo{
    char cfgName[MAXFILENAME];  
	int cfgOffset;   //逻辑文件在pkg文件中的偏移
	int cfgLen;    //逻辑文件长度，和逻辑文件头一致
	WORD cfgVer;  //逻辑文件版本，和逻辑文件头一致
	WORD cfgCrc;  //逻辑文件校验
	WORD pkgIndex;  //该逻辑文件所在的pkg文件序号
	DWORD dwRsv[5];
};

//每条pkg信息对应的文件名固定为cscxx.pkg  xx为该信息序号的两位十进制数表示
//如第一条pkg的信息文件名为csc00.pkg 以此类推.
struct VPPkgInfo{
	char pkgName[MAXFILENAME];
	int pkgLen;     /*pkg文件长度，包括文件头,数据,不包括校验，与pkg文件头一致*/
	WORD pkgVer;    /*pkg版本号，十进制数表示,次版本号占两位.如1.03为103，与pkg文件头一致*/
	WORD pkgCrc;    /*pkg文件校验，与pkg文件尾一致*/
	int pkgValid;   /*该条记录信息是否有效,有效为1,无效为0*/
	DWORD rsv[5];
};	


#define MAX_PKG_NUM   40
#define MAX_PKG_SIZE (50*1024)  /*50k*/
	
struct VPIndHead{
	struct VPAddr addr;      /*addr.cfg逻辑文件存储区*/
	WORD wAddrVer;    /*addr.cfg逻辑文件版本*/
	int nLength;      /*文件长度，包括文件头,数据,不包括校验*/
	WORD wVersion;    /*十进制数表示,次版本号占两位.如1.03为103*/
	WORD indNum;      /*文件索引信息个数*/
	DWORD rsv[5];
	struct VPPkgInfo pkgInfo[MAX_PKG_NUM];
};	

struct VPPkgHead{
	DWORD nLength;   /*文件长度，包括文件头,数据,不包括校验*/
	WORD wVersion;   /*十进制数表示,次版本号占两位.如1.03为103*/
	WORD pad;         
	DWORD rsv[5];
};	

struct VFileMsg{
    BYTE type;
	BYTE attr[5];
};

struct VFileMsgs{
	BYTE type;
    BYTE num;
	struct VFileMsg filemsg[20];
};

struct VFileSynchro{
	BYTE num;
	WORD wTaskID[10];
};

#pragma pack()
extern struct VFileHead *g_pParafileTmp;
extern DWORD g_paraFTBSem;

WORD GetParaCrc16(const BYTE *p, const int l);
DWORD GetParaCrc32(BYTE *p, int l);

int ReadParaFile(const char *filename, BYTE *buf, int buf_len);
int WriteParaFile(const char *filename, struct VFileHead *buf);
int DelParaFile(const char *filename);
int ReadParaFileCrc(const char *filename, WORD *crc);

int GetAppPortId(int para_port_id);
int GetParaPortId(int app_port_id);

int CreateSysConfig(const char *path);
int CreateExtIoConfig(char *path);

int CreateSysAddr(const char *path, struct VPAddr *pPAddr);
void CreateRunParaConfig(void);
#ifdef INCLUDE_CELL
int CreateCellConfig(void);
#endif
int SaveSysAddr(const char *path, const struct VPAddr *pPAddr);

int AddOnePort(const char *path, const struct VPPort *pPAddPort);

int CreateMinEqpSystme(const char *path);

int AddOneTrueEqp(char *path, DWORD type, struct VPTECfg *pPTECfg, struct VPEqp *pPAddEqp);

int SysParaInit(void);
BOOL ReadSelfRunParadef(WORD num,BYTE* buf);
int SetSysAddr(const WORD *addr, const char *ip1, const char *ip2, const char *gateway1, const char *gateway2);
int ReadPtCt(int fd, int *pt, int *Un, int *ct, int *In, int *ct0, int *In0);
int SavePtCt(int fd, const int *pt, const int *Un, const int *ct, const int *In, const int *ct0, const int *In0);
void WriteMsgFile(BYTE *pbuf);
void ReadfileSynchro(WORD wTaskID, WORD wEqpID, struct VFileSynchro *VFSyn);	
#ifdef __cplusplus
}
#endif

#endif
