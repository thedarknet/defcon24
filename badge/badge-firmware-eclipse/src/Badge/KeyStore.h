#ifndef KEY_STORE_H
#define KEY_STORE_H

#include <stm32f1xx_hal.h>

class ContactStore {
public:
#define THE_CURVE uECC_secp192r1()
	//should trait this in the future for different curves
	static const uint8_t PUBLIC_KEY_LENGTH = 48; //uncompressed size
	static const uint8_t PUBLIC_KEY_COMPRESSED_LENGTH = 25; //compressed size
	static const uint8_t PRIVATE_KEY_LENGTH = 24;
	static const uint8_t DaemonPublic[PUBLIC_KEY_LENGTH];
	static const uint8_t SIGNATURE_LENGTH = 48;
	static const uint8_t SIGNATURE_BYTES_USED = 16;

	static const uint8_t AGENT_NAME_LENGTH = 12;
	static const uint8_t CURRENT_VERSION = 0xDC;

	static const uint8_t MAX_CONTACTS = 68; //6K/88 bytes
	/////////////////////////////
	//start Address[0-1] = Version byte
	//  	Address[2-3] = 	byte 2: ? - use for uber badge?
	//						byte 3: Number of contacts
	//		address[4-5] = Badge Settings
	//					byte 1: bits 0-3: screen save type
	//					byte 1: bits 4-7: sleep timer
	//					byte 2: bits 0-3: screen saver time
	//					byte 2: bits 4-7: ?
	//[ my info ]
	//start Address[6-7] = radio unique id
	//start Address[8-31] = badge owner private key
	//start Address[32-43] = badge owner agent name
	//[ address book ]
	//start address[44-132] = Contact0
	//				[0-1] unique id
	//				[2-27] public key (compressed version - 25 (26) bytes)
	//				[28-75] Signature (contact signs you're id+public key)
	//				[76-87] Agent name
	//start address[(every 46 bytes)] = Contact1
	/////////////////////////////

	class Contact {
	public:
		static const uint8_t SIZE = sizeof(uint16_t) + PUBLIC_KEY_LENGTH + SIGNATURE_LENGTH + AGENT_NAME_LENGTH;

		uint16_t getUniqueID();
		const char *getAgentName();
		uint8_t *getPublicKey();
		uint8_t *getPairingSignature();
		void setUniqueID(uint16_t id);
		void setAgentname(const char name[AGENT_NAME_LENGTH]);
		void setPublicKey(const uint8_t key[PUBLIC_KEY_LENGTH]);
		void setPairingSignature(const uint8_t sig[SIGNATURE_LENGTH]);
		Contact(uint32_t startAddress);
		Contact(const Contact &c);
	protected:
		uint32_t StartAddress;
	};

	class RecordInfo {
	public:
		static const uint8_t SIZE = 6;
		struct DataStructure {
			uint32_t isUberBadge :1;
			uint32_t Reserved1 :7;
			uint32_t NumContacts :8;
			uint32_t ScreenSaverType :4;
			uint32_t SleepTimer :4;
			uint32_t ScreenSaverTime :4;
			uint32_t Reserved2 :4;
		};
	public:
		RecordInfo(uint32_t start);
		uint16_t getVersion();
		uint8_t setNumContacts(uint8_t n);
		uint8_t getNumContacts();
		bool getContactAt(uint8_t n, Contact &c);
		void setScreenSaverType(uint8_t value);
		uint8_t getScreenSaverType();
		void setScreenSaverTime(uint8_t value);
		uint8_t getScreenSaverTime();
		void setSleepTime(uint8_t n);
		uint8_t getSleepTime();
		bool isUberBadge();
	protected:
		uint32_t getContactStartAddress();
		DataStructure getRecordDS();
		void writeRecordDS(const DataStructure &ds);
	private:
		uint32_t StartAddress;
	};

	class MyInfo {
	public:
		static const uint8_t SIZE = sizeof(uint16_t) + PUBLIC_KEY_LENGTH + PRIVATE_KEY_LENGTH + AGENT_NAME_LENGTH;
	public:
		uint16_t getUniqueID();
		const char *getAgentName();
		uint8_t *getPublicKey();
		void setAgentname(const char name[AGENT_NAME_LENGTH]);
		uint8_t *getPrivateKey();
		MyInfo(uint32_t startAddress);
	private:
		uint32_t StartAddress;
	};

public:
	ContactStore(uint32_t startAddr, uint32_t size);
	MyInfo &getMyInfo();
	RecordInfo &getRecordInfo();
	bool init();
	bool addContact(uint16_t uid, char agentName[AGENT_NAME_LENGTH], uint8_t key[PUBLIC_KEY_LENGTH],
			uint8_t sig[SIGNATURE_LENGTH]);
	uint8_t getNumContactsThatCanBeStored();
private:
	uint32_t StartAddress;
	uint32_t StorageSize;
	RecordInfo RecInfo;
	MyInfo MeInfo;
};

#endif
