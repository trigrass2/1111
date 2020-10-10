/*
 * @Author: zhongwei
 * @Date: 2019-11-21 13:51:37
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 14:23:30
 * @Description: OS模拟驱动
 * @FilePath: os_none.h
 */
#ifndef _NONE_OS_VM_H 
#define _NONE_OS_VM_H

extern volatile OS_TIME OSTime;
extern volatile INT32U OS_ISRReg;
extern volatile INT8U OSIntNesting;
extern volatile OS_IVG OSCurIntIVG;
extern volatile INT32U OS_IdleCnt;

//模拟系统初始化
void OSInit(void);
void OSStart(void);

INT8U OSIsRuning(void);

void OSTimeTick(void);

//获取当前OS时间
OS_TIME  OSTimeGet (void);

//设置OS系统时间
void  OSTimeSet (OS_TIME ticks);

//OS延时处理
void OSTimeDly(INT16U ticks);

OS_IVG OSIntEnter(OS_IVG ivg);                  //中断进入前处理
void OSIntExit(OS_IVG ivg, OS_IVG old_ivg);     //中断退出前处理
OS_IVG OSGetCurIVG(void);                       //获取当前所在中断优先级
INT32U OSIntRegGet(void);                       //获取OS中断标志
INT8U OSIsIntEnter(OS_IVG ivg);
INT32U OSGetIntCount(OS_IVG ivg);

//创建一个信号量
INT16S  OS_Signal_Create(void);
//获取一个信号量
void OS_Signal_Pend(INT16S signal);
//释放一个信号量
void OS_Signal_Post(INT16S signal);
//访问一个信号量
INT8U OS_Signal_Accept(INT16S signal);

//清除全部任务分析数据
void OS_ClearUsage(void);

//获取任务使用量信息
INT64U OS_GetIVGUsage(OS_IVG ivg);

#endif	//_NONE_OS_VM_H
