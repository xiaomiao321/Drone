#include "App_freeRTOS_Task.h"

// STM32F103C8T6 => SRAM 20k  => 分配12K给操作系统

// 电源管理任务
void power_task(void *args);
// 最小推荐填写128 => 128*4 = 512B
#define POWER_TASK_STACK_SIZE 128
// 任务优先级 => 数值越小 优先级越小  => 最大4  => 不推荐使用最小优先级0
#define POWER_TASK_PRIORITY 4
TaskHandle_t power_task_handle;

/**
 * @brief 启动freeRTOS操作系统
 *
 */
void App_freeRTOS_start(void)
{
    // 1. 创建电源管理任务
    xTaskCreate(power_task, "power_task", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &power_task_handle);
    // 2. 启动调度器
    vTaskStartScheduler();
}

void power_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {

        // 每10s执行一次  =>  启动电源  避免自动关机
        vTaskDelayUntil(&xLastWakeTime, 10000);

        // 启动电源
        Int_IP5305T_start();
    }
}
