#ifndef __COMMON_IMU_H
#define __COMMON_IMU_H

#include "Com_debug.h"
#include "Com_config.h"
#include "math.h"

/* 表示四元数的结构体 */
typedef struct
{
    float q0;
    float q1;
    float q2;
    float q3;
} Quaternion_Struct;

extern float RtA;
extern float Gyro_G;
extern float Gyro_Gr;

/**
 * @brief 读取 MPU6050 的 6 轴数据，解算出当前的欧拉角
 * @param gyroAccel mpu6050 的 6 轴数据
 * @param eulerAngle 解算得到的欧拉角
 * @param dt 采样周期 (单位 s)
 */
void Common_IMU_GetEulerAngle(Gyro_Accel_Struct *gyroAccel,
                              Euler_struct *eulerAngle,
                              float dt);

/**
 * @brief 获取 Z 轴上的加速度 (已经归一化，会考虑 z 轴加速度的合成)
 * @return Z 轴加速度
 */
float Common_IMU_GetNormAccZ(void);

#endif
