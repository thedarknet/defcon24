
#include "KeyStore.h"
#include <string.h>

const uint8_t ContactStore::DaemonPublic[ContactStore::PUBLIC_KEY_LENGTH] = 
					{ 0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00};

ContactStore::Contact::Contact() : UniqueID(0) {
	memset(&AgentName[0],0,sizeof(AgentName));
	memset(&MyPublicKey[0],0,sizeof(MyPublicKey));
	memset(&MyPrivateKey[0],0,sizeof(MyPrivateKey));
}

					
ContactStore::ContactStore(uint32_t startAddr, uint32_t size) :  StartAddress(startAddr), StorageSize(size) {
	
}

bool ContactStore::init() {
	//load version and check it
	//load preamble bytes
	//set number of stored contacts
	return true;
}

bool ContactStore::addContact(const ContactStore::Contact &c) {
	return true;
}

bool ContactStore::removeContact(const char *AgentName) {
	return true;
}

uint16_t ContactStore::getNumContactsThatCanBeStored() {
	return -1;
}

uint16_t ContactStore::getCurrentStoredContactCount() {
	return -1;
}