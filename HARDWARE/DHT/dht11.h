#ifndef __DHT11_H__
#define __DHT11_H__


extern void     dht11_init(void);
extern int32_t  dht11_read_data(uint8_t *pbuf);
extern void     dht11_pin_mode(GPIOMode_TypeDef pin_mode);



#endif
