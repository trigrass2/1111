/*------------------------------------------------------------------------
 Module:		sntp.c
 Author:		helen
 Project:		
 Creation Date: 2015-3-12
 Description:	
------------------------------------------------------------------------*/
#include "sys.h"
#include "sntp.h"
#include "clock.h"

#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>     
#include <net/route.h>   


#define CLI_INTERVAL_TIME (100*60)
//#define CLI_INTERVAL_TIME (600*60)
#define MAX_SRVR_ADDR_LEN 18
#define MAX_SNTP_TCP_TIMEOUT_NUM 3

char sntpServerAddr[18];
int sntpSocket = -1;

extern struct VSysInfo g_Sys;

static BOOL HaveDataToRead(int hsocket, int timeout)
{
	BOOL bExpire;
	BOOL bNoError;
	int Status;
	struct timeval tv;
	fd_set fds;

	tv.tv_sec = (long)(timeout / 1000);
	tv.tv_usec = (long)(timeout % 1000);
	FD_ZERO(&fds);
	FD_SET(hsocket, &fds);
	Status= select(32, &fds, NULL, NULL, &tv);
	if (Status == ERROR)
		return FALSE;
	else
	{
		bNoError = TRUE;
		bExpire = !(Status == 0);
		return (bNoError && bExpire);
	}
}

static BOOL UpdateSrvrAddr(struct sockaddr_in *psockAddr, int timeout)
{
	int hSock;			
	struct sockaddr_in srcAddr;
	int nAddrLen = sizeof(srcAddr);	
	SNTP_PACKET fullpacket;
	int ReceiveSize=0;
	int ret;
	int len;
//	int optval = 1;

	ReceiveSize = sizeof(fullpacket);

	memset(&srcAddr, 0, sizeof(srcAddr));
	
	/* Initialize source address. */
    srcAddr.sin_addr.s_addr = INADDR_ANY;
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_port = htons(123);

	hSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (hSock == ERROR)
	{
		return FALSE;
	}

	ret = bind(hSock, (struct sockaddr *)&srcAddr, sizeof (srcAddr));
    if (ret == ERROR) 
	{
		closesocket(hSock);
        return FALSE;
	}
 

	if (HaveDataToRead(hSock, timeout*1000))
	{
		len = recvfrom(hSock, (char*)&fullpacket, ReceiveSize, 0, 
					(struct sockaddr *)psockAddr, (socklen_t*)&nAddrLen);
		if(len == ERROR)
		{
			closesocket(hSock);
			return FALSE;
		}
		else if (len > 1)
		{
			if ((fullpacket.leapVerMode & 0x07) == SNTP_MODE_5)
			{
				closesocket(hSock);
				return TRUE;
			}
		}
	}

	closesocket(hSock);
    return FALSE;
}

DWORD sntpcFractionToNsec(DWORD sntpFraction)
{
    DWORD factor = 0x8AC72305; /* Conversion factor from base 2 to base 10 */
    DWORD divisor = 10;        /* Initial exponent for mantissa. */
    DWORD mask = 1000000000;   /* Pulls digits of factor from left to right. */
    int loop;
    DWORD nsec = 0;
    BOOL shift = FALSE;        /* Shifted to avoid overflow? */
 
    if (sntpFraction & 0xF0000000)
    {
        sntpFraction /= 10;
        shift = TRUE;
    }

    for (loop = 0; loop < 10; loop++)    /* Ten digits in mantissa */
    {
		nsec += sntpFraction * (factor/mask)/divisor;  
		factor %= mask;  
		mask /= 10;        
		divisor *= 10;    
    }

    if (shift)
        nsec *= 10;

    return (nsec);
}


BOOL sntpcTimeGet(char* pTargetAddr, long timeout, struct VTimeSpec* pCurrTime)
{
    
    SNTP_PACKET sntpRequest;    
    SNTP_PACKET sntpReply;       
	
    struct sockaddr_in dstAddr;
    struct sockaddr_in servAddr;
    struct timeval sockTimeout;
	
    fd_set readFds;
    int result;
    int servAddrLen;

  	bzero ((char *)&dstAddr, sizeof (dstAddr));
    dstAddr.sin_addr.s_addr = inet_addr(pTargetAddr);;
    dstAddr.sin_family = AF_INET;
    dstAddr.sin_port = htons(123);

    
    /* Initialize SNTP message buffers. */
  
    bzero ((char *)&sntpRequest, sizeof (sntpRequest));
    bzero ((char *)&sntpReply, sizeof (sntpReply));
  
    sntpRequest.leapVerMode = SNTP_CLIENT_REQUEST;
  
    bzero ((char *) &servAddr, sizeof (servAddr));
    servAddrLen = sizeof (servAddr);
  
    /* Transmit SNTP request. */
  
    if (sendto(sntpSocket, (char*)&sntpRequest, sizeof(sntpRequest), 0,
                (struct sockaddr *)&dstAddr, sizeof (dstAddr)) == -1) 
    {
        closesocket(sntpSocket);
		sntpSocket = -1;
        return (ERROR);
    }
    
    /* Convert timeout value to format needed by select() call. */

    if (timeout != WAIT_FOREVER)
    {
        sockTimeout.tv_sec = timeout/1000;
        sockTimeout.tv_usec =  timeout%1000;
    }

    /* Wait for reply at the ephemeral port selected by the sendto () call. */

    FD_ZERO (&readFds);
    FD_SET (sntpSocket, &readFds);

    if (timeout == WAIT_FOREVER)
        result = select (FD_SETSIZE, &readFds, NULL, NULL, NULL);
    else
        result = select (FD_SETSIZE, &readFds, NULL, NULL, &sockTimeout);

    if (result == -1) 
    {
        closesocket(sntpSocket);
		sntpSocket = -1;
        return (ERROR);
    }

    if (result == 0)    /* Timeout interval expired. */
    {
        closesocket(sntpSocket); 
			sntpSocket = -1;
        return (ERROR);
    }

    result = recvfrom (sntpSocket, (char*)&sntpReply, sizeof (sntpReply),
                       0, (struct sockaddr *)&servAddr, (socklen_t*)&servAddrLen);

    if (result == -1) 
    {
        closesocket(sntpSocket);
		sntpSocket = -1;
        return (ERROR);	  
    }

    /*
     * Return error if the server clock is unsynchronized, or the version is 
     * not supported.
     */

    if (((sntpReply.leapVerMode & SNTP_LI_MASK)==SNTP_LI_3) || (sntpReply.transmitTimestampSec == 0))
        return (ERROR);

    if (((sntpReply.leapVerMode & SNTP_VN_MASK)==SNTP_VN_0) || ((sntpReply.leapVerMode & SNTP_VN_MASK)>SNTP_VN_3))
	    return (ERROR);

    /* Add test for 2036 base value here! */

    sntpReply.transmitTimestampSec = ntohl(sntpReply.transmitTimestampSec) - SNTP_UNIX_OFFSET;

    sntpReply.transmitTimestampFrac = ntohl(sntpReply.transmitTimestampFrac);

    pCurrTime->tv_sec = sntpReply.transmitTimestampSec;
    pCurrTime->tv_nsec = sntpcFractionToNsec (sntpReply.transmitTimestampFrac);

    return (OK);
}


static BOOL SetSysClockFromSNTPS(char *pServerAddr, int timeout)
{
    struct VTimeSpec CurrTime;
	struct VCalClock CalTime;
	DWORD second;

	if (sntpcTimeGet(pServerAddr, timeout, &CurrTime) == OK )
    {
        second = CurrTime.tv_sec + 28800;       //加8个小时
		CalTime.dwMinute = second / 60;
		CalTime.wMSecond = (second%60)*1000 + CurrTime.tv_nsec/1000000;
 	
	    SetSysClock((void *)&CalTime, CALCLOCK);
		
        return TRUE;
    }
	shellprintf("sntp false\n");

    return FALSE;
}

/* Create socket for transmission. */
void InitSntpSocket(void)
{
    int optval;
	int result;


   	sntpSocket = socket (AF_INET, SOCK_DGRAM, 0);
    if (sntpSocket == -1) 
        return;
	
    optval = 1;
    result = setsockopt (sntpSocket, SOL_SOCKET, SO_BROADCAST, 
                         (char *)&optval, sizeof (optval));

	if (result == -1)
	   sntpSocket = -1;

	return;
}

void sntp(void)
{ 
    BOOL bBroadcast = TRUE;
	DWORD serverIp;
	DWORD events;


	if (!(g_Sys.MyCfg.dwCfg & 0x02))
		return;

	serverIp = g_Sys.MyCfg.dSntpServer;
	if (serverIp != 0)
		bBroadcast = FALSE;

	sprintf(sntpServerAddr, "%d.%d.%d.%d", (serverIp>>24)&0xff, (serverIp>>16)&0xff, (serverIp>>8)&0xff, serverIp&0xff);

	tmEvEvery(SNTP_ID, CLI_INTERVAL_TIME, EV_TM1);
	
//	strcpy(sntpServerAddr, "192.168.5.99");
//   bBroadcast = FALSE;
    for (; ;)
	{		
		struct sockaddr_in sockAddr;
		char srvrAddr[16];

		evReceive(SNTP_ID , EV_TM1, &events);

		if (events & EV_TM1)
		{
		    if (sntpSocket == -1)
				InitSntpSocket();
			if (sntpSocket == -1)
				continue;

			if (bBroadcast)
			{			

				unsigned long addr;

				if (UpdateSrvrAddr(&sockAddr, CLI_INTERVAL_TIME) == FALSE)
					continue;

				addr=sockAddr.sin_addr.s_addr;
				sprintf(srvrAddr, "%d.%d.%d.%d", (int)(addr>>24)&0xFF, (int)(addr>>16)&0xFF,
						(int)(addr>>8)&0xFF, (int)addr&0xFF);

				SetSysClockFromSNTPS(srvrAddr, 2000);
			
			}
			else
			{
                SetSysClockFromSNTPS(sntpServerAddr, 2000);
			}
		}
	}

}


