/*
 * @Author: zhongwei
 * @Date: 2019/12/20 19:24:41
 * @Description: 字符串显示处理
 * @File: plt_viewstring.c
 *
*/

#include "plt_inc_c.h"
#include "plt_viewstring.h"

//字符串资源是否为空
SECTION_PLT_CODE BOOL str_IsStringResEmpty(TID nStrID)
{
    if(nStrID >=0)
    {
        const char * sz = str_GetStringLocal(nStrID);
        return str_IsStringEmpty(sz);
    }
    else
    {
        return PLT_TRUE;
    }
}

//软件版本字符串
SECTION_PLT_CODE const char * str_GetSoftVersionString(int32 ver, c32_t ch)
{
    E_SNPRINTF(ch,sizeof(c32_t), "V%d.%02d", ver / 100, ver % 100);
    return ch;
}

typedef struct{
    int32 si;
    const char * sz;
}TSI_DEFINE;

SECTION_PLT_CONST static const TSI_DEFINE _si_unit[]={
    { SIUNIT_NONE, "None" },    //SIUNIT_NONE=0
    { SIUNIT_EMPTY, "" },   //SIUNIT_EMPTY=1
    { SIUNIT_m, "m" },  //SIUNIT_m=2
    { SIUNIT_kg, "kg" },    //SIUNIT_kg=3
    { SIUNIT_s, "s" },  //SIUNIT_s=4
    { SIUNIT_A, "A" },  //SIUNIT_A=5
    { SIUNIT_K, "K" },  //SIUNIT_K=6
    { SIUNIT_mol, "mol" }, //SIUNIT_mol=7
    { SIUNIT_cd, "cd" },    //SIUNIT_cd=8
    { SIUNIT_deg, "deg" },  //SIUNIT_deg=9
    { SIUNIT_rad, "rad" },  //SIUNIT_rad=10
    { SIUNIT_sr, "sr" },    //SIUNIT_sr=11
    { 0, NULL },
    { 0, NULL },
    { 0, NULL },
    { 0, NULL },
    { 0, NULL },
    { 0, NULL },
    { 0, NULL },
    { 0, NULL },
    { 0, NULL },
    { SIUNIT_Gy, "Gy" },    //SIUNIT_Gy=21
    { SIUNIT_q, "q" },      //SIUNIT_q=22
    { SIUNIT_OC, "℃" },   //SIUNIT_OC=23
    { SIUNIT_Sv, "Sv" },    //SIUNIT_Sv=24
    { SIUNIT_F, "F" },  //SIUNIT_F=25
    { SIUNIT_C, "C" },  //SIUNIT_C=26
    { SIUNIT_S, "S" },  //SIUNIT_S=27
    { SIUNIT_H, "H" },  //SIUNIT_H=28
    { SIUNIT_V, "V" },  //SIUNIT_V=29
    { SIUNIT_ohm, "ohm" },  //SIUNIT_ohm=30
    { SIUNIT_J, "J" },  //SIUNIT_J=31
    { SIUNIT_N, "N" },  //SIUNIT_N=32
    { SIUNIT_Hz, "Hz" },    //SIUNIT_Hz=33
    { SIUNIT_lx, "lx" },    //SIUNIT_lx=34
    { SIUNIT_Lm, "Lm" },    //SIUNIT_Lm=35
    { SIUNIT_Wb, "Wb" },    //SIUNIT_Wb=36
    { SIUNIT_T, "T" },  //SIUNIT_T=37
    { SIUNIT_W, "W" },  //SIUNIT_W=38
    { SIUNIT_Pa, "Pa" },    //SIUNIT_Pa=39
    { 0, NULL },
    { SIUNIT_m2, "m2" },    //SIUNIT_m2=41
    { SIUNIT_m3, "m3" },    //SIUNIT_m3=42
    { SIUNIT_m_s, "m/s" },  //SIUNIT_m_s=43
    { SIUNIT_m_s2, "m/s2" },    //SIUNIT_m_s2=44
    { SIUNIT_m3_s, "m3/s" },    //SIUNIT_m3_s=45
    { SIUNIT_m_m3, "m/m3" },    //SIUNIT_m_m3=46
    { SIUNIT_M, "M" },  //SIUNIT_M=47
    { SIUNIT_kg_m3, "kg/m3" },  //SIUNIT_kg_m3=48
    { SIUNIT_m2_s, "m2/s" },    //SIUNIT_m2_s=49
    { SIUNIT_W_mK, "W/m K" },   //SIUNIT_W_mK=50
    { SIUNIT_J_K, "J/K" },  //SIUNIT_J_K=51
    { SIUNIT_ppm, "ppm" },  //SIUNIT_ppm=52
    { SIUNIT_1_s, "1/s" },  //SIUNIT_1_s=53
    { SIUNIT_rad_s, "rad/s" },  //SIUNIT_rad_s=54
    { SIUNIT_W_m2, "W/m2" },    //SIUNIT_W_m2=55
    { SIUNIT_J_m2, "J/m2" },    //SIUNIT_J_m2=56
    { SIUNIT_S_m, "S/m" },  //SIUNIT_S_m=57
    { SIUNIT_K_s, "K/s" },  //SIUNIT_K_s=58
    { SIUNIT_Pa_s, "Pa/s" },    //SIUNIT_Pa_s=59
    { SIUNIT_J_KgK, "J/kg K" }, //SIUNIT_J_KgK=60
    { SIUNIT_VA, "VA" },    //SIUNIT_VA=61
    { SIUNIT_Watts, "Watts" },  //SIUNIT_Watts=62
    { SIUNIT_VAr, "VAr" },  //SIUNIT_VAr=63
    { SIUNIT_phi, "phi" },  //SIUNIT_phi=64
    { SIUNIT_cos_phi, "cos_phi" },  //SIUNIT_cos_phi=65
    { SIUNIT_Vs, "Vs" },    //SIUNIT_Vs=66
    { SIUNIT_V2, "V2" },    //SIUNIT_V2=67
    { SIUNIT_As, "As" },    //SIUNIT_As=68
    { SIUNIT_A2, "A2" },    //SIUNIT_A2=69
    { SIUNIT_A2t, "A2t" },  //SIUNIT_A2t=70
    { SIUNIT_VAh, "VAh" },  //SIUNIT_VAh=71
    { SIUNIT_Wh, "Wh" },    //SIUNIT_Wh=72
    { SIUNIT_VArh, "VArh" },    //SIUNIT_VArh=73
    { SIUNIT_V_Hz, "V/Hz" },    //SIUNIT_V_Hz=74
    { SIUNIT_Hz_s, "Hz/s" },    //SIUNIT_Hz_s=75
    { SIUNIT_char, "char" },    //SIUNIT_char=76
    { SIUNIT_char_s, "char/s" },    //SIUNIT_char_s=77
    { SIUNIT_kgm2, "kgm2" },    //SIUNIT_kgm2=78
    { SIUNIT_dB, "dB" },    //SIUNIT_dB=79
    { SIUNIT_J_Wh, "J_Wh" },    //SIUNIT_J_Wh=80
    { SIUNIT_W_s, "W/s" },  //SIUNIT_W_s=81
    { SIUNIT_l_s, "l/s" },  //SIUNIT_l_s=82
    { SIUNIT_dBm, "dBm" },  //SIUNIT_dBm=83
};

SECTION_PLT_CONST static const int32 _si_unit_count = sizeof(_si_unit)/sizeof(TSI_DEFINE);
SECTION_PLT_CONST static const TSI_DEFINE * const _si_unit_ptr_0 = &_si_unit[0];            //0值对应的起始点

SECTION_PLT_CONST static const TSI_DEFINE _si_multiple[]={
    { SIMULTIPLE_yocto, "y" },  //SIMULTIPLE_yocto=-24
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_zepto, "z" },  //SIMULTIPLE_zepto=-21
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_atto, "a" },   //SIMULTIPLE_atto=-18
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_femto, "f" },  //SIMULTIPLE_femto=-15
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_piko, "p" },   //SIMULTIPLE_piko=-12
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_nano, "n" },   //SIMULTIPLE_nano=-9
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_micro, "μ" }, //SIMULTIPLE_micro=-6
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_milli, "m" },  //SIMULTIPLE_milli=-3
    { SIMULTIPLE_centi, "c" },  //SIMULTIPLE_centi=-2
    { SIMULTIPLE_deci, "d" },   //SIMULTIPLE_deci=-1
    { SIMULTIPLE_none, "None" },    //SIMULTIPLE_none=0
    { SIMULTIPLE_deka, "da" },  //SIMULTIPLE_deka=1
    { SIMULTIPLE_hekto, "h" },  //SIMULTIPLE_hekto=2
    { SIMULTIPLE_kilo, "k" },   //SIMULTIPLE_kilo=3
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_mega, "M" },   //SIMULTIPLE_mega=6
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_giga, "G" },   //SIMULTIPLE_giga=9
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_tera, "T" },   //SIMULTIPLE_tera=12
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_petra, "P" },  //SIMULTIPLE_petra=15
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_exa, "E" },    //SIMULTIPLE_exa=18
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_zetta, "Z" },  //SIMULTIPLE_zetta=21
    { 0, NULL },
    { 0, NULL },
    { SIMULTIPLE_yotta, "Y" },  //SIMULTIPLE_yotta=24
};

SECTION_PLT_CONST static const int32 _si_multiple_count = sizeof(_si_multiple)/sizeof(TSI_DEFINE);
SECTION_PLT_CONST static const TSI_DEFINE * const _si_multiple_ptr_0 = &_si_multiple[24];           //0值对应的起始点


//读61850量纲字符串
SECTION_PLT_CODE const char * str_GetSIUnit_String(int8 nSIUnit)
{
    int32 nMax = (_si_unit_ptr_0 - _si_unit) + _si_unit_count;
    int32 nMin = 0 - (_si_unit_ptr_0 - _si_unit);
    if(nSIUnit >= nMin && nSIUnit <= nMax)
    {
        const char * szUnit = _si_unit_ptr_0[nSIUnit].sz;
        if(szUnit != NULL)
        {
            return szUnit;
        }
    }

    return "N/A";
}
 
//读61850量纲倍数
SECTION_PLT_CODE const char * str_GetSIMultiple_String(int8 nSIMultiple)
{
    int32 nMax = (_si_multiple_ptr_0 - _si_multiple) + _si_multiple_count;
    int32 nMin = 0 - (_si_multiple_ptr_0 - _si_multiple);
    if(nSIMultiple >= nMin && nSIMultiple < nMax)
    {
        const char * szUnit = _si_multiple_ptr_0[nSIMultiple].sz;
        if(szUnit != NULL)
        {
            return szUnit;
        }
    }

    return "N/A";
}

//GIN字符串  XXXX
SECTION_PLT_CODE const char * str_GetGinString(TGin gin, c32_t buf)
{
    E_SPRINTF(buf, "%04x", gin);
    return buf;
}

//品质字符串
SECTION_PLT_CODE const char * str_GetCommQualityString(TQuality quality, c32_t buf)
{
    E_SPRINTF(buf, "%d", quality);
    return buf;
}

//检修字符串
SECTION_PLT_CODE const char * str_GetTestQualityString(TQuality test, c32_t buf)
{
    E_SPRINTF(buf, "%d", test);
    return buf;
}

//越限字符串
SECTION_PLT_CODE const char * str_GetOverQualityString(TQuality over, c32_t buf)
{
    E_SPRINTF(buf, "%d", over);
    return buf;
}

//获取BOOL变量显示字符串
SECTION_PLT_CODE const char * str_GetBoolViewString(BOOL b, c32_t ch)
{
    if (IsTrue(b))
    {
        return "1";
    }
    else if (IsFalse(b))
    {
        return "0";
    }
    else
    {
        return "--";
    }
}

//获取双点信息值
SECTION_PLT_CODE const char * str_GetDualPtViewString(int32 i, c32_t ch)
{
    E_SPRINTF(ch,"%d", i);
    return ch;
}

//获取整数显示字符串
SECTION_PLT_CODE const char * str_GetInt32ViewString(int32 i32, c32_t ch)
{
    E_SNPRINTF(ch, sizeof(c32_t), "%d", i32);
    return ch;
}

//获取无符号整数显示字符串
SECTION_PLT_CODE const char * str_GetUint32ViewString(uint32 u32, c32_t ch)
{
    E_SNPRINTF(ch, sizeof(c32_t), "%u", u32);
    return ch;
}

//获取整数显示字符串
SECTION_PLT_CODE const char * str_GetInt64ViewString(int64 i64, c32_t ch)
{
    E_SNPRINTF(ch, sizeof(c32_t), "%lld", i64);
    return ch;
}

//获取无符号整数显示字符串
SECTION_PLT_CODE const char * str_GetUint64ViewString(uint64 u64, c32_t ch)
{
    E_SNPRINTF(ch, sizeof(c32_t), "%llu", u64);
    return ch;
}

//显示16进制（32位整数）
SECTION_PLT_CODE const char * str_GetHex32ViewString(uint32 hex, c32_t ch)
{
    E_SNPRINTF(ch, sizeof(c32_t), "0x%x", hex);
    return ch;
}

//显示16进制（64位整数）
SECTION_PLT_CODE const char * str_GetHex64ViewString(uint64 hex, c32_t ch)
{
    E_SNPRINTF(ch, sizeof(c32_t), "0x%llx", hex);
    return ch;
}

//获取浮点数fp64显示字符串
SECTION_PLT_CODE const char * str_Getfp32ViewString(fp32 f32, c64_t ch)
{
    E_SNPRINTF(ch, sizeof(c64_t), "%.3f", f32);
    return ch;
}

//获取浮点数fp64显示字符串，指定显示小数位数
SECTION_PLT_CODE const char * str_Getfp32ViewStringEx(fp32 f32,int8 nFraction, c64_t ch)
{
    char tmp[16];
    E_SPRINTF(tmp, "%%.%df", nFraction);
    E_SNPRINTF(ch, sizeof(c64_t), tmp, f32);
    return ch;
}


//获取浮点数fp64显示字符串
SECTION_PLT_CODE const char * str_Getfp64ViewString(fp64 f64, c64_t ch)
{
    E_SNPRINTF(ch, sizeof(c64_t), "%.6f", f64);
    return ch;
}

//获取浮点数fp64显示字符串，指定显示小数位数
SECTION_PLT_CODE const char * str_Getfp64ViewStringEx(fp64 f64,int8 nFraction, c64_t ch)
{
    char tmp[16];
    E_SPRINTF(tmp, "%%.%df", nFraction);
    E_SNPRINTF(ch, sizeof(c64_t), tmp, f64);
    return ch;
}

//获得TVAR_DATA显示字符串
SECTION_PLT_CODE const char * str_GetTVAR_DATAViewString(TVAR_DATA var, c64_t ch)
{
    switch (var.type)
    {
    case VAR_DATA_TYPE_NONE:
        return "--";
    case VAR_DATA_TYPE_BOOL:
        return str_GetBoolViewString(var.data.b, ch);
    case VAR_DATA_TYPE_INT32:
        return str_GetInt32ViewString(var.data.i_32, ch);
    case VAR_DATA_TYPE_INT64:
        return str_GetInt64ViewString(var.data.i_64, ch);
    case VAR_DATA_TYPE_FP64:
        return str_Getfp64ViewString(var.data.f_64, ch);
    default:
        return "--";
    }
}

//获得TVAR_DATA显示字符串，指定显示的小数位数
SECTION_PLT_CODE const char * str_GetTVAR_DATAViewStringEx(TVAR_DATA var,int8 nFraction, c64_t ch)
{
    switch (var.type)
    {
    case VAR_DATA_TYPE_NONE:
        return "--";
    case VAR_DATA_TYPE_BOOL:
        return str_GetBoolViewString(var.data.b, ch);
    case VAR_DATA_TYPE_INT32:
        return str_Int32FloatToString(var.data.i_32, nFraction, ch);
    case VAR_DATA_TYPE_INT64:
        return str_Int64FloatToString(var.data.i_64, nFraction, ch);
    case VAR_DATA_TYPE_FP64:
        return str_Getfp64ViewStringEx(var.data.f_64, nFraction, ch);
    default:
        return "--";
    }
}

//有符号整数转换为浮点字符串(带小数位数)
SECTION_PLT_CODE const char * str_Int32FloatToString(int32 value,int8 nFraction, c32_t buf)
{
    int32 max_len = sizeof(c32_t);
    char ch[32];
    if (value >= 0)
    {
        // 有小数部分
        if (nFraction > 0)
        {
            E_SPRINTF(ch, "%%d.%%0%dd", nFraction);
            E_SNPRINTF(buf, max_len, ch, value / TenMultiple[nFraction], value % TenMultiple[nFraction]);
        }
        else
        {
            E_SNPRINTF(buf, max_len, "%d", value);
        }
    }
    else
    { // 负数处理

        uint32 u32 = f_abs(value);
        // 有小数部分
        if (nFraction > 0)
        {
            E_SPRINTF(ch, "-%%u.%%0%du", nFraction);
            E_SNPRINTF(buf, max_len, ch, u32 / TenMultiple[nFraction], u32 % TenMultiple[nFraction]);
        }
        else
        {
            E_SNPRINTF(buf, max_len, "-%u", u32);
        }
    }

    return buf;
}

//无符号整数转换为浮点字符串
SECTION_PLT_CODE const char * str_UInt32FloatToString(uint32 value,int8 nFraction, c32_t buf) 
{
    int32 max_len = sizeof(c32_t);
    char ch[16];

    // 有小数部分
    if (nFraction > 0)
    {
        E_SPRINTF(ch, "%%u.%%0%du", nFraction);
        E_SNPRINTF(buf, max_len, ch, value / TenMultiple[nFraction], value % TenMultiple[nFraction]);
    }
    else
    {
        E_SNPRINTF(buf, max_len, "%u", value);
    }

    return buf;
}

//有符号64整数转换为浮点字符串(带小数位数)
SECTION_PLT_CODE const char * str_Int64FloatToString(int64 value,int8 nFraction, c64_t buf)
{
    int32 max_len = sizeof(c64_t);
    char ch[32];
    if (value >= 0)
    {
        // 有小数部分
        if (nFraction > 0)
        {
            E_SPRINTF(ch, "%%lld.%%0%dd", nFraction);
            E_SNPRINTF(buf, max_len, ch, value / TenMultiple[nFraction], value % TenMultiple[nFraction]);
        }
        else
        {
            E_SNPRINTF(buf, max_len, "%lld", value);
        }
    }
    else
    { // 负数处理

        uint64 u64 = f_abs(value);
        // 有小数部分
        if (nFraction > 0)
        {
            E_SPRINTF(ch, "-%%llu.%%0%du", nFraction);
            E_SNPRINTF(buf, max_len, ch, u64 / TenMultiple[nFraction], u64 % TenMultiple[nFraction]);
        }
        else
        {
            E_SNPRINTF(buf, max_len, "-%llu", u64);
        }
    }

    return buf;
}

//有符号整数转换为浮点字符串(0补全)
SECTION_PLT_CODE const char * str_Int32FloatToStringComplete(int32 value,uint8 iIntger,uint8 iPoint,c64_t pValue)
{
    char ch[32];
    if(value>=0)
    {
        // 有小数部分
        if(iPoint)
        {
            E_SPRINTF(ch,"%%0%dd.%%0%dd",iIntger, iPoint);
            E_SPRINTF(pValue, ch, value/TenMultiple[iPoint], value%TenMultiple[iPoint]);
        }
        else
        {
            E_SPRINTF(ch,"%%0%dd",iIntger);
            E_SPRINTF(pValue, ch, value);
        }
    }
    else
    {// 负数处理

        uint32 u32 = f_abs(value);
        // 有小数部分
        if(iPoint)
        {
            E_SPRINTF(ch, "-%%%du.%%0%du", iIntger,iPoint);
            E_SPRINTF(pValue, ch, u32/TenMultiple[iPoint], u32%TenMultiple[iPoint]);
        }
        else
        {
            E_SPRINTF(ch, "-%%%du", iIntger);
            E_SPRINTF(pValue, ch, u32);
        }
    }

    return pValue;
}

//无符号整数转换为浮点字符串(0补全)
SECTION_PLT_CODE const char * str_UInt32FloatToStringComplete(uint32 value,uint8 iIntger,uint8 iPoint,c64_t pValue)
{
    char ch[16];

    // 有小数部分
    if(iPoint>0)
    {
        E_SPRINTF(ch,"%%0%du.%%0%du",iIntger, iPoint);
        E_SPRINTF(pValue, ch, value/TenMultiple[iPoint], value%TenMultiple[iPoint]);
    }
    else
    {
        E_SPRINTF(ch, "%%%du", iIntger);
        E_SPRINTF(pValue, ch, value);
    }


    return pValue;
}

//矩阵字符串转换0000-FFFF
SECTION_PLT_CODE const char * str_MatrixToString(uint32 value, uint8 nMatrix, char * pValue)
{
    char ch[16];
    if(nMatrix > 0)
    {
        E_SPRINTF(ch, "%%0%dX", nMatrix);
    }
    else
    {
        E_STRCAT(ch, "%04X");
    }
    E_SPRINTF(pValue, ch, value);
    return pValue;
}

//转换uint32格式IP地址为string格式XXX.XXX.XXX.XXX
SECTION_PLT_CODE const char * str_GetIPString(uint32 ip, c32_t ch)
{
    E_SNPRINTF(ch,sizeof(c32_t),"%d.%d.%d.%d",(ip >> 24)&0xff, (ip >> 16)&0xff,(ip >> 8)&0xff,ip&0xff);
    return ch;
}

//IP字符串转32位int数   
SECTION_PLT_CODE uint32 str_to_ip(const char * ip)  
{  
    uint32 uResult = 0;  
    int nShift = 24;  
    int temp = 0;  
    const char *pStart = ip;  
    const char *pEnd = ip;  
      
    while (*pEnd != '\0')  
    {  
        while (*pEnd!='.' && *pEnd!='\0')  
        {  
            pEnd++;  
        }  
        temp = 0;  
        for (; pStart!=pEnd; ++pStart)  
        {  
            temp = temp * 10 + *pStart - '0';  
        }     
          
        uResult += temp<<nShift;  
        nShift -= 8;  
          
        if (*pEnd == '\0')  
            break;  
        pStart = pEnd + 1;  
        pEnd++;  
    }  
      
    return uResult;  
} 

//获取MAC地址字符串
SECTION_PLT_CODE const char * str_GetMacAddrString(const uint8 * mac_addr, c32_t ch)
{
    E_SNPRINTF(ch, sizeof(c32_t), "%02x-%02x-%02x-%02x-%02x-%02x", 
                mac_addr[0], mac_addr[1],mac_addr[2],
                mac_addr[3], mac_addr[4],mac_addr[5]);
    return ch;
}

//定值区转换位字符串
SECTION_PLT_CODE const char * str_ToSettingGroupString(TVAL nVal, c32_t ch)
{
//#ifdef _MANG_UNIT_
//    E_SNPRINTF(ch, sizeof(c32_t), str_GetStringLocal(ID_STRING_PROT_SETTING_GROUP_BdIn), nVal);
//#else
    E_SNPRINTF(ch, sizeof(c32_t), "%d", nVal);
//#endif
    return ch;
}

//获取Goose APPID字符串
SECTION_PLT_CODE const char * str_GetAppIdString(uint16 app_id, c32_t ch)
{
    E_SNPRINTF(ch, sizeof(c32_t), "%04x",app_id);
    return ch;
}

