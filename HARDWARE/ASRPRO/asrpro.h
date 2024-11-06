#ifndef __ASRPRO_H__
#define __ASRPRO_H__
#include "includes.h"
extern volatile uint8_t  g_usart2_rx_buf[32];
extern volatile uint32_t g_usart2_rx_cnt;
extern volatile uint32_t g_usart2_rx_end;

extern void asr_init(uint32_t baud);
extern void asr_send_str(char * str);

extern void usart2_init(uint32_t baud);
extern void usart_send_str(USART_TypeDef* USARTx,char *str);
extern void usart_send_bytes(USART_TypeDef* USARTx,uint8_t *buf,uint32_t len);

#endif
