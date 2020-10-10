/*
 * @Author: zhongwei
 * @Date: 2020/2/18 15:23:56
 * @Description: 共享内存
 * @File: plt_share_memory.c
 *
*/

#include "plt_include.h"
#include "plt_shared_memory.h"

#ifdef _PROT_UNIT_

/**
 * g_SharedMemory
 * @Description:  共享内存
 * @author zhongwei (2019/11/27 14:00:29)
 *  
 * 在内存中的地址是0x3f000000 
*/
SECTION_SHARED_DATA TSHARE_MEMORY_STRUCT g_SharedMemory;

#else

/**
 * @Function: g_pSharedMemory
 * @Description: BM与Linux共享内存的起始地址 
 * @author zhongwei (2020/2/7 9:48:15)
 * 
*/
TSHARE_MEMORY_STRUCT * g_pSharedMemory = NULL;

#endif  //_PROT_UNIT_


/**
 * @Function: CheckSharedBaremetalMemory
 * @Description: 检查 Baremetal共享内存  TSHARED_BAREMETAL_STRUCT
 * @author zhongwei (2020/2/19 10:26:21)
 * 
 * @return SECTION_PLT_CODE BOOL 
 * 
*/
SECTION_PLT_CODE BOOL CheckSharedBaremetalMemory(void)
{
    int i;
    const TSHARED_BAREMETAL_STRUCT * pShareBM = GetSharedBMMemory();

    if (IsSharedMemoryValid() == PLT_FALSE)
    {
        LOGMSG("CheckSharedBaremetalMemory unvalid\n\r",0,0,0,0,0,0);
        return PLT_FALSE;
    }

    for (i=0;i<FLAG_SHARED_COUNT;i++)
    {
        if (pShareBM->flag_begin[i] != FLAG_SHARED_MEMORY_VALID)
        {
            LOGMSG("CheckSharedBaremetalMemory flag_begin[%d] chk fail.\n\r", i,0,0,0,0,0);
            return PLT_FALSE;
        }
        if (pShareBM->flag_end[i] != FLAG_SHARED_MEMORY_VALID)
        {
            LOGMSG("CheckSharedBaremetalMemory flag_end[%d] chk fail.\n\r", i,0,0,0,0,0);
            return PLT_FALSE;
        }
    }

    //结构版本比较
    if (pShareBM->struct_version != VERSION_SHARED_MEMORY)
    {
        LOGMSG("CheckSharedBaremetalMemory struct_version chk fail, %d != %d\n\r", pShareBM->struct_version, VERSION_SHARED_MEMORY,0,0,0,0);
        return PLT_FALSE;
    }

//#ifdef _PROT_UNIT_ //只有BM需要检查装置uuid和软件uuid
//    if (E_STRCMP(pShareBM->device_uuid, get_c_DEVICE_UUID_(_CURRENT_CPU_NUM_)) != 0)
//    {
//        LOGMSG("CheckSharedBaremetalMemory device_uuid chk fail\n\r",0,0,0,0,0,0);
//        return PLT_FALSE;
//    }
//
//    if (E_STRCMP(pShareBM->software_uuid, get_c_SOFTWARE_UUID_(_CURRENT_CPU_NUM_)) != 0)
//    {
//        LOGMSG("CheckSharedBaremetalMemory software_uuid chk fail\n\r",0,0,0,0,0,0);
//        return PLT_FALSE;
//    }
//#endif  //_PROT_UNIT_

    return PLT_TRUE;
}

/**
 * @Function: CheckSharedLinuxMemory
 * @Description: Linux共享内存检查  TSHARED_LINUX_STRUCT
 * @author zhongwei (2020/2/19 10:50:02)
 * 
 * @param void 
 * 
 * @return SECTION_PLT_CODE BOOL 
 * 
*/
SECTION_PLT_CODE BOOL CheckSharedLinuxMemory(void)
{
    int i;
    const TSHARED_LINUX_STRUCT *pSharedLinux = GetSharedLinuxMemory();

    if (IsSharedMemoryValid() == PLT_FALSE)
    {
        LOGMSG("CheckSharedLinuxMemory unvalid\n\r",0,0,0,0,0,0);
        return PLT_FALSE;
    }

    for (i = 0; i < FLAG_SHARED_COUNT; i++)
    {
        if (pSharedLinux->flag_begin[i] != FLAG_SHARED_MEMORY_VALID)
        {
            LOGMSG("CheckSharedLinuxMemory linux flag_begin[%d] chk fail.\n\r", i, 0, 0, 0, 0, 0);
            return PLT_FALSE;
        }
        if (pSharedLinux->flag_end[i] != FLAG_SHARED_MEMORY_VALID)
        {
            LOGMSG("CheckSharedLinuxMemory linux flag_end[%d] chk fail.\n\r", i, 0, 0, 0, 0, 0);
            return PLT_FALSE;
        }
    }

    //结构版本比较
    if (pSharedLinux->struct_version != VERSION_SHARED_MEMORY)
    {
        LOGMSG("CheckSharedLinuxMemory linux struct_version chk fail, %d != %d\n\r", pSharedLinux->struct_version, VERSION_SHARED_MEMORY, 0, 0, 0, 0);
        return PLT_FALSE;
    }

    return PLT_TRUE;
}

/**
 * @Function: CheckSharedMemory
 * @Description: 共享内存检查，TSHARE_MEMORY_STRUCT数据结构检查 
 * @author zhongwei (2020/2/19 10:50:53)
 * 
 * @return SECTION_PLT_CODE BOOL 
 * 
*/
SECTION_PLT_CODE BOOL CheckSharedMemory(void)
{
    int i;
    const TSHARE_MEMORY_STRUCT *pSharedMemory = GetSharedMemory();

    if (IsSharedMemoryValid() == PLT_FALSE)
    {
        LOGMSG("CheckSharedMemory unvalid\n\r",0,0,0,0,0,0);
        return PLT_FALSE;
    }

    //共享内存结构检查 TSHARE_MEMORY_STRUCT
    {
        for (i = 0; i < FLAG_SHARED_COUNT; i++)
        {
            if (pSharedMemory->flag_begin[i] != FLAG_SHARED_MEMORY_VALID)
            {
                LOGMSG("CheckSharedMemory flag_begin[%d] chk fail.\n\r", i,0,0,0,0,0);
                return PLT_FALSE;
            }
            if (pSharedMemory->flag_end[i] != FLAG_SHARED_MEMORY_VALID)
            {
                LOGMSG("CheckSharedMemory flag_end[%d] chk fail.\n\r", i,0,0,0,0,0);
                return PLT_FALSE;
            }
        }

        //结构版本比较
        if (pSharedMemory->struct_version != VERSION_SHARED_MEMORY)
        {
            LOGMSG("CheckSharedMemory struct_version chk fail, %d != %d\n\r", pSharedMemory->struct_version, VERSION_SHARED_MEMORY,0,0,0,0);
            return PLT_FALSE;
        }
    }

    return PLT_TRUE;
}

/************************************************************************/
//  Linux与BM启动配合的几个标志 - 开始
// 
//  启动流程见文档《软件启动流程.vsdx》
//  目前仅用了0标志，1标志的未使用
/************************************************************************/

/**
 * @Function: SetBaremetalResetFlag
 * @Description: 在BM启动成功后，置复位标志 该接口提供给BM
 * @author zhongwei (2020/2/14 14:29:34)
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE void SetBaremetalResetFlag(int step)
{
    TSHARED_BAREMETAL_STRUCT *pShareBM = GetSharedBMMemory();
    ASSERT_L1(IsSharedBMMemValid());

    PRINTF_WITH_LOG("SetBaremetalResetFlag %d\n\r", step);

    if (IsSharedBMMemValid())
    {
        if (step == 0)
        {
            pShareBM->reset_flag_0 = FLAG_SHARED_RESET_VALID;
        }
        else if (step == 1)
        {
            pShareBM->reset_flag_1 = FLAG_SHARED_RESET_VALID;
        }
    }
}

//获取Baremetal复位标识，该接口提供给Linux
SECTION_PLT_CODE uint32 GetBaremetalResetFlag(int step)
{
    TSHARED_BAREMETAL_STRUCT *pShareBM = GetSharedBMMemory();
    if (IsSharedBMMemValid())
    {
        if (step == 0)
        {
            return pShareBM->reset_flag_0;
        }
        else if (step == 1)
        {
            return pShareBM->reset_flag_1;
        }
    }
    return 0;
}

SECTION_PLT_CODE void ClrBaremetalResetFlag(void)
{
    TSHARED_BAREMETAL_STRUCT *pShareBM = GetSharedBMMemory();
    
    PRINTF_WITH_LOG("ClrBaremetalResetFlag\n\r");

#ifdef _PROT_UNIT_
    ASSERT_L1(IsSharedBMMemValid());        //只有保护单元，才要ASSERT
#endif
    if (IsSharedMemoryValid())              //对于Linux，必须保证shared memory已经map成功才能操作
    {
        pShareBM->reset_flag_0 = 0;
        pShareBM->reset_flag_0 = 1;
    }
}

/**
 * @Function: SetBaremetalWaitFlag
 * @Description: 设置Baremetal启动时等待标识，该接口提供给Linux
 * @author zhongwei (2020/2/14 15:39:21)
 * 
 * @param step 
 * 
*/
SECTION_PLT_CODE void SetBaremetalWaitFlag(int step)
{
    TSHARED_LINUX_STRUCT *pShareLinux = GetSharedLinuxMemory();
    ASSERT_L1(IsSharedLinuxMemValid());

    PRINTF_WITH_LOG("SetBaremetalWaitFlag %d\n\r", step);

    if (step == 0)
    {
        pShareLinux->wait_flag_0 = FLAG_SHARED_RESET_VALID;
    }
    else if (step == 1)
    {
        pShareLinux->wait_flag_1 = FLAG_SHARED_RESET_VALID;
    }
}

//获取BM等待标志，该接口提供给BM
SECTION_PLT_CODE uint32 GetBaremetalWaitFlag(int step)
{
    TSHARED_LINUX_STRUCT *pShareLinux = GetSharedLinuxMemory();
    if (IsSharedLinuxMemValid())
    {
        if (step == 0)
        {
            return pShareLinux->wait_flag_0;
        }
        else if (step == 1)
        {
            return pShareLinux->wait_flag_1;
        }
    }

    return 0;
}

//清除BM启动等待标志，接口提供给Linux
SECTION_PLT_CODE void ClrBaremetalWaitFlag(void)
{
    TSHARED_LINUX_STRUCT *pShareLinux = GetSharedLinuxMemory();
    ASSERT_L1(IsSharedLinuxMemValid());
    PRINTF_WITH_LOG("ClrBaremetalWaitFlag\n\r");
    if (IsSharedMemoryValid())          //对于Linux，必须保证shared memory已经map成功才能操作
    {
        pShareLinux->wait_flag_0 = 0;
        pShareLinux->wait_flag_1 = 1;
    }
}

/************************************************************************/
//  Linux与BM启动配合的几个标志 - 结束
//
/************************************************************************/


/************************************************************************/
//  其他的几个接口
//
/************************************************************************/
//读取BM内部通讯版本
SECTION_PLT_CODE int32 GetBaremetalInnerCommVerion(void)
{
    TMC_INNER_COMM_STRUCT *pInner = GetSharedBMInnerComm();
    if (IsSharedBMMemValid())
    {
        return pInner->version;
    }
    else
    {
        return 0;
    }
}

//读取Linux内部通讯版本
SECTION_PLT_CODE int32 GetLinuxInnerCommVerion(void)
{
    TMC_INNER_COMM_STRUCT *pInner = GetSharedLinuxInnerComm();
    if (IsSharedLinuxMemValid())
    {
        return pInner->version;
    }
    else
    {
        return 0;
    }
}
