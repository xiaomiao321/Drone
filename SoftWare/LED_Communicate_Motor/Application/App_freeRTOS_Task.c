#include "App_freeRTOS_Task.h"
#include "../Application/Init.h"
#include "Int_led.h"
#include "Int_motor.h"
#include "Int_nRF24L01.h"
// 外部引用在 Init.c 中定义的电机结构体
extern Motor_Struct left_top_motor;
extern Motor_Struct left_bottom_motor;
extern Motor_Struct right_top_motor;
extern Motor_Struct right_bottom_motor;

extern LED_Struct left_top_led;
extern LED_Struct right_top_led;
extern LED_Struct left_bottom_led;
extern LED_Struct right_bottom_led;
// 表示当前连接状态
Remote_State remote_state = REMOTE_DISCONNECTED;

// 表示当前的飞行状态
Flight_State flight_state = NORMAL;

void flight_task(void *args);
#define FLIGHT_TASK_STACK_SIZE 128
#define FLIGHT_TASK_PRIORITY 3
TaskHandle_t flight_task_handle;
#define FLIGHT_TASK_PERIOD 6

void led_task(void *args);
#define LED_TASK_STACK_SIZE 128
#define LED_TASK_PRIORITY 1
TaskHandle_t led_task_handle;
#define LED_TASK_PERIOD 100

// 通讯任务
void com_task(void *args);
#define COM_TASK_STACK_SIZE 256
#define COM_TASK_PRIORITY 2
TaskHandle_t com_task_handle;
// 任务周期
#define COM_TASK_PERIOD 6
/**
 * @brief 启动 freeRTOS 操作系统
 *
 */
void App_freeRTOS_start(void) {
  // 0. 初始化 nRF24L01 模块（必须在创建通讯任务之前初始化）
  Int_nRF24L01_Init();

  // 1. 创建任务
  // xTaskCreate(flight_task, "flight_task", FLIGHT_TASK_STACK_SIZE, NULL,
  //             FLIGHT_TASK_PRIORITY, &flight_task_handle);
  // xTaskCreate(led_task, "led_task", LED_TASK_STACK_SIZE, NULL,
  //             LED_TASK_PRIORITY, &led_task_handle);
  xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL,
              COM_TASK_PRIORITY, &com_task_handle);
  // 2. 启动调度器
  vTaskStartScheduler();
}

/**
 * @brief 飞行任务主循环
 *
 */
void flight_task(void *args) {
  TickType_t xLastWakeTime;

  // 首次运行时执行系统初始化（电调校准）
  static uint8_t initialized = 0;
  if (!initialized) {
    System_Init();
    initialized = 1;
  }

  // 初始化完成后获取时间基准
  xLastWakeTime = xTaskGetTickCount();

  debug_printf("flight_task: Starting motor test loop\n");

  while (1) {
    // 测试不同占空比（速度值）
    // ARR=2000 时：50%=1000, 60%=1200, 70%=1400, 80%=1600, 90%=1800

    // // 测试 1: 低速运行 (20% 占空比)
    // debug_printf("Test 1: speed=400\n");
    // left_top_motor.speed = 400;
    // right_bottom_motor.speed = 400;
    // left_bottom_motor.speed = 400;
    // right_top_motor.speed = 400;
    // Int_motor_set_speed(&left_top_motor);
    // Int_motor_set_speed(&right_bottom_motor);
    // Int_motor_set_speed(&left_bottom_motor);
    // Int_motor_set_speed(&right_top_motor);
    // vTaskDelay(pdMS_TO_TICKS(5000));

    // // 测试 2: 中低速 (30% 占空比)
    // debug_printf("Test 2: speed=600\n");
    // left_top_motor.speed = 600;
    // right_bottom_motor.speed = 600;
    // left_bottom_motor.speed = 600;
    // right_top_motor.speed = 600;
    // Int_motor_set_speed(&left_top_motor);
    // Int_motor_set_speed(&right_bottom_motor);
    // Int_motor_set_speed(&left_bottom_motor);
    // Int_motor_set_speed(&right_top_motor);
    // vTaskDelay(pdMS_TO_TICKS(5000));

    // // 测试 3: 中速 (40% 占空比)
    // debug_printf("Test 3: speed=800\n");
    // left_top_motor.speed = 800;
    // right_bottom_motor.speed = 800;
    // left_bottom_motor.speed = 800;
    // right_top_motor.speed = 800;
    // Int_motor_set_speed(&left_top_motor);
    // Int_motor_set_speed(&right_bottom_motor);
    // Int_motor_set_speed(&left_bottom_motor);
    // Int_motor_set_speed(&right_top_motor);
    // vTaskDelay(pdMS_TO_TICKS(5000));

    // // 测试 4: 中高速 (50% 占空比)
    // debug_printf("Test 4: speed=1000\n");
    // left_top_motor.speed = 1000;
    // right_bottom_motor.speed = 1000;
    // left_bottom_motor.speed = 1000;
    // right_top_motor.speed = 1000;
    // Int_motor_set_speed(&left_top_motor);
    // Int_motor_set_speed(&right_bottom_motor);
    // Int_motor_set_speed(&left_bottom_motor);
    // Int_motor_set_speed(&right_top_motor);
    // vTaskDelay(pdMS_TO_TICKS(5000));

    // // 测试 5: 高速 (60% 占空比)
    // debug_printf("Test 5: speed=1200\n");
    // left_top_motor.speed = 1200;
    // right_bottom_motor.speed = 1200;
    // left_bottom_motor.speed = 1200;
    // right_top_motor.speed = 1200;
    // Int_motor_set_speed(&left_top_motor);
    // Int_motor_set_speed(&right_bottom_motor);
    // Int_motor_set_speed(&left_bottom_motor);
    // Int_motor_set_speed(&right_top_motor);
    // vTaskDelay(pdMS_TO_TICKS(5000));

    debug_printf("flight");
    vTaskDelayUntil(&xLastWakeTime, FLIGHT_TASK_PERIOD); /*控制多久执行一次*/
  }
}
void led_task(void *args) {

  // 获取当前的基准时间
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t count = 0;
  while (1) {
    count++;
    // 前两个灯表示连接状态
    // 1. 判断当前连接状态
    if (remote_state == REMOTE_CONNECTED) {
      // 点亮前两个灯
      Int_led_turn_on(&left_top_led);
      Int_led_turn_on(&right_top_led);
    } else if (remote_state == REMOTE_DISCONNECTED) {
      // 关掉前两个灯
      Int_led_turn_off(&left_top_led);
      Int_led_turn_off(&right_top_led);
    }

    // 后两个灯表示飞行状态
    // 2. 判断当前飞行状态
    if (flight_state == IDLE) {
      // 灯慢闪烁 => 500ms亮 500ms灭
      if (count % 5 == 0) {
        // 循环5次  一次是100ms  5次等于500ms
        Int_led_toggle(&left_bottom_led);
        Int_led_toggle(&right_bottom_led);
      }
    } else if (flight_state == NORMAL) {
      // 灯快闪  =>  200ms亮 200ms灭
      if (count % 2 == 0) {
        // 循环2次  一次是100ms  2次等于200ms
        Int_led_toggle(&left_bottom_led);
        Int_led_toggle(&right_bottom_led);
      }
    } else if (flight_state == FIX_HEIGHT) {
      // 后两个灯常量
      Int_led_turn_on(&left_bottom_led);
      Int_led_turn_on(&right_bottom_led);
    } else if (flight_state == FAIL) {
      // 后两个灯灭
      Int_led_turn_off(&left_bottom_led);
      Int_led_turn_off(&right_bottom_led);
    }

    // 将count计数重置
    if (count == 10) {
      count = 0;
    }

    vTaskDelayUntil(&xLastWakeTime, LED_TASK_PERIOD);
  }
}

// RC 通道数据结构
typedef struct {
  uint16_t thr;  // 油门
  uint16_t yaw;  // 偏航（方向）
  uint16_t rol;  // 横滚
  uint16_t pit;  // 俯仰
  uint16_t aux1;
  uint16_t aux2;
  uint16_t aux3;
  uint16_t aux4;
} RC_Data_t;

RC_Data_t rc_data = {0};
uint8_t com_data[TX_PLOAD_WIDTH] = {0};

// 解析 ANO_DT 遥控器数据协议
uint8_t ANO_DT_ParseRC(uint8_t *buf, uint8_t len, RC_Data_t *rc) {
  if (buf[0] != 0xAA || buf[1] != 0xAF) return 0;
  if (buf[2] != 0x03) return 0;
  uint8_t data_len = buf[3];  // 数据区长度
  if (data_len < 12) return 0;
  // 总帧长度 = 4 字节头 + 数据区 + 1 字节校验和
  uint8_t frame_len = 4 + data_len + 1;
  if (frame_len > len) return 0;
  // 校验和：从帧头到数据区末尾
  uint8_t sum = 0;
  for (uint8_t i = 0; i < frame_len - 1; i++) sum += buf[i];
  if (sum != buf[frame_len - 1]) return 0;
  rc->thr  = (uint16_t)((buf[4] << 8) | buf[5]);
  rc->yaw  = (uint16_t)((buf[6] << 8) | buf[7]);
  rc->rol  = (uint16_t)((buf[8] << 8) | buf[9]);
  rc->pit  = (uint16_t)((buf[10] << 8) | buf[11]);
  rc->aux1 = (uint16_t)((buf[12] << 8) | buf[13]);
  rc->aux2 = (uint16_t)((buf[14] << 8) | buf[15]);
  return 1;
}

void com_task(void *args) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint32_t rx_count = 0, err_count = 0;
  debug_printf("COM task started...\r\n");
  while (1) {
    uint8_t res = Int_nRF24L01_RxPacket(com_data);
    if (res == 0) {
      rx_count++;
      if (ANO_DT_ParseRC(com_data, 32, &rc_data)) {
        debug_printf("[RC %lu] THR:%4d YAW:%4d ROL:%4d PIT:%4d\r\n",
                     rx_count, rc_data.thr, rc_data.yaw, rc_data.rol, rc_data.pit);
      } else {
        debug_printf("[RX %lu] Parse failed\r\n", rx_count);
      }
    } else {
      err_count++;
      if (err_count % 10000 == 0) {
        debug_printf("[NO DATA] %lu\r\n", err_count);
      }
    }

    // 6ms执行一次 接收数据的时间间隔应该等于发送数据的时间间隔
    vTaskDelayUntil(&xLastWakeTime, COM_TASK_PERIOD);
  }
}
