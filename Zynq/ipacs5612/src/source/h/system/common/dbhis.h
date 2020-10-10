/*------------------------------------------------------------------------
 Module:        dbhis.h
 Author:        helen
 Project:       
 State:
 Creation Date:	2016-09-07
 Description:   
------------------------------------------------------------------------*/

#ifndef _DBHIS_H
#define _DBHIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mytypes.h"
#include "clock.h"
#include "fileext.h"

#define DBHIS_SOE_MAX         1024
#define DBHIS_CO_MAX          32
#define DBHIS_FLOW_MAX        256
#define DBHIS_LOG_MAX         1024
#define DBHIS_DAY_MAX         31
#define DBHIS_MIN_MAX         96
#define DBHIS_MIN_INTERVAL    15

#define DBHIS_FILE_SEG        1024
#define DBHIS_SOCKET_MAX      1024


#define YK_Addr   0x6001
#define YC_Addr   0x4001
#define YX_Addr   0x0001

enum
{
    DBhis_type_soe =0,
	DBhis_type_co,
	DBhis_type_exv,
	DBhis_type_fixpt,
	DBhis_type_frz,
	DBhis_type_flowrev,
	DBhis_type_ulog,
	DBhis_type_num,
};

struct VHisIndex{
	int wp_offset;     /*xml文件写data的offset*/
	int num_offset;
	WORD num;
	WORD head_len;
};	

struct VHisRcd
{
    int num;
	int num_wp;
	int file_size;
	int file_head;
	int file_wp;
	int file_crc;
	BYTE first;
};

struct VHisDll{
	DWORD tab;
	int type;
	int date;
	WORD len;
	WORD max;
	WORD md;               /*分钟间隔  0不冻结;*/
	int (*get_hisdll_data)(void *pdata, char *buf);
	int (*create_hisdll_file)(FILE* fp, struct VHisDll *pdll, struct VHisRcd *pcfg);
};



struct VHisDllMan{	
	WORD id;
	struct VHisDll dll;
	struct VHisRcd *index;
	struct VHisDllMan *next;
};	

struct VHisSocket{
    int read_write;  /*1-read 2-write 3-write imm 4-same for remove 0-none*/
	struct VHisDllMan *dllman;
	BYTE *buf;
};

struct VHisInfo
{
    int sec;
	int id;
	FILE *fp;
	struct VSysClock clock;
};

void his_init(void);
void his2file(void);
void his_Data_Write(void);
void his_Event_Write(void);
void his_cfg_write(void);
extern int his_cfg_change;

#ifdef __cplusplus
}
#endif
#endif
