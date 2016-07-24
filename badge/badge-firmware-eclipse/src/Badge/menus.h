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
	GUI_ListItemData Items[7];
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
	GUI_ListItemData Items[3]; //set agent name,
	char AgentName[ContactStore::AGENT_NAME_LENGTH];
	uint8_t InputPos;
	uint8_t SubState;
};


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
	uint8_t ContactIndex;
};

class SendMsgState: public StateBase {
public:
	static const int16_t NO_CONTACT = -1;
	enum INTERNAL_STATE {
		TYPE_MESSAGE, CONFIRM_SEND, SENDING
	};
	SendMsgState();
	virtual ~SendMsgState();
	void setContactToMessage(const uint8_t &cid);
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	int16_t ContactID;
	char MsgBuffer[60];
	uint16_t InputPos;
	INTERNAL_STATE InternalState;

};

class EngimaState: public StateBase {
public:
	EngimaState();
	enum INTERNAL_STATE {
		SET_WHEELS, ENTER_MESSAGE
	};
	virtual ~EngimaState();
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
protected:
	long mod26(long a);
	int li(char l);
	int indexof (const char* array, int find);
	const char* crypt(char *Wheels, const char *plugBoard, int plugBoardSize, const char *ct);
	void doPlug(char *r, const char *swapChars, int s);
private:
	static const uint16_t MAX_ENCRYPTED_LENGTH = 200;
	INTERNAL_STATE InternalState;
	char EntryBuffer[MAX_ENCRYPTED_LENGTH];
	char Wheels[6];
	char EncryptResult[MAX_ENCRYPTED_LENGTH];
};

class BadgeInfoState: public StateBase {
public:
	BadgeInfoState();
	virtual ~BadgeInfoState();
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	GUI_ListData BadgeInfoList;
	GUI_ListItemData Items[6];
	char ListBuffer[6][20]; //height then width
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
	GUI_ListItemData Items[4];
	char ListBuffer[4][20];
};

//=============================
class StateFactory {
public:
	static bool init();
	static StateBase *getDisplayMessageState(StateBase *bm, const char *message, uint16_t timeToDisplay);
	static StateBase *getMenuState();
	static StateBase *getSettingState();
	static StateBase *getIRPairingState();
	static StateBase *getAddressBookState();
	static StateBase *getSendMessageState();
	static StateBase *getEnigmaState();
	static StateBase *getBadgeInfoState();
	static StateBase *getRadioInfoState();

};

#endif
