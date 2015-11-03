#pragma once

namespace tokLib 
{
namespace log 
{

class command 
{
public:
    virtual ~command() {}
	virtual void execute()=0;
};

class logFiles;

class fileCommand : public command
{
private:
	logFiles* const logFiles_;
    string filePrefix_;
    string logLine_;
public:
    fileCommand(logFiles* const lf, const string& prefix,
                const string& msg, short maxLogLen=0);
	~fileCommand();

    void buildLogLine(const string& msg, short maxLogLen);
	virtual void execute();
};

}
}

