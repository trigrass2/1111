/*------------------------------------------------------------------------
 Module:		comm.h
 Author:		solar
 Project:		
 Creation Date: 2008-08-04
 Description:	
------------------------------------------------------------------------*/

#ifndef _COMM_H
#define _COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syscfg.h"
#include "comm_cfg.h"

#include "bsp.h"
#include "os.h"

#define COMM_SCAN_TM    2    //20ms

#define TX_EMPTY_NUM    5    /*���Ϳ��ַ���*/
#define RX_AVAIL_NUM    5    /*���տ����ַ���*/
#define FRAME_INTVAL    5    /*��֡���*/

/*-----------------------CommCtrl����Ͳ�������-------------------------*/
/*�����ʽ: ��λΪ������, ��λΪ������*/
#define CCC_BAUD_CTRL	0x0100
    #define BAUD_SET        0x01
	#define BAUD_GET        0x02
#define CCC_MODEM_CTRL	0x0200
	#define MODEM_IN		0x01
	#define MODEM_OUT		0x02
	#define MODEM_OUT_READ	0x04
	#define MODEM_RTS_ON	0x08   

	#define MODEM_CTS	 	0x01	/*����:�ܽ�״̬*/
	#define MODEM_DCD    	0x02
	#define MODEM_RTS		0x04
	#define MODEM_DTR		0x08
	#define MODEM_RI        0x10

#define CCC_DATA_BITS	0x0300	
#define CCC_STOP_BITS	0x0400
#define CCC_PARITY_SET	0x0500
	#define PARITY_SET_ODD		0x01
	#define PARITY_SET_EVEN		0x02
	#define PARITY_SET_NONE		0x03
	
#define CCC_INT_CTRL	0x0600
	#define RXINT_ON    	0x01
	#define RXINT_OFF  		0x02
	#define TXINT_ON    	0x04
	#define TXINT_OFF   	0x08
    #define INT_LOCK		0x10	
	#define INT_UNLOCK      0x20
	
#define CCC_TID_CTRL	0x0700		/*����:�����*/
#define CCC_EVENT_CTRL	0x0800		/*��Լ��Ҫ�������*/
	#define TX_IDLE_ON		0x01	/*����:���д���ʱ��[ticks]*/
	#define TX_IDLE_OFF		0x02
	#define COMM_IDLE_ON	0x04	/*����:���д���ʱ��[ticks]*/
	#define COMM_IDLE_OFF	0x08

#define CCC_CONNECT_CTRL  0x0900    /*�˿����ӿ���*/
    #define CONNECT_STATUS_GET	     0x01	/*ȡ������״̬*/
    #define CONNECT_OPEN		     0x02	/*������*/	
    #define CONNECT_CLOSE     	     0x04   /*�ر�����*/

#define CCC_WR_CTRL     0x0A00      /*����ͨ�ſ���*/
	#define WR_GETDCD				 1   /*ȡ�����ӳɹ��ź�*/
    #define WR_ENTERCMD              2   /*����������ģʽ*/
	#define WR_EXITCMD               3   /*�˳�������ģʽ*/
    #define WR_SIM_GET               4   /*ȡ��SIM����*/
	#define WR_SIGN                  5   /*ȡ���ź�ǿ��*/
	#define WR_FLUX_GET              6   /*ȡ����������*/
    #define WR_FLUX_RESET            7	 /*������������*/
	#define WR_MSGNO_SET             8   /*����Ϣ��������*/
	#define WR_MSG_INIT              9   /*����Ϣ��ʼ�����*/
	#define WR_MSG_QUEUE             10   /*�¶���Ϣ��ѯ����*/
	#define WR_MSG_SEND              11  /*���Ͷ���Ϣ*/
	#define WR_CALL_HAUP             12  /*�Ҷϵ绰*/
	#define WR_LOGHT_ON              13  /*����GPRSָʾ��*/
	#define WR_LOGHT_OFF 			 14  /*Ϩ��GPRSָʾ��*/
	
#define CCC_LAN           0x0B00
    #define LAN_MODE_SET             0x01   /*ģʽ����: 0:TCP-SERVER 1:TCP-CLIENT 2:UDP*/
	#define LAN_IP_SET               0x02
	#define LAN_IP_GET				 0x03
	#define LAN_MAC_SET              0x04

#define CCC_CFG_STR       0x0C00             /*�˿����ô�*/
	#define CFG_STR_SET				 0x01
	#define CFG_STR_GET		         0x02

#define CCC_CHIP_CTRL	  0x0D00
    #define CHIP_ON		    0x01
    #define CHIP_OFF		0x02

/*ѭ��������*/
typedef struct {
    BYTE	*buf;			/*��������ַ*/
	WORD	size;			/*��������С*/
	WORD	rp;				/*��ƫ��*/
	WORD	wp;				/*дƫ��*/
}VCommBuf;

/*�߼�ͨ�����*/
typedef struct {	
	VCommBuf rx_buf;
	VCommBuf tx_buf;

    char cfgstr[3][30];  /*���ô�*/
	
	int (*read) (int no, BYTE* pbuf, int buflen, DWORD flags);
	int (*write)(int no, BYTE* pbuf, int buflen, DWORD flags);
	int (*ctrl) (int no, WORD command, BYTE *para);	

	WORD rx_wp;	

	DWORD rx_idle;			/*���տ���ʱ��*/
	DWORD tx_idle;			/*���Ϳ���ʱ��*/
	
	BYTE  comm_idle_ev;		/*ͨѶ������������*/
	DWORD  comm_idle_limit;	/*ͨѶ����ʱ��*/

	BYTE  tx_idle_ev;		/*���Ϳ�����������*/
	DWORD  tx_idle_limit;	/*���Ϳ���ʱ��*/

	//DWORD ev_send;

    int phy_no;
	int fd;

	int tid;		    	/*�ϲ�����id*/
	int tid_watchdog;
}VCommChan;

#define COMM_DATABUF_LEN   8192  /*pow 2*/
#define COMM_WRITEFILE_LINE  2048    /*����Ĵ�*/
#define COMM_WRITEFILE_LEN   8192
#define COMM_WRITELINE_LEN   96  /*ÿһ�г���*/

struct VCommShow{
	int nCommNo;
	DWORD dwBufShow;
	//BYTE byKeyNum;
	//BYTE cKeyWord[5];

	int nMaintCommNo;
	WORD wReadPtr;
	WORD wWrtPrt;
	DWORD dwCount;
	DWORD dwSem;
	BYTE abyBuf[COMM_DATABUF_LEN]; 
};


struct VCommFileLine{
	char filename[64];
	DWORD wReadPtr;
	DWORD dwSize;
};

struct VCommFile{
	struct VCommFileLine* pCommFileLine;
	BYTE commbuf[COMM_WRITEFILE_LEN];
	DWORD dwSem;
};


extern VCommChan *g_CommChan;

typedef struct {
	char ip[20];
	WORD port;
}VLanIP;

typedef struct {
	BYTE mac[6];
}VLanMac;

#ifdef INCLUDE_SERIAL
    #include "serial.h"  
#endif
#ifdef INCLUDE_GPRS
    #include "gprs/gprs.h"  
#endif
#ifdef INCLUDE_NET
    #include "lan/tcp.h"  
#endif

int commRead (int no, BYTE* pbuf, int buflen, DWORD flags);
int commWrite(int no, BYTE* pbuf, int buflen, DWORD flags);
int commCtrl (int no, WORD command, BYTE *para );

int commMShowReq(int no);
int commBufQuery(int no, int len, char *buf);
int commMShowCancle(void);

int comshow(int no, int min);
int netshow(int no, int min);
int commshow(int no, int min);

int commInit (void);
int commRegister(int no, int phy_no, WORD bufsize, const void *comread, const void *comwrite, const void *comctrl);

void commBufFill(int no, BYTE flag, int len, BYTE *buf);
void commPrint(int no, BYTE flag, int len, BYTE *buf);
BYTE hexCharToBYTE(const char ch);
#ifdef __cplusplus
}
#endif

#endif


