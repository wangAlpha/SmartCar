/**
 *-------------基于"拉普兰德K60底层库V3.1"的智能车工程-----------------
 * @file PIT.c
 * @version 0.0
 * @date 2017-04-26
 * @brief 定时中断，实现任务实时调度
 *
 * 开发者 湖南师范大学 飞思卡尔光电直立组 音速战神
 * 硬件平台 MK60DN512VLL10
 * @par None
**/
#include "common.h"
#include "PIT.h"
#include "Balance.h"
#include "Speed.h"
#include "Direction.h"
#include "Camera.h"
#include "Param.h"

//---------------------------------------------------
int32_t Get_Speed = 0;
int16_t Set_Speed = 0;
// ----------------设定速度--------------------------
int16_t Speed_Pwm = 0; //速度
int16_t Old_Speed_Pwm = 0;
int16_t Angle_Pwm = 0;
int16_t Left_Pwm = 0;  //左轮输出速度
int16_t Right_Pwm = 0; //右轮输出速度
int16_t Direction_Pwm = 0; //方向控制
int16_t Old_Direction_Pwm = 0;
bool Is_Run = FALSE;   // 电机是否运行标志
// -------------------计数器-------------------------
uint8_t Timer_1ms = 0;
uint8_t LCD_Counter = 0;   //LCD刷新计数器
uint8_t Direction_Count = 0;
uint8_t CCD_Count = 0;
uint8_t Speed_Count = 0;
uint8_t Pulse_Count = 0;
uint8_t Start_Flag = 0; // 发车标志
uint8_t Stop_Flag = 0;// 停车标志，为1时开启终点检测，为2时停车，关闭中断
uint16_t Start_Count = 0;
uint16_t Stop_Count = 0;
// -------------------直立数据------------------------
float Acc = 0.0;    //下落加速度
float Gyro = 0.0;   //角速度
float Angle = 0.0;  //实际角度
float Dt = 0.0;     //时间
// ---------------------------------------------------
void PIT_IRQ(void)
{
    if (Start_Flag == 0) {
        if (++Start_Count >= 2000) { // 
            Start_Flag = 1;
            Start_Count = 0;
        }
    }
    else if (Start_Flag == 1) { // 已经启动
        if (++Start_Count >= 12000) { // 12s之后
            Start_Flag = 2;
            Stop_Flag = 1;  //开始检测起跑线
            Start_Count = 0;
        }
    }
    else if (Start_Flag == 3) { // 检测到斑马线
        if (++Start_Count >= 500) {
            Stop_Flag = 4;//停车
            Start_Flag = 0;
            Start_Count = 0;
        }
    }
    Is_Run = (Stop_Flag == 4 || Switch_4) ? (FALSE) : (TRUE);  // 是否停车和启动
    if (Timer_1ms == 0) {
        // 读取速度
        Key_Update(); //按键检测
        Run_Light();  //程序运行灯
        ++LCD_Counter; //OLED刷屏
    }
    else if (Timer_1ms == 1) {
        // 该放点啥呢
    }
    else if (Timer_1ms == 2) {
        Read_AccGyro(); //读取角角速度和角速度，耗时220us
        Dt = Micros() * 0.000002f; // 获得两次的采样间隔,并放大2倍
        Angle = Kalman_Filter(Acc, Gyro, Dt);     //进行滤波融合，耗时20us
        Angle_Pwm = Balance_Control(Angle, Gyro); // Note：Gyro为滤波后的效果
        // 三个控制量融合
        Right_Pwm = Left_Pwm = Angle_Pwm; // 角度控制量
        // 速度量 分成20份平滑输出
        Left_Pwm  -= ((Speed_Pwm - Old_Speed_Pwm) * (Speed_Count + 1)) / 20 + Old_Speed_Pwm;
        Right_Pwm -= ((Speed_Pwm - Old_Speed_Pwm) * (Speed_Count + 1)) / 20 + Old_Speed_Pwm;
        // 方向量 分成4份平滑
        Left_Pwm  -= (((Direction_Pwm - Old_Direction_Pwm) * (Direction_Count + 1)) >> 2) + Old_Direction_Pwm;
        Right_Pwm += (((Direction_Pwm - Old_Direction_Pwm) * (Direction_Count + 1)) >> 2) + Old_Direction_Pwm;
        // 电机输出
        Is_Run ? Motor_Out(Left_Pwm, Right_Pwm) : Motor_Out(0, 0);
    }
    else if (Timer_1ms == 3) {
        if (++Speed_Count >= 20) {// 速度控制 100ms 控制一次
            Get_Speed = Get_Pulse();
            Set_Speed = Set_Pulse();
            Old_Speed_Pwm = Speed_Pwm; // 记下上一次的值
            Speed_Pwm = Speed_Control(Set_Speed, Get_Speed);
        }
    }
    else if (Timer_1ms == 4) {
        if (++Direction_Count >= 4) {// 方向控制 20ms调用一次
            Old_Direction_Pwm = Direction_Pwm; // 记下上一次的值
            Direction_Pwm = Direction_Control(Center_Line);
            Direction_Count = 0;
        }
    }
    if (++Timer_1ms >= 5) { //计数器
        Timer_1ms = 0;
    }
}
