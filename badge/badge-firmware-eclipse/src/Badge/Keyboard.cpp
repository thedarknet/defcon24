#include "Keyboard.h"
#include <gpio.h>
#include <string.h>

KeyBoardLetterCtx::KeyBoardLetterCtx() {
}

void KeyBoardLetterCtx::processButtonPush(uint8_t button, const char *buttonLetters) {
	if (LastPin == button) {
		LetterSelection++;
		if (LetterSelection >= strlen(buttonLetters)) {
			LetterSelection = 0;
		}
	} else {
		LetterSelection = 0;
		setCurrentLetterInBufferAndInc();
	}
	if(buttonLetters[LetterSelection]=='\b') {
		Buffer[CursorPosition] = '\0';
		decPosition();
	} else {
		CurrentLetter = buttonLetters[LetterSelection];
	}
	LastPin = button;
	timerStart();
}
bool KeyBoardLetterCtx::isKeySelectionTimedOut() {
	if (Started && (HAL_GetTick() - LastTimeLetterWasPushed) > 3000) {
		return true;
	}
	return false;
}
void KeyBoardLetterCtx::timerStart() {
	Started = true;
	LastTimeLetterWasPushed = HAL_GetTick();
}
void KeyBoardLetterCtx::timerStop() {
	Started = false;
}

void KeyBoardLetterCtx::decPosition() {
	if (--CursorPosition <-1 ) {
		CursorPosition = - 1;
	}
}

void KeyBoardLetterCtx::incPosition() {
	if (++CursorPosition >= BufferSize) {
		CursorPosition = BufferSize - 1;
	}
}
uint8_t KeyBoardLetterCtx::getCursorPosition() {
	return CursorPosition;
}
void KeyBoardLetterCtx::setCurrentLetterInBufferAndInc() {
	if(CursorPosition>=0) {
		Buffer[CursorPosition] = CurrentLetter;
	}
	incPosition();
}
void KeyBoardLetterCtx::blinkLetter() {
	uint32_t tmp = HAL_GetTick() / 500;
	if (tmp != LastBlinkTime) {
		LastBlinkTime = tmp;
		UnderBar = !UnderBar;
	}
	if (UnderBar) {
		Buffer[CursorPosition] = '_';
	} else {
		Buffer[CursorPosition] = CurrentLetter;
	}
}
void KeyBoardLetterCtx::init(char *b, uint16_t s) {
	Buffer = b;
	Started = false;
	UnderBar = true;
	CurrentLetter = ' ';
	BufferSize = s;
	CursorPosition = -1;
	LastTimeLetterWasPushed = 0;
	LetterSelection = 0;
	LastBlinkTime = 0;
	LastPin = 0xFF;
	ButtonReleased = 1;
}

QKeyboard::QKeyboard(PinConfig Y1Pin, PinConfig Y2Pin, PinConfig Y3Pin, PinConfig X1Pin, PinConfig X2Pin,
		PinConfig X3Pin, PinConfig X4Pin) :
		LastSelectedPin(NO_PIN_SELECTED), TimesLastPinSelected(0) {
	YPins[0] = Y1Pin;
	YPins[1] = Y2Pin;
	YPins[2] = Y3Pin;
	XPins[0] = X1Pin;
	XPins[1] = X2Pin;
	XPins[2] = X3Pin;
	XPins[3] = X4Pin;
}

bool lightAll = true;

void QKeyboard::setAllLightsOn(bool b) {
	lightAll = b;
}

void QKeyboard::scan() {
	HAL_GPIO_WritePin(LED_OUT1_GPIO_Port, LED_OUT1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LED_OUT2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LED_OUT3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LED_OUT4_Pin, GPIO_PIN_RESET);
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
	if (lightAll) {
		for (uint8_t x = 0; x < xPins; ++x) {
			HAL_GPIO_WritePin(XPins[x].Port, XPins[x].Pin, GPIO_PIN_RESET);
		}
		HAL_GPIO_WritePin(LED_OUT1_GPIO_Port, LED_OUT1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, LED_OUT2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, LED_OUT3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, LED_OUT4_Pin, GPIO_PIN_SET);
	}
}

void QKeyboard::reset() {
	TimesLastPinSelected = 0;
	LastSelectedPin = NO_PIN_SELECTED;
}

void QKeyboard::updateContext(KeyBoardLetterCtx &ctx) {
	if (ctx.isKeySelectionTimedOut() && getLastPinSeleted() == QKeyboard::NO_PIN_SELECTED) {
		ctx.setCurrentLetterInBufferAndInc();
		ctx.timerStop();
	} else if (getLastPinSeleted() != QKeyboard::NO_PIN_SELECTED && ctx.hasReleasedButton()) {
		ctx.setButtonPushed();
		const char *current = 0;
		switch (getLastPinSeleted()) {
		case 0:
			current = ".,?1";
			break;
		case 1:
			current = "ABC2";
			break;
		case 2:
			current = "DEF3";
			break;
		case 3:
			current = "GHI4";
			break;
		case 4:
			current = "JKL5";
			break;
		case 5:
			current = "MNO6";
			break;
		case 6:
			current = "PQRS7";
			break;
		case 7:
			current = "TUV8";
			break;
		case 8:
			current = "WXYZ9";
			break;
		case 9:
			current = "##+";
			break;
		case 10:
			current = "0\b";
			break;
		}
		if(getLastPinSeleted()<11) {
			ctx.processButtonPush(getLastPinSeleted(), current);
		}
	} else if (getLastPinSeleted()==QKeyboard::NO_PIN_SELECTED) {
		ctx.setButtonReleased();
	}
	ctx.blinkLetter();
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
