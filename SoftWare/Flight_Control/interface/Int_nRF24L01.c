#include "Int_nRF24L01.h"

// 定义一个静态的发送地址  => 必须与遥控器 TX_ADDRESS 一致
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0xAA, 0xBB, 0xCC, 0x00,
                                    0x01}; // 与遥控器对频地址

// SPI 读写一个字节 => 写入的字节是传入的参数  读取的字节是返回值
static uint8_t SPI_RW(uint8_t byte) {
  uint8_t rx_data = 0;
  HAL_SPI_TransmitReceive(&hspi1, &byte, &rx_data, 1, 1000);
  return rx_data;
}

/********************************************************
函数功能：写寄存器的值（单字节）
入口参数：reg:寄存器映射地址（格式：NRF_WRITE_REG|reg）
                                        value:寄存器的值
返回  值：状态寄存器的值
*********************************************************/
uint8_t Int_nRF24L01_Write_Reg(uint8_t reg, uint8_t value) {
  uint8_t status;

  CS_LOW;
  status = SPI_RW(reg);
  SPI_RW(value);
  CS_HIGH;

  return (status);
}

/********************************************************
函数功能：写寄存器的值（多字节）
入口参数：reg:寄存器映射地址（格式：NRF_WRITE_REG|reg）
                                        pBuf:写数据首地址
                                        bytes:写数据字节数
返回  值：状态寄存器的值
*********************************************************/
uint8_t Int_nRF24L01_Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t size) {
  uint8_t status, byte_ctr;

  CS_LOW;
  status = SPI_RW(reg);
  for (byte_ctr = 0; byte_ctr < size; byte_ctr++) {
    SPI_RW(*pBuf++);
  }

  CS_HIGH;

  return (status);
}

/********************************************************
函数功能：读取寄存器的值（单字节）
入口参数：reg:寄存器映射地址（格式：NRF_READ_REG|reg）
返回  值：寄存器值
*********************************************************/
uint8_t Int_nRF24L01_Read_Reg(uint8_t reg) {
  uint8_t value;

  CS_LOW;
  SPI_RW(reg);
  value = SPI_RW(0);
  CS_HIGH;

  return (value);
}

/********************************************************
函数功能：读取寄存器的值（多字节）
入口参数：reg:寄存器映射地址（NRF_READ_REG|reg）
                                        pBuf:接收缓冲区的首地址
                                        bytes:读取字节数
返回  值：状态寄存器的值
*********************************************************/
uint8_t Int_nRF24L01_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t size) {
  uint8_t status, byte_ctr;

  CS_LOW;
  status = SPI_RW(reg);
  for (byte_ctr = 0; byte_ctr < size; byte_ctr++) {
    pBuf[byte_ctr] = SPI_RW(0); // 读取数据，低字节在前
  }
  CS_HIGH;

  return (status);
}

/********************************************************
函数功能：nRF24L01 接收模式初始化
入口参数：无
返回  值：无
*********************************************************/
void Int_nRF24L01_RX_Mode(void) {
  CE_LOW;
  // 配置 RX 地址和 TX 地址（用于发送 ACK）
  Int_nRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);
  Int_nRF24L01_Write_Buf(NRF_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
  // 使能通道 0
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + EN_AA, 0x01);
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + EN_RXADDR, 0x01);
  // 射频通道
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + RF_CH, CHANNEL);
  // 接收数据宽度
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);
  // RF 设置：2Mbps, 0dBm
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + RF_SETUP, 0x0f);
  // CONFIG: CRC 使能，16 位 CRC，上电，接收模式
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + CONFIG, 0x0f);
  // 清除中断标志
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + STATUS, 0xff);
  // 清除 FIFO
  Int_nRF24L01_Write_Reg(FLUSH_TX, 0xff);
  Int_nRF24L01_Write_Reg(FLUSH_RX, 0xff);
  // nRF24L01 特殊配置（匹配遥控器 TX2 模式）- 必须与遥控器一致
  SPI_RW(0x50);  // 选择寄存器组
  SPI_RW(0x73);
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + 0x1c, 0x01);  // 兼容模式配置
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + 0x1d, 0x06);  // 兼容模式配置
  CE_HIGH;
}

/********************************************************
函数功能：nRF24L01 发送模式初始化
入口参数：无
返回  值：无
*********************************************************/
void Int_nRF24L01_TX_Mode(void) {
  CE_LOW;
  Int_nRF24L01_Write_Buf(NRF_WRITE_REG + TX_ADDR, TX_ADDRESS,
                       TX_ADR_WIDTH); // 写入发送地址
  Int_nRF24L01_Write_Buf(
      NRF_WRITE_REG + RX_ADDR_P0, TX_ADDRESS,
      TX_ADR_WIDTH); // 为了应答接收设备，接收通道 0 地址和发送地址相同
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + EN_AA, 0x01); // 使能接收通道 0 自动应答
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + EN_RXADDR, 0x01); // 使能接收通道 0
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + SETUP_RETR,
                       0x1a); // 自动重发延时等待 250us+86us，自动重发 10 次
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + RF_CH, CHANNEL); // 选择射频通道 0x40
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + RF_SETUP,
                       0x0f); // 数据传输率 2Mbps，发射功率 0dBm
  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + CONFIG,
                       0x0e); // CRC 使能，16 位 CRC 校验，上电
  CE_HIGH;
}

/********************************************************
函数功能：读取接收数据
入口参数：rxbuf:接收数据存放首地址
返回  值：0:接收到数据 1:没有接收到数据
*********************************************************/
uint8_t Int_nRF24L01_RxPacket(uint8_t *rxbuf) {
  uint8_t state;
  uint8_t rx_len;

  // 读取状态寄存器
  state = Int_nRF24L01_Read_Reg(STATUS);

  if (state & RX_DR) { // 接收到数据
    // 先读取有效数据长度
    rx_len = Int_nRF24L01_Read_Reg(R_RX_PL_WID);

    // 验证数据长度
    if (rx_len <= TX_PLOAD_WIDTH && rx_len > 0) {
      // 读取实际长度的数据
      Int_nRF24L01_Read_Buf(RD_RX_PLOAD, rxbuf, rx_len);
      // 清除 RX FIFO
      Int_nRF24L01_Write_Reg(FLUSH_RX, 0xff);
      // 清除 RX_DR 中断标志
      Int_nRF24L01_Write_Reg(NRF_WRITE_REG + STATUS, RX_DR);
      return 0;
    } else {
      // 数据长度无效，清除 FIFO
      Int_nRF24L01_Write_Reg(FLUSH_RX, 0xff);
      Int_nRF24L01_Write_Reg(NRF_WRITE_REG + STATUS, RX_DR);
      return 1;
    }
  }
  return 1; // 没收到任何数据
}

/********************************************************
函数功能：发送一个数据包
入口参数：txbuf:要发送的数据
返回  值：0: 发送成功 1: 发送失败
*********************************************************/
uint8_t Int_nRF24L01_TxPacket(uint8_t *txbuf) {
  uint8_t state;
  CE_LOW; // CE 拉低，使能 nRF24L01 配置
  Int_nRF24L01_Write_Buf(WR_TX_PLOAD, txbuf,
                       TX_PLOAD_WIDTH); // 写数据到 TX FIFO,32 个字节
  CE_HIGH;                              // CE 置高，使能发送

  state = Int_nRF24L01_Read_Reg(STATUS); // 读取状态寄存器的值
  while (((state & TX_DS) == 0) && ((state & MAX_RT) == 0)) {
    state = Int_nRF24L01_Read_Reg(STATUS);
  }

  Int_nRF24L01_Write_Reg(NRF_WRITE_REG + STATUS,
                       state); // 清除 TX_DS 或 MAX_RT 中断标志
  if (state & MAX_RT) { // 达到最大重发次数
    Int_nRF24L01_Write_Reg(FLUSH_TX, 0xff); // 清除 TX FIFO 寄存器
    return 1;
  }
  if (state & TX_DS) { // 发送完成
    return 0;
  }
  return 1; // 发送失败
}

uint8_t nrf24l01_rx_buff[5] = {0};

/**
 * @brief nRF24L01 的初始化检测
 *
 * @return uint8_t  0:检测成功  1:检测失败
 */
uint8_t Int_nRF24L01_Check(void) {
  // 1. 测试 SPI 通信能够正常读写寄存器
  // 1.1 写入发送地址
  Int_nRF24L01_Write_Buf(NRF_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);

  // 1.2 读取同样的数据
  Int_nRF24L01_Read_Buf(NRF_READ_REG + TX_ADDR, nrf24l01_rx_buff, TX_ADR_WIDTH);

  for (uint8_t i = 0; i < TX_ADR_WIDTH; i++) {
    if (nrf24l01_rx_buff[i] != TX_ADDRESS[i]) {
      return 1;
    }
  }
  return 0;
}

/**
 * @brief 硬件接口层 nRF24L01 的初始化
 *
 */
void Int_nRF24L01_Init(void) {
  // 上电之后的芯片延迟 >100ms
  HAL_Delay(200);
  // 校验检测
  while (Int_nRF24L01_Check() == 1) {
    // 每两次检测间隔 10ms
    HAL_Delay(10);
  }

  // 设置默认的状态为接收模式
  Int_nRF24L01_RX_Mode();
  debug_printf("nRF24L01 Init Success!\r\n");
}
