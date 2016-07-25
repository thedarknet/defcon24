// ArduinoCompat/HardwareSPI.cpp
//
// Interface between Arduino-like SPI interface and STM32F4 Discovery and similar
// using STM32F4xx_DSP_StdPeriph_Lib_V1.3.0


#include "HardwareSPI.h"
#include <spi.h>

HardwareSPI SPI;

HardwareSPI::HardwareSPI() {
}

void HardwareSPI::begin() {
	//HAL_SPI_MspInit(&hspi1);
}

void HardwareSPI::end(void) {
	HAL_SPI_MspDeInit(&hspi1);
}

uint8_t HardwareSPI::transfer(uint8_t data) {
	/*
	 typedef enum
	 {
	 HAL_OK       = 0x00,
	 HAL_ERROR    = 0x01,
	 HAL_BUSY     = 0x02,
	 HAL_TIMEOUT  = 0x03
	 } HAL_StatusTypeDef;
	 */

	if (HAL_OK == HAL_SPI_TransmitReceive(&hspi1, &data, &data, 1, 1000)) {
		return data;
	}
	return 0;

}

