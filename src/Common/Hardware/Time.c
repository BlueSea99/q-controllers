//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ���װ��time�����Ͷ�ʱ���⣬�ɱ���������������stm32��Ŀ�����ٴ��뿪����
*/
//------------------------------------------------------------------//
#include "Drivers.h"

void Tim1_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Tim2_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Tim3_Init(void)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Tim4_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//uS_Base ��ʾ��λ��Ϊ1ʱ����λ��us��Ϊ100ʱ����λ��100us����Сֵ1�����ֵ900
//���ն�ʱֵ= Val x uS_Base x 1us
//�¶�ʱ�趨�Ḳ�Ǿ��趨
//AutoReload�����趨��һ�ζ�ʱ����ѭ����ʱ
//val��uS_Base��������һ��Ϊ0����ֹͣ��ǰ��ʱ��
//val����Ϊ1������tim������
void Tim1_Set(u16 Val,u16 uS_Base,bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM1, DISABLE);

    if(Val==0||uS_Base==0) return;

	if(Val==1)//���val����Ϊ1������
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base>900) uS_Base=900;
    
    //��ʱƵ��Ϊ�� 72M/(Ԥ��Ƶ+1)/Ԥװ��
    RCC_GetClocksFreq(&RCC_Clocks);//��ȡϵͳƵ��
    TIM_TimeBaseStructure.TIM_Period        = (Val-1);//��װ��������ֵʱ��cnt��1
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.PCLK1_Frequency*2/1000000)*uS_Base - 1);//tim1ʹ��APB2��72mʱ�ӣ�������tim��һ��

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM1,TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM1,TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM1, TIM_IT_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);/* ʹ��TIM1�� ������� �ж� */    

	TIM_Cmd(TIM1, ENABLE);/* ʹ��TIM1 */	 
}

//uS_Base ��ʾ��λ��Ϊ1ʱ����λ��us��Ϊ100ʱ����λ��100us����Сֵ1�����ֵ900
//���ն�ʱֵ= Val x uS_Base x 1us
//�¶�ʱ�趨�Ḳ�Ǿ��趨
//AutoReload�����趨��һ�ζ�ʱ����ѭ����ʱ
//val��uS_Base��������һ��Ϊ0����ֹͣ��ǰ��ʱ��
//val����Ϊ1������tim������
void Tim2_Set(u16 Val,u16 uS_Base,bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM2, DISABLE);

    if(Val==0||uS_Base==0) return;

	if(Val==1)//���val����Ϊ1������
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base>900) uS_Base=900;
    
    //��ʱƵ��Ϊ�� 72M/(Ԥ��Ƶ+1)/Ԥװ��
    RCC_GetClocksFreq(&RCC_Clocks);//��ȡϵͳƵ��
    TIM_TimeBaseStructure.TIM_Period        = (Val-1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.PCLK1_Frequency*2/1000000)*uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM2,TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM2,TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM2, TIM_IT_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);/* ʹ��TIM2�� ������� �ж� */    

	TIM_Cmd(TIM2, ENABLE);/* ʹ��TIM2 */	 
}

void Tim3_Set(u16 Val, u16 uS_Base, bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM3, DISABLE);

    if((Val == 0) || (uS_Base == 0)) return;

	if(Val==1)//���val����Ϊ1������
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base > 900) uS_Base = 900;
    
    //��ʱƵ��Ϊ�� 72M/(Ԥ��Ƶ+1)/Ԥװ��
    RCC_GetClocksFreq(&RCC_Clocks);//��ȡϵͳƵ��
    TIM_TimeBaseStructure.TIM_Period        = (Val - 1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.PCLK1_Frequency*2 / 1000000) * uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);/* ʹ��TIM3�� ������� �ж� */    

	TIM_Cmd(TIM3, ENABLE);/* ʹ��TIM3 */	 
}

void Tim4_Set(u16 Val, u16 uS_Base, bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM4, DISABLE);

    if(Val == 0||uS_Base == 0) return;
	
	if(Val==1)//���val����Ϊ1������
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base > 900) uS_Base = 900;
    
    //��ʱƵ��Ϊ�� 72M/(Ԥ��Ƶ+1)/Ԥװ��
    RCC_GetClocksFreq(&RCC_Clocks);//��ȡϵͳƵ��
    TIM_TimeBaseStructure.TIM_Period        = (Val - 1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.PCLK1_Frequency*2 / 1000000) * uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM4, TIM_IT_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);/* ʹ��TIM4�� ������� �ж� */    

	TIM_Cmd(TIM4, ENABLE);/* ʹ��TIM4 */	 
}

//PA6: TIM3_CH1
//ռ�ձ�=Pluse/Val
//����=Val*uS_Base
void IO7_PWM_CONFIG(u16 Val, u16 uS_Base,u16 Pluse)
{
	static u16 Old_Val=0;
	static u16 Old_uS_Base=0;
	static u16 Old_Pluse=0;
	
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;  
    TIM_OCInitTypeDef  TIM_OCInitStructure;  
    RCC_ClocksTypeDef RCC_Clocks;

	if(Old_Val==Val && Old_uS_Base==uS_Base && Old_Pluse==Pluse)//�����ظ�����
	{
		return;
	}
	else
	{
		Old_Val=Val;
		Old_uS_Base=uS_Base;
		Old_Pluse=Pluse;
	}
	Debug("IO7 %u %u %u\n\r",Val,Pluse,uS_Base);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  
    TIM_DeInit(TIM3);

    if(Val && uS_Base && Pluse)//������Чֵ
	{ 
		if(Pluse>=Val)//�ø�
		{
			GPIO_InitTypeDef GPIO_InitStructure; 

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

			GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6; 
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
			GPIO_Init(GPIOA, &GPIO_InitStructure); 
			GPIO_SetBits(GPIOA,GPIO_Pin_6);

			return;
		}
		else//pwm
		{
			GPIO_InitTypeDef GPIO_InitStructure; 

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

			GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6; 
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;           // ����������� 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
			GPIO_Init(GPIOA, &GPIO_InitStructure); 
		}
	} 
	else //�õ�
	{
		GPIO_InitTypeDef GPIO_InitStructure; 

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOA, &GPIO_InitStructure); 
		GPIO_ResetBits(GPIOA,GPIO_Pin_6);

		return;
	}
    
    if(Val == 0 || uS_Base == 0 || Pluse==0) return;

	if(Val==1)//���val����Ϊ1������
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base > 900) uS_Base = 900;    

    RCC_GetClocksFreq(&RCC_Clocks);//��ȡϵͳƵ��
    
    TIM_TimeBaseStructure.TIM_Period = (Val-1);//��װ��������ֵʱ��cnt��1
    TIM_TimeBaseStructure.TIM_Prescaler = ((RCC_Clocks.PCLK1_Frequency*2/1000000)*uS_Base - 1);//tim1ʹ��APB2��72mʱ�ӣ�������tim��һ��

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;                                //����ʱ�ӷ�Ƶϵ��������Ƶ  
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;                 //���ϼ������ģʽ  
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);  
    
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;                           //����ΪPWMģʽ1  
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;                
    TIM_OCInitStructure.TIM_Pulse = Pluse;                                       //��������ֵ�������������������ֵʱ����ƽ��������  
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;                    //����ʱ������ֵС��CCR1ʱΪ�ߵ�ƽ  
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;     //MOE=0 ���� TIM1����ȽϿ���״̬
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);                                    //ʹ��ͨ��1      
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  

    TIM_Cmd(TIM3,ENABLE);                                                      //ʹ��TIM3      
    TIM_CtrlPWMOutputs(TIM3, ENABLE);
}

//PA7: TIM1_CH1N
//ռ�ձ�=Pluse/Val
//����=Val*uS_Base
void IO8_PWM_CONFIG(u16 Val, u16 uS_Base,u16 Pluse)
{   
	static u16 Old_Val=0;
	static u16 Old_uS_Base=0;
	static u16 Old_Pluse=0;
	
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    RCC_ClocksTypeDef RCC_Clocks;

	if(Old_Val==Val && Old_uS_Base==uS_Base && Old_Pluse==Pluse)//�����ظ�����
	{
		return;
	}
	else
	{
		Old_Val=Val;
		Old_uS_Base=uS_Base;
		Old_Pluse=Pluse;
	}
   	Debug("IO8 %u %u %u\n\r",Val,Pluse,uS_Base);
   	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);  
    TIM_DeInit(TIM1);

	if(Val && uS_Base && Pluse)
	{ 
		if(Pluse>=Val)//�ø�
		{
			GPIO_InitTypeDef GPIO_InitStructure; 

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

			GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7; 
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
			GPIO_Init(GPIOA, &GPIO_InitStructure); 
			GPIO_SetBits(GPIOA,GPIO_Pin_7);

			return;
		}
		else//pwm
		{
			GPIO_InitTypeDef GPIO_InitStructure; 

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

			GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7; 
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;           // ����������� 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
			GPIO_Init(GPIOA, &GPIO_InitStructure); 
		}
	} 
	else//�õ�
	{ 
		GPIO_InitTypeDef GPIO_InitStructure; 

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOA, &GPIO_InitStructure); 
		GPIO_ResetBits(GPIOA,GPIO_Pin_7);

		return;
	}
  
    if(Val == 0 || uS_Base == 0 || Pluse==0) return;

	if(Val==1)//���val����Ϊ1������
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base > 900) uS_Base = 900;    

    RCC_GetClocksFreq(&RCC_Clocks);//��ȡϵͳƵ��
    
    TIM_TimeBaseStructure.TIM_Period = (Val-1);//��װ��������ֵʱ��cnt��1
    TIM_TimeBaseStructure.TIM_Prescaler = ((RCC_Clocks.PCLK1_Frequency*2/1000000)*uS_Base - 1);//tim1ʹ��APB2��72mʱ�ӣ�������tim��һ��
    
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;        //����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //������ģʽ  /* ���ϼ���ģʽ */  
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;        //���� ���� ����ֵ
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;           //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;         //ʹ������Ƚ�״̬
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; //ʹ��  ���� ���״̬
    TIM_OCInitStructure.TIM_Pulse = Pluse;                //���ô�װ�벶��ȽϼĴ���������ֵ         ռ��ʱ�� 
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;       //�������:TIM����Ƚϼ��Ե�
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;     //���� ������Ը�
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;     //MOE=0 ���� TIM1����ȽϿ���״̬
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;  //MOE=0 ���� TIM1����ȽϿ���״̬
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);        //�趨�õĲ��� ��ʼ��TIM  
    TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable);

    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}


