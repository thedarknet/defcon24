
#include "KeyStore.h"
#include <string.h>

const uint8_t ContactStore::DaemonPublic[ContactStore::PUBLIC_KEY_LENGTH] = 
					{ 0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00};

ContactStore::RecordInfo::RecordInfo(uint32_t start) : StartAddress(start) {}
	
class FLASH_LOCKER {
	public:
		FLASH_LOCKER() {
			HAL_FLASH_Unlock();
		}
		~FLASH_LOCKER() {
			HAL_FLASH_Lock();
		}
};
		
uint8_t ContactStore::RecordInfo::getVersion() {
	return *((uint8_t*)StartAddress);
}

uint16_t ContactStore::RecordInfo::getNumRecords() {
	return *((uint16_t*)(StartAddress+NumRecordsOffset));
}

void ContactStore::RecordInfo::setNumRecords(uint16_t num) {
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,StartAddress+1,num);
}

void ContactStore::RecordInfo::incRecordCount() {
	RecordInfo ri(StartAddress);
	uint16_t n = ri.getNumRecords();
	ri.setNumRecords(n+1);
}

bool ContactStore::RecordInfo::isFull() {
	RecordInfo ri(StartAddress);
	return ri.getNumRecords()>=MAX_CONTACTS;
}

ContactStore::MyInfo ContactStore::getMyInfo() {
	return MyInfo(StartAddress+RecordInfo::SIZE);
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
	FLASH_LOCKER f;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,StartAddress,id);
}

void ContactStore::Contact::setAgentname(const char name[AGENT_NAME_LENGTH]) {
	int len = strlen(name);
	FLASH_LOCKER f;
	uint32_t s = StartAddress+sizeof(uint16_t);
	for(int i=0;i<AGENT_NAME_LENGTH;i++,s++) {
		if(i<AGENT_NAME_LENGTH) {
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,s,name[i]);
		} else {
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,s,0);
		}
	}
}

void ContactStore::Contact::setPublicKey(uint8_t key[PUBLIC_KEY_LENGTH]) {
	uint32_t s = StartAddress+sizeof(uint16_t)+AGENT_NAME_LENGTH;
	FLASH_LOCKER f;
	for(int i=0;i<PRIVATE_KEY_LENGTH;i++,s++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,s,key[i]);
	}
}

					
ContactStore::ContactStore(uint32_t startAddr, uint32_t size) :  StartAddress(startAddr), StorageSize(size), RecInfo(startAddr),
	MeInfo(startAddr+RecordInfo::SIZE){
	
}
	


bool ContactStore::init() {
	FLASH_LOCKER f;
	FLASH_PageErase(StartAddress);
	RecordInfo ri(StartAddress);
	if(0xDC==ri.getVersion()) {
		return true;
	}
	return false;
}

bool ContactStore::addContact(uint16_t uid, char agentName[AGENT_NAME_LENGTH], uint8_t key[PUBLIC_KEY_LENGTH]) { 
	RecordInfo ri(StartAddress);
	if(!ri.isFull()) {
		uint32_t location = getStartContactsAddress()+(ri.getNumRecords()*Contact::SIZE);
		Contact c(location);
		c.setUniqueID(uid);
		c.setAgentname(agentName);
		c.setPublicKey(key);
		return true;
	} 
	return false;
}

uint16_t ContactStore::getNumContactsThatCanBeStored() {
	return MAX_CONTACTS;
}

uint16_t ContactStore::getCurrentStoredContactCount() {
	RecordInfo ri(StartAddress);
	return ri.getNumRecords();
}

uint32_t ContactStore::getStartContactsAddress() {
	return (StartAddress+RecordInfo::SIZE+MyInfo::SIZE);
}

