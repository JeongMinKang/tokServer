#pragma once

class authPeer : public peer
{
private:
    string symmetricKey_;
    
    int write(char type, short flag, const string& str);
    void errHandle(errType et, int code);
    
    int authServerPublicKey(const short size, const char* buffer);
    int shareSymmetricKey(const short size, const char* buffer);
    int validEmail(const short size, const char* buffer);
    int signUp(const short size, const char* buffer);
    int signIn(const short size, const char* buffer);
    int tokenInfo(const short size, const char* buffer);
    int renewToken(const short size, const char* buffer);
    int deleteAccount(const short size, const char* buffer);
    
public:
    authPeer(boost::asio::ip::tcp::socket socket, peerSet& peers);
	~authPeer();
    
    virtual int onStart();
    virtual int onStop();
};
