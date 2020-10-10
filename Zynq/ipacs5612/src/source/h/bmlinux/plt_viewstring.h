/*
 * @Author: zhongwei
 * @Date: 2019/12/20 19:24:21
 * @Description: 字符串显示处理
 * @File: plt_viewstring.h
 *
*/
#ifndef SRC_H_BAREMETAL_PLT_VIEWSTRING_H_
#define SRC_H_BAREMETAL_PLT_VIEWSTRING_H_

//字符串资源是否为空
BOOL str_IsStringResEmpty(TID nStrID);

//软件版本字符串
const char * str_GetSoftVersionString(int32 ver, c32_t ch);

//读取本地字符串
#define str_GetStringLocal(nStrID)      get_RegStringResourceStrLocal(nStrID)
#define str_GetStringResLocal(pRes)     get_RegStringResourceLocal(pRes)

//读61850量纲字符串
const char * str_GetSIUnit_String(int8 nSIUnit);
//读61850量纲倍数
const char * str_GetSIMultiple_String(int8 nSIMultiple);
//GIN字符串  XXXX
const char * str_GetGinString(TGin gin, c32_t buf);
//品质字符串
const char * str_GetCommQualityString(TQuality quality, c32_t buf);
//检修字符串
const char * str_GetTestQualityString(TQuality test, c32_t buf);
//越限字符串
const char * str_GetOverQualityString(TQuality over, c32_t buf);

//获取BOOL变量显示字符串
const char * str_GetBoolViewString(BOOL b, c32_t ch);
//获取双点信息值
const char * str_GetDualPtViewString(int32 i, c32_t ch);
//获取整数显示字符串
const char * str_GetInt32ViewString(int32 i32, c32_t ch);
//获取无符号整数显示字符串
const char * str_GetUint32ViewString(uint32 u32, c32_t ch);
//获取整数显示字符串
const char * str_GetInt64ViewString(int64 i64, c32_t ch);
//获取无符号整数显示字符串
const char * str_GetUint64ViewString(uint64 u64, c32_t ch);
//显示16进制（32位整数）
const char * str_GetHex32ViewString(uint32 hex, c32_t ch);

//显示16进制（64位整数）
const char * str_GetHex64ViewString(uint64 hex, c32_t ch);
//获取浮点数fp64显示字符串
const char * str_Getfp32ViewString(fp32 f32, c64_t ch);
//获取浮点数fp64显示字符串，指定显示小数位数
const char * str_Getfp32ViewStringEx(fp32 f32,int8 nFraction, c64_t ch);
//获取浮点数fp64显示字符串
const char * str_Getfp64ViewString(fp64 f64, c64_t ch);
//获取浮点数fp64显示字符串，指定显示小数位数
const char * str_Getfp64ViewStringEx(fp64 f64,int8 nFraction, c64_t ch);
//获得TVAR_DATA显示字符串
const char * str_GetTVAR_DATAViewString(TVAR_DATA var, c64_t ch);
//获得TVAR_DATA显示字符串，指定显示的小数位数
const char * str_GetTVAR_DATAViewStringEx(TVAR_DATA var,int8 nFraction, c64_t ch);
//有符号整数转换为浮点字符串(带小数位数)
const char * str_Int32FloatToString(int32 value,int8 nFraction, c32_t buf);
//无符号整数转换为浮点字符串
const char * str_UInt32FloatToString(uint32 value,int8 nFraction, c32_t buf) ;
//有符号64整数转换为浮点字符串(带小数位数)
const char * str_Int64FloatToString(int64 value,int8 nFraction, c64_t buf);
//有符号整数转换为浮点字符串(0补全)
const char * str_Int32FloatToStringComplete(int32 value,uint8 iIntger,uint8 iPoint,c64_t pValue);
//无符号整数转换为浮点字符串(0补全)
const char * str_UInt32FloatToStringComplete(uint32 value,uint8 iIntger,uint8 iPoint,c64_t pValue);
//矩阵字符串转换0000-FFFF
const char * str_MatrixToString(uint32 value, uint8 nMatrix, char * pValue);
//转换uint32格式IP地址为string格式XXX.XXX.XXX.XXX
const char * str_GetIPString(uint32 ip, c32_t ch);
//IP字符串转32位int数   
uint32 str_to_ip(const char * ip);

//获取MAC地址字符串
const char * str_GetMacAddrString(const uint8 * mac_addr, c32_t ch);

//将测值转换为字符串接口
const char * str_ToMeasureValueString(TVAR_DATA var, int32 nType, int8 nFraction , c64_t ch);

//定值的值转换为字符串
const char * str_ToSettingValueString(TVAL nVal, int16 nValueType, const void * pValueDef, int32 nSpecialType, TVAR_DATA varSpecialValue, c64_t ch);

//定值区转换位字符串
const char * str_ToSettingGroupString(TVAL nVal, c32_t ch);

//定值区转换位字符串
const char * str_ToSettingGroupString(TVAL nVal, c32_t ch);

//获取Goose APPID字符串
const char * str_GetAppIdString(uint16 app_id, c32_t ch);

#endif /* SRC_H_BAREMETAL_PLT_VIEWSTRING_H_ */
