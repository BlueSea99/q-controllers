#ifndef __ROM_FLASH_SAVE_H__
#define __ROM_FLASH_SAVE_H__

#include "Product.h"

typedef struct{//�������ݽṹ
	u16 Ver;//�޸ĺ�ᵼ�»ָ�Ĭ��

	//�û��Լ���������ݴ洢��ע���ܳ��Ȳ�Ҫ����ROM_FLASH_PAGE_SIZE
	u32 Num;//ʾ��

	
	u32 ChkSum;//ϵͳ�ڲ�ʹ�ã������޸�
}RFS_BLOCK;//�洢�飬���1k


void RFS_Debug(void);
void RFS_Init(void);
void RFS_BurnDefaultToRom(void);
void RFS_BurnToRom(void);
const RFS_BLOCK *RFS_DefDB(void);
RFS_BLOCK *RFS_DB(void);


#endif

