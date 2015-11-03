#pragma once

class authServer : public server
{
private:
    string masterPwd_;
    string priKey_;
    string pubKey_;
    
    map<int, string> appIds_;
    vector<pair<errType, string> > errType_;    //에러타입, 에러타입스트링
    map<int, string> errMessage_;               //에러코드, 에러메시지
    
    authServer();
    ~authServer();
    
public:
    static authServer& getInstance() { static authServer instance; return instance; }
    
    virtual int onInit();
    virtual void onStop();
    virtual void moveSocket(boost::asio::ip::tcp::socket socket);
    
    inline void masterPwd(const string& masterPwd) { masterPwd_ = masterPwd; }
    inline const string& masterPwd() { return masterPwd_; }
    inline void priKey(const string& priKey) { priKey_ = priKey; }
    inline const string& priKey() { return priKey_; }
    inline void pubKey(const string& pubKey) { pubKey_ = pubKey; }
    inline const string& pubKey() { return pubKey_; }
    
    string& errTypeString(errType et);
    string errMessage(int code);
    
    bool isValidAppId(const int appId);
    bool isValidEmail(const string& email, const int appId);
        
};