/**
 *-------------����"��������K60�ײ��V3.1"�����ܳ�����-----------------
 * @file Main.c
 * @version 0.0
 * @date 2017-04-26
 * @brief ������
 *
 * ������ ����ʦ����ѧ ��˼�������ֱ���� ����ս��
 * Ӳ��ƽ̨ MK60DN512VLL10
 * @par None
**/
#include "common.h"
#include "Init.h"
#include "Param.h"
#include "Balance.h"
#include "Camera.h"
#include "PIT.h"

void main(void)
{
    BoardInit();
    Param_Init();
    UART_Config();
    //LPLD_LPTMR_DelayMs(3000); // ��ʱ����
    Stop_Flag = 4;
    for (;;) {
        if (Is_DispPhoto) {
            LPLD_GPIO_ClearIntFlag(PORTE); // ������жϱ�־λ
            enable_irq(PORTE_IRQn); // ʹ�ܳ��ж�
            Is_DispPhoto = false;
            Image_Binary();
            Image_Analysis();
            Display();             // ��������Զ��ر�
            //SendImage();
        }
        __WFI();                   //˯��ģʽ
    }
}
