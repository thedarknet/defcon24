#include "Keyboard.h"
#include <gpio.h>

QKeyboard::QKeyboard(PinConfig Y1Pin, PinConfig Y2Pin, PinConfig Y3Pin, PinConfig X1Pin, PinConfig X2Pin,
		PinConfig X3Pin, PinConfig X4Pin) :
		LastSelectedPin(NO_PIN_SELECTED), TimesLastPinSelected(0), CurrentLetter(0) {
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
			if (HAL_GPIO_ReadPin(YPins[y].Port, YPins[y].Pin) == GPIO_PIN_RESET) {
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
	setLetter();
	//++lightRow;
	//lightRow = lightRow % 4;
}

void QKeyboard::setLetter() {
	int letterSelection = 0;
	switch (getLastPinSeleted()) {
	case 0:
		static const char *zero = ".,?";//ABC1";
		CurrentLetter = zero[letterSelection];
		break;
	case 1:
		static const char *one = "DEF2";
		CurrentLetter = one[letterSelection];
		break;
	case 2:
		static const char *two = "GHI3";
		CurrentLetter = two[letterSelection];
		break;
	case 3:
		static const char *three = "JKL4";
		CurrentLetter = three[letterSelection];
		break;
	case 4:
		static const char *four = "MNO5";
		CurrentLetter = four[letterSelection];
		break;
	case 5:
		static const char *five = "PQR6";
		CurrentLetter = five[letterSelection];
		break;
	case 6:
		static const char *six = "STU7";
		CurrentLetter = six[letterSelection];
		break;
	case 7:
		static const char *seven = "VWX8";
		CurrentLetter = seven[letterSelection];
		break;
	case 8:
		static const char *eight = "YZ9";
		CurrentLetter = eight[letterSelection];
		break;
	case 9:
		static const char *nine = "#\b";
		CurrentLetter = nine[letterSelection];
		break;
	case 10:
		CurrentLetter = '0';
		break;
	case 11:
		CurrentLetter = '\n';
		break;
	}
}

char QKeyboard::getLetter() {
	return CurrentLetter;
}

uint8_t QKeyboard::getNumber() {
	switch (getLastPinSeleted()) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		return getLastPinSeleted() + 1;
		break;
	case 10:
		return 0;
		break;
	case 9:
	case 11:
		return NOT_A_NUMBER;
		break;
	}
}

uint8_t QKeyboard::getLastPinPushed() {
	return LastSelectedPin;
}

uint8_t QKeyboard::getLastPinSeleted() {
	if (LastSelectedPin != QKeyboard::NO_PIN_SELECTED && TimesLastPinSelected >= QKeyboard::TIMES_BUTTON_MUST_BE_HELD) {
		return LastSelectedPin;
	}
	return QKeyboard::NO_PIN_SELECTED;
}
