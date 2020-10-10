/*
 * @Author: zhongwei
 * @Date: 2019/12/24 16:11:16
 * @Description: 用于导出代码的类型定义
 * @File: plt_if_type.h
 *
*/

#ifndef SRC_H_EXPORT_PLT_IF_TYPE_H_
#define SRC_H_EXPORT_PLT_IF_TYPE_H_

typedef TQuality        TTQuality_array[MAX_POINT_COUNT_PER_TYPE];
typedef int32           Tint32_array[MAX_POINT_COUNT_PER_TYPE];
typedef uint32          Tuint32_array[MAX_POINT_COUNT_PER_TYPE];
typedef BOOL            TBOOL_array[MAX_POINT_COUNT_PER_TYPE];
typedef TVAR_DATA       TTVAR_DATA_array[MAX_POINT_COUNT_PER_TYPE];
typedef int8            Tint8_array[MAX_POINT_COUNT_PER_TYPE];
typedef uint8           Tuint8_array[MAX_POINT_COUNT_PER_TYPE];
typedef fp32            Tfp32_array[MAX_POINT_COUNT_PER_TYPE];
typedef fp64            Tfp64_array[MAX_POINT_COUNT_PER_TYPE];
typedef char_p          Tchar_p_array[MAX_POINT_COUNT_PER_TYPE];

#endif /* SRC_H_EXPORT_PLT_IF_TYPE_H_ */
