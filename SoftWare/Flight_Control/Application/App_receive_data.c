#include "App_receive_data.h"
#include "Int_buzzer.h"
#include "Int_spl06.h"
#include "string.h"
#include "task.h"

// 遥控器连接状态
extern Remote_State remote_state;
// 飞行状态
extern Flight_State flight_state;

// 遥控器数据（全局可访问）
static RC_Data_t rc_data = {0};
static uint8_t com_data[32] = {0};

// 回复数据缓冲区 (飞控状态)
static uint8_t back_buff[32] = {0};

// 油门状态
static Thr_state thr_state = FREE;
// MAX 状态的进入时间
static uint32_t max_enter_time = 0;
// MIN 状态的进入时间
static uint32_t min_enter_time = 0;
// 重试次数
static uint8_t retry_count = 0;

// 定高高度 (在 App_freeRTOS_Task.c 中定义)
extern uint16_t fix_height;

/**
 * @brief 解析 ANO_DT 遥控器数据协议 (25 字节帧)
 * 
 * 帧格式:
 * [0xAA][0xAF][0x03][LEN][DATA...][SUM]
 *   1B     1B     1B    1B   20B      1B
 * 
 * @param buf 接收缓冲区
 * @param len 缓冲区长度
 * @param rc 解析后的 RC 数据结构
 * @return 1: 解析成功 0: 解析失败
 */
uint8_t ANO_DT_ParseRC(uint8_t *buf, uint8_t len, RC_Data_t *rc)
{
    // 1. 检查帧头
    if (buf[0] != 0xAA || buf[1] != 0xAF)
        return 0;
    
    // 2. 检查功能字 (0x03 = RC 遥控数据)
    if (buf[2] != 0x03)
        return 0;
    
    // 3. 检查数据长度
    uint8_t data_len = buf[3];
    if (data_len < 12)  // 至少需要 4 个通道 (8 字节)
        return 0;
    
    // 4. 计算总帧长度 = 4 字节头 + 数据区 + 1 字节校验和
    uint8_t frame_len = 4 + data_len + 1;
    if (frame_len > len)
        return 0;
    
    // 5. 校验和验证 (从帧头到数据区末尾)
    uint8_t sum = 0;
    for (uint8_t i = 0; i < frame_len - 1; i++)
        sum += buf[i];
    if (sum != buf[frame_len - 1])
        return 0;
    
    // 6. 解析通道数据 (大端模式：高字节在前)
    rc->thr  = (uint16_t)((buf[4] << 8) | buf[5]);
    rc->yaw  = (uint16_t)((buf[6] << 8) | buf[7]);
    rc->rol  = (uint16_t)((buf[8] << 8) | buf[9]);
    rc->pit  = (uint16_t)((buf[10] << 8) | buf[11]);
    rc->aux1 = (uint16_t)((buf[12] << 8) | buf[13]);
    rc->aux2 = (uint16_t)((buf[14] << 8) | buf[15]);
    rc->aux3 = (uint16_t)((buf[16] << 8) | buf[17]);
    rc->aux4 = (uint16_t)((buf[18] << 8) | buf[19]);
    
    return 1;
}

/**
 * @brief 构建飞控状态回复帧 (ANO_DT 协议 0x05 功能字)
 * 
 * 回复帧格式:
 * [0xAA][0xAF][0x05][LEN][VoltageH][VoltageL][HW][0x00][0x00][SUM]
 *   1B     1B     1B    1B     1B        1B      1B   1B    1B    1B
 * 
 * @param voltage 电池电压 (mV * 100)
 * @param hardware 硬件类型 (1=四轴)
 */
static void ANO_DT_BuildStatusFrame(uint16_t voltage, uint8_t hardware)
{
    memset(back_buff, 0, 32);
    
    // 帧头
    back_buff[0] = 0xAA;
    back_buff[1] = 0xAF;
    // 功能字 0x05 = 飞控状态
    back_buff[2] = 0x05;
    // 数据长度
    back_buff[3] = 0x08;
    
    // 电压数据 (大端模式)
    back_buff[4] = (voltage >> 8) & 0xFF;
    back_buff[5] = voltage & 0xFF;
    
    // 硬件类型
    back_buff[6] = hardware;
    
    // 软件版本 (低字节)
    back_buff[7] = 0x01;
    
    // 计算校验和
    uint8_t sum = 0;
    for (uint8_t i = 0; i < 11; i++)
        sum += back_buff[i];
    back_buff[11] = sum;
}

/**
 * @brief 读取遥控器数据并解析
 * @return uint8_t 0: 接收并解析成功 1: 失败
 */
uint8_t App_receive_data(void)
{
    memset(com_data, 0, 32);
    uint8_t res = Int_nRF24L01_RxPacket(com_data);
    
    if (res == 0)
    {
        // 接收到数据，解析
        if (ANO_DT_ParseRC(com_data, 32, &rc_data))
        {
            // 解析成功，构建回复帧
            // 电压示例：假设 11.1V = 1110 (单位：0.1V)
            ANO_DT_BuildStatusFrame(1110, 1);  // 1=四轴飞行器
            
            // 切换到发送模式并回复
            Int_nRF24L01_TX_Mode();
            uint16_t count = 100;
            while (Int_nRF24L01_TxPacket(back_buff) == 1 && count--)
            {
                // 等待发送完成
            }
            // 切回接收模式
            Int_nRF24L01_RX_Mode();
            
            return 0; // 成功
        }
    }
    return 1; // 失败
}

/**
 * @brief 处理连接状态
 * @param res 上一次接收数据的返回值
 */
void App_process_connect_state(uint8_t res)
{
    if (res == 0)
    {
        // 接收数据成功，认为连接成功
        remote_state = REMOTE_CONNECTED;
        retry_count = 0;
    }
    else
    {
        // 接收数据失败
        retry_count++;
        if (retry_count >= MAX_RETRY_TIMES)
        {
            remote_state = REMOTE_DISCONNECTED;
            retry_count = 0;
        }
    }
}

/**
 * @brief 处理解锁逻辑
 * @return uint8_t 0: 解锁成功 1: 未解锁
 * 
 * 解锁流程 (日本手 Mode 1):
 * 1. 油门推到最大 (>=1900) 保持 1 秒
 * 2. 油门拉到最小 (<=1100) 保持 1 秒
 * 3. 解锁完成
 */
uint8_t App_process_unlock(void)
{
    switch (thr_state)
    {
    case FREE:
        // 步骤 1: 检测油门推到最大
        if (rc_data.thr >= 1900)
        {
            thr_state = MAX;
            max_enter_time = xTaskGetTickCount();
        }
        break;
        
    case MAX:
        // 检测油门离开最大位
        if (rc_data.thr < 1900)
        {
            // 判断在 MAX 状态是否持续 >= 1 秒
            if (xTaskGetTickCount() - max_enter_time >= pdMS_TO_TICKS(1000))
            {
                thr_state = LEAVE_MAX;
            }
            else
            {
                thr_state = FREE;
            }
        }
        break;
        
    case LEAVE_MAX:
        // 步骤 2: 检测油门拉到最小
        if (rc_data.thr <= 1100)
        {
            thr_state = MIN;
            min_enter_time = xTaskGetTickCount();
        }
        break;
        
    case MIN:
        // 检测油门离开最小位
        if (xTaskGetTickCount() - min_enter_time <= pdMS_TO_TICKS(1000))
        {
            // 1 秒内离开，失败
            if (rc_data.thr > 1100)
            {
                thr_state = FREE;
            }
        }
        else
        {
            // 在最小位保持 >= 1 秒，解锁成功
            thr_state = UNLOCK;
        }
        break;
        
    case UNLOCK:
        // 已解锁状态
        break;
        
    default:
        break;
    }

    return (thr_state == UNLOCK) ? 0 : 1;
}

/**
 * @brief 处理飞机的飞行状态
 *
 * 状态机转换:
 * IDLE -> NORMAL: 解锁成功
 * NORMAL -> FIX_HEIGHT: AUX1 < 1450 或 AUX1 > 1550
 * FIX_HEIGHT -> NORMAL: 1450 <= AUX1 <= 1550
 * ANY -> FAIL: 遥控器失联
 */
void App_process_flight_state(void)
{
    static uint8_t last_flight_state = IDLE;  // 记录上一状态
    switch (flight_state)
    {
    case IDLE:
        if (App_process_unlock() == 0)
        {
            flight_state = NORMAL;
            thr_state = FREE;
            debug_printf("Flight: UNLOCKED\r\n");
            // 解锁成功：长蜂鸣提示
            Int_buzzer_long_beep();
        }
        break;

    case NORMAL:
        // 检测状态变化：从其他状态进入 NORMAL
        if (last_flight_state != NORMAL) {
            // 从定高退出到正常模式：单短蜂鸣
            if (last_flight_state == FIX_HEIGHT) {
                Int_buzzer_short_beep();
            }
        }

        // 判断进入定高 (使用 AUX1 通道，< 1450 或 > 1550 进入定高)
        if (rc_data.aux1 < 1450 || rc_data.aux1 > 1550)
        {
            flight_state = FIX_HEIGHT;
            // 记录当前高度 (使用气压计)
            fix_height = (uint16_t)Int_SPL06_Get_Altitude();
            debug_printf("Flight: FIX_HEIGHT, alt=%d m\r\n", fix_height);
            // 进入定高模式：双蜂鸣提示
            Int_buzzer_double_beep();
        }
        // 判断进入失控状态
        if (remote_state == REMOTE_DISCONNECTED)
        {
            flight_state = FAIL;
            debug_printf("Flight: FAIL (RC lost)\r\n");
        }
        break;

    case FIX_HEIGHT:
        // 取消定高 (使用 AUX1 通道，1450 <= AUX1 <= 1550 退出定高)
        if (rc_data.aux1 >= 1450 && rc_data.aux1 <= 1550)
        {
            flight_state = NORMAL;
            debug_printf("Flight: NORMAL\r\n");
            // 退出定高模式：单短蜂鸣
            Int_buzzer_short_beep();
        }
        // 判断故障
        if (remote_state == REMOTE_DISCONNECTED)
        {
            flight_state = FAIL;
        }
        break;

    case FAIL:
        // 失控处理：等待任务通知
        // 注意：实际应用中应该执行失控保护逻辑 (如自动降落)
        flight_state = IDLE;
        debug_printf("Flight: IDLE (reset)\r\n");
        break;

    default:
        break;
    }
    
    // 更新上一状态
    last_flight_state = flight_state;
}

/**
 * @brief 获取遥控器数据结构
 * @return RC_Data_t 指针
 */
RC_Data_t *App_get_rc_data(void)
{
    return &rc_data;
}

/**
 * @brief 获取通道值字符串 (用于调试)
 * @return 格式化的通道数据字符串
 */
const char *App_get_rc_string(void)
{
    static char rc_str[64];
    snprintf(rc_str, sizeof(rc_str), 
             "THR:%4d YAW:%4d ROL:%4d PIT:%4d AUX1:%4d AUX2:%4d",
             rc_data.thr, rc_data.yaw, rc_data.rol, 
             rc_data.pit, rc_data.aux1, rc_data.aux2);
    return rc_str;
}
