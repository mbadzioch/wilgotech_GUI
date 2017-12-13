/*

 * display.c
 *
 *  Created on: 8 lis 2016
 *      Author: Marcin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stm32f4xx_conf.h"
#include "main_set.h"
#include "delay.h"
#include "uart.h"
#include "u8g.h"
#include "u8g_arm.h"
#include "timer.h"
#include "display.h"
#include "interface.h"


typedef enum{
	LOGO,
	PARAMS,
	SETTINGS,
	CHART,
	ERRSCREEN
}dispMenu_E;

typedef enum{
	FONT_SMALL,
	FONT_MEDIUM,
	FONT_LARGE
}dispFont_E;




struct{
	dispMenu_E menu;
	dispFont_E font;
	uint8_t fontHeight;
	uint8_t fontWidth;
	uint8_t lineHeight;
	uint8_t timLogo;
	uint8_t timRefresh;
}dispMain;

struct{
	uint16_t 	oldestItem;	// Punkt wykresu najbardziej na lewo
	uint16_t 	newestItem;	// Punkt wykresu najbardziej na prawo
	uint8_t 	param;		// Aktualnie wyświetlany parametr
	int16_t 	min;
	int16_t 	max;

}dispChart;

struct{
	int16_t		dataTab[8][TAB_SIZE]; // Bufor kołowy na rekordy w pamięci:
	int16_t		actDataTab[8];			// Aktualne parametry
	uint16_t	dataTopPointer; // Który element tablicy z danymi jest najświeższy
	int16_t		firstColumn;	// Jaki jest pierwszy parametr wyświetlany po dacie; minVal = 0
	int16_t		firstLine;		// Który wiersz jest wyświetlany jako pierwszy; minVal = 0
	int16_t		lineScale;		// Skalowanie - co ile próbek mają być wyświetlane linie
	uint8_t		lineScaleChange;	// Czy skalujemy czy przewijamy wiersze
	uint8_t		impState;			// Czy kręcenie impulsatora zmienia kolumny czy wiersze
	uint8_t 	impBlockColFlag; 	// Ustawiana gdy osiągniemy max kolumn
	uint8_t 	impBlockLineFlag; 	// Ustawiana gdy osiągniemy max wierszy
}dispParams;

struct{
	int16_t valueTab[7];	// Tablica z ustawieniami
	uint8_t fontHeight;
	uint8_t lineHeight;
	int8_t  xpos; // Pozycja kursora
	int8_t  ypos; // Pozycja kursora
	uint8_t impMul; // Mnożnik impulsatora
}dispSettings;

const char *paramNames[]={"Data","Wilg","Prze","Tz","Ti","Wi","To","Wo"};
const char *dayNames[]={"Pn","Wt","Sr","Cz","Pt","Sb","Nd"};
const char *setParams[]={"Nawilzanie","Przeplyw ustawiony","Alarm zolty","Alarm czerwony","Interwal wilgotnosc","Interwal przeplyw","Czcionka"};
const char *setFontNames[]={"mala","sred","duza"};
const char *setParamUnit[]={"ml","kg","\xb1kg","\xb1kg","min","sek"};
//uint8_t *paramNames={"Data","Wil","Przep","Tz"};
const char *chartParNames[]={"","Wilgotnosc zboza [%]","Przeplyw [kg]","Temperatura zboza [\xb0 C]","Temperatura wewnatrz [\xb0 C]","Wilgotnosc wewnatrz [%]","Temperatura na zewnatrz [\xb0 C]","Wilgotnosc na zewnatrz [%]"};

const uint16_t setValLimits[7][2]={{0,100},{0,9000},{0,999},{0,999},{15,120},{10,999},{0,2}};

u8g_t u8g;



static void Draw_Logo(void);
static void Disp_Change(void);
static void Disp_DrawParams(void);
static void Disp_DrawOther(void);
static void Disp_DrawParCol(void);
static void Disp_SetFont(void);
static void Disp_DataConv(uint8_t column,char *convbuf, int16_t *val,uint8_t dest);
static void Disp_GetActParam(int16_t *tab,uint16_t pos,int16_t *val);
static void Disp_GetColWidth(uint8_t column,uint16_t lines,uint8_t *width);
static void Disp_DrawSettings(void);
static void Disp_DrawSettingsNames(void);
static void Disp_DrawSettingsValues(void);
static void Disp_SaveSettings(void);
static void Disp_DrawChart(void);
static void Disp_DrawChartElements(void);
static void Disp_DrawChartGuidelines(void);
static void Disp_DrawChartData(void);

void Disp_TestData(void)
{
	uint16_t k=0,l=0;
	int8_t wm=1,pm=10,tm=1;
	uint16_t dat=0;
	int16_t wilg=100,przep=1900,temzbo=180;

	dispParams.actDataTab[0]=00745;
	dispParams.actDataTab[1]=123;
	dispParams.actDataTab[2]=1243;
	dispParams.actDataTab[3]=120;
	dispParams.actDataTab[4]=140;
	dispParams.actDataTab[5]=40;
	dispParams.actDataTab[6]=25;
	dispParams.actDataTab[7]=70;
	dispParams.dataTab[0][20]=32300;
	dispParams.dataTab[0][19]=32245;
	dispParams.dataTab[0][18]=32230;
	dispParams.dataTab[0][17]=32215;
	dispParams.dataTab[0][16]=32200;
	dispParams.dataTab[0][15]=32145;
	dispParams.dataTab[0][14]=32130;
	dispParams.dataTab[0][13]=32115;
	dispParams.dataTab[0][12]=32100;
	dispParams.dataTopPointer=999;


	for(uint16_t i=0;i<1000;i++){
		dispParams.dataTab[0][i]=(int16_t)dat;
		dat+=15;
		if(dat%100 == 60)dat += 40;
		if((dat/100)%100 == 24) dat += 7600;
		if(dat>=60000)dat=0;
		dispParams.dataTab[1][i]=wilg;
		wilg+=wm;
		if(wilg>150)wm=-2;
		if(wilg<100)wm=3;
		dispParams.dataTab[2][i]=przep;
		przep+=pm;
		if(przep>2500)pm=-20;
		if(przep<1500)pm=30;
		dispParams.dataTab[3][i]=temzbo;
		temzbo+=tm;
		if(temzbo>250)tm=-2;
		if(temzbo<150)tm=3;
	  for(uint8_t j=4;j<8;j++){
		dispParams.dataTab[j][i]=dispParams.actDataTab[j];
	  }
	}

	/*for(uint16_t i=0;i<1000;i++){
		dispParams.dataTab[0][i]=dispParams.actDataTab[0]+(15*k);
		k++;

		if(k==4){
			k=0;
			dispParams.actDataTab[0]+=100;
		}
		if(dispParams.actDataTab[0]%1000 >= 2400){
			l++;
			dispParams.actDataTab[0]=10000*l;
		}
		for(uint8_t j=1;j<8;j++){
			dispParams.dataTab[j][i]=dispParams.actDataTab[j];
		}
	}*/
}
void Disp_Init(void)
{
	u8g_InitComFn(&u8g, &u8g_dev_uc1608_240x128_2x_hw_spi, u8g_com_hw_spi_fn);
	u8g_SetDefaultForegroundColor(&u8g);
	dispMain.font=FONT_LARGE;
	Disp_SetFont();

	dispMain.menu=CHART;
	dispParams.lineScale=1;

	dispSettings.valueTab[6]=2;
	dispSettings.impMul=1;

	dispChart.oldestItem=100;
	dispChart.param=1;

	Draw_Logo();
	Timer_Register(&dispMain.timLogo,1000,AUTOSTOP);
	Timer_Register(&dispMain.timRefresh,100,AUTORESET);
	Disp_TestData();
}

/*
 * 	Główna funkcja modułu
 */
void Disp_Main(void)
{
	Disp_Change();
	if(Timer_Check(&dispMain.timRefresh)){
		switch(dispMain.menu){
		case LOGO:
			if(Timer_Check(&dispMain.timLogo)==1)dispMain.menu=CHART;
			break;
		case PARAMS:
			Disp_DrawParams();
			break;
		case SETTINGS:
			Disp_DrawSettings();
			break;
		case CHART:
			Disp_DrawChart();
			break;
		case ERRSCREEN:

			break;
		default:
			break;
		}
	}
}

/*
 * 	Funkcja wprowadzająca reakcję na naciśnięcie przycisków / impulsatora
 *
 * 	Zwraca:
 * 		0 - brak zmian
 * 		1 - zmiana
 */
static void Disp_Change(void)
{
	//Disp_SetFont();

	switch(dispMain.menu){
	case PARAMS:
		if(keyboardState.key1.state==KEY_SHORT){
			keyboardState.key1.state=KEY_READOUT;
			dispMain.menu=SETTINGS;
		}
		if(keyboardState.key2.state==KEY_SHORT){
			keyboardState.key2.state=KEY_READOUT;
			dispParams.impState = 0;
		}
		if(keyboardState.key3.state==KEY_SHORT){
			keyboardState.key3.state=KEY_READOUT;
			dispParams.impState = 1;
		}
		if(keyboardState.key4.state==KEY_SHORT){
			keyboardState.key4.state=KEY_READOUT;
			dispParams.lineScaleChange ^= 1;
		}
		if(dispParams.impState == 1){
			if((dispParams.impBlockLineFlag == 0) || (keyboardState.imp<0)){
				if(dispParams.lineScaleChange == 1)dispParams.lineScale+=keyboardState.imp;
				else dispParams.firstLine+=keyboardState.imp;
				if(dispParams.lineScale < 1)dispParams.lineScale=1;
			}
		}
		else if((dispParams.impBlockColFlag == 0) || (keyboardState.imp<0))dispParams.firstColumn+=keyboardState.imp;

		if(dispParams.firstLine<0)dispParams.firstLine=0;
		if(dispParams.firstColumn<0)dispParams.firstColumn=0;
		break;
	case SETTINGS:
		if(keyboardState.key1.state==KEY_SHORT){
			keyboardState.key1.state=KEY_READOUT;
			Disp_SaveSettings();
			dispMain.menu=CHART;
		}
		if(keyboardState.key2.state==KEY_SHORT){
			keyboardState.key2.state=KEY_READOUT;

			dispSettings.xpos--;
			if(dispSettings.xpos < 0)dispSettings.xpos=0;
			if(dispSettings.xpos > 6)dispSettings.xpos=6;
		}
		if(keyboardState.key3.state==KEY_SHORT){
			keyboardState.key3.state=KEY_READOUT;
			dispSettings.xpos++;
			if(dispSettings.xpos < 0)dispSettings.xpos=0;
			if(dispSettings.xpos > 6)dispSettings.xpos=6;
		}
		if(keyboardState.key4.state==KEY_SHORT){
			keyboardState.key4.state=KEY_READOUT;
			switch(dispSettings.impMul){
			case 1:
				dispSettings.impMul=10;
				break;
			case 10:
				dispSettings.impMul=100;
				break;
			case 100:
				dispSettings.impMul=1;
				break;
			default:
				dispSettings.impMul=1;
				break;
			}
		}

		dispSettings.valueTab[dispSettings.xpos]+=(keyboardState.imp*dispSettings.impMul);
		if(dispSettings.valueTab[dispSettings.xpos] < setValLimits[dispSettings.xpos][0]){
			dispSettings.valueTab[dispSettings.xpos] = setValLimits[dispSettings.xpos][0];
		}
		else if(dispSettings.valueTab[dispSettings.xpos] > setValLimits[dispSettings.xpos][1]){
			dispSettings.valueTab[dispSettings.xpos] = setValLimits[dispSettings.xpos][1];
		}
		break;
	case CHART:
		if(keyboardState.key1.state==KEY_SHORT){
			keyboardState.key1.state=KEY_READOUT;
			Disp_SaveSettings();
			dispMain.menu=PARAMS;
		}
		if(keyboardState.key2.state==KEY_SHORT){
			keyboardState.key2.state=KEY_READOUT;
			if(dispChart.param>1)dispChart.param--;
		}
		if(keyboardState.key3.state==KEY_SHORT){
			keyboardState.key3.state=KEY_READOUT;
			if(dispChart.param<7)dispChart.param++;
		}
		if((dispChart.oldestItem>10 || keyboardState.imp < 0)&&(dispChart.oldestItem<990 || keyboardState.imp > 0))dispChart.oldestItem-=(keyboardState.imp*4);
		break;
	default:
		break;
	}
	keyboardState.imp=0;
}
static void Disp_SaveSettings(void)
{
	dispMain.font=dispSettings.valueTab[6];
	Disp_SetFont();

	wigloSettings.water=dispSettings.valueTab[0];
	wigloSettings.flowSet=dispSettings.valueTab[1];
	wigloSettings.flowAlarmYellow=dispSettings.valueTab[2];
	wigloSettings.flowAlarmRed=dispSettings.valueTab[3];
	wigloSettings.intervalHum=dispSettings.valueTab[4];
	wigloSettings.intervalFlow=dispSettings.valueTab[5];
	wigloSettings.font=dispSettings.valueTab[6];
}
//---------------------------------================ MENU DANYCH ==============-----------------------------------
static void Disp_DrawParams(void)
{
	u8g_FirstPage(&u8g);
	do{
		Disp_DrawOther();
		Disp_DrawParCol();
	}while(u8g_NextPage(&u8g));
}
static void Disp_DrawOther(void)
{
	u8g_DrawLine(&u8g,0,dispMain.lineHeight,MAX_X,dispMain.lineHeight);
}
/*
 * Rysowanie kolumn
 *
 *
 */
static void Disp_DrawParCol(void)
{
	uint8_t x_pos=0,y_pos=0,colWidth=0,nextColWidth=0,column=0,coltemp;
	int16_t temp;
	uint16_t lineCnt=0;
	char convbuf[16];

	while(MAX_X-x_pos >= nextColWidth){

		y_pos = dispMain.fontHeight;
		lineCnt=0;
		colWidth=0;
		while(y_pos<MAX_Y){
			if(lineCnt==0){
				if(column==0){
					if(dispParams.impState==0)u8g_DrawStr(&u8g,x_pos,y_pos,"Data    >");
					else{
						u8g_DrawStr(&u8g,x_pos,y_pos,"Data   ");
						if(dispParams.lineScaleChange==0)u8g_DrawStr180(&u8g,u8g_GetStrPixelWidth(&u8g,"Data    ")+5,0,"\x5e");
						else u8g_DrawStr180(&u8g,u8g_GetStrPixelWidth(&u8g,"Data    ")+5,0,"\x5e\x5e");
					}
				}
				else{
					u8g_DrawStr(&u8g,x_pos,y_pos,*(paramNames+column));
				}

				y_pos+=3;
				lineCnt++;
			}
			else if(lineCnt == 1){
				Disp_DataConv(column,convbuf,&dispParams.actDataTab[column],1);
				u8g_DrawStr(&u8g,x_pos,y_pos,convbuf);
				if((coltemp = u8g_GetStrWidth(&u8g,convbuf)) > colWidth)colWidth=coltemp;
				lineCnt=lineCnt+dispParams.lineScale;
			}
			else{
				Disp_GetActParam(dispParams.dataTab[column],(lineCnt-2)+dispParams.firstLine,&temp);
				Disp_DataConv(column,convbuf,&temp,1);
				u8g_DrawStr(&u8g,x_pos,y_pos,convbuf);
				if((coltemp = u8g_GetStrWidth(&u8g,convbuf)) > colWidth)colWidth=coltemp;
				lineCnt=lineCnt+dispParams.lineScale;
				if((lineCnt-2)+dispParams.firstLine >= TAB_SIZE){
					dispParams.impBlockLineFlag=1;
					break;
				}
				else{
					dispParams.impBlockLineFlag=0;
				}
			}
			y_pos += dispMain.lineHeight;
		}
		x_pos += colWidth+X_SPACING;
		// Rysujemy linię
		u8g_DrawLine(&u8g,x_pos,0,x_pos,MAX_Y);
		x_pos += X_SPACING;
		if(column==0)column+= dispParams.firstColumn;
		column++;
		if(column >= PARAM_CNT){
			dispParams.impBlockColFlag=1;
			break;
		}
		else{
			dispParams.impBlockColFlag=0;
		}
		Disp_GetColWidth(column,lineCnt-dispParams.lineScale,&nextColWidth);
	}
}
static void Disp_GetColWidth(uint8_t column,uint16_t lines,uint8_t *width)
{
	int16_t i=dispParams.dataTopPointer;
	int16_t temp=0;
	char tempbuf[24];

	while(lines>0){
		if(dispParams.dataTab[column][i] > temp)temp=dispParams.dataTab[column][i];
		i--;
		if(i<0)i=TAB_SIZE-1;
		lines--;
	}
	Disp_DataConv(column,tempbuf,&temp,1);

	*width = u8g_GetStrWidth(&u8g,tempbuf);
}
static void Disp_GetActParam(int16_t *tab,uint16_t pos,int16_t *val)
{
	if(dispParams.dataTopPointer >= pos){
		*val = *(tab+dispParams.dataTopPointer-pos);
	}
	else{
		*val = *(tab+(TAB_SIZE-(pos-dispParams.dataTopPointer)));
	}
}
static void Disp_DataConv(uint8_t column,char *convbuf, int16_t *val,uint8_t dest)
{
	if(dest==1){	// Wyświetlanie parametrów w tabeli
	switch(column){
	case 0:	// DATA
		strcpy(convbuf,*(dayNames+(((uint16_t)*val)/10000)));
		sprintf((convbuf+2)," %.2d:%.2d",(((uint16_t)*val)/100)%100,((uint16_t)*val)%100);
		break;
	case 1: // Wilgotnosc
		sprintf(convbuf,"%d.%.1d",*val/10,*val%10);
		break;
	case 2: // Przeplyw
		sprintf(convbuf,"%d",*val);
		break;
	case 3: // Temp zboza
		sprintf(convbuf,"%d.%.1d\xb0""C",*val/10,*val%10);
		break;
	case 4: // Temp pow Wew
		sprintf(convbuf,"%d.%.1d\xb0",*val/10,*val%10);
		break;
	case 5: // Wilg pow wew
		sprintf(convbuf,"%d",*val);
		break;
	case 6: // Temp pow zew
		sprintf(convbuf,"%d.%.1d\xb0",*val/10,*val%10);
		break;
	case 7: // Wilg pow zew
		sprintf(convbuf,"%d",*val);
		break;
	default:
		break;
	}
	}
	else if(dest ==2 ){	// Wyświetlanie na wykresie
		switch(column){
		case 0:	// DATA
			strcpy(convbuf,*(dayNames+(((uint16_t)*val)/10000)));
			sprintf((convbuf+2)," %.2d:%.2d",(((uint16_t)*val)/100)%100,((uint16_t)*val)%100);
			break;
		case 1: // Wilgotnosc
			sprintf(convbuf,"%d.%.1d",*val/10,*val%10);
			strcat(convbuf,"%");
			break;
		case 2: // Przeplyw
			sprintf(convbuf,"%d",*val);
			break;
		case 3: // Temp zboza
			sprintf(convbuf,"%d.%.1d\xb0",*val/10,*val%10);
			break;
		case 4: // Temp pow Wew
			sprintf(convbuf,"%d.%.1d\xb0",*val/10,*val%10);
			break;
		case 5: // Wilg pow wew
			sprintf(convbuf,"%d",*val);
			strcat(convbuf,"%");
			break;
		case 6: // Temp pow zew
			sprintf(convbuf,"%d.%.1d\xb0",*val/10,*val%10);
			break;
		case 7: // Wilg pow zew
			sprintf(convbuf,"%d",*val);
			strcat(convbuf,"%");
			break;
		default:
			break;
		}
	}
}
//-----------------------================ MENU USTAWIEN ===============-----------------------------
static void Disp_DrawSettings(void)
{
	if(dispMain.font != FONT_LARGE){
		u8g_SetFont(&u8g, u8g_font_helvB12);
	}
	dispSettings.fontHeight=u8g_GetFontCapitalAHeight(&u8g);
	dispSettings.lineHeight = 1.3*dispSettings.fontHeight;

	u8g_FirstPage(&u8g);
	do{
		Disp_DrawSettingsNames();
		Disp_DrawSettingsValues();
	}while(u8g_NextPage(&u8g));
}

static void Disp_DrawSettingsNames(void)
{
	uint8_t ypos=0;
	ypos=dispSettings.fontHeight;

	u8g_DrawStr(&u8g,40,ypos,"Ustawienia");

	u8g_DrawLine(&u8g,0,dispSettings.lineHeight-1,MAX_X,dispSettings.lineHeight-1);

	for(uint8_t i=0;i<7;i++){
		ypos+=(dispSettings.lineHeight+1);
		if((i==dispSettings.xpos)){
			u8g_DrawStr(&u8g,0,ypos,"\xbb");
		}
		u8g_DrawStr(&u8g,10,ypos,*(setParams+i));
		u8g_DrawLine(&u8g,0,ypos+1,MAX_X,ypos+1);
	}
	u8g_DrawLine(&u8g,170,0,170,MAX_Y-3);
}
static void Disp_DrawSettingsValues(void)
{
	uint8_t ypos=0;
	char convbuf[10];

	ypos=dispSettings.fontHeight;

	sprintf(convbuf,"i: x%d",dispSettings.impMul);
	u8g_DrawStr(&u8g,190,ypos,convbuf);

	for(uint8_t i=0;i<7;i++){
		ypos+=(dispSettings.lineHeight+1);
		if(i==6){
			u8g_DrawStr(&u8g,190,ypos,*(setFontNames+dispSettings.valueTab[i]));
		}
		else{
			sprintf(convbuf,"%d",dispSettings.valueTab[i]);
			u8g_DrawStr(&u8g,175,ypos,convbuf);
			if(i<2)u8g_DrawStr(&u8g,222,ypos,*(setParamUnit+i));
			else u8g_DrawStr(&u8g,212,ypos,*(setParamUnit+i));
		}
	}

}
//-------------------==================== Wykresy ===================---------------------------
static void Disp_DrawChart(void)
{
	if(dispMain.font != FONT_MEDIUM){
		u8g_SetFont(&u8g, u8g_font_helvB10);
	}
	dispSettings.fontHeight=u8g_GetFontCapitalAHeight(&u8g);
	dispSettings.lineHeight = 1.3*dispSettings.fontHeight;


	u8g_FirstPage(&u8g);
	do{
		Disp_DrawChartElements();
		Disp_DrawChartGuidelines();
		Disp_DrawChartData();
	}while(u8g_NextPage(&u8g));
}

static void Disp_DrawChartElements(void)
{
	char convbuf[16];
	int16_t temp;

	u8g_DrawLine(&u8g,0,dispMain.lineHeight,MAX_X,dispMain.lineHeight);
	u8g_DrawLine(&u8g,0,MAX_Y-dispMain.lineHeight,MAX_X,MAX_Y-dispMain.lineHeight);
	u8g_DrawLine(&u8g,40,dispMain.lineHeight,40,MAX_Y);

	u8g_DrawStr(&u8g,0,dispSettings.fontHeight,*(chartParNames+dispChart.param));

	//Disp_GetActParam(dispParams.dataTab[0],dispChart.newestItem,&temp);
	//Disp_DataConv(0,convbuf,&temp,2);
	u8g_DrawStr(&u8g,200,MAX_Y,"Teraz");

	Disp_GetActParam(dispParams.dataTab[0],dispChart.oldestItem,&temp);
	Disp_DataConv(0,convbuf,&temp,2);
	u8g_DrawStr(&u8g,45,MAX_Y,convbuf);
}
static void Disp_DrawChartGuidelines(void)
{
	int16_t temp=0,avg=0;
	char numbuf[10];

	Disp_GetActParam(dispParams.dataTab[dispChart.param],0,&temp);
	dispChart.min = temp;
	dispChart.max = temp;

	// Sprawdzac od najstarszego do najnowszego!
	for(uint16_t i=0;i<dispChart.oldestItem;i++){
		Disp_GetActParam(dispParams.dataTab[dispChart.param],i,&temp);
		if(temp < dispChart.min)dispChart.min=temp;
		if(temp > dispChart.max)dispChart.max=temp;
	}
	avg = (dispChart.min+dispChart.max)/2;

	Disp_DataConv(dispChart.param,numbuf,&dispChart.min,2);
	u8g_DrawStr(&u8g,0,MAX_Y-dispMain.lineHeight,numbuf);

	for(uint8_t l=40;l<MAX_X;l+=20){
		u8g_DrawLine(&u8g,l,CHART_MIN,l+5,CHART_MIN);
	}

	Disp_DataConv(dispChart.param,numbuf,&avg,2);
	u8g_DrawStr(&u8g,0,MAX_Y/2+7,numbuf);

	for(uint8_t l=40;l<MAX_X;l+=20){
		u8g_DrawLine(&u8g,l,MAX_Y/2,l+5,MAX_Y/2);
	}

	Disp_DataConv(dispChart.param,numbuf,&dispChart.max,2);
	u8g_DrawStr(&u8g,0,2*dispMain.lineHeight,numbuf);

	for(uint8_t l=40;l<MAX_X;l+=20){
		u8g_DrawLine(&u8g,l,CHART_MAX,l+5,CHART_MAX);
	}
}
static void Disp_DrawChartData(void)
{
	int16_t tempa,tempb,i;
	double diffx,actx;

	diffx = 200.0/(double)dispChart.oldestItem;
	if(diffx == 0)diffx=1;
	actx=MAX_X;

	// rysować od najnowszego do najstarszego
	Disp_GetActParam(dispParams.dataTab[dispChart.param],0,&tempa);
	tempa = CHART_MAX+((double)CHART_DIF*((double)(tempa-dispChart.min)/(double)(dispChart.max-dispChart.min)));

	for(i=1;i<dispChart.oldestItem;i++){

		Disp_GetActParam(dispParams.dataTab[dispChart.param],i,&tempb);
		tempb = CHART_MAX+((double)CHART_DIF*((double)(tempb-dispChart.min)/(double)(dispChart.max-dispChart.min)));
		u8g_DrawLine(&u8g,round(actx-diffx),tempb,round(actx),tempa);
		tempa = tempb;

		if(actx>40){
			actx-=diffx;
		}
		else {
			break;
		}
	}
}
//----------------------================= Inne ======================---------------------------
static void Disp_SetFont(void)
{
	switch(dispMain.font){
	case FONT_SMALL:
		u8g_SetFont(&u8g, u8g_font_helvB08);
		break;
	case FONT_MEDIUM:
		u8g_SetFont(&u8g, u8g_font_helvB10);
		break;
	case FONT_LARGE:
		u8g_SetFont(&u8g, u8g_font_helvB12);
		break;
	default:
		u8g_SetFont(&u8g, u8g_font_helvB12);
		break;
	}
	dispMain.fontHeight=u8g_GetFontCapitalAHeight(&u8g);
	dispMain.fontWidth=u8g_GetFontBBXWidth(&u8g);

	dispMain.lineHeight = 1.3*dispMain.fontHeight;
}
static void Draw_Logo(void)
{
	const char logo [] = {
			0x00, 0x00, 0x00, 0x00, 0x3F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0x00, 0x03, 0xFF, 0xFF,
			0x80, 0x00, 0x3F, 0xFF, 0x0F, 0xFF, 0xC0, 0x1F, 0x00, 0x01, 0xF0, 0x00, 0x7F, 0xFE, 0x07, 0xFF,
			0xC0, 0xF0, 0x00, 0x00, 0x3C, 0x00, 0xFF, 0xFC, 0x03, 0xFF, 0xE1, 0x80, 0x00, 0x00, 0x07, 0x01,
			0xFF, 0xF8, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0xC3, 0xFF, 0xF0, 0x00, 0xFF, 0xFC, 0x00,
			0x00, 0x00, 0x00, 0xE7, 0xFF, 0xE0, 0x00, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xC0,
			0x00, 0x3F, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0x80, 0x00, 0x1F, 0xFF, 0x00, 0x00, 0x00,
			0x00, 0x3F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x7F, 0xFE, 0x00, 0x00, 0x0F,
			0xFF, 0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0x00, 0x00, 0x0F, 0xFF, 0xE0, 0x00, 0x00, 0x01, 0xFF,
			0xF8, 0x00, 0x00, 0x19, 0xFF, 0xF0, 0x00, 0x00, 0x03, 0xFF, 0xF8, 0x00, 0x00, 0x31, 0xFF, 0xF0,
			0x00, 0x00, 0x07, 0xFF, 0xF8, 0x00, 0x00, 0x60, 0xFF, 0xF8, 0x00, 0x00, 0x1F, 0xFF, 0xD8, 0x00,
			0x00, 0xC0, 0x7F, 0xFC, 0x00, 0x00, 0x1F, 0xFF, 0x8C, 0x00, 0x00, 0xC0, 0x3F, 0xFE, 0x00, 0x00,
			0x3F, 0xFF, 0x06, 0x00, 0x01, 0x80, 0x1F, 0xFF, 0x00, 0x00, 0x7F, 0xFE, 0x06, 0x00, 0x01, 0x00,
			0x0F, 0xFF, 0x80, 0xC1, 0xFF, 0xFC, 0x03, 0x00, 0x03, 0x00, 0x07, 0xFF, 0xC1, 0xE1, 0xFF, 0xF8,
			0x01, 0x00, 0x06, 0x00, 0x03, 0xFF, 0xE1, 0xFF, 0xFF, 0xF0, 0x01, 0x80, 0x06, 0x00, 0x01, 0xFF,
			0xC3, 0xFF, 0xFF, 0xE0, 0x00, 0xC0, 0x0C, 0x00, 0x01, 0xFF, 0xC7, 0xFF, 0xFF, 0xC0, 0x00, 0x40,
			0x0C, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x60, 0x18, 0x00, 0x00, 0x7F, 0xFF, 0xFF,
			0xFF, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x3F, 0xFF, 0xEF, 0xFE, 0x00, 0x00, 0x20, 0x18, 0x00,
			0x00, 0x1F, 0xFF, 0xCF, 0xFC, 0x00, 0x00, 0x30, 0x10, 0x00, 0x00, 0x0F, 0xCF, 0x9F, 0xF8, 0x00,
			0x00, 0x30, 0x30, 0x00, 0x00, 0x07, 0xC7, 0xBF, 0xE0, 0x00, 0x00, 0x10, 0x30, 0x00, 0x00, 0x07,
			0xE3, 0x3F, 0xC0, 0x00, 0x00, 0x18, 0x70, 0x00, 0x00, 0x03, 0xFA, 0x7F, 0xC0, 0x00, 0x00, 0x08,
			0x60, 0x00, 0x00, 0x03, 0xFC, 0x3F, 0xE0, 0x00, 0x00, 0x08, 0x60, 0x00, 0x00, 0x07, 0xFC, 0xBF,
			0xF0, 0x00, 0x00, 0x08, 0x60, 0x00, 0x00, 0x0F, 0xF8, 0x3F, 0xF0, 0x00, 0x00, 0x08, 0x60, 0x00,
			0x00, 0x0F, 0xF3, 0xDF, 0xF8, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x00, 0x1F, 0xE7, 0xCF, 0xFC, 0x00,
			0x00, 0x0C, 0x60, 0x00, 0x00, 0x3F, 0xEF, 0xE7, 0xFC, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x01, 0xFF,
			0xCF, 0xFF, 0xFE, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x03, 0xFF, 0xDF, 0xFF, 0xFF, 0x00, 0x00, 0x0C,
			0x60, 0x00, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x0C, 0x60, 0x00, 0x07, 0xFF, 0xC0, 0xFF,
			0xFF, 0x80, 0x00, 0x0C, 0x60, 0x00, 0x0F, 0xFF, 0xC1, 0xFF, 0xFF, 0xC0, 0x00, 0x0C, 0x60, 0x00,
			0x1F, 0xFF, 0xE0, 0x7F, 0xFF, 0xE0, 0x00, 0x0C, 0x60, 0x00, 0x1F, 0xFF, 0xC0, 0x3F, 0xFF, 0xE0,
			0x00, 0x0C, 0x60, 0x00, 0x3F, 0xFF, 0xFC, 0x1F, 0xFF, 0xF0, 0x00, 0x0C, 0x60, 0x00, 0x3F, 0xFF,
			0xFE, 0x0F, 0xFF, 0xE0, 0x00, 0x0C, 0x60, 0x00, 0x7F, 0xFF, 0xFC, 0x07, 0xFF, 0xE0, 0x00, 0x0C,
			0x60, 0x00, 0xFF, 0xFF, 0xF0, 0x03, 0xFF, 0xF0, 0x00, 0x0C, 0x60, 0x00, 0xFF, 0xFF, 0xFE, 0x01,
			0xFF, 0xF8, 0x00, 0x0C, 0x60, 0x01, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xF8, 0x00, 0x0C, 0x60, 0x03,
			0xFF, 0xFF, 0xFE, 0x00, 0x7F, 0xFC, 0x00, 0x0C, 0x60, 0x07, 0xFF, 0xFF, 0xFE, 0x00, 0x3F, 0xFE,
			0x00, 0x0C, 0x60, 0x07, 0xFF, 0xFF, 0xE0, 0x00, 0x1F, 0xFF, 0x80, 0x0C, 0x60, 0x0F, 0xFF, 0xFF,
			0xE0, 0x00, 0x1F, 0xFF, 0x80, 0x0C, 0x60, 0x1F, 0xFF, 0xFF, 0xF0, 0x00, 0x0F, 0xFF, 0xC0, 0x0C,
			0x60, 0x1F, 0xFF, 0xFF, 0xFC, 0x00, 0x07, 0xFF, 0xE0, 0x0C, 0x60, 0x3F, 0xFB, 0xFF, 0xFC, 0x00,
			0x03, 0xFF, 0xF0, 0x0C, 0x60, 0x7F, 0xF3, 0xFF, 0xF8, 0x00, 0x03, 0xFF, 0xFC, 0x0C, 0x60, 0xFF,
			0xE3, 0xFF, 0xFF, 0x00, 0x03, 0xFF, 0xFC, 0x0C, 0x61, 0xFF, 0xE3, 0xFF, 0xFF, 0x00, 0x03, 0x7F,
			0xFE, 0x0C, 0x61, 0xFF, 0xC3, 0xFF, 0xF0, 0x00, 0x03, 0x3F, 0xFF, 0x8C, 0x63, 0xFF, 0x83, 0xFF,
			0xFC, 0x00, 0x03, 0x1F, 0xFF, 0x8C, 0x67, 0xFF, 0x03, 0xFF, 0xFF, 0x00, 0x03, 0x0F, 0xFF, 0xCC,
			0x7F, 0xFE, 0x03, 0xFF, 0xFF, 0x00, 0x03, 0x07, 0xFF, 0xFC, 0x7F, 0xFE, 0x07, 0xFF, 0xFF, 0x00,
			0x03, 0x03, 0xFF, 0xFC, 0x7F, 0xFC, 0x07, 0xFF, 0xF8, 0x00, 0x03, 0x01, 0xFF, 0xFC, 0x7F, 0xF8,
			0x07, 0xFF, 0xF0, 0x00, 0x01, 0x01, 0xFF, 0xFC, 0x7F, 0xF0, 0x07, 0xFF, 0xF0, 0x00, 0x01, 0x00,
			0x00, 0x0C, 0x60, 0x00, 0x07, 0xFF, 0xF0, 0x00, 0x01, 0x80, 0x00, 0x0C, 0x60, 0x00, 0x07, 0xFF,
			0xFE, 0x00, 0x01, 0x80, 0x00, 0x0C, 0x60, 0x00, 0x0F, 0xFF, 0xFE, 0x00, 0x01, 0x80, 0x00, 0x0C,
			0x60, 0x00, 0x0F, 0xFF, 0xF2, 0x00, 0x00, 0xC0, 0x00, 0x0C, 0x60, 0x00, 0x0F, 0xFF, 0xF0, 0x00,
			0x00, 0xC0, 0x00, 0x0C, 0x60, 0x00, 0x0F, 0xFF, 0xFF, 0x00, 0x00, 0xC0, 0x00, 0x0C, 0x60, 0x00,
			0x0F, 0xFF, 0xF6, 0x00, 0x00, 0xC0, 0x00, 0x0C, 0x60, 0x00, 0x1F, 0xFF, 0xF0, 0x00, 0x00, 0x40,
			0x00, 0x0C, 0x60, 0x00, 0x1F, 0xFF, 0xF0, 0x00, 0x00, 0x40, 0x00, 0x0C, 0x60, 0x00, 0x1F, 0xFF,
			0xFF, 0x00, 0x00, 0x40, 0x00, 0x0C, 0x60, 0x00, 0x1F, 0xFF, 0xFF, 0x00, 0x00, 0x60, 0x00, 0x0C,
			0x60, 0x00, 0x1F, 0xFF, 0xF8, 0x00, 0x00, 0x60, 0x00, 0x0C, 0x60, 0x00, 0x1F, 0xFF, 0xFE, 0x00,
			0x00, 0x60, 0x00, 0x0C, 0x60, 0x00, 0x3F, 0xFF, 0xFF, 0x00, 0x00, 0x60, 0x00, 0x0C, 0x60, 0x00,
			0x3F, 0xFF, 0xFF, 0x00, 0x00, 0x70, 0x00, 0x0C, 0x60, 0x00, 0x3F, 0xFF, 0xFE, 0x00, 0x00, 0x70,
			0x00, 0x0C, 0x60, 0x00, 0x3F, 0xFF, 0xE0, 0x00, 0x00, 0x70, 0x00, 0x0C, 0x60, 0x00, 0x3F, 0xFF,
			0xE0, 0x00, 0x00, 0x70, 0x00, 0x0C, 0x60, 0x00, 0x3F, 0xFF, 0xFF, 0x00, 0x00, 0x70, 0x00, 0x0C,
			0x60, 0x00, 0x3F, 0xFF, 0xFF, 0x00, 0x00, 0x70, 0x00, 0x0C, 0x60, 0x00, 0x3F, 0xFF, 0xFF, 0x80,
			0x00, 0x38, 0x00, 0x0C, 0x60, 0x00, 0x3F, 0xFF, 0xFF, 0xF8, 0x00, 0x38, 0x00, 0x0C, 0x60, 0x00,
			0x7F, 0xFF, 0xFF, 0xFF, 0xF0, 0x38, 0x00, 0x0C, 0x60, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0x38,
			0x00, 0x0C, 0x60, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x0C, 0x60, 0x00, 0x7F, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x0C, 0x60, 0x00, 0x79, 0x44, 0x49, 0x68, 0x0E, 0xE4, 0x00, 0x0C,
			0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x00, 0xE3, 0x8C, 0x00,
			0x00, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x00, 0xE3, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x60, 0x00,
			0x00, 0xF7, 0x8C, 0xC6, 0x7C, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x00, 0xF7, 0x8E, 0x6C, 0x66, 0x00,
			0x00, 0x0C, 0x60, 0x00, 0x00, 0xDD, 0x9C, 0x6C, 0x66, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x00, 0xDD,
			0x8C, 0x6C, 0x66, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x00, 0xDD, 0x8C, 0x38, 0x66, 0x00, 0x00, 0x0C,
			0x60, 0x00, 0x00, 0xC9, 0x8C, 0x38, 0x66, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x00, 0x00, 0x00, 0x30,
			0x00, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x30, 0x00, 0x0C, 0x61, 0xF8,
			0x00, 0x18, 0x00, 0x00, 0x00, 0x60, 0x00, 0x0C, 0x60, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x0C, 0x60, 0x30, 0x3C, 0x1B, 0x03, 0xC1, 0xE0, 0xF0, 0x64, 0xCC, 0x60, 0x60, 0x46, 0x1D,
			0x82, 0x41, 0x80, 0x90, 0x6E, 0xCC, 0x60, 0x60, 0x1E, 0x19, 0x86, 0x61, 0x81, 0x98, 0x6E, 0xCC,
			0x60, 0xC0, 0x36, 0x19, 0x86, 0x61, 0x81, 0x98, 0x7B, 0xCC, 0x61, 0x80, 0x66, 0x19, 0x86, 0x61,
			0x81, 0x98, 0x3B, 0x8C, 0x61, 0x80, 0x66, 0x1D, 0x86, 0x41, 0x81, 0x90, 0x3B, 0x8C, 0x61, 0xF8,
			0x3E, 0x1B, 0x03, 0x81, 0x80, 0xE0, 0x31, 0x8C, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x0C, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x7F, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8
			};
	u8g_FirstPage(&u8g);
	do{
		u8g_DrawBitmap(&u8g,80,4,10,120,logo);
	}while(u8g_NextPage(&u8g));
}
