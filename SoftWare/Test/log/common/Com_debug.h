#ifndef __COM_DEBUG_H__
#define __COM_DEBUG_H__

#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include <string.h>

// 日志输出打印在 CPU 运行上非常占用资源 => 通过比特率可以计算  打印 10 字节左右大概需要 1ms  非常影响飞机的飞行
// 所以在后续飞机需要正常飞行的时候  需要关闭打印功能
// 设计一个日志输出打印开关
#define DEBUG_LOG_ENABLE 1

#ifdef DEBUG_LOG_ENABLE

// 辅助宏：从路径中提取文件名（处理 \ 和 / 两种路径分隔符）
#define GET_BASE_NAME(p, sep) ((strrchr((p), (sep)) != NULL) ? (strrchr((p), (sep)) + 1) : (p))
#define GET_FILE_NAME(p) GET_BASE_NAME(GET_BASE_NAME((p), '\\'), '/')

// 使用宏定义的方式能实现打印日志之前 先添加文件名和行号
#define debug_printf(format, ...) printf("[%s:%d]  " format, GET_FILE_NAME(__FILE__), __LINE__, ##__VA_ARGS__)

#else
// 如果没有开启日志输出打印
#define debug_printf(format, ...)
#endif // ifdef DEBUG_LOG_ENABLE

#endif // __COM_DEBUG_H__
