#include "../menus.h"
#include "irmenu.h"
#include <tim.h>
#include <uECC.h>
#include <sha256.h>
#include "ir.h"


IRState::IRState() :
		TimeoutOnSync(10000) {

}

IRState::~IRState() {

}

ErrorType IRState::onInit() {
	HAL_StatusTypeDef status = HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);
	if (HAL_OK == status) {
		IRInit();
		return ErrorType();
	}
	return ErrorType(ErrorType::TIMER_ERROR);
}

ReturnStateContext IRState::onRun(QKeyboard &kb) {
	uint8_t otherRadioID[2] = { 0x00, 0x00 };
	uint8_t otherCompressedPubKey[ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH] = { 0x00 };
	uint8_t otherSignature[ContactStore::SIGNATURE_LENGTH] = { 0x00 };
	//sign their radio id + compressed public key
	ShaOBJ shaObj;
	sha256_add(&shaObj, &otherRadioID[0], sizeof(otherRadioID));
	sha256_add(&shaObj, &otherCompressedPubKey[0], sizeof(otherCompressedPubKey));
	uint8_t digest[32];
	sha256_digest(&shaObj, digest);
	uint8_t signature[ContactStore::SIGNATURE_LENGTH];
	if (uECC_sign(getContactStore().getMyInfo().getPrivateKey(), &digest[0], sizeof(digest), signature, THE_CURVE)
			== 1) {
	}
	//verify what you get back
	//they are signing you're radio id and your compressed public key
	uint8_t myCompressedPubKey[ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH];
	uECC_compress(getContactStore().getMyInfo().getPublicKey(), &myCompressedPubKey[0], THE_CURVE);
	ShaOBJ shactx;
	uint16_t radioID = getContactStore().getMyInfo().getUniqueID();
	sha256_add(&shactx, (unsigned char*) &radioID, 2);
	sha256_add(&shactx, &myCompressedPubKey[0], sizeof(myCompressedPubKey));
	sha256_digest(&shaObj, digest);
	uint8_t otherUnCompressedPubKey[ContactStore::PUBLIC_KEY_LENGTH];
	uECC_decompress(&otherCompressedPubKey[0], &otherUnCompressedPubKey[0], THE_CURVE);
	if (uECC_verify(&otherUnCompressedPubKey[0], &digest[0], sizeof(digest), otherSignature, THE_CURVE) == 1) {
		//we have validated what we got back
	}

	gui_lable_multiline("not implemented yet", 0, 20, 120, 50, SSD1306_COLOR_BLACK, 0);
	gui_lable_multiline("press a key to return", 0, 30, 120, 50, SSD1306_COLOR_BLACK, 0);
	if (kb.getLastPinSeleted() == QKeyboard::NO_PIN_SELECTED) {
		return ReturnStateContext(this);
	} else {
		return ReturnStateContext(StateFactory::getMenuState());
	}
}

ErrorType IRState::onShutdown() {
	HAL_StatusTypeDef status = HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_2);
	if (HAL_OK == status) {
		IRStop();
		return ErrorType();
	}
	return ErrorType(ErrorType::TIMER_ERROR);
}
