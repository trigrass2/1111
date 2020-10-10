/*
 * @Author: zhongwei
 * @Date: 2019-11-20 09:24:03
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-20 09:28:04
 * @Description: Zynq cpu硬件参数定义
 * @FilePath: hd_parameters.h
 */
#ifndef SRC_HARDWARE_PLT_HD_PARAMETERS_H_
#define SRC_HARDWARE_PLT_HD_PARAMETERS_H_

#include "xparameters.h"

//Core Freq arm的核心频率
#define APP_CPU_CLK_FREQ		XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ			//对于7015是766MHz

//Timer Freq 时钟频率 (CPU核心频率的1/2)
#define APP_TIMER_CLK_FREQ      (APP_CPU_CLK_FREQ /2)

#endif /* SRC_HARDWARE_PLT_HD_PARAMETERS_H_ */
