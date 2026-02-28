/*
 * @file    Test_hw.c
 * @brief   气压计、陀螺仪、晶振焊接测试源文件
 *          使用状态位表明通讯状态（无串口）
 */

#include "Test_hw.h"
#include "gpio.h"
#include "i2c.h"

/* ============================================================================
 * 全局状态变量定义
 * ============================================================================
 * 状态和结果的关系：
 * - g_clock_ok, g_mpu6050_ok, g_spl06_ok 分别为 1 时表示对应硬件正常
 * - g_all_passed 为 1 当且仅当上述三个状态全部为 1
 * - Test_Result_Enum 是综合测试结果，用于 LED 显示模式选择
 * ============================================================================
 */
uint32_t g_clock_freq = 0; // 当前系统时钟频率（Hz）
uint8_t g_clock_ok = 0;    // 时钟状态：1=正常，0=失败
uint8_t g_mpu6050_ok = 0;  // 陀螺仪状态：1=正常，0=失败
uint8_t g_spl06_ok = 0;    // 气压计状态：1=正常，0=失败
uint8_t g_all_passed = 0;  // 综合测试状态：1=全部通过，0=有失败

// 调试变量：保存最后一次 SPL06 读取状态
volatile uint8_t g_spl06_last_status = 0; // HAL_I2C_Mem_Read 返回状态
volatile uint8_t g_spl06_last_id = 0xFF;  // 最后一次读取的 ID 值

// 外部 SystemCoreClock 变量声明（在 system_stm32f1xx.c 中定义）
extern uint32_t SystemCoreClock;

// 上一次测试结果（用于 LED 显示）
static Test_Result_Enum last_result = TEST_ALL_FAIL;

/**
 * @brief 获取系统时钟频率
 * @return 系统时钟频率（Hz）
 */
uint32_t Test_HW_Get_Clock_Freq(void) {
  // 更新 SystemCoreClock 变量
  SystemCoreClockUpdate();
  return SystemCoreClock;
}

/**
 * @brief 检查系统时钟频率
 * @return 1 时钟正常，0 时钟异常
 */
uint8_t Test_HW_Check_Clock(void) {
  g_clock_freq = Test_HW_Get_Clock_Freq();

  // 检查时钟频率是否在期望范围内（60-80MHz）
  if (g_clock_freq >= CLOCK_FREQ_MIN && g_clock_freq <= CLOCK_FREQ_MAX) {
    g_clock_ok = 1;
    return 1;
  } else {
    g_clock_ok = 0;
    return 0;
  }
}

/**
 * @brief 检查 MPU6050 通讯
 * @return 1 通讯正常，0 通讯失败
 */
uint8_t Test_HW_Check_MPU6050(void) {
  uint8_t device_id = 0;
  HAL_StatusTypeDef status;

  // 读取 MPU6050 设备 ID 寄存器（0x75）
  // MPU6050 的 WHO_AM_I 寄存器应返回 0x68
  status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR_READ, MPU_DEVICE_ID_REG,
                            I2C_MEMADD_SIZE_8BIT, &device_id, 1, 100);

  if (status == HAL_OK && device_id == 0x68) {
    g_mpu6050_ok = 1;
    return 1;
  } else {
    g_mpu6050_ok = 0;
    return 0;
  }
}

/**
 * @brief 检查 SPL06 通讯
 * @return 1 通讯正常，0 通讯失败
 */
uint8_t Test_HW_Check_SPL06(void) {
  uint8_t product_id = 0xFF; // 初始化为 0xFF，便于调试区分
  HAL_StatusTypeDef status;

  // 读取 SPL06 产品 ID 寄存器（0x0D）
  // SPL06 的产品 ID 应返回 0x10
  // 设备地址：0x76 (SDO 接地) 或 0x77 (SDO 接 VDD)
  status = HAL_I2C_Mem_Read(&hi2c2, SPL06_DEVICE_ADDR << 1, SPL06_ID,
                            I2C_MEMADD_SIZE_8BIT, &product_id, 1, 100);

  // 保存状态便于调试：
  // status 可能值：HAL_OK(0), HAL_ERROR(1), HAL_BUSY(2), HAL_TIMEOUT(3)
  // product_id 读取值：正常应为 0x10
  g_spl06_last_status = (uint8_t)status;
  g_spl06_last_id = product_id;

  if (status == HAL_OK && product_id == 0x10) {
    g_spl06_ok = 1;
    return 1;
  } else {
    g_spl06_ok = 0;
    return 0;
  }
}

/**
 * @brief 获取综合测试结果
 * @return 测试结果枚举
 */
Test_Result_Enum Test_HW_Get_Result(void) {
  uint8_t fail_count = 0;
  Test_Result_Enum result = TEST_OK;

  // 统计失败数量
  if (g_clock_ok == 0)
    fail_count++;
  if (g_mpu6050_ok == 0)
    fail_count++;
  if (g_spl06_ok == 0)
    fail_count++;

  // 根据失败数量确定结果
  if (fail_count == 0) {
    result = TEST_OK;
  } else if (fail_count >= 2) {
    result = TEST_ALL_FAIL;
  } else {
    // 只有一项失败
    if (g_clock_ok == 0)
      result = TEST_CLOCK_FAIL;
    else if (g_mpu6050_ok == 0)
      result = TEST_MPU6050_FAIL;
    else
      result = TEST_SPL06_FAIL;
  }

  return result;
}

/**
 * @brief 初始化硬件测试（检查时钟、传感器）
 * @return 测试结果枚举
 */
Test_Result_Enum Test_HW_Init(void) {
  // 1. 检查系统时钟
  Test_HW_Check_Clock();

  // 2. 检查 MPU6050 通讯
  Test_HW_Check_MPU6050();

  // 3. 检查 SPL06 通讯
  Test_HW_Check_SPL06();

  // 4. 设置全部通过标志
  g_all_passed = (g_clock_ok && g_mpu6050_ok && g_spl06_ok) ? 1 : 0;

  // 5. 获取综合结果
  last_result = Test_HW_Get_Result();

  return last_result;
}

/**
 * @brief LED 闪烁辅助函数
 * @param count 闪烁次数
 * @param on_time 点亮时间（ms）
 * @param off_time 熄灭时间（ms）
 */
static void Test_HW_LED_Blink(uint8_t count, uint32_t on_time,
                              uint32_t off_time) {
  for (uint8_t i = 0; i < count; i++) {
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    HAL_Delay(on_time);
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    HAL_Delay(off_time);
  }
}

/**
 * @brief 运行持续测试循环并 LED 指示
 * @note 此函数阻塞并持续更新 LED 状态和全局变量
 *
 * LED 指示模式：
 * - 所有测试通过：慢闪（1s 周期）
 * - MPU6050 失败：双闪 + 间隔
 * - SPL06 失败：三闪 + 间隔
 * - 时钟失败：常亮
 * - 全部失败：快速双闪
 */
void Test_HW_Run_Loop(void) {
  Test_Result_Enum result;

  // 先进行初始化测试
  result = Test_HW_Init();

  // 如果时钟失败，LED 常亮表示严重错误
  if (result == TEST_CLOCK_FAIL) {
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    while (1) {
      // 时钟故障，系统无法正常运行，保持常亮
      HAL_Delay(1000);
    }
  }

  // 主循环
  while (1) {
    // 重新运行测试
    result = Test_HW_Init();

    switch (result) {
    case TEST_OK:
      // 所有测试通过：慢闪（1s 周期）
      Test_HW_LED_Blink(1, LED_SLOW_BLINK, LED_SLOW_BLINK);
      break;

    case TEST_MPU6050_FAIL:
      // 陀螺仪失败：双闪
      Test_HW_LED_Blink(2, LED_DOUBLE_BLINK, LED_DOUBLE_BLINK);
      HAL_Delay(LED_ON_TIME);
      break;

    case TEST_SPL06_FAIL:
      // 气压计失败：三闪
      Test_HW_LED_Blink(3, LED_TRIPLE_BLINK, LED_TRIPLE_BLINK);
      HAL_Delay(LED_ON_TIME);
      break;

    case TEST_ALL_FAIL:
      // 多项失败：快速双闪
      Test_HW_LED_Blink(2, LED_FAST_BLINK, LED_FAST_BLINK);
      HAL_Delay(LED_ON_TIME);
      break;

    default:
      break;
    }
  }
}
