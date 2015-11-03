#include "preCompiled.h"
#include "server.h"

namespace tokLib
{
namespace network
{

const string LOG_SERVER("server");

server::server() :
    acceptor_(ioService_), socket_(ioService_), strand_(ioService_),
    preAcceptSockSize_(0), workerThreadPoolSize_(0)
{
}

server::~server()
{
}

int server::init(app::config* cfg)
{
    if (cfg==nullptr || cfg->peerSize==0)
        return errDefault;
    
    preAcceptSockSize_ = cfg->preAcceptSockSize;
	workerThreadPoolSize_ = cfg->workerThreadPoolSize;
    poolPeer_.reset(new boost::pool<>(cfg->peerSize));
    
//    boost::asio::ip::tcp::resolver resolver(ioService_);
//    boost::asio::ip::tcp::resolver::query query(ip.empty()?"0.0.0.0":ip, port);
//    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    const string& ip = cfg->ip;
    boost::asio::ip::address ipAddr(boost::asio::ip::address::from_string(ip.empty()?"0.0.0.0":ip));
    boost::asio::ip::tcp::endpoint endpoint(ipAddr, cfg->tcpPort);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();
    
	// accept socket create
	for (int i=0; i<preAcceptSockSize_; ++i)
		doAccept();
    
    if (onInit() != errNone)
    {
        writeFileLog(LOG_SERVER, "onInit error");
        return errDefault;
    }
    
    writeFileLog(LOG_SERVER, "init end");
    return errNone;
}

void server::run()
{
    for (short i=0; i<workerThreadPoolSize_; ++i)
    {
        shared_ptr<thread> threadPtr(new thread([this](){ ioService_.run(); }));
        threads_.push_back(threadPtr);
	}
    
    writeFileLog(LOG_SERVER, "run end");
}

void server::stop()
{
   	acceptor_.close();
	ioService_.stop();
    
	for (size_t i=0; i<threads_.size(); ++i)
		threads_[i]->join();
    
    onStop();
    
    writeFileLog(LOG_SERVER, "server stop end");
}

void server::doAccept()
{
    acceptor_.async_accept(socket_, strand_.wrap(
        [this](boost::system::error_code error)
        {
            if (!error)
            {
                moveSocket(move(socket_));
            }
            doAccept();
        }));
    
	return;
}
    
void server::freePeer(void* peer)
{
    poolPeer_->free(peer);
}

}
}