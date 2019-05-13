#include "Drivers.h"

#if 1//��ʱ��+io����ʽ
#define IrPluseGen_ISR TIM3_IRQHandler
#define IrPluseTimerSet Tim3_Set
#define IrPluseTimerID TIM3

//���÷��ͻ���ֹͣ����38k����
//Val����ʱ����ir
void SetSendIrData(u8 Val) 
{
	IrPluseTimerSet((Val)?13:0,1,TRUE);
	IOOUT_SetIoStatus(IOOUT_IR_OUT,(Val)?TRUE:FALSE);
}

//����ʱ����38k�ز�
void IrPluseGen_ISR(void)
{
	if(TIM_GetITStatus(IrPluseTimerID, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(IrPluseTimerID, TIM_IT_Update);

		if(IOOUT_ReadIoStatus(IOOUT_IR_OUT))//�����Ǹߵ�ƽ
			IOOUT_SetIoStatus(IOOUT_IR_OUT,FALSE);
		else
			IOOUT_SetIoStatus(IOOUT_IR_OUT,TRUE);
	}
}
#else//pwm��ʽ
//���÷��ͻ���ֹͣ����38k����
//Val����ʱ����ir
void SetSendIrData(u8 Val) 
{
	PWM1_CONFIG((Val)?26:0,1,13);//pa6,tim3
}
#endif

