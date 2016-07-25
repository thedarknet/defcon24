#include "../menus.h"
#include "irmenu.h"
#include <tim.h>
#include <uECC.h>
#include <sha256.h>
#include "ir.h"
#include "menus/EventState.h"

////////////////////////////////////////////////////
typedef struct SHA256_HashContext {
	uECC_HashContext uECC;
	ShaOBJ ctx;
} SHA256_HashContext;

void init_SHA256(const uECC_HashContext *base) {
	SHA256_HashContext *context = (SHA256_HashContext *) base;
	sha256_init(&context->ctx);
}

void update_SHA256(const uECC_HashContext *base, const uint8_t *message, unsigned message_size) {
	SHA256_HashContext *context = (SHA256_HashContext *) base;
	sha256_add(&context->ctx, message, message_size);
}

void finish_SHA256(const uECC_HashContext *base, uint8_t *hash_result) {
	SHA256_HashContext *context = (SHA256_HashContext *) base;
	//SHA256_Final(hash_result, &context->ctx);
	sha256_digest(&context->ctx, hash_result);
}

//////////////////////////

enum {
	BOB_WAITING_FOR_FIRST_TRANSMIT,
	BOB_WAITING_FOR_SECOND_TRANSMIT,
	I_AM_ALICE_DISABLE_LISTEN,
	ALICE_INIT_CONVERSATION,
	ALICE_RECEIVE_ONE
};

IRState::IRState(uint16_t timeOutMS, uint16_t retryCount) :
		TimeoutMS(timeOutMS), RetryCount(retryCount), CurrentRetryCount(0), TimeInState(0), TransmitInternalState(
				ALICE_INIT_CONVERSATION), ReceiveInternalState(BOB_WAITING_FOR_FIRST_TRANSMIT) {

}

IRState::~IRState() {

}

ErrorType IRState::onInit() {
	//stop receiving
	IRStopRX();
	CurrentRetryCount = 0;
	memset(&AIC, 0, sizeof(AIC));
	memset(&BRTI, 0, sizeof(BRTI));
	memset(&ATBS, 0, sizeof(ATBS));
	TransmitInternalState = ALICE_INIT_CONVERSATION;
	ReceiveInternalState = I_AM_ALICE_DISABLE_LISTEN;
	return ErrorType();
}

void IRState::ListenForAlice() {
	uint32_t bytesAvailable = IRBytesAvailable();
	if (ReceiveInternalState == BOB_WAITING_FOR_FIRST_TRANSMIT) {
		if (bytesAvailable >= 40) {
			IRStopRX();
			uint8_t *buf = IRGetBuff();
			if (buf[0] == 1) {
				AliceInitConvo *aic = (AliceInitConvo*) buf;
				memcpy(&AIC,aic,sizeof(AIC));
				uint8_t message_hash[SHA256_HASH_SIZE];
				ShaOBJ messageHashCtx;
				sha256_init(&messageHashCtx);
				sha256_add(&messageHashCtx, (uint8_t*) &aic->AliceRadioID, sizeof(aic->AliceRadioID));
				sha256_add(&messageHashCtx, (uint8_t*) &aic->AlicePublicKey, sizeof(aic->AlicePublicKey));
				sha256_digest(&messageHashCtx, &message_hash[0]);
				uint8_t signature[ContactStore::SIGNATURE_LENGTH];

				uint8_t tmp[32 + 32 + 64];
				SHA256_HashContext ctx = { { &init_SHA256, &update_SHA256, &finish_SHA256, 64, 32, &tmp[0] } };
				uECC_sign_deterministic(getContactStore().getMyInfo().getPrivateKey(), message_hash,
						sizeof(message_hash), &ctx.uECC, signature, THE_CURVE);
				BRTI.irmsgid = 2;
				BRTI.BoBRadioID = getContactStore().getMyInfo().getUniqueID();
				memcpy(&BRTI.BoBPublicKey[0], getContactStore().getMyInfo().getCompressedPublicKey(),
						sizeof(BRTI.BoBPublicKey));
				strncpy(&BRTI.BobAgentName[0], getContactStore().getSettings().getAgentName(),
						sizeof(BRTI.BobAgentName));
				memcpy(&BRTI.SignatureOfAliceData[0], &signature[0], sizeof(BRTI.SignatureOfAliceData));
				IRTxBuff((uint8_t*) &BRTI, sizeof(BRTI));
				ReceiveInternalState = BOB_WAITING_FOR_SECOND_TRANSMIT;
				TimeInState = HAL_GetTick();
			}
			IRStartRx();
		} else {
			//reset buffers!
			ReceiveInternalState = BOB_WAITING_FOR_FIRST_TRANSMIT;
		}
	} else if (ReceiveInternalState == BOB_WAITING_FOR_SECOND_TRANSMIT) {
		if (bytesAvailable >= sizeof(AliceToBobSignature)) {
			uint8_t *buf = IRGetBuff();
			if (buf[0] == 3) {
				IRStopRX();
				AliceToBobSignature *atbs = (AliceToBobSignature*) buf;
				uint8_t uncompressedPublicKey[ContactStore::PUBLIC_KEY_LENGTH];
				uECC_decompress(&AIC.AlicePublicKey[0], &uncompressedPublicKey[0], THE_CURVE);
				uint8_t msgHash[SHA256_HASH_SIZE];
				ShaOBJ msgHashCtx;
				sha256_init(&msgHashCtx);
				uint16_t radioID = getContactStore().getMyInfo().getUniqueID();
				sha256_add(&msgHashCtx, (uint8_t*) &radioID, sizeof(uint16_t));
				sha256_add(&msgHashCtx, (uint8_t*) getContactStore().getMyInfo().getCompressedPublicKey(),
						ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH);
				//verify alice's signature of my public key and unique id
				sha256_digest(&msgHashCtx, &msgHash[0]);
				if (uECC_verify(&uncompressedPublicKey[0], &msgHash[0], sizeof(msgHash), &atbs->signature[0],
				THE_CURVE)) {
					//ok to add to contacts
					if (getContactStore().addContact(AIC.AliceRadioID, &AIC.AliceName[0], &AIC.AlicePublicKey[0],
							&atbs->signature[0])) {
						char displayBuf[24];
						sprintf(&displayBuf[0], "New Contact: %s", &AIC.AliceName[0]);
						StateFactory::getEventState()->addMessage(&displayBuf[0]);
					} else {
						char displayBuf[24];
						sprintf(&displayBuf[0], "New Contact: %s", &AIC.AliceName[0]);
						StateFactory::getEventState()->addMessage(&displayBuf[0]);
					}
				}
				IRStartRx();
			} else {
				//reset buffers!
				ReceiveInternalState = BOB_WAITING_FOR_FIRST_TRANSMIT;
				IRStopRX();
				IRStartRx();
			}
		} else {
			if ((HAL_GetTick() - TimeInState) > TimeoutMS) {
				TransmitInternalState = BOB_WAITING_FOR_FIRST_TRANSMIT;
				IRStopRX();
				IRStartRx();
			}
		}
	} else {
		//I_AM_ALICE_DISABLE_LISTEN:
		//break;
	}

}

//alice starts convo
//bob
ReturnStateContext IRState::onRun(QKeyboard &kb) {
	static const char *msg1 = "Init convo complete.";
	static const char *msg2 = "Listening for Bob";
	static const char *msg3 = "Sent final msg to Bob";
	static const char *msg4 = "Contact added";
	uint32_t bytesAvailable = IRBytesAvailable();
	if (TransmitInternalState == ALICE_INIT_CONVERSATION) {
		AIC.irmsgid = 1;
		memcpy(&AIC.AlicePublicKey[0], getContactStore().getMyInfo().getCompressedPublicKey(),
				ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH);
		AIC.AliceRadioID = getContactStore().getMyInfo().getUniqueID();
		strncpy(&AIC.AliceName[0], getContactStore().getSettings().getAgentName(), sizeof(AIC.AliceName));
		IRTxBuff((uint8_t*) &AIC, sizeof(AIC));
		TransmitInternalState = ALICE_RECEIVE_ONE;
		gui_lable_multiline(msg1, 0, 10, 128, 64, 0, 0);
		TimeInState = HAL_GetTick();
		//ok start recieving
		IRStartRx();
	} else if (TransmitInternalState == ALICE_RECEIVE_ONE) {
		gui_lable_multiline(msg1, 0, 10, 128, 64, 0, 0);
		gui_lable_multiline(msg2, 0, 20, 128, 64, 0, 0);
		if (bytesAvailable >= 88) {
			uint8_t *buf = IRGetBuff();
			if (buf[0] == 2) {
				//first stop recieving
				IRStopRX();
				BobReplyToInit *brti = (BobReplyToInit*) buf;
				//using signature validate our data that bob signed
				uint8_t uncompressedPublicKey[ContactStore::PUBLIC_KEY_LENGTH];
				uECC_decompress(&brti->BoBPublicKey[0], &uncompressedPublicKey[0], THE_CURVE);
				uint8_t msgHash[SHA256_HASH_SIZE];
				ShaOBJ msgHashCtx;
				sha256_init(&msgHashCtx);
				uint16_t radioID = getContactStore().getMyInfo().getUniqueID();
				sha256_add(&msgHashCtx, (uint8_t*) &radioID, sizeof(uint16_t));
				sha256_add(&msgHashCtx, (uint8_t*) getContactStore().getMyInfo().getCompressedPublicKey(),
						ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH);
				sha256_digest(&msgHashCtx, &msgHash[0]);
				if (uECC_verify(&uncompressedPublicKey[0], &msgHash[0], sizeof(msgHash), &brti->SignatureOfAliceData[0],
				THE_CURVE)) {

					uint8_t message_hash[SHA256_HASH_SIZE];
					ShaOBJ messageHashCtx;
					sha256_init(&messageHashCtx);
					sha256_add(&messageHashCtx, (uint8_t*) &brti->BoBRadioID, sizeof(brti->BoBRadioID));
					sha256_add(&messageHashCtx, (uint8_t*) &brti->BoBPublicKey[0], sizeof(brti->BoBPublicKey));
					sha256_digest(&messageHashCtx, &message_hash[0]);

					uint8_t tmp[32 + 32 + 64];
					ATBS.irmsgid = 3;
					SHA256_HashContext ctx = { { &init_SHA256, &update_SHA256, &finish_SHA256, 64, 32, tmp } };
					uECC_sign_deterministic(getContactStore().getMyInfo().getPrivateKey(), message_hash,
							sizeof(message_hash), &ctx.uECC, &ATBS.signature[0], THE_CURVE);

					IRTxBuff((uint8_t*) &ATBS, sizeof(ATBS));

					gui_lable_multiline(msg3, 0, 30, 128, 64, 0, 0);
					//ok to add to contacts
					if (getContactStore().addContact(brti->BoBRadioID, &brti->BobAgentName[0], &brti->BoBPublicKey[0],
							&brti->SignatureOfAliceData[0])) {
						char displayBuf[24];
						sprintf(&displayBuf[0], "New Contact: %s", &brti->BobAgentName[0]);
						gui_lable_multiline(msg4, 0, 40, 128, 64, 0, 0);
						StateFactory::getEventState()->addMessage(&displayBuf[0]);
					} else {
						char displayBuf[24];
						sprintf(&displayBuf[0], "Failed to save contact: %s", &brti->BobAgentName[0]);
						StateFactory::getEventState()->addMessage(&displayBuf[0]);
					}
				} else {
					char displayBuf[24];
					sprintf(&displayBuf[0], "Signature Check Failed with %s", &brti->BobAgentName[0]);
					StateFactory::getEventState()->addMessage(&displayBuf[0]);
				}
			}
			return ReturnStateContext(StateFactory::getMenuState());
		}
		if ((HAL_GetTick() - TimeInState) > TimeoutMS) {
			CurrentRetryCount++;
			TransmitInternalState = ALICE_INIT_CONVERSATION;
			IRStopRX();
			if (CurrentRetryCount >= RetryCount) {
				return ReturnStateContext(
						StateFactory::getDisplayMessageState(StateFactory::getMenuState(), "Failed to pair", 5000));
			}
		}
	}
	return ReturnStateContext(this);
}

void IRState::BeTheBob() {
	ReceiveInternalState = BOB_WAITING_FOR_FIRST_TRANSMIT;
	IRStartRx();
}

ErrorType IRState::onShutdown() {
	//go back to listening for alice
	BeTheBob();
	return ErrorType();
}
