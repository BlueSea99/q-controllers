//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ�������һ�׻���spi flash�����ݻ�����ƣ��ɱ�����������������Ŀ�����ٴ��뿪����
�����Ƶ���ҪĿ�ģ����������Ŀ��Լ���£���¼���µ����ݣ����������Լ����Ŀʱ��
��ɵļ�¼�ᱻɾ������������ݣ�������id����ȡ��
*/
//------------------------------------------------------------------//
#include "SysDefines.h"
#include "SpiFlashApi.h"

#define GetAddr(Site) (gInfoStartAddr[Name]+((Site)-1)*gInfoAttrib[Name].BlockBytes)
#define GetSite(Addr) (((Addr)-gInfoStartAddr[Name])/gInfoAttrib[Name].BlockBytes+1)

typedef struct{
	u8 StartSector;//��ʼ����
	u8 BackupSector;//��������
	u8 SectorNum;//ռ��������
	u16 BlockBytes;//��Ԫ�ֽ�������8��ͷ�ֽ�
	u16 MaxItem;//Ӧ�ó�����������ɵ�����¼������ʵ������С����ȼ���
	u16 ChkSizeof;//���ڴ�С���
}INFO_TEMP_ATTRIB;

//�û�����
#if PRODUCT_IS_JUMPER
static const INFO_TEMP_ATTRIB gInfoAttrib[ITN_MAX]={
{1,5,4,FLASH_PAGE_SIZE,IR_TEMP_MAX_NUM,sizeof(IR_RECORD)},//ir signal buf
{9,13,4,FLASH_PAGE_SIZE,RF_TEMP_MAX_NUM,sizeof(RF_RECORD)},//rf signal buf
};
#endif

//��ҳʹ��
#define INFO_NULL 0xffffffff
static INFO_ADDR gInfoStartAddr[ITN_MAX]={0};//�洢����ʼ��ַ
static INFO_ADDR gInfoEndAddr[ITN_MAX]={0};//�洢��������ַ
static u16 gInfoOccupy[ITN_MAX]={0};//ͨ������������ռ����
static INFO_SITE gIdleSite[ITN_MAX]={1};//����λ��������


//�����洢�������е���Ϣ
//������ʵ����Ч������
u16 InfoTempStatistics(INFO_TEMP_NAME Name)
{
	INFO_ADDR FlashAddr;
	INFO_ID InfoID;
	u16 Count=0;

	if(Name>=ITN_MAX) return 0;
	
	gInfoOccupy[Name]=0;

	for(FlashAddr=gInfoStartAddr[Name];FlashAddr<gInfoEndAddr[Name];FlashAddr+=gInfoAttrib[Name].BlockBytes)
	{
		Count++;
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		
		if(InfoID!=INFO_NULL)//�ҵ���Ч��Ϣ
		{
			gInfoOccupy[Name]=Count;
		}
	}

	return gInfoOccupy[Name];
}


//��ʼ��info���ݿ�
void InfoTempInit(void)
{
	INFO_TEMP_NAME Name;
	INFO_ADDR FlashAddr;
	INFO_ID InfoID;

	for(Name=(INFO_TEMP_NAME)0;Name<ITN_MAX;Name++)
	{
		if(AlignTo4(gInfoAttrib[Name].ChkSizeof)!=gInfoAttrib[Name].ChkSizeof)
		{
			Debug("[%s]InfoSize is not align 4! sizeof()=%d\n\r",gNameInfoName[Name],gInfoAttrib[Name].ChkSizeof);
			while(1);
		}
	
		if((gInfoAttrib[Name].ChkSizeof)>gInfoAttrib[Name].BlockBytes)
		{
			Debug("[%s]BlockSize is small,pls check it! %d>%d\n\r",gNameInfoName[Name],(gInfoAttrib[Name].ChkSizeof),gInfoAttrib[Name].BlockBytes);
			while(1);
		}

		if((gInfoAttrib[Name].SectorNum*FLASH_SECTOR_BYTES/gInfoAttrib[Name].BlockBytes)<gInfoAttrib[Name].MaxItem)
		{
			Debug("[%s]MaxItem is too big.%d<%d\n\r",gNameInfoName[Name],(gInfoAttrib[Name].SectorNum*FLASH_SECTOR_BYTES/gInfoAttrib[Name].BlockBytes),gInfoAttrib[Name].MaxItem);
			while(1);
		}
	}

	Debug("Name     Sector(Backup)  Vaild  Deleted  Remainder  Occupation  IdelSite  sizeof\n\r");

	for(Name=(INFO_TEMP_NAME)0;Name<ITN_MAX;Name++)
	{
		u16 Remainder=0,Vaild=0,Deleted=0;
		
		//���ҵ�ַ
		FlashAddr=gInfoAttrib[Name].BackupSector*FLASH_SECTOR_BYTES;//��������ͷ�ֽ�
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		if(InfoID == INFO_NULL)//��־Ϊ�գ���������������
		{
			Debug("            %4d        ",gInfoAttrib[Name].StartSector);
			gInfoStartAddr[Name]=gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES;
			gInfoEndAddr[Name]=(gInfoAttrib[Name].StartSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;
		}
		else//�����ڱ�����
		{
			Debug("            %4d(*)     ",gInfoAttrib[Name].BackupSector);
			gInfoStartAddr[Name]=gInfoAttrib[Name].BackupSector*FLASH_SECTOR_BYTES;
			gInfoEndAddr[Name]=(gInfoAttrib[Name].BackupSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;
		}
	
		//�ȼ���ǲ��ǺϷ�
		for(FlashAddr=gInfoStartAddr[Name];FlashAddr<gInfoEndAddr[Name];FlashAddr+=gInfoAttrib[Name].BlockBytes)
		{
			SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);

			if(InfoID == INFO_NULL) Remainder++;
			else if(InfoID == 0) Deleted++;
			else Vaild++;
		}

		//���ռ�ʣ��
		Debug("%4d      %4d      %4d      ",Vaild,Deleted,Remainder);

		if(Remainder*4 < Vaild)
		{
			Debug("Remainder info is too little!Tidy Info System!\n\r");	
		}

		//�����������������ֽ�(Ӧ��id)��ʵ�ʴ洢λ�õĹ�ϵ
		gInfoOccupy[Name]=0;
		gIdleSite[Name]=1;
		InfoTempStatistics(Name);
		FindIdleSite(Name);//������λ��������
		Debug("%4d       %4d      %4d",gInfoOccupy[Name],gIdleSite[Name],gInfoAttrib[Name].ChkSizeof);
		
		Debug("\r[%s]\n\r",gNameInfoName[Name]);
	}

	Debug("\n\r");
}

//�����info��Ϣ��flash��������Դ洢λ��
//����INFO_RES_SPACE_FULL��ʾû�ռ�
INFO_SITE SaveInfoTemp(INFO_TEMP_NAME Name,void *pData,u16 Byte)
{
	INFO_ADDR FlashAddr;
	INFO_ID InfoID;

	if(Name>=ITN_MAX) return INFO_PARAM_ERROR;
	if(pData==NULL) return INFO_PARAM_ERROR;
	if(*(u32 *)pData==INFO_NULL) return INFO_PARAM_ERROR;
	
	if(Byte > gInfoAttrib[Name].BlockBytes)
	{
		Debug("Info Size Is Too Big!\n\r");
		while(1);
	}
	
	FlashAddr=GetAddr(gIdleSite[Name]);
	if(FlashAddr<gInfoEndAddr[Name]) 
	{
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		if(InfoID==INFO_NULL)
		{
			SpiFlsWriteData(FlashAddr,Byte,pData);
			if(INFO_SPACE_FULL == FindIdleSite(Name)) return INFO_SPACE_FULL; //���¿�λ������
			return GetSite(FlashAddr);
		}
		else
		{
			Debug("FindIdleSite is not null!S %d %d\n\r",gIdleSite[Name],FlashAddr);
			while(1);
		}
	}
	else
	{
		//Debug("Info Space Full!Rebuild it!S\n\r");
		return INFO_SPACE_FULL;
	}

//	return INFO_SPACE_FULL;
}

//����INFO_RES_ERROR��ʾ������
//������Чֵ��ʾʵ�ʴ洢��ַ
//Site������Ч���֣��᷵�ش�����
INFO_SITE ReadInfoBySite(INFO_TEMP_NAME Name,INFO_SITE Site,void *pData,u16 Byte)
{
	INFO_ADDR FlashAddr=GetAddr(Site);
	
	if(Name>=ITN_MAX) return INFO_PARAM_ERROR;
	if(pData==NULL) return INFO_PARAM_ERROR;
	if(Site==0) return INFO_PARAM_ERROR;
	if(Site > gInfoOccupy[Name]) return INFO_SPACE_FULL;
	
	if((FlashAddr < gInfoStartAddr[Name])||(FlashAddr >= gInfoEndAddr[Name])||(FlashAddr%gInfoAttrib[Name].BlockBytes)) return INFO_PARAM_ERROR;
	
	SpiFlsReadData(FlashAddr,Byte,pData);
	return GetSite(FlashAddr);
}

//��ָ����������appid
//�������ǰ����
INFO_SITE ReadInfoTemp(INFO_TEMP_NAME Name,INFO_ID AppID,void *pData,u16 Byte)
{
	INFO_ADDR FlashAddr=gInfoStartAddr[Name]+(gInfoOccupy[Name]-1)*gInfoAttrib[Name].BlockBytes;
	INFO_ADDR EndAddr=gInfoStartAddr[Name];
	INFO_ID InfoID;

	if(Name>=ITN_MAX) return INFO_PARAM_ERROR;
	if(AppID==0) return INFO_PARAM_ERROR;
	
	for(;FlashAddr>=EndAddr;FlashAddr-=gInfoAttrib[Name].BlockBytes)
	{
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		if(InfoID==AppID)
		{
			if(pData!=NULL)SpiFlsReadData(FlashAddr,Byte,pData);
			return GetSite(FlashAddr);
		}
	}

	return INFO_RES_ERROR;
}

u16 InfoTempTotalInc(INFO_TEMP_NAME Name)
{
	if(Name>=ITN_MAX) return 0;
	
	gInfoOccupy[Name]++;
	return gInfoOccupy[Name];
}

//����info temp
//ɾ��ǰ�ķ�֮һ�����ݣ�ת�����´洢��
void FlushTempInfo(INFO_TEMP_NAME Name)
{
	u16 TotalNum=gInfoAttrib[Name].SectorNum*FLASH_SECTOR_BYTES/gInfoAttrib[Name].BlockBytes;
	INFO_ADDR BackupAddr;
	INFO_ADDR FlashAddr;
	u16 NullNum=0;
	INFO_ID InfoID;
	INFO_ID *pInfoID;

	if(Name>=ITN_MAX) return;

	//������û�������Ҫ
	for(FlashAddr=gInfoStartAddr[Name];FlashAddr<gInfoEndAddr[Name];FlashAddr+=gInfoAttrib[Name].BlockBytes,BackupAddr+=gInfoAttrib[Name].BlockBytes)
	{
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		if(InfoID==INFO_NULL) NullNum++;
	}

	//����հ�λ�ò���8��֮һ,����
	if(NullNum<(TotalNum/8))
	{
		Debug("Need Flush!Null %d,Total %d\n\r",NullNum,TotalNum);
	}
	else
	{
		Debug("Not Need Flush!Null %d,Total %d\n\r",NullNum,TotalNum);
		return;//��������
	}	
	
	//�������������
	if(gInfoStartAddr[Name] == gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES)
		BackupAddr=gInfoAttrib[Name].BackupSector*FLASH_SECTOR_BYTES;
	else
		BackupAddr=gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES;

	//��ʼ����
	pInfoID=Q_Malloc(gInfoAttrib[Name].BlockBytes);
	for(FlashAddr=gInfoStartAddr[Name]+((gInfoAttrib[Name].SectorNum*FLASH_SECTOR_BYTES)>>2);//���ķ�֮һ����ʼ
			FlashAddr<gInfoEndAddr[Name];
			FlashAddr+=gInfoAttrib[Name].BlockBytes)
	{
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)pInfoID);
		if(*pInfoID!=0 && *pInfoID!=INFO_NULL)//��Ч��ֱ�ӿ���
		{
			//Debug("Cp %d >> %d [%d]\n\r",GetSite(FlashAddr),GetSite(BackupAddr),gInfoAttrib[Name].BlockBytes);
			SpiFlsReadData(FlashAddr,gInfoAttrib[Name].BlockBytes,(void *)pInfoID);
			SpiFlsWriteData(BackupAddr,gInfoAttrib[Name].BlockBytes,(void *)pInfoID);
			BackupAddr+=gInfoAttrib[Name].BlockBytes;
		}
	}

	//����ԭ�ȴ洢
	if(gInfoStartAddr[Name] == gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES)
	{
		SpiFlsEraseSector(gInfoStartAddr[Name]/FLASH_SECTOR_BYTES,gInfoAttrib[Name].SectorNum);
		gInfoStartAddr[Name] = gInfoAttrib[Name].BackupSector*FLASH_SECTOR_BYTES;
		gInfoEndAddr[Name]=(gInfoAttrib[Name].BackupSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;
	}
	else
	{
		SpiFlsEraseSector(gInfoStartAddr[Name]/FLASH_SECTOR_BYTES,gInfoAttrib[Name].SectorNum);
		gInfoStartAddr[Name]=gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES;
		gInfoEndAddr[Name]=(gInfoAttrib[Name].StartSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;		
	}
	
	//����ͳ��
	gInfoOccupy[Name]=0;
    gIdleSite[Name]=1;
	InfoTempStatistics(Name);
	FindIdleSite(Name);
	
	Q_Free(pInfoID);	
}

//�������temp
void CleanTempInfo(INFO_TEMP_NAME Name)
{
	//u16 TotalNum=gInfoAttrib[Name].SectorNum*FLASH_SECTOR_BYTES/gInfoAttrib[Name].BlockBytes;
	//INFO_ADDR BackupAddr;
	//INFO_ADDR FlashAddr;
	//u16 NullNum=0;
	//INFO_ID InfoID;
	//INFO_ID *pInfoID;

	if(Name>=ITN_MAX) return;
	if(gInfoOccupy[Name]==0) return;

	//����ԭ�ȴ洢
	if(gInfoStartAddr[Name] == gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES)
	{
		SpiFlsEraseSector(gInfoStartAddr[Name]/FLASH_SECTOR_BYTES,gInfoAttrib[Name].SectorNum);
		gInfoStartAddr[Name] = gInfoAttrib[Name].BackupSector*FLASH_SECTOR_BYTES;
		gInfoEndAddr[Name]=(gInfoAttrib[Name].BackupSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;
	}
	else
	{
		SpiFlsEraseSector(gInfoStartAddr[Name]/FLASH_SECTOR_BYTES,gInfoAttrib[Name].SectorNum);
		gInfoStartAddr[Name]=gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES;
		gInfoEndAddr[Name]=(gInfoAttrib[Name].StartSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;		
	}
	
	//����ͳ��
	gInfoOccupy[Name]=0;
    gIdleSite[Name]=1;
}

//����һ������λ��
//���¿���λ��������
//����INFO_RES_SPACE_FULL��ʾû�ռ�
INFO_SITE FindIdleSite(INFO_TEMP_NAME Name)
{
	INFO_ADDR FlashAddr;
	INFO_ID InfoID;

	if(Name>=ITN_MAX) return INFO_PARAM_ERROR;
	
	for(FlashAddr=GetAddr(gIdleSite[Name]);FlashAddr<gInfoEndAddr[Name];FlashAddr+=gInfoAttrib[Name].BlockBytes)
	{
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		if(InfoID==INFO_NULL)
		{
			gIdleSite[Name]=GetSite(FlashAddr);
			return gIdleSite[Name];
		}
	}

	//Debug("Info Space Full!Rebuild it!F\n\r");
	gIdleSite[Name]=0x7fff;//����7fff��ʾ����
	
	return INFO_SPACE_FULL;
}

//���ص�λ���С
//ע���ǿ��С��һ����256���������ӻ���������
u16 GetInfoBlockSize(INFO_TEMP_NAME Name)
{
	if(Name>=ITN_MAX) return 0;
	return gInfoAttrib[Name].BlockBytes;
}

//������Ϣ��С,��ʵ��ʹ�ô�С
u16 GetInfoSize(INFO_TEMP_NAME Name)
{
	if(Name>=ITN_MAX) return 0;
	return gInfoAttrib[Name].ChkSizeof;
}

//�����û��趨�������������
u16 GetInfoMaxAllowNum(INFO_TEMP_NAME Name)
{
	if(Name>=ITN_MAX) return 0;
	return  gInfoAttrib[Name].MaxItem;
}

//����ͳ�Ƶ�����
u16 GetInfoTotal(INFO_TEMP_NAME Name)
{
	if(Name>=ITN_MAX) return 0;
	return gInfoOccupy[Name];
}
