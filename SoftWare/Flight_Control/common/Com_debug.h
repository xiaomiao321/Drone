#ifndef __COM_DEBUG_H__
#define __COM_DEBUG_H__

#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

// 日志输出开关
#define DEBUG_LOG_ENABLE 1

// 调试级别
#define DEBUG_LEVEL_NONE    0
#define DEBUG_LEVEL_ERROR   1
#define DEBUG_LEVEL_WARN    2
#define DEBUG_LEVEL_INFO    3
#define DEBUG_LEVEL_VERBOSE 4

// 设置调试级别 (INFO 级别)
#define DEBUG_LEVEL DEBUG_LEVEL_INFO

// 调试输出宏
#if DEBUG_LOG_ENABLE

#define GET_BASE_NAME(p, sep) ((strrchr((p), (sep)) != NULL) ? (strrchr((p), (sep)) + 1) : (p))
#define GET_FILE_NAME(p) GET_BASE_NAME(GET_BASE_NAME((p), '\\'), '/')

#define debug_printf(format, ...) printf("[%s:%d] " format, GET_FILE_NAME(__FILE__), __LINE__, ##__VA_ARGS__)

// 分级调试宏
#define LOG_ERROR(format, ...) \
    do { if (DEBUG_LEVEL >= DEBUG_LEVEL_ERROR) \
        printf("[ERR] " format "\r\n", ##__VA_ARGS__); } while(0)

#define LOG_WARN(format, ...) \
    do { if (DEBUG_LEVEL >= DEBUG_LEVEL_WARN) \
        printf("[WARN] " format "\r\n", ##__VA_ARGS__); } while(0)

#define LOG_INFO(format, ...) \
    do { if (DEBUG_LEVEL >= DEBUG_LEVEL_INFO) \
        printf("[INFO] " format "\r\n", ##__VA_ARGS__); } while(0)

#define LOG_DEBUG(format, ...) \
    do { if (DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE) \
        printf("[DBG] " format "\r\n", ##__VA_ARGS__); } while(0)

// 系统状态打印宏
#define LOG_SYSTEM_STATUS(task_name, stack_free, heap_free) \
    do { if (DEBUG_LEVEL >= DEBUG_LEVEL_INFO) \
        printf("[SYS] %s - StackFree:%d HeapFree:%d\r\n", \
               task_name, stack_free, heap_free); } while(0)

#else

#define debug_printf(format, ...)
#define LOG_ERROR(format, ...)
#define LOG_WARN(format, ...)
#define LOG_INFO(format, ...)
#define LOG_DEBUG(format, ...)
#define LOG_SYSTEM_STATUS(task_name, stack_free, heap_free)

#endif

// 调试输出函数声明
void Debug_PrintMemoryUsage(void);
void Debug_PrintTaskStatus(void);
void Debug_PrintRCData(uint16_t thr, uint16_t yaw, uint16_t rol, uint16_t pit);
void Debug_PrintEulerAngle(float roll, float pitch, float yaw);
void Debug_PrintGyro(int16_t gx, int16_t gy, int16_t gz);
void Debug_PrintAccel(int16_t ax, int16_t ay, int16_t az);
void Debug_PrintBarometer(float pressure, float temperature, float altitude);
void Debug_PrintPIDOutput(const char *name, float desire, float measure, float output);

#endif // __COM_DEBUG_H__
