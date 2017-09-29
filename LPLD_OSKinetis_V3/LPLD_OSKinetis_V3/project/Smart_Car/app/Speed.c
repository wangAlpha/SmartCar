/**
 *-------------基于"拉普兰德K60底层库V3.1"的智能车工程-----------------
 * @file Speed.c
 * @version 0.0
 * @date 2017-04-26
 * @brief 速度控制，编码脉冲测量，电机PWM输出
 *
 * 开发者 湖南师范大学 飞思卡尔光电直立组 音速战神
 * 硬件平台 MK60DN512VLL10
 * @par None
**/
#include "common.h"
#include "Speed.h"
#include "PIT.h"
#include "Direction.h"
#include "Camera.h"

/*----------------------速度PID参数-----------------*/
int16_t S_Kp = 250;//220; // 放大参数，避免浮点运算
int16_t S_Ki = 4;
int16_t S_Kd = 0;

int16_t S_Last_Error = 0;
int16_t S_Pre_Error = 0;
int16_t S_Error = 0;
int32_t S_Inter = 0;
// -------------------------------------------------
const int16_t Max_Speed = 10000;
int16_t Speed_A = 20;
int16_t Speed_Hightest = 1500;
int16_t Speed_Lowest   = 1500;
int16_t Speed_Straight = 1500;

#define SQUARE(x) ((x)*(x)) // 平方

// 设定速度，根据赛道的情况设定
int16_t Set_Pulse(void)
{
    int32_t set_speed = 0;
    if (Start_Flag == 3) { //刹车
        set_speed = -4000;
    }
    else { //
        set_speed = Speed_Hightest;
    }
    return set_speed;
}

// 速度控制,采用位置式PID控制
int16_t Speed_Control(int16_t set_speed, int32_t get_speed)
{
    S_Error = set_speed - get_speed;
    if (!Obstacle_Flag) { // 积分分离
        S_Inter += S_Error;
    }
    if (S_Inter > 50000) { //积分抗饱和
        S_Inter = 50000;
    }
    else if (S_Inter < -50000) {
        S_Inter = -50000;
    }
    //int16_t S_Index = (Start_Flag == 0) ? 6 : S_Ki;//10
    return (S_Kp * S_Error + S_Ki * S_Inter) / 100;
}
// 获取右边编码器脉冲数
int16_t Get_RightPulse(void)
{
    int16_t count = (int16_t)FTM2->CNT;
    FTM2->CNT = 0;
    return count;
}

// 获取左边编码器脉冲数
int16_t Get_LeftPulse(void)
{
    int16_t count = (int16_t)FTM1->CNT;
    FTM1->CNT = 0;
    return count;
}

// 获取速度
int16_t Get_Pulse(void)
{
    return ((Get_LeftPulse() + Get_RightPulse()) >> 1);
}

// 电机速度输出，未加死区
void Motor_Out(int16_t right_duty, int16_t left_duty)
{
    // PWM饱和限幅
    if (left_duty > Max_Speed) {
        left_duty = Max_Speed;
    }
    else if (left_duty < -Max_Speed) {
        left_duty = -Max_Speed;
    }
    if (right_duty > Max_Speed) {
        right_duty = Max_Speed;
    }
    else if (right_duty < -Max_Speed) {
        right_duty = -Max_Speed;
    }
    // 左边速度
    if (left_duty >= 0) {
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch6, 0);
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch7, left_duty);
    }
    else {
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch6, -left_duty);
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch7, 0);
    }
    // 右边速度
    if (right_duty >= 0) {
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch4, right_duty);
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch5, 0);
    }
    else {
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch4, 0);
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch5, -right_duty);
    }
}
