

#ifndef MDCP_H
#define MDCP_H

#include "../public/pslave.h"	
#include "prset.h"

#define MAX_FRAME_LEN		250
#define START_CODE			0x68
#define END_CODE			0x16

#define FUN_01_RunStateSet		0x01
#define FUN_02_RunStateGet		0x02
#define FUN_03_SetValSet		0x03
#define FUN_04_SetValGet		0x04
#define FUN_05_SimStart			0x05
#define FUN_06_SimDataSet		0x06
#define FUN_07_SimYK			0x07
#define FUN_0A_SimEnd			0x0A
#define FUN_0B_RealData			0x0B
#define FUN_0C_RealYK			0x0C
#define FUN_0D_RealSOE			0x0D
#define FUN_A0_HeartBeat		0xA0
#define FUN_A1_ACK				0xA1
#define FUN_A2_NACK			0xA2
#define FUN_F0_FileCmd		0xF0
#define FUN_F1_FileData 	0xF1
#define FUN_F2_FAMS 	0xF2
#define FUN_F3_FAMS 	0xF3
#define FUN_SET_IP          0xF4

#define	SWITCHER_NUM			16
#define	SIM_START				1
#define	SIM_END				2

#define MAX_BUF            2048

typedef struct
{
	BYTE StartCode;
	BYTE Length;
	BYTE TypeId;
	BYTE Data[MAX_FRAME_LEN];
}VFrameHead;

#pragma pack(1)
//1.断路器保护参数
typedef struct
{
	BYTE	OCIYb;
	float		OCIDz;
	float		OCITime;
	BYTE      	byIPrYb;
	BYTE	OpenMode;
	BYTE	ReportMode;
	BYTE	RCYb;
	BYTE	RCCount;
	float		RC1Time;
	float		RC2Time;
	float       RC3Time;
	float		RCLockTime;
	BYTE	OCIIYb;
	float		OCIIDz;
	float		OCIITime;
	BYTE      byIIPrYb;
	BYTE      byGLFx;
	BYTE      byGLYx;
	float       AngleLow;
	float      AngleUpp;
	
	BYTE     byXDLJD;
	BYTE     byXDLJD_Trip; 

	float    XDL_U0;
	float    XDL_U0_diff;	
	float    XDL_I0_diff;
	float    XDL_I_diff;
	float    XDL_Time;
	float    XDL_Coef;
		
}VBreakerProtectPara;

//2.电压型保护参数
typedef struct
{
	BYTE	SVOpenYb;
	BYTE	LBreakerYb;
	BYTE	SLFeederNo;
	float		XTime;
	float		YTime;
	float		ZTime;
	BYTE	XLockYb;
	BYTE	YLockYb;	
	BYTE	IVLockYb;
	BYTE	LoseVolYb;
	float       CYTime;
	float		CYDz;
	BYTE       U0Yb;
	float		U0Dz;
	BYTE       FZBS;
	float       FZBSTime;

	BYTE     byXDLJD;
	BYTE     byXDLJD_Trip; 
	float    XDL_U0;
	float    XDL_U0_diff;	
	float    XDL_I0_diff;
	float    XDL_I_diff;
	float    XDL_Time;
	float    XDL_Coef;
}VVProtectPara;

//3.电流计数型保护参数
typedef struct
{
	BYTE	CCountYb;
	BYTE	SLFeederNo;
	BYTE	CCount;
	float		CCountRstTime;
	float		CCountAccTime;
	BYTE	OCIYb;
	float		OCIDz;
	float		OCITime;
	BYTE	OpenMode;
	BYTE	ReportMode;
	BYTE	RCYb;
	BYTE	RCCount;
	float		RC1Time;
	float		RC2Time;
	float       RC3Time;
	float		RCLockTime;
	BYTE      byIPrYb;
}VCCountProtectPara;

//4.自适应型保护参数
typedef struct
{
	BYTE	SVOpenYb;
	BYTE	LBreakerYb;
	BYTE	SLFeederNo;
	float		XTime;
	float		YTime;
	float       ZTime;
	float       STime;
	float		CTime;
	BYTE	XLockYb;
	BYTE	YLockYb;	
	BYTE	IVLockYb;
	BYTE      LoseVolYb;
	BYTE      OCIYb;
	float       OCIDz;
	float       OCITime;
	BYTE      byIPrYb;
	BYTE      byxjdl;
	float       CYTime;
	float		CYDz;
	BYTE       U0Yb;
	float		U0Dz;
	BYTE       FZBS;
	float       FZBSTime;

	BYTE     byXDLJD;
	BYTE     byXDLJD_Trip; 
	float    XDL_U0;
	float    XDL_U0_diff;	
	float    XDL_I0_diff;
	float    XDL_I_diff;
	float    XDL_Time;
	float    XDL_Coef;
	BYTE bYbZsyjd;
	BYTE bYbDu;
	BYTE bYbU0Trip;
	float fTU0;
}ZSYProtectPara;

//5.电压电流型
typedef struct
{
	BYTE	SVOpenYb;
	BYTE	LBreakerYb;
	BYTE	SLFeederNo;
	float		XTime;
	float		YTime;
	float		ZTime;
	BYTE	XLockYb;
	BYTE	YLockYb;	
	BYTE	IVLockYb;
	BYTE	LoseVolYb;
	BYTE      DYdlYb;
	BYTE      OCIYb;
	float       OCIDz;
	float       OCITime;
	BYTE      byIPrYb;
	float       CYTime;
	float		CYDz;
	BYTE       U0Yb;
	float		U0Dz;
	BYTE       FZBS;
	float       FZBSTime;

	BYTE     byXDLJD;
	BYTE     byXDLJD_Trip; 
	float    XDL_U0;
	float    XDL_U0_diff;	
	float    XDL_I0_diff;
	float    XDL_I_diff;
	float    XDL_Time;
	float    XDL_Coef;
	BYTE     bYbDu;
	BYTE bYbU0Trip;
	float fTU0;
}DYDLProtectPara;

//6. 变电站出口(零流)
typedef struct
{
	BYTE	OCIYb;
	float		OCIDz;
	float		OCITime;
	BYTE      	byIPrYb;
	BYTE	OpenMode;
	BYTE	ReportMode;
	BYTE	RCYb;
	BYTE	RCCount;
	float		RC1Time;
	float		RC2Time;
	float       RC3Time;
	float		RCLockTime;
	BYTE	OCIIYb;
	float		OCIIDz;
	float		OCIITime;
	BYTE      byIIPrYb;
	BYTE      I0Yb;
	float		I0IDz;
	float		I0ITime;
	BYTE	byI0PrYb;
	BYTE      byGLFx;
	BYTE      byGLYx;
	float      AngleLow;
	float      AngleUpp;

	BYTE     byXDLJD;
	BYTE     byXDLJD_Trip; 
	float    XDL_U0;
	float    XDL_U0_diff;	
	float    XDL_I0_diff;
	float    XDL_I_diff;
	float    XDL_Time;
	float    XDL_Coef;
	
}VI0ProtectPara;
#pragma pack()
typedef struct
{
	DWORD	SwID;
	DWORD	DataTime;
	WORD	YcVal[8];
	BYTE	YxVal[8];
	BYTE	bComm;
}VSimData;

typedef struct
{
	DWORD	SwID;
	BYTE	RunMode;
	BYTE	FAMode;
	BYTE	ProtectMode;
}VSimRunStatus;

typedef struct
{
	BYTE	SimStatus;
	BYTE	Modal;	//light show
	BYTE	SwNum;
	VSimRunStatus	*pSimRunStatus;
}VSimInfo;

typedef struct
{
	DWORD 	SwID;
	WORD	ProParaLength;
	BYTE	SwType;
	BYTE	FaMode;
	BYTE	ProtectMode;
	void		*pProtectPara;
}VProtectPara;

typedef struct
{
	BYTE	YkFlag;
	BYTE	YkID;
	BYTE	YkVal;
}VFaYkInfo;


typedef struct
{
	WORD wFilelen;
	WORD wCuroffset;
	BOOL bRecBegin;	
	BYTE Num;
}VFaCfgFile;

#ifdef TEST_FRAME



void SetProtectPara(DWORD SwID, void* ProtectPara);
void GetProtectPara(DWORD SwID, void *ProtectPara);

#endif

void SimInit(void);
void SimStart(void);
void SimEnd(void);
void SetRunMode(DWORD SwID, BYTE RunMode);
BYTE GetRunMode(DWORD SwID);
void SetRunStatus(DWORD SwID, VSimInfo* SimInfo);
void GetRunStatus(DWORD SwID, VSimInfo *SimInfo);
void WriteSimData(DWORD SwID, void *SimData);

#ifdef __cplusplus
extern "C" {
#endif
BOOL  ReadSimData(DWORD SwID, void *SimData);
int FaYkOperate(int YkID, int YkVal);
int FaYkSelect(int YkID, int YkVal);
#ifdef __cplusplus
}
#endif

class CMDCP: public CPSecondary
{
	public:
		VFrameHead *m_pRecFrame;
		VFrameHead *m_pSendFrame;
		BYTE		m_byCurrentProProMode;
		BYTE        m_setbuf[MAX_BUF];
		VFaCfgFile  m_cfgfile;	
		
	public:
		CMDCP();
		BOOL Init(WORD wTaskID);
		void InitStateSet(void);
		virtual BOOL DoReceive();
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
		
		void RecFrame(void);

		void Rec_01_StateSet(void);
		void Rec_02_StateGet(void);
		void Rec_03_SetValSet(void);
		void Rec_04_SetValGet(void);
		void Rec_05_SimStart(void);
		void Rec_06_SimDataSet(void);
		void Rec_07_SimYK(void);
		void Rec_0A_SimEnd(void);
		void Rec_0C_RealYK(void);
		int  Rec_F1_FileData(void);
		int  Rec_F0_FileCmd(void);

		void Send_07_SimYk(void);
		void Send_A0_HeartBeat(void);
		void Send_A1_ACK(BYTE recfuncode);
		void Send_A2_NACK(BYTE recfuncode);
		void Rec_F2_FAMS(void);
		void Rec_F3_FAMS(void);
		void Set_Ip(void);

		BOOL SendFrameHead(BYTE FunCode);
		BOOL SendFrameTail(void);

		void DoTimeOut100ms(void);
		void DoCommSendIdle(void);
		void DoTimeOut(void);

		BYTE ChkSum(BYTE *buf, WORD len);
		

		#ifdef TEST_FRAME

		void Rec_16_GetSimData(void);
		#endif
		
};

#endif
