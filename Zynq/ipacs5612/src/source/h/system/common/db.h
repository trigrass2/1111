/*------------------------------------------------------------------------
 Module:       	db.h
 Author:        solar
 Project:       
 State:         
 Creation Date:	2008-09-01
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 13 $
------------------------------------------------------------------------*/

#ifndef _DB_H
#define _DB_H

#ifdef __cplusplus
extern "C" {
#endif
#include "syscfg.h"
#include "clock.h"


#define YX_STATE(value)  (value&0x81)
#define YX_ISHE(value)   (YX_STATE(value)==0x81)
#define YX_ISFEN(value)  ((YX_STATE(value)==0x01)||(YX_STATE(value)==0x00))

#pragma pack(1)
struct VDBDYX{
  WORD wNo;
  BYTE byValue1;  
  BYTE byValue2;  
};

struct VDYX{
  BYTE byValue1;   
  BYTE byValue2;  
};

struct VDBDSOE{
  WORD wNo;
  BYTE byValue1;  
  BYTE byValue2;  
  struct VCalClock Time;
};

struct VDBDCOS{
  WORD wNo;
  BYTE byValue1;  
  BYTE byValue2;  
};

struct VDBYX{
  WORD wNo;
  BYTE byValue;     /*D0=1 ��Ч =0 ��Ч
        			  D1=1 ������ =0 δ������
        			  D2=1 ��ȡ�� =0 δ��ȡ��
					  D3=1 �ǵ�ǰֵ =0 ��ǰֵ
                      D4-D6 ����
					  D7=1 �� =0 ��  
                    */
};

struct  VDBSOE{
  WORD wNo;
  BYTE byValue;
  struct VCalClock Time;
};

struct VDBCO{
	WORD wNo;
	BYTE cmd;
	BYTE val;
	struct VCalClock Time;
};

struct VDBFLOW{
	WORD wNo;
	long val;
	struct VCalClock Time;
};

struct VDBCOS{
  WORD wNo;
  BYTE byValue;
};

struct VDBYCF_L{
  WORD wNo;
  long lValue;
  BYTE byFlag;      /*D0=1 ��Ч =0 ��Ч
        			  D1=1 ������ =0 δ������
        			  D2=1 ��ȡ�� =0 δ��ȡ��
					  D3=1 �ǵ�ǰֵ =0 ��ǰֵ
                      D4=1 ��� =0 δ���
                      D5   ����
                      D6=1 �����ʽ =0 ����    
                      D7=1 �������ͽ�ֹ =0 ����
                    */
};

struct VDBYCF_F{
  WORD wNo;
  float fValue;
  BYTE byFlag;      /*D0=1 ��Ч =0 ��Ч
        			  D1=1 ������ =0 δ������
        			  D2=1 ��ȡ�� =0 δ��ȡ��
					  D3=1 �ǵ�ǰֵ =0 ��ǰֵ
                      D4=1 ��� =0 δ���
                      D5   ����
                      D6=1 �����ʽ =0 ����    
                      D7=1 �������ͽ�ֹ =0 ����
                    */
};

struct VDBYCF{
  WORD wNo;
  short nValue;
  BYTE byFlag;      /*D0=1 ��Ч =0 ��Ч
        			  D1=1 ������ =0 δ������
        			  D2=1 ��ȡ�� =0 δ��ȡ��
					  D3=1 �ǵ�ǰֵ =0 ��ǰֵ
                      D4=1 ��� =0 δ���
                      D5   ����
                      D6=1 �����ʽ =0 ����    
                      D7=1 �������ͽ�ֹ =0 ����
                    */
};

struct VDBYC_F{
  WORD wNo;
  float fValue;  
};

struct VDBYC_L{
  WORD wNo;
  long lValue;  
};

struct VDBYC{
  WORD wNo;
  short nValue;  
};

typedef long VYC;

struct VYCF_F{
  float fValue;
  BYTE byFlag;      /*D0=1 ��Ч =0 ��Ч
        			  D1=1 ������ =0 δ������
        			  D2=1 ��ȡ�� =0 δ��ȡ��
					  D3=1 �ǵ�ǰֵ =0 ��ǰֵ
                      D4=1 ��� =0 δ���
                      D5   ����
                      D6=1 �����ʽ =0 ����    
                      D7=1 �������ͽ�ֹ =0 ����
                    */
};

struct VYCF_L{
  long lValue;
  BYTE byFlag;      /*D0=1 ��Ч =0 ��Ч
        			  D1=1 ������ =0 δ������
        			  D2=1 ��ȡ�� =0 δ��ȡ��
					  D3=1 �ǵ�ǰֵ =0 ��ǰֵ
                      D4=1 ��� =0 δ���
                      D5   ����
                      D6=1 �����ʽ =0 ����    
                      D7=1 �������ͽ�ֹ =0 ����
                    */
};
struct VYCF_L_Cfg{
  	BYTE tyctype;
	char tycunit[5];
	BYTE tycfdnum;
};

struct VYCF{
  short nValue;
  BYTE byFlag;      /*D0=1 ��Ч =0 ��Ч
        			  D1=1 ������ =0 δ������
        			  D2=1 ��ȡ�� =0 δ��ȡ��
					  D3=1 �ǵ�ǰֵ =0 ��ǰֵ
                      D4=1 ��� =0 δ���
                      D5   ����
                      D6=1 �����ʽ =0 ����    
                      D7=1 �������ͽ�ֹ =0 ����
                    */
};

struct VDBDDF{
  WORD wNo;
  long lValue;
  BYTE byFlag;  /* D0=1 һλС����
                           D1=1 ��λС����
                           D2=1 ��λС����
                           D3=1 ��λС����                    
                           D4-D6 ����
                           D7=1 ʱ����Ч D7=0 ʱ����Ч*/
};

struct VDBDD{
  WORD wNo;
  long lValue;
};

struct VDBDDFT{
  WORD wNo;
  long lValue;  
  BYTE byFlag;  /* D0=1 һλС����
                           D1=1 ��λС����
                           D2=1 ��λС����
                           D3=1 ��λС����                    
                           D4-D6 ����
                           D7=1 ʱ����Ч D7=0 ʱ����Ч*/
  struct VCalClock Time;
};

struct VDDF{
  long lValue;
  BYTE byFlag;    /* D0=1 һλС����
                             D1=1 ��λС����
                             D2=1 ��λС����
                             D3=1 ��λС����                    
                             D4-D6 ����
                             D7=1 ʱ����Ч D7=0 ʱ����Ч*/
};

struct VDDFT{
  long lValue;  
  BYTE byFlag;  /* D0=1 һλС����
                           D1=1 ��λС����
                           D2=1 ��λС����
                           D3=1 ��λС����                    
                           D4-D6 ����
                           D7=1 ʱ����Ч D7=0 ʱ����Ч*/
  struct VCalClock Time;
};


#define YK_INVALID 			0x00
#define YK_CLOSE            0x01
#define YK_OPEN             0x02
#define YK_NULL			    0x03
#define CTR_BYID            0x04
#define USEDEF_PLUSE        0x00
#define SHORT_PLUSE         0x10
#define LONG_PLUSE          0x20
#define STAND_PLUSE         0x30

#define DEF_YK_INVALID_VALUE (USEDEF_PLUSE|CTR_BYID|YK_INVALID)
#define DEF_YK_CLOSE_VALUE   (USEDEF_PLUSE|CTR_BYID|YK_CLOSE)
#define DEF_YK_OPEN_VALUE    (USEDEF_PLUSE|CTR_BYID|YK_OPEN)
#define DEF_YK_NULL_VALUE (USEDEF_PLUSE|CTR_BYID|YK_NULL)

#define CONTROLOK           0x00
#define CONTROLIDINVAILD    0x01
#define CONTROLIDDOING      0x02
#define CONTROLFAILED       0x03
#define CONTROLDISABLE      0x04
#define CONTROLUNKNOWERROR  0x05

struct VDBCtrl{
  WORD wID;       
  BYTE byStatus;  /*��ʾ������������Ľ��*/
};

struct VDBYK{
  WORD wID;       /*ң�غ�*/
  BYTE byStatus;  /*��ʾ������������Ľ��
                   0��  ����
                   ��0������

                   1����ŷǷ�
                   2���õ����ڱ�����
                   3������Ӳ��������
                   4�����ƽ�ֹ
                   5���������� */
  BYTE byValue;   /*ң������
                  D0~D1 
                   00 �Ƿ� 
                   01 ��բ�̵���
                   10 ��բ�̵���
                   11 NULL���������ֺ�/��բ�̵�����
                  D2��1���ƶ���Ϊ���غţ����п��غŲ��ң���ȱʡ�� 0�����ƶ���Ϊ˳���ţ�ƫ�ƣ�
                  D3������
                  D4-D7:  0���豸�Զ������壨ȱʡ��
                          1��������
                          2 ������
                          3 �������*/                 
};

struct VDBYT{
  WORD wID; 	  /*ң�غ�*/
  BYTE byStatus;  /*��ʾ������������Ľ��
				   0��	����
				   ��0������

				   1����ŷǷ�
				   2���õ����ڱ�����
				   3������Ӳ��������
				   4�����ƽ�ֹ
				   5���������� */
  DWORD dwValue;   /*ң��ֵ*/
};

struct VYKEventInfo{
  struct VCalClock Time;
  DWORD dwEqpName;
  WORD wID;
  BYTE byAttr;
  BYTE byRsv;
}; 

#define STDSWNUM 10

#define SWCOMMERR	0x05	/*�豸ͨѶ����*/
#define SWERROR 0x06	/*���ع���*/

struct VFaultSort
{
	BYTE DAType;	/*1��ʾ����ʽ���ƣ�2 ��ʾ�ֲ�ʽ���ƣ� */
	BYTE ReportType;	/*�������ͣ�1��ʾ���߹��ϴ����ģ�2��ʾ����������Ͷ���� ��4 ��ʾ���������ϴ����� ��8��ʾ����Ͷ�ָ����ģ�5 DA��ʽת������*/
	WORD ReportFun;	/*���Ĺ�����Ч��־��D1������ʶ��D2�����ϸ��� D3���ָ� ��1 ��ʾ��Ч��0��ʾ��Ч��*/
	WORD FailProc;	/*ʧ�������׶�*/
};

struct VFaultPos
{
	DWORD StartSW; /*����������ʼ���ر��*/
	DWORD EndSW; /*��������ĩ�˿��ر��*/
};

struct VYkResult
{
	DWORD SW;	/*���غ�,0��ʾ��Ч*/
	DWORD GZCSW; /*���Ϻ�������*/
	BYTE Phases;	/*��������=1 ң��Ԥ��=2ң��ִ��*/
	BYTE Status;	/*ң�ؽ��0:��ʾ�ɹ�*/	
};

struct VFAProcInfo
{
	struct VCalClock   Time ;
	struct VFaultSort  Sort ; 
	struct VFaultPos   Position ;
	struct VYkResult   Ioslation[STDSWNUM];
	struct VYkResult   Resume[STDSWNUM];
	BYTE Rsv[8];
}; 

struct VFAProc{
	WORD wWritePtr;
	WORD wPoolNum;
	struct VFAProcInfo *pPoolHead;	
	WORD wCellLen;
	DWORD dwSem;
};

struct VSysFlowEventInfo{
	WORD num;
	struct VDBFLOW dbflow[8]; //�ݶ���һ���ߵ�8��������
};

struct VSysFlowEvent{	
	WORD wWritePtr;
	WORD wPoolNum;  
	struct VSysFlowEventInfo *pPoolHead;	
	WORD wCellLen;
	DWORD dwSem;
	WORD wMmiPtr;        //only for mmi pop
};	



struct VSysEventInfo{
	struct VCalClock time;
	DWORD type;
	WORD rsv;
	WORD para;
	char msg[SYS_LOG_MSGLEN];
};

struct VSysEvent{	
	WORD wWritePtr;
	WORD wPoolNum;  
	struct VSysEventInfo *pPoolHead;	
	WORD wCellLen;
	DWORD dwSem;
	WORD wMmiPtr;        //only for mmi pop
};	

struct VAssertInfo{
	struct VCalClock time;
	DWORD rsv[2];
	char  msg[SYS_LOG_MSGLEN];
};

struct VAssert{
	WORD wWritePtr;
	WORD wPoolNum;
	struct VAssertInfo *pPoolHead;
	WORD wCellLen;
};
#pragma pack()

/*MSGID*/

/*MSGID*/
#define  MI_MASK         0xF0

/*YK*/
#define  MI_YK_MASK      0x10
#define  MI_YKSELECT     0x11
#define  MI_YKOPRATE     0x12
#define  MI_YKCANCEL     0x13
#define  MI_YKSIM        0x14
#define  MI_YKUNSIM      0x15

/*YT*/
#define  MI_YT_MASK      0x20
#define  MI_YTSELECT     0x21
#define  MI_YTOPRATE     0x22
#define  MI_YTCANCEL     0x23

/*TSDATA*/
#define  MI_ROUTE        0x30

/*DA����*/
//#define  MI_SIM		     0x40

/*EXTIO*/
#define  MI_EXTIO        0x50

/*PARA*/
#define  MI_PARA       0x60


/*MSGATTR*/
/*YK && YT*/
#define  MA_REQ          0x01
#define  MA_RET          0x02

/*TSDATA*/
#define  MA_TSDATA       0x01

/*DA����*/
/*#define  MA_SIM_ENTER    0x01
#define  MA_SIM_SET	     0x02
#define  MA_SIM_EXE	     0x03
#define  MA_SIM_EXIT     0x04
#define  MA_SIM_YK       0x05
#define  MA_SIM_PARA     0x06*/

/*EXTIO*/
#define  MA_EXTIO_CFGCRC      0x01
#define  MA_EXTIO_CFGUPLOAD   0x02
    
#define  MAXDATASIZE   	 2048
#define  MAXMSGSIZE    	 (MAXDATASIZE+sizeof(struct VMsgHead))

struct VMsgHead{
	WORD wLength;
	BYTE byMsgID;    /*D0~D6  MsgID
	      D7 �������У�1��ʾ�Է���Ӧ����ʱ���跢�ͽ�������
	       ��0��ʾ�Է�����Ӧ���Ļ��Ӧ����ʱ�跢�ͽ�������*/     
	BYTE byMsgAttr;
	WORD wThID;
	WORD wEqpID;
	BYTE byRsv[4];
};

struct VMsg{
	struct VMsgHead Head;
	BYTE abyData[MAXDATASIZE];
};
struct VFileMsgProM{
	WORD  pLen;
	BYTE  pMsgID;
	BYTE  flag;
	DWORD addr;
    DWORD filelen;
	BYTE msgData[MAXDATASIZE];
};

struct VYKMsg{
	struct VMsgHead Head;
	struct VDBYK Info;
};


#ifdef __cplusplus
}
#endif

#endif
