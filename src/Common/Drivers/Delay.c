#include "SysDefines.h"

volatile u32 gSysTick;
volatile u32 gRtcMonitorCnt=100;//ģ��rtc����Ǩ��

void DelayMs(u32 nTime)
{  
	u32 TimeStamp=gSysTick;
	
	while(((gSysTick-TimeStamp)*SYS_TICK_PERIOD_MS)<nTime);  //�ȴ�����
}

u32 GetSysTick(void)
{
	return gSysTick;
}

u32 GetSysStartMs(void)
{
	return gSysTick*SYS_TICK_PERIOD_MS;
}

u32 GetRtcCount(void)
{
	return gRtcMonitorCnt;
}

static u32 SysTick_Setup(u32 ticks)
{ 
  if (ticks > SysTick_LOAD_RELOAD_Msk)  return (1);            /* Reload value impossible */
                                                               
  SysTick->LOAD  = (ticks & SysTick_LOAD_RELOAD_Msk) - 1;      /* set reload register */
  NVIC_SetPriority(SysTick_IRQn,SYSTICK_INT_Priority);  //����ϵͳ�δ�Ϊ��߶�ʱ��
  SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | 
                   SysTick_CTRL_TICKINT_Msk   | 
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
  return (0);                                                  /* Function successful */
}

//ֹͣϵͳ�δ�
void SysTick_Stop(void)
{
	SysTick->CTRL&=(~SysTick_CTRL_ENABLE_Msk);
}

void SysTick_Init(void)
{
	//����Ϊ72000����1ms,720000����10ms,�Դ�����
	SysTick_Setup(SystemFrequency * SYS_TICK_PERIOD_MS / 1000);
}

