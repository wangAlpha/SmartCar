#include "common.h"
#include "HW_GPIO.h"
#include "tft320_9341.h"
#include "8X16.H" //包含8*16字符

#define time_delay_ms(x) delay(x)
#define time_delay_us(x) delay_us(x)


#define LCD_WR_REG(x) TFT320_Write_Cmd(x)
#define LCD_WR_DATA(x) TFT320_Write_Data(x)


// ------------------  ASCII字模的数据表 ------------------------ //
// 码表从0x20~0x7e                                                //
// 字库: C:\Users\tang\Desktop\tft320_9341\tft320_9341\lcd汉字取模软件\lcd汉字取模软件\Asc12x24E.dat 横向取模左高位//
// -------------------------------------------------------------- //

void delay(uint32_t i)
{
    uint32_t j = 48000, k = 1000 * i;
    for (; j > 0; j--)
        for (; k > 0; k--);
}

void delay_us(uint32_t i)
{
    uint32_t j = 48, k = 1000 * i;
    for (; j > 0; j--)
        for (; k > 0; k--);
}

__INLINE void PTC_W0_OUT(uint16_t x)
{
    PTC->PDOR &= (0xFFFF0000);
    PTC->PDOR |= x;
}

/****           TFT320初始化                      *****/
void TFT320_INIT(void)
{
    for (uint32_t i = 0; i < 16; i++) { //端口初始化
        LPLD_GPIO_Output_b(PTC, i, 1);
    }
    for (uint32_t i = 0; i < 5; i++) {
        LPLD_GPIO_Output_b(PTB, 19 + i, 1);
    }
    TFT320_CS(1);
    TFT320_RD(1);
    TFT320_RW(1);
    time_delay_ms(5);
    TFT320_RES(0);
    time_delay_ms(15);
    TFT320_RES(1);
    time_delay_ms(100);
    //************* Start Initial Sequence **********//
    LCD_WR_REG(0xcf);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xc1);
    LCD_WR_DATA(0x30);
    LCD_WR_REG(0xed);
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x12);
    LCD_WR_DATA(0x81);
    LCD_WR_REG(0xcb);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x2c);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x02);
    LCD_WR_REG(0xea);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0xe8);
    LCD_WR_DATA(0x85);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x79);
    LCD_WR_REG(0xC0); //Power control
    LCD_WR_DATA(0x23); //VRH[5:0]
    LCD_WR_REG(0xC1); //Power control
    LCD_WR_DATA(0x11); //SAP[2:0];BT[3:0]
    LCD_WR_REG(0xC2);
    LCD_WR_DATA(0x11);
    LCD_WR_REG(0xC5); //VCM control
    LCD_WR_DATA(0x3d);
    LCD_WR_DATA(0x30);
    LCD_WR_REG(0xc7);
    LCD_WR_DATA(0xaa);
    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);
    LCD_WR_REG(0x36); // Memory Access Control
    LCD_WR_DATA(0x08);
    LCD_WR_REG(0xB1); // Frame Rate Control
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x11);
    LCD_WR_REG(0xB6); // Display Function Control
    LCD_WR_DATA(0x0a);
    LCD_WR_DATA(0xa2);
    LCD_WR_REG(0xF2); // 3Gamma Function Disable
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0xF7);
    LCD_WR_DATA(0x20);
    LCD_WR_REG(0xF1);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x30);
    LCD_WR_REG(0x26); //Gamma curve selected
    LCD_WR_DATA(0x01);
    LCD_WR_REG(0xE0); //Set Gamma
    LCD_WR_DATA(0x0f);
    LCD_WR_DATA(0x3f);
    LCD_WR_DATA(0x2f);
    LCD_WR_DATA(0x0c);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x0a);
    LCD_WR_DATA(0x53);
    LCD_WR_DATA(0xd5);
    LCD_WR_DATA(0x40);
    LCD_WR_DATA(0x0a);
    LCD_WR_DATA(0x13);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0xE1); //Set Gamma
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x0f);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x2c);
    LCD_WR_DATA(0xa2);
    LCD_WR_DATA(0x3f);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(0x0e);
    LCD_WR_DATA(0x0c);
    LCD_WR_DATA(0x37);
    LCD_WR_DATA(0x3c);
    LCD_WR_DATA(0x0F);
    LCD_WR_REG(0x11); //Exit Sleep
    time_delay_ms(100);//此处必须要延时80ms或者更多
    LCD_WR_REG(0x29); //
}

/********            写数据/命令   cmd=0,写命令，  cmd=1   写数据              ******** */
__INLINE void TFT320_Write_Cmd(uint16_t dat)
{
    TFT320_CS(0);
    TFT320_RS(0);
    TFT320_DATA16(dat);
    TFT320_RW(0);
    TFT320_RW(1);
    TFT320_CS(1);
}

__INLINE void TFT320_Write_Data(uint16_t dat)
{
    TFT320_CS(0);
    TFT320_RS(1);
    TFT320_DATA16(dat);
    TFT320_RW(0);
    TFT320_RW(1);
    TFT320_CS(1);
}
/**********           对某个 x 寄存器（地址）     写 数据                **********/
void TFT320_Init_data(uint16_t x,  uint16_t y)
{
    TFT320_Write_Cmd(x); //写命令
    TFT320_Write_Data(y); //写数据
}

/*************************定义显示窗体*******************************
     x0:  窗体中X坐标中较小者
     x1:  窗体中X坐标中较大者
     y0:  窗体中Y坐标中较小者
     y1:  窗体中Y坐标中较大者
**********************************************************************************/
void TFT320_LCD_SetPos(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1)
{
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(x0 >> 8);
    LCD_WR_DATA(x0);
    LCD_WR_DATA(x1 >> 8);
    LCD_WR_DATA(x1);
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(y0 >> 8);
    LCD_WR_DATA(y0);
    LCD_WR_DATA(y1 >> 8);
    LCD_WR_DATA(y1);
    LCD_WR_REG(0x2c);
}

/***********            清屏       bColor 指清屏的颜色              **************/
void TFT320_ClearScreen(uint16_t bColor)
{
    TFT320_LCD_SetPos(0, 239, 0, 319);
    for(int32_t i = 320 * 240 -1 ; i > 0; --i){
        TFT320_Write_Data(bColor);
    }
}

/**********      写8*16字符                          ***********/
void TFT320_Write_8x16(uint16_t x,  uint16_t y,  uint8_t c, uint16_t fColor,  uint16_t bColor)
{
    uint16_t m;
    TFT320_LCD_SetPos(x, x + 8 - 1, y, y + 16 - 1);
    for (int32_t i = 0; i < 16; i++) {
        m = Font8x16[c * 16 + i];
        for (int32_t j = 0; j < 8; j++) {
            if ((m & 0x80) == 0x80)  //如果最高位是1
                TFT320_Write_Data(fColor);
            else
                TFT320_Write_Data(bColor);
            m <<= 1;
        }
    }
}


/***************   写8*16字符串   或   16*16 汉字                           ******************/
void TFT320_Write_String(uint16_t x,  uint16_t y,  uint8_t *s, uint16_t fColor,  uint16_t bColor)
{
    uint32_t l = 0;
    while (*s) {
        TFT320_Write_8x16(x + l * 8 ,   y,  *s,  fColor,   bColor);
        s++;
        l++;//位置++
    }
}


void TFT320_write_line(uint16_t x, uint16_t y, uint16_t xx, uint16_t yy, uint16_t fColor)
{
    uint32_t j = 0;
    TFT320_LCD_SetPos(x, x + xx - 1, y, y + yy - 1);
    for (j = 0; j < (xx * yy); j++) {
        TFT320_Write_Data(fColor); //写数据
    }
}


/**画点图  */
//输入 ad 采样数组就好
void TFT320_Point_Chart(uint16_t x, uint16_t y, uint16_t xx, uint16_t yy, int *data , uint16_t fColor)
{
    int16_t j = 0, temp1 = 0, temp2 = 4096;
    for (j = 0; j < xx; j++) {
        if ((*data) >= temp1) {  //取最大值
            temp1 = *data;
        }
        else if ((*data) < temp2) {
            temp2 = *data;
        }
        data++;
    }
    data -= xx;
    float y_width = ((float)(yy) / (float)temp1);  //      //最小值        默认最小值是 0
    TFT320_write_line(x , y , 1 , yy , White);
    TFT320_write_line(x , (yy + y) - (uint16_t)(((temp2 + temp1) / 2)*y_width) , xx , 1 , White);
    for (j = 1; j < xx; j++) {
        TFT320_write_line(x + j , (yy + y) - (uint16_t)((*data)*y_width) , 1 , 1 , fColor);
        data++;
    }
    TFT320_write_line(x - 1 , y + 1 , 3 , 1 , White);
    TFT320_write_line(x + xx - 2 , y + yy - 1 , 1 , 3 , White);
}
