/*
 * @Author: zhongwei
 * @Date: 2019-11-21 09:16:56
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 14:04:25
 * @Description: 中断处理接口函数
 * @FilePath: \ZynqBM\src\hardware\hd_isr.h
 */
#ifndef SRC_HARDWARE_PLT_HD_ISR_H_
#define SRC_HARDWARE_PLT_HD_ISR_H_

#include "os_cpu_a.h"

/*
    中断优先级定义，提供32个优先级 0~31, 其中0最高优先级中断
    关于中断优先级的说明见 XScuGic_SetPriorityTriggerType 函数的说明
    以及文档《UG585》 ICDIPR0~23 寄存器配置
    每个中断有8bit的优先级配置寄存器，其中低3bit总是0，高5bit用于配置priority    
*/


//使用的中断ID IRQ_ID
enum E_IRQ_INTR_ID{
    SGI_INTR_ID_FAST = 0,                       //irg=0 快速任务中断
    SGI_INTR_ID_SLOW,                           //irq=1 慢速任务中断
    SGI_INTR_ID_COMMU,                          //irq=2 通讯任务中断
    SGI_INTR_ID_USER,                           //irq=3 用户任务中断
    SGI_INTR_COUNT,                             //使用的软中断个数
    
    TIMER_IRPT_INTR = XPAR_SCUTIMER_INTR,       //irq=29 私有定时器中断  采样中断 由于采样中断由FPGA触发，因此这里不再使用

    TIMER_TTC_INTR = XPAR_XTTCPS_0_INTR + 1,    //irq=43 TTC定时器ttc0-1 tick中断

    FPGA_0_INTR = XPS_FPGA0_INT_ID,             //FPGA 0号中断，用于触发采样处理中断

    IDLE_INTR_ID = num_interrupt_kind - 1,      //irq=95 空闲任务
};

//中断触发方式
enum E_IRQ_TRIGER_TYPE{
    SGI_TRIGER_TIME = 1,                        //时间触发（触发时间需要转换为cycle计数器值）
    SGI_TRIGER_TICK = 2,                        //tick触发，对应PrivateTimer tick数
    SGI_TRIGER_FREQ = 3,                        //触发频率，每秒触发次数
    SGI_TRIGER_FPGA = 4,                        //FPGA触发
    SGI_TRIGER_EXTERNAL = 5,                    //外部中断
    SGI_TRIGER_NONE = 0xff,                     //无需触发（用于空闲任务）
};

//中断配置结构
typedef struct TIRQ_CONFIG{
    int32 irq_id;                       //软中断id 0~95
    char * szName;                      //中断名称
    uint8 priority;                     //中断优先级 0~31
    Xil_InterruptHandler Handler;       //中断入口
    uint32 switch_overtime;             //任务切换超时（tick计数）
    uint8 triger_type;                  //触发方式 - E_IRQ_TRIGER_TYPE
    uint32 triger_cnt;                  //触发计数值
}TIRQ_CONFIG;

typedef void (*tsk_func)(const TTimeRun * pTimeRun);

//中断状态结构
typedef struct TIRQ_STATUS{
    BOOL b_enabled;                     //中断是否使能
    const TIRQ_CONFIG * p_irq_config;   //对应的中断配置
    tsk_func tsk_exec;                  //关联执行任务
    tsk_func tsk_exec_bak;              //重复保存，用于自检
    uint32 cur_triger_cnt;              //当前的触发计数限值(运行中可能会修改)
    uint32 last_enter_cycle_cnt;        //上次进入中断时的cycle计数器值
    uint32 last_leave_cycle_cnt;        //上次退出中断时的cycle计数器值
    uint32 max_cycle_cnt;               //最大中断耗时cycle计数
    OS_TIME last_os_time;               //最后进入中断时的系统时间
    uint64 tick_cnt;                    //每进入一次中断，计数器加一
    BOOL b_chk_disable;                 //是否禁止中断自检
    uint32 chk_overtimer;               //中断自检超时计数器(进入中断时清零，在自检任务中检查)
    //走时处理
    TTimeRun time_run;                  //走时
    int32 ms_reserve;                   //ms走时剩余，放大3倍
    int32 us_reserve;                   //us走时剩余，放大3倍
    //任务切换处理（用于软中断）
    uint32 switch_cnt;                  //切换计数

    fp64 usage_per;                     //CPU占用百分比
    fp64 usage_per_max;                 //最大CPU占用百分比
}TIRQ_STATUS;

extern TIRQ_STATUS irq_status[num_interrupt_kind];

//注册任务处理函数 需要在hd_SetupInterruptSystem后调用
void hd_ResigterIrqTsk(int irq_id, tsk_func func);

//获取中断状态
TIRQ_STATUS * hd_GetIrqStatus(int irq_id);

//获取中断配置
const TIRQ_CONFIG * hd_GetIrqConfig(int irq_id);

//初始化中断
STATUS hd_SetupInterruptSystem(void);

//配置私有定时器中断
void hd_SetScuPrivateTimer(uint32 period, Xil_InterruptHandler Handler, uint8 priority);

//私有定时器初始化
void hd_InitIsr_PrivateTimer(void);

//注册软中断
STATUS hd_setSGIInt(int intr, Xil_InterruptHandler Handler, uint8 priority);

//软中断初始化
void hd_InitIsr_SgiInt(void);

//在当前CPU触发软中断
STATUS hd_SGI_cur_cpu_rasie(int intr);

//初始化TTC定时器 
STATUS hd_SetTtcTimer(int IntrID, uint32 freq, Xil_InterruptHandler Handler, u8 priority);

//TTC定时器中断初始化
void hd_InitIsr_TtcTimer(void);

//初始化FGPA中断
void hd_InitIsr_Fpga(void);

//空闲任务初始化
void hd_InitIsr_IdleTsk(void);

#endif /* SRC_HARDWARE_PLT_HD_ISR_H_ */
