//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ���װ��ir�����⣬�ɱ������������������������Ŀ�����ٴ��뿪����
*/
//------------------------------------------------------------------//

#include "Drivers.h"
#include "IrOut.h"
#include "Q_Heap.h"

#define DECODE_38K_MYSELF 0//�Լ�����38kHz������
static u32 gLast13usPluseTimeCnt=0;//38k�������һ��������ʱ�����

//Ӳ����������
#define IrTimer_ISR TIM2_IRQHandler
#define IrTimerSet Tim2_Set
#define IrlTimerID TIM2
#define WorkLed(x) do{}while(0);
#define StudyLed(x) do{LedSet(IOOUT_LED1,(x)?1:0);}while(0);
#define FireLed(x) do{LedSet(IOOUT_LED2,(x)?1:0);}while(0);

//��������趨
#define IR_RECV_PLUSE_MIN_LIMIT 8//��С�����������
#define IR_RECV_MAX_WAVE_SPACE_MS 50 //�������ʱ��������λms
#define IR_RECV_MAX_TIMING_US 50000 //����ʱ��ʱ�����ʱ
#define IR_CHECK_TIME_UNIT_US 1 //��ʱ����׼
#define IR_PULSE_RECORD_NUM 1024//��¼�������
#define IR_PULSE_BUF_LEN (sizeof(u16)*IR_PULSE_RECORD_NUM)

#define OS_EnterCritical()
#define OS_ExitCritical()
#define OS_TaskYield()

#if 1
/********************************* IR Recv *******************************************/
static volatile bool gIsRecvState=FALSE;//�Ƿ��ڽ���״̬
static volatile u16 *gpIrPluseTimes=NULL;//��¼ÿ����ƽ����ʱ��
static volatile u16 gIrPluseNum=0;//��ǰ��¼�ĵ�ƽ����
static IR_RECORD gIrRecord;

void DisplayIrRecord(const IR_RECORD *pIrRcd)
{
	u16 i;
	u16 n;

	if(pIrRcd==NULL) return;

	Debug("IR Idx: ");
	for(i=0;i<MAX_IR_IDX_NUM;i++) 
	{
		if(pIrRcd->IdxTimes[i]==0) break;
		if(pIrRcd->IdxTimes[i]&0x01) //�ߵ�ƽ
		{
			Debug("-");
		}
		else
		{
			Debug("_");
		}
		
		Debug("%u ",pIrRcd->IdxTimes[i]);
	}
	Debug("\n\r");

	Debug("IR List[%u]:\n\r",pIrRcd->PulseNum);
	if(pIrRcd->PulseNum>MAX_IR_PLUSE_NUM) return;

	for(i=0;i<pIrRcd->PulseNum;i++)
	{
		if(i&0x01) //�ߵ�ƽ
		{
			n=pIrRcd->Pluse[i>>1]>>4;Debug("-");
		}
		else
		{
			n=pIrRcd->Pluse[i>>1]&0x0f;Debug("_");
		}

		Debug("%u",pIrRcd->IdxTimes[n]);
		//if(i%16==(16-1)) Debug("\n\r");
	}	

	Debug("\n\r\n\r");
}

//��ʼ���պ����ź�
void StartRecvIr(void)
{
	gIsRecvState=TRUE;
	MemSet(&gIrRecord,0,sizeof(gIrRecord));
	if(gpIrPluseTimes==NULL)
	{
		gpIrPluseTimes=Q_Malloc(IR_PULSE_BUF_LEN);
		//MemSet((void *)gpIrPluseTimes,0,IR_PULSE_BUF_LEN);
	}
	gIrPluseNum=0;
	WorkLed(1);
	StudyLed(0);
	IOIN_OpenExti(IOIN_IR_IN);
}

//�رս��պ����ź�
void StopRecvIr(void)
{
	IOIN_CloseExti(IOIN_IR_IN);
	WorkLed(0);
	StudyLed(0);
	gIsRecvState=FALSE;
	gIrPluseNum=0;
	if(gpIrPluseTimes)
	{
		Q_Free(gpIrPluseTimes);
		gpIrPluseTimes=NULL;
	}
}

//��ȡ����ĺ����źţ�ֻ��ȡһ��
bool CaptureRecvIr(IR_RECORD *pIr)
{
	MemCpy(pIr,&gIrRecord,sizeof(IR_RECORD));
	MemSet(&gIrRecord,0,sizeof(IR_RECORD));

	if(pIr->Pluse==0 || pIr->Type!=SIT_IR) return FALSE;

	return TRUE;
}

//�����ڲ�����
IR_RECORD *SetCaptureBuf(IR_RECORD *pIr)
{
	MemCpy(&gIrRecord,pIr,sizeof(IR_RECORD));

	return &gIrRecord;
}

//������ͬ����������бȽ�
static u8 ComparePluseToIdx(u16 Num,u16 *pItem)
{
	u16 i;

	for(i=0;i<MAX_IR_IDX_NUM;i++)
	{
		if(pItem[i]==0) return 0;
		if(((pItem[i]^Num)&0x01)==0)//ͬ�Ǹߵ�ƽ����ͬ�ǵ͵�ƽ
		{
			if(FuzzyEqual(Num,pItem[i],IR_FUZZY_PARAM)==TRUE) return i+1;
		}
	}

	return 0;
}

//�������ε���¼��
static bool RecordIrPluse(const u16 *pPluseMs,u16 Num,IR_RECORD *pIrRcd)
{
	u16 Res,n;
	u16 IdxCnt=0;

	if(pIrRcd==NULL) return FALSE;
	MemSet(pIrRcd,0,sizeof(IR_RECORD));

	//Debug("Pluse[%u]\n\r",Num);
	//DisplayBufU16_Dec(pPluseMs,Num,16);

	if(Num<IR_RECV_PLUSE_MIN_LIMIT) return FALSE;
	if(Num>MAX_IR_PLUSE_NUM) return FALSE;
	if(Num&0x01==0) return FALSE;//��¼��������Ϊ��������ʼ����

	for(n=0;n<Num;n++)
	{
		if(pPluseMs[n]==0) break;
		Res=ComparePluseToIdx(pPluseMs[n],pIrRcd->IdxTimes);
		if(Res==0)//δ�Ƚϳ��ݲ�ֵ������������
		{
			if(IdxCnt>=MAX_IR_IDX_NUM) 
			{
				Debug("Res Need too big!\n\r");
				return FALSE;//����ʧ�ܣ�����������
			}
			pIrRcd->IdxTimes[IdxCnt]=pPluseMs[n];
			IdxCnt++;
			Res=IdxCnt;
		}
		else if(Res>MAX_IR_IDX_NUM)
		{
			Debug("Idx error!\n\r");
			return FALSE;
		}

		if(n&0x01) //�ߵ�ƽ
		{
			pIrRcd->Pluse[n>>1]|=((Res-1)<<4);//��4λ
		}
		else
		{
			pIrRcd->Pluse[n>>1]=(Res-1)&0x0f;//��4λ
		}
	}

	pIrRcd->PulseNum=n;	
	pIrRcd->Type=SIT_IR;
	pIrRcd->SendCnt=0;

	//DisplayIrRecord(pIrRcd);

	return TRUE;
}

//���������ݼ�¼������
void StoragePluseTime2Array(bool IrFire,u16 TimeCnt)
{
	if(gIrPluseNum == 0) StudyLed(1);

	if(gIrPluseNum < IR_PULSE_RECORD_NUM)
	{
		gpIrPluseTimes[gIrPluseNum]=TimeCnt;//��¼��ƽʱ��
		if(IrFire) gpIrPluseTimes[gIrPluseNum]|=1;//���λΪ1����ʾ��ir�ź�
		else gpIrPluseTimes[gIrPluseNum]&=~1;//���λΪ0����ʾir�źž�Ĭ
		gIrPluseNum++;
	}
}

//�û��������IR����ܽ��е�ƽ�仯ʱ���ã����ж��н���
//ir���սţ���̬��1�����յ��źű�0�������жϽ���ʱ��io״̬�ͽ���ʱ�պ��෴
void IrPulseIn_ISR(void)
{
	u32 Count=TIM_GetCounter(IrlTimerID);
	bool IrFireEnd=IOIN_ReadIoStatus(IOIN_IR_IN)?TRUE:FALSE;
	Debug("%c",IrFireEnd?'-':'-');
	if(Count==0) 
	{
#if DECODE_38K_MYSELF //�Լ�����38k
		gLast13usPluseTimeCnt=0;
#endif
		gIrPluseNum=0;
		IrTimerSet(IR_RECV_MAX_TIMING_US,IR_CHECK_TIME_UNIT_US,FALSE);//	����ÿ1us����һ�������Ķ�ʱ�������50ms	
		return;
	}
	
#if DECODE_38K_MYSELF //�Լ�����38k
	if(Count-gLast13usPluseTimeCnt < 15)//38k��һ������13us
	{
		gLast13usPluseTimeCnt=Count;
		return;
	}
	else //
	{
		if(IrFireEnd)
		{
			return;//�����г���13us��fire���壬����ֱ�ӷ���
		}
		else
		{
			StoragePluseTime2Array(TRUE,gLast13usPluseTimeCnt);//��¼ǰ���fire�ܳ�
			StoragePluseTime2Array(FALSE,Count);//��¼�źž�Ĭ�ܳ�
			gLast13usPluseTimeCnt=0;
		}
	}	
#else
	StoragePluseTime2Array(IrFireEnd,Count);
#endif	

	IrTimerSet(IR_RECV_MAX_TIMING_US,IR_CHECK_TIME_UNIT_US,FALSE);//	����ÿ1us����һ�������Ķ�ʱ�������50ms	
}

//ir���崦����ɺ�Ĵ������ж��д���
//������Ϻ󣬻�õ���ȷ��gpIrRecvDatas���
static void IrRecvEnd_ISR(void)
{
	bool Res;	
	bool NeedYield;

	StudyLed(0);

#if DECODE_38K_MYSELF //�Լ�����38k
	if(gLast13usPluseTimeCnt)	//�������һ���ź������ʱ��
	{
		StoragePluseTime2Array(TRUE,gLast13usPluseTimeCnt);
	}
#endif
	
	OS_EnterCritical();
	Res=RecordIrPluse((void *)gpIrPluseTimes,gIrPluseNum,&gIrRecord);
	gIrPluseNum=0;
	MemSet((void *)gpIrPluseTimes,0,IR_PULSE_BUF_LEN);
	OS_ExitCritical();
	
	if(Res)//��������Ч�ź�
	{
		//SysEventSend(SEN_IR_CAPTURE,0,&gIrRecord,&NeedYield);
	}
	
	DisplayIrRecord(&gIrRecord);

	if(NeedYield)
	{
		//Debug("NeedYeild %u\n\r",OS_GetNowMs());
		OS_TaskYield();
	}	
}
#endif

#if 1
/********************************* IR Send *******************************************/
static volatile u16 gIrSendBitNum=0;//�Ѿ����͵��������
static volatile u16 gIrSendPluseNum=0;//��ǰ��¼�ĵ�ƽ����
static volatile u16 *gpIrSendPluseTimes;//��¼ÿ����ƽ����ʱ��
static volatile bool gSendingFlag=FALSE;//���ڷ��ͱ�ʶ

//�����ݻָ��ɵ�ƽʱ��
//�����������
static u16 RestorePluseTime(u16 *pPluseTimes,const IR_RECORD *pIrRcd)
{
	u16 i,n;
	
	if(pPluseTimes==NULL) return 0;
	if(pIrRcd==NULL) return 0;
	if(pIrRcd->PulseNum>MAX_IR_PLUSE_NUM) return 0;

	for(i=0;i<pIrRcd->PulseNum;i++)
	{
		if(i&0x01) //�ߵ�ƽ
		{
			n=pIrRcd->Pluse[i>>1]>>4;
		}
		else
		{
			n=pIrRcd->Pluse[i>>1]&0x0f;
		}

		pPluseTimes[i]=pIrRcd->IdxTimes[n];
	}
	pPluseTimes[i]=0;

	return pIrRcd->PulseNum;
}

//������ɺ�Ļص�
static void IrSent_CB(void)
{
	if(gpIrSendPluseTimes)
	{
		Q_Free(gpIrSendPluseTimes);
		gpIrSendPluseTimes=NULL;
	}
	
	gSendingFlag=FALSE;
	FireLed(0);
}

//pIrRecord���洢������Ϣ�����飬ΪNULLʱ���͸ո��յ���ir
//SendQueueLen�������Ա����
//�������ж��е���
void StartSendIr(const IR_RECORD *pIrRcd)
{	
	StopRecvIr();

	if(gSendingFlag==TRUE)//���������
	{ 
		Debug("Old UnFinish\n\r");
		return;//�ɵ�û�����꣬�˳�
	}

	gSendingFlag=TRUE;

	if(pIrRcd == NULL) //������
	{
		pIrRcd=&gIrRecord;
	}

	//DisplayIrRecord(pIrRcd);

	if(gpIrSendPluseTimes==NULL) gpIrSendPluseTimes=Q_Malloc(IR_PULSE_BUF_LEN);
	//MemSet((void *)gpIrSendPluseTimes,0,sizeof(gpIrSendPluseTimes));
	gIrSendPluseNum=RestorePluseTime((void *)gpIrSendPluseTimes,pIrRcd);
	if(0)//for debug
	{
		u16 i;
		Debug("Send[%u]:",gIrSendPluseNum);
		for(i=0;i<gIrSendPluseNum;i++)Debug("%c%u",(i&0x01)?'-':'_',gpIrSendPluseTimes[i]);
	}

	if(gIrSendPluseNum == 0)//������
	{
		gSendingFlag=FALSE;
		return;
	}

	gIrSendBitNum=0;
	SetSendIrData(1);//��ʼ���Ͳ���
	//Debug("_%u",gpIrSendPluseTimes[0]);
	IrTimerSet(gpIrSendPluseTimes[0],IR_CHECK_TIME_UNIT_US,FALSE);//	����ÿ1us����һ�������Ķ�ʱ��
	FireLed(1);
	
	//while(gSendingFlag==TRUE);//�ȴ����ͽ���
}

//��������Ķ�ʱ���ж�
static void IrSend_ISR(void)
{
	if(gIrSendBitNum < gIrSendPluseNum)
	{
		gIrSendBitNum++;
		SetSendIrData(gIrSendBitNum&0x01);		
		//if(gIrSendBitNum&0x01) Debug("-%u",gpIrSendPluseTimes[gIrSendBitNum]);
		//else Debug("_%u",gpIrSendPluseTimes[gIrSendBitNum]);
		if(gpIrSendPluseTimes[gIrSendBitNum]==0)//����ĩβ�ˣ�����ȫ�����ִ꣬��wave space��ʱ
		{
			gIrSendBitNum = gIrSendPluseNum;
			SetSendIrData(0);//ֹͣ���Ͳ���
			IrTimerSet(0,IR_CHECK_TIME_UNIT_US,FALSE);
			IrSent_CB();
		}
		else
		{
			IrTimerSet(gpIrSendPluseTimes[gIrSendBitNum],IR_CHECK_TIME_UNIT_US,FALSE);
		}
	}
	else if(gIrSendBitNum == gIrSendPluseNum)
	{
		SetSendIrData(0);//ֹͣ���Ͳ���
		IrTimerSet(0,IR_CHECK_TIME_UNIT_US,FALSE);
		IrSent_CB();
	}
	else 
	{
		Debug("IrSendBit %d,IrSendNum %d\n\r",gIrSendBitNum,gIrSendPluseNum);
		while(1);
	}
}
#endif

//��ʱ���жϺ���
void IrTimer_ISR(void)
{
	if(TIM_GetITStatus(IrlTimerID, TIM_IT_Update) != RESET)//�����ʱ�����ڣ�˵����ƽ����ʱ�䳤��65525us��˵���ɼ�����
	{
		TIM_ClearITPendingBit(IrlTimerID, TIM_IT_Update);		

		if(gIsRecvState)//�������
		{
			IrRecvEnd_ISR();
		}
		else//������
		{
			IrSend_ISR();
		}		
	}
}

