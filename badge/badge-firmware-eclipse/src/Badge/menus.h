#ifndef BADGE_MENUS_H
#define BADGE_MENUS_H

#include "badge.h"
#include "gui.h"

class StateBase;

struct MenuRet {
	MenuRet(StateBase *next, const ErrorType &er) :
			NextMenuToRun(next), Err(er) {
	}
	MenuRet(StateBase *n) :
			NextMenuToRun(n), Err() {
	}
	StateBase *NextMenuToRun;
	ErrorType Err;
};

class StateBase {
public:
	StateBase();
	MenuRet run();
	uint32_t timeInState();
	virtual ~StateBase();
protected:
	ErrorType init();
	ErrorType shutdown();
	virtual ErrorType onInit()=0;
	virtual MenuRet onRun()=0;
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
	virtual MenuRet onRun();
	virtual ErrorType onShutdown();

private:
	GUI_TickerData td;
	uint16_t TimeInLogoState;
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
