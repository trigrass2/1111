/*
 * @Author: zhongwei
 * @Date: 2019/12/2 11:21:30
 * @Description: 导出代码的头文件（平台代码不能包含该头文件）
 * @File: Export_Inc.h
 * 
 * 该文件只在zynqBM下的代码才会include，在libpub及libbm中是不会include的
*/

#ifndef SRC_INTERFACE_EXPORT_INC_H_
#define SRC_INTERFACE_EXPORT_INC_H_


#ifdef _PROT_UNIT_
#include "plt_include.h"                //libbm/h_baremetal
#endif

#endif /* SRC_INTERFACE_EXPORT_INC_H_ */
