#include "menus.h"

StateBase::StateBase() :
		StateData(0), StateStartTime(0) {
}

ReturnStateContext StateBase::run() {
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
		sr = onRun();
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

ReturnStateContext LogoState::onRun() {
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

ReturnStateContext DisplayMessageState::onRun() {
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
	Display_Message_State.setTimeInLogo(timeToDisplay);
	return &Display_Message_State;
}

StateBase* StateFactory::getMenuState() {
	return 0;
}

