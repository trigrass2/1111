#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "syscfg.h"

#include "i2c.h"
#include "os.h"

#define DEFAULT_I2C_BUS_PREFIX      "/dev/i2c-"
#define MAX_OPTICAL_POWER_CHAN_CNT 	8
#define OPTICAL_POWER_I2C_ADDR		0x51
#define OPTICAL_TX_POWER_REG		102
#define OPTICAL_RX_POWER_REG		104

#define MAX_BYTES            		(16*1024)
BYTE spi_buf[MAX_BYTES];

DWORD g_i2c_sem[4] = {0};
int fd_i2c[4] = {-1};

void I2CInit(int no)
{
	char i2cBusName[0x20] = {0};
	
	g_i2c_sem[no] = smMCreate();
	
	sprintf(i2cBusName, "%s%d", DEFAULT_I2C_BUS_PREFIX, no);
	fd_i2c[no] = open(i2cBusName, O_RDWR);
	if(fd_i2c[no] < 0)
	{
		printf("I2CInit %d open error \n",no);
	}
}

int i2c_write_byte(int fd,
		 BYTE addr,
		 BYTE offset,
		 BYTE *buf,
		 DWORD len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;
	int i;

	if(len>MAX_BYTES)
	{
		return -1;
	}

	if(offset+len>MAX_BYTES)
	{
		return -1;
	}

	spi_buf[0]=offset;
	
	for(i=0;i<len;i++)
	{
	    spi_buf[1+i]=buf[i];
	}

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  = addr;
	i2cmsg.flags = 0;
	i2cmsg.len   = 1+len;
	i2cmsg.buf   = spi_buf;

	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
		perror("ioctl()");
		fprintf(stderr,"ioctl returned %d\n",i);
		return -1;
	}

	return 0;
}

int i2c_read_byte(int fd,
			BYTE addr,
			BYTE offset,
			BYTE *buf,
			DWORD len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;
	int i;

	if(len>MAX_BYTES)
	{
		return -1;
	}

	if(i2c_write_byte(fd,addr,offset,NULL,0)<0)
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

int i2c_write_word(int fd,
		 BYTE addr,
		 WORD offset,
		 BYTE *buf,
		 DWORD len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;
	int i;

	if(len>MAX_BYTES)
	{
		return -1;
	}

	if(offset+len>MAX_BYTES)
	{
		return -1;
	}

	spi_buf[0]=(offset >> 8) & 0xff;
	spi_buf[1]= offset & 0xff;
	
	for(i=0;i<len;i++)
	{
	    spi_buf[2+i]=buf[i];
	}

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  = addr;
	i2cmsg.flags = 0;
	i2cmsg.len   = 2+len;
	i2cmsg.buf   = spi_buf;
	
	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
		perror("ioctl()");
		fprintf(stderr,"ioctl returned %d\n",i);
		return -1;
	}

	return 0;
}

int i2c_read_word(int fd,
			BYTE addr,
			WORD offset,
			BYTE *buf,
			DWORD len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;
	int i;

	if(len>MAX_BYTES)
	{
		return -1;
	}

	if(i2c_write_word(fd,addr,offset,NULL,0)<0)
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


int I2CSendBuffer_BAddr(int no,BYTE *buf, BYTE id, BYTE addr, DWORD len)
{
	int retval; 

	if (buf == NULL) 
	{ 
		printf("i2c bus write error ! \r\n"); 
		return -1; 
	} 
	if(fd_i2c[no] < 0)
		return -1;
	id = id/2;
	if (g_i2c_sem[no]) smMTake(g_i2c_sem[no]);
	retval = i2c_write_byte(fd_i2c[no], id, addr, buf, len);
	if (g_i2c_sem[no]) smMGive(g_i2c_sem[no]);

	return retval; 
}

int I2CReceBuffer_BAddr(int no,BYTE *buf, BYTE id, BYTE addr, DWORD len) 
{ 
	int retval; 

	if (buf == NULL) 
	{ 
		printf("i2c bus write error ! \r\n"); 
		return -1; 
	} 
	if(fd_i2c[no] < 0)
		return -1;
	id = id/2;
	if (g_i2c_sem[no]) smMTake(g_i2c_sem[no]);
	retval = i2c_read_byte(fd_i2c[no], id, addr, buf, len);
	if (g_i2c_sem[no]) smMGive(g_i2c_sem[no]);

	return retval; 
} 

int I2CSendBuffer_WAddr(int no,BYTE *buf, BYTE id, WORD addr, DWORD len)
{
	int retval; 

	if (buf == NULL) 
	{ 
		printf("i2c bus write error ! \r\n"); 
		return -1; 
	} 
	if(fd_i2c[no] < 0)
		return -1;
	id = id/2;
	if (g_i2c_sem[no]) smMTake(g_i2c_sem[no]);
	retval = i2c_write_word(fd_i2c[no], id, addr, buf, len);
	if (g_i2c_sem[no]) smMGive(g_i2c_sem[no]);

	return retval; 
}

int I2CReceBuffer_WAddr(int no,BYTE *buf, BYTE id, WORD addr, DWORD len)
{
	int retval; 

	if (buf == NULL) 
	{ 
		printf("i2c bus write error ! \r\n"); 
		return -1; 
	} 
	if(fd_i2c[no] < 0)
		return -1;
	id = id/2;
	if (g_i2c_sem[no]) smMTake(g_i2c_sem[no]);
	retval = i2c_read_word(fd_i2c[no], id, addr, buf, len);
	if (g_i2c_sem[no]) smMGive(g_i2c_sem[no]);

	return retval; 
}


