
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
		
uint16_t ContactStore::RecordInfo::getVersion() {
	return *((uint16_t*)StartAddress);
}

uint32_t ContactStore::RecordInfo::getStartContactsAddress() {
	return (StartAddress+RecordInfo::SIZE+MyInfo::SIZE);
}

uint16_t ContactStore::RecordInfo::getNumRecords() {
	int retVal = 0;
	uint32_t contactsAddr = getStartContactsAddress();
	//while(*((uint16_t*)contactsAddr)!=0xFFFF) retVal++;
	return retVal;
}

bool ContactStore::RecordInfo::isFull() {
	RecordInfo ri(StartAddress);
	return ri.getNumRecords()>=MAX_CONTACTS;
}

ContactStore::MyInfo &ContactStore::getMyInfo() {
	return MeInfo;
}

ContactStore::RecordInfo &ContactStore::getRecordInfo() {
	return RecInfo;
}

ContactStore::Contact::Contact(uint32_t startAddress) : StartAddress(startAddress) {
	
}

ContactStore::MyInfo::MyInfo(uint32_t startAddress) : ContactStore::Contact(startAddress) {
	
}

uint8_t *ContactStore::MyInfo::getPrivateKey() {
	return ((uint8_t*)(StartAddress+sizeof(ContactStore::Contact::SIZE)));
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
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,StartAddress,id);
}

void ContactStore::Contact::setAgentname(const char name[AGENT_NAME_LENGTH]) {
	//int len = strlen(name);
	FLASH_LOCKER f;
	uint32_t s = StartAddress+sizeof(uint16_t);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s,(*((uint32_t *)&name[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+4,(*((uint32_t *)&name[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+8,(*((uint32_t *)&name[8])));
}

void ContactStore::Contact::setPublicKey(const uint8_t key[PUBLIC_KEY_LENGTH]) {
	uint32_t s = StartAddress+sizeof(uint16_t)+AGENT_NAME_LENGTH;
	FLASH_LOCKER f;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s,(*((uint32_t *)&key[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+4,(*((uint32_t *)&key[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+8,(*((uint32_t *)&key[8])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+12,(*((uint32_t *)&key[12])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+16,(*((uint32_t *)&key[16])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+20,(*((uint32_t *)&key[20])));
}

					
ContactStore::ContactStore(uint32_t startAddr, uint32_t size) :  StartAddress(startAddr), StorageSize(size), RecInfo(startAddr),
	MeInfo(startAddr+RecordInfo::SIZE){
	
}
	


bool ContactStore::init() {
	FLASH_LOCKER f;
	RecordInfo ri(StartAddress);
	uint16_t temp = ri.getVersion();
	if(0xDCDC==ri.getVersion()) {
		return true;
	}
	return false;
}

bool ContactStore::addContact(uint16_t uid, char agentName[AGENT_NAME_LENGTH], uint8_t key[PUBLIC_KEY_LENGTH]) { 
	RecordInfo ri(StartAddress);
	if(!ri.isFull()) {
		uint32_t location = RecInfo.getStartContactsAddress()+(ri.getNumRecords()*Contact::SIZE);
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


