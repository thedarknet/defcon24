#include "MessageState.h"

MessageState::RadioMessage::RadioMessage() :
		Msg(), FromUID(0), Rssi(0) {
	memset(&Msg[0], 0, sizeof(Msg));
}

MessageState::MessageState() :
		RMsgs(), InternalState(MESSAGE_LIST), RadioList("Radio Msgs", Items, 0, 0, 128, 64, 0,
				(sizeof(Items) / sizeof(Items[0]))), CurrentPos(0), NewMessage(0) {
	memset(&RMsgs[0], 0, sizeof(RMsgs));
}

MessageState::~MessageState() {

}

uint32_t min(uint32_t one, uint32_t two) {
	if (one < two)
		return one;
	return two;
}

uint32_t lastBlinkTime = 0;
bool turnOnLED = false;
void MessageState::blink() {
	if (NewMessage) {
		if (HAL_GetTick() - lastBlinkTime > 1000) {
			if (turnOnLED) {
				HAL_GPIO_WritePin(LED_IR_STATUS_GPIO_Port, LED_IR_STATUS_Pin, GPIO_PIN_SET);
			} else {
				HAL_GPIO_WritePin(LED_IR_STATUS_GPIO_Port, LED_IR_STATUS_Pin, GPIO_PIN_RESET);
			}
			turnOnLED = !turnOnLED;
		}
	} else {
		HAL_GPIO_WritePin(LED_IR_STATUS_GPIO_Port, LED_IR_STATUS_Pin, GPIO_PIN_RESET);
	}
}

void MessageState::addRadioMessage(const char *msg, uint16_t msgSize, uint16_t uid, uint8_t rssi) {
	memset(&RMsgs[CurrentPos].Msg[0], 0, sizeof(RMsgs[CurrentPos].Msg));
	memcpy(&RMsgs[CurrentPos].Msg[0], msg, min(msgSize, sizeof(RMsgs[CurrentPos].Msg)));
	RMsgs[CurrentPos].Rssi = rssi;
	RMsgs[CurrentPos].FromUID = uid;
	CurrentPos++;
	CurrentPos = CurrentPos % ((sizeof(RMsgs) / sizeof(RMsgs[0])));
	NewMessage = true;
}

ErrorType MessageState::onInit() {
	InternalState = MESSAGE_LIST;
	//look at the newest message (the one just before cur pos bc currentpos is inc'ed after adding a message
	uint8_t v = CurrentPos == 0 ? MAX_R_MSGS - 1 : CurrentPos - 1;
	for (uint16_t i = 0; i < MAX_R_MSGS; i++) {
		Items[i].id = RMsgs[v].FromUID;
		ContactStore::Contact c;
		if (RMsgs[v].FromUID==RF69_BROADCAST_ADDR) {
			Items[i].text = "Broadcast Msg";
		} else if (getContactStore().findContactByID(RMsgs[v].FromUID, c)) {
			Items[i].text = c.getAgentName();
			Items[i].setShouldScroll();
		} else {
			Items[i].text = "";
		}
		v = v == 0 ? (MAX_R_MSGS - 1) : v - 1;
	}

	gui_set_curList(&RadioList);
	return ErrorType();
}

static char MsgDisplayBuffer[62] = { '\0' };
static char FromBuffer[20] = { '\0' };

ReturnStateContext MessageState::onRun(QKeyboard & kb) {
	StateBase *nextState = this;
	uint8_t key = kb.getLastKeyReleased();
	if (InternalState == MESSAGE_LIST) {
		NewMessage = false;
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
			if (Items[RadioList.selectedItem].id != 0) {
				MsgDisplayBuffer[0] = '\0';
				for (uint16_t i = 0; i < (sizeof(RMsgs) / sizeof(RMsgs[0])); i++) {
					if (RMsgs[i].FromUID == Items[RadioList.selectedItem].id) {
						strncpy(&MsgDisplayBuffer[0], RMsgs[i].Msg, sizeof(RMsgs[i].Msg));
					}
				}
				if (MsgDisplayBuffer[0] == '\0') {
					sprintf(&MsgDisplayBuffer[0], "Message from %s is gone only 8 stored in memory.",
							Items[RadioList.selectedItem].text);
					nextState = StateFactory::getDisplayMessageState(StateFactory::getMessageState(),
							&MsgDisplayBuffer[0], 5000);
				} else {
					InternalState = DETAIL;
					sprintf(&FromBuffer[0], "F: %s", Items[RadioList.selectedItem].text);
				}
				gui_set_curList(0);
			}
			break;
		}
		}
	} else {
		//find message in array:
		gui_lable_multiline(&FromBuffer[0], 0, 10, 128, 64, 1, 0);
		gui_lable_multiline(&MsgDisplayBuffer[0], 0, 20, 128, 64, 0, 0);
		if (key == 9 || key == 11) {
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
