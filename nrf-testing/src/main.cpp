#include <Arduino.h>
#include <SPI.h>

#include "nrf24driver.h"
#include "registercmds.h"

// --- Module Pin Mapping ---
#define MOD1_CSN PB0
#define MOD1_CE  PB10

#define MOD2_CSN PA4
#define MOD2_CE  PA3

// --- Test Configuration ---
const uint32_t PACKETS_PER_BATCH = 100; //100 packets per batch for a clean 100% success rate calculation
const float MIN_SUCCESS_RATE     = 95.0; // a 95 % quality threshold, meaning out of 100 packages 95 have to arrive successfully

// Extended SPI Speed Steps (Testing past 10 MHz limit)
const uint32_t SPI_SPEED_STEPS[] = {
  1000000,   // 1 MHz
  4000000,   // 4 MHz
  8000000,   // 8 MHz
  10000000,  // 10 MHz (Official Datasheet Max)
  12000000,  // Overclock Step 1 (12 MHz attempt)
  16000000,  // Overclock Step 2 (16 MHz attempt)
  20000000   // Overclock Step 3 (20 MHz attempt)
};

// Calculate the number of elements in the SPI_SPEED_STEPS array
const size_t NUM_SPEED_STEPS = sizeof(SPI_SPEED_STEPS) / sizeof(SPI_SPEED_STEPS[0]);



void setup() {
  Serial.begin(115200);
  //waits for the serial monitor to open for a total of 3 seconds, after which the code will execute regardless
  while (!Serial && millis() < 3000);

  // Initialize the SPI related CSN and CE pins and set initial states for both nRF24 modules
  //Tells the blackpill that these specific pins work as outputs for the inputs of the nRF24 modules
  pinMode(MOD1_CSN, OUTPUT);
  pinMode(MOD1_CE, OUTPUT);
  pinMode(MOD2_CSN, OUTPUT);
  pinMode(MOD2_CE, OUTPUT);

  //Driving CSN high during startup/setup prevents the modules from picking up random startup noise on the SPI bus as valid commands
  //Driving CE low during startup/setup puts the modules into standby mode, preventing them from transmitting or receiving any data until the test begins
  digitalWrite(MOD1_CSN, HIGH);
  digitalWrite(MOD1_CE, LOW);
  digitalWrite(MOD2_CSN, HIGH);
  digitalWrite(MOD2_CE, LOW);

  //Mandatory functioncall to initialize the SPI bus and set the default parameters for the SPI peripheral on the blackpill
  SPI.begin();

  Serial.println("\n==============================================");
  Serial.println("  nRF24 SPI Overclocking & Threshold Benchmark ");
  Serial.println("==============================================\n");
}

void loop() {
  //These three variables get declared at the start of each loop iteration to ensure they are reset for each new test cycle
  uint32_t maxStableSpeed = 0; //saves the current stable speed as the maximum stable one, incrementally updated for each loop
  uint32_t failedSpeed = 0; // if the current speed fails the threshold test, it gets saved under this variable
int overclockAttempt = 0; //to print out the current overclock attempt, this variable saves the current overclock attempt incrementally with each loop

//------------Start of the main loop--------------//
//This main for loop runs through for each tested speed specified in the SPI_SPEED_STEPS array, starting from 1 MHz up to the highest overclock attempt of 20 MHz for a total of 7 iterations
  for (size_t step = 0; step < NUM_SPEED_STEPS; step++) {
    uint32_t currentSpeed = SPI_SPEED_STEPS[step]; //Saves the currently tested frequency speed from the SPI_SPEED_STEPS array into a variable
    uint32_t speedMHz = currentSpeed / 1000000; //Puts the tested frequency in a readable MHz format

    //Serial.printf("Testing SPI @ %2lu MHz... ", speedMHz);

if (currentSpeed > 10000000) { //if current speed is above the (theoritical) maximum speed of the nRF24 module, run this if statement
    overclockAttempt++; //Overclock attempt counter increments for each speed above 10 MHz up to the highest overclock attempt 3 (20 MHz)
    
    Serial.printf("Overclock attempt %d: %2lu MHz...\r\n", overclockAttempt, speedMHz);
  } else {
    overclockAttempt = 0; // Reset overclock attempt counter for standard speeds
    Serial.printf("Testing SPI @ %2lu MHz...\r\n", speedMHz);
  }

    //initialize the hardware configuration for both nRF24 modules at the currently tested speed
    initModulesAtSpeed(currentSpeed);

    uint32_t txSuccessCount = 0; //counts the number of successful transmissions from the TX module (Radio 1) to the RX module (Radio 2)
    uint32_t rxSuccessCount = 0; //counts the number of successful receptions from the RX module (Radio 2) to the TX module (Radio 1)
    uint32_t spiFaultCount  = 0; //counts the SPI bus faults

    unsigned long startTime = millis(); //Start of the stopwatch for the current loop iteration

    //inner loop for each packet in the current batch (100)
    for (uint32_t i = 1; i <= PACKETS_PER_BATCH; i++) {
      //clears the register of the transmitting module (Radio 1) to ensure that the status register doesnt include leftovers from a previous loop iteration
      //writeReg(MOD1_CSN, 0x07, 0x70);
      writeReg(MOD1_CSN, NRF_REG_STATUS, NRF_STATUS_CLEAR_ALL);
      digitalWrite(MOD1_CSN, LOW); //start of the SPI transaction
      //SPI.transfer(0xA0);
      SPI.transfer(NRF_CMD_W_TX_PAYLOAD); // Load/prime the TX FIFO buffer on Radio 1 (TX module)
      SPI.transfer('S'); //Shifts out 'S' into byte position 0
      SPI.transfer('P'); //Shifts out 'P' into byte position 1
      SPI.transfer('D'); //Shifts out 'D' into byte position 2
      SPI.transfer((uint8_t)('0' + (i % 10))); //generates a dynamic numerical character to complete the 4-byte payload
      digitalWrite(MOD1_CSN, HIGH); //end of the SPI transaction

      digitalWrite(MOD1_CE, HIGH); //triggers the packet transfer transmission
      delayMicroseconds(15); //CE needs to be pulled high for a minimum of 10 microseconds to trigger the transmission succesfully
      digitalWrite(MOD1_CE, LOW); //return to standby after sending the package

      delayMicroseconds(1500); //breathing room before the next cycle

      //uint8_t m1_stat = readReg(MOD1_CSN, 0x07);
      uint8_t m1_stat = readReg(MOD1_CSN, NRF_REG_STATUS); //grabs a snapshot of the current Radio 1 (TX) register
      //checks for catastrophic spi faults: 0x00 for a shorted/dead bus or 0xFF for a disconnected/ floating MISO line
      if (m1_stat == 0x00 || m1_stat == 0xFF) {
        spiFaultCount++; //counts up for each found catastrophic spi fault
        //Isolates the TX_DS (Data Sent) interrupt-flag-bit from the status register, if said bit is high (1), txsucesscount counts up
      //} else if (m1_stat & (1 << 5)) {
      } else if (m1_stat & NRF_STATUS_TX_DS) {
        txSuccessCount++; //counts up for each succesful transmission
      }

      //uint8_t m2_stat = readReg(MOD2_CSN, 0x07);
      uint8_t m2_stat = readReg(MOD2_CSN, NRF_REG_STATUS); //grabs a snapshot of the current Radio 2 (RX) register
      //checks for catastrophic spi faults: 0x00 for a shorted/dead bus or 0xFF for a disconnected/ floating MISO line
      if (m2_stat == 0x00 || m2_stat == 0xFF) { 
        spiFaultCount++; //counts up for each found catastrophic spi fault
      //Isolates the RX_DR (Data Ready) flag-bit 
       //} else if (m2_stat & (1 << 6)) {
    } else if (m2_stat & NRF_STATUS_RX_DR) {
        /*digitalWrite(MOD2_CSN, LOW); //If RX_DR is high (1), initiate the following Radio readout sequence

        //SPI.transfer(0x61); 
        SPI.transfer(NRF_CMD_R_RX_PAYLOAD); //Reads the packet sitting at the front of Radio 2's (RX) Receive FIFO buffer
        //Required NOP-byte transmit over MOSI for all of the 4 payload bytes to get the data from the Radio's MISO line
        for (int k = 0; k < 4; k++) SPI.transfer(0xFF); 
        digitalWrite(MOD2_CSN, HIGH); //ends the transaction
        //writeReg(MOD2_CSN, 0x07, (1 << 6));
        writeReg(MOD2_CSN, NRF_REG_STATUS, NRF_STATUS_RX_DR); //clearing the flag */
        readReceivedPacket(i); //calls the function to read the received packet and print it out
        rxSuccessCount++; //counts up for each succesful reception
      }
    }

    //subtracts the initial millis starttime watch against the current millis value, ending the stopwatch procedure
    //assigned value now shows the  time in milliseconds it took for the packages to transmit, receive, and be verified 
    unsigned long duration = millis() - startTime; 
    float passRate = ((float)rxSuccessCount / PACKETS_PER_BATCH) * 100.0; //calculates the success rate in percentages

    //Serial.printf("Done (%lu ms) | RX Success: %3lu/%lu (%.1f%%%%) | SPI Faults: %lu ", 
              //duration, rxSuccessCount, PACKETS_PER_BATCH, passRate, spiFaultCount);
    Serial.printf("Done (%lu ms) | RX Success: %3lu/%lu (%.1f%%) | SPI Faults: %lu ", 
                  duration, rxSuccessCount, PACKETS_PER_BATCH, passRate, spiFaultCount);
    

    //if the calculated passrate is above the minimum success rate and the spi fault count is equal to 0, save the currently tested speed as the (current) maximum stable one
    if (passRate >= MIN_SUCCESS_RATE && spiFaultCount == 0) { 
      maxStableSpeed = currentSpeed; 
      Serial.println("[PASS]");
    } else {
      failedSpeed = currentSpeed; //capture the current incrementally failed speed
      Serial.println("[FAIL]");
      break;
    }
  }

  Serial.println("\n==============================================");
  if (maxStableSpeed > 0) {

  //if (failedSpeed == 0) {
    //Im unhappy with how these strings are formulated, need a better idea here
    Serial.printf(" RESULT: Max Stable SPI Clock = %lu MHz\n", maxStableSpeed / 1000000);
  } else { //This isnt good, implement a function if needed
    //Serial.println(" RESULT: Failed to pass threshold at minimum speed (1 MHz).");
    Serial.println(" RESULT: Failed transmission because of minimum succesrate not being reached or a critical spi fault.\n");
    Serial.printf("Max stable speed before failure = %lu MHz", failedSpeed / 1000000);
  }
  Serial.println("==============================================\n");

  delay(10000);
}