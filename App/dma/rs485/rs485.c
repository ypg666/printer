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
* �� �� ��         : RS485_Init
* ��������		   : USART2��ʼ������
* ��    ��         : bound:������
* ��    ��         : ��
*******************************************************************************/  
void RS485_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG|RCC_APB2Periph_GPIOA,ENABLE); //ʹ��GPIOA\Gʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2ʱ��
	
	/*  ����GPIO��ģʽ��IO�� */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;	//TX-485	//�������PA2
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;		  //�����������
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);		/* ��ʼ����������IO */
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;	//RX-485	   //��������PA3
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;	    //ģ������
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;	//CS-485
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;	   //�������
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	
	//USART2 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������2
	
	USART_Cmd(USART2, ENABLE);  //ʹ�ܴ��� 2
	
	USART_ClearFlag(USART2, USART_FLAG_TC);
		
//	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//���������ж�
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//���������ж�

	//Usart2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

	DMAx_Init(DMA1_Channel6,(u32)&USART2->DR,(u32)send_buf,send_buf_len);
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);  //ʹ�ܴ���2��DMA����     
	DMAx_Enable(DMA1_Channel6,send_buf_len);     //��ʼһ��DMA���䣡

	RS485_TX_EN=0;				//Ĭ��Ϊ����ģʽ	
}

/*******************************************************************************
* �� �� ��         : USART2_IRQHandler
* ��������		   : USART2�жϺ���
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/ 
void USART2_IRQHandler(void)
{
	unsigned char nu=0;
	u8 sp[1];
	sp[0]=0x5f;
	
	OSIntEnter(); 	   //�����ж�
		
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)//���յ�����  ���ϴ�DMA�ѽ�����ϣ�
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
			USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);  //ʹ�ܴ���2��DMA����     
			DMAx_Enable(DMA1_Channel6,send_buf_len);     //��ʼһ��DMA���䣡
			nu = USART1->SR;
      nu = USART2->DR; //��������жϱ�־λ
	}  
	
	OSIntExit();	 //�˳��ж�
} 	




