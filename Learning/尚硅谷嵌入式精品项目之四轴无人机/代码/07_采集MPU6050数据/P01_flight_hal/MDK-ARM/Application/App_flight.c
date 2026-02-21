#include "App_flight.h"

Gyro_Accel_Struct gyro_accel_data = {0};
Euler_struct euler_angle = {0};

/**
 * @brief 根据陀螺仪测量的数据 计算出欧拉角
 *
 */
void App_flight_get_euler_angle(void)
{
    // 1. 使用MPU6050的硬件接口  得到六轴数据 
    Int_MPU6050_Get_Data(&gyro_accel_data);

    // 先打印角速度
    // debug_printf(":%d,%d,%d\n", gyro_accel_data.gyro.gyro_x, gyro_accel_data.gyro.gyro_y, gyro_accel_data.gyro.gyro_z);

    // 打印加速度
    debug_printf(":%d,%d,%d\n",gyro_accel_data.accel.accel_x, gyro_accel_data.accel.accel_y, gyro_accel_data.accel.accel_z);
}
