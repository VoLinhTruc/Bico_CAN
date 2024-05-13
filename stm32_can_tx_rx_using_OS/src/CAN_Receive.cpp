#include "CAN_Receive.h"

CAN_message_t CAN_RX_msg;

void tbCanReceive( void *pvParameters )
{
	// Setup - begin --------------------------------------------------------------------

	// Setup - end --------------------------------------------------------------------

	// Infnite loop
	while (1)
	{
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
		}
	}
}