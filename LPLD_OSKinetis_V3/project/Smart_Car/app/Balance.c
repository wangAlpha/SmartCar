/**
 *-------------基于"拉普兰德K60底层库V3.1"的智能车工程-----------------
 * @file Balance.c
 * @version 0.0
 * @date 2017-04-26
 * @brief 直立控制，获取小车的角度，进行直立控制
 *
 * 开发者 湖南师范大学 飞思卡尔光电直立组 音速战神
 * 硬件平台 MK60DN512VLL10
 * @par None
**/
#include "common.h"
#include "DEV_MPU6050.h"
#include "Balance.h"
#include "PIT.h"

const float PI = 3.1415f;
// PID 参数
int16_t A_Kp = 1100;//1000; //1200//放大参数，消除浮点
int16_t A_Kd = -50;//-30;//-40
// 零偏
static float Offset_Gyro = 8.4041015624999992;
static float Offset_Acc  = 277.69140625;

// 直立控制
int16_t Balance_Control(float angle, float gyro)
{
    static int angle_counter = 0;
    if (angle < -40 || angle > 40) { // 保护程序
        if (++angle_counter > 50) {
            Stop_Flag = 4;
        }
    }
    else {
        angle_counter = 0;
    }
    return (int16_t)(angle * A_Kp + gyro * A_Kd);
}

// 获取小车倒下的加速度和角速度
void Read_AccGyro(void)
{
    //范围为2g 16384 LSB/g
    //x=sinx得到角度（弧度）, deg = rad*180/3.14，乘以1.2适当放大
    Acc = (MPU6050_GetResult(ACCEL_XOUT_H) - Offset_Acc) / 16384.0f * 1.2f * 180.0f / PI; //弧度转角度
    //范围为500deg/s时，换算关系：65.5 LSB/(deg/s)
    Gyro = -(MPU6050_GetResult(GYRO_YOUT_H) - Offset_Gyro) / 65.5f; // 转换为角速度
    //Get_Offset();
}
// 获取零偏
void Get_Offset(void)
{
    static uint32_t i = 0;
    if (i >= 10240) {
        Offset_Gyro /= 10240.0;
        Offset_Acc  /= 10240.0;
        while (true);
    }
    else {
        ++i;
        Offset_Gyro += Gyro;
        Offset_Acc  += Acc;
    }
}
// 返回上次与这个函数的时间间隔 单位(us)
uint32_t Micros(void)
{
    uint32_t timer = (g_bus_clock - PIT->CHANNEL[1].CVAL) / 50;
    PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK; //关闭定时器
    PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TEN_MASK;  //重启定时器
    return timer;
}
/*-------------Kalman Filter-------------------------*/
#pragma optimize=speed
float Kalman_Filter(float acc, float gyro, float dt)
{
    static const float Q_angle = 0.001f;
    static const float Q_gyro  = 0.003f;
    static const float R_angle = 0.01f;//减小R以加快收敛
    static const char  C_0 = 1;
    static float angle = 0.0;
    static float Q_bias, Angle_err;
    static float PCt_0, PCt_1, E;
    static float K_0, K_1, t_0, t_1;
    static float Pdot[4] = {0, 0, 0, 0};
    static float PP[2][2] = { { 1, 0 }, { 0, 1 } };
    angle  += (gyro - Q_bias) * dt; //先验估计
    Pdot[0] = Q_angle - PP[0][1] - PP[1][0]; // Pk-先验估计误差协方差的微分
    Pdot[1] = -PP[1][1];
    Pdot[2] = -PP[1][1];
    Pdot[3] = Q_gyro;
    PP[0][0] += Pdot[0] * dt;   // Pk-先验估计误差协方差微分的积分
    PP[0][1] += Pdot[1] * dt;   // =先验估计误差协方差
    PP[1][0] += Pdot[2] * dt;
    PP[1][1] += Pdot[3] * dt;
    Angle_err = acc - angle;   //zk-先验估计
    PCt_0 = C_0 * PP[0][0];
    PCt_1 = C_0 * PP[1][0];
    E = R_angle + C_0 * PCt_0;
    K_0 = PCt_0 / E;
    K_1 = PCt_1 / E;
    t_0 = PCt_0;
    t_1 = C_0 * PP[0][1];
    PP[0][0] -= K_0 * t_0;       //后验估计误差协方差
    PP[0][1] -= K_0 * t_1;
    PP[1][0] -= K_1 * t_0;
    PP[1][1] -= K_1 * t_1;
    angle  += K_0 * Angle_err;  //后验估计
    Q_bias += K_1 * Angle_err;  //后验估计
    Gyro    = gyro - Q_bias;     //输出值(后验估计)的微分=角速度
    return angle;
}
