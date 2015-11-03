#pragma once

#include <mysql.h>

namespace tokLib 
{
namespace db 
{

class myDb : private boost::noncopyable
{
private:
	dbConfig config_;
    vector<MYSQL*> mySqls_;
	
	int connect(short index, bool assign=false);
    int querySql(short index, const string& qr, dbQueryResult& dqr);
	int checkConnect(short index);
	int reconnect(short index);

public:
	myDb();
	~myDb();

	int create(dbConfig& config);
	int terminate();
    int query(short index, const string& qr, dbQueryResult& dqr);
};

}
}

