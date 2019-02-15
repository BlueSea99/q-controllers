//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ����������¼���صĻ���
*/
//------------------------------------------------------------------//
#include "SysDefines.h"

typedef struct{
	EVENT_BIT_FLAG Event;
	s32 Param;
	void *pParam;
	void *pNext;
}EVENT_ITEM;
static volatile EVENT_ITEM *gpEventItems=NULL;
static volatile EVENT_ITEM *gpEventItemsLast=NULL;
static EVENT_BIT_FLAG gCurrentEvent;//��¼��ǰ������¼�

void EventDebug(void)
{
	volatile EVENT_ITEM *pHandler=gpEventItems;

	while(pHandler!=NULL)
	{
		Debug("Event[%u] p=%u,0x%x | %x -> %x\n\r",pHandler->Event,pHandler->Param,pHandler->pParam,(u32)pHandler,(u32)pHandler->pNext);
		pHandler=pHandler->pNext;
	}

	Debug("Last:0x%x\n\r",(u32)gpEventItemsLast);
}

//���۶��ٴΣ�������һ�δ���
void SetEventFlag(EVENT_BIT_FLAG Event)
{
	//Debug("F%u ",Event);
	if(Event>(u32)EBF_NULL && Event<(u32)EBF_MAX) 
	{
		EVENT_ITEM *p=Q_Malloc(sizeof(EVENT_ITEM));
		IntSaveInit();
		
		p->Event=Event;
		p->Param=0;
		p->pParam=NULL;
		p->pNext=NULL;

		EnterCritical();
		if(gpEventItems==NULL)
		{
			gpEventItems=p;
			gpEventItemsLast=p;
		}
		else
		{
			volatile EVENT_ITEM *pItem=gpEventItems;
			
			//������û��һ���ģ�����
			while(pItem!=NULL)
			{
				if(pItem->Event==Event)
				{
					pItem->Param=0;
					pItem->pParam=NULL;
					Q_Free(p);
					LeaveCritical();
					return;
				}
				pItem=pItem->pNext;
			}

			//û��һ���ģ������µ�
			gpEventItemsLast->pNext=(void *)p;
			gpEventItemsLast=p;
		}
		LeaveCritical();
	}
}

//����һ�Σ�����һ���¼�����
void SendEvent(EVENT_BIT_FLAG Event,s32 S32Param,void *pParam)
{
	//Debug("E%u ",Event);
	if(Event>(u32)EBF_NULL && Event<(u32)EBF_MAX) 
	{
		volatile EVENT_ITEM *p=Q_Malloc(sizeof(EVENT_ITEM));//��WaitEvent���ͷ�
		IntSaveInit();
		
		p->Event=Event;
		p->Param=S32Param;
		p->pParam=pParam;
		p->pNext=NULL;

		EnterCritical();
		if(gpEventItems==NULL)
		{
			gpEventItems=p;
			gpEventItemsLast=p;
		}
		else
		{
			gpEventItemsLast->pNext=(void *)p;
			gpEventItemsLast=p;
		}
		LeaveCritical();
	}
}

void CleanAllEvent(void)
{
	IntSaveInit();
	EnterCritical();
	while(gpEventItems!=NULL)
	{
		volatile EVENT_ITEM *p=gpEventItems->pNext;
		Q_Free((void *)gpEventItems);
		gpEventItems=p;
	}
	gpEventItemsLast=NULL;
	LeaveCritical();
}

//�ȴ��¼�
void *WaitEvent(EVENT_BIT_FLAG *pEvent,s32 *pS32)
{
	void *pRet=NULL;
	IntSaveInit();

	gCurrentEvent=EBF_NULL;//��wait event��ʾǰ����¼��Ѿ���������
	
	while(gpEventItems==NULL)//wait data in flag.����Ҳ��û�£�����������
	{
		ControllerEvtPost(EBF_IDLE,0,NULL);
	}

	EnterCritical();
	gCurrentEvent=gpEventItems->Event;//��¼��ǰִ�е��¼�
	*pEvent=gpEventItems->Event;
	*pS32=gpEventItems->Param;
	pRet=gpEventItems->pParam;

	if(gpEventItems==gpEventItemsLast)
	{
		Q_Free((void *)gpEventItems);
		gpEventItems=gpEventItemsLast=NULL;
	}
	else
	{
		volatile EVENT_ITEM *p=gpEventItems->pNext;
		Q_Free((void *)gpEventItems);
		gpEventItems=p;	
	}
	LeaveCritical();

	return pRet;
}

//����¼�������û����ɣ��������ˣ�����true
bool CheckEventFinished(EVENT_BIT_FLAG Event)
{
	bool Ret=TRUE;
	IntSaveInit();
	
	if(gCurrentEvent==Event) return FALSE;

	EnterCritical();
	{
		volatile EVENT_ITEM *pItem=gpEventItems;
		
		//������û��һ����
		while(pItem!=NULL)
		{
			if(pItem->Event==Event)
			{
				Ret=FALSE;
				break;
			}
			pItem=pItem->pNext;
		}
	}
	LeaveCritical();

	return Ret;
}

