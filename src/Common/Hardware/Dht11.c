//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ���װ��dht11�������ɱ���������������stm32��Ŀ�����ٴ��뿪����
*/
//------------------------------------------------------------------//

#include "SysDefines.h"

#define DHT_GPIO_RCC RCC_APB2Periph_GPIOA 
#define DHT_GPIO_GROUP  GPIOA
#define DHT_GPIO_PIN GPIO_Pin_0

#define DHT_PIN_1() GPIO_SetBits(DHT_GPIO_GROUP,DHT_GPIO_PIN)
#define DHT_PIN_0() GPIO_ResetBits(DHT_GPIO_GROUP,DHT_GPIO_PIN)
#define DHT_PIN_READ() GPIO_ReadInputDataBit(DHT_GPIO_GROUP,DHT_GPIO_PIN)

static void Dht_Input(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_InitStructure.GPIO_Pin = DHT_GPIO_PIN;    
	GPIO_Init(DHT_GPIO_GROUP, &GPIO_InitStructure);	
}

static void Dht_Output(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_InitStructure.GPIO_Pin = DHT_GPIO_PIN;    
	GPIO_Init(DHT_GPIO_GROUP, &GPIO_InitStructure);
	DHT_PIN_1();
}

#define DHT_WAIT_CNT 200
#define DHT_HIGH_PLUSE_CNT 100
bool Dht_Read(u16 *pTemp,u16 *pHumidity)
{
	//static bool First=TRUE;
	u32 i=0;
	u8 Sum=0,Idx=0;
	u32 Data=0;
	u8 *pBytes=(void *)&Data;
	bool Ret=FALSE;

	/*if(First)
	{
		RCC_AHB1PeriphClockCmd(DHT_GPIO_RCC, ENABLE);
		First=FALSE;
	}*/
	
	if(pTemp==NULL || pHumidity==NULL) return FALSE;
	*pTemp=0;
	*pHumidity=0;
	
	Dht_Output();
	DHT_PIN_0();
	DelayMs(20);//��ʼ�źű������18ms
	DHT_PIN_1();

	//OS_EnterCritical();
	Dht_Input();
	i=0;
	while(DHT_PIN_READ()==1) //�ȴ��͵�ƽ��Ӧ�źţ�80us��ʱ�˳�
		if(i++>DHT_WAIT_CNT) goto over;//ÿus��i����6.5����

	i=0;
	while(DHT_PIN_READ()==0)//�ȴ���Ӧ�źŽ���
		if(i++>DHT_WAIT_CNT) goto over;

	i=0;	
	while(DHT_PIN_READ()==1)//�ȴ����ݵ͵�ƽ
		if(i++>DHT_WAIT_CNT) goto over;
		
	//��ʼ��ȡ���ݣ���ʱ�Ǹߵ�ƽ
	for(Idx=0;Idx<40;Idx++)
	{
		i=0;
		while(DHT_PIN_READ()==0) if(i++>DHT_WAIT_CNT) goto over;//�ȴ����ݸߵ�ƽ

		i=0;
		while(DHT_PIN_READ()==1)//����ߵ�ƽ��ʱ����28us����70us 
		{
			if(i++>DHT_WAIT_CNT)
			{
				goto over;
			}
		}

		if(i>DHT_HIGH_PLUSE_CNT)
		{
			if(Idx<32) SetBit(Data,31-Idx);
			else SetBit(Sum,7-(Idx-32));
		}
	}

	i=0;
	while(DHT_PIN_READ()==0) if(i++>DHT_WAIT_CNT) goto over;//�ȴ��ߵ�ƽ,�ȴ�50us�����źŽ���

	//����У���
	if(pBytes[0]+pBytes[1]+pBytes[2]+pBytes[3]==Sum)
	{
		*pTemp=LBit16(Data)>>8;
		*pHumidity=HBit16(Data)>>8;
		Ret=TRUE;
	}
	
over:
	//OS_ExitCritical();
	Dht_Output();

	/*if(Ret) 
	{
		Debug("%x %x %x %x = %x\n\r",pBytes[0],pBytes[1],pBytes[2],pBytes[3],Sum);
		Debug("DHT:%u.%u %u.%u\n\r",HBit8(*pTemp),LBit8(*pTemp),HBit8(*pHumidity),LBit8(*pHumidity));	
	}*/
	
	return Ret;
}



