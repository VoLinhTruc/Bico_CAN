#ifndef COMMON_INCLUDE_H
#define COMMON_INCLUDE_H

#include "Arduino.h"
#include "STM32_CAN.h"
#include "STM32FreeRTOS.h"

#define START_OF_UART_MESSAGE_TOKEN 0xAA
#define END_OF_UART_MESSAGE_TOKEN 0xBB

extern STM32_CAN Can; //Use PB8/PB9 pins for CAN1.

#endif //COMMON_INCLUDE_H
