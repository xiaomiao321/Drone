#ifndef __APP_FLIGHT__
#define __APP_FLIGHT__

#include "math.h"
#include "Com_debug.h"
#include "Com_filter.h"
#include "Com_imu.h"
#include "Com_pid.h"
#include "Int_motor.h"
#include "Int_mpu6050.h"
#include "Int_spl06.h"
#include "App_receive_data.h"

/**
 * @brief 初始化：MPU6050 初始化、电机启动、气压计初始化
 *
 */
void App_flight_init(void);

/**
 * @brief 读取传感器数据并解算欧拉角
 *
 */
void App_flight_get_euler_angle(void);

/**
 * @brief 根据欧拉角计算 PID 的目标值
 *
 */
void App_flight_pid_process(void);

/**
 * @brief 根据 PID 输出值控制电机
 *
 */
void App_flight_control_motor(void);

/**
 * @brief 定高气压计 PID 处理
 *
 */
void App_flight_fix_height_pid_process(void);

#endif // __APP_FLIGHT__
