#ifndef KEY_STORE_H
#define KEY_STORE_H

#include <stm32f1xx_hal.h>

class ContactStore {
	public:
		static const uint8_t PUBLIC_KEY_LENGTH = 24;
		static const uint8_t PRIVATE_KEY_LENGTH = 12;
		static const uint8_t DaemonPublic[PUBLIC_KEY_LENGTH];
		static const uint8_t AGENT_NAME_LENGTH = 12;
		static const uint8_t VersionOffset = 0;
		static const uint8_t NumRecordsOffset = 1;
		static const uint8_t ReseveOffset1 = 3;
		static const uint8_t CURRENT_VERSION = 0xDC;
		//start Address[0] = Version byte
		//start Address[1-2] = NumOfRecords
		//start Address[3] = Reserved
		//start Address[4-(4+sizeof(MyInfo))] - my info
		//start address[MyInfo+1] = array of contacts
	
		class RecordInfo {
			public:
				static const uint8_t SIZE = 3;
			public:
				RecordInfo(uint32_t start);
				uint8_t getVersion();
				uint16_t getNumRecords();
				void setNumRecords(uint16_t num);
			private:
					uint32_t StartAddress;
		};
	
	  
		class Contact {
			public:
				uint16_t getUniqueID();
				const char *getAgentName();
				uint8_t *getPublicKey();
				void setUniqueID(uint16_t id);
				void setAgentname(const char *name);
				void setPublicKey(uint8_t *key);
				Contact(uint32_t startAddress);
			protected:
				uint32_t StartAddress;
				//uint16_t UniqueID;
				//char AgentName[16];
				//uint8_t MyPublicKey[PUBLIC_KEY_LENGTH];
		};
	
		class MyInfo : public Contact {
			public:
				//uint8_t MyPrivateKey[PRIVATE_KEY_LENGTH];
				uint8_t *getPrivateKey();
				MyInfo(uint32_t startAddress);
		};
		static const uint8_t OffSetToFirstContact = sizeof(MyInfo);
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
		RecordInfo RecInfo;
		MyInfo MeInfo;
};

#endif
