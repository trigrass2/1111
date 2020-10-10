/*
 * @Author: zhongwei
 * @Date: 2019/12/13 15:17:47
 * @Description: SOE处理接口
 * @File: plt_soe_handle.h
 *
*/
#ifndef SRC_PROJECT_PLT_SOE_HANDLE_H_
#define SRC_PROJECT_PLT_SOE_HANDLE_H_


//SOE存储结构
extern TSOE_BUFFER_SAVE g_SoeBufferSave;

#define GetSoeBufferSave()      (&g_SoeBufferSave)
#define GetSoeBuffer(soe_type)  (&GetSoeBufferSave()->soeBuffer[soe_type])


void PowerOn_InitSoeBuffer(void);

//SOE BUFFER是否已初始化
BOOL soe_IsSoeBufferInitialed(void);
//SOE缓存是否使能
BOOL soe_IsSoeBufferEnabled(int16 soe_type);

//获取SOE的个数
int32 soe_GetSoeBufferCount(int16 soe_type);
//获取SOE的可存储的最大个数
int32 soe_GetSoeBufferMaxCount(int16 soe_type);
//获取SOE指针
const void* soe_GetOneSoe(int16 soe_type, int index);
//获取SOE内容
void * soe_GetOneSoeEx(int16 soe_type, int index, void *pSoeBuf);
//获取根据raw_idx，获取SOE存储地址
void * soe_GetRawSoe(int16 soe_type, int raw_idx);
//添加一个SOE，直接添加到数据结构
void soe_AddOneSoe(int16 soe_type, const void * pSoeNew);


//添加一个新的SOE到记录中
void soe_AddNewTraceSoe(const TSOE_TRACE_STRUCT * pSoeNew);
//获取Trace SOE的总数
int32 soe_GetTraceSoeCount(void);
//根据index读取TRACE SOE
const TSOE_TRACE_STRUCT * soe_GetTraceSoe(int index);
TSOE_TRACE_STRUCT * soe_GetTraceSoeEx(int index, TSOE_TRACE_STRUCT * pSoeBuf);
//清除全部TRACE SOE
void soe_ClrTraceSoe(void);


//添加一个新的SOE到记录中
void soe_AddNewActionSoe(const TSOE_ACTION_STRUCT * pSoeNew);
//Action SOE的总数
int32 soe_GetActionSoeCount(void);
//根据index读取ACTION SOE
const TSOE_ACTION_STRUCT * soe_GetActionSoe(int index);
TSOE_ACTION_STRUCT * soe_GetActionSoeEx(int index, TSOE_ACTION_STRUCT * pSoeBuf);
//清除全部Action SOE
void soe_ClrActionSoe(void);


//添加一个新的SOE到记录中
void soe_AddNewRSignalSoe(const TSOE_RSIGNAL_STRUCT * pSoeNew);
//RSignal SOE的总数
int32 soe_GetRSignalSoeCount(void);
//根据index读取RSignal SOE
const TSOE_RSIGNAL_STRUCT * soe_GetRSignalSoe(int index);
TSOE_RSIGNAL_STRUCT * soe_GetRSignalSoeEx(int index, TSOE_RSIGNAL_STRUCT * pSoeBuf);
//清除全部RSignal SOE
void soe_ClrRSignalSoe(void);


//添加一个新的SOE到记录中
void soe_AddNewMeasureChgSoe(const TSOE_MEASURE_CHG_STRUCT * pSoeNew);
//遥测变化 SOE的总数
int32 soe_GetMeasureChgSoeCount(void);
//根据index读取遥测变化 SOE
const TSOE_MEASURE_CHG_STRUCT * soe_GetMeasureChgSoe(int index);
TSOE_MEASURE_CHG_STRUCT * soe_GetMeasureChgSoeEx(int index, TSOE_MEASURE_CHG_STRUCT * pSoeBuf);
//清除全部遥测变化 SOE
void soe_ClrMeasureChgSoe(void);

/************************************************************************/
//  报告生成接口
//
/************************************************************************/
//生成遥信变位SOE
void soe_GenRSignalSoe(int16 point_type, TID id, uint8 state, uint8 flag);
//生成动作报告SOE
void soe_GenActionSoe(TID action_id, uint8 state, uint16 phase);

#endif /* SRC_PROJECT_PLT_SOE_HANDLE_H_ */
