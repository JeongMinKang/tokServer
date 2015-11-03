#include "preCompiled.h"
#include "mainProc.h"
#include "configFile.h"
#include "database.h"

#include <sys/stat.h>
#include <signal.h>

namespace tokLib
{
namespace app
{
    
const string LOG_MAIN_PROC("mainProc");


mainProc::mainProc(config& data) : config_(data)
{
}

mainProc::~mainProc()
{
}

void mainProc::onReadConfig(configFile& cfg)
{
}
    
int mainProc::onInit()
{
    return errNone;
}
    
int mainProc::main(int argc, const char* argv[])
{
    int ret = errNone;
    if (3==argc)
    {
        if (readConfig(string(argv[1])) != errNone)
        {
            cout << "readConfig error\n";
            return errDefault;
        }
        
        if (!strcmp(argv[2], "start") || !strcmp(argv[2], "nofork"))
        {
			if (checkAleadyRun())
            {
                cout << "service aleady running...\n";
				return errDefault;
			}
			
            if (!strcmp(argv[2], "start"))
                daemonize();
			
			sigset_t sigset;
			initStopSignal(&sigset);
            
			// server start
            if ((ret = onInit()) != errNone)
            {
                cout << "mainProc::onInit error\n";
                return ret;
            }
			if ((ret = onStart()) != errNone)
            {
                cout << "mainProc::onStart error\n";
                return ret;
            }
			
			// wait stop signal
			waitStopSignal(&sigset);
        }
        else if (!strcmp(argv[2], "stop"))
        {
			// service 종료처리
			// 프로세스 파일 읽어서 프로세스 아이디 구한 뒤 종료시그널 보낸다.
			sendStopSignal();
        }
        else if (!strcmp(argv[2], "?"))
        {
            cout << "Service Start : authServer cfgname start\n";
            cout << "Service Stop : authServer cfgname stop\n";
		}
        else
        {
            cout << "invalid pararmeter, service stop\n";
		}
	}
    else
    {
        cout << "invalid pararmeter, service stop\n";
	}
    
    return 0;
}
    
bool mainProc::checkAleadyRun()
{
    ifstream pidFile;
    pidFile.open(config_.pidFile, ios_base::in);
    if (pidFile.is_open())
    {
        pidFile.close();
        return true;
    }
    return false;
}

int mainProc::readConfig(const string& configName)
{
    int ret = errNone;
    
    char cwd[256]={0,};
	getcwd(cwd, 256);
	string basicPath(cwd);
    const string::const_reverse_iterator riter = basicPath.rbegin();
	if (*riter != '/')
		basicPath.insert(basicPath.end(), '/');
    
    config_.configFile = basicPath + string("cfg/") + configName + string(".cfg");
    config_.pidFile = basicPath + string("pid/") + configName + string(".pid");
    
    configFile cfg(config_.configFile);
	if ((ret=cfg.init()) != errNone)
		return ret;
    
	//log
    cfg.getValue(string("log"), string("log_path"), config_.logPath);
    cfg.getValue(string("log"), string("log_file_ext"), config_.logFileExt);
    cfg.getValue(string("log"), string("log_max_len"), config_.maxStringLen);
    
    //server
    cfg.getValue(string("server"), string("server_ip"), config_.ip);
    cfg.getValue(string("server"), string("tcp_port"), config_.tcpPort);
    cfg.getValue(string("server"), string("udp_port"), config_.udpPort);
    cfg.getValue(string("server"), string("pre_accept_sock_size"), config_.preAcceptSockSize);
    cfg.getValue(string("server"), string("worker_thread_pool_size"), config_.workerThreadPoolSize);
    
	//db
	config_.db.poolSize = config_.workerThreadPoolSize + 2;
    cfg.getValue(string("db"), string("port"), config_.db.port);
    cfg.getValue(string("db"), string("host"), config_.db.host);
    cfg.getValue(string("db"), string("user"), config_.db.user);
    cfg.getValue(string("db"), string("password"), config_.db.pwd);
    cfg.getValue(string("db"), string("name"), config_.db.name);
    cfg.getValue(string("db"), string("socket"), config_.db.socket);
    cfg.getValue(string("db"), string("query_file"), config_.db.queryFile);
    if (!config_.db.queryFile.empty())
        db::dbQuery::getInstance().parse(config_.db.queryFile);
    
    // read added config
    onReadConfig(cfg);
    
    return ret;
}
    
int mainProc::daemonize()
{
    pid_t pid;
    if ((pid = fork()) < 0)
        exit(0);
    else if (pid !=0)
    {
        makePidFile(pid);
        exit(0);
    }
    
    //chdir("/");   //프로세스가 루트디렉토리에서 작업을 수행하도록 변경
    setsid();       //현재프로세스(자식)의 PID가 세션의 제어권을 가지도록 함
    umask(0);       //데몬이 생성하는 파일 설정권한 변경
    
    return errNone;
}

int mainProc::makePidFile(pid_t pid)
{
    ofstream pidFile;
    pidFile.open(config_.pidFile.c_str(), ios_base::out);
    if (pidFile.is_open())
    {
        pidFile << pid << endl;
        pidFile.close();
    }
    return errNone;
}
    
int mainProc::initStopSignal(sigset_t* sigset)
{
    if (!sigset)
    {
        cout << "initStopSignal, stop signal sigset is null\n";
        return errDefault;
    }
    
    if (sigemptyset(sigset)==-1 || sigaddset(sigset, SIGTERM)==-1)
    {
        cout << "signal set and add error\n";
        return errDefault;
    }
    
    // stop 시그널 처리하기위해...시그널마스크에 추가(SIG_BLOCK)
    // 메인스레드 이외의 다른 스레드는 SIGTERM 받지 않는다.
    pthread_sigmask(SIG_BLOCK, sigset, NULL);
    
    return errNone;
}
    
int mainProc::waitStopSignal(sigset_t* sigset)
{
    if (!sigset)
    {
        cout << "waitStopSignal, stop signal sigset is null\n";
        return errDefault;
    }
    
    while (1)
    {
        int signum=0, status=0;
        status = sigwait(sigset, &signum);
        
        switch (signum)
        {
            case SIGTERM:
                onStop();
                return errNone;
        }
    }
    return errNone;
}
    
void mainProc::sendStopSignal()
{
    ifstream pidFile;
    pidFile.open(config_.pidFile.c_str());
    if (pidFile.is_open())
    {
        string pid;
        pidFile >> pid;
        int pidn = atoi(pid.c_str());
        
        if (pidn)
            kill(pidn, SIGTERM);
        
        pidFile.close();
    }
    else
        cout << "pid file not found\n";
    
    return;
}
    
}
}
