// RHSPIDriver.cpp
//
// Copyright (C) 2014 Mike McCauley
// $Id: RHSPIDriver.cpp,v 1.10 2015/12/16 04:55:33 mikem Exp $

#include <RHSPIDriver.h>
#include <stm32f1xx_hal_spi.h>
#include <spi.h>

#define SPI_TIMEOUT 1000

RHSPIDriver::RHSPIDriver(uint8_t slaveSelectPin, RHGenericSPI& spi)
    : 
    _spi(spi),
    _slaveSelectPin(slaveSelectPin)
{
}

bool RHSPIDriver::init()
{
    //all handled by HAL init
    return true;
}

uint8_t RHSPIDriver::spiRead(uint8_t reg)
{
	uint8_t t[2];
	t[0] = reg & ~RH_SPI_WRITE_MASK;
	t[1] = 0;
	HAL_SPI_TransmitReceive(&hspi1,&t[0],&t[0],2,SPI_TIMEOUT);
	uint8_t status = t[0];
	return t[1];
}

uint8_t RHSPIDriver::spiWrite(uint8_t reg, uint8_t val)
{
	uint8_t t[2];
	t[0] = reg | RH_SPI_WRITE_MASK;
	t[1] = val;
	HAL_SPI_TransmitReceive(&hspi1,&t[0],&t[0],2,SPI_TIMEOUT);
	return t[0];
}

uint8_t RHSPIDriver::spiBurstRead(uint8_t reg, uint8_t* dest, uint8_t len)
{
	memset(dest,0,len);
	dest[0] = reg&~RH_SPI_WRITE_MASK;
	HAL_SPI_TransmitReceive(&hspi1,dest,dest,1,SPI_TIMEOUT);
	uint8_t status = dest[0];
	HAL_SPI_TransmitReceive(&hspi1,dest,dest,len,SPI_TIMEOUT);
	return (hspi1.RxXferSize - hspi1.RxXferCount);
}

uint8_t RHSPIDriver::spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len)
{
	reg |=RH_SPI_WRITE_MASK;
	HAL_SPI_TransmitReceive(&hspi1,&reg,&reg,1,SPI_TIMEOUT);
	uint8_t status = reg;
	HAL_SPI_Receive(&hspi1,const_cast<uint8_t*>(src),len,SPI_TIMEOUT);
	return (hspi1.RxXferSize - hspi1.RxXferCount);
}

void RHSPIDriver::setSlaveSelectPin(uint8_t slaveSelectPin)
{
    //_slaveSelectPin = slaveSelectPin;
}
