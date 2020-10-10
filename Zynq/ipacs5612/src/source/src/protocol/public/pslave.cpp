/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/
#include "syscfg.h"
	
#ifdef INCLUDE_PSLAVE
	
#include "os.h"
#include "pslave.h"

static VUFlagInfo UFlagIndex[]=
      {
      {DSOEUFLAG,        TASKFLAG_DSOEUFLAG,      EQPFLAG_DSOEUFLAG,     TASKFLAG_SENDDSOE,      EQPFLAG_SENDDSOE},
      {DCOSUFLAG,        TASKFLAG_DCOSUFLAG,      EQPFLAG_DCOSUFLAG,	 TASKFLAG_SENDDCOS,      EQPFLAG_SENDDCOS},
      {SSOEUFLAG,        TASKFLAG_SSOEUFLAG,      EQPFLAG_SSOEUFLAG,	 TASKFLAG_SENDSSOE,      EQPFLAG_SENDSSOE},
      {SCOSUFLAG,        TASKFLAG_SCOSUFLAG,      EQPFLAG_SCOSUFLAG,	 TASKFLAG_SENDSCOS,      EQPFLAG_SENDSCOS},
      {FAINFOUFLAG,      TASKFLAG_FAINFOUFLAG,    0,                     TASKFLAG_SENDFAINFO,    0},
	  {ACTEVENTUFLAG,    TASKFLAG_ACTEVENTUFLAG,  0, 				     TASKFLAG_SENDACTEVENT,  0},
	  {DOEVENTUFLAG,     TASKFLAG_DOEVENTUFLAG,   0, 				     TASKFLAG_SENDDOEVENT,   0},
	  {WARNEVENTUFLAG,   TASKFLAG_WARNEVENTUFLAG, 0, 				     TASKFLAG_SENDWARNEVENT, 0},
      {0,0,0,0,0},
      };  


/***************************************************************
	Function:CPSecond
		规约基类构造函数
	参数：无
	返回：无
***************************************************************/
CPSecondary::CPSecondary():CProtocol()
{
	m_byProtocolType=SECONDARY;	

	memset(m_UFlagInfo, 0, sizeof(m_UFlagInfo));
	
	m_UFlagInfo[0].dwFlag=SCOSUFLAG;
	m_UFlagInfo[1].dwFlag=DCOSUFLAG;
	m_UFlagInfo[2].dwFlag=FAINFOUFLAG;
    m_UFlagInfo[3].dwFlag=SSOEUFLAG;
    m_UFlagInfo[4].dwFlag=DSOEUFLAG;
    m_UFlagInfo[5].dwFlag=ACTEVENTUFLAG;
}

BOOL CPSecondary::Init(WORD wTaskID, WORD wMinEqpNum,void *pCfg,WORD wCfgLen)
{
    int i;
	VUFlagInfo *pFlagInfo;
	
	//ReIndex UFlagInfo
	for (i=0; i<sizeof(m_UFlagInfo)/sizeof(m_UFlagInfo[0]); i++)
	{
	  for (pFlagInfo=UFlagIndex; pFlagInfo->dwFlag!=0; pFlagInfo++)
		if (pFlagInfo->dwFlag==m_UFlagInfo[i].dwFlag) break;
	
	  m_UFlagInfo[i].dwEqpFlag=pFlagInfo->dwEqpFlag;
	  m_UFlagInfo[i].dwTaskFlag=pFlagInfo->dwTaskFlag;
	  m_UFlagInfo[i].dwSendEqpFlag=pFlagInfo->dwSendEqpFlag;
	  m_UFlagInfo[i].dwSendTaskFlag=pFlagInfo->dwSendTaskFlag;
	}	

    if (CProtocol::Init(wTaskID,wMinEqpNum,pCfg,wCfgLen)==FALSE)  return(FALSE);    

	SetDefFlag();
	
	return(TRUE);
}


/***************************************************************
	Function:InitEqpInfo
		初始化装置信息
	参数：无
	
	返回：TRUE 成功，FALSE 失败 
***************************************************************/
BOOL CPSecondary::InitEqpInfo(WORD wMinEqpNum)
{
	WORD wEqpID;
	VTVEqpInfo EqpInfo;
	VEqpGroupInfo *pEqpGroupInfo = NULL;
	int EqpNo;
	
	memset(&m_EqpGroupExtInfo,0,sizeof(VSecEqpExtInfo));	

	if (wMinEqpNum==0)
	{
	    m_wEqpNum=0;
        return(TRUE);
	}

	m_wEqpNum = GetTaskEqpNum(m_wTaskID);

	if (m_wEqpNum<wMinEqpNum)		
	{
		myprintf(m_wTaskID, LOG_ATTR_WARN, "Eqpnum < MinEqpum, protcol init error!");
		return FALSE;
	}

	if (m_wEqpNum==0)  return(TRUE);
	
	if (m_wEqpNum > 1)
	{
		myprintf(m_wTaskID, LOG_ATTR_INFO, "Secondary Protcol Init EqpInfo Error,m_wEqpNum > 1.");
		return FALSE;	
	}

	GetTaskEqpID(m_wTaskID, &m_wEqpGroupID);
	if (ReadEqpInfo(m_wTaskID, m_wEqpGroupID, &EqpInfo, TRUE) == ERROR)
	{
		myprintf(m_wTaskID, LOG_ATTR_INFO, "Secondary Protcol Init EqpInfo Error,Read EqpGroup Info Error.");
		return FALSE;
	}
	if (EqpInfo.wType != GROUPEQP)
	{
		myprintf(m_wTaskID, LOG_ATTR_INFO, "Secondary Protcol Init EqpInfo Error,it is not EqpGroup.");
		return FALSE;	
	}
			
	pEqpGroupInfo = (VEqpGroupInfo *)&EqpInfo;
	m_EqpGroupInfo.wType = pEqpGroupInfo->wType;
	m_EqpGroupInfo.wSourceAddr = pEqpGroupInfo->wSourceAddr;
	m_EqpGroupInfo.wDesAddr = pEqpGroupInfo->wDesAddr;
	m_EqpGroupInfo.dwFlag = pEqpGroupInfo->dwFlag;
	m_EqpGroupInfo.wEqpNum = pEqpGroupInfo->wEqpNum;
	m_EqpGroupInfo.pEqpRunInfo = pEqpGroupInfo->pEqpRunInfo;
	m_EqpGroupInfo.pwEqpID = pEqpGroupInfo->pwEqpID;
	
	m_wEqpNum = m_EqpGroupInfo.wEqpNum; 					
	m_pEqpInfo = (VPtEqpInfo*)calloc(m_wEqpNum, sizeof(VPtEqpInfo));
	m_pEqpExtInfo= (VSecEqpExtInfo*)calloc(m_wEqpNum, sizeof(VSecEqpExtInfo));

	if (!m_pEqpInfo||!m_pEqpExtInfo)
		return FALSE;
	
	for (EqpNo=0; EqpNo<m_wEqpNum; EqpNo++)
	   memset(m_pEqpExtInfo+EqpNo,0,sizeof(VSecEqpExtInfo));	

	if ((m_wEqpNum==0)||(!m_pEqpInfo))
	{
		myprintf(m_wTaskID, LOG_ATTR_INFO, "Secondary Protcol Init EqpInfo Error,m_pEqpInfo = NULL.");
		return FALSE;
	}

	m_pwEqpID = m_EqpGroupInfo.pwEqpID;
	m_wEqpID = m_pwEqpID[0];
	
	for (EqpNo = 0; EqpNo < m_wEqpNum; EqpNo++)
	{
		wEqpID = m_pwEqpID[EqpNo];
		if (ReadEqpInfo(m_wTaskID, wEqpID, &EqpInfo, TRUE) == ERROR)
		{
			myprintf(m_wTaskID, LOG_ATTR_INFO, "Secondary Protcol Init EqpInfo Error,ReadEqpInfo Error.");
			return FALSE;
		}
		m_pEqpInfo[EqpNo].wEqpID = wEqpID;						 
		m_pEqpInfo[EqpNo].dwFlag= EqpInfo.dwFlag;
		m_pEqpInfo[EqpNo].wAddress= EqpInfo.wSourceAddr;		 
		m_pEqpInfo[EqpNo].wDAddress = EqpInfo.wDesAddr; 		 
		m_pEqpInfo[EqpNo].wYCNum = EqpInfo.wYCNum;				 
		m_pEqpInfo[EqpNo].wVYCNum = EqpInfo.wVYCNum;				 
		m_pEqpInfo[EqpNo].wDYXNum = EqpInfo.wDYXNum;			 
		m_pEqpInfo[EqpNo].wSYXNum = EqpInfo.wSYXNum;			 
		m_pEqpInfo[EqpNo].wVYXNum = EqpInfo.wVYXNum;			 
		m_pEqpInfo[EqpNo].wDDNum = EqpInfo.wDDNum;				 
		m_pEqpInfo[EqpNo].wYKNum = EqpInfo.wYKNum;				 
		m_pEqpInfo[EqpNo].wTSDataNum = EqpInfo.wTSDataNum;		 
		m_pEqpInfo[EqpNo].wYTNum = EqpInfo.wYTNum;				 
		m_pEqpInfo[EqpNo].wTQNum = EqpInfo.wTQNum;				 
		m_pEqpInfo[EqpNo].pEqpRunInfo = EqpInfo.pEqpRunInfo;  
		memset(&m_pEqpInfo[EqpNo].Flags,0,sizeof(VFLAGS));

		if (EqpInfo.wYCNum==0)
		{
		  m_pEqpExtInfo[EqpNo].OldYC=NULL;
		  m_pEqpExtInfo[EqpNo].CurYC=NULL;
		  m_pEqpExtInfo[EqpNo].ChangeYCSend=NULL;
		   m_pEqpExtInfo[EqpNo].LastYC = NULL;
		}  
		else
		{
		  m_pEqpExtInfo[EqpNo].OldYC= (VYCF_L*)calloc(EqpInfo.wYCNum, sizeof(VYCF_L));
		  m_pEqpExtInfo[EqpNo].CurYC= (VYCF_L*)calloc(EqpInfo.wYCNum, sizeof(VYCF_L));
		   m_pEqpExtInfo[EqpNo].LastYC = (VYCF_L*)calloc(EqpInfo.wYCNum, sizeof(VYCF_L));
		  m_pEqpExtInfo[EqpNo].YCCFG = (VYCF_L_Cfg*)calloc(EqpInfo.wYCNum,sizeof(VYCF_L_Cfg));
		   m_pEqpExtInfo[EqpNo].LastYCNum = (BYTE*)calloc(EqpInfo.wYCNum, sizeof(BYTE));
		  	
		  m_pEqpExtInfo[EqpNo].ChangeYCSend= (BOOL*)calloc(EqpInfo.wYCNum, sizeof(BOOL));
		  if (!m_pEqpExtInfo[EqpNo].OldYC||!m_pEqpExtInfo[EqpNo].CurYC||!m_pEqpExtInfo[EqpNo].ChangeYCSend)
				return FALSE;
		  if (!m_pEqpExtInfo[EqpNo].LastYC|| !m_pEqpExtInfo[EqpNo].LastYCNum)
            		    return FALSE;
		  memset(m_pEqpExtInfo[EqpNo].OldYC, 0, EqpInfo.wYCNum*sizeof(VYCF_L));
		  memset(m_pEqpExtInfo[EqpNo].LastYC, 0, EqpInfo.wYCNum*sizeof(VYCF_L));
		  memset(m_pEqpExtInfo[EqpNo].LastYCNum, 0, EqpInfo.wYCNum * sizeof(BYTE));
		  memset(m_pEqpExtInfo[EqpNo].ChangeYCSend, 0, EqpInfo.wYCNum * sizeof(BOOL));
		}	 

		m_pEqpExtInfo[EqpNo].wChangYCNo=0; 			
	}//end of for
	
	return TRUE;
}


/***************************************************************
	Function:GetEqpAddr
		获取当前装置地址，暂空
	参数：无
	
	返回：当前装置地址
***************************************************************/
WORD CPSecondary::GetEqpAddr(void)   
{
	return m_EqpGroupInfo.wDesAddr;
}

/***************************************************************
	Function:GetOwnAddr
		获取自身地址
	参数：无
	
	返回：自身地址
***************************************************************/
WORD CPSecondary::GetOwnAddr(void)		  
{
	return m_EqpGroupInfo.wSourceAddr;
}

WORD CPSecondary::GetEqpOwnAddr(void)
{
	return 	m_pEqpInfo[m_wEqpNo].wDAddress;
}

/***************************************************************
	Function:SwitchToAddress
		根据地址切换到相应模块
	参数：wAddress
		  wAddress 要切换到的地址
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CPSecondary::SwitchToAddress(WORD wAddress)
{
    if ((wAddress == GetBroadcastAddr())||(wAddress==GetOwnAddr()))
		return TRUE;
	return FALSE;
}

/***************************************************************
	Function:GotoEqpbyAddress
		根据地址切换到装置组内相应的装置
		参数：wAddress
		  wAddress 要切换到的地址
	返回：TRUE 成功，FALSE 失败
***************************************************************/
BOOL CPSecondary::GotoEqpbyAddress(WORD wAddress)
{
	WORD wEqpNo;

	for (wEqpNo = 0; wEqpNo < m_wEqpNum; wEqpNo++)
	{
	  if(pGetEqpInfo(wEqpNo)->wDAddress == wAddress)
	  {
		 SwitchToEqpNo(wEqpNo);
		 return TRUE;
	  }
	}
	return FALSE;
}

/***************************************************************
	Function:DoYKRet
		遥控返校处理（从站类规约用）
	参数：pMsg
	
	返回：TRUE 成功 FALSE 失败
***************************************************************/
BOOL CPSecondary::DoYKRet(void)
{
	VYKInfo *pYKInfo;

	if (SwitchToEqpID(m_pMsg->Head.wEqpID) != TRUE)
	{
		return FALSE;
	}
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);

	memcpy(pYKInfo,m_pMsg,sizeof(VYKInfo));
	
	SetEqpFlag(EQPFLAG_YKRet);
	
	return TRUE;
}


/***************************************************************
	Function:DoYKReq
		设置遥控命令（从站类规约用）
	参数：无
	
	返回：TRUE 成功 FALES 失败
***************************************************************/
BOOL CPSecondary::DoYKReq(void)
{
	VYKInfo *pYKInfo;

    if (m_wEqpNum==0)  return FALSE;
	
	pYKInfo = &(pGetEqpInfo(m_wEqpNo)->YKInfo);
	
	memcpy(m_pMsg,pYKInfo,sizeof(VYKInfo));

	TaskSendMsg(DB_ID, m_wTaskID, m_wEqpID, pYKInfo->Head.byMsgID,MA_REQ, sizeof(VYKInfo)-sizeof(VMsgHead), m_pMsg);

	return TRUE;
}


BOOL CPSecondary::DoYTRet(void)
{
	VYTInfo *pYTInfo;

	if (SwitchToEqpID(m_pMsg->Head.wEqpID) != TRUE)
	{
		return FALSE;
	}
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);

	memcpy(pYTInfo,m_pMsg,sizeof(VYTInfo));
	
	SetEqpFlag(EQPFLAG_YTRet);
	
	return TRUE;
}


BOOL CPSecondary::DoYTReq(void)
{
	VYTInfo *pYTInfo;

    if (m_wEqpNum==0)  return FALSE;
	
	pYTInfo = &(pGetEqpInfo(m_wEqpNo)->YTInfo);
	
	memcpy(m_pMsg,pYTInfo,sizeof(VYTInfo));

	TaskSendMsg(DB_ID, m_wTaskID, m_wEqpID, pYTInfo->Head.byMsgID,MA_REQ, sizeof(VYTInfo)-sizeof(VMsgHead), m_pMsg);
	
	return TRUE;
}

WORD CPSecondary::SearchChangeYC(WORD wNum, WORD wBufLen, VDBYCF_L *pBuf, BOOL bActive)
{
	float wAbsVal,nChangeVal;	
	long maxval,A;
	int i,j,k,m;
    VYCF_L *pCurYC, *pOldYC,*pLastYC;
	VDBYCF_L *pSendYC;
	struct VYCF_L_Cfg *pYcCfg;
	VRunParaCfg *prunparacfg;
	float fval1,fval2,fval3;
	WORD deadvalue;
	BYTE *pycnum;

	prunparacfg = &g_Sys.MyCfg.RunParaCfg;
	if (m_pEqpInfo[m_wEqpNo].wYCNum == 0)  return(0);
	if (wBufLen < sizeof(VDBYCF_L))  return(0);
	
	for (i=0; i<m_pEqpInfo[m_wEqpNo].wYCNum; i++)
	{
        m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=FALSE;
	}    
	
	::ReadRangeYCF_L(m_wEqpID, m_pEqpExtInfo[m_wEqpNo].wChangYCNo, m_pEqpInfo[m_wEqpNo].wYCNum-m_pEqpExtInfo[m_wEqpNo].wChangYCNo, m_pEqpInfo[m_wEqpNo].wYCNum*sizeof(VYCF_L) , m_pEqpExtInfo[m_wEqpNo].CurYC+m_pEqpExtInfo[m_wEqpNo].wChangYCNo);

	j = 0;
	pycnum = m_pEqpExtInfo[m_wEqpNo].LastYCNum;
    for (i=m_pEqpExtInfo[m_wEqpNo].wChangYCNo; i<m_pEqpInfo[m_wEqpNo].wYCNum; i++)
    {         
		if (j >=  wNum)
			break;

		pCurYC = m_pEqpExtInfo[m_wEqpNo].CurYC + i;
		pOldYC = m_pEqpExtInfo[m_wEqpNo].OldYC + i;
		pLastYC = m_pEqpExtInfo[m_wEqpNo].LastYC + i;
		
		pYcCfg = m_pEqpExtInfo[m_wEqpNo].YCCFG + i;
			
		if ((bActive == 0) && (pCurYC->byFlag & 0x80)) continue;    //active send disable

		if (pCurYC->byFlag != pOldYC->byFlag)
		{
			m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i] = TRUE;
			j++;
		}
		else
		{
			::ReadYCCfg(m_wEqpID, i,pYcCfg,1);
			if(((pYcCfg->tyctype >= YC_TYPE_Ua) && (pYcCfg->tyctype <= YC_TYPE_SU0))
			||((pYcCfg->tyctype >= YC_TYPE_Uab) && (pYcCfg->tyctype <= YC_TYPE_Uca))
			||((pYcCfg->tyctype >= YC_TYPE_SUa) && (pYcCfg->tyctype <= YC_TYPE_SUc)))//交流电压
			{
				if (pOldYC->byFlag & 0x40)
					memcpy(&fval1,(void*)&pOldYC->lValue,4);
				else
					fval1 = pOldYC->lValue;	
				if (pCurYC->byFlag & 0x40)
					memcpy(&fval2,(void*)&pCurYC->lValue,4);
				else
					fval2=pCurYC->lValue;

				if (pOldYC->byFlag & 0x40)
					nChangeVal = (float)(prunparacfg->wPt2 * m_pBaseCfg->wACDeadValue)/1000; //比例系数暂未考虑
				else
					nChangeVal = (float)(prunparacfg->wPt2 * 100 * m_pBaseCfg->wACDeadValue)/1000; //比例系数暂未考虑
				if(GetYC_ABC(m_wEqpID,i,&A,&maxval,NULL) == OK)
				{
					if(m_pEqpInfo[m_wEqpNo].dwFlag & YCQUOTIETYENABLED)
						nChangeVal = nChangeVal*A/maxval;
				}
			}
			else if(((pYcCfg->tyctype >= YC_TYPE_Ia) && (pYcCfg->tyctype <= YC_TYPE_SI0)) 
       			   || ((pYcCfg->tyctype >= YC_TYPE_SIa) && (pYcCfg->tyctype <= YC_TYPE_SIc)))//电流死区
			{
				if (pOldYC->byFlag & 0x40)
					memcpy(&fval1,(void*)&pOldYC->lValue,4);
				else
					fval1 = pOldYC->lValue;	
				if (pCurYC->byFlag & 0x40)
					memcpy(&fval2,(void*)&pCurYC->lValue,4);
				else
					fval2=pCurYC->lValue;
				
				if((pYcCfg->tyctype == YC_TYPE_I0) || (pYcCfg->tyctype == YC_TYPE_SI0))
					nChangeVal = prunparacfg->LineCfg[pYcCfg->tycfdnum].wCt02 *1000 * m_pBaseCfg->wIDeadValue/1000;//比例系数暂未考虑
				else
					nChangeVal = prunparacfg->LineCfg[pYcCfg->tycfdnum].wCt2 * 1000 * m_pBaseCfg->wIDeadValue/1000;//比例系数暂未考虑

				if (pOldYC->byFlag & 0x40)
					nChangeVal = nChangeVal/1000;
				
				if(GetYC_ABC(m_wEqpID,i,&A,&maxval,NULL) == OK)
				{
					if(m_pEqpInfo[m_wEqpNo].dwFlag & YCQUOTIETYENABLED)
						nChangeVal = nChangeVal*A/maxval;
				}
			}
			else
			{
				if(pYcCfg->tyctype == YC_TYPE_FDc) //直流电压死区
				{
					deadvalue = m_pBaseCfg->wDCDeadValue;
				}
				else if(((pYcCfg->tyctype >= YC_TYPE_Pa) && (pYcCfg->tyctype <= YC_TYPE_Qc)) 
					  || (pYcCfg->tyctype == YC_TYPE_P) || (pYcCfg->tyctype == YC_TYPE_Q) ||(pYcCfg->tyctype == YC_TYPE_Q)) //功率死区
				{
					deadvalue = m_pBaseCfg->wPQDeadValue;
				}
				else if(pYcCfg->tyctype == YC_TYPE_SFreq) //频率死区
				{
					deadvalue = m_pBaseCfg->wFDeadValue;
				}
				else if(pYcCfg->tyctype == YC_TYPE_Cos) //功率因数死区
				{
					deadvalue = m_pBaseCfg->wPWFDeadValue;
				}
				else
				{
					deadvalue = m_pBaseCfg->wYCDeadValue;
				}
				
				if (pOldYC->byFlag & 0x40)
					memcpy(&fval1,(void*)&pOldYC->lValue,4);
				else
					fval1 = pOldYC->lValue;

				
				if(pYcCfg->tyctype == YC_TYPE_SFreq)
				{
					if(pOldYC->byFlag & 0x40)
						nChangeVal = 50.0*deadvalue/1000;
					else
						nChangeVal = 5000.0*deadvalue/1000;
				}
				else if(((pYcCfg->tyctype >= YC_TYPE_Pa) && (pYcCfg->tyctype <= YC_TYPE_Qc)) 
					  || (pYcCfg->tyctype == YC_TYPE_P) || (pYcCfg->tyctype == YC_TYPE_Q) ||(pYcCfg->tyctype == YC_TYPE_Q))
				{
					if(pOldYC->byFlag & 0x40)
						nChangeVal = (float)prunparacfg->wPt2*prunparacfg->LineCfg[pYcCfg->tycfdnum].wCt2*1.732*deadvalue/1000;
					else
						nChangeVal = (float)prunparacfg->wPt2*prunparacfg->LineCfg[pYcCfg->tycfdnum].wCt2*1.732*10*deadvalue/1000;
				}
				else if(pYcCfg->tyctype == YC_TYPE_Cos) //功率因数死区
				{
					if(pOldYC->byFlag & 0x40)
						nChangeVal = 1.0*deadvalue/1000;
					else
						nChangeVal = 1000*deadvalue/1000;
				}
				else
					nChangeVal=fval1*deadvalue/1000;

				
				if (nChangeVal<0) nChangeVal=0-nChangeVal;

				if (pCurYC->byFlag & 0x40)
					memcpy(&fval2,(void*)&pCurYC->lValue,4);
				else
					fval2=pCurYC->lValue;
				if (pLastYC->byFlag & 0x40)
					memcpy(&fval3, (void*)&pLastYC->lValue, 4);
				else
					fval3 = pLastYC->lValue;
				
				if(GetYC_ABC(m_wEqpID,i,NULL,&maxval,NULL) == OK)
				{
					if(maxval>50)
					{
						nChangeVal = maxval;
							nChangeVal = nChangeVal*deadvalue/1000;
					}
				}

				/*if(fval3 > fval2)
					wAbsVal = fval3 - fval2;
				else
					wAbsVal = fval2 - fval3;
				if(wAbsVal > maxval / 400)
				{
					pycnum[i] = 0;
					continue;
				}*/
			
			}
			
			if(fval1> fval2)
			    wAbsVal = fval1 - fval2;
			else 
				wAbsVal = fval2 - fval1;

			if ((wAbsVal != 0) && (wAbsVal >=nChangeVal))  
			{
				if(++pycnum[i] > 2)
				{
					m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]=TRUE;
					j++;
				}
			}
			else
				pycnum[i] = 0;
		}			
    }	
	
	pSendYC=(VDBYCF_L*)pBuf; 
	j = k = 0;
	m = m_pEqpExtInfo[m_wEqpNo].wChangYCNo;
    for (i=m; i<m_pEqpInfo[m_wEqpNo].wYCNum; i++)
	{
	    if (m_pEqpExtInfo[m_wEqpNo].ChangeYCSend[i]==TRUE)
	    {
           pSendYC->wNo=i;
		   pSendYC->lValue=m_pEqpExtInfo[m_wEqpNo].CurYC[i].lValue;
		   pSendYC->byFlag=m_pEqpExtInfo[m_wEqpNo].CurYC[i].byFlag;
		   m_pEqpExtInfo[m_wEqpNo].OldYC[i].lValue=pSendYC->lValue;
		   m_pEqpExtInfo[m_wEqpNo].OldYC[i].byFlag=pSendYC->byFlag;
		   pSendYC++;
		   j++;
			 m_pEqpExtInfo[m_wEqpNo].wChangYCNo = i+1;//yc 变化号
		   k+=sizeof(VDBYCF_L);
    	   if (k>wBufLen-sizeof(VDBYCF_L)) break;
	    }
	}  

	if (j == 0)  m_pEqpExtInfo[m_wEqpNo].wChangYCNo = 0; 

	for (i = m_pEqpExtInfo[m_wEqpNo].wChangYCNo; i < m_pEqpInfo[m_wEqpNo].wYCNum; i++)
    {
        m_pEqpExtInfo[m_wEqpNo].LastYC[i].lValue = m_pEqpExtInfo[m_wEqpNo].CurYC[i].lValue;
        m_pEqpExtInfo[m_wEqpNo].LastYC[i].byFlag = m_pEqpExtInfo[m_wEqpNo].CurYC[i].byFlag;	
    }
		
	return j;
}

void CPSecondary::FillOldData(void)
{
    int i;
    if (m_pEqpInfo[m_wEqpNo].wYCNum == 0)  return;


    ::ReadRangeYCF_L(m_wEqpID, 0, m_pEqpInfo[m_wEqpNo].wYCNum, m_pEqpInfo[m_wEqpNo].wYCNum * sizeof(VYCF_L) , m_pEqpExtInfo[m_wEqpNo].CurYC);
    for (i = 0; i < m_pEqpInfo[m_wEqpNo].wYCNum; i++)
    {
        m_pEqpExtInfo[m_wEqpNo].OldYC[i].lValue = m_pEqpExtInfo[m_wEqpNo].CurYC[i].lValue;
        m_pEqpExtInfo[m_wEqpNo].OldYC[i].byFlag = m_pEqpExtInfo[m_wEqpNo].CurYC[i].byFlag;

    }

}

void CPSecondary::DoUrgency(void)
{
    int wEqpNo,i;
	BOOL bProcExit;

    for (i=0; i<sizeof(m_UFlagInfo)/sizeof(m_UFlagInfo[0]); i++)
    {

	   if (TestFlag(m_wTaskID, m_wEqpGroupID, m_UFlagInfo[i].dwFlag))
       {           
	       SetTaskFlag(m_UFlagInfo[i].dwTaskFlag);
         
		   if ((m_UFlagInfo[i].dwFlag==FAINFOUFLAG) || (m_UFlagInfo[i].dwFlag==ACTEVENTUFLAG) || (m_UFlagInfo[i].dwFlag==DOEVENTUFLAG) || (m_UFlagInfo[i].dwFlag==WARNEVENTUFLAG))
           {
			   if (atOnceProcUrgency(0,m_UFlagInfo[i].dwFlag)==2)  
			      return;
		       continue;
		   }   
		   /*else  
		   	   ClearFlag(m_wTaskID, m_wEqpGroupID, m_UFlagInfo[i].dwFlag);*/

		   bProcExit=FALSE;
           for (wEqpNo=0; wEqpNo<m_wEqpNum; wEqpNo++)
           {
              if (TestFlag(m_wTaskID, pGetEqpInfo(wEqpNo)->wEqpID, m_UFlagInfo[i].dwFlag))
              {
             	 SetEqpFlag(wEqpNo,m_UFlagInfo[i].dwEqpFlag);  
			     if (atOnceProcUrgency(wEqpNo,m_UFlagInfo[i].dwFlag)==2)
				 {
                    bProcExit=TRUE;
    				break;
			     }	 
    		  }			  
	       }  	

		   for (wEqpNo=0; wEqpNo<m_wEqpNum; wEqpNo++)
		   {
			  if (TestFlag(m_wTaskID, pGetEqpInfo(wEqpNo)->wEqpID, m_UFlagInfo[i].dwFlag))
			  {
                 break;
			  }
		   } 

		   if (wEqpNo==m_wEqpNum)  ClearFlag(m_wTaskID, m_wEqpGroupID, m_UFlagInfo[i].dwFlag);
		   	
		   if (bProcExit==TRUE)  return;
	    }
    }
}

/***************************************************************
	Function:atOnceProcUrgency
		立即出来紧急数据
	参数：
	
	返回：=0  无此类紧急数据
		  =1  处理完毕此类紧急数据，接着处理其他紧急数据
		  =2  处理完毕此类紧急数据，等待下一帧报文再次调用后再处理其他紧急数据
	
***************************************************************/
int CPSecondary::atOnceProcUrgency(WORD wEqpNo,DWORD dwUFlag)
{

	switch (dwUFlag)
	{
	   case  DSOEUFLAG:
		   return(atOnceProcDSOE(wEqpNo));
	   case  DCOSUFLAG:  
		   return(atOnceProcDCOS(wEqpNo));
	   case  SSOEUFLAG:	
		   return(atOnceProcSSOE(wEqpNo));
	   case  SCOSUFLAG:	
		   return(atOnceProcSCOS(wEqpNo));
	   case  FAINFOUFLAG:
		   return(atOnceProcFAInfo(wEqpNo));
	   case  ACTEVENTUFLAG:
		   return(atOnceProcActEvent(wEqpNo));
	   case  DOEVENTUFLAG:
		   return(atOnceProcDoEvent(wEqpNo));
	   case  WARNEVENTUFLAG:
		   return(atOnceProcWarnEvent(wEqpNo));
	}	   

	return 0;
}


WORD CPSecondary::GetSendNum(WORD wEqpNo,DWORD dwUFlag)
{
	switch (dwUFlag)
	{
	   case  DSOEUFLAG:
	   	   if (m_wEqpNum==0)  return(0);
		   return(m_pEqpExtInfo[wEqpNo].wSendDSOENum);
	   case  DCOSUFLAG:  
		   if (m_wEqpNum==0)  return(0);
		   return(m_pEqpExtInfo[wEqpNo].wSendDCOSNum);
	   case  SSOEUFLAG:	
		   if (m_wEqpNum==0)  return(0);
		   return(m_pEqpExtInfo[wEqpNo].wSendSSOENum);
	   case  SCOSUFLAG:	
		   if (m_wEqpNum==0)  return(0);
		   return(m_pEqpExtInfo[wEqpNo].wSendSCOSNum);
	   case  FAINFOUFLAG:
		   return(m_EqpGroupExtInfo.wSendFAInfoNum);
	   case  ACTEVENTUFLAG:
		   return(m_EqpGroupExtInfo.wSendActEventNum);
	   case  DOEVENTUFLAG:
		   return(m_EqpGroupExtInfo.wSendDoEventNum);
	   case  WARNEVENTUFLAG:
		   return(m_EqpGroupExtInfo.wSendWarnEventNum);
	   default:
	   	   return(0);
	}	       
}


/***************************************************************
	Function:ClearUFlag
		清除紧急数据标志
	参数：
	
	返回：=0  无新的紧急数据
		  =1  处理了新的紧急数据并发送多帧
		  =2  处理新的紧急数据并发送一帧，等待再次调用后再处理下一紧急数据	
***************************************************************/
int CPSecondary::ClearUFlag(WORD wEqpNo) 
{
    int i;
	WORD wSendNum,wProcResult=0;

	for (i=0; i<sizeof(m_UFlagInfo)/sizeof(m_UFlagInfo[0]); i++)
	{
		wSendNum=GetSendNum(wEqpNo,m_UFlagInfo[i].dwFlag);
        switch (m_UFlagInfo[i].dwFlag)
        {
		    case FAINFOUFLAG:
			case ACTEVENTUFLAG:
			case DOEVENTUFLAG:
			case WARNEVENTUFLAG:				
				if (CheckClearTaskFlag(m_UFlagInfo[i].dwSendTaskFlag))
				{
					ReadPtrIncrease(m_wTaskID,m_wEqpGroupID,wSendNum,m_UFlagInfo[i].dwFlag);
					if (TestFlag(m_wTaskID, m_wEqpGroupID, m_UFlagInfo[i].dwFlag))
					{
						SetTaskFlag(m_UFlagInfo[i].dwTaskFlag);	
						wProcResult=1;
						if (atOnceProcUrgency(0,m_UFlagInfo[i].dwFlag)==2) return(2);
					}			
				}
				break;
			default:
				if (CheckClearTaskFlag(m_UFlagInfo[i].dwSendTaskFlag))
				{
                    if (CheckClearEqpFlag(m_UFlagInfo[i].dwSendEqpFlag))
                    {
						ReadPtrIncrease(m_wTaskID,pGetEqpInfo(wEqpNo)->wEqpID,wSendNum,m_UFlagInfo[i].dwFlag);
						if (TestFlag(m_wTaskID, pGetEqpInfo(wEqpNo)->wEqpID, m_UFlagInfo[i].dwFlag))
				        {
                           SetTaskFlag(m_UFlagInfo[i].dwTaskFlag);
         				   SetEqpFlag(wEqpNo,m_UFlagInfo[i].dwEqpFlag);  
    					   wProcResult=1;						   
		        		   if (atOnceProcUrgency(wEqpNo,m_UFlagInfo[i].dwFlag)==2) return(2);
						}  
                    }	
				}
			    break;
        }				    	
	}

	return(wProcResult);
}


void CPSecondary::DoTimeOut(void)
{
    int i;

	thClearDog(m_wTaskID, COMMONDOGTM);
	m_dwTimerCount++; 
    for (i=0; i<7; i++)
    {
	   if (TestFlag(m_wTaskID, m_wEqpGroupID, m_UFlagInfo[i].dwFlag))
       {           
           evSend(m_wTaskID, EV_UFLAG);
		   break;
	   }
    }	
	
	if (m_pBaseCfg->wCmdControl&ALLDATAPROC)
	{
	  if (TimerOn(m_pBaseCfg->Timer.wAllData*60))
	  { 
		  SetAllEqpFlag(EQPFLAG_YC);
		  SetAllEqpFlag(EQPFLAG_YX); 
	  }  
	}	
	if (m_pBaseCfg->wCmdControl&DDPROC)
	{
	  if (TimerOn(m_pBaseCfg->Timer.wDD*60))
	  { 
		  SetAllEqpFlag(EQPFLAG_DD);	
	  }  
	}	
}

void CPSecondary::SetDefFlag(void)
{}

int CPSecondary::atOnceProcDSOE(WORD wEqpNo)
{
     return 0;
}

int CPSecondary::atOnceProcDCOS(WORD wEqpNo)
{
     return 0;
}

int CPSecondary::atOnceProcSSOE(WORD wEqpNo)
{
     return 0;
}

int CPSecondary::atOnceProcSCOS(WORD wEqpNo)
{
     return 0;
}

int CPSecondary::atOnceProcEVCOS(WORD wEqpNo)
{   
     return 0;
}

int CPSecondary::atOnceProcEVSOE(WORD wEqpNo)
{
     return 0;
}

int CPSecondary::atOnceProcFAInfo(WORD wEqpNo)
{
     return 0;
}

int CPSecondary::atOnceProcActEvent(WORD wEqpNo)
{
     return 0;
}

int CPSecondary::atOnceProcDoEvent(WORD wEqpNo)
{
     return 0;
}

int CPSecondary::atOnceProcWarnEvent(WORD wEqpNo)
{
     return 0;
}

int CPSecondary::atOnceProcRoute(WORD wEqpNo)
{
     return 0;
}

#endif

