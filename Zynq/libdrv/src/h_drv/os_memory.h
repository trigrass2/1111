/*
 * @Author: zhongwei
 * @Date: 2019/11/26 11:20:59
 * @Description: 动态内存系统
 * @File: os_memory.h
 *
*/
#ifndef SRC_OS_OS_MEMORY_H_
#define SRC_OS_OS_MEMORY_H_

void mem_Initial(void);

unsigned short mem_AddMemBlock(unsigned int nBegin, size_t nLength);
void * mem_Malloc(size_t nSize);
size_t mem_GetAllocedMemory(void);
size_t mem_GetTotalMemory(void);

#endif /* SRC_OS_OS_MEMORY_H_ */
