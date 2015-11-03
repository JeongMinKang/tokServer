#include "preCompiled.h"
#include "logger.h"
#include "logCommand.h"
#include "logFiles.h"

namespace tokLib 
{
namespace log 
{

mutex mutexDcl_; // (Double-Checked Locking)
logger* logger::instance_=0;

void logFileInfo(string& folder, string& fileExt)
{
	if (folder.length()) 
	{
        const string::const_reverse_iterator riter = folder.rbegin();
#ifdef WINDOWS
		if (*riter != '\\')
			folder.insert(folder.end(), '\\');
#else
		if (*riter != '/')
			folder.insert(folder.end(), '/');
#endif
	} 
	else
		return;

	if (fileExt.length()) 
	{
        const string::const_iterator iter = fileExt.begin();
		if (*iter != '.')
			fileExt.insert(fileExt.begin(), '.');
	}
	
	logger::getInstance()->createLogFiles(folder, fileExt);
}

void startLog(short maxLogLen)
{
	logger::getInstance()->maxLogLen(maxLogLen);
	logger::getInstance()->start();
}

void stopLog()
{
	logger::getInstance()->stop();
	logger::releaseInstance();
}

void pushLog(const string& filePrefix, const string& msg)
{
	fileCommand* log = 
		new fileCommand(logger::getInstance()->getLogFiles(), 
						filePrefix, msg, logger::getInstance()->maxLogLen());
	logger::getInstance()->push(log);

	return;
}

// logger class ////////////////////////////////////////////////
// called in single thread
logger* logger::getInstance()
{
	if (0==instance_) 
	{
        unique_lock<mutex> lock(mutexDcl_);
		if (0==instance_)
			instance_ = new logger;
	}
	return instance_;
}

void logger::releaseInstance()
{
	if (instance_) 
	{
		delete instance_;
		instance_=0;
	}
}

logger::logger() : stop_(false), maxLogLen_(0), logFiles_(NULL)
{
}

logger::~logger()
{
	if (!stop_)
		stop();
}

void logger::start()
{
    thread_.reset(new thread(bind(&logger::pop, this)));
}

void logger::stop()
{
	stop_=true;

	// pop wakeup
	cond_.notify_one();
	// pop thread end
	if (thread_) thread_->join();

	destroyLogFiles();
}

void logger::createLogFiles(const string& folder, const string& fileExt)
{
	try 
	{
		logFiles_ = new logFiles(folder, fileExt);
    } 
    catch (exception& e) 
    {
        cout << "standard exception: " << e.what() << endl;
	}
}

void logger::destroyLogFiles()
{
	if (logFiles_) 
	{
		delete logFiles_;
		logFiles_ = NULL;
	}
}

void logger::push(command* log)
{
    unique_lock<mutex> lock(mutex_);
	logs_.push_back(log);
	cond_.notify_one();
}

void logger::pop()
{
    vector<command*> logs;

	while (!stop_) 
	{
        unique_lock<mutex> lock(mutex_);
		if (logs_.empty())
			cond_.wait(lock); //mutex unlock

		// mutex lock
		logs_.swap(logs);
		lock.unlock();
		
        for_each(logs.begin(), logs.end(), bind(&logger::write, this, placeholders::_1));
		logs.clear();
	}
}

void logger::write(command* log)
{
	if (log) 
	{
		log->execute();
		delete log;
	}
}

}
}
