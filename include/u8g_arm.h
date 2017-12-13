/*
  
  u8g_arm.h
  
  Header file for u8g LPC122x utility procedures 

  Universal 8bit Graphics Library
  
  Copyright (c) 2013, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

*/


#ifndef _U8G_ARM_H
#define _U8G_ARM_H

#include "u8g.h"
#include "stm32f4xx_conf.h"

#define SPI_PORT_PCLK RCC_AHB1Periph_GPIOA
#define SPI_PORT GPIOA
#define SPI_SCK_PIN GPIO_Pin_5
#define SPI_SCK_PIN_SOURCE GPIO_PinSource5
#define SPI_MOSI_PIN GPIO_Pin_7
#define SPI_MOSI_PIN_SOURCE GPIO_PinSource7

#define DISP_PORT_PCLK RCC_AHB1Periph_GPIOA
#define DISP_PORT GPIOA
#define DISP_CS_PIN GPIO_Pin_4
#define DISP_A0_PIN GPIO_Pin_2
#define DISP_RST_PIN GPIO_Pin_3


#define CS_ON()        GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define CS_OFF()       GPIO_ResetBits(GPIOA, GPIO_Pin_4)

void SPI2_Init(void);
void SPI_Out (unsigned char);
void set_gpio_level(GPIO_TypeDef*, uint16_t, uint8_t);

uint8_t u8g_com_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr);

#endif


