/* 作者:江秋荣 */
#include "includes.h"

static	GPIO_InitTypeDef    GPIO_InitStructure;
static	USART_InitTypeDef  USART_InitStructure;
static	NVIC_InitTypeDef   NVIC_InitStructure;
static	EXTI_InitTypeDef   EXTI_InitStructure;

void ble_init(uint32_t baud)
{

	
	//端口B硬件时钟打开
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOE,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	//串口3硬件时钟打开
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	
	//配置PB10 PB11为AF模式（复用功能）
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11; 	//9 10号引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;//输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;//高速，速度越高，响应越快，但是功耗会更高
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;//不使能上下拉电阻
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	//由于引脚支持很多功能，需要指定该引脚的功能，当前要制定支持USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);	
	
	//配置USART1相关参数：波特率、数据位、停止位、校验位
	USART_InitStructure.USART_BaudRate = baud;					//波特率，就是通信的速度
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //8位数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//不需要校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件流控制功能不需要
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//串口允许发送和接收数据
	USART_Init(USART3, &USART_InitStructure);
	
	//配置中断触发方式，接收到一个字节，就通知CPU处理
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	//NVIC配置其的优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;			//中断号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//打开通道给NVIC管理
	NVIC_Init(&NVIC_InitStructure);
	
	//使能USART3工作
	USART_Cmd(USART3, ENABLE);
	//PE6
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	// /* 将外部中断连接到指定的引脚，特别说明：引脚编号决定了使用哪个外部中断 */
	// SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource6);
	
	// /* 配置外部中断0 */
	// EXTI_InitStructure.EXTI_Line = EXTI_Line6; //指定外部中断6
	// EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//工作在中断模式
	// EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;//双边沿触发：下降沿与上升沿触发中断
	// EXTI_InitStructure.EXTI_LineCmd = ENABLE;//允许外部中断6工作
	// EXTI_Init(&EXTI_InitStructure);
	
	// /* 通过NVIC管理外部中断9~5的中断请求：中断号、优先级、中断打开/关闭 */
	// NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//中断号
	// NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;//抢占优先级
	// NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//响应优先级
	// NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//中断打开
	// NVIC_Init(&NVIC_InitStructure);

}



extern QueueHandle_t g_queue_usart;
	
volatile uint8_t g_ble_buf[128]={0};
volatile uint8_t g_buf_size=0;


//串口3中断服务函数
void USART3_IRQHandler(void)
{
	uint8_t d;
	uint32_t ulReturn;

	
	/* 进入临界段，临界段可以嵌套 */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();
	
	
	if(USART_GetITStatus(USART3,USART_IT_RXNE) == SET)
	{
		d = USART_ReceiveData(USART3);
		g_ble_buf[g_buf_size++]=d;
		
		//等待串口发送完毕
		//while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
		if(d=='#' || g_buf_size>= sizeof(g_ble_buf))
		{
			
			//发送中断事件
			xQueueSendFromISR(g_queue_usart,(void *)&g_ble_buf,NULL);
			
			g_buf_size=0;
			memset((void *)g_ble_buf,0,64);
		}
		
		/* 告诉CPU，已经完成接收中断请求，可以响应新的接收中断请求 */
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
		
	}
	
	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );

}

uint32_t ble_connect_sta_get(void)
{
	if(PEin(6))
	{
		return FLAG_BLE_STATUS_CONNECT;
	}
	else
	{
		return FLAG_BLE_STATUS_NONE;
	}
}

// void EXTI9_5_IRQHandler(void)
// {
	
// 	uint32_t ulReturn;
	
// 	/* 进入临界段，临界段可以嵌套 */
// 	ulReturn = taskENTER_CRITICAL_FROM_ISR();
	
// 	if(EXTI_GetITStatus(EXTI_Line6) == SET)
// 	{
// 		// if(PEin(6))
// 		// {
// 		// 	g_ble_status=FLAG_BLE_STATUS_CONNECT;
// 		// }
// 		// else{
// 		// 	g_ble_status=FLAG_BLE_STATUS_NONE;
// 		// }
		
// 		/* 告诉CPU，已经完成接收中断请求，可以响应新的接收中断请求 */
// 		EXTI_ClearITPendingBit(EXTI_Line6);
		
// 	}
	
// 	/* 退出临界段 */
// 	taskEXIT_CRITICAL_FROM_ISR( ulReturn );

// }

