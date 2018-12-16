#ifndef EVENT_IN_HANDLER_H
#define EVENT_IN_HANDLER_H


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

	//ϵͳ�¼�
	EBF_IDLE,
	EBF_INIT,
	
	//���ݴ���
	EBF_USER_COM_CMD,//�û��������������
	
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

typedef EVENT_HANDLER_RESUTL (*pEvtFunc)(int,void *);
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
void EventControllerRegister(const EVENT_FUNC_ITEM *pItemArray,const char *pName);
void EventControllerPost(EVENT_BIT_FLAG Event,int Param,void *p);


#endif

