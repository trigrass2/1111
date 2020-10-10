#ifndef SAC103P_H
#define SAC103P_H

#include "../public/pmaster.h"
#include "udpclass.h" 
//���ͱ�ʶ

//���ӷ���
#define M_TM_TA			1		//��ʱ��ı���
#define M_TMR_TA		2		//�������ʱ��Ĵ�ʱ��ı���
#define M_MEI_NA		3		//����ֵI
#define M_TME_TA		4		//�������ʱ��Ĵ�ʱ��ı���ֵ
#define M_ID_NA			5		//
#define M_SYN_TA		6		//ʱ��ͬ��
#define M_TGI_NA		8		//�ܲ�ѯ�����ٻ�����ֹ
#define M_MEII_NA		9		//����ֵII
#define M_GD_NA			10		//ͨ�÷�������
#define M_GI_NA			11		//ͨ�÷����ʶ
#define M_LRD_TA		23		//����¼���Ŷ���
#define M_RTC_NA		27		//����¼��ͨ������׼������
#define M_RTT_NA		28		//����־��״̬��λ����׼������
#define M_TDT_TA		29		//���ʹ���־��״̬��λ
#define M_TDN_NA		30		//�����Ŷ�ֵ
#define M_EOT_NA		31		//���ͽ���

//���Ʒ���
#define C_SYN_TA		6		//ʱ��ͬ��
#define C_IGI_NA		7		//�ܲ�ѯ�����ٻ���
#define C_GD_NA			10		//ͨ�÷�������
#define C_GRC_NA		20		//һ������
#define C_GC_NA			21		//ͨ�÷�������
#define C_ODT_NA		24		//�Ŷ����ݴ��������
#define C_ADT_NA		25		//�Ŷ����ݴ�����Ͽ�


//����ԭ��

//���ӷ���
#define COT_SPONT		1		//�Է���ͻ����
#define COT_PERCYC		2		//ѭ��
#define COT_RESETFCB	3		//��λ֡����λ
#define COT_RESETCU		4		//��λͨ�ŵ�Ԫ
#define COT_START		5		//����/��������
#define COT_PWRON		6		//��Դ����
#define COT_TESTMODE	7		//����ģʽ
#define COT_SYNTIME		8		//ʱ��ͬ��
#define COT_CALL		9		//�ܲ�ѯ
#define COT_CALLSTOP	10		//�ܲ�ѯ��ֹ
#define COT_LOCOP		11		//���ز���
#define COT_REMOP		12		//Զ������
#define COT_CMDACK		20		//����Ŀ϶��Ͽ�
#define COT_CMDNACK		21		//����ķ��Ͽ�
#define COT_ADT			31		//�Ŷ����ݵĴ���
#define COT_GCWRACK		40		//ͨ�÷���д����Ŀ϶��Ͽ�
#define COT_GCWRNACK	41		//ͨ�÷���д����ķ��Ͽ�
#define COT_GCRDACK		42		//��ͨ�÷����������Ч������Ӧ
#define COT_GCRDNACK	43		//��ͨ�÷����������Ч������Ӧ
#define COT_GCWRCON		44		//ͨ�÷���дȷ��

//���Ʒ���
#define COT_YK			0X0C
#define COT_COMCMD		20		//һ������
#define COT_GCWRITE		40		//ͨ�÷���д����
#define COT_GCREAD		42		//ͨ�÷��������


//�̶�֡�����Ĺ�����
//���Ʒ���
#define CFUN_RESETCU		0x40
#define CFUN_RESETFCB		0x47
#define CFUN_LINKSTATUS		0x49
#define CFUN_CLASSONE		0x4a
#define CFUN_CLASSTWO		0x4b
#define CFUN_SENDACK		0x43
#define CFUN_SENDNOACK		0x44


//���ӷ���
#define MFUN_ACK			0 



//������Ķ���
//���Ʒ���
#define C_PRM				0x40
#define C_FCB				0x20
#define C_FCV				0x10

//���ӷ���
#define M_ACD				0x20
#define M_DFC				0x10


//��������
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
#define GJEVNT_NUM				90//���䱣���ı����¼�Ҳ���뵽�澯�¼�������

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

