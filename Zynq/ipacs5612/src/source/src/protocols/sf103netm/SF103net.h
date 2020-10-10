#ifndef SF103NET_H
#define SF103NET_H

#include "../public/pmaster.h"

//#define MAXIPACSDEVICENUM  12		//���������豸�������
#define MAXDEVICENUM  12	

#define  MAX_AUTO_RECOVER		20		//20����Ҫ�Զ�����

/*************������Ķ���*************/
//���Ʒ���
#define C_PRM				0x40
#define C_FCB				0x20
#define C_FCV				0x10


//���ӷ���
#define M_ACD				0x20
#define M_DFC				0x10
#define A_CMD				0X0F

#define VSQ_SQ_1			0X80


/*************�����붨��*************/
//����
#define CMD_RESTU_M			0		//��λͨѶ��Ԫ
#define CMD_CONF_M			3		//��������
#define	CMD_DENY_M			4		//�����޻ش�
#define CMD_RESTFCB_M		7		//��λ֡����λ
#define CMD_CALINSTATE_M	9		//�ٻ���·״̬
#define CMD_CALPRIDATA_M	10		//�ٻ�1������
#define CMD_CALSECDATA_M	11		// �ٻ���������

//�ӷ�
#define CMD_CONF_S			0		//ȷ��֡
#define CMD_LINBUSY_S		1		//��·æ
#define CMD_PAKGE_S			8		//�����ݰ���Ӧ����֡
#define CMD_NUDATA_S		9		//��վ����������
#define CMD_LINSTATE_S		11		//��·״̬��Ӧ


/*************TYPE����*************/
#define TYPE_ASDU01_YBSTATE	0X01	//����ѹ�弰�澯�ȿ�����״̬
#define TYPE_ASDU02_PROINF		0X02	//���ͱ���������Ϣ
#define TYPE_ASDU05_SIGNINF	0X05	//��ʶ����
#define TYPE_ASDU06_TIME		0X06	//��ʱ
#define TYPE_ASDU07_ALLCALL	0X07	//�����ܲ�ѯ
#define TYPE_ASDU08_CALLOVER	0X08	//�ܲ�ѯ����

#define TYPE_ASDU23_RDSTATE		0X17	//����¼���Ŷ���
#define TYPE_ASDU24_RDDATACALL	0X18	//�Ŷ����ݴ��������
#define TYPE_ASDU25_RDDATACON		0X19	//�Ŷ����ݴ�����Ͽ� 
#define TYPE_ASDU26_RDDATAACT		0X1A	//�Ŷ����ݴ���׼������
#define TYPE_ASDU27_RDCOMMACT		0X1B	//����¼��ͨ������׼������
#define TYPE_ASDU28_SIGNSTATE		0X1C	//����־��״̬��λ����׼������
#define TYPE_ASDU29_TRANSSTAT		0X1D	//����־��״̬��λ����
#define TYPE_ASDU30_TRSRDDATA		0X1E	//�����Ŷ�ֵ
#define TYPE_ASDU31_RDDATAOVER	0X1F	//�Ŷ����ݴ������

#define TYPE_ASDU36_SENDDNL		0X24	//��������������
#define TYPE_ASDU38_STEPINF		0X26	//�ܲ�ѯ����λ���Ͳ�λ��
#define TYPE_ASDU39_STEPINFSOE	0X27	//��λ�õ�SOE
#define TYPE_ASDU40_YX				0X28	//���ͱ�λң��
#define TYPE_ASDU41_SOE			0X29	//����SOE
#define TYPE_ASDU44_YX				0X2C	//����ȫң��
#define TYPE_ASDU50_YC				0X32	//ң������
#define TYPE_ASDU64_YK				0X40	//ң��ѡ��/ִ��/����
#define TYPE_ASDU88_CALLENERGY	0X58	//�����������ٻ������ᣩ
#define TYPE_ASDU42_YX				0X2A	//����ȫң��
//ͨ�÷���
#define TYPE_ASDU10_GEN_GROUPDATA	0X0a	//ͨ�÷���������Ӧ���װ����Ӧ�Ķ�һ�����������
#define TYPE_ASDU11_GEN_DATA		0X11	//ͨ�÷���������Ӧ���װ����Ӧ�Ķ�������Ŀ��Ŀ¼��
#define TYPE_ASDU21_GEN_READ		0X15	//ͨ�÷��������
#define TYPE_ASDU51_YC				0X33	//ң������



/*************����ԭ��COT*************/
//��
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

//��
#define COT_TIMER		8		//ʱ��ͬ��
#define COT_ALLCALL		9		//�ܲ�ѯ
#define COT_COMCMD		20		//һ������
#define COT_RDTRANS		31		//�Ŷ����ݴ���
#define COT_GCWRITE		40		//ͨ�÷���д����
#define COT_GCREAD		42		//ͨ�÷��������

#define DCO_SE_EXE			0
#define DCO_SE_SELECT		0x80
#define DCO_DCS_OPEN		0x1
#define DCO_DCS_CLOSE		0x2

#define   FEN   0x00
#define   HE	0x01

#define    K	6
#define    W 	8


#define	COT_ACT		    6 	//����
#define	COT_ACTCON		7 	//����ȷ��
#define	COT_DEACT		8 	//ֹͣ����
#define	COT_DEACTCON	9 	//ֹͣ����ȷ��
#define	COT_ACTTERM		10 	//�������

#define YKENABLE                        0x08

#define SWS_ON				0x81
#define SWS_OFF				0x01

#define			ACD_ON		(USER_EQPFLAG + 0)

#define			TY_ON		(USER_EQPFLAG + 1)

#define       DATA_ONE			1

#define       DATA_TWO			2
	
#define       HEAD_V			0x68

#define       TAIL_H			0x16

#define 		MAX_LEN				0xff

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

#define APCI_LEN    9

#define Yk_value	0xee
#define  YK_SE		0x00
#define  YK_ACT		0x01
#define  YK_CA		0x02

typedef enum
{
	YK_SELECT = 0,	//ykѡ��
	YK_ACTION ,
	YK_REVOCAT 
}YKStep;


typedef enum
{
	LINK_DISCONNECT = 0,	//��·�Ͽ�
	LINK_CONNECT ,			//��·����
	LINK_IDLE,				//��·����
	LINK_BUSY,				//��·æ
	LINK_ERROR				//��·��������
}LinkSate;


typedef enum
{
	TRANSIDLE = 0,  //�������
	TRANSSEND,		//��Ҫ����
	TRANSWAIT ,		//�ȴ�Ӧ��
	TRANSRECV		//�ȵ�Ӧ��
	
}DeviceTransStep;

typedef enum
{
	SENDSECDATA = 0,//�����������
	SENDPRIDATA ,	//����һ������
	SENDRESETFCB,	//��λ֡����λ
	SENDRESETCU,	//��λͨѶ��Ԫ
	SENDCALLALL ,	//�ܲ�ѯ
	SENDCALLENERGY,	//�ٻ�������
	SENDYKSELECT,
	SENDYKACT,
	SENDYKREVOCAT,
	ALLFRAMETYPE	//������
}FrameType;

typedef struct
{
	BYTE addr;			//װ�õ�ַ
	BYTE bfcb;			//֡����λ
	BYTE bfcv;			//֡������Чλ
	BYTE linkstate;		//��·״̬
	
	WORD timercount[ALLFRAMETYPE];	//�����ۻ�ʱ��	
	BOOL bReSendflg[ALLFRAMETYPE];	//�ط���ʶ
	BYTE bReSendTimes[ALLFRAMETYPE];//�ط�����
	BYTE transstep[ALLFRAMETYPE]; 	//���̿���
}DeviceInfo;

typedef struct 
{	
	BYTE type;      //���ͱ�ʶ
	BYTE vsq;       //�ɱ�ṹ�޶���
	BYTE cot;       //����ԭ��
	BYTE pubaddr;   //��ַ2
	BYTE fun;
	BYTE info;
	BYTE data[256-12]; //���ݿ�ʼ
}pn103Frame68;



typedef struct 
{
	BYTE byStartCode;
	BYTE byAPDULen_L;
	BYTE byAPDULen;
	BYTE addr_L;
	BYTE addr_H;
	BYTE byControl1;
	BYTE byControl2;
	BYTE byControl3;
	BYTE byControl4;
	BYTE byASDU[255-6];	
}VIecsf103Frame;

typedef struct 
{
	BYTE head;       //�����ַ�
	BYTE control;    //������
	BYTE address;    //��ַ
	BYTE sum;        //У����
	BYTE stop;       //������
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
typedef struct 
{
	struct VCalClock  VClock;
	BYTE No;	//�¼����
	BYTE Dev_no;//װ�ú�
	DWORD tick;
}AUTODELAY_RES;//�Զ���ʱ�ָ�
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

class CSF103NETm : public CPPrimary
{
	public:
		VIecsf103Frame * m_pReceive; 		//����֡ͷָ��
		VIecsf103Frame * m_pSend;  			//����֡ͷָ��
		
		//DeviceInfo	SDeviceInfo[MAXIPACSDEVICENUM];
		DeviceInfo *SDeviceInfo;
		AUTODELAY_RES	*Auto_Recover;	//�Զ��ָ�
		DWORD 	event_time ;
		BYTE m_dwTestTimerCount;
		struct V103Para *pCfg;
		BYTE Ycgroup_num;
		BYTE Yxgroup_num;

		WORD m_wSendNum;	//Send Counter	
		WORD m_wRecvNum;	//Receive Counter
		WORD m_wAckNum;		//Receive Ack Counter
		WORD m_wAckRecvNum;    //Have Ack Receive Counter
		BYTE ip_l;
		BYTE ip_h;
		BOOL flaa;
	public:
		CSF103NETm();
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


		void Settime_clock(void);
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
		BOOL RecFrame68_ASDU10_YC();
		BOOL RecFrame68_ASDU10_YX();
		BYTE time_trans(struct VSysClock *tt,BYTE *a);
		BOOL DoReceviceYc(BYTE bAddr,BYTE bInf,WORD bData);		
		void WriteData(void);		
		BYTE MakeControlCode(BYTE bCmd,BYTE FramList);		
		BYTE MakeFrame10(BYTE bCmd,BYTE FramList);		
		BYTE MakeFrame68Head(BYTE type, BYTE cot, BYTE fun, BYTE info,BYTE flag,BYTE zhi);		
		//BYTE MakeFrame68Tail_yk(BYTE bDevNum,BYTE bCmd,BYTE vsq,BYTE FramList,BYTE a);
		BYTE ChkSum(BYTE *buf, WORD len);
		void SendFrame10_Cmd0();		
		void SendFrame10_Cmd07();		
		void SendFrame10_Cmd10();		
		void SendFrame10_Cmd11();		
		void SendFrame68_ASDU06(BYTE addr);		
		void SendFrame68_ASDU07(BYTE c);		
		void SendFrame68_ASDU88();		
		void SendFrame68_ASDU64(int ykId,BYTE type,BYTE data);	
		void SendFrame68_ASDU21(BYTE flag);
		BOOL Chose_Dev(WORD no,BYTE bFun,BYTE bInf,BYTE *Gn,BYTE Flag);
		BYTE Get_Value(BYTE *buf,BYTE dateType,BYTE datelen,DWORD *val1,float *val2);
		void ykresult(BYTE cot,BYTE type);
		void YkResult(BOOL Result);
		int judge_yc_dd(BYTE fun,BYTE inf);
		int GetNum_Yx();
		BYTE Get_ykdata(BYTE tt,BYTE dd);
		int Chose_ID(int YkNo,BYTE *fun,BYTE *inf);
		void SendUFrame(BYTE byControl);
		void SendSFrame(void);
		void SendFrameHead(BYTE byFrameType);
		void SendFrameTail();
		void Write_Auto_Recov(WORD dev_no,WORD no, struct VSysClock *time);
		AUTODELAY_RES *Choese_Station();
		BOOL Find_Gz(BYTE grop,BYTE entry);
};

#endif


