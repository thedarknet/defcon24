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

uint32_t min(uint32_t one, uint32_t two) {
	if(one<two) return one;
	return two;
}

void MessageState::addRadioMessage(const char *msg, uint16_t msgSize, uint16_t uid, uint8_t rssi) {
	memset(&RMsgs[CurrentPos].Msg[0],0,sizeof(RMsgs[CurrentPos].Msg));
	memcpy(&RMsgs[CurrentPos].Msg[0],msg,min(msgSize,sizeof(RMsgs[CurrentPos].Msg)));
	RMsgs[CurrentPos].Rssi = rssi;
	RMsgs[CurrentPos].FromUID = uid;
	CurrentPos++;
	CurrentPos = CurrentPos % ((sizeof(RMsgs) / sizeof(RMsgs[0])));
}


uint8_t msgdec(uint8_t c, uint8_t m) {
	if(c==0) {
		return m;
	}
	return (c-1);
}

ErrorType MessageState::onInit() {
	InternalState = MESSAGE_LIST;
	uint8_t v = CurrentPos==0?0:CurrentPos-1;
	for(int i=0;i<(sizeof(RMsgs) / sizeof(RMsgs[0]));i++) {
		Items[i].id = RMsgs[v].FromUID;
		ContactStore::Contact c;
		if(getContactStore().findContactByID(RMsgs[v].FromUID,c)) {
			Items[i].text = c.getAgentName();
			Items[i].setShouldScroll();
		} else {
			Items[i].text = "";
		}
		v = msgdec(v,4);
	}

	gui_set_curList(&RadioList);
	return ErrorType();
}

static char MsgDisplayBuffer[62] = {'\0'};
static char FromBuffer[20] = {'\0'};

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
				break;
			case 11: {
				if(Items[RadioList.selectedItem].id!=0) {
					uint8_t pos = 0xFF;
					MsgDisplayBuffer[0]='\0';
					for(int i=0;i<(sizeof(RMsgs) / sizeof(RMsgs[0]));i++) {
						if(RMsgs[i].FromUID==Items[RadioList.selectedItem].id) {
							strncpy(&MsgDisplayBuffer[0],RMsgs[i].Msg,sizeof(RMsgs[i].Msg));
						}
					}
					if(MsgDisplayBuffer[0]=='\0') {
						sprintf(&MsgDisplayBuffer[0],"Message from %s is gone only 8 stored in memory.",Items[RadioList.selectedItem].text);
						nextState = StateFactory::getDisplayMessageState(StateFactory::getMessageState(), &MsgDisplayBuffer[0],5000);
					} else {
						InternalState = DETAIL;
						sprintf(&FromBuffer[0],"F: %s",Items[RadioList.selectedItem].text);
					}
					gui_set_curList(0);
				}
				break;
			}
		}
	} else {
		//find message in array:
		gui_lable_multiline(&FromBuffer[0],0,10,128,64,1,0);
		gui_lable_multiline(&MsgDisplayBuffer[0],0,20,128,64,0,0);
		if(key==9 || key==11) {
			InternalState = MESSAGE_LIST;
			gui_set_curList(&RadioList);
		}
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
