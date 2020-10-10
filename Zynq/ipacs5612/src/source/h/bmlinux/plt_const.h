/*
 * @Author: zhongwei
 * @Date: 2019-11-19 11:52:02
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-19 11:52:24
 * @Description: 常数定义
 * @FilePath: \ZynqBM\src\include\plt_const.h
 */
#ifndef SRC_INCLUDE_PLT_CONST_H_
#define SRC_INCLUDE_PLT_CONST_H_

/************************************************************************/
//      仅裸核程序使用的常量定义
//
/************************************************************************/
#ifdef _PROT_UNIT_


#endif  //_PROT_UNIT_

/************************************************************************/
//      仅Linux程序使用的常量定义
//
/************************************************************************/
#ifdef _MANG_UNIT_


#endif  //_MANG_UNIT_


/************************************************************************/
//      裸核和Linux均使用的常量定义
//
/************************************************************************/
#define ID_STRING_NULL      0        //空字符串ID值，0

#define LINK_NONE           -1

#define UNKNOW_GIN          0xffff

//每种数据类型最多支持的通用分类GIN组数
#define MAX_GRP_COUNT_PER_DATE_GROUP            4

//每种类型最大测点个数
#define MAX_POINT_COUNT_PER_TYPE                1024

//BM内部设定值的最大个数
#define MAX_POINT_TYPE_INNER_BM_SET_COUNT                   255
//BM内部调试数据的最大个数
#define MAX_POINT_TYPE_INNER_BM_DBGDATA_COUNT               255

#endif /* SRC_INCLUDE_PLT_CONST_H_ */
