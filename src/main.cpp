#include <Arduino.h>
#include <SPI.h>

/*GND Purple 1
VCC Brown 2
CE Red 3 (CE_2, PA3)
CSN Black 4 (CSN_2, PA4) black 4
SCK Brown 5 (PA5)
MOSI White 6 (PA7)
MISO Black 7 (PA6)
IRQ Black 8 (not actively tested here) */




// Define your GPIO pins based on your board layout
#define CSN_2 PA4   // Chip Select Not (Slam LOW to talk to nRF)
#define CE_2 PA3  // Chip Enable (Leave LOW for now, not needed for SPI check)

// nRF24L01+ Instruction Mnemonics
#define R_REGISTER    0x00  // Command to read a register
#define CONFIG_REG    0x00  // Address of the CONFIG register

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); } // Wait for serial monitor to open


     // Initialize the hardware SPI bus
    SPI.begin();

    pinMode(CSN_2, OUTPUT);
    pinMode(CE_2, OUTPUT);
    
    // De-select the nRF chip initially (CSN is Active-LOW)
    digitalWrite(CSN_2, HIGH); 
    digitalWrite(CE_2, LOW);

   
    Serial.println("--- nRF24L01+ SPI Sanity Check ---");
}

void loop() {
    // 1. Start SPI transaction: 1 MHz clock, MSBFIRST, SPI_MODE0
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    
    // 2. Pull CSN LOW to select the nRF24L01+
    digitalWrite(CSN_2, LOW);

    // 3. Send the Read Command for the CONFIG register
    // The nRF always shifts out its STATUS register byte on the very first transfer
    uint8_t status = SPI.transfer(R_REGISTER | CONFIG_REG);

    // 4. Send a dummy byte (0x00) to clock out the actual contents of the CONFIG register
    uint8_t configVal = SPI.transfer(0x00);

    // 5. Pull CSN HIGH to complete the transaction
    digitalWrite(CSN_2, HIGH);
    SPI.endTransaction();

    // 6. Print the results
    Serial.print("Status Register (Hex): 0x");
    Serial.println(status, HEX);
    
    Serial.print("Config Register (Hex): 0x");
    Serial.print(configVal, HEX);
    
    // Check against factory default expected value
    if (configVal == 0x08) {
        Serial.println(" -> SUCCESS! (Expected 0x08)");
    } else if (configVal == 0x00 || configVal == 0xFF) {
        Serial.println(" -> ERROR: Bus dead or floating. Check wiring/power.");
    } else {
        Serial.println(" -> UNEXPECTED VALUE: Communication partial or corrupted.");
    }

    Serial.println("---------------------------------------");
    delay(2000); // Repeat check every 2 seconds
}