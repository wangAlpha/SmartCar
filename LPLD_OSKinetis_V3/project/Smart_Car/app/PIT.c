/**
 *-------------����"��������K60�ײ��V3.1"�����ܳ�����-----------------
 * @file PIT.c
 * @version 0.0
 * @date 2017-04-26
 * @brief ��ʱ�жϣ�ʵ������ʵʱ����
 *
 * ������ ����ʦ����ѧ ��˼�������ֱ���� ����ս��
 * Ӳ��ƽ̨ MK60DN512VLL10
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
// ----------------�趨�ٶ�--------------------------
int16_t Speed_Pwm = 0; //�ٶ�
int16_t Old_Speed_Pwm = 0;
int16_t Angle_Pwm = 0;
int16_t Left_Pwm = 0;  //��������ٶ�
int16_t Right_Pwm = 0; //��������ٶ�
int16_t Direction_Pwm = 0; //�������
int16_t Old_Direction_Pwm = 0;
bool Is_Run = FALSE;   // ����Ƿ����б�־
// -------------------������-------------------------
uint8_t Timer_1ms = 0;
uint8_t LCD_Counter = 0;   //LCDˢ�¼�����
uint8_t Direction_Count = 0;
uint8_t CCD_Count = 0;
uint8_t Speed_Count = 0;
uint8_t Pulse_Count = 0;
uint8_t Start_Flag = 0; // ������־
uint8_t Stop_Flag = 0;// ͣ����־��Ϊ1ʱ�����յ��⣬Ϊ2ʱͣ�����ر��ж�
uint16_t Start_Count = 0;
uint16_t Stop_Count = 0;
// -------------------ֱ������------------------------
float Acc = 0.0;    //������ٶ�
float Gyro = 0.0;   //���ٶ�
float Angle = 0.0;  //ʵ�ʽǶ�
float Dt = 0.0;     //ʱ��
// ---------------------------------------------------
void PIT_IRQ(void)
{
    if (Start_Flag == 0) {
        if (++Start_Count >= 2000) { // 
            Start_Flag = 1;
            Start_Count = 0;
        }
    }
    else if (Start_Flag == 1) { // �Ѿ�����
        if (++Start_Count >= 12000) { // 12s֮��
            Start_Flag = 2;
            Stop_Flag = 1;  //��ʼ���������
            Start_Count = 0;
        }
    }
    else if (Start_Flag == 3) { // ��⵽������
        if (++Start_Count >= 500) {
            Stop_Flag = 4;//ͣ��
            Start_Flag = 0;
            Start_Count = 0;
        }
    }
    Is_Run = (Stop_Flag == 4 || Switch_4) ? (FALSE) : (TRUE);  // �Ƿ�ͣ��������
    if (Timer_1ms == 0) {
        // ��ȡ�ٶ�
        Key_Update(); //�������
        Run_Light();  //�������е�
        ++LCD_Counter; //OLEDˢ��
    }
    else if (Timer_1ms == 1) {
        // �÷ŵ�ɶ��
    }
    else if (Timer_1ms == 2) {
        Read_AccGyro(); //��ȡ�ǽ��ٶȺͽ��ٶȣ���ʱ220us
        Dt = Micros() * 0.000002f; // ������εĲ������,���Ŵ�2��
        Angle = Kalman_Filter(Acc, Gyro, Dt);     //�����˲��ںϣ���ʱ20us
        Angle_Pwm = Balance_Control(Angle, Gyro); // Note��GyroΪ�˲����Ч��
        // �����������ں�
        Right_Pwm = Left_Pwm = Angle_Pwm; // �Ƕȿ�����
        // �ٶ��� �ֳ�20��ƽ�����
        Left_Pwm  -= ((Speed_Pwm - Old_Speed_Pwm) * (Speed_Count + 1)) / 20 + Old_Speed_Pwm;
        Right_Pwm -= ((Speed_Pwm - Old_Speed_Pwm) * (Speed_Count + 1)) / 20 + Old_Speed_Pwm;
        // ������ �ֳ�4��ƽ��
        Left_Pwm  -= (((Direction_Pwm - Old_Direction_Pwm) * (Direction_Count + 1)) >> 2) + Old_Direction_Pwm;
        Right_Pwm += (((Direction_Pwm - Old_Direction_Pwm) * (Direction_Count + 1)) >> 2) + Old_Direction_Pwm;
        // ������
        Is_Run ? Motor_Out(Left_Pwm, Right_Pwm) : Motor_Out(0, 0);
    }
    else if (Timer_1ms == 3) {
        if (++Speed_Count >= 20) {// �ٶȿ��� 100ms ����һ��
            Get_Speed = Get_Pulse();
            Set_Speed = Set_Pulse();
            Old_Speed_Pwm = Speed_Pwm; // ������һ�ε�ֵ
            Speed_Pwm = Speed_Control(Set_Speed, Get_Speed);
        }
    }
    else if (Timer_1ms == 4) {
        if (++Direction_Count >= 4) {// ������� 20ms����һ��
            Old_Direction_Pwm = Direction_Pwm; // ������һ�ε�ֵ
            Direction_Pwm = Direction_Control(Center_Line);
            Direction_Count = 0;
        }
    }
    if (++Timer_1ms >= 5) { //������
        Timer_1ms = 0;
    }
}
