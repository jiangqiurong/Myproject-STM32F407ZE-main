/* 作者:江秋荣 */
#include "includes.h"

void beep_init(void)
{    	 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);			

	//GPIOF8初始化设置 
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;		
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			    //普通输出模式，
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		    //100MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;				 //上拉
	GPIO_Init(GPIOF, &GPIO_InitStructure);						//初始化GPIOF，把配置的数据写入寄存器
	
	//蜂鸣器禁鸣
	BEEP(0);
}

