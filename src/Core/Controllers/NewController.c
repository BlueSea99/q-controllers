#include "SysDefines.h"
#include "Product.h"

//�������¼�������
static EVENT_HANDLER_RESUTL Init_EF(int a,void *p)
{
	Debug("InitEF\n\r");

	return EFR_OK;
}

//ϵͳwhile(1)�յ�ʱѭ�����õ��¼�
static EVENT_HANDLER_RESUTL Idle_EF(int a,void *p)
{

	return EFR_OK;
}

//���������²��ɿ�����¼�
static EVENT_HANDLER_RESUTL KeyHandler_EF(int a,void *p)
{
	u16 KeyIo=a&0xffff;
	u16 Ms=a>>16;

	Debug("New Key%u %umS\n\r",KeyIo,Ms);
	SendEvent(EBF_USER_EVT1,0,NULL);

	return EFR_OK;
}

//�û��Զ����¼�
static EVENT_HANDLER_RESUTL Evt1_EF(int a,void *p)
{
	Debug("New Event 1\n\r");

	return EFR_OK;
}

//�����������ע��
static const EVENT_FUNC_ITEM gNewController[]={
{EBF_INIT,Init_EF},
{EBF_IDLE,Idle_EF},
{EBF_KEY,KeyHandler_EF},
{EBF_USER_EVT1,Evt1_EF},




{EBF_NULL,NULL}//����������һ���Դ˽�β
};

void NewControllerReg(void)
{
	EventControllerRegister(gNewController,"My New Controller");
}

