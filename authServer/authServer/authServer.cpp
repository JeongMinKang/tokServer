#include "preCompiled.h"
#include "authServer.h"
#include "authPeer.h"
#include "dbQueryDef.h"

const string LOG_AUTH_SERVER("authServer");
extern authConfig appConfig_;
extern defDb db_;

authServer::authServer()
{
}

authServer::~authServer()
{
}

int authServer::onInit()
{
    // mysql db create
    if (db_.create(appConfig_.db) != errNone)
    {
        writeFileLog(LOG_AUTH_SERVER, "db create error");
		return errDefault;
	}
   
    appDb db(db_);
    if (db.query(dbQuery::getInstance().query(dbQueryDef::selectAppId)) != errNone)
        return errDefault;
    while (db.fetch() == errNone)
    {
        int appId = 0;
        string desc;
        if (db.fetchValue("id", appId) != errNone) return errDefault;
        if (db.fetchValue("description", desc) != errNone) return errDefault;
        appIds_.insert(make_pair(appId, desc));
    }
    db.clear();
    
    //error type
    errType_.push_back(make_pair(errAuthServerPublicKey, "auth_server_public_key"));
    errType_.push_back(make_pair(errShareSymmetricKey, "share_symmetric_key"));
    errType_.push_back(make_pair(errSignUp, "sign_up"));
    errType_.push_back(make_pair(errSignIn, "sign_in"));
    errType_.push_back(make_pair(errTokenInfo, "token_info"));
    errType_.push_back(make_pair(errRenewToken, "renew_token"));
    errType_.push_back(make_pair(errTypeMax, "err_type_max"));
    
    //error message
    if (db.query(dbQuery::getInstance().query(dbQueryDef::selectErrorMessage)) != errNone)
        return errDefault;
    while (db.fetch() == errNone)
    {
        int err=0;
        string errMsg;
        if (db.fetchValue("id", err) != errNone) return errDefault;
        if (db.fetchValue("message", errMsg) != errNone) return errDefault;
        errMessage_.insert(make_pair(err, errMsg));
    }
    db.clear();
        
    return errNone;
}

void authServer::onStop()
{
    return;
}

void authServer::moveSocket(boost::asio::ip::tcp::socket socket)
{
    return allocPeer<authPeer>(move(socket));
}

string& authServer::errTypeString(errType et)
{
    if (et < 0 || et >= errTypeMax)
        et = errTypeMax;
    return errType_[et].second;
}

string authServer::errMessage(int code)
{
    string errMessage("");
    auto iter = errMessage_.find(code);
    if (iter != errMessage_.end())
        return iter->second;
    return errMessage;
}

bool authServer::isValidAppId(const int appId)
{
    auto iter = appIds_.find(appId);
    if (iter == appIds_.end())
        return false;
    return true;
}

bool authServer::isValidEmail(const string& email, const int appId)
{
    appDb db(db_);
    if (db.query(dbQuery::getInstance().query(dbQueryDef::selectEmail), email, appId) != errNone)
        return false;
    
    if (db.fetch() == errNone)  //값이 이미 있으면 false 리턴
        return false;
    
    return true;
}
