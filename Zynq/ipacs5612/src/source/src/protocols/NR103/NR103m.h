#ifndef _NR103M_H_
#define _NR103M_H_

#include "../public/pmaster.h"

//���Ʒ���ASDU����
#define TYPE_ASDU06H_TIME			0X06	//��ʱ
#define TYPE_ASDU07H_ALLCALL		0X07	//�����ܲ�ѯ
#define TYPE_ASDU08H_CALLOVER		0X08	//�ܲ�ѯ����
#define TYPE_ASDU14H_NORMAL		0x14//һ������
#define TYPE_ASDU15H_GEN_READ	0X15	//ͨ�÷��������
#define TYPE_ASDU23H_RDSTATE		0X17	//����¼���Ŷ���
#define TYPE_ASDU18H_RDDATACALL	0X18	//�Ŷ����ݴ��������
#define TYPE_ASDU19H_RDDATACON	0X19	//�Ŷ����ݴ�����Ͽ� 
#define TYPE_ASDUDCH_GEN_CALL		0XDC	//ͨ�÷��������

//���ӷ���ASDU����
#define TYPE_ASDU05H_FLAG 				0X05	//��ʶ
#define TYPE_ASDU0AH_GEN_GROUPDATA	0X0a	//ͨ�÷���������Ӧ���װ����Ӧ�Ķ�һ�����������
#define TYPE_ASDU0BH_GEN_DATA		0X11	//ͨ�÷���������Ӧ���װ����Ӧ�Ķ�������Ŀ��Ŀ¼��
#define TYPE_ASDU1CH_SOE				0X1C	//����־��״̬��λ����׼������
#define TYPE_ASDU1DH_SOE				0X1D	//���ʹ���־��״̬��λ
#define TYPE_ASDU40H_YK				0X40	//����Ƽ��ϰ汾װ��ֻ����0x40ң��
#define TYPE_ASDU29H_YK				0X29	//ң�ط�ȷ��



//����ԭ��
#define COT_SPONT 	1 	//ͻ��
#define COT_PERCYC	2 	//ѭ��
#define COT_TIMER		8		//ʱ��ͬ��
#define COT_ALLCALL		9		//�ܲ�ѯ
#define COT_COMCMD		20		//һ������
#define COT_RDTRANS		31		//�Ŷ����ݴ���
#define COT_GCWRITE		40		//ͨ�÷���д����
#define COT_GCREAD		42		//ͨ�÷��������

//ͨ�÷������ݣ���������
#define TYPE_NODATA 	0 // ������
#define TYPE_ASCII 		1 // OS8ASCII��ASCII 8 λ�룩
#define TYPE_BSI 		2 // ����8 λ����BS1��
#define TYPE_UI 			3 // �޷���������UI��
#define TYPE_I 			4 // ������I��
#define TYPE_UF 			5 // �޷��Ÿ�������UF��
#define TYPE_F 			6 // ��������F��
#define TYPE_SI754 		7 // IEEE ��׼754 ��ʵ����R32.23��
#define TYPE_I754 		8 // IEEE ��׼754 ʵ����R64.53��
#define TYPE_DYK 		9 // ˫����Ϣ����6.6.5
#define TYPE_SYK 		10 // ������Ϣ
#define TYPE_TRANSIENT_ERROR 	11 	// ��˲��Ͳ���˫����Ϣ	0=˲�䣨TRANSIENT��
									//	1=����OFF��
									//2=�ϣ�ON��
									// 	3=����ERROR��
#define TYPE_YC_F	12 // ��Ʒ�������ı���ֵ�� 6.6.8
#define TYPE_TIME_B 	14 // ������ʱ��� 6.6.29
#define TYPE_GEN_NO 	15 // ͨ�÷����ʶ��ż� 6.6.31
#define TYPE_RET 		16 // ���ʱ��� 6.6.15
#define TYPE_TYPE_INF 	17 	// �������ͺ���Ϣ���CP16{Type,INF}
							//��������(Type) = UI8[0��255]
							//��Ϣ���(INF) = UI8[0��255]
#define TYPE_YX_TIME 	18 	// ��ʱ��ı���
							//CP48{˫����Ϣ��DPI�������ã�RES�����ĸ�8 λλ��ʱ�䣨TIME��������Ϣ��SIN��}
							//����˫����Ϣ��DPI�� = UI2[0��1]<0��3>����6.6.5
							//���ã�RES��= BS6[2��7]<0>
							//�ĸ�8 λλ��ʱ�䣨TIME��= CP32Time2a[8��39] ��6.6.28
							//������Ϣ��SIN�� = UI8[40��47]<0��3>����6.6.23
#define TYPE_YX_RET 	19 	// �����ʱ���ʱ�걨��
							//�����ʱ���ʱ�걨�� = CP80{˫����Ϣ��DPI�������ã�RES����
							//���ʱ�䣨RET����
							//������ţ�FAN��
							//�ĸ� 8 λλ��ʱ�䣨TIME����
							//������Ϣ��SIN��}
							//����˫����Ϣ��DPI�� = UI2[0��1]<0��3>����6.6.5
							//���ã�RES��= BS6[2��7]<0>
							//���ʱ�䣨RET�� = UI16[8��23]����6.6.15
							//������ţ�FAN��= UI16[24��39]����6.6.6
							//�ĸ�8 λλ��ʱ�䣨TIME��= CP32Time2a[8��39] ��6.6.28
							//������Ϣ��SIN�� = UI8[40��47]<0��3>����6.6.23
#define TYPE_YC_RET 	20 	// �����ʱ��ı���ֵ
							//�����ʱ��ı���ֵ= CP96{����ֵ��VAL����
							//���ʱ�䣨RET����
							//������ţ�FAN��
							//�ĸ� 8 λλ��ʱ�䣨TIME�� }
							//���б���ֵ��VAL�� = R32.23[0��31]
							//���ʱ�䣨RET�� = UI16[32��47]����6.6.15
							//������ţ�FAN��= UI16[48��63]����6.6.6
							//�ĸ�8 λλ��ʱ�䣨TIME��= CP32Time2a[64��95] ��6.6.28
#define TYPE_STRUCT    23 //���ݽṹ

#define TYPE_YX_CP56Time2a 203 	//���ֽ�ʱ�걨��
								// CP72{˫����Ϣ��DPI�������ã�RES�����߸�8 λλ��ʱ�䣨TIME����
								//������Ϣ��SIN��}
								//����˫����Ϣ��DPI�� = UI2[0��1]<0��3>����6.6.5
								//���ã�RES��= BS6[2��7]<0>
								//�߸�8 λλ��ʱ�䣨TIME��= CP56Time2a[8��63] ��6.6.29
								//������Ϣ��SIN�� = UI8[64��71]<0��3>����6.6.23
#define TYPE_YX_CP56Time2aRET 204 	// �����ʱ������ֽ�ʱ�걨��
									//�����ʱ���ʱ�걨�� = CP104{˫����Ϣ��DPI�������ã�RES����
									//���ʱ�䣨RET����
									//������ţ�FAN��
									//�߸� 8 λλ��ʱ�䣨TIME����
									//������Ϣ��SIN��}
									//����˫����Ϣ��DPI�� = UI2[0��1]<0��3>����6.6.5
									//���ã�RES��= BS6[2��7]<0>
									//���ʱ�䣨RET�� = UI16[8��23]����6.6.15
									//������ţ�FAN��= UI16[24��39]����6.6.6
									//�߸�8 λλ��ʱ�䣨TIME��= CP56Time2a[40��95]
									//������Ϣ��SIN�� = UI8[96��104]<0��3>����6.6.23
#define TYPE_YC_CP56Time2a 205 		// �����ʱ�����ֽ�ʱ��ı���ֵ
									//�����ʱ��ı���ֵ= CP120{����ֵ��VAL����
									//���ʱ�䣨RET����
									//������ţ�FAN��
									//�߸� 8 λλ��ʱ�䣨TIME�� }
									//���б���ֵ��VAL�� = R32.23[0��31]
									//���ʱ�䣨RET�� = UI16[32��47]����6.6.15
									//������ţ�FAN��= UI16[48��63]����6.6.6
									//�߸�8 λλ��ʱ�䣨TIME��= CP56Time2a[64��119] ��
									
#define TYPE_YK_CP8 	206 			// �������1 byte��
									//��������=CP8 {CCS,DCS,RES,ACT,S/E }
									//��������״̬=CCS:=UI3[1��3] <0��7>
									//<0>: = ������
									//<1>: = ѡ��ͬ�ڹ�����ʽ
									//<2>: = ѡ�����ѹ������ʽ
									//<3>: = ѡ�񲻼칤����ʽ
									//<4>: = ѡ��ϻ�������ʽ
									//<5��7>: = ����
									//˫����״̬=DCS:=UI2[4��5] <0��3>
									//RES:=UI1[6]
									//ACT:=BS1[7] <0��1> <0>: = ������Ч
									//<1>: = ����
									//S/E:=BS1[8] <0��1> <0>: = ִ��
									//<1>: = ѡ��


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
		BYTE NO:6;      	//D0-D5��Ŀ              
		BYTE COUNT:1;      // D6������λ                
		BYTE CONT:1;      //D6����״̬λ              
	} bit;                                   
	BYTE all;   	
}VNGD;

typedef struct _tagPacketHead
{
	WORD 	wFirstFlag; //0xEB90H
	DWORD 	wLength; //���ݳ���
	WORD 	wSecondFlag; //0xEB90H
	WORD 	wSourceFactoryId; //Դ��վ��ַ
	WORD 	wSourceAddr; //Դ�豸��ַ
	WORD 	wDestinationFactoryId; //Ŀ�곧վ��ַ
	WORD 	wDestinationAddr; //Ŀ���豸��ַ
	WORD 	wDataNumber; //���ݱ��
	WORD 	wDeviceType; //�豸����
	WORD 	wDeviceState; //�豸����״̬
	WORD 	wFirstRouterAddr; //����·���׼�·��װ�õ�ַ
	WORD 	wLastRouterAddr; //����·��ĩ��·��װ�õ�ַ
	WORD 	wReserve1; //�����ֽ�0xFFFF
	BYTE 	bTYPE;	//���ͱ�ʶ
	BYTE 	bVSQ;	 //
	BYTE 	bCOT;	//
	BYTE 	bASDU_ADDR;	//������ַ
	BYTE 	bFUN;
	BYTE 	bINF;
}NRFrameHead;

// ���ӷ���
//INF��Ϣ���	����
#define INF_GROPT_M 	240	//240	�����б�������ı���
#define INF_ITEMALL_M	241	//241	��һ�����ȫ����Ŀ��ֵ������
#define INF_ITEMDIR_M	243	//243	��������Ŀ��Ŀ¼
#define INF_ITEMVALUE_M 	244	//244	��������Ŀ��ֵ������
#define INF_CALLSTOP_M	245	//245	ͨ�÷������ݵ����ٻ���ֹ
#define INF_CON_M	249	//249	��ȷ�ϵ�д��Ŀ
#define INF_ACT_M	250	//250	��ִ�е�д��Ŀ
#define INF_STOP_M	251	//251	����ֹ��д��Ŀ
// ���Ʒ���
//INF��Ϣ���	����
#define	INF_TIME_S 0//0			��ʱ
#define	INF_GROPT_S 240//240	�����б�������ı���
#define	INF_ITEMALL_S 241//241	��һ�����ȫ����Ŀ��ֵ������
#define	INF_ITEMDIR_S	243 //243	��������Ŀ��Ŀ¼
#define	INF_ITEMVALUE_S 244//244	��������Ŀ��ֵ������
#define	INF_CALLALL_S		245//245	ͨ�÷������ݵ����ٻ�
#define	INF_WRITE_S	248//248	д��Ŀ
#define	INF_CON_S	249//249	��ȷ�ϵ�д��Ŀ
#define	INF_ACT_S	250//250	��ִ�е�д��Ŀ
//251	д��Ŀ����ֹ

typedef struct _tagApci
{
	BYTE bRII;	//������Ϣ��ʶ
	VNGD bNgd; 
	BYTE bGroup;//���
	BYTE bEntery;//��Ŀ��
	BYTE bKod;	//�������
	BYTE bDataType;
	BYTE bDataSize;
	BYTE bNum;//bit0-6num bit7����״̬λ

}NRAPCI;
typedef struct _tagGIN
{
	BYTE bGroup;//���
	BYTE bEntery;//��Ŀ��
	BYTE bKod;	//�������
	BYTE bDataType;
	BYTE bDataSize;
	BYTE bNum;//bit0-6num bit7����״̬λ
}DataGroup;

#pragma pack()

class CNR103m : public CPPrimary
{
	public:
		NRFrameHead * m_pReceive; 	//����֡ͷָ��
		NRFrameHead * m_pSend;  		//����֡ͷָ��		
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
