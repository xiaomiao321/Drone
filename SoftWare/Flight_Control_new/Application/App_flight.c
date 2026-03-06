#include "App_flight.h"

Gyro_Accel_Struct gyro_accel_data = {0};
Euler_struct euler_angle = {0};
Gyro_struct last_gyro = {0};
float gyro_z_sum = 0;
SPL06_Data_Struct spl06_data = {0}; // 气压计数据全局变量

// 外部引用
extern Remote_State remote_state;
extern Flight_State flight_state;
extern TaskHandle_t com_task_handle;
extern uint16_t fix_height;

// PID 参数结构
// 俯仰角 PID (外环角度，内环角速度)
PID_Struct pitch_pid = {.kp = -7.00f, .ki = 0.00f, .kd = 0.00f};
PID_Struct gyro_y_pid = {.kp = 3.00f, .ki = 0.00f, .kd = 0.50f};

// 横滚角 PID (外环角度，内环角速度)
PID_Struct roll_pid = {.kp = -7.00f, .ki = 0.00f, .kd = 0.00f};
PID_Struct gyro_x_pid = {.kp = 3.00f, .ki = 0.00f, .kd = 0.50f};

// 偏航角 PID (外环角度，内环角速度)
PID_Struct yaw_pid = {.kp = -3.00f, .ki = 0.00f, .kd = 0.00f};
PID_Struct gyro_z_pid = {.kp = -5.00f, .ki = 0.00f, .kd = 0.00f};

// 定高 PID
PID_Struct height_pid = {.kp = -0.60f, .ki = 0.00f, .kd = -0.20f};

/**
 * @brief 初始化：MPU6050 初始化、电机启动、气压计初始化
 */
void App_flight_init(void) {
  // 初始化 MPU6050 (I2C1)
  Int_MPU6050_Init();

  // 初始化气压计 SPL06 (I2C2)
  if (Int_SPL06_Init() != 0) {
    debug_printf("SPL06 init failed!\r\n");
  }

  // 电机已经在 Init.c 中初始化，这里不需要重复初始化
}

/**
 * @brief 读取传感器数据并解算欧拉角
 */
void App_flight_get_euler_angle(void) {
  // 1. 读取 MPU6050 数据
  Int_MPU6050_Get_Data(&gyro_accel_data);

  // 2. 角速度低通滤波
  gyro_accel_data.gyro.gyro_x =
      Common_Filter_LowPass(gyro_accel_data.gyro.gyro_x, last_gyro.gyro_x);
  gyro_accel_data.gyro.gyro_y =
      Common_Filter_LowPass(gyro_accel_data.gyro.gyro_y, last_gyro.gyro_y);
  gyro_accel_data.gyro.gyro_z =
      Common_Filter_LowPass(gyro_accel_data.gyro.gyro_z, last_gyro.gyro_z);
  last_gyro.gyro_x = gyro_accel_data.gyro.gyro_x;
  last_gyro.gyro_y = gyro_accel_data.gyro.gyro_y;
  last_gyro.gyro_z = gyro_accel_data.gyro.gyro_z;

  // 3. 加速度卡尔曼滤波
  gyro_accel_data.accel.accel_x =
      Common_Filter_KalmanFilter(&kfs[0], gyro_accel_data.accel.accel_x);
  gyro_accel_data.accel.accel_y =
      Common_Filter_KalmanFilter(&kfs[1], gyro_accel_data.accel.accel_y);
  gyro_accel_data.accel.accel_z =
      Common_Filter_KalmanFilter(&kfs[2], gyro_accel_data.accel.accel_z);

  // 4. 四元数解算欧拉角
  Common_IMU_GetEulerAngle(&gyro_accel_data, &euler_angle, 0.006f);
}

/**
 * @brief 根据欧拉角计算 PID 的目标值
 */
void App_flight_pid_process(void) {
  RC_Data_t *rc = App_get_rc_data();

  // 俯仰角控制 (遥控器中立位 1500，范围 1000-2000，转换到 +-10 度)
  pitch_pid.desire = (rc->pit - 1500) / 50.0f;
  pitch_pid.measure = euler_angle.pitch;
  gyro_y_pid.measure = (gyro_accel_data.gyro.gyro_y * 2000.0f / 32768.0f);
  Com_PID_Calc_Chain(&pitch_pid, &gyro_y_pid);

  // 横滚角控制 (遥控器中立位 1500，范围 1000-2000，转换到 +-10 度)
  roll_pid.desire = (rc->rol - 1500) / 50.0f;
  roll_pid.measure = euler_angle.roll;
  gyro_x_pid.measure = (gyro_accel_data.gyro.gyro_x * 2000.0f / 32768.0f);
  Com_PID_Calc_Chain(&roll_pid, &gyro_x_pid);

  // 偏航角控制 (遥控器中立位 1500，范围 1000-2000，转换到 +-10 度)
  yaw_pid.desire = (rc->yaw - 1500) / 50.0f;
  yaw_pid.measure = euler_angle.yaw;
  gyro_z_pid.measure = (gyro_accel_data.gyro.gyro_z * 2000.0f / 32768.0f);
  Com_PID_Calc_Chain(&yaw_pid, &gyro_z_pid);
}

/**
 * @brief 根据 PID 输出值控制电机
 *
 * 电机布局 (X 型四旋翼):
 *       前
 *        ^
 *   M1      M3
 *     \    /
 *      \  /
 *      /  \
 *     /    \
 *   M2      M4
 *
 * M1: 左上前 (CCW)  M2: 左下前 (CW)
 * M3: 右上前 (CW)   M4: 右下前 (CCW)
 */
void App_flight_control_motor(void) {
  RC_Data_t *rc = App_get_rc_data();
  int16_t thr_base = rc->thr; // 基础油门

  // 根据飞行状态计算电机输出
  int16_t motor_speed[4] = {0}; // M1, M2, M3, M4

  switch (flight_state) {
  case IDLE:
    // 怠速状态，所有电机停转
    for (int i = 0; i < 4; i++)
      motor_speed[i] = 0;
    break;

  case NORMAL:
  case FIX_HEIGHT:
    // 正常飞行/定高模式
    // 电机混合控制公式:
    // M1 (左上) = thr + pitch - roll - yaw
    // M2 (左下) = thr - pitch - roll + yaw
    // M3 (右上) = thr + pitch + roll + yaw
    // M4 (右下) = thr - pitch + roll - yaw
    motor_speed[0] =
        thr_base + gyro_y_pid.output - gyro_x_pid.output - gyro_z_pid.output;
    motor_speed[1] =
        thr_base - gyro_y_pid.output - gyro_x_pid.output + gyro_z_pid.output;
    motor_speed[2] =
        thr_base + gyro_y_pid.output + gyro_x_pid.output + gyro_z_pid.output;
    motor_speed[3] =
        thr_base - gyro_y_pid.output + gyro_x_pid.output - gyro_z_pid.output;

    // 定高模式下添加高度 PID 输出
    if (flight_state == FIX_HEIGHT) {
      for (int i = 0; i < 4; i++)
        motor_speed[i] += height_pid.output;
    }
    break;

  case FAIL:
    // 故障状态，逐渐降低转速
    for (int i = 0; i < 4; i++) {
      motor_speed[i] = -2; // 每周期减少 2
    }
    break;

  default:
    for (int i = 0; i < 4; i++)
      motor_speed[i] = 0;
    break;
  }

  // 限制电机速度范围 (0-2000)
  for (int i = 0; i < 4; i++) {
    motor_speed[i] = Com_limit(motor_speed[i], 2000, 0);
  }

  // 安全保护：油门最低时强制停转
  if (rc->thr < 50) {
    for (int i = 0; i < 4; i++)
      motor_speed[i] = 0;
  }

  // 设置电机速度
  // 注意：这里需要根据你的实际电机 TIM 通道配置来调用
  extern Motor_Struct left_top_motor;
  extern Motor_Struct left_bottom_motor;
  extern Motor_Struct right_top_motor;
  extern Motor_Struct right_bottom_motor;

  left_top_motor.speed = motor_speed[0];
  left_bottom_motor.speed = motor_speed[1];
  right_top_motor.speed = motor_speed[2];
  right_bottom_motor.speed = motor_speed[3];

  Int_motor_set_speed(&left_top_motor);
  Int_motor_set_speed(&left_bottom_motor);
  Int_motor_set_speed(&right_top_motor);
  Int_motor_set_speed(&right_bottom_motor);
}

/**
 * @brief 定高气压计 PID 处理 (24ms 周期调用)
 */
void App_flight_fix_height_pid_process(void) {
  // 读取气压计数据
  Int_SPL06_Read_Data(&spl06_data);

  // 目标高度：解锁定高时的高度
  height_pid.desire = (float)fix_height;

  // 当前高度：从气压计读取
  height_pid.measure = spl06_data.altitude;

  // 执行 PID 计算
  Com_PID_Calc(&height_pid);

  // 调试输出 (每次定高都记录)
  LOG_DEBUG("[Height] Target=%.2f m Current=%.2f m PID=%.2f", height_pid.desire,
            height_pid.measure, height_pid.output);
}
