/*
 * @Author: zhongwei
 * @Date: 2020/3/26 14:22:51
 * @Description: SOE发送处理，发送到Linux
 * @File: plt_soe_send.cc
 *
*/

#include "plt_include.h"
#include "plt_soe_send.h"
#include "plt_soe_handle.h"

#define SOE_SEM_SR      CPU_SR_ALLOC
#define SOE_SEM_TAKE    CPU_CRITICAL_ENTER
#define SOE_SEM_GIVE    CPU_CRITICAL_EXIT

SECTION_PLT_CODE static int32 _get_soe_save_index(int16 soe_type)
{
    if (soe_IsSoeBufferInitialed() && soe_IsSoeBufferEnabled(soe_type))
    {
        const TSOE_BUFFER *pBuffer = GetSoeBuffer(soe_type);
        return pBuffer->nIndex;
    }

    return 0;
}

//清空SOE发送 标志，所有报告类型的发送缓冲区清空
SECTION_PLT_CODE void soe_ClearSoeSendIndex(uint16 s_port)
{
    int16 soe_type;

    ASSERT_L4(s_port < S_TO_COUNT);

    for (soe_type=0;soe_type<SOE_TYPE_COUNT;soe_type++)
    {
        if(soe_IsSoeBufferEnabled(soe_type))
        {
            soe_SoeSendClear(s_port, soe_type);
        }
    }
}

//按SOE类型置全部已发送
SECTION_PLT_CODE void soe_SoeSendClear(uint16 s_port, int16 soe_type)
{
    int32 index;
    SOE_SEM_SR();
    ASSERT_L4(s_port < S_TO_COUNT);
    ASSERT_L4(soe_type >=0 && soe_type < SOE_TYPE_COUNT);

    SOE_SEM_TAKE();
    index = _get_soe_save_index(soe_type);
    if(index >=0)
    {
        SetSoeSendIdx(soe_type,s_port,index);
    }
    SOE_SEM_GIVE();
}

//重置SOE为未发送
SECTION_PLT_CODE void soe_ResetSoeSendIndex(uint16 s_port)
{
    int32 index;
    int32 count;
    int32 mmax_count;
    int16 soe_type;
    SOE_SEM_SR();

    ASSERT_L4(s_port < S_TO_COUNT);

    for (soe_type=0;soe_type<SOE_TYPE_COUNT;soe_type++)
    {
        if(soe_IsSoeBufferEnabled(soe_type))
        {
            int idx;
            SOE_SEM_TAKE();
            count = soe_GetSoeBufferCount(soe_type);
            index = _get_soe_save_index(soe_type);
            mmax_count = soe_GetSoeBufferMaxCount(soe_type);
            if (count >= 0 && index >= 0 && mmax_count > 0)
            {
                idx = index - count;
                if (idx < 0)
                {
                    idx += mmax_count;
                }

                SetSoeSendIdx(soe_type, s_port, idx);
            }
            SOE_SEM_GIVE();
        }
    }
}

//SOE待发送缓冲区是否为空
SECTION_PLT_CODE BOOL soe_SoeSendIsEmpty(uint16 s_port, int16 soe_type)
{
    BOOL bEmpty;

    if(!soe_IsSoeBufferEnabled(soe_type))
    {
        return PLT_TRUE;
    }

    SOE_SEM_SR();

    ASSERT_L2(s_port < S_TO_COUNT);
    ASSERT_L2(soe_type >=0 && soe_type < SOE_TYPE_COUNT);
    
    SOE_SEM_TAKE();
    bEmpty = BoolFrom(GetSoeSendIdx(soe_type, s_port) == _get_soe_save_index(soe_type));
    SOE_SEM_GIVE();
    
    return bEmpty;
}

//确认SOE发送
SECTION_PLT_CODE void soe_SoeSendAck(uint16 s_port, int16 soe_type, int32 count)
{
    SOE_SEM_SR();
    ASSERT_L4(s_port < S_TO_COUNT);
    ASSERT_L4(soe_type >=0 && soe_type < SOE_TYPE_COUNT);

    if(!soe_IsSoeBufferEnabled(soe_type))
    {
        return;
    }

    if(!soe_SoeSendIsEmpty(s_port, soe_type))
    {
        int send_index;
        int mmax_count = soe_GetSoeBufferMaxCount(soe_type);
        ASSERT_L2(mmax_count > 0);

        SOE_SEM_TAKE();
        send_index = GetSoeSendIdx(soe_type, s_port);
        SetSoeSendIdx(soe_type,s_port,((send_index + count) % mmax_count));
        SOE_SEM_GIVE();
    }
}

/**
 * @Function: soe_SoeSendGet
 * @Description: 获取待发送SOE 
 * @author zhongwei (2020/3/26 14:54:11)
 * 
 * @param s_port 发送端口
 * @param soe_type 
 * @param pCount 返回可发送个数
 * 
 * @return const void* 指向待发送SOE结构的头指针
 * 
*/
SECTION_PLT_CODE const void * soe_SoeSendGet(uint16 s_port, int16 soe_type, int32 * pCount)
{
    const void * pSOE = NULL;
    ASSERT_L4(s_port < S_TO_COUNT);
    ASSERT_L4(soe_type >=0 && soe_type < SOE_TYPE_COUNT);

    if(!soe_IsSoeBufferEnabled(soe_type) || soe_SoeSendIsEmpty(s_port, soe_type))
    {
        *pCount = 0;
        return NULL;
    }

    int32 nReserve = 0;     //剩余发送个数
    int32 index = _get_soe_save_index(soe_type);                //SOE保存指针
    int32 mmax_count = soe_GetSoeBufferMaxCount(soe_type);       //SOE最大个数
    int32 send_idx = GetSoeSendIdx(soe_type, s_port);          //发送指针

    //计算待发送SOE个数
    if (index > send_idx)
    {
        //剩余个数为Index-Send
        nReserve = index - send_idx;
    }
    else
    {
        //剩余个数为 NUM - Send
        nReserve = mmax_count - send_idx;
    }

    pSOE = soe_GetRawSoe(soe_type, send_idx);
    *pCount = nReserve;
    return pSOE;
}

