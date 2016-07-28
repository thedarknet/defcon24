#include "badge.h"
#include "stdint.h"
#include "gui.h"
#include "menus.h"
#include <RFM69.h>
#include <Keyboard.h>
#include <KeyStore.h>
#include <tim.h>
#include <usart.h>
#include "menus/irmenu.h"

char sendingBuf[64] = { '\0' };
char receivingBuf[64] = { '\0' };

#define ONE_TIME 0

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

RFM69 Radio(RFM69_SPI_NSS_Pin, RFM69_Interrupt_DIO0_Pin, true);

static const uint16_t SETTING_SECTOR = 57; //0x800e400
static const uint16_t FIRST_CONTACT_SECTOR = SETTING_SECTOR + 1; //0x800e800
static const uint16_t NUM_CONTACT_SECTOR = 64 - FIRST_CONTACT_SECTOR;
static const uint32_t MY_INFO_ADDRESS = 0x800FFD4;

ContactStore MyContacts(SETTING_SECTOR, FIRST_CONTACT_SECTOR, NUM_CONTACT_SECTOR, MY_INFO_ADDRESS); //0x2710); //0x4E00);

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
	/*
	 HAL_FLASH_Unlock();
	 FLASH_EraseInitTypeDef EraseInitStruct;
	 EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	 EraseInitStruct.Banks = FLASH_BANK_1;
	 EraseInitStruct.PageAddress = FLASH_BASE + (57 * FLASH_PAGE_SIZE);
	 EraseInitStruct.NbPages = 1;
	 uint32_t SectorError = 0;

	 //if(FLASH_PageErase(FLASH_BASE + ( 57 * FLASH_PAGE_SIZE))==HAL_OK) {
	 if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK) {
	 HAL_FLASH_Lock();
	 HAL_FLASH_Unlock();
	 HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 0x800e400, 0xDCDC);
	 HAL_FLASH_Lock();
	 uint16_t v = *((uint16_t *) 0x800e400);
	 char b[10];
	 sprintf(&b[0], "%d", v);

	 HAL_FLASH_Unlock();
	 HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 0x800e400, 0x0);
	 HAL_FLASH_Lock();
	 v = *((uint16_t *) 0x800e400);
	 sprintf(&b[0], "%d", v);
	 HAL_FLASH_Unlock();
	 HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 0x800e400, 0xDCDC);
	 HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 0x800e402, 0xDCDC);
	 HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 0x800e412, 0xDCDC);
	 HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 0x800e4A2, 0xDCDC);
	 }

	 HAL_FLASH_Lock();
	 */
#if ONE_TIME==1
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

	HAL_StatusTypeDef s = HAL_FLASH_Unlock();
	//assert(s==HAL_OK);
	uint16_t loc = 0;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION, 0xDCDC);
	loc += 2;
	static const unsigned int defaults1 = 0b00100001;//screen saver type = 1 sleep time = 2
	static const unsigned int defaults2 = 0b00000001;//screen saver time = 1
	unsigned char reserveFlags = 0;// makeUber == 1 ? 0x1 : 0x0;
	uint16_t ReserveContacts = ((reserveFlags) << 8) | 0x0;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION + loc, ReserveContacts);
	uint16_t Settings = (defaults1 << 8) | defaults2;
	loc += 2;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION + loc, Settings);
	loc += 2;
	uint8_t RadioID[2] = {0x3, 0x4};
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION + loc, *((uint32_t*) &RadioID[0]));
	loc += 2;
	uint8_t privateKey[ContactStore::PRIVATE_KEY_LENGTH] = {0xab, 0x34, 0x4e, 0x58, 0x3f, 0x2a, 0x56, 0x39, 0x17, 0xef, 0x5c, 0xff, 0x8b,
		0xf8, 0x72, 0xe8, 0x87, 0x65, 0xd5, 0x11, 0x26, 0x58, 0x14, 0xb4};
	for (int i = 0; i < ContactStore::PRIVATE_KEY_LENGTH/2; i++, loc += 2) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION + loc, *((uint16_t*) &privateKey[i]));
	}
	uint8_t agentName[ContactStore::AGENT_NAME_LENGTH] = {0x0};
	for (int i = 0; i < ContactStore::AGENT_NAME_LENGTH/2; i++, loc += 2) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION + loc, *((uint16_t*) &agentName[i]));
	}

	HAL_FLASH_Lock();
#pragma GCC diagnostic pop
#endif
}

StateBase *CurrentState = 0;
static const uint32_t TIME_BETWEEN_INITS = 100;

uint32_t startBadge() {
	uint32_t retVal = 0;
	initFlash();

	GUI_ListItemData items[4];
	GUI_ListData DrawList((const char *) "Self Check", items, uint8_t(0), uint8_t(0), uint8_t(128), uint8_t(64),
			uint8_t(0), uint8_t(0));
	//DO SELF CHECK
	if (gui_init()) {
		delay(1000);
		items[0].set(0, "OLED_INIT");
		DrawList.ItemsCount++;
		retVal |= COMPONENTS_ITEMS::OLED;
		gui_set_curList(&DrawList);
	}
	gui_draw();
	delay(TIME_BETWEEN_INITS);
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
	delay(TIME_BETWEEN_INITS);

	if (MyContacts.init()) {
		items[2].set(2, "Flash mem INIT");
		retVal |= COMPONENTS_ITEMS::FLASH_MEM;
	} else {
		items[2].set(2, "Flash mem FAILED");
	}
	//test for IR??
	DrawList.ItemsCount++;
	delay(TIME_BETWEEN_INITS);
	gui_draw();
	delay(1000);

	gui_set_curList(0);

	gui_lable_multiline("#dcdn16", 0, 10, 128, 64, 0, 0);
	gui_lable_multiline("><>", 0, 40, 128, 64, 0, 0);
	gui_lable_multiline("Cyberez Inc", 0, 50, 128, 64, 0, 0);
	gui_draw();
	delay(3000);

	////////
	state = INITIAL_STATE;

	StateFactory::getIRPairingState()->BeTheBob();
	CurrentState = StateFactory::getMenuState();
	return true;
}

void loopBadge() {

	//check to see if keyboard should be ignored
	uint32_t tick = HAL_GetTick();
	KB.scan();

	ReturnStateContext rsc = CurrentState->run(KB);

	if (rsc.Err.ok()) {
		if (CurrentState != rsc.NextMenuToRun) {
			//on state switches reset keyboard and give a 1 second pause on reading from keyboard.
			KB.reset();
			//did we just switch to the IR pairing state?
		}
		CurrentState = rsc.NextMenuToRun;
	}
	StateFactory::getIRPairingState()->ListenForAlice();
	//configure 1 time listen RegListen1, RegListen2, RegListen3
	//defaults are good except for ListenCriteria should be 1 not 0

	//how to stop listen mode
	// Program RegOpMode with ListenOn=0, ListenAbort=1, and the desired setting for the Mode bits (Sleep, Stdby, FS, Rx
	//	or Tx mode) in a single SPI access
	//Program RegOpMode with ListenOn=0, ListenAbort=0, and the desired setting for the Mode bits (Sleep, Stdby, FS, Rx
	//	or Tx mode) in a second SPI access
	//
	// Cycle:
	//	Set mode to standby with listen on
	//		If interrupt fires grab data, stop listen mode, start listen mode.
	//	if you need to transmit
	//		stop listen mode
	//		Transmit
	//		go back into listen mode
	//goto standby and listen

	//getRadio().setListen();

	gui_draw();
}

