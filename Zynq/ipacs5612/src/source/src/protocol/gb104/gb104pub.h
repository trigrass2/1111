/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef GB104PUB_H
#define GB104PUB_H
#include "syscfg.h"

#define GB104_V2002
#ifdef GB104_V2002
#define ADDR_YX_LO		0x0001
#define ADDR_YX_HI		0x4000

#define ADDR_YC_LO		0x4001
#define ADDR_YC_HI		0x5000
#define ADDR_YCPARA_LO		0x5001
#define ADDR_YCPARA_HI		0x6000

#define ADDR_YK_LO		0x6001
#define ADDR_YK_HI		0x6200

#define ADDR_YT_LO		0x6201
#define ADDR_YT_HI		0x6400

#define ADDR_DD_LO		0x6401
#define ADDR_DD_HI		0x6600

#define	ADDR_RCD_YC		0x680A  //2016 ����¼����ַ

#else
#define ADDR_YX_LO		0x001
#define ADDR_YX_HI		0x400

#define ADDR_YC_LO		0x701
#define ADDR_YC_HI		0x900

#define ADDR_YK_LO		0xb01
#define ADDR_YK_HI		0xb80

#define ADDR_YT_LO		0xb81
#define ADDR_YT_HI		0xc00

#define ADDR_DD_LO		0xc01
#define ADDR_DD_HI		0xc80
#endif

#define ADDR_BH_LO		0x401
#define ADDR_BH_HI		0x700


#define ADDR_PARA_LO	0x901
#define ADDR_PARA_HI	0xb00





#define ADDR_STEP_LO	0xc81
#define ADDR_STEP_HI	0xca0

#define ADDR_BIN_LO		0xca1
#define ADDR_BIN_HI		0xcb0

#define ADDR_BCD_LO		0xcb1
#define ADDR_BCD_HI		0xcc0

#define ADDR_RTU_STATE	0xcc1

#define ADDR_FILE_LO	0xcc2
#define ADDR_FILE_HI	0xe00



#define	M_SP_NA		1 	//����ʱ��ĵ�����Ϣ
#define	M_DP_NA		3 	//����ʱ���˫����Ϣ
#define	M_ST_NA		5 	//��λ����Ϣ
#define	M_BO_NA		7 	//��վԶ���ն�״̬
#define	M_ME_NA		9 	//����ֵ
#define M_ME_TA   10  //��ʱ���һ��ֵ
#define	M_ME_NB		11	//��Ȼ�����ֵadd by lqh 20080329
#define M_ME_TB   12  //��ʱ���Ȼ�ֵ
#define M_ME_NC   13  //����ֵ �̸�����
#define M_ME_TC   14  //��ʱ��̸�����
#define	M_IT_NA		15 	//�������������
#define	M_PS_NA		20 	//����״̬��λ����ĳ��鵥����Ϣ
#define	M_ME_ND		21 	//����Ʒ�������Ĳ���ֵ

#define M_SP_TB   30  //����ʱ��ĵ�����Ϣ
#define M_DP_TB   31  //����ʱ���˫����Ϣ
#define M_ST_TB   32  //����ʱ��Ĳ�λ����Ϣ
#define M_BO_TB   33  //����ʱ���32λλ��
#define M_ME_TD   34  //����ʱ��Ĳ���ֵ����һ��ֵ
#define M_ME_TE   35  //����ʱ��Ĳ���ֵ����Ȼ�ֵ
#define M_ME_TF   36  //����ʱ��Ĳ���ֵ���̸�����

#define	M_EI_NA		70 	//��ʼ������

#define	C_SC_NA		45 	//����ң������
#define	C_DC_NA		46 	//˫��ң������
#define	C_RC_NA		47 	//��������
#define	C_SE_NA		48 	//�趨����
#define	C_SE_NB_1		49 	//�趨����
#define	C_SE_NC_1		50 	//�趨����
#define	C_SE_TA_1		61 	//����һ��
#define	C_SE_TB_1		62 	//����Ȼ�
#define	C_SE_TC_1		63 	//���̸���

#define	C_IC_NA		100 	//�ٻ�����
#define	C_CI_NA		101 	//���������ٻ�����
#define	C_RD_NA		102 	//����������
#define	C_CS_NA		103 	//ʱ��ͬ������
#define C_TS_NA     104   	//��������
#define	C_RP_NA		105 	//��λ��������
#define	C_CD_NA		106 	//��λ��������
#define C_TS_TA     107   	//��ʱ��Ĳ�������
#define P_ME_NA     110   	//����
#define P_AC_NA     113   	//��������

/*104 - 2016 ��������*/
#define M_FT_NA_1	42		//�����¼�
#define C_SR_NA_1	200     //�л���ֵ��
#define C_RR_NA_1	201	    //����ֵ����
#define C_RS_NA_1	202     //�������Ͷ�ֵ
#define C_WS_NA_1	203     //д�����Ͷ�ֵ
#define F_FR_NA_1	210	    //�ļ�����
#define F_SR_NA_1	211     //�������
/*******END*********/

/*2016 ���� ¼���ļ��������ͱ�־*/
//#define F_FR_NA_1	120		//�ļ�׼������
//#define F_SR_NA_1	121		//��׼������
#define F_SC_NA_1	122		//�ٻ�Ŀ¼.ѡ���ļ�.�ٻ��ļ�.�ٻ���
#define F_LS_NA_1	123		//���Ľڣ����Ķ�
#define F_AF_NA_1	124		//�Ͽ��ļ�.�Ͽɽ�
#define F_SG_NA_1	125		//��
#define F_DR_NA_1	126		//Ŀ¼



#define M_FA_NA		142		//FA������Ϣ
#define C_FA_SIM	143		//FA ģ��
#define C_YK_EXT	150		//Ⱥ��
#define S_PROT_VAL	180		//������ֵ
#define S_PROT_EVENT 181
#define S_File_List      182
#define S_File_Trans   183
#define M_IT_NB_1   206   //�������ۻ������̸�����
#define M_IT_NC_1   207  //��ʱ����ۻ������̸�����

//����ԭ��
#define	COT_PERCYC	  	1 	//����/ѭ��
#define	COT_BACK		2 	//����ɨ��
#define	COT_SPONT		3 	//ͻ��
#define	COT_INIT	  	4 	//��ʼ��
#define	COT_REQ		    5 	//���������
#define	COT_ACT		    6 	//����
#define	COT_ACTCON		7 	//����ȷ��
#define	COT_DEACT		8 	//ֹͣ����
#define	COT_DEACTCON	9 	//ֹͣ����ȷ��
#define	COT_ACTTERM		10 	//�������
#define	COT_RETREM		11 	//Զ����������ķ�����Ϣ
#define	COT_RETLOC		12 	//������������ķ�����Ϣ
#define	COT_FILE		13 	//�ļ�����

#define	COT_INTROGEN	20 	//��Ӧ���ٻ�
#define	COT_INRO1		21 	//��Ӧ��1���ٻ�
#define	COT_INRO2		22 	//��Ӧ��2���ٻ�
#define	COT_INRO3		23 	//��Ӧ��3���ٻ�
#define	COT_INRO4		24 	//��Ӧ��4���ٻ�
#define	COT_INRO5		25 	//��Ӧ��5���ٻ�
#define	COT_INRO6		26 	//��Ӧ��6���ٻ�
#define	COT_INRO7		27 	//��Ӧ��7���ٻ�
#define	COT_INRO8		28 	//��Ӧ��8���ٻ�
#define	COT_INRO9		29 	//��Ӧ��9���ٻ�
#define	COT_INRO10		30  //��Ӧ��10���ٻ�
#define	COT_INRO11		31 	//��Ӧ��11���ٻ�
#define	COT_INRO12		32 	//��Ӧ��12���ٻ�
#define	COT_INRO13		33 	//��Ӧ��13���ٻ�
#define	COT_INRO14		34 	//��Ӧ��14���ٻ�
#define	COT_INRO15		35 	//��Ӧ��15���ٻ�
#define	COT_INRO16		36  //��Ӧ��16���ٻ�
#define	COT_REQCOGCN	37  //��Ӧ���������ٻ�
#define	COT_REQCO1		38 	//��Ӧ��1��������ٻ�
#define	COT_REQCO2		39 	//��Ӧ��2��������ٻ�
#define	COT_REQCO3		40 	//��Ӧ��3��������ٻ�
#define	COT_REQCO4		41 	//��Ӧ��4��������ٻ�

#define	COT_PN_BIT		0x40

#ifdef INCLUDE_DF104FORMAT
	#define PARA_K		8
	#define PARA_W		4
#else
	#define PARA_K		12
	#define PARA_W		8
#endif
#define PARA_T0		30
#define PARA_T1		15
#define PARA_T2		10
#define PARA_T3		10

#define FRM_I		0
#define FRM_S		1
#define FRM_U		3

#define  STARTDT_ACT 0x7
#define  STARTDT_CON 0x0B
#define  STOPDT_ACT  0x13
#define  STOPDT_CON  0x23
#define  TESTFR_ACT  0x43
#define  TESTFR_CON  0x83
#define  REMOTE_MAINT  0xC3
#define MAX_FRAME_COUNTER   0x7fff

#define START_CODE          0x68

#define MIN_RECEIVE_LEN  	6    //��С����֡��
#define MAX_FRAME_LEN  		255
  
#define APCI_LEN            6
#define CONTROL_LEN			4
#ifdef INCLUDE_DF104FORMAT
	#define ASDUID_LEN		5
#else
	#define ASDUID_LEN		6
#endif
#define ASDUINFO_LEN		MAX_FRAME_LEN - APCI_LEN - ASDUID_LEN
#define VSQ_NUM 		0x7f
#define VSQ_SQ  		0x80

#define YC_GRP_NUM		40
				#if(TYPE_USER == USER_GUANGXI)
#define YC_GRP_NUM		20
#endif
#define YX_GRP_NUM		128
#define DD_GRP_NUM		32	//dd num per group

#ifdef INCLUDE_DF104FORMAT
	#define SCOS_LEN		3
	#define SSOE_LONG_LEN	10
	#define CHANGE_YC_LEN	4
	#define	INFO_ADR_LEN	2
#else
	#define SCOS_LEN		4
	#define SSOE_LONG_LEN	11
	#define CHANGE_YC_LEN	5
	#define	INFO_ADR_LEN	3
#endif
		


#define ETHERNET_COMM_ON	1
#define ETHERNET_COMM_OFF	0
#define FILE_NAME_LEN	32

struct VIec104Timer
{
	BOOL bRun;
	DWORD wCounter;
	WORD wInitVal; 	
};

struct VIec104Frame
{
	BYTE byStartCode;
	BYTE byAPDULen;
	BYTE byControl1;
	BYTE byControl2;
	BYTE byControl3;
	BYTE byControl4;
	BYTE byASDU[MAX_FRAME_LEN-6];	
};

#ifdef INCLUDE_DF104FORMAT
	struct VASDU
	{
		BYTE byTypeID;	   
		BYTE byQualifier;
		BYTE byReasonLow;	
		BYTE byAddressLow;	
		BYTE byAddressHigh; 
		BYTE byInfo[MAX_FRAME_LEN - 12];	   
	};
#else
	struct VASDU
	{
		BYTE byTypeID;	   
		BYTE byQualifier;
		BYTE byReasonLow;	
		BYTE byReasonHigh;	
		BYTE byAddressLow;	
		BYTE byAddressHigh; 
		BYTE byInfo[MAX_FRAME_LEN - 12];	   
	};
#endif

struct VBackFrame
{
	WORD wSendNum;
	WORD wFrameLen;
	BYTE byBuf[MAX_FRAME_LEN];	
};

typedef 	struct 
	{
		DWORD TypeID;	   
		DWORD VSQ;
		DWORD COT;	
		DWORD Address;	
		DWORD Info;	   
		BYTE  Infooff;
		BYTE  COToff;
	} VDWASDU;

typedef struct{
BYTE typeidlen;/*��Ϣ���ͳ���*/
BYTE conaddrlen;/*Ӧ�ò㹫����ַ����*/
BYTE VSQlen;/*�ɱ�ṹ�޶��ʳ���*/
BYTE COTlen;/*����ԭ�򳤶�*/
BYTE infoaddlen;/*��Ϣ��ַ����*/
BYTE yxtype;/*1.����ʱ�ꣻ2.��ʱ�ꣻ3��CP56Time2aʱ��*/
BYTE yctype;/*9.����ֵ, ��һ��ֵ;10.��ʱ��Ĳ���ֵ, ��һ��ֵ11.����ֵ, ��Ȼ�ֵ12.����ֵ, ��ʱ��ı�Ȼ�ֵ
              13.����ֵ, �̸�����14.����ֵ, ��ʱ��Ķ̸�����21.����ֵ, ����Ʒ�������ʵĹ�һ��ֵ
               34.��CP56Time2aʱ��Ĳ���ֵ, ��һ��ֵ35��CP56Time2aʱ��Ĳ���ֵ, ��Ȼ�ֵ
		36��CP56Time2aʱ��Ĳ���ֵ, �̸�����	*/
BYTE ddtype;/*15.�ۼ���16��ʱ����ۼ���37��CP56Time2aʱ����ۼ���*/
WORD k;/*����δȷ�ϵ������������ͷ�ֹͣ����*/
WORD w;/*���ܵ�w��I֡����Ҫ����ȷ��*/
WORD t0;
WORD t1;
WORD t2;
WORD t3;
WORD t4;
WORD baseyear;/*1900��2000*/
	WORD jiami;/*0����*/
	WORD tmp[9];//tmp[6]��������ʱ��fdno�����Ϊ0���ʾ��Ч
				//tmp[7]������0x8243��ѹ��ֵ��0-�ֵ������1-��ֵ
}Vgb104para;/*104��Լ�����ṹ*/
typedef struct{
	WORD limitchange;
	WORD pinghuashijian;
	WORD limitmax;
	WORD limitmmin;
}Vlimitpara;
typedef struct{
	WORD wSendSSOENum;
	WORD wSendDSOENum;
}VSendsoenum;
typedef struct{
	BYTE line;
	BYTE flag;
	BYTE off; 
}Vprotval;

typedef struct{

	BYTE flag;
	DWORD dirid;
	BYTE dirnamelen;
	char dirname[100];
	DWORD dwStrtTime;      /*Ŀ¼/�ļ� ����ʱ�� ��1970��1��1��0���0�뿪ʼ��ʱ��,�����ʾ*/
	DWORD dwStopTime;
	
} VFileDirInfo;

typedef struct{
	
	WORD flag; /*��������� D0: �ɹ���D1δ֪���� D2У����� D3�ļ����Ȳ���Ӧ D4�ļ�ID�뼤��ID��һ��*/
	BYTE filenamelen;
	char filename[100];
	DWORD fileid;
	DWORD datano;
	DWORD filesize;

} VFileInfo;

typedef struct{

	WORD filename;
	BYTE filelen[3];
	BYTE fileSOF;
	BYTE filetime[7];

}VFileDir;

typedef struct{
	WORD SoeNo;
	BYTE SoeValue;
	struct VCalClock Time;
	WORD DelayTime;
	BYTE flag;
}VDelaySoe;

typedef struct{
	WORD CosNo;
	BYTE CosValue;
	WORD DelayTime;
	BYTE flag;
}VDelayCos;


#endif
