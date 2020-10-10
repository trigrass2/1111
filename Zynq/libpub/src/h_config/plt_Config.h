/*
 * @Author: zhongwei
 * @Date: 2019-11-19 11:07:20
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 14:34:37
 * @Description: 平台配置代码
 * @FilePath: \ZynqBM\src\config\plt_Config.h
 */
#ifndef SRC_INCLUDE_PLT_CONFIG_H_
#define SRC_INCLUDE_PLT_CONFIG_H_

//检查是否在调试模式下
#ifdef _DEBUG
//#   warning ">>>>>> in debug mode."
#endif

//检查是否配置了_DEBUG和_NDEBUG
#if defined(_DEBUG) && defined(_NDEBUG)
#	error "You cann't define _DEBUG and _NDEBUG together"
#endif

//#ifndef _DEBUG
//#   define _NDEBUG
//#endif

/** 默认使能TRACE功能 **/
#ifndef TRACE_ENABLE
#   define TRACE_ENABLE 1
#endif	//TRACE_ENABLE

/** 默认使能ASSERT功能 **/
#ifndef ASSERT_ENABLE
#   define ASSERT_ENABLE 1
#endif	//ASSERT_ENABLE

/** 默认使能LOGMSG功能 **/
#ifndef LOGMSG_ENABLE
#   define LOGMSG_ENABLE 1
#endif	//LOGMSG_ENABLE

/** 默认使能PRINTF功能 **/
#ifndef PRINTF_ENABLE
#   define PRINTF_ENABLE 1
#endif	//PRINTF_ENABLE

//语言支持
#define _LANGUAGE_CN_ENABLE
#define _LANGUAGE_EN_ENABLE
//#define _LANGUAGE_RU_ENABLE

#ifdef _PROT_UNIT_
#include "plt_PU_Config.h"
#endif

#ifdef _MANG_UNIT_
#include "plt_MU_Config.h"
#endif

//对于BM程序，强制定义CPU个数为1
#ifdef _PROT_UNIT_
#   define get_cpu_count()     1
#endif

#endif /* SRC_INCLUDE_PLT_CONFIG_H_ */
