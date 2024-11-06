#ifndef __I2C_H__
#define __I2C_H__

#define SCL_W	PBout(8)
#define SDA_W	PBout(9)
#define SDA_R	PBin(9)


uint8_t i2c_recv_byte(void);
void i2c_send_byte(uint8_t byte);
extern void i2c_stop1(void);
extern void i2c_start1(void);
void i2c_init(void);
extern uint8_t i2c_wait_ack1(void);
void i2c_ack(uint8_t ack);


#endif
