#ifndef ENIGMA_STATE_H
#define ENIGMA_STATE_H

#include "../menus.h"
#include <sha256.h>

class EngimaState: public StateBase {
public:
	EngimaState();
	enum INTERNAL_STATE {
		SET_WHEEL, PLUG_BOARD, ENTER_MESSAGE, DECRYPT, QUEST_COMPLETION
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
	char PlugBoard[6];
	char EncryptResult[MAX_ENCRYPTED_LENGTH];
	uint8_t ResultHash[SHA256_HASH_SIZE];
	uint8_t DisplayOffset;
};

#endif
