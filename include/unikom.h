/*
 * unikom.h
 *
 *  Created on: 21 sty 2017
 *      Author: Marcin
 */

#ifndef UNIKOM_H_
#define UNIKOM_H_

typedef enum{
	KOM_OK = 0,
	KOM_INVALID_FRAME,
	KOM_INVALID_CMD,
	KOM_INVALID_DATANAME,
	KOM_CMDEXEC_FAIL,
	KOM_DATAPROC_FAIL,
	KOM_TIMEOUT,
	KOM_CRC_ERR,
	KOM_BUF_OVF,
	KOM_ERR
}komErrType;

typedef enum{
	CMD = 0x01,
	DATA
}komFrameType;

typedef struct{
	uint16_t 		bufReadPointer;
	komFrameType 	frameType;
	char	 		nameString[16];	// Command/Data string
	uint8_t  		listPos;			// Pozycja na liście komend / danych
	komErrType 		error;
}komMainS;


void Kom_Main(void);



#endif /* UNIKOM_H_ */
