
#ifndef BADGE_H
#define BADGE_H

#include <stdint.h>

class ContactStore;
class RFM69;

ContactStore &getContactStore();
RFM69 &getRadio();

class ErrorType {
public:
	enum ERROR_ENUM {
		NO_ERROR = 0
		, OLED_INIT_ERROR
		, RADIO_INIT_ERROR
		, IR_INIT_ERROR
		, FLASH_MEM_ERROR
		, TIMER_ERROR
	};
public:
	ErrorType(uint8_t e) : Error(e) {}
	ErrorType() : Error(NO_ERROR) {}
	bool ok() {return Error==NO_ERROR;}
	uint8_t getError() {return Error;}
	const char *getMessage();
private:
	uint8_t Error;
};

enum COMPONENTS_ITEMS {
	OLED = (1<<0),
	RADIO = (1<<1),
	IR = (1<<2),
	FLASH_MEM = (1<<3)
};

uint32_t startBadge(void);

void loopBadge(void);

void delay(uint32_t time);

#endif
