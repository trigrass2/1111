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
    {"�����ܸ���",      "�����ܸ���", 0},
	{"FAͶ��",          "FAͶ��",   0},
};
#endif
#if(DEV_SP == DEV_SP_TTU)

static VDefTVYkCfg PDefTVYkCfg_Public2[] = 
{
	{"����������", 	"����������", 0},
	{"����",		    "����", 0},
	{"����",			"����",   0},
};
#elif(DEV_SP == DEV_SP_DTU_IU)
static VDefTVYkCfg PDefTVYkCfg_Public2[] = 
{
	{"����������",  "����������", 0},
	{"�����ܸ���",		"�����ܸ���", 0},
	{"FAͶ��",			"FAͶ��",   0},
#ifdef INCLUDE_FA_DIFF
	{"�FAͶ��",		"�FAͶ��",   0},
#endif
};
#else
static VDefTVYkCfg PDefTVYkCfg_Public2[] = 
{
	{"����˳�/�", 	"����˳�/�", 0},
	{"�����ܸ���",		"�����ܸ���", 0},
	{"FAͶ��",			"FAͶ��",   0},
#ifdef INCLUDE_FA_DIFF
	{"�FAͶ��",		"�FAͶ��",   0},
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

//���� ���ö˿� ��У�˿� ʹ�ܶ˿� ����λ ��Уλ ʹ��λ ʹ���鶨�� ��ʱ
//ʹ���鶨��    ���м̵���������, �̵���1��ӦD0, ����
//              Ȼ�󽫸ü̵�����Ӧʹ�̵ܼ�������Ͻ�����м̵���������������
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

//���� ��� ִ�� ״̬

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
    {"����1",      "����1", 0},
	{"����2", 	   "����2", 0},
	{"����3",      "����3", 0},
	{"����4", 	   "����4", 0},
	{"����5",      "����5", 0},
	{"����6", 	   "����6", 0},
	{"����7",      "����7", 0},
	{"����8", 	   "����8", 0},
	{"����9",      "����9", 0},
	{"����10", 	   "����10", 0},
	
};
#endif

#if (DEV_SP == DEV_SP_FTU)

VYkPort  YkPort_Dio_3[] = 
{
	{BSP_PORT_YK,   0x002F,   0,   0,   1}, 
	{BSP_PORT_YKFJ, 0x0001,   0,   0,   0},  

};

//���� ���ö˿� ��У�˿� ʹ�ܶ˿� ����λ ��Уλ ʹ��λ ʹ���鶨�� ��ʱ
//ʹ���鶨��    ���м̵���������, �̵���1��ӦD0, ����
//              Ȼ�󽫸ü̵�����Ӧʹ�̵ܼ�������Ͻ�����м̵���������������
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

//���� ��� ִ�� ״̬
VYkPoint YkPoint_Dio_3[] = 
{
	{0, 0, 0, 0,},
	{2, 3, 1, 0,},
	{3, 2, 1, 0,},
	{4, 5, 0, 0,},
	{5, 4, 0, 0,},
	{6, -1, -1, 0,},
};
 //wdj   ��ػ ����ͼֽ�ֺ�����,�ߵ��ֺϡ�
static VDo2Point Do2Point_Dio_3[] = 
{
	{0, 2, 1,},
	{0, 5, 0,},
    {0, 4, 3,},		                       
};   
 static VDefTVYkCfg PDefTVYkCfg_Dio_3[] = 
 {
   {"����1��/��",	   "����1��/��", 0},
	 {"����",			 "����", 0},
 };
#endif

#if (DEV_SP == DEV_SP_WDG)
VYkPort  YkPort_Dio_2[] = 
{
	{BSP_PORT_YK,   0x002F,   0,   0,   1}, 
	{BSP_PORT_YKFJ, 0x0001,   0,   0,   0},  

};

//���� ���ö˿� ��У�˿� ʹ�ܶ˿� ����λ ��Уλ ʹ��λ ʹ���鶨�� ��ʱ
//ʹ���鶨��    ���м̵���������, �̵���1��ӦD0, ����
//              Ȼ�󽫸ü̵�����Ӧʹ�̵ܼ�������Ͻ�����м̵���������������
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

//���� ��� ִ�� ״̬
VYkPoint YkPoint_Dio_2[] = 
{
	{0, 0, 0, 0,},
	{2, 3, 1, 0,},
	{3, 2, 1, 0,},
	{4, 5, 0, 0,},
	{5, 4, 0, 0,},
	{6, -1, -1, 0,},
};
 //wdj   ��ػ
static VDo2Point Do2Point_Dio_2[] = 
{
	{0, 2, 1,},
	{0, 5, 0,},
  {0, 3, 4,},
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_2[] = 
{
  {"����1��/��",      "����1��/��", 0},
	{"����", 	        "����", 0},
};

// ����ʽ
VYkPort  YkPort_Dio_6[] = 
{
	{BSP_PORT_YK,   0x002F,   0,   0,   1}, 
	{BSP_PORT_YKFJ, 0x0001,   0,   0,   0},  

};

//���� ���ö˿� ��У�˿� ʹ�ܶ˿� ����λ ��Уλ ʹ��λ ʹ���鶨�� ��ʱ
//ʹ���鶨��    ���м̵���������, �̵���1��ӦD0, ����
//              Ȼ�󽫸ü̵�����Ӧʹ�̵ܼ�������Ͻ�����м̵���������������
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

//���� ��� ִ�� ״̬
VYkPoint YkPoint_Dio_6[] = 
{
	{0, 0, 0, 0,},
	{2, 3, 1, 0,},
	{3, 2, 1, 0,},
	{4, 5, 0, 0,},
	{5, 4, 0, 0,},
	{6, -1, -1, 0,},
};
 //wdj   ��ػ
static VDo2Point Do2Point_Dio_6[] = 
{
	{0, 2, 1,},
	{0, 5, 0,},
  {0, 3, 4,},
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_6[] = 
{
  {"����1��/��",      "����1��/��", 0},
	{"����", 	        "����", 0},
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
  {"����%d��/��",      "����%d��/��", 0},
  {"����%d��/��",      "����%d��/��", 0},
  {"����%d��/��",      "����%d��/��", 0},
  {"����%d��/��",      "����%d��/��", 0},
  {"����%d��/��",      "����%d��/��", 0},
  {"����%d��/��",      "����%d��/��", 0},
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
  {"����%d��/��",      "����%d��/��", 0},
  {"����%d��/��",      "����%d��/��", 0},
  {"����%d��/��",      "����%d��/��", 0},
  {"����%d��/��",      "����%d��/��", 0},
};

/*���12YX2YK�İ忨  ң���ڵ�1 (��CPU4)��ң�Ű��� 2016��9��19��13:53:27 cjl*/
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
  {"����%d��/��",      "����%d��/��", 0},
  {"����%d��/��",      "����%d��/��", 0},
};

#endif

#if (DEV_SP == DEV_SP_TTU)
VYkPort  YkPort_Dio_5[] = 
{
	{BSP_PORT_YK,   0x002F,   0,   0,   1}, 
	{BSP_PORT_YKFJ, 0x0003,   0,   0,   0},  

};

//���� ���ö˿� ��У�˿� ʹ�ܶ˿� ����λ ��Уλ ʹ��λ ʹ���鶨�� ��ʱ
//ʹ���鶨��    ���м̵���������, �̵���1��ӦD0, ����
//              Ȼ�󽫸ü̵�����Ӧʹ�̵ܼ�������Ͻ�����м̵���������������
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

//���� ��� ִ�� ״̬
VYkPoint YkPoint_Dio_5[] = 
{
	{0, 0, 0, 0,},
	{2, 3, 1, 0,},
	{3, 2, 1, 0,},
	{5, 6, 4, 0,},
	{6, 5, 4, 0,},
	{-1, -1, -1, 0,},

};
 //wdj   ��ػ ����ͼֽ�ֺ�����,�ߵ��ֺϡ�
static VDo2Point Do2Point_Dio_5[] = 
{
	{0, 2, 1,},
	{0, 4, 3,},
	{1, 5, 0,},		                       
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_5[] = 
{
    {"����1��/��",    "����1��/��", 0},
    {"����1��/��", 	  "����1��/��", 0},
};

#endif
#if (DEV_SP == DEV_SP_DTU_IU)
VYkPort  YkPort_Dio_7[] = 
{
	{BSP_PORT_YK,   0x002F,   0,   0,   1}, 
	{BSP_PORT_YKFJ, 0x0003,   0,   0,   0},  

};

//���� ���ö˿� ��У�˿� ʹ�ܶ˿� ����λ ��Уλ ʹ��λ ʹ���鶨�� ��ʱ
//ʹ���鶨��    ���м̵���������, �̵���1��ӦD0, ����
//              Ȼ�󽫸ü̵�����Ӧʹ�̵ܼ�������Ͻ�����м̵���������������
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

//���� ��� ִ�� ״̬
VYkPoint YkPoint_Dio_7[] = 
{
	{0, 0, 0, 0,},
	{2, 3, 1, 0,},
	{3, 2, 1, 0,},
	{5, 6, 4, 0,},
	{6, 5, 4, 0,},
	{-1, -1, -1, 0,},
};
 //wdj   ��ػ ����ͼֽ�ֺ�����,�ߵ��ֺϡ�
static VDo2Point Do2Point_Dio_7[] = 
{
	{0, 2, 1,},
	{0, 4, 3,},
    {1, 5, 0,},		                       
};   

static VDefTVYkCfg PDefTVYkCfg_Dio_7[] = 
{
  {"����1��/��", 	  "����1��/��", 0},
    {"����1��/��",      "����1��/��", 0},
};
#endif
#endif

//���û���е�ػYK, �����ѡ��PDefTVYkCfg_Public1, ͬʱdonumΪDo2Point
//����е�ػYK, �����ѡ��PDefTVYkCfg_Public2, ͬʱdonumΪDo2Point-1
//������������ϸ�����,��������ػ��ʱ��ᵼ������
//Ҳ����˵���е�ػ���˳���ʱ��,���������YKΪ���ڽڵ�,����YK�б�������
//������ת��������ݿ��е�"��ػ"���, �����õ��ʱ��, ��donum+1 ��YK
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

