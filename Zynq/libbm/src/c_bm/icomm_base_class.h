/*
 * @Author: zhongwei
 * @Date: 2020/4/3 17:46:03
 * @Description: 内部通讯基础类
 * @File: icomm_base_class.h
 *
*/

#ifndef SRC_ICOMM_ICOMM_BASE_CLASS_H_
#define SRC_ICOMM_ICOMM_BASE_CLASS_H_

/************************************************************************/
//  CInnerAssemble
//
/************************************************************************/
typedef struct tagInnerAssemble{
    const uint8 * _pkg_head;      //指向报文头（CODE的位置）
    uint32        _pkg_len;       //报文长度
    BOOL          _new_pkg;       //是否有新报文
    uint32        _pkg_rx_len;    //报文内容长度（含报文头）
    uint32        _pkg_data_len;  //报文数据长度（不含报文头）
    /*接收计数*/
    uint32        _nCntRcvBytes;                 //总接收字节计数 
    uint32        _nCntRcvPkgsOK;                //接收包数（正确）
    uint32        _nCntRcvPkgsErr;               //接收包数（错误）
    void (*OnRecieve)(struct tagInnerAssemble * this, const uint8 * ptr, uint32 len);
    void (*Clear)(struct tagInnerAssemble * this);
}CInnerAssemble;

//构造函数
void CInnerAssemble_construct(CInnerAssemble * this);
//析构函数
void CInnerAssemble_destruct(CInnerAssemble * this);
//清空报文
void CInnerAssemble_Clear(CInnerAssemble * this);
//是否有新报文
BOOL CInnerAssemble_HasNewPackage(CInnerAssemble * this);
//获取报文长度
uint32 CInnerAssemble_GetRxCount(CInnerAssemble * this);
//获取报文内容长度
uint32 CInnerAssemble_GetRxDataCount(CInnerAssemble * this);
//获取报文头指针
const uint8 * CInnerAssemble_GetPackage(CInnerAssemble * this);
//获取特征码
uint8 CInnerAssemble_GetCode(CInnerAssemble * this);
//获取标志
uint8 CInnerAssemble_GetFlag(CInnerAssemble * this);
//获取请求ID
uint16 CInnerAssemble_GetInvokeID(CInnerAssemble * this);
//收到新的一帧报文
void CInnerAssemble_OnRecieve(CInnerAssemble * this, const uint8 * ptr, uint32 len);

/************************************************************************/
//  CInnerSend
//
/************************************************************************/
typedef struct tagInnerSend{
    BOOL        _b_sending;     //是否在发送数据
    const uint8 * _pkg_head;    //待发送报文头指针
    uint32      _pkg_len;       //待发送报文长度
    uint32      _nSndBytes;     //发送字节数
    uint32      _nSndPkgs;      //发送包数
    BOOL (*canSend)(struct tagInnerSend * this);
    void (*OnSend)(struct tagInnerSend * this);
    void (*send_to)(struct tagInnerSend * this, const uint8 * pTxBuff, uint32 nTxLen);
    BOOL (*HD_CheckOK)(struct tagInnerSend * this);
    void (*HD_Reset)(struct tagInnerSend * this);
}CInnerSend;

//析构函数
void CInnerSend_destruct(CInnerSend * this);
//析构函数
void CInnerSend_destruct(CInnerSend * this);
//发送数据
void CInnerSend_Send(CInnerSend * this, const uint8 * pTxBuff, uint32 nTxLen);
//当前是否可发送数据
BOOL CInnerSend_canSend(CInnerSend * this);
void CInnerSend_OnSend(CInnerSend * this);
void CInnerSend_send_to(CInnerSend * this, const uint8 * pTxBuff, uint32 nTxLen);
BOOL CInnerSend_HD_CheckOK(struct tagInnerSend * this);
void CInnerSend_HD_Reset(struct tagInnerSend * this);

/************************************************************************/
//  CInnerComm
//
/************************************************************************/
typedef struct tagInnerComm{
    CInnerAssemble * _inner_assemble;
    CInnerSend * _inner_send;
    uint16 _s_port;                     //soe端口
    BOOL _b_master;                     //通讯主还是从
    uint8 _tx_buffer[ICOMM_PKG_MAX_LENGTH * 2];
    int32 _tx_length;       //待发送报文长度
    uint8 _rx_buffer[ICOMM_PKG_MAX_LENGTH + 32];
    int32 _rx_length;
    BOOL _b_enabled;            //是否使能
    BOOL _b_connected;          //是否连接上
    uint32              _nRxTimer;         //接收超时计数
    uint32              _nTxTimer;         //发送超时计数
    uint32              _nHDPortTimer;     //通讯硬件超时计数

    uint32              _nDisConnTimer;    //通讯中断计数

    uint32              _nRxOver;          //接收超时时间 ms
    uint32              _nTxOver;          //发送超时 ms
    uint32              _nHDPortOver;      //硬件异常 ms
}CInnerComm;

#endif /* SRC_ICOMM_ICOMM_BASE_CLASS_H_ */
