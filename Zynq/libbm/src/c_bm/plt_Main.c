/*
 * @Author: zhongwei
 * @Date: 2019-11-18 15:42:44
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 17:27:56
 * @Description: 
 * @FilePath: \ZynqBM\src\plt_Main.c
 */
/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include "plt_include.h"

void ApplicationInit(void);

//主函数
SECTION_PLT_CODE int plt_main(void)
{
    //初始化Printf打印的UART口 - UART0
    hd_Uart0PsInit();  

    //动态内存初始化
    mem_Initial();
    //注册字符串资源
    PowerOn_InitStringResource();

    //时钟初始化
    PowerOn_InitClock();

    init_logmsg();      //logmsg初始化

    //初始化内存
    PowerOn_Inital_Memory();

    //init_platform();

    //系统初始化
    OSInit();

    PRINTF_L1("into main core_num=%d\n\r", hd_get_current_cpu_num());

    //硬件初始化
    PowerOn_Initial_Hardware();

	test_goose_rcv();

	ApplicationInit(); //后面在这个位置打开
	
    OS_StartToRun();
	
    while (1)
    {
        extern void xIdelTaskHandler(void *CallBackRef);
        OS_IdleCnt++;

        //调用空闲任务handler
        xIdelTaskHandler(NULL);
    }

    PRINTF_L1("main exit, core_num=%d\n\r", hd_get_current_cpu_num());

    //cleanup_platform();
    return 0;
}
