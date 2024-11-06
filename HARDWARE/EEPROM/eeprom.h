#ifndef __EEPROM_H__
#define __EEPROM_H__

#define EEPROM_SCL_W	PBout(8)
#define EEPROM_SDA_W	PBout(9)
#define EEPROM_SDA_R	PBin(9)

extern void     at24c02_init(void);
extern int32_t  at24c02_write(uint8_t addr,uint8_t *buf,uint8_t len);
extern int32_t  at24c02_read(uint8_t addr,uint8_t *buf,uint8_t len);

#endif
