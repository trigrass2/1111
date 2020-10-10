#ifndef SAC103P_H
#define SAC103P_H

#include "../public/pmaster.h"
#include "udpclass.h" 
//类型标识

//监视方向
#define M_TM_TA			1		//带时标的报文
#define M_TMR_TA		2		//具有相对时间的带时标的报文
#define M_MEI_NA		3		//被测值I
#define M_TME_TA		4		//具有相对时间的带时标的被测值
#define M_ID_NA			5		//
#define M_SYN_TA		6		//时间同步
#define M_TGI_NA		8		//总查询（总召唤）终止
#define M_MEII_NA		9		//被测值II
#define M_GD_NA			10		//通用分类数据
#define M_GI_NA			11		//通用分类标识
#define M_LRD_TA		23		//被记录的扰动表
#define M_RTC_NA		27		//被记录的通道传输准备就绪
#define M_RTT_NA		28		//带标志的状态变位传输准备就绪
#define M_TDT_TA		29		//传送带标志的状态变位
#define M_TDN_NA		30		//传送扰动值
#define M_EOT_NA		31		//传送结束

//控制方向
#define C_SYN_TA		6		//时间同步
#define C_IGI_NA		7		//总查询（总召唤）
#define C_GD_NA			10		//通用分类数据
#define C_GRC_NA		20		//一般命令
#define C_GC_NA			21		//通用分类命令
#define C_ODT_NA		24		//扰动数据传输的命令
#define C_ADT_NA		25		//扰动数据传输的认可


//传送原因

//监视方向
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

//控制方向
#define COT_YK			0X0C
#define COT_COMCMD		20		//一般命令
#define COT_GCWRITE		40		//通用分类写命令
#define COT_GCREAD		42		//通用分类读命令


//固定帧长报文功能码
//控制方向
#define CFUN_RESETCU		0x40
#define CFUN_RESETFCB		0x47
#define CFUN_LINKSTATUS		0x49
#define CFUN_CLASSONE		0x4a
#define CFUN_CLASSTWO		0x4b
#define CFUN_SENDACK		0x43
#define CFUN_SENDNOACK		0x44


//监视方向
#define MFUN_ACK			0 



//控制域的定义
//控制方向
#define C_PRM				0x40
#define C_FCB				0x20
#define C_FCV				0x10

//监视方向
#define M_ACD				0x20
#define M_DFC				0x10


//功能类型
#define FUNTYPE_JLBH		128
#define FUNTYPE_GLBH		160
#define FUNTYPE_BYQCDBH		176
#define FUNTYPE_XLCDBH		192
#define FUNTYPE_GEN			254
#define FUNTYPE_GLB			255

#define GRP_SYS             0x00

#define GRP_EVENT_STATUS		0x04
#define GRP_EVENT_ALARM			0x05

#define GRP_YC_BH			0x06
#define GRP_YC_COM			0x07

#define GRP_YX_STATUS		0x08
#define GRP_YX_COM			0x09
#define GRP_YX_SOE			0x18
				
#define GRP_YK_KG			0x0b
#define GRP_YK_YB			0x0e
#define GRP_YK_FT			0x0c

#define GRP_YT             0x0d

#define MIN_FRM_LEN			6
#define FRM_HEAD_LEN		4
#define START_BYTE_68		0x68
#define START_BYTE_10		0x10
#define STOP_BYTE			0x16

#define TCP_COMM_ON	1
#define TCP_COMM_OFF	0

#define YC_I_NUM			4
#define YC_II_NUM			9

#define UDP_PORT			0x408


#define YX_BEGIN_NO				0
#define YX_NUM					40//21
#define YB_BEGIN_NO				YX_BEGIN_NO + YX_NUM
#define YB_NUM					20//10
#define BHEVNT_BEGIN_NO			YB_BEGIN_NO + YB_NUM
#define BHEVNT_NUM				30//16
#define GJEVNT_BEGIN_NO			BHEVNT_BEGIN_NO + BHEVNT_NUM
#define GJEVNT_NUM				90//主变保护的保护事件也编码到告警事件里面了

#define YC_BEGIN_NO				0
#define YC_NUM					30//10
#define YKFT_BEGIN_NO			YC_BEGIN_NO + YC_NUM
#define YKFT_NUM				10

#define ASDU5_LEN				19

#define BHFG_YKNO				0


struct V103Frame
{
	BYTE TypeId;
	BYTE Qualifier;
	BYTE Reason;
	BYTE CommAddress;
	BYTE Info;
};





class CGB103P : public CPPrimary
{
	public:
		V103Frame *m_pReceive;
		V103Frame *m_pSend;
		BYTE m_byRecControl;
		BOOL m_bTcpConnect; 
		BYTE m_byRII;
		VLanIP UdpHost;
		VUDP m_vUdp;
		WORD m_wYtValue;
		WORD m_wTaskID;
	public:
	
		CGB103P();
		BOOL Init(WORD wTaskID);
		virtual void CheckBaseCfg(void);
		virtual void SetBaseCfg(void);
		
		virtual DWORD DoCommState(void);
		virtual void DoTimeOut(void);
		void SendUdpNetId(void);
		void SendUDP30s(void);
		void UdpCfgStrSet(int no);
		virtual BOOL DoReceive();
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
		BYTE GetFrameLen(BYTE *Buf, WORD Len);
		BYTE GetASDU10FrameLen(BYTE *Buf, WORD Len);
		void DoASDU10(void);
		void DoBhEvent(BYTE *Buf, BYTE DataLen);
		void DoGjEvent(BYTE *Buf, BYTE DataLen);
		void DoYcCom(BYTE *Buf, BYTE DataLen);
		void DoYxStatus(BYTE *Buf, BYTE DataLen);
		void DoYkYb(BYTE *Buf, BYTE DataLen);
		void DoSoe(BYTE *Buf, BYTE DataLen);
		void DoYkFt(BYTE *Buf, BYTE DataLen);
		
		virtual BOOL DoYKReq(void);
		void DoYkReturn(BYTE *Buf, BYTE InfoNo);
		BOOL SendYkCommand(void);
		virtual BOOL DoYTReq(void);
		void DoYtReturn(BYTE *Buf, BYTE InfoNo);
		BOOL SendYtCommand(void);
		
		virtual BOOL ReqCyc(void);
		
		BOOL GetTypeId(void);
		BOOL SetClock(void);
		BOOL GetAllData(void);
		
		void SendFrameHead(void);
		void SendFrameTail(void);
	
};



#endif

