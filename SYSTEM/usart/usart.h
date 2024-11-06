#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

//如果想串口中断接收，请不要注释以下宏定义
void usart_init(u32 baud);

#endif


