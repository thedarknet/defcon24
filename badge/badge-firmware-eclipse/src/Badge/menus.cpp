#include "menus.h"

StateBase::StateBase() :
		StateData(0), StateStartTime(0) {
}

MenuRet StateBase::run() {
	MenuRet sr(this);
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

MenuRet LogoState::onRun() {
	gui_ticker(&td);
	td.x++;
	if (td.x > 127)
		td.x = 0;
	if (timeInState() > TimeInLogoState) {
		return MenuRet(StateFactory::getMenuState());
	} else {
		return MenuRet(this);
	}
}

ErrorType LogoState::onShutdown() {
	return true;
}

LogoState::~LogoState() {
}


//============================================================
LogoState Logo_State(uint16_t(5000));


bool StateFactory::init() {
	return true;
}

StateBase *StateFactory::getLogoState(uint16_t timeToDisplay) {
	Logo_State.setTimeInLogo(timeToDisplay);
	return &Logo_State;
}

StateBase *StateFactory::getDisplayMessageState(StateBase *bm,
		const char *message, uint16_t timeToDisplay) {

	return 0;
}

StateBase* StateFactory::getMenuState() {
	return 0;
}

