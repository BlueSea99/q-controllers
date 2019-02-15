//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ���װ��rtc������������ʱ��⣬�ɱ���������������stm32��Ŀ�����ٴ��뿪����
*/
//------------------------------------------------------------------//

#include "Drivers.h"

#define RTC_START_YEAR 1980  //�ɴ�1904�����κ�һ�����꿪ʼ,����ı��ֵ�����ڳ�ֵҪ�ı�
#define RTC_WEEK_INIT_OFFSET 2//����ȡֵ0������ȡֵ6�Դ�����
#define DAY_SECONDS 86400	//һ���������

//�����������������
const int Leap_Month_Seconds[13]={
	0,
	DAY_SECONDS*31,
	DAY_SECONDS*(31+29),
	DAY_SECONDS*(31+29+31),
	DAY_SECONDS*(31+29+31+30),
	DAY_SECONDS*(31+29+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31+30+31),
};

//�������������������
const int Month_Seconds[13]={
	0,
	DAY_SECONDS*31,
	DAY_SECONDS*(31+28),
	DAY_SECONDS*(31+28+31),
	DAY_SECONDS*(31+28+31+30),
	DAY_SECONDS*(31+28+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31+30+31),
};

#define RTCClockSource_LSE // ʹ���ⲿʱ�ӣ� 32.768KHz 


//ʱ��ת����
//��ʽ���Է���0
u32 RtcTime2Cnt(RTC_TIME *pTime)
{
	u32 TimeCnt,Tmp;
	u8 LeapFlag=0;

	TimeCnt=pTime->year-RTC_START_YEAR;
	if((TimeCnt>135)||(pTime->year<RTC_START_YEAR)) return 0; //��ݼ��
	
	if(TimeCnt) Tmp=(TimeCnt-1)/4+1;//�ж��������
	else Tmp=0;
	LeapFlag=(TimeCnt%4)?0:1;
	TimeCnt=(TimeCnt*365+Tmp)*DAY_SECONDS;//�껻��ɵ�����

	if((pTime->mon<1)||(pTime->mon>12)) return 0;  //�·ݼ��
	
	if(LeapFlag)
	{
		if(pTime->day>((Leap_Month_Seconds[pTime->mon]-Leap_Month_Seconds[pTime->mon-1])/DAY_SECONDS)) return 0; //�ռ��
		Tmp=Leap_Month_Seconds[pTime->mon-1];
	}
	else
	{
		if(pTime->day>((Month_Seconds[pTime->mon]-Month_Seconds[pTime->mon-1])/DAY_SECONDS)) return 0;//�ռ��
		Tmp=Month_Seconds[pTime->mon-1];
	}

	if(pTime->hour>23) return 0; //Сʱ���
	if(pTime->min>59) return 0;	 //���Ӽ��
	if(pTime->sec>59) return 0;	 //����

	TimeCnt+=(Tmp+(pTime->day-1)*DAY_SECONDS);

	TimeCnt+=(pTime->hour*3600 + pTime->min*60 + pTime->sec);

	return TimeCnt;
}

//����תʱ��
void RtcCnt2Time(u32 TimeCnt,RTC_TIME *pTime)
{ 
  u32 Tmp,i;

  //���������꣬���ǵ�����Ĵ��ڣ���4��һ������
  Tmp=TimeCnt%(DAY_SECONDS*366+DAY_SECONDS*365*3);
  if(Tmp<DAY_SECONDS*366) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+0;
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*1) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+1;
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*2) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+2;
  else pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+3;

  if(Tmp<DAY_SECONDS*366) //����
  {
		for(i=1;i<13;i++)
	    {
			if(Tmp<Leap_Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Leap_Month_Seconds[i-1];//�������ѯ���渴�ӵļ���
				break;
		  	}
	   }
  }
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*1)
  {
		Tmp-=DAY_SECONDS*366;

	  	for(i=1;i<13;i++)
		{
			if(Tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Month_Seconds[i-1];
				break;
			}
		}		
  }
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*2)
  {
  		Tmp-=DAY_SECONDS*366+DAY_SECONDS*365*1;

	  	for(i=1;i<13;i++)
		{
			if(Tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Month_Seconds[i-1];
				break;
			}
		}	  		
  }
  else
  {
		Tmp-=DAY_SECONDS*366+DAY_SECONDS*365*2;
		
	  	for(i=1;i<13;i++)
		{
			if(Tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Month_Seconds[i-1];
				break;
			}
		}			
  }

  pTime->week=(TimeCnt/DAY_SECONDS+RTC_WEEK_INIT_OFFSET)%7;//��Ϊ1912.1.1������һ������ƫ��ֵ=1
  //ע���ȡ��ֵ��ΧΪ0-6��������Ϊ�˷���������ģ��Ӷ������������ʾ

  pTime->day=Tmp/DAY_SECONDS+1;
  Tmp=Tmp%DAY_SECONDS;

  pTime->hour = Tmp/3600;
  Tmp=Tmp%3600;

  pTime->min = Tmp/60;

  pTime->sec = Tmp%60;
}

/*******************************************************************************
* Function Name  : RtcConfiguration
* Description    : Configures the rtc.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
#define RTC_RST_FLAG		0xa5a5
void RtcConfiguration(void)
{
    u32 rtcintcnt=0x200000;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  //�򿪵�Դ����ͱ��ݼĴ���ʱ��
    PWR_BackupAccessCmd(ENABLE);            //ʹ��RTC�ͱ��ݼĴ����ķ���(��λĬ�Ϲر�)
    BKP_DeInit();                           //BKP���踴λ
    RCC_LSEConfig(RCC_LSE_ON);              //���ⲿ���پ���
    
    //while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (--rtcintcnt));//�ȴ�LSE׼����
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    if(rtcintcnt!=0)//�ڲ�����
    {
    	Debug("RTC LSE WORKING\n\r");
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //ѡ��LSEλRTCʱ��
        RCC_RTCCLKCmd(ENABLE);                  //ʹ��RTCʱ��
        RTC_WaitForSynchro();                   //�ȴ�RTC�Ĵ�����APBʱ��ͬ��
        RTC_WaitForLastTask();                  //�ȴ�RTC�Ĵ���д�������(�����ڶ�RTC�Ĵ���д����Ǯ����)
        RTC_ITConfig(RTC_IT_SEC, ENABLE);       //ʹ��RTC���ж�
        RTC_WaitForLastTask();                  //�ȴ�RTC�Ĵ���д�������
        RTC_ITConfig(RTC_IT_ALR, ENABLE);
        RTC_WaitForLastTask();                  //�ȴ�RTC�Ĵ���д�������
        RTC_SetPrescaler(32767);                //����RTCԤ��Ƶ��ֵ����1���źż��㹫ʽ fTR_CLK = fRTCCLK/(PRL+1)
        RTC_WaitForLastTask();   
    }
    else//���ⲿ����
    {
    	Debug("!!!RTC LSE NO WORK!!!\n\r");
        rtcintcnt=0x200000;    
        RCC_HSEConfig(RCC_HSE_ON);/* Enable HSE */
        while ( (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) && (--rtcintcnt) );/* Wait till HSE is ready */
        if ( rtcintcnt == 0 )
        {
            return;
        }
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);/* Select HSE/128 as RTC Clock Source */
    
	    RCC_RTCCLKCmd(ENABLE);/* Enable RTC Clock */    
	    RTC_WaitForSynchro();/* Wait for RTC registers synchronization */
	    RTC_WaitForLastTask();/* Wait until last write operation on RTC registers has finished */
	    
	    /* Set RTC prescaler: set RTC period to 1sec */
	    RTC_SetPrescaler(8000000/128-1); /* RTC period = RTCCLK/RTC_PR = (8MHz/128)/(8000000/128) */
	    
	    RTC_WaitForLastTask();/* Wait until last write operation on RTC registers has finished */
    }
}

/*******************************************************************************
* Function Name  : RTC_Config
* Description    : �ϵ�ʱ���ñ��������Զ�����Ƿ���ҪRTC��ʼ���� 
*                       ����Ҫ���³�ʼ��RTC�������RTC_Configuration()�����Ӧ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RtcSetUp(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	RTC_TIME NowTime;

	/* ʹ��rtc�ж� */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//��鱸�üĴ�����ֵ�Ƿ�ı��ˣ�����ı��ˣ�˵��battery�����ˣ���Ҫ��������rtc
	if(BKP_ReadBackupRegister(BKP_RTC_FLAG) != RTC_RST_FLAG)
	{
		/* Backup data register value is not correct or not yet programmed (when
		 	the first time the program is executed) */
		Debug("RTC need configure!\n\r");
		
		/* RTC Configuration */
		RtcConfiguration();

		NowTime.year=2012;
		NowTime.mon=1;
		NowTime.day=1;
		NowTime.hour=0;
		NowTime.min=0;
		NowTime.sec=30;
		if(RtcAdjustTime(&NowTime,RtcOp_SetTime)==TRUE)
		{
			Debug("RTC set sucess!\n\r");
		}
		else
		{
			Debug("RTC set error!\n\r");
		}

/*
		NowTime.sec=0;
		if(RtcAdjustTime(&NowTime,RtcOp_SetAlarm)==TRUE)
		{
			Debug("Alarm set sucess!\n\r");
		}
		else
		{
			Debug("Alarm set error!\n\r");
		}		
*/
		//���üĴ���д��һ��ֵ�����´���������
		BKP_WriteBackupRegister(BKP_RTC_FLAG, RTC_RST_FLAG);  
	}
	else
	{
		if(RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)//����Ƿ��ϵ縴λ
	    {
	      Debug("System power on reset!\n\r");
	    }
	    else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)//����Ƿ��ֶ���λ
	    {
	      Debug("System reset!\n\r");
	    }	   
	    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	    PWR_BackupAccessCmd(ENABLE);	
	    
		Debug("No need to configure RTC....\n\r");
		
		RTC_WaitForSynchro();//�ȴ�RTC�Ĵ�����APBʱ��ͬ��
	    RTC_ITConfig(RTC_IT_SEC, ENABLE);//ʹ��RTC���ж�
	    RTC_WaitForLastTask();//�ȴ�д�������
	    RTC_ITConfig(RTC_IT_ALR, ENABLE);
	    RTC_WaitForLastTask();//�ȴ�д�������
	}
	RCC_ClearFlag();//�����λ��־
	
	RTC_WaitForLastTask();

	Debug("RTC configured finished\n\r");
	RTC_WaitForLastTask();

	RtcReadTime(&NowTime,RtcOp_GetAlarm);
	Debug("Alarm:%d.%d.%d %d:%d:%d\n\r",NowTime.year,NowTime.mon,NowTime.day,
		NowTime.hour,NowTime.min,NowTime.sec);
}

void RtcSetUp2(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	RTC_TIME NowTime;

	/* ʹ��rtc�ж� */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/*PWRʱ�ӣ���Դ���ƣ���BKPʱ�ӣ�RTC�󱸼Ĵ�����ʹ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/*ʹ��RTC�ͺ󱸼Ĵ�������*/
	PWR_BackupAccessCmd(ENABLE);

	/*��ָ���ĺ󱸼Ĵ�����BKP_DR1���ж�������*/
	if(BKP_ReadBackupRegister(BKP_RTC_FLAG) != RTC_RST_FLAG)
	{
		/* ������BKP��ȫ���Ĵ�������Ϊȱʡֵ */
		BKP_DeInit(); 

		/* ���� LSE���ⲿ���پ���*/
		RCC_LSEConfig(RCC_LSE_ON); 
		/*�ȴ��ⲿ������ ��Ҫ�ȴ��Ƚϳ���ʱ��*/
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

		/*ʹ���ⲿ����32.768K��ΪRTCʱ��*/
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);  
		RCC_RTCCLKCmd(ENABLE);
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();

		//����RTC�����ж�(���������жϺ�����жϿ�����)
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		RTC_WaitForLastTask();
		RTC_ITConfig(RTC_IT_ALR, ENABLE);
		RTC_WaitForLastTask();
		
		//32768����Ԥ��Ƶֵ��32767,����һ����˵���񶼲���ô׼
		RTC_SetPrescaler(32767);  //�����ҪУ׼����,���޸Ĵ˷�Ƶֵ
		RTC_WaitForLastTask();

		//д��RTC�󱸼Ĵ���1 0xa5a5
		BKP_WriteBackupRegister(BKP_RTC_FLAG, RTC_RST_FLAG);  
		//RTC_Blank=1; /*�����־����RTC��û��Ԥ���(����˵��û����Ŧ�۵��) �ô���ѽɶ���������*/
	}
	//���RTC�Ѿ�����
	else
	{
		//�ȴ�RTC��APBͬ��
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();

		//ʹ�����ж� 
		RTC_ITConfig(RTC_IT_SEC, ENABLE);  //�����Էŵ�ǰ����
		RTC_WaitForLastTask();//�ȴ�д�������
	    RTC_ITConfig(RTC_IT_ALR, ENABLE);
		RTC_WaitForLastTask(); //�ֵ�....
	 }

	//�����־
	RCC_ClearFlag(); 

	RtcReadTime(&NowTime,RtcOp_GetAlarm);
	Debug("Alarm:%d.%d.%d %d:%d:%d\n\r",NowTime.year,NowTime.mon,NowTime.day,
		NowTime.hour,NowTime.min,NowTime.sec);
}

static uint32_t RTC_GetAlarmCount(void)
{
  uint16_t tmp = 0;
  tmp = RTC->ALRL;
  return (((uint32_t)RTC->ALRH << 16 ) | tmp) ;
}

//��ȡ��ǰʱ��ĺ���
//ֻ��Ҫ����һ���ṹ��ʵ�壬����ַ�����pTime���Ϳ��Եõ���ǰ������ʱ��
//�㷨�ȽϷ�����ѧϰ����Ҫ���Ŀ�
//��arm�����������Ҫ�Ƚϳ���ʱ�䣬���Ծ����������ó������෨
void RtcReadTime(RTC_TIME *pTime,RTC_OPERATE Op)
{ 
	u32 TimeCnt=0;

	RTC_WaitForLastTask();
	if(Op==RtcOp_GetAlarm) TimeCnt = RTC_GetAlarmCount();//��ȡ��ǰ����ֵ
	else TimeCnt = RTC_GetCounter();//��ȡ��ǰ��ʱ��ֵ
	RTC_WaitForLastTask();

	//Debug("RTC READ CNT:%u\n\r",TimeCnt);
	RtcCnt2Time(TimeCnt,pTime);
}

//ʱ���������
//���������ýṹ���ַ������Ҫ�趨����ֵ��ע��Ҫ����ṹ��ʵ�壡
bool RtcAdjustTime(RTC_TIME *pTime,RTC_OPERATE Op)
{
	u32 TimeCnt;

	TimeCnt=RtcTime2Cnt(pTime);

	if(TimeCnt == 0) return FALSE;
	
	switch(Op)
	{	
		case RtcOp_SetTime:
			RTC_WaitForLastTask(); 
			RTC_SetCounter(TimeCnt);	
			RTC_WaitForLastTask();

			RTC_WaitForLastTask();

			RTC_WaitForLastTask();
			break;
		case RtcOp_SetAlarm:
			RTC_WaitForLastTask(); 
			RTC_SetAlarm(TimeCnt);
			RTC_WaitForLastTask();
			break;
	}
	
	return TRUE;
}



