#include "Int_mpu6050.h"

/* ============================================================================
 * 全局数据变量定义（用于调试器查看）
 * ============================================================================
 */
Gyro_struct g_mpu6050_gyro = {0};       // 陀螺仪原始数据
Accel_struct g_mpu6050_accel = {0};     // 加速度计原始数据
Gyro_Accel_Struct g_mpu6050_data = {0}; // 完整数据
uint8_t g_mpu6050_data_ready = 0;       // 数据就绪标志（每次读取后翻转）

// 加速度计偏置值
int32_t acc_x_offset = 0;
int32_t acc_y_offset = 0;
int32_t acc_z_offset = 0;

int32_t gyro_x_offset = 0;
int32_t gyro_y_offset = 0;
int32_t gyro_z_offset = 0;

/**
 * @brief 写寄存器
 *
 * @param reg 寄存器地址
 * @param data 寄存器值
 */
static void Int_MPU6050_Write_Reg(uint8_t reg, uint8_t data) {
  // HAL 库固有模式 I2C 写操作
  // 1. 句柄 (hi2c1) 2. 设备地址 (0x68) 3. 寄存器地址 reg 4. 寄存器地址位宽 5.
  // 写数据的地址 6. 写字节个数 7. 超时时间
  HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR_WRITE, reg, I2C_MEMADD_SIZE_8BIT,
                    &data, 1, 100);
}

/**
 * @brief 读寄存器
 *
 * @param reg 寄存器地址
 * @param data 读取的数据
 */
static void Int_MPU6050_Read_Reg(uint8_t reg, uint8_t *data) {
  // 1. 句柄 (hi2c1) 2. 设备地址 (0x68) 3. 寄存器地址 reg 4. 寄存器地址位宽 5.
  // 读数据的地址 6. 读字节个数 7. 超时时间
  HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR_READ, reg, I2C_MEMADD_SIZE_8BIT, data,
                   1, 100);
}

/**
 * @brief 批量读取寄存器（连续读取）
 * @param reg 起始寄存器地址
 * @param data 读取的数据缓冲区
 * @param len 读取字节数
 */
static void Int_MPU6050_Read_Regs(uint8_t reg, uint8_t *data, uint16_t len) {
  HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR_READ, reg, I2C_MEMADD_SIZE_8BIT, data,
                   len, 100);
}

/**
 * @brief 在初始化 MPU6050 之后 对 MPU6050 进行偏置校准
 *
 */
static void Int_MPU6050_calculate_offset(void) {
  // 1. 等待飞机停止平稳
  // 判断飞机是否停止平稳的标准：前后两次加速度计的差值小于 200，连续 100 次
  Accel_struct current_accel = {0};
  Accel_struct last_accel = {0};
  uint8_t count = 0;
  Int_MPU6050_Get_Acc(&last_accel);

  while (count < 100) {
    Int_MPU6050_Get_Acc(&current_accel);
    // 判断飞机是否平稳 选用的参数要小 否则一直无法判断为平稳
    if (abs(current_accel.accel_x - last_accel.accel_x) < 400 &&
        abs(current_accel.accel_y - last_accel.accel_y) < 400 &&
        abs(current_accel.accel_z - last_accel.accel_z) < 400) {
      count++;
    } else {
      count = 0;
    }
    last_accel = current_accel;
    // vTaskDelay(6);
  }

  // 2. 飞机已经平稳 开始进行偏置校准
  Gyro_Accel_Struct gyro_accel_data = {0};
  int32_t acc_x_sum = 0;
  int32_t acc_y_sum = 0;
  int32_t acc_z_sum = 0;

  int32_t gyro_x_sum = 0;
  int32_t gyro_y_sum = 0;
  int32_t gyro_z_sum = 0;
  for (uint8_t i = 0; i < 100; i++) {
    // 重新读取加速度和角速度
    Int_MPU6050_Get_Data(&gyro_accel_data);
    acc_x_sum += (gyro_accel_data.accel.accel_x - 0);
    acc_y_sum += (gyro_accel_data.accel.accel_y - 0);
    // Z 轴加速度的初始值应该有 1g  => 大约是 16384
    acc_z_sum += (gyro_accel_data.accel.accel_z - 16384);

    gyro_x_sum += (gyro_accel_data.gyro.gyro_x - 0);
    gyro_y_sum += (gyro_accel_data.gyro.gyro_y - 0);
    gyro_z_sum += (gyro_accel_data.gyro.gyro_z - 0);

    // 每次采集数据需要时间延迟  首尾采集取平均值作为偏置
    // vTaskDelay(6);
  }

  acc_x_offset = acc_x_sum / 100;
  acc_y_offset = acc_y_sum / 100;
  acc_z_offset = acc_z_sum / 100;

  gyro_x_offset = gyro_x_sum / 100;
  gyro_y_offset = gyro_y_sum / 100;
  gyro_z_offset = gyro_z_sum / 100;
}

/**
 * @brief 初始化 MPU6050 芯片
 *
 */
void Int_MPU6050_Init(void) {
  // 1. 复位芯片 清除当前寄存器的值 => 写电源管理寄存器 1 => DEVICE_RESET
  Int_MPU6050_Write_Reg(0x6B, 0x80);
  uint8_t data = 0;
  // 等待复位之后 0x6B 寄存器的值为 0x40 表示当前为低功耗模式
  while (data != 0x40) {
    Int_MPU6050_Read_Reg(0x6B, &data);
  }
  // 唤醒 MPU6050  进入到正常工作状态
  Int_MPU6050_Write_Reg(0x6B, 0x00);

  // 2. 选择合适的量程 在够用的范围内 选择越小越好 => 精度更高
  // 2.1 写陀螺仪量程为 +-2000 度/s
  Int_MPU6050_Write_Reg(0x1B, 3 << 3);

  // 2.2 写加速度计量程为 +-2g
  Int_MPU6050_Write_Reg(0x1C, 0x00);

  // 3. 关闭中断使能  因为不配置中断
  Int_MPU6050_Write_Reg(0x38, 0x00);

  // 4. 用户控制寄存器 禁止 FIFO 模块  禁止扩展 I2C
  Int_MPU6050_Write_Reg(0x6A, 0x00);

  // 5. 设置采样频率 => 采集的是加速度计和陀螺仪数据 => 默认频率 1000HZ => 1ms
  // 读取一次 采样频率应该高于后续数据的使用频率  否则丢失数据 => 奈奎斯特
  // 采样频率 >= 2 倍使用频率 设置采样分频为 2 => 即写入值为 2-1
  Int_MPU6050_Write_Reg(0x19, 0x01);

  // 6. 设置低通滤波截止值为 184Hz 188Hz => 1
  Int_MPU6050_Write_Reg(0x1A, 1);

  // 7. 选择使用的系统时钟为 PLL 时钟源
  Int_MPU6050_Write_Reg(0x6B, 0x01);

  // 8. 使能加速度计和陀螺仪
  Int_MPU6050_Write_Reg(0x6C, 0x00);

  // 9. 执行偏置校准
  Int_MPU6050_calculate_offset();
}

/**
 * @brief 获取陀螺仪数据  => 需要减去偏置校准 => 直接输出原始值
 *
 * @param gyro
 */
void Int_MPU6050_Get_Gyro(Gyro_struct *gyro) {
  // 存储陀螺仪的寄存器地址从 0x43 开始 连续 8 位 先后  XYZ 的顺序
  uint8_t high = 0;
  uint8_t low = 0;
  // X 轴
  Int_MPU6050_Read_Reg(MPU_GYRO_XOUTH_REG, &high);
  Int_MPU6050_Read_Reg(MPU_GYRO_XOUTL_REG, &low);
  gyro->gyro_x = (int16_t)((high << 8) | low) - gyro_x_offset;
  // Y 轴
  Int_MPU6050_Read_Reg(MPU_GYRO_YOUTH_REG, &high);
  Int_MPU6050_Read_Reg(MPU_GYRO_YOUTL_REG, &low);
  gyro->gyro_y = (int16_t)((high << 8) | low) - gyro_y_offset;
  // Z 轴
  Int_MPU6050_Read_Reg(MPU_GYRO_ZOUTH_REG, &high);
  Int_MPU6050_Read_Reg(MPU_GYRO_ZOUTL_REG, &low);
  gyro->gyro_z = (int16_t)((high << 8) | low) - gyro_z_offset;
}

/**
 * @brief 获取加速度计数据  直接读取 需要偏置校准  Z 轴值约为 16384
 *
 * @param acc
 */
void Int_MPU6050_Get_Acc(Accel_struct *acc) {
  uint8_t high = 0;
  uint8_t low = 0;
  // X 轴
  Int_MPU6050_Read_Reg(MPU_ACCEL_XOUTH_REG, &high);
  Int_MPU6050_Read_Reg(MPU_ACCEL_XOUTL_REG, &low);
  acc->accel_x = (int16_t)((high << 8) | low) - acc_x_offset;
  // Y 轴
  Int_MPU6050_Read_Reg(MPU_ACCEL_YOUTH_REG, &high);
  Int_MPU6050_Read_Reg(MPU_ACCEL_YOUTL_REG, &low);
  acc->accel_y = (int16_t)((high << 8) | low) - acc_y_offset;
  // Z 轴
  Int_MPU6050_Read_Reg(MPU_ACCEL_ZOUTH_REG, &high);
  Int_MPU6050_Read_Reg(MPU_ACCEL_ZOUTL_REG, &low);
  acc->accel_z = (int16_t)((high << 8) | low) - acc_z_offset;
}

/**
 * @brief 获取所有的数据
 *
 * @param data
 */
void Int_MPU6050_Get_Data(Gyro_Accel_Struct *data) {
  Int_MPU6050_Get_Gyro(&data->gyro);
  Int_MPU6050_Get_Acc(&data->accel);
}

/**
 * @brief 测试读取 MPU6050 数据（更新全局变量）
 * @return 1 成功，0 失败
 */
uint8_t Int_MPU6050_Test_Read(void) {
  uint8_t buf[14] = {0};
  HAL_StatusTypeDef status;

  // 连续读取 14 个寄存器（加速度 + 温度 + 陀螺仪）
  status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR_READ, MPU_ACCEL_XOUTH_REG,
                            I2C_MEMADD_SIZE_8BIT, buf, 14, 100);

  if (status != HAL_OK) {
    return 0;
  }

  // 解析加速度计数据
  g_mpu6050_accel.accel_x = (int16_t)((buf[0] << 8) | buf[1]) - acc_x_offset;
  g_mpu6050_accel.accel_y = (int16_t)((buf[2] << 8) | buf[3]) - acc_y_offset;
  g_mpu6050_accel.accel_z = (int16_t)((buf[4] << 8) | buf[5]) - acc_z_offset;

  // 解析温度数据（可选）
  // int16_t temp = (int16_t)((buf[6] << 8) | buf[7]);

  // 解析陀螺仪数据
  g_mpu6050_gyro.gyro_x = (int16_t)((buf[8] << 8) | buf[9]) - gyro_x_offset;
  g_mpu6050_gyro.gyro_y = (int16_t)((buf[10] << 8) | buf[11]) - gyro_y_offset;
  g_mpu6050_gyro.gyro_z = (int16_t)((buf[12] << 8) | buf[13]) - gyro_z_offset;

  // 更新完整数据结构
  g_mpu6050_data.accel = g_mpu6050_accel;
  g_mpu6050_data.gyro = g_mpu6050_gyro;

  // 翻转就绪标志
  g_mpu6050_data_ready = !g_mpu6050_data_ready;

  return 1;
}
