/*
 * @Author: zhongwei
 * @Date: 2020/3/26 10:13:10
 * @Description: 内部通讯配置信息文件
 * @File: icomm_config.h
 *
*/

#ifndef SRC_ICOMM_ICOMM_CONFIG_H_
#define SRC_ICOMM_ICOMM_CONFIG_H_

typedef struct{
    int16 soe_type;         //SOE报告类型
    int32 max_per_pkg;      //每包最大SOE数量 (0动态限制)
}TICOMM_SOE_CONFIG;

extern const TICOMM_SOE_CONFIG g_icomm_soe_config[];
extern const int g_icomm_soe_config_count;


typedef struct{
    int16 point_type;       //测点类型
    BOOL bChgSend;          //是否变化上送
    uint32 uPeriodMs;       //定时上送时间，0-不送 1-定时时间，ms
}TICOMM_POINT_CONFIG;

extern const TICOMM_POINT_CONFIG g_icomm_point_config[];
extern const int g_icomm_point_config_count;

#endif /* SRC_ICOMM_ICOMM_CONFIG_H_ */
