#pragma once

#include <boost/asio.hpp>
#include "readBuffer.h"
#include "message.h"

namespace tokLib
{
namespace network
{

class peerSet;

class peer :
    public enable_shared_from_this<peer>,
    private boost::noncopyable
{
protected:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_service::strand strand_;
    peerSet& peers_;
    typedef map<short, function<int (const short, const char*)> > handlerType;
	handlerType handler_;
    
private:
    readBuffer readBuffer_;
	messageQueue writeMsgs_;
    
public:
    peer(boost::asio::ip::tcp::socket socket, peerSet& peers);
	virtual ~peer();
    
    inline boost::asio::ip::tcp::socket& socket() { return socket_; }
	void start();
	void stop();
    virtual int onStart();
    virtual int onStop();
    void read();
    void write(const message& msg);
    void write();
    
    friend class readBuffer;
};
    
typedef shared_ptr<peer> peerPtr;
    
class peerSet
{
private:
    set<peerPtr> peers_;
    
public:    
    void add(peerPtr pr);
    void remove(peerPtr pr);
};

}
}