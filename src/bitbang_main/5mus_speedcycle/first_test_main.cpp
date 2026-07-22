#include <Arduino.h>

#define CE_1 PB10
#define calpin PB6
#define SCK PA5
#define CSN_2 PA4
#define CE_2 PA3

void setup() { 
pinMode(CE_1, OUTPUT);
pinMode(CE_2, OUTPUT);
pinMode(SCK, OUTPUT);
pinMode(CSN_2, OUTPUT);
//digitalWrite(CSN_2, HIGH); // SPI idle state is HIGH
//digitalWrite(SCK, LOW);  // SPI idle state is LOW //Be SUPER careful with this, shortcircuits to pins that are set up to low will fry your board, comment it out if necessary

    pinMode(calpin, OUTPUT); //used to calibrate the oscilloscope, not related to anything else in the code
    pinMode(PC13, OUTPUT); //strictly used as a heartbeat LED, not related to anything else in the code
    
    
}
    
void loop() {
  
  digitalWrite(PC13, LOW);  // ON
  delay(250);
  digitalWrite(PC13, HIGH); // OFF
  delay(250);

  /*
  digitalWrite(CE_1, HIGH);
  delayMicroseconds(15); 
  digitalWrite(CE_1, LOW);
  delayMicroseconds(10);

  digitalWrite(CE_2, HIGH);
  delayMicroseconds(15);
  digitalWrite(CE_2, LOW);
  delayMicroseconds(10);  
  */

   /*
  digitalWrite(SCK, HIGH);
  delayMicroseconds(5);  // 5µs High = 100 kHz target
  digitalWrite(SCK, LOW);
  delayMicroseconds(5);  // 5µs Low = 100 kHz target
  /* */
 
  /* 
  digitalWrite(CSN_2, HIGH);
  delayMicroseconds(5);  // 5µs High = 100 kHz target
  digitalWrite(CSN_2, LOW);
  delayMicroseconds(5);  // 5µs Low = 100 kHz target
  /* */
  
  /* 
  digitalWrite(CE_2, HIGH);
  delayMicroseconds(5);  // 5µs High = 100 kHz target
  digitalWrite(CE_2, LOW);
  delayMicroseconds(5);  // 5µs Low = 100 kHz target
  /* */


  /* */
  digitalWrite(CE_1, HIGH);
  delayMicroseconds(5);  // 5µs High = 100 kHz target
  digitalWrite(CE_1, LOW);
  delayMicroseconds(5);  // 5µs Low = 100 kHz target
  /* */


  /* 
  // 1. Start the frame
  digitalWrite(CSN_2, LOW);
  delayMicroseconds(5); // Small setup time before clock starts
  
  // 2. Burst 8 clock cycles (1 Byte)
  for (int i = 0; i < 8; i++) {
    digitalWrite(SCK, HIGH);
    delayMicroseconds(5);
    digitalWrite(SCK, LOW);
    delayMicroseconds(5);
  }
  
  delayMicroseconds(5); // Small hold time after clock ends
  // 3. End the frame
  digitalWrite(CSN_2, HIGH);

  // 4. Long pause so it's easy to find in PulseView
  delay(50);

  /*
  digitalWrite(calpin, HIGH);
  delayMicroseconds(500);
  digitalWrite(calpin, LOW);
  delayMicroseconds(500);
  */
}

