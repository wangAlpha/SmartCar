/**
 * @file DEV_MPU6050.c
 * @version 0.1[By LPLD]
 * @date 2015-02-22
 * @brief MPU60506轴运动处理组件驱动程序
 *
 * 更改建议:可根据实际硬件修改
 *
 * 十分感谢该文件作者对底层库的贡献！
 * http://www.lpld.cn
 * mail:support@lpld.cn
 *
 * 文件出处：LPLD_K60底层库群
 * 作者：投笔从戎
 */
#include "common.h"
#include "DEV_MPU6050.h"

/*
 *   MPU6050_WriteReg
 *   MPU6050写寄存器状态
 *
 *   参数：
 *    RegisterAddress 寄存器地址
 *    Data 所需要写得内容
*
 *   返回值
 *    无
 */
void MPU6050_WriteReg(uint8_t RegisterAddress, uint8_t Data)
{
    //发送从机地址
    LPLD_I2C_StartTrans(I2C0, SlaveAddress, I2C_MWSR);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //写MPU6050寄存器地址
    LPLD_I2C_WriteByte(I2C0, RegisterAddress);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //向寄存器中写具体数据
    LPLD_I2C_WriteByte(I2C0, Data);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    LPLD_I2C_Stop(I2C0);
    LPLD_SYSTICK_DelayUs(5);
}

/*
 *   MPU6050_ReadReg
 *   MPU6050读寄存器状态
 *
 *   参数：
 *    RegisterAddress 寄存器地址
 *
 *   返回值
 *    所读寄存器状态
 */
uint8_t MPU6050_ReadReg(uint8_t RegisterAddress)
{
    uint8_t result;
    //发送从机地址
    LPLD_I2C_StartTrans(I2C0, SlaveAddress, I2C_MWSR);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //写MPU6050寄存器地址
    LPLD_I2C_WriteByte(I2C0, RegisterAddress);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //再次产生开始信号
    LPLD_I2C_ReStart(I2C0);
    //发送从机地址和读取位
    LPLD_I2C_WriteByte(I2C0, (SlaveAddress << 1 | I2C_MRSW));
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //转换主机模式为读
    LPLD_I2C_SetMasterWR(I2C0, I2C_MRSW);
    //关闭应答ACK
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_OFF);//关闭ACK
    //读IIC数据
    result = LPLD_I2C_ReadByte(I2C0);
    LPLD_I2C_WaitAck(I2C0, I2C_ACK_ON);
    //发送停止信号
    LPLD_I2C_Stop(I2C0);
    //读IIC数据
    result = LPLD_I2C_ReadByte(I2C0);
    LPLD_SYSTICK_DelayUs(3);
    return result;
}

/*
 *   MPU6050_GetResult
 *   获得MPU6050结果
 *
 *   参数：
 *    无
 *
 *   返回值
 *    转换结果
 */
int16_t MPU6050_GetResult(uint8_t Regs_Addr)
{
    return (MPU6050_ReadReg(Regs_Addr) << 8 | MPU6050_ReadReg(Regs_Addr + 1));
}

/*
 *   MPU6050_Init
 *   初始化MPU6050，包括初始化MPU6050所需的I2C接口以及MPU6050的寄存器
 *
 *   参数：
 *    无
 *
 *   返回值
 *    无
 */
void MPU6050_Init()
{
    //初始化MPU6050
    I2C_InitTypeDef MPU6050_init_struct;
    MPU6050_init_struct.I2C_I2Cx   = I2C0;
    MPU6050_init_struct.I2C_IntEnable = FALSE;
    MPU6050_init_struct.I2C_ICR    = MPU6050_SCL_400KHZ;
    MPU6050_init_struct.I2C_SclPin = MPU6050_SCLPIN;
    MPU6050_init_struct.I2C_SdaPin = MPU6050_SDAPIN;
    MPU6050_init_struct.I2C_OpenDrainEnable = TRUE;
    MPU6050_init_struct.I2C_Isr    = NULL;
    LPLD_I2C_Init(MPU6050_init_struct);
    // 注意，采样率必须为IIC采样率的2倍！使用时低通滤波造成的延时需要考虑进去
    LPLD_SYSTICK_DelayMs(50);
    MPU6050_WriteReg(PWR_MGMT_1, 0x80);   //解除休眠状态，所有寄存器值自动归为默认值
    LPLD_SYSTICK_DelayMs(50);             //上电需要一些时间
    MPU6050_WriteReg(PWR_MGMT_1, 0x00);   //设置时钟源为内部8MHz时钟
    MPU6050_WriteReg(SMPLRT_DIV, 0x00);   //陀螺仪采样率，1kHz SMPLRT_DIV = 1000/500 - 1
    MPU6050_WriteReg(CONFIG, 0x03);       //低通滤波频率 98Hz 0x03 4.9ms延时
    MPU6050_WriteReg(GYRO_CONFIG, 0x01);  //陀螺仪自检及测量范围，(不自检，500deg/s) 0x18
    MPU6050_WriteReg(ACCEL_CONFIG, 0x00); //加速计不自检， 范围2g
}
