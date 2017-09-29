#pragma once

// PD²ÎÊý
extern int16_t A_Kp;
extern int16_t A_Kd;

void Read_AccGyro(void);
float Kalman_Filter(float, float, float);
int16_t Balance_Control(float, float);
uint32_t Micros(void);
void Get_Offset(void);
