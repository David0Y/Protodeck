#include <Arduino.h>
#include <SPI.h>

#include "nrf24driver.h"
#include "registercmds.h"
#include "stm32f4heartbeat.h"

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

// 10 MHz for single speed test, change the value to whichever you wanna test as a single speed
const uint32_t SPI_SINGLE_SPEED[] = { 10000000 }; // saved as array to be compatible with the function
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
  pinMode(HEARTBEAT_PIN, OUTPUT);
  //Driving CSN high during startup/setup prevents the modules from picking up random startup noise on the SPI bus as valid commands
  //Driving CE low during startup/setup puts the modules into standby mode, preventing them from transmitting or receiving any data until the test begins
  digitalWrite(MOD1_CSN, HIGH);
  digitalWrite(MOD1_CE, LOW);
  digitalWrite(MOD2_CSN, HIGH);
  digitalWrite(MOD2_CE, LOW);

  // Set timer overflow to 2 Hz (triggers an interrupt every 500ms)
    heartbeatTimer->setOverflow(2, HERTZ_FORMAT); 
    heartbeatTimer->attachInterrupt(heartbeatISR);
    heartbeatTimer->resume(); // Start the hardware timer

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
  int testOutcome = 0; //switch-case determination variable used in runSpiFrequencySweep

  //IMPORTANT: Only use one of the two functions below and comment out the other one that isnt used

  //use this function for testing several speeds one after another
  //runSpiFrequencySweep(SPI_SPEED_STEPS, maxStableSpeed, failedSpeed, NUM_SPEED_STEPS, PACKETS_PER_BATCH, MIN_SUCCESS_RATE, testOutcome);
  //use this function for testing a single speed
  runSpiFrequencySweep(SPI_SINGLE_SPEED, maxStableSpeed, failedSpeed, 1, PACKETS_PER_BATCH, MIN_SUCCESS_RATE, testOutcome);

  Serial.println("\r\n==============================================\r\n");
  
  switch (testOutcome) {
        case 4: // Multiple speeds tested successfully (full sweep pass)
            Serial.printf("RESULT: Max Stable SPI Clock = %lu MHz\n", maxStableSpeed / 1000000);
            triggerSuccessBlink();
            break;

        case 1: // Single speed tested successfully
            Serial.printf("RESULT: Successful Single-Speed Test at %lu MHz\n", maxStableSpeed / 1000000);
            triggerSuccessBlink();
            break;

        case 2: // Multiple speeds tested, failed partway through
            Serial.println("RESULT: Failed transmission because minimum success rate was not reached or due to a critical SPI fault.\r\n");
            Serial.printf("Max stable speed before failure = %lu MHz\r\n", failedSpeed / 1000000);
            triggerFailurePause();
            break;

        case 3: // Single speed tested and failed
            Serial.println("RESULT: Single-speed test failed. Minimum success rate not reached or critical SPI fault.\r\n");
            Serial.printf("Failed speed = %lu MHz\r\n", failedSpeed / 1000000);
            triggerFailurePause();
            break;
            
        default:
            break;
    }
  
  
  /*if (maxStableSpeed > 0) {
   
    //Serial.printf(" RESULT: Max Stable SPI Clock = %lu MHz\n", maxStableSpeed / 1000000);
    Serial.printf(" RESULT: Succesful test run at %lu MHz\n", maxStableSpeed / 1000000);
    triggerSuccessBlink();
  } else { //This isnt good, implement a function if needed
    //Serial.println(" RESULT: Failed to pass threshold at minimum speed (1 MHz).");
    Serial.println(" RESULT: Failed transmission because of minimum succesrate not being reached or a critical spi fault.\r\n");
    Serial.printf("Max stable speed before failure = %lu MHz", failedSpeed / 1000000);
    triggerFailurePause();
  }*/
  Serial.println("\r\n==============================================\r\n");
  
  delay(10000);



}