#ifndef __PICTIME_H__
#define __PICTIME_H__

extern void lcd_draw_hour(uint8_t hour);
extern void lcd_draw_minutes(uint8_t min);
extern void lcd_draw_seconds(uint8_t sec, uint16_t x_sec, uint16_t y_sec);
#endif
