#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "nrf24driver.h"

void readReceivedPacket(uint32_t packetIndex) {
    // 1. Select the receiver module (Radio 2)
    digitalWrite(MOD2_CSN, LOW);
    
    // 2. Send command to read payload from RX FIFO
    SPI.transfer(NRF_CMD_R_RX_PAYLOAD);
    
    // 3. Read the 4 bytes of payload data by clocking out dummy bytes over MOSI
    uint8_t b0 = SPI.transfer(0xFF);
    uint8_t b1 = SPI.transfer(0xFF);
    uint8_t b2 = SPI.transfer(0xFF);
    uint8_t b3 = SPI.transfer(0xFF);
    
    // 4. Deselect chip to end transaction
    digitalWrite(MOD2_CSN, HIGH);

    // 5. Clear the RX_DR (Data Ready) interrupt flag in the status register
    writeReg(MOD2_CSN, NRF_REG_STATUS, NRF_STATUS_RX_DR);

    // 6. Print out the captured packet bytes cleanly via Serial
    Serial.printf("RX Packet #%lu -> Payload: %c%c%c%c (Hex: %02X %02X %02X %02X)\r\n", 
                  packetIndex, b0, b1, b2, b3, b0, b1, b2, b3);
}