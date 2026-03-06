#include "Com_debug.h"

// 重定向 printf 到 UART
extern UART_HandleTypeDef huart2;

int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 1000);
    return ch;
}

int fputc(int ch, FILE *f)
{
    return __io_putchar(ch);
}

/**
 * @brief 打印内存使用情况 (任务堆栈 + FreeRTOS Heap)
 */
void Debug_PrintMemoryUsage(void)
{
    LOG_INFO("=== Memory Usage ===");
    LOG_INFO("FreeRTOS Heap Free: %d bytes", (int)xPortGetFreeHeapSize());
    LOG_INFO("Min Heap Ever: %d bytes", (int)xPortGetMinimumEverFreeHeapSize());
}

/**
 * @brief 打印所有任务状态
 */
void Debug_PrintTaskStatus(void)
{
#if (configUSE_TRACE_FACILITY == 1)
    TaskStatus_t *tasks;
    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    
    tasks = pvPortMalloc(task_count * sizeof(TaskStatus_t));
    if (tasks != NULL)
    {
        task_count = uxTaskGetSystemState(tasks, task_count, NULL);
        
        LOG_INFO("=== Task Status ===");
        for (UBaseType_t i = 0; i < task_count; i++)
        {
            LOG_INFO("Task: %s, Priority: %lu, StackWatermark: %lu",
                    tasks[i].pcTaskName,
                    tasks[i].uxCurrentPriority,
                    tasks[i].usStackHighWaterMark);
        }
        
        vPortFree(tasks);
    }
#else
    LOG_INFO("Task Status: Enable configUSE_TRACE_FACILITY to view details");
#endif
}

/**
 * @brief 打印遥控器通道数据
 */
void Debug_PrintRCData(uint16_t thr, uint16_t yaw, uint16_t rol, uint16_t pit)
{
    LOG_DEBUG("RC: THR=%4d YAW=%4d ROL=%4d PIT=%4d", thr, yaw, rol, pit);
}

/**
 * @brief 打印欧拉角
 */
void Debug_PrintEulerAngle(float roll, float pitch, float yaw)
{
    LOG_DEBUG("Euler: R=%6.2f P=%6.2f Y=%6.2f deg", roll, pitch, yaw);
}

/**
 * @brief 打印陀螺仪数据
 */
void Debug_PrintGyro(int16_t gx, int16_t gy, int16_t gz)
{
    LOG_DEBUG("Gyro: X=%6d Y=%6d Z=%6d", gx, gy, gz);
}

/**
 * @brief 打印加速度计数据
 */
void Debug_PrintAccel(int16_t ax, int16_t ay, int16_t az)
{
    LOG_DEBUG("Accel: X=%6d Y=%6d Z=%6d", ax, ay, az);
}

/**
 * @brief 打印气压计数据
 */
void Debug_PrintBarometer(float pressure, float temperature, float altitude)
{
    LOG_DEBUG("Baro: P=%.2f Pa T=%.2f C Alt=%.2f m", pressure, temperature, altitude);
}

/**
 * @brief 打印 PID 输出
 */
void Debug_PrintPIDOutput(const char *name, float desire, float measure, float output)
{
    LOG_DEBUG("PID[%s]: Desire=%6.2f Measure=%6.2f Output=%6.2f", 
              name, desire, measure, output);
}
