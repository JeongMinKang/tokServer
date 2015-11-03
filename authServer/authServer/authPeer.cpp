#include "preCompiled.h"
#include "authPeer.h"
#include "authServer.h"
#include "appCrypto.h"
#include "token.h"
#include "dbQueryDef.h"
#include "../../tokLib/json/json.h"
#include "../../tokLib/uniqueId.h"
#include "../../tokLib/crypto.h"

const string LOG_AUTH_PEER("authPeer");
extern defDb db_;

authPeer::authPeer(boost::asio::ip::tcp::socket socket, peerSet& peers) :
    peer(move(socket), peers)
{
}

authPeer::~authPeer()
{
}

int authPeer::onStart()
{
    handler_.insert(make_pair(cmdAuthServerPublicKey,
        bind(&authPeer::authServerPublicKey, this, std::placeholders::_1, std::placeholders::_2)));
    handler_.insert(make_pair(cmdShareSymmetricKey,
        bind(&authPeer::shareSymmetricKey, this, std::placeholders::_1, std::placeholders::_2)));
    handler_.insert(make_pair(cmdValidEmail,
        bind(&authPeer::validEmail, this, std::placeholders::_1, std::placeholders::_2)));
    handler_.insert(make_pair(cmdSignUp,
        bind(&authPeer::signUp, this, std::placeholders::_1, std::placeholders::_2)));
    handler_.insert(make_pair(cmdSignIn,
        bind(&authPeer::signIn, this, std::placeholders::_1, std::placeholders::_2)));
    handler_.insert(make_pair(cmdTokenInfo,
        bind(&authPeer::tokenInfo, this, std::placeholders::_1, std::placeholders::_2)));
    handler_.insert(make_pair(cmdRenewToken,
        bind(&authPeer::renewToken, this, std::placeholders::_1, std::placeholders::_2)));
    handler_.insert(make_pair(cmdDeleteAccount,
        bind(&authPeer::deleteAccount, this, std::placeholders::_1, std::placeholders::_2)));
    
    return errNone;
}

int authPeer::onStop()
{
    return errNone;
}

int authPeer::write(char type, short flag, const string& str)
{
    message msg;
    msg.encodeHeader(type, flag, str.length());
    msg.setBody(str.c_str());
    peer::write(msg);

    return errNone;
}

void authPeer::errHandle(errType et, int code)
{
    string& errTypeString = authServer::getInstance().errTypeString(et);
    string errMessage = authServer::getInstance().errMessage(code);
    
    Json::Value error;
    error["error"]["message"] = errMessage;
    error["error"]["type"] = errTypeString;
    error["error"]["code"] = code;
    write(notiPacket, notiError, error.asString());
    
    return;
}

int authPeer::authServerPublicKey(const short size, const char* buffer)
{
    try
    {
        const char* json = buffer + headerLength;
        
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) throw 1000;
        Json::Value pubKey = root["auth_server_public_key_req"];
        if (pubKey.isNull()) throw 1100;
        if (pubKey["app_id"].isNull() || !pubKey["app_id"].isInt()) throw 1101;
        if (!authServer::getInstance().isValidAppId(pubKey["app_id"].asInt())) throw 1102;
        
        //public key send to client
        Json::Value send;
        send["auth_server_public_key_res"]["app_id"] = pubKey["app_id"].asInt();
        send["auth_server_public_key_res"]["public_key"] = authServer::getInstance().pubKey();
        write(cmdPacket, cmdAuthServerPublicKey, send.asString());
    }
    catch (int& err)
    {
        errHandle(errAuthServerPublicKey, err);
        writeFileLog(LOG_AUTH_PEER, "error: " << err);
        return errDefault;
    }
    
    return errNone;
}

int authPeer::shareSymmetricKey(const short size, const char* buffer)
{
    try
    {
        const char* json = buffer + headerLength;
        
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) throw 1000;
        Json::Value symKey = root["share_symmetric_key_req"];
        if (symKey.isNull()) throw 1200;
        if (symKey["app_id"].isNull() || !symKey["app_id"].isInt()) throw 1201;
        if (!authServer::getInstance().isValidAppId(symKey["app_id"].asInt())) throw 1202;
        if (symKey["symmetric_key"].isNull()) throw 1203;
        //전송받은 대칭키를 인증서버 개인키로 복호화하여 저장한다.
        if (appCrypto::symmetricKey(symmetricKey_, symKey["symmetric_key"].asString(),
            authServer::getInstance().priKey()) != errNone) throw 1204;
        
        Json::Value send;
        send["share_symmetric_key_res"]["app_id"] = symKey["app_id"].asInt();
        send["share_symmetric_key_res"]["result"] = true;
        write(cmdPacket, cmdShareSymmetricKey, send.asString());
    }
    catch (int& err)
    {
        errHandle(errShareSymmetricKey, err);
        writeFileLog(LOG_AUTH_PEER, "error: " << err);
        return errDefault;
    }
    
    return errNone;
}

int authPeer::validEmail(const short size, const char* buffer)
{
    try
    {
        const char* json = buffer + headerLength;
        
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) throw 1000;
        Json::Value validEmail = root["valid_email_req"];
        if (validEmail.isNull()) throw 1300;
        if (validEmail["app_id"].isNull() || !validEmail["app_id"].isInt()) throw 1301;
        int appId = validEmail["app_id"].asInt();
        if (!authServer::getInstance().isValidAppId(appId)) throw 1302;
        if (validEmail["email"].isNull()) throw 1303;
        string email = validEmail["email"].asString();
        bool isValidEmail=true;
        if (!authServer::getInstance().isValidEmail(email, appId))
            isValidEmail=false;
        
        Json::Value send;
        send["valid_email_res"]["app_id"] = validEmail["app_id"].asInt();
        send["valid_email_res"]["is_valid"] = isValidEmail;
        write(cmdPacket, cmdValidEmail, send.asString());
    }
    catch (int& err)
    {
        errHandle(errValidEmail, err);
        writeFileLog(LOG_AUTH_PEER, "error: " << err);
        return errDefault;
    }
    return errNone;
}

int authPeer::signUp(const short size, const char* buffer)
{
    try
    {
        const char* json = buffer + headerLength;
        
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) throw 1000;
        Json::Value signUp = root["sign_up_req"];
        if (signUp.isNull()) throw 1400;
        if (signUp["app_id"].isNull() || !signUp["app_id"].isInt()) throw 1401;
        if (!authServer::getInstance().isValidAppId(signUp["app_id"].asInt())) throw 1402;
        if (signUp["email"].isNull()) throw 1403;
        if (signUp["name"].isNull()) throw 1404;
        if (signUp["password"].isNull()) throw 1405;
        int appId = signUp["app_id"].isInt();
        string email(signUp["email"].asString());
        string name(signUp["name"].asString());
        string password, hashPwd;
        if (!authServer::getInstance().isValidEmail(email, appId)) throw 1406;
        if (tokLib::security::crypto::decrypt(
            password, signUp["password"].asString(), symmetricKey_) != errNone) throw 1407;
        if (tokLib::security::crypto::hash(hashPwd, password) != errNone) throw 1408;
        string phone(signUp["phone"].asString());
        int durationTokenType=threeMonthToken;
        if (!signUp["duration_token"].isNull())
        {
            durationTokenType = signUp["duration_token"].asInt();
            if (!validDurationTokenType(durationTokenType)) throw 1409;
        }
        
        uint64_t userId = uniqueId::getUniqueId();
        //토큰은 암호화하지 않고 불투명 스트링이고 userId를 이용하여 만들고 토큰으로부터 userId를 구할 수 있어야 한다.
        tokenGenerator tokenGen(userId, durationTokenType);
        if (tokenGen.generate() != errNone) throw 1410;
        
        //디비에 sign up 데이터 저장한다.
        appDb db(db_);
        if (db.query(dbQuery::getInstance().query(dbQueryDef::insertUserInfo),
            email, appId, name, hashPwd, userId, phone) != errNone) throw 1411;
        db.clear();
        
        Json::Value send;
        send["sign_up_res"]["app_id"] = signUp["app_id"].asInt();
        send["sign_up_res"]["access_token"] = tokenGen.token();
        write(cmdPacket, cmdSignUp, send.asString());
    }
    catch (int& err)
    {
        errHandle(errSignUp, err);
        writeFileLog(LOG_AUTH_PEER, "error: " << err);
        return errDefault;
    }
    return errNone;
}

int authPeer::signIn(const short size, const char* buffer)
{
    try
    {
        const char* json = buffer + headerLength;
        
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) throw 1000;
        Json::Value signIn = root["sign_in_req"];
        if (signIn.isNull()) throw 1500;
        if (signIn["app_id"].isNull() || !signIn["app_id"].isInt()) throw 1501;
        if (!authServer::getInstance().isValidAppId(signIn["app_id"].asInt())) throw 1502;
        if (signIn["email"].isNull()) throw 1503;
        if (signIn["password"].isNull()) throw 1504;
        string password;
        if (tokLib::security::crypto::decrypt(
            password, signIn["password"].asString(), symmetricKey_) != errNone)
            throw 1505;
        int durationTokenType=threeMonthToken;
        if (!signIn["duration_token"].isNull())
        {
            durationTokenType = signIn["duration_token"].asInt();
            if (!validDurationTokenType(durationTokenType)) throw 1506;
        }

        //전송된 비밀번호 해시값 만든다.
        string hashPwd;
        if (tokLib::security::crypto::hash(hashPwd, password) != errNone) throw 1507;
        
        //디비에서 사용자 정보 가져와서 비번 비교한다.
        uint64_t userId=0;
        int appId = signIn["app_id"].isInt();
        string email(signIn["email"].asString());
        string hashPwdFromDb;
        
        appDb db(db_);
        if (db.query(dbQuery::getInstance().query(dbQueryDef::selectIdPassword),
            email, appId) != errNone) throw 1508;
        while (db.fetch() == errNone)
        {
            if (db.fetchValue("user_id", userId) != errNone) throw 1509;
            if (db.fetchValue("password", hashPwdFromDb) != errNone) throw 1510;
        }
        db.clear();
        
        if (hashPwd.compare(hashPwdFromDb)) throw 1511;
        
        tokenGenerator tokenGen(userId, durationTokenType);
        if (tokenGen.generate() != errNone) throw 1512;
        
        Json::Value send;
        send["sign_in_res"]["app_id"] = signIn["app_id"].asInt();
        send["sign_in_res"]["access_token"] = tokenGen.token();
        write(cmdPacket, cmdSignIn, send.asString());
    }
    catch (int& err)
    {
        errHandle(errSignIn, err);
        writeFileLog(LOG_AUTH_PEER, "error: " << err);
        return errDefault;
    }
    return errNone;
}

int authPeer::tokenInfo(const short size, const char* buffer)
{
    try
    {
        const char* json = buffer + headerLength;
        
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) throw 1000;
        Json::Value tokenInfo = root["token_info_req"];
        if (tokenInfo.isNull()) throw 1600;
        if (tokenInfo["app_id"].isNull() || !tokenInfo["app_id"].isInt()) throw 1601;
        if (!authServer::getInstance().isValidAppId(tokenInfo["app_id"].asInt())) throw 1602;
        if (tokenInfo["input_token"].isNull()) throw 1603;
        if (tokenInfo["access_token"].isNull()) throw 1604;
        
        tokenParser parserInput(tokenInfo["input_token"].asString());
        if (parserInput.parse() != errNone) throw 1605;
        tokenParser parserAccess(tokenInfo["access_token"].asString());
        if (parserAccess.parse() != errNone) throw 1606;
        if (!parserInput.isValid() || !parserAccess.isValid()) throw 1607;
        
        string email, name, phone; //from db
        appDb db(db_);
        if (db.query(dbQuery::getInstance().query(dbQueryDef::selectEmailEtc), parserInput.uid())
            != errNone) throw 1608;
        while (db.fetch() == errNone)
        {
            if (db.fetchValue("email", email) != errNone) throw 1609;
            if (db.fetchValue("name", name) != errNone) throw 1610;
            if (db.fetchValue("phone", name) != errNone) throw 1611;
        }
        db.clear();
            
        Json::Value send;
        send["token_info_res"]["app_id"] = tokenInfo["app_id"].asInt();
        send["token_info_res"]["is_valid"] = true;
        send["token_info_res"]["expires"] = parserInput.end();
        send["token_info_res"]["issued"] = parserInput.begin();
        send["token_info_res"]["user_id"] = parserInput.uid();
        send["token_info_res"]["email"] = email;
        send["token_info_res"]["name"] = name;
        send["token_info_res"]["phone"] = phone;
        write(cmdPacket, cmdTokenInfo, send.asString());
    }
    catch (int& err)
    {
        errHandle(errTokenInfo, err);
        writeFileLog(LOG_AUTH_PEER, "error: " << err);
        return errDefault;

    }
    return errNone;
}

int authPeer::renewToken(const short size, const char* buffer)
{
    try
    {
        const char* json = buffer + headerLength;
        
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) throw 1000;
        Json::Value renewToken = root["renew_token_req"];
        if (renewToken.isNull()) throw 1700;
        if (renewToken["app_id"].isNull() || !renewToken["app_id"].isInt()) throw 1701;
        if (!authServer::getInstance().isValidAppId(renewToken["app_id"].asInt())) throw 1702;
        int durationTokenType=threeMonthToken;
        if (!renewToken["duration_token"].isNull())
        {
            durationTokenType = renewToken["duration_token"].asInt();
            if (!validDurationTokenType(durationTokenType)) throw 1703;
        }
        tokenParser parser(renewToken["access_token"].asString());
        if (parser.parse() != errNone) throw 1704;
        if (!parser.isValid())
        {
            appDb db(db_);
            if (db.query(dbQuery::getInstance().query(dbQueryDef::selectId), parser.uid())
                != errNone) throw 1705;
            if (db.fetch() != errNone) throw 1706;
        }
        tokenGenerator generator(parser.uid(), durationTokenType);
        if (generator.generate() != errNone) throw 1707;
        
        Json::Value send;
        send["renew_token_res"]["app_id"] = renewToken["app_id"].asInt();
        send["renew_token_res"]["access_token"] = generator.token();
        write(cmdPacket, cmdRenewToken, send.asString());
    }
    catch (int& err)
    {
        errHandle(errRenewToken, err);
        writeFileLog(LOG_AUTH_PEER, "error: " << err);
        return errDefault;
        
    }
    return errNone;
}

int authPeer::deleteAccount(const short size, const char* buffer)
{
    try
    {
        const char* json = buffer + headerLength;
        
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) throw 1000;
        Json::Value deleteAccount = root["delete_account_req"];
        if (deleteAccount.isNull()) throw 1800;
        if (deleteAccount["app_id"].isNull() || !deleteAccount["app_id"].isInt()) throw 1801;
        if (!authServer::getInstance().isValidAppId(deleteAccount["app_id"].asInt())) throw 1802;
        tokenParser parser(deleteAccount["access_token"].asString());
        if (parser.parse() != errNone) throw 1803;
        if (!parser.isValid()) throw 1804;
        if (deleteAccount["db_delete"].isNull()) throw 1805;
        bool dbDelete = deleteAccount["db_delete"].asBool();
        string qr;
        if (dbDelete)
            qr = dbQuery::getInstance().query(dbQueryDef::deleteUser);
        else
            qr = dbQuery::getInstance().query(dbQueryDef::updateDeleteFlag);
        appDb db(db_);
        if (db.query(qr, parser.uid()) != errNone) throw 1806;
        db.clear();

        Json::Value send;
        send["delete_account_res"]["app_id"] = deleteAccount["app_id"].asInt();
        send["delete_account_res"]["result"] = true;
        write(cmdPacket, cmdDeleteAccount, send.asString());
    }
    catch (int& err)
    {
        errHandle(errDeleteAccount, err);
        writeFileLog(LOG_AUTH_PEER, "error: " << err);
        return errDefault;
    }
    return errNone;
}


