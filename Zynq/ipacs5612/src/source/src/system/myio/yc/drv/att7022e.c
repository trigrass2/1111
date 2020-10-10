/*
****************************************************************************
* Copyright (c) 2018
* All rights reserved.
* �������ƣ�att7022e.c
* �� �� �ţ�1.0
* ��    �ܣ�7022E����оƬ
* ԭʼ�����˼�ʱ�䣺
*           cjl 20180207
* �޸���Ա��ʱ�估�޸����ݣ�
* ��   ���� 
****************************************************************************
*/

#include "syscfg.h"
#ifdef INCLUDE_ATT7022E
#include <math.h> 
#include "sys.h"
#include "att7022e.h"
extern VYcGain *pYcGain;

#define ACPulseConst 6400
#define TC_DEFAULT_7022E      23   //7022E �¶�Ĭ��У��ֵTC

const struct MIX_CONFIG_CELL_TYPE *FixupConfigTab = NULL;
TEMP_INFO    TempInfo7022E;
CURRENT_INFO CurrentInfo7022E;
DWORD ITempRegVal[6];
int AllGainVal;

DWORD HFconst = 0;
V7022Gain *p7022gain;
int gain7022Valid=0;
BOOL Att7022eInit = 0;
  
 int InstantSampData7022E[100];   // ʵʱ��ȡATT7022��REGSֵ
 AT7022_DD g_Att7022_Dd[32];
 AT7022_DD_JFPG g_Att7022_Dd_JFPG[8];
 long UAtt7022e[4];
 long IAtt7022e[4];
 long PQAtt7022e[10];
 short  AD_RESULT[7][MaxSampPointNum];  // 6��128��AD����ֵ

extern BYTE extEnergyT[48];
extern short mP;
extern short mQ;

/* ����������ʷ */
static BYTE bFlowDirHis = 0;
static BYTE bActPowerDirHis	= 0;

static BOOL bWriteeeprom = 0;

// �̶��������ݱ�:�����в���Ϊ��������
const struct MIX_CONFIG_CELL_TYPE FixupConfigTab1[] =     // ����100V
{
    {0x00C3,0x03,0x000000},        // �ָ�У����ֵ���ϵ��ʼֵ

    {0x0081,0x03,0x00b9FE},        // ģʽ���üĴ���
    {0x0082,0x03,0x000100},        // ADC �������üĴ���
    {0x0096,0x03,0x000000},        // �޹���λУ��
    {0x009d,0x03,0x00013A},        // �𶯵�����ֵ���� Ib1.5A-50mv,��������:0.1%(0.5����)����0.08%Ib����
    {0x00b0,0x03,0x000001},        // �ж�ʹ�ܼĴ���
    {0x00b1,0x03,0x003437},        // ��·ģ�����üĴ���
    {0x00b5,0x03,0x00000f},        // ����pin ����������ѡ�����
    {0x00b6,0x03,0x000034},        // �𶯹������üĴ���
    {0x00b7,0x03,0x000480},
};

// �̶��������ݱ�:�����в���Ϊ��������
const struct MIX_CONFIG_CELL_TYPE FixupConfigTab2[] =     // ����57.7V
{
    {0x00C3,0x03,0x000000},        // �ָ�У����ֵ���ϵ��ʼֵ

    {0x0081,0x03,0x00b9FE},        // ģʽ���üĴ���
    {0x0082,0x03,0x000100},        // ADC �������üĴ���
    {0x0096,0x03,0x000000},        // �޹���λУ��
    {0x009d,0x03,0x000160},        // �𶯵�����ֵ����
    {0x00b0,0x03,0x000001},        // �ж�ʹ�ܼĴ���
    {0x00b1,0x03,0x003437},        // ��·ģ�����üĴ���
    {0x00b5,0x03,0x00000f},        // ����pin ����������ѡ�����
    {0x00b6,0x03,0x000034},        // �𶯹������üĴ���
    {0x00b7,0x03,0x000300},        // ��λ����������ʩ Iregion=INT[Is*2^5]=INT[Ib*N*�������õ�*2^5]=INT[1.5*40*40%*2^5]=0x300
};

// �̶��������ݱ�:�����в���Ϊ��������
const struct MIX_CONFIG_CELL_TYPE FixupConfigTab3[] =     // ����220V 3200/6400����
{    
    {0x00C3,0x03,0x000000},        // �ָ�У����ֵ���ϵ��ʼֵ

    {0x0081,0x03,0x00b9FF},        // ģʽ���üĴ���
    {0x0082,0x03,0x000100},        // ADC �������üĴ��� �����ѹ2������
    {0x0096,0x03,0x000000},        // �޹���λУ��
    {0x009d,0x03,0x000189},        // �𶯵�����ֵ���� Ib1.5A-50mv,��������:0.1%(0.5����) 2012-9-26
    {0x00b0,0x03,0x000001},        // �ж�ʹ�ܼĴ���
    {0x00b1,0x03,0x003437},        // ��·ģ�����üĴ���  �����¶ȼ�� 2012-9-28
    {0x00b5,0x03,0x00000f},        // ����pin ����������ѡ�����
    {0x00b6,0x03,0x000041},        // �𶯹������üĴ��� INT(0.6*220*1.5*0x13e * 3200*0.1%*2^23/(2.592*10^10)) 2012-9-26 
    {0x00b7,0x03,0x0004E0},        // ��λ������������ Iregion=INT[Is*2^5]=INT[Ib*N*�������õ�*2^5]=INT[1.5*40*65%*2^5]=0x4E0
};

short  TempAllgainDefault[25] = 
      {-90,-79,-68,-58,-49,-40,-31,-23,
       -12,-9, 0, 0, 0, 0, 0, 0, 
       0, 0, 0,-9,-12,-17,-22,-28,-34};

// ��ʼг��У�������оƬ�����ṩ
const double DefaultHarmParaTab[22] = 
    {1.00000000000000
    ,1.00000000000000,1.00362187060665,1.00969162604172,1.01825901332331
    ,1.02939520355364,1.04319331488342,1.05977378492696,1.07927443401769
    ,1.10187021533519,1.12776405100967,1.15718387269867,1.19042304659018
    ,1.22777858879375,1.26962990669109,1.31639900106378,1.36856044411429
    ,1.42671649331348,1.49149037623292,1.56363705732262,1.64398947583697
    ,1.73366802372421};

 // ��ѹ������ E211
struct MIX_CONFIG_CELL_TYPE VoltCurrRMSTab[] = 
{
    {0x000d, 3, DATAU},
    {0x000e, 3, 1+DATAU},
    {0x000f, 3, 2+DATAU},
    {0x002b, 3, 3+DATAU},  //�����ѹʸ������Чֵ

    {0x0010, 3, DATAI},
    {0x0011, 3, DATAI+1},
    {0x0012, 3, DATAI+2},
    //{0x0013, 3, DATAI+3},    //�������ʸ���͵���Чֵ

    {0x0029, 3, DATAI+3},   //����·ADC�����źŵ���Чֵ
    {0x003f, 3, DATAI+5}   //����·ADC�����������
};

 // ���� E212
struct MIX_CONFIG_CELL_TYPE PowerRMSTab[] = 
{
    {0x0009, 3, DATAS},
    {0x000a, 3, DATAS+1},
    {0x000b, 3, DATAS+2},
    {0x000c, 3, DATAS+3},   //������

    {0x0001, 3, DATAP},
    {0x0002, 3, DATAP+1},
    {0x0003, 3, DATAP+2},
    {0x0004, 3, DATAP+3},

    {0x0005, 3, DATAQ},
    {0x0006, 3, DATAQ+1},
    {0x0007, 3, DATAQ+2},
    {0x0008, 3, DATAQ+3}
};

// ��ǡ�Ƶ�ʡ��������� E213
struct MIX_CONFIG_CELL_TYPE AngleFreqTab[] = 
{
    {0x0018, 3, DATAUIAngle},   //A��������ѹ�н�
    {0x0019, 3, DATAUIAngle+1},
    {0x001a, 3, DATAUIAngle+2},

    {0x0026, 3, DATAUaUbAngle},  //UA UB �н�
    {0x0027, 3, DATAUaUbAngle+1},  //UA UC �н�
    {0x0028, 3, DATAUaUbAngle+2},  //UB UC �н�

    {0x001c, 3, DATAFreq},   //��Ƶ��

    {0x002a, 3, DATATPSD}, //�¶ȴ����������

    {0x0014, 3, DATACOS},
    {0x0015, 3, DATACOS+1},
    {0x0016, 3, DATACOS+2},
    {0x0017, 3, DATACOS+3}
};
 // ���� E214
struct MIX_CONFIG_CELL_TYPE EnergPluseTab[] = 
{
    {0x001e, 3, DATAEpa},
    {0x001f, 3, DATAEpa+1},
    {0x0020, 3, DATAEpa+2},
    {0x0021, 3, DATAEpa+3},

    {0x0022, 3, DATAEqa},
    {0x0023, 3, DATAEqa+1},
    {0x0024, 3, DATAEqa+2},
    {0x0025, 3, DATAEqa+3},

    {0x0035, 3, DATAEsa},
    {0x0036, 3, DATAEsa+1},
    {0x0037, 3, DATAEsa+2},
    {0x0038, 3, DATAEsa+3}
};
// ״̬ E215
struct MIX_CONFIG_CELL_TYPE StatusSignTab[] = 
{
    {0x001d, 3, DATAEFlag},
    {0x002c, 3, DATASfalg},

    {0x003d, 3, DATAPFlag},
    {0x003e, 3, DATAChknum}
};

// ��ѹ���������������������
struct MIX_CONFIG_CELL_TYPE CalMetConfigTab[] =
{
    // ��ѹ����������
    {0x0017,3,0},
    {0x0018,3,0},
    {0x0019,3,0},
    {0x001a,3,0},
    {0x001b,3,0},
    {0x001c,3,0},
    {0x0020,3,0},
    // ��������
    {0x0004,3,0},
    {0x0005,3,0},
    {0x0006,3,0},
    {0x0007,3,0},
    {0x0008,3,0},
    {0x0009,3,0},

    {0x000d,3,0},
    {0x000e,3,0},
    {0x000f,3,0}
};

/*! 
 * @brief     SPI���ʽӿ��û�̬ʵ�� 
 * @param     *spi_device SPI������Ϣ 
 * @param     *tx_buf     ��������buffer 
 * @param     tx_size     �������ݳ���(�ֽ���)     
 * @param     *rx_buf     ��������buffer ���ؽ��յ�����     
 * @param     rx_size     �������ݳ��ȣ��ֽ�����(SPIֻ�������ݽ��� �������Ҫ�������� rx_size��tx_size��� tpmоƬ����(�����Э�����⴦��))    
 * @retval    int 
 * @return    0 �ɹ� ��0 ʧ�� 
 * @note       
 */ 
int drv_user_spi_transfer (spi_dev_cfg_s *spi_device, char *tx_buf, unsigned int tx_size, char *rx_buf, unsigned int rx_size) 
{ 
    int retval;
    int fd;
    spi_transfer_s spi_data = {0}; 
    unsigned int len; 
    char dev_name[SPI_USER_NAME_MAX_LEN] ={0}; 
 
    /* ������η���ֻҪ�������� ����Ҫ�������� ����buffer����Ϊ�� */ 
    if ((NULL == spi_device) || (NULL == tx_buf) || (0 == tx_size) ||(NULL == spi_device->bus_name) ) 
    { 
        DPRINT("spi transfer invalid para\r\n"); 
        return -1;   /* �˴�Ϊʾ�����룬Ϊ�˷�����ʾ�������쳣����ֵ������Ϊ-1 */ 
    } 
 
    if ((NULL == rx_buf) && (rx_size)) 
    { 
        DPRINT("spi transfer rx buffer or size invalid\r\n"); 
        return -1; 
    } 
    len = strlen(spi_device->bus_name); 
    if ((0 == len) || (len > SPI_NAME_MAX_LEN)) 
    { 
        DPRINT("spi transfer device name invalid\r\n"); 
        return -1; 
    } 
 
    /*����SPI�����û�̬��Ҫ���ݸ��ں�̬������*/ 
    memset(&spi_data, 0, sizeof(spi_data)); 
    spi_data.spi_dev = spi_device; 
    spi_data.rx_buff = rx_buf;   
    spi_data.rx_size = rx_size; 
    spi_data.tx_buff = tx_buf; 
    spi_data.tx_size = tx_size;       
 
    retval = snprintf(dev_name, sizeof(dev_name), "/dev/%s", spi_device->bus_name); 
    if (retval < 0) 
    { 
        DPRINT("spi transfer %s snprintf fail\r\n", spi_device->bus_name); 
        return -1; 
    } 
 
    fd = open(dev_name, O_RDWR | O_CLOEXEC); 
    if (fd < 0) 
    { 
        DPRINT("spi transfer open device %s fail\r\n", spi_device->bus_name); 
        return -1;     
    } 
 
    retval = ioctl(fd, SPI_TRANSFER_DATA, &spi_data); 
 
    if (retval < 0) 
    { 
        DPRINT("spi transfer fail return code:%d\r\n", retval); 
    } 
 
    close(fd); 
 
    return retval; 
}//lint !e429 
 
/*! 
 * @brief     ade9087����оƬSPI���ʽӿ� 
 * @param     *cmd_buf    ��������buffer 
 * @param     cmd_size    ���������(�ֽ���)     
 * @param     *rx_addr    ��������buffer ���ؽ��յ�����     
 * @param     rx_len      �������ݳ���(�ֽ���)     
 * @param     dummy_num   �����������������֮�����ֽ���    
 * @retval    int 
 * @return    0 �ɹ� ��0 ʧ�� 
 * @note       
 */ 
int ade9078_board_spi_read(const void *cmd_buf, unsigned int cmd_size, void *rx_addr, unsigned int rx_len, unsigned int dummy_num) 
{ 
    int retval; 
    spi_dev_cfg_s dev_cfg = {0}; 
    char* data_buf; 
    unsigned int data_len = cmd_size + rx_len + dummy_num; 
 
    if ((cmd_buf == NULL ) || (rx_addr == NULL) || (0 == data_len) || (data_len > SPI_OP_MAX_LEN)) 
    { 
        DPRINT("spi bus read para error ! \r\n"); 
        return -1; 
    } 
 
    data_buf = (char *)malloc(data_len); 
    if (NULL == data_buf) 
    { 
        DPRINT("spi bus read malloc error ! \r\n"); 
        return -1; 
    } 
 
    memset(&dev_cfg, 0, sizeof(dev_cfg)); 
 
#if (__SIZEOF_LONG__ == 4) 
    dev_cfg.pad0 = 0; /***�˴����뱣֤��������Ϊ0�������ں�̬���ʻ�������***/ 
#endif 
    dev_cfg.bus_name = "spi_bus0"; 
    dev_cfg.mode = SPI_DEV_MODE_1; 
    dev_cfg.speed = 400000;   /*SPI���߿�壬�˴�����Ϊ1M*/ 
    dev_cfg.bits_per_word = 8; 
    dev_cfg.protocal_type = SPI_DEV_PROTOCAL_NORMAL; 
    dev_cfg.cs_id = 1;   /*Ƭѡ��ţ���Ӳ������*/ 
 
    memcpy(data_buf, cmd_buf, cmd_size); 
    retval = drv_user_spi_transfer (&dev_cfg, data_buf, data_len, data_buf, data_len); 
    (void)memcpy((char *)rx_addr, data_buf + cmd_size + dummy_num, rx_len); 
 
    free(data_buf); 
 
    return retval; 
} 
 
/*! 
 * @brief     ade9087����оƬSPI���ʽӿ� 
 * @param     *cmd_buf    ��������buffer 
 * @param     cmd_size    ���������(�ֽ���)     
 * @param     *tx_addr    д������buffer ���ؽ��յ�����     
 * @param     tx_len      д�����ݳ���(�ֽ���)     
 * @retval    int 
 * @return    0 �ɹ� ��0 ʧ�� 
 * @note       
 */ 
int ade9078_board_spi_write(const void *cmd_buf, unsigned int cmd_size,const void *tx_addr, unsigned int tx_len) 
{ 
    int retval; 
    spi_dev_cfg_s dev_cfg = {0}; 
    char* data_buf; 
    unsigned int data_len = cmd_size + tx_len; 
 
    if ((cmd_buf == NULL ) || (tx_addr == NULL) || (0 == data_len) || (data_len > SPI_OP_MAX_LEN)) 
    { 
        DPRINT("spi bus read para error ! \r\n"); 
        return -1; 
    } 
 
    data_buf = (char *)malloc(data_len); 
    if (NULL == data_buf) 
    { 
        DPRINT("spi bus read malloc error ! \r\n"); 
        return -1; 
    } 
 
    memset(&dev_cfg, 0, sizeof(dev_cfg)); 
 
#if (__SIZEOF_LONG__ == 4) 
    dev_cfg.pad0 = 0; /***�˴����뱣֤��������Ϊ0�������ں�̬���ʻ�������***/ 
#endif 
    dev_cfg.bus_name = "spi_bus0"; 
    dev_cfg.mode = SPI_DEV_MODE_1; 
    dev_cfg.speed = 400000; 
    dev_cfg.bits_per_word = 8; 
    dev_cfg.protocal_type = SPI_DEV_PROTOCAL_NORMAL; 
    dev_cfg.cs_id = 1; 
 
    memcpy(data_buf, cmd_buf, cmd_size); 
    memcpy(data_buf+cmd_size, tx_addr, tx_len); 
    retval = drv_user_spi_transfer (&dev_cfg, data_buf, data_len, data_buf, data_len); 
    free(data_buf); 
 
    return retval; 
}

/*******************************************************************************
* ��������: WriteReg7022E
* ��������: д7022E�����Ĵ���������д
* �������: addr   �Ĵ�����ַ��MAX_CAL_REG_ADDR
*           spval  ��д��ļĴ�������
* �������: ��
* �� �� ֵ: 0 �ɹ�; < 0 ʧ��
*******************************************************************************/
int WriteReg7022E(WORD addr, DWORD spval)
{
    BYTE sendbuf[4] = {0};

    sendbuf[0] = (BYTE)(addr |0x80);
    sendbuf[1] = ((unsigned long)spval >> 16) & 0xFF;
    sendbuf[2] = ((unsigned long)spval >> 8) & 0xFF; 
    sendbuf[3] = (unsigned long)spval & 0xFF;

    ade9078_board_spi_write(sendbuf, 1, sendbuf+1, 3);
	return 0;
}
/*******************************************************************************
* ��������: ReadReg7022E
* ��������: ��7022E����Ĵ��������ض�
* �������: addr       �Ĵ�����ַ������Ĵ��� 0x00~MAX_OUT_REG_ADDR��У�����Ĵ��� 0x00~MAX_CAL_REG_ADDR
* �������: SPIDataBuf �����ļĴ������ݣ����ֽ���ǰ
* �� �� ֵ: 0 �ɹ�; < 0 ʧ��
*******************************************************************************/
int ReadReg7022E(WORD addr, BYTE * const SPIDataBuf)
{
    BYTE sendbuf[4] = {0};
    BYTE recvbuf[4] = {0};
    
	if (SPIDataBuf == NULL)
	{
		return -1;
	}
	sendbuf[0] = addr & 0x7F;
     
    ade9078_board_spi_read(sendbuf, 1, recvbuf+1, 3,0);

	SPIDataBuf[0] = recvbuf[3];
	SPIDataBuf[1] = recvbuf[2];
	SPIDataBuf[2] = recvbuf[1];
	SPIDataBuf[3] = recvbuf[0];
	return 0;
}

/*******************************************************************************
* ��������: WriteContFun7022E
* ��������: У��дʹ�ܴ򿪻�ر�
* �������: Mark 0 У��дʹ�ܴ򿪣�1 У��дʹ�ܹر�
* �������: ��
* �� �� ֵ: >=0:����    -1:����
*******************************************************************************/
int WriteContFun7022E(WORD Mark)
{
    int i = 0;
    const struct MIX_CONFIG_CELL_TYPE RunStopConfigTab[]=
    {
        {0x00C9,3,0x00005A},
        {0x00C9,3,0x000001}
    };
    
    if(Mark != 0)
    {
        i = 1;
    }

    if (WriteReg7022E(RunStopConfigTab[i].Addr, (DWORD)RunStopConfigTab[i].Data) < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

// ATT7022 CPOL = 0; CPHA = 1;����Ϊ�� ,�����ط����ݣ��½���ȡ����
int  Reset_ACChip(void)
{
	int i;
	DWORD chipid;

	//�ϵ�
	Power_ATT7022E(1);
	usleep(500);
	//��λ
	Power_ATT7022E(3);
	usleep(500);

	for( i = 0 ; i < 3; i++)
	{
		ReadReg7022E(0,(BYTE*)&chipid);
		if(chipid == 0x007122A0)
		{
			myprintf(SYS_ID, LOG_ATTR_ERROR,"at7022 device ID %08X\n",chipid);
			return OK;
		}
	}
	myprintf(SYS_ID, LOG_ATTR_ERROR,"Ӳ��λ7022Eʧ��!!! \n");
	WriteWarnEvent(NULL, SYS_ERR_AD, 0, "Ӳ��λ7022Eʧ��!!! \n");
	return ERROR;
}

int Power_ATT7022E(STATUS iType)
{
	BYTE data;
	BYTE regaddr;
	BYTE id = 0x22;

	regaddr = 0x06;
	I2CReceBuffer_BAddr(&data, id, regaddr, 1);
	data = data&0x7F;
	I2CSendBuffer_BAddr(&data, id, regaddr, 1);

	regaddr = 0x07;
	I2CReceBuffer_BAddr(&data, id, regaddr, 1);
	data = data&0xFE;
	I2CSendBuffer_BAddr(&data, id, regaddr, 1);
	
	if(iType == 1)
	{
		//�ϵ�
		regaddr = 0x03;
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data|0x01;	//ֻ�ı����״̬λ������
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
	}
	else if(iType == 2)
	{
		//�µ�
		regaddr = 0x03;
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data&0xFE;	//ֻ�ı����״̬λ������
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
	}
	else if(iType == 3)
	{
		//��λ
		regaddr = 0x02;
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data&0x7F;	//ֻ�ı����״̬λ������
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
		usleep(100000);
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data|0x80;	//ֻ�ı����״̬λ������
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
		usleep(100000);
	}
	return 0;
}

/*******************************************************************************
* ��������: EnableTempCurrMeter
* ��������: ʹ���¶ȡ���������(�����ϵ��״μ���¶����������)
* �������: ��
* �������: ��
* �� �� ֵ: 0 �ɹ�; -1 ʧ��
*******************************************************************************/
int EnableTempCurrMeter(void)
{
    int i;
    // ʹ���¶ȡ���������������
    const struct MIX_CONFIG_CELL_TYPE EnableTempCurrTab[] =
    {
        {0x0001,0x03,0x00B97F},        // ģʽ���üĴ���
        {0x0031,0x03,0x000012}        // ��·ģ�����üĴ���
    };

    if (WriteContFun7022E(0x00) < 0)   // ��дʹ��
    {
        return -1;
    }
    for (i = 0; i < (int)(sizeof(EnableTempCurrTab) / sizeof(struct MIX_CONFIG_CELL_TYPE)); i++)
    {
        if (WriteReg7022E(EnableTempCurrTab[i].Addr, (DWORD)EnableTempCurrTab[i].Data) < 0)
        {
            WriteContFun7022E(0x01);   // �ر�дʹ��
            shellprintf("ʹ���¶ȡ���������Error!!!\n");
            myprintf(SYS_ID, LOG_ATTR_ERROR, "ʹ���¶ȡ���������Error!!!");
            return -1;
        }
    }
    WriteContFun7022E(0x01);           // �ر�дʹ��

    shellprintf("ʹ���¶ȡ���������OK!!!\n");

    return 0;
}

/*******************************************************************************
* ��������: FixupConfigFun7022E
* ��������: ATT7022E�̶����ñ�����������֯
* �������: step 0 ��ʼ����1 д�̶�����
* �������: ��
* �� �� ֵ: ��֯���ݳ���
*******************************************************************************/
/*���� �����������������ʡ���λ����*/
int FixupConfigFun7022E(WORD step)
{
    WORD i = 0;
    WORD RegCnt;
    int    retnum = 0;
    float  Un;       // ���ѹ

    Un = 220;    // ���ѹ 220V
    FixupConfigTab = FixupConfigTab3;
    RegCnt = (WORD)(sizeof(FixupConfigTab3)/sizeof(struct MIX_CONFIG_CELL_TYPE));

    if (step == 0)       // �ָ�У����ֵ���ϵ��ʼֵ
    {
        if (WriteReg7022E(FixupConfigTab[0].Addr, (DWORD)FixupConfigTab[0].Data) < 0)
        {
            retnum = -1;
        }
    }
    else                 // �̶����ã��ӵڶ����Ĵ�����ʼ
    {
        for(i = 1; i < RegCnt; i++)
        {
            if (WriteReg7022E(FixupConfigTab[i].Addr, (DWORD)FixupConfigTab[i].Data) < 0)
            {
                retnum = -1;
                break;
            }
        }
        if (i >= RegCnt)
        {
            // �����Ƶ���峣������ֵ���·����Ĵ���
            HFconst = (WORD)(110668277*2/ (ACPulseConst * Un*(RATED_CURR_VALUE/1.5)));
            if (WriteReg7022E(0x9E, (DWORD)HFconst) < 0)
            {
                retnum = -1;
            }
        }
    }
    return retnum;
}

/*******************************************************************************
* ��������: CalMetConfigFun7022E
* ��������: �·�У�����--ATT7022E
* �������: ��
* �������: ��
* �� �� ֵ: >=0 ������< 0 ʧ��
*******************************************************************************/
int CalMetConfigFun7022E()
{
    short retnum = 0;
    int i, j = 0;
    DWORD RegAddr = 0;
    WORD gainno;
    WORD tempno[16] = {0};
	int val,idx;
    

    for(i = 0 ; i < ycNum; i++ )
    {

        gainno = (WORD)pYcCfg[i].gainNo;
        if(pYcCfg[i].type == YC_TYPE_Ua)
        {
            CalMetConfigTab[0].Data = pYcGain[gainno].a;
            tempno[0] = gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Ub)
        {
            CalMetConfigTab[1].Data = pYcGain[gainno].a;
             tempno[1]  = gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Uc)
        {
            CalMetConfigTab[2].Data = pYcGain[gainno].a;
             tempno[2] =  gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Ia)
        {
            CalMetConfigTab[3].Data = pYcGain[gainno].a;
             tempno[3] = gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Ib)
        {
            CalMetConfigTab[4].Data = pYcGain[gainno].a;
             tempno[4] = gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Ic)
        {
            CalMetConfigTab[5].Data = pYcGain[gainno].a;
             tempno[5] = gainno;
        }
	   if(pYcCfg[i].type == YC_TYPE_I0)
        {
            CalMetConfigTab[6].Data = pYcGain[gainno].a;
             tempno[6] = gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Pa)
        {
            CalMetConfigTab[7].Data = pYcGain[gainno].a;
             tempno[7] =  gainno;

             CalMetConfigTab[13].Data = pYcGain[gainno].b;
             tempno[13] =  gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Pb)
        {
            CalMetConfigTab[8].Data = pYcGain[gainno].a;
             tempno[8] = gainno;
             
             CalMetConfigTab[14].Data = pYcGain[gainno].b;
             tempno[14] =  gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Pc)
        {
            CalMetConfigTab[9].Data = pYcGain[gainno].a;
             tempno[9] = gainno;

             CalMetConfigTab[15].Data = pYcGain[gainno].b;
             tempno[15] =  gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Qa)
        {
            CalMetConfigTab[10].Data = pYcGain[gainno].a;
             tempno[10] =  gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Qb)
        {
            CalMetConfigTab[11].Data = pYcGain[gainno].a;
             tempno[11] =  gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Qc)
        {
            CalMetConfigTab[12].Data = pYcGain[gainno].a;
             tempno[12] = gainno;
        }

       
    }

    // �·���ѹ�������������������
    for(i = 0; i < (int)(sizeof(CalMetConfigTab)/sizeof(struct MIX_CONFIG_CELL_TYPE)); i++)
    {
        if(!(pYcGain[tempno[i]].status & YC_GAIN_BIT_VALID))
        {
            continue;
        }

        if (WriteReg7022E(CalMetConfigTab[i].Addr, (DWORD)CalMetConfigTab[i].Data) < 0)
        {
            shellprintf("�·���ѹ������������������� Error!!! Addr=%X Data=%X\n"
                , (unsigned)CalMetConfigTab[i].Addr, (unsigned)CalMetConfigTab[i].Data);
            return -1;
        }
    }

    // �·�ȫͨ������
	val = (int)p7022gain->allGain[TempInfo7022E.TempZone]&0xFFFF;
    if (WriteReg7022E(0x32, (DWORD)val) < 0)
    {
        shellprintf("��ʼ��дȫͨ������ Error! HisTempSection=%d Gain=%u\n", TempInfo7022E.TempZone, val);
        myprintf(SYS_ID, LOG_ATTR_ERROR, "��ʼ��дȫͨ������ Error! CurTempSection=%d Gain=%u"
            ,TempInfo7022E.TempZone, val);
    }
    else
    {
        // ���㵱ǰ�¶�����Ӧȫͨ������
        AllGainVal = val;
    }
    // �·���λУ������
    for (i = 0; i < 3; i++)
    {
      
		idx = CurrentInfo7022E.IZone[i] + TempInfo7022E.ITempZone*I_ZONE_NUM;

        RegAddr =(DWORD)(((CurrentInfo7022E.IZone[i] >= 2)?0x0D:0x10) + i);  // �Ĵ�����ַ
        j = (CurrentInfo7022E.IZone[i] >= 2)?0:3;    // ����ƫ�ƣ�����2~3 ��ƫ�ƣ�����0~2 ��Ӧ0x90~0x92�Ĵ���
        val = (int)p7022gain->allIAng[i][idx] & 0xFFFF;
		
        // д����оƬ�ǲ���Ĵ���
        if (WriteReg7022E((WORD)RegAddr, (DWORD)val) < 0)
        {
            retnum = -2;
            shellprintf("��ʼ��д�ǲ���Ĵ���ʧ��!");
        
            // дʧ�ܺ���д�ϴ�ֵ����ֹ�Ĵ�����ֵ����
            WriteReg7022E((WORD)RegAddr, ITempRegVal[i+j]);
        }
        else
        {
            // д�ɹ��󣬼�¼��Ӧ��ֵ�����ر���
            ITempRegVal[i+j] = (DWORD)val; 
        }

		
        // ���������ڵͶ��Ҹ߶�û�в�������д100%�����
        if ((CurrentInfo7022E.IZone[i] <= 1) && (ITempRegVal[(3-j)+i] == 0))
        {
            idx =  2 + TempInfo7022E.ITempZone*I_ZONE_NUM;
            ITempRegVal[(3-j)+i] = (int)p7022gain->allIAng[i][idx]&0xFFFF;
			RegAddr = (DWORD)(0x0D + i);
        }

        // ���������ڸ߶��ҵͶ�û�в�������д20%�����
        if ((CurrentInfo7022E.IZone[i] >= 2) && (ITempRegVal[(3-j)+i] == 0))
        {
            idx = 1 + TempInfo7022E.ITempZone*I_ZONE_NUM;
            ITempRegVal[(3-j)+i] = (int)p7022gain->allIAng[i][idx]&0xFFFF;
            RegAddr = (DWORD)(0x10 + i);
        }
        
        if (ITempRegVal[(3-j)+i] != 0)
        {
            // д����оƬ�෴�ǲ���Ĵ���
            if (WriteReg7022E((WORD)RegAddr, ITempRegVal[(3-j)+i]) < 0)
            {
                retnum = -2;
            }
        }
    }
    return retnum;
}

/*******************************************************************************
* ��������: FixupConfigEMU7022E
* ��������: дEMU���ã�ʹ����������
* �������: flag:WRITE_REG_FLAG д�Ĵ���, CLEAR_REG_FLAG ��Ĵ���
* �������: ��
* �� �� ֵ: >=0:����    <=-1:����
*******************************************************************************/
int FixupConfigEMU7022E()
{
    DWORD EMUData;
    int retnum = 0;

    EMUData = 0x003DC4;  // ��������EMU

    if (WriteReg7022E(0x03, EMUData) < 0)
    {
        shellprintf( "��ʼ��EMU���ô���!!! \n");
        retnum = -1;
    }

    return retnum;
}


/*******************************************************************************
* ��������: ComInitATTFun7022E
* ��������: ͨ�ų�ʼ��ATT7022����
* �������: ��
* �������: ��
* �� �� ֵ: >=0:����    <=-1:����
*******************************************************************************/
int ComInitATTFun7022E(void)
{
    int retnum = 0;

     // �����ʼ��
     shellprintf("ATT7022E �����ʼ��\n");
     retnum += FixupConfigFun7022E(0);
     thSleep(1);

	 // �̶�����
     shellprintf("ATT7022E д�̶�����\n");
     retnum += FixupConfigFun7022E(1);

     // У������
     shellprintf("ATT7022E дУ������\n");
     retnum += CalMetConfigFun7022E();
            
     // EMU���ã�ʹ����������
     shellprintf("ATT7022E дEMU����\n");
     thSleep(60);  // ��Ҫ500ms����
     retnum += FixupConfigEMU7022E();
   

	if(retnum < 0)
       return -1;
    
    return retnum;
}

/*******************************************************************************
* ��������: InitMeaICFun7022E
* ��������: ����оƬ��ʼ��
* �������: ��
* �������: ��
* �� �� ֵ: >=0:����    <=-1:����
*******************************************************************************/
int InitMeaICFun7022E(void)
{    
    int retnum = 0;

    //retnum += ReadMeaICModeMark7022E();    // �жϽ��ɽ��߷�ʽ���ж����������ߡ��������ߡ�57.7V��
    retnum += WriteContFun7022E(0x00);     // ʹ��SPIУ������д����
    if(retnum < 0)
    {
        shellprintf("��ʼ��7022E��дʹ��ʧ��!!!\n");
        return -1;
    }
    retnum += ComInitATTFun7022E();        // ��֯ATT7022E�Ŀ��ƼĴ�������
    retnum += WriteContFun7022E(0x01);     // 7022У��Ĵ���дʹ�ܹر�
    
    return retnum;
}


/*******************************************************************************
* ��������: ReadACInstDataFun7022E
* ��������: ��ȡATT7022��ʵʱ����Ĵ�������
* �������: DIMark ���ݱ�ʶ
* �������: ��
* �� �� ֵ: 1 ������ -1 �쳣
*******************************************************************************/
int ReadACInstDataFun7022E(WORD DIMark)
{
       
    int i;
	if(Att7022eInit != 1)
		return -1;
	
    if (DIMark & ATT7022_REG_UI)
    {
        for(i = 0; i < (int)(sizeof(VoltCurrRMSTab)/sizeof(struct MIX_CONFIG_CELL_TYPE)); i++)
        {
            if (ReadReg7022E(VoltCurrRMSTab[i].Addr, (BYTE*)&InstantSampData7022E[VoltCurrRMSTab[i].Data]) < 0)
            {
                return -1;
            }
        }
    }

	if (DIMark & ATT7022_REG_PQ)
   	{
        for(i = 0; i < (int)(sizeof(PowerRMSTab)/sizeof(struct MIX_CONFIG_CELL_TYPE)); i++)
        {
            if (ReadReg7022E(PowerRMSTab[i].Addr, (BYTE*)&InstantSampData7022E[PowerRMSTab[i].Data]) < 0)
            {
                return -1;
            }
        }
	}

    if (DIMark & ATT7022_REG_OTHYC)
    {
        for(i = 0; i < (int)(sizeof(AngleFreqTab)/sizeof(struct MIX_CONFIG_CELL_TYPE)); i++)
        {
            if (ReadReg7022E(AngleFreqTab[i].Addr, (BYTE*)&InstantSampData7022E[AngleFreqTab[i].Data]) < 0)
            {
                return -1;
            }
        }
    }

    if (DIMark & ATT7022_REG_E)
    {
        for(i = 0; i < (int)(sizeof(EnergPluseTab)/sizeof(struct MIX_CONFIG_CELL_TYPE)); i++)
        {
            if (ReadReg7022E(EnergPluseTab[i].Addr, (BYTE*)&InstantSampData7022E[EnergPluseTab[i].Data]) < 0)
            {
                return -1;
            }
        }
    }

    if (DIMark & ATT7022_REG_STS)
    {
        for(i = 0; i < (int)(sizeof(StatusSignTab)/sizeof(struct MIX_CONFIG_CELL_TYPE)); i++)
        {
            if (ReadReg7022E(StatusSignTab[i].Addr,(BYTE*)& InstantSampData7022E[StatusSignTab[i].Data]) < 0)
            {
                return -1;
            }
        }
    }

    return 0;
}

void Real_U(int i ,  long* data)
{// 100��
	DWORD *pSrc;
	DWORD *pDest;
	INT64U MidNum;

	pSrc =(DWORD*)&  InstantSampData7022E[i]; //DATAU = 0
	pDest = (DWORD*)data; 

	MidNum = pSrc[0];

	MidNum *= 100;
	if(i == 3)
		MidNum >>= 12;
	else
		MidNum >>= 13;

	*pDest = (DWORD)MidNum;

	UAtt7022e[i] = (long)MidNum;
  
}

void Real_I(int i ,  long* data)
{ // 1000 ��
	DWORD *pSrc;
	int *pDest;
	INT64U MidNum;
	DWORD N = 60/RATED_CURR_VALUE; // NΪ����ϵ�����������Ibȡ��Ϊ50mVʱN=60/Ib

	pSrc = (DWORD*)& InstantSampData7022E[DATAI+i];
	pDest = (int*)data;

	MidNum = ((INT64S)(pSrc[0]) * 1000);    //ϵ���������
	MidNum >>= 13;

	pDest[0] = (int)(MidNum/N);
	
	IAtt7022e[i] = (long)(MidNum/N);
}

int PowerCalSubProgram(int PowSrcNum, BYTE CalMark)
{// �Ŵ�10��
    INT64S MidNum;
	
	if(Att7022eInit != 1)
		return 0;	
	if(PowSrcNum > (1 << 23))
	{
		PowSrcNum = PowSrcNum - (1 << 24);
	}
	

    /* ����У��ϵ�� */
    
       ////////////////////2592 ����д 2018��2��9�� 09:01:30///////////////////////////////
	MidNum = 2592*100000/(HFconst*ACPulseConst/100);
	if (CalMark == 0)                        //���๦��(�������)����ʹ��
        MidNum = (INT64S)((INT64S)PowSrcNum *10 * MidNum /(1 << 23));
    else if (CalMark == 1)                    //�ܹ���(�������)����ʹ��
        MidNum = (INT64S)((INT64S)PowSrcNum *10 * MidNum /(1 << 22));
    
    //MidNum /= PulsConst;
	//shellprintf("PQ	ԭʼ%d    ���� %d \n", PowSrcNum , MidNum);
		
    return (int)MidNum;
}

void Real_PQ(int num,long* data)
{
        switch(num)
        {
            case 0:
                *data = PowerCalSubProgram(InstantSampData7022E[DATAP], 0) ;
                break;
            case 1:
                *data= PowerCalSubProgram(InstantSampData7022E[DATAP+1], 0) ;
                break;
            case 2:
                *data = PowerCalSubProgram(InstantSampData7022E[DATAP+2], 0) ;
                break;
            case 4:
                *data = PowerCalSubProgram(InstantSampData7022E[DATAQ], 0) ;
                break;
             case 5:
                *data = PowerCalSubProgram(InstantSampData7022E[DATAQ+1], 0) ;
                break;
             case 6:
                *data = PowerCalSubProgram(InstantSampData7022E[DATAQ+2], 0) ;
                break;
            case 3:
                 *data = PowerCalSubProgram(InstantSampData7022E[DATAP+3], 1);
                 break;
            case 7:
                *data = PowerCalSubProgram(InstantSampData7022E[DATAQ+3], 1);
            break;
            case 8:
                *data = PowerCalSubProgram(InstantSampData7022E[DATAS+3], 1);
              break;
			default:
				break;
        }
        PQAtt7022e[num] = *data;
}

void ReadCos(long*data)
{
    DWORD ulcos = (DWORD)InstantSampData7022E[DATACOS+3];
    int temp;
    long temp1;
    ulcos = ulcos & 0xffffff;  //24λ��
    if(ulcos >= (1 << 23))
      temp= (int)(ulcos - (1<<24));
    else
      temp = (int)ulcos;
    temp1 = (temp*125)/(1<<20);  // (temp*1000)/(1<<23)
    
    if(temp1 > 1000)
      temp1 = 999;
    else  if(temp1 < -1000)
      temp1 = -999;
     
    *data = temp1;
}

void ReadFreq(long* data)
{
		DWORD freq;
		freq = (DWORD)((InstantSampData7022E[DATAFreq]*100)/1024)/8;

		if((UAtt7022e[0] < 1000) && (UAtt7022e[1] < 1000) && (UAtt7022e[2] < 1000))
			*data = 5000;
		else
			*data = (long)freq;
}

//���¶�
int  ReadTemp(void)
{
    long temp,cmptemp;
	
	if(Att7022eInit != 1)
				return 0;

    ReadReg7022E(0x2A , (BYTE*)&InstantSampData7022E[DATATPSD]);
		
    if(InstantSampData7022E[DATATPSD] & 0x80)
        temp = (long)((InstantSampData7022E[DATATPSD] & 0x000000ff)|0xffffff00);
    else
        temp = (InstantSampData7022E[DATATPSD] & 0x000000ff); 

    cmptemp = p7022gain->TC - temp*726/1000;

	TempInfo7022E.TempCur =  (short)cmptemp;

	if (cmptemp <= TEMP_NORMAL_LOW)
        TempInfo7022E.ITempZone = I_LOWT_ZONE;
	else if (cmptemp >= TEMP_NORMAL_HIGH)
		TempInfo7022E.ITempZone = I_HIGHT_ZONE;
	else 
		TempInfo7022E.ITempZone = I_NORT_ZONE;

	return TempInfo7022E.TempCur;
}

void JudgeIZone(short I, short In, int phase)
{
    int IZone = 0;
	
	if ((phase > 2) || (phase < 0)) 
	{
		return;
	}
	
	if (abs(I - CurrentInfo7022E.IBak[phase]) > In * 0.01) // I�ı�������, hll���޸�
    {
	    if (I > CurrentInfo7022E.IBak[phase]) // �������ϱ仯
	    {
	        if (I > (In * 2.75))
	        {
	            IZone = 4;
	        }
	        else if (I > (In * 1.55))
	        {
	            IZone = 3;
	        }
	        else if (I > (In * 0.65))
	        {
	            IZone = 2;
	        }
	        else if (I > (In * 0.122))
	        {
	            IZone = 1;
            }
            else
	        {
	            IZone = 0;
	        }
	    }
	    else  // �������±仯
	    {
	        if (I < (In * 0.118))
	        {
	            IZone = 0;
	        }
	        else if (I < (In * 0.6))
	        {
	            IZone = 1;
	        }
	        else if (I < (In * 1.5))
	        {
	            IZone = 2;
	        }
	        else if (I < (In * 2.7))
	        {
	            IZone = 3;
	        }
	        else
	        {
	            IZone = 4;
	        }
	    }
		CurrentInfo7022E.IZone[phase] = (BYTE)IZone;
    }
    CurrentInfo7022E.IBak[phase] = I;	
}


int PhaRegDeal(BYTE idex, BYTE hisZone, BYTE curZone)
{
 
    if (gain7022Valid !=0) 
        return -1;
    
    if (curZone == hisZone)
        return -1;

    if ((hisZone <= 1) && (curZone <= 1))       // 5% <--> 20%
    {
        if (ITempRegVal[3+idex] == 0)        // ����ǰ��Ӧ�ļĴ���û��ֵ�����������Ӧ�ļĴ���ֵ 
            return -1;

		if(WriteContFun7022E(0x00) < 0)
           return -1;

        if (WriteReg7022E(0x10 + idex, 0x00000000) < 0)
            return -2;
         
        ITempRegVal[3+idex] = 0;
        WriteContFun7022E(0x01);
        return 0;
    }
    else if ((hisZone >= 2) && (curZone >= 2))  // 100% <--> 200% <--> 400%
    {
        if (ITempRegVal[idex] == 0)        // ����ǰ��Ӧ�ļĴ���û��ֵ�����������Ӧ�ļĴ���ֵ 
            return -1;
		
        if(WriteContFun7022E(0x00) < 0)
           return -1;
		
        if (WriteReg7022E(0x0D + idex, 0x00000000) < 0)
            return -2;

        ITempRegVal[idex] = 0;
        WriteContFun7022E(0x01);
        return 0;
    }
    else
    {
        return -1;
    }
}

int ITempGainReg(void)
{
    int i,idex,j;
	WORD addr;
	int WriteStart=0;
	static int TempZoneBak=0;
    static int IAltCnt[3]={0};
	static int IZoneBak[3]={2,2,2};
	if(Att7022eInit != 1)
			return -1;
	for (i=0; i<3; i++)
	{
	    if (((CurrentInfo7022E.IZone[i] == IZoneBak[i])&&
			(CurrentInfo7022E.IZone[i]!= CurrentInfo7022E.IZoneBak[i]))||
			((TempInfo7022E.ITempZone == TempZoneBak)&&
			(TempInfo7022E.ITempZone != TempInfo7022E.ITempZoneBak)))
	    {
	        IAltCnt[i]++;
			if (IAltCnt[i] >= 2)
			{
			   IAltCnt[i] = 0;
			  /* ������
			  if (PhaRegDeal(i, CurrentInfo7022E.IZoneBak[i], CurrentInfo7022E.IZone[i]) >= 0) // У��״̬�£���������仯�����ж��Ƿ�����ͬ�Ĵ���
	           {
	                CurrentInfo7022E.IZoneBak[i] = CurrentInfo7022E.IZone[i];
	                continue;
	           }*/
               idex = CurrentInfo7022E.IZone[i] + TempInfo7022E.ITempZone*I_ZONE_NUM;

			   addr = (CurrentInfo7022E.IZone[i] >= 2) ? 0x0D:0x10;
			   j = (CurrentInfo7022E.IZone[i] >= 2)?0:3;

               if (p7022gain->allIAng[i][idex] == (short)ITempRegVal[i+j])
               {
                   CurrentInfo7022E.IZoneBak[i] = CurrentInfo7022E.IZone[i];
                   TempInfo7022E.ITempZoneBak = TempInfo7022E.ITempZone;     
                   continue;
               }

			   if (WriteStart == 0)   // �״�д�ǲ�������Ҫдʹ��
               {
                    if(WriteContFun7022E(0x00) < 0)
                       break;
					thSleep(1);
               }
			   if (WriteReg7022E(addr, (unsigned)(long)p7022gain->allIAng[i][idex]) < 0)
			   	   WriteReg7022E(addr, ITempRegVal[i+j]);
			   else
			   {
			       CurrentInfo7022E.IZoneBak[i] = CurrentInfo7022E.IZone[i];
                   TempInfo7022E.ITempZoneBak = TempInfo7022E.ITempZone; 
				   ITempRegVal[i+j] = (unsigned)(long)p7022gain->allIAng[i][idex];
				   WriteStart = 1;
			   }
			}
	    }
		else
		{
		     IAltCnt[i] = 0;
             IZoneBak[i] = CurrentInfo7022E.IZone[i]; 
		}
	}
	
	TempZoneBak = TempInfo7022E.ITempZone;
	if (WriteStart)
	   WriteContFun7022E(0x01);
	
	return 0;
}

int CheckTempZone(void)
{
	static int tempAltCnt=0;


    // �����¶����䣬��Ϊ��ǰ�¶��쳣���¶����䲻��ı�
    if ((TempInfo7022E.TempCur < -50)||(TempInfo7022E.TempCur > 90))
    {
        tempAltCnt = 0;
        return 0;
    }

    // �¶��ޱ仯���¶����䲻��ı�
    if (TempInfo7022E.TempCur == TempInfo7022E.TempBak)
    {
        tempAltCnt = 0;
        return 0;
    }

    if (TempInfo7022E.TempCur > TempInfo7022E.TempBak)
    {
        TempInfo7022E.TempZone = (char)(((TempInfo7022E.TempCur - TEMP_LOW_CMP) + 2)/TEMP_STEP_CMP);
    }    
    else
    {
        TempInfo7022E.TempZone = (char)(((TempInfo7022E.TempCur - TEMP_LOW_CMP) + 3)/TEMP_STEP_CMP);
    }
    
    if (TempInfo7022E.TempZone < 0)
    {
        TempInfo7022E.TempZone = 0;
    }

    if (TempInfo7022E.TempZone > TEMP_ZONE_MAX)
    {
        TempInfo7022E.TempZone = TEMP_ZONE_MAX;
    }

    // ��ǰ�¶����估��ʷ�¶���������ڳ��������¶����䲻��ı� 0~50�Ȳ����²�,10~50
    if(((TempInfo7022E.TempZone >= TEMP_ZONE_NOR_LOW)
        && (TempInfo7022E.TempZone <= TEMP_ZONE_NOR_HIGH))
        && ((TempInfo7022E.TempZoneBak >= TEMP_ZONE_NOR_LOW)
        && (TempInfo7022E.TempZoneBak <= TEMP_ZONE_NOR_HIGH)))
    {
        tempAltCnt = 0;
        return 0;
     }
    
    if (TempInfo7022E.TempZone != TempInfo7022E.TempZone)
    {
        tempAltCnt++;

        // ����5���¶����䲻һ�£�����������¶�����
        if (tempAltCnt >= 5)
        {
            tempAltCnt = 0;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }        
}

/*******************************************************************************
* ��������: AllChannelGain
* ��������: ���ݵ�ǰ�¶���������ȫͨ������Ĵ���
* �������: ��
* �������: ��
* �� �� ֵ: >= ���óɹ���< 0������ʧ�� 
*******************************************************************************/
static int AllChannelGain()
{
	int val;  
		if(Att7022eInit != 1)
			return -1;
    val  = (int)p7022gain->allGain[TempInfo7022E.TempZone];

    if (AllGainVal == val)
    {
        TempInfo7022E.TempZoneBak = (BYTE)TempInfo7022E.TempZone;
		return 0;
    }

	  // У��Ĵ���дʹ�ܿ���
    if (WriteContFun7022E(0x00) < 0)
    {
        WriteContFun7022E(0x01);
        return -1;
    }
	
    if (WriteReg7022E(0x32, (DWORD)val) < 0)
    {
        WriteContFun7022E(0x01);
        return -1;
    }
    else
       TempInfo7022E.TempZoneBak = (BYTE)TempInfo7022E.TempZone;
    
    // У��Ĵ���дʹ�ܹر�
    WriteContFun7022E(0x01);
    
   return 0; 
	
}

int TempAllGainReg(void)
{
    int retnum = 0;
    
    if (CheckTempZone() > 0)
    {
        retnum = AllChannelGain();
    }
    
    return retnum;
}


void SetDefault7022Gain(V7022Gain *pgain)
{
    int i,j;
    memcpy(pgain->allGain, TempAllgainDefault, TEMP_ZONE_MAX*2);
	memset(pgain->allIAng, 0, 3*15*2);
    for(i = 0 ; i < 6 ; i++) // г������ȱʡ����
    {
        for(j = 0; j < 20 ; j++)
        {
            pgain->HarmGain[i][j] = (short)DefaultHarmParaTab[j];
        }
    }
	pgain->TC = TC_DEFAULT_7022E;
}

int ATT7022GainEEWrite(V7022Gain *pgain)
{
    int ret,len;
    WORD crc;
	V7022Gain pgainread;
	DWORD ver = ATT7022E_GAIN_VER1;

	len = sizeof(V7022Gain);

	crc = GetParaCrc16((BYTE*)pgain, len-2);
	pgain->crc = crc;

	ret = extNvRamSet(NVRAM_AD_ATT_GAIN, (BYTE*)&ver, 4);
	if (ret != OK)
	{
	    thSleep(1);
	    extNvRamSet(NVRAM_AD_ATT_GAIN, (BYTE*)&ver, 4);
	}

	ret = extNvRamSet(NVRAM_AD_ATT_GAIN+4, (BYTE*)pgain, len);
	if (ret != OK)
	{
	    thSleep(1);
	    extNvRamSet(NVRAM_AD_ATT_GAIN+4, (BYTE*)pgain, len);
	}

	extNvRamGet(NVRAM_AD_ATT_GAIN, (BYTE*)&ver, 4);
	if (ver != ATT7022E_GAIN_VER1)
	{
	    return ERROR;
	}
	extNvRamGet(NVRAM_AD_ATT_GAIN+4, (BYTE*)&pgainread, len);
	crc = GetParaCrc16((BYTE*)&pgainread, len-2);
	if (crc != pgainread.crc)
	{
	    return ERROR;
	}
	return OK;
}

int ATT7022GainEEGet(V7022Gain *pgain)
{
    WORD crc;
	DWORD ver = 0;
	int len;

	extNvRamGet(NVRAM_AD_ATT_GAIN, (BYTE*)&ver, 4);
	if (ver != ATT7022E_GAIN_VER1)
	{
	    extNvRamGet(NVRAM_AD_ATT_GAIN, (BYTE*)&ver, 4);
		if (ver != ATT7022E_GAIN_VER1)
			return ERROR;
	}

    len = sizeof(V7022Gain);
    extNvRamGet(NVRAM_AD_ATT_GAIN+4, (BYTE*)pgain, len);
	crc = GetParaCrc16((BYTE*)pgain, len-2);
	if (crc != pgain->crc)
	{
	    extNvRamGet(NVRAM_AD_ATT_GAIN+4, (BYTE*)pgain, len);
	    crc = GetParaCrc16((BYTE*)pgain, len-2);
		if (crc != pgain->crc)
			return ERROR;
	}
	return OK;
}

int ATT7022GainFileRead(V7022Gain *pgain, int len)
{
    FILE *fp;
	DWORD ver;
	char path[4*MAXFILENAME];
	
	GetMyPath(path, "gain7022.sys");
	fp = fopen(path, "rb");
	if (fp == NULL) return ERROR;

	if (sizeof(DWORD) != fread(&ver, 1, sizeof(DWORD), fp))
	{
		fclose(fp);
		return ERROR;
	}
	if (ver != ATT7022E_GAIN_VER1)
	{
		fclose(fp);
		return ERROR;
	}
	if ((DWORD)len != fread(pgain, sizeof(BYTE), (DWORD)len, fp))
	{
		fclose(fp);
		return ERROR;
	}
	if (pgain->crc != GetParaCrc16((BYTE*)pgain, len-2))
	{
		fclose(fp);
		return ERROR;
	}

	fclose(fp);
	return OK;
}

int ATT7022GainFileWrite(const V7022Gain *pgain, int len)
{
    FILE *fp;
	char path[4*MAXFILENAME];
	DWORD ver;
	
	GetMyPath(path, "gain7022.sys");
	fp = fopen(path, "wb");
	if (fp == NULL) return ERROR;

   	ver = ATT7022E_GAIN_VER1;
	if (sizeof(DWORD) != fwrite(&ver, 1, sizeof(DWORD), fp))
	{
		fclose(fp);
		return ERROR;
	}
	
	if ((DWORD)len != fwrite(pgain, sizeof(BYTE), (DWORD)len, fp))
	{
		fclose(fp);
		return ERROR;
	}
	fflush(fp);
	fsync(fileno(fp));
	
    fclose(fp);
	return OK;
}

int ATT7022GainRead(void)
{
	V7022Gain pgain;
	
	if (ATT7022GainEEGet(&pgain) == OK)
	{
	    ATT7022GainFileWrite(&pgain, sizeof(V7022Gain));
		return OK;
	}

	return ERROR;
}

int ATT7022GainWrite(void)
{
	int len;
	V7022Gain pgain;

	len = sizeof(V7022Gain);
	if (ATT7022GainFileRead(&pgain, len) == ERROR) 
		return ERROR;
	if (ATT7022GainEEWrite(&pgain) == ERROR) 
		return ERROR;

	return OK;

}

int ATT7022GainClear(void)
{
    DWORD ver = 0;
	if(Att7022eInit != 1)
			return -1;
  	ATT7022GainRead();
	extNvRamSet(NVRAM_AD_ATT_GAIN, (BYTE*)&ver, 4);
	ver = 1;
	extNvRamGet(NVRAM_AD_ATT_GAIN, (BYTE*)&ver, 4);
	if (ver != 0)
	{
	    extNvRamSet(NVRAM_AD_ATT_GAIN, (BYTE*)&ver, 4);
	}

	SetDefault7022Gain(p7022gain);
	return OK;
}

int LoadATT7022Gain(void)
{
    p7022gain = malloc(sizeof(V7022Gain));

	if (p7022gain == NULL)
		return -1;

	memset((BYTE*)p7022gain, 0, sizeof(V7022Gain));
		
    if (ATT7022GainEEGet(p7022gain) != OK)
    {
        if (ATT7022GainFileRead(p7022gain, sizeof(V7022Gain)) != OK)
        {
            SetDefault7022Gain(p7022gain);
			WriteWarnEvent(NULL, SYS_ERR_AD, 0, "ATT7022E Gain Error!");
        }
		return 0;
    }

	gain7022Valid = 1;

	return 0;
}

void ReadHarmInstWaveFun7022E(void)
{
    int  i , j ,  WaveAddr,Cnt;
	DWORD data;
   	if(Att7022eInit != 1)
		return ; 
	WriteReg7022E(0xC9 , 0x5A);
    WriteReg7022E(0xC5 , 0x00);
	if((UAtt7022e[0] < 500) && (UAtt7022e[1] < 500) && (UAtt7022e[2] < 500))
    {
    	WriteReg7022E(0xC4 , 50*(1<<13));
     	WriteReg7022E(0xC5 , 0x03);
    }
    else
    {
		WriteReg7022E(0xC5 , 0x02);
    }
	WriteReg7022E(0xC1 , 0x00);
	WriteReg7022E(0xC9 , 0x01);
	thSleep(5); //��������δ�ᵽ��ʱ
    WaveAddr = 0;
    for(i = 0; i < MaxSampPointNum*3/4; i++)
    {
        for( j = 0; j < 7; j++)
        {
            for( Cnt = 0 ; Cnt < 6 ; Cnt++)
            {
                data = 0;
                if(ReadReg7022E(0x7F , (BYTE*)&data) == OK)
				{
				    AD_RESULT[j][i] = (short)(data&0xFFFF);
					WaveAddr++;
					//shellprintf("%d  ",AD_RESULT[j][i]);
                    WriteReg7022E(0xC1 , (DWORD)WaveAddr);
					break;
				}
                WriteReg7022E(0xC1 , (DWORD)WaveAddr);
            }
     
        }
		//shellprintf("\n");
    }
		
	WriteReg7022E(0xC9 , 0x5A);
	WriteReg7022E(0xC5 , 0x00);
    WriteReg7022E(0xC5 , 0x01);

    for(i = 0 ; i < 3 ; i++)
    {
        ReadReg7022E((WORD)(0x48 + i) , (BYTE*)&InstantSampData7022E[DATAUCurBaseVal+i]);
        ReadReg7022E((WORD)(0x4B + i) , (BYTE*)&InstantSampData7022E[DATAICurBaseVal+i]);
    }
}

//�����޵��ֵ
long Att7022GetEP(int chn)
{	
	int i;
	DWORD val = 0;
	long powdir;
	float ddvalue;
	long dudir;

	i = chn;
	
	dudir = InstantSampData7022E[DATASfalg];
	powdir = InstantSampData7022E[DATAPFlag];
	switch (i)
	{
		case 0:  //�����й�
				if((dudir & EMU_PDIR_MASK) != EMU_PDIR_MASK )
				{	
					val = (DWORD)InstantSampData7022E[DATAEpa+3];
				}
				break;
		case 1: 	//�����޹�
				if((dudir & EMU_QDIR_MASK) != EMU_QDIR_MASK )
				{	
					val = (DWORD)InstantSampData7022E[DATAEqa+3];
				}
				break;	
		case 2: //һ����
				if(((powdir & PSIGN_MASK) != PSIGN_MASK ) && ((powdir & QSIGN_MASK) != QSIGN_MASK))
				{
					val = (DWORD)InstantSampData7022E[DATAEqa+3];
				}
				break;
		case 3: // ������
				if(((powdir & PSIGN_MASK) != PSIGN_MASK ) && ((powdir & QSIGN_MASK) == QSIGN_MASK))
				{	
					val = (DWORD)InstantSampData7022E[DATAEqa+3];
				}
				break;	
		case 4://�����й����
				if((dudir & EMU_PDIR_MASK) == EMU_PDIR_MASK )
				{	
					val = (DWORD)InstantSampData7022E[DATAEpa+3];
				}
				break;
		case 5:	//�����޹�
				if((dudir & EMU_QDIR_MASK) == EMU_QDIR_MASK )
				{	
					val = (DWORD)InstantSampData7022E[DATAEqa+3] ;
				}
				break;
		case 6:	//������
				if(((powdir & PSIGN_MASK) == PSIGN_MASK) &&((powdir & QSIGN_MASK) != QSIGN_MASK))
				{
					val = (DWORD)InstantSampData7022E[DATAEqa+3];
	      			}
			   	break;
		case 7:	//������
				if(((powdir & PSIGN_MASK) == PSIGN_MASK ) && ((powdir & QSIGN_MASK) == QSIGN_MASK))
				{
					val = (DWORD)InstantSampData7022E[DATAEqa+3];
				}
				break;
		default:	break;	
	}
	if(val > 0)
		bWriteeeprom = 1;
	
	
	val += g_Att7022_Dd[chn].AT7022_DD_decimal;
	if(val >= ACPulseConst)
		g_Att7022_Dd[chn].AT7022_DD_int++;

	g_Att7022_Dd[chn].AT7022_DD_decimal = val%ACPulseConst;

	ddvalue = g_Att7022_Dd[chn].AT7022_DD_int + (float)g_Att7022_Dd[chn].AT7022_DD_decimal/ACPulseConst;			
	memcpy(&val,&ddvalue,4);
	
	return  (long)val;	
}

//��⳱������
void ActPowerFlowFaultEvent(BYTE bType)
{
	int  vlt[3] = {0};						//-��ǰ�����ѹ-
	int  cur[3] = {0};						//-��ǰ�������-
	int  actPower[3] = {0};					//-��ǰ�����й�	
	int  actPowerFlowAll;
	WORD  lastTimesLmt = 60;   			    //����ʱ����ֵ,60s
	static DWORD lastTimes[2] = {0};	//�¼�����/�ָ�����ʱ��
	BYTE	tmp_flag = 0;   /* �����������ָ����㹦�ʷ������� */
	BYTE	is_now_flag = 0; 
	int i;
	struct  VDDF dbExtDdValue;
	struct VDDFT Ddbuf;
	static struct VCalClock bFlowRevFrzCP56Time;
	struct VSysFlowEventInfo Flowinfo;

	//-��ȡʵʱ��ѹ������-
    vlt[0] = UAtt7022e[0];
    vlt[1] = UAtt7022e[1];
    vlt[2] = UAtt7022e[2];
    cur[0] = IAtt7022e[0];
    cur[1] = IAtt7022e[1];
    cur[2] = IAtt7022e[2];
    actPower[0] = PQAtt7022e[0];
    actPower[1] = PQAtt7022e[1];
    actPower[2] = PQAtt7022e[2];
	
	if((cur[0] < 100)&&(vlt[0] < 100)&&(cur[1] < 100)&&(vlt[1] < 50)&&(cur[2] < 50)&&(vlt[2] < 50)) //1V 0.05A
	{	
		return;//-��ѹ����ͣ��-
	}

	actPowerFlowAll = actPower[0] + actPower[1] + actPower[2];

	if(bType == 1)		//��������
	{
		if(actPowerFlowAll < 0)
		{
			tmp_flag = labs(actPowerFlowAll) > POWER_REV;
		}
	}
	else if(bType == 2)	//��������ָ�
	{
		if(actPowerFlowAll > 0)
			tmp_flag = actPowerFlowAll > POWER_REV;
	}
	else
	{
		return;
	}
	
	if(tmp_flag)		//���������Ӧ�ķ�������������Ӧ��ʱ������ۼ�
	{	
		if(lastTimes[bType-1] >= (DWORD)(lastTimesLmt-1))
		{
			is_now_flag = 1;
		}
		else
		{
			lastTimes[bType-1]++;
			is_now_flag = 0;
		}
		if((bType == 1) && (bActPowerDirHis == 0))
		{
			GetSysClock(&bFlowRevFrzCP56Time, CALCLOCK);
			bActPowerDirHis = 1;
		}
		else if((bType == 2) && (bActPowerDirHis == 1))
		{
			 GetSysClock(&bFlowRevFrzCP56Time, CALCLOCK);
			bActPowerDirHis = 0;
		}	
	}
	else
	{
		lastTimes[bType-1] = 0;
		is_now_flag = 0;
	}

	if(is_now_flag)
	{
		if((1 == bType) && (bFlowDirHis == 0))
		{
			bFlowDirHis = 1;
			memset(&Flowinfo , 0 , sizeof(struct VSysFlowEventInfo));
			for(i=0;i<8; i++)
			{
				ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i , 1 ,  sizeof(dbExtDdValue),&dbExtDdValue);
				Ddbuf.byFlag = dbExtDdValue.byFlag;
				Ddbuf.lValue= dbExtDdValue.lValue;
				Ddbuf.Time = bFlowRevFrzCP56Time;
				WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+3*8, 1, &Ddbuf);
				
				Flowinfo.dbflow[Flowinfo.num].Time = bFlowRevFrzCP56Time;
				Flowinfo.dbflow[Flowinfo.num].val = dbExtDdValue.lValue;
				Flowinfo.dbflow[Flowinfo.num++].wNo = (WORD)i + 3*8;
			}
			//if(Flowinfo.num)   //��ʱ�����γ����ļ�
			//	 WriteFlowEvent(NULL, 0, 0, &Flowinfo);
			//g_ddtideflag |= 0x01;
		}
		else if((2 == bType) && (bFlowDirHis == 1))
		{
			bFlowDirHis = 0;
			memset(&Flowinfo , 0 , sizeof(struct VSysFlowEventInfo));
			for(i=0;i<8; i++)
			{
				ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i , 1 , sizeof(dbExtDdValue), &dbExtDdValue);
				Ddbuf.byFlag = dbExtDdValue.byFlag;
				Ddbuf.lValue= dbExtDdValue.lValue;
				Ddbuf.Time = bFlowRevFrzCP56Time;
				WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+3*8, 1, &Ddbuf);

				Flowinfo.dbflow[Flowinfo.num].Time = bFlowRevFrzCP56Time;
				Flowinfo.dbflow[Flowinfo.num].val = dbExtDdValue.lValue;
				Flowinfo.dbflow[Flowinfo.num++].wNo = (WORD)i + 3*8;
			}
			//if(Flowinfo.num)   //��ʱ�����γ����ļ�
			//	 WriteFlowEvent(NULL, 0, 0, &Flowinfo);
			//g_ddtideflag |= 0x01;
		}
	}
}

void att7022ddfrzfif( struct VSysClock *pclk)
{
	int i;
	struct  VDDF dbExtDdValue;
	struct VDDFT Ddbuf;
	struct VCalClock bFrzfifCP56Time;
	
	ToCalClock(pclk,&bFrzfifCP56Time);
	
	for(i=0;i<8; i++)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i , 1 , sizeof(dbExtDdValue), &dbExtDdValue);
		Ddbuf.byFlag = dbExtDdValue.byFlag;
		Ddbuf.lValue= dbExtDdValue.lValue;
		Ddbuf.Time = bFrzfifCP56Time;
		WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+8, 1, &Ddbuf);	
	}
	//g_ddfifflag |= 0x01;
}

void att7022ddfrzday( struct VSysClock *pclk)
{
	int i;
	struct  VDDF dbExtDdValue;
	struct VDDFT Ddbuf;
	struct VCalClock bFrzddCP56Time;
	ToCalClock(pclk,&bFrzddCP56Time);

	for(i=0;i<8; i++)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i , 1 , sizeof(dbExtDdValue),&dbExtDdValue);
		Ddbuf.byFlag = dbExtDdValue.byFlag;
		Ddbuf.lValue= dbExtDdValue.lValue;
		Ddbuf.Time = bFrzddCP56Time;
		WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+8*2, 1, &Ddbuf);	
	}
	//g_dddayflag |= 0x01;
}
void att7022ddAllday( struct VSysClock *pclk)
{
	struct VDDF dbExtDdValue[2];
	struct VDDFT Ddbuf;
	struct VCalClock bFrzddCP56Time;
	ToCalClock(pclk,&bFrzddCP56Time);

	ReadRangeDDF(g_Sys.wIOEqpID ,  0 , 1 , sizeof(struct VDDF),&dbExtDdValue[0]);
	ReadRangeDDF(g_Sys.wIOEqpID ,  4 , 1 , sizeof(struct VDDF),&dbExtDdValue[1]);
	Ddbuf.byFlag = dbExtDdValue[0].byFlag;
	Ddbuf.lValue= dbExtDdValue[0].lValue + dbExtDdValue[1].lValue;
	Ddbuf.Time = bFrzddCP56Time;
	WriteRangeDDFT(g_Sys.wIOEqpID, 32, 1, &Ddbuf);

	ReadRangeDDF(g_Sys.wIOEqpID ,  1 , 1 , sizeof(struct VDDF),&dbExtDdValue[0]);
	ReadRangeDDF(g_Sys.wIOEqpID ,  5 , 1 , sizeof(struct VDDF),&dbExtDdValue[1]);
	Ddbuf.byFlag = dbExtDdValue[0].byFlag;
	Ddbuf.lValue= dbExtDdValue[0].lValue + dbExtDdValue[1].lValue;
	Ddbuf.Time = bFrzddCP56Time;
	WriteRangeDDFT(g_Sys.wIOEqpID, 33, 1, &Ddbuf);

}
void att7022ddClearJFPG( struct VSysClock *pclk)
{
	int i;
	struct VDDF dbExtDdValue;
	struct VDDFT Ddbuf;
	struct VCalClock bFrzddCP56Time;
	
	memset(g_Att7022_Dd_JFPG, 0 , sizeof(g_Att7022_Dd_JFPG));
	extNvRamSet(NVRAM_AD_DD_JFPG, (BYTE*)g_Att7022_Dd_JFPG, sizeof(g_Att7022_Dd_JFPG));
	ToCalClock(pclk,&bFrzddCP56Time);
	for(i=0;i<8; i++)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i+34 , 1 , sizeof(dbExtDdValue),&dbExtDdValue);
		Ddbuf.byFlag = dbExtDdValue.byFlag;
		Ddbuf.lValue= 0;
		Ddbuf.Time = bFrzddCP56Time;
		WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+34, 1, &Ddbuf);	
	}
}
void att7022ddCalJFPG( struct VSysClock *pclk)
{
	int i,j;
	float dd[2] = {0};
	struct VDDF JFPGDdValue[8];
	struct VDDFT Ddbuf;
	struct VCalClock bFrzddCP56Time;
	ToCalClock(pclk,&bFrzddCP56Time);
	
	
	j = (pclk->byHour*60+pclk->byMinute)/30;//��ǰʱ��
	if(extEnergyT[j]==0)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  34 , 1 , sizeof(struct VDDF),&JFPGDdValue[0]);
		ReadRangeDDF(g_Sys.wIOEqpID ,  35 , 1 , sizeof(struct VDDF),&JFPGDdValue[1]);
		memcpy((void*)&dd[0],(void*)&JFPGDdValue[0].lValue,4);
		memcpy((void*)&dd[1],(void*)&JFPGDdValue[1].lValue,4);
		
		dd[0] +=(float)(mP)/(1000*60*10);
		dd[1] +=(float)(mQ)/(1000*60*10);
		
		for(i = 0;i<2;i++)
		{	
			g_Att7022_Dd_JFPG[i].DD_JFPG = dd[i];
			Ddbuf.byFlag = JFPGDdValue[i].byFlag;
			memcpy((void*)&Ddbuf.lValue,(void*)&dd[i],4);
			Ddbuf.Time = bFrzddCP56Time;
			WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+34, 1, &Ddbuf);
		}
		
	}
	else if(extEnergyT[j]==1)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  36 , 1 , sizeof(struct VDDF),&JFPGDdValue[2]);
		ReadRangeDDF(g_Sys.wIOEqpID ,  37 , 1 , sizeof(struct VDDF),&JFPGDdValue[3]);
		memcpy((void*)&dd[0],(void*)&JFPGDdValue[2].lValue,4);
		memcpy((void*)&dd[1],(void*)&JFPGDdValue[3].lValue,4);
		dd[0] += (float)(mP)/(1000*60*10);
		dd[1] += (float)(mQ)/(1000*60*10);
		for(i = 0;i<2;i++)
		{	
			g_Att7022_Dd_JFPG[i+2].DD_JFPG = dd[i];
			Ddbuf.byFlag = JFPGDdValue[i+2].byFlag;
			memcpy((void*)&Ddbuf.lValue,(void*)&dd[i],4);
			Ddbuf.Time = bFrzddCP56Time;
			WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+36, 1, &Ddbuf);
		}
	}
	else if(extEnergyT[j]==2)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  38 , 1 , sizeof(struct VDDF),&JFPGDdValue[4]);
		ReadRangeDDF(g_Sys.wIOEqpID ,  39 , 1 , sizeof(struct VDDF),&JFPGDdValue[5]);
		memcpy((void*)&dd[0],(void*)&JFPGDdValue[4].lValue,4);
		memcpy((void*)&dd[1],(void*)&JFPGDdValue[5].lValue,4);
		dd[0] += (float)(mP)/(1000*60*10);
		dd[1] += (float)(mQ)/(1000*60*10);
		for(i = 0;i<2;i++)
		{	
			g_Att7022_Dd_JFPG[i+4].DD_JFPG = dd[i];
			Ddbuf.byFlag = JFPGDdValue[i+4].byFlag;
			memcpy((void*)&Ddbuf.lValue,(void*)&dd[i],4);
			Ddbuf.Time = bFrzddCP56Time;
			WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+38, 1, &Ddbuf);
		}
	}
	else if(extEnergyT[j]==3)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  40 , 1 , sizeof(struct VDDF),&JFPGDdValue[6]);
		ReadRangeDDF(g_Sys.wIOEqpID ,  41 , 1 , sizeof(struct VDDF),&JFPGDdValue[7]);
		memcpy((void*)&dd[0],(void*)&JFPGDdValue[6].lValue,4);
		memcpy((void*)&dd[1],(void*)&JFPGDdValue[7].lValue,4);
		dd[0] += (float)(mP)/(1000*60*10);
		dd[1] += (float)(mQ)/(1000*60*10);
		for(i = 0;i<2;i++)
		{	
			g_Att7022_Dd_JFPG[i+6].DD_JFPG = dd[i];
			Ddbuf.byFlag = JFPGDdValue[i+6].byFlag;
			memcpy((void*)&Ddbuf.lValue,(void*)&dd[i],4);
			Ddbuf.Time = bFrzddCP56Time;
			WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+40, 1, &Ddbuf);
		}
	}

	for(i =0;i <8;i++)
	{
		g_Att7022_Dd_JFPG[i].DD_Time.wYear = pclk->wYear;
		g_Att7022_Dd_JFPG[i].DD_Time.byMonth = pclk->byMonth;
		g_Att7022_Dd_JFPG[i].DD_Time.byDay = pclk->byDay;
		g_Att7022_Dd_JFPG[i].DD_Time.byWeek = pclk->byWeek;
		g_Att7022_Dd_JFPG[i].DD_Time.byHour = pclk->byHour;
		g_Att7022_Dd_JFPG[i].DD_Time.byMinute = pclk->byMinute;
		g_Att7022_Dd_JFPG[i].DD_Time.bySecond = pclk->bySecond;
		g_Att7022_Dd_JFPG[i].DD_Time.wMSecond = pclk->wMSecond;	
	}

	extNvRamSet(NVRAM_AD_DD_JFPG, (BYTE*)g_Att7022_Dd_JFPG, sizeof(g_Att7022_Dd_JFPG));

}

void Att7022e_Eqp(void)
{
	int i;
	struct  VDDF dbExtDdValue;
	
	DWORD val;
	struct VSysClock curclock;
	static struct VSysClock oldclock = {0};
	
	GetSysClock(&curclock, SYSCLOCK);
	if((oldclock.wYear == 0) && (oldclock.byMonth == 0))
		memcpy(&oldclock, &curclock, sizeof(struct VSysClock));
		
	for( i = 0 ; i < 8 ; i++)
	{
		val = (DWORD)Att7022GetEP(i);
		memcpy((BYTE*)&dbExtDdValue.lValue,(BYTE*)&val,4);
		dbExtDdValue.byFlag = 0x01;	  
		WriteRangeDDF(g_Sys.wIOEqpID ,  (WORD)i , 1 ,  &dbExtDdValue);
	}
	
	if(curclock.bySecond != oldclock.bySecond)
	{
		ActPowerFlowFaultEvent(1);
		ActPowerFlowFaultEvent(2);
	}
	if(curclock.byMinute != oldclock.byMinute)
	{
		if(bWriteeeprom)
		{
			extNvRamSet(NVRAM_AD_DD+sizeof(DWORD), (BYTE*)g_Att7022_Dd, sizeof(g_Att7022_Dd));
			bWriteeeprom = 0;
		}
		if(curclock.byMinute%15 == 0)
		{
			att7022ddfrzfif(&curclock);
		}
		att7022ddCalJFPG(&curclock);
	}
	if(curclock.byDay != oldclock.byDay)
	{
		att7022ddfrzday(&curclock);
		att7022ddAllday(&curclock);//�����й��޹�
	}
	memcpy(&oldclock, &curclock, sizeof(struct VSysClock));
}

void Att7022eDd_Clear(void)
{
	DWORD type;
	struct VSysClock curclock;
	memset(g_Att7022_Dd , 0 , sizeof(g_Att7022_Dd));
	GetSysClock(&curclock, SYSCLOCK);
	extNvRamGet(NVRAM_AD_DD, (BYTE *)&type, sizeof(DWORD));	
	if (type != g_Sys.dwAioType)
	{
		type = g_Sys.dwAioType;
		extNvRamSet(NVRAM_AD_DD+sizeof(DWORD), (BYTE*)g_Att7022_Dd, sizeof(g_Att7022_Dd));
		extNvRamSet(NVRAM_AD_DD, (BYTE *)&type, sizeof(DWORD));
	}
	else
	{
		extNvRamSet(NVRAM_AD_DD+sizeof(DWORD), (BYTE*)g_Att7022_Dd, sizeof(g_Att7022_Dd));
	}
	att7022ddClearJFPG(&curclock);
}

void Att7022eDd_Init(void)
{
	DWORD type;
	int i;
	struct VDDF JFPGDdValue[8];
	struct VDDFT Ddbuf;
	struct VCalClock bFrzddCP56Time;
	struct VSysClock curclock;	
	memset(g_Att7022_Dd , 0 , sizeof(g_Att7022_Dd));
	memset(g_Att7022_Dd_JFPG, 0 , sizeof(g_Att7022_Dd_JFPG));
	GetSysClock(&curclock, SYSCLOCK);
	ToCalClock(&curclock,&bFrzddCP56Time);
	extNvRamGet(NVRAM_AD_DD, (BYTE *)&type, sizeof(DWORD));	
	if (type != g_Sys.dwAioType)
	{
		type = g_Sys.dwAioType;
		extNvRamSet(NVRAM_AD_DD+sizeof(DWORD), (BYTE*)g_Att7022_Dd, sizeof(g_Att7022_Dd));
		extNvRamSet(NVRAM_AD_DD, (BYTE *)&type, sizeof(DWORD));
	}
	else
	{
		extNvRamGet(NVRAM_AD_DD+sizeof(DWORD),(BYTE*) g_Att7022_Dd, sizeof(g_Att7022_Dd));
	}		
	extNvRamGet(NVRAM_AD_DD_JFPG,(BYTE*) g_Att7022_Dd_JFPG, sizeof(g_Att7022_Dd_JFPG));	
	for(i=0;i<8;i++)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i+34 , 1 , sizeof(struct VDDF),&JFPGDdValue[i]);
		
		Ddbuf.byFlag = JFPGDdValue[i].byFlag;
		memcpy((void*)&Ddbuf.lValue,(void*)&g_Att7022_Dd_JFPG[i].DD_JFPG,4);
		Ddbuf.Time = bFrzddCP56Time;
		WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+34, 1, &Ddbuf);
	}
	
}


/*******************************************************************************
* ��������: InitCurrTempPara
* ��������: �жϳ�ʼ״̬�µ��¶��������������Ȳ���
* �������: ��
* �������: ��
* �� �� ֵ: 0 �ɹ�; < 0 ʧ��
*******************************************************************************/
int InitCurrTempPara(void)
{
	long IVal[3];
	int i;
   
	ReadACInstDataFun7022E(ATT7022_REG_UI);

    for (i=0; i<3; i++)
	{
	     Real_I(i, IVal+i);
		 JudgeIZone((short)IVal[i], RATED_CURR_VALUE*1000, i);   //In�ı�����Ĭ��1.5A�� hll���޸�
         CurrentInfo7022E.IBak[i] = (short)IVal[i];
    }
	
    ReadTemp();

    CheckTempZone();

 
    TempInfo7022E.TempZoneBak = (BYTE)TempInfo7022E.TempZone;
    TempInfo7022E.TempBak = TempInfo7022E.TempCur;   
	TempInfo7022E.ITempZoneBak = TempInfo7022E.ITempZone;

    for (i=0; i<3; i++)
       CurrentInfo7022E.IZoneBak[i] = CurrentInfo7022E.IZone[i];
    
    return 0;
}


/*******************************************************************************
* ��������: Init7022E
* ��������: ����оƬ��ʼ��
* �������: ��
* �������: ��
* �� �� ֵ: > 0 ��ʼ����ɣ� < 0 ��ʼ��ʧ�ܻ��ϵ��״γ�ʼ�����
*******************************************************************************/
int Init7022E(void)
{  
    Att7022eDd_Init();
    if (Reset_ACChip() < 0)                 // ��λ7022E
    {
        return ERROR;
    }

	LoadATT7022Gain();
	//InitHREnv7022E();  //��ȡг��У������
	EnableTempCurrMeter();   // ʹ���¶ȡ���������
	thSleep(1);
	InitCurrTempPara();      // �жϵ�ǰ�¶��������������Ȳ���
    if (InitMeaICFun7022E() < 0)
    {
        shellprintf("7022E��ʼ��ʧ�ܣ����³�ʼ��!!!\n");
        return ERROR;
    }
	
    Att7022eInit = 1;
	return OK;
}

void Cal_U_gain(BYTE id ,int value,int TheoreticVolt, VYcGain* pycgain)
{
	int ActualVolt;    // ʵ�ʵ�ѹ
	INT64U MidNum = 0;
	short gain = 0;
         
	if((id < 1) || (id >3))
		return;
	
	ActualVolt =  value;
	
	if(ActualVolt != 0)
	{
		if(ActualVolt <= TheoreticVolt)
		{
			//������
			if(TheoreticVolt > 1.9*ActualVolt)
				return;
			MidNum = (DWORD)(TheoreticVolt - ActualVolt);
            MidNum = MidNum << 15;
            ActualVolt  = (int)(MidNum /(DWORD)ActualVolt);		
			gain = (short)ActualVolt;
			if(gain < 0)
				return;
		}
		else
		{// ������
			MidNum = (DWORD)(ActualVolt  - TheoreticVolt);
            MidNum <<= 15;
            ActualVolt  = (int)(MidNum / (DWORD)ActualVolt);
            ActualVolt  = (int)(~(unsigned int)ActualVolt);
            ActualVolt  += 1;
			gain = (short)ActualVolt;
		}
	}
	pycgain->a = pycgain->a +  gain + ((short)pycgain->a*gain)/(1<<15);
	if (WriteContFun7022E(0x00) < 0)   // ��дʹ��
	{
		return ;
	}
	thSleep(1);

	pycgain->a  =  pycgain->a & 0xffff;
	WriteReg7022E(CalMetConfigTab[0].Addr +id -1, (DWORD)pycgain->a);

	WriteContFun7022E(0x01);           // �ر�дʹ��
	
}

void Cal_I_gain(BYTE id ,int value,int TheoreticCurr, VYcGain* pycgain)
{
	int ActualCurr;  //ʵ�ʵ���
	short gain = 0;
	INT64U MidNum;
	
	if((id < 1) || (id >4))
		return;
    ActualCurr = value;
		
	if(ActualCurr != 0)
	{
		if(ActualCurr <= TheoreticCurr)
        {    // ������
			if(TheoreticCurr > 1.9*ActualCurr)
				return;
            MidNum = (DWORD)(TheoreticCurr - ActualCurr);
            MidNum = MidNum << 15;
            ActualCurr = (int)(MidNum / (DWORD)ActualCurr);
			gain = (short)ActualCurr;
		  	if(gain <  0)
				return;
        }
        else
        {    // ������
            MidNum = (DWORD)(ActualCurr - TheoreticCurr);
            MidNum <<= 15;
            ActualCurr = (int)(MidNum / (DWORD)ActualCurr);
            ActualCurr = (int)(~(unsigned int)ActualCurr);
            ActualCurr += 1;
            gain = (short)ActualCurr;
        }
	}
	
	pycgain->a  =  pycgain->a + gain + ((short)pycgain->a*gain)/(1<<15);
	if (WriteContFun7022E(0x00) < 0)   // ��дʹ��
	{
		return;
	}
	thSleep(1);

	pycgain->a  =  pycgain->a & 0xffff;
	if (id == 4)
		WriteReg7022E(CalMetConfigTab[6].Addr, (DWORD)pycgain->a);
	else
		WriteReg7022E(CalMetConfigTab[3].Addr + id - 1, (DWORD)pycgain->a);
	WriteContFun7022E(0x01);           // �ر�дʹ��
	
}

void Cal_PQ_gain(BYTE id, int gain1, int gain2,VYcGain* pycgain)
{
	float Pgain;
	int temp;
	short GP1 = 0;
	
	if((id < 1) || (id >3))
		return;
    
       //������˵
  	temp = (gain2 -gain1)*1000;
	Pgain = (temp/(float)gain1)/1000;
	
	if(Pgain >= 0)
	{
		if(gain2 > 1.9*gain1)
			return;
		GP1 = (short)(Pgain * (1 << 15));
		if(GP1 < 0)
			return;
	}
	else
		GP1 = (short)((1<< 16) + Pgain * (1 << 15));
		
	if (WriteContFun7022E(0x00) < 0)   // ��дʹ��
    {
            return;
    }
	thSleep(1);
	pycgain->a += GP1 +  ((short)pycgain->a*GP1)/(1<<15);
	pycgain->a = pycgain->a & 0xffff;
	WriteReg7022E(CalMetConfigTab[7].Addr+ id -1 , (DWORD)pycgain->a); 
	WriteReg7022E(CalMetConfigTab[10].Addr + id -1 , (DWORD)pycgain->a); 
	
	WriteContFun7022E(0x01);           // �ر�дʹ��
}

//��λУ��
int Cal_angle_gain(BYTE id, int value, int pTheoretic)
{
	int Actual; //ʵ��ֵ; COS = 1,�Ƕ�60 �� �Ŵ�100��
	INT64U MidNum;
	short GP1 = 0,gain;
	WORD addr;
	DWORD val;
	BYTE Zone;

	if ((id < 1) || (id >3))
		return -1;
	if(Att7022eInit != 1)
			return -1;
	if ((TempInfo7022E.TempCur < -50)||(TempInfo7022E.TempCur > 80))
		return -1;
	
	Actual = value;

	if((Actual !=  0) && (pTheoretic != 0))
	{
		if(Actual < pTheoretic)
		{
			//������
			MidNum = (DWORD)(pTheoretic  - Actual);
			MidNum *= 100000;
			MidNum = MidNum/(DWORD)pTheoretic;

			MidNum = MidNum * (1 << 15);

			GP1 = (short)(MidNum/(100000 * 1.732));
		}
		else
		{
			MidNum =  (DWORD)(Actual - pTheoretic);
			MidNum *= 100000;
			MidNum = MidNum/(DWORD)pTheoretic;
			MidNum = MidNum * (1 << 15);

			GP1 = (short)((1<<16) -MidNum/(100000*1.732));
		}
	}

	Zone = (BYTE)(CurrentInfo7022E.IZone[id-1] + TempInfo7022E.ITempZone*I_ZONE_NUM);
	gain = p7022gain->allIAng[id-1][Zone];
	p7022gain->allIAng[id-1][Zone] = gain + GP1 ;
	gain = p7022gain->allIAng[id-1][Zone];
	if (id == 3)
       ATT7022GainEEWrite(p7022gain);
	
	if (WriteContFun7022E(0x00) < 0)   // ��дʹ��
    {
        return -1;
    }
	thSleep(1);
	val =(int)gain&0xFFFF;
	addr = (CurrentInfo7022E.IZone[id-1] >= 2) ? 0x0D:0x10;
	WriteReg7022E(addr + id -1 , val);
	WriteContFun7022E(0x01);           // �ر�дʹ��

	return 0;
}

void Cal_T_gain(WORD T)
{
	if(Att7022eInit != 1)
			return;
    p7022gain->TC = (short)(p7022gain->TC + T - TempInfo7022E.TempCur);

    ReadTemp();
	CheckTempZone();
}

int Cal_all_gain(int value, int pTheoretic, int T)
{
	INT64U MidNum;
	int i,Actual;
	BYTE zone;
	short allgain = 0,temp; 
	V7022Gain *gain;
	DWORD val;
	float a;
	float b;
	float c = -12.4202;
	float T1, Y1;
	float T2  = 25.0, Y2 = 0.0;
	float det; // ����ʽ
	
 	if ((T < -60)||(T > 100))
    {
        return -1;
	}

	//Cal_T_gain(T);
	
	if(Att7022eInit != 1)
		return -1;
	
	Actual = value;

	if(Actual !=  0)
	{
		if(Actual < pTheoretic)
		{
			//������
			MidNum = (DWORD)(pTheoretic  - Actual);
			MidNum *= 100000;
			Actual = (int)(MidNum/(DWORD)pTheoretic);
			MidNum = (DWORD)Actual;
			MidNum = MidNum * (1 << 15);
            allgain = (short)((MidNum >> 1) / (100000 - (DWORD)Actual));
		}
		else
		{
			MidNum =  (DWORD)(Actual - pTheoretic);
			MidNum *= 100000;
			Actual = (int)MidNum/pTheoretic;
			MidNum = (DWORD)Actual;
			MidNum = MidNum * (1 << 15);
			allgain = (short)((MidNum >> 1) / (DWORD)(100000 + Actual));
			allgain = (short)(1<<16) - allgain;
		}
	}
    gain = p7022gain;
	zone = (BYTE)TempInfo7022E.TempZone;
  
    allgain += gain->allGain[zone];
    T1 = TEMP_LOW_CMP + zone*TEMP_STEP_CMP;
    Y1 = allgain;
    det= T1*T1*T2 - T2*T2*T1;
    a  = (1 / det) * (T2*(Y1-c) - T1*(Y2-c));
    b  = (1 / det) * (-T2*T2*(Y1-c) + T1*T1*(Y2-c)); 

    if (TempInfo7022E.TempCur < 0)
    {
        for (i = 0, temp = TEMP_LOW_CMP; i < TEMP_ZONE_NOR_LOW; i++, temp += 5)
	    {
	        gain->allGain[i] = (short)(a*temp*temp + b*temp + c);
	        if (abs(gain->allGain[i]) > ALLGAIN_LOW_TEMP_MAX)
	            gain->allGain[i] = -ALLGAIN_LOW_TEMP_MAX;
	    }
	    for (i = TEMP_ZONE_NOR_LOW; i < TEMP_ZONE_LOW_NUM; i++)
	    {
	        gain->allGain[i] = 0; // 0~25�Ȳ�����ȫͨ������
	    }
		if (abs(allgain) > ALLGAIN_LOW_TEMP_MAX)
        {
            allgain = -ALLGAIN_LOW_TEMP_MAX;
        }
		ATT7022GainEEWrite(gain);
    }
	else
	{
	    
	    for (i = TEMP_ZONE_LOW_NUM; i < TEMP_ZONE_NOR_HIGH; i++)
	    {
	        gain->allGain[i] = 0; // 30~50�Ȳ�����ȫͨ������
	    }
	    for (i = TEMP_ZONE_NOR_HIGH, temp = TEMP_NORMAL_HIGH; i < TEMP_ZONE_MAX; i++, temp += 5)
	    {
	        gain->allGain[i] = (short)(a*temp*temp + b*temp + c);
	        if (abs(gain->allGain[i]) > ALLGAIN_HIGH_TEMP_MAX)
	            gain->allGain[i] = -ALLGAIN_HIGH_TEMP_MAX;
	    }
	    
		if (abs(allgain) > ALLGAIN_HIGH_TEMP_MAX)
        {
            allgain = -ALLGAIN_HIGH_TEMP_MAX;
        }
		ATT7022GainEEWrite(gain);
	}

	if (WriteContFun7022E(0x00) < 0)   // ��дʹ��
    {
        return -1;
    }
	thSleep(1);
	val  = (int)allgain & 0x0000FFFF;
	WriteReg7022E(0x32 , val);
	WriteContFun7022E(0x01);           // �ر�дʹ��
   
		return 0;
}

int att_gainread(void)
{
    return(ATT7022GainRead());
}

int att_gainclear(void)
{
    return(ATT7022GainClear());
}

#if 0
int  Angle_UI(int i )  // 0 IaUa 1IbUb 2IcUc 
{// �Ŵ�100��
	int pSrc;
	int pDest;
	INT64S MidNum = 0;
  
        //��д2018��2��9�� 09:04:07
    pSrc =  InstantSampData7022E[DATAUIAngle + i];

    if(pSrc > (1 << 20))
    {
        pSrc = pSrc & ((1 << 24) - 1);
        pSrc = pSrc - (1 << 24);
    }
    MidNum = pSrc;
    MidNum = (MidNum * 100 * 180) ;
    pDest = MidNum/(1 << 20);

    return pDest;
}

int  Angle_UU(int i )  // 0 UaUb 1UbUc 2UbUc 
{// �Ŵ�100��
	int pSrc;
	int pDest;
	INT64S MidNum = 0;
  
        //��д2018��2��9�� 09:04:07
    pSrc =  InstantSampData7022E[DATAUaUbAngle + i];

    if(pSrc > (1 << 20))
    {
        pSrc = pSrc & ((1 << 24) - 1);
        pSrc = pSrc - (1 << 24);
    }
    MidNum = pSrc;
    MidNum = (MidNum * 100 * 180) ;
    pDest = MidNum/(1 << 20);

    return pDest;
}

void ReadAngle(int i , long* data)
{
   // 0Ua 1Ub 2Uc 3U0,4Ia 5Ib 6Ic 7I0
    switch( i )
    {
      case 0:
         *data = 0;
         break;
       case 1:
        *data = Angle_UU(0);
           break;
        case 2:
          *data = Angle_UU(0) - Angle_UU(2);

            break;
         case 3:
          *data = 0;
            break;
          case 4:
            *data = Angle_UI(0);
              break;
           case 5:
            *data = Angle_UI(1) ;
              break;
           case 6:
             *data =  Angle_UI(2);
              break;
           case 7:
              *data = 0;
                break;
    }
}

/****************************************************************************
* ��������: EE
* ��������: �������
* �������: b1  ����1
*           b2  ����2
* �������: ��
* �� �� ֵ: �˻�
****************************************************************************/
static struct compx EE(struct compx b1,struct compx b2)
{
    struct compx b3;
    
    b3.real = (b1.real * b2.real - b1.imag * b2.imag);
    b3.imag = (b1.real * b2.imag + b1.imag * b2.real);
    
    return (b3);
}

/****************************************************************************
* ��������: FFT
* ��������: FFT����
* �������: xin  ԭʼ������������
*           N    FFT����ĵ���
* �������: xin  ������
* �� �� ֵ: ��
****************************************************************************/
void FFT(struct compx *xin, int N)
{
    int i, j = 1, k = 0, l = 0, m = 0;
    int lei = 0, ip = 0;
    struct compx v, w, t; 
    
    for (m = 1, i = N; (i/=2)!= 1; m++)
        ;
    for (i = 1; i <= N-1; i ++)
    {
        if (i<j)
        {
            t = xin[j];
            xin[j] = xin[i];
            xin[i] = t;
        }
        k = N/2;
        while (k<j)
        {
            j = j-k;
            k = k/2;
        }
        j = j+k;
    }
    for (l = 1; l <= m; l++)
    {
        lei = pow2l[l-1];        
        v.real = 1.0;  
        v.imag = 0.0;
        w.real = COS_TAB[l-1];
        w.imag = SIN_TAB[l-1];
        for (j = 1; j <= lei; j ++)
        {    
            for(i = j; i <= N; i = i+2*lei)
            {    
                ip = i+lei;
                t = EE(xin[ip],v);
                xin[ip].real = xin[i].real-t.real;
                xin[ip].imag = xin[i].imag-t.imag;
                xin[i].real = xin[i].real+t.real;
                xin[i].imag = xin[i].imag+t.imag;
            }
            v = EE(v,w);
        }
    }    
}
/****************************************************************************
* ��������: CalVoltCurrBaseRMS
* ��������: �������ͨ��������Чֵ
* �������: ��
* �������: ��
* �� �� ֵ: 0 ����
****************************************************************************/
int CalVoltCurrBaseRMS(void)
{
    int i = 0;

    for( i = 0 ; i < 6; i++)
    {
       VoltCurrBaseRMS[i] = (float)InstantSampData7022E[DATAUCurBaseVal + i ]/8192;
       if(i >= 3)
       {
           VoltCurrBaseRMS[i] /= 40;  /* N = 60/Ib = 40 */
       }
    }
    return 0;
}

/****************************************************************************
* ��������: CalHRRealData7022E
* ��������: ����г��ʵʱ���ݣ�����Ƶ�ʡ�����г��������
* �������: ��
* �������: ��
* �� �� ֵ: ��
****************************************************************************/
void CalHRRealData7022E(void)
{
    long freq = 0.0;    
    long result[3] = {0};
    int i;
    ReadFreq( &freq);
    for(i = 0 ; i < 3; i++)
      Real_U( i , &result[i]);

    if(((result[0] > 3000) ||(result[1] > 3000) || (result[2] > 3000))
      && ((freq > 5300) ||(freq < 4700)))
    {

      shellprintf("����Ƶ�ʴ��� !\n");
    }
    
     // �����ͨ��������Чֵ
    CalVoltCurrBaseRMS();
    // fft���㵱ǰ˲ʱ��Чֵ�������ݵ�ǰ������fft������ĵ�ǰ��Чֵ
    // ��ͬ��ʵʱ��Чֵ����Ϊʵʱ��Чֵ����Ҫ��֮ǰ��6������ֵ���㷽�������
     mt_fft7022E();
     
}

/****************************************************************************
* ��������: mt_fft7022E
* ��������: ��ѹ������ͨ������г����������
* �������: ��
* �������: ��
* �� �� ֵ: ��
****************************************************************************/
void mt_fft7022E(void)
{  
    int i = 0, n = 0;
    struct compx xxin[SN+1];  // SN=64, 64��FFT
    float temp = 0.0;    
    float harmVoltDevid = 1.0;
    float harmCurrDevid = CURR_CAL_FACTOR; 
    float amend_offset = 0;

    
     harmVoltDevid = 4.545454 / VOLT_CAL_FACTOR_3P4L_220; // 1.118193

     // 6��ͨ�����μ��������2~19��г����Чֵ
    for (i = 0; i <= 5; i ++)
    {
       for (n = 1; n <= SN; n ++)        
        {             
            xxin[n].real = AD_RESULT[i][n+3]; // ����ǰ��4����		
            xxin[n].imag = 0.0;
        }

        FFT (xxin, SN);
        for (n = 1; n < 20; n ++)           // �������г������
        {  
            if (i < 3) // ���ε�ѹг�������ļ���
            {
                temp=sqrt((xxin[n+1].real*xxin[n+1].real+xxin[n+1].imag*xxin[n+1].imag))*1.576155207/1.4142/64.0;
            }
            else    // ���ε���г�������ļ���
            {
                temp=sqrt((xxin[n+1].real*xxin[n+1].real+xxin[n+1].imag*xxin[n+1].imag))/20.65530595/1.4142/64.0;
            }
            temp /= 20.0;

            if (n == 1)
            {
                amend_offset = VoltCurrBaseRMS[i] / temp;
            }

            if (i < 3) // ���ε�ѹг����������
            {
                component_O_7022E[i][n] = temp * amend_offset / harmVoltDevid * amend_mt[i][n];
            }
            else      // ���ε���г����������
            {
                component_O_7022E[i][n] = temp * amend_offset * harmCurrDevid * amend_mt[i][n];
            }
        }
         if ((i <= 2)&&(component_O_7022E[i][1] < 5.0))
        {
            // �����ѹг���Ļ�����ѹ��С��5V���򽫸���г����������
            for(n = 0; n <= 19; n ++)        
            {
                component_O_7022E[i][n] = 0.0;
            }
        }
        else if ((i>=3)&&(component_O_7022E[i][1] < 0.005))
        {    
            // �������г���Ļ���������С��5mA���򽫸���г����������
            for (n = 0; n <= 19; n ++)        
            {
                component_O_7022E[i][n] = 0.0;
            }
        }
    }

}


/****************************************************************************
* ��������: HarmCalibrate7022E
* ��������: г��У������У����̨��չ��07��Լ
* �������: pParamBuffer  ��̨�����У���Լ���������ݱ�ʶ�ͱ�ʶ��Ӧ������
*           len           �������ݳ���
* �������: ��
* �� �� ֵ: 0 ʧ�ܣ�1 �ɹ�
****************************************************************************/
int HarmCalibrate7022E(BYTE* pParamBuffer, BYTE len)
{
    int i = 0,j = 0;
    float amend = 0.0;
    BYTE DI0,DI1,DI2,DI3,DI4;
    WORD component[6][19];
    int    phase = 0;
    int    val = 0;
    int    ret = 0;
    
    if (len > 5)
    {
        return 0;
    }
    
    DI0 = pParamBuffer[0];
    DI1 = pParamBuffer[1];
    DI2 = pParamBuffer[2];
    DI3 = pParamBuffer[3];
    DI4 = pParamBuffer[4];
    
    if ((DI0 >> 4) == 1 || (DI0 >> 4) == 2)
    {   
        // У��ѹ�����
    }
    else
    {
        // ����Ƿ���г��������ʼ����Լ 0xEFCDAB8967
        if (memcmp(pParamBuffer, "\xEF\xCD\xAB\x89\x67", 5) == 0)
        {
            // �����ѹг�������ʼ��
            for (i = 0; i < 3; i++)
            {
                component[i][0] = 10000;    // г����ʼ��������ͨ�����������ʼ��Ϊ 1.0 * 10000
                for (j = 0; j < 18; j++)
                {
                    component[i][j+1] = amend_off_U[j] * 10000;
                }
            }

            // �������г�������ʼ��
            for (i = 0; i < 3; i++)
            {
                component[i+3][0] = 10000;  // г����ʼ��������ͨ�����������ʼ��Ϊ 1.0 * 10000
                for (j = 0; j < 18; j++)
                {
                    component[i+3][j+1] = amend_off_I[j] * 10000;
                }
            }
        }
    }

    phase = (DI0 & 0x0f)-1;  // 0~5����λ������0~2λabc��ѹ 3~5λabc����
    
    // �����׼ֵ ��ѹ:XXX.XXX  ����:XX.XXXX
    val=((DI4 >> 4) & 0xf) * 100000 + (DI4 & 0x0f) * 10000 + ((DI3 >> 4) & 0xf) * 1000 + (DI3 & 0x0f)*100
        + ((DI2 >> 4) & 0xf) * 10 + (DI2 & 0x0f);

    DI1 = (DI1>>4)*10+(DI1&0x0f);
    
    if(phase >= 0 && phase < 6
        && DI1 >= 1 && DI1 < 20)
    {
    }
    else
    {   
        if (phase == 0x0E) // г����ʼ����Լ
        {
        }
        else
        {
            // ��Լ����
            return 0;
        }
    }

    // ����У���������ѹΪ1λС��������Ϊ2λС�����ֱ�� 0.1��0.01
    // У��������: ��׼ֵ * ��ǰУ����� / ʵʱ��Чֵ == �µ�У�����
    if (phase >= 0 && phase < 3)
    {   
        amend = ((float)val)*amend_mt[phase][DI1]/component_O_7022E[phase][DI1]*0.001;
        
        WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"%c���ѹ����У�� Theoretic=%d Actual=%f amend=%f"
            , 'A' + phase, val, component_O_7022E[phase][DI1], amend);
    }
    else if (phase >= 3 && phase < 6)
    {
        amend = ((float)val)*amend_mt[phase][DI1]/component_O_7022E[phase][DI1]*0.0001;
        WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"%c���������У�� Theoretic=%d Actual=%f amend=%f"
            , 'A' + phase - 3, val, component_O_7022E[phase][DI1], amend);
    }
    
    for(i = 0; i < 6; i++)
    {
        for(j = 0; j < 19; j++)
        {
            if(i >= 0 && i < 3)
            {
                component[i][j] = amend_mt[i][j+1]*10000;
            }
            else
            {
                component[i][j] = amend_mt[i][j+1]*10000;
            }
        }
    }

    // ���ö�Ӧ����µ�У�����
    if (phase >= 0 && phase < 3)
    {
        component[phase][DI1-1] = (WORD)((DWORD)(amend*10000));
    }
    else if (phase >= 3 && phase < 6)
    {
        component[phase][DI1-1] = (WORD)((DWORD)(amend*10000));
    }
    
    if (DI1 == 1)  // ����У׼
    {   
        // ����У׼����2-19�ν��о���У׼
        if (phase < 3)    
        {
            for(i = 0; i < 18; i++)
            {
                component[phase][i+1] = component[phase][DI1-1]*amend_off_U[i];
            }
        }
        else if (phase < 6)
        {
            for(i = 0; i < 18; i++)
            {
                component[phase][i+1] = component[phase][DI1-1]*amend_off_I[i];
            }
        }
    }

    for(i = 0 ; i < 6 ; i++)
    {
      for( j = 0 ; j < 19; j++)
      {
         p7022gain->HarmGain[i][j+1] = component[i][j];
      }
    }

    ret = ATT7022GainEEWrite(p7022gain);
    if (ret < 0)
    {
        shellprintf("Cal HarmPara Error!!! ret=%d\n", ret);
        WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"Cal HarmPara Error!!");
    }

    return ret;
}


/****************************************************************************
* ��������: SetHarmPara7022E
* ��������: ����г������
* �������: pParamBuffer  ��̨�����У������
*           len           �������ݳ���
* �������: ��
* �� �� ֵ:  >= 0 �ɹ���< 0 ʧ��
****************************************************************************/
int SetHarmPara7022E(BYTE* pParamBuffer, BYTE len)
{
    BYTE  *pDat = NULL;
    int i = 0;
    int j = 0;
    int retnum = 0;
    
    pDat = pParamBuffer;
    for (i = 0; i < 6; i++, pDat += 2)
    {
        p7022gain->HarmGain[i][1] = pDat[0] + ((WORD)pDat[1] << 8);
        if (i <= 2)
        {
            for (j = 0; j < 18; j++)
            {
                p7022gain->HarmGain[i][j+2] = p7022gain->HarmGain[i][1] * amend_off_U[j];
            }
        }
        else
        {
            for (j = 0; j < 18; j++)
            {
                p7022gain->HarmGain[i][j+2] = p7022gain->HarmGain[i][1] * amend_off_I[j];
            }
        }
    }         
    retnum = ATT7022GainEEWrite(p7022gain);
    if (retnum < 0)
    {
        shellprintf("�·�г��У�����������ʧ��!!! ret=%d\n", retnum);
        WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"�·�г��У�����������ʧ��!!! ");
    }
    return retnum;
}

// г��У���������ʼ��Ϊ1
float amend_mt[6][20] =
{
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    ,{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    ,{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    ,{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    ,{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    ,{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

/***************************************************************************
* ��������: InitHREnv7022E
* ��������: ��ʼ����������ȡг��У��������ڳ�ʼ����Уг�����Ժ���øú�����ȡ��ǰУ׼����
* �������: ��
* �������: pTime  �洢��ǰʱ��ṹָ��
* �� �� ֵ: Ŀǰ������1������ܹ���EEProm(��ʾ��У��)����ʹ�ã�����ʹ��ȱʡ����(��ʱ��Чֵ�����ݽ���׼ȷ����У��)
****************************************************************************/
int InitHREnv7022E( )
{
    int   component[6][19];
    int   i = 0,j = 0;    
    BYTE2_INT16_TYPE *pHarmGain = NULL;
    float ftemp = 0.0;
    
    // ��г��У������
    for(i= 0; i < 6; i++)
    {

        pHarmGain = (BYTE2_INT16_TYPE *)&(p7022gain->HarmGain[1]);
    
        for(j=0; j<19; j++)
        {
            component[i][j] = pHarmGain->Uinteger;
            pHarmGain ++;
        }
    }
    
    for(i = 0; i < 6; i ++)
    {
        for(j = 0; j < 19; j ++)
        {
            if(i <= 2)
            {
                if((component[i][j] >= 21000) || (component[i][j] < 5000))
                {
                    shellprintf("г����ѹ���泬����!!! ��������Ϊ5000~21000\n");
                    continue;
                }
                ftemp = component[i][j];
                ftemp *= 0.0001;
                amend_mt[i][j+1] = ftemp;
            }
            else
            {
                if((component[i][j] >= 21000) || (component[i][j] < 5000))
                {
                    shellprintf("г���������泬����!!! ��������Ϊ5000~21000\n");
                    continue;
                }
                ftemp = component[i][j];
                ftemp *= 0.0001;
                amend_mt[i][j+1] = ftemp;
            }
        }
    }

    return 1;
}


#define PI 3.1415926535897932384626433832795
double Re( double m,double zeta )
{
	double result=0.0;
	double temp = 10;
	if (zeta<-180)
		zeta+=360;
	if (zeta>180)
		zeta-=360;
	result=m*cos(PI*zeta/180);
	if (abs(result)<(pow(temp,-6)))
	{
		return 0;
	}
	return result;
}
double Im( double m,double zeta )
{
	double result=0.0;
	double temp = 10;
	if (zeta<-180)
		zeta+=360;
	if (zeta>180)
		zeta-=360;
	result=m*sin(PI*zeta/180);
	if (abs(result)<(pow(temp,-6)))
	{
		return 0;
	}
	return result;
}

double CalMo( double m,double zeta )
{
	double result=0.0;
	double temp = 10;
	result=sqrt(m*m+zeta*zeta);
	if (abs(result)<(pow(temp,-6)))
	{
		return 0;
	}
	return result; 
}

/*�������: Ampa: A���ֵ��Pha:A����λ
Ampa:A���ֵ��Pha:A����λ
Ampa:A���ֵ��Pha:A����λ
����ֵ:�������
*/
double CalPosSequence( double Ampa,double Pha,double Ampb,double Phb,double Ampc,double Phc )
{
	double result=0.0;
	double result_Re=0.0;
	double result_Im=0.0;
	result_Re=Re(Ampa,Pha);
	result_Re+=Re(Ampb,Phb+120);
	result_Re+=Re(Ampc,Phc-120);
	result_Re/=3;
	result_Im=Im(Ampa,Pha);
	result_Im+=Im(Ampb,Phb+120);
	result_Im+=Im(Ampc,Phc-120);
	result_Im/=3;
	result=CalMo(result_Re,result_Im);
	return result;
}

/*�������: Ampa: A���ֵ��Pha:A����λ
Ampa:A���ֵ��Pha:A����λ
Ampa:A���ֵ��Pha:A����λ
����ֵ:�������
*/
double CalNegSequence( double Ampa,double Pha,double Ampb,double Phb,double Ampc,double Phc )
{
	double result=0.0;
	double result_Re=0.0;
	double result_Im=0.0;
	result_Re=Re(Ampa,Pha);
	result_Re+=Re(Ampb,Phb-120);
	result_Re+=Re(Ampc,Phc+120);
	result_Re/=3;
	result_Im=Im(Ampa,Pha);
	result_Im+=Im(Ampb,Phb-120);
	result_Im+=Im(Ampc,Phc+120);
	result_Im/=3;
	result=CalMo(result_Re,result_Im);
	return result;
}

/*�������: Ampa: A���ֵ��Pha:A����λ
Ampa:A���ֵ��Pha:A����λ
Ampa:A���ֵ��Pha:A����λ
����ֵ:�������
*/

double CalZeroSequence( double Ampa,double Pha,double Ampb,double Phb,double Ampc,double Phc )
{
	double result=0.0;
	double result_Re=0.0;
	double result_Im=0.0;
	result_Re=Re(Ampa,Pha);
	result_Re+=Re(Ampb,Phb);
	result_Re+=Re(Ampc,Phc);
	result_Re/=3;
	result_Im=Im(Ampa,Pha);
	result_Im+=Im(Ampb,Phb);
	result_Im+=Im(Ampc,Phc);
	result_Im/=3;
	result=CalMo(result_Re,result_Im);
	return result;
}
/*г��У׼  ??
�Ƕȼ���ֵ  ??
��г������λ����ֵ
���������������ϴ��ȵ�

��г������λ��������
��sqrt abs cos sin pow ��math.h�ĺ�����֪���Ƿ���ã�����*/
#endif
#endif
