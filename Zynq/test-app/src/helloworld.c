/*
 * Copyright (c) 2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <linux/rtc.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include <linux/spi/spidev.h>


/* todo list
 * 1, test rtc
 * 2, test I2C device
 * 3, test uart ttyUL
 */

#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

//为了保证用户输入的波特率是个正确的值，所以需要这两个数组验证，对于设置波特率时候，前面要加个B
int speed_arr[] = { B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300,
    B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300, };

int name_arr[] = {230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300,
    115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300, };

/*-----------------------------------------------------------------------------
  函数名:      set_speed
  参数:        int fd ,int speed
  返回值:      void
  描述:        设置fd表述符的串口波特率
 *-----------------------------------------------------------------------------*/
void set_speed(int fd ,int speed)
{
    struct termios opt;
    int i;
    int status;

    tcgetattr(fd,&opt);
    for(i = 0;i < sizeof(speed_arr)/sizeof(int);i++)
    {
        if(speed == name_arr[i])                        //找到标准的波特率与用户一致
        {
            tcflush(fd,TCIOFLUSH);                      //清除IO输入和输出缓存
            cfsetispeed(&opt,speed_arr[i]);         //设置串口输入波特率
            cfsetospeed(&opt,speed_arr[i]);         //设置串口输出波特率

            status = tcsetattr(fd,TCSANOW,&opt);    //将属性设置到opt的数据结构中，并且立即生效
            if(status != 0)
                perror("tcsetattr fd:");                //设置失败
            return ;
        }
        tcflush(fd,TCIOFLUSH);                          //每次清除IO缓存
    }
}
/*-----------------------------------------------------------------------------
  函数名:      set_parity
  参数:        int fd
  返回值:      int
  描述:        设置fd表述符的奇偶校验
 *-----------------------------------------------------------------------------*/
int set_parity(int fd)
{
    struct termios opt;

    if(tcgetattr(fd,&opt) != 0)                 //或许原先的配置信息
    {
        perror("Get opt in parity error:");
        return -1;
    }

    /*通过设置opt数据结构，来配置相关功能，以下为八个数据位，不使能奇偶校验*/
    opt.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                | INLCR | IGNCR | ICRNL | IXON);
    opt.c_oflag &= ~OPOST;
    opt.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    opt.c_cflag &= ~(CSIZE | PARENB);
    opt.c_cflag |= CS8;

    tcflush(fd,TCIFLUSH);                           //清空输入缓存

    if(tcsetattr(fd,TCSANOW,&opt) != 0)
    {
        perror("set attr parity error:");
        return -1;
    }

    return 0;
}
/*-----------------------------------------------------------------------------
  函数名:      serial_init
  参数:        char *dev_path,int speed,int is_block
  返回值:      初始化成功返回打开的文件描述符
  描述:        串口初始化，根据串口文件路径名，串口的速度，和串口是否阻塞,block为1表示阻塞
 *-----------------------------------------------------------------------------*/
int serial_init(char *dev_path,int speed,int is_block)
{
    int fd;
    int flag;

    flag = 0;
    flag |= O_RDWR;                     //设置为可读写的串口属性文件
    if(is_block == 0)
        flag |=O_NONBLOCK;              //若为0则表示以非阻塞方式打开

    fd = open(dev_path,flag);               //打开设备文件
    if(fd < 0)
    {
        perror("Open device file err:");
        close(fd);
        return -1;
    }

    /*打开设备文件后，下面开始设置波特率*/
    set_speed(fd,speed);                //考虑到波特率可能被单独设置，所以独立成函数

    /*设置奇偶校验*/
    if(set_parity(fd) != 0)
    {
        perror("set parity error:");
        close(fd);                      //一定要关闭文件，否则文件一直为打开状态
        return -1;
    }

    return fd;
}
/*-----------------------------------------------------------------------------
  函数名:      serial_send
  参数:        int fd,char *str,unsigned int len
  返回值:      发送成功返回发送长度，否则返回小于0的值
  描述:        向fd描述符的串口发送数据，长度为len，内容为str
 *-----------------------------------------------------------------------------*/
int serial_send(int fd,char *str,unsigned int len)
{
    int ret;

    if(len > strlen(str))                    //判断长度是否超过str的最大长度
        len = strlen(str);

    ret = write(fd,str,len);
    if(ret < 0)
    {
        perror("serial send err:");
        return -1;
    }

    return ret;
}

/*-----------------------------------------------------------------------------
  函数名:      serial_read
  参数:        int fd,char *str,unsigned int len,unsigned int timeout
  返回值:      在规定的时间内读取数据，超时则退出，超时时间为ms级别
  描述:        向fd描述符的串口接收数据，长度为len，存入str，timeout 为超时时间
 *-----------------------------------------------------------------------------*/
int serial_read(int fd, char *str, unsigned int len, unsigned int timeout)
{
    fd_set rfds;
    struct timeval tv;
    int ret;                                //每次读的结果
    int sret;                               //select监控结果
    int readlen = 0;                        //实际读到的字节数
    char * ptr;

    ptr = str;                          //读指针，每次移动，因为实际读出的长度和传入参数可能存在差异

    FD_ZERO(&rfds);                     //清除文件描述符集合
    FD_SET(fd,&rfds);                   //将fd加入fds文件描述符，以待下面用select方法监听

    /*传入的timeout是ms级别的单位，这里需要转换为struct timeval 结构的*/
    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout%1000)*1000;

    /*防止读数据长度超过缓冲区*/
    //if(sizeof(&str) < len)
    //  len = sizeof(str);


    /*开始读*/
    while(readlen < len)
    {
        sret = select(fd+1,&rfds,NULL,NULL,&tv);        //检测串口是否可读

        if(sret == -1)                              //检测失败
        {
            perror("select:");
            break;
        }
        else if(sret > 0)
        {
            ret = read(fd,ptr,1);
            if(ret < 0)
            {
                perror("read err:");
                break;
            }
            else if(ret == 0)
                break;

            readlen += ret;                             //更新读的长度
            ptr     += ret;                             //更新读的位置
        }
        else                                                    //超时
        {
            //printf("timeout!\n");
            break;
        }
    }

    return readlen;
}

void rtc_test()
{
	int retval, fd;
	struct rtc_time rtc_tm;

	fd = open("/dev/rtc0", O_RDONLY);
	if (-1 == fd)
	{
		perror("error open /dev/rtc0");
		exit(EXIT_FAILURE);
	}

	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	if (-1 == retval)
	{
		perror("error RTC_RD_TIME ioctrl");
		exit(EXIT_FAILURE);
	}

	printf("\r\nsec=%d, min=%d, hour=%d day=%d mon=%d year=%d\r\n",
			rtc_tm.tm_sec,
			rtc_tm.tm_min,
			rtc_tm.tm_hour,
			rtc_tm.tm_mday,
			rtc_tm.tm_mon+1,
			rtc_tm.tm_year+1900
			);

	close(fd);
}

void temperature_and_humidity_test()
{
	FILE *p_humidity=NULL;
	FILE *p_temperature=NULL;
	int humidity;
	int temperature;

	/* read humidity */
	p_humidity = fopen("/sys/bus/iio/devices/iio\:device1/in_humidityrelative_input","r");
	if (p_humidity == NULL)
	{
		perror("error open humidity :");
	}
	fscanf(p_humidity, "%d", &humidity);
	printf("read humidity %d\r\n", humidity);
	fclose(p_humidity);


	/* read teemperature */
	p_temperature = fopen("/sys/bus/iio/devices/iio\:device1/in_temp_input","r");
	if (p_temperature == NULL)
	{
		perror("error open termperature :");
	}
	fscanf(p_temperature, "%d", &temperature);
	printf("read temperature %d\r\n", temperature);
	fclose(p_temperature);
}

#define MAX_BYTES            		2
#define DEFAULT_I2C_BUS_PREFIX      "/dev/i2c-"
#define MAX_OPTICAL_POWER_CHAN_CNT 	8
#define OPTICAL_POWER_I2C_ADDR		0x51
#define OPTICAL_TX_POWER_REG		102
#define OPTICAL_RX_POWER_REG		104

int i2c_write(int fd,
		 unsigned int addr,
		 unsigned int offset,
		 unsigned char *buf,
		 unsigned int len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;
	int i;
	char _buf[MAX_BYTES+1];

	if(len>MAX_BYTES)
	{
		return -1;
	}

	if(offset+len>256)
	{
		return -1;
	}

	_buf[0]=offset;
	for(i=0;i<len;i++)
	{
	    _buf[1+i]=buf[i];
	    printf("----_writedata:%x------\n",_buf[1+i]);
	}

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  = addr;
	i2cmsg.flags = 0;
	i2cmsg.len   = 1+len;
	i2cmsg.buf   = _buf;

	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
		perror("ioctl()");
		fprintf(stderr,"ioctl returned %d\n",i);
		return -1;
	}

	return 0;
}

int i2c_read(int fd,
			unsigned int addr,
			unsigned int offset,
			unsigned char *buf,
			unsigned int len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;
	int i;

	if(len>MAX_BYTES)
	{
		return -1;
	}

	if(i2c_write(fd,addr,offset,NULL,0)<0)
	{
	    return -1;
	}

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  = addr;
	i2cmsg.flags = I2C_M_RD;
	i2cmsg.len   = len;
	i2cmsg.buf   = buf;

	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
	    perror("ioctl()");
	    fprintf(stderr,"ioctl returned %d\n",i);
	    return -1;
	}
	return 0;
}

int get_optical_tx_power(int chan_idx)
{
	int fd;
	char i2cBusName[0x20] = {0};
	unsigned char readbuf[2];

	if (chan_idx >= MAX_OPTICAL_POWER_CHAN_CNT)
		return 0;

	sprintf(i2cBusName, "%s%d", DEFAULT_I2C_BUS_PREFIX, chan_idx);
	fd =open(i2cBusName, O_RDWR);

	i2c_read(fd,
			OPTICAL_POWER_I2C_ADDR,
			OPTICAL_TX_POWER_REG,
			readbuf,
			2);

	close(fd);
	return ((readbuf[0] << 8) + readbuf[1]);
}

int get_optical_rx_power(int chan_idx)
{
	int fd;
	char i2cBusName[0x20] = {0};
	unsigned char readbuf[2];

	if (chan_idx >= MAX_OPTICAL_POWER_CHAN_CNT)
		return 0;

	sprintf(i2cBusName, "%s%d", DEFAULT_I2C_BUS_PREFIX, chan_idx);
	fd =open(i2cBusName, O_RDWR);

	i2c_read(fd,
			OPTICAL_POWER_I2C_ADDR,
			OPTICAL_RX_POWER_REG,
			readbuf,
			2);

	close(fd);
	return ((readbuf[0] << 8) + readbuf[1]);
}

int optical_power_test()
{
	int tx_power_val;
	int rx_power_val;

	tx_power_val = get_optical_tx_power(0);
	rx_power_val = get_optical_rx_power(0);

	printf("Read Optical power Tx power 0x%04x\r\n", tx_power_val);
	printf("Read Optical power Rx power 0x%04x\r\n", rx_power_val);

	return 0;
}

#define UART_TTY_S0	"/dev/ttyS0"
#define UART_TTY_S1	"/dev/ttyS1"
#define UART_TTY_S2	"/dev/ttyS2"
#define UART_TTY_S3	"/dev/ttyS3"

#define UART_SPEED 9600

#define ENABLE_UART0
#define ENABLE_UART1
#define ENABLE_UART2
#define ENABLE_UART3

int ttyUl_test()
{
    int fd0 = 0;
    int fd1 = 0;
    int fd2 = 0;
    int fd3 = 0;
    int ret;
    int readLen;
    char str[]="hello linux serial!";
    char buf[100];
    int i;

#ifdef ENABLE_UART0
    fd0 =  serial_init(UART_TTY_S0,UART_SPEED,1);
    if(fd0 < 0)
    {
        perror("serial init err:");
        return -1;
    }
#endif

#ifdef ENABLE_UART1
    fd1 =  serial_init(UART_TTY_S1,UART_SPEED,1);
    if(fd1 < 0)
    {
        perror("serial init err:");
        return -1;
    }
#endif

#ifdef ENABLE_UART2
    fd2 =  serial_init(UART_TTY_S2,UART_SPEED,1);
    if(fd2 < 0)
    {
        perror("serial init err:");
        return -1;
    }
#endif

#ifdef ENABLE_UART3
    fd3 =  serial_init(UART_TTY_S3,UART_SPEED,1);
    if(fd3 < 0)
    {
        perror("serial init err:");
        return -1;
    }
#endif

    while(1)
    {
#ifdef ENABLE_UART0
        /* ttyS0 send */
        ret = serial_send(fd0,str,22);
        printf("ttyS0 send len %d : \t%s \r\n",ret, str);

        /* ttyS0 read */
        memset(buf, 0, sizeof(buf));
        readLen = serial_read(fd0,buf,100,100);
        if (readLen)
        {
			printf("ttyS0 receive len %d : \t%s\r\n",readLen, buf);
			/*
			for (i=0; i<readLen; i++)
				printf("%x ", buf[i]);
			printf("\r\n");
			*/
        }
#endif

#ifdef ENABLE_UART1
        /* ttyS1 send */
        ret = serial_send(fd1,str,22);
        printf("ttyS1 send len %d : \t%s \n",ret, str);

        /* ttyS1 read */
        memset(buf, 0, sizeof(buf));
        readLen = serial_read(fd1,buf,100,100);
        if (readLen)
        {
			printf("ttyS1 receive len %d : \t%s\r\n",readLen, buf);
			/*
			for (i=0; i<readLen; i++)
				printf("%x ", buf[i]);
			printf("\r\n");
			*/
        }
#endif

#ifdef ENABLE_UART2
        /* ttyS2 send */
        ret = serial_send(fd2,str,22);
        printf("ttyS2 send len %d : \t%s \n",ret, str);

        /* ttyS1 read */
        memset(buf, 0, sizeof(buf));
        readLen = serial_read(fd2,buf,100,100);
        if (readLen)
        {
			printf("ttyS2 receive len %d : \t%s\r\n",readLen, buf);
			/*
			for (i=0; i<readLen; i++)
				printf("%x ", buf[i]);
			printf("\r\n");
			*/
        }
#endif

#ifdef ENABLE_UART3
        /* ttyS3 send */
        ret = serial_send(fd3,str,22);
        printf("ttyS3 send len %d : \t%s \n",ret, str);

        /* ttyS3 read */
        memset(buf, 0, sizeof(buf));
        readLen = serial_read(fd3,buf,100,100);
        if (readLen)
        {
			printf("ttyS3 receive len %d : \t%s\r\n",readLen, buf);
			/*
			for (i=0; i<readLen; i++)
				printf("%x ", buf[i]);
			printf("\r\n");
			*/
        }
#endif

    	sleep(2);
    }

#ifdef ENABLE_UART0
    close(fd0);
#endif
#ifdef ENABLE_UART1
    close(fd1);
#endif
#ifdef ENABLE_UART2
    close(fd2);
#endif
#ifdef ENABLE_UART3
    close(fd3);
#endif
    return 0;
}


#define  devicename  "/dev/spidev0.%d"
#define BYTE char
#define DWORD int
#define WORD short

static BYTE mode;
static BYTE bits = 8;
static DWORD speed = 1200000;
static WORD delay;
int SC1161Y_fd = -1;
int AD7913_fd = -1;

#define   AD7913_STAUTS0       0x09
#define   AD7913_READ          0x04
#define   AD7913_WRITE         0x00

#define FALSE  	-1
#define TRUE	0
#define OK 		0

void SPI_init(int no)
{
	int ret = 0;
	int fd = 0;
	char name[64];

	sprintf(name, devicename,no);
	printf("SPI_init name %s \n",name);

	fd = open(name, O_RDWR);
	if (fd< 0)
		printf("can't open device");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		printf("can't get spi mode");

	mode |= SPI_CPHA;
	mode |= SPI_CPOL;

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		printf("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		printf("can't get spi mode");

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't get bits per word");

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
	sleep(2);

	if(no == 0)
		SC1161Y_fd = fd;
	else
		AD7913_fd = fd;
}

static int SPITrans_BAddr(int SPIx, BYTE phy_no,BYTE *send,BYTE *recv,DWORD len)
{
	int ret;
	int i = 0;
	int fd;
	struct spi_ioc_transfer tr = {
	     .tx_buf = (unsigned long)send,   //¶šÒå·¢ËÍ»º³åÇøÖžÕë
	     .rx_buf = (unsigned long)recv,   //¶šÒåœÓÊÕ»º³åÇøÖžÕë
	     .len = len,

	};

	if(phy_no == 0)
		fd = SC1161Y_fd;
	else
		fd = AD7913_fd;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);//ÖŽÐÐspidev.cÖÐioctlµÄdefaultœøÐÐÊýŸÝŽ«Êä

	if (ret < 1)
	{
		printf("can't send spi message \n");
		return FALSE;
	}

		printf("tr.len = %d\n",tr.len);

	return TRUE;
}


static int ad7913Read(int SPIx, BYTE phy_no, BYTE addr, int len)
{
     int ret=-1;
     int val=0;
     BYTE send[4] = {0};
	 BYTE recv[4] = {0};


	 if (len > 3) return -1;

	 send[0] = (addr << 3) | AD7913_READ;

#if 1
	 ret = SPITrans_BAddr(SPIx, phy_no, send, recv, len+1);
#else
	 ret = SPIRead_BAddr(send[0], recv, len);
#endif

	 if (ret < 0) return -1;

	 if (len == 1)
	 	val = recv[1];

	 return val;
}

static int ad7913Write(int SPIx, BYTE phy_no, BYTE addr, BYTE value)
{
     BYTE send[2] = {0};
	 BYTE recv[2] = {0};

	 send[0] = (addr << 3) | AD7913_WRITE;
	 send[1] = value;

#if 1
	 SPITrans_BAddr(SPIx, phy_no, send, recv, 2);
#else
	 SPISend_BAddr(send, 2);
#endif

	 return OK;
}

static void ad7913Init(void)
{
	 int val = -1;
	 DWORD count = 0;

	 SPI_init(1);

	 printf("ad7913Init 1 \n");
	 do
	 {
	      val = ad7913Read(0, 1, AD7913_STAUTS0, 1);
		  printf( "count %d val %d \n", count,val);
		  count++;
		  if (count > 2000)
		  {
		      printf( "AD7913 read error! \n");
			  return;
		  }
	 }while (val & 0x01);

	 printf("ad7913Init 2 \n");
}

int main()
{
	//rtc_test();

	//temperature_and_humidity_test();

	//optical_power_test();

	//ttyUl_test();

	ad7913Init();

	exit(EXIT_SUCCESS);

    return 0;
}
