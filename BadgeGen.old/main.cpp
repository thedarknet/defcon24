#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include "cryptopp/pubkey.h"
#include "cryptopp/base64.h"
#include "cryptopp/sha.h"
#include "cryptopp/eccrypto.h"
#include "cryptopp/osrng.h"
#include "cryptopp/oids.h"
#include "cryptopp/files.h"
#include "cryptopp/seckey.h"
#include "cryptopp/modes.h"

typedef CryptoPP::ECDSA<CryptoPP::EC2N, CryptoPP::SHA1>::PrivateKey PrivateECCKeyType;
typedef CryptoPP::ECDSA<CryptoPP::EC2N, CryptoPP::SHA1>::PublicKey PubECCKeyType;
typedef CryptoPP::ECDSA<CryptoPP::EC2N, CryptoPP::SHA1>::Signer PubECCSignerType;
typedef CryptoPP::ECDSA<CryptoPP::EC2N, CryptoPP::SHA1>::Verifier PubECCVerifierType;
typedef CryptoPP::FileSink FileSink;

static byte iv[CryptoPP::AES::BLOCKSIZE] = { '\0' };
static CryptoPP::AutoSeededRandomPool random_pool;

#define DEBUGMSG(a) std::cout << a << std::endl
#define WARNMSG(a) std::cout << a << std::endl
#define INFOMSG(a) std::cout << a << std::endl

bool exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

static void setUpIV(const char *productName) {
	CryptoPP::SHA sha;
	byte prep[CryptoPP::SHA::DIGESTSIZE];
	sha.CalculateDigest(&prep[0], (const byte *) productName,
			strlen(productName));
	memcpy(&iv[0], &prep[0], CryptoPP::AES::BLOCKSIZE);
}

static void GenerateKey(byte key[CryptoPP::SHA::DIGESTSIZE],
		const std::string &password) {
	CryptoPP::SHA sha;
	sha.CalculateDigest(&key[0], (const byte *) password.c_str(),
			password.length());
}

bool createDaemonKeyPair(const char *password,
		const std::string &privateKeyFileName,
		const std::string &publicKeyFileName) {
	PrivateECCKeyType privateKey;
	privateKey.Initialize(random_pool, CryptoPP::ASN1::secp160r1());
	if (privateKey.Validate(random_pool, 3)) {
		DEBUGMSG("private Exponent:= " << privateKey.GetPrivateExponent());
		PubECCKeyType publicKey;
		privateKey.MakePublicKey(publicKey);
		if (publicKey.Validate(random_pool, 3)) {
			DEBUGMSG(
					"Point on Curve 'X':= " << std::hex << publicKey.GetPublicElement().x << std::dec);
			DEBUGMSG(
					"Point on Curve 'Y':= " << std::hex << publicKey.GetPublicElement().y << std::dec);
			//encrypt private key with Password
			std::string privateKeyString;
			CryptoPP::StringSink privateKeySink(privateKeyString);
			privateKey.Save(privateKeySink);
			byte key[CryptoPP::SHA::DIGESTSIZE];
			GenerateKey(key, password);
			assert(CryptoPP::SHA::DIGESTSIZE >= CryptoPP::AES::BLOCKSIZE);
			CryptoPP::Base64Encoder* base64_enc = new CryptoPP::Base64Encoder(
					new FileSink(privateKeyFileName.c_str()));
			CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption aes(key,
					CryptoPP::AES::BLOCKSIZE, iv);
			CryptoPP::StreamTransformationFilter aes_enc(aes, base64_enc);
			aes_enc.Put((const byte *) privateKeyString.c_str(),
					privateKeyString.length());
			aes_enc.MessageEnd();
			publicKey.Save(
					CryptoPP::Base64Encoder(
							new FileSink(publicKeyFileName.c_str())).Ref());
			INFOMSG(
					"Key pair successfully made: private key file is encrypted with " << aes.StaticAlgorithmName().c_str());
			INFOMSG("Key Size was: " << sizeof(key));
			INFOMSG("Private key file: " << privateKeyFileName.c_str());
			INFOMSG("Public key file: " << publicKeyFileName.c_str());
			return true;
		}
	}
	return false;
}

static bool makeKeyPair(std::string &privateKeyStr, std::string &publicKeyStr) {
	PrivateECCKeyType privateKey;
	privateKey.Initialize(random_pool, CryptoPP::ASN1::sect163r1());
	if (privateKey.Validate(random_pool, 3)) {
		DEBUGMSG("private Exponent:= " << privateKey.GetPrivateExponent());
		PubECCKeyType publicKey;
		privateKey.MakePublicKey(publicKey);
		if (publicKey.Validate(random_pool, 3)) {
			DEBUGMSG(
					"Point on Curve 'X':= " << std::hex << publicKey.GetPublicElement().x << std::dec);
			DEBUGMSG(
					"Point on Curve 'Y':= " << std::hex << publicKey.GetPublicElement().y << std::dec);
			CryptoPP::StringSink privateKeySink(privateKeyStr);
			privateKey.Save(privateKeySink);

			CryptoPP::StringSink publicKeySink(publicKeyStr);
			publicKey.Save(publicKeySink);

			INFOMSG("Size of private key: " << privateKeyStr.length());
			INFOMSG("Size of pub key: " << publicKeyStr.length());

			return true;
		}
	}
	return false;
}

static bool readInPrivateKey(const std::string &password,
		PrivateECCKeyType &PrivateKey, const std::string &privateKeyFileName) {
	bool bRetVal = false;
	if (exists(privateKeyFileName.c_str())) {
		//need to check to insure file exists
		std::string cipher;
		CryptoPP::Base64Decoder* base_dec = new CryptoPP::Base64Decoder(
				new CryptoPP::StringSink(cipher));
		CryptoPP::FileSource file(privateKeyFileName.c_str(), true, base_dec,
				false);
		byte key[CryptoPP::SHA::DIGESTSIZE];
		GenerateKey(key, password);
		assert(CryptoPP::SHA::DIGESTSIZE >= CryptoPP::AES::BLOCKSIZE);
		std::string plantextPrivateKey;
		CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::BLOCKSIZE);
		CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(
				aesDecryption, iv);
		CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption,
				new CryptoPP::StringSink(plantextPrivateKey));
		stfDecryptor.Put(reinterpret_cast<const unsigned char*>(cipher.data()),
				cipher.size());
		stfDecryptor.MessageEnd();
		CryptoPP::StringSource tprivateKeySink(
				(const byte *) plantextPrivateKey.data(),
				plantextPrivateKey.length(), true);
		PrivateKey.Load(tprivateKeySink);
		DEBUGMSG("private Exponent:= " << PrivateKey.GetPrivateExponent());
		bRetVal = true;
	} else {
		WARNMSG(
				"********************************************************************************");
		WARNMSG(
				"Can not open private key file:  " << privateKeyFileName.c_str());
		WARNMSG(
				"********************************************************************************");
	}
	return bRetVal;
}

static bool readInPublicKey(const std::string &publicKeyFileName,
		PubECCKeyType &publicKey) {
	bool bRetVal = false;
	if (exists((publicKeyFileName.c_str()))) {
		//check if file exists
		std::string encoded;
		CryptoPP::Base64Decoder* base_dec = new CryptoPP::Base64Decoder(
				new CryptoPP::StringSink(encoded));
		CryptoPP::FileSource file(publicKeyFileName.c_str(), true, base_dec,
				false);
		CryptoPP::StringSource publicStringSink((const byte *) encoded.c_str(),
				encoded.length(), true);
		publicKey.Load(publicStringSink);
		DEBUGMSG(
				"Point on Curve 'X':= " << std::hex << publicKey.GetPublicElement().x << std::dec);
		DEBUGMSG(
				"Point on Curve 'Y':= " << std::hex << publicKey.GetPublicElement().y << std::dec);
		bRetVal = true;
	} else {
		WARNMSG(
				"****************************************************************************************");
		WARNMSG("Can not open public key file:  " << publicKeyFileName.c_str());
		WARNMSG(
				"****************************************************************************************");
	}
	return bRetVal;
}

void generateBadgeKeys(int num) {
	for (int i = 0; i < num; i++) {
		std::string pub;
		std::string priv;
		unsigned char networkID = random_pool.GenerateByte() % 6;
		unsigned char nodeID = random_pool.GenerateByte();
		makeKeyPair(priv, pub);
	}
}

void usage() {
	std::cerr << "usage" << std::endl;
}

//REMEMBER WE NEED a 2 BYTE RADIO ID:
// byte 1 is the network ID we'll use so between 0-6
// byte 2 # between 0-255
int main(int argc, char *argv[]) {
	char create = 0, generate = 0;
	;
	int ch = 0;
	int numberToGen = 0;

	while ((ch = getopt(argc, argv, "cn:")) != -1) {
		switch (ch) {
		case 'c':
			create = 1;
			break;
		case 'n':
			numberToGen = atoi(optarg);
			generate = 1;
			break;
		case '?':
		default:
			usage();
			return -1;
			break;
		}
	}

	if (1 == create) {
		std::string priv("priv");
		std::string pub("pub");
		createDaemonKeyPair("tmp", priv, pub);
	} else if (1 == generate) {
		generateBadgeKeys(numberToGen);
	}
	return 0;
}
