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
	strncpy(&this->Message[0],msg,sizeof(this->Message));
}

ReturnStateContext DisplayMessageState::onRun(QKeyboard &kb) {
	gui_lable_multiline(&this->Message[0], 0, 10, 120, 50, SSD1306_COLOR_BLACK, 0);
	if (timeInState() > TimeInState) {
		return ReturnStateContext(StateFactory::getMenuState());
	} else {
		return ReturnStateContext(this);
	}
}

ErrorType DisplayMessageState::onShutdown() {
	return true;
}


//class MenuState : public StateBase {
//public:
//	MenuState();
//	virtual ~MenuState();
//protected:
//	virtual ErrorType onInit();
///	virtual ReturnStateContext onRun(QKeyboard &kb);
//	virtual ErrorType onShutdown();
//private:
//	GUI_ListData SettingList;
//	GUI_ListItemData Items[6];
	/*
	 * SETTINGS,
		IR_PAIR,
		ADDRESS_BOOK,
		SEND_MESSAGE,
		CHALLENGES
		Badge Info
		Radio Info
	 */
//};


SettingState::SettingState(StateBase *nextState) : StateBase(), NextState(nextState),
		SettingList((const char *)"MENU", (GUI_ListItemData**) &Items, 0, 0, 128,64,0, sizeof(Items)/sizeof(Items[0])), InputPos(0),SubState(0) {

	memset(&AgentName[0],0,sizeof(AgentName));
	Items[0].id = 0;
	Items[0].text = (const char *)"Set Agent Name";
	Items[1].id = 1;
	Items[1].text = (const char *)"Set Screen Saver";
	Items[2].id = 2;
	Items[2].text = (const char *)"Set Sleep Time";
	Items[3].id = 3;
	Items[3].text = (const char *)"UNKNOWN";
}

ErrorType SettingState::onInit() {
	gui_set_curList(&SettingList);
	SubState = 0;
	return ErrorType();
}

ReturnStateContext SettingState::onRun(QKeyboard &kb) {
	uint8_t key = kb.getLastPinSeleted();
	StateBase *nextState = this;
	switch(SubState) {
	case 0:
		switch(key) {
		case 1: {
			SettingList.selectedItem = SettingList.selectedItem==0 ? sizeof(Items)/sizeof(Items[0])-1 : SettingList.selectedItem--;
			break;
			}
		case 7: {
			SettingList.selectedItem = SettingList.selectedItem==sizeof(Items)/sizeof(Items[0])-1 ? 0 : SettingList.selectedItem++;
			break;
			}
		case 9: {
			nextState = this->getNextState();
		}
			break;
		case 11: {
			SubState = SettingList.selectedItem+100;
		}
			break;
		default:
			break;
		}
	break;
	case 100: {
		gui_lable((const char*)"Enter you're agent name:",0,10,128,64,0,0);
		uint8_t letter = kb.getLetter();
		if(letter==255) {
			AgentName[InputPos] = '\0';
			InputPos--;
		} else if (letter==254) {
			//done
			getContactStore().getMyInfo().setAgentname(&AgentName[0]);
			gui_lable((const char *)"Saving ...",0,30,128,64,0,0);
			nextState = StateFactory::getMenuState();
		} else {
			AgentName[InputPos++] = letter;
		}
		gui_lable(&AgentName[0],0,20,128,64,0,0);
		break;
	}
	case 101: {
		gui_lable((const char*)"Choose Number of Screen Saver:",0,10,128,64,0,0);
		gui_lable((const char*)"1: Game of Life",0,20,128,64,0,0);
		uint8_t ss = kb.getLastPinSeleted();
		if(ss==1) {
			getContactStore().getRecordInfo().setScreenSaverType(ss);
		}
		break;
	}
		break;
	}
	return ReturnStateContext(nextState);
}

ErrorType SettingState::onShutdown() {

}


class IRState : public StateBase {
public:
	IRState(StateBase *ns);
	virtual ~IRState();
	void setNextState(StateBase *b) {NextState = b;}
	StateBase *getNextState() {return NextState;}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
};

class AddressState : public StateBase {
public:
	AddressState(StateBase *nextState);
	virtual ~AddressState();
	void setNextState(StateBase *b) {NextState = b;}
	StateBase *getNextState() {return NextState;}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
	GUI_ListData SettingList;
	GUI_ListItemData Items[4]; //set agent name,
};

class SendMsgState : public StateBase {
public:
	SendMsgState(StateBase *nextState);
	virtual ~SendMsgState();
	void setNextState(StateBase *b) {NextState = b;}
	StateBase *getNextState() {return NextState;}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
};

class EngimaState : public StateBase {
public:
	EngimaState(StateBase *nextState);
	virtual ~EngimaState();
	void setNextState(StateBase *b) {NextState = b;}
	StateBase *getNextState() {return NextState;}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
};

class BadgeInfoState : public StateBase {
public:
	BadgeInfoState(StateBase *nextState);
	virtual ~BadgeInfoState();
	void setNextState(StateBase *b) {NextState = b;}
	StateBase *getNextState() {return NextState;}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
};

class RadioInfoState : public StateBase {
public:
	RadioInfoState(StateBase *nextState);
	virtual ~RadioInfoState();
	void setNextState(StateBase *b) {NextState = b;}
	StateBase *getNextState() {return NextState;}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	StateBase *NextState;
};


//============================================================
LogoState Logo_State(uint16_t(5000));
DisplayMessageState Display_Message_State(3000,0);

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

StateBase* StateFactory::getMenuState() {
	return 0;
}

