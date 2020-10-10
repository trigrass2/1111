#include "syscfg.h"
#ifdef INCLUDE_PB_RELATE
#include "pb.h"
#include "sys.h"
#include "db.h"

VPbPara ParaFile;
static VPbSet  g_pbSet[MAX_FD_NUM];
BYTE extEnergyT[48]={0};

BYTE PowFlag;
BYTE GZFlag = 0;
BYTE ZZFlag = 0;
VPbCalTime pCalTime[2];
static VPbData g_pbYc[MAX_FD_NUM];
static VPbTime g_pbTime[MAX_FD_NUM];
static VPbCfg  g_pbYcCfg[MAX_FD_NUM];
static VPbUPass g_pbUPass;
extern VPbSet  g_pbSet[MAX_FD_NUM];
static VPbAvg  g_pbAvg[MAX_FD_NUM];

static VYcCfg *pPbCfg;
static int  pbfdnum;
static int  pbycnum;

short mP = 0;
short mQ = 0;

long pbGetUnBlanceU(int fd)
{
 	 return(g_pbYc[fd].unBlance_U);
}

long pbGetUnBlanceI(int fd)
{
     return(g_pbYc[fd].unBlance_I);
}

short pbGetPerPassU(int fd,int no)
{
    if((no > 2) || (no < 0))
      return 0;
    else
     return (short)g_pbYc[fd].perPass_U[no];
}

short pbGetLoadRate(int fd)
{
	return(g_pbYc[fd].LoadRate);
}

short pbGetPerPassUM(int fd, int no)
{   
    if((no > 2) || (no < 0))
      return 0;
    else
     return (short)g_pbYc[fd].perPass_U_m[no];
}

short pbGetOverU(int fd, int no)
{
    if((no > 2) || (no < 0))
      return 0;
    else
     return (short)g_pbTime[fd].u_over_t[no];
}

short pbGetBelowU(int fd, int no)
{
    if((no > 2) || (no < 0))
      return 0;
    else
     return (short)g_pbTime[fd].u_below_t[no];
}

short pbGetOverUM(int fd, int no)
{
    if((no > 2) || (no < 0))
      return 0;
    else
     return (short)g_pbTime[fd].u_over_t_m[no];
}

short pbGetBelowUM(int fd, int no)
{
    if((no > 2) || (no < 0))
      return 0;
    else
     return (short)g_pbTime[fd].u_below_t_m[no];
}

short pbGetAvgU(int fd, int no)
{
    if((no > 2) || (no < 0))
      return 0;
    else
     return (short)g_pbAvg[fd].uavg[no];
}

void PbYcRead(void)
{
     int i,j;
	 VPbCfg *pCfg;
	 VPbData *pData;
     for (i=0; i<pbfdnum; i++)
     {
        pCfg = g_pbYcCfg + i;
		pData = g_pbYc + i;
		for (j=0; j<3; j++) 
        {
           if (pCfg->u_index[j] != -1)
              ReadRangeAllYC(g_Sys.wIOEqpID, (WORD)pCfg->u_index[j], 1, sizeof(short), pData->u+j);
		}
		for (j=0; j<3; j++)
		{
           if (pCfg->i_index[j] != -1)
              ReadRangeAllYC(g_Sys.wIOEqpID, (WORD)pCfg->i_index[j], 1, sizeof(short), pData->i+j);    
		}
		for (j=0; j<3; j++) 
        {
           if (pCfg->uph_index[j] != -1)
              ReadRangeAllYC(g_Sys.wIOEqpID, (WORD)pCfg->uph_index[j], 1, sizeof(short), pData->u_phase+j);
		}
		for (j=0; j<3; j++)
		{
           if (pCfg->iph_index[j] != -1)
              ReadRangeAllYC(g_Sys.wIOEqpID, (WORD)pCfg->iph_index[j], 1, sizeof(short), pData->i_phase+j);    
		}
		if (pCfg->p_index != -1)
		{
			ReadRangeAllYC(g_Sys.wIOEqpID, (WORD)pCfg->p_index, 1, sizeof(short), &pData->p); 
			mP = pData->p;
		}
             
		if (pCfg->q_index != -1)
		{
			ReadRangeAllYC(g_Sys.wIOEqpID, (WORD)pCfg->q_index, 1, sizeof(short), &pData->q); 
			mQ = pData->q;
		}
              
		if (pCfg->s_index != -1)
              ReadRangeAllYC(g_Sys.wIOEqpID, (WORD)pCfg->s_index, 1, sizeof(short), &pData->s);  
     }
}

void PbUnBlance(void)
{
	int i;
	VPbCfg *pCfg;
	VPbData *pData;
	VPbTime *pTime;
	for (i=0; i<pbfdnum; i++)
	{
		pData = g_pbYc + i;
		pTime = g_pbTime + i;
		pCfg  = g_pbYcCfg + i;
		if ((pCfg->uph_index[0] != -1) && (pCfg->uph_index[1] != -1))
		{
			if ((pData->u_phase[0] <= 100)&&(pData->u_phase[1] >= 100))
				pData->unBlance_U = 1000;
			else if (pData->u_phase[0] <= 100)
				pData->unBlance_U = 0;
//			else if(pData->u_phase[1] > pData->u_phase[0])
//				pData->unBlance_U = 1000;
			else
			{
				pData->unBlance_U = (long)pData->u_phase[1]*1000/(long)pData->u_phase[0];
				if(pData->unBlance_U > 100000)
					pData->unBlance_U = 1000;
			}
		}
		else if ((pCfg->uph_index[0] != -1) && (pCfg->uph_index[2] != -1))
		{
			if ((pData->u_phase[0] <= 100)&&(pData->u_phase[2] >= 100))
				pData->unBlance_U = 1000;
			else if (pData->u_phase[0] <= 100)
				pData->unBlance_U = 0;
//			else if(pData->u_phase[2] > pData->u_phase[0])
//				pData->unBlance_U = 1000;
			else
			{
				pData->unBlance_U = (long)pData->u_phase[2]*1000/(long)pData->u_phase[0];
				if(pData->unBlance_U > 100000)
					pData->unBlance_U = 1000;
			}
		}
		else
			pData->unBlance_U = 0;

		if ((pCfg->iph_index[0] != -1) && (pCfg->iph_index[1] != -1))
		{
			if ((pData->i_phase[0] <= 100)&&(pData->i_phase[1] >= 100))
				pData->unBlance_I = 1000;
			else if (pData->i_phase[0] <= 100)
				pData->unBlance_I = 0;
	//		else if(pData->i_phase[1] > pData->i_phase[0])
	//			pData->unBlance_I = 1000;
			else
			{
				pData->unBlance_I = (long)pData->i_phase[1]*1000/(long)pData->i_phase[0];
				if(pData->unBlance_I > 100000)
					pData->unBlance_I = 1000;
			}
		}
		else if ((pCfg->iph_index[0] != -1) && (pCfg->iph_index[2] != -1))
		{	 
			if ((pData->i_phase[0] <= 100)&&(pData->i_phase[2] >= 100))
				pData->unBlance_I = 1000;
			else if (pData->i_phase[0] <= 100)
				pData->unBlance_I = 0;
	//		else if(pData->i_phase[2] > pData->i_phase[0])
	//			pData->unBlance_I = 1000;
			else
			{
				pData->unBlance_I = (long)pData->i_phase[2]*1000/(long)pData->i_phase[0];
				if(pData->unBlance_I > 100000)
					pData->unBlance_I = 1000;
			}
		}
		else
		 	pData->unBlance_I = 0;
  
		if (pData->unBlance_U > g_pbSet[i].u_unblance.unblance_up)
		{
		    pTime->u_unblance_t++;
			if (pTime->u_unblance_t >= g_pbSet[i].u_unblance.t)
			   pData->yx |= PB_YX_U_UNB;
		}
		else if (pData->unBlance_U < g_pbSet[i].u_unblance.unblance_rec)
		{
			pTime->u_unblance_t = 0;
			pData->yx &= ~PB_YX_U_UNB;
		}
			
	    if (pData->unBlance_I > g_pbSet[i].i_unblance.unblance_up)
		{
			pTime->i_unblance_t++;
			if (pTime->i_unblance_t >= g_pbSet[i].i_unblance.t)
				pData->yx |= PB_YX_I_UNB;
		}
		else if (pData->unBlance_I < g_pbSet[i].i_unblance.unblance_rec)
		{
		    pTime->i_unblance_t = 0;
			pData->yx &= ~PB_YX_I_UNB;
		}
  
     }
}
void PbOverLoad(void)
{
     int i;
	 VPbCfg *pCfg;
	 VPbData *pData;
	 VPbTime *pTime;
	 VRunParaCfg *prunparacfg;
	 prunparacfg = &g_Sys.MyCfg.RunParaCfg;
     for (i=0; i<pbfdnum; i++)
     {
         pData = g_pbYc + i;
		 pTime = g_pbTime + i;
		 pCfg  = g_pbYcCfg + i;
		 
	    if (pCfg->s_index == -1)
			continue;
		if ((pData->LoadRate*100)>(short)prunparacfg->dwGzVal)//过载重载门限值为80%,下载定值为80，扩大100倍
		{
			if(pTime->GZ_t == 0)
			{
				GetSysClock(&(pCalTime[0].startT), SYSCLOCK);
			}
		  
		   pTime->GZ_t++;
		   
		   if ((pTime->GZ_t/60) >= prunparacfg->wGzT)
		   	{
		   		pData->yx |= PB_YX_GZ;
				GZFlag = 1;
				
		   	}
		   	 
		}
		else
		{
			if(GZFlag == 1)
			{
			  pCalTime[0].total_t = pTime->GZ_t;
			  GetSysClock(&(pCalTime[0].endT), SYSCLOCK);
			  histtu2file_write((BYTE*)&pCalTime[0], 0, SYSEV_FLAG_HL);
			  GZFlag = 0;
			}
		    pTime->GZ_t = 0;
			pData->yx &= ~PB_YX_GZ;
		}
     }
}
void PbHeavyLoad(void)
{
     int i;
	 VPbCfg *pCfg;
	 VPbData *pData;
	 VPbTime *pTime;
	 VRunParaCfg *prunparacfg;
	 prunparacfg = &g_Sys.MyCfg.RunParaCfg;
     for (i=0; i<pbfdnum; i++)
     {
         pData = g_pbYc + i;
		 pTime = g_pbTime + i;
		 pCfg  = g_pbYcCfg + i;
		 
	    if (pCfg->s_index == -1)
			continue;
		if ((pData->LoadRate*100)>(short)prunparacfg->dwZzVal)
		{
			if(pTime->ZZ_t == 0)
			{
				GetSysClock(&(pCalTime[1].startT), SYSCLOCK);
			}
		   pTime->ZZ_t++;
		   if ((pTime->ZZ_t/60) >= prunparacfg->wZzT)
		   	{
		   		pData->yx |= PB_YX_ZZ;
				ZZFlag = 1;
		   	}
		   	 
		}
		else
		{
			if(ZZFlag == 1)
			{
			  pCalTime[1].total_t = pTime->ZZ_t;
			  GetSysClock(&(pCalTime[1].endT), SYSCLOCK);
			  histtu2file_write((BYTE*)&pCalTime[1], 0, SYSEV_FLAG_OL);
			  ZZFlag = 0;
			}
		    pTime->ZZ_t = 0;
			pData->yx &= ~PB_YX_ZZ;
		}
     }
}

void PbUOff(void)
{
     int i,j;
	 VPbCfg *pCfg;
	 VPbData *pData;
	 VPbTime *pTime;
     for (i=0; i<pbfdnum; i++)
     {
         pData = g_pbYc + i;
		 pTime = g_pbTime + i;
		 pCfg  = g_pbYcCfg + i;

		 for (j=0; j<3; j++)
		 {
		    if (pCfg->u_index[j] == -1)
				continue;
			if ((pData->u[j]<g_pbSet[i].u_off.u_min)&&
			    (pData->i[j]<g_pbSet[i].u_off.i_min))
			{
			   pTime->u_off_t[j]++;
			   if (pTime->u_off_t[j] > g_pbSet[i].u_off.t)
			   	 pData->yx |= ((unsigned int)PB_YX_A_OFF<<j);
			}
			else if (pData->u[j]>g_pbSet[i].u_off.u_min)
			{
			    pTime->u_off_t[j] = 0;
				pData->yx &= ~((unsigned int)PB_YX_A_OFF<<j);
			}
				
		 }
     }
}

void PbULow(void)
{
	int i,j;
	VPbCfg *pCfg;
	VPbData *pData;
	VPbTime *pTime;
	for (i=0; i<pbfdnum; i++)
    {
        pData = g_pbYc + i;
		pTime = g_pbTime + i;
		pCfg  = g_pbYcCfg + i;

		for (j=0; j<3; j++)
		{
			if (pCfg->u_index[j] == -1)
				continue;
			if (pData->u[j]<g_pbSet[i].u_low.u_min) 
			#if TTU_TEST
			    if(pData->i[j]>g_pbSet[i].u_low.i_min) 
			#endif
		    {
			   pTime->u_low_t[j]++;
			   if (pTime->u_low_t[j] > g_pbSet[i].u_low.t)
			   	 pData->yx |= ((unsigned int)PB_YX_A_LOW<<j);
		    }
			else if (pData->u[j]>g_pbSet[i].u_low.u_min)
			{
			    pTime->u_low_t[j] = 0;
				pData->yx &= ~((unsigned int)PB_YX_A_LOW<<j);
			}
				
		}
	}
}

void PbBelowU(void)
{//低电压，电压越下限
	int i,j;
	VPbCfg *pCfg;
	VPbData *pData;
	VPbTime *pTime;
	VRunParaCfg *prunparacfg;
	 prunparacfg = &g_Sys.MyCfg.RunParaCfg;
    for (i=0; i<pbfdnum; i++)
    {
        pData = g_pbYc + i;
		pTime = g_pbTime + i;
		pCfg  = g_pbYcCfg + i;

		for (j=0; j<3; j++)
		{
			if (pCfg->u_index[j] == -1)
				continue;
			if ((pData->u[j]*10)<prunparacfg->dwDyVal) //100*10<1000?
		    {
			   pTime->UYXX_t[j]++;
			   if ((pTime->UYXX_t[j]/60) >= prunparacfg->wDyT)
			   	 pData->yx |= ((unsigned int)PB_YX_A_Below<<j);
		    }
			else
			{
			    pTime->UYXX_t[j] = 0;
				pData->yx &= ~((unsigned int)PB_YX_A_Below<<j);
			}
				
		 }
	}
}

void PbOverU(void)
{//过电压，电压越上限
	int i,j;
	VPbCfg *pCfg;
	VPbData *pData;
	VPbTime *pTime;
	VRunParaCfg *prunparacfg;
	 prunparacfg = &g_Sys.MyCfg.RunParaCfg;
    for (i=0; i<pbfdnum; i++)
    {
    	pData = g_pbYc + i;
		pTime = g_pbTime + i;
		pCfg  = g_pbYcCfg + i;

		for (j=0; j<3; j++)
		{
			if (pCfg->u_index[j] == -1)
				continue;
			if ((pData->u[j]*10)>prunparacfg->dwGyVal) 
		    {
			    pTime->UYSX_t[j]++;
			   if ((pTime->UYSX_t[j]/60) >= prunparacfg->wGyT)
			   	 pData->yx |= ((unsigned int)PB_YX_A_Over<<j);
		    }
			else
			{
			    pTime->UYSX_t[j] = 0;
				pData->yx &= ~((unsigned int)PB_YX_A_Over<<j);
			}
				
		 }
	}
}

void PbPowOff(void)
{
     int i,j;
	 VPbCfg *pCfg;
	 VPbData *pData;
	 VPbTime *pTime;
	 BOOL powoff;

     for (i=0; i<pbfdnum; i++)
     {
         pData = g_pbYc + i;
		 pTime = g_pbTime + i;
		 pCfg  = g_pbYcCfg + i;

         powoff = TRUE;
		 for (j=0; j<3; j++)
		 {
		    if (pCfg->u_index[j] == -1)
				continue;
			if ((pData->u[j]>g_pbSet[i].pow_off.u_min)||
			    (pData->i[j]>g_pbSet[i].pow_off.i_min))
			{
			  powoff = FALSE;
			  break;
			}				
		 }
		 if (powoff)
		 {
            pTime->pow_off_t++;
			if (pTime->pow_off_t > g_pbSet[i].pow_off.t)
			{
				pData->yx |= PB_YX_POW_OFF;
				PowFlag = 1;
				extNvRamSet(NVRAM_PowerOff, &PowFlag, 1);
			}
				
		 }
		 else
		 {
		    pTime->pow_off_t = 0;
			pData->yx &= ~PB_YX_POW_OFF;
		 }
  
     }

}
void PbPowOn(void)
{
     int i;
	 VPbData *pData;
#if 1
     for (i=0; i<pbfdnum; i++)
     {
         pData = g_pbYc + i;

			if (((pData->u[0]>g_pbSet[i].pow_off.u_min)||(pData->u[0]>g_pbSet[i].pow_off.u_min)
				||(pData->u[0]>g_pbSet[i].pow_off.u_min))&&PowFlag)
			{
			   	 pData->yx |= PB_YX_POW_ON;
				 PowFlag = 0;
				extNvRamSet(NVRAM_PowerOff, &PowFlag, 1);
			}
			else
			{
				pData->yx &= ~PB_YX_POW_ON;
			}
				
		 }
#endif
}
void PbYx(void)
{
    int no,value;
	unsigned int var, mask;
    int i,j;
	struct VDBCOS cos;
	struct VDBSOE soe;
	
	for (i=0; i<pbfdnum; i++)
	{
	    var = g_pbYc[i].yx ^ g_pbYc[i].yx_bak;
        mask = 1;
		for (j=0; j<PB_YX_NUM; j++)
		{
		    if (var & mask)
		    {
				VSysClock pTime;
		        no = g_pbYcCfg[i].yx_start + j;
				if (g_pbYc[i].yx & mask) value = 0x81;
				else value = 0x01;

				cos.wNo = (WORD)no;
				cos.byValue = (BYTE)value;

				WriteSCOS(g_Sys.wIOEqpID, 1, &cos);

				soe.wNo = (WORD)no;
				soe.byValue = (BYTE)value;
				GetSysClock(&(soe.Time), CALCLOCK);
				WriteSSOE(g_Sys.wIOEqpID, 1, &soe);
		    }
			mask <<= 1;
		}

		g_pbYc[i].yx_bak = g_pbYc[i].yx;
		
	}
}

void PbPerPassU(void)
{
     int i,j;
	 VPbData *pData = 0;
	 VPbTime *pTime = 0;
	 
     GetSysClock(&(g_pbUPass.cur), SYSCLOCK);
	 if (g_pbUPass.cur.byDay != g_pbUPass.last.byDay)
	 {
	     g_pbUPass.total_t = 0;
		 for (i=0; i<pbfdnum; i++)
		 {
			pTime = g_pbTime + i;
			pTime->pass_t[0] = pTime->pass_t[1] = pTime->pass_t[2] =  0;
		 }
		 extNvRamSet((DWORD)(NVRAM_MonPassU + i*12), (BYTE*)pTime->pass_t_m, 12);
		 extNvRamSet((DWORD)(NVRAM_PowerOff+4), (BYTE*)&g_pbUPass.total_t_m, 4);
	 }
	 if (g_pbUPass.cur.byMonth != g_pbUPass.last.byMonth)
	 {
	     g_pbUPass.total_t_m = 0;
		 for (i=0; i<pbfdnum; i++)
		 {
			pTime = g_pbTime + i;
			pTime->pass_t_m[0] = pTime->pass_t_m[1] = pTime->pass_t_m[2] = 0;
		 }
		 extNvRamSet((DWORD)(NVRAM_PowerOff+1), (BYTE*)&g_pbUPass.cur.byMonth, 1);
		 extNvRamSet((DWORD)(NVRAM_PowerOff+4), (BYTE*)&g_pbUPass.total_t_m, 4);//清零
	 }
	 g_pbUPass.total_t++;
	 g_pbUPass.total_t_m++;

	 for (i=0; i<pbfdnum; i++)
	 {
		pData = g_pbYc + i;
		pTime = g_pbTime + i;
		for (j=0; j<3; j++)
		{
			if ((pData->u[j] <  g_pbSet[i].u_pass_max) && (pData->u[j] > g_pbSet[i].u_pass_min))
			{
				pTime->pass_t[j]++;
				pTime->pass_t_m[j]++;
			}
			if (g_pbUPass.total_t != 0)
				pData->perPass_U[j] = (long)(pTime->pass_t[j]*100/g_pbUPass.total_t);
			if (g_pbUPass.total_t_m != 0)
				pData->perPass_U_m[j] = (long)(pTime->pass_t_m[j]*100/g_pbUPass.total_t_m);
		}
	 }	 
}
void PbPassUTime(void)
{
     int i,j;
	 VPbData *pData = 0;
	 VPbTime *pTime = 0;
	 
     GetSysClock(&(g_pbUPass.cur), SYSCLOCK);
	 if (g_pbUPass.cur.byDay != g_pbUPass.last.byDay)
	 {
		 for (i=0; i<pbfdnum; i++)
		 {
			pTime = g_pbTime + i;
			pTime->u_over_t[0] = pTime->u_over_t[1] = pTime->u_over_t[2] =  0;
			pTime->u_below_t[0] = pTime->u_below_t[1] = pTime->u_below_t[2] = 0;
		 }
		 extNvRamSet((DWORD)(NVRAM_MonOverU + i*12), (BYTE*)pTime->u_over_t_m, 12);
		 extNvRamSet((DWORD)(NVRAM_MonBelowU + i*12), (BYTE*)pTime->u_below_t_m, 12);
	 }
	 if (g_pbUPass.cur.byMonth != g_pbUPass.last.byMonth)
	 {
	     for (i=0; i<pbfdnum; i++)
		 {
			pTime = g_pbTime + i;
			pTime->u_over_t_m[0] = pTime->u_over_t_m[1] = pTime->u_over_t_m[2] =  0;
			pTime->u_below_t_m[0] = pTime->u_below_t_m[1] = pTime->u_below_t_m[2] =  0;
		 }
	 }

	 for (i=0; i<pbfdnum; i++)
	 {
		pData = g_pbYc + i;
		pTime = g_pbTime + i;
		for (j=0; j<3; j++)
		{
			if (pData->yx & ((unsigned int)PB_YX_A_Over<<j))
			{
				pTime->u_over_t[j]++;
				pTime->u_over_t_m[j]++;
			}
			else if (pData->yx & ((unsigned int)PB_YX_A_Below<<j))
			{
			    pTime->u_below_t[j]++;
				pTime->u_below_t_m[j]++;
			}
		}
	 }	 
}

void PbLoadRate(void)
{
     int i;
	 VPbCfg *pCfg;
	 VPbData *pData;
	 short CT;
	 float T;
	 VRunParaCfg *prunparacfg;
	 prunparacfg = &g_Sys.MyCfg.RunParaCfg;
	
     for (i=0; i<pbfdnum; i++)
     {
         pData = g_pbYc + i;
		 pCfg  = g_pbYcCfg + i;
		 CT = (short)(prunparacfg->LineCfg[i].wCt1/prunparacfg->LineCfg[i].wCt2);
		 T = (float)g_pbSet[i].otherSet.capacity/(float)CT;
		 
		 if (pCfg->s_index == -1)
			 continue;
	     pData->LoadRate = (short)(((WORD)pData->s)/T)/10;;//扩大了1000倍
     }
}

void pbAvgVal(void)
{
     int i,j,k,idx;
	 long sum;
	 VPbData *pData;
	 VPbAvg *pAvg;

	 
	 for (i=0; i<pbfdnum; i++)
	 {
	 	pData = g_pbYc + i;
		pAvg  = g_pbAvg + i;

		idx = pAvg->index;
		if (idx == 15) 
	 	{
	 		idx = 0;
			pAvg->index = 0;
	 	}
	 	if (pAvg->num < 15) pAvg->num++;
	     
		for (j=0; j<3; j++)
		{
		    pAvg->u15[j][idx] = pData->u[j];
			sum = 0;
			for (k=0; k<pAvg->num; k++)
				sum += pAvg->u15[j][k];
			pAvg->uavg[j] = (short)(sum / pAvg->num);
		} 
		pAvg->index++;
	 }
	
	 
}


int PbDefaultPara(void)
{
	char fname[MAXFILENAME];
	VPbPara *pPara;
	struct VFileHead *head;
	int i;
	head  = (struct VFileHead *)g_pParafile;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;

	pPara = (VPbPara*)((BYTE*)g_pParafile + sizeof(struct VFileHead));

	pPara->u_pass_max = 110;
	pPara->u_pass_min = 90;
	pPara->u_unblance = 25;
	pPara->i_unblance = 25;
	pPara->u_low = 60;
	pPara->u_low_t = 5;
	pPara->u_off = 20;
	pPara->u_off_t = 5;
	pPara->i_u_off = 0.1;
	pPara->pow_off = 20;
	pPara->pow_off_t = 5;
	pPara->u_unblance_t = 5;
	pPara->i_unblance_t = 5;
	pPara->capacity = 10;//配变容量
	pPara->u_unblance_set = 0.1; 
	pPara->i_unblance_set = 0.1;
	pPara->KGTime= 15;
	pPara->capacity_DL = 10;
	for(i=0;i<48;i++)
	{
		pPara->EnergyT[i] = 0;
	}

	head->nLength = sizeof(struct VFileHead) + sizeof(VPbPara);
	strcpy(fname, "PbPara.cfg");
	if(WriteParaFile(fname, (struct VFileHead *)g_pParafile) == ERROR)
		return ERROR;

	return OK;  
}

void PbSetPara(VPbPara *pSet)
{
    int i,j;
	VFdCfg *pCfg;
    for (i=0; i<fdNum; i++)
    {
        pCfg = pFdCfg + i; 
		g_pbSet[i].u_pass_max = pCfg->Un*pSet->u_pass_max;
	    g_pbSet[i].u_pass_min = pCfg->Un*pSet->u_pass_min;
		g_pbSet[i].u_unblance.unblance_up = pSet->u_unblance*10;
		g_pbSet[i].u_unblance.unblance_rec = pSet->u_unblance*9;
		g_pbSet[i].u_unblance.t = pSet->u_unblance_t;
		g_pbSet[i].i_unblance.unblance_up = pSet->i_unblance*10;
		g_pbSet[i].i_unblance.unblance_rec = pSet->i_unblance*9;
		g_pbSet[i].i_unblance.t = pSet->i_unblance_t;
		g_pbSet[i].u_low.u_min = pSet->u_low*100;
		g_pbSet[i].u_low.u_rec = pSet->u_low*11*10;
		g_pbSet[i].u_low.i_min = pSet->i_u_off*1000;
		g_pbSet[i].u_low.t = pSet->u_low_t;
		g_pbSet[i].u_off.u_min = pSet->u_off*100;
		g_pbSet[i].u_off.u_rec = pSet->u_off*11*10;
		g_pbSet[i].u_off.i_min = pSet->i_u_off*1000;
		g_pbSet[i].u_off.t = pSet->u_off_t;
		g_pbSet[i].pow_off.u_min = pSet->pow_off*100;
		g_pbSet[i].pow_off.u_rec = pSet->pow_off*11*10;
		g_pbSet[i].pow_off.i_min = pSet->i_u_off*1000;
		g_pbSet[i].pow_off.t = pSet->pow_off_t;
		
		g_pbSet[i].otherSet.capacity = pSet->capacity;
		g_pbSet[i].otherSet.i_unblance_set = pSet->i_unblance_set;
		g_pbSet[i].otherSet.u_unblance_set = pSet->u_unblance_set;
		g_pbSet[i].otherSet.KGTime= pSet->KGTime;
		g_pbSet[i].otherSet.capacity_DL = pSet->capacity_DL;
		for(j=0;j<48;j++)
		{
		  g_pbSet[i].otherSet.EnergyT[j] = pSet->EnergyT[j];
		  extEnergyT[j] = pSet->EnergyT[j];
		}
    }
}

int PbParaRead(void)
{
	char fname[MAXFILENAME];

	strcpy(fname, "PbPara.cfg");
	if (ReadParaFile(fname, (BYTE*)g_pParafile, MAXFILELEN) == ERROR)
		return ERROR;
	return OK;
}
void PbInit(void)
{
    int i,j;
	BYTE mon;
	VPbPara *pbPara = (VPbPara*)(g_pParafile+1);
	
	pbfdnum =  g_Sys.MyCfg.wFDNum;
	pbycnum =  g_Sys.MyCfg.wYCNum-g_Sys.MyCfg.wYCNumPublic;
	pPbCfg  = g_Sys.MyCfg.pYc;;
			

    if (PbParaRead() == ERROR)
	{
		PbDefaultPara();
		PbParaRead();
    }
	memcpy((BYTE*)&ParaFile,(BYTE*)pbPara,sizeof(VPbPara));
	
	PbSetPara(pbPara);
	extNvRamGet(NVRAM_PowerOff, &PowFlag, 1);
	extNvRamGet(NVRAM_PowerOff+1, &mon, 1);
	memset(g_pbYc, 0, sizeof(g_pbYc));
	memset(g_pbTime, 0, sizeof(g_pbTime));
	memset(g_pbAvg, 0, sizeof(g_pbAvg));
	g_pbUPass.total_t = 0;
	g_pbUPass.total_t_m = 0;
	GetSysClock(&(g_pbUPass.last), SYSCLOCK);
	GetSysClock(&(g_pbUPass.cur),  SYSCLOCK);

	if (mon == g_pbUPass.cur.byMonth)
		extNvRamGet(NVRAM_PowerOff+4, (BYTE*)&g_pbUPass.total_t_m, 4);


	for (i=0; i<pbfdnum; i++)
	{
	    for (j=0; j<3; j++)
	    {
	        g_pbYcCfg[i].u_index[j] = -1;
			g_pbYcCfg[i].i_index[j] = -1;
			g_pbYcCfg[i].uph_index[j] = -1;
			g_pbYcCfg[i].iph_index[j] = -1;
	    }
		g_pbYcCfg[i].s_index = -1;
		g_pbYcCfg[i].p_index = -1;
		g_pbYcCfg[i].q_index = -1;

		g_pbYcCfg[i].yx_start = (BYTE)(g_Sys.MyCfg.SYXIoNo[2].wIoNo_Low + i*PB_YX_TOTAL+PB_YX_START);
		for (j=0; j<pbycnum; j++)
		{
		    if (pPbCfg[j].type == YC_TYPE_UPhZ)
		    {
		       if (pPbCfg[j].fdNo == i)
			   {
			       g_pbYcCfg[i].u_index[0] = (short)pPbCfg[j].arg1;
				   if (pPbCfg[j].arg2 == 3)
				   {
				      g_pbYcCfg[i].u_index[1] = (short)pPbCfg[j].arg1+1;
					  g_pbYcCfg[i].u_index[2] = (short)pPbCfg[j].arg1+2;
				   }
				   else
				   {
				      g_pbYcCfg[i].u_index[2] = (short)pPbCfg[j].arg1+1;
				   }
		       }
		    }
			if (pPbCfg[j].type == YC_TYPE_IPhZ)
			{
			    if (pPbCfg[j].fdNo == i)
			    {
			       g_pbYcCfg[i].i_index[0] = (short)pPbCfg[j].arg1;
				   g_pbYcCfg[i].i_index[1] = (short)pPbCfg[j].arg1+1;
				   g_pbYcCfg[i].i_index[2] = (short)pPbCfg[j].arg1+2;
			    }
			}
			if (pPbCfg[j].type == YC_TYPE_Uunb)
			   {
			    g_pbYcCfg[i].uph_index[0] = (short)pPbCfg[j].arg1;
				g_pbYcCfg[i].uph_index[1] = (short)pPbCfg[j].arg1+1;
				g_pbYcCfg[i].uph_index[2] = (short)pPbCfg[j].arg1+2;
		       }
			if (pPbCfg[j].type == YC_TYPE_Iunb)
			{
			    g_pbYcCfg[i].iph_index[0] = (short)pPbCfg[j].arg1;
				g_pbYcCfg[i].iph_index[1] = (short)pPbCfg[j].arg1+1;
				g_pbYcCfg[i].iph_index[2] = (short)pPbCfg[j].arg1+2;
			}
			if (pPbCfg[j].type == YC_TYPE_P)
		    {
		       if (pPbCfg[j].fdNo == i)
			   {
			       g_pbYcCfg[i].p_index = (short)j;
		       }
		    }
			if (pPbCfg[j].type == YC_TYPE_Q)
		    {
		       if (pPbCfg[j].fdNo == i)
			   {
			       g_pbYcCfg[i].q_index = (short)j;
		       }
		    }
			if (pPbCfg[j].type == YC_TYPE_S)
		    {
		       if (pPbCfg[j].fdNo == i)
			   {
			       g_pbYcCfg[i].s_index = (short)j;
		       }
		    }
		}
        if (mon == g_pbUPass.cur.byMonth)
		{
			extNvRamGet((DWORD)(NVRAM_MonOverU+12*i), (BYTE*)g_pbTime[i].u_over_t_m, 12);
			extNvRamGet((DWORD)(NVRAM_MonBelowU+12*i), (BYTE*)g_pbTime[i].u_below_t_m, 12);
			extNvRamGet((DWORD)(NVRAM_MonPassU+12*i), (BYTE*)g_pbTime[i].pass_t_m, 12);		
        }

	}
}

void pb(int thid)
{
   DWORD event;
   DWORD count = 0;

   tmEvEvery(thid, SECTOTICK(1), EV_TM1);   //1s
   
   for (; ;)
   {
		evReceive(thid, EV_TM1, &event);

		if (event & EV_TM1)
		{
			PbYcRead();
			PbUnBlance();
			PbLoadRate();
			PbUOff();
			PbULow();
			PbPowOff();
			PbOverLoad();
			PbHeavyLoad(); 
			PbBelowU();
			PbOverU();
			PbYx();
			count++;
			if ((count%60)==0)
			{
				PbPowOn();
				PbPerPassU();
				PbPassUTime();
				pbAvgVal();
			}
		}
   }
}

// ERROR OK
int WritePbSet(BYTE* pdata,WORD len)
{
	char fname[MAXFILENAME];
	VPbPara *pPara;
	struct VFileHead *head;

	head  = (struct VFileHead *)g_pParafile;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	
	pPara = (VPbPara*)((BYTE*)g_pParafile + sizeof(struct VFileHead));
	memcpy((BYTE*)pPara,pdata,len);
	memcpy((BYTE*)&ParaFile,(BYTE*)pPara,len);
	head->nLength = sizeof(struct VFileHead) + sizeof(VPbPara);
	strcpy(fname, "PbPara.cfg");
	if(WriteParaFile(fname, (struct VFileHead *)g_pParafile) == ERROR)
		return ERROR;
	PbSetPara(pPara);
	return OK;  
}
// Num;
int ReadPbSet(BYTE* pdata,WORD len)
{
	VPbPara *pbPara = (VPbPara*)(g_pParafile+1);
	if (PbParaRead() == ERROR)
		return 0;
	memcpy(pdata,pbPara,len);
	memcpy((BYTE*)&ParaFile,(BYTE*)pbPara,len);
	return sizeof(VPbPara);
}

int ReadPbRunPara(WORD parano,char* pbuf)
{
	struct VParaInfo *fParaInfo;
	float fvalue;
	WORD svalue,no;
	BYTE bvalue;
	fParaInfo = (struct VParaInfo *)pbuf;
	svalue = fvalue = bvalue = 0;
	
	if((parano < PRPARA_TTU_ADDR) || (parano >(PRPARA_TTU_ADDR+PRP_SHORTCIR_CAP)))
		return ERROR;
	
	if((parano > (PRPARA_TTU_ADDR+PRP_SWTICH_TIME_CYC)) && (parano <(PRPARA_TTU_ADDR+PRP_SHORTCIR_CAP)))
		return ERROR;
	
	no = parano - PRPARA_TTU_ADDR;
	switch(no)
	{
		case PRP_CAPACITY_TTU:
			fParaInfo->type = USHORT_TYPE;
			fParaInfo->len = USHORT_LEN;
			svalue = g_pbSet[0].otherSet.capacity;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
		case PRP_UNBALANCE_I:
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			fvalue = g_pbSet[0].otherSet.i_unblance_set;
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case PRP_UNBALANCE_V:
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			fvalue = g_pbSet[0].otherSet.u_unblance_set;
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
			case PRP_RATE_TIME_1:
			case PRP_RATE_TIME_2:
			case PRP_RATE_TIME_3:
			case PRP_RATE_TIME_4:
			case PRP_RATE_TIME_5:
			case PRP_RATE_TIME_6:
			case PRP_RATE_TIME_7:
			case PRP_RATE_TIME_8:
			case PRP_RATE_TIME_9:
			case PRP_RATE_TIME_10:
			case PRP_RATE_TIME_11:
			case PRP_RATE_TIME_12:
			case PRP_RATE_TIME_13:
			case PRP_RATE_TIME_14:
			case PRP_RATE_TIME_15:
			case PRP_RATE_TIME_16:
			case PRP_RATE_TIME_17:
			case PRP_RATE_TIME_18:
			case PRP_RATE_TIME_19:
			case PRP_RATE_TIME_20:
			case PRP_RATE_TIME_21:
			case PRP_RATE_TIME_22:
			case PRP_RATE_TIME_23:
			case PRP_RATE_TIME_24:
			case PRP_RATE_TIME_25:
			case PRP_RATE_TIME_26:
			case PRP_RATE_TIME_27:
			case PRP_RATE_TIME_28:
			case PRP_RATE_TIME_29:
			case PRP_RATE_TIME_30:
			case PRP_RATE_TIME_31:
			case PRP_RATE_TIME_32:
			case PRP_RATE_TIME_33:
			case PRP_RATE_TIME_34:
			case PRP_RATE_TIME_35:
			case PRP_RATE_TIME_36:
			case PRP_RATE_TIME_37:
			case PRP_RATE_TIME_38:
			case PRP_RATE_TIME_39:
			case PRP_RATE_TIME_40:
			case PRP_RATE_TIME_41:
			case PRP_RATE_TIME_42:
			case PRP_RATE_TIME_43:
			case PRP_RATE_TIME_44:
			case PRP_RATE_TIME_45:
			case PRP_RATE_TIME_46:
			case PRP_RATE_TIME_47:
			case PRP_RATE_TIME_48:
				fParaInfo->type = UTINY_TYPE;
				fParaInfo->len = UTINY_LEN;
				bvalue = g_pbSet[0].otherSet.EnergyT[no-3];
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;

		case PRP_SWTICH_TIME_CYC:
			fParaInfo->type = USHORT_TYPE;
			fParaInfo->len = USHORT_LEN;
			svalue = g_pbSet[0].otherSet.KGTime;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
		case PRP_SHORTCIR_CAP:
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			fvalue = g_pbSet[0].otherSet.capacity_DL;
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		default:
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
	}
	return OK;
}
int WritePbRunParaYZ(WORD parano,char* pbuf)
{

	struct VParaInfo *fParaInfo;
	float fvalue;
	WORD svalue;
	BYTE bvalue;
	WORD no;
	
	fParaInfo = (struct VParaInfo*)pbuf;
	svalue = fvalue = bvalue = 0;
	
	if((parano < PRPARA_TTU_ADDR) || (parano >(PRPARA_TTU_ADDR+PRP_SHORTCIR_CAP)))
		 return 1;

	no = parano - PRPARA_TTU_ADDR;
	switch(no)
	{
		case PRP_UNBALANCE_I:
		case PRP_UNBALANCE_V:

			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			break;
			
		case PRP_SHORTCIR_CAP:
			
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((fvalue < 0) || (fvalue > 3000))
				return 3;
			break;
		case PRP_CAPACITY_TTU:
		case PRP_SWTICH_TIME_CYC:
			if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue < 1) || (svalue > 3000))
				return 3;
			break;
		case PRP_RATE_TIME_1:
		case PRP_RATE_TIME_2:
		case PRP_RATE_TIME_3:
		case PRP_RATE_TIME_4:
		case PRP_RATE_TIME_5:
		case PRP_RATE_TIME_6:
		case PRP_RATE_TIME_7:
		case PRP_RATE_TIME_8:
		case PRP_RATE_TIME_9:
		case PRP_RATE_TIME_10:
		case PRP_RATE_TIME_11:
		case PRP_RATE_TIME_12:
		case PRP_RATE_TIME_13:
		case PRP_RATE_TIME_14:
		case PRP_RATE_TIME_15:
		case PRP_RATE_TIME_16:
		case PRP_RATE_TIME_17:
		case PRP_RATE_TIME_18:
		case PRP_RATE_TIME_19:
		case PRP_RATE_TIME_20:
		case PRP_RATE_TIME_21:
		case PRP_RATE_TIME_22:
		case PRP_RATE_TIME_23:
		case PRP_RATE_TIME_24:
		case PRP_RATE_TIME_25:
		case PRP_RATE_TIME_26:
		case PRP_RATE_TIME_27:
		case PRP_RATE_TIME_28:
		case PRP_RATE_TIME_29:
		case PRP_RATE_TIME_30:
		case PRP_RATE_TIME_31:
		case PRP_RATE_TIME_32:
		case PRP_RATE_TIME_33:
		case PRP_RATE_TIME_34:
		case PRP_RATE_TIME_35:
		case PRP_RATE_TIME_36:
		case PRP_RATE_TIME_37:
		case PRP_RATE_TIME_38:
		case PRP_RATE_TIME_39:
		case PRP_RATE_TIME_40:
		case PRP_RATE_TIME_41:
		case PRP_RATE_TIME_42:
		case PRP_RATE_TIME_43:
		case PRP_RATE_TIME_44:
		case PRP_RATE_TIME_45:
		case PRP_RATE_TIME_46:
		case PRP_RATE_TIME_47:
		case PRP_RATE_TIME_48:
												
			if((fParaInfo->type != UTINY_TYPE) || (fParaInfo->len != UTINY_LEN))
				return 2;
			memcpy(&bvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if(svalue > 3)
				return 3;
			break;
		
		default:
			break;
	}
	return 0;
}
int WritePbRunPara(WORD parano,char* pbuf,WORD ParaFlag)
{
	struct VParaInfo *fParaInfo;
	float fvalue;
	WORD svalue;
	BYTE bvalue;
	WORD no;
	
	fParaInfo = (struct VParaInfo*)pbuf;
	svalue =0;
	if((parano < PRPARA_TTU_ADDR) || (parano >(PRPARA_TTU_ADDR+PRP_SHORTCIR_CAP)))
		return 1;
	
	if((parano > (PRPARA_TTU_ADDR+PRP_SWTICH_TIME_CYC)) && (parano <(PRPARA_TTU_ADDR+PRP_SHORTCIR_CAP)))
		return 1;
	
	
		no = parano - PRPARA_TTU_ADDR;
		switch(no)
		{
			case PRP_CAPACITY_TTU:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue < 1) || (svalue > 3000))
					return 3;
			
				g_pbSet[0].otherSet.capacity = svalue;
				ParaFile.capacity = svalue;
				break;
			case PRP_UNBALANCE_I:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			
				g_pbSet[0].otherSet.i_unblance_set = fvalue;
				ParaFile.i_unblance_set = fvalue;
				if(ParaFlag == 0)
					ParaFile.i_unblance = fvalue*100;
				else
					ParaFile.i_unblance = fvalue;
				break;
			case PRP_UNBALANCE_V:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			
				g_pbSet[0].otherSet.u_unblance_set = fvalue;
				ParaFile.u_unblance_set = fvalue;
				if(ParaFlag == 0)
					ParaFile.u_unblance = fvalue*100;
				else
					ParaFile.u_unblance = fvalue;
				break;
				
			case PRP_RATE_TIME_1:
			case PRP_RATE_TIME_2:
			case PRP_RATE_TIME_3:
			case PRP_RATE_TIME_4:
			case PRP_RATE_TIME_5:
			case PRP_RATE_TIME_6:
			case PRP_RATE_TIME_7:
			case PRP_RATE_TIME_8:
			case PRP_RATE_TIME_9:
			case PRP_RATE_TIME_10:
			case PRP_RATE_TIME_11:
			case PRP_RATE_TIME_12:
			case PRP_RATE_TIME_13:
			case PRP_RATE_TIME_14:
			case PRP_RATE_TIME_15:
			case PRP_RATE_TIME_16:
			case PRP_RATE_TIME_17:
			case PRP_RATE_TIME_18:
			case PRP_RATE_TIME_19:
			case PRP_RATE_TIME_20:
			case PRP_RATE_TIME_21:
			case PRP_RATE_TIME_22:
			case PRP_RATE_TIME_23:
			case PRP_RATE_TIME_24:
			case PRP_RATE_TIME_25:
			case PRP_RATE_TIME_26:
			case PRP_RATE_TIME_27:
			case PRP_RATE_TIME_28:
			case PRP_RATE_TIME_29:
			case PRP_RATE_TIME_30:
			case PRP_RATE_TIME_31:
			case PRP_RATE_TIME_32:
			case PRP_RATE_TIME_33:
			case PRP_RATE_TIME_34:
			case PRP_RATE_TIME_35:
			case PRP_RATE_TIME_36:
			case PRP_RATE_TIME_37:
			case PRP_RATE_TIME_38:
			case PRP_RATE_TIME_39:
			case PRP_RATE_TIME_40:
			case PRP_RATE_TIME_41:
			case PRP_RATE_TIME_42:
			case PRP_RATE_TIME_43:
			case PRP_RATE_TIME_44:
			case PRP_RATE_TIME_45:
			case PRP_RATE_TIME_46:
			case PRP_RATE_TIME_47:
			case PRP_RATE_TIME_48:
				if((fParaInfo->type != UTINY_TYPE) || (fParaInfo->len != UTINY_LEN))
						return 2;
					memcpy(&bvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					if((bvalue < 0) || (bvalue > 3))
						return 3;
				
					g_pbSet[0].otherSet.EnergyT[no-3]= bvalue;
					ParaFile.EnergyT[no-3] = bvalue;
					break;

			case PRP_SWTICH_TIME_CYC:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
						return 2;
					memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					if((svalue < 1) || (svalue > 3000))
						return 3;
				
					g_pbSet[0].otherSet.KGTime= svalue;
					ParaFile.KGTime = svalue;
					break;
			case PRP_SHORTCIR_CAP:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
						return 2;
					memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
					if((fvalue < 0) || (fvalue > 3000))
						return 3;
				
					g_pbSet[0].otherSet.capacity_DL = fvalue;
					ParaFile.capacity_DL = fvalue;
					break;
			default:
				break;
		}
		if(WritePbSet((BYTE*)&ParaFile,sizeof(VPbPara))== ERROR)
			return ERROR;
	return 0;
}

#endif
