/*------------------------------------------------------------------------
 Module:       	db.h
 Author:        solar
 Project:       
 State:         
 Creation Date:	2008-09-01
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 13 $
------------------------------------------------------------------------*/

#ifndef _DB_H
#define _DB_H

#ifdef __cplusplus
extern "C" {
#endif
#include "syscfg.h"
#include "clock.h"


#define YX_STATE(value)  (value&0x81)
#define YX_ISHE(value)   (YX_STATE(value)==0x81)
#define YX_ISFEN(value)  ((YX_STATE(value)==0x01)||(YX_STATE(value)==0x00))

#pragma pack(1)
struct VDBDYX{
  WORD wNo;
  BYTE byValue1;  
  BYTE byValue2;  
};

struct VDYX{
  BYTE byValue1;   
  BYTE byValue2;  
};

struct VDBDSOE{
  WORD wNo;
  BYTE byValue1;  
  BYTE byValue2;  
  struct VCalClock Time;
};

struct VDBDCOS{
  WORD wNo;
  BYTE byValue1;  
  BYTE byValue2;  
};

struct VDBYX{
  WORD wNo;
  BYTE byValue;     /*D0=1 有效 =0 无效
        			  D1=1 被封锁 =0 未被封锁
        			  D2=1 被取代 =0 未被取代
					  D3=1 非当前值 =0 当前值
                      D4-D6 保留
					  D7=1 合 =0 分  
                    */
};

struct  VDBSOE{
  WORD wNo;
  BYTE byValue;
  struct VCalClock Time;
};

struct VDBCO{
	WORD wNo;
	BYTE cmd;
	BYTE val;
	struct VCalClock Time;
};

struct VDBFLOW{
	WORD wNo;
	long val;
	struct VCalClock Time;
};

struct VDBCOS{
  WORD wNo;
  BYTE byValue;
};

struct VDBYCF_L{
  WORD wNo;
  long lValue;
  BYTE byFlag;      /*D0=1 有效 =0 无效
        			  D1=1 被封锁 =0 未被封锁
        			  D2=1 被取代 =0 未被取代
					  D3=1 非当前值 =0 当前值
                      D4=1 溢出 =0 未溢出
                      D5   保留
                      D6=1 浮点格式 =0 整形    
                      D7=1 主动发送禁止 =0 允许
                    */
};

struct VDBYCF_F{
  WORD wNo;
  float fValue;
  BYTE byFlag;      /*D0=1 有效 =0 无效
        			  D1=1 被封锁 =0 未被封锁
        			  D2=1 被取代 =0 未被取代
					  D3=1 非当前值 =0 当前值
                      D4=1 溢出 =0 未溢出
                      D5   保留
                      D6=1 浮点格式 =0 整形    
                      D7=1 主动发送禁止 =0 允许
                    */
};

struct VDBYCF{
  WORD wNo;
  short nValue;
  BYTE byFlag;      /*D0=1 有效 =0 无效
        			  D1=1 被封锁 =0 未被封锁
        			  D2=1 被取代 =0 未被取代
					  D3=1 非当前值 =0 当前值
                      D4=1 溢出 =0 未溢出
                      D5   保留
                      D6=1 浮点格式 =0 整形    
                      D7=1 主动发送禁止 =0 允许
                    */
};

struct VDBYC_F{
  WORD wNo;
  float fValue;  
};

struct VDBYC_L{
  WORD wNo;
  long lValue;  
};

struct VDBYC{
  WORD wNo;
  short nValue;  
};

typedef long VYC;

struct VYCF_F{
  float fValue;
  BYTE byFlag;      /*D0=1 有效 =0 无效
        			  D1=1 被封锁 =0 未被封锁
        			  D2=1 被取代 =0 未被取代
					  D3=1 非当前值 =0 当前值
                      D4=1 溢出 =0 未溢出
                      D5   保留
                      D6=1 浮点格式 =0 整形    
                      D7=1 主动发送禁止 =0 允许
                    */
};

struct VYCF_L{
  long lValue;
  BYTE byFlag;      /*D0=1 有效 =0 无效
        			  D1=1 被封锁 =0 未被封锁
        			  D2=1 被取代 =0 未被取代
					  D3=1 非当前值 =0 当前值
                      D4=1 溢出 =0 未溢出
                      D5   保留
                      D6=1 浮点格式 =0 整形    
                      D7=1 主动发送禁止 =0 允许
                    */
};
struct VYCF_L_Cfg{
  	BYTE tyctype;
	char tycunit[5];
	BYTE tycfdnum;
};

struct VYCF{
  short nValue;
  BYTE byFlag;      /*D0=1 有效 =0 无效
        			  D1=1 被封锁 =0 未被封锁
        			  D2=1 被取代 =0 未被取代
					  D3=1 非当前值 =0 当前值
                      D4=1 溢出 =0 未溢出
                      D5   保留
                      D6=1 浮点格式 =0 整形    
                      D7=1 主动发送禁止 =0 允许
                    */
};

struct VDBDDF{
  WORD wNo;
  long lValue;
  BYTE byFlag;  /* D0=1 一位小数点
                           D1=1 二位小数点
                           D2=1 三位小数点
                           D3=1 四位小数点                    
                           D4-D6 保留
                           D7=1 时标有效 D7=0 时标无效*/
};

struct VDBDD{
  WORD wNo;
  long lValue;
};

struct VDBDDFT{
  WORD wNo;
  long lValue;  
  BYTE byFlag;  /* D0=1 一位小数点
                           D1=1 二位小数点
                           D2=1 三位小数点
                           D3=1 四位小数点                    
                           D4-D6 保留
                           D7=1 时标有效 D7=0 时标无效*/
  struct VCalClock Time;
};

struct VDDF{
  long lValue;
  BYTE byFlag;    /* D0=1 一位小数点
                             D1=1 二位小数点
                             D2=1 三位小数点
                             D3=1 四位小数点                    
                             D4-D6 保留
                             D7=1 时标有效 D7=0 时标无效*/
};

struct VDDFT{
  long lValue;  
  BYTE byFlag;  /* D0=1 一位小数点
                           D1=1 二位小数点
                           D2=1 三位小数点
                           D3=1 四位小数点                    
                           D4-D6 保留
                           D7=1 时标有效 D7=0 时标无效*/
  struct VCalClock Time;
};


#define YK_INVALID 			0x00
#define YK_CLOSE            0x01
#define YK_OPEN             0x02
#define YK_NULL			    0x03
#define CTR_BYID            0x04
#define USEDEF_PLUSE        0x00
#define SHORT_PLUSE         0x10
#define LONG_PLUSE          0x20
#define STAND_PLUSE         0x30

#define DEF_YK_INVALID_VALUE (USEDEF_PLUSE|CTR_BYID|YK_INVALID)
#define DEF_YK_CLOSE_VALUE   (USEDEF_PLUSE|CTR_BYID|YK_CLOSE)
#define DEF_YK_OPEN_VALUE    (USEDEF_PLUSE|CTR_BYID|YK_OPEN)
#define DEF_YK_NULL_VALUE (USEDEF_PLUSE|CTR_BYID|YK_NULL)

#define CONTROLOK           0x00
#define CONTROLIDINVAILD    0x01
#define CONTROLIDDOING      0x02
#define CONTROLFAILED       0x03
#define CONTROLDISABLE      0x04
#define CONTROLUNKNOWERROR  0x05

struct VDBCtrl{
  WORD wID;       
  BYTE byStatus;  /*表示请求命令产生的结果*/
};

struct VDBYK{
  WORD wID;       /*遥控号*/
  BYTE byStatus;  /*表示请求命令产生的结果
                   0：  正常
                   非0：错误

                   1－点号非法
                   2－该点正在被操作
                   3－控制硬件有问题
                   4－控制禁止
                   5－其他错误 */
  BYTE byValue;   /*遥控属性
                  D0~D1 
                   00 非法 
                   01 合闸继电器
                   10 分闸继电器
                   11 NULL（无需区分合/分闸继电器）
                  D2：1控制对象为开关号（进行开关号查找）（缺省） 0：控制对象为顺序点号（偏移）
                  D3：保留
                  D4-D7:  0－设备自定义脉冲（缺省）
                          1－短脉冲
                          2 长脉冲
                          3 持续输出*/                 
};

struct VDBYT{
  WORD wID; 	  /*遥控号*/
  BYTE byStatus;  /*表示请求命令产生的结果
				   0：	正常
				   非0：错误

				   1－点号非法
				   2－该点正在被操作
				   3－控制硬件有问题
				   4－控制禁止
				   5－其他错误 */
  DWORD dwValue;   /*遥调值*/
};

struct VYKEventInfo{
  struct VCalClock Time;
  DWORD dwEqpName;
  WORD wID;
  BYTE byAttr;
  BYTE byRsv;
}; 

#define STDSWNUM 10

#define SWCOMMERR	0x05	/*设备通讯故障*/
#define SWERROR 0x06	/*开关故障*/

struct VFaultSort
{
	BYTE DAType;	/*1表示集中式控制；2 表示分布式控制； */
	BYTE ReportType;	/*报文类型：1表示馈线故障处理报文，2表示开闭所备自投报文 ，4 表示开闭所故障处理报文 ，8表示备自投恢复报文，5 DA方式转换报文*/
	WORD ReportFun;	/*报文功能有效标志：D1：故障识别，D2：故障隔离 D3：恢复 。1 表示有效，0表示无效。*/
	WORD FailProc;	/*失败所处阶段*/
};

struct VFaultPos
{
	DWORD StartSW; /*故障区域起始开关编号*/
	DWORD EndSW; /*故障区域末端开关编号*/
};

struct VYkResult
{
	DWORD SW;	/*开关号,0表示无效*/
	DWORD GZCSW; /*故障后续开关*/
	BYTE Phases;	/*所处步骤=1 遥控预置=2遥控执行*/
	BYTE Status;	/*遥控结果0:表示成功*/	
};

struct VFAProcInfo
{
	struct VCalClock   Time ;
	struct VFaultSort  Sort ; 
	struct VFaultPos   Position ;
	struct VYkResult   Ioslation[STDSWNUM];
	struct VYkResult   Resume[STDSWNUM];
	BYTE Rsv[8];
}; 

struct VFAProc{
	WORD wWritePtr;
	WORD wPoolNum;
	struct VFAProcInfo *pPoolHead;	
	WORD wCellLen;
	DWORD dwSem;
};

struct VSysFlowEventInfo{
	WORD num;
	struct VDBFLOW dbflow[8]; //暂定，一回线的8个潮流数
};

struct VSysFlowEvent{	
	WORD wWritePtr;
	WORD wPoolNum;  
	struct VSysFlowEventInfo *pPoolHead;	
	WORD wCellLen;
	DWORD dwSem;
	WORD wMmiPtr;        //only for mmi pop
};	



struct VSysEventInfo{
	struct VCalClock time;
	DWORD type;
	WORD rsv;
	WORD para;
	char msg[SYS_LOG_MSGLEN];
};

struct VSysEvent{	
	WORD wWritePtr;
	WORD wPoolNum;  
	struct VSysEventInfo *pPoolHead;	
	WORD wCellLen;
	DWORD dwSem;
	WORD wMmiPtr;        //only for mmi pop
};	

struct VAssertInfo{
	struct VCalClock time;
	DWORD rsv[2];
	char  msg[SYS_LOG_MSGLEN];
};

struct VAssert{
	WORD wWritePtr;
	WORD wPoolNum;
	struct VAssertInfo *pPoolHead;
	WORD wCellLen;
};
#pragma pack()

/*MSGID*/

/*MSGID*/
#define  MI_MASK         0xF0

/*YK*/
#define  MI_YK_MASK      0x10
#define  MI_YKSELECT     0x11
#define  MI_YKOPRATE     0x12
#define  MI_YKCANCEL     0x13
#define  MI_YKSIM        0x14
#define  MI_YKUNSIM      0x15

/*YT*/
#define  MI_YT_MASK      0x20
#define  MI_YTSELECT     0x21
#define  MI_YTOPRATE     0x22
#define  MI_YTCANCEL     0x23

/*TSDATA*/
#define  MI_ROUTE        0x30

/*DA仿真*/
//#define  MI_SIM		     0x40

/*EXTIO*/
#define  MI_EXTIO        0x50

/*PARA*/
#define  MI_PARA       0x60


/*MSGATTR*/
/*YK && YT*/
#define  MA_REQ          0x01
#define  MA_RET          0x02

/*TSDATA*/
#define  MA_TSDATA       0x01

/*DA仿真*/
/*#define  MA_SIM_ENTER    0x01
#define  MA_SIM_SET	     0x02
#define  MA_SIM_EXE	     0x03
#define  MA_SIM_EXIT     0x04
#define  MA_SIM_YK       0x05
#define  MA_SIM_PARA     0x06*/

/*EXTIO*/
#define  MA_EXTIO_CFGCRC      0x01
#define  MA_EXTIO_CFGUPLOAD   0x02
    
#define  MAXDATASIZE   	 2048
#define  MAXMSGSIZE    	 (MAXDATASIZE+sizeof(struct VMsgHead))

struct VMsgHead{
	WORD wLength;
	BYTE byMsgID;    /*D0~D6  MsgID
	      D7 请求报文中＝1表示对方回应报文时无需发送紧急事项
	       ＝0表示对方不回应报文或回应报文时需发送紧急事项*/     
	BYTE byMsgAttr;
	WORD wThID;
	WORD wEqpID;
	BYTE byRsv[4];
};

struct VMsg{
	struct VMsgHead Head;
	BYTE abyData[MAXDATASIZE];
};
struct VFileMsgProM{
	WORD  pLen;
	BYTE  pMsgID;
	BYTE  flag;
	DWORD addr;
    DWORD filelen;
	BYTE msgData[MAXDATASIZE];
};

struct VYKMsg{
	struct VMsgHead Head;
	struct VDBYK Info;
};


#ifdef __cplusplus
}
#endif

#endif
