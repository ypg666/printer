#include "rs485.h"
#include "SysTick.h"
#include "dma.h"
#include "rs485.h"
#include "string.h"
#include "usart.h"
#include "includes.h"
#include "tftlcd.h"

u8 send_buf[send_buf_len];
u8 trans_buf[send_buf_len];
u8 sum_buf[send_buf_len];
u8 num_buf[send_buf_len];
u8 num1_buf[send_buf_len];
int n=0;
/*******************************************************************************
* 函 数 名         : RS485_Init
* 函数功能		   : USART2初始化函数
* 输    入         : bound:波特率
* 输    出         : 无
*******************************************************************************/  
void RS485_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG|RCC_APB2Periph_GPIOA,ENABLE); //使能GPIOA\G时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
	
	/*  配置GPIO的模式和IO口 */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;	//TX-485	//串口输出PA2
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;		  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);		/* 初始化串口输入IO */
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;	//RX-485	   //串口输入PA3
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;	    //模拟输入
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;	//CS-485
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;	   //推挽输出
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	
	//USART2 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART2, &USART_InitStructure); //初始化串口2
	
	USART_Cmd(USART2, ENABLE);  //使能串口 2
	
	USART_ClearFlag(USART2, USART_FLAG_TC);
		
//	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启接受中断
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启接受中断

	//Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

	DMAx_Init(DMA1_Channel6,(u32)&USART2->DR,(u32)send_buf,send_buf_len);
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);  //使能串口2的DMA发送     
	DMAx_Enable(DMA1_Channel6,send_buf_len);     //开始一次DMA传输！

	RS485_TX_EN=0;				//默认为接收模式	
}

/*******************************************************************************
* 函 数 名         : USART2_IRQHandler
* 函数功能		   : USART2中断函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/ 
void USART2_IRQHandler(void)
{
	unsigned char nu=0;
	u8 sp[1];
	sp[0]=0x5f;
	
	OSIntEnter(); 	   //进入中断
		
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)//接收到数据  （上次DMA已接收完毕）
	{	 	
			n++;
//			printf("%s",send_buf); 
			LCD_Clear(WHITE);		
			memset(trans_buf,0,send_buf_len); 
			memcpy(trans_buf,send_buf,send_buf_len);
			strcat(sum_buf, trans_buf);
			strcat(sum_buf, sp);
			sprintf(num1_buf,"%d",n);
			sprintf(num_buf,"%d",strlen(sum_buf)-1);
			memset(send_buf,0,send_buf_len);  
			USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);  //使能串口2的DMA接收     
			DMAx_Enable(DMA1_Channel6,send_buf_len);     //开始一次DMA传输！
			nu = USART1->SR;
      nu = USART2->DR; //清除空闲中断标志位
	}  
	
	OSIntExit();	 //退出中断
} 	




