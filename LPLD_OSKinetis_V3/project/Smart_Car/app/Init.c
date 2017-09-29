/**
 *-------------基于"拉普兰德K60底层库V3.1"的智能车工程-----------------
 * @file Init.c
 * @version 0.0
 * @date 2017-04-26
 * @brief 硬件模块模块初始化配置
 *
 * 开发者 湖南师范大学 飞思卡尔光电直立组 音速战神
 * 硬件平台 MK60DN512VLL10
 * @par None
**/
#include "common.h"
#include "DEV_OLED.h"
#include "DEV_MPU6050.h"
#include "Init.h"
#include "PIT.h"
#include "Param.h"
#include "Camera.h"
#include "DEV_SCCB.h"

// 按键初始化
static void Key_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    // KEY[1..4] 轻触开关
    GPIO_InitStruct.GPIO_PTx        = PTB;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin20 | GPIO_Pin21 | GPIO_Pin22 | GPIO_Pin23;
    GPIO_InitStruct.GPIO_Dir        = DIR_INPUT;
    GPIO_InitStruct.GPIO_Output     = OUTPUT_H;
    GPIO_InitStruct.GPIO_PinControl = INPUT_PULL_UP;
    LPLD_GPIO_Init(GPIO_InitStruct);
    // Switch[1..4] 拨码开关
    GPIO_InitStruct.GPIO_PTx  = PTC;
    GPIO_InitStruct.GPIO_Pins = GPIO_Pin0 | GPIO_Pin1 | GPIO_Pin2 | GPIO_Pin5;
    LPLD_GPIO_Init(GPIO_InitStruct);
}

// 定时器初始化
static void PIT_Config(void)
{
    PIT_InitTypeDef PIT_InitStruct;
    PIT_InitStruct.PIT_Pitx = PIT0;
    PIT_InitStruct.PIT_PeriodMs = 1;
    PIT_InitStruct.PIT_PeriodUs = 0;
    PIT_InitStruct.PIT_PeriodS  = 0;
    PIT_InitStruct.PIT_Isr = PIT_IRQ;
    LPLD_PIT_Init(PIT_InitStruct);
    LPLD_PIT_EnableIrq(PIT_InitStruct);
    PIT_InitStruct.PIT_Pitx = PIT1;
    PIT_InitStruct.PIT_PeriodMs = 0;
    PIT_InitStruct.PIT_PeriodUs = 0;
    PIT_InitStruct.PIT_PeriodS  = 1;
    PIT_InitStruct.PIT_Isr = NULL;
    LPLD_PIT_Init(PIT_InitStruct);
    PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK; //关闭定时器
    PIT_InitStruct.PIT_Pitx = PIT2;
    LPLD_PIT_Init(PIT_InitStruct);
    PIT->CHANNEL[2].TCTRL &= ~PIT_TCTRL_TEN_MASK; //关闭定时器
}
// 程序运行灯
static void RunLight_Config(void)
{
    //LED 显示芯片是否运行
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_PTx        = PTC;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin6;
    GPIO_InitStruct.GPIO_Dir        = DIR_OUTPUT;
    GPIO_InitStruct.GPIO_PinControl = OUTPUT_DSH;
    GPIO_InitStruct.GPIO_Output     = OUTPUT_H;
    LPLD_GPIO_Init(GPIO_InitStruct);
    GPIO_InitStruct.GPIO_PTx        = PTA;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin12;
    LPLD_GPIO_Init(GPIO_InitStruct);
}

// 编码器 电机初始化
static void Speed_Config(void)
{
    // PWM配置 边缘对齐 10KHz
    FTM_InitTypeDef FTM_InitStruct;
    FTM_InitStruct.FTM_Ftmx           = FTM0;
    FTM_InitStruct.FTM_Mode           = FTM_MODE_PWM;
    FTM_InitStruct.FTM_PwmFreq        = 18000;//18kHz
    FTM_InitStruct.FTM_PwmDeadtimeCfg = DEADTIME_CH45; //插入死区
    FTM_InitStruct.FTM_PwmDeadtimeDiv = DEADTIME_DIV4; //PWM死区对总线的分频系数
    FTM_InitStruct.FTM_PwmDeadtimeVal = 25;//25个死区
    LPLD_FTM_Init(FTM_InitStruct);
    FTM_InitStruct.FTM_PwmDeadtimeCfg = DEADTIME_CH67;
    LPLD_FTM_Init(FTM_InitStruct);
    LPLD_FTM_PWM_Enable(FTM0, FTM_Ch4, 0, PTD4, ALIGN_LEFT);
    LPLD_FTM_PWM_Enable(FTM0, FTM_Ch5, 0, PTD5, ALIGN_LEFT); //正方向
    LPLD_FTM_PWM_Enable(FTM0, FTM_Ch6, 0, PTD6, ALIGN_LEFT);
    LPLD_FTM_PWM_Enable(FTM0, FTM_Ch7, 0, PTD7, ALIGN_LEFT); //正方向
    // ----------------编码器配置---------------------------
    FTM_InitStruct.FTM_Ftmx      = FTM2;
    FTM_InitStruct.FTM_Mode      = FTM_MODE_QD;
    FTM_InitStruct.FTM_QdMode    = QD_MODE_PHAB; //AB相解码
    FTM_InitStruct.FTM_ToiEnable = FALSE;// 防止编码器计数产生溢出
    LPLD_FTM_Init(FTM_InitStruct);
    LPLD_FTM_QD_Enable(FTM2, PTB18, PTB19);
    FTM_InitStruct.FTM_Ftmx      = FTM1;
    LPLD_FTM_Init(FTM_InitStruct);
    LPLD_FTM_QD_Enable(FTM1,  PTB0, PTB1);
    // 为编码器配置上拉
    // 右编码器
    PORTB->PCR[18] |= INPUT_PULL_UP;
    PORTB->PCR[19] |= INPUT_PULL_UP;
    // 左编码器
    PORTB->PCR[0] |= INPUT_PULL_UP;
    PORTB->PCR[1] |= INPUT_PULL_UP;
}

//电池电量获取
static void ADC_Config(void)
{
    ADC_InitTypeDef ADC_InitStruct;
    ADC_InitStruct.ADC_Adcx              = ADC0;
    ADC_InitStruct.ADC_DiffMode          = ADC_SE;
    ADC_InitStruct.ADC_BitMode           = SE_16BIT;
    ADC_InitStruct.ADC_SampleTimeCfg     = SAMTIME_LONG;
    ADC_InitStruct.ADC_LongSampleTimeSel = LSAMTIME_20EX;
    ADC_InitStruct.ADC_HwAvgSel          = HW_32AVG;
    ADC_InitStruct.ADC_MuxSel            = MUX_ADXXA;
    ADC_InitStruct.ADC_CalEnable         = TRUE;
    ADC_InitStruct.ADC_HwTrgCfg          = HW_TRGDSABLE; //禁用硬件触发
    LPLD_ADC_Init(ADC_InitStruct);
    LPLD_ADC_Chn_Enable(ADC0, DAD0);
}

// 中断优先级配置
static void NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    // 定时器优先级
    //8个响应优先级，两个抢占优先级
    NVIC_InitStruct.NVIC_IRQChannelGroupPriority = NVIC_PriorityGroup_2;
    // 摄像头场中断
    NVIC_InitStruct.NVIC_IRQChannel                   = PORTE_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // 最高抢占优先级
    NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 0;        //最高响应优先级
    NVIC_InitStruct.NVIC_IRQChannelEnable             = TRUE;
    LPLD_NVIC_Init(NVIC_InitStruct);
    // 摄像头行中断
    NVIC_InitStruct.NVIC_IRQChannel                   = PORTD_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStruct.NVIC_IRQChannelEnable             = TRUE;
    LPLD_NVIC_Init(NVIC_InitStruct);
    // 定时器中断
    NVIC_InitStruct.NVIC_IRQChannel                   = PIT0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStruct.NVIC_IRQChannelEnable             = TRUE;
    LPLD_NVIC_Init(NVIC_InitStruct);
    // 串口中断
    /*NVIC_InitStruct.NVIC_IRQChannel = UART2_RX_TX_IRQn; // 最低
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelEnable = TRUE;
    LPLD_NVIC_Init(NVIC_InitStruct);*/
}

// 摄像头端口初始化
static void GPIO_Config()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_PTx        = PTA;
    GPIO_InitStruct.GPIO_Dir        = DIR_OUTPUT;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin16;
    GPIO_InitStruct.GPIO_PinControl = IRQC_DIS | INPUT_PULL_DIS; // 禁用中断，禁用PULL
    GPIO_InitStruct.GPIO_Output     = OUTPUT_L;
    GPIO_InitStruct.GPIO_Isr        = NULL;
    LPLD_GPIO_Init(GPIO_InitStruct);
    //Y[0..7]数据口初始化   PTC8-15
    GPIO_InitStruct.GPIO_PTx        = PTC;
    GPIO_InitStruct.GPIO_Dir        = DIR_INPUT;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin8_15;
    GPIO_InitStruct.GPIO_PinControl = IRQC_DIS | INPUT_PULL_DIS; // 禁用中断，禁用PULL
    GPIO_InitStruct.GPIO_Output     = OUTPUT_L;
    GPIO_InitStruct.GPIO_Isr        = NULL;
    LPLD_GPIO_Init(GPIO_InitStruct);
    //HREF 行信号接口初始化     PTD1
    GPIO_InitStruct.GPIO_PTx        = PTD;
    GPIO_InitStruct.GPIO_Dir        = DIR_INPUT;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin1;
    GPIO_InitStruct.GPIO_PinControl = IRQC_RI | INPUT_PULL_DOWN; // 上升沿中断，下拉
    GPIO_InitStruct.GPIO_Isr        = Row_IRQ;
    LPLD_GPIO_Init(GPIO_InitStruct);
    //YSYN 场信号接口初始化     PTE6
    GPIO_InitStruct.GPIO_PTx        = PTE;
    GPIO_InitStruct.GPIO_Dir        = DIR_INPUT;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin6;
    GPIO_InitStruct.GPIO_PinControl = IRQC_RI | INPUT_PULL_DOWN;
    GPIO_InitStruct.GPIO_Isr        = Field_IRQ;
    LPLD_GPIO_Init(GPIO_InitStruct);
    //PCLK信号接口初始化 C18
    GPIO_InitStruct.GPIO_PTx        = PTC;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin18;
    GPIO_InitStruct.GPIO_Dir        = DIR_INPUT;
    GPIO_InitStruct.GPIO_PinControl = IRQC_DMARI | INPUT_PULL_DIS; // 上升沿触发DMA 请求，禁用PULL
    GPIO_InitStruct.GPIO_Isr        = NULL;
    LPLD_GPIO_Init(GPIO_InitStruct);
}

// DMA数据传输配置
static void DMA_Config(void)
{
    DMA_InitTypeDef DMA_InitStruct;
    DMA_InitStruct.DMA_CHx                   = DMA_CH0; //0通道
    DMA_InitStruct.DMA_Req                   = PORTC_DMAREQ; //请求源
    DMA_InitStruct.DMA_PeriodicTriggerEnable = FALSE;
    DMA_InitStruct.DMA_MajorLoopCnt          = H;//一次采集的
    DMA_InitStruct.DMA_MinorByteCnt          = 1;//每次读入一个字节
    DMA_InitStruct.DMA_SourceAddr            = (uint32_t)&PTC->PDIR + 1;//源地址 PTC0- C7
    DMA_InitStruct.DMA_DestAddr              = (uint32_t)Data_Buffer; //存放图像的数组  //uint8_t DMABuffer[400];
    DMA_InitStruct.DMA_DestAddrOffset        = 1;//目的地址偏移
    DMA_InitStruct.DMA_AutoDisableReq = TRUE; // 自动禁用请求
    // ---------------------------------------------------------
    DMA_InitStruct.DMA_SourceAddrOffset           = 0;
    DMA_InitStruct.DMA_SourceDataSize             = DMA_SRC_8BIT;
    DMA_InitStruct.DMA_DestDataSize               = DMA_DST_8BIT;
    DMA_InitStruct.DMA_LastSourceAddrAdj          = 0;
    DMA_InitStruct.DMA_LastDestAddrAdj            = 0;
    DMA_InitStruct.DMA_MajorCompleteIntEnable     = FALSE;
    DMA_InitStruct.DMA_MajorHalfCompleteIntEnable = FALSE;
    DMA_InitStruct.DMA_Isr                        = NULL;
    LPLD_DMA_Init(DMA_InitStruct);
}

// OV7620配置
void Camera_Config(void)
{
    GPIO_Config();
    DMA_Config();
    LPLD_SCCB_Init();
    LPLD_SCCB_WriteReg(0x12, 0x80);
    LPLD_SCCB_WriteReg(0x14, 0x20); //设置分辨率 QVGA 320*240
    enable_irq(PORTE_IRQn);         //使能场中断
    disable_irq(PORTD_IRQn);        //禁止行中断
}
// 蓝牙串口配置
void UART_Config(void)
{
    UART_InitTypeDef UART_InitStruct;
    UART_InitStruct.UART_Uartx    = UART1; //使用UART1
    UART_InitStruct.UART_BaudRate = 115200;//设置波特率
    UART_InitStruct.UART_RxPin    = PTC3;  //接收引脚
    UART_InitStruct.UART_TxPin    = PTC4;  //发送引脚
    LPLD_UART_Init(UART_InitStruct);       //初始化UART
}


void BoardInit(void)
{
    // 先禁止中断，防止中断触发导致异常
    DisableInterrupts;
    RunLight_Config();
    LCD_Init();
    Key_Config();
    Speed_Config();
    ADC_Config();
    NVIC_Config();
    Camera_Config();
    MPU6050_Init();
    // LPLD_Flash_Init();
    PIT_Config();
    EnableInterrupts;
}
