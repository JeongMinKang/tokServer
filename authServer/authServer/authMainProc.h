#pragma once
#include "authServer.h"

class authMainProc : public mainProc
{
private:

public:
	authMainProc(config& data);
	~authMainProc();
	
    void onReadConfig(configFile& cfg);
    int onInit();
	int onStart();
	void onStop();
};