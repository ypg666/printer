/********************   One Artisan's Regret And Pride   ***********************
*
*	@File    	： main.c
*	@Brief   	： 扫码打印机主函数
*	@Hardware	： STM32F103ZE
*	@Date		： 2018.11
*	@Description： 扫码枪通过485接串口2，DMA方式传输数据（DMA1通道6）。打印机通过232接串口3，DMA方式接收数据(DMA1通道2)。LCD为TFTLCD_HX8352C。
* 注意空闲中断的清除位的方式与常规中断不同。
* 注意打印机波特率115200
*	@History	： 
*
*	Rev1.0 
*		Date：
*		Modification：更新

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

//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		512
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define LED1_TASK_PRIO		6
//任务堆栈大小	
#define LED1_STK_SIZE 		128
//任务控制块
OS_TCB Led1TaskTCB;
//任务堆栈	
CPU_STK LED1_TASK_STK[LED1_STK_SIZE];
void led1_task(void *p_arg);

//任务优先级
#define LCD1_TASK_PRIO		5
//任务堆栈大小	
#define LCD1_STK_SIZE 		2048
//任务控制块
OS_TCB LCD1TaskTCB;
//任务堆栈	
CPU_STK LCD1_TASK_STK[LCD1_STK_SIZE];
void lcd1_task(void *p_arg);

//任务优先级
#define PRINT1_TASK_PRIO		4
//任务堆栈大小	
#define PRINT1_STK_SIZE 		512
//任务控制块
OS_TCB PRINT1TaskTCB;
//任务堆栈	
CPU_STK PRINT1_TASK_STK[PRINT1_STK_SIZE];
void print1_task(void *p_arg);

int main()
{  	
	OS_ERR err;
	
	Systick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //中断优先级分组 分2组
	LED_Init();
	KEY_Init();
	BEEP_Init();beep=1;
	USART1_Init(9600);
	USART3_Init(115200);
	RS485_Init(9600);
	TFTLCD_Init();			//LCD初始化
	
	OSInit(&err);		//初始化UCOSIII
	
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值 
	OSStart(&err);  //开启UCOSIII
	while(1);
}

//开始任务函数
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
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//进入临界区
	//创建LED1任务
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

	//创建LCD1任务
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
								 
	//创建PRINT1任务
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
				 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//进入临界区
}

//led1任务函数
void led1_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	while(1)
	{
		led1=!led1;
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); //延时200ms
	}
}

//lcd1任务函数
void lcd1_task(void *p_arg)
{
	unsigned int i;
	OS_ERR err;
	p_arg = p_arg;
	
//	LCD_ShowFontHZ(10, 10,"条码内容");
//	LCD_ShowFontHZ(10, 100,"复合条码内容");
//	LCD_ShowFontHZ(115, 370,"打印请按");
//	LCD_ShowFontHZ(115, 340,"字符数 ");
	while(1)
	{
//		i=strlen(sum_buf);sum_buf[i-1]=0x00;
		switch(s)
		{
		case 1:
			
			LCD_ShowString(45,10,tftlcd_data.width,tftlcd_data.height,24,"1.");
			LCD_ShowFontHZ(70, 10,"条码内容");
			LCD_ShowFontHZ(115, 340,"复合数 ");
			FRONT_COLOR=BLACK;
			LCD_ShowString(10,60,tftlcd_data.width,tftlcd_data.height,24,trans_buf);
			LCD_ShowString(210,340,tftlcd_data.width,tftlcd_data.height,24,num1_buf);
			LCD_ShowFontHZ(115, 370,"打印请按");
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
			break;
		
		case 2:
			
			LCD_ShowString(15,10,tftlcd_data.width,tftlcd_data.height,24,"2.");
			LCD_ShowFontHZ(40, 10,"复合条码内容");
			LCD_ShowFontHZ(115, 340,"字符数");
			LCD_ShowFontHZ(115, 370,"打印请按");
			LCD_ShowString(10,60,tftlcd_data.width,tftlcd_data.height,24,sum_buf);
			LCD_ShowString(210,340,tftlcd_data.width,tftlcd_data.height,24,num_buf);
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
			break;
		
		case 3:
			
			LCD_ShowString(35,10,tftlcd_data.width,tftlcd_data.height,24,"3.");
			LCD_ShowFontHZ(70, 10,"参数设置");
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
			break;
		
		default: OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
	}
	}
}


//print1任务函数
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
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时
		switch(key)
		{
		case KEY_UP:
		
		//	i=strlen(sum_buf);
			memset(print_buf,0,send_buf_len); 
		
			print_buf[0]=0x1a;print_buf[1]=0x5b;print_buf[2]=0x01;print_buf[3]=0x01;print_buf[4]=0x01;print_buf[5]=0x01;print_buf[6]=0x01;print_buf[7]=0x80;print_buf[8]=0x01;print_buf[9]=0x08;print_buf[10]=0x01;print_buf[11]=0x01;  //页开始
			print_buf[12]=0x1a;print_buf[13]=0x31;print_buf[14]=0x01;print_buf[15]=0x03;print_buf[16]=0x03;print_buf[17]=0x80;print_buf[18]=0x01;print_buf[19]=0x40;print_buf[20]=0x01;print_buf[21]=0x04;print_buf[22]=0x01; //二维码位置
			if(s==1){i=strlen(trans_buf);strcat(print_buf,trans_buf);}
			else if(s==2)
				{
					if(n==1)
					{memset(print_buf,0,send_buf_len);beep=0;OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_HMSM_STRICT,&err);beep=1;break;}
					else
					{i=strlen(sum_buf);strcat(print_buf,sum_buf);print_buf[22+i]=0x00;}
				}   //二维码内容
			print_buf[23+i]=0x00;
			
			print_buf[3]=0x00;print_buf[4]=0x00;print_buf[5]=0x00;print_buf[6]=0x00;print_buf[11]=0x00;print_buf[14]=0x00;print_buf[18]=0x00;print_buf[20]=0x00;print_buf[22]=0x00;
			
			print_buf[23+i+1]=0x1a;print_buf[23+i+2]=0x5d;print_buf[23+i+3]=0x00; //页结束
			print_buf[23+i+4]=0x1a;print_buf[23+i+5]=0x4f;print_buf[23+i+6]=0x00; //页打印
			

			
			DMAy_Init(DMA1_Channel2,(u32)&USART3->DR,(u32)print_buf,30+i);
			USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);  //使能串口3的DMA发送     
			DMAx_Enable(DMA1_Channel2,30+i); 

			key=0;
			LCD_Clear(WHITE);
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时
//			break;
		
		case KEY_DOWN:
		
			memset(trans_buf,0,send_buf_len);  
			memset(sum_buf,0,send_buf_len);
			memset(num_buf,0,send_buf_len);
			memset(num1_buf,0,send_buf_len);
			key=0;
			n=0;
			LCD_Clear(WHITE);
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时
			break;
		
		case KEY_LEFT:
		
			s=s-1;
			if(s==0){s=3;}
			key=0;
			LCD_Clear(WHITE);
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时
			break;
		
		case KEY_RIGHT:
		
			s++;
			if(s==4){s=1;}
			key=0;
			LCD_Clear(WHITE);
			OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时
			break;

		default: OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
	}
	}
}
