#pragma once

namespace tokLib
{
namespace app
{
    
class configFile
{
private:
    string cfgFile_;
    typedef map<string, vector<string> > cfgMap;
    cfgMap cfgMap_;
    
    inline void insertMap(const string& key, vector<string>& data)
    {
        cfgMap_.insert(make_pair(key, data));
    }
    string getValue(const string& section, const string& key);
    
public:
    explicit configFile(const string& fileName);
    ~configFile();
    
    int init();
    
    template<typename T>
    int getValue(string section, const string& key, T& value)
    {
        string ret = getValue(section, key);
        if (ret.empty()) return errDefault;
        value = boost::lexical_cast<T>(ret);
        
        return errNone;
    }
};
    
}
}

