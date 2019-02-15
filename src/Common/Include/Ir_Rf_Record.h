#ifndef IR_AND_RF_H
#define IR_AND_RF_H

typedef enum{
	SIT_NULL=0,
	SIT_IR,
	SIT_RF_STD,
	SIT_RF_OTHERS,

}SIG_TYPE;

//------------------------------------ IR ----------------------------------------
#define IR_FUZZY_PARAM 10 //ģ���Աȵİٷֱ�����
#define MAX_IR_IDX_NUM 16
#define MAX_IR_PLUSE_NUM 428
typedef struct{
	u32 ID;
	SIG_TYPE Type:2;//�ź�����
	u16 SendCnt:4;//���͸���������4��Ϊʵ�ʷ��͸�����ir����
	u16 PulseNum:10;//�������
	
	u16 IdxTimes[MAX_IR_IDX_NUM];//����ֵ
	u8 Pluse[MAX_IR_PLUSE_NUM/2];//ÿ���ֽڷָ�4λ�͵�4λ���洢2�����峤�ȣ��ȴ��4λ����͵�ƽ
}IR_RECORD;


//------------------------------------ RF ----------------------------------------
#define RF_FUZZY_PARAM 10 //ģ���Աȵİٷֱ�����
#define MAX_RF_IDX_NUM 16
#define MAX_RF_PLUSE_NUM 428

typedef struct{
	u32 ID;//�洢id
	SIG_TYPE Type:2;//�ź�����
	u16 SendCnt:4;//���͸���������4��Ϊʵ�ʷ��͸���
	u16 PulseNum:10;//�������

	u32 Code;//���룬��24λ��Ч
	u16 BasePeriod;// 4a��ʱ�䳤�ȣ�ͬ��λ��λΪ124a����ߵ�ƽΪ12a��խ�ߵ�ƽΪ4a���͵�ƽΪ4a��12a
}RF_STD_RECORD;

typedef struct{
	u32 ID;
	SIG_TYPE Type:2;//�ź�����
	u16 SendCnt:4;//���͸���������4��Ϊʵ�ʷ��͸���
	u16 PulseNum:10;//�������

	u16 IdxTimes[MAX_RF_IDX_NUM];//����ֵ
	u8 Pluse[MAX_RF_PLUSE_NUM/2];//ÿ���ֽڷָ�4λ�͵�4λ���洢2�����峤�ȣ��ȴ��4λ����ߵ�ƽ
}RF_RECORD;











#endif
