//============================================================================
// Name        : BadgeGen2.cpp
// Author      : CmdC0de
// Version     :
// Copyright   : DCDarkNet Industries LLC  all right reserved
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "sha256.h"
#include <uECC.h>
#include <memory.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

using namespace std;

#if 1

uECC_Curve theCurve = uECC_secp192r1();
#define PRIVATE_KEY_SIZE = 24;
#define PUBLIC_KEY = 48;
#define COMPRESSED_PUBLIC_KEY = 25;
#define SIGNATURE_SIZE = 48;
#define BYTES_OF_SIGNATURE_TO_USE = 16;

bool makeKey(uint8_t privKey[24], uint8_t pubKey[48], uint8_t compressPub[26]) {
	memset(&privKey[0], 0, 24);
	memset(&pubKey[0], 0, 48);
	memset(&compressPub[0], 0, 26);
	if (uECC_make_key(pubKey, privKey, theCurve) == 1) {

		uECC_compress(pubKey, compressPub, theCurve);

		if (uECC_valid_public_key(pubKey, theCurve) == 1) {
			return true;
		}
	}
	return false;
}

void printKeys(uint8_t privKey[24], uint8_t pubKey[26]) {
	cout << "PrivateKey:" << endl;
	cout << "\t";
	for (int i = 0; i < 24; i++) {
		if (i != 0) {
			cout << ":";
		}
		cout << setfill('0') << setw(2) << hex << (int) privKey[i] << dec;
	}
	cout << endl;
	cout << "PublicKey:" << endl;
	cout << "\t";
	for (int i = 0; i < 26; i++) {
		if (i != 0) {
			cout << ":";
		}
		cout << setfill('0') << setw(2) << hex << (int) pubKey[i] << dec;
	}
	cout << endl;
}

bool exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

void usage() {
	cout
			<< "BadgeGen -u <make uber init file> -c <create daemon keys> -n <number of badge keys to generate> -w <3 letter string to set wheels> -m <message to encrypt/decrypt>"
			<< endl;
}

const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint32_t NUM_ROTORS = 13;
const char rotors[NUM_ROTORS][27] = { "DVOARQWTUZJCNFLSPMBHEYIGKX", "GHQZUJFWLVMTKOPIRSDEACXYBN",
		"AKUOCLVJYIXMQPERBWSNGFZHTD", "BKLOSUDPJIRHZEXCGQMNVYFATW", "LICFJPORWQVHANKEBUDYMGZXTS",
		"CAWFYLKXSZTGHPINMDREUQBJVO", "PYVREUXHKIWDNQAZTLSMBOJGFC", "LQRHNSTPAFIVJYMDGUOZKECWXB",
		"JAUMCWHXTIZDYORQNSKBEFGLPV", "VRKNGZQOUXTMDIECJYPFSAWBLH", "LUHMZRVEGYSPJFADQCWTKBNXIO",
		"SDIJUOBALVMYRNGWKHPQCXTFZE", "LIVPNYCUGSRFBXKQHMOEWZTDAJ" };

long mod26(long a) {
	return (a % 26 + 26) % 26;
}

int li(char l) {
	// Letter index
	return l - 'A';
}

int indexof(const char* array, int find) {
	return strchr(array, find) - array;
}

void doPlug(char *r, const char *swapChars, int s) {
	for (int l = 0; l < s; l += 2) {
		int first = strchr(r, swapChars[l]) - r;
		int second = strchr(r, swapChars[l + 1]) - r;
		char tmp = r[first];
		r[first] = r[second];
		r[second] = tmp;
	}
}

char EncryptResult[200];

const char* crypt(char *Wheels, const char *plugBoard, int plugBoardSize, const char *ct) {
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
		if (isspace(ct[x]))
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

int main(int argc, char *argv[]) {
	char create = 0, generate = 0, makeUber = 0;

	uint8_t privateKey[24] = { 0x00 };
	uint8_t unCompressPubKey[48] = { 0x00 };
	uint8_t compressPubKey[26] = { 0x00 }; //only need 25
	uint8_t RadioID[2];
	uint8_t Signature[48];
	char *wheels = 0;
	char *msg = 0;
	char *plugBoard = 0;

	int ch = 0;
	int numberToGen = 0;

	while ((ch = getopt(argc, argv, "eucn:w:m:p:")) != -1) {
		switch (ch) {
		case 'c':
			create = 1;
			break;
		case 'n':
			numberToGen = atoi(optarg);
			generate = 1;
			break;
		case 'p':
			plugBoard = optarg;
			if (strlen(plugBoard) % 2 != 0) {
				usage();
				return -1;
			}
			break;
		case 'u':
			makeUber = 1;
			break;
		case 'w':
			wheels = optarg;
			if (strlen(wheels) != 6) {
				usage();
				return -1;
			}
			break;
		case 'm':
			msg = optarg;
			break;
		case '?':
		default:
			usage();
			return -1;
			break;
		}
	}

	if (1 == create) {
		if (makeKey(privateKey, unCompressPubKey, compressPubKey)) {
			printKeys(privateKey, compressPubKey);
		} else {
			cerr << "Error generating key" << endl;
		}
	} else if (1 == generate) {
		for (int i = 0; i < numberToGen; i++) {
			if (makeKey(privateKey, unCompressPubKey, compressPubKey)) {
				uECC_RNG_Function f = uECC_get_rng();
				f(&RadioID[0], 2);
				cout << "RadioID: " << endl;
				cout << "\t" << setfill('0') << setw(2) << hex << (int) RadioID[0] << dec << ":";
				cout << setfill('0') << setw(2) << hex << (int) RadioID[1] << dec << endl;
				printKeys(privateKey, compressPubKey);
				cout << endl;
				cout << endl;
				ostringstream oss;
				oss << setfill('0') << setw(2) << hex << (int) RadioID[0] << (int) RadioID[1] << ends;
				std::string fileName = oss.str();
				if (!exists("./keys")) {
					mkdir("./keys", 0700);
				}
				std::string fullFileName = "./keys/" + fileName;
				if (exists(fullFileName)) {
					numberToGen++;
				} else {
					//see keystore.h for format
					static const unsigned int defaults1 = 0b00100001; //screen saver type = 1 sleep time = 2
					static const unsigned int defaults2 = 0b00000001; //screen saver time = 1
					unsigned char reserveFlags = makeUber == 1 ? 0x1 : 0x0;
					char agentName[12] = { '\0' };
					ofstream of(fullFileName.c_str());
					//                   			magic 	magic	reserved	Num Contacts 		settings 1		Settings 2
					const unsigned char magic[6] = { 0xDC, 0xDC, reserveFlags, 0x0, defaults1, defaults2 };
					of.write((const char *) &magic[0], sizeof(magic));
					of.write((const char *) &RadioID[0], sizeof(RadioID));
					//of.write((const char *)&compressPubKey[0], sizeof(compressPubKey));
					of.write((const char *) &privateKey[0], sizeof(privateKey));
					of.write(&agentName[0], sizeof(agentName));  //just zero-ing out memory
					of.flush();
				}
			}
		}
	} else if (wheels != 0) {
		cout << crypt(wheels, plugBoard, strlen(plugBoard), msg) << endl;
	} else {
		usage();
	}
	return 0;
}
#else

int main() {
	cout << "uECC_secp160r1" << endl;
	cout << uECC_curve_private_key_size(uECC_secp160r1()) << endl;
	cout << uECC_curve_public_key_size(uECC_secp160r1()) << endl;
	cout << "uECC_secp192r1" << endl;
	cout << uECC_curve_private_key_size(uECC_secp192r1()) << endl;
	cout << uECC_curve_public_key_size(uECC_secp192r1()) << endl;
	cout << "uECC_secp224r1" << endl;
	cout << uECC_curve_private_key_size(uECC_secp224r1()) << endl;
	cout << uECC_curve_public_key_size(uECC_secp224r1()) << endl;

	cout << "gen key pair" << endl;
#if 1
	uECC_Curve theCurve = uECC_secp192r1();
#else
	uECC_Curve theCurve = uECC_secp160r1();
#endif

	int pubKeyBufSize = uECC_curve_public_key_size(theCurve) * 2;
	int privKeyBufSize = uECC_curve_private_key_size(theCurve);
	uint8_t *pubKey = new unsigned char[pubKeyBufSize];
	memset(pubKey, 0, pubKeyBufSize);
	uint8_t *privKey = new uint8_t[privKeyBufSize];
	memset(privKey, 0, privKeyBufSize);
	int pubKeyCompressSize = privKeyBufSize + 1;
	uint8_t *comPubKey = new uint8_t[pubKeyCompressSize];

	if (uECC_make_key(pubKey, privKey, theCurve) == 1) {
		for (int i = 0; i < pubKeyBufSize; i++) {
			cout << hex << (int) pubKey[i] << " ";
		}
		cout << endl;

		uECC_compress(pubKey, comPubKey, theCurve);
		for (int i = 0; i < pubKeyCompressSize; i++) {
			cout << hex << (int) comPubKey[i] << " ";
		}
		cout << endl;

		if (uECC_valid_public_key(pubKey, theCurve) == 1) {
			cout << "valid pub key" << endl;
		}

		ShaOBJ shaCtx;
		sha256_init(&shaCtx);
		const char *msg = "this is my message";
		sha256_add(&shaCtx,(const uint8_t *)msg,strlen(msg));
		uint8_t digest[32];
		sha256_digest(&shaCtx,digest);

		int signatureSize = privKeyBufSize*2;
		uint8_t *signature = new uint8_t[signatureSize];
		memset(signature,0,signatureSize);

		if(uECC_sign(privKey,&digest[0],sizeof(digest),signature,theCurve)==1) {
			for(int i=0;i<signatureSize;i++) {
				cout << hex << (int)signature[i];
			}
			cout << dec << endl;
			if(uECC_verify(pubKey,&digest[0],sizeof(digest),signature,theCurve)==1) {
				cout << "signature check!" << endl;
			}
		}
		printf("Private Key Size: %u\n",privKeyBufSize);
		printf("Compressed pub key size: %u\n",privKeyBufSize+1);
		printf("Signature: %u\n",signatureSize);
		int total = privKeyBufSize+1+2+signatureSize+12;
		printf("total for pairing: %u\n",total);
		int canFit = (1024*10)/total;
		printf("Can fit: %u\n",canFit);
		int canFitSmallerSig = (1024*10)/(privKeyBufSize+1+2+16+12);
		cout << "Can fit smaller sig: " << canFitSmallerSig << endl;

	} else {
		cout << "keygen failed" << endl;
	}
	return 0;
}
#endif
