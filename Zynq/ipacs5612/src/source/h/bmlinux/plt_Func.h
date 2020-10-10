/*
 * @Author: zhongwei
 * @Date: 2019-11-19 10:04:53
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-19 11:20:03
 * @Description: 公共函数定义
 * @FilePath: \ZynqBM\src\include\plt_Func.h
 */
#ifndef SRC_INCLUDE_PLT_FUNC_H_
#define SRC_INCLUDE_PLT_FUNC_H_

#include <stdarg.h>

#define MAKEWORD(a, b)       ((uint16)(((uint8)(a)) | ((uint16)((uint8)(b))) << 8))
#define MAKEDWORD(a, b)      ((uint32)(((uint16)(a)) | ((uint32)((uint16)(b))) << 16))
#define MAKEDDWORD(a, b)     ((uint64)(((uint32)(a)) | ((uint64)((uint32)(b))) << 32))
#define MAKEUINT32(a,b,c,d)  MAKEDWORD(MAKEWORD((a),(b)),MAKEWORD((c),(d)))
#define MAKEUINT64(a,b,c,d,e,f,g,h)  MAKEDDWORD(MAKEUINT32((a),(b),(c),(d)),MAKEUINT32((e),(f),(g),(h)))
#define LOWORD(l)            ((uint16)((l) & 0xFFFF))
#define HIWORD(l)            ((uint16)(((uint32)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)            ((uint8)((w) & 0xFF))
#define HIBYTE(w)            ((uint8)(((uint16)(w) >> 8) & 0xFF))
#define LODWORD(ll)          ((uint32)((ll) & 0xFFFFFFFFU))
#define HIDWORD(ll)          ((uint32)(((uint64)(ll) >> 32) & 0xFFFFFFFFU))

#define f_abs(a) (((a)>0)?(a):(0-(a)))
#define f_max(a,b)   (((a) > (b)) ? (a) : (b))
#define f_min(a,b)   (((a) < (b)) ? (a) : (b))
#define f_max3(a,b,c)    f_max(f_max((a),(b)),(c))
#define f_min3(a,b,c)    f_min(f_min((a),(b)),(c))

//计算10的次方的函数 n<8
extern const uint32 TenMultiple[];
#define TEN_MULTIPLE(n)     TenMultiple[n]
#define MAX_TEN_MULTIPLE 9      //n最多支持到9

/*
    BOOL值判断接口定义
    BM和Linux侧的判别不一样
*/
#ifdef _PROT_UNIT_
    #define IsTrue(b)           ((b)==PLT_TRUE)     //是否为TRUE
    #define IsFalse(b)          ((b)==PLT_FALSE)    //是否为FALSE（加强检测，只要不为TRUE，即FALSE）
#else
    #define IsTrue(b)           (b)                 //是否为TRUE
    #define IsFalse(b)          (!(b))              //是否为FALSE（加强检测，只要不为TRUE，即FALSE）
#endif

//任何数转BOOL类型
#define BoolFrom(i)         (((i)==0)?PLT_FALSE:PLT_TRUE)   //i值转为E_TRUE/E_FALSE
#define BoolTo(b)           (IsTrue(b)?1:0)             //b值转为1/0
//BOOL类型逻辑计算
#define BoolNot(b)          (IsTrue(b)?PLT_FALSE:PLT_TRUE)                      //取反
#define BoolAnd(a,b)        ((IsTrue(a) && IsTrue(b))?PLT_TRUE:PLT_FALSE)       //AND
#define BoolOr(a,b)         ((IsTrue(a) || IsTrue(b))?PLT_TRUE:PLT_FALSE)       //OR
#define BoolXor(a,b)        ((IsTrue(a) ^ IsTrue(b))?PLT_TRUE:PLT_FALSE)        //异或
#define BoolUp(prev,now)    ((!IsTrue(prev) && IsTrue(now))?PLT_TRUE:PLT_FALSE)             //BOOL上升延检测
#define BoolDown(prev,now)  ((IsTrue(prev) && (!IsTrue(now)))?PLT_TRUE:PLT_FALSE)           //BOOL下降延检测

//CRC16校验函数
uint16 crc16(const void * buf,uint32 len);
//CRC16校验函数增强版
void crc16_ex(const void * buf, uint32 len, uint16 * crc);

//CRC32
uint32 crc32_calc(uint32 crc,const uint8 * buffer, uint32 size);
uint32 f_crc32(const uint8 * buffer, uint32 size);
#ifdef _MANG_UNIT_
int calc_img_crc(const char *in_file, unsigned int *img_crc);
#endif	//_MANG_UNIT_

//字符串是否为空
BOOL str_IsStringEmpty(const char * str);
//字符裁剪，去除开始的空格和末尾空格
char * str_strip(char * str);

//按位表示需要的个数
#define _TO_BIT_EXPRESS(num, bits)  ((num)/(bits)+((((num)%(bits))==0)?0:1))

//! 按位将32位浮点数转换为整数
#define BIT_CONVERT_FP32_TO_UINT32(fl) (*((uint32 *)((fp32 *)(&fl))))
//! 按位将32位整数转换为浮点数
#define BIT_CONVERT_UINT32_TO_FP32(ui) (*((fp32 *)((uint32 *)(&ui))))

#define BIT_CONVERT_FP64_TO_UINT64(fl) (*((uint64 *)((fp64 *)(&fl))))
#define BIT_CONVERT_UINT64_TO_FPT64(fl) (*((fp64 *)((uint64 *)(&ui))))

//为了解决浮点转定点时精度丢失的问题，进行浮点数修正
//例如浮点2.0实际存储的值是1.999999998
#define FP_TO_INT(type, fl) ((type)((fl)<0?((fl)-0.000001):((fl)+0.000001)))   //0.000001
#define FP_TO_INT32(fl) FP_TO_INT(int32, fl)
#define FP_TO_INT64(fl) FP_TO_INT(int64, fl)

#define FP_TO_INT32_EX(fl) ((int32)((fl)<0?((fl)-0.1):((fl)+0.1)))      //0.1

//带四舍五入的除法
#define f_RoundDiv(a,b) (((a)/(b))+((a)%(b))*2/(b))

//快速开方算法
fp64 f_sqrt_fast(fp64 x0, fp64 x1);
fp64 f_CarmackSqrt(fp64 f);

//根据GIN得到组号
TGrpNo i_GetGinGrpNo(TGin gin);

//根据GIN得到条目号
TItemNo i_GetGinItemNo(TGin gin);

//根据组号条目号生成GIN号
TGin i_MakeGin(TGrpNo grp_no, TItemNo item_no);

//分配内存，并初始化为0
void * f_malloc(size_t __size);

#endif /* SRC_INCLUDE_PLT_FUNC_H_ */
