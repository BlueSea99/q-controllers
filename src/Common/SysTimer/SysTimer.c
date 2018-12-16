#include "SysDefines.h"

volatile SYS_TIMER_RCD gpSysTimerRcd[SYS_TIMER_MAX_NUM];

void DebugSysTimer(void)
{
	u8 Idx;
	
	for(Idx=0;Idx<SYS_TIMER_MAX_NUM;Idx++)
	{
		if(gpSysTimerRcd[Idx].Id)
		{
			SYS_TIMER_RCD *pRcd=(void *)&gpSysTimerRcd[Idx];
			Debug("[%d]ID:%u,Type%u,%d,DefV %u,Cnt%d->%u,Task %u\n\r",Idx,pRcd->Id,pRcd->Type,pRcd->ExcInISR,pRcd->DefVal,pRcd->Count,pRcd->ValueTick,pRcd->TaskId);
		}
		else
		{
			Debug("[%d]NULL\r\n",Idx);
		}
	}
	
	Debug("\r\n");
}

void SysTimerInit(void)
{
	MemSet((void *)gpSysTimerRcd,0,sizeof(gpSysTimerRcd));
}

#if 0
static u8 FindIdxByTimerId(u32 Id)
{
	u8 Idx;

	if(Id==0) return 0xff;
	
	for(Idx=0;Idx<SYS_TIMER_MAX_NUM;Idx++)
	{
		if(gpSysTimerRcd[Idx].Id==Id)
		{
			break;
		}
	}
	if(Idx==SYS_TIMER_MAX_NUM)
	{
		Debug("SYS TIMER FOR TASK ERROR!\n\r");
		while(1);
		return 0xff;
	}

	return Idx;
}
#endif

//����ϵͳ��ʱ��
//ʱ�䵥λ��Ϊms
u32 AddSysTimer(SYS_TIMER_TYPE Type,u32 DefValue,EVENT_BIT_FLAG Event,pVoidFunc pCallBackFunc,bool ExcInISR)
{
	static u32 IdCnt=1000;
	u8 i;
	IntSaveInit();

	EnterCritical();               
	
	//�ҿ�λ��
	for(i=0;i<SYS_TIMER_MAX_NUM;i++)
	{
		if(gpSysTimerRcd[i].Id==0)
		{
			break;
		}
	}
	if(i==SYS_TIMER_MAX_NUM)
	{
		Debug("SYS TIMER NUM SMALL!\n\r");
		LeaveCritical();
		DebugSysTimer();
		RebootBoard();
		while(1);
	}
	
	//��ʼ����
	DefValue/=SYS_SCHEDULE_PERIOS_MS;
	if(DefValue==0) DefValue=1;//��ֹValueС��SYS_SCHEDULE_PERIOS_MSʱ������ֵΪ0���Ӷ��رն�ʱ��
	switch(Type)
	{
		case STT_AUTO:
			{
				gpSysTimerRcd[i].Id=IdCnt++;
				gpSysTimerRcd[i].ExcInISR=ExcInISR;
				gpSysTimerRcd[i].Type=Type;
				gpSysTimerRcd[i].Event=Event;
				gpSysTimerRcd[i].Count=0;
				gpSysTimerRcd[i].ValueTick=DefValue;
				gpSysTimerRcd[i].DefVal=DefValue;
				gpSysTimerRcd[i].pCB=pCallBackFunc;
				gpSysTimerRcd[i].pTaskTimoutCB=NULL;
				gpSysTimerRcd[i].TaskId=0;
				LeaveCritical();
				return gpSysTimerRcd[i].Id;
			}				
		case STT_MANUAL:
			{
				gpSysTimerRcd[i].Id=IdCnt++;
				gpSysTimerRcd[i].ExcInISR=ExcInISR;
				gpSysTimerRcd[i].Type=Type;
				gpSysTimerRcd[i].Event=Event;
				gpSysTimerRcd[i].Count=0;
				gpSysTimerRcd[i].ValueTick=0;
				gpSysTimerRcd[i].DefVal=DefValue;
				gpSysTimerRcd[i].pCB=pCallBackFunc;
				gpSysTimerRcd[i].pTaskTimoutCB=NULL;
				gpSysTimerRcd[i].TaskId=0;
				LeaveCritical();
				return gpSysTimerRcd[i].Id;
			}
	}

	Debug("Add Sys Timer Error!Type %d\n\r",Type);
	LeaveCritical();
	DebugSysTimer();
	RebootBoard();
	while(1);
//	return 0;
}

//ִ�к��Զ�start
u32 AddSysTimerForTask(u32 Value,pStdFunc pCallBackFunc,u8 TaskId)
{
	u32 Timer=0;
	IntSaveInit();
				
	if(pCallBackFunc==NULL) return 0;

	EnterCritical();
	Timer=AddSysTimer(STT_MANUAL,Value,EBF_NULL,NULL,FALSE);

	if(Timer)
	{
		u8 i;
		for(i=0;i<SYS_TIMER_MAX_NUM;i++)
		{
			if(gpSysTimerRcd[i].Id==Timer)
			{
				gpSysTimerRcd[i].pCB=NULL;
				gpSysTimerRcd[i].pTaskTimoutCB=pCallBackFunc;
				gpSysTimerRcd[i].TaskId=TaskId;
				LeaveCritical();
				StartSysTimer(Timer,Value);
				return Timer;
			}
		}
		if(i==SYS_TIMER_MAX_NUM)
		{
			Debug("SYS TIMER FOR TASK(%d) ERROR!\n\r",TaskId);
			LeaveCritical();
			DebugSysTimer();
			RebootBoard();
			while(1);
		}
	}

	LeaveCritical();
	return Timer;
}

//�޸Ķ�ʱ��ֵ�������¼�ʱ
//valueΪ0������ֹͣ��ʱ��
bool ChangeSysTimerVal(u32 Id,u32 Value)
{
	u8 Idx;
	IntSaveInit();

	if(Id==0) return FALSE;

	EnterCritical();	
	for(Idx=0;Idx<SYS_TIMER_MAX_NUM;Idx++)
	{
		if(gpSysTimerRcd[Idx].Id==Id && 
			gpSysTimerRcd[Idx].Type != STT_NULL)
		{
			break;
		}
	}
	if(Idx==SYS_TIMER_MAX_NUM)
	{
		Debug("ChangeSysTimerVal Error SYS TIMER ID %d!\n\r",Id);
		LeaveCritical();
		DebugSysTimer();
		while(1);
	}
	Value/=SYS_SCHEDULE_PERIOS_MS;
	if(Value==0) Value=1;//��ֹValueС��SYS_SCHEDULE_PERIOS_MSʱ������ֵΪ0���Ӷ��رն�ʱ��
	gpSysTimerRcd[Idx].ValueTick=Value;
	gpSysTimerRcd[Idx].Count=0;

	LeaveCritical();
	return TRUE;
}

//���valueΪ0��������Ĭ��ʱ��
//ֻ��STT_MANUAL��Ч
//�Զ���ʱ������ChangeSysTimerVal
bool StartSysTimer(u32 Id,u32 Value)
{
	u8 Idx;
	IntSaveInit();

	if(Id==0) return FALSE;

	EnterCritical();	
	for(Idx=0;Idx<SYS_TIMER_MAX_NUM;Idx++)
	{
		if(gpSysTimerRcd[Idx].Id==Id && 
			gpSysTimerRcd[Idx].Type != STT_NULL)
		{
			break;
		}
	}
	if(Idx==SYS_TIMER_MAX_NUM)
	{
		Debug("StartSysTimer Error SYS TIMER ID %d!\n\r",Id);
		LeaveCritical();
		DebugSysTimer();
		while(1);
	}
	
	if(gpSysTimerRcd[Idx].Type!=STT_MANUAL) 
	{
		LeaveCritical();
		return FALSE;
	}
	
	if(Value)//ָ���˶�ʱֵ
	{
		Value/=SYS_SCHEDULE_PERIOS_MS;
		if(Value==0) Value=1;//��ֹValueС��SYS_SCHEDULE_PERIOS_MSʱ������ֵΪ0���Ӷ��رն�ʱ��
		gpSysTimerRcd[Idx].ValueTick=Value;
		gpSysTimerRcd[Idx].Count=0;
	}
	else if(gpSysTimerRcd[Idx].DefVal)//ʹ��Ĭ��ֵ
	{
		gpSysTimerRcd[Idx].ValueTick=gpSysTimerRcd[Idx].DefVal;
		gpSysTimerRcd[Idx].Count=0;
	}

	LeaveCritical();
	return TRUE;
}

//ֻ��STT_MANUAL��Ч
//�Զ���ʱ������ChangeSysTimerVal
bool StopSysTimer(u32 Id)
{
	u8 Idx;
	IntSaveInit();

	if(Id==0) return FALSE;

	EnterCritical();	
	for(Idx=0;Idx<SYS_TIMER_MAX_NUM;Idx++)
	{
		if(gpSysTimerRcd[Idx].Id==Id && 
			gpSysTimerRcd[Idx].Type != STT_NULL)
		{
			break;
		}
	}
	if(Idx==SYS_TIMER_MAX_NUM)
	{
		Debug("StopSysTimer Error SYS TIMER ID %d!\n\r",Id);
		LeaveCritical();
		DebugSysTimer();
		while(1);
	}

	if(gpSysTimerRcd[Idx].Type!=STT_MANUAL) 
	{
		LeaveCritical();
		return FALSE;
	}
	
	gpSysTimerRcd[Idx].Count=0;//
	gpSysTimerRcd[Idx].ValueTick=0;//�ñ�־
	LeaveCritical();
	
	return TRUE;
}

//��鶨ʱ���Ƿ��ڹ���״̬
bool SysTimerWorking(u32 Id)
{
	bool Working=FALSE;
	u8 Idx;
	IntSaveInit();
	
	if(Id==0) return FALSE;

	EnterCritical();	
	for(Idx=0;Idx<SYS_TIMER_MAX_NUM;Idx++)
	{
		if(gpSysTimerRcd[Idx].Id==Id && 
			gpSysTimerRcd[Idx].Type != STT_NULL)
		{
			break;
		}
	}
	if(Idx==SYS_TIMER_MAX_NUM)
	{
		Debug("SysTimerWorking Error SYS TIMER ID %d!\n\r",Id);
		LeaveCritical();
		DebugSysTimer();
		while(1);
	}

	if(gpSysTimerRcd[Idx].Type==STT_AUTO || gpSysTimerRcd[Idx].Type==STT_MANUAL)
	{
		if(gpSysTimerRcd[Idx].ValueTick) Working=TRUE;			
	}
	
	LeaveCritical();
	return Working;
}

//��ȡʣ���ʱ
u32 GetSysTimerRemain(u32 Id)
{
	u8 Idx;
	u32 Remain=0;
	IntSaveInit();
	
	if(Id==0) return 0;

	EnterCritical();	
	for(Idx=0;Idx<SYS_TIMER_MAX_NUM;Idx++)
	{
		if(gpSysTimerRcd[Idx].Id==Id && 
			gpSysTimerRcd[Idx].Type != STT_NULL)
		{
			break;
		}
	}
	if(Idx==SYS_TIMER_MAX_NUM)
	{
		Debug("GetSysTimerRemain Error SYS TIMER ID %d!\n\r",Id);
		LeaveCritical();
		DebugSysTimer();
		while(1);
	}

	if(gpSysTimerRcd[Idx].Type==STT_AUTO || gpSysTimerRcd[Idx].Type==STT_MANUAL)
	{
		if(gpSysTimerRcd[Idx].ValueTick)
			Remain=(gpSysTimerRcd[Idx].ValueTick-gpSysTimerRcd[Idx].Count)*SYS_SCHEDULE_PERIOS_MS;
	}
	
	LeaveCritical();
	return Remain;
}

//��ȡʣ���ʱ
u32 GetSysTimerCount(u32 Id)
{
	u8 Idx;
	u32 Count=0;
	
	IntSaveInit();
	
	if(Id==0) return 0;

	EnterCritical();
	for(Idx=0;Idx<SYS_TIMER_MAX_NUM;Idx++)
	{
		if(gpSysTimerRcd[Idx].Id==Id && 
			gpSysTimerRcd[Idx].Type != STT_NULL)
		{
			break;
		}
	}
	if(Idx==SYS_TIMER_MAX_NUM)
	{
		Debug("GetSysTimerRemain Error SYS TIMER ID %d!\n\r",Id);
		LeaveCritical();
		DebugSysTimer();
		while(1);
	}

	if(gpSysTimerRcd[Idx].Type==STT_AUTO || gpSysTimerRcd[Idx].Type==STT_MANUAL)
	{
		if(gpSysTimerRcd[Idx].ValueTick)
			Count=gpSysTimerRcd[Idx].Count*SYS_SCHEDULE_PERIOS_MS;
	}

	LeaveCritical();
	return Count;
}

bool DeleteSysTimer(u32 Id)
{
	u8 Idx;
	IntSaveInit();
	
	if(Id==0) return FALSE;
	
	EnterCritical();
	for(Idx=0;Idx<SYS_TIMER_MAX_NUM;Idx++)
	{
		if(gpSysTimerRcd[Idx].Id==Id)
		{
			break;
		}
	}
	if(Idx==SYS_TIMER_MAX_NUM)
	{
		Debug("DeleteSysTimer Error SYS TIMER ID %d!\n\r",Id);
		LeaveCritical();
		DebugSysTimer();
		while(1);
	}	
	
	//�����������Ŀǰ�ƣ���ѹ��idx
	for(Idx++;Idx<SYS_TIMER_MAX_NUM;Idx++)
	{
		if(gpSysTimerRcd[Idx].Id==Id)//����Ϣ���͸���
			MemCpy((void *)&gpSysTimerRcd[Idx-1],(void *)&gpSysTimerRcd[Idx],sizeof(SYS_TIMER_RCD));
		else//û��Ϣ�����˳�
			break;
	}

	//�������һ��
	gpSysTimerRcd[Idx-1].Id=0;
	gpSysTimerRcd[Idx-1].Type=STT_NULL;

	LeaveCritical();	
	return TRUE;
}

