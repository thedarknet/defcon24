#ifndef EVENT_STATE_H
#define EVENT_STATE_H


#include "../menus.h"

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
	MessageData Msgs[4];
};

#endif
