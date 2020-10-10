/*------------------------------------------------------------------------
 Module:		tcp.h
 Author:		solar
 Project:		
 State: 		
 Creation Date: 2008-08-15
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef _NET_H
#define _NET_H

#include "syscfg.h"
#include "os.h"
#include "comm.h"


#define  NET_BUF_NUM          2048


#define  CLIENTSTACKSIZE      (1024*16)
#define  SERVERSTACKSIZE      (1024*16)

#define  CONNECTINTERVAL      30   /*30s for client reconnet interval*/						   

#define TCP_STATE_OPEN 	      1
#define TCP_STATE_CLOSE	      2
						    
typedef struct {  
    int no;

	int nSamePortminor;

	int nSocket;
    int nListenSocket;

    int mode;            /*0-NONE 缺省
                           1-网络服务器
						   2-网络客户端
						 */
	int anyip;					 
    VLanIP lan[3];

	int anymac;
	VLanMac mac[3];

	int status;
}VTcpLogChan;

/*------------------------------------------------------------------------
							   函数声明
------------------------------------------------------------------------*/
void Client(void);
void Server(void);
void Udp_Server(void);
int TcpInit (void);
void ScanSocket(void);

int TcpRead (int no, BYTE* pbuf, int buflen, DWORD flags);
int TcpWrite (int no, const BYTE* pbuf, int buflen, DWORD flags);
int TcpCtrl (int no, WORD command, DWORD *para);

int TcpAccept(WORD port, int listen_num, struct timeval *tmval, char *ip);

#endif /*_Net_H*/

