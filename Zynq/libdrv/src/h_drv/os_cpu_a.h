/*
 * @Author: zhongwei
 * @Date: 2019-11-21 11:19:14
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 14:51:01
 * @Description: Contex-A9 ARM v7 开关中断接口（从usos-ii移植）
 * @FilePath: cpu_a.h
 * 
 * 使用临界区说明
 * xxxxx()
 * {
 *      CPU_SR_ALLOC();             //分配CPU_SR
 *      CPU_CRITICAL_ENTER();       //进入临界区
 *      ...
 *      CPU_CRITICAL_EXIT();        //退出临界区
 * }
 */
#ifndef SRC_HARDWARE_CPU_A_H_
#define SRC_HARDWARE_CPU_A_H_


typedef unsigned int    BOOLEAN;    /* Unsigned  8 bit quantity                                        */
typedef unsigned char   INT8U;      /* Unsigned  8 bit quantity                                        */
typedef signed   char   INT8S;      /* Signed    8 bit quantity                                        */
typedef unsigned short  INT16U;     /* Unsigned 16 bit quantity                                        */
typedef signed   short  INT16S;     /* Signed   16 bit quantity                                        */
typedef unsigned int    INT32U;     /* Unsigned 32 bit quantity                                        */
typedef signed   int    INT32S;     /* Signed   32 bit quantity                                        */
typedef unsigned long long    INT64U;
typedef long long    INT64S;
typedef float           FP32;       /* Single precision floating point                                 */
typedef double          FP64;       /* Double precision floating point                                 */

typedef INT32U          OS_STK;     /* Each stack entry is 32-bit wide                                 */
typedef INT32U          OS_CPU_SR;  /* Define size of CPU status register                              */
typedef INT8U	        OS_IVG;		/* OS IVG中断优先级 */	

typedef uint64			OS_TIME;	//OS时间类型

#define OS_TICKS_PER_SEC        1000    //定义每秒tick数 1ms一次

#define OS_MAX_SIGNAL_COUNT		10		//信号量个数，10个

#define IRQ_NONE        0xff

#define num_interrupt_kind      96      //zynq总共96个中断

//每秒任务tick数
#define  os_get_ticks_per_sec()    OS_TICKS_PER_SEC

#define  CPU_SR_ALLOC()     OS_CPU_SR  cpu_sr = (OS_CPU_SR)0

#define  CPU_INT_DIS() __asm__ __volatile__ ("mrs  %[sr_res], cpsr\r\n" "cpsid if\r\n" "dsb\r\n" : [sr_res]"=r" (cpu_sr) :: "memory");

#define  CPU_INT_EN()  __asm__ __volatile__ ("dsb\r\n" "msr  cpsr_c, %[sr_val]\r\n" :: [sr_val]"r" (cpu_sr) : "memory");

#define  CPU_CRITICAL_ENTER()  do { CPU_INT_DIS(); } while (0)          /* Disable   interrupts.                        */
#define  CPU_CRITICAL_EXIT()   do { CPU_INT_EN();  } while (0)          /* Re-enable interrupts.                        */



void        CPU_IntDis       (void);
void        CPU_IntEn        (void);

OS_CPU_SR   CPU_SR_Save      (void);
void        CPU_SR_Restore   (OS_CPU_SR  cpu_sr);

void        CPU_WaitForInt   (void);
void        CPU_WaitForEvent (void);

#endif /* SRC_HARDWARE_CPU_A_H_ */
