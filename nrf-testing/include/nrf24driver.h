#pragma once

#include <Arduino.h>
#include <SPI.h>

// ==========================================
// nRF24L01+ REGISTER MAP & COMMAND DEFINITIONS
// ==========================================


// --- Module Pin Mapping ---
#define MOD1_CSN PB0
#define MOD2_CSN PA4
#define MOD2_CE  PA3
#define MOD1_CE  PB10

// SPI Command Masks & Directives
#define NRF_CMD_W_REG_MASK      0x20  // Write command mask (OR'd with register address to write)
#define NRF_CMD_R_REG_MASK      0x00  // Read command mask (OR'd with register address to read)
#define NRF_REG_MASK            0x1F  // Bitmask for extracting lower 5 bits of a register address
#define NRF_CMD_FLUSH_TX        0xE1  // Command to clear/flush the TX FIFO buffer
#define NRF_CMD_FLUSH_RX        0xE2  // Command to clear/flush the RX FIFO buffer
#define NRF_CMD_W_TX_PAYLOAD    0xA0
#define NRF_CMD_R_RX_PAYLOAD    0x61



// Register Addresses
#define NRF_REG_CONFIG          0x00  // Configuration register (Power up, PRX/PTX, CRC)
#define NRF_REG_EN_AA           0x01  // Enable Auto Acknowledgment across data pipes
#define NRF_REG_EN_RXADDR       0x02  // Enabled RX addresses / data pipes
#define NRF_REG_SETUP_AW        0x03  // Setup address width (3, 4, or 5 bytes)
#define NRF_REG_SETUP_RETR      0x04  // Setup automatic retransmission (delay and count)
#define NRF_REG_RF_CH           0x05  // RF channel frequency setting (0 to 125)
#define NRF_REG_RF_SETUP        0x06  // RF setup register (Data rate, RF output power)
#define NRF_REG_STATUS          0x07  // Status register (RX_DR, TX_DS, MAX_RT interrupt flags)
#define NRF_REG_RX_ADDR_P0      0x0A  // Receive address for data pipe 0 (5 bytes)
#define NRF_REG_RX_ADDR_P1      0x0B  // Receive address for data pipe 1 (5 bytes)
#define NRF_REG_TX_ADDR         0x10  // Transmit address (used for auto-ack / PTX mode)
#define NRF_REG_RX_PW_P0        0x11  // RX payload width in bytes for data pipe 0
#define NRF_REG_RX_PW_P1        0x12  // RX payload width in bytes for data pipe 1

// Configuration Bitmasks & Preset Values
#define NRF_CONFIG_PWR_UP       (1 << 1)  // Power Up bit (0 = Power Down, 1 = Power Up)
#define NRF_CONFIG_PRIM_RX      (1 << 0)  // RX/TX control bit (0 = PTX transmitter, 1 = PRX receiver)

#define NRF_EN_AA_NONE          0x00      // Disable Auto-Acknowledgment on all pipes
#define NRF_EN_RXADDR_P0        (1 << 0)  // Enable data pipe 0
#define NRF_EN_RXADDR_P1        (1 << 1)  // Enable data pipe 1

#define NRF_STATUS_TX_DS        (1 << 5)
#define NRF_STATUS_RX_DR        (1 << 6)

#define NRF_RF_CH_DEFAULT       10        // Default operating channel (2410 MHz)
#define NRF_RF_SETUP_1MBPS      0x06      // RF Setup value: 1 Mbps air data rate, 0 dBm output power
#define NRF_STATUS_CLEAR_ALL    0x70      // Bitmask to clear all 3 interrupt flags (RX_DR, TX_DS, MAX_RT)

#define NRF_PAYLOAD_LEN_4       4         // Static payload length of 4 bytes
#define NRF_ADDR_BYTE_DEFAULT   0xE7      // Default base byte pattern used for pipe addresses


// ==========================================
// LOW-LEVEL HARDWARE ABSTRACTION FUNCTIONS
// ==========================================

//This function writes a single byte to a specified register through the selected CSN-pin of the used nRF24 module
 void writeReg(uint8_t csn, uint8_t reg, uint8_t val);

//This function reads a single byte from a specified register through the selected CSN-pin of the used nRF24 module
 uint8_t readReg(uint8_t csn, uint8_t reg);

//This function writes a 5-byte address to a specified register through the selected CSN-pin of the used nRF24 module
 void setAddress(uint8_t csn, uint8_t reg, uint8_t addrByte);

//This function encapsulates the entire hardware reset, configuration, and state-machine bringup sequence for both radios
 void initModulesAtSpeed(uint32_t spiHz);
