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

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_io.h"
#include "xuartns550.h"
#include "xscugic_hw.h"
#include "xscugic.h"

XUartNs550 XUartNs550Instance;
XUartNs550 *UartNs550InstancePtr = &XUartNs550Instance;

XUartNs550 XUartNs550Instance1;
XUartNs550 *UartNs550InstancePtr1 = &XUartNs550Instance1;


char send_ptr[100] = "abcdefghijklmn!";
char recv_buf[100] = {0};

#define INTC_DEVICE_ID XPAR_PS7_SCUGIC_0_DEVICE_ID //只有0，一个中断控制器
XScuGic InterruptController;

void  XUartNs550_DataHandler(void *CallBackRef, u32 Event,
		unsigned int ByteCount)
{
	(void) CallBackRef;
	(void) Event;
	(void) ByteCount;
}

int initGic()
{
	int status;
	XScuGic_Config *IntcConfig;
	XScuGic *GicInstancePtr = &InterruptController;

	//1. 初始化异常处理系统
	Xil_ExceptionInit();
	//2. 初始化中断控制器
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
  if (NULL == IntcConfig)
	{
	  printf("SetupInterruptSystem  LookupConfig fial.\n\r");
		return XST_FAILURE;
	}
  status = XScuGic_CfgInitialize(GicInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
    if (status != XST_SUCCESS)
    {
    	printf("SetupInterruptSystem CfgInitialize fial.\n\r");
        return XST_FAILURE;
    }
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                     GicInstancePtr);
	//4. 连接　Uart１对应的中断ID
	status = XScuGic_Connect(GicInstancePtr, XPAR_FABRIC_AXI_UART16550_9_IP2INTC_IRPT_INTR, (Xil_InterruptHandler)XUartNs550_InterruptHandler, &UartNs550InstancePtr);
	if(status != XST_SUCCESS)
	{
		printf("Connect GIC failed\n");
		return XST_FAILURE;
	}
	//5. 设置Uart中断回调函数
	XUartNs550_SetHandler(UartNs550InstancePtr, XUartNs550_DataHandler ,NULL);
	//6. 设置Uart中断类型，timeout
	XUartNs550_EnableIntr(UartNs550InstancePtr->BaseAddress);
	//设置字节
	XUartNs550_SetFifoThreshold(UartNs550InstancePtr,XUN_FIFO_TRIGGER_08);
	//7. 使能Uart中断
	XScuGic_Enable(GicInstancePtr, XPAR_FABRIC_AXI_UART16550_9_IP2INTC_IRPT_INTR);
	//8. 使能异常处理系统
	Xil_ExceptionEnable();
	return status;
}

void xuartns550_test()
{
	int Status;
	int sendbyte;
	int recvbyte;
	int i;
	Status = XUartNs550_Initialize(UartNs550InstancePtr, XPAR_AXI_UART16550_9_DEVICE_ID);
	if(XST_SUCCESS != Status)
	{
		return Status;
	}
		
    /*Status = XUartNs550_Initialize(UartNs550InstancePtr1, XPAR_AXI_UART16550_10_DEVICE_ID);

	if(XST_SUCCESS != Status)
	{
		return Status;
	}*/

	Status = XUartNs550_SetBaudRate(UartNs550InstancePtr, 230400);
	if (Status != XST_SUCCESS) {
		UartNs550InstancePtr->IsReady = 0;
		return Status;
	}

	//initGic();
	//XUartNs550_SetHandler(UartNs550InstancePtr, XUartNs550_DataHandler ,NULL);

	//XUartNs550_EnableIntr(UartNs550InstancePtr->BaseAddress);

	/*Status = XUartNs550_SetBaudRate(UartNs550InstancePtr1, 230400);
	if (Status != XST_SUCCESS) {
		UartNs550InstancePtr1->IsReady = 0;
		return Status;
	}*/

	while(1)
	{
		sendbyte = XUartNs550_Send(UartNs550InstancePtr,
									send_ptr,
									strlen(send_ptr));
		printf("uart 0 send %d bytes : %s\r\n", sendbyte, send_ptr);

		sleep(1);
		memset(recv_buf, 0, sizeof(recv_buf));
		recvbyte = XUartNs550_Recv(UartNs550InstancePtr,
								recv_buf,
								sizeof(recv_buf));

		if (recvbyte > 0)
		{
			printf("uart 0 recv %d bytes : %s\r\n", recvbyte, recv_buf);
		}
	}

}


int main()
{
	int regVal;

    init_platform();

	printf("Enter %s \r\n", __FUNCTION__);

    regVal = Xil_In32(0x40008018);
    regVal = Xil_In32(0x40008020);
    regVal = Xil_In32(0x40008024);


#if 0
    while(1)
    {
    	printf("Enter %s \r\n", __FUNCTION__);
    }
#endif
    xuartns550_test();

    regVal = Xil_In32(0x43c01000);
    regVal = Xil_In32(0x43c01004);
    regVal = Xil_In32(0x43c0101c);

    regVal = Xil_In32(0x43c11000);
    regVal = Xil_In32(0x43c11004);
    regVal = Xil_In32(0x43c1101c);

    regVal = Xil_In32(0x43c21000);
    regVal = Xil_In32(0x43c21004);
    regVal = Xil_In32(0x43c2101c);

    print("Hello World\n\r");

    cleanup_platform();
    return 0;
}
