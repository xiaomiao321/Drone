#include "gpio.h"
#include "main.h"
#include "tim.h"
extern uint32_t tick;
void MainInit() { HAL_TIM_Base_Start_IT(&htim2); }
void MainTask() { tick++; }
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim2) {
    MainTask();
  }
}