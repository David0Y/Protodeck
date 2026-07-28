#include <Arduino.h>
#include <SPI.h>

// --- Module Pin Mapping ---
#define MOD1_CSN PB0
#define MOD1_CE  PB10

#define MOD2_CSN PA4
#define MOD2_CE  PA3

// --- Test Configuration ---
const uint32_t PACKETS_PER_BATCH = 100;
const float MIN_SUCCESS_RATE     = 95.0;

// Extended SPI Speed Steps (Testing past 10 MHz limit)
const uint32_t SPI_SPEED_STEPS[] = {
  1000000,   // 1 MHz
  4000000,   // 4 MHz
  8000000,   // 8 MHz
  10000000,  // 10 MHz (Official Datasheet Max)
  12000000,  // 12 MHz (Overclock Step 1)
  16000000,  // 16 MHz (Overclock Step 2)
  20000000   // 20 MHz (Overclock Step 3)
};
const size_t NUM_SPEED_STEPS = sizeof(SPI_SPEED_STEPS) / sizeof(SPI_SPEED_STEPS[0]);

// --- Low-Level SPI Helper Functions ---

void writeReg(uint8_t csn, uint8_t reg, uint8_t val) {
  digitalWrite(csn, LOW);
  SPI.transfer(0x20 | (reg & 0x1F));
  SPI.transfer(val);
  digitalWrite(csn, HIGH);
}

uint8_t readReg(uint8_t csn, uint8_t reg) {
  digitalWrite(csn, LOW);
  SPI.transfer(0x00 | (reg & 0x1F));
  uint8_t val = SPI.transfer(0xFF);
  digitalWrite(csn, HIGH);
  return val;
}

void setAddress(uint8_t csn, uint8_t reg, uint8_t addrByte) {
  digitalWrite(csn, LOW);
  SPI.transfer(0x20 | (reg & 0x1F));
  for (int i = 0; i < 5; i++) SPI.transfer(addrByte);
  digitalWrite(csn, HIGH);
}

void initModulesAtSpeed(uint32_t spiHz) {
  SPI.endTransaction();
  SPI.beginTransaction(SPISettings(spiHz, MSBFIRST, SPI_MODE0));

  writeReg(MOD1_CSN, 0x00, 0x00);
  writeReg(MOD2_CSN, 0x00, 0x00);
  delay(10);

  // Radio Settings: Channel 10, 1Mbps RF, No Auto-ACK
  writeReg(MOD1_CSN, 0x01, 0x00);
  writeReg(MOD1_CSN, 0x02, 0x00);
  writeReg(MOD1_CSN, 0x05, 10);
  writeReg(MOD1_CSN, 0x06, 0x06);

  writeReg(MOD2_CSN, 0x01, 0x00);
  writeReg(MOD2_CSN, 0x02, 0x01);
  writeReg(MOD2_CSN, 0x05, 10);
  writeReg(MOD2_CSN, 0x06, 0x06);
  writeReg(MOD2_CSN, 0x11, 4);

  digitalWrite(MOD1_CSN, LOW); SPI.transfer(0xE1); digitalWrite(MOD1_CSN, HIGH);
  digitalWrite(MOD2_CSN, LOW); SPI.transfer(0xE2); digitalWrite(MOD2_CSN, HIGH);
  writeReg(MOD1_CSN, 0x07, 0x70);
  writeReg(MOD2_CSN, 0x07, 0x70);

  setAddress(MOD1_CSN, 0x10, 0xE7);
  setAddress(MOD2_CSN, 0x0A, 0xE7);

  writeReg(MOD1_CSN, 0x00, 0x02);
  writeReg(MOD2_CSN, 0x00, 0x03);
  delay(5);

  digitalWrite(MOD2_CE, HIGH);
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  pinMode(MOD1_CSN, OUTPUT);
  pinMode(MOD1_CE, OUTPUT);
  pinMode(MOD2_CSN, OUTPUT);
  pinMode(MOD2_CE, OUTPUT);

  digitalWrite(MOD1_CSN, HIGH);
  digitalWrite(MOD1_CE, LOW);
  digitalWrite(MOD2_CSN, HIGH);
  digitalWrite(MOD2_CE, LOW);

  SPI.begin();

  Serial.println("\n==============================================");
  Serial.println("  nRF24 SPI Overclocking & Threshold Benchmark ");
  Serial.println("==============================================\n");
}

void loop() {
  uint32_t maxStableSpeed = 0;

  for (size_t step = 0; step < NUM_SPEED_STEPS; step++) {
    uint32_t currentSpeed = SPI_SPEED_STEPS[step];
    
    Serial.printf("Testing SPI @ %2lu MHz... ", currentSpeed / 1000000);

    initModulesAtSpeed(currentSpeed);

    uint32_t txSuccessCount = 0;
    uint32_t rxSuccessCount = 0;
    uint32_t spiFaultCount  = 0;

    unsigned long startTime = millis();

    for (uint32_t i = 1; i <= PACKETS_PER_BATCH; i++) {
      writeReg(MOD1_CSN, 0x07, 0x70);

      digitalWrite(MOD1_CSN, LOW);
      SPI.transfer(0xA0);
      SPI.transfer('S');
      SPI.transfer('P');
      SPI.transfer('D');
      SPI.transfer((uint8_t)('0' + (i % 10)));
      digitalWrite(MOD1_CSN, HIGH);

      digitalWrite(MOD1_CE, HIGH);
      delayMicroseconds(15);
      digitalWrite(MOD1_CE, LOW);

      delayMicroseconds(1500);

      uint8_t m1_stat = readReg(MOD1_CSN, 0x07);
      if (m1_stat == 0x00 || m1_stat == 0xFF) {
        spiFaultCount++;
      } else if (m1_stat & (1 << 5)) {
        txSuccessCount++;
      }

      uint8_t m2_stat = readReg(MOD2_CSN, 0x07);
      if (m2_stat == 0x00 || m2_stat == 0xFF) {
        spiFaultCount++;
      } else if (m2_stat & (1 << 6)) {
        digitalWrite(MOD2_CSN, LOW);
        SPI.transfer(0x61);
        for (int k = 0; k < 4; k++) SPI.transfer(0xFF);
        digitalWrite(MOD2_CSN, HIGH);

        writeReg(MOD2_CSN, 0x07, (1 << 6));
        rxSuccessCount++;
      }
    }

    unsigned long duration = millis() - startTime;
    float passRate = ((float)rxSuccessCount / PACKETS_PER_BATCH) * 100.0;

    Serial.printf("Done (%lu ms) | RX Success: %3lu/%lu (%.1f%%) | SPI Faults: %lu ", 
                  duration, rxSuccessCount, PACKETS_PER_BATCH, passRate, spiFaultCount);

    if (passRate >= MIN_SUCCESS_RATE && spiFaultCount == 0) {
      maxStableSpeed = currentSpeed;
      Serial.println("--> [PASS]");
    } else {
      Serial.println("--> [FAIL]");
      break;
    }
  }

  Serial.println("\n==============================================");
  if (maxStableSpeed > 0) {
    Serial.printf(" RESULT: Max Stable SPI Clock = %lu MHz\n", maxStableSpeed / 1000000);
  } else {
    Serial.println(" RESULT: Failed to pass threshold at minimum speed (1 MHz).");
  }
  Serial.println("==============================================\n");

  delay(10000);
}