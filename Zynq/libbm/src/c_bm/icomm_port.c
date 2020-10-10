/*
 * @Author: zhongwei
 * @Date: 2020/3/25 14:48:03
 * @Description: libcpp对外提供的接口定义
 * @File: cpp_port.cc
 *
*/

#include "plt_include.h"
#include "icomm_port.h"
#include "icomm_code.h"
#include "icomm_config.h"

/************************************************************************/
//  内部通讯处理接口
//
/************************************************************************/
//内部通讯初始化 需要在 PowerOn_Init_McInnerComm 后调用
SECTION_PLT_CODE void PowerOn_Initial_Inner_Comm(void)
{
    TMC_INNER_COMM_STRUCT *pInnerSend = GetSharedBMInnerComm();
    if (pInnerSend != NULL && IsSharedBMMemValid())
    {
        pInnerSend->version = ICOMM_VERION;    //置上规约版本
        PRINTF_WITH_LOG("PowerOn_Initial_Inner_Comm version=%d linux_ver=%d\n\r", pInnerSend->version, GetLinuxInnerCommVerion());
    }

    //PowerOn_Initial_InnerCommBMPlt();
}

//在通讯任务中调用
SECTION_PLT_CODE void inner_comm_handle_intask(const TTimeRun * pTimeRun)
{
}

