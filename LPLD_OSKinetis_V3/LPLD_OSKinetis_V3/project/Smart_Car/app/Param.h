#pragma once

extern bool Is_Run;

// 轻触开关
#define Key_1 PTB23_I
#define Key_2 PTB22_I
#define Key_3 PTB21_I
#define Key_4 PTB20_I
// 拨动开关
#define Switch_1 PTC0_I
#define Switch_2 PTC1_I
#define Switch_3 PTC2_I
#define Switch_4 PTC5_I

void Param_Init(void);
void Param_Save(void);
void Run_Light(void);
void Key_Update(void);
void Send_Data(void);
void SendImage(void);
void Display(void);
