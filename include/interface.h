/*
 * keyboard.h
 *
 *  Created on: 21 sty 2017
 *      Author: Marcin
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <stdio.h>

//#define INTERFACE_DEBUG		// Stan przycisków i impulsatora na PC_Debug

// Długość przyciśnięcia pomnożona przez okres timera - akt. x 10ms
#define SHORT_PRESS 2
#define LONG_PRESS 20

typedef enum{
	KEY_OFF = 0,
	KEY_SHORT,
	KEY_READOUT,
	KEY_LONG
}keyState_E;

typedef struct{
	keyState_E 	state;
	uint8_t 	cnt;
	uint8_t 	readFlag; // Zerowana gdy aplikacja odczyta zmianę klawiatury, ustawiana na 1 gdy pojawi się akcja ze strony użytkownika
}keyState_S;

struct{
	int8_t 		imp;
	keyState_S  key1;
	keyState_S  key2;
	keyState_S  key3;
	keyState_S  key4;
}keyboardState;


void Interface_Init(void);


#endif /* KEYBOARD_H_ */
