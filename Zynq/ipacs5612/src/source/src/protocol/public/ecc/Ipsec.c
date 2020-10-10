#include "syscfg.h"
#include "bsp.h"
#include "spi.h"
#include "sgc1126a.h"
#include "sys.h"
#include "os.h"
#include "comm.h"
#include "Ipsec.h"

#ifdef INCLUDE_SEC_CHIP

VIpsecChan IpsecChan;

static DWORD IpsecCosMSem; //cos信号量保护，一旦发送了cos则必须等待返回才行。
static DWORD IpsecSendMSem; //send发送信号量保护
void IPSecNetSetClient(void);
void IPSecNetSetPort(void);
void IPSecNetSetClientRev(void);
void IPSecCloseClientSocket(void);

BYTE* spirxtempbuffer = NULL;

int g_ipsecinit = 0;
int g_ipsecrst = 0;
//int g_ipsecready = 0;
int g_ipsecparaset = 0;
BYTE temp[IPSEC_BUFFER_SIZE];
#if (TYPE_CPU == CPU_STM32F4)

/////M4引脚待定////
#define ipsecrst_pin         GPIO_Pin_6 // 复位引脚拉低多少时间
#define ipsecrst_gpio        GPIOF      //
#define ipsecint_pin         GPIO_Pin_9 // 终端脚
#define ipsecint_gpio        GPIOH   
#define IPSECSPI   SPI2
EXTI_InitTypeDef EXTI_InitStructure;

#elif (TYPE_CPU == CPU_SAM9X25)

#define IPSECSPI   SPI1
#define ipsecrst_pin         GPIO_Pin_7 // 复位引脚拉低多少时间
#define ipsecrst_gpio        GPIOC      //
#define ipsecint_pin         GPIO_Pin_5 // 终端脚
#define ipsecint_gpio        GPIOC   
#define irq_int_pio          (5 + 3*32) //中断号
#endif

#define read_int_state READ_GPIO(ipsecint_gpio,ipsecint_pin)




BYTE Lrc(BYTE* buf,int len)
{
	int i;
	BYTE lrc = 0;
	for(i = 0; i < len; i++)
	{
		lrc ^= buf[i];
	}
	lrc = ~lrc;
	return lrc;
}

//spi 接收数据区
void SendByteToRxBuffer(BYTE rx_data)
{
	IpsecChan.rxbuf.buf[IpsecChan.rxbuf.wp] = rx_data;
	IpsecChan.rxbuf.wp = (IpsecChan.rxbuf.wp + 1) & (IPSEC_BUFFER_SIZE - 1);
}
//从spi接收数据区取数
BYTE GetByteFromRxBuffer()
{
	BYTE buffer = IpsecChan.rxbuf.buf[IpsecChan.rxbuf.rp];
	IpsecChan.rxbuf.rp = (IpsecChan.rxbuf.rp + 1) & (IPSEC_BUFFER_SIZE - 1);
	return buffer;
}

// spi 发送驱动 发送,将缓存区数据发送
void SPIM_TxRx()
{
	BYTE rx_data,tx_data;
	DWORD gpio_state; 
	//disable 引脚中断
#if (TYPE_CPU == CPU_STM32F4)
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	stm32EXTIInit(&EXTI_InitStructure);
#elif (TYPE_CPU == CPU_SAM9X25)
	rt_hw_interrupt_mask(irq_int_pio); 
#endif
	
	while(1)
	{
		gpio_state =  read_int_state;
		if((gpio_state == 0) && (IpsecChan.txbuf.rp == IpsecChan.txbuf.wp))
			break;
		
		if(IpsecChan.txbuf.rp != IpsecChan.txbuf.wp)
		{
			tx_data = IpsecChan.txbuf.buf [IpsecChan.txbuf.rp];
			IpsecChan.txbuf.rp = (IpsecChan.txbuf.rp + 1) & (IPSEC_BUFFER_SIZE - 1);
		}
		else
			tx_data = 0x00;
					
		SPITrans_BAddr( IPSECSPI, 0, &tx_data, &rx_data,1);
		if(gpio_state)
		{
			SendByteToRxBuffer(rx_data);
			shellprintf("%02x ",rx_data);
		}
	}
// 使能引脚驱动
#if (TYPE_CPU == CPU_STM32F4)
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	stm32EXTIInit(&EXTI_InitStructure);
#elif (TYPE_CPU == CPU_SAM9X25)
	rt_hw_interrupt_umask(irq_int_pio); 
#endif
}

#if (TYPE_CPU == CPU_SAM9X25)
// 中断接收 驱动
//int time = 0;
//DWORD t1[100],t2[100];
//DWORD count[100] = {0};
//添加防止死循环的情况
void EXT_IPSecIRQHandler(int vector, void *param)
{
	//spi处理
	BYTE tx_data,rx_data;
	WORD num = 0;
	DWORD gpio_state = 1;
	tx_data = 0x00;
	
//	t1[time] = Get100usCnt();
	while(gpio_state)
	{
//		count[time]++;
		SPITrans_BAddr( IPSECSPI, 0, &tx_data, &rx_data,1);
		SendByteToRxBuffer(rx_data);
		gpio_state = read_int_state;
		if(++num	> 2048)
			break;
	}
	if(num > 2048)
		WriteDoEvent(NULL, 0,"spi 接收过多2048 ");
//	t2[time] = Get100usCnt();
//	time++;
//测试
//	SendByteToRxBuffer(0xAA);
//	SendByteToRxBuffer(0xC1);
//	SendByteToRxBuffer(0x01);
//	SendByteToRxBuffer(0x00);
//	SendByteToRxBuffer(0x02);
//	SendByteToRxBuffer(0x3D);
//	SendByteToRxBuffer(0x00);
//	SendByteToRxBuffer(0x00);
//	SendByteToRxBuffer(0xFF);
}
#else
void EXT_IPSecIRQHandler()
{
	//spi处理
	BYTE tx_data,rx_data;
	DWORD gpio_state = 1;
	tx_data = 0x00;

	while(gpio_state)
	{
		SPITrans_BAddr( IPSECSPI, 0, &tx_data, &rx_data,1);
		SendByteToRxBuffer(rx_data);
		gpio_state = read_int_state;
	}
//测试
//	SendByteToRxBuffer(0xAA);
//	SendByteToRxBuffer(0xC1);
//	SendByteToRxBuffer(0x01);
//	SendByteToRxBuffer(0x00);
//	SendByteToRxBuffer(0x02);
//	SendByteToRxBuffer(0x3D);
//	SendByteToRxBuffer(0x00);
//	SendByteToRxBuffer(0x00);
//	SendByteToRxBuffer(0xFF);
}
#endif

BOOL SPIM_ProcessDataSip(BYTE* buffer,WORD len)
{
	BOOL ret = FALSE;
	BYTE ip[5] = {0};
	BYTE mask[5] = {0};
	BYTE gateway[5] = {0};
	int i;
	if((len == 2) && (buffer[0] == 0x00) && (buffer[1] == 0x00))
	{
		shellprintf("设置源IP、掩码、网关成功 \n");
		IpsecChan.int_no = READDIP;
		g_ipsecrst = 1;
		ret = TRUE;
	}
	else if(len == 12)
	{
		memcpy(ip,buffer,4);
		memcpy(mask,buffer+4,4);
		memcpy(gateway,buffer+8,4);
		shellprintf(" 读取源IP为 %d:%d:%d:%d,掩码为 %d:%d:%d:%d,网关为%d:%d:%d:%d \n",ip[3],ip[2],ip[1],ip[0],
		mask[3],mask[2],mask[1],mask[0],gateway[3],gateway[2],gateway[1],gateway[0]);
		ret = TRUE;
		for(i = 0; i < 12; i++)
		{
			if(buffer[i] != IpsecChan.netpara.srip[i])
			{
				ret = FALSE;
				shellprintf("读取的源IP与设置的不一致 \n");
			}
		}
		if(ret == TRUE)
			IpsecChan.int_no = READDIP;
		else
			IpsecChan.int_no = SETSIP;
	}
	else
	{
		shellprintf("设置读取源IP、掩码、网关失败 \n");
	}
	return ret;
}

BOOL SPIM_ProcessDataDip(BYTE* buffer,WORD len)
{
	BOOL ret = FALSE;
	BYTE ip[5] = {0};
	BYTE mask[5] = {0};
	BYTE gateway[5] = {0};
	int i;
	if((len == 2) && (buffer[0] == 0x00) && (buffer[1] == 0x00))
	{
		shellprintf("设置目标IP、掩码、网关成功 \n");
		ret = TRUE;
		g_ipsecrst = 1;
		IpsecChan.int_no = READPRTL;
	}
	else if(len == 12)
	{
		memcpy(ip,buffer,4);
		memcpy(mask,buffer+4,4);
		memcpy(gateway,buffer+8,4);
		shellprintf(" 读取目标IP为 %d:%d:%d:%d,掩码为 %d:%d:%d:%d,网关为%d:%d:%d:%d \n",ip[3],ip[2],ip[1],ip[0],
		mask[3],mask[2],mask[1],mask[0],gateway[3],gateway[2],gateway[1],gateway[0]);
		ret = TRUE;
		for(i = 0; i < 12;i++)
		{
			if(buffer[i] != IpsecChan.netpara.drip[i])
			{
				ret = FALSE;
				shellprintf("读取的目的IP等与设置的不一样 \n");
				break;
			}
		}
		if(ret == TRUE)
			IpsecChan.int_no = READPRTL;
		else
			IpsecChan.int_no = SETDIP;
	}
	else
	{
		shellprintf("设置读取目标IP、掩码、网关失败 \n");
	}
	return ret;
}

BOOL SPIM_ProcessDataSmac(BYTE* buffer,WORD len)
{
	BOOL ret = FALSE;
	BYTE mac[7] = {0};
	int i;
	if((len == 2) && (buffer[0] == 0x00) && (buffer[1] == 0x00))
	{
		shellprintf("设置源MAC 成功\n");
		IpsecChan.int_no = READPRTL;
		g_ipsecrst = 1;
		ret = TRUE;
	}
	else if(len == 6)
	{
		memcpy(mac,buffer,6);
		shellprintf(" 读取源MAC %02X:%02X:%02X:%02X:%02X:%02X \n",mac[0],mac[1],mac[2],mac[3],
		mac[4],mac[5]);
		ret = TRUE;
		for(i = 0;i < 6;i++)
		{
			if(buffer[i] != IpsecChan.netpara.srmac[i])
			{
				ret = FALSE;
				shellprintf("读取的源mac与设置的不一致 \n");
			}
		}
		if(ret == TRUE)
			IpsecChan.int_no = READPRTL;
		else
			IpsecChan.int_no = SETMAC;
	}
	else
	{
		shellprintf("设置读取源MAC失败 \n");
	}
	return ret;
}

BOOL SPIM_ProcessDataPrmode(BYTE* buffer,WORD len)
{
	BOOL ret = FALSE;
	if((len == 2) && (buffer[0] == 0x00) && (buffer[1] == 0x00))
	{
		shellprintf("设置安全芯片模式成功 \n");
		IpsecChan.int_no = READRTC;
		g_ipsecrst = 1;
		ret = TRUE;
	}
	else if(len == 1)
	{
		if(buffer[0] == 00)
			shellprintf("IPSec协议应用模式 \n");
		else if(buffer[0] == 0x01)
			shellprintf("旁路模式 \n");
		ret = TRUE;
		if(buffer[0] != IpsecChan.netpara.Prmode)
		{
			shellprintf("读取的协议模式与设置的不一致 \n");
			ret = FALSE;
		}
		if(ret == TRUE)
			IpsecChan.int_no = READRTC;
		else
			IpsecChan.int_no = SETPRTL;
	}
	else
	{
		shellprintf("设置读取IPSec安全芯片模式失败 \n");
	}
	return ret;
}

BOOL SPIM_ProcessDataRtc(BYTE* buffer,WORD len)
{
	BOOL ret = FALSE;
	int temp;
	struct VSysClock SysTime;
	struct VCalClock CalTime1,CalTime2;
	
	if((len == 2) && (buffer[0] == 0x00) && (buffer[1] == 0x00))
	{
		shellprintf("设置安全芯片时钟成功 \n");
		IpsecChan.int_no = IPSECRST;
		ret = TRUE;
	}
	else if(len == 6)
	{
		shellprintf("读取安全芯片时钟 %d年%d月%d日%d时%d分%d秒 \n",buffer[5],buffer[4],
		buffer[3],buffer[2],buffer[1],buffer[0]);
		
		SysTime.wYear = buffer[5]+2000;
		SysTime.byMonth = buffer[4];
		SysTime.byDay = buffer[3];
		SysTime.byHour = buffer[2];
		SysTime.byMinute = buffer[1];
		SysTime.bySecond = buffer[0];
		SysTime.wMSecond = 0;
		GetSysClock(&CalTime2, CALCLOCK);
		ToCalClock(&SysTime,&CalTime1);
		temp = CalTime2.dwMinute - CalTime1.dwMinute;
		temp = temp*60;
		temp = temp + (CalTime2.wMSecond- CalTime1.wMSecond)/1000;
		
		if(temp > 60||temp < -60)
		{
			ret = FALSE;
			IpsecChan.int_no = SETRTC;
		}
		else
		{
			ret = TRUE;
			shellprintf("芯片rtc时间滞后 %d s \n",temp);
			IpsecChan.int_no = IPSECRST;
		}
	}
	else
	{
		shellprintf("设置读取IPSec安全芯片时钟失败 \n");
	}
	return ret;
}

//无意义，去掉，读连接状态
BOOL SPIM_ProcessDataSta(BYTE* buffer,WORD len)
{
	BOOL ret = FALSE;
	if((len == 2) && (buffer[0] == 0x00) && (buffer[1] == 0x00))
	{
		shellprintf("tcp已连接 \n");
		IpsecChan.status = 1;
		ret = TRUE;
	}
	else
	{
		shellprintf("tcp未连接 \n");
		IpsecChan.status = 0;
	}
	return ret;
}
BOOL SPIM_ProcessDataPhy(BYTE* buffer,WORD len)
{
	int i;
	for(i=0;i<len;i++)
		shellprintf("%02X ",buffer[i]);
	shellprintf("\n");
	return TRUE;
}

//接收 0x55 + sw1 + sw2 + len1 + len2 + data + lrc2
//IpsecChan.cos.buf 不含 0x55与lrc2
void SPIM_ProcessDataCos(BYTE* buffer,WORD len)
{
//	if()
//	{
//		
//	}
	WORD datalen = 0;
  BYTE lrc3 = 0;	
	if(buffer[0] != 0x55)
		shellprintf("接收cos 头不是0x55 \n");	
	if(len < 6)
		shellprintf("接收cos 长度太短 \n");
	datalen  = MAKEWORD(buffer[4],buffer[3]);
	if((datalen + 6) != len)
		shellprintf("接收cos长度不对 \n");
	lrc3 = Lrc(buffer+1,len-2);
	
	if(lrc3 != buffer[len-1])
		shellprintf("接收cos校验不对 \n");
	if(len > 2047)
		shellprintf("接收cos长度过长 \n");
	memcpy(IpsecChan.cos.buf,buffer+1,len-2);
	IpsecChan.cos.len = len-2;
	smMGive(IpsecCosMSem);
}

void spiwritetocomm(BYTE* buffer,WORD len)
{
	VCommBuf *pfifo;
	int i;
	pfifo = &(g_CommChan[IpsecChan.no].rx_buf);
	for(i = 0; i < len; i++)
	{
		pfifo->buf[pfifo->wp] = buffer[i];
		pfifo->wp = (pfifo->wp + 1) & (IPSEC_BUFFER_SIZE - 1);
	}
	evSend(g_CommChan[IpsecChan.no].tid, EV_RX_AVAIL); //告诉规约驱动有数据到规约处理
}

void SPIM_ProcessDataNet(BYTE* buffer,WORD len)
{
  BYTE com,tun;
	WORD port,datalen = 0;
	BYTE ret = TRUE;
	
	com = buffer[0];
	if(len < 3)
	{
		shellprintf("接收长度太短小于3 \n");
		WriteDoEvent(NULL,0,"spi转发网络长度小于3 ");
		return;
	}
	switch(com)
	{
		case 0x00:  // 隧道朔天定义为socket
			if(buffer[1] == 0x00)
				shellprintf("收到发送网络数据的确认 加密隧道%d \n",buffer[2]);
			else
				shellprintf("com == 0x00 接收错误 \n");
		  break;	
		case 0x81: //tcp数据通知   
			tun = buffer[1];
//			if(IpsecChan.netpara.Prmode)
//			{
				IpsecChan.netpara.tun = tun; //杭州朔天不建议设置，由芯片决定
//			}				
//			else
//			{
//				if(tun != IpsecChan.netpara.tun)
//				{
//					shellprintf("发送隧道与接收隧道不一致 \n");
//				}
//			}
		  datalen = MAKEWORD(buffer[3],buffer[2]);
			if((datalen + 4) != len)
				shellprintf("接收tcp长度有误 \n");
			spiwritetocomm(buffer+4, datalen);
			break;
		case 0x83: //udp数据通知
			shellprintf(" 收到udp数据通知,舍弃 \n");
		case 0x85:
			ret = TRUE;
			port = MAKEWORD(buffer[2],buffer[1]);
			if(port != IpsecChan.netpara.port)
			{
				shellprintf("收到的port不一致 \n");
				ret = FALSE;
				return;
			}
			if(buffer[3] == 0x00)
			{
				IpsecChan.netpara.listensocket = buffer[4];
				shellprintf("建立监听socket成功,%d \n",IpsecChan.netpara.listensocket);
				WriteDoEvent(NULL,0,"建立监听socket成功 ");
				ret = TRUE;
			}
			else
			{
				shellprintf("建立监听socket失败\n");
				WriteDoEvent(NULL,0,"建立监听socket失败 ");
			}
				
			if(ret == TRUE)
				IpsecChan.int_no = IPSECSETNUM;
			break;
		case 0x86://收到connetsocket的连接
			if(buffer[1] != IpsecChan.netpara.listensocket)
			{
				shellprintf("tcp连接listen号不对 \n");
				return;
			}
			if(buffer[3] == 0x00)
			{
				IpsecChan.netpara.connectsocket = buffer[2];
				shellprintf("服务器模式tcp已连接 \n");
				WriteDoEvent(NULL,0,"服务器模式tcp已连接 ");
                   IpsecChan.netpara.tun = IpsecChan.netpara.connectsocket;
				IpsecChan.status |= serverconnect;
				evSend(g_CommChan[IpsecChan.no].tid, EV_COMM_STATUS);
			}
		  break;
		case 0x87://tcp连接关闭
			if((buffer[1] == IpsecChan.netpara.connectsocket) && (buffer[2] == 0x00))
			{
				IpsecChan.netpara.connectsocket = 0xff;
				shellprintf("服务器模式与主站连接关闭 \n");
				WriteDoEvent(NULL,0,"服务器模式与主站连接关闭 ");
				IpsecChan.status &= ~serverconnect;
				evSend(g_CommChan[IpsecChan.no].tid, EV_COMM_STATUS);
			}
			else if((buffer[1] == IpsecChan.netpara.clientsocket) && (buffer[2] == 0x00))
			{
				IpsecChan.netpara.clientsocket = 0xff;
				shellprintf("客户端模式与主站连接关闭 \n");
				WriteDoEvent(NULL,0,"客户端模式与主站连接关闭 ");
				IpsecChan.status &= ~clientconnect;
				evSend(g_CommChan[IpsecChan.no].tid, EV_COMM_STATUS);
			}
			else 
			{
				shellprintf("tcp连接关闭错误 \n");
			}
			break;
		case 0x89:
			if(len < 7)
			{
				shellprintf("创建TCP Client连接返回长度太短 \n");
				return;
			}
			if((memcmp(buffer+1, IpsecChan.netpara.drip,4) != 0) && (memcmp(buffer+1, IpsecChan.netpara.driprev,4) != 0))
			{
				ret = FALSE;
				shellprintf("客户端模式返回的目的IP不一致 \n");
				WriteDoEvent(NULL,0,"客户端模式返回的目的IP不一致 ");
			}
			port =  MAKEWORD(buffer[6],buffer[5]);
			if(port != IpsecChan.netpara.port)
			{
				ret = FALSE;
				shellprintf("客户端模式返回的端口号不一致 \n");
				WriteDoEvent(NULL,0,"客户端模式返回的端口号不一致 ");
			}
			if((buffer[7] == 0x00) && ret) //
			{
				IpsecChan.netpara.clientsocket = buffer[8];
				shellprintf("tcp client socket %d \n",buffer[8]);
				WriteDoEvent(NULL,0,"客户端模式与主站连接");
				IpsecChan.netpara.tun = IpsecChan.netpara.clientsocket;
				IpsecChan.int_no = IPSECSETNUM;
				IpsecChan.status |= clientconnect;
			}
			else
			{
				shellprintf("tcp client 未成功 %d \n",buffer[8]);
                   if(memcmp(buffer+1, IpsecChan.netpara.drip,4)  == 0) //
                   {
                        shellprintf("tcp连主通道不成功切备用通道 \n");
				     WriteDoEvent(NULL,0,"tcp连主通道不成功切备用通道");
                       IPSecNetSetClientRev();
                    }
			}
			break;
		default:
			shellprintf("转发网络数据指令未知 com = %d \n",com);
			break;
	}
}


//spi接收数据到规约处理搜索
void SPIM_ReadRxBuffer(void)
{
	int state = START;
	BYTE byte,act1,act2,len1,len2,lrc2,lrc4;
	WORD len,rest_len,act;
	
	while(IpsecChan.rxbuf.rp != IpsecChan.rxbuf.wp) 
	{
		byte = GetByteFromRxBuffer();
		switch(state)
		{
			case START:
				if(byte == 0xAA)
					state = ACT1;
				break;
			case ACT1:
				act1 = byte;
				state = ACT2;
			  break;
			case ACT2:
				act2 = byte;
				state = LEN1;
				act = MAKEWORD(act2,act1);
				break;
			case LEN1:
				len1 = byte;
				state = LEN2;
				break;
			case LEN2:
				len2 = byte;
				state = LRC4;
				len = MAKEWORD(len2,len1);
				break;
			case LRC4:
				lrc4 = ~(act1^act2^len1^len2);
				if(byte == lrc4)
				{
					if(len != 0)
					{
						rest_len = len;
						state = DATA;
					}
					else // 无DATA数据,交付数据包
					{
						//SPIM_Recv();  暂无此情况
						switch(act)
						{
							case 0xC100: // 朔天死机情况
								if(IpsecChan.netpara.tcpmode) //客户端
								{
									IPSecNetSetClient();
								}
								else //服务器
								{
									IPSecNetSetPort();
								}
								break;
							case 0xC001: //朔天就绪指令
//								g_ipsecready = 1;
                                     WriteDoEvent(NULL,0,"朔天芯片就绪");
								break;
						}
						state = START;
					}
				}
				else
				{
					//校验不对，重新搜索
					IpsecChan.rxbuf.rp = (IpsecChan.rxbuf.rp + IPSEC_BUFFER_SIZE -4) & (IPSEC_BUFFER_SIZE - 1);
					state = START;
				}
				break;
			case DATA:
				spirxtempbuffer[len - rest_len] = byte;
				rest_len--;
				if(rest_len == 0)
				{
					state = LRC2;
					lrc2 = Lrc(spirxtempbuffer,len);
				}
				break;
			case LRC2:
				if(byte == lrc2)
				{
					switch(act)
					{
						case ACT_SIP:
							SPIM_ProcessDataSip(spirxtempbuffer,len);
							break;
						case ACT_DIP:
							SPIM_ProcessDataDip(spirxtempbuffer,len);
							break;
						case ACT_SMAC:
							SPIM_ProcessDataSmac(spirxtempbuffer,len);
							break;
						case ACT_PRMODE:
							SPIM_ProcessDataPrmode(spirxtempbuffer,len);
							break;
						case ACT_RTC:
							SPIM_ProcessDataRtc(spirxtempbuffer,len);
							break;
						case 0xC109:
							SPIM_ProcessDataPhy(spirxtempbuffer,len);
							break;
						case ACT_COS:
							SPIM_ProcessDataCos(spirxtempbuffer,len);
							break;
						case ACT_NETDATA:
							SPIM_ProcessDataNet(spirxtempbuffer,len);
							break;
						case ACT_NETSTA:
							SPIM_ProcessDataSta(spirxtempbuffer,len);
							break;
						default:
							break;
					}					
				}
				state = START;
				break;
			default:
				break;
		}
	}
}

//spi 扫描函数，20ms 扫描一次，无接收时才发送

//查3分钟没数则断开连接
void IPSec_Scan()
{
	int i = 0,len=0;
	static DWORD cnt = 0;
	static DWORD reccnt = 0;
	if(!g_ipsecinit)
		return;
	if((IpsecChan.netpara.tcpmode == 1) && g_ipsecparaset && ((IpsecChan.status & clientconnect) == 0))//客户端未连接且
	{
		cnt++;
		if(cnt >= (30*100)/COMM_SCAN_TM) // 30s检测
		{
			cnt = 0;
			IPSecNetSetClient();
		}
	}
	else
		cnt = 0;
  
	if((IpsecChan.netpara.tcpmode == 1) && g_ipsecparaset && (IpsecChan.status & clientconnect))//客户端已连接
	{
		reccnt++;
		if(reccnt >= (150*100)/COMM_SCAN_TM) // 150s检测
		{
			reccnt = 0;
			IPSecCloseClientSocket();
			shellprintf("3分钟未收到数据断开客户端连接 \n");
			WriteDoEvent(NULL,0,"3分钟未收到数据断开客户端连接 ");
		}
	}
	else
		reccnt = 0;
    
	if((IpsecChan.txbuf.rp != IpsecChan.txbuf.wp) && (read_int_state == 0))//有数据才发送,无数据接收时才能发送
	{
		smMTake(IpsecSendMSem);

#ifdef INCLUDE_COMM_SHOW	
		i = IpsecChan.txbuf.rp;
		while(i != IpsecChan.txbuf.wp)
		{
			temp[len++] = IpsecChan.txbuf.buf[i];
			i = (i + 1) & (IPSEC_BUFFER_SIZE - 1);
		}
		commPrint(IpsecChan.no, 4, len, temp);
		commBufFill(IpsecChan.no, 4, len, temp);
#endif		
		SPIM_TxRx();
		smMGive(IpsecSendMSem);
	}
	if((read_int_state == 0) && (IpsecChan.rxbuf.rp != IpsecChan.rxbuf.wp)) //无接收数据时解析
	{
#ifdef INCLUDE_COMM_SHOW	
		len = 0;
		i = IpsecChan.rxbuf.rp;
		while(i != IpsecChan.rxbuf.wp)
		{
			temp[len++] = IpsecChan.rxbuf.buf[i];
			i = (i + 1) & (IPSEC_BUFFER_SIZE - 1);
		}
		commPrint(IpsecChan.no, 3, len, temp);
		commBufFill(IpsecChan.no, 3, len, temp);
#endif				
		SPIM_ReadRxBuffer();
		reccnt = 0;
	}
}

void IPSecReset(void)
{
	CLEAR_GPIO(ipsecrst_gpio,ipsecrst_pin);
	thSleep(1);
	SET_GPIO(ipsecrst_gpio,ipsecrst_pin);
	thSleep(20);
}

void IPSecinit(void)
{
#if (TYPE_CPU == CPU_SAM9X25)
	//复位引脚
	Pin   GPIO_InitStructure;
	GPIO_InitStructure.id         =  ID_PIOC; // 改
	GPIO_InitStructure.attribute  =  PIO_DEFAULT;
	GPIO_InitStructure.type       =  PIO_OUTPUT_1;
	GPIO_InitStructure.mask       =  ipsecrst_pin | GPIO_Pin_6;
	sysPinInit(ipsecrst_gpio,&GPIO_InitStructure);
	//中断引脚
	GPIO_InitStructure.id         =  ID_PIOC; // 改
	GPIO_InitStructure.attribute  =  PIO_IT_RISE_EDGE;
	GPIO_InitStructure.type       =  PIO_INPUT;
	GPIO_InitStructure.mask       =  ipsecint_pin; 
	GPIO_InitStructure.pio = ipsecint_gpio;
	sysPinInit(ipsecint_gpio,&GPIO_InitStructure);
	
	PIO_ConfigureIt(&GPIO_InitStructure);
	rt_hw_interrupt_install(irq_int_pio ,EXT_IPSecIRQHandler,RT_NULL,"ipsec_it"); //改
	rt_hw_interrupt_umask(irq_int_pio);
#else
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = ipsecrst_pin;
	sysPinInit(ipsecrst_gpio, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = ipsecint_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  sysPinInit(ipsecint_gpio, &GPIO_InitStructure);
	
	syscfgEXTILineConfig(EXTI_PortSourceGPIOH, EXTI_PinSource9); //改
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line9; //改
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	stm32EXTIInit(&EXTI_InitStructure);
	EXTI->PR = EXTI_Line9;
	stm32fIntEnable(EXTI9_5_IRQn, INT_PRIO_1MS);//优先级	
#endif	

}

// spi组报文
void IPSecSend(WORD cmd,WORD N,BYTE* sBuf,WORD slen,BYTE* rBuf,WORD* rlen) //组包
{
	BYTE lrc1,lrc2;
	
	rBuf[0] = 0xAA;
	*rlen = 1;
	rBuf[*rlen++] = HIBYTE(cmd);
	rBuf[*rlen++] = LOBYTE(cmd);
	rBuf[*rlen++] = HIBYTE(N);
	rBuf[*rlen++] = LOBYTE(N);
	rBuf[*rlen++] = HIBYTE(slen);
	rBuf[*rlen++] = LOBYTE(slen);
	lrc1 = Lrc(rBuf,6);
	rBuf[*rlen++] = lrc1;
	if((sBuf == NULL) || (slen == 0))
		return;
	memcpy(rBuf+7,sBuf,slen); // 
	*rlen += slen;
	lrc2 = Lrc(sBuf,slen);
	rBuf[*rlen++] = lrc2;
}

//添加命令头 0xAA cmd type N1 N2 len1 len2 lrc1
void IpsecSendHead(WORD cmd,WORD N,int len)
{
	smMTake(IpsecSendMSem);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = 0xAA;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = HIBYTE(cmd);
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = LOBYTE(cmd);
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = HIBYTE(N);
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = LOBYTE(N);
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = HIBYTE(len);
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = LOBYTE(len);
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = ~(HIBYTE(cmd) ^ LOBYTE(cmd) ^ HIBYTE(N)
	         ^ LOBYTE(N) ^ HIBYTE(len) ^ LOBYTE(len));
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
}

// 添加校验 lrc2
void IpsecSendTail(int len)
{
	BYTE lrc2 = 0;
	int i,wp;
	if(len == 0)
	{
		smMGive(IpsecSendMSem);
		return;
	}
	for(i = 0; i < len; i++)
	{
		wp = (IpsecChan.txbuf.wp + IPSEC_BUFFER_SIZE - len + i) & (IPSEC_BUFFER_SIZE-1);
		lrc2 ^= IpsecChan.txbuf.buf[wp];
	}
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = ~lrc2;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	smMGive(IpsecSendMSem);
}

//与ecrProcData功能类似 
//发送0x55 + (cla ins p1 p2 len1 len2 + data) + lrc3
//接收0x55 + (sw1 + sw2 + len1 + len2 + data) + lrc3，spiChan.cos.buf 不含 0x55与lrc3
//还缺 cos的组包 重写 ecrProcData 
WORD IPSecWriteCOS(BYTE* cmd, BYTE cmdLen, BYTE* sBuf,WORD sLen, BYTE* rBuf, WORD* rLen)
{
	int i;
	WORD sw = 0xffff;
	BYTE lrc3 = 0;
	if(cmdLen == 0)
		return 0xffff;
	IpsecSendHead(CMD_COS,N1N2SET,sLen+2+cmdLen);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = 0x55;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	for( i = 0; i < cmdLen; i++)
	{
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = cmd[i];
		IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
		lrc3 ^= cmd[i];
	}
	for( i = 0; i < sLen; i++)
	{
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = sBuf[i];
		IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
		lrc3 ^= sBuf[i];
	}
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = ~lrc3;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecSendTail(sLen+2+cmdLen);
	
	smMTake(IpsecCosMSem);
	smMTake_tm(IpsecCosMSem,500); //等5s,等cos回复
	
	*rLen = MAKEWORD(IpsecChan.cos.buf[3],IpsecChan.cos.buf[2]);
	sw = MAKEWORD(IpsecChan.cos.buf[1],IpsecChan.cos.buf[0]);
	if(IpsecChan.cos.len  == (*rLen+4)) //防止溢出
		memcpy(rBuf,IpsecChan.cos.buf+4,*rLen);
	//读完清掉
	IpsecChan.cos.buf[0] = IpsecChan.cos.buf[1] = 0xff;
	IpsecChan.cos.buf[2] = IpsecChan.cos.buf[3] = 0;
	smMGive(IpsecCosMSem);
	return sw;
}

//将发送数据转为 IPSec指令发送报文 COM + TUN + LNG（2） + DATA

void IpsecNetsend(int len)
{
	VCommBuf *pfifo;
	pfifo = &(g_CommChan[IpsecChan.no].tx_buf);

	if(len == 0)
		return;
	
	IpsecSendHead(CMD_NETDATA,N1N2SET,len+4);
	if(IpsecChan.netpara.tcpresp)
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = 0x02; // 0x02;//COM，tcp暂不需要收到回复，调试时可能需要打开
	else
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = 0x01; 
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = IpsecChan.netpara.tun; //TUN
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = HIBYTE(len); //len
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = LOBYTE(len); //len
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	
	while(pfifo->rp != pfifo->wp)
	{
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = pfifo->buf[pfifo->rp];
		pfifo->rp = (pfifo->rp + 1) & (IPSEC_BUFFER_SIZE - 1);
		IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	}
	IpsecSendTail(len+4);
}

//Data 12字节，设置源IP掩码网关
void IPSecSetSIP()
{
	int i;
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_SIP,N1N2SET,12);
	for(i = 0; i < 12;i++)
	{
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = IpsecChan.netpara.srip[i];
		IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	}
	IpsecSendTail(12);
}

//发送数据为空，读取源IP掩码网关
void IPSecGetSIP()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_SIP,N1N2GET,0);
	IpsecSendTail(0);
}

//Data 12字节，设置目标IP掩码网关
void IPSecSetDIP()
{	
	int i;
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_DIP,N1N2SET,12);
	for(i = 0; i < 12;i++)
	{
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = IpsecChan.netpara.drip[i];
		IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	}
	IpsecSendTail(12);
}

//无发送数据，读取目标IP掩码网关
void IPSecGetDIP()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_DIP,N1N2GET,0);
	IpsecSendTail(0);
}

//data 6个字节，设置源mac
void IPSecSetSMAC()
{
	int i;
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_SMAC,N1N2SET,6);
	for(i = 0; i < 6;i++)
	{
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = IpsecChan.netpara.srmac[i];
		IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	}
	IpsecSendTail(6);
}

//无发送数据，读取源mac
void IPSecGetSMAC()
{	
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_SMAC,N1N2GET,0);
	IpsecSendTail(0);
}

//无发送数据 设置安全芯片模式,mode 0启用协议，01旁路不启用
void IPSecSetPrtlMode()
{
	if(!g_ipsecinit)
		return;
	if(IpsecChan.netpara.Prmode == 0) // 0为启用协议，01为旁路不启用
		IpsecSendHead(CMD_PRMODE,N1N2SET,0);
	else
		IpsecSendHead(CMD_PRMODE,N1N2NIPSEC,0);
	IpsecSendTail(0);
}

//无发生数据，获取安全芯片模式
void IPSecGetPrtlMode()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_PRMODE,N1N2GET,0);
	IpsecSendTail(0);
}

//6个字节，设置安全芯片时钟 ssmmhhDDMMYY
void IPSecSetRTC()
{	
	struct VSysClock SysTime;
	if(!g_ipsecinit)
		return;
	GetSysClock((void *)&SysTime, SYSCLOCK);
	IpsecSendHead(CMD_RTC,N1N2SET,6);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = SysTime.bySecond;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = SysTime.byMinute;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = SysTime.byHour;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = SysTime.byDay;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = SysTime.byMonth;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = SysTime.wYear%100;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecSendTail(6);
}

//无发生数据,获取安全芯片时钟
void IPSecGetRTC()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_RTC,N1N2GET,0);
	IpsecSendTail(0);
}

//获取当前 tcp的连接状态 
void IPSecNetSTA(BYTE* sbuf,WORD slen,BYTE* rBuf,WORD* rlen)
{	
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_NETSTA,N1N2SET,0);
	IpsecSendTail(0);
}

void IPSecGetPhy(BYTE phyid)
{	
	if(!g_ipsecinit)
		return;
	IpsecSendHead(0xA109,N1N2GET,01);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = phyid;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecSendTail(01);
}

void IPSecSetPhy(BYTE phyid,WORD value)
{	
	if(!g_ipsecinit)
		return;
	IpsecSendHead(0xA109,N1N2GET,03);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = phyid;
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = HIBYTE(value);
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = LOBYTE(value);
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecSendTail(03);
}

// COM 1:发送tcp数据无确认，2发送tcp数据有确认
//      3发送udp数据无确认  4发送udp数据有确认
void IPSecNetData(BYTE COM,BYTE* buf,WORD len)
{
	int i;
	if(!g_ipsecinit)
		return;
	if(len == 0)
		return;
	
	IpsecSendHead(CMD_NETDATA,N1N2SET,len+4);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = COM; //
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = IpsecChan.netpara.tun; //TUN
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = HIBYTE(len); //len
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = LOBYTE(len); //len
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	for(i = 0;i < len;i++)
	{
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = buf[i]; //
		IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	}
	IpsecSendTail(len+4);
}

//创建 tcp socket 并监听返回结果,发送0x05,port 接收0x85
void IPSecNetSetPort()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_NETDATA,N1N2SET,3);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = 0x05; // COM 0x05
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = HIBYTE(IpsecChan.netpara.port); //port
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = LOBYTE(IpsecChan.netpara.port); //port
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecSendTail(3);
}

//关闭socket,0x08,connectsocket
void IPSecCloseSocket()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_NETDATA,N1N2SET,2);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = 0x08; // COM 0x08
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = IpsecChan.netpara.connectsocket; //
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecSendTail(2);
	IpsecChan.status &= ~serverconnect;
}

//关闭socket,0x08,
void IPSecCloseClientSocket()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_NETDATA,N1N2SET,2);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = 0x08; // COM 0x08
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = IpsecChan.netpara.clientsocket;//
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecSendTail(2);
	IpsecChan.status &= ~clientconnect;
}

void IPSecNetSetClient()
{
	int i;
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_NETDATA,N1N2SET,7);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = 0x09; // COM 0x09
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	for(i = 0; i < 4;i++)
	{
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = IpsecChan.netpara.drip[i];
		IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	}
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = HIBYTE(IpsecChan.netpara.port); //port
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = LOBYTE(IpsecChan.netpara.port); //port
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecSendTail(7);
}

void IPSecNetSetClientRev()
{
	int i;
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_NETDATA,N1N2SET,7);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = 0x09; // COM 0x09
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	for(i = 0; i < 4;i++)
	{
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = IpsecChan.netpara.driprev[i];
		IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	}
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = HIBYTE(IpsecChan.netpara.port); //port
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = LOBYTE(IpsecChan.netpara.port); //port
	IpsecChan.txbuf.wp = (IpsecChan.txbuf.wp + 1) & (IPSEC_BUFFER_SIZE-1);
	IpsecSendTail(7);
}

int IPSecRead (int no, BYTE* pbuf, int buflen, DWORD flags)
{
	int i,wp;	
	VCommBuf *pfifo;	

	pfifo = &(g_CommChan[IpsecChan.no].rx_buf);
  wp = pfifo->wp;

	for(i=0; i<buflen; i++)
	{
	  if(pfifo->rp == wp) break;		
    *(pbuf+i) = *(pfifo->buf+ pfifo->rp);
		pfifo->rp = (pfifo->rp+1)&(IPSEC_BUFFER_SIZE-1);
	}
	return i;
}

int IPSecWrite (int no, BYTE* pbuf, int buflen, DWORD flags)
{
	int i; WORD wp;
	VCommBuf *pfifo;

	if (g_ipsecinit == 0) return 0;
	g_CommChan[IpsecChan.no].tx_idle = 0;
	pfifo = &(g_CommChan[IpsecChan.no].tx_buf);
	if(buflen == 0) return 0;
		
	for(i=0; i<buflen; i++)
	{		
	 	wp = (pfifo->wp+1)&(IPSEC_BUFFER_SIZE-1);
		if(pfifo->rp == wp) break;
		*(pfifo->buf+ pfifo->wp) = *(pbuf+i);
		pfifo->wp = wp;
	}
	//tcp组包 
	IpsecNetsend(buflen);
	
	g_CommChan[no].ctrl(no, CCC_INT_CTRL|TXINT_ON, NULL);
	return buflen;
}

int IPSecCtrl (int no, WORD command, DWORD *para)
{
	int ret = ERROR;
	switch(command&0xFF00)
	{
		case CCC_CONNECT_CTRL:
			if(command & CONNECT_CLOSE)
			{	
				if (IpsecChan.status)
				{
					//关闭连接
					if(IpsecChan.netpara.tcpmode == 0) //服务器模式
						IPSecCloseSocket();
					else
						IPSecCloseClientSocket();
				}	
				ret = OK;			
			}
			if (command & CONNECT_STATUS_GET)
			{
				if ((IpsecChan.status & serverconnect) && (IpsecChan.netpara.tcpmode == 0))
					*para=1;
				else if((IpsecChan.status & clientconnect) && (IpsecChan.netpara.tcpmode == 1))
					*para=1;
				else
					*para=0;
				ret = OK;
			}
			break;
	}
	return ret;;
}

//IPSec芯片一些参数初始化
BOOL IPSecParaInit()
{
	VIpsecPara* pIpsecpara;
	if (ReadParaFile("IPSec.cfg", (BYTE *)g_pParafile, MAXFILELEN) == ERROR)
	{
		return ERROR;
	}
	pIpsecpara =  (VIpsecPara *)(g_pParafile+1);
	if((pIpsecpara->cfg & 0x01) == 0)
		return ERROR;
	memcpy(IpsecChan.netpara.srip,pIpsecpara->sip,12);
	memcpy(IpsecChan.netpara.drip,pIpsecpara->dip,12);
	memcpy(IpsecChan.netpara.srmac,pIpsecpara->srmac,6);
	IpsecChan.netpara.Prmode = pIpsecpara->Prmode;
	IpsecChan.netpara.tun = 0;
	IpsecChan.netpara.port = pIpsecpara->port;
	IpsecChan.netpara.tcpresp = pIpsecpara->tcpresp;
	IpsecChan.netpara.tcpmode = pIpsecpara->tcpmode;
	IpsecChan.netpara.clientsocket = IpsecChan.netpara.listensocket = IpsecChan.netpara.connectsocket = 0xAA;
        memcpy(IpsecChan.netpara.driprev,pIpsecpara->diprev,12);
	return OK;
}

//设置网络参数等初始化，规约调用 设置,超时退出 1分钟
void IPsecParaSet(WORD delay)
{
	int old = -1;
	int cnt = 0;
	int refasong = 0;
	if(!g_ipsecinit)
		return;
	thSleep(delay);
	while((IpsecChan.int_no < IPSECSETNUM) && (cnt < (3000/20)))
	{
		if((IpsecChan.int_no == old) && (refasong < 24)) //一样则不再发送
		{
			thSleep(20);
			refasong++;
		}
		else
		{
			switch(IpsecChan.int_no)
			{
				case SETSIP: //设置源IP等
					IPSecSetSIP();
					break;
				case READSIP: //读一下看是否一致
					IPSecGetSIP();
					break;
				case SETDIP: //设置目的IP
					IPSecSetDIP();
					break;
				case READDIP: //读目的IP
					IPSecGetDIP();
					break;
				case SETMAC: //设置源MAC
					IPSecSetSMAC();
					break;
				case READMAC: //读取源MAC
					IPSecGetSMAC();
					break;
				case SETPRTL: //设置协议应用模式
					IPSecSetPrtlMode();
					break;
				case READPRTL: //读取协议应用模式
					IPSecGetPrtlMode();
					break;
				case SETRTC: //设置安全芯片时间
					IPSecSetRTC();
					break;
				case READRTC: //读取安全芯片时间
					IPSecGetRTC();
					break;
				case IPSECRST: //复位ipsec
					if(g_ipsecrst == 1) //需要复位
					{
						IPSecReset();
						thSleep(1000);
					}
					IpsecChan.int_no = TCPSET;
					break;
				case TCPSET: //创建listsocket
					if(IpsecChan.netpara.tcpmode == 0)
						IPSecNetSetPort();
					else
						IPSecNetSetClient();
                       IpsecChan.int_no = IPSECSETNUM;
				  break;
				default:
					IpsecChan.int_no = IPSECSETNUM;
					break;
			}
			old = IpsecChan.int_no;
			refasong = 0;
			thSleep(20);
		}
		cnt++;
	}
	if(IpsecChan.int_no < IPSECSETNUM)
	{
		shellprintf("设置IPSEC芯片网络参数错误，超时 \n");
		WriteDoEvent(NULL,0,"设置IPSEC芯片网络参数错误，超时 ");
	}
//    g_ipsecready = 0;
     g_ipsecparaset = 1;
}

void IPSecSpiInit()
{
	int no;

	memset(&IpsecChan, 0 ,sizeof(VIpsecChan));
	IpsecCosMSem = smMCreate();
	IpsecSendMSem = smMCreate();
	IpsecChan.no = COMM_IPSEC_NO - COMM_START_NO;//
	IpsecChan.rxbuf.size = IpsecChan.txbuf.size = IPSEC_BUFFER_SIZE;
	IpsecChan.rxbuf.rp = IpsecChan.rxbuf.wp = IpsecChan.txbuf.rp = IpsecChan.txbuf.wp = 0;
	IpsecChan.rxbuf.buf = (BYTE*)calloc(IPSEC_BUFFER_SIZE,1);
	IpsecChan.txbuf.buf = (BYTE*)calloc(IPSEC_BUFFER_SIZE,1);
	memset(IpsecChan.rxbuf.buf,0,IPSEC_BUFFER_SIZE);
	memset(IpsecChan.txbuf.buf,0,IPSEC_BUFFER_SIZE);
	IpsecChan.cos.len = 0;
	IpsecChan.cos.buf = (BYTE*)calloc(IPSEC_BUFFER_SIZE,1);
	memset(IpsecChan.cos.buf,0,IPSEC_BUFFER_SIZE);

	spirxtempbuffer = (BYTE*)calloc(IPSEC_BUFFER_SIZE,1);
	memset(spirxtempbuffer, 0, IPSEC_BUFFER_SIZE);
	
	if(IPSecParaInit() == ERROR)
		return;
	
	if(g_Task[COMM_IPSEC_NO].CommCfg.Port.id == 0) //该端口已设置规约了 ,故指待测不可配
		return;
		
	no = commRegister(COMM_IPSEC_NO-COMM_START_NO, 0, IPSEC_BUFFER_SIZE, (void *)IPSecRead, (void *)IPSecWrite, (void *)IPSecCtrl);
  if(no < 0)  return; 
#if (TYPE_CPU == CPU_STM32F4)	
	SPIInit(IPSECSPI, SPI_BaudRatePrescaler_32, SPI_DataSize_8b); // M4 
#else
	SPIInit(IPSECSPI, 20, SPI_DataSize_8b); // arm9  100/20 = 5M
#endif

	IPSecinit();
	IPSecReset(); //因与ipsec网关需要协商，不建议复位，除非设网络参数
	g_ipsecinit = 1;
}

void ipsecnettest(int id)
{
				switch(id)
			{
				case 0: //设置源IP等
					IPSecSetSIP();
					break;
				case 1: //读一下看是否一致
					IPSecGetSIP();
					break;
				case 2: //设置目的IP
					IPSecSetDIP();
					break;
				case 3: //读目的IP
					IPSecGetDIP();
					break;
				case 4: //设置源MAC
					IPSecSetSMAC();
					break;
				case 5: //读取源MAC
					IPSecGetSMAC();
					break;
				case 6: //设置协议应用模式
					IPSecSetPrtlMode();
					break;
				case 7: //读取协议应用模式
					IPSecGetPrtlMode();
					break;
				case 8: //设置安全芯片时间
					IPSecSetRTC();
					break;
				case 9: //读取安全芯片时间
					IPSecGetRTC();
					break;
				case 10: //创建listsocket
					IPSecNetSetPort();
					break;
				case 11: // 创建 client
					IPSecNetSetClient();
					break;
				case 12: // 关闭连接
					IPSecCloseSocket();
				  break;
				case 13: // 关闭client连接
					IPSecCloseClientSocket();
				  break;
                  case 14:
                        IPSecNetSetClientRev();
                     break;
			}
}
#endif

