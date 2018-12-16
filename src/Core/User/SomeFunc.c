#include "SysDefines.h"

void IWDG_Configuration(void)
{
	/* д��0x5555,�����������Ĵ���д�빦�� */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* ����ʱ�ӷ�Ƶ,40K/256=156HZ(6.4ms)*/
	IWDG_SetPrescaler(IWDG_Prescaler_256);

	/* ι��ʱ�� 1s/6.4MS=156 .ע�ⲻ�ܴ���0xfff*/
	IWDG_SetReload(156*5);

	/* ι��*/
	IWDG_ReloadCounter();

	/* ʹ�ܹ���*/
	IWDG_Enable();
}

void IWDG_PeriodCB(void)
{
	IWDG_ReloadCounter();
}

//�����忨
void RebootBoard(void) 
{
	SaveCpuStatus();//�ر������ж�

	SCB->AIRCR  = ((0x05FA << SCB_AIRCR_VECTKEY_Pos) | 
	               (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) | 
	               SCB_AIRCR_SYSRESETREQ_Msk);              
	__DSB();                                                      

	while(1);
}

void DefaultConfig(void)
{
	//RFS_BurnDefaultToRom();
	RebootBoard();
}

