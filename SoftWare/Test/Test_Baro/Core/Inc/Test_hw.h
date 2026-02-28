/*
 * @file    Test_hw.h
 * @brief   气压计、陀螺仪、晶振焊接测试头文件
 *          使用状态位表明通讯状态（无串口）
 */

#ifndef __TEST_HW__
#define __TEST_HW__

#include "stm32f1xx_hal.h"
#include "Int_mpu6050.h"
#include "Int_spl06.h"
#include "main.h"
#include <stdint.h>

/* ============================================================================
 * LED 状态定义（使用 GPIOA Pin 12 的 LED1）
 * ============================================================================
 * 闪烁模式表示测试状态：
 * - 快闪 (200ms):       测试进行中/初始化
 * - 慢闪 (1s):          所有测试通过
 * - 双闪：MPU6050 通讯失败
 * - 三闪：SPL06 通讯失败
 * - 常亮：晶振/时钟故障
 * - 常灭：系统停止/严重错误
 * ============================================================================
 */

// 测试结果枚举（综合状态）
typedef enum
{
    TEST_OK = 0,           // 全部通过
    TEST_MPU6050_FAIL,     // 仅陀螺仪失败
    TEST_SPL06_FAIL,       // 仅气压计失败
    TEST_CLOCK_FAIL,       // 仅时钟失败
    TEST_ALL_FAIL          // 多项失败
} Test_Result_Enum;

// 时钟频率阈值（期望 72MHz，接受 60-80MHz 范围）
#define CLOCK_FREQ_MIN    60000000UL
#define CLOCK_FREQ_MAX    80000000UL
#define CLOCK_FREQ_TARGET 72000000UL

// SPL06 设备地址（根据 SDO 引脚决定：SDO 接地=0x76，SDO 接 VDD=0x77）
#define SPL06_DEVICE_ADDR  0x76

// LED 时序常量（毫秒）
#define LED_FAST_BLINK     200    // 200ms 测试进行中
#define LED_SLOW_BLINK     1000   // 1s 成功
#define LED_DOUBLE_BLINK   150    // 双闪模式
#define LED_TRIPLE_BLINK   150    // 三闪模式
#define LED_ON_TIME        500    // 闪烁间隔

/* ============================================================================
 * 全局状态变量（可直接访问）
 * ============================================================================
 */
extern uint32_t g_clock_freq;        // 当前系统时钟频率（Hz）
extern uint8_t  g_clock_ok;          // 时钟状态：1=正常，0=失败
extern uint8_t  g_mpu6050_ok;        // 陀螺仪状态：1=正常，0=失败
extern uint8_t  g_spl06_ok;          // 气压计状态：1=正常，0=失败
extern uint8_t  g_all_passed;        // 综合测试状态：1=全部通过，0=有失败

// 调试变量（用于 SPL06 故障排查）
extern volatile uint8_t g_spl06_last_status;  // HAL_I2C_Mem_Read 返回状态
extern volatile uint8_t g_spl06_last_id;      // 最后一次读取的 ID 值

/**
 * @brief 初始化硬件测试（检查时钟、传感器）
 * @return 测试结果枚举
 */
Test_Result_Enum Test_HW_Init(void);

/**
 * @brief 检查系统时钟频率
 * @return 1 时钟正常，0 时钟异常
 */
uint8_t Test_HW_Check_Clock(void);

/**
 * @brief 检查 MPU6050 通讯
 * @return 1 通讯正常，0 通讯失败
 */
uint8_t Test_HW_Check_MPU6050(void);

/**
 * @brief 检查 SPL06 通讯
 * @return 1 通讯正常，0 通讯失败
 */
uint8_t Test_HW_Check_SPL06(void);

/**
 * @brief 运行持续测试循环并 LED 指示
 * @note 此函数阻塞并持续更新 LED 状态和全局变量
 */
void Test_HW_Run_Loop(void);

/**
 * @brief 获取系统时钟频率
 * @return 系统时钟频率（Hz）
 */
uint32_t Test_HW_Get_Clock_Freq(void);

/**
 * @brief 获取综合测试结果
 * @return 测试结果枚举
 */
Test_Result_Enum Test_HW_Get_Result(void);

#endif // __TEST_HW__
