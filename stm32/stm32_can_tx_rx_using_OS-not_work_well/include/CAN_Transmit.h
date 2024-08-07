#ifndef CAN_TRANSMIT_H
#define CAN_TRANSMIT_H

#include "Common_Include.h"

#define TIMESTAMP_INDEX_IN_UART_MESSAGE 1
#define CAN_FRAME_DLC_INDEX_IN_UART_MESSAGE 4
#define CAN_ID_INDEX_IN_UART_MESSAGE 5
#define CAN_ID_LENGTH_IN_UART_MESSAGE 4
#define CAN_PAYLOAD_INDEX_IN_UART_MESSAGE (CAN_ID_INDEX_IN_UART_MESSAGE + CAN_ID_LENGTH_IN_UART_MESSAGE)
#define UART_MESSAGE_MAX_SIZE 17

typedef enum 
{
  UART_MESSAGE_RECEIVING_IDLE = 0,
  UART_MESSAGE_RECEIVING_INPROGRESS,
  UART_MESSAGE_RECEIVING_DONE_OK,
  UART_MESSAGE_RECEIVING_DONE_NOT_OK
} Message_Receiving_State;

void tbCanTransmit(void* pvParameters);

#endif //CAN_TRANSMIT_H
