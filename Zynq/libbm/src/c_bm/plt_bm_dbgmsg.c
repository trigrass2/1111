/*
 * @Author: zhongwei
 * @Date: 2019/12/23 9:47:50
 * @Description: BM侧调试打印指令
 * @File: plt_bm_dbgmsg.c
 *
*/
#include "plt_inc_c.h"
#include "plt_bm_dbgmsg.h"


SECTION_PLT_CONST const char *_DebugMessageStr[] = {
    "none",                                         //DEBUG_MESSAGE_NONE
    "debug message use printf",                     //DEBUG_MESSAGE_USE_PRINTF
    "task infomation",                              //DEBUG_MESSAGE_TASK
    "irq nesting",                                  //DEBUG_MESSAGE_IRQ_NESTING
    "On new action SOE",                            //DEBUG_MESSAGE_ON_ACTIN_SOE
    "On new signal SOE",                            //DEBUG_MESSAGE_ON_SIGNAL_SOE
    "On new measure chg SOE",                       //DEBUG_MESSAGE_ON_MEASURE_CHG_SOE
    "Load config data",                             //DEBUG_MESSAGE_LOAD_CONFIG_DATA
    "Goose Recieve test",                           //DEBUG_MESSAGE_GOOSE_RCV
    "SV Recieve test",                              //DEBUG_MESSAGE_SV_RCV
    NULL,                                           //最后以NULL结尾
};

//版本
SECTION_PLT_CODE const char * get_version_libbm(void)
{
    return VERSION_LIB_BM;
}

//编译时间
SECTION_PLT_CODE TFullDateTime get_buildtime_libbm(void)
{
    return ParseDateTimeString(__DATE__, __TIME__);
}

//打印libbm信息
SECTION_PLT_CODE void Print_libbm(void)
{
    c64_t ch;
    PRINTF("LibBM Version : %s\n\r", get_version_libbm());
    TFullDateTime fdt = get_buildtime_libbm();
    PRINTF("Build Time    : %s\n\r", str_GetFullDateTimeWithoutMsString(&fdt, ch));
}

