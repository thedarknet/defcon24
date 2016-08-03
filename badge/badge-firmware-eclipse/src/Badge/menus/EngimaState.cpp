#include "EnigmaState.h"

////////////////////////////////////////////////////////////
EngimaState::EngimaState() :
		InternalState(SET_WHEEL), EntryBuffer(), Wheels(), EncryptResult(), DisplayOffset(0) {

}
EngimaState::~EngimaState() {

}

ErrorType EngimaState::onInit() {
	gui_set_curList(0);
	memset(&EntryBuffer[0], 0, sizeof(EntryBuffer));
	memset(&Wheels[0], 0, sizeof(Wheels));
	memset(&EncryptResult[0], 0, sizeof(EncryptResult));
	memset(&PlugBoard[0], 0, sizeof(PlugBoard));
	memset(&ResultHash[0], 0, sizeof(ResultHash));
	InternalState = SET_WHEEL;
	DisplayOffset = 0;
	getKeyboardContext().init(&Wheels[0], sizeof(Wheels));
	return ErrorType();
}

ReturnStateContext EngimaState::onRun(QKeyboard &kb) {
	StateBase* nextState = this;
	static uint32_t LastScrollTime = HAL_GetTick();
	switch (InternalState) {
	case SET_WHEEL:
		gui_lable_multiline("Enter password (for rotors)", 0, 10, 128, 64, 0, 0);
		kb.updateContext(getKeyboardContext());
		gui_lable_multiline(&Wheels[0], 0, 30, 128, 64, 0, 0);
		if (kb.getLastKeyReleased() == 11) {
			InternalState = PLUG_BOARD;
			getKeyboardContext().finalize();
			getKeyboardContext().init(&PlugBoard[0], sizeof(PlugBoard));
		} else if (kb.getLastKeyReleased() == 9) {
			nextState = StateFactory::getMenuState();
		}
		break;
	case PLUG_BOARD:
		gui_lable_multiline("Enter plug board pairs:", 0, 10, 128, 64, 0, 0);
		kb.updateContext(getKeyboardContext());
		gui_lable_multiline(&PlugBoard[0], 0, 30, 128, 64, 0, 0);
		if (kb.getLastKeyReleased() == 11) {
			InternalState = ENTER_MESSAGE;
			getKeyboardContext().finalize();
			getKeyboardContext().init(&EntryBuffer[0], sizeof(EntryBuffer));
		} else if (kb.getLastKeyReleased() == 9) {
			nextState = StateFactory::getMenuState();
		}
		break;
	case ENTER_MESSAGE: {
		gui_lable_multiline("Enter cipher text: ", 0, 10, 128, 64, 0, 0);
		kb.updateContext(getKeyboardContext());
		uint16_t offset =
				getKeyboardContext().getCursorPosition() > 37 ? getKeyboardContext().getCursorPosition() - 32 : 0;
		gui_lable_multiline(&EntryBuffer[offset], 0, 30, 128, 64, 0, 0);
		if (kb.getLastKeyReleased() == 11) {
			InternalState = DECRYPT;
			getKeyboardContext().finalize();
			crypt(&Wheels[0], &PlugBoard[0], strlen(&PlugBoard[0]), &EntryBuffer[0]);
			DisplayOffset = 0;
			LastScrollTime = HAL_GetTick();
		} else if (kb.getLastKeyReleased() == 9) {
			nextState = StateFactory::getMenuState();
		}
	}
		break;
	case DECRYPT: {
		gui_lable_multiline("Decodes to:", 0, 10, 128, 64, 0, 0);
		uint32_t decryptedLen = strlen(&EncryptResult[0]);
		if (decryptedLen > 48 && ((HAL_GetTick()-LastScrollTime)>500)) {
			LastScrollTime = HAL_GetTick();
			DisplayOffset = (DisplayOffset + 1) % decryptedLen;
		}
		gui_lable_multiline(&EncryptResult[DisplayOffset], 0, 20, 128, 64, 0, 0);
		if (kb.getLastKeyReleased() == 11) {
			InternalState = QUEST_COMPLETION;
			ShaOBJ sha;
			sha256_init(&sha);
			sha256_add(&sha, (const uint8_t*) getContactStore().getMyInfo().getPrivateKey(), 8);
			sha256_add(&sha, (const uint8_t*) &EncryptResult[0], strlen(&EncryptResult[0]));
			sha256_digest(&sha, &ResultHash[0]);
			memset(&EntryBuffer[0], 0, sizeof(EntryBuffer));
			sprintf(&EntryBuffer[0], "%02x%02x%02x%02x%02x%02x%02x%02x", ResultHash[0], ResultHash[1], ResultHash[2], ResultHash[3],
					ResultHash[4], ResultHash[5], ResultHash[6], ResultHash[7]);
		} else if (kb.getLastKeyReleased() == 9) {
			nextState = StateFactory::getMenuState();
		}
	}
		break;
	case QUEST_COMPLETION:
		gui_lable_multiline("Daemon code: ", 0, 10, 128, 64, 0, 0);
		gui_lable_multiline(&EntryBuffer[0], 0, 30, 128, 64, 0, 0);
		if (kb.getLastKeyReleased()==11) {
			nextState = StateFactory::getMenuState();
		}
		break;
	default:
		break;
	}
	return ReturnStateContext(nextState);
}

const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint32_t NUM_ROTORS = 13;
const char rotors[NUM_ROTORS][27] = { "DVOARQWTUZJCNFLSPMBHEYIGKX", "GHQZUJFWLVMTKOPIRSDEACXYBN",
		"AKUOCLVJYIXMQPERBWSNGFZHTD", "BKLOSUDPJIRHZEXCGQMNVYFATW", "LICFJPORWQVHANKEBUDYMGZXTS",
		"CAWFYLKXSZTGHPINMDREUQBJVO", "PYVREUXHKIWDNQAZTLSMBOJGFC", "LQRHNSTPAFIVJYMDGUOZKECWXB",
		"JAUMCWHXTIZDYORQNSKBEFGLPV", "VRKNGZQOUXTMDIECJYPFSAWBLH", "LUHMZRVEGYSPJFADQCWTKBNXIO",
		"SDIJUOBALVMYRNGWKHPQCXTFZE", "LIVPNYCUGSRFBXKQHMOEWZTDAJ" };

long EngimaState::mod26(long a) {
	return (a % 26 + 26) % 26;
}

int EngimaState::li(char l) {
// Letter index
	return l - 'A';
}

int EngimaState::indexof(const char* array, int find) {
	return strchr(array, find) - array;
}

void EngimaState::doPlug(char *r, const char *swapChars, int s) {
	for (int l = 0; l < s; l += 2) {
		int first = strchr(r, swapChars[l]) - r;
		if (first < 0)
			first = 0;
		int second = strchr(r, swapChars[l + 1]) - r;
		if (second < 0)
			second = 0;
		char tmp = r[first];
		r[first] = r[second];
		r[second] = tmp;
	}
}

int islower(int __c) {
	return __c >= 'a' && __c <= 'z';
}

int toupper(int __c) {
	return islower(__c) ? (__c & ~32) : __c;
}

const char* EngimaState::crypt(char *Wheels, const char *plugBoard, int plugBoardSize, const char *ct) {
	static const char reflector[] = "YRUHQSLDPXNGOKMIEBFZCWVJAT";
// Sets initial permutation
	int L = li(toupper(Wheels[1]));
	int M = li(toupper(Wheels[3]));
	int R = li(toupper(Wheels[5]));

	memset(&EncryptResult[0], 0, sizeof(EncryptResult));
	char *outPtr = &EncryptResult[0];

	int rotorIdx0 = li(toupper(Wheels[0])) % NUM_ROTORS;
	int rotorIdx1 = li(toupper(Wheels[2])) % NUM_ROTORS;
	int rotorIdx2 = li(toupper(Wheels[4])) % NUM_ROTORS;

	char r0[27] = { '\0' };
	strcpy(&r0[0], rotors[rotorIdx0]);
	doPlug(&r0[0], plugBoard, plugBoardSize);
	char r1[27] = { '\0' };
	strcpy(&r1[0], rotors[rotorIdx1]);
	doPlug(&r1[0], plugBoard, plugBoardSize);
	char r2[27] = { '\0' };
	strcpy(&r2[0], rotors[rotorIdx2]);
	doPlug(&r2[0], plugBoard, plugBoardSize);

	for (uint16_t x = 0; x < strlen(ct) && x < sizeof(EncryptResult); x++) {
		if (ct[x] == ' ')
			continue;

		int ct_letter = li(toupper(ct[x]));

		// Step right rotor on every iteration
		R = mod26(R + 1);

		// Pass through rotors
		char a = r2[mod26(R + ct_letter)];
		char b = r1[mod26(M + li(a) - R)];
		char c = r0[mod26(L + li(b) - M)];

		// Pass through reflector
		char ref = reflector[mod26(li(c) - L)];

		// Inverse rotor pass
		int d = mod26(indexof(&r0[0], alpha[mod26(li(ref) + L)]) - L);
		int e = mod26(indexof(&r1[0], alpha[mod26(d + M)]) - M);
		char f = alpha[mod26(indexof(&r2[0], alpha[mod26(e + R)]) - R)];

		*outPtr = f;
		outPtr++;
	}

	return &EncryptResult[0];
}

ErrorType EngimaState::onShutdown() {
	return ErrorType();
}
