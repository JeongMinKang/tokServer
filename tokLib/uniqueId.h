#pragma once

namespace tokLib
{
namespace app
{

//reference flocon
class uniqueId : private boost::noncopyable
{
private:
    uint64_t count_;
    uint64_t current_;
    chrono::system_clock::time_point timePoint_;
    
    uniqueId();
    ~uniqueId();
    
public:
    static uint64_t getUniqueId();
};

}
}