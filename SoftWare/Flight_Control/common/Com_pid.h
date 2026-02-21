#ifndef __COM_PID__
#define __COM_PID__

#include "main.h"

#define PID_PERIOD 0.006f

// PID 结构体   如果 CPU 性能足够强大  建议使用 double
// kp,ki,kd 需要在初始化时确定   目标值和测量值  需要在计算时赋值
typedef struct
{
    float kp;        // 比例系数  值越大响应速度越快
    float ki;        // 积分系数  消除稳态误差  防止积分饱和  一般不使用
    float kd;        // 微分系数  值越大 阻尼效果越强  抑制超调
    float err;       // 误差值
    float desire;    // 目标值
    float measure;   // 测量值
    float last_err;  // 上一次的误差
    float integral;  // 误差累积
    float output;    // 输出值
} PID_Struct;

// 单次 PID 计算
void Com_PID_Calc(PID_Struct *pid);

// 串级 PID 计算
void Com_PID_Calc_Chain(PID_Struct *out_pid, PID_Struct *in_pid);

/**
 * @brief 限制值在指定的范围内
 *
 * @param speed
 * @param max_speed
 * @param min_speed
 * @return int16_t
 */
int16_t Com_limit(int16_t speed, int16_t max_speed, int16_t min_speed);

#endif // __COM_PID__
