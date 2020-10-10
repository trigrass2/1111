/*
 * @Author: zhongwei
 * @Date: 2019-11-19 09:55:27
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 14:47:48
 * @Description: 公共include
 * @FilePath: \ZynqBM\src\include\plt_inc_c.h
 */
#ifndef SRC_INCLUDE_PLT_INC_C_H_
#define SRC_INCLUDE_PLT_INC_C_H_

//标准C/C++ 头文件
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "plt_Config.h"                 //Config配置
#include "plt_SectionDef.h"             //section定义
#include "plt_const.h"                  //常量
#include "plt_BaseType.h"               //基本数据类型
#include "plt_SoeType.h"                //SOE数据类型定义
#include "plt_Export_BuildInEnum.h"
#include "plt_Lib.h"                    //LIB库统一定义
#include "plt_pub_struct.h"             //公共数据结构定义
#include "plt_Func.h"                   //公共函数接口
#include "plt_trace.h"                  //TRACE/ASSERT功能接口
#include "plt_configdata.h"
#include "plt_Time.h"                   //时间处理公共接口
#include "plt_string_res.h"
#include "plt_exception.h"
#include "plt_shared_memory.h"
#include "plt_string_res.h"
#include "plt_viewstring.h"

#ifdef __cplusplus
}
#endif

#endif /* SRC_INCLUDE_PLT_INC_C_H_ */
