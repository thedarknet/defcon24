// ArduinoCompat/HardwareSPI.cpp
//
// Interface between Arduino-like SPI interface and STM32F4 Discovery and similar
// using STM32F4xx_DSP_StdPeriph_Lib_V1.3.0

#include <RadioHead.h>
#if (RH_PLATFORM == RH_PLATFORM_STM32F1_HAL)

#include <HardwareSPI.h>
#include <spi.h>


HardwareSPI::HardwareSPI(uint32_t spiPortNumber) :
    _spiPortNumber(spiPortNumber) {
}

void HardwareSPI::begin() {
	//HAL_SPI_MspInit(&hspi1);
}

void HardwareSPI::end(void) {
	HAL_SPI_MspDeInit(&hspi1);
}

class ChipSelect {
public:
	ChipSelect() {
		HAL_GPIO_WritePin(GPIOA,RFM69_SPI_NSS_Pin,GPIO_PIN_RESET);
	}
	~ChipSelect() {
		HAL_GPIO_WritePin(GPIOA,RFM69_SPI_NSS_Pin,GPIO_PIN_SET);
	}
};

uint8_t HardwareSPI::transfer(uint8_t data)
{
	/*
	typedef enum 
{
  HAL_OK       = 0x00,
  HAL_ERROR    = 0x01,
  HAL_BUSY     = 0x02,
  HAL_TIMEOUT  = 0x03
} HAL_StatusTypeDef;
	*/
	ChipSelect cs;
  if(HAL_OK==HAL_SPI_TransmitReceive(&hspi1, &data, &data, 1, 1000)) {
		return data;
	}
		return 0;

}

#endif
