#include "Arduino.h"
#include "Common_Include.h"
#include "CAN_Transmit.h"
#include "CAN_Receive.h"

STM32_CAN Can(CAN1, ALT); //Use PB8/PB9 pins for CAN1.

void setup() {
  // put your setup code here, to run once:
  	Serial.begin(115200);
	Can.begin();
	Can.setBaudRate(500000);  //500KBPS
	pinMode(PC13, OUTPUT);
	
	TaskHandle_t tb_can_transmit = NULL;
	xTaskCreate(tbCanTransmit, "tb_can_transmit", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &tb_can_transmit);
	
	TaskHandle_t tb_can_receive = NULL;
	xTaskCreate(tbCanReceive, "tb_can_receive", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &tb_can_receive);

	vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}


