#include "SysDefines.h"
#include "SpiFlashApi.h"

#define DB_Debug Debug

//�ⲿ����
extern const SYS_DB_STRUCT gDefSysDb;
extern SYS_DB_STRUCT gSysDb;
void SysSvnInit(void);
u32 Sys_GetValue(u16 Item,u32 IntParam,void *Val);
bool Sys_SetValue(u16 Item,u32 IntParam,void *pParam,u8 ByteLen);
void Sys_Default(void);

typedef u32 (*GetValueCallBackFunc)(u16 Item,u32 IntParam,void *Val);
typedef bool (*SetValueCallBackFunc)(u16 Item,u32 IntParam,void *pParam,u8 ByteLen);
typedef void (*BurnDefaultCallBackFunc)(void);

#define DBF_REMOVED 0x55550000//����
#define DBF_USED 0x55555555//ʹ����
#define DBF_IDLE 0x5555ffff//����

typedef struct{	
	u32 Flag;//��ȡ��־�������û���Ԥ
	u32 Ver;//�汾�������û���Ԥ
	u32 ChkSum;//У��ͣ������û���Ԥ
	
	u8 Data[4];//�û���������
}DB_STRUCT;	//���ݿ�

typedef struct{
	bool ForceDef;//һ���ı��ֵ����������ָ�Ĭ��ֵ
	u8 StartSector;//��ʼ������
	u8 SectorNum;//ռ����������
	u8 UnitPageNum;//ÿ��Ԫռ��ҳ�����
	const DB_STRUCT *pDefaultData;//Ĭ��ֵ�洢�ռ�
	DB_STRUCT *pData;//��ǰֵ�洢�ռ�
	u32 DataBytes;//�洢�ṹ���С����ver check��ǰ�岿��
	GetValueCallBackFunc GetValue;
	SetValueCallBackFunc SetValue;
	BurnDefaultCallBackFunc BurnDefault;
}SUB_DB_STRUCT;

const SUB_DB_STRUCT gSubDbs[SDN_MAX]={
{FALSE,0,1,1,(void *)&gDefSysDb,(void *)&gSysDb,sizeof(SYS_DB_STRUCT),Sys_GetValue,Sys_SetValue,Sys_Default},
};

#define GetStartAddr(n) (gSubDbs[n].StartSector*FLASH_SECTOR_BYTES)

//DB �汾
#if PRODUCT_IS_JUMPER
#define QDB_VER 2
#endif
#define GetDbVer(n) (QDB_VER+gSubDbs[n].ForceDef+gSubDbs[n].StartSector+gSubDbs[n].SectorNum+gSubDbs[n].UnitPageNum  \
									+gSubDbs[n].DataBytes+MakeHash33((void *)gSubDbs[n].pDefaultData,gSubDbs[n].DataBytes))

static u32 gNowDbAddr[SDN_MAX]={0};//��ǰdb��Чλ��

#if 0 //for debug
static void DB_BufDisp(void)
{
	u8 buf[FLASH_PAGE_SIZE];
	int i,j;

	DB_Debug("Spi Flash:\n\r");
	for(j=0;j<16;j++)
	{
		Q_SpiFlashSync(FlashRead,GetStartAddr(Name)+j*256,sizeof(buf),buf);
		DB_Debug("page %d context:",j);
		for(i=0;i<FLASH_PAGE_SIZE;i++)
		{
			if(buf[i]!=0xff)
			{
				DB_Debug("0x%02x ",buf[i]);
			}
		}
		DB_Debug("\n\r");
	}
}
#endif

//дĬ��ֵ��flash��ϵͳ
void QDB_BurnDefaultToSpiFlash(SUB_DB_NAME Name)
{
	DB_Debug("Now burn default database to spi flash!\n\r");
	SpiFlsEraseSector(gSubDbs[Name].StartSector,gSubDbs[Name].SectorNum);//��������db�������

	//��Ĭ��ֵд��ϵͳ��flash
	MemCpy((void *)gSubDbs[Name].pData,(void *)gSubDbs[Name].pDefaultData,gSubDbs[Name].DataBytes);//��Ĭ�����ݿ�����ϵͳ
	if(gSubDbs[Name].BurnDefault != NULL) gSubDbs[Name].BurnDefault();
	gSubDbs[Name].pData->Flag=DBF_USED;
	gSubDbs[Name].pData->Ver=GetDbVer(Name);
	gSubDbs[Name].pData->ChkSum=0;//�ȸ�0ֵ������Ӱ��У���
	gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
	DB_Debug("Database chksum is %x %x-%x\n\r",gSubDbs[Name].pData->ChkSum,gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
	SpiFlsWriteData(GetStartAddr(Name),gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);//��Ĭ�����ݿ�����flash db����

	gNowDbAddr[Name]=GetStartAddr(Name);
}

//��������д��flash
void QDB_BurnToSpiFlash(SUB_DB_NAME Name)
{
	u32 NewChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);//��¼��У���
	if(gSubDbs[Name].pData->ChkSum==NewChkSum) return;//У���һ�������账��		
		
	if(gNowDbAddr[Name] == GetStartAddr(Name)+(gSubDbs[Name].SectorNum*FLASH_SECTOR_PAGE_NUM-gSubDbs[Name].UnitPageNum)*FLASH_PAGE_SIZE)//���һҳ��������������
	{
		SpiFlsEraseSector(gSubDbs[Name].StartSector,gSubDbs[Name].SectorNum);//��������db�������
		gSubDbs[Name].pData->Flag=DBF_USED;
		gSubDbs[Name].pData->Ver=GetDbVer(Name);
		gSubDbs[Name].pData->ChkSum=0;//�ȸ�0ֵ������Ӱ��У���
		gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);		
		DB_Debug("Database Chk:%x,P:%x,Len:%u\n\r",gSubDbs[Name].pData->ChkSum,gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
		SpiFlsWriteData(GetStartAddr(Name),gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);//�����ݿ�����flash db����
		gNowDbAddr[Name]=GetStartAddr(Name);
		Debug("Burn Addr:%d\n\r",gNowDbAddr[Name]);
	}
	else //�����һҳ��ֱ��д��
	{
		u32 DbFlag=DBF_REMOVED;
		SpiFlsWriteData(gNowDbAddr[Name],sizeof(u32),(void *)&DbFlag);//������־
		Debug("Set Remove Flag At Page:%d\n\r",gNowDbAddr[Name]/FLASH_PAGE_SIZE);
		
		gNowDbAddr[Name]+=(gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE);
		gSubDbs[Name].pData->Flag=DBF_USED;
		gSubDbs[Name].pData->Ver=GetDbVer(Name);
		gSubDbs[Name].pData->ChkSum=0;//�ȸ�0ֵ������Ӱ��У���
		gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
		DB_Debug("Database chksum is %x %x-%x\n\r",gSubDbs[Name].pData->ChkSum,gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
		SpiFlsWriteData(gNowDbAddr[Name],gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);//�����ݿ�����flash db����
		Debug("Burn At Page:%d\n\r",gNowDbAddr[Name]/FLASH_PAGE_SIZE);
	}
}

//�Ӵ洢��������ݿ⵽�ڴ棬ͨ��ָ�뷵�ص�ǰ�洢ҳ
static void QDB_ReadFromSpiFlash(SUB_DB_NAME Name)
{
	u32 i;
	u32 DbFlag;
	u32 ChkSum;

	for(i=0;i<(gSubDbs[Name].SectorNum*FLASH_SECTOR_PAGE_NUM/gSubDbs[Name].UnitPageNum);i++) //�����洢��
	{
		SpiFlsReadData(GetStartAddr(Name)+i*gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE,sizeof(u32),(void *)&DbFlag);//����־λ
		
		if(DbFlag == DBF_USED)//������ȷ����
		{
			Debug("Read DB Idx:%d,Addr:%d\n\r",i,GetStartAddr(Name)+i*gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE);
			SpiFlsReadData(GetStartAddr(Name)+i*gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE,gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);
			gNowDbAddr[Name]=GetStartAddr(Name)+i*gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE;
			if(gSubDbs[Name].pData->Ver!=GetDbVer(Name))//���汾�Ƿ���ȷ
			{
				DB_Debug("Database version 0x%x is not right(!=0x%x)!\n\rBurn default database to flash\n\r",gSubDbs[Name].pData->Ver,GetDbVer(Name));
				for(Name=(SUB_DB_NAME)0;Name<SDN_MAX;Name++) QDB_BurnDefaultToSpiFlash(Name);
				return;
			}

			ChkSum=gSubDbs[Name].pData->ChkSum;
			gSubDbs[Name].pData->ChkSum=0;//�ȸ�0ֵ������Ӱ��У���
			if(ChkSum!= MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes))//���У����Ƿ���ȷ
			{
				DB_Debug("Database chksum is not right!(%x != %x) %x-%x\n\rsys halt!\n\r",ChkSum, MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes),gSubDbs[Name].pData->Data,gSubDbs[Name].DataBytes);
				QDB_BurnDefaultToSpiFlash(Name);
			}

			return;
		}
	}

	DB_Debug("Not read database for DB[%d]!\n\rBurn default database!\n\r",Name);
	QDB_BurnDefaultToSpiFlash(Name);
}

//��ʼ������ ��������
// 1.��flash��ȡ���ݿ�����
// 2.���û�ж������ݿ⣬��Ĭ��ֵ���ݿ���д��flash
// 3.��������ݿ⣬���ȡ���ݿ⵽����
void QDB_Init(void)
{	
	SUB_DB_NAME Name;
	
	Debug("Database init\n\r");

	for(Name=(SUB_DB_NAME)0;Name<SDN_MAX;Name++)
	{
		if((gSubDbs[Name].DataBytes) > gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE)
		{
			Debug("!!!DB[%d] init error!\n\r***Flash size[%d] for database[%d] is too small!\n\r",Name,gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE,gSubDbs[Name].DataBytes);
			while(1);
		}
		
		QDB_ReadFromSpiFlash(Name);
		
		Debug("DB[%d] Burn Sector:%d\n\r",Name,gSubDbs[Name].StartSector);
		Debug("DB[%d] Version:%x-%x\n\r",Name,gSubDbs[Name].pData->Ver,GetDbVer(Name));
		Debug("DB[%d] ChkSum:%x\n\r",Name,gSubDbs[Name].pData->ChkSum);
		Debug("DB[%d] Size:%d Byte < UnitPageNum x FLASH_PAGE_SIZE (=%d Byte)\n\r\n\r",Name,gSubDbs[Name].DataBytes,gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE);
	}

	SysSvnInit();	
}

//�����ݿ⻺���ȡֵ��ֻ��ʹ��pDB_Setting��ȡ
u32 QDB_GetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *Val)
{
	if(gSubDbs[Name].GetValue != NULL)	
		return gSubDbs[Name].GetValue(Item,IntParam,Val);

	return 0;
}

//дֵ�����ݿ⻺�棬ֻ��ʹ��pDB_Setting�洢
bool QDB_SetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *pParam,u8 ByteLen)
{
	if(gSubDbs[Name].SetValue != NULL)
		return gSubDbs[Name].SetValue(Item,IntParam,pParam,ByteLen);	

	return FALSE;
}


