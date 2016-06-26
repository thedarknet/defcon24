
#ifndef BADGE_H
#define BADGE_H

#include <stdint.h>

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
