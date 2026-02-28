#ifndef __INT_SPL06__
#define __INT_SPL06__

#include "i2c.h"
// #include "Com_debug.h"
#include "Com_config.h"
#include "math.h"

/* ============================================================================
 * SPL06-001 设备地址
 * 注意：SDO 引脚决定地址
 * - SDO 接地 (GND):    7 位地址 0x76, 写 0xEC, 读 0xED
 * - SDO 接电源 (VDDIO): 7 位地址 0x77, 写 0xEE, 读 0xEF
 * 本模块 SDO 接地，使用 0x76
 * ============================================================================
 */
#define SPL06_ADDR 0x76       // 7 位设备地址 (SDO 接地)
#define SPL06_ADDR_WRITE 0xEC // 写地址 (0x76 << 1)
#define SPL06_ADDR_READ  0xED // 读地址 (0x76 << 1 | 1)

// 寄存器地址
#define SPL06_PRS_B2 0x00   // 压力数据高 8 位
#define SPL06_PRS_B1 0x01   // 压力数据中 8 位
#define SPL06_PRS_B0 0x02   // 压力数据低 8 位
#define SPL06_TMP_B2 0x03   // 温度数据高 8 位
#define SPL06_TMP_B1 0x04   // 温度数据中 8 位
#define SPL06_TMP_B0 0x05   // 温度数据低 8 位
#define SPL06_PRS_CFG 0x06  // 压力配置寄存器
#define SPL06_TMP_CFG 0x07  // 温度配置寄存器
#define SPL06_MEAS_CFG 0x08 // 测量配置和状态寄存器
#define SPL06_CFG_REG 0x09  // 中断和 FIFO 配置寄存器
#define SPL06_INT_STS 0x0A  // 中断状态寄存器
#define SPL06_FIFO_STS 0x0B // FIFO 状态寄存器
#define SPL06_RESET 0x0C    // 复位寄存器
#define SPL06_ID 0x0D       // 产品 ID 寄存器
#define SPL06_COEF 0x10     // 校准系数起始地址

// 校准系数结构体
typedef struct {
  int32_t c0;  // 温度补偿系数 c0 (12 位)
  int32_t c1;  // 温度补偿系数 c1 (12 位)
  int32_t c00; // 压力补偿系数 c00 (20 位)
  int32_t c10; // 压力补偿系数 c10 (20 位)
  int32_t c01; // 压力补偿系数 c01 (16 位)
  int32_t c11; // 压力补偿系数 c11 (16 位)
  int32_t c20; // 压力补偿系数 c20 (16 位)
  int32_t c21; // 压力补偿系数 c21 (16 位)
  int32_t c30; // 压力补偿系数 c30 (16 位)
} SPL06_Coef_Struct;

// SPL06 数据结构
typedef struct {
  float pressure;    // 补偿后的压力 (Pa)
  float temperature; // 补偿后的温度 (°C)
  float altitude;    // 计算的高度 (m)
} SPL06_Data_Struct;

// 气压计配置
#define SPL06_PM_PRC 0x06   // 压力过采样率：64 次 (高精度)
#define SPL06_PM_RATE 0x04  // 压力测量率：16 次/秒
#define SPL06_TMP_PRC 0x05  // 温度过采样率：1 次 (单次)
#define SPL06_TMP_RATE 0x00 // 温度测量率：1 次/秒

/**
 * @brief 初始化 SPL06-001 气压计
 * @return 0: 成功 1: 失败
 */
uint8_t Int_SPL06_Init(void);

/**
 * @brief 读取压力数据 (原始值)
 * @return 24 位压力原始值
 */
int32_t Int_SPL06_Read_Pressure_Raw(void);

/**
 * @brief 读取温度数据 (原始值)
 * @return 24 位温度原始值
 */
int32_t Int_SPL06_Read_Temperature_Raw(void);

/**
 * @brief 读取补偿后的压力和温度
 * @param data 数据存储结构体指针
 */
void Int_SPL06_Read_Data(SPL06_Data_Struct *data);

/**
 * @brief 根据气压计算海拔高度
 * @param pressure 当前气压 (Pa)
 * @param sea_level_pressure 海平面气压 (Pa)，默认 101325 Pa
 * @return 海拔高度 (米)
 */
float Int_SPL06_Calc_Altitude(float pressure, float sea_level_pressure);

/**
 * @brief 获取当前高度
 * @return 高度 (米)
 */
float Int_SPL06_Get_Altitude(void);

/**
 * @brief 软件复位 SPL06
 */
void Int_SPL06_Soft_Reset(void);

/**
 * @brief 读取产品 ID
 * @return 产品 ID (应为 0x10)
 */
uint8_t Int_SPL06_Read_Product_ID(void);

/* ============================================================================
 * 全局数据变量（用于调试器查看）
 * ============================================================================
 */
extern SPL06_Data_Struct g_spl06_data;     // 气压计数据（压力、温度、高度）
extern int32_t g_spl06_raw_pressure;       // 原始压力值
extern int32_t g_spl06_raw_temperature;    // 原始温度值
extern uint8_t g_spl06_data_ready;         // 数据就绪标志（每次读取后翻转）
extern SPL06_Coef_Struct g_spl06_coef;     // 校准系数（调试用）

/**
 * @brief 测试读取 SPL06 数据（更新全局变量）
 * @return 1 成功，0 失败
 */
uint8_t Int_SPL06_Test_Read(void);

#endif // __INT_SPL06__
