#include <iostream>
#include <string>
#include <fstream>

#include <unistd.h>

#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <hex.h>
#include <osrng.h>
#include <rsa.h>

using namespace std;

// default key
byte tokqueKey[CryptoPP::AES::DEFAULT_KEYLENGTH] = { 0xF8, 0x72, 0x2E, 0x38, 0xD8, 0x77, 0x2E, 0x2E, 0xC6, 0x38, 0xC4, 0x75, 0x49, 0x75, 0xD7, 0x33 };
// initial vector
byte tokqueIV[CryptoPP::AES::BLOCKSIZE] =
    { 0xC0, 0x74, 0x2E, 0x46, 0x47, 0x38, 0xFC, 0x72, 0x2E, 0x38, 0xDC, 0x77, 0x2E, 0x2E, 0xE6, 0x38 };

void genKeyPair(char* inputPath, char* pwdc);


int main(int argc, const char * argv[])
{
	cout << "=======================================" << endl;
	cout << "tokque public and private key generator" << endl;
	cout << "=======================================" << endl;
	cout << endl;
    
	cout << "key path setting, insert path" << endl;
	cout << "just input enter key to use default path(/current_path/key)" << endl;
	cout << "path : ";
    char input[256];
	cin.getline(input, 256);
	if (0==strlen(input))
    {
        char cwd[256]={0,};
        getcwd(cwd, 256);
		strncpy(input, cwd, 256);
        strcat(input, "/key/");
    }
    cout << endl;
    
	char pwd[128]={0,};
    char pwdc[128]={0,};
    cout << "input master password." << endl;
pwdinput:
	cout << "password : ";
	cin.getline(pwd, 128);
	cout << "password confirm : ";
	cin.getline(pwdc, 128);
    
    if (strcmp(pwd, pwdc))
    {
		cout << "passwords not match, retype password." << endl;
		goto pwdinput;
	}
	
    genKeyPair(input, pwdc);
    
    cout << "complete public/private key." << endl;
    cout << endl;

    return 0;
}

void genKeyPair(char* inputPath, char* pwdc)
{
    string savePath(inputPath), pwd(pwdc);
	if (savePath.length())
    {
		auto riter = savePath.rbegin();
		if (*riter != '/')
			savePath.insert(savePath.end(), '/');
    }
    else
    {
        cout << "public/private key path empty." << endl;
        return ;
    }
    string masterPwdPath, priKeyPath, pubKeyPath;
    masterPwdPath = savePath + "master.pwd";
    priKeyPath = savePath + "pri.key";
    pubKeyPath = savePath + "pub.key";
        
    //BER public/private key
    CryptoPP::AutoSeededRandomPool rng;
	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, 3072);
    
	CryptoPP::RSA::PrivateKey priKey(params);
	CryptoPP::RSA::PublicKey pubKey(params);
    
	CryptoPP::RSAES_PKCS1v15_Decryptor decryptor(priKey);
	CryptoPP::RSAES_PKCS1v15_Encryptor encryptor(pubKey);

    string strPriKey, strPubKey;
    CryptoPP::HexEncoder priEncoder(new CryptoPP::StringSink(strPriKey));
    decryptor.AccessKey().Save(priEncoder);
    CryptoPP::HexEncoder pubEncoder(new CryptoPP::StringSink(strPubKey));
    encryptor.AccessKey().Save(pubEncoder);
    
    unsigned char key[CryptoPP::AES::DEFAULT_KEYLENGTH] = {0,};
	memcpy(key, pwd.c_str(), pwd.length());
    
    CryptoPP::AES::Encryption aesEnc(key);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEnc(aesEnc, tokqueIV);
    
    string hexEncoded;
	CryptoPP::StringSource(strPriKey, true,
        new CryptoPP::StreamTransformationFilter(cbcEnc,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(hexEncoded), false)));
    
    //private key save
    ofstream priOf;
	priOf.open(priKeyPath.c_str(), ios_base::out|ios_base::binary);
	if (priOf.is_open())
    {
		priOf << hexEncoded;
		priOf.close();
	}
    
    //public key save, 공개키는 암호화할 필요 없다.
    ofstream pubOf;
    pubOf.open(pubKeyPath.c_str(), ios_base::out|ios_base::binary);
    if (pubOf.is_open())
    {
        pubOf << strPubKey;
        pubOf.close();
    }
    
    //master password
    CryptoPP::AES::Encryption aesEncPwd(tokqueKey);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncPwd(aesEncPwd, tokqueIV);
    string pwdHexEncoded;
    CryptoPP::StringSource(pwd, true,
        new CryptoPP::StreamTransformationFilter(cbcEncPwd,
            new CryptoPP::HexEncoder(new CryptoPP::StringSink(pwdHexEncoded), false)));
    //            CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING));
    ofstream pwdOf;
    pwdOf.open(masterPwdPath.c_str(), ios_base::out|ios_base::binary);
    if (pwdOf.is_open())
    {
        pwdOf << pwdHexEncoded;
        pwdOf.close();
    }

    return;
}