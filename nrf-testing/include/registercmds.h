#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "nrf24driver.h"

const uint8_t HEARTBEAT_PIN = LED_BUILTIN; // Or your custom wiring pin
HardwareTimer *heartbeatTimer = new HardwareTimer(TIM2);
volatile bool isSuccessBlinking = false;
volatile bool suppressHeartbeat = false; // Flag to pause toggling on failure
volatile uint8_t successStep = 0;

// This ISR fires automatically in the background via hardware
/*void heartbeatISR() {
    digitalWrite(HEARTBEAT_PIN, !digitalRead(HEARTBEAT_PIN));
}*/

void heartbeatISR() {
    if (isSuccessBlinking) {
        successStep++;
        switch (successStep) {
            case 1: // First flash ON
                digitalWrite(HEARTBEAT_PIN, HIGH);
                break;
            case 2: // First flash OFF
                digitalWrite(HEARTBEAT_PIN, LOW);
                break;
            case 3: // Second flash ON
                digitalWrite(HEARTBEAT_PIN, HIGH);
                break;
            case 4: // Second flash OFF & Reset
                digitalWrite(HEARTBEAT_PIN, LOW);
                isSuccessBlinking = false;
                successStep = 0;
                // Restore timer to normal 2 Hz heartbeat (500ms intervals)
                heartbeatTimer->setOverflow(2, HERTZ_FORMAT);
                break;
        }
    }
    else if (suppressHeartbeat) {
        // Force LED OFF and skip toggling while the failure suppression is active
        digitalWrite(HEARTBEAT_PIN, LOW);
    } else {
        // Normal background heartbeat toggle
        digitalWrite(HEARTBEAT_PIN, !digitalRead(HEARTBEAT_PIN));
    }
}

void triggerSuccessBlink() {
    // Pause timer momentarily to reconfigure frequency for snappy blinks
    heartbeatTimer->pause();
    
    isSuccessBlinking = true;
    successStep = 0;
    
    // Set timer frequency to 12 Hz (~83ms per interrupt tick)
    heartbeatTimer->setOverflow(12, HERTZ_FORMAT); 
    
    // Fire the first ON state immediately
    digitalWrite(HEARTBEAT_PIN, HIGH);
    successStep = 1;
    
    heartbeatTimer->resume();
}

// Call this immediately when a test run fails
void triggerFailurePause() {
    suppressHeartbeat = true;
    digitalWrite(HEARTBEAT_PIN, LOW); // Force off instantly
}

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