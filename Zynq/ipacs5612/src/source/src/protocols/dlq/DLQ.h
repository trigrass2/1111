#ifndef DLQ_H
#define DLQ_H

#include "../public/pmaster.h"

#define MAXDEVICENUM  12		//下面链接设备最大数量

#define  MAX_AUTO_RECOVER		20		//20个需要自动复归

#define CONST_VAL		21

#define REQ_IDEAL			0
#define REQ_YC				1
#define REQ_YX   			2
#define REQ_SOE 			3
#define REQ_YK              6
#define REQ_DD				5

#define SWS_ON				0x81
#define SWS_OFF				0x01

#define SWITCH_STARTNUM_YX	0X81
#define SWITCH_STARTNUM_YC	0X4201
#define MAX_SWITCH_PARA_NUM 256

#define MAX_DLQ_FRAME_SIZE	255

#define BASE_CS_ADDR	0x8481
#define DEV_DZNUM		30

#define YK_F_DI			0x06010201
#define YK_H_DI	        0x06010101

#define HEAD_		0x68
#define CaS_NUM		21
#define YCDI_NUM	8		

#define SJ_CODE		0x11
#define CLEAR		0x1B
#define SET_CaS		0x14
#define SET_TIME	0x08

#define DI_YC_U     0x0201ff00
#define DI_YC_I 	0x0202ff00
#define DI_YC_SI	0x0290ff00

#define DI_EVNET_CNT	0x03810200//

#define DI_YX_ONE		0x038D0000
#define DI_YX_TWO		0x038E0000
#define DI_YX_THREE		0x038F0000
#define DI_YX_FOUR		0x03910000
#define DI_YX_FIVE		0x03920000
#define DI_YX_SIX		0x03930000

#define DI_CAS_1						0x04000411
#define DI_CAS_2 						0x04000412
#define DI_CAS_3 						0x04001004
#define DI_CAS_4						0x04001406
#define DI_CAS_5						0x04001003
#define DI_CAS_6						0x04001404//短路电流
#define DI_CAS_7						0x04001405//短路短延时
#define DI_CAS_8						0x04001401
#define DI_CAS_9						0x04001403
#define DI_CAS_10						0x04001301
#define DI_CAS_11						0x04001002
#define DI_CAS_12						0x04001302
#define DI_CAS_13						0x04001303
#define DI_CAS_14						0x04001201
#define DI_CAS_15						0x04001001
#define DI_CAS_16						0x11111111


#define DI_YX_DEVTYPE					0x04000410
#define DI_STAT_TWO						0x04000502
#define DI_STAT_ONE 					0x04000501

typedef struct SwitchEvent
{
	WORD YxNum;
	WORD YxNo[MAX_GZYX_NUM];
	WORD YxValue[MAX_GZYX_NUM];
	WORD YcNum;
	WORD YcNo[MAX_GZYC_NUM];
	DWORD YcValue[MAX_GZYC_NUM]; //先电压后电流
	struct VCalClock Time[MAX_GZYX_NUM];
}SwitchEvent;
#pragma pack(1)

typedef struct
{
	SwitchEvent evt[MAX_GZEVT_NUM];
	WORD wReadPtr;
	WORD wWritePtr;
}SWITCHEVTOPT;

struct SwitchPara{
	WORD parano;
	BYTE type;
	BYTE len;
	char valuech[PARACHAR_MAXLEN];
};	

typedef struct SwitchParaOpt
{
	struct SwitchPara para[MAX_SWITCH_PARA_NUM];
	WORD wReadPtr;
	WORD wWritePtr;
}SwitchParaOpt;

typedef enum
{
	YK_SELECT = 0,	//yk选择
	YK_ACTION ,
	YK_REVOCAT 
}YKStep;


//add by xhp
typedef struct 
{
	BYTE head;      //启动字符
	BYTE dev_addr[6];    //长度1
	BYTE head2;      //启动字符
	BYTE control;    //控制域
	BYTE data_len;		//数据长度
	
	BYTE datebuf[256-10];
}DlqFrame;

typedef struct 
{
	struct VCalClock  VClock;
	BYTE No;	//事件序号
	BYTE Dev_no;//装置号
	DWORD tick;
}AUTODELAY_RES;//自动延时恢复

typedef struct
{
      BYTE readflag;
      BYTE switchno;
      WORD switchparaoffset;
      WORD paraoffset;
}ParaGet;
#pragma pack()

const BYTE Broadcase_addr[6] = {0x99,0x99,0x99,0x99,0x99,0x99};
const BYTE PASSWORD[4]={0x00};
const BYTE OPERRATE[4]={0x00};
const BYTE Clear[4]={0xff};

const DWORD yc_Di[8]= {0x04000501,0x04000410,0x0201ff00,0x0202ff00,0x0290ff00,0x04000502,0x04000501,0x038102FF};//0x04000501

const DWORD yx_Di[6]={0x03920001,0x038F0001,0x03930001,0x03910001,0x038D0001,0x038E0001};

const DWORD dz_di[]={0x04001004,0x04001004,0x04001004,0x04001406,0x04001003,
					  0x04001404,0x04001405,0x04001003,0x04001401,0x04001403,
					  0x04001003,0x04001301,0x04001002,0x04001302,0x04001002,
					  0x04001303,0x04001002,0x04001002,0x04001201,0x04001001,
					  0x11111111};//最后一个不知道di
						
const DWORD dz_di1[]={0x04000411,0x04000412,0x04001004,0x04001406,0x04001003,
					  0x04001404,0x04001405,0x04001003,0x04001401,0x04001403,
					  0x04001003,0x04001301,0x04001002,0x04001302,0x04001002,
					  0x04001303,0x04001002,0x04001002,0x04001201,0x04001001,
					  0x11111111};//最后一个不知道di

const DWORD Clear_di[] = {0x03810205,0x03810201,0x03810204,0x03810202,0x03810203,0x03810206};

int ReadSwitchEvent(SwitchEvent *event);
int SetConst_value(WORD parano,char *pbuf);
int GetConst_value(WORD parano,char *pbuf);
void ClearConst_Value(void);
extern BOOL g_SetTimeFlag;

//#endif
class CDLQ : public CPPrimary
{
	public:
		DlqFrame * m_pReceive; 		//接收帧头指针
		DlqFrame * m_pSend;  		//发送帧头指针
		AUTODELAY_RES	*Auto_Recover;	//自动恢复
		AUTODELAY_RES	*old_Recover;	//自动恢复
		AUTODELAY_RES	*choese_Recover;	//自动恢复

		DWORD 	event_time ;
		BYTE 	m_byReqDataType;

		BYTE m_byNextReq;
		BYTE m_index;
		DWORD m_Delay_time;
		BYTE m_Station_t;
		BYTE m_Dlq_slef[2];
		BYTE m_yx[MAXDEVICENUM]; //多装置有问题
		DWORD m_Dii;
		DWORD m_EventTimes[6];
		BYTE m_DW_4;
		BOOL m_first;
              WORD m_oldeqpno;

		WORD m_SYDLDZZ[MAXDEVICENUM][6];// 剩余电流动作组		
		WORD m_SYDLDZSJ[MAXDEVICENUM][6];// 剩余电流动作时间
		BYTE m_CONTROL4[MAXDEVICENUM];
	public:
		CDLQ();
		BOOL Init(WORD wTaskID);
	
		virtual void DoTimeOut(void);
		virtual BOOL DoReceive();	
		virtual BOOL ReqUrgency(void);
		virtual BOOL ReqCyc(void);
		virtual BOOL ReqNormal(void);	
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
	
		void Time4bytetoCalClock(BYTE *bTime4,VCalClock *CalCloc);				
		void WriteData(void);		
		BYTE ChkSum(BYTE *buf, WORD len);

		void YkResult(BOOL Result);
		void SendWriteCmd(BYTE FunCode,DWORD RegAddr, BYTE ShiftAddress, WORD Value);
		void SendFrameHead(BYTE FunCode, BYTE ShiftAddress);
		void SendFrameTail();
		void SendSetTime(void);
		void SendWriteMReg(DWORD RegAddrStart, BYTE RegNum, BYTE ShiftAddress, BYTE *Value);
		void SendReadCmd(BYTE FunCode,DWORD RegAddr, BYTE ShiftAddress, WORD Length);
		void DoYxData(BYTE * buf,BYTE Len);
		void DoYcData(BYTE * buf,BYTE Len,BYTE *addr);
		void DoddData(BYTE * buf,BYTE Len);
		void Deal_yc(BYTE Lenn,BYTE *buf,BYTE X,BYTE Y);
		void BcdX_To_BinY(BYTE X, BYTE Y, BYTE *DesBin, BYTE *SrcBcd, WORD Multi);
		WORD WriteConst_value(BYTE addr,BYTE num,BYTE *buf);
		void DoYx(BYTE * buf,BYTE Len);
		void DoYx_alarm(BYTE * buf,BYTE Len);
		void DoYx_alarm1(BYTE * buf,BYTE Len);
		void DoYx_alarm2(BYTE * buf,BYTE Len);
		void DoYx_alarm3(BYTE * buf,BYTE Len);
		void DoYx_alarm4(BYTE * buf,BYTE Len);
		void DoYx_alarm5(BYTE * buf,BYTE Len);

		void Write_Auto_Recov(WORD dev_no,WORD no,struct VSysClock *time);
		AUTODELAY_RES *Choese_Station();
		BYTE SetKGVal(int eqpno, int addr, int flag, BYTE data);

		void Add_data_three(BYTE *buf,BYTE len);//+33
		void subtract_data_three(BYTE *buf,BYTE len);//-33
		void DoYkData(BYTE * buf,BYTE Len,BYTE re);

		void Deal_DD(DWORD di);
		void Value_DD(BYTE * buf,BYTE Len,BYTE flag);

		void Int_ToStr(DWORD nIp, BYTE* pVal);

		void Write_Event(WORD yxNo,WORD Yx_va,BYTE *buf,BYTE len,BYTE flag);
		void Set_Const_Val(DWORD di,WORD addr,SwitchPara *apara);
		void Set_value_trans(DWORD di,BYTE *addr,SwitchPara*para);
		BYTE HexToBcd(BYTE hex);
		WORD Hex2Bcd2(WORD HexNum);
		DWORD Hex2Bcd4(DWORD HexNum);
		BYTE time_trans(struct VSysClock *tt,BYTE *a);
		DWORD Get_DI_yx(BYTE * Index);
		void WriteParaSOE(int oldpara, int newpara, struct VDBSOE *soe1, struct VDBSOE *soe2);
		void Get_Dw(BYTE dw,BYTE *z_dw,BYTE *t_dw);
        void Deal_bb(BYTE Lenn,BYTE *buf);
		void Clear_All_Data();

		void Dz_makeFram(SwitchPara *para,DWORD Di);
		void ProcData(BYTE *tmpbuf,int len);
		
};

#endif

