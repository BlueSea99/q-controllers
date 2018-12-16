#include "SysDefines.h"
#include "Product.h"

typedef struct{//�������ݽṹ
	u16 Ver;//�޸ĺ�ᵼ�»ָ�Ĭ��
	u32 Num;//ʾ��


	
	u32 ChkSum;
}RFS_BLOCK;//�洢�飬���1k

static const RFS_BLOCK gDefBlock={//Ĭ������
1,//Ver
0,//Num



};

static volatile RFS_BLOCK gSysBlock;//���û���

void RFS_Debug(void)
{
	Debug("  -------------------------------------------------------------------\n\r");
	Frame();Debug("  |SnHash:%u\n\r",GetHwID(NULL));
#if ADMIN_DEBUG
	Frame();Debug("  |SoftVer:%u.%u(*)\n\r",__gBinSoftVer,RELEASE_DAY);
#else
	Frame();Debug("  |SoftVer:%u.%u\n\r",__gBinSoftVer,RELEASE_DAY);
#endif
	Frame();Debug("  |Num:%u\n\r",gSysBlock.Num);

	Debug("  -------------------------------------------------------------------\n\r");
}

//���ݴ洢��ʼ��
void RFS_Init(void)
{
	if(sizeof(RFS_BLOCK)>ROM_FLASH_PAGE_SIZE)
	{
		Debug("InfoSaveBlock %u too big!\n\r",sizeof(RFS_BLOCK));
		while(1);
	}

	//��������
	Rom_ReadPage((void *)&gSysBlock,IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE,sizeof(gSysBlock));

	if(MakeHash33((void *)&gSysBlock,sizeof(RFS_BLOCK)-4)!=gSysBlock.ChkSum)
	{
		Debug("RFS ChkSum error!\n\r");
		RFS_BurnDefaultToRom();
	}

	if(gSysBlock.Ver!=gDefBlock.Ver)
	{
		Debug("Ver Error,Reset Database!\n\r");
		RFS_BurnDefaultToRom();
	}
}

//�洢Ĭ�����ݵ�rom
void RFS_BurnDefaultToRom(void)
{
	__IO u32 *pChkSum=(void *)(IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE+sizeof(RFS_BLOCK)-4);
	
	MemCpy((void *)&gSysBlock,&gDefBlock,sizeof(RFS_BLOCK));
	gSysBlock.ChkSum=MakeHash33((void *)&gSysBlock,sizeof(RFS_BLOCK)-4);

	if(*pChkSum!=gSysBlock.ChkSum)		
		Rom_WritePage((void *)&gSysBlock,IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE,sizeof(gSysBlock));
}

//�洢��ǰ���ݵ�rom
void RFS_BurnToRom(void)
{
	__IO u32 *pChkSum=(void *)(IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE+sizeof(RFS_BLOCK)-4);

	gSysBlock.ChkSum=MakeHash33((void *)&gSysBlock,sizeof(RFS_BLOCK)-4);

	if(*pChkSum!=gSysBlock.ChkSum)	
		Rom_WritePage((void *)&gSysBlock,IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE,sizeof(gSysBlock));
}

//��ȡֵ
u32 RFS_GetNum(u16 Item)
{
	switch(Item)
	{
		case RFSI_NUM:
			return gSysBlock.Num;









	}

    return 0;
}

//����ֵ
bool RFS_SetNum(u16 Item,u32 Value,void *p)
{
	switch(Item)
	{
		case RFSI_NUM:
			gSysBlock.Num=Value;
			break;








		default:
			return FALSE;
	}

	return TRUE;
}


