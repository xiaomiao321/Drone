#include "Int_motor.h"
#include "FreeRTOS.h"
#include "task.h"
/**
 * @brief 设置电机速度（实际是比较值，最大为 1000，默认值为 200）
 *
 * @param motor 电机结构体指针
 */
void Int_motor_set_speed(Motor_Struct *motor) {
  if (motor->speed > 2000) {
    debug_printf("motor speed is too big\r\n");
    return;
  }

  __HAL_TIM_SET_COMPARE(motor->tim, motor->channel, motor->speed);
}

/**
 * @brief 启动电机，传入电机的结构体
 *
 * @param motor 电机结构体指针
 */
void Int_motor_start(Motor_Struct *motor) {
  HAL_TIM_PWM_Start(motor->tim, motor->channel);
}

/**
 * @brief 电机启动流程：50% 占空比启动，持续指定时间后调整到目标速度
 *        电调（ESC）启动流程：先输出 50% 占空比 PWM 持续 3 秒以上进行校准，
 *        然后再调整占空比改变转速
 *
 * @param motor 电机结构体指针
 * @param target_speed 目标速度值
 * @param startup_time_ms 启动时间（毫秒），建议 3000ms 以上
 */
void Int_motor_startup(Motor_Struct *motor, uint16_t target_speed,
                       uint32_t startup_time_ms) {
  // 1. 启动 PWM 输出
  HAL_TIM_PWM_Start(motor->tim, motor->channel);

  // 2. 设置 50% 占空比（ARR=2000 时，50% 为 1000）
  __HAL_TIM_SET_COMPARE(motor->tim, motor->channel, 1000);

  // 3. 延时等待电调校准完成（建议 3 秒以上）
  // 注意：此函数在任务中调用，使用 vTaskDelay 而非 HAL_Delay
  vTaskDelay(startup_time_ms / portTICK_PERIOD_MS);

  // 4. 设置目标速度
  motor->speed = target_speed;
  Int_motor_set_speed(motor);
}
