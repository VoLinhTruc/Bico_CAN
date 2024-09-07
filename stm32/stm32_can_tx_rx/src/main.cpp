/*
This is simple example to send random data to CAN bus in 20Hz rate, using delay (not recommended in real implementations).
*/

#include "Arduino.h"
#include "STM32_CAN.h"

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

// STM32_CAN Can(CAN1, DEF); //Use PA11/12 pins for CAN1.
STM32_CAN Can(CAN1, ALT); //Use PB8/PB9 pins for CAN1.

CAN_message_t CAN_TX_msg;
CAN_message_t CAN_RX_msg;

Message_Receiving_State uart_message_receiving_state = UART_MESSAGE_RECEIVING_IDLE;
uint8_t uart_receving_message_buf[UART_MESSAGE_MAX_SIZE] = {0};
uint8_t uart_receving_message_buf_index = 0;
uint8_t can_frame_payload_length = 0;

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
  switch (uart_message_receiving_state)
  {
    case UART_MESSAGE_RECEIVING_IDLE:
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
    
    case UART_MESSAGE_RECEIVING_INPROGRESS:
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
    
    case UART_MESSAGE_RECEIVING_DONE_OK:
      // Serial.println("UART_MESSAGE_RECEIVING_DONE_OK");
      CAN_TX_msg.timestamp = ((uint32_t)uart_receving_message_buf[TIMESTAMP_INDEX_IN_UART_MESSAGE+3] << 24) |\
       ((uint32_t)uart_receving_message_buf[TIMESTAMP_INDEX_IN_UART_MESSAGE+2] << 16) |\
        ((uint32_t)uart_receving_message_buf[TIMESTAMP_INDEX_IN_UART_MESSAGE+1] << 8) |\
         ((uint32_t)uart_receving_message_buf[TIMESTAMP_INDEX_IN_UART_MESSAGE+0]);

      CAN_TX_msg.len = uart_receving_message_buf[CAN_FRAME_DLC_INDEX_IN_UART_MESSAGE];

      // CAN_TX_msg.id is 4 byte (MSB) number, the logic below take the correct byte from buf array to CAN_TX_msg.id
      CAN_TX_msg.id = ((uint32_t)uart_receving_message_buf[CAN_ID_INDEX_IN_UART_MESSAGE+3] << 24) |\
       ((uint32_t)uart_receving_message_buf[CAN_ID_INDEX_IN_UART_MESSAGE+2] << 16) |\
        ((uint32_t)uart_receving_message_buf[CAN_ID_INDEX_IN_UART_MESSAGE+1] << 8) |\
         ((uint32_t)uart_receving_message_buf[CAN_ID_INDEX_IN_UART_MESSAGE+0]);

      CAN_TX_msg.flags.extended = (CAN_TX_msg.id > VALUE_11BIT_MAX);
      
      for (uint8_t i = 0; i < CAN_TX_msg.len; i++)
      {
        CAN_TX_msg.buf[i] = uart_receving_message_buf[CAN_PAYLOAD_INDEX_IN_UART_MESSAGE + i];
      }

      Can.write(CAN_TX_msg);
      uart_message_receiving_state = UART_MESSAGE_RECEIVING_IDLE;

      digitalWrite(PC13, !digitalRead(PC13));
    break;
    
    case UART_MESSAGE_RECEIVING_DONE_NOT_OK:
      // Serial.println("UART_MESSAGE_RECEIVING_DONE_NOT_OK");
      uart_message_receiving_state = UART_MESSAGE_RECEIVING_IDLE;
    break;

    default:
    break;
  }

  if (Can.read(CAN_RX_msg) ) {    
    Serial.write(START_OF_UART_MESSAGE_TOKEN);

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

    Serial.write(END_OF_UART_MESSAGE_TOKEN);

    digitalWrite(PC13, !digitalRead(PC13));
  }
}
