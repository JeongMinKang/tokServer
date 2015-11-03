#include "preCompiled.h"
#include "crypto.h"

namespace tokLib
{
namespace security
{
    
const string LOG_CRYPTO("crypto");
    
// initial vector
unsigned char tokqueInitialVector[CryptoPP::AES::BLOCKSIZE] = { 0xC0, 0x74, 0x2E, 0x46, 0x47, 0x38, 0xFC, 0x72, 0x2E, 0x38, 0xDC, 0x77, 0x2E, 0x2E, 0xE6, 0x38 };
    
int crypto::decrypt(string& output, const string& input, const string& key)
{
    unsigned char keyBin[CryptoPP::AES::DEFAULT_KEYLENGTH] = {0,};
    size_t keyLen = key.length();
    if (keyLen > CryptoPP::AES::DEFAULT_KEYLENGTH)
        keyLen = CryptoPP::AES::DEFAULT_KEYLENGTH;
    memcpy(keyBin, key.c_str(), keyLen);
    
    return decrypt(output, input, keyBin);
}
    
int crypto::decrypt(string& output, const string& input, const unsigned char* key)
{
    try
    {
        CryptoPP::SecByteBlock keyBlock(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
        CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
        dec.SetKeyWithIV(keyBlock, CryptoPP::AES::DEFAULT_KEYLENGTH, tokqueInitialVector);
        
        CryptoPP::StringSource(input, true,
            new CryptoPP::HexDecoder(
                new CryptoPP::StreamTransformationFilter(dec,
                    new CryptoPP::StringSink(output))));
    }
    catch (CryptoPP::Exception& e)
    {
        writeFileLog(LOG_CRYPTO, "decrypt exception occur, what: " << e.what());
        return errDefault;
    }
    return errNone;
}
    
int crypto::encrypt(string& output, const string& input, const string& key)
{
    byte keyBin[CryptoPP::AES::DEFAULT_KEYLENGTH] = {0,};
    size_t keyLen = key.length();
    if (keyLen > CryptoPP::AES::DEFAULT_KEYLENGTH)
        keyLen = CryptoPP::AES::DEFAULT_KEYLENGTH;
    memcpy(keyBin, key.c_str(), keyLen);
    
    return encrypt(output, input, keyBin);
}
    
int crypto::encrypt(string& output, const string& input, const unsigned char* key)
{
    try
    {
        CryptoPP::SecByteBlock keyBlock(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
        CryptoPP::AES::Encryption aesEnc(keyBlock, CryptoPP::AES::DEFAULT_KEYLENGTH);
        CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEnc(aesEnc, tokqueInitialVector);
        
        CryptoPP::StringSource(input, true,
            new CryptoPP::StreamTransformationFilter(cbcEnc,
                new CryptoPP::HexEncoder(
                    new CryptoPP::StringSink(output), false)));
    }
    catch (CryptoPP::Exception& e)
    {
        writeFileLog(LOG_CRYPTO, "encrypt exception occur, what: " << e.what());
        return errDefault;
    }
    
    return errNone;
}
    
int crypto::hash(string& output, const string& input)
{
    try
    {
        CryptoPP::SHA256 hash;
        
        CryptoPP::StringSource(input, true,
            new CryptoPP::HashFilter(hash,
                new CryptoPP::HexEncoder(
                    new CryptoPP::StringSink(output), false)));
    }
    catch (CryptoPP::Exception& e)
    {
        writeFileLog(LOG_CRYPTO, "hash exception occur, what: " << e.what());
        return errDefault;
    }

    return errNone;
}
    
}
}
