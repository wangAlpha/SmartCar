#pragma once

#define H (312) //图像宽度
#define V (64)  //图像高度
#define PicSize H*V

#define Width (256) //实际图像宽度
#define Hight (64)

extern uint8_t Data_Buffer[V][H];
extern uint8_t Camera_Image[Hight][Width];
extern bool Is_DispPhoto;

extern int16_t L_Line_0[8];
extern int16_t R_Line_0[8];
extern int16_t L_Line_1[8];
extern int16_t R_Line_1[8];
extern int16_t Center_Line_0[8];
extern int16_t Center_Line_1[8];
extern int16_t Center_Line;

extern int16_t Speed_Buffer;

extern uint8_t Track_Type;

extern uint8_t Track_Type;
extern uint8_t Straight_Flag;
extern uint8_t Obstacle_Flag;
extern uint8_t Obstacle_Counter;
extern uint8_t Rampway_Counter;
extern uint8_t Circle_Control;
extern uint8_t Circle_Flag;

extern const uint16_t Point_0; // 主线
extern const uint16_t Point_1; // 辅助线

void Field_IRQ(void);
void Row_IRQ(void);
void Ostu(void);
void Image_Binary(void);
void Image_Analysis(void);
