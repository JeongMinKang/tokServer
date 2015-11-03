#pragma once

namespace tokLib
{
namespace network
{
    
class peer;
    
class readBuffer : private boost::noncopyable
{
private:
    enum { maxBufLength=2048, };
    peer* peer_;
    char buffer_[maxBufLength];
    short bufOffset_;
    
    int parse(short bufLen);
    
public:
    explicit readBuffer(peer* p);
    ~readBuffer();
    
    inline char* data() { return buffer_+bufOffset_; }
    inline short length() { return maxBufLength-bufOffset_; }
    int doWork(short recvn);
};
    
}
}
