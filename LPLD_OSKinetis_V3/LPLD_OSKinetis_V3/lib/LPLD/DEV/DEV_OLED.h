#pragma once

void LCD_Init(void);
void LCD_Clear(void);
void LCD_Write_English_String(uint8_t x, uint8_t y, uint8_t ch[]);
void LCD_P8x16Str(uint8_t x, uint8_t y, uint8_t ch[]);
void LCD_Print(uint8_t x, uint8_t y, uint8_t ch[]);
void LCD_PutPixel(uint8_t x, uint8_t y);
void LCD_Rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t gif);
void Draw_BMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t bmp[]);
void LCD_Fill(uint8_t dat);

void LCD_Set_Pos(uint8_t x, uint8_t y);
void LCD_WrCmd(uint8_t cmd);
void LCD_WrDat(uint8_t data);
