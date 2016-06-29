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
	ErrorType init();
	ErrorType shutdown();
	virtual ErrorType onInit()=0;
	virtual ReturnStateContext onRun(QKeyboard &kb)=0;
	virtual ErrorType onShutdown()=0;
	static const uint32_t INIT_BIT = 0x01;
private:
	bool hasBeenInitialized() {
		return (StateData & INIT_BIT) != 0;
	}
	uint32_t StateData;
	uint32_t StateStartTime;
};

class LogoState: public StateBase {
public:
	LogoState(uint16_t timeInState);
	virtual ~LogoState();
	void setTimeInLogo(uint16_t t) {TimeInLogoState = t;}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();

private:
	GUI_TickerData td;
	uint16_t TimeInLogoState;
};

class DisplayMessageState : public StateBase {
public:
	DisplayMessageState(uint16_t timeInState, StateBase *nextState);
	virtual ~DisplayMessageState();
	void setMessage(const char *msg);
	void setTimeInState(uint16_t t) {TimeInState = t;}
	void setNextState(StateBase *b) {NextState = b;}
	StateBase *getNextState() {return NextState;}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	char Message[64];
	uint16_t TimeInState;
	StateBase *NextState;
};

class MenuState : public StateBase {
public:
	MenuState();
	virtual ~MenuState();
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	GUI_ListData SettingList;
	GUI_ListItemData Items[6];
	/*
	 * SETTINGS,
		IR_PAIR,
		ADDRESS_BOOK,
		SEND_MESSAGE,
		CHALLENGES
		Badge Info
		Radio Info
	 */
};


class SettingState : public StateBase {
public:
	SettingState(StateBase *nextState);
	virtual ~SettingState();
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
	char AgentName[ContactStore::AGENT_NAME_LENGTH];
	uint8_t InputPos;
	uint8_t SubState;
};

/*
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

*/
//=============================
class StateFactory {
public:
	static bool init();
	static StateBase *getDisplayMessageState(StateBase *bm,
			const char *message, uint16_t timeToDisplay);
	static StateBase* getMenuState();
	static StateBase* getLogoState(uint16_t timeToDisplay);
};

#endif
