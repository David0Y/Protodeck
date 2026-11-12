#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "nrf24driver.h"

 //void runSpiFrequencySweep(const uint32_t *speeds,uint32_t &maxStableSpeed, uint32_t &failedSpeed, uint32_t NUM_SPEED_STEPS);
void runSpiFrequencySweep(const uint32_t *speeds,uint32_t &maxStableSpeed, uint32_t &failedSpeed, uint32_t NUM_SPEED_STEPS,  uint32_t PACKETS_PER_BATCH, uint32_t MIN_SUCCESS_RATE, int &testOutcome);
 void handleOverclockLogging(uint32_t currentSpeed, uint32_t speedMHz);
 void sendTestPacket(uint32_t i);
 void pollTransceiverStatuses(uint32_t &txCount, uint32_t &rxCount, uint32_t &faultCount, uint32_t &loopcount);
 void readReceivedPacket(uint32_t packetIndex);