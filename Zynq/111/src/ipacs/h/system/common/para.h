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

#define  MAXFILELEN   (80*1024)

#define CFG_FILE_VER          100   /*1.00*/

#define GEN_NAME_LEN          40
#define GEN_NAMELONG_LEN      128

#define  MAXFILENAME  32           /*�ļ����Ϊ32*/

#define PBYXNUM     	 12

#define PARACHAR_MAXLEN  64
#define Old_MAXLEN   	 30






#pragma pack(1)

struct VParaInfo{
	BYTE type;
	BYTE len;
	char valuech[PARACHAR_MAXLEN];
};	

struct VFileHead{
	int nLength;	  /*�ļ����ȣ������ļ�ͷ,����,�ļ�У������ind�ļ���*/
	WORD wVersion;	  /*ʮ��������ʾ,�ΰ汾��ռ��λ.��1.03Ϊ103*/
	WORD wAttr; 	  /*��������*/
	DWORD rsv[5];
};

#define FD_CFG_PT_58V   0x20      

struct VPFdCfg{
	char kgname[GEN_NAME_LEN];
	DWORD kgid;  /*���治��ʾ���Զ�ȡװ�õ�ַ׺�ϻ��ߺţ����ַΪ2�ĵ�1���ߣ�Ϊ201*/

	DWORD cfg;	 /*D0 = 1  ��ѹ�ȼ�Ϊ��ѹ  =0 ��ѹ�ȼ�Ϊ��ѹ(Ĭ��)
				   D1 = 1  ����һ�β������ϴ�ʱ����һλ��Чλ(��Ĭ��)
				   D2 = 1  ����һ�β๦���ϴ�ʱ��λΪMw��Mvar(��Ĭ��)
				   D3 = 1  �޹�����������Ч  =0 ���޹�����(Ĭ��)
				   D4 = 1  ���ϼ����Ч =0 ���ϼ����Ч(Ĭ��)
				   D5 = 1  ��ѹ����Ϊ���ѹ =0 ����Ϊ�ߵ�ѹ(Ĭ��)
				 */

	WORD Un;	  /*���ζ��ѹ:220V��100V,ֵȡ220��100*/
	WORD In;	  /*���ζ����:5A��1A,ֵȡ5��1*/
	
	int pt; 	  /*PT���ã�X/Un*/
	int ct; 	  /*CT���ã�X/In*/
	
	char  kg_stateno;	 //��Ӧ����״̬Ӳң�ŵ��
	char  kg_faultno;	 //��Ӧ���ع���Ӳң�ŵ��, -1��ʾϵͳ��ң��
	char  kg_ykno;		 //��Ӧ����ң�ص��
	char  kg_vectorno;	 //��Ӧ���ع��Ϸ���Ӳң�ŵ��, -1��ʾϵͳ��ң�� 	   

	BYTE  kg_stateno_ioaddr;  //����״̬ң�Ŷ�Ӧ��io�����ַ, 0Ϊ����
	BYTE  kg_faultno_ioaddr;  //���ع���ң�Ŷ�Ӧ��io�����ַ, 0Ϊ����
	BYTE  kg_ykno_ioaddr;	  //����ң�ض�Ӧ��io�����ַ, 0Ϊ����
	BYTE  kg_vectorno_ioaddr; //���ع��Ϸ���ң�Ŷ�Ӧ��io�����ַ, 0Ϊ����
	
	WORD  In0;
	WORD  rsv1;
	int   ct0;

	char  kg_closeno;		//��Ӧ�����ֺ�ң��
	char  kg_openno;	  //��Ӧ�����ַ�ң��
	char  kg_startno;	   //��Ӧ��������ң��
	char  kg_unlockno;	//��Ӧ���ر���ң��

	char kg_ykkeepno;	//��Ӧң�ر��ֵĵ��
	char kg_remoteno;	//������Զ����Ӧ��ң�ŵ��
	BYTE rsv2[2];
	DWORD rsv[4];	
};	

struct VPAiCfg{
	int type;	   //��YC_TYPE
	int rsv1;	 
	DWORD  cfg;    //��Ϊ0
	DWORD admin;
	DWORD admax;
	DWORD rsv[3];
};

struct VPDiCfg{
	int type;	  /*=0 ң�ţ����㣩
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
	int type;			/*=0 ˫���� (�Ϸ�˫�̵���)
						  =1 ������ (��һ���̵���)
						  ��������ʱontime��Ч
						*/		
	DWORD cfg;			//��Ϊ0
	DWORD hzontime; 	  /*ң�رպ�ʱ��,1ms��λ��Ĭ��2000*/
	short ybno; 		/*ѹ������: -2��ѹ��
									-1��ѹ��
							  >=0Ӳѹ��ң�ź�
						*/ 
	WORD pad;				  
	DWORD yztime;		/*ң��Ԥ�ó�ʱʱ��,1ms��λ,Ĭ��30000*/	
	DWORD fzontime;
	DWORD rsv[2];		
};	

struct VPYcCfg{
	int type;
	DWORD  cfg; 	//��Ϊ0
	
	int arg1;
	int arg2;
	short toZero;	  //��0ֵ��ȱʡΪ5

	WORD bak[5];	
	DWORD rsv[5];	
};


struct	VPSysConfig
{
	DWORD dwType;	  /*�豸�ͺ�(���ֽڣ� ��ģ�����ɣ�����Ա����*/
	DWORD dwName;	  /*�豸��ʶ��ϵͳ��Ψһ��ȡ(dwType&0xFFF)+���豸��ϵͳ����Ŀ,����ʾ*/
	char  sByName[GEN_NAME_LEN];   /*�豸����,�豸����ʱ�û���ά������*/
	DWORD dwCfg;	 /*D0=1 �����Զ��
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

struct VNetIP{
	char sIP[20];
	DWORD dwMask;
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

#pragma pack()

WORD GetParaCrc16(const BYTE *p, const int l);
void CreateRunParaConfig(void);

int WriteParaBmFile(const char * filename,BYTE * buf,int len);
int SysParaInit(void);
int ReadParaFile(char *filename, BYTE *buf, int buf_len);
int WriteParaFile(const char *filename, struct VFileHead *buf);
int CreateSysConfig(char *path);
void WriteFzT(WORD fztime);
void WriteHzT(WORD hztime);
void WriteYxFdT(WORD yxfdtime);
void WriteULP(WORD svalue);
void WriteILP(WORD svalue);
void WritePLP(WORD svalue);
void WriteFLP(WORD svalue);
void WriteZLLP(WORD svalue);
void WriteCosLP(WORD svalue);

#ifdef __cplusplus
}
#endif

#endif

