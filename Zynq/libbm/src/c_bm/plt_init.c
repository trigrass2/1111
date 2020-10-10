/*
 * @Author: zhongwei
 * @Date: 2019-11-21 09:36:55
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 16:35:46
 * @Description: 初始化
 * @FilePath: \ZynqBM\src\project\plt_init.c
 */
#include "plt_include.h"
#include "plt_init.h"
#include "bm_sv_rcv.h"
#include "bm_goose_rcv.h"

/**
 * @Function: PowerOn_Inital_Memory
 * @Description: 上电内存初始化，包括MMU，共享内存等 
 * @author zhongwei (2019/11/28 15:15:19)
 * 
 * @param void 
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE void PowerOn_Inital_Memory(void)
{
    extern void PowerOn_InitException(void);

    PowerOn_Init_MMU();             //MMU初始化

    PowerOn_InitException();        //异常处理初始化

    PowerOn_Init_SharedMemory();    //共享内存初始化

    PowerOn_Init_PrintToLinux();    //Print共享打印内存初始化

    PowerOn_Init_McInnerComm();     //BM发送到Linux通讯内存初始化

    PowerOn_InitSoeBuffer();        //SOE缓冲区初始化

    //共享内存自检
    if (IsFalse(CheckSharedMemory()))
    {
        ASSERT(!"CheckSharedMemory in PowerOn_Inital_Memory fail!");     //共享内存检查失败，则报异常
    }
}

/**
 * @Author: zhongwei
 * @Date: 2019-11-21 09:39:28
 * @description: 上电，硬件初始化 
 * @param {type} 
 * @return: PLT_TRUE/PLT_FALSE
 */
SECTION_PLT_CODE BOOL PowerOn_Initial_Hardware(void)
{
    //硬件BSP初始化
    hd_bsp_initial();

    if(hd_SetupInterruptSystem() != XST_SUCCESS)
    {
        PRINTF_L1("hd_SetupInterruptSystem fail!\n\r");
        return PLT_FALSE;
    }
    //任务注册到驱动
    PowerOn_RegisterTsk();

    //注册FPGA消息回调函数（注册到驱动层）
    fpgs_msg_register_OnRcvSv(on_bm_sv_rcv);
    fpgs_msg_register_OnRcvGoose(on_bm_goose_rcv);

    return PLT_TRUE;
}

//等待Linux配置数据，在SetBaremetalResetFlag(0)后调用
//需要一直等待，并每秒打印等待信息 GetBaremetalWaitFlag(0)
SECTION_PLT_CODE void WaitForLinux(void)
{
    uint32 uSleep = 0;
    const uint32 uSleepMs = 100;  //每次等待100ms
    uint32 i=0;
    while (1)
    {
        if (GetBaremetalWaitFlag(0) == FLAG_SHARED_RESET_VALID)
        {
            PRINTF_WITH_LOG("WaitForLinux wait_flag0 success.\n\r");
            break;
        }

        i++;
        if (uSleep >= 1000)
        {
            uSleep = 0;
            PRINTF_WITH_LOG("WaitForLinux wait_flag0 %d ...\n\r", i);
        }
        hd_sleep_mseconds(uSleepMs);
        uSleep += uSleepMs;
    }
}

/**
 * @Author: zhongwei
 * @Date: 2019-11-21 14:45:15
 * @description: OS系统启动，需要在设定值等初始化之前
 * @param {type} 
 * @return: 
 */
SECTION_PLT_CODE void OS_StartToRun(void)
{
    extern void SetDebugMessageStr(const char ** str_arr );
    extern void PowerOn_Initial_Inner_Comm(void);

    SetDebugMessageStr(_DebugMessageStr);

    SetBaremetalResetFlag(0);        //BM启动成功标志，用以通知Linux应用

    //等待Linux通知可以继续执行
    //WaitForLinux();

    SetBaremetalResetFlag(1);        //BM等待成功标志，用以通知Linux应用

    PowerOn_Initial_Inner_Comm();      //内部通讯初始化

    //字符串资源转换 - 在所有字符串注册完成后调用
    RegisterStringToArray();

    //初始化最高优先级定时器中断 - 不再使用，改由FPGA触发
    hd_InitIsr_PrivateTimer();

    //FPGA中断初始化 SV/Goose接收中断
    hd_InitIsr_Fpga();

    //初始化ttc定时器
    hd_InitIsr_TtcTimer();

    //初始化软中断
    hd_InitIsr_SgiInt();

    //初始化空闲任务中断
    hd_InitIsr_IdleTsk();

    //启动系统
    OSStart();
}
