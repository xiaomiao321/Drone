#ifndef __INT_MPU6050__
#define __INT_MPU6050__

#include "i2c.h"
#include "Com_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h"

// 设备地址
#define MPU6050_ADDR 0x68
// 读写地址
#define MPU6050_ADDR_WRITE 0xD0
#define MPU6050_ADDR_READ 0xD1

#define MPU_SELF_TESTX_REG 0X0D   // 自检寄存器 X
#define MPU_SELF_TESTY_REG 0X0E   // 自检寄存器 Y
#define MPU_SELF_TESTZ_REG 0X0F   // 自检寄存器 Z
#define MPU_SELF_TESTA_REG 0X10   // 自检寄存器 A
#define MPU_SAMPLE_RATE_REG 0X19  // 采样频率分频器
#define MPU_CFG_REG 0X1A          // 配置寄存器
#define MPU_GYRO_CFG_REG 0X1B     // 陀螺仪配置寄存器
#define MPU_ACCEL_CFG_REG 0X1C    // 加速度计配置寄存器
#define MPU_MOTION_DET_REG 0X1F   // 运动检测阈值寄存器
#define MPU_FIFO_EN_REG 0X23      // FIFO 使能寄存器
#define MPU_I2CMST_CTRL_REG 0X24  // IIC 主控制器寄存器
#define MPU_I2CSLV0_ADDR_REG 0X25 // IIC 从机 0 设备地址寄存器
#define MPU_I2CSLV0_REG 0X26      // IIC 从机 0 数据地址寄存器
#define MPU_I2CSLV0_CTRL_REG 0X27 // IIC 从机 0 控制寄存器
#define MPU_I2CSLV1_ADDR_REG 0X28 // IIC 从机 1 设备地址寄存器
#define MPU_I2CSLV1_REG 0X29      // IIC 从机 1 数据地址寄存器
#define MPU_I2CSLV1_CTRL_REG 0X2A // IIC 从机 1 控制寄存器
#define MPU_I2CSLV2_ADDR_REG 0X2B // IIC 从机 2 设备地址寄存器
#define MPU_I2CSLV2_REG 0X2C      // IIC 从机 2 数据地址寄存器
#define MPU_I2CSLV2_CTRL_REG 0X2D // IIC 从机 2 控制寄存器
#define MPU_I2CSLV3_ADDR_REG 0X2E // IIC 从机 3 设备地址寄存器
#define MPU_I2CSLV3_REG 0X2F      // IIC 从机 3 数据地址寄存器
#define MPU_I2CSLV3_CTRL_REG 0X30 // IIC 从机 3 控制寄存器
#define MPU_I2CSLV4_ADDR_REG 0X31 // IIC 从机 4 设备地址寄存器
#define MPU_I2CSLV4_REG 0X32      // IIC 从机 4 数据地址寄存器
#define MPU_I2CSLV4_DO_REG 0X33   // IIC 从机 4 写数据寄存器
#define MPU_I2CSLV4_CTRL_REG 0X34 // IIC 从机 4 控制寄存器
#define MPU_I2CSLV4_DI_REG 0X35   // IIC 从机 4 读数据寄存器

#define MPU_I2CMST_STA_REG 0X36 // IIC 主状态寄存器
#define MPU_INTBP_CFG_REG 0X37  // 中断/旁路配置寄存器
#define MPU_INT_EN_REG 0X38     // 中断使能寄存器
#define MPU_INT_STA_REG 0X3A    // 中断状态寄存器

#define MPU_ACCEL_XOUTH_REG 0X3B // 加速度值，X 轴高 8 位寄存器
#define MPU_ACCEL_XOUTL_REG 0X3C // 加速度值，X 轴低 8 位寄存器
#define MPU_ACCEL_YOUTH_REG 0X3D // 加速度值，Y 轴高 8 位寄存器
#define MPU_ACCEL_YOUTL_REG 0X3E // 加速度值，Y 轴低 8 位寄存器
#define MPU_ACCEL_ZOUTH_REG 0X3F // 加速度值，Z 轴高 8 位寄存器
#define MPU_ACCEL_ZOUTL_REG 0X40 // 加速度值，Z 轴低 8 位寄存器

#define MPU_TEMP_OUTH_REG 0X41 // 温度值高 8 位寄存器
#define MPU_TEMP_OUTL_REG 0X42 // 温度值低 8 位寄存器

#define MPU_GYRO_XOUTH_REG 0X43 // 陀螺仪值，X 轴高 8 位寄存器
#define MPU_GYRO_XOUTL_REG 0X44 // 陀螺仪值，X 轴低 8 位寄存器
#define MPU_GYRO_YOUTH_REG 0X45 // 陀螺仪值，Y 轴高 8 位寄存器
#define MPU_GYRO_YOUTL_REG 0X46 // 陀螺仪值，Y 轴低 8 位寄存器
#define MPU_GYRO_ZOUTH_REG 0X47 // 陀螺仪值，Z 轴高 8 位寄存器
#define MPU_GYRO_ZOUTL_REG 0X48 // 陀螺仪值，Z 轴低 8 位寄存器

#define MPU_I2CSLV0_DO_REG 0X63 // IIC 从机 0 数据寄存器
#define MPU_I2CSLV1_DO_REG 0X64 // IIC 从机 1 数据寄存器
#define MPU_I2CSLV2_DO_REG 0X65 // IIC 从机 2 数据寄存器
#define MPU_I2CSLV3_DO_REG 0X66 // IIC 从机 3 数据寄存器

#define MPU_I2CMST_DELAY_REG 0X67 // IIC 主延时控制寄存器
#define MPU_SIGPATH_RST_REG 0X68  // 信号通道复位寄存器
#define MPU_MDETECT_CTRL_REG 0X69 // 运动检测控制寄存器
#define MPU_USER_CTRL_REG 0X6A    // 用户控制寄存器
#define MPU_PWR_MGMT1_REG 0X6B    // 电源管理寄存器 1
#define MPU_PWR_MGMT2_REG 0X6C    // 电源管理寄存器 2
#define MPU_FIFO_CNTH_REG 0X72    // FIFO 计数寄存器高 8 位
#define MPU_FIFO_CNTL_REG 0X73    // FIFO 计数寄存器低 8 位
#define MPU_FIFO_RW_REG 0X74      // FIFO 读写寄存器
#define MPU_DEVICE_ID_REG 0X75    // 设备 ID 寄存器

/**
 * @brief 初始化 MPU6050 芯片
 *
 */
void Int_MPU6050_Init(void);

/**
 * @brief 读取 MPU6050 寄存器（用于调试）
 * @param reg 寄存器地址
 * @param data 读取的数据
 */
void Int_MPU6050_Read_Reg(uint8_t reg, uint8_t *data);

/**
 * @brief 获取陀螺仪数据
 *
 * @param gyro
 */
void Int_MPU6050_Get_Gyro(Gyro_struct *gyro);

/**
 * @brief 获取加速度计数据
 *
 * @param acc
 */
void Int_MPU6050_Get_Acc(Accel_struct *acc);

/**
 * @brief 获取所有的数据
 *
 * @param data
 */
void Int_MPU6050_Get_Data(Gyro_Accel_Struct *data);

#endif // __INT_MPU6050__
