#include "irigb.h"
#include "bsp.h"
#include "sys.h"
#include "stdio.h"

//目前只支持箱式FTU的引脚
#ifdef IRIGB_FLAG
#define  ISLEAP(y)          ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)  /* Judge leap year, If true,return 1; else return 0 */

const int AverMonthDay[]={0,31,59,90,120,151,181,212,243,273,304,334,365};
const int LeapMonthDay[]={0,31,60,91,121,152,182,213,244,274,305,335,366};

static struct TSysTime IrigBTime;

EXTI_InitTypeDef EXTI_InitStructure;
unsigned char second[7];
unsigned char minute[7];
unsigned char hour[6];
unsigned char year[8];
unsigned char day[10];
static struct IRIG_BTIME TimeNow;
static struct IRIG_BDATE TimeDateNow;


void IrigbInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	sysPinInit(GPIOC, &GPIO_InitStructure);
	syscfgEXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource11);

	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Line = EXTI_Line11;
	stm32EXTIInit(&EXTI_InitStructure);
	EXTI->PR = EXTI_Line11;

	stm32fIntEnable(EXTI15_10_IRQn, INT_PRIO_UART);     
	//信号线
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  	sysPinInit(GPIOC, &GPIO_InitStructure);
	SET_GPIO(GPIOC,GPIO_Pin_10);
	
}
void Irigb_isr(void)
{ 
	SetSysClock(&IrigBTime, SYSCLOCK);
	//关中断
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	stm32EXTIInit(&EXTI_InitStructure);
}

void Irig_B_ms_Scan(void)
{//Status表示IO口电平状态，lvyi
	static unsigned char iPreSts_s;                   /* 前一轮询点采集到的通道数据 */
	static unsigned char iWidth_s;                    /* 有效脉冲的宽度 */
	static unsigned char iPreCode_s=0;                /* 上一个脉冲码 */
	static unsigned char iCode_s;                     /* 当前脉冲码 */
	static unsigned char iMark_s=0;                   /* 码序号 */

	unsigned char Status = 0;
	Status |= (READ_GPIO(GPIOC, GPIO_Pin_11)>> 11);

	if(!Status)
	{
		if (iPreSts_s)                  /* 解码时刻如果是第一次为零 */
		{
			switch(iWidth_s)
			{
				case 0:
					break;
				case 1:
				case 2:
				case 3:
					iCode_s=0;    //0 code
					iMark_s++;
					break;
				case 4: 
				case 5:
				case 6:
					iCode_s=1;   // 1-code
					iMark_s++;             
					break;
				case 7:
				case 8:
				case 9:
					iCode_s=2;    //p code
					iMark_s++;

					if(iMark_s==99)			//开中断GpsSetBTime
					{
						Irig_B_Parse();
						Irig_B_Create_Date(&TimeDateNow,&TimeNow);
						Irig_B_Create_AbsTime(&TimeDateNow,&TimeNow);
						GpsSetBTime();
					}
				     /* 捕捉到准时参考点? */
					if (iPreCode_s==2)             //
					{
						iCode_s=0;
						iPreCode_s=0;
						iWidth_s=0;
						iMark_s=0;
					}
				break;
			default:
				break; 
			}
		}
		
		iPreCode_s=iCode_s;

		if (iMark_s>0 && iMark_s<59) //可以对时处理了
		{
			/* 利用Day<0为出错标志 */
			if(iMark_s>0 && iMark_s<5)
			{
				second[iMark_s-1]=iCode_s;
			}
			if(iMark_s>5 && iMark_s<9)
			{
				second[iMark_s-2]=iCode_s;
			}
			if(iMark_s>9 && iMark_s<14)
			{
				minute[iMark_s-10]=iCode_s;
			}
			if(iMark_s>14 && iMark_s<18)
			{
				minute[iMark_s-11]=iCode_s;
			}
			if(iMark_s>19 && iMark_s<24)
			{
				hour[iMark_s-20]=iCode_s;
			}
			if(iMark_s>24 && iMark_s<27)
			{
				hour[iMark_s-21]=iCode_s;
			}
			if(iMark_s>29 && iMark_s<34)
			{
				day[iMark_s-30]=iCode_s;
			}
			if(iMark_s>34 && iMark_s<39)
			{
				day[iMark_s-31]=iCode_s;
			}
			if(iMark_s>39 && iMark_s<42)
			{
				day[iMark_s-32]=iCode_s;
			}
			if(iMark_s>49 && iMark_s<54)
			{
				year[iMark_s-50]=iCode_s;
			}
			if(iMark_s>54 && iMark_s<59)
			{
				year[iMark_s-51]=iCode_s;
			}
		}
		
	}
	else
	{
		if (iPreSts_s)
            iWidth_s++;
        else
            iWidth_s=1;
	}
	iPreSts_s=Status;
}
static void Irig_B_Parse(void)
{
	TimeNow.Second=second[0]+second[1]*2+second[2]*4+second[3]*8+10*(second[4]+second[5]*2+second[6]*4);
	TimeNow.Minute=minute[0]+minute[1]*2+minute[2]*4+minute[3]*8+10*(minute[4]+minute[5]*2+minute[6]*4);
	TimeNow.Hour=hour[0]+hour[1]*2+hour[2]*4+hour[3]*8+10*(hour[4]+hour[5]*2);
	TimeNow.Day=day[0]+day[1]*2+day[2]*4+day[3]*8+10*(day[4]+day[5]*2+day[6]*4+day[7]*8)+100*(day[8]+day[9]*2);
	TimeNow.Year=year[0]+year[1]*2+year[2]*4+year[3]*8+10*(year[4]+year[5]*2+year[6]*4+year[7]*8);
}

//生成（年-月-日）
static void Irig_B_Create_Date(struct IRIG_BDATE *pdt, const struct IRIG_BTIME *ptm)
{
	int cDay;
	unsigned char cMonth;
	if(ptm->Year>=1)
		pdt->iYear=ptm->Year+2000;
	else
	{
		if (pdt->iMonth==12 && ptm->Day<5)
		    pdt->iYear++;
		else if (pdt->iMonth==1 && ptm->Day>360)
		    pdt->iYear--;
	}
	cDay=ptm->Day-1;

	cMonth=cDay/29;
	if (pdt->iYear & 0x03)              //Not Leap year
	{
		if (cDay<AverMonthDay[cMonth])
		    cMonth--;
		pdt->iDate=(unsigned char)(cDay-AverMonthDay[cMonth]+1);
	}
	else								//Leap year
	{
		if (cDay<LeapMonthDay[cMonth])
		    cMonth--;
		pdt->iDate=(unsigned char)(cDay-LeapMonthDay[cMonth]+1);
	}
	pdt->iMonth=(unsigned char)(cMonth+1);
}

void Irig_B_Create_AbsTime(struct IRIG_BDATE *pdt, const struct IRIG_BTIME *ptm)
{
	IrigBTime.Year=pdt->iYear;
	IrigBTime.Month=pdt->iMonth;
	IrigBTime.Day=pdt->iDate;
	IrigBTime.Hour=ptm->Hour;
	IrigBTime.Minute=ptm->Minute;
	IrigBTime.Second=ptm->Second;
	IrigBTime.MSecond=0;
	CaculateWeekDay(IrigBTime.Year,IrigBTime.Month,IrigBTime.Day);
}

void CaculateWeekDay(int year,int month, int day)
{
	if(month==1||month==2) 
	{
		month+=12;
		year--;
	}
	int iWeek=(day+2*month+3*(month+1)/5+year+year/4-year/100+year/400)%7;
	IrigBTime.byWeek = iWeek;
}

void GpsSetBTime(void)
{

	syscfgEXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource11);			//开中断PC11,b码对时
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	stm32EXTIInit(&EXTI_InitStructure);
}
#endif

