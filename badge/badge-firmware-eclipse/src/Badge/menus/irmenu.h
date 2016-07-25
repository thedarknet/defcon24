#ifndef IRMENU_H_
#define IRMENU_H

#include "../menus.h"

class IRState: public StateBase {
public:
	struct AliceInitConvo {
		uint8_t irmsgid;
		uint8_t AlicePublicKey[ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH];
		uint16_t AliceRadioID;
		char AliceName[ContactStore::AGENT_NAME_LENGTH];
	};

	struct BobReplyToInit {
		uint8_t irmsgid;
		uint8_t BoBPublicKey[ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH];
		uint16_t BoBRadioID;
		char BobAgentName[ContactStore::AGENT_NAME_LENGTH];
		uint8_t SignatureOfAliceData[ContactStore::SIGNATURE_LENGTH];
	};

	struct AliceToBobSignature {
		uint8_t irmsgid;
		uint8_t signature[48];
	};
public:
	IRState(uint16_t timeOutMS, uint16_t RetryCount);
	virtual ~IRState();
	void ListenForAlice();
	void BeTheBob();
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
private:
	uint16_t TimeoutMS;
	uint8_t RetryCount;
	uint8_t CurrentRetryCount;
	uint32_t TimeInState;
	AliceInitConvo AIC;
	BobReplyToInit BRTI;
	AliceToBobSignature ATBS;
	uint16_t TransmitInternalState;
	uint16_t ReceiveInternalState;
};

#endif

