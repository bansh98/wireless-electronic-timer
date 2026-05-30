#ifndef __NRF24L01P_H__
#define __NRF24L01P_H__

// ????????,???? uint8_t ??? STM32 ???
#include "stm32f1xx_hal.h" 

/* ??????? */
#define RF24L01P_REG_CONFIG         0x00
#define RF24L01P_REG_EN_AA          0x01
#define RF24L01P_REG_EN_RXADDR      0x02
#define RF24L01P_REG_SETUP_AW       0x03
#define RF24L01P_REG_SETUP_RETR     0x04
#define RF24L01P_REG_RF_CH          0x05
#define RF24L01P_REG_RF_SETUP       0x06
#define RF24L01P_REG_STATUS         0x07
#define RF24L01P_REG_RX_ADDR_P0     0x0A
#define RF24L01P_REG_TX_ADDR        0x10
#define RF24L01P_REG_RX_PW_P0       0x11
#define RF24L01P_REG_FIFO_STATUS    0x17

/* ?? */
#define RF24L01P_CMD_R_REGISTER     0x00
#define RF24L01P_CMD_W_REGISTER     0x20
#define RF24L01P_CMD_R_RX_PAYLOAD   0x61
#define RF24L01P_CMD_W_TX_PAYLOAD   0xA0
#define RF24L01P_CMD_FLUSH_TX       0xE1
#define RF24L01P_CMD_FLUSH_RX       0xE2
#define RF24L01P_CMD_NOP            0xFF

/* ???? */
typedef enum {
    _250kbps,
    _1Mbps,
    _2Mbps
} nrf24l01p_data_rate_t;

/* ???? */
void nrf24l01p_tx_init(uint8_t channel, nrf24l01p_data_rate_t data_rate);
void nrf24l01p_rx_init(uint8_t channel, nrf24l01p_data_rate_t data_rate);
void nrf24l01p_tx_transmit(uint8_t* data);
uint8_t nrf24l01p_rx_receive(uint8_t* data);

// ????
void nrf24l01p_reset(void);
uint8_t nrf24l01p_get_status(void);
uint8_t nrf24l01p_check(void); // ??NRF????

#endif