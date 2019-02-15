//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ���װ��һ�׶�ʱ�����ƣ��ɱ�����������������Ŀ�����ٴ��뿪����
*/
//------------------------------------------------------------------//
#include "SysDefines.h"

#define T_Debug(x,y...)

#define TFUNC_RCD_NUM 32//�ܼ�¼��

#if 1
static volatile u32 gTfThisTimCnt=0;//���ζ�ʱ����

//��ʱ����λSec
volatile u32 gSecFuncRtc=0;//�ᱻ�ⲿ���ж����ã��ݼ�
#define TfTimSet(Val) {gSecFuncRtc=Val;gTfThisTimCnt=Val;} //�趨��ʱ��
#define TfTimCancle() {gSecFuncRtc=0;}   //ȡ����ʱ��
#define GetTfTimCount() (gTfThisTimCnt-gSecFuncRtc)			//��ȡ��ʱ�����˶��

#endif

//�������ݲ�ͬ�ο�ʱ��Ķ�ʱ��������һ��


typedef struct{
	u8 Flag;//������ʹ��ʱ����idx+1
	u16 EntresCnt;//�����������һ�ν���Ϊ1���Դ�����
	u32 IntParam;//���������ص�api��
	void *pParam;//���������ص�api��
	u32 ThisRem;//����ʣ��ʱ�䣬��ʱ������ʱ����ֵ���
	u32 TotalRem;//����ʣ��ʱ��.ThisRem+TotalRem=ʵ���ܵ���ʱ�䣬��ʱ������ʱ����ֵ���
	u32 Interval;//�������ʱ��
	u32 TotalTim;// ��ʼ������ʱ��
	pStdFunc pStdCB;//�ص�����
	pExpFunc pExpCB;//�ص���������һ������������¼EntresCnt
}TFUNC_RCD;

typedef struct{
	bool NeedExcFlag;
	u16 EntresCnt;
	u32 IntParam;//���������ص�api��
	void *pParam;//���������ص�api��
	pStdFunc pStdCB;//�ص�����
	pExpFunc pExpCB;//�ص���������һ������������¼EntresCnt
}FUNC_CB_INFO;


static volatile TFUNC_RCD gTfRcd[TFUNC_RCD_NUM];//��¼��
static volatile TFUNC_RCD *gpNearest=NULL;//��̶�ʱ�ļ�¼


#define PrintNowTime()  T_Debug("                                                 @ %d Sec\r",GetRtcCount()-SysVars()->SysStartRtc);

void SecFuncRcdDisp(void)
{
	u8 i;
	u8 n=0xff;

	
	if(gpNearest!=NULL)
	{
		n=((u32)gpNearest-(u32)gTfRcd)/sizeof(TFUNC_RCD);
	}

	Debug("\n\r---------------------------[TIMING FUNC LIST]---------------------------\n\r");
	for(i=0;i<TFUNC_RCD_NUM;i++)
	{
		if(gTfRcd[i].Flag == 0) continue;
		if(gTfRcd[i].pStdCB) Debug("AddSecFunc%x[%u]:%u ,",gTfRcd[i].pStdCB,i,gTfRcd[i].IntParam);
		else Debug("AddSecFunc%x[%u]:%u ,",gTfRcd[i].pExpCB,i,gTfRcd[i].IntParam);
		Debug("ThisRem:%d,TotalRem:%d,Interval:%d %s\n\r",
			gTfRcd[i].ThisRem,gTfRcd[i].TotalRem,gTfRcd[i].Interval,n==i?"<--":"");
	}
	Debug("------------------------------------------------------------------------\n\r\n\r");
}

void SecFuncInit(void)
{
	TfTimSet(0);
	MemSet((void *)gTfRcd,0,sizeof(gTfRcd));
}

//��ʱTimCnt���ں�ִ�лص�����
//���Interval��Ϊ�㣬����Interval��ִ�лص�
//�ص������������жϻ���������ִ��
//�ص�������pRecord->TotalRem == 0��ʾ��ʱ��������
//�����Ϊ�㣬���ʾ����ص�
//TimCntΪ0����ִ�и����ȼ���next func
//���Interval==0��pCallBack����ΪpStdFunc�����Ҫע��
//��TimCnt==-1ʱ������ѭ����IntervalΪѭ��ʱ��
bool AddSecFunc(u32 TimCnt,u32 Interval,pExpFunc pCallBack,int IntParam,void *pParam)
{
	u8 i;
	TFUNC_RCD *pNewRecord;
	IntSaveInit();
	
	if(pCallBack==NULL) return FALSE;

	if(Interval)//TimCnt==0����1�������ֻ�������ж�pcallback�����ͣ������ν���
	{
		if(TimCnt==0) return AddNextExpFunc(TRUE,pCallBack,1,IntParam,pParam);
		//else if(TimCnt==1) return AddNextExpFunc(FALSE,pCallBack,1,IntParam,pParam);
	}
	else
	{
		if(TimCnt==0) return AddNextStdFunc(TRUE,(pStdFunc)pCallBack,IntParam,pParam);
		//else if(TimCnt==1) return AddNextStdFunc(FALSE,(pStdFunc)pCallBack,IntParam,pParam);
	}

	PrintNowTime();
	T_Debug(" +++ SecFunc:%dSec[%d]\n\r",TimCnt,Interval);
	
	EnterCritical();
	for(i=0;i<TFUNC_RCD_NUM;i++)//�ҿ�λ��
	{
		if(gTfRcd[i].Flag == 0) break;
	}
	if(i==TFUNC_RCD_NUM) 
	{
		for(i=0;i<TFUNC_RCD_NUM;i++)
		{
			Debug("TF[%d]:%x\n\r",i,gTfRcd[i].pExpCB!=NULL?(u32)gTfRcd[i].pExpCB:(u32)gTfRcd[i].pStdCB);
		}
		Debug("SecFunc FULL!\r\n");
		LeaveCritical();
		RebootBoard();
		while(1);
	}
	
	//��¼�µ�
	pNewRecord=(void *)&gTfRcd[i];
	pNewRecord->Flag=i+1;
	pNewRecord->TotalTim=TimCnt;
	if(Interval)//�м��ʱ��
	{
		pNewRecord->ThisRem=Interval;
		if(pNewRecord->TotalTim==(u32)-1) pNewRecord->TotalRem=(u32)-1;
		else pNewRecord->TotalRem=TimCnt-Interval;
		pNewRecord->pStdCB=NULL;
		pNewRecord->pExpCB=pCallBack;
	}
	else//һ����
	{
		pNewRecord->ThisRem=TimCnt;
		pNewRecord->TotalRem=0;
		pNewRecord->pStdCB=(pStdFunc)pCallBack;
		pNewRecord->pExpCB=NULL;
	}
	pNewRecord->Interval=Interval;
	pNewRecord->IntParam=IntParam;
	pNewRecord->pParam=pParam;
	pNewRecord->EntresCnt=0;

	//�ҵ���С�Ķ�ʱ
	if(gpNearest == NULL)//δ�洢��ֱ�Ӹ�ֵ
	{
		gpNearest=pNewRecord;

		//������Сֵ��ʱ
		TfTimSet(gpNearest->ThisRem);
	}
	else
	{
		u32 TimCnt=GetTfTimCount();
		if(TimCnt>gpNearest->ThisRem) TimCnt=gpNearest->ThisRem;//��ֹ�����������
		T_Debug(" Rem(%d)-Cnt(%d) <? New(%d)\n\r",gpNearest->ThisRem,GetTfTimCount(),pNewRecord->ThisRem);
		if((gpNearest->ThisRem-TimCnt) <= pNewRecord->ThisRem)//��Сֵ��ȻС
		{
			T_Debug(" XXX No Need Change TimeSet\n\r");
			pNewRecord->ThisRem+=TimCnt;//�ȼ�����ȥ��ʱ�䣬���㵽�ڵ�ʱ��۵�
			LeaveCritical();
			//SecFuncRcdDisp();
			return TRUE;
		}
		else//�¼ӽ����ĸ������ı�ָ��
		{
			T_Debug(" XXX Change Nearest\n\r",gpNearest->ThisRem,TimCnt);

			//���е�δ���ڵ�ʱ�Ӷ�Ҫ��ȥ��ȥ��ʱ��
			{
				TFUNC_RCD *pRecord;
				for(i=0,pRecord=(void *)gTfRcd;i<TFUNC_RCD_NUM;i++,pRecord++)//�����Щ��Ҫ����
				{
					if(pRecord->Flag == 0) continue;
					if(pRecord == pNewRecord) continue;
					pRecord->ThisRem-=TimCnt;
				}
			}			
			
			gpNearest=pNewRecord;

			//������Сֵ��ʱ
			TfTimSet(gpNearest->ThisRem);
		}
	}

	LeaveCritical();
	//SecFuncRcdDisp();
	return TRUE;
}

//��Ӳ���Ҫ��ʱ����Interval�Ķ�ʱ����
//��ʱTimCnt���ں�ִ�лص�����
//�ص������������жϻ���������ִ��
//TimCntΪ0����ִ�и����ȼ���next func
bool AddOnceSecFunc(u32 TimCnt,pStdFunc pCallBack,int IntParam,void *pParam)
{
	return AddSecFunc(TimCnt,0,(pExpFunc)pCallBack,IntParam,pParam);
}

//Ŀǰ��main��ִ��
void SecFuncExpired(void)
{
	u16 i;
	u32 MinVal=0xffffffff;
	TFUNC_RCD *pRecord;
	FUNC_CB_INFO *pCbInfo=Q_Malloc(sizeof(FUNC_CB_INFO)*TFUNC_RCD_NUM);
	u32 ThisTimCnt=GetTfTimCount();
	
	IntSaveInit();
	
	PrintNowTime();
	T_Debug(" --- NowConsume:%dSec\n\r",ThisTimCnt);

	EnterCritical();
	for(i=0,pRecord=(void *)gTfRcd;i<TFUNC_RCD_NUM;i++,pRecord++)//�����Щ��Ҫ����
	{
		pCbInfo[i].NeedExcFlag=FALSE;
		if(pRecord->Flag == 0) continue;
		if(pRecord->ThisRem <= ThisTimCnt)//�����˵�ʱ��
		{
			pRecord->ThisRem=0;
			pRecord->EntresCnt++;
			
			pCbInfo[i].NeedExcFlag=TRUE;//�����				
			pCbInfo[i].EntresCnt=pRecord->EntresCnt;
			pCbInfo[i].IntParam=pRecord->IntParam;//������Ϣ���ص���
			pCbInfo[i].pParam=pRecord->pParam;
			pCbInfo[i].pStdCB=pRecord->pStdCB;
			pCbInfo[i].pExpCB=pRecord->pExpCB;

			if(pRecord->Interval == 0) //û�м��
			{
				PrintNowTime();
				T_Debug(" --- SecFunc:%dSec\n\r",pRecord->TotalTim);
				MemSet((void *)pRecord,0,sizeof(TFUNC_RCD));	//���ټ�¼	
			}
			else //�м������
			{
				//׼���´ζ�ʱ
				if(pRecord->TotalRem == 0)//�����϶��
				{	
					PrintNowTime();
					T_Debug(" --- SecFunc:%dSec\n\r",pRecord->TotalTim);
					MemSet((void *)pRecord,0,sizeof(TFUNC_RCD));	//���ټ�¼
				}
				else if(pRecord->TotalRem < pRecord->Interval) //С�ڼ�϶
				{
					PrintNowTime();
					T_Debug(" ||| SecFunc:%dSec\n\r",pRecord->TotalTim);
					pRecord->ThisRem=pRecord->TotalRem;
					pRecord->TotalRem=0;
				}
				else //���ж�μ�϶
				{
					PrintNowTime();
					T_Debug(" ||| SecFunc:%dSec\n\r",pRecord->TotalTim);
					pRecord->ThisRem=pRecord->Interval;
					if(pRecord->TotalTim!=(u32)-1) pRecord->TotalRem-=pRecord->Interval;
				}	
			}
		}
		else
		{
			pRecord->ThisRem-=ThisTimCnt;
		}
	}

	gpNearest=NULL;
	for(i=0,pRecord=(void *)gTfRcd;i<TFUNC_RCD_NUM;i++,pRecord++)
	{
		if(pRecord->Flag == 0) continue;
		if(pRecord->ThisRem < MinVal) 
		{
			MinVal=pRecord->ThisRem;
			gpNearest=pRecord;
		}
	}
		
	if(gpNearest != NULL)//�Ѿ��ҵ���һ�ε���С��ʱ��
	{
		TfTimSet(gpNearest->ThisRem);	
	}
	else
	{
		TfTimCancle();
	}
	LeaveCritical();
	
	//SecFuncRcdDisp();

	//��ʼִ��callback
	for(i=0;i<TFUNC_RCD_NUM;i++)
	{		
		FUNC_CB_INFO *pCb=&pCbInfo[i];
		if(pCb->NeedExcFlag==TRUE)
		{
			if(pCb->pStdCB!=NULL) pCb->pStdCB(pCb->IntParam,pCb->pParam);
			else if(pCb->pExpCB!=NULL) pCb->pExpCB(pCb->EntresCnt,pCb->IntParam,pCb->pParam);
		}				
	}

	Q_Free(pCbInfo);
}


//���һ��cb�Ƿ��Ѿ������õ���ʱ������
bool SecFuncAlready(void *pCB)
{
	u8 i;
	IntSaveInit();

	if(pCB!=NULL)
	{
		EnterCritical();
		for(i=0;i<TFUNC_RCD_NUM;i++)
		{
			if(gTfRcd[i].Flag == 0) continue;

			if(gTfRcd[i].pStdCB==pCB || gTfRcd[i].pExpCB==pCB)
			{
				LeaveCritical();
				return TRUE;
			}			
		}	
		LeaveCritical();
	}
	
	return FALSE;
}

//ɾ������ͬһ���ص������Ķ�ʱ����
void DeleteSecFuncByCB(void *pCB)
{
	TFUNC_RCD *pRecord;
	u32 MinVal=0xffffffff;
	u16 i;
	IntSaveInit();

	if(pCB!=NULL)
	{
		EnterCritical();
		for(i=0;i<TFUNC_RCD_NUM;i++)
		{
			if(gTfRcd[i].Flag == 0) continue;

			if(gTfRcd[i].pStdCB==pCB || gTfRcd[i].pExpCB==pCB)
			{
				MemSet((void *)&gTfRcd[i],0,sizeof(TFUNC_RCD));	//���ټ�¼	
			}			
		}

		//��������Ķ�ʱ����
		gpNearest=NULL;
		for(i=0,pRecord=(void *)gTfRcd;i<TFUNC_RCD_NUM;i++,pRecord++)
		{
			if(pRecord->Flag == 0) continue;
			if(pRecord->ThisRem < MinVal) 
			{
				MinVal=pRecord->ThisRem;
				gpNearest=pRecord;
			}
		}
			
		if(gpNearest != NULL)//�Ѿ��ҵ���һ�ε���С��ʱ��
		{
			TfTimSet(gpNearest->ThisRem);	
		}
		else
		{
			TfTimCancle();
		}
		
		LeaveCritical();
	}
}

