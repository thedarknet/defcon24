
#include "KeyStore.h"
#include <string.h>

const uint8_t ContactStore::DaemonPublic[ContactStore::PUBLIC_KEY_LENGTH] = 
					{ 0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00};

						
ContactStore::RecordInfo::RecordInfo(uint32_t start) : StartAddress(start) {}
		
uint8_t ContactStore::RecordInfo::getVersion() {
	return *((uint8_t*)StartAddress);
}

uint16_t ContactStore::RecordInfo::getNumRecords() {
	return *((uint16_t*)(StartAddress+NumRecordsOffset));
}
		

ContactStore::Contact::Contact(uint32_t startAddress) : StartAddress(startAddress) {
	
}

ContactStore::MyInfo::MyInfo(uint32_t startAddress) : ContactStore::Contact(startAddress) {
	
}

uint8_t *ContactStore::MyInfo::getPrivateKey() {
	return ((uint8_t*)StartAddress+sizeof(ContactStore::Contact));
}

uint16_t ContactStore::Contact::getUniqueID() {
	return *((uint16_t*)StartAddress);
}
				
const char *ContactStore::Contact::getAgentName() {
	return ((char*)(StartAddress+sizeof(uint16_t)));
}
				
uint8_t *ContactStore::Contact::getPublicKey() {
	return ((uint8_t*)(StartAddress+sizeof(uint16_t)+AGENT_NAME_LENGTH));
}


void ContactStore::Contact::setUniqueID(uint16_t id) {
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,StartAddress,id);
}

void ContactStore::Contact::setAgentname(const char *name) {
	int len = strlen(name);
	uint32_t s = StartAddress+sizeof(uint16_t);
	for(int i=0;i<AGENT_NAME_LENGTH;i++,s++) {
		if(i<AGENT_NAME_LENGTH) {
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,s,name[i]);
		} else {
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,s,0);
		}
	}
}

void ContactStore::Contact::setPublicKey(uint8_t *key) {
	uint32_t s = StartAddress+sizeof(uint16_t)+AGENT_NAME_LENGTH;
	for(int i=0;i<PRIVATE_KEY_LENGTH;i++,s++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,s,key[i]);
	}
}


					
ContactStore::ContactStore(uint32_t startAddr, uint32_t size) :  StartAddress(startAddr), StorageSize(size), RecInfo(startAddr),
	MeInfo(startAddr+RecordInfo::SIZE){
	
}

bool ContactStore::init() {
	RecordInfo ri(StartAddress);
	if('0xDC'==ri.getVersion()) {
		return true;
	}
	return false;
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