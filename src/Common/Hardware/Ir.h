#ifndef _IR_DRIVER_H_
#define _IR_DRIVER_H_

#include "Ir_Rf_Record.h"

void DisplayIrRecord(const IR_RECORD *pIrRcd);

void IrPulseIn_ISR(void);

void StartRecvIr(void);//��ʼ�ȴ����պ����ź�
void StopRecvIr(void);//ֹͣ���պ����ź�
void StartSendIr(const IR_RECORD *pIrRcd);//�������ݷ�������ź�

bool CaptureRecvIr(IR_RECORD *pIr);
IR_RECORD *SetCaptureBuf(IR_RECORD *pIr);

#endif

