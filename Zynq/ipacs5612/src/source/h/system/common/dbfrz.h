/*------------------------------------------------------------------------
 Module:        dbfrz.h
 Author:        solar
 Project:       
 State:
 Creation Date:	2009-02-25
 Description:   
------------------------------------------------------------------------*/

#ifndef _DBFRZ_H
#define _DBFRZ_H

#ifdef __cplusplus
extern "C" {
#endif

#define FF_DATE_HOUR              1
#define FF_DATE_DAY               2
#define FF_DATE_MONTH             3

/*冻结文件名模版,FF = Freeze File*/
#define FF_HOUR_DLL			      "%s/frz/hour/h_%s_%s%s.dat"	/*小时冻结-名称-yc/dd-mm.dat*/
#define FF_DAY_DLL		      	  "%s/frz/day/d_%s_%s%s.dat"     /*日冻结-名称-yc/dd.dat*/
#define FF_MONTH_DLL		      "%s/frz/month/m_%s_%s%s.dat"   /*月冻结-名称-yc/dd.dat*/

#define FF_MAX_REC_LEN      (2*1024)
#define FF_NAME_LEN			(4*MAXFILENAME)			/*冻结文件名长度*/

enum{
	FF_TAB_YC = 0,
	FF_TAB_DD
};	

#define FF_TYPE_VALUE           0
#define FF_TYPE_MAXMIN          1


/*冻结密度类型*/
#define FF_MIDU_NULL			0		
#define FF_MIDU_1			    1		
#define FF_MIDU_2		      	2		
#define FF_MIDU_3		     	3		

extern int FF_MD_DEFAULT;    /*缺省冻结密度min*/

typedef struct{
	DWORD minute;
    DWORD offset;
}VFFIndexInfo;	

typedef struct{
	WORD wp;
	WORD len;
	WORD max;
	WORD num;
	VFFIndexInfo *info;
}VFFIndex;	

typedef struct{
	DWORD tab;
	int type;
	int date;
	int capacity;    /*容量 最大存储的点个数  0==系统默认*/
	WORD len;
	WORD md;         /*冻结分钟间隔  0不冻结; 会向后自动转换为最小冻结密度的倍数; 日冻结月冻结无效*/
	void (*get_ffdll_data)(WORD id, BYTE *buf);
}VFFDll;	

struct VFFDllMan{	
	WORD id;
	VFFIndex index;
	VFFDll dll;
	struct VFFDllMan *next;
};	

#pragma pack(1)
/*通用冻结文件头*/
typedef struct{
	DWORD minute;

	WORD rec_len;			    /*记录长度,不包含每项前四字节的分钟数*/
	WORD rec_max;			    /*最大记录数*/
	WORD rec_num;			    /*当前记录数*/
	WORD rec_wp;                /*写指针,暂未用*/
	WORD rsv[10];	 
}VFFHead;
#pragma pack()

#define FUNDLL_TYPE_DAY_RESET   0

typedef struct{
	int type;
	void *argc;
	void (*fun)(struct VSysClock *pclock, void *argc);
}VFunDll;	

struct VFunDllMan{	
	VFunDll dll;
	struct VFunDllMan *next;
};	

int FunDll_Regist(VFunDll *pdll);

void freeze(void);
int FFInit(struct VThSetupInfo *pInfo);

void FFReset(void);
int FFDll_CurveRead(WORD id, DWORD tab, int type, struct VSysClock *td, BYTE *pbuf, int min, DWORD rec_offset, WORD size, WORD num);

WORD ReadYCCurve(WORD wEqpID, WORD wNo, struct VSysClock *td, WORD wNum, int m, long *buf, BOOL asSend);
WORD ReadYCDay(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend);
WORD ReadYCMonth(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend);
WORD ReadYCMaxMin(WORD wEqpID, WORD wNo, struct VSysClock *td, struct VMaxMinYC *buf, BOOL asSend);
WORD ReadDDCurve(WORD wEqpID, WORD wNo, struct VSysClock *td, WORD wNum, int m, long *buf, BOOL asSend);
WORD ReadDDDay(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend);
WORD ReadDDMonth(WORD wEqpID, WORD wNo, struct VSysClock *td, long *buf, BOOL asSend);

#ifdef __cplusplus
}
#endif

#endif /*_LCTU_FREEZE_H*/

