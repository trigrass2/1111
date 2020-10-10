/*
 * @Author: zhongwei
 * @Date: 2019-11-21 09:14:09
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 09:14:44
 * @Description: 中断任务处理
 * @FilePath: \ZynqBM\src\project\plt_task.h
 */
#ifndef SRC_PROJECT_PLT_TASK_H_
#define SRC_PROJECT_PLT_TASK_H_

void scantime500us(void);

//yx防抖等
void tskTime500us(const TTimeRun * pTimeRun);

//AD采样中断
void tskADSimple(const TTimeRun * pTimeRun);

//系统tick任务
void tskOSTick(const TTimeRun * pTimeRun);

//快速任务
void tskFast(const TTimeRun * pTimeRun);

//慢速任务
void tskSlow(const TTimeRun * pTimeRun);

//通讯任务
void tskCommu(const TTimeRun * pTimeRun);

//用户任务
void tskUser(const TTimeRun * pTimeRun);

//空闲任务, 在while循环中调用
void tskIdle(const TTimeRun * pTimeRun);

//任务入口注册 需要在PowerOn_Initial_Hardware后调用
void PowerOn_RegisterTsk(void);

#endif /* SRC_PROJECT_PLT_TASK_H_ */
