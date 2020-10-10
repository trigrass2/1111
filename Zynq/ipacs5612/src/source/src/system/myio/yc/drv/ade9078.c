
#include "syscfg.h"
#ifdef  INCLUDE_ADE9078
#include "sys.h"
#include "ade9078.h"
extern VYcGain *pYcGain;

static STATUS fd = 0;
short  AD_RESULT[7][MaxSampPointNum];  // 6相128点AD采样值
float AWATT_EP[6] = {0};   //只需要总有功无功
float Ade9078_Dd[32] ={0};
Ade9078_DD_JFPG g_Ade9078_Dd_JFPG[8];

static BYTE bFlowDirHis = 0;
static BYTE bActPowerDirHis	= 0;

long UAde9078[4];
long IAde9078[4];
long PQAde9078[8];

extern BYTE extEnergyT[48];
extern short mP;
extern short mQ;

static int write_reg(STATUS addr, STATUS byte,const BYTE* data)
{
	STATUS ret,i;
	adc_reg_req_st adc;
	
	memset(&adc, 0, sizeof(adc_reg_req_st));
	adc.regaddr = (unsigned int)addr;
	adc.reglen = (unsigned int)byte;
	adc.regdata = (BYTE *)malloc(adc.reglen);
	if (adc.regdata == NULL)
	{
		DPRINT("malloc failed.\r\n");
		return 0;
	}
	memset(adc.regdata, 0, adc.reglen);

	for(i = 0;i < byte;i++)
	{
		adc.regdata[i] = data[byte-(1+i)];
	}
	
	ret = ioctl(fd, ADC_IO_WRITE, &adc);
	if (ret)
	{
		DPRINT("adc write failed.\n");
		return 0;
	}
	free(adc.regdata);
	return 1;
}

static int read_reg(STATUS addr, STATUS byte, BYTE* data)
{
	STATUS ret,i;
	adc_reg_req_st adc;

	memset(&adc, 0, sizeof(adc_reg_req_st));
	adc.regaddr = (unsigned int)addr;
	adc.reglen = (unsigned int)byte;
	adc.regdata = (BYTE *)malloc(adc.reglen);
	if (adc.regdata == NULL)
	{
		DPRINT("malloc failed.\r\n");
		return 0;
	}
	memset(adc.regdata, 0, adc.reglen);
	ret = ioctl(fd, ADC_IO_READ, &adc);
	if (ret)
	{
		DPRINT("adc read failed.\n");
		return 0;
	}

	for(i = 0; i < byte;i++)
	{
		data[i] =  adc.regdata[byte-(1+i)];
	}
		
	free(adc.regdata);
	return 1;
}

static int opendev(const BOOL *devpath)
{
	fd = open(devpath, O_RDWR);
	if (fd < 0)
	{
		DPRINT("open /dev/adc0 failed.\n");
		return -1;
	}
	return 0;
}

int Power_Ade9078(STATUS iType)
{
	BYTE data;
	BYTE regaddr;
	BYTE id = 0x22;

	regaddr = 0x06;
	I2CReceBuffer_BAddr(&data, id, regaddr, 1);
	data = data&0X7F;
	I2CSendBuffer_BAddr(&data, id, regaddr, 1);

	regaddr = 0x07;
	I2CReceBuffer_BAddr(&data, id, regaddr, 1);
	data = data&0XFE;
	I2CSendBuffer_BAddr(&data, id, regaddr, 1);
	
	if(iType == 1)
	{
		//上电
		regaddr = 0x03;
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data|0x01;
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
	}
	else if(iType == 2)
	{
		//下电
		regaddr = 0x03;
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data&0xFE;
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
	}
	else if(iType == 3)
	{
		//复位
		regaddr = 0x02;
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data&0x7F;
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
		usleep(100000);
		I2CReceBuffer_BAddr(&data, id, regaddr, 1);
		data = data|0x80;
		I2CSendBuffer_BAddr(&data, id, regaddr, 1);
		usleep(100000);
	}
	return 0;
}

int ADE9078_Init()
{
	STATUS ret;
	DWORD ID;
	STATUS addr[] = {0x490, 0x491, 0x494, 0x495, 0x420, 0x421, 0x480, 0x4B0};
	STATUS data[] = {0xc001, 0x3F, 0xF0D, 0xF0D, 0x100044, 0x100044, 0x1, 0x2021};

	Ade9078Dd_Init();
	
	if (!fd)
	{
		ret = opendev("/dev/adc0");
		if (ret == -1)
		{
			DPRINT("/dev/adc0 open failed.\n");
			return -1;
		}
	}

	//上电
	Power_Ade9078(1);
	//复位
	Power_Ade9078(3);
	
	read_reg(PART_ID, 4, (BYTE*)&ID);
	myprintf(SYS_ID, LOG_ATTR_WARN,"Read Chip Id: %08x ",ID);
	//初始化 
	write_reg(addr[0], 2, (unsigned char*)&data[0]);
	
	write_reg(addr[1], 2, (unsigned char*)&data[1]);
	
	write_reg(addr[2], 2, (unsigned char*)&data[2]);
	
	write_reg(addr[3], 2, (unsigned char*)&data[3]);
	
	write_reg(addr[4], 4, (unsigned char*)&data[4]);
	
	write_reg(addr[5], 4, (unsigned char*)&data[5]);
	
	write_reg(addr[6], 2, (unsigned char*)&data[6]);
	
	write_reg(addr[7], 2, (unsigned char*)&data[7]);

	ADE9078_SetConfig();
		
	return 0;
}

void ADE9078_Fini()
{
	STATUS ret;
	//下电
	ret = ioctl(fd, ADC_IO_DISABLE);
	if (ret)
	{
		DPRINT("ioctl disable failed.\n");
		return ;
	}
	if (fd)
	{
		close(fd);
	}
}

void ADE9078_Reset()
{
	STATUS ret;
	//下电
	ret = ioctl(fd, ADC_IO_DISABLE);
	if (ret)
	{
		DPRINT("ioctl disable failed.\n");
		return ;
	}
	sleep(1);

	//上电
	ret = ioctl(fd, ADC_IO_ENABLE);
	if (ret)
	{
		DPRINT("ioctl enable failed.\n");
		return ;
	}
	
}

int ADE9078_ReadU(STATUS iIdx, long *value)
{//100倍
	STATUS num = 0;

	if (iIdx < 0 || iIdx > 3)
	{
		*value = 0;
		return -1;
	}

	if (iIdx == 0)
	{
		read_reg(AVRMS, 4, (unsigned char*)&num);
	}
	else if (iIdx == 1)
	{
		read_reg(BVRMS, 4, (unsigned char*)&num);
	}
	else if (iIdx == 2)
	{
		read_reg(CVRMS, 4, (unsigned char*)&num);
	}
		
	*value = (long)(num*100.0/Kv);
	UAde9078[iIdx] = *value;
	return 0;
}

int ADE9078_ReadI(STATUS iIdx, long *value)
{//1000倍
	STATUS num = 0;

	if (iIdx < 0 || iIdx > 3)
	{
		*value = 0;
		return -1;
	}

	if (iIdx == 0)
	{
		read_reg(AIRMS, 4, (unsigned char*)&num);
	}
	else if (iIdx == 1)
	{
		read_reg(BIRMS, 4, (unsigned char*)&num);
	}
	else if (iIdx == 2)
	{
		read_reg(CIRMS, 4, (unsigned char*)&num);
	}
	else if (iIdx == 3)
	{
		read_reg(NIRMS, 4, (unsigned char*)&num);
	}

	*value = (long)(num*1000.0/Ki);
	IAde9078[iIdx] = *value;
	return 0;
}

int ADE9078_ReadP(STATUS iIdx, long *value)
{//10倍
	STATUS num = 0;

	if (iIdx < 0 || iIdx > 2)
	{
		*value = 0;
		return -1;
	}

	if (iIdx == 0)
	{
		read_reg(AWATT, 4, (unsigned char*)&num);
	}
	else if (iIdx == 1)
	{
		read_reg(BWATT, 4, (unsigned char*)&num);
	}
	else if(iIdx == 2)
	{
		read_reg(CWATT, 4, (unsigned char*)&num);
	}

	*value = (long)(num*10000.0/Kw);
	PQAde9078[iIdx] = *value;
	return 0;
}

int ADE9078_ReadQ(STATUS iIdx, long *value)
{
	STATUS num = 0;

	if (iIdx < 0 || iIdx > 2)
	{
		*value = 0;
		return -1;
	}

	if (iIdx == 0)
	{
		read_reg(AVAR, 4, (unsigned char*)&num);
	}
	else if (iIdx == 1)
	{
		read_reg(BVAR, 4, (unsigned char*)&num);
	}
	else if(iIdx == 2)
	{
		read_reg(CVAR, 4, (unsigned char*)&num);
	}

	*value = (long)(num*10000.0/Kw);
	PQAde9078[iIdx+4] = *value;
	return 0;
}

int ADE9078_ReadSx(STATUS iIdx, long *value)
{
	STATUS num = 0;

	if (iIdx < 0 || iIdx > 2)
	{
		*value = 0;
		return -1;
	}

	if (iIdx == 0)
	{
		read_reg(AVA, 4, (unsigned char*)&num);
	}
	else if (iIdx == 1)
	{
		read_reg(BVA, 4, (unsigned char*)&num);
	}
	else if (iIdx == 2)
	{
		read_reg(CVA, 4, (unsigned char*)&num);
	}

	*value = (long)(num*10000.0/Kw);

	return 0;
}

int ADE9078_ReadS(long *value)
{
	long num = 0;
	
	ADE9078_ReadSx(0,&num);
	*value = num;
	ADE9078_ReadSx(1,&num);
	*value += num;
	ADE9078_ReadSx(2,&num);
	*value += num;

	return 0;
}

int ADE9078_ReadPF(STATUS iIdx, float *value)
{
	STATUS num = 0;
	int k = 0x7ffffff;

	if (iIdx < 0 || iIdx > 2)
	{
		*value = 0;
		return -1;
	}

	if (iIdx == 0)
	{
		read_reg(APF, 4, (unsigned char*)&num);
	}
	else if (iIdx == 1)
	{
		read_reg(BPF, 4, (unsigned char*)&num);
	}
	else if (iIdx == 2)
	{
		read_reg(CPF, 4, (unsigned char*)&num);
	}

	*value = (float)(num*1.0/k);

	return 0;
}

int ADE9078_Readf(STATUS iIdx, long *value)
{
	STATUS num = 0;
	if (iIdx < 0 || iIdx > 3)
	{
		*value = 0;
		return -1;
	}
	
	if (iIdx == 0)
	{
		read_reg(APERIOD, 4, (unsigned char*)&num);
	}
	else if (iIdx == 1)
	{
		read_reg(BPERIOD, 4, (unsigned char*)&num);
	}
	else if (iIdx == 2)
	{
		read_reg(CPERIOD, 4, (unsigned char*)&num);
	}
	else if (iIdx == 3)
	{
		read_reg(COM_PERIOD, 4, (unsigned char*)&num);
	}
	
	*value = (long)(((4000*(1<<16))/(num*1.0 + 1)) *100);

	return 0;
	
}

int ADE9078_Readw(STATUS iIdx, long *value)
{
	if (iIdx < 1 || iIdx > 3)
	{
		return -1;
	}

	*value = 0;
	
	if (iIdx == 1)
	{
		read_reg(AWATTHR_HI, 4, (BYTE*)value);
	}
	else if (iIdx == 2)
	{
		read_reg(BWATTHR_HI, 4, (BYTE*)value);
	}
	else
	{
		read_reg(CWATTHR_HI, 4, (BYTE*)value);
	}
	
	return 0;
}

int ADE9078_ReadV(STATUS iIdx, long *value)
{
	*value = 0;
	if (iIdx < 1 || iIdx > 3)
	{
		return -1;
	}

	if (iIdx == 1)
	{
		read_reg(AVARHR_HI, 4, (BYTE*)value);
	}
	else if (iIdx == 2)
	{
		read_reg(BVARHR_HI, 4, (BYTE*)value);
	}
	else
	{
		read_reg(CVARHR_HI, 4, (BYTE*)value);
	}

	return 0;
}

int ADE9078_ReadAV(STATUS iIdx, long *value)
{
	if (iIdx < 1 || iIdx > 3)
	{
		return -1;
	}

	if (iIdx == 1)
	{
		read_reg(AVAHR_HI, 4, (BYTE*)value);
	}
	else if (iIdx == 2)
	{
		read_reg(BVAHR_HI, 4, (BYTE*)value);
	}
	else
	{
		read_reg(CVAHR_HI, 4, (BYTE*)value);
	}
	
	return 0;
}

int ADE9078_ReadPMPQ(STATUS iType, long *value)
{
	STATUS num;

	if (iType < 1 || iType > 4)
	{
		return -1;
	}

	if (iType == 1)
	{
		//正总有功功率
		read_reg(PWATT_ACC, 4, (unsigned char*)&num);
	}
	else if (iType == 2)
	{
		//负总有功功率
		read_reg(NWATT_ACC, 4, (unsigned char*)&num);
	}
	else if (iType == 3)
	{
		//正总无功功率
		read_reg(PVAR_ACC, 4, (unsigned char*)&num);
	}
	else
	{
		//负总无功功率
		read_reg(NVAR_ACC, 4, (unsigned char*)&num);
	}

	*value = (long)(num*10000.0/Kw);

	return 0;
}

int ADE9078_ReadWaveform(void)
{
	STATUS i;
	STATUS addr;
	int count = 0;
	ADE_REGISTER_UNION data;

	data.ul_Register = 0x0;
	write_reg(WFB_CFG, 2, (unsigned char*)&data.ul_Register);
	data.ul_Register = 0x800000;
	write_reg(STATUS0, 4, (unsigned char*)&data.ul_Register);
	
	data.ul_Register = 0x1F;
	write_reg(WFB_CFG, 2, (unsigned char*)&data.ul_Register);

	data.ul_Register = 0x0;
	
	while ((data.uc_Register[2] & 0x80) == 0) //超过200ms退出
	{
		read_reg(STATUS0, 4, (unsigned char*)&data.ul_Register);
		usleep(1000);
		count++;
		if(count > 200) //超时
			return ERROR;
	}
	
	i = 0;
	for (addr = 0x800; addr < 0xA00; addr=addr+4)
	{
		read_reg(addr, 4, (unsigned char*)&data.ul_Register);
		if(i >= MaxSampPointNum)
			continue;
		//Ia
		AD_RESULT[Ia][i] = data.us_Register[0];
		//Ua
		AD_RESULT[Ua][i] = data.us_Register[1];
		i++;
	}
	i = 0;
	for (addr = 0x801; addr < 0xA00; addr=addr+4)
	{
		read_reg(addr, 4, (unsigned char*)&data.ul_Register);
		if(i >= MaxSampPointNum)
			continue;
		//Ib
		AD_RESULT[Ib][i] = data.us_Register[0];
		//Ub
		AD_RESULT[Ub][i] = data.us_Register[1];
		i++;
	}
	i = 0;
	for (addr = 0x802; addr < 0xA00; addr=addr+4)
	{
		read_reg(addr, 4, (unsigned char*)&data.ul_Register);
		if(i >= MaxSampPointNum)
			continue;
		//Ic
		AD_RESULT[Ic][i] = data.us_Register[0];
		//Uc
		AD_RESULT[Uc][i] = data.us_Register[1];
		i++;
	}
	i = 0;
	for (addr = 0x803; addr < 0xA00; addr=addr+4)
	{
		read_reg(addr, 4, (unsigned char*)&data.ul_Register);
		if(i >= MaxSampPointNum)
			continue;
		//Ic
		AD_RESULT[I0][i] = data.us_Register[0];
		i++;
	}

	return OK;
}

int ADE9078_SetConfig()
{
	int i;
	int addr[] = {AVGAIN, BVGAIN, CVGAIN, AIGAIN, BIGAIN, CIGAIN, NIGAIN, APHCAL0, BPHCAL0, CPHCAL0};
	long data[10] = {0};
	WORD tempno[10] = {0};
	WORD gainno;
	
	for(i = 0 ; i < ycNum; i++ )
    {

        gainno = (WORD)pYcCfg[i].gainNo;
        if(pYcCfg[i].type == YC_TYPE_Ua)
        {
            data[0] = pYcGain[gainno].a;
            tempno[0] = gainno;
        }
		if(pYcCfg[i].type == YC_TYPE_Ub)
        {
            data[1] = pYcGain[gainno].a;
            tempno[1]  = gainno;
        }
		if(pYcCfg[i].type == YC_TYPE_Uc)
        {
            data[2] = pYcGain[gainno].a;
            tempno[2] =  gainno;
        }
		if(pYcCfg[i].type == YC_TYPE_Ia)
        {
            data[3] = pYcGain[gainno].a;
            tempno[3] = gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Ib)
        {
            data[4] = pYcGain[gainno].a;
            tempno[4] = gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Ic)
        {
            data[5] = pYcGain[gainno].a;
            tempno[5] = gainno;
        }
	   if(pYcCfg[i].type == YC_TYPE_I0)
        {
            data[6] = pYcGain[gainno].a;
            tempno[6] = gainno;
        }
	   if(pYcCfg[i].type == YC_TYPE_Pa)
        {
            data[7] = pYcGain[gainno].a;
            tempno[7] =  gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Pb)
        {
            data[8] = pYcGain[gainno].a;
            tempno[8] =  gainno;
        }
       if(pYcCfg[i].type == YC_TYPE_Pc)
        {
           data[9] = pYcGain[gainno].a;
            tempno[9] =  gainno;
        }
		
	}
	
	for (i = 0; i < (int)(sizeof(addr)/sizeof(addr[0])); i++)
	{
		if(!(pYcGain[tempno[i]].status & YC_GAIN_BIT_VALID))
            continue;
		write_reg(addr[i], 4, (unsigned char*)(data+i));
	}
	
	return 0;
}

int ADE9078_GainClear()
{
	unsigned int i;
	int addr[] = {AVGAIN, BVGAIN, CVGAIN, AIGAIN, BIGAIN, CIGAIN, NIGAIN,APHCAL0, BPHCAL0, CPHCAL0};
	long data = 0;
	
	for (i = 0; i < sizeof(addr)/sizeof(addr[0]); i++)
	{
		write_reg(addr[i], 4, (unsigned char*)(&data));
	}
	
	return 0;
}

int ADE908_GainI(int iIdx, int fReadI, int iStandardI,VYcGain* pycgain)
{
	STATUS xIGAIN;
	float k;

	if (iIdx < 0 || iIdx > 3)
	{
		return 0;
	}

	k = (float)((fReadI - iStandardI)*1.0/fReadI);
	xIGAIN = (int)(-k * (1<<27));
	pycgain->a = xIGAIN;
	if (iIdx == 0)
	{
		write_reg(AIGAIN, 4, (unsigned char*)&xIGAIN);
		DPRINT("adc gain AIDAIN succeed, addr = %#X, val = %#X\n", AIGAIN, xIGAIN);
	}
	else if (iIdx == 1)
	{
		write_reg(BIGAIN, 4, (unsigned char*)&xIGAIN);
		DPRINT("adc gain BIDAIN succeed, addr = %#X, val = %#X\n", BIGAIN, xIGAIN);
	}
	else if (iIdx == 2)
	{
		write_reg(CIGAIN, 4, (unsigned char*)&xIGAIN);
		DPRINT("adc gain CIDAIN succeed, addr = %#X, val = %#X\n", CIGAIN, xIGAIN);
	}
	else if (iIdx == 3)
	{
		write_reg(NIGAIN, 4, (unsigned char*)&xIGAIN);
		DPRINT("adc gain NIDAIN succeed, addr = %#X, val = %#X\n", NIGAIN, xIGAIN);
	}

	return 0;
}

int ADE9078_GainU(int iIdx, int fReadU, int iStandardU,VYcGain* pycgain)
{
	STATUS xVGAIN;
	float k;

	if (iIdx < 0 || iIdx > 2)
	{
		return -1;
	}

	k = (float)((fReadU - iStandardU)*1.0/fReadU);
	xVGAIN = (int)(-k * (1<<27));
	pycgain->a = xVGAIN;
	if (iIdx == 0)
	{
		write_reg(AVGAIN, 4, (unsigned char*)&xVGAIN);
		DPRINT("adc gain AVGAIN succeed, addr = %#X, val = %#X\n", AVGAIN, xVGAIN);
	}
	else if (iIdx == 1)
	{
		write_reg(BVGAIN, 4, (unsigned char*)&xVGAIN);
		DPRINT("adc gain BVGAIN succeed, addr = %#X, val = %#X\n", BVGAIN, xVGAIN);
	}
	else if (iIdx == 2)
	{
		write_reg(CVGAIN, 4, (unsigned char*)&xVGAIN);
		DPRINT("adc gain CVGAIN succeed, addr = %#X, val = %#X\n", CVGAIN, xVGAIN);
	}

	return 0;
}

int ADE9078_GainP(STATUS iIdx, float err)  //暂时不需要，电压电流整了之后即可
{
	STATUS xPGAIN;

	if (iIdx < 0 || iIdx > 2)
	{
		return -1;
	}

	xPGAIN = (int)((err/(1+err)) * (1<<27));
	
	if (iIdx == 0)
	{
		write_reg(APGAIN, 4, (unsigned char*)&xPGAIN);
		DPRINT("adc gian APGAIN succeed, val = %#X\n", xPGAIN);
	}
	else if (iIdx == 1)
	{
		write_reg(BPGAIN, 4, (unsigned char*)&xPGAIN);
		DPRINT("adc gian BPGAIN succeed, val = %#X\n", xPGAIN);
	}
	else if(iIdx == 2)
	{
		write_reg(CPGAIN, 4, (unsigned char*)&xPGAIN);
		DPRINT("adc gian CPGAIN succeed, val = %#X\n", xPGAIN);
	}

	return 0;
}

int ADE9078_GainPhase(STATUS iIdx,int fReadU, int iStandardU, VYcGain* pycgain)
{
	STATUS xPHCAL0;
	float k;
	double fPF = 0.5,err;

	if (iIdx < 0 || iIdx > 2)
	{
		return -1;
	}
	
	err = (double)((fReadU - iStandardU)*1.0/iStandardU);

		
	k = (float)(acos(fPF * (1 + err)) - M_PI/3);
	
	xPHCAL0 = (int)((sin(k - 0.07854) + sin(0.07854))/(sin(2 * 0.07854 - k) * (1<<27)));
	pycgain->a = xPHCAL0;
	if (iIdx == 0)
	{
		write_reg(APHCAL0, 4, (unsigned char*)&xPHCAL0);
		DPRINT("adc gian AI_PCF_BEAT0 succeed, val = %#X\n", xPHCAL0);
	}
	else if (iIdx == 1)
	{
		write_reg(BPHCAL0, 4, (unsigned char*)&xPHCAL0);
		DPRINT("adc gian BI_PCF_BEAT0 succeed, val = %#X\n", xPHCAL0);
	}
	else if(iIdx == 2)
	{
		write_reg(CPHCAL0, 4, (unsigned char*)&xPHCAL0);
		DPRINT("adc gian CI_PCF_BEAT0 succeed, val = %#X\n", xPHCAL0);
	}

	return 0;
}

void Ade9078_ReadEP(void)
{
	long eptemp;
	int i;
		
	for(i = 0; i < 3;i++)
	{
		ADE9078_Readw(1+i,&eptemp); //有功,带符号
		AWATT_EP[i] = (float)(eptemp*1.0/(Kw*4/8192));
	}

	for(i = 0; i < 3;i++)
	{
		ADE9078_ReadV(1+i,&eptemp); //无功,带符号
		AWATT_EP[3+i] = (float)(eptemp*1.0/(Kw*4/8192));
	}

	for(i = 0;i < 6;i++) //将ws转为kwh
	{
		AWATT_EP[i] = AWATT_EP[i]/(1000*3600);
	}

		//正向有功,反向有功
	for(i = 0;i < 3;i++)
	{
		if(AWATT_EP[i] < 0) //反向
			Ade9078_Dd[4] -= AWATT_EP[i];
		else
			Ade9078_Dd[0] += AWATT_EP[i];
	}

	//正向无功，反向无功
	for(i = 0;i < 3;i++)
	{
		if(AWATT_EP[i+3] < 0) //反向
			Ade9078_Dd[5] -= AWATT_EP[i+3];
		else
			Ade9078_Dd[1] += AWATT_EP[i+3];
	}

	//无功
	for(i = 0;i < 3;i++)
	{
		if((AWATT_EP[i] < 0)  && (AWATT_EP[i+3] < 0))  //三
			Ade9078_Dd[7] -= AWATT_EP[i+3];
		else if((AWATT_EP[i] < 0)  && (AWATT_EP[i+3] > 0)) //二
			Ade9078_Dd[6] += AWATT_EP[i+3];
		else if((AWATT_EP[i] > 0)  && (AWATT_EP[i+3] > 0)) //一
			Ade9078_Dd[2] += AWATT_EP[i+3];
		else if((AWATT_EP[i] > 0)  && (AWATT_EP[i+3] < 0)) //四
			Ade9078_Dd[3] -= AWATT_EP[i+3];
	}
	
#if 0  //因电能已带了符号位，故删掉	
	read_reg(PHSIGN, 2, (BYTE*)&wsign); //0为正，1为负
	
	printf("PHSIGN  %x  \n",wsign);
	
	//正向有功,反向有功
	for(i = 0;i < 3;i++)
	{
		if(wsign & (AWSIGN << (2*i))) //反向
			Ade9078_Dd[4] += AWATT_EP[i];
		else
			Ade9078_Dd[0] += AWATT_EP[i];
	}

	//正向无功，反向无功
	for(i = 0;i < 3;i++)
	{
		if(wsign & (AVARSIGN << (2*i))) //反向
			Ade9078_Dd[5] += AWATT_EP[i+3];
		else
			Ade9078_Dd[1] += AWATT_EP[i+3];
	}

	//无功
	for(i = 0;i < 3;i++)
	{
		if((wsign & (AWSIGN << (2*i)))  && (wsign & (AVARSIGN << (2*i))))  //三
			Ade9078_Dd[7] += AWATT_EP[i+3];
		else if((wsign & (AWSIGN << (2*i)))  && ((wsign & (AVARSIGN << (2*i))) == 0)) //二
			Ade9078_Dd[6] += AWATT_EP[i+3];
		else if(((wsign & (AWSIGN << (2*i))) == 0)  && ((wsign & (AVARSIGN << (2*i))) == 0)) //一
			Ade9078_Dd[2] += AWATT_EP[i+3];
		else if(((wsign & (AWSIGN << (2*i))) == 0)  && (wsign & (AVARSIGN << (2*i)))) //四
			Ade9078_Dd[3] += AWATT_EP[i+3];
	}
#endif

}


//正向有功、正向无功、一象限、四象限、反向有功、反向无功、二象限、三象限
long Ade9078_GetEP(int chn)
{	
	DWORD val;
	
	if((chn < 0) || (chn > 7))
		return 0;

	memcpy(&val,&Ade9078_Dd[chn],4);
	return  (long)val;
}

void  Ade9078ddfrzfif( struct VSysClock *pclk)
{
	int i;
	struct  VDDF dbExtDdValue;
	struct VDDFT Ddbuf;
	struct VCalClock bFrzfifCP56Time;
	
	ToCalClock(pclk,&bFrzfifCP56Time);
	
	for(i=0;i<8; i++)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i , 1 , sizeof(dbExtDdValue), &dbExtDdValue);
		Ddbuf.byFlag = dbExtDdValue.byFlag;
		Ddbuf.lValue= dbExtDdValue.lValue;
		Ddbuf.Time = bFrzfifCP56Time;
		WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+8, 1, &Ddbuf);	
	}
}

void Ade9078ddfrzday( struct VSysClock *pclk)
{
	int i;
	struct  VDDF dbExtDdValue;
	struct VDDFT Ddbuf;
	struct VCalClock bFrzddCP56Time;
	ToCalClock(pclk,&bFrzddCP56Time);

	for(i=0;i<8; i++)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i , 1 , sizeof(dbExtDdValue),&dbExtDdValue);
		Ddbuf.byFlag = dbExtDdValue.byFlag;
		Ddbuf.lValue= dbExtDdValue.lValue;
		Ddbuf.Time = bFrzddCP56Time;
		WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+8*2, 1, &Ddbuf);	
	}
}

void Ade9078ddAllday( struct VSysClock *pclk)
{
	struct VDDF dbExtDdValue[2];
	struct VDDFT Ddbuf;
	struct VCalClock bFrzddCP56Time;
	ToCalClock(pclk,&bFrzddCP56Time);

	ReadRangeDDF(g_Sys.wIOEqpID ,  0 , 1 , sizeof(struct VDDF),&dbExtDdValue[0]);
	ReadRangeDDF(g_Sys.wIOEqpID ,  4 , 1 , sizeof(struct VDDF),&dbExtDdValue[1]);
	Ddbuf.byFlag = dbExtDdValue[0].byFlag;
	Ddbuf.lValue= dbExtDdValue[0].lValue + dbExtDdValue[1].lValue;
	Ddbuf.Time = bFrzddCP56Time;
	WriteRangeDDFT(g_Sys.wIOEqpID, 32, 1, &Ddbuf);

	ReadRangeDDF(g_Sys.wIOEqpID ,  1 , 1 , sizeof(struct VDDF),&dbExtDdValue[0]);
	ReadRangeDDF(g_Sys.wIOEqpID ,  5 , 1 , sizeof(struct VDDF),&dbExtDdValue[1]);
	Ddbuf.byFlag = dbExtDdValue[0].byFlag;
	Ddbuf.lValue= dbExtDdValue[0].lValue + dbExtDdValue[1].lValue;
	Ddbuf.Time = bFrzddCP56Time;
	WriteRangeDDFT(g_Sys.wIOEqpID, 33, 1, &Ddbuf);

}

void Ade9078ddCalJFPG( struct VSysClock *pclk)
{
	int i,j;
	float dd[2] = {0};
	struct VDDF JFPGDdValue[8];
	struct VDDFT Ddbuf;
	struct VCalClock bFrzddCP56Time;
	ToCalClock(pclk,&bFrzddCP56Time);
	
	
	j = (pclk->byHour*60+pclk->byMinute)/30;//当前时段
	if(extEnergyT[j]==0)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  34 , 1 , sizeof(struct VDDF),&JFPGDdValue[0]);
		ReadRangeDDF(g_Sys.wIOEqpID ,  35 , 1 , sizeof(struct VDDF),&JFPGDdValue[1]);
		memcpy((void*)&dd[0],(void*)&JFPGDdValue[0].lValue,4);
		memcpy((void*)&dd[1],(void*)&JFPGDdValue[1].lValue,4);
		
		dd[0] +=(float)(mP)/(1000*60*10);
		dd[1] +=(float)(mQ)/(1000*60*10);
		
		for(i = 0;i<2;i++)
		{	
			g_Ade9078_Dd_JFPG[i].DD_JFPG = dd[i];
			Ddbuf.byFlag = JFPGDdValue[i].byFlag;
			memcpy((void*)&Ddbuf.lValue,(void*)&dd[i],4);
			Ddbuf.Time = bFrzddCP56Time;
			WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+34, 1, &Ddbuf);
		}
		
	}
	else if(extEnergyT[j]==1)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  36 , 1 , sizeof(struct VDDF),&JFPGDdValue[2]);
		ReadRangeDDF(g_Sys.wIOEqpID ,  37 , 1 , sizeof(struct VDDF),&JFPGDdValue[3]);
		memcpy((void*)&dd[0],(void*)&JFPGDdValue[2].lValue,4);
		memcpy((void*)&dd[1],(void*)&JFPGDdValue[3].lValue,4);
		dd[0] += (float)(mP)/(1000*60*10);
		dd[1] += (float)(mQ)/(1000*60*10);
		for(i = 0;i<2;i++)
		{	
			g_Ade9078_Dd_JFPG[i+2].DD_JFPG = dd[i];
			Ddbuf.byFlag = JFPGDdValue[i+2].byFlag;
			memcpy((void*)&Ddbuf.lValue,(void*)&dd[i],4);
			Ddbuf.Time = bFrzddCP56Time;
			WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+36, 1, &Ddbuf);
		}
	}
	else if(extEnergyT[j]==2)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  38 , 1 , sizeof(struct VDDF),&JFPGDdValue[4]);
		ReadRangeDDF(g_Sys.wIOEqpID ,  39 , 1 , sizeof(struct VDDF),&JFPGDdValue[5]);
		memcpy((void*)&dd[0],(void*)&JFPGDdValue[4].lValue,4);
		memcpy((void*)&dd[1],(void*)&JFPGDdValue[5].lValue,4);
		dd[0] += (float)(mP)/(1000*60*10);
		dd[1] += (float)(mQ)/(1000*60*10);
		for(i = 0;i<2;i++)
		{	
			g_Ade9078_Dd_JFPG[i+4].DD_JFPG = dd[i];
			Ddbuf.byFlag = JFPGDdValue[i+4].byFlag;
			memcpy((void*)&Ddbuf.lValue,(void*)&dd[i],4);
			Ddbuf.Time = bFrzddCP56Time;
			WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+38, 1, &Ddbuf);
		}
	}
	else if(extEnergyT[j]==3)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  40 , 1 , sizeof(struct VDDF),&JFPGDdValue[6]);
		ReadRangeDDF(g_Sys.wIOEqpID ,  41 , 1 , sizeof(struct VDDF),&JFPGDdValue[7]);
		memcpy((void*)&dd[0],(void*)&JFPGDdValue[6].lValue,4);
		memcpy((void*)&dd[1],(void*)&JFPGDdValue[7].lValue,4);
		dd[0] += (float)(mP)/(1000*60*10);
		dd[1] += (float)(mQ)/(1000*60*10);
		for(i = 0;i<2;i++)
		{	
			g_Ade9078_Dd_JFPG[i+6].DD_JFPG = dd[i];
			Ddbuf.byFlag = JFPGDdValue[i+6].byFlag;
			memcpy((void*)&Ddbuf.lValue,(void*)&dd[i],4);
			Ddbuf.Time = bFrzddCP56Time;
			WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+40, 1, &Ddbuf);
		}
	}

	for(i =0;i <8;i++)
	{
		g_Ade9078_Dd_JFPG[i].DD_Time.wYear = pclk->wYear;
		g_Ade9078_Dd_JFPG[i].DD_Time.byMonth = pclk->byMonth;
		g_Ade9078_Dd_JFPG[i].DD_Time.byDay = pclk->byDay;
		g_Ade9078_Dd_JFPG[i].DD_Time.byWeek = pclk->byWeek;
		g_Ade9078_Dd_JFPG[i].DD_Time.byHour = pclk->byHour;
		g_Ade9078_Dd_JFPG[i].DD_Time.byMinute = pclk->byMinute;
		g_Ade9078_Dd_JFPG[i].DD_Time.bySecond = pclk->bySecond;
		g_Ade9078_Dd_JFPG[i].DD_Time.wMSecond = pclk->wMSecond;	
	}

	extNvRamSet(NVRAM_AD_DD_JFPG, (BYTE*)g_Ade9078_Dd_JFPG, sizeof(g_Ade9078_Dd_JFPG));

}

void Ade9078ddClearJFPG( struct VSysClock *pclk)
{
	int i;
	struct VDDF dbExtDdValue;
	struct VDDFT Ddbuf;
	struct VCalClock bFrzddCP56Time;
	
	memset(g_Ade9078_Dd_JFPG, 0 , sizeof(g_Ade9078_Dd_JFPG));
	extNvRamSet(NVRAM_AD_DD_JFPG, (BYTE*)g_Ade9078_Dd_JFPG, sizeof(g_Ade9078_Dd_JFPG));
	ToCalClock(pclk,&bFrzddCP56Time);
	for(i=0;i<8; i++)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i+34 , 1 , sizeof(dbExtDdValue),&dbExtDdValue);
		Ddbuf.byFlag = dbExtDdValue.byFlag;
		Ddbuf.lValue= 0;
		Ddbuf.Time = bFrzddCP56Time;
		WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+34, 1, &Ddbuf);	
	}
}

//检测潮流方向
void Ade9078PowerFlowFaultEvent(BYTE bType)
{
	int  vlt[3] = {0};						//-当前３相电压-
	int  cur[3] = {0};						//-当前３相电流-
	int  actPower[3] = {0};					//-当前３相有功	
	int  actPowerFlowAll;
	WORD  lastTimesLmt = 60;   			    //持续时间限值,60s
	static DWORD lastTimes[2] = {0};	//事件发生/恢复持续时间
	BYTE	tmp_flag = 0;   /* 潮流反向或反向恢复满足功率方向条件 */
	BYTE	is_now_flag = 0; 
	int i;
	struct  VDDF dbExtDdValue;
	struct VDDFT Ddbuf;
	static struct VCalClock bFlowRevFrzCP56Time;
	struct VSysFlowEventInfo Flowinfo;

	//-读取实时电压、电流-
    vlt[0] = UAde9078[0];
    vlt[1] = UAde9078[1];
    vlt[2] = UAde9078[2];
    cur[0] = IAde9078[0];
    cur[1] = IAde9078[1];
    cur[2] = IAde9078[2];
    actPower[0] = PQAde9078[0];
    actPower[1] = PQAde9078[1];
    actPower[2] = PQAde9078[2];
	
	if((cur[0] < 100)&&(vlt[0] < 100)&&(cur[1] < 100)&&(vlt[1] < 50)&&(cur[2] < 50)&&(vlt[2] < 50)) //1V 0.05A
	{	
		return;//-电压电流停电-
	}

	actPowerFlowAll = actPower[0] + actPower[1] + actPower[2];

	if(bType == 1)		//潮流反向
	{
		if(actPowerFlowAll < 0)
		{
			tmp_flag = labs(actPowerFlowAll) > POWER_REV;
		}
	}
	else if(bType == 2)	//潮流反向恢复
	{
		if(actPowerFlowAll > 0)
			tmp_flag = actPowerFlowAll > POWER_REV;
	}
	else
	{
		return;
	}
	
	if(tmp_flag)		//如果满足相应的发生条件，将对应的时间进行累加
	{	
		if(lastTimes[bType-1] >= (DWORD)(lastTimesLmt-1))
		{
			is_now_flag = 1;
		}
		else
		{
			lastTimes[bType-1]++;
			is_now_flag = 0;
		}
		if((bType == 1) && (bActPowerDirHis == 0))
		{
			GetSysClock(&bFlowRevFrzCP56Time, CALCLOCK);
			bActPowerDirHis = 1;
		}
		else if((bType == 2) && (bActPowerDirHis == 1))
		{
			 GetSysClock(&bFlowRevFrzCP56Time, CALCLOCK);
			bActPowerDirHis = 0;
		}	
	}
	else
	{
		lastTimes[bType-1] = 0;
		is_now_flag = 0;
	}

	if(is_now_flag)
	{
		if((1 == bType) && (bFlowDirHis == 0))
		{
			bFlowDirHis = 1;
			memset(&Flowinfo , 0 , sizeof(struct VSysFlowEventInfo));
			for(i=0;i<8; i++)
			{
				ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i , 1 ,  sizeof(dbExtDdValue),&dbExtDdValue);
				Ddbuf.byFlag = dbExtDdValue.byFlag;
				Ddbuf.lValue= dbExtDdValue.lValue;
				Ddbuf.Time = bFlowRevFrzCP56Time;
				WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+3*8, 1, &Ddbuf);
				
				Flowinfo.dbflow[Flowinfo.num].Time = bFlowRevFrzCP56Time;
				Flowinfo.dbflow[Flowinfo.num].val = dbExtDdValue.lValue;
				Flowinfo.dbflow[Flowinfo.num++].wNo = (WORD)i + 3*8;
			}
			//if(Flowinfo.num)
			//	 WriteFlowEvent(NULL, 0, 0, &Flowinfo);
		}
		else if((2 == bType) && (bFlowDirHis == 1))
		{
			bFlowDirHis = 0;
			memset(&Flowinfo , 0 , sizeof(struct VSysFlowEventInfo));
			for(i=0;i<8; i++)
			{
				ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i , 1 , sizeof(dbExtDdValue), &dbExtDdValue);
				Ddbuf.byFlag = dbExtDdValue.byFlag;
				Ddbuf.lValue= dbExtDdValue.lValue;
				Ddbuf.Time = bFlowRevFrzCP56Time;
				WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+3*8, 1, &Ddbuf);

				Flowinfo.dbflow[Flowinfo.num].Time = bFlowRevFrzCP56Time;
				Flowinfo.dbflow[Flowinfo.num].val = dbExtDdValue.lValue;
				Flowinfo.dbflow[Flowinfo.num++].wNo = (WORD)i + 3*8;
			}
			//if(Flowinfo.num)
			//	 WriteFlowEvent(NULL, 0, 0, &Flowinfo);
		}
	}
}


void Ade9078_Eqp(void)
{
	int i;
	struct  VDDF dbExtDdValue;
	
	DWORD val;
	struct VSysClock curclock;
	static struct VSysClock oldclock = {0};
	
	GetSysClock(&curclock, SYSCLOCK);
	if((oldclock.wYear == 0) && (oldclock.byMonth == 0))
		memcpy(&oldclock, &curclock, sizeof(struct VSysClock));
		
	for( i = 0 ; i < 8 ; i++)
	{
		val = (DWORD)Ade9078_GetEP(i);
		memcpy((BYTE*)&dbExtDdValue.lValue,(BYTE*)&val,4);
		dbExtDdValue.byFlag = 0x01;	  
		WriteRangeDDF(g_Sys.wIOEqpID ,  (WORD)i , 1 ,  &dbExtDdValue);
	}
	
	if(curclock.bySecond != oldclock.bySecond)
	{
		Ade9078PowerFlowFaultEvent(1);
		Ade9078PowerFlowFaultEvent(2);
	}
	
	if(curclock.byMinute != oldclock.byMinute)
	{
		extNvRamSet(NVRAM_AD_DD+sizeof(DWORD), (BYTE*)Ade9078_Dd, sizeof(Ade9078_Dd)); //一分钟写一次文件

		if(curclock.byMinute%15 == 0)
		{
			Ade9078ddfrzfif(&curclock);
		}
		Ade9078ddCalJFPG(&curclock);
	}
	if(curclock.byDay != oldclock.byDay)
	{
		Ade9078ddfrzday(&curclock);
		Ade9078ddAllday(&curclock);//日总有功无功
	}
	memcpy(&oldclock, &curclock, sizeof(struct VSysClock));
}

void Ade9078Dd_Clear(void) //需要遥控调用
{
	DWORD type;
	struct VSysClock curclock;
	memset(Ade9078_Dd , 0 , sizeof(Ade9078_Dd));
	GetSysClock(&curclock, SYSCLOCK);
	extNvRamGet(NVRAM_AD_DD, (BYTE *)&type, sizeof(DWORD));	
	if (type != g_Sys.dwAioType)
	{
		type = g_Sys.dwAioType;
		extNvRamSet(NVRAM_AD_DD+sizeof(DWORD), (BYTE*)Ade9078_Dd, sizeof(Ade9078_Dd));
		extNvRamSet(NVRAM_AD_DD, (BYTE *)&type, sizeof(DWORD));
	}
	else
	{
		extNvRamSet(NVRAM_AD_DD+sizeof(DWORD), (BYTE*)Ade9078_Dd, sizeof(Ade9078_Dd));
	}
	Ade9078ddClearJFPG(&curclock);
}

void Ade9078Dd_Init(void)
{
	DWORD type;
	int i;
	struct VDDF JFPGDdValue[8];
	struct VDDFT Ddbuf;
	struct VCalClock bFrzddCP56Time;
	struct VSysClock curclock;	
	memset(Ade9078_Dd , 0 , sizeof(Ade9078_Dd));
	memset(g_Ade9078_Dd_JFPG, 0 , sizeof(g_Ade9078_Dd_JFPG));
	GetSysClock(&curclock, SYSCLOCK);
	ToCalClock(&curclock,&bFrzddCP56Time);
	extNvRamGet(NVRAM_AD_DD, (BYTE *)&type, sizeof(DWORD));	
	if (type != g_Sys.dwAioType)
	{
		type = g_Sys.dwAioType;
		extNvRamSet(NVRAM_AD_DD+sizeof(DWORD), (BYTE*)Ade9078_Dd, sizeof(Ade9078_Dd));
		extNvRamSet(NVRAM_AD_DD, (BYTE *)&type, sizeof(DWORD));
	}
	else
	{
		//extNvRamGet(NVRAM_AD_DD+sizeof(DWORD),(BYTE*) Ade9078_Dd, sizeof(Ade9078_Dd));
	}		
	extNvRamGet(NVRAM_AD_DD_JFPG,(BYTE*) g_Ade9078_Dd_JFPG, sizeof(g_Ade9078_Dd_JFPG));	
	for(i=0;i<8;i++)
	{
		ReadRangeDDF(g_Sys.wIOEqpID ,  (WORD)i+34 , 1 , sizeof(struct VDDF),&JFPGDdValue[i]);
		
		Ddbuf.byFlag = JFPGDdValue[i].byFlag;
		memcpy((void*)&Ddbuf.lValue,(void*)&g_Ade9078_Dd_JFPG[i].DD_JFPG,4);
		Ddbuf.Time = bFrzddCP56Time;
		WriteRangeDDFT(g_Sys.wIOEqpID, (WORD)i+34, 1, &Ddbuf);
	}
	
}



#endif

