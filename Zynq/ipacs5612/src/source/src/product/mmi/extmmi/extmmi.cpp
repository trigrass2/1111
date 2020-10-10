#include "syscfg.h"

#ifdef INCLUDE_EXT_MMI
#include "sys.h"
#include "extmmi.h"
#include "extmmidef.h"
#include "mmi_drv.h"

#undef EXT_MMI_TEST
static DWORD LedLight = 0;
static DWORD MMIYx;
static VExtMMI *pExtMmi;
static int  mmi_init=0;

extern WORD pannelYx;
extern struct monSerial g_shell_serial;
static BYTE sendflag=0;
DWORD MMI_FileLen=0;
int UpdateTime = 0;

/*放大10倍*/
static WORD  ITable[10][10]=
{
   {0,     5,  10,  15,  20,  25,  30,  35,  40,  45,},
   {50,   55,  60,  65,  70,  75,  80,  85,  90,  95,},
   {100, 105, 110, 115, 120, 125, 130, 135, 140, 145,},
   {150, 155, 160, 165, 170, 175, 180, 185, 190, 195,},
   {200, 205, 210, 215, 220, 225, 230, 235, 240, 245,},
   {250, 255, 260, 265, 270, 275, 280, 285, 290, 295,},
   {300, 305, 310, 315, 320, 325, 330, 335, 340, 345,},
   {350, 355, 360, 365, 370, 375, 380, 385, 390, 395,},
   {400, 405, 410, 415, 420, 425, 430, 435, 440, 445,},
   {450, 455, 460, 465, 470, 475, 480, 485, 490, 495,},
};

/*放大100倍*/
static WORD  I0Table[10][10]=
{
   {0,     1,   2,   3,   4,   5,   6,   7,   8,   9,},
   {10,   11,  12,  13,  14,  15,  16,  17,  18,  19,},
   {20,   25,  30,  35,  40,  45,  50,  55,  60,  65,},
   {70,   75,  80,  85,  90,  95, 100, 105, 110, 115,},
   {120, 125, 130, 135, 140, 145, 150, 155, 160, 165,},
   {170, 175, 180, 185, 190, 195, 200, 205, 210, 215,},
   {220, 240, 260, 280, 300, 320, 340, 360, 380, 400,},
   {420, 440, 460, 480, 500, 520, 540, 560, 580, 600,},
   {620, 640, 660, 680, 700, 720, 740, 760, 780, 800,},
   {820, 840, 860, 880, 900, 920, 940, 960, 980, 1000,},
};

/*放大10倍,s*/
static WORD TITable[10]=
{0,  1,  5,  10,  15,  20,  50,  80,  100,  150,};

#if (TYPE_USER != USER_BEIJING)
static WORD TI0Table[10][10]=
{
	{0,   1,  2,  3,  4,  5,  6,  7,  8,  9,},
	{10, 11, 12, 13, 14, 15, 16, 17, 18, 19,},
	{20, 21, 22, 23, 24, 25, 26, 27, 28, 29,},
	{30, 31, 32, 33, 34, 35, 36, 37, 38, 39,},
	{40, 41, 42, 43, 44, 45, 46, 47, 48, 49,},
	{50, 51, 52, 53, 54, 55, 56, 57, 58, 59,},
	{60, 61, 62, 63, 64, 65, 66, 67, 68, 69,},
	{70, 71, 72, 73, 74, 75, 76, 77, 78, 79,},
	{80, 81, 82, 83, 84, 85, 86, 87, 88, 89,},
	{100, 200, 500, 1200, 2400, 6000, 18000, 36000, 60000, 100,},
};
#else 
static WORD TI0Table[10]=
    {0,  2,  4,  6,  8,  10,  12,  14,  16,  18,};
#endif
//新罩式定值表
/*放大10 倍*/
static WORD  ITable_1[10]=
{ 0,  12,  24,  36,  48,  60,  72,  84,  96,  120,};
/*放大10 倍*/
static WORD TITable_1[10][10]=
{
    {0,   1,  2,  3,  4,  5,  6,  7,  8,  9,},
    {10, 11, 12, 13, 14, 15, 16, 17, 18, 19,},
    {20, 21, 22, 23, 24, 25, 26, 27, 27, 27,},
    {27, 27, 27, 27, 27, 27, 27, 27, 27, 27,},
    {27, 27, 27, 27, 27, 27, 27, 27, 27, 27,},
    {27, 27, 27, 27, 27, 27, 27, 27, 27, 27,},
    {27, 27, 27, 27, 27, 27, 27, 27, 27, 27,},
    {27, 27, 27, 27, 27, 27, 27, 27, 27, 27,},
    {27, 27, 27, 27, 27, 27, 27, 27, 27, 27,},
    {27, 27, 27, 27, 27, 27, 27, 27, 27, 27,},
   
};
/*放大10倍*/
static WORD  U0Table_1[20]=
{0,  50,  100,  150,  200,  250,  300,  350,  400,  450,
 500,550, 600,  650,  700,  750,  800,  850,  900,  950,};

/*放大100倍*/
static WORD  I0Table_1[10][10]=
{
   {0,     20,   40,   60,   80,   100,   120,   140,   160,   180,},
   {200,   220,  240,  260,  280,  300,  320,  340,  360,  380,},
   {400,   500,  1000,  2000,  3000,  4000,  5000,  6000,  6000,  6000,},
   {6000,   6000,  6000,  6000,  6000,  6000, 6000, 6000, 6000, 6000,},
   {6000,   6000,  6000,  6000,  6000,  6000, 6000, 6000, 6000, 6000,},
   {6000,   6000,  6000,  6000,  6000,  6000, 6000, 6000, 6000, 6000,},
   {6000,   6000,  6000,  6000,  6000,  6000, 6000, 6000, 6000, 6000,},
   {6000,   6000,  6000,  6000,  6000,  6000, 6000, 6000, 6000, 6000,},
   {6000,   6000,  6000,  6000,  6000,  6000, 6000, 6000, 6000, 6000,},
   {6000,   6000,  6000,  6000,  6000,  6000, 6000, 6000, 6000, 6000,},
};
/*  放大100倍 */
static DWORD TI0Table_1[10][10]=
{
	{0,   20,  40,  60,  80,  100,  120,  140,  160,  180,},
	{200, 500, 1000, 2000, 4000, 6000, 30000, 60000, 100000, 120000,},
	{240000, 300000, 360000, 420000, 480000, 540000, 600000, 660000, 720000, 720000,},
	{720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000,},
	{720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000,},
	{720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000,},
	{720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000,},
	{720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000,},
	{720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000,},
	{720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000, 720000,},
};

void mmiLight(int id, int on)
{
   DWORD mask;
   DWORD light;
   DWORD bit;


   light = LedLight;

   if (id == BSP_LIGHT_WARN_ID) id = 0;
   if (id >= BSP_LIGHT_KG_ID) id -= (BSP_LIGHT_KG_ID-1);

   bit = 0x01 << id;

   if (on)  light |= bit;
   else light &= (~bit);		

   mask = light ^ LedLight;
   LedLight = light;
   
   if (g_wdg_ver)
    {
      if((id == (BSP_LIGHT_I0_ID - BSP_LIGHT_KG_ID + 1)) ||(id == (BSP_LIGHT_I_ID - BSP_LIGHT_KG_ID + 1)))
        {
          if(on)
            LedLight |= ((DWORD)0x01 << 31); // 此标志位来决定液晶灯闪还是常亮
          else
            LedLight &= ~((DWORD)0x01 << 31);
        }
    }

   if (mmi_init == 0) return;
   
   if (mask)
       evSend(MMI_ID, EV_LIGHT);
}

int GetExtMmiYx(int index)
{

    if (mmi_init == 0) return 0;

    if (MMIYx & (0x01<<index))
		return 1;
	else
		return 0;
}

void extmmi_init(int thid)
{
    pExtMmi = new VExtMMI;
	pExtMmi->m_thid = thid;
	pExtMmi->m_commid = thid;

    MMIYx = 0;
	
	thDisableDog(thid);

	mmi_init = 1;

}

void extmmi_run(int events)
{
	pExtMmi->Run(events);
}


VExtMMI::VExtMMI():VPcol()
{
	m_pSend = (VExtMMIFrmHead *)m_Send.pBuf;        
	m_pRec = NULL;
	m_byCnt = 0;
	m_nNoFrm = 0;
	m_bCommCnt = 0;
	m_dwCount = 0;
	m_bIn = FALSE;
	m_dwCommCtrlTimer = 50;
	m_filests = TRUE;
	m_first =  TRUE;
}

DWORD VExtMMI::SearchOneFrame(BYTE *Buf,WORD Len)
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


void VExtMMI::SendFrameHead(BYTE fmType, BYTE fmFun, BYTE fmCtrl)
{
    m_Send.wReadPtr = 0;

	m_pSend->byCode1 = EXTMMI_CODE1;
	m_pSend->byCode2 = EXTMMI_CODE2;

    if (fmCtrl & EXTMMI_CTRL_MASTER)    
		m_pSend->byCnt = ++m_byCnt;
	else
		m_pSend->byCnt = m_pRec->byCnt; 
	m_pSend->byType = fmType;
	m_pSend->byFun = fmFun;
	m_pSend->byCtrl = fmCtrl;

    m_Send.wWritePtr = sizeof(VExtMMIFrmHead);
}

void VExtMMI::SendFrameTail()
{  
	WORD crcCode;
	
    m_pSend->byLen = m_Send.wWritePtr;
	

	crcCode = Crc16(0xFFFF,m_Send.pBuf,m_Send.wWritePtr);
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(crcCode);
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(crcCode);
		
 
	DoSend((m_pSend->byCtrl<<24));   
}       

int VExtMMI::writeComm(BYTE *buf, int len, DWORD dwFlag)
{       
	if (len)
	{
		len = ::commWrite(m_commid, buf, len, dwFlag);

		if ((dwFlag & (EXTMMI_CTRL_MASTER<<24)) == 0) return len;
		
	}
	
	commCtrl(m_commid, CCC_EVENT_CTRL|COMM_IDLE_ON, &m_dwCommCtrlTimer);

	return len;
}

int VExtMMI::readComm(BYTE *buf, int len, struct timeval *tmval)
{
    return(commRead(m_commid, buf, len, 0));
}


int VExtMMI::ProcFrm(void)
{   
  
    m_pRec = (VExtMMIFrmHead *)m_RecFrm.pBuf;

	m_bCommCnt = 0;

	sendflag=1;

	if (((m_pRec->byCtrl&EXTMMI_CTRL_MASTER) == 0) && (m_byCnt != m_pRec->byCnt)) 
	{
	   logMsg("error%d, %d\n",m_byCnt,m_pRec->byCnt,0,0,0,0);
	   return 0;
	}

    switch (m_pRec->byType)
    {
	case EXTMMI_TYPE_CONNECT:
			ProcConnect();
		    break;
		case EXTMMI_TYPE_HBEAT:
			ProcHBeat();
			break;
		case EXTMMI_TYPE_PRSET:
			ProcPr();
			break;
		case EXTMMI_TYPE_LIGHT:
			ProcLight();
			break;
		case EXTMMI_TYPE_DIO:
			ProcYx();
			break;
		case EXTMMI_TYPE_IP:
			ProcIp();
			break;
		case EXTMMI_TYPE_PROG_WRITE:
			ProcProg();
			break;
    }

	return 0;
}

void VExtMMI::DoTimeOut(void)
{

	m_nNoFrm++;
	UpdateTime--;
	if(UpdateTime<0)
	{
		UpdateTime=-1;
		
		SendLight();
		
		if(m_dwCount++<=(100/KEYSCANTIME))
			return;
		m_dwCount = 0;
		
		m_bCommCnt++;
		
		if ((m_bCommCnt >= EXTMMI_TIMEOUT)&&m_bIn)
		{
			WriteWarnEvent(NULL, SYS_ERR_COMM, 0, "MMI板通讯异常");
			SendLight();
			m_bCommCnt=0;
			m_bIn = FALSE;
			WriteUlogEvent(NULL, 03, 1, "MMI communication error!");
		}
		else
		{
			 if(g_Sys.dwErrCode&SYS_ERR_COMM)
			SysClearErr(SYS_ERR_COMM);
		}
	
		if (!m_bIn)
		{
			evSend(m_thid, EV_INIT);
			return;
		}
	
		if ((m_bCommCnt % 5 ==0) && m_bIn)
		   SendHBeat();
	}
		
}



void VExtMMI::SendConnect(void)
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

void VExtMMI::SendLight(void)
{
	if(!sendflag) return;
    
	
	sendflag = 0;
    m_bLight = TRUE;
    if (!m_bIn) return;
	SendFrameHead(EXTMMI_TYPE_DIO, EXTMMI_FUN_WRITE, EXTMMI_CTRL_MASTER);
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(LOWORD(LedLight));
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(LOWORD(LedLight));
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(HIWORD(LedLight));
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(HIWORD(LedLight));
	SendFrameTail();
}

void VExtMMI::SendConf(BYTE byType,BYTE code)
{
    SendFrameHead(byType, EXTMMI_FUN_CONF, 0);
	m_Send.pBuf[m_Send.wWritePtr++] = code;
	SendFrameTail();
}

void VExtMMI::SendHBeat(void)
{
	if (!m_bIn) return;
	SendFrameHead(EXTMMI_TYPE_DIO, EXTMMI_FUN_WRITE, EXTMMI_CTRL_MASTER);
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(LOWORD(LedLight));
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(LOWORD(LedLight));
	m_Send.pBuf[m_Send.wWritePtr++] = LOBYTE(HIWORD(LedLight));
	m_Send.pBuf[m_Send.wWritePtr++] = HIBYTE(HIWORD(LedLight));
	SendFrameTail();
}



void VExtMMI::ProcConnect(void)
{
    BYTE *pAck;
	pAck = (BYTE*)(m_pRec+1);
    if ((m_pRec->byCnt == m_byCnt)&&(*pAck == EXTMMI_ACK_OK))
    {
		m_bIn = TRUE;
		 if(g_Sys.dwErrCode&SYS_ERR_COMM)
		 {
			SysClearErr(SYS_ERR_COMM);
			WriteUlogEvent(NULL, 03, 0, "MMI communication resumed!");
		 }
		 SendLight();
    }
    else
		evSend(m_thid, EV_INIT);
		
}

void VExtMMI::ProcIp(void)
{
	WORD addr;
	BYTE *pData = (BYTE *)(m_pRec+1);
#if (TYPE_USER == USER_BEIJING)
	float u0set;
#else
	static int error=0;
	char myIp1[20],str[5], *p;
#endif
	 

        if (m_pRec->byFun == EXTMMI_FUN_WRITE)
        {
                addr = pData[1]<<8|pData[0];
#if (TYPE_USER == USER_BEIJING)
		addr = addr % 100;
		if(addr > 20)
			u0set = 100;
		else
			u0set = U0Table_1[addr]/10;
    
		WriteMMIU0Set(0,u0set);
#else		
                
            #ifndef _YIERCI_RH_
                if ((addr <=255)&&(addr >= 1))
                {
                     SysClearErr(SYS_ERR_CFG);
                      error = 0;
                }
                if (((addr > 255)||(addr < 1))&&(error == 0))
                {
                    addr = 255;
                    error = 1;
                    WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "面板IP地址配置错误！");
                 }
                strcpy(myIp1, g_Sys.AddrInfo.Lan2.sIP);
                p = strrchr(myIp1, '.');
                if (p != NULL)			
                {
                    *p = '\0';
                    sprintf(str, ".%d", addr);
                    strcat(myIp1, str);
                }       
                shellprintf("ip recv, %s\n", myIp1);
                SetSysAddr(NULL,NULL,myIp1,NULL,NULL) ;
             #else
             
                if(!pData[1])
                SetFaTypePrset(1);
                else
                SetFaTypePrset(0);
             #endif   
#endif
            
		if(m_pRec->byCtrl & EXTMMI_CTRL_MASTER)
			SendConf(EXTMMI_TYPE_IP, EXTMMI_ACK_OK);
		  
    }
	
}

void VExtMMI::ProcLight(void)
{
    BYTE *pAck;
	pAck = (BYTE*)(m_pRec+1);
    if ((m_pRec->byCnt == m_byCnt)&&(*pAck == EXTMMI_ACK_OK))
    {
		m_bLight = FALSE;
    }
    //else
		//evSend(m_thid, EV_LIGHT);
}


void VExtMMI:: ProcYx(void)
{
     DWORD yx;
	 BYTE *pData = (BYTE*)(m_pRec+1);
  
    pannelYx = 0;
    yx = MAKEDWORD(MAKEWORD(*pData, *(pData+1)), MAKEWORD(*(pData+2), *(pData+3)));
	if (yx & EXTMMI_YX_LOCAL)
		pannelYx |= 0x01;
	else
		pannelYx |= 0x02;
	if (yx & EXTMMI_YX_BS)
	{
	    pannelYx &= ~0x3;
	    pannelYx |= 0x04;
	}

	m_dwYx = 0;
	
    if (yx & EXTMMI_YX_RESET)
		m_dwYx |= 0x01;

	if ((yx & EXTMMI_YX_YKSYN) && (yx & EXTMMI_YX_YKF))
		m_dwYx |= 0x02;

	if ((yx & EXTMMI_YX_YKSYN) && (yx & EXTMMI_YX_YKH))
	{
	    m_dwYx |= 0x04;
#if(DEV_SP == DEV_SP_WDG)
        m_dwYx |= 0x01;
#endif
	}

	if (yx & EXTMMI_YX_TRIPYB)
		m_dwYx |= 0x08;

	/*	if (yx & EXTMMI_YX_YKF_CTRL)
	m_dwYx |= 0x02;

	if (yx & EXTMMI_YX_YKH_CTRL)
	m_dwYx |= 0x04;*/

	if (yx & EXTMMI_YX_CELL)
		m_dwYx |= 0x20;

	if (yx & EXTMMI_YX_FA)
		m_dwYx |= 0x100;

	//  add by gys 2017-07-28  16:11  user: shangdong
	if (yx & EXTMMI_YX_UA_CY)
		m_dwYx |= 0x1000;

	if (yx & EXTMMI_YX_UC_CY)
		m_dwYx |= 0x2000;
       
       if (yx & EXTMMI_YX_SF)  //手分
		m_dwYx |= 0x4000;

       if (yx & EXTMMI_YX_SH) //手合
		m_dwYx |= 0x8000;
       
        if (yx & EXTMMI_YX_MS) //集中就地
		m_dwYx |= 0x10000;

         if (yx & EXTMMI_YX_FUGUI) //信号复归
		m_dwYx |= 0x20000;
         
         if(yx & EXTMMI_YX_CHUKOU) //出口
            m_dwYx |= 0x40000;

        
	MMIYx = m_dwYx;

	if (m_pRec->byCtrl & EXTMMI_CTRL_MASTER)
	   SendConf(EXTMMI_TYPE_DIO, EXTMMI_ACK_OK);

#if (DEV_SP == DEV_SP_WDG)  //调试串口为面板上的串口2，cjl 2016-9-12 09:57:41
	if ((yx & EXTMMI_YX_CHANGE) && (g_shell_serial.used == 0))
	{
		g_shell_serial.commno = COMM_START_NO + 1;
		g_CommChan[g_shell_serial.commno - COMM_START_NO].tid = SHELL_ID;
		g_shell_serial.used = 1;
	}
#endif

	if (m_first)
	{
	    evSend(SELF_DIO_ID, EV_DATAREFRESH);
		m_first = FALSE;
	}

}

void VExtMMI::ProcHBeat(void)
{
    if (m_pRec->byCtrl & EXTMMI_CTRL_MASTER)
    {
        m_bCommCnt = 0;
		SendConf(EXTMMI_TYPE_HBEAT, EXTMMI_ACK_OK);
    } 
}

void VExtMMI::ProcPr(void)
{
    VMMISet *pSet;
	VMMIPrSet PrSet;
    VMMISet_1 *pSet_1;
    VMMIPrSet_1 PrSet_1;
	float val;
	BYTE x,y;

#ifdef INCLUDE_PR
    if (m_pRec->byFun == EXTMMI_FUN_WRITE)
    {
	   if(g_wdg_ver==0)
        {   
            pSet = (VMMISet *)(m_pRec+1);


            x = (pSet->I1Set[0] < 9) ? pSet->I1Set[0]:9;
            y = (pSet->I1Set[1] < 9) ? pSet->I1Set[1]:9;
            val = ITable[x][y];
            PrSet.I1Set = val/10;

            x = (pSet->I2Set[0] < 9) ? pSet->I2Set[0]:9;
            y = (pSet->I2Set[1] < 9) ? pSet->I2Set[1]:9;
            val = ITable[x][y];
            PrSet.I2Set = val/10;

            x = (pSet->I0Set[0] < 9) ? pSet->I0Set[0]:9;
            y = (pSet->I0Set[1] < 9) ? pSet->I0Set[1]:9;
            val = I0Table[x][y];
            PrSet.I0Set = val/100;

            x = (pSet->T2Set < 9) ? pSet->T2Set:9;
            val = TITable[x];
            PrSet.T2Set = val/10;

            x = (pSet->T0Set[0] < 9) ? pSet->T0Set[0]:9;
            y = (pSet->T0Set[1] < 9) ? pSet->T0Set[1]:9;
        
#if (TYPE_USER == USER_BEIJING)
            val = TITable[x];
            PrSet.T1Set = val/10;
            val = TI0Table[y];
            PrSet.T0Set = val/10;
#else
            val = TI0Table[x][y];
            PrSet.T0Set = val/10;   
            if ((x == 9)&&(y == 9))
            PrSet.I0Gj = TRUE;
            else
            PrSet.I0Gj = FALSE;
#endif
            if(WriteMMIPrSet(0, (BYTE*)&PrSet) == OK)
                 WriteDoEvent(NULL, 0, "液晶修改保护定值!");
            else
                WriteDoEvent(NULL, 0, "液晶修改保护定值失败!");

           }
       else
        {
            pSet_1 = (VMMISet_1 *)(m_pRec+1);
            x = (pSet_1->I2Set < 9) ? pSet_1->I2Set:9;
            val = ITable_1[x];
            PrSet_1.I2Set = val/10;

            x = (pSet_1->I0Set[0] < 9) ? pSet_1->I0Set[0]:9;
            y = (pSet_1->I0Set[1] < 9) ? pSet_1->I0Set[1]:9;
            val = I0Table_1[x][y];
            PrSet_1.I0Set = val/100;

            x = (pSet_1->T2Set[0] < 9) ? pSet_1->T2Set[0]:9;
            y = (pSet_1->T2Set[1] < 9) ? pSet_1->T2Set[1]:9;
            val = TITable_1[x][y];
            PrSet_1.T2Set = val/10;

            x = (pSet_1->U0Set < 9) ? pSet_1->U0Set:9;
            val = U0Table_1[x];
            PrSet_1.U0Set = val/10;

            x = (pSet_1->T0Set[0] < 9) ? pSet_1->T0Set[0]:9;
            y = (pSet_1->T0Set[1] < 9) ? pSet_1->T0Set[1]:9;
            val = TI0Table_1[x][y];
            PrSet_1.T0Set = val/100;
            if(WriteMMIPrSet_1(0, (BYTE*)&PrSet_1) == OK)
			WriteDoEvent(NULL, 0, "液晶修改保护定值!");
		else
			WriteDoEvent(NULL, 0, "液晶修改保护定值失败!");
            
         }

		
		
	 //   shellprintf("pr recv, I1 %x, %x,  I2 %x,%x, I0 %x,%x\n",pSet->I1Set[0], pSet->I1Set[1], pSet->I2Set[0],pSet->I2Set[1],pSet->I0Set[0], pSet->I0Set[1]);
	//	shellprintf("pr recv, T2 %x,  T0 %x,%x\n",pSet->T2Set, pSet->T0Set[0],pSet->T0Set[1]);

		if(m_pRec->byCtrl & EXTMMI_CTRL_MASTER)
			SendConf(EXTMMI_TYPE_PRSET, EXTMMI_ACK_OK);
    }
	
#endif
	
}
void VExtMMI::DoTSDataMsg()
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
void VExtMMI::ProcProg(void)
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


void VExtMMI::Run(DWORD events)
{
	int flag,num;
	BYTE msgId;
	
	if (events & EV_TM1) 
		DoTimeOut();

	if (events & EV_RX_AVAIL) 
		DoReceive(NULL, &flag);

	if (events & EV_INIT)
		SendConnect();

	if (events & EV_LIGHT)
	{
		if(UpdateTime<0)
			SendLight();
	}
	
	if (events & EV_MSG)
	{
		num = msgReceive(MMI_ID, (BYTE *)&DispMsg, MAXMSGSIZE, OS_NOWAIT);	
		if (num > 0)
		{
			  msgId = DispMsg.Head.byMsgID;//&(~MSGNOUFLAG);

			  switch (msgId)
			  {
				case MI_ROUTE:
				  DoTSDataMsg();
			    default:
			      break;
			  }    
		}
			 
	}
}


#endif
