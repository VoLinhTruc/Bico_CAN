#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>

// Default for ESP32
#define CAN_TX		5
#define CAN_RX		4

#define LED 8
#define START_OF_UART_MESSAGE_TOKEN 0xAA
#define END_OF_UART_MESSAGE_TOKEN 0xBB
#define TIMESTAMP_INDEX_IN_UART_MESSAGE 1
#define CAN_FRAME_DLC_INDEX_IN_UART_MESSAGE 4
#define CAN_ID_INDEX_IN_UART_MESSAGE 5
#define CAN_ID_LENGTH_IN_UART_MESSAGE 4
#define CAN_PAYLOAD_INDEX_IN_UART_MESSAGE (CAN_ID_INDEX_IN_UART_MESSAGE + CAN_ID_LENGTH_IN_UART_MESSAGE)
#define UART_MESSAGE_MAX_SIZE 17
#define VALUE_11BIT_MAX 0x7FF

typedef enum 
{
  UART_MESSAGE_RECEIVING_IDLE = 0,
  UART_MESSAGE_RECEIVING_INPROGRESS,
  UART_MESSAGE_RECEIVING_DONE_OK,
  UART_MESSAGE_RECEIVING_DONE_NOT_OK
} Message_Receiving_State;

Message_Receiving_State uart_message_receiving_state = UART_MESSAGE_RECEIVING_IDLE;
uint8_t uart_receving_message_buf[UART_MESSAGE_MAX_SIZE] = {0};
uint8_t uart_receving_message_buf_index = 0;
uint8_t can_frame_payload_length = 0;

void setup() 
{
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  // start the CAN bus at 500 kbps
  if(ESP32Can.begin(ESP32Can.convertSpeed(500), CAN_TX, CAN_RX, 10, 10)) 
  {
      Serial.println("CAN bus started!");
  } else {
      Serial.println("CAN bus failed!");
  }
}

void loop() 
{
  switch (uart_message_receiving_state)
  {
    case UART_MESSAGE_RECEIVING_IDLE:
    {
      // Serial.println("UART_MESSAGE_RECEIVING_IDLE");
      if (Serial.available() > 0) 
      {
        uint8_t incomingByte = Serial.read();
        if (incomingByte == START_OF_UART_MESSAGE_TOKEN)
        {
          uart_message_receiving_state = UART_MESSAGE_RECEIVING_INPROGRESS;
          uart_receving_message_buf_index = 0;
          can_frame_payload_length = 0;
        }
      }
      break;
    }
    
    case UART_MESSAGE_RECEIVING_INPROGRESS:
    {
      // Serial.println("UART_MESSAGE_RECEIVING_INPROGRESS");
      if (Serial.available() > 0) 
      {
        uint8_t incomingByte = Serial.read();
        if (uart_receving_message_buf_index == CAN_FRAME_DLC_INDEX_IN_UART_MESSAGE)
        {
          can_frame_payload_length = incomingByte;
        }
        if (uart_receving_message_buf_index < CAN_PAYLOAD_INDEX_IN_UART_MESSAGE + can_frame_payload_length)
        {
          uart_receving_message_buf[uart_receving_message_buf_index] = incomingByte;
          uart_receving_message_buf_index++;
          uart_message_receiving_state = UART_MESSAGE_RECEIVING_INPROGRESS;
        }
        else
        {
          if (incomingByte == END_OF_UART_MESSAGE_TOKEN)
          {
            uart_message_receiving_state = UART_MESSAGE_RECEIVING_DONE_OK;
          }
          else
          {
            uart_message_receiving_state = UART_MESSAGE_RECEIVING_DONE_NOT_OK;
          }
        }
      }
      break;
    }
    
    case UART_MESSAGE_RECEIVING_DONE_OK:
    {
      // Serial.println("UART_MESSAGE_RECEIVING_DONE_OK");
      // CAN_TX_msg.timestamp = ((uint32_t)uart_receving_message_buf[TIMESTAMP_INDEX_IN_UART_MESSAGE+3] << 24) |\
       ((uint32_t)uart_receving_message_buf[TIMESTAMP_INDEX_IN_UART_MESSAGE+2] << 16) |\
        ((uint32_t)uart_receving_message_buf[TIMESTAMP_INDEX_IN_UART_MESSAGE+1] << 8) |\
         ((uint32_t)uart_receving_message_buf[TIMESTAMP_INDEX_IN_UART_MESSAGE+0]);

      uint8_t dlc = uart_receving_message_buf[CAN_FRAME_DLC_INDEX_IN_UART_MESSAGE];

      // CAN_TX_msg.id is 4 byte (MSB) number, the logic below take the correct byte from buf array to CAN_TX_msg.id
      uint32_t can_id = ((uint32_t)uart_receving_message_buf[CAN_ID_INDEX_IN_UART_MESSAGE+3] << 24) |\
       ((uint32_t)uart_receving_message_buf[CAN_ID_INDEX_IN_UART_MESSAGE+2] << 16) |\
        ((uint32_t)uart_receving_message_buf[CAN_ID_INDEX_IN_UART_MESSAGE+1] << 8) |\
         ((uint32_t)uart_receving_message_buf[CAN_ID_INDEX_IN_UART_MESSAGE+0]);

      bool is_can_extended = (can_id > VALUE_11BIT_MAX);

      CanFrame tx_frame = { 0 };
      tx_frame.identifier = can_id;
      tx_frame.extd = is_can_extended;
      tx_frame.data_length_code = dlc;
      
      for (uint8_t i = 0; i < dlc; i++)
      {
        tx_frame.data[i] = uart_receving_message_buf[CAN_PAYLOAD_INDEX_IN_UART_MESSAGE + i];
      }

      ESP32Can.writeFrame(tx_frame);

      uart_message_receiving_state = UART_MESSAGE_RECEIVING_IDLE;
      digitalWrite(LED, !digitalRead(LED));
    break;
    }
    
    case UART_MESSAGE_RECEIVING_DONE_NOT_OK:
    {
      // Serial.println("UART_MESSAGE_RECEIVING_DONE_NOT_OK");
      uart_message_receiving_state = UART_MESSAGE_RECEIVING_IDLE;
      break;
    }

    default:
    {
      break;
    }
  }

  byte i = 0;
  CanFrame rxFrame;
  if(ESP32Can.readFrame(rxFrame, 1)) 
  {
    Serial.write(START_OF_UART_MESSAGE_TOKEN);

    uint32_t timestamp = millis();
    Serial.write(timestamp & 0xFF);
    Serial.write((timestamp >> 8) & 0xFF);
    Serial.write((timestamp >> 16) & 0xFF);
    Serial.write((timestamp >> 24) & 0xFF);
    
    Serial.write(rxFrame.data_length_code);

    uint32_t can_id = rxFrame.identifier;
    Serial.write(can_id & 0xFF);
    Serial.write((can_id >> 8) & 0xFF);
    Serial.write((can_id >> 16) & 0xFF);
    Serial.write((can_id >> 24) & 0xFF);
    
    for(int i=0; i < rxFrame.data_length_code; i++) 
    {
      Serial.write(rxFrame.data[i]);
    }
    
    Serial.write(END_OF_UART_MESSAGE_TOKEN);
    
    digitalWrite(LED, !digitalRead(LED));
  }
}