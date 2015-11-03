#pragma once

namespace tokLib 
{
namespace log 
{

class logFiles;
class command;

class logger : private boost::noncopyable
{
private:
	static logger* instance_;
	bool stop_;
	short maxLogLen_;

	logFiles* logFiles_;

    mutex mutex_;
    condition_variable cond_;
    shared_ptr<thread> thread_;

    vector<command*> logs_;
	
	logger();
	~logger();
	
	void pop();
	void write(command* log);
	
public:
	static logger* getInstance();
	static void releaseInstance();
	
	void start();
	void stop();
	
	void maxLogLen(const short maxLogLen) { maxLogLen_=maxLogLen; }
	const short maxLogLen() const { return maxLogLen_; }
	
	logFiles* const getLogFiles() const { return logFiles_; }

    void createLogFiles(const string& folder, const string& fileExt);
	void destroyLogFiles();

	void push(command* log);
};

void logFileInfo(string& folder, string& fileExt);
//void logDbInfo(LOG_DB_INFO& dbInfo);
void startLog(short maxLogLen=0);
void stopLog();
void pushLog(const string& filePrefix, const string& msg); // write to files
//void pushLog(const string& msg); // write to db

#define writeFileLog(prefix, msg) \
	{ \
        ostringstream oss; \
        oss << "file:" << __FILE__ << " function:" \
            << __FUNCTION__ << " line:" << __LINE__ << " " << msg; \
		tokLib::log::pushLog(prefix, oss.str().c_str()); \
	}
/*
#define writeDbLog(msg) \
	{ \
        ostringstream oss; \
        oss << "file:" << __FILE__ << " function:" \
            << __FUNCTION__ << " line:" << __LINE__ << " " << msg; \
		tokLib::log::pushLog(oss.str().c_str()); \
	}
*/
}
}

