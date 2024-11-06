#include "includes.h"


void at24c02_init(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;		//第8 9根引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;				//设置输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;				//开漏模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//不需要上拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//只要是输出模式，有初始电平状态
	EEPROM_SCL_W=1;
	EEPROM_SDA_W=1;
}

static void sda_pin_mode(GPIOMode_TypeDef pin_mode)
{
	GPIO_InitTypeDef   GPIO_InitStructure;	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		//第9根引脚
	GPIO_InitStructure.GPIO_Mode = pin_mode;	//设置输出/输入模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	//开漏模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//不需要上拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}
static void i2c_start(void)
{
	//保证SDA引脚为输出模式
	sda_pin_mode(GPIO_Mode_OUT);
	
	EEPROM_SCL_W=1;
	EEPROM_SDA_W=1;
	delay_us(5);
	
	EEPROM_SDA_W=0;
	delay_us(5);

	EEPROM_SCL_W=0;
	delay_us(5);
}


static void i2c_stop(void)
{
	//保证SDA引脚为输出模式
	sda_pin_mode(GPIO_Mode_OUT);
	
	EEPROM_SCL_W=1;
	EEPROM_SDA_W=0;
	delay_us(5);
	
	EEPROM_SDA_W=1;
	delay_us(5);
}

static void i2c_send_byte(uint8_t byte)
{
	int32_t i;
	
	//保证SDA引脚为输出模式
	sda_pin_mode(GPIO_Mode_OUT);
	
	EEPROM_SCL_W=0;
	EEPROM_SDA_W=0;
	delay_us(5);
	
	for(i=7; i>=0; i--)
	{
		if(byte & (1<<i))
			EEPROM_SDA_W=1;
		else
			EEPROM_SDA_W=0;
	
		delay_us(5);
	
		EEPROM_SCL_W=1;
		delay_us(5);
		
		EEPROM_SCL_W=0;
		delay_us(5);		
	}
}

static void i2c_ack(uint8_t ack)
{
	//保证SDA引脚为输出模式
	sda_pin_mode(GPIO_Mode_OUT);
	
	EEPROM_SCL_W=0;
	EEPROM_SDA_W=0;
	delay_us(5);
	
	if(ack)
		EEPROM_SDA_W=1;
	else
		EEPROM_SDA_W=0;

	delay_us(5);

	EEPROM_SCL_W=1;
	delay_us(5);
	
	EEPROM_SCL_W=0;
	delay_us(5);		
}

static uint8_t i2c_wait_ack(void)
{
	uint8_t ack;
	//保证SDA引脚为输入模式
	sda_pin_mode(GPIO_Mode_IN);

	//紧接着第九个时钟周期，将SCL拉高
	EEPROM_SCL_W=1;
	delay_us(5);
	
	if(EEPROM_SDA_R)
		ack=1;
	else
		ack=0;
	
	//继续保持占用总线
	EEPROM_SCL_W=0;
	delay_us(5);

	return ack;
}

static uint8_t i2c_recv_byte(void)
{

	uint8_t d=0;
	int32_t i;
	
	
	//保证SDA引脚为输入模式
	sda_pin_mode(GPIO_Mode_IN);

	
	for(i=7; i>=0; i--)
	{
		EEPROM_SCL_W=1;
		delay_us(5);
		
		if(EEPROM_SDA_R)
			d|=1<<i;
		
		//继续保持占用总线
		EEPROM_SCL_W=0;
		delay_us(5);	
	}

	return d;
}


int32_t  at24c02_write(uint8_t addr,uint8_t *buf,uint8_t len)
{
	uint8_t ack;
	uint8_t *p=buf;
	
	//发送起始信号
	i2c_start();
	
	//进行设备寻址，写访问地址为0xA0
	i2c_send_byte(0xA0);
	
	ack=i2c_wait_ack();
	if(ack)
	{
		printf("device address error\r\n");
		return -1;
	}

	//发送要放问eeprom数据的起始地址
	i2c_send_byte(addr);
	
	ack=i2c_wait_ack();
	if(ack)
	{
		printf("word address error\r\n");
		return -2;
	}
	
	//连续发送数据
	while(len--)
	{
		i2c_send_byte(*p++);
		
		ack=i2c_wait_ack();
		if(ack)
		{
			printf("write data error\r\n");
			return -3;
		}	
	}
	
	i2c_stop();
	printf("write data success\r\n");
	return 0;

}


int32_t  at24c02_read(uint8_t addr,uint8_t *buf,uint8_t len)
{
	uint8_t ack;
	uint8_t *p=buf;
	
	//发送起始信号
	i2c_start();
	
	//进行设备寻址，写访问地址为0xA0
	i2c_send_byte(0xA0);
	
	ack=i2c_wait_ack();
	if(ack)
	{
		printf("device address error\r\n");
		return -1;
	}

	//发送要访问eeprom数据的起始地址
	i2c_send_byte(addr);
	
	ack=i2c_wait_ack();
	if(ack)
	{
		printf("word address error\r\n");
		return -2;
	}
	
	//发送起始信号，用于切换访问方向，从写访问切换读访问
	i2c_start();
	
	//进行设备寻址，写访问地址为0xA1
	i2c_send_byte(0xA1);
	ack=i2c_wait_ack();
	if(ack)
	{
		printf("device address error\r\n");
		return -3;
	}	
	
	
	//连续读取数据
	len=len-1;
	
	while(len--)
	{
		//读取数据
		*p=i2c_recv_byte();
		
		//发送应答信号
		i2c_ack(0);
		
		p++;
	}
	
	//读取最后一个字节
	*p=i2c_recv_byte();
	//发送无应答信号
	i2c_ack(1);
	
	i2c_stop();

	return 0;
}

