#ifndef _COM_CONFIG_H
#define _COM_CONFIG_H

#include "main.h"

// 连接状态
typedef enum
{
    REMOTE_CONNECTED = 0,
    REMOTE_DISCONNECTED,
}Remote_State;

// 飞行状态
typedef enum
{
    IDLE = 0,
    NORMAL,
    FIX_HEIGHT,
    FAIL,
}Flight_State;

// 油门解锁状态
typedef enum
{
    FREE = 0,
    MAX,
    LEAVE_MAX,
    MIN,
    UNLOCK,
} Thr_state;

typedef struct
{
    int16_t thr;
    int16_t yaw;
    int16_t pit;
    int16_t rol;
    uint8_t shutdown;   // 1: 关闭  0: 不关机
    uint8_t fix_height; // 1. 切换定高和不定高 0: 不切换
} Remote_Data;




#endif // !_COM_CONFIG_H
