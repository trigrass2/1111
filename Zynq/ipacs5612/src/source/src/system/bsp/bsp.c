#include "syscfg.h"

#include "bsp.h"
#include "os.h"
#include <netinet/tcp.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>     
#include <net/route.h>    

int GetNetAddr(int no,BYTE* szAddr)
{
	char *addr;
	int sock;
	struct ifreq ifreq;

	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		DPRINT("socket failed.\n");
		return -1;
	}
	
	sprintf (ifreq.ifr_name, "eth%d",no);

	if (ioctl (sock, SIOCGIFADDR, &ifreq) < 0)
	{
		DPRINT("ioctl SIOCGIFADDR failed.\n");
		return -1;
	}
	memcpy(szAddr,&((struct sockaddr_in*)&(ifreq.ifr_addr))->sin_addr,sizeof(struct in_addr));

	close(sock);
	return 0;
}

int ESDK_GetNetMac(int no,char *szMac)
{
	int sock;
	struct ifreq ifreq;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		DPRINT("socket failed.\n");
		return -1;
	}
	sprintf (ifreq.ifr_name, "eth%d",no);

	if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
	{
		DPRINT ("ioctl SIOCGIFHWADDR failed.\n");
		return -1;
	}
	sprintf(szMac, "%02X:%02X:%02X:%02X:%02X:%02X", (unsigned char) ifreq.ifr_hwaddr.sa_data[0], (unsigned char) ifreq.ifr_hwaddr.sa_data[1], 
											   (unsigned char) ifreq.ifr_hwaddr.sa_data[2], (unsigned char) ifreq.ifr_hwaddr.sa_data[3], 
											   (unsigned char) ifreq.ifr_hwaddr.sa_data[4], (unsigned char) ifreq.ifr_hwaddr.sa_data[5]);
	close(sock);
	return 0;
}

int ESDK_GetNetAddr(int no,char *szAddr)
{
	char *addr;
	int sock;
	struct ifreq ifreq;

	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		DPRINT("socket failed.\n");
		return -1;
	}
	sprintf (ifreq.ifr_name, "eth%d",no);

	if (ioctl (sock, SIOCGIFADDR, &ifreq) < 0)
	{
		DPRINT("ioctl SIOCGIFADDR failed.\n");
		return -1;
	}
	addr = inet_ntoa(((struct sockaddr_in*)&(ifreq.ifr_addr))->sin_addr);
	memcpy(szAddr, addr, strlen(addr));

	close(sock);
	return 0;
}

int ESDK_SetNetInfo(int no,const char *szAddr, const char *szMask, const char *szGatway)
{
	int sock;
	struct ifreq ifr; 
	struct sockaddr_in *sin;
	struct rtentry  rt;


	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		DPRINT("socket failed %d\n", sock);
		return -1;
	}
	sprintf (ifr.ifr_name, "eth%d",no);
	sin = (struct sockaddr_in*)&ifr.ifr_addr;
	sin->sin_family = AF_INET;

	if(szAddr != NULL)
	{
		//IP
		if(inet_aton(szAddr, &(sin->sin_addr)) < 0)
		{
			DPRINT("inet_aton failed.\n");
			close(sock);
			return -1;
		}

		if(ioctl(sock, SIOCSIFADDR, &ifr) < 0)
		{
			DPRINT("ioctl SIOCSIFADDR failed.\n");
			close(sock);
			return -1;
		}
	}

	if(szMask != NULL)
	{
		//mask
		if(inet_aton(szMask, &(sin->sin_addr)) < 0)
		{
			DPRINT("inet_aton failed.\n");
			close(sock);
			return -1;
		}
		if(ioctl(sock, SIOCSIFNETMASK, &ifr) < 0)
		{
			DPRINT("ioctl SIOCSIFNETMASK failed.\n");
			close(sock);
			return -1;
		}
	}

	//getway
	if(szGatway != NULL)
	{
		memset(&rt, 0, sizeof(struct rtentry));
		memset(sin, 0, sizeof(struct sockaddr_in));
		sin->sin_family = AF_INET;
		sin->sin_port = 0;
		if(inet_aton(szGatway, &sin->sin_addr)<0)
		{
			DPRINT("inet_aton failed.\n");
		}
		memcpy ( &rt.rt_gateway, sin, sizeof(struct sockaddr_in));
		((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;
		((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;
		rt.rt_flags = RTF_GATEWAY;

		if (ioctl(sock, SIOCADDRT, &rt)<0)
		{
			DPRINT("ioctl SIOCADDRT failed.\n");
			close(sock);
			return -1;
		}
	}

	close(sock);
	return 0;
}

int set_ip(int no, char *ip)
{
    struct ifreq temp;
    struct sockaddr_in *addr;
    int fd = 0;
    int ret = -1;
    sprintf (temp.ifr_name, "eth%d",no);
    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
      return -1;
    }
    addr = (struct sockaddr_in *)&(temp.ifr_addr);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    ret = ioctl(fd, SIOCSIFADDR, &temp);
    close(fd);
    if(ret < 0)
    {
       printf("set_ip %s error,errno=%d [%s]\n", temp.ifr_name, errno, strerror(errno));
       return -1;
    }
    return 0;
}

void reboot(int flag)
{
	system("reboot");
}

static DWORD devLight = 0x00;
static DWORD old_DevLight = 0x00;

BOOL Read_GPIO(int no,int id)   //暂时未用此接口
{
	BYTE value;
	if(ReadIO(no,id,&value) == OK)
	{
		return value;
	}
	else
		return 0;
}

void turnLight(int id, BYTE on)
{
	DWORD bit = 0x01<<id;
	
	if (on)  devLight |= bit;
	else devLight &= (~bit);		
	
	if ((devLight^old_DevLight) == 0)  return;

    SetLinuxLed(id,on); 

	old_DevLight = devLight;
}

