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

//ͨѶ���λ����
#define FRAME_SHIELD   0xFFFF0000      //������
#define FRAME_OK       0x00010000      //��⵽һ��������֡
#define FRAME_ERR      0x00020000      //��⵽һ��У������֡
#define FRAME_LESS     0x00030000      //��⵽һ����������֡����δ���룩

//��־λ����
#define SYS_FLAG	   0	  //ϵͳ������־λ��
#define SYS_EQPFLAG    0	  //ϵͳ������־λ��

#define USER_FLAG	   32	  //�û���־λ��
#define USER_EQPFLAG   32	  //�û���־λ�� 

//�����־
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


//װ�ñ�־
#define EQPFLAG_POLL	   (SYS_EQPFLAG+0)	  
#define EQPFLAG_YC		   (SYS_EQPFLAG+1)	  //ң��
#define EQPFLAG_YX		   (SYS_EQPFLAG+2)   //ң��
#define EQPFLAG_CLOCK	   (SYS_EQPFLAG+3)   //��ʱ
#define EQPFLAG_DD		   (SYS_EQPFLAG+4)   //���

/*�ر�ע��:���ڿ�������ͷ��ر�־(��EQPFLAG_YKReq QPFLAG_YKRet)
  ��Ϊֻ�е��������־,��ʾ������Ϣ������ϣ�������һ����
  ������־�����ܵ������ر����������ƣ�����ͨ����·״̬�ȣ�
  ������Ϣ������ֱ������״̬�ָ�������������������*/
#define EQPFLAG_YKReq	   (SYS_EQPFLAG+5)	  //��ң��
#define EQPFLAG_WaitYK     (SYS_EQPFLAG+6)	  //�ȴ�ң��
#define EQPFLAG_YKRet	   (SYS_EQPFLAG+7)	  //��ң�ط�У
#define EQPFLAG_YTReq	   (SYS_EQPFLAG+8)	  //��ң��
#define EQPFLAG_WaitYT     (SYS_EQPFLAG+9)	  //�ȴ�ң��
#define EQPFLAG_YTRet	   (SYS_EQPFLAG+10)	  //��ң����У
#define EQPFLAG_TQReq	   (SYS_EQPFLAG+11)	  //��ͬ��
#define EQPFLAG_WaitTQ     (SYS_EQPFLAG+12)	  //�ȴ�ͬ��
#define EQPFLAG_TQRet	   (SYS_EQPFLAG+13)	  //��ͬ�ڷ�У

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
    WORD   wBufSize;		//��������С
    WORD   wReadPtr;		//��ָ��
    WORD   wOldReadPtr;  	//��ָ��
    WORD   wWritePtr;		//дָ��
    DWORD  dwFlag;			//������(��Ҫ���ڷ���)
    BYTE   *pBuf;			//������
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
	WORD   wEqpID;		 //װ��ID
	WORD   wAddress;	 //װ�ñ����ַ
	WORD   wDAddress;	 //�Է���ַ
	char   sExtAddr[12];
	char   sTelNo[20];   /*�ֻ�����*/
	
	DWORD  dwFlag;
	BYTE   byEqpStatus;	                      
    VFLAGS	Flags;       //ÿһģ����Ʊ�־λ��Ŀǰ����128λ��

	WORD wYCNum;
	WORD wVYCNum;
	WORD wDYXNum;
	WORD wSYXNum;
	WORD wVYXNum;        /*����ң�Ÿ���*/
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
	WORD wAllData;      //�ٻ�ȫ���ݼ�� (��վ��)����λ����(minute) 
	                    //����ȫ���ݼ�� (��վ��) 
	WORD wSetClock;     //���Ӽ������վ��������λ����(minute) 
	WORD wDD;           //�ٻ���ȼ��(��վ��)����λ����(minute) 
	                    //���͵�ȼ��(��վ��) 
	WORD wScanData2;    //��������ɨ����(��վ��)����λ������(MSecond)
};

#define YKOPENABLED	     0x01
#define CLOCKENABLE	     0x02 
#define ALLDATAPROC      0x04
#define DDPROC           0x08
#define BROADCASTTIME	 0x10

struct VProtocolBaseCfg
{
    char wProtcolName[GEN_NAME_LEN];
	WORD wCmdControl;  /*D0=1:ң������
				        D1=1:��������
				        D2=1:��ʱ�ٻ�ȫ����(��վ��)
				                      ��ʱ����ȫ����(��վ��)
				                  D3=1:��ʱ�ٻ����(��վ��)
				                       ��ʱ���͵��(��վ��)
				                  D4=1:�㲥����
				                  D5~D15 ����*/
	WORD wMaxALLen;//Ӧ�ò㱨����󳤶�
	WORD wMaxDLlen;//��·�㱨����󳤶�
	WORD wMaxRetryCount;//��·�㱨���ط�����
	WORD wMaxErrCount;//�����������дͨѶ�ж�����
	WORD wCtrlResultReqCount; //���Ʋ������ѯ�ʴ���
	WORD wActSendRetryCount;  //�����ϱ����Դ���?
	WORD wYCDeadValue; //
	WORD wBroadcastAddr;
	struct VProtTimer Timer;
	//��VPSysConfig.dwCfg ��D3 ����������װ��
	//D3=1,������װ��,��Լ��չ��
	WORD wIDeadValue;      //��������
	WORD wACDeadValue;   //������ѹ����
	WORD wDCDeadValue;   //ֱ����ѹ����
	WORD wPQDeadValue;   //��������
	WORD wFDeadValue;      //Ƶ������
	WORD wPWFDeadValue;//������������
	WORD wUTHDeadValue;//��ѹг������
	WORD wITHDeadValue;//����г������
	WORD wUNDeadValue;//��ƽ�������
	WORD wLoadDeadValue;//����������
	DWORD dwRsv[6];	
};
#pragma pack()

/*------------------------------------------------------------------------
 ��Լ���ඨ��
------------------------------------------------------------------------*/
class CProtocol
{
	public:

		CProtocol();//���캯��

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


		//��־�������
		VFLAGS m_TaskFlags;			//ÿһ������Ʊ�־λ��Ŀǰ����128λ��
		VFLAGS m_EqpFlagsMap;		//�����豸��־λӳ���        
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

		//�л����
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

		WORD m_wRetryCount;  //�ط�����ͳ��
		WORD  GetMaxRetryCount(void);
		BOOL  GetRetryFlag(void); //��ȡ�ط���־		
		WORD GetRetryCount(void); //��ȡ�ط�����
		void ResetRetryCount(void);   //�ط�������0
		void IncRetryCount(void);	  //�ط�������1
		void DisableRetry(void);   //��ֹ�ط�
		BOOL SendRetry(void); //�ط�����

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
