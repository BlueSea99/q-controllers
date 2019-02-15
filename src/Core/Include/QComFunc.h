#ifndef __Q_COM_FUNC_H__
#define __Q_COM_FUNC_H__

//��ͷ�ļ���������qwifiͨ�ŵĺ���
//���Ҫʹ�ô�����qwifiͨ�ţ��������ͷ�ļ�

typedef enum{
	QSE_NULL=0,
	QSE_READY,
	QSE_CONNECTING,
	QSE_DISCONNECT,
	QSE_RESET,
}QWIFI_STATE;

enum{
	SMF_SYS=0,//ϵͳ��Ϣ���������轫��Ϣ�㲥����������
	SMF_GSM,//���ţ��������轫��Ϣgsm��ָ���û�
	SMF_DEV_STR,//�豸�ַ�������dut���͸�app����΢����Ϣ�޸�
	SMF_PUSH,//������Ϣ���������轫��Ϣ�����ͷ���ָ���û�
};
typedef u8 SRV_MSG_FLAG;

const char *QCom_GetLastCmd(void);
void QCom_GetVarValue(const char *pTag);
void QCom_SetVarValue(const char *pTag,int Value,bool Signed);
void QCom_SendStr(u32 StrId,const char *pStr);
void QCom_SendMsg(u8 Flag,const char *pMsg);
void QCom_SendSta(void);
void QCom_ResetQwifi(void);

#endif
