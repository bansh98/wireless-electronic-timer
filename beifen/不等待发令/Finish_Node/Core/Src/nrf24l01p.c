#include "nrf24l01p.h"
#include "spi.h"
#include "main.h" // ??????,???? NRF_CSN_Pin

// ???? SPI1 (???? spi.h ?)
extern SPI_HandleTypeDef hspi1;
#define NRF_SPI &hspi1  

// --- ?????? ---
static void cs_high(void) {
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);
}

static void cs_low(void) {
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET);
}

static void ce_high(void) {
    HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_SET);
}

static void ce_low(void) {
    HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_RESET);
}

// --- ????? ---
static uint8_t write_reg(uint8_t reg, uint8_t value) {
    uint8_t cmd = RF24L01P_CMD_W_REGISTER | reg;
    uint8_t status;
    cs_low();
    HAL_SPI_TransmitReceive(NRF_SPI, &cmd, &status, 1, 100);
    HAL_SPI_Transmit(NRF_SPI, &value, 1, 100);
    cs_high();
    return status;
}

static uint8_t read_reg(uint8_t reg) {
    uint8_t cmd = RF24L01P_CMD_R_REGISTER | reg;
    uint8_t val;
    cs_low();
    HAL_SPI_Transmit(NRF_SPI, &cmd, 1, 100);
    HAL_SPI_Receive(NRF_SPI, &val, 1, 100);
    cs_high();
    return val;
}

// --- ???? ---
void nrf24l01p_reset(void) {
    ce_low();
    write_reg(RF24L01P_REG_CONFIG, 0x08);
    write_reg(RF24L01P_REG_EN_AA, 0x01); // ??0????
    write_reg(RF24L01P_REG_EN_RXADDR, 0x01);
    write_reg(RF24L01P_REG_SETUP_AW, 0x03);
    write_reg(RF24L01P_REG_SETUP_RETR, 0x1A); // ????
    write_reg(RF24L01P_REG_RF_CH, 40);
    write_reg(RF24L01P_REG_RF_SETUP, 0x0F);
    
    // ?????
    uint8_t cmd_flush = RF24L01P_CMD_FLUSH_TX;
    cs_low(); HAL_SPI_Transmit(NRF_SPI, &cmd_flush, 1, 100); cs_high();
    cmd_flush = RF24L01P_CMD_FLUSH_RX;
    cs_low(); HAL_SPI_Transmit(NRF_SPI, &cmd_flush, 1, 100); cs_high();
}

void nrf24l01p_tx_init(uint8_t channel, nrf24l01p_data_rate_t data_rate) {
    nrf24l01p_reset();
    
    write_reg(RF24L01P_REG_RF_CH, channel); // ????
    
    // ????
    uint8_t setup = 0x06; // ????
    if(data_rate == _250kbps) setup |= 0x20;
    else if(data_rate == _2Mbps) setup |= 0x08;
    write_reg(RF24L01P_REG_RF_SETUP, setup);
    
    // ??????
    uint8_t config = read_reg(RF24L01P_REG_CONFIG);
    config &= ~0x01; // PRIM_RX = 0 (TX??)
    config |= 0x02;  // PWR_UP = 1
    write_reg(RF24L01P_REG_CONFIG, config);
    
    ce_low();
}

void nrf24l01p_rx_init(uint8_t channel, nrf24l01p_data_rate_t data_rate) {
    nrf24l01p_reset();
    
    write_reg(RF24L01P_REG_RF_CH, channel);
    
    uint8_t setup = 0x06; 
    if(data_rate == _250kbps) setup |= 0x20;
    else if(data_rate == _2Mbps) setup |= 0x08;
    write_reg(RF24L01P_REG_RF_SETUP, setup);
    
    write_reg(RF24L01P_REG_RX_PW_P0, 32); // ??????32??
    
    // ??????
    uint8_t config = read_reg(RF24L01P_REG_CONFIG);
    config |= 0x01;  // PRIM_RX = 1 (RX??)
    config |= 0x02;  // PWR_UP = 1
    write_reg(RF24L01P_REG_CONFIG, config);
    
    ce_high(); // ????
}

void nrf24l01p_tx_transmit(uint8_t* data) {
    uint8_t cmd = RF24L01P_CMD_W_TX_PAYLOAD;
    cs_low();
    HAL_SPI_Transmit(NRF_SPI, &cmd, 1, 100);
    HAL_SPI_Transmit(NRF_SPI, data, 32, 100);
    cs_high();
    
    ce_high(); HAL_Delay(1); ce_low(); // ??????
}

uint8_t nrf24l01p_rx_receive(uint8_t* data) {
    uint8_t status = read_reg(RF24L01P_REG_STATUS);
    
    if (status & 0x40) { // RX_DR ?? (????)
        uint8_t cmd = RF24L01P_CMD_R_RX_PAYLOAD;
        cs_low();
        HAL_SPI_Transmit(NRF_SPI, &cmd, 1, 100);
        HAL_SPI_Receive(NRF_SPI, data, 32, 100);
        cs_high();
        
        write_reg(RF24L01P_REG_STATUS, 0x40); // ?????
        return 1; // ????
    }
    return 0;
}