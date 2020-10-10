// disp.h: interface for the Cdisp class.
//
//////////////////////////////////////////////////////////////////////
/*------------------------------------------------------------------------
 $Rev: 4 $
------------------------------------------------------------------------*/

#ifndef DISP_H
#define DISP_H
#include "sys.h" 
#include "prSet.h"

#include "pcol.h"

#define EXTMMI_TYPE_CONNECT     0x01
#define EXTMMI_TYPE_PRSET       0x02
#define EXTMMI_TYPE_IP          0x03
#define EXTMMI_TYPE_YX          0x04
#define EXTMMI_TYPE_LIGHT       0x04
#define EXTMMI_TYPE_LIGHTMD     0x05
#define EXTMMI_TYPE_YKMD        0x06
#define EXTMMI_TYPE_PROG_WRITE  0x07
#define EXTMMI_TYPE_HBEAT       0xA0

#define EXTMMI_CTRL_MASTER     0x01
#define EXTMMI_CTRL_REQPOLL    0x02
#define EXTMMI_CTRL_RANGE      0x04
#define EXTMMI_CTRL_SECT_1     0x08
#define EXTMMI_CTRL_SECT_E     0x10
#define EXTMMI_CTRL_Addr       0xC0    //by lvyi,判断分合闸面板地址


#define EXTMMI_FUN_READ        0x00
#define EXTMMI_FUN_WRITE       0x01
#define EXTMMI_FUN_CTRL        0x02
#define EXTMMI_FUN_CONF        0x03
#define EXTMMI_FUN_DATA        0x04
#define EXTMMI_FUN_END         0x05

#define EXTMMI_ACK_OK          0
#define EXTMMI_ACK_ERR         1

#define EXTMMI_CODE1	       0xEB
#define EXTMMI_CODE2	       0x90
#define EXTMMI_CRCLEN          2


#define DIS_TYPE_CLR   		0x61
#define DIS_TYPE_RST     	0x62
#define DIS_TYPE_GB    	    0x63
#define DIS_TYPE_DATA     	0x64
#define DIS_TYPE_BLKLIGHT   0x65
#define DIS_TYPE_KEY       	0x66

#define EXTMMI_CTRL_MASTER     0x01

#define EXTDISP_MODEL_DISP   0
#define EXTDISP_MODEL_NODISP 1
#define EXTDISP_MODEL_LAMP   2

#define  LOBYTE(w)     ((BYTE)(w))
#define  HIBYTE(w)     ((BYTE)((WORD)(w) >> 8))

#pragma pack(1)
typedef struct{
	BYTE byCode1;	
	BYTE byCode2;
	BYTE byLen;   
	BYTE byCnt;
	BYTE byFun;
	BYTE byType;
	BYTE byCtrl; 
}VExtMMIFrmHead;	

#pragma pack()

#define EXTMMI_MAXFRM_LEN        256
#define EXTMMI_MAXFRMDATA_LEN   (EXTMMI_MAXFRM_LEN-sizeof(VExtMMIFrmHead)-1)

#define EXTMMI_HBEAT_INTVAL      4 //5s
#define EXTMMI_TIMEOUT           30//30s

#define EXTMMI_BUF_MAX           2048

/************/

#define closeTIME 240
#define TURNTIME 120
#define TURNnextTIME 9
#define KEYLONGDOWN 1
enum {
	UPKEY=1,
	DOWNKEY=2,
	LEFTKEY=4,
	RIGHTKEY=8,
	QUITKEY=0x80,
	ENTERKEY=0x40,
	ADDKEY=0x10,
	SUBBKEY=0x20,
	FUNKEY=0x100,
	RSTKEY=0x200

};
enum{
	nomal_alarm=1,
	important_alarm
};
enum {
	MENU1 =1,
	MENU2 ,
	MENU3 ,
	MENU4 ,
	MENU5 ,
	MENU6 ,
	MENU7 ,
	MENU8 ,
	MENU9 ,
	MENU81 ,
	MENU82 ,
	MENU91 ,
	MENU92 ,
	MENU93 ,
	MENU911 ,
	MENU9111 ,
	MENU9112 ,
	MENU912 ,
	MENU931 ,
	MENU9311 ,
	MENU93111 ,
	MENU9312 ,
	MENU93121 ,
	MENU9313 ,
	MENU93131 ,
	MENU811 ,
	MENU8111 ,
	MENU11,
	MENU111,
	MENU112,
	MENU113,
	MENU12,
	MENU13,
	MENU14,
	MENU15,
	MENU16,
	MENU17,
	MENU121,
	MENU131,
	MENU21,
	MENU22,
	MENU23,
	MENU24,
	MENU31,
	MENU32,
	MENU33,
	MENU34,
	MENU35,
	MENU210,
	MENU211,
	MENU212,
	MENU213,
	MENU214,
	MENU311,
	MENU312,
	MENU313,
	MENU314,
	MENU315,
	MENU321,
	MENU331,
	MENU341,
	MENU3411,
	MENU2111,
	MENU2121,
	MENU2131,
	MENU2141,
	MENU3111,
	MENU3121,
	MENU3131,
	MENU3141,
	MENU31411,
	MENU3211,
	MENU3311,
	MENU41,
	MENU42,
	MENU43,
	MENU44,
	MENU45,
	MENU411,
	MENU412,
	MENU413,
	MENU414,
	MENU415,
	MENU416,
	MENU4111,
	MENU421,
	MENU4211,
	MENU431,
	MENU4311,
	MENU441,
	MENU442,
	MENU443,
	MENU444,
	MENU445,
	MENU4411,
	MENU4421,
	MENU4431,
	MENU4441,
	MENU44111,
	MENU44211,
	MENU44311,
	MENU44411,
	MENU51,
	MENU52,
	MENU53,
	MENU511,
	MENU512,
	MENU513,
	MENU514,
	MENU515,
	MENU516,
	MENU517,
	MENU5161,
	MENU51611,
	MENU516111,
	MENU5162,
	MENU5111,
	MENU5121,
	MENU5131,
	MENU5141,
	MENU51111,
	MENU51211,
	MENU51311,
	MENU51411,
	MENU5151,
	MENU5152,
	MENU51511,
	MENU515111,
	MENU5151111,
	MENU515112,
	MENU51512,
	MENU515121,
	MENU51513,
	MENU515131,
	MENU51514,
	MENU515141,
	MENU51515,
	MENU515151,
	MENU51516,
	MENU515161,
	MENU51517,
	MENU515171,
	MENU51518,
	MENU515181,
	MENU51519,
	MENU515191,
	MENU5151a,
	MENU5151a1,
	MENU5151b,
	MENU5151b1,
	MENU5151c,
	MENU5151c1,
	MENU5151d,
	MENU5151d1,
	MENU5151e,
	MENU5151e1,
	MENU5151f,
	MENU5151f1,
	MENU5151g,
	MENU5151g1,
	MENU5151h,
	MENU5151h1,
	MENU5151j,
	MENU5151j1,
	MENU5151i,
	MENU5151i1,
	MENU5151k,
	MENU5151k1,
	MENU5151l,
	MENU5151l1,
	MENU5151m,
	MENU5151m1,
	MENU5151n,
	MENU5151n1,
	MENU521,
	MENU531,
	MENU61,
	MENU62,
	MENU63,
	MENU64,
	MENU65,
	MENU611,
	MENU621,
	MENU631,
	MENU641,
	MENU71,
	MENU711,
	MENU712,
	MENU713,
	MENU714,
	MENU7111,
	MENU7121,
	MENU7131,
	MENU7141,
	MENU715,
	MENU7151,
	MENU716,
	MENU7161,
	MENU71611,
	MENU7162,
	MENU71621,
	MENU7163,
	MENU71631,
	MENU717,
	MENU718,
	MENU7171
};
enum {
	dispbool_1=1,
	dispbyte_2,
	disphex_4,
	dispDWORD_4,
	dispfloat_5
};
	enum{
		err_psw=1
	};
#define BSP_LIGHT_BEIGUANG_ID 0x800000
typedef struct {
char  name[20];
char valattr;/*存储属性*/
char dispattr;/*显示属性*/
DWORD *pval;
float maxval[2];
char  description[20];
}Vrow;
	typedef struct{
	  BYTE x;
	  BYTE y;
	  BYTE num;
} mousestruct;
	typedef struct{
		BYTE eventflag;
	  WORD WarnEventoff;
	  WORD DoEventoff;
	  WORD ActEventoff;
} VEventoff;
	
class VPcol;
class Cdisp  : public VPcol
{

public:

	Cdisp();
	void grun(DWORD evFlag);
	virtual ~Cdisp();
	  void (Cdisp::*fun_menu)();
	void init(WORD wTaskID);

	
public:
	BYTE addalarmmenu(void (Cdisp::*fun)(), WORD ID,WORD TOPID);
	BYTE addturnmenu(void (Cdisp::*fun)(), WORD ID,WORD TOPID);
	BYTE Comparebuf();
	WORD Crc16(WORD SpecCode, BYTE *p, int l);
	void doykyzovertime();
	void doyk();
	void dofrontyk();
	void dealkey(WORD keyvalue);
	void dealrxddata();
	void DoTimeOut(WORD wTimerID);
	void DoReadCommEvent(void);
	BYTE gotohead();
	float getdispdata(BYTE *psrc,BYTE n1,BYTE n2);
	void M811_tiaozhatoutui();
	void M310_DownData();
	void M314_YK();
	void M210_DownData();
	void M213_SOE();
	void M212_YX();
	void M211_YC();
	void M113_meterage();
	void M112_meterage();
	void M111_meterage();
	void M41_viewmsg();
	void M411_runinfo();
	void M716_jueyuanzd();
	void M421_alarminfo();
	void M9_proset();
	void M410_Event();
	void setdecnum(BYTE *psrc);
	void selecteqp();
	int sendtoscreen();
	void selectykaction();
	void selectykeqp();
	void M_YC();
	void dealreckey();
	void MainMenu(void);
	void WriteDispBuff(char* psrc,BYTE x,BYTE y,BYTE len);
	BYTE TxdDispallscreen1();
	BYTE TxdDispallscreen2();
	BYTE TxdDispallscreen3();
	BYTE TxdDispallscreen4();
	BYTE TxdDispallscreen5();
	void Entersonmenu();
	void Returntopmenu();
	void Return2topmenu();
	void Downmenu();
	void dealupdownvalue(int ttl,BYTE row);
	void clearscreen();
	void dealleftrightvalue(int ttl);
	void pgupgroupeqp();
	void pgdngroupeqp();
	void selectgroupeqp();
	void M51_setpara();
	void seteqpip1();
	void seteqpaddr();
	void seteqpip2();
	void geteqpid();
	void listdowneqp();
	void listupeqp();
	void listeqp_1();
	void pgupeqp();
	void pgdneqp();
	void procYKMsg(BYTE msgId);
	void DoTSDataMsg();
	void doykcx();
	void doykfh07();
	void doykfh08();
	void doykfh09();
	void doykfh10();
	void doykfh11();
	void doykfh12();
	void doykfh13();
	void doykfh14();
	void doykfh15();
	void doykfh16();
	void doykfh17();
	void doykfh18();
	void checkpwd();
	void checkpwd1();
	void checkhomepwd();
	void dispsetclock();
	void M610_viewpara();
	void M611_basemessage();
	void M613_verinfo();
	void M71_hometest();
	void M711_screentest();
	void M712_stopdog();
	void M717_reboot1();
	void M713_yxtest();
	void M714_yktest();
	void M715_zdtest();
	void M716_bmtest();
	void Mset_success_and_reset();

	void infoview(BYTE attr);
	void infoalarmview(BYTE attr);
	void M410_DoEvent();
	void M4_key(WORD poolNum);
	void M410_AlarmEvent();
	void M410_ActEvent();
	void M410_Eventnr();
	BYTE TxdBlacklight();
	void M410_clearEvent();
	void M411_clearEvent();
	void M412_clearEvent();
	void M413_clearEvent();
	void M440_clearEventitem();
	void M52_setpara();
	void M521_selectline();
	void M931_selectline();
	void M911_disp_proYB();
		void M932_looklinepara();

	BYTE zhengDispData1();
	BYTE fanDispData();
	BYTE fanDispData1();
	BYTE zhengDispData2();
	BYTE zhengDispData3();
	BYTE zhengDispData4();
	BYTE zhengDispData5();
	BYTE zhengDispData6();
	BYTE zhengDispData7();
	BYTE zhengDispData8();
	void disp_description(BYTE row);
	void disp_setrow(BYTE row);
	void disp_setmouse(BYTE key);
	void set_menu();
	void dispclock();
	void disp_setmouse1(BYTE key);
	void disp_dealupdownkey(BYTE key);
	void dealpopalarm();
	void delete_alarmmenu();
	void M9311_setPT();
	void M9311_setCT();
	void M9311_setCT0();
		void Malarm_event();
		void Malarm_eventinit();
		void Mpwd_err();
		void Mset_success();
		void dispcontrolword();
		void Mset_YB();
		void Mdisp_YB(BYTE row);
		void disp_currow(BYTE row);
		void disp_con(BYTE row,const TKGTABLE *ptkg);
		void Mset_con(const TKGTABLE *ptkg);
		void Mmeter_success();
		void M110_rundata();
		char * delchar(char str[]);
		void Mmea_YC();
		void Mmea_YC1();
		void Mmea_YC2();
		void menucycle();
		int pro_init(int proset);
		int sendtopopscreen();
		void set_proIP();
		void set_pro();
		void set_proYB();
		void set_proset();
		BYTE Get_zhengshu(BYTE *psrc,BYTE num);
		void readYB();
		void writeYB();
		void readcon();
		void writecon();
		void set_proCON1();
		void changestring(char *src,char *dec);
		void M912_disp_proCON1();
		void M911_selectline();
		void M9_Setting_management(void);
			void Setlinpara(BYTE line);
			void M9311_setlinepara();
			void getlinpara(BYTE line);
			void addalarmnum(BYTE num);

	//
protected:
	BYTE m_refreshrow;
	WORD m_keyval;
	WORD m_keybak;
	BYTE m_dispcmd;
	BYTE m_secbak;
	BYTE m_ykstate;
	BYTE m_ykdelay;
	BYTE m_setkeyvalue;/*光标左右移动的值*/
	BYTE m_wrrdfalg;
	BYTE trip_flag;//保护传动跳闸、告警标志
	WORD selftest_tm;//保护传动自动测试最大时间
	WORD exit_tm;//保护传动自动测试退出时间
		WORD MASK;
typedef struct{
	  BYTE keyvalue[5];
	  void (Cdisp::*fun)();
} menustruct;
 typedef struct _winmenu{
	struct _winmenu*  topmenu;
	struct _winmenu * sunmenu;
	struct _winmenu * beforemenu;
	struct _winmenu *nextmenu;
	  BYTE 	para;
	  BYTE 	key;
	  BYTE 	flag;
	  BYTE 	row;
	  BYTE 	page;
	  WORD     ID;
	  void (Cdisp::*fun)();
}  winmenu;
 winmenu   mainwinmenu;
 winmenu   turnwinmenu;
 winmenu   alarmwinmenu;
winmenu  *pwinmenu;
winmenu  *pturnmenu;
	BYTE addmenu(void (Cdisp::*fun)(), WORD ID,WORD TOPID);
	winmenu* searchtopnextmanu(winmenu * p);
		winmenu* searchtopmanu(WORD TOPID);
	typedef struct{
	  BYTE x;
	  BYTE y;
	  BYTE num;
} mousestruct;
	VSysClock   clock;/*时钟*/
	VCalClock m_oldclock;
	typedef struct{
		VSysClock lastresettime;
		WORD resetnum;
		} _serrmsg;
	_serrmsg errmsg;
	DWORD m_lamp_state;/*灯的状态*/
	BYTE m_comm_num;
	BYTE m_txdbuf[250];
	BYTE m_txdlen;
	BYTE m_rxdlen;
	BYTE m_deallen;
	BYTE m_rxdbuf[250];
	int m_menunum;/*最大菜单数*/
	BYTE m_dispbuf[500];
	BYTE m_bakdispbuf[200];
	BYTE m_dwPubBuf[4000];
	char *tt;
	//BYTE m_y;//光标
	//BYTE m_x;//光标
	menustruct* pmenu;/*菜单指针*/
	BYTE m_curmenu;
	BYTE m_eqpdot;
	BYTE m_ykdot;
	BYTE m_ykflag;
	BYTE m_turnnext;
	BYTE m_turnstop;
	BYTE m_turnflag;
	BYTE m_backlightflag;
	BYTE m_clritem;
	BYTE m_conindex;
	WORD m_wTaskID;
	BYTE m_curvalue[6];/*菜单每一层的值*/
	//BYTE m_num;
	BYTE m_commtimeout;/*通讯超时*/
	BYTE m_keyupdownval;/*上下键值*/
	BYTE m_keyleftrightval;/*左右键值*/
	WORD m_conword[4];
	WORD m_eqpid[200];
	WORD m_cureqpid;
	WORD m_lamp_delay;
	BYTE m_curCtrlMsgID;
	BYTE m_mousevalue[10];
	BYTE m_YKflag;
	BYTE m_dispflag;
	BYTE m_yktesttime;
	BYTE  m_pwd_flag;
	BYTE m_controlword_dispflag;
	BYTE m_keylongdowntime;
	WORD m_backlighttm;
	WORD m_turndispflag;
	WORD m_YKID;
	WORD m_soeptr;
	DWORD m_huohuajiange;
	DWORD m_lamp_bak;
	BYTE m_huohuaturn;
	BYTE m_deadval;
	BYTE m_reset_delay;
	BYTE m_reflash_delay;
	BYTE m_popup_flag;
	BYTE m_pwd[4];
	WORD m_reflash_time;
	tagTYBTABLE* m_yb_miaoshu;
	TKGTABLE* m_con1_miaoshu;
	char* m_con2_miaoshu;
	TKGTABLE* m_con3_miaoshu;
	char* m_con4_miaoshu;
	tagTSETTABLE* m_set_miaoshu;
	TSETVAL* m_proset;
	BYTE* pproset;
	WORD m_pronum;
	WORD m_ybnum;
	WORD m_CON1num;
	WORD m_CON2num;
	WORD m_CONBYTEnum;
	WORD m_setnum;
	WORD m_seccount;
	DWORD m_tmpYB;
	DWORD m_lamp_turn_light;
	DWORD* m_pcon;
	mousestruct newmouse;
	mousestruct oldmouse;
//	VEqpInfo*  m_curdispeqp;
	VEqpInfo*  m_pcurdispeqp[4];
	VEqpInfo*  m_pgroupeqp[4];
	VEqpInfo *m_pEqpInfo;	 
	VEqpInfo*  m_pgroupeqpinfo;
	VMsg DispMsg;
	VDBYK    m_YKmsg;
	char sYKNameTab[GEN_NAMETAB_LEN];
	struct VSysEventInfo m_Eventinfo[4];
	mousestruct m_setmouse;
	VEventoff    m_Eventoff;
	int pt[6]; 
	BYTE alarmdispptr[33];
	BYTE m_err_No;
	BYTE m_disp_tm;
	BYTE m_moniflag;

	int m_thid;
	int m_commid;
	BOOL m_bIn;  //mmi板在线
	DWORD m_dwYx;
	DWORD m_key;
	BYTE m_size;
	
	BYTE m_disp_model;
    WORD m_wEqpNum;
	WORD m_wEqpGroupID;
	struct VEqpGroupInfo m_EqpGroupInfo;
	struct VTVEqpInfo* m_ownEqpInfo;
	WORD *m_pwEqpID;//by lvyi
	//WORD  m_wEqpID;
	WORD *m_wEqpID;
	BOOL  m_disp_init;
	

	VExtMMIFrmHead *m_pSend; 
	VExtMMIFrmHead *m_pRec;
	DWORD m_dwCommCtrlTimer;
	BYTE m_byCnt;
	BOOL m_bLight;
	BYTE m_byBuf[EXTMMI_BUF_MAX];
	
	virtual DWORD SearchOneFrame(BYTE *Buf,WORD Len);

	void SendFrameHead(BYTE fmType, BYTE fmFun, BYTE fmCtrl);
	void SendFrameTail();		
	virtual int readComm(BYTE *buf, int len, struct timeval *tmval); 			
	virtual int writeComm(BYTE *buf, int len, DWORD dwFlag);

	virtual int ProcFrm(void);	
	WORD SendLight(DWORD light);
	void SendConnect(void);
	void SendConf(BYTE byType,BYTE code,BYTE DispAddr);// by lvyi 带上地址

	void ProcConnect(void);
	void ProcYx(void);
	void ProcProg(void);
	
    BOOL InitEqpInfo(void);
	void SendLightMd(BYTE DispAddr);// by lvyi 带上地址
	void ProcYkMd(void);

	};


#endif // !defined(AFX_DISP_H__73A39485_3387_49B0_B7AB_0A2DBA6C9B32__INCLUDED_)
