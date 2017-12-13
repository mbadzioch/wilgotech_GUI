#include "u8g_arm.h"
#include "delay.h"

void u8g_Delay(uint16_t val){
	Delay_us(1000UL*(uint32_t)val);
}

void u8g_MicroDelay(void){
	Delay_us(1);
}

void u8g_10MicroDelay(void){
	Delay_us(10);
}

void set_gpio_level(GPIO_TypeDef* reg, uint16_t pin, uint8_t level){
  if ( level == 0 ) {
	  GPIO_ResetBits(reg, pin);
  }
  else {
	  GPIO_SetBits(reg, pin);
  }
}

void SPI2_Init(void){
	GPIO_InitTypeDef SPI_PinInit;
	SPI_InitTypeDef SPI2_Init;

	//Display ports init
	GPIO_InitTypeDef DISP_Pin_Init;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //clock SPI

	DISP_Pin_Init.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
	DISP_Pin_Init.GPIO_Mode = GPIO_Mode_OUT;
	DISP_Pin_Init.GPIO_OType = GPIO_OType_PP;
	DISP_Pin_Init.GPIO_Speed = GPIO_Speed_100MHz;
	DISP_Pin_Init.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &DISP_Pin_Init);

	GPIO_SetBits(GPIOA,GPIO_Pin_1);

	SPI_PinInit.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	SPI_PinInit.GPIO_Mode = GPIO_Mode_AF;
	SPI_PinInit.GPIO_OType = GPIO_OType_PP;
	SPI_PinInit.GPIO_Speed = GPIO_Speed_100MHz;
	SPI_PinInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &SPI_PinInit);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	//CS_OFF(); //czy tu na pewno CS=0?


	SPI2_Init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI2_Init.SPI_CPHA = SPI_CPHA_2Edge; // CPHA = 1
	SPI2_Init.SPI_CPOL = SPI_CPOL_High; // CPOL = 1
	SPI2_Init.SPI_CRCPolynomial = 0; // standard CRC-8
	SPI2_Init.SPI_DataSize = SPI_DataSize_8b;//
	SPI2_Init.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI2_Init.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI2_Init.SPI_Mode = SPI_Mode_Master;
	SPI2_Init.SPI_NSS = SPI_NSS_Soft;
	SPI_Init (SPI1, &SPI2_Init);
	SPI_Cmd	 (SPI1, ENABLE);
}

void SPI_Out(uint8_t data){
	SPI1->DR = data;
	while ((SPI1->SR & SPI_I2S_FLAG_TXE) == 0); //wait until transmit complete
	while (SPI1->SR & SPI_I2S_FLAG_BSY);
}

uint8_t u8g_com_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr){
	switch(msg){
		case U8G_COM_MSG_STOP:
			//STOP THE DEVICE
		break;

		case U8G_COM_MSG_INIT:{
			//INIT HARDWARE INTERFACES, TIMERS, GPIOS...

			SPI2_Init();
			u8g_MicroDelay();
		}
		break;

		case U8G_COM_MSG_ADDRESS:
			//SWITCH FROM DATA TO COMMAND MODE /* define cmd (arg_val = 0) or data mode (arg_val = 1) */
			u8g_10MicroDelay();
			set_gpio_level(DISP_PORT, DISP_A0_PIN, arg_val);
			u8g_10MicroDelay();
		break;

		case U8G_COM_MSG_CHIP_SELECT:
			if ( arg_val == 0 ) {
				// disable
				uint8_t i;
				// this delay is required to avoid that the display is switched off too early --> DOGS102 with LPC1114
				for( i = 0; i < 3; i++ )
					u8g_10MicroDelay();
				set_gpio_level(DISP_PORT, DISP_CS_PIN, 1);
			}
			else {
				/* enable */
				set_gpio_level(DISP_PORT, DISP_CS_PIN, 0);
			}
			u8g_MicroDelay();
		break;

		case U8G_COM_MSG_RESET:
			//TOGGLE THE RESET PIN ON THE DISPLAY BY THE VALUE IN arg_val
			set_gpio_level(DISP_PORT, DISP_RST_PIN, arg_val);
			u8g_10MicroDelay();
		break;

		case U8G_COM_MSG_WRITE_BYTE:
			//WRITE BYTE TO DEVICE
			SPI_Out(arg_val);
			u8g_MicroDelay();
		break;

		case U8G_COM_MSG_WRITE_SEQ:
		case U8G_COM_MSG_WRITE_SEQ_P:{
			//WRITE A SEQUENCE OF BYTES TO THE DEVICE
			register uint8_t *ptr = arg_ptr;
			while( arg_val > 0 ){
				SPI_Out(*ptr++);
				arg_val--;
			}
		} break;
	}
	return 1;
}



