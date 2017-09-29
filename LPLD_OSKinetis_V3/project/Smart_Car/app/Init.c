/**
 *-------------����"��������K60�ײ��V3.1"�����ܳ�����-----------------
 * @file Init.c
 * @version 0.0
 * @date 2017-04-26
 * @brief Ӳ��ģ��ģ���ʼ������
 *
 * ������ ����ʦ����ѧ ��˼�������ֱ���� ����ս��
 * Ӳ��ƽ̨ MK60DN512VLL10
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

// ������ʼ��
static void Key_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    // KEY[1..4] �ᴥ����
    GPIO_InitStruct.GPIO_PTx        = PTB;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin20 | GPIO_Pin21 | GPIO_Pin22 | GPIO_Pin23;
    GPIO_InitStruct.GPIO_Dir        = DIR_INPUT;
    GPIO_InitStruct.GPIO_Output     = OUTPUT_H;
    GPIO_InitStruct.GPIO_PinControl = INPUT_PULL_UP;
    LPLD_GPIO_Init(GPIO_InitStruct);
    // Switch[1..4] ���뿪��
    GPIO_InitStruct.GPIO_PTx  = PTC;
    GPIO_InitStruct.GPIO_Pins = GPIO_Pin0 | GPIO_Pin1 | GPIO_Pin2 | GPIO_Pin5;
    LPLD_GPIO_Init(GPIO_InitStruct);
}

// ��ʱ����ʼ��
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
    PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK; //�رն�ʱ��
    PIT_InitStruct.PIT_Pitx = PIT2;
    LPLD_PIT_Init(PIT_InitStruct);
    PIT->CHANNEL[2].TCTRL &= ~PIT_TCTRL_TEN_MASK; //�رն�ʱ��
}
// �������е�
static void RunLight_Config(void)
{
    //LED ��ʾоƬ�Ƿ�����
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

// ������ �����ʼ��
static void Speed_Config(void)
{
    // PWM���� ��Ե���� 10KHz
    FTM_InitTypeDef FTM_InitStruct;
    FTM_InitStruct.FTM_Ftmx           = FTM0;
    FTM_InitStruct.FTM_Mode           = FTM_MODE_PWM;
    FTM_InitStruct.FTM_PwmFreq        = 18000;//18kHz
    FTM_InitStruct.FTM_PwmDeadtimeCfg = DEADTIME_CH45; //��������
    FTM_InitStruct.FTM_PwmDeadtimeDiv = DEADTIME_DIV4; //PWM���������ߵķ�Ƶϵ��
    FTM_InitStruct.FTM_PwmDeadtimeVal = 25;//25������
    LPLD_FTM_Init(FTM_InitStruct);
    FTM_InitStruct.FTM_PwmDeadtimeCfg = DEADTIME_CH67;
    LPLD_FTM_Init(FTM_InitStruct);
    LPLD_FTM_PWM_Enable(FTM0, FTM_Ch4, 0, PTD4, ALIGN_LEFT);
    LPLD_FTM_PWM_Enable(FTM0, FTM_Ch5, 0, PTD5, ALIGN_LEFT); //������
    LPLD_FTM_PWM_Enable(FTM0, FTM_Ch6, 0, PTD6, ALIGN_LEFT);
    LPLD_FTM_PWM_Enable(FTM0, FTM_Ch7, 0, PTD7, ALIGN_LEFT); //������
    // ----------------����������---------------------------
    FTM_InitStruct.FTM_Ftmx      = FTM2;
    FTM_InitStruct.FTM_Mode      = FTM_MODE_QD;
    FTM_InitStruct.FTM_QdMode    = QD_MODE_PHAB; //AB�����
    FTM_InitStruct.FTM_ToiEnable = FALSE;// ��ֹ�����������������
    LPLD_FTM_Init(FTM_InitStruct);
    LPLD_FTM_QD_Enable(FTM2, PTB18, PTB19);
    FTM_InitStruct.FTM_Ftmx      = FTM1;
    LPLD_FTM_Init(FTM_InitStruct);
    LPLD_FTM_QD_Enable(FTM1,  PTB0, PTB1);
    // Ϊ��������������
    // �ұ�����
    PORTB->PCR[18] |= INPUT_PULL_UP;
    PORTB->PCR[19] |= INPUT_PULL_UP;
    // �������
    PORTB->PCR[0] |= INPUT_PULL_UP;
    PORTB->PCR[1] |= INPUT_PULL_UP;
}

//��ص�����ȡ
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
    ADC_InitStruct.ADC_HwTrgCfg          = HW_TRGDSABLE; //����Ӳ������
    LPLD_ADC_Init(ADC_InitStruct);
    LPLD_ADC_Chn_Enable(ADC0, DAD0);
}

// �ж����ȼ�����
static void NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    // ��ʱ�����ȼ�
    //8����Ӧ���ȼ���������ռ���ȼ�
    NVIC_InitStruct.NVIC_IRQChannelGroupPriority = NVIC_PriorityGroup_2;
    // ����ͷ���ж�
    NVIC_InitStruct.NVIC_IRQChannel                   = PORTE_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // �����ռ���ȼ�
    NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 0;        //�����Ӧ���ȼ�
    NVIC_InitStruct.NVIC_IRQChannelEnable             = TRUE;
    LPLD_NVIC_Init(NVIC_InitStruct);
    // ����ͷ���ж�
    NVIC_InitStruct.NVIC_IRQChannel                   = PORTD_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStruct.NVIC_IRQChannelEnable             = TRUE;
    LPLD_NVIC_Init(NVIC_InitStruct);
    // ��ʱ���ж�
    NVIC_InitStruct.NVIC_IRQChannel                   = PIT0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStruct.NVIC_IRQChannelEnable             = TRUE;
    LPLD_NVIC_Init(NVIC_InitStruct);
    // �����ж�
    /*NVIC_InitStruct.NVIC_IRQChannel = UART2_RX_TX_IRQn; // ���
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelEnable = TRUE;
    LPLD_NVIC_Init(NVIC_InitStruct);*/
}

// ����ͷ�˿ڳ�ʼ��
static void GPIO_Config()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_PTx        = PTA;
    GPIO_InitStruct.GPIO_Dir        = DIR_OUTPUT;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin16;
    GPIO_InitStruct.GPIO_PinControl = IRQC_DIS | INPUT_PULL_DIS; // �����жϣ�����PULL
    GPIO_InitStruct.GPIO_Output     = OUTPUT_L;
    GPIO_InitStruct.GPIO_Isr        = NULL;
    LPLD_GPIO_Init(GPIO_InitStruct);
    //Y[0..7]���ݿڳ�ʼ��   PTC8-15
    GPIO_InitStruct.GPIO_PTx        = PTC;
    GPIO_InitStruct.GPIO_Dir        = DIR_INPUT;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin8_15;
    GPIO_InitStruct.GPIO_PinControl = IRQC_DIS | INPUT_PULL_DIS; // �����жϣ�����PULL
    GPIO_InitStruct.GPIO_Output     = OUTPUT_L;
    GPIO_InitStruct.GPIO_Isr        = NULL;
    LPLD_GPIO_Init(GPIO_InitStruct);
    //HREF ���źŽӿڳ�ʼ��     PTD1
    GPIO_InitStruct.GPIO_PTx        = PTD;
    GPIO_InitStruct.GPIO_Dir        = DIR_INPUT;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin1;
    GPIO_InitStruct.GPIO_PinControl = IRQC_RI | INPUT_PULL_DOWN; // �������жϣ�����
    GPIO_InitStruct.GPIO_Isr        = Row_IRQ;
    LPLD_GPIO_Init(GPIO_InitStruct);
    //YSYN ���źŽӿڳ�ʼ��     PTE6
    GPIO_InitStruct.GPIO_PTx        = PTE;
    GPIO_InitStruct.GPIO_Dir        = DIR_INPUT;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin6;
    GPIO_InitStruct.GPIO_PinControl = IRQC_RI | INPUT_PULL_DOWN;
    GPIO_InitStruct.GPIO_Isr        = Field_IRQ;
    LPLD_GPIO_Init(GPIO_InitStruct);
    //PCLK�źŽӿڳ�ʼ�� C18
    GPIO_InitStruct.GPIO_PTx        = PTC;
    GPIO_InitStruct.GPIO_Pins       = GPIO_Pin18;
    GPIO_InitStruct.GPIO_Dir        = DIR_INPUT;
    GPIO_InitStruct.GPIO_PinControl = IRQC_DMARI | INPUT_PULL_DIS; // �����ش���DMA ���󣬽���PULL
    GPIO_InitStruct.GPIO_Isr        = NULL;
    LPLD_GPIO_Init(GPIO_InitStruct);
}

// DMA���ݴ�������
static void DMA_Config(void)
{
    DMA_InitTypeDef DMA_InitStruct;
    DMA_InitStruct.DMA_CHx                   = DMA_CH0; //0ͨ��
    DMA_InitStruct.DMA_Req                   = PORTC_DMAREQ; //����Դ
    DMA_InitStruct.DMA_PeriodicTriggerEnable = FALSE;
    DMA_InitStruct.DMA_MajorLoopCnt          = H;//һ�βɼ���
    DMA_InitStruct.DMA_MinorByteCnt          = 1;//ÿ�ζ���һ���ֽ�
    DMA_InitStruct.DMA_SourceAddr            = (uint32_t)&PTC->PDIR + 1;//Դ��ַ PTC0- C7
    DMA_InitStruct.DMA_DestAddr              = (uint32_t)Data_Buffer; //���ͼ�������  //uint8_t DMABuffer[400];
    DMA_InitStruct.DMA_DestAddrOffset        = 1;//Ŀ�ĵ�ַƫ��
    DMA_InitStruct.DMA_AutoDisableReq = TRUE; // �Զ���������
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

// OV7620����
void Camera_Config(void)
{
    GPIO_Config();
    DMA_Config();
    LPLD_SCCB_Init();
    LPLD_SCCB_WriteReg(0x12, 0x80);
    LPLD_SCCB_WriteReg(0x14, 0x20); //���÷ֱ��� QVGA 320*240
    enable_irq(PORTE_IRQn);         //ʹ�ܳ��ж�
    disable_irq(PORTD_IRQn);        //��ֹ���ж�
}
// ������������
void UART_Config(void)
{
    UART_InitTypeDef UART_InitStruct;
    UART_InitStruct.UART_Uartx    = UART1; //ʹ��UART1
    UART_InitStruct.UART_BaudRate = 115200;//���ò�����
    UART_InitStruct.UART_RxPin    = PTC3;  //��������
    UART_InitStruct.UART_TxPin    = PTC4;  //��������
    LPLD_UART_Init(UART_InitStruct);       //��ʼ��UART
}


void BoardInit(void)
{
    // �Ƚ�ֹ�жϣ���ֹ�жϴ��������쳣
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
