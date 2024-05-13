#include "CAN_Transmit.h"

CAN_message_t CAN_TX_msg;

Message_Receiving_State uart_message_receiving_state = UART_MESSAGE_RECEIVING_IDLE;
uint8_t uart_receving_message_buf[UART_MESSAGE_MAX_SIZE] = {0};
uint8_t uart_receving_message_buf_index = 0;
uint8_t can_frame_payload_length = 0;

void tbCanTransmit( void *pvParameters )
{
	// Setup - begin --------------------------------------------------------------------

	// Setup - end --------------------------------------------------------------------

	// Infnite loop
	while (1)
	{
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
	}
}