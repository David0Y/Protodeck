#pragma once

#include <Arduino.h>
#include <SPI.h>

// ==========================================
// nRF24L01+ REGISTER MAP & COMMAND DEFINITIONS
// ==========================================

#define MOD1_CSN PB0
#define MOD2_CSN PA4
#define MOD2_CE  PA3

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
static inline void writeReg(uint8_t csn, uint8_t reg, uint8_t val) {
  digitalWrite(csn, LOW); //Pulls the CSN pin low to select the specific nRF24 module on the shared SPI bus
  SPI.transfer(NRF_CMD_W_REG_MASK | (reg & NRF_REG_MASK)); //Sends the write instruction (command) byte, combines the write instruction bit 0x20 with the 5-bit register address
  SPI.transfer(val); //clocks out the payload data byte to be written into that target register
  digitalWrite(csn, HIGH); //Pulls CSN high to end the SPI transaction, latches the configuration into the chip's internal register
}

//This function reads a single byte from a specified register through the selected CSN-pin of the used nRF24 module
static inline uint8_t readReg(uint8_t csn, uint8_t reg) {
  digitalWrite(csn, LOW); //Selects the targeted nRF24 module through the CSN pin
  SPI.transfer(NRF_CMD_R_REG_MASK | (reg & NRF_REG_MASK)); //Sends the read instruction byte, combines the read instruction bit 0x00 with the 5-bit register address
  uint8_t val = SPI.transfer(0xFF); // Shifts out a dummy byte (0xFF) to generate the SPI clock pulses
  digitalWrite(csn, HIGH); //Pulls CSN high to deselect the chip and end the SPI transaction
  return val;
}

//This function writes a 5-byte address to a specified register through the selected CSN-pin of the used nRF24 module
static inline void setAddress(uint8_t csn, uint8_t reg, uint8_t addrByte) {
  digitalWrite(csn, LOW); //Selects the targeted nRF24 module through the CSN pin
  SPI.transfer(NRF_CMD_W_REG_MASK | (reg & NRF_REG_MASK)); //Sends the write instruction (command) byte, combines the write instruction bit 0x20 with the 5-bit register address
  for (int i = 0; i < 5; i++) {
    SPI.transfer(addrByte); //Loops 5 times consecutively, shifting out all 5 bytes of the address in one unbroken SPI stream
  }
  digitalWrite(csn, HIGH); //Pulls CSN high to deselect the chip and end the SPI transaction
}

//This function encapsulates the entire hardware reset, configuration, and state-machine bringup sequence for both radios
static inline void initModulesAtSpeed(uint32_t spiHz) {
  SPI.endTransaction(); // Ends any active SPI transaction session
  //Configures the spi bus hardware parameters by setting the master clock frequency, specifying the bit order (MSB first), and selecting the SPI mode (mode 0)
  SPI.beginTransaction(SPISettings(spiHz, MSBFIRST, SPI_MODE0));

  // Reset/Power down registers initially
  writeReg(MOD1_CSN, NRF_REG_CONFIG, 0x00);
  writeReg(MOD2_CSN, NRF_REG_CONFIG, 0x00);
  delay(10);

  // Configure Radio 1 (TX Setup)
  writeReg(MOD1_CSN, NRF_REG_EN_AA, NRF_EN_AA_NONE); //Writes 0x00 to Radio 1's EN_AA (Auto-Acknowledgment) register
  writeReg(MOD1_CSN, NRF_REG_EN_RXADDR, NRF_EN_RXADDR_P0); //Writes 0x01 to Radio 1's EN_RXADDR register, enabling Data Pipe 0
  writeReg(MOD1_CSN, NRF_REG_RF_CH, NRF_RF_CH_DEFAULT); //Writes channel value 10 (2410 MHz) to Radio 1's RF_CH register, tuning its physical radio frequency synthesizer to channel 10
  writeReg(MOD1_CSN, NRF_REG_RF_SETUP, NRF_RF_SETUP_1MBPS); //Writes 0x06 to Radio 1's RF_SETUP register, configuring its internal modulator for a 1 Mbps air data rate and maximum 0 dBm output power


  // Configure Radio 2 (RX Setup)
  writeReg(MOD2_CSN, NRF_REG_EN_AA, NRF_EN_AA_NONE); //Writes 0x00 to Radio 2's EN_AA (Auto-Acknowledgment) register
  writeReg(MOD2_CSN, NRF_REG_EN_RXADDR, NRF_EN_RXADDR_P0); // Enable Pipe 0 to match TX
  writeReg(MOD2_CSN, NRF_REG_RF_CH, NRF_RF_CH_DEFAULT); //Writes channel value 10 (2410 MHz) to Radio 2's RF_CH register, tuning its physical radio frequency synthesizer to the same channel as Radio 1
  writeReg(MOD2_CSN, NRF_REG_RF_SETUP, NRF_RF_SETUP_1MBPS); //Writes 0x06 to Radio 2's RF_SETUP register, matching Radio 1's 1 Mbps air data rate and 0 dBm settings
  writeReg(MOD2_CSN, NRF_REG_RX_PW_P0, NRF_PAYLOAD_LEN_4); // Configure Pipe 0 payload width

  // Flush FIFOs to clear any stale data
  digitalWrite(MOD1_CSN, LOW); SPI.transfer(NRF_CMD_FLUSH_TX); digitalWrite(MOD1_CSN, HIGH);
  digitalWrite(MOD2_CSN, LOW); SPI.transfer(NRF_CMD_FLUSH_RX); digitalWrite(MOD2_CSN, HIGH);

  // Clear status interrupt flags
  writeReg(MOD1_CSN, NRF_REG_STATUS, NRF_STATUS_CLEAR_ALL);
  writeReg(MOD2_CSN, NRF_REG_STATUS, NRF_STATUS_CLEAR_ALL);

  // Set pipe addresses (Both must use Pipe 0 and match 0xE7)
  setAddress(MOD1_CSN, NRF_REG_TX_ADDR, NRF_ADDR_BYTE_DEFAULT);
  setAddress(MOD2_CSN, NRF_REG_RX_ADDR_P0, NRF_ADDR_BYTE_DEFAULT); 

  // Power up modules: Radio 1 in TX mode, Radio 2 in RX mode
  writeReg(MOD1_CSN, NRF_REG_CONFIG, NRF_CONFIG_PWR_UP);                                    // PRIM_RX = 0 (TX)
  writeReg(MOD2_CSN, NRF_REG_CONFIG, NRF_CONFIG_PWR_UP | NRF_CONFIG_PRIM_RX);               // PRIM_RX = 1 (RX)
  delay(5);

  // Keep Radio 2's CE pin HIGH continuously to maintain RX listening mode
  digitalWrite(MOD2_CE, HIGH);
}
/*static inline void initModulesAtSpeed(uint32_t spiHz) {
  SPI.endTransaction(); //Restarts the SPI transaction
  SPI.beginTransaction(SPISettings(spiHz, MSBFIRST, SPI_MODE0)); //Configures the spi bus hardware parameters by setting the master clock frequency, specifying the bit order (MSB first), and selecting the SPI mode (mode 0)

  // Reset/Power down registers initially
  writeReg(MOD1_CSN, NRF_REG_CONFIG, 0x00);
  writeReg(MOD2_CSN, NRF_REG_CONFIG, 0x00);
  delay(10);

  // Configure Radio 1 (TX Setup)
  writeReg(MOD1_CSN, NRF_REG_EN_AA, NRF_EN_AA_NONE); //Writes 0x00 to Radio 1's EN_AA (Auto-Acknowledgment) register
  writeReg(MOD1_CSN, NRF_REG_EN_RXADDR, NRF_EN_RXADDR_P0); //Writes 0x01 to Radio 1's EN_RXADDR register, enabling Data Pipe 0
  writeReg(MOD1_CSN, NRF_REG_RF_CH, NRF_RF_CH_DEFAULT); //Writes channel value 10 (2410 MHz) to Radio 1's RF_CH register, tuning its physical radio frequency synthesizer to channel 10
  writeReg(MOD1_CSN, NRF_REG_RF_SETUP, NRF_RF_SETUP_1MBPS); //Writes 0x06 to Radio 1's RF_SETUP register, configuring its internal modulator for a 1 Mbps air data rate and maximum 0 dBm output power

  // Configure Radio 2 (RX Setup)
  writeReg(MOD2_CSN, NRF_REG_EN_AA, NRF_EN_AA_NONE); 
  writeReg(MOD2_CSN, NRF_REG_EN_RXADDR, NRF_EN_RXADDR_P0); // Use Pipe 0 to match TX
  writeReg(MOD2_CSN, NRF_REG_RF_CH, NRF_RF_CH_DEFAULT); 
  writeReg(MOD2_CSN, NRF_REG_RF_SETUP, NRF_RF_SETUP_1MBPS); 
  writeReg(MOD2_CSN, NRF_REG_RX_PW_P0, NRF_PAYLOAD_LEN_4); // Configure Pipe 0 payload width

  writeReg(MOD2_CSN, NRF_REG_EN_AA, NRF_EN_AA_NONE); //Writes 0x00 to Radio 2's EN_AA (Auto-Acknowledgment) register
  writeReg(MOD2_CSN, NRF_REG_EN_RXADDR, NRF_EN_RXADDR_P1); //Writes 0x02 to Radio 2's EN_RXADDR register, enabling Data Pipe 1
  writeReg(MOD2_CSN, NRF_REG_RF_CH, NRF_RF_CH_DEFAULT); //Writes channel value 10 (2410 MHz) to Radio 2's RF_CH register, tuning its physical radio frequency synthesizer to the same channel as Radio 1
  writeReg(MOD2_CSN, NRF_REG_RF_SETUP, NRF_RF_SETUP_1MBPS); //Writes 0x06 to Radio 2's RF_SETUP register, matching Radio 1's 1 Mbps air data rate and 0 dBm settings
  writeReg(MOD2_CSN, NRF_REG_RX_PW_P1, NRF_PAYLOAD_LEN_4); //Writes 4 to Radio 2's RX_PW_P1 register, locking Data Pipe 1 to expect a static, fixed payload width of exactly 4 bytes per incoming packet
   
  // Flush FIFOs to clear any stale data
  digitalWrite(MOD1_CSN, LOW); SPI.transfer(NRF_CMD_FLUSH_TX); digitalWrite(MOD1_CSN, HIGH);
  digitalWrite(MOD2_CSN, LOW); SPI.transfer(NRF_CMD_FLUSH_RX); digitalWrite(MOD2_CSN, HIGH);

  // Clear status interrupt flags
  writeReg(MOD1_CSN, NRF_REG_STATUS, NRF_STATUS_CLEAR_ALL);
  writeReg(MOD2_CSN, NRF_REG_STATUS, NRF_STATUS_CLEAR_ALL);

  // Set pipe addresses
  /*setAddress(MOD1_CSN, NRF_REG_TX_ADDR, NRF_ADDR_BYTE_DEFAULT);
  setAddress(MOD2_CSN, NRF_REG_RX_ADDR_P1, NRF_ADDR_BYTE_DEFAULT);
  setAddress(MOD1_CSN, NRF_REG_TX_ADDR, NRF_ADDR_BYTE_DEFAULT);
  setAddress(MOD2_CSN, NRF_REG_RX_ADDR_P0, NRF_ADDR_BYTE_DEFAULT); // Target Pipe 0 address register (0x0A)

  // Power up modules: Radio 1 in TX mode, Radio 2 in RX mode
  writeReg(MOD1_CSN, NRF_REG_CONFIG, NRF_CONFIG_PWR_UP);                                    // PRIM_RX = 0 (TX)
  writeReg(MOD2_CSN, NRF_REG_CONFIG, NRF_CONFIG_PWR_UP | NRF_CONFIG_PRIM_RX);               // PRIM_RX = 1 (RX)
  delay(5);
} */