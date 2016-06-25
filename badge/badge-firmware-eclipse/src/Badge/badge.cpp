#include "badge.h"
#include "stdint.h"
#include "gui.h"
//#include <RH_RF69.h>
#include <RFM69.h>
#include <Keyboard.h>
#include <KeyStore.h>
#include <tim.h>
#include <usart.h>

GUI_TickerData td("HI FROM STM32", 0, 0, 120, 10);
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

//RH_RF69 Radio(RFM69_SPI_NSS_Pin, RFM69_Interrupt_DIO0_Pin);
RFM69 Radio;

//start at 45K for 19K
//start at 55K for 10
static const uint32_t START_STORAGE_LOCATION = 0x800d400; //0x800B000;
ContactStore MyContacts(START_STORAGE_LOCATION, 0x2710); //0x4E00);

void delay(uint32_t time) {
	HAL_Delay(time);
}

void initUARTIR() {
	HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);
}

void stopUARTIR() {
	HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_2);
}

uint32_t nextStateSwitchTime = 0;

void startBadge() {
#if ONE_TIME==1
	HAL_FLASH_Unlock();
	//FLASH_PageErase(START_STORAGE_LOCATION);
	//HAL_FLASH_PageErase(START_STORAGE_LOCATION);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, START_STORAGE_LOCATION,
			0xDCDC);
	//private key loc
	uint32_t loc = START_STORAGE_LOCATION + ContactStore::RecordInfo::SIZE
			+ ContactStore::Contact::SIZE;
	uint8_t privKey[ContactStore::PRIVATE_KEY_LENGTH] = { 1, 2, 3, 4, 5, 6, 7,
			8, 9, 10, 11, 12 };
	for (int i = 0; i < ContactStore::PRIVATE_KEY_LENGTH; i += 4) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, loc + i,
				(*((uint32_t *) &privKey[i])));
	}
	HAL_FLASH_Lock();
#endif
	gui_init();
	//Radio.init();
	//Radio.setHeaderFrom(1);
	//Radio.setHeaderTo(2);
	//Radio.setHeaderId(0);

#if 0
	Radio.initialize(RF69_915MHZ,1);
#define INITIAL_STATE 6
#elif 0
	Radio.initialize(RF69_915MHZ, 2);
	#define INITIAL_STATE 7
#else
#define INITIAL_STATE 1
#endif
	Radio.setPowerLevel(31);
	char buf[16] = { 0 };
	int result = MyContacts.init();
	sprintf(&buf[0], "Flash init: %d", result);
	gui_text(&buf[0], 0, 40, 0);
	gui_draw();
#if ONE_TIME==1
	if (MyContacts.getRecordInfo().getNumRecords() == 0) {
		tmp++;
	}
	MyContacts.getMyInfo().setUniqueID(1);
	if (MyContacts.getMyInfo().getUniqueID() == 1) {
		tmp++;
	}
	MyContacts.getMyInfo().setAgentname("Demetrius");
	if (0
			== memcmp(MyContacts.getMyInfo().getAgentName(), "Demetrius",
					strlen("Demetrius"))) {
		tmp++;
	}
	uint8_t pubkey[ContactStore::PUBLIC_KEY_LENGTH] = { 0 };
	MyContacts.getMyInfo().setPublicKey(pubkey);
	uint8_t *readPrivKey = MyContacts.getMyInfo().getPrivateKey();

	sprintf(&buf[0], "K(priv): %u%u%u%u%u%u%u%u%u%u%u%u", privKey[0],
			privKey[1], privKey[2], privKey[3], privKey[4], privKey[5],
			privKey[6], privKey[7], privKey[8], privKey[9], privKey[10],
			privKey[11]);
	gui_text(&buf[0], 0, 40, 0);
	gui_draw();

#endif

	//Radio.send((const uint8_t*)"HI",strlen("HI"));
	//Radio.setModeIdle();
	//Radio.send(2,"hi",2,true);
	//printf("Here");
	//gui_ticker(&td);
	//td.bg
	uint32_t f = Radio.getFrequency();
	sprintf(&buf[0], "Fre:  %d", f);
	gui_text(&buf[0], 0, 20, 0);
	//Radio.readAllRegs();
	sprintf(&buf[0], "rssi: %d", Radio.readRSSI(false));
	nextStateSwitchTime = HAL_GetTick() + 5000;
	initUARTIR();

	////////
	state = INITIAL_STATE;
}

int counter = 0;

void checkStateTimer(int nextState, int timeToNextSwitch) {
	if (nextStateSwitchTime < HAL_GetTick()) {
		state = nextState;
		nextStateSwitchTime = HAL_GetTick() + timeToNextSwitch;
	}
}

uint32_t lastSendTime = 0;

void loopBadge() {
	char buf[128];

	//gui_text(&sendingBuf[0],0,20,0);
	//gui_text(&receivingBuf[0],0,30,0);
	KB.scan();

	switch (state) {
	case 0: {
		gui_ticker(&td);
		td.x++;
		if (td.x > 127)
			td.x = 0;
		checkStateTimer(1, 2000);
	}
		break;
	case 1: // default deplay
	{
		sprintf(&buf[0], "UID: %d\n%s\n%s",
				MyContacts.getMyInfo().getUniqueID(),
				MyContacts.getMyInfo().getAgentName(), "HPUB");
		gui_lable_multiline(&buf[0], 0, 10, 120, 50, SSD1306_COLOR_BLACK, 0);
		checkStateTimer(2, 10000);
	}
		break;
	case 2: {
		sprintf(&buf[0], "Key Pushed: %d\nKey Sel:%d", KB.getLastPinPushed(),
				KB.getLastPinSeleted());
		gui_lable_multiline(&buf[0], 0, 10, 120, 50, SSD1306_COLOR_BLACK, 0);
		checkStateTimer(2, 10000);
	}
		break;
	case 5: {
		sprintf(sendingBuf, "Case 0:");
		if (nextStateSwitchTime == 0) {
			nextStateSwitchTime = HAL_GetTick() + 2000;
		}
		if (nextStateSwitchTime < HAL_GetTick()) {
			state = 1;
		}
	}
		break;
	case 6:
		//HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		if (HAL_GetTick() - lastSendTime > 1000) {
			lastSendTime = HAL_GetTick();
			sprintf(sendingBuf, "Sending 'Hi for the %d time'", counter++);
			//Radio.send((const uint8_t*)&sendingBuf[0],strlen(sendingBuf));
			Radio.send(2, sendingBuf, strlen(sendingBuf), true);

			gui_lable_multiline(sendingBuf, 0, 10, 120, 50, SSD1306_COLOR_BLACK,
					0);
		}
		nextStateSwitchTime = HAL_GetTick() + 10000;
		HAL_Delay(1000);
		state = 6;
		break;
	case 7: {
		if(HAL_GetTick() - lastSendTime > 1000) {
			sprintf(receivingBuf, "Receiving for %d time", counter++);
			lastSendTime = HAL_GetTick();
			gui_lable_multiline(receivingBuf, 0, 20, 120, 50, SSD1306_COLOR_BLACK,
					0);
			if (Radio.receiveDone()) {
				memset(&receivingBuf[0], 0, sizeof(receivingBuf));
				//Radio.recv((uint8_t*)&receivingBuf[0],&bufSize);
				gui_lable_multiline(receivingBuf, 0, 30, 120, 50,
						SSD1306_COLOR_BLACK, 0);
				nextStateSwitchTime = HAL_GetTick() + 4000;
				state = 7;
			} else {
				if (nextStateSwitchTime < HAL_GetTick()) {
					state = 7;
					nextStateSwitchTime = HAL_GetTick() + 2000;
					gui_lable_multiline("timeout", 0, 30, 120, 50,
							SSD1306_COLOR_BLACK, 0);
				}
			}
			HAL_Delay(1000);
		}
	}
		break;
	case 8: {
		gui_lable_multiline("receiver", 0, 10, 120, 50, SSD1306_COLOR_BLACK, 0);
		HAL_StatusTypeDef status;
		char receiveBuf[64] = { '\0' };
		status = HAL_UART_Receive(&huart2, (uint8_t *) &receiveBuf[0],
				sizeof(receiveBuf) - 1, 5000);
		if (status == HAL_OK
				|| (status == HAL_TIMEOUT
						&& huart2.RxXferCount != huart2.RxXferSize)) {
			//Error_Handler();
			sprintf(&buf[0], "UART:  %s", &receiveBuf[0]);
			gui_lable_multiline(&buf[0], 0, 30, 120, 50, SSD1306_COLOR_BLACK,
					0);
		}
	}
		break;
	case 9: {
		gui_lable_multiline("transmitter", 0, 10, 120, 50, SSD1306_COLOR_BLACK,
				0);
		if (KB.getLastPinSeleted() != QKeyboard::NO_PIN_SELECTED) {
			state = 9;
		}
		char transbuf[64] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
		//strcpy(&transbuf[0], "AAAAAAAA");
		HAL_StatusTypeDef status;
		status = HAL_UART_Transmit(&huart2, (uint8_t*) &transbuf[0], 10, 5000);
		if (HAL_OK == status) {
			gui_lable_multiline("sent ok", 0, 10, 120, 50, SSD1306_COLOR_BLACK,
					0);
		}
		delay(500);

	}
		break;
	case 10: {
		HAL_GetDEVID();
		HAL_GetREVID();
		HAL_GetHalVersion();
	}
		break;
	}

	//const char * str = "HI STM32";
	//gui_text(str,10,10,SSD1306_COLOR_WHITE);
	gui_draw();
}

