/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef	_PCOLBASE_H
#define	_PCOLBASE_H

#include "syscfg.h"

#include "sys.h" 
#include "pcol.h"
#include "fileext.h"

#define PRIMARY				0x01
#define SECONDARY			0x02

#define MAX_PUBBUF_LEN		1024
#define MAX_RECV_LEN		1024
#define MAX_SEND_LEN		MAX_PUBBUF_LEN * sizeof(DWORD)

#define MAX_EQP_NUM 		32	

//通讯检测位定义
#define FRAME_SHIELD   0xFFFF0000      //屏蔽字
#define FRAME_OK       0x00010000      //检测到一个完整的帧
#define FRAME_ERR      0x00020000      //检测到一个校验错误的帧
#define FRAME_LESS     0x00030000      //检测到一个不完整的帧（还未收齐）

//标志位定义
#define SYS_FLAG	   0	  //系统保留标志位区
#define SYS_EQPFLAG    0	  //系统保留标志位区

#define USER_FLAG	   32	  //用户标志位区
#define USER_EQPFLAG   32	  //用户标志位区 

//任务标志
#define TASKFLAG_ALLDATA	      (SYS_FLAG+0)  
#define TASKFLAG_CLOCK 	          (SYS_FLAG+1)	
#define TASKFLAG_DD		          (SYS_FLAG+2)	
#define TASKFLAG_DSOEUFLAG        (SYS_FLAG+3)  
#define TASKFLAG_DCOSUFLAG        (SYS_FLAG+4) 
#define TASKFLAG_SSOEUFLAG        (SYS_FLAG+5) 
#define TASKFLAG_SCOSUFLAG        (SYS_FLAG+6)
#define TASKFLAG_FAINFOUFLAG      (SYS_FLAG+7)
#define TASKFLAG_ACTEVENTUFLAG    (SYS_FLAG+8)
#define TASKFLAG_DOEVENTUFLAG     (SYS_FLAG+9)
#define TASKFLAG_WARNEVENTUFLAG   (SYS_FLAG+10)
#define TASKFLAG_ROUTEUFLAG	      (SYS_FLAG+11)
#define TASKFLAG_SENDDSOE	      (SYS_FLAG+12)  
#define TASKFLAG_SENDDCOS	      (SYS_FLAG+13) 
#define TASKFLAG_SENDSSOE	      (SYS_FLAG+14) 
#define TASKFLAG_SENDSCOS	      (SYS_FLAG+15)
#define TASKFLAG_SENDFAINFO       (SYS_FLAG+16)
#define TASKFLAG_SENDACTEVENT     (SYS_FLAG+17)
#define TASKFLAG_SENDDOEVENT      (SYS_FLAG+18)
#define TASKFLAG_SENDWARNEVENT    (SYS_FLAG+19)
#define TASKFLAG_SENDROUTE        (SYS_FLAG+20)


//装置标志
#define EQPFLAG_POLL	   (SYS_EQPFLAG+0)	  
#define EQPFLAG_YC		   (SYS_EQPFLAG+1)	  //遥测
#define EQPFLAG_YX		   (SYS_EQPFLAG+2)   //遥信
#define EQPFLAG_CLOCK	   (SYS_EQPFLAG+3)   //对时
#define EQPFLAG_DD		   (SYS_EQPFLAG+4)   //电度

/*特别注意:对于控制请求和返回标志(如EQPFLAG_YKReq QPFLAG_YKRet)
  因为只有当程序清标志,表示本条消息出来完毕，触发下一条，
  因此清标志不能受到其他特别条件的限制，比如通道链路状态等，
  否则消息会阻塞直到链接状态恢复，这样会有严重问题*/
#define EQPFLAG_YKReq	   (SYS_EQPFLAG+5)	  //有遥控
#define EQPFLAG_WaitYK     (SYS_EQPFLAG+6)	  //等待遥控
#define EQPFLAG_YKRet	   (SYS_EQPFLAG+7)	  //有遥控返校
#define EQPFLAG_YTReq	   (SYS_EQPFLAG+8)	  //有遥调
#define EQPFLAG_WaitYT     (SYS_EQPFLAG+9)	  //等待遥调
#define EQPFLAG_YTRet	   (SYS_EQPFLAG+10)	  //有遥调返校
#define EQPFLAG_TQReq	   (SYS_EQPFLAG+11)	  //有同期
#define EQPFLAG_WaitTQ     (SYS_EQPFLAG+12)	  //等待同期
#define EQPFLAG_TQRet	   (SYS_EQPFLAG+13)	  //有同期返校

#define EQPFLAG_DSOEUFLAG	   (SYS_EQPFLAG+14) 
#define EQPFLAG_DCOSUFLAG	   (SYS_EQPFLAG+15) 
#define EQPFLAG_SSOEUFLAG	   (SYS_EQPFLAG+16) 
#define EQPFLAG_SCOSUFLAG	   (SYS_EQPFLAG+17) 
#define EQPFLAG_EVSOEUFLAG	   (SYS_EQPFLAG+18)
#define EQPFLAG_EVCOSUFLAG	   (SYS_EQPFLAG+19)
#define EQPFLAG_SENDDSOE	   (SYS_EQPFLAG+20) 
#define EQPFLAG_SENDDCOS	   (SYS_EQPFLAG+21) 
#define EQPFLAG_SENDSSOE	   (SYS_EQPFLAG+22) 
#define EQPFLAG_SENDSCOS	   (SYS_EQPFLAG+23) 
#define EQPFLAG_SENDEVSOE	   (SYS_EQPFLAG+24)
#define EQPFLAG_SENDEVCOS	   (SYS_EQPFLAG+25)

#define EQPFLAG_FASIM		   (SYS_EQPFLAG+26)
#define EQPFLAG_ROUTE		   (SYS_EQPFLAG+27)

#define EQPFLAG_VBIT		   (SYS_EQPFLAG+28)
#define EQPFLAG_PARASYN		   (SYS_EQPFLAG+29)

extern WORD m_w101sTaskID;
extern WORD m_w104sTaskID;
extern WORD m_w101mTaskID;
extern WORD m_w104mTaskID;
extern BYTE writeflag;


struct VpcolBuf 
{
    WORD   wBufSize;		//缓冲区大小
    WORD   wReadPtr;		//读指针
    WORD   wOldReadPtr;  	//读指针
    WORD   wWritePtr;		//写指针
    DWORD  dwFlag;			//控制码(主要用于发送)
    BYTE   *pBuf;			//缓冲区
};

struct VYKInfo
{
    VMsgHead Head;
    VDBYK Info;
};

struct VYTInfo
{
    VMsgHead Head;
    VDBYT Info;
};

struct VCtrlInfo{
	WORD  Type;
	WORD  DOT;
	WORD  DCO;
};

struct VPtEqpInfo		
{		
	WORD   wEqpID;		 //装置ID
	WORD   wAddress;	 //装置本身地址
	WORD   wDAddress;	 //对方地址
	char   sExtAddr[12];
	char   sTelNo[20];   /*手机号码*/
	
	DWORD  dwFlag;
	BYTE   byEqpStatus;	                      
    VFLAGS	Flags;       //每一模块控制标志位（目前共有128位）

	WORD wYCNum;
	WORD wVYCNum;
	WORD wDYXNum;
	WORD wSYXNum;
	WORD wVYXNum;        /*虚拟遥信个数*/
	WORD wDDNum;
	WORD wYKNum;
	WORD wTSDataNum;
	WORD wYTNum;
	WORD wTQNum;
	VEqpRunInfo *pEqpRunInfo;	

	VYKInfo YKInfo;
	VYTInfo YTInfo;
	
	void *pProtocolData;
};

#pragma pack(1)
struct VProtTimer
{
	WORD wAllData;      //召唤全数据间隔 (主站方)，单位：分(minute) 
	                    //发送全数据间隔 (从站方) 
	WORD wSetClock;     //对钟间隔（主站方），单位：分(minute) 
	WORD wDD;           //召唤电度间隔(主站方)，单位：分(minute) 
	                    //发送电度间隔(从站方) 
	WORD wScanData2;    //二级数据扫描间隔(从站方)，单位：毫秒(MSecond)
};

#define YKOPENABLED	     0x01
#define CLOCKENABLE	     0x02 
#define ALLDATAPROC      0x04
#define DDPROC           0x08
#define BROADCASTTIME	 0x10

struct VProtocolBaseCfg
{
    char wProtcolName[GEN_NAME_LEN];
	WORD wCmdControl;  /*D0=1:遥控允许
				        D1=1:对钟允许
				        D2=1:定时召唤全数据(主站方)
				                      定时发送全数据(从站方)
				                  D3=1:定时召唤电度(主站方)
				                       定时发送电度(从站方)
				                  D4=1:广播对钟
				                  D5~D15 保留*/
	WORD wMaxALLen;//应用层报文最大长度
	WORD wMaxDLlen;//链路层报文最大长度
	WORD wMaxRetryCount;//链路层报文重发次数
	WORD wMaxErrCount;//最大错误次数，写通讯中断事项
	WORD wCtrlResultReqCount; //控制操作结果询问次数
	WORD wActSendRetryCount;  //主动上报重试次数?
	WORD wYCDeadValue; //
	WORD wBroadcastAddr;
	struct VProtTimer Timer;
	//以VPSysConfig.dwCfg 中D3 来区分新老装置
	//D3=1,代表新装置,规约扩展了
	WORD wIDeadValue;      //电流死区
	WORD wACDeadValue;   //交流电压死区
	WORD wDCDeadValue;   //直流电压死区
	WORD wPQDeadValue;   //功率死区
	WORD wFDeadValue;      //频率死区
	WORD wPWFDeadValue;//功率因数死区
	WORD wUTHDeadValue;//电压谐波死区
	WORD wITHDeadValue;//电流谐波死区
	WORD wUNDeadValue;//不平衡度死区
	WORD wLoadDeadValue;//负载率死区
	DWORD dwRsv[6];	
};
#pragma pack()

/*------------------------------------------------------------------------
 规约基类定义
------------------------------------------------------------------------*/
class CProtocol
{
	public:

		CProtocol();//构造函数

		BOOL Init(WORD wTaskID, WORD wMinEqpNum,void *pCfg,WORD wCfgLen);
		void ProtocolExit(void);

		BYTE m_byProtocolType;
		WORD m_wTaskID;
		VTask *m_pTaskInfo;
		char *m_ProtocolName;

		WORD m_wCommID;
        WORD m_wCommCtrl;
		DWORD m_dwCommCtrlTimer;
		
		WORD m_dwHaveRxdFm;
		DWORD m_dwPubBuf[MAX_PUBBUF_LEN];
		DWORD m_dwCycCount;
		
		BOOL m_bPollMasterSendFlag;
		
        VProtocolBaseCfg *m_pBaseCfg;	
		BOOL ReadProtcolCfg(void *pCfg,WORD wCfgLen);
        virtual void CheckBaseCfg(void);
        virtual void CheckCfg(void);
        virtual void SetBaseCfg(void);
        virtual void SetDefCfg(void);

		WORD m_wEqpNo;
		WORD m_wEqpNum;
		WORD *m_pwEqpID;
		WORD m_wEqpID;
		VPtEqpInfo *m_pEqpInfo;
		VPtEqpInfo *pGetEqpInfo(WORD wEqpNo);
		virtual BOOL InitEqpInfo(WORD wMinEqpNum);

		VpcolBuf m_RecBuf;				
		VpcolBuf m_RecFrame;		  
		VpcolBuf m_SendBuf; 					
		int ReadFromComm(void);
		int WriteToComm(DWORD Flag);		
		virtual void CommStatusProcByRT(BOOL bCommOk);
		virtual void CommStatusProcByRTNoErrCnt(BOOL bCommOk);		
		void NeatenCommBuf(VpcolBuf *pCommIO);
		BOOL SearchFrame(void);
		virtual DWORD SearchOneFrame(BYTE *Buf,WORD Len);
		virtual BOOL DoReceive(void);
		virtual BOOL DoSend(void);


		//标志操作相关
		VFLAGS m_TaskFlags;			//每一任务控制标志位（目前共有128位）
		VFLAGS m_EqpFlagsMap;		//所有设备标志位映射表        
		VFLAGS *pGetEqpFlags(DWORD dwEqpNo);		
		DWORD GetTaskFlag(DWORD FlagNo);
		void  SetTaskFlag(DWORD FlagNo);
		void  ClearTaskFlag(DWORD FlagNo);
		DWORD GetEqpFlagMap(DWORD FlagNo);
		void  SetEqpFlagMap(DWORD FlagNo);
		void  ClearEqpFlagMap(DWORD FlagNo);

		DWORD  GetEqpFlag(DWORD dwEqpNo,DWORD dwFlagNo);
		void   SetEqpFlag(DWORD dwEqpNo,DWORD dwFlagNo);
		void   ClearEqpFlag(DWORD dwEqpNo,DWORD dwFlagNo);
		DWORD  GetEqpFlag(DWORD dwFlagNo);
		void   SetEqpFlag(DWORD dwFlagNo);
		void   ClearEqpFlag(DWORD dwFlagNo);

		void   SetAllEqpFlag(DWORD dwFlagNo);	
		void   ClearAllEqpFlag(DWORD dwFlagNo);	
		BOOL   SwitchToEqpFlag(DWORD dwFlagNo); 
		BOOL   SwitchClearEqpFlag(DWORD dwFlagNo);
		BOOL   CheckClearTaskFlag(DWORD dwFlagNo);
		BOOL   CheckClearEqpFlag(DWORD dwFlagNo);

		//切换相关
		BOOL SwitchToEqpNo(DWORD dwEqpNo);
		virtual BOOL SwitchToAddress(WORD wAddress);		
		BOOL SwitchToEqpID(WORD wEqpID); 
		WORD GetEqpNofromID(WORD wEqpID) ;
		
		WORD SearchHead(BYTE *Buf, WORD Len,short Offset1,BYTE Key1);
		WORD SearchHead(BYTE *Buf, WORD Len,short Offset1,BYTE Key1,short Offset2,BYTE Key2);
		void SendWordLH(WORD wData);
		void SendWordLH(DWORD dwWPtr,WORD wData);
		void SendDWordLH(DWORD dwData);
		void SendDWordLH(DWORD dwWPtr,DWORD dwData);

		virtual WORD GetEqpAddr(void);
		virtual WORD GetOwnAddr(void);
		WORD  GetBroadcastAddr(void);

		WORD m_wRetryCount;  //重发次数统计
		WORD  GetMaxRetryCount(void);
		BOOL  GetRetryFlag(void); //获取重发标志		
		WORD GetRetryCount(void); //获取重发次数
		void ResetRetryCount(void);   //重发次数清0
		void IncRetryCount(void);	  //重发次数加1
		void DisableRetry(void);   //禁止重发
		BOOL SendRetry(void); //重发报文

		VMsg *m_pMsg;
		virtual BOOL DoYKReq(void);		
		virtual BOOL DoYKRet(void);
		virtual BOOL DoYTReq(void);		
		virtual BOOL DoYTRet(void);
		virtual BOOL DoFaSim(void);	
		virtual void DoTSDataMsg(void);
		virtual void DoParaMsg(void);
		DWORD m_dwTimerCount;
		BOOL TimerOn(DWORD TimeOut);		
		WORD GetTimeOutValue(WORD l);

		BOOL DoMessage(void);
		void DoReadCommEvent(void);
		void DoWriteCommEvent(void);
		virtual void DoCommIdle(void);
		virtual void DoCommSendIdle(void);
		virtual void DoUrgency(void);
		virtual void DoTimeOut(void);
		virtual DWORD DoCommState(void);
		virtual void DoDataRefresh(void);
		virtual void DoClearSoe(void);
		
		void Run(void); 	
};




#endif
