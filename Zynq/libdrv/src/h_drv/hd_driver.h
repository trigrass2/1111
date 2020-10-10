/*
 * @Author: zhongwei
 * @Date: 2019-11-20 08:51:38
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-20 10:06:31
 * @Description: 硬件驱动接口
 * @FilePath: hd_driver.h
 */

#ifndef SRC_PROJECT_PLT_HD_DRIVER_H_
#define SRC_PROJECT_PLT_HD_DRIVER_H_

#include "xenv_standalone.h"
#include "hd_parameters.h"
#include "hd_mmu.h"
#include "hd_driver_const.h"

//libdrv版本
#define VERSION_LIB_DRV         "1.00"
const char * get_version_libdrv(void);               //版本
TFullDateTime get_buildtime_libdrv(void);   //编译时间

enum E_HD_DRIVER_DEBUG_MSG{
    HD_DRV_DEBUG_NONE           = 0x00,     //无调试
    HD_DRV_DEBUG_IRQ_NESTING    = 0x01,     //IRQ嵌套
    HD_DRV_DEBUG_GOOSE_RCV      = 0x02,     //Goose接收
    HD_DRV_DEBUG_SV_RCV         = 0x04,     //SV接收
};

extern uint32 g_HdDrvDebug;
#define IsDrvDebugMsg(n)   (g_HdDrvDebug & (n))

//NOP空指令
#define NOP() __asm volatile("NOP")

/*
指令中有时还有出现cpsr_cf, cpsr_all, cpsr_c等，这里：
        c 指  CPSR中的control field ( PSR[7:0])
        f 指  flag field (PSR[31:24])
        x 指  extend field (PSR[15:8])
        s 指  status field ( PSR[23:16])

其中cpsr的位表示为：
31 30 29 28  ---   7   6   -   4     3     2     1     0
N   Z   C   V      I   F       M4    M3    M2    M1    M0
                               0    0    0    0    0     User26 模式
                               0    0    0    0    1     FIQ26 模式
                               0    0    0    1    0     IRQ26 模式
                               0    0    0    1    1     SVC26 模式
                               1    0    0    0    0     User 模式
                               1    0    0    0    1     FIQ 模式
                               1    0    0    1    0     IRQ 模式
                               1    0    0    1    1     SVC 模式
                               1    0    1    1    1     ABT 模式
                               1    1    0    1    1     UND 模式
                               1    1    1    1    1     SYS 模式
 
  I[7]  - IRQ 1:禁止/0:允许
  F[6]  - FIQ 1:禁止/0:允许
  T[5]  - 工作状态 1:Thumb 0:ARM
 
*/

//读取当前寄存器的CPSR
#define hd_get_cpsr()   mfcpsr()

//读取spsr，spsr保存的是上一个模式的cpsr
#define hd_get_spsr()   ({u32 rval = 0U; \
              __asm__ __volatile__(\
                "mrs    %0, spsr\n"\
                : "=r" (rval)\
              );\
              rval;\
             })

//cpsr_c中低5位的值 M[4:0]
enum CPU_MODE{
    CPU_MODE_USR = 0x10,        //0b10000 User模式 在平台代码中，实际无使用usr模式
    CPU_MODE_FIQ = 0x11,        //0b10001 FIQ模式
    CPU_MODE_IRQ = 0x12,        //0b10010 IRQ模式
    CPU_MODE_SVC = 0x13,        //0b10011 SVC模式
    CPU_MODE_ABT = 0x17,        //0b10111 ABT模式
    CPU_MODE_UND = 0x1B,        //0b11011 UND模式
    CPU_MODE_SYS = 0x1F,        //0b11111 SYS模式  大部分工作在SYS模式下
};

//获取当前CPU模式
#define hd_get_cur_cpu_mode()   (hd_get_cpsr() & 0x1F)

//CPU核心频率
#define hd_get_cpu_clk_freq()       APP_CPU_CLK_FREQ

//Timer时钟频率
#define hd_get_timer_clk_freq()     APP_TIMER_CLK_FREQ

//获取当前CPU core num
uint32 hd_get_current_cpu_num(void);

//获取当前CPU Main ID Register
uint32 hd_get_current_cpu_main_id(void);

//获取当前cycle计数器的值 uint32
uint32 hd_get_cycles(void);

//获取当前cycle计数器的值 uint64
uint64 hd_get_cycles64(void);

//硬件休眠一段cycle
void hd_sleep_cycles(int64 cycle);

//硬件休眠函数
void hd_sleep_seconds(uint32 seconds);
void hd_sleep_mseconds(uint32 mseconds);
void hd_sleep_useconds(uint32 useconds);

//从串口读取一个字符
char SHELL_inchar(void);
//往串口打印一个字符
void SHELL_outchar(char c);

//硬件BSP初始化
void hd_bsp_initial(void);

//UART0初始化，stdout输出端口
int hd_Uart0PsInit(void);

//设置GPIO方向为输出，并置Data初值
void hd_gpio_SetDirectionOut(u32 io, u32 Data);
//设置GPIO为输入，并返回输入的值
u32 hd_gpio_SetDirectionIn(u32 io);
//设置开出
void hd_gpio_SetOut(u32 io, u32 Data);
//读取开入
u32 hd_gpio_ReadIn(u32 io);

//刷新闭锁继电器
void hd_do_stall_flush(void);

#endif /* SRC_PROJECT_PLT_HD_DRIVER_H_ */
