/**
 *-------------����"��������K60�ײ��V3.1"�����ܳ�����-----------------
 * @file Camera.c
 * @version 0.0
 * @date 2017-04-26
 * @brief ����ͼ��ɼ���������Ϣ��ȡ
 *
 * ������ ����ʦ����ѧ ��˼�������ֱ���� ����ս��
 * Ӳ��ƽ̨ MK60DN512VLL10
 * @par None
**/
#include "common.h"
#include "DEV_SCCB.h"
#include "Camera.h"
#include "PIT.h"

uint8_t Data_Buffer[V][H] = {0};   //���ݴ�ų�
uint8_t Camera_Image[Hight][Width] = {0};  //���ݶ�ֵ���������
bool BinArray[256] = {0};          //��ֵ����ֵ�����ڶ�ֵ����������
bool Is_DispPhoto = false;         //�Ƿ�ɼ���һ��
uint16_t V_Cnt = 0;                //����������
uint8_t Threshold = 127;           //��ֵ��ϵͳ����ʱ����

int16_t Speed_Buffer = 0;
/*----------------------------------------------------*/
int16_t L_Line_0[8] = {200, 200, 200, 200, 200, 200, 200, 200};
int16_t R_Line_0[8] = {56, 56, 56, 56, 56, 56, 56, 56};

int16_t L_Line_1[8] = {180, 180, 180, 180, 180, 180, 180, 180};
int16_t R_Line_1[8] = {76, 76, 76, 76, 76, 76, 76, 76};

int16_t Center_Line = 128;
int16_t Center_Line_0[8] = {128, 128, 128, 128, 128, 128, 128, 128};
int16_t Center_Line_1[8] = {128, 128, 128, 128, 128, 128, 128, 128};

/*----------------------������־----------------------*/
uint8_t Track_Type = 0;
uint8_t Straight_Flag = 0; // ֱ����־
uint8_t Obstacle_Flag = 0; //�ϰ���
uint8_t Obstacle_Counter = 0;
uint8_t Rampway_Counter = 0;
uint8_t Circle_Control = 0; //0��������1��������
uint8_t Circle_Flag = 0;    // Բ����־
uint16_t Circle_Counter = 0;

const uint16_t Point_0 = 18;//����
const uint16_t Point_1 = 54; // ������
const int16_t Half_Road_0 = 84 + 18;// ���߲�����ȣ�����
const int16_t Half_Road_1 = 44 + 18;// ���䲹����ȣ�������

// �����ֵ
int16_t Abs(int16_t x)
{
    return (x > 0) ? (x) : (-x);
}
// ���ж�
void Field_IRQ(void)
{
    if (LPLD_GPIO_IsPinxExt(PORTE, GPIO_Pin6)) {
        LPLD_DMA_LoadDstAddr(DMA_CH0, &Data_Buffer[0][0]); // ��⿪ʼ�źţ�����Ŀ�ĵ�ַ
        enable_irq(PORTD_IRQn);  //ʹ�����ж�
        disable_irq(PORTE_IRQn); //��ֹ���ж�
    }
}

// ���ж�
void Row_IRQ(void)
{
    // ����Ƿ������ж�
    if (LPLD_GPIO_IsPinxExt(PORTD, GPIO_Pin1)) {
        if (V_Cnt & 0x02 == 0x02) { // �����Ƿ�ɼ�
            LPLD_DMA_EnableReq(DMA_CH0); //ѭ���ɼ�һ��
        }
        ++V_Cnt;
        if (V_Cnt > V * 3 + 0) {
            LPLD_DMA_DisableReq(DMA_CH0);
            disable_irq(PORTD_IRQn); // �������ж�
            disable_irq(PORTE_IRQn); // ���ó��ж�
            Is_DispPhoto = true;     // �ɼ���һ��
            V_Cnt = 0;               // ����������
        }
    }
}
//����㷨 �õ������ֵ
void Ostu(void)
{
    // ���ɻҶ���״ͼ
    float histogram[256] = {0};
    for (uint32_t i = 0; i < V; ++i) {
        for (uint32_t j = 56 - 12; j < H - 12; ++j) {
            ++histogram[Data_Buffer[i][j]];
        }
    }
    uint32_t size = Hight * Width;
    float u = 0;// ����ͼ��ƽ���Ҷ�
    for (uint32_t i = 0; i < 256; ++i) {
        histogram[i] = histogram[i] / size;
        u += i * histogram[i];
    }
    float maxVariance = 0, w0 = 0, avgValue = 0;
    for (uint32_t i = 0; i < 256; ++i) {
        w0 += histogram[i];  //ǰ���Ҷ�
        avgValue += i * histogram[i];
        float t = avgValue / w0 - u;
        float variance = t * t * w0 / (1 - w0); // g = w0/(1-w0)*(u0-u1)^2
        if (variance > maxVariance) { //�ҵ�ʹ�ҶȲ�����ֵ
            maxVariance = variance;
            Threshold = i;            //�Ǹ�ֵ������ֵ
        }
    }
    for (uint32_t i = Threshold; i < 256; ++i) { // ������ֵ���飬���ڶ�ֵ������
        *(BinArray + i) = true;
    }
}

//��ֵ�� 2ms
#pragma optimize=speed
void Image_Binary(void)
{
    uint8_t *p = &Camera_Image[0][0], (*p_d)[H] = Data_Buffer;
    bool *p_b = BinArray;                    // ָ������������Ч���������ٶ�
    for (int32_t i = 0, y = 0; i < V; ++i) { // ��ֵ����ͬʱ���гߴ�ü�
        for (int32_t j = 56 - 12; j < H - 12; j += 2) {
            *(p + y) = *(p_b + * (*(p_d + i) + j));
            *(p + y + 1) = *(p_b + * (*(p_d + i) + j + 1));
            y += 2;
        }
    }
}
uint8_t White_Flag = 0;
// ͼ�����
void Image_Analysis(void)
{
    // ����Բ����ʮ�ֵĿհ׶�Ϊ40cm���������ߺ͸����ߵľ���Ϊ40cm���ϣ��Ա���
    // ��������һ���ٶ����꣬40cm���ڵ�������
    // ����
    int16_t old_center_line = Center_Line_0[0];
    for (int16_t i = old_center_line; i < Width; ++i) { //���
        L_Line_0[0] = i;
        if ((Camera_Image[Point_0][i - 1] == true) && Camera_Image[Point_0][i] == false) {
            break;
        }
    }
    for (int16_t i = old_center_line; i >= 0; --i) { // �ұ�
        R_Line_0[0] = i;
        if ((Camera_Image[Point_0][i + 1] == true) && (Camera_Image[Point_0][i] == false)) {
            break;
        }
    }
    // ��������
    if ((L_Line_0[0] != Width - 1) && (R_Line_0 != 0)) { // ���߶��б�
        Center_Line_0[0] = (L_Line_0[0] + R_Line_0[0]) >> 1;
    }
    else if ((L_Line_0[0] != Width - 1) && (R_Line_0[0] == 0)) { // �ұ߶���
        Center_Line_0[0] = L_Line_0[0] - Half_Road_0;
    }
    else if ((L_Line_0[0] == Width - 1) && (R_Line_0[0] != 0)) { // ��߶���
        Center_Line_0[0] = R_Line_0[0] + Half_Road_0;
    }
    else { // ȫû��,������ȫ�ں�ȫ�׵��µ�
        Center_Line_0[0] = Center_Line_0[1];
    }
    // ������
    old_center_line = Center_Line_1[0];
    for (int16_t i = old_center_line; i < Width; ++i) { // ���
        L_Line_1[0] = i;
        if ((Camera_Image[Point_1][i - 1] == true) && (Camera_Image[Point_1][i] == false)) {
            break;
        }
    }
    for (int16_t i = old_center_line; i >= 0; --i) { //�ұ�
        R_Line_1[0] = i;
        if ((Camera_Image[Point_1][i + 1] == true) && (Camera_Image[Point_1][i] == false)) {
            break;
        }
    }
    // ��������
    if ((L_Line_1[0] != Width - 1) && (R_Line_1[0] != 0)) {
        Center_Line_1[0] = (L_Line_1[0] + R_Line_1[0]) >> 1;
    }
    else if ((L_Line_1[0] != Width - 1) && (R_Line_1[0] == 0)) { // �ұ߶���
        Center_Line_1[0] = L_Line_1[0] - Half_Road_1;
    }
    else if ((L_Line_1[0] == Width - 1) && (R_Line_1[0] != 0)) { // ��߶���
        Center_Line_1[0] = R_Line_1[0] + Half_Road_1;
    }
    else { // ȫû��
        Center_Line_1[0] = Center_Line_1[1];
    }
    Track_Type = 0;
    /*--------------------------Բ���ж�-----------------------------*/
    if ((L_Line_0[0] - R_Line_0[0]) > 200
        && (L_Line_0[0] - R_Line_0[0]) >= (L_Line_0[2] - R_Line_0[2])
        && (L_Line_0[2] - R_Line_0[2]) >= (L_Line_0[4] - R_Line_0[4])
        && (Camera_Image[Point_1][Width / 2] == false)) {
        if (Circle_Control == 0) { // ��������
            Center_Line_0[0] = R_Line_0[0] + Half_Road_0;
        }
        else { // ��������
            Center_Line_0[0] = L_Line_0[0] - Half_Road_0;
        }
        Track_Type = 1;
        Circle_Flag = 1;
    }
    if (Circle_Flag == 1) {
        if (++Circle_Counter >= 100) {
            Circle_Counter = 0;
            Circle_Flag = 0;
        }
    }
    /*-----------------------------ʮ���ж�----------------------------*/
    /*��ȷ������Բ���������������*/
    if (Track_Type == 0) {
        if (((L_Line_0[0] - R_Line_0[0]) > 180)
            && (R_Line_0[0] == 0) && (L_Line_0[0] == Width - 1)
            && (R_Line_0[1] == 0) && (L_Line_0[1] == Width - 1)) { //{
            if (R_Line_1[0] != 0 && L_Line_1[0] != Width - 1) { //�������б�
                Center_Line_0[0] = Center_Line_1[0];
            }
            else { // ������û��
                Center_Line_0[0] = Center_Line_0[1];
            }
        }
    }
    /*-------------------ֱ���ж�--------------------------*/
    if (Track_Type != 1) {
        if ((Abs(Center_Line_0[0] - Center_Line_1[0]) < 25)
            && (Abs(Center_Line_0[2] - Center_Line_1[2]) < 25)
            && (Abs(Center_Line_0[4] - Center_Line_1[4] < 25))) { // СSҲ�ж�Ϊֱ��
            Straight_Flag = 1;
        }
        else {
            Straight_Flag = 0;
        }
    }
    /*-------------------------------�ϰ�����----------------------------*/
    if (Track_Type == 0) {
      if (((L_Line_0[1] - R_Line_0[1]) < 210) && ((L_Line_0[1] - R_Line_0[1]) > 140)//R:130
            && ((L_Line_0[0] - R_Line_0[0]) < 140) && ((L_Line_0[0] - R_Line_0[0]) > 80)) { //ֱ��������ȴ��150���ң����ϰ���100����
            if (R_Line_0[0] > 70) {
                Obstacle_Flag = 1;  //�ұ����ϰ���
            }
            if (L_Line_0[0] < 185) {
                Obstacle_Flag = 2;//������ϰ���
            }
        }
    }
    if (Obstacle_Flag != 0) {
        Obstacle_Counter++;
        if (Obstacle_Flag == 1) {
            if (Obstacle_Counter < 11) {//18
                Center_Line_0[0] = L_Line_0[0] - 45;//38;//42;    //38;|1000
            }
            else {
                Center_Line_0[0] = L_Line_0[0] - 50;    //43;//35
            }
        }
        else if (Obstacle_Flag == 2) {
            if (Obstacle_Counter < 11) {//18
                Center_Line_0[0] = R_Line_0[0] + 32;    //32;    //37;|1000
            }
            else {
                Center_Line_0[0] = R_Line_0[0] + 52;//40;    //50;
            }
        }
        if (Obstacle_Counter > 35) {//25
            Obstacle_Counter = 0;
            Obstacle_Flag = 0;
            Track_Type = 0;
        }
    }
    /*----------------------------��ʼ�߼��------------------------------*/
    if (Start_Flag == 2) {
        if ((L_Line_0[0] - R_Line_0[0]) < 50 || (L_Line_0[2] - R_Line_0[2] < 50)) {
            uint32_t counter = 0; // �������
            for (uint32_t i = 0; i < Width - 2; ++i) {
                if ((Camera_Image[Point_0][i] == true) && Camera_Image[Point_0][i + 1] == false) {
                    ++counter;
                }
            }
            if (counter >= 4) {
                Start_Flag = 3;
            }
        }
    }
    /*-------------------------�������޷�----------------------------------*/
    if (Center_Line_0[0] > Width - 2) {
        Center_Line_0[0] = Width - 2;
    }
    else if (Center_Line_0[0] < 1) {
        Center_Line_0[0] = 1;
    }
    if (Center_Line_1[0] > Width - 2) {
        Center_Line_1[0] = Width - 2;
    }
    else if (Center_Line_1[0] < 1) {
        Center_Line_1[0] = 1;
    }
    /*-----------------------------�����ߺͱ�------------------------------*/
    for (int32_t i = 7; i > 0; --i) {
        R_Line_0[i] = R_Line_0[i - 1];
        L_Line_0[i] = L_Line_0[i - 1];
        R_Line_1[i] = R_Line_1[i - 1];
        L_Line_1[i] = L_Line_1[i - 1];
        Center_Line_0[i] = Center_Line_0[i - 1];
        Center_Line_1[i] = Center_Line_1[i - 1];
    }
    int32_t counter = 0;
    for (int32_t i = 0; i < Width - 1; ++i) {
        if ((Camera_Image[Point_1][i] == true)) {
            ++counter;
        }
    }
    if (counter > 225) {
        Center_Line = Center_Line_0[0];
    }
    else {
        Center_Line = (Center_Line_0[0] + Center_Line_0[1] + Center_Line_0[2] +  Center_Line_1[0]) >> 2;
    }
}
