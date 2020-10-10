/*------------------------------------------------------------------------
 Module:		mmi.c
 Author:		solar
 Project:		
 Creation Date: 2011-09-28
 Description:	
------------------------------------------------------------------------*/

#include "syscfg.h"

#include "sys.h"
#include "mmi_drv.h"
#include "extmmidef.h"

#define PRRESET_DI_INPUT
BYTE commid=0;
extern int g_prInit ;
extern BYTE g_dtu_lcdenable;
extern BYTE g_dtu_comm;

#if(DEV_SP == DEV_SP_DTU_PU)
DWORD g_IUlight = 0;
extern BYTE g_IUTable[16];
#endif

//���鰴ť����
int GetPrResetInput(void)
{
	static int old_val = 0;
	int val, ret;	

    val = 0;

#if (DEV_SP == DEV_SP_DTU)
    val = (GetMyExtDiValue(9));
#ifdef INCLUDE_EXT_DISP
    val = val ||GetExtMmiYx(EXTMMI_RESET);
#endif
#endif

	if ((!old_val) && val)
		ret = 1;
	else
    	ret = 0;

	old_val = val;

	return ret;
}

int GetFaOnInput(void)
{
	int val;

	val = 0;

#if(DEV_SP == DEV_SP_DTU)
    	val =  GetMyExtDiValue(8);
#endif

#if((TYPE_USER == USER_GUIYANG)&&(DEV_SP == DEV_SP_FTU))
	val = !GetExtMmiYx(EXTMMI_AUTO);//ͶΪ����ʽ
#else
#if(DEV_SP == DEV_SP_WDG)
	if(g_wdg_ver)
		val = (!GetExtMmiYx(XINEXTMMI_MS));// ����ʽ1 Ϊ�ֶ� 0Ϊ����
	else
#endif
#if((DEV_SP == DEV_SP_WDG)|(DEV_SP == DEV_SP_FTU))
		val = GetExtMmiYx(EXTMMI_FA);
#endif
#endif	
    return val;
}

//��Ϊ����������
#ifdef _TEST_VER_
#if(DEV_SP == DEV_SP_DTU)
int GetBYInput(void)
{
	static int old_byval = 0;
	int val,ret;
	val = GetMyExtDiValue(10);

	if((old_byval == 0) && val)
		ret = 2;
	else if(old_byval && (val == 0))
		ret = 1;
	else
		ret = 0;
	
	old_byval = val;
	return ret;
}
//���ò������ó���
void SetBYOutput()
{
	int i,ret;
	if (g_prInit)
	{
		ret = GetBYInput();
		if( ret == 2) // Ͷ
		{
			for(i = 0 ; i < fdNum;i++)
				BOMAPrset(i ,1);
		}
		else if(ret == 1) // ��
		{
			for(i = 0 ; i < fdNum;i++)
				BOMAPrset(i ,0);
		}
	}
}
#endif
#endif

#if defined(_YIERCI_RH_) ||(TYPE_USER == USER_GUIYANG)
#if(DEV_SP == DEV_SP_FTU)||(DEV_SP ==DEV_SP_WDG)
extern int GetBsCtrl(void);
//�͵���FA  ,�����Ͳ��� FTU ��fa,��Ϊ�͵أ�����������
int GetFaTypeInput(void)
{
	static int old_byval = 0;
	int val,ret;
	val = GetFaOnInput();

	//if((old_byval == 0) && val)
	if(val)
		ret = 2;
	else
	//else if(old_byval && (val == 0))
		ret = 1;
	//else
		//ret = 0;
	old_byval = val;
	return ret;
}
//���ü����;͵���
void SetFaType()
{
	int ret;
	if (g_prInit)
	{
		ret = GetFaTypeInput();
		if( ret == 2) // Ͷ
		{
			SetFaTypePrset(1);
		}
		else if(ret == 1) // ��
		{
			SetFaTypePrset(0);
		}
	}
}
/* ��բ�������ش��ڱ���λ��ʱ���Զ��������������բ��ң�غ�բ*/
#ifdef _YIERCI_RH_

void SetHzbs()
{
	static BOOL bHZBS = 0;
	if(g_prInit)
	{
		if(GetBsCtrl())
		{
			if(!bHZBS)
			{
				pFdCfg[0].yk_close_dis = 1;
				bHZBS = 1;
				SbFzStateSave(1);
			}
		}
		else
		{
			if(bHZBS)
			{
				pFdCfg[0].yk_close_dis = 0;
				bHZBS = 0;
				SbFzStateSave(0);
			}
		}
	}
}
#endif
#endif
#endif

//��ػ��ť
int GetCellInput(void)
{
    static int old_val = 0;
	int val, ret;

    val = 0;
   	
	#if(DEV_SP == DEV_SP_WDG)
	 val = GetExtMmiYx(EXTMMI_CELL);
	#endif
	
	if ((!old_val) && val)
		ret = 1;
	else
    ret = 0;

	old_val = val;

	return ret;
}

int GetTripOnInput(void)
{
  int val = 1;

#if (DEV_SP == DEV_SP_FTU)
#ifdef INCLUDE_EXT_DISP
	 val =  GetExtMmiYx(EXTMMI_TRIP);
#endif
#endif
#if (DEV_SP == DEV_SP_WDG)
	if(g_wdg_ver)
	 val =  GetExtMmiYx(XINEXTMMI_TRIP);  //����ʽ
#endif
     return val;
}

void mmi(void)
{
	DWORD events;
    BYTE commcnt=0;
#if(DEV_SP == DEV_SP_DTU_PU)   
    BYTE i;
#endif
#ifdef INCLUDE_EXT_MMI
    extmmi_init(MMI_ID);
    evSend(MMI_ID, EV_INIT);
#endif
#ifdef INCLUDE_EXT_DISP

#if (DEV_SP == DEV_SP_DTU)

	if(g_dtu_lcdenable == 1)
	{	
		g_CommChan[g_dtu_comm - COMM_SERIAL_START_NO].tid = MMI_ID;
		extdisp_init(g_dtu_comm);
    	evSend(MMI_ID, EV_INIT);
	}
	
#endif

#if (DEV_SP == DEV_SP_FTU)

	extdisp_init(MMI_ID);
    evSend(MMI_ID, EV_INIT);
#endif
#endif

	tmEvEvery(MMI_ID, KEYSCANTIME, EV_TM1);

    for (; ;)
    {
	    evReceive(MMI_ID , EV_RX_AVAIL|EV_COMM_IDLE|EV_TM1|EV_UFLAG|EV_MSG|EV_EXT_MMI, &events);

		if (events&EV_TM1)
	    {
			if (GetPrResetInput())	
			{
#ifdef INCLUDE_PR
				ResetProtect(0);
#endif
				WriteDoEvent(NULL, 0, "�ֶ�����");
				events |= EV_PROTECT_RESET;          			
			}
			if (GetCellInput())
			{
    		   cellLocalCtrl();
			}
			
//��Ϊ����������
#ifdef _TEST_VER_
#if(DEV_SP == DEV_SP_DTU)
		SetBYOutput();
#endif
#endif

#ifdef _YIERCI_RH_
#if(DEV_SP == DEV_SP_FTU)
		SetFaType();
		SetHzbs();
#endif
#elif(TYPE_USER == USER_GUIYANG)
	SetFaType();
#endif
#ifdef _YIERCI_RH_
#if(DEV_SP == DEV_SP_WDG)
	if(g_wdg_ver) SetFaType();  //����ʽ
#endif
#endif

#if ((DEV_SP == DEV_SP_WDG) ||(DEV_SP == DEV_SP_TTU))			
			ledCheck();
#endif	
#ifdef _GUANGZHOU_TEST_FTU_
            ledCheck();
#endif
            commcnt++;
			if(commid == 1)
			{
				if(commcnt%2 == 1)
				{
					turnLight(BSP_LIGHT_COMM_ID, 1);
				}
				else
					turnLight(BSP_LIGHT_COMM_ID, 0);
			}
            if (commcnt > 10)
            { 
                commcnt = 0;
				commid = 0;
				turnLight(BSP_LIGHT_COMM_ID, 0);
            }
#if(DEV_SP == DEV_SP_DTU_PU)        
        for(i=BSP_LIGHT_COMM1_ID;i<=BSP_LIGHT_COMM16_ID;i++)
        {
            //��ȡ�����Ԫ����״̬
            g_IUlight = 0;
            commCtrl(g_IUTable[i-BSP_LIGHT_COMM1_ID], CCC_CONNECT_CTRL | CONNECT_STATUS_GET, (DWORD *)&g_IUlight);
            turnLight(i,g_IUlight);
        }
#endif
#if (DEV_SP == DEV_SP_FTU) 
			mmi_drv();
#endif
		}

#ifdef INCLUDE_EXT_MMI
		extmmi_run(events);
#endif	

#ifdef INCLUDE_EXT_DISP
#if (DEV_SP == DEV_SP_DTU)
		if(!g_dtu_lcdenable)
			continue;
#endif
		extdisp_run(events);
#endif
    }	
}
