#ifndef _COM_CONFIG_H
#define _COM_CONFIG_H

#include "main.h"
// #include "FreeRTOS.h"

// 遥控状态
typedef enum {
  REMOTE_CONNECTED = 0,
  REMOTE_DISCONNECTED,
} Remote_State;

// 飞行状态
typedef enum {
  IDLE = 0,
  NORMAL,
  FIX_HEIGHT,
  FAIL,
} Flight_State;

// 油门状态
typedef enum {
  FREE = 0,
  MAX,
  LEAVE_MAX,
  MIN,
  UNLOCK,
} Thr_state;

// 陀螺仪数据  16 位 ADC 原始值
typedef struct {
  int16_t gyro_x; // 右滚转为正   表示横滚
  int16_t gyro_y; // 前俯旋转为正 表示俯仰
  int16_t gyro_z; // 顺时针旋转为正  表示偏航
} Gyro_struct;

// 加速度计 16 位 ADC 原始值
typedef struct {
  int16_t accel_x; // 向前为正
  int16_t accel_y; // 向左为正
  int16_t accel_z; // 垂直向上的加速度为正
} Accel_struct;

typedef struct {
  Gyro_struct gyro;
  Accel_struct accel;
} Gyro_Accel_Struct;

// 解算得到的欧拉角
typedef struct {
  float yaw;
  float pitch;
  float roll;
} Euler_struct;

// 遥控器数据结构 (在 App_receive_data.h 中定义)
// 这里只声明，实际定义在 App_receive_data.h
typedef struct RC_Data_s Remote_Data;

#endif // !_COM_CONFIG_H
