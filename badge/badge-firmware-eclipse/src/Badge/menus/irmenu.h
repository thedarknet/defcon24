#ifndef IRMENU_H_
#define IRMENU_H

#include "../menus.h"

class IRState: public StateBase {
public:
	IRState();
	virtual ~IRState();
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	uint16_t TimeoutOnSync;
};

#endif

