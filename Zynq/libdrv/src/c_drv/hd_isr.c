/*
 * @Author: zhongwei
 * @Date: 2019-11-21 09:17:14
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 17:43:06
 * @Description: 
 * @FilePath: hd_isr.c
 */
#include "plt_inc_c.h"
#include "libdrv.h"

#define INTC_DEVICE_ID XPAR_PS7_SCUGIC_0_DEVICE_ID //只有0，一个中断控制器
SECTION_PLT_DATA XScuGic InterruptController;      /* Instance of the Interrupt Controller */

SECTION_PLT_DATA u32 DataAbortAddr=0;       /* Address of instruction causing data abort */
SECTION_PLT_DATA u32 PrefetchAbortAddr=0;   /* Address of instruction causing prefetch abort */
SECTION_PLT_DATA u32 UndefinedExceptionAddr=0;   /* Address of instruction causing Undefined
                                                     exception */

extern void xPrivateTimerIntrHandler(void *CallBackRef);
extern void xTtcTimerHandler(void *CallBackRef);
extern void xSgiIntrFastHandler(void *CallBackRef);
extern void xSgiIntrSlowHandler(void *CallBackRef);
extern void xSgiIntrCommuHandler(void *CallBackRef);
extern void xSgiIntrUserHandler(void *CallBackRef);
extern void xFpgaIntHandler(void *CallBackRef);
extern void xIdelTaskHandler(void *CallBackRef);

//IRQ配置
SECTION_PLT_CONST static const TIRQ_CONFIG irq_config[]={
    //irq_id                         szName              priority    Handler                     switch_overtime                 triger_type         triger_cnt
    {SGI_INTR_ID_FAST   /* 0*/,      "fastTsk",          4,          xSgiIntrFastHandler,        os_get_ticks_per_sec(),         SGI_TRIGER_TICK,    0/*采样中断中触发*/  },
    {SGI_INTR_ID_SLOW   /* 1*/,      "slowTsk",          8,          xSgiIntrSlowHandler,        os_get_ticks_per_sec()*2,       SGI_TRIGER_TIME,    (hd_get_timer_clk_freq() / (1000 / 4))/*tick中断中触发 4ms*/   },
    {SGI_INTR_ID_COMMU  /* 2*/,      "commuTsk",         12,         xSgiIntrCommuHandler,       os_get_ticks_per_sec()*20,      SGI_TRIGER_TIME,    (hd_get_timer_clk_freq() / (1000 / 10))/*tick中断中触发 10ms*/ },
    {SGI_INTR_ID_USER   /* 3*/,      "userTsk",          16,         xSgiIntrUserHandler,        os_get_ticks_per_sec()*20,      SGI_TRIGER_TIME,    (hd_get_timer_clk_freq() / (1000 / 10))/*tick中断中触发 10ms*/ },
    {TIMER_IRPT_INTR    /*29*/,      "timerInt",         1,          xPrivateTimerIntrHandler,   os_get_ticks_per_sec(),         SGI_TRIGER_TIME,    (hd_get_timer_clk_freq() / 2000) },   //500us用来遥信防抖等，之前用来采样0.833
    {TIMER_TTC_INTR     /*43*/,      "ttcInt",           2,          xTtcTimerHandler,           os_get_ticks_per_sec(),         SGI_TRIGER_FREQ,    os_get_ticks_per_sec()},
    {FPGA_0_INTR        /*61*/,      "fpgaInt",          1,          xFpgaIntHandler,            os_get_ticks_per_sec(),         SGI_TRIGER_FPGA,    0/*FPGA定时触发*/ },
    {IDLE_INTR_ID       /*95*/,      "idleTsk",          31,         xIdelTaskHandler,           os_get_ticks_per_sec()*180,     SGI_TRIGER_NONE,    0/*循环调用*/},
};

//IRQ状态
SECTION_PLT_DATA TIRQ_STATUS irq_status[num_interrupt_kind];

//注册任务处理函数 需要在hd_SetupInterruptSystem后调用
SECTION_PLT_CODE void hd_ResigterIrqTsk(int irq_id, tsk_func func)
{
    TIRQ_STATUS * pIrq = hd_GetIrqStatus(irq_id);
    if (pIrq != NULL)
    {
        ASSERT_L1(pIrq->tsk_exec == NULL);  //不允许重复注册
        pIrq->tsk_exec = pIrq->tsk_exec_bak = func;
        PRINTF_L1("hd_ResigterIrqTsk irq=%d func=0x%x\n\r", irq_id, func);
    }
    else
    {
        ASSERT_L3(PLT_FALSE);
    }
}

//获取当前中断状态
SECTION_PLT_CODE TIRQ_STATUS * hd_GetIrqStatus(int irq_id)
{
    if (irq_id >=0 && irq_id < num_interrupt_kind)
    {
        return &irq_status[irq_id];
    }
    else
    {
        return NULL;
    }
}

//获取中断配置
const TIRQ_CONFIG * hd_GetIrqConfig(int irq_id)
{
    if (irq_id >=0 && irq_id<SGI_INTR_COUNT)
    {
        return &irq_config[irq_id];
    }
    else
    {
        int id;
        for (id=SGI_INTR_COUNT;id<(sizeof(irq_config)/sizeof(TIRQ_CONFIG));id++)
        {
            if (irq_id == irq_config[id].irq_id)
            {
                return &irq_config[id];
            }
        }
    }

    ASSERT_L1(FALSE);
    return NULL;
}

/************************************************************************/
//   函数调用回溯
//
/************************************************************************/


/************************************************************************/
//          异常处理中断 - 开始
//
//          xDataAbortHandler - 数据异常
//          xPrefetchAbortHandler - Prefetch异常
//          xUndefinedExceptionHandler - 未定义的指令异常
//          
//          关于A9异常，查看文档 DDI0388G_cortex_a9_r3p0_trm 4.2.19
//                              arm_cortexa9_trm_100511_0401_10_en 4.3.15
//          Evernote笔记《ARM A9 错误寄存器》
/************************************************************************/

void xDataAbortHandler(void *CallBackRef)
{
    exp_OnException("Data Abort", DataAbortAddr);

    while (1)
    {
        __asm("NOP");
    }
}

void xPrefetchAbortHandler(void *CallBackRef)
{
    exp_OnException("Prefetch Abort", PrefetchAbortAddr);

    while (1)
    {
        __asm("NOP");
    }
}

void xUndefinedExceptionHandler(void *CallBackRef)
{
    exp_OnException("Undefined", UndefinedExceptionAddr);

    while (1)
    {
        __asm("NOP");
    }
}

/************************************************************************/
//          异常处理中断 - 结束
//
/************************************************************************/

/**
 * @Function: hd_SetupInterruptSystem
 * @Description: 
 *             初始化中断，必须在所有中断注册之前调用
 * @author zhongwei (2019/11/25 10:49:36)
 * 
 * @return SECTION_PLT_CODE STATUS 
 * 
*/
SECTION_PLT_CODE STATUS hd_SetupInterruptSystem(void)
{
    XScuGic *GicInstancePtr = &InterruptController;
    XScuGic_Config *IntcConfig;
    int32 Status;
    u32 CPUID;
    CPUID = XScuGic_GetCpuID();

    //中断状态初始化
    {
        int32 idx;
        E_MEMSET(irq_status, 0, sizeof(irq_status));
        for (idx=0; idx<(sizeof(irq_config)/sizeof(TIRQ_CONFIG)); idx++ )
        {
            const TIRQ_CONFIG * pIrqConfig = &irq_config[idx];
            TIRQ_STATUS *pIrqStatus = &irq_status[pIrqConfig->irq_id];
            ASSERT_L1(pIrqConfig->irq_id >= 0 && pIrqConfig->irq_id < num_interrupt_kind);
            pIrqStatus->p_irq_config = pIrqConfig;
            pIrqStatus->b_chk_disable = PLT_FALSE;
            pIrqStatus->cur_triger_cnt = pIrqConfig->triger_cnt;

            pIrqStatus->usage_per = pIrqStatus->usage_per_max = 0.0;
        }
    }

    IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);

    if (NULL == IntcConfig)
    {
        PRINTF_L1("SetupInterruptSystem CPUID=%lu LookupConfig fial.\n\r", CPUID);
        return XST_FAILURE;
    }

    PRINTF_L1("SetupInterruptSystem CPUID=%lu, CpuBaseAddress=0x%x\n\r", CPUID, (uint32)IntcConfig->CpuBaseAddress);

    Status = XScuGic_CfgInitialize(GicInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS)
    {
        PRINTF_L1("SetupInterruptSystem CPUID=%lu CfgInitialize fial.\n\r", CPUID);
        return XST_FAILURE;
    }
    /*
        * Initialize the  exception table
    */
    Xil_ExceptionInit();
    /*
        * Register the interrupt controller handler with the exception table
    */
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                 GicInstancePtr);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
                                (Xil_ExceptionHandler)xDataAbortHandler,
                                  GicInstancePtr);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT,
                                (Xil_ExceptionHandler)xPrefetchAbortHandler,
                                  GicInstancePtr);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNDEFINED_INT,
                                (Xil_ExceptionHandler)xUndefinedExceptionHandler,
                                  GicInstancePtr);
    /*
        * Enable non-critical exceptions
    */
    {//add cjl,防止第二次进不来中断,清数据待验证
        int32 idx;
		for (idx=0; idx<(sizeof(irq_config)/sizeof(TIRQ_CONFIG)); idx++ )
	    {
            const TIRQ_CONFIG * pIrqConfig = &irq_config[idx];
			if((pIrqConfig->irq_id != TIMER_TTC_INTR) && (pIrqConfig->irq_id != FPGA_0_INTR))
				XScuGic_CPUWriteReg(GicInstancePtr, XSCUGIC_EOI_OFFSET, pIrqConfig->irq_id | 0x400);
			else
				XScuGic_CPUWriteReg(GicInstancePtr, XSCUGIC_EOI_OFFSET, pIrqConfig->irq_id);
		}
    }
	
    Xil_ExceptionEnable();
    return XST_SUCCESS;
}

/************************************************************************/
//          私有定时器中断注册接口 - 开始
//   每个Zynq Core都有一个Private timer，一般用来做采样中断定时器
//      XPAR_SCUTIMER_INTR = 29
/************************************************************************/

SECTION_PLT_DATA static XScuTimer xPtivateTimer;   //私有定时器
#define TIMER_DEVICE_ID XPAR_XSCUTIMER_0_DEVICE_ID //Private Timer Device ID（因为只有一个，所有始终是0）

/**
 * @Author: zhongwei
 * @Date: 2019-11-21 10:07:40
 * @description: 配置私有定时器中断
 * @param 
 *      period - 定时器周期， 以APP_TIMER_CLK_FREQ为基准
 *      Handler - 中断入口函数
 *      priority - 中断优先级
 * @return: 
 */
SECTION_PLT_CODE void hd_SetScuPrivateTimer(uint32 period, Xil_InterruptHandler Handler, uint8 priority)
{
    XScuGic *GicInstancePtr = &InterruptController;
    XScuTimer_Config *TMRConfigPtr; //timer config
    int DeviceID = TIMER_DEVICE_ID;
    TMRConfigPtr = XScuTimer_LookupConfig(DeviceID);
    XScuTimer_CfgInitialize(&xPtivateTimer, TMRConfigPtr, TMRConfigPtr->BaseAddr);
    XScuTimer_SelfTest(&xPtivateTimer);

    XScuTimer_LoadTimer(&xPtivateTimer, period);
    XScuTimer_SetPrescaler(&xPtivateTimer, 0); ///must,zhan added
    //自动装载
    XScuTimer_EnableAutoReload(&xPtivateTimer);
    //set up the timer interrupt
    XScuGic_Connect(GicInstancePtr, TIMER_IRPT_INTR, (Xil_ExceptionHandler)Handler, (void *)&xPtivateTimer);
    XScuGic_SetPriorityTriggerType(GicInstancePtr, TIMER_IRPT_INTR, (priority << 3), 0x01/*高电平触发*/);
    //enable the interrupt for the Timer at GIC
    XScuGic_Enable(GicInstancePtr, TIMER_IRPT_INTR);
    //enable interrupt on the timer
    XScuTimer_EnableInterrupt(&xPtivateTimer);
    //启动定时器
    XScuTimer_Start(&xPtivateTimer);
}

/**
 * @Author: zhongwei
 * @Date: 2019-11-21 10:28:05
 * @description: 私有定时器初始化
 *              必须在hd_SetupInterruptSystem后才能调用
 * @param {type} 
 *      period - 定时器周期
 * @return: 
 */
SECTION_PLT_CODE void hd_InitIsr_PrivateTimer(void)
{
    const int irq_id = TIMER_IRPT_INTR;
    TIRQ_STATUS *pStatus = &irq_status[irq_id];
    const TIRQ_CONFIG *pConfig = pStatus->p_irq_config;
    uint32 period=0;

    ASSERT_L1(pStatus->p_irq_config != NULL);

    if (pConfig->triger_type == SGI_TRIGER_TIME)
    {
        period = pStatus->cur_triger_cnt;
    }
    else if (pConfig->triger_type == SGI_TRIGER_FREQ)
    {
        period = hd_get_timer_clk_freq() / pStatus->cur_triger_cnt;
    }

    ASSERT_L1(period > 0);

    ASSERT_L1(pStatus->tsk_exec == pStatus->tsk_exec_bak && pStatus->tsk_exec != NULL);

    pStatus->b_enabled = PLT_TRUE;  //中断使能
    pStatus->last_enter_cycle_cnt = hd_get_cycles();
    pStatus->last_os_time = OSTimeGet();
    hd_SetScuPrivateTimer(period, pConfig->Handler, pConfig->priority);

    PRINTF_L1("hd_InitIsr_PrivateTimer name=%s period=%d priority=%d ok\n\r", pConfig->szName, period, pConfig->priority);
}

/************************************************************************/
//          私有定时器中断 - 结束
//
/************************************************************************/


/************************************************************************/
//  FPGA中断 - 开始
//  是0.833中断
/************************************************************************/
/**
 * @Function: hd_SetFpgaInt0
 * @Description: 配置FPGA中断 
 * @author zhongwei (2020/3/30 14:39:40)
 * 
 * @return SECTION_PLT_CODE STATUS 
 * 
*/
SECTION_PLT_CODE STATUS hd_SetFpgaInt0(int intr, Xil_InterruptHandler Handler, uint8 priority)
{
    int status;
    XScuGic *GicInstancePtr = &InterruptController;

    status = XScuGic_Connect(GicInstancePtr,intr,Handler,(void *)0);

    if(status != XST_SUCCESS)
    {
        PRINTF_L1("hd_SetFpgaInt0 XScuGic_Connect Int=%d fail.\n\r", intr);
        return XST_FAILURE;
    }
  
    // Set interrupt type of SW1 to rising edge
    XScuGic_SetPriorityTriggerType(GicInstancePtr, intr, (priority << 3), 0x03/*上升沿触发*/);

    // Enable SW1~SW3 interrupts in the controller
    XScuGic_Enable(GicInstancePtr, intr);

    // set irq map to current cpu
    XScuGic_InterruptMaptoCpu(GicInstancePtr, XScuGic_GetCpuID(), intr);

    return XST_SUCCESS;
}

//初始化FGPA中断
SECTION_PLT_CODE void hd_InitIsr_Fpga(void)
{
    const int irq_id = FPGA_0_INTR;
    TIRQ_STATUS *pStatus = &irq_status[irq_id];
    const TIRQ_CONFIG *pConfig = pStatus->p_irq_config;

    ASSERT_L1(pStatus->p_irq_config != NULL);

    //必须是FPGA触发
    ASSERT_L1(pConfig->triger_type == SGI_TRIGER_FPGA);

    ASSERT_L1(pStatus->tsk_exec == pStatus->tsk_exec_bak && pStatus->tsk_exec != NULL);

    pStatus->b_enabled = PLT_TRUE;  //中断使能
    pStatus->last_enter_cycle_cnt = hd_get_cycles();
    pStatus->last_os_time = OSTimeGet();
    if(hd_SetFpgaInt0(irq_id, pConfig->Handler, pConfig->priority) == XST_SUCCESS)
    {
        PRINTF_L1("hd_InitIsr_Fpga name=%s priority=%d ok\n\r", pConfig->szName, pConfig->priority);
    }
    else
    {
        PRINTF_L1("hd_InitIsr_Fpga name=%s priority=%d fail!\n\r", pConfig->szName, pConfig->priority);
    }
}

/************************************************************************/
//  FPGA中断 - 结束
//
/************************************************************************/

/************************************************************************/
//          软件中断注册接口 - 开始
//     0 SGI_INTR_ID_FAST - 快速中断
//     1 SGI_INTR_ID_SLOW - 慢速中断
//     2 SGI_INTR_ID_COMMU - 通讯中断
//     3 SGI_INTR_ID_USER - 用户中断
/************************************************************************/


/**
 * @Author: zhongwei
 * @Date: 2019-11-21 13:28:16
 * @description: 注册软中断
 * @param {type} 
 *      intr - 软中断ID 0~15
 *      Handler - 中断入口函数
 *      priority - 中断优先级
 * @return: XST_FAILURE/XST_SUCCESS
 */
SECTION_PLT_CODE STATUS hd_setSGIInt(int intr, Xil_InterruptHandler Handler, uint8 priority)
{
    STATUS Status;
    uint8 prio;
    uint8 trigger;
    XScuGic *GicInstancePtr = &InterruptController;

    Status = XScuGic_Connect(GicInstancePtr, intr, (Xil_ExceptionHandler)Handler, 0);
    if (Status != XST_SUCCESS)
    {
        PRINTF_L1("hd_setSGIInt XScuGic_Connect SGI=%d fail.\n\r", intr);
        return XST_FAILURE;
    }
    XScuGic_Enable(GicInstancePtr, intr);

    XScuGic_GetPriorityTriggerType(GicInstancePtr, intr, &prio, &trigger);
    prio = priority << 3;
    XScuGic_SetPriorityTriggerType(GicInstancePtr, intr, prio, trigger);
    XScuGic_GetPriorityTriggerType(GicInstancePtr, intr, &prio, &trigger);

    PRINTF_L1("hd_setSGIInt SGI=%d Prio=%d, trigger=%d\n\r", intr, prio, trigger);

    //XScuGic_InterruptMaptoCpu(GicInstancePtr, XScuGic_GetCpuID(), intr);

    return XST_SUCCESS;
}

SECTION_PLT_CODE STATUS hd_setInterInt(int intr, Xil_InterruptHandler Handler, uint8 priority,void *CallBackRef)
{
    STATUS Status;
    uint8 prio;
    XScuGic *GicInstancePtr = &InterruptController;

    Status = XScuGic_Connect(GicInstancePtr, intr, (Xil_ExceptionHandler)Handler, CallBackRef);
    if (Status != XST_SUCCESS)
    {
        PRINTF_L1("hd_setSGIInt XScuGic_Connect SGI=%d fail.\n\r", intr);
        return XST_FAILURE;
    }
    XScuGic_Enable(GicInstancePtr, intr);	
    prio = priority << 3;
    XScuGic_SetPriorityTriggerType(GicInstancePtr, intr, prio, 0x03);

    PRINTF_L1("hd_setSGIInt SGI=%d Prio=%d\n\r", intr, prio);

    XScuGic_InterruptMaptoCpu(GicInstancePtr, XScuGic_GetCpuID(), intr);

    return XST_SUCCESS;
}

/**
 * @Function: hd_InitIsr_SgiInt
 * @Description: 注册软中断 
 * @author zhongwei (2019/11/25 10:38:01)
 * 
 * @param void 
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE void hd_InitIsr_SgiInt(void)
{
    int irq_id;
    for (irq_id = SGI_INTR_ID_FAST; irq_id < SGI_INTR_COUNT; irq_id++)
    {
        TIRQ_STATUS *pStatus = &irq_status[irq_id];
        const TIRQ_CONFIG *pConfig = pStatus->p_irq_config;

        if (pConfig->triger_type == SGI_TRIGER_NONE)
        {
            continue;
        }

        ASSERT_L1(pConfig != NULL);
        ASSERT_L1(pStatus->tsk_exec == pStatus->tsk_exec_bak && pStatus->tsk_exec != NULL);

        pStatus->b_enabled = PLT_TRUE;  //中断使能
        pStatus->last_enter_cycle_cnt = hd_get_cycles();
        pStatus->last_os_time = OSTimeGet();

        if(hd_setSGIInt(irq_id, pConfig->Handler, pConfig->priority) != XST_SUCCESS)
        {
            PRINTF_L1("hd_InitIsr_SgiInt name=%s priority=%d fail.\n\r", pConfig->szName, pConfig->priority);
        }
        else
        {
            PRINTF_L1("hd_InitIsr_SgiInt name=%s priority=%d ok\n\r", pConfig->szName, pConfig->priority);
        }
    }
}

/**
 * @Author: zhongwei
 * @Date: 2019-11-21 13:33:00
 * @description: 在当前CPU触发软中断
 * @param {type} 
 *      intr - 软中断id
 * @return: XST_FAILURE/XST_SUCCESS
 */
SECTION_PLT_CODE STATUS hd_SGI_cur_cpu_rasie(int intr)
{
    STATUS Status;
    XScuGic *GicInstancePtr = &InterruptController;
    Status = XScuGic_SoftwareIntr(GicInstancePtr, intr, 0x1 << hd_get_current_cpu_num());
    return Status;
}

/************************************************************************/
//          软件中断注册接口 - 结束
/************************************************************************/

/************************************************************************/
//          TTC定时器中断注册接口 - 开始
//      TTC定时器用来做系统tick计数器（目前只能选TTC0）
//      Zynq提供2组各3个TTC定时器
// 
//  TTC中断 IRQ ID
//      TTC0:42,43,44  选用43
//      TTC1:69,70,71
/************************************************************************/
#define TTC_TICK_DEVICE_ID  XPAR_XTTCPS_1_DEVICE_ID     //0,1,2 ttc的设备ID
static XTtcPs TtcPsInst;    /* Three timer counters */

/**
 * @Function: hd_SetTtcTimer
 * @Description: 初始化TTC定时器 
 * @author zhongwei (2019/11/25 11:11:02)
 * 
 * @param IntrID  IRQ id
 * @param freq  触发频率，每秒次数
 * @param Handler 中断入口
 * @param priority 中断优先级
 * 
 * @return STATUS 
 * 
*/
STATUS hd_SetTtcTimer(int IntrID, uint32 freq, Xil_InterruptHandler Handler, u8 priority)
{
    XScuGic *GicInstancePtr = &InterruptController;
    XTtcPs_Config *Config;
    XTtcPs *pTimer;
    u16 Interval;
    u8 Prescaler;
    STATUS Status;
    int DeviceID = IntrID - XPAR_XTTCPS_0_INTR;

    if (DeviceID < 0 || DeviceID >=3)       //TTC0的0,1,2
    {
        PRINTF_L1("hd_SetTtcTimer IntrID=%d DeviceID=%d fail.\n\r", IntrID, DeviceID);
        return XST_FAILURE;
    }

    pTimer = &TtcPsInst;

    Config = XTtcPs_LookupConfig(DeviceID);
    if (NULL == Config)
    {
        PRINTF_L1("hd_SetTtcTimer XTtcPs_LookupConfig DeviceID=%d fail.\n\r", DeviceID);
        return XST_FAILURE;
    }

    Status = XTtcPs_CfgInitialize(pTimer, Config, Config->BaseAddress);
    if (Status != XST_SUCCESS) {
        if(Status == XST_DEVICE_IS_STARTED)
        {
            PRINTF_L1("CPU1: hd_SetTtcTimer timer is started\n\r");
            XTtcPs_Stop(pTimer);
            XTtcPs_CfgInitialize(pTimer, Config, Config->BaseAddress);
        }
    }

    XTtcPs_SetOptions(pTimer, XTTCPS_OPTION_INTERVAL_MODE|XTTCPS_OPTION_WAVE_DISABLE);

    XTtcPs_CalcIntervalFromFreq(pTimer, freq, &Interval, &Prescaler); //freq=1:1s interrupt,TTC时钟为111MHZ

    PRINTF_L1("hd_SetTtcTimer TTC0-%d IntrID %d Interval 0x%x Prescaler 0x%x\n\r",DeviceID, IntrID, Interval, Prescaler);

    XTtcPs_SetInterval(pTimer, Interval);
    XTtcPs_SetPrescaler(pTimer, Prescaler);

    Status = XScuGic_Connect(GicInstancePtr, IntrID, (Xil_ExceptionHandler)Handler, (void *)pTimer);
    if (Status != XST_SUCCESS) 
    {
        PRINTF_L1("hd_SetTtcTimer XScuGic_Connect IntrID=%d fail.\n\r", IntrID)
        return XST_FAILURE;
    }

    ///////
    XScuGic_SetPriorityTriggerType(GicInstancePtr, IntrID, priority<<3, 0x01);
    XScuGic_InterruptMaptoCpu(GicInstancePtr, XScuGic_GetCpuID(), IntrID);
    //LOG("CPU1: CPUID %d ICDIPTR10 0x%x...\n",XScuGic_GetCpuID(), Xil_In32(0xf8f01828));

    XScuGic_Enable(&InterruptController, IntrID);

    XTtcPs_EnableInterrupts(pTimer, XTTCPS_IXR_INTERVAL_MASK);

    XTtcPs_Start(pTimer);

    return Status;
}

/**
 * @Function: hd_InitIsr_TtcTimer
 * @Description: TTC定时器中断初始化 
 * @author zhongwei (2019/11/25 11:54:05)
 * 
 * @param void 
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE void hd_InitIsr_TtcTimer(void)
{
    const int irq_id = TIMER_TTC_INTR;
    TIRQ_STATUS *pStatus = &irq_status[irq_id];
    const TIRQ_CONFIG *pConfig = pStatus->p_irq_config;
    
    ASSERT_L1(pStatus->p_irq_config != NULL);
    
    if (pConfig->triger_type == SGI_TRIGER_FREQ)    //只支持周期
    {
        ASSERT_L1(pStatus->tsk_exec == pStatus->tsk_exec_bak && pStatus->tsk_exec != NULL);

        pStatus->b_enabled = PLT_TRUE;  //中断使能
        pStatus->last_enter_cycle_cnt = hd_get_cycles();
        pStatus->last_os_time = OSTimeGet();

        if(hd_SetTtcTimer(pConfig->irq_id, pStatus->cur_triger_cnt, pConfig->Handler, pConfig->priority) != XST_SUCCESS)
        {
            PRINTF_L1("hd_InitIsr_TtcTimer name=%s freq=%d priority=%d fail.\n\r", pConfig->szName, pStatus->cur_triger_cnt, pConfig->priority);
        }
        else
        {
            PRINTF_L1("hd_InitIsr_TtcTimer name=%s freq=%d priority=%d ok\n\r", pConfig->szName, pStatus->cur_triger_cnt, pConfig->priority);
        }
    }
}

/************************************************************************/
//          TTC定时器中断注册接口 - 结束
/************************************************************************/

/************************************************************************/
//          空闲任务中断注册接口 - 开始
/************************************************************************/
SECTION_PLT_CODE void hd_InitIsr_IdleTsk(void)
{
    const int irq_id = IDLE_INTR_ID;
    TIRQ_STATUS *pStatus = &irq_status[irq_id];
    //const TIRQ_CONFIG *pConfig = pStatus->p_irq_config;

    ASSERT_L1(pStatus->p_irq_config != NULL);
    ASSERT_L1(pStatus->tsk_exec == pStatus->tsk_exec_bak && pStatus->tsk_exec != NULL);
    pStatus->b_enabled = PLT_TRUE;  //中断使能
    pStatus->last_enter_cycle_cnt = hd_get_cycles();
    pStatus->last_os_time = OSTimeGet();
}

/************************************************************************/
//          空闲任务中断注册接口 - 结束
/************************************************************************/

/**
 * @Function: _isrTimeRun
 * @Description:  计算中断走时
 * @author zhongwei (2019/11/25 16:32:10)
 * 
 * @param pIrqStatus IRQ中断
 * @param last_cycle 上次进中断的cycles计数
 * 
 * @return const TTimeRun* 返回走时信息
 * 
*/
const TTimeRun * _isrTimeRun(TIRQ_STATUS * pIrqStatus, uint32 last_cycle, OS_TIME last_os_time)
{
    TTimeRun *time_run = &pIrqStatus->time_run;
    uint32 ms_cycle, us_cycle;
    const uint32 cycles_per_ms = hd_get_timer_clk_freq()*3/1000;    //放大3倍后的每毫秒cycles
    const uint32 cycles_per_us = hd_get_timer_clk_freq()*3/1000000; //放大3倍后的每微秒cycles
    
    ASSERT_L1(pIrqStatus != NULL);

    time_run->os_tick = pIrqStatus->last_os_time/*当前*/ - last_os_time;      //计算系统走时tick
    time_run->cycles = pIrqStatus->last_enter_cycle_cnt - last_cycle;       //cycle走时

    ms_cycle = 3*time_run->cycles + pIrqStatus->ms_reserve;
    us_cycle = 3*time_run->cycles + pIrqStatus->us_reserve;

    //为了减少ms计算精度，放大3倍
    time_run->ms = ms_cycle / cycles_per_ms;
    pIrqStatus->ms_reserve = ms_cycle % cycles_per_ms;

    //为了减少us计算精度，放大3倍
    time_run->us = us_cycle / cycles_per_us;
    pIrqStatus->us_reserve = ms_cycle % cycles_per_us;

    return time_run;
}

/************************************************************************/
//          中断入口函数 - 开始
//
/************************************************************************/
#define IRQ_SR_ALLOC(irq_id)    \
                const OS_IVG cur_ivg = irq_id;                                      \
                TIRQ_STATUS * pIrqStatus=NULL;                                      \
                OS_IVG old_ivg=-1;                                                  \
                uint32 last_cycle;                                                  \
                OS_TIME last_os_time;                                               \
                const TTimeRun * time_run=NULL;

#define BEFORE_INTO_IRQ()   \
                pIrqStatus = hd_GetIrqStatus(cur_ivg);                              \
                old_ivg = OSIntEnter(cur_ivg);                                      \
                ASSERT_L1(pIrqStatus != NULL && IsTrue(pIrqStatus->b_enabled));     \
                last_cycle = pIrqStatus->last_enter_cycle_cnt;                      \
                last_os_time = pIrqStatus->last_os_time;                            \
                pIrqStatus->last_enter_cycle_cnt = hd_get_cycles();                 \
                pIrqStatus->last_os_time = OSTimeGet();                             \
                pIrqStatus->tick_cnt++;                                             \
                pIrqStatus->chk_overtimer=0;                                        \
                time_run = _isrTimeRun(pIrqStatus, last_cycle, last_os_time);

#define AFTER_LEAVE_IRQ()   \
                pIrqStatus->last_leave_cycle_cnt = hd_get_cycles();                 \
                { uint32 cycle_cnt = pIrqStatus->last_leave_cycle_cnt - pIrqStatus->last_enter_cycle_cnt;    \
                if(cycle_cnt > pIrqStatus->max_cycle_cnt) pIrqStatus->max_cycle_cnt=cycle_cnt;}              \
                ASSERT_L1(OSGetCurIVG() == cur_ivg);                                \
                OSIntExit(cur_ivg, old_ivg);

#define TASK_RUN()          \
                ASSERT_L2(pIrqStatus->tsk_exec == pIrqStatus->tsk_exec_bak);  \
                if(pIrqStatus->tsk_exec != NULL){pIrqStatus->tsk_exec(time_run);}

/**
 * @Function: xPrivateTimerIntrHandler
 * @Description:  Scu Private Timer中断回调
 * @author zhongwei (2019/11/25 9:23:04)
 * 
 * @param CallBackRef 
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE void xPrivateTimerIntrHandler(void *CallBackRef)
{
    XScuTimer *TimerInstancePtr = (XScuTimer *)CallBackRef;
    IRQ_SR_ALLOC(TIMER_IRPT_INTR);

    //进入中断前的处理
    BEFORE_INTO_IRQ();

    ASSERT_L1(TimerInstancePtr != NULL);

    XScuTimer_ClearInterruptStatus(TimerInstancePtr); //清除中断标志

    /*
        在允许中断嵌套之前，ARM处于 IRQ模式下（CPSR_c) M[4:0]= 0b10010，堆栈使用的是_irq_stack
        在调用允许中断嵌套后，ARM会切换到系统模式 M[4:0]= 0b11111，其后，堆栈会使用_stack（系统堆栈和用户堆栈是共用的）
    */
    Xil_EnableNestedInterrupts();   //允许中断嵌套（与Xil_DisableNestedInterrupts必须配对使用）

    //tskADSimple(time_run);
    TASK_RUN();

    /*
        禁止中断嵌套后，会切换后IRQ模式，继续使用irq_stack
    */
    Xil_DisableNestedInterrupts();  //禁止中断嵌套

    //退出中断时处理
    AFTER_LEAVE_IRQ();
}

//10ms tick定时器中断
SECTION_PLT_CODE void xTtcTimerHandler(void *CallBackRef)
{
    XTtcPs * pPs = (XTtcPs *)CallBackRef;
    u32 StatusEvent;

    IRQ_SR_ALLOC(TIMER_TTC_INTR);

    //进入中断前的处理
    BEFORE_INTO_IRQ();

    OSTimeTick();           //系统计时器走时

    ASSERT_L1(pPs != NULL);

    StatusEvent = XTtcPs_GetInterruptStatus(pPs);
    XTtcPs_ClearInterruptStatus(pPs, StatusEvent);

    Xil_EnableNestedInterrupts();   //允许中断嵌套（与Xil_DisableNestedInterrupts必须配对使用）

    //tskOSTick(time_run);
    TASK_RUN();
        
    Xil_DisableNestedInterrupts();  //禁止中断嵌套

    //退出中断时处理
    AFTER_LEAVE_IRQ();
}

/**
 * @Function: xSgiIntrFastHandler
 * @Description: 快速中断入口函数，软中断
 * @author zhongwei (2019/11/24 19:02:48)
 * 
 * @param void 
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE void xSgiIntrFastHandler(void *CallBackRef)
{
    IRQ_SR_ALLOC(SGI_INTR_ID_FAST);

    //进入中断前的处理
    BEFORE_INTO_IRQ();

    Xil_EnableNestedInterrupts();   //允许中断嵌套（与Xil_DisableNestedInterrupts必须配对使用）

    //tskFast(time_run);
    TASK_RUN();

    Xil_DisableNestedInterrupts();  //禁止中断嵌套

    //退出中断时处理
    AFTER_LEAVE_IRQ();
}

//软中断
SECTION_PLT_CODE void xSgiIntrSlowHandler(void *CallBackRef)
{
    IRQ_SR_ALLOC(SGI_INTR_ID_SLOW);

    //进入中断前的处理
    BEFORE_INTO_IRQ();

    Xil_EnableNestedInterrupts();   //允许中断嵌套（与Xil_DisableNestedInterrupts必须配对使用）

    //tskSlow(time_run);
    TASK_RUN();

    Xil_DisableNestedInterrupts();  //禁止中断嵌套

    //退出中断时处理
    AFTER_LEAVE_IRQ();
}

//软中断
SECTION_PLT_CODE void xSgiIntrCommuHandler(void *CallBackRef)
{
    IRQ_SR_ALLOC(SGI_INTR_ID_COMMU);

    //进入中断前的处理
    BEFORE_INTO_IRQ();

    Xil_EnableNestedInterrupts();   //允许中断嵌套（与Xil_DisableNestedInterrupts必须配对使用）

    //tskCommu(time_run);
    TASK_RUN();

    Xil_DisableNestedInterrupts();  //禁止中断嵌套

    //退出中断时处理
    AFTER_LEAVE_IRQ();
}

//软中断
SECTION_PLT_CODE void xSgiIntrUserHandler(void *CallBackRef)
{
    IRQ_SR_ALLOC(SGI_INTR_ID_USER);

    //进入中断前的处理
    BEFORE_INTO_IRQ();

    Xil_EnableNestedInterrupts();   //允许中断嵌套（与Xil_DisableNestedInterrupts必须配对使用）

    //tskUser(time_run);
    TASK_RUN();

    Xil_DisableNestedInterrupts();  //禁止中断嵌套

    //退出中断时处理
    AFTER_LEAVE_IRQ();
}

//FPGA触发的AD采样处理中断
SECTION_PLT_CODE void xFpgaIntHandler(void *CallBackRef)
{
    IRQ_SR_ALLOC(FPGA_0_INTR);

    //进入中断前的处理
    BEFORE_INTO_IRQ();

    Xil_EnableNestedInterrupts();   //允许中断嵌套（与Xil_DisableNestedInterrupts必须配对使用）

    //tskADSimple(time_run);
    TASK_RUN();

    Xil_DisableNestedInterrupts();  //禁止中断嵌套

    //退出中断时处理
    AFTER_LEAVE_IRQ();
}

// 非中断，在主循环中调用
SECTION_PLT_CODE void xIdelTaskHandler(void *CallBackRef)
{
    IRQ_SR_ALLOC(IDLE_INTR_ID);

    //进入中断前的处理
    BEFORE_INTO_IRQ();

    //tskIdle(time_run);
    TASK_RUN();

    //退出中断时处理
    AFTER_LEAVE_IRQ();
}



/************************************************************************/
//          中断入口函数 - 结束
//
/************************************************************************/
