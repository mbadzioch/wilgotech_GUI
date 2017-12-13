/*
 * display.h
 *
 *  Created on: 8 lis 2016
 *      Author: Marcin
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#define TAB_SIZE 1000
#define PARAM_CNT 8

#define MAX_X 240
#define MAX_Y 128

#define X_SPACING 7

#define CHART_MAX 22
#define CHART_MIN 107
#define CHART_DIF 85

typedef struct{
	uint16_t dayminsec;
	uint16_t humWheat;
	uint16_t flow;
	int16_t  tempWheat;
	int16_t  tempIn;
	int16_t  tempOut;
	uint16_t humAirIn;
	uint16_t humAirOut;
}wilgoData_S;

struct{
	uint8_t water;
	uint16_t flowSet;
	uint16_t flowAlarmYellow;
	uint16_t flowAlarmRed;
	uint16_t intervalHum;
	uint16_t intervalFlow;
	uint8_t font;
}wigloSettings;

void Disp_Init(void);
void Disp_Main(void);
void Disp_DataHandler(uint16_t* bufPointer,uint8_t type);

#endif /* DISPLAY_H_ */
