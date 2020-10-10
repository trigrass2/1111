/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef GB104PUB_H
#define GB104PUB_H
#include "syscfg.h"

#define GB104_V2002
#ifdef GB104_V2002
#define ADDR_YX_LO		0x0001
#define ADDR_YX_HI		0x4000

#define ADDR_YC_LO		0x4001
#define ADDR_YC_HI		0x5000
#define ADDR_YCPARA_LO		0x5001
#define ADDR_YCPARA_HI		0x6000

#define ADDR_YK_LO		0x6001
#define ADDR_YK_HI		0x6200

#define ADDR_YT_LO		0x6201
#define ADDR_YT_HI		0x6400

#define ADDR_DD_LO		0x6401
#define ADDR_DD_HI		0x6600

#define	ADDR_RCD_YC		0x680A  //2016 新增录波地址

#else
#define ADDR_YX_LO		0x001
#define ADDR_YX_HI		0x400

#define ADDR_YC_LO		0x701
#define ADDR_YC_HI		0x900

#define ADDR_YK_LO		0xb01
#define ADDR_YK_HI		0xb80

#define ADDR_YT_LO		0xb81
#define ADDR_YT_HI		0xc00

#define ADDR_DD_LO		0xc01
#define ADDR_DD_HI		0xc80
#endif

#define ADDR_BH_LO		0x401
#define ADDR_BH_HI		0x700


#define ADDR_PARA_LO	0x901
#define ADDR_PARA_HI	0xb00





#define ADDR_STEP_LO	0xc81
#define ADDR_STEP_HI	0xca0

#define ADDR_BIN_LO		0xca1
#define ADDR_BIN_HI		0xcb0

#define ADDR_BCD_LO		0xcb1
#define ADDR_BCD_HI		0xcc0

#define ADDR_RTU_STATE	0xcc1

#define ADDR_FILE_LO	0xcc2
#define ADDR_FILE_HI	0xe00



#define	M_SP_NA		1 	//不带时标的单点信息
#define	M_DP_NA		3 	//不带时标的双点信息
#define	M_ST_NA		5 	//步位置信息
#define	M_BO_NA		7 	//子站远动终端状态
#define	M_ME_NA		9 	//测量值
#define M_ME_TA   10  //带时标归一化值
#define	M_ME_NB		11	//标度化测量值add by lqh 20080329
#define M_ME_TB   12  //带时标标度化值
#define M_ME_NC   13  //测量值 短浮点数
#define M_ME_TC   14  //带时标短浮点数
#define	M_IT_NA		15 	//电能脉冲记数量
#define	M_PS_NA		20 	//具有状态变位检出的成组单点信息
#define	M_ME_ND		21 	//不带品质描述的测量值

#define M_SP_TB   30  //带长时标的单点信息
#define M_DP_TB   31  //带长时标的双点信息
#define M_ST_TB   32  //带长时标的步位置信息
#define M_BO_TB   33  //带长时标的32位位串
#define M_ME_TD   34  //带长时标的测量值，归一化值
#define M_ME_TE   35  //带长时标的测量值，标度化值
#define M_ME_TF   36  //带长时标的测量值，短浮点数

#define	M_EI_NA		70 	//初始化结束

#define	C_SC_NA		45 	//单点遥控命令
#define	C_DC_NA		46 	//双点遥控命令
#define	C_RC_NA		47 	//升降命令
#define	C_SE_NA		48 	//设定命令
#define	C_SE_NB_1		49 	//设定命令
#define	C_SE_NC_1		50 	//设定命令
#define	C_SE_TA_1		61 	//设点归一化
#define	C_SE_TB_1		62 	//设点标度化
#define	C_SE_TC_1		63 	//设点短浮点

#define	C_IC_NA		100 	//召唤命令
#define	C_CI_NA		101 	//电能脉冲召唤命令
#define	C_RD_NA		102 	//读数据命令
#define	C_CS_NA		103 	//时钟同步命令
#define C_TS_NA     104   	//测试命令
#define	C_RP_NA		105 	//复位进程命令
#define	C_CD_NA		106 	//复位进程命令
#define C_TS_TA     107   	//带时标的测试命令
#define P_ME_NA     110   	//参数
#define P_AC_NA     113   	//参数激活

/*104 - 2016 新增类型*/
#define M_FT_NA_1	42		//故障事件
#define C_SR_NA_1	200     //切换定值区
#define C_RR_NA_1	201	    //读定值区号
#define C_RS_NA_1	202     //读参数和定值
#define C_WS_NA_1	203     //写参数和定值
#define F_FR_NA_1	210	    //文件传输
#define F_SR_NA_1	211     //软件升级
/*******END*********/

/*2016 新增 录波文件传输类型标志*/
//#define F_FR_NA_1	120		//文件准备就绪
//#define F_SR_NA_1	121		//节准备就绪
#define F_SC_NA_1	122		//召唤目录.选择文件.召唤文件.召唤节
#define F_LS_NA_1	123		//最后的节，最后的段
#define F_AF_NA_1	124		//认可文件.认可节
#define F_SG_NA_1	125		//段
#define F_DR_NA_1	126		//目录



#define M_FA_NA		142		//FA故障信息
#define C_FA_SIM	143		//FA 模拟
#define C_YK_EXT	150		//群控
#define S_PROT_VAL	180		//保护定值
#define S_PROT_EVENT 181
#define S_File_List      182
#define S_File_Trans   183
#define M_IT_NB_1   206   //电能量累积量，短浮点数
#define M_IT_NC_1   207  //带时标的累积量，短浮点数

//传送原因：
#define	COT_PERCYC	  	1 	//周期/循环
#define	COT_BACK		2 	//背景扫描
#define	COT_SPONT		3 	//突发
#define	COT_INIT	  	4 	//初始化
#define	COT_REQ		    5 	//请求或被请求
#define	COT_ACT		    6 	//激活
#define	COT_ACTCON		7 	//激活确认
#define	COT_DEACT		8 	//停止激活
#define	COT_DEACTCON	9 	//停止激活确认
#define	COT_ACTTERM		10 	//激活结束
#define	COT_RETREM		11 	//远程命令引起的返送信息
#define	COT_RETLOC		12 	//当地命令引起的返送信息
#define	COT_FILE		13 	//文件传送

#define	COT_INTROGEN	20 	//响应总召唤
#define	COT_INRO1		21 	//响应第1组召唤
#define	COT_INRO2		22 	//响应第2组召唤
#define	COT_INRO3		23 	//响应第3组召唤
#define	COT_INRO4		24 	//响应第4组召唤
#define	COT_INRO5		25 	//响应第5组召唤
#define	COT_INRO6		26 	//响应第6组召唤
#define	COT_INRO7		27 	//响应第7组召唤
#define	COT_INRO8		28 	//响应第8组召唤
#define	COT_INRO9		29 	//响应第9组召唤
#define	COT_INRO10		30  //响应第10组召唤
#define	COT_INRO11		31 	//响应第11组召唤
#define	COT_INRO12		32 	//响应第12组召唤
#define	COT_INRO13		33 	//响应第13组召唤
#define	COT_INRO14		34 	//响应第14组召唤
#define	COT_INRO15		35 	//响应第15组召唤
#define	COT_INRO16		36  //响应第16组召唤
#define	COT_REQCOGCN	37  //响应计数量总召唤
#define	COT_REQCO1		38 	//响应第1组计数量召唤
#define	COT_REQCO2		39 	//响应第2组计数量召唤
#define	COT_REQCO3		40 	//响应第3组计数量召唤
#define	COT_REQCO4		41 	//响应第4组计数量召唤

#define	COT_PN_BIT		0x40

#ifdef INCLUDE_DF104FORMAT
	#define PARA_K		8
	#define PARA_W		4
#else
	#define PARA_K		12
	#define PARA_W		8
#endif
#define PARA_T0		30
#define PARA_T1		15
#define PARA_T2		10
#define PARA_T3		10

#define FRM_I		0
#define FRM_S		1
#define FRM_U		3

#define  STARTDT_ACT 0x7
#define  STARTDT_CON 0x0B
#define  STOPDT_ACT  0x13
#define  STOPDT_CON  0x23
#define  TESTFR_ACT  0x43
#define  TESTFR_CON  0x83
#define  REMOTE_MAINT  0xC3
#define MAX_FRAME_COUNTER   0x7fff

#define START_CODE          0x68

#define MIN_RECEIVE_LEN  	6    //最小接收帧长
#define MAX_FRAME_LEN  		255
  
#define APCI_LEN            6
#define CONTROL_LEN			4
#ifdef INCLUDE_DF104FORMAT
	#define ASDUID_LEN		5
#else
	#define ASDUID_LEN		6
#endif
#define ASDUINFO_LEN		MAX_FRAME_LEN - APCI_LEN - ASDUID_LEN
#define VSQ_NUM 		0x7f
#define VSQ_SQ  		0x80

#define YC_GRP_NUM		40
				#if(TYPE_USER == USER_GUANGXI)
#define YC_GRP_NUM		20
#endif
#define YX_GRP_NUM		128
#define DD_GRP_NUM		32	//dd num per group

#ifdef INCLUDE_DF104FORMAT
	#define SCOS_LEN		3
	#define SSOE_LONG_LEN	10
	#define CHANGE_YC_LEN	4
	#define	INFO_ADR_LEN	2
#else
	#define SCOS_LEN		4
	#define SSOE_LONG_LEN	11
	#define CHANGE_YC_LEN	5
	#define	INFO_ADR_LEN	3
#endif
		


#define ETHERNET_COMM_ON	1
#define ETHERNET_COMM_OFF	0
#define FILE_NAME_LEN	32

struct VIec104Timer
{
	BOOL bRun;
	DWORD wCounter;
	WORD wInitVal; 	
};

struct VIec104Frame
{
	BYTE byStartCode;
	BYTE byAPDULen;
	BYTE byControl1;
	BYTE byControl2;
	BYTE byControl3;
	BYTE byControl4;
	BYTE byASDU[MAX_FRAME_LEN-6];	
};

#ifdef INCLUDE_DF104FORMAT
	struct VASDU
	{
		BYTE byTypeID;	   
		BYTE byQualifier;
		BYTE byReasonLow;	
		BYTE byAddressLow;	
		BYTE byAddressHigh; 
		BYTE byInfo[MAX_FRAME_LEN - 12];	   
	};
#else
	struct VASDU
	{
		BYTE byTypeID;	   
		BYTE byQualifier;
		BYTE byReasonLow;	
		BYTE byReasonHigh;	
		BYTE byAddressLow;	
		BYTE byAddressHigh; 
		BYTE byInfo[MAX_FRAME_LEN - 12];	   
	};
#endif

struct VBackFrame
{
	WORD wSendNum;
	WORD wFrameLen;
	BYTE byBuf[MAX_FRAME_LEN];	
};

typedef 	struct 
	{
		DWORD TypeID;	   
		DWORD VSQ;
		DWORD COT;	
		DWORD Address;	
		DWORD Info;	   
		BYTE  Infooff;
		BYTE  COToff;
	} VDWASDU;

typedef struct{
BYTE typeidlen;/*信息类型长度*/
BYTE conaddrlen;/*应用层公共地址长度*/
BYTE VSQlen;/*可变结构限定词长度*/
BYTE COTlen;/*传送原因长度*/
BYTE infoaddlen;/*信息地址长度*/
BYTE yxtype;/*1.不带时标；2.带时标；3带CP56Time2a时标*/
BYTE yctype;/*9.测量值, 规一化值;10.带时标的测量值, 规一化值11.测量值, 标度化值12.测量值, 带时标的标度化值
              13.测量值, 短浮点数14.测量值, 带时标的短浮点数21.测量值, 不带品质描述词的规一化值
               34.带CP56Time2a时标的测量值, 规一化值35带CP56Time2a时标的测量值, 标度化值
		36带CP56Time2a时标的测量值, 短浮点数	*/
BYTE ddtype;/*15.累计量16带时标的累计量37带CP56Time2a时标的累计量*/
WORD k;/*发送未确认的最大次数，发送方停止发送*/
WORD w;/*接受到w个I帧，需要发送确认*/
WORD t0;
WORD t1;
WORD t2;
WORD t3;
WORD t4;
WORD baseyear;/*1900或2000*/
	WORD jiami;/*0加密*/
	WORD tmp[9];//tmp[6]用作级联时的fdno，如果为0则表示无效
				//tmp[7]福建读0x8243过压定值，0-额定值倍数，1-幅值
}Vgb104para;/*104规约参数结构*/
typedef struct{
	WORD limitchange;
	WORD pinghuashijian;
	WORD limitmax;
	WORD limitmmin;
}Vlimitpara;
typedef struct{
	WORD wSendSSOENum;
	WORD wSendDSOENum;
}VSendsoenum;
typedef struct{
	BYTE line;
	BYTE flag;
	BYTE off; 
}Vprotval;

typedef struct{

	BYTE flag;
	DWORD dirid;
	BYTE dirnamelen;
	char dirname[100];
	DWORD dwStrtTime;      /*目录/文件 创建时间 从1970年1月1日0点分0秒开始的时间,以秒表示*/
	DWORD dwStopTime;
	
} VFileDirInfo;

typedef struct{
	
	WORD flag; /*结果描述符 D0: 成功，D1未知错误 D2校验错误 D3文件长度不对应 D4文件ID与激活ID不一致*/
	BYTE filenamelen;
	char filename[100];
	DWORD fileid;
	DWORD datano;
	DWORD filesize;

} VFileInfo;

typedef struct{

	WORD filename;
	BYTE filelen[3];
	BYTE fileSOF;
	BYTE filetime[7];

}VFileDir;

typedef struct{
	WORD SoeNo;
	BYTE SoeValue;
	struct VCalClock Time;
	WORD DelayTime;
	BYTE flag;
}VDelaySoe;

typedef struct{
	WORD CosNo;
	BYTE CosValue;
	WORD DelayTime;
	BYTE flag;
}VDelayCos;


#endif
