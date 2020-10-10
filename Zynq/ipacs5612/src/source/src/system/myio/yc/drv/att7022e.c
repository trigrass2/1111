/*
****************************************************************************
* Copyright (c) 2018
* All rights reserved.
* 程序名称：att7022e.c
* 版 本 号：1.0
* 功    能：7022E采样芯片
* 原始开发人及时间：
*           cjl 20180207
* 修改人员、时间及修改内容：
* 其   他： 
****************************************************************************
*/

#include "syscfg.h"
#ifdef INCLUDE_ATT7022E
#include <math.h> 
#include "sys.h"
#include "att7022e.h"
extern VYcGain *pYcGain;

#define ACPulseConst 6400
#define TC_DEFAULT_7022E      23   //7022E 温度默认校正值TC

const struct MIX_CONFIG_CELL_TYPE *FixupConfigTab = NULL;
TEMP_INFO    TempInfo7022E;
CURRENT_INFO CurrentInfo7022E;
DWORD ITempRegVal[6];
int AllGainVal;

DWORD HFconst = 0;
V7022Gain *p7022gain;
int gain7022Valid=0;
BOOL Att7022eInit = 0;
  
 int InstantSampData7022E[100];   // 实时读取ATT7022的REGS值
 AT7022_DD g_Att7022_Dd[32];
 AT7022_DD_JFPG g_Att7022_Dd_JFPG[8];
 long UAtt7022e[4];
 long IAtt7022e[4];
 long PQAtt7022e[10];
 short  AD_RESULT[7][MaxSampPointNum];  // 6相128点AD采样值

extern BYTE extEnergyT[48];
extern short mP;
extern short mQ;

/* 潮流方向历史 */
static BYTE bFlowDirHis = 0;
static BYTE bActPowerDirHis	= 0;

static BOOL bWriteeeprom = 0;

// 固定配置数据表:其中有部分为冗余设置
const struct MIX_CONFIG_CELL_TYPE FixupConfigTab1[] =     // 三线100V
{
    {0x00C3,0x03,0x000000},        // 恢复校表数值到上电初始值

    {0x0081,0x03,0x00b9FE},        // 模式配置寄存器
    {0x0082,0x03,0x000100},        // ADC 增益配置寄存器
    {0x0096,0x03,0x000000},        // 无功相位校正
    {0x009d,0x03,0x00013A},        // 起动电流阈值设置 Ib1.5A-50mv,启动电流:0.1%(0.5级表)，以0.08%Ib设置
    {0x00b0,0x03,0x000001},        // 中断使能寄存器
    {0x00b1,0x03,0x003437},        // 电路模块配置寄存器
    {0x00b5,0x03,0x00000f},        // 数字pin 上下拉电阻选择控制
    {0x00b6,0x03,0x000034},        // 起动功率设置寄存器
    {0x00b7,0x03,0x000480},
};

// 固定配置数据表:其中有部分为冗余设置
const struct MIX_CONFIG_CELL_TYPE FixupConfigTab2[] =     // 四线57.7V
{
    {0x00C3,0x03,0x000000},        // 恢复校表数值到上电初始值

    {0x0081,0x03,0x00b9FE},        // 模式配置寄存器
    {0x0082,0x03,0x000100},        // ADC 增益配置寄存器
    {0x0096,0x03,0x000000},        // 无功相位校正
    {0x009d,0x03,0x000160},        // 起动电流阈值设置
    {0x00b0,0x03,0x000001},        // 中断使能寄存器
    {0x00b1,0x03,0x003437},        // 电路模块配置寄存器
    {0x00b5,0x03,0x00000f},        // 数字pin 上下拉电阻选择控制
    {0x00b6,0x03,0x000034},        // 起动功率设置寄存器
    {0x00b7,0x03,0x000300},        // 相位补偿区域设施 Iregion=INT[Is*2^5]=INT[Ib*N*比例设置点*2^5]=INT[1.5*40*40%*2^5]=0x300
};

// 固定配置数据表:其中有部分为冗余设置
const struct MIX_CONFIG_CELL_TYPE FixupConfigTab3[] =     // 四线220V 3200/6400公共
{    
    {0x00C3,0x03,0x000000},        // 恢复校表数值到上电初始值

    {0x0081,0x03,0x00b9FF},        // 模式配置寄存器
    {0x0082,0x03,0x000100},        // ADC 增益配置寄存器 三相电压2倍增益
    {0x0096,0x03,0x000000},        // 无功相位校正
    {0x009d,0x03,0x000189},        // 起动电流阈值设置 Ib1.5A-50mv,启动电流:0.1%(0.5级表) 2012-9-26
    {0x00b0,0x03,0x000001},        // 中断使能寄存器
    {0x00b1,0x03,0x003437},        // 电路模块配置寄存器  开启温度检测 2012-9-28
    {0x00b5,0x03,0x00000f},        // 数字pin 上下拉电阻选择控制
    {0x00b6,0x03,0x000041},        // 起动功率设置寄存器 INT(0.6*220*1.5*0x13e * 3200*0.1%*2^23/(2.592*10^10)) 2012-9-26 
    {0x00b7,0x03,0x0004E0},        // 相位补偿区域设置 Iregion=INT[Is*2^5]=INT[Ib*N*比例设置点*2^5]=INT[1.5*40*65%*2^5]=0x4E0
};

short  TempAllgainDefault[25] = 
      {-90,-79,-68,-58,-49,-40,-31,-23,
       -12,-9, 0, 0, 0, 0, 0, 0, 
       0, 0, 0,-9,-12,-17,-22,-28,-34};

// 初始谐波校表参数，芯片厂商提供
const double DefaultHarmParaTab[22] = 
    {1.00000000000000
    ,1.00000000000000,1.00362187060665,1.00969162604172,1.01825901332331
    ,1.02939520355364,1.04319331488342,1.05977378492696,1.07927443401769
    ,1.10187021533519,1.12776405100967,1.15718387269867,1.19042304659018
    ,1.22777858879375,1.26962990669109,1.31639900106378,1.36856044411429
    ,1.42671649331348,1.49149037623292,1.56363705732262,1.64398947583697
    ,1.73366802372421};

 // 电压、电流 E211
struct MIX_CONFIG_CELL_TYPE VoltCurrRMSTab[] = 
{
    {0x000d, 3, DATAU},
    {0x000e, 3, 1+DATAU},
    {0x000f, 3, 2+DATAU},
    {0x002b, 3, 3+DATAU},  //三相电压矢量和有效值

    {0x0010, 3, DATAI},
    {0x0011, 3, DATAI+1},
    {0x0012, 3, DATAI+2},
    //{0x0013, 3, DATAI+3},    //三相电流矢量和的有效值

    {0x0029, 3, DATAI+3},   //第七路ADC输入信号的有效值
    {0x003f, 3, DATAI+5}   //第七路ADC采样数据输出
};

 // 功率 E212
struct MIX_CONFIG_CELL_TYPE PowerRMSTab[] = 
{
    {0x0009, 3, DATAS},
    {0x000a, 3, DATAS+1},
    {0x000b, 3, DATAS+2},
    {0x000c, 3, DATAS+3},   //合视在

    {0x0001, 3, DATAP},
    {0x0002, 3, DATAP+1},
    {0x0003, 3, DATAP+2},
    {0x0004, 3, DATAP+3},

    {0x0005, 3, DATAQ},
    {0x0006, 3, DATAQ+1},
    {0x0007, 3, DATAQ+2},
    {0x0008, 3, DATAQ+3}
};

// 相角、频率、功率因数 E213
struct MIX_CONFIG_CELL_TYPE AngleFreqTab[] = 
{
    {0x0018, 3, DATAUIAngle},   //A相电流与电压夹角
    {0x0019, 3, DATAUIAngle+1},
    {0x001a, 3, DATAUIAngle+2},

    {0x0026, 3, DATAUaUbAngle},  //UA UB 夹角
    {0x0027, 3, DATAUaUbAngle+1},  //UA UC 夹角
    {0x0028, 3, DATAUaUbAngle+2},  //UB UC 夹角

    {0x001c, 3, DATAFreq},   //线频率

    {0x002a, 3, DATATPSD}, //温度传感器的输出

    {0x0014, 3, DATACOS},
    {0x0015, 3, DATACOS+1},
    {0x0016, 3, DATACOS+2},
    {0x0017, 3, DATACOS+3}
};
 // 能量 E214
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
// 状态 E215
struct MIX_CONFIG_CELL_TYPE StatusSignTab[] = 
{
    {0x001d, 3, DATAEFlag},
    {0x002c, 3, DATASfalg},

    {0x003d, 3, DATAPFlag},
    {0x003e, 3, DATAChknum}
};

// 电压、电流、功率增益参数表
struct MIX_CONFIG_CELL_TYPE CalMetConfigTab[] =
{
    // 电压、电流增益
    {0x0017,3,0},
    {0x0018,3,0},
    {0x0019,3,0},
    {0x001a,3,0},
    {0x001b,3,0},
    {0x001c,3,0},
    {0x0020,3,0},
    // 功率增益
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
 * @brief     SPI访问接口用户态实现 
 * @param     *spi_device SPI配置信息 
 * @param     *tx_buf     发送数据buffer 
 * @param     tx_size     发送数据长度(字节数)     
 * @param     *rx_buf     接收数据buffer 返回接收的数据     
 * @param     rx_size     接收数据长度（字节数）(SPI只负责数据交换 所以如果要接收数据 rx_size和tx_size相等 tpm芯片除外(特殊的协议特殊处理))    
 * @retval    int 
 * @return    0 成功 非0 失败 
 * @note       
 */ 
int drv_user_spi_transfer (spi_dev_cfg_s *spi_device, char *tx_buf, unsigned int tx_size, char *rx_buf, unsigned int rx_size) 
{ 
    int retval;
    int fd;
    spi_transfer_s spi_data = {0}; 
    unsigned int len; 
    char dev_name[SPI_USER_NAME_MAX_LEN] ={0}; 
 
    /* 如果本次访问只要发送数据 不需要接收数据 接收buffer可以为空 */ 
    if ((NULL == spi_device) || (NULL == tx_buf) || (0 == tx_size) ||(NULL == spi_device->bus_name) ) 
    { 
        DPRINT("spi transfer invalid para\r\n"); 
        return -1;   /* 此处为示例代码，为了方便演示，所有异常返回值都定义为-1 */ 
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
 
    /*构造SPI传输用户态需要传递给内核态的数据*/ 
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
 * @brief     ade9087交采芯片SPI访问接口 
 * @param     *cmd_buf    发送命令buffer 
 * @param     cmd_size    发送命令长度(字节数)     
 * @param     *rx_addr    接收数据buffer 返回接收的数据     
 * @param     rx_len      接收数据长度(字节数)     
 * @param     dummy_num   命令字与读出的数据之间间隔字节数    
 * @retval    int 
 * @return    0 成功 非0 失败 
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
    dev_cfg.pad0 = 0; /***此处必须保证补的数据为0，否则内核态访问会有问题***/ 
#endif 
    dev_cfg.bus_name = "spi_bus0"; 
    dev_cfg.mode = SPI_DEV_MODE_1; 
    dev_cfg.speed = 400000;   /*SPI总线跨板，此处设置为1M*/ 
    dev_cfg.bits_per_word = 8; 
    dev_cfg.protocal_type = SPI_DEV_PROTOCAL_NORMAL; 
    dev_cfg.cs_id = 1;   /*片选编号，由硬件决定*/ 
 
    memcpy(data_buf, cmd_buf, cmd_size); 
    retval = drv_user_spi_transfer (&dev_cfg, data_buf, data_len, data_buf, data_len); 
    (void)memcpy((char *)rx_addr, data_buf + cmd_size + dummy_num, rx_len); 
 
    free(data_buf); 
 
    return retval; 
} 
 
/*! 
 * @brief     ade9087交采芯片SPI访问接口 
 * @param     *cmd_buf    发送命令buffer 
 * @param     cmd_size    发送命令长度(字节数)     
 * @param     *tx_addr    写入数据buffer 返回接收的数据     
 * @param     tx_len      写入数据长度(字节数)     
 * @retval    int 
 * @return    0 成功 非0 失败 
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
    dev_cfg.pad0 = 0; /***此处必须保证补的数据为0，否则内核态访问会有问题***/ 
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
* 函数名称: WriteReg7022E
* 函数功能: 写7022E参数寄存器，带重写
* 输入参数: addr   寄存器地址，MAX_CAL_REG_ADDR
*           spval  待写入的寄存器数据
* 输出参数: 无
* 返 回 值: 0 成功; < 0 失败
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
* 函数名称: ReadReg7022E
* 函数功能: 读7022E输出寄存器，带重读
* 输入参数: addr       寄存器地址，输出寄存器 0x00~MAX_OUT_REG_ADDR；校表区寄存器 0x00~MAX_CAL_REG_ADDR
* 输出参数: SPIDataBuf 读出的寄存器数据，低字节在前
* 返 回 值: 0 成功; < 0 失败
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
* 函数名称: WriteContFun7022E
* 函数功能: 校表写使能打开或关闭
* 输入参数: Mark 0 校表写使能打开；1 校表写使能关闭
* 输出参数: 无
* 返 回 值: >=0:正常    -1:出错
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

// ATT7022 CPOL = 0; CPHA = 1;空闲为低 ,上升沿放数据，下降沿取数据
int  Reset_ACChip(void)
{
	int i;
	DWORD chipid;

	//上电
	Power_ATT7022E(1);
	usleep(500);
	//复位
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
	myprintf(SYS_ID, LOG_ATTR_ERROR,"硬复位7022E失败!!! \n");
	WriteWarnEvent(NULL, SYS_ERR_AD, 0, "硬复位7022E失败!!! \n");
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
		//上电
		regaddr = 0x03;
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data|0x01;	//只改变最低状态位，拉高
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
	}
	else if(iType == 2)
	{
		//下电
		regaddr = 0x03;
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data&0xFE;	//只改变最低状态位，拉底
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
	}
	else if(iType == 3)
	{
		//复位
		regaddr = 0x02;
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data&0x7F;	//只改变最高状态位，拉低
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
		usleep(100000);
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data|0x80;	//只改变最高状态位，拉高
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
		usleep(100000);
	}
	return 0;
}

/*******************************************************************************
* 函数名称: EnableTempCurrMeter
* 函数功能: 使能温度、电流测量(用于上电首次检测温度与电流区间)
* 输入参数: 无
* 输出参数: 无
* 返 回 值: 0 成功; -1 失败
*******************************************************************************/
int EnableTempCurrMeter(void)
{
    int i;
    // 使能温度、电流测量的配置
    const struct MIX_CONFIG_CELL_TYPE EnableTempCurrTab[] =
    {
        {0x0001,0x03,0x00B97F},        // 模式配置寄存器
        {0x0031,0x03,0x000012}        // 电路模块配置寄存器
    };

    if (WriteContFun7022E(0x00) < 0)   // 打开写使能
    {
        return -1;
    }
    for (i = 0; i < (int)(sizeof(EnableTempCurrTab) / sizeof(struct MIX_CONFIG_CELL_TYPE)); i++)
    {
        if (WriteReg7022E(EnableTempCurrTab[i].Addr, (DWORD)EnableTempCurrTab[i].Data) < 0)
        {
            WriteContFun7022E(0x01);   // 关闭写使能
            shellprintf("使能温度、电流测量Error!!!\n");
            myprintf(SYS_ID, LOG_ATTR_ERROR, "使能温度、电流测量Error!!!");
            return -1;
        }
    }
    WriteContFun7022E(0x01);           // 关闭写使能

    shellprintf("使能温度、电流测量OK!!!\n");

    return 0;
}

/*******************************************************************************
* 函数名称: FixupConfigFun7022E
* 函数功能: ATT7022E固定配置报文数据区组织
* 输入参数: step 0 初始化；1 写固定配置
* 输出参数: 无
* 返 回 值: 组织数据长度
*******************************************************************************/
/*待定 启动电流，启动功率、相位补偿*/
int FixupConfigFun7022E(WORD step)
{
    WORD i = 0;
    WORD RegCnt;
    int    retnum = 0;
    float  Un;       // 额定电压

    Un = 220;    // 额定电压 220V
    FixupConfigTab = FixupConfigTab3;
    RegCnt = (WORD)(sizeof(FixupConfigTab3)/sizeof(struct MIX_CONFIG_CELL_TYPE));

    if (step == 0)       // 恢复校表数值到上电初始值
    {
        if (WriteReg7022E(FixupConfigTab[0].Addr, (DWORD)FixupConfigTab[0].Data) < 0)
        {
            retnum = -1;
        }
    }
    else                 // 固定配置，从第二个寄存器开始
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
            // 计算高频脉冲常数设置值并下发给寄存器
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
* 函数名称: CalMetConfigFun7022E
* 函数功能: 下发校表参数--ATT7022E
* 输入参数: 无
* 输出参数: 无
* 返 回 值: >=0 正常；< 0 失败
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

    // 下发电压、电流、功率增益参数
    for(i = 0; i < (int)(sizeof(CalMetConfigTab)/sizeof(struct MIX_CONFIG_CELL_TYPE)); i++)
    {
        if(!(pYcGain[tempno[i]].status & YC_GAIN_BIT_VALID))
        {
            continue;
        }

        if (WriteReg7022E(CalMetConfigTab[i].Addr, (DWORD)CalMetConfigTab[i].Data) < 0)
        {
            shellprintf("下发电压、电流、功率增益参数 Error!!! Addr=%X Data=%X\n"
                , (unsigned)CalMetConfigTab[i].Addr, (unsigned)CalMetConfigTab[i].Data);
            return -1;
        }
    }

    // 下发全通道增益
	val = (int)p7022gain->allGain[TempInfo7022E.TempZone]&0xFFFF;
    if (WriteReg7022E(0x32, (DWORD)val) < 0)
    {
        shellprintf("初始化写全通道增益 Error! HisTempSection=%d Gain=%u\n", TempInfo7022E.TempZone, val);
        myprintf(SYS_ID, LOG_ATTR_ERROR, "初始化写全通道增益 Error! CurTempSection=%d Gain=%u"
            ,TempInfo7022E.TempZone, val);
    }
    else
    {
        // 计算当前温度区对应全通道增益
        AllGainVal = val;
    }
    // 下发相位校正参数
    for (i = 0; i < 3; i++)
    {
      
		idx = CurrentInfo7022E.IZone[i] + TempInfo7022E.ITempZone*I_ZONE_NUM;

        RegAddr =(DWORD)(((CurrentInfo7022E.IZone[i] >= 2)?0x0D:0x10) + i);  // 寄存器地址
        j = (CurrentInfo7022E.IZone[i] >= 2)?0:3;    // 计算偏移，区间2~3 无偏移，区间0~2 对应0x90~0x92寄存器
        val = (int)p7022gain->allIAng[i][idx] & 0xFFFF;
		
        // 写计量芯片角差补偿寄存器
        if (WriteReg7022E((WORD)RegAddr, (DWORD)val) < 0)
        {
            retnum = -2;
            shellprintf("初始化写角差补偿寄存器失败!");
        
            // 写失败后，重写上次值，防止寄存器的值错乱
            WriteReg7022E((WORD)RegAddr, ITempRegVal[i+j]);
        }
        else
        {
            // 写成功后，记录相应的值到本地变量
            ITempRegVal[i+j] = (DWORD)val; 
        }

		
        // 若电流处于低段且高段没有参数，则写100%点参数
        if ((CurrentInfo7022E.IZone[i] <= 1) && (ITempRegVal[(3-j)+i] == 0))
        {
            idx =  2 + TempInfo7022E.ITempZone*I_ZONE_NUM;
            ITempRegVal[(3-j)+i] = (int)p7022gain->allIAng[i][idx]&0xFFFF;
			RegAddr = (DWORD)(0x0D + i);
        }

        // 若电流处于高段且低段没有参数，则写20%点参数
        if ((CurrentInfo7022E.IZone[i] >= 2) && (ITempRegVal[(3-j)+i] == 0))
        {
            idx = 1 + TempInfo7022E.ITempZone*I_ZONE_NUM;
            ITempRegVal[(3-j)+i] = (int)p7022gain->allIAng[i][idx]&0xFFFF;
            RegAddr = (DWORD)(0x10 + i);
        }
        
        if (ITempRegVal[(3-j)+i] != 0)
        {
            // 写计量芯片相反角差补偿寄存器
            if (WriteReg7022E((WORD)RegAddr, ITempRegVal[(3-j)+i]) < 0)
            {
                retnum = -2;
            }
        }
    }
    return retnum;
}

/*******************************************************************************
* 函数名称: FixupConfigEMU7022E
* 函数功能: 写EMU配置，使能能量计量
* 输入参数: flag:WRITE_REG_FLAG 写寄存器, CLEAR_REG_FLAG 清寄存器
* 输出参数: 无
* 返 回 值: >=0:正常    <=-1:出错
*******************************************************************************/
int FixupConfigEMU7022E()
{
    DWORD EMUData;
    int retnum = 0;

    EMUData = 0x003DC4;  // 三相四线EMU

    if (WriteReg7022E(0x03, EMUData) < 0)
    {
        shellprintf( "初始化EMU设置错误!!! \n");
        retnum = -1;
    }

    return retnum;
}


/*******************************************************************************
* 函数名称: ComInitATTFun7022E
* 函数功能: 通信初始化ATT7022处理
* 输入参数: 无
* 输出参数: 无
* 返 回 值: >=0:正常    <=-1:出错
*******************************************************************************/
int ComInitATTFun7022E(void)
{
    int retnum = 0;

     // 软件初始化
     shellprintf("ATT7022E 软件初始化\n");
     retnum += FixupConfigFun7022E(0);
     thSleep(1);

	 // 固定配置
     shellprintf("ATT7022E 写固定配置\n");
     retnum += FixupConfigFun7022E(1);

     // 校表配置
     shellprintf("ATT7022E 写校表配置\n");
     retnum += CalMetConfigFun7022E();
            
     // EMU配置，使能能量计量
     shellprintf("ATT7022E 写EMU配置\n");
     thSleep(60);  // 需要500ms以上
     retnum += FixupConfigEMU7022E();
   

	if(retnum < 0)
       return -1;
    
    return retnum;
}

/*******************************************************************************
* 函数名称: InitMeaICFun7022E
* 函数功能: 计量芯片初始化
* 输入参数: 无
* 输出参数: 无
* 返 回 值: >=0:正常    <=-1:出错
*******************************************************************************/
int InitMeaICFun7022E(void)
{    
    int retnum = 0;

    //retnum += ReadMeaICModeMark7022E();    // 判断交采接线方式，判断是三相三线、三相四线、57.7V等
    retnum += WriteContFun7022E(0x00);     // 使能SPI校表数据写操作
    if(retnum < 0)
    {
        shellprintf("初始化7022E，写使能失败!!!\n");
        return -1;
    }
    retnum += ComInitATTFun7022E();        // 组织ATT7022E的控制寄存器数据
    retnum += WriteContFun7022E(0x01);     // 7022校表寄存器写使能关闭
    
    return retnum;
}


/*******************************************************************************
* 函数名称: ReadACInstDataFun7022E
* 函数功能: 读取ATT7022的实时输出寄存器数据
* 输入参数: DIMark 数据标识
* 输出参数: 无
* 返 回 值: 1 正常； -1 异常
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
{// 100倍
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
{ // 1000 倍
	DWORD *pSrc;
	int *pDest;
	INT64U MidNum;
	DWORD N = 60/RATED_CURR_VALUE; // N为比例系数，当额定电流Ib取样为50mV时N=60/Ib

	pSrc = (DWORD*)& InstantSampData7022E[DATAI+i];
	pDest = (int*)data;

	MidNum = ((INT64S)(pSrc[0]) * 1000);    //系数具体待定
	MidNum >>= 13;

	pDest[0] = (int)(MidNum/N);
	
	IAtt7022e[i] = (long)(MidNum/N);
}

int PowerCalSubProgram(int PowSrcNum, BYTE CalMark)
{// 放大10倍
    INT64S MidNum;
	
	if(Att7022eInit != 1)
		return 0;	
	if(PowSrcNum > (1 << 23))
	{
		PowSrcNum = PowSrcNum - (1 << 24);
	}
	

    /* 功率校表系数 */
    
       ////////////////////2592 数待写 2018年2月9日 09:01:30///////////////////////////////
	MidNum = 2592*100000/(HFconst*ACPulseConst/100);
	if (CalMark == 0)                        //分相功率(最大需量)计算使用
        MidNum = (INT64S)((INT64S)PowSrcNum *10 * MidNum /(1 << 23));
    else if (CalMark == 1)                    //总功率(最大需量)计算使用
        MidNum = (INT64S)((INT64S)PowSrcNum *10 * MidNum /(1 << 22));
    
    //MidNum /= PulsConst;
	//shellprintf("PQ	原始%d    计算 %d \n", PowSrcNum , MidNum);
		
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
    ulcos = ulcos & 0xffffff;  //24位数
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

//读温度
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
	
	if (abs(I - CurrentInfo7022E.IBak[phase]) > In * 0.01) // I的倍数多少, hll待修改
    {
	    if (I > CurrentInfo7022E.IBak[phase]) // 电流向上变化
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
	    else  // 电流向下变化
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
        if (ITempRegVal[3+idex] == 0)        // 若当前相应的寄存器没有值，则不需清除相应的寄存器值 
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
        if (ITempRegVal[idex] == 0)        // 若当前相应的寄存器没有值，则不需清除相应的寄存器值 
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
			  /* 无意义
			  if (PhaRegDeal(i, CurrentInfo7022E.IZoneBak[i], CurrentInfo7022E.IZone[i]) >= 0) // 校表状态下，电流区间变化，则判断是否共用相同寄存器
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

			   if (WriteStart == 0)   // 首次写角差增益需要写使能
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


    // 超出温度区间，认为当前温度异常，温度区间不予改变
    if ((TempInfo7022E.TempCur < -50)||(TempInfo7022E.TempCur > 90))
    {
        tempAltCnt = 0;
        return 0;
    }

    // 温度无变化，温度区间不予改变
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

    // 当前温度区间及历史温度区间均处于常温区，温度区间不予改变 0~50度不做温补,10~50
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

        // 连续5次温度区间不一致，才允许更换温度区间
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
* 函数名称: AllChannelGain
* 函数功能: 根据当前温度区间设置全通道增益寄存器
* 输入参数: 无
* 输出参数: 无
* 返 回 值: >= 设置成功；< 0，设置失败 
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

	  // 校表寄存器写使能开启
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
    
    // 校表寄存器写使能关闭
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
    for(i = 0 ; i < 6 ; i++) // 谐波参数缺省处理
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
	thSleep(5); //新联代码未提到延时
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

//四象限电度值
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
		case 0:  //正向有功
				if((dudir & EMU_PDIR_MASK) != EMU_PDIR_MASK )
				{	
					val = (DWORD)InstantSampData7022E[DATAEpa+3];
				}
				break;
		case 1: 	//正向无功
				if((dudir & EMU_QDIR_MASK) != EMU_QDIR_MASK )
				{	
					val = (DWORD)InstantSampData7022E[DATAEqa+3];
				}
				break;	
		case 2: //一象限
				if(((powdir & PSIGN_MASK) != PSIGN_MASK ) && ((powdir & QSIGN_MASK) != QSIGN_MASK))
				{
					val = (DWORD)InstantSampData7022E[DATAEqa+3];
				}
				break;
		case 3: // 四象限
				if(((powdir & PSIGN_MASK) != PSIGN_MASK ) && ((powdir & QSIGN_MASK) == QSIGN_MASK))
				{	
					val = (DWORD)InstantSampData7022E[DATAEqa+3];
				}
				break;	
		case 4://反向有功电度
				if((dudir & EMU_PDIR_MASK) == EMU_PDIR_MASK )
				{	
					val = (DWORD)InstantSampData7022E[DATAEpa+3];
				}
				break;
		case 5:	//反向无功
				if((dudir & EMU_QDIR_MASK) == EMU_QDIR_MASK )
				{	
					val = (DWORD)InstantSampData7022E[DATAEqa+3] ;
				}
				break;
		case 6:	//二象限
				if(((powdir & PSIGN_MASK) == PSIGN_MASK) &&((powdir & QSIGN_MASK) != QSIGN_MASK))
				{
					val = (DWORD)InstantSampData7022E[DATAEqa+3];
	      			}
			   	break;
		case 7:	//三象限
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

//检测潮流方向
void ActPowerFlowFaultEvent(BYTE bType)
{
	int  vlt[3] = {0};						//-当前３相电压-
	int  cur[3] = {0};						//-当前３相电流-
	int  actPower[3] = {0};					//-当前３相有功	
	int  actPowerFlowAll;
	WORD  lastTimesLmt = 60;   			    //持续时间限值,60s
	static DWORD lastTimes[2] = {0};	//事件发生/恢复持续时间
	BYTE	tmp_flag = 0;   /* 潮流反向或反向恢复满足功率方向条件 */
	BYTE	is_now_flag = 0; 
	int i;
	struct  VDDF dbExtDdValue;
	struct VDDFT Ddbuf;
	static struct VCalClock bFlowRevFrzCP56Time;
	struct VSysFlowEventInfo Flowinfo;

	//-读取实时电压、电流-
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
		return;//-电压电流停电-
	}

	actPowerFlowAll = actPower[0] + actPower[1] + actPower[2];

	if(bType == 1)		//潮流反向
	{
		if(actPowerFlowAll < 0)
		{
			tmp_flag = labs(actPowerFlowAll) > POWER_REV;
		}
	}
	else if(bType == 2)	//潮流反向恢复
	{
		if(actPowerFlowAll > 0)
			tmp_flag = actPowerFlowAll > POWER_REV;
	}
	else
	{
		return;
	}
	
	if(tmp_flag)		//如果满足相应的发生条件，将对应的时间进行累加
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
			//if(Flowinfo.num)   //暂时先屏蔽潮流文件
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
			//if(Flowinfo.num)   //暂时先屏蔽潮流文件
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
	
	
	j = (pclk->byHour*60+pclk->byMinute)/30;//当前时段
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
		att7022ddAllday(&curclock);//日总有功无功
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
* 函数名称: InitCurrTempPara
* 函数功能: 判断初始状态下的温度区间与电流区间等参数
* 输入参数: 无
* 输出参数: 无
* 返 回 值: 0 成功; < 0 失败
*******************************************************************************/
int InitCurrTempPara(void)
{
	long IVal[3];
	int i;
   
	ReadACInstDataFun7022E(ATT7022_REG_UI);

    for (i=0; i<3; i++)
	{
	     Real_I(i, IVal+i);
		 JudgeIZone((short)IVal[i], RATED_CURR_VALUE*1000, i);   //In的倍数，默认1.5A， hll待修改
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
* 函数名称: Init7022E
* 函数功能: 计量芯片初始化
* 输入参数: 无
* 输出参数: 无
* 返 回 值: > 0 初始化完成； < 0 初始化失败或上电首次初始化完成
*******************************************************************************/
int Init7022E(void)
{  
    Att7022eDd_Init();
    if (Reset_ACChip() < 0)                 // 复位7022E
    {
        return ERROR;
    }

	LoadATT7022Gain();
	//InitHREnv7022E();  //读取谐波校表数据
	EnableTempCurrMeter();   // 使能温度、电流测量
	thSleep(1);
	InitCurrTempPara();      // 判断当前温度区间与电流区间等参数
    if (InitMeaICFun7022E() < 0)
    {
        shellprintf("7022E初始化失败，重新初始化!!!\n");
        return ERROR;
    }
	
    Att7022eInit = 1;
	return OK;
}

void Cal_U_gain(BYTE id ,int value,int TheoreticVolt, VYcGain* pycgain)
{
	int ActualVolt;    // 实际电压
	INT64U MidNum = 0;
	short gain = 0;
         
	if((id < 1) || (id >3))
		return;
	
	ActualVolt =  value;
	
	if(ActualVolt != 0)
	{
		if(ActualVolt <= TheoreticVolt)
		{
			//正增益
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
		{// 负增益
			MidNum = (DWORD)(ActualVolt  - TheoreticVolt);
            MidNum <<= 15;
            ActualVolt  = (int)(MidNum / (DWORD)ActualVolt);
            ActualVolt  = (int)(~(unsigned int)ActualVolt);
            ActualVolt  += 1;
			gain = (short)ActualVolt;
		}
	}
	pycgain->a = pycgain->a +  gain + ((short)pycgain->a*gain)/(1<<15);
	if (WriteContFun7022E(0x00) < 0)   // 打开写使能
	{
		return ;
	}
	thSleep(1);

	pycgain->a  =  pycgain->a & 0xffff;
	WriteReg7022E(CalMetConfigTab[0].Addr +id -1, (DWORD)pycgain->a);

	WriteContFun7022E(0x01);           // 关闭写使能
	
}

void Cal_I_gain(BYTE id ,int value,int TheoreticCurr, VYcGain* pycgain)
{
	int ActualCurr;  //实际电流
	short gain = 0;
	INT64U MidNum;
	
	if((id < 1) || (id >4))
		return;
    ActualCurr = value;
		
	if(ActualCurr != 0)
	{
		if(ActualCurr <= TheoreticCurr)
        {    // 正增益
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
        {    // 负增益
            MidNum = (DWORD)(ActualCurr - TheoreticCurr);
            MidNum <<= 15;
            ActualCurr = (int)(MidNum / (DWORD)ActualCurr);
            ActualCurr = (int)(~(unsigned int)ActualCurr);
            ActualCurr += 1;
            gain = (short)ActualCurr;
        }
	}
	
	pycgain->a  =  pycgain->a + gain + ((short)pycgain->a*gain)/(1<<15);
	if (WriteContFun7022E(0x00) < 0)   // 打开写使能
	{
		return;
	}
	thSleep(1);

	pycgain->a  =  pycgain->a & 0xffff;
	if (id == 4)
		WriteReg7022E(CalMetConfigTab[6].Addr, (DWORD)pycgain->a);
	else
		WriteReg7022E(CalMetConfigTab[3].Addr + id - 1, (DWORD)pycgain->a);
	WriteContFun7022E(0x01);           // 关闭写使能
	
}

void Cal_PQ_gain(BYTE id, int gain1, int gain2,VYcGain* pycgain)
{
	float Pgain;
	int temp;
	short GP1 = 0;
	
	if((id < 1) || (id >3))
		return;
    
       //功率再说
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
		
	if (WriteContFun7022E(0x00) < 0)   // 打开写使能
    {
            return;
    }
	thSleep(1);
	pycgain->a += GP1 +  ((short)pycgain->a*GP1)/(1<<15);
	pycgain->a = pycgain->a & 0xffff;
	WriteReg7022E(CalMetConfigTab[7].Addr+ id -1 , (DWORD)pycgain->a); 
	WriteReg7022E(CalMetConfigTab[10].Addr + id -1 , (DWORD)pycgain->a); 
	
	WriteContFun7022E(0x01);           // 关闭写使能
}

//相位校正
int Cal_angle_gain(BYTE id, int value, int pTheoretic)
{
	int Actual; //实际值; COS = 1,角度60 ； 放大100倍
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
			//正增益
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
	
	if (WriteContFun7022E(0x00) < 0)   // 打开写使能
    {
        return -1;
    }
	thSleep(1);
	val =(int)gain&0xFFFF;
	addr = (CurrentInfo7022E.IZone[id-1] >= 2) ? 0x0D:0x10;
	WriteReg7022E(addr + id -1 , val);
	WriteContFun7022E(0x01);           // 关闭写使能

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
	float det; // 行列式
	
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
			//正增益
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
	        gain->allGain[i] = 0; // 0~25度不补偿全通道增益
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
	        gain->allGain[i] = 0; // 30~50度不补偿全通道增益
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

	if (WriteContFun7022E(0x00) < 0)   // 打开写使能
    {
        return -1;
    }
	thSleep(1);
	val  = (int)allgain & 0x0000FFFF;
	WriteReg7022E(0x32 , val);
	WriteContFun7022E(0x01);           // 关闭写使能
   
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
{// 放大100倍
	int pSrc;
	int pDest;
	INT64S MidNum = 0;
  
        //待写2018年2月9日 09:04:07
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
{// 放大100倍
	int pSrc;
	int pDest;
	INT64S MidNum = 0;
  
        //待写2018年2月9日 09:04:07
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
* 函数名称: EE
* 函数功能: 复数相乘
* 输入参数: b1  复数1
*           b2  复数2
* 输出参数: 无
* 返 回 值: 乘积
****************************************************************************/
static struct compx EE(struct compx b1,struct compx b2)
{
    struct compx b3;
    
    b3.real = (b1.real * b2.real - b1.imag * b2.imag);
    b3.imag = (b1.real * b2.imag + b1.imag * b2.real);
    
    return (b3);
}

/****************************************************************************
* 函数名称: FFT
* 函数功能: FFT计算
* 输入参数: xin  原始各个采样数据
*           N    FFT计算的点数
* 输出参数: xin  计算结果
* 返 回 值: 无
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
* 函数名称: CalVoltCurrBaseRMS
* 函数功能: 计算各个通道基波有效值
* 输入参数: 无
* 输出参数: 无
* 返 回 值: 0 正常
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
* 函数名称: CalHRRealData7022E
* 函数功能: 计算谐波实时数据（电网频率、各次谐波分量）
* 输入参数: 无
* 输出参数: 无
* 返 回 值: 无
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

      shellprintf("电网频率错误 !\n");
    }
    
     // 计算各通道基波有效值
    CalVoltCurrBaseRMS();
    // fft计算当前瞬时有效值，即根据当前采样点fft计算出的当前有效值
    // 不同于实时有效值，因为实时有效值还需要从之前的6个滑差值计算方均根获得
     mt_fft7022E();
     
}

/****************************************************************************
* 函数名称: mt_fft7022E
* 函数功能: 电压、电流通道各次谐波分量计算
* 输入参数: 无
* 输出参数: 无
* 返 回 值: 无
****************************************************************************/
void mt_fft7022E(void)
{  
    int i = 0, n = 0;
    struct compx xxin[SN+1];  // SN=64, 64点FFT
    float temp = 0.0;    
    float harmVoltDevid = 1.0;
    float harmCurrDevid = CURR_CAL_FACTOR; 
    float amend_offset = 0;

    
     harmVoltDevid = 4.545454 / VOLT_CAL_FACTOR_3P4L_220; // 1.118193

     // 6个通道依次计算基波、2~19次谐波有效值
    for (i = 0; i <= 5; i ++)
    {
       for (n = 1; n <= SN; n ++)        
        {             
            xxin[n].real = AD_RESULT[i][n+3]; // 丢掉前面4个点		
            xxin[n].imag = 0.0;
        }

        FFT (xxin, SN);
        for (n = 1; n < 20; n ++)           // 计算各次谐波分量
        {  
            if (i < 3) // 各次电压谐波分量的计算
            {
                temp=sqrt((xxin[n+1].real*xxin[n+1].real+xxin[n+1].imag*xxin[n+1].imag))*1.576155207/1.4142/64.0;
            }
            else    // 各次电流谐波分量的计算
            {
                temp=sqrt((xxin[n+1].real*xxin[n+1].real+xxin[n+1].imag*xxin[n+1].imag))/20.65530595/1.4142/64.0;
            }
            temp /= 20.0;

            if (n == 1)
            {
                amend_offset = VoltCurrBaseRMS[i] / temp;
            }

            if (i < 3) // 各次电压谐波分量计算
            {
                component_O_7022E[i][n] = temp * amend_offset / harmVoltDevid * amend_mt[i][n];
            }
            else      // 各次电流谐波分量计算
            {
                component_O_7022E[i][n] = temp * amend_offset * harmCurrDevid * amend_mt[i][n];
            }
        }
         if ((i <= 2)&&(component_O_7022E[i][1] < 5.0))
        {
            // 各相电压谐波的基波电压若小于5V，则将各次谐波分量消零
            for(n = 0; n <= 19; n ++)        
            {
                component_O_7022E[i][n] = 0.0;
            }
        }
        else if ((i>=3)&&(component_O_7022E[i][1] < 0.005))
        {    
            // 各相电流谐波的基波电流若小于5mA，则将各次谐波分量消零
            for (n = 0; n <= 19; n ++)        
            {
                component_O_7022E[i][n] = 0.0;
            }
        }
    }

}


/****************************************************************************
* 函数名称: HarmCalibrate7022E
* 函数功能: 谐波校表，功率校表主台扩展的07规约
* 输入参数: pParamBuffer  主台输入的校表规约，包括数据标识和标识对应的数据
*           len           报文数据长度
* 输出参数: 无
* 返 回 值: 0 失败；1 成功
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
        // 校电压或电流
    }
    else
    {
        // 检查是否是谐波参数初始化规约 0xEFCDAB8967
        if (memcmp(pParamBuffer, "\xEF\xCD\xAB\x89\x67", 5) == 0)
        {
            // 各相电压谐波增益初始化
            for (i = 0; i < 3; i++)
            {
                component[i][0] = 10000;    // 谐波初始化，将各通道基波增益初始化为 1.0 * 10000
                for (j = 0; j < 18; j++)
                {
                    component[i][j+1] = amend_off_U[j] * 10000;
                }
            }

            // 各相电流谐波增益初始化
            for (i = 0; i < 3; i++)
            {
                component[i+3][0] = 10000;  // 谐波初始化，将各通道基波增益初始化为 1.0 * 10000
                for (j = 0; j < 18; j++)
                {
                    component[i+3][j+1] = amend_off_I[j] * 10000;
                }
            }
        }
    }

    phase = (DI0 & 0x0f)-1;  // 0~5的相位索引，0~2位abc电压 3~5位abc电流
    
    // 计算基准值 电压:XXX.XXX  电流:XX.XXXX
    val=((DI4 >> 4) & 0xf) * 100000 + (DI4 & 0x0f) * 10000 + ((DI3 >> 4) & 0xf) * 1000 + (DI3 & 0x0f)*100
        + ((DI2 >> 4) & 0xf) * 10 + (DI2 & 0x0f);

    DI1 = (DI1>>4)*10+(DI1&0x0f);
    
    if(phase >= 0 && phase < 6
        && DI1 >= 1 && DI1 < 20)
    {
    }
    else
    {   
        if (phase == 0x0E) // 谐波初始化规约
        {
        }
        else
        {
            // 规约错误
            return 0;
        }
    }

    // 计算校表参数，电压为1位小数，点流为2位小数，分别乘 0.1、0.01
    // 校表参数获得: 基准值 * 当前校表参数 / 实时有效值 == 新的校表参数
    if (phase >= 0 && phase < 3)
    {   
        amend = ((float)val)*amend_mt[phase][DI1]/component_O_7022E[phase][DI1]*0.001;
        
        WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"%c相电压基波校正 Theoretic=%d Actual=%f amend=%f"
            , 'A' + phase, val, component_O_7022E[phase][DI1], amend);
    }
    else if (phase >= 3 && phase < 6)
    {
        amend = ((float)val)*amend_mt[phase][DI1]/component_O_7022E[phase][DI1]*0.0001;
        WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"%c相电流基波校正 Theoretic=%d Actual=%f amend=%f"
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

    // 设置对应相的新的校表参数
    if (phase >= 0 && phase < 3)
    {
        component[phase][DI1-1] = (WORD)((DWORD)(amend*10000));
    }
    else if (phase >= 3 && phase < 6)
    {
        component[phase][DI1-1] = (WORD)((DWORD)(amend*10000));
    }
    
    if (DI1 == 1)  // 基波校准
    {   
        // 基波校准，对2-19次进行经验校准
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
* 函数名称: SetHarmPara7022E
* 函数功能: 设置谐波参数
* 输入参数: pParamBuffer  主台输入的校表数据
*           len           报文数据长度
* 输出参数: 无
* 返 回 值:  >= 0 成功；< 0 失败
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
        shellprintf("下发谐波校表参数，保存失败!!! ret=%d\n", retnum);
        WriteWarnEvent(NULL, SYS_ERR_GAIN, 0,"下发谐波校表参数，保存失败!!! ");
    }
    return retnum;
}

// 谐波校表参数，初始均为1
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
* 函数名称: InitHREnv7022E
* 函数功能: 初始化环境，读取谐波校表参数，在初始化、校谐波表以后调用该函数获取当前校准参数
* 输入参数: 无
* 输出参数: pTime  存储当前时间结构指针
* 返 回 值: 目前都返回1，如果能够读EEProm(表示已校表)，则使用；否则使用缺省参数(此时有效值等数据将不准确，须校表)
****************************************************************************/
int InitHREnv7022E( )
{
    int   component[6][19];
    int   i = 0,j = 0;    
    BYTE2_INT16_TYPE *pHarmGain = NULL;
    float ftemp = 0.0;
    
    // 将谐波校表数据
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
                    shellprintf("谐波电压增益超限制!!! 正常增益为5000~21000\n");
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
                    shellprintf("谐波电流增益超限制!!! 正常增益为5000~21000\n");
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

/*输入参数: Ampa: A相幅值，Pha:A相相位
Ampa:A相幅值，Pha:A相相位
Ampa:A相幅值，Pha:A相相位
返回值:正序分量
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

/*输入参数: Ampa: A相幅值，Pha:A相相位
Ampa:A相幅值，Pha:A相相位
Ampa:A相幅值，Pha:A相相位
返回值:负序分量
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

/*输入参数: Ampa: A相幅值，Pha:A相相位
Ampa:A相幅值，Pha:A相相位
Ampa:A相幅值，Pha:A相相位
返回值:零序分量
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
/*谐波校准  ??
角度及幅值  ??
填谐波及相位及幅值
电能量及电能量上传等等

填谐波及相位及正负序
另sqrt abs cos sin pow 等math.h的函数不知道是否可用？？？*/
#endif
#endif
