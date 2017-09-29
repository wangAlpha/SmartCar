#pragma once

extern int16_t S_Ki;
extern int16_t S_Kp;
extern int16_t S_Kd;
extern int32_t S_Inter;
extern int16_t Old_Speed_Pwm;
extern int16_t Speed_Hightest;
extern int16_t Speed_Lowest;
extern int16_t Speed_A;
extern int16_t Speed_Straight;

int16_t Get_Pulse(void);
int16_t Set_Pulse();
int16_t Speed_Control(int16_t , int32_t);
void Motor_Out(int16_t , int16_t);
