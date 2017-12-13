/*
/*
 * unikom.c
 *
 *  Created on: 21 sty 2017
 *      Author: Marcin
 */
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx_conf.h"
#include "main_set.h"
#include "string.h"
#include "delay.h"
#include "uart.h"
#include "unikom.h"

#define KOM_CMD_CNT 3
#define KOM_DATATYPE_CNT 2

const char *komCmdList[]={	"CMD1",
							"CMD2",
							"CMD3"
};

const char *komDataTypeList[]={	"DATA",
								"SETTINGS",
								"ERRORS"
};

komMainS komMain;
/*
 * 	Można dodać z rowerka funkcję PC_Raport która wysyła stan urządzenia co okreslony czas
 */
int8_t Kom_FindStart(void);
int8_t Kom_FindCommand(void);
int8_t Kom_FindDataType(void);
int8_t Kom_ExecuteCmd(void);
int8_t Kom_ProcessData(void);

void Kom_Main(void)
{
	if(uartMain.frameCnt>0){
		if(Kom_FindStart()==1){
			switch(komMain.frameType){
			case CMD:
				if(Kom_FindCommand()==1){
					if(!Kom_ExecuteCmd())komMain.error|=KOM_CMDEXEC_FAIL;
				}
				else{
					komMain.error|=KOM_INVALID_CMD;
				}
				break;
			case DATA:
				if(Kom_FindDataType()==1){
					if(!Kom_ProcessData())komMain.error|=KOM_DATAPROC_FAIL;
				}
				else{
					komMain.error|=KOM_INVALID_DATANAME;
				}
				break;
			default:
				break;
			}
		}
		if(komMain.error != KOM_OK){
			sprintf(cbuf,"KomErr: %d",komMain.error);
			PC_Debug(cbuf);
			// Odpowiedź błędu
		}
		if(--uartMain.frameCnt==0){
			komMain.bufReadPointer=0;
		}
	}
	else if(uartMain.status == UART_OVERFLOW){
		komMain.error|=KOM_BUF_OVF;
		uartMain.buf_top=0;
		uartMain.frameCnt=0;
		uartMain.status=UART_IDLE;
	}
}
/*
 *  Szuka znaku poczatku ramki
 *
 *  Zwraca:
 *  		1 - sukces
 *  		0 - failure
 */
int8_t Kom_FindStart(void)
{
	uint16_t i,k=0;
	for(i=komMain.bufReadPointer;i<REC_BUF_SIZE;i++){
		if((uartMain.buf[i] == UART_START_CMD) || (uartMain.buf[i] == UART_START_DATA)){// Najpierw szukamy znaku początku ramki
			switch(uartMain.buf[i]){
			case UART_START_CMD:
				komMain.frameType=CMD;
				break;
			case UART_START_DATA:
				komMain.frameType=DATA;
				break;
			default:
				break;
			}
			uartMain.buf[i]=0;	// Aby nigdy nie odczytać tej ramki drugi raz
			for(i=i+1;k<16;i++){
				if(uartMain.buf[i]==UART_DELIMIT || uartMain.buf[i]==UART_STOP){
					komMain.bufReadPointer=i+1;	// Ustawiamy na dane
					break;
				}
				if((uartMain.buf[i] < 'z') && (uartMain.buf[i]>'a')){	// Zamiana małych liter na duże
					uartMain.buf[i] = uartMain.buf[i]-('a'-'A');
				}
				komMain.nameString[k]=uartMain.buf[i];
				k++;
			}
			return 1;
		}
	}
	// Nie znaleziono znaku startu
	komMain.bufReadPointer=0;
	komMain.error |= KOM_INVALID_FRAME;
	return 0;
}
/*
 *	Porównywanie nazw komend i nazw danych
 */
int8_t Kom_FindCommand(void)
{
	uint8_t i;
	for(i=0;i<KOM_CMD_CNT;i++){
		if(strcmp(komMain.nameString,komCmdList[i])==0){
			komMain.listPos=i;
			return 1;
		}
	}
	return 0;
}
int8_t Kom_FindDataType(void)
{
	uint8_t i;
	for(i=0;i<KOM_DATATYPE_CNT;i++){
		if(strcmp(komMain.nameString,komDataTypeList[i])==0){
			komMain.listPos=i;
			return 1;
		}
	}
	return 0;
}
int8_t Kom_ExecuteCmd(void)
{
	switch(komMain.listPos){
	case 0:
		PC_Debug("ELO\n\r");
		break;
	case 1:
		break;
	}
	return 1;
}
int8_t Kom_ProcessData(void)
{
	//Disp_DataHandler(&komMain.bufReadPointer,komMain.listPos);
	// Może CMD->że gotowy na przyjęcie danych
	return 1;
}
