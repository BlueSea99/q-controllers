#ifndef SYS_DEFINES_H
#define SYS_DEFINES_H

#define SYS_TICK_PERIOD_MS  1  // ������ĺ�����
#define SYS_SCHEDULE_PERIOS_MS  4//�������ʱ�䣬����Ϊ2��n�η�
#define SYS_SCHEDULE_PERIOD_MASK 0x03//�����������
#define SYS_SCHEDULE_PERIOD_OFFSET 2//�൱�ڳ˳�����λ
#define WDG_CB_MS 500//ι������
#define TRIGGER_IN_POLL_MS 200//��ѯ�ź������ʱ��

#define Ms2Sch(Ms) ((Ms)>>SYS_SCHEDULE_PERIOD_OFFSET)
#define Sch2Ms(Period) ((Period)<<SYS_SCHEDULE_PERIOD_OFFSET)

#define MAX_VAR(a,b)  (a)>(b)?(a):(b)
#define MIN_VAR(a,b)  (a)<(b)?(a):(b)

#define Frame() Debug("                                                                    |\r");

#include "LimitMarco.h"
#include "Drivers.h"
#include "Q_Heap.h"
#include "Q_Queue.h"
#include "EventInHandler.h"
#include "NextLoopFunc.h"
#include "MsFunc.h"
#include "SecFunc.h"
#include "SysTimer.h"
#include "SpiFlashApi.h"
#include "RomFlashSave.h"
#include "SomeFunc.h"

extern const u32 __gBinSoftVer;
extern const u32 __gBinSize;
extern const u32 __gBinSum;

 
#endif

