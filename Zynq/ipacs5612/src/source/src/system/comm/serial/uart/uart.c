/*------------------------------------------------------------------------
 Module:        uart.c 
 Author:        helen
 Project:       
 State:			
 Creation Date:	2014-9-24
 Description:   coldfire的UART驱动实现
------------------------------------------------------------------------*/

#include "comm_cfg.h"

#ifdef INCLUDE_UART
#include "comm.h"
#include "serial.h"
#include "uart.h"
#include "os.h"
#include <unistd.h>
#include <termios.h>
#include <fcntl.h> 

#define UART_NUM  9

static VUartChan g_UartChan[UART_NUM];

static int uartOpen(int phy_no);
static int uartClose(int fd);
static int setlock(int fd,DWORD type);
static int  uartBaudSet(int fd, DWORD baud);
static int  uartOptsSet(int fd, DWORD options);	
static int uart_regist(int phy_no);
void uart_rx(int phy_no);
void uart_tx(int phy_no);

/*------------------------------------------------------------------------
 Procedure:     uartInit ID:1
 Purpose:       ???uart,dma mode, no interrupt
 Input:
 Output:
 Errors:        
------------------------------------------------------------------------*/
int uartInit(void)
{
	int i; 
	
	memset(g_UartChan, 0, sizeof(g_UartChan));

	for(i=0; i<UART_NUM; i++) 
	{
		g_UartChan[i].no = i;
		g_UartChan[i].addr = i + 1;
		
		g_CommChan[g_UartChan[i].no].fd = -1;
		uart_regist(i);
	}
	
	return 0;
}
/*------------------------------------------------------------------------
 Procedure:     uartCtrl ID:1
 Purpose:       每个通道控制函数
 Input:         no: 通信口统一编号;
 				command: 控制命令; para: 命令参数
 Output:		0:OK; -1:ERROR.
 Errors:
------------------------------------------------------------------------*/
int uartCtrl(int no, WORD command, BYTE *para)
{
	int fd,phy_no; 
	DWORD opt,baud;

	if (no < 0) return ERROR;
	
	fd = g_CommChan[no].fd;
	phy_no = g_CommChan[no].phy_no;
	switch(command&0xFF00)
	{
		case CCC_BAUD_CTRL:
			if (command & BAUD_SET) 
			{
			    memcpy((BYTE*)&baud, para, 4);
			    uartBaudSet(fd, baud); 
				g_UartChan[phy_no].baudRate = baud;
			}
			if (command & BAUD_GET) 
			{
			   memcpy(para, (BYTE*)&(g_UartChan[phy_no].baudRate), 4);
			}
			break;
		case CCC_INT_CTRL:
			break;
		case CCC_CONNECT_CTRL:
			if (command & CONNECT_OPEN) g_CommChan[no].fd = uartOpen(phy_no);
			if (command & CONNECT_CLOSE) uartClose(fd);
			break;
		case CCC_DATA_BITS:
			if(*para !=7 && *para != 8) return ERROR;			
			opt = g_UartChan[phy_no].options&(~CSIZE);
			if     (*para == 8) opt = opt|CS8;
			else   				opt = opt|CS7;
			uartOptsSet(fd, opt);
			g_UartChan[phy_no].options = opt;
			break;
		case CCC_STOP_BITS:					/*???1&2位都能连接*/
			if(*para > 2 || *para == 0) return ERROR;	
			opt = g_UartChan[phy_no].options&(~CSTOPB);
			if (*para == 2) opt = opt|CSTOPB;
			else opt = opt & (~CSTOPB);
			uartOptsSet(fd, opt);
			g_UartChan[phy_no].options = opt;
			break;
		case CCC_PARITY_SET: 					/*???奇偶校验都能连接*/
			if(*para==0 || *para>3) return ERROR;
			opt = g_UartChan[phy_no].options&(~(PARENB|PARODD));
			if(*para == PARITY_SET_EVEN)    opt = opt|PARENB;
			if(*para == PARITY_SET_ODD)		opt = opt|PARODD;
			uartOptsSet(fd, opt);
			g_UartChan[phy_no].options = opt;
			break;
		case CCC_CFG_STR:
			if (command & CFG_STR_SET) SerialCfgStrSet(no, (char *)para);
			break;			
		default:
			return ERROR;
	};
	
	return OK;
}

/*------------------------------------------------------------------------
 Procedure:     uart_open ID:1
 Purpose:       初始化每个通道
 Input:         phy_no: 通道物理号;
 Output:		0:OK; -1:ERROR.
 Errors:
------------------------------------------------------------------------*/
static int uart_regist(int phy_no)
{
	int no; 
	
	no = commRegister(g_UartChan[phy_no].no, phy_no, SERIAL_BUF_NUM, (void *)SerialRead, (void *)SerialWrite, (void *)uartCtrl);
	g_UartChan[phy_no].no = no;
	if (no < 0) return ERROR;
	return OK;
}



void uart_scan(void)
{
	int i;

	for (i=0; i<UART_NUM; i++)
	{
		if(g_UartChan[i].no < 0)
			continue;
		if (g_CommChan[g_UartChan[i].no].fd < 0) 
			continue;
		uart_rx(i);
		uart_tx(i);
	}
}


void uart_rx(int phy_no)
{
    int len;
	int fd,i;
	BYTE buf[SERIAL_BUF_NUM];
	VCommBuf *pfifo;
	int rev = 0;

	fd = g_CommChan[g_UartChan[phy_no].no].fd;

	pfifo = &(g_CommChan[g_UartChan[phy_no].no].rx_buf);
#if 0
	speed = (int)g_UartChan[phy_no].baudRate;
	/*串口接收数据超时时间按20个字节间隔时间来计算，300bps 发送一个字节需要36667us
		一个字节包括起始位、停止位、校验位、8位数据 1000/(300/11) = 36.666ms*/
    if ((speed%300!=0 )||(speed < 300))	/*若波特率不对，默认9600*/
		delay = 20*36667/(9600/300);
	else
		delay = 20*36667/(speed/300);
#endif	
    len = read(fd, buf, 1024);	
	for (i=0; i<len; i++)
	{
	    pfifo->buf[pfifo->wp] = buf[i];

		pfifo->wp = (pfifo->wp+1)&(SERIAL_BUF_NUM-1);
		rev++;
	}

#if 0
	len = read(fd, buf, 1024);	
	while (len > 0)		
	{
		usleep(delay);
		for (i=0; i<len; i++)
		{
		    pfifo->buf[pfifo->rp] = buf[i];
			pfifo->rp = (pfifo->rp+1)&(SERIAL_BUF_NUM-1);
			rev++;
		}
		len = read(fd, buf, 1024);
		if (len == 0)
           break;
	}
#endif

	if(rev > 0)
		evSend(g_CommChan[phy_no].tid, EV_RX_AVAIL);
	return;	
}

void uart_tx(int phy_no)
{
    int fd;
	VCommBuf *pfifo;

	pfifo = &(g_CommChan[g_UartChan[phy_no].no].tx_buf);
	fd = g_CommChan[g_UartChan[phy_no].no].fd;
	
	/*无发送buffer*/	
	if(fd == -1)
	{
		return;
	}
	while (pfifo->rp != pfifo->wp)
	{
		write(fd, pfifo->buf+pfifo->rp, 1);
		pfifo->rp = (pfifo->rp+1)&(SERIAL_BUF_NUM-1);
    }
}

static int uartOpen(int phy_no)
{
    int fd;
	char name[32];
		
	sprintf(name, "/dev/ttyS%d", phy_no);
	fd = open(name, O_RDWR|O_NOCTTY|O_NONBLOCK );	/*打开串口*/
	if (fd == -1)		
	{ 	
		printf("串口打开失败 \n");
		close(fd);
		return -1;	
	}	
#if 0		
	if (setlock(fd, F_WRLCK) == -1)	/*锁串口*/
	{
        close(fd);
		return -1;
	}
#endif
	return fd;
}

static int setlock(int fd, DWORD type)
{
	struct flock lock;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 1;
	lock.l_type = type;

	if (fcntl(fd, F_SETLK, &lock) == 0)
		return 1;
	else
		return -1;
}

static int uartClose(int fd)
{
    close(fd);
	return -1;
}

static int uartBaudSet(int fd, DWORD baud)
{
	struct termios Opt;
	int i,status;
	DWORD speed_arr[10] = {B115200, B57600,B38400, B19200, B9600, B4800, B2400, B1200, B600, B300};
    DWORD name_arr[10] = {115200, 57600, 38400,  19200,  9600,  4800,  2400,  1200, 600, 300};

#if 1
	if(fd == -1)
	{
		printf("uartBaudSet = -1 \n");
		return ERROR;
	}
	tcgetattr(fd, &Opt); 	
	
    for (i=0; i<10; i++)
	{
	    if (baud == name_arr[i])
	    {
	        cfsetispeed(&Opt, speed_arr[i]); /*设置输入波特率*/
	        cfsetospeed(&Opt, speed_arr[i]); /*设置输出波特率*/
			break;
	    }
    }
	status = tcsetattr(fd, TCSANOW, &Opt);  /*设置串口新的termios结构*/
	if  (status != 0)
	{	
		printf("uartBaudSet error \n");
		return -1;     
	}
    
	tcflush(fd,TCIOFLUSH); /*清除串口的输入输出队列中的数据*/
#endif	
    return (OK);
}

static int uartOptsSet(int fd, DWORD options)
{	
    struct termios Opt;
#if 1	
	if(fd == -1)
	{
		printf("uartOptsSet = -1 \n");
		return ERROR;
	}
	if(tcgetattr(fd, &Opt) != 0)
	{
		printf("uartOptsSet tcgetattr err \n");
		return ERROR;
	}
	
	Opt.c_oflag = 0;
	Opt.c_lflag = 0;
	Opt.c_iflag = IGNBRK;
	Opt.c_cflag |= CLOCAL | CREAD;

    /* Reset the transmitters  & receivers  */
	
    switch (options & CSIZE)
	{
	      case CS8:
		  	Opt.c_cflag &= ~CSIZE;
	        Opt.c_cflag |= CS8;
	      break;
		  case CS7:
		  	Opt.c_cflag &= ~CSIZE;
	        Opt.c_cflag |= CS7;
	      break;
	      default:
		  	Opt.c_cflag &= ~CSIZE;
	        Opt.c_cflag |= CS8;
	      break;
	}

	switch (options & (PARENB|PARODD))
	{
		case PARENB:
		    Opt.c_cflag &= ~PARODD;
			Opt.c_cflag |= PARENB;
		    break;
		case PARODD:
		    Opt.c_cflag |= (PARODD | PARENB);
		    break;
		default:
		    Opt.c_cflag &= ~PARENB;
		    break;
	}

    if (options & CSTOPB)
	    Opt.c_cflag |= CSTOPB;
    else
	    Opt.c_cflag &= ~CSTOPB;


	tcflush(fd,TCIFLUSH);					/* 清输入队列 */
	Opt.c_cc[VTIME] = 100; 		        /* 设置超时10 seconds*/   
	Opt.c_cc[VMIN] = 0; 			    /* 更新设置，使马上生效 */
	
	Opt.c_iflag &= ~(IXON|IXOFF|IXANY);	/*关软件流控*/
	
	Opt.c_cflag &= ~CRTSCTS;            /*关硬件流控*/

	Opt.c_iflag &= ~IGNCR;	
	Opt.c_iflag &= ~ICRNL;  

	if (tcsetattr(fd,TCSANOW,&Opt) != 0)  /* 对串口进行参数设置 */
	{
		printf("uartOptsSet tcsetattr err \n");
		return (ERROR);  
	}
#endif
    return (OK);
}

#endif
