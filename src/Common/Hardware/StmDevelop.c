//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ���������stm32Ӳ����صĺ�������ֲQ-Ctrlʱ�˲�������д
*/
//------------------------------------------------------------------//
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

//��ȡӲ��ΨһID
u32 GetHwID(u8 *pID)
{
	static u32 HwID=0;
	u8 i;

	if(pID==NULL)
	{
		if(HwID==0) HwID=MakeHash33((u8 *)0x1FFFF7E8,12);
	}
	else
	{
		HwID=MakeHash33((u8 *)0x1FFFF7E8,12);
	    for(i=0;i<12;i++) pID[i]=*(u8 *)(0x1FFFF7E8+i);
	}
	
	return HwID;
}

#define MAX_SYSCALL_INTERRUPT_PRIORITY 	(1<<4) 

__asm u32 SaveCpuStatus(void)
{
#if 0
	PRESERVE8

	push { r0 }
	mov r0, #MAX_SYSCALL_INTERRUPT_PRIORITY
	msr basepri, r0
	pop { r0 }
	bx r14
#else 
    MRS     R0, PRIMASK 
    CPSID   I
    BX      LR
#endif
}

__asm void RestoreCpuStatus(u32 cpu_sr)
{
#if 0
	PRESERVE8

	push { r0 }
	mov r0, #0
	msr basepri, r0
	pop { r0 }
	bx r14
#else
    MSR     PRIMASK, R0
    BX      LR
#endif
}





