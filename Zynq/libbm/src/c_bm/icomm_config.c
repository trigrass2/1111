/*
 * @Author: zhongwei
 * @Date: 2020/3/26 10:13:10
 * @Description: 内部通讯配置信息文件
 * @File: icomm_config.c
 *
*/

#include "plt_include.h"
#include "icomm_config.h"
#include "icomm_code.h"
/************************************************************************/
//  SOE发送配置
//  所有这里列出来的SOE要发送到Linux，未列出的不发送
//  发送优先级按照这里定义的顺序，前面的先发送
/************************************************************************/
SECTION_PLT_CONST const TICOMM_SOE_CONFIG g_icomm_soe_config[]=
{
//  soe_type                max_per_pkg
    {SOE_RSIGNAL,           (ICOMM_PKG_DATA_LENGTH - 8) / sizeof(TSOE_RSIGNAL_STRUCT)},
    {SOE_ACTION,            0 },
    {SOE_MEASURE_CHG,       0 },
    {SOE_PU_TRACE,          0 },
};

SECTION_PLT_CONST const int g_icomm_soe_config_count = sizeof(g_icomm_soe_config)/sizeof(TICOMM_SOE_CONFIG);

