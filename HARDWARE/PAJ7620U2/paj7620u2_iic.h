#ifndef __PAJ7620U2_IIC_H
#define __PAJ7620U2_IIC_H
#include "stm32f4xx.h"
#include "sys.h"


#define GS_SDA_IN()  {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=0<<9*2;}	//PB9输入模式
#define GS_SDA_OUT() {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=1<<9*2;}  //PB9输出模式

//IO操作函数	

#define GS_IIC_SCL    PBout(8) 		//SCL
#define GS_IIC_SDA    PBout(9) 		//SDA	 
#define GS_READ_SDA   PBin(9) 		//输入SDA 


uint8_t GS_Write_Byte(uint8_t REG_Address,uint8_t REG_data);
uint8_t GS_Read_Byte(uint8_t REG_Address);
uint8_t GS_Read_nByte(uint8_t REG_Address,uint16_t len,uint8_t *buf);
void GS_i2c_init(void);
void GS_WakeUp(void);

#endif


