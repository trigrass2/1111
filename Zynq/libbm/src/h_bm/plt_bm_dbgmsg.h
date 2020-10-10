/*
 * @Author: zhongwei
 * @Date: 2019/12/23 9:46:38
 * @Description: BM侧 DEBUG_MESSAGE定义
 * @File: plt_bm_dbgmsg.h
 *
*/
#ifndef SRC_H_PUBLIC_PLT_BM_DBGMSG_H_
#define SRC_H_PUBLIC_PLT_BM_DBGMSG_H_

//libbm版本
#define VERSION_LIB_BM         "1.00"
const char * get_version_libbm(void);               //版本
TFullDateTime get_buildtime_libbm(void);   //编译时间

enum E_DEBUG_MESSAGE
{
    DEBUG_MESSAGE_NONE              = 0,            //无输出
    DEBUG_MESSAGE_USE_PRINTF,                       //使用PRINTF输出
    DEBUG_MESSAGE_TASK,                             //任务信息
    DEBUG_MESSAGE_IRQ_NESTING,                      //中断嵌套情况
    DEBUG_MESSAGE_ON_ACTION_SOE,                    //动作报告时打印
    DEBUG_MESSAGE_ON_SIGNAL_SOE,                    //遥信报告时打印
    DEBUG_MESSAGE_ON_MEASURE_CHG_SOE,               //遥测变化报告时打印
    DEBUG_MESSAGE_LOAD_CONFIG_DATA,                 //初始配置数据
    DEBUG_MESSAGE_GOOSE_RCV,                        //Goose接收测试
    DEBUG_MESSAGE_SV_RCV,                           //SV接收测试
    DEBUG_MESSAGE_COUNT,                            //总个数
};

extern const char * _DebugMessageStr[];

#endif /* SRC_H_PUBLIC_PLT_BM_DBGMSG_H_ */
