/*作    者:江秋荣*/
#ifndef __TFT_H__
#define __TFT_H__

#include "lcd_font.h"

#define LCD_WIDTH 240
#define LCD_HEIGHT 240

#define LCD_SOFT_SPI_ENABLE 0

#define RED 0XF800    // 红色
#define GREEN 0X07E0  // 绿色
#define BLUE 0X001F   // 蓝色
#define WHITE 0XFFFF  // 白色
#define BLACK 0X0000  // 黑色
#define YELLOW 0xFFE0 // 黄色
#define GREY 0x7BEF   // 灰色

#if LCD_SOFT_SPI_ENABLE
#define SPI_SCK_0 PBout(15) = 0
#define SPI_SCK_1 PBout(15) = 1

#define SPI_SDA_0 PDout(10) = 0
#define SPI_SDA_1 PDout(10) = 1

#define LCD_RST_0 PDout(8) = 0
#define LCD_RST_1 PDout(8) = 1

#define LCD_DC_0 PEout(14) = 0
#define LCD_DC_1 PEout(14) = 1

#define LCD_BLK_0 PEout(12) = 0
#define LCD_BLK_1 PEout(12) = 1

#else

#define SPI_SCK_0 PBout(3) = 0
#define SPI_SCK_1 PBout(3) = 1

#define SPI_SDA_0 PBout(5) = 0
#define SPI_SDA_1 PBout(5) = 1

#define LCD_RST_0 PGout(8) = 0
#define LCD_RST_1 PGout(8) = 1

#define LCD_DC_0 PGout(7) = 0
#define LCD_DC_1 PGout(7) = 1

#define LCD_BLK_0 PBout(4) = 0
#define LCD_BLK_1 PBout(4) = 1

#endif
extern void lcd_vs_set(uint16_t vs);
extern void lcd_idle(uint16_t on);
extern void lcd_display_on(uint16_t on);
extern void lcd_init(void);
extern void lcd_addr_set(uint32_t x_s, uint32_t y_s, uint32_t x_e, uint32_t y_e);
extern void lcd_clear(uint32_t color);
extern void lcd_fill(uint32_t x_s, uint32_t y_s, uint32_t x_len, uint32_t y_len, uint32_t color);
extern void lcd_send_cmd(uint8_t cmd);
extern void lcd_send_data(uint8_t dat);
extern void lcd_draw_point(uint32_t x, uint32_t y, uint32_t color);
extern void lcd_draw_picture(uint32_t x_s, uint32_t y_s, uint32_t width, uint32_t height, const uint8_t *pic);
extern void lcd_show_chn(uint32_t x, uint32_t y, uint8_t no, uint32_t fc, uint32_t bc, uint32_t font_size);
extern void lcd_show_string(u16 x, u16 y, const char *p, u16 fc, u16 bc, u8 font_size, u8 mode);
extern void lcd_show_integer(uint32_t x, uint32_t y, uint32_t num, uint32_t len, uint32_t fc, uint32_t bc, uint32_t font_size);
extern void lcd_show_float(uint32_t x, uint32_t y, float num, uint32_t len, uint32_t fc, uint32_t bc, uint32_t font_size);
extern void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
extern void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
extern void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
#endif
