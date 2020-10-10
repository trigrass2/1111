/*
 * @Author: zhongwei
 * @Date: 2019/12/15 14:25:11
 * @Description: 软件信息访问接口
 * @File: plt_if_build.h
 * 
 * plt_if_build.c在应用层编译，（手工编写）
*/
#ifndef SRC_INCLUDE_PLT_IF_BUILD_H_
#define SRC_INCLUDE_PLT_IF_BUILD_H_

//读取编译时间 在应用层实现的接口 plt_if_build.c文件中
const char * getBuildDate(void);
const char * getBuildTime(void);

#endif  //SRC_INCLUDE_PLT_IF_BUILD_H_
