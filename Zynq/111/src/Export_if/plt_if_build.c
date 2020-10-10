/*
 * @Author: zhongwei
 * @Date: 2019/12/15 14:24:50
 * @Description: 软件信息 BM侧
 * @File: plt_if_build.c
 * 
 * 本文件仅用于BM侧程序
*/
#include "plt_inc_c.h"

#ifdef _PROT_UNIT_

#include "Export_Inc.h"

NO_ELIMINATE SECTION_PLT_CONST const char *const szBuildDate = __DATE__;            //编译日期
NO_ELIMINATE SECTION_PLT_CONST const char * const szBuildTime = __TIME__;           //编译时间

//读取编译时间
SECTION_PLT_CODE const char * getBuildDate(void)
{
    return szBuildDate;
}

SECTION_PLT_CODE const char * getBuildTime(void)
{
    return szBuildTime;
}

#endif  //_PROT_UNIT_

