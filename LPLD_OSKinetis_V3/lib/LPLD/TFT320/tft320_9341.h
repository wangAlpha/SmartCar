#ifndef __TFT320_9341_H
#define __TFT320_9341_H

#include "HW_GPIO.h"

/* LCD color */
#define White          0xffff       //RGB 565.
#define Black          0x0000
#define Blue           0x001F
#define Blue2          0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0

//定义TFT320 数据端口 ( DB0~DB15  ) ----------------------------------------PORTC
#define TFT320_DATA16(x)         PTC_W0_OUT(x)//写DB0~DB15数据

#define TFT320_RS(x)    LPLD_GPIO_Output_b(PTB,23,x)            //写RS数据
#define TFT320_RW(x)    LPLD_GPIO_Output_b(PTB,22,x)            //写RW数据
#define TFT320_RD(x)    LPLD_GPIO_Output_b(PTB,21,x)        //写CS数据
#define TFT320_RES(x)   LPLD_GPIO_Output_b(PTB,19,x)            //写RES数据
#define TFT320_CS(x)    LPLD_GPIO_Output_b(PTB,20,x)            //写CS数据


//调用函数
void PTC_W0_OUT(uint16_t x);

/****           TFT320初始化                      *****/
void TFT320_INIT(void);

/***********            清屏       bColor 指清屏的颜色              **************/
void TFT320_ClearScreen(uint16_t bColor);


/***************   写8*16字符串   或   16*16 汉字                           ******************/
void TFT320_Write_String(uint16_t x,  uint16_t y,  uint8_t *s, uint16_t fColor,  uint16_t bColor);

void TFT320_Write_Data(uint16_t dat);

void TFT320_Write_Cmd(uint16_t dat);

/**********           对某个 x 寄存器（地址）     写 数据                **********/
void TFT320_Init_data(uint16_t x,  uint16_t y);

void TFT320_LCD_SetPos(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1);

void TFT320_write_line(uint16_t x, uint16_t y, uint16_t xx, uint16_t yy, uint16_t fColor);
void TFT320_Point_Chart(uint16_t x, uint16_t y, uint16_t xx, uint16_t yy, int *data , uint16_t fColor);

#endif
