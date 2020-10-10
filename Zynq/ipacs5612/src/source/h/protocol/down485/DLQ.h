#ifndef DLQ_H
#define DLQ_H

#include "pmaster.h"
#include "DataBase_Mqtt.h"
#include "mqttdb.h"
#define MAXDEVICENUM  12		//下面链接设备最大数量
#define  MAX_AUTO_RECOVER		20		//20个需要自动复归
#define CONST_VAL		21
#define REQ_IDEAL			0
#define REQ_YC				1
#define REQ_YX   			2
#define REQ_SOE 			3
#define REQ_YK              6
#define REQ_DD				5
#define REQ_YT				7

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
#define YCDI_NUM	5





/*============DORECEIVE 控制码============*/
//主站请求读数据
#define CMD_READ_DATA		0x11
#define CMD_READ_CONTINUE	0X12
#define CMD_READ_ADDR		0X13
#define CMD_WRITE_DATA		0x14
#define CMD_CLEAR		    0x1B
#define CMD_CTRL			0X1C

/*============ProcData 标志=============*/
#define VALUE_NOW       0x02000000
#define VALUE_EVENT     0x03000000
#define VALUE_PARA      0x04000000
#define VALUE_STATWORD	0x04000500

/*============Event 累计标志=============*/
#define DI_EVENT_COUNT       0x03810200
/*============Event 开关=============*/
#define SWS_ON				0x1
#define SWS_OFF				0x0

/**********************参数***********************/
//基本信息块
#define DI_DLQINFO					    0x04000400
//硬件版本号
#define DI_DEVTYPE                      0x04000410
//额定剩余电流动作值参数组
#define DI_DEV_SI						0x04000411
//额定极限不驱动时间参数组
#define DI_DEV_TIME						0x04000412

//运行状态字参数块
#define DI_STAT_WORD					0x04000500
//运行状态字1
#define DI_STAT_ONE					    0x04000501
//运行状态字2
#define DI_STAT_TWO					    0x04000502

//控制字参数块
#define DI_CONTROL_WORD 			    0x04001000
//控制字1
#define DI_CONTROL_ONE 			        0x04001001
//控制字2
#define DI_CONTROL_TWO 			        0x04001002
//控制字3
#define DI_CONTROL_THR 			        0x04001003
//控制字4
#define DI_CONTROL_FOR 			        0x04001004
#define DI_CONTROL_FIVE 			    0x04001005
#define DI_CONTROL_ALL 			    	0x040010FF

//自检参数块
#define DI_SELFCHECK 		            0x04001200
//定时自检整定时间
#define DI_SELFCHECK_SETTIME		    0x04001201

//电压整定参数块
#define DI_SET_U                        0x04001300
//过电压整定值
#define DI_SET_U_OVER                   0x04001301
//欠电压整定值
#define DI_SET_U_LEAK                   0x04001302
//断相电压整定值
#define DI_SET_U_BREAK                  0x04001303
#define DI_SET_U_ALL                  	0x040013FF

//电流整定参数块
#define DI_SET_I                        0x04001400
//额定电流整定值过载保护动作电流Ir1
#define DI_SET_I_IR1                    0x04001401
//过载保护动作时间t1
#define DI_SET_I_T1                     0x04001403
//短路短延时动作电流Ir2 (*Ir1)
#define DI_SET_I_IR2                    0x04001404
//短路短延时动作时间t2
#define DI_SET_I_T2                     0x04001405
//短路瞬时电流Ir3 (*Ir1)
#define DI_SET_I_IR3                    0x04001406
#define DI_SET_I_ALL                    0x040014FF

#define    EQPFLAG_YK    12

#pragma pack(1)
typedef struct
{
    char   cYxName[20];     //遥信名称


}YCYXNAME;





typedef struct
{
    char   cYxName[20];     //遥信名称
    BYTE   bState2;       //状态字2
    DWORD  dCountDI;       //遥信累计量
    DWORD  dEventDI;       //遥信事件DI

}YXMAP;

typedef struct
{
    char   cEventName[40]; //遥控名称
    BYTE   bFlag;         //读到的参数的点号
    BYTE   bNo;        	  //104 点号
  	BYTE   bCover; 
} EVENTMAP;

typedef struct
{
    char   cYxName[10];     //写遥测名称
    BYTE   bLen;           //遥测字节长度
    BYTE   bNo;            //遥测序号
    float  bUnit;          //遥测转换单位
}WYCMAP;

typedef struct
{
    char   ParaName[40]; 
    BYTE   bNo;        	  
  	DWORD  DI;
} PARAMAP;

typedef struct
{
    struct VCalClock  VClock;
    BYTE No;	//事件序号
    BYTE Dev_no;//装置号
    DWORD tick;
}AUTODELAY_RES;//自动延时恢复

typedef struct
{
    BYTE head;           //启动字符
    BYTE dev_addr[6];    //装置地址
    BYTE head2;          //启动字符
    BYTE control;        //控制域
    BYTE data_len;       //数据长度

    BYTE datebuf[256-10];
}DlqFrame;

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

typedef struct
{
	WORD wIaft[6]; 
	WORD wIactdelay[6];
	BYTE ctr1;
	BYTE ctr2;
	BYTE ctr3;
	BYTE ctr4;
	BYTE ctr5;
	BYTE Tzj[3];
	WORD Ugy;
	WORD Uqy;
	WORD Udx;
	DWORD Igz_Ir1;
	BYTE T1gz;
	BYTE Idl_Ir2;
	WORD Tdl;
	BYTE Idl_Ir3;
	
}SwitchPara;


#pragma pack(0)

class CDLQ : public CPPrimary
{
    public:
        DlqFrame * m_pReceive; 	  	        
        DlqFrame * m_pSend;  		        
        AUTODELAY_RES	*Auto_Recover;	    
        AUTODELAY_RES	*old_Recover;	    
        AUTODELAY_RES	*choese_Recover;	
		WORD m_ParaNum;
		WORD m_ParaIndex;
		WORD m_ParaSynOver;        
		DWORD m_DelayTime;
        BOOL m_first;
        WORD m_oldeqpno;
        WORD m_SYDLDZZ[MAXDEVICENUM][6];// 剩余电流动作组
        WORD m_SYDLDZSJ[MAXDEVICENUM][6];//剩余电流动作时间
        BYTE m_CONTROL4[MAXDEVICENUM];
		SwitchPara *m_Para;
		
    public:
        CDLQ();
        BOOL Init(WORD wTaskID);

        virtual void  DoTimeOut(void);
        virtual BOOL  DoReceive();
        virtual BOOL  ReqUrgency(void);
        virtual BOOL  ReqCyc(void);
        virtual BOOL  ReqNormal(void);
        virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
        void SendFrameHead(BYTE FunCode, BYTE ShiftAddress);
        void AddDataThree(BYTE *Buf,BYTE Len);//+33
        void SubDataThree(BYTE *Buf,BYTE Len);//-33
        BYTE ChkSum(BYTE *Buf, WORD Len);
        void SendFrameTail();
        void WriteData(void);
        void SendWriteCmd(BYTE FunCode,DWORD RegAddr, BYTE ShiftAddress, WORD Value);
        void SendSetTime(void);
        void SendWriteMReg(DWORD RegAddrStart, BYTE RegNum, BYTE ShiftAddress, BYTE *Value);
        void YkResult(BOOL Result);
        void SendReadCmd(BYTE FunCode,DWORD RegAddr, BYTE ShiftAddress, WORD Length);
        void ProcData(BYTE *Buf,int Len);
        void RecDataACK(BYTE *Buf, int Len, BYTE Control);
        void DoCurData(BYTE *Buf, BYTE Len, DWORD DI);
        void DoEventData(BYTE *Buf, BYTE Len, DWORD DI);
        void DoStatWordData(BYTE *Buf, BYTE Len, DWORD DI);
        void DoParaData(BYTE *Buf, BYTE Len, DWORD DI);
        void WriteCurData(BYTE *Buf,BYTE Len,BYTE Index);
        void WriteCurSIData(BYTE *Buf, BYTE Len);

        void WriteEventData(BYTE *Buf, BYTE Len, DWORD DI);
        void DoEventDlqCheck(BYTE * Buf,BYTE Len);
        void DoEventLossPower(BYTE * Buf, BYTE Len);
        void DoEventProtect(BYTE * Buf,BYTE Len);
        void DoEventThrowBack(BYTE * Buf, BYTE Len);
        void DoEventGateChange(BYTE * Buf,BYTE Len);
        void DoEventAlarm(BYTE * Buf,BYTE Len);

        void DoParaDlqInfo(BYTE *Buf, BYTE Len, DWORD DI);
        void DoParaControlWord(BYTE *Buf,BYTE Len, DWORD DI);
        void DoParaSelfCheck(BYTE *Buf,BYTE Len,DWORD DI);
        void DoParaSetU(BYTE *Buf,BYTE Len,DWORD DI);
        void DoParaSetI(BYTE *Buf, BYTE Len, DWORD DI);

        void DoStatWordOne(BYTE * Buf);
        void DoYkData(BYTE * Buf,BYTE Len,BYTE re);
        void  DoHardwareVersion(BYTE *Buf, BYTE Len, WORD DI);

    
       void WriteMqttData(int EqpId,WORD wNo,char *DataName,float value,int Type);
       void WriteChangeMqttData(int EqpId,WORD wNo,char *DataName, VSysClock* pTime,int value,int Type);
        void WriteAutoRecov(WORD dev_no,WORD no,struct VSysClock *time);
        AUTODELAY_RES *ChoeseStation();
        void  WriteEvent(WORD yxNo,WORD Yx_va,BYTE *buf,BYTE len,BYTE flag);
        BYTE  TimeTrans(struct VSysClock *tt,BYTE *a);
        void  WriteParaSoe(int oldpara, int newpara, struct VDBSOE *soe1, struct VDBSOE *soe2,VSysClock*ptime);
        void ClearAllData();

        BYTE  Hex2Bcd(BYTE HexNum);
        WORD  Hex2Bcd2(WORD HexNum);
        DWORD Hex2Bcd4(DWORD HexNum);
        void  BcdX2BinY(BYTE X, BYTE Y, BYTE *DesBin, BYTE *SrcBcd, WORD Multi);
        YXMAP *FindIndexofYxMap(WORD StatWord);
        EVENTMAP *FindAlarmEvent(BYTE bFlag);
        EVENTMAP * FindGateChangEvent(BYTE bFlag);
        EVENTMAP * FindThrowBackEvent(BYTE bFlag);
        EVENTMAP * FindProtectEvent(BYTE bFlag);
		BOOL DoSend(void);
		void SendReadPara(DWORD DI);
		void SendWritePara(DWORD DI,BYTE *bValue,BYTE len);



     // WORD FindIndexofEventMap(BYTE bFlag);


  private:



        BYTE     m_CurIndex;
        BYTE     m_ConIndex;

        YXMAP * YxMapIndex;
        BYTE m_Dlq_slef[2];


        BYTE     m_EventIndex;
        BYTE     m_EventCount;
        BYTE     m_StatWord2;


};







#endif // DLQ_H

