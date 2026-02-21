#include "Com_debug.h"

// 外部声明 huart2 (定义在 usart.c 中)
extern UART_HandleTypeDef huart2;

// 重定向 printf 到 UART (适用于 GCC 编译器)
// __io_putchar 是 ARM 半主机库中 printf 调用的底层函数
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 1000);
    return ch;
}

// 兼容其他编译器的 fputc 重定向
int fputc(int ch, FILE *f)
{
    return __io_putchar(ch);
}
