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

/*
 * 	Rodzaje komend:
 *
 * 		C - cmd - wywołuje akcję urządzenia					-> odpowiada ACK
 * 		S - set - ustawia dany parametr w urządzeniu		-> odpowiada read'em danego parametru
 * 		R - read - pozwala na odczyt danego parametru		-> odpowiada wartością parametru / danymi
 * 		D - data - służy do przesyłu kontenera danych		-> odpowiada ACK
 * 		A - ack - informuje o poprawnym odbiorze komendy
 *
 *
 *
 *	Obsługiwane parametry:
 *
 *		C;MEASURE - wymusza dokonanie pomiaru
 *		S;LED1;50 - ustawia jasność diody LED na 50%
 *		R;A<struct_name> - odczyt struktury w ASCII
 *		R;B<struct_name> - odczyt struktury binarnie
 *		R;ALED1 - odczyt jasności diody LED w ASCII
 *		D;A<struct_name>;DATA - przesył zawartości struktury w ASCII
 */

enum{
	IDLE = 0x01,
	RECEIVING,
	ACTION,
	RESPONDING,
	ACK_WAIT,
	SENDING,
}komState;


static int8_t Kom_Idle(void);
static int8_t Kom_Receiving(void);
static int8_t Kom_Responding(void);
static int8_t Kom_AckWait(void);
static int8_t Kom_Sending(void);
static int8_t Kom_AckSend(errcode_E errcode,encoding_E code);
static int8_t Kom_FrameDecode();

/*
 * Główny proces modułu komunikacji
 */
void Kom_Main()
{
	int8_t ret;

	switch(komState){
	case IDLE:
		if((ret=Kom_Idle()) != 0){
			if(ret<0)Kom_Error(ret);
			else if(ret==1)Kom_Receiving();
			else if(ret==2)Kom_Sending();
		}
		break;
	case RECEIVING:
		if((ret=Kom_Receiving()) != 0){
			if(ret<0)Kom_Error(ret);
			else if(ret==1)Kom_Action();
		}
		break;
	case ACTION:
		if((ret=Kom_Action()) != 0){
			if(ret<0)Kom_Error(ret);
			else if(ret==1)Kom_Responding();
		}
		break;
	case RESPONDING:
		if((ret=Kom_Responding()) != 0){
			if(ret<0)Kom_Error(ret);
			else if(ret==1)Kom_AckWait();
			else if(ret==2)Kom_Idle();
		}
		break;
	case ACK_WAIT:
		if((ret=Kom_AckWait()) != 0){
			if(ret<0)Kom_Error(ret);
			else if(ret==1)Kom_Idle();
			else if(ret==2)Kom_Responding();
			else if(ret==3)Kom_Sending();
		}
		break;
	case SENDING:
		if((ret=Kom_Sending()) != 0){
			if(ret<0)Kom_Error(ret);
			else if(ret==1)Kom_AckWait();
			else if(ret==2)Kom_Idle();
		}
		break;
	default:
		break;
	}
}
/*
 * Jeśli bufor odbiorczy jest niepusty -> przechodzi do odbioru
 *
 * Jeśli jest sygnał "wyślij zapytanie" (np minął zadany czas) -> przechodzi do nadawania
 */
static int8_t Kom_Idle(void)
{
	komState=IDLE;
}
/*
 * Przeprowadza odbiór danych z bufora
 */
static int8_t Kom_Receiving(void)
{
	komState=RECEIVING;
	// Poprawić wszystkie litery na duże!
}
/*
 *	Odpowiada na zapytania
 */
static int8_t Kom_Responding(void)
{
	komState=RESPONDING;
}
/*
 *	Sprawdza czy nadeszło ACK
 */
static int8_t Kom_AckWait(void)
{
	komState=ACK_WAIT;
}
/*
 * Realizuje wysyłanie zapytań
 */
static int8_t Kom_Sending(void)
{
	komState=SENDING;
}

static int8_t Kom_AckSend(errcode_E errcode,encoding_E code)
{
	uint8_t msg[16];

	msg[0]=':';
	msg[1]='A'; 	// ACK
	msg[2]=';';
	if(code == ASCII){
		sprintf(cbuf,"%d",errcode);
		msg[3]=cbuf[0];
	}
	else if(code == BINARY){
		msg[3]=errcode;
	}
	msg[5]=NULL;

	return Uart_Send(PC_UART,msg,6);
}
/*
 * Funkcja odczytująca dane z ramki i wywołująca odpowiednią akcję
 *
 * Przyjmuje:
 * 		Wiadomość do zdekodowania
 *
 * Zwraca:
 *  2 - ack
 * 	1 - sukces
 * 	-1 - błąd komendy lub parametru
 *
 */
static int8_t Kom_FrameDecode()
{
	switch(1){
	case 'C':
		// przeszukuje komendy i zwraca numer odpowiedniej
		break;
	case 'S':
		break;
	case 'R':
		break;
	case 'D':
		// przeszukuje struktury i zwraca numer odpowiedniej
		break;
	case 'A':
		return 2;
		break;
	default:
		return -1;
		break;
	}
}


