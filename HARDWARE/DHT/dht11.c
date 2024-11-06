#include "includes.h"

static GPIO_InitTypeDef 		GPIO_InitStructure;

void dht11_init(void)
{
	//使能端口G硬件时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);


	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	
	//初始化
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	
	//PG9输出高电平，这是引脚的初始电平状态
	PGout(9)=1;	
}

void dht11_pin_mode(GPIOMode_TypeDef pin_mode)
{

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode  = pin_mode;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	
	//初始化
	GPIO_Init(GPIOG,&GPIO_InitStructure);
}


int32_t dht11_read_data(uint8_t *pbuf)
{
	uint32_t t=0;
	int32_t i=0,j=0;
	uint8_t d=0;
	uint8_t *p = pbuf;
	uint8_t check_sum=0;
	
	//保证PG9为输出模式
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	
	//PG9输出低电平
	PGout(9)=0;
	
	//延时至少18ms
	delay_ms(18);
	
	//PG9输出高电平
	PGout(9)=1;	
	
	//延时30us
	delay_us(30);
	
	//保证PG9为输入模式
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOG,&GPIO_InitStructure);

	//等待低电平出现，添加超时处理
	t=0;
	while(PGin(9))
	{
		t++;
		
		delay_us(1);
		
		if(t>=4000)
			return -1;
	}
	
	//通过超时检测低电平的合法性
	t=0;
	while(PGin(9)==0)
	{
		t++;
		delay_us(1);
		
		if(t>=100)
			return -2;
	}


	//通过超时检测高电平的合法性
	t=0;
	while(PGin(9))
	{
		t++;
		delay_us(1);
		
		if(t>=100)
			return -3;
	}
	
	for(j=0; j<5; j++)
	{
		//接收一个字节，高位优先接收数据，bit7 bit6 ...... bit0
		d=0;
		for(i=7; i>=0; i--)
		{
			//通过超时检测低电平的合法性
			t=0;
			while(PGin(9)==0)
			{
				t++;
				delay_us(1);
				
				if(t>=100)
					return -4;
			}
		
			//延时40us~60us左右
			delay_us(40);
			
			//判断引脚的电平
			if(PGin(9))
			{
				//将d变量对应的bit位置1
				//0x01 1<<0:0000 0001
				//0x02 1<<1:0000 0010
				//.......
				//0x80 1<<7:1000 0000
				d|=1<<i;
				
				//等待剩下的高电平持续完毕
				t=0;
				while(PGin(9))
				{
					t++;
					delay_us(1);
					
					if(t>=100)
						return -5;
				}			
			
			}
		
		}	
	
		p[j]=d;
	}
	
	//校验数据
	check_sum = (p[0]+p[1]+p[2]+p[3])&0xFF;
	
	if(check_sum != p[4])
		return -6;
	
	return 0;
}
