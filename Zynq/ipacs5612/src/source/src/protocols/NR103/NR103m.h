#ifndef _NR103M_H_
#define _NR103M_H_

#include "../public/pmaster.h"

//控制方向ASDU类型
#define TYPE_ASDU06H_TIME			0X06	//对时
#define TYPE_ASDU07H_ALLCALL		0X07	//启动总查询
#define TYPE_ASDU08H_CALLOVER		0X08	//总查询结束
#define TYPE_ASDU14H_NORMAL		0x14//一般命令
#define TYPE_ASDU15H_GEN_READ	0X15	//通用分类读命令
#define TYPE_ASDU23H_RDSTATE		0X17	//被记录的扰动表
#define TYPE_ASDU18H_RDDATACALL	0X18	//扰动数据传输的命令
#define TYPE_ASDU19H_RDDATACON	0X19	//扰动数据传输的认可 
#define TYPE_ASDUDCH_GEN_CALL		0XDC	//通用分类读命令

//监视方向ASDU类型
#define TYPE_ASDU05H_FLAG 				0X05	//标识
#define TYPE_ASDU0AH_GEN_GROUPDATA	0X0a	//通用分类数据响应命令（装置响应的读一个组的描述）
#define TYPE_ASDU0BH_GEN_DATA		0X11	//通用分类数据响应命令（装置响应的读单个条目的目录）
#define TYPE_ASDU1CH_SOE				0X1C	//带标志的状态变位传输准备就绪
#define TYPE_ASDU1DH_SOE				0X1D	//传送带标志的状态变位
#define TYPE_ASDU40H_YK				0X40	//南瑞科技老版本装置只能用0x40遥控
#define TYPE_ASDU29H_YK				0X29	//遥控否定确认



//传送原因
#define COT_SPONT 	1 	//突发
#define COT_PERCYC	2 	//循环
#define COT_TIMER		8		//时钟同步
#define COT_ALLCALL		9		//总查询
#define COT_COMCMD		20		//一般命令
#define COT_RDTRANS		31		//扰动数据传输
#define COT_GCWRITE		40		//通用分类写命令
#define COT_GCREAD		42		//通用分类读命令

//通用分类数据，数据类型
#define TYPE_NODATA 	0 // 无数据
#define TYPE_ASCII 		1 // OS8ASCII（ASCII 8 位码）
#define TYPE_BSI 		2 // 成组8 位串（BS1）
#define TYPE_UI 			3 // 无符号整数（UI）
#define TYPE_I 			4 // 整数（I）
#define TYPE_UF 			5 // 无符号浮点数（UF）
#define TYPE_F 			6 // 浮点数（F）
#define TYPE_SI754 		7 // IEEE 标准754 短实数（R32.23）
#define TYPE_I754 		8 // IEEE 标准754 实数（R64.53）
#define TYPE_DYK 		9 // 双点信息，见6.6.5
#define TYPE_SYK 		10 // 单点信息
#define TYPE_TRANSIENT_ERROR 	11 	// 带瞬变和差错的双点信息	0=瞬变（TRANSIENT）
									//	1=开（OFF）
									//2=合（ON）
									// 	3=出错（ERROR）
#define TYPE_YC_F	12 // 带品质描述的被测值见 6.6.8
#define TYPE_TIME_B 	14 // 二进制时间见 6.6.29
#define TYPE_GEN_NO 	15 // 通用分类标识序号见 6.6.31
#define TYPE_RET 		16 // 相对时间见 6.6.15
#define TYPE_TYPE_INF 	17 	// 功能类型和信息序号CP16{Type,INF}
							//功能类型(Type) = UI8[0～255]
							//信息序号(INF) = UI8[0～255]
#define TYPE_YX_TIME 	18 	// 带时标的报文
							//CP48{双点信息（DPI），备用（RES），四个8 位位组时间（TIME）附加信息（SIN）}
							//其中双点信息（DPI） = UI2[0～1]<0～3>，见6.6.5
							//备用（RES）= BS6[2～7]<0>
							//四个8 位位组时间（TIME）= CP32Time2a[8～39] 见6.6.28
							//附加信息（SIN） = UI8[40～47]<0～3>，见6.6.23
#define TYPE_YX_RET 	19 	// 带相对时间的时标报文
							//带相对时间的时标报文 = CP80{双点信息（DPI），备用（RES），
							//相对时间（RET），
							//故障序号（FAN）
							//四个 8 位位组时间（TIME），
							//附加信息（SIN）}
							//其中双点信息（DPI） = UI2[0～1]<0～3>，见6.6.5
							//备用（RES）= BS6[2～7]<0>
							//相对时间（RET） = UI16[8～23]，见6.6.15
							//故障序号（FAN）= UI16[24～39]，见6.6.6
							//四个8 位位组时间（TIME）= CP32Time2a[8～39] 见6.6.28
							//附加信息（SIN） = UI8[40～47]<0～3>，见6.6.23
#define TYPE_YC_RET 	20 	// 带相对时间的被测值
							//带相对时间的被测值= CP96{被测值（VAL），
							//相对时间（RET），
							//故障序号（FAN）
							//四个 8 位位组时间（TIME） }
							//其中被测值（VAL） = R32.23[0～31]
							//相对时间（RET） = UI16[32～47]，见6.6.15
							//故障序号（FAN）= UI16[48～63]，见6.6.6
							//四个8 位位组时间（TIME）= CP32Time2a[64～95] 见6.6.28
#define TYPE_STRUCT    23 //数据结构

#define TYPE_YX_CP56Time2a 203 	//七字节时标报文
								// CP72{双点信息（DPI），备用（RES），七个8 位位组时间（TIME），
								//附加信息（SIN）}
								//其中双点信息（DPI） = UI2[0～1]<0～3>，见6.6.5
								//备用（RES）= BS6[2～7]<0>
								//七个8 位位组时间（TIME）= CP56Time2a[8～63] 见6.6.29
								//附加信息（SIN） = UI8[64～71]<0～3>，见6.6.23
#define TYPE_YX_CP56Time2aRET 204 	// 带相对时间的七字节时标报文
									//带相对时间的时标报文 = CP104{双点信息（DPI），备用（RES），
									//相对时间（RET），
									//故障序号（FAN）
									//七个 8 位位组时间（TIME），
									//附加信息（SIN）}
									//其中双点信息（DPI） = UI2[0～1]<0～3>，见6.6.5
									//备用（RES）= BS6[2～7]<0>
									//相对时间（RET） = UI16[8～23]，见6.6.15
									//故障序号（FAN）= UI16[24～39]，见6.6.6
									//七个8 位位组时间（TIME）= CP56Time2a[40～95]
									//附加信息（SIN） = UI8[96～104]<0～3>，见6.6.23
#define TYPE_YC_CP56Time2a 205 		// 带相对时间七字节时标的被测值
									//带相对时间的被测值= CP120{被测值（VAL），
									//相对时间（RET），
									//故障序号（FAN）
									//七个 8 位位组时间（TIME） }
									//其中被测值（VAL） = R32.23[0～31]
									//相对时间（RET） = UI16[32～47]，见6.6.15
									//故障序号（FAN）= UI16[48～63]，见6.6.6
									//七个8 位位组时间（TIME）= CP56Time2a[64～119] 见
									
#define TYPE_YK_CP8 	206 			// 控制命令（1 byte）
									//控制命令=CP8 {CCS,DCS,RES,ACT,S/E }
									//控制命令状态=CCS:=UI3[1—3] <0—7>
									//<0>: = 不允许
									//<1>: = 选择同期工作方式
									//<2>: = 选择检无压工作方式
									//<3>: = 选择不检工作方式
									//<4>: = 选择合环工作方式
									//<5—7>: = 备用
									//双命令状态=DCS:=UI2[4—5] <0—3>
									//RES:=UI1[6]
									//ACT:=BS1[7] <0—1> <0>: = 命令有效
									//<1>: = 撤销
									//S/E:=BS1[8] <0—1> <0>: = 执行
									//<1>: = 选择


#pragma pack(1)
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


typedef union
{
	struct
	{
		BYTE NO:6;      	//D0-D5数目              
		BYTE COUNT:1;      // D6计数器位                
		BYTE CONT:1;      //D6后续状态位              
	} bit;                                   
	BYTE all;   	
}VNGD;

typedef struct _tagPacketHead
{
	WORD 	wFirstFlag; //0xEB90H
	DWORD 	wLength; //数据长度
	WORD 	wSecondFlag; //0xEB90H
	WORD 	wSourceFactoryId; //源厂站地址
	WORD 	wSourceAddr; //源设备地址
	WORD 	wDestinationFactoryId; //目标厂站地址
	WORD 	wDestinationAddr; //目标设备地址
	WORD 	wDataNumber; //数据编号
	WORD 	wDeviceType; //设备类型
	WORD 	wDeviceState; //设备网络状态
	WORD 	wFirstRouterAddr; //传输路径首级路由装置地址
	WORD 	wLastRouterAddr; //传输路径末级路由装置地址
	WORD 	wReserve1; //保留字节0xFFFF
	BYTE 	bTYPE;	//类型标识
	BYTE 	bVSQ;	 //
	BYTE 	bCOT;	//
	BYTE 	bASDU_ADDR;	//公共地址
	BYTE 	bFUN;
	BYTE 	bINF;
}NRFrameHead;

// 监视方向：
//INF信息序号	描述
#define INF_GROPT_M 	240	//240	读所有被定义组的标题
#define INF_ITEMALL_M	241	//241	读一个组的全部条目的值或属性
#define INF_ITEMDIR_M	243	//243	读单个条目的目录
#define INF_ITEMVALUE_M 	244	//244	读单个条目的值或属性
#define INF_CALLSTOP_M	245	//245	通用分类数据的总召唤终止
#define INF_CON_M	249	//249	带确认的写条目
#define INF_ACT_M	250	//250	带执行的写条目
#define INF_STOP_M	251	//251	带终止的写条目
// 控制方向：
//INF信息序号	描述
#define	INF_TIME_S 0//0			对时
#define	INF_GROPT_S 240//240	读所有被定义组的标题
#define	INF_ITEMALL_S 241//241	读一个组的全部条目的值或属性
#define	INF_ITEMDIR_S	243 //243	读单个条目的目录
#define	INF_ITEMVALUE_S 244//244	读单个条目的值或属性
#define	INF_CALLALL_S		245//245	通用分类数据的总召唤
#define	INF_WRITE_S	248//248	写条目
#define	INF_CON_S	249//249	带确认的写条目
#define	INF_ACT_S	250//250	带执行的写条目
//251	写条目的终止

typedef struct _tagApci
{
	BYTE bRII;	//返回信息标识
	VNGD bNgd; 
	BYTE bGroup;//组号
	BYTE bEntery;//条目号
	BYTE bKod;	//描述类别
	BYTE bDataType;
	BYTE bDataSize;
	BYTE bNum;//bit0-6num bit7后续状态位

}NRAPCI;
typedef struct _tagGIN
{
	BYTE bGroup;//组号
	BYTE bEntery;//条目号
	BYTE bKod;	//描述类别
	BYTE bDataType;
	BYTE bDataSize;
	BYTE bNum;//bit0-6num bit7后续状态位
}DataGroup;

#pragma pack()

class CNR103m : public CPPrimary
{
	public:
		NRFrameHead * m_pReceive; 	//接收帧头指针
		NRFrameHead * m_pSend;  		//发送帧头指针		
		struct V103Para *pCfg;
		
		BYTE m_YKno;
		WORD m_HeartBeat;
		WORD m_Offset;
		WORD m_DataNo;
		WORD m_dwTestTimerCount;
		DWORD event_time;
		BYTE m_YKflag;
		BYTE m_YKState;

		
	public:

		CNR103m();
		BOOL Init(WORD wTaskID);
		virtual void CheckBaseCfg(void);
		virtual void SetBaseCfg(void);
		virtual void CheckCfg(void);
		virtual DWORD DoCommState(void);
		virtual void DoTimeOut(void);
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
		virtual BOOL DoReceive();
		BOOL Chose_Dev(WORD no,BYTE bFun,BYTE bInf,BYTE *Gn,BYTE Flag);
		void DoGenGroupData_Asdu0A();
		void DoRecDYK();
		void DoRecSYK();
		void DoStruct();
		void DoRecYC754();
		void DoRecYC_F();
		void DoRecYX_TIME();
		void DoGenData_Asdu0B();
		int Chose_ID(int YkNo,BYTE *fun,BYTE *inf);
		void SendFrameHead(BYTE bTYPE, BYTE bVSQ, BYTE bCOT, BYTE bFUN, BYTE bINF);
		void SendFrameTail();
		virtual BOOL ReqCyc(void);
		virtual BOOL ReqNormal(void);
		virtual void DoYkReturn();
		virtual BOOL DoYKReq(void);
		BOOL SendYkCommand(void);
		void SendSetClock();
		void SendAllCall();
		void DoRecYX_RET();
		void  DoRecYC_RET();
		void  DoRecYX_CP56Time2a();
		void  DoRecYX_CP56Time2aRET();
		void  DoRecYC_CP56Time2a();
		void  DoRecYK_CP8();
		void SendHeart();

};



#endif
