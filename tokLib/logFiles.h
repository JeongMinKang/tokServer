#pragma once

namespace tokLib 
{
namespace log 
{

class logFiles : private boost::noncopyable
{
private:
    string logPath_, fileExt_;
    typedef pair<string, FILE*> filePair; //time string, file pointer
    typedef map<string, filePair> logFilesMap; //file prefix, filePair
	logFilesMap logFiles_;

public:
    logFiles(const string& logPath, const string& fileExt);
	~logFiles();
    void write(const string& prefix, const string& dateStr,
        const string& hourStr, const string& logLine);
};

}
}
