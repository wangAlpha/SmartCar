/**
 *-------------����"��������K60�ײ��V3.1"�����ܳ�����-----------------
 * @file Speed.c
 * @version 0.0
 * @date 2017-04-26
 * @brief �ٶȿ��ƣ�����������������PWM���
 *
 * ������ ����ʦ����ѧ ��˼�������ֱ���� ����ս��
 * Ӳ��ƽ̨ MK60DN512VLL10
 * @par None
**/
#include "common.h"
#include "Speed.h"
#include "PIT.h"
#include "Direction.h"
#include "Camera.h"

/*----------------------�ٶ�PID����-----------------*/
int16_t S_Kp = 250;//220; // �Ŵ���������⸡������
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

#define SQUARE(x) ((x)*(x)) // ƽ��

// �趨�ٶȣ���������������趨
int16_t Set_Pulse(void)
{
    int32_t set_speed = 0;
    if (Start_Flag == 3) { //ɲ��
        set_speed = -4000;
    }
    else { //
        set_speed = Speed_Hightest;
    }
    return set_speed;
}

// �ٶȿ���,����λ��ʽPID����
int16_t Speed_Control(int16_t set_speed, int32_t get_speed)
{
    S_Error = set_speed - get_speed;
    if (!Obstacle_Flag) { // ���ַ���
        S_Inter += S_Error;
    }
    if (S_Inter > 50000) { //���ֿ�����
        S_Inter = 50000;
    }
    else if (S_Inter < -50000) {
        S_Inter = -50000;
    }
    //int16_t S_Index = (Start_Flag == 0) ? 6 : S_Ki;//10
    return (S_Kp * S_Error + S_Ki * S_Inter) / 100;
}
// ��ȡ�ұ߱�����������
int16_t Get_RightPulse(void)
{
    int16_t count = (int16_t)FTM2->CNT;
    FTM2->CNT = 0;
    return count;
}

// ��ȡ��߱�����������
int16_t Get_LeftPulse(void)
{
    int16_t count = (int16_t)FTM1->CNT;
    FTM1->CNT = 0;
    return count;
}

// ��ȡ�ٶ�
int16_t Get_Pulse(void)
{
    return ((Get_LeftPulse() + Get_RightPulse()) >> 1);
}

// ����ٶ������δ������
void Motor_Out(int16_t right_duty, int16_t left_duty)
{
    // PWM�����޷�
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
    // ����ٶ�
    if (left_duty >= 0) {
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch6, 0);
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch7, left_duty);
    }
    else {
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch6, -left_duty);
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch7, 0);
    }
    // �ұ��ٶ�
    if (right_duty >= 0) {
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch4, right_duty);
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch5, 0);
    }
    else {
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch4, 0);
        LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch5, -right_duty);
    }
}
