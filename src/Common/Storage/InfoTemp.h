#ifndef INFO_TEMP_H
#define INFO_TEMP_H

#include "Product.h"


typedef s32 INFO_ADDR;//���Ե�ַ
typedef s32 INFO_SITE;//��Ե�ַ����0��ʼ
typedef u32 INFO_ID;//�û�����ָ����id������ֵ
typedef u32 INFO_IDX;//��Ч��Ϣ��������1��ʼ

#define INFO_FAILD (-1)
#define INFO_NOT_FOUND (-2)
#define INFO_PARAM_ERROR (-3)
#define INFO_RES_ERROR (-4)
#define INFO_SPACE_FULL (-5)


typedef enum{
#if PRODUCT_IS_JUMPER
	ITN_IR=0,
	ITN_RF,
#endif

	ITN_MAX
}INFO_TEMP_NAME;



u16 InfoTempStatistics(INFO_TEMP_NAME Name);
void InfoTempInit(void);
INFO_SITE SaveInfoTemp(INFO_TEMP_NAME Name,void *pData,u16 Byte);
INFO_SITE ReadInfoBySite(INFO_TEMP_NAME Name,INFO_SITE Site,void *pData,u16 Byte);
INFO_SITE ReadInfoTemp(INFO_TEMP_NAME Name,INFO_ID AppID,void *pData,u16 Byte);
u16 InfoTempTotalInc(INFO_TEMP_NAME Name);
void FlushTempInfo(INFO_TEMP_NAME Name);
void CleanTempInfo(INFO_TEMP_NAME Name);
INFO_SITE FindIdleSite(INFO_TEMP_NAME Name);
u16 GetInfoBlockSize(INFO_TEMP_NAME Name);
u16 GetInfoSize(INFO_TEMP_NAME Name);
u16 GetInfoMaxAllowNum(INFO_TEMP_NAME Name);
u16 GetInfoTotal(INFO_TEMP_NAME Name);





#endif

