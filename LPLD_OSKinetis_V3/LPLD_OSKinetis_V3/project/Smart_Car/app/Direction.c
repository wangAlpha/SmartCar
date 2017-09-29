/**
 *-------------基于"拉普兰德K60底层库V3.1"的智能车工程-----------------
 * @file Direction.c
 * @version 0.0
 * @date 2017-04-26
 * @brief 方向控制，根据赛道状态的不同进行方向控制
 *
 * 开发者 湖南师范大学 飞思卡尔光电直立组 音速战神
 * 硬件平台 MK60DN512VLL10
 * @par None
**/
#include "common.h"
#include "Direction.h"
#include "Camera.h"

int16_t D_Kp_a = 10;//放大参数，消除浮点运算
int16_t D_Kp_b = 120;//150;//200;
int16_t D_Kd = 1550;//1800;

int16_t D_Error = 0;
int16_t D_Last_Error = 0;

const int16_t Set_Center_Line = Width / 2;
const int16_t D_Dead = 1; // 方向死区

int16_t Direction_Control(int16_t line)
{
    D_Last_Error = D_Error;           // e(n-1)
    D_Error = Set_Center_Line - line; // e(n)
    int16_t out_kp = 0, out_kd = 0;
    // Kp = e(n) ^ 2 * 1/a + b
    if (Circle_Flag) {
        out_kp = 130;//(int16_t)(D_Error * D_Error) / 10.0f / 101 + 120;//155;
        out_kd = 1550;
    }
    else if (Obstacle_Flag != 0) {
        out_kp = 100;//150;
        out_kd = 1000;//1550;
    }
    else {
        out_kp = (int16_t)((D_Error * D_Error) / 10.0f / D_Kp_a + D_Kp_b);
        out_kd = D_Kd;
    }
    int16_t output = 0;
    if ((D_Error > D_Dead) || (D_Error < -D_Dead)) { // 只有超过死区才进行调节
        output = (out_kp * D_Error + out_kd * (D_Error - D_Last_Error)) / 10;
    }
    return output;
}
