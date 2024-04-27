/*
This is simple example to send random data to CAN bus in 20Hz rate, using delay (not recommended in real implementations).
*/

#include "STM32_CAN.h"

#define MESSAGE_MAX_SIZE 16
#define START_OF_FRAME_TOKEN 0xAA
#define END_OF_FRAME_TOKEN 0xBB
#define FRAME_DLC_INDEX 4
#define ARBITRATION_ID_INDEX_START 5
#define ARBITRATION_ID_LENGTH 4
#define PAYLOAD_INDEX_START (ARBITRATION_ID_INDEX_START + ARBITRATION_ID_LENGTH)
#define MESSAGE_MAX_SIZE 17

typedef enum 
{
  MESSAGE_RECEIVING_IDLE = 0,
  MESSAGE_RECEIVING_INPROGRESS,
  MESSAGE_RECEIVING_DONE_OK,
  MESSAGE_RECEIVING_DONE_NOT_OK
} Message_Receiving_State;

STM32_CAN Can(CAN1, DEF); //Use PA11/12 pins for CAN1.

static CAN_message_t CAN_TX_msg;
static CAN_message_t CAN_RX_msg;

Message_Receiving_State message_receiving_state = MESSAGE_RECEIVING_IDLE;
uint8_t receving_message[MESSAGE_MAX_SIZE] = {0};
uint8_t receving_message_index = 0;
uint8_t frame_payload_length = 0;

void setup() {
  // PIN_SERIAL_TX
  // PIN_SERIAL_RX
  Serial.begin(115200);

  Can.begin();
  //Can.setBaudRate(250000);  //250KBPS
  Can.setBaudRate(500000);  //500KBPS
  //Can.setBaudRate(1000000);  //1000KBPS

    pinMode(PC13, OUTPUT);  
}

void loop() {
  switch (message_receiving_state)
  {
    case MESSAGE_RECEIVING_IDLE:
      // // Serial.println("MESSAGE_RECEIVING_IDLE");

      if (Serial.available() > 0) 
      {
        uint8_t incomingByte = Serial.read();
        if (incomingByte == START_OF_FRAME_TOKEN)
        {
          message_receiving_state = MESSAGE_RECEIVING_INPROGRESS;
          receving_message_index = 0;
          frame_payload_length = 0;
        }
      }

    break;
    
    case MESSAGE_RECEIVING_INPROGRESS:
      // // Serial.println("MESSAGE_RECEIVING_INPROGRESS");

      if (Serial.available() > 0) 
      {
        uint8_t incomingByte = Serial.read();
        if (receving_message_index == FRAME_DLC_INDEX)
        {
          frame_payload_length = incomingByte;
        }
        if (receving_message_index < PAYLOAD_INDEX_START + frame_payload_length)
        {
          receving_message[receving_message_index] = incomingByte;
          receving_message_index++;
          message_receiving_state = MESSAGE_RECEIVING_INPROGRESS;
        }
        else
        {
          if (incomingByte == END_OF_FRAME_TOKEN)
          {
            message_receiving_state = MESSAGE_RECEIVING_DONE_OK;
          }
          else
          {
            message_receiving_state = MESSAGE_RECEIVING_DONE_NOT_OK;
          }
        }
      }
    break;
    
    case MESSAGE_RECEIVING_DONE_OK:
      // // Serial.println("MESSAGE_RECEIVING_DONE_OK");

      CAN_TX_msg.id = ((uint32_t)receving_message[ARBITRATION_ID_INDEX_START+3] << 24) |\
       ((uint32_t)receving_message[ARBITRATION_ID_INDEX_START+2] << 16) |\
        ((uint32_t)receving_message[ARBITRATION_ID_INDEX_START+1] << 8) |\
         ((uint32_t)receving_message[ARBITRATION_ID_INDEX_START+0]);

      CAN_TX_msg.len = receving_message[FRAME_DLC_INDEX];

      for (uint8_t i = 0; i < CAN_TX_msg.len; i++)
      {
        CAN_TX_msg.buf[i] = receving_message[PAYLOAD_INDEX_START + i];
      }

      Can.write(CAN_TX_msg);
      message_receiving_state = MESSAGE_RECEIVING_IDLE;
      
      // Serial.println();
      // Serial.println("CAN send OK");

      digitalWrite(PC13, !digitalRead(PC13));
    break;
    
    case MESSAGE_RECEIVING_DONE_NOT_OK:
      // // Serial.println("MESSAGE_RECEIVING_DONE_OK");

      message_receiving_state = MESSAGE_RECEIVING_IDLE;
      
      // Serial.println();
      // Serial.println("CAN send NOT OK");
    break;

    default:
    break;
  }


  if (Can.read(CAN_RX_msg) ) {    
    Serial.write(0xAA);

    uint32_t timestamp = millis();
    Serial.write(timestamp & 0xFF);
    Serial.write((timestamp >> 8) & 0xFF);
    Serial.write((timestamp >> 16) & 0xFF);
    Serial.write((timestamp >> 24) & 0xFF);
    
    Serial.write(CAN_RX_msg.len);

    Serial.write(CAN_RX_msg.id & 0xFF);
    Serial.write((CAN_RX_msg.id >> 8) & 0xFF);
    Serial.write((CAN_RX_msg.id >> 16) & 0xFF);
    Serial.write((CAN_RX_msg.id >> 24) & 0xFF);

    for(int i=0; i < CAN_RX_msg.len; i++) 
    {
      Serial.write(CAN_RX_msg.buf[i]);
    }

    Serial.write(0xBB);
  }

}
