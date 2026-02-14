#include "App_freeRTOS_Task.h"

//STM32F103C8T6 => SRAM 20k  => 分配12K给操作系统

void task1(void *args);
// 最小推荐填写128 => 128*4 = 512B
#define TASK1_STACK_SIZE 128
// 任务优先级 => 数值越小 优先级越小  => 最大4  => 不推荐使用最小优先级0
#define TASK1_PRIORITY 1
TaskHandle_t task1_handle;

void task2(void *args);
// 最小推荐填写128 => 128*4 = 512B
#define TASK2_STACK_SIZE 128
// 任务优先级 => 数值越小 优先级越小  => 最大4  => 不推荐使用最小优先级0
#define TASK2_PRIORITY 1
TaskHandle_t task2_handle;


/**
 * @brief 启动freeRTOS操作系统
 *
 */
void App_freeRTOS_start(void)
{
    // 1. 创建任务
    xTaskCreate(task1,"task1",TASK1_STACK_SIZE,NULL,TASK1_PRIORITY,&task1_handle);

    // 创建多个任务
    xTaskCreate(task2,"task2",TASK2_STACK_SIZE,NULL,TASK2_PRIORITY,&task2_handle);

    // 2. 启动调度器
    vTaskStartScheduler();
}


void task1(void *args)
{
    // task1任务启动之后 不断执行的内容
    while(1)
    {
        debug_printf("task1\r\n");
        vTaskDelay(1000);// 延迟1000ms => 释放CPU占用
    }
}


void task2(void *args)
{
    // task1任务启动之后 不断执行的内容
    while(1)
    {
        debug_printf("task2\r\n");
        vTaskDelay(900);// 延迟900ms => 释放CPU占用
    }
}

