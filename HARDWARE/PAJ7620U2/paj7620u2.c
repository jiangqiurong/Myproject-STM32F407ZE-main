#include "includes.h"

static GPIO_InitTypeDef	GPIO_InitStructure;
static EXTI_InitTypeDef	EXTI_InitStructure;	
static NVIC_InitTypeDef	NVIC_InitStructure;

//选择PAJ7620U2 BANK区域
void paj7620u2_selectBank(bank_e bank)
{
	switch(bank)
	{
		case BANK0: GS_Write_Byte(PAJ_REGITER_BANK_SEL,PAJ_BANK0);break;//BANK0寄存器区域
		case BANK1: GS_Write_Byte(PAJ_REGITER_BANK_SEL,PAJ_BANK1);break;//BANK1寄存器区域
		default:break;
	}		
}

//PAJ7620U2唤醒
uint8_t paj7620u2_wakeup(void)
{ 
	uint8_t data=0x0a;
	GS_WakeUp();//唤醒PAJ7620U2
	delay_ms(5);//唤醒时间>400us
	GS_WakeUp();//唤醒PAJ7620U2
	delay_ms(5);//唤醒时间>400us
	paj7620u2_selectBank(BANK0);//进入BANK0寄存器区域
	data = GS_Read_Byte(0x00);//读取状态
	if(data!=0x20) 
		return 0; //唤醒失败
	
	return 1;
}

/* 
默认原版paj7620u2代码没有支持中断处理
修改日期：2021/7/25
修改人：粤嵌.温子祺
*/
void paj7620u2_irq_init(void)
{
	//使能端口E的硬件时钟，端口E才能工作，说白了就是对端口E上电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	//使能系统配置硬件时钟，说白了就是对系统配置的硬件供电
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	//配置硬件，配置GPIO，端口E，第6个引脚
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;			//第6个引脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;		//输入模式
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;	//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;		//由于外部没有有上拉电阻，需要使能上拉电阻
	GPIO_Init(GPIOC,&GPIO_InitStructure);	

	//将PE6引脚连接到EXTI6
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC,EXTI_PinSource7);	


	EXTI_InitStructure.EXTI_Line = EXTI_Line7;				//EXTI6
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//中断触发
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				//使能
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;	//外部中断6的中断号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;						//响应优先级 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVIC使能
	NVIC_Init(&NVIC_InitStructure);			
}

//PAJ7620U2初始化
//返回值：0:失败 1:成功
uint8_t paj7620u2_init(void)
{
	uint8_t i;
	uint8_t status;
	
	/* 增加中断 */
	paj7620u2_irq_init();	
	
	GS_i2c_init();//IIC初始化
    status = paj7620u2_wakeup();//唤醒PAJ7620U2
	if(!status) 
		return 0;
	paj7620u2_selectBank(BANK0);//进入BANK0寄存器区域
	for(i=0;i<INIT_SIZE;i++)
	{
		GS_Write_Byte(init_Array[i][0],init_Array[i][1]);//初始化PAJ7620U2
	}
    paj7620u2_selectBank(BANK0);//切换回BANK0寄存器区域

	return 1;
}


//手势识别测试
uint16_t gesture_get(void)
{
    uint8_t status;
	uint8_t data[2]={0x00};
	uint16_t gesture_data = 0;

	//读取手势状态			
	status = GS_Read_nByte(PAJ_GET_INT_FLAG1,2,&data[0]);
	
	if(!status)
	{   
		gesture_data =(unsigned short int)data[1]<<8 | data[0];
#if 0	
		if(gesture_data) 
		{
			switch(gesture_data)
			{
				case GES_UP:               printf(KERN_DEBUG"Up\r\n");	 	//向上		  
				break;
					 
				case GES_DOWN:             printf(KERN_DEBUG"Down\r\n"); 		//向下				
				break; 
					 
				case GES_LEFT:             printf(KERN_DEBUG"Left\r\n");   	//向左             
				break; 
				
				case GES_RIGHT:            printf(KERN_DEBUG"Right\r\n");     //向右          
				break; 
				
				case GES_FORWARD:          printf(KERN_DEBUG"Forward\r\n"); 	//向前            
				break; 
				
				case GES_BACKWARD:         printf(KERN_DEBUG"Backward\r\n"); 	//向后           
				break; 
				
				case GES_CLOCKWISE:        printf(KERN_DEBUG"Clockwise\r\n"); //顺时针      
				break; 
				
				case GES_COUNT_CLOCKWISE:  printf(KERN_DEBUG"AntiClockwise\r\n");  //逆时针
				break; 
				
				case GES_WAVE:             printf(KERN_DEBUG"Wave\r\n");  	//挥动              
				break; 
				
				default:   
				break;
				
			}		
		}
#endif		
	}
	
	return gesture_data;
}



void EXTI9_5_IRQHandler(void)
{
	uint32_t ulReturn;

	/* 进入临界段，临界段可以嵌套 */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();	

	/* 检测标志位 */
	if(EXTI_GetITStatus(EXTI_Line7) == SET)
	{
		xEventGroupSetBitsFromISR(g_event_group,EVENT_GROUP_GESTURE,NULL);
		
		EXTI_ClearITPendingBit(EXTI_Line7);
	}

	if(EXTI_GetITStatus(EXTI_Line6) == SET)
	{
		// if(PEin(6))
		// {
		// 	g_ble_status=FLAG_BLE_STATUS_CONNECT;
		// }
		// else{
		// 	g_ble_status=FLAG_BLE_STATUS_NONE;
		// }
		
		/* 告诉CPU，已经完成接收中断请求，可以响应新的接收中断请求 */
		EXTI_ClearITPendingBit(EXTI_Line6);
		
	}
	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );	
}



