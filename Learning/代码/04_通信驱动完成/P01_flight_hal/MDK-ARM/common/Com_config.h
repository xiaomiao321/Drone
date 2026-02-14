#ifndef _COM_CONFIG_H
#define _COM_CONFIG_H

#include "main.h"

// Á¬½Ó×´Ì¬
typedef enum
{
    REMOTE_CONNECTED = 0,
    REMOTE_DISCONNECTED,
}Remote_State;

// ·ÉÐÐ×´Ì¬
typedef enum
{
    IDLE = 0,
    NORMAL,
    FIX_HEIGHT,
    FAIL,
}Flight_State;

#endif // !_COM_CONFIG_H
