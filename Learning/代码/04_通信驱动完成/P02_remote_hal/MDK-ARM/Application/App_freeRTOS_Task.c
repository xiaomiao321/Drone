#include "App_freeRTOS_Task.h"

// STM32F103C8T6 => SRAM 20k  => 分配12K给操作系统

// 电源管理任务
void power_task(void *args);
// 最小推荐填写128 => 128*4 = 512B
#define POWER_TASK_STACK_SIZE 128
// 任务优先级 => 数值越小 优先级越小  => 最大4  => 不推荐使用最小优先级0
#define POWER_TASK_PRIORITY 4
TaskHandle_t power_task_handle;
#define POWER_TASK_PERIOD 10000

// 通讯任务
void com_task(void *args);
#define COM_TASK_STACK_SIZE 128
#define COM_TASK_PRIORITY 3
TaskHandle_t com_task_handle;
// 任务周期
#define COM_TASK_PERIOD 6

/**
 * @brief 启动freeRTOS操作系统
 *
 */
void App_freeRTOS_start(void)
{
    // 1. 创建电源管理任务
    xTaskCreate(power_task, "power_task", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &power_task_handle);

    // 2. 创建通讯任务
    xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &com_task_handle);

    // 3. 启动调度器
    vTaskStartScheduler();
}

void power_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {

        // 每10s执行一次  =>  启动电源  避免自动关机
        vTaskDelayUntil(&xLastWakeTime, POWER_TASK_PERIOD);

        // 启动电源
        Int_IP5305T_start();
    }
}

uint8_t com_buff[TX_PLOAD_WIDTH] = {0};

void com_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        // 调用SI24R1的接口 发送数据
        // 1. 进入TX模式
        Int_SI24R1_TX_Mode();
        // 2. 发送数据
        com_buff[0] = 'h';
        com_buff[1] = 'e';
        com_buff[2] = 'l';
        com_buff[3] = 'l';
        com_buff[4] = 'o';
        com_buff[5] = '!';

        Int_SI24R1_TxPacket(com_buff);

        // 3. 恢复到RX模式
        Int_SI24R1_RX_Mode();

        // 6ms执行一次
        vTaskDelayUntil(&xLastWakeTime, COM_TASK_PERIOD);
    }
}
