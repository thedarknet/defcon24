#include "EventState.h"

EventState::MessageData::MessageData() {
	memset(&Msg[0],0,sizeof(Msg));
}

EventState::EventState() : Msgs() {
}

EventState::~EventState() {

}

void EventState::addMessage(const char *pMsg) {

}

ErrorType EventState::onInit() {
	return ErrorType();
}

ReturnStateContext EventState::onRun(QKeyboard &kb) {
	if (kb.getLastPinSeleted() == QKeyboard::NO_PIN_SELECTED) {
		return ReturnStateContext(this);
	} else {
		return ReturnStateContext(StateFactory::getMenuState());
	}
}

ErrorType EventState::onShutdown() {
	return ErrorType();
}
