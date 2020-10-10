/*------------------------------------------------------------------------
 Module:		tcp.c
 Author:		solar
 Project:		
 State: 		
 Creation Date: 2008-08-15
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
 ------------------------------------------------------------------------*/

#include "syscfg.h"
#include "comm_cfg.h"

#ifdef INCLUDE_NET
#include "os.h"
#include "sys.h"
#include "tcp.h"
#include "bsp.h"
#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>     
#include <net/route.h>   


static VTcpLogChan *tcpChan;
static BYTE tempBuf[2*NET_BUF_NUM];
static fd_set read_fds, write_fds;
static DWORD tcp_sem;
#define closesocket(s)       close(s)

void inet_ntoa_b (struct  in_addr addr, char  * ip)
{
    unsigned int   addr_ip;


    addr_ip = (unsigned int)addr.s_addr;

	if (ip == NULL) return;

    sprintf(ip, "%u.%u.%u.%u", (addr_ip&0xFF000000)>>24, ((addr_ip&0xFF0000)>>16), ((addr_ip&0xFF00)>>8), (addr_ip&0xFF));
 
}
int connectWithTimeout (int sock_id, const struct sockaddr *paddr_remote, socklen_t addr_len, struct timeval  *ptimeout)
{
	fd_set writefds, readfds;

	int ret, error, errsize;

	ret = connect(sock_id, paddr_remote, addr_len);

	if ((ret == -1) &&(errno == EINPROGRESS))
	{
	    	FD_ZERO(&writefds);
		FD_ZERO(&readfds);
		FD_SET(sock_id, &writefds);
		FD_SET(sock_id, &readfds);
			
		if (select(sock_id+1, &readfds, &writefds, NULL, ptimeout) <= 0)
			return ERROR;
		else if (FD_ISSET(sock_id,&readfds)||FD_ISSET(sock_id,&writefds))//当连接建立遇到错误时，描述符变为即可读或可写，rc=2 遇到这种情况，可调用getsockopt函数获取socket状态
		{
			error = -1;
			errsize = sizeof(error);
			//检测进程是否阻塞在select中，有错误发生时，select将返回，并将发生错误的socket标记为可读写
			if (getsockopt(sock_id, SOL_SOCKET, SO_ERROR, (char*)&error, &errsize) < 0)
				return ERROR;
			else
			{
				if (error != 0)	//socket错误
					return ERROR;
				else			//socket正常
					return OK;
			}
		}
		
	}

	return ret;
}

int DelConnect(VTcpLogChan *ptcp);

/*------------------------------------------------------------------------
 Procedure: 	NetInit ID:1
 Purpose:		初始化网络通讯口
 Input: 						
 Output:		0:OK; -1:ERROR.
 Errors:
------------------------------------------------------------------------*/
int TcpInit (void)
{
    int log_no;
	unsigned int len;

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	tcp_sem = smMCreate();

	
    len = sizeof(VTcpLogChan)*((unsigned int)COMM_NET_NUM);
	tcpChan = (VTcpLogChan *)malloc(len);
	if (tcpChan == NULL) return ERROR;
	
	memset(tcpChan,0,len);
	
	for (log_no=0; log_no<COMM_NET_NUM; log_no++)
	{
		tcpChan[log_no].no = (COMM_NET_START_NO-COMM_START_NO)+log_no;
		tcpChan[log_no].nSocket = -1;
		tcpChan[log_no].nListenSocket = -1;
		tcpChan[log_no].nSamePortminor = -1;
		tcpChan[log_no].anyip = 1;
		tcpChan[log_no].anymac = 1;

		tcpChan[log_no].status = TCP_STATE_CLOSE;
	}

    for (log_no=0; log_no<COMM_NET_NUM; log_no++)
	{
	    tcpChan[log_no].no = commRegister(tcpChan[log_no].no, log_no, NET_BUF_NUM, (void *)TcpRead, (void *)TcpWrite, (void *)TcpCtrl);
	}	
	
	return OK;
}

int TcpRead (int no, BYTE* pbuf, int buflen, DWORD flags)
{
	int i,wp;	
	VCommBuf *pfifo;	

	pfifo = &(g_CommChan[no].rx_buf);

	/*int lock*/
    wp = pfifo->wp;
	/*int inlock*/

	for(i=0; i<buflen; i++)
	{
	    if(pfifo->rp == wp) break;		
        *(pbuf+i) = *(pfifo->buf+ pfifo->rp);
		pfifo->rp = (pfifo->rp+1)&(NET_BUF_NUM-1);
	}
	
	//g_CommChan[no].ev_send &= ~EV_RX_AVAIL;

	return i;
}

int TcpWrite (int no, const BYTE* pbuf, int buflen, DWORD flags)
{
	int i; WORD wp;
	VCommBuf *pfifo;
	int nSocket;

	g_CommChan[no].tx_idle = 0;

	if(buflen == 0) return 0;

	nSocket = tcpChan[g_CommChan[no].phy_no].nSocket;
	if (nSocket == -1) return 0;

	pfifo = &(g_CommChan[no].tx_buf);

	/*g_CommChan[no].ev_send &= ~EV_TX_IDLE;
	g_CommChan[no].ev_send &= ~EV_COMM_IDLE;*/

	/*int lock*/
	for(i=0; i<buflen; i++)
	{
		wp = (pfifo->wp+1)&(NET_BUF_NUM-1);
		if(pfifo->rp == wp) break;
		*(pfifo->buf+ pfifo->wp) = *(pbuf+i);
		pfifo->wp = wp;
	}
	/*int unlock*/

	/*g_CommChan[no].ev_send &= ~EV_TX_AVAIL;*/		

	FD_SET(nSocket, &write_fds);
	
	return i;
}

void TcpCfgStrSet(int log_no, const char *cfgstr)
{
    int i;
	char tmpstr[3][30], *str[3];

	//logMsg("log_no=%d\n", log_no,0,0,0,0,0);

	memcpy(tmpstr, cfgstr, sizeof(tmpstr));

	for (i=0; i<3; i++)
	{
		if (strcmp(tmpstr[i], "") == 0)  continue;

		GetStrFromMyFormatStr(tmpstr[i], ':', 3, &str[0], &str[1], &str[2]);
		if ((str[0] != NULL) && (tcpChan[log_no].mode == 0)) tcpChan[log_no].mode = atoi(str[0]);
		if (str[1] != NULL)
		{
			if ((*str[1] != '\0') && (strcmp(str[1], "0.0.0.0") != 0) && (strcmp(str[1], "0") != 0))
			{
				strcpy(tcpChan[log_no].lan[i].ip, str[1]);
				tcpChan[log_no].anyip = 0;
			}	
		}	
		if (str[2] != NULL) tcpChan[log_no].lan[i].port = (WORD)atoi(str[2]);		
	}	
}

int TcpCtrl (int no, WORD command, DWORD *para)
{
	int log_no;
	int ret = ERROR;

	/*for(log_no=0; log_no<COMM_NET_NUM; log_no++)
		if(tcpChan[log_no].no == no ) break;
		
	if(log_no == COMM_NET_NUM) return ERROR;*/

	log_no = g_CommChan[no].phy_no;

    switch(command & 0xFF00)
    {
		case CCC_CONNECT_CTRL:
			if(command & CONNECT_CLOSE)
			{	
				if (tcpChan[log_no].nSocket>=0)
				{
					DelConnect(&tcpChan[log_no]);		
					if (tcpChan[log_no].mode == COMM_TYPE_CLIENT)
						evSend(COMM_CLIENT_ID, EV_TM1);
				}	
				ret = OK;			
			}
			if (command & CONNECT_STATUS_GET)
			{
				if (tcpChan[log_no].status == TCP_STATE_OPEN)
					*para=1;
				else
					*para=0;
				ret = OK;
			}
			break;
		case CCC_LAN:
			if(command & LAN_MODE_SET)
			{	
				tcpChan[log_no].mode = (int)*para;
				ret = OK;			
			}
			else if (command & LAN_IP_SET)
			{
				memcpy((BYTE*)tcpChan[log_no].lan, (BYTE*)para, sizeof(VLanIP));
				ret = OK;
			}
			else if (command & LAN_IP_GET)
			{
				memcpy((BYTE*)para, (BYTE*)tcpChan[log_no].lan, sizeof(VLanIP));
				ret = OK;
			}
			else if (command & LAN_MAC_SET)
			{
			    tcpChan[log_no].anymac = 0;
				memcpy((BYTE*)tcpChan[log_no].mac, (BYTE*)para, sizeof(VLanMac));
				ret = OK;
			}
			break;
		case CCC_CFG_STR:			
			if (command & CFG_STR_SET) 
			{
				TcpCfgStrSet(log_no, (char *)para);			
				ret = OK;
			}	
			break;			
		default:
			return ERROR;			
	}
	
	return ret;
}

/*------------------------------------------------------------------------
 Procedure: 	GetCommfdset ID:1
 Purpose:		得到活动socket的fdset
 Input: 		
				
 Output:		0无效  >0最大Fd+1
 Errors:
------------------------------------------------------------------------*/
int GetCommfdset(fd_set *pfdset)
{
    int log_no,maxfd=-1;
	
	FD_ZERO(pfdset);
	
	for(log_no=0; log_no<COMM_NET_NUM; log_no++){
	  if (tcpChan[log_no].nSocket==-1) continue;
	
	  FD_SET(tcpChan[log_no].nSocket,pfdset);

	  if (tcpChan[log_no].nSocket>maxfd)  maxfd=tcpChan[log_no].nSocket;
	}

	return(maxfd+1);
}

/*------------------------------------------------------------------------
 Procedure: 	ResetMonir ID:1
 Purpose:		复位网络口状态
 Input: 		
				
 Output:		
 Errors:
------------------------------------------------------------------------*/
void ResetChan(const VTcpLogChan *ptcp)
{
    /*VNetBuf  *pbuf;*/
	VCommChan *pChan;

	pChan = g_CommChan + ptcp->no;
		 
	pChan->rx_idle = 0;			/*接收空闲时间*/
	pChan->tx_idle = 0;			/*发送空闲时间*/		
}

/*------------------------------------------------------------------------
 Procedure: 	DelConnect ID:1
 Purpose:		删除一个socket连接
 Input: 		
				
 Output:		
 Errors:
------------------------------------------------------------------------*/
int DelConnect(VTcpLogChan *ptcp)
{
    int nsocket;

	smMTake(tcp_sem);                 //该函数被多个任务调度,必须有信号量保护
	if (ptcp->nSocket < 0)
	{
	    smMGive(tcp_sem);
	    return 0;
	}

	nsocket = ptcp->nSocket;
	ptcp->nSocket = -1;
	closesocket(nsocket);

	FD_CLR(nsocket, &read_fds);
	FD_CLR(nsocket, &write_fds);	
	smMGive(tcp_sem);

	/*evSend*/	  /*app comm must reset when evReset*/
	ptcp->status = TCP_STATE_CLOSE;
	evSend(g_CommChan[ptcp->no].tid, EV_COMM_STATUS);
    //logMsg("Close %s!\n",ptcp->lan[0].ip,0,0,0,0,0);
	/*CommWrite(1,cEr,16,0);*/
	return(0);
}

/*------------------------------------------------------------------------
 Procedure: 	AddConnect ID:1
 Purpose:		增加一个socket连接
 Input: 		
				
 Output:		
 Errors:
------------------------------------------------------------------------*/
int AddConnect(VTcpLogChan *ptcp,int nSocket)
{	
	int optval;/*optvallen;*/
	
    /*test change para*/
    optval=1;
	if (setsockopt(nSocket,SOL_SOCKET,SO_KEEPALIVE,(char *)&optval,sizeof(optval))<0)
	{
		return(-1);
	}	
    optval=5;
	if(setsockopt(nSocket,IPPROTO_TCP,TCP_KEEPIDLE,(void *)&optval,sizeof(optval)) < 0)
    {
        return(-1);
	}
    optval = 3;
	if(setsockopt(nSocket,IPPROTO_TCP,TCP_KEEPINTVL,(void *)&optval,sizeof(optval)) < 0)
	{
	    return(-1);
	}
    optval = 2;
	if(setsockopt(nSocket,IPPROTO_TCP,TCP_KEEPCNT,(void *)&optval,sizeof(optval)) < 0)
	{
	    return(-1);
	}
    /*test???*/
	/*optval=1;
	if (setsockopt(nSocket,IPPROTO_TCP,TCP_NODELAY,(char *)&optval,sizeof(optval))<0)
	{
		return(-1);
	}*/

    /*optval=1;
    if (ioctl(nSocket, FIONBIO, optval)<0)
	{
		return(-1);
	}*/	

	optval = fcntl(nSocket, F_GETFL, 0);
	if(fcntl(nSocket, F_SETFL, optval|O_NONBLOCK) < 0)
	{
		return(-1);
	}
				
	/*getsockopt test????*/
	/*optval=0; optvallen=sizeof(optval);
	if (getsockopt(nSocket,SOL_SOCKET,SO_ERROR,(char*)&optval,&optvallen)<0)
	{
		return(-1);
	}	
	if (optval)	return(-1);*/
	if (ptcp->nSocket>=0) DelConnect(ptcp);   /*there have a evSend??*/
		
	ResetChan(ptcp);  /*???*/
	ptcp->nSocket = nSocket;
	FD_SET(nSocket,&read_fds);

	ptcp->status = TCP_STATE_OPEN;
	evSend(g_CommChan[ptcp->no].tid, EV_COMM_STATUS);
	/*CommWrite(1, cOK, 13, 0);*/
	return(0);
}

/*------------------------------------------------------------------------
 Procedure: 	NetProcErr ID:1
 Purpose:		socket错误处理
 Input: 		minor: 网络口号(偏移)
				
 Output:		
 Errors:
------------------------------------------------------------------------*/
void NetProcErr(int log_no)
{
	VTcpLogChan *ptcp;

	if ((errno==EWOULDBLOCK)||(errno==EINPROGRESS))   return;

	ptcp=tcpChan+log_no;	 

	if (ptcp->nSocket>=0)
	{	
	    DelConnect(ptcp);
     	/*reconnect*/	
    	if (ptcp->mode == COMM_TYPE_CLIENT)
	{
		  evSend(COMM_CLIENT_ID, EV_TM1);
		  /*CommWrite(1, cRetry, 7, 0);*/
    	}  
	}	
}

/*------------------------------------------------------------------------
 Procedure: 	ReadSocket ID:1
 Purpose:		接收socket数据到缓冲区
 Input: 		minor: 网络口号(偏移)
				
 Output:		
 Errors:
------------------------------------------------------------------------*/
void ReadSocket(int log_no)
{
	int no, nSocket, nRecNum, i;
	VCommBuf *pfifo;

	no = tcpChan[log_no].no;    
	
	nSocket = tcpChan[log_no].nSocket;
	if (nSocket == -1) return;

	nRecNum = recv(nSocket, (char *)tempBuf, sizeof(tempBuf), 0);

	
	if (nRecNum<=0)  NetProcErr(log_no);
	else
	{
		pfifo = &g_CommChan[tcpChan[log_no].no].rx_buf;
		/*taskLock();*/
		for (i=0; i<nRecNum; i++)
		{
			*(pfifo->buf+ pfifo->wp) = *(tempBuf+i);
			pfifo->wp = (pfifo->wp+1)&(NET_BUF_NUM-1);
		}	
		
		evSend(g_CommChan[no].tid, EV_RX_AVAIL);
		/*taskUnlock();*/		
	}
}

/*------------------------------------------------------------------------
 Procedure: 	WriteSocket ID:1
 Purpose:		将数据从缓冲区写到socket发送
 Input: 		minor: 网络口号(偏移)
				
 Output:		
 Errors:
------------------------------------------------------------------------*/
void WriteSocket(int log_no)
{
	VCommBuf *pfifo;
	int nSocket,nSendNum;
	WORD rp;
	DWORD countw;

	nSocket = tcpChan[log_no].nSocket;
	if (nSocket == -1) return;      /*this must have because read colese*/

	pfifo = &g_CommChan[tcpChan[log_no].no].tx_buf;
	countw = 0;

	/*taskLock();*/
	rp=pfifo->rp;	
	while(rp != pfifo->wp){
		*(tempBuf+countw) = *(pfifo->buf + rp);
		rp = (rp+1)&(NET_BUF_NUM-1);
		countw++;		
	}
	/*taskUnlock();*/
		
	if (countw==0)	
	{		
		FD_CLR(nSocket, &write_fds);
		return;
	}	

	nSendNum = send(nSocket ,(char *)tempBuf, countw ,0);

	/*logMsg("net sendnum=%d\n",nSendNum);*/

	if (nSendNum < 0)  NetProcErr(log_no); 	 
	else
	{
		/*taskLock();*/
		pfifo->rp = (pfifo->rp+nSendNum)&(NET_BUF_NUM-1);
		/*taskUnlock();*/
	}
}

void ScanSocket(void)
{
	int log_no,nSocket;
	fd_set readfds,writefds;
	struct timeval tmval;
		
    memcpy(&readfds, &read_fds, sizeof(read_fds));
    memcpy(&writefds, &write_fds, sizeof(write_fds));

    tmval.tv_sec = 0;
	tmval.tv_usec = 0;
    if (select(FD_SETSIZE, &readfds, &writefds, (fd_set*)0, &tmval)<=0) return;
    for(log_no=0; log_no<COMM_NET_NUM; log_no++){
	  	if ((nSocket=tcpChan[log_no].nSocket) == -1) continue;
		if (FD_ISSET(nSocket,&readfds))  ReadSocket(log_no);
		if (FD_ISSET(nSocket,&writefds))  WriteSocket(log_no);
    }	
}

/*------------------------------------------------------------------------
 Procedure: 	ConnectSetup ID:1
 Purpose:		客户端连接建立
 Input: 		
				
 Output:		
 Errors:
------------------------------------------------------------------------*/
void ConnectSetup(void)
{
	int nSocket,retry,log_no,ret,optval,i;
	/*int flaglen;*/
	struct sockaddr_in server;
	VTcpLogChan *ptcp;
	struct timeval tmval;	
	/*fd_set readfds,writefds;*/

	retry=0;
	for(log_no=0; log_no<COMM_NET_NUM; log_no++)
	{
		ptcp=tcpChan+log_no;
		if ((ptcp->no < 0) || (ptcp->mode != COMM_TYPE_CLIENT) || (ptcp->nSocket >= 0))
			 continue;

		for (i=0; i<3; i++)
		{
            		if (strcmp(ptcp->lan[i].ip, "") == 0) continue;
 
			if ((nSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
	    		{
		    		myprintf(COMM_ID, LOG_ATTR_ERROR, "netport%d kfatal-cannot create socket!",ptcp->no);
			    	exit(0);
			}  

			optval = fcntl(nSocket, F_GETFL, 0);
			if(fcntl(nSocket, F_SETFL, optval|O_NONBLOCK) < 0)
			{
				continue;
			}

			bzero((char *)&server,sizeof(server));
			server.sin_family = AF_INET;
			server.sin_port = htons(ptcp->lan[i].port);

			/*?? in for() ??*/
			tmval.tv_sec=3;   /*connnet timeout 3s*/
			tmval.tv_usec=0;		

			if (inet_aton(ptcp->lan[i].ip,&server.sin_addr)<0) 
		   	{
	 	    		closesocket(nSocket);
		   	    	continue;
			}

		    ret=connectWithTimeout(nSocket,(struct sockaddr *)&server, sizeof(server),&tmval);		   

		    if (ret==0)  /*connect success*/
		    {
			    if (AddConnect(ptcp,nSocket)<0)
				   closesocket(nSocket);
				else
					break;
	  		}
		    else   /*connect failed*/
			    closesocket(nSocket); 
		}
		if (i==3) retry = 1;
	}	

	if	(retry)  
	{
		thSleep(SECTOTICK(CONNECTINTERVAL));
		evSend(COMM_CLIENT_ID, EV_TM1);
	}		
}

/*------------------------------------------------------------------------
 Procedure: 	Client ID:1
 Purpose:		客户端
 Input: 		
				
 Output:		
 Errors:
------------------------------------------------------------------------*/
void Client(void)
{
    DWORD evFlag;

	evSend(COMM_CLIENT_ID, EV_TM1);
	
	for(;;)
	{
	  evReceive(COMM_CLIENT_ID,EV_TM1,&evFlag);
	
	  if (evFlag&EV_TM1)  ConnectSetup();
	}
}

STATUS CmpTcpCfg(int log_no1, int log_no2)
{
	VTcpLogChan *ptcp1, *ptcp2;
	int i,j;

	ptcp1 = tcpChan+log_no1;
	ptcp2 = tcpChan+log_no2;

    if ((ptcp1->anyip)&&(ptcp2->anyip))  return(ERROR);

    for (i=0; i<3; i++)
    {
		if (ptcp1->lan[i].ip[0] == '\0') continue;
		
		for (j=0; j<3; j++)
		{
			if (ptcp2->lan[j].ip[0] == '\0') continue;

            if (strcmp(ptcp1->lan[i].ip, ptcp2->lan[j].ip)==0)  return(ERROR);
		}
    }	   	

    return(OK);
}

/*------------------------------------------------------------------------
 Procedure: 	Server ID:1
 Purpose:		服务器端，侦听
 Input: 		
				
 Output:		
 Errors:
------------------------------------------------------------------------*/
void Server(void)
{
	int log_no,nSocket,sindex,smax,optval,listenflag,log_no1,log_no2,i,acceptflag,time_wait;
	DWORD peeraddrlen;
	struct sockaddr_in myaddr,peeraddr;
	struct in_addr peerin_addr;
	VTcpLogChan *ptcp;
	fd_set listenfds,readfds;
	char portname_ch[SYS_LOG_MSGLEN];
	char portname_en[SYS_LOG_MSGLEN];
	char msg[2*SYS_LOG_MSGLEN];
	

	for(log_no1=0; log_no1<COMM_NET_NUM; log_no1++)
	{
		if ((tcpChan[log_no1].no < 0) || (tcpChan[log_no1].mode != COMM_TYPE_SERVER))
			continue;
  
		for(log_no2=0; log_no2<log_no1; log_no2++)
		{
			if ((tcpChan[log_no2].no < 0) || (tcpChan[log_no2].mode != COMM_TYPE_SERVER))
				continue;

			if (tcpChan[log_no1].lan[0].port == tcpChan[log_no2].lan[0].port)
			{
				if (CmpTcpCfg(log_no1, log_no2) == ERROR)
				{
					tcpChan[log_no1].mode = COMM_TYPE_NONE;
					CommNo2MaintNo(COMM_NET_START_NO+log_no1, NULL, portname_ch, portname_en);
					sprintf(msg, "%s重复端口号", portname_ch);
					WriteWarnEvent(NULL, SYS_ERR_CFG, 0, msg);
					sprintf(msg, "warning: double portno at %s!", portname_en);		
					myprintf(SYS_ID, LOG_ATTR_WARN, msg);
				}	
				else
					tcpChan[log_no1].nSamePortminor = log_no2;
			}
		}  
	}	

	FD_ZERO(&listenfds);

	smax = -1;    
	listenflag=0;
	for(log_no=0; log_no<COMM_NET_NUM; log_no++)
	{
		ptcp = tcpChan+log_no;
		if ((ptcp->no < 0) || (ptcp->mode != COMM_TYPE_SERVER))
			continue;

 		if (ptcp->nSamePortminor == -1)
 		{
			if ((nSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
			{
		    	myprintf(COMM_ID,  LOG_ATTR_ERROR, "netport%d kfatal-cannot create socket!",ptcp->no);
				return;
			} 
			time_wait = 1;
			if (setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, &time_wait, sizeof(time_wait)) < 0)
				myprintf(COMM_ID,  LOG_ATTR_ERROR, "setsockopt TIME_WAIT fialed!");
		    		
		    bzero((char *)&myaddr,sizeof(myaddr));
			myaddr.sin_family = AF_INET;
			myaddr.sin_addr.s_addr=htonl(INADDR_ANY);
			myaddr.sin_port = htons(ptcp->lan[0].port);
			/* setsockopt SO_REUSEADDR*/
			/*optval=1;
			if (setsockopt(nSocket,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,sizeof(optval))<0)
			{
				myprintf(COMMID,"NetComm%d failed for setreuseraddr errno==%d",NETCOMMSTARTNO+minor,errno);
				close(nSocket);
				continue;
			}*/	

	        if (bind(nSocket, (struct sockaddr *)&myaddr,sizeof(myaddr))<0)
	        {
				myprintf(COMM_ID,  LOG_ATTR_ERROR, "NetComm%d failed for bind errno==%d", ptcp->no, errno);
				closesocket(nSocket);
				continue;
	        }
				
	        if (listen(nSocket,1)<0)
	        {
				myprintf(COMM_ID, LOG_ATTR_ERROR, "NetComm%d failed for listen errno==%d", ptcp->no, errno);
				closesocket(nSocket);
				continue;
			}	

			FD_SET(nSocket, &listenfds);
			ptcp->nListenSocket = nSocket;

			if (smax < nSocket)
				smax = nSocket;
 		}	
		else
			ptcp->nListenSocket = tcpChan[ptcp->nSamePortminor].nListenSocket;

		listenflag = 1;
	}	

    if (listenflag == 0)
    {
        myprintf(COMM_ID, LOG_ATTR_INFO, "there no servercfg and tServer exit."); 
		thSuspend(COMM_SERVER_ID);
    }
	
    smax += 1;   /*max socket bit*/
    for (;;)
    {
        memcpy(&readfds,&listenfds,sizeof(fd_set));
        if (select(smax,&readfds,(fd_set *)0,(fd_set *)0,(struct timeval *)0)<=0)
        /*if (select(FD_SETSIZE,&readfds,(struct fd_set *)0,(struct fd_set *)0,(struct timeval *)0)<=0)*/
			continue;
		for (sindex=0; sindex<smax; sindex++)
		{
            if (FD_ISSET(sindex, &readfds))		
            {
				/*set non-bolocking mode*/	
				

				optval = fcntl(sindex, F_GETFL, 0);
				if(fcntl(sindex, F_SETFL, optval|O_NONBLOCK) < 0)
					continue;
				/*optval=1;
				if (ioctl(sindex, FIONBIO, optval)<0)
					continue;*/
				peeraddrlen = sizeof(peeraddr);
                nSocket = accept(sindex,(struct sockaddr *)&peeraddr,(socklen_t*)&peeraddrlen);
				if (nSocket < 0)
				{	
                	/*myprintf(COMM_ID,"accept errno=%d",errno);*/
					continue;
				}  

				acceptflag = 0;
				for(log_no=0; log_no<COMM_NET_NUM; log_no++)
				{
					ptcp = tcpChan+log_no;
					if	((ptcp->nListenSocket == sindex) && (ptcp->anyip == 0))
					{
						for (i=0; i<3; i++)
						{
							if (inet_aton(ptcp->lan[i].ip, &peerin_addr)<0)
								continue;
							if (peeraddr.sin_addr.s_addr == peerin_addr.s_addr)
							/*if (peeraddr.sin_addr.s_addr == inet_addr(ptcp->lan[i].ip))*/
							{
							    if (ptcp->anymac)
							    {
							       acceptflag = 1;
								   break;
							    }
							}	
						}
						if (acceptflag) break;
					}	
				}

				if (acceptflag == 0)
				{
					for(log_no=0; log_no<COMM_NET_NUM; log_no++)
					{
						ptcp = tcpChan+log_no;					
						if	((ptcp->nListenSocket == sindex) && (ptcp->anyip) &&(ptcp->anymac))
						{
							acceptflag=1;
							break;
						}	
					}	
				}	

				if (acceptflag)
				{
					//CommNo2MaintNo(COMM_NET_START_NO+log_no, NULL, portname_ch, portname_en);
					if (AddConnect(tcpChan+log_no, nSocket)<0)  closesocket(nSocket);
				}
				else
					closesocket(nSocket);
			}
		}	
    }
}

int TcpAccept(WORD port, int listen_num, struct timeval *tmval, char *ip)
{
	int nSocket, newSocket, rc, smax;
	DWORD peeraddrlen;
	struct sockaddr_in myaddr,peeraddr;
	fd_set readfds;

	newSocket = -1;
	if ((nSocket = socket(AF_INET, SOCK_STREAM,0)) < 0)
		goto Ret;
    		
    bzero((char *)&myaddr,sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(port);

    if (bind(nSocket, (struct sockaddr *)&myaddr, sizeof(myaddr))<0)
		goto Ret;
		
	if (listen(nSocket, listen_num) < 0)
		goto Ret;

	FD_ZERO(&readfds);
	FD_SET(nSocket, &readfds);

	smax = nSocket+1;	/*max socket bit*/
	
	rc=select(smax, &readfds, (fd_set *)0, (fd_set *)0, tmval);
	if (rc <= 0)  goto Ret;

	if (FD_ISSET(nSocket, &readfds))	
	{
		peeraddrlen = sizeof(peeraddr);
		newSocket = accept(nSocket, (struct sockaddr *)&peeraddr, (socklen_t*)&peeraddrlen);		
        if (ip != NULL) inet_ntoa_b(peeraddr.sin_addr, ip);
	}  

Ret:
	if (nSocket >= 0) closesocket(nSocket);
	return newSocket;	
}

#define MAINT_PORT 1234
static int udp_socket = -1;
BYTE chnBuf[4096];
BYTE udpbuf[2048];

extern WORD CheckSum(BYTE *buf, int l);
void Udp_Server()
{
	int optval;
	int rec, fromlen,recvlen;
	char ip[20] = {0};
	BYTE ipeth0[5];
	BYTE ipeth1[5];
	WORD crc,crc2;
	struct sockaddr_in peeraddr, myaddr;
	
	if ((udp_socket=socket(AF_INET,SOCK_DGRAM,0))<0)  return;

	optval=1;
	if (setsockopt(udp_socket,SOL_SOCKET,SO_BROADCAST,(char *)&optval,sizeof(optval))<0)
	{
		closesocket(udp_socket);
		return;
	}

	bzero((char *)&myaddr,sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	myaddr.sin_port=htons(MAINT_PORT);

    if (bind(udp_socket, (struct sockaddr *)&myaddr,sizeof(myaddr))<0)
    {
		closesocket(udp_socket);
		return;
    }

	GetNetAddr(0,ipeth0);
	GetNetAddr(1,ipeth1);
	
	for (; ;)
	{
	
    	fromlen=sizeof(peeraddr);	
	    rec = recvfrom(udp_socket, (char *)udpbuf, 25, 0, (struct sockaddr *)&peeraddr, (socklen_t*)&fromlen); 	
		
		if (rec > 0)
		{

		   if ((udpbuf[0] == 0xEB)&&(udpbuf[1] == 0x90)&&
		   	   (udpbuf[2] == 0x05)&&(udpbuf[3] == 0x00)&&
		   	   (udpbuf[4] == 0x0A))
		   {
		       crc = MAKEWORD(udpbuf[5], udpbuf[6]);
			   crc2 = CheckSum(udpbuf, 5);
			   if (crc == crc2)
			   {
				   udpbuf[2]  = 0x0D;
				   udpbuf[3]  = 0x01;
				   udpbuf[4]  = 0x0A;
				   udpbuf[5]  = ipeth0[0];
				   udpbuf[6]  = ipeth0[1];
				   udpbuf[7]  = ipeth0[2];
				   udpbuf[8]  = ipeth0[3];
				   udpbuf[9]  = ipeth1[0];
				   udpbuf[10] = ipeth1[1];
				   udpbuf[11] = ipeth1[2];
				   udpbuf[12] = ipeth1[3];
				   crc = CheckSum(udpbuf, 13);
                   udpbuf[13] = LOBYTE(crc);
				   udpbuf[14] = HIBYTE(crc);
						
				   //peeraddr.sin_addr.s_addr = htonl(0xffffffff);
				   sprintf(ip,"%d.%d.%d.255",ipeth0[0],ipeth0[1],ipeth0[2]);					 
				   peeraddr.sin_addr.s_addr = inet_addr(ip);
				   rec = sendto(udp_socket,(char *)udpbuf, 15, 0, (struct sockaddr *)&peeraddr, fromlen);	
				   printf("udp ret %d \n",rec);

				   sprintf(ip,"%d.%d.%d.255",ipeth1[0],ipeth1[1],ipeth1[2]);					 
				   peeraddr.sin_addr.s_addr = inet_addr(ip);
				   rec = sendto(udp_socket,(char *)udpbuf, 15, 0, (struct sockaddr *)&peeraddr, fromlen);
				   printf("udp ret %d \n",rec);
			   }
			   
		   }
		   else if((rec > 16) && (udpbuf[0] == 0x68)&&(udpbuf[5] == 0x68) && (udpbuf[7] == 0x0c) && (udpbuf[8] == 0x20)) //广播招波形
		   {
		   		
		   		if((udpbuf[14] == 0x01) && (udpbuf[15] == 0x00)) //仅第一个通道触发
					SetBmUrgency(0x01);
				if(Maint_BM_RCV(0x0c20,udpbuf+9,MAKEWORD(udpbuf[1], udpbuf[2]) -3,
				(BYTE*)chnBuf, (WORD*)&recvlen) == OK)
				{
					udpbuf[0]  = 0x68;
					udpbuf[1]  = LOBYTE(recvlen + 3); //len
					udpbuf[2]  = HIBYTE(recvlen + 3);
					udpbuf[3]  = LOBYTE(recvlen + 3);
					udpbuf[4]  = HIBYTE(recvlen + 3);
					udpbuf[5]  = 0x68; //
					udpbuf[6]  = ipeth0[3];
					udpbuf[7]  = 0x8c;
					udpbuf[8]  = 0x20;
					//9  10 11 12
					
					printf("recvlen %d \r\n",recvlen);
					memcpy(udpbuf + 13, chnBuf ,recvlen);

					crc  = CheckSum(udpbuf + 6, recvlen + 3);  
					
					udpbuf[13 + recvlen] = LOBYTE(crc);
					udpbuf[14 + recvlen] = HIBYTE(crc);
					udpbuf[15 + recvlen] = 0x16;

				   sprintf(ip,"%d.%d.%d.255",ipeth0[0],ipeth0[1],ipeth0[2]);					 
				   peeraddr.sin_addr.s_addr = inet_addr(ip);
				   rec = sendto(udp_socket,(char *)udpbuf, 16 + recvlen, 0, (struct sockaddr *)&peeraddr, fromlen);	
				   printf("udp ret %d \n",rec);

				   sprintf(ip,"%d.%d.%d.255",ipeth1[0],ipeth1[1],ipeth1[2]);					 
				   peeraddr.sin_addr.s_addr = inet_addr(ip);
				   rec = sendto(udp_socket,(char *)udpbuf, 16 + recvlen, 0, (struct sockaddr *)&peeraddr, fromlen);
				   printf("udp ret %d \n",rec);
				}
		   }
		}	
	}

}

#endif    /*INCLUDE_NET*/
