#ifndef KEY_STORE_H
#define KEY_STORE_H

#include <stm32f1xx_hal.h>

class ContactStore {
	public:
		static const uint8_t PUBLIC_KEY_LENGTH = 24;
		static const uint8_t PRIVATE_KEY_LENGTH = 12;
		static const uint8_t DaemonPublic[PUBLIC_KEY_LENGTH];
		static const uint8_t VersionOffset = 0;
		static const uint8_t NumRecordsOffset = 1;
		static const uint8_t ReseveOffset1 = 2;
		static const uint8_t ReseveOffset2 = 3;
	
		struct Contact {
			uint16_t UniqueID;
			char AgentName[16];
			uint8_t MyPublicKey[PUBLIC_KEY_LENGTH];
			uint8_t MyPrivateKey[PRIVATE_KEY_LENGTH];
			Contact();
		};
	public:
		ContactStore(uint32_t startAddr, uint32_t size);
		bool init();
		bool addContact(const Contact &c);
		bool removeContact(const char *AgentName);
		uint16_t getNumContactsThatCanBeStored();
		uint16_t getCurrentStoredContactCount();
	private:
		uint32_t StartAddress;
		uint32_t StorageSize;
		uint16_t CurrentStoredContacts;
};

#endif