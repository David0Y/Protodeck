#include <Arduino.h>
#include <SPI.h>

/* 
  Shared SPI Pins (Bus 1):
  SCK:  PA5 | MISO: PA6 | MOSI: PA7

  Module I:  CSN: PB0 | CE: PB10
  Module II: CSN: PA4 | CE: PA3
*/

#define CSN_1 PB0
#define CE_1  PB10
#define CSN_2 PA4
#define CE_2  PA3

#define R_REGISTER  0x00
#define CONFIG_REG  0x00

// Test frequencies to sweep through (in Hz)
const uint32_t testFrequencies[] = {
  1000000UL,  // 1 MHz  (Safe Baseline)
  2000000UL,  // 2 MHz
  4000000UL,  // 4 MHz
  8000000UL,  // 8 MHz  (Near nRF24 Spec Limit)
  10000000UL, // 10 MHz (Official nRF24 Max Spec)
  12000000UL  // 12 MHz (Overclock Attempt)
};

const uint8_t numFreqs = sizeof(testFrequencies) / sizeof(testFrequencies[0]);

// Tests a single module at a given SPI clock speed
bool testModuleAtSpeed(uint8_t csnPin, uint32_t speedHz) {
  SPI.beginTransaction(SPISettings(speedHz, MSBFIRST, SPI_MODE0));
  
  digitalWrite(csnPin, LOW);
  uint8_t status = SPI.transfer(R_REGISTER | CONFIG_REG);
  uint8_t configVal = SPI.transfer(0x00);
  digitalWrite(csnPin, HIGH);
  
  SPI.endTransaction();

  // Return true ONLY if valid config (0x08) and status (0x0E) are read
  return (configVal == 0x08 && (status & 0x0F) == 0x0E);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  SPI.begin();

  pinMode(CSN_1, OUTPUT);
  pinMode(CE_1, OUTPUT);
  pinMode(CSN_2, OUTPUT);
  pinMode(CE_2, OUTPUT);

  digitalWrite(CSN_1, HIGH);
  digitalWrite(CE_1, LOW);
  digitalWrite(CSN_2, HIGH);
  digitalWrite(CE_2, LOW);

  Serial.println("\n==================================================");
  Serial.println("     nRF24L01+ SPI Clock Speed Benchmark          ");
  Serial.println("==================================================");
}

void loop() {
  Serial.println("\n--- Starting Speed Sweep ---");

  for (uint8_t i = 0; i < numFreqs; i++) {
    uint32_t targetSpeed = testFrequencies[i];
    
    Serial.print("Testing @ ");
    Serial.print(targetSpeed / 1000000UL);
    Serial.print(" MHz -> ");

    // Run 50 rapid transactions to catch intermittent clock jitter/bit slips
    bool mod1Pass = true;
    bool mod2Pass = true;

    for (int run = 0; run < 50; run++) {
      if (!testModuleAtSpeed(CSN_1, targetSpeed)) mod1Pass = false;
      if (!testModuleAtSpeed(CSN_2, targetSpeed)) mod2Pass = false;
    }

    // Report results
    Serial.print("Mod I (PB0): ");
    Serial.print(mod1Pass ? "[PASS] " : "[FAIL] ");

    Serial.print("| Mod II (PA4): ");
    Serial.println(mod2Pass ? "[PASS]" : "[FAIL]");

    delay(100); // Short settle time between frequencies
  }

  Serial.println("--------------------------------------------------");
  Serial.println("Sweep complete. Restarting in 5 seconds...");
  delay(5000);
}