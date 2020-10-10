/*
 * @Author: zhongwei
 * @Date: 2019/11/25 16:16:51
 * @Description: 平台裸核使用的数据结构定义
 * @File: plt_bm_struct.h
 *
*/
#ifndef SRC_INCLUDE_PLT_STRUCT_H_
#define SRC_INCLUDE_PLT_STRUCT_H_

#ifdef _PROT_UNIT_

//由于SOE可能需要同时往多个CPU发送数据，因此需要特殊处理
enum E_S_TO_TYPE{
    S_TO_LINUX, 
    S_TO_OTHER,
    S_TO_COUNT,
};

//BM侧报告的存储格式
typedef struct {
    BOOL bEnable;             //是否使能(在BM侧，仅遥信、动作和TRACE使能)
    int32 nMaxCount;          //最大个数 pStruct 分配数组的个数
    int32 nStructSize;        //SOE数据结构大小(sizeof(TSOE_RSIGNAL_STRUCT)等)
    int32 nCount;             //当前保存的SOE个数

    int32 nIndex;             //Index指针

    void *pStructBegin;       //指向特定的结构数组起始地址  实际占用空间为 (nStructSize * nMaxCount)
    void *pStructEnd;         //指向特定的结构数组结尾地址

    uint32 reserve;            //预留1个uint32结构(对于遥信报告，保存arm_no序号信息)
    void *pStructBeginRepet;              //重复保存起始地址  pStructBegin
}TSOE_BUFFER;

//SOE发送标识
typedef struct{
    int32 nSendIdx[SOE_TYPE_COUNT][S_TO_COUNT];
}TSOE_SEND;

typedef struct {
    BOOL                    bInitialed;                 //是否已初始化完成
    TSOE_SEND               soeSend;
    TSOE_BUFFER             soeBuffer[SOE_TYPE_COUNT];
}TSOE_BUFFER_SAVE;

#endif  //_PROT_UNIT_

#endif /* SRC_INCLUDE_PLT_STRUCT_H_ */
