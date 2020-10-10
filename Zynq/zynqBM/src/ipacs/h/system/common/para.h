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

#define  MAXFILELEN   (80*1024)

#define CFG_FILE_VER          100   /*1.00*/

#define GEN_NAME_LEN          40
#define GEN_NAMELONG_LEN      128

#define  MAXFILENAME  32           /*文件名最长为32*/

#define PBYXNUM     	 12

#define PARACHAR_MAXLEN  64
#define Old_MAXLEN   	 30






#pragma pack(1)

struct VParaInfo{
	BYTE type;
	BYTE len;
	char valuech[PARACHAR_MAXLEN];
};	

struct VFileHead{
	int nLength;	  /*文件长度，包括文件头,数据,文件校验置于ind文件中*/
	WORD wVersion;	  /*十进制数表示,次版本号占两位.如1.03为103*/
	WORD wAttr; 	  /*参数属性*/
	DWORD rsv[5];
};

#define FD_CFG_PT_58V   0x20      

struct VPFdCfg{
	char kgname[GEN_NAME_LEN];
	DWORD kgid;  /*界面不显示，自动取装置地址缀上回线号，如地址为2的第1回线，为201*/

	DWORD cfg;	 /*D0 = 1  电压等级为低压  =0 电压等级为中压(默认)
				   D1 = 1  交采一次侧数据上传时减少一位有效位(非默认)
				   D2 = 1  交采一次侧功率上传时单位为Mw或Mvar(非默认)
				   D3 = 1  无功补偿功能有效  =0 无无功补偿(默认)
				   D4 = 1  故障检测无效 =0 故障检测有效(默认)
				   D5 = 1  电压接入为相电压 =0 接入为线电压(默认)
				 */

	WORD Un;	  /*二次额定电压:220V或100V,值取220或100*/
	WORD In;	  /*二次额定电流:5A或1A,值取5或1*/
	
	int pt; 	  /*PT设置：X/Un*/
	int ct; 	  /*CT设置：X/In*/
	
	char  kg_stateno;	 //对应开关状态硬遥信点号
	char  kg_faultno;	 //对应开关故障硬遥信点号, -1表示系统软遥信
	char  kg_ykno;		 //对应开关遥控点号
	char  kg_vectorno;	 //对应开关故障方向硬遥信点号, -1表示系统软遥信 	   

	BYTE  kg_stateno_ioaddr;  //开关状态遥信对应的io插件地址, 0为本机
	BYTE  kg_faultno_ioaddr;  //开关故障遥信对应的io插件地址, 0为本机
	BYTE  kg_ykno_ioaddr;	  //开关遥控对应的io插件地址, 0为本机
	BYTE  kg_vectorno_ioaddr; //开关故障方向遥信对应的io插件地址, 0为本机
	
	WORD  In0;
	WORD  rsv1;
	int   ct0;

	char  kg_closeno;		//对应开关手合遥信
	char  kg_openno;	  //对应开关手分遥信
	char  kg_startno;	   //对应开关启动遥信
	char  kg_unlockno;	//对应开关闭锁遥信

	char kg_ykkeepno;	//对应遥控保持的点号
	char kg_remoteno;	//环网柜远方对应的遥信点号
	BYTE rsv2[2];
	DWORD rsv[4];	
};	

struct VPAiCfg{
	int type;	   //见YC_TYPE
	int rsv1;	 
	DWORD  cfg;    //暂为0
	DWORD admin;
	DWORD admax;
	DWORD rsv[3];
};

struct VPDiCfg{
	int type;	  /*=0 遥信（单点）
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
	int type;			/*=0 双点型 (合分双继电器)
						  =1 单点型 (仅一个继电器)
						  当单点型时ontime无效
						*/		
	DWORD cfg;			//暂为0
	DWORD hzontime; 	  /*遥控闭合时间,1ms单位，默认2000*/
	short ybno; 		/*压板配置: -2无压板
									-1软压板
							  >=0硬压板遥信号
						*/ 
	WORD pad;				  
	DWORD yztime;		/*遥控预置超时时间,1ms单位,默认30000*/	
	DWORD fzontime;
	DWORD rsv[2];		
};	

struct VPYcCfg{
	int type;
	DWORD  cfg; 	//暂为0
	
	int arg1;
	int arg2;
	short toZero;	  //归0值，缺省为5

	WORD bak[5];	
	DWORD rsv[5];	
};


struct	VPSysConfig
{
	DWORD dwType;	  /*设备型号(四字节） 由模板生成，程序员定义*/
	DWORD dwName;	  /*设备标识，系统内唯一，取(dwType&0xFFF)+该设备在系统内数目,不显示*/
	char  sByName[GEN_NAME_LEN];   /*设备名称,设备创建时用户在维护定义*/
	DWORD dwCfg;	 /*D0=1 蓄电池自动活化
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

struct VNetIP{
	char sIP[20];
	DWORD dwMask;
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

#pragma pack()

WORD GetParaCrc16(const BYTE *p, const int l);
void CreateRunParaConfig(void);

int WriteParaBmFile(const char * filename,BYTE * buf,int len);
int SysParaInit(void);
int ReadParaFile(char *filename, BYTE *buf, int buf_len);
int WriteParaFile(const char *filename, struct VFileHead *buf);
int CreateSysConfig(char *path);
void WriteFzT(WORD fztime);
void WriteHzT(WORD hztime);
void WriteYxFdT(WORD yxfdtime);
void WriteULP(WORD svalue);
void WriteILP(WORD svalue);
void WritePLP(WORD svalue);
void WriteFLP(WORD svalue);
void WriteZLLP(WORD svalue);
void WriteCosLP(WORD svalue);

#ifdef __cplusplus
}
#endif

#endif

