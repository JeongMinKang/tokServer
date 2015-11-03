#include "preCompiled.h"
#include "configFile.h"

namespace tokLib
{
namespace app
{

const char LEFT_TOKEN = '[';
const char RIGHT_TOKEN = ']';
const char VALUE_TOKEN = '=';

configFile::configFile(const string& fileName) : cfgFile_(fileName)
{
}
    
configFile::~configFile()
{
}

int configFile::init()
{
    ifstream ifs;
    ifs.open(cfgFile_.c_str(), ios_base::in);
    if (!ifs.is_open())
        return errDefault;
    
    vector<string> data;
    string key, line;
    while (!ifs.eof())
    {
        getline(ifs, line);
        if (!line.empty() && LEFT_TOKEN==line.at(0) && RIGHT_TOKEN==line.at(line.length()-1))
        {
            if (!data.empty() && !key.empty())
            {
                insertMap(key, data);
                data.clear();
            }
            key = line;
        }
        else if (!line.empty())
        {
            data.push_back(line);
        }
    }
    
    if (!data.empty() && !key.empty())
        insertMap(key, data);
    
    ifs.close();
    
    return errNone;
}
    
string configFile::getValue(const string& section, const string& key)
{
    string strValue("");
    
    string strSection, strKey(key);
	if (section.at(0) != LEFT_TOKEN)
		strSection = LEFT_TOKEN;
	strSection += section;
	if (section.at(section.length()-1) != RIGHT_TOKEN)
		strSection += RIGHT_TOKEN;
    
	auto iter(cfgMap_.find(strSection));
	if (iter==cfgMap_.end()) return strValue;
	if (iter->second.empty()) return strValue;
    
	strKey += VALUE_TOKEN;
	auto begin(iter->second.begin()), end(iter->second.end());
	for (; begin!=end; ++begin)
    {
		size_t posn=begin->find(strKey), keyLen=strKey.length();
        if (string::npos != posn)
        {
			strValue.assign(*begin, posn+keyLen, begin->length()-keyLen);
			break;
		}
	}
    
    return strValue;
}
    

}
}