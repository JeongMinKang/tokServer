#include "preCompiled.h"
#include "logCommand.h"
#include "logFiles.h"
#include <sys/time.h>

namespace tokLib 
{
namespace log 
{

const string curTimeLine()
{
    char timeLine[32]={0,};
#ifdef WINDOWS
	SYSTEMTIME curTime;
	GetLocalTime(&curTime);
    _snwprintf_s(timeLine, _countof(timeLine), 32, "%04d%02d%02d%02d%02d%02d.%03d",
				 curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, 
				 curTime.wMinute, curTime.wSecond, curTime.wMilliseconds);
#else
	struct timeval tv;
    gettimeofday(&tv, 0);
    struct tm* curTime;
    curTime = localtime(&tv.tv_sec);
    snprintf(timeLine, 32, "%04d%02d%02d%02d%02d%02d.%06d",
			 curTime->tm_year+1900, curTime->tm_mon+1, curTime->tm_mday,
			 curTime->tm_hour, curTime->tm_min, curTime->tm_sec, tv.tv_usec);
#endif
    return string(timeLine);
}

fileCommand::fileCommand(logFiles* const lf, const string& prefix,
                         const string& msg, short maxLogLen)
	: logFiles_(lf), filePrefix_(prefix)
{
	buildLogLine(msg, maxLogLen);
}

fileCommand::~fileCommand()
{
}

void fileCommand::buildLogLine(const string& msg, short maxLogLen)
{
    string timeLine(curTimeLine()); //YYYYMMDDHHMMSS.MMM
    timeLine += string(" ");

	size_t size = msg.size();
	if (0!=maxLogLen && static_cast<short>(size-1)>maxLogLen) 
    {
		logLine_.reserve(timeLine.size()+maxLogLen+4);
		logLine_.assign(timeLine.begin(), timeLine.end());
		logLine_.insert(logLine_.end(), msg.begin(), msg.begin()+maxLogLen);
        string shorten("...");
		logLine_.insert(logLine_.end(), shorten.begin(), shorten.end());
	} 
    else 
    {
		logLine_.reserve(timeLine.size()+size);
		logLine_.assign(timeLine.begin(), timeLine.end());
		logLine_.insert(logLine_.end(), msg.begin(), msg.end());
	}
}

void fileCommand::execute()
{
    string dateStr, hourStr;
	dateStr.assign(logLine_.begin(), logLine_.begin()+8);
	hourStr.assign(logLine_.begin()+8, logLine_.begin()+10);

	logFiles_->write(filePrefix_, dateStr, hourStr, logLine_);
}

}
}
