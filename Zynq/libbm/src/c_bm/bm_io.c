/*
 * @Author: zhongwei
 * @Date: 2020/4/2 14:36:49
 * @Description: BM开入开出控制处理
 * @File: bm_io.c
 *
*/

#include "plt_include.h"
#include "bm_io.h"
#include "xgpiops.h"            //GPIO

extern XGpioPs Gpio;

//开出控制测试
SECTION_PLT_CODE void io_do_set_test(int io, int state)
{
    if (!XGpioPs_GetDirectionPin(&Gpio, io))
    {
        XGpioPs_SetDirectionPin(&Gpio, io, 1);
    }
    if (!XGpioPs_GetOutputEnablePin(&Gpio, io))
    {
        XGpioPs_SetOutputEnablePin(&Gpio, io, 1);
    }
    
    XGpioPs_WritePin(&Gpio, io, state);
}

//开入读取测试
SECTION_PLT_CODE int io_di_get_test(int io)
{
    if (XGpioPs_GetOutputEnablePin(&Gpio, io))
    {
        XGpioPs_SetOutputEnablePin(&Gpio, io, 0);
    }
    if (XGpioPs_GetDirectionPin(&Gpio, io))
    {
        XGpioPs_SetDirectionPin(&Gpio, io, 0);
    }

    return XGpioPs_ReadPin(&Gpio, io);
}


XGpioPs GpioPs_Init()
{
	XGpioPs_Config* GpioConfigPtr;
	XGpioPs psGpioInstancePtr;

	GpioConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	XGpioPs_CfgInitialize(&psGpioInstancePtr, GpioConfigPtr, GpioConfigPtr->BaseAddr);

	return psGpioInstancePtr;
}

int iotest()
{
	static XGpioPs psGpioInstancePtr;
	psGpioInstancePtr = GpioPs_Init(psGpioInstancePtr);   //GPIO初始化

	//EMIO配置为输出

    XGpioPs_SetDirectionPin(&psGpioInstancePtr, 56,1);
    XGpioPs_SetDirectionPin(&psGpioInstancePtr, 57,1);
	XGpioPs_SetDirectionPin(&psGpioInstancePtr, 58,1);
    XGpioPs_SetDirectionPin(&psGpioInstancePtr, 59,1);

    //使能EMIO输出
    XGpioPs_SetOutputEnablePin(&psGpioInstancePtr, 56,1);
    XGpioPs_SetOutputEnablePin(&psGpioInstancePtr, 57,1);
	XGpioPs_SetOutputEnablePin(&psGpioInstancePtr, 58,1);
    XGpioPs_SetOutputEnablePin(&psGpioInstancePtr, 59,1);

	while(1)
	{
		XGpioPs_WritePin(&psGpioInstancePtr, 56, 1);//EMIO的第2位输出1
		usleep(200000);	//延时
		XGpioPs_WritePin(&psGpioInstancePtr, 56, 0);//EMIO的第2位输出0
		usleep(200000);	//延时
		XGpioPs_WritePin(&psGpioInstancePtr, 57, 1);//EMIO的第3位输出1
		usleep(200000);	//延时
		XGpioPs_WritePin(&psGpioInstancePtr, 57, 0);//EMIO的第3位输出0
		usleep(200000);	//延时
		XGpioPs_WritePin(&psGpioInstancePtr, 58, 1);//EMIO的第0位输出1
		usleep(200000);	//延时
		XGpioPs_WritePin(&psGpioInstancePtr, 58, 0);//EMIO的第0位输出0
		usleep(200000);	//延时
		XGpioPs_WritePin(&psGpioInstancePtr, 59, 1);//EMIO的第1位输出1
		usleep(200000);	//延时
		XGpioPs_WritePin(&psGpioInstancePtr, 59, 0);//EMIO的第1位输出0
		usleep(200000);	//延时
	}
    return 0;
}



