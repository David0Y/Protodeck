#include <Arduino.h>
#include <SPI.h>

/* 
  Shared SPI Pins (Bus 1):
  SCK:  PA5
  MISO: PA6
  MOSI: PA7

  Module I:
  CSN_1: PB0
  CE_1:  PB10

  Module II:
  CSN_2: PA4
  CE_2:  PA3
*/

// --- Pin Definitions ---
// Module I
#define CSN_1 PB0
#define CE_1  PB10

// Module II
#define CSN_2 PA4
#define CE_2  PA3

// nRF24L01+ Register Commands
#define R_REGISTER  0x00  // Command to read a register
#define CONFIG_REG  0x00  // Address of the CONFIG register

// Struct to hold sanity check test results
struct NRF_Status {
  uint8_t statusVal;
  uint8_t configVal;
};

// Helper function to query a specific nRF module by its CSN pin
NRF_Status checkNRF(uint8_t csnPin) {
  NRF_Status result;

  // 1. Start SPI transaction (1 MHz, MSBFIRST, SPI_MODE0)
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  
  // 2. Pull CSN LOW to select the target module
  digitalWrite(csnPin, LOW);

  // 3. Send Read Command; receives STATUS byte on first transfer
  result.statusVal = SPI.transfer(R_REGISTER | CONFIG_REG);

  // 4. Send dummy byte to clock out the CONFIG register value
  result.configVal = SPI.transfer(0x00);

  // 5. Pull CSN HIGH to release the SPI bus
  digitalWrite(csnPin, HIGH);
  SPI.endTransaction();

  return result;
}

// Helper function to print evaluation for a module with explicit naming
void printResults(const char* moduleName, NRF_Status res) {
  Serial.print("[");
  Serial.print(moduleName);
  Serial.print("] Status: 0x");
  if (res.statusVal < 0x10) Serial.print("0"); // Leading zero padding
  Serial.print(res.statusVal, HEX);

  Serial.print(" | Config: 0x");
  if (res.configVal < 0x10) Serial.print("0");
  Serial.print(res.configVal, HEX);

  // Evaluate result with the module name explicitly highlighted in the success/error string
  if (res.configVal == 0x08) {
    Serial.print(" -> SUCCESS: ");
    Serial.print(moduleName);
    Serial.println(" responded correctly (Config = 0x08)!");
  } else if (res.configVal == 0x00 || res.configVal == 0xFF) {
    Serial.print(" -> ERROR: ");
    Serial.print(moduleName);
    Serial.println(" is DEAD or FLOATING. Check power/CSN wiring!");
  } else {
    Serial.print(" -> UNEXPECTED: ");
    Serial.print(moduleName);
    Serial.println(" responded, but CONFIG value is corrupted.");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  // Initialize hardware SPI bus
  SPI.begin();

  // Configure CSN and CE pins as outputs
  pinMode(CSN_1, OUTPUT);
  pinMode(CE_1, OUTPUT);
  pinMode(CSN_2, OUTPUT);
  pinMode(CE_2, OUTPUT);
  
  // CRITICAL: Pull BOTH CSN pins HIGH immediately!
  // If either boots LOW, it will corrupt SPI reads for the other module.
  digitalWrite(CSN_1, HIGH); 
  digitalWrite(CE_1, LOW);
  
  digitalWrite(CSN_2, HIGH); 
  digitalWrite(CE_2, LOW);

  Serial.println("--- Dual nRF24L01+ SPI Sanity Check ---");
}

void loop() {
  // Test Module I (PB0 / PB10)
  NRF_Status mod1 = checkNRF(CSN_1);
  printResults("Module I (PB0)", mod1);

  // Test Module II (PA4 / PA3)
  NRF_Status mod2 = checkNRF(CSN_2);
  printResults("Module II (PA4)", mod2);

  Serial.println("------------------------------------------------------------------");
  delay(2000); // Check every 2 seconds
}