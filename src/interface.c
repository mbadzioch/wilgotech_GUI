/*
 * keyboard.c
 *
 *  Created on: 21 sty 2017
 *      Author: Marcin
 */
#include <interface.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx_conf.h"
#include "uart.h"
#include "main_set.h"
#include "interface.h"

volatile uint8_t impEnable=0;

static void IF_KeyInit(void);
static void IF_LedInit(void);
static void IF_OutputInit(void);
static void IF_InputInit(void);
static void IF_TimerInit(void);
static void IF_KeyCheck(void);

void Interface_Init(void)
{
	IF_KeyInit();
	IF_LedInit();
	IF_OutputInit();
	IF_InputInit();
	IF_TimerInit();
}
/*
 * 	Funkcja sprawdzająca przyciśnięcie przycisków i podająca dane do struktury keyboard_S
 *
 * 	Jeśli w dwóch kolejnych obiegach Timera przycisk jest wciśnięty - uznajemy to za wciśnięcie short
 *
 * 	Jeśli przycisk jest trzymany przez 2 sekundy (20 obiegów timera) - uznajemy to za wciśnięcie long
 */

static void IF_KeyCheck(void)
{
	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4)==0){
		if(keyboardState.key1.cnt!=LONG_PRESS)keyboardState.key1.cnt++;
		if(keyboardState.key1.cnt==SHORT_PRESS){
			keyboardState.key1.readFlag=1;
			keyboardState.key1.state=KEY_SHORT;
		}
		if(keyboardState.key1.cnt==LONG_PRESS){
			keyboardState.key1.state=KEY_LONG;
		}
	}
	else{
		keyboardState.key1.cnt=0;
		keyboardState.key1.state=KEY_OFF;
	}
	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==0){
		if(keyboardState.key2.cnt!=LONG_PRESS)keyboardState.key2.cnt++;
		if(keyboardState.key2.cnt==SHORT_PRESS){
			keyboardState.key2.readFlag=1;
			keyboardState.key2.state=KEY_SHORT;
		}
		if(keyboardState.key2.cnt==LONG_PRESS){
			keyboardState.key2.state=KEY_LONG;
		}
	}
	else{
		keyboardState.key2.cnt=0;
		keyboardState.key2.state=KEY_OFF;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)==0){
		if(keyboardState.key3.cnt!=LONG_PRESS)keyboardState.key3.cnt++;
		if(keyboardState.key3.cnt==SHORT_PRESS){
			keyboardState.key3.readFlag=1;
			keyboardState.key3.state=KEY_SHORT;
		}
		if(keyboardState.key3.cnt==LONG_PRESS){
			keyboardState.key3.state=KEY_LONG;
		}
	}
	else{
		keyboardState.key3.cnt=0;
		keyboardState.key3.state=KEY_OFF;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0){
		if(keyboardState.key4.cnt!=LONG_PRESS)keyboardState.key4.cnt++;
		if(keyboardState.key4.cnt==SHORT_PRESS){
			keyboardState.key4.readFlag=1;
			keyboardState.key4.state=KEY_SHORT;
		}
		if(keyboardState.key4.cnt==LONG_PRESS){
			keyboardState.key4.state=KEY_LONG;
		}
	}
	else{
		keyboardState.key4.cnt=0;
		keyboardState.key4.state=KEY_OFF;
	}
#ifdef INTERFACE_DEBUG
	sprintf(cbuf,"K1: %x K2: %x K3: %x K4: %x IMP: %d\n\r",keyboardState.key1.state,keyboardState.key2.state,keyboardState.key3.state,keyboardState.key4.state,keyboardState.imp);
	PC_Debug(cbuf);
#endif

}
/*
 * Key:
 *
 * S1 - PC4
 * S2 - PC5
 * S3 - PB0
 * S4 - PB1

 *
 * IMPA - PB2
 * IMPB - PB10
 */
static void IF_KeyInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	// Clocks

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	// GPIO Config

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// NVIC Config

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource2);

	EXTI_StructInit(&EXTI_InitStructure);
	EXTI_InitStructure.EXTI_Line=EXTI_Line2;
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
/*
 * LED:
 *
 * 	Red - PB14
 * 	Orange - PB13
 * 	Green - PB12
 */
static void IF_LedInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// Clocks

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	// GPIO Config

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
/*
 * Przekaźniki:
 *
 * 	REL1 - PC11
 * 	REL2 - PC10
 * 	ZAWWODY - PA15
 *
 *	OUT_P_MOS1 - PB5
 *	OUT_P_MOS2 - PB4
 *
 *	OUT_N_MOS1 - PD2
 *	OUT_N_MOS2 - PC12
 *
 *	SOUNDER - PB8
 *
 */
static void IF_OutputInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// Clocks

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	// GPIO Config

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB,GPIO_Pin_8);
}
/*
 * Wejścia optoizolowane:
 *
 * 	IN.CZUJNIK 	PB15
 * 	IN.OPTO 	PC6
 * 	S.KRAN		PC7
 */
static void IF_InputInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// Clocks

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	// GPIO Config

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
/*
 * Inicjalizacja timera do obsługi klawiszy i imupsatora
 *
 * 	Okres przerwania: 100ms - 10Hz
 */
static void IF_TimerInit(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 100;
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/10000;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_Cmd(TIM3, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
}
void TIM3_IRQHandler()
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET){
		IF_KeyCheck();
		impEnable=1;
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}
}
/*
 * Przerwanie
 */
void EXTI2_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line2);
		if(impEnable==1){
			impEnable=0;
			if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10) == 0)keyboardState.imp++;
			else keyboardState.imp--;
		}
	}
}

