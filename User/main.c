/********************   One Artisan's Regret And Pride   ***********************
*
*	@File    	�� main.c
*	@Brief   	�� ɨ���ӡ��������
*	@Hardware	�� STM32F103ZE
*	@Date		�� 2018.11
*	@Description�� ɨ��ǹͨ��485�Ӵ���2��DMA��ʽ�������ݣ�DMA1ͨ��6������ӡ��ͨ��232�Ӵ���3��DMA��ʽ��������(DMA1ͨ��2)��LCDΪTFTLCD_HX8352C��
* ע������жϵ����λ�ķ�ʽ�볣���жϲ�ͬ��
* ע���ӡ��������115200
*	@History	�� 
*
*	Rev1.0 
*		Date��
*		Modification������

*----------------------All rights reserved-------------------------------------
*
********************                  ********************/

#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "key.h"
#include "includes.h"
#include "usart.h"
#include "dma.h"
#include "rs485.h"
#include "string.h"
#include "tftlcd.h"
#include "beep.h"

int s=1;

//�������ȼ�
#define START_TASK_PRIO		3
//�����ջ��С	
#define START_STK_SIZE 		512
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);

//�������ȼ�
#define LED1_TASK_PRIO		6
//�����ջ��С	
#define LED1_STK_SIZE 		128
//������ƿ�
OS_TCB Led1TaskTCB;
//�����ջ	
CPU_STK LED1_TASK_STK[LED1_STK_SIZE];
void led1_task(void *p_arg);

//�������ȼ�
#define LCD1_TASK_PRIO		5
//�����ջ��С	
#define LCD1_STK_SIZE 		2048
//������ƿ�
OS_TCB LCD1TaskTCB;
//�����ջ	
CPU_STK LCD1_TASK_STK[LCD1_STK_SIZE];
void lcd1_task(void *p_arg);

//�������ȼ�
#define PRINT1_TASK_PRIO		4
//�����ջ��С	
#define PRINT1_STK_SIZE 		512
//������ƿ�
OS_TCB PRINT1TaskTCB;
//�����ջ	
CPU_STK PRINT1_TASK_STK[PRINT1_STK_SIZE];
void print1_task(void *p_arg);

int main()
{  	
	OS_ERR err;
	
	Systick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //�ж����ȼ����� ��2��
	LED_Init();
	KEY_Init();
	BEEP_Init();beep=1;
	USART1_Init(9600);
	USART3_Init(115200);
	RS485_Init(9600);
	TFTLCD_Init();			//LCD��ʼ��
	
	OSInit(&err);		//��ʼ��UCOSIII
	
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ 
	OSStart(&err);  //����UCOSIII
	while(1);
}

//��ʼ������
void start_task(void *p_arg)
{
	OS_ERR err;
	
	CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
	
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
	
	cpu_clk_freq = BSP_CPU_ClkFreq();                           /* Determine SysTick reference freq.                    */
    cnts = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;        /* Determine nbr SysTick increments                     */
    OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).              */

    Mem_Init();       
	
	
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//�����ٽ���
	//����LED1����
	OSTaskCreate((OS_TCB 	* )&Led1TaskTCB,		
				 (CPU_CHAR	* )"led1 task", 		
                 (OS_TASK_PTR )led1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LED1_TASK_PRIO,     
                 (CPU_STK   * )&LED1_TASK_STK[0],	
                 (CPU_STK_SIZE)LED1_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);

	//����LCD1����
	OSTaskCreate((OS_TCB 	* )&LCD1TaskTCB,		
				 (CPU_CHAR	* )"lcd1 task", 		
                 (OS_TASK_PTR )lcd1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LCD1_TASK_PRIO,     
                 (CPU_STK   * )&LCD1_TASK_STK[0],	
                 (CPU_STK_SIZE)LCD1_STK_SIZE/10,	
                 (CPU_STK_SIZE)LCD1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
								 
	//����PRINT1����
	OSTaskCreate((OS_TCB 	* )&PRINT1TaskTCB,		
				 (CPU_CHAR	* )"print1 task", 		
                 (OS_TASK_PTR )print1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )PRINT1_TASK_PRIO,     
                 (CPU_STK   * )&PRINT1_TASK_STK[0],	
                 (CPU_STK_SIZE)PRINT1_STK_SIZE/10,	
                 (CPU_STK_SIZE)PRINT1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);								 
				 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//����ʼ����			 
	OS_CRITICAL_EXIT();	//�����ٽ���
}

//led1������
void led1_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	while(1)
	{
		led1=!led1;
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ200ms
	}
}

//lcd1������
void lcd1_task(void *p_arg)
{
	unsigned int i;
	OS_ERR err;
	p_arg = p_arg;
	
//	LCD_ShowFontHZ(10, 10,"��������");
//	LCD_ShowFontHZ(10, 100,"������������");
//	LCD_ShowFontHZ(115, 370,"��ӡ�밴");
//	LCD_ShowFontHZ(115, 340,"�ַ��� ");
	while(1)
	{
//		i=strlen(sum_buf);sum_buf[i-1]=0x00;
		switch(s)
		{
		case 1:
			
			LCD_ShowString(45,10,tftlcd_data.width,tftlcd_data.height,24,"1.");
			LCD_ShowFontHZ(70, 10,"��������");
			LCD_ShowFontHZ(115, 340,"������ ");
			FRONT_COLOR=BLACK;
			LCD_ShowString(10,60,tftlcd_data.width,tftlcd_data.height,24,trans_buf);
			LCD_ShowString(210,340,tftlcd_data.width,tftlcd_data.height,24,num1_buf);
			LCD_ShowFontHZ(115, 370,"��ӡ�밴");
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ20ms
			break;
		
		case 2:
			
			LCD_ShowString(15,10,tftlcd_data.width,tftlcd_data.height,24,"2.");
			LCD_ShowFontHZ(40, 10,"������������");
			LCD_ShowFontHZ(115, 340,"�ַ���");
			LCD_ShowFontHZ(115, 370,"��ӡ�밴");
			LCD_ShowString(10,60,tftlcd_data.width,tftlcd_data.height,24,sum_buf);
			LCD_ShowString(210,340,tftlcd_data.width,tftlcd_data.height,24,num_buf);
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ20ms
			break;
		
		case 3:
			
			LCD_ShowString(35,10,tftlcd_data.width,tftlcd_data.height,24,"3.");
			LCD_ShowFontHZ(70, 10,"��������");
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ20ms
			break;
		
		default: OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ20ms
	}
	}
}


//print1������
void print1_task(void *p_arg)
{
	OS_ERR err;
	u8 key;
	u8 print_buf[500];
//	int i=atoi(num_buf);
	unsigned int i;
//	sscanf(num_buf,"%d",&i);
	

	p_arg = p_arg;
	
	while(1)
	{
		key=KEY_Scan(0);
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ
		switch(key)
		{
		case KEY_UP:
		
		//	i=strlen(sum_buf);
			memset(print_buf,0,send_buf_len); 
		
			print_buf[0]=0x1a;print_buf[1]=0x5b;print_buf[2]=0x01;print_buf[3]=0x01;print_buf[4]=0x01;print_buf[5]=0x01;print_buf[6]=0x01;print_buf[7]=0x80;print_buf[8]=0x01;print_buf[9]=0x08;print_buf[10]=0x01;print_buf[11]=0x01;  //ҳ��ʼ
			print_buf[12]=0x1a;print_buf[13]=0x31;print_buf[14]=0x01;print_buf[15]=0x03;print_buf[16]=0x03;print_buf[17]=0x80;print_buf[18]=0x01;print_buf[19]=0x40;print_buf[20]=0x01;print_buf[21]=0x04;print_buf[22]=0x01; //��ά��λ��
			if(s==1){i=strlen(trans_buf);strcat(print_buf,trans_buf);}
			else if(s==2)
				{
					if(n==1)
					{memset(print_buf,0,send_buf_len);beep=0;OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_HMSM_STRICT,&err);beep=1;break;}
					else
					{i=strlen(sum_buf);strcat(print_buf,sum_buf);print_buf[22+i]=0x00;}
				}   //��ά������
			print_buf[23+i]=0x00;
			
			print_buf[3]=0x00;print_buf[4]=0x00;print_buf[5]=0x00;print_buf[6]=0x00;print_buf[11]=0x00;print_buf[14]=0x00;print_buf[18]=0x00;print_buf[20]=0x00;print_buf[22]=0x00;
			
			print_buf[23+i+1]=0x1a;print_buf[23+i+2]=0x5d;print_buf[23+i+3]=0x00; //ҳ����
			print_buf[23+i+4]=0x1a;print_buf[23+i+5]=0x4f;print_buf[23+i+6]=0x00; //ҳ��ӡ
			

			
			DMAy_Init(DMA1_Channel2,(u32)&USART3->DR,(u32)print_buf,30+i);
			USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);  //ʹ�ܴ���3��DMA����     
			DMAx_Enable(DMA1_Channel2,30+i); 

			key=0;
			LCD_Clear(WHITE);
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ
//			break;
		
		case KEY_DOWN:
		
			memset(trans_buf,0,send_buf_len);  
			memset(sum_buf,0,send_buf_len);
			memset(num_buf,0,send_buf_len);
			memset(num1_buf,0,send_buf_len);
			key=0;
			n=0;
			LCD_Clear(WHITE);
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ
			break;
		
		case KEY_LEFT:
		
			s=s-1;
			if(s==0){s=3;}
			key=0;
			LCD_Clear(WHITE);
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ
			break;
		
		case KEY_RIGHT:
		
			s++;
			if(s==4){s=1;}
			key=0;
			LCD_Clear(WHITE);
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ
			break;

		default: OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ20ms
	}
	}
}
