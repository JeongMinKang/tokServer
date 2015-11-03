#include "preCompiled.h"
#include "logFiles.h"

namespace tokLib 
{
namespace log 
{

#ifdef WINDOWS
#define snprintf _snwprintf
#endif

logFiles::logFiles(const string& logPath, const string& fileExt)
	: logPath_(logPath), fileExt_(fileExt)
{
}

logFiles::~logFiles()
{
    for_each(logFiles_.begin(), logFiles_.end(),
        [](const pair<string, filePair>& entry) { fclose(entry.second.second); });
}

void writeToFile(FILE* fp, const string& logLine)
{
#ifdef WINDOWS
    fwprintf(fp, "%s\r\n", logLine.c_str());
#else
    fprintf(fp, "%s\n", logLine.c_str());
#endif
	fflush(fp);
}

void logFiles::write(const string& prefix, const string& dateStr,
                     const string& hourStr, const string& logLine)
{
    string timeLine(dateStr+hourStr);

	logFilesMap::iterator iter = logFiles_.find(prefix);
	if (iter!=logFiles_.end()) 
    {
		if (!iter->second.first.compare(timeLine)) 
        {
			writeToFile(iter->second.second, logLine);
			return;
		} 
        else 
        {
			fclose(iter->second.second);
			logFiles_.erase(iter);			
		}
	}

    char filePath[256]={0,};
    snprintf(filePath, 256, "%s%s[%s][%s]%s", logPath_.c_str(),
		prefix.c_str(), dateStr.c_str(), hourStr.c_str(), fileExt_.c_str());

#ifdef WINDOWS
    FILE* fp = _wfsopen(filePath, "ab", _SH_DENYWR);
	if (fp) 
    {
		u_short byteOrderMark = 0xFEFF;
		fwrite(&byteOrderMark, sizeof(u_short), 1, fp);
	}
#else
	FILE* fp = fopen(filePath, "at");
#endif

	if (fp) 
    {
		logFiles_.insert(logFilesMap::value_type(prefix, make_pair(timeLine, fp)));
		writeToFile(fp, logLine);
	}
}

}
}
