/*
 * @Author: zhongwei
 * @Date: 2019-11-20 08:51:54
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 11:45:27
 * @Description: 硬件驱动接口
 * @FilePath: hd_driver.c
 */
#include "plt_inc_c.h"
#include "libdrv.h"

SECTION_PLT_DATA uint32 g_HdDrvDebug=0;
//版本
SECTION_PLT_CODE const char * get_version_libdrv(void)
{
    return VERSION_LIB_DRV;
}

//编译时间
SECTION_PLT_CODE TFullDateTime get_buildtime_libdrv(void)
{
    return ParseDateTimeString(__DATE__, __TIME__);
}

//获取当前CPU core num
SECTION_PLT_CODE uint32 hd_get_current_cpu_num(void)
{
    uint32 affinity;
    __asm ("MRC p15, 0, %0, c0, c0, 5" : "=r" (affinity));
    return affinity & 0xFF;
}

/**
 * @Function: hd_get_current_cpu_main_id
 * @Description:  获取当前CPU Main ID Register
 * @author zhongwei (2019/12/15 15:23:50)
 * 
 * @return SECTION_PLT_CODE uint32 
 *  
 * 具体含义请参照《DDI0388G_cortex_a9_ARM Technical Reference Manual_r3p0_trm》文档MIDR寄存器说明 
*/
SECTION_PLT_CODE uint32 hd_get_current_cpu_main_id(void)
{
    uint32 id;
    __asm ("MRC p15, 0, %0, c0, c0, 0" : "=r" (id));
    return id;
}


/**
 * @Author: zhongwei
 * @Date: 2019-11-20 09:00:30
 * @description: 获取当前cycle计数器的值
 * @param {type} 
 * @return: 当前CPU cycle计数器的值，uint32
 */
SECTION_PLT_CODE uint32 hd_get_cycles(void)
{
    XTime ctm;              //uint64
    XTime_GetTime(&ctm);
    return LODWORD(ctm);     //获取低位uint32
}

/**
 * @Author: zhongwei
 * @Date: 2019-11-20 09:02:00
 * @description: 获取当前cycle计数器的值
 * @param {type} 
 * @return: uint64 
 * cycle counter在cpu_init.S中初始化
 */
SECTION_PLT_CODE uint64 hd_get_cycles64(void)
{
    XTime ctm;              //uint64
    XTime_GetTime(&ctm);
    return ctm;
}

//硬件休眠一段cycle
SECTION_PLT_CODE void hd_sleep_cycles(int64 cycle)
{
    uint32 cycle1, cycle2;
    int64 cycle_use = cycle;
    cycle1 = hd_get_cycles();       //获取当前表示的周期数

    while(cycle_use>0)
    {
        cycle2 = hd_get_cycles();

        cycle_use = cycle_use - (uint32)(cycle2 - cycle1);

        cycle1 = cycle2;
    }
}

//硬件休眠函数 休眠s
SECTION_PLT_CODE void hd_sleep_seconds(uint32 seconds)
{
    sleep(seconds);
}

//硬件休眠函数 休眠ms
SECTION_PLT_CODE void hd_sleep_mseconds(uint32 mseconds)
{
    usleep(mseconds * 1000);
}

//硬件休眠函数 休眠us
SECTION_PLT_CODE void hd_sleep_useconds(uint32 useconds)
{
    usleep(useconds);
}

XUartPs Uart_Ps; /* The instance of the UART Driver */
#define UART0_DEVICE_ID                  XPAR_XUARTPS_0_DEVICE_ID   //UART0

//UART0初始化
SECTION_PLT_CODE int hd_Uart0PsInit(void)
{
    int Status;
    XUartPs_Config *Config;

/*
* Initialize the UART driver so that it's ready to use
* Look up the configuration in the config table and then initialize it.
*/
    Config = XUartPs_LookupConfig(UART0_DEVICE_ID);
    if (NULL == Config)
    {
        return XST_FAILURE;
    }


    Status = XUartPs_CfgInitialize(&Uart_Ps, Config, Config->BaseAddress);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }


    XUartPs_SetBaudRate(&Uart_Ps, 115200);


    XUartPs_EnableUart(&Uart_Ps);


    return XST_SUCCESS;
}

#define  SHELL_UART_IN_ADDR     STDIN_BASEADDRESS
#define  SHELL_UART_OUT_ADDR    STDOUT_BASEADDRESS

//从串口读取一个字符
SECTION_PLT_CODE char SHELL_inchar(void)
{
    u32 RecievedByte;
    /* Wait until there is data */
    if (XUartPs_IsReceiveData(SHELL_UART_IN_ADDR))
    {
        RecievedByte = XUartPs_ReadReg(SHELL_UART_IN_ADDR, XUARTPS_FIFO_OFFSET);
        /* Return the byte received */
        return (u8)RecievedByte;
    }

    return 0;
}

//往串口打印一个字符
SECTION_PLT_CODE void SHELL_outchar(char c)
{
    XUartPs_SendByte(SHELL_UART_OUT_ADDR, c);
}

XGpioPs Gpio;
#define GPIO_DEVICE_ID  XPAR_XGPIOPS_0_DEVICE_ID

/**
 * @Function: hd_gpio_initial
 * @Description:  GPIO初始化
 * @author zhongwei (2019/12/13 10:20:40)
 * 
 * @param void 
 * 
*/
SECTION_PLT_CODE void hd_gpio_initial(void)
{
    XGpioPs_Config *ConfigPtr;
    ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
    XGpioPs_CfgInitialize(&Gpio, ConfigPtr, ConfigPtr->BaseAddr);
    /** 将几个内置的GPIO初始化 **/
    //GPIO_OUT_STALL CPU喂狗的脉冲信号
    hd_gpio_SetDirectionOut(GPIO_OUT_STALL, 0);

	
	
#if 0
    //GPIO_OUT_RETURN 复归
    hd_gpio_SetDirectionOut(GPIO_OUT_RETURN, 0);
    /*
        总线型开入配置
        包括4个地址线和8个数据线
    */
    //数据线置为输入
    for (int i=0;i<GPIO_IN_BUS_DATA_CNT;i++)
    {
        hd_gpio_SetDirectionIn(GPIO_IN_BUS_DATA0 + i);
    }
    //地址线置为输出
    for (int i=0;i<GPIO_IN_BUS_ADDR_CNT;i++)
    {
        hd_gpio_SetDirectionOut(GPIO_IN_BUS_ADDR0+i, 0);
    }
#endif
}

//设置GPIO方向为输出，并置Data初值
SECTION_PLT_CODE void hd_gpio_SetDirectionOut(u32 io, u32 Data)
{
    XGpioPs_WritePin(&Gpio, io, Data);
    XGpioPs_SetDirectionPin(&Gpio, io, 1);
    XGpioPs_SetOutputEnablePin(&Gpio, io, 1);
    XGpioPs_WritePin(&Gpio, io, Data);
}

//设置GPIO为输入，并返回输入的值
SECTION_PLT_CODE u32 hd_gpio_SetDirectionIn(u32 io)
{
    XGpioPs_SetDirectionPin(&Gpio, io, 0);
    //XGpioPs_SetOutputEnablePin(&Gpio, io, 0);
    return XGpioPs_ReadPin(&Gpio, io);
}

//设置开出
SECTION_PLT_CODE void hd_gpio_SetOut(u32 io, u32 Data)
{
    XGpioPs_WritePin(&Gpio, io, Data);
}

//读取开入
SECTION_PLT_CODE u32 hd_gpio_ReadIn(u32 io)
{
    return XGpioPs_ReadPin(&Gpio, io);
}

//刷新闭锁继电器
SECTION_PLT_CODE void hd_do_stall_flush(void)
{
    static uint8 last = 0;
    XGpioPs_WritePin(&Gpio, GPIO_OUT_STALL, !last);
    last = !last;
}

/**
 * @Author: zhongwei
 * @Date: 2019-11-20 10:40:59
 * @description: 硬件BSP初始化
 * @param {type} 
 * @return: 
 */
SECTION_PLT_CODE void hd_bsp_initial(void)
{
    PRINTF_L1("hd_bsp_initial cpu main id = 0x%x\n\r", hd_get_current_cpu_main_id());
    //GPIO初始化
    hd_gpio_initial();

    /*
        FPGA相关初始化
    */
    //AXI初始化
    axi_hp_sys_init();
    //注册FPGA报文处理回调函数
    {
        extern void fpga_msg_callback(uint8 *msg, uint32 msgLen, uint8 msgType);
        regist_axi_hp_msg_callback(fpga_msg_callback);
    }
}

//打印输出BSP信息
SECTION_PLT_CODE void Print_libdrv(void)
{
    c64_t ch;
    PRINTF("BSP Version : %s\n\r", get_version_libdrv());
    TFullDateTime fdt = get_buildtime_libdrv();
    PRINTF("Build Time  : %s\n\r", str_GetFullDateTimeWithoutMsString(&fdt, ch));
    PRINTF("CPU CLOCK   : %u\n\r", hd_get_cpu_clk_freq());
    PRINTF("TIMER CLOCK : %u\n\r", hd_get_timer_clk_freq());
    PRINTF("cpu num     : %u\n\r", hd_get_current_cpu_num());
    PRINTF("main id     : 0x%x\n\r", hd_get_current_cpu_main_id());
    PRINTF("Platform Info: %d\n\r", XGetPlatform_Info());
}

//打印输出GPIO信息
SECTION_PLT_CODE void Print_gpio(void)
{
    PRINTF("MaxPinNum = %d MaxBankNum=%d\n\r", Gpio.MaxPinNum, Gpio.MaxBanks);
    for (u32 pin=0;pin < Gpio.MaxPinNum;pin++)
    {
        u8 Bank;
        u8 PinNumber;
        XGpioPs_GetBankPin((u8)pin, &Bank, &PinNumber); //读取Bank和bank上的PinNumber
        u32 dir = XGpioPs_GetDirectionPin(&Gpio, pin);
        u32 out_en = XGpioPs_GetOutputEnablePin(&Gpio, pin);
        u32 data = XGpioPs_ReadPin(&Gpio, pin);

        PRINTF("  Pin[%d] bank=%d pin_num=%d dir=%d out=%d data=%d\n\r", pin, Bank, PinNumber, dir, out_en, data);
    }
}

