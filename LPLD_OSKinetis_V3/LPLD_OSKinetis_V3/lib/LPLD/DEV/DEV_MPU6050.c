/**
 * @file DEV_MPU6050.c
 * @version 0.1[By LPLD]
 * @date 2015-02-22
 * @brief MPU60506���˶����������������
 *
 * ���Ľ���:�ɸ���ʵ��Ӳ���޸�
 *
 * ʮ�ָ�л���ļ����߶Եײ��Ĺ��ף�
 * http://www.lpld.cn
 * mail:support@lpld.cn
 *
 * �ļ�������LPLD_K60�ײ��Ⱥ
 * ���ߣ�Ͷ�ʴ���
 */
#include "common.h"
#include "DEV_MPU6050.h"

/*
 *   MPU6050_WriteReg
 *   MPU6050д�Ĵ���״̬
 *
 *   ������
 *    RegisterAddress �Ĵ�����ַ
 *    Data ����Ҫд������
*
 *   ����ֵ
 *    ��
 */
void MPU6050_WriteReg(uint8_t RegisterAddress, uint8_t Data)
{
    //���ʹӻ���ַ
    LPLD_I2C_StartTrans(I2C0, SlaveAddress, I2C_MWSR);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //дMPU6050�Ĵ�����ַ
    LPLD_I2C_WriteByte(I2C0, RegisterAddress);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //��Ĵ�����д��������
    LPLD_I2C_WriteByte(I2C0, Data);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    LPLD_I2C_Stop(I2C0);
    LPLD_SYSTICK_DelayUs(5);
}

/*
 *   MPU6050_ReadReg
 *   MPU6050���Ĵ���״̬
 *
 *   ������
 *    RegisterAddress �Ĵ�����ַ
 *
 *   ����ֵ
 *    �����Ĵ���״̬
 */
uint8_t MPU6050_ReadReg(uint8_t RegisterAddress)
{
    uint8_t result;
    //���ʹӻ���ַ
    LPLD_I2C_StartTrans(I2C0, SlaveAddress, I2C_MWSR);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //дMPU6050�Ĵ�����ַ
    LPLD_I2C_WriteByte(I2C0, RegisterAddress);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //�ٴβ�����ʼ�ź�
    LPLD_I2C_ReStart(I2C0);
    //���ʹӻ���ַ�Ͷ�ȡλ
    LPLD_I2C_WriteByte(I2C0, (SlaveAddress << 1 | I2C_MRSW));
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //ת������ģʽΪ��
    LPLD_I2C_SetMasterWR(I2C0, I2C_MRSW);
    //�ر�Ӧ��ACK
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_OFF);//�ر�ACK
    //��IIC����
    result = LPLD_I2C_ReadByte(I2C0);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //����ֹͣ�ź�
    LPLD_I2C_Stop(I2C0);
    //��IIC����
    result = LPLD_I2C_ReadByte(I2C0);
    LPLD_SYSTICK_DelayUs(3);
    return result;
}

/*
 *   MPU6050_GetResult
 *   ���MPU6050���
 *
 *   ������
 *    ��
 *
 *   ����ֵ
 *    ת�����
 */
int16_t MPU6050_GetResult(uint8_t Regs_Addr)
{
    return (MPU6050_ReadReg(Regs_Addr) << 8 | MPU6050_ReadReg(Regs_Addr + 1));
}

/*
 *   MPU6050_Init
 *   ��ʼ��MPU6050��������ʼ��MPU6050�����I2C�ӿ��Լ�MPU6050�ļĴ���
 *
 *   ������
 *    ��
 *
 *   ����ֵ
 *    ��
 */
void MPU6050_Init()
{
    //��ʼ��MPU6050
    I2C_InitTypeDef MPU6050_init_struct;
    MPU6050_init_struct.I2C_I2Cx   = I2C0;
    MPU6050_init_struct.I2C_IntEnable = FALSE;
    MPU6050_init_struct.I2C_ICR    = MPU6050_SCL_400KHZ;
    MPU6050_init_struct.I2C_SclPin = MPU6050_SCLPIN;
    MPU6050_init_struct.I2C_SdaPin = MPU6050_SDAPIN;
    MPU6050_init_struct.I2C_OpenDrainEnable = TRUE;
    MPU6050_init_struct.I2C_Isr    = NULL;
    LPLD_I2C_Init(MPU6050_init_struct);
    // ע�⣬�����ʱ���ΪIIC�����ʵ�2����ʹ��ʱ��ͨ�˲���ɵ���ʱ��Ҫ���ǽ�ȥ
    LPLD_SYSTICK_DelayMs(50);
    MPU6050_WriteReg(PWR_MGMT_1, 0x80);   //�������״̬�����мĴ���ֵ�Զ���ΪĬ��ֵ
    LPLD_SYSTICK_DelayMs(50);             //�ϵ���ҪһЩʱ��
    MPU6050_WriteReg(PWR_MGMT_1, 0x00);   //����ʱ��ԴΪ�ڲ�8MHzʱ��
    MPU6050_WriteReg(SMPLRT_DIV, 0x00);   //�����ǲ����ʣ�1kHz SMPLRT_DIV = 1000/500 - 1
    MPU6050_WriteReg(CONFIG, 0x03);       //��ͨ�˲�Ƶ�� 98Hz 0x03 4.9ms��ʱ
    MPU6050_WriteReg(GYRO_CONFIG, 0x01);  //�������Լ켰������Χ��(���Լ죬500deg/s) 0x18
    MPU6050_WriteReg(ACCEL_CONFIG, 0x00); //���ټƲ��Լ죬 ��Χ2g
}
