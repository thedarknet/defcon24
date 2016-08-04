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

	static const uint8_t MAX_CONTACTS = 66; //1024/88 bytes = 11 per sector * 6 sectors
	/////////////////////////////
	// Sector 57: erase then rotate though saving SettingInfo
	//				  	byte 0: 0xDC
	//					byte 1: 0xDC
	//					byte 2: 0
	//					byte 3: <number of contacts>
	//					byte 4: bits 0-3: screen save type
	//					byte 4: bits 4-7: sleep timer
	//					byte 5: bits 0-3: screen saver time
	//					byte 5: bits 4-7: ?
	//					byte 6-17: Agent Name
	// Sector 58:
	//				0-87 Contact 0
	//				...
	//				880-967 Contact 10
	//
	//	Sector 59:
	//				0-87 Contact 11
	//				...
	//				880-967 Contact 22
	//
	//	...
	//	Sector 63:
	//				0-87 Contact 55
	//				...
	//				880-967 Contact 66

	//			980-1020 My Info
	//					My Info Address[0-1] = 0xdcdc
	//					My Info Address[2-3] = radio unique id
	//					My Info Address[4-27] = badge owner private key
	//					My Info Address[28-29] = static settings
	//[ address book ]
	//		Contact
	//				[0-1] unique id
	//				[2-27] public key (compressed version - 25 (26) bytes)
	//				[28-75] Signature (contact signs you're id+public key)
	//				[76-87] Agent name
	//start address[(every 46 bytes)] = Contact1
	/////////////////////////////

	class Contact {
	public:
		friend class ContactStore;
		static const uint8_t SIZE = sizeof(uint16_t) + PUBLIC_KEY_LENGTH + SIGNATURE_LENGTH + AGENT_NAME_LENGTH;

		uint16_t getUniqueID();
		const char *getAgentName();
		uint8_t *getPublicKey();
		uint8_t *getPairingSignature();
		void setUniqueID(uint16_t id);
		void setAgentname(const char name[AGENT_NAME_LENGTH]);
		void setPublicKey(const uint8_t key[PUBLIC_KEY_LENGTH]);
		void setPairingSignature(const uint8_t sig[SIGNATURE_LENGTH]);
		Contact(uint32_t startAddress=0);
	protected:
		uint32_t StartAddress;
	};

	class SettingsInfo {
	public:
		static const uint8_t SIZE = 6+AGENT_NAME_LENGTH;
		struct DataStructure {
			uint32_t Reserved1 :8;
			uint32_t NumContacts :8;
			uint32_t ScreenSaverType :4;
			uint32_t SleepTimer :4;
			uint32_t ScreenSaverTime :4;
			uint32_t Reserved2 :4;
		};
	public:
		SettingsInfo(uint16_t SettingSector);
		bool init();
		uint16_t getVersion();
		uint8_t setNumContacts(uint8_t n);
		uint8_t getNumContacts();
		bool getContactAt(uint8_t n, Contact &c);
		bool setScreenSaverType(uint8_t value);
		uint8_t getScreenSaverType();
		bool setScreenSaverTime(uint8_t value);
		uint8_t getScreenSaverTime();
		bool setSleepTime(uint8_t n);
		uint8_t getSleepTime();
		const char *getAgentName();
		bool isNameSet();
		bool setAgentname(const char name[AGENT_NAME_LENGTH]);
		void resetToFactory();
	protected:
		bool writeSettings(const DataStructure &ds);
		DataStructure getSettings();
	private:
		uint16_t SettingSector;
		uint32_t StartAddress;
		uint32_t CurrentAddress;
		char AgentName[AGENT_NAME_LENGTH];
	};

	class MyInfo {
	public:
		//								0xdcdc			//radio id									//static settings
		static const uint8_t SIZE = sizeof(uint16_t) + sizeof(uint16_t) + PRIVATE_KEY_LENGTH + sizeof(uint16_t);
	public:
		bool init();
		uint16_t getUniqueID();
		uint8_t *getPublicKey();
		uint8_t *getPrivateKey();
		bool isUberBadge();
		uint8_t *getCompressedPublicKey();
		MyInfo(uint32_t startAddress);
	protected:
		uint16_t getFlags();
	private:
		uint32_t StartAddress;
	};

public:
	ContactStore(uint8_t SettingSector, uint8_t startingContactSector, uint8_t numContactSectors, uint32_t addressOfMyInfo);
	MyInfo &getMyInfo();
	SettingsInfo &getSettings();
	bool init();
	bool addContact(uint16_t uid, char agentName[AGENT_NAME_LENGTH], uint8_t key[PUBLIC_KEY_LENGTH],
			uint8_t sig[SIGNATURE_LENGTH]);
	uint8_t getNumContactsThatCanBeStored();
	bool getContactAt(uint16_t numContact, Contact &c);
	bool findContactByID(uint16_t uid, Contact &c);
	void resetToFactory();
private:
	SettingsInfo Settings;
	MyInfo MeInfo;
	uint8_t StartingContactSector;
	uint8_t NumContactSectors;
};

#endif
