/*
 * uart.h
 *
 *  Created on: 12 wrz 2016
 *      Author: Marcin
 */

#ifndef UART_H_
#define UART_H_

#include "unikom.h"

// Frame symbols

#define UART_START_CMD ':'
#define UART_START_DATA '$'
#define UART_DELIMIT ' '
#define UART_STOP '\r'


// Interfaces
#define PC_UART USART1

// Receive buffers

#define REC_BUF_SIZE 1024

typedef enum{
	UART_IDLE = 0x01,
	UART_CMD_REC,
	UART_DATA_REC,
	UART_OVERFLOW,
	UART_ERROR
}uart_status_E;



typedef struct{
	uint8_t  buf[REC_BUF_SIZE];
	uint16_t buf_top;
	uint16_t msgLen;
	uint8_t  frameCnt;
	uart_status_E status;
}uart_main_S;

uart_main_S uartMain;

void Uart_Init(void);
int8_t Uart_Send(USART_TypeDef* UARTx,uint8_t* msg,uint16_t* size);
void PC_Debug(uint8_t *s);
void USART1_IRQHandler(void);


#endif /* UART_H_ */
