#pragma once

namespace tokLib
{
namespace app
{
    
class configFile;

class mainProc : private boost::noncopyable
{
private:
    config& config_;
    
    bool checkAleadyRun();
    int readConfig(const string& configName);
    int daemonize();
    int makePidFile(pid_t pid);
    
    int initStopSignal(sigset_t* sigset);
	int waitStopSignal(sigset_t* sigset);
	void sendStopSignal();

public:
	mainProc(config& data);
	virtual ~mainProc();
	
	int main(int argc, const char* argv[]);
	
    virtual void onReadConfig(configFile& cfg);    // APP에서 설정사항 추가로 읽을 수 있음
    virtual int onInit();
	virtual int onStart()=0;
	virtual void onStop()=0;
};

}
}