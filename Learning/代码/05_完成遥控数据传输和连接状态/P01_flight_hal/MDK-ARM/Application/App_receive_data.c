#include "App_receive_data.h"

extern Remote_Data remote_data;

uint8_t rx_buff[TX_PLOAD_WIDTH] = {0};

extern Remote_State remote_state;

// 重试次数
uint8_t retry_count = 0;
/**
 * @brief 接收遥控器发送的遥控数据 => 解析为结构体
 *
 * @return uint8_t 0:校验通过 是正常的数据 1:没收到数据 或者 校验失败
 */
uint8_t App_receive_data(void)
{
    memset(rx_buff, 0, TX_PLOAD_WIDTH);
    Int_SI24R1_RxPacket(rx_buff);
    if (strlen((char *)rx_buff) == 0)
    {
        return 1;
    }

    // 1. 帧头校验
    if (rx_buff[0] != FRAME_HEAD_CHECK_1 || rx_buff[1] != FRAME_HEAD_CHECK_2 || rx_buff[2] != FRAME_HEAD_CHECK_3)
    {
        return 1;
    }

    // 2. 帧尾校验
    uint32_t sum = 0;
    uint32_t sum_receive = 0;

    for (uint8_t i = 0; i < 13; i++)
    {
        sum += rx_buff[i];
    }
    // 高位在前
    sum_receive = rx_buff[13] << 24 | rx_buff[14] << 16 | rx_buff[15] << 8 | rx_buff[16];

    if (sum != sum_receive)
    {
        return 1;
    }

    // 3. 保存数据
    remote_data.thr = (rx_buff[3] << 8) | rx_buff[4];
    remote_data.yaw = (rx_buff[5] << 8) | rx_buff[6];
    remote_data.pit = (rx_buff[7] << 8) | rx_buff[8];
    remote_data.rol = (rx_buff[9] << 8) | rx_buff[10];
    remote_data.shutdown = rx_buff[11];
    remote_data.fix_height = rx_buff[12];

    debug_printf(":%d,%d,%d,%d,%d,%d\n", remote_data.thr, remote_data.yaw, remote_data.pit, remote_data.rol, remote_data.shutdown, remote_data.fix_height);
    return 0;
}

/**
 * @brief 处理连接状态的状态
 *
 * @param res 上一次接收数据的返回值
 */
void App_process_connect_state(uint8_t res)
{
    if (res == 0)
    {
        // 接收数据成功一次 即为连接成功
        // 此处使用的全局变量 只有当前一个地方会修改 LED灯控任务当中是读取使用
        remote_state = REMOTE_CONNECTED;
        retry_count = 0;
    }
    else if (res == 1)
    {
        // 接收数据失败 即为
        retry_count++;
        if (retry_count >= MAX_RETRY_TIMES)
        {
            remote_state = REMOTE_DISCONNECTED;
            retry_count = 0;
        }
    }
}
