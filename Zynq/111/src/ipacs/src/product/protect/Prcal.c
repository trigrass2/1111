#include "syscfg.h"

#ifdef INCLUDE_PR
#include "PrCal.h"
#include "sys.h"

static long arlFt0[MAX_AI_NUM*2];
static long arlFt10[MAX_AI_NUM*2];
#ifdef INCLUDE_HW2_PROTECT
static long arlFt0_2[MAX_AI_NUM*2];
#endif

#ifdef INCLUDE_PR_PRO
VAiTqVal *pTqVal;  //计算频率用
static VSlipFreqBuf spFreqBuf;
static VAiFreqBuf aiFreqBuf;
#endif

VAiProtCal aiProtCal[MAX_AI_NUM];
VFdProtCal *pVal;
VFdCfg *pCfg;
extern VFdCfg   *pCfgSL;
extern VPubProtCal *pValSL;

VBusProtCal BusVal;

extern VPrRunInfo *pRunInfo;
extern VPrRunInfo *prRunInfo;
extern VPrRunSet *prRunSet[2];
extern VPrSetPublic *pPublicSet;
extern VFdProtCal *fdProtVal;


#ifdef _GUANGZHOU_TEST_VER_
#define KG_CH_YB      10
#endif

#ifdef INCLUDE_FA_DIFF
VAiProtCal aiBusProtCal[3][24]; //4个点，6次谐波,3相
#endif

//#define COMPLEX_EJ120  COMPLEX(500, 866)
void createProtFdU(int fd, int yc1, int yc2, int flag)
{
    int m,t;
    for (m=yc1; m<yc2; m++)
	{
		if ((pYcCfg[m].fdNo == fd)||(flag == 1)) 
		{
			t = pYcCfg[m].type;
			//如果是物理通道对应的电压							
			if ((t == YC_TYPE_Ua) && (pFdCfg[fd].pdU[0] == NULL))
			{
				pFdCfg[fd].pdU[0] = &(aiProtCal[pYcCfg[m].arg1].dUI);
				pFdCfg[fd].pdUSlip[0] = &(aiProtCal[pYcCfg[m].arg1].dUDelta);
			
				//三表法 3U0有效 Uxx用计算值
				if (m+2 < ycNum)
				{									
					//三表法配置 && 三表法接入
					if ((pYcCfg[m+1].type == YC_TYPE_Ub) && 
						(pYcCfg[m+2].type == YC_TYPE_Uc) &&
						(pFdCfg[fd].cfg & FD_CFG_PT_58V))
					{
						//SI0为Ia下一地址值
						pFdCfg[fd].pdSU0  = pFdCfg[fd].pdU[0]+1;							
						pFdCfg[fd].pdU[3] = pFdCfg[fd].pdSU0;
						pFdCfg[fd].pdUxx[0] = pFdCfg[fd].pdU[0]+2;
					}
					//二表法配置或接入 3U0无效 Uxx为自身
					else
						pFdCfg[fd].pdUxx[0] = pFdCfg[fd].pdU[0];
				}	
				//二表法配置 3U0无效 Uxx为自身
				else
					pFdCfg[fd].pdUxx[0] = pFdCfg[fd].pdU[0];
			}
			else if ((t == YC_TYPE_Ub) && (pFdCfg[fd].pdU[1] == NULL))
			{
				pFdCfg[fd].pdU[1] = &(aiProtCal[pYcCfg[m].arg1].dUI);
				pFdCfg[fd].pdUSlip[1] = &(aiProtCal[pYcCfg[m].arg1].dUDelta);
				//三表法 Uxx用计算值
				if ((m-1 >= 0) && (m+1 < ycNum))
				{									
					//三表法配置 && 三表法接入
					if ((pYcCfg[m-1].type == YC_TYPE_Ua) && 
						(pYcCfg[m+1].type == YC_TYPE_Uc) && 
						(pFdCfg[fd].cfg & FD_CFG_PT_58V))
						pFdCfg[fd].pdUxx[1] = pFdCfg[fd].pdU[1]+2;
					//二表法配置或接入	 Uxx为自身
					else
						pFdCfg[fd].pdUxx[1] = pFdCfg[fd].pdU[1];
				}	
				//二表法配置 Uxx为自身
				else
					pFdCfg[fd].pdUxx[1] = pFdCfg[fd].pdU[1];								
			}	
			else if ((t == YC_TYPE_Uc) && (pFdCfg[fd].pdU[2] == NULL))
			{
				pFdCfg[fd].pdU[2] = &(aiProtCal[pYcCfg[m].arg1].dUI);
				pFdCfg[fd].pdUSlip[2] = &(aiProtCal[pYcCfg[m].arg1].dUDelta);
				//三表法 Uxx用计算值
				if (m-2 >= 0)
				{									
					//三表法配置 && 三表法接入
					if ((pYcCfg[m-2].type == YC_TYPE_Ua) && 
						(pYcCfg[m-1].type == YC_TYPE_Ub) && 
						(pFdCfg[fd].cfg & FD_CFG_PT_58V))
						pFdCfg[fd].pdUxx[2] = pFdCfg[fd].pdU[2]+2;
					//二表法配置或接入	Uxx为自身
					else
						pFdCfg[fd].pdUxx[2] = pFdCfg[fd].pdU[2];
				}	
				//二表法配置 Uxx为自身
				else
					pFdCfg[fd].pdUxx[2] = pFdCfg[fd].pdU[2];																
			}									
			else if ((t == YC_TYPE_U0) && (pFdCfg[fd].pdU0 == NULL))
			{
				pFdCfg[fd].pdU0 = &(aiProtCal[pYcCfg[m].arg1].dUI);
				//保护零序用测量零序
				pFdCfg[fd].pdU[3] = pFdCfg[fd].pdU0;					
			}				
		}	
	}
}

void createProtFdUxx(int fd)
{
    if (pFdCfg[fd].cfg & FD_CFG_PT_58V)
    {
        if ((pFdCfg[fd].pdU[0] == NULL)&&(pFdCfg[fd].pdU[1] != NULL))
		{
		    pFdCfg[fd].pdUxx[0]  = pFdCfg[fd].pdU[1];
			pFdCfg[fd].pdUxx[1]  = pFdCfg[fd].pdU[1]+2;
			pFdCfg[fd].gain_u[0] = pFdCfg[fd].gain_u[1];
        }
		if ((pFdCfg[fd].pdU[1] == NULL)&&(pFdCfg[fd].pdU[2] != NULL))
		{
		    pFdCfg[fd].pdUxx[1]  = pFdCfg[fd].pdU[2];
			pFdCfg[fd].pdUxx[2]  = pFdCfg[fd].pdU[2]+2;
			pFdCfg[fd].gain_u[1] = pFdCfg[fd].gain_u[2];
		}
		if ((pFdCfg[fd].pdU[2] == NULL)&&(pFdCfg[fd].pdU[0] != NULL))
		{
		    pFdCfg[fd].pdUxx[2]  = pFdCfg[fd].pdU[0];
		    pFdCfg[fd].pdUxx[0]  = pFdCfg[fd].pdU[0]+2;
			pFdCfg[fd].gain_u[2] = pFdCfg[fd].gain_u[0];
		}
    }
	else
	{
	    //两表法固定只能接Uab,Ubc或Ucb
	    if ((pFdCfg[fd].pdU[1] == NULL)&&(pFdCfg[fd].pdU[2] != NULL))
		{
		    pFdCfg[fd].pdUxx[1]  = pFdCfg[fd].pdU[2];
			pFdCfg[fd].gain_u[1] = pFdCfg[fd].gain_u[2];
	    }
		if ((pFdCfg[fd].pdU[2] == NULL)&&(pFdCfg[fd].pdU[1] != NULL))
		{
		   pFdCfg[fd].pdUxx[2]  = pFdCfg[fd].pdU[1]+2;
		   pFdCfg[fd].gain_u[2] = pFdCfg[fd].gain_u[1];
		}
	}

}

void createProtFdRef(void)
{
    int i, j, t, k,  n, e;

	#ifdef INCLUDE_PR_PRO
	pTqVal = (VAiTqVal *)malloc(sizeof(VAiTqVal));
	#endif
	
	for (i=0; i<fdNum; i++)
    {
		memset(pFdCfg[i].pdI, NULL, 4*sizeof(DWORD *));
		memset(pFdCfg[i].pdU, NULL, 4*sizeof(DWORD *));
		memset(pFdCfg[i].pdUxx, NULL, 3*sizeof(DWORD *));
		memset(pFdCfg[i].pdI_Hw2, NULL, 4*sizeof(DWORD *));
		memset(pFdCfg[i].pdUSlip, NULL, 3*sizeof(DWORD *));

		pFdCfg[i].pdI0= NULL;
		pFdCfg[i].pdSI0 = NULL;
        pFdCfg[i].pdUI0Bak = NULL;
		pFdCfg[i].pdU0= NULL;
		pFdCfg[i].pdSU0 = NULL;

		//pr disable
		if (pFdCfg[i].cfg & 0x10) continue;

        /*建立回线对AI通道的有效值地址指针,如果无该项值,则地址为缺省NULL*/

		for (j=0; j<ycNum; j++)
		{
 			if (pYcCfg[j].fdNo == i) 
 			{
                t = pYcCfg[j].type;
                //如果是物理通道对应的电流
				if ((t == YC_TYPE_Ia) && (pFdCfg[i].pdI[0] == NULL))
				{
					pFdCfg[i].pdI[0] = &(aiProtCal[pYcCfg[j].arg1].dUI);
					pFdCfg[i].pdI_Hw2[0] = &(aiProtCal[pYcCfg[j].arg1].dUI_Hw);

					//三表法 3I0有效
					if (j+2 < ycNum)
                    {
						if ((pYcCfg[j+1].type == YC_TYPE_Ib) && (pYcCfg[j+2].type == YC_TYPE_Ic))
						{
							//SI0为Ia下一地址值
							pFdCfg[i].pdSI0  = pFdCfg[i].pdI[0]+1;							
							pFdCfg[i].pdI[3] = pFdCfg[i].pdSI0;
						}
                    }	
					
                    //寻找该回线对应的电压
					k = -1;
					for (n=0; n<ycNum; n++)
					{
						if (pYcCfg[n].type == YC_TYPE_Pa)
						{
							if (pYcCfg[n].arg2 == j)
							{
								k = pYcCfg[n].arg1;
								break;
							}
							else if (pYcCfg[n].arg1 == j)
							{
								k = pYcCfg[n].arg2;
								break;
							}
						}
					}
					
					//找到该回线的电压	
					if (k >= 0)
					{
						for (e=k; e<ycNum; e++)
						{
							t = pYcCfg[e].type;

							if (!(((t>=YC_TYPE_Ua) && (t<=YC_TYPE_SU0)) || \
								 ((t>=YC_TYPE_SUa) && (t<=YC_TYPE_SUc))))
								break;
						}	
						createProtFdU(i, k, e, 1);				
					}
				}
				else if ((t == YC_TYPE_Ib) && (pFdCfg[i].pdI[1] == NULL))
				{
					pFdCfg[i].pdI[1] = &(aiProtCal[pYcCfg[j].arg1].dUI);
					pFdCfg[i].pdI_Hw2[1] = &(aiProtCal[pYcCfg[j].arg1].dUI_Hw);

				}
				else if ((t == YC_TYPE_Ic) && (pFdCfg[i].pdI[2] == NULL))
				{
					pFdCfg[i].pdI[2] = &(aiProtCal[pYcCfg[j].arg1].dUI);
					pFdCfg[i].pdI_Hw2[2] = &(aiProtCal[pYcCfg[j].arg1].dUI_Hw);

				}
                //测量零序
				else if ((t == YC_TYPE_I0) && (pFdCfg[i].pdI0 == NULL))
				{
					pFdCfg[i].pdI0 = &(aiProtCal[pYcCfg[j].arg1].dUI);
                    //保护零序用测量零序
					pFdCfg[i].pdI[3] = pFdCfg[i].pdI0;				
          
                    pFdCfg[i].pdUI0Bak = &(aiProtCal[pYcCfg[j].arg1].dUI0Bak); 					
				}
			} 
		}

        //该回线只有电压
		if (pFdCfg[i].pdI[0] == NULL)
            createProtFdU(i, 0, ycNum, 0);
		createProtFdUxx(i);		
	}

	for (k=0; k<aiNUM; k++)
	{
        i = pAiCfg[k].fdNo;
		if (i >= 0)
		{
			if (pFdCfg[i].cfg & FD_CFG_PT_58V)   
				pAiCfg[k].pt_58v = 1;
		}
	}
}

void createProtFdGain(void)
{
    int i, j, k, ai_chn;
	long gain;
	BYTE valid = 1;
	
	for (i=0; i<fdNum; i++)
    {
        valid = 1;
        for(j=0; j<4; j++)
        {
           pFdCfg[i].gain_u[j] = 65536;
           pFdCfg[i].gain_i[j] = 65536;
        }
        pFdCfg[i].gain_u0 = 65536;
        pFdCfg[i].gain_i0 = 65536;

        k = 0;
		gain = 0;
        for (j=0; j<3; j++)
        {
			ai_chn = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_UA+j];
			if (ai_chn >= 0)
			{
			    if(YC_GAIN_VALID_GET(pMeaGain[ai_chn].status) == 0)
				   valid = 0;  
				pFdCfg[i].gain_u[j] = pMeaGain[ai_chn].a;
				gain += pMeaGain[ai_chn].a;
				k++;
			}
		}
		if (k > 0) pFdCfg[i].gain_u[3] = gain/k;
		
        k = 0;
		gain = 0;
        for (j=0; j<3; j++)
        {
			ai_chn = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_IA+j];
			if (ai_chn >= 0)
			{
			    if(YC_GAIN_VALID_GET(pMeaGain[ai_chn].status) == 0)
				   valid = 0; 
				pFdCfg[i].gain_i[j]= pMeaGain[ai_chn].a;
				gain += pMeaGain[ai_chn].a;
				k++;
			}
		}
		if (k > 0) pFdCfg[i].gain_i[3] = gain/k;

		ai_chn = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_U0];
        if (ai_chn >= 0)
		{
		    pFdCfg[i].gain_u0 = pMeaGain[ai_chn].a;
		//	if(YC_GAIN_VALID_GET(pMeaGain[ai_chn].status) == 0)
		//	   valid = 0; 
        }
		else
			pFdCfg[i].gain_u0 = pFdCfg[i].gain_u[3];
		
		ai_chn = pFdCfg[i].mmi_meaNo_ai[MMI_MEA_I0];
        if (ai_chn >= 0)
		{
		    pFdCfg[i].gain_i0 = pMeaGain[ai_chn].a;
		//	if(YC_GAIN_VALID_GET(pMeaGain[ai_chn].status) == 0)
		//	   valid = 0;
        }
		else
			pFdCfg[i].gain_i0 = pFdCfg[i].gain_i[3];
		if (valid == 0)
		    WriteWarnEvent(NULL, SYS_ERR_GAIN, 0, "回线%d增益系数错误", i+1);
		pFdCfg[i].valid = valid;
	}
}

#ifdef INCLUDE_FA_S


int FaValtoFd(void)//(DWORD SwID,VFasimSet*pbuf)
{
       int i,fd;
	VFasimData pbuf;
	BOOL ch = false;

  	struct VDBYCF_L  prYcSet;//add by lqh 20150611
	struct VDBYX prYxSet;	//add by lqh 20150611

	//pbuf = (VFasimData*)malloc(sizeof(VFasimData));
	fd = prRunInfo->fd;	
	if(ReadSimData(fd, &pbuf) == TRUE)
	{
	      FaSimWriteYcUIset(fd, &pbuf);//rem by lqh 20150602 for test
	      FaSimWriteYxUIset(fd, &pbuf);
		  for(i=0; i<4; i++)
		  {
		    pVal->dU[i] = (long long)pbuf.wU[i]*pFdCfg->gain_u[i] /100 / 100;			//add /100 by lqh 20150608
		    pVal->dI[i] = (long long)pbuf.wI[i]* pFdCfg->gain_i[i] / 5 / 1000;	//add /1000 by lqh  20150608

		  }
		pVal->dI0 = pVal->dSI0 = pVal->dI[3]* pFdCfg->gain_i[3] / 5;//?????
		pVal->dU0 = pVal->dSU0 = pVal->dU[3];//??????
		#ifdef INCLUDE_YX
		if (pbuf.byYx[0] == 1)
		{
			#if defined(INCLUDE_FA)||defined(INCLUDE_FA_SH)
	        if ((pVal->dYx & PR_KG_YX_STATUS) == 0)
		   	  NotifyYx(pRunInfo->fd, 0x01);
			#endif
		   pVal->dYx |= PR_KG_YX_STATUS;
		}
		else 
		{
		   #if defined(INCLUDE_FA)||defined(INCLUDE_FA_SH)
		   if (pVal->dYx & PR_KG_YX_STATUS)
		   	   NotifyYx(pRunInfo->fd, 0x0);
		   #endif
		   pVal->dYx &= (~PR_KG_YX_STATUS);
		}
		pVal->dYx |= PR_KG_YX_CH;
		#endif
		return OK;
	
    }
	#if 0	//rem by lqh 20150612 for test
	else		//add by lqh 20150611 unlock yc and yx
	{
	
		for(i = 0; i < 8; i++)
		{
			prYcSet.wNo = pFdCfg[0].mmi_meaNo_yc[i];
			prYcSet.lValue = 0;
			prYcSet.byFlag |= 0x01;
			UnLockYC(0,1,&prYcSet);		
		}

		prYxSet.wNo = pFdCfg[0].kg_stateno;
		prYxSet.byValue = 0x01;
		UnLockSYX(0,1,&prYxSet);	
	}
	#endif
	return ERROR;
}

void FaSimWriteYcUIset(int SwID,VFasimData*pbuf)
{
	int i,j,k,m,offset=4;
  	struct VDBYCF_L  prYcSet;
	//if( ReadSimData(SwID, pbuf) == true)   //FaValtoFd(SwID,pbuf) == OK
	//{
		//for (i=0; i<fdNum; i++)
	    	//{
			//for (j=0; j<ycNum; j++)
			//{
	 			//if (pYcCfg[j].fdNo == i)

					for(k=0;k<4;k++)
					{
						prYcSet.wNo = pFdCfg[SwID].mmi_meaNo_yc[k];
						prYcSet.lValue = pbuf->wI[k];//*1000;//.dI[k]* pFdCfg->gain_i[k] / pFdCfg->In;	//rem *1000 by lqh 20150608
						prYcSet.byFlag |= 0x01;
						LockYC(0,1,&prYcSet);		
					}
					for(m=0;m<4;m++)
					{
						prYcSet.wNo = pFdCfg[SwID].mmi_meaNo_yc[m+offset];
						prYcSet.lValue = pbuf->wU[m];//*100;	//rem *100 by lqh 20150608
						prYcSet.byFlag |= 0x01;
						LockYC(0,1,&prYcSet);
						//ppPlanVal->dU[i][j] = 57 * pFdCfg->gain_u[j] / 100;
					}
			//}
		  // }
	//}

}


void FaSimWriteYxUIset(int SwID,VFasimData*pbuf)
{
	int i;
  	struct VDBYX prYxSet;

	//if(ReadSimData(SwID, pbuf) == true)
	//{
		//for(i=0;i<SwID;i++)
		//{
				prYxSet.wNo = pFdCfg[SwID].kg_stateno;
				if(pbuf->byYx[0])
				prYxSet.byValue = 0x81;
				else
					prYxSet.byValue = 0x01;
				LockSYX(0,1,&prYxSet);	
		//}	
	//}
}
#endif

void protValtoPub(void)
{
  int i;
	
	for (i=0; i<4; i++)
	{
		if (pCfgSL->pdU[i]) pValSL->dU[i] = *(pCfgSL->pdU[i]);
	}
	
	if(pPublicSet->dAi_S_No != 0)
		pValSL->dU[0] = aiProtCal[pPublicSet->dAi_S_No-1].dUI;
	
	if(pPublicSet->dAi_F_No != 0)
		pValSL->dU[2] = aiProtCal[pPublicSet->dAi_F_No-1].dUI;

#ifdef INCLUDE_YX
	if (GetMyDiValue(pCfgSL->kg_stateno))
	{
#if defined(INCLUDE_FA)||defined(INCLUDE_FA_SH)
       if ((pValSL->dYx & PR_KG_YX_STATUS) == 0)
	   	  NotifyYx(pRunInfo->fd, 0x01);
#endif
	   pValSL->dYx |= PR_KG_YX_STATUS;
	}
	else 
	{
#if defined(INCLUDE_FA)||defined(INCLUDE_FA_SH)
	   if (pValSL->dYx & PR_KG_YX_STATUS)
	   	   NotifyYx(pRunInfo->fd, 0x0);
#endif
	   pValSL->dYx &= (~PR_KG_YX_STATUS);
	}
#endif
}

extern int GetFaOnInput(void);
void protValtoFd(void)
{
    int i;

#ifdef INCLUDE_FA_S
    if(FaValtoFd() == OK) return;
#endif
	for (i=0; i<4; i++)
	{
		if (pCfg->pdI[i]) pVal->dI[i] = *(pCfg->pdI[i]);
		if (pCfg->pdU[i]) pVal->dU[i] = *(pCfg->pdU[i]);
        if (pCfg->pdI_Hw2[i]) pVal->dI_Hw2[i] = *(pCfg->pdI_Hw2[i]);
	}

	
	if (pCfg->pdI0) pVal->dI0 = *(pCfg->pdI0);
	if (pCfg->pdSI0) pVal->dSI0 = *(pCfg->pdSI0);

	if (pCfg->pdU0) pVal->dU0 = *(pCfg->pdU0);
	if (pCfg->pdSU0) pVal->dSU0 = *(pCfg->pdSU0);
    if(pCfg->pdUI0Bak) pVal->dUI0Bak = *(pCfg->pdUI0Bak);
	
	for (i=0; i<3; i++)
	{
	     if (pCfg->pdUxx[i]) 
			 pVal->dUxx[i] = *(pCfg->pdUxx[i]);
		 if (pCfg->pdUSlip[i])
		 	 pVal->dUDelta[i] = *(pCfg->pdUSlip[i]);
	}

	pVal->dIMax = (pVal->dI[0] > pVal->dI[1]) ? pVal->dI[0]:pVal->dI[1];
	pVal->dIMax = (pVal->dIMax > pVal->dI[2]) ? pVal->dIMax:pVal->dI[2];

#ifdef INCLUDE_YX
	if (GetMyDiValue(pCfg->kg_stateno))
	{
#if defined(INCLUDE_FA)||defined(INCLUDE_FA_SH)
       if ((pVal->dYx & PR_KG_YX_STATUS) == 0)
	   	  NotifyYx(pRunInfo->fd, 0x01);
#endif

#ifdef INCLUDE_FA_DIFF
	   if ((pVal->dYx & PR_KG_YX_STATUS) == 0)
	   	  NotifyFaDiffYx(pRunInfo->fd, 0x01);
#endif

	   pVal->dYx |= PR_KG_YX_STATUS;
	}
	else 
	{
#if defined(INCLUDE_FA)||defined(INCLUDE_FA_SH)
	   if (pVal->dYx & PR_KG_YX_STATUS)
	   	   NotifyYx(pRunInfo->fd, 0x0);
#endif

#ifdef INCLUDE_FA_DIFF
	   if (pVal->dYx & PR_KG_YX_STATUS)
	   	  NotifyFaDiffYx(pRunInfo->fd, 0x0);
#endif

	   pVal->dYx &= (~PR_KG_YX_STATUS);
	}
	
#ifdef _GUANGZHOU_TEST_VER_
	if (GetMyDiValue(KG_CH_YB))  pVal->dYx |= PR_KG_YX_CH;
	else pVal->dYx &= (~PR_KG_YX_CH);
#else
#if (TYPE_USER == USER_WUXI)
    if(GetFaOnInput()) 
			pVal->dYx |= PR_KG_YX_CH;
	else
			pVal->dYx &= (~PR_KG_YX_CH);
#else
	pVal->dYx |= PR_KG_YX_CH;
#endif
#endif

#endif
}

#ifdef INCLUDE_FA_DIFF
void protBusValtoFd(void)
{
   int i,j;
   VFdProtCal *pVal;
   VBusProtCal *pBusVal = &BusVal;

   for (i=0; i<3; i++)
   {
      for (j=0; j<24; j++)
      {
          pBusVal->dBusI[i][j] = aiBusProtCal[i][j].dUI;
      }

   }

   pBusVal->dBrkI[0] = pBusVal->dBrkI[1] = pBusVal->dBrkI[2] = 0;
   for (i=0; i<fdNum; i++)
   {

       pVal = fdProtVal + i;
       if ((pVal->dYx & PR_KG_YX_STATUS) == 0)
	   	  continue;
	   pBusVal->dBrkI[0] += pVal->dI[0];
	   pBusVal->dBrkI[1] += pVal->dI[1];
	   pBusVal->dBrkI[2] += pVal->dI[2];
   }
}

void protCal_HwEven(int ai_chn, int count, int hw, DWORD* result) 
{
    int i, j;
	long lCos[4];   //实部
	long lSin[4];   //虚部
	long lSam,result1;
	const long *pFAFre, *pFAFim;
	int cur,last;

    if (hw < 1) return;
	
	pFAFre = &laFAFre[hw-1][0];	
    pFAFim = &laFAFim[hw-1][0];

	last = (wFtSamCnt - FT_SAM_POINT_N - count - FT_DIFF)&(SAMFT_BUF_LEN-1);


	lCos[0]=lCos[1]=lCos[2]=lCos[3]=0;
	lSin[0]=lSin[1]=lSin[2]=lSin[3]=0;	
	cur = last;
	for (i=0; i<FT_SAM_POINT_N; i++)
	{
		lSam=nFtBus[(cur+FT_DIFF)&(SAMFT_BUF_LEN-1)][ai_chn]-nFtBus[cur][ai_chn];
		lCos[0]+=lSam*pFAFre[i];
		lSin[0]+=lSam*pFAFim[i];
	    cur = (cur + 1)&(SAMFT_BUF_LEN - 1);

	}

	for (i=1; i<count; i++)
	{
	    lSam=nFtBus[(last+FT_DIFF)&(SAMFT_BUF_LEN-1)][ai_chn]-nFtBus[last][ai_chn];
		result1 = lSam*pFAFim[i];
		lSin[i] = lSin[i-1] - result1;

		cur = (last+FT_SAM_POINT_N)&(SAMFT_BUF_LEN-1);
		lSam=nFtBus[(cur+FT_DIFF)&(SAMFT_BUF_LEN-1)][ai_chn]-nFtBus[cur][ai_chn];
		result1 = lSam*pFAFim[i];
		lSin[i] = lSin[i] + result1;
		lCos[i] = lCos[i-1];

		last = (last+1)&(SAMFT_BUF_LEN-1);
		
	}

	for (i=0; i<count; i++)
	{
	   lCos[i] >>=7;
	   lSin[i] >>=7;
	   result[i] = AmXY(lCos[i],lSin[i]);
	}
}

void protCal_BusVal(void)
{
    WORD wFtCur,j;
	DWORD result[4];

	int i,k;
	
    wFtCur = wFtPtr;
	j = (wFtCur-4)&(FT_BUF_LEN-1);
	
	for (i=0; i<4; i++)
	{
		for (k=0; k<6; k++)
	    {
			aiBusProtCal[0][i*6+k].UI.x = lFtBus[j][2*k];
			aiBusProtCal[0][i*6+k].UI.y = lFtBus[j][2*k+1];

			aiBusProtCal[0][i*6+k].dUI = F_AMP(aiBusProtCal[0][i*6+k].UI);

			aiBusProtCal[1][i*6+k].UI.x = lFtBus[j][2*k+12];
			aiBusProtCal[1][i*6+k].UI.y = lFtBus[j][2*k+12+1];

			aiBusProtCal[1][i*6+k].dUI = F_AMP(aiBusProtCal[1][i*6+k].UI);

			aiBusProtCal[2][i*6+k].UI.x = lFtBus[j][2*k+24];
			aiBusProtCal[2][i*6+k].UI.y = lFtBus[j][2*k+24+1];

			aiBusProtCal[2][i*6+k].dUI = F_AMP(aiBusProtCal[2][i*6+k].UI);
		}

		j = (j+1)&(FT_BUF_LEN-1);
	}
}
#endif
int RD_Last_P(int type, WORD pts, COMPLEX *pCmp)
{
	WORD wFtCur; 
	int j;

	wFtCur = wFtPtr;
	j = (wFtCur - pts)&(FT_BUF_LEN-1);

	pCmp->x = 0;
	pCmp->y = 0;

	if(pCfg->mmi_meaNo_ai[type] < 0)
	   return 0;

	pCmp->x = lFt[j][pCfg->mmi_meaNo_ai[type]*2];
	pCmp->y = lFt[j][pCfg->mmi_meaNo_ai[type]*2+1];
	return 1;
}
//static WORD wPrtFtPtr;

int protCal_Hw2(int ai_chn)
{
    int i;
	long lCos;   //实部
	long lSin;   //虚部
	long lSam;
	const long *pFAFre, *pFAFim;
	int cur;

	if (ai_chn >= aiNUM) return 0;

	pFAFre = &laFAFre[1][0];	
    pFAFim = &laFAFim[1][0];

	cur = (wFtSamCnt - FT_SAM_POINT_N)&(SAMFT_BUF_LEN-1);

	lCos=0;lSin=0;		
	for(i=0; i<FT_SAM_POINT_N; i++)
	{
		lSam=nFtSam[cur&(SAMFT_BUF_LEN-1)][ai_chn]-nFtSam[(cur-FT_DIFF)&(SAMFT_BUF_LEN-1)][ai_chn];
		lCos+=lSam*pFAFre[i];
		lSin+=lSam*pFAFim[i];
	    cur = (cur + 1)&(SAMFT_BUF_LEN - 1);

	}

	
	lCos >>=7;
	lSin >>=7;
	return(AmXY(lCos,lSin));
}

#ifdef INCLUDE_PR_PRO
void protCal_AiFVal()
{
    int i, j, k, m, type,index1,index2,cnt;
	WORD wFtCur,freq1,freq2;   
	VFdCfg *pTempCfg;
	DWORD ul;
	long sum, ang, diff_ang, last_ang, diff_f;

    wFtCur = wFtPtr;
	m = 0;
    //前两回线的A相电压
    if (pPublicSet->bTqLine)
	{
		for (i=0; i<(fdNum-pPublicSet->byTqLine); i++)
		{
		   pTempCfg = pFdCfg + pPublicSet->byTqLine + i;
		   if((type=pTempCfg->mmi_meaNo_ai[MMI_MEA_UA]) < 0)
		      continue;
		   if (m == 0)
		   	  index1 = type;
		   else if (type == index1)
		   	  continue;
		   else
		   	  index1 = type;
		   for (j=0; j<10; j++)
		   {
		      k = (wFtCur-(FT_SAM_POINT_N>>1)*j)&(FT_BUF_LEN-1);
			  aiFreqBuf.u[9-j][m].x = lFt[k][index1*2];
			  aiFreqBuf.u[9-j][m].y = lFt[k][index1*2+1];
		   }
		   pTqVal->cTqU[m] = aiProtCal[index1].UI;
		   pTqVal->dTqU[m] = aiProtCal[index1].dUI;

		   m++;
		   if (m >= 2)
		   	 break;
		}
    }
    if (m < 2)
    {
        pTempCfg = pFdCfg + pPublicSet->byTqLine;
		if((type=pTempCfg->mmi_meaNo_ai[MMI_MEA_UA]) > 0)
		   index1 = type;
		for (j=0; j<10; j++)
		{
		    k = (wFtCur-(FT_SAM_POINT_N>>1)*j)&(FT_BUF_LEN-1);
			aiFreqBuf.u[9-j][0].x = lFt[k][index1*2];
			aiFreqBuf.u[9-j][0].y = lFt[k][index1*2+1];
		}
		pTqVal->cTqU[0] = aiProtCal[index1].UI;
		pTqVal->dTqU[0] = aiProtCal[index1].dUI;
		if((type=pTempCfg->mmi_meaNo_ai[MMI_MEA_UC]) > 0)
		   index1 = type;
		for (j=0; j<10; j++)
		{
		    k = (wFtCur-(FT_SAM_POINT_N>>1)*j)&(FT_BUF_LEN-1);
			aiFreqBuf.u[9-j][1].x = lFt[k][index1*2];
			aiFreqBuf.u[9-j][1].y = lFt[k][index1*2+1];
		}
		pTqVal->cTqU[1] = aiProtCal[index1].UI;
		pTqVal->dTqU[1] = aiProtCal[index1].dUI;
    }
	if (aiFreqBuf.bCmpOrg == FALSE)
	{
	    if (wFtCur > 100)  //5个周波
	       aiFreqBuf.bCmpOrg = TRUE;
		for (i=0; i<2; i++)
        {
           pTqVal->wTqFreq[i]  = 5000;
		   pTqVal->wTqFSlip[i] = 0;
		}
		return;
	}
	spFreqBuf.cnt[spFreqBuf.index] = wFtCur;
	for (i=0; i<2; i++)
	{
		sum = 0;
		last_ang = 0;
		for (j=0; j<9; j++)
		{
		   ul = F_AMP(aiFreqBuf.u[9-j][i]);
   	       if (ul<pPublicSet->dTqUMin[0])
		   {
		       pTqVal->wTqFreq[i] = 5000;
			   spFreqBuf.freq[spFreqBuf.index][i] = 5000;
		       break;
		   }
		   else
		   {
		       ang = F_ANG(aiFreqBuf.u[9-j][i], ul);
			   if (j > 0)                    //只加80ms
			   {
				   diff_ang = ang - last_ang;
				   if (diff_ang > 11796480)
				   	 diff_ang -= 23592960;
				   if (diff_ang <= -11796480)
				   	 diff_ang += 23592960;
				   sum += diff_ang;	
			   }
			   last_ang = ang;
		   }
		}
		diff_f = (sum>>8) *125/ 36;   //freq_sum/(360/12.5),12.5 = 1000/80
		diff_f = diff_f >> 8;
		pTqVal->wTqFreq[i] = 5000 - diff_f;
		spFreqBuf.freq[spFreqBuf.index][i] = pTqVal->wTqFreq[i];
	}
	spFreqBuf.index = (spFreqBuf.index + 1)&(AIFREQ_BUF_LEN-1);


	if (spFreqBuf.bSlipOrg == FALSE)
	{
	   if (spFreqBuf.index > 11)
	   	  spFreqBuf.bSlipOrg = TRUE;
	   pTqVal->wTqFSlip[0] = 0;
	   pTqVal->wTqFSlip[1] = 0;
	   return;
	}

    index1 = (spFreqBuf.index - 1)&(AIFREQ_BUF_LEN - 1);
    cnt = spFreqBuf.cnt[index1];
	index2 = (spFreqBuf.index - 10 -1)&(AIFREQ_BUF_LEN - 1);
	cnt = cnt - spFreqBuf.cnt[index2];
	for (i=0; i<2; i++)
	{
	    freq1 = spFreqBuf.freq[index1][i];
		freq2 = spFreqBuf.freq[index2][i];
		if (cnt == 0)
			spFreqBuf.delta[index1][i] = 0;
		else
		    spFreqBuf.delta[index1][i] = (freq1 - freq2)*1000/cnt;

		diff_f = 0;
		for (j=0; j<6; j++)
		{
		   index2 = (index1 - j)&(AIFREQ_BUF_LEN - 1);
		   diff_f += spFreqBuf.delta[index2][i];
		}
		diff_f = (diff_f > 0) ? diff_f:(-diff_f);
		pTqVal->wTqFSlip[i] = diff_f/6;
	}
}
#endif

void protCal_AiVal(void)
{
    int i, j,  na, nb, nc;
	WORD wFtCur;   
	long *pFtB0;		//当前傅氏滤波缓冲指针
	long *pFtB100;		//计算电压差,100ms前电压复式
	DWORD dUxxPre=0;
	COMPLEX xUxxPre;


	wFtCur = wFtPtr;
	j = (wFtCur-100)&(FT_BUF_LEN-1);


	//wPrtFtPtr=wFtCur;
	

	for( i=0; i<(aiNUM<<1); i++ )
	{
		arlFt0[i]  = lFt[wFtCur][i];
		arlFt10[i] = lFt[j][i];
	}

    pFtB0=arlFt0;		
	pFtB100=arlFt10;	

	memset(aiProtCal, 0, sizeof(aiProtCal));

	for (i=0; i<aiNUM; i++)
	{
		if (pAiCfg[i].protect_enable)
        {
			if( (pAiCfg[i].type == YC_TYPE_Ua)||(pAiCfg[i].type == YC_TYPE_Ub)||(pAiCfg[i].type == YC_TYPE_Uc) || (pAiCfg[i].type == YC_TYPE_I0))
        	{//WAAA   半周 计算电压!
				aiProtCal[i].UI.x =*pFtB0;
				aiProtCal[i].UIPre.x = *pFtB100;
				pFtB0++; pFtB100++;
				aiProtCal[i].UI.y =*pFtB0;
				aiProtCal[i].UIPre.y = *pFtB100;
				pFtB0++; pFtB100++;
        	}
			else
        	{
				aiProtCal[i].UI.x = *pFtB0;
				pFtB0++; pFtB100++;
				aiProtCal[i].UI.y = *pFtB0;
				pFtB0++; pFtB100++;
				aiProtCal[i].dUI_Hw = protCal_Hw2(i);
        	}				
			
			aiProtCal[i].dUI = AmXY(aiProtCal[i].UI.x, aiProtCal[i].UI.y);
					
			if(pAiCfg[i].type == YC_TYPE_I0)
				aiProtCal[i].dUI0Bak = AmXY(aiProtCal[i].UIPre.x, aiProtCal[i].UIPre.y);
        }
		else
		{
			pFtB0+=2;
			pFtB100+=2;
		}		
	}


	for (i=0; i<aiNUM; i++)
	{
        if (!pAiCfg[i].protect_enable) continue;
		
		dUxxPre = 0;
		
		if (pAiCfg[i].type == YC_TYPE_Ia)
		{            
			nb = pYcCfg[pAiCfg[i].ycNo+1].arg1;
			nc = pYcCfg[pAiCfg[i].ycNo+2].arg1;

            if ((nb < aiNUM) && (nc < aiNUM))
            {
				aiProtCal[i].SUI0 = ADD(ADD(aiProtCal[i].UI,aiProtCal[nb].UI),aiProtCal[nc].UI);
	            aiProtCal[i].dSUI0 = F_AMP(aiProtCal[i].SUI0);
            }	
		}
		else if (pAiCfg[i].pt_58v) /*Y*/
		{
			if (pAiCfg[i].type == YC_TYPE_Ua)
			{
				nb = pYcCfg[pAiCfg[i].ycNo+1].arg1;
				nc = pYcCfg[pAiCfg[i].ycNo+2].arg1;

	            if ((nb < aiNUM) && (pAiCfg[nb].type == YC_TYPE_Ub) && 
					(nc < aiNUM) && (pAiCfg[nc].type == YC_TYPE_Uc))
	            {				
					aiProtCal[i].SUI0 = ADD(ADD(aiProtCal[i].UI,aiProtCal[nb].UI),aiProtCal[nc].UI);
	                aiProtCal[i].dSUI0 = F_AMP(aiProtCal[i].SUI0);
	            }	
				if ((nb < aiNUM) && (pAiCfg[nb].type == YC_TYPE_Ub))
				{
				    aiProtCal[i].Uxx = SUB(aiProtCal[i].UI, aiProtCal[nb].UI);
					aiProtCal[i].dUxx = F_AMP(aiProtCal[i].Uxx);

                    xUxxPre = SUB(aiProtCal[i].UIPre, aiProtCal[nb].UIPre);
					dUxxPre = F_AMP(xUxxPre);
				}
			}			
			else if (pAiCfg[i].type == YC_TYPE_Ub)
			{
				nc = pYcCfg[pAiCfg[i].ycNo+1].arg1;

				if ((nc < aiNUM) && (pAiCfg[nc].type == YC_TYPE_Uc))
				{
					aiProtCal[i].Uxx = SUB(aiProtCal[i].UI, aiProtCal[nc].UI);
					aiProtCal[i].dUxx = F_AMP(aiProtCal[i].Uxx); 

					xUxxPre = SUB(aiProtCal[i].UIPre, aiProtCal[nc].UIPre);
					dUxxPre = F_AMP(xUxxPre);
				}	
			}
			else if (pAiCfg[i].type == YC_TYPE_Uc)
			{
			    na = 0;
			    if((pAiCfg[i].ycNo-1) >= 0)
			       na = pYcCfg[pAiCfg[i].ycNo-1].arg1;
				if((na < aiNUM)&&(pAiCfg[na].type == YC_TYPE_Ub))
                {
                   if((pAiCfg[i].ycNo-2) >= 0)
                     na = pYcCfg[pAiCfg[i].ycNo-2].arg1;
				}
	          			
				if ((na < aiNUM)&&(pAiCfg[na].type == YC_TYPE_Ua))
				{
					aiProtCal[i].Uxx = SUB(aiProtCal[i].UI, aiProtCal[na].UI);
				    aiProtCal[i].dUxx = F_AMP(aiProtCal[i].Uxx);  

					xUxxPre = SUB(aiProtCal[i].UIPre, aiProtCal[na].UIPre);
					dUxxPre = F_AMP(xUxxPre);

				}
	
			}
			if (aiProtCal[i].dUxx > dUxxPre)
				aiProtCal[i].dUDelta = (aiProtCal[i].dUxx - dUxxPre)*10;
			else
			    aiProtCal[i].dUDelta = (dUxxPre - aiProtCal[i].dUxx)*10;
		}
		else /*VV*/
		{
		    if (pAiCfg[i].type == YC_TYPE_Ua)
			{
			    aiProtCal[i].Uxx = aiProtCal[i].UI;
				aiProtCal[i].dUxx = F_AMP(aiProtCal[i].Uxx);

				xUxxPre = aiProtCal[i].UIPre;
				dUxxPre = F_AMP(xUxxPre);
				
			}		
		    if (pAiCfg[i].type == YC_TYPE_Uc)
			{
	            if ((pAiCfg[i].ycNo-1) >= 0)
	            {				
					na = pYcCfg[pAiCfg[i].ycNo-1].arg1;

					if ((na < aiNUM)&&(pAiCfg[na].type == YC_TYPE_Ua))
					{
						aiProtCal[i].Uxx = SUB(aiProtCal[i].UI, aiProtCal[na].UI);
						aiProtCal[i].dUxx = F_AMP(aiProtCal[i].Uxx); 

						xUxxPre = SUB(aiProtCal[i].UIPre, aiProtCal[na].UIPre);
				        dUxxPre = F_AMP(xUxxPre);
					}
	            }
		    }
			if (aiProtCal[i].dUxx <= dUxxPre)
				aiProtCal[i].dUDelta = (dUxxPre - aiProtCal[i].dUxx)*10;
			else
			    aiProtCal[i].dUDelta = (aiProtCal[i].dUxx - dUxxPre)*10;
		}
	}
#ifdef INCLUDE_FA_DIFF
    protCal_BusVal();
#endif
}

#endif
