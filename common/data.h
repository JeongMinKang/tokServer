#pragma once

const int errNone = 0;
const int errDefault = -1;

namespace tokLib 
{
    
namespace db
{
    
const string dbEmptyValue("");
    
enum queryType
{
    getResultSet=0,
    queryOnly,
    setTextLen,
};

struct dbConfig
{
    short poolSize;
    unsigned short port;
    string host;
    string user;
    string pwd;
    string name;
    string socket;
    string queryFile;
    dbConfig() : poolSize(0), port(0) {}
};

struct dbQueryResult
{
    int colLen;
    int rowLen;
    int colOffset;
    vector<vector<pair<string, string> > > datas;   //column name, value
    dbQueryResult() : colLen(0), rowLen(0), colOffset(0) {}
    void clear() { colLen=0; rowLen=0; colOffset=0; datas.clear(); }
};

} //namespace db
    
namespace app
{
    
struct config
{
    //LOG
    string logPath;
    string logFileExt;
    short maxStringLen;
    //SERVER
    string ip;
    unsigned short tcpPort;
    unsigned short udpPort;
    int preAcceptSockSize;
    short workerThreadPoolSize;
    int peerSize;
    string configFile;
    string pidFile;
    //DB
    db::dbConfig db;
    //function
    config() {}
    virtual ~config() {}
};
    
}
    
    
namespace network
{
    
#pragma pack(push, 1)
struct header
{
    char type;          //패킷타입      0:noti, 1:cmd
    short flag;         //패킷플래그
    short length;       //패킷실제길이 (헤더길이+바디길이)
    int checksum;       //패킷체크썸
    int reserved;       //예약공간
    header() : type(0), flag(0), length(0), checksum(0), reserved(0) {}
    void clear() { type = flag = length = checksum = reserved = 0; }
};
#pragma pack(pop)
    
const int headerLength = sizeof(header);
    
} //namespace network

} //namespace tokLib