/*
 * @Author: zhongwei
 * @Date: 2019-11-19 10:15:43
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-19 10:34:31
 * @Description: section定义，定义方法见gcc文档ld.pdf
 * @FilePath: plt_SectionDef.h
 */
#ifndef SRC_INCLUDE_PLT_SECTIONDEF_H_
#define SRC_INCLUDE_PLT_SECTIONDEF_H_

/************************************************************************/
//  裸核侧section定义
//
/************************************************************************/

#if defined(_WIN32)
#define SECTION_PLT_CODE
#define SECTION_PLT_DATA
#define SECTION_PLT_CONST
#define SECTION_IDEEXP_CODE
#define SECTION_IDEEXP_DATA
#define SECTION_IDEEXP_CONST
#define SECTION_DYN_MEMORY
#define SECTION_SHARED_DATA
#define SECTION_NOCACHE_DATA
#define SECTION_DEF_DataDef
#elif defined(_PROT_UNIT_)

#define SECTION_PLT_CODE            __attribute__((section(".platform_code")))          //平台代码段
#define SECTION_PLT_DATA            __attribute__((section(".platform_data")))          //平台数据段
#define SECTION_PLT_CONST           __attribute__((section(".platform_const")))         //平台const数据段

#define SECTION_IDEEXP_CODE         __attribute__((section(".datadef_code")))           //IDE导出代码段
#define SECTION_IDEEXP_DATA         __attribute__((section(".datadef_data")))           //IDE导出数据段
#define SECTION_IDEEXP_CONST        __attribute__((section(".datadef_const")))          //IDE导出const数据段

#define SECTION_DYN_MEMORY          __attribute__((section(".dyn_memory")))             //动态内存空间

#define SECTION_SHARED_DATA         __attribute__((section(".shared_data")))            //不开cache的核间共享内存
#define SECTION_NOCACHE_DATA        __attribute__((section(".no_cache")))               //不开cache的内存

/*
    IDE Export出的数据段
*/
#define SECTION_DEF_DataDef         SECTION_IDEEXP_CONST

#else
/************************************************************************/
//  Linux侧section定义
//  由于Linux无需进行section定义，因此这里均定义为空
/************************************************************************/

#define SECTION_PLT_CODE
#define SECTION_PLT_DATA
#define SECTION_PLT_CONST
#define SECTION_IDEEXP_CODE
#define SECTION_IDEEXP_DATA
#define SECTION_IDEEXP_CONST
#define SECTION_DYN_MEMORY
#define SECTION_SHARED_DATA
#define SECTION_NOCACHE_DATA
#define SECTION_DEF_DataDef
#endif

//告诉GCC，变量或函数不想被有优化掉
#define NO_ELIMINATE  __attribute__ ((__used__))

#endif /* SRC_INCLUDE_PLT_SECTIONDEF_H_ */
