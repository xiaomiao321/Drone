#include "task.h"
#include "gpio.h"
#include "main.h"
#include "math.h"
#include "stdint.h"
#include "tim.h"

uint32_t tick = 0;
// void MainInit(void) {
//   HAL_TIM_Base_Start_IT(&htim2);
//   HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
// }

// void MainTask(void) { tick++; }

// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//   if (htim == &htim2) {
//     tick++;
//   }
// }
