#ifndef __nRF24L01P__
#define __nRF24L01P__

#include "config.h"

//nRF24L01+ PIN DEFINITION
#define	MOSI      P13             // Master Out, Slave In pin (output)
#define	MISO      P10             // Master In, Slave Out pin (input)
#define	SCK       P12             // Serial Clock pin, (output)
#define	CSN       P15             // Slave Select pin, (output to CSN)
#define	CE        P14             // Chip Enable pin signal (output)
#define	IRQ       P11             // Interrupt signal, from nRF24L01 (input)

#define TX_ADR_WIDTH   5  				// 5字节宽度的发送/接收地址
#define TX_PLOAD_WIDTH 32  				// 数据通道有效数据宽度

//********************************************************************************************************************//
// SPI(nRF24L01+) commands
#define READ_REG        0x00  // Define read command to register
#define WRITE_REG       0x20  // Define write command to register
#define RD_RX_PLOAD     0x61  // Define RX payload register address
#define WR_TX_PLOAD     0xA0  // Define TX payload register address
#define FLUSH_TX        0xE1  // Define flush TX register command
#define FLUSH_RX        0xE2  // Define flush RX register command
#define REUSE_TX_PL     0xE3  // Define reuse TX payload register command
#define NOP             0xFF  // Define No Operation, might be used to read status register

//********************************************************************************************************************//
// SPI(nRF24L01+) registers(addresses)
#define CONFIG          0x00  // 'Config' register address
#define EN_AA           0x01  // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR       0x02  // 'Enabled RX addresses' register address
#define SETUP_AW        0x03  // 'Setup address width' register address
#define SETUP_RETR      0x04  // 'Setup Auto. Retrans' register address
#define RF_CH           0x05  // 'RF channel' register address
#define RF_SETUP        0x06  // 'RF setup' register address
#define STATUS          0x07  // 'Status' register address
#define OBSERVE_TX      0x08  // 'Observe TX' register address
#define RPD             0x09  // 'Received Power Detector' register address
#define RX_ADDR_P0      0x0A  // 'RX address pipe0' register address
#define RX_ADDR_P1      0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2      0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3      0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4      0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5      0x0F  // 'RX address pipe5' register address
#define TX_ADDR         0x10  // 'TX address' register address
#define RX_PW_P0        0x11  // 'RX payload width, pipe0' register address
#define RX_PW_P1        0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2        0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3        0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4        0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5        0x16  // 'RX payload width, pipe5' register address
#define FIFO_STATUS     0x17  // 'FIFO Status Register' register address

//********************************************************************************************************************//
// STATUS Register 
#define RX_DR						0x40  /**/
#define TX_DS						0x20
#define MAX_RT					0x10

//********************************************************************************************************************//
//                                        FUNCTION's PROTOTYPES                                                       //
//********************************************************************************************************************//
//nRF24L01+ API Functions
void nRF24L01P_Init(void); //24L01+ Pin Init
u8 nRF24L01P_Write_Reg(u8 reg, u8 value); 
u8 nRF24L01P_Write_Buf(u8 reg, const u8 *pBuf, u8 bytes);
u8 nRF24L01P_Read_Reg(u8 reg);
u8 nRF24L01P_Read_Buf(u8 reg, u8 *pBuf, u8 bytes);

void nRF24L01P_RX_Mode(void);
void nRF24L01P_TX_Mode(void);
u8 nRF24L01P_RxPacket(u8 *rxbuf);
u8 nRF24L01P_TxPacket(u8 *txbuf);

//********************************************************************************************************************//
#endif

