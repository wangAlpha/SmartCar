/**
 *-------------基于"拉普兰德K60底层库V3.1"的智能车工程-----------------
 * @file Camera.c
 * @version 0.0
 * @date 2017-04-26
 * @brief 赛道图像采集，赛道信息提取
 *
 * 开发者 湖南师范大学 飞思卡尔光电直立组 音速战神
 * 硬件平台 MK60DN512VLL10
 * @par None
**/
#include "common.h"
#include "DEV_SCCB.h"
#include "Camera.h"
#include "PIT.h"

uint8_t Data_Buffer[V][H] = {0};   //数据存放池
uint8_t Camera_Image[Hight][Width] = {0};  //数据二值化存放数组
bool BinArray[256] = {0};          //二值化数值，用于二值化运算提速
bool Is_DispPhoto = false;         //是否采集完一场
uint16_t V_Cnt = 0;                //行数计数器
uint8_t Threshold = 127;           //阈值，系统启动时设置

int16_t Speed_Buffer = 0;
/*----------------------------------------------------*/
int16_t L_Line_0[8] = {200, 200, 200, 200, 200, 200, 200, 200};
int16_t R_Line_0[8] = {56, 56, 56, 56, 56, 56, 56, 56};

int16_t L_Line_1[8] = {180, 180, 180, 180, 180, 180, 180, 180};
int16_t R_Line_1[8] = {76, 76, 76, 76, 76, 76, 76, 76};

int16_t Center_Line = 128;
int16_t Center_Line_0[8] = {128, 128, 128, 128, 128, 128, 128, 128};
int16_t Center_Line_1[8] = {128, 128, 128, 128, 128, 128, 128, 128};

/*----------------------赛道标志----------------------*/
uint8_t Track_Type = 0;
uint8_t Straight_Flag = 0; // 直道标志
uint8_t Obstacle_Flag = 0; //障碍物
uint8_t Obstacle_Counter = 0;
uint8_t Rampway_Counter = 0;
uint8_t Circle_Control = 0; //0优先向左，1优先向右
uint8_t Circle_Flag = 0;    // 圆环标志
uint16_t Circle_Counter = 0;

const uint16_t Point_0 = 18;//主线
const uint16_t Point_1 = 54; // 辅助线
const int16_t Half_Road_0 = 84 + 18;// 丢边补偿宽度，主线
const int16_t Half_Road_1 = 44 + 18;// 丢变补偿宽度，辅助线

// 求绝对值
int16_t Abs(int16_t x)
{
    return (x > 0) ? (x) : (-x);
}
// 场中断
void Field_IRQ(void)
{
    if (LPLD_GPIO_IsPinxExt(PORTE, GPIO_Pin6)) {
        LPLD_DMA_LoadDstAddr(DMA_CH0, &Data_Buffer[0][0]); // 检测开始信号，加载目的地址
        enable_irq(PORTD_IRQn);  //使能行中断
        disable_irq(PORTE_IRQn); //禁止场中断
    }
}

// 行中断
void Row_IRQ(void)
{
    // 检测是否是行中断
    if (LPLD_GPIO_IsPinxExt(PORTD, GPIO_Pin1)) {
        if (V_Cnt & 0x02 == 0x02) { // 该行是否采集
            LPLD_DMA_EnableReq(DMA_CH0); //循环采集一行
        }
        ++V_Cnt;
        if (V_Cnt > V * 3 + 0) {
            LPLD_DMA_DisableReq(DMA_CH0);
            disable_irq(PORTD_IRQn); // 禁用行中断
            disable_irq(PORTE_IRQn); // 禁用场中断
            Is_DispPhoto = true;     // 采集完一场
            V_Cnt = 0;               // 计数器清零
        }
    }
}
//大津算法 得到最佳阈值
void Ostu(void)
{
    // 生成灰度柱状图
    float histogram[256] = {0};
    for (uint32_t i = 0; i < V; ++i) {
        for (uint32_t j = 56 - 12; j < H - 12; ++j) {
            ++histogram[Data_Buffer[i][j]];
        }
    }
    uint32_t size = Hight * Width;
    float u = 0;// 整幅图的平均灰度
    for (uint32_t i = 0; i < 256; ++i) {
        histogram[i] = histogram[i] / size;
        u += i * histogram[i];
    }
    float maxVariance = 0, w0 = 0, avgValue = 0;
    for (uint32_t i = 0; i < 256; ++i) {
        w0 += histogram[i];  //前景灰度
        avgValue += i * histogram[i];
        float t = avgValue / w0 - u;
        float variance = t * t * w0 / (1 - w0); // g = w0/(1-w0)*(u0-u1)^2
        if (variance > maxVariance) { //找到使灰度差最大的值
            maxVariance = variance;
            Threshold = i;            //那个值就是阈值
        }
    }
    for (uint32_t i = Threshold; i < 256; ++i) { // 生成阈值数组，用于二值化加速
        *(BinArray + i) = true;
    }
}

//二值化 2ms
#pragma optimize=speed
void Image_Binary(void)
{
    uint8_t *p = &Camera_Image[0][0], (*p_d)[H] = Data_Buffer;
    bool *p_b = BinArray;                    // 指针访问数组可有效提升运算速度
    for (int32_t i = 0, y = 0; i < V; ++i) { // 二值化的同时进行尺寸裁剪
        for (int32_t j = 56 - 12; j < H - 12; j += 2) {
            *(p + y) = *(p_b + * (*(p_d + i) + j));
            *(p + y + 1) = *(p_b + * (*(p_d + i) + j + 1));
            y += 2;
        }
    }
}
uint8_t White_Flag = 0;
// 图像分析
void Image_Analysis(void)
{
    // 赛道圆环和十字的空白都为40cm，所以主线和辅助线的距离为40cm以上，以便检测
    // 假若以以一般速度跑完，40cm以内的线足矣
    // 主线
    int16_t old_center_line = Center_Line_0[0];
    for (int16_t i = old_center_line; i < Width; ++i) { //左边
        L_Line_0[0] = i;
        if ((Camera_Image[Point_0][i - 1] == true) && Camera_Image[Point_0][i] == false) {
            break;
        }
    }
    for (int16_t i = old_center_line; i >= 0; --i) { // 右边
        R_Line_0[0] = i;
        if ((Camera_Image[Point_0][i + 1] == true) && (Camera_Image[Point_0][i] == false)) {
            break;
        }
    }
    // 计算中线
    if ((L_Line_0[0] != Width - 1) && (R_Line_0 != 0)) { // 两边都有边
        Center_Line_0[0] = (L_Line_0[0] + R_Line_0[0]) >> 1;
    }
    else if ((L_Line_0[0] != Width - 1) && (R_Line_0[0] == 0)) { // 右边丢边
        Center_Line_0[0] = L_Line_0[0] - Half_Road_0;
    }
    else if ((L_Line_0[0] == Width - 1) && (R_Line_0[0] != 0)) { // 左边丢边
        Center_Line_0[0] = R_Line_0[0] + Half_Road_0;
    }
    else { // 全没边,可能是全黑和全白导致的
        Center_Line_0[0] = Center_Line_0[1];
    }
    // 辅助线
    old_center_line = Center_Line_1[0];
    for (int16_t i = old_center_line; i < Width; ++i) { // 左边
        L_Line_1[0] = i;
        if ((Camera_Image[Point_1][i - 1] == true) && (Camera_Image[Point_1][i] == false)) {
            break;
        }
    }
    for (int16_t i = old_center_line; i >= 0; --i) { //右边
        R_Line_1[0] = i;
        if ((Camera_Image[Point_1][i + 1] == true) && (Camera_Image[Point_1][i] == false)) {
            break;
        }
    }
    // 计算中线
    if ((L_Line_1[0] != Width - 1) && (R_Line_1[0] != 0)) {
        Center_Line_1[0] = (L_Line_1[0] + R_Line_1[0]) >> 1;
    }
    else if ((L_Line_1[0] != Width - 1) && (R_Line_1[0] == 0)) { // 右边丢边
        Center_Line_1[0] = L_Line_1[0] - Half_Road_1;
    }
    else if ((L_Line_1[0] == Width - 1) && (R_Line_1[0] != 0)) { // 左边丢边
        Center_Line_1[0] = R_Line_1[0] + Half_Road_1;
    }
    else { // 全没边
        Center_Line_1[0] = Center_Line_1[1];
    }
    Track_Type = 0;
    /*--------------------------圆环判定-----------------------------*/
    if ((L_Line_0[0] - R_Line_0[0]) > 200
        && (L_Line_0[0] - R_Line_0[0]) >= (L_Line_0[2] - R_Line_0[2])
        && (L_Line_0[2] - R_Line_0[2]) >= (L_Line_0[4] - R_Line_0[4])
        && (Camera_Image[Point_1][Width / 2] == false)) {
        if (Circle_Control == 0) { // 优先向右
            Center_Line_0[0] = R_Line_0[0] + Half_Road_0;
        }
        else { // 优先向左
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
    /*-----------------------------十字判定----------------------------*/
    /*先确定不是圆环，避免造成误判*/
    if (Track_Type == 0) {
        if (((L_Line_0[0] - R_Line_0[0]) > 180)
            && (R_Line_0[0] == 0) && (L_Line_0[0] == Width - 1)
            && (R_Line_0[1] == 0) && (L_Line_0[1] == Width - 1)) { //{
            if (R_Line_1[0] != 0 && L_Line_1[0] != Width - 1) { //辅助线有边
                Center_Line_0[0] = Center_Line_1[0];
            }
            else { // 辅助线没边
                Center_Line_0[0] = Center_Line_0[1];
            }
        }
    }
    /*-------------------直道判断--------------------------*/
    if (Track_Type != 1) {
        if ((Abs(Center_Line_0[0] - Center_Line_1[0]) < 25)
            && (Abs(Center_Line_0[2] - Center_Line_1[2]) < 25)
            && (Abs(Center_Line_0[4] - Center_Line_1[4] < 25))) { // 小S也判定为直道
            Straight_Flag = 1;
        }
        else {
            Straight_Flag = 0;
        }
    }
    /*-------------------------------障碍物检测----------------------------*/
    if (Track_Type == 0) {
      if (((L_Line_0[1] - R_Line_0[1]) < 210) && ((L_Line_0[1] - R_Line_0[1]) > 140)//R:130
            && ((L_Line_0[0] - R_Line_0[0]) < 140) && ((L_Line_0[0] - R_Line_0[0]) > 80)) { //直线赛道宽度大概150左右，有障碍物100左右
            if (R_Line_0[0] > 70) {
                Obstacle_Flag = 1;  //右边有障碍物
            }
            if (L_Line_0[0] < 185) {
                Obstacle_Flag = 2;//左边有障碍物
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
    /*----------------------------开始线检测------------------------------*/
    if (Start_Flag == 2) {
        if ((L_Line_0[0] - R_Line_0[0]) < 50 || (L_Line_0[2] - R_Line_0[2] < 50)) {
            uint32_t counter = 0; // 跳变计数
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
    /*-------------------------中心线限幅----------------------------------*/
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
    /*-----------------------------存中线和边------------------------------*/
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
