/*
 * @Author: zhongwei
 * @Date: 2019-11-20 08:53:11
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 09:37:26
 * @Description: SoC平台公共包含头文件
 * @FilePath: \ZynqBM\src\project\plt_include.h
 */
#ifndef SRC_PROJECT_PLT_INCLUDE_H_
#define SRC_PROJECT_PLT_INCLUDE_H_

//xilinx SDK包含的头文件
#include "xstatus.h"
#include "xil_types.h"
#include "xil_printf.h"                 //xil_printf
#include "xil_exception.h"              //中断


#include "plt_inc_c.h"      //include下的头文件包含
#include "libdrv.h"  //硬件驱动接口

#include "plt_bm_struct.h"              //裸核使用的数据结构定义
#include "plt_init.h"       //初始化
#include "plt_task.h"       //任务处理
#include "plt_ram.h"        //内存存储结构
#include "plt_mc_print.h"           //打印输出到Linux
#include "plt_mc_inner_comm.h"      //共享内存通讯接口
#include "plt_soe_handle.h"     //soe处理

#include "plt_bm_dbgmsg.h"              //BM侧调试信息定义
#include "plt_bm_time.h"        //BM侧时间处理

#include "bm_io.h"


#endif /* SRC_PROJECT_PLT_INCLUDE_H_ */
