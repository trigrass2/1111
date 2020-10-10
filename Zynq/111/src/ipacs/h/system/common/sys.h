/*------------------------------------------------------------------------
 Module:        sys.h
 Author:        solar
 Creation Date: 2008-08-01
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 12 $
------------------------------------------------------------------------*/

#ifndef _SYS_H
#define _SYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syscfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os.h"
#include "db.h"
#include "para.h"
#include "myio.h"
#include "prLogic.h"
#include "bsp.h"


#define  SYS_ID                0
#define  WATCHDOG_ID           1
#define  SELF_DIO_ID           2
#define  PRLOW_ID              3
#define  MAINT_ID              4
#define  SELF_MEA_ID           5

#define BM_EQP_MYIO             0
#define BM_EQP_LINE1            1
#define BM_EQP_LINE2            2
#define BM_EQP_SOCFA            3

/*D0-D15 系统错误*/
#define  SYS_ERR_SYS         0x00000001   /*系统错误,如资源短缺,任务异常退出等*/
#define  SYS_ERR_ADDR        0x00000002   /*地址有冲突*/
#define  SYS_ERR_CFG         0x00000004   /*参数错误或硬件跳线错误*/
#define  SYS_ERR_OBJ         0x00000008   /*动态库错误*/
#define  SYS_ERR_COMM        0x00000010   /*通信错误*/
#define  SYS_ERR_GAIN        0x00000020   /*交采整定文件错误*/
#define  SYS_ERR_LOGIN       0x00000040   /*板卡型号与上级参数不一致,登录失败*/
#define  SYS_ERR_SUBCFG      0x00000080   /*下级参数错误*/
#define  SYS_ERR_AD          0x00000100   /*AD异常告警*/
#define  SYS_ERR_AIO         0x00000200   /*模拟板异常告警*/
#define  SYS_ERR_DIO         0x00000400   /*数字板异常告警*/
#define  SYS_ERR_SLT         0x00000800   /*SLT 告警*/
#define  SYS_ERR_PR          0x00080000   /*保护告警*/
#define  SYS_ERR_FILE        0x00100000   /*文件系统告警*/
#define  SYS_ERR_SRAM        0x00200000   /*sram告警*/
#define  SYS_ERR_FAT         0x00400000   /*sram告警*/
#define  SYS_ERR_JIAMI      0x00800000  /*加密芯片没焊或异常*/
#define  SYS_ERR_TMP        0x01000000 /*公共单元温度芯片没焊或异常*/

/*Cfg Attr,此定义与底板类型定义有关联,见syscfg.h*/
#define  CFG_ATTR_NULL       0x0000       /*无定义,如port.cfg,device.cfg中的attr*/
#define  CFG_ATTR_IO         0x0001       /*本机I/O对应的cfg属性*/
#define  CFG_ATTR_USEDEF     0x0002       /*自定义设备对应的cfg属性*/
#define  CFG_ATTR_TABLE      0x0003       /*发送表设备对应的cfg属性*/
#define  CFG_ATTR_GROUP      0x0004       /*设备组对应的cfg属性*/
#define  CFG_ATTR_DYQC       0x0005       /*低压无功及数据处理模块*/
#define  CFG_ATTR_FA         0x0006       /*FA实设备*/
#define  CFG_ATTR_IPACS      0x1000       /*IPACS系列对应的cfg属性*/


enum {
	SYSEV_FLAG_NULL = 0,
	SYSEV_FLAG_FA,
	SYSEV_FLAG_ACT,
	SYSEV_FLAG_DO,
	SYSEV_FLAG_WARN,
	SYSEV_FLAG_DSOE,	
	SYSEV_FLAG_SSOE,
	SYSEV_FLAG_ASSERT,
#ifdef INCLUDE_B2F_COS
	SYSEV_FLAG_DCOS,
	SYSEV_FLAG_SCOS,
#endif
#if (DEV_SP == DEV_SP_QC)
	SYSEV_FLAG_QC,
#endif	
	SYSEV_FLAG_CO,
	SYSEV_FLAG_FLOW,
	SYSEV_FLAG_ULOG,
	SYSEV_FLAG_HL,
	SYSEV_FLAG_OL,
	MAX_SYSEV_FLAG_NUM
};	

struct VSysCfgExt{
	char sWsName[GEN_NAME_LEN];
	char sWsSechema[GEN_NAMELONG_LEN];
	DWORD dWsIp;
	DWORD dWsVer;

	char  sMaintIp[20];
	BYTE  byMac[6];

	DWORD dwKgTime;

	VDiCfg YxIn[8];

	char xmlVer[16];
	char xmlRev[16];
	DWORD xmlCrc;
	short YXNO[12];
	struct VNetIP Lan3;
    char sGateWay3[20];
	char sRoute3[20];
	struct VNetIP Lan4;
    char sGateWay4[20];
	char sRoute4[20];
	BYTE YKDelayTime;
};

#define IONO_BUF_NUM  3
struct VIoNo{
	WORD wIoNo_Low;
	WORD wIoNo_High;
	WORD wNum;
};

struct  VMyConfig{
	DWORD dwType;        /*设备型号*/
	DWORD dwName;        /*设备标识名称，系统内唯一，组态软件不显示*/
	char  sByName[GEN_NAME_LEN];   /*设备名称,用户可在组态软件定义和修改*/
	DWORD dwCfg;        			/*D0=1 蓄电池自动活化
	                 							   D1=1 sntp对时
	                 							   D2 =1 ，遥测1次值上传维护遥测界面
	                 							   D3 = 1,国网版本,规约基本信息扩展
	                 				 			   D3 = 0，以前版本规约信息没扩展，电流电压死区没有	
                                                   D4 = 1零序电压小信号，D4 = 0零序电压额定正常  
	                 				 			*/

	DWORD dwAIType;
	DWORD dwIOType;
	BYTE  byCPLDType;
	BYTE  byYcNum[BSP_AC_BOARD];
	BYTE  byYxNum[BSP_IO_BOARD];
	BYTE  byYkNum[BSP_IO_BOARD];

	WORD  wAllIoFDNum;

	WORD  wAllIoAINum;
	WORD  wAllIoDINum;
	WORD  wAllIoDONum;
	
	WORD  wAllIoYCNum;
	WORD  wAllIoDYXNum;
	WORD  wAllIoSYXNum;
	WORD  wAllIoDDNum;
	WORD  wAllIoYKNum;	
	WORD  wAllIoYTNum;

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

    WORD  wYCNumPublic;
	WORD  wYCNumAdd;
	//0-di, 1-di_public, 2-softyx_public+softyx_fd
    struct VIoNo SYXIoNo[IONO_BUF_NUM];
	//0-do, 1-do_public, 2-softyk
    struct VIoNo YKIoNo[IONO_BUF_NUM];

#ifdef INCLUDE_CELL
	VCellCfg CellCfg;
#endif
#ifdef INCLUDE_USWITCH
	VUSwitchCfg USwitchCfg;
#endif
	VRunParaCfg RunParaCfg; 

    DWORD dSntpServer;

	VFdCfg *pFd;
#ifdef INCLUDE_FA_PUB
	VFdCfg *pFaPrFd;
#endif

	VAiCfg *pAi;
    VDiCfg *pDi;
	VDoCfg *pDo;
	
	VYcCfg *pYc;
	
	struct VSysCfgExt SysExt;
};


struct VSysInfo{
	
	DWORD dwErrCode; 
	struct VMyConfig MyCfg;	
	DWORD dwAioType;
	DWORD dwDioType;
	BYTE  byCpldVer;
};

extern struct VFileHead *g_pParafile;
extern struct VSysInfo g_Sys;  

STATUS extNvRamGet(DWORD offset, BYTE *buf, int len);
STATUS extNvRamSet(DWORD offset, BYTE *buf, int len);
void WriteActEvent(struct VCalClock *ptm, DWORD type, int fd, const char *fmt, ... );
void WriteDoEvent(struct VCalClock *ptm, int para, const char *fmt, ... );
void WriteWarnEvent(struct VCalClock *ptm, DWORD errcode, int para, const char *fmt, ... );
void WriteFlowEvent(struct VCalClock *ptm, DWORD type, int value, struct VSysFlowEventInfo *pflow);
void WriteUlogEvent(struct VCalClock *ptm, DWORD type, int value, const char *fmt, ... );
void WriteFaEvent(struct VCalClock *ptm, const char *fmt, ... );
int myprintf(int thid, int attr, const char *fmt, ... );
void shellprintf( const char *fmt, ... );
void BSP_IO_Init(void);
void ClockInit(void);
void WriteRangeYCF_L(WORD wEqpID,WORD no, long value);
void ReadRangeYCF_L(WORD wEqpID,WORD no, long* value);
void WriteSYX(WORD wEqpID,WORD no, BYTE value);
void WriteDYX(WORD wEqpID,WORD no, BYTE value1,BYTE value2);
void WriteSSOE(WORD wEqpID,WORD no, BYTE value, struct VCalClock tm);
void WriteDSOE(WORD wEqpID,WORD no, BYTE value1,BYTE value2, struct VCalClock tm);
void WriteMyDiValue(WORD no,  WORD value);
void maint(void);
void ReadSYX(WORD wEqpID,WORD no, BYTE* value);
void WriteRunPara(BYTE *pbuf,WORD len);
BYTE* ReadRecordBuf(int *len);
int WriteBmRecord(WORD fd, WORD smpfreq,WORD smpnum, struct VCalClock tm,struct VCalClock prtm);
int WriteBmEvent(int flag,BYTE* buf, int len);
void ReadLinuxLed(DWORD *mask, DWORD* val);
int WriteMyioType(	DWORD dwAIType ,DWORD dwIOType);
DWORD GetAiType(void);
DWORD GetIoType(void);
void BMSetYkYb(DWORD yb);

#ifdef __cplusplus
}
#endif

#endif

