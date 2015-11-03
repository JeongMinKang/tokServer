#include "preCompiled.h"
#include "readBuffer.h"
#include "peer.h"

namespace tokLib
{
namespace network
{

readBuffer::readBuffer(peer* p) : peer_(p), bufOffset_(0)
{
	memset(buffer_, 0, maxBufLength);
}

readBuffer::~readBuffer()
{
}

int readBuffer::doWork(short recvn)
{
	short bufLen = bufOffset_ + recvn;
	int processn = parse(bufLen);
	if (errDefault == processn)
    {
		bufOffset_ = 0;
		return errDefault;
	}

	if (processn >= bufLen)
    {
		bufOffset_ = 0;
		memset(buffer_, 0, maxBufLength);
	}
    else
    {
		if (processn > 0)
        {
			int notWorked = bufLen - processn;
			memmove(buffer_, buffer_+processn, notWorked);
			memset(buffer_+notWorked, 0, maxBufLength-notWorked);
			bufOffset_ = notWorked;
		}
        else if (0 == processn)
			bufOffset_ = bufLen;
	}

	return errNone;
}

int readBuffer::parse(short bufLen)
{
	int worked=0;
	header hdr;
	const char* buf = buffer_;

	while (bufLen >= headerLength)
    {
        memcpy(&hdr, buf, headerLength);

		if (bufLen >= hdr.length)
        {
			peer::handlerType::iterator iter = peer_->handler_.find(hdr.flag);
			if (iter!=peer_->handler_.end())
            {   //헤더포함된 버퍼를 넘기는건 처리하는 곳에서 헤더정보가 필요할 수 있기 때문이다.
				if (iter->second(hdr.length, buf) != errNone)
                    return errDefault;
            }
		}
        else
			break;

		worked += hdr.length;
		buf += hdr.length;
		bufLen -= hdr.length;
	}

	return worked;
}

}
}