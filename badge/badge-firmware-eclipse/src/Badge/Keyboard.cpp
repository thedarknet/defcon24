#include "Keyboard.h"
#include <gpio.h>

QKeyboard::QKeyboard(PinConfig Y1Pin, PinConfig Y2Pin, PinConfig Y3Pin,
		PinConfig X1Pin, PinConfig X2Pin, PinConfig X3Pin, PinConfig X4Pin) :
		LastSelectedPin(NO_PIN_SELECTED), TimesLastPinSelected(0) {
	YPins[0] = Y1Pin;
	YPins[1] = Y2Pin;
	YPins[2] = Y3Pin;
	XPins[0] = X1Pin;
	XPins[1] = X2Pin;
	XPins[2] = X3Pin;
	XPins[3] = X4Pin;
}

int lightRow = 0;

void QKeyboard::scan() {
	//HAL_GPIO_WritePin(LED_OUT1_GPIO_Port, LED_OUT1_Pin, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(GPIOB, LED_OUT2_Pin, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(GPIOB, LED_OUT3_Pin, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(GPIOB, LED_OUT4_Pin, GPIO_PIN_RESET);
	uint8_t selectedPin = NO_PIN_SELECTED;
	int xPins = sizeof(XPins) / sizeof(XPins[0]);
	int yPins = sizeof(YPins) / sizeof(YPins[0]);
	for (int r = 0; r < xPins && selectedPin == NO_PIN_SELECTED; ++r) {
		for (uint8_t x = 0; x < xPins; ++x) {
			//set the correct X pin
			if (x != r) {
				HAL_GPIO_WritePin(XPins[x].Port, XPins[x].Pin, GPIO_PIN_SET);
			} else {
				HAL_GPIO_WritePin(XPins[x].Port, XPins[x].Pin, GPIO_PIN_RESET);
			}
		}
		//if (lightRow == r) {
		//	HAL_GPIO_WritePin(LED_OUT1_GPIO_Port, LED_OUT1_Pin, GPIO_PIN_SET);
		//	HAL_GPIO_WritePin(GPIOB, LED_OUT2_Pin, GPIO_PIN_SET);
		//	HAL_GPIO_WritePin(GPIOB, LED_OUT3_Pin, GPIO_PIN_SET);
		//	HAL_GPIO_WritePin(GPIOB, LED_OUT4_Pin, GPIO_PIN_SET);
		//}
		//Read Y bins
		for (uint8_t y = 0; y < yPins; y++) {
			if (HAL_GPIO_ReadPin(YPins[y].Port, YPins[y].Pin)
					== GPIO_PIN_RESET) {
				selectedPin = r * yPins + y;
				break;
			}
		}
	}
	if (selectedPin == LastSelectedPin) {
		TimesLastPinSelected++;
	} else {
		LastSelectedPin = selectedPin;
		TimesLastPinSelected = 0;
	}
	//++lightRow;
	//lightRow = lightRow % 4;
}

char QKeyboard::getLetter() {
	return 'A';
}

uint8_t QKeyboard::getNumber() {
	return 0;
}

uint8_t QKeyboard::getLastPinPushed() {
	return LastSelectedPin;
}

uint8_t QKeyboard::getLastPinSeleted() {
	if (LastSelectedPin != QKeyboard::NO_PIN_SELECTED
			&& TimesLastPinSelected >= QKeyboard::TIMES_BUTTON_MUST_BE_HELD) {
		return LastSelectedPin;
	}
	return QKeyboard::NO_PIN_SELECTED;
}
