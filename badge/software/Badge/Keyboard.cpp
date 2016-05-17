
#include "Keyboard.h"
#include <gpio.h>

QKeyboard::QKeyboard(PinConfig Y1Pin, PinConfig Y2Pin, PinConfig Y3Pin, PinConfig X1Pin
	, PinConfig X2Pin, PinConfig X3Pin, PinConfig X4Pin) : LastSelectedPin(NO_PIN_SELECTED), TimesLastPinSelected(0) {
	YPins[0] = Y1Pin;
	YPins[1] = Y2Pin;
	YPins[2] = Y3Pin;
	XPins[0] = X1Pin;
	XPins[1] = X2Pin;
	XPins[2] = X3Pin;
	XPins[3] = X4Pin;
}

void QKeyboard::scan() {
	uint8_t pinToSet = 0;
	uint8_t selectedPin = NO_PIN_SELECTED;
	for (uint8_t x=0;x<sizeof(XPins)/sizeof(XPins[0]);x++) {
		//set the correct X pin
		if((pinToSet++)==x) {
			HAL_GPIO_WritePin(XPins[x].Port,XPins[x].Pin,GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(XPins[x].Port,XPins[x].Pin,GPIO_PIN_RESET);
		}
		//Read Y bins
		for(uint8_t y=0;y<sizeof(YPins)/sizeof(YPins[0]);y++) {
			if(HAL_GPIO_ReadPin(YPins[y].Port,YPins[y].Pin)==GPIO_PIN_RESET) {
				selectedPin = (x*sizeof(XPins)/sizeof(XPins[0])) + y;
				break;
			} 
		}
	}
	if(selectedPin==LastSelectedPin) {
		TimesLastPinSelected++;
	} else {
		LastSelectedPin = selectedPin;
		TimesLastPinSelected=0;
	}
}

uint8_t QKeyboard::getLastPinPushed() {
	return LastSelectedPin;
}

uint8_t QKeyboard::getLastPinSeleted() {
	if(LastSelectedPin!=QKeyboard::NO_PIN_SELECTED && TimesLastPinSelected>=QKeyboard::TIMES_BUTTON_MUST_BE_HELD) {
		return LastSelectedPin;
	}
	return QKeyboard::NO_PIN_SELECTED;
}
