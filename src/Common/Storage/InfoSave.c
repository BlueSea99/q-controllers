//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl��һ�׻����¼��Ŀ�������ܣ������MCV��ܣ�Q-Ctrl����Э��
�洢��Data�������������IO������������Controller�����ߵ��߼�����
���DIC��ܡ�
Q-Ctrl����stm32�д��������������ֱ�ӵ��ã�Ҳ������ֲ��������Ƭ��ƽ̨��
�������ϵͳ��֧�֣������ؿ�������̹��������£��ɴ���������Ҫ����ϵͳ
���ܴ���ĸ���ҵ��
By Karlno ����Ƽ�

���ļ�������һ�׻���spi flash�����ݴ洢���ƣ��ɱ�����������������Ŀ�����ٴ��뿪����
���洢������Ҫ������¼��ͬ�������ݣ��ǳ������ݿ��е����ݱ�
*/
//------------------------------------------------------------------//
#include "SysDefines.h"
#include "SpiFlashApi.h"



//info�洢�鲻Ӧ��ʵ��flash��0��ַ��ʼ��������Ϊ�˱��Ȿҳ�����е�FlashAddr����Ϊ0

//Addr:ָ��Ϣ�洢��ľ��Ե�ַ
//Site:ָ��Ϣ�洢���λ�ã���1��ʼ���洢�ĵ�1����Ϣ���ַΪ1���洢�ĵ�2����Ϣ��Ϊ2
//ID:ָ��Ϣ�ڲ���Ӧ�ó���ָ����id�������0
//Idx:ָ��Ϣ�洢����Ч��Ϣ��˳����������1��ʼ�������1,3����Ϣ����Ч����2����Ϣ��Ϊ�����飬��ô��������Ϣ��IdxΪ2��������3

typedef enum{
	IBF_Removed=0xe0,//�˿����ã���ϢҲ�Ѿ���ɾ��
	IBF_Vaild=0xf0,//�˿���Ϣ��Ч
	IBF_Null=0xfe,//�˿�հ�δʹ��
}INFO_BLOCK_FLAG;

typedef struct{
	INFO_BLOCK_FLAG Flag;///������ʾ����Ϣ����ڻ��Ǳ�ɾ��
	INFO_TYPE Type;//����
	u16 Resv;
}INFO_ITEM_HEADER;

typedef struct{
	INFO_BLOCK_FLAG Flag;///������ʾ����Ϣ����ڻ��Ǳ�ɾ��
	INFO_TYPE Type;//����
	u16 Resv;
	INFO_ID AppID;//���û����壬��info�ṹ�޹أ�����ŵ��û����ݵ���ǰ��
}INFO_HEADER_AND_ID;

typedef struct{
	u32 StartAddr;//��ʼ�洢��ַ
	u32 StartSec;//��ʼ����
	u32 SectorNum;//ռ��������
	u32 UnitSize;//��Ԫ�ֽ�������4��ͷ�ֽ�
	u32 UnitTotal;//��Ԫ����
}INFO_BLOCK_ATTRIB;//�洢����Ϣ

typedef struct{
	INFO_TYPE Type;
	INFO_BLOCK Block;
	u16 ItemBytes;//����С�ڵ���INFO_BLOCK_ATTRIB.ItemBytes-4
}INFO_TYPE_ATTRIB;

//�洢����ÿ���洢����������1024���ɹ��洢����
//ÿ������64kB����256ҳ
static const INFO_BLOCK_ATTRIB gBlockAttrib[IBN_MAX]={ //ÿ���洢����Ϣ
//StartAddr			StartSec		SectorNum			UnitSize		UnitTotal
{(FM_INFOSAVE_BASE_SECTOR)*FLASH_SECTOR_BYTES,		FM_INFOSAVE_BASE_SECTOR,		1,	FLASH_PAGE_SIZE/8	,	2048},//IBN_32B
{(FM_INFOSAVE_BASE_SECTOR+1)*FLASH_SECTOR_BYTES,		FM_INFOSAVE_BASE_SECTOR+1,	2,	FLASH_PAGE_SIZE/4	,	2048},//IBN_64B
{(FM_INFOSAVE_BASE_SECTOR+3)*FLASH_SECTOR_BYTES,		FM_INFOSAVE_BASE_SECTOR+3,	4,	FLASH_PAGE_SIZE/2	,	2048},//IBN_128B
{(FM_INFOSAVE_BASE_SECTOR+7)*FLASH_SECTOR_BYTES,		FM_INFOSAVE_BASE_SECTOR+7,	8,	FLASH_PAGE_SIZE,		2048},//IBN_256B
{(FM_INFOSAVE_BASE_SECTOR+15)*FLASH_SECTOR_BYTES,	FM_INFOSAVE_BASE_SECTOR+15,	16,	FLASH_PAGE_SIZE*2	,	2048},//IBN_512B
};

static const INFO_TYPE_ATTRIB gTypeAttrib[IFT_MAX]={ //ÿ�����͵���Ϣ
//Type		Block		ItemBytes
{IFT_STR,				IBN_64B,		sizeof(STR_RECORD)},
{IFT_VARIABLE,	IBN_128B,	sizeof(VARIABLE_RECORD)},
{IFT_RF_DATA,		IBN_256B,	sizeof(RF_RECORD)},
{IFT_IR_DATA,		IBN_256B,	sizeof(IR_RECORD)},
{IFT_KEYS,			IBN_32B,		sizeof(KEYS_RECORD)},
{IFT_DEV,				IBN_128B,	sizeof(DEVICE_RECORD)},
{IFT_TRIGGER,		IBN_128B,	sizeof(TRIGGER_RECORD)},
{IFT_SCENE,			IBN_512B,	sizeof(SCENE_RECORD)},
};


//Ѱַ�������ÿ��item�����ʹ�Ŵ˴���0��ʾ��ɾ����0xff��ʾδ��
static INFO_TYPE gpTypeMapB32[2048];//�����С���ӦgBlockAttrib�޸�
static INFO_TYPE gpTypeMapB64[2048];//�����С���ӦgBlockAttrib�޸�
static INFO_TYPE gpTypeMapB128[2048];//�����С���ӦgBlockAttrib�޸�
static INFO_TYPE gpTypeMapB256[2048];//�����С���ӦgBlockAttrib�޸�
static INFO_TYPE gpTypeMapB512[2048];//�����С���ӦgBlockAttrib�޸�
static INFO_TYPE *gpTypeMap[IBN_MAX]={gpTypeMapB32,gpTypeMapB64,gpTypeMapB128,gpTypeMapB256,gpTypeMapB512};

#if 1 //��������أ�����ŵ�map����֮��ʹ��
//���ݶ����Ĵ洢���ݣ�����map
static void BuildTypeMap(INFO_BLOCK Block)
{
	INFO_TYPE *pMap=gpTypeMap[Block];
	INFO_ITEM_HEADER Header;
	u32 Unit;

	MemSet(pMap,IFT_IDLE,gBlockAttrib[Block].UnitTotal);

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//������ȡ��Ϣͷ
	{
		SpiFlsReadData(gBlockAttrib[Block].StartAddr+Unit*gBlockAttrib[Block].UnitSize,sizeof(Header),(void *)&Header);

		if(Header.Flag==IBF_Null)
		{
			pMap[Unit]=IFT_IDLE;
		}		
		else if(Header.Flag==IBF_Vaild)
		{
			pMap[Unit]=Header.Type;
		}
		else if(Header.Flag==IBF_Removed)
		{
			pMap[Unit]=IFT_NOVALID;
		}
		else
		{
			Debug("Format Block Is Error!%u\n\r",Block);
			while(1);
		}
	}
}

//�޸��������
static void SetItemMapFlag(INFO_BLOCK Block,INFO_SITE Site,INFO_TYPE Type)
{
	gpTypeMap[Block][Site]=Type;
}

//�õ���ʵ��ַ
static INFO_ADDR SiteToAddr(INFO_BLOCK Block,INFO_SITE Site)
{
	return gBlockAttrib[Block].StartAddr+Site*gBlockAttrib[Block].UnitSize;
}

//������������λ��
static INFO_SITE FindItemByIdx(INFO_BLOCK Block,INFO_TYPE Type,INFO_IDX Idx)
{
	INFO_TYPE *pMap=gpTypeMap[Block];
	u16 Site;
	u16 Cnt=0;
	
	for(Site=0;Site<gBlockAttrib[Block].UnitTotal;Site++)
	{
		if(pMap[Site]==Type)
		{
			if(++Cnt==Idx)
			{
				return Site;
			}
		}
	}

	return INFO_NOT_FOUND;
}

//����app id��Ѱ��Ŀ��Ҫ��ȡflash�����ԱȽϷ�ʱ��
static INFO_SITE FindItemByAppID(INFO_BLOCK Block,INFO_TYPE Type,INFO_ID AppID)
{
	INFO_TYPE *pMap=gpTypeMap[Block];
	INFO_HEADER_AND_ID Header;
	u16 Site;
	
	for(Site=0;Site<gBlockAttrib[Block].UnitTotal;Site++)
	{
		if(pMap[Site]==Type)//�ҵ�������
		{
			u32 Addr=SiteToAddr(Block,Site);

			SpiFlsReadData(Addr,sizeof(Header),(void *)&Header);
			if(Header.AppID==AppID)
			{
				return Site;
			}
		}
	}

	return INFO_NOT_FOUND;
}
#endif


#if 1 //�ڲ�����
//ͳ�Ƹ�������info����
static void StatsInfoItemNum(INFO_BLOCK Block,u16 *pIdleNum,u16 *pValidNum,u16 *pRemovedNum)
{
	INFO_ITEM_HEADER Header;
	u32 Unit;

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//������ȡ��Ϣͷ
	{
		SpiFlsReadData(gBlockAttrib[Block].StartAddr+Unit*gBlockAttrib[Block].UnitSize,sizeof(Header),(void *)&Header);

		if(Header.Flag==IBF_Null)
		{
			(*pIdleNum)++;
		}		
		else if(Header.Flag==IBF_Vaild)
		{
			(*pValidNum)++;
		}
		else if(Header.Flag==IBF_Removed)
		{
			(*pRemovedNum)++;
		}
		else
		{
			Debug("Format Block Is Error!%u\n\r",Block);
			while(1);
		}
	}

	//Debug("Block[%u] Idle:%4u, Vaild:%4u, Removed:%4u @ Sector %u:%u[%u/%u]\n\r",Block,*pIdleNum,*pValidNum,*pRemovedNum,MainAddr/FLASH_SECTOR_BYTES,gBlockAttrib[Block].SectorNum,(FM_INFOSAVE_BASE_SECTOR+GetStartSecOffset(Block,FALSE)),(FM_INFOSAVE_BASE_SECTOR+GetStartSecOffset(Block,TRUE)));
}

//���洢��
//����洢��δ����ʽ��������FALSE
//����洢���ѱ���ʽ��������TRUE
static bool CheckBlockFat(INFO_BLOCK Block)
{
	INFO_ITEM_HEADER Header;
	u32 Unit;

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//������ȡ��Ϣͷ
	{
		SpiFlsReadData(gBlockAttrib[Block].StartAddr+Unit*gBlockAttrib[Block].UnitSize,sizeof(Header),(void *)&Header);

		if(Header.Flag!=IBF_Removed && Header.Flag!=IBF_Vaild && Header.Flag!=IBF_Null)
		{
			return FALSE;
		}		
	}

	return TRUE;
}

//���洢���Ƿ�ȫΪ0xff
static bool CheckBlockIdle(INFO_BLOCK Block)
{
	u32 j,Addr,EndAddr=(gBlockAttrib[Block].StartSec+gBlockAttrib[Block].SectorNum)*FLASH_SECTOR_BYTES;
	u32 *pBuf=Q_Malloc(FLASH_PAGE_SIZE);

	for(Addr=gBlockAttrib[Block].StartAddr;Addr<EndAddr;Addr+=FLASH_PAGE_SIZE)//��ҳ��ȡ
	{
		SpiFlsReadData(Addr,FLASH_PAGE_SIZE,(void *)pBuf);

		for(j=0;j<(FLASH_PAGE_SIZE>>2);j++)
		{
			if(pBuf[j]!=0xffffffff)
			{
				Q_Free(pBuf);
				return FALSE;
			}		
		}
	}

	Q_Free(pBuf);
	return TRUE;
}

//��������ʽ��ĳ��
static void FromatBlock(INFO_BLOCK Block)
{
	if(CheckBlockIdle(Block)!=TRUE)//��ȫ��
	{
		u32 Now;
		Debug("FromatBlock[%u] EraseSec:%u-%u,",Block,gBlockAttrib[Block].StartSec,gBlockAttrib[Block].SectorNum);
		Now=GetNowMs();
		SpiFlsEraseSector(gBlockAttrib[Block].StartSec,gBlockAttrib[Block].SectorNum);
		Debug("Finish %dmS\n\r",GetNowMs()-Now);
	}

	//��ʼ��ʽ��
	{
		INFO_BLOCK_FLAG Flag=IBF_Null;
		u32 Unit;

		for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//����������Ϣͷ
		{
			SpiFlsWriteData(gBlockAttrib[Block].StartAddr+Unit*gBlockAttrib[Block].UnitSize,sizeof(INFO_BLOCK_FLAG),&Flag);
		}
	}

	//���map
	MemSet(gpTypeMap[Block],IFT_IDLE,gBlockAttrib[Block].UnitTotal);
}

//������϶�����´洢
//����flash����ֻ����4k���ص�
//�˹��̲��ܶϵ�
//��ɺ�Ὠ��map
static void FlushBlock(INFO_BLOCK Block)
{
	u32 EndAddr=(gBlockAttrib[Block].StartSec+gBlockAttrib[Block].SectorNum)*FLASH_SECTOR_BYTES;
	u32 UnitSize=gBlockAttrib[Block].UnitSize;
	u32 UintNum=(FLASH_MIN_SEC_BYTES/gBlockAttrib[Block].UnitSize);
	u32 DstAddr=gBlockAttrib[Block].StartAddr;
	u8 *pSecBuf=Q_Malloc(FLASH_MIN_SEC_BYTES);
	INFO_TYPE *pMap=gpTypeMap[Block];
	INFO_ITEM_HEADER *pBuf;
	u32 MapIdx=0,Unit,Addr;

	MemSet(pMap,IFT_IDLE,gBlockAttrib[Block].UnitTotal);//���map
			
	for(Addr=gBlockAttrib[Block].StartAddr;Addr<EndAddr;Addr+=FLASH_MIN_SEC_BYTES)
	{
		SpiFlsReadData(Addr,FLASH_MIN_SEC_BYTES,(void *)pSecBuf);//ÿ4kΪһ����λ��ȫ��������buf��
		SpiFlsEraseMinSec(Addr);//����4k����

		for(Unit=0,pBuf=(void *)pSecBuf;Unit<UintNum;Unit++,pBuf=(void *)(UnitSize+(u32)pBuf))//���unit��飬������Чunit��������
		{
			if(pBuf->Flag==IBF_Vaild)//��������Ч��
			{
				SpiFlsWriteData(DstAddr,UnitSize,(void *)pBuf);
				DstAddr+=UnitSize;
				pMap[MapIdx++]=pBuf->Type;//���map
			}		
		}		
	}

	//ʣ��Ŀռ�ȫ����ʽ��
	{
		INFO_BLOCK_FLAG Flag=IBF_Null;

		for(;DstAddr<EndAddr;DstAddr+=UnitSize)//����������Ϣͷ
		{
			SpiFlsWriteData(DstAddr,sizeof(INFO_BLOCK_FLAG),&Flag);
		}
	}	

	Q_Free(pSecBuf);
}

//Force�Ƿ�ǿ������
//���ռ�÷�����̫�࣬����������洢��
//���������������true
//��ɺ󣬻Ὠ��map
static bool TidyBlock(INFO_BLOCK Block)
{
	u16 IdleNum=0,ValidNum=0,RemovedNum=0;
	u16 Total=gBlockAttrib[Block].UnitTotal;

	//ͳ�Ƹ���
	StatsInfoItemNum(Block,&IdleNum,&ValidNum,&RemovedNum);

	if((IdleNum+ValidNum+RemovedNum)!=Total)
	{
		Debug("Block Flag Num Is Error!\n\r");
	}

	//����Ƿ���Ҫ����
	if(IdleNum<(Total>>2) && RemovedNum>(Total>>3))//�հ׵�Ԫ��ĿС�������ķ�֮һ������ɾ����Ԫ����������˷�֮һ��������
	{
		u32 Now;
		Debug("Need Tidy Block[%u], Idle:%4u, Valid:%4u, Remove:%4u ... ",Block,IdleNum,ValidNum,RemovedNum);
		Now=GetNowMs();
		FlushBlock(Block);
		Debug("Finish by %umS\n\r",GetNowMs()-Now);
		return TRUE;
	}
	else
	{
		Debug("No Need Tidy Block[%u], Idle:%4u, Valid:%4u, Remove:%4u\n\r",Block,IdleNum,ValidNum,RemovedNum);
		BuildTypeMap(Block);//����������������ڲ���
		return FALSE;
	}
}
#endif

//չʾ������Ϣ
void DebugInfoSave(INFO_BLOCK Block)
{
	u16 IdleNum=0,ValidNum=0,RemovedNum=0;
	INFO_TYPE *pMap;
	u16 Unit;

	if(Block>=IBN_MAX) return;

	OS_EnterCritical();
	
	pMap=gpTypeMap[Block];
	
	//չʾռ������ʣ�����ȵ�
	StatsInfoItemNum(Block,&IdleNum,&ValidNum,&RemovedNum);
	
	Debug("Block[%u] Idle:%4u, Vaild:%4u, Removed:%4u @ Sector %u:%u\n\r",Block,IdleNum,ValidNum,RemovedNum,gBlockAttrib[Block].StartSec,gBlockAttrib[Block].SectorNum);
	
	//չʾÿ������map
	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)
	{
		switch(pMap[Unit])
		{
			case IFT_IDLE:
				Debug("_");
				break;
			case IFT_NOVALID:
				Debug("*");
				break;
			default:
				Debug("%x",pMap[Unit]);
		}

		if(Unit%64==(64-1)) Debug("\n\r");
	}

	OS_ExitCritical();
}

//�����洢�������Ƿ�Ҫ��ʽ������ȫ��flagȫ����Ϊ0xfe
//���ÿ���洢�鲻���Ͼ͸�ʽ��
//���洢�������������Ҫʱ����洢���������洢��typeӳ���
//ForceClean==TRUE�ָ���������
//ɾ�������ʱ���Զ�����
//�κ�ʱ�򶼿��Ե��ô˺������������洢����С
void InfoBuildBlock(INFO_BLOCK Block,bool ForceClean)
{
	if(Block>=IBN_MAX) return;

	OS_EnterCritical();
	
	if(ForceClean)//ǿ�ƻָ���������
	{
		FromatBlock(Block);//��������ʽ��
	}
	else //�Լ�
	{
		if(CheckBlockFat(Block)==TRUE)//��������£�Ӧ���Ǹ�ʽ��״̬(��Ч��)
		{
			TidyBlock(Block);//����洢��
		}
		else
		{
			FromatBlock(Block);//��������ʽ��1��
		}	
	}

	OS_ExitCritical();
}

//��ʼ��info���ݿ�
//ForceClean==TRUE�ָ���������
//ɾ�������ʱ���Զ�����
//�κ�ʱ�򶼿��Ե��ô˺��������������д洢����С
void InfoSaveInit(bool ForceClean)
{
	INFO_BLOCK Block;
	INFO_TYPE Type;
	
#if 1	//������
	for(Block=(INFO_BLOCK)0;Block<IBN_MAX;Block++)
	{
		if(gBlockAttrib[Block].UnitTotal > gBlockAttrib[Block].SectorNum*FLASH_SECTOR_BYTES/gBlockAttrib[Block].UnitSize)
		{
			Debug("INFO_BLOCK %u UnitTotal is too big!\n\r",Block);
			while(1);
		}

		if(gBlockAttrib[Block].StartAddr!=gBlockAttrib[Block].StartSec*FLASH_SECTOR_BYTES)
		{
			Debug("INFO_BLOCK %u StartAddr or StartSec error!\n\r",Block);
			while(1);
		}

		if(Block!=(INFO_BLOCK)0)
		{
			if(gBlockAttrib[Block].StartSec!=gBlockAttrib[Block-1].StartSec+gBlockAttrib[Block-1].SectorNum)
			{
				Debug("INFO_BLOCK %u StartSec error!\n\r",Block);
				while(1);
			}
		}		
	}

	for(Type=(INFO_TYPE)0;Type<IFT_MAX;Type++)
	{
		if(gTypeAttrib[Type].Type!=Type)
		{
			Debug("INFO_TYPE %u Type is error!\n\r",Type);
			while(1);
		}
		
		if((gTypeAttrib[Type].ItemBytes+sizeof(INFO_ITEM_HEADER))>gBlockAttrib[gTypeAttrib[Type].Block].UnitSize)
		{
			Debug("INFO_TYPE %u Param is error! %u\n\r",Type,gTypeAttrib[Type].ItemBytes+sizeof(INFO_ITEM_HEADER));
			while(1);
		}

		if(gTypeAttrib[Type].ItemBytes % 4)
		{
			Debug("INFO_TYPE %u ItemBytes is error!\n\r",Type);
			while(1);
		}
	}		
#endif 

	for(Block=(INFO_BLOCK)0;Block<IBN_MAX;Block++)
	{
		InfoBuildBlock(Block,ForceClean);
	}

	for(Type=(INFO_TYPE)0;Type<IFT_MAX;Type++)
	{
		Debug("[%s]:%u\n\r",gNameInfoName[Type],GetTypeInfoTotal(Type));	
	}

	Debug("\n\r");
}

//�����info��Ϣ��flash�����ؾ��Դ洢λ��
//����INFO_RES_SPACE_FULL��ʾû�ռ�
INFO_ADDR SaveInfo(INFO_TYPE Type,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_ITEM_HEADER Header;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;

	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByIdx(Block,IFT_IDLE,1);
	if(Site<0)
	{
		OS_ExitCritical();
		return INFO_SPACE_FULL;	//��Դ����
	}
	
	SetItemMapFlag(Block,Site,Type);//�޸��������
	Addr=SiteToAddr(Block,Site);//�õ���ʵ��ַ

	Header.Flag=IBF_Vaild;
	Header.Type=Type;
	Header.Resv=0xffff;
	SpiFlsWriteData(Addr,sizeof(Header),(void *)&Header);
	if(pData!=NULL) SpiFlsWriteData(Addr+sizeof(Header),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return Addr;
}

//ɾ��flash��ָ��λ�õ�info��Ϣ
//����ԭ���ľ��Դ洢λ��
INFO_ADDR DeleteInfo(INFO_TYPE Type,INFO_ID AppID)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_ITEM_HEADER Header;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;
	if(AppID==0) return INFO_PARAM_ERROR;

	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByAppID(Block,Type,AppID);
	if(Site<0) 
	{
		OS_ExitCritical();
		return 0;
	}
	
	SetItemMapFlag(Block,Site,IFT_NOVALID);//�޸��������
	Addr=SiteToAddr(Block,Site);//�õ���ʵ��ַ

	Header.Flag=IBF_Removed;
	Header.Type=IFT_IDLE;
	Header.Resv=0xffff;
	SpiFlsWriteData(Addr,sizeof(Header),(void *)&Header);
	
	OS_ExitCritical();
	return Addr;
}

//������ͬappid��info��Ϣ��flash�����ؾ��Դ洢λ��
//�������ݶԱȺ�У��
//����flashֻ����1��0�����ԣ�������ݲ��ԣ����ܴ洢������ǳ���Ԥ��
INFO_ADDR CoverInfo(INFO_TYPE Type,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_ITEM_HEADER Header;
	INFO_ID AppID;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;
	
	if(pData==NULL) return INFO_PARAM_ERROR;
	else MemCpy(&AppID,pData,sizeof(INFO_ID));//����id

	if(AppID==0) return INFO_PARAM_ERROR;
		
	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByAppID(Block,Type,AppID);
	if(Site<0) 
	{
		OS_ExitCritical();
		return INFO_PARAM_ERROR;
	}	
	Addr=SiteToAddr(Block,Site);//�õ���ʵ��ַ

	SpiFlsWriteData(Addr+sizeof(Header),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return Addr;
}

//��ʵ�����ݵ�ͷ4�ֽ���Ϊappid����ѯ�洢�飬ƥ��ʱ������Ϣ
//���ض�ȡ��С
u16 ReadInfoByAppID(INFO_TYPE Type,INFO_ID AppID,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;

	if(Type>=IFT_MAX) return 0;
	if(AppID==0) return 0;
	
	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByAppID(Block,Type,AppID);
	if(Site<0)
	{
		OS_ExitCritical();
		return 0;
	}
	
	Addr=SiteToAddr(Block,Site);//�õ���ʵ��ַ

	if(pData!=NULL)
		SpiFlsReadData(Addr+sizeof(INFO_ITEM_HEADER),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return gTypeAttrib[Type].ItemBytes;
}

//��������˳���ȡinfo��Ϣ
//���ض�ȡ��С
u16 ReadInfoByIdx(INFO_TYPE Type,INFO_IDX Idx,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;

	if(Type>=IFT_MAX) return 0;

	Block=gTypeAttrib[Type].Block;

	if(Idx==0 || Idx>gBlockAttrib[Block].UnitTotal) return 0;

	OS_EnterCritical();
	Site=FindItemByIdx(Block,Type,Idx);
	if(Site<0)
	{
		OS_ExitCritical();
		return 0;
	}
	
	Addr=SiteToAddr(Block,Site);//�õ���ʵ��ַ

	if(pData!=NULL)
		SpiFlsReadData(Addr+sizeof(INFO_ITEM_HEADER),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return gTypeAttrib[Type].ItemBytes;
}

//��ȡ��Ϣ����
u16 GetTypeInfoTotal(INFO_TYPE Type)
{
	INFO_TYPE *pMap;
	INFO_BLOCK Block=gTypeAttrib[Type].Block;
	s32 i,Num=0;

	if(Type>=IFT_MAX) return 0;

	OS_EnterCritical();
	pMap=gpTypeMap[Block];
	
	for(i=0;i<gBlockAttrib[Block].UnitTotal;i++)
	{
		if(pMap[i]==IFT_IDLE) break;
		if(pMap[i]==Type) Num++;
	}	

	OS_ExitCritical();
	return Num;
}

//��ȡ��Ϣ��С
u16 GetTypeItemSize(INFO_TYPE Type)
{
	if(Type>=IFT_MAX) return 0;
	
	return gTypeAttrib[Type].ItemBytes;
}

//��ȡ�հ׵�Ԫ��
u16 GetBlockFreeUnit(INFO_BLOCK Block)
{
	INFO_TYPE *pMap;
	s32 i,Num=0;

	if(Block>=IBN_MAX) return 0;

	OS_EnterCritical();
	pMap=gpTypeMap[Block];

	for(i=gBlockAttrib[Block].UnitTotal-1;i>=0;i--,Num++)//��ƨ����ǰ���հ׸񼴿�
	{
		if(pMap[i]!=IFT_IDLE) break;
	}	

	OS_ExitCritical();
	return Num;
}

//��ȡָ�����͵Ŀ�����Ŀ
u16 GetTypeFreeUnit(INFO_TYPE Type)
{
	return GetBlockFreeUnit(gTypeAttrib[Type].Block);
}

//��ȡָ�����͵Ŀ��С
u16 GetTypeBlockSize(INFO_TYPE Type)
{
	return 1<<(5+gTypeAttrib[Type].Block);
}

