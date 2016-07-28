#include "MessageState.h"

MessageState::RadioMessage::RadioMessage() :
		Msg(), FromUID(0), Rssi(0) {
	memset(&Msg[0], 0, sizeof(Msg));
}

MessageState::MessageState() :
		RMsgs(), InternalState(MESSAGE_LIST), RadioList("Radio Msgs", Items, 0, 0, 128, 64, 0,
				(sizeof(Items) / sizeof(Items[0]))), CurrentPos(0) {
	memset(&RMsgs[0],0,sizeof(RMsgs));
}

MessageState::~MessageState() {

}

void MessageState::addRadioMessage(const char *msg, uint16_t uid, uint8_t rssi) {
	strncpy((char *) &RMsgs[CurrentPos].Msg[0], msg, sizeof(RMsgs[CurrentPos].Msg));
	RMsgs[CurrentPos].Rssi = rssi;
	RMsgs[CurrentPos].FromUID = uid;
	CurrentPos++;
	CurrentPos = CurrentPos % ((sizeof(RMsgs) / sizeof(RMsgs[0])));
}

static const char *UNKNOWN = "UNKNOWN";

uint8_t msgdec(uint8_t c, uint8_t m) {
	if(c==0) {
		return m;
	}
	return (c-1);
}

ErrorType MessageState::onInit() {
	uint8_t v = CurrentPos;
	for(int i=0;i<5;i++) {
		Items[i].id = RMsgs[v].FromUID;
		ContactStore::Contact c;
		if(getContactStore().findContactByID(RMsgs[v].FromUID,c)) {
			Items[i].text = c.getAgentName();
		} else {
			Items[i].text = UNKNOWN;
		}
		Items[i].resetScrollable();
		v = msgdec(v,4);
	}

	gui_set_curList(&RadioList);
	return ErrorType();
}

ReturnStateContext MessageState::onRun(QKeyboard &kb) {
	StateBase *nextState = this;
	uint8_t key = kb.getLastKeyReleased();
	if (InternalState == MESSAGE_LIST) {
		switch (key) {
			case 1: {
				if (RadioList.selectedItem == 0) {
					RadioList.selectedItem = sizeof(Items) / sizeof(Items[0]) - 1;
				} else {
					RadioList.selectedItem--;
				}
				break;
			}
			case 7: {
				if (RadioList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
					RadioList.selectedItem = 0;
				} else {
					RadioList.selectedItem++;
				}
				break;
			}
			case 9: {
				nextState = StateFactory::getMenuState();
			}
		}
	} else {

	}
	return ReturnStateContext(nextState);
}

ErrorType MessageState::onShutdown() {
	gui_set_curList(0);
	return ErrorType();
}

/////////////////////
/*
EventState::MessageData::MessageData() {
	memset(&Msg[0], 0, sizeof(Msg));
}

EventState::EventState() :
		Msgs(), CurrentPos(0), EventList("Event Log", Items, 0, 0, 128, 64, 0, (sizeof(Items) / sizeof(Items[0]))) {
}

EventState::~EventState() {

}

void EventState::addMessage(const char *pMsg) {
	strncpy((char *) &Msgs[CurrentPos].Msg[0], pMsg, sizeof(Msgs[CurrentPos]));
	CurrentPos++;
	CurrentPos = CurrentPos % ((sizeof(Msgs) / sizeof(Msgs[0])));
}

ErrorType EventState::onInit() {
	for (uint8_t i = 0; i < (sizeof(Items) / sizeof(Items[0])); i++) {
		Items[i].id = i;
		Items[i].text = &Msgs[i].Msg[0];
		Items[i].resetScrollable();
	}
	gui_set_curList(&EventList);
	return ErrorType();
}

ReturnStateContext EventState::onRun(QKeyboard &kb) {
	uint8_t key = kb.getLastKeyReleased();
	StateBase *nextState = this;
	switch (key) {
	case 1: {
		if (EventList.selectedItem == 0) {
			EventList.selectedItem = sizeof(Items) / sizeof(Items[0]) - 1;
		} else {
			EventList.selectedItem--;
		}
		break;
	}
	case 7: {
		if (EventList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
			EventList.selectedItem = 0;
		} else {
			EventList.selectedItem++;
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

ErrorType EventState::onShutdown() {
	gui_set_curList(0);
	return ErrorType();
}
*/
