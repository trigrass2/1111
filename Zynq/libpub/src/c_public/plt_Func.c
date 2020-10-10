/*
 * @Author: zhongwei
 * @Date: 2019-11-19 10:04:59
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-19 10:25:26
 * @Description: 公共函数定义
 * @FilePath: \ZynqBM\src\include\plt_Func.c
 */

#include "plt_inc_c.h"
#include "plt_Func.h"

SECTION_PLT_DATA
const uint32 TenMultiple[MAX_TEN_MULTIPLE+1]={
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000
};

/************************************************************************/
//      CRC16计算函数集
//      对应Modbus-CRC，不同于一般常用的CRC-CCITT
//
//      CCITT-CRC16：多项式是X16+X12+X5+1，对应的数字是0x11021，左移16位
//      本CRC16为ANSI CRC16：其多项式是X16+X15+X2+1，对应的数字是0x18005，左移16位
/************************************************************************/
SECTION_PLT_DATA 
const uint16 crc16_xtab[256] =
{
   0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
   0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
   0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
   0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
   0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
   0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
   0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
   0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
   0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
   0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
   0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
   0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
   0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
   0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
   0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
   0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
   0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
   0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
   0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
   0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
   0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
   0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
   0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
   0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
   0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
   0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
   0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
   0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
   0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
   0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
   0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
   0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};


///////////////////////////////////////////////////////////////
/// \brief     : CRC16校验
/// \param buf  缓冲区数据
//  \param len  长度
/// \return uint16 返回CRC校验值
///
/// \author    : 仲伟
/// \date      : 2006年8月21日
/// \note      : 查表法计算CRC16 ，速度较快
///////////////////////////////////////////////////////////////
SECTION_PLT_CODE 
uint16 crc16(const void * buf,uint32 len)
{
    uint16 crc = 0xffff;
    const uint8 * p=(const uint8 *)buf;
    while (len--)
        crc = (crc >> 8) ^ crc16_xtab[ (uint8) (crc ^ *p++) ];

    return crc;
}


///////////////////////////////////////////////////////////////
/// \brief     :  增强版本的CRC16校验
/// \param buf 输入缓冲区
//  \param len 长度
//  \param crc 输入/输出 CRC校验值
/// \return void
///
/// \author    : 仲伟
/// \date      : 2006年8月21日
/// \note      : 查表法计算CRC16 ，速度较快
///////////////////////////////////////////////////////////////
SECTION_PLT_CODE
void crc16_ex(const void * buf, uint32 len, uint16 * crc)
{
    const uint8 * p=(const uint8 *)buf;
    while (len--)
        *crc = ((*crc) >> 8) ^ crc16_xtab[ (uint8) ((*crc) ^ *p++) ];
}

//字符串是否为空
SECTION_PLT_CODE BOOL str_IsStringEmpty(const char * str)
{
    return ((str==NULL || str[0] == 0 || (str[0] == ' ' && str[1] == 0))?PLT_TRUE:PLT_FALSE);
}

//字符裁剪，去除开始的空格和末尾空格
SECTION_PLT_CODE char * str_strip(char * str)
{
    if (str != NULL)
    {
        int len = E_STRLEN(str);
        if (len > 0)
        {
            int b = 0;
            int i;
            //去除开头的空格
            for (i=0;i<len;i++)
            {
                if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r')
                {
                    b = i;
                    break;
                }
            }
            //去除末尾的空格
            for (i=(len-1);i>=b;i--)
            {
                if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')
                {
                    str[i] = 0;
                }
                else
                {
                    break;
                }
            }

            return &str[b];
        }
    }

    return str;
}

SECTION_PLT_CODE fp64 f_sqrt_fast(fp64 x0, fp64 x1)
{
    fp64 y0, y1, large, small, temp;
    fp64 tempLong;

    y0 = fabs(x0);
    y1 = fabs(x1);
    if(y0 < y1)
    {
        small = y0;
        large = y1;
    }
    else
    {
        small = y1;
        large = y0;
    }
  
    temp = large + small / 3.0;
    if(temp < 0.00001)
        return (0.0);
    tempLong = temp + (large * large + small * small) / temp;
    return tempLong / 2;
}
   
/**
 * @Function: f_InvSqrt
 * @Description: 快速开方算法 （卡马克）
 * @author zhongwei (2019/12/30 16:06:18)
 * 
 * @param f 
 * 
 * @return SECTION_PLT_CODE fp64 
 *  
 * 参考：https://blog.csdn.net/zmazon/article/details/8217866
 *  
*/
SECTION_PLT_CODE fp64 f_CarmackSqrt(fp64 f)
{
    fp32 x = f;
    fp32 xhalf = 0.5f * x;

    int32 i = *(int *)&x; // get bits for floating VALUE
    i = 0x5f375a86 - (i >> 1); // gives initial guess y0
    x = *(fp32 *)&i; // convert bits BACK to float
    x = x * (1.5f -   xhalf * x * x); // Newton step, repeating increases accuracy
    return 1.0/x;
}


//根据GIN得到组号
SECTION_PLT_CODE TGrpNo i_GetGinGrpNo(TGin gin)
{
    return LOBYTE(gin);
}

//根据GIN得到条目号
SECTION_PLT_CODE TItemNo i_GetGinItemNo(TGin gin)
{
    return HIBYTE(gin);
}

//根据组号条目号生成GIN号
SECTION_PLT_CODE TGin i_MakeGin(TGrpNo grp_no, TItemNo item_no)
{
    return MAKEWORD(grp_no, item_no);
}

//分配内存，并初始化为0
SECTION_PLT_CODE void * f_malloc(size_t __size)
{
    void * ptr = malloc(__size);
    if (ptr != NULL)
    {
        E_MEMSET(ptr, 0,  __size);
    }

    return ptr;
}

