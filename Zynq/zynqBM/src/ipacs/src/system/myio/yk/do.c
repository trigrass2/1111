/*------------------------------------------------------------------------
 Module:       	do.c
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-09-1
 Description:  
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 9 $
------------------------------------------------------------------------*/

#include <syscfg.h>

#include "do.h"
#include "sys.h"

#if 0
static VDefTVYkCfg PDefTVYkCfg_Public1[] = 
{
    {"保护总复归",      "保护总复归", 0},
	{"FA投入",          "FA投入",   0},
};
#endif
#if(DEV_SP == DEV_SP_TTU)

static VDefTVYkCfg PDefTVYkCfg_Public2[] = 
{
	{"电度数据清除", 	"电度数据清除", 0},
	{"保留",		    "保留", 0},
	{"保留",			"保留",   0},
};
#elif(DEV_SP == DEV_SP_DTU_IU)
static VDefTVYkCfg PDefTVYkCfg_Public2[] = 
{
	{"电度数据清除",  "电度数据清除", 0},
	{"保护总复归",		"保护总复归", 0},
	{"FA投入",			"FA投入",   0},
#ifdef INCLUDE_FA_DIFF
	{"差动FA投入",		"差动FA投入",   0},
#endif
};
#else
static VDefTVYkCfg PDefTVYkCfg_Public2[] = 
{
	{"电池退出/活化", 	"电池退出/活化", 0},
	{"保护总复归",		"保护总复归", 0},
	{"FA投入",			"FA投入",   0},
#ifdef INCLUDE_FA_DIFF
	{"差动FA投入",		"差动FA投入",   0},
#endif
};
#endif


#ifdef INCLUDE_YK

#if(DEV_SP == DEV_SP_CTRL)
VYkPort  YkPort_Dio_3[] = 
{
	{BSP_PORT_YK,   0x03FF,   0,   0,   1}, 
  //{BSP_PORT_YKFJ, 0x0001,   0,   0,   0},  

};

//属性 设置端口 反校端口 使能端口 设置位 反校位 使能位 使能组定义 延时
//使能组定义    所有继电器大排序, 继电器1对应D0, 类推
//              然后将该继电器对应使能继电器所管辖的所有继电器按上述规则标记
VYkRelay YkRelay_Dio_3[] = 
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, -1},   
	{0, 0, 0, -1, 0x01, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x02, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x04, 0, 0, 0, 0, -1},	         

	{0, 0, 0, -1, 0x08, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x10, 0, 0, 0, 0, -1},	
	{0, 0, 0, -1, 0x20, 0, 0, 0, 0, -1},	

	{0, 0, 0, -1, 0x40, 0, 0, 0, 0, -1},	         
	{0, 0, 0, -1, 0x80, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x100, 0, 0, 0, 0, -1},	
	{0, 0, 0, -1, 0x200, 0, 0, 0, 0, -1},	
};

//本身 配对 执行 状态

 VYkPoint YkPoint_Dio_3[] = 
 {
	{0, 0, 0, 0,},
    {1, 2, 0, 0,},
	{2, 1, 0, 0,},
    {3, 4, 0, 0,},
	{4, 3, 0, 0,},
	{5, 6, 0, 0,},
    {6, 5, 0, 0,},
	{7, 8, 0, 0,},
	{8, 7, 0, 0,},
	{9, 10, 0, 0,},
	{10, 9, 0, 0,},
	{11, -1, -1, 0,},
 };

static VDo2Point Do2Point_Dio_3[] = 
{	   
    {1, 1, 0,},
	{1, 2, 0,},	
    {1, 3, 0,},
	{1, 4, 0,},		
    {1, 5, 0,},
	{1, 6, 0,},	
    {1, 7, 0,},
	{1, 8, 0,},	
    {1, 9, 0,},
	{1, 10, 0,},	
	{1, 11, 0,},
};   
static VDefTVYkCfg PDefTVYkCfg_Dio_3[] = 
{
    {"开关1",      "开关1", 0},
	{"开关2", 	   "开关2", 0},
	{"开关3",      "开关3", 0},
	{"开关4", 	   "开关4", 0},
	{"开关5",      "开关5", 0},
	{"开关6", 	   "开关6", 0},
	{"开关7",      "开关7", 0},
	{"开关8", 	   "开关8", 0},
	{"开关9",      "开关9", 0},
	{"开关10", 	   "开关10", 0},
	
};
#endif

#if (DEV_SP == DEV_SP_FTU)

VYkPort  YkPort_Dio_3[] = 
{
	{BSP_PORT_YK,   0x002F,   0,   0,   1}, 
	{BSP_PORT_YKFJ, 0x0001,   0,   0,   0},  

};

//属性 设置端口 反校端口 使能端口 设置位 反校位 使能位 使能组定义 延时
//使能组定义    所有继电器大排序, 继电器1对应D0, 类推
//              然后将该继电器对应使能继电器所管辖的所有继电器按上述规则标记
VYkRelay YkRelay_Dio_3[] = 
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, -1},   
	{1, 0, 1, -1, 0x01, 0x01, 0, 0, 0, -1},	
	{0, 0, 0, -1, 0x02, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x04, 0, 0, 0, 0, -1},	         
	{0, 0, 0, -1, 0x08, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x10, 0, 0, 0, 0, -1},	
	{0, 0, 0, -1, 0x20, 0, 0, 0, 0, -1},	
};

//本身 配对 执行 状态
VYkPoint YkPoint_Dio_3[] = 
{
	{0, 0, 0, 0,},
	{2, 3, 1, 0,},
	{3, 2, 1, 0,},
	{4, 5, 0, 0,},
	{5, 4, 0, 0,},
	{6, -1, -1, 0,},
};
 //wdj   电池活化 由于图纸分合有误,颠倒分合。
static VDo2Point Do2Point_Dio_3[] = 
{
	{0, 2, 1,},
	{0, 5, 0,},
    {0, 4, 3,},		                       
};   
 static VDefTVYkCfg PDefTVYkCfg_Dio_3[] = 
 {
   {"开关1跳/合",	   "开关1跳/合", 0},
	 {"储能",			 "储能", 0},
 };
#endif

#if (DEV_SP == DEV_SP_WDG)
VYkPort  YkPort_Dio_2[] = 
{
	{BSP_PORT_YK,   0x002F,   0,   0,   1}, 
	{BSP_PORT_YKFJ, 0x0001,   0,   0,   0},  

};

//属性 设置端口 反校端口 使能端口 设置位 反校位 使能位 使能组定义 延时
//使能组定义    所有继电器大排序, 继电器1对应D0, 类推
//              然后将该继电器对应使能继电器所管辖的所有继电器按上述规则标记
VYkRelay YkRelay_Dio_2[] = 
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, -1},   

	{1, 0, 1, -1, 0x01, 0x01, 0, 0, 0, -1},			 
	{0, 0, 0, -1, 0x02, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x04, 0, 0, 0, 0, -1},	         

	{0, 0, 0, -1, 0x08, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x10, 0, 0, 0, 0, -1},	
	{0, 0, 0, -1, 0x20, 0, 0, 0, 0, -1},			 

};

//本身 配对 执行 状态
VYkPoint YkPoint_Dio_2[] = 
{
	{0, 0, 0, 0,},
	{2, 3, 1, 0,},
	{3, 2, 1, 0,},
	{4, 5, 0, 0,},
	{5, 4, 0, 0,},
	{6, -1, -1, 0,},
};
 //wdj   电池活化
static VDo2Point Do2Point_Dio_2[] = 
{
	{0, 2, 1,},
	{0, 5, 0,},
  {0, 3, 4,},
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_2[] = 
{
  {"开关1跳/合",      "开关1跳/合", 0},
	{"储能", 	        "储能", 0},
};

// 新罩式
VYkPort  YkPort_Dio_6[] = 
{
	{BSP_PORT_YK,   0x002F,   0,   0,   1}, 
	{BSP_PORT_YKFJ, 0x0001,   0,   0,   0},  

};

//属性 设置端口 反校端口 使能端口 设置位 反校位 使能位 使能组定义 延时
//使能组定义    所有继电器大排序, 继电器1对应D0, 类推
//              然后将该继电器对应使能继电器所管辖的所有继电器按上述规则标记
VYkRelay YkRelay_Dio_6[] = 
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, -1},   

	{1, 0, 1, -1, 0x01, 0x01, 0, 0, 0, -1},			 
	{0, 0, 0, -1, 0x02, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x04, 0, 0, 0, 0, -1},	         

	{0, 0, 0, -1, 0x08, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x10, 0, 0, 0, 0, -1},	
	{0, 0, 0, -1, 0x20, 0, 0, 0, 0, -1},			 

};

//本身 配对 执行 状态
VYkPoint YkPoint_Dio_6[] = 
{
	{0, 0, 0, 0,},
	{2, 3, 1, 0,},
	{3, 2, 1, 0,},
	{4, 5, 0, 0,},
	{5, 4, 0, 0,},
	{6, -1, -1, 0,},
};
 //wdj   电池活化
static VDo2Point Do2Point_Dio_6[] = 
{
	{0, 2, 1,},
	{0, 5, 0,},
  {0, 3, 4,},
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_6[] = 
{
  {"开关1跳/合",      "开关1跳/合", 0},
	{"储能", 	        "储能", 0},
};

#endif

#if (DEV_SP == DEV_SP_DTU)
VYkPort  YkPort_Dio_2[] = 
{
	{BSP_PORT_YKD1,     0x1,   0,   0,    1},
	{BSP_PORT_YKD1+1,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+2,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+3,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+4,   0x1,   0,   0,    1},
	{BSP_PORT_YKD1+5,   0x1,   0,   0,    1},
	{BSP_PORT_YKD1+6,   0x1,   0,   0,    1},
	{BSP_PORT_YKD1+7,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+8,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+9,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+10,  0x1,   0,   0,    1},
	{BSP_PORT_YKD1+11,  0x1,   0,   0,    1},
	{BSP_PORT_YKD1+12,  0x1,   0,   0,    1},
	{BSP_PORT_YKD1+13,  0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+14,  0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+15,  0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+16,  0x1,   0,   0,    1},
	{BSP_PORT_YKD1+17,  0x1,   0,   0,    1},
	{BSP_ADDR_YK1_FJ,   0x003F,   0,   0,   0}, 
};

VYkRelay YkRelay_Dio_2[] = 
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, -1},   

	{0, 0,  0,   -1, 0x01, 0, 0, 0, 0, -1},			 
	{0, 1,  0,   -1, 0x01, 0, 0, 0, 0, -1},
	{1, 2,  18,  -1, 0x01, 0x01, 0, 0, 0, -1},	         

	{0, 3,  0,   -1, 0x01, 0, 0, 0, 0, -1},
	{0, 4,  0,   -1, 0x01, 0, 0, 0, 0, -1},	
	{1, 5,  18,  -1, 0x01, 0x02, 0, 0, 0, -1},	

	{0, 6,  0,   -1, 0x01, 0, 0, 0, 0, -1},			 
	{0, 7,  0,   -1, 0x01, 0, 0, 0, 0, -1},
	{1, 8,  18,  -1, 0x01, 0x04, 0, 0, 0, -1},	         

	{0, 9,   0,  -1, 0x01, 0, 0, 0, 0, -1},
	{0, 10,  0,  -1, 0x01, 0, 0, 0, 0, -1},	
	{1, 11,  18,  -1, 0x01, 0x08, 0, 0, 0, -1},

	{0, 12,  0,   -1, 0x01, 0, 0, 0, 0, -1},			 
	{0, 13,  0,   -1, 0x01, 0, 0, 0, 0, -1},
	{1, 14,  18,   -1, 0x01, 0x10, 0, 0, 0, -1},	         

	{0, 15,  0,  -1, 0x01, 0, 0, 0, 0, -1},
	{0, 16,  0,  -1, 0x01, 0, 0, 0, 0, -1},	
	{1, 17,  18,  -1, 0x01, 0x20, 0, 0, 0, -1},

};

VYkPoint YkPoint_Dio_2[] = 
{
    {0, 0, 0, 0,},
	{2, 1, 3, 0,},
	{1, 2, 3, 0,},
	{5, 4, 6, 0,},
	{4, 5, 6, 0,},
	{8, 7, 9, 0,},
	{7, 8, 9, 0,},
	{11, 10, 12, 0,},
	{10, 11, 12, 0,},
	{14, 13, 15, 0,},
	{13, 14, 15, 0,},
	{17, 16, 18, 0,},
	{16, 17, 18, 0,},
};

static VDo2Point Do2Point_Dio_2[] = 
{
	{0, 2, 1,},
	{0, 4, 3,},
  {0, 6, 5,},
  {0, 8, 7,},
	{0, 10, 9,},
  {0, 12, 11,},
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_2[] = 
{
  {"开关%d跳/合",      "开关%d跳/合", 0},
  {"开关%d跳/合",      "开关%d跳/合", 0},
  {"开关%d跳/合",      "开关%d跳/合", 0},
  {"开关%d跳/合",      "开关%d跳/合", 0},
  {"开关%d跳/合",      "开关%d跳/合", 0},
  {"开关%d跳/合",      "开关%d跳/合", 0},
};

VYkPort  YkPort_Dio_9[] = 
{
	{BSP_PORT_YKD1,     0x1,   0,   0,    1},
	{BSP_PORT_YKD1+1,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+2,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+3,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+4,   0x1,   0,   0,    1},
	{BSP_PORT_YKD1+5,   0x1,   0,   0,    1},
	{BSP_PORT_YKD1+6,   0x1,   0,   0,    1},
	{BSP_PORT_YKD1+7,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+8,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+9,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+10,  0x1,   0,   0,    1},
	{BSP_PORT_YKD1+11,  0x1,   0,   0,    1},
	{BSP_ADDR_YK1_FJ,   0x000F,   0,   0,   0}, 
};

VYkRelay YkRelay_Dio_9[] = 
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, -1},   

	{0, 0,  0,   -1, 0x01, 0, 0, 0, 0, -1},			 
	{0, 1,  0,   -1, 0x01, 0, 0, 0, 0, -1},
	{1, 2,  12,  -1, 0x01, 0x01, 0, 0, 0, -1},	         

	{0, 3,  0,   -1, 0x01, 0, 0, 0, 0, -1},
	{0, 4,  0,   -1, 0x01, 0, 0, 0, 0, -1},	
	{1, 5,  12,  -1, 0x01, 0x02, 0, 0, 0, -1},	

	{0, 6,  0,   -1, 0x01, 0, 0, 0, 0, -1},			 
	{0, 7,  0,   -1, 0x01, 0, 0, 0, 0, -1},
	{1, 8,  12,  -1, 0x01, 0x04, 0, 0, 0, -1},	         

	{0, 9,   0,  -1, 0x01, 0, 0, 0, 0, -1},
	{0, 10,  0,  -1, 0x01, 0, 0, 0, 0, -1},	
	{1, 11,  12,  -1, 0x01, 0x08, 0, 0, 0, -1},

};

VYkPoint YkPoint_Dio_9[] = 
{
    {0, 0, 0, 0,},
	{2, 1, 3, 0,},
	{1, 2, 3, 0,},
	{5, 4, 6, 0,},
	{4, 5, 6, 0,},
	{8, 7, 9, 0,},
	{7, 8, 9, 0,},
	{11, 10, 12, 0,},
	{10, 11, 12, 0,},	
};

static VDo2Point Do2Point_Dio_9[] = 
{
	{0, 2, 1,},
	{0, 4, 3,},
  {0, 6, 5,},
  {0, 8, 7,},
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_9[] = 
{
  {"开关%d跳/合",      "开关%d跳/合", 0},
  {"开关%d跳/合",      "开关%d跳/合", 0},
  {"开关%d跳/合",      "开关%d跳/合", 0},
  {"开关%d跳/合",      "开关%d跳/合", 0},
};

/*针对12YX2YK的板卡  遥控在第1 (离CPU4)块遥信板上 2016年9月19日13:53:27 cjl*/
VYkPort  YkPort_Dio_10[] = 
{
	{BSP_PORT_YKD1 + 3*18,     0x1,   0,   0,    1},
	{BSP_PORT_YKD1+1 +3*18,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+2 + 3*18,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+3 + 3*18,   0x1,   0,   0,    1}, 
	{BSP_PORT_YKD1+4 +  3*18,   0x1,   0,   0,    1},
	{BSP_PORT_YKD1+5 + 3*18,   0x1,   0,   0,    1},
	{0x29 + 3*8,   0x000F,   0,   0,   0}, 
};

VYkRelay YkRelay_Dio_10[] = 
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, -1},   

	{0, 0,  0,   -1, 0x01, 0, 0, 0, 0, -1},			 
	{0, 1,  0,   -1, 0x01, 0, 0, 0, 0, -1},
	{1, 2,  6,  -1, 0x01, 0x01, 0, 0, 0, -1},	         

	{0, 3,  0,   -1, 0x01, 0, 0, 0, 0, -1},
	{0, 4,  0,   -1, 0x01, 0, 0, 0, 0, -1},	
	{1, 5,  6,  -1, 0x01, 0x02, 0, 0, 0, -1},	


};

VYkPoint YkPoint_Dio_10[] = 
{
    {0, 0, 0, 0,},
	{2, 1, 3, 0,},
	{1, 2, 3, 0,},
	{5, 4, 6, 0,},
	{4, 5, 6, 0,},
};

static VDo2Point Do2Point_Dio_10[] = 
{
	{0, 2, 1,},
	{0, 4, 3,},
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_10[] = 
{
  {"开关%d跳/合",      "开关%d跳/合", 0},
  {"开关%d跳/合",      "开关%d跳/合", 0},
};

#endif

#if (DEV_SP == DEV_SP_TTU)
VYkPort  YkPort_Dio_5[] = 
{
	{BSP_PORT_YK,   0x002F,   0,   0,   1}, 
	{BSP_PORT_YKFJ, 0x0003,   0,   0,   0},  

};

//属性 设置端口 反校端口 使能端口 设置位 反校位 使能位 使能组定义 延时
//使能组定义    所有继电器大排序, 继电器1对应D0, 类推
//              然后将该继电器对应使能继电器所管辖的所有继电器按上述规则标记
VYkRelay YkRelay_Dio_5[] = 
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, -1},   

	{1, 0, 1, -1, 0x01, 0x01, 0, 0, 0, -1},			 
	{0, 0, 0, -1, 0x02, 0, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x04, 0, 0, 0, 0, -1},	         

	{1, 0, 1, -1, 0x08, 0x02, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x10, 0, 0, 0, 0, -1},	
	{0, 0, 0, -1, 0x20, 0, 0, 0, 0, -1},				 

};

//本身 配对 执行 状态
VYkPoint YkPoint_Dio_5[] = 
{
	{0, 0, 0, 0,},
	{2, 3, 1, 0,},
	{3, 2, 1, 0,},
	{5, 6, 4, 0,},
	{6, 5, 4, 0,},
	{-1, -1, -1, 0,},

};
 //wdj   电池活化 由于图纸分合有误,颠倒分合。
static VDo2Point Do2Point_Dio_5[] = 
{
	{0, 2, 1,},
	{0, 4, 3,},
	{1, 5, 0,},		                       
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_5[] = 
{
    {"开关1跳/合",    "开关1跳/合", 0},
    {"动作1跳/合", 	  "动作1跳/合", 0},
};

#endif
#if (DEV_SP == DEV_SP_DTU_IU)
VYkPort  YkPort_Dio_7[] = 
{
	{BSP_PORT_YK,   0x002F,   0,   0,   1}, 
	{BSP_PORT_YKFJ, 0x0003,   0,   0,   0},  

};

//属性 设置端口 反校端口 使能端口 设置位 反校位 使能位 使能组定义 延时
//使能组定义    所有继电器大排序, 继电器1对应D0, 类推
//              然后将该继电器对应使能继电器所管辖的所有继电器按上述规则标记
VYkRelay YkRelay_Dio_7[] = 
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, -1},   

	{1, 0, 1, -1, 0x08, 0x02, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x10, 0, 0, 0, 0, -1},	
	{0, 0, 0, -1, 0x20, 0, 0, 0, 0, -1},			 

	{1, 0, 1, -1, 0x01, 0x01, 0, 0, 0, -1},
	{0, 0, 0, -1, 0x02, 0, 0, 0, 0, -1},	
	{0, 0, 0, -1, 0x04, 0, 0, 0, 0, -1},			 

};

//本身 配对 执行 状态
VYkPoint YkPoint_Dio_7[] = 
{
	{0, 0, 0, 0,},
	{2, 3, 1, 0,},
	{3, 2, 1, 0,},
	{5, 6, 4, 0,},
	{6, 5, 4, 0,},
	{-1, -1, -1, 0,},
};
 //wdj   电池活化 由于图纸分合有误,颠倒分合。
static VDo2Point Do2Point_Dio_7[] = 
{
	{0, 2, 1,},
	{0, 4, 3,},
    {1, 5, 0,},		                       
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_7[] = 
{
  {"动作1跳/合", 	  "动作1跳/合", 0},
    {"开关1跳/合",      "开关1跳/合", 0},
};
#endif
#endif

//如果没有有电池活化YK, 则必须选用PDefTVYkCfg_Public1, 同时donum为Do2Point
//如果有电池活化YK, 则必须选用PDefTVYkCfg_Public2, 同时donum为Do2Point-1
//上述规则必须严格遵守,否则做电池活化的时候会导致死机
//也就是说当有电池活化，退出的时候,最后两个个YK为出口节点,但在YK列表中隐藏
//而将其转定义的数据库中的"电池活化"点号, 当做该点的时候, 做donum+1 点YK
static VDefDoCfg defMyDoCfg[] = 
{
	{0, 0, NULL, 0, NULL, 0, NULL, NULL, 0, NULL, sizeof(PDefTVYkCfg_Public2)/sizeof(PDefTVYkCfg_Public2[0]), PDefTVYkCfg_Public2},
#ifdef INCLUDE_YK
#if ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_CTRL))
    {3, sizeof(Do2Point_Dio_3)/sizeof(Do2Point_Dio_3[0])-1, Do2Point_Dio_3, sizeof(YkPort_Dio_3)/sizeof(YkPort_Dio_3[0]), YkPort_Dio_3, sizeof(YkRelay_Dio_3)/sizeof(YkRelay_Dio_3[0])-1, YkRelay_Dio_3, YkPoint_Dio_3, sizeof(PDefTVYkCfg_Dio_3)/sizeof(PDefTVYkCfg_Dio_3[0]), PDefTVYkCfg_Dio_3, sizeof(PDefTVYkCfg_Public2)/sizeof(PDefTVYkCfg_Public2[0]), PDefTVYkCfg_Public2},
#endif    
#if (DEV_SP == DEV_SP_WDG)
    {2, sizeof(Do2Point_Dio_2)/sizeof(Do2Point_Dio_2[0])-1, Do2Point_Dio_2, sizeof(YkPort_Dio_2)/sizeof(YkPort_Dio_2[0]), YkPort_Dio_2, sizeof(YkRelay_Dio_2)/sizeof(YkRelay_Dio_2[0])-1, YkRelay_Dio_2, YkPoint_Dio_2, sizeof(PDefTVYkCfg_Dio_2)/sizeof(PDefTVYkCfg_Dio_2[0]), PDefTVYkCfg_Dio_2, sizeof(PDefTVYkCfg_Public2)/sizeof(PDefTVYkCfg_Public2[0]), PDefTVYkCfg_Public2},
    {6, sizeof(Do2Point_Dio_6)/sizeof(Do2Point_Dio_6[0])-1, Do2Point_Dio_6, sizeof(YkPort_Dio_6)/sizeof(YkPort_Dio_6[0]), YkPort_Dio_6, sizeof(YkRelay_Dio_6)/sizeof(YkRelay_Dio_6[0])-1, YkRelay_Dio_6, YkPoint_Dio_6, sizeof(PDefTVYkCfg_Dio_6)/sizeof(PDefTVYkCfg_Dio_6[0]), PDefTVYkCfg_Dio_6, sizeof(PDefTVYkCfg_Public2)/sizeof(PDefTVYkCfg_Public2[0]), PDefTVYkCfg_Public2},
#endif  
#if (DEV_SP == DEV_SP_DTU)
    {2, sizeof(Do2Point_Dio_2)/sizeof(Do2Point_Dio_2[0]), Do2Point_Dio_2, sizeof(YkPort_Dio_2)/sizeof(YkPort_Dio_2[0]), YkPort_Dio_2, sizeof(YkRelay_Dio_2)/sizeof(YkRelay_Dio_2[0])-1, YkRelay_Dio_2, YkPoint_Dio_2, sizeof(PDefTVYkCfg_Dio_2)/sizeof(PDefTVYkCfg_Dio_2[0]), PDefTVYkCfg_Dio_2, 0, NULL},
    {9, sizeof(Do2Point_Dio_9)/sizeof(Do2Point_Dio_9[0]), Do2Point_Dio_9, sizeof(YkPort_Dio_9)/sizeof(YkPort_Dio_9[0]), YkPort_Dio_9, sizeof(YkRelay_Dio_9)/sizeof(YkRelay_Dio_9[0])-1, YkRelay_Dio_9, YkPoint_Dio_9, sizeof(PDefTVYkCfg_Dio_9)/sizeof(PDefTVYkCfg_Dio_9[0]), PDefTVYkCfg_Dio_9, 0, NULL},
	{10, sizeof(Do2Point_Dio_10)/sizeof(Do2Point_Dio_10[0]), Do2Point_Dio_10, sizeof(YkPort_Dio_10)/sizeof(YkPort_Dio_10[0]), YkPort_Dio_10, sizeof(YkRelay_Dio_10)/sizeof(YkRelay_Dio_10[0])-1, YkRelay_Dio_10, YkPoint_Dio_10, sizeof(PDefTVYkCfg_Dio_10)/sizeof(PDefTVYkCfg_Dio_10[0]), PDefTVYkCfg_Dio_10, 0, NULL},
#endif	
#if (DEV_SP == DEV_SP_TTU)
	{5, sizeof(Do2Point_Dio_5)/sizeof(Do2Point_Dio_5[0])-1, Do2Point_Dio_5, sizeof(YkPort_Dio_5)/sizeof(YkPort_Dio_5[0]), YkPort_Dio_5, sizeof(YkRelay_Dio_5)/sizeof(YkRelay_Dio_5[0])-1, YkRelay_Dio_5, YkPoint_Dio_5, sizeof(PDefTVYkCfg_Dio_5)/sizeof(PDefTVYkCfg_Dio_5[0]), PDefTVYkCfg_Dio_5, sizeof(PDefTVYkCfg_Public2)/sizeof(PDefTVYkCfg_Public2[0]), PDefTVYkCfg_Public2},
#endif 
#if(DEV_SP == DEV_SP_DTU_IU )
	{7, sizeof(Do2Point_Dio_7)/sizeof(Do2Point_Dio_7[0])-1, Do2Point_Dio_7, sizeof(YkPort_Dio_7)/sizeof(YkPort_Dio_7[0]), YkPort_Dio_7, sizeof(YkRelay_Dio_7)/sizeof(YkRelay_Dio_7[0])-1, YkRelay_Dio_7, YkPoint_Dio_7, sizeof(PDefTVYkCfg_Dio_7)/sizeof(PDefTVYkCfg_Dio_7[0]), PDefTVYkCfg_Dio_7, sizeof(PDefTVYkCfg_Public2)/sizeof(PDefTVYkCfg_Public2[0]), PDefTVYkCfg_Public2},
#endif
#if(DEV_SP == DEV_SP_DTU_PU )
	{9, 0, NULL, 0, NULL, 0, NULL, NULL, 0, NULL, sizeof(PDefTVYkCfg_Public2)/sizeof(PDefTVYkCfg_Public2[0]), PDefTVYkCfg_Public2},
#endif

#endif	
	{(DWORD)-1,  0, NULL, 0, NULL, 0, NULL, NULL, 0, NULL, 0, NULL},
};	

VDefDoCfg *GetDefDoCfg(DWORD type)
{
	VDefDoCfg *pcfg = defMyDoCfg;


    while (pcfg->ext_type != (DWORD)-1)
    {
		if (pcfg->ext_type == type) break;
		pcfg++;
	}

    if (pcfg->ext_type == (DWORD)-1)  return NULL;
	else return pcfg;
}

int GetDefPDoCfg(DWORD type, WORD num, struct VPDoCfg *pPCfg)
{
    int i;
	VDo2Point *pDo2Point;
	VDefDoCfg *pcfg = GetDefDoCfg(type);

    if (pcfg == NULL) return ERROR;

    pDo2Point = pcfg->do2point;
	
	for (i=0; i<num; i++)
	{
		pPCfg->type = pDo2Point->type;
		pPCfg->cfg = 0;
		pPCfg->hzontime = 2000;
		pPCfg->fzontime = 2000;
		pPCfg->ybno = -1;
		pPCfg->yztime = 30000;
		pPCfg++;
		pDo2Point++;
	}

	return OK;
}

VDefTVYkCfg *GetDefTVYkCfg_Do(DWORD type, int *pnum)
{
	VDefDoCfg *pcfg = GetDefDoCfg(type);

    if (pcfg == NULL) 
	{
		*pnum = 0;
		return NULL;
    }	
	else
	{
		*pnum = pcfg->tvykcfgnum_do;
		return(pcfg->ptvykcfg_do);
	}
}

VDefTVYkCfg *GetDefTVYkCfg_Public(DWORD type, int *pnum)
{
	VDefDoCfg *pcfg = GetDefDoCfg(type);

    if (pcfg == NULL) 
	{
		*pnum = 0;
		return NULL;
    }	
	else
	{
#if (TYPE_IO == IO_TYPE_EXTIO)
		*pnum = 0;
		return NULL;
#else
		*pnum = pcfg->tvykcfgnum_public;
		return(pcfg->ptvykcfg_public);
#endif
	}
}


#if (TYPE_IO == IO_TYPE_MASTER)

static VDo2Point Do2Point_ExtDio_2[] = 
{
	{0, 1, 2,},
	{0, 3, 4,},
	{0, 5, 6,},
	{0, 7, 8,},
	{0, 9, 10,},
	{0, 11, 12,},		
	{0, 13, -1,},		
};   

static VDefTVYkCfg PDefTVYkCfg_ExtDio_2[] = 
{
    {"YK1[%d]",     "YK1[%d]", 0},
	{"YK2[%d]",	    "YK2[%d]", 0},
	{"YK3[%d]", 	"YK3[%d]", 0},
	{"YK4[%d]", 	"YK4[%d]", 0},
	{"YK5[%d]", 	"YK5[%d]", 0},
	{"YK6[%d]", 	"YK6[%d]", 0},
	{"YK7[%d]", 	"YK7[%d]", 0},
};

static VDefDoCfg defExtDoCfg[] = 
{
	{0,  0, NULL, 0, NULL, 0, NULL, NULL, 0, NULL, 0, NULL},
	{1,  0, NULL, 0, NULL, 0, NULL, NULL, 0, NULL, 0, NULL},
	{2, sizeof(Do2Point_ExtDio_2)/sizeof(Do2Point_ExtDio_2[0]), Do2Point_ExtDio_2, 0, NULL, 0, NULL, NULL, sizeof(PDefTVYkCfg_ExtDio_2)/sizeof(PDefTVYkCfg_ExtDio_2[0]), PDefTVYkCfg_ExtDio_2, 0, NULL},
	{3, sizeof(Do2Point_Dio_3)/sizeof(Do2Point_Dio_3[0]), Do2Point_Dio_3, 0, NULL, 0, NULL, NULL, sizeof(PDefTVYkCfg_Dio_3)/sizeof(PDefTVYkCfg_Dio_3[0]), PDefTVYkCfg_Dio_3, 0, NULL},
	{-1,  0, NULL, 0, NULL, 0, NULL, NULL, 0, NULL, 0, NULL},
};	

VDefDoCfg *GetDefExtDoCfg(DWORD type)
{
	DWORD ext_t;
	VDefDoCfg *pcfg = defExtDoCfg;

	ext_t = GetExtDioType(type);

    while (pcfg->ext_type != -1)
    {
		if (pcfg->ext_type == ext_t) break;
		pcfg++;
	}

    if (pcfg->ext_type == -1)  return NULL;
	else return pcfg;
}

int GetDefExtPDoCfg(DWORD type, WORD num, struct VPDoCfg *pPCfg)
{
    int i;
	VDo2Point *pDo2Point;
	VDefDoCfg *pcfg = GetDefExtDoCfg(type);

    if (pcfg == NULL) return ERROR;

    pDo2Point = pcfg->do2point;
	
	for (i=0; i<num; i++)
	{
		pPCfg->type = pDo2Point->type;
		pPCfg->cfg = 0;
		pPCfg->hzontime = 2000;
		pPCfg->fzontime = 2000;
		pPCfg->ybno = -2;
		pPCfg->yztime = 30000;
		pPCfg++;
		pDo2Point++;
	}

	return OK;
}

VDefTVYkCfg *GetDefExtTVYkCfg_Do(DWORD type, int *pnum)
{
	VDefDoCfg *pcfg = GetDefExtDoCfg(type);

    if (pcfg == NULL) 
	{
		*pnum = 0;
		return NULL;
    }	
	else
	{
		*pnum = pcfg->tvykcfgnum_do;
		return(pcfg->ptvykcfg_do);
	}
}

VDefTVYkCfg *GetDefExtTVYkCfg_Public(DWORD type, int *pnum)
{
	VDefDoCfg *pcfg = GetDefExtDoCfg(type);

    if (pcfg == NULL) 
	{
		*pnum = 0;
		return NULL;
    }	
	else
	{
		*pnum = pcfg->tvykcfgnum_public;
		return(pcfg->ptvykcfg_public);
	}
}

#endif

