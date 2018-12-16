#ifndef QSYS_DATABASE_H
#define QSYS_DATABASE_H

#include "stm32f10x.h"
#include "DbForSys.h"

typedef enum{
	SDN_SYS=0,//id,ip��ַ,ϵͳ����

	SDN_MAX
}SUB_DB_NAME;

void QDB_Init(void);
void QDB_BurnDefaultToSpiFlash(SUB_DB_NAME Name);
void QDB_BurnToSpiFlash(SUB_DB_NAME Name);
u32 QDB_GetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *Val);
bool QDB_SetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *pParam,u8 ByteLen);

void CopyNameToTemp(void);
void RestoreNameToDb(void);

#define QDB_GetNum(Name,Item) QDB_GetValue(Name,Item,0,NULL)//��ȡ�����������ϵͳ����
#define QDB_GetIp(Name,Item,pIpAddrBuf) QDB_GetValue(Name,Item,0,pIpAddrBuf)//��ȡ����ip���ϵͳ����
#define QDB_GetStr(Name,Item,pStr) QDB_GetValue(Name,Item,0,pStr)//��ȡ�����ַ������ϵͳ�������ַ�����������㹻�������ȴӷ���ֵ����

#define QDB_SetNum(Name,Item,Num) QDB_SetValue(Name,Item,Num,NULL,0)//���������������ϵͳ����
#define QDB_SetIp(Name,Item,pIpAddrBuf) QDB_SetValue(Name,Item,0,pIpAddrBuf,4)//��������ip���ϵͳ����
#define QDB_SetStr(Name,Item,pStr) QDB_SetValue(Name,Item,0,pStr,strlen(pStr))//���������ַ�����ϵͳ����

void DispDbForSys(void);

//ϵͳ״̬
#define GetSysStatus(Item) QDB_GetValue(SDN_SYS,Item,0,NULL) 
#define GetSysStatusNum(Item,Num) QDB_GetValue(SDN_SYS,Item,Num,NULL) 
#define SetSysStatus(Item,Num) QDB_SetValue(SDN_SYS,Item,Num,NULL,0)


#endif

