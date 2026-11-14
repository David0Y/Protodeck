#include "spi_sweep_runner.h"



void runSpiFrequencySweep(const uint32_t *speeds,uint32_t &maxStableSpeed, uint32_t &failedSpeed, uint32_t NUM_SPEED_STEPS,  uint32_t PACKETS_PER_BATCH, uint32_t MIN_SUCCESS_RATE, int &testOutcome, uint32_t *txpayload) {
    
    for (size_t step = 0; step < NUM_SPEED_STEPS; step++) {
        
        
        uint32_t currentSpeed = speeds[step];
        uint32_t speedMHz = currentSpeed / 1000000;

        handleOverclockLogging(currentSpeed, speedMHz);
        initModulesAtSpeed(currentSpeed);

        uint32_t txSuccessCount = 0;
        uint32_t rxSuccessCount = 0;
        uint32_t spiFaultCount = 0;

        unsigned long startTime = millis();

        
        for (uint32_t i = 1; i <= PACKETS_PER_BATCH; i++) {
            //uint8_t txPayload[4] = {'P', 'T', 'D', (uint8_t)(i & 0xFF)};
            txpayload[3] = (uint8_t)(i & 0xFF); //remove this if you  dont want to be the fourth (last) byte to be overwritten dynamically
            sendTestPacket(i, txpayload);
            pollTransceiverStatuses(txSuccessCount, rxSuccessCount, spiFaultCount, i, txpayload);
        }

        unsigned long duration = millis() - startTime;
        float passRate = ((float)rxSuccessCount / PACKETS_PER_BATCH) * 100.0;

        Serial.printf("Done (%lu ms) | RX Success: %3lu/%lu (%.1f%%) | SPI Faults: %lu\r\n", 
                      duration, rxSuccessCount, PACKETS_PER_BATCH, passRate, spiFaultCount);

        if (passRate >= MIN_SUCCESS_RATE && spiFaultCount == 0) {
            maxStableSpeed = currentSpeed;
            //Serial.println("\r\n[PASS]");
        } else {
            failedSpeed = currentSpeed;
            Serial.println("\r\n[FAIL]");
            break;
        }
    }
    testOutcome = 0; //switch-case determination variable

    if (NUM_SPEED_STEPS == 1) {
        testOutcome = (failedSpeed == 0) ? 1 : 3; // 1 = Single Pass, 3 = Single Fail
    } else {
        testOutcome = (failedSpeed == 0) ? 4 : 2; // 4 = Multi Pass, 2 = Multi Fail
    }

      



}

void handleOverclockLogging(uint32_t currentSpeed, uint32_t speedMHz) {
    static int overclockAttempt = 0; //to print out the current overclock attempt, this variable saves the current overclock attempt incrementally with each loop
    if (currentSpeed > 10000000) {
        overclockAttempt++;
        Serial.printf("Overclock attempt %d: %2lu MHz...\r\n ", overclockAttempt, speedMHz);
    } else {
        overclockAttempt = 0;
        Serial.printf("Testing SPI @ %2lu MHz...\r\n", speedMHz);
    }
}

void sendTestPacket(uint32_t i, uint32_t *txpayload) {
    
    writeReg(MOD1_CSN, NRF_REG_STATUS, NRF_STATUS_CLEAR_ALL);
    
    digitalWrite(MOD1_CSN, LOW);
    SPI.transfer(NRF_CMD_W_TX_PAYLOAD);
    // Transmit the array over SPI
    for (int j = 0; j < 4; j++) {
        SPI.transfer(txpayload[j]);
    }
    /*SPI.transfer('P');
    SPI.transfer('T');
    SPI.transfer('D');
    //SPI.transfer((uint8_t)('0' + (i % 10)));
    SPI.transfer((uint8_t)((i & 0xFF)));*/
    digitalWrite(MOD1_CSN, HIGH);

    digitalWrite(MOD1_CE, HIGH);
    delayMicroseconds(15);
    digitalWrite(MOD1_CE, LOW);

    delayMicroseconds(1500);
}

void pollTransceiverStatuses(uint32_t &txCount, uint32_t &rxCount, uint32_t &faultCount, uint32_t &loopcount, uint32_t *txpayload) {
    // Check Radio 1 (TX)
    uint8_t m1_stat = readReg(MOD1_CSN, NRF_REG_STATUS);
    if (m1_stat == 0x00 || m1_stat == 0xFF) { //a bit too drastic,a more detailed fault detection is something for later
        faultCount++;
    } else if (m1_stat & NRF_STATUS_TX_DS) {
        txCount++;
    }

    // Check Radio 2 (RX)
    uint8_t m2_stat = readReg(MOD2_CSN, NRF_REG_STATUS);
    if (m2_stat == 0x00 || m2_stat == 0xFF) {
        faultCount++;
    } else if (m2_stat & NRF_STATUS_RX_DR) {
      readReceivedPacket(loopcount, txpayload); //calls the function to read the received packet and print it out
        rxCount++; //counts up for each succesful reception
    }
}



void readReceivedPacket(uint32_t packetIndex, uint32_t *txpayload) {
    uint8_t rxPayload[4];
    
    // Select the receiver module (Radio 2)
    digitalWrite(MOD2_CSN, LOW);
    
    // Send command to read payload from RX FIFO
    SPI.transfer(NRF_CMD_R_RX_PAYLOAD);
    
    // Read the 4 bytes of the received payload data by clocking out dummy bytes over MOSI
   for (int j = 0; j < 4; j++) {
        rxPayload[j] = SPI.transfer(0xFF);
    }
    /* uint8_t b0 = SPI.transfer(0xFF);
    uint8_t b1 = SPI.transfer(0xFF);
    uint8_t b2 = SPI.transfer(0xFF);
    uint8_t b3 = SPI.transfer(0xFF); */
    
    // Deselect chip to end transaction
    digitalWrite(MOD2_CSN, HIGH);

    // 5. Clear the RX_DR (Data Ready) interrupt flag in the status register
    writeReg(MOD2_CSN, NRF_REG_STATUS, NRF_STATUS_RX_DR);

    // Print out the captured packet bytes cleanly via Serial
// Print side-by-side comparison (treating the first 3 as chars, 4th as hex)
    Serial.printf("Packet #%3lu | TX: %c%c%c[0x%02X] (%02X %02X %02X %02X) | RX: %c%c%c[0x%02X] (%02X %02X %02X %02X)\r\n", 
                  packetIndex,
                  txpayload[0], txpayload[1], txpayload[2], txpayload[3],
                  txpayload[0], txpayload[1], txpayload[2], txpayload[3],
                  rxPayload[0], rxPayload[1], rxPayload[2], rxPayload[3],
                  rxPayload[0], rxPayload[1], rxPayload[2], rxPayload[3]);


    /*Serial.printf("RX Packet #%lu -> Payload: %c%c%c0x%02X (Hex: %02X %02X %02X %02X)\r\n", 
                  packetIndex, b0, b1, b2, b3, b0, b1, b2, b3);*/

}