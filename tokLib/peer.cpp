#include "preCompiled.h"
#include "peer.h"

namespace tokLib
{
namespace network
{
    
const string LOG_PEER("peer");

peer::peer(boost::asio::ip::tcp::socket socket, peerSet& peers) :
    socket_(move(socket)), strand_(socket_.get_io_service()),
    peers_(peers), readBuffer_(this)
{
}

peer::~peer()
{
}

void peer::start()
{
    peers_.add(shared_from_this());
    onStart();
    
    read();
}

void peer::stop()
{
    onStop();
    
    handler_.clear();
    peers_.remove(shared_from_this());
}
    
int peer::onStart()
{
    return errNone;
}
    
int peer::onStop()
{
    return errNone;
}
    
void peer::read()
{
    //self 변수 두는 이유는 람다내에서 stop 하더라도 peer가 사라지는 것을 막고
    //read 함수가 종료될 때 peer 사라져서 메모리에서 delete되도록 하기 위함
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(readBuffer_.data(), readBuffer_.length()), strand_.wrap(
        [this, self](boost::system::error_code error, size_t bytesTransferred)
        {
            if (!error || bytesTransferred)
            {
                if (readBuffer_.doWork(static_cast<short>(bytesTransferred)) != errNone)
                {
                    // error
                    stop();
                    return;
                }
                read();
            }
            else if (error != boost::asio::error::would_block &&
                     error != boost::asio::error::interrupted &&
                     error != boost::asio::error::try_again &&
                     error != boost::asio::error::in_progress)
            {
                stop();
                if (error != boost::asio::error::eof)
                {
                    writeFileLog(LOG_PEER, "read error " << error.message());
                }
            }
        }));
}
    
void peer::write(const message& msg)
{
    bool writeInProgress = !writeMsgs_.empty();
	writeMsgs_.push_back(msg);
    if (!writeInProgress)
    {
        write();
    }
}

void peer::write()
{
    auto self(shared_from_this());
    socket_.async_write_some(
        boost::asio::buffer(writeMsgs_.front().data(), writeMsgs_.front().length()),
        [this, self](boost::system::error_code error, size_t bytesTransferred)
        {
            if (!error)
            {
                writeMsgs_.pop_front();
                if (!writeMsgs_.empty())
                    write();
            }
            else if (error != boost::asio::error::would_block &&
                     error != boost::asio::error::interrupted &&
                     error != boost::asio::error::try_again &&
                     error != boost::asio::error::in_progress)
            {
                stop();
                if (error != boost::asio::error::eof)
                {
                    writeFileLog(LOG_PEER, "write error " << error.message());
                }
            }
        });
}
    

//peerSet class
void peerSet::add(peerPtr pr)
{
    peers_.insert(pr);
}

void peerSet::remove(peerPtr pr)
{
    peers_.erase(pr);
}

}
}