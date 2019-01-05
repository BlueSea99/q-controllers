#include "stm32f10x.h"
#include "uart.h"
#include "PublicFunc.h"
#include "Q_Heap.h"
#include "Q_Queue.h"

#define QUEUE_RIGHT_CODE 0x25842311//�����

typedef struct{
	u32 RightCode;//����������ȷ�ľ��
	u16 ItemSize;//����QUEUE_ITEM����
	u16 MaxNum;
	u16 NowTotal;//��ǰitem����
	void *pFirst;
	void *pLast;
	void *pNull;	
}QUEUE_HEADER;

typedef struct{
	void *pPre;
	void *pNext;
}QUEUE_ITEM;

//�½�����
void *Q_NewQueue(u16 ItemSize,u16 MaxNum)
{
	QUEUE_HEADER *pQueue;
	u16 MallcoSize;
	IntSaveInit();

	EnterCritical();
	ItemSize=AlignTo4(ItemSize);// 4�ֽڶ��룻
	MallcoSize=sizeof(QUEUE_HEADER)+MaxNum*(sizeof(QUEUE_ITEM)+ItemSize);
	//Debug("Queue Mallco %d\r\n",MallcoSize);
	pQueue=Q_Malloc(MallcoSize);//��Q_DeleteQueue���ͷ�
	//Debug("MallcoQ %x\r\n",(u32)pQueue);
	MemSet((void *)pQueue,0,MallcoSize);
	pQueue->RightCode=QUEUE_RIGHT_CODE;
	pQueue->ItemSize=ItemSize;
	pQueue->MaxNum=MaxNum;
	pQueue->NowTotal=0;
	pQueue->pFirst=NULL;
	pQueue->pLast=NULL;
	pQueue->pNull=(void *)&pQueue[1];//ҪԽ��ͷ��������������1
	LeaveCritical();
	
	return pQueue;
}

//ɾ��һ�����У��������Դ
bool Q_DeleteQueue(void **ppHandler)
{
	QUEUE_HEADER *pQueue=*ppHandler;	
	IntSaveInit();
	
	EnterCritical();
	if(*ppHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) 
	{
		LeaveCritical();
		return FALSE;//����ľ��
	}
	//Debug("FreeQ %x\r\n",(u32)pQueue);
	
	pQueue->RightCode=0;
	*ppHandler=NULL;
	Q_Free(pQueue);
	LeaveCritical();
	
	return TRUE;
}

//�������������
bool Q_QueueClean(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;
	u16 Size;
	IntSaveInit();

	EnterCritical();
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE)
	{
		LeaveCritical();
		return FALSE;//����ľ��
	}
	
	Size=pQueue->MaxNum*(sizeof(QUEUE_ITEM)+pQueue->ItemSize);
	//Debug("Clean %d\n\r",Size);
	MemSet((void *)&pQueue[1],0,Size);//ҪԽ��ͷ��������������1
	pQueue->NowTotal=0;
	pQueue->pFirst=NULL;
	pQueue->pLast=NULL;
	pQueue->pNull=(void *)&pQueue[1];//ҪԽ��ͷ��������������1
	LeaveCritical();
	
	return TRUE;
}

//ԭ�Ӳ���
//�Ƚ��ȳ�Э��
//���force=false��������Ͳ�����
//���force=true����������Ͱ�ͷ���ļ���
bool Q_QueueAddItem(void *pHandler,void *New,bool Force)
{
	QUEUE_HEADER *pQueue=pHandler;
	QUEUE_ITEM *pNull=NULL;
	IntSaveInit();

	EnterCritical();
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE || New==NULL)
	{
		LeaveCritical();
		return FALSE;//����ľ��
	}
	
	if(pQueue->pNull==NULL && Force==FALSE) //û�п�λ����
	{
		LeaveCritical();
		return FALSE;
	}	
	
	if(pQueue->pNull==NULL && Force==TRUE) //ǿ�м���,��ɾ��ͷ��
	{
		QUEUE_ITEM *pFirst=pQueue->pFirst;
		pQueue->NowTotal--;

		if(pQueue->pLast==pQueue->pFirst) //����ֻ��һ����Ա
		{
			pQueue->pFirst=NULL;
			pQueue->pLast=NULL;
		}
		else
		{
			QUEUE_ITEM *pNewFirst=pFirst->pNext;
			pQueue->pFirst=pNewFirst;
			pNewFirst->pPre=pNewFirst;//���ⱻ�������п�
		}
		pFirst->pNext=NULL;
		pFirst->pPre=NULL;	

		if(pQueue->pNull==NULL) //����û�ռ䣬��������
		{
			pQueue->pNull=pFirst;
		}
	}
	
	pNull=pQueue->pNull;//�õ���λ��

	//Debug("QueueAddItem\r\n");

	if(pQueue->pFirst==NULL)//�¼ӵ��ǵ�һ��
	{
		pQueue->pFirst=pNull;
		pQueue->pLast=pNull;
		pNull->pPre=pNull;//���ܶ�Ϊ�գ�����ᱻ�������п�
		pNull->pNext=NULL;
	}
	else if(pQueue->pLast!=NULL)//��ӵ�����β��
	{
		QUEUE_ITEM *pLast=pQueue->pLast;

		pQueue->pLast=pNull;
		pLast->pNext=pNull;
		pNull->pPre=pLast;
		pNull->pNext=NULL;
	}
	else
	{
		Debug("Queue Add Item Error!\r\n");
		LeaveCritical();
		while(1);
	}

	MemCpy((void *)&pNull[1],New,pQueue->ItemSize);//�������ݣ�ҪԽ��ͷ��������������1
	pQueue->NowTotal++;
	
	//��ʼ�ҿ��еĿ�
	pQueue->pNull=NULL;
	{
		u16 i;
		QUEUE_ITEM *pNewNull=(void *)&pQueue[1];//ҪԽ��ͷ��������������1
		
		for(i=0;i<pQueue->MaxNum;i++) //���µĿ�λ��
		{
			if(pNewNull->pNext==NULL && pNewNull->pPre==NULL)
			{
				pQueue->pNull=pNewNull;
				LeaveCritical();
				return TRUE;
			}		
			pNewNull=(void *)(pQueue->ItemSize+(u32)&pNewNull[1]);
		}		
	}
	
	LeaveCritical();
	return TRUE;//�Ӵ˴����ر�ʾ������
}


//ԭ�Ӳ���
//�������Ƚ��ȳ��������˺�������������ӵ�����ͷ�����ƻ����Ƚ��ȳ�����
//���force=false��������Ͳ�����
//���force=true����������Ͱ�β���ļ���
bool Q_QueueAddItemToFirst(void *pHandler,void *New,bool Force)
{
	QUEUE_HEADER *pQueue=pHandler;
	QUEUE_ITEM *pNull=NULL;
	IntSaveInit();

	EnterCritical();
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE || New==NULL)
	{
		LeaveCritical();
		return FALSE;//����ľ��
	}
	
	if(pQueue->pNull==NULL && Force==FALSE) //û�п�λ����
	{
		LeaveCritical();
		return FALSE;
	}	
	
	if(pQueue->pNull==NULL && Force==TRUE) //�ռ�����ǿ�м���,��ɾ��β��
	{
		QUEUE_ITEM *pLast=pQueue->pLast;
		pQueue->NowTotal--;

		if(pQueue->pLast==pQueue->pFirst) //����ֻ��һ����Ա
		{
			pQueue->pFirst=NULL;
			pQueue->pLast=NULL;
		}
		else//���ж����Ա
		{
			QUEUE_ITEM *pNewLast=pLast->pPre;
			pQueue->pLast=pNewLast;
			pNewLast->pNext=NULL;
		}
		pLast->pNext=NULL;
		pLast->pPre=NULL;	

		if(pQueue->pNull==NULL) //����û�ռ䣬��������
		{
			pQueue->pNull=pLast;
		}
	}
	
	pNull=pQueue->pNull;//�õ���λ��

	//Debug("QueueAddItem\r\n");

	if(pQueue->pFirst==NULL)//�¼ӵ��ǵ�һ��
	{
		pQueue->pFirst=pNull;
		pQueue->pLast=pNull;
		pNull->pPre=pNull;//���ܶ�Ϊ�գ�����ᱻ�������п�
		pNull->pNext=NULL;
	}
	else if(pQueue->pLast!=NULL)//��ӵ�����ͷ��
	{
		QUEUE_ITEM *pFirst=pQueue->pFirst;

		pQueue->pFirst=pNull;
		pFirst->pPre=pNull;
		pNull->pPre=pNull;//��ֵ����ֹ���������п�
		pNull->pNext=pFirst;
	}
	else
	{
		Debug("Queue Add Item To First Error!\r\n");
		LeaveCritical();
		while(1);
	}

	MemCpy((void *)&pNull[1],New,pQueue->ItemSize);//�������ݣ�ҪԽ��ͷ��������������1
	pQueue->NowTotal++;
	
	//��ʼ�ҿ��еĿ�
	pQueue->pNull=NULL;
	{
		u16 i;
		QUEUE_ITEM *pNewNull=(void *)&pQueue[1];//ҪԽ��ͷ��������������1
		
		for(i=0;i<pQueue->MaxNum;i++) //���µĿ�λ��
		{
			if(pNewNull->pNext==NULL && pNewNull->pPre==NULL)
			{
				pQueue->pNull=pNewNull;
				LeaveCritical();
				return TRUE;
			}		
			pNewNull=(void *)(pQueue->ItemSize+(u32)&pNewNull[1]);
		}		
	}
	
	LeaveCritical();
	return TRUE;//�Ӵ˴����ر�ʾ������
}


//ȡ����ͷ
bool Q_FetchQueueFirst(void *pHandler,void *pRead,bool NeedDelete)
{
	QUEUE_HEADER *pQueue=pHandler;
	QUEUE_ITEM *pFirst=NULL;
	IntSaveInit();

	EnterCritical();
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE)
	{
		LeaveCritical();
		return FALSE;//����ľ��
	}
	
	pFirst=pQueue->pFirst;
	if(pFirst==NULL)	//û������
	{
		if(pRead!=NULL) MemSet(pRead,0,pQueue->ItemSize);
		LeaveCritical();
		return FALSE;
	}

	if(pRead!=NULL) MemCpy(pRead,(void *)&pFirst[1],pQueue->ItemSize);//�������ݳ�ȥ��ҪԽ��ͷ��������������1
	
	if(NeedDelete)
	{		
		pQueue->NowTotal--;

		if(pQueue->pLast==pQueue->pFirst) //����ֻ��һ����Ա
		{
			pQueue->pFirst=NULL;
			pQueue->pLast=NULL;
		}
		else
		{
			QUEUE_ITEM *pNewFirst=pFirst->pNext;
			pQueue->pFirst=pNewFirst;
			pNewFirst->pPre=pNewFirst;//���ⱻ�������п�
		}
		pFirst->pNext=NULL;
		pFirst->pPre=NULL;	

		if(pQueue->pNull==NULL) //����û�ռ䣬��������
		{
			pQueue->pNull=pFirst;
		}	
	}
	
	LeaveCritical();
	return TRUE;
}

//ȡ����β
bool Q_FetchQueueLast(void *pHandler,void *pRead,bool NeedDelete)
{
	QUEUE_HEADER *pQueue=pHandler;
	QUEUE_ITEM *pLast=NULL;
	IntSaveInit();

	EnterCritical();
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE)
	{
		LeaveCritical();
		return FALSE;//����ľ��
	}
	
	pLast=pQueue->pLast;
	if(pLast==NULL)	//û������
	{
		if(pRead!=NULL) MemSet(pRead,0,pQueue->ItemSize);
		LeaveCritical();
		return FALSE;
	}
	
	if(pRead!=NULL) MemCpy(pRead,(void *)&pLast[1],pQueue->ItemSize);//�������ݳ�ȥ��ҪԽ��ͷ��������������1

	if(NeedDelete)
	{
		pQueue->NowTotal--;
		
		if(pQueue->pLast==pQueue->pFirst) //����ֻ��һ����Ա
		{
			pQueue->pFirst=NULL;
			pQueue->pLast=NULL;
		}
		else
		{
			QUEUE_ITEM *pNewLast=pLast->pPre;
			pQueue->pLast=pNewLast;
			pNewLast->pNext=NULL;
		}
		pLast->pNext=NULL;
		pLast->pPre=NULL;

		if(pQueue->pNull==NULL) //����û�ռ䣬��������
		{
			pQueue->pNull=pLast;
		}		
	}
	
	LeaveCritical();
	return TRUE;
}

//idx��1��ʼ
//ȡָ�������Ķ���
//δȡ������false
bool Q_FetchQueueItem(void *pHandler,u16 Idx,void *pRead,bool NeedDelete)
{
	QUEUE_HEADER *pQueue=pHandler;
	QUEUE_ITEM *pItem=NULL;
	u16 Cnt=0;
	IntSaveInit();

	EnterCritical();
	if(Idx==0 || pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE)
	{
		LeaveCritical();
		return FALSE;//����ľ��
	}
	
	pItem=pQueue->pFirst;
	if(pItem==NULL)	//û������
	{
		if(pRead!=NULL) MemSet(pRead,0,pQueue->ItemSize);
		LeaveCritical();
		return FALSE;
	}

	for(Cnt=1;Cnt<Idx;Cnt++)
	{
		pItem=pItem->pNext;
		if(pItem==NULL) 
		{
			LeaveCritical();
			return FALSE;//���껹û������Ӧ��idx
		}
	}	

	if(pRead!=NULL) MemCpy(pRead,(void *)&pItem[1],pQueue->ItemSize);//�������ݳ�ȥ��ҪԽ��ͷ��������������1

	if(NeedDelete)
	{
		pQueue->NowTotal--;
		
		if(Idx==1)//ͷ��Ա
		{
			if(pQueue->pLast==pQueue->pFirst) //����ֻ��һ����Ա
			{
				pQueue->pFirst=NULL;
				pQueue->pLast=NULL;
			}
			else
			{
				QUEUE_ITEM *pNewFirst=pItem->pNext;
				pQueue->pFirst=pNewFirst;
				pNewFirst->pPre=pNewFirst;//���ⱻ�������п�
			}
		}
		else if(pItem->pNext==NULL)//β��Ա
		{			
			QUEUE_ITEM *pNewLast=pItem->pPre;
			pQueue->pLast=pNewLast;
			pNewLast->pNext=NULL;
		}
		else//�м��Ա
		{
			QUEUE_ITEM *pItemTmp;
			pItemTmp=pItem->pPre;
			pItemTmp->pNext=pItem->pNext;
			pItemTmp=pItem->pNext;
			pItemTmp->pPre=pItem->pPre;
		}

		//ɾ������
		pItem->pNext=NULL;
		pItem->pPre=NULL;	

		if(pQueue->pNull==NULL) //����û�ռ䣬��������
		{
			pQueue->pNull=pItem;
		}		
	}
	
	LeaveCritical();
	return TRUE;
}

//��ȡ������Ŀ����
u16 Q_GetQueueItemTotal(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;

	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) return 0;//����ľ��

	return pQueue->NowTotal;
}

//���п�
bool Q_QueueEmpty(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;
	
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) return FALSE;//����ľ��

	if(pQueue->NowTotal==0) return TRUE;

	return FALSE;
}

//���зǿ�
bool Q_QueueNotEmpty(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;
	
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) return FALSE;//����ľ��

	if(pQueue->NowTotal==0) return FALSE;

	return TRUE;
}

//������
bool Q_QueueFull(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;

	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) return FALSE;//����ľ��
	
	if(pQueue->pNull==NULL) return TRUE;

	return FALSE;
}

//����δ��
bool Q_QueueNotFull(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;

	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) return FALSE;//����ľ��
	
	if(pQueue->pNull==NULL) return FALSE;

	return TRUE;
}

