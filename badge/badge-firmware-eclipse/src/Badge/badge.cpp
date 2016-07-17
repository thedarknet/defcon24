#include "badge.h"
#include "stdint.h"
#include "gui.h"
#include "menus.h"
#include <RFM69.h>
#include <Keyboard.h>
#include <KeyStore.h>
#include <tim.h>
#include <usart.h>

char sendingBuf[64] = { '\0' };
char receivingBuf[64] = { '\0' };

#define ONE_TIME 1

int state = 0;
int tmp = 0;

//RH_RF69 Radio(RFM69_SPI_NSS_Pin,RFM69_Interrupt_DIO0_Pin);
QKeyboard KB(QKeyboard::PinConfig(KEYBOARD_Y1_GPIO_Port, KEYBOARD_Y1_Pin),
		QKeyboard::PinConfig(KEYBOARD_Y2_GPIO_Port, KEYBOARD_Y2_Pin),
		QKeyboard::PinConfig(KEYBOARD_Y3_GPIO_Port, KEYBOARD_Y3_Pin),
		QKeyboard::PinConfig(KEYBOARD_X1_GPIO_Port, KEYBOARD_X1_Pin),
		QKeyboard::PinConfig(KEYBOARD_X2_GPIO_Port, KEYBOARD_X2_Pin),
		QKeyboard::PinConfig(KEYBOARD_X3_GPIO_Port, KEYBOARD_X3_Pin),
		QKeyboard::PinConfig(KEYBOARD_X4_GPIO_Port, KEYBOARD_X4_Pin));

RFM69 Radio(RFM69_SPI_NSS_Pin, RFM69_Interrupt_DIO0_Pin,true);
//RFM69 Radio;

//start at 45K for 19K
//start at 55K for 10
//start at 58K for 6k
static const uint32_t START_STORAGE_LOCATION = 0x800e290; //0x800d400; //0x800B000;
//static const uint32_t START_STORAGE_LOCATION = 0x800d400; //0x800B000;
ContactStore MyContacts(START_STORAGE_LOCATION, 0x1770); //0x2710); //0x4E00);

ContactStore &getContactStore() {
	return MyContacts;
}

RFM69 &getRadio() {
	return Radio;
}

void delay(uint32_t time) {
	HAL_Delay(time);
}


const char *ErrorType::getMessage() {
	return "ErrorType:  TODO";
}

ErrorType::ErrorType(const ErrorType &r) {
	this->Error = r.Error;
}

void initFlash() {
#if ONE_TIME==1
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

	HAL_StatusTypeDef s = HAL_FLASH_Unlock();
	//assert(s==HAL_OK);
	uint16_t loc = 0;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION, 0xDCDC);
	loc += 2;
	static const unsigned int defaults1 = 0b00100001; //screen saver type = 1 sleep time = 2
	static const unsigned int defaults2 = 0b00000001; //screen saver time = 1
	unsigned char reserveFlags = 0; // makeUber == 1 ? 0x1 : 0x0;
	uint16_t ReserveContacts = ((reserveFlags) << 8) | 0x0;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION + loc, ReserveContacts);
	uint16_t Settings = (defaults1 << 8) | defaults2;
	loc += 2;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION + loc, Settings);
	loc += 2;
	uint8_t RadioID[2] = { 0x3, 0x4 };
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION + loc, *((uint32_t*) &RadioID[0]));
	loc += 2;
	uint8_t privateKey[ContactStore::PRIVATE_KEY_LENGTH] = { 0xab, 0x34, 0x4e, 0x58, 0x3f, 0x2a, 0x56, 0x39, 0x17, 0xef, 0x5c, 0xff, 0x8b,
			0xf8, 0x72, 0xe8, 0x87, 0x65, 0xd5, 0x11, 0x26, 0x58, 0x14, 0xb4};
	for (int i = 0; i < ContactStore::PRIVATE_KEY_LENGTH/2; i++, loc += 2) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION + loc, *((uint16_t*) &privateKey[i]));
	}
	uint8_t agentName[ContactStore::AGENT_NAME_LENGTH] = { 0x0 };
	for (int i = 0; i < ContactStore::AGENT_NAME_LENGTH/2; i++, loc += 2) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION + loc, *((uint16_t*) &agentName[i]));
	}

	HAL_FLASH_Lock();
#pragma GCC diagnostic pop
#endif
}

StateBase *CurrentState = 0;

uint32_t startBadge() {
	uint32_t retVal = 0;
	initFlash();

	GUI_ListItemData items[4];
	GUI_ListData DrawList((const char *) "Self Check", items, uint8_t(0), uint8_t(0), uint8_t(128),
			uint8_t(64), uint8_t(0), uint8_t(0));
	//DO SELF CHECK
	if (gui_init()) {
		items[0].set(0, "OLED_INIT");
		DrawList.ItemsCount++;
		retVal |= COMPONENTS_ITEMS::OLED;
		gui_set_curList(&DrawList);
	}
	gui_draw();
#if 1
#define INITIAL_STATE 6
	if (Radio.initialize(RF69_915MHZ, 1)) {
#elif 0
#define INITIAL_STATE 7
		if(Radio.initialize(RF69_915MHZ, 2)) {
#else
#define INITIAL_STATE 1
#endif
		items[1].set(1, "RADIO INIT");
		Radio.setPowerLevel(31);
		retVal |= COMPONENTS_ITEMS::RADIO;
	} else {
		items[1].set(1, "RADIO FAILED");
	}
	DrawList.ItemsCount++;
	gui_draw();

	if (MyContacts.init()) {
		items[2].set(2, "Flash mem INIT");
		retVal |= COMPONENTS_ITEMS::FLASH_MEM;
	} else {
		items[2].set(2, "Flash mem FAILED");
	}
	//test for IR??
	DrawList.ItemsCount++;
	gui_draw();


	//remove nextStateswitch
	//nextStateSwitchTime = HAL_GetTick() + 5000;
	//move to use of IR
	//initUARTIR();

	////////
	state = INITIAL_STATE;
	gui_set_curList(0);
	CurrentState = StateFactory::getMenuState();
	return true;
}

int counter = 0;

uint32_t lastSendTime = 0;


void loopBadge() {

	KB.scan();
	ReturnStateContext rsc = CurrentState->run(KB);

	if (rsc.Err.ok()) {
		CurrentState = rsc.NextMenuToRun;
	}
	gui_draw();
}

