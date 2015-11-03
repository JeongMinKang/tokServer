#include "preCompiled.h"
#include "authMainProc.h"

authConfig appConfig_;

int main(int argc, const char* argv[])
{
    authMainProc mp(appConfig_);
    mp.main(argc, argv);

    return 0;
}

