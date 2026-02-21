#include "Init.h"
#include "../interface/Int_led.h"
#include "../interface/Int_motor.h"
#include "FreeRTOS.h"
#include "main.h"
#include "task.h"
#include "tim.h"

// 定义四个电机结构体
Motor_Struct left_top_motor = {
    .tim = &htim3, .channel = TIM_CHANNEL_1, .speed = 200};
Motor_Struct left_bottom_motor = {
    .tim = &htim4, .channel = TIM_CHANNEL_4, .speed = 200};
Motor_Struct right_top_motor = {
    .tim = &htim2, .channel = TIM_CHANNEL_2, .speed = 200};
Motor_Struct right_bottom_motor = {
    .tim = &htim1, .channel = TIM_CHANNEL_3, .speed = 200};

LED_Struct left_top_led = {.port = LED1_GPIO_Port, .pin = LED1_Pin};
LED_Struct right_top_led = {.port = LED2_GPIO_Port, .pin = LED2_Pin};
LED_Struct right_bottom_led = {.port = LED3_GPIO_Port, .pin = LED3_Pin};
LED_Struct left_bottom_led = {.port = LED4_GPIO_Port, .pin = LED4_Pin};

/**
 * @brief 系统初始化函数
 *        先开启四个通道的 PWM 进行电调校准，延时 3500ms 后设置目标速度
 *        注意：此函数需在 FreeRTOS 任务中调用，使用 vTaskDelay 延时
 */
void System_Init(void) {
  debug_printf("System_Init: Starting ESC calibration...\n");

  // 1. 启动所有四个电机的 PWM 输出（50% 占空比用于电调校准）
  Int_motor_start(&left_top_motor);
  Int_motor_start(&left_bottom_motor);
  Int_motor_start(&right_top_motor);
  Int_motor_start(&right_bottom_motor);

  // 2. 设置 50% 占空比（ARR=2000 时，50% 为 1000）
  __HAL_TIM_SET_COMPARE(left_top_motor.tim, left_top_motor.channel, 1000);
  __HAL_TIM_SET_COMPARE(left_bottom_motor.tim, left_bottom_motor.channel, 1000);
  __HAL_TIM_SET_COMPARE(right_top_motor.tim, right_top_motor.channel, 1000);
  __HAL_TIM_SET_COMPARE(right_bottom_motor.tim, right_bottom_motor.channel,
                        1000);

  debug_printf("System_Init: Calibrating ESC, please wait 3.5s...\n");

  // 3. 延时 3500ms 等待电调校准完成（使用 FreeRTOS 延时）
  vTaskDelay(pdMS_TO_TICKS(3500));

  // 4. 设置所有电机的目标速度
  left_top_motor.speed = 400;
  left_bottom_motor.speed = 400;
  right_top_motor.speed = 400;
  right_bottom_motor.speed = 400;
  Int_motor_set_speed(&left_top_motor);
  Int_motor_set_speed(&left_bottom_motor);
  Int_motor_set_speed(&right_top_motor);
  Int_motor_set_speed(&right_bottom_motor);

  debug_printf("System_Init: ESC calibration complete!\n");
}
