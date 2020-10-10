/*
 * @Author: zhongwei
 * @Date: 2020/4/3 17:46:03
 * @Description: 内部通讯基础类
 * @File: icomm_base_class.c
 *
*/

#include "plt_include.h"
#include "icomm_code.h"
#include "icomm_config.h"
#include "icomm_base_class.h"

/************************************************************************/
//  CInnerAssemble
//
/************************************************************************/
//构造函数
SECTION_PLT_CODE void CInnerAssemble_construct(CInnerAssemble * this)
{
    this->_pkg_head = NULL;
    this->_pkg_len = 0;
    this->_new_pkg = PLT_FALSE;
    this->_pkg_rx_len = this->_pkg_data_len = 0;
    this->_nCntRcvBytes = this->_nCntRcvPkgsErr = this->_nCntRcvPkgsOK = 0;

    this->OnRecieve = CInnerAssemble_OnRecieve;
    this->Clear = CInnerAssemble_Clear;
}

//析构函数
SECTION_PLT_CODE void CInnerAssemble_destruct(CInnerAssemble * this)
{

}

//清空报文
SECTION_PLT_CODE void CInnerAssemble_Clear(CInnerAssemble * this)
{
    this->_new_pkg = PLT_FALSE;
    this->_pkg_head = NULL;
}

//是否有新报文
SECTION_PLT_CODE BOOL CInnerAssemble_HasNewPackage(CInnerAssemble * this)
{
    return this->_new_pkg;
}

//获取报文长度
SECTION_PLT_CODE uint32 CInnerAssemble_GetRxCount(CInnerAssemble * this)
{
    if (this->_new_pkg && this->_pkg_head != NULL)
    {
        return this->_pkg_rx_len;
    }

    return 0;
}

//获取报文内容长度
SECTION_PLT_CODE uint32 CInnerAssemble_GetRxDataCount(CInnerAssemble * this)
{
    if (this->_new_pkg && this->_pkg_head != NULL)
    {
        return this->_pkg_data_len;
    }

    return 0;
}

//获取报文头指针
SECTION_PLT_CODE const uint8 * CInnerAssemble_GetPackage(CInnerAssemble * this)
{
    if (this->_new_pkg)
    {
        return this->_pkg_head;
    }

    return NULL;
}

//获取特征码
SECTION_PLT_CODE uint8 CInnerAssemble_GetCode(CInnerAssemble * this)
{
    if (this->_new_pkg && this->_pkg_head != NULL)
    {
        return this->_pkg_head[0];
    }
    else
    {
        return 0;
    }
}

//获取标志
SECTION_PLT_CODE uint8 CInnerAssemble_GetFlag(CInnerAssemble * this)
{
    if (this->_new_pkg && this->_pkg_head != NULL)
    {
        return this->_pkg_head[1];
    }
    else
    {
        return 0;
    }
}

//获取请求ID
SECTION_PLT_CODE uint16 CInnerAssemble_GetInvokeID(CInnerAssemble * this)
{
    if (this->_new_pkg && this->_pkg_head != NULL)
    {
        return MAKEWORD(this->_pkg_head[4], this->_pkg_head[5]);
    }
    else
    {
        return 0;
    }
}

//收到新的一帧报文
SECTION_PLT_CODE void CInnerAssemble_OnRecieve(CInnerAssemble * this, const uint8 * ptr, uint32 len)
{
    this->_new_pkg = PLT_FALSE;
    this->_pkg_head = NULL;
    this->_pkg_len = 0;

    this->_nCntRcvBytes += len;

    if (ptr != NULL && len > 0)
    {
        //进行报文解析
        if (len >= 6)
        {
            uint32 rx_len = MAKEWORD(ptr[2], ptr[3]);
            if (rx_len <= len)  //长度满足
            {
                this->_pkg_len = len;

                this->_pkg_rx_len = rx_len;               //报文实际长度
                this->_pkg_data_len = this->_pkg_rx_len - 6;    //报文数据长度

                this->_pkg_head = ptr;
                this->_new_pkg = PLT_TRUE;

                this->_nCntRcvPkgsOK++;
                return;             //接收到一封完整的报文
            }
        }
    }

    //报文错误
    this->_nCntRcvPkgsErr++; 
}

/************************************************************************/
//  CInnerSend
//
/************************************************************************/
//构造函数
SECTION_PLT_CODE void CInnerSend_construct(CInnerSend * this)
{
    this->_b_sending = PLT_FALSE;
    this->_pkg_head = NULL;
    this->_pkg_len = 0;
    this->_nSndBytes = this->_nSndPkgs = 0;
    this->canSend = CInnerSend_canSend;
    this->OnSend = CInnerSend_OnSend;
    this->send_to = CInnerSend_send_to;
    this->HD_CheckOK = CInnerSend_HD_CheckOK;
    this->HD_Reset = CInnerSend_HD_Reset;
}

//析构函数
SECTION_PLT_CODE void CInnerSend_destruct(CInnerSend * this)
{

}

//发送数据
SECTION_PLT_CODE void CInnerSend_Send(CInnerSend * this, const uint8 * pTxBuff, uint32 nTxLen)
{
    ASSERT_L3(!this->_b_sending);

    this->send_to(this, pTxBuff,nTxLen);    //直接发送，canSend在外部检查
}

//当前是否可发送数据
SECTION_PLT_CODE BOOL CInnerSend_canSend(CInnerSend * this)
{
//  if (!_b_sending)
//  {
//      return !mc_inner_send_buf_is_full();
//  }

    return PLT_FALSE;       //正在发送，当前不能继续发送
}

SECTION_PLT_CODE void CInnerSend_OnSend(CInnerSend * this)
{
    if (this->_b_sending)
    {
        if (this->canSend(this))
        {
            if (this->_pkg_head != NULL && this->_pkg_len > 0)
            {
                this->send_to(this, this->_pkg_head, this->_pkg_len);
            }

            this->_b_sending = PLT_FALSE; //发送了数据，或者当前数据无需发送
        }
    }
}

SECTION_PLT_CODE void CInnerSend_send_to(CInnerSend * this, const uint8 * pTxBuff, uint32 nTxLen)
{
//  if (!mc_inner_send_to_linux(pTxBuff, nTxLen))   //发送失败
//  {
//      LOGMSG("CInnerSend::send_to p=0x%x len=%d fail.\n\r", (int)pTxBuff, nTxLen,0,0,0,0);
//  }
}

SECTION_PLT_CODE BOOL CInnerSend_HD_CheckOK(struct tagInnerSend * this)
{
    return PLT_TRUE;
}

SECTION_PLT_CODE void CInnerSend_HD_Reset(struct tagInnerSend * this)
{
    
}
