#ifndef  KEYBOARD_H
#define KEYBOARD_H

#include <stm32f1xx_hal.h>

class KeyBoardLetterCtx {
private:
	int16_t CursorPosition:10;
	int16_t Started : 1;
	int16_t UnderBar :1;
	char *Buffer;
	char CurrentLetter;
	uint16_t BufferSize;
	uint32_t LastTimeLetterWasPushed;
	uint8_t LetterSelection;
	uint32_t LastBlinkTime;
	uint8_t LastPin;
public:
	void processButtonPush(uint8_t button, const char *buttonLetters);
	bool isKeySelectionTimedOut();
	void timerStart();
	void timerStop();
	void incPosition();
	void decPosition();
	uint8_t getCursorPosition();
	void setCurrentLetterInBufferAndInc();
	void blinkLetter();
	KeyBoardLetterCtx();
	void resetChar();
	void finalize();
	void init(char *b, uint16_t s);
};

KeyBoardLetterCtx &getKeyboardContext();

class QKeyboard {
public:
	struct PinConfig {
		GPIO_TypeDef *Port;
		uint16_t Pin;
		PinConfig() :
				Port(0), Pin(0) {
		}
		PinConfig(GPIO_TypeDef *port, uint16_t pin) :
				Port(port), Pin(pin) {
		}
	};
	static const uint8_t NO_PIN_SELECTED = 0xFF;
	static const uint8_t NOT_A_NUMBER = 0xFF;
	static const uint8_t NO_LETTER_SELECTED = 0xFF;
	static const uint8_t TIMES_BUTTON_MUST_BE_HELD = 5;
public:
	QKeyboard(PinConfig Y1Pin, PinConfig Y2Pin, PinConfig Y3Pin, PinConfig X1Pin, PinConfig X2Pin, PinConfig X3Pin,
			PinConfig X4Pin);
	void scan();
	///last button pushed
	uint8_t getLastPinPushed();
	//last button pushed and held for at least TIMES_BUTTON_MUST_BE_HELD
	uint8_t getLastPinSeleted();
	uint8_t getLastKeyReleased();
	bool wasKeyReleased();
	void updateContext(KeyBoardLetterCtx &ctx);
	void reset();
	void setAllLightsOn(bool b);
protected:
	void setLetter();
private:
	PinConfig YPins[3];
	PinConfig XPins[4];
	uint8_t LastSelectedPin;
	uint8_t TimesLastPinSelected;
	uint8_t KeyJustReleased;
};

#endif
