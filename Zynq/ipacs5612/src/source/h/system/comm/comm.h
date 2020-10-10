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

#define TX_EMPTY_NUM    5    /*发送空字符数*/
#define RX_AVAIL_NUM    5    /*接收可用字符数*/
#define FRAME_INTVAL    5    /*分帧间隔*/

/*-----------------------CommCtrl命令和参数定义-------------------------*/
/*命令格式: 高位为主命令, 低位为子命令*/
#define CCC_BAUD_CTRL	0x0100
    #define BAUD_SET        0x01
	#define BAUD_GET        0x02
#define CCC_MODEM_CTRL	0x0200
	#define MODEM_IN		0x01
	#define MODEM_OUT		0x02
	#define MODEM_OUT_READ	0x04
	#define MODEM_RTS_ON	0x08   

	#define MODEM_CTS	 	0x01	/*参数:管脚状态*/
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
	
#define CCC_TID_CTRL	0x0700		/*参数:任务号*/
#define CCC_EVENT_CTRL	0x0800		/*规约需要事项控制*/
	#define TX_IDLE_ON		0x01	/*参数:空闲触发时间[ticks]*/
	#define TX_IDLE_OFF		0x02
	#define COMM_IDLE_ON	0x04	/*参数:空闲触发时间[ticks]*/
	#define COMM_IDLE_OFF	0x08

#define CCC_CONNECT_CTRL  0x0900    /*端口连接控制*/
    #define CONNECT_STATUS_GET	     0x01	/*取得连接状态*/
    #define CONNECT_OPEN		     0x02	/*打开连接*/	
    #define CONNECT_CLOSE     	     0x04   /*关闭连接*/

#define CCC_WR_CTRL     0x0A00      /*无线通信控制*/
	#define WR_GETDCD				 1   /*取得连接成功信号*/
    #define WR_ENTERCMD              2   /*进入命令行模式*/
	#define WR_EXITCMD               3   /*退出命令行模式*/
    #define WR_SIM_GET               4   /*取得SIM卡号*/
	#define WR_SIGN                  5   /*取得信号强度*/
	#define WR_FLUX_GET              6   /*取得数据流量*/
    #define WR_FLUX_RESET            7	 /*数据流量清零*/
	#define WR_MSGNO_SET             8   /*短消息中心设置*/
	#define WR_MSG_INIT              9   /*短消息初始化相关*/
	#define WR_MSG_QUEUE             10   /*新短消息查询处理*/
	#define WR_MSG_SEND              11  /*发送短消息*/
	#define WR_CALL_HAUP             12  /*挂断电话*/
	#define WR_LOGHT_ON              13  /*点亮GPRS指示灯*/
	#define WR_LOGHT_OFF 			 14  /*熄灭GPRS指示灯*/
	
#define CCC_LAN           0x0B00
    #define LAN_MODE_SET             0x01   /*模式设置: 0:TCP-SERVER 1:TCP-CLIENT 2:UDP*/
	#define LAN_IP_SET               0x02
	#define LAN_IP_GET				 0x03
	#define LAN_MAC_SET              0x04

#define CCC_CFG_STR       0x0C00             /*端口配置串*/
	#define CFG_STR_SET				 0x01
	#define CFG_STR_GET		         0x02

#define CCC_CHIP_CTRL	  0x0D00
    #define CHIP_ON		    0x01
    #define CHIP_OFF		0x02

/*循环缓冲区*/
typedef struct {
    BYTE	*buf;			/*缓冲区地址*/
	WORD	size;			/*缓冲区大小*/
	WORD	rp;				/*读偏移*/
	WORD	wp;				/*写偏移*/
}VCommBuf;

/*逻辑通道相关*/
typedef struct {	
	VCommBuf rx_buf;
	VCommBuf tx_buf;

    char cfgstr[3][30];  /*配置串*/
	
	int (*read) (int no, BYTE* pbuf, int buflen, DWORD flags);
	int (*write)(int no, BYTE* pbuf, int buflen, DWORD flags);
	int (*ctrl) (int no, WORD command, BYTE *para);	

	WORD rx_wp;	

	DWORD rx_idle;			/*接收空闲时间*/
	DWORD tx_idle;			/*发送空闲时间*/
	
	BYTE  comm_idle_ev;		/*通讯空闲事项允许*/
	DWORD  comm_idle_limit;	/*通讯空闲时限*/

	BYTE  tx_idle_ev;		/*发送空闲事项允许*/
	DWORD  tx_idle_limit;	/*发送空闲时限*/

	//DWORD ev_send;

    int phy_no;
	int fd;

	int tid;		    	/*上层任务id*/
	int tid_watchdog;
}VCommChan;

#define COMM_DATABUF_LEN   8192  /*pow 2*/
#define COMM_WRITEFILE_LINE  2048    /*后面改大*/
#define COMM_WRITEFILE_LEN   8192
#define COMM_WRITELINE_LEN   96  /*每一行长度*/

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


