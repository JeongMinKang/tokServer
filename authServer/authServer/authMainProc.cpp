#include "preCompiled.h"
#include "authMainProc.h"
#include "authServer.h"
#include "authPeer.h"
#include "appCrypto.h"

const string LOG_AUTH_MAIN_PROC("authMainProc");
extern authConfig appConfig_;

authMainProc::authMainProc(config& data) : mainProc(data)
{
}

authMainProc::~authMainProc()
{
}

void authMainProc::onReadConfig(configFile& cfg)
{   //설정사항 추가로 읽음
    
    //key
    cfg.getValue(string("key"), string("master_password"), appConfig_.masterPwd);
    cfg.getValue(string("key"), string("private_key"), appConfig_.priKey);
    cfg.getValue(string("key"), string("public_key"), appConfig_.pubKey);
    
    return;
}

int authMainProc::onInit()
{
    int ret=0;
    
    string masterPwd;
    if ((ret = appCrypto::masterPassword(masterPwd)) != errNone)
    {
        cout << "cryptoUtil::getMasterPassword error: " << ret << endl;
        return ret;
    }
    authServer::getInstance().masterPwd(masterPwd);
    
    string priKey;
    if ((ret = appCrypto::privateKey(priKey, masterPwd)) != errNone)
    {
        cout << "cryptoUtil::getPrivateKey error: " << ret << endl;
        return ret;
    }
    authServer::getInstance().priKey(priKey);
    
    string pubKey;
    if ((ret = appCrypto::publicKey(pubKey)) != errNone)
    {
        cout << "cryptoUtil::getPublicKey error: " << ret << endl;
        return ret;
    }
    authServer::getInstance().pubKey(pubKey);

    return errNone;
}

int authMainProc::onStart()
{
    tokLib::log::logFileInfo(appConfig_.logPath, appConfig_.logFileExt);
	tokLib::log::startLog(500);
    
    writeFileLog(LOG_AUTH_MAIN_PROC, "service start...");
    
    appConfig_.peerSize = sizeof(authPeer);
    int ret = authServer::getInstance().init(&appConfig_);
    if (ret != errNone)
        return ret;
   
    authServer::getInstance().run();
    
    return errNone;
}

void authMainProc::onStop()
{
    authServer::getInstance().stop();
    
    remove(appConfig_.pidFile.c_str());
    writeFileLog(LOG_AUTH_MAIN_PROC, "service stop...");
    
	sleep(1);
	tokLib::log::stopLog();

    return;
}