#ifndef PN103M_H
#define PN103M_H

#include "../public/pmaster.h"

//#define MAXIPACSDEVICENUM  12		//下面链接设备最大数量

/*************控制域的定义*************/
//控制方向
#define C_PRM				0x40
#define C_FCB				0x20
#define C_FCV				0x10


//监视方向
#define M_ACD				0x20
#define M_DFC				0x10
#define A_CMD				0X0F

#define VSQ_SQ_1			0X80


/*************功能码定义*************/
//主方
#define CMD_RESTU_M			0		//复位通讯单元
#define CMD_CONF_M			3		//传送数据
#define	CMD_DENY_M			4		//发送无回答
#define CMD_RESTFCB_M		7		//复位帧计数位
#define CMD_CALINSTATE_M	9		//召唤链路状态
#define CMD_CALPRIDATA_M	10		//召唤1级数据
#define CMD_CALSECDATA_M	11		// 召唤二级数据

//从方
#define CMD_CONF_S			0		//确认帧
#define CMD_LINBUSY_S		1		//链路忙
#define CMD_PAKGE_S			8		//以数据包响应请求帧
#define CMD_NUDATA_S		9		//从站无请求数据
#define CMD_LINSTATE_S		11		//链路状态响应


/*************TYPE定义*************/
#define TYPE_ASDU01_YBSTATE	0X01	//上送压板及告警等开关量状态
#define TYPE_ASDU02_PROINF		0X02	//上送保护动作信息
#define TYPE_ASDU05_SIGNINF	0X05	//标识报文
#define TYPE_ASDU06_TIME		0X06	//对时
#define TYPE_ASDU07_ALLCALL	0X07	//启动总查询
#define TYPE_ASDU08_CALLOVER	0X08	//总查询结束
#define TYPE_ASDU09_YC			0x09

#define TYPE_ASDU23_RDSTATE		0X17	//被记录的扰动表
#define TYPE_ASDU24_RDDATACALL	0X18	//扰动数据传输的命令
#define TYPE_ASDU25_RDDATACON		0X19	//扰动数据传输的认可 
#define TYPE_ASDU26_RDDATAACT		0X1A	//扰动数据传输准备就绪
#define TYPE_ASDU27_RDCOMMACT		0X1B	//被记录的通道传输准备就绪
#define TYPE_ASDU28_SIGNSTATE		0X1C	//带标志的状态变位传输准备就绪
#define TYPE_ASDU29_TRANSSTAT		0X1D	//带标志的状态变位传输
#define TYPE_ASDU30_TRSRDDATA		0X1E	//传输扰动值
#define TYPE_ASDU31_RDDATAOVER	0X1F	//扰动数据传输结束

#define TYPE_ASDU36_SENDDNL		0X24	//电能脉冲量上送
#define TYPE_ASDU38_STEPINF		0X26	//总查询及变位上送步位置
#define TYPE_ASDU39_STEPINFSOE	0X27	//步位置的SOE
#define TYPE_ASDU40_YX				0X28	//上送变位遥信
#define TYPE_ASDU41_SOE			0X29	//上送SOE
#define TYPE_ASDU44_YX				0X2C	//上送全遥信
#define TYPE_ASDU50_YC				0X32	//遥测上送
#define TYPE_ASDU64_YK				0X40	//遥控选择/执行/撤消
#define TYPE_ASDU88_CALLENERGY	0X58	//电能脉冲量召唤（冻结）
#define TYPE_ASDU42_YX				0X2A	//上送全遥信
//通用分类
#define TYPE_ASDU10_GEN_GROUPDATA	0X0a	//通用分类数据响应命令（装置响应的读一个组的描述）
#define TYPE_ASDU11_GEN_DATA		0X11	//通用分类数据响应命令（装置响应的读单个条目的目录）
#define TYPE_ASDU21_GEN_READ		0X15	//通用分类读命令
#define TYPE_ASDU51_YC				0X33	//遥测上送



/*************传送原因COT*************/
//从
#define COT_SPONT		1		//自发（突发）
#define COT_PERCYC		2		//循环
#define COT_RESETFCB	3		//复位帧计数位
#define COT_RESETCU		4		//复位通信单元
#define COT_START		5		//启动/重新启动
#define COT_PWRON		6		//电源合上
#define COT_TESTMODE	7		//测试模式
#define COT_SYNTIME		8		//时间同步
#define COT_CALL		9		//总查询
#define COT_CALLSTOP	10		//总查询终止
#define COT_LOCOP		11		//当地操作
#define COT_REMOP		12		//远方操作
#define COT_CMDACK		20		//命令的肯定认可
#define COT_CMDNACK		21		//命令的否定认可
#define COT_ADT			31		//扰动数据的传送
#define COT_GCWRACK		40		//通用分类写命令的肯定认可
#define COT_GCWRNACK	41		//通用分类写命令的否定认可
#define COT_GCRDACK		42		//对通用分类读命令有效数据响应
#define COT_GCRDNACK	43		//对通用分类读命令无效数据响应
#define COT_GCWRCON		44		//通用分类写确认

//主
#define COT_TIMER		8		//时钟同步
#define COT_ALLCALL		9		//总查询
#define COT_COMCMD		20		//一般命令
#define COT_RDTRANS		31		//扰动数据传输
#define COT_GCWRITE		40		//通用分类写命令
#define COT_GCREAD		42		//通用分类读命令

#define DCO_SE_EXE			0
#define DCO_SE_SELECT		0x80
#define DCO_DCS_OPEN		0x1
#define DCO_DCS_CLOSE		0x2

#define   FEN   0x00
#define   HE	0x01

#define    K	6
#define    W 	8


#define	COT_ACT		    6 	//激活
#define	COT_ACTCON		7 	//激活确认
#define	COT_DEACT		8 	//停止激活
#define	COT_DEACTCON	9 	//停止激活确认
#define	COT_ACTTERM		10 	//激活结束

#define YKENABLE                        0x08

#define SWS_ON				0x81
#define SWS_OFF				0x01

#define			ACD_ON		(USER_EQPFLAG + 0)

#define			TY_ON		(USER_EQPFLAG + 1)

#define       DATA_ONE			1

#define       DATA_TWO			2
	
#define       HEAD_V			0x68

#define       TAIL_H			0x16

#define FRM_I		0
#define FRM_S		1
#define FRM_U		3


#define  STARTDT_ACT 0x7
#define  STARTDT_CON 0x0B
#define  STOPDT_ACT  0x13
#define  STOPDT_CON  0x23
#define  TESTFR_ACT  0x43
#define  TESTFR_CON  0x83

#define MAX_FRAME_COUNTER   0x7fff

#define APCI_LEN    6
#define MAX_LEN		0xff

const int INF_3311[] = {112,20,22,24,31,168,159,33,34,160,158,200,28,95,86,40,41,44,47,50,51,113,114,201,210,3,4,5,7,11,12,13,230,231,241,252,219,221,222,255};
const int INF_3315SH[] = {118,119,37,50,51,44,40,41,210,3,4,5,7,11,12,13,230,231,241,219,244,221,222,255};
const int INF_3315A[] =  {30,31,32,33,71,73,74,75,76,77,150,153,9,70,190,182,79,1,254,7,204,8,11,206,186,237,13,181,227,255};
const int INF_3361[] = {64,65,98,101,104,105,44,45,46,69,96,97,118,123,200,28,29,22,23,50,51,38,39,106,107,108,201,3,4,5,7,11,12,13,241,242,243,247,219,221,222,255,108,201};
const int INF_3317[] = {30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,96,97,98,99,100,104,5,9,182,190,1,254,7,204,8,11,206,186,237,13,181,227};

#define Yk_value	0xee
#define  YK_SE		0x00
#define  YK_ACT		0x01
#define  YK_CA		0x02

typedef enum
{
	YK_SELECT = 0,	//yk选择
	YK_ACTION ,
	YK_REVOCAT 
}YKStep;


typedef enum
{
	LINK_DISCONNECT = 0,	//链路断开
	LINK_CONNECT ,			//链路建立
	LINK_IDLE,				//链路空闲
	LINK_BUSY,				//链路忙
	LINK_ERROR				//链路发生错误
}LinkSate;


typedef enum
{
	TRANSIDLE = 0,  //传输空闲
	TRANSSEND,		//需要发送
	TRANSWAIT ,		//等待应答
	TRANSRECV		//等到应答
	
}DeviceTransStep;

typedef enum
{
	SENDSECDATA = 0,//请求二级数据
	SENDPRIDATA ,	//请求一级数据
	SENDRESETFCB,	//复位帧计数位
	SENDRESETCU,	//复位通讯单元
	SENDCALLALL ,	//总查询
	SENDCALLENERGY,	//召唤电能量
	SENDYKSELECT,
	SENDYKACT,
	SENDYKREVOCAT,
	ALLFRAMETYPE	//总数量
}FrameType;

#pragma pack(1)

typedef struct
{
	BYTE addr;			//装置地址
	BYTE bfcb;			//帧计数位
	BYTE bfcv;			//帧计数有效位
	BYTE linkstate;		//链路状态
	
	WORD timercount[ALLFRAMETYPE];	//空闲累积时间	
	BOOL bReSendflg[ALLFRAMETYPE];	//重发标识
	BYTE bReSendTimes[ALLFRAMETYPE];//重发次数
	BYTE transstep[ALLFRAMETYPE]; 	//进程控制
}DeviceInfo;

typedef struct 
{	
	BYTE type;      //类型标识
	BYTE vsq;       //可变结构限定词
	BYTE cot;       //传送原因
	BYTE pubaddr;   //地址2
	BYTE fun;
	BYTE info;
	BYTE data[256-12]; //数据开始
}pn103Frame68;



typedef struct 
{
	BYTE byStartCode;
	BYTE byAPDULen;
	BYTE byControl1;
	BYTE byControl2;
	BYTE byControl3;
	BYTE byControl4;
	BYTE byASDU[MAX_LEN-6];	
}VIecpn103Frame;

typedef struct 
{
	BYTE head;       //启动字符
	BYTE control;    //控制域
	BYTE address;    //地址
	BYTE sum;        //校验码
	BYTE stop;       //结束码
}Ipc103Frame10;

typedef struct 
{
	BYTE groupno;
	BYTE entyrno;
	BYTE kod;
	BYTE datatype;
	BYTE datalen;
	BYTE datanum;
	BYTE data[MAX_LEN-1];
}TONGYONGINFO;


struct  sPara
{
	BYTE fun;
	BYTE inf;
	BYTE num;
};

struct VPara
{
	BYTE Gr_num;
	struct sPara  Para[5];
};

struct V103Para
{
	BYTE by103Cfg ;
	struct VPara YXPara ;
	struct VPara YCPara ;
	struct VPara DDPara ;
	struct VPara YKPara ;
};
#pragma pack()

class CPN103m : public CPPrimary
{
	public:
		VIecpn103Frame * m_pReceive; 		//接收帧头指针
		VIecpn103Frame * m_pSend;  		//发送帧头指针
		
		//DeviceInfo	SDeviceInfo[MAXIPACSDEVICENUM];
		DeviceInfo *SDeviceInfo;
		DWORD 	event_time ;
		BYTE m_dwTestTimerCount;
		struct V103Para *pCfg;
		BYTE Ycgroup_num;

		WORD m_wSendNum;	//Send Counter	
		WORD m_wRecvNum;	//Receive Counter
		WORD m_wAckNum;		//Receive Ack Counter
		WORD m_wAckRecvNum;    //Have Ack Receive Counter
		BOOL fla;

		BYTE ybfun;
		BYTE ybinf;
		BYTE ybflag;
		BYTE ybdco;
		BYTE ybnum;
		BYTE ybstate[32];
		
	public:
		CPN103m();
		BOOL Init(WORD wTaskID);
		virtual void CheckBaseCfg(void);
		virtual void SetBaseCfg(void);
		virtual void CheckCfg(void);
	
		virtual void DoTimeOut(void);
		virtual BOOL DoReceive();	
		//virtual void DoCommSendIdle(void);
		virtual DWORD SearchOneFrame(BYTE *Buf,WORD Len);
		
		virtual BOOL ReqUrgency(void);
		virtual BOOL ReqCyc(void);
		virtual BOOL ReqNormal(void);

		virtual DWORD DoCommState(void);

		void DoIFrame(void);
		void DoSFrame(void);
		void DoUFrame(void);

		BOOL SendYkCommand(void);
		
		BOOL RecFrame10();		
		BOOL SetACDflg(BYTE bAddr);	
		BOOL RecFrame68();		
		BOOL RecFrame10_Cmd0();		
		BOOL RecFrame68_ASDU01();		
		BOOL RecFrame68_ASDU02();		
		BOOL RecFrame68_ASDU05();		
		BOOL RecFrame68_ASDU08();		
		BOOL RecFrame68_ASDU23();		
		BOOL RecFrame68_ASDU26();		
		BOOL RecFrame68_ASDU27();		
		BOOL RecFrame68_ASDU28();		
		BOOL RecFrame68_ASDU29();		
		BOOL RecFrame68_ASDU30();		
		BOOL RecFrame68_ASDU31();		
		BOOL RecFrame68_ASDU32();		
		BOOL RecFrame68_ASDU42();	
		BOOL RecFrame68_ASDU38();		
		BOOL RecFrame68_ASDU39();		
		BOOL RecFrame68_ASDU40();		
		BOOL RecFrame68_ASDU41();		
		void Time4bytetoCalClock(BYTE *bTime4,VCalClock *CalCloc);		
		BOOL RecFrame68_ASDU44();		
		BOOL RecFrame68_ASDU50();		
		BOOL RecFrame68_ASDU64();	
		BOOL RecFrame68_ASDU10();
		BOOL RecFrame68_ASDU51();
		BYTE time_trans(struct VSysClock *tt,BYTE *a);
		BOOL DoReceviceYc(BYTE bAddr,BYTE bInf,WORD bData);		
		void WriteData(void);		
		BYTE MakeControlCode(BYTE bCmd,BYTE FramList);		
		BYTE MakeFrame10(BYTE bCmd,BYTE FramList);		
		BYTE MakeFrame68Head(BYTE type, BYTE cot, BYTE fun, BYTE info);	
		BYTE MakeFrame68_YB(BYTE type, BYTE cot, BYTE fun, BYTE info,BYTE flag,BYTE DCO);
		BYTE MakeFrame68_ReadYB(BYTE type, BYTE cot);
		BYTE ChkSum(BYTE *buf, WORD len);
		void SendFrame10_Cmd0();		
		void SendFrame10_Cmd07();		
		void SendFrame10_Cmd10();		
		void SendFrame10_Cmd11();		
		void SendFrame68_ASDU06(BYTE addr);		
		void SendFrame68_ASDU07();		
		void SendFrame68_ASDU64(int ykId,BYTE type,BYTE data,BYTE fun,BYTE inf);	
		void SendFrame68_ASDU21(BYTE flag);
		BOOL Chose_Dev(WORD no,BYTE bFun,BYTE bInf,BYTE *Gn,BYTE Flag);
		BYTE Get_Value(BYTE *buf,BYTE dateType,BYTE datelen,DWORD *val1,float *val2);
		void YkResult(BOOL Result);
		int judge_yc_dd(BYTE fun,BYTE inf);
		int GetNum_Yx();
		BYTE Get_ykdata(BYTE tt,BYTE dd);
		int Chose_ID(int YkNo,BYTE *fun,BYTE *inf);
		void SendUFrame(BYTE byControl);
		void SendSFrame(void);
		void SendFrameHead(BYTE byFrameType);
		void SendFrameTail();
};

#endif


