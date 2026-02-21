#include "App_freeRTOS_Task.h"
#include "App_flight.h"
#include "App_receive_data.h"
#include "Com_debug.h"
#include "Init.h"
#include "Int_buzzer.h"
#include "Int_led.h"
#include "Int_motor.h"
#include "Int_mpu6050.h"
#include "Int_nRF24L01.h"
#include "Int_spl06.h"

// 外部引用在 Init.c 中定义的电机结构体
extern Motor_Struct left_top_motor;
extern Motor_Struct left_bottom_motor;
extern Motor_Struct right_top_motor;
extern Motor_Struct right_bottom_motor;

extern LED_Struct left_top_led;
extern LED_Struct right_top_led;
extern LED_Struct left_bottom_led;
extern LED_Struct right_bottom_led;

// 全局状态变量
Remote_State remote_state = REMOTE_DISCONNECTED;
Flight_State flight_state = IDLE;
uint16_t fix_height = 0;

// 任务句柄
TaskHandle_t flight_task_handle;
TaskHandle_t led_task_handle;
TaskHandle_t com_task_handle;
TaskHandle_t baro_task_handle;

// 任务配置 (STM32F103C8T6 10KB RAM 优化版)
// 堆栈大小单位：字 (4 字节)
// 总任务堆栈：1792 字 = 7168 字节
// FreeRTOS Heap: 3072 字节
// 静态数据：约 1000 字节
// 总计：约 7200 字节 (70% 使用率，留 30% 余量)

#define FLIGHT_TASK_STACK_SIZE 160 // 640 字节 - 姿态解算需要较多堆栈
#define FLIGHT_TASK_PRIORITY 3
#define FLIGHT_TASK_PERIOD 6

#define LED_TASK_STACK_SIZE 80 // 320 字节 - LED 状态指示
#define LED_TASK_PRIORITY 1
#define LED_TASK_PERIOD 100

#define COM_TASK_STACK_SIZE 128 // 512 字节 - 遥控器数据解析
#define COM_TASK_PRIORITY 2
#define COM_TASK_PERIOD 6

#define BARO_TASK_STACK_SIZE 96 // 384 字节 - 气压计数据读取
#define BARO_TASK_PRIORITY 2
#define BARO_TASK_PERIOD 24

// 任务函数声明
void flight_task(void *args);
void led_task(void *args);
void com_task(void *args);
void baro_task(void *args);

/**
 * @brief 启动 freeRTOS 操作系统
 */
void App_freeRTOS_start(void) {
  // 0. 初始化 nRF24L01 模块
  Int_nRF24L01_Init();

  // 1. 创建任务
  // xTaskCreate(flight_task, "flight", FLIGHT_TASK_STACK_SIZE, NULL,
  //             FLIGHT_TASK_PRIORITY, &flight_task_handle);
  // xTaskCreate(led_task, "led", LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY,
  //             &led_task_handle);
  xTaskCreate(com_task, "com", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY,
              &com_task_handle);
  // xTaskCreate(baro_task, "baro", BARO_TASK_STACK_SIZE, NULL,
  // BARO_TASK_PRIORITY,
  //             &baro_task_handle);

  // 2. 启动调度器
  vTaskStartScheduler();
}

/**
 * @brief 飞行任务主循环 (6ms 周期)
 */
void flight_task(void *args) {
  TickType_t xLastWakeTime;
  uint32_t loop_count = 0;

  // 首次运行时执行系统初始化
  static uint8_t initialized = 0;
  if (!initialized) {
    System_Init();
    App_flight_init();
    initialized = 1;

    // 打印启动信息
    LOG_INFO("================================");
    LOG_INFO("Flight Control System Started");
    LOG_INFO("================================");
    LOG_INFO("MCU: STM32F103C8T6 @ 72MHz");
    LOG_INFO("FreeRTOS Heap: %d bytes", xPortGetFreeHeapSize());
    LOG_INFO("Sensors: MPU6050 (I2C1), SPL06 (I2C2)");
    LOG_INFO("================================");
  }

  xLastWakeTime = xTaskGetTickCount();
  LOG_INFO("Flight task running...");

  while (1) {
    loop_count++;

    // 1. 读取传感器数据并解算欧拉角
    App_flight_get_euler_angle();

    // 2. 计算 PID 目标值
    App_flight_pid_process();

    // 3. 根据 PID 输出控制电机
    App_flight_control_motor();

    // 4. 处理飞行状态机
    App_process_flight_state();

    // 每 100 次循环 (600ms) 打印一次调试信息
    if (loop_count % 100 == 0) {
      extern Euler_struct euler_angle;
      extern Remote_State remote_state;
      extern Flight_State flight_state;

      const char *state_str = flight_state == IDLE         ? "IDLE"
                              : flight_state == NORMAL     ? "NORMAL"
                              : flight_state == FIX_HEIGHT ? "FIX_HEIGHT"
                                                           : "FAIL";
      const char *conn_str = remote_state == REMOTE_CONNECTED ? "OK" : "LOST";

      LOG_INFO("[%lu] State:%s RC:%s R:%.1f P:%.1f Y:%.1f", loop_count,
               state_str, conn_str, euler_angle.roll, euler_angle.pitch,
               euler_angle.yaw);

      // 打印堆栈水位线
      UBaseType_t stack_watermark = uxTaskGetStackHighWaterMark(NULL);
      LOG_DEBUG("Flight stack watermark: %lu words", stack_watermark);
    }

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(FLIGHT_TASK_PERIOD));
  }
}

/**
 * @brief LED 任务 (100ms 周期)
 */
void led_task(void *args) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t count = 0;

  while (1) {
    count++;

    // 前两个灯表示连接状态
    if (remote_state == REMOTE_CONNECTED) {
      Int_led_turn_on(&left_top_led);
      Int_led_turn_on(&right_top_led);
    } else {
      Int_led_turn_off(&left_top_led);
      Int_led_turn_off(&right_top_led);
    }

    // 后两个灯表示飞行状态
    switch (flight_state) {
    case IDLE:
      // 慢闪 (500ms)
      if (count % 5 == 0) {
        Int_led_toggle(&left_bottom_led);
        Int_led_toggle(&right_bottom_led);
      }
      break;
    case NORMAL:
      // 快闪 (200ms)
      if (count % 2 == 0) {
        Int_led_toggle(&left_bottom_led);
        Int_led_toggle(&right_bottom_led);
      }
      break;
    case FIX_HEIGHT:
      // 常亮
      Int_led_turn_on(&left_bottom_led);
      Int_led_turn_on(&right_bottom_led);
      break;
    case FAIL:
      // 熄灭 + 蜂鸣器报警 (每 500ms 一次)
      Int_led_turn_off(&left_bottom_led);
      Int_led_turn_off(&right_bottom_led);
      if (count % 5 == 0) {
        Int_buzzer_alarm();
      }
      break;
    }

    if (count >= 10)
      count = 0;

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(LED_TASK_PERIOD));
  }
}

/**
 * @brief 通讯任务 (6ms 周期) - 使用你的 ANO_DT 协议实现
 */
void com_task(void *args) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t res;
  uint32_t rx_count = 0;
  uint32_t err_count = 0;

  LOG_INFO("COM task started");

  while (1) {
    // 1. 接收并解析遥控器数据
    res = App_receive_data();

    // 2. 处理连接状态
    App_process_connect_state(res);

    // 3. 调试输出 (每 100 次输出一次，避免刷屏)
    if (res == 0) {
      rx_count++;
      if (rx_count % 100 == 0) {
        LOG_INFO("[RC %lu] %s", rx_count, App_get_rc_string());
      }
    } else {
      err_count++;
      if (err_count % 1000 == 0) {
        LOG_WARN("[NO DATA] err=%lu", err_count);
      }
    }

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(COM_TASK_PERIOD));
  }
}

/**
 * @brief 气压计任务 (24ms 周期) - 用于定高
 */
void baro_task(void *args) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint32_t read_count = 0;

  LOG_INFO("Baro task started");

  while (1) {
    // 只在定高模式下执行定高 PID
    if (flight_state == FIX_HEIGHT) {
      App_flight_fix_height_pid_process();
      read_count++;

      // 每 50 次读取打印一次气压计数据
      if (read_count % 50 == 0) {
        extern SPL06_Data_Struct spl06_data;
        LOG_DEBUG("[Baro] P=%.1f Pa T=%.1f C Alt=%.2f m", spl06_data.pressure,
                  spl06_data.temperature, spl06_data.altitude);
      }
    }

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(BARO_TASK_PERIOD));
  }
}
