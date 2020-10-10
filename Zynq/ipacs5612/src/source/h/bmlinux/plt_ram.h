/*
 * @Author: zhongwei
 * @Date: 2019/11/27 13:57:28
 * @Description: BM内存存储
 * @File: plt_ram.h
 *
*/

#ifndef SRC_PROJECT_PLT_RAM_H_
#define SRC_PROJECT_PLT_RAM_H_


//MMU初始化
void PowerOn_Init_MMU(void);


//初始化 BM<->Linux共享内存
void PowerOn_Init_SharedMemory(void);

#endif /* SRC_PROJECT_PLT_RAM_H_ */
