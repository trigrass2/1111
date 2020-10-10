/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef GB101PUB_H
#define GB101PUB_H
#include "mytypes.h"
#define GB101_V2002
#ifdef GB101_V2002
#define ADDR_YX_LO		0x0001
#define ADDR_YX_HI		0x4000

#define ADDR_YC_LO		0x4001
#define ADDR_YC_HI		0x5000
#define ADDR_YCPARA_LO		0x5001
#define ADDR_YCPARA_HI		0x6000

#define ADDR_YK_LO		0x6001
#define ADDR_YK_HI		0x6200

#define ADDR_YT_LO		0x6201
#define ADDR_YT_HI		0x6400
#define ADDR_DD_LO		0x6401
#define ADDR_DD_HI		0x6600

#define	ADDR_RCD_YC		0x680A  //2016 新增录波地址

#else
#define ADDR_YX_LO		0x001
#define ADDR_YX_HI		0x400

#define ADDR_YC_LO		0x701
#define ADDR_YC_HI		0x900

#define ADDR_YK_LO		0xb01
#define ADDR_YK_HI		0xb80

#define ADDR_YT_LO		0xb81
#define ADDR_YT_HI		0xc00
#define ADDR_DD_LO		0xc01
#define ADDR_DD_HI		0xc80
#endif


#define ADDR_BH_LO		0x401
#define ADDR_BH_HI		0x700



#define ADDR_PARA_LO	0x901
#define ADDR_PARA_HI	0xb00

#define ADDR_STEP_LO	0xc81
#define ADDR_STEP_HI	0xca0

#define ADDR_BIN_LO		0xca1
#define ADDR_BIN_HI		0xcb0

#define ADDR_BCD_LO		0xcb1
#define ADDR_BCD_HI		0xcc0

#define ADDR_RTU_STATE	0xcc1

#define ADDR_FILE_LO	0xcc2
#define ADDR_FILE_HI	0xe00


#define	M_SP_NA		1 	//不带时标的单点信息
#define	M_SP_TA		2	//带短时标的单点信息
#define	M_DP_NA		3 	//不带时标的双点信息
#define	M_DP_TA		4	  //带短时标的双点信息
#define	M_ST_NA		5 	//步位置信息
#define	M_BO_NA		7 	//子站远动终端状态
#define	M_ME_NA		9 	//测量值 归一化值
#define M_ME_TA   10  //带时标归一化值
#define M_ME_NB   11  //测量值 标度化值
#define M_ME_TB   12  //带时标标度化值
#define M_ME_NC   13  //测量值 短浮点数
#define M_ME_TC   14  //带时标短浮点数
#define	M_IT_NA		15 	//电能脉冲记数量
#define M_IT_NB_1   206   //电能量累积量，短浮点数
#define M_IT_NC_1   207  //带时标的累积量，短浮点数
#define	M_PS_NA		20 	//具有状态变位检出的成组单点信息
#define	M_ME_ND		21 	//不带品质描述的测量值

#define M_SP_TB   30  //带长时标的单点信息
#define M_DP_TB   31  //带长时标的双点信息
#define M_ST_TB   32  //带长时标的步位置信息
#define M_BO_TB   33  //带长时标的32位位串
#define M_ME_TD   34  //带长时标的测量值，归一化值
#define M_ME_TE   35  //带长时标的测量值，标度化值
#define M_ME_TF   36  //带长时标的测量值，短浮点数
#define	M_EI_NA		70 	//初始化结束

#define M_FT_NA_1	42		//故障事件

#define	C_SC_NA		45 	//单点遥控命令
#define	C_DC_NA		46 	//双点遥控命令
#define	C_RC_NA		47 	//升降命令
#define	C_SE_NA		48 	//设定命令
#define	C_SE_NB_1		49 	//设定命令
#define	C_SE_NC_1		50 	//设定命令
#define	C_SE_TA_1		61 	//设点归一化
#define	C_SE_TB_1		62 	//设点标度化
#define	C_SE_TC_1		63 	//设点短浮点


#define	C_IC_NA		100 	//召唤命令
#define	C_CI_NA		101 	//电能脉冲召唤命令
#define	C_RD_NA		102 	//读数据命令
#define	C_CS_NA		103 	//时钟同步命令
#define C_TS_NA     104   	//测试命令
#define	C_RP_NA		105 	//复位进程命令
#define C_TS_TA     107   	//带时标的测试命令
#define P_ME_NA     110   	//参数
#define P_AC_NA     113   	//参数激活

/*2016 新增 录波文件传输类型标志*/
#define F_FR_NA_1_2	120		//文件准备就绪
#define F_SR_NA_1_2	121		//节准备就绪
#define F_SC_NA_1	122		//召唤目录.选择文件.召唤文件.召唤节
#define F_LS_NA_1	123		//最后的节，最后的段
#define F_AF_NA_1	124		//认可文件.认可节
#define F_SG_NA_1	125		//段
#define F_DR_NA_1	126		//目录


/*101 - 2016 新增类型*/
#define C_SR_NA_1	200     //切换定值区
#define C_RR_NA_1	201	    //读定值区号
#define C_RS_NA_1	202     //读参数和定值
#define C_WS_NA_1	203     //写参数和定值
#define F_FR_NA_1	210	    //文件传输
#define F_SR_NA_1	211     //软件升级
/*******END*********/


#define C_TSDT_NA		129		//透明数据转发
#define C_FILE_OP	130		//文件操作
#define M_FA_NA		142		//FA故障信息
#define C_FA_SIM	143		//FA 模拟
//传送原因：
#define	COT_PERCYC	  	1 	//周期/循环
#define	COT_BACK		2 	//背景扫描
#define	COT_SPONT		3 	//突发
#define	COT_INIT	  	4 	//初始化
#define	COT_REQ		    5 	//请求或被请求
#define	COT_ACT		    6 	//激活
#define	COT_ACTCON		7 	//激活确认
#define	COT_DEACT		8 	//停止激活
#define	COT_DEACTCON	9 	//停止激活确认
#define	COT_ACTTERM		10 	//激活结束
#define	COT_RETREM		11 	//远程命令引起的返送信息
#define	COT_RETLOC		12 	//当地命令引起的返送信息
#define	COT_FILE		13 	//文件传送

#define	COT_INTROGEN	20 	//响应总召唤
#define	COT_INRO1		21 	//响应第1组召唤
#define	COT_INRO2		22 	//响应第2组召唤
#define	COT_INRO3		23 	//响应第3组召唤
#define	COT_INRO4		24 	//响应第4组召唤
#define	COT_INRO5		25 	//响应第5组召唤
#define	COT_INRO6		26 	//响应第6组召唤
#define	COT_INRO7		27 	//响应第7组召唤
#define	COT_INRO8		28 	//响应第8组召唤
#define	COT_INRO9		29 	//响应第9组召唤
#define	COT_INRO10		30  //响应第10组召唤
#define	COT_INRO11		31 	//响应第11组召唤
#define	COT_INRO12		32 	//响应第12组召唤
#define	COT_INRO13		33 	//响应第13组召唤
#define	COT_INRO14		34 	//响应第14组召唤
#define	COT_INRO15		35 	//响应第15组召唤
#define	COT_INRO16		36  //响应第16组召唤
#define	COT_REQCOGCN	37  //响应计数量总召唤
#define	COT_REQCO1		38 	//响应第1组计数量召唤
#define	COT_REQCO2		39 	//响应第2组计数量召唤
#define	COT_REQCO3		40 	//响应第3组计数量召唤
#define	COT_REQCO4		41 	//响应第4组计数量召唤
#define	INIT_OVER		70 	//初始化结束

#define	COT_PN_BIT		0x40

#define		S_INITOK		0
#define		S_REQLINK		1
#define		S_RESETLINK		2
#define		S_LINKOK		3
#define		S_REQRES		4
#define		S_SENDCON		5
#define		S_LINKBUSY		6
#define		S_RECREQ        7
#define		S_RECRESET      8
#define		S_NULL 			254


#define 	PRM_MASTER		1
#define		PRM_SLAVE		0

#define		MFC_RESETLINK		0
#define		MFC_TRANSDATA		3
#define		MFC_REQLINK			9
#define		MFC_REQCLASS1		10
#define		MFC_REQCLASS2		11

#define		SFC_CONFIRM			0
#define		SFC_LINKBUSY		1
#define		SFC_DATACON			8
#define		SFC_LINKOK			11
#define		SFC_LINKNOTWORK		14
#define		SFC_LINKNOTFINISH	15
#define		SFC_RECREQ		    0x49
#define		SFC_RECRESET	    0x40

//帧控制域
#define 	BITS_DIR	0x80	//传输方向位
#define 	BITS_PRM	0x40	//启动报文位
#define 	BITS_FCB	0x20	//帧计数位
#define 	BITS_AVI	0x10	//帧计数有效位
#define		BITS_ACD	0x20
#define		BITS_DFC	0x10
#define 	BITS_CODE	0x4F	//功能码所占位
#define 	BITS_FCBV 	0x30	//功能码所占位

#define CONTROL_LEN			2
#define ASDUID_LEN			4

#define SCOS_LEN		3
#define SSOE_LONG_LEN	10
#define SSOE_SHORT_LEN	6
#define CHANGE_YC_LEN	4

#define MAX_FRAME_LEN  		255

typedef struct 
{
	BYTE Start; 
	BYTE Control; 
	BYTE Data[4];//BYTE Address; 
	//BYTE ChkSum; 
	//BYTE Stop; 
}VFrame10; 

typedef struct 
{
	BYTE Start1; 
	BYTE Length1; 
	BYTE Length2; 
	BYTE Start2; 
	BYTE Control; 
	BYTE Data[MAX_FRAME_LEN-5];//BYTE LinkAddress;
	//BYTE Style;  
	//BYTE Definitive; //结构限定词
	//BYTE Reason; 
	//BYTE CommonAddress;   
	//BYTE Data;  
}VFrame68; 

typedef union 
{
	VFrame10 Frame10;
	VFrame68 Frame68;
} VIec101Frame; 

struct VIec101Cfg
{
	BYTE byLinkAdr;
	BYTE byCommAdr;
	BYTE byInfoAdr;
	BYTE byCOT;
};

#pragma pack(1)
typedef struct{
	BYTE linkaddrlen;/*链路地址长度*/
	BYTE typeidlen;/*信息类型长度*/
	BYTE conaddrlen;/*应用层公共地址长度*/
	BYTE VSQlen;/*可变结构限定词长度*/
	BYTE COTlen;/*传送原因长度*/
	BYTE infoaddlen;/*信息地址长度*/
	BYTE mode;/*传输模式*/
	BYTE yxtype;/*1.不带时标；2.带时标；3带CP56Time2a时标*/
	BYTE yctype;/*9.测量值, 规一化值;10.带时标的测量值, 规一化值11.测量值, 标度化值12.测量值, 带时标的标度化值
	              13.测量值, 短浮点数14.测量值, 带时标的短浮点数21.测量值, 不带品质描述词的规一化值
	               34.带CP56Time2a时标的测量值, 规一化值35带CP56Time2a时标的测量值, 标度化值
	  36带CP56Time2a时标的测量值, 短浮点数 */
	BYTE ddtype;/*15.累计量16带时标的累计量37带CP56Time2a时标的累计量*/
	WORD baseyear;/*1900或2000*/
	WORD jiami;/*0加密*/
	WORD tmp[9];//tmp[6]用作级联时fdno
				//tmp[7]福建读0x8243过压定值，0-额定值倍数，1-幅值
}Vgb101para;/*101规约参数结构*/
#pragma pack ()

typedef struct {
	DWORD LinkAddr;	   
	DWORD TypeID;	   
	DWORD VSQ;
	DWORD COT;	
	DWORD Address;	
	DWORD Info;	   
	BYTE  Infooff;	   
	BYTE  COToff;
} VDWASDU;

//测量值参数(qpm )
typedef struct{
	WORD limitchange;       //门限值
	WORD pinghuashijian; //平滑系数
	WORD limitmax;          //传送测量上限
	WORD limitmmin;        //传送测量下限
}Vlimitpara;

typedef struct{

	WORD filename;
	BYTE filelen[3];
	BYTE fileSOF;
	BYTE filetime[7];

}VFileDir;


#endif
