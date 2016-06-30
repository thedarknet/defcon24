#include "menus.h"

StateBase::StateBase() :
		StateData(0), StateStartTime(0) {
}

ReturnStateContext StateBase::run(QKeyboard &kb) {
	ReturnStateContext sr(this);
	if (!hasBeenInitialized()) {
		ErrorType et = init();
		if (!et.ok()) {
			sr.NextMenuToRun = StateFactory::getDisplayMessageState(
					StateFactory::getMenuState(), et.getMessage(), 10000);
		} else {
			StateData |= INIT_BIT;
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
		StateData |= INIT_BIT;
		StateStartTime = HAL_GetTick();
		return true;
	}
	return false;
}
ErrorType StateBase::shutdown() {
	ErrorType et = onShutdown();
	StateData = 0;
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
	if (timeInState() > TimeInLogoState) {
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

DisplayMessageState::DisplayMessageState(uint16_t timeInState,
		StateBase *nextState) :
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
	gui_lable_multiline(&this->Message[0], 0, 10, 120, 50, SSD1306_COLOR_BLACK,
			0);
	if (timeInState() > TimeInState) {
		return ReturnStateContext(StateFactory::getMenuState());
	} else {
		return ReturnStateContext(this);
	}
}

ErrorType DisplayMessageState::onShutdown() {
	return true;
}

MenuState::MenuState() :
		StateBase(), MenuList("Main Menu", (GUI_ListItemData**) Items, 0, 10,
				128, 64, 0, (sizeof(Items) / sizeof(Items[0]))) {
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
		MenuList.selectedItem =
				MenuList.selectedItem == 0 ?
						sizeof(Items) / sizeof(Items[0]) - 1 :
						MenuList.selectedItem--;
		break;
	}
	case 7: {
		MenuList.selectedItem =
				MenuList.selectedItem == sizeof(Items) / sizeof(Items[0]) - 1 ?
						0 : MenuList.selectedItem++;
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

SettingState::SettingState(StateBase * nextState) :
		StateBase(), NextState(nextState), SettingList((const char *) "MENU",
				(GUI_ListItemData**) &Items, 0, 0, 128, 64, 0,
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
			SettingList.selectedItem =
					SettingList.selectedItem == 0 ?
							sizeof(Items) / sizeof(Items[0]) - 1 :
							SettingList.selectedItem--;
			break;
		}
		case 7: {
			SettingList.selectedItem =
					SettingList.selectedItem
							== sizeof(Items) / sizeof(Items[0]) - 1 ?
							0 : SettingList.selectedItem++;
			break;
		}
		case 9: {
			nextState = this->getNextState();
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
		gui_lable((const char*) "Enter you're agent name:", 0, 10, 128, 64, 0,
				0);
		char letter = kb.getLetter();
		if (letter == 127) {
			AgentName[InputPos] = '\0';
			InputPos--;
		} else if (letter == 254) {
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
		gui_lable((const char*) "Choose Number of Screen Saver:", 0, 10, 128,
				64, 0, 0);
		gui_lable((const char*) "1: Game of Life", 0, 20, 128, 64, 0, 0);
		uint8_t ss = kb.getLastPinSeleted();
		if (ss == 1) {
			getContactStore().getRecordInfo().setScreenSaverType(ss);
		}
		break;
	}
	case 102:
		gui_lable((const char*) "Time until badge goes to sleep:", 0, 10, 128,
				64, 0, 0);
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

class IRState: public StateBase {
public:
	IRState(StateBase *ns);
	virtual ~IRState();
	void setNextState(StateBase *b) {
		NextState = b;
	}
	StateBase *getNextState() {
		return NextState;
	}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
};

class AddressState: public StateBase {
public:
	AddressState(StateBase *nextState);
	virtual ~AddressState();
	void setNextState(StateBase *b) {
		NextState = b;
	}
	StateBase *getNextState() {
		return NextState;
	}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
	GUI_ListData SettingList;
	GUI_ListItemData Items[4]; //set agent name,
};

class SendMsgState: public StateBase {
public:
	SendMsgState(StateBase *nextState);
	virtual ~SendMsgState();
	void setNextState(StateBase *b) {
		NextState = b;
	}
	StateBase *getNextState() {
		return NextState;
	}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
};

class EngimaState: public StateBase {
public:
	EngimaState(StateBase *nextState);
	virtual ~EngimaState();
	void setNextState(StateBase *b) {
		NextState = b;
	}
	StateBase *getNextState() {
		return NextState;
	}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
};

BadgeInfoState::BadgeInfoState() :
		StateBase(), BadgeInfoList("Badge Info:", (GUI_ListItemData**) Items, 0,
				0, 128, 64, 0, (sizeof(Items) / sizeof(Items[0]))) {

	for (int i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
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
	sprintf(&ListBuffer[0][1], "DEVID: %u", HAL_GetDEVID());
	sprintf(&ListBuffer[0][2], "REVID: %u", HAL_GetREVID());
	sprintf(&ListBuffer[0][3], "HAL Version: %u", HAL_GetHalVersion());
	sprintf(&ListBuffer[0][4], "UID: %u",
			getContactStore().getMyInfo().getUniqueID());
	sprintf(&ListBuffer[0][5], "Num contacts: %u",
			getContactStore().getRecordInfo().getNumContacts());
	for (uint32_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
		Items[i].text = &ListBuffer[0][i];
	}
	return ErrorType();
}

ReturnStateContext BadgeInfoState::onRun(QKeyboard &kb) {
	uint8_t key = kb.getLastPinSeleted();
	StateBase *nextState = this;
	switch (key) {
	case 1: {
		BadgeInfoList.selectedItem =
				BadgeInfoList.selectedItem == 0 ?
						sizeof(Items) / sizeof(Items[0]) - 1 :
						BadgeInfoList.selectedItem--;
		break;
	}
	case 7: {
		BadgeInfoList.selectedItem =
				BadgeInfoList.selectedItem
						== sizeof(Items) / sizeof(Items[0]) - 1 ?
						0 : BadgeInfoList.selectedItem++;
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

class RadioInfoState: public StateBase {
public:
	RadioInfoState(StateBase *nextState);
	virtual ~RadioInfoState();
	void setNextState(StateBase *b) {
		NextState = b;
	}
	StateBase *getNextState() {
		return NextState;
	}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
};

//============================================================
LogoState Logo_State(uint16_t(5000));
DisplayMessageState Display_Message_State(3000, 0);
MenuState MenuState;

bool StateFactory::init() {
	return true;
}

StateBase *StateFactory::getLogoState(uint16_t timeToDisplay) {
	Logo_State.setTimeInLogo(timeToDisplay);
	return &Logo_State;
}

StateBase *StateFactory::getDisplayMessageState(StateBase *bm,
		const char *message, uint16_t timeToDisplay) {
	Display_Message_State.setMessage(message);
	Display_Message_State.setNextState(bm);
	Display_Message_State.setTimeInState(timeToDisplay);
	return &Display_Message_State;
}

StateBase * StateFactory::getMenuState() {
	return &MenuState;
}

StateBase *StateFactory::getSettingState() {
	return 0;
}

StateBase *StateFactory::getIRPairingState() {
	return 0;
}

StateBase *StateFactory::getAddressBookState() {
	return 0;
}

StateBase *StateFactory::getSendMessageState() {
	return 0;
}

StateBase*StateFactory::getEnigmaState() {
	return 0;
}

StateBase* StateFactory::getBadgeInfoState() {
	return 0;
}

StateBase* StateFactory::getRadioInfoState() {
	return 0;
}

