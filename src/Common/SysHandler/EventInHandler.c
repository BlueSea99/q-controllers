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
		volatile EVENT_ITEM *p=Q_Malloc(sizeof(EVENT_ITEM));
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
		EventControllerPost(EBF_IDLE,0,NULL);
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

//------------------------------------------���������------------------------------------------
typedef struct{
	const char *pName;
	const EVENT_FUNC_ITEM *ItemArray;
	u16 EvtFuncTotal;
}EVENT_CTRLER_ITME;//��������¼��

#define EVENT_CONTROLLER_MAX 64
static EVENT_CTRLER_ITME gEvtCtrlers[EVENT_CONTROLLER_MAX]={{NULL,0}};//�������б�
static u16 gEvtCtrlerNum=0;

//������ע���Լ���ϵͳ
void EventControllerRegister(const EVENT_FUNC_ITEM *pItemArray,const char *pName)
{
	u16 i=0;
	
	for(i=0;i<EBF_MAX;i++)
	{
		if(pItemArray[i].Event==EBF_NULL || pItemArray[i].Event>=EBF_MAX) break;
		if(pItemArray[i].EvtFunc==NULL) break;
	}

	if(i)
	{
		gEvtCtrlers[gEvtCtrlerNum].pName=pName;
		gEvtCtrlers[gEvtCtrlerNum].ItemArray=pItemArray;
		gEvtCtrlers[gEvtCtrlerNum].EvtFuncTotal=i;
		gEvtCtrlerNum++;
	}
}

//�������¼��ַ�
void EventControllerPost(EVENT_BIT_FLAG Event,int Param,void *p)
{
	u16 CtrlerNum=0,EvtNum=0;

	for(CtrlerNum=0;CtrlerNum<gEvtCtrlerNum;CtrlerNum++)//��ѯ������
	{
		const EVENT_FUNC_ITEM *pItemArray=gEvtCtrlers[CtrlerNum].ItemArray;
		EVENT_HANDLER_RESUTL Res=EFR_OK;
		
		for(EvtNum=0;EvtNum<gEvtCtrlers[CtrlerNum].EvtFuncTotal;EvtNum++)//��ѯ�¼��ص�
		{
			if(Event==pItemArray[EvtNum].Event && pItemArray[EvtNum].EvtFunc!=NULL)
			{
				Res=pItemArray[EvtNum].EvtFunc(Param,p);//�¼��������ص�
				if(Res==EFR_STOP) goto EvtFinish;
				break;//ͬһ���¼�ÿ��������ֻ�ܶ�Ӧһ���ص�
			}
		}
	}

EvtFinish:
	return;
}

