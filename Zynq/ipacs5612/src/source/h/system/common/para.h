/*------------------------------------------------------------------------
 Module:       	para.h
 Author:        solar
 Project:      
 Creation Date:	2008-08-05
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
------------------------------------------------------------------------*/

#ifndef _PARA_H
#define _PARA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syscfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file.h"

#define CFG_IND_FILENAME      "ipacs.cfg"
#define CFG_PKG_FILENAME      "ipacs%02d.cfg"

#define CFG_FILE_VER          100   /*1.00*/

#define GEN_NAME_LEN          40
#define GEN_NAMETAB_LEN       15    /*��д*/
#define GEN_NAMELONG_LEN      128

#define PBYXNUM     	 12
#define NEXTPARALEN      4


#define PARACHAR_MAXLEN  64
#define Old_MAXLEN   	 30


#define FLOAT_TYPE      38
#define FLOAT_LEN       4

#define USHORT_TYPE     45
#define USHORT_LEN       2

#define BOOL_TYPE        1
#define BOOL_LEN         1

#define TINY_TYPE       43
#define TINY_LEN        1

#define SHORT_TYPE      33
#define SHORT_LEN       2


#define DOUBLE_TYPE     39
#define DOUBLE_LEN      8

#define INT_TYPE        2
#define INT_LEN         4

#define STRING_TYPE     4
#define OCTSTR_TYPE     4

#define UINT_TYPE   	35
#define UINT_LEN    	4

#define LONG_TYPE   	36
#define LONG_LEN    	8

#define ULONG_TYPE   	37
#define ULONG_LEN    	8

#define UTINY_TYPE		32
#define UTINY_LEN		1

#define SYS_DY_YXNO  (g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low + 18)
#define SYS_GY_YXNO  (g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low + 19)

#define SELFPARA_ADDR 0x8001

#define SELFPARA_TYPE  0
#define SELFPARA_OS      1
#define SELFPARA_MFR    2
#define SELFPARA_HV      3
#define SELFPARA_SV      4
#define SELFPARA_CRC    5
#define SELFPARA_PROTOCOL   6 //�����û��
#define SELFPARA_DEVTYPE      7
#define SELFPARA_ID       8
#define SELFPARA_MAC    9

#define SELFPARA_IP1       10
#define SELFPARA_IP2       11

#define SELFPARA_COM1    12
#define SELFPARA_COM2    13
#define SELFPARA_COM3    14
#define SELFPARA_COM4    15

#define SELFPARA_PT2     16
#define SELFPARA_CT2      17

#define SELFPARA_MFR_GB2312         18
#define SELFPARA_PROTOCOL_GB2312   19
														   
#define SELFPARA_NUM    20


#define SELFPARA_OSNAME  "other 1.0.0.0"
#define SELFPARA_DEVTYPE_NAME "iPACS-5612%s"
#define HVVER_LEN  7
#define SVVER_LEN  8


#define RUNPARA_ADDR  0x8020
#define RPR_DV_NUM      0x06
#define RPR_DV_I               0x00
#define RPR_DV_AC            0x01
#define RPR_DV_DC            0x02
#define RPR_DV_P              0x03
#define RPR_DV_F               0x04
#define RPR_DV_COS         0x05

#define RPR_PT1                 0x06
#define RPR_PT2                 0x07
#define RPR_V_DY              0x08
#define RPR_T_DY              0x09
#define RPR_V_GY              0x0A
#define RPR_T_GY              0x0B
#define RPR_V_ZZ               0x0C
#define RPR_T_ZZ               0x0D
#define RPR_V_GZ              0x0E
#define RPR_T_GZ              0x0F
#define RPR_T_YXFD         0x10
#define RPR_T_FZ               0x11
#define RPR_T_HZ              0x12
#define RPR_T_HH              0x13
#define RPR_T_HHT            0x14
#define RPR_V_ILP              0x15
#define RPR_V_ULP            0x16
#define RPR_V_UYX           0x17
#define RPR_V_IYX            0x18
#define RPR_V_IYXYB        0x19


#define RUNPARA_LINEADDR 0x8040
#ifdef _TEST_VER_
#define RUNPARA_LINENUM   20
#else
#define RUNPARA_LINENUM   16
#endif
#define RPR_LINE_CT1       0x00
#define RPR_LINE_CT2       0x01
#define RPR_LINE_CT01     0x02
#define RPR_LINE_CT02     0x03

#define RUNPARA_LINEADDR1 0x804A
#define PRP_LBYB_GL        0x0A  //����¼��
#define PRP_LBYB_WY        0x0B // ʧѹ¼��
#define PRP_LBYB_I0       0x0C  //  ����ͻ��
#define PRP_LBYB_U0        0x0D //��ѹͻ��
#define RPR_V_IL1P            0x0E //������Ư
#define RPR_V_UL1P            0x0F //��ѹ��Ư
#define RPR_V_ZLDYLP            0x10 //ֱ����ѹ��Ư
#define RPR_V_PLP            0x11 //������Ư
#define RPR_V_FLP            0x12 //Ƶ����Ư
#define RPR_V_COSLP            0x13 //����������Ư


#define PRPARA_ADDR    0x8220     //������ֵ�׵�ַ
#define PRP_YB_LEDKEEP      0x00         //���ϵ��Զ�����Ͷ��
#define PRP_T_LEDKEEP         0x01		  //���ϵ��Զ�����ʱ��
#define PRP_T_YXKEEP     0x02		
#define PRP_YB_FTUSD    0x03
#define PRP_T_X                0x04
#define PRP_T_Y                0x05
#define PRP_T_C                0x06
#define PRP_T_S                0x07
#define PRP_T_DXJD         0x08        //����ӵ���բʱ��
#define PRP_T_HZ             0x09       //ѡ����բ�غ�ʱ��
#define PRP_YB_ZSYI       0x0A        //����Ӧ����·���ϴ���
#define PRP_YB_ZSYI0     0x0B       //����Ӧ����ӵع��ϴ���
#define PRP_YB_CH1        0x0C       //һ���غ�բ
#define PRP_T_CH1           0x0D       //һ���غ�ʱ��
#define PRP_YB_IH           0x0E       //�����������Ͷ��
#define PRP_I_IH              0x0F       //����������ض�ֵ
#define PRP_YB_CHN       0x10        // ����غ�բͶ��
#define PRP_I_CHN          0x11        // �غ�բ����
#define PRP_T_CHN          0x12        // �ǵ�һ���غ�բ��ʱ
#define PRP_YB_JDMODE 0x13       //ϵͳ�ӵط�ʽ��0�����Ե㲻�ӵ�1����������Ȧ�ӵ�
														   // 2����С����ӵ�3��������ӵ�
#define PRP_YB_FDFJ       0x15       // �ֶλ�ֽ磬0���ֶ�/���翪��1���ֽ翪��
#define PRP_YB_FDLL       0x16       // ����ֶ�ģʽ��0 �ֶ� 1����
#define PRP_YB_JDFA       0x17      // �͵���FAģʽ��0������Ӧ�ۺ��� FA  1: ��ѹʱ��2: ��ѹ����


#define PRPARA_LINE_ADDR     0x8240
#define PRPARA_LINE_ADDREND  0x8400
#define PRPARA_LINE_NUM      0x20

#define PRP_YB_GLTRIP    0x00      //����ͣ����բͶ��
#define PRP_YB_I1TT      0x01      //����һ��Ͷ��
#define PRP_YB_I1TRIP     0x02      //����һ�γ���
#define PRP_I_I1                0x03      //����һ�ζ�ֵ
#define PRP_T_I1               0x04      //����һ��ʱ��
#define PRP_YB_I2TT        0x05      //��������Ͷ��
#define PRP_YB_I2TRIP    0x06      //�������γ���
#define PRP_I_I2               0x07      //�������ζ�ֵ
#define PRP_T_I2               0x08      //��������ʱ��
#define PRP_YB_I0TT        0x09      //�������Ͷ��
#define PRP_YB_I0TRIP     0x0A     //�����������
#define PRP_I_I0                0x0B     //���������ֵ
#define PRP_T_I0               0x0C     //�������ʱ��
#define PRP_YB_ILTT       0x0D     //С�����ӵ�Ͷ��
#define PRP_YB_ILTRIP    0x0E      //С�����ӵس���
#define PRP_YB_FZDBS    0x10   // ���ڶϵ�������Ͷ��
#define PRP_I_FZDBS    0x11   // ���ڶϵ�����ֵ
#if 0
#define PRP_YB_GLLB       0x11      //��������¼��
#define PRP_YB_SYLB       0x12     //��·ʧѹ¼��
#define PRP_YB_U0LB       0x13      //�����ѹ¼��
#define PRP_YB_I0LB        0x14      //�������¼��
#endif
#define PRPARA_TTU_ADDR     0x8401	//TTU ��������
#define PRPARA_TTU_ADDREND  0x8439
#define PRPARA_TTU_NUM      0x57
#define PRP_CAPACITY_TTU	0x00	//�������
#define PRP_UNBALANCE_I		0X01	//������ƽ�ⶨֵ
#define PRP_UNBALANCE_V		0x02	//��ѹ��ƽ�ⶨֵ
#define PRP_RATE_TIME_1		0X03	//���ܼ���ʱ��1
#define PRP_RATE_TIME_2		0X04	//���ܼ���ʱ��2
#define PRP_RATE_TIME_3		0X05	//���ܼ���ʱ��3
#define PRP_RATE_TIME_4		0X06	//���ܼ���ʱ��4
#define PRP_RATE_TIME_5		0X07	//���ܼ���ʱ��5
#define PRP_RATE_TIME_6		0X08	//���ܼ���ʱ��6
#define PRP_RATE_TIME_7		0X09	//���ܼ���ʱ��7
#define PRP_RATE_TIME_8		0X0A	//���ܼ���ʱ��8
#define PRP_RATE_TIME_9		0X0B	//���ܼ���ʱ��9
#define PRP_RATE_TIME_10	0X0C	//���ܼ���ʱ��10
#define PRP_RATE_TIME_11	0X0D	//���ܼ���ʱ��11
#define PRP_RATE_TIME_12	0X0E	//���ܼ���ʱ��12
#define PRP_RATE_TIME_13	0X0F	//���ܼ���ʱ��13
#define PRP_RATE_TIME_14	0X10	//���ܼ���ʱ��14
#define PRP_RATE_TIME_15	0X11	//���ܼ���ʱ��15
#define PRP_RATE_TIME_16	0X12	//���ܼ���ʱ��16
#define PRP_RATE_TIME_17	0X13	//���ܼ���ʱ��17
#define PRP_RATE_TIME_18	0X14	//���ܼ���ʱ��18
#define PRP_RATE_TIME_19	0X15	//���ܼ���ʱ��19
#define PRP_RATE_TIME_20	0X16	//���ܼ���ʱ��20
#define PRP_RATE_TIME_21	0X17	//���ܼ���ʱ��21
#define PRP_RATE_TIME_22	0X18	//���ܼ���ʱ��22
#define PRP_RATE_TIME_23	0X19	//���ܼ���ʱ��23
#define PRP_RATE_TIME_24	0X1A	//���ܼ���ʱ��24
#define PRP_RATE_TIME_25	0X1B	//���ܼ���ʱ��25
#define PRP_RATE_TIME_26	0X1C	//���ܼ���ʱ��26
#define PRP_RATE_TIME_27	0X1D	//���ܼ���ʱ��27
#define PRP_RATE_TIME_28	0X1E	//���ܼ���ʱ��28
#define PRP_RATE_TIME_29	0X1F	//���ܼ���ʱ��29
#define PRP_RATE_TIME_30	0X20	//���ܼ���ʱ��30
#define PRP_RATE_TIME_31	0X21	//���ܼ���ʱ��31
#define PRP_RATE_TIME_32	0X22	//���ܼ���ʱ��32
#define PRP_RATE_TIME_33	0X23	//���ܼ���ʱ��33
#define PRP_RATE_TIME_34	0X24	//���ܼ���ʱ��34
#define PRP_RATE_TIME_35	0X25	//���ܼ���ʱ��35
#define PRP_RATE_TIME_36	0X26	//���ܼ���ʱ��36
#define PRP_RATE_TIME_37	0X27	//���ܼ���ʱ��37
#define PRP_RATE_TIME_38	0X28	//���ܼ���ʱ��38
#define PRP_RATE_TIME_39	0X29	//���ܼ���ʱ��39
#define PRP_RATE_TIME_40	0X2A	//���ܼ���ʱ��40
#define PRP_RATE_TIME_41	0X2B	//���ܼ���ʱ��41
#define PRP_RATE_TIME_42	0X2C	//���ܼ���ʱ��42
#define PRP_RATE_TIME_43	0X2D	//���ܼ���ʱ��43
#define PRP_RATE_TIME_44	0X2E	//���ܼ���ʱ��44
#define PRP_RATE_TIME_45	0X2F	//���ܼ���ʱ��45
#define PRP_RATE_TIME_46	0X30	//���ܼ���ʱ��46
#define PRP_RATE_TIME_47	0X31	//���ܼ���ʱ��47
#define PRP_RATE_TIME_48	0X32	//���ܼ���ʱ��48
#define PRP_SWTICH_TIME_CYC	0X33	//�뿪�صĶ�ʱ����
#define PRP_SHORTCIR_CAP	0X38	//�����С��·����

#define PRP_HAR_DEAD_V		0X34	//��ѹг������
#define PRP_HAR_DEAD_I		0X35	//����г������
#define PRP_DEAD_UNBALANCE	0X36	//��ƽ�������
#define PRP_DEAD_LOAD		0X37	//����������
#define PRPARA_SWITCH_ADDR     0x8481	//��ѹ����������ʼ��ַ
#define PRPARA_SWITCH_ADDREND  0x84D1	//��ѹ�������ؽ�����ַ
#define PRPARA_SWITCH_NUM		PRPARA_SWITCH_ADDREND-PRPARA_SWITCH_ADDR+1
#define PRPARA_EVER_SWITCH_NUM 30	//ÿ������Ĳ�������

#define PRP_RESDUAL_I_VALUE   	 0X00	//�ʣ���������ֵ
#define	PRP_RESDUAL_I_TIME   	 0X01	//����޲�����ʱ��
#define	PRP_RESDUAL_I_CONTROL  	 0X02	//ʣ���������������(������4��
#define	PRP_SHORT_I_VALUE		 0X03	//��·˲ʱ����������ֵ
#define PRP_SHORT_I_CONTROL		 0X04	//��·˲ʱ����������(������3��
#define	PRP_SHORTDELAY_I_VALUE	 0X05	//��·����ʱ����������ֵ
#define	PRP_SHORTDELAY_I_TIME	 0X06	//��·����ʱ����ʱ�䶨ֵ
#define	PRP_SHORTDELAY_I_CONTROL 0X07	//��·����ʱ����������(������3��
#define	PRP_OVERLOAD_I_VALUE	 0X08	//���ر���������ֵ
#define	PRP_OVERLOAD_I_TIME		 0X09	//���ر���ʱ�䶨ֵ
#define	PRP_OVERLOAD_I_CONTROL	 0X0A	//���ر���������(������3��
#define	PRP_OVER_U_VALUE		 0X0B	//��ѹ������ѹ��ֵ
#define	PRP_OVER_U_CONTROL		 0X0C	//��ѹ����������(������2��
#define	PRP_UNDER_U_VALUE		 0X0D	//Ƿѹ������ѹ��ֵ
#define	PRP_UNDER_U_CONTROL		 0X0E	//Ƿѹ����������(������2��
#define	PRP_BREAKPHASE_U_VALUE	 0X0F	//���ౣ����ѹ��ֵ
#define	PRP_BREAKPHASE_U_CONTROL 0X10	//���ౣ��������(������2��
#define	PRP_ZERO_U_CONTROL		 0X11	//ȱ�㱣��������(������2��
#define	PRP_SELF_CHECK_TIME		 0X12	//��ʱ�Լ�ʱ��
#define	PRP_REST_GATE_CONTROL	 0X13	//�غ�բ������(������1��
#define	PRP_SWITCH_RETURN_TIME	 0X14	//����ң�Ÿ���ʱ��


#if(DEV_SP  == DEV_SP_TTU)
#define PARA_END PRPARA_SWITCH_ADDREND
#else
#define PARA_END PRPARA_LINE_ADDREND
#endif
#define FILE_TYPE_PR       0x01
#define FILE_TYPE_RUN      0x02
#define FILE_TYPE_PEOGRAM 0x03

#pragma pack(1)

struct VParaInfo{
	BYTE type;
	BYTE len;
	char valuech[PARACHAR_MAXLEN];
};	

struct VNextParaInfo{
	WORD infoaddr;
	BYTE type;
	BYTE len;
	char valuech[PARACHAR_MAXLEN];
};	

struct VParaDef{
	DWORD parano;
	char paraname[30];
};

struct VFileHead{
	int nLength;      /*�ļ����ȣ������ļ�ͷ,����,�ļ�У������ind�ļ���*/
	WORD wVersion;    /*ʮ��������ʾ,�ΰ汾��ռ��λ.��1.03Ϊ103*/
	WORD wAttr;       /*��������*/
	DWORD rsv[5];
};

struct VNetIP{
	char sIP[20];
	DWORD dwMask;
};

//�ṹ������GetSubParaFile()�кܴ��ϵ,�������׸Ķ�
struct VPAddr{
	WORD  wAddr;         /*�豸��ַ,ȫϵͳΨһ,��Lan1��Ч���Զ�ȡLan1�ĵ����Ҳ�����Ķ�*/
	struct VNetIP Lan1;
	WORD  wCantonCode;
	char  sExtAddr[12];

	WORD  wAsMasterAddr; /*�豸��Ϊ��վ���ĵ�ַ,ȱʡ��Addr��ͬ,ά����ʱ����ʾ*/
	char  sTelNo[20];

	struct VNetIP Lan2;
	char sGateWay1[20];

    BYTE byDefFlag;      /*�����Ƿ���ȱʡ���ɱ�־,��Ӧά�����,�̶�Ϊ0x55*/     
	BYTE byValidFlag;    /*������Ч��־,�̶�Ϊ0x55��Ч*/     

	char sGateWay2[20];
	char sGW2_Dest[20];
};

#define FD_CFG_PT_58V   0x20      

struct VPFdCfg{
	char kgname[GEN_NAME_LEN];
	DWORD kgid;  /*���治��ʾ���Զ�ȡװ�õ�ַ׺�ϻ��ߺţ����ַΪ2�ĵ�1���ߣ�Ϊ201*/

	DWORD cfg;   /*D0 = 1  ��ѹ�ȼ�Ϊ��ѹ  =0 ��ѹ�ȼ�Ϊ��ѹ(Ĭ��)
	               D1 = 1  ����һ�β������ϴ�ʱ����һλ��Чλ(��Ĭ��)
				   D2 = 1  ����һ�β๦���ϴ�ʱ��λΪMw��Mvar(��Ĭ��)
				   D3 = 1  �޹�����������Ч  =0 ���޹�����(Ĭ��)
				   D4 = 1  ���ϼ����Ч =0 ���ϼ����Ч(Ĭ��)
				   D5 = 1  ��ѹ����Ϊ���ѹ =0 ����Ϊ�ߵ�ѹ(Ĭ��)
				 */

	WORD Un;      /*���ζ��ѹ:220V��100V,ֵȡ220��100*/
    WORD In;      /*���ζ����:5A��1A,ֵȡ5��1*/
	
	int pt;       /*PT���ã�X/Un*/
	int ct;       /*CT���ã�X/In*/
	
	char  kg_stateno;    //��Ӧ����״̬Ӳң�ŵ��
	char  kg_faultno;    //��Ӧ���ع���Ӳң�ŵ��, -1��ʾϵͳ��ң��
	char  kg_ykno;       //��Ӧ����ң�ص��
	char  kg_vectorno;   //��Ӧ���ع��Ϸ���Ӳң�ŵ��, -1��ʾϵͳ��ң��        

	BYTE  kg_stateno_ioaddr;  //����״̬ң�Ŷ�Ӧ��io�����ַ, 0Ϊ����
	BYTE  kg_faultno_ioaddr;  //���ع���ң�Ŷ�Ӧ��io�����ַ, 0Ϊ����
	BYTE  kg_ykno_ioaddr;     //����ң�ض�Ӧ��io�����ַ, 0Ϊ����
	BYTE  kg_vectorno_ioaddr; //���ع��Ϸ���ң�Ŷ�Ӧ��io�����ַ, 0Ϊ����
    
	WORD  In0;
	WORD  rsv1;
	int   ct0;

	char  kg_closeno;       //��Ӧ�����ֺ�ң��
	char  kg_openno;      //��Ӧ�����ַ�ң��
	char  kg_startno;      //��Ӧ��������ң��
	char  kg_unlockno;	//��Ӧ���ر���ң��

	char kg_ykkeepno;   //��Ӧң�ر��ֵĵ��
	char kg_remoteno;   //������Զ����Ӧ��ң�ŵ��
    BYTE rsv2[2];
    DWORD rsv[4];	
};	

struct VPAiCfg{
    int type;      //��YC_TYPE
    int rsv1;    
	DWORD  cfg;    //��Ϊ0
	DWORD admin;
	DWORD admax;
	DWORD rsv[3];
};

struct VPDiCfg{
    int type;     /*=0 ң�ţ����㣩
                    =1 �������루���ݿ��д��������������͵��֮��*/
	DWORD  cfg;   /*D0��=1 ң��ȡ����=0 ��ȡ��	  
					D1��ʹ�ܣ�=1 ������Ч��=0 ������Ч
					D2=1 ������SOE	��0����SOE
					D3=1 ������COS	��0����COS
					*/
	
    WORD  dtime;  /*ң�ŷ���ʱ���������(1ms)*/
	WORD  yxno;   /*��ͨ����Ӧ���߼�ң�ŵ��*/

	WORD T1;
	WORD N;
	float T2;
	WORD re_yxno;
	WORD rsv[5];
};

struct VPDoCfg{
	int type;           /*=0 ˫���� (�Ϸ�˫�̵���)
	                      =1 ������ (��һ���̵���)
                          ��������ʱontime��Ч
                        */  	
	DWORD cfg;          //��Ϊ0
    DWORD hzontime;       /*ң�رպ�ʱ��,1ms��λ��Ĭ��2000*/
    short ybno;         /*ѹ������: -2��ѹ��
                                    -1��ѹ��
                              >=0Ӳѹ��ң�ź�
                        */ 
    WORD pad;                 
	DWORD yztime; 		/*ң��Ԥ�ó�ʱʱ��,1ms��λ,Ĭ��30000*/	
	DWORD fzontime;
	DWORD rsv[2];	    
};	

struct VPYcCfg{
	int type;
    DWORD  cfg;     //��Ϊ0
    
	int arg1;
	int arg2;
	short toZero;     //��0ֵ��ȱʡΪ5

	WORD bak[5];	
	DWORD rsv[5];	
};

struct  VPSysConfig
{
	DWORD dwType;     /*�豸�ͺ�(���ֽڣ� ��ģ�����ɣ�����Ա����*/
	DWORD dwName;     /*�豸��ʶ��ϵͳ��Ψһ��ȡ(dwType&0xFFF)+���豸��ϵͳ����Ŀ,����ʾ*/
	char  sByName[GEN_NAME_LEN];   /*�豸����,�豸����ʱ�û���ά������*/
	DWORD dwCfg;     /*D0=1 �����Զ��
	                 				 D1=1 sntp��ʱ
	                 				 D2 =1 ��ң��1��ֵ�ϴ�ά��ң�����
	                 				 D3 = 1,�����汾,��Լ������Ϣ��չ
	                 				 D3 = 0����ǰ�汾��Լ��Ϣû��չ��������ѹ����û��
	                                 D4 = 1�������ѹС�źţ�D4 = 0�������ѹ�����
	                 			*/

	WORD  wFDNum;

	WORD  wAINum;
	WORD  wDINum;
	WORD  wDONum;	

	WORD  wYCNum;
	WORD  wDYXNum;
	WORD  wSYXNum;
	WORD  wDDNum;
	WORD  wYKNum;	
	WORD  wYTNum;

	WORD  wYcNumPublic;
	WORD  wYcNumAdd;
	DWORD dSntpServer;
	DWORD dwAIType;
	DWORD dwIOType;
	DWORD rsv[5]; 	
};

struct VPSysExtConfig
{
    char sWsName[GEN_NAME_LEN];
	char sWsSechema[GEN_NAMELONG_LEN];
	DWORD dWsIp;
	DWORD dWsVer;
	DWORD dwMaintIp;
	BYTE  byMac[6];
	DWORD dwKgTime;
	struct VPDiCfg YxIn[8];
	char xmlVer[16];
	char xmlRev[16];
	DWORD xmlCrc;
	struct VNetIP Lan3;
	char sGateWay3[20];
	short YXNO[12];
	BYTE YKDelayTime;
	struct VNetIP Lan4;
	char sGateWay4[20];
	char sRoute3[20];
	char sRoute4[20];
	BYTE rsv1[3];
	DWORD rsv[52];
};

struct VPFdExtConfig
{
    short kg_fabsno[8];
	short fabs_eqpid[8];
	WORD rsv[24];
};

struct VPCellCfg{
	DWORD dwCfg;        //D0--��ػʹ��
	DWORD dwNum;        //��������
	WORD  wID;          //��ػID
	WORD Udis;  //��ػ��ֵ
    WORD wrsv;
	DWORD rsv[4];
};	

#define CELL_MODE_ENDTIME            0x80000000	
struct VPCellCtrl{
	DWORD dwMode;       //0--��ֹ 1--����ģʽ 2---����ģʽ, bit31:����ʱ��
	WORD wMonthBits;
	WORD wWeekBits;
	DWORD dwDayBits;
	WORD wHour;
	WORD wMin;

	DWORD dwTime;

	DWORD rsv[4];	
};		

struct VPUSwitchCfg{
	DWORD dwCfg;        //D0--ʹ��
	DWORD dwNum;        //��������

	DWORD rsv[5];
};	
	
struct VPUSwitchCtrl{
	DWORD dwMode;       //0--��ֹ1--��ѹģʽ 2-����״̬ģʽ
	WORD wUFdNo1;       //�ɼ����ڵĵ�ѹ��� 
	WORD wUFdNo2;       //�ɼ����ڵĵ�ѹ��� 

	WORD wUFdYx1;	
	WORD wUFdYx2;
	
	DWORD rsv[5];	
};		

struct VPExtIoAddr{
	DWORD dwType;
	WORD  wAddr;         
	struct VNetIP Lan;
};

struct VPExtIoInfo{
    struct VPExtIoAddr ioAddr;
	DWORD rsv[10];  	
};	

#define PARA_SERIAL_START_NO      50
#define PARA_SERIAL_NUM           16
#define PARA_NET_SYS_NUM          10

#define PARA_NET_START_NO         (PARA_SERIAL_START_NO+PARA_SERIAL_NUM)
#define PARA_NET_USR_NO           (PARA_NET_START_NO+PARA_NET_SYS_NUM)

#define PARA_CAN_STARTNO          (PARA_NET_START_NO-1)
#define PARA_START_NO             PARA_SERIAL_START_NO

struct VPPort{
	int id;                   /*ͨ�Ŷ˿ںţ�
                                ���ڣ�����1-����15��Ӧ�˿ں�Ϊ51-65
                                ��̫�����ӣ�Socket1��Ӧ�˿ں�Ϊ76���Դ�����*/
	char pcol[GEN_NAME_LEN];  /*��Լ���ƣ���protocol.cin��һ��*/
	int pcol_attr;            /*��Լ���ԣ�0��������Լ   1���ӷ���Լ*/
	char cfgstr[3][30];   /*��Լ���ô�
                            ��������:type:ip:port-1:0.0.0.0:8080
                            �ͻ���:type:ip:port-2:192.166.0.2:8080
							����:type:baud,databits,stopbits,parity-4:9600,8,1,N  

							�˿����� (type) 
							0:NULL
							1:TCP-SERVER
							2:TCP-CLIENT
							3:UDP
							4:RS232 */ 

	int bakmode;    //�˿�ת��ģʽ
	int bakcomm;    //ת���˿�
	
	char modelname[GEN_NAMETAB_LEN+1];
	DWORD rsv[6];	
};
  
struct VPEqp{  
	DWORD dwName;
    char sCfgName[GEN_NAME_LEN];  /*��Ӧ�����ļ����ƣ��������ļ�����׺,�磺.cde��.cdt��*/
	WORD wCommID;  /*��Ӧ�˿ں�*/
	DWORD rsv[5];  
};

struct VPTECfg{
	WORD wSourceAddr;  /*������ַ (��̬�����ʱ����ʾ-�Զ�ȡAddr.cfg�е�Addr)*/
	WORD wDesAddr;     /*�¼�װ�õ�ַ*/
	char sExtAddr[12]; /*��չ��ַ*/
	char sTelNo[20];   /*�ֻ�����*/

	DWORD dwFlag;  /*D0:ң���ϵ��
	                 D1:��ȳ�ϵ��
	               */  
	               
	WORD wYCNum;
	WORD wVYCNum;	/*����ң�����*/
	WORD wDYXNum;
	WORD wSYXNum;
	WORD wVYXNum; /*����ң�Ÿ���*/
	WORD wDDNum;
	WORD wYKNum;
	WORD wYTNum;
	WORD wTSDataNum;
	WORD wTQNum;

	BYTE byYCFrzMD;   //ң�ⶳ���ܶ� 1-60min Ĭ��15min
	BYTE byYCFrzCP;   //ң�ⶳ������ 1-180day Ĭ��30day

	BYTE byDDFrzMD;   //ң�ⶳ���ܶ� 5-60min Ĭ��60min
	BYTE byDDFrzCP;   //ң�ⶳ������ 1-180day Ĭ��30day	
	
	DWORD rsv[4]; 	
};

struct VPTrueYC{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;  /*D0=1 ����(ȱʡ) =0 ������
	                D1=1 ���� =0 ����������(ȱʡ)
	                D2=1 �ն��� =0���ն���(ȱʡ)
	                D3=1 �¶��� =0���¶���(ȱʡ)	                
					D4��D5:  ��ֵһ���� 
					   0����Ч
					   1������
					   2������
					D6��D7:  ��ֵ������ 
					   0����Ч
					   1������
					   2������
					D8=1 �ռ�ֵ���� =0�������ռ�ֵ����(ȱʡ)
					D9=1 �������ͽ�ֹ =0������������(ȱʡ)

					D29��1 ��ȡ�� =0����(ȱʡ)
					D30��1 �ǵ�ǰֵ =0����(ȱʡ)
					D31��1 ���� =0����(ȱʡ)
					*/
	long  lA;     /*ϵ��      ȱʡ1*/
	long  lB;     /*����ֵ    ȱʡ1*/
	long  lC;     /*����ֵ    ȱʡ0     ������� vlaue*a/b+c*/
	WORD  wFrzKey; //���ݶ���ؼ���	

	long  lLimit1;	    //��ֵ1
	WORD  wLimitT1;		//��ֵ1ʱ�� S
	WORD  wLimit1VYxNo;  //��ֵ1��Ӧ��ң�ź�
	long  lLimit2;	    //��ֵ2
	WORD  wLimitT2;		//��ֵ2ʱ�� S
	WORD  wLimit2VYxNo;	//��ֵ2��Ӧ��ң�ź�
	DWORD rsv[5];
};

struct VPTrueYX{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;  /*D0=1 ����(ȱʡ) =0 ������
                    D1=1 ȡ�� =0 ��ȡ��(ȱʡ)
                    D2=1 ���ݿ����SOE  =0 ������(ȱʡ)
                    D3=1���ݿ����COS  =0 ������(ȱʡ)
                    D4=1 ���������ź�  =0 ������(ȱʡ)

					D29��1 ��ȡ�� =0����(ȱʡ)
					D30��1 �ǵ�ǰֵ =0����(ȱʡ)
					D31��1 ���� =0����(ȱʡ)*/
	DWORD rsv[5];
};

struct VPTrueDD{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;  /*D0=1 ����(ȱʡ) =0 ������
	                D1=1 ���� =0 ����������(ȱʡ)
	                D2=1 �ն��� =0���ն���(ȱʡ)
	                D3=1 �¶��� =0���¶���(ȱʡ)
	                
					D29��1 ��ȡ�� =0����(ȱʡ)
					D30��1 �ǵ�ǰֵ =0����(ȱʡ)
					D31��1 ���� =0����(ȱʡ)
				*/
    long  lA;     /*ϵ��      ȱʡ1*/
    long  lB;     /*����ֵ    ȱʡ1*/    
    long  lC;     /*ԭʼֵ    ȱʡ0*/
	WORD  wFrzKey; //���ݶ���ؼ���
	DWORD rsv[5];
};

struct VPTrueCtrl{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;
    WORD wID;
	DWORD rsv[5];
};

struct VPTSDataCfg{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;	  /*D0=1 ת��Ϊ����ң�� =0��ת��(ȱʡ)
				        D1=1 ת��Ϊ�� =0 ת��Ϊ��*/
    WORD wKeyWordNum;
    BYTE abyKeyWord[14];
    WORD awOffset[14];
    WORD wVYXNo; /*��Ӧ��ң�ŵ��*/
	DWORD rsv[5];
};

#define FA_VEQP_FLAG        0x80000000
struct VPVECfg{
	WORD wSourceAddr;   /*������ַ*/
	WORD wDesAddr;	    /*�ϼ�װ�õ�ַ*/
	char sExtAddr[12];
	char sTelNo[20];	/*�ֻ�����*/

    DWORD dwFlag;       /*D0=1:ң���ϵ���� 
                          D1-D15: ����*/

	WORD wYCNum;
	WORD wDYXNum;
	WORD wSYXNum;
	WORD wDDNum;
	WORD wYKNum;
	WORD wYTNum;
	WORD wTQNum;
	DWORD rsv[5];   
};

struct VPVirtual{  /*������װ��Ԫ�صĽṹͷ��������˽ṹһ��*/
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;       
    DWORD dwTEName;   /*���õ�ʵװ�õ�Name*/
    WORD wOffset ;    /*��ʵװ���е�ƫ��*/
    WORD wSendNo;    /*�������*/
};

struct VPVirtualYC{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;      /*D0-D7 ����
                        D9=1 �������ͽ�ֹ =0������������(Ĭ��)
                        D11=1�������ߡ���ֵ��=0 ���������߼�ֵ
	                  */
    DWORD dwTEName;   /*���õ�ʵװ�õ�Name*/
    WORD wOffset ;    /*��ʵװ���е�ƫ��*/
    WORD wSendNo;    /*�������*/
    long  lA;  /*ϵ��      ȱʡ1*/
    long  lB;  /*����ֵ    ȱʡ1*/
    long  lC;  /*����ֵ    ȱʡ0     ������� vlaue*a/b+c*/ 
	WORD  wFrzKey;
	DWORD rsv[5];   
};

struct VPVirtualYX{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;       /*D0=1  ��ͬ������������
                           =0  ��ͬ������Ż����
                         D1-D15 ����*/
    DWORD dwTEName;   /*���õ�ʵװ�õ�Name*/
    WORD wOffset ;    /*��ʵװ���е�ƫ��*/
    WORD wSendNo;    /*�������*/
	DWORD rsv[5];   
};

struct VPVirtualDD{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;      /*D0=1  �����ȣ��ܼ�ʱ������
                          =0  �����ȣ��ܼ�ʱ���ӣ�
						D1=1 ����(ȱʡ) =0 ����������(ȱʡ)	
                        D2-D15 ����*/ 
    DWORD dwTEName;   /*���õ�ʵװ�õ�Name*/
    WORD wOffset ;    /*��ʵװ���е�ƫ��*/
    WORD wSendNo;    /*�������*/
	WORD  wFrzKey;
	DWORD rsv[5];   
};

struct VPVirtualCtrl{ 
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
    DWORD dwCfg;       
    DWORD dwTEName;   /*���õ�ʵװ�õ�Name*/
    WORD wOffset ;    /*��ʵװ���е�ƫ��*/

    WORD wID;
	DWORD rsv[5];   
};

struct VPEGCfg{
	WORD wSourceAddr;  /*������ַ*/
	WORD wDesAddr;     /*�ϼ�װ�õ�ַ*/

	DWORD dwFlag;      /*D0: =0ת��FA(Ĭ��) =1 ��ת��
	                     D1: =0ת��QcEvent(Ĭ��) =1��ת��
	                     D2: =0ת��ActEvent(Ĭ��) =1��ת��
	                     D3: =0ת��DoEvent(Ĭ��) =1��ת��
	                     D4: =0ת��WarnEvent(Ĭ��) =1��ת��
	                     D5-D31:����*/            
	WORD wEqpNum;

	DWORD rsv[5];   
};

#ifdef DB_EPOWER_STAT
struct VPEECfg{ 			/*epower���װ������*/
	DWORD dwCfg1; 			  /*D0-�������������
								D1-�������������
								D2-���޹��������������
								D3-���޹��������������
								D4-�����޹��������������
								D5-�����޹��������������
								*/	  
	DWORD dwCfg2; 		   
	WORD wPQ_DD_Frz_Cyc;	  /*min*/
	WORD wXX_DD_Frz_Cyc; 

	DWORD dwCBTimeCfgDay;   /*D0-D30��Ӧÿ��1��-31��*/
	BYTE  byCBTimeCfgHou;
	BYTE  byCBTimeCfgMin;  

	DWORD adwRsv[8];	
};	
#endif

//ע��:addr.cfg�߼��ļ��Ƚ�����,���̶��洢����.ind�ļ�֮��,
//������.pkg�ļ���,��˶�Ӧadd.cfg�߼��ļ�Ҫ���⴦��
struct VPIndInfo{
    char cfgName[MAXFILENAME];  
	int cfgOffset;   //�߼��ļ���pkg�ļ��е�ƫ��
	int cfgLen;    //�߼��ļ����ȣ����߼��ļ�ͷһ��
	WORD cfgVer;  //�߼��ļ��汾�����߼��ļ�ͷһ��
	WORD cfgCrc;  //�߼��ļ�У��
	WORD pkgIndex;  //���߼��ļ����ڵ�pkg�ļ����
	DWORD dwRsv[5];
};

//ÿ��pkg��Ϣ��Ӧ���ļ����̶�Ϊcscxx.pkg  xxΪ����Ϣ��ŵ���λʮ��������ʾ
//���һ��pkg����Ϣ�ļ���Ϊcsc00.pkg �Դ�����.
struct VPPkgInfo{
	char pkgName[MAXFILENAME];
	int pkgLen;     /*pkg�ļ����ȣ������ļ�ͷ,����,������У�飬��pkg�ļ�ͷһ��*/
	WORD pkgVer;    /*pkg�汾�ţ�ʮ��������ʾ,�ΰ汾��ռ��λ.��1.03Ϊ103����pkg�ļ�ͷһ��*/
	WORD pkgCrc;    /*pkg�ļ�У�飬��pkg�ļ�βһ��*/
	int pkgValid;   /*������¼��Ϣ�Ƿ���Ч,��ЧΪ1,��ЧΪ0*/
	DWORD rsv[5];
};	


#define MAX_PKG_NUM   40
#define MAX_PKG_SIZE (50*1024)  /*50k*/
	
struct VPIndHead{
	struct VPAddr addr;      /*addr.cfg�߼��ļ��洢��*/
	WORD wAddrVer;    /*addr.cfg�߼��ļ��汾*/
	int nLength;      /*�ļ����ȣ������ļ�ͷ,����,������У��*/
	WORD wVersion;    /*ʮ��������ʾ,�ΰ汾��ռ��λ.��1.03Ϊ103*/
	WORD indNum;      /*�ļ�������Ϣ����*/
	DWORD rsv[5];
	struct VPPkgInfo pkgInfo[MAX_PKG_NUM];
};	

struct VPPkgHead{
	DWORD nLength;   /*�ļ����ȣ������ļ�ͷ,����,������У��*/
	WORD wVersion;   /*ʮ��������ʾ,�ΰ汾��ռ��λ.��1.03Ϊ103*/
	WORD pad;         
	DWORD rsv[5];
};	

struct VFileMsg{
    BYTE type;
	BYTE attr[5];
};

struct VFileMsgs{
	BYTE type;
    BYTE num;
	struct VFileMsg filemsg[20];
};

struct VFileSynchro{
	BYTE num;
	WORD wTaskID[10];
};

#pragma pack()
extern struct VFileHead *g_pParafileTmp;
extern DWORD g_paraFTBSem;

WORD GetParaCrc16(const BYTE *p, const int l);
DWORD GetParaCrc32(BYTE *p, int l);

int ReadParaFile(const char *filename, BYTE *buf, int buf_len);
int WriteParaFile(const char *filename, struct VFileHead *buf);
int DelParaFile(const char *filename);
int ReadParaFileCrc(const char *filename, WORD *crc);

int GetAppPortId(int para_port_id);
int GetParaPortId(int app_port_id);

int CreateSysConfig(const char *path);
int CreateExtIoConfig(char *path);

int CreateSysAddr(const char *path, struct VPAddr *pPAddr);
void CreateRunParaConfig(void);
#ifdef INCLUDE_CELL
int CreateCellConfig(void);
#endif
int SaveSysAddr(const char *path, const struct VPAddr *pPAddr);

int AddOnePort(const char *path, const struct VPPort *pPAddPort);

int CreateMinEqpSystme(const char *path);

int AddOneTrueEqp(char *path, DWORD type, struct VPTECfg *pPTECfg, struct VPEqp *pPAddEqp);

int SysParaInit(void);
BOOL ReadSelfRunParadef(WORD num,BYTE* buf);
int SetSysAddr(const WORD *addr, const char *ip1, const char *ip2, const char *gateway1, const char *gateway2);
int ReadPtCt(int fd, int *pt, int *Un, int *ct, int *In, int *ct0, int *In0);
int SavePtCt(int fd, const int *pt, const int *Un, const int *ct, const int *In, const int *ct0, const int *In0);
void WriteMsgFile(BYTE *pbuf);
void ReadfileSynchro(WORD wTaskID, WORD wEqpID, struct VFileSynchro *VFSyn);	
#ifdef __cplusplus
}
#endif

#endif
