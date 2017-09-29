/**
 *-------------基于"拉普兰德K60底层库V3.1"的智能车工程-----------------
 * @file Main.c
 * @version 0.0
 * @date 2017-04-26
 * @brief 主函数
 *
 * 开发者 湖南师范大学 飞思卡尔光电直立组 音速战神
 * 硬件平台 MK60DN512VLL10
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
    //LPLD_LPTMR_DelayMs(3000); // 延时发车
    Stop_Flag = 4;
    for (;;) {
        if (Is_DispPhoto) {
            LPLD_GPIO_ClearIntFlag(PORTE); // 清除场中断标志位
            enable_irq(PORTE_IRQn); // 使能场中断
            Is_DispPhoto = false;
            Image_Binary();
            Image_Analysis();
            Display();             // 电机启动自动关闭
            //SendImage();
        }
        __WFI();                   //睡眠模式
    }
}
