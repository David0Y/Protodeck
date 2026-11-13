#include "nrf24driver.h"

// ==========================================
// LOW-LEVEL HARDWARE ABSTRACTION FUNCTIONS
// ==========================================

//This function writes a single byte to a specified register through the selected CSN-pin of the used nRF24 module
 void writeReg(uint8_t csn, uint8_t reg, uint8_t val) {
  digitalWrite(csn, LOW); //Pulls the CSN pin low to select the specific nRF24 module on the shared SPI bus
  SPI.transfer(NRF_CMD_W_REG_MASK | (reg & NRF_REG_MASK)); //Sends the write instruction (command) byte, combines the write instruction bit 0x20 with the 5-bit register address
  SPI.transfer(val); //clocks out the payload data byte to be written into that target register
  digitalWrite(csn, HIGH); //Pulls CSN high to end the SPI transaction, latches the configuration into the chip's internal register
}

//This function reads a single byte from a specified register through the selected CSN-pin of the used nRF24 module
 uint8_t readReg(uint8_t csn, uint8_t reg) {
  digitalWrite(csn, LOW); //Selects the targeted nRF24 module through the CSN pin
  SPI.transfer(NRF_CMD_R_REG_MASK | (reg & NRF_REG_MASK)); //Sends the read instruction byte, combines the read instruction bit 0x00 with the 5-bit register address
  uint8_t val = SPI.transfer(0xFF); // Shifts out a dummy byte (0xFF) to generate the SPI clock pulses
  digitalWrite(csn, HIGH); //Pulls CSN high to deselect the chip and end the SPI transaction
  return val;
}

//This function writes a 5-byte address to a specified register through the selected CSN-pin of the used nRF24 module
 void setAddress(uint8_t csn, uint8_t reg, uint8_t addrByte) {
  digitalWrite(csn, LOW); //Selects the targeted nRF24 module through the CSN pin
  SPI.transfer(NRF_CMD_W_REG_MASK | (reg & NRF_REG_MASK)); //Sends the write instruction (command) byte, combines the write instruction bit 0x20 with the 5-bit register address
  for (int i = 0; i < 5; i++) {
    SPI.transfer(addrByte); //Loops 5 times consecutively, shifting out all 5 bytes of the address in one unbroken SPI stream
  }
  digitalWrite(csn, HIGH); //Pulls CSN high to deselect the chip and end the SPI transaction
}

//This function encapsulates the entire hardware reset, configuration, and state-machine bringup sequence for both radios
void initModulesAtSpeed(uint32_t spiHz) {
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

