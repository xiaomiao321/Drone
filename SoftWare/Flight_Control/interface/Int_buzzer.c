#include "Int_buzzer.h"
#include "FreeRTOS.h"
#include "task.h"
/**
 * @brief 初始化蜂鸣器
 */
void Int_buzzer_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // 使能 GPIOB 时钟
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // 配置 PB0 为推挽输出
  GPIO_InitStruct.Pin = BUZZER_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);

  // 初始化时关闭蜂鸣器
  BUZZER_OFF();
}

/**
 * @brief 蜂鸣器开启
 */
void Int_buzzer_on(void) { BUZZER_ON(); }

/**
 * @brief 蜂鸣器关闭
 */
void Int_buzzer_off(void) { BUZZER_OFF(); }

/**
 * @brief 蜂鸣器翻转
 */
void Int_buzzer_toggle(void) { BUZZER_TOGGLE(); }

/**
 * @brief 短蜂鸣 (100ms)
 */
void Int_buzzer_short_beep(void) {
  Int_buzzer_on();
  HAL_Delay(100);
  Int_buzzer_off();
  HAL_Delay(50);
}

/**
 * @brief 长蜂鸣 (500ms)
 */
void Int_buzzer_long_beep(void) {
  Int_buzzer_on();
  vTaskDelay(pdMS_TO_TICKS(500));
  Int_buzzer_off();
}

/**
 * @brief 双蜂鸣 (2 次短蜂鸣)
 */
void Int_buzzer_double_beep(void) {
  Int_buzzer_short_beep();
  vTaskDelay(pdMS_TO_TICKS(100));
  Int_buzzer_short_beep();
}

/**
 * @brief 三蜂鸣 (3 次短蜂鸣，用于错误提示)
 */
void Int_buzzer_triple_beep(void) {
  Int_buzzer_short_beep();
  vTaskDelay(pdMS_TO_TICKS(100));
  Int_buzzer_short_beep();
  vTaskDelay(pdMS_TO_TICKS(100));
  Int_buzzer_short_beep();
}

/**
 * @brief 连续蜂鸣 (用于失控报警，需要在循环中调用)
 *        调用一次产生 100ms 蜂鸣 + 100ms 间隔
 */
void Int_buzzer_alarm(void) {
  Int_buzzer_on();
  vTaskDelay(pdMS_TO_TICKS(100));
  Int_buzzer_off();
  vTaskDelay(pdMS_TO_TICKS(100));
}
