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

//����TFT320 ���ݶ˿� ( DB0~DB15  ) ----------------------------------------PORTC
#define TFT320_DATA16(x)         PTC_W0_OUT(x)//дDB0~DB15����

#define TFT320_RS(x)    LPLD_GPIO_Output_b(PTB,23,x)            //дRS����
#define TFT320_RW(x)    LPLD_GPIO_Output_b(PTB,22,x)            //дRW����
#define TFT320_RD(x)    LPLD_GPIO_Output_b(PTB,21,x)        //дCS����
#define TFT320_RES(x)   LPLD_GPIO_Output_b(PTB,19,x)            //дRES����
#define TFT320_CS(x)    LPLD_GPIO_Output_b(PTB,20,x)            //дCS����


//���ú���
void PTC_W0_OUT(uint16_t x);

/****           TFT320��ʼ��                      *****/
void TFT320_INIT(void);

/***********            ����       bColor ָ��������ɫ              **************/
void TFT320_ClearScreen(uint16_t bColor);


/***************   д8*16�ַ���   ��   16*16 ����                           ******************/
void TFT320_Write_String(uint16_t x,  uint16_t y,  uint8_t *s, uint16_t fColor,  uint16_t bColor);

void TFT320_Write_Data(uint16_t dat);

void TFT320_Write_Cmd(uint16_t dat);

/**********           ��ĳ�� x �Ĵ�������ַ��     д ����                **********/
void TFT320_Init_data(uint16_t x,  uint16_t y);

void TFT320_LCD_SetPos(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1);

void TFT320_write_line(uint16_t x, uint16_t y, uint16_t xx, uint16_t yy, uint16_t fColor);
void TFT320_Point_Chart(uint16_t x, uint16_t y, uint16_t xx, uint16_t yy, int *data , uint16_t fColor);

#endif
