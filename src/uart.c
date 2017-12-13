/*
 * uart.c
 *
 *  Created on: 12 wrz 2016
 *      Author: Marcin
 */
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx_conf.h"
#include "main_set.h"
#include "uart.h"
#include "delay.h"


//----------------========= HW Init =========------------------------
/*
 *
 * Konfiguracja UART:
 *
 * 	USART6: Komunikacja z czytnikiem
 *
 * 	PA: 11,12
 *
 * 	9600,8,1,0,0
 *
 * 	USART2: Debug
 *
 * 	PA: 2,3
 *
 *  115200,8,1,0,0
 */
void Uart_Init()
{
	USART_InitTypeDef UART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Clocks

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// GPIO Config

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);


	// UART Config

	USART_StructInit(&UART_InitStructure);
	UART_InitStructure.USART_BaudRate = 115200;
	UART_InitStructure.USART_WordLength = USART_WordLength_8b;
	UART_InitStructure.USART_StopBits = USART_StopBits_1;
	UART_InitStructure.USART_Parity = USART_Parity_No;
	UART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	UART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1,&UART_InitStructure);
	USART_Cmd(USART1, ENABLE);

	// NVIC Config

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // Przygotowanie buforów
}
/*
 * 	Nadawanie przez UART
 *
 * 	USARTx - wybrany interfejs
 * 	msg    - ramka do przesłania
 * 	size   - długość ramki wyrażona w bajtach
 *
 * 	Returns:
 * 		1 - OK
 */
int8_t Uart_Send(USART_TypeDef* USARTx,uint8_t* msg,uint16_t* size)
{
	uint16_t cnt=0;

	while(cnt < *size){
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
		USART_SendData(USARTx, *msg++); // Send Char
		cnt++;
	}
	return 1;
}
/*
 *	Funkcja dla wysyłania komunikatów na terminal
 */
void PC_Debug(uint8_t *s)
{
#if UART_DEBUG
	while(*s){
	 // wait until data register is empty
		while(USART_GetFlagStatus(PC_UART, USART_FLAG_TXE) == RESET); // Wait for Empty
		    USART_SendData(PC_UART, *s++); // Send Char
	 }
#endif
}
// -------------=== Przerwania ===--------------------

/*
 * PC:
 */
void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(PC_UART, USART_IT_RXNE) == SET){
		if (USART_GetFlagStatus(PC_UART, USART_FLAG_RXNE)){
			if(uartMain.buf_top < REC_BUF_SIZE){
				uartMain.buf[uartMain.buf_top] = USART_ReceiveData(PC_UART);

				if(uartMain.buf[uartMain.buf_top] == UART_START_CMD){						// Jeśli ramka komendy
					if(uartMain.status == UART_IDLE)uartMain.status=UART_CMD_REC;
				}
				else if(uartMain.buf[uartMain.buf_top] == UART_START_DATA){					// Jeśli ramka danych
					if(uartMain.status == UART_IDLE)uartMain.status=UART_DATA_REC;
				}
				else if(uartMain.buf[uartMain.buf_top] == UART_STOP){						// Jeśli znak stopu i odbieramy komendę
					if(uartMain.status==UART_CMD_REC){
						uartMain.status=UART_IDLE;
						uartMain.frameCnt++;
					}
				}
				if(uartMain.status==UART_DATA_REC){									// Jeśli trwa odbiór danych
					if(uartMain.buf[uartMain.buf_top-1]== UART_DELIMIT && (uartMain.msgLen==0)){	// Jeśli mamy delimiter i msgLen == 0
						uartMain.msgLen = (uint16_t)uartMain.buf[uartMain.buf_top]<<8;		// Pobieramy długość danych w bajtach (HB)
					}
					else if(uartMain.buf[uartMain.buf_top-2]== UART_DELIMIT && (uartMain.msgLen&0x0f==0)){	// Jeśli mamy delimiter i msgLen == 0
						uartMain.msgLen |= uartMain.buf[uartMain.buf_top];					// Pobieramy długość danych w bajtach (LB)
					}
					else{
						if(--uartMain.msgLen == 0){
							uartMain.status=UART_IDLE;
							uartMain.frameCnt++;
						}
					}
				}
				uartMain.buf_top++;
			}
			else{
				uartMain.status=UART_OVERFLOW;
			}
			USART_ClearITPendingBit(PC_UART,USART_IT_RXNE);
		}
	}
}
