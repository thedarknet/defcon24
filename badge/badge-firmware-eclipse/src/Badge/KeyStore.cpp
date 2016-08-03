#include "KeyStore.h"
#include <string.h>
#include <uECC.h>

const uint8_t ContactStore::DaemonPublic[ContactStore::PUBLIC_KEY_LENGTH] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const uint32_t CONTACTS_PER_PAGE = FLASH_PAGE_SIZE / ContactStore::Contact::SIZE;

class FLASH_LOCKER {
public:
	FLASH_LOCKER() {
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP| FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);
	}
	~FLASH_LOCKER() {
		HAL_FLASH_Lock();
	}
};

#define SECTOR_TO_ADDRESS(sector) (FLASH_BASE + (FLASH_PAGE_SIZE*sector))

ContactStore::SettingsInfo::SettingsInfo(uint16_t sector) :
		SettingSector(sector), StartAddress(SECTOR_TO_ADDRESS(sector)), AgentName() {
	CurrentAddress = StartAddress;
	memset(&AgentName[0], 0, sizeof(AgentName));
}

bool ContactStore::SettingsInfo::init() {
	for (uint32_t addr = StartAddress; addr < (StartAddress + FLASH_PAGE_SIZE); addr += SettingsInfo::SIZE) {
		uint16_t value = *((uint16_t*) addr);
		if (value == 0xDCDC) {
			CurrentAddress = addr;
			const char *AgentNameAddr = ((const char *) (CurrentAddress + sizeof(uint16_t) + sizeof(uint32_t)));
			strncpy(&AgentName[0], AgentNameAddr, sizeof(AgentName));
			return true;
		}
	}
	//couldn't find DS
	CurrentAddress = StartAddress + FLASH_PAGE_SIZE; //force a page erase
	DataStructure ds;
	ds.Reserved1 = 0;
	ds.Reserved2 = 0;
	ds.ScreenSaverTime = 1;
	ds.ScreenSaverType = 0;
	ds.SleepTimer = 3;
	ds.NumContacts = 0;
	return writeSettings(ds);
}

bool ContactStore::SettingsInfo::setAgentname(const char name[AGENT_NAME_LENGTH]) {
	strncpy(&AgentName[0], &name[0], sizeof(AgentName));
	DataStructure ds = getSettings();
	return writeSettings(ds);
}

const char *ContactStore::SettingsInfo::getAgentName() {
	return &AgentName[0];
}

uint16_t ContactStore::SettingsInfo::getVersion() {
	return *((uint16_t*) CurrentAddress);
}

uint8_t ContactStore::SettingsInfo::getNumContacts() {
	DataStructure ds = getSettings();
	return ds.NumContacts;
}

ContactStore::SettingsInfo::DataStructure ContactStore::SettingsInfo::getSettings() {
	return *((ContactStore::SettingsInfo::DataStructure*) (CurrentAddress + sizeof(uint16_t)));
}

void ContactStore::SettingsInfo::resetToFactory() {
	{
		FLASH_LOCKER f;
		FLASH_EraseInitTypeDef EraseInitStruct;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.Banks = FLASH_BANK_1;
		EraseInitStruct.PageAddress = StartAddress;
		EraseInitStruct.NbPages = 1;
		uint32_t SectorError = 0;

		HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
	}
	init();
}

bool ContactStore::SettingsInfo::writeSettings(const DataStructure &ds) {
	FLASH_LOCKER f;
	uint32_t startNewAddress = CurrentAddress + SettingsInfo::SIZE;
	uint32_t endNewAddress = startNewAddress + SettingsInfo::SIZE;
	if (endNewAddress >= (StartAddress + FLASH_PAGE_SIZE)) {
		FLASH_EraseInitTypeDef EraseInitStruct;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.Banks = FLASH_BANK_1;
		EraseInitStruct.PageAddress = StartAddress;
		EraseInitStruct.NbPages = 1;
		uint32_t SectorError = 0;

		if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
			return false;
		}
		CurrentAddress = StartAddress;
		startNewAddress = CurrentAddress;
		endNewAddress = CurrentAddress + SettingsInfo::SIZE;
	} else {
		//zero out the one we were on
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, CurrentAddress, 0); //2
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, CurrentAddress + 2, 0); //4
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, CurrentAddress + 6, 0);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, CurrentAddress + 10, 0);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, CurrentAddress + 14, 0);
		CurrentAddress = startNewAddress;
	}
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, CurrentAddress, 0xDCDC);
	uint32_t data = *((uint32_t*) &ds);
	if (HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (CurrentAddress + sizeof(uint16_t)), data)) {
		uint32_t agentStart = CurrentAddress + sizeof(uint16_t) + sizeof(uint32_t);
		if (HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, agentStart, (*((uint32_t *) &AgentName[0])))) {
			if (HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, agentStart + 4, (*((uint32_t *) &AgentName[4])))) {
				if (HAL_OK
						== HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, agentStart + 8, (*((uint32_t *) &AgentName[8])))) {
					return true;
				}
			}
		}
	}
	return false;
}

uint8_t ContactStore::SettingsInfo::setNumContacts(uint8_t num) {
	if (num > MAX_CONTACTS)
		return MAX_CONTACTS;
	DataStructure ds = getSettings();
	ds.NumContacts = num;
	writeSettings(ds);
	return num;
}

bool ContactStore::SettingsInfo::setScreenSaverType(uint8_t value) {
	DataStructure ds = getSettings();
	ds.ScreenSaverType = value & 0xF;
	return writeSettings(ds);
}

uint8_t ContactStore::SettingsInfo::getScreenSaverType() {
	DataStructure ds = getSettings();
	return ds.ScreenSaverType;
}

bool ContactStore::SettingsInfo::setScreenSaverTime(uint8_t value) {
	DataStructure ds = getSettings();
	ds.ScreenSaverTime = value & 0xF;
	return writeSettings(ds);
}

uint8_t ContactStore::SettingsInfo::getScreenSaverTime() {
	return getSettings().ScreenSaverTime+1;
}

bool ContactStore::SettingsInfo::setSleepTime(uint8_t n) {
	DataStructure ds = getSettings();
	ds.SleepTimer = n & 0xF;
	return writeSettings(ds);
}

uint8_t ContactStore::SettingsInfo::getSleepTime() {
	return getSettings().SleepTimer;
}

// MyInfo
//===========================================================
ContactStore::MyInfo::MyInfo(uint32_t startAddress) :
		StartAddress(startAddress) {

}

bool ContactStore::MyInfo::init() {
	return (*(uint16_t*) StartAddress) == 0xdcdc;
}

uint8_t *ContactStore::MyInfo::getPrivateKey() {
	return ((uint8_t*) (StartAddress + sizeof(uint16_t) + sizeof(uint16_t)));
}

uint16_t ContactStore::MyInfo::getUniqueID() {
	return *((uint16_t*) (StartAddress + sizeof(uint16_t)));
}

//TODO make this a member var of MyInfo
uint8_t publicKey[ContactStore::PUBLIC_KEY_LENGTH] = { 0 };
uint8_t *ContactStore::MyInfo::getPublicKey() {
	//given our private key, generate our public key and ensure its good
	if (uECC_compute_public_key(getPrivateKey(), &publicKey[0], THE_CURVE)) {
		if (uECC_valid_public_key(&publicKey[0], THE_CURVE) == 1) {
			return &publicKey[0];
		}
	}
	return 0;
}

uint8_t compressedPublicKey[ContactStore::PUBLIC_KEY_COMPRESSED_LENGTH] = { 0 };
uint8_t *ContactStore::MyInfo::getCompressedPublicKey() {
	uECC_compress(getPublicKey(), &compressedPublicKey[0], THE_CURVE);
	return &compressedPublicKey[0];
}

ContactStore::Contact::Contact(uint32_t startAddr) :
		StartAddress(startAddr) {
}

uint16_t ContactStore::Contact::getUniqueID() {
	return *((uint16_t*) StartAddress);
}

const char *ContactStore::Contact::getAgentName() {
	return ((char*) (StartAddress + sizeof(uint16_t) + PUBLIC_KEY_LENGTH + SIGNATURE_LENGTH));
}

uint8_t *ContactStore::Contact::getPublicKey() {
	return ((uint8_t*) (StartAddress + sizeof(uint16_t)));
}

uint8_t *ContactStore::Contact::getPairingSignature() {
	return ((uint8_t*) (StartAddress + sizeof(uint16_t) + PUBLIC_KEY_LENGTH));
}

void ContactStore::Contact::setUniqueID(uint16_t id) {
	FLASH_LOCKER f;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, StartAddress, id);
}

void ContactStore::Contact::setAgentname(const char name[AGENT_NAME_LENGTH]) {
	//int len = strlen(name);
	FLASH_LOCKER f;
	uint32_t s = StartAddress + sizeof(uint16_t) + PUBLIC_KEY_LENGTH + SIGNATURE_LENGTH;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s, (*((uint32_t *) &name[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 4, (*((uint32_t *) &name[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 8, (*((uint32_t *) &name[8])));
}

void ContactStore::Contact::setPublicKey(const uint8_t key[PUBLIC_KEY_LENGTH]) {
	uint32_t s = StartAddress + sizeof(uint16_t);
	FLASH_LOCKER f;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s, (*((uint32_t *) &key[0])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 4, (*((uint32_t *) &key[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 8, (*((uint32_t *) &key[8])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 12, (*((uint32_t *) &key[12])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 16, (*((uint32_t *) &key[16])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 20, (*((uint32_t *) &key[20])));
}

void ContactStore::Contact::setPairingSignature(const uint8_t sig[SIGNATURE_LENGTH]) {
	uint32_t s = StartAddress + sizeof(uint16_t) + PUBLIC_KEY_LENGTH;
	FLASH_LOCKER f;
	//for(uint32_t i=0;i<sizeof(sig);i+=4) {
	//	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s+i, (*((uint32_t *) &sig[i])));
	//}
	//if you look at the assembler being generated the loop will only run twice so we'll run roll it due to time
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 4, (*((uint32_t *) &sig[4])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 8, (*((uint32_t *) &sig[8])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 12, (*((uint32_t *) &sig[12])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 16, (*((uint32_t *) &sig[16])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 20, (*((uint32_t *) &sig[20])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 24, (*((uint32_t *) &sig[24])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 28, (*((uint32_t *) &sig[28])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 32, (*((uint32_t *) &sig[32])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 36, (*((uint32_t *) &sig[36])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 40, (*((uint32_t *) &sig[40])));
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, s + 44, (*((uint32_t *) &sig[44])));
}

//====================================================

ContactStore::MyInfo &ContactStore::getMyInfo() {
	return MeInfo;
}

ContactStore::SettingsInfo &ContactStore::getSettings() {
	return Settings;
}

//=============================================
ContactStore::ContactStore(uint8_t SettingSector, uint8_t startingContactSector, uint8_t numContactSectors,
		uint32_t addressOfMyInfo) :
		Settings(SettingSector), MeInfo(addressOfMyInfo), StartingContactSector(startingContactSector), NumContactSectors(
				numContactSectors) {

}

void ContactStore::resetToFactory() {
	getSettings().resetToFactory();
	{
		FLASH_LOCKER f;
		FLASH_EraseInitTypeDef EraseInitStruct;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.Banks = FLASH_BANK_1;
		EraseInitStruct.PageAddress = SECTOR_TO_ADDRESS(StartingContactSector);
		EraseInitStruct.NbPages = NumContactSectors - 1;
		uint32_t SectorError = 0;

		HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
	}
}

bool ContactStore::init() {
	if (Settings.init() && getMyInfo().init()) {
		//version is good, validate keys:
		uint8_t publicKey[PUBLIC_KEY_LENGTH] = { 0 };
		//given our private key, generate our public key and ensure its good
		if (uECC_compute_public_key(MeInfo.getPrivateKey(), &publicKey[0], THE_CURVE)) {
			if (uECC_valid_public_key(&publicKey[0], THE_CURVE) == 1) {
				return true;
			}
		}
	}
	return false;
}

bool ContactStore::getContactAt(uint16_t numContact, Contact &c) {
	uint8_t currentContacts = Settings.getNumContacts();
	if (numContact < currentContacts) {
		//determine page
		uint16_t sector = StartingContactSector + numContact / CONTACTS_PER_PAGE;
		//valid page is in our range
		if (sector < (StartingContactSector + NumContactSectors)) {
			uint16_t offSet = numContact % CONTACTS_PER_PAGE;
			uint32_t sectorAddress = SECTOR_TO_ADDRESS(sector);
			c.StartAddress = sectorAddress + (offSet * Contact::SIZE);
			return true;
		}
	}
	return false;
}

bool ContactStore::findContactByID(uint16_t uid, Contact &c) {
	if(uid==0) return false;
	uint8_t currentContacts = Settings.getNumContacts();
	for(int i=0;i<currentContacts;i++) {
		if(getContactAt(i,c)) {
			if(uid==c.getUniqueID()) {
				return true;
			}
		}
	}
	c.StartAddress = 0;
	return false;
}

bool ContactStore::addContact(uint16_t uid, char agentName[AGENT_NAME_LENGTH], uint8_t key[PUBLIC_KEY_LENGTH],
		uint8_t sig[SIGNATURE_LENGTH]) {
	uint8_t currentContacts = Settings.getNumContacts();
	uint8_t newNumContacts = Settings.setNumContacts(currentContacts + 1);

	if (newNumContacts == currentContacts) {
		return false;
	}
	Contact c(0xFFFFFFFF);
	if (getContactAt(currentContacts, c)) {
		c.setUniqueID(uid);
		c.setAgentname(agentName);
		c.setPublicKey(key);
		c.setPairingSignature(sig);
		return true;
	}
	return false;
}

uint8_t ContactStore::getNumContactsThatCanBeStored() {
	return MAX_CONTACTS;
}

