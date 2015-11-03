#pragma once

namespace tokLib
{
namespace network
{

class message
{
private:
	enum { maxBodyLength = 1500-20-20-headerLength, };   //ethernet mtu-ipheader-tcpheader-header

private:
    header header_;
	char data_[headerLength+maxBodyLength];
    
public:
	message() { clear(); }

	const char* data() const { return data_; }
	char* data() { return data_; }

    short length() const { return header_.length; }

	const char* body() const { return data_+headerLength; }
	char* body() { return data_+headerLength; }
    short flag() const { return header_.flag; }

	bool decodeHeader()
	{
        try
        {
            memcpy(&header_, data_, headerLength);
            if (header_.flag==0 || header_.length==0)
                throw -1;
            if (header_.length > headerLength+maxBodyLength)
                throw -1;
        }
        catch (int& error)
        {
            clear();
            return false;
        }
        
        return true;
	}

    void encodeHeader(char type, short flag, short bodyLength)
	{
        header_.type = type;
        header_.flag = flag;
        header_.length = headerLength+bodyLength;
        memcpy(data_, &header_, headerLength);
 	}

	void setBody(const char* body)
	{
		memcpy(data_+headerLength, body, header_.length-headerLength);
	}

	void clear()
	{
        header_.clear();
        memset(data_, 0, headerLength+maxBodyLength);
	}
};

typedef deque<message> messageQueue;

}
}
