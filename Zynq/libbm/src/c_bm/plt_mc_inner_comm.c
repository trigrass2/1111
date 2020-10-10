/*
 * @Author: zhongwei
 * @Date: 2019/11/28 14:52:08
 * @Description: 核间通讯处理接口
 * @File: plt_mc_inner_comm.c
 *
*/


#include "plt_include.h"
#include "plt_mc_inner_comm.h"

/**
 * @Function: PowerOn_Init_McInnerComm
 * @Description: 初始化 
 * @author zhongwei (2019/11/28 14:56:48)
 * 
 * @param void 
 * 
*/
SECTION_PLT_CODE void PowerOn_Init_McInnerComm(void)
{
    TMC_INNER_COMM_STRUCT *pInnerSend = GetSharedBMInnerComm();
    if (pInnerSend != NULL && IsSharedBMMemValid())
    {
        pInnerSend->nPkgBufLen = MC_INNER_COMM_PKG_BUF_NUM;     //写一次长度
    }
    else
    {
        PRINTF_WITH_LOG("TMC_INNER_COMM_STRUCT send isn't available!\n\r",0);
    }

    //检查Linux侧接收缓冲区
    TMC_INNER_COMM_STRUCT *pInnerRcv = GetSharedLinuxInnerComm();


	
	
	TSHARE_MEMORY_STRUCT *pSharedMemory = GetSharedMemory();
	TSHARED_LINUX_STRUCT *pSharedLinux = GetSharedLinuxMemory();
	
	TSHARED_BAREMETAL_STRUCT *pSharedBM = GetSharedBMMemory();
	
	PRINTF_WITH_LOG("pSharedLinux %08x  pSharedBM %08x  \n\r",pSharedLinux,pSharedBM);

    if (IsSharedLinuxMemValid() && pInnerRcv != NULL && pInnerRcv->nPkgBufLen== MC_INNER_COMM_PKG_BUF_NUM)
    {
        
    }
    else
    {
        PRINTF_WITH_LOG("TMC_INNER_COMM_STRUCT rcv isn't available!\n\r",0);
    }
}

/**
 * @Function: mc_inner_ClearSendBuf
 * @Description: 清空发送缓冲区 
 * @author zhongwei (2019/11/28 14:53:20)
 * 
 * @param void 
 * 
*/
SECTION_PLT_CODE void mc_inner_ClearSendBuf(void)
{
    TMC_INNER_COMM_STRUCT *pInnerSend = GetSharedBMInnerComm();
    if (IsSharedBMMemValid())
    {
        int i;
        pInnerSend->nBegin = pInnerSend->nEnd = 0;
        for (i=0;i<MC_INNER_COMM_PKG_BUF_NUM;i++)
        {
            pInnerSend->pkgBuf[i].len = 0;      //清空数据
        }
    }
}

//缓冲区是否满
SECTION_PLT_CODE BOOL mc_inner_send_buf_is_full(void)
{
    TMC_INNER_COMM_STRUCT *pInnerSend = GetSharedBMInnerComm();
    return BoolFrom(pInnerSend->nBegin == (1+pInnerSend->nEnd) ||
            (pInnerSend->nBegin == 0 && pInnerSend->nEnd == MC_INNER_COMM_PKG_BUF_NUM));
}

//缓冲区是否为空
SECTION_PLT_CODE BOOL mc_inner_send_buf_is_empty(void)
{
    TMC_INNER_COMM_STRUCT *pInnerSend = GetSharedBMInnerComm();
    return BoolFrom(pInnerSend->nBegin == pInnerSend->nEnd);
}

/**
 * @Function: mc_inner_send_to_linux
 * @Description: 发送报文到Linux的接口 
 * @author zhongwei (2019/11/28 15:19:10)
 * 
 * @param data 
 * @param len 
 * 
 * @return SECTION_PLT_CODE BOOL 
 * 
*/
SECTION_PLT_CODE BOOL mc_inner_send_to_linux(const uint8 * data, int32 len)
{
    TMC_INNER_COMM_STRUCT *pInnerSend = GetSharedBMInnerComm();
    ASSERT_L1(data != NULL && len > 0);
    if (len > MC_INNER_COMM_PKG_LEN)    //超长，不发送
    {
        LOGMSG_L2("mc_inner_send_to_linux len=%d, too long!\n\r", len,0,0,0,0,0);
        return PLT_FALSE;
    }

    if (IsSharedBMMemValid())
    {
        int32 nEnd;
        if (mc_inner_send_buf_is_full())
        {
            LOGMSG_L1("mc_inner_buf_is_full, send fail.\n\r",0,0,0,0,0,0);
            return PLT_FALSE;
        }

        //检查缓冲区是否正常
        if (pInnerSend->nEnd < 0 || pInnerSend->nEnd >= MC_INNER_COMM_PKG_BUF_NUM)
        {
            mc_inner_ClearSendBuf();
            LOGMSG_L2("mc_inner_send_to_linux buf end error, clean all!\n\r",0,0,0,0,0,0);
        }

        nEnd = pInnerSend->nEnd;
        E_MEMCPY(pInnerSend->pkgBuf[nEnd].data, data, len);
        pInnerSend->pkgBuf[nEnd].len = len;
        nEnd++;
        if (nEnd >= MC_INNER_COMM_PKG_BUF_NUM)
        {
            nEnd = 0;
        }

        pInnerSend->nEnd = nEnd;

        return PLT_TRUE;
    }

    return PLT_FALSE;
}

//接收缓冲区是否为空
SECTION_PLT_CODE BOOL mc_inner_rcv_buf_is_empty(void)
{
    TMC_INNER_COMM_STRUCT *pInnerRcv = GetSharedLinuxInnerComm();
    if (IsSharedLinuxMemValid())
    {
        return BoolFrom(pInnerRcv->nBegin == pInnerRcv->nEnd);
    }

    return PLT_TRUE;    //其他清空下均为空
}

/**
 * @Function: mc_inner_rcv_from_linux
 * @Description: 从Linux获取通讯数据的接口 
 * @author zhongwei (2019/11/28 15:41:09)
 * 
 * @param data 
 * @param max_len 
 * 
 * @return SECTION_PLT_CODE int32 
 * 
*/
SECTION_PLT_CODE int32 mc_inner_rcv_from_linux(uint8 * data, int32 max_len)
{
    TMC_INNER_COMM_STRUCT *pInnerRcv = GetSharedLinuxInnerComm();
    if (IsFalse(mc_inner_rcv_buf_is_empty()))
    {
        int32 nRcvLen = 0;
        int32 nBegin = pInnerRcv->nBegin;
        const TMC_INNER_COMM_PKG *pPkg = &pInnerRcv->pkgBuf[nBegin];

        if (pPkg->len <= max_len)
        {
            nRcvLen = pPkg->len;
            E_MEMCPY(data, pPkg->data, nRcvLen);
        }

        nBegin++;
        if (nBegin >= MC_INNER_COMM_PKG_BUF_NUM)
        {
            nBegin = 0;
        }

        pInnerRcv->nBegin = nBegin;

        return nRcvLen;
    }

    return 0;
}

/**
 * @Function: mc_GetLinuxCommandLine
 * @Description: 获取Linux发送到BM侧的命令行 
 * @author zhongwei (2020/2/10 15:07:50)
 * 
 * @param cmd 
 * 
 * @return SECTION_PLT_CODE BOOL 
 * 
*/
SECTION_PLT_CODE BOOL mc_GetLinuxCommandLine(c128_t cmd)
{
    if (IsSharedLinuxMemValid())
    {
        TMC_COMMAND_LINE *pCmdLine = GetSharedLinuxCommandLine();
        if (IsTrue(pCmdLine->bCmd))
        {
            if (pCmdLine->flag_begin == FLAG_SHARED_MEMORY_VALID &&
                pCmdLine->flag_end == FLAG_SHARED_MEMORY_VALID)
            {
                E_STRNCPY(cmd, pCmdLine->str_cmd, sizeof(c128_t));
                pCmdLine->bCmd = PLT_FALSE;
                return PLT_TRUE;
            }
        }
    }

    return PLT_FALSE;
}

