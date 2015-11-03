#include "preCompiled.h"
#include "uniqueId.h"

namespace tokLib
{
namespace app
{
    
mutex uniqueIdMutex_;

uniqueId::uniqueId() : count_(0), current_(0),
    timePoint_(chrono::system_clock::from_time_t(1388502000)) //select unix_timestamp('2014-01-01 00:00:00');
{
}
    
uniqueId::~uniqueId()
{
}
    
uint64_t uniqueId::getUniqueId()
{
    unique_lock<mutex> lock(uniqueIdMutex_);
    
    static uniqueId uid;
    
    chrono::system_clock::time_point now = chrono::system_clock::now();
    auto diff = now - uid.timePoint_;
    auto ms = chrono::duration_cast<chrono::milliseconds>(diff);
    uint64_t time = ms.count(); //41 bit
    
    if (uid.count_ > 8191) //8192(2^13-1)
    {
        chrono::system_clock::time_point nowLoop;
        uint64_t timeLoop;
        
        while (true)
        {
            nowLoop = chrono::system_clock::now();
            auto diffLoop = nowLoop - uid.timePoint_;
            auto msLoop = chrono::duration_cast<chrono::milliseconds>(diffLoop);
            timeLoop = msLoop.count();
            
            if (timeLoop > uid.current_) break;
        }
    }
    
    uint64_t pid = (uint64_t)getpid();
    pid = pid % 1024;   //10 bit
    
    if (uid.current_ < time)
    {
        uid.current_ = time;
        uid.count_ = 0;
    }
    
    uint64_t identifier = time << 23 | pid << 13 | (uid.count_++);

    return identifier;
}

}
}
