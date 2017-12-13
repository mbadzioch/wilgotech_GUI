/*
 * komunikacja.c
 *
 *  Created on: 12 wrz 2016
 *      Author: Marcin
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx_conf.h"
#include "komunikacja.h"
#include "uart.h"
#include "delay.h"

char buf[256];
uint8_t uartflag,sendRapFlag=0;

uint32_t param=0;
static void KomInputHandler(void);
static void PCRaport(void);

void KomunikacjaInit(void)
{
	/*
	 * Tutaj odpalamy powitanie, ustawiamy stan początkowy oraz inicjalizujemy timer do taktowania częstości wysyłania raportu
	 */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	/*
	 * Timer 4 - przerwanie co 5s
	 */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 10000;
	TIM_TimeBaseStructure.TIM_Prescaler = 7200;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_Cmd(TIM4, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0f;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0f;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);

	PC_Send("\n\rFingerprint Reader v0.1\n\r");
}


void KomunikacjaMain(void)
{
	KomInputHandler();
	if(sendRapFlag==1){
		sendRapFlag=0;
		PCRaport();
	}
}
static void PCRaport(void)
{

}
static void KomInputHandler(void)
{
	char uBuf[REC_BUF_SIZE],uflag;
	uint8_t ep,isfi;
	uint16_t temp,m;

	if(uartPcBufReadyFlag>0){
		ep=uartPcBufReadyFlag-1;
		m=1;
		temp=0;
		uartPcBufReadyFlag=0;
		PC_Get(uBuf,&uflag);
		for(uint8_t i=0;i<=ep;i++){
			if((uBuf[i] < 'z') && (uBuf[i]>'a')){
				uBuf[i] = uBuf[i]-('a'-'A');
			}
		}
		switch(uBuf[0]){
		case 'S':
			if(uBuf[1] == 'S'){

			}
			else if(uBuf[1] == 'R'){

			}
			else if(uBuf[1]==','){

			}
			else{
				PC_Send("Blad!\n\r");
			}
			break;
		case 'C':

		/*case 'L':
			Reader_SendCmd(CMD_CMOS_LED,1);
			PC_Send(readerrname[Reader_Receive(&param)]);
			sprintf(buf,"  Par: %x\n\r",param);
			PC_Send(buf);
			break;
		case 'O':
			Reader_SendCmd(CMD_OPEN,0);
			PC_Send(readerrname[Reader_Receive(&param)]);
			sprintf(buf,"  Par: %x\n\r",param);
			PC_Send(buf);
			break;
		case 'K':
			Reader_SendCmd(CMD_CMOS_LED,0);
			PC_Send(readerrname[Reader_Receive(&param)]);
			sprintf(buf,"  Par: %x\n\r",param);
			PC_Send(buf);
			break;
			break;
		case 'I':
			Reader_SendCmd(CMD_IS_PRESS_FINGER,0);
			PC_Send(readerrname[Reader_Receive(&param)]);
			sprintf(buf,"  Par: %x\n\r",param);
			PC_Send(buf);
			break;
		case 'C':
			Reader_SendCmd(CMD_CHECK_ENROLLED,5);
			PC_Send(readerrname[Reader_Receive(&param)]);
			sprintf(buf,"  Par: %x\n\r",param);
			PC_Send(buf);
			break;
		case 'F':
			isfi=Reader_IsFinger();
			sprintf(buf,"Palec: %x\n\r",isfi);
			PC_Send(buf);
			break;*/
		default:
			PC_Send("Blad!\n\r");
			break;
		}
	}
}

void TIM4_IRQHandler()
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update) != RESET)
	{
		sendRapFlag=1;
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	}
}










