/*------------------------------------------------------------------------
 Module:       	di.c
 Author:        solar
 Project:       
 State:			
 Creation Date:	2008-10-10
 Description:   
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 7 $
------------------------------------------------------------------------*/

#include <syscfg.h>

#include "di.h"
#include "sys.h"

#if (DEV_SP == DEV_SP_TTU)
static VDefTVYxCfg PDefTVYxCfg_Public[] = 
{
	{"宽带载波",    "宽带载波", 0},
	{"计量芯片状态",	"计量芯片状态", 0},
	{"外部电源",	"外部电源", 0},
	{"保留",		"保留", 0},

	{"保留",		"保留", 0},
	{"保留",		"保留", 0},
	{"保留",		"保留", 0},
	{"保留",		"保留", 0},
	{"装置告警",	"装置告警", 0},
	{"LAN网告警",	"LAN网告警",0},
	{"开关异常",	"开关异常", 0},
	{"保留",		"保留", 0},
};
#elif (DEV_SP == DEV_SP_CTRL)
static VDefTVYxCfg PDefTVYxCfg_Public[] = 
{
	{"装置告警",	"装置告警", 0},
	{"LAN网告警",	"LAN网告警",0},
	{"开关异常",	"开关异常", 0},
	{"保留",		"保留", 0},
};	
#elif (DEV_SP == DEV_SP_DTU_IU)||(DEV_SP == DEV_SP_DTU_PU)
static VDefTVYxCfg PDefTVYxCfg_Public[] = 
{
	{"保留",		"保留", 0},
	{"保留",		"保留", 0},
	{"保留",		"保留", 0},
	{"保留",		"保留", 0},
	{"本地",	    	"本地", 0},
	{"远方",		"远方", 0},
	{"闭锁",		"闭锁", 0},
	{"备用",	    "备用", 0},
	{"装置告警",    "装置告警", 0},
       {"LAN网告警",   "LAN网告警",0},
       {"开关异常",    "开关异常", 0},
       {"保留",        "保留", 0},
};	
#else
static VDefTVYxCfg PDefTVYxCfg_Public[] = 
{
    {"电源故障",    "电源故障", 0},
	{"电池欠压",	"电池欠压", 0},
	{"电池活化",	"电池活化", 0},
	{"失电告警",	"失电告警", 0},

	{"本地",	    "本地", 0},
	{"远方",		"远方", 0},
	{"闭锁",		"闭锁", 0},
#if (DEV_SP == DEV_SP_DTU)
	{"柜门开启",	"柜门开启", 0},
#else
	{"备用",	    "备用", 0},
#endif
        {"装置告警",    "装置告警", 0},
        {"LAN网告警",   "LAN网告警",0},
        {"开关异常",    "开关异常", 0},
        {"保留",        "保留", 0},

};	
#endif

static VDefTVYxCfg PDefTVYxCfg_Comm[] = 
{
	{"通信状态",		"通信状态", 0},		
	{"板卡通信状态[%d]",		"通信状态[%d]", 0},    	
};		

#ifdef INCLUDE_YX
#if (DEV_SP == DEV_SP_FTU)
static WORD Di_BitMap_3[] = 
{
	0x01FF
}; 

static BYTE YxNo_Dio_3[] = 
{
	0,1,2,3,4,5,6,7,8
}; 
   
static VDefTVYxCfg PDefTVYxCfg_Dio_3[] = 
{
    {"SYX0开关1合位",     "SYX0合位1", 0},
	{"SYX1",	          "SYX1", 0},
	{"SYX2",	          "SYX2", 0},
	{"SYX3",	          "SYX3", 0},
	{"SYX4",              "SYX4", 0},
    {"SYX5",              "SYX5", 0},
    {"SYX6",              "SYX6", 0},
    {"SYX7",              "SYX7", 0},
    {"SYX8",              "SYX8", 0},

};		

static VDefTVYxCfg PDefTVDYxCfg_Dio_3[] = 
{
    {"DYX0开关1状态",     "DYX0开关1", 0},
	{"DYX1",	  "DYX1", 0},
	{"DYX2",	  "DYX2", 0},
    {"DYX3",      "DYX3", 0},
};
#endif

#if (DEV_SP == DEV_SP_CTRL)
static WORD Di_BitMap_3[] = 
{
	0x03FF
}; 

static BYTE YxNo_Dio_3[] = 
{
	0,1,2,3,4,5,6,7,8,9
}; 
   
static VDefTVYxCfg PDefTVYxCfg_Dio_3[] = 
{
    {"SYX0",              "SYX0", 0},
    {"SYX1",              "SYX1", 0},
	{"SYX2",	          "SYX2", 0},
	{"SYX3",	          "SYX3", 0},
	{"SYX4",              "SYX4", 0},
    {"SYX5",              "SYX5", 0},
    {"SYX6",              "SYX6", 0},
    {"SYX7",              "SYX7", 0},
    {"SYX8",              "SYX8", 0},
    {"SYX9",              "SYX9", 0},

};		

static VDefTVYxCfg PDefTVDYxCfg_Dio_3[] = 
{
    {"DYX0",      "DYX0", 0},
    {"DYX1",     "DYX1", 0},
	{"DYX2",	  "DYX2", 0},
    {"DYX3",      "DYX3", 0},
    {"DYX4",      "DYX4", 0},
};

#endif

#if (DEV_SP == DEV_SP_WDG)
static WORD Di_BitMap_2[] = 
{
	0x001F
}; 

static BYTE YxNo_Dio_2[] = 
{
	0,1,2,3,4
}; 

static VDefTVYxCfg PDefTVYxCfg_Dio_2[] = 
{
    {"SYX0开关1合位",     "SYX0合位1", 0},
	{"SYX1",	          "SYX1", 0},
	{"SYX2",	          "SYX2", 0},
	{"SYX3",	          "SYX3", 0},
	{"SYX4",              "SYX4", 0},
};		

static VDefTVYxCfg PDefTVDYxCfg_Dio_2[] = 
{
    {"DYX0开关1状态",     "DYX0开关1", 0},
	{"DYX1",	  "DYX1", 0},
};
static WORD Di_BitMap_6[] = 
{
	0x01FF
}; 

static BYTE YxNo_Dio_6[] = 
{
	0,1,2,3,4,5,6,7,8,
}; 

static VDefTVYxCfg PDefTVYxCfg_Dio_6[] = 
{
    {"SYX0开关1合位",     "SYX0合位1", 0},
	{"SYX1",	          "SYX1", 0},
	{"SYX2",	          "SYX2", 0},
	{"SYX3",	          "SYX3", 0},
	{"SYX4",              "SYX4", 0},
	{"SYX5",              "SYX5", 0},
	{"SYX6",              "SYX6", 0},
	{"SYX7",              "SYX7", 0},
	{"SYX8",              "SYX8", 0},
	
};		

static VDefTVYxCfg PDefTVDYxCfg_Dio_6[] = 
{
    {"DYX0开关1状态",     "DYX0开关1", 0},
	{"DYX1",	  "DYX1", 0},
	{"DYX2",	  "DYX2", 0},
	{"DYX3",	  "DYX3", 0},
};
#endif

#if (DEV_SP == DEV_SP_DTU)
static WORD Di_BitMap_1[] = 
{
	0x00FF, 
	0x00FF, 
	0x00FF,
	0x00FF,
	0x000F
}; 

static BYTE YxNo_Dio_1[] = 
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	10,11,12,13,14,15,16,17,18,19,
	20,21,22,23,24,25,26,27,28,29,
	30,31,32,33,34,35,
}; 

static VDefTVYxCfg PDefTVYxCfg_Dio_1[] = 
{
    {"SYX%d开关%d合位",        "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",	       "SYX%d分位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d开关%d合位",	       "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",	       "SYX%d分位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d开关%d合位",	       "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",        "SYX%d分位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d开关%d合位",	       "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",	       "SYX%d合位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d开关%d合位",	       "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",        "SYX%d分位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d开关%d合位",	       "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",	       "SYX%d分位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
};		

static VDefTVYxCfg PDefTVDYxCfg_Dio_1[] = 
{
    {"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
};


static WORD Di_BitMap_8[] = 
{
	0x00FF, 
	0x00FF, 
	0x0003,
}; 

static BYTE YxNo_Dio_8[] = 
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	10,11,12,13,14,15,16,17,
}; 

static VDefTVYxCfg PDefTVYxCfg_Dio_8[] = 
{
  {"SYX%d开关%d合位",        "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",	       "SYX%d分位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d开关%d合位",	       "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",	       "SYX%d分位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d开关%d合位",	       "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",        "SYX%d分位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
};		

static VDefTVYxCfg PDefTVDYxCfg_Dio_8[] = 
{
  {"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
};


static WORD Di_BitMap_10[] = 
{
	0x00FF, 
	0x000F, 
}; 

static BYTE YxNo_Dio_10[] = 
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	10,11,
}; 

static VDefTVYxCfg PDefTVYxCfg_Dio_10[] = 
{
  {"SYX%d开关%d合位",        "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",	       "SYX%d分位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d开关%d合位",	       "SYX%d合位%d", 0},
	{"SYX%d开关%d分位",	       "SYX%d分位%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",                  "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
	{"SYX%d",	               "SYX%d", 0},
};		

static VDefTVYxCfg PDefTVDYxCfg_Dio_10[] = 
{
  {"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
	{"DYX%d",     "DYX%d", 0},
	{"DYX%d",	  "DYX%d", 0},
};


#endif

#if (DEV_SP == DEV_SP_TTU)
static WORD Di_BitMap_5[] = 
{
	0x00FF
}; 

static BYTE YxNo_Dio_5[] = 
{
	0,1,2,3,4,5,6,7
}; 
   
static VDefTVYxCfg PDefTVYxCfg_Dio_5[] = 
{
    {"SYX0开关1合位",     "SYX0合位1", 0},
	{"SYX1",	          "SYX1", 0},
	{"SYX2",	          "SYX2", 0},
	{"SYX3",	          "SYX3", 0},
	{"SYX4",              "SYX4", 0},
    {"SYX5",              "SYX5", 0},
    {"SYX6",              "SYX6", 0},
    {"SYX7",              "SYX7", 0},
};		

static VDefTVYxCfg PDefTVDYxCfg_Dio_5[] = 
{
    {"DYX0开关1状态",     "DYX0开关1", 0},
	{"DYX1",	  "DYX1", 0},
	{"DYX2",	  "DYX2", 0},
    {"DYX3",      "DYX3", 0},
};
#endif
#if(DEV_SP == DEV_SP_DTU_IU)
static WORD Di_BitMap_7[] = 
{
	0x00FF
}; 

static BYTE YxNo_Dio_7[] = 
{
	0,1,2,3,4,5,6,7
}; 
   
static VDefTVYxCfg PDefTVYxCfg_Dio_7[] = 
{
    {"SYX0开关1合位",     "SYX0合位1", 0},
	{"SYX1",	          "SYX1", 0},
	{"SYX2",	          "SYX2", 0},
	{"SYX3",	          "SYX3", 0},
	{"SYX4",              "SYX4", 0},
    {"SYX5",              "SYX5", 0},
    {"SYX6",              "SYX6", 0},
    {"SYX7",              "SYX7", 0},
};		

static VDefTVYxCfg PDefTVDYxCfg_Dio_7[] = 
{
    {"DYX0开关1状态",     "DYX0开关1", 0},
	{"DYX1",	  "DYX1", 0},
	{"DYX2",	  "DYX2", 0},
    {"DYX3",      "DYX3", 0},
};
#endif
#if(DEV_SP == DEV_SP_DTU_PU)
static WORD Di_BitMap_9[] = 
{
	0x003F
}; 

static BYTE YxNo_Dio_9[] = 
{
	0,1,2,3,4,5
}; 
   
static VDefTVYxCfg PDefTVYxCfg_Dio_9[] = 
{
    {"SYX0开关1合位",     "SYX0合位1", 0},
	{"SYX1",	          "SYX1", 0},
	{"SYX2",	          "SYX2", 0},
	{"SYX3",	          "SYX3", 0},
	{"SYX4",              "SYX4", 0},
    {"SYX5",              "SYX5", 0},
};		

static VDefTVYxCfg PDefTVDYxCfg_Dio_9[] = 
{
    {"DYX0开关1状态",     "DYX0开关1", 0},
	{"DYX1",	  "DYX1", 0},
	{"DYX2",	  "DYX2", 0},
};
#endif

#endif

static VDefDiCfg defMyDiCfg[] = 
{
	{0, 0, 0, NULL, NULL, 0, NULL, sizeof(PDefTVYxCfg_Public)/sizeof(PDefTVYxCfg_Public[0]), PDefTVYxCfg_Public,  0, NULL},
#ifdef INCLUDE_YX
#if ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_CTRL))
	{3, sizeof(Di_BitMap_3)/sizeof(Di_BitMap_3[0]), sizeof(YxNo_Dio_3)/sizeof(YxNo_Dio_3[0]), (void *)Di_BitMap_3, (void *)YxNo_Dio_3, sizeof(PDefTVYxCfg_Dio_3)/sizeof(PDefTVYxCfg_Dio_3[0]), PDefTVYxCfg_Dio_3, 0, NULL,  sizeof(PDefTVDYxCfg_Dio_3)/sizeof(PDefTVDYxCfg_Dio_3[0]), PDefTVDYxCfg_Dio_3},
#endif	
#if (DEV_SP == DEV_SP_WDG)
	{2, sizeof(Di_BitMap_2)/sizeof(Di_BitMap_2[0]), sizeof(YxNo_Dio_2)/sizeof(YxNo_Dio_2[0]), (void *)Di_BitMap_2, (void *)YxNo_Dio_2, sizeof(PDefTVYxCfg_Dio_2)/sizeof(PDefTVYxCfg_Dio_2[0]), PDefTVYxCfg_Dio_2, 0, NULL,  sizeof(PDefTVDYxCfg_Dio_2)/sizeof(PDefTVDYxCfg_Dio_2[0]), PDefTVDYxCfg_Dio_2},
	{6, sizeof(Di_BitMap_6)/sizeof(Di_BitMap_6[0]), sizeof(YxNo_Dio_6)/sizeof(YxNo_Dio_6[0]), (void *)Di_BitMap_6, (void *)YxNo_Dio_6, sizeof(PDefTVYxCfg_Dio_6)/sizeof(PDefTVYxCfg_Dio_6[0]), PDefTVYxCfg_Dio_6, 0, NULL,  sizeof(PDefTVDYxCfg_Dio_6)/sizeof(PDefTVDYxCfg_Dio_6[0]), PDefTVDYxCfg_Dio_6},
#endif	
#if (DEV_SP == DEV_SP_DTU)
	{1, sizeof(Di_BitMap_1)/sizeof(Di_BitMap_1[0]), sizeof(YxNo_Dio_1)/sizeof(YxNo_Dio_1[0]), (void *)Di_BitMap_1, (void *)YxNo_Dio_1, sizeof(PDefTVYxCfg_Dio_1)/sizeof(PDefTVYxCfg_Dio_1[0]), PDefTVYxCfg_Dio_1, 0, NULL,  sizeof(PDefTVDYxCfg_Dio_1)/sizeof(PDefTVDYxCfg_Dio_1[0]), PDefTVDYxCfg_Dio_1},
  {8, sizeof(Di_BitMap_8)/sizeof(Di_BitMap_8[0]), sizeof(YxNo_Dio_8)/sizeof(YxNo_Dio_8[0]), (void *)Di_BitMap_8, (void *)YxNo_Dio_8, sizeof(PDefTVYxCfg_Dio_8)/sizeof(PDefTVYxCfg_Dio_8[0]), PDefTVYxCfg_Dio_8, 0, NULL,  sizeof(PDefTVDYxCfg_Dio_8)/sizeof(PDefTVDYxCfg_Dio_8[0]), PDefTVDYxCfg_Dio_8},	
  {10, sizeof(Di_BitMap_10)/sizeof(Di_BitMap_10[0]), sizeof(YxNo_Dio_10)/sizeof(YxNo_Dio_10[0]), (void *)Di_BitMap_10, (void *)YxNo_Dio_10, sizeof(PDefTVYxCfg_Dio_10)/sizeof(PDefTVYxCfg_Dio_10[0]), PDefTVYxCfg_Dio_10, 0, NULL,  sizeof(PDefTVDYxCfg_Dio_10)/sizeof(PDefTVDYxCfg_Dio_10[0]), PDefTVDYxCfg_Dio_10},	
#endif
#if (DEV_SP == DEV_SP_TTU)
	{5, sizeof(Di_BitMap_5)/sizeof(Di_BitMap_5[0]), sizeof(YxNo_Dio_5)/sizeof(YxNo_Dio_5[0]), (void *)Di_BitMap_5, (void *)YxNo_Dio_5, sizeof(PDefTVYxCfg_Dio_5)/sizeof(PDefTVYxCfg_Dio_5[0]), PDefTVYxCfg_Dio_5, 0, NULL,  sizeof(PDefTVDYxCfg_Dio_5)/sizeof(PDefTVDYxCfg_Dio_5[0]), PDefTVDYxCfg_Dio_5},
#endif
#if (DEV_SP == DEV_SP_DTU_IU) 
	{7, sizeof(Di_BitMap_7)/sizeof(Di_BitMap_7[0]), sizeof(YxNo_Dio_7)/sizeof(YxNo_Dio_7[0]), (void *)Di_BitMap_7, (void *)YxNo_Dio_7, sizeof(PDefTVYxCfg_Dio_7)/sizeof(PDefTVYxCfg_Dio_7[0]), PDefTVYxCfg_Dio_7, 0, NULL,  sizeof(PDefTVDYxCfg_Dio_7)/sizeof(PDefTVDYxCfg_Dio_7[0]), PDefTVDYxCfg_Dio_7},
#endif	
#if (DEV_SP == DEV_SP_DTU_PU) 
	{9, sizeof(Di_BitMap_9)/sizeof(Di_BitMap_9[0]), sizeof(YxNo_Dio_9)/sizeof(YxNo_Dio_9[0]), (void *)Di_BitMap_9, (void *)YxNo_Dio_9, sizeof(PDefTVYxCfg_Dio_9)/sizeof(PDefTVYxCfg_Dio_9[0]), PDefTVYxCfg_Dio_9, 0, NULL,  sizeof(PDefTVDYxCfg_Dio_9)/sizeof(PDefTVDYxCfg_Dio_9[0]), PDefTVDYxCfg_Dio_9},
#endif	

#endif	
	{(DWORD)-1, 0, 0, NULL, NULL, 0, NULL, 0, NULL, 0, NULL}
};		

VDefDiCfg *GetDefDiCfg(DWORD type)
{
	VDefDiCfg *pcfg = defMyDiCfg;


    while (pcfg->ext_type != (DWORD)-1)
    {
		if (pcfg->ext_type == type) break;
		pcfg++;
	}

    if (pcfg->ext_type == (DWORD)-1)  return NULL;
	else return pcfg;
}

int GetDefPDiCfg(DWORD type, WORD num, struct VPDiCfg *pPCfg)
{
    int i;
	BYTE *yxno;
	VDefDiCfg *pcfg = GetDefDiCfg(type);

    if (pcfg == NULL) return ERROR;

    yxno = (BYTE *)pcfg->yxno;
	for (i=0; i<num; i++)
	{
		pPCfg->type = DITYPE_SYX;
		pPCfg->cfg = 0;
		pPCfg->dtime = DEFAULT_YX_DELAY_TIME;  //20ms
		pPCfg->yxno = yxno[i];   //GetDefDiNum
		pPCfg++;
	}

	return OK;
}

VDefTVYxCfg *GetDefTVYxCfg_Di(DWORD type, int *pnum)
{
	VDefDiCfg *pcfg = GetDefDiCfg(type);

    if (pcfg == NULL) 
	{
		*pnum = 0;
		return NULL;
    }	
	else
	{
		*pnum = pcfg->tvyxcfgnum_di;
		return(pcfg->ptvyxcfg_di);
	}
}

VDefTVYxCfg *GetDefTVYxCfg_Public(DWORD type, int *pnum)
{
	VDefDiCfg *pcfg = GetDefDiCfg(type);

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
		*pnum = pcfg->tvyxcfgnum_public;
		return(pcfg->ptvyxcfg_public);
#endif
	}
}

VDefTVYxCfg *GetDefTVYxCfg_Comm(void)
{
	return(PDefTVYxCfg_Comm);
}

VDefTVYxCfg *GetDefTVDYxCfg(DWORD type, int *pnum)
{
	VDefDiCfg *pcfg = GetDefDiCfg(type);

    if (pcfg == NULL) 
	{
		*pnum = 0;
		return NULL;
    }	
	else
	{
#ifdef INCLUDE_DYX
		*pnum = pcfg->tvdyxcfgnum;
		return(pcfg->ptvdyxcfg);
#else
		*pnum = 0;
		return(NULL);
#endif
	}
}

#if (TYPE_IO == IO_TYPE_MASTER)

static BYTE YxNo_ExtDio_1[] = 
{
	0,1,2,3,4,5,6,
	7,8,9,10,11,12,13,14,
	15,16,17,18,19
}; 
   
static VDefTVYxCfg PDefTVYxCfg_ExtDio_1[] = 
{
    {"SYX0[%d]",   			  "SYX0[%d]", 0},
	{"SYX1[%d]",			  "SYX1[%d]", 0},
	{"SYX2[%d]",			  "SYX2[%d]", 0},
	{"SYX3[%d]",			  "SYX3[%d]", 0},
	{"SYX4[%d]",              "SYX4[%d]", 0},
	{"SYX5[%d]",              "SYX5[%d]", 0},
	{"SYX6[%d]",	          "SYX6[%d]", 0},
	{"SYX7[%d]", 			  "SYX7[%d]", 0},
	{"SYX8[%d]", 			  "SYX8[%d]", 0},
	{"SYX9[%d]", 			  "SYX9[%d]", 0},
	{"SYX10[%d]", 			  "SYX10[%d]", 0},
	{"SYX11[%d]", 	          "SYX11[%d]", 0},
	{"SYX12[%d]", 	          "SYX12[%d]", 0},
	{"SYX13[%d]", 			  "SYX13[%d]", 0},
	{"SYX14[%d]", 			  "SYX14[%d]", 0},
	{"SYX15[%d]", 			  "SYX15[%d]", 0},
	{"SYX16[%d]",       	  "SYX16[%d]", 0},
	{"SYX17[%d]", 			  "SYX17[%d]", 0},
	{"SYX18[%d]", 	          "SYX18[%d]", 0},
	{"SYX19[%d]", 			  "SYX19[%d]", 0},
};		

static VDefTVYxCfg PDefTVDYxCfg_ExtDio_1[] = 
{
    {"DYX0[%d]",      "DYX0[%d]", 0},
	{"DYX1[%d]",	  "DYX1[%d]", 0},
	{"DYX2[%d]",	  "DYX2[%d]", 0},
	{"DYX3[%d]",	  "DYX3[%d]", 0},
	{"DYX4[%d]",	  "DYX4[%d]", 0},
	{"DYX5[%d]",	  "DYX5[%d]", 0},
	{"DYX6[%d]",	  "DYX6[%d]", 0},
	{"DYX7[%d]",	  "DYX7[%d]", 0},
	{"DYX8[%d]",	  "DYX8[%d]", 0},
	{"DYX9[%d]",	  "DYX9[%d]", 0},
};

static VDefDiCfg defExtDiCfg[] = 
{
	{0, 0, 0, NULL, NULL, 0, NULL, 0, NULL, 0, NULL},
	{1, 0, sizeof(YxNo_ExtDio_1)/sizeof(YxNo_ExtDio_1[0]), NULL, (void *)YxNo_ExtDio_1, sizeof(PDefTVYxCfg_ExtDio_1)/sizeof(PDefTVYxCfg_ExtDio_1[0]), PDefTVYxCfg_ExtDio_1, 0, NULL,  sizeof(PDefTVDYxCfg_ExtDio_1)/sizeof(PDefTVDYxCfg_ExtDio_1[0]), PDefTVDYxCfg_ExtDio_1},
	{2, 0, 0, NULL, NULL, 0, NULL, 0, NULL, 0, NULL},
	{3, 0, sizeof(YxNo_Dio_3)/sizeof(YxNo_Dio_3[0]), NULL, (void *)YxNo_Dio_3, sizeof(PDefTVYxCfg_Dio_3)/sizeof(PDefTVYxCfg_Dio_3[0]), PDefTVYxCfg_Dio_3, 0, NULL,  sizeof(PDefTVDYxCfg_Dio_3)/sizeof(PDefTVDYxCfg_Dio_3[0]), PDefTVDYxCfg_Dio_3},	
	{-1, 0, 0, NULL, NULL, 0, NULL, 0, NULL, 0, NULL}
};		

VDefDiCfg *GetDefExtDiCfg(DWORD type)
{
	DWORD ext_t;
	VDefDiCfg *pcfg = defExtDiCfg;

	ext_t = GetExtDioType(type);

    while (pcfg->ext_type != -1)
    {
		if (pcfg->ext_type == ext_t) break;
		pcfg++;
	}

    if (pcfg->ext_type == -1)  return NULL;
	else return pcfg;
}

int GetDefExtPDiCfg(DWORD type, WORD num, struct VPDiCfg *pPCfg)
{
    int i;
	BYTE *yxno;
	VDefDiCfg *pcfg = GetDefExtDiCfg(type);

    if (pcfg == NULL) return ERROR;

    yxno = (BYTE *)pcfg->yxno;
	for (i=0; i<num; i++)
	{
		pPCfg->type = DITYPE_SYX;
		pPCfg->cfg = 0;
		pPCfg->dtime = DEFAULT_YX_DELAY_TIME;  //20ms
		pPCfg->yxno = yxno[i];   //GetDefDiNum
		pPCfg++;
	}

	return OK;
}

VDefTVYxCfg *GetDefExtTVYxCfg_Di(DWORD type, int *pnum)
{
	VDefDiCfg *pcfg = GetDefExtDiCfg(type);

    if (pcfg == NULL) 
	{
		*pnum = 0;
		return NULL;
    }	
	else
	{
		*pnum = pcfg->tvyxcfgnum_di;
		return(pcfg->ptvyxcfg_di);
	}
}

VDefTVYxCfg *GetDefExtTVYxCfg_Public(DWORD type, int *pnum)
{
	VDefDiCfg *pcfg = GetDefExtDiCfg(type);

    if (pcfg == NULL) 
	{
		*pnum = 0;
		return NULL;
    }	
	else
	{
		*pnum = pcfg->tvyxcfgnum_public;
		return(pcfg->ptvyxcfg_public);
	}
}

VDefTVYxCfg *GetDefExtTVYxCfg_Comm(void)
{
	return(&PDefTVYxCfg_Comm[1]);
}

VDefTVYxCfg *GetDefExtTVDYxCfg(DWORD type, int *pnum)
{
	VDefDiCfg *pcfg = GetDefExtDiCfg(type);

    if (pcfg == NULL) 
	{
		*pnum = 0;
		return NULL;
    }	
	else
	{
#ifdef INCLUDE_DYX
		*pnum = pcfg->tvdyxcfgnum;
		return(pcfg->ptvdyxcfg);
#else
		*pnum = 0;
		return(NULL);
#endif
	}
}

#endif

