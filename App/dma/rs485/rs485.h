#ifndef _rs485_H
#define _rs485_H

#include "system.h"

#define send_buf_len 500
extern u8 trans_buf[send_buf_len];  //��������
extern u8 sum_buf[send_buf_len];		//������������
extern u8 num_buf[send_buf_len];		//�ַ���
extern u8 num1_buf[send_buf_len];		//����������
extern int n;												//������
//ģʽ����
#define RS485_TX_EN		PGout(3)	//485ģʽ����.0,����;1,����.
														 
void RS485_Init(u32 bound);


#endif
