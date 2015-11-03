#include "preCompiled.h"
#include "database.h"
#include "json/json.h"

namespace tokLib
{
namespace db
{
        
dbQuery::dbQuery() : errQuery_(string(""))
{
}

dbQuery::~dbQuery()
{
}
    
int dbQuery::parse(const string& queryFile)
{
    ifstream ifs(queryFile.c_str(), ios_base::in|ios_base::binary);
    if (!ifs) return errDefault;
    
    ifs.seekg (0, ifs.end);
    int length = ifs.tellg();
    ifs.seekg (0, ifs.beg);
    
    char* buffer = new char[length+1];
    ifs.read(buffer, length);
    buffer[length] = 0;
    
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(buffer, root))
        return errDefault;
    
    for (auto iter=root.begin(); iter!=root.end(); ++iter)
    {
        Json::Value key = iter.key();
        Json::Value value = *iter;
        int index = stoi(key.asString());
        query_.insert(make_pair(index, value.asString()));
    }
        
    delete[] buffer;
    
    return errNone;
}

const string& dbQuery::query(const int queryNumber)
{
    auto iter = query_.find(queryNumber);
    if (iter != query_.end())
        return iter->second;
    return errQuery_;
}

}
}