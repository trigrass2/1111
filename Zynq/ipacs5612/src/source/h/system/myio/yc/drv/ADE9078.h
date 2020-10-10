#ifndef _ADE9078_H
#define	_ADE9078_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include "mytypes.h"
#include "yc.h"

typedef struct 
{ 
	unsigned int regaddr; 
	unsigned int reglen; 
	unsigned int batchmode; 
	unsigned char *regdata; 
} adc_reg_req_st;

typedef union
{
	char uc_Register[4];
	short us_Register[2];
	int  ul_Register;
} ADE_REGISTER_UNION;

typedef struct 
{
	struct VSysClock DD_Time;
	float DD_JFPG;
} Ade9078_DD_JFPG;

enum _Name { Ia, Ua, Ib, Ub, Ic, Uc, I0, Max };

//寄存器地址（读）
#define		PART_ID		0x472
#define		AVRMS		0x20D
#define		BVRMS		0x22D
#define		CVRMS		0x24D
#define		AIRMS		0x20C
#define		BIRMS		0x22C
#define		CIRMS		0x24C
#define		NIRMS		0x266
#define		AWATT		0x210
#define		BWATT		0x230
#define		CWATT		0x250
#define		AVAR		0x211
#define		BVAR		0x231
#define		CVAR		0x251
#define		AVA			0X212
#define		BVA			0x232
#define		CVA			0x252
#define		APF			0x216
#define		BPF			0x236
#define		CPF			0x256
#define		PWATT_ACC	0x397
#define		NWATT_ACC	0x39B
#define		PVAR_ACC	0x39F
#define		NVAR_ACC	0x3A3
#define		AWATTHR_LO	0x2E6
#define		AWATTHR_HI	0x2E7
#define		BWATTHR_LO	0x322
#define		BWATTHR_HI	0x323
#define		CWATTHR_LO	0x35E
#define		CWATTHR_HI	0x35F
#define		AVARHR_LO	0x2F0
#define		AVARHR_HI 	0x2F1
#define		BVARHR_LO	0x32C
#define		BVARHR_HI	0x32D
#define		CVARHR_LO	0x368
#define		CVARHR_HI	0x369
#define		AVAHR_LO	0x2FA
#define		AVAHR_HI	0x2FB
#define		BVAHR_LO	0x336
#define		BVAHR_HI	0x337
#define		CVAHR_LO	0x372
#define		CVAHR_HI	0x373

//寄存器地址（写）
#define		AIGAIN		0x000
#define		BIGAIN		0x020
#define		CIGAIN		0x040
#define		NIGAIN		0x06D
#define		AVGAIN		0x00B
#define		BVGAIN		0x02B
#define		CVGAIN		0x04B
#define		APHCAL0		0x006
#define		BPHCAL0		0x026
#define		CPHCAL0		0x046
#define		APGAIN		0x00E
#define		BPGAIN		0x02E
#define		CPGAIN		0x04E
#define		WFB_CFG		0x4A0
#define		STATUS0		0x402

#define		APERIOD		0x418
#define		BPERIOD		0x419
#define		CPERIOD		0x41A
#define		COM_PERIOD	0x41B

#define     PHSIGN      0x49D //0为正 1为负
#define     AWSIGN      0x01
#define     AVARSIGN    0x02
#define     BWSIGN      0x04
#define     BVARSIGN    0x08
#define     CWSIGN      0x10
#define     CVARSIGN    0x20


//系数
#define	Kv		(17765939.0/264.0)
#define Ki		(7461540.0/6.0)
#define Kw		(987659/1.584)
#define MaxSampPointNum           128        // 电压、电流谐波采样点数
#define POWER_REV (15000*5/1000) //潮流反向下限0.5% 额定功率

#define ADC_IO_READ		_IOW('D', 1, adc_reg_req_st)
#define ADC_IO_WRITE	_IOW('D', 2, adc_reg_req_st)
#define ADC_IO_DISABLE	_IOW('D', 3, adc_reg_req_st)
#define ADC_IO_ENABLE	_IOW('D', 4, adc_reg_req_st)

//初始化
int ADE9078_Init(void);

//反初始化
void ADE9078_Fini(void);

//复位
void ADE9078_Reset(void);

//读电压
int ADE9078_ReadU(int iIdx, long *value);

//读电流
int ADE9078_ReadI(int iIdx, long *value);

//读总有功功率
int ADE9078_ReadP(int iIdx, long *value);

//读总无功功率
int ADE9078_ReadQ(int iIdx, long *value);

//读总视在功率
int ADE9078_ReadS(long *value);

//读功率因数
int ADE9078_ReadPF(int iIdx, float *value);

//读电网频率
int ADE9078_Readf(STATUS iIdx, long *value);

//读有功电能
int ADE9078_Readw(STATUS iIdx, long *value);

//读无功电能
int ADE9078_ReadV(STATUS iIdx, long *value);

//读视在电能
int ADE9078_ReadAV(STATUS iIdx, long *value);

//读正、反向总有功功率和总无功功率
int ADE9078_ReadPMPQ(STATUS iType, long *value);

//读64点采样波形
int ADE9078_ReadWaveform(void);

//设置校准
int ADE9078_SetConfig();

//校准电流
int ADE908_GainI(int iIdx, int fReadI, int iStandardI,VYcGain* pycgain);

//校准电压
int ADE9078_GainU(int iIdx, int fReadU, int iStandardU,VYcGain* pycgain);

//校准功率
int ADE9078_GainP(int iIdx, float err);

//校准相位
int ADE9078_GainPhase(STATUS iIdx,int fReadU, int iStandardU, VYcGain* pycgain);

//清校表
int ADE9078_GainClear(void);

void Ade9078Dd_Init(void);

void Ade9078_ReadEP(void);

long Ade9078_GetEP(int chn);

void Ade9078_Eqp(void);

void Ade9078Dd_Clear(void);


#endif // !_ADE9078_H
