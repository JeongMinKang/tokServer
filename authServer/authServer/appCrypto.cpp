#include "preCompiled.h"
#include "appCrypto.h"
#include "../../tokLib/crypto.h"

#include <osrng.h>
#include <rsa.h>
#include <iomanip>

extern authConfig appConfig_;
const short keyBufSize = 4096*2;
// default key, 어플리케이션 별로 주어지는 키로 절대 노출되면 안된다.
unsigned char authSvrKey[CryptoPP::AES::DEFAULT_KEYLENGTH] = { 0xF8, 0x72, 0x2E, 0x38, 0xD8, 0x77, 0x2E, 0x2E, 0xC6, 0x38, 0xC4, 0x75, 0x49, 0x75, 0xD7, 0x33 };

int appCrypto::masterPassword(string& masterPwd)
{
    //master password decrypt
    ifstream ifs;
    ifs.open(appConfig_.masterPwd.c_str(), ios_base::in|ios_base::binary);
	if (!ifs.is_open())
		return -1;

    char buf[keyBufSize] = {0,};
    ifs.read(buf, keyBufSize);
    ifs.close();
    
    string encMasterPwd(buf);
    return tokLib::security::crypto::decrypt(masterPwd, encMasterPwd, authSvrKey);
}

int appCrypto::privateKey(string& priKey, const string& masterPwd)
{
    //private key decrypt
    ifstream ifs;
	ifs.open(appConfig_.priKey.c_str(), ios_base::in|ios_base::binary);
	if (!ifs.is_open())
		return -1;
    
    char buf[keyBufSize] = {0,};
	ifs.read(buf, keyBufSize);
	ifs.close();

    string priKeyStr(buf);
    return tokLib::security::crypto::decrypt(priKey, priKeyStr, masterPwd);
}

int appCrypto::publicKey(string& pubKey)
{
    //public key
    ifstream ifs;
    ifs.open(appConfig_.pubKey.c_str(), ios_base::in|ios_base::binary);
    if (!ifs.is_open())
        return -1;

    char buf[keyBufSize] = {0,};
    ifs.read(buf, keyBufSize);
    ifs.close();
    
    pubKey = buf;
    
    return errNone;
}

int appCrypto::symmetricKey(string& symKey, const string& encSymKey, const string& priKey)
{
    try
    {
        symKey.clear();
        
        // data decryption
        CryptoPP::StringSource priKeySrc(priKey, true, new CryptoPP::HexDecoder);
        CryptoPP::RSAES_PKCS1v15_Decryptor decryptor(priKeySrc);
        
        CryptoPP::AutoSeededRandomPool rng;
        CryptoPP::StringSource(encSymKey, true,
            new CryptoPP::HexDecoder(
                new CryptoPP::PK_DecryptorFilter(rng, decryptor,
                    new CryptoPP::StringSink(symKey))));
    }
    catch (CryptoPP::Exception& e)
    {
        cout << "decrypt symmetric key exception occur, what: " << e.what() << endl;
        return errDefault;
    }
    
    return errNone;
}


