#include "SysDefines.h"

#define NEXT_FUNC_RIGHT_CODE 0x95884756

//����
#define NEXT_LOOP_FUNC_MAX 16

typedef struct{
	u32 RightCode;
	bool IsQuick;
	pVoidFunc pVoidCallback;
	pStdFunc pStdCallback;
	pExpFunc pExpCallback;
	int IntParam;
	int IntParam2;
	void *pParam;
}NEXT_FUNC_RCD;
static void *gpNextFuncQueue=NULL;

void NextLoopFuncInit(void)
{
	if(gpNextFuncQueue==NULL) gpNextFuncQueue=Q_NewQueue(sizeof(NEXT_FUNC_RCD),NEXT_LOOP_FUNC_MAX);
}

//�ŵ��¸�ѭ��ִ�еĺ���
//����IsQuick�����Ƿŵ�mainִ��
bool AddNextVoidFunc(bool IsQuick,pVoidFunc pCallBack)
{
	NEXT_FUNC_RCD Rcd;

	if(pCallBack==NULL) return FALSE;

	if(gpNextFuncQueue==NULL) 
	{
		Debug("NextFunc Not Init!\n\r");
		while(1);
	}
	
	if(Q_QueueFull(gpNextFuncQueue))
	{
		Debug("AddNextVoidFull!%x\n\r",pCallBack);

		while(Q_FetchQueueFirst(gpNextFuncQueue,&Rcd,TRUE)==TRUE)
		{
			if(Rcd.pVoidCallback)
				Debug("Rcd:%x\n\r",Rcd.pVoidCallback);
			if(Rcd.pStdCallback)	
				Debug("Rcd:%x(%d,p%d)\n\r",Rcd.pStdCallback,Rcd.IntParam,Rcd.pParam);
			else if(Rcd.pExpCallback)
				Debug("Rcd:%x(%d,%d,p%d)\n\r",Rcd.pExpCallback,Rcd.IntParam,Rcd.IntParam2,Rcd.pParam);
		}

		RebootBoard();
		while(1);
//		return FALSE;
	}	

	Rcd.RightCode=NEXT_FUNC_RIGHT_CODE;
	Rcd.IsQuick=IsQuick;
	Rcd.pVoidCallback=pCallBack;
	Rcd.pStdCallback=NULL;
	Rcd.pExpCallback=NULL;
	Rcd.IntParam=0;
	Rcd.IntParam2=0;
	Rcd.pParam=NULL;
	if(Q_QueueAddItem(gpNextFuncQueue,&Rcd,FALSE)==FALSE) return FALSE;

	if(IsQuick) SetEventFlag(EBF_NEXT_QUICK_FUNC);
	else SetEventFlag(EBF_NEXT_LOOP_FUNC);	
	return TRUE;	
}

//�ŵ��¸�ѭ��ִ�еĺ���
bool AddNextStdFunc(bool IsQuick,pStdFunc pCallBack,int IntParam,void *pParam)
{
	NEXT_FUNC_RCD Rcd;

	if(pCallBack==NULL) return FALSE;

	if(gpNextFuncQueue==NULL) return FALSE;
	
	if(Q_QueueFull(gpNextFuncQueue))
	{
		Debug("AddNextStdFull!%x %d %d\n\r",pCallBack,IntParam,pParam);

		while(Q_FetchQueueFirst(gpNextFuncQueue,&Rcd,TRUE)==TRUE)
		{
			if(Rcd.pVoidCallback)
				Debug("Rcd:%x\n\r",Rcd.pVoidCallback);
			if(Rcd.pStdCallback)	
				Debug("Rcd:%x(%d,p%d)\n\r",Rcd.pStdCallback,Rcd.IntParam,Rcd.pParam);
			else if(Rcd.pExpCallback)
				Debug("Rcd:%x(%d,%d,p%d)\n\r",Rcd.pExpCallback,Rcd.IntParam,Rcd.IntParam2,Rcd.pParam);
		}

		RebootBoard();
		while(1);
//		return FALSE;
	}	

	Rcd.RightCode=NEXT_FUNC_RIGHT_CODE;
	Rcd.IsQuick=IsQuick;
	Rcd.pVoidCallback=NULL;
	Rcd.pStdCallback=pCallBack;
	Rcd.pExpCallback=NULL;
	Rcd.IntParam=IntParam;
	Rcd.IntParam2=0;
	Rcd.pParam=pParam;
	if(Q_QueueAddItem(gpNextFuncQueue,&Rcd,FALSE)==FALSE) return FALSE;

	if(IsQuick) SetEventFlag(EBF_NEXT_QUICK_FUNC);
	else SetEventFlag(EBF_NEXT_LOOP_FUNC);	
	return TRUE;	
}

//�ŵ��¸�ѭ��ִ�еĺ���
//IsQuick�����Ƿ�ŵ��¸�ѭ��ͷ����ѭ��β
bool AddNextExpFunc(bool IsQuick,pExpFunc pCallBack,int IntParam,int IntParam2,void *pParam)
{
	NEXT_FUNC_RCD Rcd;

	if(pCallBack==NULL) return FALSE;

	if(gpNextFuncQueue==NULL) return FALSE;
	
	if(Q_QueueFull(gpNextFuncQueue))
	{
		Debug("AddNextExpFull!%x %d %d\n\r",pCallBack,IntParam,pParam);

		while(Q_FetchQueueFirst(gpNextFuncQueue,&Rcd,TRUE)==TRUE)
		{
			if(Rcd.pVoidCallback)
				Debug("Rcd:%x\n\r",Rcd.pVoidCallback);
			if(Rcd.pStdCallback)	
				Debug("Rcd:%x(%d,p%d)\n\r",Rcd.pStdCallback,Rcd.IntParam,Rcd.pParam);
			else if(Rcd.pExpCallback)
				Debug("Rcd:%x(%d,%d,p%d)\n\r",Rcd.pExpCallback,Rcd.IntParam,Rcd.IntParam2,Rcd.pParam);
		}

		RebootBoard();
		while(1);
//		return FALSE;
	}	

	Rcd.RightCode=NEXT_FUNC_RIGHT_CODE;
	Rcd.IsQuick=IsQuick;
	Rcd.pVoidCallback=NULL;
	Rcd.pStdCallback=NULL;
	Rcd.pExpCallback=pCallBack;
	Rcd.IntParam=IntParam;
	Rcd.IntParam2=IntParam2;
	Rcd.pParam=pParam;
	if(Q_QueueAddItem(gpNextFuncQueue,&Rcd,FALSE)==FALSE) return FALSE;

	if(IsQuick) SetEventFlag(EBF_NEXT_QUICK_FUNC);
	else SetEventFlag(EBF_NEXT_LOOP_FUNC);	
	return TRUE;	
}
//����main��
//�ظ�TURE��ʾ���л������ݣ���Ҫ�����¸�loopִ��
bool NextFuncExcute(bool IsQuick)
{
	NEXT_FUNC_RCD Rcd;
	u16 Idx=1;
	
	while(1)
	{
		if(Q_FetchQueueItem(gpNextFuncQueue,Idx,&Rcd,FALSE)==FALSE) break;
	
		if(Rcd.RightCode!=NEXT_FUNC_RIGHT_CODE) continue;

		if(Rcd.IsQuick==IsQuick)
		{
			if(Rcd.pVoidCallback!=NULL) Rcd.pVoidCallback();
			else if(Rcd.pStdCallback!=NULL) Rcd.pStdCallback(Rcd.IntParam,Rcd.pParam);
			else if(Rcd.pExpCallback!=NULL) Rcd.pExpCallback(Rcd.IntParam,Rcd.IntParam2,Rcd.pParam);
			Q_FetchQueueItem(gpNextFuncQueue,Idx,NULL,TRUE);
		}
		else
		{
			Idx++;
		}
	}
	
	return TRUE;
}





