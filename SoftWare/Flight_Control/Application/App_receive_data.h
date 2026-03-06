#ifndef __APP_RECEIVE_DATA__
#define __APP_RECEIVE_DATA__

#include "Int_nRF24L01.h"
#include "Com_config.h"

// 数据帧头校验值 (用于 sg 协议)
#define FRAME_HEAD_CHECK_1 's'
#define FRAME_HEAD_CHECK_2 'g'
#define FRAME_HEAD_CHECK_3 'g'

// 遥控器超时
// 通信任务周期 6ms，100 次 = 600ms 无数据才认为失联
#define MAX_RETRY_TIMES 100

// RC 通道数据结构 (ANO_DT 协议)
typedef struct RC_Data_s
{
    uint16_t thr;  // 油门 (1000-2000)
    uint16_t yaw;  // 偏航（方向）(1000-2000)
    uint16_t rol;  // 横滚 (1000-2000)
    uint16_t pit;  // 俯仰 (1000-2000)
    uint16_t aux1; // 辅助通道 1 (1000-2000)
    uint16_t aux2; // 辅助通道 2 (1000-2000)
    uint16_t aux3; // 辅助通道 3 (1000-2000)
    uint16_t aux4; // 辅助通道 4 (1000-2000)
} RC_Data_t;

/**
 * @brief 解析 ANO_DT 遥控器数据协议
 * @param buf 接收缓冲区
 * @param len 缓冲区长度
 * @param rc 解析后的 RC 数据结构
 * @return 1: 解析成功 0: 解析失败
 */
uint8_t ANO_DT_ParseRC(uint8_t *buf, uint8_t len, RC_Data_t *rc);

/**
 * @brief 读取遥控器数据并解析
 * @return uint8_t 0: 接收并解析成功 1: 失败
 */
uint8_t App_receive_data(void);

/**
 * @brief 处理连接状态
 * @param res 上一次接收数据的返回值
 */
void App_process_connect_state(uint8_t res);

/**
 * @brief 处理解锁逻辑
 * @return uint8_t 0: 解锁成功 1: 未解锁
 */
uint8_t App_process_unlock(void);

/**
 * @brief 处理飞机的飞行状态
 */
void App_process_flight_state(void);

/**
 * @brief 获取遥控器数据结构
 * @return RC_Data_t 指针
 */
RC_Data_t *App_get_rc_data(void);

/**
 * @brief 获取通道值字符串 (用于调试)
 * @return 格式化的通道数据字符串
 */
const char *App_get_rc_string(void);

#endif // __APP_RECEIVE_DATA__
