#pragma once
#include "peer.h"

namespace tokLib
{
namespace network
{

class server : private boost::noncopyable
{
protected:
    boost::asio::io_service ioService_;
    
private:
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_service::strand strand_;
	int preAcceptSockSize_;
	short workerThreadPoolSize_;
    peerSet peers_;    
    vector<shared_ptr<thread> > threads_;
    unique_ptr<boost::pool<> > poolPeer_;

public:
    server();
    virtual ~server();
    
    int init(app::config* cfg);
	void run();
	void stop();
	void doAccept();
    
    virtual int onInit() { return errNone; }
    virtual void onStop() { return; }
    virtual void moveSocket(boost::asio::ip::tcp::socket socket)=0;
    
    struct peerDeleter
    {
        peerDeleter(server* s) : svr(s) {}
        template<typename T>
        void operator() (T* ptr)
        {
            ptr->~T();
            svr->freePeer(ptr);
        }
        server* svr;
    };
    
    template<typename T>
    void allocPeer(boost::asio::ip::tcp::socket socket)
    {
        void* buf = poolPeer_->malloc();
        if (buf == 0) return;
        peerDeleter pd(this);
        peerPtr pr(new (buf) T(move(socket), peers_), pd);
        pr->start();
    }
    void freePeer(void* peer);
};

}
}