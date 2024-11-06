#include "stm32f4xx.h"
#include "sys.h"
#include "i2c.h"
#include "delay.h"


static GPIO_InitTypeDef GPIO_InitStructure;


void i2c_init(void)
{

	//使能端口B的硬件时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	


	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9;	//第 8 9 个引脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;		//输出模式
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;		//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;		//开漏
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;		//不需要上下拉电阻
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	
	//只要是输出模式，肯定会有初始电平状态，看时序图，空闲状态为高电平
	SCL_W=1;
	SDA_W=1;

}

void sda_pin_mode(GPIOMode_TypeDef pin_mode)
{
	//配置硬件，配置GPIO，端口F，第9个引脚
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;			//第9 个引脚
	GPIO_InitStructure.GPIO_Mode=pin_mode;				//输出模式
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;		//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;		//开漏
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;		//不需要上下拉电阻
	GPIO_Init(GPIOB,&GPIO_InitStructure);


}


void i2c_start1(void)
{	
	//保证SDA引脚为输出模式
	sda_pin_mode(GPIO_Mode_OUT);
	
	SDA_W=1;
	SCL_W=1;
	delay_us(1);
	
	SDA_W=0;
	delay_us(1);
	
	SCL_W=0;//总线进入忙状态
	delay_us(1);
}


void i2c_stop1(void)
{
	//保证SDA引脚为输出模式
	sda_pin_mode(GPIO_Mode_OUT);
	
	SDA_W=0;
	SCL_W=1;
	delay_us(1);
	
	SDA_W=1;
	delay_us(1);

}


void i2c_send_byte(uint8_t byte)
{
	int32_t i=0;
	
	//保证SDA引脚为输出模式
	sda_pin_mode(GPIO_Mode_OUT);

	SDA_W=0;
	SCL_W=0;
	delay_us(1);
	
	//最高有效位优先传输，通过时序图观察到
	for(i=7; i>=0; i--)
	{
		//检测对应的bit位是1还是0
		if(byte & (1<<i))
			SDA_W=1;
		else
			SDA_W=0;
	
		delay_us(1);
	
		//时钟线拉高，数据有效
		SCL_W=1;
		delay_us(1);

	
		//时钟线拉低，数据变更
		SCL_W=0;
		delay_us(1);		
	
	}
}


uint8_t i2c_recv_byte(void)
{
	uint8_t d=0;
	int32_t i;
	
	//保证SDA引脚为输入模式
	sda_pin_mode(GPIO_Mode_IN);

	for(i=7; i>=0; i--)
	{
		//时钟线拉高，数据有效
		SCL_W=1;
		delay_us(1);
		
		//读取SDA引脚电平
		if(SDA_R)
			d|=1<<i;


		//时钟线拉低，保持占用总线，总线是忙状态
		SCL_W=0;
		delay_us(1);	
	
	}

	return d;
}


void i2c_ack(uint8_t ack)
{

	//保证SDA引脚为输出模式
	sda_pin_mode(GPIO_Mode_OUT);

	SDA_W=0;
	SCL_W=0;
	delay_us(1);
	

	if(ack)
		SDA_W=1;
	else
		SDA_W=0;

	delay_us(1);

	//时钟线拉高，数据有效
	SCL_W=1;
	delay_us(1);


	//时钟线拉低，数据变更
	SCL_W=0;
	delay_us(1);		
}

uint8_t i2c_wait_ack1(void)
{
	uint8_t ack=0;
	//保证SDA引脚为输入模式
	sda_pin_mode(GPIO_Mode_IN);

	//时钟线拉高，数据有效
	SCL_W=1;
	delay_us(1);
	
	//读取SDA引脚电平
	if(SDA_R)
		ack=1;//无应答
	else
		ack=0;//有应答

	//时钟线拉低，保持占用总线，总线是忙状态
	SCL_W=0;
	delay_us(1);


	return ack;
}

void IIC_WriteBytes(u8 WriteAddr,u8* data,u8 dataLength)
{		
	u8 i;	
    i2c_start1();  

	i2c_send_byte(WriteAddr);	    //发送写命令
	i2c_wait_ack1();
	
	for(i=0;i<dataLength;i++)
	{
		i2c_send_byte(data[i]);
		i2c_wait_ack1();
	}				    	   
    i2c_stop1();//产生一个停止条件 
	delay_ms(10);	 
}

void IIC_ReadBytes(u8 deviceAddr, u8 writeAddr,u8* data,u8 dataLength)
{		
	u8 i;	
    i2c_start1();  

	i2c_send_byte(deviceAddr);	    //发送写命令
	i2c_wait_ack1();
	i2c_send_byte(writeAddr);
	i2c_wait_ack1();
	i2c_send_byte(deviceAddr|0X01);//进入接收模式			   
	i2c_wait_ack1();
	
	for(i=0;i<dataLength-1;i++)
	{
		data[i] = i2c_recv_byte();
		i2c_ack(0);
	}		
	data[dataLength-1] = i2c_recv_byte();	
	i2c_ack(1);
    i2c_stop1();//产生一个停止条件 
	delay_ms(10);	 
}

void IIC_Read_One_Byte(u8 daddr,u8 addr,u8* data)
{				  	  	    																 
    i2c_start1();  
	
	i2c_send_byte(daddr);	   //发送写命令
	i2c_wait_ack1();
	i2c_send_byte(addr);//发送地址
	i2c_wait_ack1();		 
	i2c_start1();  	 	   
	i2c_send_byte(daddr|0X01);//进入接收模式			   
	i2c_wait_ack1();	 
    *data = i2c_recv_byte();
	i2c_ack(1);
    i2c_stop1();//产生一个停止条件	    
}

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data)
{				   	  	    																 
    i2c_start1();  
	
	i2c_send_byte(daddr);	    //发送写命令
	i2c_wait_ack1();
	i2c_send_byte(addr);//发送地址
	i2c_wait_ack1();	   	 										  		   
	i2c_send_byte(data);     //发送字节							   
	i2c_wait_ack1();  		    	   
    i2c_stop1();//产生一个停止条件 
	delay_ms(10);	 
}
