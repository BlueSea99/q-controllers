//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�

Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��

���л��ڿ�������ƽ̨���еĿ�����������Ʒ��������ϵ�����Ŷӣ���ѷ�����
��������Ƶ��q-iot.cn�����д������г���������Ӧ��������۳�˰�Ѽ�ά�����ú�
��ȫ���ṩ�������ߣ��Դ˹������ڿ�Դ��ҵ��

By Karlno ����Ƽ�

���ļ�����Ŀ�������Ҫ��������ģ�壬���û��붨��һ��ҵ�����ʱ������
ֱ�ӿ������ļ����޸Ŀ����������к���������
*/
//------------------------------------------------------------------//

#include "SysDefines.h"
#include "Product.h"

//�������¼�������
static EVENT_HANDLER_RESUTL Init_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	Debug("Test InitEF\n\r");

	return EFR_OK;
}

static EVENT_HANDLER_RESUTL Evt1_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	Debug("Test Event 1\n\r");

	return EFR_OK;
}

//�����������ע��
static const EVENT_FUNC_ITEM gTestController[]={
{EBF_INIT,Init_EF},
{EBF_USER_EVT1,Evt1_EF},




{EBF_NULL,NULL}//����������һ���Դ˽�β
};

void TestControllerReg(void)
{
	ControllerRegister(gTestController,"My Test Controller");
}

