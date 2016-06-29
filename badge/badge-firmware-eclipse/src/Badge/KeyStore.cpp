
#include "KeyStore.h"
#include <string.h>

const uint8_t ContactStore::DaemonPublic[ContactStore::PUBLIC_KEY_LENGTH] = 
					{ 0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00};

class FLASH_LOCKER {
	public:
		FLASH_LOCKER() {
			HAL_FLASH_Unlock();
		}
		~FLASH_LOCKER() {
			HAL_FLASH_Lock();
		}
};

ContactStore::RecordInfo::RecordInfo(uint32_t start) : StartAddress(start) {}

		
uint16_t ContactStore::RecordInfo::getVersion() {
	return *((uint16_t*)StartAddress);
}

uint8_t ContactStore::RecordInfo::getNumContacts() {
	DataStructure ds = getRecordDS();
	return ds.NumContacts;
}

ContactStore::RecordInfo::DataStructure ContactStore::RecordInfo::getRecordDS() {
	return *((ContactStore::RecordInfo::DataStructure*)(StartAddress+sizeof(uint16_t)));
}

void ContactStore::RecordInfo::writeRecordDS(const DataStructure &ds) {
	FLASH_LOCKER f;
	uint32_t data = *((uint32_t*)&ds);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,(StartAddress+sizeof(uint16_t)),data);
}


uint8_t ContactStore::RecordInfo::setNumContacts(uint8_t num) {
	if(num>MAX_CONTACTS) return MAX_CONTACTS;
	DataStructure ds = getRecordDS();
	ds.NumContacts = num;
	writeRecordDS(ds);
	return num;
}

uint32_t ContactStore::RecordInfo::getContactStartAddress() {
	return SIZE + MyInfo::SIZE;
}

bool ContactStore::RecordInfo::getContactAt(uint8_t n, ContactStore::Contact &c) {
	uint8_t currnetNum = getNumContacts();
	if(n>currnetNum) {
		return false;
	}
	c = ContactStore::Contact(getContactStartAddress()+(n*ContactStore::Contact::SIZE));
	return true;
}


void ContactStore::RecordInfo::setScreenSaverType(uint8_t value) {
	DataStructure ds = getRecordDS();
	ds.ScreenSaverType = value&0xF;
	writeRecordDS(ds);
}

uint8_t ContactStore::RecordInfo::getScreenSaverType() {
	DataStructure ds =getRecordDS();
	return ds.ScreenSaverType;
}

void ContactStore::RecordInfo::setScreenSaverTime(uint8_t value) {
	DataStructure ds = getRecordDS();
	ds.ScreenSaverTime = value&0xF;
	writeRecordDS(ds);
}

uint8_t ContactStore::RecordInfo::getScreenSaverTime() {
	return getRecordDS().ScreenSaverTime;
}

void ContactStore::RecordInfo::setSleepTime(uint8_t n) {
	DataStructure ds = getRecordDS();
	ds.SleepTimer = n&0xF;
	writeRecordDS(ds);
}

uint8_t ContactStore::RecordInfo::getSleepTime() {
	return getRecordDS().SleepTimer;
}

bool ContactStore::RecordInfo::isUberBadge() {
	return getRecordDS().isUberBadge;
}


// MyInfo
//===========================================================
ContactStore::MyInfo::MyInfo(uint32_t startAddress) : StartAddress(startAddress) {
	
}

uint8_t *ContactStore::MyInfo::getPrivateKey() {
	return ((uint8_t*)(StartAddress+sizeof(uint16_t)+PUBLIC_KEY_LENGTH));
}

uint16_t ContactStore::MyInfo::getUniqueID() {
	return *((uint16_t*)(StartAddress));
}

const char *ContactStore::MyInfo::getAgentName() {
	return ((const char *)(StartAddress+sizeof(uint16_t)+PUBLIC_KEY_LENGTH+PRIVATE_KEY_LENGTH));
}

uint8_t *ContactStore::MyInfo::getPublicKey() {
	return ((uint8_t *)(StartAddress+sizeof(uint16_t)));
}

void ContactStore::MyInfo::setAgentname(const char name[AGENT_NAME_LENGTH]) {
	FLASH_LOCKER f;
	uint32_t s = StartAddress+sizeof(uint16_t)+PUBLIC_KEY_LENGTH+PRIVATE_KEY_LENGTH;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s,(*((uint32_t *)&name[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+4,(*((uint32_t *)&name[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+8,(*((uint32_t *)&name[8])));
}


//Contacts
//==================================================
ContactStore::Contact::Contact(uint32_t startAddress) : StartAddress(startAddress) {

}

ContactStore::Contact::Contact(const ContactStore::Contact &c) {
	StartAddress = c.StartAddress;
}

uint16_t ContactStore::Contact::getUniqueID() {
	return *((uint16_t*)StartAddress);
}
				
const char *ContactStore::Contact::getAgentName() {
	return ((char*)(StartAddress+sizeof(uint16_t)+PUBLIC_KEY_LENGTH+SIGNATURE_LENGTH));
}
				
uint8_t *ContactStore::Contact::getPublicKey() {
	return ((uint8_t*)(StartAddress+sizeof(uint16_t)));
}

uint8_t *ContactStore::Contact::getPairingSignature() {
	return ((uint8_t*)(StartAddress+sizeof(uint16_t)+PUBLIC_KEY_LENGTH));
}

void ContactStore::Contact::setUniqueID(uint16_t id) {
	FLASH_LOCKER f;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,StartAddress,id);
}

void ContactStore::Contact::setAgentname(const char name[AGENT_NAME_LENGTH]) {
	//int len = strlen(name);
	FLASH_LOCKER f;
	uint32_t s = StartAddress+sizeof(uint16_t)+PUBLIC_KEY_LENGTH+SIGNATURE_LENGTH;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s,(*((uint32_t *)&name[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+4,(*((uint32_t *)&name[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+8,(*((uint32_t *)&name[8])));
}

void ContactStore::Contact::setPublicKey(const uint8_t key[PUBLIC_KEY_LENGTH]) {
	uint32_t s = StartAddress+sizeof(uint16_t);
	FLASH_LOCKER f;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s,(*((uint32_t *)&key[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+4,(*((uint32_t *)&key[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+8,(*((uint32_t *)&key[8])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+12,(*((uint32_t *)&key[12])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+16,(*((uint32_t *)&key[16])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+20,(*((uint32_t *)&key[20])));
}

void ContactStore::Contact::setPairingSignature(const uint8_t sig[SIGNATURE_LENGTH]) {
	uint32_t s = StartAddress+sizeof(uint16_t)+PUBLIC_KEY_LENGTH;
	FLASH_LOCKER f;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s,(*((uint32_t *)&sig[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+4,(*((uint32_t *)&sig[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+8,(*((uint32_t *)&sig[8])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+12,(*((uint32_t *)&sig[12])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,s+16,(*((uint32_t *)&sig[16])));
}

//====================================================

ContactStore::MyInfo &ContactStore::getMyInfo() {
	return MeInfo;
}

ContactStore::RecordInfo &ContactStore::getRecordInfo() {
	return RecInfo;
}


//=============================================
ContactStore::ContactStore(uint32_t startAddr, uint32_t size) :  StartAddress(startAddr), StorageSize(size), RecInfo(startAddr),
	MeInfo(startAddr+RecordInfo::SIZE) {
	
}


bool ContactStore::init() {
	FLASH_LOCKER f;
	RecordInfo ri(StartAddress);
	//uint16_t temp = ri.getVersion();
	if(0xDCDC==ri.getVersion()) {
		return true;
	}
	return false;
}

bool ContactStore::addContact(uint16_t uid, char agentName[AGENT_NAME_LENGTH], uint8_t key[PUBLIC_KEY_LENGTH], uint8_t sig[SIGNATURE_LENGTH]) {
	uint8_t currentContacts = RecInfo.getNumContacts();
	uint8_t newNumContacts = RecInfo.setNumContacts(currentContacts+1);

	if(newNumContacts==currentContacts) {
		return false;
	}
	Contact c(0xFFFFFFFF);
	RecInfo.getContactAt(newNumContacts,c);
	c.setUniqueID(uid);
	c.setAgentname(agentName);
	c.setPublicKey(key);
	c.setPairingSignature(sig);
	return true;
}

uint8_t ContactStore::getNumContactsThatCanBeStored() {
	return MAX_CONTACTS;
}



