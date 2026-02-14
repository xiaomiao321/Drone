#ifndef __APP_FLIGHT__
#define __APP_FLIGHT__

#include "Int_mpu6050.h"
#include "Com_debug.h"

/**
 * @brief 根据陀螺仪测量的数据 计算出欧拉角
 * 
 */
void App_flight_get_euler_angle(void);

#endif // __APP_FLIGHT__
