#ifndef __INT_MOTOR__
#define __INT_MOTOR__

#include "tim.h"
#include "Com_debug.h"

typedef struct
{
    TIM_HandleTypeDef *tim;
    uint16_t channel;
    uint16_t speed;
} Motor_Struct;

/**
 * @brief 设置电机速度（实际是比较值，最大为 1000，默认值为 200）
 *
 * @param motor 电机结构体指针
 */
void Int_motor_set_speed(Motor_Struct *motor);

/**
 * @brief 启动电机，传入电机的结构体
 *
 * @param motor 电机结构体指针
 */
void Int_motor_start(Motor_Struct *motor);

/**
 * @brief 电机启动流程：50% 占空比启动，持续指定时间后调整到目标速度
 *
 * @param motor 电机结构体指针
 * @param target_speed 目标速度值
 * @param startup_time_ms 启动时间（毫秒），建议 3000ms 以上
 */
void Int_motor_startup(Motor_Struct *motor, uint16_t target_speed, uint32_t startup_time_ms);

#endif // __INT_MOTOR__
