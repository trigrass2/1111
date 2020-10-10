
#ifndef _ATT7022_H
#define _ATT7022_H

#include <unistd.h> 
#include <termios.h> 
#include <unistd.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/ioctl.h> 

#include "syscfg.h"

#define MaxSampPointNum           128        // ��ѹ������г����������
#define ATT7022E_FFT   32     // ����FFT�����������
//#define RATED_CURR_VALUE          (1.5)
#define RATED_CURR_VALUE          (5)
/* ATT7022E�������йص��� �����Ĵ�������Ƶ���峣������������Ĵ��������������Ĵ��� 0X1D��
��λ���� ����޹�*/

#define I_NORT_ZONE                  0
#define I_LOWT_ZONE                  1
#define I_HIGHT_ZONE                 2
#define I_ZONE_NUM                   5

#define TEMP_LOWEST                  (-50)
#define TEMP_HIGHEST                 90
#define TEMP_NORMAL_LOW              10
#define TEMP_NORMAL_HIGH             55
#define TEMP_LOW_CMP                 (-40)
#define TEMP_STEP_CMP                5
#define TEMP_ZONE_MAX                25
#define TEMP_ZONE_NOR_LOW            10    // �����²����¶���������
#define TEMP_ZONE_NOR_HIGH           19    // �����²����¶���������
#define TEMP_ZONE_LOW_NUM            14    // ���¶�14,���¶�10
#define ALLGAIN_LOW_TEMP_MAX         130   // ����ȫͨ��������ֵ��err=0.8%
//#define ALLGAIN_HIGH_TEMP_MAX        (49)    // ����ȫͨ��������ֵ��err=0.3%
#define ALLGAIN_HIGH_TEMP_MAX         130   // ����ȫͨ��������ֵ��err=0.8%

#define ATT7022E_GAIN_VER1           0x80AA0001

#define DATAU                    0
#define DATAI                    4
#define DATAS                    10
#define DATAP                    14
#define DATAQ                    18
#define DATAUIAngle              22
#define DATAUaUbAngle            26
#define DATAFreq                 30
#define DATATPSD                 31
#define DATACOS                  32
#define DATAEpa                  36
#define DATAEqa                  40
#define DATAEsa                  44
#define DATAEFlag                48
#define DATASfalg                49
#define DATAPFlag                50
#define DATAChknum               51

#define DATAUCurBaseVal         52   /*������Чֵ Ua52 Ub53 Uc54*/
#define DATAICurBaseVal           55   /*������Чֵ Ia55 Ib56 Ic57*/

#define ATT7022_REG_UI           0x01
#define ATT7022_REG_PQ           0x02
#define ATT7022_REG_OTHYC        0x04
#define ATT7022_REG_E            0x08
#define ATT7022_REG_STS          0x10


#define 	PSIGN_MASK   0x00000008
#define 	QSIGN_MASK	0x00000080
#define 	EMU_PDIR_MASK   	0x00001000
#define 	EMU_QDIR_MASK		0x00002000
#define POWER_REV (15000*5/1000) //������������0.5% �����


typedef unsigned long long  INT64U;
typedef signed   long long  INT64S;

struct MIX_CONFIG_CELL_TYPE
{
    WORD Addr;
    WORD len;
    long Data;
};

typedef union 
{   // С��ģʽ�µ�˫�ֽ�������
    BYTE   Byte2[2];
    short  Sinteger;
    WORD  Uinteger;
}  __attribute__ ((packed))BYTE2_INT16_TYPE;


typedef struct 
{
	DWORD AT7022_DD_int;
	DWORD AT7022_DD_decimal;
} AT7022_DD;

typedef struct 
{
	struct VSysClock DD_Time;
	float DD_JFPG;
} AT7022_DD_JFPG;


typedef struct
{   
    char TempZone;
	BYTE TempZoneBak;
	BYTE ITempZone;
	BYTE ITempZoneBak;
	short TempCur;
	short TempBak;
}TEMP_INFO;

typedef struct
{
	BYTE  IZone[3];
	BYTE  IZoneBak[3];
    short IBak[3];
}CURRENT_INFO;

typedef struct{
    short TC;
	short allGain[25];   
	short allIAng[3][15];   //3*3*5;0~4�ε������� 
	short HarmGain[6][20];   /*г������*/
	WORD  crc;
}V7022Gain;

/* SPI�����¹ҵĴ��豸���������ݽṹ ��������ĳһ��SPI�¹ҵ��豸�ľ�����Ϣ */ 
#pragma pack(4) 
typedef struct 
{ 
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ 
#if (__SIZEOF_LONG__ == 4) 
	unsigned int    pad0; 
#endif 
#endif 
	char*     bus_name; 
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ 
#if (__SIZEOF_LONG__ == 4) 
	unsigned int    pad0; 
#endif 
#endif 
	unsigned int    mode; 
	unsigned int    speed; 
	unsigned char   bits_per_word; 
	unsigned char   protocal_type; 
	unsigned int    cs_id; 
	unsigned int    cmd_size; 
	unsigned int    rsv1[2]; 
} spi_dev_cfg_s; 
#pragma pack() 

/* �û�̬��װspi_tansfer�ӿ�ʱ��Ҫ���ݸ��ں˵���Ϣ�����ݽṹ���� */ 
/* ������ݽṹ������д�С������ Ҫ���Ǵ�С�� */ 
#pragma pack(4) 
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ 
typedef  struct spi_transfer 
{ 
	spi_dev_cfg_s *spi_dev;  
#if (__SIZEOF_LONG__ == 4) 
	unsigned int pad0; 
#endif 
	void * tx_buff; 
#if (__SIZEOF_LONG__ == 4) 
	unsigned int pad1; 
#endif 
	void * rx_buff; 
#if (__SIZEOF_LONG__ == 4) 
	unsigned int pad2; 
#endif 
	unsigned int tx_size; 
	unsigned int rx_size; 
} spi_transfer_s; 
#else 
typedef  struct spi_transfer 
{ 
#if (__SIZEOF_LONG__ == 4) 
	unsigned int pad0; 
#endif 
	spi_dev_cfg_s *spi_dev;  
#if (__SIZEOF_LONG__ == 4) 
	unsigned int pad1; 
#endif 
	void * tx_buff; 
#if (__SIZEOF_LONG__ == 4) 
	unsigned int pad2; 
#endif 
	void * rx_buff; 
	unsigned int tx_size; 
	unsigned int rx_size; 
} spi_transfer_s; 
#endif 
#pragma pack() 

/* SPI����ioctl�����ֶ��� */ 
#define SPI_BASE 'S' 
#define SPI_TRANSFER_DATA _IOW (SPI_BASE, 0, spi_transfer_s) 
#define SPI_NAME_MAX_LEN 32 
#define SPI_USER_NAME_MAX_LEN (SPI_NAME_MAX_LEN + 8) 

/* SPI���豸����ģʽ�궨�� */ 
#define SPI_DEV_CPHA 0x01     /* clock phase */ 
#define SPI_DEV_CPOL 0x02      /* clock polarity */ 

/*�����ĸ�����ΪSPI����ʱ��Ĭ�ϸߵ�ƽ���ߵ͵�ƽ�Լ���һ��ʱ���ز������ǵڶ���ʱ���ز�����ѡ�õĹ���ģʽ */ 
#define SPI_DEV_MODE_0  0       //(0 | 0) 
#define SPI_DEV_MODE_1  SPI_DEV_CPHA   //(0 | SPI_DEV_CPHA) 
#define SPI_DEV_MODE_2 (SPI_DEV_CPOL | 0) 
#define SPI_DEV_MODE_3 (SPI_DEV_CPOL | SPI_DEV_CPHA) 

/* SPI���豸����Э��궨�� ������Ҫ���һЩ��SPI���߷��ʵ��豸��SPIЭ��������޸� �Ǳ�׼ ��Ҫ���⴦�� Ʃ��tcg��׼�ж�SPIЭ���������չ �������ػ���*/ 
#define  SPI_DEV_PROTOCAL_NORMAL 0 
#define  SPI_DEV_PROTOCAL_TPM 1 
#define  SPI_DEV_PROTOCAL_ENCRYPTION 2 

#define SPI_OP_MAX_LEN 0x20000 



extern short  AD_RESULT[7][MaxSampPointNum];  // 6��128��AD����ֵ
int Init7022E(void);
int CalVoltCurrBaseRMS(void);
int ReadACInstDataFun7022E(WORD DIMark);
void ReadHarmInstWaveFun7022E(void);
void Real_U(int i ,  long* data);
void Real_I(int i ,  long* data);
void Real_PQ(int num,long* data);
void Real_ANGLE(long value);
void ReadFreq(long* data);
int  ReadTemp(void);
void ReadCos(long* data);
void JudgeIZone(short I, short In, int phase);
int ITempGainReg(void);
int TempAllGainReg(void);
void Cal_U_gain(BYTE id ,int value,int TheoreticVolt, VYcGain* pycgain);
void Cal_I_gain(BYTE id ,int value,int TheoreticCurr, VYcGain* pycgain);
void Cal_PQ_gain(BYTE id, int gain1, int gain2,VYcGain* pycgain);
int  Cal_angle_gain(BYTE id, int value, int pTheoretic);
int Cal_all_gain(int value, int pTheoretic, int T);
int att_gainread(void);
int att_gainclear(void);
int InitHREnv7022E(void);
void mt_fft7022E(void);
void ReadRms(int i ,  int wave_num , long* data);
void ReadAngle(int i , long* data);
void Att7022e_Eqp(void);
void Att7022eDd_Init(void);
void Att7022eDd_Clear(void);
double CalPosSequence( double Ampa,double Pha,double Ampb,double Phb,double Ampc,double Phc );
double CalNegSequence( double Ampa,double Pha,double Ampb,double Phb,double Ampc,double Phc );
double CalZeroSequence( double Ampa,double Pha,double Ampb,double Phb,double Ampc,double Phc );
int Power_ATT7022E(STATUS iType);
int ATT7022GainClear(void);
#endif

