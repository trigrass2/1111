#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <linux/spi/spidev.h>

#include "os.h"
#include "spi.h"


#define  devicename  "/dev/spidev0.%d"  //设备名

int SC1161Y_fd = -1;
int AD7913_fd = -1;

void SPIInit(int no, int baud, int bytes)
{
	int ret = 0;
	int fd = 0;
	BYTE mode,lsb;
	
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
	
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bytes);  //SPI的bit/word设置可写
	if (ret == -1)
		printf("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bytes);   //SPI的bit/word设置可读
	if (ret == -1)
		printf("can't get bits per word");

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &baud);     //SPI的波特率设置可写
	if (ret == -1)
		printf("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &baud);   //SPI的波特率设置可读
	if (ret == -1)
		printf("can't get max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb);   //SPI的波特率设置可读
	if (ret == -1)
		printf("can't get max speed hz");
	
	printf("spi mode: %d\n", mode);
	printf("bits per word: %d  %d \n", bytes,lsb);
	printf("max speed: %d Hz (%d KHz)\n", baud, baud/1000);

	if(no == 0)
		SC1161Y_fd = fd;
	else
		AD7913_fd = fd;
}

int SPITrans_BAddr(int no,BYTE phy_no,BYTE *send,BYTE *recv,int len)
{
	int ret;
	int i = 0;
	int fd;
	struct spi_ioc_transfer tr = {
	     .tx_buf = (unsigned long)send,   //定义发送缓冲区指针
	     .rx_buf = (unsigned long)recv,   //定义接收缓冲区指针
	     .len = len,         

	};

	if(no == 0)
		fd = SC1161Y_fd;
	else
		fd = AD7913_fd;
	
	if(fd < 0)
		printf("SPITrans_BAddr %d error \n",no);
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);//执行spidev.c中ioctl的default进行数据传输

	if (ret < 1)
	{
		printf("can't send spi message \n");
		return FALSE;
	}
	
	return TRUE;
}

