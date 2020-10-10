/*
 * @Author: zhongwei
 * @Date: 2019/12/27 11:28:43
 * @Description: 字符串资源管理
 * @File: plt_string_res.h
 *
*/

#ifndef SRC_H_PUBLIC_PLT_STRING_RES_H_
#define SRC_H_PUBLIC_PLT_STRING_RES_H_

//初始化注册字符串
void PowerOn_InitStringResource(void);

//字符串资源注册
TID RegisterStringResource(const TSTRING_RESOURCE * pRes, BOOL bMalloc);

//注册字符串
TID RegisterStringResourceStr(const char * cn, const char * en, const char * ru);

//将注册字符串转换为数组
/* 
    需要注册的字符串资源注册完成后调用，可以加快字符串访问速度 
*/
void RegisterStringToArray(void);

//读取注册字符串资源
const TSTRING_RESOURCE * get_RegStringResource(TID id);
//读取注册字符串
const char * get_RegStringResourceLocal(const TSTRING_RESOURCE * pRes);
//读取注册字符串
const char * get_RegStringResourceStrLocal(TID id);

//获取字符串资源总个数
int32 get_RegStringResourceCount(void);


const char * get_InnerStringResourceLocal(const TINNER_STRING_RESOURCE * pRes);

#endif /* SRC_H_PUBLIC_PLT_STRING_RES_H_ */
