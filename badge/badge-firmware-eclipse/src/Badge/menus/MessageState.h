#ifndef EVENT_STATE_H
#define EVENT_STATE_H

#include "../menus.h"
#include <RFM69.h>

class MessageState: public StateBase {
public:
	struct RadioMessage {
		char Msg[RF69_MAX_DATA_LEN];
		uint16_t FromUID;
		int8_t Rssi;
		RadioMessage();
	};
	enum {
		MESSAGE_LIST, DETAIL
	};
public:
	MessageState();
	virtual ~MessageState();
	void addRadioMessage(const char *msg, uint16_t msgSize, uint16_t uid, uint8_t rssi);
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	RadioMessage RMsgs[8];
	uint8_t InternalState;
	GUI_ListData RadioList;
	GUI_ListItemData Items[8];
	uint8_t CurrentPos;
};
/*
class EventState: public StateBase {
public:
	struct MessageData {
		char Msg[24];
		MessageData();
	};
public:
	EventState();
	virtual ~EventState();
	void addMessage(const char *pMsg);
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	MessageData Msgs[5];
	uint8_t CurrentPos;
	GUI_ListData EventList;
	GUI_ListItemData Items[5];
};
*/
#endif
