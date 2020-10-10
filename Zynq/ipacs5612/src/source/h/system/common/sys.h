/*------------------------------------------------------------------------
 Module:        sys.h
 Author:        solar
 Creation Date: 2008-08-01
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 12 $
------------------------------------------------------------------------*/

#ifndef _SYS_H
#define _SYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syscfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include "os.h"
#include "os_linux.h"
#include "comm.h"
#include "clock.h"
#include "db.h"
#include "file.h"
#include "para.h"
#include "myio.h"



#define PR_VYX_NUM         32
#define MAX_GZYX_NUM  10
#define MAX_GZYC_NUM 10
#define MAX_GZEVT_NUM  256

#define BM_EQP_MYIO             0
#define BM_EQP_LINE1            1
#define BM_EQP_LINE2            2
#define BM_EQP_SOCFA            3


typedef struct TDBGzEvent
{
	WORD fd;
	WORD YxNum;
	WORD YxNo[MAX_GZYX_NUM];
	WORD YxValue[MAX_GZYX_NUM];
	WORD YcNum;
	WORD YcNo[MAX_GZYC_NUM];
	DWORD YcValue[MAX_GZYC_NUM]; //�ȵ�ѹ�����
	struct VCalClock Time[MAX_GZYX_NUM];
}VDBGzEvent;

typedef struct TPrGzEvtOpt
{
	VDBGzEvent evt[MAX_GZEVT_NUM];
	WORD wReadPtr;
	WORD wWritePtr;
}VPrGzEvtOpt;







#define  MyFatalErr()        for(;;)

/*D0-D15 ϵͳ����*/
#define  SYS_ERR_SYS         0x00000001   /*ϵͳ����,����Դ��ȱ,�����쳣�˳���*/
#define  SYS_ERR_ADDR        0x00000002   /*��ַ�г�ͻ*/
#define  SYS_ERR_CFG         0x00000004   /*���������Ӳ�����ߴ���*/
#define  SYS_ERR_OBJ         0x00000008   /*��̬�����*/
#define  SYS_ERR_COMM        0x00000010   /*ͨ�Ŵ���*/
#define  SYS_ERR_GAIN        0x00000020   /*���������ļ�����*/
#define  SYS_ERR_LOGIN       0x00000040   /*�忨�ͺ����ϼ�������һ��,��¼ʧ��*/
#define  SYS_ERR_SUBCFG      0x00000080   /*�¼���������*/
#define  SYS_ERR_AD          0x00000100   /*AD�쳣�澯*/
#define  SYS_ERR_AIO         0x00000200   /*ģ����쳣�澯*/
#define  SYS_ERR_DIO         0x00000400   /*���ְ��쳣�澯*/
#define  SYS_ERR_SLT         0x00000800   /*SLT �澯*/
#define  SYS_ERR_PR          0x00080000   /*�����澯*/
#define  SYS_ERR_FILE        0x00100000   /*�ļ�ϵͳ�澯*/
#define  SYS_ERR_SRAM        0x00200000   /*sram�澯*/
#define  SYS_ERR_FAT         0x00400000   /*sram�澯*/
#define  SYS_ERR_JIAMI      0x00800000  /*����оƬû�����쳣*/
#define  SYS_ERR_TMP        0x01000000 /*������Ԫ�¶�оƬû�����쳣*/

/*Cfg Attr,�˶�����װ����Ͷ����й���,��syscfg.h*/
#define  CFG_ATTR_NULL       0x0000       /*�޶���,��port.cfg,device.cfg�е�attr*/
#define  CFG_ATTR_IO         0x0001       /*����I/O��Ӧ��cfg����*/
#define  CFG_ATTR_USEDEF     0x0002       /*�Զ����豸��Ӧ��cfg����*/
#define  CFG_ATTR_TABLE      0x0003       /*���ͱ��豸��Ӧ��cfg����*/
#define  CFG_ATTR_GROUP      0x0004       /*�豸���Ӧ��cfg����*/
#define  CFG_ATTR_DYQC       0x0005       /*��ѹ�޹������ݴ���ģ��*/
#define  CFG_ATTR_FA         0x0006       /*FAʵ�豸*/
#define  CFG_ATTR_IPACS      0x1000       /*IPACSϵ�ж�Ӧ��cfg����*/

#if (TYPE_OS == OS_LINUX)
#define  COMMONSTACKSIZE   	 (1024*96)
#define  COMMONPRIORITY    	 (USR_TASK_PRIO+15)

#define  MAXPROTOCOLNUM      15
#endif

#define  COMMONQLEN       	 5
#define  COMMONDOGTM    	 3000

#define  NULL_ID               0
#define  WATCHDOG_ID           1
#define  CLOCK_ID              1
#define  SHELL_ID              2
#define  COMM_ID               3
#define  COMM_SERVER_ID        4
#define  COMM_CLIENT_ID        5
#define  GPRS_ID               6
#define  DB_ID                 7
#define  FADIFF_ID             8
#define  SELF_DIO_ID           9
#define  SELF_MEA_ID           10
#define  PR_ID                 11
#define  COMTRADE_ID           12
#define  B2F_ID                13
#define  MISC_ID               14
#define  MMI_ID                15
#define  MYGOOSE_RX_ID         16
#define  MYGOOSE_TX_ID         17
#define  FA_ID                 18
#define  FA_CLIENT_ID          19
#define  FA_SERVER_ID          20
#define  WEBSERVER_ID          21
#define  WIFI_ID               22
#define  FRZ_ID                22
#define  WEBSERVICE_ID         23
#define  SELF_YX_ID            23
#define  SNTP_ID               24
#define  WEBACT_ID             25
#define  PRLOW_ID              26
#define  RTISR_ID              27
#define  COMM_UDP_SERVER_ID 28
#define  HIS_ID                29
#define  XML_ID                30



#define  BUS_CAN_ID            COMM_CAN_STARTNO
#define  MAINT_ID              COMM_NET_START_NO
#ifdef INCLUDE_WIFI
#define  WIFI_MAINT_ID         COMM_WIFI_STARTNO
#endif
#ifdef INCLUDE_PB_RELATE
#define PB_ID                  32
#endif

#if ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_WDG))
#undef  MMI_ID
#define MMI_ID                COMM_SERIAL_START_NO
//wdj 
//#undef  SHELL_ID
//#define SHELL_ID             (COMM_SERIAL_START_NO+1)                 
#endif

#define DIFFPRO0_ID  (COMM_SERIAL_START_NO + 2)
#define DIFFPRO1_ID  (COMM_SERIAL_START_NO + 3)

#define LINEPRO0_ID  (COMM_SERIAL_START_NO + 2)
#define LINEPRO1_ID  (COMM_SERIAL_START_NO + 3)

#define  DRAM  0
#define  NVRAM 1

#define  COLDRESTART  0x12345678 
#define  WARMRESTART  0x5555AAAA

#define  TRUEEQPDSOENUM      32
#define  TRUEEQPDCOSNUM      32
#ifdef _TEST_VER_
#define  TRUEEQPSSOENUM      2048
#define  TRUEEQPSCOSNUM      1024
#else
#define  TRUEEQPSSOENUM      256
#define  TRUEEQPSCOSNUM      64
#endif
#define  VIRTUALEQPDSOENUM   32
#define  VIRTUALEQPDCOSNUM   32
#ifdef  _TEST_VER_
#define  VIRTUALEQPSSOENUM   4096
#define  VIRTUALEQPSCOSNUM   4096
#else
#define  VIRTUALEQPSSOENUM   256
#define  VIRTUALEQPSCOSNUM   64
#endif
#define  VIRTUALEQPCONUM     64

#define  FAPROCNUM           32
#define  SYSEVENTNUM         32
#define  SYSULOGNUM         32
#define  SYSFLOWNUM         32
struct VThSetupInfo{
    int thid;
    char tName[GEN_NAME_LEN];
	int nPriority;
	int nStackSize;
	int nQLen;
	ENTRYPTR entryPtr;
};	
  
struct VProtocol{
	char name[GEN_NAME_LEN];
	char entryName[GEN_NAME_LEN];
	ENTRYPTR entry;
};  

struct VSysCfgExt{
	char sWsName[GEN_NAME_LEN];
	char sWsSechema[GEN_NAMELONG_LEN];
	DWORD dWsIp;
	DWORD dWsVer;

	char  sMaintIp[20];
	BYTE  byMac[6];

	DWORD dwKgTime;

	VDiCfg YxIn[8];

	char xmlVer[16];
	char xmlRev[16];
	DWORD xmlCrc;
	short YXNO[12];
	struct VNetIP Lan3;
    char sGateWay3[20];
	char sRoute3[20];
	struct VNetIP Lan4;
    char sGateWay4[20];
	char sRoute4[20];
	BYTE YKDelayTime;
};

#define IONO_BUF_NUM  3
struct VIoNo{
	WORD wIoNo_Low;
	WORD wIoNo_High;
	WORD wNum;
};
	
struct  VMyConfig{
	DWORD dwType;        /*�豸�ͺ�*/
	DWORD dwName;        /*�豸��ʶ���ƣ�ϵͳ��Ψһ����̬�������ʾ*/
	char  sByName[GEN_NAME_LEN];   /*�豸����,�û�������̬���������޸�*/
	DWORD dwCfg;        			/*D0=1 �����Զ��
	                 							   D1=1 sntp��ʱ
	                 							   D2 =1 ��ң��1��ֵ�ϴ�ά��ң�����
	                 							   D3 = 1,�����汾,��Լ������Ϣ��չ
	                 				 			   D3 = 0����ǰ�汾��Լ��Ϣû��չ��������ѹ����û��	
                                                   D4 = 1�����ѹС�źţ�D4 = 0�����ѹ�����  
	                 				 			*/

	DWORD dwAIType;
	DWORD dwIOType;
	BYTE  byCPLDType;
	BYTE  byYcNum[BSP_AC_BOARD];
	BYTE  byYxNum[BSP_IO_BOARD];
	BYTE  byYkNum[BSP_IO_BOARD];

	WORD  wAllIoFDNum;

	WORD  wAllIoAINum;
	WORD  wAllIoDINum;
	WORD  wAllIoDONum;
	
	WORD  wAllIoYCNum;
	WORD  wAllIoDYXNum;
	WORD  wAllIoSYXNum;
	WORD  wAllIoDDNum;
	WORD  wAllIoYKNum;	
	WORD  wAllIoYTNum;

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

    WORD  wYCNumPublic;
	WORD  wYCNumAdd;
	//0-di, 1-di_public, 2-softyx_public+softyx_fd
    struct VIoNo SYXIoNo[IONO_BUF_NUM];
	//0-do, 1-do_public, 2-softyk
    struct VIoNo YKIoNo[IONO_BUF_NUM];

#ifdef INCLUDE_CELL
	VCellCfg CellCfg;
#endif
#ifdef INCLUDE_USWITCH
	VUSwitchCfg USwitchCfg;
#endif
   VRunParaCfg RunParaCfg; 
    DWORD dSntpServer;

	VFdCfg *pFd;
	VFdCfg *pFaPrFd;

	VAiCfg *pAi;
    VDiCfg *pDi;
	VDoCfg *pDo;
	
	VYcCfg *pYc;

	struct VSysCfgExt SysExt;

};

#define COMM_TYPE_NONE     0
#define COMM_TYPE_SERVER   1   /*1-TCP��������*/
#define COMM_TYPE_CLIENT   2   /*2-TCP�ͻ���*/
#define COMM_TYPE_UDP      3
#define COMM_TYPE_RS232    4

struct VPort{
	int id;
	
	char pcol[GEN_NAME_LEN];	
	int pcol_attr;           /*0---master
	                           1---slave
	                         */                               
	char cfgstr[3][30];

	int bakmode;
	int bakcomm;
	
	int load;
};

struct VComm{
	struct VPort Port;

	void *pvProtocolCfg;  
};

#define YCQUOTIETYENABLED  0x01
#define DDQUOTIETYENABLED  0x02

struct VTECfg{
	WORD wSourceAddr;  /*������ַ*/
	WORD wDesAddr;     /*װ�õ�ַ*/
	char sExtAddr[12];
	char sTelNo[20];   /*�ֻ�����*/

	DWORD dwFlag;  /*D0:ң���ϵ��
	                 D1:��ȳ�ϵ��
	               */  

	WORD wYCNum;
	WORD wVYCNum;
	WORD wDYXNum;
	WORD wSYXNum;
	WORD wVYXNum;   /*����ң�Ÿ���*/
	WORD wDDNum;
	WORD wYKNum;
	WORD wYTNum;
	WORD wTSDataNum;
	WORD wTQNum;

	BYTE byYCFrzMD;   //ң�ⶳ���ܶ� 1-60min Ĭ��15min
	BYTE byYCFrzCP;   //ң�ⶳ������ 1-180day Ĭ��30day

	BYTE byDDFrzMD;   //��ȶ����ܶ� 5-60min Ĭ��60min
	BYTE byDDFrzCP;   //��ȶ������� 1-180day Ĭ��30day	

	WORD wInvRefEGNum;  /*���ô�װ�õ�װ������Ŀ*/ 
	WORD wYCMaxMinNum;
};

struct VTrueDYX{
	BYTE byValue1;  /*D0=1 ��Ч =0 ��Ч
        			  D1=1 ������ =0 δ������
        			  D2=1 ��ȡ�� =0 δ��ȡ��
					  D3=1 �ǵ�ǰֵ =0 ��ǰֵ
                      D4-D6 ����
					  D7=1 �� =0 ��  
                    */
	BYTE byValue2;
};

struct VTrueYX{
	BYTE byValue;   /*D0=1 ��Ч =0 ��Ч
        			  D1=1 ������ =0 δ������
        			  D2=1 ��ȡ�� =0 δ��ȡ��
					  D3=1 �ǵ�ǰֵ =0 ��ǰֵ
                      D4-D6 ����
					  D7=1 �� =0 ��  
                    */
};

struct VMaxMinYC{
	VYC lMax;
	struct VCalClock max_tm;
	VYC lMin;
	struct VCalClock min_tm;	
};	

struct VTrueYC{
	VYC lValue;
	BYTE byFlag;    /*D0=1 ��Ч =0 ��Ч
        			  D1=1 ������ =0 δ������
        			  D2=1 ��ȡ�� =0 δ��ȡ��
					  D3=1 �ǵ�ǰֵ =0 ��ǰֵ
                      D4=1 ��� =0 δ���
                      D5   ����
                      D6=1 �����ʽ =0 ����    
                      D7=1 �������ͽ�ֹ =0 ����
                    */
	BYTE byRsv;
};

struct VDataRef{
	WORD wEqpID;
	WORD wNo;
};

struct VTrueCfg{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;  /*D0=1 ����(ȱʡ) =0 ������
                    D30��1 �ǵ�ǰֵ =0����(ȱʡ)
					D31��1 ������ =0����(ȱʡ)
	              */	              
	WORD wSendNo;    /*0xFFFF Ϊ�����ͣ���Ч*/
	WORD wVENum;     /*���ô������ݵ���װ�ø���*/
	struct VDataRef *pInvRef;  /*���ô������ݵ���װ����Ϣ*/
};

struct VTrueYCCfg{
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
	WORD wSendNo;  /*0xFFFF Ϊ�����ͣ���Ч*/
	WORD wVENum;  /*���ô������ݵ���װ�ø���*/
	struct VDataRef *pInvRef;  /*���ô������ݵ���װ����Ϣ*/
	long lA;  /*ϵ��*/    
	long lB;  /*����ֵ*/    
	long lC;  /*����ֵ      ������� vlaue*lA/lB+lC*/
	WORD wFrzKey;     

	WORD wHourFrzNo;   /*0xFFFF Ϊ�޶��ᣬ��Ч*/
	WORD wDayFrzNo;   /*0xFFFF Ϊ�޶��ᣬ��Ч*/
	WORD wMonthFrzNo;  /*0xFFFF Ϊ�޶��ᣬ��Ч*/
	WORD wMaxMinNo;    /*0xFFFF Ϊ�޼�ֵ����Ч*/
	
	long lLimit1;	    //��ֵ1
	DWORD dwLimitT1;	//��ֵ1ʱ��ms
	WORD  wLimit1VYxNo;  //��ֵ1��Ӧ��ң�ź�
	long lLimit2;	    //��ֵ2
	DWORD dwLimitT2;	//��ֵ2ʱ��ms
	WORD  wLimit2VYxNo;	//��ֵ2��Ӧ��ң�ź�

	BYTE byL1Flag;
	BYTE byL2Flag;

	DWORD dwL1Cnt;
	DWORD dwL2Cnt;

	BYTE tyctype;
	char tycunit[5];
	BYTE tycfdnum;
};

struct VTrueYXCfg{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;  /*D0=1 ����(ȱʡ) =0 ������
	    D1=1 ȡ�� =0 ��ȡ��(ȱʡ)
	    D2=1 ���ݿ����SOE  =0 ������(ȱʡ)
	    D3=1 ���ݿ����COS  =0 ������(ȱʡ)
	    D4=1 ���������ź�  =0 ������(ȱʡ)

		D29��1 ��ȡ�� =0����(ȱʡ)
		D30��1 �ǵ�ǰֵ =0����(ȱʡ)
		D31��1 ���� =0����(ȱʡ)
	    */
	WORD wSendNo;  /*0xFFFF Ϊ�����ͣ���Ч*/
	WORD wVENum;  /*���ô������ݵ���װ�ø���*/
	struct VDataRef *pInvRef;  /*���ô������ݵ���װ����Ϣ*/
};

struct VTrueDDCfg{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;   /*D0=1 ����(ȱʡ) =0 ������
	                D1=1 ���� =0 ����������(ȱʡ)
	                D2=1 �ն��� =0���ն���(ȱʡ)
	                D3=1 �¶��� =0���¶���(ȱʡ)	  

					D29��1 ��ȡ�� =0����(ȱʡ)
					D30��1 �ǵ�ǰֵ =0����(ȱʡ)
					D31��1 ���� =0����(ȱʡ)
	               */ 
	WORD wSendNo;  /*0xFFFF Ϊ�����ͣ���Ч*/
	WORD wVENum;  /*���ô������ݵ���װ�ø���*/
	struct VDataRef *pInvRef;  /*���ô������ݵ���װ����Ϣ*/
	long lA;  /*ϵ��    ȱʡ1*/
	long lB;  /*����ֵ    ȱʡ1*/
	long lC;  /*ԭʼֵ    ȱʡ0*/
	WORD wFrzKey;
	WORD wHourFrzNo;  	/*0xFFFF Ϊ�޶��ᣬ��Ч*/
	WORD wDayFrzNo;   /*0xFFFF Ϊ�޶��ᣬ��Ч*/
	WORD wMonthFrzNo;  /*0xFFFF Ϊ�޶��ᣬ��Ч*/
};

struct VTrueCtrlCfg{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;
	BYTE byFlag;  /* D0=1 ��Ч =0 ��Ч
	      D1=1 ������ =0 δ������
	      D2=1 ��ȡ�� =0 δ��ȡ��
	      D3=1 ����״̬ =0����̬
	      D4-D6 ����
	      D7=1 ���ܿ� =0 û�ܿ�*/ 
	DWORD dwValue;   /*Դ����ֵ*/
	WORD byTaskID;    /*ԴTaskID*/
	WORD wEqpID;   /*Դ����װ�úţ�����Ϊ����*/
	WORD wID;    /*Դ������ţ�����Ϊ����*/
	struct VCalClock Time;  /*���һ���ܿ�ʱ��  ���ڶ��ܿصĳ�ʱ���� ��ʱ����???*/ 

	WORD wSimYxTEqpID;  /*��ң�ض�Ӧ�ķ���ң�ŵ�ʵװ�ú�*/
	WORD wSimYxNo;      /*��ң�ض�Ӧ�ķ���ң�ŵĵ��*/
};

struct VTSDataCfg{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;  /*D0=1 ת��Ϊ����ң�� =0��ת��(ȱʡ)
	     D1=1 ת��Ϊ�� =0 ת��Ϊ��*/
	WORD wKeyWordNum;
	BYTE abyKeyWord[14];
	WORD awOffset[14];
	WORD wVYXNo; /*��Ӧ��ң�ŵ��*/
};

struct VTagCfg{
	WORD wNum;
	WORD *pwIndex;
};


struct VPool{
	WORD wWritePtr;
	WORD wNum;  
	void *pHead;
	WORD wCellLen;
};

struct VDSOE{
	WORD wWritePtr;
	WORD wPoolNum;
	struct VDBDSOE *pPoolHead;	
	WORD wCellLen;
};

struct VDCOS{
	WORD wWritePtr;
	WORD wPoolNum;
	struct VDBDCOS *pPoolHead;	
	WORD wCellLen;
};

struct VSOE{
	WORD wWritePtr;
	WORD wPoolNum;
	struct VDBSOE *pPoolHead;	
	WORD wCellLen;
};

struct VCOS{
	WORD wWritePtr;
	WORD wPoolNum;
	struct VDBCOS *pPoolHead;	
	WORD wCellLen;
};

struct VCO{
	WORD wWritePtr;
	WORD wPoolNum;
	struct VDBCO *pPoolHead;	
	WORD wCellLen;
};

struct VFLOW{
	WORD wWritePtr;
	WORD wPoolNum;
	struct VDBFLOW *pPoolHead;	
	WORD wCellLen;
};

struct VTrueDD{
	long lValue;
	BYTE  byFlag;  /* D0=1 һλС����
	                D1=1 ��λС����
	                D2=1 ��λС����
	                D3=1 ��λС����                    
	                D4-D6 ����
	                D7=1 ʱ����Ч D7=0 ʱ����Ч*/
	BYTE byRsv;
					
	struct VCalClock Time;
};


struct VTrueCtrl{
	WORD wID;
};

struct VTrueEqp{
	struct VTECfg Cfg;

	WORD *pwInvRefEGID; /*���ô�װ�õ�װ����ID��*/

	struct VTrueYC *pYC;
	struct VMaxMinYC *pMaxMinYC; /*����NVRam*/
	struct VTrueYCCfg *pYCCfg;
	struct VTagCfg YCSendCfg;
	struct VTagCfg YCHourFrz;
	struct VTagCfg YCDayFrz;
	struct VTagCfg YCMonthFrz;
	
	struct VTrueDYX *pDYX;   /*����NVRam*/
	struct VTrueYXCfg *pDYXCfg;
	struct VTagCfg DYXSendCfg;

	struct VDSOE DSOE;   /*����NVRam*/	
	struct VDCOS DCOS;   /*����NVRam*/

	struct VTrueYX *pSYX;     /*����NVRam*/
	struct VTrueYXCfg *pSYXCfg;
	struct VTagCfg SYXSendCfg;

	struct VSOE SSOE;   /*����NVRam*/
	struct VCOS SCOS;   /*����NVRam*/

	struct VTrueDD *pDD;
	struct VTrueDDCfg *pDDCfg;  
	struct VTagCfg DDSendCfg;
	struct VTagCfg DDHourFrz;
	struct VTagCfg DDDayFrz;
	struct VTagCfg DDMonthFrz;

	struct VTrueCtrl *pYK;
	struct VTrueCtrlCfg *pYKCfg;

	struct VTSDataCfg *pTSDataCfg;

	struct VTrueCtrl *pYT;
	struct VTrueCtrlCfg *pYTCfg;  
};

struct VVECfg{	
	WORD wSourceAddr;  /*������ַ*/
	WORD wDesAddr;     /*װ�õ�ַ*/
	char sExtAddr[12];
	char sTelNo[20];   /*�ֻ�����*/

	DWORD dwFlag;      /* D0: ң���ϵ��
										D5 ��������xml�ļ�
	                      D1-D31:����*/ 

	WORD wYCNum;
	WORD wDYXNum;
	WORD wSYXNum;
	WORD wDDNum;
	WORD wYKNum;
	WORD wYTNum;
	WORD wTQNum;

	WORD wInvRefEGNum;  /*���ô�װ�õ�װ������Ŀ*/
};

struct VVirtual{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;     
	WORD wTEID;      /*���õ�ʵװ�õ�ID*/
	WORD wOffset;    /*��ʵװ���е�ƫ��*/
	WORD wSendNo;
	WORD wStartNo;   /*��ͬ�����������װ���е���ʼ��*/
	WORD wNum;       /*��ͬ������Ÿ���*/
};

struct VVirtualYC{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;     /*D0-D7 ����
                       D9=1 �������ͽ�ֹ =0������������(Ĭ��)
                       D11=1�������ߡ���ֵ��=0 ���������߼�ֵ
	                 */
	WORD wTEID;      /*���õ�ʵװ�õ�ID*/
	WORD wOffset;    /*��ʵװ���е�ƫ��*/
	WORD wSendNo;
	WORD wStartNo;   /*��ͬ�����������װ���е���ʼ��*/
	WORD wNum;       /*��ͬ������Ÿ���*/
	short lA;
	short lB;
	short lC;
	
	WORD wHourFrzNo;  	/*0xFFFF Ϊ�޶��ᣬ��Ч*/
};

struct VVirtualYX{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;     /*D0=1  ��ͬ������������
                       =0  ��ͬ������Ż����*/
	WORD wTEID;      /*���õ�ʵװ�õ�ID*/
	WORD wOffset;    /*��ʵװ���е�ƫ��*/	
	WORD wSendNo;
	WORD wStartNo;   /*��ͬ�����������װ���е���ʼ��*/
	WORD wNum;       /*��ͬ������Ÿ���*/
};

struct VVirtualDD{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;         /*D0=1  �����ȣ��ܼ�ʱ������
	                     =0  �����ȣ��ܼ�ʱ���ӣ�
	                     D1=1 ����(ȱʡ) =0 ����������(ȱʡ)   	                     
	                     D2-D15 ����*/
	WORD wTEID;        /*���õ�ʵװ�õ�ID*/
	WORD wOffset;      /*��ʵװ���е�ƫ��*/	
	WORD wSendNo;
	WORD wStartNo;     /*��ͬ�����������װ���е���ʼ��*/
	WORD wNum;         /*��ͬ������Ÿ���*/

	WORD wHourFrzNo;	/*0xFFFF Ϊ�޶��ᣬ��Ч*/	
};

struct VVirtualCtrl{
	char sName[GEN_NAME_LEN];
	char sNameTab[GEN_NAMETAB_LEN];
	DWORD dwCfg;      /*����*/
	WORD wTEID;       /*���õ�ʵװ�õ�ID*/
	WORD wOffset;     /*��ʵװ���е�ƫ��*/

	WORD wID;
};

struct VVirtualEqp{
	struct VVECfg Cfg;

	WORD *pwInvRefEGID; /*���ô�װ�õ�װ����ID��*/

	struct VVirtualYC  *pYC;
	struct VTagCfg YCHourFrz;

	struct VVirtualYX  *pDYX;
	struct VDSOE DSOE;   /*����NVRam*/
	struct VDCOS DCOS;   /*����NVRam*/

	struct VVirtualYX  *pSYX;
	struct VSOE SSOE;   /*����NVRam*/
	struct VCOS SCOS;   /*����NVRam*/
	
	struct VCO pCO;
	
	struct VVirtualDD  *pDD;
	struct VTagCfg DDHourFrz;

	struct VVirtualCtrl  *pYK;
	struct VVirtualCtrl  *pYT;
};

struct VEGCfg{	
	WORD wSourceAddr;  /*������ַ*/
	WORD wDesAddr;     /*�ϼ�װ�õ�ַ*/

	DWORD dwFlag;      /*D0: =0ת��FA(Ĭ��) =1 ��ת��
	                     D1: =0ת��QcEvent(Ĭ��) =1��ת��
	                     D2: =0ת��ActEvent(Ĭ��) =1��ת��
	                     D3: =0ת��DoEvent(Ĭ��) =1��ת��
	                     D4: =0ת��WarnEvent(Ĭ��) =1��ת��
	                     D5-D31:����*/               
	WORD wEqpNum;
}; 

struct VEqpGroup{
	struct VEGCfg Cfg;

	WORD *pwEqpID;
};


#define COMMBREAK  0x0
#define COMMOK     0x1

struct VCommRunInfo{
	WORD wState;
	DWORD dwRxNum;
	DWORD dwRxBadNum;
	DWORD dwTxNum;  
	DWORD dwBreakNum;
	DWORD dwTimeOutNum;
};  

#define DSOEUFLAG           0x00000001
#define DCOSUFLAG           0x00000002
#define SSOEUFLAG           0x00000004
#define SCOSUFLAG           0x00000008
#define FAINFOUFLAG         0x00000010
#define ACTEVENTUFLAG       0x00000020
#define DOEVENTUFLAG        0x00000040
#define WARNEVENTUFLAG      0x00000080
#define FAEVENTUFLAG        0x00000100

struct VEqpRunInfo{
	WORD wEqpID;
	DWORD dwEqpName;
	DWORD dwUFlag;
	WORD wDSOEReadPtr;
	WORD wDCOSReadPtr;
	WORD wSSOEReadPtr;
	WORD wSCOSReadPtr;
	WORD wFAInfoReadPtr;
	WORD wActEventReadPtr;
	WORD wDoEventReadPtr;
	WORD wWarnEventReadPtr;	
	struct VCommRunInfo CommRunInfo;
};

#define TRUEEQP       0x00
#define VIRTUALEQP    0x01
#define GROUPEQP      0x02

struct VTVEqpInfo{   /*ʵװ������װ�ù��õ��豸��Ϣ*/
	WORD wType;        /*TRUEEQP  or  VIRTUALEQP*/

	WORD wSourceAddr;  /*������ַ*/
	WORD wDesAddr;     /*װ�õ�ַ*/
	char sExtAddr[12];
	char sTelNo[20];   /*�ֻ�����*/

	DWORD dwFlag;  /*D0-1: ����    (D0:��������D1: ң������)
	               D2:ң���ϵ��
	               D3:��ȳ�ϵ��
	               D4-D31:����*/

	WORD wYCNum;
	WORD wVYCNum;
	WORD wDYXNum;
	WORD wSYXNum;
	WORD wVYXNum;   /*����ң�Ÿ���*/
	WORD wDDNum;
	WORD wYKNum;
	WORD wYTNum;
	WORD wTSDataNum;
	WORD wTQNum;
	struct VEqpRunInfo *pEqpRunInfo;
};  

  
struct VEqpGroupInfo{
	WORD wType;        /*GROUPEQP*/

	WORD wSourceAddr;  /*������ַ*/
	WORD wDesAddr;     /*�ϼ�װ�õ�ַ*/
	DWORD dwFlag; /*����*/

	WORD wEqpNum;
	struct VEqpRunInfo *pEqpRunInfo;

	WORD *pwEqpID;
};
  
struct VTask{
	struct VComm CommCfg; 

	WORD wEqpGroupNum;
	WORD wGroupEqpNum;
	struct VEqpRunInfo *pEqpGroupRunInfo;
	struct VEqpRunInfo *pGroupEqpRunInfo;  
	WORD *pwGroupEqpIDList;

	WORD wEqpNum;
	struct VEqpRunInfo *pEqpRunInfo;
	WORD *pwEqpIDList;

	void *pCache;
}; 

struct VEqpInfo{
	WORD wID;
	DWORD dwName;
	char sCfgName[GEN_NAME_LEN];
	WORD wCommID;

	struct VTrueEqp *pTEqpInfo;
	struct VVirtualEqp *pVEqpInfo;

	struct VEqpGroup  *pEqpGInfo;

	WORD wTaskID; 
}; 
 
struct VEqp{
	WORD *pwNum;
	struct VEqpInfo *pInfo;
};  

struct VMemBlock{
	WORD  wFlag;
	WORD  wID;
	DWORD dwSize;
}; 

/*struct VNVRamInfo{
	PART_ID PartId;
	BYTE *pbyPtr;
	WORD  wBlkID;
};*/ 

struct VYKEvent{
	WORD wWritePtr;
	WORD wPoolNum;  
	struct VYKEventInfo *pPoolHead;	
	WORD wCellLen;
	DWORD dwSem;
};

#define  MAX_EXT_DEV_NUM    10

struct  VExtConfig{
	WORD wAddr;
	DWORD dwType;      

    WORD  wFDOffset;
	WORD  wFDNum;

    WORD  wAIOffset;
	WORD  wAINum;
	WORD  wDIOffset;
	WORD  wDINum;
	WORD  wDOOffset;
	WORD  wDONum;

	WORD  wYCOffset;
	WORD  wYCNum;
	WORD  wDYXOffset;
	WORD  wDYXNum;
	WORD  wSYXOffset;
	WORD  wSYXNum;
	WORD  wDDOffset;
	WORD  wDDNum;
	WORD  wYKOffset;
	WORD  wYKNum;	
	WORD  wYTOffset;
	WORD  wYTNum;
};

#define MAX_LD_FILE_NUM   (MAXPROTOCOLNUM+10)

struct VLdFileTbl{
    char name[GEN_NAME_LEN];       /*�ļ���Ӧģ������,��GB104*/
	char filename[GEN_NAME_LEN];   /*�ļ�����,��gb104.sx.out*/
	char user[GEN_NAME_LEN];
	char ver[GEN_NAME_LEN];
	DWORD crc;
	int load;       /*0-no load 1-load success 2-load fail*/
};	

enum {
	SYSEV_FLAG_NULL = 0,
	SYSEV_FLAG_FA,
	SYSEV_FLAG_ACT,
	SYSEV_FLAG_DO,
	SYSEV_FLAG_WARN,
	SYSEV_FLAG_DSOE,	
	SYSEV_FLAG_SSOE,
	SYSEV_FLAG_ASSERT,
#ifdef INCLUDE_B2F_COS
	SYSEV_FLAG_DCOS,
	SYSEV_FLAG_SCOS,
#endif
	SYSEV_FLAG_CO,
	SYSEV_FLAG_FLOW,
	SYSEV_FLAG_ULOG,
	SYSEV_FLAG_HL,
	SYSEV_FLAG_OL,
	MAX_SYSEV_FLAG_NUM
};	

enum {
	RI_FLAG_NULL = 0,
	RI_FLAG_EG,
	RI_FLAG_GE,
	RI_FLAG_EQP,
	MAX_RI_FLAG_NUM	
};	

struct VSysEventFlag{
	char sFName[MAXFILENAME];
	int nCellLen;
};	

#define SYS_RESET_POWER     0xAA550000
#define SYS_RESET_LOCAL     0xAA550001
#define SYS_RESET_REMOTE    0xAA550002

struct VSysResetInfo{
    DWORD code;
	char cause[GEN_NAME_LEN];
	struct VCalClock clock;
};	

#define RUNF_NOLOGIN         0x00000001
#define RUNF_DIF             0x00000002
#define RUNF_YXF             0x00000004
#define RUNF_DICOSF          0x00000008
#define RUNF_YXCOSF          0x00000010
#define RUNF_DISOEF          0x00000020
#define RUNF_YXSOEF          0x00000040

#define RUNF_CFGSYNC_MASK    0xFF000000
#define RUNF_CFGSYNC_DI      0x01000000
#define RUNF_CFGSYNC_FILE    0x02000000
#define RUNF_CFGSYNC_REFYX   0x04000000

struct VSysInfo{
	DWORD *pdwRestart;

	DWORD dwErrCode; 
	struct VSysResetInfo ResetInfo;

	//struct VNVRamInfo NVRamInfo;

	struct VMyConfig MyCfg;	

	struct VFAProc *pFAProc;

	struct VSysEvent *pActEvent;
	struct VSysEvent *pDoEvent;
	struct VSysEvent *pWarnEvent;
	struct VSysEvent *pUlogEvent;
	struct VSysFlowEvent *pFlowEvent;
	
	struct VSysEvent *pFaEvent;
    int LdFileNum;
	struct VLdFileTbl LdFileTbl[MAX_LD_FILE_NUM];

	struct VPAddr AddrInfo;

    WORD wIOEqpID;
	WORD wLinePr0EqpID;
	WORD wLinePr1EqpID;
	WORD wSocFaEqpID;

	DWORD dwAioType;
	DWORD dwDioType;
	BYTE  byCpldVer;
	
	struct VEqp Eqp;

	WORD  wExtIoNum;
#if (TYPE_IO == IO_TYPE_MASTER)
	struct VPExtIoAddrList *pExtIoAddrList;
	struct VExtIoConfig *pExtIoCfg;
	DWORD dwExtIoSyncSem;

	BYTE byIOCmdBuf[MAXMSGSIZE];
	DWORD dwExtIoCmdSem;
	DWORD dwExtIoDataSem;
#endif

#if (TYPE_IO == IO_TYPE_EXTIO)
    BYTE byIORcdBuf[MAXMSGSIZE];
    DWORD dwExtIoRcdSem;
	DWORD dwExtIoDataSem;
    DWORD dwRunFlag;
	struct VPExtIoAddr ExtIoAddr;
#endif	
	char InPara[SELFPARA_NUM][PARACHAR_MAXLEN];
	BYTE byDevType;// װ������00 ����30��01 ,����21 ��02����22��
};

extern struct VFileHead *g_pParafile;
extern DWORD g_ParafileSem;
extern DWORD g_fileSem;
extern struct VSysInfo g_Sys;  
extern struct VTask *g_Task; 

#ifdef INCLUDE_FRZ
#include "dbfrz.h"
#endif
#ifdef INCLUDE_HIS
#include "dbhis.h"
#endif

#ifdef INCLUDE_B2F
#include "b2f.h"
#endif
#ifdef INCLUDE_FA_PUB

#include "fadef.h" //add by wdj
#include "fapub.h"
#endif
#ifdef INCLUDE_PB_RELATE
#include "pb.h"
#endif
#ifdef INCLUDE_YC
#include "yc.h"
#endif
#ifdef INCLUDE_NET_GOOSE
#include "mygoose.h"
#endif

#define faultLight_turn(on_off)   \
		if (on_off) turnLight(BSP_LIGHT_WARN_ID, 1); \
		else turnLight(BSP_LIGHT_WARN_ID, 0)

/*system api*/

void BMSenderrCode(DWORD dwErrCode);
myinline void SysSetErr(DWORD errcode) 
{
    g_Sys.dwErrCode |= errcode;	 
	faultLight_turn(g_Sys.dwErrCode);
	BMSenderrCode(g_Sys.dwErrCode);
}

myinline void SysClearErr(DWORD errcode) 
{
    g_Sys.dwErrCode &= (~errcode); 
	faultLight_turn(g_Sys.dwErrCode);
	BMSenderrCode(g_Sys.dwErrCode);
}

#if (TYPE_IO == IO_TYPE_EXTIO)
myinline void SetRunFlag(DWORD runflag)
{
	taskLock();
	g_Sys.dwRunFlag |= runflag;	
	taskUnlock();
}

myinline void ClearRunFlag(DWORD runflag)
{
	taskLock();
    g_Sys.dwRunFlag &= (~runflag);
	taskUnlock();
}

myinline BOOL TestRunFlag(DWORD runflag)
{
    if (g_Sys.dwRunFlag & runflag) return(TRUE);
	else return(FALSE);
}
#endif

int fault(void);

void AddrInit(void);

void SpecialTaskSetup(struct VThSetupInfo* pthSetupInfo, int thid);
void CommNo2MaintNo(int portno, int *maint_portno, char *maint_portno_name_ch, char *maint_portno_name_en);

#if (TYPE_IO == IO_TYPE_MASTER)	
struct VPExtIoAddrList *FindOneExtIo(int addr);
int AddOneExtIo(struct VPExtIoAddr *pExtIoAddr);
#endif

/*Mem*/
void MemoryMalloc(void **ppPtr,DWORD dwSize,BYTE byMode);
void PoolMalloc(struct VPool *pPool,const WORD RelEleNum,const WORD PoolNum,const WORD CellNum);
void PoolReset(struct VPool *pPool);
#ifdef INCLUDE_B2F
void PoolMalloc_B2F(struct VPool *pPool,const WORD RelEleNum,const WORD PoolNum,const WORD CellNum, WORD wID, int flag);
void PoolReset_B2F(struct VPool *pPool, WORD wID, int flag);
void WritePollBuf2File(struct VPool *pPool, BYTE *pBuf, WORD wID, int flag);
#endif

#ifdef INCLUDE_HIS
void his2file_write(BYTE *pBuf, WORD wID, int flag);
#endif
#ifdef INCLUDE_HIS_TTU
void histtu2file_write(BYTE *pBuf, WORD wID, int flag);
#endif

/*DYX*/
void WriteDYX(WORD wEqpID,WORD wNum, struct VDBDYX *buf,BOOL bLock);
void WriteRangeDYX(WORD wEqpID,WORD wBeginNo,WORD wNum, struct VDYX *buf);
void WriteDCOS(WORD wEqpID,WORD wNum,struct VDBDCOS *buf);
void WriteDSOE(WORD wEqpID,WORD wNum,struct VDBDSOE *buf);
WORD ReadRangeDYX(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VDYX *buf);
WORD ReadDCOS(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VDBDCOS *buf);
WORD ReadDSOE(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VDBDSOE *buf);
WORD ReadRangeAllDYX(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VDYX *buf);
STATUS QueryDCOSWrtOffset(WORD wEqpID,WORD correctOffset,WORD *pwWriteOffset);
WORD QueryDCOS(WORD wEqpID,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VDBDCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
STATUS QueryDSOEWrtOffset(WORD wEqpID,WORD correctOffset,WORD *pwWriteOffset);
WORD QueryDSOE(WORD wEqpID,WORD wNum,WORD wReadOffset ,WORD wBufLen,struct VDBDSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen);

/*SYX*/
void atomicWriteSCOS(WORD wEqpID,WORD wNum,struct VDBCOS *buf,BOOL semflag,BOOL isStatusNo,BOOL bLock) ;
void atomicWriteSSOE(WORD wEqpID,WORD wNum,struct VDBSOE *buf,BOOL semflag,BOOL isStatusNo,BOOL bLock) ;
void SetRangeSYXCfg(WORD wEqpID,WORD wBeginNo,int nNum,DWORD dwCfg);
void ClearRangeSYXCfg(WORD wEqpID,WORD wBeginNo,int nNum,DWORD dwCfg);
void LockSYX(WORD wEqpID,WORD wNum, struct VDBYX *buf);
void UnLockSYX(WORD wEqpID,WORD wNum, struct VDBYX *buf);
void LockDD(WORD wEqpID,WORD wNum, struct VDBDDF *buf);
void UnLockDD(WORD wEqpID,WORD wNum, struct VDBDDF *buf);
void ClearSYX(WORD wEqpID);
void WriteSYX(WORD wEqpID,WORD wNum, struct VDBYX *buf);
void WriteRangeSYX(WORD wEqpID,WORD wBeginNo,WORD wNum, BYTE *buf);
void WriteRangeSYXBit(WORD wEqpID,WORD wBeginNo,WORD wNum, BYTE *buf);
void WriteSCOS(WORD wEqpID,WORD wNum,struct VDBCOS *buf);
void WriteSSOE(WORD wEqpID,WORD wNum,struct VDBSOE *buf);
WORD atomicReadSYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYX *buf,BOOL asSend);
WORD ReadSYX(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYX *buf);
WORD ReadRangeSYX(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf);
WORD ReadRangeSYXBit(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf);
WORD ReadSCOS(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VDBCOS *buf);
WORD ReadSSOE(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VDBSOE *buf);
WORD ReadRangeAllSYX(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf);
WORD ReadRangeAllSYXBit(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,BYTE *buf);
STATUS QuerySCOSWrtOffset(WORD wEqpID,WORD correctOffset,WORD *pwWriteOffset);
WORD QuerySCOS(WORD wEqpID,WORD wNum,WORD wReadOffset ,WORD wBufLen,struct VDBCOS *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
STATUS QuerySSOEWrtOffset(WORD wEqpID,WORD correctOffset,WORD *pwWriteOffset);
WORD QuerySSOE(WORD wEqpID,WORD wNum,WORD wReadOffset ,WORD wBufLen,struct VDBSOE *buf,WORD *pwWriteOffset,WORD *pwPoolLen);

/*YC*/
void SetRangeYCCfg(WORD wEqpID,WORD wBeginNo,int nNum,DWORD dwCfg);
void ClearRangeYCCfg(WORD wEqpID,WORD wBeginNo,int nNum,DWORD dwCfg);
void LockYC(WORD wEqpID, WORD wNum, struct VDBYCF_L *buf);
void UnLockYC(WORD wEqpID, WORD wNum, struct VDBYCF_L *buf);
void WriteYCF_F(WORD wEqpID, WORD wNum, struct VDBYCF_F *buf);
void WriteYC_F(WORD wEqpID, WORD wNum, struct VDBYC_F *buf);
void WriteRangeYCF_F(WORD wEqpID, WORD wBeginNo, WORD wNum, struct VYCF_F *buf);
void WriteRangeYC_F(WORD wEqpID, WORD wBeginNo, WORD wNum, float *buf);
void WriteYCF_L(WORD wEqpID,WORD wNum, struct VDBYCF_L *buf);
void WriteYC_L(WORD wEqpID,WORD wNum, struct VDBYC_L *buf);
void WriteYCCfg(WORD wEqpID,WORD wNo, WORD type, WORD fdnum,char*unit);
void WriteRangeYCF_L(WORD wEqpID,WORD wBeginNo,WORD wNum, struct VYCF_L *buf);
void WriteRangeYC_L(WORD wEqpID,WORD wBeginNo,WORD wNum, long *buf);
void WriteYCF(WORD wEqpID,WORD wNum, struct VDBYCF *buf);
void WriteYC(WORD wEqpID,WORD wNum, struct VDBYC *buf);
STATUS ReadSYXSendNo(WORD wEqpID, WORD wNo,WORD *wSendNo);
STATUS ReadDYXSendNo(WORD wEqpID, WORD wNo,WORD *wSendNo);
STATUS ReadYCSendNo(WORD wEqpID, WORD wNo,WORD *wSendNo);
STATUS ReadDDSendNo(WORD wEqpID, WORD wNo,WORD *wSendNo);

STATUS TEYcNoToVEYcNo(WORD wTEqpID, WORD wVEqpID, WORD wTENo,WORD *wVENo);

STATUS TEYxNoToVEYxNo(WORD wTEqpID, WORD wVEqpID, WORD wTENo,WORD *wVENo);

void WriteRangeYCF(WORD wEqpID,WORD wBeginNo,WORD wNum, struct VYCF *buf);
void WriteRangeYC(WORD wEqpID,WORD wBeginNo,WORD wNum, short *buf);
STATUS GetYC_ABC(WORD wEqpID, WORD wNo, long *A, long *B, long *C);
WORD ReadYCF_L(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYCF_L *buf);
WORD ReadYC_L(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYC_L *buf);
WORD ReadRangeYCF_L(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VYCF_L *buf);
WORD ReadRangeYC_L(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen, long *buf);
WORD ReadRangeAllYC_L(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen, long *buf);
WORD ReadRangeAllYCF_L(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VYCF_L *buf);
WORD ReadYCF(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYCF *buf);
WORD ReadYC(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBYC *buf);
STATUS ReadYCCfg(WORD wEqpID,WORD wNo,struct VYCF_L_Cfg* buf,BOOL asSend);
WORD ReadRangeYCF(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VYCF *buf);
WORD ReadRangeYC(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,short *buf);
WORD ReadRangeAllYC(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,short *buf);
WORD ReadRangeAllYCF(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VYCF *buf);

/*DD*/
void WriteDDF(WORD wEqpID,WORD wNum, struct VDBDDF *buf);
void WriteDDFT(WORD wEqpID,WORD wNum, struct VDBDDFT *buf);
void WriteDD(WORD wEqpID,WORD wNum, struct VDBDD *buf);
void WriteRangeDDF(WORD wEqpID,WORD wBeginNo,WORD wNum, struct VDDF *buf);
void WriteRangeDDFT(WORD wEqpID,WORD wBeginNo,WORD wNum, struct VDDFT *buf);
void WriteRangeDD(WORD wEqpID,WORD wBeginNo,WORD wNum, long *buf);
WORD ReadDDF(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDDF *buf);
WORD ReadDDFT(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDDFT *buf);
WORD ReadDD(WORD wEqpID,WORD wNum, WORD wBufLen,struct VDBDD *buf);
WORD ReadRangeDDF(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VDDF *buf);
WORD ReadRangeDDFT(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,struct VDDFT *buf);
WORD ReadRangeDD(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,long *buf);
WORD ReadRangeAllDD(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,long *buf);

/*YK*/
WORD ReadRangeAllYK(WORD wEqpID,WORD wBeginNo,WORD wNum, WORD wBufLen,WORD *buf);
	
/*TSData*/
void WriteTSData(WORD wEqpID,WORD wBufLen,BYTE *buf,struct VCalClock *pTime);

/*CommStatus*/
void WriteCommStatus(WORD wTaskID,WORD wEqpID,BOOL bStatus);
BOOL ReadCommStatus(WORD wTaskID,WORD wEqpID);

/*FA*/
void WriteFAInfo(WORD wNum,struct VFAProcInfo *buf);
WORD ReadFAInfo(WORD wTaskID,WORD wEqpID,WORD wNum,WORD wBufLen,struct VFAProcInfo *buf);
void QueryFAInfoWtrOffset(WORD *pwWriteOffset,WORD correctOffset);
WORD QueryFAInfo(WORD wTaskID,WORD wNum,WORD wReadOffset,WORD wBufLen,struct VFAProcInfo *buf,WORD *pwWriteOffset,WORD *pwPoolLen);

/*Systme Event*/
WORD GetSysEventMmiPtr(struct VSysEvent *pEvent);
void IncreaseSysEventMmiPtr(struct VSysEvent *pEvent, WORD wNum);
void WriteActEvent(struct VCalClock *ptm, DWORD type, int fd, const char *fmt, ... );
void WriteDoEvent(struct VCalClock *ptm, int para, const char *fmt, ... );
void WriteWarnEvent(struct VCalClock *ptm, DWORD errcode, int para, const char *fmt, ... );
void WriteFlowEvent(struct VCalClock *ptm, DWORD type, int value, struct VSysFlowEventInfo *pflow);
void WriteUlogEvent(struct VCalClock *ptm, DWORD type, int value, const char *fmt, ... );
WORD ReadActEvent(WORD wTaskID, WORD wEqpID, WORD wNum, WORD wBufLen, struct VSysEventInfo *buf);
WORD ReadDoEvent(WORD wTaskID, WORD wEqpID, WORD wNum, WORD wBufLen, struct VSysEventInfo *buf);
WORD ReadWarnEvent(WORD wTaskID, WORD wEqpID, WORD wNum, WORD wBufLen, struct VSysEventInfo *buf);
void QuerySysEventWtrOffset(struct VSysEvent *pEvent, WORD *pwWriteOffset,WORD correctOffset);
WORD QuerySysEventInfo(struct VSysEvent *pEvent,WORD wNum,WORD wReadOffset,WORD wBufLen, struct VSysEventInfo *buf,WORD *pwWriteOffset,WORD *pwPoolLen);
void SysEventReset(void);
void SysSoeReset(void);
void ClearSysEvent(struct VSysEvent *pEvent, int flag);

#ifdef INCLUDE_FA_PUB
WORD ReadFaEvent(WORD wTaskID, WORD wEqpID, WORD wNum, WORD wBufLen, struct VSysEventInfo *buf);
void WriteFaEvent(struct VCalClock *ptm, const char *fmt, ... );
#endif

/*EqpInfo*/
STATUS ReadEqpGEqpNum(WORD wEqpGID,WORD *pwNum);
STATUS ReadEqpInfo(WORD wTaskID,WORD wEqpID,void *pEqpInfo,BOOL asSend);

/*UFlag*/
BOOL TestFlag(WORD wTaskID,WORD wEqpID,DWORD dwUFlag);
void ClearFlag(WORD wTaskID,WORD wEqpID,DWORD dwUFlag);
void ReadPtrIncrease(WORD wTaskID,WORD wEqpID,WORD wNum,DWORD dwUFlag);
STATUS GetPoolPtr(WORD wTaskID,WORD wEqpID,DWORD dwUFlag,WORD *pwReadPtr,WORD *pwWritePtr);
void writeTEqpFlag(WORD wEqpID, DWORD dwFlag);

/*Systme Info*/
STATUS GetEqpID(DWORD dwName,WORD *pID);
STATUS GetEqpIDByCfgName(char *sCfgName, WORD *pID);
STATUS GetEqpIDByAddr(WORD wAddress,WORD *pID);
STATUS GetTaskID(const WORD wEqpID,WORD * pwTaskID);
struct VComm *GetCommCfg(const WORD wTaskID);
struct VTask *GetTaskInfo(const WORD wTaskID);
WORD GetTaskEqpNum(WORD wTaskID);
WORD GetTaskTVEqpNum(WORD wTaskID);
WORD GetTaskEqpGroupNum(WORD wTaskID);
WORD GetTaskEqpID(WORD wTaskID,WORD *pwEqpID);
WORD GetTaskTVEqpID(WORD wTaskID,WORD *pwEqpID);
WORD *GetTaskEqpIDList(WORD wTaskID);
WORD *GetTaskTVEqpIDList(WORD wTaskID);
void  SetupCommonTask(int thid,char *tName,int nPriority,int nStackSize,ENTRYPTR entryPtr,int nQLen);

/*Message  Info*/
void TaskSendMsg(WORD wToTaskID,WORD wMyTaskID,WORD wEqpID,BYTE byMsgID,BYTE byMsgAttr,WORD wDataLen,struct VMsg *pMessage);

/*Restart*/
void sysRestart(DWORD type, DWORD code, const char *cause);

/*Route Info*/
void UpRouteByEqpID(const WORD wEqpID,struct VMsg *pMsg);
void UpRoute(const WORD wAddr,struct VMsg *pMsg);
void DownRouteByEqpID(const WORD wEqpID,struct VMsg *pMsg);
void AbsDownRoute(const WORD wAddr,struct VMsg *pMsg);

/*wdg*/
void TaskDogMon(int interval);

/*Show*/
STATUS kgshow(WORD wTaskID);
STATUS devtypeshow(void);
STATUS vershow(void);

/*Ip*/
void GetAllFormatIP(char *normalformatIP, char *allformatIP);
void GetNormalFormatIP(char *allformatIP, char *normalformatIP);

/*float*/
long F24ToLong(BYTE *m_pbyData, short factor);
long F754ToLong(BYTE *m_pbyData, short factor);
void LongToFloat(int num, long ld, BYTE *fd);
void YCLongToFloat (WORD wEqpID,int num, long ld, BYTE *fd);
BYTE Gb101_SendFData(BYTE *pdec, BYTE type, struct VDBYCF_L*dd);

int ReadSelfPara(WORD parano,char* pbuf);
int ReadSelfParaGb2312(WORD parano,char* pbuf);

void SelfPara_Init(void);
void SelfPara_Init1(void);
void UpdateUserProgram(const char *fname) ;
int ReadRunPara(WORD parano,char* pbuf);
int WriteRunParaYZ(WORD parano,char* pbuf);
int WriteRunPara(WORD parano,char* pbuf);
int WriteRunParaFile(void);
int CheckParaType_Len(BYTE Type,BYTE Len);

void PublicInit(void);
void PubPollAndTableSetup(void);
void PortInfoInit(void);
void SysConfigInit(void);
STATUS GetDevInfo(void);
void DevInit(void);
int ProtocolInit(void);
void TaskSetup(void);
void ResetShow(void);
void NVRamInit(void);


//linux����˽�������
void SetIoNo(void);

int WriteSet(int fd, BYTE *pSet);
int ReadBMRangeYCF_L(WORD wEqpID,WORD no, long* value);
int ReadBMSYX(WORD wEqpID,WORD no, BYTE* value);
int ReadBMMyDiValue(WORD no,  WORD* value);
int ReadBMDYX(WORD wEqpID,WORD no, BYTE* value1,BYTE* value2);
int WriteYxTime_BM(WORD yxfdtime);
int WriteYkT_Bm(BYTE type,WORD time); 
int ReadBMSOE(WORD wEqpID,WORD *no, WORD *value, struct VCalClock *tm);
STATUS extNvRamGetBM(DWORD offset, BYTE *buf, int len);
STATUS extNvRamSetBM(DWORD offset, BYTE *buf, int len);
int Maint_BM(WORD dwCode,BYTE *buf,WORD len);
int Maint_BM_RCV(WORD dwCode,BYTE *sendbuf,WORD len,BYTE* recvbuf,WORD* recvlen);
int yk_BM(int type,int srcid,int id,int value,int time);
int	WriteBMZero(BYTE type,WORD svalue);
BYTE GetMyExtDiValue(int no);
int ReadPrPara(WORD parano,char *pbuf,WORD type);
int WritePrParaYZ(WORD parano,char *pbuf);
int WritePrPara(WORD parano,char *pbuf,WORD type);
int WritePrParaFile();
int BMWritePrPara(WORD parano,char *pbuf,WORD type,WORD len);
BOOL BM_PrReadGzEvent(BYTE* pbuf);
int prReadGzEvent(VDBGzEvent* pGzEvt);
void ResetProtect(int fd); 
void BMResetProtect(int fd); 
void ykTypeSave(int type, int id, int val);
void BmWriteRunPara(BYTE *buf,WORD len);
int ReadBmRecord(WORD* fd);
BYTE* ReadBmRecordData(WORD* smpfreq,WORD* smpnum, struct VCalClock* tm,struct VCalClock* prtm);
BOOL prGzEventInit();
int ReadBmEventFlag();
int ReadBmEvent(BYTE* buf, int len);
void WriteSysEvent(struct VSysEvent *pEvent, int flag, WORD wNum, struct VSysEventInfo *buf);
void WriteBmEvent();
void BMSetYkYb(DWORD Yb);
int ReadMyioType(DWORD* dwAIType ,DWORD* dwIOType);
void WriteEQPFromBM();
void SetBmUrgency(WORD bit);
#ifdef __cplusplus
}
#endif

#endif
