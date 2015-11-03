#include "preCompiled.h"
#include "token.h"
#include "../../tokLib/crypto.h"

#include <iomanip>

extern unsigned char authSvrKey[CryptoPP::AES::DEFAULT_KEYLENGTH];
const int timeStringLength = 10;
const int uidStringLength = 20;

tokenGenerator::tokenGenerator(const uint64_t uid, const int durationType)
    : uid_(uid), durationType_(durationType)
{
}

int tokenGenerator::generate()
{
    int duration=0;
    switch (durationType_)
    {
        case oneHourToken:
            duration = 60*60;
            break;
        case oneDayToken:
            duration = 60*60*24;
            break;
        case oneMonthToken:
            duration = 60*60*24*30;
            break;
        case threeMonthToken:
            duration = 60*60*24*30*3;
            break;
    }
    
    auto beginTime = chrono::system_clock::now();
    auto begin = chrono::system_clock::to_time_t(beginTime);
    chrono::seconds sec(duration);
    auto end = chrono::system_clock::to_time_t(beginTime + sec);
    
    ostringstream oss;
    oss << begin << end << setfill('0') << setw(uidStringLength) << uid_;
    
    return tokLib::security::crypto::encrypt(token_, oss.str(), authSvrKey);
}


tokenParser::tokenParser(const string& token)
    : token_(token), uid_(0), valid_(false)
{
}

int tokenParser::parse()
{
    int ret = errNone;
    string token;
    if ((ret=tokLib::security::crypto::decrypt(token, token_, authSvrKey)) != errNone)
        return ret;
    
    if (timeStringLength*2+uidStringLength != token.length())
        return errDefault;
    
    begin_.assign(token.begin(), token.begin()+timeStringLength);
    end_.assign(token.begin()+timeStringLength, token.begin()+timeStringLength*2);
    string uid;
    uid.assign(token.begin()+timeStringLength*2, token.end());
    
    chrono::system_clock::time_point begin, end;
    uid_ = stoll(uid);

    time_t beginTime = stol(begin_);
    begin = chrono::system_clock::from_time_t(beginTime);
    time_t endTime = stol(end_);
    end = chrono::system_clock::from_time_t(endTime);
    
    chrono::system_clock::time_point now = chrono::system_clock::now();
    if (now>begin && now<end)
        valid_ = true;
    
    return errNone;
}
