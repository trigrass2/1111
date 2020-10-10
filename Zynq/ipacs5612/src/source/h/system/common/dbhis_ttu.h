/*------------------------------------------------------------------------
 Module:        dbhis.h
 Author:        helen
 Project:       
 State:
 Creation Date:	2016-09-07
 Description:   
------------------------------------------------------------------------*/

#ifndef _DBHISTTU_H
#define _DBHISTTU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mytypes.h"
#include "clock.h"
#include "fileext.h"

#define DBHISTTU_SOE_MAX         1024
#define DBHISTTU_CO_MAX          32
#define DBHISTTU_FLOW_MAX        256
#define DBHISTTU_LOG_MAX         1024
#define DBHISTTU_DAY_MAX         31
#define DBHISTTU_MIN_MAX         96
#define DBHISTTU_MIN_INTERVAL    15

#define DBHISTTU_FILE_SEG        1024
#define DBHISTTU_SOCKET_MAX      1024

#define DD_Addr   0x6401
#define YK_Addr   0x6001
#define YC_Addr   0x4001
#define YX_Addr   0x0001

enum
{
  DBhisttu_type_soe =0, 
	DBhisttu_type_co,
	DBhisttu_type_hl, //
	DBhisttu_type_ol, //
	DBhisttu_type_exv,    //
	DBhisttu_type_fixpt,  //
	DBhisttu_type_frz,    //
	DBhisttu_type_voltd,  //
	DBhisttu_type_volttd, //
	DBhisttu_type_voltm,  // 
	DBhisttu_type_volttm, //
	DBhisttu_type_flowrev,
	DBhisttu_type_ulog,
	DBhisttu_type_num,
};

struct VHisTtuIndex{
	int wp_offset;     /*xml文件写data的offset*/
	int num_offset;
	WORD num;
	WORD head_len;
};	

struct VHisTtuRcd
{
    int num;
	int num_wp;
	int file_size;
	int file_head;
	int file_wp;
	int file_crc;
	BYTE first;
};

struct VHisTtuDll{
	DWORD tab;
	int type;
	int date;
	WORD len;
	WORD max;
	WORD md;               /*分钟间隔  0不冻结;*/
	int (*get_histtudll_data)(void *pdata, char *buf);
	int (*create_histtudll_file)(FILE* fp,const struct VHisTtuDll *pdll, struct VHisTtuRcd *pcfg);
};



struct VHisTtuDllMan{	
	WORD id;
	struct VHisTtuDll dll;
	struct VHisTtuRcd *index;
	struct VHisTtuDllMan *next;
};	

struct VHisTtuSocket{
    int read_write;  /*1-read 2-write 3-write imm 4-same for remove 0-none*/
	struct VHisTtuDllMan *dllman;
	BYTE *buf;
};

struct VHisTtuInfo
{
    int sec;
	int id;
	FILE *fp;
	struct VSysClock clock;
};

void his_ttu_init(void);
void histtu2file(void);
void histtu_Data_Write(void);
void histtu_Event_Write(void);
void histtu_cfg_write(void);
extern int histtu_cfg_change;

#ifdef __cplusplus
}
#endif
#endif
