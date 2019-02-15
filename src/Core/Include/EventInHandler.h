#ifndef __EVENT_IN_HANDLER_H__
#define __EVENT_IN_HANDLER_H__


typedef enum{
	EBF_NULL=0,

	//�����¼����û�������������
	EBF_NEXT_QUICK_FUNC,
	
	//Ӳ���ж��¼�
	EBF_KEY,//key input
	EBF_PIO_IN,//io input
	EBF_SEC_FUNC,
	EBF_30SEC,
	EBF_TIM2,
	EBF_TIM4,
	EBF_IR,

	//ϵͳ�¼�
	EBF_IDLE,
	EBF_INIT,
	
	//���ݴ���
	EBF_Q_COM_CMD,//��qwifi���ӵĴ������������
	EBF_QWIFI_STATE,//
	EBF_QWIFI_KEY,//
	EBF_QWIFI_VAR,//
	EBF_QWIFI_MSG,//
	EBF_QWIFI_READ_VAR_RET,
	EBF_QWIFI_READ_VAR_ERROR,
	EBF_QWIFI_SET_VAR_RET,
	EBF_QWIFI_SET_VAR_ERROR,
	EBF_QWIFI_MSG_RET,//
	EBF_QWIFI_STR_RET,//


	
	//�ڲ�����
	EBF_SYS_CMD,//ϵͳ��������

	//�û��Զ����¼�
	EBF_USER_EVT1,
	EBF_USER_EVT2,
	EBF_USER_EVT3,
	EBF_USER_EVT4,









	
	//ϵͳ����
	EBF_NEXT_LOOP_FUNC,//�¸�ѭ������
	
	EBF_MAX
}EVENT_BIT_FLAG;

typedef enum{
	EFR_OK=0,
	EFR_STOP,//�ص�������ش˽�������¼����ٷ��������������

	EFR_MAX
}EVENT_HANDLER_RESUTL;

typedef EVENT_HANDLER_RESUTL (*pEvtFunc)(EVENT_BIT_FLAG,int,void *);
typedef struct{
	EVENT_BIT_FLAG Event;
	pEvtFunc EvtFunc;
}EVENT_FUNC_ITEM;


void EventDebug(void);
void SetEventFlag(EVENT_BIT_FLAG BitFlag);
void SendEvent(EVENT_BIT_FLAG BitFlag,s32 S32Param,void *pParam);
void CleanAllEvent(void);
void *WaitEvent(EVENT_BIT_FLAG *pEvent,s32 *pS32);
bool CheckEventFinished(EVENT_BIT_FLAG Event);


#endif

