/*
 This example show how to send more than 8 bytes with can usign CFP (CAN Fragmentation Protocol)
*/

#include <Arduino.h>
#include <CAN.h>

#include <CFP.h>

#define TX_GPIO_NUM   21  // Connects to CTX
#define RX_GPIO_NUM   22  // Connects to CRX


#define SAMPLE_CFP_SOURCE 0
#define SAMPLE_CFP_DESTINATION 1
#define SAMPLE_CFP_ID 404

uint8_t sample_data[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 16, 17};

CAN_CFP_DATA s_packet = {sample_data, sizeof(sample_data)};

//==================================================================================//
void setup() {
  Serial.begin (115200);
  while (!Serial);
  delay (1000);
  Serial.println ("CAN Receiver/Receiver");
  // Set the pins
  CAN.setPins (RX_GPIO_NUM, TX_GPIO_NUM);
  // start the CAN bus at 500 kbps
  if (!CAN.begin (500E3)) {
    Serial.println ("Starting CAN failed!");
    while (1);
  }
  else {
    Serial.println ("CAN Initialized");
  }
}

void loop() {
  listeCFP();
}