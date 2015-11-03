#include "preCompiled.h"
#include "myDb.h"
#include "database.h"

namespace tokLib 
{
namespace db 
{

const string LOG_MY_DB("myDb");

myDb::myDb()
{
}
myDb::~myDb()
{
	terminate();
}

int myDb::create(dbConfig& config)
{
	config_ = config;

    int ret = errNone;
	for (short i=0; i<config_.poolSize; ++i) 
	{
		if ((ret=connect(i)) != errNone)
			return ret;
	}

	return errNone;
}

int myDb::connect(short index, bool assign)
{
	MYSQL* mySql = mysql_init((MYSQL*)NULL);
	if (mySql) 
	{
		MYSQL* conSql=NULL;
		conSql = mysql_real_connect(mySql, config_.host.c_str(), config_.user.c_str(),
            config_.pwd.c_str(), config_.name.empty()?NULL:config_.name.c_str(), config_.port,
            config_.socket.empty()?NULL:config_.socket.c_str(), CLIENT_MULTI_RESULTS);
		if (NULL==conSql) 
		{
            writeFileLog(LOG_MY_DB, "index: " << index << \
                ", mysql_real_connect error: " << mysql_error(mySql));
			return errDefault;
		}
		mysql_set_character_set(mySql, "utf8");
		if (!assign) mySqls_.push_back(mySql);
		else mySqls_[index] = mySql;
	} 
	else 
	{
        writeFileLog(LOG_MY_DB, "index: " << index << ", mysql_init error");
		return errDefault;
	}
	return errNone;
}

int myDb::terminate()
{
    for_each(mySqls_.begin(), mySqls_.end(), bind<void>(mysql_close, placeholders::_1));
	mySqls_.clear();
	return errNone;
}

int myDb::query(short index, const string& qr, dbQueryResult& dqr)
{
	if (index < 0) 
	{
        writeFileLog(LOG_MY_DB, "db index error, index: " << index);
		return errDefault;
	}

    writeFileLog(LOG_MY_DB, "index: " << index << ", query: " << qr);

	int ret = querySql(index, qr, dqr);
	if (ret != errNone)
	{
		if (checkConnect(index) != errNone)
		{
			if (reconnect(index) != errNone)
            {
                writeFileLog(LOG_MY_DB, "index: " << index << ", db reconnect error");
			}
            else
				return querySql(index, qr, dqr);
		}
	}

	return errNone;
}

int myDb::querySql(short index, const string& qr, dbQueryResult& dqr)
{
	dqr.colLen = dqr.rowLen = dqr.colOffset = 0;
	dqr.datas.clear();
	MYSQL_RES* sqlRes=NULL;

	try 
	{
		if (mysql_real_query(mySqls_[index], qr.c_str(), qr.size())) 
		{
            writeFileLog(LOG_MY_DB, "index: " << index << \
                ", mysql_real_query error. errormsg: " << mysql_error(mySqls_[index]));
			return errDefault;
		} 
		else 
		{
			do 
			{
				sqlRes = mysql_store_result(mySqls_[index]);
				if (sqlRes != NULL) 
				{
					MYSQL_FIELD* fields=NULL;
					dqr.rowLen = mysql_num_rows(sqlRes);
					dqr.colLen = mysql_num_fields(sqlRes);
					fields = mysql_fetch_fields(sqlRes);

					int count=0;
					for (; count<dqr.colLen; ++count)
						dqr.colOffset += fields[count].length;

					MYSQL_ROW sqlRow;
					while ((sqlRow=mysql_fetch_row(sqlRes)) != NULL) 
					{
						vector<pair<string, string>> data;
						for (count=0; count<dqr.colLen; ++count) 
						{
							if (fields[count].length>0 && sqlRow[count]!=NULL) 
								data.push_back(make_pair(fields[count].name, string(sqlRow[count])));
							else
								data.push_back(make_pair(fields[count].name, dbEmptyValue));
						} // end of for
						dqr.datas.push_back(data);
					} // end of while
					mysql_free_result(sqlRes);
				}

			} 
			while (!mysql_next_result(mySqls_[index]));
		}
    } 
    catch (exception& e) 
    {
        writeFileLog(LOG_MY_DB, "index: " << index << ", exception: " << e.what());
		if (sqlRes) mysql_free_result(sqlRes);
		return errDefault;
	}

	return errNone;
}

int myDb::checkConnect(short index)
{
	if (0!=mysql_ping(mySqls_[index])) 
	{
        writeFileLog(LOG_MY_DB, "index: " << index << ", mysql server not connected");
		return errDefault;
	}

	return errNone;
}

int myDb::reconnect(short index)
{
	mysql_close(mySqls_[index]);
	return connect(index, true);
}

}
}
