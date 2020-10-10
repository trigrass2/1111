/*
 * @Author: zhongwei
 * @Date: 2019/12/23 13:47:07
 * @Description: 异常存储数据定义
 * @File: plt_exception.h
 *
*/
#ifndef SRC_H_PUBLIC_PLT_EXCEPTION_H_
#define SRC_H_PUBLIC_PLT_EXCEPTION_H_

/*
	保存最大的中断栈空间长度（uint32长度）
*/
#define MAX_SAVE_IRQ_STACK_CNT  		64
/* 
	保存最大的用户栈空间长度 
*/
#define MAX_SAVE_USR_STACK_CNT  		512

//保存异常描述字符串的最大长度
#define MAX_EXCEPTION_STRING_LENGTH 	1024

/**
 * @Function: TCPU_REGISTER
 * @Description:  寄存器定义
 * @author zhongwei (2019/12/23 15:05:50)
 * 未分组寄存器r0-r7为所有模式共用，共8个。
 * 分组寄存器中r8-r12，快速中断模式有自己的一组寄存器，其他模式共用，所以有10个。
 * 分组寄存器中r13，r14，除了用户模式和系统模式共用外，其他模式各一组，所以共有2*7 - 2 = 12个。
 * r15和CPSR共用，共2个；SPSR除了用户模式和系统模式没有外，其他模式各一个，共5个。 
 *  
 * R：Register；寄存器
 * PC：Program Counter；程序计数器
 * CPSR：Current Program Status Register；当前程序状态寄存器
 * SPSR：Saved Program Status Register；保存的程序状态寄存器
 * SP：Stack Pointer；数据栈指针
 * LR：Link Register；连接寄存器
 * SB：静态基址寄存器
 * SL：数据栈限制指针
 * FP：帧指针
 * IP：Intra-Procedure-call Scratch Register；内部程序调用暂存寄存器
 *  
 * r10 - SL 
 * r11 - fp 
 * r12 - ip 
 * r13 - sp 
 * r14 - lr 
 * r15 - pc 
*/
typedef struct{
	uint32 r[16];   //r0-r15
	uint32 cpsr;
	uint32 spsr;
}TCPU_REGISTER;

typedef struct{
	/*
		出现异常时寄存器状态保存
	*/
	uint32 abort_addr;
	uint32 cur_dfsr;
	uint32 cur_dfar;
	uint32 cur_ifsr;
	uint32 cur_ifar;
	uint32 cur_cpsr;
	uint32 cur_spsr;
	uint32 cur_sp;
	uint32 cur_lr;
	uint32 cur_pc;

	uint32 lr_usr;
	uint32 sp_usr_begin;
	uint32 sp_usr_count;
	uint32 sp_usr[MAX_SAVE_USR_STACK_CNT];  	//usr模式下堆栈保存

	uint32 lr_abt;
	uint32 sp_abt_begin;
	uint32 sp_abt_count;
	uint32 sp_abt[MAX_SAVE_IRQ_STACK_CNT];  	//abt模式下堆栈保存

	uint32 lr_irq;
	uint32 sp_irq_begin;
	uint32 sp_irq_count;
	uint32 sp_irq[MAX_SAVE_IRQ_STACK_CNT];  	//isr模式下堆栈保存

	char szException[MAX_EXCEPTION_STRING_LENGTH+1];
}TEXCEPTION_CPU_STRUCT;

/*
	堆栈空间定义
*/
typedef struct{
	uint32 heap;
	uint32 heap_end;
	uint32 stack_end;
	uint32 stack;
	uint32 irq_stack_end;
	uint32 irq_stack;
	uint32 supervisor_stack_end;
	uint32 supervisor_stack;
	uint32 abort_stack_end;
	uint32 abort_stack;
	uint32 fiq_stack_end;
	uint32 fiq_stack;
	uint32 undef_stack_end;
	uint32 undef_stack;

	uint32 vector_table;					//中断向量表
}TSTACK_DEFINE;

#ifdef _PROT_UNIT_

void exp_OnException(const char * szException, uint32 abort_addr);

uint32 _GetSysModeSp(void); //获取系统模式的sp
uint32 _GetIrqModeSp(void); //获取IRQ模式的so
uint32 _GetSysModeLr(void); //获取系统模式的lr
uint32 _GetIrqModeLr(void); //获取IRQ模式的lr

void exp_SaveUsrSP(uint32 sp);
void exp_SaveAbtSP(uint32 sp);
void exp_SaveUndSP(uint32 sp);
void exp_SaveIrqSP(uint32 sp);
int32 exp_get_stack_used(uint32 begin/*大*/, uint32 end/*小*/);
void Print_StackInfo(void);

#endif

#endif /* SRC_H_PUBLIC_PLT_EXCEPTION_H_ */
