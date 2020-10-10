/*
 * FileName:       
 * Author:         shuaifuqiang  Version: v1.0  Date: 2008-12-21
 * Description:    
 * Version:        
 * Function List:  
 *                 1.
 * History:        
 *     <author>   <time>    <version >   <desc>
 */
 
/*------------------------------------------------------------------------
  $Rev: 12 $
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef INCLUDE_EXT_DISP

#include <string.h>
#include <os.h>
#include "extdisp.h"
#include "mmi_drv.h"
#include "extmmidef.h"

#define maxdatarow   4
#define maxcol   31
#define KEYSCANTIME 10


//////////////////////////////////////////////////////////////////////
#ifdef INCLUDE_PR
extern BYTE prTableBuf[MAX_TABLE_FILE_SIZE];
#endif

DWORD LedLight = 0;
static DWORD MMIYx;
static int  mmi_init=0;

static BYTE m_key_temp=20;

static BYTE commcunt;
static BYTE timecnt=0;


static BYTE sendflag=0;
//static BYTE ligcunt=0;

extern WORD pannelYx;
WORD extSwitch;
extern struct monSerial g_shell_serial;

//by lvyi
BYTE yk_flag=0;
int MdFlag = 0;
DWORD MMI_FileLen=0;
int UpdateTime = 0;
BYTE pubval=0;
static Cdisp* Disp;

static char *unit[50]={
	"V",	
	"V",
	"V",
	"V",
	"V",   /*开口U0*/
	"V",  /*自产U0*/ 
		
	"A",
	"A",
	"A",
	"A",
	"A",  /*自产I0*/ 
		
	"Kw",
	"Kw",
	"Kw",  
		
	"Kvar",  
	"Kvar",   
	"Kvar",    
		
	"V",  
	"V",  
	"V", 
		
	"Kw",    
	"Kvar",  
	"KVA",    
	"",   
		
	"Hz",
	
	"V",  /*自产Ua*/
	"V",  /*自产Ub*/
	"V",  /*自产Uc*/

	"A",  /*自产Ia*/
	"A",  /*自产Ib*/
	"A",  /*自产Ic*/

	"",
	
	"",
	"",

	"V",
	"Hz",
	"",
	"",


};

 
void mmiLight(int id, int on)
{
   DWORD light;
   DWORD bit;

   light = LedLight;

  // if (id == BSP_LIGHT_WARN_ID) id = 0;
   if(id < 1) return;

   bit = 0x01 << (id-1);

   if (on)  light |= bit;
   else light &= (~bit);		

   LedLight = light;
}

int GetExtMmiYx(int index)
{

    if (mmi_init == 0) return 0;

    if (MMIYx & (0x01<<index))
		return 1;
	else
		return 0;
}

	
extern "C" void extdisp_init(int thid)
{
	Disp = new Cdisp(); 	
	Disp->init(thid);
}

extern "C" void extdisp_run(DWORD evFlag)
{
	Disp->grun(evFlag);
}

/*
 * Function:       
 * Description:    
 * Calls:          
 * Called By:      
 * Table Accessed: 
 * Table Updated:  
 * Input:          
 * Output:         
 * Return:         
 * Others:         
 */
Cdisp::Cdisp():VPcol()
{
	memset(	m_dispbuf,0x20,sizeof(m_dispbuf));
	m_dispcmd=0;
	m_pSend = (VExtMMIFrmHead *)m_Send.pBuf;        
	m_pRec = NULL;
	m_byCnt = 0;
	m_bIn = FALSE;
	m_dwCommCtrlTimer = 50;
	m_size=31;	
}

Cdisp::~Cdisp()
{
	delete pmenu;
}

void Cdisp::init(WORD wTaskID)
{
	m_seccount = 0;
	tt=(char*)m_dwPubBuf;
	m_commid = wTaskID;//mmi ,30
	#if(TYPE_USER == USER_GUIYANG)
		MMIYx = (0x01<< EXTMMI_TRIP) | (0x01<<EXTMMI_FA)| (0x01<<EXTMMI_AUTO);
	#else
		MMIYx = (0x01<< EXTMMI_TRIP) | (0x01<<EXTMMI_FA);
	#endif
	mmi_init = 1;
	newmouse.x=0;
	newmouse.y=2;
	m_curmenu=0;
	m_reflash_delay=3;
	memset(m_curvalue,0,6);
	m_curvalue[0]=1;
	m_wrrdfalg=0;
	MASK=0;

	m_lamp_delay=0;
	m_commtimeout=8;
	m_wTaskID=wTaskID;
	m_popup_flag=0;
	m_turnflag=0;
	m_turndispflag=TURNTIME;

	Malarm_eventinit();
	pwinmenu=&mainwinmenu;
	pturnmenu=&turnwinmenu;
	mainwinmenu.sunmenu=NULL;
	alarmwinmenu.sunmenu=new winmenu;
	turnwinmenu.nextmenu=NULL;

	if (addmenu(&Cdisp::MainMenu,(WORD)MENU1,(WORD)0)==false) return;
	if (addmenu(&Cdisp::MainMenu,(WORD)MENU9,0)==false) return;
	if (addmenu(&Cdisp::MainMenu,(WORD)MENU2,0)==false) return;
	if (addmenu(&Cdisp::MainMenu,(WORD)MENU3,0)==false) return;
	if (addmenu(&Cdisp::MainMenu,(WORD)MENU6,0)==false) return;
	if (addmenu(&Cdisp::MainMenu,(WORD)MENU51,0)==false) return;
	if (addmenu(&Cdisp::MainMenu,(WORD)MENU4,0)==false) return;
	if (addmenu(&Cdisp::MainMenu,(WORD)MENU7,0)==false) return;
	
	if (addmenu(&Cdisp::M110_rundata,(WORD)MENU11,(WORD)MENU1)==false) return;
	if (addmenu(&Cdisp::M110_rundata,(WORD)MENU12,(WORD)MENU1)==false) return;
	if (addmenu(&Cdisp::M110_rundata,(WORD)MENU13,(WORD)MENU1)==false) return;
	
	if (addmenu(&Cdisp::M111_meterage,(WORD)MENU111,(WORD)MENU11)==false) return;
	if (addmenu(&Cdisp::M112_meterage,(WORD)MENU112,(WORD)MENU11)==false) return;
	//if (addmenu(&Cdisp::M113_meterage,(WORD)MENU113,(WORD)MENU11)==false) return;
	if (addmenu(&Cdisp::M211_YC,(WORD)MENU121,(WORD)MENU12)==false) return;
	if (addmenu(&Cdisp::Mmea_YC2,(WORD)MENU131,(WORD)MENU13)==false) return;
	
	if (addmenu(&Cdisp::selectgroupeqp,(WORD)MENU210,(WORD)MENU2)==false) return;
	if (addmenu(&Cdisp::listdowneqp,(WORD)MENU31,(WORD)MENU3)==false) return;
	if (addmenu(&Cdisp::listupeqp,(WORD)MENU21,(WORD)MENU210)==false) return;
	
	if (addmenu(&Cdisp::M210_DownData,(WORD)MENU211,(WORD)MENU21)==false) return;
	if (addmenu(&Cdisp::M210_DownData,(WORD)MENU212,(WORD)MENU21)==false) return;
	if (addmenu(&Cdisp::M210_DownData,(WORD)MENU213,(WORD)MENU21)==false) return;
	
	if (addmenu(&Cdisp::M310_DownData,(WORD)MENU311,(WORD)MENU31)==false) return;
	if (addmenu(&Cdisp::M310_DownData,(WORD)MENU312,(WORD)MENU31)==false) return;
	if (addmenu(&Cdisp::M310_DownData,(WORD)MENU313,(WORD)MENU31)==false) return;
	if (addmenu(&Cdisp::M310_DownData,(WORD)MENU314,(WORD)MENU31)==false) return;
	
	if (addmenu(&Cdisp::M_YC,MENU2111,MENU211)==false) return;
	if (addmenu(&Cdisp::M212_YX,MENU2121,MENU212)==false) return;
	if (addmenu(&Cdisp::M213_SOE,MENU2131,MENU213)==false) return;
	if (addmenu(&Cdisp::M_YC,MENU3111,MENU311)==false) return;
	if (addmenu(&Cdisp::M212_YX,MENU3121,MENU312)==false) return;
	if (addmenu(&Cdisp::M213_SOE,MENU3131,MENU313)==false) return;
	if (addmenu(&Cdisp::checkhomepwd,MENU3141,MENU314)==false) return;
	if (addmenu(&Cdisp::M314_YK,MENU31411,MENU3141)==false) return;
	if (addmenu(&Cdisp::M51_setpara,MENU511,MENU51)==false) return;
	if (addmenu(&Cdisp::M51_setpara,MENU512,MENU51)==false) return;
	if (addmenu(&Cdisp::M51_setpara,MENU513,MENU51)==false) return;
	if (addmenu(&Cdisp::M51_setpara,MENU514,MENU51)==false) return;

	
	if (addmenu(&Cdisp::seteqpaddr,MENU5111,MENU511)==false) return;
	if (addmenu(&Cdisp::checkpwd,MENU51111,MENU511)==false) return;
	if (addmenu(&Cdisp::seteqpip1,MENU5121,MENU512)==false) return;
	if (addmenu(&Cdisp::checkpwd,MENU51211,MENU512)==false) return;
	if (addmenu(&Cdisp::seteqpip2,MENU5131,MENU513)==false) return;
	if (addmenu(&Cdisp::checkpwd,MENU51311,MENU513)==false) return;
	
	if (addmenu(&Cdisp::dispsetclock,MENU5141,MENU514)==false) return;
	if (addmenu(&Cdisp::checkpwd,MENU51411,MENU514)==false) return;
	
	if (addmenu(&Cdisp::M610_viewpara,MENU61,MENU6)==false) return;
	if (addmenu(&Cdisp::M610_viewpara,MENU62,MENU6)==false) return;
	if (addmenu(&Cdisp::M610_viewpara,MENU63,MENU6)==false) return;
	if (addmenu(&Cdisp::M610_viewpara,MENU64,MENU6)==false) return;
	
	if (addmenu(&Cdisp::M611_basemessage,MENU611,MENU61)==false) return;
	if (addmenu(&Cdisp::M613_verinfo,MENU621,MENU62)==false) return;
	if (addmenu(&Cdisp::M411_runinfo,MENU631,MENU63)==false) return;
	if (addmenu(&Cdisp::M421_alarminfo,MENU641,MENU64)==false) return;
	
	if (addmenu(&Cdisp::checkhomepwd,MENU71,MENU7)==false) return;
	if (addmenu(&Cdisp::M71_hometest,MENU711,MENU71)==false) return;
	if (addmenu(&Cdisp::M71_hometest,MENU712,MENU71)==false) return;
	if (addmenu(&Cdisp::M71_hometest,MENU713,MENU71)==false) return;
	if (addmenu(&Cdisp::M71_hometest,MENU714,MENU71)==false) return;
	if (addmenu(&Cdisp::M71_hometest,MENU715,MENU71)==false) return;
	if (addmenu(&Cdisp::M71_hometest,MENU716,MENU71)==false) return;
	
	if (addmenu(&Cdisp::M711_screentest,MENU7111,MENU711)==false) return;
	if (addmenu(&Cdisp::M712_stopdog,MENU7121,MENU712)==false) return;
	if (addmenu(&Cdisp::M713_yxtest,MENU7131,MENU713)==false) return;
	if (addmenu(&Cdisp::M714_yktest,MENU7141,MENU714)==false) return;
	if (addmenu(&Cdisp::M715_zdtest,MENU7151,MENU715)==false) return;
	if (addmenu(&Cdisp::M716_bmtest,MENU7161,MENU716)==false) return;
	
	if (addmenu(&Cdisp::M41_viewmsg,MENU41,MENU4)==false) return;
	if (addmenu(&Cdisp::M41_viewmsg,MENU42,MENU4)==false) return;
	if (addmenu(&Cdisp::M41_viewmsg,MENU43,MENU4)==false) return;
	if (addmenu(&Cdisp::M41_viewmsg,MENU44,MENU4)==false) return;
	
	if (addmenu(&Cdisp::M410_ActEvent,MENU411,MENU41)==false) return;
	if (addmenu(&Cdisp::M410_DoEvent,MENU421,MENU42)==false) return;
	if (addmenu(&Cdisp::M410_AlarmEvent,MENU431,MENU43)==false) return;
	
	if (addmenu(&Cdisp::M440_clearEventitem,MENU441,MENU44)==false) return;
	if (addmenu(&Cdisp::M440_clearEventitem,MENU442,MENU44)==false) return;
	if (addmenu(&Cdisp::M440_clearEventitem,MENU443,MENU44)==false) return;
	if (addmenu(&Cdisp::M440_clearEventitem,MENU444,MENU44)==false) return;
	
	if (addmenu(&Cdisp::checkpwd1,MENU4411,MENU441)==false) return;
	if (addmenu(&Cdisp::checkpwd1,MENU4421,MENU442)==false) return;
	if (addmenu(&Cdisp::checkpwd1,MENU4431,MENU443)==false) return;
	if (addmenu(&Cdisp::checkpwd1,MENU4441,MENU444)==false) return;
	if (addmenu(&Cdisp::M410_clearEvent,MENU44111,MENU441)==false) return;
	if (addmenu(&Cdisp::M411_clearEvent,MENU44211,MENU442)==false) return;
	if (addmenu(&Cdisp::M412_clearEvent,MENU44311,MENU443)==false) return;
	if (addmenu(&Cdisp::M413_clearEvent,MENU44411,MENU444)==false) return;
	
	if (addmenu(&Cdisp::M410_Eventnr,MENU4111,MENU411)==false) return;
	if (addmenu(&Cdisp::M410_Eventnr,MENU4211,MENU421)==false) return;
	if (addmenu(&Cdisp::M410_Eventnr,MENU4311,MENU431)==false) return;
	
	if (addmenu(&Cdisp::M9_Setting_management,MENU91,MENU9)==false) return;
	if (addmenu(&Cdisp::M9_Setting_management,MENU92,MENU9)==false) return;
	if (addmenu(&Cdisp::M9_Setting_management,MENU93,MENU9)==false) return;
	
	if (addmenu(&Cdisp::M911_selectline,MENU911,MENU91)==false) return;
	if (addmenu(&Cdisp::M9_proset,MENU912,MENU911)==false) return;
	if (addmenu(&Cdisp::M911_disp_proYB,MENU9111,MENU912)==false) return;
	if (addmenu(&Cdisp::M912_disp_proCON1,MENU9112,MENU912)==false) return;
	
	if (addmenu(&Cdisp::M521_selectline,MENU5151,MENU92)==false) return;
	if (addmenu(&Cdisp::set_menu,MENU51511,MENU5151)==false) return;
	if (addmenu(&Cdisp::set_pro,MENU515111,MENU51511)==false) return;
	if (addmenu(&Cdisp::checkpwd,MENU51512,MENU5151)==false) return;
	
	if (addmenu(&Cdisp::M931_selectline,MENU931,MENU93)==false) return;
	if (addmenu(&Cdisp::M932_looklinepara,MENU9311,MENU931)==false) return;
	if (addmenu(&Cdisp::M9311_setlinepara,MENU93111,MENU9311)==false) return;
	if (addmenu(&Cdisp::checkpwd,MENU93131,MENU931)==false) return;

	addturnmenu(&Cdisp::Mmea_YC,MENU11,MENU1);

	pwinmenu=mainwinmenu.sunmenu;
	m_backlighttm=0;
	m_comm_num=0;
	m_lamp_delay=0;
	m_popup_flag=0;
	m_backlightflag=0;
	m_reflash_delay=0;
	pro_init(1);
	m_lamp_turn_light=0;
	m_disp_model = EXTDISP_MODEL_DISP;
	m_disp_init = InitEqpInfo();
}

/***************************************************************
	Function:InitEqpInfo
		初始化装置信息
	参数：无
	
	返回：TRUE 成功，FALSE 失败 
***************************************************************/
BOOL Cdisp::InitEqpInfo(void)
{
	WORD wEqpID;
	VTVEqpInfo EqpInfo;
	VEqpGroupInfo *pEqpGroupInfo = NULL;
	int EqpNo;
	
	m_wEqpNum = GetTaskEqpNum(m_wTaskID);

	if (m_wEqpNum==0)  return(TRUE);
	
	if (m_wEqpNum > 1)// 1个设备组，2个虚设备
	{
		myprintf(m_wTaskID, LOG_ATTR_INFO, "MMI Init EqpInfo Error,m_wEqpNum > 2.");
		return FALSE;	
	}

	GetTaskEqpID(m_wTaskID, &m_wEqpGroupID);
	if (ReadEqpInfo(m_wTaskID, m_wEqpGroupID, &EqpInfo, TRUE) == ERROR)
	{
		myprintf(m_wTaskID, LOG_ATTR_INFO, "MMI Init EqpInfo Error,Read EqpGroup Info Error.");
		return FALSE;
	}
	if (EqpInfo.wType != GROUPEQP)
	{
		myprintf(m_wTaskID, LOG_ATTR_INFO, "MMI Init EqpInfo Error,it is not EqpGroup.");
		return FALSE;	
	}
	g_Sys.Eqp.pInfo[m_wEqpGroupID].wTaskID = MMI_ID;
	pEqpGroupInfo = (VEqpGroupInfo *)&EqpInfo;
	m_EqpGroupInfo.wType = pEqpGroupInfo->wType;
	m_EqpGroupInfo.wSourceAddr = pEqpGroupInfo->wSourceAddr;
	m_EqpGroupInfo.wDesAddr = pEqpGroupInfo->wDesAddr;
	m_EqpGroupInfo.dwFlag = pEqpGroupInfo->dwFlag;
	m_EqpGroupInfo.wEqpNum = pEqpGroupInfo->wEqpNum;
	m_EqpGroupInfo.pEqpRunInfo = pEqpGroupInfo->pEqpRunInfo;
	m_EqpGroupInfo.pwEqpID = pEqpGroupInfo->pwEqpID;
	
	m_wEqpNum = m_EqpGroupInfo.wEqpNum; 					
	m_ownEqpInfo = (VTVEqpInfo*)calloc(m_wEqpNum, sizeof(VTVEqpInfo));

	if ((m_wEqpNum==0)||(!m_ownEqpInfo))
	{
		myprintf(m_wTaskID, LOG_ATTR_INFO, "MMI Init EqpInfo Error, real device is null.");
		return FALSE;
	}

	m_pwEqpID = m_EqpGroupInfo.pwEqpID;
	//by lvyi
	//m_wEqpID = m_pwEqpID[0];
	m_wEqpID = m_pwEqpID;
	for (EqpNo = 0; EqpNo < m_wEqpNum; EqpNo++)
	{
		wEqpID = m_pwEqpID[EqpNo];
		if (ReadEqpInfo(m_wTaskID, wEqpID, &EqpInfo, TRUE) == ERROR)
		{
			myprintf(m_wTaskID, LOG_ATTR_INFO, "MMI Init EqpInfo Error,ReadEqpInfo Error.");
			return FALSE;
		}
		m_ownEqpInfo[EqpNo].dwFlag= EqpInfo.dwFlag;
		m_ownEqpInfo[EqpNo].wYCNum = EqpInfo.wYCNum;				 
		m_ownEqpInfo[EqpNo].wVYCNum = EqpInfo.wVYCNum;				 
		m_ownEqpInfo[EqpNo].wDYXNum = EqpInfo.wDYXNum;			 
		m_ownEqpInfo[EqpNo].wSYXNum = EqpInfo.wSYXNum;			 
		m_ownEqpInfo[EqpNo].wVYXNum = EqpInfo.wVYXNum;			 
		m_ownEqpInfo[EqpNo].wDDNum = EqpInfo.wDDNum;				 
		m_ownEqpInfo[EqpNo].wYKNum = EqpInfo.wYKNum;				 
		m_ownEqpInfo[EqpNo].wTSDataNum = EqpInfo.wTSDataNum;		 
		m_ownEqpInfo[EqpNo].wYTNum = EqpInfo.wYTNum;				 
		m_ownEqpInfo[EqpNo].wTQNum = EqpInfo.wTQNum;				 
		m_ownEqpInfo[EqpNo].pEqpRunInfo = EqpInfo.pEqpRunInfo;  
	
	
	}//end of for
	
	return TRUE;
}

DWORD Cdisp::SearchOneFrame(BYTE *Buf, WORD Len)
{
	VExtMMIFrmHead *pHead;
	int crclen;
	WORD crc,crc1;

	if (Len < sizeof(VExtMMIFrmHead))  return(PCOL_FRAME_LESS);
		
	pHead = (VExtMMIFrmHead *)Buf;

	if ((pHead->byCode1 != EXTMMI_CODE1) || (pHead->byCode2 != EXTMMI_CODE2))  return(PCOL_FRAME_ERR|1);    

	if (pHead->byLen > EXTMMI_MAXFRM_LEN) return(PCOL_FRAME_ERR|1);

	crclen = EXTMMI_CRCLEN;

	if (Len < (pHead->byLen+crclen)) return(PCOL_FRAME_LESS);

	crc = Buf[pHead->byLen]<<8|Buf[pHead->byLen+1];

	crc1 = Crc16(0xFFFF, Buf, pHead->byLen);

	if (crc != crc1) 
	{
	   logMsg("crc error %x, %x\n",crc,crc1,0,0,0,0);
	   return(PCOL_FRAME_ERR|1);
	}

	return(PCOL_FRAME_OK|(pHead->byLen+crclen));    
}

void Cdisp::SendFrameHead(BYTE fmType, BYTE fmFun, BYTE fmCtrl)
{
    m_Send.wReadPtr = 0;

	m_pSend->byCode1 = EXTMMI_CODE1;
	m_pSend->byCode2 = EXTMMI_CODE2;

    if (fmCtrl & EXTMMI_CTRL_MASTER)    
		m_pSend->byCnt = ++m_byCnt;
	else
		m_pSend->byCnt = m_pRec->byCnt; 
	
	m_pSend->byFun = fmFun;
	m_pSend->byType = fmType;
	m_pSend->byCtrl = fmCtrl;

    m_Send.wWritePtr = sizeof(VExtMMIFrmHead);
}

void Cdisp::SendFrameTail()
{  
	WORD crcCode;
	
    m_pSend->byLen = m_Send.wWritePtr;
	
	crcCode = Crc16(0xFFFF,m_Send.pBuf,m_Send.wWritePtr);
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(crcCode);
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(crcCode);
		
	DoSend((m_pSend->byCtrl<<24));   
}  

int Cdisp::writeComm(BYTE *buf, int len, DWORD dwFlag)
{       
	if (len)
	{
		len = ::commWrite(m_commid, buf, len, dwFlag);

		if ((dwFlag & (EXTMMI_CTRL_MASTER<<24)) == 0) return len;
		
	}
	
	//commCtrl(m_commid, CCC_EVENT_CTRL|COMM_IDLE_ON, &m_dwCommCtrlTimer);

	return len;
}

int Cdisp::readComm(BYTE *buf, int len, struct timeval *tmval)
{
    return(commRead(m_commid, buf, len, 0));
}

void Cdisp::SendConnect(void)
{
	int len;
    SendFrameHead(EXTMMI_TYPE_CONNECT, EXTMMI_FUN_CTRL, EXTMMI_CTRL_MASTER);
	m_Send.pBuf[m_Send.wWritePtr++] = 0x5A;
	len = strlen(g_Sys.InPara[SELFPARA_MFR_GB2312]);
	m_Send.pBuf[m_Send.wWritePtr++] = len;
	memcpy(&m_Send.pBuf[m_Send.wWritePtr],&g_Sys.InPara[SELFPARA_MFR_GB2312][0],len);
	m_Send.wWritePtr+=len;
	SendFrameTail();
}

WORD Cdisp::SendLight(DWORD light)
{
	m_bLight = TRUE;
    if (!m_bIn) return 0;
	SendFrameHead(EXTMMI_TYPE_LIGHT, EXTMMI_FUN_WRITE, EXTMMI_CTRL_MASTER);
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(LOWORD(light));
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(LOWORD(light));
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(HIWORD(light));
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(HIWORD(light));
	
	SendFrameTail();

	return m_Send.wWritePtr;
}
//by lvyi,带上地址DispAddr
void Cdisp::SendLightMd(BYTE DispAddr)
{
  int i,num,j;
	BYTE buf[128];
	BYTE LED0=0;
	WORD wSendNum;
	BYTE YXValue;
	
    if (!m_bIn) return;

	if((DispAddr + 1)>m_wEqpNum)
	{
		return;
	}
	
	memset(buf, 0, 128);
	//by lvyi
	if((LedLight & BSP_LIGHT_COMM_ID) == BSP_LIGHT_COMM_ID)
	  	LED0 |= 0x02;
	  else
	  	LED0 &= ~0x02;

	  if((LedLight & BSP_LIGHT_WARN_ID) == BSP_LIGHT_WARN_ID)
	  	LED0 |= 0x01;
	  else
	  	LED0 &= ~0x01;

		//i = ReadRangeSYXBit(m_wEqpID[DispAddr], 0, m_ownEqpInfo[DispAddr].wSYXNum, 128, buf);
		i = 0;
		for(j = 0; j < m_ownEqpInfo[DispAddr].wSYXNum; j++)
		{
			ReadSYXSendNo(m_wEqpID[DispAddr],j,&wSendNum); //发送序号一样 多走一遍而已
			if(ReadRangeSYX(m_wEqpID[DispAddr], j, 1, 1, &YXValue) != 1)
				break;
			i = wSendNum+1;
			if(YXValue & 0x80)
				buf[wSendNum/8] |= 1 << (wSendNum%8);
			else
				buf[wSendNum/8] &= ~(1 << (wSendNum%8));
		}
		
		num = i/8;
		if (i%8) num++;

        SendFrameHead(EXTMMI_TYPE_LIGHTMD, EXTMMI_FUN_WRITE, EXTMMI_CTRL_MASTER|(DispAddr<<6));
        if(m_disp_model == EXTDISP_MODEL_LAMP)
        {
            m_Send.pBuf[m_Send.wWritePtr++] = num+2;//by lijun
            m_Send.pBuf[m_Send.wWritePtr++] = LED0;// by lijun
            m_Send.pBuf[m_Send.wWritePtr++] = i; //by lijun
        }
        else
        {
            m_Send.pBuf[m_Send.wWritePtr++] = num+1;//by lvyi
            m_Send.pBuf[m_Send.wWritePtr++] = LED0;// by lvyi
        }
        for (i=0; i<num; i++)
            m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(buf[i]);
        SendFrameTail();	
}

void Cdisp::SendConf(BYTE byType,BYTE code,BYTE DispAddr)
{
    SendFrameHead(byType, EXTMMI_FUN_CONF, 0|(DispAddr<<6));
	m_Send.pBuf[m_Send.wWritePtr++] = code;
	SendFrameTail();
}

void Cdisp::ProcConnect(void)
{
    BYTE *pAck;
	pAck = (BYTE*)(m_pRec+1);
    if ((m_pRec->byCnt == m_byCnt)&&(*pAck == EXTMMI_ACK_OK))
    {
		m_bIn = TRUE;
		if (((m_pRec->byCtrl)&0x3f )== 2)
		{
		   m_disp_model = EXTDISP_MODEL_NODISP;
		   SendLightMd(0);//by lvyi 
		}
            else if(((m_pRec->byCtrl)&0x3f )== 4)
            {
                m_disp_model = EXTDISP_MODEL_LAMP;
		   SendLightMd(0);
            }
		else
		{		   
			SendLight(LedLight);
			m_disp_model = EXTDISP_MODEL_DISP;
		}
    }
    else
		evSend(MMI_ID, EV_INIT);	
}

void Cdisp::ProcYkMd(void)
{
    int no;
	VMsg msg;	
	VDBYK *pDBYK;
    BYTE *pNo = (BYTE*)(m_pRec+1);

	no = *pNo/2;

	if(yk_flag == 1)
	{
		if((m_pRec->byCtrl&EXTMMI_CTRL_Addr) == 0)
		{
		 	if (no >= m_ownEqpInfo[0].wYKNum)
		    SendConf(EXTMMI_TYPE_YKMD, EXTMMI_ACK_ERR,0);

   		    pDBYK=(VDBYK *)msg.abyData;
			pDBYK->wID = no+1;
			if ( *pNo % 2 )
       			pDBYK->byValue = 0x6; //分闸
			else
	   			pDBYK->byValue = 0x5; //合闸
	   	
			m_YKmsg.wID = pDBYK->wID;// by lvyi
			m_YKmsg.byValue = pDBYK->byValue&0xf;// by lvyi
    		TaskSendMsg(DB_ID, MMI_ID, m_wEqpID[0], MI_YKSELECT, MA_REQ, sizeof(VDBYK), &msg);//by lvyi

			SendConf(EXTMMI_TYPE_YKMD, EXTMMI_ACK_OK,0);
		}
		else
		{
			if (no >= m_ownEqpInfo[1].wYKNum)
			SendConf(EXTMMI_TYPE_YKMD, EXTMMI_ACK_ERR,1);

    		pDBYK=(VDBYK *)msg.abyData;
			pDBYK->wID = no+1;//遥控号，维护软件配的序号
			if ( *pNo % 2 )
       			pDBYK->byValue = 0x6; //分闸
			else
	   			pDBYK->byValue = 0x5; //合闸
	   	
			m_YKmsg.wID = pDBYK->wID;// by lvyi
			m_YKmsg.byValue = pDBYK->byValue&0xf;// by lvyi
    		TaskSendMsg(DB_ID, MMI_ID, m_wEqpID[1], MI_YKSELECT, MA_REQ, sizeof(VDBYK), &msg);//by lvyi

			SendConf(EXTMMI_TYPE_YKMD, EXTMMI_ACK_OK,1);
		}		
	}
	else
	{//防止地址0的面板预控没按下，而地址1的面板YK按下；
	//此时要发送错误帧，在面板程序里把temp_yk清掉
		if((m_pRec->byCtrl&EXTMMI_CTRL_Addr) == 0)
		{
				SendConf(EXTMMI_TYPE_YKMD, EXTMMI_ACK_ERR,0);
		}
		else
		{
				SendConf(EXTMMI_TYPE_YKMD, EXTMMI_ACK_ERR,1);
		}
		return;
	}
	
	
}

void Cdisp:: ProcYx(void)
{
     DWORD yx;
	 BYTE *pData = (BYTE*)(m_pRec+1);
	//by lvyi,先判地址,只处理地址为0的遥信,至于灯板,默认为0，兼容
	 if((m_pRec->byCtrl&EXTMMI_CTRL_Addr) == 0)
	 {
	 	
	 		yx = MAKEDWORD(MAKEWORD(*pData, *(pData+1)), MAKEWORD(*(pData+2), *(pData+3)));
             if (m_disp_model == EXTDISP_MODEL_LAMP)
            {    

                            if (yx & EXTMMI_YX_RESET)
                            	m_dwYx |= 0x01;
#if (DEV_SP == DEV_SP_DTU)
                                m_dwYx |= 0x08;
#endif
                     MMIYx = m_dwYx;
                    if (m_pRec->byCtrl & EXTMMI_CTRL_MASTER)
                              SendConf(EXTMMI_TYPE_YX, EXTMMI_ACK_OK,0);
            }
            else
		   {
	            MdFlag = 1;
	            if(yx & EXTMMI_YX_YuK)      //by lvyi,预控置位
	            {
	                yk_flag = 1;
	            }
	            else
	            {
	                yk_flag = 0;
	            }
				//FTU 及分合闸灯板处理本地远方
				if((DEV_SP != DEV_SP_DTU) || (m_disp_model == EXTDISP_MODEL_NODISP))
				{
					if (!(yx & EXTMMI_YX_LOCAL))
					{
						pannelYx |= 0x01;
						pannelYx &= ~0x02;
					}
					else
					{
						pannelYx &= ~0x01;
						pannelYx |= 0x02;
					}

					#if(TYPE_USER != USER_GUIYANG)
					if (yx & EXTMMI_YX_BS)
					{
						pannelYx &= ~0x3;
						pannelYx |= 0x04;
					}
					else
					#endif
					{
						pannelYx &= ~0x04;
					}
					
				}

				extSwitch = yx & 0xf;
				m_key=0;
				m_dwYx = 0;

				m_key=yx;

				if (yx & EXTMMI_YX_RESET)
					m_dwYx |= 0x01;

				if ((yx & EXTMMI_YX_YKSYN) && (yx & EXTMMI_YX_YKF))
					m_dwYx |= 0x02;

				if ((yx & EXTMMI_YX_YKSYN) && (yx & EXTMMI_YX_YKH))
					m_dwYx |= 0x04;

				if (yx & EXTMMI_YX_TRIPYB)
					m_dwYx |= 0x08;
#if (DEV_SP == DEV_SP_DTU)
				    m_dwYx |= 0x08;
#endif


				if (yx & EXTMMI_YX_FA)
					m_dwYx |= 0x100;
#if((TYPE_USER == USER_GUIYANG)&&(DEV_SP == DEV_SP_FTU))
				if (yx & EXTMMI_YX_BS)
					m_dwYx |= 0x200;
#endif
				m_dwYx |= ((yx<<1) & 0x3E00);

				MMIYx = m_dwYx;
#if(TYPE_USER == USER_MULTIMODE)
				pannelYx &= ~0x01;
				pannelYx |= 0x02;
				pannelYx &= ~0x04;
				MMIYx = (0x01<< EXTMMI_TRIP) | (0x01<<EXTMMI_FA);
#endif

				if (m_pRec->byCtrl & EXTMMI_CTRL_MASTER)
				  SendConf(EXTMMI_TYPE_YX, EXTMMI_ACK_OK,0);

#if (DEV_SP == DEV_SP_FTU) //FTU 调试串口为液晶通讯串口1  cjl   2016-9-12 09:57:00
				if ((yx & EXTMMI_YX_CHANGE) && (g_shell_serial.used == 0))
				{
					readComm(m_Rec.pBuf, PCOLCOMMBUFLEN, 0);
					m_Rec.wOldReadPtr = m_Rec.wReadPtr = m_Rec.wWritePtr = 0;
					g_CommChan[m_commid - COMM_START_NO].tid = SHELL_ID;
					g_shell_serial.used = 1;
					g_shell_serial.commno = m_commid;
					m_commid = -1;
				}
#endif
				if (m_disp_model == EXTDISP_MODEL_NODISP)
					return;
				if(UpdateTime<0)
				{
					dealreckey();
				}
					
		}
    }
}
void Cdisp::ProcProg(void)
{
	struct VFileMsgProM pFileMsgProM;
	BYTE *pData = (BYTE*)(m_pRec+1);
	pFileMsgProM.flag = pData[0];
	pFileMsgProM.addr = MAKEDWORD(MAKEWORD(pData[1], pData[2]), MAKEWORD(pData[3], pData[4]));
	pFileMsgProM.pLen = MAKEWORD(pData[5], pData[6]);
	pFileMsgProM.pMsgID = MI_ROUTE;
	msgSend(MAINT_ID, &pFileMsgProM,sizeof(VMsgHead), 1);
	if(MMI_FileLen == (pFileMsgProM.addr+pFileMsgProM.pLen))
	{//升级结束，打开其他报文交互
		UpdateTime = -1;
	}
		
}

int Cdisp::ProcFrm(void)
{   
  
    m_pRec = (VExtMMIFrmHead *)m_RecFrm.pBuf;
	
//by lvyi,只有在接收到地址为0的面板发来的数据时，才把计数清0
//这样面板0掉线就会重连，兼容老面板
	if((m_pRec->byCtrl&EXTMMI_CTRL_Addr) == 0)

	{
		commcunt=0;
	}
	
	
	if(((m_pRec->byCtrl&EXTMMI_CTRL_MASTER) == 0) && (m_byCnt != m_pRec->byCnt)) 
	{
	   logMsg("error%d, %d\n",m_byCnt,m_pRec->byCnt,0,0,0,0);
	   return 0;
	}
	sendflag = 1;

    switch (m_pRec->byType)
    {
		case EXTMMI_TYPE_CONNECT:
			ProcConnect();
		    break;
		case EXTMMI_TYPE_YX:
			ProcYx();
			break;
		case EXTMMI_TYPE_YKMD:
			ProcYkMd();
			break;
		case EXTMMI_TYPE_PROG_WRITE:
			ProcProg();
			break;
    }

	return 0;
}

Cdisp::winmenu* Cdisp::searchtopmanu(WORD TOPID)
{
	winmenu *p = &mainwinmenu;
	if (p->sunmenu == NULL) return NULL;
	p = p->sunmenu;
	while (p != &mainwinmenu)
	{
		if(p->ID == TOPID)
			return p;
		if(p->sunmenu != NULL)
			p = p->sunmenu;
		else if(p->flag == 0xe)
		{
			p = searchtopnextmanu(p);
		}
		else
			p = p->nextmenu;
	}
	return NULL;
}
Cdisp::winmenu* Cdisp::searchtopnextmanu(winmenu *p)
{
	p = p->topmenu;
	while (p != &mainwinmenu)
	{
		if(p->flag != 0xe)
			return p->nextmenu;
		else
			p = p->topmenu;
	}
	return &mainwinmenu;
}

char * Cdisp::delchar(char str[])
{
	int i,j;
	for(i = 0; str[i] != 0; i++ )
	{
		if(str[i] == '[' || str[i] == ']' || str[i] == '\n')
			for(j = i; str[j] != 0; j++)
				str[j] = str[j+1];

	}
	return str;

}

BYTE Cdisp::addmenu(void (Cdisp::*fun)(), WORD ID,WORD TOPID)
{
	int n=500;
	
	if(searchtopmanu(ID) != NULL) return false;
	
	winmenu *p = (winmenu*)malloc(sizeof(winmenu));
	if (p == NULL) 
		return false;
	winmenu *pp = searchtopmanu(TOPID);
	if (pp == NULL) 
	{
		pp = &mainwinmenu;
		pp->topmenu = &mainwinmenu;
	}
	p->flag = 0xe;
	p->fun = fun;
	p->sunmenu = NULL;
	p->ID = ID;
	
	if (pp->sunmenu == NULL)
	{
		pp->sunmenu = p;
		p->topmenu = pp;
		p->beforemenu = p;
		p->nextmenu = p;
		
		p->para = 1;
		return true;

	}
	pp = pp->sunmenu;
	while ((pp->flag != 0xe)&&(n-->5))
	{
		pp = pp->nextmenu;
	}
	pp->flag = 0;
	p->topmenu = pp->topmenu;
	p->beforemenu = pp;
	p->nextmenu = pp->nextmenu;
	pp->nextmenu->beforemenu = p;
	pp->nextmenu = p;
	p->para = pp->para+1;
	
	return true;
}
void Cdisp::delete_alarmmenu()
{
	winmenu *pp = &alarmwinmenu;
	winmenu *p = pp->sunmenu;
	BYTE i;
	for (i = 0; i < 32; i++)
	{
		alarmdispptr[i] = alarmdispptr[i+1];
	}
	
	p->flag = 0;
	 
	if (alarmdispptr[0] == 0xff)
	{
		pp->flag = 0;
		m_popup_flag = 0;
	}
								
	clearscreen();
}
BYTE Cdisp::addalarmmenu(void (Cdisp::*fun)(), WORD ID,WORD TOPID)
{
	winmenu *pp = &alarmwinmenu;
	winmenu *p = pp->sunmenu;
	m_popup_flag = 0x55;
	clearscreen();
	if ((ID == nomal_alarm) && (pp->page != 0x55))
	{
		p->fun = fun;
		p->sunmenu = NULL;
		p->ID = ID;
		p->page = 0x55;
		p->flag = 0;
	}
	else
	{
		p->page = 0;
		pp->fun = fun;
		pp->ID = ID;
		pp->flag = 0;
		pp->page = 0x55;
		WORD poolNum,wrtOffset;

		if (QuerySysEventInfo(g_Sys.pWarnEvent,1,0,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum)==0)
			m_Eventoff.WarnEventoff=0;
		else
		{
			while (wrtOffset != m_Eventoff.WarnEventoff)
			{
				addalarmnum(m_Eventoff.WarnEventoff);
				m_Eventoff.WarnEventoff++;
				if (m_Eventoff.WarnEventoff >= 32)
					m_Eventoff.WarnEventoff = 0;
			}
		}
		if(QuerySysEventInfo(g_Sys.pActEvent,1,0,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum) == 0)
			m_Eventoff.ActEventoff = 0;
		else
		{
			while (wrtOffset != m_Eventoff.ActEventoff)
			{
				addalarmnum(m_Eventoff.ActEventoff + 32);
				m_Eventoff.ActEventoff++;
				if(m_Eventoff.ActEventoff >= 32)
					m_Eventoff.ActEventoff = 0;
			}
		}
		if (QuerySysEventInfo(g_Sys.pDoEvent,1,0,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum) == 0)
			m_Eventoff.DoEventoff = 0;
		else
		{
			while (wrtOffset != m_Eventoff.DoEventoff)
			{
				addalarmnum(m_Eventoff.DoEventoff + 64);
				m_Eventoff.DoEventoff++;
				if(m_Eventoff.DoEventoff >= 32)
					m_Eventoff.DoEventoff = 0;
			}
		}


	}

	return true;
}

BYTE Cdisp::addturnmenu(void (Cdisp::*fun)(), WORD ID,WORD TOPID)
{
	winmenu *p;
	winmenu *pp = &turnwinmenu;
	if ((pp->beforemenu == NULL)||(pp->nextmenu == NULL))
	{
		pp->beforemenu = pp;
		pp->nextmenu = pp;
		pp->fun = fun;
		pp->ID = ID;
		return true;
	}
	p=new winmenu;
	p->fun = fun;
	p->ID = ID;
	pp->beforemenu->nextmenu = p;
	p->beforemenu = pp->beforemenu;
	pp->beforemenu = p;
	p->nextmenu = pp;
	return true;
}
void Cdisp::dealpopalarm()
{
	winmenu *pp = alarmwinmenu.sunmenu;
	if (pp->flag == 0x55)
	{
		if (pp->para != 0)
			pp->para--;
		else
		{
			delete_alarmmenu();
			m_popup_flag = 0;
		}

	}
	
}

void Cdisp::grun(DWORD evFlag)
{
	int num,flag;
	BYTE msgId;
	WORD key;

	if (m_disp_init == 0) return;
	
	tt = (char*)m_dwPubBuf;

	if (evFlag & EV_RX_AVAIL)
	{
		DoReceive(NULL, &flag);
	}
	
	if (evFlag & EV_SUSPEND)
	{
		thSuspend(m_wTaskID);
	}

	if (evFlag & EV_INIT)
	   SendConnect();

	if (evFlag & EV_MSG)
	{
		num = msgReceive(MMI_ID, (BYTE *)&DispMsg, MAXMSGSIZE, OS_NOWAIT);	//by lvyi "taskid->MMI_ID "
		if (num > 0)
		{
			  msgId = DispMsg.Head.byMsgID;//&(~MSGNOUFLAG);

			  switch (msgId)
			  {
			    case MI_YKSELECT:
			    case MI_YKOPRATE:
			    case MI_YKCANCEL:
			      procYKMsg(msgId); 
				  break;
				case MI_ROUTE:
				  DoTSDataMsg();
			    default:
			      break;
			  }    
		}
			 
	}
	if (evFlag & EV_TM1)
	{
		UpdateTime--;
		if(UpdateTime<0)
			UpdateTime=-1;
	}
	if(UpdateTime<0)
	{
		
		if (((m_disp_model == EXTDISP_MODEL_NODISP))||((m_disp_model == EXTDISP_MODEL_LAMP)))
		{ 
			if( (evFlag & EV_UFLAG) && (m_disp_model == EXTDISP_MODEL_LAMP))
			{
				SendLightMd(0);
			}
			
			if (evFlag & EV_TM1)
			{
	
				if(MdFlag == 1)
				{
					MdFlag = 0;
					SendLightMd(1); //by lvyi
					return;
				}
				if (!m_bIn && !(timecnt++ % 10))
				{
					evSend(MMI_ID, EV_INIT);
					return;
				}
				else if (!m_bIn)
					return;
	
				if (m_seccount++ > 3)// by lvyi "5->3"
				{
					
					SendLightMd(0);
					m_seccount=0;//by lvyi
					commcunt++;
					if ((commcunt >= EXTMMI_TIMEOUT) && m_bIn)
					{
						WriteWarnEvent(NULL, SYS_ERR_COMM, 0, "MMI板通讯异常");
						m_bIn = FALSE;
						commcunt = 0;
					}
					else
					{
						if(g_Sys.dwErrCode&SYS_ERR_COMM)
						SysClearErr(SYS_ERR_COMM);
					}
				}
					
			}
			return;
		}
		if (evFlag & EV_PROTECT_RESET)
		{
			addalarmmenu(&Cdisp::Malarm_event, important_alarm, important_alarm);
			if(m_popup_flag == 0x55)
			{
				dealkey(QUITKEY);
			}
		}
		if (evFlag & EV_UFLAG)
		{
			m_Eventoff.eventflag = 0x55;
			selftest_tm = 0;
			addalarmmenu(&Cdisp::Malarm_event, important_alarm, important_alarm);
			m_backlightflag = 0;
			m_backlighttm = 0;
			m_lamp_delay = 0;
			clearscreen();
			MASK=0;
		}

		if (evFlag & EV_TM1)
		{
			if (!m_bIn && !(timecnt++ % 10))
			{
				evSend(MMI_ID, EV_INIT);
				return;
			}
			else if (!m_bIn)
				return;

			if (m_seccount++ > 5)
				DoTimeOut(0);
			
			key = m_key >> 14;
			if (m_reflash_time++ == 50)
			{
				m_dispcmd = 1;
			}
			if (m_reflash_time == 200)
			{
				m_dispcmd = 1;
			}
			if (m_reflash_time > 600)
			{
				m_reflash_time = 0;
				m_dispcmd = 1;
			}
			m_keyval = 0;
			if ((key & 1))
				m_keyval = UPKEY;
			if (key & 2)
				m_keyval = DOWNKEY;
			if (key & 4)
				m_keyval = LEFTKEY;
			if (key & 8)
				m_keyval = RIGHTKEY;
			if (key & 0x10)
				m_keyval = ADDKEY;
			if (key & 0x20)
				m_keyval = SUBBKEY;
			if (key & 0x40)
				m_keyval = ENTERKEY;
			if(key & 0x80)
				m_keyval = QUITKEY;
		
			if (m_keybak != m_keyval)
			{	
				m_moniflag = 0;
				m_backlighttm = 0;
				m_backlightflag = 0;
				if (m_turndispflag < TURNTIME)
					m_turndispflag = 0;
				m_comm_num = 0;
				if (m_keyval)
				{
					m_dispcmd = 1;
					m_reflash_time = 0;
				}
				if (m_lamp_delay > closeTIME)
				{
					m_reflash_delay = 2;
				}
				m_lamp_delay = 0;
				if (m_reflash_delay != 0)
				{
					m_reflash_delay--;
					m_keybak = m_keyval;
					m_keyval = 0;
				}
				if ((m_keyval != m_keybak) && (m_keyval == SUBBKEY))
					ResetProtect(0);
				if (m_keyval & ((~MASK))) 
				{
					clearscreen();
					dealkey(m_keyval);
					m_keybak = m_keyval;
				}

			}
			m_disp_tm = 0;
			//if((++ligcunt) % 2 == 0)
			{	
				menucycle();
				//ligcunt=0;
			}			
			m_keybak = m_keyval;	
		}
	}	
}
void Cdisp::DoTimeOut(WORD wTimerID)
{
	DWORD tmp;

	commcunt++;
	if((commcunt % 5) == 0)
	{
		if(m_lamp_turn_light > 0)
		{
			tmp = m_lamp_turn_light;
		}
		else
		{
			tmp = LedLight;
		}
		
		m_lamp_state = tmp;
		SendLight(m_lamp_state);
	}
		
	
	if ((commcunt >= EXTMMI_TIMEOUT) && m_bIn)
	{
		WriteWarnEvent(NULL, SYS_ERR_COMM, 0, "MMI板通讯异常");
		SendLight(LedLight);//发送告警灯
		m_bIn = FALSE;
		commcunt = 0;
	}
	else
	{
	    if(g_Sys.dwErrCode&SYS_ERR_COMM)
		SysClearErr(SYS_ERR_COMM);
	}

	m_commtimeout++;
	m_seccount = 0;
	m_ykdelay++;
	m_backlighttm++;
	m_turndispflag++;
	GetSysClock(&clock, SYSCLOCK);

	if (m_yktesttime)
	{
		m_yktesttime--;
	}
	if(m_keyval == RIGHTKEY)
	{
		if(m_keylongdowntime > KEYLONGDOWN)
		{
			m_controlword_dispflag = 0x55;
		}
		else
		{
			m_keylongdowntime++;
			m_controlword_dispflag = 0;
		}
		
	}
	else
	{
		m_keylongdowntime = 0;
		m_controlword_dispflag = 0;
	}
		
		/*厂内测试用*/
	if (selftest_tm != 0)
		selftest_tm--;
	/*弹出告警*/
	dealpopalarm();
}

WORD Cdisp::Crc16(WORD SpecCode, BYTE *p, int l)
{
	int i;
	WORD index, crc=SpecCode;

	for(i=0; i<l; i++)
	{
		index = ((crc^p[i])&0x00FF);
		crc = ((crc>>8)&0x00FF)^crccode[index];
	}

	return(crc);
}  

BYTE Cdisp::zhengDispData1()
{	
	int i;
	SendFrameHead(DIS_TYPE_DATA, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = 0;
	m_Send.pBuf[m_Send.wWritePtr++] = 1;
	
	for(i = m_Send.wWritePtr; i < m_size + 9; i++)
		m_Send.pBuf[i] = m_dispbuf[i-9];

	m_Send.wWritePtr = i;

	SendFrameTail();
	return m_Send.wWritePtr;
}

BYTE Cdisp::zhengDispData2()
{	
	int i;
	SendFrameHead(DIS_TYPE_DATA, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = 0;
	m_Send.pBuf[m_Send.wWritePtr++] = 4;
	
	
	for(i = m_Send.wWritePtr; i < m_size + 9; i++)
		m_Send.pBuf[i] = m_dispbuf[i-9+m_size];

	m_Send.wWritePtr = i;
	SendFrameTail();

	return m_Send.wWritePtr;
}

BYTE Cdisp::zhengDispData3()
{	
	int i;
	SendFrameHead(DIS_TYPE_DATA, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = 0;
	m_Send.pBuf[m_Send.wWritePtr++] = 7;
	
	for(i = m_Send.wWritePtr; i < m_size + 9; i++)
		m_Send.pBuf[i] = m_dispbuf[i-9+m_size*2];

	m_Send.wWritePtr = i;
	SendFrameTail();

	return m_Send.wWritePtr;
}

BYTE Cdisp::zhengDispData4()
{	
	int i;
	SendFrameHead(DIS_TYPE_DATA, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = 0;
	m_Send.pBuf[m_Send.wWritePtr++] = 10;
	
	for(i = m_Send.wWritePtr; i < m_size + 9; i++)
		m_Send.pBuf[i] = m_dispbuf[i-9+m_size*3];

	m_Send.wWritePtr = i;
	SendFrameTail();

	return m_Send.wWritePtr;
}

BYTE Cdisp::zhengDispData5()
{	
	int i;
	SendFrameHead(DIS_TYPE_DATA, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = 0;
	m_Send.pBuf[m_Send.wWritePtr++] = 13;
	
	for(i = m_Send.wWritePtr; i < m_size + 9; i++)
		m_Send.pBuf[i] = m_dispbuf[i-9+m_size*4];

	m_Send.wWritePtr = i;
	SendFrameTail();

	return m_Send.wWritePtr;
}

BYTE Cdisp::fanDispData()
{
	int i;
	SendFrameHead(DIS_TYPE_GB, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = newmouse.y;
	
	if(newmouse.x == 1)
		m_Send.pBuf[m_Send.wWritePtr++] = newmouse.x + 3;
	else if(newmouse.x == 2)
		m_Send.pBuf[m_Send.wWritePtr++] = newmouse.x + 5;
	else if(newmouse.x == 3)
		m_Send.pBuf[m_Send.wWritePtr++] = newmouse.x + 7;
	else if(newmouse.x == 4)
		m_Send.pBuf[m_Send.wWritePtr++] = newmouse.x + 9;
	else if(newmouse.x == 0)
		m_Send.pBuf[m_Send.wWritePtr++] = newmouse.x + 1;

	m_Send.pBuf[m_Send.wWritePtr++] = newmouse.num;
	
	for(i = m_Send.wWritePtr; i < newmouse.num + 10; i++)
		m_Send.pBuf[i] = m_dispbuf[i-10+newmouse.y+((newmouse.x)*m_size)];

	m_Send.wWritePtr = i;
	SendFrameTail();
	
	return m_Send.wWritePtr;
}

BYTE Cdisp::TxdBlacklight()
{
	SendFrameHead(DIS_TYPE_BLKLIGHT, EXTMMI_FUN_WRITE, 1);
	m_Send.pBuf[m_Send.wWritePtr++] = m_backlightflag;
	SendFrameTail();

	return m_Send.wWritePtr;
}

void Cdisp::WriteDispBuff(char *psrc, BYTE x, BYTE y, BYTE len)
{
	BYTE  i = y + (x * m_size);
	BYTE* psrc1 = (BYTE*)psrc;
	while(*psrc1 != 0)
	{
		m_dispbuf[i] = *psrc1;
		psrc1++;
		i++;
	}
}

void Cdisp::dispclock()
{
	#define off 0
	BYTE charaddr[][2] = {
		{1,0+off},
		{1,1+off},			
		{1,2+off},			
		{1,3+off},			
		{1,5+off},		
		{1,6+off},
		{1,8+off},			
		{1,9+off},			
		{2,2+off},
		{2,3+off},
		{2,5+off},
		{2,6+off},
		{2,8+off},
		{2,9+off}
	};

	m_wrrdfalg = 1;
	MASK = 0x4f;
	
	GetSysClock(&clock, SYSCLOCK);
	m_setkeyvalue = 0;
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	WriteDispBuff(tt,charaddr[8][0],charaddr[8][1],15);
	m_setkeyvalue = 0;
		
	if(m_setkeyvalue > 200) m_setkeyvalue = (sizeof(charaddr)/sizeof(charaddr[0]))-1;
	else if(m_setkeyvalue >= (sizeof(charaddr)/sizeof(charaddr[0])))
		m_setkeyvalue = 0;

	newmouse.x = charaddr[m_setkeyvalue][0];
	newmouse.y = charaddr[m_setkeyvalue][1];
	newmouse.num = 1;
	
	m_keybak = m_keyval;

}


void Cdisp::MainMenu(void)
{
	char row1[]="主 菜 单";
	char row2[]="实时数据";
	char row3[]="定值管理";
	char row4[]="上传数据";
	char row5[]="采集数据";
	char row6[]="终端信息";
	char row7[]="终端配置";
	char row8[]="事件记录";
	char row9[]="出厂调试";
	
	WriteDispBuff(row1,0,10,16);
	
	if(pwinmenu->para < 10)
	{
		
		
		WriteDispBuff(row2,1,3,16);
		WriteDispBuff(row3,2,3,16);
		WriteDispBuff(row4,3,3,16);
		WriteDispBuff(row5,4,3,16);
		
		WriteDispBuff(row6,1,17,16);
		WriteDispBuff(row7,2,17,16);
		WriteDispBuff(row8,3,17,16);
		WriteDispBuff(row9,4,17,16);
		
	}
	
	if(pwinmenu->para < 5)
	{
		newmouse.y = 3;
	}
	else if(pwinmenu->para < 10)
	{
		newmouse.y = 17;
	}
	
	m_curmenu = 0;
	
	if(m_curvalue[0] == 0)
		m_curvalue[0] = 1;
	
	newmouse.x = (pwinmenu->para - 1) % 4 + 1;
	
	newmouse.num = 8;
	m_eqpdot = 0;
	MASK = 0;
	m_pEqpInfo = g_Sys.Eqp.pInfo;
	m_pgroupeqp[0] = NULL;
	m_pcurdispeqp[0] = NULL;
	m_keyupdownval = 0;
	memset(m_mousevalue,0,10);


}


void Cdisp::M9_Setting_management(void)
{
	char row1[]="(2)定值管理";
	char row2[]="1.定值查看";
	char row3[]="2.定值设置";
	char row4[]="3.回线配置";
	WriteDispBuff(row1,0,0,16);
	if(pwinmenu->para < 4)
	{
		WriteDispBuff(row2,1,0,16);
		WriteDispBuff(row3,2,0,16);
		WriteDispBuff(row4,3,0,16);
	}
	newmouse.y = 0;
	m_curmenu = 0;
	
	if(m_curvalue[0] == 0)
		m_curvalue[0] = 1;
	
	newmouse.x = (pwinmenu->para - 1) % 3 + 1;
	newmouse.num = 10;
	m_eqpdot = 0;
	MASK = 0;
	m_pEqpInfo = g_Sys.Eqp.pInfo;
	m_pgroupeqp[0] = NULL;
	m_pcurdispeqp[0] = NULL;
	m_keyupdownval = 0;
	memset(m_mousevalue,0,10);
}

void Cdisp::menucycle()
{
	if(m_popup_flag == 0x55)
	{
		MASK = 0;
		if(alarmwinmenu.page == 0x55)
			(this->*(alarmwinmenu).fun)();
		else if(alarmwinmenu.sunmenu->page == 0x55)
			(this->*(alarmwinmenu.sunmenu)->fun)();
		else
			m_popup_flag = 0;
		//m_turndispflag = 0;
	}
	else if((m_turndispflag >= TURNTIME))
	{
		m_turndispflag = TURNTIME;
		if((pturnmenu->flag != 0x55) && (pturnmenu->flag != 0x66))
		{
			pturnmenu->flag = 0x55;
			pturnmenu->row = 0;
			clearscreen();
		}		
		if(m_turnflag == 0)
		{
			m_turnflag = 1;
			m_turnstop = 0;
			pturnmenu = &turnwinmenu;
			pturnmenu->flag = 0x55;
	 	}
	 	if(m_turnstop == 0)
		{
			pturnmenu->row++;
			if(pturnmenu->row > TURNnextTIME*10)
			{
				pturnmenu->row = 0;
				pturnmenu->page++;
			}
		}
		MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ADDKEY|SUBBKEY|QUITKEY|ENTERKEY|FUNKEY;
		(this->*pturnmenu->fun)();
		if(m_keybak != m_keyval)
		{
			pwinmenu = mainwinmenu.sunmenu;
			pwinmenu->flag = 0;
			m_turndispflag = 0;
			m_turnflag = 0;
			pturnmenu->flag = 0;
			clearscreen();
			MASK = 0;
		}
	}	
	else if(pwinmenu != NULL)
	{
		if((pwinmenu->flag != 0x55) && (pwinmenu->flag != 0x66))
		{
			pwinmenu->flag = 0x55;
			pwinmenu->row = 0;
			clearscreen();
		}
		(this->*pwinmenu->fun)();
	}
	
	if((m_backlighttm >= TURNTIME))
	{
		if(m_backlighttm == TURNTIME)
			if(m_dispcmd > 7)
				m_dispcmd = 7;
		m_backlightflag = 1;
		m_backlighttm = 0;
	}

	if(m_popup_flag)
	{
		Comparebuf();
		if(m_keybak == m_keyval)
			sendtopopscreen();
		m_comm_num = 0;
	}
	else 
	{
		Comparebuf();
		if(m_keybak == m_keyval)
			sendtoscreen();
	}

}
void Cdisp::dealreckey()
{
	if(m_keybak != m_keyval)
	{	
		m_backlighttm = 0;
		m_comm_num = 0;
		
		if(m_lamp_delay > closeTIME)
		{
			m_reflash_delay = 2;
		}
		m_lamp_delay = 0;
		
		if(m_reflash_delay != 0)
		{
			m_reflash_delay--;
			m_keybak = m_keyval;
			m_keyval = 0;
		}
		
		if((m_keyval != m_keybak) && (m_keyval == SUBBKEY))
			ResetProtect(0);
		if(m_keyval & ((~MASK))) 
		{
			dealkey(m_keyval);
			m_keybak = m_keyval;
		}

	}
	menucycle();
	m_keybak = m_keyval;
}


void Cdisp::dealkey(WORD keyvalue)
{	
	if(m_popup_flag == 0x55)
	{
		if(keyvalue == QUITKEY)
		{
			clearscreen();
			while(alarmdispptr[0] != 0xff)
				delete_alarmmenu();
		}
		return;
	}
	
	if(keyvalue != ENTERKEY)
		pwinmenu->flag = 0;
	switch(keyvalue)
	{
		case UPKEY:  
			pwinmenu = pwinmenu->beforemenu;
			clearscreen();
			break;
		case DOWNKEY:  
			pwinmenu = pwinmenu->nextmenu;
			pwinmenu->flag = 0;
			clearscreen();
			break;
		case LEFTKEY: 
			pwinmenu = pwinmenu->beforemenu;
			pwinmenu = pwinmenu->beforemenu;
			pwinmenu = pwinmenu->beforemenu;
			pwinmenu = pwinmenu->beforemenu;
			clearscreen();
			break;
		case RIGHTKEY:  
			pwinmenu = pwinmenu->nextmenu;
			pwinmenu = pwinmenu->nextmenu;
			pwinmenu = pwinmenu->nextmenu;
			pwinmenu = pwinmenu->nextmenu;
			clearscreen();
			break;
		case QUITKEY:  
			pwinmenu->flag = 0;
			if(pwinmenu->topmenu != &mainwinmenu)
				pwinmenu = pwinmenu->topmenu;
			else if(pwinmenu == mainwinmenu.sunmenu)
			{
				pturnmenu = &turnwinmenu;
				turnwinmenu.flag = 0;
				m_turndispflag = TURNTIME;
			}
			else
			{
				pwinmenu = mainwinmenu.sunmenu;
			}

			clearscreen();
			break;
		case ENTERKEY:
			if(pwinmenu->sunmenu != NULL)
			{
				pwinmenu = pwinmenu->sunmenu;
				pwinmenu->flag = 0;
			}
			else
			{
				pwinmenu = pwinmenu->topmenu;	
			}
			clearscreen();
			break;
		case FUNKEY:
			pwinmenu->flag = 0;
			while(pwinmenu->topmenu != &mainwinmenu)
				pwinmenu = pwinmenu->topmenu;
			pwinmenu = pwinmenu->nextmenu;
			pwinmenu->flag = 0;
			clearscreen();
			break;
		default:
			break;
	}
	pwinmenu->key = keyvalue;

}


BYTE Cdisp::Comparebuf()
{
	int i;
	for(i = 0;  i< 5 * 31; i++)
	{
		if(m_dispbuf[i] != m_bakdispbuf[i])
		{
			memcpy(m_bakdispbuf, m_dispbuf, 5*31);
			if(m_dispcmd > 7)
				m_dispcmd = 1;
			return 1;
		}
	}
	
	return 0;
}

void Cdisp::M111_meterage()
{
#ifdef INCLUDE_YC
	int i;
	VMmiMeaValue mmiVal[4];
	newmouse.num = 0;
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		WriteDispBuff("无回线", 2, 2, 18);
		return;
	}
	dealleftrightvalue(g_Sys.MyCfg.wFDNum);
	meaRead_Mmi(m_keyleftrightval, 0, 4, mmiVal);
	sprintf((char*)tt,"%s  <>选回线",delchar(g_Sys.MyCfg.pFd[m_keyleftrightval].kgname));
	WriteDispBuff(tt, 0, 0, 16);
	for(i = 0; i < maxdatarow; i++)
	{
		if(!mmiVal[i].valid) continue;
		if(mmiVal[i].tbl->unit[0] == 'A')
			sprintf((char*)tt,"%s=%5.3f%s %4.0f°    ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);
		else if(mmiVal[i].tbl->unit[0] == 'H')
			sprintf((char*)tt,"%s=%5.1f%s %4.0f°   ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);		
		else if(strcmp(mmiVal[i].tbl->unit,"kΩ") == 0)
		{
			if(mmiVal[i].rms < 0)
				sprintf((char*)tt,"%s= ∞  %s          ",mmiVal[i].tbl->name, mmiVal[i].tbl->unit);		
			else
				sprintf((char*)tt,"%s=%5.1f%s         ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit);		
		}
		else
			sprintf((char*)tt,"%s=%5.2f%s %4.0f°    ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);

		WriteDispBuff(tt, i+1, 0, 16);
	
	}
#else
	  WriteDispBuff("无回线", 2, 2, 18);
#endif
}
void Cdisp::M112_meterage()
{
#ifdef INCLUDE_YC
	int i;
	VMmiMeaValue mmiVal[4];
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		WriteDispBuff("无回线", 2, 2, 18);
		return;
	}	
	dealleftrightvalue(g_Sys.MyCfg.wFDNum);
	meaRead_Mmi(m_keyleftrightval, 4, 4, mmiVal);
	sprintf((char*)tt,"%s  <>选回线",delchar(g_Sys.MyCfg.pFd[m_keyleftrightval].kgname));
	WriteDispBuff(tt, 0, 0, 16);
	for(i = 0; i < maxdatarow; i++)
	{

		if(!mmiVal[i].valid) continue;

		if(mmiVal[i].tbl->unit[0] == 'A')
			sprintf((char*)tt,"%s=%5.3f%s %4.0f°   ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);		
		else if(mmiVal[i].tbl->unit[0] == 'H')
			sprintf((char*)tt,"%s=%5.1f%s %4.0f°   ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);		
		else if(strcmp(mmiVal[i].tbl->unit,"kΩ")==0)
		{
			if(mmiVal[i].rms < 0)
				sprintf((char*)tt,"%s= ∞  %s            ",mmiVal[i].tbl->name, mmiVal[i].tbl->unit);		
			else
				sprintf((char*)tt,"%s=%5.1f%s           ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit);		
		}
		else
			sprintf((char*)tt,"%s=%5.2f%s %4.0f°   ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);
		WriteDispBuff(tt, i+1, 0, 16);
	}
	newmouse.num = 0;
#else
	
	WriteDispBuff("无回线", 2, 2, 18);

#endif

}

void Cdisp::M113_meterage()
{
#ifdef INCLUDE_YC
	int i;
	VMmiMeaValue mmiVal[4];
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		WriteDispBuff("无回线",2,2,18);
		return;
	}	
	dealleftrightvalue(g_Sys.MyCfg.wFDNum);
	meaRead_Mmi(m_keyleftrightval,  8, 4, mmiVal);
	sprintf((char*)tt,"%s  <>选回线",delchar(g_Sys.MyCfg.pFd[m_keyleftrightval].kgname));
	WriteDispBuff(tt, 0, 0, 16);
	for(i = 0; i < maxdatarow; i++)
	{

		if(!mmiVal[i].valid) continue;
	
		if(mmiVal[i].tbl->unit[0] == 'A')
			sprintf((char*)tt,"%s=%5.3f%s %4.0f°   ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);
		else if(mmiVal[i].tbl->unit[0] == 'H')
			sprintf((char*)tt,"%s=%5.1f%s %4.0f°   ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);		
		else if(strcmp(mmiVal[i].tbl->unit,"kΩ")==0)
		{
			if(mmiVal[i].rms < 0)
				sprintf((char*)tt,"%s= ∞  %s          ",mmiVal[i].tbl->name, mmiVal[i].tbl->unit);		
			else
				sprintf((char*)tt,"%s=%5.1f%s         ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit);		
		}
		else
			sprintf((char*)tt,"%s=%5.2f%s %4.0f°   ",mmiVal[i].tbl->name, mmiVal[i].rms,mmiVal[i].tbl->unit,mmiVal[i].ang);
		WriteDispBuff(tt,i+1,0,16);
	}
	newmouse.num = 0;
#else
	
	WriteDispBuff("无回线", 2, 2, 18);

#endif

}


void Cdisp::M211_YC()
{
	int i;
	short buf[5];
	int val;
	BYTE n;
	struct VTrueYCCfg *pYCCfg = NULL;
	struct VVirtualYC   *pVYCCfg = NULL;
	VYcCfg *pYCTYPE = NULL;
	
	if(m_pEqpInfo->pTEqpInfo != NULL)
	{
		pYCCfg = m_pEqpInfo->pTEqpInfo->pYCCfg;
		dealupdownvalue((int)m_pEqpInfo->pTEqpInfo->Cfg.wYCNum, maxdatarow);
	}
	else if(m_pEqpInfo->pVEqpInfo != NULL)
	{
		pVYCCfg = m_pEqpInfo->pVEqpInfo->pYC;
		dealupdownvalue((int)m_pEqpInfo->pVEqpInfo->Cfg.wYCNum, maxdatarow);
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
	}
	sprintf((char*)tt,"设备:%s                        ",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	
	ReadRangeAllYC(0,m_keyupdownval*maxdatarow,maxdatarow,maxdatarow*sizeof(short),buf);
	pYCTYPE = g_Sys.MyCfg.pYc;
	
	n=ReadRangeAllYC(m_pEqpInfo->wID,m_keyupdownval*maxdatarow,maxdatarow,maxdatarow*sizeof(short),buf);
	
	i = 0;
	while(i<m_keyupdownval * maxdatarow)
	{
		if(pYCCfg)
			pYCCfg++;
		if(pVYCCfg)
			pVYCCfg++;
		pYCTYPE++;
		i++;
	}
	for(i = 0; i < n; i++)
	{
		if(pYCCfg)
		{
			sprintf((char*)tt,"%s                  %s",delchar(pYCCfg->sNameTab), unit[pYCTYPE->type]);
			WriteDispBuff(tt,i+1,0,10);
			val =(int)( buf[i]*pYCCfg->lA/pYCCfg->lB+pYCCfg->lC);
			sprintf((char*)tt,"%d  ",val);
			WriteDispBuff(tt,i+1,11,6);
			pYCCfg++;
			pYCTYPE++;
	
		}
		if(pVYCCfg)
		{
			sprintf((char*)tt,"%s                   %s",delchar(pVYCCfg->sNameTab), unit[pYCTYPE->type]);
			WriteDispBuff(tt,i+1,0,10);
			val =(float)( buf[i]*pYCCfg->lA/pYCCfg->lB+pYCCfg->lC);
			sprintf((char*)tt,"%d  ",val);
			WriteDispBuff(tt,i+1,11,6);
			pVYCCfg++;
			pYCTYPE++;
			
		}
		
	}
	newmouse.num = 0;

}


void Cdisp::M_YC()
{
	int i;
	short buf[5];
	static float val;
	BYTE n;
	struct VTrueYCCfg *pYCCfg = NULL;
	struct VVirtualYC   *pVYCCfg = NULL;
	VYcCfg *pYCTYPE = NULL;
	
	if(m_pEqpInfo->pTEqpInfo != NULL)
	{
		pYCCfg = m_pEqpInfo->pTEqpInfo->pYCCfg;
		dealupdownvalue((int)m_pEqpInfo->pTEqpInfo->Cfg.wYCNum, maxdatarow);
	}
	else if(m_pEqpInfo->pVEqpInfo != NULL)
	{
		pVYCCfg = m_pEqpInfo->pVEqpInfo->pYC;
		dealupdownvalue((int)m_pEqpInfo->pVEqpInfo->Cfg.wYCNum, maxdatarow);
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
	}
	sprintf((char*)tt,"设备:%s                        ",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	
	ReadRangeAllYC(0,m_keyupdownval*maxdatarow,maxdatarow,maxdatarow*sizeof(short),buf);
	pYCTYPE = g_Sys.MyCfg.pYc;
	
	n = ReadRangeAllYC(m_pEqpInfo->wID,m_keyupdownval*maxdatarow,maxdatarow,maxdatarow*sizeof(short),buf);
	
	i = 0;
	while(i < m_keyupdownval * maxdatarow)
	{
		if(pYCCfg)
			pYCCfg++;
		if(pVYCCfg)
			pVYCCfg++;
		pYCTYPE++;
		i++;
	}
	for(i = 0; i < n; i++)
	{
		if(pYCCfg)
		{
			sprintf((char*)tt,"%s       ",delchar(pYCCfg->sNameTab));
			WriteDispBuff(tt,i+1,0,10);
			val = (float)( buf[i]*pYCCfg->lA/pYCCfg->lB+pYCCfg->lC);
			sprintf((char*)tt,"%7.2f  ",val);
			WriteDispBuff(tt,i+1,10,6);
			pYCCfg++;
			pYCTYPE++;
	
		}
		if(pVYCCfg)
		{
			sprintf((char*)tt,"%s        ",delchar(pVYCCfg->sNameTab));
			
			WriteDispBuff(tt,i+1,0,10);
			val = (float)( buf[i]*pVYCCfg->lA/pVYCCfg->lB+pVYCCfg->lC);
			sprintf((char*)tt,"%7.2f  ",val);
			WriteDispBuff(tt,i+1,10,6);
			pVYCCfg++;
			pYCTYPE++;
			
		}
	}
	newmouse.num = 0;

}


void Cdisp::M212_YX()
{
	int i;
	BYTE buf[4];
	BYTE n;
	struct VTrueYXCfg *pSYXCfg = NULL;
	struct VVirtualYX *pVYXCfg = NULL;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
	}
	if(m_pEqpInfo->pTEqpInfo != NULL)
	{
		pSYXCfg = m_pEqpInfo->pTEqpInfo->pSYXCfg;
		dealupdownvalue((int)m_pEqpInfo->pTEqpInfo->Cfg.wVYXNum+m_pEqpInfo->pTEqpInfo->Cfg.wSYXNum,maxdatarow);
	}
	else if(m_pEqpInfo->pVEqpInfo != NULL)
	{
		pVYXCfg=m_pEqpInfo->pVEqpInfo->pSYX;
		dealupdownvalue((int)m_pEqpInfo->pVEqpInfo->Cfg.wSYXNum,maxdatarow);
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
	}
	sprintf((char*)tt,"设备:%s                        ",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	
	n = ReadRangeAllSYX(m_pEqpInfo->wID,m_keyupdownval*maxdatarow,maxdatarow,maxdatarow*sizeof(BYTE),buf);
	i = 0;
	while(i < m_keyupdownval * maxdatarow)
	{
		if(pSYXCfg)
			pSYXCfg++;
		if(pVYXCfg)
			pVYXCfg++;
		i++;
	}
	for(i = 0; i < n; i++)
	{
		if(pSYXCfg)
		{
			sprintf((char*)tt,"%s                   ",delchar(pSYXCfg->sNameTab));
			pSYXCfg++;
		}
		if(pVYXCfg)
		{
			sprintf((char*)tt,"%s                   ",delchar(pVYXCfg->sNameTab));
			pVYXCfg++;
		}
		WriteDispBuff(tt,i+1,0,14);
		if(buf[i] & 0x80)
			WriteDispBuff("  合", i+1, 16, 2);
		else
			WriteDispBuff("  分", i+1, 16, 2);	
	}
	newmouse.num = 0;
}

void Cdisp::M213_SOE()
{
	
	WORD poolNum,wrtOffset,curPtr;
	VDBSOE pValue;
	VSysClock tm;
	MASK  = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ENTERKEY;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
	}
	if (QuerySSOEWrtOffset((WORD)m_pEqpInfo->wID,1,&curPtr) == ERROR) 
	{
		WriteDispBuff("无SOE记录",2,8,10);
		return;
	}
	if(QuerySSOE((WORD)m_pEqpInfo->wID,1,curPtr,MAXDATASIZE,&pValue,&wrtOffset,&poolNum))
	{
		if(m_soeptr != wrtOffset)
		{
			m_soeptr = wrtOffset;
		}
		if(m_keybak != m_keyval)
		{
			if(m_keyval)
				clearscreen();
			switch(m_keyval)
			{
				case 2://down
					m_keyupdownval++;
					if(m_keyupdownval >= poolNum)
						m_keyupdownval = 0;  
					break;
				case 1://up
					if(m_keyupdownval <= 0)
						m_keyupdownval = poolNum;
					m_keyupdownval--;
					break;
				case ENTERKEY:
					Returntopmenu();
					return;
			}
		}
			
	while(QuerySSOE((WORD)m_pEqpInfo->wID,1,(poolNum+curPtr-m_keyupdownval)%poolNum,MAXDATASIZE,&pValue,&wrtOffset,&poolNum)==0)
	{
		m_keyupdownval++;
	}
	m_keyupdownval = m_keyupdownval % poolNum;
	
	sprintf((char*)tt,"设备:%s", m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	if(m_pEqpInfo->pTEqpInfo != NULL)
	{
		sprintf((char*)tt,"遥信点:%s               ",delchar(m_pEqpInfo->pTEqpInfo->pSYXCfg[pValue.wNo].sNameTab));
	}
	else if(m_pEqpInfo->pVEqpInfo != NULL)
	{
		sprintf((char*)tt,"遥信点:%s               ",delchar(m_pEqpInfo->pVEqpInfo->pSYX[pValue.wNo].sNameTab));
	}
	
	WriteDispBuff(tt,1,0,16);
	if(pValue.byValue == 0x81)
		sprintf((char*)tt," 分->合    %d/%d",(m_keyupdownval)%poolNum+1,poolNum);
	else
		sprintf((char*)tt," 合->分    %d/%d",(m_keyupdownval)%poolNum+1,poolNum);
	WriteDispBuff(tt,2,0,16);
	CalClockTo(&pValue.Time,&tm);  
	
	sprintf((char*)tt,"%04d-%02d-%02d %02d:%02d:%02d:%03d",tm.wYear,tm.byMonth,tm.byDay, tm.byHour,tm.byMinute,tm.bySecond,tm.wMSecond);	
	WriteDispBuff(tt,3,0,16);

	}
	else
		WriteDispBuff("无SOE记录",2,8,10);

	newmouse.num=0;
}
void Cdisp::M410_Eventnr()
{
	char buff[26];
	char row1[]="                         ";
	BYTE i = pwinmenu->topmenu->row%maxdatarow;
	VSysClock tm;
	MASK = DOWNKEY|UPKEY|LEFTKEY|RIGHTKEY;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->topmenu->page = 1;
		m_setmouse.num = 1;
		pwinmenu->row = pwinmenu->para;
		m_setkeyvalue = 0;
		pwinmenu->page = 0;
	}
	changestring(delchar(m_Eventinfo[i].msg),(char*)tt);
	if(m_setkeyvalue == 0)
	{
		if(strlen((char*)tt) > 26)
		{
			memcpy(buff, delchar(tt), 26);
			WriteDispBuff(buff,1,0,26);
			memset(buff, 0, 26);
			memcpy(buff, tt+26, 26);
			WriteDispBuff(row1,2,0,26);
			WriteDispBuff(buff,2,0,26);
			WriteDispBuff(row1,3,0,26);
		
		}
		else
		{
			WriteDispBuff((char*)delchar(tt),1,0,strlen((char*)tt));
			WriteDispBuff(row1,2,0,16);
		}
		
		//WriteDispBuff((char*)delchar(tt),1,0,16*2);
		CalClockTo(&m_Eventinfo[i].time,&tm);  
		sprintf((char*)tt,"%04d-%02d-%02d",tm.wYear, tm.byMonth,tm.byDay);
		WriteDispBuff(tt,0,0,16);	
		sprintf((char*)tt,"%02d:%02d:%02d:%03d",tm.byHour,tm.byMinute,tm.bySecond,tm.wMSecond);
		WriteDispBuff(tt,0,12,16);	
	}
	else if(((m_setkeyvalue-1)*64+32) > strlen(tt))
		m_setkeyvalue = 0;
	else
		WriteDispBuff((char*)tt+32+(m_setkeyvalue-1)*64,0,0,16*4);

	newmouse.num=0;
	
	if(m_keyval != m_keybak)
	{
		switch(m_keyval)
		{
			case UPKEY:
			case DOWNKEY:
			case LEFTKEY:
			case RIGHTKEY:
				//m_setkeyvalue++;
				//clearscreen();
				break;
		}
	}
}
void Cdisp::M410_ActEvent()
{
	
	WORD poolNum,wrtOffset,curPtr;
	WORD curpage;
	VSysClock tm;
	int i;
	MASK = 0x4f;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
	}
	curpage = (pwinmenu->row/maxdatarow)*maxdatarow;
					
	if(QuerySysEventInfo(g_Sys.pActEvent,1,0,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum)==0)
	{
		WriteDispBuff("无动作记录",2,8,15);
		newmouse.num = 0;
		return;
	}
	sprintf((char*)tt,"动作记录%d/%d",pwinmenu->row + 1,poolNum);
	WriteDispBuff(tt,0,0,15);
	for(i = 0; i < maxdatarow; i++)
	{
		if((wrtOffset) >= curpage + i)
			curPtr = wrtOffset - (curpage + i);
		else
			curPtr = poolNum + wrtOffset - (curpage + i);
		curPtr--;	
		curPtr %= poolNum;

		if(curpage+i>=poolNum)
		{
			WriteDispBuff("返回",i+1,5,16);	
			m_Eventinfo[i].time.dwMinute = 0;
			break;
		}
		
		if(QuerySysEventInfo(g_Sys.pActEvent,1,curPtr,sizeof(m_Eventinfo[0]),m_Eventinfo+i,&wrtOffset,&poolNum))
		{
			CalClockTo(&m_Eventinfo[i].time,&tm);  
			sprintf((char*)tt,"%02d-%02d %02d:%02d:%02d:%03d",tm.byMonth, tm.byDay,tm.byHour,tm.byMinute,tm.bySecond,tm.wMSecond);
			WriteDispBuff(tt,i+1,0,16);	
		}
		else if(curpage+i == wrtOffset)
		{
  			WriteDispBuff("返回",i+1,5,16);	
			m_Eventinfo[i].time.dwMinute = 0;
			if(pwinmenu->row > wrtOffset)
				pwinmenu->row = 0;
			break;
		}
		else
			pwinmenu->row = 0;
	}
	M4_key(poolNum);
	newmouse.x = (pwinmenu->row % maxdatarow) + 1;
	newmouse.y = 0;
	newmouse.num = 18;
}
void Cdisp::M410_AlarmEvent()
{
	
	WORD poolNum,wrtOffset,curPtr;
	WORD curpage;
	VSysClock tm;
	int i;
	MASK = 0x4f;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
	}
	curpage = (pwinmenu->row / maxdatarow) * maxdatarow;
	if(QuerySysEventInfo(g_Sys.pWarnEvent,1,0,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum)==0)
	{
		WriteDispBuff("无告警记录",2,8,15);
		newmouse.num = 0;
		return;
	}
					
	sprintf((char*)tt,"告警记录%d/%d   ",pwinmenu->row+1,poolNum);
	WriteDispBuff(tt,0,0,15);
	for(i = 0; i < maxdatarow; i++)
	{
		if((wrtOffset) >= curpage + i)
			curPtr = wrtOffset - (curpage + i);
		else
			curPtr = poolNum + wrtOffset-( curpage + i);
		curPtr--;	
		curPtr %= poolNum;
		if(curpage+i >= poolNum)
		{
			WriteDispBuff("返回",i+1,5,16);	
			m_Eventinfo[i].time.dwMinute = 0;
			break;
		}
		if(QuerySysEventInfo(g_Sys.pWarnEvent,1,curPtr,sizeof(m_Eventinfo[0]),m_Eventinfo+i,&wrtOffset,&poolNum))
		{
			CalClockTo(&m_Eventinfo[i].time,&tm);  
			sprintf((char*)tt,"%02d-%02d %02d:%02d:%02d:%03d",tm.byMonth, tm.byDay,tm.byHour,tm.byMinute,tm.bySecond,tm.wMSecond);
			WriteDispBuff(tt,i+1,0,16);	
		}
		else if(curpage+i == wrtOffset)
		{
  			WriteDispBuff("返回",i+1,5,16);	
			m_Eventinfo[i].time.dwMinute = 0;
			if(pwinmenu->row > wrtOffset)
				pwinmenu->row = 0;
			break;
		}
		else
			pwinmenu->row = 0;
		
	}
	M4_key(poolNum);
	newmouse.x = (pwinmenu->row % maxdatarow) + 1;
	newmouse.y = 0;
	newmouse.num = 18;
}
void Cdisp::M410_clearEvent()
{
	MASK = 0x0;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
		ClearSysEvent(g_Sys.pActEvent, SYSEV_FLAG_ACT);
	}
	WriteDispBuff("                             ",0,0,16);
	WriteDispBuff("      动作清除成功",1,0,16);
	WriteDispBuff("                           ",2,0,16);
	WriteDispBuff("       按取消退出",3,0,16);
	newmouse.num = 0;
}
void Cdisp::M411_clearEvent()
{
	MASK = 0x0;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
		ClearSysEvent(g_Sys.pDoEvent, SYSEV_FLAG_DO);
	}
	WriteDispBuff("                             ",0,0,16);
	WriteDispBuff("       操作清除成功 ",1,0,16);
	WriteDispBuff("                             ",2,0,16);
	WriteDispBuff("        按取消退出                          ",3,0,16);
	newmouse.num=0;
}
void Cdisp::M412_clearEvent()
{
	MASK =  0x0;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
		ClearSysEvent(g_Sys.pWarnEvent, SYSEV_FLAG_WARN);
	}
	WriteDispBuff("                             ",0,0,16);
	WriteDispBuff("       告警清除成功",1,0,16);
	WriteDispBuff("                             ",2,0,16);
	WriteDispBuff("        按取消退出 ",3,0,16);
	newmouse.num = 0;
}
void Cdisp::M413_clearEvent()
{
	MASK = 0x0;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
		SysEventReset();
	}
	WriteDispBuff("                             ",0,0,16);
	WriteDispBuff("      所有记录清除成功",1,0,16);
	WriteDispBuff("                             ",2,0,16);
	WriteDispBuff("         按取消退出 ",3,0,16);
	newmouse.num = 0;
}
void Cdisp::M440_clearEventitem()
{
	char row1[]="1.动作记录";
	char row2[]="2.操作记录";
	char row3[]="3.告警记录";
	char row4[]="4.所有记录";
	char row6[]="选择记录类型";

	m_pwd_flag=0;
	WriteDispBuff(row6,0,0,16);
	if(pwinmenu->para < 5)
	{
		WriteDispBuff(row1,1,0,10);
		WriteDispBuff(row2,2,0,10);
		WriteDispBuff(row3,3,0,10);
		WriteDispBuff(row4,4,0,10);
	}
	newmouse.y = 0;

	newmouse.x = (pwinmenu->para-1) % maxdatarow + 1;
	newmouse.num = 10;
	MASK = 0;
}

void Cdisp::M410_DoEvent()
{
	WORD poolNum,wrtOffset,curPtr;
	WORD curpage;
	VSysClock tm;
	int i;
	MASK = 0x4f;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
	}
	curpage = (pwinmenu->row / maxdatarow) * maxdatarow;
	
	if(QuerySysEventInfo(g_Sys.pDoEvent,1,0,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum)==0)
	{
		WriteDispBuff("无操作记录",2,8,15);
		newmouse.num = 0;
		return;
	}
	sprintf((char*)tt,"操作记录%d/%d",pwinmenu->row+1,poolNum);
	WriteDispBuff(tt,0,0,15);					
	
	for(i = 0; i < maxdatarow; i++)
	{
		if((wrtOffset) >= curpage + i)
			curPtr = wrtOffset -(curpage + i);
		else
			curPtr = poolNum + wrtOffset - (curpage + i);
		curPtr--;	
		curPtr %= poolNum;
		
		if(curpage+i>=poolNum)
		{
			WriteDispBuff("返回",i+1,5,16);	
			m_Eventinfo[i].time.dwMinute = 0;
			break;
		}
		
		if(QuerySysEventInfo(g_Sys.pDoEvent,1,curPtr,sizeof(m_Eventinfo[0]),m_Eventinfo+i,&wrtOffset,&poolNum))
		{
			CalClockTo(&m_Eventinfo[i].time,&tm);  
			sprintf((char*)tt,"%02d-%02d %02d:%02d:%02d:%03d",tm.byMonth, tm.byDay,tm.byHour,tm.byMinute,tm.bySecond,tm.wMSecond);
			WriteDispBuff(tt,i+1,0,16);	
		}
		else if(curpage+i == wrtOffset)
		{
  			WriteDispBuff("返回",i+1,5,16);	
			m_Eventinfo[i].time.dwMinute = 0;
			if(pwinmenu->row > wrtOffset)
				pwinmenu->row = 0;
			break;
		}
		else
			pwinmenu->row = 0;			
	}

	M4_key(poolNum);
	newmouse.x = (pwinmenu->row % maxdatarow) +1;
	newmouse.y = 0;
	newmouse.num = 18;
}

void Cdisp::M4_key(WORD poolNum)
{
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case DOWNKEY://up
				  pwinmenu->row++;
				  if(pwinmenu->row >= poolNum)
				  	pwinmenu->row = 0;
					break;
			case UPKEY://down
					 if(pwinmenu->row <= 0)
				  		pwinmenu->row = 0;//poolNum-1;
					 else
					 	pwinmenu->row--;
					 break;
				case ENTERKEY:
					if(m_Eventinfo[pwinmenu->row%maxdatarow].time.dwMinute != 0)
						Entersonmenu();
					else
						Returntopmenu();
					break;
					
		}
	}
}
void Cdisp::M210_DownData()
{
	char row1[]="1.遥    测";
	char row2[]="2.单点遥信";
	char row3[]="3.单点SOE";
	char row6[]="(3)上传数据";
	WriteDispBuff(row6,0,0,16);
	if(pwinmenu->para < 5)
	{
		WriteDispBuff(row1,1,0,10);
		WriteDispBuff(row2,2,0,10);
		WriteDispBuff(row3,3,0,10);
	}

	newmouse.y = 0 ;	
	newmouse.x = (pwinmenu->para-1) % maxdatarow + 1;
	
	newmouse.num = 10;
	m_keyupdownval = 0;
	MASK = 0;
}
void Cdisp::M610_viewpara()
{
	char row1[]="1.基本信息";
	char row2[]="2.版本信息";
	char row3[]="3.运行信息";
	char row4[]="4.告警信息";
	char row6[]="(5)终端信息";
	WriteDispBuff(row6,0,0,16);
	if(pwinmenu->para < 5)
	{
		WriteDispBuff(row1,1,0,10);
		WriteDispBuff(row2,2,0,10);
		WriteDispBuff(row3,3,0,10);
		WriteDispBuff(row4,4,0,10);
	}
	
	newmouse.y = 0;
	newmouse.x = (pwinmenu->para - 1) % maxdatarow + 1;
	newmouse.num = 10;
	MASK = 0;
}
void Cdisp::M41_viewmsg()
{
	char row1[]="1.动作记录";
	char row2[]="2.操作记录";
	char row3[]="3.告警记录";
	char row4[]="4.记录清除";
	char row6[]="(7)事件记录";
	WriteDispBuff(row6,0,0,16);
	if(pwinmenu->para < 5)
	{
		WriteDispBuff(row1,1,0,10);
		WriteDispBuff(row2,2,0,10);
		WriteDispBuff(row3,3,0,10);
		WriteDispBuff(row4,4,0,10);
	}
	
	newmouse.y = 0;
	
	newmouse.x = (pwinmenu->para - 1) % 4 + 1;
	
	newmouse.num = 10;
	MASK = 0;
}



void Cdisp::M314_YK()
{
	MASK = 0xcf;
	m_pwd_flag = 0;
	switch(m_ykflag)
	{
		case 1:
			selectykaction();
			break;
		case 2:
			dofrontyk();
			break;
		case 4:
			doyk();
			break;
		case 5:
			doykcx();
			break;
		case 6:
			doykyzovertime();
			break;
		case 7:
			doykfh07();
			break;
		case 8:
			doykfh08();
			break;
		case 9:
			doykfh09();
			break;
		case 10:
			doykfh10();
			break;
		case 11:
			doykfh11();
			break;
		case 12:
			doykfh12();
			break;
		case 13:
			doykfh13();
			break;
		case 14:
			doykfh14();
			break;
		case 15:
			doykfh15();
			break;
		case 16:
			doykfh16();
			break;
		case 17:
			doykfh17();
			break;
		case 18:
			doykfh18();
			break;	
		default:
			selectykeqp();
			break;
	
	}
	
}


void Cdisp::M310_DownData()
{
	char row1[]="1.遥    测";
	char row2[]="2.单点遥信";
	char row3[]="3.单点SOE";
	char row4[]="4.遥    控";
	//char row5[]="5.返回";
	char row6[]="(4)采集数据 ";
	WriteDispBuff(row6,0,0,16);
	if(pwinmenu->para < 5)
	{
		WriteDispBuff(row1,1,0,10);
		WriteDispBuff(row2,2,0,10);
		WriteDispBuff(row3,3,0,10);
		WriteDispBuff(row4,4,0,10);
	}
	
	newmouse.y = 0;
	
	newmouse.x = (pwinmenu->para-1) % maxdatarow + 1;

	newmouse.num = 10;
	MASK = 0;
	m_ykflag = 0;
	m_ykdot = 0;
	m_keyupdownval = 0;
}

void Cdisp::selectykeqp()
{
	char row0[]="请选择遥控点号:";
	int maxnum;
	int i;
	WORD buf[4]; 
	struct VTrueCtrlCfg *pTYK = NULL;
	struct VVirtualCtrl *pVYK = NULL;
	MASK = 0x4f;
	m_keyupdownval = m_mousevalue[5];
	if(m_pEqpInfo->pTEqpInfo != NULL)
	{
		maxnum = m_pEqpInfo->pTEqpInfo->Cfg.wYKNum;
		pTYK = m_pEqpInfo->pTEqpInfo->pYKCfg;
	}
	if(m_pEqpInfo->pVEqpInfo != NULL)
	{
		maxnum = m_pEqpInfo->pVEqpInfo->Cfg.wYKNum;
		pVYK = m_pEqpInfo->pVEqpInfo->pYK;
	}
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 1://up
				m_keyupdownval--;
				break;
			case 2://down
				m_keyupdownval++;
				break;
			case 0x10://pgup
				m_ykdot--;
				break;
			case 0x20://pgdown
				m_ykdot++;
				break;
			case 0x40://enter
				if(m_keyupdownval >= maxnum)
				{
					Returntopmenu();
					Returntopmenu();
				}
				else
				{
					m_keyupdownval = 0;
					m_ykflag = 1;
					clearscreen();
				}
				return;
		}

	}
	if(m_keyupdownval > 250) m_keyupdownval = maxnum - 1;
	else if(m_keyupdownval >= maxnum + 1)
		m_keyupdownval = 0;	
	if(m_ykdot > 200)
		m_ykdot = (maxnum-1) / maxdatarow;
	if(m_ykdot * maxdatarow >= maxnum)
		m_ykdot = 0;
	m_ykdot = m_keyupdownval/maxdatarow;
	if(m_keyval == 1)
	{
		while((m_keyupdownval >= maxnum) && (m_keyupdownval != 0))
			m_keyupdownval--;
	}
	WriteDispBuff(row0,0,0,16);
	i = 0;
	while(i < m_ykdot * maxdatarow)
	{
		if(pTYK) pTYK++;
		if(pVYK) pVYK++;
		i++;
	}
	for(i = 0; i < maxdatarow; i++)
	{
		if(m_ykdot * maxdatarow + i < maxnum)
		{
			if(ReadRangeAllYK((WORD)m_pEqpInfo->wID,m_ykdot*maxdatarow+i,1,4,buf) == 0)
				continue;
			if(m_pEqpInfo->pTEqpInfo != NULL)
				sprintf((char*)tt,"%d.%s",buf[0],pTYK->sNameTab);
			if(m_pEqpInfo->pVEqpInfo != NULL)
				sprintf((char*)tt,"%d.%s",buf[0],pVYK->sNameTab);
			WriteDispBuff(delchar(tt),i+1,1,16);
			if(m_keyupdownval % maxdatarow == i)
			{
				if(pVYK)
				{
					m_YKID = buf[0];
					memcpy(sYKNameTab,pVYK->sNameTab,15);
				}
				if(pTYK)
				{
					m_YKID = buf[0];
					memcpy(sYKNameTab,pTYK->sNameTab,15);
				}

			}
			if(pTYK) pTYK++;
			if(pVYK) pVYK++;
		}
		else
		{
			WriteDispBuff("返回",i+1,1,16);
			break;
		}
	}
	newmouse.x = (m_keyupdownval) % maxdatarow + 1;
	m_mousevalue[5] = m_keyupdownval;
	newmouse.y = 1;
	newmouse.num = 14;
}


void Cdisp::selectykaction()
{
	MASK = 0xcf;
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
			switch(m_keyval)
			{
				case 1://left
				case 4://left
				case 2://right
				case 8://right
				if(m_keyupdownval)
					m_keyupdownval = 0;
				else
					m_keyupdownval = 1;
					break;
				case 0x40://enter
					m_ykflag = 2;
					m_ykstate = m_keyupdownval;
					clearscreen();
					m_YKflag = 0;
					return;
				case 0x80://enter
					m_ykflag = 0;
					clearscreen();
					return;

			}
	}
		
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff("合  分",3,4,16);
	newmouse.x=3;
	if(m_keyupdownval != 1) m_keyupdownval = 0;
	newmouse.y = (m_keyupdownval + 1) * 4;
	newmouse.num = 2;
}

void Cdisp::dofrontyk()
{
	char row3[]="        按回车执行";
	char row2[]="         预置跳闸";
	char row4[]="         预置合闸";
	VDBYK * pDBYK = (VDBYK *)DispMsg.abyData;
	
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
	switch(m_keyval)
	{
		case 0x40://enter
			m_ykflag = 17;
			m_ykdelay = 0;
			clearscreen();
			pDBYK->wID = LOWORD(m_YKID);
			pDBYK->byValue = (m_ykstate+1)|CTR_BYID|USEDEF_PLUSE;
			pDBYK->byStatus = 0;
			m_YKmsg.wID = pDBYK->wID;
			m_YKmsg.byValue = pDBYK->byValue&0xf;
			m_curCtrlMsgID = MI_YKSELECT;
		 	TaskSendMsg(DB_ID,MMI_ID,m_pEqpInfo->wID,m_curCtrlMsgID,MA_REQ,sizeof(VDBYK),&DispMsg);	
		 	return;
		case 0x80://enter
			m_ykflag = 0;
			clearscreen();
			return;
		}
	}
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	if(m_ykstate == 1)
		WriteDispBuff(row2,3,0,16);
	else
		WriteDispBuff(row4,3,0,16);
	WriteDispBuff(row3,4,0,16);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
}

void Cdisp::doykcx()
{
	char row3[]="        enter-执行撤销";
	char row2[]="           执行撤销";
	VDBYK * pDBYK = (VDBYK *)DispMsg.abyData;
	
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;

				clearscreen();
				pDBYK->wID = LOWORD(m_YKID);
				pDBYK->byValue = (m_ykstate + 1) | CTR_BYID | USEDEF_PLUSE;
				pDBYK->byStatus = 0;
				m_YKmsg.wID = pDBYK->wID;
				m_YKmsg.byValue = pDBYK->byValue & 0xf;
				m_curCtrlMsgID = MI_YKCANCEL;
				TaskSendMsg(DB_ID,MMI_ID,m_pEqpInfo->wID,m_curCtrlMsgID,MA_REQ,sizeof(VDBYK),&DispMsg);	
				return;
		}
	}
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row2,3,0,10);
	WriteDispBuff(row3,4,0,16);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
}


void Cdisp::doykfh07()
{
	char row3[]="       点号非法";
	
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;
				clearscreen();
				return;

		}
	}
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
}
void Cdisp::doykfh08()
{
	char row3[]="     该点正在被操作";
	VDBYK * pDBYK = (VDBYK *)DispMsg.abyData;
	
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;
				clearscreen();
				pDBYK->wID = LOWORD(m_YKID);
				pDBYK->byValue = (m_ykstate + 1) | CTR_BYID | USEDEF_PLUSE;
				pDBYK->byStatus = 0;
				m_YKmsg.wID = pDBYK->wID;
				m_YKmsg.byValue = pDBYK->byValue&0xf;
				m_curCtrlMsgID = MI_YKCANCEL;
				TaskSendMsg(DB_ID,MMI_ID,m_pEqpInfo->wID,m_curCtrlMsgID,MA_REQ,sizeof(VDBYK),&DispMsg);	
			return;
		}
	}
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
}
void Cdisp::doykfh09()
{
	char row3[]="    控制硬件有问题";

	VDBYK * pDBYK = (VDBYK *)DispMsg.abyData;
	
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;
				clearscreen();
			 	pDBYK->wID = LOWORD(m_YKID);
			  	pDBYK->byValue = (m_ykstate + 1) | CTR_BYID | USEDEF_PLUSE;
			  	pDBYK->byStatus = 0;
				m_YKmsg.wID = pDBYK->wID;
				m_YKmsg.byValue = pDBYK->byValue&0xf;
				m_curCtrlMsgID = MI_YKCANCEL;
			 	TaskSendMsg(DB_ID,MMI_ID,m_pEqpInfo->wID,m_curCtrlMsgID,MA_REQ,sizeof(VDBYK),&DispMsg);	
				return;
		}
	}
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
}
void Cdisp::doykfh10()
{
	char row3[]="      控制禁止";

	VDBYK * pDBYK = (VDBYK *)DispMsg.abyData;
	
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;
				clearscreen();
			 	pDBYK->wID = LOWORD(m_YKID);
			  	pDBYK->byValue = (m_ykstate + 1) | CTR_BYID | USEDEF_PLUSE;
			  	pDBYK->byStatus = 0;
				m_YKmsg.wID = pDBYK->wID;
				m_YKmsg.byValue = pDBYK->byValue & 0xf;
				m_curCtrlMsgID = MI_YKCANCEL;
			 	TaskSendMsg(DB_ID,MMI_ID,m_pEqpInfo->wID,m_curCtrlMsgID,MA_REQ,sizeof(VDBYK),&DispMsg);	
				Returntopmenu();
				Returntopmenu();
				return;
		}
	}
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
}
void Cdisp::doykfh11()
{
	char row3[]="      预制失败";
	VDBYK * pDBYK = (VDBYK *)DispMsg.abyData;
	
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;
				clearscreen();
				pDBYK->wID = LOWORD(m_YKID);
				pDBYK->byValue = (m_ykstate + 1) | CTR_BYID | USEDEF_PLUSE;
				pDBYK->byStatus = 0;
				m_YKmsg.wID = pDBYK->wID;
				m_YKmsg.byValue = pDBYK->byValue&0xf;
				m_curCtrlMsgID = MI_YKCANCEL;
				TaskSendMsg(DB_ID,MMI_ID,m_pEqpInfo->wID,m_curCtrlMsgID,MA_REQ,sizeof(VDBYK),&DispMsg);	
				Returntopmenu();
				Returntopmenu();
				return;
		}
	}
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num=0;
}
void Cdisp::doykfh12()
{
	char row3[]="      执行失败";
	VDBYK * pDBYK = (VDBYK *)DispMsg.abyData;
	
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;
				clearscreen();
			 	pDBYK->wID = LOWORD(m_YKID);
			 	pDBYK->byValue = (m_ykstate + 1) | CTR_BYID | USEDEF_PLUSE;
			 	pDBYK->byStatus = 0;
				m_YKmsg.wID = pDBYK->wID;
				m_YKmsg.byValue = pDBYK->byValue&0xf;
				m_curCtrlMsgID = MI_YKCANCEL;
				TaskSendMsg(DB_ID,MMI_ID,m_pEqpInfo->wID,m_curCtrlMsgID,MA_REQ,sizeof(VDBYK),&DispMsg);	
				Returntopmenu();
				Returntopmenu();
				break;
		}
	}
}
void Cdisp::doykfh13()
{
	char row3[]="       撤销失败";

	VDBYK * pDBYK = (VDBYK *)DispMsg.abyData;
		
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;	
				clearscreen();
				pDBYK->wID = LOWORD(m_YKID);
				pDBYK->byValue =(m_ykstate + 1) | CTR_BYID | USEDEF_PLUSE;
				pDBYK->byStatus = 0;
				m_YKmsg.wID = pDBYK->wID;
				m_YKmsg.byValue = pDBYK->byValue&0xf;
				m_curCtrlMsgID = MI_YKCANCEL;
				TaskSendMsg(DB_ID,MMI_ID,m_pEqpInfo->wID,m_curCtrlMsgID,MA_REQ,sizeof(VDBYK),&DispMsg);	
				Returntopmenu();
				Returntopmenu();
				break;
		}
	}
}
void Cdisp::doykfh14()
{
	char row3[]="       执行成功";

	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;
				clearscreen();
				break;
		}
	}
}

void Cdisp::doykfh15()
{
	char row3[]="      撤销成功";

	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;
				clearscreen();
				break;
		}
	}
	if(m_ykdelay > 5)
		m_ykflag = 0 ;

}
void Cdisp::doykfh16()
{
	char row3[]="      预置成功";

	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80:	
				m_ykflag = 4;
				clearscreen();
			break;
		}
	}
	if(m_ykdelay > 5)
		m_ykflag = 4;

}
void Cdisp::doykfh17()
{
	char row3[]="     执行中...";
	int i;
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(row3,2,0,10);
	for( i = 0; i < m_ykdelay; i++)
	{
		WriteDispBuff(".",2,i+7,10);
	}
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
	if(m_ykdelay > 5)
		m_ykflag = 18;

}
void Cdisp::doykfh18()
{
	char row3[]="     超时,操作失败";
		
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	WriteDispBuff(row3,3,0,10);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	
	newmouse.num = 0;
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 0x40://enter
			case 0x80://enter
				m_ykflag = 0;
				clearscreen();
				break;
		}
	}
	if(m_ykdelay > 5)
		m_ykflag = 0;
			

}
void Cdisp::doyk()
{
	char row3[]="       按回车执行";
	char row2[]="     	遥控跳闸";
	char row4[]="       遥控合闸";
	VDBYK * pDBYK = (VDBYK *)DispMsg.abyData;
	
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
	switch(m_keyval)
	{
		case 0x40://enter
			m_ykflag = 17;
			m_ykdelay = 0;
			clearscreen();
			pDBYK->wID = LOWORD(m_YKID);
			pDBYK->byValue = (m_ykstate + 1) | CTR_BYID | USEDEF_PLUSE;
			pDBYK->byStatus = 0;
			m_YKmsg.wID = pDBYK->wID;
			m_YKmsg.byValue = pDBYK->byValue & 0xf;
			m_curCtrlMsgID = MI_YKOPRATE;
			TaskSendMsg(DB_ID,MMI_ID,m_pEqpInfo->wID,m_curCtrlMsgID,MA_REQ,sizeof(VDBYK),&DispMsg);	
			return;
		case 0x80://enter
			m_ykflag = 0;
			clearscreen();
			pDBYK->wID = LOWORD(m_YKID);
			pDBYK->byValue = (m_ykstate + 1) | CTR_BYID | USEDEF_PLUSE;
			pDBYK->byStatus = 0;
			m_YKmsg.wID = pDBYK->wID;
			m_YKmsg.byValue = pDBYK->byValue&0xf;
			m_curCtrlMsgID = MI_YKCANCEL;
			TaskSendMsg(DB_ID,MMI_ID,m_pEqpInfo->wID,m_curCtrlMsgID,MA_REQ,sizeof(VDBYK),&DispMsg);	
			break;
		}
	}
	sprintf((char*)tt,"设备:%s",m_pEqpInfo->sCfgName);
	WriteDispBuff(tt,0,0,16);
	sprintf((char*)tt,"遥控点号:%s",delchar(sYKNameTab));
	WriteDispBuff(tt,1,0,16);
	if(m_ykstate == 1)
		WriteDispBuff(row2,3,0,16);
	else
		WriteDispBuff(row4,3,0,16);
	WriteDispBuff(row3,4,0,16);
	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	newmouse.num = 0;
}

void Cdisp::doykyzovertime()
{

	char row2[]="     预置跳闸执行超时";
	char row3[]="       任意键退出";
	char row4[]="     预置合闸执行超时";

	if(m_ykstate == 1)
		WriteDispBuff(row2,3,0,16);
	else
		WriteDispBuff(row4,3,0,16);
	WriteDispBuff(row3,4,0,16);

	sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
	WriteDispBuff(tt,0,12,8);
	MASK = 0x0f;
	
	switch(m_keyval)
	{
		case 0x10:
			m_ykflag--;
			memset(m_dispbuf,0x20,sizeof(m_dispbuf));
			break;
		case 0x1:
		case 0x2:
		case 0x4:
		case 0x8:
		case 0x20:
			m_ykflag -= 2;
			memset(m_dispbuf,0x20,sizeof(m_dispbuf));
			break;
	}
	newmouse.num = 0;
}

int Cdisp::sendtoscreen()
{
	WORD len=0;
	DWORD tmp;

	if(!sendflag)
		return 0;

	sendflag=0;
			
	switch(m_dispcmd)
	{
		case 1:
		case 3:
		case 5:
			len=zhengDispData1();
			len=zhengDispData2();
			len=zhengDispData3();
			len=zhengDispData4();	
			len=zhengDispData5();
			break;
		case 2:
		case 4:
		case 6:
			len=fanDispData();
			break;
		case 7:
			len=TxdBlacklight();
			break;
		
	}
	m_dispcmd++;
	//if(len == 0)
	{
		if(m_lamp_turn_light > 0)
		{
			tmp = m_lamp_turn_light;
		}
		else
		{
			tmp = LedLight;
		}
		
		m_lamp_state = tmp;
		len = SendLight(m_lamp_state);	
		
	}
	return len;

}
int Cdisp::sendtopopscreen()
{
	int len=0;
	DWORD tmp;

	if(!sendflag)
		return 0;

	sendflag = 0;
	
	switch(m_dispcmd)
	{
		case 2:
		case 4:
		case 6:
			len = fanDispData();
			break;
		case 1:
		case 3:
		case 5:
			len = TxdDispallscreen1();
			len = TxdDispallscreen2();
			len = TxdDispallscreen3();
			len = TxdDispallscreen4();
			len = TxdDispallscreen5();
			break;
		case 7:
			len = TxdBlacklight();
			break;
	}
	m_dispcmd++;
	//if(len == 0)
	{	
		if(m_lamp_turn_light > 0)
		{
			tmp = m_lamp_turn_light;
		}
		else
		{
			tmp = LedLight;
		}
		
		m_lamp_state = tmp;
		len = SendLight(m_lamp_state);	
	}
	
	return len;

}

BYTE Cdisp::TxdDispallscreen1()
{
	int i;
	if(m_lamp_delay > closeTIME) 
	{
		return 0;
	}

	SendFrameHead(DIS_TYPE_DATA, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = 0;
	m_Send.pBuf[m_Send.wWritePtr++] = 1;
	
	for(i = m_Send.wWritePtr; i< m_size + 9; i++)
		m_Send.pBuf[i] = m_dispbuf[i-9];

	m_Send.wWritePtr = i;
	SendFrameTail();
	
	memcpy(m_bakdispbuf,m_dispbuf,m_size*3);

	return m_Send.wWritePtr;
}

BYTE Cdisp::TxdDispallscreen2()
{
	int i;
	if(m_lamp_delay > closeTIME) 
	{
		return 0;
	}

	SendFrameHead(DIS_TYPE_DATA, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = 0;
	m_Send.pBuf[m_Send.wWritePtr++] = 4;
	
	for(i = m_Send.wWritePtr; i < m_size + 9; i++)
		m_Send.pBuf[i] = m_dispbuf[i-9+m_size];

	m_Send.wWritePtr = i;
	SendFrameTail();
	
	memcpy(m_bakdispbuf,m_dispbuf,m_size*3);

	return m_Send.wWritePtr;
}

BYTE Cdisp::TxdDispallscreen3()
{
	int i;
	if (m_lamp_delay > closeTIME) 
	{
		return 0;
	}

	SendFrameHead(DIS_TYPE_DATA, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = 0;
	m_Send.pBuf[m_Send.wWritePtr++] = 7;
	
	for(i = m_Send.wWritePtr; i < m_size + 9; i++)
		m_Send.pBuf[i] = m_dispbuf[i-9+m_size*2];

	m_Send.wWritePtr = i;
	SendFrameTail();
	
	memcpy(m_bakdispbuf,m_dispbuf,m_size*3);

	return m_Send.wWritePtr;
}

BYTE Cdisp::TxdDispallscreen4()
{
	int i;
	if(m_lamp_delay>closeTIME) 
	{
		return 0;
	}

	SendFrameHead(DIS_TYPE_DATA, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = 0;
	m_Send.pBuf[m_Send.wWritePtr++] = 10;
	
	for(i = m_Send.wWritePtr;i < m_size + 9; i++)
		m_Send.pBuf[i] = m_dispbuf[i-9+m_size*3];

	m_Send.wWritePtr = i;
	SendFrameTail();
	
	memcpy(m_bakdispbuf,m_dispbuf,m_size*3);

	return m_Send.wWritePtr;
}

BYTE Cdisp::TxdDispallscreen5()
{
	int i;
	if(m_lamp_delay > closeTIME) 
	{
		return 0;
	}

	SendFrameHead(DIS_TYPE_DATA, EXTMMI_FUN_WRITE, 1);
	
	m_Send.pBuf[m_Send.wWritePtr++] = 0;
	m_Send.pBuf[m_Send.wWritePtr++] = 13;
	
	for(i = m_Send.wWritePtr; i < m_size + 9; i++)
		m_Send.pBuf[i] = m_dispbuf[i-9+m_size*4];

	m_Send.wWritePtr = i;
	SendFrameTail();
	
	memcpy(m_bakdispbuf,m_dispbuf,m_size*3);

	return m_Send.wWritePtr;
}



void Cdisp::Entersonmenu()
{
	dealkey(ENTERKEY);
}
void Cdisp::Returntopmenu()
{
	dealkey(QUITKEY);
				
}
void Cdisp::Return2topmenu()
{
	dealkey(QUITKEY);
	dealkey(QUITKEY);
	dealkey(QUITKEY);			
}
void Cdisp::Downmenu()
{
	dealkey(DOWNKEY);			
}

void Cdisp::dealupdownvalue(int ttl,BYTE row)
{
	MASK = 0xf;
	if(ttl == 0) 
	{
		m_keyupdownval = 0;
		return;
	}
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case UPKEY:  //up
			case LEFTKEY:  //up
				m_keyupdownval--;
				break;
			case DOWNKEY:  //down
			case RIGHTKEY:  //down
				m_keyupdownval++;
				break;
		}
	}
	if(m_keyupdownval > 250) m_keyupdownval = (ttl - 1) / row;
	else if(m_keyupdownval * row >= ttl)
		m_keyupdownval = 0;	
}

void Cdisp::dealleftrightvalue(int ttl)
{
	MASK = 0xc;
	if(ttl == 0) 
	{
		m_keyleftrightval = 0;
		return;
	}
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case LEFTKEY://left
				m_keyleftrightval--;
				break;
			case RIGHTKEY://right
				m_keyleftrightval++;
				break;
		}
	}
	if(m_keyleftrightval > 250) m_keyleftrightval = ttl - 1;
	else if(m_keyleftrightval >= ttl)
		m_keyleftrightval = 0;	
}
void Cdisp::clearscreen()
{
	memset(	m_dispbuf,0x20,sizeof(m_dispbuf));
}

void Cdisp::selectgroupeqp()
{
	char row0[]="请选择装置组:";
	int i;
	MASK = LEFTKEY|RIGHTKEY|UPKEY|DOWNKEY|ENTERKEY;
	if(m_pgroupeqp[0] == NULL)
	{
		pgdngroupeqp();
	}
	m_keyupdownval = m_mousevalue[3];
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();

		switch(m_keyval)
		{
			case UPKEY://up
				if(m_keyupdownval == 0)
					m_keyupdownval++;
				m_keyupdownval--;
				break;
			case DOWNKEY://down
				if(m_keyupdownval == 3)
					pgdngroupeqp();
				m_keyupdownval++;
				break;
			case ADDKEY://pgup
				pgupgroupeqp();
				break;
			case RIGHTKEY://pgdown
				m_keyupdownval=0;
				pgdngroupeqp();
				break;
			case LEFTKEY:
				m_keyupdownval=0;
				pgdngroupeqp();
				break;
			case ENTERKEY://pgdown
				if(m_pgroupeqp[m_keyupdownval] != NULL)
				{
					m_eqpid[0] = m_pgroupeqp[m_keyupdownval]->pEqpGInfo->Cfg.wEqpNum;
					memcpy(m_eqpid+1,m_pgroupeqp[m_keyupdownval]->pEqpGInfo->pwEqpID,m_eqpid[0]*2);
					Entersonmenu();
				}
				else
					Returntopmenu();
				return;
				}
	}
	
	if (m_keyupdownval > 250) m_keyupdownval = 3;
	else if (m_keyupdownval > 3)
		m_keyupdownval = 0;	
	if (m_keyval == 1) //UP
	{
		while((m_pgroupeqp[m_keyupdownval] == NULL) && (m_keyupdownval != 0))
			m_keyupdownval--;
	}
	if (m_keyval == 2) //DOWN
	{
		if ((m_pgroupeqp[m_keyupdownval] == NULL))
		{
			if ((m_keyupdownval == 0) || (m_pgroupeqp[m_keyupdownval-1] == NULL))
			{
				m_keyupdownval = 0;
				pgdngroupeqp();
			}
		}
	}
	WriteDispBuff(row0,0,1,16);
	for(i = 0; i < 4; i++)
	{
		if(m_pgroupeqp[i] != NULL)
		{
			sprintf((char*)tt,"%s",m_pgroupeqp[i]->sCfgName);
			WriteDispBuff(tt,i+1,10,16);
		}
		else
		{
			WriteDispBuff("返回",i+1,10,16);
			break;
		}
	}
	newmouse.x = m_keyupdownval + 1;
	m_mousevalue[3] = m_keyupdownval;
	newmouse.y = 10;
	newmouse.num = 6;
	m_pgroupeqpinfo = m_pgroupeqp[m_keyupdownval];
	
			
}

void Cdisp::pgupgroupeqp()
{
	int nn;
	int i;
	VEqpInfo*  peqp = g_Sys.Eqp.pInfo;
	nn = *g_Sys.Eqp.pwNum;
	while( nn > 1)
	{
		peqp++;
		nn--;
	}
	if((m_pgroupeqp[0] == NULL) || (m_pgroupeqp[3] == NULL)||(m_pgroupeqp[0] == g_Sys.Eqp.pInfo))
	{
		m_pgroupeqp[3] = peqp;
	}
	else		
		m_pgroupeqp[3] = m_pgroupeqp[0] - 1;
	
	for(i = 4; i > 0; i--)
	{
		while((m_pgroupeqp[i-1]->pEqpGInfo == NULL) && (m_pgroupeqp[i-1] > g_Sys.Eqp.pInfo))
			m_pgroupeqp[i-1]--;
		m_pgroupeqp[i-2] = m_pgroupeqp[i-1]-1;
		if(m_pgroupeqp[i-1] < g_Sys.Eqp.pInfo)
			m_pgroupeqp[i-1] = NULL;
	}
	nn = 3;
	while((m_pgroupeqp[0] == NULL) && (nn-- > 0))
	{
		for(i = 0; i < 4; i++)
		{
			m_pgroupeqp[i] = m_pgroupeqp[i+1];
		}
		m_pgroupeqp[i] = NULL;
	}		
}
void Cdisp::pgdngroupeqp()
{
	int nn;
	int i;
	VEqpInfo*  peqp = g_Sys.Eqp.pInfo;
	nn = *g_Sys.Eqp.pwNum;
	while( nn > 1)
	{
		peqp++;
		nn--;
	}
	if((m_pgroupeqp[0] == NULL) || (m_pgroupeqp[2] == NULL)||(m_pgroupeqp[0]->pEqpGInfo == NULL) || (m_pgroupeqp[2]->pEqpGInfo == NULL))
	{
		m_pgroupeqp[0] = g_Sys.Eqp.pInfo;
	}
	else
		m_pgroupeqp[0] = m_pgroupeqp[3]+1;
	
	for(i = 0; i < 4; i++)
	{

		while((m_pgroupeqp[i]->pEqpGInfo == NULL) && (m_pgroupeqp[i] <= peqp))
			m_pgroupeqp[i]++;
		m_pgroupeqp[i+1] = m_pgroupeqp[i] + 1;
		if(m_pgroupeqp[i] > peqp)
		{
			m_pgroupeqp[i] = NULL;
		}
	}		
}

void Cdisp::geteqpid()
{
	VEqpInfo*  peqp = g_Sys.Eqp.pInfo;
	int i;
	int nn = *g_Sys.Eqp.pwNum;
	int j = 1;
	for(i = 0;  i < nn; i++)
	{
		if((peqp->pTEqpInfo != NULL)||(peqp->pVEqpInfo != NULL))
			m_eqpid[j++] = peqp->wID;
		peqp++;
	}
	m_eqpid[0] = j-1;
}

void Cdisp::checkpwd()
{
	BYTE charaddr[][2]={
		{1,10},
		{1,11},			
		{1,12},			
		{1,13}					
	};
	char row0[] = "请输入密码";
	m_wrrdfalg = 1;
	MASK = UPKEY|DOWNKEY|ADDKEY|SUBBKEY|LEFTKEY|RIGHTKEY|QUITKEY|ENTERKEY;
	if(m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]] == 0x20)
	{
		m_pwd[0]='5';
		m_pwd[1]='6';
		m_pwd[2]='7';
		m_pwd[3]='8';
		m_setkeyvalue=0;
	}
	sprintf((char*)tt,"****");
	WriteDispBuff(tt,charaddr[0][0],charaddr[0][1],4);
	WriteDispBuff(row0,0,0,maxcol);
	if(m_setkeyvalue > 200) m_setkeyvalue = (sizeof(charaddr)/sizeof(charaddr[0]))-1;

	newmouse.x = charaddr[m_setkeyvalue][0];
	newmouse.y = charaddr[m_setkeyvalue][1];
	newmouse.num = 1;
	m_dispbuf[charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]] = m_pwd[m_setkeyvalue];
	if(m_keybak != m_keyval)
	{
		if(m_setkeyvalue+1<(sizeof(charaddr)/sizeof(charaddr[0]))||(m_keyval!=0x40))
		{
			if((m_setkeyvalue >= (sizeof(charaddr)/sizeof(charaddr[0])))&&(m_keyval!=0x40))
				m_setkeyvalue = 0;
			setdecnum(m_pwd+m_setkeyvalue);
		}
		else
		{
			newmouse.num = 0;
			if((long)getdispdata(m_pwd,4,0) != 8888)
			{	
				pwinmenu->page = 0;
				Returntopmenu();
				addalarmmenu(&Cdisp::Mpwd_err, nomal_alarm, nomal_alarm);
			}
			else if((long)getdispdata(m_pwd,4,0) == 8888)
			{
				m_pwd_flag = 1;
				Downmenu();
			}
			m_wrrdfalg = 0;
			MASK = 0;
		
		}
	}
	if(m_keyval == 0x80)
		Returntopmenu();
	m_keybak = m_keyval;
}
void Cdisp::checkpwd1()
{
	BYTE charaddr[][2]={
		{1,10},
		{1,11},			
		{1,12},			
		{1,13}						
	};
	char row0[]="请输入密码";
	m_wrrdfalg = 1;
	MASK = UPKEY|DOWNKEY|ADDKEY|SUBBKEY|LEFTKEY|RIGHTKEY|QUITKEY|ENTERKEY;
	if(m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]]==0x20)
	{
		m_pwd[0]='5';
		m_pwd[1]='6';
		m_pwd[2]='7';
		m_pwd[3]='8';
		m_setkeyvalue=0;
	}
	sprintf((char*)tt,"****");
	WriteDispBuff(tt,charaddr[0][0],charaddr[0][1],4);
	WriteDispBuff(row0,0,0,maxcol);
	if(m_setkeyvalue > 200) m_setkeyvalue = (sizeof(charaddr)/sizeof(charaddr[0]))-1;
	else if(m_setkeyvalue >= (sizeof(charaddr)/sizeof(charaddr[0])))
		m_setkeyvalue = 0;
	newmouse.x = charaddr[m_setkeyvalue][0];
	newmouse.y = charaddr[m_setkeyvalue][1];
	newmouse.num = 1;
	m_dispbuf[charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]]=m_pwd[m_setkeyvalue];
	if(m_keybak != m_keyval)
	{
		if(m_setkeyvalue+1 < (sizeof(charaddr)/sizeof(charaddr[0]))||(m_keyval!=0x40))
			setdecnum(m_pwd+m_setkeyvalue);
		else
		{
			if((long)getdispdata(m_pwd,4,0) != 8888)
			{	
				newmouse.num=0;
				pwinmenu->page = 0;
				Returntopmenu();
				addalarmmenu(&Cdisp::Mpwd_err, nomal_alarm, nomal_alarm);
			}
			else
			{
				m_pwd_flag = 1;
				Downmenu();
			}
			m_wrrdfalg = 0;
			MASK=0;
		
		}
	}
	if(m_keyval == 0x80)
		Returntopmenu();
	m_keybak = m_keyval;
}
void Cdisp::checkhomepwd()
{
	BYTE charaddr[][2]={
		{1,10},
		{1,11},			
		{1,12},			
		{1,13}				
	};
	char row0[]="请输入密码";
	m_wrrdfalg = 1;
	MASK = UPKEY|DOWNKEY|ADDKEY|SUBBKEY|LEFTKEY|RIGHTKEY|QUITKEY|ENTERKEY;
	if(m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]] == 0x20)
	{
		m_pwd[0]='5';
		m_pwd[1]='6';
		m_pwd[2]='7';
		m_pwd[3]='8';
		m_setkeyvalue=0;
	}
	sprintf((char*)tt,"****");
	WriteDispBuff(tt,charaddr[0][0],charaddr[0][1],4);
	WriteDispBuff(row0,0,0,maxcol);
	if(m_setkeyvalue > 200) m_setkeyvalue = (sizeof(charaddr)/sizeof(charaddr[0]))-1;

	newmouse.x = charaddr[m_setkeyvalue][0];
	newmouse.y = charaddr[m_setkeyvalue][1];
	newmouse.num = 1;
	m_dispbuf[charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]]=m_pwd[m_setkeyvalue];
	if(m_keybak != m_keyval)
	{
		if(m_setkeyvalue < (sizeof(charaddr)/sizeof(charaddr[0]))-1||(m_keyval!=ENTERKEY))
		{
			if((m_setkeyvalue >= (sizeof(charaddr)/sizeof(charaddr[0])))&&(m_keyval!=ENTERKEY))
				m_setkeyvalue = 0;
			setdecnum(m_pwd+m_setkeyvalue);
		}
		else
		{
			if((long)getdispdata(m_pwd,4,0) != 7777)
			{	
				newmouse.num = 0;
				pwinmenu->page = 0;
				Returntopmenu();
				addalarmmenu(&Cdisp::Mpwd_err, nomal_alarm, nomal_alarm);
			}
			else
			{
				m_pwd_flag = 1;
				Entersonmenu();
			}
			m_wrrdfalg=0;
			MASK=0;
		
		}
	}
	if(m_keyval == QUITKEY)
		Returntopmenu();
	m_keybak = m_keyval;
}
void Cdisp::seteqpaddr()
{
	BYTE charaddr[][2]={
		{1,10},
		{1,11},			
		{1,12},			
		{1,13},			
		{1,14}		
	};
	char row0[]="装置地址设置:";
	WORD addr;
	m_wrrdfalg = 1;
	MASK = 0x4f|ADDKEY|SUBBKEY;
	if(m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]]==0x20)
	{
		sprintf((char*)tt,"%05d",g_Sys.AddrInfo.wAddr);
		WriteDispBuff(tt,charaddr[0][0],charaddr[0][1],5);
		m_setkeyvalue = 0;
	}
	WriteDispBuff(row0,0,0,16);
	if(m_setkeyvalue > 200) m_setkeyvalue = (sizeof(charaddr)/sizeof(charaddr[0]))-1;
	else if(m_setkeyvalue>=(sizeof(charaddr)/sizeof(charaddr[0])))
		m_setkeyvalue = 0;

	newmouse.x = charaddr[m_setkeyvalue][0];
	newmouse.y = charaddr[m_setkeyvalue][1];
	newmouse.num = 1;
	if(m_pwd_flag == 1)
	{
		m_pwd_flag = 0;
		addr = m_conword[0];
		m_wrrdfalg = 0;
		SetSysAddr(&addr,NULL,NULL,NULL,NULL) ;
		Returntopmenu();
		pwinmenu->page = 0;
		addalarmmenu(&Cdisp::Mset_success, nomal_alarm, nomal_alarm);
		return;
	}	
	if(m_keybak != m_keyval)
	{
		if(m_setkeyvalue + 1 < (sizeof(charaddr)/sizeof(charaddr[0]))||(m_keyval!=0x40))
			setdecnum(m_dispbuf+charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]);
		else
		{
			m_conword[0] = (WORD)getdispdata(m_dispbuf+charaddr[0][0]*maxcol+charaddr[0][1],5,0);
			Downmenu();
		}
	}
	m_keybak = m_keyval;
}
void Cdisp::seteqpip1()
{
	BYTE charaddr[][2]={
		{1,8},
		{1,9},			
		{1,10},			
		{1,12},			
		{1,13},		
		{1,14},
		{1,16},			
		{1,17},			
		{1,18},
		{1,20},			
		{1,21},			
		{1,22},			
	};
	BYTE i,j;
	char row0[]="以太网1设置:";
	BYTE net[4];
	char ip[20];
	m_wrrdfalg = 1;
	MASK = 0x4F|ADDKEY|SUBBKEY;
	if(m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]] == 0x20)
	{
		i = 0;
		j = 0;
		net[0] = net[1] = net[2] = net[3] = 0;
		for(j = 0; j < 4; j++)
		{
			while ((g_Sys.AddrInfo.Lan1.sIP[i] >= '0') && (g_Sys.AddrInfo.Lan1.sIP[i]<='9'))
			{
				net[j] *= 10;
				net[j] += (g_Sys.AddrInfo.Lan1.sIP[i] & 0xf);
				i++;
			}
			i++;
		}
		sprintf((char*)tt,"%03d.%03d.%03d.%03d",net[0],net[1],net[2],net[3]);
		WriteDispBuff(tt,charaddr[0][0],charaddr[0][1],15);
		m_setkeyvalue = 0;
	}
	
	WriteDispBuff(row0,0,0,16);
	if(m_setkeyvalue > 200) m_setkeyvalue = (sizeof(charaddr)/sizeof(charaddr[0]))-1;
	else if(m_setkeyvalue >= (sizeof(charaddr)/sizeof(charaddr[0])))
		m_setkeyvalue = 0;

	newmouse.x = charaddr[m_setkeyvalue][0];
	newmouse.y = charaddr[m_setkeyvalue][1];
	newmouse.num = 1;
	if(m_pwd_flag == 1)
	{
		m_pwd_flag = 0;
		sprintf(ip,"%d.%d.%d.%d",m_conword[0],m_conword[1],m_conword[2],m_conword[3]);
		m_wrrdfalg = 0;
		SetSysAddr(NULL,ip,NULL,NULL,NULL) ;
		Returntopmenu();
		pwinmenu->page = 0;
		addalarmmenu(&Cdisp::Mset_success, nomal_alarm, nomal_alarm);
		return;
	}	
	if(m_keybak != m_keyval)
	{
		if(m_setkeyvalue + 1 < (sizeof(charaddr)/sizeof(charaddr[0]))||(m_keyval!=0x40))
			setdecnum(m_dispbuf+charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]);
		else
		{
			for(i = 0; i < 4; i++)		
				m_conword[i] = (BYTE)getdispdata(m_dispbuf+charaddr[i*3][0]*maxcol+charaddr[i*3][1],3,0);
			Downmenu();
		}
	}
	m_keybak = m_keyval;

}

void Cdisp::M611_basemessage()
{
	
	BYTE i,j;
	char row0[]="基本信息:";
	BYTE net[4];
	
	i = 0;
	j = 0;
	net[0] = net[1] = net[2] = net[3] = 0;
	for(j = 0; j < 4; j++)
	{
		while ((g_Sys.AddrInfo.Lan1.sIP[i]>='0')&&(g_Sys.AddrInfo.Lan1.sIP[i]<='9'))
		{
			net[j] *= 10;
			net[j] += (g_Sys.AddrInfo.Lan1.sIP[i]&0xf);
			i++;
		}
		i++;
	}
	
	sprintf((char*)tt,"IP1:%d.%d.%d.%d",net[0],net[1],net[2],net[3]);
	WriteDispBuff(tt,2,0,16);
	
	i = 0;
	j = 0;
	net[0] = net[1] = net[2] = net[3] = 0;
	for(j = 0; j < 4; j++)
	{
		while ((g_Sys.AddrInfo.Lan2.sIP[i] >= '0')&&(g_Sys.AddrInfo.Lan2.sIP[i] <= '9'))
		{
			net[j] *= 10;
			net[j] += (g_Sys.AddrInfo.Lan2.sIP[i] & 0xf);
			i++;
		}
		i++;
	}
	
	sprintf((char*)tt,"IP2:%d.%d.%d.%d",net[0],net[1],net[2],net[3]);
	WriteDispBuff(tt,3,0,16);
	
	WriteDispBuff(row0,0,0,16);
	sprintf((char*)tt,"装置地址: %05d",g_Sys.AddrInfo.wAddr);
	WriteDispBuff(tt,1,0,15);
	sprintf((char*)tt,"%04d-%02d-%02d %02d:%02d:%02d",clock.wYear,clock.byMonth,clock.byDay,clock.byHour,clock.byMinute,clock.bySecond);
	WriteDispBuff(tt,4,0,16);

	newmouse.num = 0;

}

void Cdisp::M613_verinfo()
{
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		pwinmenu->page = 0;
		pwinmenu->para = 0;
	}
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case UPKEY:
				if(pwinmenu->page == 0)
					pwinmenu->page = (g_Sys.LdFileNum<<1)-1;
				else
				  	pwinmenu->page--;
				break;
			case DOWNKEY:
				pwinmenu->page++;
				if(pwinmenu->page >= (g_Sys.LdFileNum<<1)) pwinmenu->page = 0;
					break;
		}

	}	 
	
	sprintf((char*)tt,"版本信息%d/%d  ",pwinmenu->page+1,g_Sys.LdFileNum<<1);
	if((pwinmenu->page % 2) == 0)
	{
		WriteDispBuff(tt,0,0,16);
		sprintf((char*)tt,"模块名称:%s",g_Sys.LdFileTbl[pwinmenu->page>>1].name);
		WriteDispBuff(tt,1,0,16);
		sprintf((char*)tt,"版本号:%s",g_Sys.LdFileTbl[pwinmenu->page>>1].ver);
		WriteDispBuff(tt,2,0,16);
		sprintf((char*)tt,"校验和:0x%04x",g_Sys.LdFileTbl[pwinmenu->page>>1].crc);
		WriteDispBuff(tt,3,0,16);
	}
	else
	{
		WriteDispBuff(tt,0,0,16);
		sprintf((char*)tt,"用户:%s",g_Sys.LdFileTbl[pwinmenu->page>>1].user);
		WriteDispBuff(tt,1,0,16);
		if(g_Sys.LdFileTbl[pwinmenu->page>>1].load==0)
			WriteDispBuff("状态:未加载",2,0,16);
		if(g_Sys.LdFileTbl[pwinmenu->page>>1].load==1)
			WriteDispBuff("状态:加载成功",2,0,16);
		if(g_Sys.LdFileTbl[pwinmenu->page>>1].load==2)
			WriteDispBuff("状态:加载失败",2,0,16);
	}

	newmouse.num = 0;
}
void Cdisp::M411_runinfo()
{
	sprintf((char*)tt,"运行信息%d/%d  ",pwinmenu->page+1,pwinmenu->para);

	WriteDispBuff(tt,0,0,16);
	infoview(LOG_ATTR_INFO);

}
void Cdisp::M421_alarminfo()
{
	sprintf((char*)tt,"告警信息%d/%d  ",pwinmenu->page+1,pwinmenu->para);
	
	WriteDispBuff(tt,0,0,16);
	infoalarmview(LOG_ATTR_WARN|LOG_ATTR_ERROR);
}
void Cdisp::infoview(BYTE attr)
{
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY;
	WORD i;
	char buff[26];
	char row1[]="                                 ";
	newmouse.num=0;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		pwinmenu->page = 0;
		pwinmenu->para = 0;
		for(i = 0; i < sysLog.wWrtPtr; i++)
		{
			if(sysLog.aLogMsg[i].attr == attr)
				pwinmenu->para++;
		}
	}
	if(pwinmenu->para == 0) return; 
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case UPKEY:
				i = 0;
				if(pwinmenu->row == 0)
				{
					pwinmenu->row = sysLog.wWrtPtr;
					pwinmenu->page = pwinmenu->para;
				}

				pwinmenu->row--;
				while(sysLog.aLogMsg[pwinmenu->row].attr != attr)
				{
					pwinmenu->row--;
					if(pwinmenu->row == 0)
						pwinmenu->row = sysLog.wWrtPtr;
					if(i++ > sysLog.wWrtPtr) break;
				}
				if(pwinmenu->page == 0) pwinmenu->page = pwinmenu->para;
				pwinmenu->page--;
				break;
			case DOWNKEY:
				pwinmenu->row++;
				if(pwinmenu->row >= sysLog.wWrtPtr) 
				{				  
					pwinmenu->row = 0;	
					pwinmenu->page = 0;
				}
				else
					pwinmenu->page++;
				if(pwinmenu->page >= pwinmenu->para) pwinmenu->page = 0;
				break;
		}
			
	}
	while(sysLog.aLogMsg[pwinmenu->row].attr != attr)
	{
		pwinmenu->row++;
		if(pwinmenu->row > sysLog.wWrtPtr) 
			pwinmenu->row = 0;	
		if(i++ > sysLog.wWrtPtr) break;
	}	
	if(sysLog.aLogMsg[pwinmenu->row].attr != attr) return;
	if(sysLog.aLogMsg[pwinmenu->row].thid != 0xFFFF)
		sprintf((char*)tt,"%d-%s:                  ",pwinmenu->page+1,GetThName(sysLog.aLogMsg[pwinmenu->row].thid));
	else
		pwinmenu->row = 0;	
	WriteDispBuff(tt,1,0,16);
	sprintf((char*)tt,"%s                        ",delchar(sysLog.aLogMsg[pwinmenu->row].sMsg));
	if(strlen((char*)tt) > 26)
	{
		memcpy(buff, delchar(tt), 26);
		WriteDispBuff(row1,2,0,26);
		WriteDispBuff(buff,2,0,26);
		memset(buff, 0, 26);
		memcpy(buff, tt+26, 26);
		WriteDispBuff(row1,3,0,26);
		WriteDispBuff(buff,3,0,26);
		
	}
	else
	{
		WriteDispBuff((char*)delchar(tt),2,0,strlen((char*)tt));
		WriteDispBuff(row1,3,0,16);
	}
	WriteDispBuff(row1,4,0,16);
	newmouse.num = 0;
}
void Cdisp::infoalarmview(BYTE attr)
{
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY;
	WORD i;
	newmouse.num = 0;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		pwinmenu->page = 0;
		pwinmenu->para = 0;
		for(i = 0; i < sysLog.wWrtPtr; i++)
		{
			if(sysLog.aLogMsg[i].attr != 0)
				pwinmenu->para++;
		}
	}
	if(pwinmenu->para == 0) 
	{
		WriteDispBuff("                 ",0,0,16);
		WriteDispBuff("无告警信息!",2,10,16);
		return;
	}
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case UPKEY:
				i = 0;
				if(pwinmenu->row == 0)
				{
					pwinmenu->row = sysLog.wWrtPtr;
					pwinmenu->page = pwinmenu->para;
				}
				else
					pwinmenu->row--;
				while(sysLog.aLogMsg[pwinmenu->row].attr == 0)
				{
					pwinmenu->row--;
					if(pwinmenu->row == 0)
						pwinmenu->row = sysLog.wWrtPtr;
					if(i++ > sysLog.wWrtPtr) break;
				}
				if(pwinmenu->page == 0) pwinmenu->page = pwinmenu->para;
				pwinmenu->page--;
				break;
			case DOWNKEY:
				pwinmenu->row++;
				if(pwinmenu->row > sysLog.wWrtPtr) 
				{				  
					pwinmenu->row = 0;	
					pwinmenu->page = 0;
				}
				else
					pwinmenu->page++;
				if(pwinmenu->page >= pwinmenu->para) pwinmenu->page = 0;
				break;
		}

	}
	while(sysLog.aLogMsg[pwinmenu->row].attr == 0 )
	{
		pwinmenu->row++;
		if(pwinmenu->row > sysLog.wWrtPtr) 
			pwinmenu->row = 0;	
		if(i++ > sysLog.wWrtPtr) break;
	}	
	while(sysLog.aLogMsg[pwinmenu->row].attr == 0 )
	{
		pwinmenu->row++;
		if(pwinmenu->row > sysLog.wWrtPtr)
		{
			pwinmenu->row = 0;
			break;
		}
	}
	if(sysLog.aLogMsg[pwinmenu->row].attr == 0) return;
	
	if(sysLog.aLogMsg[pwinmenu->row].thid != 0xFFFF)
	{
		sprintf((char*)tt,"%d-%s:",pwinmenu->page+1,GetThName(sysLog.aLogMsg[pwinmenu->row].thid));

		WriteDispBuff(tt,1,0,16);
		sprintf((char*)tt,"%s         ",sysLog.aLogMsg[pwinmenu->row].sMsg);
		WriteDispBuff((char*)tt,2,0,strlen((char*)tt));
		newmouse.num = 0;
	}
	else
	{
		WriteDispBuff("                 ",0,0,16);
		WriteDispBuff("无告警信息!",2,10,16);
		newmouse.num = 0;
	}
}

void Cdisp::M71_hometest()
{
	char row0[]="(8)出厂调试";
	char row1[]="1.液晶测试";
	char row2[]="2.复位";
	char row3[]="3.开入测试";
	char row4[]="4.开出测试";
	char row5[]="5.交采整定";
	char row6[]="6.拨码测试";

	m_pwd_flag = 0;

	WriteDispBuff(row0,0,0,16);
	if(pwinmenu->para < 5)
	{
		WriteDispBuff(row1,1,0,16);
		WriteDispBuff(row2,2,0,16);
		WriteDispBuff(row3,3,0,16);
		WriteDispBuff(row4,4,0,16);
	}
	else if(pwinmenu->para < 9)
	{
		WriteDispBuff(row5,1,0,16);
		WriteDispBuff(row6,2,0,16);
	}

	
	newmouse.y = 0;
	newmouse.x = (pwinmenu->para - 1 ) % maxdatarow + 1;
	newmouse.num = 10;
	m_lamp_turn_light = 0;
	MASK = 0;
}

void Cdisp::M712_stopdog()
{
	 reboot(0);
}
void Cdisp::M717_reboot1()
{
	 reboot(1);
}
void Cdisp::M711_screentest()
{
	char row0[]="面板灯轮流点亮";
	char row1[]="任意键退出";
	
	WriteDispBuff(row0,0,0,16);
	WriteDispBuff(row1,2,0,16);
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_lamp_turn_light = 1;
	}
	if((m_keybak != m_keyval) && (m_keyval))
	{
		Returntopmenu();
	}
	if(pwinmenu->row++ > 3)
	{
		pwinmenu->row = 0;
		if(m_lamp_turn_light)
			m_lamp_turn_light <<= 1;
		if(m_lamp_turn_light > 0x400000)
			m_lamp_turn_light = 1;
	}
	if(m_lamp_turn_light == 0)
		m_lamp_turn_light = 1;

	newmouse.x = 0;
	newmouse.y = 0;
	newmouse.num = 0;
	MASK = UPKEY | DOWNKEY | ENTERKEY;
}


void Cdisp::M713_yxtest()
{
	
	BYTE yx[18];
	BYTE i;
	char zf[] = "分合";
	if(pwinmenu->row)
	{
		ReadRangeAllSYX(g_Sys.wIOEqpID,18,18,18*sizeof(BYTE),yx);
	}
	else
	{
		ReadRangeAllSYX(g_Sys.wIOEqpID,0,18,18*sizeof(BYTE),yx);
	}
	if(pwinmenu->flag == 0x55)
		{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
	}
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		if(m_keyval)
		{
			Returntopmenu();
			return;
		}
		
	}
	if(pwinmenu->row > 200) pwinmenu->row = 1;
	if(pwinmenu->row > 1) pwinmenu->row = 0;
		WriteDispBuff("开入调试",0,0,16);
	sprintf((char*)tt,"%d-%d",pwinmenu->row*18+1,pwinmenu->row*18+10);
	WriteDispBuff(tt,1,0,16);
	for(i = 0; i < 10; i++)
	{
		if(yx[i] & 0x80)
			memcpy(tt+i*2,zf+2,2);
		else
			memcpy(tt+i*2,zf,2);
	}
	tt[i*2] = 0;
	WriteDispBuff(tt,2,0,16);
	sprintf((char*)tt,"%d-%d",pwinmenu->row*18+11,pwinmenu->row*18+18);
	WriteDispBuff(tt,3,0,16);
	for(i = 0;i < 8; i++)
	{
		if(yx[i+10] & 0x80)
			memcpy(tt+i*2,zf+2,2);
		else
			memcpy(tt+i*2,zf,2);
	}
	tt[i*2] = 0;
	WriteDispBuff(tt,4,0,16);
	
	newmouse.num = 0;
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY;
}
void Cdisp::M714_yktest()
{
	char fen[]="分";
	char he[]="合";
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_yktesttime = 5;
	}
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case ENTERKEY://up
				pwinmenu->row++;
				break;
			case QUITKEY://up
				for(int i = 0; i < g_Sys.MyCfg.wDONum; i++)
				  	ykOutput(i, 0);
				clearscreen();
				Returntopmenu();
				return;
		}

	}
	WriteDispBuff("开出调试",0,0,16);
	if(pwinmenu->row & 1)
		sprintf((char*)tt,"第%d路: %s",pwinmenu->row/2+1,fen);
	else
		sprintf((char*)tt,"第%d路: %s",pwinmenu->row/2+1,he);
	
	WriteDispBuff(tt,1,4,16);
	sprintf((char*)tt,"倒计时%d秒",m_yktesttime);
	WriteDispBuff(tt,3,4,12);
	if(m_yktesttime == 0)
	{
		ykOutput(pwinmenu->row/2+1, (pwinmenu->row+1)&1);
		m_yktesttime = 5;
		pwinmenu->row++;
		if(pwinmenu->row >= g_Sys.MyCfg.wDONum*2)
		{
			Returntopmenu();
		}
	}
	newmouse.num = 0;
	MASK = ENTERKEY|QUITKEY;
}
void Cdisp::M715_zdtest()
{
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_yktesttime = 5;
	}
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case ENTERKEY://up	
				pwinmenu->row++;
				if( pwinmenu->row > 2)
				{
					Returntopmenu();
					pwinmenu->page = 0;
					addalarmmenu(&Cdisp::Mmeter_success, nomal_alarm, nomal_alarm);
					return;
				}
				break;
		}
			
	}
	WriteDispBuff("交流插件整定",0,0,16);
	sprintf(tt,"第%d步:",pwinmenu->row+1);
	WriteDispBuff(tt,1,0,16);
	
	if(pwinmenu->row == 1)
		WriteDispBuff("UI夹角+45°整定",2,0,16);
	else if(pwinmenu->row == 2)
		WriteDispBuff("UI夹角-45°整定",2,0,16);
	else
		WriteDispBuff("UI都为零开始整定",2,0,16);
	WriteDispBuff("源加好后按确认",3,0,16);
	newmouse.num = 0;
	MASK = ENTERKEY | UPKEY | DOWNKEY;
}

void Cdisp::M716_bmtest()
{
	int i;
	char row0[]="面板拨码开关测试(返回键退出)";
	
	WriteDispBuff(row0,0,0,16);

	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
	}
	if((m_keybak != m_keyval) && (m_keyval))
	{
		Returntopmenu();
	}
	if(extSwitch & 0x02)
		WriteDispBuff("闭锁:投入",2,0,16);
	else
		WriteDispBuff("闭锁:退出",2,0,16);

	if(extSwitch & 0x01)
		WriteDispBuff("操作:远方",2,12,16);
	else
		WriteDispBuff("操作:本地",2,12,16);

	if(extSwitch & 0x08)
		WriteDispBuff("出口: 投入",3,0,16);
	else
		WriteDispBuff("出口: 退出",3,0,16);

	if(extSwitch & 0x04)
		WriteDispBuff("FA:投入",3,12,16);
	else
		WriteDispBuff("FA:退出",3,12,16);
	
	WriteDispBuff("YX:",1,0,16);
	for(i=0; i<5; i++)
	{
		if(MMIYx & (1<<i+9) )
			WriteDispBuff("合",1,i*3+5,16);
		else
			WriteDispBuff("分",1,i*3+5,16);
	}
	
	newmouse.x = 0;
	newmouse.y = 0;
	newmouse.num = 0;
	//MASK = UPKEY | DOWNKEY | ENTERKEY;

}

void Cdisp::M716_jueyuanzd()
{
	int flag;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
	}
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case ENTERKEY://up
				pwinmenu->row++;
				if( flag == 0)
				{
					Returntopmenu();
					pwinmenu->page = 0;
					addalarmmenu(&Cdisp::Mmeter_success, nomal_alarm, nomal_alarm);
				}
				break;
		}

	}
	WriteDispBuff("绝缘整定",0,0,16);
	WriteDispBuff("源加好后按确认",2,0,16);
	newmouse.num = 0;
	MASK = ENTERKEY|UPKEY|DOWNKEY;
}
void Cdisp::seteqpip2()
{
	BYTE charaddr[][2]={
		{1,8},
		{1,9},			
		{1,10},			
		{1,12},			
		{1,13},		
		{1,14},
		{1,16},			
		{1,17},			
		{1,18},
		{1,20},			
		{1,21},			
		{1,22},				
	};
	BYTE i,j;
	char row0[]="以太网2设置:";
	BYTE net[4];
	char ip[20];
	m_wrrdfalg = 1;
	MASK  = 0x4F|ADDKEY|SUBBKEY;
	if(m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]] == 0x20)
	{
		i = 0;
		j = 0;
		net[0] = net[1] = net[2] = net[3] = 0;
		for(j = 0; j < 4; j++)
		{
			while ((g_Sys.AddrInfo.Lan2.sIP[i] >= '0') && (g_Sys.AddrInfo.Lan2.sIP[i] <= '9'))
			{
				net[j] *= 10;
				net[j] += (g_Sys.AddrInfo.Lan2.sIP[i] & 0xf);
				i++;
			}
			i++;
		}
		sprintf((char*)tt,"%03d.%03d.%03d.%03d",net[0],net[1],net[2],net[3]);
		WriteDispBuff(tt,charaddr[0][0],charaddr[0][1],15);
		m_setkeyvalue = 0;
	}
	WriteDispBuff(row0,0,0,16);
	if(m_setkeyvalue > 200) m_setkeyvalue = (sizeof(charaddr)/sizeof(charaddr[0]))-1;
	else if(m_setkeyvalue >= (sizeof(charaddr)/sizeof(charaddr[0])))
		m_setkeyvalue = 0;

	newmouse.x = charaddr[m_setkeyvalue][0];
	newmouse.y = charaddr[m_setkeyvalue][1];
	newmouse.num = 1;
	if(m_pwd_flag == 1)
	{
		m_pwd_flag = 0;
		sprintf(ip,"%d.%d.%d.%d",m_conword[0],m_conword[1],m_conword[2],m_conword[3]);
		m_wrrdfalg = 0; 
		SetSysAddr(NULL,NULL,ip,NULL,NULL) ;
		Returntopmenu();
		pwinmenu->page = 0;
		addalarmmenu(&Cdisp::Mset_success, nomal_alarm, nomal_alarm);
		return;
	}	
	if(m_keybak != m_keyval)
	{
		if(m_setkeyvalue+1 < (sizeof(charaddr)/sizeof(charaddr[0]))||(m_keyval != 0x40))
			setdecnum(m_dispbuf+charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]);
		else
		{
			for(i = 0; i < 4; i++)		
				m_conword[i] = (BYTE)getdispdata(m_dispbuf+charaddr[i*3][0]*maxcol+charaddr[i*3][1],3,0);
		
			Downmenu();
		}
	}
	m_keybak = m_keyval;

}
void Cdisp::dispsetclock()
{
	#define offset 8
	BYTE charaddr[][2]={
		{1,0+offset},
		{1,1+offset},			
		{1,2+offset},			
		{1,3+offset},			
		{1,5+offset},		
		{1,6+offset},
		{1,8+offset},			
		{1,9+offset},			
		{2,2+offset},
		{2,3+offset},
		{2,5+offset},
		{2,6+offset},
		{2,8+offset},
		{2,9+offset}
	};
	char row0[]="时钟设置:";
	VSysClock*  pclock = (VSysClock*)sYKNameTab;
	m_wrrdfalg = 1;
	MASK = 0x4f|ADDKEY|SUBBKEY;
	if(m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]] == 0x20)
	{
		GetSysClock(&clock,SYSCLOCK);
		m_setkeyvalue=0;
		sprintf((char*)tt,"%04d-%02d-%02d",clock.wYear,clock.byMonth,clock.byDay);
		WriteDispBuff(tt,charaddr[0][0],charaddr[0][1],15);
		sprintf((char*)tt,"%02d:%02d:%02d",clock.byHour,clock.byMinute,clock.bySecond);
		WriteDispBuff(tt,charaddr[8][0],charaddr[8][1],15);
		m_setkeyvalue=0;
	}
	WriteDispBuff(row0,0,0,10);
	if(m_setkeyvalue > 200) m_setkeyvalue = (sizeof(charaddr)/sizeof(charaddr[0]))-1;
	else if(m_setkeyvalue >= ( sizeof(charaddr)/sizeof(charaddr[0])))
		m_setkeyvalue = 0;

	newmouse.x = charaddr[m_setkeyvalue][0];
	newmouse.y = charaddr[m_setkeyvalue][1];
	newmouse.num = 1;
	if(m_pwd_flag == 1)
	{
		m_pwd_flag = 0;
		SetSysClock(pclock,SYSCLOCK);
		m_wrrdfalg = 0;
		Returntopmenu();
		pwinmenu->page = 0;
		addalarmmenu(&Cdisp::Mset_success, nomal_alarm, nomal_alarm);
		return;
	}	
	if(m_keybak != m_keyval)
	{
		if(m_setkeyvalue + 1 < (sizeof(charaddr)/sizeof(charaddr[0]))||(m_keyval != 0x40))
			setdecnum(m_dispbuf+charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]);
		else
		{
			pclock->wYear = getdispdata(m_dispbuf+charaddr[0][0]*maxcol+charaddr[0][1],4,0);
			pclock->byMonth = getdispdata(m_dispbuf+charaddr[4][0]*maxcol+charaddr[4][1],2,0);
			pclock->byDay = getdispdata(m_dispbuf+charaddr[6][0]*maxcol+charaddr[6][1],2,0);
			pclock->byHour = getdispdata(m_dispbuf+charaddr[8][0]*maxcol+charaddr[8][1],2,0);
			pclock->byMinute = getdispdata(m_dispbuf+charaddr[10][0]*maxcol+charaddr[10][1],2,0);
			pclock->bySecond = getdispdata(m_dispbuf+charaddr[12][0]*maxcol+charaddr[12][1],2,0);
			pclock->wMSecond = 0;
			Downmenu();
		}
	}
	m_keybak = m_keyval;

}
void Cdisp::M51_setpara()
{
	char row0[]="(6)终端配置";
	char row1[]="1.装置地址";
	char row2[]="2.以太网地址1";
	char row3[]="3.以太网地址2";
	char row4[]="4.时      钟";
	WriteDispBuff(row0,0,0,16);
	WriteDispBuff(row1,1,0,16);
	WriteDispBuff(row2,2,0,16);
	WriteDispBuff(row3,3,0,16);
	WriteDispBuff(row4,4,0,16);

   	newmouse.y = 0;
	
	newmouse.x = (pwinmenu->para - 1) % maxdatarow + 1;
	newmouse.num = 13;
	MASK = 0;
}

void Cdisp::M521_selectline()
{
	newmouse.num = 0;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
		m_key_temp = 20;
		m_keyleftrightval = 0;
		if(pwinmenu->key == QUITKEY)
		{
			Returntopmenu();
			return;
		}
	}
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		WriteDispBuff("无回线",2,2,18);
		Returntopmenu();
		return;
	}
	dealupdownvalue(g_Sys.MyCfg.wFDNum+1,1);
	if(m_key_temp != m_keyupdownval)
	{
		m_key_temp = m_keyupdownval;
		if(m_keyupdownval == 0)
		{
			pro_init(0);
			sprintf((char*)tt,"公共定值");
		}
		else
		{
			pro_init(1);
			if(m_keyupdownval < g_Sys.MyCfg.wFDNum+1)
				sprintf((char*)tt," %s",delchar(g_Sys.MyCfg.pFd[m_keyupdownval-1].kgname));
		}
	}
	
	MASK=ENTERKEY|LEFTKEY|RIGHTKEY;
	WriteDispBuff("保护定值设置:",0,0,16);

	if(!strcmp(tt, "公共定值"))
		pubval = 1;
	else
		pubval = 2;

	WriteDispBuff(tt,2,0,16);
	WriteDispBuff("回线名称:",1,0,16);
	WriteDispBuff("<>选回线",3,3,16);
	newmouse.x = 2;
	newmouse.num = 8;
	newmouse.y = 0;
	if(m_keybak != m_keyval)
	{
		if(m_keyval == ENTERKEY)
		{
			pwinmenu->page = m_keyupdownval;
			if(m_keyupdownval == 0)
			{
				if(m_proset)
					ReadSet(0,(BYTE*)pproset);
			}
			else
			{
				if(m_proset)
					ReadSet(m_keyupdownval,(BYTE*)pproset);
			}
			m_pwd_flag = 0;
				
			Entersonmenu();
			pwinmenu->page = m_keyupdownval;
		}
	}
		
}


void Cdisp::M911_selectline()
{
	newmouse.num = 0;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
		m_key_temp = 20;
		m_keyleftrightval = 0;
		if(pwinmenu->key == QUITKEY)
		{
			Returntopmenu();
			return;
		}
	}
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		WriteDispBuff("无回线",2,2,18);
		Returntopmenu();
		return;
	}
	
	dealupdownvalue(g_Sys.MyCfg.wFDNum + 1,1);
	if(m_key_temp != m_keyupdownval)
	{
		m_key_temp = m_keyupdownval;
		if(m_keyupdownval == 0)
		{
			pro_init(0);
			sprintf((char*)tt,"公共定值");
		}
		else
		{
			pro_init(1);
			if(m_keyupdownval < g_Sys.MyCfg.wFDNum + 1)
				sprintf((char*)tt," %s",delchar(g_Sys.MyCfg.pFd[m_keyupdownval-1].kgname));
		}
	}
	MASK = ENTERKEY|LEFTKEY|RIGHTKEY;
	WriteDispBuff("保护定值查看:",0,0,16);
	if(!strcmp(tt, "公共定值"))
		pubval = 1;
	else
		pubval = 2;
	WriteDispBuff(tt,2,0,16);
	WriteDispBuff("回线名称:",1,0,16);
	WriteDispBuff("<>选回线",3,3,16);
	newmouse.x = 2;
	newmouse.num = 8;
	newmouse.y = 0;
	if(m_keybak != m_keyval)
	{
		if(m_keyval == ENTERKEY)
		{
			pwinmenu->page = m_keyupdownval;
			if(m_keyupdownval == 0)
			{
				if(m_proset)
					ReadSet(0,(BYTE*)pproset);
			}
			else
			{
				if(m_proset)
					ReadSet(m_keyupdownval,(BYTE*)pproset);
			}
			m_pwd_flag = 0;
				
			Entersonmenu();
			pwinmenu->page = m_keyupdownval;
		}
	}
		
}

void Cdisp::M931_selectline()
{
	newmouse.num = 0;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 0;
		m_keyleftrightval = 0;
		if(pwinmenu->key == QUITKEY)
		{
			Returntopmenu();
			return;
		}
	}
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		WriteDispBuff("无回线",2,2,18);
		Returntopmenu();
		return;
	}
	dealupdownvalue(g_Sys.MyCfg.wFDNum+1,1);
	MASK=ENTERKEY|LEFTKEY|RIGHTKEY;
	WriteDispBuff("回线参数设置:",0,0,16);
	if(m_keyupdownval < g_Sys.MyCfg.wFDNum)
		sprintf((char*)tt,"%s",delchar(g_Sys.MyCfg.pFd[m_keyupdownval].kgname));
	else
		sprintf((char*)tt," 返回");
		
	WriteDispBuff(tt,2,0,16);
	WriteDispBuff("回线名称:",1,0,16);
	WriteDispBuff("<>选回线",3,3,16);
	newmouse.x = 2;
	newmouse.num = 6;
	newmouse.y = 0;
	if(m_keybak != m_keyval)
	{

		if(m_keyval == ENTERKEY)
		{
			if(m_keyupdownval >= g_Sys.MyCfg.wFDNum)
			{
				Returntopmenu();
				return;
			}
			pwinmenu->page = m_keyupdownval;
			getlinpara(m_keyupdownval);
			m_pwd_flag = 0;
			Entersonmenu();
			pwinmenu->page = m_keyupdownval;
		}
	}

}
void Cdisp::M932_looklinepara()
{
	BYTE flag;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_keyupdownval = 1;
		m_keyleftrightval = 0;
	}
	WriteDispBuff("回线参数                        ",0,0,16);
	if(m_keyupdownval < 5)
	{
		sprintf((char*)tt,"PT    %06d:%03d                                   ",pt[0],pt[1]);
		WriteDispBuff(tt,1,0,16);
		sprintf((char*)tt,"CT    %06d:%03d                                ",pt[2],pt[3]);
		WriteDispBuff(tt,2,0,16);
		sprintf((char*)tt,"CT0   %06d:%03d                                ",pt[4],pt[5]);
		WriteDispBuff(tt,3,0,16);
		sprintf((char*)tt,"退出/保存 ");
		WriteDispBuff(tt,4,0,16);
	}
	
	if(m_keybak != m_keyval)
	{
			
		switch(m_keyval)
		{
			case QUITKEY:
				flag = 0;
				if(g_Sys.MyCfg.pFd[pwinmenu->page].pt != pt[0])
					flag = 1;
				if(g_Sys.MyCfg.pFd[pwinmenu->page].Un != pt[1])
					flag =1;
				if(g_Sys.MyCfg.pFd[pwinmenu->page].ct != pt[2])
					flag = 1;
				if(g_Sys.MyCfg.pFd[pwinmenu->page].In != pt[3])
					flag = 1;
				if(g_Sys.MyCfg.pFd[pwinmenu->page].ct0 != pt[4])
					flag = 1;
				if(g_Sys.MyCfg.pFd[pwinmenu->page].In0 != pt[5])
					flag = 1;
				if(flag)
					Downmenu();
				else
				{
					Returntopmenu();
					return;
				}
				break;
			case UPKEY:  //up
			case LEFTKEY:  //up
				m_keyupdownval--;
				if(m_keyupdownval < 1)
				m_keyupdownval = 4;
				break;
			case DOWNKEY:  //down
			case RIGHTKEY:  //down
				m_keyupdownval++;
				if(m_keyupdownval > 4)
				m_keyupdownval = 1;
				break;
			case ENTERKEY:  //down
				if(m_keyupdownval < 4)
				{
					Entersonmenu();
					pwinmenu->para = m_keyupdownval;
				}
				else
				{
					flag = 0;
					if(g_Sys.MyCfg.pFd[pwinmenu->page].pt != pt[0])
						flag = 1;
					if(g_Sys.MyCfg.pFd[pwinmenu->page].Un != pt[1])
						flag = 1;
					if(g_Sys.MyCfg.pFd[pwinmenu->page].ct != pt[2])
						flag = 1;
					if(g_Sys.MyCfg.pFd[pwinmenu->page].In != pt[3])
						flag = 1;
					if(g_Sys.MyCfg.pFd[pwinmenu->page].ct0 != pt[4])
						flag = 1;
					if(g_Sys.MyCfg.pFd[pwinmenu->page].In0 != pt[5])
						flag = 1;
					if(flag)
						Downmenu();
					else
					{
						Returntopmenu();
						return;
					}
				}
				break;
		}
				
	}
    newmouse.y = 0;
	newmouse.x = (m_keyupdownval - 1) % maxdatarow + 1;
	newmouse.num = 16;
	MASK = QUITKEY|UPKEY|DOWNKEY|ENTERKEY;
	if(m_pwd_flag == 1)
	{
		m_pwd_flag = 0;
		Setlinpara(pwinmenu->page);
		Returntopmenu();
		Returntopmenu();
		pwinmenu->page = 0;
		addalarmmenu(&Cdisp::Mset_success_and_reset, nomal_alarm, nomal_alarm);
	}	
}
void Cdisp::getlinpara(BYTE line)
{
	pt[0] = g_Sys.MyCfg.pFd[line].pt;
	pt[1] = g_Sys.MyCfg.pFd[line].Un;
	pt[2] = g_Sys.MyCfg.pFd[line].ct;
	pt[3] = g_Sys.MyCfg.pFd[line].In;
	pt[4] = g_Sys.MyCfg.pFd[line].ct0;
	pt[5] = g_Sys.MyCfg.pFd[line].In0;
}
void Cdisp::Setlinpara(BYTE line)
{
	g_Sys.MyCfg.pFd[line].pt = pt[0];
	g_Sys.MyCfg.pFd[line].Un = pt[1];
	g_Sys.MyCfg.pFd[line].ct = pt[2];
	g_Sys.MyCfg.pFd[line].In = pt[3];
	g_Sys.MyCfg.pFd[line].ct0 = pt[4];
	g_Sys.MyCfg.pFd[line].In0 = pt[5];
	SavePtCt(line,pt,pt+1,pt+2,pt+3,pt+4,pt+5);		
}
void Cdisp::M9311_setlinepara()
{
	switch(pwinmenu->para)
	{
		case 1:
			M9311_setPT();
			break;
		case 2:
			M9311_setCT();
			break;
		case 3:
			M9311_setCT0();
			break;

	}
}
void Cdisp::M9311_setPT()
{
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ENTERKEY|ADDKEY|SUBBKEY;
	BYTE charaddr[][2]={
		{2,4},
		{2,5},			
		{2,6},			
		{2,7},
		{2,8},			
		{2,9},			
		{2,11},			
		{2,12},			
		{2,13},			
	};

	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		m_setmouse.x = 0;
		m_setmouse.y = 0;
		m_setmouse.num = 1;
		pwinmenu->row = pwinmenu->para;
		m_setkeyvalue = 0;
		pwinmenu->page = 0;
		clearscreen();

		sprintf((char*)tt,"%06d:%03d                                   ",pt[0],pt[1]);

		WriteDispBuff(tt,2,4,12);	
	}
	
	if(m_keyval != m_keybak)
	{
		switch(m_keyval)
		{
			case UPKEY:
			case DOWNKEY:
			case LEFTKEY:
			case RIGHTKEY:
			case ADDKEY:
			case SUBBKEY:
				setdecnum(m_dispbuf+charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]);
				break;
			case ENTERKEY:
				m_setkeyvalue++;
				if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
				{
					pt[0]=(DWORD)getdispdata(m_dispbuf+charaddr[0][0]*maxcol+charaddr[0][1],6,0);
					pt[1]=(DWORD)getdispdata(m_dispbuf+charaddr[6][0]*maxcol+charaddr[6][1],3,0);
					Returntopmenu();
					return;
				}
				break;
		}

	}
	sprintf((char*)tt,"PT变比");
	WriteDispBuff(tt,0,0,15);

	newmouse.x = 2;
	newmouse.num = m_setmouse.num;
	if(m_setkeyvalue <= 0)
		m_setkeyvalue = 0;
	if(m_setkeyvalue >= 9)
		m_setkeyvalue = 8;
	newmouse.y = charaddr[m_setkeyvalue][1];
		

}
void Cdisp::M9311_setCT()
{
	MASK=UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ENTERKEY|ADDKEY|SUBBKEY;
	BYTE charaddr[][2]={
		{2,4},
		{2,5},			
		{2,6},			
		{2,7},
		{2,8},			
		{2,9},			
		{2,11},			
		{2,12},			
		{2,13},			
	};

	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		m_setmouse.x = 0;
		m_setmouse.y = 0;
		m_setmouse.num = 1;
		pwinmenu->row = pwinmenu->para;
		m_setkeyvalue = 0;
		pwinmenu->page = 0;
 		clearscreen();

		sprintf((char*)tt,"%06d:%03d                                   ",pt[2],pt[3]);

		WriteDispBuff(tt,2,4,12);	
	}
	if(m_keyval != m_keybak)
	{
		switch(m_keyval)
		{
			case UPKEY:
			case DOWNKEY:
			case LEFTKEY:
			case RIGHTKEY:
			case ADDKEY:
			case SUBBKEY:
				setdecnum(m_dispbuf+charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]);
				break;
			case ENTERKEY:
				m_setkeyvalue++;
				if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
				{
					pt[2] = (DWORD)getdispdata(m_dispbuf+charaddr[0][0]*maxcol+charaddr[0][1],6,0);
					pt[3] = (DWORD)getdispdata(m_dispbuf+charaddr[6][0]*maxcol+charaddr[6][1],3,0);
					Returntopmenu();
					return;
				}
				break;
							

		}

	}
	sprintf((char*)tt,"CT变比");
	WriteDispBuff(tt,0,0,15);

	newmouse.x = 2;
	newmouse.num = m_setmouse.num;
	if(m_setkeyvalue <= 0)
		m_setkeyvalue = 0;
	if(m_setkeyvalue >= 9)
		m_setkeyvalue = 8;
	newmouse.y = charaddr[m_setkeyvalue][1];
	

}
void Cdisp::M9311_setCT0()
{
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ENTERKEY|ADDKEY|SUBBKEY;
	BYTE charaddr[][2]={
		{2,4},
		{2,5},			
		{2,6},			
		{2,7},
		{2,8},			
		{2,9},			
		{2,11},			
		{2,12},			
		{2,13},			
	};

	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		m_setmouse.x = 0;
		m_setmouse.y = 0;
		m_setmouse.num = 1;
		pwinmenu->row = pwinmenu->para;
		m_setkeyvalue = 0;
		pwinmenu->page = 0;
		clearscreen();

		sprintf((char*)tt,"%06d:%03d                                   ",pt[4],pt[5]);

		WriteDispBuff(tt,2,4,12);	
	}
	if(m_keyval != m_keybak)
	{
		switch(m_keyval)
		{
			case UPKEY:
			case DOWNKEY:
			case LEFTKEY:
			case RIGHTKEY:
			case ADDKEY:
			case SUBBKEY:
				setdecnum(m_dispbuf+charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]);
				break;
			case ENTERKEY:
				m_setkeyvalue++;
				if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
				{
					pt[4] = (DWORD)getdispdata(m_dispbuf+charaddr[0][0]*maxcol+charaddr[0][1],6,0);
					pt[5] = (DWORD)getdispdata(m_dispbuf+charaddr[6][0]*maxcol+charaddr[6][1],3,0);
					Returntopmenu();
					return;
				}
				break;
		}

	}
	sprintf((char*)tt,"零序变比");
	WriteDispBuff(tt,0,0,15);
	
	newmouse.x = 2;
	newmouse.num = m_setmouse.num;
	if(m_setkeyvalue <= 0)
		m_setkeyvalue = 0;
	if(m_setkeyvalue >= 9)
		m_setkeyvalue = 8;
	newmouse.y = charaddr[m_setkeyvalue][1];
		

}
				
void Cdisp::listdowneqp()
{
	MASK = 0x3f;
	if(pwinmenu->flag == 0x55)
	{
		if(pwinmenu->key == ENTERKEY)
		{
			pwinmenu->flag = 0x66;
			pwinmenu->row = 0;
			geteqpid();
			m_keyupdownval = 0;
			pgdneqp();
		}
	}
	listeqp_1();			
}

void Cdisp::listupeqp()
{
	MASK = 0x3f;
	if(pwinmenu->flag == 0x55)
	{
		if(pwinmenu->key == ENTERKEY)
		{
			pwinmenu->flag = 0x66;
			pwinmenu->row = 0;
			m_keyupdownval = 0;
			pgdneqp();
		}
	}
	listeqp_1();		
}

void Cdisp::listeqp_1()
{
	char row0[] = "请选择装置:";
	int i;
	MASK = LEFTKEY|RIGHTKEY|UPKEY|DOWNKEY|ENTERKEY;
	m_keyupdownval = pwinmenu->row;
	if(m_keybak != m_keyval)
	{
		if(m_keyval)
			clearscreen();
		switch(m_keyval)
		{
			case 1://up
				if(m_keyupdownval == 0)
					m_keyupdownval++;
				m_keyupdownval--;
				break;
			case 2://down
				if(m_keyupdownval == 3)
					pgdneqp();
				m_keyupdownval++;
				break;
			case LEFTKEY://pgup
				m_keyupdownval=0;
				pgdneqp();
				break;
			case RIGHTKEY://pgdown
				m_keyupdownval=0;
				pgdneqp();
				break;
			case ENTERKEY://pgdown
				if(m_pEqpInfo != NULL)
					Entersonmenu();
				else
					Returntopmenu();
				return;
		}
				
	}
	if(m_keyupdownval > 250) m_keyupdownval = 2;
	else if(m_keyupdownval > 3)
		m_keyupdownval = 0;	
	if(m_keyval == 1)
	{
		while((m_pcurdispeqp[m_keyupdownval] == NULL) && (m_keyupdownval != 0))
			m_keyupdownval--;
	}
	if(m_keyval == 2)
	{
		if((m_pcurdispeqp[m_keyupdownval] == NULL))
		{
			if((m_keyupdownval == 0) || ( m_pcurdispeqp[m_keyupdownval-1] == NULL))
			{
				m_keyupdownval = 0;
				pgdneqp();
			}
		}
	}
	WriteDispBuff(row0,0,1,16);
	for(i = 0; i < 4; i++)
	{
		if(m_pcurdispeqp[i] != NULL)
		{
			sprintf((char*)tt,"%s",m_pcurdispeqp[i]->sCfgName);
			WriteDispBuff(tt,i+1,10,16);
		}
		else
		{
			WriteDispBuff("返回",i+1,10,16);
			break;
		}
	}
	newmouse.x = m_keyupdownval + 1;
	pwinmenu->row = m_keyupdownval;
	newmouse.y = 10;
	newmouse.num = 6;
	m_pEqpInfo = m_pcurdispeqp[m_keyupdownval];
	
			
}
void Cdisp::pgupeqp()
{
	int i;
	if(m_eqpid[0] == 0)
		return;
	if(m_cureqpid < 4)
	{
		if(m_eqpid[0] > 4)
			m_cureqpid = m_eqpid[0]-4;
		else
			m_cureqpid = 0;
	}
	else
		m_cureqpid -= 4;
			
	if(m_cureqpid > 250)
		m_cureqpid = 0;
	for(i = 0; i < 4; i++)
	{
		if(m_cureqpid + i < m_eqpid[0])
			m_pcurdispeqp[i] = &g_Sys.Eqp.pInfo[m_eqpid[m_cureqpid+1+i]];
		else
			m_pcurdispeqp[i] = NULL;
	}
			
}
void Cdisp::pgdneqp()
{
	int i;
	if(m_eqpid[0] == 0)
		return;
	if(m_pcurdispeqp[0] == NULL) 
	{
		m_cureqpid = 0;
	}
	else
	{
		m_cureqpid += 4;
		if(m_cureqpid >= m_eqpid[0])
			m_cureqpid = 0;
	}
	for(i = 0; i < 4;i++)
	{
		if(m_cureqpid + i < m_eqpid[0])
			m_pcurdispeqp[i] = &g_Sys.Eqp.pInfo[m_eqpid[m_cureqpid+1+i]];
		else
			m_pcurdispeqp[i] = NULL;
	}
			
}


void Cdisp::set_menu()
{
	m_key_temp = 20;
	m_keyupdownval = 0;
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ADDKEY|SUBBKEY|ENTERKEY|QUITKEY;
	BYTE charaddr[][2] = {
		{4,0},
		{4,4},			
		{4,8},			
		{4,12}			
	};
	DWORD tmpdataH;
	WORD tmpdataL;
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		
		m_setkeyvalue = 1;
		pwinmenu->page = 0;
		readYB();
		readcon();
 	}
	m_setmouse.num = 4;
	if(m_keyval != m_keybak)
	{
		clearscreen();
		switch(m_keyval)
		{
			case UPKEY:
			case DOWNKEY:
			case LEFTKEY:
				m_setkeyvalue--;
				if((m_proset[pwinmenu->row].bySetAttrib>>4) == SETOPT_READ)
					if(m_setkeyvalue == 2)
						m_setkeyvalue--;
				break;
			case RIGHTKEY:
				m_setkeyvalue++;
				if((m_proset[pwinmenu->row].bySetAttrib>>4) == SETOPT_READ)
					if(m_setkeyvalue == 2)
						m_setkeyvalue++;
				break;
			case ENTERKEY:
				switch(m_setkeyvalue)
				{
					case 0://上页
						if(pwinmenu->row == 0)
							pwinmenu->row = m_setnum;
						if(pwinmenu->row != 0)
							pwinmenu->row--;
				
						while((m_proset[pwinmenu->row].bySetAttrib >> 4) == SETOPT_HIDE)
						{
							if(pwinmenu->row == 0)
								pwinmenu->row = m_setnum;
							if(pwinmenu->row != 0)
								pwinmenu->row--;
						}
						break;
					case 1://下页
						pwinmenu->row++;
						if(pwinmenu->row >= m_setnum)
							pwinmenu->row = 0;
						while((m_proset[pwinmenu->row].bySetAttrib >> 4) == SETOPT_HIDE)
						{
							pwinmenu->row++;
							if(pwinmenu->row >= m_setnum)
								pwinmenu->row = 0;
						}
						break;
					case 2://修改
						tmpdataL = pwinmenu->row;
						tmpdataH = pwinmenu->topmenu->page;
						Entersonmenu();
						pwinmenu->para = tmpdataL;
						pwinmenu->page = tmpdataH;
						break;
					case 3://返回
						if(pwinmenu->page)
							Downmenu();
						else
						{
							Returntopmenu();
							return;
						}
						break;
					default:
						m_setkeyvalue = 1;
						break;
					}
					break;
				case QUITKEY:
					if(pwinmenu->page)
						Downmenu();
					else
					{
						Returntopmenu();
						return;
					}
					break;				
				}

			}
		if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
			m_setkeyvalue  = 0;
		sprintf((char*)tt,"定值整定");
		WriteDispBuff(tt,0,0,16);
		WriteDispBuff("上页下页修改保存",4,0,16);
		
		WriteDispBuff(m_set_miaoshu[pwinmenu->row].szName,1,0,16);
		switch(m_proset[pwinmenu->row].bySetAttrib & 0xf)
		{
			case SETTYPE_YB:
			case SETTYPE_KG:
				sprintf((char*)tt,"%02x%02x%02x%02x",m_proset[pwinmenu->row].byValHih,m_proset[pwinmenu->row].byValHil,m_proset[pwinmenu->row].byValLoh,m_proset[pwinmenu->row].byValLol);
				WriteDispBuff(tt,2,2,8);		
				break;
			case SETTYPE_IP:
				sprintf((char*)tt,"%03d.%03d.%03d.%03d",m_proset[pwinmenu->row].byValHih,m_proset[pwinmenu->row].byValHil,m_proset[pwinmenu->row].byValLoh,m_proset[pwinmenu->row].byValLol);
				WriteDispBuff(tt,2,1,15);
				break;
			default:
				tmpdataH = m_proset[pwinmenu->row].byValHih << 8;
				tmpdataH |= m_proset[pwinmenu->row].byValHil;
				tmpdataH <<= 8;
				tmpdataH |= m_proset[pwinmenu->row].byValLoh;
				tmpdataH <<= 8;
				tmpdataH |= m_proset[pwinmenu->row].byValLol;
				switch(m_proset[pwinmenu->row].byValAttrib & 3)
				{
					case 1:
						tmpdataL = tmpdataH & 0xf;
						tmpdataH >>= 4;
						tmpdataL <<= 8;
						break;
					case 2:
						tmpdataL = tmpdataH & 0xff;
						tmpdataH >>= 8;
						tmpdataL <<= 4;
						break;
					case 3:
						tmpdataL = tmpdataH & 0xfff;
						tmpdataH >>= 12;
						break;
					default:
						tmpdataL = 0;
						break;
				}
				if(m_proset[pwinmenu->row].bySetAttrib == 0x2D)
					//sprintf((char*)tt,"%x%03x",tmpdataH,tmpdataL);
					sprintf((char*)tt,"%x",tmpdataH);
					
				else
					sprintf((char*)tt,"%x.%03x",tmpdataH,tmpdataL);
				
				WriteDispBuff(tt,2,4,12);			
				break;		
		}
		newmouse.x = charaddr[m_setkeyvalue][0];
		newmouse.num = m_setmouse.num;
		newmouse.y = charaddr[m_setkeyvalue][1];
		if(m_pwd_flag == 1)
		{
			m_pwd_flag = 0;
			if(pwinmenu->topmenu->page == 0)
			{
				if(WriteSet(0, (BYTE*)pproset) == OK)
					WriteDoEvent(NULL, 0, "液晶修改保护定值!");
				else
					WriteDoEvent(NULL, 0, "液晶修改保护定值失败!");
			}
			else
			{
				if(WriteSet(pwinmenu->topmenu->page, (BYTE*)pproset) == OK)
					WriteDoEvent(NULL, 0, "液晶修改保护定值!");
				else
					WriteDoEvent(NULL, 0, "液晶修改保护定值失败!");
			}
			Returntopmenu();
			Returntopmenu();
			pwinmenu->page = 0;
			addalarmmenu(&Cdisp::Mset_success, nomal_alarm, nomal_alarm);
		}

}
void Cdisp::set_pro()
{
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ADDKEY|SUBBKEY|ENTERKEY;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->row = pwinmenu->para;
	}
	switch(m_proset[pwinmenu->row].bySetAttrib & 0xf)
	{
		case SETTYPE_YB:
			set_proYB();
			break;
		case SETTYPE_KG:
			set_proCON1();
			break;
		case SETTYPE_IP:
			set_proIP();
			break;
		default:
			set_proset();				
			break;
			
	}

}

void Cdisp::set_proset()
{
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ADDKEY|SUBBKEY|ENTERKEY;
	BYTE charaddr[][2]={
		{2,4},
		{2,5},
		{2,6},
		{2,7},
		{2,8},
		{2,9},			
		{2,10},			
		{2,11},
		{2,13},			
		{2,14},			
		{2,15},			
	};
	DWORD tmpdataH;
	WORD tmpdataL;
	WORD i;
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->topmenu->page = 1;
		m_setmouse.x = 0;
		m_setmouse.y = 0;
		m_setmouse.num = 1;
		pwinmenu->row = pwinmenu->para;
		m_setkeyvalue = 0;
		pwinmenu->page = 0;
 		clearscreen();
		
		switch(m_proset[pwinmenu->row].bySetAttrib & 0xf)
		{
			case SETTYPE_YB:
			case SETTYPE_KG:
				break;
			case SETTYPE_IP:
				break;
			default:
				tmpdataH = m_proset[pwinmenu->row].byValHih;
					tmpdataH <<= 8;
				tmpdataH |= m_proset[pwinmenu->row].byValHil;
					tmpdataH <<= 8;
				tmpdataH |= m_proset[pwinmenu->row].byValLoh;
					tmpdataH <<= 8;
				tmpdataH |= m_proset[pwinmenu->row].byValLol;
				switch(m_proset[pwinmenu->row].byValAttrib & 3)
				{
					case 1:
						tmpdataL = tmpdataH & 0xf;
						tmpdataH >>= 4;
						tmpdataL <<= 8;
						break;
					case 2:
						tmpdataL = tmpdataH & 0xff;
						tmpdataH >>= 8;
						tmpdataL <<= 4;
						break;
					case 3:
						tmpdataL = tmpdataH & 0xfff;
						tmpdataH >>= 12;
						break;
					default:
						tmpdataL = 0;
						break;
				}	
				if(m_proset[pwinmenu->row].bySetAttrib == 0x2D)
					//sprintf((char*)tt,"%08x%04x",tmpdataH,tmpdataL);
					sprintf((char*)tt,"%08x",tmpdataH);
				else
					sprintf((char*)tt,"%08x.%03x",tmpdataH,tmpdataL);
				break;
					
		}
		WriteDispBuff(tt,2,4,12);	
	}
		if(m_keyval != m_keybak)
		{
			switch(m_keyval)
			{
				case UPKEY:
				case ADDKEY:
				case DOWNKEY:
				case SUBBKEY:
				case LEFTKEY:
				case RIGHTKEY:
					setdecnum(m_dispbuf+charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]);
					break;
				case ENTERKEY:
					m_setkeyvalue++;
					if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
					{
						tmpdataL = Get_zhengshu(m_dispbuf+charaddr[0][0]*maxcol+charaddr[0][1],8);
					
						tmpdataH = 0;
						for(i = 0;i < 8 - tmpdataL; i++)
						{
							tmpdataH <<= 4;
							tmpdataH |= m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]+i+tmpdataL] & 0xf;
						}
						for(i = 0;(i < tmpdataL) && (i < 3); i++)
						{
							tmpdataH <<= 4;
							tmpdataH |= m_dispbuf[charaddr[8][0]*maxcol+charaddr[8][1]+i] & 0xf;
						}
						/*if(m_proset[pwinmenu->row].bySetAttrib == 0x2D)
						{
							if(((tmpdataH & 0x0f) - 1) >= g_Sys.MyCfg.wFDNum)
							{
								Returntopmenu();
								return;
							}
						}*/
						m_proset[pwinmenu->row].byValHih = tmpdataH >> 24;
						m_proset[pwinmenu->row].byValHil = tmpdataH >> 16;
						m_proset[pwinmenu->row].byValLoh = tmpdataH >> 8;
						m_proset[pwinmenu->row].byValLol = tmpdataH;
						m_proset[pwinmenu->row].byValAttrib &= ~3;
						m_proset[pwinmenu->row].byValAttrib |= i;
						
						Returntopmenu();
						return;  
					}
					break;
				}

		}
		sprintf((char*)tt,"保护定值");
		WriteDispBuff(tt,0,0,15);
		
		WriteDispBuff(m_set_miaoshu[pwinmenu->row].szName,1,0,16);
		
		newmouse.x = 2;
		newmouse.num = m_setmouse.num;
		if(m_setkeyvalue <= 0)
			m_setkeyvalue = 11;
		if(m_setkeyvalue >= 11)
			m_setkeyvalue = 0;
		newmouse.y = charaddr[m_setkeyvalue][1];
		

}
void Cdisp::set_proIP()
{
	MASK=UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ADDKEY|SUBBKEY|ENTERKEY;
	BYTE charaddr[][2]={
		{2,1},
		{2,2},			
		{2,3},			
		{2,5},
		{2,6},			
		{2,7},			
		{2,9},			
		{2,10},			
		{2,11},			
		{2,13},			
		{2,14},			
		{2,15},			
	};
	WORD tmpdataH;
	WORD tmpdataL;
	WORD i;
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->topmenu->page = 1;
		m_setmouse.x = 0;
		m_setmouse.y = 0;
		m_setmouse.num = 1;
		pwinmenu->row = pwinmenu->para;
		m_setkeyvalue = 0;
		pwinmenu->page = 0;
 		clearscreen();
		switch(m_proset[pwinmenu->row].bySetAttrib & 0xf)
		{
			case SETTYPE_YB:
			case SETTYPE_KG:
				break;
			case SETTYPE_IP:
				sprintf((char*)tt,"%03d.%03d.%03d.%03d",m_proset[pwinmenu->row].byValHih,m_proset[pwinmenu->row].byValHil,
				m_proset[pwinmenu->row].byValLoh,m_proset[pwinmenu->row].byValLol);
				break;
			default:							
				break;
				
		}
		WriteDispBuff(tt,2,1,15);	
	}
	if(m_keyval != m_keybak)
	{
		switch(m_keyval)
		{
			case UPKEY:
			case ADDKEY:
			case DOWNKEY:
			case SUBBKEY:
			case LEFTKEY:
			case RIGHTKEY:
				setdecnum(m_dispbuf+charaddr[m_setkeyvalue][0]*maxcol+charaddr[m_setkeyvalue][1]);
				break;
			case ENTERKEY:
				m_setkeyvalue++;
				if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
				{					
					tmpdataH = 0;
					tmpdataL = 0;
					for(i = 0; i < 3;i++)
					{
						tmpdataH *= 10;
						tmpdataH += m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]+i+tmpdataL] & 0xf;
					}
					m_proset[pwinmenu->row].byValHih = tmpdataH;
					tmpdataH = 0;
					tmpdataL += 4;
					for(i = 0; i < 3; i++)
					{
						tmpdataH *= 10;
						tmpdataH += m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]+i+tmpdataL] & 0xf;
					}
					m_proset[pwinmenu->row].byValHil=tmpdataH;
					tmpdataL += 4;
					tmpdataH = 0;
						
					for(i = 0; i < 3; i++)
					{
						tmpdataH *= 10;
						tmpdataH += m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]+i+tmpdataL] & 0xf;
					}
					m_proset[pwinmenu->row].byValLoh = tmpdataH;
					tmpdataL += 4;
					tmpdataH = 0;
					for(i = 0; i < 3; i++)
					{
						tmpdataH*=10;
						tmpdataH+=m_dispbuf[charaddr[0][0]*maxcol+charaddr[0][1]+i+tmpdataL] & 0xf;
					}
					m_proset[pwinmenu->row].byValLol = tmpdataH;
					tmpdataL += 4;
					
					Returntopmenu();
					return;
				}
				break;
								

		}

	}
	sprintf((char*)tt,"保护定值");
	WriteDispBuff(tt,0,0,15);
	
	WriteDispBuff(m_set_miaoshu[pwinmenu->row].szName,1,0,16);
	
	newmouse.x = charaddr[m_setkeyvalue][0];
	newmouse.num = m_setmouse.num;
	newmouse.y = charaddr[m_setkeyvalue][1];
		

}
BYTE Cdisp::Get_zhengshu(BYTE *psrc,BYTE num)
{
	int i;
	for(i = 0; i < num; i++)
	{
		if(psrc[i] != 0x30)
			break;
	}
	return i;
}
void Cdisp::set_proYB()
{
	MASK=UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ADDKEY|SUBBKEY|ENTERKEY;
	BYTE charaddr[][2]={
		{4,0},
		{4,4},			
		{4,8},			
		{4,12}			
	};
	BYTE m_state;
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->topmenu->page = 0;
		pwinmenu->row = pwinmenu->para;
		pwinmenu->para = 0;
		m_setkeyvalue = 1;
		pwinmenu->page = 0;
 	}
	m_setmouse.num = 4;
	if(m_keyval != m_keybak)
	{
		clearscreen();
		switch(m_keyval)
		{
			case UPKEY:
			case DOWNKEY:
			case LEFTKEY:
			case RIGHTKEY:
				m_setkeyvalue++;
				if(m_yb_miaoshu[pwinmenu->para].byYbOpt == SETOPT_READ)
					if(m_setkeyvalue == 2)
						m_setkeyvalue++;
				break;
			case ENTERKEY:
				switch(m_setkeyvalue)
				{
					case 0://上页
						if(pubval == 1) //公共定值软压板
						{
							if(pwinmenu->para == 0)
								pwinmenu->para = m_ybnum;
						}
						else if(pubval == 2)  //开关软压板
						{
							if(pwinmenu->para == 0)
								pwinmenu->para = m_ybnum+1;
							
						}
						if(pwinmenu->para != 0)
							pwinmenu->para--;
						while(m_yb_miaoshu[pwinmenu->para].byYbOpt == SETOPT_HIDE)
						{
							if(pwinmenu->para == 0)
								pwinmenu->para = m_ybnum+1;
							if(pwinmenu->para != 0)
								pwinmenu->para--;
						}
						break;
					case 1://下页
						pwinmenu->para++;
						if(pubval == 2)  //开关软压板
						{
							if(pwinmenu->para > m_ybnum)
								pwinmenu->para = 0;
						}
						else if(pubval == 1) //公共定值软压板
						{
							if(pwinmenu->para >= m_ybnum)
								pwinmenu->para = 0;
						}
							
						while(m_yb_miaoshu[pwinmenu->para].byYbOpt == SETOPT_HIDE)
						{
							pwinmenu->para++;
							if(pwinmenu->para > m_ybnum)
								pwinmenu->para = 0;
						}
						break;
					case 2://修改
						if(pubval == 1)
						{
							if(m_yb_miaoshu[pwinmenu->para].byYbOpt == SETOPT_READ)
								break;
							pwinmenu->topmenu->page = 1;
							m_tmpYB ^= (1<<m_yb_miaoshu[pwinmenu->para].byYbNo);
						}
						else if(pubval == 2)
						{
							if(m_yb_miaoshu[pwinmenu->para-1].byYbOpt == SETOPT_READ)
								break;
							pwinmenu->topmenu->page = 1;
							m_tmpYB ^= (1<<m_yb_miaoshu[pwinmenu->para-1].byYbNo);

						}
						break;
					case 3://返回
						writeYB();
						Returntopmenu();
						return;
					default:
						m_setkeyvalue = 1;
						break;
				}
				break;
			}

		}
		sprintf((char*)tt,"压板投退");
		while(m_yb_miaoshu[pwinmenu->para].byYbOpt == SETOPT_HIDE)
		{
			pwinmenu->para++;
			if(pwinmenu->para >= m_ybnum)
				pwinmenu->para = 0;
		}
		WriteDispBuff(tt,0,0,16);
		if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
			m_setkeyvalue = 0;
		if(pubval == 2)  //开关软压板
		{
			WriteDispBuff(m_yb_miaoshu[pwinmenu->para-1].szName,1,0,16);
			m_state = 0;
			if(m_tmpYB & (1 << m_yb_miaoshu[pwinmenu->para-1].byYbNo))
				m_state = 1;
		}
		else if(pubval == 1) //公共定值开关
		{
			WriteDispBuff(m_yb_miaoshu[pwinmenu->para].szName,1,0,16);
			m_state = 0;
			if(m_tmpYB & (1 << m_yb_miaoshu[pwinmenu->para].byYbNo))
				m_state = 1;
		}

		WriteDispBuff("上页下页修改返回",4,0,16);
		if(m_state)
			WriteDispBuff("投入",2,10,4);
		else
			WriteDispBuff("退出",2,10,4);

		newmouse.x = charaddr[m_setkeyvalue][0];
		newmouse.num = m_setmouse.num;
		newmouse.y = charaddr[m_setkeyvalue][1];

}
void Cdisp::set_proCON1()
{
	MASK=UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ADDKEY|SUBBKEY|ENTERKEY;
	BYTE charaddr[][2]={
		{4,0},
		{4,4},			
		{4,8},			
		{4,12}			
	};
	BYTE m_state;
	BYTE i;
	WORD m_con;
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		m_setmouse.x = 0;
		m_setmouse.y = 0;
		pwinmenu->para = 0;
		m_setkeyvalue = 1;
		pwinmenu->page = 0;
		m_conindex = 0;
		for(i = 0; i < pwinmenu->row + 1; i++)
		{
			if((m_proset[i].bySetAttrib & 0xf) == SETTYPE_KG)
				m_conindex++;
		}
		pwinmenu->para = 0;
 	}
	if(m_conindex == 1)
	{
		if(m_CON1num == 0)
		{
			Returntopmenu();
			return;
		}
		m_setmouse.num = 4;
		m_state=0;
		for(i = 0; i < 3; i++)
		{
			if(m_con1_miaoshu[pwinmenu->para].byKGBit[i] == 0xff)
				break;
			m_con = 0;
			if(m_con < m_CONBYTEnum)
				m_state |= (((m_pcon[m_con]>>(m_con1_miaoshu[pwinmenu->para].byKGBit[i]&0x1f))&1)<<i);
			else
				break;
		}
		if(m_keyval != m_keybak)
		{
			clearscreen();
			switch(m_keyval)
			{
				case UPKEY:
				case ADDKEY:
				case DOWNKEY:
				case SUBBKEY:
				case LEFTKEY:
				case RIGHTKEY:
					m_setkeyvalue++;
					if(m_con1_miaoshu[pwinmenu->para].byKGOpt == SETOPT_READ)
						if(m_setkeyvalue == 2)
							m_setkeyvalue++;
					break;
				case ENTERKEY:
					switch(m_setkeyvalue)
					{
						case 0://上页
							if(pwinmenu->para == 0)
								pwinmenu->para = m_CON1num;
							if(pwinmenu->para != 0)
								pwinmenu->para--;
							while(m_con1_miaoshu[pwinmenu->para].byKGOpt == SETOPT_HIDE)
							{
								if(pwinmenu->para == 0)
									pwinmenu->para = m_CON1num;
								if(pwinmenu->para != 0)
									pwinmenu->para--;
							}
							break;
						case 1://下页
							pwinmenu->para++;
							if(pwinmenu->para >= m_CON1num)
								pwinmenu->para = 0;
							while(m_con1_miaoshu[pwinmenu->para].byKGOpt == SETOPT_HIDE)
							{
								pwinmenu->para++;
								if(pwinmenu->para >= m_CON1num)
									pwinmenu->para = 0;
							}
							break;
						case 2://修改
							if(m_con1_miaoshu[pwinmenu->para].byKGOpt == SETOPT_READ)
								break;
							pwinmenu->topmenu->page = 1;
							if(m_state + 1 < m_con1_miaoshu[pwinmenu->para].byMsNum)
								m_state++;
							else
								m_state=0;
							for(i=0;i<3;i++)
							{
								if(m_con1_miaoshu[pwinmenu->para].byKGBit[i] == 0xff)
									break;
								m_con=0;
								if(m_con<m_CONBYTEnum)
								{
									m_pcon[m_con] &= ~(1<<(m_con1_miaoshu[pwinmenu->para].byKGBit[i]&0x1f));
									m_pcon[m_con] |= (((m_state>>i)&1)<<(m_con1_miaoshu[pwinmenu->para].byKGBit[i]&0x1f));
								}
								else
									break;
							}
							break;
						case 3://返回
							writecon();
							Returntopmenu();
							return;
						default:
							m_setkeyvalue = 1;
							break;
						}
					break;
			}

		}
		sprintf((char*)tt,"控制字");
		for(i = 0;  i < 3;i++)
		{
			if(m_con1_miaoshu[pwinmenu->para].byKGBit[i] == 0xff)
				break;
			sprintf((char*)tt+strlen(tt),"D%d ",m_con1_miaoshu[pwinmenu->para].byKGBit[i]);
		}
		WriteDispBuff(tt,0,0,16);
		if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
			m_setkeyvalue = 0;
	
		WriteDispBuff(m_con1_miaoshu[pwinmenu->para].szName,1,0,16);
		WriteDispBuff("上页下页修改返回",4,0,16);
		sprintf((char*)tt,"%d-------",m_state);
		WriteDispBuff((char*)tt,2,0,8);
		WriteDispBuff((char*)m_con1_miaoshu[pwinmenu->para].szBit+m_state*9,2,8,8);

		newmouse.x = charaddr[m_setkeyvalue][0];
		newmouse.num = m_setmouse.num;
		newmouse.y = charaddr[m_setkeyvalue][1];
	}
	else
	{
		if(m_CON2num == 0)
		{
			Returntopmenu();
			return;
		}
		m_setmouse.num = 4;
		m_state = 0;
		for(i = 0; i < 3;i++)
		{
			if(m_con3_miaoshu[pwinmenu->para].byKGBit[i] == 0xff)
				break;
			m_con = 1;
			if(m_con<m_CONBYTEnum)
				m_state |= (((m_pcon[m_con]>>(m_con3_miaoshu[pwinmenu->para].byKGBit[i]&0x1f))&1)<<i);
			else
				break;
		}
		if(m_keyval != m_keybak)
		{
			clearscreen();
			switch(m_keyval)
			{
				case UPKEY:
				case DOWNKEY:
				case LEFTKEY:
				case RIGHTKEY:
					m_setkeyvalue++;
					if(m_con3_miaoshu[pwinmenu->para].byKGOpt == SETOPT_READ)
						if(m_setkeyvalue == 2)
							m_setkeyvalue++;
					break;
				case ENTERKEY:
					switch(m_setkeyvalue)
					{
						case 0://上页
							if(pwinmenu->para == 0)
								pwinmenu->para = m_CON2num;
							if(pwinmenu->para != 0)
								pwinmenu->para--;
							while(m_con3_miaoshu[pwinmenu->para].byKGOpt == SETOPT_HIDE)
							{
								if(pwinmenu->para == 0)
									pwinmenu->para = m_CON2num;
								if(pwinmenu->para != 0)
									pwinmenu->para--;
							}
							break;
						case 1://下页
							pwinmenu->para++;
							if(pwinmenu->para >= m_CON2num)
							pwinmenu->para = 0;
							while(m_con3_miaoshu[pwinmenu->para].byKGOpt == SETOPT_HIDE)
							{
								pwinmenu->para++;
								if(pwinmenu->para >= m_CON2num)
									pwinmenu->para =  0;
							}	
							break;
						case 2://修改
							if(m_con3_miaoshu[pwinmenu->para].byKGOpt == SETOPT_READ)
								break;
							pwinmenu->topmenu->page = 1;
							if(m_state+1<m_con3_miaoshu[pwinmenu->para].byMsNum)
								m_state++;
							else
								m_state = 0;
							for(i = 0;i < 3; i++)
							{
								if(m_con3_miaoshu[pwinmenu->para].byKGBit[i] == 0xff)
									break;
								m_con = 1;
								if(m_con < m_CONBYTEnum)
								{
									m_pcon[m_con] &= ~(1<<(m_con3_miaoshu[pwinmenu->para].byKGBit[i]&0x1f));
									m_pcon[m_con] |= (((m_state>>i)&1)<<(m_con3_miaoshu[pwinmenu->para].byKGBit[i]&0x1f));
								}
								else
									break;
							}
							break;
						case 3://返回
							writecon();
							Returntopmenu();
							return;
							default:
							m_setkeyvalue = 1;
							break;
						}
				break;
			}

		}
		sprintf((char*)tt,"控制字");
		for(i = 0; i < 3; i++)
		{
			if(m_con3_miaoshu[pwinmenu->para].byKGBit[i] == 0xff)
				break;
			sprintf((char*)tt+strlen(tt),"D%d ",m_con3_miaoshu[pwinmenu->para].byKGBit[i]);
		}
		WriteDispBuff(tt,0,0,16);
		if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
			m_setkeyvalue = 0;
	
		WriteDispBuff(m_con3_miaoshu[pwinmenu->para].szName,1,0,16);
		WriteDispBuff("上页下页修改返回",4,0,16);
		sprintf((char*)tt,"%d-------",m_state);
		WriteDispBuff((char*)tt,2,0,8);
		WriteDispBuff((char*)m_con3_miaoshu[pwinmenu->para].szBit+m_state*9,2,8,8);

		newmouse.x = charaddr[m_setkeyvalue][0];
		newmouse.num = m_setmouse.num;
		newmouse.y = charaddr[m_setkeyvalue][1];
	}

}


void Cdisp::changestring(char *src,char *dec)
{
	WORD len=strlen(src);
	WORD i,j;
	for(i = 0,j = 0;i < len; i++)
	{
		if((src[i] == 0xd)||(src[i] == 0xa))
		{
			i++;
			if(src[i] != 0xa)
				i--;
			while((j & 0xf) != 0)
			{
				dec[j++] = 0x20;
			}
			continue;
		}
		dec[j++] = src[i];
	}
	dec[j++] = 0;
}


void Cdisp::Mpwd_err()
{
	MASK = QUITKEY;
	WriteDispBuff("密码错误",1,10,16);
	WriteDispBuff("请按返回键退出!",3,7,16);

	newmouse.x = 1;
	newmouse.num = 0;
	newmouse.y = 1;
	if(m_keybak != m_keyval)
	{
		switch(m_keyval)
		{
			
			case ENTERKEY:
				clearscreen();
				delete_alarmmenu();
				break;
			case QUITKEY:
				clearscreen();
				//while(alarmdispptr[0] != 0xff)
				delete_alarmmenu();
				break;
		}
	}
}
void Cdisp::Mset_success()
{

	MASK = ENTERKEY|QUITKEY;
	WriteDispBuff("定值设置成功!",1,8,16);

	newmouse.x = 1;
	newmouse.num = 0;
	newmouse.y = 1;
	
	if(m_keybak != m_keyval)
	{
		switch(m_keyval)
		{
			case ENTERKEY:
				clearscreen();
				delete_alarmmenu();
				break;
			case QUITKEY:
				clearscreen();
				while(alarmdispptr[0] != 0xff)
				delete_alarmmenu();
				break;
		}
	}
}

void Cdisp::Mset_success_and_reset()
{
	MASK=ENTERKEY|QUITKEY|UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY;
	WriteDispBuff("  修改成功!  ",1,8,16);
	WriteDispBuff("更新参数请等待...",2,8,16);
 
	newmouse.x = 1;
	newmouse.num = 0;
	newmouse.y = 1;
	if(alarmwinmenu.sunmenu->flag != 0x66)
	{
		alarmwinmenu.sunmenu->para = 8;
		alarmwinmenu.sunmenu->flag = 0x66;
	}
	if(alarmwinmenu.sunmenu->para-- == 0)
	{
		reboot(0);
	}
}


void Cdisp::Mmeter_success()
{
	MASK=ENTERKEY|QUITKEY;
	WriteDispBuff("                             ",1,0,16);
	WriteDispBuff("    整定成功!                              ",2,0,16);
	WriteDispBuff("               ",3,0,16);
	WriteDispBuff("                                                  ",4,0,16);

	newmouse.x = 1;
	newmouse.num = 0;
	newmouse.y = 1;
	if(m_keybak != m_keyval)
	{
		switch(m_keyval)
		{
			case ENTERKEY:
				clearscreen();
				delete_alarmmenu();
				break;
			case QUITKEY:
				clearscreen();
				while(alarmdispptr[0] != 0xff)
					delete_alarmmenu();
				break;
		}
	}
}
void Cdisp::M110_rundata()
{
	char row1[]="1.本地交采值";
	char row2[]="2.上传遥测值";
	char row3[]="3.本地遥测值";
	char row6[]="(1)实时数据";
	WriteDispBuff(row6,0,0,16);
	WriteDispBuff(row1,1,0,16);
	WriteDispBuff(row2,2,0,16);
	WriteDispBuff(row3,3,0,16);
	newmouse.y = 0;
	newmouse.x = (pwinmenu->para - 1) % 3 + 1;
	newmouse.num = 12;
	m_keyupdownval = 0;
	MASK = 0;
}

void Cdisp::Mmea_YC()
{
#ifdef INCLUDE_YC
	
	int i;
	struct VDBYX yx;
	short RecYCNum;
	struct VTrueYCCfg *pYCCfg = NULL;
	VMeaYc * pyc;
	clearscreen();

	if(pturnmenu->flag != 0x66)
	{
		pturnmenu->flag = 0x66;
		pturnmenu->row = 0;
		m_setkeyvalue = 0;
		pturnmenu->page = 0;
 	}			
	if(g_Sys.Eqp.pInfo[0].pTEqpInfo != NULL)
	{
		pYCCfg = g_Sys.Eqp.pInfo[0].pTEqpInfo->pYCCfg;
	}
	else
	{
		m_lamp_delay = closeTIME;
		return;
	}
    RecYCNum = meaRead_Yc1( 0, 11, (VMeaYc *)m_dwPubBuf);
	yx.wNo=0;
	ReadSYX(g_Sys.wIOEqpID, 1, sizeof(VDBYX), &yx);
	if(RecYCNum == 0)
	{
		pturnmenu = pturnmenu->nextmenu;
		pturnmenu->flag = 0;
		return;
	}
	pyc = (VMeaYc *)m_dwPubBuf;
	for(i = 0; i < 11;i++)
	{
		if(i >= RecYCNum)
			WriteDispBuff("  ",i+1,0,16);	
		else
		{
			if(strcmp(pYCCfg[i].sNameTab,"Ua[1]") == 0  || strcmp(pYCCfg[i].sNameTab,"Ua1") == 0)
			{
				sprintf((char*)tt+500,"Ua1: ");
				sprintf((char*)tt+500+5,"%06f ",pyc[i].value*10);
				sprintf((char*)tt+500+11,"%s  ",pyc[i].unit);
				
				WriteDispBuff(tt+500,0,0,16);
			}
			if(strcmp(pYCCfg[i].sNameTab,"Uc[1]") == 0 || strcmp(pYCCfg[i].sNameTab,"Uc1") == 0)
			{
				sprintf((char*)tt+500,"Uc1: ");
				sprintf((char*)tt+500+5,"%06f ",pyc[i].value*10);
				sprintf((char*)tt+500+11,"%s ",pyc[i].unit);
				WriteDispBuff(tt+500,1,0,16);
			}
			if(strcmp(pYCCfg[i].sNameTab,"Uo[1]") == 0 || strcmp(pYCCfg[i].sNameTab,"Uo1") == 0)
			{
				sprintf((char*)tt+500,"Uo1: ");
				sprintf((char*)tt+500+5,"%06f  ",pyc[i].value*10);
				sprintf((char*)tt+500+11,"%s   ",pyc[i].unit);
				WriteDispBuff(tt+500,2,0,16);
			}
			
			if(strcmp(pYCCfg[i].sNameTab,"Ia[1]") == 0 || strcmp(pYCCfg[i].sNameTab,"Ia1") == 0)
			{
				sprintf((char*)tt+500,"Ia1: ");
				sprintf((char*)tt+500+5,"%06f  ",pyc[i].value*10);
				sprintf((char*)tt+500+11,"%s   ",pyc[i].unit);
				WriteDispBuff(tt+500,0,15,16);
			}
			if(strcmp(pYCCfg[i].sNameTab,"Ib[1]") == 0 || strcmp(pYCCfg[i].sNameTab,"Ib1") == 0)
			{
				sprintf((char*)tt+500,"Ib1: ");
				sprintf((char*)tt+500+5,"%06f  ",pyc[i].value*10);
				sprintf((char*)tt+500+11,"%s   ",pyc[i].unit);
				WriteDispBuff(tt+500,1,15,16);
			}
			if(strcmp(pYCCfg[i].sNameTab,"Ic[1]") == 0 || strcmp(pYCCfg[i].sNameTab,"Ic1") == 0)
			{
				sprintf((char*)tt+500,"Ic1: ");
				sprintf((char*)tt+500+5,"%06f  ",pyc[i].value*10);
				sprintf((char*)tt+500+11,"%s   ",pyc[i].unit);
				WriteDispBuff(tt+500,2,15,16);
			}
		}

	}
	GetSysClock(&clock,SYSCLOCK);
	sprintf((char*)tt+500,"%04d-%02d-%02d %02d:%02d:%02d       ",clock.wYear,clock.byMonth,clock.byDay,clock.byHour,clock.byMinute,clock.bySecond);
	
	WriteDispBuff(tt+500,4,0,16);
	
	newmouse.num = 0;
#else
	
		WriteDispBuff("无数据", 2, 2, 18);
	
#endif

}
void Cdisp::Mmea_YC1()
{
#ifdef INCLUDE_YC 
	int i;
	short RecYCNum;
	struct VTrueYCCfg *pYCCfg = NULL;
	VMeaYc * pyc;
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_setkeyvalue = 0;
		pwinmenu->page = 0;
 	}			
	if(g_Sys.Eqp.pInfo[0].pTEqpInfo != NULL)
	{
		pYCCfg = g_Sys.Eqp.pInfo[0].pTEqpInfo->pYCCfg;
	}
	else
	{
		WriteDispBuff("无数据",2,2,18);
		return;
	}
	sprintf((char*)tt,"设备:%s                        ",g_Sys.Eqp.pInfo[0].sCfgName);
	WriteDispBuff(tt,0,0,16);
    RecYCNum = meaRead_Yc1( pwinmenu->row, maxdatarow, (VMeaYc *)m_dwPubBuf);
	if(RecYCNum == 0)
	{
		pwinmenu->row = 0;
	    RecYCNum = meaRead_Yc1( pwinmenu->row, maxdatarow, (VMeaYc *)m_dwPubBuf);		
	}
	pyc = (VMeaYc *)m_dwPubBuf;
	for(i = 0; i < maxdatarow; i++)
	{
		if(i >= RecYCNum)
			WriteDispBuff("                                ",i+1,0,16);	
		else
		{
			sprintf((char*)tt+500,"%s                   ",pYCCfg[pwinmenu->row+i].sNameTab);
			sprintf((char*)tt+500+6,"%07f    ",pyc[i].value);
			sprintf((char*)tt+500+13,"%s    ",pyc[i].unit);
			WriteDispBuff(tt+500,i+1,0,16);
		}

	}
	if(m_keybak != m_keyval)
	{
		switch(m_keyval)
		{
			case UPKEY:
				if(pwinmenu->row == 0)
				pwinmenu->row = g_Sys.Eqp.pInfo[0].pTEqpInfo->Cfg.wYCNum;
				pwinmenu->row -= maxdatarow;
				break;
			case DOWNKEY:	
				pwinmenu->row += maxdatarow;
				if(pwinmenu->row >= g_Sys.Eqp.pInfo[0].pTEqpInfo->Cfg.wYCNum)
					pwinmenu->row = 0;
				break;
		}
	}
	newmouse.num = 0;
#else
	WriteDispBuff("无数据", 2, 2, 18);
#endif

}

int Cdisp::pro_init(int proset)
{
	WORD i,num;
	m_proset = NULL;

	#ifdef INCLUDE_PR
	if(proset == 0)
	{
		if (ReadParaFile("ProSetPubTable.cfg",prTableBuf,MAX_TABLE_FILE_SIZE) == ERROR) 
			return false;
	}
	else if(proset == 1)
	{
		if (ReadParaFile("ProSetTable.cfg",prTableBuf,MAX_TABLE_FILE_SIZE) == ERROR) 
			return false;
	}
	else
		return false;
	TTABLETYPE *ptable = (TTABLETYPE *)(prTableBuf+sizeof(VFileHead));
	if(strstr(ptable->szName,"Relay") == 0)
		return false;
	BYTE* paddr = (BYTE*)(ptable+1);
	m_ybnum = *paddr;
	paddr++;
	m_yb_miaoshu = new tagTYBTABLE[m_ybnum];
	memcpy(m_yb_miaoshu,paddr,m_ybnum*sizeof(tagTYBTABLE));
	paddr += m_ybnum*sizeof(tagTYBTABLE);
	m_CON1num = *paddr;
	paddr++;
	m_con1_miaoshu = new TKGTABLE[m_CON1num];
	m_con2_miaoshu = (char*)m_dwPubBuf;
	num = 0;
	for(i = 0;i < m_CON1num; i++)
	{
		memcpy(m_con1_miaoshu + i,paddr,sizeof(TKGTABLE));
		paddr += sizeof(TKGTABLE) - sizeof(DWORD);
		memcpy(m_con2_miaoshu + num * 9, paddr, m_con1_miaoshu[i].byMsNum * 9);
		num += m_con1_miaoshu[i].byMsNum;
		paddr += m_con1_miaoshu[i].byMsNum * 9;
	}
	m_con2_miaoshu = new char[num * 9];
	memcpy(m_con2_miaoshu, m_dwPubBuf, num*9);
	num = 0;
	for(i = 0;i < m_CON1num; i++)
	{
		m_con1_miaoshu[i].szBit = (char**)(m_con2_miaoshu + num * 9);
		num += m_con1_miaoshu[i].byMsNum;
	}	

	m_CON2num = *paddr;
	paddr++;
	m_con3_miaoshu = 0;
	m_con3_miaoshu = new TKGTABLE[m_CON2num];
	m_con4_miaoshu = (char*)m_dwPubBuf;
	num = 0;
	for(i = 0; i < m_CON2num; i++)
	{
		memcpy(m_con3_miaoshu+i, paddr, sizeof(TKGTABLE));
		paddr += sizeof(TKGTABLE)-sizeof(DWORD);
		memcpy(m_con4_miaoshu + num * 9, paddr, m_con3_miaoshu[i].byMsNum * 9);
		num += m_con3_miaoshu[i].byMsNum;
		paddr += m_con3_miaoshu[i].byMsNum * 9;
	}
	m_con4_miaoshu = new char[num * 9];
	memcpy(m_con4_miaoshu, m_dwPubBuf, num * 9);
	num = 0;
	for(i = 0; i < m_CON2num; i++)
	{
		m_con3_miaoshu[i].szBit = (char**)(m_con4_miaoshu + num * 9);
		num += m_con3_miaoshu[i].byMsNum;
	}	
		
	m_setnum = *paddr;
	paddr++;
	m_set_miaoshu = new tagTSETTABLE[m_setnum];
	memcpy(m_set_miaoshu,paddr,m_setnum*sizeof(tagTSETTABLE));
	pproset = new BYTE[PR_SET_SIZE];
	m_proset = (TSETVAL*)(pproset+sizeof(TTABLETYPE));

	m_CONBYTEnum = 2;
	m_pcon = new DWORD[m_CONBYTEnum];
	if(m_proset)
		ReadSet(0,(BYTE*)pproset);
	readYB();
	readcon();
#endif
	return true;
}

void Cdisp::M9_proset()
{
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ENTERKEY;
	DWORD tmpdataH;
	WORD tmpdataL;
	m_key_temp = 20;
	m_keyupdownval = 0;
	
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_setmouse.x = 0;
		m_setmouse.y = 0;

		m_setkeyvalue = 1;
		pwinmenu->page = 1;
		readYB();
		readcon();
	}
	if(m_keyval != m_keybak)
	{
		clearscreen();
		switch(m_keyval)
		{
			case UPKEY:
			case DOWNKEY:
			case LEFTKEY:
			case RIGHTKEY:
				if(((m_proset[pwinmenu->row].bySetAttrib & 0xf) != SETTYPE_YB)&&
					((m_proset[pwinmenu->row].bySetAttrib&0xf) != SETTYPE_KG)&&
					((m_proset[pwinmenu->page].bySetAttrib&0xf) != SETTYPE_YB)&&
					((m_proset[pwinmenu->page].bySetAttrib&0xf) != SETTYPE_KG))
								pwinmenu->row = pwinmenu->page;
				
				pwinmenu->row++;
				
				if(pwinmenu->row >= m_setnum)
					pwinmenu->row = 0;
				while((m_proset[pwinmenu->row].bySetAttrib >> 4 ) == SETOPT_HIDE)
				{
					pwinmenu->row++;
					if(pwinmenu->row >= m_setnum)
						pwinmenu->row = 0;
				}
				pwinmenu->page = pwinmenu->row;
				pwinmenu->page++;
				
				if(pwinmenu->page >= m_setnum)
					pwinmenu->page = 0;
				while((m_proset[pwinmenu->page].bySetAttrib >> 4) == SETOPT_HIDE)
				{
					pwinmenu->page++;
					if(pwinmenu->page >= m_setnum)
						pwinmenu->page = 0;
				}
				break;
			case ENTERKEY:
				switch(m_proset[pwinmenu->row].bySetAttrib & 0xf)
				{
					case SETTYPE_YB:
						Entersonmenu();
						break;
					case SETTYPE_KG:	
						Entersonmenu();
						Downmenu();
						break;
				}
				break;
			}

	}
						
	WriteDispBuff(m_set_miaoshu[pwinmenu->page].szName,2,0,16);
	newmouse.num = 16;

	switch(m_proset[pwinmenu->page].bySetAttrib & 0xf)
	{
		case SETTYPE_YB:
		case SETTYPE_KG:
			sprintf((char*)tt,"%02x%02x%02x%02x",m_proset[pwinmenu->page].byValHih,m_proset[pwinmenu->page].byValHil,
			m_proset[pwinmenu->page].byValLoh,m_proset[pwinmenu->page].byValLol);
			WriteDispBuff(tt,3,2,8);
			break;
		case SETTYPE_IP:
			sprintf((char*)tt,"%03d.%03d.%03d.%03d",m_proset[pwinmenu->page].byValHih,m_proset[pwinmenu->page].byValHil,
			m_proset[pwinmenu->page].byValLoh,m_proset[pwinmenu->page].byValLol);
			WriteDispBuff(tt,3,1,15);
			break;
		default:
			tmpdataH = m_proset[pwinmenu->page].byValHih;
			tmpdataH <<= 8;
			tmpdataH |= m_proset[pwinmenu->page].byValHil;
			tmpdataH <<= 8;
			tmpdataH |= m_proset[pwinmenu->page].byValLoh;
			tmpdataH <<= 8;
			tmpdataH |= m_proset[pwinmenu->page].byValLol;
			switch(m_proset[pwinmenu->page].byValAttrib & 3)
			{
				case 1:
					tmpdataL = tmpdataH & 0xf;
					tmpdataH >>= 4;
					tmpdataL <<= 8;
					break;
				case 2:
					tmpdataL=tmpdataH & 0xff;
					tmpdataH >>= 8;
					tmpdataL <<= 4;
					break;
				case 3:
					tmpdataL=tmpdataH & 0xfff;
					tmpdataH >>= 12;
					break;
				default:
					tmpdataL = 0;
					break;
			}
		if(m_proset[pwinmenu->page].bySetAttrib == 0x2D)
			sprintf((char*)tt,"%x",tmpdataH);
			//sprintf((char*)tt,"%x%03x",tmpdataH,tmpdataL);
		else
			sprintf((char*)tt,"%x.%03x",tmpdataH,tmpdataL);
		WriteDispBuff(tt,3,4,12);
	}
	WriteDispBuff(m_set_miaoshu[pwinmenu->row].szName,0,0,16);
	
	switch(m_proset[pwinmenu->row].bySetAttrib & 0xf)
	{
		case SETTYPE_YB:
		case SETTYPE_KG:
			newmouse.num = 16;
			sprintf((char*)tt,"%02x%02x%02x%02x",m_proset[pwinmenu->row].byValHih,m_proset[pwinmenu->row].byValHil,
			m_proset[pwinmenu->row].byValLoh,m_proset[pwinmenu->row].byValLol);
			WriteDispBuff(tt,1,2,8);
			break;
		case SETTYPE_IP:
			newmouse.num = 0;
			sprintf((char*)tt,"%03d.%03d.%03d.%03d",m_proset[pwinmenu->row].byValHih,m_proset[pwinmenu->row].byValHil,
			m_proset[pwinmenu->row].byValLoh,m_proset[pwinmenu->row].byValLol);
			WriteDispBuff(tt,1,1,15);
			break;
		default:
			newmouse.num = 0;
			tmpdataH = m_proset[pwinmenu->row].byValHih;
			tmpdataH <<= 8;
			tmpdataH |= m_proset[pwinmenu->row].byValHil;
			tmpdataH <<= 8;
			tmpdataH |= m_proset[pwinmenu->row].byValLoh;
			tmpdataH <<= 8;
			tmpdataH |= m_proset[pwinmenu->row].byValLol;
			switch(m_proset[pwinmenu->row].byValAttrib&3)
			{
				case 1:
					tmpdataL = tmpdataH & 0xf;
					tmpdataH >>= 4;
					tmpdataL <<= 8;
					break;
				case 2:
					tmpdataL = tmpdataH & 0xff;
					tmpdataH >>= 8;
					tmpdataL <<= 4;
					break;
				case 3:
					tmpdataL = tmpdataH & 0xfff;
					tmpdataH >>= 12;
					break;
				default:
					tmpdataL = 0;
					break;
			}
		if(m_proset[pwinmenu->row].bySetAttrib == 0x2D)
			//sprintf((char*)tt,"%x.%03x",tmpdataH,tmpdataL);
			sprintf((char*)tt,"%x",tmpdataH);
		else
			sprintf((char*)tt,"%x.%03x",tmpdataH,tmpdataL);
			WriteDispBuff(tt,1,4,12);
			break;			
	}
	newmouse.x = 1;
	newmouse.y = 1;
	

}
void Cdisp::M911_disp_proYB()
{
	MASK=UPKEY|DOWNKEY;
	BYTE charaddr[][2]={
		{3,0},
		{3,4},			
		{3,8},			
		{3,12}			
	};
	BYTE m_state;
	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = pwinmenu->topmenu->page;
		pwinmenu->para = 0;
		m_setkeyvalue = 1;
		pwinmenu->page = 0;
 	}
	m_setmouse.num = 4;
	if(m_keyval != m_keybak)
	{
		clearscreen();
		switch(m_keyval)
		{
			case UPKEY:
			case DOWNKEY:
			case LEFTKEY:
			case RIGHTKEY:
			case ENTERKEY:		
				pwinmenu->para++;
				
				if(pubval == 2)  //开关软压板
				{
					if(pwinmenu->para > m_ybnum)
						pwinmenu->para = 0;
				}
				else if(pubval == 1) //公共定值软压板
				{
					if(pwinmenu->para >= m_ybnum)
						pwinmenu->para = 0;
				}

				while(m_yb_miaoshu[pwinmenu->para].byYbOpt == SETOPT_HIDE)
				{
					pwinmenu->para++;
					if(pwinmenu->para > m_ybnum)
						pwinmenu->para = 0;
				}
				break;
		}

	}
	sprintf((char*)tt,"压板投退(回路%d)",pwinmenu->page);
	while(m_yb_miaoshu[pwinmenu->para].byYbOpt == SETOPT_HIDE)
	{
		pwinmenu->para++;
		if(pwinmenu->para >= m_ybnum)
			pwinmenu->para = 0;
	}
	WriteDispBuff("压板投退",0,0,16);
	WriteDispBuff("按返回键返回",4,0,16);
	if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
		m_setkeyvalue = 0;
	if(pubval == 2)  //开关软压板
	{
		WriteDispBuff(m_yb_miaoshu[pwinmenu->para-1].szName,1,0,16);
		m_state = 0;
		if(m_tmpYB & (1 << m_yb_miaoshu[pwinmenu->para-1].byYbNo))
			m_state = 1;
	}
	else if(pubval == 1) //公共定值软压板
	{
		WriteDispBuff(m_yb_miaoshu[pwinmenu->para].szName,1,0,16);
		m_state = 0;
		if(m_tmpYB & (1 << m_yb_miaoshu[pwinmenu->para].byYbNo))
			m_state = 1;
	}
		
	
	if(m_state)
		WriteDispBuff("投入",2,10,4);
	else
		WriteDispBuff("退出",2,10,4);

	newmouse.x = charaddr[m_setkeyvalue][0];
	newmouse.num = 0;
	newmouse.y = charaddr[m_setkeyvalue][1];

}
void Cdisp::M912_disp_proCON1()
{
	MASK=UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY|ENTERKEY;
	BYTE charaddr[][2]={
		{3,0},
		{3,4},			
		{3,8},			
		{3,12}			
	};
	BYTE m_state;
	BYTE i;
	WORD m_con;

	if(g_Sys.MyCfg.wFDNum == 0)
	{
		Returntopmenu();
		return;
	}
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = pwinmenu->topmenu->page;
		pwinmenu->para = 0;
		m_setkeyvalue = 1;
		pwinmenu->page = 0;
		m_conindex = 0;
		for(i = 0;i< pwinmenu->row; i++)
		{
			if((m_proset[i].bySetAttrib & 0xf) == SETTYPE_KG)
				m_conindex++;
		}
		
		pwinmenu->para = 0;
 	}
	if(m_conindex == 1)
	{
		if(m_CON1num == 0)
		{
			Returntopmenu();
			return;
		}
		m_setmouse.num = 4;
		m_state = 0;
		for(i = 0;i < 3; i++)
		{
			if(m_con1_miaoshu[pwinmenu->para].byKGBit[i] == 0xff)
				break;
			m_con = 0;
			if(m_con  < m_CONBYTEnum)
				m_state |= (((m_pcon[m_con]>>(m_con1_miaoshu[pwinmenu->para].byKGBit[i]&0x1f))&1)<<i);
			else
				break;
		}
		if(m_keyval != m_keybak)
		{
			clearscreen();
			switch(m_keyval)
			{
				case UPKEY:
				case DOWNKEY:
				case LEFTKEY:
				case RIGHTKEY:
				case ENTERKEY:
					pwinmenu->para++;
					if(pwinmenu->para >= m_CON1num)
						pwinmenu->para = 0;
					while(m_con1_miaoshu[pwinmenu->para].byKGOpt == SETOPT_HIDE)
					{
						pwinmenu->para++;
						if(pwinmenu->para >= m_CON1num)
						pwinmenu->para = 0;
					}
					break;
			}

		}
		sprintf((char*)tt,"控制字");
		for(i = 0;  i < 3; i++)
		{
			if(m_con1_miaoshu[pwinmenu->para].byKGBit[i] == 0xff)
				break;
			sprintf((char*)tt+strlen(tt),"D%d ",m_con1_miaoshu[pwinmenu->para].byKGBit[i]);

		}
		WriteDispBuff(tt,0,0,16);
		WriteDispBuff("按返回键返回",4,0,16);
		if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
			m_setkeyvalue = 0;

		WriteDispBuff(m_con1_miaoshu[pwinmenu->para].szName,1,0,16);
		sprintf((char*)tt,"%d-------",m_state);
		WriteDispBuff((char*)tt,2,0,8);
		WriteDispBuff((char*)m_con1_miaoshu[pwinmenu->para].szBit+m_state*9,2,8,8);

		newmouse.x = charaddr[m_setkeyvalue][0];
		newmouse.num = 0;
		newmouse.y = charaddr[m_setkeyvalue][1];
	}
	else 
	{
		if(m_CON2num == 0)
		{
			Returntopmenu();
			return;
		}
		m_setmouse.num = 4;
		m_state = 0;
		for(i = 0;i < 3; i++)
		{
			if(m_con3_miaoshu[pwinmenu->para].byKGBit[i] == 0xff)
				break;
			m_con = 1;
			if(m_con < m_CONBYTEnum)
				m_state |= (((m_pcon[m_con]>>(m_con3_miaoshu[pwinmenu->para].byKGBit[i]&0x1f))&1)<<i);
			else
				break;
		}
		if(m_keyval != m_keybak)
		{
			clearscreen();
			switch(m_keyval)
			{
				case UPKEY:
				case DOWNKEY:
				case LEFTKEY:
				case RIGHTKEY:
				case ENTERKEY:
					pwinmenu->para++;
					if(pwinmenu->para >= m_CON2num)
						pwinmenu->para = 0;
					while(m_con3_miaoshu[pwinmenu->para].byKGOpt == SETOPT_HIDE)
					{
						pwinmenu->para++;
						if(pwinmenu->para >= m_CON2num)
							pwinmenu->para = 0;
					}
					break;
			}

		}
		sprintf((char*)tt,"控制字");
		for(i = 0;i < 3; i++)
		{
			if(m_con3_miaoshu[pwinmenu->para].byKGBit[i] == 0xff)
				break;
			sprintf((char*)tt+strlen(tt),"D%d ",m_con3_miaoshu[pwinmenu->para].byKGBit[i]);
		}
		WriteDispBuff(tt,0,0,16);
		WriteDispBuff("按返回键返回",4,0,16);
		if(m_setkeyvalue >= sizeof(charaddr)/sizeof(charaddr[0]))
			m_setkeyvalue = 0;
		
		WriteDispBuff(m_con3_miaoshu[pwinmenu->para].szName,1,0,16);
		sprintf((char*)tt,"%d-------",m_state);
		WriteDispBuff((char*)tt,2,0,8);
		WriteDispBuff((char*)m_con3_miaoshu[pwinmenu->para].szBit+m_state*9,2,8,8);

		newmouse.x = charaddr[m_setkeyvalue][0];
		newmouse.num = 0;
		newmouse.y = charaddr[m_setkeyvalue][1];
	}

}

void Cdisp::Mmea_YC2()
{
#ifdef INCLUDE_YC
	int i;
	short RecYCNum;
	struct VTrueYCCfg *pYCCfg = NULL;
	VMeaYc * pyc;
	MASK = UPKEY|DOWNKEY|LEFTKEY|RIGHTKEY;
	if(pwinmenu->flag == 0x55)
	{
		pwinmenu->flag = 0x66;
		pwinmenu->row = 0;
		m_setkeyvalue = 0;
		pwinmenu->page = 0;
	}	
	if(g_Sys.Eqp.pInfo[0].pTEqpInfo != NULL)
	{
		pYCCfg = g_Sys.Eqp.pInfo[0].pTEqpInfo->pYCCfg;
	}
	else
	{
		WriteDispBuff("无数据",2,2,18);
		return;
	}
	sprintf((char*)tt,"设备:%s                        ",g_Sys.Eqp.pInfo[0].sCfgName);
	WriteDispBuff(tt,0,0,16);
    RecYCNum = meaRead_Yc2( pwinmenu->row, maxdatarow, (VMeaYc *)m_dwPubBuf);
	if(RecYCNum == 0)
	{
		pwinmenu->row = 0;
		RecYCNum = meaRead_Yc2( pwinmenu->row, maxdatarow, (VMeaYc *)m_dwPubBuf);		
	}
	pyc = (VMeaYc *)m_dwPubBuf;
	for(i = 0;i < maxdatarow; i++)
	{
		if(i >= RecYCNum)
			WriteDispBuff("                                ",i+1,0,16);	
		else
		{
			sprintf((char*)tt+500,"%s                   ",delchar(pYCCfg[pwinmenu->row+i].sNameTab));
			sprintf((char*)tt+500+10,"%05f    ",pyc[i].value);
			sprintf((char*)tt+500+15,"%s    ",pyc[i].unit);
			WriteDispBuff(tt+500,i+1,0,16);
		}

	}
	if(m_keybak != m_keyval)
	{
		switch(m_keyval)
		{
			case UPKEY:
				if(pwinmenu->row == 0)
					pwinmenu->row = g_Sys.Eqp.pInfo[0].pTEqpInfo->Cfg.wYCNum;
				pwinmenu->row -= maxdatarow;
				break;
			case DOWNKEY:	
				pwinmenu->row += maxdatarow;
				if(pwinmenu->row >= g_Sys.Eqp.pInfo[0].pTEqpInfo->Cfg.wYCNum)
					pwinmenu->row = 0;

				break;
		}
	}
	newmouse.num = 0 ;
#else 
	WriteDispBuff("无数据", 2, 2, 18);
	
#endif

}
void Cdisp::addalarmnum(BYTE num)
{
	BYTE i;
	for(i = 31; i > 0; i--)
	{
		alarmdispptr[i] = alarmdispptr[i-1];
	}
	alarmdispptr[0] = num;


}
void Cdisp::Malarm_event()
{
	WORD poolNum,wrtOffset;
	int j;
	int type;
	
	MASK = ENTERKEY|QUITKEY|DOWNKEY;
	if(alarmdispptr[0] == 0xff)
	{
		delete_alarmmenu();
		return;
	}
	j = alarmdispptr[0] % 32;
	type = alarmdispptr[0] / 32;
	switch(type)
	{
		case 0:
			QuerySysEventInfo(g_Sys.pWarnEvent,1,j,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum);
			break;
		case 1:
			QuerySysEventInfo(g_Sys.pActEvent,1,j,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum);
			break;
		case 2:
			QuerySysEventInfo(g_Sys.pDoEvent,1,j,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum);
			break;
	}
	VSysClock tm;

	changestring(m_Eventinfo[0].msg,(char*)tt);
	if(m_setkeyvalue == 0)
	{
		WriteDispBuff((char*)tt+m_setkeyvalue*31,2,0,16*2);
		CalClockTo(&m_Eventinfo[0].time,&tm);  
		sprintf((char*)tt,"%04d-%02d-%02d   %d  ",tm.wYear, tm.byMonth,tm.byDay,j);
		WriteDispBuff(tt,0,0,16);	
		sprintf((char*)tt,"%02d:%02d:%02d:%03d  ",tm.byHour,tm.byMinute,tm.bySecond,tm.wMSecond);
		WriteDispBuff(tt,1,0,16);	
	}
	else if(((m_setkeyvalue - 1) * 62 + 31) > strlen(tt))
		m_setkeyvalue = 0;
	else
		WriteDispBuff((char*)tt+31+(m_setkeyvalue-1)*62,0,0,16*4);
	
	newmouse.num = 0;
	if(m_keyval != m_keybak)
	{
		switch(m_keyval)
		{
			case UPKEY:
			case DOWNKEY:
			case LEFTKEY:
			case RIGHTKEY:
				clearscreen();
				m_setkeyvalue++;
				break;
			case ENTERKEY:
				clearscreen();
				delete_alarmmenu();
				break;
			case QUITKEY:
				clearscreen();
				while(alarmdispptr[0] != 0xff)
				delete_alarmmenu();
				break;
		}
	}
}

void Cdisp::Malarm_eventinit()
{
	WORD poolNum,wrtOffset;
	QuerySysEventInfo(g_Sys.pActEvent,1,0,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum);
	m_Eventoff.ActEventoff=wrtOffset;

	QuerySysEventInfo(g_Sys.pDoEvent,1,0,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum);
	m_Eventoff.DoEventoff=wrtOffset;

	QuerySysEventInfo(g_Sys.pWarnEvent,1,0,sizeof(m_Eventinfo[0]),m_Eventinfo,&wrtOffset,&poolNum);
	m_Eventoff.WarnEventoff=wrtOffset;
	memset(alarmdispptr,0xff,33);
}
float Cdisp::getdispdata(BYTE *psrc, BYTE n1, BYTE n2)
{
	float  fdata = 0;
	while(n1--)
	{
		fdata *= 10;
		fdata += ((*psrc) & 0xf);
		psrc++;
	}
	if(n2)
		{psrc++;}
	else
		{return fdata;}
	n1 = n2;
	while(n2--)
	{
		fdata *= 10;
		fdata += *psrc & 0xf;
		psrc++;
	}
	while(n1--)
	{
		fdata /= 10;
	}
	return fdata;
}
void Cdisp::readYB()
{
	WORD i;
	for(i = 0; i < m_setnum; i++)
	{
		if((m_proset[i].bySetAttrib & 0xf) == SETTYPE_YB)
		{
			m_tmpYB = m_proset[i].byValHih;
			m_tmpYB <<= 8;
			m_tmpYB |= m_proset[i].byValHil;
			m_tmpYB <<= 8;
			m_tmpYB |= m_proset[i].byValLoh;
			m_tmpYB <<= 8;
			m_tmpYB |= m_proset[i].byValLol;
			break;
		}

	}
}
void Cdisp::writeYB()
{
	WORD i;
	for( i = 0;i < m_setnum; i++)
	{
		if((m_proset[i].bySetAttrib & 0xf) == SETTYPE_YB)
		{
			m_proset[i].byValHih = m_tmpYB >> 24;
			m_proset[i].byValHil = m_tmpYB >> 16;
			m_proset[i].byValLoh = m_tmpYB >> 8;
			m_proset[i].byValLol = m_tmpYB;
			break;
		}

	}
	
}
void Cdisp::readcon()
{
	WORD i,num = 0;
	for(i = 0;i < m_setnum; i++)
	{
		if((m_proset[i].bySetAttrib & 0xf) == SETTYPE_KG)
		{
			m_pcon[num] = m_proset[i].byValHih;
			m_pcon[num] <<= 8;
			m_pcon[num] |= m_proset[i].byValHil;
			m_pcon[num] <<= 8;
			m_pcon[num] |= m_proset[i].byValLoh;
			m_pcon[num] <<= 8;
			m_pcon[num]|= m_proset[i].byValLol;
			num++;
			if(num >= m_CONBYTEnum)
				break;
		}

	}
}
void Cdisp::writecon()
{
	WORD i,num = 0;
	for(i = 0; i < m_setnum; i++)
	{
		if((m_proset[i].bySetAttrib & 0xf) == SETTYPE_KG)
		{
			m_proset[i].byValHih = m_pcon[num] >> 24;
			m_proset[i].byValHil = m_pcon[num] >> 16;
			m_proset[i].byValLoh = m_pcon[num] >> 8;
			m_proset[i].byValLol = m_pcon[num] >> 0;
			num++;
			if(num >= m_CONBYTEnum)
				break;
		}

	}

}
void Cdisp::setdecnum(BYTE *psrc)
{
	char tt[]={'0','1','2','3','4','5','6','7','8','9','0'};
	char num;
	if(m_keybak != m_keyval)
	{
		switch(m_keyval)
		{
			case SUBBKEY:
			case DOWNKEY:
				for(num = 0; num < sizeof(tt); num++)
				{
					if(tt[num] == *psrc)
						break;
				}
				if(num >= sizeof(tt))
				{
					*psrc= 0;
					return;
				}
				if(num == 0)
					num = sizeof(tt) - 1;
				*psrc = tt[num-1];
				break;
			case ADDKEY:
			case UPKEY:
				for(num = 0; num < sizeof(tt); num++)
				{
					if(tt[num] == *psrc)
						break;
				}
				if(num >= sizeof(tt))
				{
					*psrc = 0;
					return;
				}
				*psrc = tt[num+1];
				break;

			case LEFTKEY:
				m_setkeyvalue--;
				break;
			case RIGHTKEY:
				m_setkeyvalue++;
				break;
			case ENTERKEY:
				m_setkeyvalue++;
				break;
			}
		}
	m_keybak = m_keyval;
				
}

		
void Cdisp::procYKMsg(BYTE msgId)
{
	VDBYK *pDBYK = (VDBYK *)DispMsg.abyData;
	VDBYK *pDBYKSend;
	VMsg msg;

	if (m_disp_model == EXTDISP_MODEL_NODISP)
	{
	    msgId=DispMsg.Head.byMsgID;
	    switch (msgId)
	    {
	    	case MI_YKSELECT:
	    	if((m_pRec->byCtrl&EXTMMI_CTRL_Addr) == 0)
	    	{
		    	if ((pDBYK->byStatus == 0)&&(m_YKmsg.wID == pDBYK->wID)&&(m_YKmsg.byValue == pDBYK->byValue & 0x0F))
		    	{
		    	    pDBYKSend = (VDBYK*)msg.abyData;
					pDBYKSend->wID = pDBYK->wID;
					pDBYKSend->byValue = pDBYK->byValue;
					TaskSendMsg(DB_ID, MMI_ID, m_wEqpID[0], MI_YKOPRATE, MA_REQ, sizeof(VDBYK), &msg);// by lvyi
		    	}
				
	    	}
			else
			{
				if ((pDBYK->byStatus == 0)&&(m_YKmsg.wID == pDBYK->wID)&&(m_YKmsg.byValue == pDBYK->byValue & 0x0F))
		    	{
		    	    pDBYKSend = (VDBYK*)msg.abyData;
					pDBYKSend->wID = pDBYK->wID;
					pDBYKSend->byValue = pDBYK->byValue;
					TaskSendMsg(DB_ID, MMI_ID, m_wEqpID[1], MI_YKOPRATE, MA_REQ, sizeof(VDBYK), &msg);// by lvyi
		    	}
			}
	     	 break;
	    }
		return;

	}
  
	if ((DispMsg.Head.byMsgAttr == MA_RET) && (m_curCtrlMsgID == msgId) && (m_pEqpInfo->wID==DispMsg.Head.wEqpID) \
		&&(m_YKmsg.wID == pDBYK->wID) && (m_YKmsg.byValue == pDBYK->byValue & 0x0F))
	clearscreen();
	switch (pDBYK->byStatus)
	{
		case CONTROLOK:
			switch (msgId)
			{
				case MI_YKSELECT:
					m_ykflag = 16;
					m_ykdelay = 0;
					break;
				case MI_YKOPRATE:
					m_ykflag = 14;
					m_ykdelay = 0;
					break;
				case MI_YKCANCEL:
					m_ykflag = 15;
					m_ykdelay = 0;
					break;
			}
	  
		  	break;
		case CONTROLIDINVAILD:
			m_ykflag = 7;
			strcpy((char*)tt,"点号非法");
			break;
		case CONTROLIDDOING:
			m_ykflag = 8;
			strcpy((char*)tt,"该点正在被操作");
			break;
		case CONTROLFAILED:
			m_ykflag = 9;
			strcpy((char*)tt,"控制硬件有问题");
			break;
		case CONTROLDISABLE:
			m_ykflag = 10;
			strcpy((char*)tt,"控制禁止");
			break;
		default:
		switch (msgId)
		{
			case MI_YKSELECT:
				m_ykflag = 11;
				strcpy((char*)tt,"预制失败");
				break;
			case MI_YKOPRATE:
				m_ykflag = 12;
				strcpy((char*)tt,"执行失败");
				break;
			case MI_YKCANCEL:
				m_ykflag = 13;
				strcpy((char*)tt,"撤销失败");
				break;
		 } 
		 break;
	  }    
	
	 			
}
void Cdisp::DoTSDataMsg()
{
    if (!m_bIn) return;
	struct VFileMsgProM *pFileMsgProM;
	UpdateTime = 50;
	pFileMsgProM =(VFileMsgProM *)&DispMsg;
	MMI_FileLen = pFileMsgProM->filelen;
	SendFrameHead(EXTMMI_TYPE_PROG_WRITE, EXTMMI_FUN_WRITE, EXTMMI_CTRL_MASTER);
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(LOWORD(pFileMsgProM->filelen));
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(LOWORD(pFileMsgProM->filelen));
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(HIWORD(pFileMsgProM->filelen));
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(HIWORD(pFileMsgProM->filelen));
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(LOWORD(pFileMsgProM->addr));
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(LOWORD(pFileMsgProM->addr));
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(HIWORD(pFileMsgProM->addr));
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(HIWORD(pFileMsgProM->addr));
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(pFileMsgProM->pLen);
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(pFileMsgProM->pLen);
	memcpy(&m_Send.pBuf[m_Send.wWritePtr],&pFileMsgProM->msgData,pFileMsgProM->pLen);
	m_Send.wWritePtr+=pFileMsgProM->pLen;
	SendFrameTail();
}

#endif

