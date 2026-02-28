#include "Int_spl06.h"

/* ============================================================================
 * 全局数据变量定义（用于调试器查看）
 * ============================================================================
 */
SPL06_Data_Struct g_spl06_data = {0}; // 气压计数据（压力、温度、高度）
int32_t g_spl06_raw_pressure = 0;     // 原始压力值
int32_t g_spl06_raw_temperature = 0;  // 原始温度值
uint8_t g_spl06_data_ready = 0;       // 数据就绪标志（每次读取后翻转）

// 校准系数全局变量（添加到全局变量方便调试器查看）
SPL06_Coef_Struct g_spl06_coef = {0}; // 校准系数（调试用）
static SPL06_Coef_Struct spl06_coef = {0};

// 海平面气压基准 (Pa) - 标准大气压
#define SEA_LEVEL_PRESSURE 101325.0f

// 比例因子 (根据数据手册 Table 4)
// 过采样率对应的比例因子
#define SCALE_FACTOR_SINGLE 524288.0f
#define SCALE_FACTOR_2X 1572864.0f
#define SCALE_FACTOR_4X 3670016.0f
#define SCALE_FACTOR_8X 7864320.0f
#define SCALE_FACTOR_16X 253952.0f
#define SCALE_FACTOR_32X 516096.0f
#define SCALE_FACTOR_64X 1040384.0f
#define SCALE_FACTOR_128X 2088960.0f

/**
 * @brief 写寄存器
 * @param reg 寄存器地址
 * @param data 数据
 */
static void Int_SPL06_Write_Reg(uint8_t reg, uint8_t data) {
  // 硬件 I2C 写操作，使用 SPL06_ADDR_WRITE (0xEC)
  HAL_I2C_Mem_Write(&hi2c2, SPL06_ADDR_WRITE, reg, I2C_MEMADD_SIZE_8BIT, &data,
                    1, 100);
}

/**
 * @brief 读寄存器
 * @param reg 寄存器地址
 * @param data 读取的数据
 * @param size 读取字节数
 */
static void Int_SPL06_Read_Reg(uint8_t reg, uint8_t *data, uint16_t size) {
  // 硬件 I2C 读操作，使用 SPL06_ADDR_READ (0xED)
  HAL_I2C_Mem_Read(&hi2c2, SPL06_ADDR_READ, reg, I2C_MEMADD_SIZE_8BIT, data,
                   size, 100);
}

/**
 * @brief 读取校准系数
 */
static void Int_SPL06_Read_Coef(void) {
  uint8_t buf[3] = {0};

  // 读取 c0 (12 位) - 存储在 0x10 和 0x11 的高 4 位
  Int_SPL06_Read_Reg(0x10, buf, 2);
  spl06_coef.c0 = (int32_t)((int16_t)((buf[0] << 4) | (buf[1] >> 4)));
  // 符号扩展 (12 位有符号数)
  if (spl06_coef.c0 & 0x800)
    spl06_coef.c0 |= 0xF000;

  // 读取 c1 (12 位) - 存储在 0x11 的低 4 位和 0x12
  spl06_coef.c1 = (int32_t)((int16_t)(((buf[1] & 0x0F) << 8) | buf[2]));
  // 符号扩展 (12 位有符号数)
  if (spl06_coef.c1 & 0x800)
    spl06_coef.c1 |= 0xF000;

  // 读取 c00 (20 位) - 存储在 0x13, 0x14, 0x15 的高 4 位
  Int_SPL06_Read_Reg(0x13, buf, 3);
  spl06_coef.c00 =
      (int32_t)((int32_t)((buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4)));
  // 符号扩展 (20 位有符号数)
  if (spl06_coef.c00 & 0x80000)
    spl06_coef.c00 |= 0xFFF00000;

  // 读取 c10 (20 位) - 存储在 0x15 的低 4 位，0x16, 0x17
  // 需要重新读取 0x15, 0x16, 0x17
  Int_SPL06_Read_Reg(0x15, buf, 3); // buf[0]=0x15, buf[1]=0x16, buf[2]=0x17
  spl06_coef.c10 =
      (int32_t)((int32_t)(((buf[0] & 0x0F) << 16) | (buf[1] << 8) | buf[2]));
  // 符号扩展 (20 位有符号数)
  if (spl06_coef.c10 & 0x80000)
    spl06_coef.c10 |= 0xFFF00000;

  // 读取 c01 (16 位) - 存储在 0x18, 0x19
  Int_SPL06_Read_Reg(0x18, buf, 2);
  spl06_coef.c01 = (int32_t)((int16_t)((buf[0] << 8) | buf[1]));

  // 读取 c11 (16 位) - 存储在 0x1A, 0x1B
  Int_SPL06_Read_Reg(0x1A, buf, 2);
  spl06_coef.c11 = (int32_t)((int16_t)((buf[0] << 8) | buf[1]));

  // 读取 c20 (16 位) - 存储在 0x1C, 0x1D
  Int_SPL06_Read_Reg(0x1C, buf, 2);
  spl06_coef.c20 = (int32_t)((int16_t)((buf[0] << 8) | buf[1]));

  // 读取 c21 (16 位) - 存储在 0x1E, 0x1F
  Int_SPL06_Read_Reg(0x1E, buf, 2);
  spl06_coef.c21 = (int32_t)((int16_t)((buf[0] << 8) | buf[1]));

  // 读取 c30 (16 位) - 存储在 0x20, 0x21
  Int_SPL06_Read_Reg(0x20, buf, 2);
  spl06_coef.c30 = (int32_t)((int16_t)((buf[0] << 8) | buf[1]));
  
  // 复制到全局变量供调试器查看
  g_spl06_coef = spl06_coef;
}

/**
 * @brief 计算补偿后的压力
 * @param raw_pressure 原始压力值
 * @param raw_temperature 原始温度值
 * @return 补偿后的压力 (Pa)
 */
static float Int_SPL06_Calc_Pressure(int32_t raw_pressure,
                                     int32_t raw_temperature) {
  // 根据配置获取比例因子
  // 这里使用 64 次过采样的比例因子
  float kP = SCALE_FACTOR_64X;
  float kT = SCALE_FACTOR_64X;

  // 缩放原始测量值
  float Praw_sc = (float)raw_pressure / kP;
  float Traw_sc = (float)raw_temperature / kT;

  // 计算补偿后的压力
  // Pcomp(Pa) = c00 + Praw_sc*(c10 + Praw_sc*(c20 + Praw_sc*c30)) + Traw_sc*c01
  // + Traw_sc*Praw_sc*(c11 + Praw_sc*c21)
  float pcomp =
      spl06_coef.c00 +
      Praw_sc * (spl06_coef.c10 +
                 Praw_sc * (spl06_coef.c20 + Praw_sc * spl06_coef.c30)) +
      Traw_sc * spl06_coef.c01 +
      Traw_sc * Praw_sc * (spl06_coef.c11 + Praw_sc * spl06_coef.c21);

  return pcomp;
}

/**
 * @brief 计算补偿后的温度
 * @param raw_temperature 原始温度值
 * @return 补偿后的温度 (°C)
 */
static float Int_SPL06_Calc_Temperature(int32_t raw_temperature) {
  // 根据配置获取比例因子
  float kT = SCALE_FACTOR_64X;

  // 缩放原始测量值
  float Traw_sc = (float)raw_temperature / kT;

  // 计算补偿后的温度
  // Tcomp(°C) = c0*0.5 + c1*Traw_sc
  float tcomp = spl06_coef.c0 * 0.5f + spl06_coef.c1 * Traw_sc;

  return tcomp;
}

/**
 * @brief 软件复位 SPL06
 */
void Int_SPL06_Soft_Reset(void) {
  // 写入 0x89 到复位寄存器执行软复位
  Int_SPL06_Write_Reg(SPL06_RESET, 0x89);
  // 等待复位完成
  HAL_Delay(10);
}

/**
 * @brief 读取产品 ID
 * @return 产品 ID (应为 0x10)
 */
uint8_t Int_SPL06_Read_Product_ID(void) {
  uint8_t id = 0;
  Int_SPL06_Read_Reg(SPL06_ID, &id, 1);
  return id;
}

/**
 * @brief 初始化 SPL06-001 气压计
 * @return 0: 成功 1: 失败
 */
uint8_t Int_SPL06_Init(void) {
  uint8_t id = 0;
  uint8_t status = 0;
  uint32_t timeout = 0;

  // 1. 读取产品 ID 验证通信
  id = Int_SPL06_Read_Product_ID();
  if (id != 0x10) {
    // ID 不匹配，可能是通信问题
    // debug_printf("SPL06 ID error: 0x%02X\r\n", id);
    return 1;
  }

  // 2. 软件复位
  Int_SPL06_Soft_Reset();

  // 3. 等待校准系数就绪
  timeout = 100;
  while (timeout--) {
    Int_SPL06_Read_Reg(SPL06_MEAS_CFG, &status, 1);
    if (status & 0x80) // COEF_RDY bit
      break;
    HAL_Delay(1);
  }

  if (timeout == 0) {
    // debug_printf("SPL06 coef timeout\r\n");
    return 1;
  }

  // 4. 读取校准系数
  Int_SPL06_Read_Coef();

  // 5. 等待传感器就绪
  timeout = 100;
  while (timeout--) {
    Int_SPL06_Read_Reg(SPL06_MEAS_CFG, &status, 1);
    if (status & 0x40) // SENSOR_RDY bit
      break;
    HAL_Delay(1);
  }

  if (timeout == 0) {
    // debug_printf("SPL06 sensor timeout\r\n");
    return 1;
  }

  // 6. 配置压力测量
  // PM_PRC = 6 (64 次过采样，高精度), PM_RATE = 4 (16 次/秒)
  Int_SPL06_Write_Reg(SPL06_PRS_CFG, (SPL06_PM_PRC << 4) | SPL06_PM_RATE);

  // 7. 配置温度测量
  // TMP_EXT = 1 (使用外部传感器), TMP_PRC = 1 (单次)
  Int_SPL06_Write_Reg(SPL06_TMP_CFG,
                      0x80 | (SPL06_TMP_PRC << 4) | SPL06_TMP_RATE);

  // 8. 配置中断和 FIFO (禁用)
  Int_SPL06_Write_Reg(SPL06_CFG_REG, 0x00);

  // 9. 启动连续测量模式 (压力 + 温度)
  // MEAS_CTRL = 7 (连续压力和温度测量)
  Int_SPL06_Write_Reg(SPL06_MEAS_CFG, 0x07);

  // 等待第一次测量完成
  HAL_Delay(50);

  //   debug_printf("SPL06 init OK\r\n");
  return 0;
}

/**
 * @brief 读取压力数据 (原始值)
 * @return 24 位压力原始值 (有符号)
 */
int32_t Int_SPL06_Read_Pressure_Raw(void) {
  uint8_t buf[3] = {0};
  int32_t pressure = 0;

  Int_SPL06_Read_Reg(SPL06_PRS_B2, buf, 3);

  // 组合 24 位有符号数
  pressure = ((int32_t)buf[0] << 16) | ((int32_t)buf[1] << 8) | buf[2];

  // 符号扩展 (24 位有符号数转 32 位)
  if (pressure & 0x800000)
    pressure |= 0xFF000000;

  return pressure;
}

/**
 * @brief 读取温度数据 (原始值)
 * @return 24 位温度原始值 (有符号)
 */
int32_t Int_SPL06_Read_Temperature_Raw(void) {
  uint8_t buf[3] = {0};
  int32_t temperature = 0;

  Int_SPL06_Read_Reg(SPL06_TMP_B2, buf, 3);

  // 组合 24 位有符号数
  temperature = ((int32_t)buf[0] << 16) | ((int32_t)buf[1] << 8) | buf[2];

  // 符号扩展 (24 位有符号数转 32 位)
  if (temperature & 0x800000)
    temperature |= 0xFF000000;

  return temperature;
}

/**
 * @brief 读取补偿后的压力和温度
 * @param data 数据存储结构体指针
 */
void Int_SPL06_Read_Data(SPL06_Data_Struct *data) {
  int32_t raw_pressure;
  int32_t raw_temperature;

  // 读取原始数据
  raw_pressure = Int_SPL06_Read_Pressure_Raw();
  raw_temperature = Int_SPL06_Read_Temperature_Raw();

  // 计算补偿后的值
  data->pressure = Int_SPL06_Calc_Pressure(raw_pressure, raw_temperature);
  data->temperature = Int_SPL06_Calc_Temperature(raw_temperature);

  // 计算高度
  data->altitude = Int_SPL06_Calc_Altitude(data->pressure, SEA_LEVEL_PRESSURE);
}

/**
 * @brief 根据气压计算海拔高度
 * @param pressure 当前气压 (Pa)
 * @param sea_level_pressure 海平面气压 (Pa)
 * @return 海拔高度 (米)
 *
 * 公式：Altitude = 44330 * (1 - (P/P0)^(1/5.255))
 */
float Int_SPL06_Calc_Altitude(float pressure, float sea_level_pressure) {
  // 检查气压值是否有效（正常范围 80000 - 110000 Pa）
  if (pressure <= 0.0f || pressure > 110000.0f) {
    return 0.0f;  // 无效气压返回 0
  }
  
  float ratio = pressure / sea_level_pressure;
  
  // 检查 ratio 是否为正数，避免 powf 返回 NaN
  if (ratio <= 0.0f) {
    return 0.0f;
  }
  
  float altitude = 44330.0f * (1.0f - powf(ratio, 1.0f / 5.255f));
  return altitude;
}

/**
 * @brief 获取当前高度
 * @return 高度 (米)
 */
float Int_SPL06_Get_Altitude(void) {
  SPL06_Data_Struct data;
  Int_SPL06_Read_Data(&data);
  return data.altitude;
}

/**
 * @brief 测试读取 SPL06 数据（更新全局变量）
 * @return 1 成功，0 失败
 */
uint8_t Int_SPL06_Test_Read(void) {
  uint8_t buf[6] = {0};
  HAL_StatusTypeDef status;
  int32_t raw_pressure;
  int32_t raw_temperature;

  // 连续读取 6 个寄存器（3 字节压力 + 3 字节温度）
  status = HAL_I2C_Mem_Read(&hi2c2, SPL06_ADDR_READ, SPL06_PRS_B2,
                            I2C_MEMADD_SIZE_8BIT, buf, 6, 100);

  if (status != HAL_OK) {
    return 0;
  }

  // 解析原始压力数据（24 位有符号）
  raw_pressure = ((int32_t)buf[0] << 16) | ((int32_t)buf[1] << 8) | buf[2];
  if (raw_pressure & 0x800000)
    raw_pressure |= 0xFF000000;

  // 解析原始温度数据（24 位有符号）
  raw_temperature = ((int32_t)buf[3] << 16) | ((int32_t)buf[4] << 8) | buf[5];
  if (raw_temperature & 0x800000)
    raw_temperature |= 0xFF000000;

  // 保存原始数据
  g_spl06_raw_pressure = raw_pressure;
  g_spl06_raw_temperature = raw_temperature;

  // 计算补偿后的值
  g_spl06_data.pressure =
      Int_SPL06_Calc_Pressure(raw_pressure, raw_temperature);
  g_spl06_data.temperature = Int_SPL06_Calc_Temperature(raw_temperature);
  g_spl06_data.altitude =
      Int_SPL06_Calc_Altitude(g_spl06_data.pressure, SEA_LEVEL_PRESSURE);

  // 翻转就绪标志
  g_spl06_data_ready = !g_spl06_data_ready;

  return 1;
}
