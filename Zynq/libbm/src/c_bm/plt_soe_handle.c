/*
 * @Author: zhongwei
 * @Date: 2019/12/13 15:18:18
 * @Description: SOE处理接口
 * @File: plt_soe_handle.c
 *
*/

#include "plt_include.h"
#include "plt_soe_handle.h"

/**
 * g_SoeBufferSave
 * @Description:  SOE数据保存
 * @author zhongwei (2019/12/13 15:13:36)
 * 
*/
SECTION_PLT_DATA TSOE_BUFFER_SAVE g_SoeBufferSave={0};

typedef TSOE_ACTION_STRUCT TSOE_ACTION_STRUCT_t[1024];
typedef TSOE_RSIGNAL_STRUCT TSOE_RSIGNAL_STRUCT_t[1024];
typedef TSOE_TRACE_STRUCT TSOE_TRACE_STRUCT_t[1024];
typedef TSOE_MEASURE_CHG_STRUCT TSOE_MEASURE_CHG_STRUCT_t[1024];

SECTION_PLT_DATA static const TSOE_ACTION_STRUCT_t * g_ptr_ActionSoeBuf = NULL;
SECTION_PLT_DATA static const TSOE_RSIGNAL_STRUCT_t * g_ptr_RSignalSoeBuf = NULL;
SECTION_PLT_DATA static const TSOE_TRACE_STRUCT_t * g_ptr_TraceSoeBuf = NULL;
SECTION_PLT_DATA static const TSOE_MEASURE_CHG_STRUCT_t * g_ptr_MeasureChgSoeBuf = NULL;

static void _InitSoeBuffer(int16 soe_type, size_t soe_size, int32 max_count)
{
    if (soe_type >=0 && soe_type < SOE_TYPE_COUNT)
    {
        TSOE_BUFFER *pBuffer = GetSoeBuffer(soe_type);
        size_t malloc_size = soe_size * (max_count + 8);
        pBuffer->nStructSize = soe_size;
        pBuffer->pStructBegin = pBuffer->pStructBeginRepet = E_MALLOC(malloc_size);
        ASSERT_L1(pBuffer->pStructBegin != NULL);
        pBuffer->nMaxCount = max_count;
        pBuffer->pStructEnd = pBuffer->pStructBegin + (malloc_size);

        pBuffer->bEnable = PLT_TRUE;    //使能
    }
}

/**
 * @Function: PowerOn_InitSoeBuffer
 * @Description: 上电初始化SOE缓冲区 ，必须在mem_Initial之后调用
 * @author zhongwei (2019/12/13 15:21:16)
 * 
 * @param void 
 * 
*/
SECTION_PLT_CODE void PowerOn_InitSoeBuffer(void)
{
    TSOE_BUFFER_SAVE * pSave = GetSoeBufferSave();
    E_MEMSET(pSave, 0, sizeof(TSOE_BUFFER_SAVE));

    //初始化动作报告
    _InitSoeBuffer(SOE_ACTION, sizeof(TSOE_ACTION_STRUCT), 1024);
    
    _InitSoeBuffer(SOE_RSIGNAL, sizeof(TSOE_RSIGNAL_STRUCT),1024);
    
    _InitSoeBuffer(SOE_PU_TRACE, sizeof(TSOE_TRACE_STRUCT), 256);

    _InitSoeBuffer(SOE_MEASURE_CHG, sizeof(TSOE_MEASURE_CHG_STRUCT), 128);
    
    g_ptr_ActionSoeBuf = (TSOE_ACTION_STRUCT_t *)GetSoeBuffer(SOE_ACTION)->pStructBegin;
    g_ptr_RSignalSoeBuf = (TSOE_RSIGNAL_STRUCT_t *)GetSoeBuffer(SOE_RSIGNAL)->pStructBegin;
    g_ptr_TraceSoeBuf = (TSOE_TRACE_STRUCT_t *)GetSoeBuffer(SOE_PU_TRACE)->pStructBegin;
    g_ptr_MeasureChgSoeBuf = (TSOE_MEASURE_CHG_STRUCT_t *)GetSoeBuffer(SOE_MEASURE_CHG)->pStructBegin;

    pSave->bInitialed = PLT_TRUE;
}

//SOE BUFFER是否已初始化
SECTION_PLT_CODE BOOL soe_IsSoeBufferInitialed(void)
{
    return (GetSoeBufferSave()->bInitialed);
}

//SOE缓存是否使能
SECTION_PLT_CODE BOOL soe_IsSoeBufferEnabled(int16 soe_type)
{
    if (soe_type >= 0 && soe_type < SOE_TYPE_COUNT)
    {
        TSOE_BUFFER *pBuffer = GetSoeBuffer(soe_type);
        return (pBuffer->bEnable && (pBuffer->nMaxCount > 0));
    }

    return PLT_FALSE;
}

//获取SOE的个数
SECTION_PLT_CODE int32 soe_GetSoeBufferCount(int16 soe_type)
{
    if (soe_IsSoeBufferInitialed() && soe_IsSoeBufferEnabled(soe_type))
    {
        const TSOE_BUFFER *pBuffer = GetSoeBuffer(soe_type);

        ASSERT_L4(pBuffer->pStructBegin != NULL);
        ASSERT_L2(pBuffer->nCount <= pBuffer->nMaxCount);

        return pBuffer->nCount;
    }

    return 0;
}

//获取SOE的可存储的最大个数
SECTION_PLT_CODE int32 soe_GetSoeBufferMaxCount(int16 soe_type)
{
    if (soe_IsSoeBufferInitialed() && soe_IsSoeBufferEnabled(soe_type))
    {
        const TSOE_BUFFER *pBuffer = GetSoeBuffer(soe_type);

        ASSERT_L4(pBuffer->pStructBegin != NULL);
        ASSERT_L2(pBuffer->nCount <= pBuffer->nMaxCount);

        return pBuffer->nMaxCount;
    }

    return 0;
}


//获取SOE指针
SECTION_PLT_CODE const void* soe_GetOneSoe(int16 soe_type, int index)
{
    if (soe_IsSoeBufferInitialed() && soe_IsSoeBufferEnabled(soe_type))
    {
        const TSOE_BUFFER *pBuffer = GetSoeBuffer(soe_type);

        int count = pBuffer->nCount;
        int idx = pBuffer->nIndex;

        if (index >= count || index < 0)
        {
            return NULL;        //无报告
        } 
        else
        {
            idx = idx - index - 1;      //报文所在位置
            if (idx < 0)
            {
                idx += pBuffer->nMaxCount;
            }
            ASSERT_L4(idx >= 0 && idx < pBuffer->nMaxCount);
            return soe_GetRawSoe(soe_type, idx);
        }

    }

    return NULL;
}

//获取SOE内容
SECTION_PLT_CODE void * soe_GetOneSoeEx(int16 soe_type, int index, void *pSoeBuf)
{
    if (soe_IsSoeBufferInitialed() && soe_IsSoeBufferEnabled(soe_type))
    {
        const void *pSoe = soe_GetOneSoe(soe_type, index);
        const TSOE_BUFFER *pBuffer = GetSoeBuffer(soe_type);
        if (pSoe != NULL)
        {
            E_MEMCPY(pSoeBuf, pSoe, pBuffer->nStructSize);
            return pSoeBuf;
        }
    }
    return NULL;
}

//获取根据raw_idx，获取SOE存储地址
/*
    raw_idx，物理位置
*/
SECTION_PLT_CODE void * soe_GetRawSoe(int16 soe_type, int raw_idx)
{
    if (soe_IsSoeBufferInitialed() && soe_IsSoeBufferEnabled(soe_type))
    {
        const TSOE_BUFFER *pBuffer = GetSoeBuffer(soe_type);

        ASSERT_L4(pBuffer->pStructBegin != NULL);
        ASSERT_L4(pBuffer->pStructBegin == pBuffer->pStructBeginRepet);
        ASSERT_L4(pBuffer->nIndex >= 0 && pBuffer->nIndex < pBuffer->nMaxCount);
        ASSERT_L4(pBuffer->nCount >= 0 && pBuffer->nCount <= pBuffer->nMaxCount);

        if (raw_idx >= 0 && raw_idx < pBuffer->nMaxCount)
        {
            void *ptr = (void *)((uint32)pBuffer->pStructBegin + raw_idx * pBuffer->nStructSize);
            ASSERT_L2(ptr >= pBuffer->pStructBegin && ptr < pBuffer->pStructEnd);
            return ptr;
        }
    }

    return NULL;
}

//清空SOE报告
void soe_ClrSoeBuffer(int16 soe_type)
{
    if (soe_IsSoeBufferInitialed() && soe_IsSoeBufferEnabled(soe_type))
    {
        TSOE_BUFFER *pBuffer = GetSoeBuffer(soe_type);
        CPU_SR_ALLOC();
        CPU_CRITICAL_ENTER();
        pBuffer->nCount = pBuffer->nIndex = 0;
        CPU_CRITICAL_EXIT();
    }
}

//添加一个SOE，直接添加到数据结构
SECTION_PLT_CODE void soe_AddOneSoe(int16 soe_type, const void * pSoeNew)
{
    ASSERT_L1(pSoeNew != NULL);
    if (IsTrue(soe_IsSoeBufferInitialed()))
    {
        if (soe_IsSoeBufferEnabled(soe_type))
        {
            TSOE_BUFFER *pBuffer = GetSoeBuffer(soe_type);
            int idx,count;
            void *pDestPtr = NULL;
            CPU_SR_ALLOC();
            /*
                  先置SOE的值，然后Index再增加1
                  最后计算Count的个数
            */

            CPU_CRITICAL_ENTER();

            idx = pBuffer->nIndex;
            count = pBuffer->nCount;


            pDestPtr = soe_GetRawSoe(soe_type, idx);    //复制的目标
            ASSERT_L2(pDestPtr != NULL);
            //SOE数组置值
            E_MEMCPY(pDestPtr, pSoeNew, pBuffer->nStructSize);

            //Index++
            idx++;
            if (idx >= pBuffer->nMaxCount)
            {
                idx = 0;
            }
            pBuffer->nIndex = idx;

            //Count++
            count++;
            if (count > pBuffer->nMaxCount)
            {
                count = pBuffer->nMaxCount;
            }

            pBuffer->nCount = count;

            CPU_CRITICAL_EXIT();
        }
        else
        {
             LOGMSG_L1("soe_AddOneSoe soe_type=%d, soe is disabled.\n", soe_type, 0, 0, 0, 0, 0);
        }
    } 
    else
    {
        LOGMSG_L1("soe_AddOneSoe soe_type=%d, buffer isn't initialed\n", soe_type, 0, 0, 0, 0, 0);
    }
}

/************************************************************************/
//  SOE_PU_TRACE
//
/************************************************************************/

//添加一个新的SOE到记录中
SECTION_PLT_CODE void soe_AddNewTraceSoe(const TSOE_TRACE_STRUCT * pSoeNew)
{
    //ASSERT_L2(sizeof(TSOE_TRACE_STRUCT) == GetSoeBuffer(SOE_PU_TRACE)->nStructSize);
    soe_AddOneSoe(SOE_PU_TRACE, pSoeNew);
}

//获取Trace SOE的总数
SECTION_PLT_CODE int32 soe_GetTraceSoeCount(void)
{
    return soe_GetSoeBufferCount(SOE_PU_TRACE);
}

//根据index读取TRACE SOE
SECTION_PLT_CODE const TSOE_TRACE_STRUCT * soe_GetTraceSoe(int index)
{
    return (const TSOE_TRACE_STRUCT *)soe_GetOneSoe(SOE_PU_TRACE, index);
}

SECTION_PLT_CODE TSOE_TRACE_STRUCT * soe_GetTraceSoeEx(int index, TSOE_TRACE_STRUCT * pSoeBuf)
{
    return (TSOE_TRACE_STRUCT *)soe_GetOneSoeEx(SOE_PU_TRACE, index, (void *)pSoeBuf);
}

//清除全部TRACE SOE
SECTION_PLT_CODE void soe_ClrTraceSoe(void)
{
    soe_ClrSoeBuffer(SOE_PU_TRACE);
}

/************************************************************************/
//  SOE_ACTION
//
/************************************************************************/

//添加一个新的SOE到记录中
SECTION_PLT_CODE void soe_AddNewActionSoe(const TSOE_ACTION_STRUCT * pSoeNew)
{
    ASSERT_L2(sizeof(TSOE_ACTION_STRUCT) == GetSoeBuffer(SOE_ACTION)->nStructSize);
    soe_AddOneSoe(SOE_ACTION, pSoeNew);
}

//Action SOE的总数
SECTION_PLT_CODE int32 soe_GetActionSoeCount(void)
{
    return soe_GetSoeBufferCount(SOE_ACTION);
}

//根据index读取ACTION SOE
SECTION_PLT_CODE const TSOE_ACTION_STRUCT * soe_GetActionSoe(int index)
{
    return (const TSOE_ACTION_STRUCT *)soe_GetOneSoe(SOE_ACTION, index);
}

SECTION_PLT_CODE TSOE_ACTION_STRUCT * soe_GetActionSoeEx(int index, TSOE_ACTION_STRUCT * pSoeBuf)
{
    return (TSOE_ACTION_STRUCT *)soe_GetOneSoeEx(SOE_ACTION, index, (void *)pSoeBuf);
}

//清除全部Action SOE
SECTION_PLT_CODE void soe_ClrActionSoe(void)
{
    soe_ClrSoeBuffer(SOE_ACTION);
}

/************************************************************************/
//  SOE_RSIGNAL
//
/************************************************************************/

//添加一个新的SOE到记录中
SECTION_PLT_CODE void soe_AddNewRSignalSoe(const TSOE_RSIGNAL_STRUCT * pSoeNew)
{
    ASSERT_L2(sizeof(TSOE_RSIGNAL_STRUCT) == GetSoeBuffer(SOE_RSIGNAL)->nStructSize);
    soe_AddOneSoe(SOE_RSIGNAL, pSoeNew);
}

//RSignal SOE的总数
SECTION_PLT_CODE int32 soe_GetRSignalSoeCount(void)
{
    return soe_GetSoeBufferCount(SOE_RSIGNAL);
}

//根据index读取RSignal SOE
SECTION_PLT_CODE const TSOE_RSIGNAL_STRUCT * soe_GetRSignalSoe(int index)
{
    return (const TSOE_RSIGNAL_STRUCT *)soe_GetOneSoe(SOE_RSIGNAL, index);
}

SECTION_PLT_CODE TSOE_RSIGNAL_STRUCT * soe_GetRSignalSoeEx(int index, TSOE_RSIGNAL_STRUCT * pSoeBuf)
{
    return (TSOE_RSIGNAL_STRUCT *)soe_GetOneSoeEx(SOE_RSIGNAL, index, (void *)pSoeBuf);
}

//清除全部RSignal SOE
SECTION_PLT_CODE void soe_ClrRSignalSoe(void)
{
    soe_ClrSoeBuffer(SOE_RSIGNAL);
}

/************************************************************************/
//  SOE_MEASURE_CHG
//
/************************************************************************/

//添加一个新的SOE到记录中
SECTION_PLT_CODE void soe_AddNewMeasureChgSoe(const TSOE_MEASURE_CHG_STRUCT * pSoeNew)
{
    ASSERT_L2(sizeof(TSOE_MEASURE_CHG_STRUCT) == GetSoeBuffer(SOE_MEASURE_CHG)->nStructSize);
    soe_AddOneSoe(SOE_MEASURE_CHG, pSoeNew);
    if (IsDebugMessage(DEBUG_MESSAGE_ON_MEASURE_CHG_SOE))
    {
        c64_t ch1;
        const char * szTime = str_GetFullDateTimeWithUsString(&pSoeNew->dtTime, ch1);
        LOGMSG("MeasureChgSOE %s\n\r", (int)szTime, 0, 0,0,0,0);
    }
}

//遥测变化 SOE的总数
SECTION_PLT_CODE int32 soe_GetMeasureChgSoeCount(void)
{
    return soe_GetSoeBufferCount(SOE_MEASURE_CHG);
}

//根据index读取遥测变化 SOE
SECTION_PLT_CODE const TSOE_MEASURE_CHG_STRUCT * soe_GetMeasureChgSoe(int index)
{
    return (const TSOE_MEASURE_CHG_STRUCT *)soe_GetOneSoe(SOE_MEASURE_CHG, index);
}

SECTION_PLT_CODE TSOE_MEASURE_CHG_STRUCT * soe_GetMeasureChgSoeEx(int index, TSOE_MEASURE_CHG_STRUCT * pSoeBuf)
{
    return (TSOE_MEASURE_CHG_STRUCT *)soe_GetOneSoeEx(SOE_MEASURE_CHG, index, (void *)pSoeBuf);
}

//清除全部遥测变化 SOE
SECTION_PLT_CODE void soe_ClrMeasureChgSoe(void)
{
    soe_ClrSoeBuffer(SOE_MEASURE_CHG);
}


/************************************************************************/
//  报告生成接口
//
/************************************************************************/
/**
 * @Function: soe_GenRSignalSoe
 * @Description: 生成遥信SOE 
 * @author zhongwei (2020/1/6 15:15:18)
 * 
 * @param point_type 遥信测点类型
 * @param id 测点ID
 * @param state 
 * @param flag 
 *  
 *  
 * state和flag的定义见plt_SoeType.h文件 
 *  
*/
void soe_GenRSignalSoe(int16 point_type, TID point_id, uint8 state, uint8 flag)
{
    TSOE_RSIGNAL_STRUCT soe;
    soe.dtTime = Now();
    soe.nPointType = point_type;
    soe.nPointID = point_id;
    soe.nRealIOID = -1;
    soe.state = state;
    soe.flag = flag;
    soe_AddNewRSignalSoe(&soe);
}

void soe_GenActionSoe(TID action_id, uint8 state, uint16 phase)
{
    TSOE_ACTION_STRUCT soe;
    soe.dtTime = Now();
    soe.nActionID = action_id;
    soe.state = state;
    soe.nPhase = phase;
    soe.nMeasValueCnt = 0;
    //fan、nof、qtime以后统一处理
//  soe.fan
//  soe.nof
//  soe.qtime
    //保存事件

    soe_AddNewActionSoe(&soe);

}
