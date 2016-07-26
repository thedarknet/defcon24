#ifndef ADDRESS_STATE_H
#define ADDRESS_STATE_H

#include "../menus.h"

class AddressState: public StateBase {
public:
	AddressState();
	virtual ~AddressState();
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
	void setNext4Items(uint16_t startAt);
private:
	GUI_ListData AddressList;
	GUI_ListItemData Items[4];
	ContactStore::Contact CurrentContactList[4];
	GUI_ListData ContactDetails;
	GUI_ListItemData DetailItems[5];
	char RadioIDBuf[12];
	char PublicKey[64];
	char SignatureKey[128];
};


#endif
