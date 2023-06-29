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

CAN_CFP_DATA receiveCFP(){
  // Check if a packet has been received
  twai_message_t message;
  if (twai_receive(&message, 0) != ESP_OK){
    return {NULL, 0};
  } else{
    int availableBytes = message.data_length_code;

    // received a packet
    if(DEBUG){
      Serial.print ("Received Bytes: ");
      Serial.println(availableBytes);
    }

    if (!message.extd) {
      Serial.print ("Not a extended header");

      twai_clear_receive_queue(); // Clean the receive queue
      return {NULL, 0};
    }

    if (message.rtr) {
      Serial.print ("The message is a Remote Frame");

      twai_clear_receive_queue(); // Clean the receive queue
      return {NULL, 0};
    }

    if(DEBUG){
      Serial.printf("Packet with id 0x%X\n", message.identifier);
    }
    
    // Create a buffer with the received data
    uint8_t* msg = message.data;
    
    CAN_CFP_HEADER* cfp_id = (CAN_CFP_HEADER*) malloc(sizeof(CAN_CFP_HEADER));

    parseCFPHeader(message.identifier, cfp_id);

    uint8_t remain = cfp_id->remain;
    uint8_t num_packets = remain + 1;

    // Create a packet structure
    CAN_CFP_DATA res;
    
    // Create buffer with the info of the packet
    uint8_t* buff = (uint8_t*) malloc(availableBytes + remain * CAN_MAX_LENGTH);
    uint8_t* buff_p = buff;

    res.data = buff;

    if (DEBUG) Serial.printf("Malloc: %d\n", availableBytes + remain * CAN_MAX_LENGTH);
    memset(buff_p, 0, availableBytes + remain * CAN_MAX_LENGTH);

    uint8_t start = 0;
    uint8_t end = min(start + CAN_MAX_LENGTH, availableBytes);

    memcpy(buff_p, msg, end - start);

    buff_p = buff_p + end - start;  // Update pointer


    free(cfp_id);
    if (DEBUG) Serial.printf("Reading from %d to %d, remain %d\n", start, end, remain);

    while(remain > 0){
      Serial.println("Continue reading");

      // Check if a packet has been received
      twai_message_t message;
      if (twai_receive(&message, 0) != ESP_OK){
        return {NULL, 0};
      } else{
        availableBytes = message.data_length_code;

        // received a packet
        if(DEBUG){
          Serial.print ("Received Bytes: ");
          Serial.println(availableBytes);
        }

        if (!message.extd) {
          Serial.print ("Not a extended header");

          twai_clear_receive_queue(); // Clean the receive queue
          return {NULL, 0};
        }

        if(DEBUG){
          Serial.printf("Packet with id 0x%X\n", message.identifier);
        }

        // Create a buffer with the received data
        uint8_t* msg = message.data;
        
        CAN_CFP_HEADER* cfp_id = (CAN_CFP_HEADER*) malloc(sizeof(CAN_CFP_HEADER));

        parseCFPHeader(message.identifier, cfp_id);

        remain = cfp_id->remain;

        start = end;
        end = min(start + CAN_MAX_LENGTH, start + availableBytes);

        memcpy(buff_p, msg, end - start);

        buff_p = buff_p + end - start;

        if (DEBUG) Serial.printf("Reading from %d to %d, remain %d\n", start, end, remain);
      }
    }

    res.length = end;
    res.data = (uint8_t*) realloc(res.data, end); // Adjust the buffer

    return res;
  }
}

CAN_CFP_DATA receiveCFP(uint8_t destination){
    // Check if a packet has been received
  twai_message_t message;
  if (twai_receive(&message, 0) != ESP_OK){
    return {NULL, 0};
  } else{
    int availableBytes = message.data_length_code;

    // received a packet
    if(DEBUG){
      Serial.print ("Received Bytes: ");
      Serial.println(availableBytes);
    }

    if (!message.extd) {
      Serial.print ("Not a extended header");

      twai_clear_receive_queue(); // Clean the receive queue
      return {NULL, 0};
    }

    if (message.rtr) {
      Serial.print ("The message is a Remote Frame");

      twai_clear_receive_queue(); // Clean the receive queue
      return {NULL, 0};
    }

    if(DEBUG){
      Serial.printf("Packet with id 0x%X\n", message.identifier);
    }
    
    // Create a buffer with the received data
    uint8_t* msg = message.data;
    
    CAN_CFP_HEADER* cfp_id = (CAN_CFP_HEADER*) malloc(sizeof(CAN_CFP_HEADER));

    parseCFPHeader(message.identifier, cfp_id);

    if(cfp_id->destination != destination){
      Serial.print ("Received a message with another destination");

      twai_clear_receive_queue(); // Clean the receive queue
      return {NULL, 0};
    }

    uint8_t remain = cfp_id->remain;
    uint8_t num_packets = remain + 1;

    // Create a packet structure
    CAN_CFP_DATA res;
    
    // Create buffer with the info of the packet
    uint8_t* buff = (uint8_t*) malloc(availableBytes + remain * CAN_MAX_LENGTH);
    uint8_t* buff_p = buff;

    res.data = buff;

    if (DEBUG) Serial.printf("Malloc: %d\n", availableBytes + remain * CAN_MAX_LENGTH);
    memset(buff_p, 0, availableBytes + remain * CAN_MAX_LENGTH);

    uint8_t start = 0;
    uint8_t end = min(start + CAN_MAX_LENGTH, availableBytes);

    memcpy(buff_p, msg, end - start);

    buff_p = buff_p + end - start;  // Update pointer


    free(cfp_id);
    if (DEBUG) Serial.printf("Reading from %d to %d, remain %d\n", start, end, remain);

    while(remain > 0){
      Serial.println("Continue reading");

      // Check if a packet has been received
      twai_message_t message;
      if (twai_receive(&message, 0) != ESP_OK){
        return {NULL, 0};
      } else{
        availableBytes = message.data_length_code;

        // received a packet
        if(DEBUG){
          Serial.print ("Received Bytes: ");
          Serial.println(availableBytes);
        }

        if (!message.extd) {
          Serial.print ("Not a extended header");

          twai_clear_receive_queue(); // Clean the receive queue
          return {NULL, 0};
        }

        if(DEBUG){
          Serial.printf("Packet with id 0x%X\n", message.identifier);
        }

        // Create a buffer with the received data
        uint8_t* msg = message.data;
        
        CAN_CFP_HEADER* cfp_id = (CAN_CFP_HEADER*) malloc(sizeof(CAN_CFP_HEADER));

        parseCFPHeader(message.identifier, cfp_id);

        remain = cfp_id->remain;

        start = end;
        end = min(start + CAN_MAX_LENGTH, start + availableBytes);

        memcpy(buff_p, msg, end - start);

        buff_p = buff_p + end - start;

        if (DEBUG) Serial.printf("Reading from %d to %d, remain %d\n", start, end, remain);
      }
    }

    res.length = end;
    res.data = (uint8_t*) realloc(res.data, end); // Adjust the buffer

    return res;
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
  //Configure message to transmit
  twai_message_t message;
  message.identifier = can_cfp_id;
  message.extd = 1;
  message.rtr = 0;
  message.data_length_code = end - start;
  memcpy(message.data, data, end-start);

  Serial.println(data[0]);

  //Queue message for transmission
  if (twai_transmit(&message, 0) == ESP_OK) {
      if(DEBUG) printf("Message queued for transmission\n");
  } else {
      printf("Failed to queue message for transmission\n");
      return;
  }

  data += end - start; // Update pointer

  while(remain > 0){
    remain -= 1;

    cfp_header = createCFPHeader(source, destination, CFP_MORE,
                                remain, id);
    
    can_cfp_id = buildCFPHeader(cfp_header);
    
    start = end;
    end = min(start + CAN_MAX_LENGTH, (int) frame_data.length);

    Serial.printf("Sending from %d to %d, remain %d\n", start, end, remain);

    twai_message_t message;
    message.identifier = can_cfp_id;
    message.extd = 1;
    message.data_length_code = end - start;
    message.rtr = 0;
    memcpy(message.data, data, end-start);

    Serial.println(data[0]);

    //Queue message for transmission
    if (twai_transmit(&message, 0) == ESP_OK) {
        if(DEBUG) printf("Message queued for transmission\n");
    } else {
        printf("Failed to queue message for transmission\n");
        return;
    }
    data += end - start; // Update pointer
  }
}
