#ifndef _rs485_H
#define _rs485_H

#include "system.h"

#define send_buf_len 500
extern u8 trans_buf[send_buf_len];  //条码内容
extern u8 sum_buf[send_buf_len];		//复合条码内容
extern u8 num_buf[send_buf_len];		//字符数
extern u8 num1_buf[send_buf_len];		//复合数传递
extern int n;												//复合数
//模式控制
#define RS485_TX_EN		PGout(3)	//485模式控制.0,接收;1,发送.
														 
void RS485_Init(u32 bound);


#endif
