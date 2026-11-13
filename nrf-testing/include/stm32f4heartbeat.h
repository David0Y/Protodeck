#pragma once

#include <Arduino.h>

extern const uint8_t HEARTBEAT_PIN; 
extern HardwareTimer *heartbeatTimer;
extern volatile bool isSuccessBlinking;
extern volatile bool suppressHeartbeat; 
extern volatile uint8_t successStep;







void heartbeatISR();

void triggerSuccessBlink();

// Call this immediately when a test run fails
void triggerFailurePause();
