#include "usart.h"		 

//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
int fputc(int ch,FILE *p)  //����Ĭ�ϵģ���ʹ��printf����ʱ�Զ�����
{
	USART_SendData(USART1,(u8)ch);	
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	return ch;
}


//��ʼ��IO ����1 
//bound:������
void USART1_Init(u32 bound)
{
   	//GPIO�˿�����
  	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  	//USART1_RX	  GPIOA.10��ʼ��
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  
  
	//Usart1 NVIC ����
  	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

   	//USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 	
}


void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 r;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  
	{
		r =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		USART_SendData(USART1,r);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC) != SET);
	} 
	USART_ClearFlag(USART1,USART_FLAG_TC);
} 	

 
//��ʼ��IO ����3 
//bound:������
void USART3_Init(u32 bound)

{  
    USART_InitTypeDef USART_InitStructure;  
  NVIC_InitTypeDef NVIC_InitStructure;   
    GPIO_InitTypeDef GPIO_InitStructure;    //����һ���ṹ�������������ʼ��GPIO  
   //ʹ�ܴ��ڵ�RCCʱ��  
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE); //ʹ��UART3����GPIOB��ʱ��  
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);  
  
   //����ʹ�õ�GPIO������  
   // Configure USART2 Rx (PB.11) as input floating    
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;  
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
   GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
   // Configure USART2 Tx (PB.10) as alternate function push-pull  
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
   GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
   //���ô���  
   USART_InitStructure.USART_BaudRate = bound;  
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;  
   USART_InitStructure.USART_StopBits = USART_StopBits_1;  
   USART_InitStructure.USART_Parity = USART_Parity_No;  
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  
  
  
   // Configure USART3   
   USART_Init(USART3, &USART_InitStructure);//���ô���3  
  
  // Enable USART1 Receive interrupts ʹ�ܴ��ڽ����ж�  
   USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  
   //���ڷ����ж��ڷ�������ʱ����  
   //USART_ITConfig(USART2, USART_IT_TXE, ENABLE);  
  
   // Enable the USART3   
   USART_Cmd(USART3, ENABLE);//ʹ�ܴ���3  
  
   //�����ж�����  
   //Configure the NVIC Preemption Priority Bits     
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  
  
   // Enable the USART3 Interrupt   
   NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;  
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
   NVIC_Init(&NVIC_InitStructure);  
      
      
}  



