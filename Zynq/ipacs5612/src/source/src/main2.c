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
#include "sys.h"
#include "protocol.h"
#include "fileext.h"
#include "yx.h"
#include "tcp.h"
#include  "dbfrz.h"
#ifdef INCLUDE_HIS_TTU
#include "dbhis_ttu.h"
#endif

extern void database(int thid);
extern void misc(void);
extern void comm(void);
#ifdef INCLUDE_PB_RELATE
extern void pb(int thid);
#endif
extern int recordInit(WORD wTaskID);

void myclock(void);

static struct VThSetupInfo ThSetupInfo[] = 
{
	{CLOCK_ID,      	"tClock",    	1,  					COMMONSTACKSIZE,      0,	(ENTRYPTR)myclock,		},
	{COMM_ID,       	"tComm",     	(USR_TASK_PRIO+3),		COMMONSTACKSIZE,      0,	(ENTRYPTR)comm,			},
	{COMM_CLIENT_ID,	"tClient",	 	(USR_TASK_PRIO+4),		CLIENTSTACKSIZE,	  0,	(ENTRYPTR)Client,		},
	{COMM_SERVER_ID,	"tServer",		(USR_TASK_PRIO+5),		SERVERSTACKSIZE,	  0,	(ENTRYPTR)Server,		},
	{COMM_UDP_SERVER_ID,"tUdpServer",	(USR_TASK_PRIO+7),      SERVERSTACKSIZE,	  0,	(ENTRYPTR)Udp_Server,   },
	{DB_ID,             "tDb",          (USR_TASK_PRIO+6),      COMMONSTACKSIZE,      10,   (ENTRYPTR)database,  },
	{SELF_DIO_ID,       "tDio",         (USR_TASK_PRIO+0),      COMMONSTACKSIZE,      10,   (ENTRYPTR)dio,       },
	{SELF_MEA_ID,       "tMea",         (USR_TASK_PRIO+14),     (COMMONSTACKSIZE*2),  0,    (ENTRYPTR)mea,       },
	{MISC_ID,		    "tMisc",	    (USR_TASK_PRIO+46),     COMMONSTACKSIZE,	   0,	(ENTRYPTR)misc, 	 },
#ifdef INCLUDE_PB_RELATE
	{PB_ID,         	"tPb",			(USR_TASK_PRIO+45),		COMMONSTACKSIZE,      0,	(ENTRYPTR)pb			},
#endif
#ifdef INCLUDE_B2F
	{B2F_ID,		    "tB2F", 	    (USR_TASK_PRIO+46),     (COMMONSTACKSIZE*2),  5,    (ENTRYPTR)buf2file,  }, 
#endif

#ifdef INCLUDE_HIS_TTU
	{HIS_ID, 			"tDbhis",		(USR_TASK_PRIO+46),		COMMONSTACKSIZE, 	  0,	(ENTRYPTR)histtu2file	},
#endif
#ifdef INCLUDE_FRZ
	{FRZ_ID,     		"freeze",		(USR_TASK_PRIO+8),		COMMONSTACKSIZE,      10,	(ENTRYPTR)freeze,  		},
#endif
	{-1,            	"",          	0,                   	0,                    0,	NULL,            		},
};

void myclock(void)
{
	int thid,i;
	DWORD events;
	VTimer *ptm;
	extern VThread *g_Thread;
	
	//定时器启动，最小10ms，所有线程定期器均由此触发
	//tmEvEverySet(CLOCK_ID, 1, EV_TM1);  //pre 10ms 
	
	for(;;) 
	{	
		//evReceive(CLOCK_ID, EV_TM1,&events); 	
		//if(events & EV_TM1)
		{
		    g_PthreadTicks++;
			/*if(g_PthreadTicks > 1000)
			{
				g_PthreadTicks = 0;
				printf("tmEvEverySet 100s \n");
			}*/
			for (thid=CLOCK_ID+1; thid<THREAD_MAX_NUM; thid++)
			{
				if(g_Thread[thid].active)
				{
					ptm = g_Thread[thid].tm;
					for(i = 0;i < TIMER_MAX_NUM;i++)
					{
						if (ptm->used == 0) break;
						ptm->ticks--;
						if(ptm->ticks == 0)
						{
							ptm->ticks = ptm->intval;
							evSend(ptm->thid,ptm->event);
						}					
						ptm++;
					}
				}
			}
		}
		usleep(10000);
	}
}

void ApplicationInit()
{
	PublicInit();
	
	NVRamInit();

	ClockInit();
		
	SpecialTaskSetup(ThSetupInfo,CLOCK_ID);
	
	SysParaInit();

#ifdef INCLUDE_B2F
		buf2fileInit();
#endif

	PubPollAndTableSetup();
	
	SelfPara_Init1();

	ResetShow();
	
	AddrInit();
	
	commInit();
	
#ifdef INCLUDE_NET
    SpecialTaskSetup(ThSetupInfo,COMM_CLIENT_ID);
    SpecialTaskSetup(ThSetupInfo,COMM_SERVER_ID);
    SpecialTaskSetup(ThSetupInfo,COMM_UDP_SERVER_ID);
#endif	

	SpecialTaskSetup(ThSetupInfo,COMM_ID);

	SysConfigInit();

	SetIoNo();
	
    //初始化实设备后到database注册,同时注册遥测遥信遥控信息。其他app都只读即可
	if (GetDevInfo()==OK)
	{
		myprintf(SYS_ID, LOG_ATTR_INFO, "Device.cfg init ok.");
		myprintf(SYS_ID, LOG_ATTR_INFO, "System total device num is %d.",*g_Sys.Eqp.pwNum);
		DevInit();
	}
	else
		myprintf(SYS_ID, LOG_ATTR_INFO, "Device.cfg invalid or not exist!");

	ioInit();

#ifdef INCLUDE_RECORD	
	recordInit(B2F_ID);
#endif	

	SpecialTaskSetup(ThSetupInfo,DB_ID);
	SpecialTaskSetup(ThSetupInfo,SELF_DIO_ID);
	SpecialTaskSetup(ThSetupInfo,SELF_MEA_ID);
	SpecialTaskSetup(ThSetupInfo,MISC_ID);	

#ifdef INCLUDE_PB_RELATE
	PbInit();
	SpecialTaskSetup(ThSetupInfo,PB_ID);
#endif  	

	TaskSetup();  

#ifdef INCLUDE_HIS_TTU
	his_ttu_init();
	SpecialTaskSetup(ThSetupInfo,HIS_ID);
#endif

#ifdef INCLUDE_HIS
// xml 文件初始化
	his_init();
#endif

#ifdef INCLUDE_B2F
	SpecialTaskSetup(ThSetupInfo,B2F_ID);
#endif	

	SelfPara_Init();	

}

extern void PowerOn_Init_McInnerComm(void);
extern void PowerOn_Init_SharedMemory(void);
extern  void BMinit(void);
extern void BMWritePrSetTable(void);
extern int WAV_Record_Scan(void);
extern void mc_Print_from_BM(void);


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
	i2cmsg.len   = 2;
	i2cmsg.buf   = _buf;

	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
		perror("ioctl()");
		fprintf(stderr,"ioctl returned %d\n",i);
		return -1;
	}

	i2cmsg.addr  = addr;
	i2cmsg.flags = 0;
	i2cmsg.len   = len;
	i2cmsg.buf   = buf;

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
		readbuf[i] =   200 - (i & 0xff);
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
static BYTE lsb;
static DWORD speed = 1200000;
static WORD delay;
int SC1161Y_fd = -1;
int AD7913_fd = -1;

#define   AD7913_STAUTS0       0x09
#define   AD7913_STAUTS1       0x0F

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

	ret = ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb);   //SPI的波特率设置可读
	if (ret == -1)
		printf("can't get max speed hz");
	
	printf("spi mode: %d\n", mode);
	printf("bits per word: %d  %d \n", bits,lsb);
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

int SPITrans_BAddr_Jiami(BYTE *send,BYTE *recv,int len,BOOL on,BOOL off)
{
	DWORD i,j;
	BYTE *pBuf1,*pBuf2;
	int bret = 0;
	memcpy(recv,send,len);

	usleep(200);

 	bret = SPITrans_BAddr(0, 0,send,recv,len);
	if(bret==FALSE)
		return ERROR;

	usleep(200);

	return OK;

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
		  //val = ad7913Read(0, 1, AD7913_STAUTS1, 1);
		  //printf( "AD7913_STAUTS1 val %d \n",val);
	 }while (val & 0x01);

	 printf("ad7913Init 2 \n");
}

BYTE sendbuff[1000] = {0};
BYTE recvbuff[1000] = {0};
BYTE cmdbuffer[1000] = {0};

void jiamidelay_us(WORD us)
{
    int i;
    //for(i=0; i<(us*28); i++);
   // for(i=0; i<(us*84); i++);
   for(i=0; i<(us*100); i++);
}

void JiamiReadRegs(BYTE bAddr, BYTE *pBuffer, WORD bCount,BOOL on,BOOL off)
{	
    int i;
	
    sendbuff[0] = bAddr;

    SPITrans_BAddr_Jiami( sendbuff, recvbuff, bCount,on,off);
    for(i=0;i<bCount;i++)
	   *pBuffer++ = recvbuff[i];
}

BYTE JiamiReadReg(BYTE bAddr,BOOL on,BOOL off)
{
    sendbuff[0] = bAddr;
	recvbuff[1] = 10;
 	SPITrans_BAddr_Jiami(sendbuff, recvbuff, 2,on,off);
    return recvbuff[1];
}

WORD ecrProcData1(BYTE* cmd, BYTE cmdLen, BYTE* sBuf,
                     WORD sLen, BYTE* rBuf, WORD* rLen)
{
    BYTE data, resp[4];
    BYTE lrc1, lrc2;
    WORD i, dataLen;
	WORD sendcnt = 0;

    if (cmd == NULL || rBuf == NULL) 
	{
        return 0xffff;
    }

    /* 1 send data */
    /* 1.1 caculate lrc */
    lrc1 = 0;
    for (i = 0; i < cmdLen; i++)
	{
        lrc1 ^= cmd[i];
    }
    if (sBuf != NULL && sLen > 0)
	{
	    for (i = 0; i < sLen; i++)
		{
	        lrc1 ^= sBuf[i];
	    }

    }
    lrc1 = ~lrc1;
	
    /* 1.2 send sof */
  	//JiamiWriteCtrl(0x55);
  	sendbuff[0] = 0x55;
	sendcnt += 1;
    /* 1.3 send cmd and data */
    for (i = 0; i < cmdLen; i++) 
	{
        //JiamiWriteCtrl(cmd[i]);
        sendbuff[sendcnt++] = cmd[i];
    }
    for (i = 0; i < sLen; i++)
	{
       // JiamiWriteCtrl(sBuf[i]);
       sendbuff[sendcnt++] = sBuf[i];
    }

    /* 1.4 send lrc */
    //JiamiWriteCtrl(lrc1);
	sendbuff[sendcnt++] = lrc1;
 	SPITrans_BAddr_Jiami(sendbuff,recvbuff, sendcnt,1,1);

    /* 2.1 get busy state */
    i = 0;
	data = JiamiReadReg(0xaa,1,0);

	if(data != 0x55)
	{
	    while (1) 
		{
	        data = JiamiReadReg(0xaa,0,0);

	       	if (data == 0x55)
			{
	          	 break;
	        }

	        //if over 3s return false;
			i++;
			if(i>1000)
			{
				data = JiamiReadReg(0xaa,0,1);
				return 0xFFFF;
			}
			else 
				jiamidelay_us(100);//thSleep(1);
	    }
	}

    /* 2.2 get resp */

	JiamiReadRegs(0xf8, resp, 4,0,0);
 

    dataLen = (WORD)resp[2] * 256 + resp[3];
    *rLen = dataLen;

	if(dataLen > 999)
	{
		*rLen = dataLen = 1;
		 JiamiReadRegs(0x00, rBuf, (dataLen+1),0,1);
		  return 0xffff;
	}
    /* 2.3 get data and lrc */
	 JiamiReadRegs(0x00, rBuf, (dataLen+1),0,1);

    /* 2.4 check lrc */
    lrc2 = 0;
    lrc2 = resp[0] ^ resp[1] ^ resp[2] ^ resp[3];
    for (i = 0; i < *rLen; i++) 
	{
        lrc2 ^= rBuf[i];
    }
    lrc2 = ~lrc2;
		//JIAMI_CSN_L;
    if (lrc2 != rBuf[*rLen]) 
	{
        printf("lrc err\n\r");
        return 0xffff;
    }   

    /* 3 return sw code */
    return ((WORD)resp[0] * 256 + resp[1]);
}

WORD ecrProcData(BYTE* cmd, BYTE cmdLen, BYTE* sBuf,
                     WORD sLen, BYTE* rBuf, WORD* rLen)
{
	WORD i,sw;
	if((sBuf!=NULL)||(sLen != 0))
		memcpy(cmdbuffer,sBuf,sLen);
	for(i = 0; i < 4;i++)
	{
		memcpy(sBuf,cmdbuffer,sLen);
		sw = ecrProcData1(cmd,cmdLen,sBuf,sLen,rBuf,rLen);
		printf("sw =%x\n",sw);
		if(0x9000 == sw)
		{
			break;
		}
	}
	return sw;
}



void esam(void)
{
	BYTE recvbuf[64], i;
	BYTE chipSerCmd[8];// = {0x00, 0xb0, 0x99, 0x05, 0x00, 0x08};
	WORD recvlen, sw;
	
	SPI_init(0);

	chipSerCmd[0] = 0;
	chipSerCmd[1] = 0xb0;
	chipSerCmd[2] = 0x99;
	chipSerCmd[3] = 0x5;
	chipSerCmd[4] = 0;
	chipSerCmd[5] = 0x2;
	chipSerCmd[6] = 0;
	chipSerCmd[7] = 8;
	
    sw = ecrProcData(chipSerCmd, sizeof(chipSerCmd), NULL, 0, recvbuf, &recvlen); //sc1161Y

    if (sw == 0x9000) 
	{
        shellprintf("Init Encrypt Chip Sucessed\n\r");
        shellprintf("\n\rEncrypt Chip Serial Num: ");
        for (i = 0; i < 10; i++)
		{
            shellprintf("%02x ", recvbuf[i]);
    	}
		shellprintf("\n\r");
        return TRUE;
    }
}

int main(void)
{
	rtctest();
	nvamtest();
	ad7913Init();

	//esam();
#if 0
	PowerOn_Init_SharedMemory();
	PowerOn_Init_McInnerComm();
	
	ThreadInit("jc");
	ApplicationInit();
	prGzEventInit();
	BMinit();
	
	for(;;)
	{
		sleep(1);
		BMWritePrSetTable();
#ifdef INCLUDE_RECORD	
		WAV_Record_Scan();
#endif
		WriteBmEvent();
		mc_Print_from_BM();
	}
#endif
}
