#include "DLQ.h"
#if(DEV_SP == DEV_SP_TTU)

SWITCHEVTOPT *SwitchEvtOpt = NULL;
struct VParaInfo SwitchParaInfo[PRPARA_SWITCH_NUM];
SwitchParaOpt *gSwitchParaOpt = NULL;

BYTE g_control_flag[PRPARA_EVER_SWITCH_NUM]=
{0,0,0x3,0,0x30,0,0,0x0c,0,0,
 0x03,0,0x30,0,0xc0,0,0x0c,0x03,0,0x02,
 0,0xf0,0x0c,0xfd,0xc0};
BYTE g_offset_flag[PRPARA_EVER_SWITCH_NUM]=
{0,0,0,0,4,0,0,2,0,0,
0,0,4,0,6,0,2,0,0,1,
0,4,2,0,6
};

WORD g_Time_reco;
BOOL g_switchcomm[MAXDEVICENUM];


ParaGet g_switchparaget;
BOOL g_SetTimeFlag = FALSE;

//断路器事件接口
void SwitchWriteGzEvent(SwitchEvent event)
{
	SwitchEvent *pevt;
	pevt = SwitchEvtOpt->evt + SwitchEvtOpt->wWritePtr;
	memset(pevt,0,sizeof(SwitchEvent));
	memcpy(pevt,&event,sizeof(SwitchEvent));
	SwitchEvtOpt->wWritePtr = (SwitchEvtOpt->wWritePtr+1)&(MAX_GZEVT_NUM-1);
}
int ReadSwitchEvent(SwitchEvent *event)
{
	SwitchEvent *pevt;
	if(SwitchEvtOpt == NULL)
		return ERROR;
		
	if(SwitchEvtOpt->wReadPtr != SwitchEvtOpt->wWritePtr)
	{
		pevt = SwitchEvtOpt->evt + SwitchEvtOpt->wReadPtr;
		memcpy(event,pevt,sizeof(SwitchEvent));
		SwitchEvtOpt->wReadPtr = (SwitchEvtOpt->wReadPtr+1)&(MAX_GZEVT_NUM-1);
		return OK;
	}
	return ERROR;
}


//参数接口
/*0 成功
1 信息体地址错误
2 数据类型、长度错误
3 值范围错误
4 不配
5 写失败
*/
int WriteConstvalue_opt(WORD parano,BYTE *pbuf)
{
	struct VParaInfo *fParaInfo = (struct VParaInfo*)pbuf;
	struct SwitchPara *pPara = NULL;
	
	if(gSwitchParaOpt == NULL)
		return 3;	
	if((parano < PRPARA_SWITCH_ADDR) || (parano > PRPARA_SWITCH_ADDREND))
		return 1;
	pPara = gSwitchParaOpt->para + gSwitchParaOpt->wWritePtr;
	pPara->parano = parano;
	pPara->len = fParaInfo->len;
	pPara->type = fParaInfo->type;
	memcpy(pPara->valuech ,fParaInfo->valuech,PARACHAR_MAXLEN);
	gSwitchParaOpt->wWritePtr = (gSwitchParaOpt->wWritePtr+1)&(MAX_GZEVT_NUM-1);

	return 0;
}
int ReadConstvalue_opt(struct SwitchPara *pSwitchPara)
{
	struct SwitchPara *pPara;	
	if(gSwitchParaOpt->wReadPtr != gSwitchParaOpt->wWritePtr)
	{
		pPara = gSwitchParaOpt->para+ gSwitchParaOpt->wReadPtr;		
		memcpy(pSwitchPara,pPara,sizeof(struct SwitchPara));
		gSwitchParaOpt->wReadPtr = (gSwitchParaOpt->wReadPtr+1)&(MAX_GZEVT_NUM-1);
		return 0;
	}

	return -1;
}
//设置参数。更新存储区域参数+填写参数下发队列
int SetConst_value(WORD parano,char *pbuf)
{
	struct VParaInfo *fParaInfo = NULL;
	WORD no;
	fParaInfo = (struct VParaInfo*)pbuf;
	if((parano < PRPARA_SWITCH_ADDR) || (parano > PRPARA_SWITCH_ADDREND))
		return 1;
	no = parano - PRPARA_SWITCH_ADDR;
	if(gSwitchParaOpt == NULL)
		return 3;
	memcpy(&SwitchParaInfo[no],fParaInfo,sizeof(struct VParaInfo));
	return WriteConstvalue_opt(parano,(BYTE *)pbuf);
}

int GetSwitchParaFast(void)
{
    WORD no1;
    if (g_switchparaget.switchparaoffset == (CaS_NUM-1))
	{
        no1 = g_switchparaget.paraoffset;
	    SwitchParaInfo[no1].type = USHORT_TYPE;					
	    SwitchParaInfo[no1].len= USHORT_LEN;
		SwitchParaInfo[no1].valuech[0] =  (BYTE)(g_Time_reco+1);
		SwitchParaInfo[no1].valuech[1] =  (BYTE)(g_Time_reco>>8);
        g_switchparaget.readflag = 0x55;
		
	}
    else if (g_switchcomm[g_switchparaget.switchno]==  FALSE)
    {
        g_switchparaget.readflag = 0x01;
    }
	return 0;
}

int GetConst_value(WORD parano,char *pbuf)
{
	struct VParaInfo *fParaInfo = NULL;
	WORD no,wEqpNo;
	fParaInfo = (struct VParaInfo*)pbuf;
	if((parano < PRPARA_SWITCH_ADDR) || (parano > PRPARA_SWITCH_ADDREND))
		return ERROR;
	no = parano - PRPARA_SWITCH_ADDR;
	if(gSwitchParaOpt == NULL)
		return ERROR;
	wEqpNo = no /PRPARA_EVER_SWITCH_NUM;
	
       if (g_switchparaget.readflag == 0)
      {
          g_switchparaget.readflag = 0xAA;
          g_switchparaget.switchno = wEqpNo;
          g_switchparaget.switchparaoffset = no%PRPARA_EVER_SWITCH_NUM;	  
		  GetSwitchParaFast();	  
          g_switchparaget.paraoffset = no;
		  if (g_switchparaget.switchparaoffset >= CaS_NUM)
		  	g_switchparaget.readflag = 0x1;
      }
	if (g_switchparaget.readflag  == 0x1)
	{
       g_switchparaget.readflag = 0;
	   return ERROR;
	}
       if (g_switchparaget.readflag != 0x55)
		return 0x80;
	memcpy(fParaInfo,&SwitchParaInfo[no].type,sizeof(struct VParaInfo));
       g_switchparaget.readflag = 0;
	   
	return 0;
}


extern "C" void dlqm(WORD wTaskID)		
{
	CDLQ *Dlqm = new CDLQ();	
	
	if (Dlqm->Init(wTaskID) != TRUE)
	{
		Dlqm->ProtocolExit();
	}
	
	Dlqm->Run();				
}


CDLQ::CDLQ() : CPPrimary()
{
	
}

BOOL CDLQ::Init(WORD wTaskID)
{
	BOOL rc;
	int i,j;
	VDBYX yx;
	rc = CPPrimary::Init(wTaskID,1,NULL,0);
	if (!rc)
	{
		return FALSE;
	}

	
	m_pReceive = (DlqFrame *)m_RecFrame.pBuf;
	m_pSend	 = (DlqFrame *)m_SendBuf.pBuf;
	Auto_Recover = (AUTODELAY_RES *)malloc(sizeof(AUTODELAY_RES)*MAX_AUTO_RECOVER*MAXDEVICENUM);
	if(Auto_Recover == NULL) return ERROR;
	memset(Auto_Recover,0,sizeof(AUTODELAY_RES)*MAX_AUTO_RECOVER*MAXDEVICENUM);
	
	SwitchEvtOpt = (SWITCHEVTOPT*)malloc(sizeof(SWITCHEVTOPT));
	if(SwitchEvtOpt == NULL) return ERROR;
	memset(SwitchEvtOpt,0,sizeof(SWITCHEVTOPT));
	gSwitchParaOpt = (struct SwitchParaOpt*)malloc(sizeof(struct SwitchParaOpt));
	if(gSwitchParaOpt == NULL) return ERROR;
	memset(gSwitchParaOpt,0,sizeof(SwitchParaOpt));
	g_Time_reco = 9;
    m_index = 0;
    m_Delay_time = m_pBaseCfg->Timer.wScanData2/10;
    m_Station_t = 0;
	memset(m_Dlq_slef,0,4);
   memset(m_EventTimes,0,sizeof(m_EventTimes));
    memset(m_yx,0,MAXDEVICENUM);
    m_Dii = 0;
    m_DW_4 = 0;
    m_first = TRUE;

	for (i=0; i<m_wEqpNum; i++)
	{
	    for (j=0; j<m_pEqpInfo[i].wSYXNum; j++)
		{
		    yx.wNo = j;
			yx.byValue = 0x01;
			WriteSYX(m_pEqpInfo[i].wEqpID, 1, &yx);
		}
		
		WriteCommStatus(m_wTaskID, m_pEqpInfo[i].wEqpID, FALSE);
	}
	commCtrl(m_wTaskID, CCC_EVENT_CTRL|COMM_IDLE_ON, &m_Delay_time);
	m_dwCommCtrlTimer = m_Delay_time;

    return TRUE;
}

BYTE CDLQ::time_trans(struct VSysClock *tt,BYTE *a)
{
       tt->wMSecond = 0;
       tt->bySecond = BCD2BYTE(a[0]);
       tt->byMinute = BCD2BYTE(a[1]);
       tt->byHour = BCD2BYTE(a[2]);
       tt->byDay = BCD2BYTE(a[3]);
       tt->byMonth = BCD2BYTE(a[4]);
       tt->wYear =BCD2BYTE(a[5])+2000;
       return 0;
}

 /***************************************************************
	Function：OnTimeOut
		定时处理
	参数：TimerID
		TimerID 定时器ID
	返回：无
***************************************************************/
void CDLQ::DoTimeOut(void)
{
	CPPrimary::DoTimeOut();
	
//    WORD no_tmp = 0;
//    WORD Index = 0;
//    WORD DI_Index = 0;
    DWORD tick;
    DWORD timet;
		int i;
    AUTODELAY_RES	* pauto;
//    struct SwitchPara ParaInfo;
    struct VDBSOE soe;
    struct VDBCOS cos;
	#if 0
     WORD no_tmp = 0;
    WORD Index = 0;
    WORD DI_Index = 0;
		int ret;
		WORD bValue = 0;
    struct SwitchPara ParaInfo; 
#endif  
    tick = Get100usCnt();
	for(i=0,pauto = Auto_Recover;i<MAXDEVICENUM*MAX_AUTO_RECOVER;i++,pauto++)
	{
		if(pauto->Dev_no)
		{
            timet = tick - pauto->tick;
			if(timet > g_Time_reco*10000)
			{
                soe.wNo =pauto->No;
                soe.byValue = SWS_OFF;
                cos.wNo = pauto->No;
                cos.byValue = SWS_OFF;
                WriteSCOS(pauto->Dev_no, 1, &cos);
                soe.Time.wMSecond = pauto->VClock.wMSecond + 10000;
                soe.Time.dwMinute = pauto->VClock.dwMinute;
                if (soe.Time.wMSecond >= 60000)
                {
                   soe.Time.wMSecond -= 60000;
                   soe.Time.dwMinute +=1;
                }
                WriteSSOE(pauto->Dev_no, 1, &soe); 
                pauto->Dev_no = 0;
			}
		}
	}
#if 0
	ret = ReadConstvalue_opt(&ParaInfo);
	 if(!ret)
	  {
		 m_byReqDataType = REQ_YC;
		 ClearEqpFlag(EQPFLAG_YX);
		 ClearEqpFlag(EQPFLAG_YC);
		 no_tmp = ParaInfo.parano - BASE_CS_ADDR;
		 Index = no_tmp/DEV_DZNUM;
		 DI_Index = no_tmp%DEV_DZNUM;
		 m_DW_4 = SwitchParaInfo[2+Index*DEV_DZNUM].valuech[0];
			  bValue = MAKEWORD(ParaInfo.valuech[0], ParaInfo.valuech[1]);
		 SwitchToAddress(m_pEqpInfo[Index].wDAddress);
		 if(Index == 0)
		 {
			 ParaInfo.valuech[0] = m_DW_4;
			 for(i =0 ;i<6;i++)
			 {				 
				 if(bValue == m_SYDLDZZ[m_wEqpNo][i])
				 {
					 SwitchParaInfo[2+Index*DEV_DZNUM].valuech[0]=ParaInfo.valuech[0] = ((m_DW_4&0x0f)|(i<<4));
					 ParaInfo.valuech[1] = 0;
					 break;
				 }
			 }
		 
		 }
		 else if(Index == 1)
		 {
			 ParaInfo.valuech[0] = m_DW_4;
			 for(i =0 ;i<6;i++)
			 {				 
				 if(bValue == m_SYDLDZSJ[m_wEqpNo][i])
				 {
					 SwitchParaInfo[2+Index*DEV_DZNUM].valuech[0]=ParaInfo.valuech[0] = ((m_DW_4&0x0c)|(i<<2));
					 ParaInfo.valuech[1] = 0;
					 break;
				 }
			 }			 
		 }

		 Set_Const_Val(dz_di[DI_Index],m_pEqpInfo[Index].wDAddress,&ParaInfo);
	  }

#endif
    if (m_pBaseCfg->wCmdControl & BROADCASTTIME)
	{
      if(m_first == TRUE)
      {
        SetTaskFlag(TASKFLAG_CLOCK);
        SetAllEqpFlag(EQPFLAG_DD);
        m_first= FALSE;
      }else
      { 
    	  if (TimerOn(m_pBaseCfg->Timer.wSetClock*60))
    	  { 
    		  SetTaskFlag(TASKFLAG_CLOCK);
    	  }  
    	  if (TimerOn(m_pBaseCfg->Timer.wDD*60))
    	  { 
    		   SetAllEqpFlag(EQPFLAG_DD);	//(TASKFLAG_DD);
    		   m_index = 0;
    	  }  
      }
	}
	for (i=0; i<m_wEqpNum; i++)
		g_switchcomm[i] = GetEqpCommStauts(i);
	   
}

DWORD CDLQ::SearchOneFrame(BYTE *Buf, WORD Len)
{
     WORD CheckOffset,FrameLen = 1;
//      BYTE control;
	DWORD byAddress;
	  BYTE CheckRemote,CheckLocal;

       m_pReceive = (DlqFrame *)Buf;

       if ( Len < 5 )  
          return(FRAME_LESS);

        byAddress = MAKEDWORD(MAKEWORD(m_pReceive->dev_addr[0],m_pReceive->dev_addr[1]),
			                  MAKEWORD(m_pReceive->dev_addr[2],m_pReceive->dev_addr[3]));
	   if(byAddress != m_pEqpInfo[m_wEqpNo].wDAddress)
            return (FRAME_ERR | 1);
          

        if ((m_pReceive->head != 0x68) ||(m_pReceive->head2 != 0x68))
			return (FRAME_ERR |1);

		FrameLen = m_pReceive->data_len+12;

        if (Len < FrameLen)
                return (FRAME_LESS);

        CheckOffset = FrameLen - 2;//CRC校验码的位置
                
        CheckRemote = ChkSum(Buf,CheckOffset);//接收到的校验码            
        CheckLocal = Buf[CheckOffset];//根据接收报文计算的校验码

        if ( CheckLocal != CheckRemote )   
			return (FRAME_ERR | FrameLen);  //change by wyj,modbus 无同步字,全扔
        
        return (FRAME_OK | FrameLen);    
}

BYTE CDLQ::SetKGVal(int eqpno, int addr, int flag, BYTE data)
{
     BYTE val,tmp;
     val = SwitchParaInfo[addr+eqpno*DEV_DZNUM].valuech[0];
	 tmp = (val&~flag)|data;

	 return tmp;
}

/***************************************************************
	Function：DoReceive
			   接收报文处理
	参数：无		
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CDLQ::DoReceive()
{
    WORD no_tmp = 0;
    WORD Index = 0;
    WORD DI_Index = 0;
		int ret,i;
		WORD bValue = 0;

    struct SwitchPara ParaInfo;
	ret = ReadConstvalue_opt(&ParaInfo);
	 if(!ret)
	  {
		 m_byReqDataType = REQ_YC;
		 ClearEqpFlag(EQPFLAG_YX);
		 ClearEqpFlag(EQPFLAG_YC);
		 no_tmp = ParaInfo.parano - BASE_CS_ADDR;
		 Index = no_tmp/DEV_DZNUM;
		 DI_Index = no_tmp%DEV_DZNUM;
		 m_DW_4 = SwitchParaInfo[2+Index*DEV_DZNUM].valuech[0];
		 bValue = MAKEWORD(ParaInfo.valuech[0], ParaInfo.valuech[1]);
		 SwitchToAddress(m_pEqpInfo[Index].wDAddress);
		 if(DI_Index == 0)
		 {
		
			 for(i =0 ;i<6;i++)
			 {				 
				 if(bValue == m_SYDLDZZ[m_wEqpNo][i])
				 {
					 SwitchParaInfo[21+Index*DEV_DZNUM].valuech[0]=ParaInfo.valuech[0] = i;
					 ParaInfo.valuech[1] = 0;
					 break;
				 }
			 }
		 
		 }
		 else if(DI_Index == 1)
		 {
		
			 for(i =0 ;i<6;i++)
			 {				 
				 if(bValue == m_SYDLDZSJ[m_wEqpNo][i])
				 {
					 SwitchParaInfo[22+Index*DEV_DZNUM].valuech[0]=ParaInfo.valuech[0] = i;
					 ParaInfo.valuech[1] = 0;
					 break;
				 }
			 }			 
		 }
		 else
		 	SwitchParaInfo[no_tmp].valuech[0] = bValue;
		 
		 if((DI_Index == 0)||(DI_Index == 1)||(DI_Index == 2))
	 	{
	 		ParaInfo.valuech[0]  = (SwitchParaInfo[2+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[2])|
				(SwitchParaInfo[21+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[21])|
				(SwitchParaInfo[22+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[22]);
	 	}
		 if((DI_Index == 4)||(DI_Index == 7)||(DI_Index == 10))
	 	{
			 ParaInfo.valuech[0]  = (SwitchParaInfo[4+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[4])|
				 (SwitchParaInfo[7+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[7])|
				 (SwitchParaInfo[10+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[10])|
				 (SwitchParaInfo[24+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[24]);
	 	
	 	}
		 if((DI_Index == 12)||(DI_Index == 14)||(DI_Index == 16)||(DI_Index == 17))
	 	{
			 ParaInfo.valuech[0]  = (SwitchParaInfo[12+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[12])|
				 (SwitchParaInfo[14+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[14])|
				 (SwitchParaInfo[16+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[16])|
				 (SwitchParaInfo[17+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[17]);
	 	
	 	}
		 if(DI_Index == 19)
		{
			 ParaInfo.valuech[0]  = (SwitchParaInfo[19+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[19])|
				 (SwitchParaInfo[23+Index*DEV_DZNUM].valuech[0]<<g_offset_flag[23]);
		
	 	}
	
		 Set_Const_Val(dz_di[DI_Index],m_pEqpInfo[Index].wDAddress,&ParaInfo);
	  }

	if (SearchFrame() != TRUE)
	{
		return FALSE;
	}

	
	switch(m_pReceive->control&0x5F)
		{
			case SJ_CODE:
				ProcData(m_pReceive->datebuf,m_pReceive->data_len);
				break;
			case SET_CaS:
				DoYkData(m_pReceive->datebuf,m_pReceive->data_len,m_pReceive->control);
				break;
			case CLEAR:
				m_byNextReq = REQ_YC;
				break;
            default :
                break;
		}

	//commCtrl(m_wTaskID, CCC_EVENT_CTRL|COMM_IDLE_ON, &m_Delay_time);
	DoSend();
	return(TRUE); 
}

void CDLQ::ProcData(BYTE *tmpbuf,int len)
{
    DWORD DI;
	int i ;
	WORD bValue;
	if(tmpbuf[0] == 0x36&&tmpbuf[1]==0x43)
	{
		   subtract_data_three(tmpbuf,len);
	}
	else
    subtract_data_three(tmpbuf,len);
	DI = MAKEDWORD(MAKEWORD(tmpbuf[0],tmpbuf[1]),MAKEWORD(tmpbuf[2],tmpbuf[3]));
    DWORD type = tmpbuf[3];

    if(type == 0x02)
    {
        switch(DI&0xffffff00)
	    {
		case DI_YC_U:
			Deal_yc(len-4,&tmpbuf[4],2,2);
			break;
		case DI_YC_I:
			Deal_yc(len-4,&tmpbuf[4],3,2);
			break;
		case DI_YC_SI:
			Deal_yc(len-4,&tmpbuf[4],1,2);
			break;
        default:
                break;
        }

    }else if(type == 0x03)
    {
        switch(DI&0xffffff00)
	    {
         case DI_EVNET_CNT:
		 	if(tmpbuf[0] == 0xff)
	 		{
	 			bValue =  tmpbuf[4+i];
				if(bValue>10)
					bValue =10;
	 			m_EventTimes[i] = bValue;

				SetEqpFlag(EQPFLAG_YX);
				m_byNextReq = REQ_YX;
	 		}
			else
			{
				i = tmpbuf[0]-1;
	 			bValue =  tmpbuf[4];
				if(bValue>10)
					bValue = 10;
				m_EventTimes[i] = bValue;

				SetEqpFlag(EQPFLAG_YX);
				m_byNextReq = REQ_YX;
			}
    			break;
            case DI_YX_ONE:
                DoYx_alarm(&tmpbuf[4],len-4);
                break;
            case DI_YX_TWO:
                DoYx_alarm1(&tmpbuf[4],len-4);
                break;
            case DI_YX_THREE:
                DoYx_alarm2(&tmpbuf[4],len-4);
                break;
            case DI_YX_FOUR:
                DoYx_alarm3(&tmpbuf[4],len-4);
                break;
            case DI_YX_FIVE:
                DoYx_alarm4(&tmpbuf[4],len-4);
                break;
            case DI_YX_SIX:
                DoYx_alarm5(&tmpbuf[4],len-4);
                break;
            default:
                    break;
        }
    }
	else if(type == 0x04)
    {
	    switch(DI)
	    {
		    case DI_STAT_TWO:
			m_Station_t = tmpbuf[4];
			if(m_Station_t & 0xfc)
				SetEqpFlag(EQPFLAG_YX);
			else
				m_byNextReq = REQ_YC;
			break;
        case DI_STAT_ONE:
            DoYx(&tmpbuf[4],len-4);
            break;
        case DI_YX_DEVTYPE:
            Deal_bb(len-4,&tmpbuf[4]);
            break;
		case DI_CAS_1:
			for(i = 0;i<6;i++)
			{	
				bValue = BCD2BYTE(tmpbuf[4+2*i])+BCD2BYTE(tmpbuf[5+2*i])*100;
				memcpy((BYTE*)&m_SYDLDZZ[m_wEqpNo][i],&bValue,sizeof(bValue));
			}
			Deal_DD(dz_di1[2]);
			//Value_DD(&tmpbuf[4],1,0);
			return;
		case DI_CAS_2:
			for(i = 0;i<6;i++)
			{	
				bValue = BCD2BYTE(tmpbuf[4+2*i])+BCD2BYTE(tmpbuf[5+2*i])*100;
				memcpy((BYTE*)&m_SYDLDZSJ[m_wEqpNo][i],&bValue,sizeof(bValue));
			}
			Deal_DD(dz_di1[2]);

			//Value_DD(&tmpbuf[4],1,1);
			return;
		case DI_CAS_3://控制字4
			i = ((tmpbuf[4]&0xf0)>>4);			
			Value_DD((BYTE*)&m_SYDLDZZ[m_wEqpNo][i],1,0);
			i= ((tmpbuf[4]&0x0c)>>2);
			Value_DD((BYTE*)&m_SYDLDZSJ[m_wEqpNo][i],1,1);
			Value_DD(&tmpbuf[4],1,2);
			Value_DD(&tmpbuf[4],1,21);	
			Value_DD(&tmpbuf[4],1,22);
			m_CONTROL4[m_wEqpNo] = tmpbuf[4];
			break;
		case DI_CAS_4:
			Value_DD(&tmpbuf[4],1,3);
			break;
		case DI_CAS_5:
			Value_DD(&tmpbuf[4],1,4);
			Value_DD(&tmpbuf[4],1,7);
			Value_DD(&tmpbuf[4],1,10);
			Value_DD(&tmpbuf[4],1,24);			
			break;
		case DI_CAS_6:
			Value_DD(&tmpbuf[4],1,5);
			break;
		case DI_CAS_7:
			Value_DD(&tmpbuf[4],1,6);
			break;
		case DI_CAS_8:
			Value_DD(&tmpbuf[4],1,8);
			break;
		case DI_CAS_9:
			Value_DD(&tmpbuf[4],1,9);
			break;
		case DI_CAS_10:
			Value_DD(&tmpbuf[4],1,11);
			break;
		case DI_CAS_11:
			Value_DD(&tmpbuf[4],1,12);
			Value_DD(&tmpbuf[4],1,14);
			Value_DD(&tmpbuf[4],1,16);
			Value_DD(&tmpbuf[4],1,17);
			break;
		case DI_CAS_12:
			Value_DD(&tmpbuf[4],1,13);
			break;
		case DI_CAS_13:
			Value_DD(&tmpbuf[4],1,15);
			break;
		case DI_CAS_14:
			Value_DD(&tmpbuf[4],1,18);
			break;
		case DI_CAS_15:
			Value_DD(&tmpbuf[4],1,19);
			Value_DD(&tmpbuf[4],1,23);
			
			break;
		case DI_CAS_16:
			Value_DD(&tmpbuf[4],1,20);
			break;
		default:
			break;
	    }
		if (g_switchparaget.readflag == 0xBB)
		{
			  if ((g_switchparaget.switchparaoffset == 0)||
					  (g_switchparaget.switchparaoffset == 1))  
				{
					   if (DI == dz_di1[2])
						 {  
							 g_switchparaget.readflag = 0x55;
		     	SwitchToEqpNo(m_oldeqpno);				 
						 }
				}
				else if(DI == dz_di1[g_switchparaget.switchparaoffset])
				{
 					g_switchparaget.readflag = 0x55;
			SwitchToEqpNo(m_oldeqpno);
				}
		}
    }
}

void CDLQ::Time4bytetoCalClock(BYTE *bTime4,VCalClock *CalCloc)
{
	VSysClock SysTime;
	GetSysClock((void *)&SysTime, SYSCLOCK);
	SysTime.wMSecond = (bTime4[0]+bTime4[1]*256)%1000;
	SysTime.bySecond = (bTime4[0]+bTime4[1]*256)/1000;
	SysTime.byMinute = bTime4[2];
	SysTime.byHour = bTime4[3];

	ToCalClock(&SysTime, CalCloc);
	
}

/***************************************************************
	Function：	DoReceviceYc
				接收到的遥测数据处理
	参数:
	返回：	无
***************************************************************/
//BOOL CDLQ::DoReceviceYc(BYTE bAddr,BYTE bInf,WORD bData)
//{
//	return TRUE;
//}

/***************************************************************
	Function：	WriteData
				发送数据
	参数:
	返回：	无
***************************************************************/
void CDLQ::WriteData(void)
{
	WriteToComm(GetEqpAddr());
}
void CDLQ::Clear_All_Data()
{
    SendFrameHead(0x1b, 0);
    m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x0c;

    memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)OPERRATE,4);
    m_SendBuf.wWritePtr +=4;
    memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)PASSWORD,4);
    m_SendBuf.wWritePtr +=4;
    memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)Clear,4);
    m_SendBuf.wWritePtr +=4;

    SendFrameTail();
}
BYTE CDLQ::ChkSum(BYTE *buf, WORD len)
{
	WORD checksum, i;

	checksum = 0;
	for (i = 0; i < len; i++)
	{
		checksum = checksum + *buf;
		buf ++;
		
	}
	return LOBYTE(checksum & 0xff);
}

void CDLQ::SendFrameTail()
{
   WORD len = m_SendBuf.wWritePtr - 10;
   BYTE sum;
   Add_data_three(m_SendBuf.pBuf+10,len);
   sum = ChkSum(m_SendBuf.pBuf,m_SendBuf.wWritePtr);
   m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = sum;
   m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
   WriteData();
   m_bPollMasterSendFlag = FALSE;
}

void CDLQ::SendFrameHead(BYTE FunCode, BYTE ShiftAddress)
{
    BYTE addr[6]={0};
    WORD Addr = 0;
	m_SendBuf.wReadPtr = m_SendBuf.wWritePtr = 0;
	m_pSend = (DlqFrame *)m_SendBuf.pBuf;
	
	m_pSend->head = HEAD_;
	if	(ShiftAddress == 0xff)
	{
		memcpy(m_pSend->dev_addr,Broadcase_addr,sizeof(Broadcase_addr));
	}
	else
	{
        Addr = Hex2Bcd2(m_pEqpInfo[m_wEqpNo].wDAddress);
	    addr[0] = LOBYTE(Addr);
		addr[1] = HIBYTE(Addr);
        memcpy(m_pSend->dev_addr,addr,sizeof(addr));
	}
	m_pSend->head2 = HEAD_;
	m_pSend->control = FunCode;
	m_SendBuf.wWritePtr +=9;
}

void CDLQ::SendWriteCmd(BYTE FunCode,DWORD RegAddr, BYTE ShiftAddress, WORD Value)
{
	SendFrameHead(FunCode, ShiftAddress);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x0E;
	
	memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)&RegAddr,4);
	m_SendBuf.wWritePtr +=4;
	memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)PASSWORD,4);
	m_SendBuf.wWritePtr +=4;
	memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)OPERRATE,4);
	m_SendBuf.wWritePtr +=4;
	memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)&Value,2);
	m_SendBuf.wWritePtr +=2;

	SendFrameTail();
}

void CDLQ::YkResult(BOOL Result)
{
	VYKInfo *pYKInfo;
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

	if (Result)
		pYKInfo->Info.byStatus = 0;
	else
	{
		pYKInfo->Info.byStatus = 3;
		ClearEqpFlag(EQPFLAG_YKReq);
	}
	CPPrimary::DoYKRet();
	return;
}
void CDLQ::SendWriteMReg(DWORD RegAddrStart, BYTE RegNum, BYTE ShiftAddress, BYTE *Value)
{
#if 0
	struct VSysClock time1;
    struct VCalClock time2;
    struct VDBSOE soe;
	struct VDBCOS cos;
    GetSysClock((void *)&time1, SYSCLOCK);
    ToCalClock(&time1, &time2);
    
    soe.wNo = 51;
    soe.byValue = SWS_ON;
    memcpy(&soe.Time,&time2,sizeof(VCalClock));
	WriteSSOE(m_wEqpID, 1, &soe);
	cos.wNo = 51;		
	cos.byValue = SWS_ON;
	WriteSCOS(m_wEqpID, 1, &cos);
#endif

	SendFrameHead(0x08, 0xff);
	m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 6;
	memcpy((BYTE *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr],Value,6);
	m_SendBuf.wWritePtr+=6;
	SendFrameTail();
}
void CDLQ::SendSetTime(void)
{
	VSysClock SysTime;
	BYTE pwBuf[6];
	
	GetSysClock((void *)&SysTime, SYSCLOCK);
	
	pwBuf[0] = BYTE2BCD(SysTime.bySecond);
	pwBuf[1] = BYTE2BCD(SysTime.byMinute);
	pwBuf[2] = BYTE2BCD(SysTime.byHour);
	pwBuf[3] = BYTE2BCD(SysTime.byDay);
	pwBuf[4] = BYTE2BCD(SysTime.byMonth);
	pwBuf[5] = BYTE2BCD(BYTE(SysTime.wYear-2000));
	SendWriteMReg(0, 1, 0, pwBuf);
}

void CDLQ::SendReadCmd(BYTE FunCode,DWORD RegAddr, BYTE ShiftAddress, WORD Length)
{  
	SendFrameHead(FunCode, ShiftAddress);
	if(FunCode == 0x19)
	{
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x00;
	}else
	{
		m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x04;
		memcpy((BYTE *)&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)&RegAddr,4);
		m_SendBuf.wWritePtr+=4;
	}
	SendFrameTail();
}

void CDLQ::Add_data_three(BYTE *buf,BYTE len)//+33
{
	for(BYTE i = 0;i<len;i++)
		buf[i] += 0x33;
}
void CDLQ::subtract_data_three(BYTE *buf,BYTE len)//-33
{
	for(BYTE i = 0;i<len;i++)
		buf[i] -= 0x33;
}
void CDLQ::BcdX_To_BinY(BYTE X, BYTE Y, BYTE *DesBin, BYTE *SrcBcd, WORD Multi)
{
	BYTE   i;
	BYTE   TempBuff[4] = {0};
	DWORD  TempU64 = 0;

	for(i = 0; i < X; i++)
	{
		TempBuff[i] = ((SrcBcd[i] >> 4) * 10) + (SrcBcd[i] & 0x0f);
	}
	for(i = X; i > 0; i--)
	{
		TempU64 = TempU64 * 100 + TempBuff[i - 1];
	}
	TempU64 *= Multi;

	if (Y == 8)
	{
		*(DWORD *)DesBin = TempU64;
	}
	else if (Y == 4)
	{
		*(WORD *)DesBin = (DWORD)TempU64;
	}
	else if (Y == 2)
	{
		*(WORD *)DesBin = (DWORD)TempU64;
	}
}

AUTODELAY_RES *CDLQ::Choese_Station()
{
          AUTODELAY_RES *Auto = Auto_Recover;
	for(BYTE k = 0;k<MAXDEVICENUM*MAX_AUTO_RECOVER;k++)
	{
		if(!Auto->Dev_no)
			return Auto;
		else 
			Auto++;
	}
 return NULL;
}
void CDLQ::Write_Auto_Recov(WORD dev_no,WORD no, struct VSysClock *time)
{
    AUTODELAY_RES *Auto;
    Auto = Choese_Station();
    struct VCalClock time2; 
    ToCalClock(time, &time2);
    Auto->VClock.dwMinute = time2.dwMinute;
    Auto->VClock.wMSecond = time2.wMSecond;
    Auto->Dev_no = dev_no;
    Auto->No = no;
    Auto->tick = Get100usCnt();
}

void CDLQ::Deal_bb(BYTE Lenn,BYTE *buf)
{

   struct VDBCOS cos;
   struct VSysClock time;
   struct VCalClock time2;
   GetSysClock((void *)&time, SYSCLOCK);
   ToCalClock(&time, &time2);   

   if( strstr((char*)buf,"JSRCD") == 0)
    {
        	cos.wNo = 52;		//52
        cos.byValue = SWS_OFF;
        WriteSCOS(m_wEqpID, 1, &cos);	   
    } else
      {        	
        cos.wNo = 52;		//52
        cos.byValue = SWS_ON;
        WriteSCOS(m_wEqpID, 1, &cos);	   
    }  
}

void CDLQ::Deal_yc(BYTE Lenn,BYTE *buf,BYTE X,BYTE Y)
{
	BYTE buff[8];
	WORD zhi = 0;
	struct VDBYC_F bYcData;
    struct VDBYC bYcData1;
	if( X == 2)
	{
		for(BYTE i = 0;i<(Lenn/2);i++)
		{
			bYcData.wNo = i;
			BcdX_To_BinY(2, 2, buff,&buf[2*i], 1);
			memcpy(&zhi,buff,2);
			bYcData.fValue = zhi/10.0;
		//	WriteYC(m_wEqpID,1,&bYcData); 
            WriteYC_F(m_wEqpID, 1, &bYcData);
		}
	}else if( X == 3)
	{
		for(BYTE i = 0;i<(Lenn/3);i++)
		{
			bYcData.wNo = i+3;
			BcdX_To_BinY(3, 2, buff,&buf[3*i], 1);
			memcpy(&zhi,buff,2);
			bYcData.fValue = zhi/10.0;
			 WriteYC_F(m_wEqpID,1,&bYcData);  
		}

	}else if(1 == X)
	{
		short a = buf[0];
		bYcData1.wNo =7;
		bYcData1.nValue = a;
		WriteYC(m_wEqpID,1,&bYcData1); 
		
		bYcData1.wNo = 6;
		BcdX_To_BinY(2, 2, buff,&buf[1], 1);
		memcpy(&zhi,buff,2);
		bYcData1.nValue = zhi;
		WriteYC(m_wEqpID,1,&bYcData1);
	}
}


void CDLQ::Value_DD(BYTE * buf,BYTE Len,BYTE flag)
{
	WORD no1 = m_wEqpNo*30+flag;
	//WORD no;
	struct VParaInfo fParaInfo;
    BYTE zhi[20]={0};
    DWORD   cc =0;
    float ff =0.0;
    BYTE ord;

	switch (flag)
	{
	
	case 1:
		fParaInfo.type = USHORT_TYPE; 
		fParaInfo.len = USHORT_LEN;
		memcpy(fParaInfo.valuech,buf,USHORT_LEN);
		break;
	case 0:
		fParaInfo.type = USHORT_TYPE; 
		fParaInfo.len = USHORT_LEN;
		memcpy(fParaInfo.valuech,buf,USHORT_LEN);
		
		break;
		case 2:
			
        case 4:  
        case 7:
        case 10:
			
        case 12:			
        case 14:
        case 16:
        case 17:
		case 19:
		case 21:// 增加
		case 22:
		case 23:
		case 24:
            fParaInfo.type = UTINY_TYPE; 
			fParaInfo.len = UTINY_LEN;
            ord = buf[0];
			fParaInfo.valuech[0] = (ord & g_control_flag[flag]) >> g_offset_flag[flag];
			//memcpy(fParaInfo.valuech,&ord,1);
            break;
		case 3:
        case 5:
            fParaInfo.type = UTINY_TYPE; 
			fParaInfo.len = UTINY_LEN;
            ord = (buf[0]>>4)*10+buf[0]&0x0f;
			memcpy(fParaInfo.valuech,&ord,1);
            break;
/*        case 19:
            fParaInfo.type = UTINY_TYPE; 
			fParaInfo.len = UTINY_LEN;
           // ord = (buf[0]>>4)*10+buf[0]&0x0f;
			memcpy(fParaInfo.valuech,&ord,1);
            break;
*/
        case 8:
            fParaInfo.type = FLOAT_TYPE; 
			fParaInfo.len = FLOAT_LEN;
            BcdX_To_BinY(3, 4, zhi, buf, 1);
            memcpy(&cc,zhi,4);
            ff = cc/10.0;
            memcpy(fParaInfo.valuech,&ff,sizeof(float));
            break;
        case 9:
            ff = (buf[0]>>4)*10+buf[0]&0x0f;
            fParaInfo.type = UTINY_TYPE; 
				fParaInfo.len = UTINY_LEN;
				fParaInfo.valuech[0] = ff;
            break;
        case 13:
        case 11:
        case 15:
            fParaInfo.type = FLOAT_TYPE; 
			fParaInfo.len = FLOAT_LEN;
            BcdX_To_BinY(2, 2, zhi, buf, 1);
            memcpy(&cc,zhi,4);
            ff = cc/10.0;
            memcpy(fParaInfo.valuech,&ff,sizeof(float));
            break;
        case 6:
            fParaInfo.type = USHORT_TYPE; 
			fParaInfo.len = USHORT_TYPE;
			cc = BCD2BYTE(buf[0])+BCD2BYTE(buf[1])*100;
            memcpy(fParaInfo.valuech,&cc,sizeof(USHORT_TYPE));
            break;
        case 18:
            fParaInfo.type = STRING_TYPE; 
			fParaInfo.len = 7;
			sprintf((char *)zhi,"%x%x%x",buf[2],buf[1],buf[0]);
            memcpy(fParaInfo.valuech,zhi,7);    
           break;
        case 20:
            break;//遥信复归时间    
	}
	if(gSwitchParaOpt == NULL)
			return ;
	
	memcpy(&SwitchParaInfo[no1].type,&fParaInfo,sizeof(struct VParaInfo));//add xhp
}

void CDLQ::DoYkData(BYTE * buf,BYTE Len,BYTE re)
{	 
	bool res;

	if(re == 0x94) 
		res = TRUE;
	else if(re == 0xD4)
		res = FALSE;
	
	YkResult(res);
}

void CDLQ::DoYx_alarm1(BYTE * buf,BYTE Len)//保护动作事件记录
{
	BYTE DO_Event = buf[0];
	BYTE bYXstate = SWS_ON;
    BYTE buff[100]={0};
    BYTE flag =1;
    memcpy(buff,&buf[1],Len-1);
    struct VSysClock time1;
    struct VCalClock time2;    
    struct VDBSOE soe;
	struct VDBCOS cos;
    time_trans(&time1,&buf[2]);
    ToCalClock(&time1, &time2);  
//    GetSysClock(&time2,CALCLOCK);

	switch (DO_Event)
	{
		case 2:
            Write_Event(7,bYXstate,buff,Len-1,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:剩余电流保护动作",m_wEqpID);
			soe.wNo = 7;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 7;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, 1, &cos);
			Write_Auto_Recov(m_wEqpID,7, &time1);
			
			break;
		case 22:
            Write_Event(9,bYXstate,buff,Len-1,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:短路瞬时保护动作",m_wEqpID);
			soe.wNo = 9;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 9;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, 1, &cos);
			Write_Auto_Recov(m_wEqpID,9,&time1);

			break;
		case 6:
            Write_Event(8,bYXstate,buff,Len-1,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:短路短延时保护动作",m_wEqpID);
			soe.wNo = 8;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 8;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, 1, &cos);
			Write_Auto_Recov(m_wEqpID,8,&time1);

			break;
		case 5:
             Write_Event(10,bYXstate,buff,Len-1,flag);
             WriteActEvent(NULL, 0, 0, "开关%d:过载保护动作",m_wEqpID);
			 soe.wNo = 10;
			 soe.byValue = bYXstate;
			 memcpy(&soe.Time,&time2,sizeof(VCalClock));
			 WriteSSOE(m_wEqpID, 1, &soe);
			 cos.wNo = 10;		 
			 cos.byValue = bYXstate;
			 WriteSCOS(m_wEqpID, 1, &cos);
			 Write_Auto_Recov(m_wEqpID,10,&time1);

			break;
		case 8:
             Write_Event(12,bYXstate,buff,Len-1,flag);
             WriteActEvent(NULL, 0, 0, "开关%d:欠压保护动作",m_wEqpID);
			 soe.wNo = 12;
			 soe.byValue = bYXstate;
			 memcpy(&soe.Time,&time2,sizeof(VCalClock));
			 WriteSSOE(m_wEqpID, 1, &soe);
			 cos.wNo = 12;		 
			 cos.byValue = bYXstate;
			 WriteSCOS(m_wEqpID, 1, &cos);
			 Write_Auto_Recov(m_wEqpID,12,&time1);

			break;
		case 9:
             Write_Event(11,bYXstate,buff,Len-1,flag);
             WriteActEvent(NULL, 0, 0, "开关%d:过压保护动作",m_wEqpID);
			 soe.wNo = 11;
			 soe.byValue = bYXstate;
			 memcpy(&soe.Time,&time2,sizeof(VCalClock));
			 WriteSSOE(m_wEqpID, 1, &soe);
			 cos.wNo = 11;		 
			 cos.byValue = bYXstate;
			 WriteSCOS(m_wEqpID, 1, &cos);
			 Write_Auto_Recov(m_wEqpID,11,&time1);

			break;
		case 4:
             Write_Event(13,bYXstate,buff,Len-1,flag);
             WriteActEvent(NULL, 0, 0, "开关%d:缺零保护动作",m_wEqpID);
			 soe.wNo = 13;
			 soe.byValue = bYXstate;
			 memcpy(&soe.Time,&time2,sizeof(VCalClock));
			 WriteSSOE(m_wEqpID, 1, &soe);
			 cos.wNo = 13;		 
			 cos.byValue = bYXstate;
			 WriteSCOS(m_wEqpID, 1, &cos);
             
			 Write_Auto_Recov(m_wEqpID,13,&time1);
			break;
		case 7:
             Write_Event(14,bYXstate,buff,Len-1,flag);
             WriteActEvent(NULL, 0, 0, "开关%d:缺相保护动作",m_wEqpID);
			 soe.wNo = 14;
			 soe.byValue = bYXstate;
			 memcpy(&soe.Time,&time2,sizeof(VCalClock));
			 WriteSSOE(m_wEqpID, 1, &soe);
			 cos.wNo = 14;		 
			 cos.byValue = bYXstate;
			 WriteSCOS(m_wEqpID, 1, &cos);
			 Write_Auto_Recov(m_wEqpID,14,&time1);

			break;
       default:
           WriteActEvent(NULL, 0, 0, "开关%d:默认告警1   %x",m_wEqpID,DO_Event); 
           break;

	}
}

void CDLQ::WriteParaSOE(int oldpara, int newpara, struct VDBSOE *soe1, struct VDBSOE *soe2)
{
	struct VDBCOS cos1,cos2;

    cos1.wNo = soe1->wNo;
    cos2.wNo = soe2->wNo;

    if (newpara == 0x01)
    {
        cos1.byValue = soe1->byValue = 0x01;

        cos2.byValue = soe2->byValue = 0x81;
        WriteSSOE(m_wEqpID, 1, soe2);			
	    WriteSCOS(m_wEqpID, 1, &cos2);
        if (oldpara == 0x02)
        {
            WriteSSOE(m_wEqpID, 1, soe1);			
			WriteSCOS(m_wEqpID, 1, &cos1);
        }
    }
    else if (newpara == 0x2)
    {
        cos1.byValue = soe1->byValue = 0x81;
        cos2.byValue = soe2->byValue = 0x01;
        WriteSSOE(m_wEqpID, 1, soe1);			
	    WriteSCOS(m_wEqpID, 1, &cos1);
        if (oldpara == 0x01)
        {
            WriteSSOE(m_wEqpID, 1, soe2);			
	        WriteSCOS(m_wEqpID, 1, &cos2);
        }
    }
    else
    {
        cos1.byValue = soe1->byValue = 0x01;
        cos2.byValue = soe2->byValue = 0x01;
        if (oldpara == 0x01)
        {
            WriteSSOE(m_wEqpID, 1, soe2);			
	        WriteSCOS(m_wEqpID, 1, &cos2);
        }
        else if (oldpara == 0x02)
        {
            WriteSSOE(m_wEqpID, 1, soe1);			
	        WriteSCOS(m_wEqpID, 1, &cos1);
        }
    }
}

void CDLQ::DoYx_alarm2(BYTE * buf,BYTE Len)
{
    struct VSysClock time1;
    struct VCalClock time2;
    struct VDBSOE soe1,soe2;
    time_trans(&time1,buf);
    ToCalClock(&time1, &time2);
	buf +=6;
	BYTE DO_Event = buf[0];
    BYTE oldpara = (buf[0]>>2)&0x3;
    BYTE newpara = (buf[0]&0x3);

	switch (DO_Event&0xf0)
	{
		case 16:
            soe1.wNo = 34;
            soe2.wNo = 42;
            memcpy(&soe1.Time,&time2,sizeof(VCalClock));
            memcpy(&soe2.Time,&time2,sizeof(VCalClock));
            WriteParaSOE(oldpara, newpara, &soe1, &soe2);
			WriteActEvent(NULL, 0, 0, "开关%d:剩余电流保护功能变化0x%x,0x%x",m_wEqpID,oldpara, newpara);
			break;
		case 32:
            soe1.wNo = 36;
            soe2.wNo = 44;
            memcpy(&soe1.Time,&time2,sizeof(VCalClock));
            memcpy(&soe2.Time,&time2,sizeof(VCalClock));
            WriteParaSOE(oldpara, newpara, &soe1, &soe2);
			WriteActEvent(NULL, 0, 0, "开关%d:短路瞬时功能变化0x%x,0x%x",m_wEqpID,oldpara, newpara);
			break;
		case 48:
            soe1.wNo = 35;
            soe2.wNo = 43;
            memcpy(&soe1.Time,&time2,sizeof(VCalClock));
            memcpy(&soe2.Time,&time2,sizeof(VCalClock));
            WriteParaSOE(oldpara, newpara, &soe1, &soe2);
			WriteActEvent(NULL, 0, 0, "开关%d:短路短延时功能变化0x%x,0x%x",m_wEqpID,oldpara, newpara);
			break;
		case 64:
            soe1.wNo = 37;
            soe2.wNo = 45;
            memcpy(&soe1.Time,&time2,sizeof(VCalClock));
            memcpy(&soe2.Time,&time2,sizeof(VCalClock));
            WriteParaSOE(oldpara, newpara, &soe1, &soe2);
			WriteActEvent(NULL, 0, 0, "开关%d:过载功能变化0x%x,0x%x",m_wEqpID,oldpara, newpara);
			break;
		case 96:
            soe1.wNo = 39;
            soe2.wNo = 47;
            memcpy(&soe1.Time,&time2,sizeof(VCalClock));
            memcpy(&soe2.Time,&time2,sizeof(VCalClock));
            WriteParaSOE(oldpara, newpara, &soe1, &soe2);
			 WriteActEvent(NULL, 0, 0, "开关%d:欠压功能变化0x%x,0x%x",m_wEqpID,oldpara, newpara);
			break;
		case 80:
            soe1.wNo = 38;
            soe2.wNo = 46;
            memcpy(&soe1.Time,&time2,sizeof(VCalClock));
            memcpy(&soe2.Time,&time2,sizeof(VCalClock));
            WriteParaSOE(oldpara, newpara, &soe1, &soe2);
			 WriteActEvent(NULL, 0, 0, "开关%d:过压功能变化0x%x,0x%x",m_wEqpID,oldpara, newpara);
			break;
		case 128:
            soe1.wNo = 40;
            soe2.wNo = 48;
            memcpy(&soe1.Time,&time2,sizeof(VCalClock));
            memcpy(&soe2.Time,&time2,sizeof(VCalClock));
            WriteParaSOE(oldpara, newpara, &soe1, &soe2);
			 WriteActEvent(NULL, 0, 0, "开关%d:缺零功能变化0x%x,0x%x",m_wEqpID,oldpara, newpara);
			break;
		case 112:
            soe1.wNo = 41;
            soe2.wNo = 49;
            memcpy(&soe1.Time,&time2,sizeof(VCalClock));
            memcpy(&soe2.Time,&time2,sizeof(VCalClock));
            WriteParaSOE(oldpara, newpara, &soe1, &soe2);
            WriteActEvent(NULL, 0, 0, "开关%d:断相功能变化0x%x,0x%x",m_wEqpID,oldpara, newpara);
			break;
          default:
           WriteActEvent(NULL, 0, 0, "开关%d:默认告警2    %x",m_wEqpID,DO_Event&0xf0); 
           break; 

	}
}
void CDLQ::Write_Event(WORD yxNo,WORD Yx_va,BYTE *buf,BYTE len,BYTE flag)
{
    SwitchEvent evt;
    DWORD vv[8] = {0};
    BYTE val[4]={0};
    DWORD bValue = 0;
    BYTE  phase = 0;
    if(flag ==1 || flag ==2 || flag == 3)
    {
       
        (evt.Time)->dwMinute = buf[2];
        (evt.Time)->wMSecond = buf[1];
        buf +=7;
    }else if(flag == 4)
    {
        (evt.Time)->dwMinute = buf[2];
        (evt.Time)->wMSecond = buf[1];
        buf +=8;
    }
    
    BcdX_To_BinY(2,2,val,buf,1);
    bValue = (val[0]+val[1]*256)*10;
    buf +=2;
    BcdX_To_BinY(2,2,val,buf,1);
    memcpy(&vv[0],val,2);
    buf +=2;
    BcdX_To_BinY(2,2,val,buf,1);
    memcpy(&vv[1],val,2);
    buf +=2;
    BcdX_To_BinY(2,2,val,buf,1);
    memcpy(&vv[2],val,2);
    buf +=2;
    BcdX_To_BinY(3,4,val,buf,1);
    memcpy(&vv[3],val,2);
    buf +=3;
    BcdX_To_BinY(3,4,val,buf,1);
    memcpy(&vv[4],val,2);
    buf +=3;
    BcdX_To_BinY(3,4,val,buf,1);
    memcpy(&vv[5],val,2);
    buf +=3;
    
    memcpy(&vv[6],&bValue,2);
    
    phase = buf[0]*10;
    memcpy(&vv[7],&phase,1);

    evt.YxNum =1;
    evt.YxNo[0] = m_wEqpNo*53 + yxNo+129;
    evt.YxValue[0] = Yx_va ;
    evt.YcNum = 8;
    for(BYTE i =0;i<8;i++)
        evt.YcNo[i]= m_wEqpNo*8+i+16897;
    for(BYTE j = 0;j<8;j++)
        memcpy(&evt.YcValue[j],&vv[j],sizeof(DWORD));
    SwitchWriteGzEvent(evt);
}
void CDLQ::DoYx_alarm3(BYTE * buf,BYTE Len)
{
    struct VSysClock time1;
    struct VCalClock time2;
    struct VDBSOE soe;
    struct VDBCOS cos;
    time_trans(&time1,&buf[3]);
    ToCalClock(&time1, &time2);
    
//    GetSysClock(&time2,CALCLOCK);
    
	BYTE DO_Event = buf[0];
	BYTE bYXstate;
	BYTE bYXs = SWS_ON;
    bYXstate = bYXs;
    BYTE buff[100]={0};
    BYTE flag =3;
    memcpy(buff,&buf[2],Len-2);
    shellprintf("DO_Event = %x\n ,case = %x",DO_Event,buf[1]&0x1f);
	if(DO_Event == 2)
	{

        cos.wNo = soe.wNo = 6;
        cos.byValue =  soe.byValue = SWS_ON;
        memcpy(&soe.Time,&time2,sizeof(VCalClock));
        WriteSSOE(m_wEqpID, 1, &soe);
        WriteSCOS(m_wEqpID, 1, &cos);

        cos.wNo = soe.wNo = 5;
        cos.byValue = soe.byValue = SWS_OFF;;
        memcpy(&soe.Time,&time2,sizeof(VCalClock));
        WriteSSOE(m_wEqpID, 1, &soe);
        WriteSCOS(m_wEqpID, 1, &cos);
          
        WriteActEvent(NULL, 0, 0, "开关%d:闸位变化(合->分)",m_wEqpID);
    }
    else if(DO_Event == 1)
    {
            cos.wNo = soe.wNo =6;
            cos.byValue = soe.byValue = SWS_OFF;
            memcpy(&soe.Time,&time2,sizeof(VCalClock));
            WriteSSOE(m_wEqpID, 1, &soe); 
            WriteSCOS(m_wEqpID,1, &cos);

            cos.wNo = soe.wNo =5;
            cos.byValue = soe.byValue = SWS_ON;
            memcpy(&soe.Time,&time2,sizeof(VCalClock));
            WriteSSOE(m_wEqpID, 1, &soe); 
            WriteSCOS(m_wEqpID,1, &cos);
        	  WriteActEvent(NULL, 0, 0, "开关%d:闸位分->合",m_wEqpID); 
    }
    	  switch (buf[1]&0x1f)
    	  {
#if 0
   		 case 2:
                soe.wNo = 34;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 34;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
    			 Write_Event(34,bYXs,buff,Len-2,flag);
                 WriteActEvent(NULL, 0, 0, "剩余电流->闸位分");
    			break;
    		 case 4:
                soe.wNo = 40;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 40;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
    		    Write_Event(40,bYXs,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "缺零->闸位分");
    			 break;
    		 case 5:
                soe.wNo = 37;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 37;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
    			 Write_Event(37,bYXs,buff,Len-2,flag);
                 WriteActEvent(NULL, 0, 0, "过载->闸位分");
    			break;
    		 case 6:
                soe.wNo = 35;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 35;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
    			Write_Event(35,bYXs,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "短路短延时->闸位分");
    			break;
    		 case 7:
                soe.wNo = 41;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 41;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
                Write_Event(41,bYXs,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "缺相->闸位分");
    			break;
    		 case 8:
                soe.wNo = 39;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 39;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
    			 Write_Event(39,bYXs,buff,Len-2,flag);
                 WriteActEvent(NULL, 0, 0, "欠压->闸位分");
    			break;		  
    		 case 9:
                soe.wNo = 38;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 38;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
        		 Write_Event(38,bYXs,buff,Len-2,flag);
                 WriteActEvent(NULL, 0, 0, "过压->闸位分");
    			break;
    	#endif
    		 case 13:
                soe.wNo = 33;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 33;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
    			Write_Auto_Recov(m_wEqpID,33,&time1);
                Write_Event(33,bYXs,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "开关%d:远程试验->闸位分",m_wEqpID);
    			break;
    		 case 14:
                soe.wNo =32;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 32;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
    			Write_Auto_Recov(m_wEqpID,32,&time1);
                Write_Event(32,bYXs,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "开关%d:按键试验->闸位分",m_wEqpID);
    			break;
    		case 15:
                soe.wNo = 25;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 25;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
                Write_Auto_Recov(m_wEqpID,25,&time1);
                Write_Event(25,bYXs,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "开关%d:重合闸闭锁",m_wEqpID);
    		   break;
 #if 0
             case 22:
                soe.wNo =37;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);
				cos.wNo = 37;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
                Write_Event(37,bYXs,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "短路瞬时->闸位分");
    			break;
#endif
    		 case 30:
                soe.wNo =27;
                soe.byValue = SWS_ON;;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe);  
				cos.wNo = 27;		
				cos.byValue = SWS_ON;
				WriteSCOS(m_wEqpID, 1, &cos);
                Write_Event(27,bYXs,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "开关%d:软遥控操作",m_wEqpID);
    			break;
    		 case 29:
                cos.wNo = soe.wNo =26;
                cos.byValue = soe.byValue = SWS_ON;
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe); 
				WriteSCOS(m_wEqpID, 1, &cos);
                Write_Event(26,bYXs,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "开关%d:硬遥控动作",m_wEqpID); 
    			break;
                
             case 18:
                soe.wNo =28;
                soe.byValue = SWS_ON;;
                cos.wNo = 28;
                cos.byValue = SWS_ON;
                WriteSCOS(m_wEqpID, 1, &cos);
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe); 
								Write_Auto_Recov(m_wEqpID,28,&time1);
                Write_Event(28,bYXstate,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "开关%d:手动操作",m_wEqpID); 
    			break;

                
    	      case 24:
                soe.wNo =24;
                soe.byValue = SWS_ON;;
                cos.wNo = 24;
                cos.byValue = SWS_ON;
                WriteSCOS(m_wEqpID, 1, &cos);
                memcpy(&soe.Time,&time2,sizeof(VCalClock));
                WriteSSOE(m_wEqpID, 1, &soe); 
    			Write_Auto_Recov(m_wEqpID,24,&time1);
                Write_Event(24,bYXstate,buff,Len-2,flag);
                WriteActEvent(NULL, 0, 0, "开关%d:重合闸",m_wEqpID); 
    			break;
              default:
               WriteActEvent(NULL, 0, 0, "开关%d:默认告警3  DO_Event=  %x %x",m_wEqpID,DO_Event,buf[1]&0x1f); 
               break; 

     }
}
void CDLQ::DoYx_alarm4(BYTE * buf,BYTE Len)
{ 
	BYTE DO_Event = buf[0]&0x01;
	BYTE bYXstate;
    BYTE buff[100]={0};
    BYTE flag =2;
    memcpy(buff,&buf[2],Len-2);
	if(DO_Event&0x01)
		bYXstate = SWS_ON;
	else 
	   bYXstate = SWS_OFF; 
    struct VSysClock time1;
    struct VCalClock time2;    
    struct VDBSOE soe;
	struct VDBCOS cos;
    time_trans(&time1,&buf[3]);
    ToCalClock(&time1, &time2);    
	switch (buf[1])
	{
		case 1:
			Write_Event(15,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:剩余电流预警",m_wEqpID); 
			
			soe.wNo = 15;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 15;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, 1, &cos);
			break;
		case 2:
			Write_Event(16,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:剩余电流告警",m_wEqpID); 
	
			soe.wNo = 16;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 16;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, 1, &cos);
		   break;
		case 4:
			Write_Event(22,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:缺零告警",m_wEqpID); 
			soe.wNo = 22;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 22;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, m_wEqpID, &cos);

		  break;
		case 5:
			Write_Event(19,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:过载告警",m_wEqpID); 
			soe.wNo = 19;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 19;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, m_wEqpID, &cos);
		  break;
		case 6:
			Write_Event(17,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:短路短延时告警",m_wEqpID); 
			soe.wNo = 17;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 17;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, 1, &cos);

		  break;
		case 7:
			//WriteRangeSYX(m_wEqpID,23,1,&bYXstate); //缺相
			Write_Event(23,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:缺相告警",m_wEqpID); 
			soe.wNo = 23;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 23;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, m_wEqpID, &cos);
		  break;
		case 8:
			Write_Event(21,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:欠压告警",m_wEqpID); 
		   soe.wNo = 21;
		   soe.byValue = bYXstate;
		   memcpy(&soe.Time,&time2,sizeof(VCalClock));
		   WriteSSOE(m_wEqpID, 1, &soe);
		   cos.wNo = 21;	   
		   cos.byValue = bYXstate;
		   WriteSCOS(m_wEqpID, m_wEqpID, &cos);			
		  break;
		case 9:
			Write_Event(20,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:过压告警",m_wEqpID); 
		   soe.wNo = 20;
		   soe.byValue = bYXstate;
		   memcpy(&soe.Time,&time2,sizeof(VCalClock));
		   WriteSSOE(m_wEqpID, 1, &soe);
		   cos.wNo = 20;	   
		   cos.byValue = bYXstate;
		   WriteSCOS(m_wEqpID, m_wEqpID, &cos);			
		 break;
		//case 10:
		// break;
		case 22:
			Write_Event(18,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:短路瞬时告警",m_wEqpID); 
		   soe.wNo = 18;
		   soe.byValue = bYXstate;
		   memcpy(&soe.Time,&time2,sizeof(VCalClock));
		   WriteSSOE(m_wEqpID, 1, &soe);
		   cos.wNo = 18;	   
		   cos.byValue = bYXstate;
		   WriteSCOS(m_wEqpID, m_wEqpID, &cos);			
		  break;
		case 17:
			Write_Event(29,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:合闸失败告警",m_wEqpID); 
			soe.wNo = 29;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 29;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, 1, &cos);

			break;
		case 23:
			Write_Event(30,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:跳闸失败告警",m_wEqpID); 
			soe.wNo = 30;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 30;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, 1, &cos);
		   break;
		case 16:
			Write_Event(31,bYXstate,buff,Len-2,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:互感器故障告警",m_wEqpID); 
			soe.wNo = 31;
			soe.byValue = bYXstate;
			memcpy(&soe.Time,&time2,sizeof(VCalClock));
			WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 31;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, 1, &cos);			
		   break;
       default:
           WriteActEvent(NULL, 0, 0, "开关%d:默认告警4    %x",m_wEqpID,buf[1]); 
           break;
	}

}
void CDLQ::DoYx_alarm5(BYTE * buf,BYTE Len)
{
    struct VSysClock time1;
    struct VCalClock time2;
    struct VDBSOE soe;
    struct VDBCOS cos;
    time_trans(&time1,buf);
    ToCalClock(&time1, &time2);

    soe.wNo = 3;
    soe.byValue = SWS_ON;
    memcpy(&soe.Time,&time2,sizeof(VCalClock));
	WriteSSOE(m_wEqpID, 1, &soe);
	cos.wNo = 3;		
	cos.byValue = SWS_ON;
	WriteSCOS(m_wEqpID, 1, &cos);



    soe.wNo = 4;
    soe.byValue = SWS_OFF;
    memcpy(&soe.Time,&time2,sizeof(VCalClock));
	WriteSSOE(m_wEqpID, 1, &soe);
	cos.wNo = 4;		
	cos.byValue = SWS_OFF;
	WriteSCOS(m_wEqpID, 1, &cos);


    time_trans(&time1,&buf[6]);
    ToCalClock(&time1, &time2);

    soe.wNo = 4;
    soe.byValue = SWS_ON;
    memcpy(&soe.Time,&time2,sizeof(VCalClock));
	WriteSSOE(m_wEqpID, 1, &soe);
	cos.wNo = 4;		
	cos.byValue = SWS_ON;
	WriteSCOS(m_wEqpID, 1, &cos);
      
    soe.wNo = 3;
    soe.byValue = SWS_OFF;
    memcpy(&soe.Time,&time2,sizeof(VCalClock));
	WriteSSOE(m_wEqpID, 1, &soe);
	cos.wNo = 3;		
	cos.byValue = SWS_OFF;
	WriteSCOS(m_wEqpID, 1, &cos);
    
}	
void CDLQ::DoYx_alarm(BYTE * buf,BYTE Len)
{
	 BYTE  Result = buf[0];
	 BYTE	bYXstate;
     BYTE buff[100]={0};
     BYTE flag =4;
     memcpy(buff,buf,Len);
    struct VSysClock time1;
    struct VCalClock time2;
    struct VDBSOE soe;
	struct VDBCOS cos;
    time_trans(&time1,buf);
    ToCalClock(&time1, &time2);

     m_Dlq_slef[0] = Result;
     m_Dlq_slef[1] = buf[7];

    if(Result != m_Dlq_slef[0])
    {
    	 if(Result == 0)
    	 {
    		bYXstate = SWS_OFF;
    		Write_Event(0,bYXstate,buff,Len,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:自检失败",m_wEqpID);
    	 }
    	 else if(Result == 0x11)
    	 {
    		 bYXstate = SWS_ON;
    		Write_Event(1,bYXstate,buff,Len,flag);
            WriteActEvent(NULL, 0, 0, "开关%d:自检成功",m_wEqpID);
    	 }
            soe.wNo = 0;
            soe.byValue = bYXstate;
            memcpy(&soe.Time,&time2,sizeof(VCalClock));
        	WriteSSOE(m_wEqpID, 1, &soe);
			cos.wNo = 0;		
			cos.byValue = bYXstate;
			WriteSCOS(m_wEqpID, 1, &cos);
    }

	 buf +=7;

	 if(buf[0] == 1)
	 {
		bYXstate = SWS_ON;
		Write_Auto_Recov(m_wEqpID,2,&time1);
        Write_Event(2,bYXstate,buff,Len,flag);
	 }else if(buf[0] == 2)
	 {
		bYXstate = SWS_ON;
		Write_Auto_Recov(m_wEqpID,1,&time1);
        Write_Event(1,bYXstate,buff,Len,flag);
	 }
	 soe.wNo = 2;
	 soe.byValue = bYXstate;
	 memcpy(&soe.Time,&time2,sizeof(VCalClock));
	 WriteSSOE(m_wEqpID, 1, &soe);
	 cos.wNo = 2;		 
	 cos.byValue = bYXstate;
	 WriteSCOS(m_wEqpID, 1, &cos);
}	

void CDLQ::DoYx(BYTE * buf,BYTE Len)
{
    BYTE bYXstate;
	BYTE stat_one =  buf[0];
	struct VDBYX YX;
	m_yx[m_wEqpNo] = SWS_OFF;

	bYXstate = (stat_one>>5)& 0x01;
//	if (bYXstate != m_yx[m_wEqpNo])
//	{
	    YX.wNo = 5;		
		YX.byValue = (bYXstate<<7)|0x01;
		//WriteSCOS(m_wEqpID, 1, &cos);	
		WriteSYX(m_wEqpID,1,&YX);
//	}
	

}
/*bit4-bit0   
00001-剩余电流预警，00010-剩余电流，00100-缺零，00101-过载，
00110-短路短延时，00111-缺相，01000-欠压，01001-过压，
01010-接地，01011-停电，01101-远程试验，01110-按键试验，
01111-闭锁，10010-手动， 10000-互感器故障，10001-合闸失败，
10110-短路瞬时，10111-跳闸失败，10011-设置更改，11110-软遥控，11101-硬遥控*/
void CDLQ::DoYxData(BYTE * buf,BYTE Len)
{
	//0x038D0001,0x038E0001,0x038F0001,0x03910001,0x03920001,0x03930001
        DWORD DI = 0;
    BYTE tmpbuf[50];
    memset(tmpbuf, 0, 50);
    memcpy(tmpbuf,buf,Len);
	subtract_data_three(buf,Len);
	memcpy((BYTE *)&DI,buf,4);
	Len -=4;
	buf += 4;
	switch (DI & 0xffffff00)
	{
		case 0x038D0000:
			DoYx_alarm(buf,Len);
			break;
		case 0x038E0000:
			DoYx_alarm1(buf,Len);
			break;
		case 0x038F0000:
			DoYx_alarm2(buf,Len);
		    break;
		case 0x03910000:
            DoYx_alarm3(buf,Len);
		    break;
	    case 0x03920000:
	        DoYx_alarm4(buf,Len);
			break;
	    case 0x03930000:
           DoYx_alarm5(buf,Len);
        break;
	}	
}
WORD CDLQ::WriteConst_value(BYTE addr,BYTE num,BYTE *buf)
{
	return 0;
}
void CDLQ::Deal_DD(DWORD di)
{ 
	SendReadCmd(SJ_CODE,di,0,0);
}
BYTE CDLQ::HexToBcd(BYTE HexNum)
{
    BYTE BcdNum = 0;

    BcdNum = (HexNum / 10);
    BcdNum <<= 4;
    BcdNum += (HexNum % 10);

    return (BcdNum);
}

WORD CDLQ::Hex2Bcd2(WORD HexNum)
{
    WORD BcdNum = 0;

    BcdNum = HexToBcd(HexNum / 100) * 0x100;
    BcdNum += HexToBcd(HexNum % 100);

    return (BcdNum);
}
DWORD CDLQ::Hex2Bcd4(DWORD HexNum)
{
    DWORD BcdNum = 0;

    BcdNum = (DWORD)Hex2Bcd2(HexNum / 10000) * 0x10000;
    BcdNum += Hex2Bcd2(HexNum % 10000);

    return (BcdNum);
}

void CDLQ::Get_Dw(BYTE dw,BYTE *z_dw,BYTE *t_dw)
{
    BYTE tt = dw>>4;
    switch (tt)
    {
        case 0:
            *z_dw = 0;
            break;
        case 1:
            *z_dw = 1;
            break;
       case 2:
            *z_dw = 2;
            break;
        case 3:
            *z_dw = 3;
            break;
        case 4:
            *z_dw = 4;
            break;
        case 5:
            *z_dw = 5;
            break;
        case 6:
            *z_dw = 6;
            break;
        case 7:
            *z_dw = 7;
            break;
        default :
            break;

    }
	if((!(dw & 0x08)) && dw & (!(dw & 0x04)))
		*t_dw = 0;
	else if((!(dw & 0x08)) && dw & ((dw & 0x04)))   
		*t_dw = 1;
	else if(((dw & 0x08)) && dw & (!(dw & 0x04)))
		*t_dw = 2;  
}
void CDLQ::Dz_makeFram(SwitchPara *para,DWORD Di)
{
	WORD zhi;
	WORD t;
	BYTE k;
    BYTE one_;
		DWORD tt1 = 0;
	DWORD t2 = 0;
	BYTE z_index,t_index = 0;
	float zz= 0.0;
	BYTE tempbuf[20];
	Get_Dw(m_DW_4,&z_index,&t_index);
	switch (Di)
	{
     case DI_CAS_1:
			m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x20;
			memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)&Di,4);
			m_SendBuf.wWritePtr +=4;
			memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)PASSWORD,4);
			m_SendBuf.wWritePtr +=4;
			memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)OPERRATE,4);
			m_SendBuf.wWritePtr +=4;
			zhi = MAKEWORD(para->valuech[0], para->valuech[1]);
			t = Hex2Bcd2(zhi);
			for(k = 0;k<20;k++)
				m_SendBuf.pBuf[m_SendBuf.wWritePtr+k] = 0xff;
			memcpy(&m_SendBuf.pBuf[22+z_index*2],&t,2);
			m_SendBuf.wWritePtr +=20;
            break;
        case DI_CAS_2:
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x16;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)&Di,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)PASSWORD,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)OPERRATE,4);
            m_SendBuf.wWritePtr +=4;
            zhi = MAKEWORD(para->valuech[0], para->valuech[1]);
            t = Hex2Bcd2(zhi);
            for(k = 0;k<10;k++)
                m_SendBuf.pBuf[m_SendBuf.wWritePtr+k] = 0xff;
            memcpy(&m_SendBuf.pBuf[22+t_index*2],&t,2);
            m_SendBuf.wWritePtr +=10;
            break;
        case DI_CAS_3:
	    case DI_CAS_5:		
		case DI_CAS_11:
		case DI_CAS_15:
				m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x0D;
				memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)&Di,4);
				m_SendBuf.wWritePtr +=4;
				memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)PASSWORD,4);
				m_SendBuf.wWritePtr +=4;
				memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)OPERRATE,4);
				m_SendBuf.wWritePtr +=4;
				m_SendBuf.pBuf[m_SendBuf.wWritePtr] = para->valuech[0];
				m_SendBuf.wWritePtr +=1;
			break;

		case DI_CAS_4: 
        case DI_CAS_6:
        case DI_CAS_9:

            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x0D;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)&Di,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)PASSWORD,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)OPERRATE,4);
            m_SendBuf.wWritePtr +=4;
            one_ = HexToBcd(para->valuech[0]);
            m_SendBuf.pBuf[m_SendBuf.wWritePtr] = one_;
            m_SendBuf.wWritePtr +=1;
        break;
        case DI_CAS_7:
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x0E;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)&Di,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)PASSWORD,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)OPERRATE,4);
            m_SendBuf.wWritePtr +=4;
            zhi = MAKEWORD(para->valuech[0], para->valuech[1]);
			t = Hex2Bcd2(zhi);
			//t = HexToBcd(zhi);
           // memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],&t,1);
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = t&0xff;
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = (BYTE)(t>>8);			
            break;
        case DI_CAS_8:
        case DI_CAS_10:
        case DI_CAS_12:
        case DI_CAS_13:
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x0F;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)&Di,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)PASSWORD,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)OPERRATE,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&zz,para->valuech,4);
            tt1 = zz*10;
            t2 = Hex2Bcd4(tt1);
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],&t2,3);
            m_SendBuf.wWritePtr +=3;
            break;
		case DI_CAS_14:
            m_SendBuf.pBuf[m_SendBuf.wWritePtr++] = 0x0F;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)&Di,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)PASSWORD,4);
            m_SendBuf.wWritePtr +=4;
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],(BYTE *)OPERRATE,4);
            m_SendBuf.wWritePtr +=4;
						tempbuf[2]= (para->valuech[0] - '0')*16 + (para->valuech[1] - '0');
						tempbuf[1]= (para->valuech[2] - '0')*16 + (para->valuech[3] - '0');
						tempbuf[0]= (para->valuech[4] - '0')*16 + (para->valuech[5] - '0');
         //   memcpy(tbuf,para->valuech,6);
					//	tbuf[6] = '\0';
					//	sscanf(tbuf,"%02x%02x%02x",&tempbuf[0],&tempbuf[1],&tempbuf[2]);
            memcpy(&m_SendBuf.pBuf[m_SendBuf.wWritePtr],tempbuf,3);
            m_SendBuf.wWritePtr +=3;
			
			break;
		case DI_CAS_16:
				g_Time_reco = (para->valuech[0]+para->valuech[1]*256)-1;
				break;

			
    	}
}
void CDLQ::Set_value_trans(DWORD di,BYTE *addr,SwitchPara*para)
{
    SendFrameHead(0x14, 0);
    memcpy(m_pSend->dev_addr,addr,sizeof(addr));
	Dz_makeFram(para,di);
	SendFrameTail();  
}
void CDLQ::Set_Const_Val(DWORD di,WORD addr,SwitchPara *apara)
{
   BYTE buf[6]={0};
   WORD addr1;

   addr1 = Hex2Bcd2(addr);
   buf[0] = LOBYTE(addr1);
   buf[1] = HIBYTE(addr1);
   Set_value_trans(di,buf,apara);
}
/*
bit	7	6	5	4	3	2	1~0
7自检事件
6异常告警发生/恢复
5闸位状态变化	
4保护功能退出/恢复	
3高压线路失/复电
2保护动作
标识	保留
*/
DWORD CDLQ::Get_DI_yx(BYTE * Index)
{
    DWORD DI = 0;
 if(m_Station_t&0x80)
    {
        DI = yx_Di[0];//zijian
        m_Station_t &= ~0x80;
        *Index = 0; 
    }
    else if(m_Station_t&0x40)
    {
        DI = yx_Di[1];//yichang
        m_Station_t &= ~0x40;
        *Index = 1; 
    }
    else if(m_Station_t&0x20)
    {
        DI = yx_Di[2];//zhawei
        m_Station_t &= ~0x20;
        *Index = 2; 
    }
    else if(m_Station_t&0x10)
    {
        DI = yx_Di[3];//
        m_Station_t &= ~0x10;
        *Index = 3; 
    }
    else if(m_Station_t&0x08)
    {
        DI = yx_Di[4];
        m_Station_t &= ~0x08;
        *Index = 4; 
    }
    else if(m_Station_t&0x04)
    {
        DI = yx_Di[5];
        m_Station_t &= ~0x04;
        *Index = 5; 
    }
     return DI;
}

BOOL CDLQ::ReqUrgency(void)//遥控
{//测试下，lvyi
    VYKInfo *pYKInfo;
    DWORD Di1=0;
	DWORD YK_DI;
	BYTE zhi =0 ;
	int i;


   if (SwitchToEqpFlag(EQPFLAG_YKReq))
   {	 
		ClearEqpFlag(EQPFLAG_YKReq);/*立刻清标志*/
		pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

		switch (pYKInfo->Head.byMsgID & 0x7f)
		{
			case MI_YKSELECT:  
				YkResult(TRUE);
				return(FALSE); /*没有遥控预置*/
			case MI_YKOPRATE:  
				 if (pYKInfo->Info.wID == 1)
				 {
				    m_byReqDataType = REQ_YC;
				    Clear_All_Data();
				 }
				 else
				 {
					 if ((pYKInfo->Info.byValue & 0x3) == 1)//close
					 {
						  YK_DI = YK_H_DI;	 //合
					 }
					 if ((pYKInfo->Info.byValue & 0x3) == 2)//open
					 {
						  YK_DI = YK_F_DI;	//分
					 }
					 m_byReqDataType = REQ_YK;
					 SendWriteCmd(SET_CaS,YK_DI,0,0x02);
				 }
				break;
			 case MI_YKCANCEL:
				YkResult(TRUE);
				return FALSE;
		}
		return TRUE;
		
	}   

	for(i=0;i<6;i++) 
	{
	    if(m_EventTimes[i])
	    { 
	    	if(m_EventTimes[i]>10)
				m_EventTimes[i]=10;
	        m_EventTimes[i]--;
	        m_byReqDataType = REQ_YX;			
		 SetEqpFlag(EQPFLAG_YX);
	        SendReadCmd(SJ_CODE,yx_Di[i] + m_EventTimes[i],0,0);	        
	    	return TRUE;
		}
	}
    if(m_Station_t & 0xfc)
    {
        m_byReqDataType = REQ_YC;
        m_Dii =  Get_DI_yx(&zhi);
        Di1 = Clear_di[zhi];
        SendReadCmd(SJ_CODE,Di1,0,0);
        return TRUE;
    }else
    {    	
		ClearEqpFlag(EQPFLAG_YX);
        m_byNextReq = REQ_YC;
        m_Station_t = 0;
    }
	

	return FALSE;
}

BOOL CDLQ::ReqCyc(void)  
{
    DWORD di;
	if(m_pBaseCfg->wCmdControl & BROADCASTTIME)
	{
    	if (CheckClearTaskFlag(TASKFLAG_CLOCK)||g_SetTimeFlag)
    	{
            SendSetTime();
			g_SetTimeFlag = FALSE;
            return TRUE;
    	}
	}
 
    if (g_switchparaget.readflag == 0xAA)
    {
        di = dz_di1[g_switchparaget.switchparaoffset];
        m_oldeqpno = m_wEqpNo;
        SwitchToEqpNo(g_switchparaget.switchno);		
        Deal_DD(di);
        g_switchparaget.readflag = 0xBB;
        return TRUE;    
     }
     else if (g_switchparaget.readflag == 0xBB)
     {
         g_switchparaget.readflag = 0x01; 
         SwitchToEqpNo(m_oldeqpno);
     }
	 return FALSE;
}

BOOL CDLQ::ReqNormal(void)
{ 
    
	DWORD di; 
	if (m_byNextReq == REQ_YC)
	{
		SetEqpFlag(EQPFLAG_YC);
        
		di  = yc_Di[m_index];
		SendReadCmd(SJ_CODE,di,0,0);

		m_index++;
		if (m_index >= YCDI_NUM)
		{
			m_index = 0;
			ClearEqpFlag(EQPFLAG_YC);
			ClearEqpFlag(EQPFLAG_YX);
            ClearEqpFlag(EQPFLAG_DD);
			m_byNextReq = REQ_YC;
		}
		return TRUE;
	} 
	return FALSE;
}

#endif


