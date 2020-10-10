/*
 * @Author: zhongwei
 * @Date: 2019-11-21 13:51:24
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 14:24:30
 * @Description: OS操作系统对外接口重定向
 * @FilePath: os_interface.h
 */
#ifndef _OS_INTERFACE_H_
#define _OS_INTERFACE_H_

#include "os_cpu_a.h"
#include "os_none.h"
#include "os_memory.h"

//延时多个 Tick
#define	DELAY_TICK(n)					    OSTimeDly(n)

//OS系统是否运行
#define IS_OS_RUNING()						OSIsRuning()

#endif
