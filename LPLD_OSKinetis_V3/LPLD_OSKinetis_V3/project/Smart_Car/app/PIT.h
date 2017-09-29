#pragma once

extern uint8_t LCD_Counter;
extern int16_t Set_Speed;
extern int32_t Get_Speed;
extern uint16_t Start_Count;
extern bool Is_Run;

extern uint8_t Stop_Flag;
extern uint8_t Start_Flag;

extern float Angle;
extern float Acc;
extern float Gyro;

void PIT_IRQ(void);
