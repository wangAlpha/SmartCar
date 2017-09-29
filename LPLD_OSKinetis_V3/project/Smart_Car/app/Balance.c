/**
 *-------------����"��������K60�ײ��V3.1"�����ܳ�����-----------------
 * @file Balance.c
 * @version 0.0
 * @date 2017-04-26
 * @brief ֱ�����ƣ���ȡС���ĽǶȣ�����ֱ������
 *
 * ������ ����ʦ����ѧ ��˼�������ֱ���� ����ս��
 * Ӳ��ƽ̨ MK60DN512VLL10
 * @par None
**/
#include "common.h"
#include "DEV_MPU6050.h"
#include "Balance.h"
#include "PIT.h"

const float PI = 3.1415f;
// PID ����
int16_t A_Kp = 1100;//1000; //1200//�Ŵ��������������
int16_t A_Kd = -50;//-30;//-40
// ��ƫ
static float Offset_Gyro = 8.4041015624999992;
static float Offset_Acc  = 277.69140625;

// ֱ������
int16_t Balance_Control(float angle, float gyro)
{
    static int angle_counter = 0;
    if (angle < -40 || angle > 40) { // ��������
        if (++angle_counter > 50) {
            Stop_Flag = 4;
        }
    }
    else {
        angle_counter = 0;
    }
    return (int16_t)(angle * A_Kp + gyro * A_Kd);
}

// ��ȡС�����µļ��ٶȺͽ��ٶ�
void Read_AccGyro(void)
{
    //��ΧΪ2g 16384 LSB/g
    //x=sinx�õ��Ƕȣ����ȣ�, deg = rad*180/3.14������1.2�ʵ��Ŵ�
    Acc = (MPU6050_GetResult(ACCEL_XOUT_H) - Offset_Acc) / 16384.0f * 1.2f * 180.0f / PI; //����ת�Ƕ�
    //��ΧΪ500deg/sʱ�������ϵ��65.5 LSB/(deg/s)
    Gyro = -(MPU6050_GetResult(GYRO_YOUT_H) - Offset_Gyro) / 65.5f; // ת��Ϊ���ٶ�
    //Get_Offset();
}
// ��ȡ��ƫ
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
// �����ϴ������������ʱ���� ��λ(us)
uint32_t Micros(void)
{
    uint32_t timer = (g_bus_clock - PIT->CHANNEL[1].CVAL) / 50;
    PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK; //�رն�ʱ��
    PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TEN_MASK;  //������ʱ��
    return timer;
}
/*-------------Kalman Filter-------------------------*/
#pragma optimize=speed
float Kalman_Filter(float acc, float gyro, float dt)
{
    static const float Q_angle = 0.001f;
    static const float Q_gyro  = 0.003f;
    static const float R_angle = 0.01f;//��СR�Լӿ�����
    static const char  C_0 = 1;
    static float angle = 0.0;
    static float Q_bias, Angle_err;
    static float PCt_0, PCt_1, E;
    static float K_0, K_1, t_0, t_1;
    static float Pdot[4] = {0, 0, 0, 0};
    static float PP[2][2] = { { 1, 0 }, { 0, 1 } };
    angle  += (gyro - Q_bias) * dt; //�������
    Pdot[0] = Q_angle - PP[0][1] - PP[1][0]; // Pk-����������Э�����΢��
    Pdot[1] = -PP[1][1];
    Pdot[2] = -PP[1][1];
    Pdot[3] = Q_gyro;
    PP[0][0] += Pdot[0] * dt;   // Pk-����������Э����΢�ֵĻ���
    PP[0][1] += Pdot[1] * dt;   // =����������Э����
    PP[1][0] += Pdot[2] * dt;
    PP[1][1] += Pdot[3] * dt;
    Angle_err = acc - angle;   //zk-�������
    PCt_0 = C_0 * PP[0][0];
    PCt_1 = C_0 * PP[1][0];
    E = R_angle + C_0 * PCt_0;
    K_0 = PCt_0 / E;
    K_1 = PCt_1 / E;
    t_0 = PCt_0;
    t_1 = C_0 * PP[0][1];
    PP[0][0] -= K_0 * t_0;       //����������Э����
    PP[0][1] -= K_0 * t_1;
    PP[1][0] -= K_1 * t_0;
    PP[1][1] -= K_1 * t_1;
    angle  += K_0 * Angle_err;  //�������
    Q_bias += K_1 * Angle_err;  //�������
    Gyro    = gyro - Q_bias;     //���ֵ(�������)��΢��=���ٶ�
    return angle;
}
