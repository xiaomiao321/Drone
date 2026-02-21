#ifndef __INT_BUZZER__
#define __INT_BUZZER__

#include "main.h"

// 蜂鸣器引脚定义
#define BUZZER_PORT       GPIOB
#define BUZZER_PIN        GPIO_PIN_0

// 蜂鸣器控制宏 (低电平有效)
#define BUZZER_ON()       HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET)
#define BUZZER_OFF()      HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET)
#define BUZZER_TOGGLE()   HAL_GPIO_TogglePin(BUZZER_PORT, BUZZER_PIN)

/**
 * @brief 初始化蜂鸣器
 */
void Int_buzzer_init(void);

/**
 * @brief 蜂鸣器开启
 */
void Int_buzzer_on(void);

/**
 * @brief 蜂鸣器关闭
 */
void Int_buzzer_off(void);

/**
 * @brief 蜂鸣器翻转
 */
void Int_buzzer_toggle(void);

/**
 * @brief 短蜂鸣 (100ms)
 */
void Int_buzzer_short_beep(void);

/**
 * @brief 长蜂鸣 (500ms)
 */
void Int_buzzer_long_beep(void);

/**
 * @brief 双蜂鸣 (2 次短蜂鸣)
 */
void Int_buzzer_double_beep(void);

/**
 * @brief 三蜂鸣 (3 次短蜂鸣，用于错误提示)
 */
void Int_buzzer_triple_beep(void);

/**
 * @brief 连续蜂鸣 (用于失控报警，需要在循环中调用)
 */
void Int_buzzer_alarm(void);

#endif // __INT_BUZZER__
