//
// Created by Fran Acién 2023-05-22
//

#ifndef CFP_h
#define CFP_h

#include <Arduino.h>

#include <CAN.h>

#define CAN_MAX_LENGTH 8

#define CFP_SOURCE_MASK 0x1F
#define CFP_SOURCE_OFFSET 24

#define CFP_DESTINATION_MASK 0x1F
#define CFP_DESTINATION_OFFSET 19

#define CFP_TYPE_MASK 0x1
#define CFP_TYPE_OFFSET 18

#define CFP_REMAIN_MASK 0xFF
#define CFP_REMAIN_OFFSET 10

#define CFP_ID_MASK 0x3FF
#define CFP_ID_OFFSET 0

#define CFP_BEGIN 0b0
#define CFP_MORE 0b1

typedef struct {
    uint8_t source : 5;
    uint8_t destination : 5;
    uint8_t type : 1;
    uint8_t remain : 8;
    uint16_t id : 10;
} CAN_CFP_HEADER;

typedef struct {
  uint8_t* data;      // Pointer to the data
  uint16_t length;    // How many bytes
} CAN_CFP_DATA;

void parseCFPHeader(uint32_t id, CAN_CFP_HEADER* packet);

CAN_CFP_HEADER createCFPHeader(uint8_t source, uint8_t destination, uint8_t type, uint8_t remain,
                      uint16_t id);

uint32_t buildCFPHeader(CAN_CFP_HEADER header);

void listeCFP();

void sendCFP(CAN_CFP_DATA frame_data, uint8_t source, uint8_t destination, uint16_t id);

void printCFPHeader(CAN_CFP_HEADER* packet);

#endif