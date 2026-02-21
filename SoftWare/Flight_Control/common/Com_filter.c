#include "Com_filter.h"

// 一阶低通滤波系数  值越小  低通滤波效果越强
#define ALPHA 0.15f /* 一阶低通滤波器 指数加权系数 */

/**
 * @brief 一阶低通滤波
 * 一种常用的滤波器，用于去除高频噪声，保留信号中的低频成分。
 * 在单片机应用中，一种简单常用的低通滤波器是一阶指数加权移动平均滤波（IIR 低通滤波器）
 * 通常实现为指数加权移动平均滤波器。
 * @param newValue 需要滤波的值
 * @param preFilteredValue 上一次滤波后的值
 * @return 滤波后的值
 */
int16_t Common_Filter_LowPass(int16_t newValue, int16_t preFilteredValue)
{
    return (int16_t)(ALPHA * newValue + (1 - ALPHA) * preFilteredValue);
}

/* 卡尔曼滤波 https://www.mwrf.net/tech/basic/2023/30081.html
 https://www.kalmanfilter.net/CN/default_cn.aspx*/

/* 卡尔曼滤波器数组 */
KalmanFilter_Struct kfs[3] = {
    {0.02f, 0, 0, 0, 0.001f, 0.543f},
    {0.02f, 0, 0, 0, 0.001f, 0.543f},
    {0.02f, 0, 0, 0, 0.001f, 0.543f}};

/**
 * @brief 卡尔曼滤波
 * @param kf 卡尔曼滤波结构体指针
 * @param input 输入值
 * @return 滤波后的值
 */
double Common_Filter_KalmanFilter(KalmanFilter_Struct *kf, double input)
{
    kf->Now_P = kf->LastP + kf->Q;
    kf->Kg = kf->Now_P / (kf->Now_P + kf->R);
    kf->out = kf->out + kf->Kg * (input - kf->out);
    kf->LastP = (1 - kf->Kg) * kf->Now_P;
    return kf->out;
}
