/*
 * @Author: zhongwei
 * @Date: 2019/12/23 15:17:32
 * @Description: 异常下堆栈处理
 * @File: plt_exception.c
 *
*/

#include "plt_include.h"
#include "plt_exception.h"

SECTION_PLT_DATA TEXCEPTION_CPU_STRUCT g_CpuException;
SECTION_PLT_DATA TSTACK_DEFINE g_StackDef;

extern int _heap;
extern int _heap_end;
extern int _stack_end;
extern int __stack;
extern int _irq_stack_end;
extern int __irq_stack;
extern int _supervisor_stack_end;
extern int __supervisor_stack;
extern int _abort_stack_end;
extern int __abort_stack;
extern int _fiq_stack_end;
extern int __fiq_stack;
extern int _undef_stack_end;
extern int __undef_stack;

extern int _vector_table;

extern void MyDataAbortHandler();

SECTION_PLT_CODE void PowerOn_InitException(void)
{
    E_MEMSET(&g_CpuException,0,sizeof(g_CpuException));
    E_MEMSET(&g_StackDef,0,sizeof(g_StackDef));

    //读取stack的值
    g_StackDef.heap = (uint32)&_heap;
    g_StackDef.heap_end = (uint32)&_heap_end;
    g_StackDef.stack_end = (uint32)&_stack_end;
    g_StackDef.stack = (uint32)&__stack;
    g_StackDef.irq_stack_end = (uint32)&_irq_stack_end;
    g_StackDef.irq_stack = (uint32)&__irq_stack;
    g_StackDef.supervisor_stack_end = (uint32)&_supervisor_stack_end;
    g_StackDef.supervisor_stack = (uint32)&__supervisor_stack;
    g_StackDef.abort_stack_end = (uint32)&_abort_stack_end;
    g_StackDef.abort_stack = (uint32)&__abort_stack;
    g_StackDef.fiq_stack_end = (uint32)&_fiq_stack_end;
    g_StackDef.fiq_stack = (uint32)&__fiq_stack;
    g_StackDef.undef_stack_end = (uint32)&_undef_stack_end;
    g_StackDef.undef_stack = (uint32)&__undef_stack;
    g_StackDef.vector_table = (uint32)&_vector_table;

}

/**
 * @Function: exp_OnException
 * @Description: 出现异常时调用 
 * @author zhongwei (2019/12/24 10:24:10)
 * 
 * @param szException 字符串的异常原因描述
 *        abort_addr 异常地址
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE void exp_OnException(const char * szException, uint32 abort_addr)
{
    /*
        首先尽量保存当前CPU状态寄存器
    */
    u32 dfsr, dfar;
    u32 ifsr, ifar;
    u32 cur_core = hd_get_current_cpu_num();
    u32 cpsr = hd_get_cpsr();
    u32 spsr = hd_get_spsr();
    u32 cur_sp = 0, cur_lr = 0, cur_pc = 0;

    __asm volatile("MRC p15, 0, %0, c5, c0, 0"
          : "=r"(dfsr)); //DFSR  Data  Fault Status Register
                         //dfsr = mfcp(XREG_CP15_INST_FAULT_STATUS)
    __asm volatile ("MRC p15, 0, %0, c6, c0, 0"
          : "=r"(dfar)); //DFAR  Data  Fault Address Register
    __asm volatile("MRC p15, 0, %0, c5, c0, 1"
          : "=r"(ifsr)); //IFSR   Instruction Fault Status Register
                         //ifsr = mfcp(XREG_CP15_INST_FAULT_STATUS)
    __asm volatile("MRC p15, 0, %0, c6, c0, 1"
          : "=r"(ifar)); //IFAR   Instruction Fault Address Register

    __asm volatile("MOV %0, pc" : "=r"(cur_pc));
    __asm volatile("MOV %0, sp" : "=r"(cur_sp));
    __asm volatile("MOV %0, lr" : "=r"(cur_lr));

    g_CpuException.abort_addr = abort_addr;
    g_CpuException.cur_dfsr = dfsr;
    g_CpuException.cur_dfar = dfar;
    g_CpuException.cur_ifsr = ifsr;
    g_CpuException.cur_ifar = ifar;
    g_CpuException.cur_cpsr = cpsr;
    g_CpuException.cur_spsr = spsr;
    g_CpuException.cur_sp = cur_sp;
    g_CpuException.cur_lr = cur_lr;
    g_CpuException.cur_pc = cur_pc;


    E_SPRINTF(g_CpuException.szException, "[%s] cpu=%ld abort_addr=0x%x dfsr=0x%lx dfar=0x%lx ifsr=0x%lx ifar=0x%lx cpsr=0x%lx spsr=0x%lx sp=0x%lx lr=0x%lx pc=0x%lx",
                                            szException, cur_core, abort_addr,
                                            dfsr, dfar, ifsr, ifar, cpsr, spsr, cur_sp, cur_lr, cur_pc);


    //判断当前工作模式
    switch (cpsr & 0x1F)
    {
    case CPU_MODE_USR:
    case CPU_MODE_SYS:
        {
            uint32 sp_irq;
            exp_SaveUsrSP(cur_sp);
            sp_irq = _GetIrqModeSp();
            exp_SaveIrqSP(sp_irq);
            g_CpuException.lr_abt = 0;
            g_CpuException.lr_usr = cur_lr;
            g_CpuException.lr_irq = _GetIrqModeLr();
        }
        break;
    case CPU_MODE_IRQ:
    case CPU_MODE_FIQ:
        {
            uint32 sp_sys;
            exp_SaveIrqSP(cur_sp);
            sp_sys = _GetSysModeSp();
            exp_SaveUsrSP(sp_sys);
            g_CpuException.lr_irq = cur_lr;
            g_CpuException.lr_abt = 0;
            g_CpuException.lr_usr = _GetSysModeLr();
        }
        break;
    case CPU_MODE_ABT:
        {
            uint32 sp_sys;
            uint32 sp_irq;
            exp_SaveAbtSP(cur_sp);
            sp_sys = _GetSysModeSp();
            sp_irq = _GetIrqModeSp();
            exp_SaveUsrSP(sp_sys);
            exp_SaveIrqSP(sp_irq);
            g_CpuException.lr_abt = cur_lr;
            g_CpuException.lr_usr = _GetSysModeLr();
            g_CpuException.lr_irq = _GetIrqModeLr();
            
        }
        break;
    case CPU_MODE_UND:
        {
            uint32 sp_sys;
            uint32 sp_irq;
            exp_SaveUndSP(cur_sp);
            sp_sys = _GetSysModeSp();
            sp_irq = _GetIrqModeSp();
            exp_SaveUsrSP(sp_sys);
            exp_SaveIrqSP(sp_irq);

            g_CpuException.lr_abt = cur_lr;
            g_CpuException.lr_usr = _GetSysModeLr();
            g_CpuException.lr_irq = _GetIrqModeLr();
        }
        break;
    default:
        {
            uint32 sp_sys;
            uint32 sp_irq;
            sp_sys = _GetSysModeSp();
            sp_irq = _GetIrqModeSp();
            exp_SaveUsrSP(sp_sys);
            exp_SaveIrqSP(sp_irq);

            g_CpuException.lr_abt = 0;
            g_CpuException.lr_usr = _GetSysModeLr();
            g_CpuException.lr_irq = _GetIrqModeLr();
        }
        break;
    }

    PRINTF(g_CpuException.szException);
    PRINTF("\n\r");
}

/**
 * @Function: exp_SaveUsrSP
 * @Description: 保存用户栈空间 
 * @author zhongwei (2019/12/23 15:45:00)
 * 
 * @param void 
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE void exp_SaveUsrSP(uint32 sp)
{
    int i;
    const uint32 * ptr = (const uint32 *)sp;
    for (i=0;i<MAX_SAVE_USR_STACK_CNT;i++)
    {
        g_CpuException.sp_usr[i] = *ptr++;
        if ((uint32)ptr >= g_StackDef.stack)
        {
            break;
        }
    }

    g_CpuException.sp_usr_begin = sp;
    g_CpuException.sp_usr_count = i;
}

SECTION_PLT_CODE void exp_SaveAbtSP(uint32 sp)
{
    int i;
    const uint32 * ptr = (const uint32 *)sp;
    for (i=0;i<MAX_SAVE_IRQ_STACK_CNT;i++)
    {
        g_CpuException.sp_abt[i] = *ptr++;
        if ((uint32)ptr >= g_StackDef.abort_stack)
        {
            break;
        }
    }

    g_CpuException.sp_abt_begin = sp;
    g_CpuException.sp_abt_count = i;
}

SECTION_PLT_CODE void exp_SaveUndSP(uint32 sp)
{
    int i;
    const uint32 * ptr = (const uint32 *)sp;
    for (i=0;i<MAX_SAVE_IRQ_STACK_CNT;i++)
    {
        g_CpuException.sp_abt[i] = *ptr++;
        if ((uint32)ptr >= g_StackDef.undef_stack)
        {
            break;
        }
    }

    g_CpuException.sp_abt_begin = sp;
    g_CpuException.sp_abt_count = i;
}


SECTION_PLT_CODE void exp_SaveIrqSP(uint32 sp)
{
    int i;
    const uint32 * ptr = (const uint32 *)sp;
    for (i=0;i<MAX_SAVE_IRQ_STACK_CNT;i++)
    {
        g_CpuException.sp_irq[i] = *ptr++;
        if ((uint32)ptr >= g_StackDef.irq_stack)
        {
            break;
        }
    }

    g_CpuException.sp_irq_begin = sp;
    g_CpuException.sp_irq_count = i;
}



/**
 * @Function: exp_get_stack_info
 * @Description:  堆栈检查，计算最大堆栈占用
 * @author zhongwei (2019/12/23 17:45:52)
 * 
 * @param begin     栈顶，高地址
 * @param end       栈底，底地址
 * @param pUsed 
 * 
 * @return SECTION_PLT_CODE void 
 *  
 *         <---- stack       栈顶，高地址
 *  
 *  
 *         <---- stack_end   栈底，底地址
 *  
*/
SECTION_PLT_CODE int32 exp_get_stack_used(uint32 begin/*大*/, uint32 end/*小*/)
{
    int32 used = 0;
    ASSERT_L2(begin > end);
    if (begin > end)
    {
        const uint32 * ptr_begin = (const uint32 *)begin;
        const uint32 * ptr = (const uint32 *)end;
        while ((uint32)ptr < begin)
        {
            ptr++;

            if ((*ptr) != 0)
            {
                break;
            }
        }

        used =  ptr_begin - ptr;
    }

    return used;
}

/**
 * @Function: Print_StackInfo
 * @Description:  
 * @author zhongwei (2019/12/30 18:51:16)
 * 
 * @return SECTION_PLT_CODE void 
 *  
 *  
 * 由于ld中配置为NOLOAD，因此该操作无法正确执行 
 *  
*/
SECTION_PLT_CODE void Print_StackInfo(void)
{
    const char * szFormat = "%-24s : 0x%08x\n\r";
    PRINTF(szFormat, "vector table", g_StackDef.vector_table);
    PRINTF(szFormat, "heap", g_StackDef.heap);
    PRINTF(szFormat, "heap end", g_StackDef.heap_end);
    PRINTF(szFormat, "stack", g_StackDef.stack);
    PRINTF(szFormat, "stack end", g_StackDef.stack_end);
    PRINTF(szFormat, "stack used", exp_get_stack_used(g_StackDef.stack, g_StackDef.stack_end));
    PRINTF(szFormat, "stack irq", g_StackDef.irq_stack);
    PRINTF(szFormat, "stack irq end", g_StackDef.irq_stack_end);
    PRINTF(szFormat, "stack irq used", exp_get_stack_used(g_StackDef.irq_stack, g_StackDef.irq_stack_end));
    PRINTF(szFormat, "stack svc", g_StackDef.supervisor_stack);
    PRINTF(szFormat, "stack svc end", g_StackDef.supervisor_stack_end);
    PRINTF(szFormat, "stack svc used", exp_get_stack_used(g_StackDef.supervisor_stack, g_StackDef.supervisor_stack_end));
    PRINTF(szFormat, "stack abt", g_StackDef.abort_stack);
    PRINTF(szFormat, "stack abt end", g_StackDef.abort_stack_end);
    PRINTF(szFormat, "stack abt used", exp_get_stack_used(g_StackDef.abort_stack, g_StackDef.abort_stack_end));
    PRINTF(szFormat, "stack fiq", g_StackDef.fiq_stack);
    PRINTF(szFormat, "stack fiq end", g_StackDef.fiq_stack_end);
    PRINTF(szFormat, "stack fiq used", exp_get_stack_used(g_StackDef.fiq_stack, g_StackDef.fiq_stack_end));
    PRINTF(szFormat, "stack udf", g_StackDef.undef_stack);
    PRINTF(szFormat, "stack udf end", g_StackDef.undef_stack_end);
    PRINTF(szFormat, "stack udf used", exp_get_stack_used(g_StackDef.undef_stack, g_StackDef.undef_stack_end));
}

