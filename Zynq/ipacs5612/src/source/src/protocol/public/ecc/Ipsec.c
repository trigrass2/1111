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

static DWORD IpsecCosMSem; //cos�ź���������һ��������cos�����ȴ����ز��С�
static DWORD IpsecSendMSem; //send�����ź�������
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

/////M4���Ŵ���////
#define ipsecrst_pin         GPIO_Pin_6 // ��λ�������Ͷ���ʱ��
#define ipsecrst_gpio        GPIOF      //
#define ipsecint_pin         GPIO_Pin_9 // �ն˽�
#define ipsecint_gpio        GPIOH   
#define IPSECSPI   SPI2
EXTI_InitTypeDef EXTI_InitStructure;

#elif (TYPE_CPU == CPU_SAM9X25)

#define IPSECSPI   SPI1
#define ipsecrst_pin         GPIO_Pin_7 // ��λ�������Ͷ���ʱ��
#define ipsecrst_gpio        GPIOC      //
#define ipsecint_pin         GPIO_Pin_5 // �ն˽�
#define ipsecint_gpio        GPIOC   
#define irq_int_pio          (5 + 3*32) //�жϺ�
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

//spi ����������
void SendByteToRxBuffer(BYTE rx_data)
{
	IpsecChan.rxbuf.buf[IpsecChan.rxbuf.wp] = rx_data;
	IpsecChan.rxbuf.wp = (IpsecChan.rxbuf.wp + 1) & (IPSEC_BUFFER_SIZE - 1);
}
//��spi����������ȡ��
BYTE GetByteFromRxBuffer()
{
	BYTE buffer = IpsecChan.rxbuf.buf[IpsecChan.rxbuf.rp];
	IpsecChan.rxbuf.rp = (IpsecChan.rxbuf.rp + 1) & (IPSEC_BUFFER_SIZE - 1);
	return buffer;
}

// spi �������� ����,�����������ݷ���
void SPIM_TxRx()
{
	BYTE rx_data,tx_data;
	DWORD gpio_state; 
	//disable �����ж�
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
// ʹ����������
#if (TYPE_CPU == CPU_STM32F4)
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	stm32EXTIInit(&EXTI_InitStructure);
#elif (TYPE_CPU == CPU_SAM9X25)
	rt_hw_interrupt_umask(irq_int_pio); 
#endif
}

#if (TYPE_CPU == CPU_SAM9X25)
// �жϽ��� ����
//int time = 0;
//DWORD t1[100],t2[100];
//DWORD count[100] = {0};
//��ӷ�ֹ��ѭ�������
void EXT_IPSecIRQHandler(int vector, void *param)
{
	//spi����
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
		WriteDoEvent(NULL, 0,"spi ���չ���2048 ");
//	t2[time] = Get100usCnt();
//	time++;
//����
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
	//spi����
	BYTE tx_data,rx_data;
	DWORD gpio_state = 1;
	tx_data = 0x00;

	while(gpio_state)
	{
		SPITrans_BAddr( IPSECSPI, 0, &tx_data, &rx_data,1);
		SendByteToRxBuffer(rx_data);
		gpio_state = read_int_state;
	}
//����
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
		shellprintf("����ԴIP�����롢���سɹ� \n");
		IpsecChan.int_no = READDIP;
		g_ipsecrst = 1;
		ret = TRUE;
	}
	else if(len == 12)
	{
		memcpy(ip,buffer,4);
		memcpy(mask,buffer+4,4);
		memcpy(gateway,buffer+8,4);
		shellprintf(" ��ȡԴIPΪ %d:%d:%d:%d,����Ϊ %d:%d:%d:%d,����Ϊ%d:%d:%d:%d \n",ip[3],ip[2],ip[1],ip[0],
		mask[3],mask[2],mask[1],mask[0],gateway[3],gateway[2],gateway[1],gateway[0]);
		ret = TRUE;
		for(i = 0; i < 12; i++)
		{
			if(buffer[i] != IpsecChan.netpara.srip[i])
			{
				ret = FALSE;
				shellprintf("��ȡ��ԴIP�����õĲ�һ�� \n");
			}
		}
		if(ret == TRUE)
			IpsecChan.int_no = READDIP;
		else
			IpsecChan.int_no = SETSIP;
	}
	else
	{
		shellprintf("���ö�ȡԴIP�����롢����ʧ�� \n");
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
		shellprintf("����Ŀ��IP�����롢���سɹ� \n");
		ret = TRUE;
		g_ipsecrst = 1;
		IpsecChan.int_no = READPRTL;
	}
	else if(len == 12)
	{
		memcpy(ip,buffer,4);
		memcpy(mask,buffer+4,4);
		memcpy(gateway,buffer+8,4);
		shellprintf(" ��ȡĿ��IPΪ %d:%d:%d:%d,����Ϊ %d:%d:%d:%d,����Ϊ%d:%d:%d:%d \n",ip[3],ip[2],ip[1],ip[0],
		mask[3],mask[2],mask[1],mask[0],gateway[3],gateway[2],gateway[1],gateway[0]);
		ret = TRUE;
		for(i = 0; i < 12;i++)
		{
			if(buffer[i] != IpsecChan.netpara.drip[i])
			{
				ret = FALSE;
				shellprintf("��ȡ��Ŀ��IP�������õĲ�һ�� \n");
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
		shellprintf("���ö�ȡĿ��IP�����롢����ʧ�� \n");
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
		shellprintf("����ԴMAC �ɹ�\n");
		IpsecChan.int_no = READPRTL;
		g_ipsecrst = 1;
		ret = TRUE;
	}
	else if(len == 6)
	{
		memcpy(mac,buffer,6);
		shellprintf(" ��ȡԴMAC %02X:%02X:%02X:%02X:%02X:%02X \n",mac[0],mac[1],mac[2],mac[3],
		mac[4],mac[5]);
		ret = TRUE;
		for(i = 0;i < 6;i++)
		{
			if(buffer[i] != IpsecChan.netpara.srmac[i])
			{
				ret = FALSE;
				shellprintf("��ȡ��Դmac�����õĲ�һ�� \n");
			}
		}
		if(ret == TRUE)
			IpsecChan.int_no = READPRTL;
		else
			IpsecChan.int_no = SETMAC;
	}
	else
	{
		shellprintf("���ö�ȡԴMACʧ�� \n");
	}
	return ret;
}

BOOL SPIM_ProcessDataPrmode(BYTE* buffer,WORD len)
{
	BOOL ret = FALSE;
	if((len == 2) && (buffer[0] == 0x00) && (buffer[1] == 0x00))
	{
		shellprintf("���ð�ȫоƬģʽ�ɹ� \n");
		IpsecChan.int_no = READRTC;
		g_ipsecrst = 1;
		ret = TRUE;
	}
	else if(len == 1)
	{
		if(buffer[0] == 00)
			shellprintf("IPSecЭ��Ӧ��ģʽ \n");
		else if(buffer[0] == 0x01)
			shellprintf("��·ģʽ \n");
		ret = TRUE;
		if(buffer[0] != IpsecChan.netpara.Prmode)
		{
			shellprintf("��ȡ��Э��ģʽ�����õĲ�һ�� \n");
			ret = FALSE;
		}
		if(ret == TRUE)
			IpsecChan.int_no = READRTC;
		else
			IpsecChan.int_no = SETPRTL;
	}
	else
	{
		shellprintf("���ö�ȡIPSec��ȫоƬģʽʧ�� \n");
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
		shellprintf("���ð�ȫоƬʱ�ӳɹ� \n");
		IpsecChan.int_no = IPSECRST;
		ret = TRUE;
	}
	else if(len == 6)
	{
		shellprintf("��ȡ��ȫоƬʱ�� %d��%d��%d��%dʱ%d��%d�� \n",buffer[5],buffer[4],
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
			shellprintf("оƬrtcʱ���ͺ� %d s \n",temp);
			IpsecChan.int_no = IPSECRST;
		}
	}
	else
	{
		shellprintf("���ö�ȡIPSec��ȫоƬʱ��ʧ�� \n");
	}
	return ret;
}

//�����壬ȥ����������״̬
BOOL SPIM_ProcessDataSta(BYTE* buffer,WORD len)
{
	BOOL ret = FALSE;
	if((len == 2) && (buffer[0] == 0x00) && (buffer[1] == 0x00))
	{
		shellprintf("tcp������ \n");
		IpsecChan.status = 1;
		ret = TRUE;
	}
	else
	{
		shellprintf("tcpδ���� \n");
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

//���� 0x55 + sw1 + sw2 + len1 + len2 + data + lrc2
//IpsecChan.cos.buf ���� 0x55��lrc2
void SPIM_ProcessDataCos(BYTE* buffer,WORD len)
{
//	if()
//	{
//		
//	}
	WORD datalen = 0;
  BYTE lrc3 = 0;	
	if(buffer[0] != 0x55)
		shellprintf("����cos ͷ����0x55 \n");	
	if(len < 6)
		shellprintf("����cos ����̫�� \n");
	datalen  = MAKEWORD(buffer[4],buffer[3]);
	if((datalen + 6) != len)
		shellprintf("����cos���Ȳ��� \n");
	lrc3 = Lrc(buffer+1,len-2);
	
	if(lrc3 != buffer[len-1])
		shellprintf("����cosУ�鲻�� \n");
	if(len > 2047)
		shellprintf("����cos���ȹ��� \n");
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
	evSend(g_CommChan[IpsecChan.no].tid, EV_RX_AVAIL); //���߹�Լ���������ݵ���Լ����
}

void SPIM_ProcessDataNet(BYTE* buffer,WORD len)
{
  BYTE com,tun;
	WORD port,datalen = 0;
	BYTE ret = TRUE;
	
	com = buffer[0];
	if(len < 3)
	{
		shellprintf("���ճ���̫��С��3 \n");
		WriteDoEvent(NULL,0,"spiת�����糤��С��3 ");
		return;
	}
	switch(com)
	{
		case 0x00:  // ���˷�춨��Ϊsocket
			if(buffer[1] == 0x00)
				shellprintf("�յ������������ݵ�ȷ�� �������%d \n",buffer[2]);
			else
				shellprintf("com == 0x00 ���մ��� \n");
		  break;	
		case 0x81: //tcp����֪ͨ   
			tun = buffer[1];
//			if(IpsecChan.netpara.Prmode)
//			{
				IpsecChan.netpara.tun = tun; //����˷�첻�������ã���оƬ����
//			}				
//			else
//			{
//				if(tun != IpsecChan.netpara.tun)
//				{
//					shellprintf("�����������������һ�� \n");
//				}
//			}
		  datalen = MAKEWORD(buffer[3],buffer[2]);
			if((datalen + 4) != len)
				shellprintf("����tcp�������� \n");
			spiwritetocomm(buffer+4, datalen);
			break;
		case 0x83: //udp����֪ͨ
			shellprintf(" �յ�udp����֪ͨ,���� \n");
		case 0x85:
			ret = TRUE;
			port = MAKEWORD(buffer[2],buffer[1]);
			if(port != IpsecChan.netpara.port)
			{
				shellprintf("�յ���port��һ�� \n");
				ret = FALSE;
				return;
			}
			if(buffer[3] == 0x00)
			{
				IpsecChan.netpara.listensocket = buffer[4];
				shellprintf("��������socket�ɹ�,%d \n",IpsecChan.netpara.listensocket);
				WriteDoEvent(NULL,0,"��������socket�ɹ� ");
				ret = TRUE;
			}
			else
			{
				shellprintf("��������socketʧ��\n");
				WriteDoEvent(NULL,0,"��������socketʧ�� ");
			}
				
			if(ret == TRUE)
				IpsecChan.int_no = IPSECSETNUM;
			break;
		case 0x86://�յ�connetsocket������
			if(buffer[1] != IpsecChan.netpara.listensocket)
			{
				shellprintf("tcp����listen�Ų��� \n");
				return;
			}
			if(buffer[3] == 0x00)
			{
				IpsecChan.netpara.connectsocket = buffer[2];
				shellprintf("������ģʽtcp������ \n");
				WriteDoEvent(NULL,0,"������ģʽtcp������ ");
                   IpsecChan.netpara.tun = IpsecChan.netpara.connectsocket;
				IpsecChan.status |= serverconnect;
				evSend(g_CommChan[IpsecChan.no].tid, EV_COMM_STATUS);
			}
		  break;
		case 0x87://tcp���ӹر�
			if((buffer[1] == IpsecChan.netpara.connectsocket) && (buffer[2] == 0x00))
			{
				IpsecChan.netpara.connectsocket = 0xff;
				shellprintf("������ģʽ����վ���ӹر� \n");
				WriteDoEvent(NULL,0,"������ģʽ����վ���ӹر� ");
				IpsecChan.status &= ~serverconnect;
				evSend(g_CommChan[IpsecChan.no].tid, EV_COMM_STATUS);
			}
			else if((buffer[1] == IpsecChan.netpara.clientsocket) && (buffer[2] == 0x00))
			{
				IpsecChan.netpara.clientsocket = 0xff;
				shellprintf("�ͻ���ģʽ����վ���ӹر� \n");
				WriteDoEvent(NULL,0,"�ͻ���ģʽ����վ���ӹر� ");
				IpsecChan.status &= ~clientconnect;
				evSend(g_CommChan[IpsecChan.no].tid, EV_COMM_STATUS);
			}
			else 
			{
				shellprintf("tcp���ӹرմ��� \n");
			}
			break;
		case 0x89:
			if(len < 7)
			{
				shellprintf("����TCP Client���ӷ��س���̫�� \n");
				return;
			}
			if((memcmp(buffer+1, IpsecChan.netpara.drip,4) != 0) && (memcmp(buffer+1, IpsecChan.netpara.driprev,4) != 0))
			{
				ret = FALSE;
				shellprintf("�ͻ���ģʽ���ص�Ŀ��IP��һ�� \n");
				WriteDoEvent(NULL,0,"�ͻ���ģʽ���ص�Ŀ��IP��һ�� ");
			}
			port =  MAKEWORD(buffer[6],buffer[5]);
			if(port != IpsecChan.netpara.port)
			{
				ret = FALSE;
				shellprintf("�ͻ���ģʽ���صĶ˿ںŲ�һ�� \n");
				WriteDoEvent(NULL,0,"�ͻ���ģʽ���صĶ˿ںŲ�һ�� ");
			}
			if((buffer[7] == 0x00) && ret) //
			{
				IpsecChan.netpara.clientsocket = buffer[8];
				shellprintf("tcp client socket %d \n",buffer[8]);
				WriteDoEvent(NULL,0,"�ͻ���ģʽ����վ����");
				IpsecChan.netpara.tun = IpsecChan.netpara.clientsocket;
				IpsecChan.int_no = IPSECSETNUM;
				IpsecChan.status |= clientconnect;
			}
			else
			{
				shellprintf("tcp client δ�ɹ� %d \n",buffer[8]);
                   if(memcmp(buffer+1, IpsecChan.netpara.drip,4)  == 0) //
                   {
                        shellprintf("tcp����ͨ�����ɹ��б���ͨ�� \n");
				     WriteDoEvent(NULL,0,"tcp����ͨ�����ɹ��б���ͨ��");
                       IPSecNetSetClientRev();
                    }
			}
			break;
		default:
			shellprintf("ת����������ָ��δ֪ com = %d \n",com);
			break;
	}
}


//spi�������ݵ���Լ��������
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
					else // ��DATA����,�������ݰ�
					{
						//SPIM_Recv();  ���޴����
						switch(act)
						{
							case 0xC100: // ˷���������
								if(IpsecChan.netpara.tcpmode) //�ͻ���
								{
									IPSecNetSetClient();
								}
								else //������
								{
									IPSecNetSetPort();
								}
								break;
							case 0xC001: //˷�����ָ��
//								g_ipsecready = 1;
                                     WriteDoEvent(NULL,0,"˷��оƬ����");
								break;
						}
						state = START;
					}
				}
				else
				{
					//У�鲻�ԣ���������
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

//spi ɨ�躯����20ms ɨ��һ�Σ��޽���ʱ�ŷ���

//��3����û����Ͽ�����
void IPSec_Scan()
{
	int i = 0,len=0;
	static DWORD cnt = 0;
	static DWORD reccnt = 0;
	if(!g_ipsecinit)
		return;
	if((IpsecChan.netpara.tcpmode == 1) && g_ipsecparaset && ((IpsecChan.status & clientconnect) == 0))//�ͻ���δ������
	{
		cnt++;
		if(cnt >= (30*100)/COMM_SCAN_TM) // 30s���
		{
			cnt = 0;
			IPSecNetSetClient();
		}
	}
	else
		cnt = 0;
  
	if((IpsecChan.netpara.tcpmode == 1) && g_ipsecparaset && (IpsecChan.status & clientconnect))//�ͻ���������
	{
		reccnt++;
		if(reccnt >= (150*100)/COMM_SCAN_TM) // 150s���
		{
			reccnt = 0;
			IPSecCloseClientSocket();
			shellprintf("3����δ�յ����ݶϿ��ͻ������� \n");
			WriteDoEvent(NULL,0,"3����δ�յ����ݶϿ��ͻ������� ");
		}
	}
	else
		reccnt = 0;
    
	if((IpsecChan.txbuf.rp != IpsecChan.txbuf.wp) && (read_int_state == 0))//�����ݲŷ���,�����ݽ���ʱ���ܷ���
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
	if((read_int_state == 0) && (IpsecChan.rxbuf.rp != IpsecChan.rxbuf.wp)) //�޽�������ʱ����
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
	//��λ����
	Pin   GPIO_InitStructure;
	GPIO_InitStructure.id         =  ID_PIOC; // ��
	GPIO_InitStructure.attribute  =  PIO_DEFAULT;
	GPIO_InitStructure.type       =  PIO_OUTPUT_1;
	GPIO_InitStructure.mask       =  ipsecrst_pin | GPIO_Pin_6;
	sysPinInit(ipsecrst_gpio,&GPIO_InitStructure);
	//�ж�����
	GPIO_InitStructure.id         =  ID_PIOC; // ��
	GPIO_InitStructure.attribute  =  PIO_IT_RISE_EDGE;
	GPIO_InitStructure.type       =  PIO_INPUT;
	GPIO_InitStructure.mask       =  ipsecint_pin; 
	GPIO_InitStructure.pio = ipsecint_gpio;
	sysPinInit(ipsecint_gpio,&GPIO_InitStructure);
	
	PIO_ConfigureIt(&GPIO_InitStructure);
	rt_hw_interrupt_install(irq_int_pio ,EXT_IPSecIRQHandler,RT_NULL,"ipsec_it"); //��
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
	
	syscfgEXTILineConfig(EXTI_PortSourceGPIOH, EXTI_PinSource9); //��
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line9; //��
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	stm32EXTIInit(&EXTI_InitStructure);
	EXTI->PR = EXTI_Line9;
	stm32fIntEnable(EXTI9_5_IRQn, INT_PRIO_1MS);//���ȼ�	
#endif	

}

// spi�鱨��
void IPSecSend(WORD cmd,WORD N,BYTE* sBuf,WORD slen,BYTE* rBuf,WORD* rlen) //���
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

//�������ͷ 0xAA cmd type N1 N2 len1 len2 lrc1
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

// ���У�� lrc2
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

//��ecrProcData�������� 
//����0x55 + (cla ins p1 p2 len1 len2 + data) + lrc3
//����0x55 + (sw1 + sw2 + len1 + len2 + data) + lrc3��spiChan.cos.buf ���� 0x55��lrc3
//��ȱ cos����� ��д ecrProcData 
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
	smMTake_tm(IpsecCosMSem,500); //��5s,��cos�ظ�
	
	*rLen = MAKEWORD(IpsecChan.cos.buf[3],IpsecChan.cos.buf[2]);
	sw = MAKEWORD(IpsecChan.cos.buf[1],IpsecChan.cos.buf[0]);
	if(IpsecChan.cos.len  == (*rLen+4)) //��ֹ���
		memcpy(rBuf,IpsecChan.cos.buf+4,*rLen);
	//�������
	IpsecChan.cos.buf[0] = IpsecChan.cos.buf[1] = 0xff;
	IpsecChan.cos.buf[2] = IpsecChan.cos.buf[3] = 0;
	smMGive(IpsecCosMSem);
	return sw;
}

//����������תΪ IPSecָ��ͱ��� COM + TUN + LNG��2�� + DATA

void IpsecNetsend(int len)
{
	VCommBuf *pfifo;
	pfifo = &(g_CommChan[IpsecChan.no].tx_buf);

	if(len == 0)
		return;
	
	IpsecSendHead(CMD_NETDATA,N1N2SET,len+4);
	if(IpsecChan.netpara.tcpresp)
		IpsecChan.txbuf.buf[IpsecChan.txbuf.wp] = 0x02; // 0x02;//COM��tcp�ݲ���Ҫ�յ��ظ�������ʱ������Ҫ��
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

//Data 12�ֽڣ�����ԴIP��������
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

//��������Ϊ�գ���ȡԴIP��������
void IPSecGetSIP()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_SIP,N1N2GET,0);
	IpsecSendTail(0);
}

//Data 12�ֽڣ�����Ŀ��IP��������
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

//�޷������ݣ���ȡĿ��IP��������
void IPSecGetDIP()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_DIP,N1N2GET,0);
	IpsecSendTail(0);
}

//data 6���ֽڣ�����Դmac
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

//�޷������ݣ���ȡԴmac
void IPSecGetSMAC()
{	
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_SMAC,N1N2GET,0);
	IpsecSendTail(0);
}

//�޷������� ���ð�ȫоƬģʽ,mode 0����Э�飬01��·������
void IPSecSetPrtlMode()
{
	if(!g_ipsecinit)
		return;
	if(IpsecChan.netpara.Prmode == 0) // 0Ϊ����Э�飬01Ϊ��·������
		IpsecSendHead(CMD_PRMODE,N1N2SET,0);
	else
		IpsecSendHead(CMD_PRMODE,N1N2NIPSEC,0);
	IpsecSendTail(0);
}

//�޷������ݣ���ȡ��ȫоƬģʽ
void IPSecGetPrtlMode()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_PRMODE,N1N2GET,0);
	IpsecSendTail(0);
}

//6���ֽڣ����ð�ȫоƬʱ�� ssmmhhDDMMYY
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

//�޷�������,��ȡ��ȫоƬʱ��
void IPSecGetRTC()
{
	if(!g_ipsecinit)
		return;
	IpsecSendHead(CMD_RTC,N1N2GET,0);
	IpsecSendTail(0);
}

//��ȡ��ǰ tcp������״̬ 
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

// COM 1:����tcp������ȷ�ϣ�2����tcp������ȷ��
//      3����udp������ȷ��  4����udp������ȷ��
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

//���� tcp socket ���������ؽ��,����0x05,port ����0x85
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

//�ر�socket,0x08,connectsocket
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

//�ر�socket,0x08,
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
	//tcp��� 
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
					//�ر�����
					if(IpsecChan.netpara.tcpmode == 0) //������ģʽ
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

//IPSecоƬһЩ������ʼ��
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

//������������ȳ�ʼ������Լ���� ����,��ʱ�˳� 1����
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
		if((IpsecChan.int_no == old) && (refasong < 24)) //һ�����ٷ���
		{
			thSleep(20);
			refasong++;
		}
		else
		{
			switch(IpsecChan.int_no)
			{
				case SETSIP: //����ԴIP��
					IPSecSetSIP();
					break;
				case READSIP: //��һ�¿��Ƿ�һ��
					IPSecGetSIP();
					break;
				case SETDIP: //����Ŀ��IP
					IPSecSetDIP();
					break;
				case READDIP: //��Ŀ��IP
					IPSecGetDIP();
					break;
				case SETMAC: //����ԴMAC
					IPSecSetSMAC();
					break;
				case READMAC: //��ȡԴMAC
					IPSecGetSMAC();
					break;
				case SETPRTL: //����Э��Ӧ��ģʽ
					IPSecSetPrtlMode();
					break;
				case READPRTL: //��ȡЭ��Ӧ��ģʽ
					IPSecGetPrtlMode();
					break;
				case SETRTC: //���ð�ȫоƬʱ��
					IPSecSetRTC();
					break;
				case READRTC: //��ȡ��ȫоƬʱ��
					IPSecGetRTC();
					break;
				case IPSECRST: //��λipsec
					if(g_ipsecrst == 1) //��Ҫ��λ
					{
						IPSecReset();
						thSleep(1000);
					}
					IpsecChan.int_no = TCPSET;
					break;
				case TCPSET: //����listsocket
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
		shellprintf("����IPSECоƬ����������󣬳�ʱ \n");
		WriteDoEvent(NULL,0,"����IPSECоƬ����������󣬳�ʱ ");
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
	
	if(g_Task[COMM_IPSEC_NO].CommCfg.Port.id == 0) //�ö˿������ù�Լ�� ,��ָ���ⲻ����
		return;
		
	no = commRegister(COMM_IPSEC_NO-COMM_START_NO, 0, IPSEC_BUFFER_SIZE, (void *)IPSecRead, (void *)IPSecWrite, (void *)IPSecCtrl);
  if(no < 0)  return; 
#if (TYPE_CPU == CPU_STM32F4)	
	SPIInit(IPSECSPI, SPI_BaudRatePrescaler_32, SPI_DataSize_8b); // M4 
#else
	SPIInit(IPSECSPI, 20, SPI_DataSize_8b); // arm9  100/20 = 5M
#endif

	IPSecinit();
	IPSecReset(); //����ipsec������ҪЭ�̣������鸴λ���������������
	g_ipsecinit = 1;
}

void ipsecnettest(int id)
{
				switch(id)
			{
				case 0: //����ԴIP��
					IPSecSetSIP();
					break;
				case 1: //��һ�¿��Ƿ�һ��
					IPSecGetSIP();
					break;
				case 2: //����Ŀ��IP
					IPSecSetDIP();
					break;
				case 3: //��Ŀ��IP
					IPSecGetDIP();
					break;
				case 4: //����ԴMAC
					IPSecSetSMAC();
					break;
				case 5: //��ȡԴMAC
					IPSecGetSMAC();
					break;
				case 6: //����Э��Ӧ��ģʽ
					IPSecSetPrtlMode();
					break;
				case 7: //��ȡЭ��Ӧ��ģʽ
					IPSecGetPrtlMode();
					break;
				case 8: //���ð�ȫоƬʱ��
					IPSecSetRTC();
					break;
				case 9: //��ȡ��ȫоƬʱ��
					IPSecGetRTC();
					break;
				case 10: //����listsocket
					IPSecNetSetPort();
					break;
				case 11: // ���� client
					IPSecNetSetClient();
					break;
				case 12: // �ر�����
					IPSecCloseSocket();
				  break;
				case 13: // �ر�client����
					IPSecCloseClientSocket();
				  break;
                  case 14:
                        IPSecNetSetClientRev();
                     break;
			}
}
#endif

