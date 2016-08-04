#include "AddressState.h"
#include <KeyStore.h>
#include <gui.h>
#include "SendMsgState.h"
#include <RFM69.h>

////////////////////////////////////////////////
AddressState::AddressState() :
		StateBase(), AddressList((const char *) "Address Book", Items, 0, 0, 128, 64, 0,
				sizeof(Items) / sizeof(Items[0])), CurrentContactList(), ContactDetails(
				(const char *) "Contact Details: ", DetailItems, 0, 0, 128, 64, 0,
				sizeof(DetailItems) / sizeof(DetailItems[0])) {

}

AddressState::~AddressState() {

}

static const char *BROADCAST = "Broadcast";
static const char *NONE = "NONE";

ErrorType AddressState::onInit() {
	gui_set_curList(&AddressList);
	setNext4Items(0);
	for (uint16_t i = 0; i < sizeof(DetailItems) / sizeof(DetailItems[0]); ++i) {
		if (i == (sizeof(DetailItems) / sizeof(DetailItems[0]) - 1)) {
			DetailItems[i].text = "Send Msg";
		} else {

			DetailItems[i].text = "";
		}
		DetailItems[i].id = 0;
		DetailItems[i].Scrollable = 0;
	}
	memset(&RadioIDBuf[0], 0, sizeof(RadioIDBuf));
	memset(&PublicKey[0], 0, sizeof(PublicKey));
	memset(&SignatureKey[0], 0, sizeof(SignatureKey));
	return ErrorType();
}

void AddressState::resetSelection() {
	AddressList.selectedItem = 0;
}

void AddressState::setNext4Items(uint16_t startAt) {
	uint8_t num = getContactStore().getSettings().getNumContacts();
	for (uint16_t i = startAt, j = 0; j < (4); i++, j++) {
		if (i < num) {
			if (getContactStore().getContactAt(i, CurrentContactList[j])) {
				Items[j].id = CurrentContactList[j].getUniqueID();
				Items[j].text = CurrentContactList[j].getAgentName();
			}
		} else if (i == num && getContactStore().getMyInfo().isUberBadge()) {
			//add broadcast
			Items[j].id = RF69_BROADCAST_ADDR;
			Items[j].text = BROADCAST;
		} else {
			Items[j].id = 0;
			Items[j].text = "";
		}
	}
}

ReturnStateContext AddressState::onRun(QKeyboard &kb) {
	uint8_t pin = kb.getLastKeyReleased();
	StateBase *nextState = this;

	//then we are in address mode
	if (DetailItems[0].id == 0) {
		switch (pin) {
		case 1:
			if (AddressList.selectedItem == 0) {
				//keep selection at 0 but load new values
				uint16_t startAt = Items[AddressList.selectedItem].id;
				if (startAt > 0) {
					setNext4Items(startAt - 1);
				}
			} else {
				AddressList.selectedItem--;
			}
			break;
		case 7:
			if (AddressList.selectedItem == (sizeof(Items) / sizeof(Items[0]) - 1)) {
				setNext4Items(Items[AddressList.selectedItem].id + 1);
			} else {
				AddressList.selectedItem++;
			}
			break;
		case 9:
			nextState = StateFactory::getMenuState();
			break;
		case 11:
			if(Items[AddressList.selectedItem].id != 0) {
				gui_set_curList(&ContactDetails);
				if (Items[AddressList.selectedItem].id == RF69_BROADCAST_ADDR) {
					DetailItems[0].id = 1;
					DetailItems[0].text = BROADCAST;
					DetailItems[1].id = 1;
					sprintf(&RadioIDBuf[0], "ID: %d", RF69_BROADCAST_ADDR);
					DetailItems[1].text = &RadioIDBuf[0];
					DetailItems[2].id = 1;
					DetailItems[2].text = NONE;
					DetailItems[3].id = 1;
					DetailItems[3].text = NONE;
					DetailItems[3].resetScrollable();
					ContactDetails.selectedItem = sizeof(DetailItems) / sizeof(DetailItems[0]) - 1;
					StateFactory::getSendMessageState()->setContactToMessage(
					RF69_BROADCAST_ADDR, (const char *) "Broadcast");
				} else {

					DetailItems[0].id = 1;
					DetailItems[0].text = CurrentContactList[AddressList.selectedItem].getAgentName();
					DetailItems[1].id = 1;
					sprintf(&RadioIDBuf[0], "ID: %d", CurrentContactList[AddressList.selectedItem].getUniqueID());
					DetailItems[1].text = &RadioIDBuf[0];
					DetailItems[2].id = 1;
					uint8_t *pk = CurrentContactList[AddressList.selectedItem].getPublicKey();
					sprintf(&PublicKey[0],
							"PK: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
							pk[0], pk[1], pk[2], pk[3], pk[4], pk[5], pk[6], pk[7], pk[8], pk[9], pk[10], pk[11], pk[12],
							pk[13], pk[14], pk[15], pk[16], pk[17], pk[18], pk[19], pk[20], pk[21], pk[22], pk[23], pk[24]);
					DetailItems[2].text = &PublicKey[0];
					DetailItems[2].resetScrollable();
					DetailItems[3].id = 1;
					uint8_t *sig = CurrentContactList[AddressList.selectedItem].getPairingSignature();
					sprintf(&SignatureKey[0],
							"SIG: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
							sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7], sig[8], sig[9], sig[10],
							sig[11], sig[12], sig[13], sig[14], sig[15], sig[16], sig[17], sig[18], sig[19], sig[20],
							sig[21], sig[22], sig[23], sig[24], sig[25], sig[26], sig[27], sig[28], sig[29], sig[30],
							sig[31], sig[32], sig[33], sig[34], sig[35], sig[36], sig[37], sig[38], sig[39], sig[40],
							sig[41], sig[42], sig[43], sig[44], sig[45], sig[46], sig[47]);
					DetailItems[3].text = &SignatureKey[0];
					DetailItems[3].resetScrollable();
					ContactDetails.selectedItem = sizeof(DetailItems) / sizeof(DetailItems[0]) - 1;
					StateFactory::getSendMessageState()->setContactToMessage(
							CurrentContactList[AddressList.selectedItem].getUniqueID(),
							CurrentContactList[AddressList.selectedItem].getAgentName());
				}
			}
			break;
		}
	} else {
		if (pin != QKeyboard::NO_PIN_SELECTED) {
			if (DetailItems[ContactDetails.selectedItem].Scrollable) {
				DetailItems[ContactDetails.selectedItem].resetScrollable();
			}
		}
		switch (pin) {
		case 1:
			if (ContactDetails.selectedItem == 0) {
				ContactDetails.selectedItem = sizeof(DetailItems) / sizeof(DetailItems[0]) - 1;
			} else {
				ContactDetails.selectedItem--;
			}
			break;
		case 7:
			if (AddressList.selectedItem == (sizeof(DetailItems) / sizeof(DetailItems[0]) - 1)) {
				ContactDetails.selectedItem = 0;
			} else {
				ContactDetails.selectedItem++;
			}
			break;
		case 9:
			gui_set_curList(&AddressList);
			DetailItems[0].id = 0;
			break;
		case 11:
			nextState = StateFactory::getSendMessageState();
			break;
		}
	}
	return ReturnStateContext(nextState);
}

ErrorType AddressState::onShutdown() {
	gui_set_curList(0);
	return ErrorType();
}
