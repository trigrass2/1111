/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef GB101S_H
#define GB101S_H

#include "../public/pslave.h"	
#include "gb101pub.h"
#include "../dlq/DLQ.h"
	
#define BALANCE_FLAG	(USER_FLAG + 0)  //平衡模式标志
#define YC_YX_FLAG       0xff
//接收
#define Rec_FCB			(USER_EQPFLAG + 0)  

#define CALL_YXGRP1		(USER_EQPFLAG + 1)	    
#define CALL_YXGRP2		(USER_EQPFLAG + 2)	
#define CALL_YXGRP3		(USER_EQPFLAG + 3)	 
#define CALL_YXGRP4		(USER_EQPFLAG + 4)	 
#define CALL_YXGRP5		(USER_EQPFLAG + 5)	 
#define CALL_YXGRP6		(USER_EQPFLAG + 6)	 
#define CALL_YXGRP7		(USER_EQPFLAG + 7)	
#define CALL_YXGRP8		(USER_EQPFLAG + 8)	 
#define CALL_YCGRP1		(USER_EQPFLAG + 9)	
#define CALL_YCGRP2		(USER_EQPFLAG + 10)	 
#define CALL_YCGRP3		(USER_EQPFLAG + 11)	
#define CALL_YCGRP4		(USER_EQPFLAG + 12)	 
#define CALL_ALLSTOP	(USER_EQPFLAG + 13)	 

#define CALL_DDGRP1		(USER_EQPFLAG + 14)	 
#define CALL_DDGRP2		(USER_EQPFLAG + 15)	 
#define CALL_DDGRP3		(USER_EQPFLAG + 16)	 
#define CALL_DDGRP4		(USER_EQPFLAG + 17)	 
#define CALL_ALLDDSTOP	(USER_EQPFLAG + 18)	 

#define CALL_LINK		(USER_EQPFLAG + 19)	 //召唤链路
#define CALL_DATA 		(USER_EQPFLAG + 20)	 //召唤数据

#define SET_PARA_ACT         	1
#define SET_PARA_DEACT         	1

//SendIdle 消息时应发送的帧类别
#define SEND_LINK		1	
#define SEND_CALLALL	2	
#define SEND_CALLALLDD	3	
#define SEND_BALANCE	4	

#define MODE_BALANCE	1
#define MODE_UNBALANCE	2

#define FILE_NAME_LEN	32

#define	RTU_NULL		0
#define	RTU_RECLINK		1
#define	RTU_RESET		2
#define	RTU_INITEND		3

#define	RTU_RECCALL		4
#define	RTU_SENDDATA		5
#define	RTU_CALLEND		6
#define	RTU_SETTIME		7
#define	RTU_EVENT		8


#define VSQ_SQ  		0x80
#define SWITCHCOS_BIT		0X01
#define SWITCHALLCALL_BIT   0X02

#if (TYPE_USER == USER_GDTEST)
#define	SEND_NULL		0
#define	SEND_ALLACK		1
#define	SEND_ALLDATA		2
#define	SEND_TIME		3
#define	SEND_YK		4
#define	SEND_YKCAN		5
#endif
typedef struct 
{
	DWORD SendIdleSendFrame; 	//SendIdle消息时应发送的帧类别
	WORD CallDataAddr; 			//召唤数据的地址
	
} VIec101Info; //Iec101 规约的每一模块的信息


typedef struct{

	WORD flag; /*结果描述符 D0: 成功，D1未知错误 D2校验错误 D3文件长度不对应 D4文件ID与激活ID不一致*/
	BYTE filenamelen;
	char filename[60];
	DWORD fileid;
	DWORD datano;
	DWORD filesize;

} VFileInfo;

typedef struct{

	BYTE flag;       //召唤标志 0目录下所有文件 1目录下满足搜索时间段的文件
	DWORD dirid;     //目录ID
	BYTE dirnamelen; //目录名长度
	char dirname[100];//目录名
	DWORD dwStrtTime;      /*目录/文件 创建时间 从1970年1月1日0点分0秒开始的时间,以秒表示*/
	DWORD dwStopTime;
	
} VFileDirInfo;

#define MAX101LEN	200

class CGB101S : public CPSecondary
{
	public:
		VIec101Frame *pReceiveFrame; 	//接收帧头指针
		VIec101Frame *pSendFrame;  		//发送帧头指针
		VIec101Info  *Iec101Info; //IEC101规约的每一模块的信息
		BYTE m_byTSData[255];
		WORD m_wTSDataLen;
		BYTE m_bReceiveControl;
		WORD m_wSendYcNum;		//已经发送的遥测个数，控制多帧传送用
		WORD m_wSendYxNum;		//已经发送的遥信个数，控制多帧传送用
		WORD m_wSendDYxNum;		//已经发送的遥信个数，控制多帧传送用
		WORD m_wSendDdNum;		//已经发送的电度个数，控制多帧传送用
		BOOL m_bGZ;
		BOOL m_bZZ;	
		DWORD m_fileoff;
		BYTE DirOffSet[500]; // 保存满足时间段内目录偏移
		WORD DirOffSetNum;
		DWORD startoffset;
		DWORD stopoffset;
		VFileDirInfo FileDirInfo; 
		VDirInfo * pVDirInfo;
		FileStatus DirStatus;
		VFileInfo WriteFileInfo;
		VFileInfo ReadFileInfo;
		BYTE m_calldir;
		BYTE* filetemp;
		BYTE m_bysoftupflag;
		BYTE m_byReadFileAckFlag;
		BYTE m_byReadFileFlag;
		BYTE m_byWriteFileAckFlag;
		BYTE m_byWriteFileDataAckFlag;/*1:成功，2未知错误 3校验和错误 4文件长度不对应 5 文件ID与激活ID不一致*/
		BYTE m_bySwitchValNoFlag;
		BYTE m_byReadValNoFlag;
		BYTE m_byAllParaValFlag;
		BYTE m_byMoreParaValFlag;
		BYTE m_byWriteParaStatus;
		BYTE m_bySoftUpgradeStatus;
		BYTE m_byCallEnergyFlag;
		DWORD m_dwCallDDFlag;
		DWORD m_dwfiffrzflag;
		DWORD m_dwrealdataflag;
		DWORD m_dwdayfrzflag;
		VMsg ParaMsg;
		BYTE ParaFlag;
		BYTE nextflag;
		BYTE nh;
		BYTE readcnt;
		//BYTE writecnt;
		BYTE m_fdno;
		BYTE ReadParaNum;
		DWORD set_infoaddr;
		WORD m_nextparanum;
		DWORD m_fileoffset;
		DWORD m_paraoffset;
		WORD m_paraaddrnum;
		BYTE paracmdstatue;
		struct VParaInfo WriteParaInfo[256];
		WORD WriteParaAddr[256];
		WORD m_WriteNum;
		WORD m_paranum;
		VIec101Cfg *m_pVIec101Cfg;
		VIec101Cfg m_Iec101Cfg;
		BYTE m_byLinkAdrShift;
		BYTE m_byChksumShift;
		BYTE m_byStopShift;
		BYTE m_byTypeIDShift;
		BYTE m_byQualifierShift;
		BYTE m_byReasonShift;
		BYTE m_byCommAdrShift;
		BYTE m_byInfoShift;
		BYTE m_ztype;
		BYTE m_ykdelaystop;
		BYTE m_initoverflag;
		BYTE m_byRTUStatus;//add by lqh 20081103
		BYTE m_qds;
		//sfq
		Vgb101para m_guiyuepara;
		VDWASDU m_dwasdu;
		BYTE m_initflag;
		BYTE m_callallflag;
		BYTE m_groupflag;
		BYTE m_yxchangeflag;
		BYTE m_ycchangeflag;
		BYTE m_SOEflag;
		BYTE m_YKflag;
		BYTE m_YTflag;
		BYTE m_timeflag;
		BYTE m_readtimeflag;
		BYTE m_testflag;
		BYTE m_resetflag;
		BYTE m_PRM;
		BYTE m_readparaflag;
		BYTE m_set_paraflag;
		BYTE m_QPM;
		BYTE m_readparaeqpnum;
		BYTE m_readddflag;
		BYTE m_1classflag;
		BYTE m_acdflag;
		BYTE m_recfalg;
		BYTE m_allcycleflag;
		BYTE m_fcb;
		BYTE m_sourfaaddr;
		BYTE m_zdflag;
		BYTE m_QRP;
		BYTE m_linkdelaytime;
		BYTE m_linkflag;
		BYTE m_retxdnum;
		BYTE m_YK_select_cancel;
		BYTE m_errflag;
		BYTE m_delayflag;
		BYTE m_initallflag;
		BYTE m_allcalloverflag;
		BYTE m_dir_flag;
		BYTE m_retime;
		BYTE Bitchange[256];
		BYTE m_tdata[256];
		WORD m_setycdot;
		WORD m_allsendtime;
		WORD m_delaytime;
		struct VSysClock delayclock;	
		WORD m_wSendparaNum;
		VYKInfo  YKInfo;
		DWORD m_ycpara[256];
		DWORD   m_datatxdflag;/*数据上报标志*/
		DWORD m_dwSendAllDataEqpNo;
		DWORD m_dwcycYcSendFlag;
		DWORD m_dwcycYxSendFlag;
		WORD m_ReadPtr;
		WORD m_WritePtr;
		WORD m_BufLen;
		BYTE   retrnum;
		BYTE   retrflag;
		DWORD *limitpara;
		//sfq
		#if (TYPE_USER == USER_SXXSMD)
		WORD m_wModemReset;
		#endif
		#if (TYPE_USER == USER_MENGJIN)//add by lqh 20060410
		WORD m_wResetCounter;
		#endif
		#if (TYPE_USER == USER_GDTEST)
		BYTE	m_bySendStatus;
		#endif
		VSysClock m_SysTime;

	#if defined  ECC_MODE_CHIP && defined INCLUDE_ECC
		BYTE m_jiamiType;
	#endif
     float fIDeadValue;
     float fACDeadValue;
		 float fDCDeadValue;   //直流电压死区
		 float fPQDeadValue;   //功率死区
		 float fFDeadValue;		//频率死区
		 float fPWFDeadValue;//功率因数死区
		 float fUTHDeadValue;//电压谐波死区
		 float fITHDeadValue;//电流谐波死区
		 float fUNDeadValue;//不平衡度死区
		 float fLoadDeadValue;//负载率死区
	
	public:
		CGB101S();
		BOOL Init(WORD wTaskID);					
		virtual void CheckBaseCfg(void);
		virtual void SetBaseCfg(void);
		virtual void CheckCfg();
		virtual void SetDefCfg();	
		virtual BOOL DoReceive();
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
		BOOL SendtimeReq(void);
				
		BOOL RecFrame10(void);
		BOOL RecResetLink(void);
		BOOL RecReqLink(void);
		BOOL RecCallClass1(void);
		BOOL Recdelay(void);
		BOOL RecCallClass2(void);
		void  DoRecAck(void);//add by lqh 20081103
		void Send2Ack(void);
		void ECES_Test();
		BOOL RecFrame68(void);
		BOOL RecYKCommand(void);
		BOOL RecYKSet(void);
		BOOL RecYKCancel(void);
		BOOL RecCallSomeGroup(void);
		BOOL RecCallAllCommand(void);
		BOOL RecCallAllStart(void);
		BOOL RecCallDDCommand(void);
		BOOL RecCallSomeDD(void);
		BOOL RecCallAllDD(void);
		BOOL RecReadData(void);
		BOOL RecSetClock(void);
		BOOL RecTestLink(void);
		BOOL RecResetRTU(void);
		BOOL RecSetPara(void);
		BOOL SenddelayAck();
		void RecTSData(void);
		void RecFileData(void);
		BOOL RecYTCommand(void);
		BOOL RecYTSet(void);
		BOOL RecYTFSet(void);
		BOOL SendCallAll(void);
		BOOL SendCallAllDD(void);
		BOOL SendBalance(void);
		BOOL SendYCGroup(WORD GroupNo, BYTE Reason);
		BOOL SendYCGroupContinue(WORD GroupNo, BYTE Reason);
		BOOL SendAllStop(void);
		BOOL SendAllDDStop(void);
		BOOL SendAllDDGroup(WORD GroupNo);
		BOOL SendDDGroup(WORD GroupNo,BYTE dwCode,BYTE Reason);
		BOOL SendDDGroupContinue(WORD GroupNo,BYTE dwCode,BYTE Reason);
		BOOL SendYXGroup(WORD GroupNo, BYTE Reason);
		BOOL SendYXGroupContinue(WORD GroupNo, BYTE Reason);
		BOOL SendTsetLinkAck(void);
		BOOL SendSetParaAck(void);
		BOOL SendSoe(void);
		BOOL SendChangeYC(void);
		BOOL SendSomeGroupDD(void);
		BOOL SendCallAllDDAck(void);
		BOOL SendSomeGroupDDData(WORD GroupNo);
		BOOL SendSetClockAck(void);
		BOOL SendCallAllStartAck(void);
		BOOL SendYKSetAck(void);
		BOOL SendYKCancelAck(void);
		BOOL SendYKstop(void) ;
		BOOL SendYTSetAck(void);
		BOOL SendYTstop(void) ;
		void SendInitFinish(void) ;//add by lqh 20081103
		void SendAck(void);//add by lqh 20081103
		
		BYTE GetCtrCode(BYTE PRM, BYTE dwCode,BYTE fcv);
		BOOL SendBaseFrame(BYTE PRM, BYTE dwCode);
		BOOL SendResetLink(BYTE PRM);
		BOOL SendReqLink(void);
		BOOL SendReqLinkAck(void);
		BOOL SearchClass1(void);
		BOOL SendClass1(void);
		BOOL SendClass2(BYTE byMode);
		BOOL SendCos(void);
		BOOL SendFrameHead(BYTE Style, BYTE Reason);
		BOOL SendFrameTail(BYTE PRM, BYTE dwCode, BYTE Num);
		BOOL SendFrameTail0x1(BYTE PRM, BYTE dwCode, BYTE Num);
		BOOL SendFrameTail0x2(BYTE PRM, BYTE dwCode, BYTE Num);
		int SendAllFrame(void);
		BOOL SendReadYCAck(WORD YCNo);
		BOOL SendReadYXAck(WORD YXNo);
		BOOL SendReadDDAck(WORD DDNo);
		BOOL SendNoData(void);
		BOOL initover(void);
		virtual BOOL DoYKRet(void);
		virtual BOOL DoYTRet(void);
		virtual void DoTimeOut(void);
		virtual void DoCommIdle();
		virtual void DoCommSendIdle();
		virtual void DoTSDataMsg(void);
		virtual void DoParaMsg(void);
		void DoReadDir(BYTE *pData);
		void DoReadFileStatus(BYTE *pData);
		void DoReadFileData(BYTE *pData);
		void SendTSData(void);
		void SendFileNAck(BYTE ErrorCode);
		
		WORD GetCosNum(WORD EqpID);
		BYTE ChkSum(BYTE *buf, WORD len);
		DWORD GetAddress(void);
		void initpara(void);
		void getasdu(void);
		void write_linkaddr(int  data);
		void write_typeid(int  data);
		void write_VSQ(int  data);
		void write_COT(int  data);
		void write_conaddr(int  data);
		void write_infoadd(int  data);
		BOOL SendCallgroup(void);
		BOOL SendgroupStop(void);
		BOOL RecACK(void)	;
		void SendNOAck(void);
		void Initlink(void) ;
		BYTE QDS(BYTE data);
		BYTE SIQ(BYTE data);
		BYTE DIQ(BYTE data1,BYTE data2);
		void write_time();
		void write_time3();
		BOOL RecCallAllStop(void);
		BOOL SendDYXGroup(WORD GroupNo, BYTE Reason);
		BOOL searchcos(void) ;
		BOOL searchsoe(void);
		BOOL SendtimeAck(void);
		BOOL SendDCos(void) ;
		BOOL SendDSoe(void) ;
		WORD GetDCosNum(WORD EqpID);
		int Geteqpparaaddr(WORD addr);
		void copydata(WORD len1,WORD len2,BYTE *p1,BYTE* p2);
		void Recsetpara(void);
		void Setpara(void);
		BOOL Sendpara(void);
		void writepara();
		void SendReadDd(WORD wDdNo);
		void SendReadYcpara(WORD wYcNo);
		void SendReadYc(WORD wYcNo);
 		void DoReadData(void);
		void SendReadYx(WORD wYxNo);
		BOOL CycleSendAllYcYx(BYTE reason);
		void write_10linkaddr(int  data);
		BOOL SenderrtypeAck(void) ;
		BOOL Senderrtype1Ack(void) ;
		BOOL Senderrtype3Ack(void); 
		BOOL Senderrtype4Ack(BYTE  COT) ;
		BOOL SendsetparaAck(void);
		BOOL SendresetAck(void);
		BOOL RecSetycpara(void);
		void Setycpara(void);
		BOOL SendYXM_PS_NA(WORD GroupNo, BYTE Reason);

		BOOL SendLinktesetFrame(BYTE PRM,BYTE dwCode);
		
		int GB101ReadRunPara(WORD parano,char* pbuf);
		int GB101WriteRunPara(WORD parano,char* pbuf);
		int GB101WriteRunParaFile();
		
		int ReadDeadValue(WORD parano,char*pbuf);
		int WriteDeadValue(WORD parano,char*pbuf);
		int WriteDeadValueFile();
        BYTE SystimeToSec(struct VSysClock *SysTime, DWORD *Second);
		BYTE SecToSystime(DWORD Second, struct VSysClock *SysTime);
		BOOL SendReadFileAck(void);
		BOOL SendReadFileData(void);
		BOOL SendWriteFileAck(void);
		BOOL SendReadParaVal(void); //发送参数定值
		BOOL DoFileData(void);   //处理文件命令
		BOOL SendFileDir(void);   //发送文件目录
		BOOL DoFileDir(void);
		BOOL DoWriteFile(void);
		BOOL DoReadFile(void);
		BOOL SendWriteParaValAck(void);
		BOOL SendAllEnergy(void);
		BOOL MakeRealddGroupFrame(WORD GroupNo,BYTE Reason);
		BOOL MakeFifFrzGroupFrame(WORD GroupNo,BYTE Reason);
		BOOL MakeDayFrzGroupFrame(WORD GroupNo,BYTE Reason);
		BOOL MakeTideFrzGroupFrame(WORD GroupNo,BYTE Reason);
		BOOL SendChangeFifFrz(void);
		BOOL SendChangeDayFrz(void);
		BOOL SendChangeTide(void);
		BOOL DoSwitchValNo(void);
		BOOL DoReadValNo(void);
		BOOL DoReadParaVal(void);
		BOOL DoWriteParaVal(void);
		BOOL DoSoftUpgrade(void);
		BOOL DoCallEnergy(void);
		BOOL SendReadValNoAck(void);
		BOOL SendSwitchValNoAck(void);
		BOOL SendSoftUpgradeAck(void);
		BOOL SendAllEnergyAck(void);
		BOOL SendAllEnergyOverAck(void);
		BOOL SendFaultEvent(void);
		int ReadPara(WORD addr, char* pbuf);
		int WriteParaYZ(WORD addr, char* pbuf); //参数预置
		int WriteSwitchParaYZ(WORD addr, char* pbuf);
		int WriteParaGH(WORD addr, char* pbuf); //参数固化	
		virtual void DoClearSoe(void);

		void GetRcdNametime(char *str, BYTE *buf);
#ifdef INCLUDE_GPRS
		void SendGetRSSI();
#endif		
};


#endif

