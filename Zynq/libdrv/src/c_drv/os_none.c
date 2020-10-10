#include "plt_inc_c.h"
#include "os_interface.h"
#include "os_none.h"
#include "hd_driver.h"

/** 操作系统变量 **/
SECTION_PLT_DATA volatile OS_TIME OSTime        = 0;                //系统时间
SECTION_PLT_DATA INT8U  OS_Event_Count      = 0;
SECTION_PLT_DATA volatile INT8U OS_Event[OS_MAX_SIGNAL_COUNT];
SECTION_PLT_DATA volatile INT32U OS_ISRReg   = 0;                   //系统中断标志，如果在中断中，则置1，否则置0
SECTION_PLT_DATA volatile INT8U OSIntNesting = 0;
SECTION_PLT_DATA volatile OS_IVG OSCurIntIVG  = IRQ_NONE;           //操作系统当前中断IRQ_ID,-1表示不在中断中
SECTION_PLT_DATA volatile INT32U OS_IdleCnt = 0;                    //用于计算CPU占用率
 
SECTION_PLT_DATA INT32U OS_IntCount[num_interrupt_kind];            //中断计数
SECTION_PLT_DATA INT8U OSRunning=0;

//性能分析
typedef struct{
    INT64U into_cycle;      //进入中断时的cycle
    INT64U used_cycle;      //在当前任务中消耗的时间
}TTSK_ANALYZE;

typedef struct{
    TTSK_ANALYZE task[num_interrupt_kind];
}TOS_ANALYZE;

SECTION_PLT_DATA TOS_ANALYZE os_analyze;

//模拟操作系统初始化的函数
SECTION_PLT_CODE void OSInit(void)
{
    int i;
    OSTime = 0;
    OS_Event_Count = 0;
    OS_ISRReg = 0;
    OSIntNesting = 0;
    OSRunning = 0;
    OSCurIntIVG =IRQ_NONE;

    E_MEMSET(&os_analyze, 0, sizeof(os_analyze));

    for (i=0;i<OS_MAX_SIGNAL_COUNT;i++)
    {
        OS_Event[i] = 0;
    }

    //中断计数清零
    for (i=0;i<num_interrupt_kind;i++)
    {
        OS_IntCount[i] = 0;
    }
}

SECTION_PLT_CODE void OSStart(void)
{
    OSRunning = 1;
}

SECTION_PLT_CODE INT8U OSIsRuning(void)
{
    return OSRunning;
}

//定时器1中断处理函数 模拟TICK时钟
SECTION_PLT_CODE void OSTimeTick(void)
{
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    OSTime++;
    CPU_CRITICAL_EXIT();
}

//! 获取当前OS时间
SECTION_PLT_CODE OS_TIME OSTimeGet (void)
{
    OS_TIME os_tm;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    os_tm = OSTime;
    CPU_CRITICAL_EXIT();
    return (os_tm);
}

//设置OS时间
SECTION_PLT_CODE void  OSTimeSet (OS_TIME ticks)
{
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    OSTime = ticks;
    CPU_CRITICAL_EXIT();
}

// 模拟ucosOSTimeDly的函数
SECTION_PLT_CODE void OSTimeDly(INT16U ticks)
{
    if (OSIsRuning())
    {
        OS_TIME os_t = OSTimeGet();
        OS_TIME os_diff = 0;
        do
        {
            os_diff = OSTimeGet() - os_t;
        } while (os_diff < ticks);
    }
    else
    {
        hd_sleep_cycles(hd_get_timer_clk_freq() / os_get_ticks_per_sec() * ticks);
    }
}

//进中断前处理
SECTION_PLT_CODE OS_IVG OSIntEnter(OS_IVG ivg)
{
    CPU_SR_ALLOC();
    OS_IVG     old_ivg;
    INT64U cur_cycle;

    OS_IntCount[ivg]++;                 //进入一次中断

    CPU_CRITICAL_ENTER();

    OS_ISRReg  |= (1<<ivg);
    if (OSIntNesting < 255u) {
        OSIntNesting++;
    }
    old_ivg = OSCurIntIVG;      //保存老的IVG
    OSCurIntIVG = ivg;          //切换新的IVG

    cur_cycle = hd_get_cycles64();
    if (old_ivg >=0 && old_ivg < num_interrupt_kind)
    {
        os_analyze.task[old_ivg].used_cycle += (cur_cycle - os_analyze.task[old_ivg].into_cycle);   //计算在old_ivg中消耗的时间
        os_analyze.task[old_ivg].into_cycle = cur_cycle;
    }
    if (ivg >= 0 && ivg < num_interrupt_kind)
    {
        os_analyze.task[ivg].into_cycle = cur_cycle;        //记录进入当前ivg中断的时刻
    }

//  if (IsDebugMessage(DEBUG_MESSAGE_IRQ_NESTING))
//  {
//      if (old_ivg != IRQ_NONE)
//      {
//          const TIRQ_CONFIG *pIrqIvg = hd_GetIrqConfig(ivg);
//          const TIRQ_CONFIG *pIrqIvgOld = hd_GetIrqConfig(old_ivg);
//          if(pIrqIvg==NULL || pIrqIvgOld == NULL)
//          {
//              LOGMSG("OSIntEnter irq error, ivg=%d old_ivg=%d\n\r", ivg, old_ivg,0,0,0,0);
//          }
//          else if (pIrqIvg->priority >= pIrqIvgOld->priority)
//          {
//              LOGMSG("OSIntEnter nest error, task %s(%d) nest %s(%d)\n\r",
//                     (int)pIrqIvg->szName, pIrqIvg->priority,
//                     (int)pIrqIvgOld->szName, pIrqIvgOld->priority,0,0);
//          }
//      }
//  }

    CPU_CRITICAL_EXIT();

    return old_ivg;             //返回老的IVG
}

//进中断后处理
SECTION_PLT_CODE void OSIntExit(OS_IVG ivg, OS_IVG old_ivg)
{
    INT64U cur_cycle;

    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    OS_ISRReg  &= ~(1<<ivg);
    if (OSIntNesting > 0) {
        OSIntNesting--;
    }
    OSCurIntIVG = old_ivg;

    cur_cycle = hd_get_cycles64();
    if (ivg >=0 && ivg < num_interrupt_kind)
    {
        os_analyze.task[ivg].used_cycle += (cur_cycle -  os_analyze.task[ivg].into_cycle);
        os_analyze.task[ivg].into_cycle = cur_cycle;
    }
    if (old_ivg >=0 && old_ivg < num_interrupt_kind)
    {
        os_analyze.task[old_ivg].into_cycle = cur_cycle;
    }

    CPU_CRITICAL_EXIT();
}

//计算中断进入次数
SECTION_PLT_CODE INT32U OSGetIntCount(OS_IVG ivg)
{
    return OS_IntCount[ivg];
}

//获取当前所在中断优先级
SECTION_PLT_CODE OS_IVG OSGetCurIVG(void)
{
    return OSCurIntIVG;
}

//获取OS中断标志寄存器
SECTION_PLT_CODE INT32U OSIntRegGet(void)
{
    INT32U reg;

    reg = OS_ISRReg;
    return (reg);
}

//获取某一中断正在处理标志
SECTION_PLT_CODE INT8U OSIsIntEnter(OS_IVG ivg)         //某一中断是否在处理
{
    INT32U reg = OSIntRegGet();
    return ((reg >> ivg) & 0x01);
}

/************************************************************************/
//  信号量
//      提供三个接口
//      OS_Signal_Create()      --      申请信号量
//      OS_Signal_Pend()        --      通知信号量
//      OS_Signal_Post()        --      释放信号量
//      OS_Signal_Accept()      --      访问信号量 (返回值 0:不可访问 1:可访问)
//  使用方法：
//      在初始化时调用OS_Signal_Create，创建一个信号量
//      在低优先级中断或主循环中调用 OS_Signal_Pend和OS_Signal_Post，对特定资源进行访问
//      在高优先级中断中，调用OS_Signal_Accept，获取信号量状态，以便判断后面的任务是否执行
/************************************************************************/
SECTION_PLT_CODE INT16S OS_Signal_Create(void)
{
    INT16S index = OS_Event_Count;
    if(OS_Event_Count >= OS_MAX_SIGNAL_COUNT)
    {
        return -1;              //返回-1，表示无信号请求
    }
    OS_Event_Count++;           //分配一个信号量
    return index;   
}

SECTION_PLT_CODE void OS_Signal_Pend(INT16S signal)
{
    if(signal >=0 && signal < OS_Event_Count)
    {
        OS_Event[signal] = 1;                   //信号获取
    }
}

SECTION_PLT_CODE void OS_Signal_Post(INT16S signal)
{
    if(signal >=0 && signal < OS_Event_Count)
    {
        OS_Event[signal] = 0;                   //信号释放
    }   
}

SECTION_PLT_CODE INT8U OS_Signal_Accept(INT16S signal)
{
    if(signal >=0 && signal < OS_Event_Count)
    {
        return !OS_Event[signal];
    }
    else
    {
        return 1;
    }
}

//清除全部任务分析数据
SECTION_PLT_CODE void OS_ClearUsage(void)
{
    for (int i=0;i<num_interrupt_kind;i++)
    {
        os_analyze.task[i].used_cycle = 0;
    }
}

//获取任务使用量信息
SECTION_PLT_CODE INT64U OS_GetIVGUsage(OS_IVG ivg)
{
    if (ivg >=0 && ivg < num_interrupt_kind)
    {
        INT64U usage;

        CPU_SR_ALLOC();

        CPU_CRITICAL_ENTER();
        usage = os_analyze.task[ivg].used_cycle;
        CPU_CRITICAL_EXIT();

        return usage;
    }

    return 0;
}

