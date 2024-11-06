/* ����:������ */
#include "includes.h"

volatile uint8_t  g_usart2_rx_buf[32];
volatile uint32_t g_usart2_rx_cnt=0;
volatile uint32_t g_usart2_rx_end=0;

static GPIO_InitTypeDef 	GPIO_InitStructure;
static USART_InitTypeDef 	USART_InitStructure;
static NVIC_InitTypeDef  	NVIC_InitStructure;
static EXTI_InitTypeDef		EXTI_InitStructure;
static RTC_TimeTypeDef  	RTC_TimeStructure;
static RTC_DateTypeDef  	RTC_DateStructure;
static RTC_InitTypeDef  	RTC_InitStructure;
static RTC_AlarmTypeDef 	RTC_AlarmStructure;
void asr_init(uint32_t baud)
{
	usart2_init(baud);	
	
	//�򿪶˿�D��Ӳ��ʱ�ӣ����ǹ���
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14; 	//6������
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;	//����
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;//���٣��ٶ�Խ�ߣ���ӦԽ�죬���ǹ��Ļ����
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;//��ʹ������������
	GPIO_Init(GPIOD,&GPIO_InitStructure);		
}

void asr_send_str(char * str)
{
	usart_send_str(USART2,str);
}

void usart2_init(uint32_t baud)
{
	//ʹ�ܶ˿�AӲ��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	
	//ʹ�ܴ���AӲ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	//����PA2��PA3Ϊ���ù�������
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//��PA2��PA3���ӵ�USART2��Ӳ��
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	
	
	//����USART1����ز����������ʡ�����λ��У��λ
	USART_InitStructure.USART_BaudRate = baud;//������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8λ����λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//1λֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ��������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�����ڷ��ͺͽ�������
	USART_Init(USART2, &USART_InitStructure);
	
	
	//ʹ�ܴ��ڽ��յ����ݴ����ж�
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//ʹ�ܴ���2����
	USART_Cmd(USART2,ENABLE);
}

extern QueueHandle_t g_queue_usart;


// void USART2_IRQHandler(void)
// {
// 	uint8_t d;
	
// 	if(USART_GetITStatus(USART2,USART_IT_RXNE) == SET)
// 	{
// 		d = USART_ReceiveData(USART2);
		
// 		g_usart2_rx_buf[g_usart2_rx_cnt++]=d;
		
// 		if(d == '#' || g_usart2_rx_cnt>=sizeof(g_usart2_rx_buf))
// 		{
// 			g_usart2_rx_end=1;
// 		}
		
// 		/* ����CPU���Ѿ���ɽ����ж����󣬿�����Ӧ�µĽ����ж����� */
// 		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
		
// 	}
// }

void USART2_IRQHandler(void)
{
	uint8_t d;
	
	if(USART_GetITStatus(USART2,USART_IT_RXNE) == SET)
	{
		d = USART_ReceiveData(USART2);
		
		g_usart2_rx_buf[g_usart2_rx_cnt++]=d;
		
		if(d == '#' || g_usart2_rx_cnt>=sizeof(g_usart2_rx_buf))
		{
			//�����ж��¼�
			xQueueSendFromISR(g_queue_usart,(void *)&g_usart2_rx_buf,NULL);
			g_usart2_rx_end=1;
		}
		
		/* ����CPU���Ѿ���ɽ����ж����󣬿�����Ӧ�µĽ����ж����� */
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
		
	}
}

void usart_send_str(USART_TypeDef* USARTx,char *str)
{
	char *p = str;
	
	while(*p!='\0')
	{
		USART_SendData(USARTx,*p);
		
		p++;
	
		//�ȴ����ݷ��ͳɹ�
		while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE)==RESET);
		USART_ClearFlag(USARTx,USART_FLAG_TXE);
	}
}


void usart_send_bytes(USART_TypeDef* USARTx,uint8_t *buf,uint32_t len)
{
	uint8_t *p = buf;
	
	while(len--)
	{
		USART_SendData(USARTx,*p);
		
		p++;
		
		//�ȴ����ݷ��ͳɹ�
		while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE)==RESET);
		USART_ClearFlag(USARTx,USART_FLAG_TXE);
	}
}
