#ifndef BADGE_MENUS_H
#define BADGE_MENUS_H

#include "badge.h"
#include "gui.h"
#include "Keyboard.h"
#include "KeyStore.h"

class StateBase;

struct ReturnStateContext {
	ReturnStateContext(StateBase *next, const ErrorType &er) :
			NextMenuToRun(next), Err(er) {
	}
	ReturnStateContext(StateBase *n) :
			NextMenuToRun(n), Err() {
	}
	StateBase *NextMenuToRun;
	ErrorType Err;
};

class StateBase {
public:
	StateBase();
	ReturnStateContext run(QKeyboard &kb);
	uint32_t timeInState();
	virtual ~StateBase();
protected:
	static const uint32_t INIT_BIT = 0x01;
	static const uint32_t DONT_RESET = 0x02;
	ErrorType init();
	ErrorType shutdown();
	virtual ErrorType onInit()=0;
	virtual ReturnStateContext onRun(QKeyboard &kb)=0;
	virtual ErrorType onShutdown()=0;
	void setState(uint32_t n) {
		StateData |= n;
	}
	void clearState(uint32_t n) {
		StateData = (StateData & ~n);
	}
	bool hasBeenInitialized() {
		return (StateData & INIT_BIT) != 0;
	}
	bool shouldReset() {
		return (StateData & DONT_RESET) == 0;
	}
private:
	uint32_t StateData;
	uint32_t StateStartTime;
};

class DisplayMessageState: public StateBase {
public:
	DisplayMessageState(uint16_t timeInState, StateBase *nextState);
	virtual ~DisplayMessageState();
	void setMessage(const char *msg);
	void setTimeInState(uint16_t t) {
		TimeInState = t;
	}
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
	char Message[64];
	uint16_t TimeInState;
	StateBase *NextState;
};

class MenuState: public StateBase {
public:
	MenuState();
	virtual ~MenuState();
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	GUI_ListData MenuList;
	GUI_ListItemData Items[8];
};

class SettingState: public StateBase {
public:
	SettingState();
	virtual ~SettingState();
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	GUI_ListData SettingList;
	GUI_ListItemData Items[4];
	char AgentName[ContactStore::AGENT_NAME_LENGTH];
	uint8_t InputPos;
	uint8_t SubState;
};

class BadgeInfoState: public StateBase {
public:
	BadgeInfoState();
	virtual ~BadgeInfoState();
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
	const char *getRegCode();
private:
	GUI_ListData BadgeInfoList;
	GUI_ListItemData Items[8];
	char ListBuffer[8][24]; //height then width
	char RegCode[17];
};
class RadioInfoState: public StateBase {
public:
	RadioInfoState();
	virtual ~RadioInfoState();
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	GUI_ListData RadioInfoList;
	GUI_ListItemData Items[5];
	char ListBuffer[5][20];
};

class MessageState;
class IRState;
class EventState;
class SendMsgState;
//=============================
class StateFactory {
public:
	static bool init();
	static StateBase *getDisplayMessageState(StateBase *bm, const char *message, uint16_t timeToDisplay);
	static StateBase *getMenuState();
	static StateBase *getSettingState();
	static IRState 	 *getIRPairingState();
	static StateBase *getAddressBookState();
	static SendMsgState *getSendMessageState();
	static StateBase *getEnigmaState();
	static StateBase *getBadgeInfoState();
	static StateBase *getRadioInfoState();
	static StateBase *getGameOfLifeState();
	static MessageState *getMessageState();
	static EventState* getEventState();

};

#endif
