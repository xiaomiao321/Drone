#include "Com_imu.h"

/* ============================欧拉角计算================================== */
/* ===============================初始化===================================== */

/* 转换欧拉角用到的 3 个常量 */
float RtA = 57.2957795f;       // 弧度->度
float Gyro_G = 4000.0f / 65536.0f;  // 陀螺仪初始量程+-2000 度/s 1/(65536 / 4000) = 0.03051756*2
float Gyro_Gr = 4000.0f / 65536.0f / 180.0f * 3.1415926f; // 度/s 转弧度/s

#define squa(Sq) (((float)Sq) * ((float)Sq)) /* 计算平方 */

/**
 * @brief 快速 inverse square root 1/sqrt(num)
 * @param number 输入值
 * @return 1/sqrt(number)
 */
static float Q_rsqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y)); // 1st iteration 牛顿迭代法
    return y;
}

static float normAccz; /* z 轴垂直朝上的加速度 */

/**
 * @brief 读取 MPU6050 的 6 轴数据，解算出当前的欧拉角
 * @param gyroAccel mpu6050 的 6 轴数据
 * @param eulerAngle 解算得到的欧拉角
 * @param dt 采样周期 (单位 s)
 */
void Common_IMU_GetEulerAngle(Gyro_Accel_Struct *gyroAccel,
                              Euler_struct *eulerAngle,
                              float dt)
{
    volatile struct V
    {
        float x;
        float y;
        float z;
    } Gravity, Acc, Gyro, AccGravity;

    static struct V GyroIntegError = {0};
    static float KpDef = 0.8f;
    static float KiDef = 0.0003f;
    static Quaternion_Struct NumQ = {1, 0, 0, 0};
    float q0_t, q1_t, q2_t, q3_t;
    float NormQuat;
    float HalfTime = dt * 0.5f;

    // 获取当前姿态解算中的重力向量
    Gravity.x = 2 * (NumQ.q1 * NumQ.q3 - NumQ.q0 * NumQ.q2);
    Gravity.y = 2 * (NumQ.q0 * NumQ.q1 + NumQ.q2 * NumQ.q3);
    Gravity.z = 1 - 2 * (NumQ.q1 * NumQ.q1 + NumQ.q2 * NumQ.q2);

    // 加速度归一化
    NormQuat = Q_rsqrt(squa(gyroAccel->accel.accel_x) +
                       squa(gyroAccel->accel.accel_y) +
                       squa(gyroAccel->accel.accel_z));

    Acc.x = gyroAccel->accel.accel_x * NormQuat;
    Acc.y = gyroAccel->accel.accel_y * NormQuat;
    Acc.z = gyroAccel->accel.accel_z * NormQuat;

    // 叉乘得到误差
    AccGravity.x = (Acc.y * Gravity.z - Acc.z * Gravity.y);
    AccGravity.y = (Acc.z * Gravity.x - Acc.x * Gravity.z);
    AccGravity.z = (Acc.x * Gravity.y - Acc.y * Gravity.x);

    // 积分得到陀螺仪偏差
    GyroIntegError.x += AccGravity.x * KiDef;
    GyroIntegError.y += AccGravity.y * KiDef;
    GyroIntegError.z += AccGravity.z * KiDef;

    // 陀螺仪融合加速度计偏差值
    Gyro.x = gyroAccel->gyro.gyro_x * Gyro_Gr + KpDef * AccGravity.x + GyroIntegError.x;
    Gyro.y = gyroAccel->gyro.gyro_y * Gyro_Gr + KpDef * AccGravity.y + GyroIntegError.y;
    Gyro.z = gyroAccel->gyro.gyro_z * Gyro_Gr + KpDef * AccGravity.z + GyroIntegError.z;

    // 一阶龙格库塔法，更新四元数
    q0_t = (-NumQ.q1 * Gyro.x - NumQ.q2 * Gyro.y - NumQ.q3 * Gyro.z) * HalfTime;
    q1_t = (NumQ.q0 * Gyro.x - NumQ.q3 * Gyro.y + NumQ.q2 * Gyro.z) * HalfTime;
    q2_t = (NumQ.q3 * Gyro.x + NumQ.q0 * Gyro.y - NumQ.q1 * Gyro.z) * HalfTime;
    q3_t = (-NumQ.q2 * Gyro.x + NumQ.q1 * Gyro.y + NumQ.q0 * Gyro.z) * HalfTime;

    NumQ.q0 += q0_t;
    NumQ.q1 += q1_t;
    NumQ.q2 += q2_t;
    NumQ.q3 += q3_t;

    // 四元数归一化
    NormQuat = Q_rsqrt(squa(NumQ.q0) + squa(NumQ.q1) + squa(NumQ.q2) + squa(NumQ.q3));
    NumQ.q0 *= NormQuat;
    NumQ.q1 *= NormQuat;
    NumQ.q2 *= NormQuat;
    NumQ.q3 *= NormQuat;

    /* 计算地理系下的 Z 轴分量 */
    float vecxZ = 2 * NumQ.q0 * NumQ.q2 - 2 * NumQ.q1 * NumQ.q3;
    float vecyZ = 2 * NumQ.q2 * NumQ.q3 + 2 * NumQ.q0 * NumQ.q1;
    float veczZ = 1 - 2 * NumQ.q1 * NumQ.q1 - 2 * NumQ.q2 * NumQ.q2;

    float yaw_G = gyroAccel->gyro.gyro_z * Gyro_G; // Z 轴角速度原始值 转换为 Z 角度/s
    if ((yaw_G > 0.5f) || (yaw_G < -0.5f))        // 值太小则认为是噪声，不更新偏航角
    {
        eulerAngle->yaw += yaw_G * dt; // 角速度积分成偏航角
    }

    eulerAngle->pitch = asin(vecxZ) * RtA; // 俯仰角
    eulerAngle->roll = atan2f(vecyZ, veczZ) * RtA; // 横滚角

    normAccz = gyroAccel->accel.accel_x * vecxZ + gyroAccel->accel.accel_y * vecyZ + gyroAccel->accel.accel_z * veczZ;
}

/**
 * @brief 获取 Z 轴上的加速度 (已经归一化，会考虑 z 轴加速度的合成)
 * @return Z 轴加速度
 */
float Common_IMU_GetNormAccZ(void)
{
    return normAccz;
}

/* ======================欧拉角计算================================== */
/* ========================结束===================================== */
