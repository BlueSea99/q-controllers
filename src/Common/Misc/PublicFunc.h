#ifndef QSYS_PUBLIC_FUNC_H
#define QSYS_PUBLIC_FUNC_H


//2									���ú���	(PublicFunc.c)						

//�ַ���ת��Ϊ����
//pStr������ʮ���Ƶ����֣����������Ҫ�󣬷���0
u32 StrToUint(const u8 *pStr);

//�з���ת��
//pStr�����������Ҫ�󣬷���-1
//֧��ʮ����,ʮ������,������
//���Կո���������ȷ�ַ����˳�
//��һ���ַ������ֻ��߸���
s32 StrToSint(const u8 *pStr);

//������ת��Ϊ���ͣ���������
int FloatToInt(float f);

//������תieee��׼
u32 Float2Ieee(float f_num);

//ieee��׼ת������
float Ieee2Float(u32 b_num);

#if 1
void MemSet(void *Dst,u8 C,u16 Byte);
void MemCpy(void *Dst,const void *Src,u16 Byte);
#else
#define MemSet memset
#define MemCpy memcpy
#endif

//�Ƚ������ڴ�����ֽ��Ƿ���ͬ������TRUE��ʾ��ͬ��Lenָ���Ƚϳ���
bool CompareBuf(u8 *Buf1,u8 *Buf2,u16 Len);

//��ӡָ�����ȵ��ַ���
void DisplayStrN(const u8 *Buf,u16 Len);

//��ӡbuf����
void DisplayBuf(const u8 *Buf,u16 Len,u8 RawLen);
void DisplayBufU16_Dec(const u16 *Buf,u16 Len,u8 RawLen);
void DisplayBufU16(const u16 *Buf,u16 Len,u8 RawLen);
void DisplayBufU32(const u32 *Buf,u16 Len,u8 RawLen);

bool FuzzyEqual(u32 A,u32 B,u8 Tole);

bool Str2Ip(const u8 *pStr,u8 *Ip);
char *Ip2Str(void *pIp);

void StrToLower(u8 *pStr);

bool IsNullStr(u8 *pStr);
bool NotNullStr(u8 *p);

u16 StrnCmp(const u8 *pStr1,const u8 *pStr2,u16 Bytes);
u8 *ChkStr(const u8 *pStr1, const u8 *pStr2 );
u8 *FindStr(u8 *pStr,u8 *pStrStart,u8 *pStrEnd);

u32 AlignTo4(u32 v);
bool IsAlign4(u32 v);
u32 AlignTo8(u32 v);
bool IsAlign8(u32 v);

u32 MakeHash33(u8 *pData,u32 Len);

u32 CheckSum(const u8 *pData,u32 Len);

u16 CRC16(const u8 *pData,u16 Len);

u16 Rev16(u16 Data);

u32 Rev32(u32 Data);

u32 Rand(u32 Mask);

u32 GetHwID(u8 *pID);

u32 GenerateDefPw(void);
u32 GetDutRegSn(void);
u8 *GetMacAddress(void);

void PrintChineseCharToCode(const u8 *pChineseStr);
u32 SaveCpuStatus(void);
void RestoreCpuStatus(u32);

#if 1
#define IntSaveInit() u32 cpu_sr=0;
#define EnterCritical() cpu_sr=SaveCpuStatus()
#define LeaveCritical() RestoreCpuStatus(cpu_sr)
#else
#define IntSaveInit() 
#define EnterCritical() __set_PRIMASK(1);
#define LeaveCritical() __set_PRIMASK(0);
#endif

#endif

