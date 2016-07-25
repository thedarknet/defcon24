#include "stm32f1xx_hal.h"
#include "menus.h"
#include <tim.h>
#include <RFM69.h>
#include <uECC.h>
#include <sha256.h>
#include "menus/irmenu.h"
#include "menus/GameOfLife.h"
#include "menus/EventState.h"

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
	}
	return et;
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
		StateBase(), MenuList("Main Menu", Items, 0, 10, 128, 64, 0, (sizeof(Items) / sizeof(Items[0]))) {
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
		StateBase(), SettingList((const char *) "MENU", Items, 0, 0, 128, 64, 0, sizeof(Items) / sizeof(Items[0])), InputPos(
				0), SubState(0) {

	memset(&AgentName[0], 0, sizeof(AgentName));
	Items[0].id = 0;
	Items[0].text = (const char *) "Set Agent Name";
	Items[1].id = 1;
	Items[1].text = (const char *) "Set Screen Saver";
	Items[2].id = 2;
	Items[2].text = (const char *) "Set Sleep Time";
	Items[3].id = 3;
	Items[3].text = (const char *) "Reset Badge";
}

SettingState::~SettingState() {

}

KeyBoardLetterCtx KCTX;

ErrorType SettingState::onInit() {
	gui_set_curList(&SettingList);
	SubState = 0;
	return ErrorType();
}

static const char *NUMBERS = "123456789";

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
			gui_set_curList(0);
			gui_draw();
			switch (SubState) {
			case 100:
				memset(&AgentName[0], 0, sizeof(AgentName));
				KCTX.init(&AgentName[0], sizeof(AgentName));
				break;
			case 101:
				sprintf(&AgentName[0],"Current:  %d",getContactStore().getSettings().getScreenSaverType()+1);
				break;
			case 102:
				InputPos = getContactStore().getSettings().getSleepTime();
				break;
			case 103:
				kb.reset();
				break;
			}
		}
			break;
		default:
			break;
		}
		break;
	case 100: {
		gui_lable_multiline((const char*) "Current agent name:", 0, 10, 128, 64, 0, 0);
		if (*getContactStore().getSettings().getAgentName() == '\0') {
			gui_lable_multiline((const char *) "NOT SET", 0, 20, 128, 64, 0, 0);
		} else {
			gui_lable_multiline(getContactStore().getSettings().getAgentName(), 0, 20, 128, 64, 0, 0);
		}
		gui_lable_multiline((const char*) "Set agent name:", 0, 30, 128, 64, 0, 0);
		kb.updateContext(KCTX);
		if (kb.getLastPinSeleted() == 11 && AgentName[0] != '\0' && AgentName[0] != ' ') {
			//done
			if (getContactStore().getSettings().setAgentname(&AgentName[0])) {
				nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Save Successful", 2000);
			} else {
				nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Save FAILED!", 4000);
			}
		} else {
			gui_lable_multiline(&AgentName[0], 0, 40, 128, 64, 0, 0);
		}
		break;
	}
	case 101: {
		gui_lable_multiline((const char*) "Screen Saver:", 0, 10, 128, 64, 0, 0);
		gui_lable_multiline(&AgentName[0], 0, 20, 128, 64, 0, 0);
		gui_lable_multiline((const char*) "1: Game of Life", 0, 30, 128, 64, 0, 0);
		uint8_t ss = kb.getLastPinSeleted();
		switch (ss) {
		case 0:
			if (getContactStore().getSettings().setScreenSaverType(ss)) {
				nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Setting saved", 2000);
			} else {
				nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Save FAILED!", 4000);
			}
			break;
		}
		break;
	}
	case 102:
		gui_lable_multiline((const char*) "Time until badge goes to sleep:", 0, 10, 128, 64, 0, 0);
		if (kb.getLastPinSeleted() == 9 || kb.getLastPinSeleted() == 10
				|| kb.getLastPinSeleted() == QKeyboard::NO_PIN_SELECTED) {
			//InputPos = 4;
		} else if (kb.getLastPinSeleted() == 11) {
			if (getContactStore().getSettings().setSleepTime(InputPos)) {
				nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Setting saved", 2000);
			} else {
				nextState = StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Save FAILED!", 4000);
			}
		} else {
			InputPos = kb.getLastPinSeleted();
		}
		sprintf(&AgentName[0], "%c Minutes", NUMBERS[InputPos]);
		gui_lable_multiline(&AgentName[0], 0, 30, 128, 64, 0, 0);
		break;
	case 103:
		gui_lable_multiline((const char*) "Reset to factory defaults?", 0, 10, 128, 64, 0, 0);
		gui_lable_multiline((const char*) "Press # to Cancel", 0, 30, 128, 64, 0, 0);
		gui_lable_multiline((const char*) "Press enter to do it", 0, 40, 128, 64, 0, 0);
		if(kb.getLastPinSeleted()==9) {
			nextState = StateFactory::getMenuState();
		} else if (kb.getLastPinSeleted()==11){
			getContactStore().resetToFactory();
			nextState = StateFactory::getMenuState();
		}
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

////////////////////////////////////////////////
AddressState::AddressState() :
		StateBase(), AddressList((const char *) "Address Book", Items, 0, 0, 128, 64, 0,
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
	uint8_t num = getContactStore().getSettings().getNumContacts();
	for (uint16_t i = startAt, j = 0; j < (4); i++, j++) {
		ContactStore::Contact c(0); // 0 is illegal memory
		if (i < num) {
			getContactStore().getContactAt(i, c);
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
		KCTX.init(&MsgBuffer[0], sizeof(MsgBuffer));
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
		gui_lable_multiline("Send Message: ", 0, 10, 128, 64, 0, 0);
		gui_lable_multiline(&MsgBuffer[0], 0, 20, 128, 64, 0, 0);
		//keyboard entry
		kb.updateContext(KCTX);
		uint8_t pin = kb.getLastPinSeleted();
		if (pin == 11) { //return has been pushed
			InternalState = INTERNAL_STATE::CONFIRM_SEND;
		}
	}
		break;
	case CONFIRM_SEND: {
		gui_lable_multiline("Send by pressing #", 0, 10, 128, 64, 0, 0);
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
		if (getContactStore().getContactAt(this->ContactID, c)) {
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
		InternalState(SET_WHEELS), EntryBuffer(), Wheels(), EncryptResult() {

}
EngimaState::~EngimaState() {

}

ErrorType EngimaState::onInit() {
	gui_set_curList(0);
	memset(&EntryBuffer[0], 0, sizeof(EntryBuffer));
	memset(&Wheels[0], 0, sizeof(Wheels));
	Wheels[0] = 'A', Wheels[1] = 'B', Wheels[2] = 'C';
	memset(&EncryptResult[0], 0, sizeof(EncryptResult));
	InternalState = SET_WHEELS;
	return ErrorType();
}

ReturnStateContext EngimaState::onRun(QKeyboard &kb) {
	StateBase* nextState = this;
	gui_lable_multiline("EngimaState not implemented", 0, 10, 128, 64, 0, 0);
	uint8_t pin = kb.getLastPinSeleted();
	if (pin == 11) {
		nextState = StateFactory::getMenuState();
	}
	return ReturnStateContext(nextState);
}

const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint32_t NUM_ROTORS = 13;
const char rotors[NUM_ROTORS][27] = { "DVOARQWTUZJCNFLSPMBHEYIGKX", "GHQZUJFWLVMTKOPIRSDEACXYBN",
		"AKUOCLVJYIXMQPERBWSNGFZHTD", "BKLOSUDPJIRHZEXCGQMNVYFATW", "LICFJPORWQVHANKEBUDYMGZXTS",
		"CAWFYLKXSZTGHPINMDREUQBJVO", "PYVREUXHKIWDNQAZTLSMBOJGFC", "LQRHNSTPAFIVJYMDGUOZKECWXB",
		"JAUMCWHXTIZDYORQNSKBEFGLPV", "VRKNGZQOUXTMDIECJYPFSAWBLH", "LUHMZRVEGYSPJFADQCWTKBNXIO",
		"SDIJUOBALVMYRNGWKHPQCXTFZE", "LIVPNYCUGSRFBXKQHMOEWZTDAJ" };

long EngimaState::mod26(long a) {
	return (a % 26 + 26) % 26;
}

int EngimaState::li(char l) {
// Letter index
	return l - 'A';
}

int EngimaState::indexof(const char* array, int find) {
	return strchr(array, find) - array;
}

void EngimaState::doPlug(char *r, const char *swapChars, int s) {
	for (int l = 0; l < s; l += 2) {
		int first = strchr(r, swapChars[l]) - r;
		int second = strchr(r, swapChars[l + 1]) - r;
		char tmp = r[first];
		r[first] = r[second];
		r[second] = tmp;
	}
}

int islower(int __c) {
	return __c >= 'a' && __c <= 'z';
}

int toupper(int __c) {
	return islower(__c) ? (__c & ~32) : __c;
}

const char* EngimaState::crypt(char *Wheels, const char *plugBoard, int plugBoardSize, const char *ct) {
	static const char reflector[] = "YRUHQSLDPXNGOKMIEBFZCWVJAT";
// Sets initial permutation
	int L = li(toupper(Wheels[1]));
	int M = li(toupper(Wheels[3]));
	int R = li(toupper(Wheels[5]));

	memset(&EncryptResult[0], 0, sizeof(EncryptResult));
	char *outPtr = &EncryptResult[0];

	int rotorIdx0 = li(toupper(Wheels[0])) % NUM_ROTORS;
	int rotorIdx1 = li(toupper(Wheels[2])) % NUM_ROTORS;
	int rotorIdx2 = li(toupper(Wheels[4])) % NUM_ROTORS;

	char r0[27] = { '\0' };
	strcpy(&r0[0], rotors[rotorIdx0]);
	doPlug(&r0[0], plugBoard, plugBoardSize);
	char r1[27] = { '\0' };
	strcpy(&r1[0], rotors[rotorIdx1]);
	doPlug(&r1[0], plugBoard, plugBoardSize);
	char r2[27] = { '\0' };
	strcpy(&r2[0], rotors[rotorIdx2]);
	doPlug(&r2[0], plugBoard, plugBoardSize);

	for (uint16_t x = 0; x < strlen(ct) && x < sizeof(EncryptResult); x++) {
		if (ct[x] == ' ')
			continue;

		int ct_letter = li(toupper(ct[x]));

		// Step right rotor on every iteration
		R = mod26(R + 1);

		// Pass through rotors
		char a = r2[mod26(R + ct_letter)];
		char b = r1[mod26(M + li(a) - R)];
		char c = r0[mod26(L + li(b) - M)];

		// Pass through reflector
		char ref = reflector[mod26(li(c) - L)];

		// Inverse rotor pass
		int d = mod26(indexof(&r0[0], alpha[mod26(li(ref) + L)]) - L);
		int e = mod26(indexof(&r1[0], alpha[mod26(d + M)]) - M);
		char f = alpha[mod26(indexof(&r2[0], alpha[mod26(e + R)]) - R)];

		*outPtr = f;
		outPtr++;
	}

	return &EncryptResult[0];
}

ErrorType EngimaState::onShutdown() {
	return ErrorType();
}

//////////////////////////////////////////////////////////////

BadgeInfoState::BadgeInfoState() :
		StateBase(), BadgeInfoList("Badge Info:", Items, 0, 0, 128, 64, 0, (sizeof(Items) / sizeof(Items[0]))) {

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
	sprintf(&ListBuffer[5][0], "Num contacts: %u", getContactStore().getSettings().getNumContacts());
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
		StateBase(), RadioInfoList("Radio Info:", Items, 0, 0, 128, 64, 0, (sizeof(Items) / sizeof(Items[0]))), Items(), ListBuffer() {

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
	sprintf(&ListBuffer[1][0], "RSSI: %d", getRadio().readRSSI());
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
DisplayMessageState Display_Message_State(3000, 0);
MenuState MenuState;
IRState TheIRPairingState(2000,5);
SettingState TheSettingState;
EngimaState TheEnginmaState;
AddressState TheAddressState;
SendMsgState TheSendMsgState;
RadioInfoState TheRadioInfoState;
BadgeInfoState TheBadgeInfoState;
GameOfLife TheGameOfLifeState;
EventState TheEventState;

bool StateFactory::init() {
	return true;
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

IRState *StateFactory::getIRPairingState() {
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

StateBase *StateFactory::getGameOfLifeState() {
	return &TheGameOfLifeState;
}

EventState *StateFactory::getEventState() {
	return &TheEventState;
}
