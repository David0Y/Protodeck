#include "spi_sweep_runner.h"



void runSpiFrequencySweep(const uint32_t *speeds,uint32_t &maxStableSpeed, uint32_t &failedSpeed, uint32_t NUM_SPEED_STEPS,  uint32_t PACKETS_PER_BATCH, uint32_t MIN_SUCCESS_RATE, int &testOutcome, uint32_t *txpayload) {
    
    //This for loop runs through for the amount of speeds specified in the NUM-SPEED_STEPS variable calculation 
    for (size_t step = 0; step < NUM_SPEED_STEPS; step++) {
        
        //Saves the currently tested frequency speed from the SPI_SPEED_STEPS array into a variable
        uint32_t currentSpeed = speeds[step]; 
        uint32_t speedMHz = currentSpeed / 1000000; //Puts the tested frequency in a readable MHz format
        handleOverclockLogging(currentSpeed, speedMHz); //prints out the current tested speed and speed attempt
        initModulesAtSpeed(currentSpeed); //Initializes the hardware bringup for the nRF24 modules for the currently tested speed frequency

        // counter reset at the start of each speed test
        uint32_t txSuccessCount = 0; //counts the number of successful transmissions from the TX module (Radio 1) to the RX module (Radio 2)
        uint32_t rxSuccessCount = 0; //counts the number of successful receptions from the RX module (Radio 2) to the TX module (Radio 1)
        uint32_t spiFaultCount = 0; //counts the (drastic) SPI bus faults

        unsigned long startTime = millis(); //Start of stopwatch

        //runs inner for loop for each packet in the current speed test (batch)
        for (uint32_t i = 1; i <= PACKETS_PER_BATCH; i++) {
    //-----Remove the following line if you  dont want the fourth (last) byte to be overwritten dynamically-----
            txpayload[3] = (uint8_t)(i & 0xFF); //dynamically overwrites the last byte of the payload with the current packet index (1-100) 
            sendTestPacket(i, txpayload); //sends the payload through the TX module (Radio 1) over the air
            //polls the status for both Transceiver (Radio 1 TX) and receiver (Radio 2 RX), reads out the received packages
            pollTransceiverStatuses(txSuccessCount, rxSuccessCount, spiFaultCount, i, txpayload);
        }
        //Calculates the duration of the current speed test 
        unsigned long duration = millis() - startTime; //End of stopwatch
        float passRate = ((float)rxSuccessCount / PACKETS_PER_BATCH) * 100.0; //calculates the success rate in percentages

        Serial.printf("Done (%lu ms) | RX Success: %3lu/%lu (%.1f%%) | SPI Faults: %lu\r\n", 
                      duration, rxSuccessCount, PACKETS_PER_BATCH, passRate, spiFaultCount);

//if the calculated passrate is above the minimum success rate and the (drastic) spi fault count is equal to 0, 
//save the currently tested speed as the (current) maximum stable one
        if (passRate >= MIN_SUCCESS_RATE && spiFaultCount == 0) {
            maxStableSpeed = currentSpeed;
            //Serial.println("\r\n[PASS]");
        } else {
            failedSpeed = currentSpeed; //capture the current incrementally failed speed
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
//handles the tracking of the overclock attempts
void handleOverclockLogging(uint32_t currentSpeed, uint32_t speedMHz) {
    static int overclockAttempt = 0; 
    //If the current iteration speed is greater than the physical maximum of 10 MHz, increment the counter for each following attempt
    if (currentSpeed > 10000000) { 
        overclockAttempt++;
        Serial.printf("Overclock attempt %d: %2lu MHz...\r\n ", overclockAttempt, speedMHz);
    } else { //reset the counter to 0 if the current speed is below the physical threshold
        overclockAttempt = 0;
        Serial.printf("Testing SPI @ %2lu MHz...\r\n", speedMHz);
    }
}
//Lets Radio 1 (TX) transfer the 4 bytes in the payload array over the air
void sendTestPacket(uint32_t i, uint32_t *txpayload) {
    //clearing the status interrupt flags from a previous transmission
    writeReg(MOD1_CSN, NRF_REG_STATUS, NRF_STATUS_CLEAR_ALL);
    
    digitalWrite(MOD1_CSN, LOW); //start of the SPI transaction
    SPI.transfer(NRF_CMD_W_TX_PAYLOAD); // Load/prime the TX FIFO buffer on Radio 1 (TX module)
    // Transmit the array over SPI
    for (int j = 0; j < 4; j++) {
        SPI.transfer(txpayload[j]);
    }
   
    digitalWrite(MOD1_CSN, HIGH); //end of the SPI transaction

    digitalWrite(MOD1_CE, HIGH); //triggers the packet transfer transmission
    delayMicroseconds(15); //mandatory transmission trigger delay of 10-15 microseconds (datasheet requirement)
    digitalWrite(MOD1_CE, LOW); //return to standby after sending the package

    delayMicroseconds(1500); //breathing room before the next cycle
}
//polls the status for both Transceiver (Radio 1 TX) and receiver (Radio 2 RX), reads out the received packages
void pollTransceiverStatuses(uint32_t &txCount, uint32_t &rxCount, uint32_t &faultCount, uint32_t &loopcount, uint32_t *txpayload) {
    // Check Radio 1 (TX)
    uint8_t m1_stat = readReg(MOD1_CSN, NRF_REG_STATUS); //grabs a snapshot of the current Radio 1 (TX) register
    //checks for catastrophic spi faults: 0x00 for a shorted/dead bus or 0xFF for a disconnected/ floating MISO line
    if (m1_stat == 0x00 || m1_stat == 0xFF) { //a bit too drastic,a more detailed fault detection is something for later
        faultCount++; //counts up for each found catastrophic spi fault
        //Isolates the TX_DS (Data Sent) interrupt-flag-bit from the status register, if said bit is high (1), txsucesscount counts up
    } else if (m1_stat & NRF_STATUS_TX_DS) {
        txCount++; //counts up for each succesful transmission
    }

    // Check Radio 2 (RX)
    uint8_t m2_stat = readReg(MOD2_CSN, NRF_REG_STATUS); //grabs a snapshot of the current Radio 2 (RX) register
    //checks for catastrophic spi faults: 0x00 for a shorted/dead bus or 0xFF for a disconnected/ floating MISO line
    if (m2_stat == 0x00 || m2_stat == 0xFF) {
        faultCount++; //counts up for each found catastrophic spi fault
        //Isolates the RX_DR (Data Ready) interrupt-flag-bit from the status register
    //If said bit is high (1), the function readReceivedPacket is called 
     } else if (m2_stat & NRF_STATUS_RX_DR) {  
      readReceivedPacket(loopcount, txpayload); //calls the function to read the received packet and print it out
        rxCount++; //counts up for each succesful reception
    }
}


//This function reads the received packet from Radio 2 (RX) and prints out the TX and RX payloads side by side for comparison
void readReceivedPacket(uint32_t packetIndex, uint32_t *txpayload) {
    uint8_t rxPayload[4];
    
    //Select the receiver module (Radio 2)
    digitalWrite(MOD2_CSN, LOW);
    
    //Send command to read payload from RX FIFO
    SPI.transfer(NRF_CMD_R_RX_PAYLOAD);
    
    //Read the 4 bytes of the received payload data by clocking out dummy bytes over MOSI
   for (int j = 0; j < 4; j++) {
        rxPayload[j] = SPI.transfer(0xFF);
    }
    
    
    //Deselect chip to end transaction
    digitalWrite(MOD2_CSN, HIGH);

    //Clear the RX_DR (Data Ready) interrupt flag in the status register
    writeReg(MOD2_CSN, NRF_REG_STATUS, NRF_STATUS_RX_DR);

    //Print out the captured packet bytes cleanly via Serial
// Print side-by-side comparison (treating the first 3 as chars, 4th as hex)
    Serial.printf("Packet #%3lu | TX: %c%c%c[0x%02X] (%02X %02X %02X %02X) | RX: %c%c%c[0x%02X] (%02X %02X %02X %02X)\r\n", 
                  packetIndex,
                  txpayload[0], txpayload[1], txpayload[2], txpayload[3],
                  txpayload[0], txpayload[1], txpayload[2], txpayload[3],
                  rxPayload[0], rxPayload[1], rxPayload[2], rxPayload[3],
                  rxPayload[0], rxPayload[1], rxPayload[2], rxPayload[3]);


    

}