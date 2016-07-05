#include "stm32f1xx_hal.h"
#include "menus.h"
#include <tim.h>
#include <RFM69.h>
#include <uECC.h>
#include <sha256.h>

StateBase::StateBase() :
		StateData(0), StateStartTime(0) {
}

ReturnStateContext StateBase::run(QKeyboard &kb) {
	ReturnStateContext sr(this);
	if (!hasBeenInitialized()) {
		ErrorType et = init();
		if (!et.ok()) {
			sr.NextMenuToRun = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), et.getMessage(),
					10000);
		}
	} else {
		sr = onRun(kb);
		if (sr.NextMenuToRun != this) {
			shutdown();
		}
	}
	return sr;
}

StateBase::~StateBase() {
}

ErrorType StateBase::init() {
	ErrorType et = onInit();
	if (et.ok()) {
		setState(INIT_BIT);
		StateStartTime = HAL_GetTick();
		return true;
	}
	return false;
}
ErrorType StateBase::shutdown() {
	ErrorType et = onShutdown();
	clearState(INIT_BIT);
	StateStartTime = 0;
	return et;
}

uint32_t StateBase::timeInState() {
	return HAL_GetTick() - StateStartTime;
}

//=======================================================================
LogoState::LogoState(uint16_t timeInState) :
		td("HI FROM STM32", 0, 0, 120, 10), TimeInLogoState(timeInState) {
}

ErrorType LogoState::onInit() {
	return ErrorType();
}

ReturnStateContext LogoState::onRun(QKeyboard &kb) {
	gui_ticker(&td);
	td.x++;
	if (td.x > 127)
		td.x = 0;
	if (timeInState() > TimeInLogoState || kb.getLastPinSeleted() != QKeyboard::NO_PIN_SELECTED) {
		return ReturnStateContext(StateFactory::getMenuState());
	} else {
		return ReturnStateContext(this);
	}
}

ErrorType LogoState::onShutdown() {
	return true;
}

LogoState::~LogoState() {
}

DisplayMessageState::DisplayMessageState(uint16_t timeInState, StateBase *nextState) :
		TimeInState(timeInState), NextState(nextState) {
}

DisplayMessageState::~DisplayMessageState() {
}

ErrorType DisplayMessageState::onInit() {
	return ErrorType();
}

void DisplayMessageState::setMessage(const char *msg) {
	strncpy(&this->Message[0], msg, sizeof(this->Message));
}

ReturnStateContext DisplayMessageState::onRun(QKeyboard &kb) {
	gui_lable_multiline(&this->Message[0], 0, 10, 120, 50, SSD1306_COLOR_BLACK, 0);
	if (timeInState() > TimeInState || kb.getLastPinSeleted() != QKeyboard::NO_PIN_SELECTED) {
		return ReturnStateContext(StateFactory::getMenuState());
	}
	return ReturnStateContext(this);
}

ErrorType DisplayMessageState::onShutdown() {
	return true;
}

MenuState::MenuState() :
		StateBase(), MenuList("Main Menu", (GUI_ListItemData**) Items, 0, 10, 128, 64, 0,
				(sizeof(Items) / sizeof(Items[0]))) {
	Items[0].id = 0;
	Items[0].text = (const char *) "Settings";
	Items[1].id = 1;
	Items[1].text = (const char *) "IR Pair";
	Items[2].id = 2;
	Items[2].text = (const char *) "Address Book";
	Items[3].id = 3;
	Items[3].text = (const char *) "Send Message";
	Items[4].id = 4;
	Items[4].text = (const char *) "Enigma";
	Items[5].id = 5;
	Items[5].text = (const char *) "Badge Info";
	Items[6].id = 6;
	Items[6].text = (const char *) "Radio Info";
}

MenuState::~MenuState() {

}

ErrorType MenuState::onInit() {
	gui_set_curList(&MenuList);
	return ErrorType();
}

ReturnStateContext MenuState::onRun(QKeyboard &kb) {
	uint8_t key = kb.getLastPinSeleted();
	StateBase *nextState = this;
	switch (key) {
	case 1: {
		if (MenuList.selectedItem == 0) {
			MenuList.selectedItem = sizeof(Items) / sizeof(Items[0]) - 1;
		} else {
			MenuList.selectedItem--;
		}
		break;
	}
	case 7: {
		if (MenuList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
			MenuList.selectedItem = 0;
		} else {
			MenuList.selectedItem++;
		}
		break;
	}
	case 9: {
		MenuList.selectedItem = 0;
	}
		break;
	case 11: {
		switch (MenuList.selectedItem) {
		case 0:
			nextState = StateFactory::getSettingState();
			break;
		case 1:
			nextState = StateFactory::getIRPairingState();
			break;
		case 2:
			nextState = StateFactory::getAddressBookState();
			break;
		case 3:
			nextState = StateFactory::getSendMessageState();
			break;
		case 4:
			nextState = StateFactory::getEnigmaState();
			break;
		case 5:
			nextState = StateFactory::getBadgeInfoState();
			break;
		case 6:
			nextState = StateFactory::getRadioInfoState();
			break;
		}
	}
		break;
	}
	return ReturnStateContext(nextState);
}

ErrorType MenuState::onShutdown() {
	gui_set_curList(0);
	MenuList.selectedItem = 0;
	return ErrorType();
}

SettingState::SettingState() :
		StateBase(), SettingList((const char *) "MENU", (GUI_ListItemData**) &Items, 0, 0, 128, 64, 0,
				sizeof(Items) / sizeof(Items[0])), InputPos(0), SubState(0) {

	memset(&AgentName[0], 0, sizeof(AgentName));
	Items[0].id = 0;
	Items[0].text = (const char *) "Set Agent Name";
	Items[1].id = 1;
	Items[1].text = (const char *) "Set Screen Saver";
	Items[2].id = 2;
	Items[2].text = (const char *) "Set Sleep Time";
	Items[3].id = 3;
	Items[3].text = (const char *) "UNKNOWN";
}

SettingState::~SettingState() {

}

ErrorType SettingState::onInit() {
	gui_set_curList(&SettingList);
	SubState = 0;
	return ErrorType();
}

ReturnStateContext SettingState::onRun(QKeyboard & kb) {
	uint8_t key = kb.getLastPinSeleted();
	StateBase *nextState = this;
	switch (SubState) {
	case 0:
		switch (key) {
		case 1: {
			if (SettingList.selectedItem == 0) {
				SettingList.selectedItem = sizeof(Items) / sizeof(Items[0]) - 1;
			} else {
				SettingList.selectedItem--;
			}
			break;
		}
		case 7: {
			if (SettingList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
				SettingList.selectedItem = 0;
			} else {
				SettingList.selectedItem++;
			}
			break;
		}
		case 9: {
			nextState = StateFactory::getMenuState();
		}
			break;
		case 11: {
			SubState = SettingList.selectedItem + 100;
		}
			break;
		default:
			break;
		}
		break;
	case 100: {
		gui_lable((const char*) "Enter you're agent name:", 0, 10, 128, 64, 0, 0);
		char letter = kb.getLetter();
		if (letter == 127) {
			AgentName[InputPos] = '\0';
			InputPos--;
		} else if (letter == 126) {
			//done
			getContactStore().getMyInfo().setAgentname(&AgentName[0]);
			gui_lable((const char *) "Saving ...", 0, 30, 128, 64, 0, 0);
			nextState = StateFactory::getMenuState();
		} else {
			AgentName[InputPos++] = letter;
		}
		gui_lable(&AgentName[0], 0, 20, 128, 64, 0, 0);
		break;
	}
	case 101: {
		gui_lable((const char*) "Choose Number of Screen Saver:", 0, 10, 128, 64, 0, 0);
		gui_lable((const char*) "1: Game of Life", 0, 20, 128, 64, 0, 0);
		uint8_t ss = kb.getLastPinSeleted();
		if (ss == 1) {
			getContactStore().getRecordInfo().setScreenSaverType(ss);
		}
		break;
	}
	case 102:
		gui_lable((const char*) "Time until badge goes to sleep:", 0, 10, 128, 64, 0, 0);
		if (kb.getLastPinSeleted() == 9) {
			AgentName[InputPos] = '\0';
			InputPos--;
		} else if (kb.getLastPinSeleted() == 11) {
			//done
			getContactStore().getRecordInfo().setSleepTime(atoi(&AgentName[0]));
			gui_lable((const char *) "Saving ...", 0, 30, 128, 64, 0, 0);
			nextState = StateFactory::getMenuState();
		} else {
			AgentName[InputPos++] = kb.getNumber();
		}
		gui_lable(&AgentName[0], 0, 20, 128, 64, 0, 0);
		break;
	case 103:
		break;
	}
	return ReturnStateContext(nextState);
}

ErrorType SettingState::onShutdown() {
	InputPos = 0;
	gui_set_curList(0);
	memset(&AgentName[0], 0, sizeof(AgentName));
	return ErrorType();
}

IRState::IRState() :
		TimeoutOnSync(10000) {

}

IRState::~IRState() {

}

ErrorType IRState::onInit() {
	HAL_StatusTypeDef status = HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);
	if (HAL_OK == status) {
		return ErrorType();
	}
	return ErrorType(ErrorType::TIMER_ERROR);
}

ReturnStateContext IRState::onRun(QKeyboard &kb) {
	uint8_t otherRadioID[2] = { 0x00, 0x00 };
	uint8_t otherCompressedPubKey[ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH] = { 0x00 };
	uint8_t otherSignature[ContactStore::SIGNATURE_LENGTH] = { 0x00 };
	//sign their radio id + compressed public key
	ShaOBJ shaObj;
	sha256_add(&shaObj, &otherRadioID[0], sizeof(otherRadioID));
	sha256_add(&shaObj, &otherCompressedPubKey[0], sizeof(otherCompressedPubKey));
	uint8_t digest[32];
	sha256_digest(&shaObj, digest);
	uint8_t signature[ContactStore::SIGNATURE_LENGTH];
	if (uECC_sign(getContactStore().getMyInfo().getPrivateKey(), &digest[0], sizeof(digest), signature, THE_CURVE)
			== 1) {
	}
	//verify what you get back
	//they are signing you're radio id and your compressed public key
	uint8_t myCompressedPubKey[ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH];
	uECC_compress(getContactStore().getMyInfo().getPublicKey(), &myCompressedPubKey[0], THE_CURVE);
	ShaOBJ shactx;
	uint16_t radioID = getContactStore().getMyInfo().getUniqueID();
	sha256_add(&shactx, (unsigned char*) &radioID, 2);
	sha256_add(&shactx, &myCompressedPubKey[0], sizeof(myCompressedPubKey));
	sha256_digest(&shaObj, digest);
	uint8_t otherUnCompressedPubKey[ContactStore::PUBLIC_KEY_LENGTH];
	uECC_decompress(&otherCompressedPubKey[0], &otherUnCompressedPubKey[0], THE_CURVE);
	if (uECC_verify(&otherUnCompressedPubKey[0], &digest[0], sizeof(digest), otherSignature, THE_CURVE) == 1) {
		//we have validated what we got back
	}

	gui_lable_multiline("not implemented yet", 0, 20, 120, 50, SSD1306_COLOR_BLACK, 0);
	gui_lable_multiline("press a key to return", 0, 30, 120, 50, SSD1306_COLOR_BLACK, 0);
	if (kb.getLastPinSeleted() == QKeyboard::NO_PIN_SELECTED) {
		return ReturnStateContext(this);
	} else {
		return ReturnStateContext(StateFactory::getMenuState());
	}
#if 0
	gui_lable_multiline("receiver", 0, 10, 120, 50, SSD1306_COLOR_BLACK, 0);
	HAL_StatusTypeDef status;
	char receiveBuf[64] = {'\0'};
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
	char transbuf[64] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	//strcpy(&transbuf[0], "AAAAAAAA");
	HAL_StatusTypeDef status;
	status = HAL_UART_Transmit(&huart2, (uint8_t*) &transbuf[0], 10, 5000);
	if (HAL_OK == status) {
		gui_lable_multiline("sent ok", 0, 10, 120, 50, SSD1306_COLOR_BLACK,
				0);
#endif
}

ErrorType IRState::onShutdown() {
	HAL_StatusTypeDef status = HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_2);
	if (HAL_OK == status) {
		return ErrorType();
	}
	return ErrorType(ErrorType::TIMER_ERROR);
}

////////////////////////////////////////////////
AddressState::AddressState() :
		StateBase(), AddressList((const char *) "Address Book", (GUI_ListItemData**) &Items, 0, 0, 128, 64, 0,
				sizeof(Items) / sizeof(Items[0])), ContactIndex(0) {

}

AddressState::~AddressState() {

}

ErrorType AddressState::onInit() {
	gui_set_curList(&AddressList);
	setNext4Items(0);
	return ErrorType();
}

void AddressState::setNext4Items(uint16_t startAt) {
	uint8_t num = getContactStore().getRecordInfo().getNumContacts();
	for (uint16_t i = startAt, j = 0; j < (4); i++, j++) {
		ContactStore::Contact c(0); // 0 is illegal memory
		if (i < num) {
			getContactStore().getRecordInfo().getContactAt(i, c);
			Items[j].id = c.getUniqueID();
			Items[j].text = c.getAgentName();
		} else {
			Items[j].id = 0;
			Items[j].text = "";
		}
	}
}

ReturnStateContext AddressState::onRun(QKeyboard &kb) {
	uint8_t pin = kb.getLastPinSeleted();
	StateBase *nextState = this;
	switch (pin) {
	case 1:
		if (AddressList.selectedItem == 0) {
			//keep selection at 0 but load new values
			uint16_t startAt = Items[AddressList.selectedItem].id;
			if (startAt > 0) {
				setNext4Items(startAt - 1);
			}
		} else {
			AddressList.selectedItem--;
		}
		break;
	case 7:
		if (AddressList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
			setNext4Items(Items[AddressList.selectedItem].id + 1);
		} else {
			AddressList.selectedItem++;
		}
		break;
	case 9:
		nextState = StateFactory::getMenuState();
		break;
	case 11:
		nextState = StateFactory::getSendMessageState();
		((SendMsgState*) nextState)->setContactToMessage(Items[AddressList.selectedItem].id);
		break;
	}
	return ReturnStateContext(nextState);
}

ErrorType AddressState::onShutdown() {
	gui_set_curList(0);
	return ErrorType();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
SendMsgState::SendMsgState() :
		StateBase(), ContactID(SendMsgState::NO_CONTACT), MsgBuffer(), InputPos(0), InternalState(TYPE_MESSAGE) {

}
SendMsgState::~SendMsgState() {

}
void SendMsgState::setContactToMessage(const uint8_t &cid) {
	ContactID = cid;
}

ErrorType SendMsgState::onInit() {
	if (shouldReset()) {
		memset(&MsgBuffer[0], 0, sizeof(MsgBuffer));
		InputPos = 0;
	} else {
		clearState(DONT_RESET);
	}
	InternalState = TYPE_MESSAGE;
	return ErrorType();
}

ReturnStateContext SendMsgState::onRun(QKeyboard &kb) {
	StateBase *nextState = this;
	switch (InternalState) {
	case TYPE_MESSAGE: {
		gui_lable("Send Message: ", 0, 10, 128, 64, 0, 0);
		gui_lable_multiline(&MsgBuffer[0], 0, 20, 128, 64, 0, 0);
		//keyboard entry
		char letter = kb.getLetter();
		if (letter != 0) {
			MsgBuffer[InputPos] = letter;
			if (InputPos >= sizeof(MsgBuffer)) {
				InputPos = sizeof(MsgBuffer) - 1;
			}
		} else {
			uint8_t pin = kb.getLastPinSeleted();
			if (pin == 11) { //return has been pushed
				InternalState = INTERNAL_STATE::CONFIRM_SEND;
			} else if (pin == 9) {
				if (InputPos > 0) {
					InputPos--;
				}
			}
		}
	}
		break;
	case CONFIRM_SEND: {
		gui_lable("Send by pressing #", 0, 10, 128, 64, 0, 0);
		gui_lable_multiline(&MsgBuffer[0], 0, 20, 128, 64, 0, 0);
		uint8_t pin = kb.getLastPinSeleted();
		if (pin == 9) {
			InternalState = TYPE_MESSAGE;
		} else if (pin == 11) {
			InternalState = INTERNAL_STATE::SENDING;
		}
	}
		break;
	case SENDING: {
		char buf[32];
		ContactStore::Contact c(0);
		if (getContactStore().getRecordInfo().getContactAt(this->ContactID, c)) {
			sprintf(&buf[0], "Sending Message to: %s", c.getAgentName());
			gui_lable_multiline(&buf[0], 0, 10, 128, 64, 0, 0);
			if (getRadio().sendWithRetry(c.getUniqueID(), &MsgBuffer[0], strlen(&MsgBuffer[0]), 3, 100)) {
				nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(),
						"Message Sent Successfully!", 5000);
			} else {
				nextState = StateFactory::getDisplayMessageState(StateFactory::getSendMessageState(),
						"Failed to send to address!", 10000);
				setState(DONT_RESET);
			}
		} else {
			nextState = StateFactory::getDisplayMessageState(StateFactory::getAddressBookState(),
					"Could not find Contact!", 10000);
		}
	}
		break;
	}
	return ReturnStateContext(nextState);
}

ErrorType SendMsgState::onShutdown() {
	if (shouldReset()) {
		ContactID = NO_CONTACT;
	}
	return ErrorType();
}

////////////////////////////////////////////////////////////
EngimaState::EngimaState() :
		InternalState(SET_WHEELS), EntryBuffer(), Wheels() {

}
EngimaState::~EngimaState() {

}

ErrorType EngimaState::onInit() {
	memset(&EntryBuffer[0], 0, sizeof(EntryBuffer));
	memset(&Wheels[0], 0, sizeof(Wheels));
	InternalState = SET_WHEELS;
	return ErrorType();
}

ReturnStateContext EngimaState::onRun(QKeyboard &kb) {
	StateBase* nextState = this;
	gui_lable("not implemented", 0, 10, 128, 64, 0, 0);
	uint8_t pin = kb.getLastPinSeleted();
	if (pin == 11) {
		nextState = StateFactory::getMenuState();
	}
	return ReturnStateContext(nextState);
}

ErrorType EngimaState::onShutdown() {
	return ErrorType();
}

//////////////////////////////////////////////////////////////

BadgeInfoState::BadgeInfoState() :
		StateBase(), BadgeInfoList("Badge Info:", (GUI_ListItemData**) Items, 0, 0, 128, 64, 0,
				(sizeof(Items) / sizeof(Items[0]))) {

	for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
		Items[i].id = i;
	}
	//char ListBuffer[20][6];
}

BadgeInfoState::~BadgeInfoState() {

}

ErrorType BadgeInfoState::onInit() {
	gui_set_curList(&BadgeInfoList);
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
	sprintf(&ListBuffer[0][0], "SW Version: %s", "1.0.0");
	sprintf(&ListBuffer[1][0], "DEVID: %lu", HAL_GetDEVID());
	sprintf(&ListBuffer[2][0], "REVID: %lu", HAL_GetREVID());
	sprintf(&ListBuffer[3][0], "HAL Version: %lu", HAL_GetHalVersion());
	sprintf(&ListBuffer[4][0], "UID: %u", getContactStore().getMyInfo().getUniqueID());
	sprintf(&ListBuffer[5][0], "Num contacts: %u", getContactStore().getRecordInfo().getNumContacts());
	for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
		Items[i].text = &ListBuffer[i][0];
	}
	return ErrorType();
}

ReturnStateContext BadgeInfoState::onRun(QKeyboard &kb) {
	uint8_t key = kb.getLastPinSeleted();
	StateBase *nextState = this;
	switch (key) {
	case 1: {
		if (BadgeInfoList.selectedItem == 0) {
			BadgeInfoList.selectedItem = sizeof(Items) / sizeof(Items[0]) - 1;
		} else {
			BadgeInfoList.selectedItem--;
		}
		break;
	}
	case 7: {
		if (BadgeInfoList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
			BadgeInfoList.selectedItem = 0;
		} else {
			BadgeInfoList.selectedItem++;
		}
		break;
	}
	case 9: {
		nextState = StateFactory::getMenuState();
	}
		break;
	}
	return ReturnStateContext(nextState);
}

ErrorType BadgeInfoState::onShutdown() {
	gui_set_curList(0);
	return ErrorType();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

RadioInfoState::RadioInfoState() :
		StateBase(), RadioInfoList("Radio Info:", (GUI_ListItemData**) Items, 0, 0, 128, 64, 0,
				(sizeof(Items) / sizeof(Items[0]))), Items(), ListBuffer() {

}

RadioInfoState::~RadioInfoState() {

}

ErrorType RadioInfoState::onInit() {
	gui_set_curList(&RadioInfoList);
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
	for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
		Items[i].text = &ListBuffer[i][0];
	}
	return ErrorType();
}

ReturnStateContext RadioInfoState::onRun(QKeyboard &kb) {
	StateBase *nextState = this;
	sprintf(&ListBuffer[0][0], "Frequency: %lu", getRadio().getFrequency());
	sprintf(&ListBuffer[1][0], "RSSI: %u", getRadio().readRSSI());
	sprintf(&ListBuffer[2][0], "RSSI Threshold: %u", getRadio().getRSSIThreshHold());
	sprintf(&ListBuffer[3][0], "Gain: %u", getRadio().getCurrentGain());
	uint8_t pin = kb.getLastPinSeleted();
	if (pin == 9) {
		nextState = StateFactory::getMenuState();
	}
	return ReturnStateContext(nextState);
}

ErrorType RadioInfoState::onShutdown() {
	gui_set_curList(0);
	return ErrorType();
}

//============================================================
LogoState Logo_State(uint16_t(5000));
DisplayMessageState Display_Message_State(3000, 0);
MenuState MenuState;
IRState TheIRPairingState;
SettingState TheSettingState;
EngimaState TheEnginmaState;
AddressState TheAddressState;
SendMsgState TheSendMsgState;
RadioInfoState TheRadioInfoState;
BadgeInfoState TheBadgeInfoState;

bool StateFactory::init() {
	return true;
}

StateBase *StateFactory::getLogoState(uint16_t timeToDisplay) {
	Logo_State.setTimeInLogo(timeToDisplay);
	return &Logo_State;
}

StateBase *StateFactory::getDisplayMessageState(StateBase *bm, const char *message, uint16_t timeToDisplay) {
	Display_Message_State.setMessage(message);
	Display_Message_State.setNextState(bm);
	Display_Message_State.setTimeInState(timeToDisplay);
	return &Display_Message_State;
}

StateBase * StateFactory::getMenuState() {
	return &MenuState;
}

StateBase *StateFactory::getSettingState() {
	return &TheSettingState;
}

StateBase *StateFactory::getIRPairingState() {
	return &TheIRPairingState;
}

StateBase *StateFactory::getAddressBookState() {
	return &TheAddressState;
}

StateBase *StateFactory::getSendMessageState() {
	return &TheSendMsgState;
}

StateBase*StateFactory::getEnigmaState() {
	return &TheEnginmaState;
}

StateBase* StateFactory::getBadgeInfoState() {
	return &TheBadgeInfoState;
}

StateBase* StateFactory::getRadioInfoState() {
	return &TheRadioInfoState;
}

