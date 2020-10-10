/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef GB101PUB_H
#define GB101PUB_H
#include "mytypes.h"
#define GB101_V2002
#ifdef GB101_V2002
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
#define	M_SP_TA		2	//����ʱ��ĵ�����Ϣ
#define	M_DP_NA		3 	//����ʱ���˫����Ϣ
#define	M_DP_TA		4	  //����ʱ���˫����Ϣ
#define	M_ST_NA		5 	//��λ����Ϣ
#define	M_BO_NA		7 	//��վԶ���ն�״̬
#define	M_ME_NA		9 	//����ֵ ��һ��ֵ
#define M_ME_TA   10  //��ʱ���һ��ֵ
#define M_ME_NB   11  //����ֵ ��Ȼ�ֵ
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

#define	C_IC_NA		100 	//�ٻ�����
#define	C_CI_NA		101 	//���������ٻ�����
#define	C_RD_NA		102 	//����������
#define	C_CS_NA		103 	//ʱ��ͬ������
#define C_TS_NA     104   	//��������
#define	C_RP_NA		105 	//��λ��������
#define C_TS_TA     107   	//��ʱ��Ĳ�������
#define P_ME_NA     110   	//����
#define P_AC_NA     113   	//��������

#define C_TSDT_NA		129		//͸������ת��
#define C_FILE_OP	130		//�ļ�����
#define M_FA_NA		142		//FA������Ϣ
#define C_FA_SIM	143		//FA ģ��
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
#define	INIT_OVER		70 	//��ʼ������

#define	COT_PN_BIT		0x40

#define		S_INITOK		0
#define		S_REQLINK		1
#define		S_RESETLINK		2
#define		S_LINKOK		3
#define		S_REQRES		4
#define		S_SENDCON		5
#define		S_LINKBUSY		6
#define		S_RECREQ        7
#define		S_RECRESET      8
#define		S_NULL 			254


#define 	PRM_MASTER		1
#define		PRM_SLAVE		0

#define		MFC_RESETLINK		0
#define		MFC_TRANSDATA		3
#define		MFC_REQLINK			9
#define		MFC_REQCLASS1		10
#define		MFC_REQCLASS2		11

#define		SFC_CONFIRM			0
#define		SFC_LINKBUSY		1
#define		SFC_DATACON			8
#define		SFC_LINKOK			11
#define		SFC_LINKNOTWORK		14
#define		SFC_LINKNOTFINISH	15
#define		SFC_RECREQ		    0x49
#define		SFC_RECRESET	    0x40




//֡������
#define 	BITS_DIR	0x80	//���䷽��λ
#define 	BITS_PRM	0x40	//��������λ
#define 	BITS_FCB	0x20	//֡����λ
#define 	BITS_AVI	0x10	//֡������Чλ
#define		BITS_ACD	0x20
#define		BITS_DFC	0x10
#define 	BITS_CODE	0x4F	//��������ռλ
#define 	BITS_FCBV 	0x30	//��������ռλ

#define CONTROL_LEN			2
#define ASDUID_LEN			4

#define SCOS_LEN		3
#define SSOE_LONG_LEN	10
#define SSOE_SHORT_LEN	6
#define CHANGE_YC_LEN	4

#define MAX_FRAME_LEN  		255

typedef struct 
{
	BYTE Start; 
	BYTE Control; 
	BYTE Data[4];//BYTE Address; 
	//BYTE ChkSum; 
	//BYTE Stop; 
}VFrame10; 

typedef struct 
{
	BYTE Start1; 
	BYTE Length1; 
	BYTE Length2; 
	BYTE Start2; 
	BYTE Control; 
	BYTE Data[MAX_FRAME_LEN-5];//BYTE LinkAddress;
	//BYTE Style;  
	//BYTE Definitive; //�ṹ�޶���
	//BYTE Reason; 
	//BYTE CommonAddress;   
	//BYTE Data;  
}VFrame68; 

typedef union 
{
	VFrame10 Frame10;
	VFrame68 Frame68;
} VIec101Frame; 

struct VIec101Cfg
{
	BYTE byLinkAdr;
	BYTE byCommAdr;
	BYTE byInfoAdr;
	BYTE byCOT;
};

#pragma pack(1)
typedef struct{
	BYTE linkaddrlen;/*��·��ַ����*/
	BYTE typeidlen;/*��Ϣ���ͳ���*/
	BYTE conaddrlen;/*Ӧ�ò㹫����ַ����*/
	BYTE VSQlen;/*�ɱ�ṹ�޶��ʳ���*/
	BYTE COTlen;/*����ԭ�򳤶�*/
	BYTE infoaddlen;/*��Ϣ��ַ����*/
	BYTE mode;/*����ģʽ*/
	BYTE yxtype;/*1.����ʱ�ꣻ2.��ʱ�ꣻ3��CP56Time2aʱ��*/
	BYTE yctype;/*9.����ֵ, ��һ��ֵ;10.��ʱ��Ĳ���ֵ, ��һ��ֵ11.����ֵ, ��Ȼ�ֵ12.����ֵ, ��ʱ��ı�Ȼ�ֵ
	              13.����ֵ, �̸�����14.����ֵ, ��ʱ��Ķ̸�����21.����ֵ, ����Ʒ�������ʵĹ�һ��ֵ
	               34.��CP56Time2aʱ��Ĳ���ֵ, ��һ��ֵ35��CP56Time2aʱ��Ĳ���ֵ, ��Ȼ�ֵ
	  36��CP56Time2aʱ��Ĳ���ֵ, �̸����� */
	BYTE ddtype;/*15.�ۼ���16��ʱ����ۼ���37��CP56Time2aʱ����ۼ���*/
	WORD baseyear;/*1900��2000*/
	WORD jiami;/*0����*/
	WORD tmp[9];
}Vgb101para;/*101��Լ�����ṹ*/
#pragma pack ()

typedef struct {
	DWORD LinkAddr;	   
	DWORD TypeID;	   
	DWORD VSQ;
	DWORD COT;	
	DWORD Address;	
	DWORD Info;	   
	BYTE  Infooff;	   
	BYTE  COToff;
} VDWASDU;

//����ֵ����(qpm )
typedef struct{
	WORD limitchange;       //����ֵ
	WORD pinghuashijian; //ƽ��ϵ��
	WORD limitmax;          //���Ͳ�������
	WORD limitmmin;        //���Ͳ�������
}Vlimitpara;

#endif
