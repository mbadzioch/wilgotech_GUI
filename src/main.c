#include <interface.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx_conf.h"
#include <string.h>
#include "main_set.h"
#include "delay.h"
#include "uart.h"
#include "display.h"
#include "unikom.h"
#include "timer.h"
#include "interface.h"
uint8_t tim;

/*
 * 		TODO:
 *
 *		Super-świetna komunikacja
 *		Zapis danych do buforów
 *		Przesył ustawień do modułu pomiarowego
 *
 *		Zapis i odczyt z karty SD

 *
 */

/*void test()
{
	if(uart_err_pc!=0)PC_Debug("JEST ERR\n\r");
	for(uint8_t i=0;i<KOM_BUF_SIZE;i++){
		if(uart_buf_cmd[i].cmd!=0){
			PC_Debug("\n\rCMD: ");
			if(uart_buf_cmd[i].cmd == 'C'){
				PC_Debug("C");
			}
			else{
				PC_Debug("X");
			}
			PC_Debug("\n\rParam: ");
			PC_Debug(uart_buf_cmd[i].param);
			PC_Debug("\n\rValue: ");
			PC_Debug(uart_buf_cmd[i].value);
			PC_Debug("\n\r");
		}
	}
}*/

int main()
{
	DelayInit();
	Timer_Init();
	Uart_Init();
	Disp_Init();
	Interface_Init();

	PC_Debug("Init done!\n");

	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	while (1)
	{
		Disp_Main();
		Kom_Main();
	}
}
