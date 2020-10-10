/*
 * @Author: zhongwei
 * @Date: 2019/12/23 13:47:07
 * @Description: �쳣�洢���ݶ���
 * @File: plt_exception.h
 *
*/
#ifndef SRC_H_PUBLIC_PLT_EXCEPTION_H_
#define SRC_H_PUBLIC_PLT_EXCEPTION_H_

/*
	���������ж�ջ�ռ䳤�ȣ�uint32���ȣ�
*/
#define MAX_SAVE_IRQ_STACK_CNT  		64
/* 
	���������û�ջ�ռ䳤�� 
*/
#define MAX_SAVE_USR_STACK_CNT  		512

//�����쳣�����ַ�������󳤶�
#define MAX_EXCEPTION_STRING_LENGTH 	1024

/**
 * @Function: TCPU_REGISTER
 * @Description:  �Ĵ�������
 * @author zhongwei (2019/12/23 15:05:50)
 * δ����Ĵ���r0-r7Ϊ����ģʽ���ã���8����
 * ����Ĵ�����r8-r12�������ж�ģʽ���Լ���һ��Ĵ���������ģʽ���ã�������10����
 * ����Ĵ�����r13��r14�������û�ģʽ��ϵͳģʽ�����⣬����ģʽ��һ�飬���Թ���2*7 - 2 = 12����
 * r15��CPSR���ã���2����SPSR�����û�ģʽ��ϵͳģʽû���⣬����ģʽ��һ������5���� 
 *  
 * R��Register���Ĵ���
 * PC��Program Counter�����������
 * CPSR��Current Program Status Register����ǰ����״̬�Ĵ���
 * SPSR��Saved Program Status Register������ĳ���״̬�Ĵ���
 * SP��Stack Pointer������ջָ��
 * LR��Link Register�����ӼĴ���
 * SB����̬��ַ�Ĵ���
 * SL������ջ����ָ��
 * FP��ָ֡��
 * IP��Intra-Procedure-call Scratch Register���ڲ���������ݴ�Ĵ���
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
		�����쳣ʱ�Ĵ���״̬����
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
	uint32 sp_usr[MAX_SAVE_USR_STACK_CNT];  	//usrģʽ�¶�ջ����

	uint32 lr_abt;
	uint32 sp_abt_begin;
	uint32 sp_abt_count;
	uint32 sp_abt[MAX_SAVE_IRQ_STACK_CNT];  	//abtģʽ�¶�ջ����

	uint32 lr_irq;
	uint32 sp_irq_begin;
	uint32 sp_irq_count;
	uint32 sp_irq[MAX_SAVE_IRQ_STACK_CNT];  	//isrģʽ�¶�ջ����

	char szException[MAX_EXCEPTION_STRING_LENGTH+1];
}TEXCEPTION_CPU_STRUCT;

/*
	��ջ�ռ䶨��
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

	uint32 vector_table;					//�ж�������
}TSTACK_DEFINE;

#ifdef _PROT_UNIT_

void exp_OnException(const char * szException, uint32 abort_addr);

uint32 _GetSysModeSp(void); //��ȡϵͳģʽ��sp
uint32 _GetIrqModeSp(void); //��ȡIRQģʽ��so
uint32 _GetSysModeLr(void); //��ȡϵͳģʽ��lr
uint32 _GetIrqModeLr(void); //��ȡIRQģʽ��lr

void exp_SaveUsrSP(uint32 sp);
void exp_SaveAbtSP(uint32 sp);
void exp_SaveUndSP(uint32 sp);
void exp_SaveIrqSP(uint32 sp);
int32 exp_get_stack_used(uint32 begin/*��*/, uint32 end/*С*/);
void Print_StackInfo(void);

#endif

#endif /* SRC_H_PUBLIC_PLT_EXCEPTION_H_ */
