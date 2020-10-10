/*
 * @Author: zhongwei
 * @Date: 2019-11-21 09:14:01
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 09:15:47
 * @Description: 
 * @FilePath: \ZynqBM\src\project\plt_task.c
 */
#include "plt_include.h"
#include "plt_task.h"

extern void tsk_CalcCpuUsage(const TTimeRun * pTimeRun);
extern void OnEnter_tskADSimple(void);
extern void OnLeave_tskADSimple(void);
extern void OnEnter_tskOSTick(void);
extern void OnLeave_tskOSTick(void);
extern void OnEnter_tskFast(void);
extern void OnLeave_tskFast(void);
extern void OnEnter_tskSlow(void);
extern void OnLeave_tskSlow(void);
extern void OnEnter_tskComm(void);
extern void OnLeave_tskComm(void);
extern void OnEnter_tskUser(void);
extern void OnLeave_tskUser(void);
extern void OnEnter_tskIdel(void);
extern void OnLeave_tskIdel(void);
extern void OnEnter_tsk500us(void);
extern void OnLeave_tsk500us(void);

extern void shell_handle_intask(void);
extern void fpga_msg_handle_inInt(const TTimeRun * pTimeRun);

/************************************************************************/
//  各任务处理
//
/************************************************************************/

//慢速任务
SECTION_PLT_CODE void tskTime500us(const TTimeRun * pTimeRun)
{
    OnEnter_tsk500us();
    if (OSIsRuning())
    {
    	scantime500us();
    }
    OnLeave_tsk500us();
}

/*#define max_cnt  256
DWORD cnt = 0,t1 = 0, t2 = 0,t3 = 0;
DWORD tickcnt[max_cnt];
DWORD tickcnt1[max_cnt];
DWORD tickcnt2[max_cnt];
t3 = hd_get_cycles();
tickcnt[cnt] = t2 - t1;
tickcnt1[cnt] = t3 - t2;
tickcnt2[cnt] = t3 - t1;
cnt = (cnt + 1) & (max_cnt - 1);*/

DWORD sendtick = 0,sendtick1 = 0,sendtick2 = 0;
BYTE buf[12] = {00 ,01,02,0x3,04};

//AD采样中断
extern DWORD nSamBuf[10][64]; //最大64通道
extern WORD nSamCnt;
void adInt(DWORD *nSamTmp);

SECTION_PLT_CODE void tskADSimple(const TTimeRun * pTimeRun)
{
	int i;
	
    OnEnter_tskADSimple();

    ClockRun(pTimeRun);     //时钟走时处理

    fpga_msg_handle_inInt(pTimeRun);        //FPGA数据接收处理

	//发送报文
	//sendtick = hd_get_cycles();
	//Uart_Send( 0 ,buf, 12);
	//sendtick1 = hd_get_cycles();
	for(i = 0; i < nSamCnt; i++)
    {
        adInt(nSamBuf[i]);
    }
	nSamCnt = 0;
	//sendtick2 = hd_get_cycles();
    //hd_sleep_cycles(pTimeRun->cycles / 10*9);

    //触发快速中断
    hd_SGI_cur_cpu_rasie(SGI_INTR_ID_FAST);     

    OnLeave_tskADSimple();
}

//系统tick任务
SECTION_PLT_CODE void tskOSTick(const TTimeRun * pTimeRun)
{
    OnEnter_tskOSTick();

    //刷新闭锁继电器
    hd_do_stall_flush();

    //软中断任务切换处理
    {
        /*
            SGI_INTR_ID_SLOW
            SGI_INTR_ID_COMMU
            SGI_INTR_ID_USER
        */

        int irq;
        for (irq = (SGI_INTR_ID_FAST+1); irq < SGI_INTR_COUNT ; irq++)
        {
            TIRQ_STATUS *pIrqStatus = hd_GetIrqStatus(irq);
            const TIRQ_CONFIG *pIrqConfig = pIrqStatus->p_irq_config;
            if (IsTrue(pIrqStatus->b_enabled))
            {
                uint32 cnt = 0;
                uint32 triger_cnt = 0;
                if (pIrqConfig->triger_type == SGI_TRIGER_TIME)
                {
                    cnt = pTimeRun->cycles;
                    triger_cnt = pIrqStatus->cur_triger_cnt;
                }
                else if (pIrqConfig->triger_type == SGI_TRIGER_TICK)
                {
                    cnt = pTimeRun->os_tick;
                    triger_cnt = pIrqStatus->cur_triger_cnt;
                }
                else if (pIrqConfig->triger_type == SGI_TRIGER_FREQ)
                {
                    cnt = pTimeRun->cycles;
                    triger_cnt = hd_get_timer_clk_freq() / pIrqStatus->cur_triger_cnt;
                }
                else if (pIrqConfig->triger_type == SGI_TRIGER_NONE)
                {
                    continue;
                }
                pIrqStatus->switch_cnt += cnt;
                if (pIrqStatus->switch_cnt >= triger_cnt)
                {
                    pIrqStatus->switch_cnt = 0;
                    hd_SGI_cur_cpu_rasie(irq);      //定时到，触发软中断
                }
            }
        }
    }

	extern void tRunCnt(void);
	tRunCnt();
	
    OnLeave_tskOSTick();
}

//快速任务 0.833ms,放差动保护
SECTION_PLT_CODE void tskFast(const TTimeRun * pTimeRun)
{
    OnEnter_tskFast();

    if (OSIsRuning())
    {
    	
    }

    OnLeave_tskFast();
}

//慢速任务4ms 常规保护
SECTION_PLT_CODE void tskSlow(const TTimeRun * pTimeRun)
{
    OnEnter_tskSlow();
    if (OSIsRuning())
    {
		extern void protect(void);
		protect();
    }
    OnLeave_tskSlow();
}

/*测试中断tick
DWORD t1 = 0;
DWORD t2[128];
DWORD count = 0;
DWORD t3 = 0;
DWORD t4[128];
extern DWORD gUsCnt;
t2[count] = gUsCnt - t1;
t4[count] = OSTimeGet() - t3;
t3 = OSTimeGet();
t1 = gUsCnt;
count = (count + 1) & (128-1);*/
	
//通讯任务 10ms ，共享通讯
SECTION_PLT_CODE void tskCommu(const TTimeRun * pTimeRun)
{
    OnEnter_tskComm();
	
    //BM<->Linux内部通讯处理
    {
        extern void inner_comm_handle_intask(const TTimeRun * pTimeRun);
        inner_comm_handle_intask(pTimeRun);        
    }
	
    OnLeave_tskComm();
}

//用户任务 10ms 执行自己的task
SECTION_PLT_CODE void tskUser(const TTimeRun * pTimeRun)
{
    OnEnter_tskUser();
	
    if (OSIsRuning())
    {	
    }
    //logmsg信息输出处理
    logmsg_tsk_handle();

    OnLeave_tskUser();
}

//打印任务耗时信息，简化
SECTION_PLT_CODE void Print_TaskUsageInfo(void)
{
    int irq_id;
    for (irq_id = 0; irq_id < num_interrupt_kind; irq_id++)
    {
        const TIRQ_STATUS *pStatus = &irq_status[irq_id];
        const TIRQ_CONFIG *pConfig = pStatus->p_irq_config;
        if (IsTrue(pStatus->b_enabled))
        {
            LOGMSG("[IRQ%02d %10s] os_time=%11d tick_cnt=%11u usage=%2d%% max_usage=%2d%%\n\r",
                   pConfig->irq_id,
                   (int)pConfig->szName,
                   (uint32)pStatus->last_os_time,
                   (uint32)pStatus->tick_cnt,
                   (int)pStatus->usage_per,
                   (int)pStatus->usage_per_max);
        }
    }
}

//打印任务信息，详细
SECTION_PLT_CODE void Print_TaskDetail(int irq_id)
{
    if (irq_id >=0 && irq_id<num_interrupt_kind)
    {
        const TIRQ_STATUS *pStatus = &irq_status[irq_id];
        const TIRQ_CONFIG *pConfig = pStatus->p_irq_config;
        if (IsTrue(pStatus->b_enabled))
        {
            //任务名称
            PRINTF("[IRQ%02d] Task name:%s\n\r", pConfig->irq_id, pConfig->szName);
            //优先级
            PRINTF("  Priority : %d\n\r", pConfig->priority);
            //入口地址
            PRINTF("  Handler : 0x%x\n\r", (uint32)pConfig->Handler);
            //任务超时时间
            PRINTF("  switch_overtime : %u\n\r", pConfig->switch_overtime);
            //触发
            PRINTF("  triger_type : %d\n\r", pConfig->triger_type);
            PRINTF("  triger_cnt : %u\n\r", pConfig->triger_cnt);
            //当前
            PRINTF("  cur_triger_cnt : %u\n\r", pStatus->cur_triger_cnt);
            PRINTF("  last_enter_cycle_cnt : %u\n\r", pStatus->last_enter_cycle_cnt);
            PRINTF("  last_leave_cycle_cnt : %u\n\r", pStatus->last_leave_cycle_cnt);
            PRINTF("  max_cycle_cnt : %u\n\r", pStatus->max_cycle_cnt);
            PRINTF("  last_os_time : %llu\n\r", pStatus->last_os_time);
            PRINTF("  tick_cnt : %llu\n\r", pStatus->tick_cnt);
            PRINTF("  chk_overtimer : %u\n\r", pStatus->chk_overtimer);
            PRINTF("  switch_cnt : %u\n\r", pStatus->switch_cnt);
            PRINTF("  usage : %.3f%%\n\r", pStatus->usage_per);
            PRINTF("  max_usage : %.3f%%\n\r", pStatus->usage_per_max);
        }
        else
        {
            PRINTF("irq_id=%d is disabled!\n\r", irq_id);
        }
    }
    else
    {
        PRINTF("unknow irq_id=%d!\n\r", irq_id);
    }
}


//打印全部任务详细信息
SECTION_PLT_CODE void Print_AllTaskDetail(void)
{
    int irq_id;
    for (irq_id = 0; irq_id < num_interrupt_kind; irq_id++)
    {
        const TIRQ_STATUS *pStatus = &irq_status[irq_id];
        if (IsTrue(pStatus->b_enabled))
        {
            Print_TaskDetail(irq_id);
        }
    }
}

//触发任务
SECTION_PLT_CODE void othertsk_run(const TTimeRun * pTimeRun)
{
    static uint32 us = 0;
	
    us += pTimeRun->us;

    if (us >= 500)         //每500us秒判断一次
    {
        us = 0;
		extern void DoBmUrgency(void);
		DoBmUrgency();
    }
	
	extern void tRun(void);	
	tRun();
	
}

//空闲任务, 在while循环中调用
SECTION_PLT_CODE void tskIdle(const TTimeRun * pTimeRun)
{
    OnEnter_tskIdel();

	othertsk_run(pTimeRun);
    tsk_CalcCpuUsage(pTimeRun);

    OnLeave_tskIdel();
}

/************************************************************************/
//  任务进入和退出的钩子函数
//
/************************************************************************/

SECTION_PLT_CODE void OnEnter_tsk500us(void)
{

}

SECTION_PLT_CODE void OnLeave_tsk500us(void)
{

}


SECTION_PLT_CODE void OnEnter_tskADSimple(void)
{

}

SECTION_PLT_CODE void OnLeave_tskADSimple(void)
{

}

SECTION_PLT_CODE void OnEnter_tskOSTick(void)
{

}

SECTION_PLT_CODE void OnLeave_tskOSTick(void)
{

}

SECTION_PLT_CODE void OnEnter_tskFast(void)
{

}

SECTION_PLT_CODE void OnLeave_tskFast(void)
{

}

SECTION_PLT_CODE void OnEnter_tskSlow(void)
{

}

SECTION_PLT_CODE void OnLeave_tskSlow(void)
{

}

SECTION_PLT_CODE void OnEnter_tskComm(void)
{

}

SECTION_PLT_CODE void OnLeave_tskComm(void)
{

}

SECTION_PLT_CODE void OnEnter_tskUser(void)
{

}

SECTION_PLT_CODE void OnLeave_tskUser(void)
{

}

SECTION_PLT_CODE void OnEnter_tskIdel(void)
{

}

SECTION_PLT_CODE void OnLeave_tskIdel(void)
{

}

//CPU占用率计算
SECTION_PLT_CODE void tsk_CalcCpuUsage(const TTimeRun * pTimeRun)
{
    static BOOL bInit = FALSE;
    static uint32 ms = 0;

    ms += pTimeRun->ms;
    if (!bInit)
    {
        OS_ClearUsage();
        bInit = PLT_TRUE;
    }
    if (ms >= 5000)         //每5秒计算一次
    {
        uint64 total = (uint64)ms * hd_get_timer_clk_freq() / 1000;

        int irq_id;
        for (irq_id = 0; irq_id < num_interrupt_kind; irq_id++)
        {
            TIRQ_STATUS *pStatus = &irq_status[irq_id];
//            const TIRQ_CONFIG *pConfig = pStatus->p_irq_config;

            if (IsTrue(pStatus->b_enabled))
            {
                pStatus->usage_per = 100.0 * OS_GetIVGUsage(irq_id) / total;
                if (pStatus->usage_per > pStatus->usage_per_max)
                {
                    pStatus->usage_per_max = pStatus->usage_per;
                }
            }
        }

        OS_ClearUsage();
        ms = 0;
    }
}

//任务入口注册 需要在PowerOn_Initial_Hardware后调用
SECTION_PLT_CODE void PowerOn_RegisterTsk(void)
{
    hd_ResigterIrqTsk(SGI_INTR_ID_FAST, tskFast);
    hd_ResigterIrqTsk(SGI_INTR_ID_SLOW, tskSlow);
    hd_ResigterIrqTsk(SGI_INTR_ID_COMMU, tskCommu);
    hd_ResigterIrqTsk(SGI_INTR_ID_USER, tskUser);
    hd_ResigterIrqTsk(TIMER_TTC_INTR, tskOSTick);
    hd_ResigterIrqTsk(FPGA_0_INTR, tskADSimple);
    hd_ResigterIrqTsk(IDLE_INTR_ID, tskIdle);
	
    hd_ResigterIrqTsk(TIMER_IRPT_INTR, tskTime500us); //新加cjl 2020年5月12日 14:01:59
}

