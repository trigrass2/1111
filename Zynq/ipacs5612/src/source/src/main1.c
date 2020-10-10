/*------------------------------------------------------------------------
 Module:       	main.cpp
 Author:        helen
 Project:       
 State:			
 Creation Date:	2019-05-28
 Description:  for config app 
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include <fcntl.h>
#include <sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef int       STATUS;
typedef char      BOOL;
#define OK         0
#define ERROR     -1
#define TRUE		1
#define FALSE		0

#define MAX_BYTES            		256
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


int Wi2c_write(int fd,
		 unsigned int addr,
		 unsigned int offset,
		 unsigned char *buf,
		 unsigned int len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;
	int i;
	char _buf[MAX_BYTES+2];

	if(len>MAX_BYTES)
	{
		return -1;
	}

	if(offset+len>256)
	{
		return -1;
	}

	_buf[0]=(offset >> 8) & 0xff;
	_buf[1]= offset & 0xff;
	printf("Wi2c_write  %d %d \n",_buf[0],_buf[1]);
	for(i=0;i<len;i++)
	{
	    _buf[2+i]=buf[i];
	    printf("----_writedata:%x------\n",_buf[2+i]);
	}
	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  = addr;
	i2cmsg.flags = 0;
	i2cmsg.len   = 2+len;
	i2cmsg.buf   = _buf;

	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
		perror("ioctl()");
		fprintf(stderr,"ioctl returned %d\n",i);
		return -1;
	}

	return 0;
}

int Wi2c_read(int fd,
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

	if(Wi2c_write(fd,addr,offset,NULL,0)<0)
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

void rtctest()
{
	int fd,i;
	char i2cBusName[0x20] = {0};
	unsigned char readbuf[100];

	sprintf(i2cBusName, "%s%d", DEFAULT_I2C_BUS_PREFIX, 0);
	printf(" i2cBusName %s \n",i2cBusName);
	fd =open(i2cBusName, O_RDWR);

	
	i2c_read(fd,
		0x32,
		0,
		readbuf,
		7);

	for( i = 0; i < 7; i++)
	{
		printf( "%02X ",readbuf[i]);
	}
	
	printf("\r\n");
	
	close(fd);
}

void nvamtest()
{
	int fd,i;
	char i2cBusName[0x20] = {0};
	unsigned char readbuf[100];

	sprintf(i2cBusName, "%s%d", DEFAULT_I2C_BUS_PREFIX, 0);
	printf(" i2cBusName %s \n",i2cBusName);
	fd =open(i2cBusName, O_RDWR);

	for( i = 0; i < 100; i++)
	{
		readbuf[i] =   0xff - (i & 0xff);
	}
	
	Wi2c_write(fd,
		0x50,
		100,
		readbuf,
		10);
	
	memset(readbuf,0,100);
	
	Wi2c_read(fd,
		0x50,
		100,
		readbuf,
		10);

	for( i = 0; i < 10; i++)
	{
		printf( "%02X ",readbuf[i]);
	}
	
	printf("\r\n");
	
	close(fd);
}

#include <linux/spi/spidev.h>

#define  devicename  "/dev/spidev0.%d"  //设备名

static BYTE mode;
static BYTE bits = 8;
static DWORD speed = 1200000;
static WORD delay;
int SC1161Y_fd = -1;
int AD7913_fd = -1;

#define   AD7913_STAUTS0       0x09
#define   AD7913_READ          0x04
#define   AD7913_WRITE         0x00

void SPI_init(int no)
{
	int ret = 0;
	int fd = 0;
	char name[64];

	sprintf(name, devicename,no);
	printf("SPI_init name %s \n",name);
	
	fd = open(name, O_RDWR);       //打开"/dev/spidev0.0"
	if (fd< 0)
		printf("can't open device");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode); //SPI模式设置可读
	if (ret == -1)
		printf("can't get spi mode");

	mode |= SPI_CPHA;
	mode |= SPI_CPOL;
	
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);  //SPI模式设置可写
	if (ret == -1)
		printf("can't set spi mode");
	
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode); //SPI模式设置可读
	if (ret == -1)
		printf("can't get spi mode");	
	
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);  //SPI的bit/word设置可写
	if (ret == -1)
		printf("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);   //SPI的bit/word设置可读
	if (ret == -1)
		printf("can't get bits per word");

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);     //SPI的波特率设置可写
	if (ret == -1)
		printf("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);   //SPI的波特率设置可读
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
	     .tx_buf = (unsigned long)send,   //定义发送缓冲区指针
	     .rx_buf = (unsigned long)recv,   //定义接收缓冲区指针
	     .len = len,         

	};

	if(phy_no == 0)
		fd = SC1161Y_fd;
	else
		fd = AD7913_fd;
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);//执行spidev.c中ioctl的default进行数据传输

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


int main(void)
{
	rtctest();
	nvamtest();
	ad7913Init();

	//esam();

}
