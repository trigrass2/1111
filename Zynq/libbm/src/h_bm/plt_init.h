/*
 * @Author: zhongwei
 * @Date: 2019-11-21 09:36:48
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 14:51:36
 * @Description: 初始化接口
 * @FilePath: \ZynqBM\src\project\plt_init.h
 */
#ifndef SRC_PROJECT_PLT_INIT_H_
#define SRC_PROJECT_PLT_INIT_H_

//上电内存初始化
void PowerOn_Inital_Memory(void);

//上电，硬件初始化
BOOL PowerOn_Initial_Hardware(void);

//OS系统启动，需要在设定值等初始化之前
void OS_StartToRun(void);

#endif /* SRC_PROJECT_PLT_INIT_H_ */
