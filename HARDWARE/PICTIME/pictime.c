#include "includes.h"

void lcd_draw_hour(uint8_t hour)
{
	uint8_t Onewei;
	uint8_t Twowei;
	
	Onewei = hour/10;
	Twowei = hour%10;
	
	if(Onewei == 0)
	{
		lcd_draw_picture(0,60,60,60,gImage_0);
	}
	else if(Onewei == 1)
	{
		lcd_draw_picture(0,60,60,60,gImage_1);
	}
	else if(Onewei == 2)
	{
		lcd_draw_picture(0,60,60,60,gImage_2);
	}
	
	switch (Twowei) {
    case 0:
        lcd_draw_picture(55,60,60,60,gImage_0);
        break;
    case 1:
        lcd_draw_picture(55,60,60,60,gImage_1);
        break;
    case 2:
        lcd_draw_picture(55,60,60,60,gImage_2);
        break;
	case 3:
        lcd_draw_picture(55,60,60,60,gImage_3);
        break;
	case 4:
        lcd_draw_picture(55,60,60,60,gImage_4);
        break;
	case 5:
        lcd_draw_picture(55,60,60,60,gImage_5);
        break;
	case 6:
        lcd_draw_picture(55,60,60,60,gImage_6);
        break;
	case 7:
        lcd_draw_picture(55,60,60,60,gImage_7);
        break;
	case 8:
        lcd_draw_picture(55,60,60,60,gImage_8);
        break;
	case 9:
        lcd_draw_picture(55,60,60,60,gImage_9);
        break;
	}
 
}

void lcd_draw_minutes(uint8_t min)
{
	uint8_t Onewei;
	uint8_t Twowei;
	
	Onewei = min/10;
	Twowei = min%10;
	
	if(Onewei == 0)
	{
		lcd_draw_picture(130,60,60,60,gImage_0);
	}
	else if(Onewei == 1)
	{
		lcd_draw_picture(130,60,60,60,gImage_1);
	}
	else if(Onewei == 2)
	{
		lcd_draw_picture(130,60,60,60,gImage_2);
	}
	else if(Onewei == 3)
	{
		lcd_draw_picture(130,60,60,60,gImage_3);
	}
	else if(Onewei == 4)
	{
		lcd_draw_picture(130,60,60,60,gImage_4);
	}
	else if(Onewei == 5)
	{
		lcd_draw_picture(130,60,60,60,gImage_5);
	}
	
	
	switch (Twowei) {
    case 0:
        lcd_draw_picture(180,60,60,60,gImage_0);
        break;
    case 1:
        lcd_draw_picture(180,60,60,60,gImage_1);
        break;
    case 2:
        lcd_draw_picture(180,60,60,60,gImage_2);
        break;
	case 3:
        lcd_draw_picture(180,60,60,60,gImage_3);
        break;
	case 4:
        lcd_draw_picture(180,60,60,60,gImage_4);
        break;
	case 5:
        lcd_draw_picture(180,60,60,60,gImage_5);
        break;
	case 6:
        lcd_draw_picture(180,60,60,60,gImage_6);
        break;
	case 7:
        lcd_draw_picture(180,60,60,60,gImage_7);
        break;
	case 8:
        lcd_draw_picture(180,60,60,60,gImage_8);
        break;
	case 9:
        lcd_draw_picture(180,60,60,60,gImage_9);
        break;
	}
 
}


void lcd_draw_seconds(uint8_t sec, uint16_t x_sec, uint16_t y_sec)
{
    uint8_t Onewei;
    uint8_t Twowei;
    
    Onewei = sec / 10;  // 获取秒数的十位
    Twowei = sec % 10;  // 获取秒数的个位
    
    // 显示十位数
    switch (Onewei) {
        case 0:
            lcd_draw_picture(x_sec, y_sec, 60, 60, gImage_0);
            break;
        case 1:
            lcd_draw_picture(x_sec, y_sec, 60, 60, gImage_1);
            break;
        case 2:
            lcd_draw_picture(x_sec, y_sec, 60, 60, gImage_2);
            break;
        case 3:
            lcd_draw_picture(x_sec, y_sec, 60, 60, gImage_3);
            break;
        case 4:
            lcd_draw_picture(x_sec, y_sec, 60, 60, gImage_4);
            break;
        case 5:
            lcd_draw_picture(x_sec, y_sec, 60, 60, gImage_5);
            break;
        default:
            break;
    }
    
    // 显示个位数
    switch (Twowei) {
        case 0:
            lcd_draw_picture(x_sec + 55, y_sec, 60, 60, gImage_0);
            break;
        case 1:
            lcd_draw_picture(x_sec + 55, y_sec, 60, 60, gImage_1);
            break;
        case 2:
            lcd_draw_picture(x_sec + 55, y_sec, 60, 60, gImage_2);
            break;
        case 3:
            lcd_draw_picture(x_sec + 55, y_sec, 60, 60, gImage_3);
            break;
        case 4:
            lcd_draw_picture(x_sec + 55, y_sec, 60, 60, gImage_4);
            break;
        case 5:
            lcd_draw_picture(x_sec + 55, y_sec, 60, 60, gImage_5);
            break;
        case 6:
            lcd_draw_picture(x_sec + 55, y_sec, 60, 60, gImage_6);
            break;
        case 7:
            lcd_draw_picture(x_sec + 55, y_sec, 60, 60, gImage_7);
            break;
        case 8:
            lcd_draw_picture(x_sec + 55, y_sec, 60, 60, gImage_8);
            break;
        case 9:
            lcd_draw_picture(x_sec + 55, y_sec, 60, 60, gImage_9);
            break;
        default:
            break;
    }
}
