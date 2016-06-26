#ifndef BADGE_MENUS_H
#define BADGE_MENUS_H

#include "badge.h"
#include "gui.h"

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
	ReturnStateContext run();
	uint32_t timeInState();
	virtual ~StateBase();
protected:
	ErrorType init();
	ErrorType shutdown();
	virtual ErrorType onInit()=0;
	virtual ReturnStateContext onRun()=0;
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
	virtual ReturnStateContext onRun();
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
	void setTimeInLogo(uint16_t t) {TimeInState = t;}
	void setNextState(StateBase *b) {NextState = b;}
	StateBase *getNextState() {return NextState;}
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun();
	virtual ErrorType onShutdown();
private:
	char Message[64];
	uint16_t TimeInState;
	StateBase *NextState;
};

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
