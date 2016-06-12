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
	
		static const uint16_t MAX_CONTACTS = 300;
		/////////////////////////////
		//start Address[0-1] = Version byte
		//[ my info ]
		//start Address[2-3] = radio unique id
		//start Address[4-27] = badge owner public key
		//start Address[28-39] = badge owner private key
		//start Address[40-51] = badge owner agent name
		//[ my info ]
		//start address[52-89] = Contact0
		//start address[(every 38 bytes)] = Contact1
		//start address[] - Contact N
		/////////////////////////////
	
		class RecordInfo {
			public:
				static const uint8_t SIZE = 2;
			public:
				RecordInfo(uint32_t start);
				uint16_t getVersion();
				uint16_t getNumRecords();
				uint32_t getStartContactsAddress();
				bool isFull();
			private:
				uint32_t StartAddress;
		};
	  
		class Contact {
			public:
				static const uint8_t SIZE = sizeof(uint16_t) + AGENT_NAME_LENGTH + PUBLIC_KEY_LENGTH;
				uint16_t getUniqueID();
				const char *getAgentName();
				uint8_t *getPublicKey();
				void setUniqueID(uint16_t id);
				void setAgentname(const char name[AGENT_NAME_LENGTH]);
				void setPublicKey(const uint8_t key[PUBLIC_KEY_LENGTH]);
				Contact(uint32_t startAddress);
			protected:
				uint32_t StartAddress;
		};
	
		class MyInfo : public Contact {
			public:
				static const uint8_t SIZE = Contact::SIZE + PRIVATE_KEY_LENGTH;
			public:
				uint8_t *getPrivateKey();
				MyInfo(uint32_t startAddress);
		};
		
		static const uint8_t OffSetToFirstContact = sizeof(MyInfo);
	public:
		ContactStore(uint32_t startAddr, uint32_t size);
		MyInfo &getMyInfo();
		RecordInfo &getRecordInfo();
		bool init();
		bool addContact(uint16_t uid, char agentName[AGENT_NAME_LENGTH], uint8_t key[PUBLIC_KEY_LENGTH]);
		uint16_t getNumContactsThatCanBeStored();
		uint16_t getCurrentStoredContactCount();
	protected:
		uint32_t getStartContactsAddress();
	private:
		uint32_t StartAddress;
		uint32_t StorageSize;
		RecordInfo RecInfo;
		MyInfo MeInfo;
};

#endif
