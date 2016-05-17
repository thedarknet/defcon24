#ifndef  KEYBOARD_H
#define KEYBOARD_H

#include <stm32f1xx_hal.h>

class QKeyboard {
	public:
		struct PinConfig {
			GPIO_TypeDef *Port;
			uint8_t Pin;
			PinConfig(): Port(0), Pin(0) {}
			PinConfig(GPIO_TypeDef *port, uint8_t pin) : Port(port), Pin(pin) {}
		};
		static const uint8_t NO_PIN_SELECTED = 0xFF;
		static const uint8_t TIMES_BUTTON_MUST_BE_HELD = 3;
	public:
		QKeyboard(PinConfig Y1Pin, PinConfig Y2Pin, PinConfig Y3Pin, PinConfig X1Pin
			, PinConfig X2Pin, PinConfig X3Pin, PinConfig X4Pin);
		void scan();
		///last button pushed
		uint8_t getLastPinPushed();
		//last button pushed and held for at least TIMES_BUTTON_MUST_BE_HELD
		uint8_t getLastPinSeleted();
	private:
		PinConfig YPins[3];
		PinConfig XPins[4];
		uint8_t LastSelectedPin;
		uint8_t TimesLastPinSelected;
};

#endif