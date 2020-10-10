/*
 * @Author: zhongwei
 * @Date: 2019/11/27 10:47:47
 * @Description: 裸核print打印输出到linux核的功能接口
 * @File: plt_mc_print.h
 *
*/
#ifndef SRC_PROJECT_PLT_PRINT_H_
#define SRC_PROJECT_PLT_PRINT_H_

//初始化Print共享到Linux的内存空间
void PowerOn_Init_PrintToLinux(void);

//打印输出到共享内存，将字符串输出给Linux 
void Print_To_Linux(const char * str);

#endif /* SRC_PROJECT_PLT_PRINT_H_ */
