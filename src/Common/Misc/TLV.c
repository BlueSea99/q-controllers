#include "SysDefines.h"



void TLV_Debug(u8 *pTlv,u16 Len)
{
	u8 Buf[64];
	TLV_DATA *pItem=(void *)Buf;
	u16 Idx=1;
	u8 TlvLen=1;
	
	while(TlvLen)
	{
		TlvLen=TLV_Decode(pTlv,Len,Idx++,pItem);
		if(TlvLen) Debug("[%d]%s:%s\n\r",pItem->Len,gNameSrvValueType[pItem->Type],pItem->Str);
	}
}

//�Ὣֵ���뵽pOut��ĩβ
//�������峤��
//���󷵻�0
u16 TLV_Build(u8 *pOut,u16 BufLen,SRV_VALUE_TYPE Type,u8 *ValueStr)
{
	u16 Len,Sum=0;
	u8 i;
	
	for(i=0;i<SVT_MAX;i++)//���������̫��
	{
		if(pOut[0]==SVT_NULL)//����λ��
		{
			//��ʼ���
			Len=strlen((void *)ValueStr)+1;
			if(Len>0xfe) return 0; 
			pOut[0]=Type;
			pOut[1]=Len;
			pOut+=2;
			MemCpy(pOut,ValueStr,Len);
			pOut[Len-1]=0;//������
			pOut[Len]=SVT_NULL;//����һ��tlv��type��ֵ����ֹ�´�Խ��
			return Sum+Len+2;
		}
		else//��ֵ
		{
			Len=pOut[1];
			pOut=&pOut[Len+2];
			Sum+=(Len+2);
			if((Sum+3+strlen((void *)ValueStr))>=BufLen) return 0;//Խ����
		}
	}

	return 0;
}

//����tlv
//idx��1��ʼ
//�������ݱ�������pItem��
//���󷵻�0����ȷ���س���
u8 TLV_Decode(u8 *pIn,u16 BufLen,u16 Idx,TLV_DATA *pItem)
{
	u16 Len,Sum=0;
	u8 i;

	for(i=1;i<=SVT_MAX;i++)//�������̫��
	{
		if(pIn[0]==SVT_NULL)//����λ��
		{
			return 0;
		}
		else//��ֵ
		{
			if(i==Idx)//ȡֵ
			{
				pItem->Type=pIn[0];
				pItem->Len=pIn[1];
				MemCpy(pItem->Str,&pIn[2],pItem->Len);
				return pItem->Len;
			}
			else//����
			{
				Len=pIn[1];
				pIn=&pIn[Len+2];
				Sum+=(Len+2);
				if((Sum+2)>=BufLen) return 0;//Խ����
			}
		}
	}

	return 0;
}


