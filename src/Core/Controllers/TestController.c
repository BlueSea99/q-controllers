#include "SysDefines.h"
#include "Product.h"

//�������¼�������
static EVENT_HANDLER_RESUTL Init_EF(int a,void *p)
{
	Debug("Test InitEF\n\r");

	return EFR_OK;
}

static EVENT_HANDLER_RESUTL Evt1_EF(int a,void *p)
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
	EventControllerRegister(gTestController,"My Test Controller");
}

