#pragma once

#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <hex.h>
#include <sha.h>

namespace tokLib
{
namespace security
{

extern unsigned char tokqueInitialVector[CryptoPP::AES::BLOCKSIZE];
    
class crypto : private boost::noncopyable
{
private:
    crypto();
    ~crypto();
    
public:
    static int decrypt(string& output, const string& input, const string& key);
    static int decrypt(string& output, const string& input, const unsigned char* key);
    static int encrypt(string& output, const string& input, const string& key);
    static int encrypt(string& output, const string& input, const unsigned char* key);
    static int hash(string& output, const string& input);
};
 
}
}
