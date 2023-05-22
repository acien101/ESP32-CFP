//
// Created by Fran Acién 2023-05-22 
//
#include "CFP.h"


void parseCFPHeader(uint32_t id, CAN_CFP_HEADER* packet){
  packet->source = ( id >> CFP_SOURCE_OFFSET) & CFP_SOURCE_MASK;
  packet->destination = ( id >> CFP_DESTINATION_OFFSET) & CFP_DESTINATION_MASK;
  packet->type = (id >> CFP_TYPE_OFFSET) & CFP_TYPE_MASK;
  packet->remain = (id >> CFP_REMAIN_OFFSET) & CFP_REMAIN_MASK;
  packet->id = id & CFP_ID_MASK;

  printCFPHeader(packet);
}

void printCFPHeader(CAN_CFP_HEADER* packet){
  Serial.printf("SOURCE: %d\n", packet->source);
  Serial.printf("DESTINATION: %d\n", packet->destination);
  Serial.printf("TYPE: %d\n", packet->type);
  Serial.printf("REMAIN: %d\n", packet->remain);
  Serial.printf("ID: %d\n", packet->id);
}

CAN_CFP_HEADER createCFPHeader(uint8_t source, uint8_t destination, uint8_t type, uint8_t remain,
                      uint16_t id){
  CAN_CFP_HEADER cfp_id = {source, destination, type, remain, id};

  return cfp_id;
}

uint32_t buildCFPHeader(CAN_CFP_HEADER header){
  uint32_t res = 0;   // Set to 0

  res = (header.source & CFP_SOURCE_MASK) << CFP_SOURCE_OFFSET |
        (header.destination & CFP_DESTINATION_MASK) << CFP_DESTINATION_OFFSET |
        (header.type & CFP_TYPE_MASK) << CFP_TYPE_OFFSET |
        (header.remain & CFP_REMAIN_MASK) << CFP_REMAIN_OFFSET |
        (header.id & CFP_ID_MASK) << CFP_ID_OFFSET;
  
  return res;
}

void listeCFP(){
  // Check if a packet has been received

  int packetSize = CAN.parsePacket();
  if (packetSize) {
    int availableBytes = CAN.available();

    // received a packet
    Serial.print ("Received Bytes: ");
    Serial.println(CAN.packetDlc());
    if (!CAN.packetExtended()) {
      Serial.print ("Not a extended header");

      CAN.flush();
      return;
    }

    Serial.print ("packet with id 0x");
    Serial.print (CAN.packetId(), HEX);

    // Create a buffer with the received data
    uint8_t* msg = new uint8_t[availableBytes];

    uint8_t readedBytes = 0;
    if(readedBytes = CAN.readBytes(msg, availableBytes) != availableBytes){
      Serial.print("Failed to read");
      Serial.printf("Readed: %d bytes from %d availableBytes\n", readedBytes, availableBytes);
      CAN.flush();
      return;
    }
    

    Serial.println("Reading extended id");
    uint32_t id = CAN.packetId();
    
    CAN_CFP_HEADER* cfp_id = (CAN_CFP_HEADER*) malloc(sizeof(CAN_CFP_HEADER));

    parseCFPHeader(id, cfp_id);

    uint8_t remain = cfp_id->remain;
    uint8_t num_packets = remain + 1;

    // Create buffer with the info of the packet
    uint8_t* buff = (uint8_t*) malloc(CAN.packetDlc() + remain * CAN_MAX_LENGTH);
    uint8_t* buff_p = buff;

    Serial.printf("Malloc: %d\n", CAN.packetDlc() + remain * CAN_MAX_LENGTH);
    memset(buff_p, 0, CAN.packetDlc() + remain * CAN_MAX_LENGTH);

    uint8_t start = 0;
    uint8_t end = min(start + CAN_MAX_LENGTH, CAN.packetDlc());

    memcpy(buff_p, msg, end - start);

    buff_p = buff_p + end - start;

    delete[] msg;
    free(cfp_id);

    Serial.printf("Reading from %d to %d, remain %d", start, end, remain);

    while(remain > 0){
      // Receive the next packet
      packetSize = CAN.parsePacket();
      if (packetSize) {
        availableBytes = CAN.available();

        // received a packet
        Serial.print ("Received ");
        if (!CAN.packetExtended()) {
          Serial.print ("Not a extended header");

          CAN.flush();
          return;
        }

        Serial.print ("packet with id 0x");
        Serial.print (CAN.packetId(), HEX);

        // Create a buffer with the received data
        msg = new uint8_t[availableBytes];
        readedBytes = 0;
        if(readedBytes = CAN.readBytes(msg, availableBytes) != availableBytes){
          Serial.print("Failed to read");
          Serial.printf("Readed: %d bytes from %d availableBytes\n", readedBytes, availableBytes);
          CAN.flush();
          return;
        }

        Serial.println("Reading extended id");
        id = CAN.packetId();
        
        cfp_id = (CAN_CFP_HEADER*) malloc(sizeof(CAN_CFP_HEADER));

        parseCFPHeader(id, cfp_id);

        remain = cfp_id->remain;

        start = end;
        end = min(start + CAN_MAX_LENGTH, start + CAN.packetDlc());

        memcpy(buff_p, msg, end - start);
        delete[] msg;

        buff_p = buff_p + end - start;

        Serial.printf("Reading from %d to %d, remain %d", start, end, remain);

      }
    }

    Serial.println("Reading buffer");
    for(int i = 0; i < end; i++){
      Serial.println(buff[i]);
    }

  }
}

void sendCFP(CAN_CFP_DATA frame_data, uint8_t source, uint8_t destination, uint16_t id){

  uint8_t* data = frame_data.data; // Pointer to data
  
  uint8_t num_packets = (frame_data.length - 1) / CAN_MAX_LENGTH + 1;
  uint8_t remain = num_packets - 1;

  Serial.printf("Creating packet \nNum_packets: %d\nRemain: %d\nLength: %d\n", num_packets, remain, frame_data.length);
  

  // Generate a sample header
  CAN_CFP_HEADER cfp_header = createCFPHeader(source, destination, CFP_BEGIN,
  remain, id);

  uint32_t can_cfp_id = buildCFPHeader(cfp_header);
  
  uint8_t start = 0;
  uint8_t end = min(start + CAN_MAX_LENGTH, (int) frame_data.length);

  Serial.printf("Sending from %d to %d, remain %d\n", start, end, remain);

  // Send first packet
  CAN.beginExtendedPacket(can_cfp_id);
  CAN.write(data, end - start);
  CAN.endPacket();

  data += end - start; // Update pointer

  while(remain > 0){
    remain -= 1;

    cfp_header = createCFPHeader(source, destination, CFP_MORE,
                                remain, id);
    
    can_cfp_id = buildCFPHeader(cfp_header);
    
    start = end;
    end = min(start + CAN_MAX_LENGTH, (int) frame_data.length);

    Serial.printf("Sending from %d to %d, remain %d\n", start, end, remain);

    CAN.beginExtendedPacket(can_cfp_id);
    CAN.write(data, end - start);
    CAN.endPacket();

    data += end - start; // Update pointer
  }
}
