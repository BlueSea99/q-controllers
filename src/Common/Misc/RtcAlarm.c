#include "SysDefines.h"

static volatile u32 gNowAlarmID=0;//��¼��ǰ����id

//�Ƚ�����ʱ���С��
//time1�󣬷�����ֵ
//time2�󣬷��ظ�ֵ
//һ���󣬷���0
s32 AlarmCompare(RTC_TIME *pTime1,RTC_TIME *pTime2)
{
	u32 Time1;
	u32 Time2;

	//����������������Բ�����RtcTime2Cnt���бȽ�
	Time1=(pTime1->year<<20)+(pTime1->mon<<16)+(pTime1->day<<11)+(pTime1->hour<<6)+(pTime1->min);
	Time2=(pTime2->year<<20)+(pTime2->mon<<16)+(pTime2->day<<11)+(pTime2->hour<<6)+(pTime2->min);
	
	return Time1-Time2;
}

//������������ó���ʵ�Ĵ���ʱ��
//�������Ӽ�¼�����ش���ʱ��
bool AlarmRecord2Time(RTC_TIME *pTime,ALARM_IN_DEV *pRcd)
{
	if(pRcd->Enable == FALSE) return FALSE;
	if(pRcd->AlarmDays==0 && pRcd->OnceAlarm==0) return FALSE;
	
	if(pRcd->OnceAlarm)//ֱ������
	{
		pTime->year=pRcd->Years;
		pTime->mon=pRcd->Mon;
		pTime->day=pRcd->Day;
		pTime->hour=pRcd->Hour;
		pTime->min=pRcd->Min;
		pTime->sec=0;
		return TRUE;
	}
	else if(pRcd->AlarmDays)//ѭ������
	{
		RTC_TIME NowTime;
		u8 Day;
		u8 i;
		bool Flag=FALSE;
		
		RtcReadTime(&NowTime,RtcOp_GetTime);

		for(i=NowTime.week;i<(NowTime.week+7);i++)//�ӵ����������Ŀ��ʼ
		{
			if(i >= 7) Day=i-7;
			else Day=i;
			
			if(ReadBit(pRcd->AlarmDays,Day)==0) continue;//���ղ�����
			
			if(Day == NowTime.week)//�жϵ���
			{
				if(((NowTime.hour<<16)+(NowTime.min<<8)+NowTime.sec) < ((pRcd->Hour<<16)+(pRcd->Min<<8))) //ʱ�仹û��
				{//����Ϊ����
					pTime->year=NowTime.year;
					pTime->mon=NowTime.mon;
					pTime->day=NowTime.day;
					pTime->hour=pRcd->Hour;
					pTime->min=pRcd->Min;
					pTime->sec=0;
					return TRUE;					
				}
				else//ʱ���Ѿ����ˣ��ŵ��������
				{
					Flag=TRUE;
				}
			}
			else//�ǵ���
			{
				u32 TimeCnt=0;

				if(Day<NowTime.week) Day+=7;//�ж�����
				
				pTime->year=NowTime.year;
				pTime->mon=NowTime.mon;
				pTime->day=NowTime.day;
				pTime->hour=pRcd->Hour;
				pTime->min=pRcd->Min;
				pTime->sec=0;

				TimeCnt=RtcTime2Cnt(pTime);//�õ�����ļ���
				if(TimeCnt == 0) return FALSE;//������Ҫ��
				TimeCnt+=(Day-NowTime.week)*86400;//һ�������86400

				RtcCnt2Time(TimeCnt,pTime);//ת���ض�ʱʱ��
				return TRUE;	
			}			
		}	

		if(Flag) //�ŵ����ܵĽ��죬����ʱ����������
		{
			u32 TimeCnt=0;

			//�õ����������ʱ��(�ѹ�)
			pTime->year=NowTime.year;
			pTime->mon=NowTime.mon;
			pTime->day=NowTime.day;
			pTime->hour=pRcd->Hour;
			pTime->min=pRcd->Min;
			pTime->sec=0;

			//�ѽ��������ʱ��+7��
			TimeCnt=RtcTime2Cnt(pTime);//�õ�����ļ���
			if(TimeCnt == 0) return FALSE;//������Ҫ��
			TimeCnt+=7*86400;//һ�������86400

			RtcCnt2Time(TimeCnt,pTime);//ת���ض�ʱʱ��
			return TRUE;	
		}
	}	

	return FALSE;
}

//�����������
bool AlarmClean(void)
{
	RTC_TIME Alarm;

	gNowAlarmID=0;

	//���õ��������Ժ�
	Alarm.year=2047;
	Alarm.mon=1;
	Alarm.day=1;
	Alarm.hour=Alarm.min=Alarm.sec=0;
	
	if(RtcAdjustTime(&Alarm,RtcOp_SetAlarm)==FALSE)
	{
		Debug("Alarm Clean Failed!\r\n");
		return FALSE;
	}
	
	return TRUE;
}

//�������ݿ���������
bool AlarmTidy(INFO_SITE *pSiteList,u16 Num)
{
	IN_DEVICE_RECORD InDev;
	ALARM_IN_DEV *pAlaram=&(InDev.Record.Alarm);
	RTC_TIME Now;
	RTC_TIME Alarm;
	bool ChangeFlag=FALSE;
	s16 Res,i;

	if(pSiteList==NULL) return TRUE;
	if(Num==0) return TRUE;

	RtcReadTime(&Now,RtcOp_GetTime);//��ȡ��ǰʱ��
	RtcReadTime(&Alarm,RtcOp_GetAlarm);//��ȡ��ǰ����

	if(AlarmCompare(&Now,&Alarm)>=0)//����ʱ�����ڵ�ǰ����ʾ����������
	{
		AlarmClean();		
		RtcReadTime(&Alarm,RtcOp_GetAlarm);
	}

	Debug("NowTim:%4d-%02d-%02d %02d:%02d:%02d\n\r",Now.year,Now.mon,Now.day,
			Now.hour,Now.min,Now.sec);
	Debug("NowAlm:%4d-%02d-%02d %02d:%02d:%02d\n\r\n\r",Alarm.year,Alarm.mon,Alarm.day,
			Alarm.hour,Alarm.min,Alarm.sec);
			
	for(i=0;i<Num;i++)//�����ȡ����
	{
		Res=ReadInfoBySite(IRN_IN_DEV,pSiteList[i],&InDev,sizeof(IN_DEVICE_RECORD));
		if(Res<0) break;
		if(InDev.DevType != IDT_ALARM) continue;

		if(pAlaram->Enable)//�г�ÿ����Ч������
		{
			RTC_TIME New;//������Ӽ�¼ת���ɱ�׼RTC�������

			//ת����ʽ
			if(AlarmRecord2Time(&New,pAlaram)==FALSE) continue;
			Debug(" Rcd:%4d-%02d-%02d %02d:%02d [%02x]\n\r",
				pAlaram->Years,pAlaram->Mon,pAlaram->Day,pAlaram->Hour,pAlaram->Min,pAlaram->OnceAlarm?0x80:pAlaram->AlarmDays);
			Debug(" Rtc:%4d-%02d-%02d %02d:%02d:%02d",New.year,New.mon,New.day,
			New.hour,New.min,New.sec);

			if(AlarmCompare(&Now,&New)>=0)//��ʱ�����ڵ�ǰ����ʾ��ʱ������
			{
				Debug(" ----\r\n\r\n");
				pAlaram->Enable=FALSE;//�޸�ʹ��
				CoverInfoBySite(IRN_IN_DEV,0xffff,Res,&InDev,sizeof(IN_DEVICE_RECORD));//����ԭ��Ϣ
				continue;
			}

			//�Ƚ���Сֵ
			if(AlarmCompare(&Alarm,&New)>0)//��ʱ��ȽϿ�ǰ
			{
				Debug(" <<>>");
				ChangeFlag=TRUE;
				MemCpy(&Alarm,&New,sizeof(RTC_TIME));
			}		

			Debug("\r\n\r\n");
		}	
	}

	if(ChangeFlag)
	{
		Debug("ChaAlm:%4d-%02d-%02d %02d:%02d:%02d\n\r",Alarm.year,Alarm.mon,Alarm.day,
			Alarm.hour,Alarm.min,Alarm.sec);
			
		if(RtcAdjustTime(&Alarm,RtcOp_SetAlarm)==FALSE)
			return FALSE;
	}

	return TRUE;
}

//���ӵ��ڴ���
void AlarmExpireHandler(void)
{
	RTC_TIME Now;

	//IOOUT_SetIoStatus(IOOUT_BEEP,TRUE);//��ʵ�ֶ�ʱ��Ƶ��
	
	RtcReadTime(&Now,RtcOp_GetTime);
	Now.sec=0;
	AlarmTriggerIn(RtcTime2Cnt(&Now));
}






