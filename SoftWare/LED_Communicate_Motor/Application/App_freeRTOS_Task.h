#ifndef __APP_FREERTOS_TASK__
#define __APP_FREERTOS_TASK__

#include "Com_debug.h"
#include "FreeRTOS.h"
#include "task.h"

#include "../interface/Int_motor.h"

/**
 * @brief 启动freeRTOS操作系统
 *
 */
void App_freeRTOS_start(void);

#endif // __APP_FREERTOS_TASK__
